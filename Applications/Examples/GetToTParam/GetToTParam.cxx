/*
 this code takes produces a txt file that can be read into tbmon for the ToT-to-charge conversion.
 It runs over an output root file generated by STControl. The root file must contain the result of a TOT_CALIB and a THRESHOLD_SCAN.
 The code calculates the mean and the sigma of the VCAL distribution for every ToT value (summing over all pixels in the chip).
 It produces a text file that lists the ToT value, the VCAL mean and the VCAL sigma.
 It also writes the threshold value to the end of this text file.
 In addition a root file containing the fitted histogram and the extrapolation to higher ToT values is produced. These are control plots.
 Complile with: "nmake Makefile.GetToTParam"
 Run: "GetToTParam.exe input_filename.root"
 
 */

#include <DataContainer/PixDBData.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/RootDB.h>
#include <PixController/PixScan.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>

#include "GeneralDBfunctions.h"
#include <TH1D.h>
#include <TH1F.h>
#include <TF1.h>
#include <TH2F.h>
#include <TGraph.h> 
#include <TApplication.h>
#include <TStyle.h>
#include <Fitting/FitClass.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


#include <map>
#include <sstream>
#include <stdlib.h>


using namespace PixLib;
using namespace std;

string ScanName, GroupName, ModuleName, ThreshScanName;

bool getToTCalib(string fname){
  cout << "In getTOTCalib " << endl;
  string sname, gname, mname;
  RootDB *db = new RootDB(fname.c_str());
  DBInquire *root = db->readRootRecord(1);
  cout << "In getTOTCalib: after DBInquire " << endl;
	bool found_totscan = false;
	bool found_thrscan = false; 

  for(recordIterator i = root->recordBegin(); i!=root->recordEnd();i++){
	  cout << "In getTOTCalib: first for loop " << endl;
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

	

	 	if (sname == "TOT_CALIB" && !found_totscan )
	{
		ScanName = sname;
		GroupName = gname;
		ModuleName = mname;
		cout << "found scan 1: " << ScanName << ":" << GroupName << ":" << ModuleName << endl;
		found_totscan = true;
		if (found_thrscan) 
			break;

	}
		if (sname == "THRESHOLD_SCAN" && !found_thrscan)
	{
		ThreshScanName = sname;
		GroupName = gname;
		ModuleName = mname;
		cout << "found scan 2: " << ThreshScanName << ":" << GroupName << ":" << ModuleName << endl;
		found_thrscan = true;
		if (found_totscan)
			break;

		}
		
	
  }
	if (!found_totscan)
	 {
		 cout << "DID NOT FIND TOT CALIB IN THIS FILE"  << endl;
		 delete db;
		 return false;
	 }	
	 if (!found_thrscan)
	 {
		 cout << "DID NOT FIND THRESHOLD SCAN IN THIS FILE"  << endl;
		 delete db;
		 return false;
	 }

  delete db;
  return true;
}



void plotToTHistos(string fname, string sname1, string sname2, string gname, string mname){ 

	//"get threshold"-----------------------------------------------------------------
   	PixDBData threshdata("scan2", (fname+":/"+sname2+"/"+gname).c_str(), mname.c_str());

	/*double thrend = threshdata.getScanStop(0);
	cout << "THR: end " << thrend << endl;*/

	int ToTVCAL[600][15]; //if more scan points than 500, increase size of this array;
	TH1D * ScanHisto;
	TH2F* ThreshHistoDB = threshdata.GetMap(0, PixScan::SCURVE_MEAN, 0);
	//cout << endl << "got data" << endl << endl;	
	TH2F* ThreshHisto =  new TH2F(*(ThreshHistoDB));
	//cout << endl << "created TH2F" << endl << endl;
    TF1 *threshgaus = new TF1("threshgaussian","gaus",0, 10000);
	TH1D* h1 = new TH1D("h1","h1",1000,0,10000);

	/*TApplication app("app",NULL,NULL);
	ThreshHisto->Draw("ABC");
    cout << "Press ctrl-C to stop" << endl;
    app.Run();*/ 
    for (int i=1; i<81;i++){
         for (int j=1; j<337;j++){
            double thr = ThreshHisto->GetBinContent(i,j);
            if (thr>0){
                h1->Fill(thr);
            }
        }
    }
	h1->Fit(threshgaus, "","", h1->GetMean() - h1->GetRMS(), h1->GetMean() + h1->GetRMS());
	

	double thresh = threshgaus->GetParameter(1);
	

	///end "get threshold"------------------------------------------------------------------

	//"get totcalib"------------------------------------------------------------------------
	PixDBData totdata("scan1", (fname+":/"+sname1+"/"+gname).c_str(), mname.c_str());
	Config &mcfg = totdata.getModConfig().subConfig("PixFe_0/PixFe");
	int steps = totdata.getScanSteps(0);
	double start = totdata.getScanStart(0);
	double end = totdata.getScanStop(0);
	double stepsize = (end-start)/(steps-1);
	double vcalgrad0 = (((ConfFloat&)mcfg["Misc"]["VcalGradient0"]).value());
	double vcalgrad1 = (((ConfFloat&)mcfg["Misc"]["VcalGradient1"]).value());
	double cap11 = (((ConfFloat&)mcfg["Misc"]["CInjHi"]).value());
    


	TH1D *ToTHistos[15];
	TF1 *gaus = new TF1("gaussian","gaus",0, 1000);
	TF1 *lin1 = new TF1("linear","pol1",0,16);
	TF1 *lin2 = new TF1("linear","pol1",0,16);
	int lowfitbin, highfitbin;
	//first param is mean, second is sigma
	double GausParam[15][2];
	//reads off ToT Histos for each VCAL
	for(int i = 0; i<steps; i++) {
		ScanHisto = (TH1D*)totdata.getGenericHisto(PixScan::TOT,i,0);
		for( int j = 1; j < 16; j++){
			ToTVCAL[i][j-1] = ScanHisto->GetBinContent(j);
			}
		}
	//Inputs this data into 15 ToT Histograms
	double lastval;
	int peakbin;
	double peakvalue;
	int flag = 0;
	for( int j = 0; j<15; j++){
		char* name1 = new char[100];
		sprintf(name1, "tothisto_%d", j);
		ToTHistos[j] = new TH1D(name1,"VCAL for ToT;VCAL;Entries",201,0,1000);
		for(int i=0;i<steps;i++){
			ToTHistos[j]->Fill(i*stepsize+start,ToTVCAL[i][j]);
			//find out when chip is saturated
		}
		peakbin = ToTHistos[j]->GetMaximumBin();
		peakvalue = ToTHistos[j]->GetBinContent(peakbin);
			if(ToTHistos[j]->FindLastBinAbove(peakvalue/20,1)== steps && flag == 0) {lastval= j; flag = 1;}
	}
	//Fits the first dozen or so ToT Histos with Gaussians
	lastval = lastval - 1;
	for( int j = 0; j < lastval + 1; j++){
		
		lowfitbin = ToTHistos[j]->GetMean()-3*ToTHistos[j]->GetRMS();
		highfitbin = ToTHistos[j]->GetMean()+3*ToTHistos[j]->GetRMS();
		ToTHistos[j]->Fit(gaus,"","",lowfitbin,highfitbin);
		
		double vcalmean = gaus->GetParameter(1);
		double vcalsigma = gaus->GetParameter(2);
		double chargemean = 1.1*10*cap11/1.602*(vcalgrad0+vcalgrad1*vcalmean);
		double chargesigma = 1.1*10*cap11*vcalgrad1*vcalsigma/1.602;
		GausParam[j][0] = chargemean;
		GausParam[j][1] = chargesigma;
		}
	//Extrapolates to remaining Gaussians
	TH1D *Gaussian1 = new TH1D("mean","Mean for Gaussian;ToT code;Mean",15,0,15);
	TH1D *Gaussian2 = new TH1D("sigma","Sigma for Gaussian;ToT code;Sigma",15,0,15);	
		TString filename = fname;
	for( int j = 0; j < lastval + 1; j++){
		Gaussian1->SetBinContent(j,GausParam[j][0]);
		Gaussian2->SetBinContent(j,GausParam[j][1]);
	}
	Gaussian1->Fit(lin1,"","",3,lastval);
	Gaussian2->Fit(lin2,"","",3,lastval);
	for( int j = lastval + 1; j < 14; j++){
		GausParam[j][0] = lin1->Eval(j,0,0);
		GausParam[j][1] = lin2->Eval(j,0,0);
	}
	GausParam[14][0] = 0;
	GausParam[14][1] = 0;
	filename.ReplaceAll(".root","_ToT_calib.txt");
  ofstream parameters;
 parameters.open(filename);
  for( int k = 0; k < 15; k++ ) { 
	  parameters << k+1 << " " << GausParam[k][0] << " " << GausParam[k][1] << endl;
}

  parameters << thresh;
	parameters.close();
	filename.ReplaceAll("_ToT_calib.txt", "_histos.root");
	//cout<<filename;
	TFile *outputfile = new TFile(filename, "RECREATE");
	for(int a = 0; a < 14; a ++) {ToTHistos[a]->Write();}

	Gaussian1->Write();
	Gaussian2->Write();
	//ThreshHisto->Write();
	outputfile->Close();   
   
}

int main(int argc, const char* argv[]){
  if(argc==1){printf("No Config Name Given!"); return -1;}
  printf("config name %s",argv[1]);
  if(getToTCalib(argv[1]))
    plotToTHistos(argv[1], ScanName, ThreshScanName, GroupName, ModuleName);
  return 0;
}
