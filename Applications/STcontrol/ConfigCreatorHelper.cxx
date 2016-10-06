#include "ConfigCreatorHelper.h"
#include <PixModule/PixModule.h>
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
