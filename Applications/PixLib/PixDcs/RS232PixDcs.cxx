/*------------------------------------------------------------
 *  RS232PixDcx.cxx
 *  Version:  0.1.USBsystem
 *  Created:  19 February 2009
 *  Author:   Joern Grosse-Knetter
 *
 *  Functional interface to the USB system for communication
 *  with RS232 devices
 *------------------------------------------------------------*/

#include "PixDcs/RS232PixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixRs232Device.h"
#include "SleepWrapped.h"
#include <sstream>
#include <math.h>

using namespace PixLib;

RS232PixDcsChan::RS232PixDcsChan(PixDcs *parent, DBInquire *dbInquire) : PixDcsChan(parent, dbInquire){
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
RS232PixDcsChan::RS232PixDcsChan(RS232PixDcsChan &chan_in) : PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
}
RS232PixDcsChan::~RS232PixDcsChan(){
  //  delete m_conf; // taken care of by PixDcs's config
}
void RS232PixDcsChan::configInit(){
  // Create the Config object
  m_conf = new Config("RS232PixDcsChan"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);
  conf["general"].addInt("ChannelID", m_channelID, -1, "ID of channel on this device", true);
  conf.addGroup("settings");
  RS232PixDcs* myDcs = dynamic_cast<RS232PixDcs*>(m_parent);
  if(myDcs != 0 && myDcs->getDevType()==PixDcs::SUPPLY){
    conf["settings"].addFloat("NomVolts", m_nomVolts, 0.0, "Nominal voltage (in V) on this channel", true);
    conf["settings"].addFloat("CurrLim", m_currLim, 0.0, "Current limit (in A) on this channel", true);
    conf["settings"].addInt("RampTime",m_rampTime, 1, "Time (in s) for ramping to full voltage", true);
    conf["settings"].addFloat("RampSteps",m_rampVoltSteps, 1, "Voltage-steps (in V) for ramping up/down", true);
    m_rtype = VCBOTH; // must never change this, so not in config!
  }
  else if (myDcs != 0 && myDcs->getDevType()==PixDcs::POSITION){
	  conf["settings"].addInt("Position",m_position, 0, "Position (in a. u.)", true);
	  conf["settings"].addInt("MaxSpeed",m_maxSpeed, 2000, "Max. Speed (in a. u.)", true);
	  m_rtype = POSITION;
  }
  else if (myDcs != 0 && myDcs->getDevType()==PixDcs::CHILLER){
	  conf["settings"].addFloat("NomTemp",m_nomTemp, 20.0, "Nominal Temperature (in +/-°C)", true);
	  conf["settings"].addBool("ExternalTempSensor",m_externalTempSensor, false, "Use an external temp sensor attached to chiller for regulation", true);
	  m_rtype = TEMP;
  }
  else
  {
    std::map<std::string,int> rtlist;
    rtlist["VOLTS_ONLY"] = VONLY;
    rtlist["CURR_ONLY"] = CONLY;
    rtlist["VOLTS_AND_CURR"] = VCBOTH;
    conf["general" ].addList ("ReadType", (int &)m_rtype, VONLY, rtlist, "Type of reading for this meter/ADC-channel", true);
    conf["settings"].addBool ("AutoRange", m_autoRange, true, "Automatic adjustment of measurement range (if supported)", true);
    conf["settings"].addFloat("MeasRange", m_measRange, 1.0, "Range of measurement (unless auto-range is active)", true);
  }
  conf.reset();
}
double RS232PixDcsChan::ReadParam(std::string measType) {
	// Read a string value from DDC, returning a double.
	RS232PixDcs* myDcs = dynamic_cast<RS232PixDcs*>(m_parent);
	double ret=0.0;

	if(measType=="voltage"){
		if((m_rtype==VONLY || m_rtype==VCBOTH || m_rtype==VCT) && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getVoltage(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for voltage reading, but channel does not support this!");
		return ret;
	}else if(measType=="current"){
		if((m_rtype==CONLY || m_rtype==VCBOTH || m_rtype==VCT) && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getCurrent(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for current reading, but channel does not support this!");
		return ret;
	}else if(measType=="position"){
		if(m_rtype==POSITION && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getPosition(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for position reading, but channel does not support this!");
		return ret;
	}else if(measType=="maxspeed"){
		if(m_rtype==POSITION && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getMaxSpeed(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for speed reading, but channel does not support this!");
		return ret;
	}else if(measType=="temperature"){
		if(myDcs->getDevType()==PixDcs::CHILLER && m_rtype==TEMP && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getChillerCurrentTemperature(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for current temperature reading, but channel does not support this!");
		return ret;
	}
	else if(measType=="settemperature"){
		if(myDcs->getDevType()==PixDcs::CHILLER && m_rtype==TEMP && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getChillerSetTemperature(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for set temperature reading, but channel does not support this!");
		return ret;
	}
	else if(measType=="useexternaltempsensor"){
		if(myDcs->getDevType()==PixDcs::CHILLER && m_rtype==TEMP && m_channelID>=0)
			ret = (double)myDcs->m_rs232Dev->getChillerRegulation(m_channelID);
		else
			throw PixDcsExc(PixDcsExc::ERROR, "request for external/internal regulation reading, but channel does not support this!");
		return ret;
	}

	throw PixDcsExc(PixDcsExc::ERROR, "request is not supported: "+measType);
	return ret;
}

void RS232PixDcsChan::SetParam(string varType) {
  // Set resource with string id to default value.
  if(varType=="voltage")
    SetParam(varType, (double)m_nomVolts);
  else if(varType=="currentlim")
    SetParam(varType, (double)m_currLim);
  else if(varType=="position")
      SetParam(varType, (double)m_position);
  else if(varType=="maxspeed")
      SetParam(varType, (double)m_maxSpeed);
  else if(varType=="settemperature")
	  SetParam(varType, (double)m_nomTemp);
  else if(varType=="useexternaltempsensor")
	  SetParam(varType, (double)m_externalTempSensor);
  else
    throw PixDcsExc(PixDcsExc::ERROR, "request for neither voltage, current limit, position, speed or set temperature: "+varType);
}

void RS232PixDcsChan::SetParam(string varType, double value) {
	// Set resource with string id to a double value.
	RS232PixDcs* myDcs = dynamic_cast<RS232PixDcs*>(m_parent);
	if(myDcs == 0) return;
	if(myDcs->m_rs232Dev==0) return; // can't set anything without init. hardware

	if(myDcs->getDevType()==PixDcs::SUPPLY){
		if(varType=="voltage" && m_channelID>=0){
			rampVoltage(value);	//ramp voltage to protect DUT
			//m_nomVolts = value;	//synchronize with set voltage - why?
		}else if(varType=="currentlim" && m_channelID>=0){
			myDcs->m_rs232Dev->setCurrentLimit(m_channelID, value);
			//m_currLim = value;
		}else
			throw PixDcsExc(PixDcsExc::ERROR, "request for neither voltage nor current limit for: "+varType);
	}
	else if(myDcs->getDevType()==PixDcs::POSITION){
		if(varType=="position" && m_channelID>=0){
		  myDcs->m_rs232Dev->setPosition(m_channelID, (int)(value+0.5)); // rounding to next int for value
		  m_position = myDcs->m_rs232Dev->getPosition(m_channelID);	//update stored value to the real measured value
		}else if(varType=="maxspeed" && m_channelID>=0){
		  myDcs->m_rs232Dev->setMaxSpeed(m_channelID, (int)(value+0.5)); // rounding to next int for value
		  m_maxSpeed = myDcs->m_rs232Dev->getMaxSpeed(m_channelID);	//update stored value
		}else
		  throw PixDcsExc(PixDcsExc::ERROR, "request is neither position nor Speed for: "+varType);
	}
	else if(myDcs->getDevType()==PixDcs::CHILLER){
		if(varType=="settemperature" && m_channelID>=0){
		  myDcs->m_rs232Dev->setChillerTargetTemperature(m_channelID, (float)value);
			m_nomTemp = myDcs->m_rs232Dev->getChillerSetTemperature(m_channelID);	//update stored value to the real measured value
		}else if(varType=="useexternaltempsensor" && m_channelID>=0){
		  myDcs->m_rs232Dev->setChillerRegulation(m_channelID, (((int)value)!=0));
		  m_externalTempSensor = myDcs->m_rs232Dev->getChillerRegulation(m_channelID);	//update stored value
		}else
		  throw PixDcsExc(PixDcsExc::ERROR, "request is neither position nor Speed for: "+varType);
	}
	else
		throw PixDcsExc(PixDcsExc::ERROR, "DCS device not supported: "+varType);

}

std::string RS232PixDcsChan::ReadState(std::string){
	return "unknown";
}
void RS232PixDcsChan::SetState(std::string){
  return;
}
void RS232PixDcsChan::rampVoltage(double pVoltage)
{
	RS232PixDcs* myDcs = dynamic_cast<RS232PixDcs*>(m_parent);
	float mysign=1;
	float curr_volt = (float)ReadParam("voltage");
	if(pVoltage<curr_volt) mysign=-1; // ramp up or down?
	if(((int)m_rampVoltSteps)!=0 && fabs(pVoltage-curr_volt)>m_rampVoltSteps){ // avoids endless loops and mistakes if difference between current and wanted status is small
		for(float in_volts = curr_volt; (in_volts*mysign)<=(mysign*pVoltage); in_volts+=mysign*fabs(m_rampVoltSteps)){
			myDcs->m_rs232Dev->setVoltage(m_channelID, in_volts);
//			SetParam("voltage", in_volts);
			unsigned int ustime = (unsigned int) 1000*m_rampTime;
			PixLib::sleep(ustime);
		}
	}
	myDcs->m_rs232Dev->setVoltage(m_channelID, pVoltage);
	unsigned int ustime = (unsigned int) 1000*m_rampTime;
	PixLib::sleep(ustime);
}
RS232PixDcs::RS232PixDcs(DBInquire *dbInquire, void* )//interface)
  : PixDcs(dbInquire, 0){

  m_rs232Dev = 0;
  configInit();
  m_conf->read(dbInquire);
  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for DCS channel inquire
    if((*it)->getName() == "PixDcsChan") {
      RS232PixDcsChan *gch = new RS232PixDcsChan(this, *it);
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
RS232PixDcs::~RS232PixDcs(){
  delete m_rs232Dev;
  delete m_conf;
}

std::string RS232PixDcs::ReadState(std::string){
  // Read a string value from DDC, returning a string.
  std::string ret="uninit.";
  if(m_rs232Dev!=0){
    if(m_rs232Dev->getPowerStatus() == PixRs232Device::COM_ON){
      switch(m_rs232Dev->getStatus()){
      case PixRs232Device::COM_OK:
	ret = "ON";
	if(m_devType==ADCMETER)// ON and OFF meaningless for meter
	  ret = "OK";
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
      case PixRs232Device::COM_LIMIT:
	ret = "LIM";
	break;
      case PixRs232Device::COM_WARNING:
	ret = "WARNING";
	break;
      default:
      case PixRs232Device::COM_ERROR:
	ret = "ERROR";
      }
    }
    else if(m_rs232Dev->getPowerStatus() == PixRs232Device::COM_OFF){
      ret = "OFF";
      if(m_devType==ADCMETER)// ON and OFF meaningless for meter
	ret = "OK";
    }
    else if(m_rs232Dev->getPowerStatus() == PixRs232Device::COM_LIM){
      ret = "LIM";
    }
  }
  return ret;
}
void RS232PixDcs::SetState(std::string new_state){
	if(m_rs232Dev==0) // can't set anything without init. hardware
		return;
	if(m_devType == SUPPLY){
		if(new_state!="OFF" && new_state!="ON" && new_state!="UPDATE")
			throw PixDcsExc(PixDcsExc::ERROR, "unknown request for supply, either ON/OFF/UPDATE: "+new_state);

		if((m_rs232Dev->getPowerStatus()==PixRs232Device::COM_ON || m_rs232Dev->getPowerStatus()==PixRs232Device::COM_LIM) 
		   && new_state=="OFF"){
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
				RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
				float in_volts = (float)chan->ReadParam("voltage");
				float mysign=1;
				if(in_volts<0) mysign=-1;
				if(((int)chan->m_rampVoltSteps)!=0){ // avoids endless loops
					for(; (in_volts*mysign)>=0; in_volts-=mysign*fabs(chan->m_rampVoltSteps)){
						chan->SetParam("voltage", in_volts);
						unsigned int ustime = (unsigned int) 1000*chan->m_rampTime;
						PixLib::sleep(ustime);
					}
				}
				chan->SetParam("voltage", 0.);
			}
			m_rs232Dev->setPowerStatus(PixRs232Device::COM_OFF);
			m_rs232Dev->setStatus(PixRs232Device::COM_OK);	//reset state when powered off
			return;
		}
		if(m_rs232Dev->getPowerStatus()==PixRs232Device::COM_OFF && new_state=="ON"){
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
			  RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
			  chan->SetParam("currentlim");
			  chan->SetParam("voltage", 0.);
			}
			m_rs232Dev->setPowerStatus(PixRs232Device::COM_ON);
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
			  RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
			  float mysign=1;
			  if(chan->m_nomVolts<0) mysign=-1;
			  if(((int)chan->m_rampVoltSteps)!=0){ // avoids endless loops
			for(float in_volts = 0.; (in_volts*mysign)<=(mysign*chan->m_nomVolts); in_volts+=mysign*fabs(chan->m_rampVoltSteps)){
			  chan->SetParam("voltage", in_volts);
			  unsigned int ustime = (unsigned int) 1000*chan->m_rampTime;
			  PixLib::sleep(ustime);
			}
			  }
			  chan->SetParam("voltage", chan->m_nomVolts);
			}
			return;
		}
		if(m_rs232Dev->getPowerStatus()==PixRs232Device::COM_ON && new_state=="UPDATE" ){
			for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
			  RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
			  chan->SetParam("currentlim");
			  chan->rampVoltage(chan->m_nomVolts);
			  // m_nomVolts should be the desired voltage, not what is actually set
			  // chan->m_nomVolts=chan->ReadParam("voltage");
			}
			return;
		}
	}

	if(m_devType==POSITION){
		if (new_state.compare("ON") == 0)
			m_rs232Dev->setPowerStatus(PixRs232Device::COM_ON);
		if (new_state.compare("OFF") == 0){
			m_rs232Dev->setPowerStatus(PixRs232Device::COM_OFF);//reset state when powered off
			m_rs232Dev->setStatus(PixRs232Device::COM_OK);
		}
		if (new_state.compare("UPDATE") == 0){
			for(vector<PixDcsChan*>::iterator it=m_channels.begin(); it!=m_channels.end(); ++it){
				RS232PixDcsChan* tChan = dynamic_cast<RS232PixDcsChan*>(*it);
				if(tChan != 0){
					tChan->SetParam("maxspeed", (double) tChan->m_maxSpeed);
					tChan->SetParam("position", (double) tChan->m_position);
				}
			}
		}
	}
	if(m_devType==CHILLER){
		if (new_state.compare("ON") == 0)
			m_rs232Dev->setPowerStatus(PixRs232Device::COM_ON);
		if (new_state.compare("OFF") == 0){
			m_rs232Dev->setPowerStatus(PixRs232Device::COM_OFF);//reset state when powered off
			m_rs232Dev->setStatus(PixRs232Device::COM_OK);
		}
		if (new_state.compare("UPDATE") == 0){
			for(vector<PixDcsChan*>::iterator it=m_channels.begin(); it!=m_channels.end(); ++it){
				RS232PixDcsChan* tChan = dynamic_cast<RS232PixDcsChan*>(*it);
				if(tChan != 0){
					tChan->SetParam("settemperature", (double) tChan->m_nomTemp);
					tChan->SetParam("useexternaltempsensor", (double) tChan->m_externalTempSensor);
				}
			}
		}
	}

}
void RS232PixDcs::configInit(){
  // Create the Config object
  m_conf = new Config("RS232PixDcs"); 
  Config &conf = *m_conf;
  
  std::map<std::string, int> portmap;
  portmap["COM1"] = PixRs232Device::COM1;
  portmap["COM2"] = PixRs232Device::COM2;
  portmap["COM3"] = PixRs232Device::COM3;
  portmap["COM4"] = PixRs232Device::COM4;
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown","name of PixDcs device", true);
  conf["general"].addList("Port", m_port, 0, portmap, "Serial port communicating to device", true);
  conf["general"].addList("DeviceType", (int &)m_devType, SUPPLY, m_typeMap, "Type of device", false); 
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", false);
  conf.reset();

}
void RS232PixDcs::initHW(){
  if(m_rs232Dev!=0 && m_rs232Dev->getStatus()==PixRs232Device::COM_ERROR){
      delete m_rs232Dev; 
      m_rs232Dev=0;
  }
  if(m_rs232Dev==0){
    m_rs232Dev = new PixRs232Device((PixRs232Device::Portids) m_port);
  } else{
    SetState("OFF");
    //    m_rs232Dev->resetDevice();
  }
  if(m_devType==SUPPLY){
    for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
      RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
      chan->SetParam("currentlim");
    }
  }
  if(m_devType==POSITION){
      for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
        RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
        chan->ReadParam("position");	//synchronize actual position
        chan->ReadParam("maxspeed");	//synchronize actual maxspeed
      }
  }
  if(m_devType==CHILLER){
      for(vector<PixDcsChan*>::iterator IT=m_channels.begin(); IT!=m_channels.end(); IT++){
        RS232PixDcsChan *chan = dynamic_cast<RS232PixDcsChan*>(*IT);
        chan->ReadParam("temperature");	//synchronize
        chan->ReadParam("settemperature");	//synchronize
        chan->ReadParam("useexternaltempsensor");
      }
  }
}
bool RS232PixDcs::sendCommand(std::string &/*txt*/, bool /*readResp*/){
  return false; // return false until implemented
}
void RS232PixDcs::ReadError(std::string &errTxt){
  errTxt = "";
  if(m_rs232Dev!=0) m_rs232Dev->getError(errTxt);
  return;
}
