/////////////////////////////////////////////////////////////////////
////															 ////
//// Example 1: Get a threshold map and plot it in 2D histogram  ////
//// Meant to provide examples on very low level how to get data ////
//// from RootDBD data files as used in USBpix.					 ////
//// 															 ////
//// Author: Malte	(with huge support from Jörn ;-), thanks! )  ////
////															 ////
/////////////////////////////////////////////////////////////////////


#include <DataContainer/PixDBData.h>
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"

#include <TH2F.h>
#include <TGraph.h> 
#include <TApplication.h>
#include <TStyle.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace PixLib;
using namespace std;

string ScanName, GroupName, ModuleName;

void listScans(string fname){	// this function explains how to loop over DBInquires and get the scan labels. Searching for correct one and plotting it.
string sname, gname, mname;
  RootDB *db = new RootDB(fname.c_str());
  DBInquire *root = db->readRootRecord(1);
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
	      cout << "     with module: " <<  mname << endl;
	    }
	  }
	}
	}
	}
	 
	if (sname == "THRESHOLD_SCAN"/*"fdac tune FdacVbn=30"*/)
	{
		ScanName = sname;
		GroupName = gname;
		ModuleName = mname;
		cout << "found scan: " << ScanName << ":" << GroupName << ":" << ModuleName << endl;
		break;
	}
	 else
	 {
		 cout << "wrong scan" << endl << endl;
	 }	
  }
  delete db;
}


void plotFDACd(string fname, string sname, string gname, string mname){ // how to get the data and plot it. Still to do: make histogram beatiful ;-)
    PixDBData data("scan", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
	TH2F * myData = data.GetMap(0, PixScan::SCURVE_MEAN, -1);
	cout << endl << "got data" << endl << endl;	
    TH2F * myPlot = new TH2F(*(myData)/**((TH2F*)data.GetMap(0, PixScan::SCURVE_MEAN,0))*/);
	cout << endl << "created TH2F" << endl << endl;

    TApplication app("app",NULL,NULL);
	myPlot->Draw("ABC");
    cout << "Press ctrl-C to stop" << endl;
    app.Run();
}

int main(int argc, char **argv){
  std::string fname = "test.root";
  if(argc==2) fname = argv[1];
  listScans(fname.c_str());
  plotFDACd(fname.c_str(), ScanName, GroupName, ModuleName);
  return 0;
}

