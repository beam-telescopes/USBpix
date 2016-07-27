#include "TApplication.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixDbInterface/PixDbInterface.h"
#include "PixDbInterface/RootDb.h"
#include "Histo/Histo.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "Config/ConfObj.h"

#include <iostream>
#include <exception>
#include <stdlib.h>
#include <sstream>

// example of application using the PixConfDBInterface to get informations from the Data Base

using namespace PixLib;
using std::string;

int main(int argc, char* argv[]){
  
  TApplication app("MyApp",0,0); // this is to allow pop up of graphical windows in root -> see root documentation
  
  if(argc != 3){
    std::cout << "Wrong program arguments. Usage:" << std::endl;
    std::cout << "RootDBTest <old cfg file> <new cfg file> to create a Db root file from a DB root file" << std::endl;
    return 1;
  }
  try{
    PixConfDBInterface* myDB;
    PixDbInterface* myNewDb;

    //std::cout << " first of all the application should instanziate a data base ..." << std::endl;
    myDB = new RootDB(argv[1],"READ");
    DBInquire* root = myDB->readRootRecord(1); std::cout << " read the root record." << std::endl;
    
    std::string myDbName = argv[2];
    //std::cout << " the application now instanziate a data base of new type..." << std::endl;
    myNewDb = new RootDb(myDbName, "NEW");
    DbRecord* newRoot = myNewDb->readRootRecord();       

    int modID = 990001;

    for(recordIterator appIter = root->recordBegin();appIter!=root->recordEnd();appIter++){
      if((int)(*appIter)->getName().find("application")!=(int)std::string::npos){
	for(recordIterator grpIter = (*appIter)->recordBegin();grpIter!=(*appIter)->recordEnd();grpIter++){
	  if((int)(*grpIter)->getName().find("PixModuleGroup")!=(int)std::string::npos){
	    PixModuleGroup *grp = new PixModuleGroup(myDB, *grpIter);
	    for(PixModuleGroup::moduleIterator mit = grp->modBegin(); mit!=grp->modEnd(); mit++){
	      PixModule::FEFlavour flv = (PixModule::FEFlavour)((ConfInt&)(*mit)->config()["general"]["FE_Flavour"]).value();
	      if(flv==PixModule::PM_FE_I4A || flv==PixModule::PM_FE_I4B){
		// MCC flavour not set correctly in USBpix FE-I4 data, force to be "NONE"
		Config &cfg = (*mit)->config();
		((ConfInt&)cfg["general"]["MCC_Flavour"]).setValue(PixModule::PM_NO_MCC);
	      }
	      std::cout << "Found module " << (*mit)->moduleName() << ", using it with ID " << modID << std::endl;
	      std::stringstream a;
	      a<<"M"<<modID;
	      DbRecord* modRec = newRoot->addRecord("PixModule", a.str());
	      (*mit)->config().write(modRec);
	      modID++;
	    }
	  }
	}
      }
    }
  }
  catch(PixLib::PixDBException& exc){
    std::cout << exc << std::endl; 
    exc.what(std::cout);
    throw;
  }
  catch(SctPixelRod::BaseException& exc){
    std::cout << exc << std::endl; 
    exc.what(std::cout);
    throw;
  }
  catch(std::exception& exc){
    std::cout << "std exception caught" << std::endl;
    std::cout << exc.what() << std::endl;
    throw;
  }
  catch(...){
    std::cout << "unexpected exception" << std::endl;
    throw;
  }
  return 0;
}
