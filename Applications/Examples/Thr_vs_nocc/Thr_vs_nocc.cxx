#include <DataContainer/PixDBData.h>
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"

#include <TH2F.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TLegend.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace PixLib;
using namespace std;

//TH1F * my1DHisto;
double Xval[14];
double Yval[14];
double Xerror[14];
double Yerror[14];
double MaskedPixels[14];

void fillData(string fname, string thr0, string thr1, string thr2, string thr3, string thr4, string thr5, string thr6, string thr7, string thr8, string thr9, string thr10, string thr11, string thr12, string thr13){
	cout << endl << endl << endl << "------------------- start file " << fname << "-------------------------------" << endl;
	string sname, gname, mname;
	int ThrIteration = 0;
	int NoccIteration = 0;

	for (int i = 0; i < 14; i++)
	{
		Xval[i] = 0;
		Yval[i] = 0;
		Xerror[i] = 0;
		Yerror[i] = 0;
		MaskedPixels[i] = 0;
	}


	RootDB * db = new RootDB(fname.c_str());
	DBInquire *root = db->readRootRecord(1);
	for(recordIterator i = root->recordBegin(); i!=root->recordEnd();i++)
	{
		if((*i)->getName()=="PixScanResult"){
			sname = (*i)->getDecName();
			getDecNameCore(sname);
			cout << "NEW SCAN: \"" << sname << "\"" << endl;
			for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++)
			{
				if((*ii)->getName()=="PixModuleGroup"){
					gname = (*ii)->getDecName();
					getDecNameCore(gname);
					//cout << "  with module group: " <<  gname << endl;
					for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();iii++){
						if((*iii)->getName()=="PixModule"){
							mname = (*iii)->getDecName();
							getDecNameCore(mname);
							//cout << "     with module: " <<  mname << endl;
						}
					}
				}
			}
		}

		string src = sname;
		int strpos = src.find(" ");
		string target = src.substr(0, strpos);
		if (target == "threshold_scan")
		{
			string thr = src.substr(strpos+1);

			int strpos2 = thr2.find(" ");
			string thr2corr = thr2.substr(0, strpos2);

			if(thr == thr0)
				ThrIteration = 0;
			else if (thr == thr1)
				ThrIteration = 1;
			else if (thr == thr2corr)
				ThrIteration = 2;
			else if (thr == thr3)
				ThrIteration = 3;
			else if (thr == thr4)
				ThrIteration = 4;
			else if (thr == thr5)
				ThrIteration = 5;
			else if (thr == thr6)
				ThrIteration = 6;
			else if (thr == thr7)
				ThrIteration = 7;
			else if (thr == thr8)
				ThrIteration = 8;
			else if (thr == thr9)
				ThrIteration = 9;
			else if (thr == thr10)
				ThrIteration = 10;
			else if (thr == thr11)
				ThrIteration = 11;
			else if (thr == thr12)
				ThrIteration = 12;
			else if (thr == thr13)
				ThrIteration = 13;
			else
				continue;
			
			
			// get threshold data
			PixDBData data2("thresh", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
			TH2F * myThresholdMap = data2.GetMap(0, PixScan::SCURVE_MEAN);	
			TH1F * myThresholds = new TH1F("myThresholds","Threshold", 3401, -0.5, -0.5+(float)3401);
			double thrval = 0;
			for (Int_t col = 1; col < 78; col++)
			{	
				for (Int_t row = 1; row < 337; row++)
				{
					thrval = myThresholdMap->GetBinContent(col, row);
					myThresholds->Fill(thrval);
				}
			}
			cout << "filled histo" << endl;

			double thrmean = myThresholds->GetMean();
			double mint = thrmean - 500.0; 
			double maxt = thrmean + 500.0;
			TF1 * gfitt = new TF1("Gaussian","gaus",mint,maxt); // Create the fit function
			myThresholds->Fit("Gaussian","RN"); // Fit histogram h"
			thrmean = gfitt->GetParameter(1); //myThresholds->GetMean();
			double thrsigma = gfitt->GetParameter(2);//myThresholds->GetRMS();
			cout << "fitted threshold: " << thrmean << "\t" << endl;

			Xval[ThrIteration] = thrmean;
			Xerror[ThrIteration] = thrsigma;
			delete myThresholds;
			cout << "processed " << ThrIteration << " amount of threshold scans" << endl;
		}


		if ((target == "noise_occ") || (target == "NOISE_OCC"))
		{
			string thr = src.substr(strpos+1);
			if(thr == thr0)
				NoccIteration = 0;
			else if (thr == thr1)
				NoccIteration = 1;
			else if (thr == thr2)
				NoccIteration = 2;
			else if (thr == thr3)
				NoccIteration = 3;
			else if (thr == thr4)
				NoccIteration = 4;
			else if (thr == thr5)
				NoccIteration = 5;
			else if (thr == thr6)
				NoccIteration = 6;
			else if (thr == thr7)
				NoccIteration = 7;
			else if (thr == thr8)
				NoccIteration = 8;
			else if (thr == thr9)
				NoccIteration = 9;
			else if (thr == thr10)
				NoccIteration = 10;
			else if (thr == thr11)
				NoccIteration = 11;
			else if (thr == thr12)
				NoccIteration = 12;
			else if (thr == thr13)
				NoccIteration = 13;
			else
				continue;

			int triggercount = 0;
			{
				// get noise data
				PixDBData data("nocc", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());


				TH2F * myTempHitMap = data.GetMap(0, PixScan::OCCUPANCY);
				if (myTempHitMap==0)
				{
					cout << "HitMapPointer = NULL, jumping out..." << endl;
					continue;
				}
				TH2F * myHitMap = new TH2F(*myTempHitMap);

				TH2F * myTempEnableMap = data.getMaskMap(0, "ENABLE");
				if (myTempEnableMap==0)
				{
					cout << "EnableMapPointer = NULL, jumping out..." << endl;
					continue;
				}
				TH2F * myEnableMap = new TH2F(*myTempEnableMap);

				TH1F * myNOcc = new TH1F("myNOcc","NOcc", 300000000, -0.5, -0.5+(float)300000000);
				

				// get number of triggers
				for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++)
				{
					if((*ii)->getName()=="PixModuleGroup")
					{
						for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();iii++)
						{
							if((*iii)->getName()=="PixScanData")
							{
								for(fieldIterator iiii = (*iii)->fieldBegin(); iiii!=(*iii)->fieldEnd();iiii++)
								{
									if((*iiii)->getName()=="general_repetitions")
									{
										db->DBProcess(*iiii, PixLib::READ, triggercount); 
										//getDecNameCore(mname);
										cout << endl << "found #repetitions: " <<  triggercount << endl;
									}
								}
							}
						}
					}
				}

				// get data and fill NOcc histogram
				double hitval = 0;
				double noccval = 0;
				//double enableval = 0;
				for (Int_t col = 1; col < 78; col++)
				{	
					for (Int_t row = 1; row < 337; row++)
					{
						hitval = myHitMap->GetBinContent(col, row);
						if ((myEnableMap->GetBinContent(col, row)) && (triggercount!=0))
						{
							if (hitval == 0)
								hitval = 1;
							noccval = hitval / triggercount;
							if (noccval >= 0.00001)
							{
								MaskedPixels[NoccIteration]++;
								//cout << "PIXEL IGNORED due to NOcc > 10e-5!!!" << endl;
								continue;
							}
							myNOcc->Fill(noccval);
						}
						else
						{
							MaskedPixels[NoccIteration]++;
							//cout << "pixel col " << col << " row " << row << " was masked offline..." << endl;
						}
					}
				}
				//cout << "DEBUG: filled NOcc histogram..." << endl;

				MaskedPixels[NoccIteration] = MaskedPixels[NoccIteration] / 25872;	// 26880 - three columns...   26880;
				double noccmean = myNOcc->GetMean();
				double noccrms = 0;
				Yval[NoccIteration] = noccmean;
				Yerror[NoccIteration] = noccrms;
				delete myNOcc;
				cout << "NOcc mean: " << noccmean << endl;
				cout << "This is NOcc scan  number: " << NoccIteration << endl;
			}
		}
	}
	delete db;
}

int main(int argc, char **argv){
	TApplication* theApp = new TApplication("App", &argc, argv);

	TStyle * myStyle = new TStyle("myStyle","My Style");
	//myStyle->SetTitleColor(kWhite);
	myStyle->SetPadColor(kWhite);
	myStyle->SetFrameFillColor(kWhite);
	myStyle->SetFrameBorderMode(0);
	//myStyle->SetLegendFillColor(kWhite);
	myStyle->SetTitleFillColor(kWhite);
	myStyle->SetCanvasColor(kWhite);
	myStyle->cd();

	TCanvas * myCanvas = new TCanvas("nocc", "NOcc", 800, 900);
	myCanvas->UseCurrentStyle();
	myCanvas->Divide(1,2);
	TPad *nocc_1 = 0;
	nocc_1 = (TPad*) myCanvas->GetListOfPrimitives()->FindObject("nocc_1");
	if (nocc_1 != 0)
	{
		nocc_1->SetTopMargin(0.125);
		nocc_1->SetLogy();
	}
	else 
		cout << "did not find nocc_1" << endl << endl;
	
	TPad *nocc_2 = 0;
	nocc_2 = (TPad*) myCanvas->GetListOfPrimitives()->FindObject("nocc_2");
	if (nocc_2 != 0)
		nocc_2->SetTopMargin(0.125);
	else 
		cout << "did not find nocc_2" << endl << endl;
	myCanvas->cd(1);

	// draw a frame to define the range
	myCanvas->SetBorderMode(0);
	TH1F * myFrame = myCanvas->DrawFrame(200,0.000000001,3500,0.000001);
	myFrame->SetXTitle("Threshold [e]");
	myFrame->SetYTitle("NOcc per bx per pixel");
	myFrame->SetTitle("NOcc in low threshold operation");
	

	myCanvas->cd(2);

	// create second frame for # of masked pixels...
	myCanvas->SetBorderMode(0);
	TH1F * myFramePixels = myCanvas->DrawFrame(200,0.0,3500,0.05);
	myFramePixels->SetXTitle("Threshold [e]");
	myFramePixels->SetYTitle("fraction of pixels masked");
	myFramePixels->SetTitle("Masked pixels");


	myCanvas->cd(1);
	// genererate first graph
	fillData("../thr_noise_char_irr_3d.root", "3200", "2500", "2000 2", "1800", "1600", "1400", "1200", "none", "none", "none", "none", "none", "none", "none");
	TGraphErrors * myGraph = new TGraphErrors(7, Xval, Yval, Xerror, Yerror);
	myGraph->SetName("irr3D");
	myGraph->SetMarkerStyle(20);
	myGraph->SetMarkerSize(0.7);
	myGraph->SetMarkerColor(kGreen);
	myGraph->SetLineColor(kGreen);
	myGraph->Draw("P");

	myCanvas->cd(2);
	TGraph * myGraphPixels = new TGraphErrors(7, Xval, MaskedPixels, Xerror);
	myGraphPixels->SetName("irr3D");
	myGraphPixels->SetMarkerStyle(20);
	myGraphPixels->SetMarkerSize(0.7);
	myGraphPixels->SetMarkerColor(kGreen);
	myGraphPixels->SetLineColor(kGreen);
	myGraphPixels->Draw("P");

	myCanvas->cd(1);
	// genererate second graph
	fillData("../thr_noise_char_irr_pps.root", "3200", "2500", "2000", "1800", "1600", "1400", "1200", "none", "none", "none", "none", "none", "none", "none");
	TGraphErrors * myGraph2 = new TGraphErrors(7, Xval, Yval, Xerror, Yerror);
	myGraph2->SetName("irrPPS");
	myGraph2->SetMarkerStyle(21);
	myGraph2->SetMarkerSize(0.7);
	myGraph2->SetMarkerColor(kBlue);
	myGraph2->SetLineColor(kBlue);
	myGraph2->Draw("P");

	myCanvas->cd(2);
	TGraph * myGraphPixels2 = new TGraphErrors(7, Xval, MaskedPixels, Xerror);
	myGraphPixels2->SetName("irrPPS");
	myGraphPixels2->SetMarkerStyle(21);
	myGraphPixels2->SetMarkerSize(0.7);
	myGraphPixels2->SetMarkerColor(kBlue);
	myGraphPixels2->SetLineColor(kBlue);
	myGraphPixels2->Draw("P");

	myCanvas->cd(1);
	// genererate third graph
	fillData("../thr_noise_char_bareIC_19082011.root", "3200", "2500", "2000", "1800", "1600", "1400", "1200", "1100", "1000", "900", "800", "700", "600", "500");
	TGraphErrors * myGraph3 = new TGraphErrors(14, Xval, Yval, Xerror, Yerror);
	myGraph3->SetName("bareIC");
	myGraph3->SetMarkerStyle(22);
	myGraph3->SetMarkerSize(0.7);
	myGraph3->SetMarkerColor(kRed);
	myGraph3->SetLineColor(kRed);
	myGraph3->Draw("P");

	myCanvas->cd(2);
	TGraph * myGraphPixels3 = new TGraphErrors(14, Xval, MaskedPixels, Xerror);
	myGraphPixels3->SetName("bareIC");
	myGraphPixels3->SetMarkerStyle(22);
	myGraphPixels3->SetMarkerSize(0.7);
	myGraphPixels3->SetMarkerColor(kRed);
	myGraphPixels3->SetLineColor(kRed);
	myGraphPixels3->Draw("P");

/*	myCanvas->cd(1);
	// genererate fourth graph
	fillData("../thr_noise_char_scc051.root", "3200", "2500", "2000", "1800", "1600", "1400", "1200", "1100", "1000", "900", "800", "700", "none", "none);
	TGraphErrors * myGraph4 = new TGraphErrors(12, Xval, Yval, Xerror, Yerror);
	myGraph4->SetName("unirrPPS");
	myGraph4->SetMarkerStyle(23);
	myGraph4->SetMarkerSize(0.7);
	myGraph4->SetMarkerColor(kBlack);
	myGraph4->SetLineColor(kBlack);
	myGraph4->Draw("P");

	myCanvas->cd(2);
	TGraph * myGraphPixels4 = new TGraphErrors(12, Xval, MaskedPixels, Xerror);
	myGraphPixels4->SetName("unirrPPS");
	myGraphPixels4->SetMarkerStyle(23);
	myGraphPixels4->SetMarkerSize(0.7);
	myGraphPixels4->SetMarkerColor(kBlack);
	myGraphPixels4->SetLineColor(kBlack);
	myGraphPixels4->Draw("P");
*/

	myCanvas->cd(1);
	// build legend
	TLegend * myLegend = new TLegend(0.6,0.7,0.85,0.9);
	myLegend->AddEntry("irr3D","3D irr. 5e-15","p");
	myLegend->AddEntry("irrPPS","PPS irr. 5e-15","p");
	myLegend->AddEntry("bareIC","bare FE-I4A","p");
	//myLegend->AddEntry("unirrPPS","PPS unirr.","p");
	//myLegend->AddEntry("IBLdevice","IBL device, SCC 051","p");
	myLegend->SetHeader("Used Devices");
	myLegend->SetFillColor(kWhite);
	myLegend->Draw();

	myCanvas->cd(2);
	// build legend
	TLegend * myLegendPixels = new TLegend(0.6,0.7,0.85,0.9);
	myLegendPixels->AddEntry("irr3D","3D irr. 5e-15","p");
	myLegendPixels->AddEntry("irrPPS","PPS irr. 5e-15","p");
	myLegendPixels->AddEntry("bareIC","bare FE-I4A","p");
	//myLegendPixels->AddEntry("unirrPPS","PPS unirr.","p");
	//myLegend->AddEntry("IBLdevice","IBL device, SCC 051","p");
	myLegendPixels->SetHeader("Used Devices");
	myLegendPixels->SetFillColor(kWhite);
	myLegendPixels->Draw();

	myCanvas->cd(1);
	myCanvas->SetBorderMode(0);
	myCanvas->cd(2);
	myCanvas->SetBorderMode(0);
	cout << endl << endl << "Press ctrl-C to stop" << endl << endl;
	theApp->Run();

	return 0;
}




