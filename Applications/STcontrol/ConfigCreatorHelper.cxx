#include "ConfigCreatorHelper.h"
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <DBEdtEngine.h>
#include <GeneralDBfunctions.h>

void ConfigCreatorHelper::listModuleNames(std::string fname, std::vector<std::string> &mnames){
  mnames.clear();
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
	      PixLib::getDecNameCore(modName);
	      mnames.push_back(modName);
	    }
	  }
	}
      }
    }
  }
  delete confDBInterface; //closes file
  return;
}
