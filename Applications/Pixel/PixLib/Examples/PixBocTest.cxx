#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "PixController/PixScanConfig.h"
#include "RCCVmeInterface.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/RodPixController.h"
#include "PixConfDBInterface/TurboDaqDB.h"
#include "PixConfDBInterface/RootDB.h"
#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "PixBoc/PixBoc.h"
#include "Config/Config.h"
#include "registerIndices.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

using namespace PixLib;
using namespace SctPixelRod;

std::string cfgFileName;

class application{
public:
  PixModuleGroup* grp;
  std::vector<PixModuleGroup::moduleIterator> pmod;
  std::vector<PixFe*> fe;
  PixMcc* mcc;
  VmeInterface *vme;
  PixConfDBInterface *tdb;  
  application(VmeInterface* vm, PixConfDBInterface* td) : vme(vm), tdb(td){InitializeApplication();};
  void InitializeApplication();
};

void application::InitializeApplication(){
//   DBInquire* root = tdb->readRootRecord(1);
//   recordIterator pmgIter = root->recordBegin();
//   std::cout << "Accessing Application Database: " << (*pmgIter)->getName().c_str() << std::endl;
//   pmgIter = (*pmgIter)->recordBegin();

  DBInquire* root = tdb->readRootRecord(1);
  recordIterator pmgIter = root->recordBegin();
  for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
    // Look for application inquire
    if ((*it)->getName() == "application") {
      std::cout << "Accessing Application Database: " << (*it)->getName().c_str() << std::endl;
      pmgIter = (*it)->recordBegin();
      break;
    } else if ((*it)->getName() == "PixModuleGroup") {
      pmgIter = it;
      break;
    }
  }

  std::cout << "Creating PixModuleGroup object from records" << std::endl;
  grp = new PixModuleGroup(tdb, *pmgIter, *vme);
  PixModuleGroup::moduleIterator m, mEnd=grp->modEnd();
  grp->initHW();
  grp->downloadConfig();
  for(m=grp->modBegin(); m!=mEnd; m++){
    std::cout << "*";
    pmod.push_back(m);
    std::cout << std::endl;
  }
}

/****************************************************************
 *                   main program                               *
 ****************************************************************/
int main(int argc, char **argv) {
  VmeInterface *vme;
  PixConfDBInterface *tdb;
  application * app;
  bool bocDelayScan = false;
  bool allLinks = true;
  bool testInLinks = false;
  bool scanLink[96];
  bool inLink[48];

  unsigned int il;
  for (il=0; il<96; il++) scanLink[il] = false;
  for (il=0; il<48; il++) inLink[il] = false;

  if (argc > 1) {
    cfgFileName = argv[1];
    for (int ip=2; ip<argc-1; ip++) {
      if (strcmp(argv[ip],"-d")==0) {
        bocDelayScan = true;
      } else if (strcmp(argv[ip],"-l")==0) {
        allLinks = false;
        unsigned int l1,l2;
        if (ip+2 < argc) {
          sscanf(argv[ip+1],"%d",&l1);
          sscanf(argv[ip+2],"%d",&l2);
          ip+=2;
          if (l1>=0 && l1<96 && l2>=0 && l2<96) {
            for (il=l1; il<=l2; il++) scanLink[il] = true;
          }
        }
      } else if (strcmp(argv[ip],"-i")==0) {
        testInLinks = true;
        unsigned int l1,l2;
        if (ip+2 < argc) {
          sscanf(argv[ip+1],"%d",&l1);
          sscanf(argv[ip+2],"%d",&l2);
          ip+=2;
          if (l1>=0 && l1<48 && l2>=0 && l2<48) {
            for (il=l1; il<=l2; il++) inLink[il] = true;
          }
        }
      }
    }
  } else {
    cfgFileName = "test.cfg.root";
  }
//   if (allLinks == true) {
//     for (il = 0; il<96; il++) scanLink[il] = true;
//   }    

  try {
    vme = new RCCVmeInterface();
    std::string dbName(cfgFileName);
    if(dbName.substr(dbName.size()-5, dbName.size()) == ".root") tdb = new RootDB(dbName.c_str());
    else if(dbName.substr(dbName.size()-4, dbName.size()) == ".cfg") tdb = new TurboDaqDB(dbName.c_str());
    else {std::cout<<"Invalid config file extension: please provide a .cfg o .root file"<<std::endl; return -1;}
  }
  catch (VmeException &v) {
    std::cout << "VmeException:" << std::endl;
    std::cout << "  ErrorClass = " << v.getErrorClass();
    std::cout << "  ErrorCode = " << v.getErrorCode() << std::endl;
  }
  catch (BaseException & exc){
    std::cout << exc << std::endl; 
    exit(1);
  }
  catch (...){
    cout << "error during the pre-initialing of the application, exiting" << endl;
  }

  try{
    app = new application(vme,tdb);
  }
  catch (BaseException & exc){
    cout << exc << endl;
    exit(1);
  }
  catch (...){
    cout << "error initializing the application, exiting" << endl;
    exit(1);
  }

  try{
    //int i;
    cout<<"Get Group"<<endl;
    PixModuleGroup *grp = app->grp;
    //RodPixController *rod = dynamic_cast<RodPixController *>(grp->getPixController());
    cout<<"Get BOC"<<endl;
    PixBoc *boc = grp->getPixBoc();

    if (boc) {
      cout<<"BOC initialization:"<<endl;
      boc->BocInit();
      cout<<"***********************************"<<endl;
      cout<<"Get the BOC status:"<<endl;
      boc->getBocStatus();
      cout<<"***********************************"<<endl;
      cout<<"Configure the BOC:"<<endl;
      boc->BocConfigure();
      cout<<"***********************************"<<endl;
      cout<<"Show BOC's configuration:"<<endl;
      boc->showBocConfig();
      cout<<"***********************************"<<endl;
      cout<<"Configure the BOC:"<<endl;
      boc->BocConfigure();
//       cout<<"***********************************"<<endl;
//       cout<<"Configure the BOC:"<<endl;
//       boc->BocConfigure();
      cout<<"***********************************"<<endl;
      cout<<"Show BOC's configuration:"<<endl;
      boc->showBocConfig();
      cout<<"***********************************"<<endl;
      cout<<"Now get monitor data: "<<endl;
      double monidata[12];
      boc->BocMonitor(monidata);
      for (int i=0; i<12; i++){
	cout<<"Monitorchannel "<<i<<": "<<monidata[i]<<endl;
      }
    } else {
      std::cout << "A PixBoc is required for this test" << std::endl;
    }
  }
  catch (BaseException & exc){
    cout << exc << endl;
    exit(1);
  }
  catch (...){
    cout << "error running the application, exiting" << endl;
    exit(1);
  }

  return 0;
}
