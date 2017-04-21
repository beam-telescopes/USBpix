#ifdef HAVE_GOEUSB

#include "GoeUSBPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include <usb.h>
#include "PixConfDBInterface/PixConfDBInterface.h"
#include <sstream>
#include <math.h>

#define EP2_FIFO_Size 64
#define TimeOut 2000 // ms
#define USB_VendorRequestCode (1<<6)

using namespace PixLib;

GoeUSBPixDcsChan::GoeUSBPixDcsChan(PixDcs *parent, DBInquire *dbInquire) : PixDcsChan(parent, dbInquire){
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
GoeUSBPixDcsChan::~GoeUSBPixDcsChan(){}
double GoeUSBPixDcsChan::ReadParam(std::string measType){
  Config &conf = *m_conf;
  GoeUSBPixDcs *parent = dynamic_cast<GoeUSBPixDcs*>(m_parent);

  if((measType=="voltage" && m_channelDescr!=1) || // MONDAC/CAPMEASURE don't read voltages
     (measType=="current" && m_channelDescr==1))   // VCAL doesn't read currents
      throw PixDcsExc(PixDcsExc::WARNING, "unknown channel or measurement type requested: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
  unsigned char buf[EP2_FIFO_Size];
  unsigned char *b;
  unsigned int m;
  unsigned int c = 0;
  unsigned short samples = (unsigned short)m_samples;
  double avgval = 0.;

  if (usb_control_msg(parent->m_handle, USB_VendorRequestCode, 1, m_channelDescr, (int) samples, NULL, 0, TimeOut) < 0)
    return -990.;
  if (usb_bulk_read(parent->m_handle, 0x82, (char*) buf, 2*samples, TimeOut) < (samples * 2))
    //    throw PixDcsExc(PixDcsExc::ERROR,"USB_bulk_read error!"); // should never occur
    return 999.;
  b = buf;
  while (samples--){
    unsigned int k;
    k = (unsigned int) *b++;
    k += (unsigned int) *b++ * 256;
    avgval += (double) k;
    //printf("%d %d: %u - %lf\n",(int)samples,m_channelDescr, k, avgval);
  }
  if(m_samples>0) avgval /= (double)m_samples;
  avgval -= m_offset; // subtract ADC-offset
  avgval *= m_grad;   // convert ADC counts to measurement unit
  return avgval;
}
void GoeUSBPixDcsChan::SetParam(std::string,double){}
void GoeUSBPixDcsChan::configInit(){
  // Create the Config object
  m_conf = new Config("GoeUSBPixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  std::map<std::string, int> configMap;
  configMap["VCAL"] = 1;
  configMap["CAPMEASURE"] = 2;
  configMap["MONDAC"] = 3;
  conf["general"].addList("ChannelDescr", (int &)m_channelDescr, 1, configMap,
			  "ADC-channel", true);
  // calibration constants
  conf.addGroup("calib");
  conf["calib"].addInt("NoSamples",m_samples, 3, "No. of samples taken per reading", true);
  conf["calib"].addFloat("ADC gradient",m_grad, (3.3f/1023.f), "Gradient to convert ADC counts to voltage", true);
  conf["calib"].addFloat("ADC offset",m_offset, 0.f, "Offset in ADC counts to be subtracted from readings", true);

  conf.reset();

}

GoeUSBPixDcs::GoeUSBPixDcs(DBInquire *dbInquire, void *) : PixDcs(dbInquire, 0){
  m_handle=0;
  m_problemInit = false;
  configInit();
  m_conf->read(dbInquire);

  if(m_devType!=ADCMETER)
    throw PixDcsExc(PixDcsExc::FATAL, "GoeUSBPixDcs can at the moment only be used as type meter, different type requested though");

  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for DCS channel inquire
    if((*it)->getName() == "PixDcsChan") {
      GoeUSBPixDcsChan *uch = new GoeUSBPixDcsChan(this, *it);
      m_channels.push_back(uch);
      m_conf->addConfig(uch->m_conf);
      // set default name and channel ID if none given yet
      int chID = (int)m_channels.size()-1;
      std::stringstream a;
      a << chID;
      if(uch->m_name=="unknown"){
	uch->m_name = m_name+"_Ch"+a.str();
	uch->m_channelDescr = 1;
      }
    }    
  }


}
GoeUSBPixDcs::~GoeUSBPixDcs(){
  if(m_handle!=0){
    usb_release_interface(m_handle, 0);
    usb_close(m_handle);
  }
  delete m_conf;
}
std::string GoeUSBPixDcs::ReadState(std::string){
  std::string ret="uninit.";
  if(m_handle!=0) ret = "OK";
  else if(m_problemInit) ret = "ERROR";
  return ret;
}
void GoeUSBPixDcs::SetState(std::string){
  throw PixDcsExc(PixDcsExc::WARNING, "this device is read-only, can't process request for change in state");
}
void GoeUSBPixDcs::initHW(){
  unsigned short MyUSB_VendorID=0x03eb; // Atmel code
  unsigned short MyUSB_ProductID=0x0001; // arbitrary value

  m_problemInit = true;

  struct usb_bus *bus;
  struct usb_device *dev;
  usb_init();
  usb_find_busses();
  usb_find_devices();
  for (bus = usb_busses; bus; bus = bus->next)
  {
    for (dev = bus->devices; dev; dev = dev->next)
    {
      if ((dev->descriptor.idVendor == MyUSB_VendorID) && (dev->descriptor.idProduct == MyUSB_ProductID)){
	m_handle = usb_open(dev);
	if (usb_set_configuration(m_handle, 1) < 0) {
	  usb_close(m_handle); 
	  m_handle=0;
	}
	if (usb_claim_interface(m_handle, 0) < 0) {
	  usb_close(m_handle); 
	  m_handle=0;
	}
	if(m_handle!=0) m_problemInit = false;
	return;
      }
    }
  }
  m_handle = 0;
}
void GoeUSBPixDcs::configInit(){
  // Create the Config object
  m_conf = new Config("GoeUSBPixDcs"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown","name of PixDcs device", true);
  conf["general"].addList("DeviceType", (int &)m_devType, ADCMETER, m_typeMap, "Type of device", false); 
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", true);
  conf.reset();
}
#endif
