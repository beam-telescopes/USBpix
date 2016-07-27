#include "PixRs232Device.h"
#include "ComTools.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <upsleep.h>

#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PRD_DEBUG false

PixRs232Device::PixRs232Device(Portids port) : m_port(port)
{
	debug("PixRs232Device::PixRs232Device");
	resetError();
	m_DeviceNumberChannels = 0;
	if(!ComOpen(m_port,9600,P_NONE,S_1BIT,D_8BIT)){
		m_lastErrMsg += "ERROR: Can't open interface!\n\n";
		m_Status = COM_ERROR;
		return;
	}
	m_PowerStatus = COM_OFF;
	m_Status = COM_OK;

	identifyDevice();		// to know that it is a mercury device
	updateDeviceFunction();  // then it knows position
	updateDeviceNumberChannels();
	initializeDevice();

//	double posdummy = 0;
//	printf ("Position dummy: %f\n", posdummy);

	//posdummy = getPositionMercury();
	//printf ("Position dummy: %f\n", posdummy);

	//moveAbsolute(200);  // is waiting for the getPosition function to be within certain limits

//	posdummy = getPositionMercury();
//	printf ("Position dummy after moving: %f\n", posdummy);

	if (PRD_DEBUG) printDevice();
}

PixRs232Device::~PixRs232Device(){
  ComClose(m_port);
}

void PixRs232Device::identifyDevice()
{
	debug("PixRs232Device::identifyDevice()");
	// get ID of device
	m_DeviceType = UNKNOWN;
	m_DeviceFunction = NONE;
	m_writeTerminationString = "\r\n";	//termination string of iseg CR LF, first try
	m_readTerminationString = "\r\n";	//termination string of iseg CR LF, first try

	// the following line MUST NOT BE COMMENTED! otherwise, ISEQ supply will not be indentified
	writeDevice("NONE");	//test string used to determine the device from the device answer
	std::string response;

	if (readDevice(response)) { //no error returned most likely if the termination string is correct
		debug("PixRs232Device::identifyDevice: First termination string fits, its most likely a ISEG device");
		if (response.compare("?WCN")==0) {	//ISEQ answers to NONE with ?WCN
			writeDevice("#");
			readDevice(response);
			if (response.length()>10) { // the response of a ISEG is rather long, this is the criterium to identify it
				debug("PixRs232Device::identifyDevice(): ISEQ found");
				m_DeviceType = ISEG_SHQ;
				return;
			}

		}
		else {
			debug("PixRs232Device::identifyDevice(): ERROR: in identifying device, maybe device type unknown or not connected to selected port");
			m_lastErrMsg += "ERROR identifying device, but correct termination strings\n";
			m_Status = COM_ERROR;
			return;
		}

	}

	//reset errors, time out is wanted here to identify device
	resetError();
	//std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!INIZIO"<<std::endl;

	m_writeTerminationString = "\r\n";	//termination string of iseg CR LF, first try
	m_readTerminationString = "\r\n";	//termination string of iseg CR LF, first try
	writeDevice("TE"); // test call GE_AXIS
//	Sleep(700); // windows
	usleep(700 * 1000); // linux
	readDevice(response);
	std::cout<<response<<std::endl;

	if(response.compare("TEA")==0 || response.compare("TEB")==0 || response.compare("TE@")==0){
		debug("PixRs232Device::identifyDevice(): GE_AXIS found");
		m_DeviceType = GE_AXIS;
		m_Status = COM_OK;
		return;
	}

	m_writeTerminationString = "\r";	//termination string of MICOS CR, second try
	m_readTerminationString = "\r\n\3";	//termination string of MICOS CR LF ETX, second try

	writeDevice(".0VE");	//test string used to determine the device from the device answer
	if(readDevice(response)){	//MICOS MOCO DC answers to ve with identification string Micos GmbH
		if(response.find("Micos GmbH")!=response.npos){
			m_DeviceType = MOCO_DC;
			return;
		}
		if(response.find("Automation/PI")!=response.npos){
			m_DeviceType = MERCURY;
			return;
		}
	}

	//reset errors, time out is wanted here to identify device
	resetError();

	m_writeTerminationString = "\r";	//input termination string of julabo chiller
	m_readTerminationString = "\r\n";	//output termination string of julabo chiller

	writeDevice("version");   //test string used to determine the device from the device answer
	m_DeviceType = JULABO;
	std::cout << "recog" << std::endl;
	return;
	if(readDevice(response)){
		std::string searchexp("J\0U\0L\0A\0B\0O\0",12);
		std::cout << response << std::endl;
		if(response.find(searchexp)!=response.npos){
			m_DeviceType = JULABO;
			return;
		}
	}

	debug(std::string("PixRs232Device::identifyDevice(): ERROR: cannot identifiy device, maybe device type unknown"));
	m_lastErrMsg += "ERROR: cannot identifiy device, maybe device type unknown!\n\n";
	m_Status = COM_ERROR;
}

void PixRs232Device::updateDeviceFunction()
{
	debug("PixRs232Device::updateDeviceFunction()");
	if (m_DeviceType == ISEG_SHQ) m_DeviceFunction = SUPPLY;
	else if (m_DeviceType == MOCO_DC || m_DeviceType == MERCURY || m_DeviceType == GE_AXIS) m_DeviceFunction = POSITION;
	else if (m_DeviceType == JULABO) m_DeviceFunction = CHILLER;
	else m_DeviceFunction = NONE;
}

void PixRs232Device::updateDeviceNumberChannels()
{
	debug("PixRs232Device::updateDeviceNumberChannels()");
	std::string tResponse = "";
	m_DeviceNumberChannels = 0;
	if (m_DeviceType == ISEG_SHQ){
		writeDevice("T2");	//request status bits of channel two to check if there is one (two versions available...)
		readDevice(tResponse);
		if(tResponse.compare("?WCN")!=0){
			m_DeviceNumberChannels = 2;
		}
		else
			m_DeviceNumberChannels = 1;
	}
	else if (m_DeviceType == MOCO_DC){
		for(unsigned int i = 0; i<16; ++i){	//up to 16 different controllers can be addressed via one com port, ping to find out how many are connected, they have to sorted with the start address 0!
			writeDevice("TS",i);
			if(readDevice(tResponse))
				m_DeviceNumberChannels++;
			else{
				writeDevice("TS",0);	//once after wrong read one has to read a working address, device feature...
				readDevice(tResponse);
				resetError();
				break;
			}
		}
		if(m_DeviceNumberChannels == 0){
			m_Status = COM_ERROR;
			m_lastErrMsg += "ERROR: communication errors, cant find a channel\n";
		}
	}
	else if (m_DeviceType == JULABO){
		m_DeviceNumberChannels = 1;
	}
	else if (m_DeviceType == MERCURY){
		m_DeviceNumberChannels = 1;
	}
	else if (m_DeviceType == GE_AXIS){
        m_DeviceNumberChannels = 3;
	}
}

void PixRs232Device::initializeDevice()
{
	debug("PixRs232Device::initializeDevice()");
	if (m_DeviceType == ISEG_SHQ){
		std::string response;
		writeDevice("T1");	//request status bits to check if device is on
		readDevice(response);
		debug("   ...response to T1: "+response);
		int tStatus = atoi(response.c_str());
		if((tStatus & 8) == 8)
			m_PowerStatus= COM_OFF;
		else if((tStatus & 64) == 64)
			m_PowerStatus= COM_LIM;
		else
			m_PowerStatus= COM_ON;
	}
	if (m_DeviceType == MOCO_DC || m_DeviceType == MERCURY){
		for (unsigned int i = 0; i < m_DeviceNumberChannels; ++i)
			initializeChannel(i);
	}
	if (m_DeviceType == JULABO){
		for (unsigned int i = 0; i < m_DeviceNumberChannels; ++i)
			initializeChannel(i);
	}
	if (m_DeviceType == GE_AXIS){
		m_PowerStatus= COM_ON;
		writeDevice("MO\r\n");
//		Sleep(500); // windows
		usleep(500 * 1000); // linux
		for (unsigned int i = 0; i < m_DeviceNumberChannels; ++i)initializeChannel(i);
	}
}

void PixRs232Device::initializeChannel(unsigned int pChannel)
{
	std::stringstream tDebug;
	tDebug<<"PixRs232Device::initializeChannel(): "<<pChannel;
	debug(tDebug.str());
	if (m_DeviceType == MOCO_DC){
		std::string tReadback;
		writeDevice("TS", pChannel);
		readDevice(tReadback);
        int tIntReadback = MOCOstringToInt(tReadback);
		if((tIntReadback & 128) != 128)	//BIT7 shows the motor on/off status, if one channel is on set the device on
			m_PowerStatus = COM_ON;
		writeDevice("DV2000", pChannel); //set standard Speed of 2000/s
		m_maxSpeeds[pChannel] = 2000;
	}
	if (m_DeviceType == MERCURY){

		std::string tReadback;
		char ch [2];
 		writeDevice("TS", pChannel);
    readDevice(tReadback);
    //std::cout<<"\n INIT: tReadback whole string "<<tReadback<<"\n";
    std::string refString = tReadback.substr(tReadback.find(":")+1, 2);
    //std::cout<<"\n INIT!! part of string that contains...: "<<refString<<"\n";
    for(int a=0;a<=2;a++){ch[a]=refString[a];}
		if(!(ch[0]&0x8)){  // bit set means off, bit 0 means motor on
			std::cout<<"\n MOTOR ON \n";
			m_PowerStatus = COM_ON;
		}else{
  		m_PowerStatus = COM_OFF;
		}  // otherwise motor off

		writeDevice("MN", pChannel); // switch motor on
		writeDevice("SA100000", pChannel); //set standard acceleration
		writeDevice("SV3000", pChannel); //set standard Speed of 2000/s

		m_maxSpeeds[pChannel] = 2000;
	}

	if (m_DeviceType == GE_AXIS){
		m_maxSpeeds[pChannel] = 2000;
                m_ActualPositionGE[pChannel] = 0;
	}
}

bool PixRs232Device::readDevice(std::string& pReadBack)	// read response until m_writeTerminationString is received and ignore instruction loop back
{
	debug("PixRs232Device::readDevice()");
	int iLen = 0;
	int iTimeout=0;
	char cBuffer[65];
	cBuffer[0]='\0';

	pReadBack.clear();

	// read from COM port until end of message
	while (iTimeout<__TIMEOUT){
		iLen = ComRead(m_port,cBuffer,64);
		if(iLen>0){// && iLen > m_readTerminationString.length() - 1){
			cBuffer[iLen]='\0';
			pReadBack += std::string(cBuffer);
			if(stringIsTerminated(pReadBack)) break;
		}
		else{
		  UPGen::Sleep(50);
		  iTimeout++;
		}
	}

	//a lot of error handling necessary, rather buggy communication for some devices, especially USB->COM port adapters
	if(iTimeout==__TIMEOUT){	//time out, no (termination string = m_writeTerminationString) found
		if(pReadBack.length() == 0){
			m_Status = COM_ERROR; //nothing read back is a clear error
			m_lastErrMsg += "ERROR: timeout reading from COM port, no response\n";
		}
		else{
			m_Status = COM_WARNING; //read sth. back is a warning
			m_lastErrMsg += "WARNING: timeout reading from COM port, response so far: \"" + pReadBack +"\"\n";
		}
		debug("PixRs232Device::readDevice: read back time out, abort");
		return false;
	}
	if (pReadBack.compare(m_readTerminationString) == 0){	//only! termination string found --> read again necessary
		if(m_DeviceType != ISEG_SHQ){	//ISEQ can just return termination string
			debug("PixRs232Device::readDevice: only determination string in read back, read again");
			return readDevice(pReadBack);
		}
	}
	if (pReadBack.compare(m_lastCommand) == 0){	//some devices loop back the command including termination string --> read again necessary
		debug("PixRs232Device::readDevice: loop back detected, read again");
		return readDevice(pReadBack);
	}
	if (pReadBack.find(m_lastCommand) != pReadBack.npos){	//some devices loop back the command without termination string --> get rid of loop back command
		debug("PixRs232Device::readDevice: loop back without termination string detected, deleting loop back");
		pReadBack.replace(pReadBack.find(m_lastCommand), m_lastCommand.size(), "");
	}

	pReadBack.replace(pReadBack.find(m_readTerminationString), m_readTerminationString.size(), "");	//get rid of the termination string
	if(pReadBack.length() == 0)
		debug("PixRs232Device::readDevice: only termination string");
	else
		debug(std::string("PixRs232Device::readDevice: ").append(pReadBack));

	return true;
}

void PixRs232Device::setStatus(DeviceStatus state)
{
	debug("PixRs232Device::setStatus()");
	m_Status = state;
}

void PixRs232Device::setPowerStatus(DevicePowerStatus pState)
{
  debug("PixRs232Device::setPowerStatus()");
  switch(m_DeviceType){
  case MOCO_DC: // turn motors on/off, in off state they do not keep position
	  if (pState == COM_ON)
		writeMultiDevice("mn");	//all channels motor on
	  else{
		writeMultiDevice("ab");	//abort all movements
		writeMultiDevice("mf");	//all channels motor off
	  }
	  m_PowerStatus = pState;
	  break;
  case MERCURY:
    if (pState == COM_ON)
      writeMultiDevice("mn");	//all channels motor on
    else{
      writeMultiDevice("ab");	//abort all movements
      writeMultiDevice("mf");	//all channels motor off
    }
    m_PowerStatus = pState;
    break;
  case JULABO:
    setJulaboChillerStatus(pState);
    m_PowerStatus = pState;
    break;
  case ISEG_SHQ: // can't turn on/off, but clear limit state if turned off in case limit fired
    if(pState == COM_OFF && m_PowerStatus == COM_LIM){
      std::string response;
      writeDevice("S1");
      readDevice(response);
    }
  default:
    break; // do nothing, some devices do not have a remote power switch
  }
}

void PixRs232Device::setVoltage(unsigned int channel, double voltage)
{
	debug("PixRs232Device::setVoltage()");
	std::string response;
	std::stringstream a;
	a<<(channel+1);
	char volts[50];

	if(channel>m_DeviceNumberChannels) return;

	if(m_DeviceFunction != SUPPLY){
		m_lastErrMsg += "LIMIT: cannot set a voltage for this device type!\n\n";
		m_Status = COM_LIMIT;
		return;
	}

	switch(m_DeviceType){
	case ISEG_SHQ:
		// set voltage
		UPG_sprintf(volts,"%4.2f", fabs(voltage));
		writeDevice("D"+a.str()+"="+std::string(volts));
		if (!readDevice(response))
			return;
		// set ramp speed to maximum
		writeDevice("V"+a.str()+"=255");
		if (!readDevice(response))
			return;
		// start ramp and wait till done
		writeDevice("G"+a.str());
		if (!readDevice(response))
			return;
		// response="L2H";
		while(response=="S1=L2H" || response=="S1=H2L"){
			writeDevice("S"+a.str());
			if (!readDevice(response))
				return;
		}
		break;
	default:
		break; // do nothing
	}
}
void PixRs232Device::setCurrentLimit(unsigned int, double)// unsigned int channel, double current)
{
  // not implemented for ISEG SHQ
}

float PixRs232Device::getVoltage(unsigned int channel, bool )//inAutoRange)
{
	debug("PixRs232Device::getVoltage()");
	if(m_DeviceFunction != SUPPLY)
		return 0.;

	float tVolts = 0.;

	switch(m_DeviceType){
	case ISEG_SHQ:
		if(channel < m_DeviceNumberChannels){
			std::string response;
			std::stringstream a;
			a<<(channel+1);
			writeDevice("U"+a.str());	// request read voltage of one channel
			if (!readDevice(response))
				return 0.;
			tVolts = ISEGatof(response.c_str());
		}
		else{
			m_lastErrMsg += "LIMIT: cannot get a voltage for this channel. Channel does not exist!\n\n";
			m_Status = COM_LIMIT;
		}
		break;
	default:
		break; // do nothing
	}
	return tVolts;
}

float PixRs232Device::getCurrent(unsigned int channel, bool )//inAutoRange)
{
	debug("PixRs232Device::getCurrent()");

	if(m_DeviceFunction != SUPPLY)
		return 0.;

	float tCurr = 0.;

	switch(m_DeviceType){
	case ISEG_SHQ:
		if(channel < m_DeviceNumberChannels){
			std::string response;
			std::stringstream a;
			a<<(channel+1);
			writeDevice("I"+a.str());	// request read current
			if (!readDevice(response))
				return 0.;
			tCurr = ISEGatof(response.c_str());
		}
		else{
			m_lastErrMsg += "LIMIT: cannot get a current for this channel. Channel does not exist!\n\n";
			m_Status = COM_LIMIT;
		}
		break;
 	default:
 		break; // do nothing
	}
	return tCurr;
}

double pos=0;
int PixRs232Device::setPosition(unsigned int pChannel, int pPosition)
{
	std::stringstream tDebug;
	tDebug<<"PixRs232Device::setPosition("<<pPosition<<")";
	debug(tDebug.str());
	if(m_DeviceFunction != POSITION) return 0;

	if(m_DeviceType == GE_AXIS) {
          char moveCommand[16];
          double step = (pPosition - m_ActualPositionGE[pChannel]) * 0.001;
          sprintf(moveCommand, "%dPR%f\r\n", pChannel + 1, step);
          writeDevice(moveCommand);
          usleep(250 * 1000);
          m_ActualPositionGE[pChannel] = pPosition;
        }

	if(m_DeviceType == MOCO_DC){
		if(pChannel < m_DeviceNumberChannels){
			int tDistance = abs(pPosition-getPosition(pChannel));	//calculate the distance to go
			unsigned int tMaxTime = (int)(0.0012*tDistance-0.032*m_maxSpeeds[pChannel]+150); // defines the maximum delay until the motor has to reach the desired position; empiric formular from measurements; otherwise status is set to LIMIT
			int tActualPosition = 0;
			std::stringstream tCommand;
			tCommand<<"MA"<<pPosition;	//move to position
			writeDevice(tCommand.str(), pChannel);
			for (unsigned int tActualTime = 0; tActualTime<tMaxTime; ++tActualTime){
				std::string tReadback;
				writeDevice("MS", pChannel);	//tell moving status
				readDevice(tReadback);
				int tMovingStatus = MOCOstringToInt(tReadback);	//for tMovingStatus = 4 the trajectory is completed
				writeDevice("TF", pChannel);	//tell error between set/real position
				readDevice(tReadback);
				int tMovingError = MOCOstringToInt(tReadback);
				tActualPosition = getPosition(pChannel);	//get actual position
				if((tMovingStatus & 4) == 4 && tMovingError==0 && tActualPosition == pPosition){	//moving successfully competed when tMovingStatus = 4 and the error is 0
//					std::cout<<"MOVING COMPLETE"<<std::endl;
//					std::cout<<"tDistance "<<tDistance<<"\tspeed "<<m_maxSpeeds[pChannel]<<std::endl;
//					std::cout<<"MOVEMENT TOOK "<<tActualTime<<"\tMOVEMENT ALLOWED "<<tMaxTime<<std::endl;
//					std::cout<<"MOVEMENT ALLOWED "<<tMaxTime<<std::endl;
					break;
				}
				if(tActualTime==tMaxTime){
					debug("PixRs232Device::setPosition: LIMIT: position not reached!");
					m_lastErrMsg += "LIMIT: cannot set desired position!\n\n";
					m_Status = COM_LIMIT;
					break;
				}
			}
			return tActualPosition;
		}
		else{
			m_lastErrMsg += "LIMIT: cannot set a position for this channel. Channel does not exist!\n\n";
			m_Status = COM_LIMIT;
		}
	}
	if(m_DeviceType == MERCURY){
		if(pChannel < m_DeviceNumberChannels){
		  //int tActualPosition = 0;
			std::stringstream tCommand;

			// new move commands
			double pPositionDouble = pPosition;
			moveAbsolute(pPositionDouble, pChannel);

		//tCommand<<"MA"<<pPosition;	//move to position
		//writeDevice(tCommand.str(), pChannel);

		}
		else{
			m_lastErrMsg += "LIMIT: cannot set a position for this channel. Channel does not exist!\n\n";
			m_Status = COM_LIMIT;
		}
	}

	return 0;
}

void PixRs232Device::goHome()
{
	debug("PixRs232Device::goHome()");
	if(m_DeviceFunction != POSITION)
		return;
	if(m_DeviceType == MOCO_DC || m_DeviceType == MERCURY)
		writeMultiDevice("GH");
	if(m_DeviceType == GE_AXIS)
		return;
}
void PixRs232Device::setHome()
{
	debug("PixRs232Device::setHome()");
	if(m_DeviceFunction != POSITION)
		return;
	if(m_DeviceType == MOCO_DC || m_DeviceType == MERCURY)
		writeMultiDevice("DH");
	if(m_DeviceType == GE_AXIS)
		return;
}
void PixRs232Device::getError(std::string &errtxt)
{
  errtxt = m_lastErrMsg;
  m_lastErrMsg = "";
}

void PixRs232Device::writeDevice(std::string pCommand, int pAddress)
{
	pCommand.append(m_writeTerminationString);
	if(pAddress == -1){
		ComWrite(m_port,(void*)pCommand.c_str(),pCommand.length());
		debug(std::string("Sending command ").append(pCommand));
	}
	else if (m_DeviceType == MOCO_DC || m_DeviceType == MERCURY){
		std::stringstream tCommand;
		tCommand<<"\x01"<<pAddress<<pCommand;	//add address identifier 0x1, address and command
		ComWrite(m_port,(void*)tCommand.str().c_str(),tCommand.str().length());
		std::stringstream tDebug;
		tDebug<<"Sending command "<<tCommand.str()<<" for channel address "<<pAddress;
		debug(tDebug.str());
	}
#ifdef WIN32
	Sleep(50);
#else
	usleep(50 * 1000);
#endif
	m_lastCommand = pCommand;
}

void PixRs232Device::writeMultiDevice(std::string pCommand)
{
	for(unsigned int i=0; i<m_DeviceNumberChannels;++i){
		writeDevice(pCommand, i);
	}
}

void PixRs232Device::printDevice()
{
	// print m_Instruments array
	std::cout<<"--- RESULTS OF COM "<<m_port+1<<" INIT ---\n";
	std::cout<<"PORT:\tDevice Type:\tFunction:\tPower:\tStatus:\t";
	for(unsigned int i = 0; i < m_DeviceNumberChannels; ++i){
		std::cout<<"CH"<<i+1<<"\t";
	}

	std::cout<<"\n";

	// get device type and transfer it to a std::string
	std::string deviceTypeDescription;
	if (getDeviceType() == ISEG_SHQ) deviceTypeDescription = "ISEG_SHQ";
	else if (getDeviceType() == MOCO_DC) deviceTypeDescription = "MOCO_DC";
	else if (getDeviceType() == MERCURY) deviceTypeDescription = "MERCURY";
	else if (getDeviceType() == JULABO) deviceTypeDescription = "JULABO CHILLER";
	else if (getDeviceType() == GE_AXIS) deviceTypeDescription = "GE_AXIS";
	else deviceTypeDescription = "UNKNOWN  ";

	// get device function and transfer it to a std::string
	std::string deviceFunctionDescription;
	if (getDeviceFunction() == POSITION) deviceFunctionDescription = "POSITION ";
	else if (getDeviceFunction() == METER) deviceFunctionDescription = "METER    ";
	else if (getDeviceFunction() == SUPPLY) deviceFunctionDescription = "SUPPLY   ";
	else if (getDeviceFunction() == CHILLER) deviceFunctionDescription = "CHILLER   ";
	else deviceFunctionDescription = "NONE     ";

	// get device status and transfer it to a std::string
	std::string devicePowerStatus = "unknown";
	if(getPowerStatus() == COM_ON) devicePowerStatus = "ON   ";
	else if(getPowerStatus() == COM_OFF) devicePowerStatus = "OFF  ";

	std::string deviceStatus = "unknown";
	if(getStatus() == COM_OK) deviceStatus = "OK";
	else if(getStatus() == COM_LIMIT) deviceStatus = "LIMIT";
	else if(getStatus() == COM_WARNING) deviceStatus = "WARNING";
	else if(getStatus() == COM_ERROR){
		deviceStatus = "ERROR";
		devicePowerStatus = "UNKOWN  ";
	}

	std::cout << "COM" << getPortID()+1 << "\t" << deviceTypeDescription << "\t" << deviceFunctionDescription << "\t" << devicePowerStatus<< "\t" << deviceStatus;
	for(unsigned int i = 0; i < m_DeviceNumberChannels; ++i){
		if (m_DeviceFunction == SUPPLY){
			std::cout << "\t" <<getVoltage(i) << "V /" <<getCurrent(i)<<"A";
		}
		if (m_DeviceFunction == POSITION){
			std::cout << "\t" <<getPosition(i);
		}
		if (m_DeviceFunction == CHILLER){
			std::cout << "\t Temperature: " <<getChillerCurrentTemperature(i) << " and controlled by external temp sensor: "<<getChillerRegulation(i); //FIXME
		}
	}
	std::cout<<std::endl;
	std::cout<< "----------------------------" <<std::endl;
}

PixRs232Device::DeviceType PixRs232Device::getDeviceType()
{
	return m_DeviceType;
}

PixRs232Device::DeviceStatus PixRs232Device::getStatus()
{
	return m_Status;
}

PixRs232Device::DevicePowerStatus PixRs232Device::getPowerStatus(int pChannel)
{
	if (m_DeviceType == ISEG_SHQ){
		std::string response;
		std::stringstream a;
		writeDevice("T1");	//request status bits to check if device is on
		readDevice(response);
		debug("PixRs232Device::getPowerStatus: response to T1: "+response);
		int tStatus = atoi(response.c_str());
		if((tStatus & 8) == 8)
			m_PowerStatus= COM_OFF;
		else if((tStatus & 64) == 64)
			m_PowerStatus= COM_LIM;
		else
			m_PowerStatus= COM_ON;
	}
	if (m_DeviceType == MOCO_DC || m_DeviceType == MERCURY){
		if(pChannel == -1){
			for (unsigned int iChannel = 0; iChannel < m_DeviceNumberChannels; ++iChannel){	//return COM_ON if one motor is on
				std::string tReadback;
				writeDevice("TS", iChannel);
				readDevice(tReadback);
				int tIntReadback = MOCOstringToInt(tReadback);
				if((tIntReadback & 128) != 128)	//BIT7 shows the motor on/off status, if one channel is on set the device on
					m_PowerStatus= COM_ON;
			}
		}
		else{
		  if((unsigned int)pChannel < m_DeviceNumberChannels){
				std::string tReadback;
				writeDevice("TS", pChannel);
				readDevice(tReadback);
				int tIntReadback = MOCOstringToInt(tReadback);
				if((tIntReadback & 128) != 128)	//BIT7 shows the motor on/off status, if one channel is on set the device on
					m_PowerStatus= COM_ON;
			}
			else{
				m_lastErrMsg += "LIMIT: cannot get power status for this channel. Channel does not exist!\n\n";
				m_Status = COM_LIMIT;
			}
		}
	}
	if (m_DeviceType == JULABO){
		m_PowerStatus=getJulaboChillerStatus();
	}
	return m_PowerStatus;
}

PixRs232Device::DeviceFunction PixRs232Device::getDeviceFunction()
{
	return m_DeviceFunction;
}

PixRs232Device::Portids PixRs232Device::getPortID()
{
	return m_port;
}

void PixRs232Device::debug(std::string pDebugString)
{
	if (PRD_DEBUG)
		std::cout<<pDebugString<<"\n";
}

bool PixRs232Device::stringIsTerminated(std::string pString)
{
  if(pString.length() < m_readTerminationString.length()) return false;
  return (pString.substr(pString.length() - m_readTerminationString.length(), m_readTerminationString.length()).compare(m_readTerminationString) == 0);
}

float PixRs232Device::getJulaboChillerCurrentInternalTemperature() {
	std::string response;
	writeDevice("in_pv_00");
	readDevice(response);
	float tTemperature = (float)atof(response.c_str());
	return tTemperature;
}

float PixRs232Device::getJulaboChillerCurrentExternalTemperature() {
	std::string response;
	writeDevice("in_pv_02");
	readDevice(response);
	float tTemperature = (float)atof(response.c_str());
	return tTemperature;
}

float PixRs232Device::getJulaboChillerHeatingPower() {
	std::string response;
	writeDevice("in_pv_01");
	readDevice(response);
	float tHeatingPower = (float)atof(response.c_str());
	return tHeatingPower;
}

float PixRs232Device::getJulaboChillerSetTemperature() {
	std::string response;
	writeDevice("in_sp_00");
	readDevice(response);
	float tSetTemperature = (float)atof(response.c_str());
	return tSetTemperature;
}

PixRs232Device::DeviceStatus PixRs232Device::getJulaboChillerAlarm() {
	std::string response;
	writeDevice("status");
	readDevice(response);
	int tSetTemperature = (int)(atof(response.c_str())+0.5);
	if ((tSetTemperature==0)||(tSetTemperature==1)){
		return COM_LOCAL;
	} else if((tSetTemperature==2)||(tSetTemperature==3)){
		return COM_OK;
	} else{
		return COM_ERROR;
	}
}

bool PixRs232Device::getJulaboChillerRegulation() {
	std::string response;
	writeDevice("in_mode_04");
	readDevice(response);
	int intExt = (int)atoi(response.c_str());
	if (intExt==0) {
		return false;
	} else
		return true;
}

PixRs232Device::DevicePowerStatus PixRs232Device::getJulaboChillerStatus() {
	std::string response;
	writeDevice("in_mode_05");
	readDevice(response);
	int tStatus = (int)atof(response.c_str());
	if (tStatus==0){
		return COM_OFF;
	} else if(tStatus==1){
		return COM_ON;
	}
	return COM_LIM; // just to return something else - do we need another state here??
}

void PixRs232Device::setJulaboChillerStatus(PixRs232Device::DevicePowerStatus tStatus) {
	if (tStatus==COM_ON){
		writeDevice("out_mode_05 1");
	} else if(tStatus==COM_OFF){
		writeDevice("out_mode_05 0");
	}
}

void PixRs232Device::setJulaboChillerTargetTemperature(float value) {
	char msg[64];
	UPG_sprintf(msg,"out_sp_00 %.2f", value);
	std::string output;
	output = msg;
	writeDevice(output);
}

void PixRs232Device::setJulaboChillerRegulation(bool extInt) {
	if(extInt==0){
		writeDevice("out_mode_04 0");
	} else
		writeDevice("out_mode_04 1");
}

float PixRs232Device::getChillerCurrentTemperature(unsigned int ){//pChannel) {
	switch(m_DeviceType){
	case JULABO:
		if(getJulaboChillerRegulation()==false){
			return getJulaboChillerCurrentInternalTemperature();
		} else
			return getJulaboChillerCurrentExternalTemperature();
		break;
	default:
		return -300;
		break; // do nothing beside setting an invalid value
	}
}

float PixRs232Device::getChillerSetTemperature(unsigned int ){//pChannel) {
	switch(m_DeviceType){
	case JULABO:
		return getJulaboChillerSetTemperature();
		break;
	default:
		return -300;
		break; // do nothing beside setting an invalid value
	}
}

void PixRs232Device::setChillerTargetTemperature(unsigned int ,	float value) { //pChannel
	switch(m_DeviceType){
	case JULABO:
		return setJulaboChillerTargetTemperature(value);
		break;
	default:
		return;
		break; // do nothing
	}
}

bool PixRs232Device::getChillerRegulation(unsigned int ){//pChannel) {
	switch(m_DeviceType){
	case JULABO:
		return getJulaboChillerRegulation();
		break;
	default:
		return false;
		break; // do nothing beside setting an invalid value
	}
}

void PixRs232Device::setChillerRegulation(unsigned int , bool choice) { //pChannel
	switch(m_DeviceType){
	case JULABO:
		return setJulaboChillerRegulation(choice);
		break;
	default:
		return;
		break; // do nothing
	}
}

void PixRs232Device::resetError()
{
	m_lastErrMsg = "";
	m_Status = COM_OK;
}

int PixRs232Device::getPosition(unsigned int pChannel)
{
	debug("PixRs232Device::getPosition()");
	if(m_DeviceFunction != POSITION)
		return 0;

        if (m_DeviceType == GE_AXIS) {
		return pChannel;
	}

	if(m_DeviceType == MOCO_DC){
		std::string tReadback;
		writeDevice("TP", pChannel);
		readDevice(tReadback);
		return MOCOstringToInt(tReadback);

	}else if(m_DeviceType == MERCURY){

	std::string buffer;
	writeDevice("TP");
	double position_member;
	readDevice(buffer);
	//MOCOstringToInt(tReadback);
	if (buffer == "0")
		return (0);
	//			return position_member;
	buffer = buffer.substr(3, buffer.length()-2);
	//  return (StrToFloat(buffer)/STEPS_PER_MM);
	position_member = (StrToFloat(buffer))/STEPS_PER_MM;
	printf ("Position in getPositionFunction: %f", position_member);
	//	return position_member;
	int test = (int)position_member;
	printf ("Position in getPositionFunction as INT : %d", test);
	return test;
	}
	return 0;
}

void PixRs232Device::setMaxSpeed(int pSpeed)
{
	debug("PixRs232Device::setMaxSpeed(pSpeed)");
	if(m_DeviceFunction != POSITION)
		return;
	if(m_DeviceType == MOCO_DC || m_DeviceType == MERCURY){
		if(pSpeed > 0 && pSpeed <= 1e6){
			std::stringstream tCommand;
			tCommand<<"dv"<<pSpeed;
			writeMultiDevice(tCommand.str());
			for (unsigned int iChannel = 0; iChannel<m_DeviceNumberChannels; ++iChannel)
				m_maxSpeeds[iChannel] = pSpeed;
		}
	}
}

void PixRs232Device::setMaxSpeed(unsigned int pChannel, int pSpeed)
{
	debug("PixRs232Device::setMaxSpeed(pChannel, pSpeed)");
	if(m_DeviceFunction != POSITION)
		return;
	if(pChannel < m_DeviceNumberChannels){
		std::stringstream tCommand;
		tCommand<<"dv"<<pSpeed;
		writeDevice(tCommand.str(), pChannel);
	}
	else{
		m_lastErrMsg += "LIMIT: cannot set a Speed for this channel. Channel does not exist!\n\n";
		m_Status = COM_LIMIT;
	}
	m_maxSpeeds[pChannel] = pSpeed;
}

int PixRs232Device::getSpeed(unsigned int pChannel)
{
	debug("PixRs232Device::getMaxSpeed()");
	if(m_DeviceFunction != POSITION)
		return 0;
	if(m_DeviceType == MOCO_DC){
		std::string tReadback;
		writeDevice("TV", pChannel);
		readDevice(tReadback);
		return MOCOstringToInt(tReadback);
	}
	if(m_DeviceType == MERCURY){
		std::string tReadback;
		writeDevice("TY", pChannel);
		readDevice(tReadback);
		return MOCOstringToInt(tReadback);
	}
	if(m_DeviceType == GE_AXIS)
		return 0;
	return 0;
}

int PixRs232Device::getMaxSpeed(unsigned int pChannel)
{
	debug("PixRs232Device::getMaxSpeed()");
	if(m_DeviceFunction != POSITION)
		return 0;
	if(m_DeviceType == MOCO_DC || m_DeviceType == MERCURY){
		return m_maxSpeeds[pChannel];
	}
	if(m_DeviceType == GE_AXIS)
		return 0;
	return 0;
}

float PixRs232Device::ISEGatof(const char *svalue_in){
  float value = 0.;
  std::string sexp="0", svalue = svalue_in;
  int pos = svalue.find_last_of("-");
  if(pos==(int)std::string::npos) pos = svalue.find_last_of("+");
  if(pos!=(int)std::string::npos){
    sexp = svalue.substr(pos,3);
    svalue = svalue.substr(0,pos);
  }
  value = ((float)atoi(svalue.c_str()))*(float)pow(10.,atoi(sexp.c_str()));
  return value;
}

int PixRs232Device::MOCOstringToInt(std::string pString)
{
	pString = pString.substr(pString.find(":")+1, pString.length()-pString.find(":"));
	std::stringstream tValue(pString);
	int tIntValue;
	if (!(tValue>>tIntValue)){
		debug(std::string("PixRs232Device::MOCOstringToInt: ").append(pString));
		return -1;
	}
	return tIntValue;
}

bool PixRs232Device::moveAbsolute(double val, unsigned int pChannel)
{
  char buffer[256];

  UPG_sprintf(buffer, "MA%d", (int)((float)val*STEPS_PER_MM));
  writeDevice(buffer);

  if(isMoveCompleted(pChannel))return true;
  return false;
}


bool PixRs232Device::isMoveCompleted(unsigned int pChannel)
{
  
//	unsigned int pChannel = 0;
	std::string tReadback;
	char ch [2];
	bool finished;

	do {
	writeDevice("TS", pChannel);
	readDevice(tReadback);
	//std::cout<<"\n WAIT: tReadback whole string "<<tReadback<<"\n";
	std::string refString = tReadback.substr(tReadback.find(":")+1, 2);
	//std::cout<<"\n WAIT!! part of string that contains...: "<<refString<<"\n";
	for(int a=0;a<=2;a++){ch[a]=refString[a];}
	if((ch[1]&0x4)){std::cout<<" Trajectory complete \n";
									finished = true;
									}else{
									finished = false;
									}
	}while(finished == false);

	return finished;
}


bool PixRs232Device::moveRelative(double val)
{
  char buffer[256];

  UPG_sprintf(buffer, "MR%d", (int)((float)val*STEPS_PER_MM));
  writeDevice(buffer);
  if(isMoveCompleted(0))return true;  // test function only works for channel 0!!!!!!! very bad !!!!!
  return false;
}


double PixRs232Device::getPositionMercury()
{
  std::string buffer;
  writeDevice("TP");
	double position_member;
  readDevice(buffer);
	//MOCOstringToInt(tReadback);
	if (buffer == "0")
		return (0);
//			return position_member;
  buffer = buffer.substr(3, buffer.length()-2);
	//  return (StrToFloat(buffer)/STEPS_PER_MM);
	position_member = (StrToFloat(buffer))/STEPS_PER_MM;
	printf ("Position from Function getPositionMercury: %f", position_member);
	return position_member;
}

double PixRs232Device::StrToFloat(std::string stringValue)
{
    std::stringstream ssStream(stringValue);
    double Return;
    ssStream >> Return;

    return Return; 
}

double PixRs232Device::findReference()  // not yet possible
{
	unsigned int LimitRefState = 0;
	unsigned int pChannel = 0;
	std::string tReadback;
	char ch [2];

	// Check if you are already at the reference position
	writeDevice("TS", pChannel);
	readDevice(tReadback);
	std::string refString = tReadback.substr(tReadback.find(":")+13, 2);
	for (int a=0;a<=2;a++)
	    {
	     ch[a]=refString[a];
      }
	 if(!(ch[1]&0x1) && (ch[1]&0x2) && !(ch[1]&0x4) && !(ch[1]&0x8)){
		 std::cout<<" Condition for reference fulfilled\n";
		 LimitRefState = 1;
	 }else if (!(ch[1]&0x1) && !(ch[1]&0x2) && (ch[1]&0x4) && !(ch[1]&0x8)){
		 std::cout<<" Condition for Positive limit fulfilled\n"; 
		 LimitRefState = 2;
	 }else if (!(ch[1]&0x1) && !(ch[1]&0x2) && !(ch[1]&0x4) && (ch[1]&0x8)){
		 std::cout<<" Condition for Negative limit fulfilled\n";
		 LimitRefState = 3;
	 }

	if (LimitRefState == 1) return 0;	  // if you are already at the reference position then immediately return and finish the function

	// if you are not a the reference position....
	 do {
		 moveRelative(-10);
		writeDevice("TS", pChannel);
		readDevice(tReadback);
		std::cout<<"\n tReadback whole string "<<tReadback<<"\n";
		std::string refString = tReadback.substr(tReadback.find(":")+13, 2);
		std::cout<<"\n FIND REFERENCE!! part of string that contains hex code for limits and reference: "<<refString<<"\n";
		for (int a=0;a<=2;a++)
        {
            ch[a]=refString[a];
        }
	 if(!(ch[1]&0x1) && (ch[1]&0x2) && !(ch[1]&0x4) && !(ch[1]&0x8)){
		 std::cout<<" Condition for reference fulfilled\n";
		 LimitRefState = 1;
	 }else if (!(ch[1]&0x1) && !(ch[1]&0x2) && (ch[1]&0x4) && !(ch[1]&0x8)){
		 std::cout<<" Condition for Positive limit fulfilled\n"; 
		 LimitRefState = 2;
	 }else if (!(ch[1]&0x1) && !(ch[1]&0x2) && !(ch[1]&0x4) && (ch[1]&0x8)){
		 std::cout<<" Condition for Negative limit fulfilled\n";
		 LimitRefState = 3;
	 }
		 if (LimitRefState==1) break;
	 } while(LimitRefState != 3);

	 getPositionMercury();	
	 writeDevice("ST", pChannel);
	 writeDevice("DH", pChannel);  // Defines the references switch as the home position
	 getPositionMercury();

	 return 0;
}

double PixRs232Device::findReference2()   // not yet possible
{
	unsigned int pChannel = 0;
//	std::string tReadback;
//	char ch [2];

//	 getPositionMercury();	

  writeDevice("RT", pChannel);  // reset to defaults

#ifdef WIN32
  Sleep(200);         // wait until the controller is ready for new commands  (in ms)
#else
  usleep(200*1000);
#endif

	 writeDevice("FE2", pChannel);
	 writeDevice("WS", pChannel);
	 writeDevice("DH", pChannel);
	 writeDevice("WS", pChannel);
	 writeDevice("DH", pChannel);
	 //	 writeDevice("DH", pChannel);  // Defines the references switch as the home position

	 getPositionMercury();

	 return 0;
}

