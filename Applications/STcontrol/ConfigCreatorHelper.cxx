#include "ConfigCreatorHelper.h"
#include <PixModuleGroup/PixModuleGroup.h>
#include <PixController/PixController.h>
#include <PixModule/PixModule.h>
#include <PixFe/PixFe.h>
#include <PixDcs/PixDcs.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <DBEdtEngine.h>
#include <GeneralDBfunctions.h>

void ConfigCreatorHelper::listModuleNames(std::string fname, std::vector<std::string> &mNames, std::vector<std::string> &mDecNames){
  mNames.clear();
  mDecNames.clear();
  PixLib::PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(fname.c_str(), false); 
  PixLib::DBInquire *root = confDBInterface->readRootRecord(1);
  for(PixLib::recordIterator appIter = root->recordBegin();appIter!=root->recordEnd();appIter++){
    if((int)(*appIter)->getName().find("application")!=(int)std::string::npos){
      // loop over inquires in crate inquire and create a PixModuleGroup when an according entry is found
      for(PixLib::recordIterator pmgIter = (*appIter)->recordBegin();pmgIter!=(*appIter)->recordEnd();pmgIter++){
	if((*pmgIter)->getName().find("PixModuleGroup")!=std::string::npos){
	  for(PixLib::recordIterator pmIter = (*pmgIter)->recordBegin();pmIter!=(*pmgIter)->recordEnd();pmIter++){
	    if((*pmIter)->getName().find("PixModule")!=std::string::npos){
	      std::string modName = (*pmIter)->getDecName();
	      mDecNames.push_back(modName);
	      PixLib::getDecNameCore(modName);
	      mNames.push_back(modName);
	    }
	  }
	}
      }
    }
  }
  delete confDBInterface; //closes file
  return;
}
void ConfigCreatorHelper::readModuleInfo(std::string fname, std::string mDecName, int &mccFlv, int &feFlv, int &nFe, int &nFeRows){
  PixLib::PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(fname.c_str(), false); 
  PixLib::DBInquire *root = confDBInterface->readRootRecord(1);
  std::vector<PixLib::DBInquire*> inqVec;
  try{
    inqVec = confDBInterface->DBFindRecordByName(PixLib::BYDECNAME, root->getDecName()+ mDecName +"/PixModule");
  }catch(...){
    inqVec.clear();
    std::cerr << "Error searching DB entry for module " << mDecName << std::endl;
  }
  if(inqVec.size()==1){
    PixLib::PixModule mod(inqVec[0], 0, "tmpmod");
    // problem with old cfg. files: MCC_I2 set for 1 FE_I2, which is wrong
    if(mod.getMCCFlavour()==PixLib::PixModule::PM_MCC_I2 && mod.getFEFlavour()==PixLib::PixModule::PM_FE_I2 &&
       mod.getFECount()==1){
      PixLib::ConfList mccFlvCfg = (PixLib::ConfList&)mod.config()["general"]["MCC_Flavour"];
      mccFlvCfg.setValue((int)PixLib::PixModule::PM_NO_MCC);
    }
    nFe = mod.getFECount();
    nFeRows = mod.nRowsMod()/mod.nRowsFe();
    mccFlv = (int)mod.getMCCFlavour();
    feFlv = (int)mod.getFEFlavour();
  } else
    std::cerr << "Can't find DB entry for module " << mDecName << std::endl;
  
  delete confDBInterface; //closes file
}
PixLib::PixModuleGroup* ConfigCreatorHelper::createPixModGrp(std::string name, std::string ctrlType, std::vector<std::string> mnames, 
							     std::string feType, int nFe, int nFeRows, std::string mccType, int ctrlOpt,
							     std::map<int, std::vector<std::string> > dbFnames, 
							     std::map<int, std::vector<std::string> > dbMnames){

  PixLib::PixModuleGroup *pmg = new PixLib::PixModuleGroup(name, ctrlType, mnames, feType, nFe, nFeRows, mccType, ctrlOpt);
  for(std::map<int, std::vector<std::string> >::iterator it=dbFnames.begin(); it!=dbFnames.end();it++){
    PixLib::PixModule *mod = pmg->module(it->first);
    if(mod!=0 && dbMnames.find(it->first)!=dbMnames.end()){
      // remember for later - will be overwritten when reading from DB, but we need this group's IDs
      int mid = mod->moduleId();
      int gid = mod->groupId();
      for(unsigned int j = 0; j<it->second.size(); j++){
	// open and read from DB file
	PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(it->second[j].c_str(), false); 
	DBInquire *root = confDBInterface->readRootRecord(1);
	std::vector<DBInquire*> inqVec;
	try{
	  inqVec = confDBInterface->DBFindRecordByName(PixLib::BYDECNAME, root->getDecName()+dbMnames[it->first][j]+"/PixModule");
	}catch(...){
	  inqVec.clear();
	}
	if(inqVec.size()==1){
	  if(it->second.size()==1){
	    // read DB info into module config.
	    mod->config().read(inqVec[0]);
	    // problem with old cfg. files: MCC_I2 set for 1 FE_I2, which is wrong
	    if(mod->getMCCFlavour()==PixModule::PM_MCC_I2 && mod->getFEFlavour()==PixModule::PM_FE_I2 &&
	       mod->getFECount()==1){
	      PixLib::ConfList mccFlv = (ConfList&)mod->config()["general"]["MCC_Flavour"];
	      mccFlv.setValue((int)PixModule::PM_NO_MCC);
	    }
	  } else {
	    for(recordIterator feIter = inqVec[0]->recordBegin();feIter!=inqVec[0]->recordEnd();feIter++){
	      if((*feIter)->getName().find("PixFe")!=std::string::npos){
		// read DB info into module's j-th FE config.
		mod->pixFE(j)->config().read(*feIter);
	      }
	    }
	  }
	} else{
	  std::cout << "Can't find module " << dbMnames[it->first][j] << " in file " << it->second[j] << std::endl;
	}
	delete confDBInterface; //closes file// restore IDs
	((ConfInt&)mod->config()["general"]["ModuleId"]).setValue(mid);
	((ConfInt&)mod->config()["general"]["GroupId"]).setValue(gid);
      }
    }
  }

  // polish configuration
  pmg->config().addConfig(&(pmg->getPixController()->config()));
  for(PixLib::PixModuleGroup::moduleIterator it = pmg->modBegin(); it!=pmg->modEnd(); it++)
    pmg->config().addConfig(&((*it)->config()));

  return pmg;
}
PixLib::PixDcs* ConfigCreatorHelper::createPixDcs(int type, std::string name, int index, void *controller){
  PixDcs *dcs = 0;
  std::string dcsType = "";
  switch(type){
  case 0:
    dcsType = "USBPixDcs";
    break;
  case 1:
  case 3:
    dcsType = "USBBIPixDcs";
    break;
  case 2:
    dcsType = "USBGpacPixDcs";
    break;
  default:
    dcsType = "";
  }	    
  if(dcsType != ""){
    // new DCS object for regulators
    dcs = PixDcs::make((DBInquire*) 0, controller, dcsType);
    Config &conf = dcs->config();
    ((ConfString&)conf["general"]["DeviceName"]).m_value = name;
    ((ConfInt&)conf["general"]["Index"]).setValue(index);
  }
  return dcs;
}
