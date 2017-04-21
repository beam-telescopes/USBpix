///////////////////////////////////////////////////////////////////
// PixGPIBDevice.cxx
//   Implementation file for class PixGPIBDevice
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//  Version 00-00-01 21/11/2008 Daniel Dobos
//  Modified by Peyton Murray
//  2013-05-27 Modified by Philipp Weigell
///////////////////////////////////////////////////////////////////
//
//  13-12-2013 John Matheson removed error which caused some scancard
//  Keithleys to be treated as single channel
//

#include "PixGPIBDevice.h"
#include "PixGPIBError.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <math.h>
#include <cstring>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <upsleep.h>

#if defined(USE_LINUX_GPIB)
#include "gpib/ib.h"
#else
#include "ni488.h"
#endif


#define DEBUG false

using namespace std;

////////////////////////
// constructors
////////////////////////
PixGPIBDevice::PixGPIBDevice(unsigned int board, unsigned int PAD, unsigned int SAD, bool forceMeter, bool errOnStdout)
{
    m_PixGPIBError = new PixGPIBError(errOnStdout);
    m_meterReadType = -1;
    m_Board = board;
    m_PAD = PAD;
    m_SAD = SAD;
    m_Device = ibdev(m_Board,m_PAD,m_SAD,T10s,1,0x0000);
    updateDescription();
    // the Keithleys can also be used as meter, so allow to change from HV to METER
    // 2*measurement range is the compliance
    if(forceMeter && (m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400 || m_DeviceType == KEITHLEY_2602 || m_DeviceType == KEITHLEY_6517A || m_DeviceType == KEITHLEY_6517B)) m_DeviceFunction = METER;
    else updateDeviceFunction();
    updateDeviceNumberChannels();
    initializeDevice();
    m_range = (float)-999999999.;
    m_autoRange = true;
    m_activeChannel = -1;
}

////////////////////////
// destructor
////////////////////////
PixGPIBDevice::~PixGPIBDevice()
{
    disarm();
    // Keithleys as meter must be turned off
    if ((m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400 ) && m_DeviceFunction == METER)
    {
        if(DEBUG) cout << "turning Keithley off" << endl;
        char command[120];
	UPG_sprintf(command, "OUTP OFF");
        writeDevice(command);
        UPGen::Sleep(500);
    }
    ibclr(m_Device);
    if (ibsta & ERR) m_PixGPIBError->printGPIBError("* ibclr Error");
    ibonl(m_Device, 0);
    if (ibsta & ERR) m_PixGPIBError->printGPIBError("* ibonl Error");

    delete m_PixGPIBError;
}

////////////////////////
// getBoard()
////////////////////////
unsigned int PixGPIBDevice::getBoard()
{
    return m_Board;
}

////////////////////////
// getPAD()
////////////////////////
unsigned int PixGPIBDevice::getPAD()
{
    return m_PAD;
}

////////////////////////
// getSAD()
////////////////////////
unsigned int PixGPIBDevice::getSAD()
{
    return m_SAD;
}

////////////////////////
// getDevice()
////////////////////////
unsigned int PixGPIBDevice::getDevice()
{
    return m_SAD;
}

////////////////////////
// getDescription()
////////////////////////
char * PixGPIBDevice::getDescription()
{
    return m_Description;
}

////////////////////////
// updateDescription()
////////////////////////
int PixGPIBDevice::updateDescription()
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::updateDescription()" << endl;
    ibclr(m_Device);
    if (ibsta & ERR)
    {
        m_PixGPIBError->printGPIBError("* ibclr Error");
        return 1;
    }
    char command[120];
    UPG_sprintf(command,"*IDN?");
    if (writeDevice(command)) return 1;
    ibrd(m_Device, m_Description, 120);
    if (ibsta & ERR)
    {
        m_PixGPIBError->printGPIBError("* ibrd Error");
        return 1;
    }
    m_Description[ibcntl] = '\0';
    string description = m_Description;
    if(DEBUG) cout << "String ID " << description << endl;
    if      (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,E3631A,") == 0) m_DeviceType = AGILENT_3631A;
    else if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,34401A,") == 0) m_DeviceType = HP_34401A;
    else if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,34970A,") == 0) m_DeviceType = HP_34970A;
	/*{
    m_DeviceType = HP_34970A;
	m_slotDevTypes[3] = -1;
      for(int i=0;i<3;i++){// device has 3 slots, determine type //It doesn't work. Genova Slot is number 3
	m_slotDevTypes[i] = UNKNOWN;
	char slotDescription[121];
	UPG_sprintf(command,"SYST:CTYP? %d00",i); //the HP_34970A slots are [100 200 300] not [000 100 200]
	if (writeDevice(command)) continue;
	ibrd(m_Device, slotDescription, 120);
	if (ibsta & ERR)
	  {
	    m_PixGPIBError->printGPIBError("* ibrd Error");
	    continue;
	  }
	slotDescription[ibcntl] = '\0';
	description = slotDescription;
	if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,34901A") == 0) m_slotDevTypes[i] = HP_34901A;
	if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,34902A") == 0) m_slotDevTypes[i] = HP_34902A;
	if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,34903A") == 0) m_slotDevTypes[i] = HP_34903A;
	if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,34908A") == 0) m_slotDevTypes[i] = HP_34908A;
      }
    }*/
    else if (strcmp((description.substr(0,28)).c_str(),"Agilent Technologies,E3642A,") == 0) m_DeviceType = AGILENT_E3642A;
    else if (strcmp((description.substr(0,28)).c_str(),"Agilent Technologies,E3644A,") == 0) m_DeviceType = AGILENT_E3644A;
	else if (strcmp((description.substr(0,23)).c_str(),"THURLBY-THANDAR,PL330DP") == 0)   m_DeviceType = TTI_PL330DP;
    else if (strcmp((description.substr(0,23)).c_str(),"THURLBY-THANDAR,PL330TP") == 0)   m_DeviceType = TTI_PL330TP;
    else if (strcmp((description.substr(0,25)).c_str(),"THURLBY THANDAR, PL303QMD") == 0) m_DeviceType = TTI_PL303QMD;
    else if (strcmp((description.substr(0,23)).c_str(),"THURLBY-THANDAR,QL355TP") == 0)   m_DeviceType = TTI_QL355TP;
    else if (strcmp((description.substr(0,24)).c_str(),"THURLBY-THANDAR, QL355TP") == 0)   m_DeviceType = TTI_QL355TP;
    else if (strcmp((description.substr(0,37)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 2410,") == 0) m_DeviceType = KEITHLEY_2410;
    else if (strcmp((description.substr(0,37)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 2400,") == 0) m_DeviceType = KEITHLEY_2400;
	else if (strcmp((description.substr(0,37)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 2700,") == 0) m_DeviceType = KEITHLEY_2700;//ADD 2700
    else if (strcmp((description.substr(0,37)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 2000,") == 0) m_DeviceType = KEITHLEY_2000;
    else if (strcmp((description.substr(0,37)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 2001,") == 0) m_DeviceType = KEITHLEY_2001;
	else if (strcmp((description.substr(0,39)).c_str(),"Keithley Instruments Inc., Model 2602A,") == 0) m_DeviceType = KEITHLEY_2602; // this works :-)
    else if (strcmp((description.substr(0,38)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 6517A,") == 0) m_DeviceType = KEITHLEY_6517A;
    else if (strcmp((description.substr(0,38)).c_str(),"KEITHLEY INSTRUMENTS INC.,MODEL 6517B,") == 0) m_DeviceType = KEITHLEY_6517B;
	else if (strcmp((description.substr(0,27)).c_str(),"Agilent Technologies,34410A") == 0) m_DeviceType = AGILENT_34410A;
    else if (strcmp((description.substr(0,23)).c_str(),"HEWLETT-PACKARD,HP8110A") == 0) m_DeviceType = AGILENT_8110A;
    else if (strcmp((description.substr(0,27)).c_str(),"Agilent Technologies,33250A") == 0) m_DeviceType = AGILENT_33250A;
    else if ((description.c_str()[0] == '+') || (description.c_str()[0] == '-'))				//Special handling for the old timey Fluke 8842A commands.
	{
        UPGen::Sleep(500);

        char answer[120];
        UPG_sprintf(command, "G8");
        if (writeDevice(command)) return 1;
        if (readDevice(answer)) return 1;
        description = answer;

        UPG_strcpy(m_Description, description.c_str());
        if (strcmp((description.substr(0,11)).c_str(),"FLUKE,8842A") == 0) m_DeviceType = FLUKE_8842A;
    }
    else
    {
        if(DEBUG) cout << "unidentified description: " << description << endl;
        ibclr(m_Device);
        if (ibsta & ERR)
        {
            m_PixGPIBError->printGPIBError("* ibclr Error");
            return 1;
        }
        UPG_sprintf(command,"U0X");
        writeDevice(command);
        ibrd(m_Device, m_Description, 120);
        if (ibsta & ERR)
        {
            m_PixGPIBError->printGPIBError("* ibrd Error");
            return 1;
        }
        m_Description[ibcntl] = '\0';
        description = m_Description;
        if (strcmp((description.substr(0,3)).c_str(),"487") == 0) m_DeviceType = KEITHLEY_487;
        else m_DeviceType = UNKNOWN;
    }



// check for the possible extension for this multimeters: a 10-channel mutliplexer
    if (m_DeviceType == KEITHLEY_2000 || m_DeviceType == KEITHLEY_2001 || m_DeviceType == KEITHLEY_2700)//ADD 2700
    {
      char answer[120];
      UPG_sprintf(command,"*OPT?");
      if (writeDevice(command)) return 1;
      if (readDevice(answer)) return 1;
//    if (strncmp(answer ,"0,2001-SCAN", 11) == 0 || strncmp(answer ,"0,200X-SCAN", 11) == 0)// edited by JM 11/12/13
      if (strncmp(answer ,"0,2001-SCAN", 11) == 0 ||
	  strncmp(answer ,"MEM1,2001-SCAN", 14) == 0 ||
	  strncmp(answer ,"MEM2,2001-SCAN", 14) == 0 || 
	  strncmp(answer ,"0,200X-SCAN", 11) == 0 ||
	  strncmp(answer ,"MEM1,200X-SCAN", 14) == 0 ||
	  strncmp(answer ,"MEM2,200X-SCAN", 14) == 0) //meter with any memory option got treated as single channel before
	{
	  if(m_DeviceType == KEITHLEY_2000)
	    m_DeviceType = KEITHLEY_2000_SCANCARD;
	  else
	    m_DeviceType = KEITHLEY_2001_SCANCARD;
	}
    }
    arm();
    return 0;
}

////////////////////////
// getDeviceType()
////////////////////////
DeviceType PixGPIBDevice::getDeviceType()
{
    return m_DeviceType;
}

////////////////////////
// getDeviceFunction()
////////////////////////
DeviceFunction PixGPIBDevice::getDeviceFunction()
{
    return m_DeviceFunction;
}

////////////////////////
// getDeviceNumberChannels()
////////////////////////
unsigned int PixGPIBDevice::getDeviceNumberChannels()
{
    return m_DeviceNumberChannels;
}

////////////////////////
// updateDeviceNumberChannels()
////////////////////////
void PixGPIBDevice::updateDeviceNumberChannels()
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::updateDeviceNumberChannels()" << endl;
    if (m_DeviceType == AGILENT_3631A) m_DeviceNumberChannels = 3;
    else if (m_DeviceType == HP_34401A) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == HP_34970A) m_DeviceNumberChannels = 40;//Genova Device has 40 ch
	/*
	else if (m_DeviceType == HP_34970A){
      for(int i=0;i<3;i++){
	int nCh=0;
	switch(m_slotDevTypes[i]){
	case HP_34901A:
	case HP_34903A:
	  nCh=20;
	  break;
	case HP_34902A:
	  nCh=16;
	  break;
	case HP_34908A:
	  nCh=40;
	  break;
	default:
	  break;
	}
	if(nCh>m_DeviceNumberChannels) m_DeviceNumberChannels = nCh;
      }
    }*/
    else if (m_DeviceType == AGILENT_E3642A) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == AGILENT_E3644A) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == AGILENT_364xA1CH) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == AGILENT_364xA2CH) m_DeviceNumberChannels = 2;
    else if (m_DeviceType == TTI_PL330DP) m_DeviceNumberChannels = 2;
    else if (m_DeviceType == TTI_PL330TP) m_DeviceNumberChannels = 3;
    else if (m_DeviceType == TTI_PL303QMD) m_DeviceNumberChannels = 2;
    else if (m_DeviceType == TTI_QL355TP) m_DeviceNumberChannels = 2;
    else if (m_DeviceType == KEITHLEY_237) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == KEITHLEY_487) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == KEITHLEY_2410) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == KEITHLEY_2400) m_DeviceNumberChannels = 1;
	else if (m_DeviceType == KEITHLEY_2700) m_DeviceNumberChannels = 20;//Genova KEITHLEY_2700 has 20 Ch
    else if (m_DeviceType == KEITHLEY_2000) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == KEITHLEY_2001) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == KEITHLEY_2602) m_DeviceNumberChannels = 2;  // theresa
    else if (m_DeviceType == KEITHLEY_2000_SCANCARD) m_DeviceNumberChannels = 10;
    else if (m_DeviceType == KEITHLEY_2001_SCANCARD) m_DeviceNumberChannels = 10;
    else if (m_DeviceType == KEITHLEY_6517A) m_DeviceNumberChannels = 1;   
	else if (m_DeviceType == KEITHLEY_6517B) m_DeviceNumberChannels = 1;  
    else if (m_DeviceType == AGILENT_34410A) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == FLUKE_8842A) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == AGILENT_8110A) m_DeviceNumberChannels = 1;
    else if (m_DeviceType == AGILENT_33250A) m_DeviceNumberChannels = 1;
    else m_DeviceNumberChannels = 0;
}


////////////////////////
// updateDeviceFunction()
////////////////////////
void PixGPIBDevice::updateDeviceFunction()
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::updateDeviceFunction()" << endl;
    if (m_DeviceType == AGILENT_3631A) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == HP_34401A) m_DeviceFunction = METER;
    else if (m_DeviceType == HP_34970A) m_DeviceFunction = METER;
    else if (m_DeviceType == AGILENT_E3642A) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == AGILENT_E3644A) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == AGILENT_364xA1CH) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == AGILENT_364xA2CH) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == TTI_PL330DP) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == TTI_PL330TP) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == TTI_PL303QMD) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == TTI_QL355TP) m_DeviceFunction = LV_MODULE;
    else if (m_DeviceType == KEITHLEY_237) m_DeviceFunction = HV;
    else if (m_DeviceType == KEITHLEY_487) m_DeviceFunction = HV;
    else if (m_DeviceType == KEITHLEY_2410) m_DeviceFunction = HV;
    else if (m_DeviceType == KEITHLEY_2400) m_DeviceFunction = HV;
	else if (m_DeviceType == KEITHLEY_2602) m_DeviceFunction = HV;
    else if (m_DeviceType == KEITHLEY_2000) m_DeviceFunction = METER;
    else if (m_DeviceType == KEITHLEY_2001) m_DeviceFunction = METER;
	else if (m_DeviceType == KEITHLEY_2700) m_DeviceFunction = METER;// KEITHLEY_2700 is used like METER;
    else if (m_DeviceType == KEITHLEY_2000_SCANCARD) m_DeviceFunction = METER;
    else if (m_DeviceType == KEITHLEY_2001_SCANCARD) m_DeviceFunction = METER;
    else if (m_DeviceType == KEITHLEY_6517A) m_DeviceFunction = HV;  
    else if (m_DeviceType == KEITHLEY_6517A) m_DeviceFunction = METER;  
    else if (m_DeviceType == AGILENT_34410A) m_DeviceFunction = METER;
    else if (m_DeviceType == FLUKE_8842A) m_DeviceFunction = METER;
    else if (m_DeviceType == AGILENT_8110A) m_DeviceFunction = PULSER;
    else if (m_DeviceType == AGILENT_33250A) m_DeviceFunction = PULSER;
    else m_DeviceFunction = NONE;
}

////////////////////////
// getStatus()
////////////////////////
DeviceStatus PixGPIBDevice::getStatus()
{
    return m_Status;
}

////////////////////////
// initializeDevice()
////////////////////////
int PixGPIBDevice::initializeDevice()
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::initializeDevice()" << endl;
    // send status request command
    char command[120];
    disarm();

    // special dirty trick for "old" TTi PL330's and QL355's since there is no on/off status information
    if (m_DeviceType == TTI_PL330DP || m_DeviceType == TTI_PL330TP || m_DeviceType == TTI_QL355TP)
    {
        // check voltage on channel 1
        UPG_sprintf(command, "V1O?");
        if (writeDevice(command)) return 1;
        char answer[120];
        if (readDevice(answer)) return 1;
        answer[ibcntl] = '\0';
        if(DEBUG) cout << "DEBUG   : TTi volt request - answer: " << answer << endl;
        float volts = (float)atof(answer);
        if(fabs(volts)<1e-2)
        {
            // setting is zero volts - check if this can be changed
            // if yes, status is on, if not, status is off
            // set de-/incremental delta
            UPG_sprintf(command, "DELTAV1 0.1");
            if (writeDevice(command)) return 1;

            // increment voltage
            UPG_sprintf(command, "INCV1");
            if(DEBUG) cout << "DEBUG   : executing command " << command << endl;
            if (writeDevice(command)) return 1;
	    UPGen::Sleep(500);
            // read back and check for change
            UPG_sprintf(command, "V1O?");
            if (writeDevice(command)) return 1;
            answer[0] ='\0';
            if (readDevice(answer)) return 1;
            answer[ibcntl] = '\0';
            if(DEBUG) cout << "DEBUG   : TTi increm. volt request - answer: " << answer << endl;
            // return to original value
            UPG_sprintf(command, "DECV1");
            if (writeDevice(command)) return 1;
            volts = (float)atof(answer);
            if(fabs(volts)<0.03)
            {
                m_Status = PGD_OFF ;
                return 0;
            }
            else
            {
                m_Status = PGD_ON ;
                return 0;
            }
        }
        else
        {
            m_Status = PGD_ON ;
            return 0;
        }
    }

    if (m_DeviceType == KEITHLEY_237) UPG_sprintf(command, "U3X");
    else if (m_DeviceType == KEITHLEY_487) UPG_sprintf(command, "U0X");
    // NB: since we're only using OPALL, it should be OK to check just channel 1, channel 2 must be identical
    else if (m_DeviceType == TTI_PL303QMD) UPG_sprintf(command, "OP1?");
    else if((m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400) && m_DeviceFunction == METER)
    {
        UPG_sprintf(command, ":ABORT");
        if (writeDevice(command)) return 1;
        UPG_sprintf(command, ":SOUR:VOLT 0");				//to protect DUT
        if (writeDevice(command)) return 1;
        UPG_sprintf(command, ":SOUR:VOLT:RANG MIN");		//to protect DUT
        if (writeDevice(command)) return 1;
        UPG_sprintf(command, ":SOUR:CURR 0");
        if (writeDevice(command)) return 1;
        UPG_sprintf(command, ":SOUR:CURR:RANG MIN");		//to protect DUT
        if (writeDevice(command)) return 1;
        m_Status = PGD_ON ;
        return 0;
    }
	else if(m_DeviceType == KEITHLEY_2602)
    {

	//	m_DeviceFunction = METER;
		if(m_DeviceFunction == METER){
		//getch();
//			UPG_sprintf(command, "smua.source.levelv = 0");				//to protect DUT
//		    if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smub.source.levelv = 0");				//to protect DUT
//	        if (writeDevice(command)) return 1;
//		    UPG_sprintf(command, "smua.source.rangev = 1");				//to protect DUT
//			if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smub.source.rangev = 1");				//to protect DUT
//		    if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smua.source.rangei = 1");				//to protect DUT
//	        if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smub.source.rangei = 1");				//to protect DUT
//			if (writeDevice(command)) return 1;

//			UPG_sprintf(command, "smua.source.func = smua.OUTPUT_DCAMPS");				//to protect DUT
//			if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smua.source.output = smua.OUTPUT_ON");				//to protect DUT
//			if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smub.source.func = smub.OUTPUT_DCAMPS");				//to protect DUT
//			if (writeDevice(command)) return 1;
//			UPG_sprintf(command, "smub.source.output = smub.OUTPUT_ON");				//to protect DUT
//			if (writeDevice(command)) return 1;
			UPG_sprintf(command, "display.smua.measure.func = display.MEASURE_DCVOLTS");				//to protect DUT
			if (writeDevice(command)) return 1;
			UPG_sprintf(command, "display.smub.measure.func = display.MEASURE_DCVOLTS");				//to protect DUT
			if (writeDevice(command)) return 1;			

	//		UPG_sprintf(command, "smua.measure.rangev = 10.0");				//to protect DUT
	//		if (writeDevice(command)) return 1;			
	//		UPG_sprintf(command, "smub.measure.rangev = 10.0");				//to protect DUT
	//		if (writeDevice(command)) return 1;		

			UPG_sprintf(command, "sourceOutput = smua.source.output");
			if (writeDevice(command)) return 1;
			UPG_sprintf(command, "print(sourceOutput)");

			UPG_sprintf(command, "sourceOutput = smub.source.output");
			if (writeDevice(command)) return 1;
			UPG_sprintf(command, "print(sourceOutput)");

			m_Status = PGD_ON ;
			return 0;
		}else{
			UPG_sprintf(command, "sourceOutput = smua.source.output");
			if (writeDevice(command)) return 1;
			UPG_sprintf(command, "print(sourceOutput)");

			UPG_sprintf(command, "sourceOutput = smub.source.output");
			if (writeDevice(command)) return 1;
			UPG_sprintf(command, "print(sourceOutput)");
		}
    }
    else if(m_DeviceType == KEITHLEY_2000 || m_DeviceType == KEITHLEY_2001)
    {
        m_Status = PGD_ON ;
        UPG_sprintf(command, "*RST");
        if (writeDevice(command)) return 1;
        return 0;
    }
    else if(m_DeviceType == KEITHLEY_2000_SCANCARD || m_DeviceType == KEITHLEY_2001_SCANCARD)
    {
        m_Status = PGD_ON ;
        UPG_sprintf(command, "*RST");
        if (writeDevice(command)) return 1;
        UPG_sprintf(command, "ROUTE:OPEN:ALL");
        if (writeDevice(command)) return 1;
        return 0;
    }
	else if(m_DeviceType == KEITHLEY_2700) //KEITHLEY_2700
	{
	UPG_sprintf(command, "*RST");
    writeDevice(command);
    UPG_sprintf(command, "ROUTE:OPEN:ALL");
	writeDevice(command);
	m_Status = PGD_ON ;
	return 0;
	}
    else if(m_DeviceType == AGILENT_34410A)  // nothing to do for Agilent meter
    {
        m_Status = PGD_ON ;
        return 0;
    }
    else if (m_DeviceType == AGILENT_E3642A)//Nothing to do for agilent meter.
    {
        m_Status = PGD_OFF ; //If you turn it on right away, it behaves weirdly until you turn it on and off a few times in STControl.
        return 0;
    }
    else if (m_DeviceType == AGILENT_E3644A)
    {
        m_Status = PGD_OFF ; //If you turn it on right away, it behaves weirdly until you turn it on and off a few times in STControl.
        return 0;
    }
    else if (m_DeviceType == HP_34401A)
    {
        m_Status = PGD_ON ;
        UPG_sprintf(command, "*RST");
        if (writeDevice(command)) return 1;
        return 0;
    }
    else if (m_DeviceType == HP_34970A)
    {
     /*   m_Status = PGD_ON ;
        UPG_sprintf(command, "*RST");
        if (writeDevice(command)) return 1;
        UPG_sprintf(command, "*CLS");
        if (writeDevice(command)) return 1;
        return 0;*/
		m_Status = PGD_ON ;
        UPG_sprintf(command, "*RST");
        if (writeDevice(command)) return 1; //A.G. Reset and Set Slot 3
		UPG_sprintf(command, "SYST:CPON 300");
        if (writeDevice(command)) return 1;
		m_Status = PGD_ON ;
        return 0;

    }
    else if (m_DeviceType == FLUKE_8842A)
    {
        m_Status = PGD_ON;
        UPG_sprintf(command, "* S1");
        if (writeDevice(command)) return 1;
        return 0;
    }
    else
    {
        UPG_sprintf(command, "OUTP:STAT?");
    }
    if (writeDevice(command)) return 1;			//Possibly not necessary, and maybe even a bug

    // receive status request answer and set m_Status
    char answer[120];
    if (readDevice(answer)) return 1;
    answer[ibcntl] = '\0';
    if(DEBUG) cout << "DEBUG   : initialize - answer: " << answer << endl;

    int status;
    if (m_DeviceType == KEITHLEY_237) status = (int)answer[19] - 48;
    else if (m_DeviceType == KEITHLEY_487) status = (int)answer[29] - 48;
    else status = atoi(answer);
    if (status == 1) m_Status = PGD_ON;
    else if (status == 0) m_Status = PGD_OFF;
    else m_Status = PGD_ERROR;

    if(m_DeviceType == KEITHLEY_2410){
      UPG_sprintf(command,":SOUR:VOLT:RANG 1000");
      if (writeDevice(command)) return 1;
      UPG_sprintf(command,":SENS:CURR:RANG:AUTO ON");
      if (writeDevice(command)) return 1;
      UPG_sprintf(command,":SENS:VOLT:RANG:AUTO ON");
      if (writeDevice(command)) return 1;
    }
    if(m_DeviceType == KEITHLEY_2400){
      UPG_sprintf(command,":SOUR:VOLT:RANG 200");
      if (writeDevice(command)) return 1;
      UPG_sprintf(command,":SENS:CURR:RANG:AUTO ON");
      if (writeDevice(command)) return 1;
      UPG_sprintf(command,":SENS:VOLT:RANG:AUTO ON");
      if (writeDevice(command)) return 1;
  }
    if(m_DeviceType == KEITHLEY_2602 && m_DeviceFunction != METER){  // if it is used as HV supply
		UPG_sprintf(command, "smua.source.levelv = 0");	
		if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smub.source.levelv = 0");
	    if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smua.source.rangev = 1");
		if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smub.source.rangev = 1");
		if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smua.source.Autorangei = smua.AUTORANGE_ON");
	    if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smub.source.Autorangei = smub.AUTORANGE_ON");
		if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smua.source.Autorangev = smua.AUTORANGE_ON");
	    if (writeDevice(command)) return 1;
		UPG_sprintf(command, "smub.source.Autorangev = smub.AUTORANGE_ON");
		if (writeDevice(command)) return 1;
	}
  if(m_DeviceType == KEITHLEY_6517A){
	UPG_sprintf(command,":OUTP 0");	
	if (writeDevice(command)) return 1;    
	UPG_sprintf(command,":CONF:CURR");    
	if (writeDevice(command)) return 1;    
	//UPG_sprintf(command,":SENS:CURR:RANG 2e-6");	
	UPG_sprintf(command,":SENS:CURR:RANG:AUTO ON");    
	if (writeDevice(command)) return 1;    
	UPG_sprintf(command,":SOUR:VOLT:RANG 1000");    
	if (writeDevice(command)) return 1;   
	UPG_sprintf(command,":OUTP 1");   
	if (writeDevice(command)) return 1;   
	UPG_sprintf(command,":SYST:ZCH 0"); 
	if (writeDevice(command)) return 1;  
	UPG_sprintf(command,":FORM:ELEM READ,UNIT");   
	if (writeDevice(command)) return 1;  
	UPG_sprintf(command,":UNIT:TEMP C");  
  }  
  if(m_DeviceType == KEITHLEY_6517B && m_DeviceFunction == METER){	
    if(DEBUG) cout << "DEBUG_P1" << endl;   
	  UPG_sprintf(command,":OUTP 0");
	  if (writeDevice(command)) return 1;
	  UPG_sprintf(command,":SYST:ZCH 1");   
	  if (writeDevice(command)) return 1;
	  UPG_sprintf(command,":CONF:VOLT");   
	  if (writeDevice(command)) return 1;
	  //UPG_sprintf(command,":SENS:CURR:RANG:AUTO ON"); 
	  //if (writeDevice(command)) return 1; 
	  UPG_sprintf(command,":SENS:VOLT:RANG:AUTO ON");  
	  if (writeDevice(command)) return 1;  
	  UPG_sprintf(command,":SYST:ZCH 0");   
	  if (writeDevice(command)) return 1;  
	  UPG_sprintf(command,":FORM:ELEM READ,UNIT");  
	  if (writeDevice(command)) return 1; 
	  //UPG_sprintf(command,":UNIT:TEMP C");	
	  if(DEBUG) cout << "DEBUG_P2" << endl;  
  } 
  return 0;
}

void PixGPIBDevice::setState(DeviceStatus state)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setState" << endl;
    if (m_DeviceType != KEITHLEY_2410)
    {
        if(state!=PGD_ON && state!=PGD_OFF)  return;
        if(m_DeviceFunction == METER) return;
    }
    disarm();

    char command[120];

    switch(m_DeviceType)
    {
    case TTI_PL330DP:
    case TTI_PL330TP:
    case TTI_PL303QMD:
    case TTI_QL355TP:
        UPG_sprintf(command, (state==PGD_ON)?"OPALL 1":"OPALL 0");
        break;
    case AGILENT_3631A:
    case AGILENT_E3642A:
    case AGILENT_E3644A:
    case AGILENT_8110A:
    case AGILENT_33250A:
    case KEITHLEY_2410:
    case KEITHLEY_2400:
    case KEITHLEY_2000:
    case KEITHLEY_2001:
    case KEITHLEY_6517A:
        UPG_sprintf(command, (state==PGD_ON)?"OUTP ON":"OUTP OFF");
        break;
    case KEITHLEY_2602: //theresa
		UPG_sprintf(command, "smua.source.output=%d", (state==PGD_ON)?1:0);
	    if (writeDevice(command)) return;
		UPG_sprintf(command, "smub.source.output=%d", (state==PGD_ON)?1:0);
		break;
	case KEITHLEY_2000_SCANCARD:								//Should be meters anyway, but I add them here just in case.
    case KEITHLEY_2001_SCANCARD:
    case KEITHLEY_2700: //ADD
	case FLUKE_8842A:
    case HP_34401A:
    case HP_34970A:
    default:
        // do nothing
        return;
    }
    if (writeDevice(command)) return;

    m_Status = state;
}
void PixGPIBDevice::setVoltage(unsigned int channel, double voltage)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setVoltage" << endl;
	disarm();
    char command[120];
    if(channel>=m_DeviceNumberChannels) return;
    if(m_DeviceFunction == METER) return;;
    switch(m_DeviceType){
		case TTI_PL330DP:
		case TTI_PL330TP:
		case TTI_PL303QMD:
		case TTI_QL355TP:
			UPG_sprintf(command,"V%d %f",(channel+1), (float)voltage);
			break;
		case KEITHLEY_2410:
		case KEITHLEY_2400:
		case KEITHLEY_6517A:
			UPG_sprintf(command,":SOUR:VOLT %f",(float)voltage);
			break;
		case AGILENT_3631A:
			// select requested channel first
			UPG_sprintf(command,"INST:NSEL %d", channel+1);
			if (writeDevice(command)) return;
			// then send setting command
			UPG_sprintf(command,"VOLT %f", (float)voltage);
			break;
		case AGILENT_E3642A:
		case AGILENT_E3644A:
		case AGILENT_33250A:
			UPG_sprintf(command,":VOLT %f", (float)voltage);
			break;
		case AGILENT_8110A:{
			UPG_sprintf(command,":SOUR:HOLD VOLT");
			if (writeDevice(command)) return;
			UPG_sprintf(command,":SOUR:VOLT%d %f",channel+1, (float)voltage);
			break;
		}
		default:
			// do nothing
			return;
    }
    if (writeDevice(command)) return;
    arm();
}

void PixGPIBDevice::setOVPVoltage(unsigned int channel, bool OVPEnabled, double voltage)
{
    if(DEBUG) cout << "DEBUG PixGPIBDevice::setOVPVoltage" << endl;
    disarm();
    char command[120];
    if(channel>=m_DeviceNumberChannels) return;
    if(m_DeviceFunction == METER) return;;
    switch(m_DeviceType){
		case TTI_QL355TP:
			if (OVPEnabled)
				UPG_sprintf(command,"OVP%d %f",(channel+1), (float)voltage);
			else
				UPG_sprintf(command,"OVP%d %f",(channel+1), (float)1000);
			break;
		case KEITHLEY_2410:
		case KEITHLEY_2400:
		case KEITHLEY_6517A:
			if (OVPEnabled)
				UPG_sprintf(command,":SOUR:VOLT:PROT %f", (float)voltage);
			else
				UPG_sprintf(command,":SOUR:VOLT:PROT DEF");
			break;
		default:
			// do nothing
			return;
    }
    if (writeDevice(command)) return;
    arm();
}

void PixGPIBDevice::setOCPCurrent(unsigned int channel, bool OCPEnabled, double current)
{
    if(DEBUG) cout << "DEBUG PixGPIBDevice::setOCPCurrent" << endl;
    char command[120];
    if(channel>=m_DeviceNumberChannels) return;
    if(m_DeviceFunction == METER) return;;
    switch(m_DeviceType){
		case TTI_QL355TP:
			if (OCPEnabled)
				UPG_sprintf(command,"OCP%d %f", (channel+1), (float)current);
			else
				UPG_sprintf(command,"OCP%d %f", (channel+1), (float)1000);
			break;
		default:
			// do nothing
			return;
    }
    if (writeDevice(command)) return;
    arm();
}

void PixGPIBDevice::setRemoteSensing(unsigned int channel, bool RemoteSensingEnabled)
{
    if(DEBUG) cout << "DEBUG PixGPIBDevice::setRemoteSensing" << endl;
    disarm();
    char command[120];
    if(channel>=m_DeviceNumberChannels) return;
    if(m_DeviceFunction == METER) return;
    switch(m_DeviceType){
		case TTI_QL355TP:
			UPG_sprintf(command,"SENSE%d %d", (channel+1), (int)RemoteSensingEnabled);
			break;
		case KEITHLEY_2410:
		case KEITHLEY_2400:
		case KEITHLEY_6517A:
			UPG_sprintf(command,":SYST:RSEN %d", (int)RemoteSensingEnabled);
			break;
		default:
			// do nothing
			return;
    }
    if (writeDevice(command)) return;
    arm();
}

void PixGPIBDevice::setCurrentLimit(unsigned int channel, double current)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setCurrentLimit" << endl;
    disarm();
    char command[120];
    if(channel>=m_DeviceNumberChannels) return;
    if(m_DeviceFunction == METER) return;

    switch(m_DeviceType)
    {
    case TTI_PL330DP:
    case TTI_PL330TP:
    case TTI_PL303QMD:
    case TTI_QL355TP:
        UPG_sprintf(command,"I%d %f",(channel+1),(float)current);
        break;
    case KEITHLEY_2410:
    case KEITHLEY_2400:
    case KEITHLEY_6517A:
    {
        UPG_sprintf(command,":SENS:CURR:PROT %de-7",(int)(current*1e7+.01));
        break;
    }
    case AGILENT_3631A:
        // select requested channel first
        if(channel==0) UPG_sprintf(command,"INST:SEL P6V");
        if(channel==1) UPG_sprintf(command,"INST:SEL P25V");
        if(channel==2) UPG_sprintf(command,"INST:SEL N25V");
        if (writeDevice(command)) return;
        // then send setting command
        UPG_sprintf(command,"CURR %f", (float)current);
        break;
    case AGILENT_E3642A:
    case AGILENT_E3644A:
        UPG_sprintf(command,"CURR %f", (float)current);
        break;
    default:
        // do nothing
        return;
    }
    if(DEBUG) cout << "DEBUG   : setting current limit to " << current << " with command "<< command << endl;
    if (writeDevice(command)) return;
    arm();
}

////////////////////////
// measureVoltages()
////////////////////////
int PixGPIBDevice::measureVoltages(float inRange, bool inAutoRange, int pChannel)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice:::measureVoltages" << endl;
    disarm();
    int tStatus = 0;
    if (pChannel < 0)
    {
        for (unsigned int iChannel = 0; iChannel < m_DeviceNumberChannels; ++iChannel)  	// loop over all channels
        {
            if (measureVoltage(inRange, inAutoRange, iChannel) < 1) tStatus = -1;
        }
    }
    else tStatus = measureVoltage(inRange, inAutoRange, pChannel);						// just take one channel
	if(DEBUG) cout << "DEBUG PixGPIBDevice:::measureVoltages END" << endl;
    arm();
    return tStatus;
}

////////////////////////
// measureCurrents()
////////////////////////
int PixGPIBDevice::measureCurrents(float inRange, bool inAutoRange, int pChannel)
{
    disarm();
	if(DEBUG) cout << "DEBUG PixGPIBDevice:::measureCurrents" << endl;
    int tStatus = 0;
    if (pChannel < 0)
    {
        for (unsigned int iChannel = 0; iChannel < m_DeviceNumberChannels; ++iChannel)  	// loop over all channels
        {
            if (measureCurrent(inRange, inAutoRange, iChannel) < 1) tStatus = -1;
        }
    }
    else tStatus = measureCurrent(inRange, inAutoRange, pChannel);						// just take one channel
    arm();
    return tStatus;
}

////////////////////////
// measureVoltage()
////////////////////////
int PixGPIBDevice::measureVoltage(float inRange, bool inAutoRange, int pChannel)
{
	//cout << "Mdevice type" << m_DeviceType << endl;
  disarm();
  if(DEBUG) cout << "DEBUG PixGPIBDevice::measureVoltage" << endl;
  /*if (m_DeviceType == HP_34970A){
    // strip slot first and check if OK
    int slot = pChannel/100;
    int chan = pChannel%100; 
    if(!(slot>=0 && slot<3 && m_slotDevTypes[slot]!=UNKNOWN) || chan<1 || chan>(int)m_DeviceNumberChannels) // GENOVA SLOT IS 3
      return -1;
  }
  else if (pChannel < 0 || pChannel > (((int)m_DeviceNumberChannels)-1)) return -1;*/
    char command[120];
    // send voltage measurement request command
    if (m_DeviceType == AGILENT_3631A)
    {
        if (pChannel == 0) UPG_sprintf(command, "MEAS:VOLT? P6V");
        else if (pChannel == 1) UPG_sprintf(command, "MEAS:VOLT? P25V");
        else if (pChannel == 2) UPG_sprintf(command, "MEAS:VOLT? N25V");
    }
    else if (m_DeviceType == AGILENT_E3642A)
    {
        UPG_sprintf(command, "MEAS:VOLT?");
    }
    else if (m_DeviceType == AGILENT_E3644A)
    {
        UPG_sprintf(command, "MEAS:VOLT?");
    }
    else if(m_DeviceType == AGILENT_34410A)
    {
        if(m_meterReadType!=0 || inRange!=m_range || inAutoRange!=m_autoRange)
        {
            m_range = inRange;
            m_autoRange = inAutoRange;
            if(m_autoRange)
                UPG_sprintf(command, "CONF:VOLT:DC AUTO");
            else
                UPG_sprintf(command, "CONF:VOLT:DC %.0e",m_range);
            if (writeDevice(command)) return 1;
            m_meterReadType = 0;
        }
        if (pChannel == 0) UPG_sprintf(command, "READ?");
    }
    else if (m_DeviceType == AGILENT_364xA1CH)
    {
        if (pChannel == 0) UPG_sprintf(command, "MEAS:VOLT?");
    }
    else if (m_DeviceType == AGILENT_364xA2CH)
    {
        if (pChannel == 0)
        {
            UPG_sprintf(command, "INST:SEL OUT1");
            if (writeDevice(command)) return 1;
            UPG_sprintf(command, "MEAS:VOLT?");
        }
        else if (pChannel == 1)
        {
            UPG_sprintf(command, "INST:SEL OUT2");
            if (writeDevice(command)) return 1;
            UPG_sprintf(command, "MEAS:VOLT?");
        }
    }
    else if (m_DeviceType == TTI_PL330DP || m_DeviceType == TTI_PL303QMD || m_DeviceType == TTI_PL330TP || m_DeviceType == TTI_QL355TP)
    {
        UPG_sprintf(command, "V%dO?", (pChannel+1));
    }
    else if (m_DeviceType == KEITHLEY_237)
    {
        if (pChannel == 0) UPG_sprintf(command, "G1,2,0H0X");
    }
    else if (m_DeviceType == KEITHLEY_487)
    {
        if (pChannel == 0) UPG_sprintf(command, "U8X");
    }
    else if (m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400 || m_DeviceType == KEITHLEY_6517A ||m_DeviceType == KEITHLEY_6517B) 
    {
        if (pChannel == 0)
        {
            if(m_DeviceFunction == METER)
            {
                if(m_meterReadType!=0 || inRange!=m_range || inAutoRange!=m_autoRange)
                {
                    m_range = inRange;
                    m_autoRange = inAutoRange;
                    UPG_sprintf(command, "OUTP OFF");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SOUR:FUNC CURR");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SOUR:CURR 0");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:FUNC \"VOLT\"");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:VOLT:PROT %.0e",2.f*inRange);
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:VOLT:RANG %.0e",inRange);
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, "OUTP ON");
                    if (writeDevice(command)) return 1;
                    m_meterReadType = 0;
                }
                UPG_sprintf(command, ":READ?");
            }
            else
                UPG_sprintf(command, ":SOUR:VOLT:LEV:IMM:AMPL?");
        }
    }
    else if (m_DeviceType == KEITHLEY_2000 || m_DeviceType == KEITHLEY_2001)
    {
        if (pChannel == 0 && m_DeviceFunction == METER)
        {
            if(m_meterReadType!=0 || inRange!=m_range || inAutoRange!=m_autoRange)
            {
                UPG_sprintf(command, ":SENS:FUNC 'VOLT:DC'");
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, ":SENS:VOLT:RANG %.0e",inRange);
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, ":SENS:VOLT:RANG:AUTO %s", inAutoRange?"ON":"OFF");
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, ":READ?");
                m_meterReadType = 0;
            }
        }
    }

	////Keithley 2602
    else if (m_DeviceType == KEITHLEY_2602)
    {
            if(m_DeviceFunction == METER)
            {

					UPG_sprintf(command, "smua.source.func=smua.OUTPUT_DCAMPS");
                    if (writeDevice(command)) return 1;
					UPG_sprintf(command, "smub.source.func=smub.OUTPUT_DCAMPS");
                    if (writeDevice(command)) return 1;
					UPG_sprintf(command, "smua.source.output=smua.OUTPUT_ON");
                    if (writeDevice(command)) return 1;
					UPG_sprintf(command, "smub.source.output=smub.OUTPUT_ON");
                    if (writeDevice(command)) return 1;

/*
				if(inRange!=m_range)
                {
                    UPG_sprintf(command, "smua.measure.rangev = 1", 2.f*inRange);
                    ibwrt(m_Device, command, strlen(command));
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:CURR:RANG %.0e",inRange);
                    if (writeDevice(command)) return 1;
                    m_range = inRange;
                }
				
                if(inAutoRange!=m_autoRange)
                {
                    UPG_sprintf(command, ":SENS:CURR:RANG:AUTO %s", inAutoRange?"ON":"OFF");
                    if (writeDevice(command)) return 1;
                    m_autoRange = inAutoRange;
                }
                */
				if(m_meterReadType!=1)
                {
                    UPG_sprintf(command, "smua.source.output = 0");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, "smub.source.output = 0");
                    if (writeDevice(command)) return 1;
//                    UPG_sprintf(command, ":SOUR:FUNC VOLT");
  //                  ibwrt(m_Device, command, strlen(command));
    //                if (writeDevice(command)) return 1;
      //              UPG_sprintf(command, ":SOUR:VOLT 0");
        //            ibwrt(m_Device, command, strlen(command));
          //          if (writeDevice(command)) return 1;
                  //  UPG_sprintf(command, "smua.measure.v()");
              //      if (writeDevice(command)) return 1;
                    m_meterReadType = 1;
                }
            }
			UPG_sprintf(command, "smu%c.nvbuffer1.clear()",(pChannel == 0)?'a':'b');   // if pchannel is 0 then channel a is adressed otherwise channel b
			if (writeDevice(command)) return 1;
            UPG_sprintf(command, "smu%c.measure.v(smu%c.nvbuffer1)",(pChannel == 0)?'a':'b',(pChannel == 0)?'a':'b');
			if (writeDevice(command)) return 1;
            UPG_sprintf(command, "printbuffer(1,1,smu%c.nvbuffer1)",(pChannel == 0)?'a':'b');
        
    }



////Keithley 2700
	else if(m_DeviceType == KEITHLEY_2700)
	{

		if(inAutoRange){
			UPG_sprintf(command, ":SENS:VOLT:DC:RANG:AUTO %s", inAutoRange?"ON":"OFF");
            if (writeDevice(command)) return 1;
			
		}else{
		UPG_sprintf(command, ":SENS:VOLT:DC:RANG %.0e",inRange);
		if (writeDevice(command)) return 1;
		}
	
		///cout<<"LETTURA"<<endl;
		int canale;
		canale=pChannel+100;
		UPG_sprintf(command, "ROUT:CLOS (@%d)", canale); //connect channel to multimeter.
        if (writeDevice(command)) return 1;
		UPG_sprintf(command, ":READ?");
			
	}

    else if (m_DeviceType == KEITHLEY_2000_SCANCARD || m_DeviceType == KEITHLEY_2001_SCANCARD)
    {
        if (m_DeviceFunction == METER)
        {
            if(inRange!=m_range)
            {
                UPG_sprintf(command, ":SENS:VOLT:RANG %.0e",inRange);
                if (writeDevice(command)) return 1;
                m_range = inRange;
            }
            if(inAutoRange!=m_autoRange)
            {
                UPG_sprintf(command, ":SENS:VOLT:RANG:AUTO %s", inAutoRange?"ON":"OFF");
                if (writeDevice(command)) return 1;
                m_autoRange = inAutoRange;
            }
            if(m_meterReadType!=0)
            {
                UPG_sprintf(command, ":SENS:FUNC 'VOLT:DC'");
                if (writeDevice(command)) return 1;
                m_meterReadType = 0;
            }
            if((m_activeChannel != pChannel) && (m_DeviceType != KEITHLEY_2000_SCANCARD))
            {
                char tChannelList[30] = "@";
                for (int ichannel = 0; ichannel<(int)m_DeviceNumberChannels; ++ichannel) 		//open all channels but...
                {
                    char tChannel[2];
                    if (ichannel == pChannel) continue;							//...don't open the one to measure
                    UPG_sprintf(tChannel, "%d", ichannel+1);
                    UPG_strcat(tChannelList,tChannel);										//Add channel to list
                    if (ichannel!=(((int)m_DeviceNumberChannels)-1) && 
			!(pChannel == (((int)m_DeviceNumberChannels)-1) && ichannel == (((int)m_DeviceNumberChannels)-2))) 
		      UPG_strcat(tChannelList,",");		//add the channels to open to the list
                }
                if (m_DeviceType == KEITHLEY_2000_SCANCARD)
                    UPG_sprintf(command, "ROUT:MULT:OPEN (");
                else
                    UPG_sprintf(command, "ROUT:OPEN (");
                UPG_strcat(command,tChannelList);
                UPG_strcat(command,")");
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, "ROUT:CLOSE:STAT?");//check if relay of chosen channel is closed
                char readback[120];
                if (writeDevice(command)) return 1;
                if (readDevice(readback)) return 1;
                if (strncmp(readback ,"(@1)", 4) != 0)
                {
                    UPG_sprintf(command, ":ROUT:CLOSE (@%d)", pChannel+1);//connect channel = [1:10] to the multimeter
                    if (writeDevice(command)) return 1;
                }
            }
            else if (m_DeviceType == KEITHLEY_2000_SCANCARD)
            {
                char readback[120];

                UPG_sprintf(command, "ROUT:CLOSE:STAT?");//check if relay of chosen channel is closed
                if (writeDevice(command)) return 1;
                if (readDevice(readback)) return 1;
                if ((atoi(&readback[2]) == 0) || (atoi(&readback[2]) != pChannel+1))//If the channel is already open, or if no valid conversion for atoi (such as when you have no channels open)
                {
                    UPG_sprintf(command, "ROUT:CLOS (@%d)", pChannel+1); //connect channel to multimeter.
                    if (writeDevice(command)) return 1;
                }
            }
            UPG_sprintf(command, ":READ?");
        }
    }
    else if (m_DeviceType == HP_34401A)
    {
        UPG_sprintf(command,"MEAS:VOLT:DC?");
    }
   
/*	else if (m_DeviceType == HP_34970A)
    {
      if(inAutoRange)
	UPG_sprintf(command,"MEAS:VOLT:DC?AUTO,DEF,(@%d)",pChannel); It doesn't work good
      else
	UPG_sprintf(command,"MEAS:VOLT:DC?%dMAX,DEF,(@%d)",(int)inRange, pChannel);
    }*/

	else if (m_DeviceType == HP_34970A){
		UPG_sprintf(command, "DISP:TEXT 'LV MEAS DEV'");
		if (writeDevice(command)) return 1;
	int canale;
	canale=pChannel+300;
	if(inAutoRange){
	UPG_sprintf(command,"CONF:VOLT:DC AUTO,DEF, (@%d)",canale);
	if (writeDevice(command)) return 1;
	}
	else{
	UPG_sprintf(command,"CONF:VOLT:DC %dMAX,DEF, (@%d)",(int)inRange, canale);
	if (writeDevice(command)) return 1;
	}
	UPG_sprintf(command, "READ?");

}


    else if (m_DeviceType == AGILENT_33250A)
	{
		UPG_sprintf(command,":VOLT?");
	}
    else if (m_DeviceType == FLUKE_8842A)
    {
        UPG_sprintf(command, "F1 ?");			//Tells the fluke to switch to VDC, and take a measurement.
    }
    else if (m_DeviceType == AGILENT_8110A)
	{
		UPG_sprintf(command, ":VOLT?");			//Tells the fluke to switch to VDC, and take a measurement.
	}
    else
    	return 0;								//device type not supported for voltage	 reading

    if (strlen(command) == 0)
    {
        m_Currents[pChannel] = 0.0;
        return 0;
    }
    if (writeDevice(command)) return 1; //sends read command

    // receive voltage measurement answer and set m_Voltages
    char answer[120];
    if (readDevice(answer)) return 1;
    answer[ibcntl] = '\0';
    if(DEBUG) cout << "DEBUG   : measureVoltages, Ch: " << pChannel << " - answer: " << answer;
    if (m_DeviceType == KEITHLEY_487) m_Voltages[pChannel] = (float)atof(&answer[4]);
    else m_Voltages[pChannel] = (float)atof(answer);

    if ((m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400 || m_DeviceType == KEITHLEY_6517A || m_DeviceType == KEITHLEY_6517B) && m_DeviceFunction != METER)
    {
        UPG_sprintf(command, "*CLS");
        if (writeDevice(command)) return 1;
    }
    else if (m_DeviceType == KEITHLEY_237)    // only that it shows the current at the instrument
    {
        UPG_sprintf(command, "G4,2,0H0X");
        if (writeDevice(command)) return 1;
    }
    arm();
    return 0;
}

////////////////////////
// getVoltage()
////////////////////////
float PixGPIBDevice::getVoltage(unsigned int pChannel)
{
	if(DEBUG) cout << "get voltage   ---------------" << endl;
    if(DEBUG) cout << "pChannel = " << pChannel << endl;
	if(DEBUG) cout << "m_Voltages[pChannel] = " << m_Voltages[pChannel] << endl;
    return m_Voltages[pChannel];
}


////////////////////////
// measureCurrent()
////////////////////////
int PixGPIBDevice::measureCurrent(float inRange, bool inAutoRange, int pChannel)
{
  if(DEBUG) cout << "DEBUG PixGPIBDevice::measureCurrent" << endl;
  disarm();
  /*if (m_DeviceType == HP_34970A){
    // strip slot first and check if OK
    int slot = pChannel/100;
    int chan = pChannel%100;
    if(!(slot>=0 && slot<3 && m_slotDevTypes[slot]!=UNKNOWN) || chan<1 || chan>(int)m_DeviceNumberChannels)
      return -1;
  }
  else if (pChannel < 0 || pChannel > (((int)m_DeviceNumberChannels)-1)) return -1;*/

  if (m_Status == PGD_OFF)
    {
        if(DEBUG) cout << "DEBUG   : device is off, printing default current of 0 " << endl;
        for (int pChannel = 0; pChannel < 3; pChannel++)
        {
            m_Currents[pChannel] = 0.0;
        }
        return 1;
    }
    if(DEBUG) cout << "DEBUG   : checking current of channel "  << pChannel << endl;

    // send current measurement request command
    char command[120];
    if (m_DeviceType == AGILENT_3631A)
    {
        if (pChannel == 0) UPG_sprintf(command, "MEAS:CURR? P6V");
        else if (pChannel == 1) UPG_sprintf(command, "MEAS:CURR? P25V");
        else if (pChannel == 2) UPG_sprintf(command, "MEAS:CURR? N25V");
    }
    else if (m_DeviceType == AGILENT_E3642A)
    {
        UPG_sprintf(command, "MEAS:CURR?");
    }
    else if (m_DeviceType == AGILENT_E3644A)
    {
        UPG_sprintf(command, "MEAS:CURR?");
    }
    else if(m_DeviceType == AGILENT_34410A)
    {
        if(m_meterReadType!=1 || inRange!=m_range || inAutoRange!=m_autoRange)
        {
            m_range = inRange;
            m_autoRange = inAutoRange;
            if(m_autoRange)
                UPG_sprintf(command, "CONF:CURR:DC AUTO");
            else
                UPG_sprintf(command, "CONF:CURR:DC %.0e",m_range);
            if (writeDevice(command)) return 1;
            m_meterReadType = 1;
        }
        if (pChannel == 0) UPG_sprintf(command, "READ?");
    }
    else if (m_DeviceType == AGILENT_364xA1CH)
    {
        if (pChannel == 0) UPG_sprintf(command, "MEAS:CURR?");
    }
    else if (m_DeviceType == AGILENT_364xA2CH)
    {
        if (pChannel == 0)
        {
            UPG_sprintf(command, "INST:SEL OUT1");
            if (writeDevice(command)) return 1;
            UPG_sprintf(command, "MEAS:CURR?");
        }
        else if (pChannel == 1)
        {
            UPG_sprintf(command, "INST:SEL OUT2");
            if (writeDevice(command)) return 1;
            UPG_sprintf(command, "MEAS:CURR?");
        }
    }
    else if (m_DeviceType == TTI_PL330DP || m_DeviceType == TTI_PL303QMD || m_DeviceType == TTI_PL330TP || m_DeviceType == TTI_QL355TP)
    {
        UPG_sprintf(command, "I%dO?", (pChannel+1));
    }
    else if (m_DeviceType == KEITHLEY_237)
    {
        if (pChannel == 0) UPG_sprintf(command, "G4,2,0H0X");
    }
    else if (m_DeviceType == KEITHLEY_487)
    {
        if (pChannel == 0) UPG_sprintf(command, "G0B0X");
    }
    else if (m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400 || m_DeviceType == KEITHLEY_6517A|| m_DeviceType == KEITHLEY_6517B)
    {
        if (pChannel == 0)
        {
            if(m_DeviceFunction == METER)
            {
                if(inRange!=m_range)
                {
                    UPG_sprintf(command, ":SENS:CURR:PROT %.0e", 2.f*inRange);
                    ibwrt(m_Device, command, strlen(command));
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:CURR:RANG %.0e",inRange);
                    if (writeDevice(command)) return 1;
                    m_range = inRange;
                }
                if(inAutoRange!=m_autoRange)
                {
                    UPG_sprintf(command, ":SENS:CURR:RANG:AUTO %s", inAutoRange?"ON":"OFF");
                    if (writeDevice(command)) return 1;
                    m_autoRange = inAutoRange;
                }
                if(m_meterReadType!=1)
                {
                    UPG_sprintf(command, "OUTP OFF");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SOUR:FUNC VOLT");
                    ibwrt(m_Device, command, strlen(command));
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SOUR:VOLT 0");
                    ibwrt(m_Device, command, strlen(command));
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:FUNC \"CURR\"");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, "OUTP ON");
                    if (writeDevice(command)) return 1;
                    m_meterReadType = 1;
                }
            }
            UPG_sprintf(command, ":READ?");
        }
    }
    else if (m_DeviceType == KEITHLEY_2000 || m_DeviceType == KEITHLEY_2001)
    {
        if (pChannel == 0 && m_DeviceFunction == METER)
        {
            if(m_meterReadType != 0 || inRange != m_range || inAutoRange != m_autoRange)
            {
                m_range = inRange;
                m_autoRange = inAutoRange;
                UPG_sprintf(command, ":SENS:FUNC 'CURR:DC'");
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, ":SENS:CURR:RANG %.0e",inRange);
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, ":SENS:CURR:RANG:AUTO %s", inAutoRange?"ON":"OFF");
                if (writeDevice(command)) return 1;
                UPG_sprintf(command, ":READ?");
                m_meterReadType = 1;
            }
        }
    }
    else if (m_DeviceType == KEITHLEY_2000_SCANCARD || m_DeviceType == KEITHLEY_2001_SCANCARD|| m_DeviceType == KEITHLEY_2700)
    {
        if (m_DeviceFunction == METER)
        {
            UPG_sprintf(command, ":ROUT:OPEN:ALL");	//disconnected all channels from the multimeter
            if (writeDevice(command)) return 1;
            //not supported, only via voltage drop over a resistor the current can be measured with a Keithley 200X + scan card
        }
    }
    else if (m_DeviceType == KEITHLEY_2602)
    {
        
            if(m_DeviceFunction == METER)
            {
       
/*
				if(inRange!=m_range)
                {
                    UPG_sprintf(command, "smua.measure.rangev = 1", 2.f*inRange);
                    ibwrt(m_Device, command, strlen(command));
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, ":SENS:CURR:RANG %.0e",inRange);
                    if (writeDevice(command)) return 1;
                    m_range = inRange;
                }
				
                if(inAutoRange!=m_autoRange)
                {
                    UPG_sprintf(command, ":SENS:CURR:RANG:AUTO %s", inAutoRange?"ON":"OFF");
                    if (writeDevice(command)) return 1;
                    m_autoRange = inAutoRange;
                }
                */
				if(m_meterReadType!=1)
                {
                    UPG_sprintf(command, "smua.source.output = 0");
                    if (writeDevice(command)) return 1;
                    UPG_sprintf(command, "smub.source.output = 0");
                    if (writeDevice(command)) return 1;
//                    UPG_sprintf(command, ":SOUR:FUNC VOLT");
  //                  ibwrt(m_Device, command, strlen(command));
    //                if (writeDevice(command)) return 1;
      //              UPG_sprintf(command, ":SOUR:VOLT 0");
        //            ibwrt(m_Device, command, strlen(command));
          //          if (writeDevice(command)) return 1;
                  //  UPG_sprintf(command, "smua.measure.v()");
              //      if (writeDevice(command)) return 1;
                    m_meterReadType = 1;
                }
            }
			UPG_sprintf(command, "smu%c.nvbuffer2.clear()",(pChannel == 0)?'a':'b');
			if (writeDevice(command)) return 1;
            UPG_sprintf(command, "smu%c.measure.i(smu%c.nvbuffer2)",(pChannel == 0)?'a':'b',(pChannel == 0)?'a':'b');
			if (writeDevice(command)) return 1;
            UPG_sprintf(command, "printbuffer(1,1,smu%c.nvbuffer2)",(pChannel == 0)?'a':'b');
        
    }
    else if (m_DeviceType == HP_34401A)
    {
        UPG_sprintf(command, "MEAS:CURR:DC?");
    }
    else if (m_DeviceType == HP_34970A)
    {
      // strip slot first and check if OK
      int slot = pChannel/100; //not implemented in Genova Device
      if(slot>=0 && slot<3 && m_slotDevTypes[slot]!=UNKNOWN){
	if(inAutoRange)
	  UPG_sprintf(command,"MEAS:CURR:DC?AUTO,DEF,(@%d)",pChannel);
	else
	  UPG_sprintf(command,"MEAS:CURR:DC?%dMAX,DEF,(@%d)",(int)inRange, pChannel);
      } else{
	char errmsg[1000];
	UPG_sprintf(errmsg, "Requested channel (%d) doesn't fit meter settings", pChannel);
	m_PixGPIBError->printGPIBError(errmsg);
      }
    }
    else if (m_DeviceType == FLUKE_8842A)
    {
        UPG_sprintf(command, "F5 ?");				//Tells the Fluke to switch to mADC mode and take a measurement.
    }
    else if (m_DeviceType == AGILENT_8110A){
		UPG_sprintf(command, ":CURR?");
	}
    else
        return 0;								//device type not supported for current reading
    if (strlen(command) == 0)
    {
        m_Currents[pChannel] = 0.0;
        return 0;
    }
    if (writeDevice(command)) return 1; //sends read command

    // receive current measurement answer and set m_Currents
    char answer[120];
    if (readDevice(answer)) return 1;
    answer[ibcntl] = '\0';
    if(DEBUG) cout << "DEBUG   : measureCurrents, Ch: " << pChannel << " - answer: " << answer;
    if (m_DeviceType == KEITHLEY_487) m_Currents[pChannel] = (float)atof(&answer[4]);
    else if (m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400 || m_DeviceType == KEITHLEY_6517A || m_DeviceType == KEITHLEY_6517B || m_DeviceType == KEITHLEY_2000 || m_DeviceType == KEITHLEY_2000_SCANCARD || m_DeviceType == KEITHLEY_2001 || m_DeviceType == KEITHLEY_2700)
    {
        // extract second part of comma-separated list of measurements
        string sanswer = answer;
        int pos = (int)sanswer.find(",");
        if(pos!=(int)string::npos) sanswer.erase(0, pos+1);
        pos = (int)sanswer.find(",");
        if(pos!=(int)string::npos) sanswer.erase(pos, sanswer.length()-pos);
        m_Currents[pChannel] = (float)atof(sanswer.c_str());
    }
    else m_Currents[pChannel] = (float)atof(answer);

    if ((m_DeviceType == KEITHLEY_2410 || m_DeviceType == KEITHLEY_2400) && m_DeviceFunction != METER)
    {
        UPG_sprintf(command, "*CLS");
        if (writeDevice(command)) return 1;
    }
    arm();
    return 0;
}

////////////////////////
// getCurrent()
////////////////////////
float PixGPIBDevice::getCurrent(unsigned int pChannel)
{
    return m_Currents[pChannel];
}

void PixGPIBDevice::setNburstPulses(unsigned int pChannel, unsigned int pNburstPulses)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setNburstPulses" << endl;
	if(m_DeviceFunction != PULSER || pChannel != 0 || pNburstPulses < 1)	//so far only one channel supported
		return;
	char command[120];
	if (m_DeviceType == AGILENT_8110A)
		UPG_sprintf(command, ":TRIG:COUNT %d", pNburstPulses);
	if (m_DeviceType == AGILENT_33250A)
		UPG_sprintf(command, ":BURS:NCYC %d", pNburstPulses);
	writeDevice(command);
}

void PixGPIBDevice::setFrequency(unsigned int pChannel, float pFrequency)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setFrequency" << endl;
	if(m_DeviceFunction != PULSER || pChannel != 0)//so far only one channel supported
		return;
	char command[120];
	if (m_DeviceType == AGILENT_8110A || m_DeviceType == AGILENT_33250A)
		UPG_sprintf(command, ":FREQ %.1f", pFrequency);
	writeDevice(command);
}

void PixGPIBDevice::setVoltageOffset(unsigned int pChannel, float pVoltageOffset)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setVoltageOffset" << endl;
	if(m_DeviceFunction != PULSER || pChannel != 0) //so far only one channel supported
		return;
	char command[120];
	if (m_DeviceType == AGILENT_33250A || m_DeviceType == AGILENT_8110A){
		UPG_sprintf(command, ":VOLT:OFFS %.1f", pVoltageOffset);
		writeDevice(command);
	}
}

void PixGPIBDevice::setWaveForm(unsigned int pChannel, unsigned int pWaveForm)
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::setWaveForm" << endl;
	if(m_DeviceFunction != PULSER || pChannel != 0) //so far only one channel supported
		return;
	char command[120];
	if (m_DeviceType == AGILENT_33250A){
		switch(pWaveForm){
			case 0:
				UPG_sprintf(command, "FUNC PULS");
				break;
			case 1:
				UPG_sprintf(command, "FUNC SIN");
				break;
			case 2:
				UPG_sprintf(command, "FUNC SQU");
				break;
			case 3:
				UPG_sprintf(command, "FUNC RAMP");
				break;
			case 4:
				UPG_sprintf(command, "FUNC NOIS");
				break;
		}
		writeDevice(command);
	}
}

void PixGPIBDevice::sendTrigger()
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::sendTrigger()" << endl;
	if(m_DeviceFunction != PULSER)
		return;
	char command[120];
	UPG_sprintf(command, "*TRG");
	writeDevice(command);
}

void PixGPIBDevice::resetDevice()
{
	if(DEBUG) cout << "DEBUG PixGPIBDevice::resetDevice()" << endl;
    ibclr(m_Device);
    if (ibsta & ERR) m_PixGPIBError->printGPIBError("* ibclr Error");

    char command[120];

//     switch(m_DeviceType)
//     {
//     default:
        UPG_sprintf(command,"*RST");
//     }
    if (writeDevice(command)) return;
    initializeDevice();
}

bool PixGPIBDevice::writeDevice(char (&command)[120])
{
    if (DEBUG) cout<<"command: "<<command<<endl;
    ibwrt(m_Device, command, strlen(command));
    if (ibsta & ERR)
    {
        m_PixGPIBError->printGPIBError("* ibwrt Error");
        m_Status = PGD_ERROR;
        return 1;
    }
    return 0;
}

bool PixGPIBDevice::readDevice(char (&answer)[120])
{
    ibrd(m_Device, answer, 120);
    if (ibsta & ERR)
    {
        m_PixGPIBError->printGPIBError("* ibrd Error");
        return 1;
    }
    return 0;
}
  
void PixGPIBDevice::disarm()
{
	char command[120];
	switch(m_DeviceType)
	{
		case KEITHLEY_2410:
		case KEITHLEY_2400:
			if (m_DeviceFunction != METER) 
			{
				UPG_sprintf(command, ":ABOR");
				if (writeDevice(command)) return;
				UPG_sprintf(command, ":ARM:COUNT 1");
				if (writeDevice(command)) return;
				break;
			}
		default:
			// do nothing
			return;
	}
}

void PixGPIBDevice::arm()
{
	disarm();
	char command[120];
	switch(m_DeviceType)
	{
		case KEITHLEY_2410:
		case KEITHLEY_2400:
			if (m_Status == PGD_ON && m_DeviceFunction != METER) 
			{
				UPG_sprintf(command, ":ARM:COUNT INF");
				if (writeDevice(command)) return;
				UPG_sprintf(command, ":INIT");
				if (writeDevice(command)) return;
				break;
			}
		default:
			// do nothing
			return;
	}
}
