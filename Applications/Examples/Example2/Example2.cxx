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
TH1F * my1DHisto;

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
	 
	if (sname == "fdac tune FdacVbn=30")
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
	TH2F * myData = data.getDACMap(-1, "FDAC");
	cout << endl << "got 2D data" << endl << endl;	

	my1DHisto = new TH1F("FDAC","FDAC distribution", 16, -0.5, -0.5+(float)16);

	int value = 0;
	 for (Int_t col = 0; col < 80; col++)
	 {	
		 for (Int_t row = 0; row < 336; row++)
		 {
			 value = myData->GetBinContent(col, row);
			 my1DHisto->Fill(value);
		 }
	 }
	
	my1DHisto->GetXaxis()->SetTitle("FDAC [DAC]");

	my1DHisto->SetFillStyle(1);

	TAxis * xax = my1DHisto->GetXaxis();
	TAxis * yax = my1DHisto->GetYaxis();
	xax->SetTitle("FDAC [DAC]");
	yax->SetTitle("no. pixels");


	my1DHisto->UseCurrentStyle();
	my1DHisto->Draw();
}

int main(int argc, char **argv){
  std::string fname = "test.root";
  if(argc==2) fname = argv[1];

  TApplication* theApp = new TApplication("App", &argc, argv);
  
  TCanvas* c = new TCanvas("c", "c", 800, 600);
  c->cd();
  
  listScans(fname.c_str());
  plotFDACd(fname.c_str(), ScanName, GroupName, ModuleName);
  
  cout << "Press ctrl-C to stop" << endl;
  theApp->Run();
  
  return 0;
}
