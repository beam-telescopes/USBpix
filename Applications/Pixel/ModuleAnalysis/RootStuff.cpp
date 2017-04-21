#include <string.h> 

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

#include <q3table.h>
#include <q3header.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qmessagebox.h>

//#include <stdexception>
#include <vector>

#include "RootStuff.h"
#include "ModTree.h"
#include "BadPixel.h"
#include "MATPalette.h"
#include "StdTestWin.h"
#include "OptClass.h"
#ifdef PIXEL_ROD // can link to PixScan
#include "PixDBData.h"
#include <PixScan.h>
#include <FitClass.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#endif

MATH2F::MATH2F() : TH2F(){
  m_parnt  = NULL;
  m_dodclk = true;
}
MATH2F::MATH2F(MATH2F& oldhi, bool do_dclk) : TH2F((TH2F)oldhi){
  m_parnt  = NULL;
  m_dodclk = do_dclk;
  // avoid confusion in ROOT by double-naming
  std::string my_name = GetName();
  my_name += "_MA";
  SetName(my_name.c_str());
}
MATH2F::~MATH2F(){
}
void MATH2F::ExecuteEvent(Int_t event, Int_t px, Int_t py){
  Double_t x,y;
  Float_t con;
  Int_t bin;
  int row, col, chip, binx, biny;
  if((event==kButton1Down || event==kButton1Double) && m_parnt!=NULL){
    m_parnt->m_PlotIsActive = true;
    TPad *pad = (TPad*) gPad;
    pad->AbsPixeltoXY(px,py,x,y);
    TH2F *tmphi = new TH2F(*((TH2F*)this));
    bin = tmphi->Fill(x,y);
    con = this->GetBinContent(bin);
    tmphi->Delete();
    if(m_parnt->m_lastPitem->GetPID()==MAMASK && m_parnt->options->m_Opts->m_ToggleMask &&
       event == kButton1Double){
      if(m_parnt->m_lastchip>=0){ // change bin to fit whole map
        m_parnt->m_lastPitem->GetDataItem()->PixXYtoCCR((int)x,(int)y,&chip,&col,&row);
        m_parnt->m_lastPitem->GetDataItem()->PixCCRtoBins(m_parnt->m_lastchip,col,row,binx,biny);
      } else
        m_parnt->m_lastPitem->GetDataItem()->PixXYtoBins(x,y,binx,biny);
      if(con) con=0;
      else    con=1;
      m_parnt->m_lastPitem->GetDataItem()->GetMap(-1,MAMASK)->SetBinContent(binx,biny,con);
    }
    if(event!=kButton1Double || m_dodclk){
      if(GetNbinsX()>8) // full pixel TH2F
	m_parnt->m_lastPitem->GetDataItem()->PixXYtoCCR((int)(x+.5),(int)(y+.5),&chip,&col,&row);
      else{             // summary TH2F (one bin per chip)
	col = 0;
	row = 0;
	if(((int)(y+.5))==0)
	  chip = (int)(x+.5);
	else
	  chip = 15-(int)(x+.5);
      }
      m_parnt->MapAction(chip,col,row,con,event);
    }
    m_parnt->m_PlotIsActive = false;
  }
  return;
}
//****************** changeable color palette, by JW ****************************************************
ClassImp(MATPaletteAxis)

MATPaletteAxis::MATPaletteAxis(RMain *rootparnt) : TPaletteAxis(){
  m_parnt = rootparnt;
}
MATPaletteAxis::MATPaletteAxis(MATPaletteAxis& oldpal, RMain *rootparnt) : TPaletteAxis((TPaletteAxis) oldpal){
  m_parnt = rootparnt;
}
MATPaletteAxis::~MATPaletteAxis(){
}
void MATPaletteAxis::ExecuteEvent(Int_t event, Int_t px, Int_t py){
  double n;
  std::string str, substr;
  Double_t max, min;

  if(event==kButton1Double && m_parnt!=NULL){
    str = this->GetObjectInfo(px, py);
        substr=str.substr(str.find_first_not_of("(z=)"), str.size()-4);//find_last_of(")"));
        n = atof(substr.c_str());
        max=this->fH->GetMaximum();
        min=this->fH->GetMinimum();
        if(n>(min+(max-min)/2))
      m_parnt->RescaleOverview((int)min, (int)n);//this->fH->GetMinimum(), this->fH->GetMaximum());
        else
          m_parnt->RescaleOverview((int)n, (int)max);
  }
}
void MATPaletteAxis::SetMinMax(int new_Minimum, int new_Maximum){
  if(m_parnt!=0)
    m_parnt->RescaleOverview(new_Minimum, new_Maximum);
}
//*******************************************************************************************************
RMain::RMain(optClass *optptr){
  m_MapInfo = new MapInfo();
  // current histo with mouse click
  m_currmap = 0;
  m_lastmap = 0;
  m_misc_obj.clear();
  // create ROOT canvas
  m_canvas = 0;
  //CheckCan();
  m_psfile = 0;
  m_palette = 0;
  // RMain's fit functions
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
  // standard fit functions - make sure they are around
  float x[4]={1,2,3,4}, y[4]={2,3,4,7};
  TGraph g(4,x,y);
  g.Fit("pol0","0Q");
  g.Fit("pol1","0Q");
  g.Fit("pol2","0Q");
  g.Fit("pol3","0Q");
  g.Fit("expo","0Q");
  g.Fit("gaus","0Q");
  // function for RootDB lotting
  m_RDBfunc = 0;
  // canvas top-text
  m_text = new TPaveText(0.0,0.0,1.0,1.0);
  m_text->SetFillColor(10);
  m_text->SetTextAlign(13);
  m_text->SetTextSize(0.3f);
  m_text->SetTextColor(4);
  // caption for stave/bistave overview
/*  m_stname = new TPaveText(0,0,1,1);
  m_stname->SetFillColor(10);
  m_stname->SetTextAlign(12);
  m_stname->SetTextSize(0.5f);
  m_stname->SetTextColor(4);*/

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
  // options pointer
  m_plotOpts = optptr;
  // 1D plot setting with random initialisation
  TDatime *dt = new TDatime();
  TRandom r(3600*dt->GetHour()+60*dt->GetMinute()+dt->GetSecond());
  delete dt;
  m_chip = 0;//r.Integer(2);	// random start chip
  m_row  = 0;//r.Integer(336);	// random start row
  m_col  = 0;//r.Integer(80);	// random start column
  // default DAC(VCAL) -> Qinj conversion
  int i,j;
  m_convfac = new float*[NCHIP];
  for(i=0;i<NCHIP;i++)
    m_convfac[i] = new float[4];
  for(i=0;i<NCHIP;i++){
    m_convfac[i][0] = 44.6f; 
    for(j=1;j<4;j++)
      m_convfac[i][j] = 0;
  } 

  m_fc = new FitClass();

  return;
}
RMain::~RMain(){
  for(int i=0;i<NCHIP;i++){
    delete m_convfac[i];
    m_convfac[i] = 0;
  }
  delete[] m_convfac;
  delete m_MapInfo;
  delete m_RDBfunc;
}
// main plotting routines
int RMain::PlotMapScanhi (ModItem *item, int chip, int scanpt, TH2F *user_hi, TObject *user_gr, TF1 *fun){
  int sparr[3]={scanpt, 0, 0};
  return PlotMapScanhi(item, chip, sparr, user_hi, user_gr, -1, fun);
}
int RMain::PlotMapScanhi(ModItem *item, int chip, int scanpt[3], TH2F *user_hi, 
			 TObject *user_gr, int ps_level, TF1 *in_fu){
  
  char line[1024];
  float ylow=0;
  int i, type = item->GetPID();
  int ps_type=-1;
  int binx,biny;
  float parval=0, capfac, calfac[4]={1,0,0,0};
  std::string funcname;
  TH1F *scanhi=NULL;
  TH2F *scanmap=NULL;
  TGraphErrors *scangr=NULL;
  TH2F *tmphi;
  MATH2F *rawmap;
  TF1* fu=NULL;

  CheckCan();

#ifdef PIXEL_ROD // can link to PixScan
  if(item->GetPID()>=PIXDBTYPES) ps_type = item->GetPID()-PIXDBTYPES;
#endif

  // get 2D map and make it clickable
  tmphi = 0;
  if(user_hi!=0)
    tmphi = user_hi;
  else if(item->GetPID()<PIXDBTYPES)
    tmphi = item->GetDataItem()->GetMap(chip,type);
  else{
#ifdef PIXEL_ROD // can link to PixScan
    PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
    if(pdbd!=0){
      if(ps_level<0)
	tmphi = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							  m_chip, m_col, m_row, scanpt[0], ps_level));
      else{
	int ptarr[4]={-1,-1,-1,0};
	for(int ilv=0;ilv<3;ilv++) if(ps_level<=ilv) ptarr[ilv] = scanpt[ilv];
	tmphi = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,  
							  m_chip, m_col, m_row, ptarr));
      }
      if(chip>=0 && !(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
		      m_plotOpts->m_PLplotType[ps_type]==0)) 
	tmphi = pdbd->getChipMap(chip);
    }
#endif
  }
  if(tmphi==0) return -2;
  
  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  // flip tmphi for FE-I4 - better use nCol, nRow?
  if(tmphi->GetNbinsY()%336==0) flipHistoCont(tmphi);

  rawmap = new MATH2F(*((MATH2F*) tmphi));
  if(rawmap->GetEntries()<=0) rawmap->SetEntries(100);
  m_currmap = rawmap;

  // 1D Scurve histo and fit results (if there)
  if(user_gr!=0){
    scangr = (TGraphErrors*) user_gr;
    if(scangr==0)
      scanhi = (TH1F*) user_gr;
    if(scangr==0 && scanhi==0)
      scanmap = (TH2F*) user_gr;
  } else if(item->GetPID()<PIXDBTYPES){
    switch(type){
    case TOT_DIST:
    case TOT_FDIS:
    case CAL_DIST:
    case CAL_FDIS:
      scanhi = item->GetDataItem()->GetToTDist(m_chip, m_col, m_row, false, item->GetRID());
      break;
    case TOTRAW:
      if(item->GetDataItem()->getScanLevel()==1){
	scangr = item->GetDataItem()->GetToTHi(m_chip, m_col, m_row);
	if(scangr==0)
	  scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, AVTOT);
      }else if(item->GetDataItem()->getScanLevel()==2)
	scanmap = item->GetDataItem()->GetScanMap(m_chip, m_col, m_row, TOTRAW);
      else
	scanhi = item->GetDataItem()->GetToTDist(m_chip, m_col, m_row, !m_plotOpts->m_FitGauss);
      break;
    case TOTSD:
      if(item->GetDataItem()->getScanLevel()==1)
	scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, SIGTOT);
      else if(item->GetDataItem()->getScanLevel()==2)
	scanmap = item->GetDataItem()->GetScanMap(m_chip, m_col, m_row, TOTSD);
      break;
    case TWODRAW:
      scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, TWODMEAN);
      break;
    case TWODRAWS:
      scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, TWODSIG);
      break;
    case TWODRAWC:
      scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, TWODCHI);
      break;
    case RAWSLC:
      scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, NONE, scanpt[0]);
      break;
    default:
      if(item->GetDataItem()->getScanLevel()==1)
	scangr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row, RAW);
      else if(item->GetDataItem()->getScanLevel()==2)
	scanmap = item->GetDataItem()->GetScanMap(m_chip, m_col, m_row,RAW);
      break;
    }
  }

  // user-defined range if required
  if(type<DSET && m_plotOpts->m_UsrIUse[type]){
    rawmap->SetMinimum(m_plotOpts->m_UsrIMin[type]);
    rawmap->SetMaximum(m_plotOpts->m_UsrIMax[type]);
    if(scanhi!=NULL)
      scanhi->GetXaxis()->SetRangeUser(m_plotOpts->m_UsrIMin[type],m_plotOpts->m_UsrIMax[type]);
    if(scangr!=NULL)
      scangr->GetXaxis()->SetRangeUser(m_plotOpts->m_UsrIMin[type],m_plotOpts->m_UsrIMax[type]);
  }
  if(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
     m_plotOpts->m_PLUsrIUse[ps_type]){
    rawmap->SetMinimum(m_plotOpts->m_PLUsrIMin[ps_type]);
    rawmap->SetMaximum(m_plotOpts->m_PLUsrIMax[ps_type]);
//     if(scanhi!=NULL)
//       scanhi->GetXaxis()->SetRangeUser(m_plotOpts->m_PLUsrIMin[ps_type],m_plotOpts->m_PLUsrIMax[ps_type]);
//     if(scangr!=NULL)
//       scangr->GetXaxis()->SetRangeUser(m_plotOpts->m_PLUsrIMin[ps_type]m_plotOpts->m_PLUsrIMax[ps_type]);
  }
  if(m_plotOpts->m_doUsrRg){
    rawmap->SetMinimum(m_plotOpts->m_UsrMin);
    rawmap->SetMaximum(m_plotOpts->m_UsrMax);
    if(scanhi!=NULL)
      scanhi->GetXaxis()->SetRangeUser(m_plotOpts->m_UsrMin,m_plotOpts->m_UsrMax);
    if(scangr!=NULL)
      scangr->GetXaxis()->SetRangeUser(m_plotOpts->m_UsrMin,m_plotOpts->m_UsrMax);
  }

  // plotting

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *spplt=NULL;
  if(((scanhi!=NULL && scanhi->GetEntries()>0) || scangr!=NULL  || scanmap!=NULL ) 
     && m_plotOpts->m_plotfitopt==0){
    spplt = new TPad("spplt","Sub-plot pad",0,0,1,.45);
    spplt->Draw();
    ylow = 0.45f;
  }
  TPad *pplt = new TPad("pplt","Plot pad",0,ylow,1,.94);
  pplt->Draw();
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  if(ps_type<0)
    sprintf(line,"%s: %s.",m_plotOpts->axtit[type].c_str(),item->GetDataItem()->GetName());
  else{
    std::string pslabel = item->text(0).latin1();
    int pos = pslabel.find("Plot ");
    if(pos!=(int)std::string::npos)
      pslabel.erase(pos,5);
    sprintf(line,"%s: %s.",pslabel.c_str(),item->GetDataItem()->GetName());
  }
  //  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd();
  if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==1) {
    if(!(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
	 m_plotOpts->m_PLplotType[ps_type]==0)){
      if(item->GetDataItem()->getNevents()>=0 && (i=item->GetDataItem()->getNevents()-5) < rawmap->GetMinimum())
	rawmap->SetMinimum((float)i);
      if(item->GetDataItem()->getNevents()>=0 && (i=item->GetDataItem()->getNevents()+5) > rawmap->GetMaximum())
	rawmap->SetMaximum((float)i);
    }
    rawmap->Draw("COLZ");
    if(m_plotOpts->m_doBlackBg){
      m_canvas->Modified();
      m_canvas->Update();
      ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
    }
    if(m_plotOpts->m_plotfitopt==0)
      SetStyle(2);
    else
      SetStyle(1);
    if(!(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
	 m_plotOpts->m_PLplotType[ps_type]==0)){
      rawmap->GetXaxis()->SetTitle("Column");
      rawmap->GetYaxis()->SetTitle("Row");
      if(chip<0) ModuleGrid(rawmap, nFe);
      //      else       flipAxis(rawmap);
    }
  }

  if(((scanhi!=0 && scanhi->GetEntries()>0) || scangr!=0 || scanmap!=0) && 
     (m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt>1)){
    if(m_plotOpts->m_plotfitopt==0){
      spplt->cd();
      SetStyle(0);
    } else
      SetStyle(1);
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    if(scanhi!=0)       scanhi->Draw();
    else if(scangr!=0)  scangr->Draw("AP");
    else if(scanmap!=0){
      scanmap->Draw(m_plotOpts->m_corPlotOpt.c_str());
      scanmap->GetYaxis()->SetTitleOffset(1.2f);
    }

    // get fit function if data was fitted
    fu = 0;
    if(item->GetDID()!=0)
      fu = GetFitFunc(item->GetDID()->m_fittype);
    // load fit data
    if(fu!=0){
      if(scanhi!=0)
        fu->SetRange(scanhi->GetXaxis()->GetXmin(), scanhi->GetXaxis()->GetXmax());
      else if(scangr!=0)
        fu->SetRange(scangr->GetXaxis()->GetXmin(), scangr->GetXaxis()->GetXmax());
      for(i=0;i<fu->GetNpar();i++){
        parval = 0;
        int hisType = PARA+i;
        if(hisType>PARE)
          hisType = PARF-5+i;
        tmphi = item->GetDataItem()->GetMap(-1,hisType);
        if(tmphi!=NULL){
          item->GetDataItem()->PixCCRtoBins(m_chip, m_col, m_row, binx, biny);
          parval = tmphi->GetBinContent(binx,biny);
        }
         fu->SetParameter(i,(double)parval);
      }
      // check if data for S-curve fit was fitted here or read from file
      // have to guess plateau and conversion of from file
      if(item->GetDID()!=0 && 
	 ((item->GetDID()->m_fittype==0 && item->GetDID()->GetMap(-1,PARC)==NULL) ||
         (item->GetDID()->m_fittype==1 && item->GetDID()->GetMap(-1,PARD)==NULL) ||
	  (item->GetDID()->m_fittype>8 && item->GetDID()->m_fittype<12)) ){
        calfac[0] = 1;
        calfac[1] = 0;
        calfac[2] = 0;
        calfac[3] = 0;
        if(item->GetDID()->GetLog()!=NULL){
          if(item->GetDID()->GetLog()->MyModule->CHigh)
            capfac  = item->GetDID()->GetLog()->MyModule->Chips[m_chip]->Chi;
          else
            capfac  = item->GetDID()->GetLog()->MyModule->Chips[m_chip]->Clo;
          if(item->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE"){
            calfac[0] = capfac*item->GetDID()->GetLog()->MyModule->Chips[m_chip]->Vcal/0.160218;
            calfac[1] = 0;
            calfac[2] = capfac*item->GetDID()->GetLog()->MyModule->Chips[m_chip]->VcalQu/0.160218;
            calfac[3] = capfac*item->GetDID()->GetLog()->MyModule->Chips[m_chip]->VcalCu/0.160218;
          }else if(item->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff")
            calfac[0] = item->GetDID()->GetLog()->MyModule->Chips[chip]->Clo*
              item->GetDID()->GetLog()->extcal/0.160218;
          else
            calfac[0] = 1;
        }
        fu->SetParameter(3,calfac[0]);
        fu->SetParameter(4,calfac[1]);
        fu->SetParameter(5,calfac[2]);
        fu->SetParameter(6,calfac[3]);
        if(item->GetDID()->m_fittype==0 || item->GetDID()->m_fittype==9){
          parval = item->GetDID()->GetLog()->MyScan->Nevents;
          if(parval<=0) parval = 100;
          fu->SetParameter(2,parval);
        }
      }
    }
    if(in_fu!=0)
      fu = in_fu;
    if(fu!=0){
      fu->SetLineColor(2);
      fu->SetLineWidth(1);
      fu->Draw("SAME");
      PlotFit(fu);
    }

    if(m_plotOpts->m_plotfitopt==0) TitleResize(1.5);
    else                            TitleResize(0.7f);
  }
  
  m_canvas->Modified();
  m_canvas->Update();
  //gStyle->SetOptFit(0010);
  /*
  if (type == TOT_DIST) {
    gStyle->SetStatX(0.5f);
    gStyle->SetStatY(0.8f);
  }
  */  
  return 0;
}
int RMain::PlotMapProjScat(ModItem *item, int chip, int scanpt, TH2F *user_hi, int ps_level){
  int ptarr[3]={scanpt,0,0};
  return PlotMapProjScat(item, chip, ptarr, user_hi, ps_level);
}
int RMain::PlotMapProjScat(ModItem *item, int chip, int scanpt[3], TH2F *user_hi, int ps_level){
  
  TH1F *mapproj[4]={0,0,0,0};
  MATH2F *maphi;
  TH2F *tmphi;
  TGraph *mapdis;
  Float_t *x, *y;
  Char_t line[1024], tit[128], nam[256];
  Int_t i,j, k, nfitted=0,ntot=0, bin, row, col, nbins=100;
  Int_t type = item->GetPID(), ps_type=-1;
  if(type>=DSET) type = DSET-1;
  BadPixel *badpix = NULL;
  QString toptitle;
  bool allow_dclick;
  // for TDAC couting
  int ntdach=0, ntdacl=0;
  Float_t mnmin=100000, mnmax=0, mnmean=0, mnrms=0, con, leaktot=0, dacmax=31;
  Bool_t takeneg   = type==CHI || type==MLPLOT || type==LEAK_PLOT || type==TDACP || type==FDACP ||
                     type==TDACLOG || type==FDACLOG || (type>=PARA && type<=PARE) || type==TOT_MAX;
  Bool_t issfit    = type==MEAN || type==SIG || type==CHI || type==TWODMEAN || type==TWODSIG || type==TWODCHI
                     || type==TWODMINTC;
  Bool_t splitproj = type==SIG || type == TWLK_OVERDRV_2D || type == TWLK_Q20NS_2D || type == TOT_SUMM
                      || type == TOT_FAST || type== CAL_SUMM || type==CAL_FAST || type==TWODMINTN 
                      || type==TWODSIG;
  Bool_t split_thrp= type==MEAN || type==TWODMINT || type==TWODMEAN;
  Bool_t totpeak   = type==TOT_SUMM || type==CAL_SUMM || type==TOT_FAST || type==CAL_FAST;

  CheckCan();

  // create bad pixel table if needed

  if ( type==CHI ) {
    badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
    toptitle = "Fit failed";
    badpix->Init("chi^2",toptitle.latin1());
  } else if ( type == TDACLOG || type==TDACP ) {
    badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
    toptitle = "TDAC=0,max pixel list";
    badpix->Init("TDAC",toptitle.latin1());
  } else if ( type == FDACLOG || type==FDACP ) {
    badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
    toptitle = "FDAC=0,max pixel list";
    badpix->Init("FDAC",toptitle.latin1());
  }

  // get 2D map and make clickable
  tmphi = 0;
  if(user_hi!=0){
    tmphi = user_hi;
    takeneg = true;
  }else{
    if(type==T20K)
      tmphi = TOT20K(chip,item->GetDID());
    else if(type>=TWODMEAN && type<=TWODOPT)
      tmphi  = item->GetDataItem()->Get2DMap(chip,type,scanpt[0]);
    else if(item->GetPID()<PIXDBTYPES)
      tmphi  = item->GetDataItem()->GetMap(chip,type);
    else{
#ifdef PIXEL_ROD // can link to PixScan
      ps_type = item->GetPID()-PIXDBTYPES;
      if(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type])
	takeneg = m_plotOpts->m_PLPltZero[ps_type];
      else
	takeneg = true;
      PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
      if(pdbd!=0){
	if(ps_level<0)
	  tmphi = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							    m_chip, m_col, m_row, scanpt[0], ps_level));
	else{
	  int ptarr[4]={-1,-1,-1,0};
	  for(int ilv=0;ilv<3;ilv++) if(ps_level<=ilv) ptarr[ilv] = scanpt[ilv];
	  tmphi = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							    m_chip, m_col, m_row, ptarr));
	}
	if(chip>=0) tmphi = pdbd->getChipMap(chip);
      }
      if(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
	 (m_plotOpts->m_PLplotType[ps_type]==3 || m_plotOpts->m_PLplotType[ps_type]==5 || m_plotOpts->m_PLplotType[ps_type]==6 || m_plotOpts->m_PLplotType[ps_type]==7))
	splitproj = true;
#endif
    }
  }
  if(tmphi==0) return -2;
  
  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  // flip tmphi for FE-I4 - better cut on nCol,nRow?
  if(tmphi->GetNbinsY()%336==0) flipHistoCont(tmphi);  

  allow_dclick = type==TOT_SUMM || type==TOT_FAST || type==CAL_SUMM || type==CAL_FAST;
  maphi = new MATH2F(*((MATH2F*) tmphi), allow_dclick);
  if(maphi->GetEntries()<=0) maphi->SetEntries(100);
  m_currmap = maphi;
  //if(type==T20K) tmphi->Delete();
  

  if(item->GetDID()!=0 && item->GetDID()->GetLog()!=NULL && item->GetDID()->GetLog()->MyModule->FEflavour==1){
    if(type==FDACP || type==FDACLOG) dacmax = 7;
    else                             dacmax = 127;
  }else{
    if(nFe==16){ // FE-I3
      if(type==FDACP || type==FDACLOG) dacmax = 7;
      else                             dacmax = 127;
    } else { // FE-I4
      if(type==FDACP || type==FDACLOG) dacmax = 15;
      else                             dacmax = 31;
    }
  }

  if(type==FDACP) type = TDACP;     // only needed different ID for getting the map
  if(type==FDACLOG) type = TDACLOG; // only needed different ID for getting the map

  // get projections + scatter

  x = new Float_t[nCol*nRow*nFe];
  y = new Float_t[nCol*nRow*nFe];
  

  for(i=1;i<maphi->GetNbinsX()+1;i++){
    for(j=1;j<maphi->GetNbinsY()+1;j++){
      bin = item->GetDataItem()->PixXYtoInd(i-1,j-1);
      con = maphi->GetBinContent(maphi->GetBin(i,j));
      if(totpeak && item->GetDataItem()->GetMap(-1,TOT_HIT)->GetBinContent(maphi->GetBin(i,j))<m_plotOpts->m_peakMinHit)
        con = 0;
      // fill bad pixel table if required
      if ( badpix!=NULL && type==CHI && con<=0 && m_plotOpts->m_noisetab ) 
        badpix->AddTabLine(i,j,con);
      if ( badpix!=NULL && (type==TDACLOG || type==TDACP) && ((int)con==0 || con==dacmax) 
           && m_plotOpts->m_noisetab ) 
        badpix->AddTabLine(i,j,con);
      // process data for plots
      if(bin<(nCol*nRow*nFe) && bin>=0){
        x[bin] = (Float_t) bin;
        y[bin] = con;
      }
      if(con>mnmax) mnmax = con;
      if(con<mnmin && (con>0 || takeneg)) mnmin = con;
      ntot++;
      if(con>0 || !issfit){
        mnmean += con;
        mnrms  += con*con;
        nfitted++;
      }
      if(type==TDACP || type==TDACLOG){
        if(con>dacmax-1) ntdach++;
        if(con<1)  ntdacl++;
      }
      if(type==LEAK_PLOT || type==MLPLOT) leaktot += con;
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
  
  if(type==TDACP || type==TDACLOG){
    nbins = 1+(int)dacmax;
    mnmin = -.5;
    mnmax = dacmax+.5;
  }

  if(type==MLPLOT || type==LEAK_PLOT){
    mnmax *= 8;
    mnmin *= 8;
    nbins = (int)(mnmax-mnmin);
    mnmax = ((float)mnmax)/8;
    mnmin = ((float)mnmin)/8;
  }

  if(type<DSET && m_plotOpts->m_UsrIUse[type]){
    mnmin = m_plotOpts->m_UsrIMin[type];
    mnmax = m_plotOpts->m_UsrIMax[type];
  }
  if(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
     m_plotOpts->m_PLUsrIUse[ps_type]){
    mnmin = m_plotOpts->m_PLUsrIMin[ps_type];
    mnmax = m_plotOpts->m_PLUsrIMax[ps_type];
  }
  if(m_plotOpts->m_doUsrRg){
    mnmin = m_plotOpts->m_UsrMin;
    mnmax = m_plotOpts->m_UsrMax;
  }

  // prepare the plot

  mapproj[0] = (TH1F*) gROOT->FindObject(m_plotOpts->hinam[type].c_str());	
  if(mapproj[0]!= NULL){
	  mapproj[0]->Delete();
  }
  mapproj[0] = new TH1F(m_plotOpts->hinam[type].c_str(),m_plotOpts->hitit[type].c_str(),nbins,mnmin,mnmax);
  if((splitproj && m_plotOpts->m_splitnsp) || (split_thrp && m_plotOpts->m_splitthr)){
    // prepare the histogram for the long pixel
    sprintf(nam,"%sl",m_plotOpts->hinam[type].c_str());
    if(m_plotOpts->m_PLplotType[ps_type]==6 || m_plotOpts->m_PLplotType[ps_type]==7)
      sprintf(tit,"%s VNCAP",m_plotOpts->hitit[type].c_str());
    else
      sprintf(tit,"%s long",m_plotOpts->hitit[type].c_str());
    mapproj[1] = (TH1F*) gROOT->FindObject(nam);
    if(mapproj[1]!= NULL) mapproj[1]->Delete();
    mapproj[1] = new TH1F(nam,tit,50,mnmin,mnmax);		// long pixel
    // prepare the histogram for the ganged pixel
    sprintf(nam,"%sg",m_plotOpts->hinam[type].c_str());
    sprintf(tit,"%s ganged",m_plotOpts->hitit[type].c_str());
    mapproj[2] = (TH1F*) gROOT->FindObject(nam);
    if(mapproj[2]!= NULL) mapproj[2]->Delete();
    mapproj[2] = new TH1F(nam,tit,30,mnmin,mnmax);		// ganged pixel
    // prepare the histogram for the inter-ganged pixel
    sprintf(nam,"%sbg",m_plotOpts->hinam[type].c_str());
    sprintf(tit,"%s inter-ganged",m_plotOpts->hitit[type].c_str());
    mapproj[3] = (TH1F*) gROOT->FindObject(nam);
    if(mapproj[3]!= NULL) mapproj[3]->Delete();
    mapproj[3] = new TH1F(nam,tit,30,mnmin,mnmax);		// interganged pixel
  }
  for(i=1;i<maphi->GetNbinsX()+1;i++){
    for(j=1;j<maphi->GetNbinsY()+1;j++){
      // ganged-interganged-long sectioning
      con = maphi->GetBinContent(maphi->GetBin(i,j));
      if(totpeak && item->GetDataItem()->GetMap(-1,TOT_HIT)->GetBinContent(maphi->GetBin(i,j))<m_plotOpts->m_peakMinHit)
        con = 0;
      if(con>0 || takeneg){
        if((splitproj && m_plotOpts->m_splitnsp) || (split_thrp && m_plotOpts->m_splitthr)){
	  // plot center, long/VNCAP and ganged pixels separately
	  item->GetDataItem()->PixXYtoCCR(i-1,j-1,&k,&col,&row);
	  std::string ptName = item->GetDataItem()->Pixel_TypeName(item->GetDataItem()->Pixel_Type(col, row));
	  if(ptName=="GANGED")
	    mapproj[2]->Fill(con);
	  else if(ptName=="INTER-GANGED")
	    mapproj[3]->Fill(con);
	  else if(ptName=="VNCAP" && (m_plotOpts->m_PLplotType[ps_type]==6 || m_plotOpts->m_PLplotType[ps_type]==7))
	    mapproj[1]->Fill(con);
	  else if(ptName=="LONG")
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
  if((splitproj && m_plotOpts->m_splitnsp) || (split_thrp && m_plotOpts->m_splitthr)){
    for(i=1;i<4;i++) mapproj[i]->SetMinimum(0.1f);
  }
  
  std::vector<TPad*> pads;
  pads = PadSetup();
  pads[1]->cd();
  bool haveFourPads=false;
  if(m_plotOpts->m_plotfitopt==0){
    if(nFe!=4 && ((splitproj && m_plotOpts->m_splitnsp) || (split_thrp && m_plotOpts->m_splitthr))){ // have ganged pixels and want to plot them
      pads[1]->Divide(1,4,0.001f,0.001f); // divide the Plot window in four parts
      haveFourPads=true;
    } else
      pads[1]->Divide(1,3,0.001f,0.001f); // divide the Plot window in three parts
  }
  if(m_psfile!=NULL) m_psfile->NewPage();
  // Write text on top of the plot
  m_text->Clear();
  if(type==TWLK_TIME20NS_2D || type==TWLK_Q20NS_2D){
    float dt0=20;
    if(item->GetDID()->getAnaPar(1)>-999)
      dt0 = item->GetDID()->getAnaPar(1);
    sprintf(line,"%s, #Delta t=%dns: %s.",m_plotOpts->axtit[type].c_str(),(int)dt0,item->GetDataItem()->GetName());
  }else if(ps_type<0)
    sprintf(line,"%s: %s.",m_plotOpts->axtit[type].c_str(),item->GetDataItem()->GetName());
  else{
    std::string pslabel = item->text(0).latin1();
    int pos = pslabel.find("Plot ");
    if(pos!=(int)std::string::npos)
      pslabel.erase(pos,5);
    sprintf(line,"%s: %s.",pslabel.c_str(),item->GetDataItem()->GetName());
  }
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"", item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  sprintf(line,"%d out of %d pixels with good fit",nfitted, ntot);
  if(issfit) m_text->AddText(line);
  sprintf(line,"%d pixels with DAC=0, %d pixels with DAC=%d",ntdacl, ntdach,(int)dacmax);
  if(item->GetDID()!=0 && item->GetDID()->GetLog()!=NULL && item->GetDID()->GetLog()->MyModule->FEflavour==1)
    sprintf(line,"%s",line);
  if(type==TDACP || type==TDACLOG) m_text->AddText(line);
  leaktot /=1e3;
  sprintf(line,"Sum of current: %.3f#muA",leaktot);
  if(type==LEAK_PLOT || type==MLPLOT) m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();
  pads[1]->cd();

  // Set the plot style
  if(m_plotOpts->m_plotfitopt==0){
    SetStyle(0);
  }
  else{
    SetStyle(1);
  }
  // prepare the map plot
  if(m_plotOpts->m_plotfitopt==0){
    pads[1]->cd(1);
  }
  if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==1){
    //gPad->SetRightMargin(0.1f);
    gPad->SetRightMargin(0.15f);
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    maphi->SetMinimum(mnmin);
    maphi->SetMaximum(mnmax);
    maphi->Draw("COLZ");
    if(m_plotOpts->m_doBlackBg){
      m_canvas->Modified();
      m_canvas->Update();
      ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
    }
    // rename axes to col, row and hide tick labels in case this is
    // a known histogram type or requested for PixLib histograms
    if(!(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
	 (m_plotOpts->m_PLplotType[ps_type]==4 || m_plotOpts->m_PLplotType[ps_type]==5))){
      maphi->GetXaxis()->SetTitle("Column");
      maphi->GetYaxis()->SetTitle("Row");
      if(chip<0) ModuleGrid(maphi, nFe);	  
      //      else       flipAxis(maphi);
    }
    if(m_plotOpts->m_plotfitopt==0){
      TitleResize(1.5);
    }
  }
  // prepare the distribution plot
  if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==2){
    if((splitproj && m_plotOpts->m_splitnsp) || (split_thrp && m_plotOpts->m_splitthr)){
      // Plot center, long/VNCAP and ganged pixels
      TPad *subpad=NULL;
      if(m_plotOpts->m_plotfitopt==2) pads[1]->Divide(2,(nFe==4)?1:2,0.001f,0.001f);
      for(i=0;i<((nFe==4)?2:4);i++){
	if(m_plotOpts->m_plotfitopt==0){
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
	  gPad->SetLogy(m_plotOpts->m_projlogy);
	}
	gPad->SetTopMargin(0.15f);
	gPad->SetBottomMargin(0.15f);
	mapproj[i]->Draw();
	mapproj[i]->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
	/*
	  mapproj[i]->GetXaxis()->SetLabelSize(0.06f);
	  mapproj[i]->GetXaxis()->SetTitleSize(0.06f);
	  mapproj[i]->GetYaxis()->SetLabelSize(0.06f);
	*/
	if(m_plotOpts->m_FitGauss){
	  mapproj[i]->Fit("gaus","Q");
	}
	if(m_plotOpts->m_plotfitopt==0){
	  FitBoxResize((TObject*)mapproj[i],1.3f,.7f,.1f);
	  TitleResize(1.5);
	}
	else{
	  FitBoxResize((TObject*)mapproj[i],.7f,.5f,.1f);
	  TitleResize(.7f);
	}
      }
    }
    else{
      // Plot only center pixel
      pads[1]->cd(2);
      if(mapproj[0]->GetEntries()) gPad->SetLogy(m_plotOpts->m_projlogy);
      gPad->SetTopMargin(0.15f);
      gPad->SetBottomMargin(0.15f);
      mapproj[0]->Draw();
      mapproj[0]->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
      /*
	mapproj[0]->GetXaxis()->SetLabelSize(0.06f);
	mapproj[0]->GetXaxis()->SetTitleSize(0.06f);
	mapproj[0]->GetYaxis()->SetLabelSize(0.06f);
      */
      if(type!=SIG && type !=CHI && type!=CLTOT){
	if(m_plotOpts->m_FitGauss){
	  mapproj[0]->Fit("gaus","Q");
	}
	if(m_plotOpts->m_plotfitopt==0){
	  FitBoxResize((TObject*)mapproj[0],0.7f);
	}
	else{
	  FitBoxResize((TObject*)mapproj[0],0.6f,0.4f);
	}
      }
      else if(type==SIG){
	//gStyle->SetOptStat(1110);
	PlotStat(mapproj[0]);
      }
      if(m_plotOpts->m_plotfitopt==0){
	TitleResize(1.5f);
      }
      else{
	TitleResize(.7f);
      }
    }
  }
  //Prepare the scatter plot
  if(m_plotOpts->m_plotfitopt==0) pads[1]->cd(haveFourPads?4:3);

  if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==3){
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    mapdis->SetMarkerStyle(1);
    mapdis->SetMaximum(mnmax);
    mapdis->SetMinimum(mnmin);
    mapdis->Draw("AP");
    char grtit[500];
    sprintf(grtit,"\"Channel\" = row+%d*column+%d*chip",nRow,nCol*nRow);
    mapdis->GetXaxis()->SetTitle(grtit);
    mapdis->GetYaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
    if(chip<0){
      mapdis->GetXaxis()->SetRangeUser(0,nFe*nCol*nRow);
    }
    else{
      mapdis->GetXaxis()->SetRangeUser(0,nCol*nRow);
    }
    mapdis->SetTitle(m_plotOpts->grtit[type].c_str());
    //if(type==CLTOT) mapdis->GetYaxis()->SetTitleOffset(0.8f);
    if(m_plotOpts->m_plotfitopt==0) TitleResize(1.5);
  }

  m_canvas->Modified();
  m_canvas->Update();

  if ( m_plotOpts->m_noisetab && badpix!=NULL && badpix->nRows() ) badpix->show();
  
  delete[] x;
  delete[] y;

  return 0;
}

int RMain::PlotComp(ModItem *item, int chip, BadPixel *summtab, int act_type, int ps_level, int scanpt1, int scanpt2){

  TH1F *frproj[4];
  //  TH2F *orgmap, *refmap, *frmap;
  TH2F *frmap;
  MATH2F *pltmap;
  TGraph *frdis;
  Float_t *x, *y;
  Char_t line[1024], tit[128],nam[256];
  Int_t i,j,ntot=0, nbad=0, bin, k, row, col, type=item->GetPID();
  Float_t mnmin=1e8, mnmax=-1e8, mnmean=0, mnrms=0, con;
  BadPixel *badpix = NULL;
  // special for merged bump analysis
  TH2F *scnmap=NULL;
  Float_t sccon;
  // needed for time diff.
  Float_t tdiffavg[NCHIP], ndat[NCHIP];
  for(i=0;i<NCHIP;i++){
    tdiffavg[i]=0;
    ndat[i] = 0;
  }

  CheckCan();

  // set up table
  if(type==LOOSE || type==SHORT){
    QString toptitle;
    if(type==LOOSE) toptitle = "List of pixels after unconnected-bumps cut, data set \"";
    else            toptitle = "List of pixels after merged-bumps cut, data set \"";
    toptitle += item->GetDataItem()->GetName();
    toptitle += "\", module ";
    toptitle += item->ModParent()->text(0);
    if(summtab==NULL)
      badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
    else
      badpix = summtab;
    if(type==LOOSE) badpix->Init("noise diff.",toptitle.latin1());
    else            badpix->Init("xtalk ratio",toptitle.latin1());
  }

  // merged bump ana: need noise, too
  if ( type == SHORT ) {
    scnmap = item->GetDataItem()->GetMap(chip,SIG);
    if(scnmap==NULL)  return -2;
  }
  // get processed map
  if(act_type!=NONE){
    frmap = GetComp(item,chip, act_type, ps_level, scanpt1, scanpt2);
    type = act_type;
  }else
    frmap = GetComp(item,chip, ps_level, scanpt1, scanpt2);
  if(frmap==0) return -2;

  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  x = new Float_t[nCol*nRow*nFe];
  y = new Float_t[nCol*nRow*nFe];

  // fill arrays, get min/max and # bad pixels
  for(i=1;i<frmap->GetNbinsX()+1;i++){
    for(j=1;j<frmap->GetNbinsY()+1;j++){
      bin = item->GetDataItem()->PixXYtoInd(i-1,j-1);
      con = frmap->GetBinContent(i,j);
      if(bin<(nCol*nRow*nFe) && bin>=0){
        x[bin] = (Float_t) bin;
        y[bin] = con;
      }
      if(con>mnmax && con!=0) mnmax = con;
      if (type==LOOSE) {
        if(con<mnmin && con>-1000)
          mnmin = con;
        if(con>-1000 && TMath::Abs(con)<m_plotOpts->m_diffcut){
          nbad++;
          if(m_plotOpts->m_noisetab) 
            badpix->AddTabLine(i,j,con);
        } 
      } else if(type==SHORT){
        sccon = scnmap->GetBinContent(i,j);
        if( con>m_plotOpts->m_xtfrcut && con>0 && sccon<m_plotOpts->m_xtnoisecut ){
          if(con<mnmin)
            mnmin = con;
          nbad++;
          if(m_plotOpts->m_noisetab )
            badpix->AddTabLine(i,j,con);
        } else{
          frmap->SetBinContent(i,j,-1000);
          y[bin] = -1000;
        }
      } else if(((type==TDIF || type==MINTDIF || type==NDIF || type==GDIF) && con!=-99999) || 
                ((type==TIMEDIF || type==TCALDIF || type==DELCAL) && con!=0)){
        if((con<mnmin)) mnmin = con;
      }else
        if(con<mnmin && con>0) mnmin = con;
      mnmean += con;
      mnrms  += con*con;
      ntot++;
    }
  }

  //  if(type==SHORT)
  //    scnmap->Delete();
  
  for(i=0;i<NCHIP;i++){
    if(ndat[i]>0)
      tdiffavg[i] /= ndat[i];
  }
  
  if((mnmax==-1000 || mnmin==-1000) && type==LOOSE){
    mnmax = 500;
    mnmin = -500;
  }else if(ntot>0){
    mnmean /= (float) ntot;
    mnrms   = mnrms/(float)ntot - mnmean*mnmean;
    if(mnrms>0 && type!=LOOSE && type !=SHORT){
      mnrms = sqrt(mnrms);
      if(mnmax>mnmean+8*mnrms && type!=LOOSE) mnmax = mnmean+8*mnrms;
      if(mnmin<mnmean-8*mnrms && type!=LOOSE) mnmin = mnmean-8*mnrms;
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
  
  if(type<DSET && m_plotOpts->m_UsrIUse[type]){
    mnmin = m_plotOpts->m_UsrIMin[type];
    mnmax = m_plotOpts->m_UsrIMax[type];
  }
  if(m_plotOpts->m_doUsrRg){
    mnmin = m_plotOpts->m_UsrMin;
    mnmax = m_plotOpts->m_UsrMax;
  }

  frdis = new TGraph(frmap->GetNbinsX()*frmap->GetNbinsY(),x,y);
  m_misc_obj.push_back((TObject*) frdis);

  frproj[0] = (TH1F*) gROOT->FindObject(m_plotOpts->hinam[type].c_str());
  if(frproj[0]!= NULL) frproj[0]->Delete();
  frproj[0] = new TH1F(m_plotOpts->hinam[type].c_str(),m_plotOpts->hitit[type].c_str(),100,mnmin,mnmax);
  if(type==XTFR && m_plotOpts->m_splitnsp){		// crgg ganged-interganged-long sectioning
    sprintf(nam,"%sl",m_plotOpts->hinam[type].c_str());
    sprintf(tit,"%s long",m_plotOpts->hitit[type].c_str());
    frproj[1] = (TH1F*) gROOT->FindObject(nam);
    if(frproj[1]!= NULL) frproj[1]->Delete();
    frproj[1] = new TH1F(nam,tit,50,mnmin,mnmax);
    frproj[1]->SetMinimum(0.1f);				// crgg long pixel
    sprintf(nam,"%sg",m_plotOpts->hinam[type].c_str());
    sprintf(tit,"%s ganged",m_plotOpts->hitit[type].c_str());
    frproj[2] = (TH1F*) gROOT->FindObject(nam);
    if(frproj[2]!= NULL) frproj[2]->Delete();
    frproj[2] = new TH1F(nam,tit,30,mnmin,mnmax);
    frproj[2]->SetMinimum(0.1f);				// crgg ganged pixel should be removed
    sprintf(nam,"%sbg",m_plotOpts->hinam[type].c_str());
    sprintf(tit,"%s inter-ganged",m_plotOpts->hitit[type].c_str());
    frproj[3] = (TH1F*) gROOT->FindObject(nam);
    if(frproj[3]!= NULL) frproj[3]->Delete();
    frproj[3] = new TH1F(nam,tit,30,mnmin,mnmax);
    frproj[3]->SetMinimum(0.1f);				// crgg interganged pixel should be removed
  }
  for(i=1;i<frmap->GetNbinsX()+1;i++){
    for(j=1;j<frmap->GetNbinsY()+1;j++){
      con = frmap->GetBinContent(frmap->GetBin(i,j));
      if(con>0 || type==LOOSE || type==TDIF || type==MINTDIF || type==NDIF || 
         ((type==TIMEDIF || type==TCALDIF || type==DELCAL || type==GDIF) && con!=0)){
        if(type==XTFR && m_plotOpts->m_splitnsp){
	  item->GetDataItem()->PixXYtoCCR(i-1,j-1,&k,&col,&row);
	  std::string ptName = item->GetDataItem()->Pixel_TypeName(item->GetDataItem()->Pixel_Type(col, row));
          if(ptName=="GANGED")
            frproj[2]->Fill(con);
          else if(ptName=="INTER-GANGED")
            frproj[3]->Fill(con);
          else if((ptName=="LONG"))// && m_plotOpts->m_PLplotType[ps_type]==3) || ("VNCAP" && m_plotOpts->m_PLplotType[ps_type]==5))
            frproj[1]->Fill(con);
          else
            frproj[0]->Fill(con);
        }else
          frproj[0]->Fill(con);
      }
    }
  }
  frproj[0]->SetMinimum(0.1f);
  if(type==XTFR && m_plotOpts->m_splitnsp){
    for(i=1;i<4;i++)
      frproj[i]->SetMinimum(0.1f);
  }

  if(summtab==NULL){
    std::vector<TPad*> pads;
    pads = PadSetup();
    pads[1]->cd();
    bool haveFourPads=false;
    if(m_plotOpts->m_plotfitopt==0){
      if(nFe!=4 && type==XTFR && m_plotOpts->m_splitnsp){
        pads[1]->Divide(1,4,0.001f,0.001f);
	haveFourPads=true;
      }else
        pads[1]->Divide(1,3,0.001f,0.001f);
    }
    
    if(m_psfile!=NULL) m_psfile->NewPage();
    
    m_text->Clear();
    if(scanpt2<0)
      sprintf(line,"%s: %s vs %s.",m_plotOpts->axtit[type].c_str(),item->GetDataItem()->GetName(), 
	      item->GetRefItem()->GetName());
    else
      sprintf(line,"%s: %s scan pt. %d vs %d.",m_plotOpts->axtit[type].c_str(),item->GetDataItem()->GetName(), 
	      scanpt1, scanpt2);
    m_text->AddText(line);
    sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
    m_text->AddText(line);
    if(type==LOOSE || type==SHORT){
      sprintf(line,"%d out of %d pixels with bad bumps",nbad, ntot);
      m_text->AddText(line);
    }
    pads[0]->cd();
    m_text->Draw();

    pads[1]->cd();
    
  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
    if(frmap->GetNbinsY()%336==0) flipHistoCont(frmap);
    pltmap = new MATH2F(*((MATH2F*) frmap));
    if(pltmap->GetEntries()<=0) pltmap->SetEntries(100);
    m_currmap = pltmap;
    //frmap->Delete();

    if(m_plotOpts->m_plotfitopt==0)
      SetStyle(0);
    else
      SetStyle(1);
    
    if(m_plotOpts->m_plotfitopt==0) pads[1]->cd(1);
    if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==1){
      gPad->SetRightMargin(0.15f);
      gPad->SetTopMargin(0.15f);
      gPad->SetBottomMargin(0.15f);
      pltmap->SetMinimum(mnmin);
      pltmap->SetMaximum(mnmax);
      pltmap->Draw("COLZ");
    if(m_plotOpts->m_doBlackBg){
      m_canvas->Modified();
      m_canvas->Update();
      ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
    }
    if(chip<0) ModuleGrid(pltmap, nFe);
      pltmap->GetXaxis()->SetTitle("Column");
      pltmap->GetYaxis()->SetTitle("Row");
      TitleResize(1.5);
    }
    
    if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==2){
      if(type==XTFR && m_plotOpts->m_splitnsp){
        TPad *subpad=NULL;
	if(m_plotOpts->m_plotfitopt==2) pads[1]->Divide(2,(nFe==4)?1:2,0.001f,0.001f);
        for(i=0;i<((nFe==4)?2:4);i++){
          if(m_plotOpts->m_plotfitopt==0){
            if(i%2==0){
              if(i<2)
                pads[1]->cd(2);
              else
                pads[1]->cd(3);
              subpad = (TPad*)gPad;
              subpad->Divide(2,1,0.001f,0.001f);
            }
            subpad->cd(i%2+1);
          } else
            pads[1]->cd(i+1);
          if(frproj[i]->GetEntries()) gPad->SetLogy(m_plotOpts->m_projlogy);
          gPad->SetTopMargin(0.15f);
          gPad->SetBottomMargin(0.15f);
          frproj[i]->Draw();
          frproj[i]->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
          /*
            frproj[i]->GetXaxis()->SetLabelSize(0.06f);
            frproj[i]->GetXaxis()->SetTitleSize(0.06f);
            frproj[i]->GetYaxis()->SetLabelSize(0.06f);
          */
          if (type!=LOOSE && type!=SHORT && m_plotOpts->m_FitGauss) frproj[i]->Fit("gaus","Q");
          if(m_plotOpts->m_plotfitopt==0) {
            FitBoxResize((TObject*)frproj[i],1.3f,.7f,.1f);
            TitleResize(1.5f);
          } else{
            FitBoxResize((TObject*)frproj[i],.7f,.5f,.1f);
            TitleResize(.7f);
          }
        }
      } else{
        pads[1]->cd(2);
        gPad->SetTopMargin(0.15f);
        gPad->SetBottomMargin(0.15f);
        if(frproj[0]->GetEntries()) gPad->SetLogy(m_plotOpts->m_projlogy);
        frproj[0]->Draw();
        frproj[0]->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
        if(type>OPTS){
          if (type!=LOOSE && type!=SHORT && m_plotOpts->m_FitGauss) frproj[0]->Fit("gaus","Q");
          if(m_plotOpts->m_plotfitopt==0) FitBoxResize((TObject*)frproj[0],0.7f);
          else                            FitBoxResize((TObject*)frproj[0],0.6f,0.4f);
        }
        if(m_plotOpts->m_plotfitopt==0) TitleResize(1.5f);
        else                            TitleResize(.7f);
      }
    }
    
    if(m_plotOpts->m_plotfitopt==0){
      if(haveFourPads)
        pads[1]->cd(4);
      else
        pads[1]->cd(3);
    }
    if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==3){
      gPad->SetTopMargin(0.15f);
      gPad->SetBottomMargin(0.15f);
      frdis->SetTitle(m_plotOpts->grtit[type].c_str());
      frdis->SetMarkerStyle(1);
      frdis->SetMaximum(mnmax);
      frdis->SetMinimum(mnmin);
      frdis->Draw("AP");
      char grtit[500];
      sprintf(grtit,"\"Channel\" = row+%d*column+%d*chip",nRow,nCol*nRow);
      frdis->GetXaxis()->SetTitle(grtit);
      frdis->GetYaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
      if(chip<0)
        frdis->GetXaxis()->SetRangeUser(0,nFe*nCol*nRow);
      else
        frdis->GetXaxis()->SetRangeUser(0,nCol*nRow);
      frdis->SetTitle(m_plotOpts->grtit[type].c_str());
      TitleResize(1.5);
    }
    
    m_canvas->Modified();
    m_canvas->Update();
    
    if (m_plotOpts->m_noisetab && (type==LOOSE || type==SHORT)) badpix->show();
  }

  delete[] x;
  delete[] y;

  return 0;
}

int RMain::PlotTOT(ModItem *item, int chip){
  
  TH1F *specproj, *scanhi;
  TH2F *tmphi;
  MATH2F *maphi;
  Char_t line[1024];
  Int_t i;
  Int_t type=item->GetPID();


  CheckCan();

  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  tmphi = item->GetDataItem()->GetMap(chip,type);
  if(tmphi==NULL) return -2;

  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
  if(tmphi->GetNbinsY()%336==0) flipHistoCont(tmphi);

  maphi = new MATH2F(*((MATH2F*) tmphi));
  m_currmap = maphi;
  specproj = item->GetDataItem()->GetSpecHi(chip);
  scanhi = item->GetDataItem()->GetToTDist(m_chip, m_col, m_row, !m_plotOpts->m_FitGauss, item->GetRID());

  if(m_plotOpts->m_doUsrRg){
    maphi->SetMinimum(m_plotOpts->m_UsrMin);
    maphi->SetMaximum(m_plotOpts->m_UsrMax);
  }

  std::vector<TPad*> pads;
  pads = PadSetup();
  pads[1]->cd();
  if(scanhi!=NULL)
    pads[1]->Divide(1,3,0.001f,0.001f);
  else
    pads[1]->Divide(1,2,0.001f,0.001f);
  
  if(m_psfile!=NULL) m_psfile->NewPage();
    
  m_text->Clear();
  sprintf(line,"%s: %s.",m_plotOpts->axtit[type].c_str(),item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();

  pads[1]->cd(1);
  gPad->SetRightMargin(0.15f);
  gPad->SetTopMargin(0.15f);
  gPad->SetBottomMargin(0.15f);
  maphi->SetMinimum(0);
  maphi->Draw("COLZ");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  if(chip<0) ModuleGrid(maphi, nFe);
  maphi->GetXaxis()->SetTitle("Column");
  maphi->GetYaxis()->SetTitle("Row");
  TitleResize(1.5);

  pads[1]->cd(2);
  gPad->SetLogy(m_plotOpts->m_projlogy);
  gPad->SetTopMargin(0.15f);
  gPad->SetBottomMargin(0.15f);
  specproj->Draw("histo");
  if(m_plotOpts->m_doUsrRg){
    specproj->GetXaxis()->SetRangeUser(m_plotOpts->m_UsrMin,m_plotOpts->m_UsrMax);
  } else{
    // find last filled bin and resize axis if necessary
    specproj->SetAxisRange(1e4,1e5);
    i = specproj->GetMinimumBin();
    specproj->SetAxisRange(specproj->GetBinCenter(i+1),1e5);
    while(i<300 && (specproj->GetMinimumBin()<=i+1)){
      i++;
      specproj->SetAxisRange(specproj->GetBinCenter(i+1),1e5);
    }
    i = specproj->GetMinimumBin() + 10;
    if(i>300) i=300;
    specproj->SetAxisRange(0,specproj->GetBinCenter(i));
  }
  // fit Gauss to peak
  i = specproj->GetMaximumBin();
  if(m_plotOpts->m_FitGauss)
    specproj->Fit("gaus","Q","",specproj->GetBinCenter(i-5),specproj->GetBinCenter(i+5));
  // labels
  specproj->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
  specproj->GetXaxis()->SetLabelSize(0.06f);
  specproj->GetXaxis()->SetTitleSize(0.06f);
  specproj->GetYaxis()->SetLabelSize(0.06f);
  TitleResize(1.5);
  
  if(scanhi!=NULL){
    pads[1]->cd(3);
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    scanhi->Draw();
    if(m_plotOpts->m_doUsrRg)
      scanhi->GetXaxis()->SetRangeUser((float)m_plotOpts->m_UsrMin,(float)m_plotOpts->m_UsrMax);
    else
      scanhi->GetXaxis()->SetRange();
    scanhi->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
    scanhi->GetXaxis()->SetLabelSize(0.06f);
    scanhi->GetXaxis()->SetTitleSize(0.06f);
    scanhi->GetYaxis()->SetLabelSize(0.06f);
    TitleResize(1.5);
  }

  m_canvas->Modified();
  m_canvas->Update();
  
  return 0;
}
TH2F* RMain::TOT20K(int chip, DatSet *did){
  // JGK: only used for TurboDAQ data, so leave histo size defined with NROW
  int i,j,bin;
  char line[128];
  float A,B,C,tot;
  TH2F *rethi, *orghi[3];
  rethi = (TH2F*) gROOT->FindObject("t20kmap");
  if(rethi!=NULL) rethi->Delete();

  for(i=0;i<3;i++){
    orghi[i] = did->GetMap(chip,PARA+i);
    if(orghi[i]==NULL) return NULL;
  }
  //rethi = (TH2F*) orghi[0]->Clone("t20kmap");
  //if(rethi==NULL) return NULL;
  if(chip<0){
    rethi = new TH2F("t20kmap",orghi[0]->GetTitle(),NCOL*NCHIP/2,-0.5,(NCOL*NCHIP/2)-0.5,2*NROW,-.5,2.*NROW-0.5);
  }
  else{
    rethi = new TH2F("t20kmap",orghi[0]->GetTitle(),NCOL,-0.5,NCHIP-0.5,NROW,-0.5,NROW-0.5);
  }

  sprintf(line,"Map of %s at %dke",m_plotOpts->axtit[T20K].c_str(),m_plotOpts->m_ToTPlotPnt);
  rethi->SetTitle(line);
  for(i=1;i<orghi[0]->GetNbinsX()+1;i++){
    for(j=1;j<orghi[0]->GetNbinsY()+1;j++){
      bin = orghi[0]->GetBin(i,j);
      A   = orghi[0]->GetBinContent(bin);
      B   = orghi[1]->GetBinContent(bin);
      C   = orghi[2]->GetBinContent(bin);
      tot = A+B/((1e3*(float)m_plotOpts->m_ToTPlotPnt)+C);
      rethi->SetBinContent(bin,tot);
    }
  }
  return rethi;
}
int RMain::Plot2DColour(ModItem *item, int chip){
  char line[1024];
  TH2F *tmphi;
  MATH2F *rawmap;
  int nbad=0, ntot=0, type = item->GetPID();
  bool count_bad = (type==MAMASK || type==RMASKP || type==INJCAPH || type==INJCAPL || type==ILEAK || type==HMASKP 
		    || type==SMASKP || type==PMASKP || type==DEAD_CHAN);

  CheckCan();

  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  tmphi = item->GetDataItem()->GetMap(chip,item->GetPID());
  if(tmphi==NULL) return -2;

  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
  if(tmphi->GetNbinsY()%336==0) flipHistoCont(tmphi);

  rawmap = new MATH2F(*((MATH2F*) tmphi));
  if(rawmap->GetEntries()<=0) rawmap->SetEntries(100);
  m_currmap = rawmap;

  std::vector<TPad*> pads;
  pads = PadSetup();
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  if(count_bad){
    for(int binx=1;binx<=rawmap->GetNbinsX();binx++){
      for(int biny=1;biny<=rawmap->GetNbinsY();biny++){
        ntot++;
        float cont = rawmap->GetBinContent(binx,biny);
        if(cont>0)
          nbad++;
      }
    }
  }
    
  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  sprintf(line,"Total of bad pixels: %d",nbad);
  if(count_bad) m_text->AddText(line);
  pads[0]->cd();
  m_text->Draw();

  pads[1]->cd();
  if(rawmap->GetMinimum() > 0.95*rawmap->GetMaximum())
          rawmap->SetMinimum(0.95*rawmap->GetMaximum());
  rawmap->Draw("COLZ");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  gPad->SetRightMargin(0.15f);
  rawmap->SetLabelSize(0.04f,"X");
  rawmap->SetTitleSize(0.04f,"X");
  rawmap->SetLabelSize(0.04f,"Y");
  rawmap->SetTitleSize(0.04f,"Y");
  rawmap->SetTitleOffset(1,"Y");
  if(chip<0) ModuleGrid(rawmap, nFe);
  rawmap->GetXaxis()->SetTitle("Column");
  rawmap->GetYaxis()->SetTitle("Row");

  m_canvas->Modified();
  m_canvas->Update();
  gStyle->SetOptFit(0010);

  return 0;
}

int  RMain::PlotBadSel(ModItem *item, int chip, int bit_mask, bool doAND){
  char line[1024];
  float ylow=0, cont;
  int nbad=0, binx, biny;
  TH2F *tmphi, *mm;
  MATH2F *rawmap;

  CheckCan();

  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  tmphi = item->GetDataItem()->GetMap(chip,MAMASK);
  if(tmphi==NULL) return -2;

  mm = (TH2F*)gROOT->FindObject("maskmask");
  if(mm!=0) mm->Delete();
  mm = GetBadSelMap(tmphi,bit_mask, doAND);

  for(binx=1;binx<=mm->GetNbinsX();binx++){
    for(biny=1;biny<=mm->GetNbinsY();biny++){
      cont = mm->GetBinContent(binx,biny);
      if(cont>0)
        nbad++;
    }
  }

  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
  if(mm->GetNbinsY()%336==0) flipHistoCont(mm);

  rawmap = new MATH2F(*((MATH2F*) mm));
  if(rawmap->GetEntries()<=0) rawmap->SetEntries(100);
  m_currmap = rawmap;

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,ylow,1,.94);
  pplt->Draw();
  
  if(m_psfile!=NULL) m_psfile->NewPage();
    
  m_text->Clear();
  sprintf(line,"%s. Select mask: %d",item->GetDataItem()->GetName(), bit_mask);
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  sprintf(line,"Total of bad pixels: %d",nbad);
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd();
  if(rawmap->GetMinimum() > 0.95*rawmap->GetMaximum())
          rawmap->SetMinimum(0.95*rawmap->GetMaximum());
  rawmap->Draw("COLZ");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  gPad->SetRightMargin(0.15f);
  rawmap->SetLabelSize(0.04f,"X");
  rawmap->SetTitleSize(0.04f,"X");
  rawmap->SetLabelSize(0.04f,"Y");
  rawmap->SetTitleSize(0.04f,"Y");
  rawmap->SetTitleOffset(1,"Y");
  if(chip<0) ModuleGrid(rawmap, nFe);
  rawmap->GetXaxis()->SetTitle("Column");
  rawmap->GetYaxis()->SetTitle("Row");

  m_canvas->Modified();
  m_canvas->Update();
  gStyle->SetOptFit(0010);

  return 0;
}

int RMain::PlotProfile(ModItem *item, int chip)
{// this is to plot profile histograms such as in the timewalk
  char line[1024];
  float ylow = 0, con;
  TProfile *tprof;
  TH2F *maphi;
  int i,j, mychip, col, row, int_type, type = item->GetPID();
  // lets check if the user has closed the canvas
  CheckCan();

  // create profile histo
  std::string name[NPROF], title[NPROF];
  name[0] = "Normal_Pixels";
  name[1] = "Long_Pixels";
  name[2] = "Ganged_Pixels";
  name[3] = "LongG_Pixels";
  name[4] = "All_Pixels";
  title[0] = "OverDrive Normal Pixels";
  title[1] = "OverDrive Long Pixels";
  title[2] = "OverDrive Ganged Pixels";
  title[3] = "OverDrive LongGanged Pixels";
  title[4] = "OverDrive All Pixels";
  int_type = type - TWLK_NORMAL_OVPROF;
  if(int_type==2 || int_type==3)
    tprof = new TProfile(name[int_type].c_str(),title[int_type].c_str(),10,149.5,159.5);
  else
    tprof = new TProfile(name[int_type].c_str(),title[int_type].c_str(),160,-.5,159.5);

  // fill profile
  maphi = item->GetDataItem()->GetMap(chip,TWLK_OVERDRV_2D);
  if(maphi==NULL) return -2;
  for(i=1;i<maphi->GetNbinsX()+1;i++){
    for(j=1;j<maphi->GetNbinsY()+1;j++){
      con = maphi->GetBinContent(maphi->GetBin(i,j));
      item->GetDataItem()->PixXYtoCCR(i-1,j-1,&mychip,&col,&row);      
      if(int_type==4 || int_type== item->GetDataItem()->Pixel_Type(col,row))
        tprof->Fill(row,con);
    }
  }
  
  m_canvas->cd();
  m_canvas->Clear();
  
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,ylow,1,.94);
  pplt->Draw();
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();  
  m_text->Draw();

  pplt->cd();

  tprof->SetLabelSize(0.04f,"X");
  tprof->SetTitleSize(0.04f,"X");
  tprof->SetLabelSize(0.04f,"Y");
  tprof->SetTitleSize(0.04f,"Y");
  tprof->SetLabelColor(1,"X");
  tprof->SetLabelColor(1,"Y");

  tprof->SetTitleOffset(1,"X"); 
  tprof->SetTitleOffset(2,"Y"); 
  tprof->SetNdivisions(510,"X");
  tprof->SetNdivisions(510,"Y");
  gPad->SetLeftMargin(.18f);

  tprof->Draw();
  tprof->GetXaxis()->SetTitle("Row");
  tprof->GetYaxis()->SetTitle("Averaged Overdrive (e)");

  if ((type == TWLK_NORMAL_OVPROF) || (type == TWLK_ALL_OVPROF) || (type == TWLK_LONG_OVPROF)){
    //Bool_t resiz = tprof->GetFunction("pol1")==NULL;
    Bool_t resiz = tprof->GetFunction("onemexp")==NULL;
    //tprof->Fit("pol1","","",0,150);
    m_expo->SetParameter(0,2000);
    m_expo->SetParameter(1,1000);
    m_expo->SetParameter(2,0.1);
    tprof->Fit(m_expo,"R","",0,150);
    if(resiz) FitBoxResize((TObject*)tprof,.8f,.3f,0);
  }

  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}

int  RMain::PlotCorrel(const char* name, ModItem *itema, ModItem *itemb, int pida, int pidb, int scatplt, int scanpt, 
                       int chipID, bool doPlot, int ps_level){
  TH2F *hia=0, *hib=0;
  TGraph *scatter=0;
  TH2F* scathi=0;
  TH1F* rmshi=0;
  PixelDataContainer *dida = itema->GetDataItem(), *didb = itemb->GetDataItem();
  int nBins, ps_pida=-1, ps_pidb=-1;
  bool clear_hia=false, dozeroa=true, dozerob=true;

  // get module geometry
  int nCol, nRow, nFe;
  itema->GetDataItem()->getGeom(nRow, nCol, nFe);
  int nColb, nRowb, nFeb;
  itemb->GetDataItem()->getGeom(nRowb, nColb, nFeb);
  if(nFe!=nFeb || nCol!=nColb || nRow!=nRowb) return -9;
  nBins=nCol*nRow;
  if(chipID<0) nBins *= nFe;

  if(pida>=TWODMEAN && pida<=TWODOPT)
     hia = dida->Get2DMap(chipID,pida,scanpt);
  else if(pida>OPTS && pida<DSET){
    hia = GetComp(itema, chipID,-1,-1,-1);
    hia->SetName("corrain"); // protect from overwriting this
  } else if(pida==TWODMINT || pida==TWODMINTN || pida==TWODMINTC){
    int ind;
    switch(pidb){
    default:
    case TWODMINT:
      ind = 0;
      break;
    case TWODMINTN:
      ind = 1;
      break;
    case TWODMINTC:
      ind = 2;
      break;
    }
    TH2F **maps = AllMapsMinThresh(itema->GetDID(), chipID);
    hia = maps[ind];
    hia->SetName("corrain"); // protect from overwriting this
    delete[] maps;
  } else if (pida<PIXDBTYPES){
    hia = dida->GetMap(chipID,pida);
    if(pida==TDACLOG||pida==FDACLOG){
#ifdef PIXEL_ROD // can link to PixScan
      // temporary histo will be removed later, but PixDBData wants to do so, too
      if((dynamic_cast<PixDBData*>(dida))!=0 && hia!=0) 
	hia = new TH2F(*hia); // so create copy of this one
#endif
      hia->SetName("corrain"); // protect from overwriting this
    }
  } else{
#ifdef PIXEL_ROD // can link to PixScan
    ps_pida = pida-PIXDBTYPES;
    PixDBData *pdbd = dynamic_cast<PixDBData*>(dida);
    if(pdbd!=0){
      hia = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_pida,
						      m_chip, m_col, m_row, scanpt, ps_level));
      if(chipID>=0) hia = pdbd->getChipMap(chipID);
      dozeroa = m_plotOpts->m_PLPltZero[ps_pida];
    }
#endif
  }

  if(pidb>=TWODMEAN && pidb<=TWODOPT)
    hib = didb->Get2DMap(chipID,pidb,scanpt);
  else if(pidb>OPTS && pidb<DSET){
    hib = GetComp(itemb, chipID,-1,-1,-1);
    hib->SetName("corrbin"); // protect from overwriting this
  } else if(pidb==TWODMINT || pidb==TWODMINTN || pidb==TWODMINTC){
    int ind;
    switch(pidb){
    default:
    case TWODMINT:
      ind = 0;
      break;
    case TWODMINTN:
      ind = 1;
      break;
    case TWODMINTC:
      ind = 2;
      break;
    }
    TH2F **maps = AllMapsMinThresh(itemb->GetDID(), chipID);
    hib = maps[ind];
    hib->SetName("corrbin"); // protect from overwriting this
    delete[] maps;
  }else if (pidb<PIXDBTYPES){
    if(pidb==TDACLOG||pidb==FDACLOG){
#ifdef PIXEL_ROD // can link to PixScan
      if((dynamic_cast<PixDBData*>(dida))!=0 && dida==didb && hia!=0){
	// temporary histo would be removed if getting a new one from same data object
	hia = new TH2F(*hia); // so create copy of this one
	clear_hia = true; // remember to clear this later
	hia->SetName("corrain"); // protect from overwriting this
      }
    }
#endif
    hib = didb->GetMap(chipID,pidb);
    if(pidb==TDACLOG||pidb==FDACLOG){
#ifdef PIXEL_ROD // can link to PixScan
      // temporary histo will be removed later, but PixDBData wants to do so, too
      if((dynamic_cast<PixDBData*>(didb))!=0 && hib!=0) 
	hib = new TH2F(*hib); // so create copy of this one
#endif
      hib->SetName("corrbin"); // protect from overwriting this
    }
  } else{
#ifdef PIXEL_ROD // can link to PixScan
     ps_pidb = pidb-PIXDBTYPES;
    PixDBData *pdbd = dynamic_cast<PixDBData*>(didb);
    if(dida==didb && hia!=0){ // temporary histo would be removed if getting a new one from same data object
      hia = new TH2F(*hia); // so create copy of this one
      clear_hia = true; // remember to clear this later
      hia->SetName("corrain"); // protect from overwriting this
    }
    if(pdbd!=0){
      hib = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_pidb,
						      m_chip, m_col, m_row, scanpt, ps_level));
      if(chipID>=0) hib = pdbd->getChipMap(chipID);
      dozerob = m_plotOpts->m_PLPltZero[ps_pidb];
    }
#endif
  }

  if(hia==0 || hib==0) return -2;

  Float_t *x, *y;
  x = new Float_t[nFe*nCol*nRow];
  y = new Float_t[nFe*nCol*nRow];
  Float_t mina=100000, maxa=0, meana=0, rmsa=0;
  Float_t minb=100000, maxb=0, meanb=0, rmsb=0;
  Int_t i,j,bin,ntot=0, nbina=50, nbinb=50;
  char axislabel[300], mname[100], line[1024];

  CheckCan();
  strcpy(mname,name);

  for(i=1;i<hia->GetNbinsX()+1;i++){
    for(j=1;j<hia->GetNbinsY()+1;j++){
      bin = itema->GetDataItem()->PixXYtoInd(i-1,j-1);
      if(bin<(nFe*nCol*nRow) && bin>=0){
        ntot++;
        if(scatplt==2){
          x[bin] = hia->GetBinContent(hia->GetBin(i,j)) - 
            hib->GetBinContent(hib->GetBin(i,j));
          y[bin] = x[bin];
        } else{
          x[bin] = hia->GetBinContent(hia->GetBin(i,j));
          y[bin] = hib->GetBinContent(hib->GetBin(i,j));
        }
        if(x[bin]>maxa) maxa = x[bin];
        if(x[bin]<mina && (dozeroa || x[bin]!=0)) mina = x[bin];
        meana += x[bin];
        rmsa  += x[bin]*x[bin];
        if(y[bin]>maxb) maxb = y[bin];
        if(y[bin]<minb && (dozerob || y[bin]!=0)) minb = y[bin];
        meanb += y[bin];
        rmsb  += y[bin]*y[bin];
     }
    }
  }
  // delete temp histos
  if((pida>OPTS && pida<DSET)||pida==TDACLOG||pida==FDACLOG||clear_hia)
    hia->Delete();
  if((pidb>OPTS && pidb<DSET)||pidb==TDACLOG||pidb==FDACLOG)
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
  DatSet *dsa = dynamic_cast<DatSet*>(dida);
  DatSet *dsb = dynamic_cast<DatSet*>(didb);
  if((pida==TDACP || pida==TDACLOG || pida==FDACP || pida==FDACLOG)
     && dsa!=0 && dsa->GetLog()!=0){
    if(dsa->GetLog()->MyModule->FEflavour==1){
      if(pida==FDACP || pida==FDACLOG) nbina = 7;
      else                             nbina = 127;
    }else{
      nbina = 32;
    }
    mina = -0.5;
    maxa = -0.5+(float)nbina;
  }
  if(dsb!=0 && dsb->GetLog()!=0 && (pidb==TDACP || pidb==TDACLOG || pidb==FDACP || pidb==FDACLOG)){
    if(dsb->GetLog()->MyModule->FEflavour==1){
      if(pidb==FDACP || pidb==FDACLOG) nbinb = 7;
      else                             nbinb = 127;
    }else{
      nbinb = 32;
    }
    minb = -0.5;
    maxb = -0.5+(float)nbinb;
  }
  if(pida<DSET && m_plotOpts->m_UsrIUse[pida]){
    mina = m_plotOpts->m_UsrIMin[pida];
    maxa = m_plotOpts->m_UsrIMax[pida];
  }
  if(ps_pida>=0 && ps_pida<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_pida] &&
     m_plotOpts->m_PLUsrIUse[ps_pida]){
    mina = m_plotOpts->m_PLUsrIMin[ps_pida];
    maxa = m_plotOpts->m_PLUsrIMax[ps_pida];
  }
  if(m_plotOpts->m_doUsrRg){
    mina = m_plotOpts->m_UsrMin;
    maxa = m_plotOpts->m_UsrMax;
  }
  if(pidb<DSET && m_plotOpts->m_UsrIUse[pidb]){
    minb = m_plotOpts->m_UsrIMin[pidb];
    maxb = m_plotOpts->m_UsrIMax[pidb];
  }
  if(ps_pidb>=0 && ps_pidb<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_pidb] &&
     m_plotOpts->m_PLUsrIUse[ps_pidb]){
    minb = m_plotOpts->m_PLUsrIMin[ps_pidb];
    maxb = m_plotOpts->m_PLUsrIMax[ps_pidb];
  }
  if(m_plotOpts->m_doUsrRg){
    minb = m_plotOpts->m_UsrMin;
    maxb = m_plotOpts->m_UsrMax;
  }

  if(scatplt==0){
    scatter = new TGraph(nBins,x,y);
    m_misc_obj.push_back((TObject*) scatter);
    scatter->SetMaximum(maxb);
    scatter->SetMinimum(minb);
  } else if (scatplt==1){
    scathi = (TH2F*) gROOT->FindObject("corrplt");
    if(scathi!=NULL) scathi->Delete();
    scathi = new TH2F("corrplt","Correlation plot",nbina,mina,maxa,nbinb,minb,maxb);
    for(i=0;i<nBins;i++)
      scathi->Fill(x[i],y[i]);
    sprintf(line,"Correlation plot - correl. coeff. = %.4lf",scathi->GetCorrelationFactor());
    scathi->SetTitle(line);
  } else if (scatplt==2){ // difference plot
    rmshi = (TH1F*) gROOT->FindObject("diffplt");
    if(rmshi!=NULL) rmshi->Delete();
    rmshi = new TH1F("diffplt","blabla",100,mina,maxa);
    for(i=0;i<nBins;i++)
      rmshi->Fill(x[i]);
    sprintf(line,"Difference in %s - RMS = %.4lf",m_plotOpts->axtit[pida].c_str(),rmshi->GetRMS());
    rmshi->SetTitle(line);
  } else{
    delete[] x;
    delete[] y;
    return -5;
  }

  delete[] x;
  delete[] y;

  if(!doPlot) return 0;

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Text pad",0,0,1,.94);
  pplt->Draw();
  pplt->cd();

  if(m_psfile!=NULL) m_psfile->NewPage();

  ptxt->cd();
  m_text->Clear();
  std::string  pslabela, pslabelb;
  if(ps_pida<0)
    sprintf(line,"%s: %s dataset  vs ",m_plotOpts->axtit[pida].c_str(),dida->GetName());
  else{
#ifdef PIXEL_ROD // can link to PixScan
    PixScan *ps = new PixScan();
    std::map<std::string, int> hiTypes = ps->getHistoTypes();
    for(std::map<std::string, int>::iterator it = hiTypes.begin(); it!=hiTypes.end(); it++){
      if(it->second==ps_pida)
	pslabela = it->first;
    }
    delete ps;
    sprintf(line,"%s: %s dataset  vs ",pslabela.c_str(),itema->GetDataItem()->GetName());
#endif
  }
  if(ps_pidb<0)
    sprintf(line,"%s%s: %s",line,m_plotOpts->axtit[pidb].c_str(),didb->GetName());
  else{
#ifdef PIXEL_ROD // can link to PixScan
    PixScan *ps = new PixScan();
    std::map<std::string, int> hiTypes = ps->getHistoTypes();
    for(std::map<std::string, int>::iterator it = hiTypes.begin(); it!=hiTypes.end(); it++){
      if(it->second==ps_pidb)
	pslabelb = it->first;
    }
    delete ps;
    sprintf(line,"%s%s: %s",line,pslabelb.c_str(),itemb->GetDataItem()->GetName());
#endif
  }
  if((pida>=TWODMEAN && pida<=TWODOPT)||(pidb>=TWODMEAN && pidb<=TWODOPT))
    sprintf(line,"%s - 2D-scanpt. %d",line,scanpt);
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"", mname);
  if(chipID>=0)
    sprintf(line,"%s - chip %d",line,chipID);
  m_text->AddText(line);
  m_text->Draw();

  pplt->cd();
  gPad->SetLeftMargin(0.15f);
  gPad->SetRightMargin(0.15f);
  if(scatplt==0){
    scatter->SetMarkerStyle(1);
    scatter->Draw("AP");
    scatter->GetXaxis()->SetRangeUser(mina,maxa);
    scatter->SetTitle("Correlation plot");
    // titles and size matters
    if(ps_pida<0)
      sprintf(axislabel,"%s:  %s",dida->GetName(),m_plotOpts->axtit[pida].c_str());
    else
      sprintf(axislabel,"%s:  %s",dida->GetName(),pslabela.c_str());
    scatter->GetXaxis()->SetTitle(axislabel);
    if(ps_pidb<0)
      sprintf(axislabel,"%s:  %s",didb->GetName(),m_plotOpts->axtit[pidb].c_str());
    else
      sprintf(axislabel,"%s:  %s",didb->GetName(),pslabelb.c_str());
    scatter->GetYaxis()->SetTitle(axislabel);
    scatter->GetXaxis()->SetLabelSize(0.04f);
    scatter->GetXaxis()->SetTitleSize(0.04f);
    scatter->GetYaxis()->SetLabelSize(0.04f);
    scatter->GetYaxis()->SetTitleSize(0.04f);
    scatter->GetYaxis()->SetTitleOffset(1.2f);
  } else if(scatplt==1){
    scathi->Draw(m_plotOpts->m_corPlotOpt.c_str());
    // titles and size matters
    if(ps_pida<0)
      sprintf(axislabel,"%s:  %s",dida->GetName(),m_plotOpts->axtit[pida].c_str());
    else
      sprintf(axislabel,"%s:  %s",dida->GetName(),pslabela.c_str());
    scathi->SetXTitle(axislabel);
    if(ps_pidb<0)
      sprintf(axislabel,"%s:  %s",didb->GetName(),m_plotOpts->axtit[pidb].c_str());
    else
      sprintf(axislabel,"%s:  %s",didb->GetName(),pslabelb.c_str());
    scathi->SetYTitle(axislabel);
    scathi->SetLabelSize(0.04f,"X");
    scathi->SetTitleSize(0.04f,"X");
    scathi->SetTitleOffset(1.2f,"X");
    scathi->SetLabelSize(0.04f,"Y");
    scathi->SetTitleSize(0.04f,"Y");
    scathi->SetTitleOffset(1.8f,"Y");
  } else if(scatplt==2){
    rmshi->Draw();
    // titles and size matters
    if(ps_pida<0)
      sprintf(axislabel,"#Delta%s",m_plotOpts->axtit[pida].c_str());
    else
      sprintf(axislabel,"#Delta%s",pslabela.c_str());
    rmshi->SetXTitle(axislabel);
    TitleResize(.7f);
    rmshi->SetLabelSize(0.04f,"X");
    rmshi->SetLabelSize(0.04f,"Y");
    rmshi->SetTitleSize(0.04f,"X");
    rmshi->SetTitleOffset(1.0f,"X");
  } else
    return -5;

  m_canvas->cd();
  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}
int  RMain::Plot2DCorrel(const char* name, DatSet *did, int pid, Bool_t scatplt){
  TH2F *hia;
  TH2F* scathi=NULL;
  TLogFile *log=NULL;
  float xmin, xmax;

  log = did->GetLog();
  if(log==NULL) return -4;
  
  hia = did->Get2DMap(-1,pid,(int)log->MyScan->Outer->Step/2);
  if(hia==NULL) return -2;

  xmin = log->MyScan->Outer->Pts[0];
  xmax = log->MyScan->Outer->Pts[log->MyScan->Outer->Step-1];
  if(xmin>xmax){
    xmin = xmax;
    xmax = log->MyScan->Outer->Pts[0];
  }

  scathi = (TH2F*) gROOT->FindObject("corrplt");
  if(scathi!=NULL) scathi->Delete();
  scathi = new TH2F("corrplt","Correlation plot",log->MyScan->Outer->Step,xmin,
                    xmax,100,hia->GetMinimum(),hia->GetMaximum());

  Float_t con;
  Int_t i,j,k;
  char axislabel[300], mname[100], line[1024];

  CheckCan();
  strcpy(mname,name);

  for(k=0;k<log->MyScan->Outer->Step;k++){
    hia = did->Get2DMap(-1,pid,k);
    if(hia==NULL) return -2;
    for(i=1;i<hia->GetNbinsX()+1;i++){
      for(j=1;j<hia->GetNbinsY()+1;j++){
        con = hia->GetBinContent(hia->GetBin(i,j));
        if(con>0) scathi->Fill(log->MyScan->Outer->Pts[k],con);
      }
    }
  }

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Text pad",0,0,1,.94);
  pplt->Draw();
  pplt->cd();

  if(m_psfile!=NULL) m_psfile->NewPage();

  ptxt->cd();
  m_text->Clear();
  sprintf(line,"%s dataset: %s vs outer scan parameter",did->GetName(),m_plotOpts->axtit[pid].c_str());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"", mname);
  m_text->AddText(line);
  m_text->Draw();

  pplt->cd();
  gPad->SetLeftMargin(0.15f);
  if(scatplt)
    scathi->Draw();
  else
    scathi->Draw(m_plotOpts->m_corPlotOpt.c_str());
  // titles and size matters
  sprintf(axislabel,"%s:  outer scan par.",did->GetName());
  scathi->SetXTitle(axislabel);
  sprintf(axislabel,"%s:  %s",did->GetName(),m_plotOpts->axtit[pid].c_str());
  scathi->SetYTitle(axislabel);
  scathi->SetLabelSize(0.04f,"X");
  scathi->SetTitleSize(0.04f,"X");
  scathi->SetTitleOffset(1.8f,"X");
  scathi->SetLabelSize(0.04f,"Y");
  scathi->SetTitleSize(0.04f,"Y");
  scathi->SetTitleOffset(1.8f,"Y");

  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}
/*
int RMain::PlotGraph(ModItem *item){
  ModItem *itarr[1] = {item};
  return PlotGraph(itarr,1);
}
*/
int RMain::PlotGraph(ModItem **item, int ngraphs){
  TGraph *indgraph;
  char line[1024];
  Int_t type = item[0]->GetPID(), i;
  //if(type>DSET) type = IVCORR;
  Double_t x,y;

  CheckCan();

  if(ngraphs<1) return -2;

  bool pdb = false;
#ifdef PIXEL_ROD // can link to PixScan
  PixDBData *pdbd = dynamic_cast<PixDBData*>(item[0]->GetDataItem());
  pdb = pdbd!=0;
#endif

  TMultiGraph *mg = new TMultiGraph();
  // m_misc_obj.push_back((TObject*) mg);
  TLegend *leg = NULL;
  if(ngraphs>1) leg = new TLegend(.5f,.95f-.05f*(float)ngraphs,.99f,.99f);
  // if(ngraphs>1) leg = new TLegend(.5f,.75f,.99f,.99f);
  for(i=0;i<ngraphs;i++){
    sprintf(line,"tmpgr%d",i);
    indgraph = (TGraph*) gROOT->FindObject(line);
    if(indgraph!=NULL) indgraph->Delete();
    if(type==IVCORR)
      indgraph = item[i]->GetDID()->GetCorrIV();
    else
      indgraph = item[i]->GetDataItem()->GetGraph();
    if(indgraph==NULL) return -2;
    //indgraph = (TGraph*) indgraph->Clone(line);
    indgraph->SetMarkerStyle(20);
    indgraph->SetMarkerColor(1+i);
    indgraph->SetLineColor(1+i);
    indgraph->SetMarkerSize(0.5f);
    // check first point for IV-curve, sometimes rubbish (ie I<0)
    indgraph->GetPoint(0,x,y);
    if(y<0 && item[i]->GetDID()!=0 && item[i]->GetDID()->m_istype==IVSCAN)
      indgraph->RemovePoint(0);
    mg->Add(indgraph);
    if(leg!=NULL) leg->AddEntry(indgraph,item[i]->GetDataItem()->GetName(),"p");
  }
  //sprintf(line,"%s for module %s",m_plotOpts->grtit[type].c_str(),item[0]->ModParent()->text(0).latin1());
  //mg->SetTitle(line);
  mg->SetTitle("");

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  
  m_text->Clear();
  sprintf(line," ");
  for(i=0;i<ngraphs;i++){
    if(i<ngraphs-1)
      sprintf(line,"%s%s + ",line,item[i]->GetDataItem()->GetName());
    else
      sprintf(line,"%s%s.",line,item[i]->GetDataItem()->GetName());
  }
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item[0]->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  if(m_psfile!=NULL) m_psfile->NewPage();

  pplt->cd();
  gPad->SetRightMargin(0.15f);
  mg->Draw("APL");
  if(pdb==0 || indgraph==0)
    mg->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
  else 
    mg->GetXaxis()->SetTitle(indgraph->GetXaxis()->GetTitle());
  mg->GetXaxis()->SetTitleSize(.03f);
  mg->GetXaxis()->SetLabelSize(.03f);
  if(pdb==0 || indgraph==0)
    mg->GetYaxis()->SetTitle(m_plotOpts->hitit[type].c_str());
  else 
    mg->GetYaxis()->SetTitle(indgraph->GetYaxis()->GetTitle());
  mg->GetYaxis()->SetTitleOffset(1.2f);
  mg->GetYaxis()->SetTitleSize(.03f);
  mg->GetYaxis()->SetLabelSize(.03f);
  if(leg!=NULL) leg->Draw();
  m_canvas->Update();
  m_canvas->Modified();

  return 0;
}
int RMain::PlotGraph(ModItem *item, int index, int scan_level){
  TGraph *indgraph=0;
  char line[1024];
  Int_t type = item->GetPID(), dat_type = 0, i;
  if(item->GetDID()!=0) dat_type = item->GetDID()->m_istype;
  if(type>DSET) type = IVPLOT;
  Double_t x,y;
  int ngraphs=1;
  if(item->GetDID()!=0){
    for(i=0;i<=NGRAPH;i++)
      if(item->GetDID()->GetGraph(i)==NULL) break;
    ngraphs = i;
  }

  CheckCan();

  if(ngraphs<1) return -2;

  if(dat_type==IVSCAN || dat_type==SENSORIV) ngraphs = 1; // hiding temperature stuff in graph#1

  TMultiGraph *mg = new TMultiGraph();
  //  m_misc_obj.push_back((TObject*) mg);
  TLegend *leg = NULL;
  if(ngraphs>1) leg = new TLegend(.5f,.95f-.05f*(float)ngraphs,.99f,.99f);
  for(i=0;i<ngraphs;i++){
    sprintf(line,"tmpgr%d",i);
    indgraph = (TGraph*) gROOT->FindObject(line);
    if(indgraph!=NULL) indgraph->Delete();
    if(type==IVCORR && item->GetDID()!=0)
      indgraph = item->GetDID()->GetCorrIV();
    else{
      if(scan_level>=0 && index>=0)
	indgraph = item->GetDataItem()->GetGraph(index, scan_level);
      else
	indgraph = item->GetDataItem()->GetGraph(i, 0);
    }
    if(indgraph==0) return -2;
    //indgraph = (TGraph*) indgraph->Clone(line);
    indgraph->SetMarkerStyle(20);
    indgraph->SetMarkerColor(1+i);
    indgraph->SetLineColor(1+i);
    indgraph->SetMarkerSize(0.5f);
    // check first point for IV-curve, sometimes rubbish (ie I<0)
    indgraph->GetPoint(0,x,y);
    if(y<0 && item->GetDID()!=0 && item->GetDID()->m_istype==IVSCAN)
      indgraph->RemovePoint(0);
    mg->Add(indgraph);
    if(leg!=NULL){
      if(item->GetDID()!=0 && item->GetDID()->m_istype==IVSCAN)
        leg->AddEntry(indgraph,item->text(0).latin1(),"p");
      else
        leg->AddEntry(indgraph,indgraph->GetTitle(),"p");
    }
  }
  //sprintf(line,"%s for module %s",m_plotOpts->grtit[type].c_str(),item->ModParent()->text(0).latin1());
  if(ngraphs>1 || indgraph==0)
    mg->SetTitle("");
  else
    mg->SetTitle(indgraph->GetTitle());

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  
  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  if(m_psfile!=NULL) m_psfile->NewPage();

  pplt->cd();
  gPad->SetRightMargin(0.15f);
  mg->Draw("APL");
  //mg->GetXaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
  if(indgraph!=0) mg->GetXaxis()->SetTitle(indgraph->GetXaxis()->GetTitle());
  mg->GetXaxis()->SetTitleSize(.03f);
  mg->GetXaxis()->SetLabelSize(.03f);
  //mg->GetYaxis()->SetTitle(m_plotOpts->hitit[type].c_str());
  if(indgraph!=0) mg->GetYaxis()->SetTitle(indgraph->GetYaxis()->GetTitle());
  mg->GetYaxis()->SetTitleOffset(1.2f);
  mg->GetYaxis()->SetTitleSize(.03f);
  mg->GetYaxis()->SetLabelSize(.03f);
  if(leg!=NULL) leg->Draw();
  m_canvas->Update();
  m_canvas->Modified();

  return 0;
}
int RMain::TwalkGraphs(ModItem *item, int chip){  //DatSet **rawdat){
  TGraph **tgr = new TGraph*[2];
  TH2F *tmphi;
  MATH2F *rawmap;
  DatSet **refArr = item->GetRIDArr();
  int i;
  float thresh, Qt20, t0_20ns;
  int binx, biny;

  CheckCan();

  // get module geometry
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  // check data pointers
  if(refArr==0) return -3;
  for(i=0;i<3;i++)
    if(refArr[i]==0) return -3;

  tmphi = item->GetDataItem()->GetMap(-1,TWLK_Q20NS_2D);
  if(tmphi==NULL) return -2;
  item->GetDataItem()->PixCCRtoBins(m_chip,m_col,m_row, binx, biny);
  Qt20 = tmphi->GetBinContent(binx,biny);

  tmphi = item->GetDataItem()->GetMap(-1,TWLK_TIME20NS_2D);
  if(tmphi==NULL) return -2;
  item->GetDataItem()->PixCCRtoBins(m_chip,m_col,m_row, binx, biny);
  t0_20ns = tmphi->GetBinContent(binx,biny);

  tmphi = refArr[0]->GetMap(-1,MEAN);
  if(tmphi==NULL) return -2;
  item->GetDataItem()->PixCCRtoBins(m_chip,m_col,m_row, binx, biny);
  thresh = tmphi->GetBinContent(binx,biny);

  tmphi = item->GetDataItem()->GetMap(chip,TWLK_OVERDRV_2D);
  if(tmphi==NULL) return -2;
  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
  if(tmphi->GetNbinsY()%336==0) flipHistoCont(tmphi);
  rawmap = new MATH2F(*((MATH2F*) tmphi));
  if(rawmap->GetEntries()<=0) rawmap->SetEntries(100);
  m_currmap = rawmap;

  int low_npoints  = refArr[1]->GetLog()->MyScan->Outer->Step;
  int high_npoints = refArr[2]->GetLog()->MyScan->Outer->Step;
  
  int point;
  float time,charge,xpos,ypos;
  char name[100], title[200], line[1024];
  
  sprintf(name,"chigh_%d_%d_%d",m_chip,m_col,m_row);
  sprintf(title,"LowCap TimeWalk %d %d %d",m_chip,m_col,m_row);
  tgr[0] = new TGraph();
  tgr[0]->SetName(name);
  tgr[0]->SetTitle(title);
  m_misc_obj.push_back((TObject*) tgr[0]);

  sprintf(name,"clow_%d_%d_%d",m_chip,m_col,m_row);
  sprintf(title,"HighCap TimeWalk %d %d %d",m_chip,m_col,m_row);
  tgr[1] = new TGraph();
  tgr[1]->SetName(name);
  tgr[1]->SetTitle(title);
  m_misc_obj.push_back((TObject*) tgr[1]);
  
  item->GetDataItem()->PixXY(m_chip,m_col,m_row,&xpos,&ypos);
  item->GetDataItem()->PixXYtoBins(xpos,ypos,binx,biny);
  
  for (point=0;point<low_npoints;point++){
    // so lets get every point from the low cap time walk
    charge = refArr[1]->GetLog()->MyModule->Chips[m_chip]->CalVcal((double)
                    refArr[1]->GetLog()->MyScan->Outer->Pts[point], false);//*charge_factor_low;
    time = refArr[1]->Get2DMap(-1, TWODMEAN, point)->GetBinContent(binx,biny);
    tgr[0]->SetPoint(point,charge,time);
  }
  
  
  for (point=0;point<high_npoints;point++){
    // so lets get every point from the low cap time walk
    charge = refArr[2]->GetLog()->MyModule->Chips[m_chip]->CalVcal((double)
                     refArr[2]->GetLog()->MyScan->Outer->Pts[point],true);//*charge_factor_high;
    time = refArr[2]->Get2DMap(-1, TWODMEAN, point)->GetBinContent(binx,biny)
          // correct for potential differences in trigger delay:
            + 25*(refArr[1]->GetLog()->MyScan->TrgDelay
                -refArr[2]->GetLog()->MyScan->TrgDelay);
    tgr[1]->SetPoint(point,charge,time);
  }  

  TMultiGraph *all_graphs;
  all_graphs = (TMultiGraph*) gROOT->FindObject("allgr");
  if(all_graphs!=NULL) all_graphs->Delete();
  sprintf(line,"%s for chip %d col %d row %d",m_plotOpts->grtit[TWLKGR].c_str(),m_chip,m_col,m_row);
  all_graphs = new TMultiGraph("allgr",line);
  //  m_misc_obj.push_back((TObject*) all_graphs);

  TLegend *leg = new TLegend(.7f,.75f,.99f,.99f);
  for (i=0;i<2;i++){
    tgr[i]->SetMarkerStyle(20);
    tgr[i]->SetMarkerColor(i+1);
    tgr[i]->SetMarkerSize(0.7f);
    all_graphs->Add(tgr[i]);
    if(i) leg->AddEntry(tgr[i],"Chigh","P");
    else  leg->AddEntry(tgr[i],"Clow","P");
  }

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *praw = new TPad("pplt","Plot pad",0,.65,1,.94);
  praw->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.65);
  pplt->Draw();

  if(m_psfile!=NULL) m_psfile->NewPage();

  ptxt->cd();
  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  if(item->GetDID()->getAnaPar(0)>-999){
    sprintf(line,"Q(t0) = %de",(int)item->GetDID()->getAnaPar(0));
    m_text->AddText(line);
  }
  ptxt->cd();
  m_text->Draw();

  praw->cd();
  SetStyle(0);
  rawmap->Draw("colz");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  if(chip<0) ModuleGrid(rawmap, nFe);
  rawmap->GetXaxis()->SetTitle("Column");
  rawmap->GetYaxis()->SetTitle("Row");
  TitleResize(1.5);

  pplt->cd();
  gPad->SetRightMargin(0.15f);
  gPad->SetLogx(m_plotOpts->m_projlogy);
  all_graphs->Draw("APL");
  all_graphs->GetXaxis()->SetTitle(m_plotOpts->axtit[TWLKGR].c_str());
  all_graphs->GetXaxis()->SetTitleSize(.03f);
  all_graphs->GetXaxis()->SetLabelSize(.03f);
  all_graphs->GetYaxis()->SetTitle(m_plotOpts->hitit[TWLKGR].c_str());
  all_graphs->GetYaxis()->SetTitleOffset(1.2f);
  all_graphs->GetYaxis()->SetTitleSize(.03f);
  all_graphs->GetYaxis()->SetLabelSize(.03f);

  float xmin, xmax,dt0=20;
  if(item->GetDID()->getAnaPar(1)>-999)
    dt0 = item->GetDID()->getAnaPar(1);
  if(m_plotOpts->m_projlogy)
    xmin = TMath::Log10(Qt20);
  else
    xmin = Qt20;
  xmax = xmin;
  TLine *qline = new TLine(xmin,all_graphs->GetYaxis()->GetXmin(),xmax,all_graphs->GetYaxis()->GetXmax());
  qline->SetLineColor(3);
  if(m_plotOpts->m_projlogy)
    xmin = TMath::Log10(thresh);
  else
    xmin = thresh;
  xmax = xmin;
  TLine *thline = new TLine(xmin,all_graphs->GetYaxis()->GetXmin(),xmax,all_graphs->GetYaxis()->GetXmax());
  thline->SetLineColor(2);
  if(m_plotOpts->m_projlogy){
    xmin = TMath::Log10(all_graphs->GetXaxis()->GetXmin());
    xmax = TMath::Log10(all_graphs->GetXaxis()->GetXmax());
  }else{
    xmin = all_graphs->GetXaxis()->GetXmin();
    xmax = all_graphs->GetXaxis()->GetXmax();
  }
  TLine *t2line = new TLine(xmin,t0_20ns,xmax,t0_20ns);
  t2line->SetLineColor(4);
  
  TLine *t0line = new TLine(xmin,t0_20ns+dt0,xmax,t0_20ns+dt0);
  t0line->SetLineColor(5);

  t0line->Draw();
  leg->AddEntry(t0line,"t_{0}","l");
  t2line->Draw();
  sprintf(line,"t_{0}-%dns",(int)dt0);
  leg->AddEntry(t2line,line,"l");
  qline->Draw();
  sprintf(line,"Q(t_{0}-%dns)",(int)dt0);
  leg->AddEntry(qline,line,"l");
  thline->Draw();
  leg->AddEntry(thline,"Threshold","l");

  leg->Draw();

  m_canvas->Update();
  m_canvas->Modified();
  return 0;
}
int RMain::DigExcMiss(ModItem *item, int in_chip, int plt_type, float vmin, TH2F *mask, bool merge){
  // JGK: only used for TurboDAQ data, so leave histo size defined with NROW

  char line[2001];
  TH2F *vve=NULL, *vvm=NULL, *cnte, *cntm, *mymask;
  TH1F *vve1=NULL, *vvm1=NULL;
  TGraphErrors *gr;
  TObject *oldobj;
  int chipmin=in_chip, chipmax=in_chip+1;
  if(in_chip<0){
    chipmin = 0;
    chipmax = NCHIP;
  }

  CheckCan();

  if(item->GetDID()==NULL) return -2;
  if(item->GetDID()->GetLog()==NULL) return -4;

  bool loadmask=false;
  mymask = item->GetDataItem()->GetMap(-1,MAMASK);
  if(mymask==0){
    loadmask = true;
    sprintf(line,"mask%ld",(long int)item->GetDataItem());
    mymask = new TH2F(line,("Mask of pixels bad for all "+item->GetDID()->GetLog()->MyScan->GetInner()).c_str(),
                      NCOL*NCHIP/2,-.5,NCOL*NCHIP/2-.5,2*NROW,-.5,2*NROW-.5);
  }

  bool notmasked=true, onedim = item->GetDID()->GetLog()->MyScan->GetOuter()=="No Scan";
  float dscan;
  float mccmin = (float)item->GetDID()->GetLog()->MyScan->Inner->Start;
  float mccmax = (float)item->GetDID()->GetLog()->MyScan->Inner->Stop;
  int   nmcc   = item->GetDID()->GetLog()->MyScan->Inner->Step;
  dscan = (mccmax-mccmin)/(float)(nmcc-1)/2;
  mccmin -= dscan;
  mccmax += dscan;
  float femin, femax;
  int nfe;
  if(onedim){
    femin = 1;
    femax = 1;
    nfe   = 1;
  } else{
    femin  = (float)item->GetDID()->GetLog()->MyScan->Outer->Start;
    femax  = (float)item->GetDID()->GetLog()->MyScan->Outer->Stop;
    nfe    = item->GetDID()->GetLog()->MyScan->Outer->Step;
    dscan = (femax-femin)/(float)(nfe-1)/2;
    femin -= dscan;
    femax += dscan;
  }
  Int_t chip, col, row, binx, biny, scanpt, scanmin, outpt, outmin, innpt;
  Float_t weight, xval, yval;
  Double_t *xpt, *ypt;
  std::string vtitle, ctitle;
  if(plt_type==0){
    vtitle = "Count of Pixels with ";
    ctitle = "Count of Events with ";
  }else if(plt_type==1){
    vtitle = "Count of ";
    ctitle = "Count of ";
  }else
    return -5;

  CheckCan();
  
  // remove old histos to avoid memory leaks
  oldobj = gROOT->FindObject("digem-1");
  if(oldobj!=NULL) oldobj->Delete();
  oldobj = gROOT->FindObject("digem-2");
  if(oldobj!=NULL) oldobj->Delete();
  oldobj = gROOT->FindObject("digem-3");
  if(oldobj!=NULL) oldobj->Delete();
  oldobj = gROOT->FindObject("digem-4");
  if(oldobj!=NULL) oldobj->Delete();
  // create 1D or 2D histos
  if (onedim){
    vvm1  = new TH1F("digem-1",(vtitle+"Missing Hits").c_str(), nmcc, mccmin, mccmax);
    vve1  = new TH1F("digem-2",(vtitle+"Excess Hits").c_str(), nmcc, mccmin, mccmax);
    vvm1->SetXTitle(item->GetDID()->GetLog()->MyScan->GetInner().c_str());
    vve1->SetXTitle(item->GetDID()->GetLog()->MyScan->GetInner().c_str());
  }else{
    vvm  = new TH2F("digem-1",(vtitle+"Missing Hits").c_str(), nfe, femin, femax, nmcc, mccmin, mccmax);
    vve  = new TH2F("digem-2",(vtitle+"Excess Hits").c_str(), nfe, femin, femax, nmcc, mccmin, mccmax);
    vvm->SetXTitle("FE VDD");
    vve->SetXTitle("FE VDD");
    vvm->SetYTitle("MCC VDD");
    vve->SetYTitle("MCC VDD");
  }
  if(in_chip<0){
    cntm = new TH2F("digem-3",(ctitle+"Missing Hits").c_str(), NCOL*NCHIP/2,-.5,NCOL*NCHIP/2-.5,
                    2*NROW,-.5,2*NROW-.5);		// crgg nice
    cnte = new TH2F("digem-4",(ctitle+"Excess Hits").c_str(), NCOL*NCHIP/2,-.5,NCOL*NCHIP/2-.5,
                    2*NROW,-.5,2*NROW-.5);		// crgg nice
  } else{
    sprintf(line,"%sMissing Hits chip %d",ctitle.c_str(),in_chip);
    cntm = new TH2F("digem-3",line,NCOL,-.5,NCOL-.5,NROW,-.5,NROW-.5);		// crgg nice
    sprintf(line,"%sExcess Hits chip %d",ctitle.c_str(),in_chip);
    cnte = new TH2F("digem-4",line,NCOL,-.5,NCOL-.5,NROW,-.5,NROW-.5);		// crgg nice
  }
  // load data
  int minvdd=0;
  for(chip=chipmin;chip<chipmax;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        bool minfound=false;
        item->GetDataItem()->PixCCRtoBins(chip,col,row,binx,biny);
        item->GetDataItem()->PixXY(chip,col,row,&xval,&yval);
        gr = item->GetDataItem()->GetScanHi(chip,col,row,RAW);
        notmasked = true;
        if(mask!=NULL)
          notmasked = (bool)!mask->GetBinContent(binx,biny);
        if(gr!=0 && loadmask){
          int ymin, ymax;
          scanpt =  TMath::LocMax(gr->GetN(),gr->GetY());
          ymax = (int) gr->GetY()[scanpt];
          scanpt =  TMath::LocMin(gr->GetN(),gr->GetY());
          ymin = (int) gr->GetY()[scanpt];
          if((ymin<item->GetDID()->GetLog()->MyScan->Nevents
              && ymax<item->GetDID()->GetLog()->MyScan->Nevents) ||
             (ymin>item->GetDID()->GetLog()->MyScan->Nevents
              && ymax>item->GetDID()->GetLog()->MyScan->Nevents)){
            //            printf("bad pixel %d %d %d with min/max: %d %d\n",chip,col,row,ymin,ymax);
            if(mask==0 || merge) notmasked=false;
            mymask->SetBinContent(binx,biny,1);
          }
        }else if(mymask!=0 && (mask==0 || merge))
          notmasked &= (bool)!mymask->GetBinContent(binx,biny);
        if(in_chip>=0){
          item->GetDataItem()->PixCCRtoBins(0,col,row,binx,biny);
          item->GetDataItem()->PixXY(0,col,row,&xval, &yval);
        }
        if(gr!=NULL && notmasked){
          xpt = gr->GetX();
          ypt = gr->GetY();
          scanmin = 0 ;
          outmin = 0;
          if(vmin>0)
            scanmin = TMath::BinarySearch(nmcc,item->GetDID()->GetLog()->MyScan->Inner->Pts,(int)vmin);
          if(vmin>0 && !onedim)
            outmin = TMath::BinarySearch(nmcc,item->GetDID()->GetLog()->MyScan->Outer->Pts,(int)vmin);
          for(innpt = scanmin; innpt<nmcc; innpt++){
            for(outpt = outmin; outpt<nfe; outpt++){
              scanpt = innpt + nmcc*outpt;
              if(scanpt < gr->GetN()){
                weight = 1;
                if(plt_type==1)
                  weight = TMath::Abs(ypt[scanpt]-item->GetDID()->GetLog()->MyScan->Nevents);
                if(weight==0 && !minfound && (int)xpt[scanpt]>=minvdd){
                  minvdd = (int)xpt[scanpt];
                  minfound = true;
                }else if(weight!=0 && minfound && (int)xpt[scanpt]>=minvdd){
                  minvdd = (int)xpt[scanpt];
                  minfound = false;
                }
                if(((int)ypt[scanpt])<item->GetDID()->GetLog()->MyScan->Nevents){
                  if(onedim)
                    vvm1->Fill((Float_t)xpt[scanpt], weight);
                  else
                    vvm->Fill((Float_t)xpt[scanpt], (Float_t)xpt[scanpt], weight);
                  cntm->Fill(xval, yval, weight);
                }
                if(((int)ypt[scanpt])>item->GetDID()->GetLog()->MyScan->Nevents){
                  if(onedim)
                    vve1->Fill((Float_t)xpt[scanpt], weight);
                  else
                    vve->Fill((Float_t)xpt[scanpt], (Float_t)xpt[scanpt], weight);
                  cnte->Fill(xval, yval, weight);
                }
              }
            }
          }
        }
      }
    }
  }

  // upload mask if it had to be created here
  if(loadmask)
    item->GetDID()->CreateMask(mymask,1);

  // plotting
  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  if(vmin>0)
    pplt->Divide(1,2,0.001f,0.001f);
  else
    pplt->Divide(2,2,0.001f,0.001f);
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s. Deviation from nominal %d hits.",item->GetDataItem()->GetName(), 
	  item->GetDataItem()->getNevents());
  if(vmin>0)
    sprintf(line,"%s (%s>%d)",line,item->GetDID()->GetLog()->MyScan->GetInner().c_str(),(int)vmin);
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  sprintf(line,"Min. %s = %d",item->GetDID()->GetLog()->MyScan->GetInner().c_str(), minvdd);
  if(plt_type==1)
    m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd(1);
  SetStyle(2,cnte);
  gPad->SetLogz(m_plotOpts->m_projlogy);
  if(m_plotOpts->m_projlogy){
    cnte->SetMinimum(0.1f);
    if(cnte->GetMaximum()<.1f)
      cnte->SetMaximum(1);
  }
  cnte->Draw("COLZ");
  if(in_chip<0) ModuleGrid(cnte,NCHIP);
  cnte->GetXaxis()->SetTitle("Column");
  cnte->GetYaxis()->SetTitle("Row");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }

  pplt->cd(2);
  SetStyle(2,cntm);
  gPad->SetLogz(m_plotOpts->m_projlogy);
  if(m_plotOpts->m_projlogy){
    cntm->SetMinimum(0.1f);
    if(cntm->GetMaximum()<.1f)
      cntm->SetMaximum(1);
  }
  cntm->Draw("COLZ");
  if(in_chip<0) ModuleGrid(cntm, NCHIP);
  cntm->GetXaxis()->SetTitle("Column");
  cntm->GetYaxis()->SetTitle("Row");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  
  if(vmin<=0){
    pplt->cd(3);
    gPad->SetLogy(m_plotOpts->m_projlogy);
    if(onedim){
      SetStyle(3,vve1);
      vve1->Draw();
      vve1->GetXaxis()->SetNdivisions(505);
      if(m_plotOpts->m_projlogy){
        vve1->SetMinimum(0.1f);
        if(vve1->GetMaximum()<0.1f)
          vve1->SetMaximum(1);
      }else
        vve1->SetMinimum(0);
    }else{
      SetStyle(3,vve);
      vve->Draw("COLZ");
    }
    
    pplt->cd(4);
    gPad->SetLogy(m_plotOpts->m_projlogy);
    if(onedim){
      SetStyle(3,vvm1);
      vvm1->Draw();
      vvm1->GetXaxis()->SetNdivisions(505);
      if(m_plotOpts->m_projlogy){
        vvm1->SetMinimum(0.1f);
        if(vvm1->GetMaximum()<0.1f)
          vvm1->SetMaximum(1);
      } else
        vvm1->SetMinimum(0);
    }else{
      SetStyle(3,vvm);
      vvm->Draw("COLZ");
    }
  }

  m_canvas->Update();
  m_canvas->Modified();
  return 0;
}

// fit functions
double RMain::ErrorFunc(double *x, double *par){
  double QfromVcal = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[0]*TMath::Erf((QfromVcal-par[1])/sqrt(2.)/par[2]);
  else
    return 0.;
}

double RMain::ComErFunc(double *x, double *par){
  double QfromVcal = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[2]*TMath::Erfc((par[0]-QfromVcal)/sqrt(2.)/par[1]);
  else
    return 0.;
}

double RMain::TwoErFunc(double *x, double *par){
  double QfromVcal = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[0]*(TMath::Erf((par[1]-QfromVcal)/sqrt(2.)/par[2])
                       -TMath::Erf((QfromVcal-par[1])/sqrt(2.)/par[2]));
  else
    return 0.;
}
double RMain::TOTFunc(double *x, double *par){
  double denom = par[4]+x[0]*par[3]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0]  + par[2];
  if(denom!=0)
    return par[0]+par[1]/denom;
  else
    return 0;
}
// graphics stuff
void RMain::CheckCan(){
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

void RMain::PlotStat(TH1F* hi){
  float x1, x2, y1, y2, dx, dy;
  char txt[256];
  if(hi==NULL) return;
  x2 = gPad->GetX2();
  y2 = gPad->GetY2();
  dx = x2-gPad->GetX1();
  dy = y2-gPad->GetY1();
  x1 = x2-0.3*dx;
  y1 = y2-0.25*dy;
  x2 -= 0.04*dx;
  y2 -= 0.04*dy;
  TPaveText *tx = new TPaveText(x1,y1,x2,y2,"NDC");
  sprintf(txt,"Mean   %4.0f",hi->GetMean());
  tx->AddText(txt);
  sprintf(txt,"RMS    %4.0f",hi->GetRMS());
  tx->AddText(txt);
  tx->Draw();
  return;
}
void RMain::PlotFit(TF1* fu){
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
void RMain::TitleResize(float factor){
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
void RMain::FitBoxResize(TObject *obj, float factorx, float factory, float shift){
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
void RMain::ColourScale(int type){
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
std::vector<TPad*> RMain::PadSetup(){
  std::vector<TPad*> pads;
  m_canvas->cd();
  m_canvas->Clear();
  pads.push_back(new TPad("ptxt","Text pad",0,.94,1,1));
  pads[0]->Draw();
  pads.push_back(new TPad("pplt","Plot pad",0,0,1,.94));
  pads[1]->Draw();
  return pads;
}
void RMain::ModuleGrid(TH2F *hi, int nChips){
  int nrow=2, ncol=nChips/2;
  if(nChips<3){
    nrow=1;
    ncol = nChips;
  }
  if(m_plotOpts->m_showAxisLabels){
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
  // doesn't work for FE-I4 due to flipped y axis
  // and is rubbish when zomming in anyway
  //gPad->SetGridx(1);
  //if(nrow>1) gPad->SetGridy(1);
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
  if(m_plotOpts->m_showchipid){
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
BadPixel* RMain::ThreshSumm(ModItem *item, int show){
  
  if(item==0) return 0;

  TH1F *thrproj, *nseproj;
  TH2F *thrhi, *nsehi;
  Int_t i,j, chip;
  Float_t con, avgThresh, avgNoise, threshDisp;

  if(item->GetDID()->Get_FitResArray()==0)
    item->GetDID()->Init_FitResArray();
  QString toptitle = "List of threshold/noise properties, data set \"";
  toptitle += item->GetDataItem()->GetName();
  toptitle += "\", module ";
  toptitle += item->ModParent()->text(0);
  BadPixel *badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
  badpix->setCaption("Threshold/noise summary");
  badpix->Init("avg. noise",toptitle.latin1());
  // do non-standard settings by hand
  badpix->PixelList->setNumCols(5);
  badpix->PixelList->horizontalHeader()->setLabel(1,"avg. threshold");
  badpix->PixelList->horizontalHeader()->setLabel(2,"threshold disp.");
  badpix->PixelList->horizontalHeader()->setLabel(4,"bad fits");
  badpix->PixelList->setNumRows(17);
  badpix->MaskButt->hide();

  if(item->GetDID()->Get_FitResArray()==0)
    item->GetDID()->Init_FitResArray();
  
  for(chip=0;chip<NCHIP+1;chip++){    
    if(chip<NCHIP)
      badpix->PixelList->setText(chip,0,QString::number(chip));
    else
      badpix->PixelList->setText(chip,0,"module");
    
    if(chip<NCHIP){
      thrhi  = item->GetDataItem()->GetMap(chip,MEAN);
      nsehi  = item->GetDataItem()->GetMap(chip,SIG);
    } else{
      thrhi  = item->GetDataItem()->GetMap(-1,MEAN);
      nsehi  = item->GetDataItem()->GetMap(-1,SIG);
    }
    
    if(thrhi!=0 && nsehi!=0){
      
      thrproj = (TH1F*) gROOT->FindObject("thrfithi");
      if(thrproj!= NULL) thrproj->Delete();
      thrproj = new TH1F("thrfithi","temporary fit histo",100,0,10000);
      nseproj = (TH1F*) gROOT->FindObject("nsefithi");
      if(nseproj!= NULL) nseproj->Delete();
      nseproj = new TH1F("nsefithi","temporary fit histo",100,0,1000);
      int nbadfits=0;
      for(i=1;i<thrhi->GetNbinsX()+1;i++){

        for(j=1;j<thrhi->GetNbinsY()+1;j++){
          con = thrhi->GetBinContent(thrhi->GetBin(i,j));
          if(con>0)
            thrproj->Fill(con);
          else
            nbadfits++;
          con = nsehi->GetBinContent(nsehi->GetBin(i,j));
          if(con>0)
            nseproj->Fill(con);
        }
      }
      avgThresh = 0;
      avgNoise  = 0;
      threshDisp= 0;
      if(nbadfits<2880){
	thrproj->Fit("gaus","Q0");
	nseproj->Fit("gaus","Q0");
	avgThresh = thrproj->GetFunction("gaus")->GetParameter(1);
	avgNoise  = thrproj->GetFunction("gaus")->GetParameter(2);
	threshDisp= nseproj->GetFunction("gaus")->GetParameter(1);
      }
      badpix->PixelList->setText(chip,1,QString::number(avgThresh ,'f',2));
      badpix->PixelList->setText(chip,2,QString::number(avgNoise  ,'f',2));
      badpix->PixelList->setText(chip,3,QString::number(threshDisp,'f',2));
      badpix->PixelList->setText(chip,4,QString::number(nbadfits));
      item->GetDID()->Fill_FitResArray(0, chip, avgThresh ); 
      item->GetDID()->Fill_FitResArray(1, chip, avgNoise  );
      item->GetDID()->Fill_FitResArray(2, chip, threshDisp);
      item->GetDID()->Fill_FitResArray(3, chip, nbadfits);
      if(chip<NCHIP){ // remove temporary chip histograms
	thrhi->Delete();
	nsehi->Delete();
      }
      // remove fit histos
      thrproj->Delete();
      nseproj->Delete();
    } else if(item->GetDID()->Get_FitResArray()!=0){
      badpix->PixelList->setText(chip,1,QString::number(item->GetDID()->Get_FitResArrayCell(0, chip),'f',2));
      badpix->PixelList->setText(chip,2,QString::number(item->GetDID()->Get_FitResArrayCell(1, chip),'f',2));
      badpix->PixelList->setText(chip,3,QString::number(item->GetDID()->Get_FitResArrayCell(2, chip),'f',2));
      badpix->PixelList->setText(chip,4,QString::number((int)item->GetDID()->Get_FitResArrayCell(3, chip)));
    } else{
      delete badpix;
      return 0;
    }
  }
  
  // get bad bumps map if appropriate
  TH2F *frmap = 0;
  std::vector<ModItem*> allRefs = item->getAllRefItems();
  for(std::vector<ModItem*>::iterator IT=allRefs.begin(); IT!=allRefs.end();IT++){
    if((*IT)->GetPID()==LOOSE){ // unconnected bump plot ID
      bool tmpbool = m_plotOpts->m_showcutpix;
      m_plotOpts->m_showcutpix = false;
      frmap = GetComp((*IT),-1,-1,-1,-1);
      m_plotOpts->m_showcutpix = tmpbool;
      break;
    }
  }
  if(frmap!=0){
    badpix->PixelList->setNumCols(6);
    badpix->PixelList->horizontalHeader()->setLabel(5,"bad bumps");
    int nbadbumps[NCHIP+1];
    for(chip=0;chip<NCHIP+1;chip++)
      nbadbumps[chip] = 0;
    for(i=1;i<frmap->GetNbinsX()+1;i++){
      for(j=1;j<frmap->GetNbinsY()+1;j++){
        int col, row;
	item->GetDataItem()->PixXYtoCCR(i-1,j-1,&chip,&col,&row);
        if(frmap->GetBinContent(frmap->GetBin(i,j))>-1000){
          nbadbumps[chip]++;
          nbadbumps[NCHIP]++;
        }
      }
    }
    for(chip=0;chip<NCHIP+1;chip++){
      item->GetDID()->Fill_FitResArray(4, chip, nbadbumps[chip]);
      badpix->PixelList->setText(chip,5,QString::number(nbadbumps[chip]));
    }
  } else if(item->GetDID()->Get_FitResArray()!=0
            && item->GetDID()->Get_FitResArrayCell(4, 0)>=0){
    badpix->PixelList->setNumCols(6);
    badpix->PixelList->horizontalHeader()->setLabel(5,"bad bumps");
    for(chip=0;chip<NCHIP+1;chip++)
      badpix->PixelList->setText(chip,5,QString::number((int)item->GetDID()->Get_FitResArrayCell(4, chip)));
  }

  if(show) badpix->show();
  
  return badpix;
}
BadPixel* RMain::ITHSumm(ModItem *item, int show){
  
  TH1F *ithproj;
  TH2F *ithhi;
  Int_t i,j, chip;
  Float_t con;

  QString toptitle = "List of ITH properties, data set \"";
  toptitle += item->GetDataItem()->GetName();
  toptitle += "\", module ";
  toptitle += item->ModParent()->text(0);
  BadPixel *badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
  badpix->setCaption("ITH summary");
  badpix->Init(" ",toptitle.latin1());
  // do non-standard settings by hand
  badpix->PixelList->setNumCols(2);
  badpix->PixelList->horizontalHeader()->setLabel(1,"avg. ITH");
  badpix->PixelList->setNumRows(17);
  badpix->MaskButt->hide();

  for(chip=0;chip<NCHIP+1;chip++){

    if(chip<NCHIP)
      ithhi  = item->GetDataItem()->GetMap(chip,ITH);
    else
      ithhi  = item->GetDataItem()->GetMap(-1,ITH);

    if(ithhi==NULL) return NULL;
    
    ithproj = (TH1F*) gROOT->FindObject("ithfithi");
    if(ithproj!= NULL) ithproj->Delete();
    ithproj = new TH1F("ithfithi","temporary fit histo",100,0,130);
    for(i=1;i<ithhi->GetNbinsX()+1;i++){
      for(j=1;j<ithhi->GetNbinsY()+1;j++){
        con = ithhi->GetBinContent(ithhi->GetBin(i,j));
        if(con>0)
          ithproj->Fill(con);
      }
    }
    ithproj->Fit("gaus","Q0");
    if(chip<NCHIP)
      badpix->PixelList->setText(chip,0,QString::number(chip,'f',0));
    else
      badpix->PixelList->setText(chip,0,"module");
    badpix->PixelList->setText(chip,1,QString::number(ithproj->GetFunction("gaus")->GetParameter(1)+.5,'f',0));
  }

  if(show) badpix->show();
  
  return badpix;
}
void RMain::ParComb(const char *formula, DatSet *did){
  // JGK: only used for TurboDAQ data, so leave histo size defined with NROW
  int i,j,k,npar;
  float cont, target;
  std::string tmpstr, tmpval, title,sform = formula;
  title  = "Map of calculated target^{-1}";
  sscanf(formula,"%f",&target);

  // create root-formula for evaluation
  TFormula myform("myform",formula);

  TH2F *parmap[NPARS];
  for(i=0;i<NPARS;i++){
    parmap[i] = did->GetMap(-1,PARA+i);
    if(parmap[i]==NULL) break;
  }
  npar = i;
  if(i<NPARS && parmap[i]!=NULL) npar++;

  TH2F *tmphi = (TH2F*) gROOT->FindObject("parcomb");
  if(tmphi!=NULL) tmphi->Delete();
  tmphi = new TH2F("parcomb",title.c_str(),NCOL*NCHIP/2,-0.5,(NCOL*NCHIP/2)-0.5,2*NROW,-.5,2.*NROW-0.5);

  for(i=1;i<tmphi->GetNbinsX()+1;i++){
    for(j=1;j<tmphi->GetNbinsY();j++){
      cont = 0;
      //fill parameters in formula
      for(k=0;k<npar;k++)
        myform.SetParameter(k,parmap[k]->GetBinContent(i,j));
      cont = myform.Eval(0);
      tmphi->SetBinContent(i,j,cont);
    }
  }
  return;
}
void RMain::SetStyle(int type){
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
void RMain::SetStyle(int type, TH2F *histo){
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
void RMain::SetStyle(int type, TH1F *histo){
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
std::string RMain::ErrorToMsg(int error_code){
  char tmp[10];
  const int nerrmax=9;
  int myerr = TMath::Abs(error_code);
  std::string retstrg[nerrmax+1];

  retstrg[0]  = "All OK";
  retstrg[1]  = "Can't handle this file type in the plot-routine chosen";
  retstrg[2]  = "Histogram needed for plotting/calculation doesn't exist";
  retstrg[3]  = "No reference data supplied (pointer(s) is/are NULL)";
  retstrg[4]  = "Pointer to log file in this data set is NULL";
  retstrg[5]  = "Argument out of range.";
  retstrg[6]  = "Could not find any data points to create graph.";
  retstrg[7]  = "Wrong data type submitted to plotting routine";
  retstrg[8]  = "Function needed for fitting isn't declared internally";
  retstrg[9]  = "Inconsistent module geometry";

  // not included in the above
  sprintf(tmp,"%d",error_code);
  retstrg[nerrmax]  = "Unknown error with code ";
  retstrg[nerrmax] += tmp;
  if(myerr>nerrmax) myerr = nerrmax;

  return retstrg[myerr];
}
int  RMain::PlotCombCal (ModItem *item, int chip){
  int i,k,binx, biny;
  char line[1000];
  double pts[2000],cont[2000], ptse[2000], cte[2000];
  TH2F *clomap[3], *chimap[3], *combpar[3], *tmpmap;
  TGraphErrors *tmphi, *closcan=NULL, *chiscan=NULL;
  MATH2F *combmap;
  //if(m_plotOpts->m_fixedToTSwitch) combpt = m_plotOpts->m_ToTSwitch;

  CheckCan();
  
  if(item->GetRID(0)==NULL || item->GetRID(1)==NULL)  return -3;

  // get parameter maps
  for(i=0;i<3;i++){
    clomap[i] = item->GetRID(0)->GetMap(chip,PARA+i);
    if(clomap[i]==NULL)  return -2;
    chimap[i] = item->GetRID(1)->GetMap(chip,PARA+i);
    if(chimap[i]==NULL)  return -2;
    combpar[i]= item->GetDataItem()->GetMap(chip,PARA+i);
    if(combpar[i]==NULL)  return -2;
  }
  // then form combined map
  tmpmap = TOT20K(chip,item->GetDID());
  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
  if(tmpmap->GetNbinsY()%336==0) flipHistoCont(tmpmap);
  combmap = new MATH2F(*((MATH2F*) tmpmap));
  if(combmap->GetEntries()<=0) combmap->SetEntries(100);
  m_currmap = combmap;

  // see if we have raw data, and if so get the plots
  // and convert the VCAL-scale from DACs to ke
  if(item->GetRID(0)->m_istype==TOTFR && item->GetRID(0)->GetLog()!=NULL &&
    (tmphi = item->GetRID(0)->GetToTHi(m_chip, m_col, m_row))!=NULL){
    closcan = (TGraphErrors*) gROOT->FindObject("tmpclo");
    if(closcan!=NULL) closcan->Delete();
    for(i=0;i<tmphi->GetN();i++){
      pts[i]  = item->GetRID(0)->GetLog()->MyModule->Chips[m_chip]->CalVcal(tmphi->GetX()[i],false)/1e3;
      ptse[i] = item->GetRID(0)->GetLog()->MyModule->Chips[m_chip]->CalVcal(tmphi->GetEX()[i],false)/1e3;
      cont[i] = tmphi->GetY()[i];
      cte[i]  = tmphi->GetEY()[i];
    }
    closcan = new TGraphErrors(tmphi->GetN(),pts,cont,ptse,cte);
    closcan->SetTitle("ToT vs VCAL raw (C_{low})");
    closcan->SetName("tmpclo");
    closcan->SetMarkerStyle(20);
    closcan->SetMarkerSize(.6f);
    m_misc_obj.push_back((TObject*) closcan);
  }
  if(item->GetRID(1)->m_istype==TOTFR && item->GetRID(1)->GetLog()!=NULL &&
    (tmphi = item->GetRID(1)->GetToTHi(m_chip, m_col, m_row))!=NULL){
    chiscan = (TGraphErrors*) gROOT->FindObject("tmpchi");
    if(chiscan!=NULL) chiscan->Delete();
    for(i=0;i<tmphi->GetN();i++){
      pts[i]  = item->GetRID(1)->GetLog()->MyModule->Chips[m_chip]->CalVcal(tmphi->GetX()[i],true)
        /item->GetDID()->m_ChiCloCal[m_chip]/1e3;
      ptse[i] = item->GetRID(1)->GetLog()->MyModule->Chips[m_chip]->CalVcal(tmphi->GetEX()[i],true)
        /item->GetDID()->m_ChiCloCal[m_chip]/1e3;
      cont[i] = tmphi->GetY()[i];
      cte[i]  = tmphi->GetEY()[i];
    }
    chiscan = new TGraphErrors(tmphi->GetN(),pts,cont,ptse,cte);
    chiscan->SetTitle("ToT vs VCAL raw (C_{high})");
    chiscan->SetName("tmpchi");
    chiscan->SetMarkerStyle(20);
    chiscan->SetMarkerSize(.6f);
    m_misc_obj.push_back((TObject*) chiscan);
  }

  // create the functions Clow, Chigh, combined
  TF1 *clo, *chi, *ccmb;
  clo = (TF1*) gROOT->FindObject("clo");
  if(clo!=NULL) clo->Delete();
  clo = new TF1("clo",TOTFunc,1,100,7);
  chi = (TF1*) gROOT->FindObject("chi");
  if(chi!=NULL) chi->Delete();
  chi = new TF1("chi",TOTFunc,1,100,7);
  ccmb = (TF1*) gROOT->FindObject("ccmb");
  if(ccmb!=NULL) ccmb->Delete();
  ccmb = new TF1("ccmb",TOTFunc,1,100,7);

  if(chip<0)
    item->GetDataItem()->PixCCRtoBins(m_chip,m_col, m_row, binx, biny);
  else
    item->GetDataItem()->PixCCRtoBins(0,m_col, m_row, binx, biny);
  for(k=0;k<3;k++){
    ccmb->SetParameter(k,combpar[k]->GetBinContent(binx, biny));
    clo->SetParameter(k,clomap[k]->GetBinContent(binx, biny));
    chi->SetParameter(k,chimap[k]->GetBinContent(binx, biny));
  }
  ccmb->SetParameter(3,1e3);
  chi->SetParameter(3,1e3*item->GetDID()->m_ChiCloCal[m_chip]);
  clo->SetParameter(3,1e3);
  for(k=4;k<7;k++){
    ccmb->SetParameter(k,0);
    chi->SetParameter(k,0);
    clo->SetParameter(k,0);
  }
  clo->SetLineColor(2);
  chi->SetLineColor(4);
  clo->SetLineStyle(3);
  chi->SetLineStyle(3);
  clo->SetLineWidth(1);
  chi->SetLineWidth(1);

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  pplt->Divide(1,2,0.001f,0.001f);
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd(1);
  SetStyle(2);
  combmap->SetMinimum(0);
  if(m_plotOpts->m_doUsrRg){
    combmap->SetMinimum((float)m_plotOpts->m_UsrMin);
    combmap->SetMaximum((float)m_plotOpts->m_UsrMax);
  }
  combmap->Draw("colz");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  if(chip<0) ModuleGrid(combmap, NCHIP);
  combmap->GetXaxis()->SetTitle("Column");
  combmap->GetYaxis()->SetTitle("Row");

  pplt->cd(2);
  SetStyle(0);
  gPad->SetBottomMargin(0.15f);
  gPad->SetLogx(m_plotOpts->m_projlogy);
  TLegend *l = new TLegend(.8,.7,.95,.95);
  Double_t maxy = 1.1*TMath::Max(ccmb->GetMaximum(),TMath::Max(clo->GetMaximum(),chi->GetMaximum()));
  sprintf(line,"ToT-vs-Q, chip %d, col %d, row %d",m_chip,m_col,m_row);
  TH2F *frame;
  frame = (TH2F*) gROOT->FindObject("frame");
  if(frame!=NULL) frame->Delete();
  frame = new TH2F("frame",line,2,1,100,2,0,maxy);
  frame->Draw();
  frame->GetXaxis()->SetTitle("Q (ke)");
  frame->GetYaxis()->SetTitle("ToT");
  ccmb->Draw("same");
  clo->Draw("same");
  chi->Draw("same");
  l->AddEntry(ccmb,"Combined","l");
  l->AddEntry(clo,"C_{low}","l");
  l->AddEntry(chi,"C_{high}","l");
  if(closcan!=NULL){
    closcan->Draw("P");
    closcan->SetLineColor(3);
    closcan->SetMarkerColor(3);
    l->AddEntry(closcan,"C_{low} raw","l");
  }
  if(chiscan!=NULL){
    chiscan->Draw("P");
    chiscan->SetLineColor(7);
    chiscan->SetMarkerColor(7);
    l->AddEntry(chiscan,"C_{high} raw","l");
  }
  l->Draw();
  /*
  TPaveText *flabel = new TPaveText(gPad->GetX1()+.05*(gPad->GetX2()-gPad->GetX1()),
                                    gPad->GetY2()-.07*(gPad->GetY2()-gPad->GetY1()),
                                    gPad->GetX1()+.5*(gPad->GetX2()-gPad->GetX1()),gPad->GetY2());
  flabel->SetTextSize(.05f);
  sprintf(line,"ToT-vs-Q, chip %d, col %d, row %d",m_chip,m_col,m_row);
  flabel->AddText(line);
  flabel->Draw();
  */
  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}
int  RMain::PlotCombCalProj(ModItem *item, int dochip){
  // JGK: only used for TurboDAQ data, so leave histo size defined with NROW
  int i,j,k,imap,ipt, chip, row, col, chipmin=0, chipmax=NCHIP;
  char line[1000], htit[100], hlabel[500], hiid[3]={'c','l','h'};
  std::string leglabel[3]={"Combined","C_{low}","C_{high}"};
  double Qpts[4]={10,20,50,100}, pars[7]={1,1,1,1,0,0,0}, xpt[1];
  double xmin[4]={0,0,0,0}, xmax[4]={150,250,350,500};
  TH2F *parmap[3][3];
  TH1F *totproj[3][4]; //3 for Clo, Chi, comb., 4 for 10,20,50,100ke points

  CheckCan();
  
  if(item->GetRID(0)==NULL || item->GetRID(1)==NULL)  return -3;

  if(dochip>=0){
    chipmin = dochip;
    chipmax = dochip+1;
  } 

  // get the maps
  for(i=0;i<3;i++){
    parmap[0][i] = item->GetDataItem()->GetMap(-1,PARA+i);
    parmap[1][i] = item->GetRefItem(0)->GetMap(-1,PARA+i);
    parmap[2][i] = item->GetRefItem(1)->GetMap(-1,PARA+i);
    if(parmap[0][i]==NULL || parmap[1][i]==NULL || parmap[2][i]==NULL) return -2;
  }
  m_currmap = NULL;

  // then make the projections for 10, 20, 50, 100ke
  for(ipt=0;ipt<4;ipt++){
    for(imap=0;imap<3;imap++){
      sprintf(htit,"tot%cproj%d",hiid[imap],(int)Qpts[ipt]);
      sprintf(hlabel,"Distribution of ToT at %dke",(int)Qpts[ipt]);
      totproj[imap][ipt] = (TH1F*) gROOT->FindObject(htit);
      if(totproj[imap][ipt]!=NULL) totproj[imap][ipt]->Delete();
      totproj[imap][ipt] = new TH1F(htit,hlabel,400,xmin[ipt],xmax[ipt]);
      //for(i=1;i<=parmap[0][0]->GetNbinsX();i++){
        //for(j=1;j<=parmap[0][0]->GetNbinsY();j++){
      for(chip=chipmin; chip<chipmax; chip++){
              for(col=0;col<NCOL;col++){
                for(row=0;row<NROW;row++){
            //item->GetDataItem()->PixXYtoCCR(i,j,&chip,&col,&row);
            item->GetDataItem()->PixCCRtoBins(chip,col,row,i,j);
	    for(k=0;k<3;k++)
              pars[k] = parmap[imap][k]->GetBinContent(i,j);
            if(imap==2)
              pars[3] = item->GetDID()->m_ChiCloCal[chip];
            else
              pars[3] = 1;
            xpt[0] = Qpts[ipt]*1e3;
            if(TOTFunc(xpt,pars)>0) totproj[imap][ipt]->Fill(TOTFunc(xpt,pars));
          }
        }
      }
    }
  }

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  pplt->Divide(2,2,0.001f,0.001f);
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  TLegend *l = new TLegend(.7,.8,.95,.95);

  for(ipt=0;ipt<4;ipt++){
    float ymax=1;
    pplt->cd(1+ipt);
    SetStyle(0);
    gPad->SetBottomMargin(0.15f);
    gPad->SetLeftMargin(0.15f);
    for(imap=0;imap<3;imap++){
      totproj[imap][ipt]->SetLineColor(imap+1);
      if(imap==0){
        totproj[imap][ipt]->Draw();
        totproj[imap][ipt]->SetLineStyle(1);
        totproj[imap][ipt]->GetXaxis()->SetTitle("ToT");
      } else{
        totproj[imap][ipt]->Draw("same");
        totproj[imap][ipt]->SetLineStyle(3);
      }
      if(totproj[imap][ipt]->GetMaximum()>ymax) 
        ymax = totproj[imap][ipt]->GetMaximum();
      if(ipt==1){
        l->AddEntry(totproj[imap][ipt],leglabel[imap].c_str(),"l");
        if(imap==2) l->Draw();
      }
    }
    totproj[0][ipt]->SetMaximum(1.1*ymax);
    if(ipt==3) totproj[0][ipt]->GetXaxis()->SetNdivisions(505);
  }

  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}
double RMain::CombCalRoot(double *x, double *par){
  double parl[3], parh[3];
  int i;
  for(i=0;i<3;i++){
    parl[i] = par[i];
    parh[i] = par[i+3];
  }
  return DatSet::CombCal(x[0],parl,parh,par[6],par[7])/1e3;
}

int RMain::PlotMasked(ModItem *itema, ModItem *itemb, int pida, int pidb, Bool_t posmask, int chip, int scanpt, int bit_mask){

  TH2F *hia=0, *hib=0, *mask=NULL, *umask=NULL;
  TH1F *frproj, *ufrproj;
  MATH2F *pltmap;
  Int_t i,j,bin;
  int type = itema->GetPID();
  Float_t con,ref,mnmax=0,mnmin=1000,umnmax=0,umnmin=1000;
  Char_t line[1024], name[256], namea[200], nameb[200];
  namea[0]='\0';
  nameb[0]='\0';
  BadPixel *badpix = new BadPixel(NULL,NULL,"badpix",FALSE,Qt::WDestructiveClose);
  QString toptitle = "dataset \"";
  toptitle += itema->GetDataItem()->GetName();
  toptitle += "\" with ";
  if (posmask) toptitle += "positive ";
  else toptitle += "negative ";
  toptitle += "mask from \"";
  toptitle += itemb->GetDataItem()->GetName();
  toptitle += "\"";
  if (posmask && m_plotOpts->m_zerotab) badpix->Init("Value (0 and not)",toptitle.latin1());
  else if (posmask && !m_plotOpts->m_zerotab) badpix->Init("Value (not 0 only)",toptitle.latin1());
  else badpix->Init("Value",toptitle.latin1());

  CheckCan();

  // get module geometry
  int nCol, nRow, nFe;
  itema->GetDataItem()->getGeom(nRow, nCol, nFe);
  int nColb, nRowb, nFeb;
  itemb->GetDataItem()->getGeom(nRowb, nColb, nFeb);
  if(nFe!=nFeb || nCol!=nColb || nRow!=nRowb) return -9;

  if(pida==T20K)
    hia = TOT20K(chip,itema->GetDID());
  else if(pida==BADSEL)
    hia = GetBadSelMap(itema->GetDataItem()->GetMap(-1,MAMASK),scanpt);
  else if(pida>=TWODMEAN && pida<= TWODOPT)
    hia = itema->GetDataItem()->Get2DMap(chip,pida,scanpt);
  else if(pida>OPTS && pida<PIXDBTYPES)
    hia = GetComp(itema, chip,-1,-1,-1);
  else if(pida<PIXDBTYPES)
    hia = itema->GetDataItem()->GetMap(chip,pida);
  else{
#ifdef PIXEL_ROD // can link to PixScan
    PixDBData *pdbd = dynamic_cast<PixDBData*>(itema->GetDataItem());
    if(pdbd!=0){
      hia = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)(pida-PIXDBTYPES),
							m_chip, m_col, m_row));
      PixScan ps;
      for(std::map<std::string, int>::iterator it = ps.getHistoTypes().begin();
	  it!=ps.getHistoTypes().end(); it++){
	if(it->second==(pida-PIXDBTYPES)) sprintf(namea,"%s",it->first.c_str());
      }      
      if(chip>=0) hia = pdbd->getChipMap(chip);
      if(hia!=0){
      }
    }
#endif
  }
  if (hia==NULL) return -2;
#ifdef PIXEL_ROD // can link to PixScan
  // temporary histo will be removed later, but PixDBData wants to do so, too
  if((dynamic_cast<PixDBData*>(itema->GetDataItem()))!=0){
    hia = new TH2F(*hia);
    hia->SetName("tmphia");
  }
#endif

  if(pidb>=TWODMEAN && pidb<= TWODOPT)
    hib = itema->GetDataItem()->Get2DMap(chip,pidb,scanpt);
  else if(pidb>OPTS && pidb<PIXDBTYPES)
    hib = GetComp(itemb, chip,-1,-1,-1);
  else if(pidb<PIXDBTYPES){
    hib = itemb->GetDataItem()->GetMap(chip,pidb);
    if(pidb==MAMASK && bit_mask>0){
      TH2F *mm = (TH2F*)gROOT->FindObject("maskmask");
      if(mm!=0) mm->Delete();
      mm = GetBadSelMap(hib,bit_mask);
      hib = mm;
    }
  }else{
#ifdef PIXEL_ROD // can link to PixScan
    PixDBData *pdbd = dynamic_cast<PixDBData*>(itemb->GetDataItem());
    if(pdbd!=0){
      hib = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)(pidb-PIXDBTYPES),
							m_chip, m_col, m_row));
      PixScan ps;
      for(std::map<std::string, int>::iterator it = ps.getHistoTypes().begin();
	  it!=ps.getHistoTypes().end(); it++){
	if(it->second==(pidb-PIXDBTYPES)) sprintf(nameb,"%s",it->first.c_str());
      }      
      if(chip>=0) hib = pdbd->getChipMap(chip);
    }
#endif
  }
  if (hib==NULL) return -2;
#ifdef PIXEL_ROD // can link to PixScan
  // temporary histo will be removed later, but PixDBData wants to do so, too
  if((dynamic_cast<PixDBData*>(itemb->GetDataItem()))!=0){
    hib = new TH2F(*hib);
    hib->SetName("tmphib");
  }
#endif

  if(type==FDACP) type = TDACP;     // only needed different ID for getting the map
  if(type==FDACLOG) type = TDACLOG; // only needed different ID for getting the map

  mask = (TH2F*) gROOT->FindObject("mask");
  if (mask!=NULL) mask->Delete();
  sprintf(name,"%s - masked pixels",(pida<DSET)?m_plotOpts->axtit[pida].c_str():namea);
  int nColTot = nCol*nFe/2, nRowTot=nRow*2;
  if(nFe==4){
    nColTot *= 2;
    nRowTot /= 2;
  }
  if(chip<0){
    mask = new TH2F("mask",name,nColTot,-.5,-0.5+(double)nColTot,nRowTot,-0.5,-0.5+nRowTot);
  }
  else{
    mask = new TH2F("mask",name,nCol,-0.5,-0.5+(double)nCol,nRow,-0.5,-0.5+(double)nRow);
  }
  mask->Reset();
  mask->SetTitle(name);

  umask = (TH2F*) gROOT->FindObject("umask");
  if (umask!=NULL) umask->Delete();
  sprintf(name,"%s - not-masked pixels",(pida<DSET)?m_plotOpts->axtit[pida].c_str():namea);
  if(chip<0){
    umask = new TH2F("umask",name,nColTot,-.5,-0.5+(double)nColTot,nRowTot,-0.5,-0.5+nRowTot);
  }
  else{
    umask = new TH2F("umask",name,nCol,-0.5,-0.5+(double)nCol,nRow,-0.5,-0.5+(double)nRow);
  }
  umask->Reset();
  umask->SetTitle(name);

  for(i=0;i<hia->GetNbinsX()+1;i++){
    for(j=0;j<hia->GetNbinsY()+1;j++){
      bin = hia->GetBin(i,j);
      con = hia->GetBinContent(bin);
      ref = hib->GetBinContent(bin);
      
      //positive or negative mask applied
      if (ref==0) {
        umask->SetBinContent(bin,con);
        if (umnmin>con) umnmin=con;
        if (umnmax<con) umnmax=con;
      } else {
        mask->SetBinContent(bin,con);
        if (mnmin>con) mnmin=con;
        if (mnmax<con) mnmax=con;
      }
      
      if (posmask) {
        if ( m_plotOpts->m_noisetab && ref!=0 ) {
          if ( m_plotOpts->m_zerotab ) badpix->AddTabLine(i,j,con);
          if ( !m_plotOpts->m_zerotab ) {
            if ( pida==TDACLOG && (int)con!=0 ) badpix->AddTabLine(i,j,con);
            else if ( con>0 ) badpix->AddTabLine(i,j,con);
          }
        }
      } else {
        //                if ( m_plotOpts->m_noisetab && ref!=0 ) { badpix->AddTabLine(i,j,con); }
        if ( m_plotOpts->m_noisetab && ref==0 ) {
          if ( m_plotOpts->m_zerotab ) badpix->AddTabLine(i,j,con);
          if ( !m_plotOpts->m_zerotab ) {
            if ( pida==TDACLOG && (int)con!=0 ) badpix->AddTabLine(i,j,con);
            else if ( con>0 ) badpix->AddTabLine(i,j,con);
          }
        }
      }
    }
  }
  
  if(mnmin>0)
    mnmin *= 0.8f;
  else
    mnmin *=1.2f;
  if(mnmax>0)
    mnmax *= 1.2f;
  else
    mnmax *=0.8f;
  if(type<DSET && m_plotOpts->m_UsrIUse[type]){
    mnmin = m_plotOpts->m_UsrIMin[type];
    mnmax = m_plotOpts->m_UsrIMax[type];
  }
  if(m_plotOpts->m_doUsrRg){
    mnmin = m_plotOpts->m_UsrMin;
    mnmax = m_plotOpts->m_UsrMax;
  }

  if(umnmin>0)
    umnmin *= 0.8f;
  else
    umnmin *=1.2f;
  if(umnmax>0)
    umnmax *= 1.2f;
  else
    umnmax *=0.8f;
  if(type<DSET && m_plotOpts->m_UsrIUse[type]){
    umnmin = m_plotOpts->m_UsrIMin[type];
    umnmax = m_plotOpts->m_UsrIMax[type];
  }
  if(m_plotOpts->m_doUsrRg){
    umnmin = m_plotOpts->m_UsrMin;
    umnmax = m_plotOpts->m_UsrMax;
  }

  frproj = (TH1F*) gROOT->FindObject("mskhist");
  if (frproj!= NULL) frproj->Delete();
  sprintf(name,"%s - masked pixels histo",(pida<DSET)?m_plotOpts->axtit[pida].c_str():namea);
  frproj = new TH1F("mskhist",name,100,mnmin,mnmax);
  
  ufrproj = (TH1F*) gROOT->FindObject("umskhist");
  if (ufrproj!= NULL) ufrproj->Delete();
  sprintf(name,"%s - not-masked pixels histo",(pida<DSET)?m_plotOpts->axtit[pida].c_str():namea);
  ufrproj = new TH1F("umskhist",name,100,umnmin,umnmax);
  
  for(i=0;i<hia->GetNbinsX()+1;i++){
    for(j=0;j<hia->GetNbinsY()+1;j++){
      bin = hia->GetBin(i,j);
      con = hia->GetBinContent(bin);
      ref = hib->GetBinContent(bin);
      
      if (ref==0) {
        if (con!=0) ufrproj->Fill(con);
      } else {
        if (con!=0) frproj->Fill(con);
      }
    }
  }
  
  // flip tmphi for FE-I4 added by crgg  
  //  if(tmphi->GetNbinsX()==80 && tmphi->GetNbinsY()==336){
  // JGK: can have 80 or 160 col's (PPS module plot), so can only cut on # rows
  if(mask->GetNbinsY()%336==0) flipHistoCont(mask);
  if(umask->GetNbinsY()%336==0) flipHistoCont(umask);
  if (posmask) {
    pltmap = new MATH2F(*((MATH2F*) mask));
    mask->Delete();
  } else {
    pltmap = new MATH2F(*((MATH2F*) umask));
    umask->Delete();
  }
  pltmap->SetEntries(100);
  m_currmap = pltmap;
  SetStyle(0);
  if (posmask) {
    pltmap->SetMinimum(mnmin);
    pltmap->SetMaximum(mnmax);
  } else {
    pltmap->SetMinimum(umnmin);
    pltmap->SetMaximum(umnmax);
  }
  frproj->GetXaxis()->SetTitle(m_plotOpts->axtit[pida].c_str());
  ufrproj->GetXaxis()->SetTitle(m_plotOpts->axtit[pida].c_str());

  // delete temporary histograms if created here earlier on
#ifdef PIXEL_ROD // can link to PixScan
  if((dynamic_cast<PixDBData*>(itema->GetDataItem()))!=0) hia->Delete();
  if((dynamic_cast<PixDBData*>(itemb->GetDataItem()))!=0) hib->Delete();
#endif

  m_canvas->cd();
  m_canvas->Clear();
  
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  ptxt->cd();

  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"datset %s masked with %s",itema->GetDataItem()->GetName(), itemb->GetDataItem()->GetName());
  if(pidb>=DSET) sprintf(line,"%s-%s",line, nameb);
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",itema->ModParent()->text(0).latin1());
  m_text->AddText(line);
  m_text->Draw();
  
  m_canvas->cd();
  TPad *pplt = new TPad("pplt","Text pad",0,0,1,.94);
  pplt->Draw();
  pplt->cd();
  pplt->Divide(1,3,0.001f,0.001f);

  pplt->cd(1);
  gPad->SetRightMargin(0.15f);
  gPad->SetTopMargin(0.15f);
  gPad->SetBottomMargin(0.15f);
  pltmap->Draw("COLZ");
  if(m_plotOpts->m_doBlackBg){
    m_canvas->Modified();
    m_canvas->Update();
    ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
  }
  if(chip<0) ModuleGrid(pltmap, nFe);
  pltmap->GetXaxis()->SetTitle("Column");
  pltmap->GetYaxis()->SetTitle("Row");
  if(m_plotOpts->m_plotfitopt==0) TitleResize(1.5);
  if (posmask) pplt->cd(2);
  else         pplt->cd(3);
  gPad->SetTopMargin(0.15f);
  gPad->SetBottomMargin(0.15f);
  frproj->Draw();
  if(m_plotOpts->m_FitGauss){
    frproj->Fit("gaus","Q");
    FitBoxResize((TObject*)frproj,0.7f);
  }
  TitleResize(1.5f);
  if (posmask) pplt->cd(3);
  else         pplt->cd(2);
  gPad->SetTopMargin(0.15f);
  gPad->SetBottomMargin(0.15f);
  ufrproj->Draw();
  if(m_plotOpts->m_FitGauss){
    ufrproj->Fit("gaus","Q");
    FitBoxResize((TObject*)ufrproj,0.7f);
  }
  TitleResize(1.5f);

  m_canvas->Modified();
  m_canvas->Update();

  if (m_plotOpts->m_noisetab && badpix->nRows()) badpix->show();

  return 0;
}

BadPixel* RMain::MaskTable(ModItem *item, int setchip){

  int chip, col, row;
  int binx, biny, type;
  float xval, yval;
  TH2F *tmphi;
  BadPixel *badpix=NULL;

  CheckCan();

  switch(item->GetPID()){
  case MASKTAB:
    type = MAMASK;
    break;
  case DEADTAB:
    type = DEAD_CHAN;
    break;
  default:
    return NULL;
  }

  tmphi = item->GetDataItem()->GetMap(setchip,type);
  if(tmphi==NULL) return badpix;

  QString toptitle;
  if(type==MAMASK)
    toptitle = "List of masked channels, data set \"";
  else
    toptitle = "List of dead channels, data set \"";
  toptitle += item->GetDataItem()->GetName();
  toptitle += "\", module ";
  toptitle += item->ModParent()->text(0);
  badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
  badpix->Init(" ",toptitle.latin1());
  // do non-standard settings by hand
  if(type==MAMASK) badpix->MaskButt->hide();

  int nCol, nRow, Nchips;
  item->GetDataItem()->getGeom(nRow, nCol, Nchips);
  if(setchip>=0)
    Nchips = 1;

  for(chip=0;chip<Nchips;chip++){
    for(col=0;col<nCol;col++){
      for(row=0;row<nRow;row++){
        item->GetDataItem()->PixCCRtoBins(chip,col,row,binx,biny);
        if(setchip>=0)
          item->GetDataItem()->PixXY(setchip,col,row,&xval,&yval);
        else
          item->GetDataItem()->PixXY(chip,col,row,&xval,&yval);
        if(tmphi->GetBinContent(binx,biny)>0)
          badpix->AddTabLine(1+(int)xval,1+(int)yval,1);
      }
    }
  }
  badpix->PixelList->setNumCols(3);

  badpix->show();

  return badpix;
}
TF1* RMain::GetFitFunc(int type){
  std::string funcname = GetFitName(type).c_str();
  if(funcname=="unknown") return NULL;
  TF1* retfunc =  (TF1*)gROOT->FindObject(funcname.c_str());
  if(retfunc==NULL){
    // create dummy-histo for ROOT-intrinsic functions to have it created
    TH1F hi("a","b",10,0,1);
    hi.Fit(funcname.c_str());
    retfunc = hi.GetFunction(funcname.c_str());
  }
  return retfunc;
}
int RMain::GetFitType(const char *name){
  std::string funcname = name;
  int fittype = -1;

  if(funcname=="erfu")
    fittype = 0;

  if(funcname=="totfu")
    fittype = 1;
  
  if(funcname=="pol1")
    fittype = 2;
  
  if(funcname=="expo")
    fittype = 3;
  
  if(funcname=="onemexp")
    fittype = 4;
  
  if(funcname=="twoexp")
    fittype = 5;

  if(funcname=="pol0")
    fittype = 6;
  
  if(funcname=="pol2")
    fittype = 7;
  
  if(funcname=="gaus")
    fittype = 8;
  
  if(funcname=="pol3")
    fittype = 12;
  
  return fittype;
}
std::string RMain::GetFitName(int type){
  const int nfits=14;
  std::string funcname[nfits]={"erfu","totfu","pol1","expo","onemexp","twoexp","pol0","pol2","gaus",
                               "erfu","erfu","erfu","pol3","unknown"}; 
  // three extra-entries for erfu since we have simple and 2xVirzi fits

  if(type<0 || type>(nfits-1))
    return funcname[nfits-1];
  else
    return funcname[type];

}
int  RMain::PlotScan(ModItem **item, int nitems){
  if(nitems<=0) return -2;

  CheckCan();

  TMultiGraph *mg = new TMultiGraph();
  //  m_misc_obj.push_back((TObject*) mg);
  TGraphErrors *scangr=0;
  Int_t i, j;//, type = item[0]->GetPID();
  double *x, *y, *xe, *ye;
  char line[2000];
  float calfac = 1;

  TGraphErrors *gr;
  TLegend *leg = new TLegend(.5f,.95f-.05f*(float)nitems,.99f,.99f);

  for(i=0; i<nitems;i++){
    if(item[i]->GetDID()->GetLog()==NULL) return -4;
    if(!item[i]->GetDID()->m_isscan && item[i]->GetDID()->GetLog()->MyScan->GetOuter()=="No Scan") return -1;
    // 1D Scurve histo and fit results (if there)
    switch(item[i]->GetPID()){
    case TOTRAW:
      scangr = item[i]->GetDID()->GetToTHi(m_chip, m_col, m_row);
      break;
    default:
      scangr = item[i]->GetDataItem()->GetScanHi(m_chip, m_col, m_row, RAW);
      break;
    }
    if(scangr==0)
      return -2;
    
    calfac = 1;
    if(((item[i]->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" ||
         item[i]->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff") && 
        item[i]->GetDID()->GetLog()->MyScan->GetOuter()=="No Scan")||
       item[i]->GetDID()->GetLog()->MyScan->GetOuter()=="VCal FE" ||
       item[i]->GetDID()->GetLog()->MyScan->GetOuter()=="VCal diff"){
      if(item[i]->GetDID()->GetLog()!=NULL){
        if(item[i]->GetDID()->GetLog()->MyModule->CHigh)
          calfac  = item[i]->GetDID()->GetLog()->MyModule->Chips[m_chip]->Chi;
        else
          calfac  = item[i]->GetDID()->GetLog()->MyModule->Chips[m_chip]->Clo;
        if(item[i]->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" ||
           item[i]->GetDID()->GetLog()->MyScan->GetOuter()=="VCal FE")
          //          calfac *= item[i]->GetDID()->GetLog()->MyModule->Chips[m_chip]->Vcal/0.160218;
          calfac = -1;
        else if(item[i]->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff" ||
                item[i]->GetDID()->GetLog()->MyScan->GetOuter()=="VCal diff")
          calfac  *= item[i]->GetDID()->GetLog()->extcal/0.160218;
        else
          calfac = 1;
      }
    }
    x  = scangr->GetX();
    y  = scangr->GetY();
    xe = scangr->GetEX();
    ye = scangr->GetEY();
    for(j=0;j<scangr->GetN();j++){
      if(calfac<0){
        x[j]  = item[i]->GetDID()->GetLog()->MyModule->Chips[m_chip]->CalVcal(x[j],
                              item[i]->GetDID()->GetLog()->MyModule->CHigh);
        xe[j] = item[i]->GetDID()->GetLog()->MyModule->Chips[m_chip]->CalVcal(xe[j],
                              item[i]->GetDID()->GetLog()->MyModule->CHigh);
      } else{
        x[j]  *= calfac;
        xe[j] *= calfac;
      }
    }
    gr = new TGraphErrors(scangr->GetN(),x,y,xe,ye);
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(i+1);
    gr->SetLineColor(i+1);
    m_misc_obj.push_back((TObject*) gr);
    mg->Add(gr);
    leg->AddEntry(gr,item[i]->GetDID()->GetName(), "P");
  }

  mg->SetTitle("");

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  
  m_text->Clear();
  sprintf(line," ");
  for(i=0;i<nitems;i++){
    if(i<nitems-1)
      sprintf(line,"%s%s + ",line,item[i]->GetDID()->GetName());
    else
      sprintf(line,"%s%s.",line,item[i]->GetDID()->GetName());
  }
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"   -  Chip %d, col %d, row %d",item[0]->ModParent()->text(0).latin1(),m_chip,m_col,m_row);
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  if(m_psfile!=NULL) m_psfile->NewPage();

  pplt->cd();
  gPad->SetRightMargin(0.15f);
  mg->Draw("AP");
  if(calfac==1)
    mg->GetXaxis()->SetTitle(scangr->GetXaxis()->GetTitle());
  else
    mg->GetXaxis()->SetTitle("Q_{VCAL} (e)");
  mg->GetXaxis()->SetTitleSize(.03f);
  mg->GetXaxis()->SetLabelSize(.03f);
  mg->GetYaxis()->SetTitle(scangr->GetYaxis()->GetTitle());
  mg->GetYaxis()->SetTitleOffset(1.2f);
  mg->GetYaxis()->SetTitleSize(.03f);
  mg->GetYaxis()->SetLabelSize(.03f);
  if(leg!=NULL) leg->Draw();
  m_canvas->Update();
  m_canvas->Modified();
  return 0;

}

int RMain::Plot2DComp(ModItem *item, int chip){

  TH2F *orgmap, *refmap, *frmap;
  MATH2F *pltmap;
  TGraphErrors *orggr, *refgr;
  TGraph *frgr;
  Float_t con, ref, ylow;
  Char_t line[1024];
  Int_t i,j, bin, row, col, mychip, type=item->GetPID();
  Double_t *xo, *yo, *yeo, *yr, *yer, nrat;

  CheckCan();

  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  i=-1;
  switch(type){
  case TDTRAT:
  case TDTDIF:
    i = TWODRAW;
    break;
  case TDNRAT:
  case TDNDIF:
    //i = SIG;
    i = TWODRAW;
    break;
  default:
    return -1;
  }

  // get colour maps
  int nColTot = nCol*nFe/2, nRowTot=nRow*2;
  if(nFe==4){
    nColTot *= 2;
    nRowTot /= 2;
  }
  orgmap = item->GetDataItem()->GetMap(chip,i);
  if(orgmap==NULL)  return -2;
  frmap = (TH2F*) gROOT->FindObject("frmap");
  if(frmap!=NULL) frmap->Delete();
  sprintf(line,"Map of %s (cumulated)",m_plotOpts->axtit[type].c_str());
  if(chip<0){
    frmap = new TH2F("frmap",line,nColTot,-.5,-0.5+(double)nColTot,nRowTot,-0.5,-0.5+nRowTot);
  }
  else{
    frmap = new TH2F("frmap",line,nCol,-0.5,-0.5+(double)nCol,nRow,-0.5,-0.5+(double)nRow);
  }
  // had to wait till now because refmap and orgmap have same title in root
  if(item->GetRID()==NULL)   return -3;
  refmap = item->GetRID()->GetMap(chip,i);
  if(refmap==NULL)  return -2;

  // calculate new map
  for(i=1;i<frmap->GetNbinsX()+1;i++){
    for(j=1;j<frmap->GetNbinsY()+1;j++){
      bin = frmap->GetBin(i,j);
      con = orgmap->GetBinContent(bin);
      ref = refmap->GetBinContent(bin);
      item->GetDataItem()->PixXYtoCCR(i-1,j-1,&mychip,&col,&row);
      switch(type){
      case TDNRAT:
      case TDTRAT:
        if(ref!=0)        con /= ref;
        else                con = 0;
        break;
      case TDNDIF:
      case TDTDIF:
        if(ref!=0)        con -= ref;
        else                con = 0;
        break;
      default:
        con = 0;
        break;
      }
      frmap->SetBinContent(bin,con);
    }
  }
  if(frmap->GetNbinsY()%336==0) flipHistoCont(frmap);
  pltmap = new MATH2F(*((MATH2F*) frmap));
  if(pltmap->GetEntries()<=0) pltmap->SetEntries(100);
  m_currmap = pltmap;

  // get graphs
  orggr = item->GetDataItem()->GetScanHi(m_chip, m_col, m_row);
  refgr = item->GetRefItem()->GetScanHi(m_chip, m_col, m_row);

  frgr = NULL;
  if(orggr!=NULL && refgr!=NULL){
    xo  = orggr->GetX();
    yo  = orggr->GetY();
    yeo = orggr->GetEY();
    yr  = refgr->GetY();
    yer = refgr->GetEY();
    nrat= sqrt((Double_t)item->GetDataItem()->getNevents() / (Double_t)item->GetRefItem()->getNevents());
    for(j=0;j<orggr->GetN() && j<refgr->GetN();j++){
      switch(type){
      case TDNRAT:
        if(yer[j]!=0)
          yo[j] = yeo[j]/yer[j] * nrat;
        else
          yo[j] = 0;
        break;
      case TDTRAT:
        if(yr[j]!=0)
          yo[j] /= yr[j];
        else
          yo[j] = 0;
        break;
      case TDNDIF:
        yo[j] = yeo[j]*(Double_t)item->GetDataItem()->getNevents()
          - yer[j]*(Double_t)item->GetRefItem()->getNevents();
        break;
      case TDTDIF:
        yo[j] -= yr[j];
        break;
      default:
        yo[j] = 0;
        break;
      }
    }
    frgr = new TGraph(j,xo,yo);
    frgr->GetXaxis()->SetTitle(orggr->GetXaxis()->GetTitle());
    frgr->GetYaxis()->SetTitle(m_plotOpts->axtit[type].c_str());
    frgr->SetMarkerStyle(20);
    sprintf(line,"Chip %d col %d row %d",m_chip,m_col,m_row);
    frgr->SetTitle(line);
    m_misc_obj.push_back((TObject*) frgr);
  }

  // plotting

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *spplt=NULL;
  ylow = 0;
  if(frgr!=NULL  && m_plotOpts->m_plotfitopt==0){
    spplt = new TPad("spplt","Sub-plot pad",0,0,1,.3);
    spplt->Draw();
    ylow = 0.3f;
  }
  TPad *pplt = new TPad("pplt","Plot pad",0,ylow,1,.94);
  pplt->Draw();
  
  if(m_psfile!=NULL) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s: %s vs %s.",m_plotOpts->axtit[type].c_str(),item->GetDataItem()->GetName(), item->GetRefItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd();
  if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==1) {
    if(pltmap->GetMinimum() > 0.95*pltmap->GetMaximum())
      pltmap->SetMinimum(0.95*pltmap->GetMaximum());
    pltmap->Draw("COLZ");
    if(m_plotOpts->m_doBlackBg){
      m_canvas->Modified();
      m_canvas->Update();
      ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
    }
    if(m_plotOpts->m_plotfitopt==0)
      SetStyle(2);
    else
      SetStyle(1);
    if(chip<0) ModuleGrid(pltmap, nFe);
    pltmap->GetXaxis()->SetTitle("Column");
    pltmap->GetYaxis()->SetTitle("Row");
  }

  if(frgr!=NULL && (m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt>1)){
    if(m_plotOpts->m_plotfitopt==0){
      spplt->cd();
      SetStyle(0);
    } else
      SetStyle(1);
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    frgr->Draw("AP");

    if(m_plotOpts->m_plotfitopt==0) TitleResize(1.5);
    else                            TitleResize(0.7f);
  }
  
  m_canvas->Modified();
  m_canvas->Update();

  return  0;
}
void RMain::ClearMemory(){
  if(m_lastmap!=NULL){
    m_lastmap->Delete();
    m_lastmap = NULL;
  }
  for(unsigned int i=0;i<m_misc_obj.size();i++){
    if(m_misc_obj[i]!=NULL)
      m_misc_obj[i]->Delete();
  }
  m_misc_obj.clear();
  return;
}
BadPixel* RMain::PeakSumm(ModItem *item, int show, float nom_peak){
  TH1F *peakproj;
  TH2F *peakhi;
  Int_t i,j, chip;
  Float_t con;

  QString toptitle = "List of avg. calibrated peak position, data set \"";
  toptitle += item->GetDataItem()->GetName();
  toptitle += "\", module ";
  toptitle += item->ModParent()->text(0);
  BadPixel *badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
  badpix->setCaption("Source peak summary");
  badpix->Init("n.u.",toptitle.latin1());
  // do non-standard settings by hand
  badpix->PixelList->horizontalHeader()->setLabel(1,"avg. peak pos");
  badpix->PixelList->horizontalHeader()->setLabel(2,"nom/meas. pos.");
  badpix->PixelList->setNumRows(17);
  badpix->MaskButt->hide();

  for(chip=0;chip<NCHIP+1;chip++){

    if(chip<NCHIP){
      peakhi  = item->GetDataItem()->GetMap(chip,CAL_FAST);
    } else{
      peakhi  = item->GetDataItem()->GetMap(-1,CAL_FAST);
    }
    if(peakhi==NULL) return NULL;
    
    peakproj = (TH1F*) gROOT->FindObject("peakfithi");
    if(peakproj!= NULL) peakproj->Delete();

    float mnmin = peakhi->GetMinimum();
    float mnmax = peakhi->GetMaximum();
    if(m_plotOpts->m_UsrIUse[CAL_FAST]){
      mnmin = m_plotOpts->m_UsrIMin[CAL_FAST];
      mnmax = m_plotOpts->m_UsrIMax[CAL_FAST];
    }
    if(m_plotOpts->m_doUsrRg){
      mnmin = m_plotOpts->m_UsrMin;
      mnmax = m_plotOpts->m_UsrMax;
    }

    peakproj = new TH1F("peakfithi","temporary fit histo",100,mnmin, mnmax);
    for(i=1;i<peakhi->GetNbinsX()+1;i++){
      for(j=1;j<peakhi->GetNbinsY()+1;j++){
        con = peakhi->GetBinContent(peakhi->GetBin(i,j));
        if(con>0)
          peakproj->Fill(con);
      }
    }
    i = peakproj->GetMaximumBin();
    mnmax = peakproj->GetBinCenter(i);
    peakproj->Fit("gaus","Q0", "", mnmax*.9, mnmax*1.1);
    if(chip<NCHIP)
      badpix->PixelList->setText(chip,0,QString::number(chip,'f',0));
    else
      badpix->PixelList->setText(chip,0,"module");
    con = peakproj->GetFunction("gaus")->GetParameter(1);
    badpix->PixelList->setText(chip,1,QString::number(con,'f',0));
    if(con!=0)
      con = nom_peak/con;
    badpix->PixelList->setText(chip,2,QString::number(con,'f',3));
  }

  if(show) badpix->show();
  
  return badpix;
}

TH2F* RMain::GetComp(ModItem *item, int chip, int ps_level, int scanpt1, int scanpt2){
  return GetComp(item,chip,item->GetPID(),ps_level, scanpt1, scanpt2);
}
TH2F* RMain::GetComp(ModItem *item, int chip, int type, int ps_level, int scanpt1, int scanpt2){
  TH2F *orgmap=0, *refmap=0, *frmap=0;
  int i, ps_type=-1;
  bool clean_orgmap=false;

  if(item->GetDataItem()==0) return 0;

  i=-1;
  if(type!=MINTDIF && type!=MINTRAT && item->GetPID()<PIXDBTYPES){
    switch(type){
    case XTFR:
    case XTFRNN:
    case TRAT:
    case TDIF:
    case SHORT:
    case TIMEDIF:
    case TCALDIF:
      i = MEAN;
      break;
    case NRAT:
    case NDIF:
    case NSQDIF:
    case LOOSE:
    case XTNOISE:
      i = SIG;
      break;
    case DELCAL:
      i = TIME;
      break;
    default:
      return 0;
    }
    orgmap = item->GetDataItem()->GetMap(chip,i);
  } else if(item->GetPID()<PIXDBTYPES){
    orgmap = MapMinThresh(item->GetDataItem(),chip);
    orgmap->SetName("min_map2");
  } else{
#ifdef PIXEL_ROD // can link to PixScan
    ps_type = item->GetPID()-PIXDBTYPES;
    PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
    if(pdbd!=0){
      orgmap = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							 m_chip, m_col, m_row, scanpt1, ps_level));
      if(chip>=0) orgmap = pdbd->getChipMap(chip);
    }
#endif
  }

  if(item->GetRefItem()==0 && (ps_type<0 || (scanpt1<0 && scanpt2<0)))   return 0;

  if(type!=MINTDIF && type!=MINTRAT && ps_type<0)
    refmap = item->GetRefItem()->GetMap(chip,i);
  else if(ps_type<0)
    refmap = MapMinThresh(item->GetRefItem(),chip);
  else{
#ifdef PIXEL_ROD // can link to PixScan
    //ps_type = item->GetPID()-PIXDBTYPES;
    if(scanpt2<0){ // reference to another data item
      PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetRefItem());
      if(pdbd!=0){
	refmap = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							   m_chip, m_col, m_row, scanpt1, ps_level));
	if(chip>=0) refmap = pdbd->getChipMap(chip);
      }
      else // reference is probably to a non-PixLib item
	refmap = item->GetRefItem()->GetMap(chip,-scanpt2);
    } else {
      PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
      if(pdbd!=0){
	// must make copy of "orgmap" since this will be replaced with next call
	clean_orgmap = true;
	orgmap = new TH2F(*orgmap);
	orgmap->SetName((std::string(orgmap->GetName())+"_copy").c_str());
	// now we're ready for another histo
	refmap = dynamic_cast<TH2F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							   m_chip, m_col, m_row, scanpt2, ps_level));
	if(chip>=0) refmap = pdbd->getChipMap(chip);
      }
    }
#endif
  }

  frmap = GetComp(item, type, orgmap, refmap);
  if(clean_orgmap) orgmap->Delete();
  return frmap;
}
TH2F* RMain::GetComp(ModItem *item, int type, TH2F *orgmap, TH2F *refmap){

  TH2F *frmap;
  Char_t line[1024];
  Int_t i,j, bin, row, col, mychip;
  Float_t con, ref;

  if(item==0) return 0;

  if(orgmap==0)  return 0;
  if(refmap==0)  return 0;
  if(type>=DSET) return 0;

  // get geometry info
  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  frmap = (TH2F*) gROOT->FindObject("frmap");
  if(frmap!=0) frmap->Delete();
  sprintf(line,"Map of %s",m_plotOpts->axtit[type].c_str());
  frmap = new TH2F(*orgmap);
  frmap->SetName("frmap");
  frmap->SetTitle(line);
  frmap->Reset();

  for(i=1;i<frmap->GetNbinsX()+1;i++){
    for(j=1;j<frmap->GetNbinsY()+1;j++){
      bin = frmap->GetBin(i,j);
      con = orgmap->GetBinContent(bin);
      ref = refmap->GetBinContent(bin);
      item->GetDataItem()->PixXYtoCCR(i-1,j-1,&mychip,&col,&row);
      switch(type){
      case XTFR:
      case SHORT:
      case XTNOISE:
        ref *=50;          // 100=%, but xtalk comes from 2 pixels -> 50
	if(j==1 || j==nRow || (nFe!=4 && (j==nRow+1 || j==2*nRow)))
          ref *= 2;        // unless pixel is on the edge
        if(con!=0) con = ref/con;
        else           con = 0;
        break;
      case XTFRNN:
        ref *=50;          // 100=%, but xtalk comes from 2 pixels -> 50
	if(j==1 || j==nRow || (nFe!=4 && (j==nRow+1 || j==2*nRow)))
          ref *= 2;        // unless pixel is on the edge
        if(con>0) 
          con = ref/con;
        else{
          float xt_thresh = 2e5, vcmax;
          int chip, col, row;
	  item->GetDataItem()->PixXYtoCCR(i-1,j-1,&chip,&col,&row);
          if(item->GetDID()->GetLog()!=0){
            vcmax= (float)item->GetDID()->GetLog()->MyScan->Inner->Stop;
            xt_thresh = item->GetDID()->GetLog()->MyModule->Chips[chip]->CalVcal(vcmax,true);
          }
          con = ref/xt_thresh;
        }
        break;
      case LOOSE:
        //noise threshold cut
        if ((con<m_plotOpts->m_noisecut)&&(ref<m_plotOpts->m_noisecut)) {
          if (con>0 && ref>0){
            con = con - ref; 
            if(TMath::Abs(con)<0.0001f)
              con = .0001f;
            if(TMath::Abs(con)>m_plotOpts->m_diffcut && !m_plotOpts->m_showcutpix)
              con=-1000;
          } else con=-1000;
        } else con=-1000;
        break;
      case GRAT:
      case NRAT:
      case TRAT:
      case MINTRAT:
        if(ref!=0)        con /= ref;
        else                con = 0;
        break;
      case GDIF:
      case NDIF:
      case TDIF:
      case MINTDIF:
        if(ref!=0&&con!=0) con -= ref;
        else                   con = -99999;
        break;
      case GSQDIF:
      case NSQDIF:
        if(ref!=0 && con>ref)        con = sqrt(con*con-ref*ref);
        else                con = 0;
        break;
      case TIMEDIF:
        if(con>0 && ref>0)        con -= ref;
        else                con = 0;
        break;
      case TCALDIF:
        if(con>0 && ref>0)        con -= ref;
        else                con = 0;
        if(item->GetDID()!=0 && item->GetDID()->GetLog()!=0){
          int strg = item->GetDID()->GetLog()->MyModule->MCC->StrbDelrg;
          if(strg>=0 && strg<16)
            con *= item->GetDID()->GetLog()->MyModule->MCC->strobe_cal[strg];
        }
        break;
      case DELCAL:
        if(con>0 && ref>0 && (con-ref)!=0)
          con = 25/(con-ref);
        else
          con = 0;
        break;
      default:
        con = 0;
        break;
      }
      frmap->SetBinContent(bin,con);
    }
  }
  if(type==MINTDIF || type==MINTRAT)
    orgmap->Delete();
  return frmap;
}
int RMain::PlotBiasStab(std::vector<ModItem*> data_items){
  if(data_items.size()<=0) return -2;
  char line[2000];
  float time[100], curr[100], min_t=99999, corr, min_I=99999, max_I=0;
  int i,npts=0;
  int supp = m_plotOpts->m_BIASsupp;
  int chan = m_plotOpts->m_BIASchan;
  TLogFile *log=0;
  TH1F *bthi = (TH1F*) gROOT->FindObject("bthi");
  if(bthi!=0) bthi->Delete();
  const double k = 8.617342e-5, T20 = 293.15;
  double T;
  // get current and time stamps
  for(i=0;i<(int)data_items.size();i++){
    log = data_items[i]->GetDID()->GetLog();
    if(log!=0 && (int)log->MyDCS->HVvolt[supp][chan]>=m_plotOpts->m_VbiasMin &&
       (int)log->MyDCS->HVvolt[supp][chan]<=m_plotOpts->m_VbiasMax){
      curr[npts] = log->MyDCS->HVcurr[supp][chan];
      if(log->MyDCS->Tntc>-50 && m_plotOpts->m_IbiasCorr){
        T = 273.15+(double)log->MyDCS->Tntc;
        corr = (float)T20*T20*TMath::Exp(-1.21/2/k/T20)
          /(T*T*TMath::Exp(-1.21/2/k/T));
        curr[npts] *= corr;
      }
      if(curr[npts]<min_I) min_I = curr[npts];
      if(curr[npts]>max_I) max_I = curr[npts];
      QString mytime, mydate;
      bool isOK;
      TopWin::DateFromLog(log->comments.c_str(), mydate, mytime);
      time[npts] = mytime.right(2).toFloat(&isOK)/3600 + mytime.mid(3,2).toFloat(&isOK)/60 
                      +       mytime.left(2).toFloat(&isOK)
                      +    24*mydate.mid(3,2).toFloat(&isOK)
                      + 30*24*mydate.left(2).toFloat(&isOK);
      if(time[npts]<min_t) min_t = time[npts];
      npts++;
    }
  }
  if(npts==0) return -6;
  // create histogram and make time start at zero
  int nbin = npts/5;
  if(nbin<8) nbin=8;
  bthi = new TH1F("bthi","I_{bias} stability histogram",nbin,.6*min_I, 1.4*max_I);
  for(i=0;i<npts;i++){
    time[i] -= min_t;
    bthi->Fill(curr[i]);
  }
  // create graph out of above
  TGraph *btg = new TGraph(npts,time,curr);
  m_misc_obj.push_back((TObject*) btg);
  btg->SetMarkerStyle(20);
  btg->SetTitle("I_{bias} stability graph");
  // plot
  CheckCan();
  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  pplt->Divide(1,2,0.001f,0.001f);
  
  m_text->Clear();
  sprintf(line,"Bias current history");
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",data_items[0]->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  if(m_psfile!=NULL) m_psfile->NewPage();

  pplt->cd(1);
  gPad->SetLeftMargin(0.1f);
  btg->Draw("AP");
  btg->GetXaxis()->SetTitle("Time (h)");
  if(log->MyDCS->Tntc>-50 && m_plotOpts->m_IbiasCorr)
    btg->GetYaxis()->SetTitle("T-corrected bias current (#muA)");
  else
    btg->GetYaxis()->SetTitle("Bias current (#muA)");
  btg->GetXaxis()->SetTitleSize(.045f);
  btg->GetXaxis()->SetLabelSize(.045f);
  btg->GetYaxis()->SetTitleOffset(1.0f);
  btg->GetYaxis()->SetTitleSize(.045f);
  btg->GetYaxis()->SetLabelSize(.045f);
  pplt->cd(2);
  bthi->Draw();
  SetStyle(2,bthi);
  if(log->MyDCS->Tntc>-50 && m_plotOpts->m_IbiasCorr)
    bthi->GetXaxis()->SetTitle("T-corrected bias current (#muA)");
  else
    bthi->GetXaxis()->SetTitle("Bias current (#muA)");

  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}

TH2F* RMain::GetBadSelMap(TH2F *orgmap, int bit_mask, bool doAND){
  int binx, biny;
  float cont;
  char line[2000];

  TH2F *mm = new TH2F(*orgmap);
  mm->SetName("maskmask");
  sprintf(line,"%s - selected",mm->GetTitle());
  mm->SetTitle(line);
  mm->Reset();

  for(binx=1;binx<=mm->GetNbinsX();binx++){
    for(biny=1;biny<=mm->GetNbinsY();biny++){
      cont = orgmap->GetBinContent(binx,biny);
      if(doAND)
        cont = (float)(((int)cont  & bit_mask) == bit_mask);
      else
        cont = (float)((int)cont & bit_mask);
      if(cont>0){
        mm->SetBinContent(binx,biny,1);
      }
    }
  }
  mm->SetEntries(100);
  mm->SetMaximum(20);
  return mm;
}
int RMain::nBadPixs(ModItem *item, bool show){
  int chip, col, row;
  
  BadPixel *badpix = 0;
  if(show){
    QString toptitle = "List of digitally ineff. pixels, data set \"";
    toptitle += item->GetDataItem()->GetName();
    toptitle += "\", module ";
    toptitle += item->ModParent()->text(0);
    badpix = new BadPixel(item,NULL,"badpix",FALSE,Qt::WDestructiveClose);
    badpix->setCaption("Digital bad pixel summary");
    badpix->Init("...",toptitle.latin1());
    // do non-standard settings by hand
    badpix->PixelList->setNumCols(2);
    badpix->PixelList->horizontalHeader()->setLabel(1,"bad pixels");
    badpix->PixelList->setNumRows(17);
    badpix->MaskButt->hide();
  }

  if(item==0) return -1;
  if(item->GetDID()==0) return -2;
  if(item->GetDID()->GetLog()==0) return -4;
  int cont, nbad=0, nhits = item->GetDID()->GetLog()->MyScan->Nevents;
  TH2F *mymap = item->GetDID()->GetMap(-1,RAW);

  int i,j;

  if(item->GetDID()->Get_FitResArray()==0)
    item->GetDID()->Init_FitResArray();
  if(mymap!=0){
    item->GetDID()->Init_FitResArray(0);
    for(i=1;i<mymap->GetNbinsX()+1;i++){
      for(j=1;j<mymap->GetNbinsY()+1;j++){
        cont = (int) mymap->GetBinContent(i,j);
        if(cont!=nhits){
          nbad++;
	  item->GetDataItem()->PixXYtoCCR(i-1, j-1, &chip, &col, &row);
          item->GetDID()->Increment_FitResArrayCell(4, chip);        
          item->GetDID()->Increment_FitResArrayCell(4, 2);  //crgg 2 was 16
        }
      }
    }
  } else
    nbad = (int)item->GetDID()->Get_FitResArrayCell(4,2);//crgg 2 was 16

  if(show){
    for(chip=0;chip<NCHIP+1;chip++){
      if(chip<NCHIP)
        badpix->PixelList->setText(chip,0,QString::number(chip));
      else
        badpix->PixelList->setText(chip,0,"module");
      badpix->PixelList->setText(chip,1,QString::number((int)item->GetDID()->Get_FitResArrayCell(4,chip)));
    }
    badpix->show();
  }

  return nbad;
}

TH2F* RMain::MapMinThresh(PixelDataContainer *did, int chip){
  TH2F **maps = AllMapsMinThresh(did,chip);
  TH2F *thresh = maps[0];
  delete[] maps;
  return thresh;
}
TH2F** RMain::AllMapsMinThresh(PixelDataContainer *did, int chip){

  TH2F **maps = new TH2F*[3];
  maps[0] = 0;
  maps[1] = 0;
  maps[2] = 0;
  if(did==0) return maps;

  float chi_max = m_plotOpts->m_chiCutmT;
  float nse_max = 1e5;
  float nse_max_normal = (float)m_plotOpts->m_nseCutmT;
  float nse_max_long   = (float)m_plotOpts->m_nseCutmT_long;
  float nse_max_ganged = (float)m_plotOpts->m_nseCutmT_ganged;
  float nse_max_inter  = (float)m_plotOpts->m_nseCutmT_inter;
  int i,j, scanpt, mychip, col, row;
  float con,chi,noise, old_min, tdac_val=0;

  float step = did->getScanStop(1)-did->getScanStart(1);
  if(did->getScanSteps(1)>1)
    step /= (float)(did->getScanSteps(1)-1);
  else
    return maps;

  TH2F *chi_map=0, *nse_map=0, *thr_map = new TH2F(*(did->Get2DMap(chip,TWODMEAN,0)));
  TH2F *tdac_map=0, *tdac_log=did->GetMap(chip,TDACLOG);
  if(thr_map == 0) return maps;

  gROOT->cd(); // create histos in memory
  maps[0] = (TH2F*) gROOT->FindObject("min_tmap");
  if(maps[0]!=0) maps[0]->Delete();
  maps[0] = new TH2F(*thr_map);
  maps[0]->SetName("min_tmap");
  maps[0]->SetTitle("Map of minimum threshold");
  maps[0]->Reset();

  maps[1] = (TH2F*) gROOT->FindObject("min_nmap");
  if(maps[1]!=0) maps[1]->Delete();
  maps[1] = new TH2F(*thr_map);
  maps[1]->SetName("min_nmap");
  maps[1]->SetTitle("Map of noise at minimum threshold");
  maps[1]->Reset();

  maps[2] = (TH2F*) gROOT->FindObject("min_cmap");
  if(maps[2]!=0) maps[2]->Delete();
  maps[2] = new TH2F(*thr_map);
  maps[2]->SetName("min_cmap");
  maps[2]->SetTitle("Map of chi^2 at minimum threshold");
  maps[2]->Reset();

  tdac_map = new TH2F(*thr_map);
  tdac_map->SetName("min_tdmap");
  tdac_map->SetTitle("Map of TDAC at minimum threshold");
  tdac_map->Reset();

  for(scanpt = 0; scanpt < did->getScanSteps(1); scanpt++){
    if(thr_map!=0) thr_map->Delete();
    if(nse_map!=0) nse_map->Delete();
    if(chi_map!=0) chi_map->Delete();
    thr_map = new TH2F(*(did->Get2DMap(chip,TWODMEAN,scanpt)));
    nse_map = new TH2F(*(did->Get2DMap(chip,TWODSIG,scanpt)));
    chi_map = new TH2F(*(did->Get2DMap(chip,TWODCHI,scanpt)));
    if(thr_map!=0){
      for(i=1;i<thr_map->GetNbinsX()+1;i++){
        for(j=1;j<thr_map->GetNbinsY()+1;j++){
          con     = thr_map->GetBinContent(thr_map->GetBin(i,j));
          chi     = chi_map->GetBinContent(chi_map->GetBin(i,j));
          noise   = nse_map->GetBinContent(nse_map->GetBin(i,j));
          old_min = maps[0]->GetBinContent(maps[0]->GetBin(i,j));
	  if(tdac_log!=0)
	    tdac_val= tdac_log->GetBinContent(maps[0]->GetBin(i,j))
	      +did->getScanStart(1)+((float)scanpt*(did->getScanStop(1)-did->getScanStart(1)))
	      /did->getScanSteps(1);
	  did->PixXYtoCCR(i-1,j-1,&mychip,&col,&row);
	  switch(did->Pixel_Type(col,row)){
	  case 2: //ganged pixel
	  case 3: //long ganged pixel	
	    nse_max = nse_max_ganged;
	    break;
	  case 5: //inter-ganged pixel
	    nse_max = nse_max_inter;
	    break;
	  case 1: //long pixel
	    nse_max = nse_max_long; 
	    break;
	  default:
	  case 0: // normal
	    nse_max = nse_max_normal;
	  }
          if((chi<chi_max || chi_max<0) && chi>=0 && 
              (noise<nse_max || nse_max<0) && 
             (old_min==0 || (con!=0 && con<old_min))){
            maps[0]->SetBinContent(maps[0]->GetBin(i,j),con);
            maps[1]->SetBinContent(maps[1]->GetBin(i,j),noise);
            maps[2]->SetBinContent(maps[2]->GetBin(i,j),chi);
            tdac_map->SetBinContent(maps[0]->GetBin(i,j),tdac_val);
          }
        }
      }
    }
  }
  if(thr_map!=0) thr_map->Delete();
  if(nse_map!=0) nse_map->Delete();
  if(chi_map!=0) chi_map->Delete();

  DatSet *dsit = dynamic_cast<DatSet*>(did);
  if(dsit!=0)
    dsit->SetDAC(tdac_map);
  tdac_map->Delete();

  return maps;
}
int RMain::PlotMinThresh(ModItem *item, int chip, int type){

  TH2F *min_map=0, **maps;

  maps = AllMapsMinThresh(item->GetDataItem(), chip);

  switch(type){
  case CHI:
    min_map = maps[2];
    break;
  case SIG:
    min_map = maps[1];
    break;
  case MEAN:
    min_map = maps[0];
    break;
  default:
    min_map = 0;
  }

  delete[] maps;

  if(min_map == 0) return -2;

  if(item->GetDataItem()->getScanSteps(1)<=1)
    return -3;

  int myerr = PlotMapProjScat(item, chip, 0, min_map);

  TPad *ptxt = (TPad*) gROOT->FindObject("ptxt");
  if(ptxt!=0){
    float chi_max=(float)m_plotOpts->m_chiCutmT;
    float nse_max_normal = (float)m_plotOpts->m_nseCutmT;
    float nse_max_long   = (float)m_plotOpts->m_nseCutmT_long;
    float nse_max_ganged = (float)m_plotOpts->m_nseCutmT_ganged;
    float nse_max_inter  = (float)m_plotOpts->m_nseCutmT_inter;
    ptxt->cd();
    char extra_txt[300]={'\0'};
    if(chi_max>=0)
      sprintf(extra_txt,"chi^{2} cut: %.1f  ",chi_max);
    if(nse_max_normal>=0)
      sprintf(extra_txt,"%snoise cuts (norm./long/ganged/inter-g.): %.1f %.1f %.1f %.1f",
	      extra_txt, nse_max_normal, nse_max_long, nse_max_ganged, nse_max_inter);
    if(chi_max>=0 || nse_max_normal>=0){
      m_text->AddText(extra_txt);
      m_text->Draw();
      m_canvas->Modified();
      m_canvas->Update();
    }
  }
  return myerr;
}

int RMain::PlotLvl1(ModItem *item, int chip, int scanpt){

  char line[1024];
  float ylow=0;
  int i;
  //int binx,biny;

  TH1F *lvl1hi=0;
  TH2F *tmphi, *lvl1scan=0;
  MATH2F *rawmap;

  CheckCan();

  int nCol, nRow, nFe;
  item->GetDataItem()->getGeom(nRow, nCol, nFe);

  // get 2D map and make it clickable

  tmphi = item->GetDataItem()->GetMap(chip,RAW);
  if(tmphi==0) return -2;

  if(tmphi->GetNbinsY()%336==0) flipHistoCont(tmphi);
  rawmap = new MATH2F(*((MATH2F*) tmphi));
  if(rawmap->GetEntries()<=0) rawmap->SetEntries(100);
  m_currmap = rawmap;

  // get LVL1 values for selected pixel

  if(scanpt>=0)
    lvl1hi = item->GetDID()->GetLvl1(m_chip, m_col, m_row, scanpt);
  else
    lvl1scan = item->GetDID()->GetLvl1Scan(m_chip, m_col, m_row);

  // plotting

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *spplt=0;
  if(((lvl1hi!=0 && lvl1hi->GetEntries()>0) || (lvl1scan!=0 && lvl1scan->GetEntries()>0) ) 
     && m_plotOpts->m_plotfitopt==0){
    spplt = new TPad("spplt","Sub-plot pad",0,0,1,.45);
    spplt->Draw();
    ylow = 0.45f;
  }
  TPad *pplt = new TPad("pplt","Plot pad",0,ylow,1,.94);
  pplt->Draw();
  
  if(m_psfile!=0) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd();
  if(m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt==1) {
    int nsteps=1;
    if(item->GetDID()->m_isscan && item->GetDID()->GetLog()!=0)
      nsteps = item->GetDID()->GetLog()->MyScan->Inner->Step;
    if(item->GetDID()->GetLog()!=0 && (i=nsteps*(item->GetDID()->GetLog()->MyScan->Nevents-5)) < rawmap->GetMinimum())
      rawmap->SetMinimum((float)i);
    if(item->GetDID()->GetLog()!=0 && (i=nsteps*(item->GetDID()->GetLog()->MyScan->Nevents+5)) > rawmap->GetMaximum())
      rawmap->SetMaximum((float)i);
    rawmap->Draw("COLZ");
    if(m_plotOpts->m_doBlackBg){
      m_canvas->Modified();
      m_canvas->Update();
      ((TFrame*)gROOT->FindObject("TFrame"))->SetFillColor(1);
    }
    if(m_plotOpts->m_plotfitopt==0)
      SetStyle(2);
    else
      SetStyle(1);
    if(chip<0) ModuleGrid(rawmap, nFe);
    rawmap->GetXaxis()->SetTitle("Column");
    rawmap->GetYaxis()->SetTitle("Row");
  }

  if(((lvl1hi!=0 && lvl1hi->GetEntries()>0) || (lvl1scan!=0 && lvl1scan->GetEntries()>0)) && 
     (m_plotOpts->m_plotfitopt==0 || m_plotOpts->m_plotfitopt>1)){
    if(m_plotOpts->m_plotfitopt==0){
      spplt->cd();
      SetStyle(0);
    } else
      SetStyle(1);
    gPad->SetTopMargin(0.15f);
    gPad->SetBottomMargin(0.15f);
    if(lvl1hi!=0){
      lvl1hi->Draw();
      lvl1hi->GetXaxis()->SetTitle("LVL1 ID");
      lvl1hi->GetYaxis()->SetTitle("Hits");
    } else if(lvl1scan!=0){
      lvl1scan->Draw("colz");
      if(item->GetDID()->GetLog()!=0)
	lvl1scan->GetXaxis()->SetTitle(item->GetDID()->GetLog()->MyScan->GetInner().c_str());
      else
	lvl1scan->GetXaxis()->SetTitle("scan par.");
      lvl1scan->GetYaxis()->SetTitle("LVL1 ID");
    }

    if(m_plotOpts->m_plotfitopt==0) TitleResize(1.5);
    else                            TitleResize(0.7f);
  }
  
  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}
int RMain::PlotHisto(ModItem *item, int scanpt, int ps_level, TH1F *user_hi){
  char line[1000];

  int type = item->GetPID(), ps_type=-1;
  TH1F *tmphi=user_hi;

  if(tmphi==0){
  if(type>=PIXDBTYPES)
#ifdef PIXEL_ROD // can link to PixScan
      ps_type = item->GetPID()-PIXDBTYPES;
      PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
      if(pdbd!=0)
	tmphi = dynamic_cast<TH1F*>(pdbd->getGenericHisto((PixLib::PixScan::HistogramType)ps_type,
							  m_chip, m_col, m_row, scanpt, ps_level));
#endif
  }

  if(tmphi==0) return -2;

  // plotting

  CheckCan();

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  
  if(m_psfile!=0) m_psfile->NewPage();

  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  pplt->cd();
  SetStyle(0);
  tmphi->Draw();
  TitleResize(0.7f);
  if(ps_type>=0 && ps_type<NPLHIMAX && m_plotOpts->m_PLhistoExists[ps_type] &&
     m_plotOpts->m_PLUsrIUse[ps_type])
    tmphi->GetXaxis()->SetRangeUser(m_plotOpts->m_PLUsrIMin[ps_type],m_plotOpts->m_PLUsrIMax[ps_type]);

  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}

/***************************************************************************/
// (added 27/4/04 by LT)
static int BI_plotMultiGraph(std::vector<TObject*> &m_misc_obj, const char *moduleId, TGraph *gr1, TGraph *gr2, TGraph *gr3=NULL, 
                                        TGraph *gr4=NULL, int markerStyle=7, const char drawOption[]="APL", bool axisXisTime=true, 
                                        bool temporaryGraphs=false, const char *mgTitle=NULL, const char *xLabel=NULL, const char *yLabel=NULL){

        if((gr1==NULL)&&(gr2==NULL)&&(gr3==NULL)&&(gr4==NULL)) return(0);

        TMultiGraph *mg = new TMultiGraph();
        TLegend *leg = new TLegend((float)0.87,(float)0.4,(float)0.99,(float)0.6);
        m_misc_obj.push_back((TObject*) leg);

        if(gr1!=NULL){
                if(temporaryGraphs) m_misc_obj.push_back((TObject*) gr1);
                gr1->SetLineColor(2);
                gr1->SetMarkerStyle(markerStyle);
                gr1->SetMarkerColor(2);        
                mg->Add(gr1);
                leg->AddEntry(gr1,gr1->GetTitle(),"PL"); 
        }

        if(gr2!=NULL){
                if(temporaryGraphs) m_misc_obj.push_back((TObject*) gr2);
                gr2->SetLineColor(4);
                gr2->SetMarkerStyle(markerStyle);
                gr2->SetMarkerColor(4);
                mg->Add(gr2);
                leg->AddEntry(gr2,gr2->GetTitle(),"PL"); 
        }

        if(gr3!=NULL){
                if(temporaryGraphs) m_misc_obj.push_back((TObject*) gr3);
                gr3->SetLineColor(3);
                gr3->SetMarkerStyle(markerStyle);
                gr3->SetMarkerColor(3);
                mg->Add(gr3);
                leg->AddEntry(gr3,gr3->GetTitle(),"PL");
        }

        if(gr4!=NULL){
                if(temporaryGraphs) m_misc_obj.push_back((TObject*) gr4);
                gr4->SetLineColor(5);
                gr4->SetMarkerStyle(markerStyle);
                gr4->SetMarkerColor(5);
                mg->Add(gr4);
                leg->AddEntry(gr4,gr4->GetTitle(),"PL");
        }
  
        if((gr1!=NULL)||(gr2!=NULL)||(gr3!=NULL)||(gr4!=NULL)){
        
                mg->Draw(drawOption);
                char str[100];
                strcpy(str, moduleId);
                strcat(str, ": ");
                if(mgTitle==NULL){
                        if(gr1!=NULL) strcat(str, gr1->GetTitle());                        
                        if(gr2!=NULL){
                                strcat(str, " & ");
                                strcat(str, gr2->GetTitle());
                        }
                        if(gr3!=NULL){
                                strcat(str, " & ");
                                strcat(str, gr3->GetTitle());
                        }
                        if(gr4!=NULL){
                                strcat(str, " & ");
                                strcat(str, gr4->GetTitle());
                        }
                        mg->SetTitle(str);
                }else mg->SetTitle(strcat(str, mgTitle));
                
                TAxis *axis=mg->GetXaxis();
                if(xLabel==NULL){
                        if(gr1!=NULL) axis->SetTitle(gr1->GetXaxis()->GetTitle());
                        else axis->SetTitle(gr2->GetXaxis()->GetTitle());
                }else axis->SetTitle(xLabel);
                axis->SetTitleSize((float)0.04);
                axis->CenterTitle();
                if(axisXisTime){
                        axis->SetTimeDisplay(1);
                        axis->SetTimeFormat("%H:%M%F1970-01-0100:00:00");
                }
                axis->SetLabelSize((float)0.04);                                

                axis=mg->GetYaxis();
                if(yLabel==NULL){
                        str[0]=0;
                        if(gr1!=NULL) strcpy(str, gr1->GetYaxis()->GetTitle());
                        if(gr2!=NULL){
                                strcat(str, ", ");
                                strcat(str, gr2->GetYaxis()->GetTitle());
                        }
                        if(gr3!=NULL){
                                strcat(str, ", ");
                                strcat(str, gr3->GetYaxis()->GetTitle());
                        }
                        if(gr4!=NULL){
                                strcat(str, ", ");
                                strcat(str, gr4->GetYaxis()->GetTitle());
                        }
                        axis->SetTitle(str);
                }else axis->SetTitle(yLabel);
                axis->SetTitleSize((float)0.04);
                axis->CenterTitle();
                axis->SetLabelSize((float)0.04);                                                
                leg->Draw();
  }        

  return(0);

} 
/***************************************************************************/
// burn-in scan summary (added 7/5/04 by LT)
int RMain::BI_getScanSummary(ModItem *item){

        int status;
        ModItem* child;
        const int vecSize=1000;
        double thrVec[vecSize], noiseVec[vecSize], timeThrVec[vecSize], timeDigVec[vecSize], 
                        digErrorsVec[vecSize], nfitsVec[vecSize], mccErrVec[vecSize],
                        mcc01ErrVec[vecSize], feErrVec[vecSize];

        char str[100];
        char str2[100];
        double time=0, avgNtc;
        QString theDate, theTime;

        //get data
        int thr_index=0;
        int dig_index=0;
        child =(ModItem*) item->ModParent()->firstChild();

        while(child) { // search the module entries

                const char* nam=child->GetDID()->GetName();                
                Int_t dataType=child->GetDID()->m_istype;

                if((dataType==ONEDSCAN)||(dataType==DIG)){
                        // get time of scan        
                        const char* cm=child->GetDID()->GetLog()->comments.c_str();        
                        TopWin::DateFromLog(cm, theDate, theTime);
                        time=TopWin::BI_getTimeFromStrings(theDate, theTime);
                }

                if(dataType==ONEDSCAN){ // threshold scan
                        
                        avgNtc=item->GetDID()->BI_getAvgValueOnInterval(BI_NTC, time, time+300);
                        double noise, nfits;
                        status=child->GetDID()->BI_getMeanValue(SIG, &noise, &nfits); //get noise
                        noiseVec[thr_index]=noise*10;
                        status=child->GetDID()->BI_getMeanValue(MEAN, &thrVec[thr_index], &nfits); //get threshold
                        timeThrVec[thr_index]=time;
                        nfitsVec[thr_index]=nfits/10;

                        sprintf(str, " (ntc= %+2.1f C, thr= %0.0f e, noise= %0.0f e, nfits= %0.0f, %s %s)", 
                                avgNtc, thrVec[thr_index], noise, nfits, theDate.latin1(), theTime.latin1());
                        strcpy(str2, nam);
                        strcat(str2, str);
                        child->setText(0,str2);
                
                        thr_index++;
                        if(thr_index>=vecSize) return(-1);
                                        
                }else if(dataType==DIG){  // digital scan
                        
                        avgNtc=item->GetDID()->BI_getAvgValueOnInterval(BI_NTC, time-30, time+90);
                        timeDigVec[dig_index]=time;

                        digErrorsVec[dig_index]= (double)nBadPixs(child);

                        // get sum of log file errors (see LogDisp::DisplayErr())                        
                        TErrorCnt    *MyErr=child->GetDID()->GetLog()->MyErr;
                        
                        int mccErr=0;
                        int mcc01Err=0;
                        int feErr=0;

                        for (int Chip = 0; Chip < 16; Chip++){
                                feErr+=MyErr->buff_ovfl[Chip];
                                feErr+=MyErr->illg_col[Chip];
                                feErr+=MyErr->illg_row[Chip];
                                feErr+=MyErr->miss_eoe[Chip];
                                feErr+=MyErr->unseq_lv1[Chip];
                                feErr+=MyErr->unseq_bcid[Chip];
                                feErr+=MyErr->unm_bcid[Chip];
                                feErr+=MyErr->hamming_err[Chip];
                                feErr+=MyErr->par_ups[Chip];
                                feErr+=MyErr->par_flags[Chip];
                                feErr+=MyErr->par_hits[Chip];
                                feErr+=MyErr->par_eoe[Chip];
                                feErr+=MyErr->bitflip[Chip];
                                feErr+=MyErr->mcc_hovfl[Chip];
                                feErr+=MyErr->mcc_eovfl[Chip];
                                feErr+=MyErr->mcc_bcin[Chip];
                                feErr+=MyErr->mcc_bcbw[Chip];
                                feErr+=MyErr->mcc_wfe[Chip];
                                feErr+=MyErr->mcc_wmcc[Chip];
                        }
                        mccErr+=MyErr->mcc_lvbf;
                        mccErr+=MyErr->mcc_badfc;
                        mccErr+=MyErr->mcc_badsc;
                        mcc01Err+=MyErr->mcc_err0;
                        mcc01Err+=MyErr->mcc_err1;

                        mccErrVec[dig_index]= (double)mccErr;
                        mcc01ErrVec[dig_index]= (double)mcc01Err;
                        feErrVec[dig_index]= (double)feErr;
                        
                        sprintf(str, " (ntc= %+2.1f C, badPixls= %0.0f, mccErr= %0.0f, %s %s)", avgNtc, digErrorsVec[dig_index], mccErrVec[dig_index], theDate.latin1(), theTime.latin1());
                        strcpy(str2, nam);
                        strcat(str2, str);
                        child->setText(0,str2);
                        
                        dig_index++;
                        if(dig_index>=vecSize) return(-1);
                                                
                }

                child = (ModItem*)child->nextSibling();
        }

    TGraph *thrGr=NULL;
        TGraph *noiseGr=NULL;
        TGraph *nfitsGr=NULL;
        TGraph *digErrGr=NULL;
        TGraph *mccErrGr=NULL;
        TGraph *mcc01ErrGr=NULL;
        TGraph *feErrGr=NULL;

        if(thr_index>0){
                // threshold vs time
                thrGr=new TGraph(thr_index, timeThrVec, thrVec);
                // noise vs time
                noiseGr=new TGraph(thr_index, timeThrVec, noiseVec);
                // pixels with good fit vs time
                nfitsGr=new TGraph(thr_index, timeThrVec, nfitsVec);
        }

        if(dig_index>0){
                // dig errors vs time
                digErrGr=new TGraph(dig_index, timeDigVec, digErrorsVec);
                // log errors vs time
                mccErrGr=new TGraph(dig_index, timeDigVec, mccErrVec);
                mcc01ErrGr=new TGraph(dig_index, timeDigVec, mcc01ErrVec);
                feErrGr=new TGraph(dig_index, timeDigVec, feErrVec);
        }

        //update root data
        status=item->GetDID()->BI_updateScanSummary(thrGr, noiseGr, nfitsGr, digErrGr, mccErrGr, mcc01ErrGr, feErrGr);

        return(0);
}
/********************************************************************/
// Plots summary of burn-in scans (added 27/4/04 by LT)
int RMain::PlotBI_scans(ModItem *item){


        int status=BI_getScanSummary(item);
        DatSet* datSet=item->GetDID();

        // plot data
        CheckCan();
        m_canvas->Clear();
        const int numMg=3;
        m_canvas->Divide(1, numMg, 0.001f, 0.001f);

        const char *moduleId=item->ModParent()->text(0).latin1();

        //plot threshold and noise vs Time
        m_canvas->cd(1);
        TGraph *gr1=datSet->GetGraph(BI_THRESHOLD);
        TGraph *gr2=datSet->GetGraph(BI_NOISE);
        TGraph *gr3=datSet->GetGraph(BI_GOODFITS);
        status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2, gr3, NULL, 22, "AP", true, false, "Threshold & Noise & NFits", NULL, "");
    
        //plot threshold and noise vs NTC
        m_canvas->cd(2);
        
        if((gr1!=NULL)&&(gr2!=NULL)&&(gr3!=NULL)){
                
                int numPoints=gr1->GetN();
                TGraph *g1=gr1;
                TGraph *g2=gr2;
                TGraph *g3=gr3;
                gr1=new TGraph(numPoints);
                gr2=new TGraph(numPoints);
                gr3=new TGraph(numPoints);                
                
                for(int i=0; i<numPoints; i++){
                        double time=g1->GetX()[i];
                        double ntc=datSet->BI_getAvgValueOnInterval(BI_NTC, time, time+300);
                        gr1->SetPoint(i, ntc, g1->GetY()[i]);
                        gr2->SetPoint(i, ntc, g2->GetY()[i]);
                        gr3->SetPoint(i, ntc, g3->GetY()[i]);
                }
                gr1->SetTitle("Threshold");
                gr2->SetTitle("Noise*10");
                gr3->SetTitle("NumFits/10");
                status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2, gr3, NULL, 22, "AP", false, true, "Threshold & Noise & NFits", "NTC Temperature (C)", "");
        }
  
        //plot dig. errors vs Time
        m_canvas->cd(3);
        gr1=datSet->GetGraph(BI_BADPIXLS);
        gr2=datSet->GetGraph(BI_LOGERR_MCC);
        gr3=datSet->GetGraph(BI_LOGERR_MCC01);
        TGraph *gr4=datSet->GetGraph(BI_LOGERR_FE);
        status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2, gr3, gr4, 22, "AP", true, false, "Digital Scan Errors", NULL, "");

        m_canvas->Update();
        m_canvas->Modified();

        return(0);

}

/***************************************************************************/
// Plots burn-in V,I, T vs Time (added 27/4/04 by LT)
int RMain::PlotBI_vitVsTime(ModItem *item){

        int status, i;
        TGraph *gr1;
        TGraph *gr2;

        CheckCan();
        m_canvas->Clear();
        const int numMg=4;
        m_canvas->Divide(1, numMg, 0.001f, 0.001f);

        const char *moduleId=item->ModParent()->text(0).latin1();

        for(i=0; i<3; i++){
                m_canvas->cd(i+1);
                gr1=item->GetDID()->GetGraph(2*i);
                gr2=item->GetDID()->GetGraph(2*i+1);
                status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2);
        }
        
        //bias
        m_canvas->cd(4);
        gr1=item->GetDID()->GetGraph(BI_BIAS_VOLT);
        TGraph *g=item->GetDID()->GetGraph(BI_BIAS_CURR);
        if((gr1!=NULL)&&(g!=NULL)){
                gr2= new TGraph(g->GetN());
                gr2->SetTitle("Bias Current");
                m_misc_obj.push_back((TObject*) gr2);
                //rescale to microAmps
                double x, y;
                for(i=0; i<g->GetN(); i++){
                        g->GetPoint(i, x, y);
                        gr2->SetPoint(i, x, y*1e+6);
                }        
                status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2, NULL, NULL, 7, "APL", true, false, NULL, NULL, "Volts, MicroAmps");
        }

        m_canvas->Update();
        m_canvas->Modified();

        return(0);
}
/***************************************************************************/
// Plots burn-in currents vs NTC (added 27/4/04 by LT)
int RMain::PlotBI_currentVsTemp(ModItem *item){

        int status, i;
        double time, ntc;
        TGraph *gr1;
        TGraph *gr2;

        DatSet* datSet=item->GetDID();

        CheckCan();
        m_canvas->Clear();
        const int numMg=3;
        m_canvas->Divide(1, numMg, 0.001f, 0.001f);

        const char *moduleId=item->ModParent()->text(0).latin1();

        TGraph *gntc=datSet->GetGraph(BI_NTC);
        if(gntc==NULL) return(-1);
        int ntcPoints=gntc->GetN();
        double *ntcSet=gntc->GetY();

        //digital V,I vs NTC
        m_canvas->cd(1);
        TGraph *g1=datSet->GetGraph(BI_DIG_VOLT);
        TGraph *g2=datSet->GetGraph(BI_DIG_CURR);
        if((g1!=NULL)&&(g2!=NULL)){

                if((g1->GetN()==ntcPoints)&&(g2->GetN()==ntcPoints)){
                        gr1=new TGraph(ntcPoints, ntcSet, g1->GetY());
                        gr2=new TGraph(ntcPoints, ntcSet, g2->GetY());
                }else{
                        int numPoints=g1->GetN();
                        gr1=new TGraph(numPoints);
                        for(i=0; i<numPoints; i++){
                                time=g1->GetX()[i];
                                ntc=datSet->BI_getAvgValueOnInterval(BI_NTC, time-30, time+30);
                                gr1->SetPoint(i, ntc, g1->GetY()[i]);
                        }
                        numPoints=g2->GetN();
                        gr2=new TGraph(numPoints);
                        for(i=0; i<numPoints; i++){
                                time=g2->GetX()[i];
                                ntc=datSet->BI_getAvgValueOnInterval(BI_NTC, time-30, time+30);
                                gr2->SetPoint(i, ntc, g2->GetY()[i]);
                        }
                }
                gr1->SetTitle("Digital Voltage");
                gr2->SetTitle("Digital Current");
                status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2,  NULL, NULL, 7, "AP", false, true, NULL, "NTC Temperature (C)", "Volts, Amps");

        }
        
        //analog V,I vs NTC
        m_canvas->cd(2);
        g1=datSet->GetGraph(BI_ANALOG_VOLT);
        g2=datSet->GetGraph(BI_ANALOG_CURR);
        if((g1!=NULL)&&(g2!=NULL)){

                if((g1->GetN()==ntcPoints)&&(g2->GetN()==ntcPoints)){
                        gr1=new TGraph(ntcPoints, ntcSet, g1->GetY());
                        gr2=new TGraph(ntcPoints, ntcSet, g2->GetY());
                }else{
                        int numPoints=g1->GetN();
                        gr1=new TGraph(numPoints);
                        for(i=0; i<numPoints; i++){
                                time=g1->GetX()[i];
                                ntc=datSet->BI_getAvgValueOnInterval(BI_NTC, time-30, time+30);
                                gr1->SetPoint(i, ntc, g1->GetY()[i]);
                        }
                        numPoints=g2->GetN();
                        gr2=new TGraph(numPoints);
                        for(i=0; i<numPoints; i++){
                                time=g2->GetX()[i];
                                ntc=datSet->BI_getAvgValueOnInterval(BI_NTC, time-30, time+30);
                                gr2->SetPoint(i, ntc, g2->GetY()[i]);
                        }
                }
                gr1->SetTitle("Analog Voltage");
                gr2->SetTitle("Analog Current");
                status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, gr2,  NULL, NULL, 7, "AP", false, true, NULL, "NTC Temperature (C)", "Volts, Amps");

        }

        //bias I vs NTC
        m_canvas->cd(3);
        g1=datSet->GetGraph(BI_BIAS_CURR);
        if(g1!=NULL){
                int numPoints=g1->GetN();
                gr1=new TGraph(numPoints);
                for(int i=0; i<numPoints; i++){
                        time=g1->GetX()[i];
                        ntc=datSet->BI_getAvgValueOnInterval(BI_NTC, time-30, time+30);
                        gr1->SetPoint(i, ntc, g1->GetY()[i]);
                }                        
                gr1->SetTitle("Bias Current");
                status=BI_plotMultiGraph(m_misc_obj, moduleId, gr1, NULL,  NULL, NULL, 7, "AP", false, true, NULL, "NTC Temperature (C)", "Amps");
        }
        
        m_canvas->Update();
        m_canvas->Modified();

        return(0);
}
/***************************************************************************/

//*************** by JW *******************************************************************
int RMain::GetProjection(ModItem *item, TH2F *user_hi, TH1F **projection, BadPixel *badpixtab){
  return GetProjection(item->GetPID(), item->GetDID(), user_hi, projection, badpixtab);
}
int RMain::GetProjection(int type, DatSet *data, TH2F *user_hi, TH1F **projection, BadPixel *badpixtab){		
	// crgg where is TH1F **projection coming from?
  TH2F *tmphi;
  Float_t *x, *y;
  Int_t i,j, k, bin, col, row, nbins=100;
  QString toptitle;
  Float_t mnmin=100000, mnmax=0, mnmean=0, mnrms=0, con;
  Bool_t takeneg   = type==CHI || type==MLPLOT || type==LEAK_PLOT || type==TDACP || type==FDACP ||
                     type==TDACLOG || type==FDACLOG || (type>=PARA && type<=PARE) || type==TOT_MAX;
  Bool_t issfit    = type==MEAN || type==SIG || type==CHI || type==TWODMEAN || type==TWODSIG || type==TWODCHI;
  Bool_t totpeak   = type==TOT_SUMM || type==CAL_SUMM || type==TOT_FAST || type==CAL_FAST;

  if(data==0) return -1;

// create bad pixel table if needed

  if (badpixtab != NULL)
  {
        if ( type==CHI )
        {
                toptitle = "Fit failed";
                badpixtab->Init("chi^2",toptitle.latin1());
        }
        else if ( type == TDACLOG || type==TDACP ) 
        {
                toptitle = "TDAC=0,max pixel list";
                badpixtab->Init("TDAC",toptitle.latin1());
        }
        else if ( type == FDACLOG || type==FDACP ) 
        {
                toptitle = "FDAC=0,max pixel list";
                badpixtab->Init("FDAC",toptitle.latin1());
        }
  }

// get 2D map

  if(user_hi!=0)
    tmphi = user_hi;
  else return -2;

// check FE-flavor and set TDAC-range
 
  if(data->GetLog()!=NULL && data->GetLog()->MyModule->FEflavour==1){
    if(type==FDACP || type==FDACLOG) m_MapInfo->dacmax = 7;
    else                             m_MapInfo->dacmax = 127;
  }else{
    if(tmphi->GetMaximum()>31)                m_MapInfo->dacmax=127;
    else if(tmphi->GetMaximum()<8)        m_MapInfo->dacmax=7;
  }

  if(type==FDACP) type = TDACP;     // only needed different ID for getting the map
  if(type==FDACLOG) type = TDACLOG; // only needed different ID for getting the map

// get 1D-histogram
 
  int nCol, nRow, nFe;
  data->getGeom(nRow, nCol, nFe);
  x = new Float_t[nFe*nCol*nRow];
  y = new Float_t[nFe*nCol*nRow];

  m_MapInfo->nfitted = 0;
  for(i=1;i<tmphi->GetNbinsX()+1;i++){
    for(j=1;j<tmphi->GetNbinsY()+1;j++){
      bin = data->PixXYtoInd(i-1,j-1);
      con = tmphi->GetBinContent(tmphi->GetBin(i,j));
      if(totpeak && data->GetMap(-1,TOT_HIT)->GetBinContent(tmphi->GetBin(i,j))<m_plotOpts->m_peakMinHit)
                con = 0;
    // fill bad pixel table if required
      if ( badpixtab!=NULL && type==CHI && con<=0 && m_plotOpts->m_noisetab ) 
                badpixtab->AddTabLine(i,j,con);
      if ( badpixtab!=NULL && (type==TDACLOG || type==TDACP) && (con==0 || con==m_MapInfo->dacmax) 
           && m_plotOpts->m_noisetab ) 
                badpixtab->AddTabLine(i,j,con);
    // process data for plots
      if(bin<(nFe*nCol*nRow) && bin>=0)
          {
                x[bin] = (Float_t) bin;
                y[bin] = con;
      }
          if(con>mnmax) mnmax = con;
      if(con<mnmin && (con>0 || takeneg)) mnmin = con;
      (m_MapInfo->ntot)++;
      if(con>0 || !issfit)
          {
                mnmean += con;
                mnrms  += con*con;
                (m_MapInfo->nfitted)++;
      }
      if(type==TDACP || type==TDACLOG)
          {
                if(con>m_MapInfo->dacmax-1) (m_MapInfo->ntdach)++;
                if(con<1)  (m_MapInfo->ntdacl)++;
      }
      if(type==LEAK_PLOT || type==MLPLOT) m_MapInfo->leaktot += (int)con;
    }
  }

  delete[] x;
  delete[] y;

  // set boundaries for TH1F

  if(mnmin==10000) mnmin=0;
  if(mnmax==0)     mnmax=10000;
  if(m_MapInfo->nfitted>0){
    mnmean /= (float) m_MapInfo->nfitted;
    mnrms   = mnrms/(float)m_MapInfo->nfitted - mnmean*mnmean;
    if(mnrms>0){
      mnrms = sqrt(mnrms);
      if(mnmax>mnmean+8*mnrms) mnmax = mnmean+8*mnrms;
      if(mnmin<mnmean-8*mnrms) mnmin = mnmean-8*mnrms;
    }
  }
  if(mnmax>0) mnmax *= 1.2f;
  else        mnmax *= 0.8f;

  if(mnmin>0)mnmin *= 0.8f;
  else       mnmin *= 1.2f;

//handle some special plots

  if(type==TDACP || type==TDACLOG){
    nbins = 1+(int)m_MapInfo->dacmax;
    mnmin = -.5;
    mnmax = m_MapInfo->dacmax +.5;
  }
  if(type==MLPLOT || type==LEAK_PLOT){
    mnmax *= 8;
    mnmin *= 8;
    nbins = (int)(mnmax-mnmin);
    mnmax = ((float)mnmax)/8;
    mnmin = ((float)mnmin)/8;
  }
  if(type<DSET && m_plotOpts->m_UsrIUse[type]){
    mnmin = m_plotOpts->m_UsrIMin[type];
    mnmax = m_plotOpts->m_UsrIMax[type];
  }
  if(m_plotOpts->m_doUsrRg){
    mnmin = m_plotOpts->m_UsrMin;
    mnmax = m_plotOpts->m_UsrMax;
  }

// create new TH1F
		// crgg projection[x] x should be changed
  projection[0] = new TH1F((m_plotOpts->hinam[type]+"-all").c_str(),m_plotOpts->hitit[type].c_str(),nbins,mnmin,mnmax);
  projection[1] = new TH1F((m_plotOpts->hinam[type]+"-norm").c_str(),m_plotOpts->hitit[type].c_str(),nbins,mnmin,mnmax);
  projection[2] = new TH1F((m_plotOpts->hinam[type]+"-gang").c_str(),m_plotOpts->hitit[type].c_str(),50,mnmin,mnmax);
  projection[3] = new TH1F((m_plotOpts->hinam[type]+"-intg").c_str(),m_plotOpts->hitit[type].c_str(),30,mnmin,mnmax);
  projection[4] = new TH1F((m_plotOpts->hinam[type]+"-long").c_str(),m_plotOpts->hitit[type].c_str(),30,mnmin,mnmax);

// fill 1D-histogram
  for(i=1;i<tmphi->GetNbinsX()+1;i++)
  {
    for(j=1;j<tmphi->GetNbinsY()+1;j++)
        {
      con = tmphi->GetBinContent(tmphi->GetBin(i,j));
      if(totpeak && data->GetMap(-1,TOT_HIT)->GetBinContent(tmphi->GetBin(i,j))<m_plotOpts->m_peakMinHit)
                con = 0;
      if(con>0 || takeneg)
          {
                data->PixXYtoCCR(i-1,j-1,&k,&col,&row);  // k ist wahrscheinlich die ChipNr
                if(row>999 && row%2!=0)                        //ganged pixel		// crgg this should be removed
                        projection[2]->Fill(con);
                else if(row>999)                               //interganged		// crgg this should be removed
                        projection[3]->Fill(con);
                else if(col==0 || col==17)
                        projection[4]->Fill(con);              //long
                else
                        projection[1]->Fill(con);              //normal
                projection[0]->Fill(con);                      //all
          }
          else
                projection[0]->Fill(con);
    }
  }
  
  return 0;
}

int RMain::PlotSingleProjection(ModItem *item, TPad *pplt, int Pixel_type, int chip, int scanpt){		
	// crgg Pixel_type has to be changed
  TH1F *mapproj[5];			// crgg this should be changed
  TH2F *map;
  ModItem *parent=item->ModParent();
  BadPixel *badpix = NULL;
  int type = item->GetPID();
  int i=0;
  const char* itlabel = item->GetDID()->GetName();
  const char* parlabel = parent->text(0);
  char dummy[] = " ", m[] = "M";
  char* label= new char[260];

  if(parent!=0){
        strcpy(label, m);
        strcat(label, parlabel);
        strcat(label, dummy);
        strcat(label, itlabel);
  } 
  else
        strcpy(label, itlabel);
  if(pplt==0)  return -1;
  if(type==T20K)
      map = TOT20K(chip,item->GetDID());
    else if(type>=TWODMEAN && type<=TWODOPT)
      map  = item->GetDID()->Get2DMap(chip,type,scanpt);
    else
      map  = item->GetDID()->GetMap(chip,type);
  
  GetProjection(item, map, mapproj, badpix);

  for(i=0; i<5; i++)
          mapproj[i]->SetMinimum(0.1f);


  mapproj[Pixel_type]->SetTitle(label);
  SetStyle(4, mapproj[Pixel_type]);
  mapproj[Pixel_type]->Draw();
//  for(i=0;i<5;i++)
//    delete mapproj[i];
  delete label;
  return 0;

}
int RMain::PlotMultipleProjections(ModItem **itarr, int nrProj,bool overlay, int Pixel_type, int chip, int scanpt){		
	// crgg Pixel_type has do be changed
  int i=0, nrrow=0, error=0;
  char line[2000];
  
  CheckCan();
  
  m_canvas->cd();
  m_canvas->Clear();
  if(!overlay)
    nrrow = (int)((float)(nrProj+1)/2 + 0.5);

  TPad *pdraw = new TPad("pdraw","Text pad",0,0,1,.94);
  pdraw->Draw();
  if(!overlay){
    if(nrProj>2)
      pdraw->Divide(2, nrrow,.001f,.001f);
    else
      pdraw->Divide(1, nrrow,.001f,.001f);
  }

  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  ptxt->cd();
  m_text->Clear();
  sprintf(line," ");
  sprintf(line,"Module %s \n",itarr[0]->ModParent()->text(0).latin1());
  for(i=0;i<nrProj;i++){
    if(itarr[i]->GetDID() != NULL){
      if(i<nrProj-1)
        sprintf(line,"%s%s + ",line,itarr[i]->GetDID()->GetName());
      else
        sprintf(line,"%s%s.",line,itarr[i]->GetDID()->GetName());
        }
  }
  m_text->AddText(line);
  m_text->Draw();

  if(!overlay)
    for(i=0;i<nrProj;i++)
        {
          pdraw->cd(i+1);
          error = PlotSingleProjection(itarr[i], (TPad*)gPad, Pixel_type, chip, scanpt);
        }
  else{
    pdraw->cd();
        OverlayHistos(itarr, nrProj, Pixel_type, chip, scanpt);
  }

  m_canvas->Modified();
  m_canvas->Update();
  return error;

}
int RMain::OverlayHistos(ModItem **itarr, int nrProj, int Pixel_type, int chip, int scanpt){	
	// crgg Pixel_type
  TH1F *mapproj[5];	// crgg this should be changed
  TH2F *map;
  THStack *stack = new THStack("m_stack", "Scan overlay");
  BadPixel *badpix = NULL;
  int type;
  int i=0, j=0;
  //  int error;
  short style=3001;
  const char* itlabel;
  const char* parlabel;
  char dummy[] = " ", m[] = "M";
  char* label= new char[260];
  TLegend *leg = NULL;

  if(nrProj>1)
    leg = new TLegend(.5f,.95f-.02f*(float)nrProj,.99f,.99f);
  for(i=0;i<nrProj;i++){
        type=itarr[i]->GetPID();
        if(type==T20K)
      map = TOT20K(chip,itarr[i]->GetDID());
    else if(type>=TWODMEAN && type<=TWODOPT)
      map  = itarr[i]->GetDID()->Get2DMap(chip,type,scanpt);
    else
      map  = itarr[i]->GetDID()->GetMap(chip,type);

        itlabel = itarr[i]->GetDID()->GetName();
        parlabel = itarr[i]->ModParent()->text(0);
        if(itarr[i]->parent()!=0){
      strcpy(label, m);
          strcat(label, parlabel);
          strcat(label, dummy);
          strcat(label, itlabel);
        } 
    else
          strcpy(label, itlabel);

        GetProjection(itarr[i], map, mapproj, badpix);
        style++;
        SetStyle(1);
        mapproj[Pixel_type]->SetTitle(label);
        mapproj[Pixel_type]->SetLineColor(i+2);
        stack->Add(mapproj[Pixel_type]);
        if(leg!=NULL)
          leg->AddEntry(mapproj[Pixel_type],label,"l");
        j++;
  }

  stack->Draw("nostack");
  stack->Print();
  if(leg!=NULL)
    leg->Draw();
  m_canvas->Modified();
  m_canvas->Update();

  return 0;
}

int RMain::PlotMultiHist(){

  CheckCan();
  m_canvas->cd();
  m_canvas->Clear();          

  TPad *pdraw = new TPad("pdraw","Draw pad",0,0,1,.94);
  pdraw->Draw();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  ptxt->cd();
  m_text->Clear();
  m_text->AddText("random text");
  m_text->Draw();

  pdraw->cd();
  return 0;
}

void RMain::PlotModuleSummary(std::map<int,std::vector<ModItem*> > itemmap, int summary_type, TopWin* tw, QString scanlabel){
  int chip, maxPos=-1, padnr, stavecount, stc=0, ij=0;
  bool stave=false;
  float oldMin=100000, oldMax=0, tempMin=100000, tempMax=0;
  TH2F* tempHist;
  MATH2F* tempMAHist;
  QString label, list="";
  TObject* dummy=0;
  ModItem* topitem = (ModItem*)tw->ModuleList->firstChild();
  std::vector<int> stavenumbers;
  TPaveText *textdum;
  StdTestWin tmpwin(tw,"tmptestwin");

  std::map<int, std::vector<TH2F*> > oldhi;
  std::map<int, std::vector<MATH2F*> > ModHisto;
  std::map<int, std::vector<MATH2F*> >::iterator hist_iter;
  std::map<int, std::vector<ModItem*> >::iterator map_iter;
  int firstcount, maxitems=0, histcnt=0;					
  
  map_iter = itemmap.begin();
  if(tw->SystestMode)// && topitem->GetPID()!=STVITEM)
	firstcount = (map_iter->second).size();					//number of datasets in the mapelement
  else
    firstcount = itemmap.size();							//number of key elements in the map ==> #modules or #staves

  if(itemmap.size()==0)
	  return;

  CheckCan();
  m_canvas->cd();
  m_canvas->Clear(); 
  
  int nCol, nRow, nFe;
  map_iter->second[0]->GetDataItem()->getGeom(nRow, nCol, nFe);

  for(int v=0; v<(int)m_stnames.size(); v++)
    delete m_stnames[v];
  m_stnames.clear();

  if(topitem->GetPID()==STVITEM){
	stave = true;
	if(!tw->SystestMode)
      stavecount = firstcount;				//get number of loaded stave-items
	else
	  stavecount = tw->ModuleList->childCount();
	for(map_iter=itemmap.begin(); map_iter!=itemmap.end(); ++map_iter){	//is this necessary? may get stave-id just when drawing the pad!
	  textdum = new TPaveText(0,0,1,1);
      textdum->Clear();
//	  if(!tw->SystestMode)
	    textdum->AddText(("stave "+QString::number(map_iter->first)).latin1());
//	  else
//		textdum->AddText(("stave "+(topitem->text(0))).latin1());
	  textdum->SetFillColor(10);
      textdum->SetTextAlign(13);
      textdum->SetTextSize(0.6f);
      textdum->SetTextColor(4);
	  m_stnames.push_back(textdum);
	}
  }
  else{
	stave = false;
    stavecount = 1;
    maxPos = firstcount;
  }

  for(int z=0; z<2700 && (dummy = gROOT->FindObject(("oldhi_"+QString::number(z)).latin1())); z++)
    if(dummy!=0) dummy->Delete();
  for(int y=0; y<2700 && (dummy = gROOT->FindObject(("MAhi_"+QString::number(y)).latin1())); y++)
    if(dummy!=0) dummy->Delete();

  TPad *pdraw = new TPad("pdraw", "draw pad", 0, 0,.90,.94);
  pdraw->Draw();

  TPad *plegend = new TPad("plegend","scale pad", .90,0,1,.97);
  plegend->Draw();

  TPad *pcapt = new TPad("pcapt", "caption pad", 0,.94,.90,.97);
  pcapt->Draw();
  pcapt->Divide(stavecount, 1, 0.001f, 0.001f);

  TPad *ptxt = new TPad("ptxt","Text pad",0,.97,1,1);   
  ptxt->Draw();
  ptxt->cd();

  m_text->Clear();
  m_text->AddText(("Multi-module overview: "+ scanlabel).latin1());  

// find min./max. of all modules 
  for(map_iter=itemmap.begin(); map_iter!=itemmap.end(); ++map_iter){	//loop over keys ==> staves/module
    if((int)(map_iter->second).size()>maxitems) maxitems = (map_iter->second).size();
    for(int j=0; j<(int)(map_iter->second).size(); j++){						//loop over vector ==> ModItem*s
	  (map_iter->second)[j]->GetDID()->Get_FitResArrayMinMax(summary_type, tempMin, tempMax);
      if(tempMin<oldMin) oldMin=tempMin;
      if(tempMax>oldMax) oldMax=tempMax;
	}
  }

// bad pixel counters will be "inverted", do so with min./max. too
  if(summary_type==3 || summary_type==4){
    tempMax = oldMax;
    oldMax=2881-oldMin;
    oldMin=2881-tempMax;
  }

//data issues like creating and filling histograms
  for(map_iter=itemmap.begin(); map_iter!=itemmap.end(); ++map_iter)	//loop over keys ==> staves/module
    for(int i=0; i<(int)(map_iter->second).size(); i++){						//loop over vector ==> ModItem*s
  //create title: scantype and module-ids
      label = (map_iter->second)[i]->ModParent()->text(0);
      switch((map_iter->second)[i]->ModParent()->GetModID()){
        case 1: label+=" - M6C";
                        break;
        case 2: label+=" - M5C";
                        break; 
        case 3: label+=" - M4C";
                        break; 
		case 4: label+=" - M3C";
                        break;											//just replace 'item[i]' with '(map_iter->second)[i]'
        case 5: label+=" - M2C";
                        break; 
        case 6: label+=" - M1C";
                        break; 
        case 7: label+=" - M0";
                        break; 
		case 8: label+=" - M1A";
                        break; 
        case 9: label+=" - M2A";
                        break; 
		case 10: label+=" - M3A";
                         break; 
		case 11: label+=" - M4A";
                         break; 
        case 12: label+=" - M5A";
                         break; 
        case 13: label+=" - M6A";
                         break; 
        }
	  label=label+" "+(tmpwin.prefix[((map_iter->second)[i]->GetDID()->GetStdtest()/100)]);
      if((map_iter->second)[i]->ModParent()->GetModID()>maxPos && !tw->SystestMode)
        maxPos=(map_iter->second)[i]->ModParent()->GetModID();
      else if(tw->SystestMode)
		maxPos=13;	//(map_iter->second).size();

	//create clickable 2d histogram
      tempHist = new TH2F(("oldhi_"+QString::number(histcnt)).latin1(), label.latin1(), 8, -0.5, 7.5, 2, -0.5, 1.5);
      oldhi[map_iter->first].push_back(tempHist);

      tempMAHist = new MATH2F(*(MATH2F*)tempHist);//oldhi[i]);
      tempMAHist->SetName(("MAhi_"+QString::number(histcnt)).latin1());
      ModHisto[map_iter->first].push_back(tempMAHist);

      ModuleGrid(ModHisto[map_iter->first][i], nFe);
      m_currmap = ModHisto[map_iter->first][i];
    
    //fill histograms with summary data of selected type
      for(int col=0; col<8; col++){
        for(int row=0; row<2; row++){
          if(row==0)
            chip = col;
              else
            chip = 15-col;
          if(summary_type==3 || summary_type==4){
            ModHisto[map_iter->first][i]->Fill(col, row, 2880 - (map_iter->second)[i]->GetDID()->Get_FitResArrayCell(summary_type, chip));
		  }
          else
            ModHisto[map_iter->first][i]->Fill(col, row, (map_iter->second)[i]->GetDID()->Get_FitResArrayCell(summary_type, chip));
		}
	  }
	  histcnt++;
	}
  
//plotting issues
  if(ModHisto.size()>0 && (stave || tw->SystestMode))
	  pdraw->Divide(stavecount, 13, 0.001f, 0.001f);		//ModHisto.size(), maxPos, 0, 0);
  else{
	  if(ModHisto.size()<=13)
	    pdraw->Divide(maxitems, ModHisto.size(), 0.001f, 0.001f);
	  else
		pdraw->Divide((((int)(ModHisto.size()-.5)/13)+1)*maxitems, 13, 0.001f, 0.001f);
  }

  // get color scale and plot to extra pad

    if(maxPos>1)
      pdraw->cd(1);
    else
      pdraw->cd();
    if(oldMax>0)        
      (*(ModHisto.begin()->second.begin()))->SetMaximum((int)(1.1*oldMax));
    else
      (*(ModHisto.begin()->second.begin()))->SetMaximum(1); 
    if(oldMin>10)
      (*(ModHisto.begin()->second.begin()))->SetMinimum((int)(0.9*oldMin));
    else
      (*(ModHisto.begin()->second.begin()))->SetMinimum(0);
    
    if(m_palette!=0){
		m_palette->Delete();
		m_palette = 0;
	}
    (*(ModHisto.begin()->second.begin()))->Draw("COLZ");
    gPad->Modified();
    gPad->Update();
    m_palette = (MATPaletteAxis*)((*(ModHisto.begin()->second.begin()))->FindObject("palette"));
    if(m_palette!=0){
      m_palette = new MATPaletteAxis(*(MATPaletteAxis*)m_palette, this);
      m_palette->SetName("MApalette");
    }
    plegend->cd();
    if(m_palette!=0){
      m_palette->SetX1NDC(.2);
      m_palette->SetX2NDC(.5);
      m_palette->SetLabelSize(0.2f);
      m_palette->Draw();
    }
	//clear the pad again
    if(maxPos>1)
      pdraw->cd(1);
    else
      pdraw->cd();
	gPad->Clear();

//now really plot the histograms
  if(maxPos>1)
    pdraw->cd(1);
  else
    pdraw->cd();

  if(stave){
    for(int vi=0; vi<maxitems; vi++){
	  stc=1;
	  for(hist_iter=ModHisto.begin(); hist_iter!=ModHisto.end(); ++hist_iter){
		ij++;
		if((int)(hist_iter->second).size()>vi){
		//	if(!tw->SystestMode)
		      padnr=((((itemmap.find(hist_iter->first)->second)[vi]->ModParent()->GetModID())-1)*ModHisto.size())+stc;
		//	else
		//	  padnr=ij;
			if(padnr==0)
			  padnr=ij*itemmap.size();	
		    pdraw->cd(padnr);
		    if(hist_iter->second.size()>0){
			  (hist_iter->second).at(vi)->SetMaximum((int)(1.1*oldMax));//+50);
			  if(oldMin>10)
				(hist_iter->second).at(vi)->SetMinimum((int)(0.9*oldMin));
			  else
				(hist_iter->second).at(vi)->SetMinimum(0);

			  (hist_iter->second).at(vi)->Draw("COL");
			  (hist_iter->second).at(vi)->SetWidget(tw);
			  TitleResize(3.5);
			  gPad->Modified();
			  gPad->Update();
			}
		  }
		SetStyle(1);
		stc++;
	  }
	}
  }
  else{
    for(hist_iter=ModHisto.begin(); hist_iter!=ModHisto.end(); ++hist_iter){
	  for(int vi=0; vi<maxitems; vi++){
		ij++;
		if((int)(hist_iter->second).size()>vi){		
		    padnr=ij;

		    pdraw->cd(padnr);
		    if(hist_iter->second.size()>0){
			  (hist_iter->second).at(vi)->SetMaximum((int)(1.1*oldMax));//+50);
			  if(oldMin>10)
				(hist_iter->second).at(vi)->SetMinimum((int)(0.9*oldMin));
			  else
				(hist_iter->second).at(vi)->SetMinimum(0);

			  (hist_iter->second).at(vi)->Draw("COL");
			  (hist_iter->second).at(vi)->SetWidget(tw);
			  TitleResize(3.5);
			  gPad->Modified();
			  gPad->Update();
			}
		  }
		SetStyle(1);
	  }
	}
  }
  ptxt->cd();
  m_text->SetTextSize(.6f);
  m_text->SetTextAlign(13);
  m_text->Draw();

  pcapt->cd();
  for(int stn=0; stn<stavecount; stn++)
    if(stn<(int)m_stnames.size()){
	  pcapt->cd(stn+1);
	  m_stnames[stn]->Draw();
	}
  m_canvas->Modified();
  m_canvas->Update();
  return;
}

void RMain::RescaleOverview(int Min, int Max){
  
  MATH2F *dummy;
  TPad *temppad, *legpad;  
  int count=0;
//  TPaletteAxis *temp_pal;

  for(int z=0; z<2700; z++){		// crgg what is 2700
    dummy = (MATH2F*)gROOT->FindObject(("MAhi_"+QString::number(z)).latin1());
        if(dummy!=0){
          dummy->SetMaximum(Max);
          dummy->SetMinimum(Min);
          count++;
        }
  }
  temppad = (TPad*)gROOT->FindObject("pdraw");
  if(temppad!=0){
        temppad->cd(1);
        dummy = (MATH2F*)gROOT->FindObject("MAhi_0");
        dummy->Draw("colz");
        temppad->Modified();
        temppad->Update();
		if(m_palette!=0) {
			m_palette->Delete();
			m_palette = 0;
		}
        m_palette = (MATPaletteAxis*)(dummy->FindObject("palette"));
        if(m_palette!=0){
	  m_palette = new MATPaletteAxis(*(MATPaletteAxis*)m_palette, this);
	  m_palette->SetName("MApalette");
	}
        legpad = (TPad*)gROOT->FindObject("plegend");
        if(legpad!=0)
          legpad->cd();
        if(m_palette!=0){
          m_palette->SetX1NDC(.2);
          m_palette->SetX2NDC(.5);
          m_palette->SetLabelSize(0.2f);
          m_palette->Draw();
        }
        temppad->cd(1);
        dummy->Draw("col");
        if(count<13)
          TitleResize((float)((int)(count/2)));
        else
          TitleResize(3.5);
        temppad->cd();
    temppad->Modified();
    temppad->Update();
        temppad->Draw();
  }

  m_canvas->Modified();
  m_canvas->Update();
}
//*****************************************************************************************
int RMain::plotPS_ValVSPar(std::vector<int> pars, ModItem *item, int scan_level, int scanidx[3], int in_chip, int fid, bool pltAvg){
  int nCol, nRow, nFe, nFeRows;
  item->GetDataItem()->getGeom(nRow, nCol, nFe, nFeRows);
  if(nFeRows==0) nFeRows=1;
  int nColTot = nCol*nFe/nFeRows, nRowTot=nRow*nFeRows;
  int ptarr[4]={-1,-1,-1,0};
  for(int ilv=0;ilv<3;ilv++) if(scan_level<=ilv) ptarr[ilv] = scanidx[ilv];
#ifdef PIXEL_ROD // can link to PixScan
  PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
  if(pdbd==0) return -1;

  gROOT->cd();
  TH2F *map = (TH2F*)gROOT->FindObject("accdata"), *tmpHi=0, *tmpToTHi=0;
  if(map!=0){ // remove old histo to avoid memory leak
    map->Delete();
    map = 0;
  }
  if(!pltAvg){
    if(in_chip<0){
       map = new TH2F("accdata","",nColTot,-.5,-0.5+(double)nColTot,nRowTot,-0.5,-0.5+nRowTot);
    }
    else{
      map = new TH2F("accdata","",nCol,-0.5,-0.5+(double)nCol,nRow,-0.5,-0.5+(double)nRow);
    }
  }

  int type = item->GetPID()-PIXDBTYPES;
  int i,j,row,col;
  std::vector<float> scanpt  = pdbd->getScanPoints(scan_level);
  int steps = scanpt.size();
  double nevts = (double)pdbd->getNevents();
  if(pdbd->getScanPar(scan_level)=="CAPSEL"){
    // scan against inj. capacitance, retrieve values from FE
    // TODO: consider >1 FE
    const int nCap=3;
    std::string capLabels[nCap]={"CInjLo", "CInjMed", "CInjHi"};
    Config &conf = pdbd->getModConfig().subConfig("PixFe_0/PixFe");;
    for(int i=0;i<steps;i++){
      int scanpti = (int)scanpt[i];
      float cInj = 0.;
      if(scanpti>=0 && scanpti<nCap && conf["Misc"].name()!="__TrashConfGrp__"){
	if(conf["Misc"][capLabels[scanpti]].name()!="__TrashConfObj__")
	  cInj = (dynamic_cast<ConfFloat &>(conf["Misc"][capLabels[scanpti]])).value();
	else
	  cout << "Can't find ConfObj " << capLabels[scanpti] << endl;
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
  pdbd->m_keepOpen=true;
  for(i=0;i<steps;i++){
    ptarr[scan_level] = i;
    tmpHi = (TH2F*)pdbd->getGenericHisto((PixLib::PixScan::HistogramType)type,0,0,0, ptarr);
    if(tmpHi!=0) tmpHi = new TH2F(*tmpHi);
    if(type==(int)PixLib::PixScan::TOT_MEAN){ // get sigma histo for error later on, if it was saved
      tmpToTHi = (TH2F*)pdbd->getGenericHisto(PixLib::PixScan::TOT_SIGMA,0,0,0,i, scan_level);
      if(tmpToTHi!=0) tmpToTHi = new TH2F(*tmpToTHi);
    }
    if(pltAvg){
      title = tmpHi->GetTitle();
    } else{
      title = map->GetTitle();
      if(title=="" && tmpHi!=0){
	title = tmpHi->GetTitle();
	title += " - accumulated";
	if(in_chip>=0){
	  std::stringstream a;
	  a << in_chip;
	  title += " - chip "+a.str();
	}
	map->SetTitle(title.c_str());
      }
    }
    double nval=0.;
    if(pltAvg){
      yval[i] = 0.;
      yerr[i] = 0.;
    }
    for(j=cmin; j<cmax; j++){
      for(col=0;col<nCol; col++){
	for(row=0;row<nRow; row++){
	  if(in_chip<0){
	    item->GetDataItem()->PixCCRtoBins(j, col, row, binx, biny);
	    binxf = binx; binyf = biny;
	  } else{
	    item->GetDataItem()->PixCCRtoBins(0, col, row, binx, biny);
	    item->GetDataItem()->PixCCRtoBins(in_chip, col, row, binxf, binyf);
	  }
	  if(pltAvg){
	    if(tmpHi!=0 && tmpHi->GetBinContent(binxf,binyf)!=0){
	      yval[i] += (double)tmpHi->GetBinContent(binxf,binyf);
	      yerr[i] += ((double)tmpHi->GetBinContent(binxf,binyf))*((double)tmpHi->GetBinContent(binxf,binyf));
	      nval += 1.;
	    }
	  }else{
	    if(tmpHi!=0){
	      map->SetBinContent(binx,biny,map->GetBinContent(binx,biny)+tmpHi->GetBinContent(binxf,binyf));
	      if(tmpHi!=0 && j==m_chip && col==m_col &&row==m_row){
		xval[i] = (double)scanpt[i];
		yval[i] = (double)tmpHi->GetBinContent(binxf,binyf);
		xerr[i] = 1e-4*xval[i];
		switch(type){
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
		    yerr[i] = (double)tmpToTHi->GetBinContent(binxf,binyf);
		  else
		    yerr[i] = 1e-4*yval[i];
		  break;
		default: // don't know what to do - have no error
		  yerr[i] = 1e-4*yval[i];
		}
	      }
	    } else{
	      xval[i] = (double)scanpt[i];
	      xerr[i] = 1e-4*xval[i];
	      yval[i] = 0.;
	      yerr[i] = 0.;
	    }
	  }
	}
      }
    }
    if(pltAvg){
      xval[i] = (double)scanpt[i];
      xerr[i] = 1e-4*xval[i];
      if(nval>1){
	yval[i] /= nval;
	yerr[i] = (yerr[i]-nval*yval[i]*yval[i])/(nval-1);
	if(yerr[i]>0) yerr[i] = sqrt(yerr[i]);
	else          yerr[i] = 0.;
	if(type==(int)PixLib::PixScan::SCURVE_MEAN || type==(int)PixLib::PixScan::SCURVE_SIGMA ||
	   type==(int)PixLib::PixScan::TIMEWALK){ 
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
		  item->GetDataItem()->PixCCRtoBins(j, col, row, binx, biny);
		  binxf = binx; binyf = biny;
		} else{
		  item->GetDataItem()->PixCCRtoBins(0, col, row, binx, biny);
		  item->GetDataItem()->PixCCRtoBins(in_chip, col, row, binxf, binyf);
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
    }
    delete tmpHi; tmpHi=0;
    delete tmpToTHi; tmpToTHi=0;
  }
  pdbd->m_keepOpen=false;

  if(map==0 && !pltAvg) return -2;

  gROOT->cd();
  TGraphErrors *gr = new TGraphErrors(steps,xval,yval, xerr, yerr);
  int pos;
  // graph general titel
  pos = title.find(" - accumulated");
  if(pos!=(int)std::string::npos)
    title.erase(pos,title.length()-pos);
  std::stringstream strs;
  strs << "Chip " << m_chip << " col " << m_col << " row " << m_row;
  if(pltAvg)
    gr->SetTitle("");
  else
    gr->SetTitle(strs.str().c_str());

  // graph axes title
  if(pdbd->getScanPar(scan_level)=="CAPSEL"){
    gr->GetXaxis()->SetTitle("C_{inj} (fF)");
  }else if(pdbd->getScanPar(scan_level)=="FEI4_GR"){
    Config &scfg = pdbd->getScanConfig();
    std::stringstream lnum;
    lnum << scan_level;
    std::string atit = "FEI4_GR";
    if(scfg["loops"]["feGlobRegNameLoop_"+lnum.str()].name()!="__TrashConfObj__")
      atit = ((ConfString&)scfg["loops"]["feGlobRegNameLoop_"+lnum.str()]).value();
    gr->GetXaxis()->SetTitle(atit.c_str());
  }else{
    gr->GetXaxis()->SetTitle(pdbd->getScanPar(scan_level).c_str());
  }
  if(pltAvg)
    gr->GetYaxis()->SetTitle(("Avg. of "+title).c_str());
  else
    gr->GetYaxis()->SetTitle(title.c_str());

  gr->SetMarkerStyle(20);
  gr->SetMarkerSize(.6f);
  m_misc_obj.push_back(gr);

  TF1 *fun=0;
  // if function plotting was requested, do so
  if(fid>=0){
    delete m_RDBfunc;
    int npar = m_fc->getNPar(fid);
    if(m_fc->hasVCAL(fid)) // allow special VCAL fit
      npar += 4;
    fun = new TF1(m_fc->getFuncName(fid).c_str(), (double (*)(double *, double *))m_fc->getFunc(fid), 
		  xval[0], xval[steps-1], npar);
    m_RDBfunc = fun; // will allow us to delete this later
    std::vector<std::string> labels = m_fc->getParNames(fid);
    for(int pi=0;pi<m_fc->getNPar(fid);pi++){
      double parVal = 0;
      if(pars[pi]<PixLib::PixScan::MAX_HISTO_TYPES){
	tmpHi = (TH2F*)pdbd->getGenericHisto((PixLib::PixScan::HistogramType)pars[pi], 0,0,0,-1, scan_level+1);
	item->GetDataItem()->PixCCRtoBins(m_chip, m_col, m_row, binx, biny);
	parVal = (double)tmpHi->GetBinContent(binx,biny);
      } else
	parVal = (double)pdbd->getNevents();
      fun->SetParameter(pi,parVal);
      fun->SetParName(pi,labels[pi].c_str());
    }
    if(m_fc->hasVCAL(fid)){ // allow special VCAL fit
      double Cval, parVal;
      const int nCap=3;
      std::string Cname="none", capLabels[nCap]={"CInjLo", "CInjMed", "CInjHi"};
      Config &scfg = pdbd->getScanConfig();
      if(scfg["fe"]["chargeInjCap"].name()!="__TrashConfObj__"){
	int iCap = ((ConfInt&)scfg["fe"]["chargeInjCap"]).value();
	if(iCap>=0 && iCap<nCap) Cname = capLabels[iCap];
      }
      
      // set VCAL calibration parameters
      std::stringstream a;
      a << m_chip;
      Config &mcfg = pdbd->getModConfig().subConfig("PixFe_"+a.str()+"/PixFe");
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

  if(pltAvg){
    CheckCan();
    m_canvas->cd();
    m_canvas->Clear();
    TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
    ptxt->Draw();
    TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
    pplt->Draw();
    m_text->Clear();
    char line[1024];
    sprintf(line,"Graph of module average: %s - errors bars are %s",item->GetDataItem()->GetName(),
	    (type==(int)PixLib::PixScan::SCURVE_MEAN || type==(int)PixLib::PixScan::SCURVE_SIGMA ||
	     type==(int)PixLib::PixScan::TIMEWALK)?"Gaussian sigma":"spread");
    m_text->AddText(line);
    sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
    m_text->AddText(line);
    ptxt->cd();
    m_text->Draw();
    
    if(m_psfile!=NULL) m_psfile->NewPage();
    
    pplt->cd();
    gPad->SetLeftMargin(0.12f);
    gr->Draw("AP");
    gr->GetXaxis()->SetTitleSize(.03f);
    gr->GetXaxis()->SetLabelSize(.03f);
    gr->GetYaxis()->SetTitleOffset(1.8f);
    gr->GetYaxis()->SetTitleSize(.03f);
    gr->GetYaxis()->SetLabelSize(.03f);
    m_canvas->Modified();
    m_canvas->Update();
  } else
    PlotMapScanhi(item, in_chip, -1, map, gr, fun);

  // clean up
  delete xval; delete yval;
  delete xerr; delete yerr;
  if(map!=0) map->Delete();
#endif
  return 0;
}
int RMain::PlotCM3Graph (ModItem *item, double Uload, double *Cvals){
#ifdef PIXEL_ROD // can link to PixScan
  char line[1024];
  gROOT->cd();
  TGraph *indgraph=item->GetDataItem()->GetGraph(0,0);
  if(indgraph==0) return -2;
  CheckCan();
  int sort[3][4]={{5,3,2,1}, {10,9,7,6}, {15,14,13,11}};
  char clabel[3][20];
  sprintf(clabel[0],"C_{low}");
  sprintf(clabel[1],"C_{fb}");
  sprintf(clabel[2],"C_{low}+C_{high}");
  m_canvas->cd();
  m_canvas->Clear();
  gStyle->SetOptFit(0);
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  pplt->Divide(2,3,0.001f,0.001f);
  
  m_text->Clear();
  sprintf(line,"%s. ",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  if(m_psfile!=0) m_psfile->NewPage();

  for(int ic=0;ic<3;ic++){
    TMultiGraph *mg = new TMultiGraph();
    TLegend *leg = new TLegend(.25f,.75f,.75f,.99f);
    double fpar[4], x[4]={0.,1.,2.,4.}, y[4];
    for(int ixck=0;ixck<4;ixck++){
      int bxck = ixck << 4;
      for(int k=0;k<4;k++)  y[k] = indgraph->GetY()[sort[ic][k]+bxck];
      TGraph *gr = new TGraph(4,x,y);
      gr->SetMarkerStyle(20);
      gr->SetMarkerColor(ixck+1);
      if(ic<2 || ixck<3)
	gr->Fit("pol1","q");
      else // skip last point for Chigh since most likely saturated measurement
	gr->Fit("pol1","q","",0.,3.);
      fpar[ixck] = gr->GetFunction("pol1")->GetParameter(1);
      gr->GetFunction("pol1")->SetLineColor(ixck+1);
      mg->Add(gr);
      sprintf(line,"freq. = XCK/%d",32/(int)pow(2.f,(float)ixck));
      leg->AddEntry(gr,line,"p");
    }
    double freq[4]={1.25, 2.5, 5, 10};
    TGraph *fsg = new TGraph(4,freq,fpar);
    fsg->SetMarkerStyle(20);
    fsg->Fit("pol1","q");
    double resval = fsg->GetFunction("pol1")->GetParameter(1)/Uload*1e9;
    if(Cvals!=0) Cvals[ic] = resval;
    sprintf(line,"%s = %.2f fF", clabel[ic], resval);
    fsg->SetTitle(line);
    
    pplt->cd(1+2*ic);
    gPad->SetRightMargin(0.18f);
    mg->Draw("AP");
    mg->GetXaxis()->SetTitle("N_{cap}");
    mg->GetXaxis()->SetTitleSize(.04f);
    mg->GetXaxis()->SetLabelSize(.04f);
    mg->GetYaxis()->SetTitle("I_{cap} (A)");
    mg->GetYaxis()->SetTitleOffset(1.1f);
    mg->GetYaxis()->SetTitleSize(.04f);
    mg->GetYaxis()->SetLabelSize(.04f);
    leg->Draw();
    
    pplt->cd(2+2*ic);
    gPad->SetRightMargin(0.15f);
    fsg->Draw("AP");
    fsg->GetXaxis()->SetTitle("f (MHz)");
    fsg->GetXaxis()->SetTitleSize(.04f);
    fsg->GetXaxis()->SetLabelSize(.04f);
    fsg->GetYaxis()->SetTitle("I_{cap} (A)");
    fsg->GetYaxis()->SetTitleOffset(1.1f);
    fsg->GetYaxis()->SetTitleSize(.04f);
    fsg->GetYaxis()->SetLabelSize(.04f);
  }

  m_canvas->cd();
  m_canvas->Update();
  m_canvas->Modified();

  return 0;
#endif
}
int RMain::FitGraph(int iFitFunc, ModItem *item, int plotType, double xmin, double xmax){
  char line[1024];
  CheckCan();

  TF1 *fitfunc = RMain::GetFitFunc(iFitFunc); 
  if(fitfunc==0) return -8;
  TGraph *gr = item->GetDataItem()->GetGraph(0,0);
  if(gr==0) return -2;

  m_canvas->cd();
  m_canvas->Clear();
  TPad *ptxt = new TPad("ptxt","Text pad",0,.94,1,1);
  ptxt->Draw();
  TPad *pplt = new TPad("pplt","Plot pad",0,0,1,.94);
  pplt->Draw();
  
  m_text->Clear();
  sprintf(line,"%s.",item->GetDataItem()->GetName());
  m_text->AddText(line);
  sprintf(line,"Module \"%s\"",item->ModParent()->text(0).latin1());
  m_text->AddText(line);
  ptxt->cd();
  m_text->Draw();

  if(m_psfile!=NULL) m_psfile->NewPage();

  if(plotType>0){
    pplt->Divide(1,2,.001f,.001f);
    pplt->cd(1);
  } else
    pplt->cd();
  gPad->SetRightMargin(0.15f);
  gr->SetMarkerStyle(20);
  gr->SetMarkerSize(0.5f);
  gr->GetXaxis()->SetTitleSize(.03f);
  gr->GetXaxis()->SetLabelSize(.03f);
  gr->GetYaxis()->SetTitleOffset(1.2f);
  gr->GetYaxis()->SetTitleSize(.03f);
  gr->GetYaxis()->SetLabelSize(.03f);
  gr->Draw("AP");
  fitfunc->SetLineWidth(1);
  fitfunc->SetLineColor(2);
  if(xmax>xmin)
    gr->Fit(fitfunc, "q", "", xmin, xmax);
  else
    gr->Fit(fitfunc, "q");
  if(plotType>0)
    pplt->cd(2);

  if(plotType==1){
    TGraph *rgr = new TGraph();
    for(int k=0;k<gr->GetN();k++){
      double diff = gr->GetY()[k]-fitfunc->Eval(gr->GetX()[k]);
      rgr->SetPoint(k, gr->GetX()[k], diff);
    }
    rgr->SetMarkerStyle(20);
    rgr->SetMarkerSize(0.5f);
    rgr->GetXaxis()->SetTitleSize(.03f);
    rgr->GetXaxis()->SetLabelSize(.03f);
    rgr->GetXaxis()->SetTitle(gr->GetXaxis()->GetTitle());
    rgr->GetYaxis()->SetTitleOffset(1.2f);
    rgr->GetYaxis()->SetTitleSize(.03f);
    rgr->GetYaxis()->SetLabelSize(.03f);
    char ytit[300];
    sprintf(ytit,"Difference fit-data: %s", gr->GetYaxis()->GetTitle());
    rgr->GetYaxis()->SetTitle(ytit);
    rgr->Draw("AP");
  } else if(plotType==2){
    double max=0;
    for(int k=0;k<gr->GetN();k++){
      double diff = gr->GetY()[k]-fitfunc->Eval(gr->GetX()[k]);
      if(TMath::Abs(diff)>max) max = TMath::Abs(diff);
    }
    TH1F *rhi = (TH1F*) gROOT->FindObject("reshi");
    if(rhi!=0) rhi->Delete();
    rhi = new TH1F("reshi","Fit residual",20,-1.05*max,1.05*max);
    for(int k=0;k<gr->GetN();k++){
      double diff = gr->GetY()[k]-fitfunc->Eval(gr->GetX()[k]);
      rhi->Fill(diff);
    }
    rhi->GetXaxis()->SetTitleSize(.03f);
    rhi->GetXaxis()->SetLabelSize(.03f);
    char xtit[300];
    sprintf(xtit,"Difference fit-data: %s", gr->GetYaxis()->GetTitle());
    rhi->GetXaxis()->SetTitle(xtit);
    rhi->GetYaxis()->SetTitleOffset(1.2f);
    rhi->GetYaxis()->SetTitleSize(.03f);
    rhi->GetYaxis()->SetLabelSize(.03f);
    rhi->GetYaxis()->SetTitle("Entries");
    rhi->Draw();
  } else if(plotType!=0)
    return -10;

  m_canvas->cd();
  m_canvas->Update();
  m_canvas->Modified();

  return 0;
}
void  RMain::flipHistoCont(TH2F *hi){ // only use for FE-I4 (336 rows), otherwise will produce nonsense
  for(int i=1;i<=hi->GetNbinsX();i++){
    for(int j=1;j<169;j++){ // 168 is half of 336
      float temp = hi->GetCellContent(i,j);
      int j_temp = 336-j+1;
      hi->SetBinContent(i,j,hi->GetCellContent(i,j_temp));
      hi->SetBinContent(i,j_temp,temp);
      // module with 2 FE rows: also flip top row
      if(hi->GetNbinsY()>336){
	int jtop = j+336;
	temp = hi->GetCellContent(i,jtop);
	j_temp = 672-j+1;
	hi->SetBinContent(i,jtop,hi->GetCellContent(i,j_temp));
	hi->SetBinContent(i,j_temp,temp);
      }
    }
  }
  hi->GetYaxis()->SetBinLabel(336,"0");
  hi->GetYaxis()->SetBinLabel(236,"100");
  hi->GetYaxis()->SetBinLabel(136,"200");
  hi->GetYaxis()->SetBinLabel(36,"300");
  if(hi->GetNbinsY()>336){
    hi->GetYaxis()->SetBinLabel(436,"100");
    hi->GetYaxis()->SetBinLabel(536,"200");
    hi->GetYaxis()->SetBinLabel(636,"300");
  }
  hi->GetYaxis()->SetTickLength(0.5*hi->GetYaxis()->GetTickLength());
}
