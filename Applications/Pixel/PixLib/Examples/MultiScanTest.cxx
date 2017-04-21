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
#include "Config/Config.h"
#include "Config/ConfObj.h"
#include "Config/ConfGroup.h"
#include "RodCrate/RodModule.h"
#include "registerIndices.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>
#include <pthread.h>

using namespace PixLib;
using namespace SctPixelRod;

std::vector<std::ofstream *>out;
std::vector<std::string>fileName;
std::vector<PixConfDBInterface *>tdbv;
std::vector<PixModuleGroup*>grpv;
std::vector<PixScanConfig*>cfgv;

VmeInterface *vme;

void dumpTextBuffers(RodPixController *rod, std::ofstream *log) {
  std::string txt;
    
  *log << "============================= Err Buffer" << std::endl;
  while (rod->getErrBuffer(txt)) *log << txt << std::endl;
  *log << "============================= Info Buffer" << std::endl;
  while (rod->getInfoBuffer(txt)) *log << txt << std::endl;
  *log << "============================= Diag Buffer" << std::endl;
  while (rod->getDiagBuffer(txt)) *log << txt << std::endl;
  *log << "============================= Xfer Buffer" << std::endl;
  while (rod->getXferBuffer(txt)) *log << txt << std::endl;
}

void calcThreshold(unsigned int m, PixScanConfig *cfg, RodPixController *rod, std::ofstream *log) {
  std::vector< Histo* > &thrHis = cfg->thr();
  std::vector< Histo* > &noiseHis = cfg->noise();
  if (rod->moduleActive(m)) {
    double sthr=0, snoise=0;
    int ntot = 0;
    for (int col = 0; col<144; col++) {
      for (int row = 0; row<320; row++) {
        if ((*noiseHis[m])(col,row) != 0) {
	  sthr += (*thrHis[m])(col,row);
	  snoise += (*noiseHis[m])(col,row);
	  ntot++;
        }
      }
    }
    if (ntot > 0) {
      sthr /= ntot;
      snoise /= ntot;
      *log << "Module " << m << " mean thr/noise = " << sthr << " / " << snoise << std::endl;
    }
  }
}

extern "C" void* scan(void* idx) {
  unsigned int index = 9999;
  std::ofstream *log;
  index = (int)idx;
  if (index < 0 || index >= out.size()) {
    pthread_exit((void *)2);
  }
  log = out[index];
  PixModuleGroup *grp = grpv[index];
  RodPixController *rod = dynamic_cast<RodPixController *>(grp->getPixController());
  PixScanConfig *cfg = cfgv[index];
  try {
      if (rod) {
      // Init hardware
      grp->initHW();
      grp->downloadConfig(); 

      // Dump text buffers    
      dumpTextBuffers(rod, log);

      // Module configuration
      grp->getPixController()->setConfigurationMode();
      grp->getPixController()->sendModuleConfig(0x0);
       
      int i;
      for (i=0; i<10; i++) {

	int j;
	for (j=0; j<4; j++) {
	  // Enable trapping
	  rod->setupTrapping(j,*cfg);
	}

	// Standard dump
	dumpTextBuffers(rod, log);
	grp->getPixController()->writeScanConfig(*cfg);

        // Dump text buffers    
        dumpTextBuffers(rod, log);
	
        // Start scan
        *log << "++++++++++++++++++++++++++++++++++++++ Start scan " << i << std::endl;
        rod->setCalibrationMode();
        grp->getPixController()->startScan();
	
        // Wait
        int nloop, stat = 1;
        do {
          nloop = grp->getPixController()->nTrigger();
	  *log << "Mask stage = " << (nloop>>12) << " Bin = " << (nloop&0xfff) << std::endl;
           sleep(5);
          stat = grp->getPixController()->runStatus();
        } while (stat == 1);
	
        // Dump text buffers
        dumpTextBuffers(rod, log);
	
        // Get the histograms
        *log << "Downloading Histograms..." << std::endl;
        try {
          for (unsigned int mod=0; mod<32; mod++) {
            if (rod->moduleActive(mod)) {
	      cfg->getHisto(rod, mod);
	      calcThreshold(mod, cfg, rod, log);
            }
          }
        }
        catch (BaseException & exc){
          *log << exc << endl;
          pthread_exit((void *)1);
        }
      }
    } else {
      *log << "A ROD controller is required for this test" << std::endl;
    }
  }
  catch (BaseException & exc){
    *log << exc << endl;
    pthread_exit((void *)1);
  }
  catch (...){
    *log << "error running the application, exiting" << endl;
    pthread_exit((void *)1);
  }

  pthread_exit((void *)0);
  return NULL;
}

int main(int argc, char **argv) {
  std::vector<pthread_t *> pt;

  if (argc > 1) {
    try {
      vme = new RCCVmeInterface();
    }
    catch (VmeException &v) {
      std::cout << "Error during VmeInterface initialization" << std::endl;
    }
    int i;
    for (i=1; i<argc; i++) {
      pthread_t *p;
      std::string fn(argv[i]);
      fileName.push_back(fn);
      std::string fnl = fn+".log";
      std::ofstream *of = new std::ofstream;
      of->open(fnl.c_str());
      out.push_back(of);
      PixConfDBInterface *tdb;      
      PixModuleGroup *grp;
      try {
	std::string dbName = fn;
	if (dbName.substr(dbName.size()-5, dbName.size()) == ".root") {
	  tdb = new RootDB(dbName.c_str());
	} else if (dbName.substr(dbName.size()-4, dbName.size()) == ".cfg") {
	  tdb = new TurboDaqDB(dbName.c_str());
	} else {
	  *of << "Invalid config file extension: please provide a .cfg o .root file" << std::endl; 
	  exit(-1);
	}
      }
      catch (BaseException & exc){
	*of << exc << std::endl; 
	exit(-1);
      }
      catch (...){
	*of << "error during the pre-initialing of the application, exiting" << endl;
      }
      try{
	DBInquire* root = tdb->readRootRecord(1);
	recordIterator pmgIter = root->recordBegin();
	for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
	  // Look for application inquire
	  if ((*it)->getName() == "application") {
	    *of  << "Accessing Application Database: " << (*it)->getName().c_str() << std::endl;
	    pmgIter = (*it)->recordBegin();
	    break;
	  } else if ((*it)->getName() == "PixModuleGroup") {
	    pmgIter = it;
	    break;
	  }
	}
	*of << "Creating PixModuleGroup object from records" << std::endl;
	grp = new PixModuleGroup(tdb, *pmgIter, *vme);
	//grp->initHW();
	//grp->downloadConfig(); 
     }
      catch (BaseException & exc){
	*of << exc << endl;
	exit(-1);
      }
      catch (...){
	*of << "error initializing the application, exiting" << endl;
	exit(-1);
      }
      // Scan setup
      DBInquire* root = tdb->readRootRecord(1);
      PixScanConfig *cfg = NULL;
      for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
	if((*it)->getName() == "PixScanConfig") {
          cfg = new PixScanConfig(*it);
          break;
	}      
      }
      if (cfg == NULL) {
      	*of << "Cannot find ScanTest record" << std::endl;
        pthread_exit((void *)1);
      }
      cfgv.push_back(cfg);
      grpv.push_back(grp);
      tdbv.push_back(tdb);
      pthread_create(p, NULL, scan, (void *)(i-1));
      pt.push_back(p);
    }
    sleep(1000);
    unsigned int j;
    for (j=0; j<pt.size(); j++) {
      void **dummy;  
      pthread_join(*pt[j], dummy);
    }
  }
}
