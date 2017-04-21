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

std::string cfgFileName;

bool mccDecode(Bits b, bool dump, std::string &dump_out) {
  
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
          if (debug) std::cout << "Bcid = " << bcid << std::endl;
          if (dump) {
            str << "\tBcid\n\t";
            str << bit << "       \tSync\n\t";
          }
        } else {
          status = -1;
          if (debug) std::cout << "Unable to decode Bcid\n";
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
              std::string filename, int link = -1, int id=-1) {
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
    //if (link > 0) {
    std::ostringstream tnam;
    tnam << filename;
    if (id >= 0) tnam << "_" << id;
    tnam << "_" << link;
    tnam << ".txt";
    std::ofstream tout(tnam.str().c_str());
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
    tout << "-----------------------------------------------------------------------------" << std::endl;
    tout << "FIFO Dump -------------------------------------------------------------------" << std::endl;
    tout << "-----------------------------------------------------------------------------" << std::endl;
    tout << bits << std::endl;
    std::string dump;
    mccDecode(bits, true, dump);
    tout << "-----------------------------------------------------------------------------" << std::endl;
    tout << "MCC decoding ----------------------------------------------------------------" << std::endl;
    tout << "-----------------------------------------------------------------------------" << std::endl;
    tout << dump << std::endl;
    tout.close();
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

  if (argc > 1) {
    cfgFileName = argv[1];
  } else {
    cfgFileName = "ExampleConf.cfg";
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

      // Module configuration
      grp->getPixController()->setConfigurationMode();
      grp->getPixController()->sendModuleConfig(0x0);
            
      // Dump text buffers    
      dumpTextBuffers(rod);
      
      // Manually inject events
      std::cout << "++++++++++++++++++++++++++++++++++++++ Single events" << std::endl;
//       grp->module(0)->pixMCC()->globalResetMCC();
//       grp->module(0)->pixMCC()->setRegister("CSR", 0x1c);
//       grp->module(0)->pixMCC()->setRegister("FEEN", 0xffff);
//       grp->module(0)->pixMCC()->getRegister("FEEN");
//       grp->module(0)->pixMCC()->setRegister("LV1", 0xf00);
//       grp->module(0)->pixMCC()->setRegister("WBITD", 0);//xffff);
//       grp->module(0)->pixMCC()->setRegister("WRECD", 0);//xffff);
      grp->module(0)->pixMCC()->setRegister("LV1_Contiguous", 0);
      grp->module(0)->pixMCC()->enableDataTaking();
      grp->module(0)->pixMCC()->bcr();
      grp->module(0)->pixMCC()->ecr();
      
      rod->setCalibrationDebugMode();
      ConfInt &out = dynamic_cast<ConfInt &>(grp->module(0)->config()["general"]["OutputLink1"]);
      unsigned int sl[8] = { 2, 14, 6, 18, 50, 62, 54, 66 };
      unsigned int il = sl[out.value()/16] + out.value()%16; 
      for (i=0; i<2; i++) {
       	std::cout << "+++++++++++++++++++++++++++++ Event " << i << std::endl;
       	grp->module(0)->pixMCC()->trigger();   
        if (il < 48) {
	  dumpFifo(rod, INPUT_MEM, BANK_A, 0x8000, "InMem_A",il,i);
	  dumpFifo(rod, INPUT_MEM, BANK_B, 0x8000, "InMem_B",-1,i);
	} else { 
	  dumpFifo(rod, INPUT_MEM, BANK_A, 0x8000, "InMem_A",-1,i);
	  dumpFifo(rod, INPUT_MEM, BANK_B, 0x8000, "InMem_B",il-48,i);
	}
      }
      
      // Wait
      sleep(3);

      // Dump text buffers
      dumpTextBuffers(rod);

      // Dump Slave 0 burst mem
      dumpSlaveMem(rod, 0,0x18000,0x8000,"Burst");

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
