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
#include "Config/Config.h"
#include "registerIndices.h"
#include "RodCrate/RodModule.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <TCanvas.h>
#include <TH2.h>


#include <unistd.h>

using namespace PixLib;
using namespace SctPixelRod;

std::string cfgFileName;
int boc_steps     = 12;
int pattern_length = 520;
TH2D *BOCHIST;


void histoPattern(Bits b, int size, int step){

  int count  = 0;
  Stat_t data;
  int bit;

  for(int i=0;i<b.size();i++) {
    bit = b.get(i,i);
    if(count==0 && bit==1){
      count++;
    }
    if(count>0 && count<size){
      //printf("%1x",bit);
      data = BOCHIST->GetBinContent(step+1, count+1);
      data += bit;
      BOCHIST->SetBinContent(step+1, count+1, data);
      count++;
    }
  }
  //  printf("\n");
}


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

void dumpFifo(RodPixController *rod, 
              unsigned int fifo, unsigned int bank, unsigned int len, 
              std::string filename, int link = -1, int id=-1) {
  std::vector<unsigned int> out;
  rod->readFifo(fifo, bank, len, out);

  unsigned int count = 0, bit = 0;
  Bits bits, zero(1,0), one(1,1);
  while (count<out.size()) {
    for (int i=0; i<48; i++) {
      if (i==link) {
	if ((out[count]&(0x1<<bit)) == 0) {
	  bits += zero;
	} else {
	  bits += one;
	}
      }
      bit++;
      if (bit == 32) {
	bit = 0;
	count++;
      }
    }  
  }
    std::string dump;
    histoPattern(bits, pattern_length, id);
  
}


void dumpTextBuffers(RodPixController *rod) {
  std::string txt;
    
  std::cout << "============================= Err Buffer" << std::endl;
  while (rod->getErrBuffer(txt)) std::cout << txt << std::endl;
  std::cout << "============================= Info Buffer" << std::endl;
  while (rod->getInfoBuffer(txt)) std::cout << txt << std::endl;
  std::cout << "============================= Diag Buffer" << std::endl;
  while (rod->getDiagBuffer(txt)) std::cout << txt << std::endl;
  std::cout << "============================= Xfer Buffer" << std::endl;
  while (rod->getXferBuffer(txt)) std::cout << txt << std::endl;
}


#define MAXPARS 3
int main(int argc, char **argv) {
  VmeInterface *vme;
  PixConfDBInterface *tdb;
  application * app;

  int nparameters = 0;
  std::string param[MAXPARS];
  int range[2][MAXPARS];
  int steps[MAXPARS];
  int nevents = 10;
  std::string outfile = "bocscan.root";

  if (argc > 1) {
    cfgFileName = argv[1];
  } else {
    cfgFileName = "ExampleConf.cfg";
  }


  for(int narg = 2; narg < argc; narg++){
    //AKDEBUG    if(nparameters>2){
    if(nparameters>1){
      //      cout << " Please specify ONLY 2 parameters to scan!" << endl;
      cout << " Please specify ONLY 1 parameter to scan!" << endl;
      return 0;
    }
    if (strcmp(argv[narg],"-D")==0 || strcmp(argv[narg],"-d")==0)
      {
	param[nparameters] = "DataDelay";
	sscanf(argv[narg+1],"%d",&range[0][nparameters]);
	sscanf(argv[narg+2],"%d",&range[1][nparameters]);
	sscanf(argv[narg+3],"%d",&steps[nparameters]);
	narg+=3;
	nparameters++;
      }
    else if (strcmp(argv[narg],"-T")==0 || strcmp(argv[narg],"-t")==0) 
      {
	param[nparameters] = "RxThreshold";
	sscanf(argv[narg+1],"%d",&range[0][nparameters]);
	sscanf(argv[narg+2],"%d",&range[1][nparameters]);
	sscanf(argv[narg+3],"%d",&steps[nparameters]);
	narg+=3;
	nparameters++;
      }
    else if (strcmp(argv[narg],"-B")==0 || strcmp(argv[narg],"-b")==0) 
      {
	param[nparameters] = "BRegClockPhase";
	sscanf(argv[narg+1],"%d",&range[0][nparameters]);
	sscanf(argv[narg+2],"%d",&range[1][nparameters]);
	sscanf(argv[narg+3],"%d",&steps[nparameters]);
	narg+=3;
	nparameters++;
      }
    else if (strcmp(argv[narg],"-E")==0 || strcmp(argv[narg],"-e")==0) 
      {
	sscanf(argv[narg+1],"%d",&nevents);
	narg++;
      }
    else if (strcmp(argv[narg],"-O")==0 || strcmp(argv[narg],"-o")==0) 
      {
	outfile=argv[narg + 1];
	narg++;
      }
  }


  if(nparameters==0){
    
    cout<< " Please specify what to scan! [Upto 2 parameters!]"<<endl;
    cout<< "Please use : " << argv[0] << " configfile," 
	<< " [-o/-O rootoutputfile], " 
	<< " [-e/-E nevents], "
	<< " [-d/-D min max step] for Data Delay, "
	<< " [-t/-T min max step] for RxThreshold,"
	<< " [-b/-B min max step] for B-Clock " << endl;
    return 0;
  }


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
  
  TFile mf(outfile.c_str(), "RECREATE");

  BOCHIST = new TH2D("BOC hist","parameter", steps[0], range[0][0]-0.5 ,range[1][0]+ 0.5,
		     pattern_length, -0.5, pattern_length + 0.5);

  try{
    int i;
    PixModuleGroup *grp = app->grp;
    RodPixController *rod = dynamic_cast<RodPixController *>(grp->getPixController());

    if (rod) {

      // Module configuration
      grp->getPixController()->setConfigurationMode();
      grp->getPixController()->sendModuleConfig(0x0);
            
      // Dump text buffers    
      dumpTextBuffers(rod);
      
      // Manually inject events
      std::cout << "++++++++++++++++++++++++++++++++++++++ Single events" << std::endl;

      unsigned int sl[8][4];
      // Offsets of BocLink to OutputLink
      int bl[8] = { 2, -6, 6, 6, 2, -6, 6, 6};
     
      Config &rodConf = grp->getPixController()->config();
      // write formatter link mapping as per configs
      if(rodConf["fmt"].name()!="__TrashConfGroup__"){
	for(int fID=0;fID<8;fID++){
	  std::string subName;
	  std::stringstream b;
	  b << fID;
	  subName = "linkMap_";
	  subName += b.str();
	  if(rodConf["fmt"][subName].name()!="__TrashConfObj__"){
	    int fmtVal = ((ConfInt&)rodConf["fmt"][subName]).getValue();
	    // Normal Linkmaps are 0x3210000 and 0xba98000
	    // Links for a given formatter start a 12*fmt
	    for (int j = 0; j<4; j++) sl[fID][j] = 12*fID + ((fmtVal >> (16 + 4*j)) & 0x0f);
	  }
	}
      }

      ConfInt &out = dynamic_cast<ConfInt &>(grp->module(0)->config()["general"]["OutputLink1"]);
      unsigned int il = sl[out.value()/16][out.value()%16]; 
      std::cout << "OutPutLink " << out.value() << " InMem " << il << std::endl;

      for (int np = 0; np < nparameters; np++){
	for (i=0; i<steps[np]; i++) {

	//	for (int li=0; li<96; li++)rod->setBocRegister("DataDelay",li,jj);
	// BOC has a link offset ??
	  int value = ((range[1][np]-range[0][np]+1)/steps[np])*i;
	  cout << "Scanning BOC par: " << param[np] << " val: " << value << endl; 
	  rod->setBocRegister(param[np], il + bl[out.value()/16], value);

	  for(int k =0; k < nevents; k++){
	    //	grp->module(0)->pixMCC()->globalResetMCC();
	    grp->module(0)->pixMCC()->setRegister("LV1_Contiguous", 0);
	    grp->module(0)->pixMCC()->enableDataTaking();
	    grp->module(0)->pixMCC()->bcr();
	    grp->module(0)->pixMCC()->ecr();
	    rod->setCalibrationDebugMode();
	    
	    grp->module(0)->pixMCC()->setFifo("FIFO07", 0x111111);
	    //       grp->module(0)->pixMCC()->setFifo("FIFO07", 0x155555);
	    grp->module(0)->pixMCC()->enableDataTaking();
	    rod->setCalibrationDebugMode();
	    //       //rod->setReadBackDebugMode();
	    std::cout << "+++++++++++++++++++++++++++++ Event " << k
		      << "  ++++ Step " << i << std::endl;
	    // 	  rod->writeRegister(CFG_READBACK_CNT, 0, 16, 0xff);
	    //        rod->writeRegister(RRIF_CMND_1, 25, 3, 0x5);
	    //        rod->writeRegister(RRIF_CMND_1, 19, 1, 0x1);
	    grp->module(0)->pixMCC()->trigger();   
	    //	  grp->module(0)->pixMCC()->getFifo("FIFO07");
	    //        rod->writeRegister(RRIF_CMND_1, 19, 1, 0x0);
	    // 	  rod->writeRegister(RRIF_CMND_1, 25, 3, 0x1);
	    
	    if (il < 48) {
	      dumpFifo(rod, INPUT_MEM, BANK_A, 0x8000, "InMem_A",il,i);
	    } else { 
	      dumpFifo(rod, INPUT_MEM, BANK_B, 0x8000, "InMem_B",il-48,i);
	    }
	  }
	}
      }
      // Wait
      sleep(3);
    } else {
      std::cout << "A ROD controller is required for this test" << std::endl;
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

  BOCHIST->Write();
  mf.Write();
  mf.Close();


  return 0;
}
