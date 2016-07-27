/*------------------------------------------------------------
 *  USBPixDcx.cxx
 *  Version:  0.1.USBsystem
 *  Created:  23 February 2009
 *  Author:   Joern Grosse-Knetter
 *
 *  Functional interface to the USB system for communication
 *  with the ADCs on the USB system
 *------------------------------------------------------------*/

#include "PixDcs/USBI3PixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixController/USBI3PixController.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include <USBPixDCSI3.h>

using namespace PixLib;

USBI3PixDcsChan::USBI3PixDcsChan(PixDcs *parent, DBInquire *dbInquire) : PixDcsChan(parent, dbInquire){
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
USBI3PixDcsChan::USBI3PixDcsChan(USBI3PixDcsChan &chan_in) : PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
}
USBI3PixDcsChan::~USBI3PixDcsChan(){
  //delete m_conf; // taken care of by PixDcs's config
}
void USBI3PixDcsChan::configInit(){
  // Create the Config object
  m_conf = new Config("USBI3PixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  std::map<std::string, int> configMap;
  configMap["DVDD"] = DVDD;
  configMap["AVDD"] = AVDD;
  configMap["LVDD"] = LVDD;
  conf["general"].addList("ChannelDescr", (int &)m_channelDescr, 1, configMap,
			  "USB-ADC-Type of channel", true);

  conf.reset();

  m_rtype = VCBOTH; // must never change this, so not in config!

}

double USBI3PixDcsChan::ReadParam(std::string measType) {
  Config &conf = *m_conf;
  USBI3PixDcs *parent = dynamic_cast<USBI3PixDcs*>(m_parent);
  if(parent->m_USBADC==0) throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since not yet initialised");
  if(measType=="voltage"){
    switch(m_channelDescr){
    case DVDD:
      return parent->m_USBADC->GetDVDDVoltage();
    case AVDD:
      return parent->m_USBADC->GetAVDDVoltage();
    case LVDD:
      return parent->m_USBADC->GetLVDDVoltage();
    default:
      throw PixDcsExc(PixDcsExc::WARNING, "unknown channel or measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
    }
  }else if(measType=="current"){
    switch(m_channelDescr){
    case DVDD:
      return parent->m_USBADC->GetDVDDCurrent();
    case AVDD:
      return parent->m_USBADC->GetAVDDCurrent();
    default:
      throw PixDcsExc(PixDcsExc::WARNING, "unknown channel or measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
    }
  }else 
    throw PixDcsExc(PixDcsExc::WARNING, "unknown channel or measurement type requested: "+ 
		       ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);

}

void USBI3PixDcsChan::SetParam(string, double) {
  // Set resource with string id to a double value.
}

std::string USBI3PixDcsChan::ReadState(std::string){
  return "unknown";
}
void USBI3PixDcsChan::SetState(std::string /*new_state*/){
  // nothing to set here, USBI3PixDcsChan contains just ADCs
  return;
}

USBI3PixDcs::USBI3PixDcs(DBInquire *dbInquire, void *interface)
  : PixDcs(dbInquire, interface){

  m_USBPC = (USBI3PixController*)interface; //dynamic_cast<PixUSB>(interface);
  if(m_USBPC==0) throw PixDcsExc(PixDcsExc::FATAL, 
  				"interface provided to PixDcs::make is either NULL or not USB although USB was requested");
  
  configInit();
  m_conf->read(dbInquire);

  m_problemInit = false;

  if(m_devType!=ADCMETER)
    throw PixDcsExc(PixDcsExc::FATAL, "USBI3PixDcs can at the moment only be used as type meter, different type requested though");

  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for DCS channel inquire
    if((*it)->getName() == "PixDcsChan") {
      USBI3PixDcsChan *uch = new USBI3PixDcsChan(this, *it);
      m_channels.push_back(uch);
      m_conf->addConfig(uch->m_conf);
      // set default name and channel ID if none given yet
      int chID = (int)m_channels.size()-1;
      std::stringstream a;
      a << chID;
      if(uch->m_name=="unknown"){
	uch->m_name = m_name+"_Ch"+a.str();
	uch->m_channelDescr = USBI3PixDcsChan::DVDD;
      }
    }    
  }

  m_USBADC = 0;
}
USBI3PixDcs::~USBI3PixDcs(){
  delete m_conf;
  for(vector<PixDcsChan*>::iterator it = m_channels.begin(); it!=m_channels.end(); it++){
    PixDcsChan *pdc = *it;
    delete pdc;
  }
  m_channels.clear();
  delete m_USBADC;
}
std::string USBI3PixDcs::ReadState(std::string) {
  // Read a string value from DDC, returning a string.
  std::string ret="uninit.";
  if(m_USBADC!=0) ret = "OK";
  else if(m_problemInit) ret = "ERROR";
  return ret;
}
void USBI3PixDcs::SetState(std::string){
  throw PixDcsExc(PixDcsExc::WARNING, "this device is read-only, can't process request for change in state");
}
void USBI3PixDcs::configInit(){
  // Create the Config object
  m_conf = new Config("USBI3PixDcs"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown",
		  "name of PixDcs device", true);
  conf["general"].addList("DeviceType", (int &)m_devType, ADCMETER, m_typeMap, "Type of device", false); 
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", true);
  conf.reset();
}
void USBI3PixDcs::initHW(){
  if(m_USBADC==0){
    if(m_USBPC->getUsbHandle()!=0){
      m_USBADC = new USBPixDCSI3(m_USBPC->getUsbHandle());
      m_problemInit = false;
    }else{
      m_problemInit = true;
      throw PixDcsExc(PixDcsExc::ERROR, "USB interface must be initialised before DCS object can be set up");
    }
  }
}
void USBI3PixDcs::ReadError(std::string &errTxt){
  errTxt = ""; // to be implemented
  return;
}
