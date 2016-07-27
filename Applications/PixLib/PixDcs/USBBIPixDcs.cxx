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

#include "PixDcs/USBBIPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixController/USBPixController.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include <USBPixI4DCS.h>

#define UDCS_DEB false

using namespace PixLib;

USBBIPixDcsChan::USBBIPixDcsChan(PixDcs *parent, DBInquire *dbInquire) : USBPixDcsChan(parent, dbInquire){
  configInit();
  m_conf->read(dbInquire);
}
USBBIPixDcsChan::USBBIPixDcsChan(USBBIPixDcsChan &chan_in) : USBPixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
}
// USBBIPixDcsChan::~USBBIPixDcsChan(){
//   //delete m_conf; // taken care of by PixDcs's config
// }
void USBBIPixDcsChan::configInit(){
  // NB: only create what is not taken care of in USBPixDcsChan!
  if(UDCS_DEB) cout << "USBBIPixDcsChan::configInit" << endl;
  if(m_parent->getDevType()==PixDcs::SUPPLY){
    std::map<std::string, int> &configMap = ((ConfList&)(*m_conf)["general"]["ChannelDescr"]).symbols();
    configMap.clear();
    configMap["CH1"] = CH1;
    configMap["CH2"] = CH2;
    configMap["CH3"] = CH3;
    configMap["CH4"] = CH4;
  }

  m_conf->reset();

  if(m_parent->getDevType()==PixDcs::SUPPLY)
    m_rtype = VCT; // must never change this, so not in config!
}

std::string USBBIPixDcsChan::ReadState(std::string){
  if(UDCS_DEB) cout << "USBBIPixDcsChan::ReadState" << endl;
  USBPixDcs *parent = dynamic_cast<USBPixDcs*>(m_parent);
  if(parent->m_USBADC==0){
    //throw PixDcsExc(PixDcsExc::WARNING, "can't read from device since not yet initialised");
    return "unknown";
  } 
  if(parent->getUsbHandle()==0) return "unknown";

  if(parent->m_devType!=PixDcs::SUPPLY) return "unknown";
  else{
    int limBits = 0;//parent->m_USBPC->getUSBpix()->ReadRegister(CS_BURN_IN_OC_STATE);
    int chbit = 1<<((int)m_channelDescr);
    if(UDCS_DEB) cout << "USBBIPixDcsChan::ReadState has lim. bits " << limBits << " and chbit " << chbit << std::endl;
    if(limBits<0) limBits=3;
    if(limBits&chbit && m_chanIsOn) return "LIM";

    // SW temperature limit
    float temp = (float)ReadParam("temperature");
    if(temp > ((USBBIPixDcs*)parent)->getTempLim()){
      SetState("OFF");
      return "LIM";
    }

    // current and temp. lim. are OK, return ON/OFF state
    return m_chanIsOn?"ON":"OFF";
  }
}

USBBIPixDcs::USBBIPixDcs(DBInquire *dbInquire, void *interface)
  : USBPixDcs(dbInquire, interface){

  // needed to add extra cfg. items on top of USBPixDcs's config.
  configInit();
  m_conf->read(dbInquire);

  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for DCS channel inquire
    if((*it)->getName() == "PixDcsChan") {
      USBBIPixDcsChan *uch = new USBBIPixDcsChan(this, *it);
      if(UDCS_DEB) cout << "created USBBIPixDcsChan " << uch->name()<< endl;
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
USBBIPixDcs::~USBBIPixDcs(){
  USBPixBIDCS *adc = dynamic_cast<USBPixBIDCS*>(m_USBADC);
  delete adc;
  m_USBADC = 0;
}
void USBBIPixDcs::configInit(){
  if(UDCS_DEB) cout << "USBBIPixDcs::configInit" << endl;
  // NB: only create what is not taken care of in USBPixDcsChan!
  if(m_devType==SUPPLY){
    (*m_conf)["general"].addFloat("CurrLim", m_currLim, 1.0, "Current limit (in A) on this device", true);
    (*m_conf)["general"].addFloat("TempLim", m_tempLim, 100.0, "Temperature SW limit (in °C) on this device", true);
    m_conf->reset();
  }
}
std::string USBBIPixDcs::ReadState(std::string req) {
  // check if handle of USBPixController is still valid, otherwise go into uninit state
  // must distinguis between this class and inheriting classes for deletion,
  // so keep actual functiuon in ReadState2
  if(getUsbHandle()==0){
    delete (USBPixBIDCS*)m_USBADC;
    m_USBADC=0;
  }
  return USBPixDcs::ReadState2(req);
}
void USBBIPixDcs::SetState(std::string new_state){
  if(UDCS_DEB) cout << "USBBIPixDcs::SetState" << endl;
  if(new_state=="OFF" || new_state=="ON"){
    USBPixDcs::SetState(new_state);
//   }else if(new_state=="reset"){
//     if(UDCS_DEB) cout << "USBBIPixDcs::SetState reset" << endl; // just for testing
  }else{
    throw PixDcsExc(PixDcsExc::ERROR, "unknown request for change in state (should be ON or OFF): "+new_state);
  }
}
void USBBIPixDcs::initHW(){
  if(m_USBADC!=0) delete (USBPixBIDCS*)m_USBADC; m_USBADC=0;
  if(UDCS_DEB) cout << "USBBIPixDcs::initHW: Retrieving board handle from controller " << m_USBPC->getModGroup().getName() << endl;
  SiUSBDevice *handle = (SiUSBDevice*)getUsbHandle();
  if(handle!=0){
    m_USBADC = new USBPixBIDCS(handle);
    m_USBADC->Init();
    m_problemInit = false;
    SetState("OFF"); // init. state should be off
    stringstream c;
    c << m_USBADC->GetId();
    if(UDCS_DEB) cout << "DCS device " << m_name << " has USB board ID " << handle->GetId() << " and interface board ID " << c.str() << endl;
    // set current limit here, but perhaps there is a better place
    if(m_devType==SUPPLY){
      ((USBPixBIDCS*)m_USBADC)->SetCurrentLimit(m_currLim);
      if(UDCS_DEB) cout << "Set current limit to " << m_currLim << endl;
    }
  }else{
    m_problemInit = true;
    throw PixDcsExc(PixDcsExc::ERROR, "USB interface must be initialised before DCS object can be set up");
  }
}
