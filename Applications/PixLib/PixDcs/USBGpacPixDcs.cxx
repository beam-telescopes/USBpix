#include "USBGpacPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixController/USBPixController.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include <gpac.h>

using namespace PixLib;

USBGpacPixDcs::USBGpacPixDcs(DBInquire *dbInquire, void *interface, std::string extDevType):
  PixDcs(dbInquire, interface)
{
  m_upc = (USBPixController *) (interface); //dynamic_cast<PixUSB>(interface);

  if (!m_upc)
  {
    throw PixDcsExc(PixDcsExc::FATAL, "The supplied interface for "
        "USBGpacPixDcs::USBGpacPixDcs was not a USBPixController.");
  }

  configInit();
  if(dbInquire!=0) m_conf->read(dbInquire);
  else{
    if(m_typeMap.find(extDevType)!=m_typeMap.end())
      m_devType = (DeviceType)m_typeMap[extDevType];
    else
      m_devType = SUPPLY;
  }
    
  if (m_devType == SUPPLY) {
    Config& conf = *m_conf;
    conf.addGroup("settings");
    conf["settings"].addFloat("CurrentLimit", m_currentLimit, 1.0, 
			      "Current limit per channel for all channels in A", true);
    conf.reset();
    m_devType = SUPPLY;
    if(dbInquire!=0) m_conf->read(dbInquire);
  }
  
  if(dbInquire!=0){
    
    for(recordIterator it = dbInquire->recordBegin(); 
	it != dbInquire->recordEnd(); it++){
      if((*it)->getName() == "PixDcsChan") {
	switch (m_devType)
	  {
	  case SUPPLY:
	    {
	      USBGpacSupplyPixDcsChan *uch = new USBGpacSupplyPixDcsChan(this, *it);
	      m_channels.push_back(uch);
	      m_conf->addConfig(uch->m_conf);
	      
	      // set default name and channel ID if none given yet
	      int chID = (int)m_channels.size()-1;
	      std::stringstream a;
	      a << "PWR";
	      a << chID;
	      if (uch->m_name=="unknown")
		{
		  uch->m_name = a.str();
		}
	      if (uch->m_gpac_channel == -1)
		{
		  uch->m_gpac_channel = chID;
		}
	      break;
	    }
	  case ADCMETER:
	    {
	      USBGpacAuxAdcPixDcsChan *uch = new USBGpacAuxAdcPixDcsChan(this, *it);
	      m_channels.push_back(uch);
	      m_conf->addConfig(uch->m_conf);
	      
	      // set default name and channel ID if none given yet
	      int chID = (int)m_channels.size()-1;
	      std::stringstream a;
	      a << "AUXADC";
	      a << chID;
	      if (uch->m_name=="unknown")
		{
		  uch->m_name = a.str();
		}
	      if (uch->m_gpac_channel == -1)
		{
		  uch->m_gpac_channel = chID;
		}
	      break;
	    }
	  case VOLTAGE_SOURCE:
	    {
	      USBGpacVoltageSourcePixDcsChan *uch = new USBGpacVoltageSourcePixDcsChan(this, *it);
	      m_channels.push_back(uch);
	      m_conf->addConfig(uch->m_conf);
	      
	      // set default name and channel ID if none given yet
	      int chID = (int)m_channels.size()-1;
	      std::stringstream a;
	      a << "VSRC";
	      a << chID;
	      if (uch->m_name=="unknown")
		{
		  uch->m_name = a.str();
		}
	      if (uch->m_gpac_channel == -1)
		{
		  uch->m_gpac_channel = chID;
		}
	      break;
	    }
	  case CURRENT_SOURCE:
	    {
	      USBGpacCurrentSourcePixDcsChan *uch = new USBGpacCurrentSourcePixDcsChan(this, *it);
	      m_channels.push_back(uch);
	      m_conf->addConfig(uch->m_conf);
	      
	      // set default name and channel ID if none given yet
	      int chID = (int)m_channels.size()-1;
	      std::stringstream a;
	      a << "ISRC";
	      a << chID;
	      if (uch->m_name=="unknown")
		{
		  uch->m_name = a.str();
		}
	      if (uch->m_gpac_channel == -1)
		{
		  uch->m_gpac_channel = chID;
		}
	      break;
	    }
	  case PULSER:
	    {
	      USBGpacInjectPixDcsChan *uch = new USBGpacInjectPixDcsChan(this, *it);
	      m_channels.push_back(uch);
	      m_conf->addConfig(uch->m_conf);
	      
	      // set default name and channel ID if none given yet
	      int chID = (int)m_channels.size()-1;
	      std::stringstream a;
	      a << "INJECT";
	      a << chID;
	      if (uch->m_name=="unknown")
		{
		  uch->m_name = a.str();
		}
	      if (uch->m_gpac_channel == -1)
		{
		  uch->m_gpac_channel = chID;
		}
	      break;
	    }
	  default:
	    throw PixDcsExc(PixDcsExc::FATAL, "An unsupported device type was "
			    "requested from USBGpacPixDcs.");
	  }
      }
    }
  } else { // create default channels
    switch (m_devType)
      {
      case SUPPLY:
	{
	  for(int chID=0; chID<4; chID++){
	    USBGpacSupplyPixDcsChan *uch = new USBGpacSupplyPixDcsChan(this, (DBInquire*) 0);
	    m_channels.push_back(uch);
	  
	    // set default name and channel ID if none given yet
	    std::stringstream a;
	    a << "PWR";
	    a << chID;
	    uch->m_name = a.str();
	    uch->m_gpac_channel = chID;
	    uch->m_conf->m_confName = "USBGpacSupplyPixDcsChan_"+a.str()+"/PixDcsChan";

	    m_conf->addConfig(uch->m_conf);
	  }
	  break;
	}
      case ADCMETER:
	{
	  for(int chID=0; chID<4; chID++){
	    USBGpacAuxAdcPixDcsChan *uch = new USBGpacAuxAdcPixDcsChan(this, (DBInquire*) 0);
	    m_channels.push_back(uch);
	    
	    // set default name and channel ID if none given yet
	    std::stringstream a;
	    a << "AUXADC";
	    a << chID;
	    uch->m_name = a.str();
	    uch->m_gpac_channel = chID;
	    uch->m_conf->m_confName = "USBGpacAuxAdcPixDcsChan_"+a.str()+"/PixDcsChan";

	    m_conf->addConfig(uch->m_conf);
	  }
	  break;
	}
      case VOLTAGE_SOURCE:
	{
	  for(int chID=0; chID<4; chID++){
	    USBGpacVoltageSourcePixDcsChan *uch = new USBGpacVoltageSourcePixDcsChan(this, (DBInquire*) 0);
	    m_channels.push_back(uch);
	    
	    // set default name and channel ID if none given yet
	    std::stringstream a;
	    a << "VSRC";
	    a << chID;
	    uch->m_name = a.str();
	    uch->m_gpac_channel = chID;
	    uch->m_conf->m_confName = "USBGpacVoltageSourcePixDcsChan_"+a.str()+"/PixDcsChan";

	    m_conf->addConfig(uch->m_conf);
	  }
	  break;
	}
      case CURRENT_SOURCE:
	{
	  for(int chID=0; chID<12; chID++){
	    USBGpacCurrentSourcePixDcsChan *uch = new USBGpacCurrentSourcePixDcsChan(this, (DBInquire*) 0);
	    m_channels.push_back(uch);
	    
	    // set default name and channel ID if none given yet
	    std::stringstream a;
	    a << "ISRC";
	    a << chID;
	    uch->m_name = a.str();
	    uch->m_gpac_channel = chID;
	    uch->m_conf->m_confName = "USBGpacCurrentSourcePixDcsChan_"+a.str()+"/PixDcsChan";

	    m_conf->addConfig(uch->m_conf);
	  }
	  break;
	}
      case PULSER:
	{
	  int chID = 0;
	  USBGpacInjectPixDcsChan *uch = new USBGpacInjectPixDcsChan(this, (DBInquire*) 0);
	  m_channels.push_back(uch);
	  
	  // set default name and channel ID if none given yet
	  std::stringstream a;
	  a << "INJECT";
	  a << chID;
	  uch->m_name = a.str();
	  uch->m_gpac_channel = chID;
	  uch->m_conf->m_confName = "USBGpacInjectPixDcsChan_"+a.str()+"/PixDcsChan";

	  m_conf->addConfig(uch->m_conf);

	  break;
	}
      default:
	throw PixDcsExc(PixDcsExc::FATAL, "An unsupported device type was "
			"requested from USBGpacPixDcs.");
      }
  }
  m_ctrlName = m_upc->getModGroup().getName();
}
    
USBGpacPixDcs::~USBGpacPixDcs()
{
  delete m_conf;
  for(vector<PixDcsChan*>::iterator it = m_channels.begin(); it!=m_channels.end(); it++){
    PixDcsChan *pdc = *it;
    delete pdc;
  }
  m_channels.clear();
}

void USBGpacPixDcs::configInit()
{
  m_conf = new Config("USBGpacPixDcs"); 
  Config &conf = *m_conf;
  
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown",
		  "name of PixDcs device", true);
  conf["general"].addList("DeviceType", (int &)m_devType, ADCMETER, m_typeMap, "Type of device", false); 
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", false);
  conf["general"].addString("USBPixController", m_ctrlName, "???", "Name of PixController to which adapter is attached", false);
  
  conf.reset();
}
      
std::string USBGpacPixDcs::ReadState(std::string req)
{
  if(m_upc->getUsbHandle() == 0){
    return "unknown";
    // @todo: clean up libgpac stuff
  }
  return ReadState2(req);
}

std::string USBGpacPixDcs::ReadState2(std::string req)
{
  gpac::logical * gp = m_upc->getGpac();

  std::string ret = "uninit.";

  if(gp)
  {
    if((m_devType == SUPPLY)
      || (m_devType == VOLTAGE_SOURCE)
      || (m_devType == CURRENT_SOURCE)
      || (m_devType == PULSER))
    {
      int n_On=0;
      int n_Off=0;
      for (auto &ch: m_channels)
      {
        //PixDcsChan *ch = dynamic_cast<USBGpacPixVoltageSourceDcsChan*>(it);
        std::string chState = ch->ReadState(req);
        if(chState=="ON" || chState=="LIM")  n_On++;
        if(chState=="OFF") n_Off++;
      }

      if (n_On==nchan())
      { 
        ret = "ALL ON";
      }
      else if(n_Off==nchan()) 
      {
        ret = "ALL OFF";
      }
      else
      {
        ret = "CHAN";
      }
    } 
    else
    {
      ret = "OK";
    }
  }
  else if (m_problemInit) 
  {
    ret = "ERROR";
  }
  return ret;
}
void USBGpacPixDcs::SetState(std::string new_state)
{
  if((new_state != "OFF") && (new_state != "ON"))
    return;

  if ((m_devType == SUPPLY)
      || (m_devType == VOLTAGE_SOURCE)
      || (m_devType == CURRENT_SOURCE)
      || (m_devType == PULSER))
  {
    for (auto &ch: m_channels)
    {
      ch->SetState(new_state);
    }
  }
}

void USBGpacPixDcs::initHW()
{
  gpac::logical * gp = m_upc->getGpac();
  m_problemInit = true;
  if (!gp)
  {
    throw PixDcsExc(PixDcsExc::ERROR, "USB interface must be initialised before DCS object can be set up");
  }
  
  if(m_devType == SUPPLY)
  {
    SetState("OFF");
    m_problemInit = false;
    gp->power_supplies.current_limit(m_currentLimit);
  }
  else if(m_devType == CURRENT_SOURCE)
  {
    SetState("OFF");
    m_problemInit = false;
  }
  else if(m_devType == VOLTAGE_SOURCE)
  {
    SetState("OFF");
    m_problemInit = false;
  }
  else if(m_devType == ADCMETER)
  {
    m_problemInit = false;
  }
  else if(m_devType == PULSER)
  {
    SetState("OFF");
    m_problemInit = false;
  }
}

void USBGpacPixDcs::ReadError(std::string& errTxt)
{
  // @todo
  errTxt = "";
  return; 
}

USBGpacSupplyPixDcsChan::USBGpacSupplyPixDcsChan(PixDcs *parent, 
    DBInquire *dbInquire):
  PixDcsChan(parent, dbInquire)
{
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
  m_rtype = VCBOTH;
}
      
USBGpacSupplyPixDcsChan::USBGpacSupplyPixDcsChan(USBGpacSupplyPixDcsChan &chan_in) :
  PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
  m_rtype = VCBOTH;
}

void USBGpacSupplyPixDcsChan::configInit()
{
  m_conf = new Config("USBGpacSupplyPixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("GpacChannel", m_gpac_channel, -1,
		  "GPAC channel index", true);

  conf.addGroup("settings");
  conf["settings"].addFloat("NomVolts", m_nominalVoltage, 0.0, 
      "Nominal voltage in V", true);
  conf.reset();
}

USBGpacSupplyPixDcsChan::~USBGpacSupplyPixDcsChan()
{

}

double USBGpacSupplyPixDcsChan::ReadParam(std::string tp)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();
  if (tp == "voltage")
  {
    return gp->power_supplies[m_gpac_channel].capture_voltage();
  }
  else if (tp == "current")
  {
    return gp->power_supplies[m_gpac_channel].capture_current();
  }
  else 
  {
    throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		       tp);
  }

}

void USBGpacSupplyPixDcsChan::SetParam(std::string varType, double value)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp)
  {
    throw PixDcsExc(PixDcsExc::WARNING, "PixController refused to offer a "
        "GPAC object. Has the controller been initialized?");
  }

  if (varType == "voltage")
  {
    gp->power_supplies[m_gpac_channel].output_voltage(value);
  }
  else
  {
    throw PixDcsExc(PixDcsExc::ERROR, "GPAC power supplies do not support " 
        "setting the parameter " + varType);
  }
}

void USBGpacSupplyPixDcsChan::SetParam(string varType)
{
  if (varType == "voltage")
  {
    SetParam(varType, (double) m_nominalVoltage);
  }
}

std::string USBGpacSupplyPixDcsChan::ReadState(std::string)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp || !parent->getCtrl()->getUsbHandle())
  {
    return "unknown";
  }

  if (!gp->power_supplies[m_gpac_channel].enabled())
  {
    return "OFF";
  }
  else if (gp->power_supplies[m_gpac_channel].overcurrent())
  {
    return "LIM";
  }
  else
  {
    return "ON";
  }
}

void USBGpacSupplyPixDcsChan::SetState(std::string state)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  SetParam("voltage");
  gp->power_supplies[m_gpac_channel].enabled(state == "ON");
}


USBGpacVoltageSourcePixDcsChan::USBGpacVoltageSourcePixDcsChan(PixDcs *parent, 
    DBInquire *dbInquire):
  PixDcsChan(parent, dbInquire),
  m_gpac_channel(-1),
  m_nominalVoltage(0.0),
  m_setVoltage(0.0),
  m_enabled(false)
{
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
  m_rtype = VCBOTH;
}
USBGpacVoltageSourcePixDcsChan::USBGpacVoltageSourcePixDcsChan(USBGpacVoltageSourcePixDcsChan &chan_in) :
  PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
  m_rtype = VCBOTH;
}

      
void USBGpacVoltageSourcePixDcsChan::configInit()
{
  m_conf = new Config("USBGpacVoltageSourcePixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("GpacChannel", m_gpac_channel, -1,
		  "GPAC channel index", true);
  
  conf.addGroup("settings");
  conf["settings"].addFloat("NomVolts", m_nominalVoltage, 0.0, 
      "Nominal voltage in V", true);
  conf.reset();
}

USBGpacVoltageSourcePixDcsChan::~USBGpacVoltageSourcePixDcsChan()
{

}

double USBGpacVoltageSourcePixDcsChan::ReadParam(std::string tp)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();
  if (tp == "voltage")
  {
    return gp->voltage_sources[m_gpac_channel].capture_voltage();
  }
  else if (tp == "current")
  {
    return gp->voltage_sources[m_gpac_channel].capture_current();
  }
  else 
  {
    throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		       tp);
  }
  return 0;
}

void USBGpacVoltageSourcePixDcsChan::SetParam(std::string varType, double value)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp)
  {
    throw PixDcsExc(PixDcsExc::WARNING, "PixController refused to offer a "
        "GPAC object. Has the controller been initialized?");
  }

  if (varType == "voltage")
  {
    m_setVoltage = (float) value;
    gp->voltage_sources[m_gpac_channel].output_voltage(m_enabled ? value : 0.0);
  }
  else
  {
    throw PixDcsExc(PixDcsExc::ERROR, "GPAC power supplies do not support " 
        "setting the parameter " + varType);
  }
}

void USBGpacVoltageSourcePixDcsChan::SetParam(std::string varType)
{
  if (varType == "voltage")
  {
    SetParam(varType, (double) m_nominalVoltage);
  }
}

std::string USBGpacVoltageSourcePixDcsChan::ReadState(std::string)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp || !parent->getCtrl()->getUsbHandle())
  {
    return "unknown";
  }

  if (m_enabled)
  {
    return "ON";
  }
  else
  {
    return "OFF";
  }
}

void USBGpacVoltageSourcePixDcsChan::SetState(std::string state)
{
  m_enabled = (state == "ON");
  SetParam("voltage");
}


USBGpacCurrentSourcePixDcsChan::USBGpacCurrentSourcePixDcsChan(PixDcs *parent, 
    DBInquire *dbInquire):
  PixDcsChan(parent, dbInquire),
  m_gpac_channel(0),
  m_nominalCurrent(0.0),
  m_setCurrent(0.0),
  m_enabled(false)
{
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
  m_rtype = VCBOTH;
}

USBGpacCurrentSourcePixDcsChan::USBGpacCurrentSourcePixDcsChan(USBGpacCurrentSourcePixDcsChan &chan_in) :
  PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
  m_rtype = VCBOTH;
}

void USBGpacCurrentSourcePixDcsChan::configInit()
{
  m_conf = new Config("USBGpacCurrentSourcePixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("GpacChannel", m_gpac_channel, -1,
		  "GPAC channel index", true);
  
  conf.addGroup("settings");
  conf["settings"].addFloat("NomCurrent", m_nominalCurrent, 0.0, 
      "Nominal current in A", true);
  conf.reset();
}

USBGpacCurrentSourcePixDcsChan::~USBGpacCurrentSourcePixDcsChan()
{

}

double USBGpacCurrentSourcePixDcsChan::ReadParam(std::string tp)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();
  if (tp == "voltage")
  {
    return gp->current_sources[m_gpac_channel].capture_voltage();
  }
  else if (tp == "current")
  {
    return gp->current_sources[m_gpac_channel].capture_current();
  }
  else 
  {
    throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		       tp);
  }
}

void USBGpacCurrentSourcePixDcsChan::SetParam(std::string varType,double value)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp)
  {
    throw PixDcsExc(PixDcsExc::WARNING, "PixController refused to offer a "
        "GPAC object. Has the controller been initialized?");
  }

  if (varType == "current")
  {
    m_setCurrent = (float)value;
    gp->current_sources[m_gpac_channel].output_current(m_enabled ? m_setCurrent : 0);
  }
  else
  {
    throw PixDcsExc(PixDcsExc::ERROR, "GPAC power supplies do not support " 
        "setting the parameter " + varType);
  }
}

void USBGpacCurrentSourcePixDcsChan::SetParam(string varType)
{
  if (varType == "current")
  {
    SetParam(varType, (double) m_nominalCurrent);
  }
}

std::string USBGpacCurrentSourcePixDcsChan::ReadState(std::string)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp || !parent->getCtrl()->getUsbHandle())
  {
    return "unknown";
  }

  if (m_enabled)
  {
    return "ON";
  }
  else
  {
    return "OFF";
  }
}

void USBGpacCurrentSourcePixDcsChan::SetState(std::string state)
{
  m_enabled = (state == "ON");
  SetParam("current");
}

USBGpacInjectPixDcsChan::USBGpacInjectPixDcsChan(PixDcs *parent, 
    DBInquire *dbInquire):
  PixDcsChan(parent, dbInquire),
  m_gpac_channel(-1),
  m_enabled(false),
  m_nominalHigh(0.0),
  m_nominalLow(0.0),
  m_setHigh(0.0),
  m_setLow(0.0)
{
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
  m_rtype = NONE;
}

USBGpacInjectPixDcsChan::USBGpacInjectPixDcsChan(USBGpacInjectPixDcsChan &chan_in) :
  PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
  m_rtype = NONE;
}

void USBGpacInjectPixDcsChan::configInit()
{
  m_conf = new Config("USBGpacInjectPixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("GpacChannel", m_gpac_channel, -1,
		  "GPAC channel index", true);
  
  conf.addGroup("settings");
  conf["settings"].addFloat("High", m_nominalHigh, 1.0, 
      "Voltage when pulser is high in V", true);
  conf["settings"].addFloat("Low", m_nominalLow, 0.5, 
      "Voltage when pulser is low in V", true);
  conf.reset();
}

USBGpacInjectPixDcsChan::~USBGpacInjectPixDcsChan()
{

}

double USBGpacInjectPixDcsChan::ReadParam(std::string /*tp*/)
{
  // @todo
  return 0;
}

void USBGpacInjectPixDcsChan::SetParam(std::string varType, double value)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp)
  {
    throw PixDcsExc(PixDcsExc::WARNING, "PixController refused to offer a "
        "GPAC object. Has the controller been initialized?");
  }

  if (varType == "voltage-low")
  {
    m_setLow = (float) value;
    gp->injection.low_voltage(m_enabled ? m_setLow: 0);
  }
  else if (varType == "voltage-high")
  {
    m_setHigh = (float) value;
    gp->injection.high_voltage(m_enabled ? m_setHigh : 0);
  }
  else
  {
    throw PixDcsExc(PixDcsExc::ERROR, "GPAC power supplies do not support " 
        "setting the parameter " + varType);
  }
}

void USBGpacInjectPixDcsChan::SetParam(string varType)
{
  if (varType == "voltage")
  {
    SetParam("voltage-low", (double) m_nominalLow);
    SetParam("voltage-high", (double) m_nominalHigh);
  }
}

std::string USBGpacInjectPixDcsChan::ReadState(std::string)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();

  if (!gp || !parent->getCtrl()->getUsbHandle())
  {
    return "unknown";
  }

  if (m_enabled)
  {
    return "ON";
  }
  else
  {
    return "OFF";
  }
}

void USBGpacInjectPixDcsChan::SetState(std::string state)
{
//   USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
//   gpac::logical * gp = parent->getCtrl()->getGpac();

  m_enabled = (state == "ON");
  SetParam("voltage");
}

USBGpacAuxAdcPixDcsChan::USBGpacAuxAdcPixDcsChan(PixDcs *parent, 
    DBInquire *dbInquire):
  PixDcsChan(parent, dbInquire)
{
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
  m_rtype = VONLY;
}
USBGpacAuxAdcPixDcsChan::USBGpacAuxAdcPixDcsChan(USBGpacAuxAdcPixDcsChan &chan_in) :
  PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
  m_rtype = VONLY;
}

void USBGpacAuxAdcPixDcsChan::configInit()
{
  m_conf = new Config("USBGpacAuxAdcPixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("GpacChannel", m_gpac_channel, -1,
		  "GPAC channel index", true);
  conf.reset();
}

USBGpacAuxAdcPixDcsChan::~USBGpacAuxAdcPixDcsChan()
{

}

double USBGpacAuxAdcPixDcsChan::ReadParam(std::string tp)
{
  USBGpacPixDcs *parent = dynamic_cast<USBGpacPixDcs*>(m_parent);
  gpac::logical * gp = parent->getCtrl()->getGpac();
  if (tp == "voltage")
  {
    return gp->aux_adcs[m_gpac_channel].capture_voltage();
  }
  else 
  {
    throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		       tp);
  }
}

void USBGpacAuxAdcPixDcsChan::SetParam(std::string,double)
{
  // @todo
}

void USBGpacAuxAdcPixDcsChan::SetParam(string /*varType*/)
{
  // @todo
}

std::string USBGpacAuxAdcPixDcsChan::ReadState(std::string)
{
  // @todo
  return "OK";
}

void USBGpacAuxAdcPixDcsChan::SetState(std::string)
{
  // nothing to do
}

