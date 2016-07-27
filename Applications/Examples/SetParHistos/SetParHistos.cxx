////////////////////////////////////////////////////////////////////
//// SetParHistos: fill parameter histograms from TOT_CALIB	////
//// and store result in root file from which TOTx histos are.	////
////		      						////
//// Compile with:	qmake; nmake                            ////
//// Run with:		GetParHistos YourInputFile.root		////
//// 								////
//// Author:	Joern Grosse-Knetter (Uni Goettingen)		////	
////		(based on Julia's code in PixModuleGroup)	////
////								////
////////////////////////////////////////////////////////////////////


#include <DataContainer/PixDBData.h>
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"
#include <Histo/Histo.h>
#include <PixModule/PixModule.h>
#include <PixFe/PixFe.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <Fitting/FitClass.h>  

#include <TF1.h>
#include <TH1F.h>
#include <TGraph.h> 
#include <TGraphErrors.h> 
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

// Looks at the structure of the RootDB file and checks if a TOT_CALIB_ALT scan is part of the entered file
bool fillData(string fname, string slabel){	
  bool retval = false;
  string sname, gname, mname;
  
  RootDB * db = new RootDB(fname.c_str());
  DBInquire *root = db->readRootRecord(1);
  
  ScanName = "";
  GroupName = "";
  ModuleName = "";

  for(recordIterator i = root->recordBegin(); i!=root->recordEnd();i++){
    if((*i)->getName()=="PixScanResult"){
      sname = (*i)->getDecName();
      getDecNameCore(sname);
      if (slabel=="") cout << "NEW SCAN: \"" << sname << "\"" << endl;
      for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
	if((*ii)->getName()=="PixModuleGroup"){
	  gname = (*ii)->getDecName();
	  getDecNameCore(gname);
	  if (slabel=="") cout << "  with module group: " <<  gname << endl;
	  for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();iii++){
	    if((*iii)->getName()=="PixModule"){
	      mname = (*iii)->getDecName();
	      getDecNameCore(mname);
	      if (slabel=="") cout << "     with module: " <<  mname << endl;
	    }
	  }
	}
      }
    }
    
    if (slabel != "" && sname == slabel){
      ScanName = sname;
      GroupName = gname;
      ModuleName = mname;
      cout << "found scan: " << ScanName << ":" << GroupName << ":" << ModuleName << endl;
      retval = true;
      break;
    }	
  }	
  delete db;
  return retval;
}

void procCalib(string fname, string sname, string gname, string mname, unsigned int debcol, unsigned int debrow, bool plotOnly){ 

  TCanvas *can1 = new TCanvas("can1", "ToT calib step 1", 800, 800);
  can1->Clear();
  can1->Divide(5, 3);
  TCanvas *can2 = new TCanvas("can2", "ToT calib step 2", 800, 800);
  can2->Clear();
  can2->Divide(1, 2);

  FitClass fc;
  int funcID=fc.getFuncID("ToT-calibration FE-I4 Polynomial 2nd order");

  PixDBData data("scan", (fname+":/"+sname+"/"+gname).c_str(), mname.c_str());
  int mod = ((ConfInt&)data.getModConfig()["general"]["ModuleId"]).valueInt();
  std::string mccflv = ((ConfList&)data.getModConfig()["general"]["MCC_Flavour"]).sValue();
  std::string feflv = ((ConfList&)data.getModConfig()["general"]["FE_Flavour"]).sValue();
  int nfe = 0;
  for(int i=0; i<data.getModConfig().subConfigSize(); i++)
    if(data.getModConfig().subConfig(i).name().find("PixFe")!=std::string::npos) nfe++;
  PixModule pm(0, mod, "tmp", feflv, nfe, 1, mccflv);
  pm.config() = data.getModConfig();

  const int NoOfTOTHistos=13;
  const int steps = data.getScanSteps(0);
  std::vector<float>& scanpts = data.getScanPoints(0);

  double dtot[NoOfTOTHistos];
  double aQavg[NoOfTOTHistos], aQsigma[NoOfTOTHistos];
  
  // VCAL histos for debugging
  TH1F *VCALHisto[NoOfTOTHistos];
  // retrieve raw ToTx histos
  Histo *TOTHisto[NoOfTOTHistos][steps];
  for(int tot=0; tot<NoOfTOTHistos; tot++){
    gROOT->cd(); // make sure histos are created in memory, not in file
    dtot[tot] = tot;
    stringstream a,b;
    a << "vcaltot" << tot;
    b << "VCAL for ToT=" << tot;
    VCALHisto[tot] = new TH1F(a.str().c_str(), b.str().c_str(), 32, -0.5, 1023.5);
    for(int j=0; j<steps; j++){
      TOTHisto[tot][j] = data.getGenericPixLibHisto((PixLib::PixScan::HistogramType)(PixScan::TOT0+tot), j, 0);
    }
  }
  gROOT->cd(); // make sure histos are created in memory, not in file
  TH1F *chi2hi = new TH1F("chi2hi", "#chi^{2} distribution", 100, -2, 2000);

  // create fit parameter histos
  Histo *parA, *parB, *parC, *chi2;
  unsigned int nColMod = pm.nColsMod();
  unsigned int nRowMod = pm.nRowsMod();
  std::string nam, tit;
  std::ostringstream mnum;
  mnum << mod;
  nam = "ParA_0" + mnum.str();
  tit = "Parameter A mod " + mnum.str();
  parA = new Histo(nam, tit, nColMod, -0.5, nColMod-0.5, nRowMod, -0.5, nRowMod-0.5);
  nam = "ParB_" + mnum.str();
  tit = "Parameter B mod " + mnum.str();
  parB = new Histo(nam, tit, nColMod, -0.5, nColMod-0.5, nRowMod, -0.5, nRowMod-0.5);
  nam = "ParC_" + mnum.str();
  tit = "Parameter C mod " + mnum.str();
  parC = new Histo(nam, tit, nColMod, -0.5, nColMod-0.5, nRowMod, -0.5, nRowMod-0.5);
  nam = "Chi2_" + mnum.str();
  tit = "Chi2 mod " + mnum.str();
  chi2 = new Histo(nam, tit, nColMod, -0.5, nColMod-0.5, nRowMod, -0.5, nRowMod-0.5);


  for (std::vector<PixFe*>::iterator fe = pm.feBegin(); fe != pm.feEnd(); fe++){
    int ife = (*fe)->number();
    // get FE calib.
    float vcalG0 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"])).value();
    float vcalG1 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient1"])).value();
    float vcalG2 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient2"])).value();
    float vcalG3 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient3"])).value();
    std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
    int chargeInjCap = 2;//scn->getChargeInjCap();
    float cInj     = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[chargeInjCap]])).value();
    cInj /= 0.160218f;

    for (unsigned int col=0; col<(*fe)->nCol(); col++) {
      //cout << "Col: " << col << endl;
      for (unsigned int row=0; row<(*fe)->nRow(); row++) {
	unsigned int colmod, rowmod;
	rowmod = pm.iRowMod(ife, row);
	colmod = pm.iColMod(ife, col);
	unsigned int ngood=0;
	double vcal = 500.5;
	double dchge = cInj*(vcalG0+vcalG1*vcal+vcalG2*vcal*vcal+vcalG3*vcal*vcal*vcal);
	vcal = 500.;
	dchge -= cInj*(vcalG0+vcalG1*vcal+vcalG2*vcal*vcal+vcalG3*vcal*vcal*vcal);
	for(int tot=0; tot<NoOfTOTHistos; tot++){
	  double chg_sumsqr = 0.;
	  double chg_sum = 0.;
	  double hitsum = 0.;
	  for(int j=0; j<steps; j++){
	    vcal = (double)scanpts[j];
	    double charge = cInj*(vcalG0+vcalG1*vcal+vcalG2*vcal*vcal+vcalG3*vcal*vcal*vcal);
	    chg_sumsqr += (*TOTHisto[tot][j])(colmod, rowmod) * charge * charge;
	    chg_sum += (*TOTHisto[tot][j])(colmod, rowmod) * charge;
	    hitsum += (*TOTHisto[tot][j])(colmod, rowmod);
	    if(colmod == debcol && rowmod == debrow)
	      VCALHisto[tot]->Fill(vcal, (*TOTHisto[tot][j])(colmod, rowmod));
	  }
	  if(hitsum > 2) {
	    chg_sumsqr /= hitsum;
	    aQavg[tot] = chg_sum / hitsum;
	    aQsigma[tot] = (chg_sumsqr - aQavg[tot]*aQavg[tot])/(hitsum-1.0);
	    if(aQsigma[tot]>0.) aQsigma[tot] = sqrt(aQsigma[tot]);
	    else                aQsigma[tot] = dchge;
	    if(aQsigma[tot]<dchge) aQsigma[tot] = dchge; // should not have errors smaller than half a VCAL step
	    ngood++;
	  }else{
	    aQavg[tot]=0.;
	    aQsigma[tot]=0.;
	  }
	  if(colmod == debcol && rowmod == debrow)
	    cout << "JR: pixel " << colmod << "," << rowmod << " Qavg = "<< aQavg[tot] << 
	      " Qsigma "<< aQsigma[tot] << " hitsum " << hitsum << endl;

						}
	// graph of charge vs ToT
	if(colmod == debcol && rowmod == debrow){
	  TGraphErrors *g = new TGraphErrors();//NoOfTOTHistos, dtot, aQavg, dtot_err, aQsigma);
	  int npt=0;
	  for(int i=0;i<NoOfTOTHistos;i++){
	    if(aQavg[i]>0){
	      g->SetPoint(npt, dtot[i], aQavg[i]);
	      g->SetPointError(npt, 0., aQsigma[i]);
	      npt++;
	    }
	  }
	  g->SetMarkerStyle(20);
	  for(int tot=0; tot<NoOfTOTHistos; tot++){
	    can1->cd(tot+1);
	    VCALHisto[tot]->Draw();
	    VCALHisto[tot]->GetXaxis()->SetTitle("PlsrDAC [DAC-units]");
	    VCALHisto[tot]->GetYaxis()->SetTitle("Events");
	    can1->Update();
	  }
	  can2->cd(1);
	  stringstream c;
	  c<< "example graph for col "<<colmod<<", row "<< rowmod;
	  g->SetTitle(c.str().c_str());
	  g->Draw("AP");
	  g->GetYaxis()->SetTitle("avg. injected charge [e]");
	  g->GetXaxis()->SetTitle("ToT [BCU]");
	  can2->Update();
	}
	if(ngood>4){ // it's unreasonable to fit 3-par. function with fewer than 4 valid data points
	  bool fix_par[3]={false,false,false};//,true,true,true,true};
	  // set all fit pars start values to 0 to have them guessed except for VCAL-Q conv. fact. (no. 4)
	  double par[3] = {0., 0., 0.};//, 0., 1., 0., 0.};
	  double dtot_err[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	  double chisqu = fc.runFit(NoOfTOTHistos, dtot, aQavg, dtot_err, aQsigma, par, fix_par,funcID, 0., 0.);
	  if(colmod == debcol && rowmod == debrow){
	    gROOT->cd();
	    can2->cd(1);
	    TF1 *fun = new TF1("parab", "[0]+[1]*x+[2]*x*x", 0, 15);
	    for(int i=0;i<3; i++)
	      fun->SetParameter(i, par[i]);
	    fun->SetLineColor((chisqu<1e-2 || chisqu>1e4)?2:1);
	    fun->Draw("same");
	    can2->Update();
	  }
	  if(chisqu<1e-2 || chisqu>1e4){
	    parA->set(colmod,rowmod,0.);
	    parB->set(colmod,rowmod,0.);
	    parC->set(colmod,rowmod,0.);
	  } else{
	    parA->set(colmod,rowmod,par[0]);
	    parB->set(colmod,rowmod,par[1]);
	    parC->set(colmod,rowmod,par[2]);
	  }
	  chi2->set(colmod,rowmod,chisqu);
	  chi2hi->Fill(chisqu);
	} else {
	  parA->set(colmod,rowmod,0.);
	  parB->set(colmod,rowmod,0.);
	  parC->set(colmod,rowmod,0.);
	  chi2->set(colmod,rowmod,-1.);
	  chi2hi->Fill(-1.);
	}
      }
    }
    can2->cd(2);
    chi2hi->Draw();
    can1->Update();
    can2->Update();
    if(!plotOnly){
      data.writePixLibHisto(parA, PixScan::TOTCAL_PARA, 0, 1);
      data.writePixLibHisto(parB, PixScan::TOTCAL_PARB, 0, 1);
      data.writePixLibHisto(parC, PixScan::TOTCAL_PARC, 0, 1);
      data.writePixLibHisto(chi2, PixScan::TOTCAL_CHI2, 0, 1);
      data.writeFitType(funcID, 0);
    }
  }
}

int main(int argc, const char* argv[]){
	
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  TApplication app("app",NULL,NULL);
  
  
  if(argc<2){cout << "No data name given!" << endl; return -1;}
  string slabel="";
  if(argc>=3) slabel=argv[2];
  unsigned int debcol=7;
  unsigned int debrow=203;
  bool plotOnly = false;
  if(argc==5 || argc==6){
    debcol=atoi(argv[3]);
    debrow=atoi(argv[4]);
  }
  if(argc==6 && string(argv[5])=="true") plotOnly = true;

  if(fillData(string(argv[1]), slabel)){
    procCalib(string(argv[1]), ScanName, GroupName, ModuleName, debcol, debrow, plotOnly);
    
    cout << "Press ctrl-C to stop" << endl;
    app.Run();
  }	
  
  return 0;
}

