#include "ProberPixDcs.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixProber/PixProber.h"

#include <vector>
#include <sstream>

#define PPDCS_DEB false
 
using namespace PixLib;

ProberPixDcsChan::ProberPixDcsChan(PixDcs *parent, DBInquire *dbInquire) : PixDcsChan(parent, dbInquire){
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
}

ProberPixDcsChan::ProberPixDcsChan(ProberPixDcsChan &chan_in) :
  PixDcsChan(chan_in.m_parent, 0){
  configInit();
  *m_conf = *(chan_in.m_conf);
}
ProberPixDcsChan::~ProberPixDcsChan(){
  //delete m_conf; // taken care of by PixDcs's config
}

void ProberPixDcsChan::configInit(){
  if(PPDCS_DEB) cout << "ProberPixDcsChan::configInit" << endl;
  // Create the Config object

  m_conf = new Config("ProberPixDcsChan"); 
  Config &conf = *m_conf;
  // Group general
  conf.addGroup("general");
  conf["general"].addString("ChannelName", m_name, "unknown",
		  "name of PixDcs channel", true);

      std::map<std::string, int> configMap;
      configMap["Chuck"] = Chuck;
      configMap["Scope"] = Scope;

      std::map<std::string, int> configMap1;
      configMap1["X"] = x;
      configMap1["Y"] = y;
      configMap1["Z"] = z;

      conf.addGroup("settings");               
      conf["settings"].addList("ChannelType", m_type, Chuck, configMap, //Move Chuck or Scope
 	      "Device type", true); 
      conf["settings"].addList("ChannelDirection", m_direction, x, configMap1, //direction in which to move
 	      "Direction", true);
      conf["settings"].addBool("UseDefPos", m_useDefPos, true, "Use default position", true); // use default position in update command
      conf["settings"].addFloat("DefaultPosition", m_defPos, 0.0, "Default position in micrometers", true); // used if previous item is true
          
    m_rtype = POSITION; // must never change this, so not in config!
    conf.reset();
}

double ProberPixDcsChan::ReadParam(std::string measType){ //read the position of the chuck or the Scope
  if(PPDCS_DEB) cout << "ProberPixDcsChan::ReadParam" << endl;
  Config &conf = *m_conf;
  ProberPixDcs *parent = dynamic_cast<ProberPixDcs*>(m_parent);
  if(parent==0) throw PixDcsExc(PixDcsExc::WARNING, "FAILURE::<dynamic_cast> in ProberPixDcsChan::ReadParam()");
  if(measType=="position" && m_type== Chuck){

 	   parent->m_Prober->ReadChuckPosition(m_xPos, m_yPos, m_zPos); //read the value for all direction, that's how the function works

	   if(m_direction==x) return m_xPos; //depending on the direction of the PicProberDcsChan return the value
     	   else if(m_direction==y) return m_yPos;
     	   else if(m_direction==z) return m_zPos;

    	   else{
           throw PixDcsExc(PixDcsExc::WARNING, "unknown Direction: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
           }

   }else if(measType=="position" && m_type==Scope){
	
	   parent->m_Prober->ReadScopePosition(m_xPos, m_yPos, m_zPos); //read the value for all direction, that's how the function works

	   if(m_direction==x) return m_xPos; 				//depending on the direction of the PicProberDcsChan return the value
     	   else if(m_direction==y) return m_yPos;
     	   else if(m_direction==z) return m_zPos;

    	   else{
           throw PixDcsExc(PixDcsExc::WARNING, "unknown Direction: "+ 
		      ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
           }

   }else {
          throw PixDcsExc(PixDcsExc::WARNING, "unknown measurement type requested: "+ 
		       ((ConfList&)conf["general"]["ChannelDescr"]).sValue()+ ", " + measType);
        }
  return -1;
}

void ProberPixDcsChan::SetParam(std::string varType, double pos){ //if called, sends move command to Probestation
  if(PPDCS_DEB) cout << "ProberPixDcsChan::SetParam" << endl;

  ProberPixDcs *parent = dynamic_cast<ProberPixDcs*>(m_parent);
  if(parent==0) throw PixDcsExc(PixDcsExc::WARNING, "FAILURE::<dynamic_cast> in ProberPixDcsChan::SetParam()");

  if (varType == "position")
  {
     if(m_type==Chuck){
	  if(m_direction==x){	
	     						// Move chuck in micrometers from home position 
	    parent->m_Prober->MoveChuck((float)pos, parent->m_Prober->returnPosY());
	        ReadParam("position");			// if a step was made read the position
	  }else if(m_direction==y){
				
	        parent->m_Prober->MoveChuck(parent->m_Prober->returnPosX(), (float)pos);
		ReadParam("position");
          } 
	  else if(m_direction==z){
				
	        parent->m_Prober->MoveChuckZ((float)pos);
		ReadParam("position");
          }else{
          throw PixDcsExc(PixDcsExc::ERROR, "can only process request for Position setting: "+varType);
          }
     }else if(m_type==Scope){
	if(m_direction==x){	
	     						// Move Scope in micrometers from home position 
	        parent->m_Prober->MoveScope((float)pos, parent->m_Prober->returnPosY());
	        ReadParam("position");			// if a step was made read the position
	  }else if(m_direction==y){
				
	        parent->m_Prober->MoveScope(parent->m_Prober->returnPosX(), (float)pos);
		ReadParam("position");
          } 
	  else if(m_direction==z){
				
	        parent->m_Prober->MoveScopeZ((float)pos);
		ReadParam("position");
          }else{
          throw PixDcsExc(PixDcsExc::ERROR, "can only process request for Position setting: "+varType);
          }
     }else{
	throw PixDcsExc(PixDcsExc::ERROR, "can only process request for Chuck or Scope: "+varType);
     }
  }
}


void ProberPixDcsChan::SetParam(std::string varType){ // calls the actual SetParam function with default position
	SetParam(varType, m_defPos);
}

void ProberPixDcsChan::SetState(std::string state){
  if(m_parent->ReadState("")!="OK") return;
  if(state=="UPDATE") SetParam("position");
}

std::string ProberPixDcsChan::ReadState(std::string){ // not used for prober
  return "unknown";
}

ProberPixDcs::ProberPixDcs(DBInquire *dbInquire, void *interface)
  : PixDcs(dbInquire, interface){
  m_wasInit = false;
  m_problemInit = false;

  configInit();
  m_conf->read(dbInquire);

  std::vector<std::string> list;
  PixProber::listTypes(list);
  std::string proberName = list[m_proberType];

  m_Prober = PixProber::make(0, proberName); //make a PixProber Object with prober class type defined in DB files
  // add prober's configuration as new groups of our config.
  Config &conf = *m_conf;
  for(int i=0; i<m_Prober->config().size(); i++){
    std::string grpName = "prober-"+m_Prober->config()[i].name();
    m_conf->addGroup(grpName);
    conf[grpName].copyRef(m_Prober->config()[i]);
  }
  // read again from DB file to get prober items, too
  m_conf->read(dbInquire);

  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for DCS channel inquire
    if((*it)->getName() == "PixDcsChan") {
      ProberPixDcsChan *pch = new ProberPixDcsChan(this, *it);
      if(PPDCS_DEB) cout << "created ProberPixDcsChan " << pch->name()<< endl;
      m_channels.push_back(pch);
      m_conf->addConfig(pch->m_conf);
      // set default name and channel ID if none given yet
      int chID = (int)m_channels.size()-1;
      std::stringstream a;
      a << (chID+1);
      if(pch->m_name=="unknown"){
	pch->m_name = m_name+"_Ch"+a.str();
      }
    }
  }
}

ProberPixDcs::~ProberPixDcs()
{
  for(vector<PixDcsChan*>::iterator it = m_channels.begin(); it!=m_channels.end(); it++){
    PixDcsChan *pdc = *it;
    delete pdc;
  }
  m_channels.clear();
  delete m_conf;
  delete m_Prober;
}

void ProberPixDcs::configInit(){
  if(PPDCS_DEB) cout << "ProberPixDcs::configInit" << endl;
  // Create the Config object
  m_conf = new Config("ProberPixDcs"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("DeviceName", m_name, "unknown","name of PixDcs device", true);
  conf["general"].addInt("Index", m_index, -1, "Index - order in which device is used amongst other DCS devices", false);
  
  std::vector<std::string> list;
  std::vector<std::string>::const_iterator i;
  
  PixProber::listTypes(list);
  
  std::map<std::string, int> configMap;    //choose which type of Probestation you have
  for(i=list.begin(); i!=list.end(); i++){ //read in all available types and make a list
    configMap[*i] = i-list.begin();
    
  }
  conf["general"].addList("ProbestationType", m_proberType, 0, configMap, 
			  "type of Probestation", true);
  conf.reset();

  m_devType=POSITION; // only possible option, must never change this
}

std::string ProberPixDcs::ReadState(std::string){
  if (!m_wasInit)
    return "uninit.";

  if (m_problemInit) //check if initialisation of Probestation was succesfull
  {
    return "ERROR";
  }
  else
  {
    return "OK";
  }
}

void ProberPixDcs::SetState(std::string state){
  if(ReadState("")!="OK") return;
  for(vector<PixDcsChan*>::iterator it = m_channels.begin(); it!=m_channels.end(); it++)
    (*it)->SetState(state);
}

void ProberPixDcs::initHW(){
  m_wasInit = true;
  m_problemInit = true;	//initialise: there is a problem
  try{
    m_Prober->initHW();	//initialise the Prober
  }catch(PixProberExc &exc){
    m_errorMsg +=  + "Exception (level " + exc.dumpLevel() + ") caught during PixProber::initHW(): " + exc.getDescr();
    m_errorMsg += "\n";
    return;
  }catch(...){
    m_errorMsg += "Unknown exception caught during PixProber::initHW()\n";
    return;
  }
  m_problemInit = false; //Prober sucessfully initialised, so no problem
  SetState("UPDATE");
}

void ProberPixDcs::ReadError(std::string &message){
  message += m_errorMsg;
  m_errorMsg = "";
}
