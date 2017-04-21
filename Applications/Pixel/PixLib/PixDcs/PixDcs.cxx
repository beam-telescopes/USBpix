/*------------------------------------------------------------
 *  PixDcx.cxx
 *  Version:  0.1.USBsystem
 *  Created:  19 February 2009
 *  Author:   Joern Grosse-Knetter
 *
 *  Functional interface to the USB system for communication
 *  with the USB ADCs or GPIB devices (anything else?)
 *------------------------------------------------------------*/

#include "PixDcs.h"
#ifdef HAVE_GPIB
#include "GPIBPixDcs.h"
#endif
#ifdef HAVE_GOEUSB
#include "GoeUSBPixDcs.h"
#endif
#include "USB_PixDcs.h"
#include "USBBIPixDcs.h"
#include "RS232PixDcs.h"
#include "USBGpacPixDcs.h"
#include "ProberPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"

using namespace PixLib;

PixDcs::PixDcs(DBInquire *dbInquire, void *interface) : 
  m_dbInquire(dbInquire), m_interface(interface), m_name("unknown"), m_decName("")
{
  m_typeMap["SUPPLY"] = SUPPLY;
  m_typeMap["ADC-METER"] = ADCMETER;
  m_typeMap["PULSER"] = PULSER;
  m_typeMap["POSITION"] = POSITION;
  m_typeMap["CHILLER"] = CHILLER;
  m_typeMap["VOLTAGE-SOURCE"] = VOLTAGE_SOURCE;
  m_typeMap["CURRENT-SOURCE"] = CURRENT_SOURCE; 
  if(dbInquire!=0) m_decName = dbInquire->getDecName();
}

PixDcs* PixDcs::make(DBInquire *dbInquire, void *interface, std::string type) 
{
  
  PixDcs *dcs = 0;
  
  if (type == "USBPixDcs") {
    //cout << "PixDcs* PixDcs::make with type std." << endl;
    dcs = new USBPixDcs(dbInquire, interface);
  }
  else if (type == "USBBIPixDcs") {
    //cout << "PixDcs* PixDcs::make with type BURN-IN" << endl;
    dcs = new USBBIPixDcs(dbInquire, interface);
  }
  else if (type == "USBGpacPixDcs") {
    dcs = new USBGpacPixDcs(dbInquire, interface);
  }
#ifdef HAVE_GPIB
  else if (type == "GPIBPixDcs") {
    dcs = new GPIBPixDcs(dbInquire, interface);
  }
#endif
#ifdef HAVE_GOEUSB
  else if (type == "GoeUSBPixDcs"){
    dcs = new GoeUSBPixDcs(dbInquire, interface);
  }
#endif
  else if (type == "RS232PixDcs") {
    dcs = new RS232PixDcs(dbInquire, interface);
  }
  else if (type == "ProberPixDcs") {
    dcs = new ProberPixDcs(dbInquire, interface);
  }
  else
    throw PixDcsExc(PixDcsExc::FATAL, "PixDcs::make: request for unknown class of type "+type);


  return dcs;
}
void PixDcs::listTypes(std::vector<std::string> &list)
{
  list.push_back("USBPixDcs");
  list.push_back("USBBIPixDcs");
  list.push_back("USBGpacPixDcs");
  list.push_back("RS232PixDcs");
  list.push_back("ProberPixDcs");
#ifdef HAVE_GPIB
  list.push_back("GPIBPixDcs");
#endif
#ifdef HAVE_GOEUSB
  list.push_back("GoeUSBPixDcs");
#endif
  return;
}

PixDcsChan* PixDcs::getChan(int ID){
  if(ID>=0 && ID<(int)m_channels.size())
    return m_channels[ID];
  else
    return 0;
}

PixDcsChan* PixDcs::copyChan(PixDcsChan *chan){
  USBPixDcsChan   *uc = dynamic_cast<USBPixDcsChan*>(chan);
  USBBIPixDcsChan *bc = dynamic_cast<USBBIPixDcsChan*>(chan);
  USBGpacSupplyPixDcsChan *gsc = dynamic_cast<USBGpacSupplyPixDcsChan*>(chan);
  USBGpacVoltageSourcePixDcsChan *gvc = dynamic_cast<USBGpacVoltageSourcePixDcsChan*>(chan);
  USBGpacCurrentSourcePixDcsChan *gcc = dynamic_cast<USBGpacCurrentSourcePixDcsChan*>(chan);
  USBGpacInjectPixDcsChan *gic = dynamic_cast<USBGpacInjectPixDcsChan*>(chan);
  USBGpacAuxAdcPixDcsChan *gac = dynamic_cast<USBGpacAuxAdcPixDcsChan*>(chan);
  RS232PixDcsChan *rc = dynamic_cast<RS232PixDcsChan*>(chan);
  ProberPixDcsChan *pc = dynamic_cast<ProberPixDcsChan*>(chan);
#ifdef HAVE_GPIB
  GPIBPixDcsChan  *gc = dynamic_cast<GPIBPixDcsChan*>(chan);
#endif
#ifdef HAVE_GOEUSB
  GoeUSBPixDcsChan  *xc = dynamic_cast<GoeUSBPixDcsChan*>(chan);
#endif
  if(uc!=0)
    return (PixDcsChan*)(new USBPixDcsChan(*uc));
  else if(bc!=0)
    return (PixDcsChan*)(new USBBIPixDcsChan(*bc));
  else if(gsc!=0)
    return (PixDcsChan*)(new USBGpacSupplyPixDcsChan(*gsc));
  else if(gvc!=0)
    return (PixDcsChan*)(new USBGpacVoltageSourcePixDcsChan(*gvc));
  else if(gcc!=0)
    return (PixDcsChan*)(new USBGpacCurrentSourcePixDcsChan(*gcc));
  else if(gic!=0)
    return (PixDcsChan*)(new USBGpacInjectPixDcsChan(*gic));
  else if(gac!=0)
    return (PixDcsChan*)(new USBGpacAuxAdcPixDcsChan(*gac));
  else if(rc!=0)
    return (PixDcsChan*)(new RS232PixDcsChan(*rc));
  else if(pc!=0)
    return (PixDcsChan*)(new ProberPixDcsChan(*pc));
#ifdef HAVE_GPIB
  else if(gc!=0)
    return (PixDcsChan*)(new GPIBPixDcsChan(*gc));
#endif
#ifdef HAVE_GOEUSB
  else if(xc!=0)
    return (PixDcsChan*)(new GoeUSBPixDcsChan(*xc));
#endif
	    else
	      return 0;
}
	    
void DummyPixDcs::configInit(){
  // Create the Config object
  m_conf = new Config("USBPixDcs"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown",
		  "name of PixDcs device", true);
  m_name = "unknown";
  conf["general"].addList("DeviceType", (int &)m_devType, SUPPLY, m_typeMap, "Type of device", true); 
  m_devType = SUPPLY;
}

