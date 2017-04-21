#include "PlotFactory.h"
#include "DataViewer.h"

#include <Fitting/FitClass.h>
#include <DataContainer/PixDBData.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixController/PixScan.h>

#include <QObject>

#include <TMath.h>
#include <TRandom.h>
#include <TColor.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TFormula.h>
#include <TPaveText.h>
#include <TSystem.h>
#include <TGraphErrors.h>
#include <TFrame.h>
#include <TEnv.h>
#include <TPaletteAxis.h>
#include <TClass.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TROOT.h>

MATH2F::MATH2F() : TH2F(), m_dodclk(true), m_flipped(false), m_dv(0){}

MATH2F::MATH2F(MATH2F& oldhi, bool do_dclk, DataViewer *dv) 
  : TH2F((TH2F)oldhi), m_dodclk(do_dclk), m_flipped(false), m_dv(dv){
  // avoid confusion in ROOT by double-naming
  std::string my_name = GetName();
  my_name += "_MA";
  SetName(my_name.c_str());
}

MATH2F::~MATH2F(){}

void MATH2F::ExecuteEvent(Int_t event, Int_t px, Int_t py){
  double x,y;
  float con;
  int bin;
  if((event==kButton1Down || event==kButton1Double)){
    TPad *pad = (TPad*) gPad;
    pad->AbsPixeltoXY(px,py,x,y);
    TH2F *tmphi = new TH2F(*((TH2F*)this));
    bin = tmphi->Fill(x,y);
    con = this->GetBinContent(bin);
    tmphi->Delete();
    if(m_dv!=0) m_dv->processHistoClick((int)(x+0.5), (int)(y+0.5), con, (event==kButton1Double && m_dodclk));
  }
  return;
}
// internal helper functions
void MATH2F::flipHistoCont(){ // only use for FE-I4 (336 rows), otherwise will produce nonsense
  m_flipped = true; // remember this call
  for(int i=1;i<=GetNbinsX();i++){
    for(int j=1;j<169;j++){ // 168 is half of 336
      float temp = GetBinContent(GetBin(i,j));
      int j_temp = 336-j+1;
      SetBinContent(i,j,GetBinContent(GetBin(i,j_temp)));
      SetBinContent(i,j_temp,temp);
      // module with 2 FE rows: also flip top row
      if(GetNbinsY()>336){
	int jtop = j+336;
	temp = GetBinContent(GetBin(i,jtop));
	j_temp = 672-j+1;
	SetBinContent(i,jtop,GetBinContent(GetBin(i,j_temp)));
	SetBinContent(i,j_temp,temp);
      }
    }
  }
  GetYaxis()->SetBinLabel(336,"0");
  GetYaxis()->SetBinLabel(236,"100");
  GetYaxis()->SetBinLabel(136,"200");
  GetYaxis()->SetBinLabel(36,"300");
  if(GetNbinsY()>336){
    GetYaxis()->SetBinLabel(436,"100");
    GetYaxis()->SetBinLabel(536,"200");
    GetYaxis()->SetBinLabel(636,"300");
  }
  GetYaxis()->SetTickLength(0.5*GetYaxis()->GetTickLength());
}
void MATH2F::Draw(Option_t* option){
  TH2F::Draw(option);
  float x, y;
  if(m_dv!=0){
    m_dv->getHistoPixel(x, y);
    if(x>=0 && y>=0){
      if(m_flipped){
	y = 335.-y;
	if(y<0) y += 672.;
      }
      TH2F *tmphi = new TH2F(*((TH2F*)this));
      int bin = tmphi->Fill(x,y);
      float con = this->GetBinContent(bin);
      tmphi->Delete();
      m_dv->processHistoClick((int)(x+0.5), (int)(y+0.5), con, false);
    }
  }
}


//*******************************************************************************************************
PlotFactory::PlotFactory(DataViewer *dv) : m_dv(dv){

  // current histo with mouse click
  m_currmap = 0;
  m_lastmap = 0;
  m_misc_obj.clear();
  // create ROOT canvas
  m_canvas = 0;
  // configuration management
  m_1dplotStyle    = 0;
  m_1dplotRangeMin = 0;
  m_1dplotRangeMax = 0;
  m_1dplotRangeAct = 0;
  setupConfig();
  // fit class for later use
  m_fc = new FitClass();

  // fit functions
  m_totfunc = new TF1("totfu",TOTFunc,0,50000,7);
  m_totfunc->SetParName(0,"Offset");
  m_totfunc->SetParName(1,"Numer. par");
  m_totfunc->SetParName(2,"Denom. par");
  m_totfunc->SetParName(3,"Conv. slope");
  m_totfunc->SetParName(4,"Conv. offs.");
  m_totfunc->SetParName(5,"Conv. quad.");
  m_totfunc->SetParName(6,"Conv. cubic");
  m_erfunc = new TF1("erfu",ComErFunc,0,50000,7);
  m_erfunc->SetParName(0,"Mean");
  m_erfunc->SetParName(1,"Sigma");
  m_erfunc->SetParName(2,"Constant");
  m_erfunc->SetParName(3,"Conv. slope");
  m_erfunc->SetParName(4,"Conv. offs.");
  m_erfunc->SetParName(5,"Conv. quad.");
  m_erfunc->SetParName(6,"Conv. cubic");
  m_expo = new TF1 ("onemexp","[0] - [1]*(exp(-x*[2])-1)",0,2000);
  m_expo->SetParName(0,"Offset");
  m_expo->SetParName(1,"Scaling factor");
  m_expo->SetParName(2,"Exponent factor");
  m_twoexp = new TF1 ("twoexp","[0] + [1]/2*(exp([2]*(x-[3])) - exp(-[2]*(x-[3])))",0,128);
  m_twoexp->SetParName(0,"Offset");
  m_twoexp->SetParName(1,"Scaling factor");
  m_twoexp->SetParName(2,"Exponent factor");
  m_twoexp->SetParName(3,"Saddle point");
  //TF1 *line = new TF1("distbgrd","[0]*x + [1]", 20, 40); // --- guessed values
  TF1 *f1;
  f1 = new TF1("distfunc","[0] +[1]*x +gaus(2)",0,120);
  // for box fits of threshold distributions
  f1 = new TF1("boxfunc", FitClass::boxfun, 0., 1.e6, 5);
  f1->SetParName(0,"plateau");
  f1->SetParName(1,"centre");
  f1->SetParName(2,"width");
  f1->SetParName(3,"sigma_left");
  f1->SetParName(4,"sigma_right");
  // function for RootDB plotting
  m_RDBfunc = 0;
  // canvas top-text
  m_text = new TPaveText(0.0,0.0,1.0,1.0);
  m_text->SetFillColor(10);
  m_text->SetTextAlign(13);
  m_text->SetTextSize(0.3f);
  m_text->SetTextColor(4);

  // Plotting style
  SetStyle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0010);
  int mycolors;
  if((mycolors=gEnv->GetValue("Pad.Color",-1))!=-1)
    gStyle->SetPadColor(mycolors);
  if((mycolors=gEnv->GetValue("Title.FillColor",-1))!=-1)      
    gStyle->SetTitleFillColor(mycolors);
  if((mycolors=gEnv->GetValue("Stat.Color",-1))!=-1)      
    gStyle->SetStatColor(mycolors);

  m_chip = 0;
  m_row  = 0;
  m_col  = 0;

}
PlotFactory::~PlotFactory(){
  ClearMemory();
  //if(m_canvas!=0) m_canvas->Delete();
  delete[] m_1dplotStyle;
  delete[] m_1dplotRangeMin;
  delete[] m_1dplotRangeMax;
  delete[] m_1dplotRangeAct;
  delete m_config;
}
// fit functions
double PlotFactory::ErrorFunc(double *x, double *par){
  double QfromVcal = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[0]*TMath::Erf((QfromVcal-par[1])/sqrt(2.)/par[2]);
  else
    return 0.;
}

double PlotFactory::ComErFunc(double *x, double *par){
  double QfromVcal = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[2]*TMath::Erfc((par[0]-QfromVcal)/sqrt(2.)/par[1]);
  else
    return 0.;
}

double PlotFactory::TwoErFunc(double *x, double *par){
  double QfromVcal = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[0]*(TMath::Erf((par[1]-QfromVcal)/sqrt(2.)/par[2])
                       -TMath::Erf((QfromVcal-par[1])/sqrt(2.)/par[2]));
  else
    return 0.;
}
double PlotFactory::TOTFunc(double *x, double *par){
  double denom = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0]  + par[2];
  if(denom!=0)
    return par[0]+par[1]/denom;
  else
    return 0;
}
// graphics stuff
void PlotFactory::CheckCan(){
  ColourScale(m_colourScheme);

  // user might have closed canvas on error, have to get it back then
  TCanvas *tmpcan;

  // check if object is still in root memory and is what our pointer claims
  tmpcan = (TCanvas*) gROOT->FindObject("pixcan");
  if(m_canvas!=NULL && tmpcan==m_canvas){
    m_canvas->Show();
    return; // all OK
  }
  if(tmpcan!=NULL) {
    //tmpcan->Delete();
    m_canvas = tmpcan;
    m_canvas->Show();
    return;
  }

  // OK, lost it, have to re-create the canvas
  m_canvas = new TCanvas("pixcan","Pixel Module Analysis Canvas",10,10,700,850);
  m_canvas->Clear();
  m_canvas->Show();
  m_canvas->Update();

  return;
}

void PlotFactory::SetStyle(int type){
  switch(type){
  default:
  case 0:
    gStyle->SetLabelSize(0.06f,"X");
    gStyle->SetTitleSize(0.06f,"X");
    gStyle->SetLabelSize(0.06f,"Y");
    gStyle->SetTitleSize(0.06f,"Y");
    gStyle->SetTitleOffset(0.6f,"Y");
    break;
  case 1:
    gStyle->SetLabelSize(0.03f,"X");
    gStyle->SetTitleSize(0.03f,"X");
    gStyle->SetLabelSize(0.03f,"Y");
    gStyle->SetTitleSize(0.03f,"Y");
    gStyle->SetTitleOffset(1,"Y");
    break;
  case 2:
    gStyle->SetLabelSize(0.04f,"X");
    gStyle->SetTitleSize(0.04f,"X");
    gStyle->SetLabelSize(0.04f,"Y");
    gStyle->SetTitleSize(0.04f,"Y");
    gStyle->SetTitleOffset(1.2f,"Y");
    break;
  case 3:
    gStyle->SetLabelSize(0.04f,"X");
    gStyle->SetTitleSize(0.04f,"X");
    gStyle->SetLabelSize(0.04f,"Y");
    gStyle->SetTitleSize(0.04f,"Y");
    gStyle->SetTitleOffset(1.6f,"Y");
    break;
  case 4:
    gStyle->SetLabelSize(0.02f,"X");
    gStyle->SetTitleSize(0.02f,"X");
    gStyle->SetLabelSize(0.02f,"Y");
    gStyle->SetTitleSize(0.02f,"Y");
    gStyle->SetTitleOffset(1.6f,"Y");
    break;
  }
  // pad-related
  switch(type){
  default:
  case 0:
  case 1:
    break;
  case 2:
    gPad->SetRightMargin(0.15f);
    break;
  case 3:
    gPad->SetRightMargin(0.15f);
    gPad->SetLeftMargin(0.15f);
    break;
  }
  return;
}
void PlotFactory::SetStyle(int type, TH2F *histo){
  switch(type){
  default:
  case 0:
    histo->SetLabelSize(0.06f,"X");
    histo->SetTitleSize(0.06f,"X");
    histo->SetLabelSize(0.06f,"Y");
    histo->SetTitleSize(0.06f,"Y");
    histo->SetTitleOffset(0.6f,"Y");
    break;
  case 1:
    histo->SetLabelSize(0.03f,"X");
    histo->SetTitleSize(0.03f,"X");
    histo->SetLabelSize(0.03f,"Y");
    histo->SetTitleSize(0.03f,"Y");
    histo->SetTitleOffset(1,"Y");
    break;
  case 2:
    histo->SetLabelSize(0.04f,"X");
    histo->SetTitleSize(0.04f,"X");
    histo->SetLabelSize(0.04f,"Y");
    histo->SetTitleSize(0.04f,"Y");
    histo->SetTitleOffset(1.2f,"Y");
    break;
  case 3:
    histo->SetLabelSize(0.04f,"X");
    histo->SetTitleSize(0.04f,"X");
    histo->SetLabelSize(0.04f,"Y");
    histo->SetTitleSize(0.04f,"Y");
    histo->SetTitleOffset(1.6f,"Y");
    break;
  }
  // pad-related
  switch(type){
  default:
  case 0:
  case 1:
    break;
  case 2:
    gPad->SetRightMargin(0.15f);
    break;
  case 3:
    gPad->SetRightMargin(0.15f);
    gPad->SetLeftMargin(0.15f);
    break;
  }
  return;
}
void PlotFactory::SetStyle(int type, TH1F *histo){
  switch(type){
  default:
  case 0:
    histo->SetLabelSize(0.06f,"X");
    histo->SetTitleSize(0.06f,"X");
    histo->SetLabelSize(0.06f,"Y");
    histo->SetTitleSize(0.06f,"Y");
    histo->SetTitleOffset(0.6f,"Y");
    break;
  case 1:
    histo->SetLabelSize(0.03f,"X");
    histo->SetTitleSize(0.03f,"X");
    histo->SetLabelSize(0.03f,"Y");
    histo->SetTitleSize(0.03f,"Y");
    histo->SetTitleOffset(1,"Y");
    break;
  case 2:
    histo->SetLabelSize(0.04f,"X");
    histo->SetTitleSize(0.04f,"X");
    histo->SetLabelSize(0.04f,"Y");
    histo->SetTitleSize(0.04f,"Y");
    histo->SetTitleOffset(1.2f,"Y");
    break;
  case 3:
    histo->SetLabelSize(0.04f,"X");
    histo->SetTitleSize(0.04f,"X");
    histo->SetLabelSize(0.04f,"Y");
    histo->SetTitleSize(0.04f,"Y");
    histo->SetTitleOffset(1.6f,"Y");
    break;
  case 4:
    histo->SetLabelSize(0.03f,"X");
    histo->SetTitleSize(0.03f,"X");
    histo->SetLabelSize(0.03f,"Y");
    histo->SetTitleSize(0.03f,"Y");
    histo->SetTitleOffset(1,"Y");
    break;
  }
  // pad-related
  switch(type){
  default:
  case 0:
  case 1:
    break;
  case 2:
    gPad->SetRightMargin(0.15f);
    break;
  case 3:
    gPad->SetRightMargin(0.15f);
    gPad->SetLeftMargin(0.15f);
    break;
  }
  return;
}
void PlotFactory::SaveCan(const char *path){
  CheckCan();
  m_canvas->SaveAs(path);
}
void PlotFactory::ClearMemory(){
  if(m_lastmap!=NULL){
    m_lastmap->Delete();
    m_lastmap = NULL;
  }
  if(m_currmap!=NULL){
    m_currmap->Delete();
    m_currmap = NULL;
  }
  for(unsigned int i=0;i<m_misc_obj.size();i++){
    if(m_misc_obj[i]!=NULL)
      m_misc_obj[i]->Delete();
  }
  m_misc_obj.clear();
  return;
}
std::vector<TPad*> PlotFactory::PadSetup(){
  std::vector<TPad*> pads;
  m_canvas->cd();
  m_canvas->Clear();
  pads.push_back(new TPad("ptxt","Text pad",0,.94,1,1));
  pads[0]->Draw();
  pads.push_back(new TPad("pplt","Plot pad",0,0,1,.94));
  pads[1]->Draw();
  return pads;
}
void PlotFactory::ModuleGrid(TH2F *hi, int nChips){
  int nrow=2, ncol=nChips/2;
  if(nChips<3){
    nrow=1;
    ncol = nChips;
  }
  if(m_axisLabel){
    if(gROOT->FindObject("pplt_1")!=0){ // split plot type
      hi->SetTitleOffset(0.7f,"Y");
      hi->SetLabelSize(.07f,"Y");
    } else {
      hi->SetTitleOffset(1.1f,"Y");
      hi->SetLabelSize(.045f,"Y");
    }
    hi->SetTitleOffset(0.8f,"X");
    hi->SetLabelSize(.04f,"X");
  } else {
    hi->SetTitleOffset(0.4f,"X");
    hi->SetTitleOffset(0.4f,"Y");
    hi->SetLabelOffset(1,"X");
    hi->SetLabelOffset(1,"Y");
    hi->SetLabelSize(.0001f,"X");
    hi->SetLabelSize(.0001f,"Y");
  }
  hi->SetNdivisions(-400-ncol,"X");
  hi->SetNdivisions(-400-nrow,"Y");
  // try manually drawn line
  // max. min. values in x and y
  double xposl = hi->GetXaxis()->GetXmin();
  double xposh = hi->GetXaxis()->GetXmax();
  double yposl = hi->GetYaxis()->GetXmin();
  double yposh = hi->GetYaxis()->GetXmax();
  // average of axis min,max by ncol,row is step width
  double xposm = (xposh-xposl)/(double)ncol;
  double yposm = (yposh-yposl)/(double)nrow;
  for(int i=1;i<ncol;i++){
    TLine *l = new TLine(xposm*(double)i+xposl, yposl, xposm*(double)i+xposl, yposh);
    l->SetLineStyle(3);
    l->SetLineWidth(1);
    l->Draw();
  }
  for(int i=1;i<nrow;i++){
    TLine *l = new TLine(xposl, yposm*(double)i+yposl, xposh, yposm*(double)i+yposl);
    l->SetLineStyle(3);
    l->SetLineWidth(1);
    l->Draw();
  }
  if(m_chipLabel){
    for(int col=0;col<ncol;col++){
      for(int row=0;row<nrow;row++){
        float x,y,xdiff=2,ydiff=5, tsiz=.03f;
        Double_t x1,x2,y1,y2;
        gPad->GetPadPar(x1,y1,x2,y2);
        if((y2-y1)>0){
          ydiff /= (float)(y2-y1);
          tsiz  /= (float)(y2-y1);
	}
	if(nChips==16){ // FE-I3
	  x = 9+18*(float)col;
	  y = 80+160*(float)row;
	} else{
	  x = 40+80*(float)col;
	  y = 168+336*(float)row;	  
	}
        TPaveText *txt = new TPaveText(x-xdiff,y-ydiff,x+xdiff,y+ydiff," "); 
        int chip = col;
        if(row) chip = 2*ncol-1-col;
        char t[2];
        sprintf(t,"%x",chip);
        txt->SetTextSize(tsiz);
        txt->SetBorderSize(0);
        txt->AddText(t);
        txt->Draw();
      }
    }
  }
}
int PlotFactory::PlotMapProjScat(PixDBData &pdbd, int chip, int scanpt[3], int ps_level, int ps_type){
  int pstype2 = -1;
  int scanpt2[3]={-1, -1, -1};
  int oper_type = -1;
  return PlotMapProjScat(pdbd, pdbd, chip, scanpt, ps_level, ps_type, scanpt2, pstype2, oper_type);
}
int PlotFactory::PlotMapProjScat(PixDBData &pdbd, PixDBData &pdbd2, int chip, int scanpt[3], int ps_level, int ps_type, 
				 int scanpt2[3], int ps_type2, int oper_type){
  
  TH1F *mapproj[4]={0,0,0,0};
  MATH2F *maphi;
  TH2F *tmphi=0;
  TGraph *mapdis;
  float *x, *y;
  char line[1024], tit[128], nam[256];
  int i,j, k, nfitted=0,ntot=0, bin, row, col, nbins=100;
  // for TDAC couting
  int ntdach=0, ntdacl=0;
  float mnmin=100000, mnmax=0, mnmean=0, mnrms=0, con, dacmax=31;
  //  float leaktot=0;
  bool takeneg = true;
  int splitHisto = 0;
  int NPLHIMAX = (int)PixLib::PixScan::MAX_HISTO_TYPES;
  int plotLayout = m_plotLayout;
  bool haveFourPads=false;

  if(ps_type<NPLHIMAX){
    if(m_1dplotStyle[ps_type]==MAPONLY) plotLayout = 1;
    if(m_1dplotStyle[ps_type]==MAPSPLHISCAT) splitHisto = 1;
    if(m_1dplotStyle[ps_type]==MAPVSPLHISCAT) splitHisto = 2;
  }

  CheckCan();

  // get module geometry
  int nCol, nRow, nFe;
  pdbd.getGeom(nRow, nCol, nFe);

  // get 2D map and make clickable
  if(oper_type<0){
    if( ps_type<NPLHIMAX ){
      if(ps_level<0)
	pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_type, scanpt[0], ps_level);
      else{
	int ptarr[4]={-1,-1,-1,0};
	for(int ilv=0;ilv<3;ilv++) if(ps_level<=ilv) ptarr[ilv] = scanpt[ilv];
	pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_type, ptarr);
      }
      tmphi = pdbd.getChipMap(chip);
    }else if( ps_type==(NPLHIMAX+1) || ps_type==NPLHIMAX ){
      tmphi = pdbd.getDACMap(chip, (ps_type==NPLHIMAX)?"TDAC":"FDAC");
    }else if( ps_type==NPLHIMAX+2){
      tmphi = pdbd.getMaskMap(chip, "ENABLE");
    }else if( ps_type==NPLHIMAX+3){
      tmphi = pdbd.getMaskMap(chip, (nRow%336==0)?"CAP0":"SELECT");
    }else if( ps_type==NPLHIMAX+4){
      tmphi = pdbd.getMaskMap(chip, (nRow%336==0)?"CAP1":"PREAMP");
    }else if( ps_type==NPLHIMAX+5){
      tmphi = pdbd.getMaskMap(chip, (nRow%336==0)?"ILEAK":"HITBUS");
    }
  } else {
    pdbd.getOperHisto((PixLib::PixScan::HistogramType)ps_type, &pdbd2, (PixLib::PixScan::HistogramType)ps_type2, 
		      (PixDBData::OperType) oper_type, scanpt[ps_level], scanpt2[ps_level], ps_level);
    tmphi = pdbd.getChipMap(chip);
  }

  if(tmphi==0) return -2;
  
  maphi = new MATH2F(*((MATH2F*) tmphi), false, m_dv);
  if(maphi->GetEntries()<=0) maphi->SetEntries(100);
  m_currmap = maphi;

  // flip tmphi for FE-I4
  // and limit T/FDAC plot range
  if(nRow%336==0){
    maphi->flipHistoCont();  
    if(ps_type==NPLHIMAX+1) dacmax = 15;
    if(ps_type==NPLHIMAX)   dacmax = 31;
  } else{
    if(ps_type==NPLHIMAX+1) dacmax = 7;
    if(ps_type==NPLHIMAX)   dacmax = 127;
  }
  // limit range of masks and plot as simple 2D map
  if( ps_type>(NPLHIMAX+1) && ps_type<(NPLHIMAX+6) ){
    dacmax = 1;
    plotLayout = 1;
  }
  
  // get projections + scatter

  x = new Float_t[nCol*nRow*nFe];
  y = new Float_t[nCol*nRow*nFe];

  for(i=1;i<maphi->GetNbinsX()+1;i++){
    for(j=1;j<maphi->GetNbinsY()+1;j++){
      bin = pdbd.PixXYtoInd(i-1,j-1);
      con = maphi->GetBinContent(maphi->GetBin(i,j));
      // process data for plots
      if(bin<(nCol*nRow*nFe) && bin>=0){
        x[bin] = (Float_t) bin;
        y[bin] = con;
      }
      if(con>mnmax) mnmax = con;
      if(con<mnmin && (con>0 || takeneg)) mnmin = con;
      ntot++;
//       if(con>0 || !issfit){
//         mnmean += con;
//         mnrms  += con*con;
//         nfitted++;
//       }
      if( ps_type>=NPLHIMAX && ps_type<(NPLHIMAX+6) ){
        if(con>dacmax-1) ntdach++;
        if(con<1)  ntdacl++;
      }
    }
  }
  if(mnmin==10000) mnmin=0;
  if(mnmax==0)     mnmax=10000;
  if(nfitted>0){
    mnmean /= (float) nfitted;
    mnrms   = mnrms/(float)nfitted - mnmean*mnmean;
    if(mnrms>0){
      mnrms = sqrt(mnrms);
      if(mnmax>mnmean+8*mnrms) mnmax = mnmean+8*mnrms;
      if(mnmin<mnmean-8*mnrms) mnmin = mnmean-8*mnrms;
    }
  }
  if(mnmax>0)
    mnmax *= 1.2f;
  else
    mnmax *= 0.8f;
  if(mnmin>0)
    mnmin *= 0.8f;
  else
    mnmin *= 1.2f;
  
  mapdis = new TGraph(maphi->GetNbinsX()*maphi->GetNbinsY(),x,y);
  m_misc_obj.push_back((TObject*) mapdis);
  
  if( ps_type>=NPLHIMAX && ps_type<(NPLHIMAX+6) ){
    nbins = 1+(int)dacmax;
    mnmin = -.5;
    mnmax = dacmax+.5;
  }

  if(m_useHistoRange){
    mnmin = m_histoRgMin;
    mnmax = m_histoRgMax;
  } else if(ps_type>=0 && ps_type<PixLib::PixScan::MAX_HISTO_TYPES && m_1dplotRangeAct[ps_type]){
    mnmin = m_1dplotRangeMin[ps_type];
    mnmax = m_1dplotRangeMax[ps_type];
  }

  // prepare the plot

  if(splitHisto>0){
    // prepare the histogram for the long pixel
    sprintf(nam,"%s_projl",maphi->GetName());
    if(splitHisto==1)
      sprintf(tit,"Histogrammed (long pixels)");
    else
      sprintf(tit,"Histogrammed (VNCAP pixels)");
    mapproj[1] = (TH1F*) gROOT->FindObject(nam);
    if(mapproj[1]!= NULL) mapproj[1]->Delete();
    mapproj[1] = new TH1F(nam,tit,50,mnmin,mnmax);		// long pixel
    // prepare the histogram for the ganged pixel
    sprintf(nam,"%s_projg",maphi->GetName());
    sprintf(tit,"Histogrammed (ganged pixels)");
    mapproj[2] = (TH1F*) gROOT->FindObject(nam);
    if(mapproj[2]!= NULL) mapproj[2]->Delete();
    mapproj[2] = new TH1F(nam,tit,30,mnmin,mnmax);		// ganged pixel
    // prepare the histogram for the inter-ganged pixel
    sprintf(nam,"%s_projig",maphi->GetName());
    sprintf(tit,"Histogrammed (inter-ganged pixels)");
    mapproj[3] = (TH1F*) gROOT->FindObject(nam);
    if(mapproj[3]!= NULL) mapproj[3]->Delete();
    mapproj[3] = new TH1F(nam,tit,30,mnmin,mnmax);		// interganged pixel
    sprintf(tit,"Histogrammed (std. pixels)");
  } else
    sprintf(tit,"Histogrammed");
  sprintf(nam,"%s_proj",maphi->GetName());
  mapproj[0] = (TH1F*) gROOT->FindObject(nam);	
  if(mapproj[0]!= NULL) mapproj[0]->Delete();
  mapproj[0] = new TH1F(nam, tit, nbins,mnmin,mnmax);

  for(i=1;i<maphi->GetNbinsX()+1;i++){
    for(j=1;j<maphi->GetNbinsY()+1;j++){
      // ganged-interganged-long sectioning
      con = maphi->GetBinContent(maphi->GetBin(i,j));
      if(con>0 || takeneg){
        if(splitHisto>0){
	  // plot center, long/VNCAP and ganged pixels separately
	  pdbd.PixXYtoCCR(i-1,j-1,&k,&col,&row);
	  std::string ptName = pdbd.Pixel_TypeName(pdbd.Pixel_Type(col, row));
	  if(ptName=="GANGED" && splitHisto==1){
	    mapproj[2]->Fill(con);
	    haveFourPads=true;
	  }else if(ptName=="INTER-GANGED"){
	    mapproj[3]->Fill(con);
	    haveFourPads=true;
	  }else if(ptName=="VNCAP" && splitHisto==2)
	    mapproj[1]->Fill(con);
	  else if(ptName=="LONG" && splitHisto==1)
	    mapproj[1]->Fill(con);
	  else
	    mapproj[0]->Fill(con);
	} else{	    // fill the 0-histogramm with all pixels
	  mapproj[0]->Fill(con);
	}
      }
    }
  }

  mapproj[0]->SetMinimum(0.1f); // center pixel
  if(splitHisto) for(i=1;i<4;i++) mapproj[i]->SetMinimum(0.1f);
  
  std::vector<TPad*> pads;
  pads = PadSetup();
  pads[1]->cd();
  if(plotLayout==0){
    if(splitHisto && haveFourPads){
      pads[1]->Divide(1,4,0.001f,0.001f); // divide the Plot window in four parts
      haveFourPads=true;
    } else
      pads[1]->Divide(1,3,0.001f,0.001f); // divide the Plot window in three parts
  }

  // Write text on top of the plot
  m_text->Clear();
  sprintf(line,"%s.",pdbd.GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"", pdbd.GetModName());
  m_text->AddText(line);
  sprintf(line,"%d pixels with DAC=0, %d pixels with DAC=%d",ntdacl, ntdach,(int)dacmax);
  if( ps_type==(NPLHIMAX+1) || ps_type==NPLHIMAX ) m_text->AddText(line);
//   leaktot /=1e3;
//   sprintf(line,"Sum of current: %.3f#muA",leaktot);
//   if(type==LEAK_PLOT || type==MLPLOT) m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();
  pads[1]->cd();

  // Set the plot style
  if(plotLayout==0){
    SetStyle(0);
  }
  else{
    SetStyle(1);
  }
  // prepare the map plot
  if(plotLayout==0){
    pads[1]->cd(1);
  }
  if(plotLayout==0 || plotLayout==1){
    gPad->SetRightMargin(0.15f);
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    maphi->SetMinimum(mnmin);
    maphi->SetMaximum(mnmax);
    maphi->Draw("COLZ");
    if(m_blackBkg){
      m_canvas->Modified();
      m_canvas->Update();
      ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
    }
    // rename axes to col, row and hide tick labels in case this is
    // a known histogram type or requested for PixLib histograms
    maphi->GetXaxis()->SetTitle("Column");
    maphi->GetYaxis()->SetTitle("Row");
    if(chip<0) ModuleGrid(maphi, nFe);	  
    if(plotLayout==0) TitleResize(1.5);
  }
  // prepare the distribution plot
  if(plotLayout==0 || plotLayout==2){
    if(splitHisto){
      // Plot center, long/VNCAP and ganged pixels
      TPad *subpad=NULL;
      if(plotLayout==2) pads[1]->Divide(2,haveFourPads?2:1,0.001f,0.001f);
      for(i=0;i<(haveFourPads?4:2);i++){
	if(plotLayout==0){
	  if(i%2==0){
	    if(i<2){
	      pads[1]->cd(2);
	    }
	    else{
	      pads[1]->cd(3);
	    }
	    subpad = (TPad*)gPad;
	    subpad->Divide(2,1,0.001f,0.001f);
	  }
	  subpad->cd(i%2+1);
	}
	else{
	  pads[1]->cd(i+1);
	}
	if(mapproj[i]->GetEntries()){ 
	  gPad->SetLogy(m_logScale);
	}
	gPad->SetTopMargin(0.15f);
	gPad->SetBottomMargin(0.15f);
	mapproj[i]->Draw();
	mapproj[i]->GetXaxis()->SetTitle(maphi->GetTitle());
	if(m_runFit){
	  mapproj[i]->Fit("gaus","Q");
	}
	if(plotLayout==0){
	  FitBoxResize((TObject*)mapproj[i],1.3f,.7f,.1f);
	  TitleResize(1.5);
	} else{
	  FitBoxResize((TObject*)mapproj[i],.7f,.5f,.1f);
	  TitleResize(.7f);
	}
      }
    } else{
      // Plot only center pixel
      if(plotLayout==0) pads[1]->cd(2);
      if(mapproj[0]->GetEntries()) gPad->SetLogy(m_logScale);
      gPad->SetTopMargin(0.15f);
      gPad->SetBottomMargin(0.15f);
      mapproj[0]->Draw();
      mapproj[0]->GetXaxis()->SetTitle(maphi->GetTitle());
      if(m_runFit){
	mapproj[0]->Fit("gaus","Q");
      }
      if(plotLayout==0){
	FitBoxResize((TObject*)mapproj[0],0.7f);
	TitleResize(1.5f);
      } else {
	FitBoxResize((TObject*)mapproj[0],0.6f,0.4f);
	TitleResize(.7f);
      }
    }
  }
  //Prepare the scatter plot
  if(plotLayout==0) pads[1]->cd(haveFourPads?4:3);

  if(plotLayout==0 || plotLayout==3){
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    mapdis->SetMarkerStyle(1);
    mapdis->SetMaximum(mnmax);
    mapdis->SetMinimum(mnmin);
    mapdis->Draw("AP");
    char grtit[500];
    sprintf(grtit,"\"Channel\" = row+%d*column+%d*chip",nRow,nCol*nRow);
    mapdis->GetXaxis()->SetTitle(grtit);
    mapdis->GetYaxis()->SetTitle(maphi->GetTitle());
    if(chip<0){
      mapdis->GetXaxis()->SetRangeUser(0,nFe*nCol*nRow);
    }
    else{
      mapdis->GetXaxis()->SetRangeUser(0,nCol*nRow);
    }
    mapdis->SetTitle("Scatter plot");
    if(plotLayout==0) TitleResize(1.5);
  }
  
  m_canvas->Modified();
  m_canvas->Update();

  delete[] x;
  delete[] y;

  return 0;
}

int PlotFactory::PlotHisto(PixDBData &pdbd, int scanpt, int ps_level, int ps_type){
  char line[1000];

  TH1F *tmphi = dynamic_cast<TH1F*>(pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_type, scanpt, ps_level));
  if(tmphi==0) return -2;

  // plotting

  CheckCan();
  std::vector<TPad*> pads;
  pads = PadSetup();
  
  m_text->Clear();
  sprintf(line,"%s.",pdbd.GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",pdbd.GetModName());
  m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();

  pads[1]->cd();
  SetStyle(0);
  tmphi->Draw();
  TitleResize(0.7f);

  if(m_useHistoRange)
    tmphi->GetXaxis()->SetRangeUser(m_histoRgMin, m_histoRgMax);
  else if(ps_type>=0 && ps_type<PixLib::PixScan::MAX_HISTO_TYPES && m_1dplotRangeAct[ps_type])
    tmphi->GetXaxis()->SetRangeUser(m_1dplotRangeMin[ps_type], m_1dplotRangeMax[ps_type]);

  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}

int PlotFactory::PlotGraph(std::vector<PixDBData*> pdbds){
  TGraph *indgraph=0;
  char line[1024];
  int i, ngraphs=(int)pdbds.size();

  CheckCan();

  if(ngraphs==0) return -2;

  TMultiGraph *mg = new TMultiGraph();
  TLegend *leg = NULL;
  if(ngraphs>1) leg = new TLegend(.5f,.95f-.05f*(float)ngraphs,.99f,.99f);
  for(i=0;i<ngraphs;i++){
    sprintf(line,"tmpgr%d",i);
    indgraph = (TGraph*) gROOT->FindObject(line);
    if(indgraph!=NULL) indgraph->Delete();
    indgraph = pdbds[i]->GetGraph();
    if(indgraph==NULL) return -2;
    indgraph->SetMarkerStyle(20);
    indgraph->SetMarkerColor(1+i);
    indgraph->SetLineColor(1+i);
    indgraph->SetMarkerSize(0.5f);
    mg->Add(indgraph);
    if(leg!=NULL) leg->AddEntry(indgraph,pdbds[i]->GetName(),"p");
  }
  mg->SetTitle("");

  std::vector<TPad*> pads;
  pads = PadSetup();
  
  m_text->Clear();
  sprintf(line," ");
  for(i=0;i<ngraphs;i++){
    if(i<ngraphs-1)
      sprintf(line,"%s%s + ",line, pdbds[i]->GetName());
    else
      sprintf(line,"%s%s.",line,pdbds[i]->GetName());
  }
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",pdbds[0]->GetModName());
  m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();

  pads[1]->cd();
  gPad->SetRightMargin(0.15f);
  mg->Draw("APL");
  mg->GetXaxis()->SetTitle(indgraph->GetXaxis()->GetTitle());
  mg->GetXaxis()->SetTitleSize(.03f);
  mg->GetXaxis()->SetLabelSize(.03f);
  mg->GetYaxis()->SetTitle(indgraph->GetYaxis()->GetTitle());
  mg->GetYaxis()->SetTitleOffset(1.2f);
  mg->GetYaxis()->SetTitleSize(.03f);
  mg->GetYaxis()->SetLabelSize(.03f);
  if(leg!=NULL) leg->Draw();
  m_canvas->Update();
  m_canvas->Modified();

  return 0;
}

int PlotFactory::GenericPlotting(PixDBData &pdbd, int chip, int scanpt[3], int ps_level, int ps_type){
  if(m_1dplotStyle[ps_type]==GRAPH1D){
    std::vector<PixDBData*> pdbd_vec;
    pdbd_vec.push_back(&pdbd);
    return PlotGraph(pdbd_vec);
  }
  if(m_1dplotStyle[ps_type]==HISTOASIS)
    return PlotHisto(pdbd, scanpt[ps_level], ps_level, ps_type);
  
  // default: colour map, histogram, scatter plot
  return PlotMapProjScat(pdbd, chip, scanpt, ps_level, ps_type);
}

int PlotFactory::plotValVsPar(PixDBData &pdbd, int ps_type, int scan_level, int scanidx[3], int in_chip, 
			      int sel_chip, int sel_col, int sel_row, std::vector<int> pars, int fid){
  char line[1024];
  int nCol, nRow, nFe, nFeRows;
  pdbd.getGeom(nRow, nCol, nFe, nFeRows);
  if(nFeRows==0) nFeRows=1;
  int nColTot = nCol*nFe/nFeRows, nRowTot=nRow*nFeRows;
  int ptarr[4]={-1,-1,-1,0};
  for(int ilv=0;ilv<3;ilv++) if(scan_level<=ilv) ptarr[ilv] = scanidx[ilv];

  gROOT->cd();
  TH2F *tmpHi = 0, *tmpToTHi=0;
  MATH2F *map = 0;
  if(in_chip<0){
    tmpHi = new TH2F("accdata","",nColTot,-.5,-0.5+(double)nColTot,nRowTot,-0.5,-0.5+nRowTot);
  }
  else{
    tmpHi = new TH2F("accdata","",nCol,-0.5,-0.5+(double)nCol,nRow,-0.5,-0.5+(double)nRow);
  }
  map = new MATH2F(*((MATH2F*)tmpHi), true, m_dv);
  m_currmap = map;
  tmpHi->Delete(); tmpHi=0;

  int i;
  std::vector<float> scanpt  = pdbd.getScanPoints(scan_level);
  int steps = scanpt.size();
  double nevts = (double)pdbd.getNevents();
  if(pdbd.getScanPar(scan_level)=="CAPSEL"){
    // scan against inj. capacitance, retrieve values from FE
    // TODO: consider >1 FE
    const int nCap=3;
    std::string capLabels[nCap]={"CInjLo", "CInjMed", "CInjHi"};
    Config &conf = pdbd.getModConfig().subConfig("PixFe_0/PixFe");;
    for(int i=0;i<steps;i++){
      int scanpti = (int)scanpt[i];
      float cInj = 0.;
      if(scanpti>=0 && scanpti<nCap && conf["Misc"].name()!="__TrashConfGrp__"){
	if(conf["Misc"][capLabels[scanpti]].name()!="__TrashConfObj__")
	  cInj = (dynamic_cast<ConfFloat &>(conf["Misc"][capLabels[scanpti]])).value();
	else
	  std::cout << "Can't find ConfObj " << capLabels[scanpti] << std::endl;
      }
      scanpt[i] = cInj;
    }
  }

  double *xval = new double[steps], *yval = new double[steps];
  double *xerr = new double[steps], *yerr = new double[steps];
  int binx,biny;
  std::string title="";
  pdbd.m_keepOpen=true;
  for(i=0;i<steps;i++){
    ptarr[scan_level] = i;
    pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_type, ptarr);
    tmpHi = pdbd.getChipMap(in_chip);
    if(tmpHi!=0) tmpHi = new TH2F(*tmpHi);
    if(ps_type==(int)PixLib::PixScan::TOT_MEAN){ // get sigma histo for error later on, if it was saved
      pdbd.getGenericHisto(PixLib::PixScan::TOT_SIGMA, i, scan_level);
      tmpToTHi = pdbd.getChipMap(in_chip);
      if(tmpToTHi!=0) tmpToTHi = new TH2F(*tmpToTHi);
    }
    if(tmpHi!=0){
      if(title==""){
	title = tmpHi->GetTitle();
	title += " - accumulated";
	if(in_chip>=0){
	  std::stringstream a;
	  a << in_chip;
	  title += " - chip "+a.str();
	}
	map->SetTitle(title.c_str());
      }
      map->Add(tmpHi);
      if(in_chip<0){
	pdbd.PixCCRtoBins(sel_chip, sel_col, sel_row, binx, biny);
      } else{
	pdbd.PixCCRtoBins(0, sel_col, sel_row, binx, biny);
      }
      xval[i] = (double)scanpt[i];
      yval[i] = (double)tmpHi->GetBinContent(binx,biny);
      xerr[i] = 1e-4*xval[i];
      switch(ps_type){
      case (int)PixLib::PixScan::OCCUPANCY: // binomial error
	if(nevts>0)
	  yerr[i] = yval[i]/nevts*(nevts-yval[i]);
	else
	  yerr[i] = yval[i];
	if(yerr[i]>0)       
	  yerr[i] = sqrt(yerr[i]);
	break;
      case (int)PixLib::PixScan::TOT_MEAN: // error from stored sigma
	if(tmpToTHi!=0)
	  yerr[i] = (double)tmpToTHi->GetBinContent(binx,biny);
	else
	  yerr[i] = 1e-4*yval[i];
	break;
      default: // don't know what to do - have no error
	yerr[i] = 1e-4*yval[i];
      }
    } else{
      xval[i] = (double)scanpt[i];
      xerr[i] = 1e-4*xval[i];
      yval[i] = 0.;
      yerr[i] = 0.;
    }
    delete tmpHi; tmpHi=0;
    delete tmpToTHi; tmpToTHi=0;
  }
  pdbd.m_keepOpen=false;

  if(map==0) return -2;

  gROOT->cd();
  TGraphErrors *gr = new TGraphErrors(steps,xval,yval, xerr, yerr);
  int pos;
  // graph general titel
  pos = title.find(" - accumulated");
  if(pos!=(int)std::string::npos)
    title.erase(pos,title.length()-pos);
  std::stringstream strs;
  strs << "Chip " << sel_chip << " col " << sel_col << " row " << sel_row;
  gr->SetTitle(strs.str().c_str());

  // graph axes title
  if(pdbd.getScanPar(scan_level)=="CAPSEL"){
    gr->GetXaxis()->SetTitle("C_{inj} (fF)");
  }else if(pdbd.getScanPar(scan_level)=="FEI4_GR"){
    Config &scfg = pdbd.getScanConfig();
    std::stringstream lnum;
    lnum << scan_level;
    std::string atit = "FEI4_GR";
    if(scfg["loops"]["feGlobRegNameLoop_"+lnum.str()].name()!="__TrashConfObj__")
      atit = ((ConfString&)scfg["loops"]["feGlobRegNameLoop_"+lnum.str()]).value();
    gr->GetXaxis()->SetTitle(atit.c_str());
  }else{
    gr->GetXaxis()->SetTitle(pdbd.getScanPar(scan_level).c_str());
  }
  gr->GetYaxis()->SetTitle(title.c_str());

  gr->SetMarkerStyle(20);
  gr->SetMarkerSize(.6f);
  m_misc_obj.push_back(gr);
  // clean up
  delete xval; delete yval;
  delete xerr; delete yerr;

  TF1 *fun=0;
  // if function plotting was requested, do so
  if(fid>=0){
    delete m_RDBfunc;
    int npar = m_fc->getNPar(fid);
    if(m_fc->hasVCAL(fid)) // allow special VCAL fit
      npar += 4;
    fun = new TF1(m_fc->getFuncName(fid).c_str(), (double (*)(double *, double *))m_fc->getFunc(fid), 
		  scanpt[0], scanpt[steps-1], npar);
    m_RDBfunc = fun; // will allow us to delete this later
    std::vector<std::string> labels = m_fc->getParNames(fid);
    for(int pi=0;pi<m_fc->getNPar(fid);pi++){
      double parVal = 0;
      if(pars[pi]<PixLib::PixScan::MAX_HISTO_TYPES){
	tmpHi = (TH2F*)pdbd.getGenericHisto((PixLib::PixScan::HistogramType)pars[pi], -1, scan_level+1);
	pdbd.PixCCRtoBins(sel_chip, sel_col, sel_row, binx, biny);
	parVal = (tmpHi==0)?0:(double)tmpHi->GetBinContent(binx,biny);
      } else
	parVal = (double)pdbd.getNevents();
      fun->SetParameter(pi,parVal);
      fun->SetParName(pi,labels[pi].c_str());
    }
    if(m_fc->hasVCAL(fid)){ // allow special VCAL fit
      double Cval, parVal;
      const int nCap=3;
      std::string Cname="none", capLabels[nCap]={"CInjLo", "CInjMed", "CInjHi"};
      Config &scfg = pdbd.getScanConfig();
      if(scfg["fe"]["chargeInjCap"].name()!="__TrashConfObj__"){
	int iCap = ((ConfInt&)scfg["fe"]["chargeInjCap"]).value();
	if(iCap>=0 && iCap<nCap) Cname = capLabels[iCap];
      }
      
      // set VCAL calibration parameters
      std::stringstream a;
      a << m_chip;
      Config &mcfg = pdbd.getModConfig().subConfig("PixFe_"+a.str()+"/PixFe");
      if(mcfg["Misc"].name()!="__TrashConfGrp__"){
	if(mcfg["Misc"][Cname].name()=="__TrashConfObj__")
	  return -99;
	Cval = (double)((ConfFloat&)mcfg["Misc"][Cname]).value();
	for(int j=0;j<4;j++){
	  std::stringstream b;
	  b << j;
	  if(mcfg["Misc"]["VcalGradient"+b.str()].name()=="__TrashConfObj__")
	    return -99;
	  parVal = Cval*(double)(((ConfFloat&)mcfg["Misc"]["VcalGradient"+b.str()]).value())/0.160218;
	  fun->SetParameter(j+m_fc->getNPar(fid),parVal);
	  std::stringstream a;
	  a << j;
	  fun->SetParName(j+m_fc->getNPar(fid),("VCAL par "+a.str()).c_str());
	}
      }
    }
  }

  CheckCan();
  std::vector<TPad*> pads;
  pads = PadSetup();
  
  m_text->Clear();
  sprintf(line,"%s.",pdbd.GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",pdbd.GetModName());
  m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();
  
  pads[1]->cd();
  
  if(nRow%336==0) map->flipHistoCont();
  pads[1]->Divide(1,2,0.001f,0.001f); // divide the Plot window in two parts
  SetStyle(1);
  pads[1]->cd(1);
  gPad->SetRightMargin(0.15f);
  gPad->SetTopMargin(0.15f);
  gPad->SetBottomMargin(0.15f);
  map->Draw("COLZ");
  if(m_blackBkg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  map->GetXaxis()->SetTitle("Column");
  map->GetYaxis()->SetTitle("Row");
  if(in_chip<0) ModuleGrid(map, nFe);	  
  //TitleResize(1.5);
  pads[1]->cd(2);    
  gPad->SetLeftMargin(0.12f);
  gr->Draw("AP");
  gr->GetXaxis()->SetTitleSize(.03f);
  gr->GetXaxis()->SetLabelSize(.03f);
  gr->GetYaxis()->SetTitleOffset(1.8f);
  gr->GetYaxis()->SetTitleSize(.03f);
  gr->GetYaxis()->SetLabelSize(.03f);
  // plot fit function if it exists
  if(fun!=0){
    fun->SetLineColor(2);
    fun->SetLineWidth(1);
    fun->Draw("SAME");
    PlotFit(fun);
  }
  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}

int PlotFactory::plotAvgVsPar(PixDBData &pdbd, int ps_type, int scan_level, int scanidx[3], int in_chip){
  char line[1024];
  int nCol, nRow, nFe, nFeRows;
  pdbd.getGeom(nRow, nCol, nFe, nFeRows);
  if(nFeRows==0) nFeRows=1;
  int ptarr[4]={-1,-1,-1,0};
  for(int ilv=0;ilv<3;ilv++) if(scan_level<=ilv) ptarr[ilv] = scanidx[ilv];

  gROOT->cd();
  TH2F *tmpHi = 0, *tmpToTHi=0;

  int i,j,row,col;
  std::vector<float> scanpt  = pdbd.getScanPoints(scan_level);
  int steps = scanpt.size();
  if(pdbd.getScanPar(scan_level)=="CAPSEL"){
    // scan against inj. capacitance, retrieve values from FE
    // TODO: consider >1 FE
    const int nCap=3;
    std::string capLabels[nCap]={"CInjLo", "CInjMed", "CInjHi"};
    Config &conf = pdbd.getModConfig().subConfig("PixFe_0/PixFe");;
    for(int i=0;i<steps;i++){
      int scanpti = (int)scanpt[i];
      float cInj = 0.;
      if(scanpti>=0 && scanpti<nCap && conf["Misc"].name()!="__TrashConfGrp__"){
	if(conf["Misc"][capLabels[scanpti]].name()!="__TrashConfObj__")
	  cInj = (dynamic_cast<ConfFloat &>(conf["Misc"][capLabels[scanpti]])).value();
	else
	  std::cout << "Can't find ConfObj " << capLabels[scanpti] << std::endl;
      }
      scanpt[i] = cInj;
    }
  }

  double *xval = new double[steps], *yval = new double[steps];
  double *xerr = new double[steps], *yerr = new double[steps];
  int binx,biny, binxf,binyf, cmin=0, cmax=nFe;
  std::string title;
  if(in_chip>=0){
    cmin = in_chip;
    cmax = cmin+1;
  }
  pdbd.m_keepOpen=true;
  for(i=0;i<steps;i++){
    ptarr[scan_level] = i;
    tmpHi = (TH2F*)pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_type, ptarr);
    if(tmpHi!=0) tmpHi = new TH2F(*tmpHi);
    if(ps_type==(int)PixLib::PixScan::TOT_MEAN){ // get sigma histo for error later on, if it was saved
      tmpToTHi = (TH2F*)pdbd.getGenericHisto(PixLib::PixScan::TOT_SIGMA, i, scan_level);
      if(tmpToTHi!=0) tmpToTHi = new TH2F(*tmpToTHi);
    }
    title = tmpHi->GetTitle();
    double nval=0.;
    yval[i] = 0.;
    yerr[i] = 0.;
    for(j=cmin; j<cmax; j++){
      for(col=0;col<nCol; col++){
	for(row=0;row<nRow; row++){
	  if(in_chip<0){
	    pdbd.PixCCRtoBins(j, col, row, binx, biny);
	    binxf = binx; binyf = biny;
	  } else{
	    pdbd.PixCCRtoBins(0, col, row, binx, biny);
	    pdbd.PixCCRtoBins(in_chip, col, row, binxf, binyf);
	  }
	  if(tmpHi!=0 && tmpHi->GetBinContent(binxf,binyf)!=0){
	    yval[i] += (double)tmpHi->GetBinContent(binxf,binyf);
	    yerr[i] += ((double)tmpHi->GetBinContent(binxf,binyf))*((double)tmpHi->GetBinContent(binxf,binyf));
	    nval += 1.;
	  }
	}
      }
    }
    xval[i] = (double)scanpt[i];
    xerr[i] = 1e-4*xval[i];
    if(nval>1){
      yval[i] /= nval;
      yerr[i] = (yerr[i]-nval*yval[i]*yval[i])/(nval-1);
      if(yerr[i]>0) yerr[i] = sqrt(yerr[i]);
      else          yerr[i] = 0.;
      if(ps_type==(int)PixLib::PixScan::SCURVE_MEAN || ps_type==(int)PixLib::PixScan::SCURVE_SIGMA ||
	 ps_type==(int)PixLib::PixScan::TIMEWALK){ 
	// delay, threshold, noise can have outliers, so use Gauss-fit
	double hmin = yval[i]-5*yerr[i];
	double hmax = yval[i]+5*yerr[i];
	// ignore extreme spreads due to outliers
	if(yerr[i]>10*yval[i]) {
	  hmin = 0.1*yval[i];
	  hmax = 10*yval[i];
	}
	TH1F *avghi = new TH1F("avghi","avghi",100, hmin, hmax);
	for(j=cmin; j<cmax; j++){
	  for(col=0;col<nCol; col++){
	    for(row=0;row<nRow; row++){
	      if(in_chip<0){
		pdbd.PixCCRtoBins(j, col, row, binx, biny);
		binxf = binx; binyf = biny;
	      } else{
		pdbd.PixCCRtoBins(0, col, row, binx, biny);
		pdbd.PixCCRtoBins(in_chip, col, row, binxf, binyf);
	      }
	      avghi->Fill(tmpHi->GetBinContent(binxf,binyf));
	    }
	  }
	}
	int nhits = (int)(avghi->GetEntries()-avghi->GetBinContent(0)-avghi->GetBinContent(avghi->GetNbinsX()));
	if(nhits>5){
	  avghi->Fit("gaus","0Q");
	  yval[i] = avghi->GetFunction("gaus")->GetParameter(1);
	  yerr[i] = avghi->GetFunction("gaus")->GetParameter(2);
	} else{
	  yval[i] = 0.;
	  yerr[i] = 0.;
	}
	avghi->Delete();
      }
    }else{
      yval[i] = 0.;
      yerr[i] = 0.;
    }
    delete tmpHi; tmpHi=0;
    delete tmpToTHi; tmpToTHi=0;
  }
  pdbd.m_keepOpen=false;

  gROOT->cd();
  TGraphErrors *gr = new TGraphErrors(steps,xval,yval, xerr, yerr);
  gr->SetTitle("");

  // graph axes title
  if(pdbd.getScanPar(scan_level)=="CAPSEL"){
    gr->GetXaxis()->SetTitle("C_{inj} (fF)");
  }else if(pdbd.getScanPar(scan_level)=="FEI4_GR"){
    Config &scfg = pdbd.getScanConfig();
    std::stringstream lnum;
    lnum << scan_level;
    std::string atit = "FEI4_GR";
    if(scfg["loops"]["feGlobRegNameLoop_"+lnum.str()].name()!="__TrashConfObj__")
      atit = ((ConfString&)scfg["loops"]["feGlobRegNameLoop_"+lnum.str()]).value();
    gr->GetXaxis()->SetTitle(atit.c_str());
  }else{
    gr->GetXaxis()->SetTitle(pdbd.getScanPar(scan_level).c_str());
  }
  gr->GetYaxis()->SetTitle(("Avg. of "+title).c_str());
  gr->SetMarkerStyle(20);
  gr->SetMarkerSize(.6f);
  m_misc_obj.push_back(gr);

  CheckCan();
  std::vector<TPad*> pads;
  pads = PadSetup();
  
  m_text->Clear();
  sprintf(line,"Graph of module average: %s - errors bars are %s",pdbd.GetName(),
	  (ps_type==(int)PixLib::PixScan::SCURVE_MEAN || ps_type==(int)PixLib::PixScan::SCURVE_SIGMA ||
	   ps_type==(int)PixLib::PixScan::TIMEWALK)?"Gaussian sigma":"spread");
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",pdbd.GetModName());
  m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();
  
  pads[1]->cd();
  gPad->SetLeftMargin(0.12f);
  gr->Draw("AP");
  gr->GetXaxis()->SetTitleSize(.03f);
  gr->GetXaxis()->SetLabelSize(.03f);
  gr->GetYaxis()->SetTitleOffset(1.8f);
  gr->GetYaxis()->SetTitleSize(.03f);
  gr->GetYaxis()->SetLabelSize(.03f);
  m_canvas->Modified();
  m_canvas->Update();
  
  // clean up
  delete xval; delete yval;
  delete xerr; delete yerr;

  return 0;
}
int  PlotFactory::PlotCorrel(PixDBData &pdbd, PixDBData &pdbd2, int chipID, int scanpt[3], int ps_level, 
			     int ps_pida, int ps_pidb){
  TH2F *hia=0, *hib=0;
  TGraph *scatter=0;
  TH2F* scathi=0;
  int nBins;
  int NPLHIMAX = (int)PixLib::PixScan::MAX_HISTO_TYPES;

  // get module geometry
  int nCol, nRow, nFe;
  pdbd.getGeom(nRow, nCol, nFe);
  int nColb, nRowb, nFeb;
  pdbd2.getGeom(nRowb, nColb, nFeb);
  if(nFe!=nFeb || nCol!=nColb || nRow!=nRowb) return -9;
  nBins=nCol*nRow;
  if(chipID<0) nBins *= nFe;

  if( ps_pida<NPLHIMAX ){
    if(ps_level<0)
      hia = dynamic_cast<TH2F*>(pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_pida, scanpt[0], ps_level));
    else{
      int ptarr[4]={-1,-1,-1,0};
      for(int ilv=0;ilv<3;ilv++) if(ps_level<=ilv) ptarr[ilv] = scanpt[ilv];
      pdbd.getGenericHisto((PixLib::PixScan::HistogramType)ps_pida,  ptarr);
    }
    hia = pdbd.getChipMap(chipID);
  }else if( ps_pida==(NPLHIMAX+1) || ps_pida==NPLHIMAX ){
    hia = pdbd.getDACMap(chipID, (ps_pida==NPLHIMAX)?"TDAC":"FDAC");
  }else if( ps_pida==NPLHIMAX+2){
    hia = pdbd.getMaskMap(chipID, "ENABLE");
  }else if( ps_pida==NPLHIMAX+3){
    hia = pdbd.getMaskMap(chipID, (nRow%336==0)?"CAP0":"SELECT");
  }else if( ps_pida==NPLHIMAX+4){
    hia = pdbd.getMaskMap(chipID, (nRow%336==0)?"CAP1":"PREAMP");
  }else if( ps_pida==NPLHIMAX+5){
    hia = pdbd.getMaskMap(chipID, (nRow%336==0)?"ILEAK":"HITBUS");
  }
  // temporary histo would be removed if getting a new one from same data object
  hia = new TH2F(*hia); // so create copy of this one
  hia->SetName("corrain");
  if( ps_pidb<NPLHIMAX ){
    if(ps_level<0)
      hib = dynamic_cast<TH2F*>(pdbd2.getGenericHisto((PixLib::PixScan::HistogramType)ps_pidb, scanpt[0], ps_level));
    else{
      int ptarr[4]={-1,-1,-1,0};
      for(int ilv=0;ilv<3;ilv++) if(ps_level<=ilv) ptarr[ilv] = scanpt[ilv];
      pdbd2.getGenericHisto((PixLib::PixScan::HistogramType)ps_pidb, ptarr);
    }
    hib = pdbd2.getChipMap(chipID);
  }else if( ps_pidb==(NPLHIMAX+1) || ps_pidb==NPLHIMAX ){
    hib = pdbd2.getDACMap(chipID, (ps_pidb==NPLHIMAX)?"TDAC":"FDAC");
  }else if( ps_pidb==NPLHIMAX+2){
    hib = pdbd2.getMaskMap(chipID, "ENABLE");
  }else if( ps_pidb==NPLHIMAX+3){
    hib = pdbd2.getMaskMap(chipID, (nRow%336==0)?"CAP0":"SELECT");
  }else if( ps_pidb==NPLHIMAX+4){
    hib = pdbd2.getMaskMap(chipID, (nRow%336==0)?"CAP1":"PREAMP");
  }else if( ps_pidb==NPLHIMAX+5){
    hib = pdbd2.getMaskMap(chipID, (nRow%336==0)?"ILEAK":"HITBUS");
  }
  hib = new TH2F(*hib); // so create copy of this one
  hib->SetName("corrbin");

  if(hia==0 || hib==0) return -2;

  Float_t *x, *y;
  x = new Float_t[nFe*nCol*nRow];
  y = new Float_t[nFe*nCol*nRow];
  Float_t mina=100000, maxa=0, meana=0, rmsa=0;
  Float_t minb=100000, maxb=0, meanb=0, rmsb=0;
  Int_t i,j,bin,ntot=0, nbina=50, nbinb=50;
  char axislabel[300], line[1024];

  CheckCan();

  for(i=1;i<hia->GetNbinsX()+1;i++){
    for(j=1;j<hia->GetNbinsY()+1;j++){
      bin = pdbd.PixXYtoInd(i-1,j-1);
      if(bin<(nFe*nCol*nRow) && bin>=0){
        ntot++;
	x[bin] = hia->GetBinContent(hia->GetBin(i,j));
	y[bin] = hib->GetBinContent(hib->GetBin(i,j));
        if(x[bin]>maxa) maxa = x[bin];
        if(x[bin]<mina) mina = x[bin];
        meana += x[bin];
        rmsa  += x[bin]*x[bin];
        if(y[bin]>maxb) maxb = y[bin];
        if(y[bin]<minb) minb = y[bin];
        meanb += y[bin];
        rmsb  += y[bin]*y[bin];
     }
    }
  }
  // delete temp histos
  hia->Delete();
  hib->Delete();

  if(ntot>0){
    meana /= (float) ntot;
    rmsa   = rmsa/(float)ntot - meana*meana;
    if(rmsa>0){
      rmsa = sqrt(rmsa);
      if(maxa>meana+8*rmsa) maxa = meana+8*rmsa;
      if(mina<meana-8*rmsa) mina = meana-8*rmsa;
    }
    meanb /= (float) ntot;
    rmsb   = rmsb/(float)ntot - meanb*meanb;
    if(rmsb>0){
      rmsb = sqrt(rmsb);
      if(maxb>meanb+8*rmsb) maxb = meanb+8*rmsb;
      if(minb<meanb-8*rmsb) minb = meanb-8*rmsb;
    }
  }
  if(m_useHistoRange){
    mina = m_histoRgMin;
    maxa = m_histoRgMax;
    minb = m_histoRgMin;
    maxb = m_histoRgMax;
  } else {
    if(ps_pida>=0 && ps_pida<PixLib::PixScan::MAX_HISTO_TYPES && m_1dplotRangeAct[ps_pida]){
      mina = m_1dplotRangeMin[ps_pida];
      maxa = m_1dplotRangeMax[ps_pida];
    }
    if(ps_pidb>=0 && ps_pidb<PixLib::PixScan::MAX_HISTO_TYPES && m_1dplotRangeAct[ps_pidb]){
      minb = m_1dplotRangeMin[ps_pidb];
      maxb = m_1dplotRangeMax[ps_pidb];
    }
  }

  if(m_scatplt==0){
    scatter = new TGraph(nBins,x,y);
    m_misc_obj.push_back((TObject*) scatter);
    scatter->SetMaximum(maxb);
    scatter->SetMinimum(minb);
  } else if (m_scatplt==1){
    scathi = (TH2F*) gROOT->FindObject("corrplt");
    if(scathi!=NULL) scathi->Delete();
    scathi = new TH2F("corrplt","Correlation plot",nbina,mina,maxa,nbinb,minb,maxb);
    for(i=0;i<nBins;i++)
      scathi->Fill(x[i],y[i]);
    sprintf(line,"Correlation plot - correl. coeff. = %.4lf",scathi->GetCorrelationFactor());
    scathi->SetTitle(line);
  } else{
    delete[] x;
    delete[] y;
    return -5;
  }

  delete[] x;
  delete[] y;

  std::vector<TPad*> pads;
  pads = PadSetup();
  pads[0]->cd();
  m_text->Clear();
  std::string  pslabela, pslabelb;
  PixScan *ps = new PixScan();
  std::map<std::string, int> hiTypes = ps->getHistoTypes();
  for(std::map<std::string, int>::iterator it = hiTypes.begin(); it!=hiTypes.end(); it++){
    if(it->second==ps_pida)
      pslabela = it->first;
    if(it->second==ps_pidb)
      pslabelb = it->first;
  }
  delete ps;
  sprintf(line,"%s: %s dataset  vs ",pslabela.c_str(),pdbd.GetName());
  sprintf(line,"%s%s: %s",line,pslabelb.c_str(),pdbd2.GetName());
  m_text->AddText(line);
  sprintf(line,"Modules \"%s\" and \"%s\"", pdbd.GetModName(), pdbd2.GetModName());
  if(chipID>=0)
    sprintf(line,"%s - chip %d",line,chipID);
  m_text->AddText(line);
  m_text->Draw();

  pads[1]->cd();
  gPad->SetLeftMargin(0.15f);
  gPad->SetRightMargin(0.15f);
  if(m_scatplt==0){
    scatter->SetMarkerStyle(1);
    scatter->Draw("AP");
    scatter->GetXaxis()->SetRangeUser(mina,maxa);
    scatter->SetTitle("Correlation plot");
    // titles and size matters
    sprintf(axislabel,"%s:  %s",pdbd.GetName(),pslabela.c_str());
    scatter->GetXaxis()->SetTitle(axislabel);
    sprintf(axislabel,"%s:  %s",pdbd2.GetName(),pslabelb.c_str());
    scatter->GetYaxis()->SetTitle(axislabel);
    scatter->GetXaxis()->SetLabelSize(0.04f);
    scatter->GetXaxis()->SetTitleSize(0.04f);
    scatter->GetYaxis()->SetLabelSize(0.04f);
    scatter->GetYaxis()->SetTitleSize(0.04f);
    scatter->GetYaxis()->SetTitleOffset(1.2f);
  } else if(m_scatplt==1){
    scathi->Draw("COLZ");
    // titles and size matters
    sprintf(axislabel,"%s:  %s",pdbd.GetName(),pslabela.c_str());
    scathi->SetXTitle(axislabel);
    sprintf(axislabel,"%s:  %s",pdbd2.GetName(),pslabelb.c_str());
    scathi->SetYTitle(axislabel);
    scathi->SetLabelSize(0.04f,"X");
    scathi->SetTitleSize(0.04f,"X");
    scathi->SetTitleOffset(1.2f,"X");
    scathi->SetLabelSize(0.04f,"Y");
    scathi->SetTitleSize(0.04f,"Y");
    scathi->SetTitleOffset(1.8f,"Y");
  } else
    return -5;

  m_canvas->cd();
  m_canvas->Modified();
  m_canvas->Update();
  
  return 0;
}

void PlotFactory::TitleResize(float factor){
  gPad->Modified();
  gPad->Update();
  TPaveText *tx = (TPaveText*) gPad->FindObject("title");
  if(tx!=NULL){
    tx->SetX2NDC(tx->GetX1NDC()+factor*(tx->GetX2NDC() - tx->GetX1NDC()));
    tx->SetY1NDC(tx->GetY2NDC()-factor*(tx->GetY2NDC() - tx->GetY1NDC()));
    tx->Paint(tx->GetDrawOption());
  }
  return;
}
void PlotFactory::FitBoxResize(TObject *obj, float factorx, float factory, float shift){
  float fx=factorx, fy=factory;
  if(fy<0) fy = factorx;
  gPad->Modified();
  gPad->Update();
  TPaveText *tx = (TPaveText*) obj->FindObject("stats");
  if(tx!=NULL){
    tx->SetX1NDC(tx->GetX2NDC()-fx*(tx->GetX2NDC() - tx->GetX1NDC()));
    // new ROOT version seems to resize fit box incorrectly
#if ROOT_VERSION_CODE <= ROOT_VERSION(3,10,2)
    //tx->SetY1NDC(tx->GetY2NDC()-fy*(tx->GetY2NDC() - tx->GetY1NDC())-shift);
#else
    tx->SetY1NDC(tx->GetY2NDC()-fy*(tx->GetY2NDC() - 0.65)-shift);
#endif
    tx->SetY2NDC(tx->GetY2NDC()-shift);
    tx->Paint(tx->GetDrawOption());
  }
  return;
}
void PlotFactory::ColourScale(int type){
  Int_t i;
  Float_t colfrac;
  TColor *color;
  int    palette[100];// 2D-map colour palette black-red-yellow
  switch(type){
  case 0:
    // black-red-yellow
    // start with black-to-red
    for(i=0;i<60;i++){
      colfrac = (Float_t)i/60;
      if(! gROOT->GetColor(201+i)){
        color = new TColor (201+i,colfrac,0,0,"");
      }else{
        color = gROOT->GetColor(201+i);
        color->SetRGB(colfrac,0,0);
      }
      palette[i]=201+i;
    }
    // red-to-yellow now
    for(i=0;i<40;i++){
      colfrac = (Float_t)i/40;
      if(! gROOT->GetColor(261+i)){
        color = new TColor (261+i,1,colfrac,0,"");
      }else{
        color = gROOT->GetColor(261+i);
        color->SetRGB(1,colfrac,0);
      }
      palette[i+60]=261+i;
    }
    gStyle->SetPalette(100,palette); 
    break;
  case 1:
    // black-red, *no* yellow
    for(i=0;i<100;i++){
      colfrac = (Float_t)i/100;
      if(! gROOT->GetColor(201+i)){
        color = new TColor (201+i,colfrac,0,0,"");
      }else{
        color = gROOT->GetColor(201+i);
        color->SetRGB(colfrac,0,0);
      }
      palette[i]=201+i;
    }
    gStyle->SetPalette(100,palette); 
    break;
  case 2:
    // rainbow
    gStyle->SetPalette(1); 
    break;
  case 3:
    // black-blue-purple-red-orange-yellow-white (TurboDAQ style)
    for(i=0;i<20;i++){ // black to blue
      colfrac = (Float_t)i/20;
      if(! gROOT->GetColor(201+i)){
        color = new TColor (201+i,0,0,colfrac,"");
      }else{
        color = gROOT->GetColor(201+i);
        color->SetRGB(0,0,colfrac);
      }
      palette[i]=201+i;
    }
    for(i=0;i<20;i++){ // blue to purple
      colfrac = 0.8*(Float_t)i/20;
      if(! gROOT->GetColor(221+i)){
        color = new TColor (221+i,colfrac,0,1,"");
      }else{
        color = gROOT->GetColor(221+i);
        color->SetRGB(colfrac,0,1);
      }
      palette[20+i]=221+i;
    }
    for(i=0;i<20;i++){ // purple to red
      colfrac = (Float_t)i/20;
      if(! gROOT->GetColor(241+i)){
        color = new TColor (241+i,0.8+0.2*colfrac,0,1-colfrac,"");
      }else{
        color = gROOT->GetColor(241+i);
        color->SetRGB(0.8+0.2*colfrac,0,1-colfrac);
      }
      palette[40+i]=241+i;
    }
    for(i=0;i<25;i++){ // red to orange to yellow
      colfrac = (Float_t)i/25;
      if(! gROOT->GetColor(261+i)){
        color = new TColor (261+i,1,colfrac,0,"");
      }else{
        color = gROOT->GetColor(261+i);
        color->SetRGB(1,colfrac,0);
      }
      palette[60+i]=261+i;
    }
    for(i=0;i<15;i++){ // yellow to white
      colfrac = (Float_t)i/15;
      if(! gROOT->GetColor(286+i)){
        color = new TColor (286+i,1,1,colfrac,"");
      }else{
        color = gROOT->GetColor(286+i);
        color->SetRGB(1,1,colfrac);
      }
      palette[80+i]=286+i;
    }
    gStyle->SetPalette(100,palette); 
    break;
  case 4:
    // black-white
    for(i=0;i<90;i++){
      colfrac = (Float_t)i/100;
      if(! gROOT->GetColor(201+i)){
        color = new TColor (201+i,colfrac,colfrac,colfrac,"");
      }else{
        color = gROOT->GetColor(201+i);
        color->SetRGB(colfrac,colfrac,colfrac);
      }
      palette[i]=201+i;
    }
    gStyle->SetPalette(90,palette); 
    break;
  default:
    break;
  }
}
void PlotFactory::setupConfig(){
  m_config = new PixLib::Config("PlotFactory");

  Config &cfg = *m_config;

  cfg.addGroup("General");
  cfg["General"].addString("dataPath", m_dataPath, ".", "Default path to data files", true, 2);
  cfg["General"].addBool("startChipMode", m_startChipMode, false, "Start DataViewer in single-chip mode", true);

  cfg.addGroup("Plotting");
  std::map<std::string, int> layouts;
  layouts["Split (map/histogram/scatter)"] = 0;
  layouts["Colour map only"] = 1;
  layouts["1D-Histogram only"] = 2;
  layouts["Scatter plot only"] = 3;
  std::map<std::string, int> schemes;
  schemes["Black-red-yellow"] = 0;
  schemes["Black-red"] = 1;
  schemes["Rainbow"] = 2;
  schemes["TurboDAQ-style"] = 3;
  schemes["Grey-scale"] = 4;
  std::map<std::string, int> correlStyles;
  correlStyles["Scatter"] = 0;
  correlStyles["Colour map"] = 1;
  cfg["Plotting"].addList("Layout", m_plotLayout, 0, layouts, "Layout of plots in general (overwritten by histo def.)", true);
  cfg["Plotting"].addList("colourScheme", m_colourScheme, 0, schemes, "Colour scheme for 2D maps", true);
  cfg["Plotting"].addBool("2DchipLabel", m_chipLabel, true, "Show chip label in 2D colour plots", true);
  cfg["Plotting"].addBool("2DaxisLabel", m_axisLabel, true, "Show axis labels in 2D colour plots", true);
  cfg["Plotting"].addBool("2DblackBkg", m_blackBkg, false, "Plot background of colour maps black", true);
  cfg["Plotting"].addBool("logScale", m_logScale, false, "Log scale: Y-axis for projections, X-axis for timewalk", true);
  cfg["Plotting"].addBool("runFit", m_runFit, true, "Fit Gauss to 1D-histograms", true);
  cfg["Plotting"].addBool("useHistoRg", m_useHistoRange, false, "Apply general fixed range to X-axis of 1D-histograms", true);
  cfg["Plotting"].addFloat("histoRgMin", m_histoRgMin, -9999., "Value of min. of fixed range for X-axis", true);
  cfg["Plotting"].addFloat("histoRgMax", m_histoRgMax, 9999., "Value of max. of fixed range for X-axis", true);
  cfg["Plotting"].addList("correlStyle", m_scatplt, 1, correlStyles, "Style for correlation plots", true);

  PixScan ps;
  std::map<std::string, int> ps_his = ps.getHistoTypes();
  m_1dplotStyle = new int[PixLib::PixScan::MAX_HISTO_TYPES];
  m_1dplotRangeMin = new float[PixLib::PixScan::MAX_HISTO_TYPES];
  m_1dplotRangeMax = new float[PixLib::PixScan::MAX_HISTO_TYPES];
  m_1dplotRangeAct = new bool[PixLib::PixScan::MAX_HISTO_TYPES];
  cfg.addGroup("HistoPlotOpts");
  cfg.addGroup("HistoPlotRange");
  std::map<std::string, int> plotStyles;
  plotStyles["Map/1x histogram/scatter"] = MAPHISCAT;
  plotStyles["Map/split histogram (long etc.)/scatter"] = MAPSPLHISCAT;
  plotStyles["Map/split histogram (VCAP)/scatter"] = MAPVSPLHISCAT;
  plotStyles["Colour map only"] = MAPONLY;
  plotStyles["Histogram as it is"] = HISTOASIS;
  plotStyles["Plot 1D-histo as graph"] = GRAPH1D;
  for(std::map<std::string, int>::iterator IT=ps_his.begin(); IT!=ps_his.end();IT++){
    int defPlotStyle = MAPHISCAT;
    if(IT->second==PixLib::PixScan::OCCUPANCY) defPlotStyle = MAPONLY;
    if(IT->second==PixLib::PixScan::HITOCC) defPlotStyle = MAPONLY;
    if(IT->second==PixLib::PixScan::RAW_DATA_0) defPlotStyle = HISTOASIS;
    if(IT->second==PixLib::PixScan::RAW_DATA_1) defPlotStyle = HISTOASIS;
    if(IT->second==PixLib::PixScan::TOT) defPlotStyle = HISTOASIS;
    if(IT->second==PixLib::PixScan::LVL1) defPlotStyle = HISTOASIS;
    if(IT->second==PixLib::PixScan::LV1ID) defPlotStyle = HISTOASIS;
    if(IT->second==PixLib::PixScan::BCID) defPlotStyle = HISTOASIS;
    if(IT->second==PixLib::PixScan::DCS_DATA) defPlotStyle = GRAPH1D;
    if(IT->second==PixLib::PixScan::HIT_RATE) defPlotStyle = GRAPH1D;
    if(IT->second==PixLib::PixScan::TRG_RATE) defPlotStyle = GRAPH1D;
    std::string label="2dplotStyle"+IT->first;
    std::string descript = "Plotting style for histo "+IT->first;
    cfg["HistoPlotOpts"].addList(label, m_1dplotStyle[IT->second], defPlotStyle, plotStyles, descript, true);
    label="plotRangeAct"+IT->first;
    descript = "Plotting range limit. active for histo "+IT->first;
    cfg["HistoPlotRange"].addBool(label, m_1dplotRangeAct[IT->second], false, descript, true);
    label="plotRangeMin"+IT->first;
    descript = "Plotting range minimum for histo "+IT->first;
    cfg["HistoPlotRange"].addFloat(label, m_1dplotRangeMin[IT->second], -99999., descript, true);
    label="plotRangeMax"+IT->first;
    descript = "Plotting range maximum for histo "+IT->first;
    cfg["HistoPlotRange"].addFloat(label, m_1dplotRangeMax[IT->second], 99999., descript, true);
  }
  cfg.reset();
}
void PlotFactory::PlotFit(TF1* fu){
  float x1, x2, y1, y2, dx, dy;
  char tmp[256];
  std::string name[10], num[10], txt;
  int npar, charwidth, i;
  if(fu==NULL) return;
  npar = fu->GetNpar();
  x2 = gPad->GetX2();
  y2 = gPad->GetY2();
  dx = x2-gPad->GetX1();
  dy = y2-gPad->GetY1();
  x1 = x2-0.3*dx;
  y1 = y2-(0.07+0.07*(float)(npar+1))*dy;
  x2 -= 0.04*dx;
  y2 -= 0.04*dy;
  TPaveText *tx = new TPaveText(x1,y1,x2,y2,"NDC");
  charwidth = 0;
  tx->AddText(fu->GetName());
  for(i=0;i<npar;i++){
    name[i]  = fu->GetParName(i);
    if(TMath::Abs(fu->GetParameter(i))>=.1 &&TMath::Abs(fu->GetParameter(i))<1e4)
      sprintf(tmp,"%.2f",fu->GetParameter(i));
    else
      sprintf(tmp,"%.2e",fu->GetParameter(i));
    num[i]   = tmp;
    if((int)(name[i]+num[i]).length()>charwidth) charwidth = (name[i]+num[i]).length();
  }
  charwidth+=4;
  for(i=0;i<npar;i++){
    txt = name[i] + " " + num[i];
    while((int)txt.length()<charwidth){
      name[i] += " ";
      txt = name[i] + " " + num[i];
    }
    tx->AddText(txt.c_str());
  }
  //tx->SetTextFont(40);//10*((int)tx->GetTextFont()/10));
  tx->SetTextAlign(12);
  tx->Draw();
  return;
}
