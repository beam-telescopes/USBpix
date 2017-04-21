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

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

using namespace PixLib;
using namespace SctPixelRod;

bool mccDecode(Bits b, bool dump, std::string &dump_out, int &nhits, int &nerr) {
  
  int head,lev1,bcid,mccflag,fen,feflag,row,col,tot;
  int i,j,bit,exp,nzero,status;
  bool debug = false;
  bool decoded = false;
  int nzTrail;
  std::ostringstream str;
  
  status = 0;
  head = 0;
  mccflag = 0;
  feflag = 0;

  nzTrail = 22;

  if (dump) str << "\t";

  for (i=0;i<b.size();i++) {
    
    if (status < 0) {
      if (dump) {
        for (j=0;j+i<b.size();j++) {
          bit = b.get(j+i,j+i);
          if (j%8==0 && j!=0) str << "\n\t";
          str << bit;
        }
      }
      break;
    }

    bit = b.get(i,i);
    if (bit == 0) {
      nzero++;
    } else {
      nzero = 0;
    }
    
    switch (status) {
      
    case 0: /* Wait for header */
      head = ((head << 1) | bit) & 0x1f;
      if (dump) str << bit;
      if (head == 0x1d) {
        status = 1;
        lev1 = 0;
        exp = 7;
        if (debug) std::cout << "Header found\n";
        if (dump) str << "   \tHeader\n\t";
      }
      break;
      
    case 1: /* Decode Lev1 */
      if (exp >= 0) {
        lev1 = lev1 + (bit << exp--);
        if (dump) str << bit;
      } else {
        if (bit == 1) {
          status = 2;
          bcid = 0;
          mccflag = 0;
          exp = 7;
          if (debug) std::cout << "Lev1 = " << lev1 << std::endl;
          if (dump) {
            str << "\tLev1\n\t";
            str << bit << "       \tSync\n\t";
          }
        } else {
	  status = -1;
	  if (debug) std::cout << "Unable to decode Lev1\n";
	  nerr++;
	  if (dump) {
	    str << "\tLev1\n\t";
	    str << bit << "       \tSync Error !!\n\t";
	  }
        }
      }
      break;
      
    case 2: /* Decode Bcid */
      if (exp >= 0) {
        bcid = bcid + (bit << exp--);
        if (dump) str << bit;
      } else {
        if (bit==1) {
          status = 3;
          fen = 0;
          mccflag = 0;
          exp = 7;
          nhits++;
          if (debug) std::cout << "Bcid = " << bcid << std::endl;
          if (dump) {
            str << "\tBcid\n\t";
            str << bit << "       \tSync\n\t";
          }
        } else {
          status = -1;
          if (debug) std::cout << "Unable to decode Bcid\n";
	  nerr++;
          if (dump) {
            str << "\tBcid\n\t";
            str << bit << "       \tSync Error !!\n\t";
          }
        }
      }
      break;
      
    case 3: /* Decode Fe# */
      if (exp >= 0) {
        fen = fen + (bit << exp--);
        if (dump) str << bit;
      } else {
        if (bit == 1) {
          if ((fen & 0xf0) == 0xe0) {
            fen = fen & 0x0f;
            status = 4;
            row = 0;
            exp = 7;
            if (debug) std::cout << "Fe# = " << fen << std::endl;
            if (dump) {
              str << "\tFe#\n\t";
              str << bit << "       \tSync\n\t";
            }
          } else {
            mccflag = fen; 
            fen = 0;
            exp = 7;      
            if (debug) std::cout << "MccFlag = " << mccflag << std::endl;
            if (dump) {
              str << "\tMccFlag\n\t";
              str << bit << "       \tSync\n\t";
            }
          }
        } else {
          if (nzero > 7) {
            if (nzero == nzTrail) {
              head = 0;
              status = 0;
              if (dump) {
                str << bit << "       \tEnd of event\n\t";
              }
              decoded = true;
	    }
	  } else {
	    status = -1;
	    if (debug) std::cout << "Unable to decode Fe#\n";
            nerr++;
	    if (dump) {
	      str << "\tFe#\n\t";
	      str << bit << "       \tSync Error !!\n\t";
	    }
          }
        }
      }
      break;
      
    case 4: /* Decode Row */
      if (exp >= 0) {
        row = row + (bit << exp--);
        if (dump) str << bit;
      } else {
        if ((row & 0xf8) == 0xf0) {
          status = 7;
          exp = 4;
          feflag = (row << 13) + (bit << exp--); 
          if (dump) str << bit;
        } else if ((row & 0xf8) == 0xf8) {
          status = 7;
          exp = 12;
          feflag = (row << 21) + (bit << exp--); 
          if (dump) str << bit;
        } else if ((row & 0xf0) == 0xe0) {
          if (bit==1) {
            fen = row & 0x0f;
            row = 0;
            status = 4;
            exp = 7;
            if (debug) std::cout << "Fe# = " << fen << std::endl;
            if (dump) {
              str << "\tFe#\n\t";
              str << bit << "       \tSync\n\t";
            }
          } else {
	    if (nzero > 7) {
	      if (nzero == nzTrail) {
		head = 0;
		status = 0;
		if (dump) {
		  str << bit << "       \tEnd of event\n\t";
		}
		decoded = true;
	      }
	    } else {
	      status = -1;
	      if (debug) std::cout << "Unable to decode Fe#\n";
              nerr++;
	      if (dump) {
		str << "\tFe#\n\t";
		str << bit << "       \tSync Error !!\n\t";
	      }
	    }
          }
        } else {
          if (dump) str << bit;
          status = 5;
          exp = 4;
          col = (bit << exp--);     
        }
      }
      break;
      
    case 5: /* Decode Col */
      if (exp >= 0) {
        col = col + (bit << exp--);
        if (dump) str << bit;
      } else {
        if (dump) str << bit;
	status = 6;
	exp = 7;
        tot = (bit << exp--);
      }
      break;
      
    case 6: /* Decode Tot */
      if (exp >= 0) {
        tot = tot + (bit << exp--);
        if (dump) str << bit;
      } else {
        if (bit == 1) {
          status = 4;
          row = 0;
          exp = 7;
          if (debug) std::cout << "Row = " << row << std::endl;
          if (debug) std::cout << "Col = " << col << std::endl;
          if (debug) std::cout << "Tot = " << tot << std::endl;
          if (dump) {
            str << "\tRow# Col# Tot#\n\t";
            str << bit << "       \tSync\n\t";
          }
        } else {
	  if (nzero > 7) {
	    if (nzero == nzTrail) {
	      head = 0;
	      status = 0;
	      if (dump) {
		str << bit << "       \tEnd of event\n\t";
	      }
	      decoded = true;
	    }
	  } else {
	    status = -1;
	    if (debug) std::cout << "Unable to decode Tot\n";
            nerr++;
	    if (dump) {
	      str << bit << "       \tSync Error !!\n\t";
	    }
	  }
        }
      }
      break;
      
    case 7:   /* Decode FE flag */
      if (exp >= 0) {
        feflag = feflag + (bit << exp--);
        if (dump) str << bit;
      } else {
        if (bit == 1) {
          feflag = 0;
          status = 3;
          exp = 7;
          row = 0;
          fen = 0;      
          if (debug) std::cout << "FeFlag = " << feflag << std::endl;
          if (dump) {
            str << "\tFeFlag\n\t";
            str << bit << "       \tSync\n\t";
          }
        } else {
	  status = -1;
	  if (debug) std::cout << "Unable to decode FeFlag\n";
          nerr++;
	  if (dump) {
	    str << "\tFeFlag\n\t";
	    str << bit << "       \tSync Error !!\n\t";
	  }
        }
      }
      break;
    }
  } 
  
  dump_out = str.str();

  return decoded;

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

void dumpSlaveMem(RodPixController* rod, 
                  int slave, unsigned int start, unsigned int len, 
                  std::string filename, int id=-1) {
  std::vector<unsigned int> out;  
  //rod->readSlaveMem(slave, start, len/4, out);
  rod->readSlaveMemDirect(slave, start, len/4, out);
  std::ostringstream onam;
  onam << filename;
  if (id >= 0) onam << "_" << id;
  onam << ".bin";
  std::ofstream fout(onam.str().c_str(),ios::binary);
  for (unsigned int i=0; i<out.size(); i++) {
    fout.write((char *)&(out[i]),4); 
  }      
}

void dumpMasterMem(RodPixController* rod, 
                   unsigned int start, unsigned int len,
                   std::string filename, int id=-1) {
  std::vector<unsigned int> out;
  rod->readMasterMem(start, len/4, out);
  std::ostringstream onam;
  onam << filename;
  if (id >= 0) onam << "_" << id;
  onam << ".bin";
  std::ofstream fout(onam.str().c_str(),ios::binary);
  for (unsigned int i=0; i<out.size(); i++) {
    fout.write((char *)&(out[i]),4); 
  }      
}

void dumpFifo(RodPixController *rod, 
              unsigned int fifo, unsigned int bank, unsigned int len, 
              std::string filename, int *nhits, int *nerr, bool *scanLink, int link = -1, int id=-1) {
  std::vector<unsigned int> out;
  rod->readFifo(fifo, bank, len, out);
  std::ostringstream onam;
  onam << filename;
  if (id >= 0) onam << "_" << id;
  onam << ".bin";
  std::ofstream fout(onam.str().c_str(),ios::binary);
  for (unsigned int i=0; i<out.size(); i++) {
    fout.write((char *)&(out[i]),4); 
  }      
  fout.close();
  for (link=0; link<48; link++) {
    if (scanLink[link] > 0) {
      std::ostringstream tnam;
      tnam << filename;
      if (id >= 0) tnam << "_" << id;
      tnam << "_" << link;
      tnam << ".txt";
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
      mccDecode(bits, false, dump, nhits[link], nerr[link]);
    }
  } 
              }

void stdDump(RodPixController *rod) {
  dumpMasterMem(rod, 0x400000, 0x2000,"Formatter");
  dumpMasterMem(rod, 0x402000, 0x270, "Efb");
  dumpMasterMem(rod, 0x402400, 0x120, "Router");
  dumpMasterMem(rod, 0x404400, 0x800, "Rcf");

  /*
  dumpMasterMem(rod, 0x80000000,0x60,  "CommReg");
  dumpMasterMem(rod, 0x80000060,0x800, "Idata");
  dumpMasterMem(rod, 0x80008000,0x8000,"SP");
    
  dumpMasterMem(rod, 0x02000000,0x40000,"XcodeMaster");

  dumpSlaveMem(rod, 0,0x10000,0x60,  "CommReg_0");
  dumpSlaveMem(rod, 0,0x10060,0x800, "Idata_0");
  dumpSlaveMem(rod, 0,0x18000,0x8000,"Burst_0");
    
  dumpSlaveMem(rod, 0,0xa0000000,0x40000,"XcodeSlave_0");
  */
 
  dumpMasterMem(rod, 0x80000000,0x80,  "MDSP-CommReg");
  dumpMasterMem(rod, 0x80000080,0x800, "MDSP-Idata");
  dumpMasterMem(rod, 0x80008000,0x8000,"MDSP-burst");
  dumpMasterMem(rod, 0x02102000,0x3400,"MDSP-SP0");
  dumpMasterMem(rod, 0x02105400,0x3400,"MDSP-SP1");
  dumpMasterMem(rod, 0x02000000,0x40000,"MDSP-xcode");

  dumpSlaveMem(rod, 0,0x10000,0x80,  "SDSP0-CommReg");
  dumpSlaveMem(rod, 0,0x10080,0x800, "SDSP0-Idata");
  dumpSlaveMem(rod, 0,0x18000,0x8000,"SDSP0-Burst");
  dumpSlaveMem(rod, 0,0xa0000000,0x40000,"SDSP0-xcode");

  dumpSlaveMem(rod, 1,0x10000,0x80,  "SDSP1-CommReg");
  dumpSlaveMem(rod, 1,0x10080,0x800, "SDSP1-Idata");
  dumpSlaveMem(rod, 1,0x18000,0x8000,"SDSP1-Burst");
  dumpSlaveMem(rod, 1,0xa0000000,0x40000,"SDSP1-xcode");

  dumpSlaveMem(rod, 2,0x10000,0x80,  "SDSP2-CommReg");
  dumpSlaveMem(rod, 2,0x10080,0x800, "SDSP2-Idata");
  dumpSlaveMem(rod, 2,0x18000,0x8000,"SDSP2-Burst");
  dumpSlaveMem(rod, 2,0xa0000000,0x40000,"SDSP2-xcode");

  dumpSlaveMem(rod, 3,0x10000,0x80,  "SDSP3-CommReg");
  dumpSlaveMem(rod, 3,0x10080,0x800, "SDSP3-Idata");
  dumpSlaveMem(rod, 3,0x18000,0x8000,"SDSP3-Burst");
  dumpSlaveMem(rod, 3,0xa0000000,0x40000,"SDSP3-xcode");
}

void moduleDump(RodPixController *rod) {
  dumpMasterMem(rod, 0x02400000,0x800000,"cfgDB-MDSP");
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

int main(int argc, char **argv) {
  VmeInterface *vme;
  PixConfDBInterface *tdb;
  application * app;
  bool bocDelayScanAll = false;
  bool bocDelayScan = false;
  bool bocThrScanAll = false;
  bool bocThrScan = false;
  bool allLinks = true;
  bool testInLinks = false;
  bool scanLink[96];
  bool inLink[48];
  int delay[25];
  int thr[256];
  int delayC = 0;
  int thrC = 0;
  std::string cfgFileName = "";

  unsigned int il;
  for (il=0; il<96; il++) scanLink[il] = false;
  for (il=0; il<48; il++) inLink[il] = false;
  for (il=0; il<25; il++) delay[il] = -1;
  for (il=0; il<256; il++) thr[il] = -1;

  if (argc > 1) {
    cfgFileName = argv[1];
    for (int ip=2; ip<argc-1; ip++) {
      if (strcmp(argv[ip],"-D")==0 || strcmp(argv[ip],"-d")==0) {
        bocDelayScan = true;
        if (strcmp(argv[ip],"-D")==0) bocDelayScanAll = true;
        unsigned int d1,d2;
        if (ip+2 < argc) {
          sscanf(argv[ip+1],"%d",&d1);
          sscanf(argv[ip+2],"%d",&d2);
          ip+=2;
          if (d1>=0 && d1<25 && d2>=0 && d2<25) {
            for (il=d1; il<=d2; il++) {
              if (delay[il] < 0) {
                delay[il] = delayC++;
	      }
	    }
          }
        }
      } else if (strcmp(argv[ip],"-T")==0 || strcmp(argv[ip],"-t")==0) {
        bocThrScan = true;
        if (strcmp(argv[ip],"-T")==0) bocThrScanAll = true;
        unsigned int t1,t2,t3;
        if (ip+3 < argc) {
          sscanf(argv[ip+1],"%d",&t1);
          sscanf(argv[ip+2],"%d",&t2);
          sscanf(argv[ip+3],"%d",&t3);
          ip+=3;
          for (unsigned int th=t1; th<=t2; th += t3) {
            if (th>=0 && th<=255) {
              if (thr[th] < 0) {
                thr[th] = thrC++;
	      }
	    }
          }
        }
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
    cfgFileName = "ExampleConf.cfg";
  }
  if (allLinks == true) {
    for (il = 0; il<96; il++) scanLink[il] = true;
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

  try{
    int i;
    PixModuleGroup *grp = app->grp;
    RodPixController *rod = dynamic_cast<RodPixController *>(grp->getPixController());

    if (rod) {

      int ***nhits[48], ***nerr[48];
      if (delayC == 0) {
        delayC = 1;
        delay[0] = 0;
      }
      if (thrC == 0) {
        thrC = 1;
        thr[0] = 0;
      }
      for (int i1=0; i1<48; i1++) {
        nhits[i1] = new int**[delayC];
        nerr[i1] = new int**[delayC];
	for (int i2=0; i2<delayC; i2++) {
	  nhits[i1][i2] = new int*[thrC];
	  nerr[i1][i2] = new int*[thrC];
	  for (int i3=0; i3<thrC; i3++) {
	    nhits[i1][i2][i3] = new int[96];
	    nerr[i1][i2][i3] = new int[96];
	  }
	}
      }

      int ii, mi=1;
      if (testInLinks) mi = 48;
      for (ii=0; ii<mi; ii++) {
        if (!testInLinks || inLink[ii]) {
          if (inLink[ii]) {
            ConfInt &out = dynamic_cast<ConfInt &>(grp->module(0)->config()["general"]["InputLink"]);
            out.value() = ii;
            grp->downloadConfig();
            std::cout << std::endl << "++++++++++++++++++++++++++++++++++++++ Input link " << ii << std::endl;
          } else {
            ConfInt &out = dynamic_cast<ConfInt &>(grp->module(0)->config()["general"]["InputLink"]);
            std::cout << std::endl << "++++++++++++++++++++++++++++++++++++++ Input link " << out.value() << std::endl;
	  }

          // Module configuration
          grp->getPixController()->setConfigurationMode();
          grp->getPixController()->sendModuleConfig(0x0);

          // Dump text buffers    
          dumpTextBuffers(rod);

          // Manually inject events
          grp->module(0)->pixMCC()->setRegister("LV1_Contiguous", 15);
          grp->module(0)->pixMCC()->enableDataTaking();
          grp->module(0)->pixMCC()->bcr();
          grp->module(0)->pixMCC()->ecr();

          rod->setCalibrationDebugMode();
          int id, md=1, kd;
          if (bocDelayScan) md = 25;
          for (id=0; id<md; id++) {
            if (delay[id]>=0 || bocDelayScan == false) {
	      if (bocDelayScan) {
		int ic;
		for (ic=0; ic<96; ic++) {
		  if (bocDelayScanAll || scanLink[ic]) rod->setBocRegister("DataDelay", ic, id);
		}
	      }
              kd = delay[id];
              int it, mt=1, kt;
              if (bocThrScan) mt = 256;
	      for (it=0; it<mt; it++) {
		if (thr[it]>=0 || bocThrScan == false) {
		  if (bocThrScan) {
		    int ic;
		    for (ic=0; ic<96; ic++) {
		      if (bocThrScanAll || scanLink[ic]) rod->setBocRegister("RxThreshold", ic, it);
		    }
		  }
                  kt = thr[it];
		  for (i=0; i<96; i++) { 
		    nhits[ii][kd][kt][i] = 0;
		    nerr[ii][kd][kt][i] = 0;
		  }
		  for (i=0; i<4; i++) {
		    std::cout << "+++++++++++++++++++++++++++++ Event " << i << std::endl;
		    grp->module(0)->pixMCC()->trigger();
		    dumpFifo(rod, INPUT_MEM, BANK_A, 0x8000, "InMem_A",&(nhits[ii][kd][kt][0]), &(nerr[ii][kd][kt][0]), &(scanLink[0]), -1,i);
		    dumpFifo(rod, INPUT_MEM, BANK_B, 0x8000, "InMem_B",&(nhits[ii][kd][kt][48]), &(nerr[ii][kd][kt][48]), &(scanLink[48]), -1,i);
		  }
		}
	      }
	    }
	  }
	}
      }

      if (bocDelayScan) {
        mi=1;
        if (testInLinks) mi = 48;
        for (ii=0; ii<mi; ii++) {
	  if (!testInLinks || inLink[ii]) {
	    if (inLink[ii]) {
	      std::cout << std::endl << "Input link " << ii << "----------------------------------------------------" << std::endl;
	    } else {
	      ConfInt &out = dynamic_cast<ConfInt &>(grp->module(0)->config()["general"]["InputLink"]);
	      std::cout << std::endl << "Input link " << out.value() << "----------------------------------------------------" << std::endl;
	    }
	    std::cout << "         Delay value" << std::endl;
            std::cout << "OutLink   Thr  0    1    2    3    4    5    6    7    8    9   10   11   12   ";
            std::cout <<          "13   14   15   16   17   18   19   20   21   22   23   24" << std::endl;
	    for (i=0; i<96; i++) {
	      if (scanLink[i]) {
                for (int it=0; it<256; it++) {
                  if (thr[it]>=0 || (it == 0 && !bocThrScan)) {
                    std::cout << std::hex << std::setw(2) << i/12 << ":" << i%12 << "  " << std::dec;
                    int kt = thr[it];
		    if (bocThrScan) {
		      std::cout << std::setw(3) << it << "  ";
		    } else {
		      std::cout << "     ";
		    }
		    for (int id=0; id<25; id++) {
		      if (delay[id]>=0) {
                        int kd = delay[id];
			if (nerr[ii][kd][kt][i] > 0) {
			  std::cout << std::setw(5) << -nerr[ii][kd][kt][i];
			} else {
			  std::cout << std::setw(5) << nhits[ii][kd][kt][i];
			}
		      } else {
			std::cout << " ----";
		      }
		    }
		    std::cout << std::endl;
		  }
		}
	      }
	    }
          }
	}
      } else {
        if (testInLinks) {
	  std::cout << "        InLink" << std::endl;
	  std::cout << "OutLink ";
          for (ii=0; ii<48; ii++) {
            if (inLink[ii]) std::cout << std::setw(3) << ii << "  ";
          }
	  std::cout << std::endl;
	  for (i=0; i<96; i++) {
	    if (scanLink[i]) {
              std::cout << std::hex << std::setw(2) << i/12 << ":" << i%12 << "  " << std::dec;
	      for (ii=0; ii<48; ii++) {
		if (inLink[ii]) {
		  if (nerr[ii][0][0][i] > 0) {
		    std::cout << std::setw(5) << -nerr[ii][0][0][i];
		  } else {
		    std::cout << std::setw(5) << nhits[ii][0][0][i];
		  }
		}
	      }
	      std::cout << std::endl;
	    }
	  }
	} else {
	  ConfInt &out = dynamic_cast<ConfInt &>(grp->module(0)->config()["general"]["InputLink"]);
	  std::cout << std::endl << "Input link " << out.value() << "----------------------------------------------------" << std::endl;
	  std::cout << "OutLink  nev|nerr" << std::endl;
	  for (i=0; i<96; i++) {
            if (scanLink[i]) {
              std::cout << std::hex << std::setw(2) << i/12 << ":" << i%12 << "  " << std::dec;
	      if (nerr[0][0][0][i] > 0) {
		std::cout << std::setw(5) << -nerr[0][0][0][i];
	      } else {
		std::cout << std::setw(5) << nhits[0][0][0][i];
	      }
              std::cout << std::endl;
            }
          }
	}
      }

      // Dump text buffers
      dumpTextBuffers(rod);

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

  return 0;
}
