///////////////////////////////////////////////////////////////////
// PixGPIB.cxx
//   Implementation file for class PixGPIB
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//  Version 00-00-01 21/11/2008 Daniel Dobos
///////////////////////////////////////////////////////////////////

#include "PixGPIB.h"
#include "PixGPIBError.h"
#include "PixGPIBDevice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

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
PixGPIB::PixGPIB()
{
  m_NumbDevices = 0;

  cout << "INFO    : scanning interfaces GPIB0-GPIB9" << endl;
  // scan all GPIB interfaces
  for (int board = 0; board <= 9; board++) {
    if (openBoard(board) == 0) {
      scanBoard(board);
    }
  }
  //printDevices();
}

PixGPIB::PixGPIB(unsigned int board)
{
  m_NumbDevices = 0;

  cout << "INFO    : scanning interfaces GPIB " << board << endl;  // what is board
  // scan single GPIB interfaces
  if (openBoard(board) == 0) {
    if (scanBoard(board) == 0) {
//      printDevices();
    }
  }
}

////////////////////////
// destructor
////////////////////////
PixGPIB::~PixGPIB()
{
  for (unsigned int i = 0; i < m_NumbDevices; i++)
    delete m_Devices[i];
}

////////////////////////
// openBoard()
////////////////////////
int PixGPIB::openBoard(unsigned int board)
{
  // set board as Controller-In-Charge
  SendIFC(board);
  if (ibsta & ERR) {
    if (iberr == EDVR) {
      if (DEBUG) cout << "WARNING : No driver found for interface GPIB" << board << endl;
    } else {
      char msg[100];
      UPG_sprintf(msg, "* SendIFC error");
      printGPIBError(msg);
    }
    return 1;
  }
  return 0;
}

////////////////////////
// closeBoard()
////////////////////////
int PixGPIB::closeBoard(unsigned int board)
{
  ibonl(board,0);  // take board offline
  if (ibsta & ERR) {
    char msg[100];
    UPG_sprintf(msg, "* ibonl Error");
    printGPIBError(msg);
    return 1;
  }
  return 0;
}

////////////////////////
// scanBoard()
////////////////////////
int PixGPIB::scanBoard(unsigned int board)
{

  Addr4882_t addresses[32];
  Addr4882_t instruments[31];

  // create addresses array
  for (int i = 0; i < 30; i++) {
    addresses[i] = (Addr4882_t)(i + 1);
  }
  addresses[30] = NOADDR;

  // find listeners and fill instruments array
  FindLstn(board, addresses, instruments , 31);
  int numbInstruments = ibcntl;
  if (numbInstruments == 0) {
    if (DEBUG) cout << "WARNING : No instruments found on interface GPIB" << board << endl;
    return 1;
  }
  if (ibsta & ERR) {
    if (iberr == 0) {
      cout << "WARNING : No driver found for interface GPIB" << board << endl;
    } else {
      char msg[100];
      UPG_sprintf(msg, "* FindLstn error:");
      printGPIBError(msg);
    }
    return 1;
  }
  instruments[numbInstruments] = NOADDR;

  // why should board be offline??? won't work with linux-gpib - instrument loop still OK though!
//   ibonl(board,0);  // take board offline
//   if (ibsta & ERR) {
//     printGPIBError("* ibonl Error");
//     return 1;
//   }

  // loop over instruments array
  for (int i = 0; i < numbInstruments; i++) {
    unsigned int PAD = GetPAD(instruments[i]);
    unsigned int SAD = GetSAD(instruments[i]);
    if(DEBUG) cout << "DEBUG   : starting device PAD " << PAD << " and SAD " << SAD << endl;
    m_Devices[m_NumbDevices] = new PixGPIBDevice(board,PAD,SAD,false,true);
    if(m_Devices[m_NumbDevices]->getStatus()==PGD_ERROR){
      m_Devices[m_NumbDevices]->resetDevice();
    }
    m_NumbDevices++;
  }
  return 0;
}

PixGPIBDevice* PixGPIB::getDevice(unsigned int pDeviceIndex) //for debugging purpose only
{
	if(pDeviceIndex > m_NumbDevices -1)
		return 0;
	return m_Devices[m_NumbDevices-1];
}

////////////////////////
// printDevices()
////////////////////////
int PixGPIB::printDevices()
{
  if (m_NumbDevices == 0) {
    cout << "WARNING : No instruments found!" << endl;
    return 1;
  } else {
    // print m_Instruments array
    cout << "--- RESULTS OF GPIB SCAN ---" << endl;
    cout << "Board:\tPAD:\tSAD:\tDevice Type:\tFunction:\tStatus:\tV0[V]:\tI0[I]:\tV1[V]:\tI1[I]:\tV2[V]:\tI2[I]:" << endl;
    for (unsigned int i = 0; i < m_NumbDevices; i++) {

      // get device type and transfer it to a string
      string deviceTypeDescription;
      if (m_Devices[i]->getDeviceType() == AGILENT_3631A) deviceTypeDescription = "AGILENT_3631A";
      else if (m_Devices[i]->getDeviceType() == FLUKE_8842A) deviceTypeDescription = "FLUKE_8842A";
	  else if (m_Devices[i]->getDeviceType() == HP_34401A) deviceTypeDescription = "HP_34401A";
	  else if (m_Devices[i]->getDeviceType() == HP_34970A) deviceTypeDescription = "HP_34970A";//Genova LV Device
	  else if (m_Devices[i]->getDeviceType() == AGILENT_E3642A) deviceTypeDescription = "AGILENT_E3642A";
	  else if (m_Devices[i]->getDeviceType() == AGILENT_E3644A) deviceTypeDescription = "AGILENT_E3644A";
      else if (m_Devices[i]->getDeviceType() == AGILENT_364xA1CH) deviceTypeDescription = "AGILENT_364xA1CH";
      else if (m_Devices[i]->getDeviceType() == AGILENT_364xA2CH) deviceTypeDescription = "AGILENT_364xA2CH";
      else if (m_Devices[i]->getDeviceType() == TTI_PL330DP) deviceTypeDescription = "TTI_PL330DP";
      else if (m_Devices[i]->getDeviceType() == TTI_PL330TP) deviceTypeDescription = "TTI_PL330TP";
      else if (m_Devices[i]->getDeviceType() == TTI_PL303QMD) deviceTypeDescription = "TTI_PL303QMD";
      else if (m_Devices[i]->getDeviceType() == TTI_QL355TP) deviceTypeDescription = "TTI_QL355TP";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_237) deviceTypeDescription = "KEITHLEY_237";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_487) deviceTypeDescription = "KEITHLEY_487";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_2000) deviceTypeDescription = "KEITHLEY_2000";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_2001) deviceTypeDescription = "KEITHLEY_2001";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_2000_SCANCARD) deviceTypeDescription = "KEITHLEY_2000 + SCAN CARD";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_2001_SCANCARD) deviceTypeDescription = "KEITHLEY_2001 + SCAN CARD";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_2410) deviceTypeDescription = "KEITHLEY_2410";
      else if (m_Devices[i]->getDeviceType() == KEITHLEY_2400) deviceTypeDescription = "KEITHLEY_2400";
	  else if (m_Devices[i]->getDeviceType() == KEITHLEY_2602) deviceTypeDescription = "KEITHLEY_2602"; //theresa
	  else if (m_Devices[i]->getDeviceType() == KEITHLEY_2700) deviceTypeDescription = "KEITHLEY_2700";//Genova LV Device
	  else if (m_Devices[i]->getDeviceType() == KEITHLEY_6517A) deviceTypeDescription = "KEITHLEY_6517A";
	  else if (m_Devices[i]->getDeviceType() == KEITHLEY_6517B) deviceTypeDescription = "KEITHLEY_6517B";
      else if (m_Devices[i]->getDeviceType() == AGILENT_34410A) deviceTypeDescription = "AGILENT_34410A";
      else if (m_Devices[i]->getDeviceType() == AGILENT_8110A) deviceTypeDescription = "AGILENT_8110A";
      else if (m_Devices[i]->getDeviceType() == AGILENT_33250A) deviceTypeDescription = "AGILENT_33250A";
      else deviceTypeDescription = "UNKNOWN  ";

      // get device function and transfer it to a string
      string deviceFunctionDescription;
      if (m_Devices[i]->getDeviceFunction() == LV_TPCC) deviceFunctionDescription = "LV_TPCC";
      else if (m_Devices[i]->getDeviceFunction() == LV_MODULE) deviceFunctionDescription = "LV_MODULE";
      else if (m_Devices[i]->getDeviceFunction() == HV) deviceFunctionDescription = "HV       ";
      else if (m_Devices[i]->getDeviceFunction() == METER) deviceFunctionDescription = "METER    ";
      else if (m_Devices[i]->getDeviceFunction() == PULSER) deviceFunctionDescription = "PULSER   ";
      else deviceFunctionDescription = "NONE     ";

      // get device status and tranfer it to a string
      string deviceStatus;
      if (m_Devices[i]->getStatus() == PGD_ON) deviceStatus = "ON   ";
      else if (m_Devices[i]->getStatus() == PGD_OFF) deviceStatus = "OFF  ";
      else deviceStatus = "ERROR";

      cout << "GPIB" << m_Devices[i]->getBoard() << "\t" << m_Devices[i]->getPAD() << "\t" << m_Devices[i]->getSAD() << "\t" << deviceTypeDescription << "\t" << deviceFunctionDescription
           << "\t" << deviceStatus;
      m_Devices[i]->measureVoltages(0., false, -1);
      m_Devices[i]->measureCurrents(0., false, -1);
      if (m_Devices[i]->getDeviceNumberChannels() > 0)
    	  cout << "\t" << m_Devices[i]->getVoltage(0) << "\t" << m_Devices[i]->getCurrent(0);
      if (m_Devices[i]->getDeviceNumberChannels() > 1)
    	  cout << "\t" << m_Devices[i]->getVoltage(1) << "\t" << m_Devices[i]->getCurrent(1);
      if (m_Devices[i]->getDeviceNumberChannels() > 2)
    	  cout << "\t" << m_Devices[i]->getVoltage(2) << "\t" << m_Devices[i]->getCurrent(2);
      cout << endl;

    }
    cout << "----------------------------" << endl;
  }
  return 0;
}

////////////////////////
// printGPIBError()
////////////////////////
void PixGPIB::printGPIBError(char *msg) {

    cout << msg << endl;

    cout << "* ibsta = " << hex << ibsta << dec << " <";
//    printf ("* ibsta = &H%x  <", ibsta);
    if (ibsta & ERR ) cout << " ERR";
    if (ibsta & TIMO) cout << " TIMO";
    if (ibsta & END ) cout << " END";
    if (ibsta & SRQI) cout << " SRQI";
    if (ibsta & RQS ) cout << " RQS";
    if (ibsta & CMPL) cout << " CMPL";
    if (ibsta & LOK ) cout << " LOK";
    if (ibsta & REM ) cout << " REM";
    if (ibsta & CIC ) cout << " CIC";
    if (ibsta & ATN ) cout << " ATN";
    if (ibsta & TACS) cout << " TACS";
    if (ibsta & LACS) cout << " LACS";
    if (ibsta & DTAS) cout << " DTAS";
    if (ibsta & DCAS) cout << " DCAS";
    cout << " >" <<endl;

    cout << "* iberr = " << iberr << endl;
    if (iberr == EDVR) cout << " EDVR <Driver Error>" << endl;
    if (iberr == ECIC) cout << " ECIC <Not Controller-In-Charge>" << endl;
    if (iberr == ENOL) cout << " ENOL <No Listener>" << endl;
    if (iberr == EADR) cout << " EADR <Address error>" << endl;
    if (iberr == EARG) cout << " EARG <Invalid argument>" << endl;
    if (iberr == ESAC) cout << " ESAC <Not System Controller>" << endl;
    if (iberr == EABO) cout << " EABO <Operation aborted>" << endl;
    if (iberr == ENEB) cout << " ENEB <No GPIB board>" << endl;
    if (iberr == EDMA) cout << " EDMA <DMA Error>" << endl;
    if (iberr == EOIP) cout << " EOIP <Async I/O in progress>" << endl;
    if (iberr == ECAP) cout << " ECAP <No capability>" << endl;
    if (iberr == EFSO) cout << " EFSO <File system error>" << endl;
    if (iberr == EBUS) cout << " EBUS <Command error>" << endl;
    if (iberr == ESRQ) cout << " ESRQ <SRQ stuck on>" << endl;
    if (iberr == ETAB) cout << " ETAB <Table Overflow>" << endl;
#if !defined(USE_LINUX_GPIB)
    if (iberr == ELCK) cout << " ELCK <Interface is locked>" << endl;
    if (iberr == EARM) cout << " EARM <ibnotify callback failed to rearm>" << endl;
    if (iberr == EHDL) cout << " EHDL <Input handle is invalid>" << endl;
    if (iberr == EWIP) cout << " EWIP <Wait in progress on specified input handle>" << endl;
    if (iberr == ERST) cout << " ERST <The event notification was cancelled due to a reset of the interface>" << endl;
    if (iberr == EPWR) cout << " EPWR <The interface lost power>" << endl;
#endif
    cout << "* ibcntl = " << ibcntl << endl;
}
