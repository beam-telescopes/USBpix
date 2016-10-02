/*------------------------------------------------------------
 *  USBPixDcx.cxx
 *  Version:  0.1.USBsystem
 *  Created:  23 February 2009
 *  Author:   Joern Grosse-Knetter
 *
 *  Functional interface to the USB system for communication
 *  with the ADCs on the USB system
 *------------------------------------------------------------*/

#include <USBpix.h>

#include "PixDcs/USB_PixDcs.h"
#include "PixDcs/USBBIPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixController/USBPixController.h"
#include "PixController/USB3PixController.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include <USBPixI4DCS.h>

#define UDCS_DEB false

using namespace PixLib;

USBPixDcsChan::USBPixDcsChan(PixDcs *parent, DBInquire *dbInquire) : PixDcsChan(parent, dbInquire){
  configInit();
  if(dbInquire!=0){
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
  if(m_rtype==PixDcsChan::FE_GADC){
    (*m_conf)["general"]["AdcType"].setVisible(true);
    (*m_conf)["general"]["FeIndex"].setVisible(true);
  }
  m_chanIsOn = false;
}
USBPixDcsChan::USBPixDcsChan(USBPixDcsChan &chan_in) : PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
  if(m_rtype==PixDcsChan::FE_GADC){
    (*m_conf)["general"]["AdcType"].setVisible(true);
    (*m_conf)["general"]["FeIndex"].setVisible(true);
  }
  m_chanIsOn = false;
}
USBPixDcsChan::~USBPixDcsChan(){
  //delete m_conf; // taken care of by PixDcs's config
}
void USBPixDcsChan::configInit(){
  if(UDCS_DEB) cout << "USBPixDcsChan::configInit" << endl;
  // Create the Config object
  m_conf = new Config("USBPixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);

  if(m_parent->getDevType()==PixDcs::SUPPLY){
    //    if(dynamic_cast<USBBIPixDcs*>(this)==0){ // don't set up for inherited class
      std::map<std::string, int> configMap;
      configMap["VDDA1"] = VDDA1;
      configMap["VDDA2"] = VDDA2;
      configMap["VDDD1"] = VDDD1;
      configMap["VDDD2"] = VDDD2;
      conf["general"].addList("ChannelDescr", m_channelDescr, VDDA1, configMap,
			      "USB-ADC-Type of channel", true);
      //    }
    conf.addGroup("settings");
    conf["settings"].addFloat("NomVolts", m_nomVolts, 0.0, "Nominal voltage (in V) on this channel", true);
    m_rtype = VCBOTH; // must never change this, so not in config!
  } else{
    std::map<std::string, int> configMap;
    configMap["Temp"] = PixDcsChan::TEMP;
    configMap["FE ADC"] = PixDcsChan::FE_GADC;
    conf["general"].addList("ChannelDescr", (int &)m_rtype, 0, configMap,
			    "USB-ADC-Type of channel", true);
    std::map<std::string, int> typeMap;
    typeMap["Temperature"] = 0;
    typeMap["GADC reference voltage"] = 1;  
    typeMap["Analog GND"] = 2;  
    typeMap["Analog MUX output"] = 3;
    typeMap["Analog regulator current"] = 4;
    typeMap["10 bit DAC output (Pulser input)"] = 5;
    typeMap["1/2 regulated analog voltage"] = 6;  
    typeMap["Leakage current"] = 7;  
    conf["general"].addList("AdcType", m_feAdcType, 0, typeMap,
			    "FE ADC channel to read (only for FE ADC type)", false);
    conf["general"].addInt("FeIndex", m_feIndex, 0, "Index of FE to be read", false);
  }

  conf.reset();
}

double USBPixDcsChan::ReadParam(std::string measType) {
  if(UDCS_DEB) cout << "USBPixDcsChan::ReadParam" << endl;
  Config &conf = *m_conf;
  USBPixDcs *parent = dynamic_cast<USBPixDcs*>(m_parent);
  if(parent->m_USBADC==0)
    throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since not yet initialised");
  if(parent->getUsbHandle()==0){
    throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since USB link not available");
  }
  parent->m_USBADC->UpdateMeasurements();
  if(measType=="voltage"){
    if(m_parent->getDevType()==PixDcs::SUPPLY && m_channelDescr<MAX_SUPPLY_CHANNEL){
      SupplyChannel *sc = parent->m_USBADC->PSU[m_channelDescr];
      double retval = sc->GetVoltage();
      return retval;
    } else{
      throw PixDcsExc(PixDcsExc::WARNING, "unknown channel or measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
    }
  }else if(measType=="current"){
    if(m_parent->getDevType()==PixDcs::SUPPLY && m_channelDescr<MAX_SUPPLY_CHANNEL){
      SupplyChannel *sc = parent->m_USBADC->PSU[m_channelDescr];
      double retval = sc->GetCurrent();
      return retval;
    } else{
      throw PixDcsExc(PixDcsExc::WARNING, "unknown channel or measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
    }
  }else if(measType=="temperature"){
    if(m_parent->getDevType()==PixDcs::ADCMETER && m_rtype==PixDcsChan::TEMP){
      double retval = parent->m_USBADC->GetNTCTemperature();
      return retval;
    }else if(m_parent->getDevType()==PixDcs::SUPPLY && m_rtype==PixDcsChan::VCT){
      SupplyChannel *sc = parent->m_USBADC->PSU[m_channelDescr];
      double retval = sc->GetNTCTemperature();
      return retval;
    } else{
      throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
    }
  }else if(measType=="FE GADC"){
    if(m_parent->getDevType()==PixDcs::ADCMETER && m_rtype==PixDcsChan::FE_GADC){
      std::vector<int> GADCvalues;
      parent->m_USBPC->readGADC(m_feAdcType, GADCvalues, m_feIndex);
      return (double)GADCvalues[0];
    } else{
      throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
    }
  }else {
    throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		       ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
  }
}

void USBPixDcsChan::SetParam(string varType) {
  // Set resource with string id to default value.
  SetParam(varType, (double)m_nomVolts);
}
void USBPixDcsChan::SetParam(string varType, double value) {
  if(UDCS_DEB) cout << "USBPixDcsChan::SetParam" << endl;
  // Set resource with string id to a double value.
  Config &conf = *m_conf;
  USBPixDcs *parent = dynamic_cast<USBPixDcs*>(m_parent);
  if(parent->m_USBADC==0)
    throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since not yet initialised");
  if(parent->getUsbHandle()==0){
    throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since USB link not available");
  }
  if(parent->getDevType()!=PixDcs::SUPPLY) return;
  if(varType=="voltage"){
    if(m_channelDescr<MAX_SUPPLY_CHANNEL){
      SupplyChannel *sc = parent->m_USBADC->PSU[m_channelDescr];
      sc->SetVoltage(value);
    }else{
      throw PixDcsExc(PixDcsExc::WARNING, "unknown channel type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue());
    }
  }else{
    throw PixDcsExc(PixDcsExc::ERROR, "can only process request for voltage setting: "+varType);
  }
}

std::string USBPixDcsChan::ReadState(std::string){
  if(UDCS_DEB) cout << "USBPixDcsChan::ReadState" << endl;
  USBPixDcs *parent = dynamic_cast<USBPixDcs*>(m_parent);
  if(parent->m_USBADC==0){
    //throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since not yet initialised");
    return "unknown";
  } 
  if(parent->getUsbHandle()==0) return "unknown";

  if(parent->m_devType!=PixDcs::SUPPLY) return "unknown";
  else                                  return m_chanIsOn?"ON":"OFF";
}
void USBPixDcsChan::SetState(std::string new_state){
  if(UDCS_DEB) cout << "USBPixDcsChan::SetState" << endl;
  Config &conf = *m_conf;
  USBPixDcs *parent = dynamic_cast<USBPixDcs*>(m_parent);
  if(parent->m_USBADC==0)
    throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since not yet initialised");
  if(parent->getUsbHandle()==0){
    throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since USB link not available");
  }
  if(parent->m_devType!=PixDcs::SUPPLY) return;

  if(m_channelDescr<MAX_SUPPLY_CHANNEL){
    if(m_chanIsOn && new_state=="OFF") SetParam("voltage", 0.);
    m_chanIsOn = (new_state=="ON");
    parent->getUsbHandle();
    if(m_chanIsOn) SetParam("voltage", m_nomVolts);
	SupplyChannel *sc = parent->m_USBADC->PSU[m_channelDescr];
	sc->Enable(m_chanIsOn);
    //USBpix *reg = parent->m_USBPC->getUSBpix();
    //reg->EnablePowerChannel(m_chanIsOn, m_channelDescr);
  }else{
    throw PixDcsExc(PixDcsExc::WARNING, "unknown channel type requested: "+ 
		    ((ConfList&)conf["general"]["ChannelDescr"]).sValue());}
}
USBPixDcs::USBPixDcs(DBInquire *dbInquire, void *interface)
  : PixDcs(dbInquire, interface){

  m_USBPC = (PixController*)interface; //dynamic_cast<PixUSB>(interface);
  if(m_USBPC==0) throw PixDcsExc(PixDcsExc::FATAL, 
  				"interface provided to PixDcs::make is either NULL or not USB although USB was requested");
  
  configInit();
  m_problemInit = false;

  if(dbInquire!=0){

    m_conf->read(dbInquire);
    
    std::string dcsName="";
    fieldIterator f = dbInquire->findField("ActualClassName");
    if(f!=dbInquire->fieldEnd()) dbInquire->getDB()->DBProcess(f,READ, dcsName);
    if(dcsName=="USBPixDcs"){ // if called from inherited classes, don't create own-type channels
      for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
	// Look for DCS channel inquire
	if((*it)->getName() == "PixDcsChan") {
	  USBPixDcsChan *uch = new USBPixDcsChan(this, *it);
	  if(UDCS_DEB) cout << "created USBPixDcsChan " << uch->name()<< endl;
	  m_channels.push_back(uch);
	  m_conf->addConfig(uch->m_conf);
	  // set default name and channel ID if none given yet
	  int chID = (int)m_channels.size()-1;
	  std::stringstream a;
	  a << chID;
	  if(uch->m_name=="unknown"){
	    uch->m_name = m_name+"_Ch"+a.str();
	    //uch->m_channelDescr = VDDA1;
	  }
	}    
      }
    }
  }

  m_ctrlName = m_USBPC->getModGroup().getName();
  m_USBADC = 0;
}
USBPixDcs::~USBPixDcs(){
  delete m_conf;
  for(vector<PixDcsChan*>::iterator it = m_channels.begin(); it!=m_channels.end(); it++){
    PixDcsChan *pdc = *it;
    delete pdc;
  }
  m_channels.clear();
  USBPixSTDDCS *adc = dynamic_cast<USBPixSTDDCS*>(m_USBADC);
  delete adc;
}
std::string USBPixDcs::ReadState(std::string req) {
  // check if handle of USBPixController is still valid, otherwise go into uninit state
  // must distinguis between this class and inheriting classes for deletion,
  // so keep actual functiuon in ReadState2
  if(getUsbHandle()==0){
    delete (USBPixSTDDCS*)m_USBADC;
    m_USBADC=0;
  }
  return ReadState2(req);
}
std::string USBPixDcs::ReadState2(std::string req) {
  if(UDCS_DEB) cout << "USBPixDcs::ReadState" << endl;
  // Read a string value from DDC, returning a string.
  std::string ret="uninit.";
  if(m_USBADC!=0){
    if(m_devType==SUPPLY){ // have on/off-states controller per channel
      int n_On=0;
      int n_Off=0;
      for(vector<PixDcsChan*>::iterator IT=chanBegin(); IT!=chanEnd(); IT++){
	USBPixDcsChan *ch = dynamic_cast<USBPixDcsChan*>(*IT);
	std::string chState = ch->ReadState(req);
	if(chState=="ON" || chState=="LIM")  n_On++;
	if(chState=="OFF") n_Off++;
      }
      if(n_On==nchan())        ret = "ALL ON";
      else if(n_Off==nchan())  ret = "ALL OFF";
      else                     ret = "CHAN";
    } else
      ret = "OK";
  } else if(m_problemInit) ret = "ERROR";
  return ret;
}
void USBPixDcs::SetState(std::string new_state){
  if(UDCS_DEB) cout << "USBPixDcs::SetState" << endl;
  if(new_state!="OFF" && new_state!="ON")
    throw PixDcsExc(PixDcsExc::ERROR, "unknown request for change in state (should be ON or OFF): "+new_state);
  if(m_USBADC==0) return; // can't set anything without init. hardware

  if(m_devType!=SUPPLY) return; // can only set this for supplies
  if(new_state=="ON"){ // turn on in order 0,1,2,...
    for(vector<PixDcsChan*>::iterator IT=chanBegin(); IT!=chanEnd(); IT++){
      USBPixDcsChan *ch = dynamic_cast<USBPixDcsChan*>(*IT);
      ch->SetState(new_state);
    }
  } else { // turn off in reverse order
    for(int i = ((int)m_channels.size())-1; i>=0; i--){
      USBPixDcsChan *ch = dynamic_cast<USBPixDcsChan*>(m_channels[i]);
      ch->SetState(new_state);
    }
  }
}
void USBPixDcs::configInit(){
  if(UDCS_DEB) cout << "USBPixDcs::configInit" << endl;
  // Create the Config object
  m_conf = new Config("USBPixDcs"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown",
		  "name of PixDcs device", true);
  conf["general"].addList("DeviceType", (int &)m_devType, SUPPLY, m_typeMap, "Type of device", false); 
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", false);
  conf["general"].addString("USBPixController", m_ctrlName, "???", "Name of PixController to which adapter is attached", false);
  conf.reset();
}
void USBPixDcs::initHW(){
  if(m_USBADC!=0) delete (USBPixSTDDCS*)m_USBADC; m_USBADC=0;
  if(UDCS_DEB) cout << "USBPixDcs::initHW: Retrieving board handle from controller " << m_USBPC->getModGroup().getName() << endl;
  SiUSBDevice *handle = (SiUSBDevice*)getUsbHandle();
  if(handle!=0){
    m_USBADC = new USBPixSTDDCS(handle);
    m_USBADC->Init();
    m_problemInit = false;
    SetState("OFF"); // init. state should be off
    stringstream c;
    c << m_USBADC->GetId();
    if(UDCS_DEB) cout << "DCS device " << m_name << " has USB board ID " << handle->GetId() << " and interface board ID " << c.str() << endl;
  }else{
    m_problemInit = true;
    throw PixDcsExc(PixDcsExc::ERROR, "USB interface must be initialised before DCS object can be set up");
  }
}
void USBPixDcs::ReadError(std::string &errTxt){
  errTxt = ""; // to be implemented
  return;
}
void* USBPixDcs::getUsbHandle(){
  USB3PixController *u3pc = dynamic_cast<USB3PixController*>(m_USBPC);
  USBPixController *upc = dynamic_cast<USBPixController*>(m_USBPC);
  if(u3pc!=0) return (void*)u3pc->getUsbHandle();
  else if(upc!=0) return (void*)upc->getUsbHandle();
  else return 0;
}
