#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace PixLib;
using namespace std;

int main(int argc, char **argv) {

  string cfgFile = argv[1];
  bool dumpCont = (cfgFile=="-dump");

  if(argc!=2 && (argc!=3 || !dumpCont)){
    cerr << "Usage: FEI4cfgTransf [RootDB-file] or" << endl;
    cerr << "Usage: FEI4cfgTransf -dump [RootDB-file]" << endl;
    return -1;
  }

  if(dumpCont) cfgFile = argv[2];

  FILE *testf = fopen(cfgFile.c_str(),"r");

  if(testf==0){
    cerr << "File " << cfgFile << " doesn't exit, exiting" << endl;
    return -1;
  }
  fclose(testf);

  const int ngr = 38;
  string grOldNames[ngr]={"CLK0_S012",
			  "CLK1_S012",
			  "EFUSECREF",
			  "EFUSEVREF",
			  "CLKPLLCoreEnable",
			  "PlsrExtAnaCalSW",
			  "PlsrExtDigCalSW",
			  "Disable8B10B",
			  "PLLbias",
			  "PLLCPBias",
			  "DAC8spare4",
			  "DAC8spare3",
			  "Rd_Errors",
			  "Rd_skipped",
			  "spare",
			  "DAC8spare1",
			  "DAC8spare2",
			  "DAC8spare5",
			  "FENDSpare1",
			  "FENDSpare2",
			  "FENDSpare3",
			  "CLKspare",
			  "PLLSpare",
			  "LVDSspare",
			  "LVDSspare2",
			  "CFGspare2",
			  "PlsrSpare",
			  "PxS0",
			  "PxS1",
			  "FE_Clk_pulse",
			  "vthin_AltCoarse",
			  "vthin_AltFine",
			  "ColMask0",
			  "ColMask1",
			  "ColMask2",
			  "EFUSEDC0",
			  "EFUSEDC1",
			  "EFUSEDC2"  };
  string grNewNames[ngr]={"CLK0",
			  "CLK1",
			  "EfuseCref",
			  "EfuseVref",
			  "EN_PLL",
			  "ExtAnaCalSW",
			  "ExtDigCalSW",
			  "no8b10b",
			  "PllIbias",
			  "PllIcp",
			  "PlsrDacBias",
			  "PlsrVgOPamp",
			  "ReadErrorReq",
			  "ReadSkipped",
			  "Reg13Spare",
			  "Reg17Spare",
			  "Reg18Spare",
			  "Reg19Spare",
			  "Reg21Spare",
			  "Reg22Spare1",
			  "Reg22Spare2",
			  "Reg27Spare",
			  "Reg28Spare",
			  "Reg29Spare1",
			  "Reg29Spare2",
			  "Reg2Spare",
			  "Reg31Spare",
			  "S0",
			  "S1",
			  "SR_Clock",
			  "Vthin_AltCoarse",
			  "Vthin_AltFine",
			  "DisableColumnCnfg0",
			  "DisableColumnCnfg1",
			  "DisableColumnCnfg2",
			  "SELB0",
			  "SELB1",
			  "SELB2"  };

  RootDB *db = new RootDB(cfgFile.c_str(),"UPDATE");
  
  DBInquire *root = db->readRootRecord(0);
  for(recordIterator i = root->recordBegin(); i!=root->recordEnd();i++){
    if((*i)->getName()=="application"){
      for(recordIterator ii = (*i)->recordBegin(); ii != (*i)->recordEnd(); ii++){
	if((*ii)->getName()=="PixModuleGroup"){
	  for(recordIterator iii = (*ii)->recordBegin(); iii != (*ii)->recordEnd(); iii++){
	    if((*iii)->getName()=="PixModule"){
	      for(recordIterator iv = (*iii)->recordBegin(); iv != (*iii)->recordEnd(); iv++){
		if((*iv)->getName()=="PixFe"){
		  for(recordIterator v = (*iv)->recordBegin(); v != (*iv)->recordEnd(); v++){
		    if((*v)->getName()=="GlobalRegister"){
		      if(dumpCont){
			for(fieldIterator fi = (*v)->fieldBegin(); fi!=(*v)->fieldEnd(); fi++){
			  DBField *f = *fi;
			  int value;
			  db->DBProcess(f,READ,value);
			  cout << f->getName() << "\t" << value << endl;
			}
		      } else{
			//(*v)->dump(std::cout);
			for(int i=0;i<ngr;i++){
			  fieldIterator fi = (*v)->findField("GlobalRegister_"+grOldNames[i]);
			  if(fi!=(*v)->fieldEnd()){
			    // read exisitng field
			    DBField *f = *fi;
			    int value;
			    db->DBProcess(f,READ,value);
			    //cout << "Found field " << grOldNames[i] << " with value " << value << endl;
			    // remove old field
			    (*v)->eraseField(fi);
			    // add field with new name and old content
			    f = db->makeField("GlobalRegister_"+grNewNames[i]);
			    db->DBProcess(f,COMMIT,value);
			    (*v)->pushField(f);
			  } else
			    cerr << "Field " << grOldNames[i] << " couldn't be found" << endl;
			}
		        // special treatment: Chip_SN is new
			int value=0;		      
			DBField *f = db->makeField("GlobalRegister_Chip_SN");
			db->DBProcess(f,COMMIT,value);
			(*v)->pushField(f);
			// add bit 13 to CMDcnt
			fieldIterator fi = (*v)->findField("GlobalRegister_CMDcnt");
			if(fi!=(*v)->fieldEnd()){
			  db->DBProcess(*fi,READ,value);
			  fi = (*v)->findField("GlobalRegister_CMDcnt13");
			  if(fi!=(*v)->fieldEnd()){
			    int addval;
			    db->DBProcess(*fi,READ,addval);
			    value += (addval<<13);
			    (*v)->eraseField(fi);
			    fi = (*v)->findField("GlobalRegister_CMDcnt");
			    db->DBProcess(*fi,COMMIT,value);
			  }else
			    cerr << "Field CMDcnt13 couldn't be found" << endl;
			}else
			  cerr << "Field CMDcnt couldn't be found" << endl;
			// update inquire
			db->DBProcess(*v,COMMITREPLACE);    
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  delete db;

  return 0;
}
