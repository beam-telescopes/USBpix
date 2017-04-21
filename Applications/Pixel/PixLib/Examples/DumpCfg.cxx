#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "RCCVmeInterface.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/RodPixController.h"
#include "PixConfDBInterface/TurboDaqDB.h"
#include "PixConfDBInterface/RootDB.h"
#include "Bits/Bits.h"
#include "Config/Config.h"
#include "registerIndices.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>

#include <unistd.h>

using namespace PixLib;
using namespace SctPixelRod;

std::string cfgFileName;


int main(int argc, char **argv) {

  PixConfDBInterface *tdb;
  PixModule* mod;

  if(argc > 1) {
    cfgFileName = argv[1];
  } else {
    cfgFileName = "BaseConf.cfg.root";
  }

  try {
    std::string dbName(cfgFileName);
    if(dbName.substr(dbName.size()-5, dbName.size()) == ".root") tdb = new RootDB(dbName.c_str(), "READ");
    else if(dbName.substr(dbName.size()-4, dbName.size()) == ".cfg") tdb = new TurboDaqDB(dbName.c_str());
    else {std::cout<<"Invalid config file extension: please provide a .cfg o .root file"<<std::endl; return -1;}
  }
  catch (BaseException & exc){
    std::cout << exc << std::endl; 
    exit(1);
  }
  catch (...){
    cout << "error during the pre-initialing of the application, exiting" << endl;
  }

  try{
    DBInquire* root = tdb->readRootRecord(1);
    // Look for application
    recordIterator it, itEnd = root->recordEnd();
    for(it=root->recordBegin(); it!=itEnd && (*it)->getName()!="application"; it++) {}
    if(it==itEnd) {std::cout<<"Could not find application"<<std::endl; exit(1);}
    std::cout << "Accessing Application Database: " << (*it)->getName() << std::endl;
    // Look for PixModuleGroup
    recordIterator iit, iitEnd = (*it)->recordEnd();
    for(iit=(*it)->recordBegin(); iit!=iitEnd && (*iit)->getName()!="PixModuleGroup"; iit++) {}
    if(iit==iitEnd) {std::cout<<"Could not find PixModuleGroup"<<std::endl; exit(1);}
    std::cout << "Accessing PixModuleGroup Database: " << (*iit)->getName() << std::endl;
    // Look for PixModule
    recordIterator iiit, iiitEnd = (*iit)->recordEnd();
    for(iiit=(*iit)->recordBegin(); iiit!=iiitEnd && (*iiit)->getName()!="PixModule"; iiit++) {}
    if(iiit==iiitEnd) {std::cout<<"Could not find PixModule"<<std::endl; exit(1);}
    std::cout << "Accessing PixModule Database: " << (*iiit)->getName() << std::endl;
    mod = new PixModule(*iiit, 0, "Mod0");
  }
  catch (BaseException & exc){
    cout << exc << endl;
    exit(1);
  }
  catch (...){
    cout << "error initializing the application, exiting" << endl;
    exit(1);
  }

  // Dump module configuration
  std::ifstream baseConf;
  std::ofstream outConf;
  std::string outName = cfgFileName+".cfg";
  baseConf.open("BaseConf.cfg");
  outConf.open(outName.c_str());
  while(baseConf) {
    std::string str;
    std::getline(baseConf, str);
    outConf << str << std::endl;
    if(str.find("ontroller")!=std::string::npos && str.find("_")==std::string::npos && str.find("FIELD")==std::string::npos)
      outConf << "INQUIRE PixModule Mod0" << std::endl;
  }
  mod->config().dump(outConf, "/rootRecord/Test_0/Grp0_0/");
  baseConf.close();
  outConf.close();

  return 0;
}
