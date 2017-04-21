#include <DataContainer/PixDBData.h>
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"

#include <TH2F.h>
#include <TGraph.h> 
#include <TApplication.h>
#include <TStyle.h>
#include <TROOT.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace PixLib;
using namespace std;

string ScanName, GroupName, ModuleName;
int argc;

// Looks at the structure of the RootDB file and lists all scans
void fillData(string fname){
  string sname, gname, mname;
  vector<string> snames, gnames, mnames;
	
  RootDB * db = new RootDB(fname.c_str());
  DBInquire *root = db->readRootRecord(1);
  
  unsigned int id=0;

  for(recordIterator i = root->recordBegin(); i!=root->recordEnd();i++){
    if((*i)->getName()=="PixScanResult"){
      sname = (*i)->getDecName();
      getDecNameCore(sname);
      cout << "NEW SCAN: \"" << sname << "\"" << endl;
      for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
	if((*ii)->getName()=="PixModuleGroup"){
	  gname = (*ii)->getDecName();
	  getDecNameCore(gname);
	  cout << "  with module group: " <<  gname << endl;
	  for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();iii++){
	    if((*iii)->getName()=="PixModule"){
	      mname = (*iii)->getDecName();
	      getDecNameCore(mname);
	      snames.push_back(sname);
	      gnames.push_back(gname);
	      mnames.push_back(mname);
	      cout << "     with module: " <<  mname << endl;
	      cout << "INDEX " << id << endl;
	      id++;
	    }
	  }
	}
      }
    }
  }
  delete db;

  id = snames.size();
  if (snames.size()>0){
    while(id>=snames.size()){
      cout << "Enter index of scan/module to use: ";
      cin >> id;
      if(id>=snames.size()) cerr << "illegal index, try again" << endl;
    }
    ScanName = snames[id];
    GroupName = gnames[id];
    ModuleName = mnames[id];
    cout << "Selected scan: " << ScanName << ":" << GroupName << ":" << ModuleName << endl;
  }
}	

// Gets the OCCUPANCY histogram from the path found in fillData() and writes it to the result.root file
void saveOcc(string fname, string sname, string gname, string mname){ 
    PixDBData data("scan", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
    if(data.getScanLevel()==0){
      cerr << "Selected scan has no loops activated, can't proceed!" << endl;
      return;
    }
    TFile f("result.root", "RECREATE");
    for(int i_scan=0;i_scan<data.getScanSteps(0); i_scan++){
      // this will retrieve histograms on loop level 0
      TH2F * myData = dynamic_cast<TH2F*>(data.getGenericHisto(PixScan::OCCUPANCY,i_scan,0));
      // for more complex scans, in which also loop level 1,2 need to be specified, use
      // PixDBData::getGenericHisto(PixLib::PixScan::HistogramType type, const int idx[4])
      cout << endl << "got data for scan point " << i_scan << endl << endl; 
      TH2F * myOcc = new TH2F(*(myData));
      cout << endl << "created TH2F" << endl << endl;
      f.cd();
      std::stringstream a;
      a << i_scan;
      myOcc->Write(("occupancy_"+a.str()).c_str());
    }
    f.Close();
}

int main(int argc, const char* argv[]){

  if(argc!=2){
    std::cerr << "No filename provided, can't proceed! " << std::endl;
    std::cerr << "Usage: LaserAna <RootDB file name>" << std::endl;
    return -1;
  }

  std::string fname = argv[1];
  fillData(fname);
  saveOcc(fname, ScanName, GroupName, ModuleName);

  return 0;
}

