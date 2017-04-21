////////////////////////////////////////////////////////////////////
//// GetParHistos: Get parameter histograms from TOT_CALIB_ALT	////
//// scan and plots them in a normal par.root file.				////
//// This new root file can be used for the TOT to Q conversion ////
//// needed to analyse the source scan output.					////
////															////
//// Compile with:	nmake Makefile.GetParHistos					////
//// Run with:		GetParHistos.exe YourInputFile.root			////
//// 															////
//// Author:	Julia Rieger (Uni Goettingen)					////	
////			(based on Example 1 from Malte)					////
////			Thanks to Jens and Joern!						////
////															////
////////////////////////////////////////////////////////////////////


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

// Looks at the structure of the RootDB file and checks if a TOT_CALIB_ALT scan is part of the entered file
bool fillData(string fname){	
  bool retval = false;
	string sname, gname, mname;
	
	RootDB * db = new RootDB(fname.c_str());
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
	
		if (sname == "TOT_CALIB_ALT"){
			ScanName = sname;
			GroupName = gname;
			ModuleName = mname;
			cout << "found scan: " << ScanName << ":" << GroupName << ":" << ModuleName << endl;
			retval = true;
			break;
		}
		else{
		 cout << "wrong scan" << endl << endl;
		}	
	}	
	delete db;
	return retval;
}

// Gets the TOTCAL_PARA histogram from the path found in fillData() and writes it to the par.root file
void plotParA(string fname, string sname, string gname, string mname){ 
    PixDBData data("scan", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
	TH2F * myData = data.GetMap(0, PixScan::TOTCAL_PARA,-1);
	cout << endl << "got data" << endl << endl;	
    TH2F * myPlotA = new TH2F(*(myData));
	cout << endl << "created TH2F" << endl << endl;

	TFile f("par.root", "RECREATE");

	myPlotA->Draw("colz");
	//cout << myPlotA -> GetBinContent(1,1) << endl; //(1,1) is pixel 0,0
    myPlotA->Write();
}

// Gets the TOTCAL_PARB histogram from the path found in fillData() and writes it to the par.root file
void plotParB(string fname, string sname, string gname, string mname){ 
    PixDBData data("scan", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
	TH2F * myData = data.GetMap(0, PixScan::TOTCAL_PARB,-1);
	cout << endl << "got data" << endl << endl;	
    TH2F * myPlotB = new TH2F(*(myData));
	cout << endl << "created TH2F" << endl << endl;

	TFile f("par.root", "UPDATE");

	myPlotB->Draw("colz");
    myPlotB->Write();
}

// Gets the TOTCAL_PARC histogram from the path found in fillData() and writes it to the par.root file
void plotParC(string fname, string sname, string gname, string mname){ 
    PixDBData data("scan", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
	TH2F * myData = data.GetMap(0, PixScan::TOTCAL_PARC,-1);
	cout << endl << "got data" << endl << endl;	
    TH2F * myPlotC = new TH2F(*(myData));
	cout << endl << "created TH2F" << endl << endl;

	TFile f("par.root", "UPDATE");

	myPlotC->Draw("colz");
    myPlotC->Write();
}

int main(int argc, const char* argv[]){
	
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  TApplication app("app",NULL,NULL);
  
  
  if(argc==1){cout << "No Data Name Given!" << endl; return -1;}

  if(fillData(argv[1])){
    plotParA(argv[1], ScanName, GroupName, ModuleName);
    plotParB(argv[1], ScanName, GroupName, ModuleName);
    plotParC(argv[1], ScanName, GroupName, ModuleName);
    
    cout << "Press ctrl-C to stop" << endl;
    app.Run();
  }	
  
  return 0;
}

