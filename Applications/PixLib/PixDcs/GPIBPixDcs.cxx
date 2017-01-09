/*------------------------------------------------------------
 *  GPIBPixDcx.cxx
 *  Version:  0.1.USBsystem
 *  Created:  19 February 2009
 *  Author:   Joern Grosse-Knetter
 *
 *  Functional interface to the USB system for communication
 *  with GPIB devices
 *------------------------------------------------------------*/

#include "PixDcs/GPIBPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixGPIBDevice.h"
#include "SleepWrapped.h"
#include <sstream>
#include <math.h>
#include <string.h>

using namespace PixLib;

GPIBPixDcsChan::GPIBPixDcsChan(PixDcs *parent, DBInquire *dbInquire) : PixDcsChan(parent, dbInquire){
  configInit();
  m_conf->read(dbInquire);
  // get last part of decorated name which describes this object
  std::string myDecName = dbInquire->getMyDecName();
  int pos = (int)myDecName.find_last_of("/");
  pos--;
  pos = (int)myDecName.find_last_of("/", pos);
  myDecName.erase(0,pos+1);
  // set config name to DBInquire's decorated name to allow automatc saving
  m_conf->m_confName = myDecName;
}
GPIBPixDcsChan::GPIBPixDcsChan(GPIBPixDcsChan &chan_in) : PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
}
GPIBPixDcsChan::~GPIBPixDcsChan(){
  //  delete m_conf; // taken care of by PixDcs's config
}
void GPIBPixDcsChan::configInit(){
  // Create the Config object
  m_conf = new Config("GPIBPixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("ChannelID", m_channelID, -1, "ID of channel on this device", true);
  conf.addGroup("settings");
  GPIBPixDcs* myDcs = dynamic_cast<GPIBPixDcs*>(m_parent);
  if(myDcs != 0 && myDcs->getDevType()==PixDcs::SUPPLY){
    conf["settings"].addFloat("NomVolts", m_nomVolts, 0.0, "Nominal voltage (in V) on this channel", true);
    conf["settings"].addFloat("CurrLim", m_currLim, 0.0, "Current limit (in A) on this channel", true);
    conf["settings"].addInt("RampTime",m_rampTime, 1, "Time (in s) for ramping to full voltage", true);
    conf["settings"].addFloat("RampSteps",m_rampVoltSteps, 1, "Voltage-steps (in V) for ramping up/down", true);

    conf["settings"].addFloat("OVP",       m_OVPVoltage, 0.0,   "Voltage (in V) for over-voltage protection", true);
    conf["settings"].addBool ("EnableOVP", m_OVPEnabled, false, "Enable over-voltage protection", true);

    conf["settings"].addFloat("OCP",       m_OCPCurrent, 0.0,   "Current (in A) for over-current protection", true);
    conf["settings"].addBool ("EnableOCP", m_OCPEnabled, false, "Enable over-current protection", true);

    conf["settings"].addBool ("RemoteSensing",m_RemoteSensing, false, "Enable remote sensing", true);
    m_rtype = VCBOTH; // must never change this, so not in config!
  }
  else if(myDcs != 0 && myDcs->getDevType()==PixDcs::PULSER){
	  std::map<std::string,int> tList;
	  tList["PULSE"] = 0;
	  tList["SINE"] = 1;
	  tList["SQUARE"] = 2;
	  tList["RAMP"] = 3;
	  tList["NOISE"] = 4;
	  conf["settings" ].addList ("WaveForm", (int &)m_waveFormType, 0, tList, "Wave form type", true);
      conf["settings"].addFloat("NomVolts", m_nomVolts, 0.1f, "Signal amplitude (in V) on this channel", true);
      conf["settings"].addFloat("NomVoltOffset", m_nomVoltOffset, 0.0, "Signal voltage offset (in V) on this channel", true);
      conf["settings"].addFloat("NomFrequency", m_frequency, 1e3, "Signal Frequency (in Hz) on this channel", true);
      conf["settings"].addInt("NomNburstPulses", m_nBurstPulses, 1, "Number of burst pulses on this channel", true);
      m_rtype = VONLY;	//just to show the set voltage
  }else{
    std::map<std::string,int> rtlist;
    rtlist["VOLTS_ONLY"] = VONLY;
    rtlist["CURR_ONLY"] = CONLY;
    rtlist["VOLTS_AND_CURR"] = VCBOTH;
    conf["general" ].addList ("ReadType", (int &)m_rtype, VONLY, rtlist, "Type of reading for this meter/ADC-channel", true);
    conf["settings"].addBool ("AutoRange", m_autoRange, true, "Automatic adjustment of measurement range (if supported)", true);
    conf["settings"].addFloat("MeasRange", m_measRange, 1.0f, "Range of measurement (unless auto-range is active)", true);
  }
  conf.reset();
}
double GPIBPixDcsChan::ReadParam(std::string measType) {
  // Read a string value from DDC, returning a double.
  GPIBPixDcs* myDcs = dynamic_cast<GPIBPixDcs*>(m_parent);
  double ret=0.0;
  if(m_channelID<0 || m_channelID>=(int)myDcs->m_gpibDev->getDeviceNumberChannels()){
    std::stringstream a,b;
    a << m_channelID;
    b << ((int)myDcs->m_gpibDev->getDeviceNumberChannels());
    throw PixDcsExc(PixDcsExc::ERROR, "Request for unhandled channel no. : "+a.str()+", must be between 0 and " + b.str());
  }

  if(measType=="voltage"){
    if(m_rtype!=CONLY){
      myDcs->m_gpibDev->measureVoltages(m_measRange, m_autoRange, m_channelID);
      ret = (double)myDcs->m_gpibDev->getVoltage(m_channelID);
    } else
      throw PixDcsExc(PixDcsExc::ERROR, "request for voltage reading, but channel is current-reading only");
  } else if(measType=="current"){
    if(m_rtype!=VONLY){
      myDcs->m_gpibDev->measureCurrents(m_measRange, m_autoRange, m_channelID);
      ret = (double)myDcs->m_gpibDev->getCurrent(m_channelID);
    } else
      throw PixDcsExc(PixDcsExc::ERROR, "request for current reading, but channel is voltage-reading only");
  } else
    throw PixDcsExc(PixDcsExc::ERROR, "request for neither voltage nor current: "+measType);
  return ret;
}

void GPIBPixDcsChan::SetParam(string varType) {
  // Set resource with string id to default value.
  if(varType=="voltage")
    SetParam(varType, (double)m_nomVolts);
  else if(varType=="currentlim")
    SetParam(varType, (double)m_currLim);
  else if(varType=="ocp_enabled")
    SetParam(varType, (double)m_OCPEnabled);
  else if(varType=="ocp_current")
    SetParam(varType, (double)m_OCPCurrent);
  else if(varType=="ovp_enabled")
    SetParam(varType, (double)m_OVPEnabled);
  else if(varType=="ovp_voltage")
    SetParam(varType, (double)m_OVPVoltage);
  else if(varType=="remote_sensing")
    SetParam(varType, (double)m_RemoteSensing);
  else if(varType=="currentlim")
    SetParam(varType, (double)m_currLim);
  else if(varType=="frequency")
    SetParam(varType, (double)m_frequency);
  else if(varType=="nbursts")
    SetParam(varType, (double)m_nBurstPulses);
  else if(varType=="voltageoffset")
    SetParam(varType, (double)m_nomVoltOffset);
  else if(varType=="waveform")
    SetParam(varType, (double)m_waveFormType);
  else if(varType=="trigger")
    SetParam(varType, 0);
  else
    throw PixDcsExc(PixDcsExc::ERROR, "request unknown: "+varType);
}

void GPIBPixDcsChan::SetParam(string varType, double value) {
  // Set resource with string id to a double value.
  GPIBPixDcs* myDcs = dynamic_cast<GPIBPixDcs*>(m_parent);
  if(myDcs == 0) return;
  if(myDcs->m_gpibDev==0) return; // can't set anything without init. hardware
  if(m_channelID<0 || m_channelID>=(int)myDcs->m_gpibDev->getDeviceNumberChannels()){
    std::stringstream a,b;
    a << m_channelID;
    b << ((int)myDcs->m_gpibDev->getDeviceNumberChannels());
    throw PixDcsExc(PixDcsExc::ERROR, "Request for unhandled channel no. : "+a.str()+", must be between 0 and " + b.str());
  }
  
  if (varType=="trigger"){	//every GPIB device understands the trigger command by definition
		myDcs->m_gpibDev->sendTrigger();
  		return;
  }

  if(myDcs->getDevType()==PixDcs::SUPPLY){
	  if(varType=="voltage")
	    	rampVoltage(value);
	  else if(varType=="currentlim")
		  myDcs->m_gpibDev->setCurrentLimit(m_channelID, value);
          else if(varType=="ocp_enabled")
		  myDcs->m_gpibDev->setOCPCurrent(m_channelID, value != 0.0, m_OCPCurrent);
          else if(varType=="ocp_current")
		  myDcs->m_gpibDev->setOCPCurrent(m_channelID, m_OCPEnabled, value);
          else if(varType=="ovp_enabled")
		  myDcs->m_gpibDev->setOVPVoltage(m_channelID, value != 0.0, m_OVPVoltage);
          else if(varType=="ovp_voltage")
		  myDcs->m_gpibDev->setOVPVoltage(m_channelID, m_OVPEnabled, value);
          else if(varType=="remote_sensing")
		  myDcs->m_gpibDev->setRemoteSensing(m_channelID, value != 0.0);
	  else
		throw PixDcsExc(PixDcsExc::ERROR, "request for neither voltage nor current limit: "+varType);
	  myDcs->m_gpibDev->measureVoltages();
	  myDcs->m_gpibDev->measureCurrents();
  }
  if(myDcs->getDevType()==PixDcs::PULSER){
		if(m_channelID>=0 && m_channelID<(int)myDcs->m_gpibDev->getDeviceNumberChannels()){
			if(varType=="voltage"){
			  m_nomVolts = (float)value;
			  myDcs->m_gpibDev->setVoltage(m_channelID, m_nomVolts);
			}
			else if(varType=="frequency"){
			  m_frequency = (float)value;
			  myDcs->m_gpibDev->setFrequency(m_channelID, m_frequency);
			}
			else if(varType=="nbursts"){
			  m_nBurstPulses = (int) (value+0.5);
			  myDcs->m_gpibDev->setNburstPulses(m_channelID, m_nBurstPulses);
			}
			else if(varType=="voltageoffset"){
			  m_nomVoltOffset = (float)value;
			  myDcs->m_gpibDev->setVoltageOffset(m_channelID, m_nomVoltOffset);
			}
			else if(varType=="waveform"){
			  m_waveFormType = (int)(value+0.5);
			  myDcs->m_gpibDev->setWaveForm(m_channelID, (unsigned int) m_waveFormType);
			}
			else if(varType=="sendBursts"){	//triggers the burst creation to create value bursts
			  myDcs->m_gpibDev->setNburstPulses(m_channelID, (int) (value+0.5));
			  myDcs->m_gpibDev->sendTrigger();
			}
			else
				throw PixDcsExc(PixDcsExc::ERROR, "request is neither voltge, frequency, number of bursts, votlage offset, wave form: "+varType);
		}
  }
}

std::string GPIBPixDcsChan::ReadState(std::string){
  return "unknown";
}
void GPIBPixDcsChan::SetState(std::string state){
  GPIBPixDcs* myDcs = dynamic_cast<GPIBPixDcs*>(m_parent);
  myDcs->SetState(state);
  return;
}
void GPIBPixDcsChan::rampVoltage(double pVoltage)
{
	GPIBPixDcs* myDcs = dynamic_cast<GPIBPixDcs*>(m_parent);
	float mysign=1;
	float curr_volt = (float)ReadParam("voltage");
	if(pVoltage<curr_volt) mysign=-1; // ramp up or down?
	if(((int)m_rampVoltSteps)!=0 && fabs(pVoltage-curr_volt)>m_rampVoltSteps){ // avoids endless loops and mistakes if difference between current and wanted status is small
		for(float in_volts = curr_volt; (in_volts*mysign)<=(mysign*pVoltage); in_volts+=mysign*fabs(m_rampVoltSteps)){
			myDcs->m_gpibDev->setVoltage(m_channelID, in_volts);
//			SetParam("voltage", in_volts);
			unsigned int ustime = (unsigned int) 1000*m_rampTime;
			PixLib::sleep(ustime);
		}
	}
	myDcs->m_gpibDev->setVoltage(m_channelID, pVoltage);
	// JGK: OFF sets actual voltage to 0, so this step will mess up settings
	// in any case, m_nomVolts should be the desired voltage, not what is actually set
	//m_nomVolts = ReadParam("voltage");	//update with actual reached voltage
	unsigned int ustime = (unsigned int) 1000*m_rampTime;
	PixLib::sleep(ustime);
}
GPIBPixDcs::GPIBPixDcs(DBInquire *dbInquire, void* )//interface)
  : PixDcs(dbInquire, 0){

  m_gpibDev = 0;
  configInit();
  m_conf->read(dbInquire);
  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for DCS channel inquire
    if((*it)->getName() == "PixDcsChan") {
      GPIBPixDcsChan *gch = new GPIBPixDcsChan(this, *it);
      m_channels.push_back(gch);
      m_conf->addConfig(gch->m_conf);
      // set default name and channel ID if none given yet
      int chID = (int)m_channels.size()-1;
      std::stringstream a;
      a << chID;
      if(gch->m_name=="unknown"){
	gch->m_name = m_name+"_Ch"+a.str();
	gch->m_channelID = chID;
      }
    }    
  }
}
GPIBPixDcs::~GPIBPixDcs(){
  delete m_gpibDev;
  delete m_conf;
}
std::string GPIBPixDcs::ReadState(std::string) {
  // Read a string value from DDC, returning a string.
  std::string ret="uninit.";
  if(m_gpibDev!=0){
    switch(m_gpibDev->getStatus()){
    case PGD_ON:
      ret = "ON";
      if(m_devType!=SUPPLY && m_devType!=PULSER) ret = "OK"; // ON and OFF meaningless for meter
      if(m_devType==SUPPLY){
	bool needUpdate = false;
	for(vector<PixDcsChan*>::iterator cIT = chanBegin();cIT!=chanEnd(); cIT++){
	  float nomV = ((ConfFloat&)(*cIT)->config()["settings"]["NomVolts"]).value();
	  float isV = (float)(*cIT)->ReadParam("voltage");
	  if((nomV>0 && isV<0.8*nomV) || (nomV<0 && isV>0.8*nomV)){
	    needUpdate = true;
	    break;
	  }
	}
	if(needUpdate) ret = "ON*";
      }
      break;
    case PGD_OFF:
      ret = "OFF";
      if(m_devType!=SUPPLY && m_devType!=PULSER) ret = "OK"; // ON and OFF meaningless for meter
      break;
    default:
    case PGD_ERROR:
      ret = "ERROR";
    }
  }
  return ret;
}
void GPIBPixDcs::SetState(std::string new_state){
	if(new_state!="OFF" && new_state!="ON" && new_state!="UPDATE")
		throw PixDcsExc(PixDcsExc::ERROR, "unknown request for change in state (should be ON, OFF, UPDATE): "+new_state);
	if(m_gpibDev==0) return; // can't set anything without init. hardware

	if(m_devType == SUPPLY){
		if(m_gpibDev->getStatus()==PGD_ON && new_state=="OFF"){
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
				GPIBPixDcsChan *chan = dynamic_cast<GPIBPixDcsChan*>(*IT);
				chan->rampVoltage(0);
			}
			m_gpibDev->setState(PGD_OFF);	//turn off device
			return;
		}
		if(m_gpibDev->getStatus()==PGD_OFF && new_state=="ON"){
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){	//set all channels to 0
				GPIBPixDcsChan *chan = dynamic_cast<GPIBPixDcsChan*>(*IT);
				chan->SetParam("currentlim");
				chan->SetParam("voltage", 0.);
			}
			m_gpibDev->setState(PGD_ON);	//turn device on
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){	//ramp all channels
				GPIBPixDcsChan *chan = dynamic_cast<GPIBPixDcsChan*>(*IT);
				chan->rampVoltage(chan->m_nomVolts);
			}
			return;
		}
		if(m_gpibDev->getStatus()==PGD_ON && new_state=="UPDATE" ){
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
				GPIBPixDcsChan *chan = dynamic_cast<GPIBPixDcsChan*>(*IT);
				chan->SetParam("currentlim");
				chan->rampVoltage(chan->m_nomVolts);
				// NEVER do that! m_nomVolts contains the nominal settings, which should'n be altered by what is on the device
				//chan->m_nomVolts = chan->ReadParam("voltage");	//update with actual reached voltage
			}
			return;
		}
	}

	if(m_devType==PULSER){
		if (new_state=="ON")
			m_gpibDev->setState(PGD_ON);
		if (new_state=="OFF")
			m_gpibDev->setState(PGD_OFF);
		if (new_state=="UPDATE"){
			for(vector<PixDcsChan*>::iterator it=m_channels.begin(); it!=m_channels.end(); ++it){
				GPIBPixDcsChan *tChan = dynamic_cast<GPIBPixDcsChan*>(*it);
				tChan->SetParam("voltage", (double) tChan->m_nomVolts);
				tChan->SetParam("frequency", (double) tChan->m_frequency);
				tChan->SetParam("nbursts", (double) tChan->m_nBurstPulses);
				tChan->SetParam("voltageoffset", (double) tChan->m_nomVoltOffset);
				tChan->SetParam("waveform", (double) tChan->m_waveFormType);
			}
		}
	}
}
void GPIBPixDcs::configInit(){
  // Create the Config object
  m_conf = new Config("GPIBPixDcs"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown","name of PixDcs device", true);
  conf["general"].addInt("board_ID", m_boardID, 0, "ID of GPIB interface", true);
  conf["general"].addInt("GPIB_Chan", m_gpibChan, -1, "GPIB channel number of this device", true);
  conf["general"].addList("DeviceType", (int &)m_devType, SUPPLY, m_typeMap, "Type of device", false); 
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", false);
  conf.reset();

}
void GPIBPixDcs::initHW(){
  if(m_gpibDev==0){
    m_gpibDev = new PixGPIBDevice(m_boardID, m_gpibChan, 0, (m_devType==PixDcs::ADCMETER));
  } else{
    SetState("OFF");
    m_gpibDev->resetDevice();
  }
  if(m_devType==SUPPLY){
    for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
      GPIBPixDcsChan *chan = dynamic_cast<GPIBPixDcsChan*>(*IT);
      chan->SetParam("ovp_voltage");
      chan->SetParam("ovp_enabled");
      chan->SetParam("ocp_current");
      chan->SetParam("ocp_enabled");
      chan->SetParam("remote_sensing");
      chan->SetParam("currentlim");
    }
  }
}
bool GPIBPixDcs::sendCommand(std::string &txt, bool readResp){
  if(m_gpibDev==0) return false;
  char command[120], answer[120];
#ifdef WIN32
  strncpy_s(command, txt.c_str(), 120);
#else
  strncpy(command, txt.c_str(), 120);
#endif
  bool retval = m_gpibDev->writeDevice(command);
  if(readResp){
    retval |= m_gpibDev->readDevice(answer);
    txt = answer;
  }
  return retval;
}
void GPIBPixDcs::ReadError(std::string &errTxt){
  errTxt="";
  if(m_gpibDev!=0) m_gpibDev->getError(errTxt);
  return;
}
