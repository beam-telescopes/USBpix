/////////////////////////////////////////////////////////////////////
// PixModule.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 11/04/03  Version 1.0 (CS)
//           Initial release
//

//! Class for the Pixel Module

#include "PixDcs/SleepWrapped.h"
#include "Bits/Bits.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixMcc/PixMcc.h"
#include "PixMcc/PixMccData.h"
#include "PixMcc/PixMccStructures.h"
#include "PixMcc/PixMccI1.h"
#include "PixMcc/PixMccI2.h"
#include "PixFe/PixFe.h"
#include "PixFe/PixFeData.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeI1.h"
#include "PixFe/PixFeI2.h"
#include "PixFe/PixFeI4A.h"
#include "PixFe/PixFeI4B.h"
#include "PixCcpd/PixCcpd.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Config/Config.h"
#include "PixModule/PixModule.h"

#include <string.h>

using namespace SctPixelRod;
using namespace PixLib;


PixModule::PixModule(DBInquire *dbInquire, PixModuleGroup *modGrp, std::string name) :
  m_name(name), m_group(modGrp), m_feRows(1), m_inputLink(0),  
  m_outputLink1(1), m_outputLink2(1), m_outputLink3(1), m_outputLink4(1), 
  m_delay(750), m_latency(200), m_outputBandwidth(0),
  m_readoutActive(false)
{
  createConfig(dbInquire);
  Config &conf = *m_conf;
  m_mcc = 0;
  m_ccpd = 0;

  if(dbInquire==0) return;

  for(recordIterator it = dbInquire->recordBegin(); it != dbInquire->recordEnd(); it++){
    // Look for MCC inquire
    if((*it)->getName() == "PixMcc") {
      // Read MCC derived class
      fieldIterator f = (*it)->findField("ClassInfo_ClassName");
      if(f!=(*it)->fieldEnd()) {
        std::string className;
        bool stat = dbInquire->getDB()->DBProcess(f, READ, className);
        if(stat) {
          // Switch between different MCC derived classes
          if(className == "PixMccI1") {
	    m_mcc = new PixMccI1(*it, this, "MCC");
	    conf.addConfig(&(m_mcc->config()));
	  }
          if(className == "PixMccI2") {
	    m_mcc = new PixMccI2(*it, this, "MCC");
	    conf.addConfig(&(m_mcc->config()));
	  }
        }
      }
    }
    // Look for FE inquires
    if((*it)->getName() == "PixFe"){
      std::stringstream o((*it)->getDecName().substr((*it)->getDecName().rfind("_")+1,2));
      int a;
      o >> a;
      // Read FE derived class
      fieldIterator f = (*it)->findField("ClassInfo_ClassName");
      if(f!=(*it)->fieldEnd()) {
        std::string className;
        bool stat = dbInquire->getDB()->DBProcess(f, READ, className);
        if(stat) {
          // Switch between different FE derived classes
          if(className == "PixFeI1") {
	    m_fe.push_back(new PixFeI1(*it, this,(*it)->getDecName(),a));
	    conf.addConfig(&(m_fe.back()->config()));
	  }
          if(className == "PixFeI2") {
	    m_fe.push_back(new PixFeI2(*it, this,(*it)->getDecName(),a));
	    conf.addConfig(&(m_fe.back()->config()));
	  }
          if(className == "PixFeI4A" || className == "PixFeI4") { // keep PixFeI4 for backw. compat.
	    m_fe.push_back(new PixFeI4A(*it, this,(*it)->getDecName(),a));
	    conf.addConfig(&(m_fe.back()->config()));
	    if(className == "PixFeI4") m_feFlavour = PM_FE_I4A; // needed for backw. compat.
	  }
          if(className == "PixFeI4B") {
	    m_fe.push_back(new PixFeI4B(*it, this,(*it)->getDecName(),a));
	    conf.addConfig(&(m_fe.back()->config()));
	  }
        }
      }
    }
    // Look for CCPD inquires
    if((*it)->getName() == "PixCcpd"){
      // Read CCPD derived class
      fieldIterator f = (*it)->findField("ClassInfo_ClassName");
      if(f!=(*it)->fieldEnd()) {
        std::string className;
        bool stat = dbInquire->getDB()->DBProcess(f, READ, className);
        if(stat) m_ccpd = PixCcpd::make(*it, className);
      }
      if(m_ccpd==0){// next steps needed for backward-compatibility: field "ClassInfo_ClassName" might be missing
	// default: try v1
	try{
	  m_ccpd = PixCcpd::make(*it,"PixCcpdv1");
	}catch(PixDBException &exc){
	  // exception thrown due to inconsistency Config-RootDB -> most likely v2, so let's try that
	  try{
	    m_ccpd = PixCcpd::make(*it,"PixCcpdv2");
	  }catch(...){
	    std::cerr << "PixModule constructor: exception caught while creating CCPD from DBrecord" << std::endl;
	    m_ccpd = 0;
	  }
	}catch(...){
	  std::cerr << "PixModule constructor: exception caught while creating CCPD from DBrecord" << std::endl;
	  m_ccpd = 0;
	}
      }
      if(m_ccpd!=0) conf.addConfig(&(m_ccpd->config()));
    }
  }
  // temporary - need something smarter to define FE gemoetry in module
  if(m_fe.size()>2) m_feRows = 2;
}

PixModule::PixModule(PixModuleGroup *modGrp, unsigned int id, std::string name, std::string feType, int nFe, int nFeRows, std::string mccType) :
  m_name(name), m_group(modGrp), m_feRows(nFeRows), m_inputLink(0),  
  m_outputLink1(1), m_outputLink2(1), m_outputLink3(1), m_outputLink4(1), 
  m_delay(750), m_latency(200), m_outputBandwidth(0),
  m_readoutActive(false)
{
  createConfig(0);
  Config &conf = *m_conf;

  m_mccFlavour = m_mccfl[mccType];
  m_feFlavour  = m_fefl[feType];
  m_moduleId = id;

  m_mcc = 0; // if MCC flavour = NONE, keep NULL-pointer
  m_ccpd = 0; // JGK: to do (but currently not needed): add option to have CCPD created
  
  // Switch between different MCC flavours
  if(mccType=="MCC_I1"){
    m_mcc = new PixMccI1(0, this, "MCC");
    conf.addConfig(&(m_mcc->config()));
  }
  if(mccType=="MCC_I2"){
    m_mcc = new PixMccI2(0, this, "MCC");
    conf.addConfig(&(m_mcc->config()));
  }

  for(int iFe = 0; iFe<nFe; iFe++){
    std::stringstream feLabel;
    feLabel << "FE_" << iFe;
    // Switch between different FE flavours
    if(feType=="FE_I1"){
      m_fe.push_back(new PixFeI1(0, this, feLabel.str(),iFe));
      conf.addConfig(&(m_fe.back()->config()));
    }
    if(feType=="FE_I2" || feType=="FE_I3"){
      m_fe.push_back(new PixFeI2(0, this, feLabel.str(),iFe));
      conf.addConfig(&(m_fe.back()->config()));
    }
    if(feType=="FE_I4A"){
      m_fe.push_back(new PixFeI4A(0, this, feLabel.str(),iFe));
      conf.addConfig(&(m_fe.back()->config()));
    }
    if(feType=="FE_I4B"){
      m_fe.push_back(new PixFeI4B(0, this, feLabel.str(),iFe));
      conf.addConfig(&(m_fe.back()->config()));
    }
  }
}

void PixModule::createConfig(DBInquire *dbInquire){
  // Create the Config object
  stringstream configName;
  //  configName<<m_name<<"_0"<<"/PixModule";
  configName<<m_name<<"/PixModule";
  m_conf = new Config(configName.str());
  Config &conf = *m_conf;

  // Insert the configuration parameters in the Config object
  std::map<std::string, int> tf;
  tf["FALSE"] = 0;
  tf["TRUE"] = 1;
  m_mccfl["MCC_I1"] = PM_MCC_I1;
  m_mccfl["MCC_I2"] = PM_MCC_I2;
  m_mccfl["NONE"]   = PM_NO_MCC;
  m_fefl["FE_I1"] = PM_FE_I1;
  m_fefl["FE_I2"] = PM_FE_I2;
  m_fefl["FE_I3"] = PM_FE_I2; // FE-I3 cfg. identical to FE-I2
  m_fefl["FE_I4A"] = PM_FE_I4A;
  m_fefl["FE_I4B"] = PM_FE_I4B;

  // Group general
  conf.addGroup("general");

  conf["general"].addInt("ModuleId", m_moduleId, 0, 
                         "Module Identifier", true);
  conf["general"].addInt("GroupId", m_groupId, 0, 
                         "Group Identifier", true);
  conf["general"].addList("Active", m_active, 1, tf, 
                          "Module active", true);
  conf["general"].addList("Present", m_present, 1,tf, 
                          "Module present", true);
  conf["general"].addInt("InputLink", m_inputLink, 0, 
                         "DCI link", true);
  conf["general"].addInt("OutputLink1", m_outputLink1, 0, 
                         "DTO link 1", true);
  conf["general"].addInt("OutputLink2", m_outputLink2, 0, 
                         "DTO link 2", true);
  conf["general"].addInt("OutputLink3", m_outputLink3, 0, 
                         "DTO link 3", true);
  conf["general"].addInt("OutputLink4", m_outputLink4, 0, 
                         "DTO link 4", true);
  conf["general"].addInt("BocInputLink", m_bocInputLink, -1, 
                         "BOC DCI link", true);
  conf["general"].addInt("BocOutputLink1", m_bocOutputLink1, -1, 
                         "BOC DTO link 1", true);
  conf["general"].addInt("BocOutputLink2", m_bocOutputLink2, -1, 
                         "DTO link 2", true);
  conf["general"].addList("MCC_Flavour", m_mccFlavour, PM_NO_MCC, m_mccfl, 
                          "MCC Flavour", true);
  conf["general"].addList("FE_Flavour", m_feFlavour, PM_FE_I4A, m_fefl, 
                          "FE Flavour", true);

  std::map<std::string, int> geomTyp;
  geomTyp["unknown"] = NONE;
  geomTyp["Stave"] = STAVE;
  geomTyp["Sector"] = SECTOR;

  // Group general
  conf.addGroup("geometry");

  conf["geometry"].addList("Type", (int &)m_geomType, NONE, geomTyp, 
                           "Geometry type", true);
  conf["geometry"].addInt("position", m_geomPosition, 0, 
                          "Module position in assembly", true);
  conf["geometry"].addInt("staveID", m_geomAssemblyId, 0, 
                          "Assembly identifier", true);

  std::map<std::string, int> pp0Typ;
  pp0Typ["Optical"] = OPTICAL;
  pp0Typ["Optical_Test"] = OPTICAL_TEST;
  pp0Typ["Electrical"] = ELECTRICAL;
  pp0Typ["USB"] = USB;

  // Group general
  conf.addGroup("pp0");

  conf["pp0"].addList("Type", (int &)m_pp0Type, OPTICAL, pp0Typ, 
                           "PP0 type", true);
  conf["pp0"].addInt("position", m_pp0Position, 1, 
                          "PP0 position", true);

  conf.reset();
  if(dbInquire!=0) conf.read(dbInquire);

}

PixModule::~PixModule() {
  // Delete MCC
  delete m_mcc;

  // Delete FEs
  for(std::vector<PixFe*>::iterator fe = m_fe.begin(); fe != m_fe.end(); fe++)
    delete *fe;

  // delete CCPD
  delete m_ccpd;
}

void PixModule::loadConfig(std::string name) {
  // Load configuration data into child MCC
  if(m_mcc!=0) m_mcc->loadConfig(name);

  // Load configuration data into child FEs
  for(std::vector<PixFe*>::iterator fe = m_fe.begin(); fe != m_fe.end(); fe++)
    (*fe)->loadConfig(name);

  //Load CCPD configuration data into child CCPD
  if(m_ccpd!=0) m_ccpd->loadConfig(name);
}

void PixModule::saveConfig(std::string name) {
  // Save configuration data from child MCC
  if(m_mcc!=0) m_mcc->saveConfig(name);

  // Save configuration data from child FEs
  for(std::vector<PixFe*>::iterator fe = m_fe.begin(); fe != m_fe.end(); fe++)
    (*fe)->saveConfig(name);

  // Save configuration data from child CCPD
  if(m_ccpd!=0) m_ccpd->saveConfig(name);
}

void PixModule::storeConfig(std::string name) {
  // Store configuration data into child MCC
  if(m_mcc!=0) m_mcc->storeConfig(name);

  // Store configuration data into child FEs
  for(std::vector<PixFe*>::iterator fe = m_fe.begin(); fe != m_fe.end(); fe++)
    (*fe)->storeConfig(name);

  // Store configuration data into child CCPD
  //if(m_ccpd!=0) m_ccpd->storeConfig(name);
}

bool PixModule::restoreConfig(std::string name) {
  bool ret = true;
  // Restore configuration data into child MCC
  if (m_mcc!=0 && !m_mcc->restoreConfig(name)) ret = false;
  
  // Restore configuration data into child FEs
  for(std::vector<PixFe*>::iterator fe = m_fe.begin(); fe != m_fe.end(); fe++) {
    if (!(*fe)->restoreConfig(name)) ret = false;

  // Restore configuration data into child CCPD
  if (m_ccpd!=0/* && !m_ccpd->restoreConfig(name)*/) ret = false;
  }
  return ret;
}

void PixModule::deleteConfig(std::string name) {
  // Delete configuration data into child MCC
  if(m_mcc!=0) m_mcc->deleteConfig(name);

  // Delete configuration data into child FEs
  for(std::vector<PixFe*>::iterator fe = m_fe.begin(); fe != m_fe.end(); fe++)
    (*fe)->deleteConfig(name);

  // Delete configuration data into child CCPD
  //if(m_ccpd!=0) m_ccpd->deleteConfig(name);
}

void PixModule::setConfig(int /*structId*/, int /*moduleNum*/) {
}
PixFe* PixModule::pixFE(int nFE) {
  if(nFE>=0 && nFE<(int)m_fe.size())
    return m_fe[nFE];
  else
    return 0;
}
void PixModule::reset(int type){
  if(pixMCC()!=0){
    if(type==PixModuleGroup::PMG_CMD_SRESET){
      pixMCC()->globalResetFE(31);
      pixMCC()->globalResetMCC();
    } else if(type==PixModuleGroup::PMG_CMD_ECR){
      pixMCC()->ecr();
      pixMCC()->syncFE();
    } else if(type==PixModuleGroup::PMG_CMD_BCR){
      pixMCC()->bcr();
      pixMCC()->syncFE();
    } else{
      // not implemented
    }
  }
}
unsigned int PixModule::iColMod(unsigned int iFe, unsigned int col){
  if(m_fe.size()==0) return 0;
  unsigned int nFePerRow = m_fe.size()/m_feRows;
  if(iFe<nFePerRow)
    return (col + m_fe[0]->nCol()*iFe);
  else
    return (m_fe[0]->nCol()*(m_fe.size()-iFe)-col-1);
}
unsigned int PixModule::iRowMod(unsigned int iFe, unsigned int row){
  if(m_fe.size()==0) return 0;
  unsigned int nFePerRow = m_fe.size()/m_feRows;
  if(iFe<nFePerRow)
    return row;
  else
    return (2*m_fe[0]->nRow()-1-row);
}
unsigned int PixModule::nColsMod(){
  if(m_fe.size()==0) return 0;
  return m_fe.size()*m_fe[0]->nCol()/m_feRows;
}
unsigned int PixModule::nRowsMod(){
  if(m_fe.size()==0) return 0;
  return m_fe[0]->nRow()*m_feRows;
}
unsigned int PixModule::nColsFe(){
  if(m_fe.size()==0) return 0;
  return m_fe[0]->nCol();
}
unsigned int PixModule::nRowsFe(){
  if(m_fe.size()==0) return 0;
  return m_fe[0]->nRow();
}
