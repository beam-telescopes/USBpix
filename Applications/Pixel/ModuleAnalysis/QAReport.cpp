#include "DataStuff.h"
#include "BadPixel.h"
#include "ModTree.h"
#include "StdTestWin.h"
#include "QAReport.h"
//Added by qt3to4:
#include <QPixmap>
#include "CutPanel.h"
#include "FileTypes.h"
#include "RootStuff.h"
#include "TopWin.h"
#include "LoginPanel.h"
#include "MAEngine.h"

#include <TH1F.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TMath.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TSystem.h>
#include <TGraphErrors.h>

#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
#include <q3table.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qcombobox.h>
#include <q3textbrowser.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qspinbox.h>
#include <qapplication.h>
#include <qmime.h>

#include <math.h>
#include <string>
#include <vector>

QAReport::QAReport(ModItem *in_modit, QWidget* parent, const char* , 
                   bool , Qt::WFlags fl, const char *outpath, QApplication *app)
  : QDialog(parent,fl), m_app(app){
  //  : QAReportBase(parent,name,modal,fl), m_app(app){
  setupUi(this);
    QObject::connect(CloseButt, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(BrowseButton, SIGNAL(clicked()), this, SLOT(LoadPath()));
    QObject::connect(StartButton, SIGNAL(clicked()), this, SLOT(StartReport()));
    QObject::connect(QualiTable, SIGNAL(selectionChanged()), this, SLOT(copyToCB()));
    QObject::connect(bareReworked, SIGNAL(stateChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(fp_beforeAssy, SIGNAL(stateChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(mcc_beforeAssy, SIGNAL(stateChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(discRegions, SIGNAL(valueChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(flexReworked, SIGNAL(stateChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(MCCReworked, SIGNAL(stateChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(moduleBow, SIGNAL(valueChanged(int)), this, SLOT(recalcMech()));
    QObject::connect(ReportText, SIGNAL(linkClicked(QString)), this, SLOT(processLink(QString)));

  setUpdatesEnabled(TRUE);

  modit = in_modit;
  m_parent = dynamic_cast<TopWin*>(parent);
  m_cutpnl = 0;
  rmain = 0;
  int ncutclasses = 0;
  if(m_parent!=0){
    m_cutpnl = m_parent->m_cutpanel;
    rmain =    m_parent->rootmain;
    ncutclasses = m_cutpnl->ClassList->childCount();
  }
  m_neff = 0;
  // title of panel
  QString title;
  if(modit!=0){
    title  = "Test report for module ";
    title += modit->text(0);
  }
  Headline->setText(title);

  // set up table
  QualiTable->setColumnWidth(0,200);
  QualiTable->horizontalHeader()->setLabel(0,"quantity");
  QualiTable->horizontalHeader()->setLabel(1,"value");
  QualiTable->horizontalHeader()->setLabel(2,"Nbad");
  QualiTable->setText(0,0,"Source scan bad pixels");
  QualiTable->setText(1,0,"Digital Bad Pixels");
  QualiTable->setText(2,0,"Analog Bad pixels");	
  QualiTable->setText(3,0,"Average noise (e)");	
  QualiTable->setText(4,0,"Threshold dispersion (e)");
  QualiTable->setText(5,0,"Total ranking penalty");
  QualiTable->setText(6,0,"Avg. cross-talk (%)");
  QualiTable->setText(7,0,"Avg. in-time threshold (e)");
  QualiTable->setText(8,0,"Avg. ToT(mip)");
  QualiTable->setText(9,0,"min. VDD (V)");
  QualiTable->setText(10,0,"Ileak (nA)");

  if(ncutclasses<=0 && m_cutpnl!=0){
    //    ReportText->insertLine("# no cuts defined");
//     ReportText->setText("<b>no cuts defined</b><br>");
//     ReportText->repaint();
//     BrowseButton->setEnabled(false);
//     return;
    m_cutpnl->NewClass();
    m_cutpnl->RenameEdit->setText("Default cuts");
    m_cutpnl->RenameClass();
    m_usedef = true;
  } else
    m_usedef = false;

  // set external info variables
  m_selcutID = 0;
  // passed flags
  m_IV_OK  = false;
  m_DIG_OK = false;
  m_ANA_OK = false;
  m_SRC_OK = false;
  m_all_OK = false;
  // IV stuff
  m_IV_bdV =0;
  m_IV_oper=0;
  m_IV_100 =0;
  m_IV_max =0;
  // analogue scans
  m_avgThr=-1; 
  m_sdThr=-1;
  m_avgNse=-1;
  m_avgXtalk=-1;
  m_avgITThresh=-1;
  m_sdITThresh=-1;
  m_avgTOT=-1;
  m_sdTOT=-1;
  m_minVDD  = -1;
  m_maxVDD  = -1;
  m_minVDDA = -1;
  m_maxVDDA = -1;
  m_minTHR  = -1;
  m_maxTHR  = -1;
  m_GDACfail= -1;
  m_maxXCK  = -1;

  // generate default file paths
  std::string path;
  QString fname;
  StdTestWin tmpwin(this,"tmpstdwin");
  int i, type = 0;
  bool havetype[NTYPES];
  for(i=0;i<NTYPES;i++){
    havetype[i] = false;
    if(i>0)
      testTypeSel->insertItem(tmpwin.prefix[i]);
  }
  if(modit!=0){
    ModItem *tmp = (ModItem*) modit->firstChild();
    // find out data test types
    while(tmp!=NULL){
      type = tmp->GetDID()->GetStdtest()/100;
      if(type>0 && !havetype[type]){
        havetype[type] = true;
        testTypeSel->setCurrentItem(type);
      }
      tmp = (ModItem*) tmp->nextSibling();
    }
  }
//   if(testTypeSel->count()>0)
//     testTypeSel->setCurrentItem(testTypeSel->count()-1);

  // path stuff
  if(type>=NTYPES) type = 0;
  if(modit!=0 && !modit->m_lastpath.isEmpty() && outpath==NULL){
    // suggest QA-folder relative to last data dir.
    path = modit->m_lastpath.latin1();
    int i = path.find_last_of("/");
    path.erase(i+1,path.length()-i-1);
    /*
    QString path = modit->m_lastpath;
    int i = path.find("/",-1);
    if(i!=-1) path.remove(i,path.length());
    */
    path += "qa/";
    QString mypath = path.c_str();
    LoadPath(mypath);
  }else if(outpath!=NULL){
    LoadPath(outpath);
    path = outpath;
  }
  return;
}
QAReport::~QAReport(){
  if(m_usedef && m_cutpnl!=0) // created cuts for user, will delete them now
    m_cutpnl->DeleteClass();
}

void QAReport::StartReport(){
  // get NSLOTS from StdTestWin
  ModItem *datit, *thrit;
  std::vector<ModItem*> std_arr;
  StdTestWin tmpwin(this,"tmpstdwin");
  std_arr.resize(tmpwin.NSLOTS);
  TH2F *hitocheck=0, *xtalk=NULL, *ignore=NULL;
  QString title;
  int i,j;                        // 
  int mychip;
  char hname[100], htit[400];
  int cutid = 0, bit_mask=0;
  long int plotid=0;
  float convert=1, offset=0;
  float con, ncnt=0;
  //float mod_thresh, mod_tdisp, mod_noise, mod_ndisp;
  QString cutname = "none";
  CutClassItem *lvit;
  std::vector<bool> allpassed;
  std::vector<bool> tmppassed;
  std::vector<float> valarr;
  m_VDD = -999;
  m_VDDA = -999;
  // set external info variables
  m_selcutID = ((CutClassItem*)m_cutpnl->ClassList->currentItem())->itemID();
  // passed flags
  m_IV_OK  = true;
  m_DIG_OK = true;
  m_ANA_OK = true;
  m_SRC_OK = true;
  m_all_OK = true;
  // IV stuff
  m_IV_bdV =0;
  m_IV_bdV =0;
  m_IV_oper=0;
  m_IV_100 =0;
  m_IV_max =0;
  // analogue scans
  m_avgThr=-1; 
  m_sdThr=-1;
  m_avgNse=-1;
  m_avgXtalk=-1;
  m_avgITThresh=-1;
  m_sdITThresh=-1;
  m_avgTOT=-1;
  m_sdTOT=-1;
  m_minVDD  = -1;
  m_maxVDD  = -1;
  m_minVDDA = -1;
  m_maxVDDA = -1;
  m_minTHR  = -1;
  m_maxTHR  = -1;
  m_GDACfail= -1;
  m_maxXCK  = -1;

  // create bit mask for module property plots later (so that noise/thresh. failures aren't masked)
  for(i=0;i<m_cutpnl->npcuts;i++)
    if(m_cutpnl->GetCutPars(i)->label!="noise" &&
       m_cutpnl->GetCutPars(i)->label!="diff. from mean threshold") bit_mask += (1<< i);    

  // bring canvas to foreground
  if(rmain!=0)
    rmain->CheckCan();

  // remove old data items with QA report masks
  ModItem *tmpit1, *tmpit2;
  tmpit1 = (ModItem*)modit->firstChild();
  while(tmpit1!=NULL){
    tmpit2 = tmpit1;
    tmpit1 = (ModItem*) tmpit1->nextSibling();
    if(tmpit2->text(0).find("Bad pixel map")>=0 && tmpit2->text(0).find(testTypeSel->currentText())>=0){
      delete (tmpit2->GetDID());
      delete tmpit2;
    }
  }

  int ncutclasses = m_cutpnl->ClassList->childCount();

  QString fname = FileName->text(), basename;
  if(fname!=""){
    basename = fname + "/" + modit->text(0) + "_QAReport";
    if(testTypeSel->currentItem()>0)
      basename += "_" + testTypeSel->currentText();
  }else
    basename = QString::null;

  QApplication::setOverrideCursor(Qt::waitCursor);
  ReportText->setText("<b>"+Headline->text()+"</b><br>");
  ReportText->repaint();
  if(m_app!=0) m_app->processEvents();

  // prepare - get std data items into array
  for(i=0;i<(int)std_arr.size();i++)
    std_arr[i] = NULL;
  datit = (ModItem*) modit->firstChild();
  while(datit!=NULL){
    if((i=datit->GetDID()->GetStdtest()%100)>=0 && 
       (testTypeSel->currentItem()==0 || testTypeSel->currentText()==tmpwin.prefix[datit->GetDID()->GetStdtest()/100]))
      std_arr[i] = datit; // found std-test item
    datit = (ModItem*) datit->nextSibling();
  }

  // get noise + threshold summary
  thrit = std_arr[6];
  if(thrit==0)
    thrit = std_arr[5];
//   if(thrit!=0){
//     BadPixel *thsumm = rmain->ThreshSumm(thrit, 0);
//     mod_thresh = thsumm->PixelList->text(16,1).toFloat();
//     mod_tdisp  = thsumm->PixelList->text(16,2).toFloat();
//     mod_noise  = thsumm->PixelList->text(16,3).toFloat();
//     mod_ndisp  = 30;
//     delete thsumm;
//   } else{
//     mod_thresh = 0;
//     mod_tdisp  = 0;
//     mod_noise  = 0;
//     mod_ndisp  = 0;
//   }

  //fill in result
  allpassed.resize(ncutclasses+1);
  tmppassed.resize(ncutclasses+1);
  valarr.resize(ncutclasses+1);
  m_badpix_hi.resize(ncutclasses+1);
  m_cutpnl->ClassList->setCurrentItem(m_cutpnl->ClassList->firstChild());
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  for(i=0;i<ncutclasses;i++){
    sprintf(hname,"badpixhi%d",i);
    sprintf(htit,"Bad pixel map, %s - %s",lvit->text(0).latin1(),testTypeSel->currentText().latin1());
    m_badpix_hi[i] = (TH2F*) gROOT->FindObject(hname);
    if(m_badpix_hi[i]!=NULL) m_badpix_hi[i]->Delete();
    m_badpix_hi[i] = new TH2F(hname,htit,NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
    allpassed[i] = true;
    lvit = (CutClassItem*)lvit->nextSibling();
  }
  // keep empty histo for no-mask cuts
  m_badpix_hi[ncutclasses] = new TH2F("nomask","NO mask",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);

  // first find bad pixels
  ReportText->setText(ReportText->text()+"<br><i>defect pixel tests</i><br><br>");
  ReportText->repaint();
  if(m_app!=0) m_app->processEvents();

  // digital - SRAM scan
  cutid = 0;
  hitocheck = 0;
  if(std_arr[0]!=0){
    hitocheck = std_arr[0]->GetDID()->GetMap(-1,RAW);
    if(std_arr[0]->GetDID()->GetLog()!=0)
      convert = ((float) std_arr[0]->GetDID()->GetLog()->MyScan->Nevents)/100;
    else
      convert = 100;
    cutname = std_arr[0]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);

  // digital - DFIFO scan
  cutid = 0;
  hitocheck = 0;
  if(std_arr[37]!=0){
    hitocheck = std_arr[37]->GetDID()->GetMap(-1,RAW);
    if(std_arr[37]->GetDID()->GetLog()!=0)
      convert = ((float) std_arr[37]->GetDID()->GetLog()->MyScan->Nevents)/100;
    else
      convert = 1;
    cutname = std_arr[37]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);

  // digital - masks from VDD scans
  int vddid;
  for(vddid = 23;vddid<28;vddid++){
    cutid = 0;
    hitocheck = 0;
    if(std_arr[vddid]!=0 && std_arr[vddid]->GetDID()->GetLog()!=0){
      hitocheck = new TH2F("tmpHits","temporary hitmap",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
      for(int chip=0;chip<NCHIP;chip++){
        for(int col=0;col<NCOL;col++){
          for(int row=0;row<NROW;row++){
            TGraphErrors *gr = std_arr[vddid]->GetDID()->GetScanHi(chip,col,row);
            if(gr!=0){
              int binx, biny, nLast;
              nLast = std_arr[vddid]->GetDID()->GetLog()->MyScan->Inner->Step-1;
              std_arr[vddid]->GetDataItem()->PixCCRtoBins(chip, col, row, binx, biny);
              float cont = 0;
              for(int spt=nLast-5;spt<=nLast;spt++){
                if(((int)gr->GetY()[spt])==std_arr[vddid]->GetDID()->GetLog()->MyScan->Nevents){
                  cont = (float)gr->GetY()[spt];
                  break;
                }
              }
              hitocheck->SetBinContent(binx,biny,cont);
            }
          }
        }
      }
        //std_arr[vddid]->GetDID()->Get2DMap(-1,RAW);
//       if(hitocheck==0){ // generate mask if not yet done so
//         // to do
//       }
//       convert = 1/100;
//       offset  = -1;
      convert = ((float) std_arr[vddid]->GetDID()->GetLog()->MyScan->Nevents)/100;
      offset = 0;
      cutname = std_arr[vddid]->GetDID()->GetName();
    }
    BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset);
    if(hitocheck) hitocheck->Delete();
  }

  // digital - DFIFO scan (if with ToT data)
  cutid = 10;
  hitocheck = 0;
  if(std_arr[37]!=0){
    hitocheck = std_arr[37]->GetDID()->GetMap(-1,TOTRAW);
    convert = 1;
    cutname = std_arr[37]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);

  // loose bumps: source hits
  hitocheck = NULL;
  ignore = NULL;
  cutid = 5;
  if(std_arr[14]!=NULL){
    std_arr[14]->GetDID()->LoadMask(); // loads pixels and CP that are off as mask
    hitocheck = std_arr[14]->GetDID()->GetMap(-1,TOT_HIT);
    ignore = std_arr[14]->GetDID()->GetMap(-1,RMASKP);
    convert = 1;
    cutname = std_arr[14]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, 0, ignore);
  
  // loose bumps: source ToT
  hitocheck = NULL;
  ignore = NULL;
  cutid = 6;
  if(std_arr[14]!=NULL){
    hitocheck = std_arr[14]->GetDID()->GetMap(-1,TOT_MAX);
    ignore = std_arr[14]->GetDID()->GetMap(-1,RMASKP);
    convert = 1;
    cutname = std_arr[14]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, 0, ignore);
  
  // hot/stuck: masked in src scan
  hitocheck = NULL;
  cutid = 7;
  if(std_arr[14]!=NULL){
    hitocheck = std_arr[14]->GetDID()->GetMap(-1,MAMASK);
    convert = 1;
    cutname = std_arr[14]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);
  
  // hot pixels from source scan
  hitocheck = NULL;
  cutid = 16;
  if(std_arr[14]!=NULL){
    hitocheck = std_arr[14]->GetDID()->GetMap(-1,TOT_RATE);
    convert = 1;
    cutname = std_arr[14]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);

  // analog dead: bad chi^2 from threshold scan
  hitocheck = NULL;
  cutid = 11;
  thrit = std_arr[6];
  if(thrit==0)
    thrit = std_arr[5];
  if(thrit!=0){
    convert = 1;
    hitocheck = thrit->GetDID()->GetMap(-1,CHI);
    cutname   = thrit->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);

  // anomalous ToT: ToT-vs-VCAL fit chi^2
  hitocheck = NULL;
  cutid = 14;
  if(std_arr[8]!=NULL){
    hitocheck = std_arr[8]->GetDID()->GetMap(-1,CHI);
    convert = 1;
    cutname = std_arr[8]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);

  // cross-talk
  hitocheck = NULL;
  cutid = 13;
  thrit = std_arr[6];
  if(thrit==0)
    thrit = std_arr[5];
  if(thrit!=0 && std_arr[7]!=NULL){
    // generate xtalk map
    TH2F *hia, *hib;
    hia = std_arr[7]->GetDID()->GetMap(-1,MEAN);
    hib = thrit->GetDID()->GetMap(-1,MEAN);
    float con, ref, minxt;
    int chip, col, row;
    if(hia!=NULL && hib!=NULL){
      hitocheck = new TH2F("xtmap","tmp map",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
      for(i=1;i<hia->GetNbinsX()+1;i++){
        for(j=1;j<hia->GetNbinsY()+1;j++){
          // guess minimum xtalk to replace empty bins
          std_arr[7]->GetDataItem()->PixXYtoCCR(i,j, &chip, &col, &row);
          float vcal, chi, vcmax;
          if(std_arr[7]->GetDID()->GetLog()!=0){
            vcal = std_arr[7]->GetDID()->GetLog()->MyModule->Chips[chip]->Vcal;
            chi  = std_arr[7]->GetDID()->GetLog()->MyModule->Chips[chip]->Chi;
            vcmax= (float)std_arr[7]->GetDID()->GetLog()->MyScan->Inner->Stop;
            if(vcal<0) vcal = 0.8f;
            if(chi<0)  chi  = 42;
            minxt = vcmax*chi*vcal/.160218f;
          } else
            minxt = 2e5;
          if(minxt<=0) minxt = 2e5;
          con = hia->GetBinContent(hia->GetBin(i,j));
          ref = hib->GetBinContent(hia->GetBin(i,j));
          ref *=50;          // 100=%, but xtalk comes from 2 pixels -> 50
          if(j==1 || j==320 || j==160 || j==161) 
            ref *= 2;          // unless pixel is on the edge
          if(con!=0) con = ref/con;
          else             con = ref/minxt;
          hitocheck->SetBinContent(hia->GetBin(i,j),con);
        }
      }
    }
    // rest is standard
    convert = 1;
    cutname = std_arr[7]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);
  xtalk = hitocheck;

  // merged bumps: cross-talk
  hitocheck = NULL;
  cutid = 3;
  thrit = std_arr[6];
  if(thrit==0)
    thrit = std_arr[5];
  if(thrit!=0 && std_arr[7]!=NULL){
    // generate xtalk map
    TH2F *hia, *hib, *hin;
    hia = std_arr[7]->GetDID()->GetMap(-1,MEAN);
    hib = thrit->GetDID()->GetMap(-1,MEAN);
    hin = std_arr[7]->GetDID()->GetMap(-1,SIG);
    float con, ref, noise, noisecut=999999;
    if(m_cutpnl->GetCutPars(4)->enabled)
      noisecut = (float)m_cutpnl->GetCutPars(4)->min;
    if(hia!=NULL && hib!=NULL){
      hitocheck = new TH2F("tmpfrmap","tmp map",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
      for(i=1;i<hia->GetNbinsX()+1;i++){
        for(j=1;j<hia->GetNbinsY()+1;j++){
          con   = hia->GetBinContent(hia->GetBin(i,j));
          ref   = hib->GetBinContent(hia->GetBin(i,j));
          noise = hin->GetBinContent(hia->GetBin(i,j));
          ref *=50;          // 100=%, but xtalk comes from 2 pixels -> 50
          if(j==1 || j==320 || j==160 || j==161) 
            ref *= 2;          // unless pixel is on the edge
          if(noise<noisecut && con>0)
            con = ref/con;
          else
            con = 0;
          hitocheck->SetBinContent(hia->GetBin(i,j),con);
        }
      }
    }
    // rest is standard
    convert = 1;
    cutname = std_arr[7]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);
  if(hitocheck!=NULL) hitocheck->Delete();
  
  // timewalk: in-time threshold (->overdrive)
  hitocheck = NULL;
  cutid = 15;
//   if(std_arr[15]!=NULL){
//     hitocheck = std_arr[15]->GetDID()->GetMap(-1,TWLK_OVERDRV_2D);
//     convert = 1;
//     cutname = std_arr[15]->GetDID()->GetName();
//   }
  if(std_arr[21]!=0 && (std_arr[6]!=0 || std_arr[5]!=0)){
    // generate thresh-diff. map
    TH2F *hia=0, *hib=0;
    hia = std_arr[21]->GetDID()->GetMap(-1,MEAN);
    if(std_arr[5]!=0) // normal threshold scan
      hib = std_arr[5]->GetDID()->GetMap(-1,MEAN);
    else if(std_arr[6]!=0) // antikill threshold scan
      hib = std_arr[6]->GetDID()->GetMap(-1,MEAN);
    float con, ref;
    if(hia!=0 && hib!=0){
      hitocheck = new TH2F("odmap","tmp map",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
      for(i=1;i<hia->GetNbinsX()+1;i++){
        for(j=1;j<hia->GetNbinsY()+1;j++){
          con = hia->GetBinContent(hia->GetBin(i,j));
          ref = hib->GetBinContent(hia->GetBin(i,j));
          con -= ref;
          hitocheck->SetBinContent(hia->GetBin(i,j),con);
        }
      }
    }
    convert = 1;
    cutname = std_arr[21]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);
  if(hitocheck!=0) hitocheck->Delete();

  // leakage current
  hitocheck = NULL;
  cutid = 12;
  if(std_arr[17]!=NULL){ // default: monleak at max Vbias
    hitocheck = std_arr[17]->GetDID()->GetMap(-1,LEAK_PLOT);
    cutname = std_arr[17]->GetDID()->GetName();
  }else if(std_arr[10]!=NULL){ // fall-back: monleak at 150V
    hitocheck = std_arr[10]->GetDID()->GetMap(-1,LEAK_PLOT);
    cutname = std_arr[10]->GetDID()->GetName();
  }
  convert = 0;
  offset  = 0;
  ncnt    = 0;
  if(hitocheck!=0){
    for(i=1;i<hitocheck->GetNbinsX()+1;i++){
      for(j=1;j<hitocheck->GetNbinsY()+1;j++){
        con   = hitocheck->GetBinContent(i,j);
        if(con>-100 && con<100){
          convert += con*con;
          offset  += con;
          ncnt++;
        }
      }
    }
    if(ncnt>0){
      offset /= ncnt;
      convert = sqrt(convert/ncnt-offset*offset);
    }
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset);
  
  // loose bumps: threshold scan HVoff-HVon
  hitocheck = NULL;
  cutid = 1;
  if(std_arr[1]!=NULL){
    // generate noise-diff. map
    TH2F *hia, *hib;
    hia = std_arr[1]->GetDID()->GetMap(-1,SIG);
    if(std_arr[2]!=NULL) // dedicated HV on scan
      hib = std_arr[2]->GetDID()->GetMap(-1,SIG);
    else if(std_arr[6]!=NULL) // antikill threshold scan
      hib = std_arr[6]->GetDID()->GetMap(-1,SIG);
    else if(std_arr[5]!=NULL) // normal threshold scan
      hib = std_arr[5]->GetDID()->GetMap(-1,SIG);
    else
      hib = hia;
    float con, ref, noisecut=999999;
    if(m_cutpnl->GetCutPars(2)->enabled)
      noisecut = (float)m_cutpnl->GetCutPars(2)->min;
    if(hia!=NULL && hib!=NULL){
      hitocheck = new TH2F("tmpfrmap","tmp map",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
      for(i=1;i<hia->GetNbinsX()+1;i++){
        for(j=1;j<hia->GetNbinsY()+1;j++){
          con = hia->GetBinContent(hia->GetBin(i,j));
          ref = hib->GetBinContent(hia->GetBin(i,j));
          if(con<noisecut && ref<noisecut && ref>0 && con>0)
            con = TMath::Abs(con - ref);
          else
            con = 1e8;
          hitocheck->SetBinContent(hia->GetBin(i,j),con);
        }
      }
    }
    // rest is standard
    convert = 1;
    cutname = std_arr[1]->GetDID()->GetName();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert);
  if(hitocheck!=NULL) hitocheck->Delete();
  
  // noise: noise from S-curve (sigma cut)
  // first: all normal pixels
  hitocheck = 0;
  cutid = 8;
  thrit = std_arr[6];
  if(thrit==0)
    thrit = std_arr[5];
  if(thrit!=0){
    int mychip, mycol, myrow;
    hitocheck = new TH2F(*thrit->GetDID()->GetMap(-1,SIG));
    hitocheck->SetName("tmphi");
    // cut relative to mean noise -> get mean & RMS
    convert = 0;
    offset  = 0;
    ncnt    = 0;
    if(hitocheck!=0){
      for(i=1;i<hitocheck->GetNbinsX()+1;i++){
        for(j=1;j<hitocheck->GetNbinsY()+1;j++){
          con   = hitocheck->GetBinContent(i,j);
          thrit->GetDID()->PixXYtoCCR(i-1,j-1,&mychip,&mycol,&myrow);
          if(mycol==0 || mycol==17 || myrow>152){ // not normal pixel
            con = 0;
            hitocheck->SetBinContent(i,j,0);
          }
          if(con>0){
            convert += con*con;
            offset  += con;
            ncnt++;
          }
        }
      }
      if(ncnt>0){
        offset /= ncnt;
        convert = sqrt(convert/ncnt-offset*offset);
      }
    }
    cutname = (QString(thrit->GetDID()->GetName())+" (normal)").latin1();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset);
  delete hitocheck;
  // noise: noise from S-curve (sigma cut)
  // then: all ganged pixels
  hitocheck = 0;
  cutid = 8;
  if(thrit!=0){
    int mychip, mycol, myrow;
    hitocheck = new TH2F(*thrit->GetDID()->GetMap(-1,SIG));
    hitocheck->SetName("tmphi");
    // cut relative to mean noise -> get mean & RMS
    convert = 0;
    offset  = 0;
    ncnt    = 0;
    if(hitocheck!=0){
      for(i=1;i<hitocheck->GetNbinsX()+1;i++){
        for(j=1;j<hitocheck->GetNbinsY()+1;j++){
          con   = hitocheck->GetBinContent(i,j);
          thrit->GetDID()->PixXYtoCCR(i-1,j-1,&mychip,&mycol,&myrow);
          if(myrow<153 || myrow%2==0){// not ganged
            con = 0;
            hitocheck->SetBinContent(i,j,0);
          }
          if(con>0){
            convert += con*con;
            offset  += con;
            ncnt++;
          }
        }
      }
      if(ncnt>0){
        offset /= ncnt;
        convert = sqrt(convert/ncnt-offset*offset);
      }
    }
    cutname = (QString(thrit->GetDID()->GetName())+" (ganged)").latin1();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset);
  delete hitocheck;
  // noise: noise from S-curve (sigma cut)
  // then: all ganged pixels
  hitocheck = 0;
  cutid = 8;
  if(thrit!=0){
    int mychip, mycol, myrow;
    hitocheck = new TH2F(*thrit->GetDID()->GetMap(-1,SIG));
    hitocheck->SetName("tmphi");
    // cut relative to mean noise -> get mean & RMS
    convert = 0;
    offset  = 0;
    ncnt    = 0;
    if(hitocheck!=0){
      for(i=1;i<hitocheck->GetNbinsX()+1;i++){
        for(j=1;j<hitocheck->GetNbinsY()+1;j++){
          con   = hitocheck->GetBinContent(i,j);
          thrit->GetDID()->PixXYtoCCR(i-1,j-1,&mychip,&mycol,&myrow);
          if(myrow<153 || myrow%2!=0){// not inter-ganged
            con = 0;
            hitocheck->SetBinContent(i,j,0);
          }
          if(con>0){
            convert += con*con;
            offset  += con;
            ncnt++;
          }
        }
      }
      if(ncnt>0){
        offset /= ncnt;
        convert = sqrt(convert/ncnt-offset*offset);
      }
    }
    cutname = (QString(thrit->GetDID()->GetName())+" (inter-ganged)").latin1();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset);
  delete hitocheck;
  // noise: noise from S-curve (sigma cut)
  // then: all long pixels
  hitocheck = 0;
  cutid = 8;
  if(thrit!=0){
    int mychip, mycol, myrow;
    hitocheck = new TH2F(*thrit->GetDID()->GetMap(-1,SIG));
    hitocheck->SetName("tmphi");
    // cut relative to mean noise -> get mean & RMS
    convert = 0;
    offset  = 0;
    ncnt    = 0;
    if(hitocheck!=0){
      for(i=1;i<hitocheck->GetNbinsX()+1;i++){
        for(j=1;j<hitocheck->GetNbinsY()+1;j++){
          con   = hitocheck->GetBinContent(i,j);
          thrit->GetDID()->PixXYtoCCR(i-1,j-1,&mychip,&mycol,&myrow);
          if((mycol>0 && mycol<17) || myrow>152){// not long
            con = 0;
            hitocheck->SetBinContent(i,j,0);
          }
          if(con>0){
            convert += con*con;
            offset  += con;
            ncnt++;
          }
        }
      }
      if(ncnt>0){
        offset /= ncnt;
        convert = sqrt(convert/ncnt-offset*offset);
      }
    }
    cutname = (QString(thrit->GetDID()->GetName())+" (long)").latin1();
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset);
  delete hitocheck;
  
  // non-tunable: sigma-cut on threshold
  hitocheck = NULL;
  cutid = 9;
  offset = 0;
  thrit = std_arr[6];
  if(thrit==0)
    thrit = std_arr[5];
  if(thrit!=0){
    // rest is standard
    hitocheck = thrit->GetDID()->GetMap(-1,MEAN);
    cutname   = thrit->GetDID()->GetName();
  }
  // cut relative to mean thresh. -> get mean & RMS
  convert = 0;
  offset  = 0;
  ncnt    = 0;
  if(hitocheck!=0){
    for(i=1;i<hitocheck->GetNbinsX()+1;i++){
      for(j=1;j<hitocheck->GetNbinsY()+1;j++){
        con   = hitocheck->GetBinContent(i,j);
        if(con>0){
          convert += con*con;
          offset  += con;
          ncnt++;
        }
      }
    }
    if(ncnt>0){
      offset /= ncnt;
      convert = sqrt(convert/ncnt-offset*offset);
    }
  }
  BadPixelTest(cutid, cutname.latin1(), hitocheck, convert, offset, 0, true);

  // digital - EOC scan
  cutid = 17;
  hitocheck = 0;
  TH2F *hinohit=0;
  if(std_arr[38]!=0 && std_arr[38]->GetDID()->GetLog()!=0){
    //create histogram that only shows the relevant col pair
    hitocheck = new TH2F("tmpHits","temporary hitmap",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
    hinohit   = new TH2F("tmp2Hits","temporary no-hitmap",NCOL*NCHIP/2,-.5,143.5,2*NROW,-.5,319.5);
    for(int chip=0;chip<NCHIP;chip++){
      for(int col=0;col<NCOL;col++){
        for(int row=0;row<NROW;row++){
          TGraphErrors *gr = std_arr[38]->GetDID()->GetScanHi(chip,col,row);
          if(gr!=0){
            int binx, biny;
            std_arr[38]->GetDataItem()->PixCCRtoBins(chip, col, row, binx, biny);
            float cont = (float)gr->GetY()[col/2];
            hitocheck->SetBinContent(binx,biny,cont);
            cont = 0;
            for(int scpt=0;scpt<std_arr[38]->GetDID()->GetLog()->MyScan->Inner->Step;scpt++){
              if(scpt!=col/2)
                cont += (float)gr->GetY()[scpt];
            }
            hinohit->SetBinContent(binx,biny,cont);
          }
        }
      }
    }
    convert = ((float) std_arr[38]->GetDID()->GetLog()->MyScan->Nevents)/100;
    cutname = std_arr[38]->GetDID()->GetName();
  }
  BadPixelTest(cutid, (cutname + " (scanned pair)").latin1(), hitocheck, convert);
  if(hitocheck!=0) hitocheck->Delete();
  cutid = 18;
  BadPixelTest(cutid, (cutname + " (non-scanned pairs)").latin1(), hinohit, convert);
  if(hinohit!=0) hinohit->Delete();

//   // non-tunable: absolute cut on threshold
//   hitocheck = NULL;
//   cutid = 10;
//   thrit = std_arr[6];
//   if(thrit==0)
//     thrit = std_arr[5];
//   if(thrit!=0){
//     convert = 1;
//     offset = mod_thresh;
//     hitocheck = thrit->GetDID()->GetMap(-1,MEAN);
//     cutname   = thrit->GetDID()->GetName();
//   }
//   BadPixelTest(cutid, cutname.latin1(), hitocheck, convert,offset);

  // plot bad pixel maps, remember items for later
  //TCanvas *can = (TCanvas*)gROOT->FindObject("pixcan");
  fname = basename + "_badpix.ps";
  TCanvas *can;
  //TPostScript *psfile;
  //psfile= new TPostScript(fname.latin1(),-111);
  //rmain->m_psfile = psfile;
  int rderr;
  DatSet *tmpdat;
  m_badpixit.resize(ncutclasses+1);
  rmain->ColourScale(1);
  for(i=0;i<=ncutclasses;i++){
    tmpdat = new DatSet(m_badpix_hi[i]->GetTitle(),MASKDAT,false,0,&rderr);
    tmpdat->CreateMask(m_badpix_hi[i]);
    m_badpixit[i] = m_parent->DataToTree(modit, tmpdat);
    if(i!=ncutclasses){
      ModItem *tmpplt = (ModItem*) m_badpixit[i]->firstChild();
      if(tmpplt->GetPID()!=MAMASK)
        tmpplt = (ModItem*)tmpplt->nextSibling();
      if(tmpplt!=0){
        rderr = rmain->Plot2DColour(tmpplt);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          if(ncutclasses==1) fname = basename + "_badpix.ps";
          else               fname = basename + "_badpix_cc" + QString::number(i+1) + ".ps";
          can->Print(fname.latin1());
        }
      }
    }
  }  
  rmain->ColourScale(m_parent->options->m_Opts->m_colourScale);

  // then cut on entire-module properties
  ReportText->setText(ReportText->text() + "<br><i>overall module tests</i><br><br>");
  ReportText->repaint();
  if(m_app!=0) m_app->processEvents();
  //fname = basename + "_module.ps";
  //psfile = new TPostScript(fname.latin1(),-111);
  //rmain->m_psfile = psfile;

  // tot # bad pixels, data storage
  cutid = 0;
  std::vector<long int> vplotid;
  for(i=0;i<ncutclasses;i++){
    valarr[i] = (float)m_badpix_hi[i]->GetEntries();
    m_nBadPix = (int)valarr[m_selcutID];
    m_badpix_hi[i]->Delete();
    vplotid.push_back((long int)m_badpixit[i]->findPlotItem("Plot of selected defects"));
  }
  m_badpix_hi[ncutclasses]->Delete();
  m_badpix_hi.clear();
  cutname = "Number of bad pixels";
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, vplotid);

  // sensor break-down voltage
  cutid = 1;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  bool plotTest = false;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    if(m_cutpnl->GetCutPars(1+m_cutpnl->npcuts)->enabled) plotTest = true;
    if(m_cutpnl->GetCutPars(2+m_cutpnl->npcuts)->enabled) plotTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  if(std_arr[3]!=NULL){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    double x,y,xp,yp;
    TGraph *gr = std_arr[3]->GetDID()->GetGraph();//GetCorrIV();
    if(gr==NULL)
      gr = std_arr[3]->GetDID()->GetGraph();
    for(i=1;i<gr->GetN();i++){
      gr->GetPoint(i,x,y);
      gr->GetPoint(i-1,xp,yp);
      //if(y>50) break;
      if(xp>10 && (x-xp)!=0)
        y = (y-yp)/(x-xp);
      else
        y = 0;
      if(y>0.25) break;
    }
    valarr[1] = (float) x;
    cutname = std_arr[3]->GetDID()->GetName();
    ModItem *tmpplt = new ModItem(std_arr[3],  "Plot IV curve (raw data)",  std_arr[3]->GetDID(),IVPLOT);
    if(plotTest){
      rmain->PlotGraph(tmpplt);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
	fname = basename + "_sensorIV.ps";
	can->Print(fname.latin1());
      }
    }
    delete tmpplt;
    plotid = (long int)std_arr[3]->findPlotItem("Plot IV curve (raw data)");
  }
  m_IV_OK = ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
  m_IV_bdV = valarr[1];    

  // sensor current at 150V
  cutid = 2;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[3]!=NULL){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    double curr, diff100=1e3, diff150=1e3, x,y;
    int pnt100=-1, pnt150=-1;
    TGraph *gr;// = std_arr[3]->GetDID()->GetCorrIV();
    //    if(gr==NULL)
    gr = std_arr[3]->GetDID()->GetGraph();
    for(i=0;i<gr->GetN();i++){
      gr->GetPoint(i,x,y);
      if(fabs(x-150)<diff150){
        pnt150 = i;
        diff150 = fabs(x-150);
      }
      if(fabs(x-100)<diff100){
        pnt100 = i;
        diff100 = fabs(x-100);
      }
    }
    // current at 100V
    gr->GetPoint(pnt100,x,curr);
    m_IV_100=curr;
    // current at 150V (what we cut on)
    gr->GetPoint(pnt150,x,curr);
    valarr[1] = (float) curr;
    m_IV_oper=curr;
    // i at 600V or at compliance (= last point)
    //if(x==600 || (y==COMP && i_600==600)) i_600=y;
    gr->GetPoint(gr->GetN()-1,x,curr);
    m_IV_max = curr;
    cutname = std_arr[3]->GetDID()->GetName();
    plotid = (long int)std_arr[3]->findPlotItem("Plot IV curve (raw data)");
  }
  m_IV_OK &= ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // loop threshold/noise test over both threshold scans
  std::vector<float> tdisp;
  tdisp.resize(ncutclasses);
  for(int thrID=5;thrID<7;thrID++){
    // avg. threshold
    cutid = 3;
    valarr[0] = 10*UNDEFCUT; 
    plotid = 0;
    thrit = std_arr[thrID];
    if(thrit!=0){
      // get masked threshold plot
      TH1F *tmphi;
      if(m_cutpnl->IgnoreBadPix->isChecked()){
	CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
        for(i=0;i<ncutclasses;i++){
	  m_cutpnl->ClassList->setCurrentItem(lvit);
	  if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	    rmain->PlotMasked(thrit, m_badpixit[i], MEAN, MAMASK, false,-1,0,bit_mask);
	    can = (TCanvas*) gROOT->FindObject("pixcan");
	    if(!basename.isEmpty()){
	      if(ncutclasses==1)
		fname = basename + "_thresh.ps";
	      else
		fname = basename + "_thresh_cc" + QString::number(i+1) + ".ps";
	      can->Print(fname.latin1());
	    }
	    tmphi = (TH1F*)gROOT->FindObject("umskhist");
	    if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	    //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
	    if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	      valarr[i] = tmphi->GetFunction("gaus")->GetParameter(1);
	      tdisp[i] = tmphi->GetFunction("gaus")->GetParameter(2);
	    }else{ // distribution might not be Gaussian, get arithmetic values then
	      valarr[i] = tmphi->GetMean();
	      tdisp[i] = tmphi->GetRMS();
	    }
	  } else{
	    valarr[i] = 0;
	    tdisp[i] = 0;
	  }
	  lvit = (CutClassItem*)lvit->nextSibling();
	}
	m_avgThr = valarr[m_selcutID];
	m_sdThr  = tdisp[m_selcutID];
      } else{
        rmain->PlotMasked(thrit, m_badpixit[ncutclasses], MEAN, MAMASK, false,-1,0,bit_mask);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          fname = basename + "_thresh.ps";
          can->Print(fname.latin1());
        }
        tmphi = (TH1F*)gROOT->FindObject("umskhist");
        if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
        if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
          valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
          tdisp[0] = tmphi->GetFunction("gaus")->GetParameter(2);
        }else{
          valarr[1] = tmphi->GetMean();
          tdisp[0] = tmphi->GetRMS();
        }
        m_avgThr = valarr[1];
        m_sdThr  = tdisp[0];
      }
      cutname = thrit->GetDID()->GetName();
      plotid = (long int)thrit->findPlotItem("Plot threshold");
    }
    m_ANA_OK &= ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
    
    // threshold disp.
    cutid = 4;
    valarr[0] = 10*UNDEFCUT; 
    plotid = 0;
    if(thrit!=0){
      if(m_cutpnl->IgnoreBadPix->isChecked()){
        for(i=0;i<ncutclasses;i++)
          valarr[i] = tdisp[i];
      } else{
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = tdisp[0];
      }
      cutname = thrit->GetDID()->GetName();
      plotid = (long int)thrit->findPlotItem("Plot threshold");
    }
    m_ANA_OK &= ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
    
    // avg. noise
    cutid = 5;
    valarr[0] = 10*UNDEFCUT; 
    plotid = 0;
    if(thrit!=0){
      // get masked noise plot
      TH1F *tmphi;
      if(m_cutpnl->IgnoreBadPix->isChecked()){
	CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
        for(i=0;i<ncutclasses;i++){
	  m_cutpnl->ClassList->setCurrentItem(lvit);
	  if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	    rmain->PlotMasked(thrit, m_badpixit[i], SIG, MAMASK, false,-1,0,bit_mask);
	    can = (TCanvas*) gROOT->FindObject("pixcan");
	    if(!basename.isEmpty()){
	      if(ncutclasses==1)
		fname = basename + "_noise.ps";
	      else
		fname = basename + "_noise_cc" + QString::number(i+1) + ".ps";
	      can->Print(fname.latin1());
	    }
	    tmphi = (TH1F*)gROOT->FindObject("umskhist");
	    if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	    //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
	    if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	      valarr[i] = tmphi->GetFunction("gaus")->GetParameter(1);
	    }else{ // distribution might not be Gaussian, get arithmetic values then
	      valarr[i] = tmphi->GetMean();
	    }
	  } else
	    valarr[i] = 0;
	  lvit = (CutClassItem*)lvit->nextSibling();
        }
        m_avgNse = valarr[m_selcutID];
      } else{
        rmain->PlotMasked(thrit, m_badpixit[ncutclasses], SIG, MAMASK, false,-1,0,bit_mask);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          fname = basename + "_noise.ps";
          can->Print(fname.latin1());
        }
        tmphi = (TH1F*)gROOT->FindObject("umskhist");
        if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
        if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
          valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
        }else{
          valarr[1] = tmphi->GetMean();
        }
        valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
        m_avgNse = valarr[1];
      }
      cutname = thrit->GetDID()->GetName();
      plotid = (long int)thrit->findPlotItem("Plot noise");
    }
    m_ANA_OK &= ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
  }

  // in-time stuff preparation
  std::vector<float> thresh;
  thresh.resize(ncutclasses);
  cutid = 6;
//  if(std_arr[15]!=NULL){
  if(std_arr[21]!=NULL){
    // get masked threshold plot
    TH1F *tmphi;
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      for(i=0;i<ncutclasses;i++){
	m_cutpnl->ClassList->setCurrentItem(lvit);
	if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	  rmain->PlotMasked(std_arr[21], m_badpixit[i], MEAN, MAMASK, false,-1,0,bit_mask);
	  can = (TCanvas*) gROOT->FindObject("pixcan");
	  if(!basename.isEmpty()){
	    if(ncutclasses==1)
	      fname = basename + "_int-thresh.ps";
	    else
	      fname = basename + "_int-thresh_cc" + QString::number(i+1) + ".ps";
	    can->Print(fname.latin1());
	  }
	  tmphi = (TH1F*)gROOT->FindObject("umskhist");
	  if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	  //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
	  if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	    thresh[i] = tmphi->GetFunction("gaus")->GetParameter(1);
	    tdisp[i] = tmphi->GetFunction("gaus")->GetParameter(2);
	  }else{ // distribution might not be Gaussian, get arithmetic values then
	    thresh[i] = tmphi->GetMean();
	    tdisp[i] = tmphi->GetRMS();
	  }
	} else{
	  thresh[i] = 0;
	  tdisp[i] = 0;
	}
	lvit = (CutClassItem*)lvit->nextSibling();
      }
    } else{
      //      rmain->PlotMasked(std_arr[15], m_badpixit[ncutclasses], TWLK_Q20NS_2D, MAMASK, false);
      rmain->PlotMasked(std_arr[21], m_badpixit[ncutclasses], MEAN, MAMASK, false,-1,0,bit_mask);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
        fname = basename + "_int-thresh.ps";
        can->Print(fname.latin1());
      }
      tmphi = (TH1F*)gROOT->FindObject("umskhist");
      if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
      if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
        thresh[0] = tmphi->GetFunction("gaus")->GetParameter(1);
        tdisp[0] = tmphi->GetFunction("gaus")->GetParameter(2);
      }else{
        thresh[0] = tmphi->GetMean();
        tdisp[0] = tmphi->GetRMS();
      }
    }
  }

  // avg. in-time threshold
  cutid = 6;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[21]!=NULL){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++){
        valarr[i] = thresh[i];
      }
      m_avgITThresh = valarr[m_selcutID];
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      valarr[1] = thresh[0];
      m_avgITThresh = valarr[1];
    }
    cutname = std_arr[21]->GetDID()->GetName();
    plotid = (long int)std_arr[21]->findPlotItem("Plot threshold");
  }
  m_ANA_OK &= ModuleClass(cutid, cutname.latin1(), valarr, allpassed,plotid);

  // avg. in-time threshold disp.
  cutid = 7;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[21]!=NULL){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++){
        valarr[i] = tdisp[i];
      }
      m_sdITThresh = valarr[m_selcutID];
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      valarr[1] = tdisp[0];
      m_sdITThresh = valarr[1];
    }
    cutname = std_arr[21]->GetDID()->GetName();
    plotid = (long int)std_arr[21]->findPlotItem("Plot threshold");
  }
  m_ANA_OK &= ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // ditto for timewalk scan data

  // in-time stuff preparation
  cutid = 6;
  thresh.clear();
  thresh.resize(ncutclasses);
  if(std_arr[15]!=NULL){
  // if(std_arr[21]!=NULL){
    // get masked threshold plot
    TH1F *tmphi;
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      for(i=0;i<ncutclasses;i++){
	m_cutpnl->ClassList->setCurrentItem(lvit);
	if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	  rmain->PlotMasked(std_arr[15], m_badpixit[i], TWLK_Q20NS_2D, MAMASK, false);
	  //        rmain->PlotMasked(std_arr[21], m_badpixit[i], MEAN, MAMASK, false,-1,0,bit_mask);
	  can = (TCanvas*) gROOT->FindObject("pixcan");
	  if(!basename.isEmpty()){
	    if(ncutclasses==1)
	      fname = basename + "_int-thresh_TW.ps";
	    else
	      fname = basename + "_int-thresh_TW_cc" + QString::number(i+1) + ".ps";
	    can->Print(fname.latin1());
	  }
	  tmphi = (TH1F*)gROOT->FindObject("umskhist");
	  if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	  //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
	  if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	    thresh[i] = tmphi->GetFunction("gaus")->GetParameter(1);
	    tdisp[i] = tmphi->GetFunction("gaus")->GetParameter(2);
	  }else{ // distribution might not be Gaussian, get arithmetic values then
	    thresh[i] = tmphi->GetMean();
	    tdisp[i] = tmphi->GetRMS();
	  }
	} else{
	  thresh[i] = 0;
	  tdisp[i] = 0;
	}
	lvit = (CutClassItem*)lvit->nextSibling();
      }
    } else{
      rmain->PlotMasked(std_arr[15], m_badpixit[ncutclasses], TWLK_Q20NS_2D, MAMASK, false);
      //rmain->PlotMasked(std_arr[21], m_badpixit[ncutclasses], MEAN, MAMASK, false,-1,0,bit_mask);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
        fname = basename + "_int-thresh_TW.ps";
        can->Print(fname.latin1());
      }
      tmphi = (TH1F*)gROOT->FindObject("umskhist");
      if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
      if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
        thresh[0] = tmphi->GetFunction("gaus")->GetParameter(1);
        tdisp[0] = tmphi->GetFunction("gaus")->GetParameter(2);
      }else{
        thresh[0] = tmphi->GetMean();
        tdisp[0] = tmphi->GetRMS();
      }
    }
  }

  // avg. in-time threshold
  cutid = 6;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[15]!=NULL){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++){
        valarr[i] = thresh[i];
      }
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      valarr[1] = thresh[0];
    }
    cutname = std_arr[15]->GetDID()->GetName();
    plotid = (long int)std_arr[15]->findPlotItem("Plot Q(t0-20ns)");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed,plotid);


  // now the dummy cut for the timewalk raw data
  cutid = 14;
  // Clow
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[11]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[11]->GetDID()->GetName();
    plotid = (long int)std_arr[11]->findPlotItem("Plot");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  // Chigh
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[12]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[12]->GetDID()->GetName();
    plotid = (long int)std_arr[12]->findPlotItem("Plot");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);

  // avg. cross-talk
  cutid = 8;
  valarr[0] = 10*UNDEFCUT; 
  thrit = std_arr[6];
  plotid = 0;
  if(thrit==0)
    thrit = std_arr[5];
  if(thrit!=0 && std_arr[7]!=NULL){
    ModItem *tmpplt = new ModItem(std_arr[7],"tmp crosstalk fraction", 
                                  std_arr[7]->GetDID(), XTFRNN, thrit->GetDID());
    // get masked noise plot
    TH1F *tmphi;
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      for(i=0;i<ncutclasses;i++){
	m_cutpnl->ClassList->setCurrentItem(lvit);
	if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	  rmain->PlotMasked(tmpplt, m_badpixit[i], XTFRNN, MAMASK, false,-1,0,bit_mask);
	  can = (TCanvas*) gROOT->FindObject("pixcan");
	  if(!basename.isEmpty()){
	    if(ncutclasses==1)
	      fname = basename + "_xtalk.ps";
	    else
	      fname = basename + "_xtalk_cc" + QString::number(i+1) + ".ps";
	    can->Print(fname.latin1());
	  }
	  tmphi = (TH1F*)gROOT->FindObject("umskhist");
	  if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	  //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
	  //         if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	  //           valarr[i] = tmphi->GetFunction("gaus")->GetParameter(1);
	  //         }else{ // distribution might not be Gaussian, get arithmetic values then
          valarr[i] = tmphi->GetMean();
          //        }
	} else
	  valarr[i] = 0;
	lvit = (CutClassItem*)lvit->nextSibling();
      }
      m_avgXtalk = valarr[m_selcutID];
    } else{
      rmain->PlotMasked(tmpplt, m_badpixit[ncutclasses], XTFR, MAMASK, false,-1,0,bit_mask);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
        fname = basename + "_xtalk.ps";
        can->Print(fname.latin1());
      }
      tmphi = (TH1F*)gROOT->FindObject("umskhist");
      if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      //if(TMath::Prob(tmphi->GetFunction("gaus")->GetChisquare(),tmphi->GetFunction("gaus")->GetNDF())>1e-2){
//       if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
//         valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
//       }else{
        valarr[1] = tmphi->GetMean();
        //      }
      //valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
      m_avgXtalk = valarr[1];
    }
    //    std_arr[7]->GetDID()->m_parmap[2] = NULL;
    delete tmpplt;
    cutname = std_arr[7]->GetDID()->GetName();
    plotid = (long int)std_arr[7]->findPlotItem("Plot cross-talk fraction");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
  
  // ToT stuff preparation
  cutid = 9;
  int ToT_IDs[3]={8,9,19};
  std::string ToT_labels[3]={"clo","chi","comb"};
  int ttype, store_totpnt = rmain->m_plotOpts->m_ToTPlotPnt;
  rmain->m_plotOpts->m_ToTPlotPnt = 23; // 23ke = mip?
  for(int tcnt=0;tcnt<3;tcnt++){
    ttype = 0;
    if(std_arr[ToT_IDs[tcnt]]!=0)
      ttype = std_arr[ToT_IDs[tcnt]]->GetDID()->m_istype;
    if(std_arr[ToT_IDs[tcnt]]!=0 && (ttype==TOTF || ttype==TOTFR || ttype==TOTCF)){
      // store user-define target and write mip-value into it for this plot
      // get masked ToT plot
      TH1F *tmphi;
      if(m_cutpnl->IgnoreBadPix->isChecked()){
	CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
        for(i=0;i<ncutclasses;i++){
	  m_cutpnl->ClassList->setCurrentItem(lvit);
	  if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	    rmain->PlotMasked(std_arr[ToT_IDs[tcnt]], m_badpixit[i], T20K, MAMASK, false,-1,0,bit_mask);
	    can = (TCanvas*) gROOT->FindObject("pixcan");
	    if(!basename.isEmpty()){
	      if(ncutclasses==1)
		fname = basename + "_ToT"+ToT_labels[tcnt].c_str()+"_mip.ps";
	      else
		fname = basename + "_ToT"+ToT_labels[tcnt].c_str()+"_mip_cc" + QString::number(i+1) + ".ps";
	      can->Print(fname.latin1());
	    }
	    tmphi = (TH1F*)gROOT->FindObject("umskhist");
	    if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	    if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	      thresh[i] = tmphi->GetFunction("gaus")->GetParameter(1);
	      tdisp[i] = tmphi->GetFunction("gaus")->GetParameter(2);
	    }else{ // distribution might not be Gaussian, get arithmetic values then
	      thresh[i] = tmphi->GetMean();
	      tdisp[i] = tmphi->GetRMS();
	    }
	  } else{
	    thresh[i] = 0;
	    tdisp[i] = 0;
	  }
	  lvit = (CutClassItem*)lvit->nextSibling();
        }
      } else{
        rmain->PlotMasked(std_arr[ToT_IDs[tcnt]], m_badpixit[ncutclasses], T20K, MAMASK, false,-1,0,bit_mask);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          fname = basename + "_ToT"+ToT_labels[tcnt].c_str()+"_mip.ps";
          can->Print(fname.latin1());
        }
        tmphi = (TH1F*)gROOT->FindObject("umskhist");
        if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
          thresh[0] = tmphi->GetFunction("gaus")->GetParameter(1);
          tdisp[0] = tmphi->GetFunction("gaus")->GetParameter(2);
        }else{
          thresh[0] = tmphi->GetMean();
          tdisp[0] = tmphi->GetRMS();
        }
      }
    }
    
    // avg. ToT
    cutid = 9;
    valarr[0] = 10*UNDEFCUT; 
    plotid = 0;
    ttype = 0;
    if(std_arr[ToT_IDs[tcnt]]!=0)
      ttype = std_arr[ToT_IDs[tcnt]]->GetDID()->m_istype;
    if(std_arr[ToT_IDs[tcnt]]!=0 && (ttype==TOTF || ttype==TOTFR || ttype==TOTCF)){
      if(m_cutpnl->IgnoreBadPix->isChecked()){
        for(i=0;i<ncutclasses;i++){
          valarr[i] = thresh[i];
        }
        if(tcnt==0) m_avgTOT = valarr[m_selcutID];
      } else{
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = thresh[0];
        if(tcnt==0) m_avgTOT = valarr[1];
      }
      cutname = std_arr[ToT_IDs[tcnt]]->GetDID()->GetName();
      plotid = (long int)std_arr[ToT_IDs[tcnt]]->findPlotItem("Plot ToT(Q_target) from fit par\'s");
    }
    ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
    
    // ToT disp.
    cutid = 10;
    valarr[0] = 10*UNDEFCUT; 
    plotid = 0;
    ttype = 0;
    if(std_arr[ToT_IDs[tcnt]]!=0)
      ttype = std_arr[ToT_IDs[tcnt]]->GetDID()->m_istype;
    if(std_arr[ToT_IDs[tcnt]]!=0 && (ttype==TOTF || ttype==TOTFR || ttype==TOTCF)){
      if(m_cutpnl->IgnoreBadPix->isChecked()){
        for(i=0;i<ncutclasses;i++){
          valarr[i] = tdisp[i];
        }
        if(tcnt==0) m_sdTOT = valarr[m_selcutID];
      } else{
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = tdisp[0];
        if(tcnt==0) m_sdTOT = valarr[1];
      }
      cutname = std_arr[ToT_IDs[tcnt]]->GetDID()->GetName();
      plotid = (long int)std_arr[ToT_IDs[tcnt]]->findPlotItem("Plot ToT(Q_target) from fit par\'s");
    }
    ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
  }
  // return to old user Q_target value
  rmain->m_plotOpts->m_ToTPlotPnt = store_totpnt;

  // analogue T0 - no cut for now
  cutid = 11;
  valarr[0] = 10*UNDEFCUT; 
  if(std_arr[35]!=0){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++)
        valarr[i] = 0;
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      valarr[1] = 0;
    }
    cutname = std_arr[35]->GetDID()->GetName();
    plotid = (long int)std_arr[35]->findPlotItem("Plot delay time");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // digital T0 - no cut for now
  cutid = 12;
  valarr[0] = 10*UNDEFCUT; 
  if(std_arr[34]!=0){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++)
        valarr[i] = 0;
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      valarr[1] = 0;
    }
    cutname = std_arr[34]->GetDID()->GetName();
    plotid = (long int)std_arr[34]->findPlotItem("Plot delay time");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // FDAC tuning check - cut on ToT-dsipersion
  cutid = 10;
  valarr[0] = 10*UNDEFCUT; 
  if(std_arr[16]!=0){
    TH1F *tmphi=0;
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      CutClassItem* lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      for(i=0;i<ncutclasses;i++){
	m_cutpnl->ClassList->setCurrentItem(lvit);
	if(m_cutpnl->GetCutPars(cutid+m_cutpnl->npcuts)->enabled){
	  if(std_arr[16]->GetDID()->m_isscan==0)  // compatibility with old data->check if it's a scan
	    rmain->PlotMasked(std_arr[16], m_badpixit[i], AVTOT, MAMASK, false,-1,0,bit_mask);
	  else
	    rmain->PlotMasked(std_arr[16], m_badpixit[i], AVTOT, MAMASK, false,-1,0,bit_mask);
	  can = (TCanvas*) gROOT->FindObject("pixcan");
	  if(!basename.isEmpty()){
	    if(ncutclasses==1)
	      fname = basename + "_ToT_tunever.ps";
	    else
	      fname = basename + "_ToT_tunever_cc" + QString::number(i+1) + ".ps";
	    can->Print(fname.latin1());
	  }
	  tmphi = (TH1F*)gROOT->FindObject("umskhist");
	  if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
	  if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
	    valarr[i] = tmphi->GetFunction("gaus")->GetParameter(2);
	    if(m_sdTOT<0 && m_avgTOT<0 && i==m_selcutID){
	      m_avgTOT = tmphi->GetFunction("gaus")->GetParameter(1);
	      m_sdTOT = tmphi->GetFunction("gaus")->GetParameter(2);
	    }
	  }else{ // distribution might not be Gaussian, get arithmetic values then
	    valarr[i] = tmphi->GetRMS();
	    if(m_sdTOT<0 && m_avgTOT<0 && i==m_selcutID){
	      m_avgTOT = tmphi->GetMean();
	      m_sdTOT = tmphi->GetRMS();
	    }
	  }
	} else
	  valarr[i] = 0;
	lvit = (CutClassItem*)lvit->nextSibling();
      }
    } else{
      if(std_arr[16]->GetDID()->m_isscan==0)  // compatibility with old data->check if it's a scan
        rmain->PlotMasked(std_arr[16], m_badpixit[ncutclasses], AVTOT, MAMASK, false,-1,0,bit_mask);
      else
        rmain->PlotMasked(std_arr[16], m_badpixit[ncutclasses], AVTOT, MAMASK, false,-1,0,bit_mask);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
        fname = basename + "_ToT_tunver.ps";
        can->Print(fname.latin1());
      }
      tmphi = (TH1F*)gROOT->FindObject("umskhist");
      if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      if((tmphi->GetRMS()/tmphi->GetFunction("gaus")->GetParameter(2)<1.2)){
        valarr[1] = tmphi->GetFunction("gaus")->GetParameter(2);
      }else{
        valarr[1] = tmphi->GetRMS();
      }
    }
    cutname = std_arr[16]->GetDID()->GetName();
    if(std_arr[16]->GetDID()->m_isscan==0)  // compatibility with old data->check if it's a scan
      plotid = (long int)std_arr[16]->findPlotItem("Plot raw avg. ToT");
    else
      plotid = (long int)std_arr[16]->findPlotItem("Plot ToT vs FDAC");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // now the dummy cut for those items that just need a plot to be generated
  cutid = 14;
  // digital SRAM
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[0]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[0]->GetDID()->GetName();
    plotid = (long int)std_arr[0]->findPlotItem("Plot raw data");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  // digital DFIFO
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[37]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[37]->GetDID()->GetName();
    plotid = (long int)std_arr[37]->findPlotItem("Plot raw data");
    if(plotid==0) // if loaded as ToT data
      plotid = (long int)std_arr[37]->findPlotItem("Plot hits from ToT");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[37]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[37]->GetDID()->GetName();
    cutname+= "(ToT)";
    plotid = (long int)std_arr[37]->findPlotItem("Plot raw avg. ToT");
  }
  if(plotid!=0)
    ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  // digital EOC
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[38]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[38]->GetDID()->GetName();
    plotid = (long int)std_arr[38]->findPlotItem("Plot hits vs scan par");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  // source hits
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[14]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[14]->GetDID()->GetName();
    cutname+= "(hits)";
    plotid = (long int)std_arr[14]->findPlotItem("Plot hits from ToT");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
// ToT is done per chip now
//   // source ToT
//   valarr[0] = 10*UNDEFCUT; 
//   plotid = 0;
//   if(std_arr[14]!=0){
//     valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
//     valarr[1] = 0;
//     cutname = std_arr[14]->GetDID()->GetName();
//     cutname+= "(ToT)";
//     plotid = (long int)std_arr[14]->findPlotItem("Plot raw avg. ToT");
//   }
//   ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  // HV=0 threshold scan - bad bumps
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  cutid = 14;
  if(std_arr[1]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[1]->GetDID()->GetName();
    plotid = (long int)std_arr[1]->findPlotItem("Plot unconn. bump map (noise diff.)");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
  // monleak
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[17]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    valarr[1] = 0;
    cutname = std_arr[17]->GetDID()->GetName();
    plotid = (long int)std_arr[17]->findPlotItem("Plot leakage currents");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
    
  // spread between chips in ToT
  plotTest = false;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    if(m_cutpnl->GetCutPars(15+m_cutpnl->npcuts)->enabled) plotTest = true;
    if(m_cutpnl->GetCutPars(16+m_cutpnl->npcuts)->enabled) plotTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  cutid = 15;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[14]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    std::vector<float> result;
    MAEngine::checkToT(std_arr[14]->GetDID(),result);
    valarr[1] = result[6];
    cutname = std_arr[14]->GetDID()->GetName();
    plotid = (long int)std_arr[14]->findPlotItem("Plot raw avg. ToT");
    if(plotTest){
      rmain->PlotMapProjScat(std_arr[14]->findPlotItem("Plot raw avg. ToT"), -1);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
	fname = basename + "_sourceToT.ps";
	can->Print(fname.latin1());
      }
      plotTest = false;
    }
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
    
  // chi2 of avg ToT
  cutid = 16;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[14]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    std::vector<float> result;
    MAEngine::checkToT(std_arr[14]->GetDID(),result);
    valarr[1] = result[7];
    cutname = std_arr[14]->GetDID()->GetName();
    plotid = (long int)std_arr[14]->findPlotItem("Plot raw avg. ToT");
    if(plotTest){
      rmain->PlotMapProjScat(std_arr[14]->findPlotItem("Plot raw avg. ToT"), -1);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
	fname = basename + "_sourceToT.ps";
	can->Print(fname.latin1());
      }
    }
  }
  ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
    
  cutid = 17;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[0]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    m_VDD = std_arr[0]->GetDID()->GetLog()->MyDCS->LVvolt[m_parent->options->m_Opts->m_DDsupp][m_parent->options->m_Opts->m_DDchan];
    valarr[1] = m_VDD;
    cutname = std_arr[0]->GetDID()->GetName();
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed);
    
  cutid = 18;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[0]!=0){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    m_VDDA = std_arr[0]->GetDID()->GetLog()->MyDCS->LVvolt[m_parent->options->m_Opts->m_DDAsupp][m_parent->options->m_Opts->m_DDAchan];
    valarr[1] = m_VDDA;
    cutname = std_arr[0]->GetDID()->GetName();
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed);

  // VDD range test
  // determine min and max and load min
  bool runTest = false;
  cutid = START_RANGE-NCUTS+0;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    cutid = START_RANGE-NCUTS+0;
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    cutid = START_RANGE-NCUTS+1;
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  std::vector<float> vmax, vmin;
  for(vddid = 23;vddid<28;vddid++){
    vmax.clear();
    vmax.resize(ncutclasses);  
    cutid = START_RANGE-NCUTS+0;
    valarr[0] = 10*UNDEFCUT; 
    plotid = 0;
    if(std_arr[vddid]!=0 && runTest){
      TH1F *tmphi;
      if(m_cutpnl->IgnoreBadPix->isChecked()){
        for(i=0;i<ncutclasses;i++){
          // mask is commented since plotting routine will find
          // bad pixels on its own
          rmain->DigExcMiss(std_arr[vddid], -1, 0, -999, m_badpixit[i]->GetDID()->GetMap(-1,MAMASK),true);
          can = (TCanvas*) gROOT->FindObject("pixcan");
          if(!basename.isEmpty()){
            fname = basename + "_vddrange_" + QString::number(vddid-23) + ".ps";
            can->Print(fname.latin1());
          }
          tmphi = (TH1F*)gROOT->FindObject("digem-1");
          bool minfound=false;
          vmax[i] = 0;
          for(j=0;j<tmphi->GetNbinsX();j++){
            if(tmphi->GetBinContent(j+1)==0 && !minfound){
              valarr[i] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
              minfound = true;
            }else if(tmphi->GetBinContent(j+1)!=0 && minfound){
              vmax[i] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
              minfound = false;
            }
          }
          tmphi = (TH1F*)gROOT->FindObject("digem-2");
          minfound=false;
          for(j=0;j<tmphi->GetNbinsX();j++){
            if(tmphi->GetBinContent(j+1)==0 && !minfound){
              if((tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2)>valarr[i])
                valarr[i] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
              minfound = true;
            }else if(tmphi->GetBinContent(j+1)!=0 && minfound){
              if((tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2)<vmax[i])
                vmax[i] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
              minfound = false;
            }
          }
          // might not have found a problem in the upper range, so use last tested VDD
          j=tmphi->GetNbinsX();
          if(vmax[i]==0)
            vmax[i] = tmphi->GetBinLowEdge(j) + tmphi->GetBinWidth(j)/2;
          // measurement is in mV, cut in V
          valarr[i] *= .001f;
          vmax[i]   *= .001f;
        }
        if(m_minVDD==-1 || 1000*valarr[m_selcutID]>m_minVDD)
          m_minVDD = 1000*valarr[m_selcutID];
        if(m_maxVDD==-1 || 1000*valarr[m_selcutID]<m_maxVDD)
          m_maxVDD = 1000*vmax[m_selcutID];
      }else{
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        rmain->DigExcMiss(std_arr[vddid], -1, 0);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          fname = basename + "_vddrange_" + QString::number(vddid-23) + ".ps";
          can->Print(fname.latin1());
        }
        tmphi = (TH1F*)gROOT->FindObject("digem-1");
        bool minfound=false;
        for(j=0;j<tmphi->GetNbinsX();j++){
          if(tmphi->GetBinContent(j+1)==0 && !minfound){
            valarr[1] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
            minfound = true;
          }else if(tmphi->GetBinContent(j+1)!=0 && minfound){
            vmax[1] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
            minfound = false;
          }
        }
        tmphi = (TH1F*)gROOT->FindObject("digem-2");
        minfound=false;
        for(j=0;j<tmphi->GetNbinsX();j++){
          if(tmphi->GetBinContent(j+1)==0 && !minfound){
            if((tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2)>valarr[1])
              valarr[1] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
            minfound = true;
          }else if(tmphi->GetBinContent(j+1)!=0 && minfound){
            if((tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2)<vmax[1])
              vmax[0] = tmphi->GetBinLowEdge(j+1) + tmphi->GetBinWidth(j+1)/2;
            minfound = false;
          }
        }
        // might not have found a problem in the upper range, so use last tested VDD
        j=tmphi->GetNbinsX();
        if(vmax[0]==0)
          vmax[0] = tmphi->GetBinLowEdge(j) + tmphi->GetBinWidth(j)/2;
        // measurement is in mV, cut in V
        valarr[1] *= .001f;
        vmax[0]   *= .001f;
        if(m_minVDD==-1 || 1000*valarr[1]>m_minVDD)
          m_minVDD = 1000*valarr[1];
        if(m_maxVDD==-1 || 1000*valarr[0]<m_maxVDD)
          m_maxVDD = 1000*vmax[0];
      }
      cutname = std_arr[vddid]->GetDID()->GetName();
      plotid = (long int)std_arr[vddid]->findPlotItem(DIGEMP);
    }
    ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
    // load VDD max
    cutid = START_RANGE-NCUTS+1;
    valarr[0] = 10*UNDEFCUT; 
    if(std_arr[vddid]!=NULL){
      if(m_cutpnl->IgnoreBadPix->isChecked()){
        for(i=0;i<ncutclasses;i++)
          valarr[i] = vmax[i];  // measurement is in mV, cut in V
      }else{
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = vmax[0];  // measurement is in mV, cut in V
      }
      cutname = std_arr[vddid]->GetDID()->GetName();
    }
    ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
  }

  // Xck range test
  cutid = START_RANGE-NCUTS+2;
  runTest = false;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[28]!=0 && std_arr[36]!=0 && std_arr[0]!=0 && runTest){
    valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
    //int nbad = RMain::nBadPixs(std_arr[28]);
    int freq=0;
    valarr[1] = RMain::nBadPixs(std_arr[0]);
    plotid = (long int)std_arr[0]->findPlotItem("Plot raw data");
    cutname = std_arr[0]->GetDID()->GetName();
    if(ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid))
      //        if(nbad<nmaxbad && nbad>=0)
      freq = 40;
    valarr[1] = RMain::nBadPixs(std_arr[36]);
    plotid = (long int)std_arr[36]->findPlotItem("Plot raw data");
    cutname = std_arr[36]->GetDID()->GetName();
    if(ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid))
      //if(nbad<nmaxbad && nbad>=0){
      freq = 45;
    valarr[1] = RMain::nBadPixs(std_arr[28]);
    plotid = (long int)std_arr[28]->findPlotItem("Plot raw data");
    cutname = std_arr[28]->GetDID()->GetName();
    if(ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid))
      //if(nbad<nmaxbad && nbad>=0){
      freq = 50;
    // fill frequency value and cut on it
    valarr[1] = (float)freq;
    plotid = (long int)std_arr[0]->findPlotItem("Plot raw data");
    m_maxXCK = valarr[1];
    cutname = std_arr[28]->GetDID()->GetName();
  }
  cutid = START_RANGE-NCUTS+3;
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // VDDA range test
  runTest = false;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  cutid = START_RANGE-NCUTS+4;
  m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    cutid = START_RANGE-NCUTS+4;
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    cutid = START_RANGE-NCUTS+5;
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  // load min first
  cutid = START_RANGE-NCUTS+4;
  valarr[0] = 10*UNDEFCUT; 
  ModItem *vdda_test[5];
  float vdda_nom[5]={1.7f,1.5f,1.5f,1.8f,1.8f};
  bool haveone=false;
  float vdda;
  vdda_test[0] = std_arr[6]; // use antikill threshold scan (=1.6V), too
  plotid = 0;
  if(vdda_test[0] == 0)
    vdda_test[0] = std_arr[5]; // use "normal" scan if the other scan isn't around
  for(int set=29;set<33;set++){ // other VDDA values
    vdda_test[set-28] = std_arr[set];
    if(std_arr[set]!=0) haveone=true;
  }
  if(haveone && runTest){
    vmax.clear();
    vmax.resize(ncutclasses);  
    vmin.clear();
    vmin.resize(ncutclasses);  
    for(int i=0;i<ncutclasses;i++){
      vmax[i] = -1;
      vmin[i] = 10000;
    }
    for(int set=0;set<5;set++){ // apply cut to all five threshold scans
      if(vdda_test[set]!=NULL){ 
        int ch,sp;
        ch = m_parent->options->m_Opts->m_DDAchan;
        sp = m_parent->options->m_Opts->m_DDAsupp;
        if(vdda_test[set]->GetDID()->GetLog()!=0)
          vdda = vdda_test[set]->GetDID()->GetLog()->MyDCS->LVvolt[sp][ch];
        else
          vdda = vdda_nom[set];
        TH2F *tmphi = vdda_test[set]->GetDID()->GetMap(-1,CHI), *mask;
        for(int i=0;i<ncutclasses;i++)
          tmppassed[i] = true;
        if(m_cutpnl->IgnoreBadPix->isChecked()){
          for(int i=0;i<ncutclasses;i++){
            mask  = m_badpixit[i]->GetDID()->GetMap(-1,MAMASK);
            valarr[i] = CountChi(tmphi,mask);
            cutname = vdda_test[set]->GetDID()->GetName();
            plotid = (long int)vdda_test[set]->findPlotItem("Plot threshold");
            ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid);
            if(tmppassed[i]){
              //            if(CountChi(tmphi,mask)<10){ // 10%-cut
              if(vdda>vmax[i]) vmax[i] = vdda;
              if(vdda<vmin[i] && vdda>0) vmin[i] = vdda;
            }
          }
          m_minVDDA = vmin[m_selcutID];
          m_maxVDDA = vmax[m_selcutID];
        }else{
          valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
          mask  = m_badpixit[ncutclasses]->GetDID()->GetMap(-1,MAMASK);
          valarr[1] = CountChi(tmphi,mask);
          cutname = vdda_test[set]->GetDID()->GetName();
          plotid = (long int)vdda_test[set]->findPlotItem("Plot threshold");
          if(ModuleClass(cutid, cutname.latin1(), valarr, tmppassed, plotid)){
            //          if(CountChi(tmphi,mask)<10){ // 10%-cut
            if(vdda>vmax[0]) vmax[0] = vdda;
            if(vdda<vmin[0] && vdda>0) vmin[0] = vdda;
          } else
          m_minVDDA = vmin[0];
          m_maxVDDA = vmax[0];
        }
        cutname = vdda_test[set]->GetDID()->GetName();
      }
    }
    if(m_minVDDA>3) 
      m_minVDDA = -1;
    if(m_maxVDDA<0)
      m_maxVDDA = -1;
  }
  else{
    m_minVDDA = -1;
    m_maxVDDA = -1;
  }
  // process VDDA min
  cutid = START_RANGE-NCUTS+5;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(haveone && runTest){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(int i=0;i<ncutclasses;i++)
        valarr[i] = vmin[i];
    } else{
      valarr[0] = UNDEFCUT;
      valarr[1] = vmin[0];
    }
  }
  if(std_arr[31]!=0)
    plotid = (long int)std_arr[31]->findPlotItem("Plot threshold");
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // then process VDDA max
  cutid = START_RANGE-NCUTS+6;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(haveone && runTest){
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(int i=0;i<ncutclasses;i++)
        valarr[i] = vmax[i];
    } else{
      valarr[0] = UNDEFCUT;
      valarr[1] = vmax[0];
    }
  }
  if(std_arr[32]!=0)
    plotid = (long int)std_arr[32]->findPlotItem("Plot threshold");
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // GDAC scan - all scan points OK?
  cutid = START_RANGE-NCUTS+7;
  runTest = false;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  int minpt = 0, maxpt = 0, tmpint;
  std::vector<int> fail;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[33]!=0 && std_arr[33]->GetDID()->GetLog()!=0 && runTest){ 
    tmpint = 0;
    m_GDACfail = 0;
    for(int i=0;i<ncutclasses;i++)
      valarr[i] = 0;
    cutname = std_arr[33]->GetDID()->GetName();
    TH2F *tmphi, *mask;
    int j,lastpt = std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Step;
    for(j=0;j<lastpt;j++){
      if(std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[j]>
         std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[maxpt])
        maxpt = j;
      if(std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[j]<
         std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[minpt])
        minpt = j;
      tmphi = std_arr[33]->GetDID()->Get2DMap(-1,TWODCHI,j);
      if(m_cutpnl->IgnoreBadPix->isChecked()){
        for(int i=0;i<ncutclasses;i++){
          mask  = m_badpixit[i]->GetDID()->GetMap(-1,MAMASK);
          if(CountChi(tmphi,mask)<10){
            tmpint |= std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[j];
            valarr[i] = (float)tmpint;
            if(i==m_selcutID) m_GDACfail |= std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[j];
          }
        }
      }else{
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        mask  = m_badpixit[ncutclasses]->GetDID()->GetMap(-1,MAMASK);
        if(CountChi(tmphi,mask)<10){
          tmpint |= std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[j];
          valarr[1] = (float)tmpint;
          m_GDACfail |= std_arr[33]->GetDID()->GetLog()->MyScan->Outer->Pts[j];
        }
      }
    }
    //m_GDACfail = 31&(~m_GDACfail);
    ModItem *tmp_it = std_arr[33]->findPlotItem("Inner scan fit results vs outer par");
    if(tmp_it!=0)
      plotid = (long int)tmp_it->findPlotItem("Plot threshold");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // GDAC scan - min GDAC
  cutid = START_RANGE-NCUTS+8;
  runTest = false;
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  m_cutpnl->ClassList->setCurrentItem(lvit);
  for(i=0;i<ncutclasses;i++){
    if(m_cutpnl->GetCutPars(cutid+NCUTS)->enabled) runTest = true;
    lvit = (CutClassItem*)lvit->nextSibling();
    m_cutpnl->ClassList->setCurrentItem(lvit);
  }
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[33]!=0 && runTest){ 
    TH1F *tmphi;
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++){
        rmain->PlotMasked(std_arr[33], m_badpixit[i], TWODMEAN, MAMASK, false, -1, minpt,bit_mask);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          if(ncutclasses==1)
            fname = basename + "_gdacmin.ps";
          else
            fname = basename + "_gdacmin_cc" + QString::number(i+1) + ".ps";
          can->Print(fname.latin1());
        }
        tmphi = (TH1F*)gROOT->FindObject("umskhist");
        if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
        valarr[i] = tmphi->GetFunction("gaus")->GetParameter(1);
        if(i==m_selcutID) m_minTHR = valarr[i];
      }
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      TH1F *tmphi;
      //rmain->PlotMapProjScat(std_arr[33], -1, 0);
      rmain->PlotMasked(std_arr[33], m_badpixit[ncutclasses], TWODMEAN, MAMASK, false, -1, minpt,bit_mask);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
        fname = basename + "_gdacmin.ps";
        can->Print(fname.latin1());
      }
      tmphi = (TH1F*)gROOT->FindObject("umskhist");
      if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
      valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
      m_minTHR = valarr[1];
    }
    cutname = std_arr[33]->GetDID()->GetName();
    ModItem *tmp_it = std_arr[33]->findPlotItem("Inner scan fit results vs outer par");
    if(tmp_it!=0)
      plotid = (long int)tmp_it->findPlotItem("Plot threshold");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  // GDAC scan - max GDAC
  cutid = START_RANGE-NCUTS+8;
  valarr[0] = 10*UNDEFCUT; 
  plotid = 0;
  if(std_arr[33]!=0 && std_arr[33]->GetDID()->GetLog()!=0 && runTest){ 
    TH1F *tmphi;
    if(m_cutpnl->IgnoreBadPix->isChecked()){
      for(i=0;i<ncutclasses;i++){
        rmain->PlotMasked(std_arr[33], m_badpixit[i], TWODMEAN, MAMASK, false, -1, maxpt,bit_mask);
        can = (TCanvas*) gROOT->FindObject("pixcan");
        if(!basename.isEmpty()){
          if(ncutclasses==1)
            fname = basename + "_gdacmax.ps";
          else
            fname = basename + "_gdacmax_cc" + QString::number(i+1) + ".ps";
          can->Print(fname.latin1());
        }
        tmphi = (TH1F*)gROOT->FindObject("umskhist");
        if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
        valarr[i] = tmphi->GetFunction("gaus")->GetParameter(1);
        if(i==m_selcutID) m_maxTHR = valarr[i];
      }
    } else{
      valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
      TH1F *tmphi;
      rmain->PlotMasked(std_arr[33], m_badpixit[ncutclasses], TWODMEAN, MAMASK, false, -1, maxpt,bit_mask);
      can = (TCanvas*) gROOT->FindObject("pixcan");
      if(!basename.isEmpty()){
        fname = basename + "_gdacmax.ps";
        can->Print(fname.latin1());
      }
      tmphi = (TH1F*)gROOT->FindObject("umskhist");
      if(tmphi->GetFunction("gaus")==NULL) tmphi->Fit("gaus","Q");
      valarr[1] = tmphi->GetFunction("gaus")->GetParameter(1);
      m_maxTHR = valarr[1];
    }
    cutname = std_arr[33]->GetDID()->GetName();
    ModItem *tmp_it = std_arr[33]->findPlotItem("Inner scan fit results vs outer par");
    if(tmp_it!=0)
      plotid = (long int)tmp_it->findPlotItem("Plot threshold");
  }
  ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);

  if(m_parent->options->m_Opts->m_showChipCuts){
    // chip cuts (specal version only)
    ReportText->setText(ReportText->text() + "<br><i>chip cuts</i><br><br>");
    ReportText->repaint();
    if(m_app!=0) m_app->processEvents();
    // IDD after register test
    cutid = NMCUTS+0;
    for(mychip=0;mychip<NCHIP;mychip++){
      valarr[0] = 10*UNDEFCUT; 
      plotid = 0;
      if(std_arr[20]!=NULL){
        TGraph *gr = std_arr[20]->GetDID()->GetGraph(0);
        double id, curr;
        gr->GetPoint(mychip,id,curr);
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = (float) curr;
        cutname  = std_arr[20]->GetDID()->GetName();
        cutname += "/chip "+QString::number(mychip);
        plotid = (long int)std_arr[20]->findPlotItem("Plot FE currents");
      }
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);  
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
        m_cutpnl->ClassList->setCurrentItem(lvit);
        if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
        lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
    // IDDA after register test
    cutid = NMCUTS+1;
    for(mychip=0;mychip<NCHIP;mychip++){
      valarr[0] = 10*UNDEFCUT; 
      plotid = 0;
      if(std_arr[20]!=NULL){
        TGraph *gr = std_arr[20]->GetDID()->GetGraph(1);
        double id, curr;
        gr->GetPoint(mychip,id,curr);
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = (float) curr;
        cutname  = std_arr[20]->GetDID()->GetName();
        cutname += "/chip "+QString::number(mychip);
        plotid = (long int)std_arr[20]->findPlotItem("Plot FE currents");
      }
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);  
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
        m_cutpnl->ClassList->setCurrentItem(lvit);
        if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
        lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
    // IDD after digital scan
    cutid = NMCUTS+2;
    for(mychip=0;mychip<NCHIP;mychip++){
      valarr[0] = 10*UNDEFCUT; 
      plotid = 0;
      if(std_arr[20]!=NULL){
        TGraph *gr = std_arr[20]->GetDID()->GetGraph(2);
        double id, curr;
        gr->GetPoint(mychip,id,curr);
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = (float) curr;
        cutname  = std_arr[20]->GetDID()->GetName();
        cutname += "/chip "+QString::number(mychip);
        plotid = (long int)std_arr[20]->findPlotItem("Plot FE currents");
      }
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);  
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
        m_cutpnl->ClassList->setCurrentItem(lvit);
        if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
        lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
    // IDDA after digital scan
    cutid = NMCUTS+3;
    for(mychip=0;mychip<NCHIP;mychip++){
      valarr[0] = 10*UNDEFCUT; 
      plotid = 0;
      if(std_arr[20]!=NULL){
        TGraph *gr = std_arr[20]->GetDID()->GetGraph(3);
        double id, curr;
        gr->GetPoint(mychip,id,curr);
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = (float) curr;
        cutname  = std_arr[20]->GetDID()->GetName();
        cutname += "/chip "+QString::number(mychip);
        plotid = (long int)std_arr[20]->findPlotItem("Plot FE currents");
      }
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);  
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
        m_cutpnl->ClassList->setCurrentItem(lvit);
        if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
        lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
    // # bad pixels
    cutid = NMCUTS+4;
    vplotid.clear();
    for(mychip=0;mychip<NCHIP;mychip++){
      vplotid.push_back(0);
      for(i=0;i<ncutclasses;i++){
        valarr[i]=0;
        TH2F *map = m_badpixit[i]->GetDID()->GetMap(mychip,MAMASK);
        vplotid[mychip] = (long int)m_badpixit[i]->findPlotItem("Plot of masked channels");
        for(int bx=0;bx<map->GetNbinsX();bx++){
          for(int by=0;by<map->GetNbinsY();by++){
            if(map->GetBinContent(bx+1,by+1)!=0)
              valarr[i]++;
          }
        }
      }
      cutname = "Number of bad pixels/chip "+QString::number(mychip);
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, vplotid);  
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
        m_cutpnl->ClassList->setCurrentItem(lvit);
        if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
        lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
    // CP mask from register test
    cutid = NMCUTS+5;
    for(mychip=0;mychip<NCHIP;mychip++){
      valarr[0] = 10*UNDEFCUT; 
      plotid = 0;
      if(std_arr[0]!=0 && std_arr[0]->GetDID()->GetLog()!=0){
        valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
        valarr[1] = std_arr[0]->GetDID()->GetLog()->MyModule->Chips[mychip]->ColMask;
        ModItem *tmp_it = std_arr[0]->findPlotItem("Log File");
        if(tmp_it!=0)
          plotid = (long int)tmp_it->findPlotItem("Display");
      }
      cutname = "CP mask (regtest) chip "+QString::number(mychip);
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);  
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
        m_cutpnl->ClassList->setCurrentItem(lvit);
        if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
        lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
    // source ToT - per chip
    cutid = NMCUTS+6;
    for(mychip=0;mychip<NCHIP;mychip++){
      valarr[0] = 10*UNDEFCUT; 
      plotid = 0;
      if(std_arr[14]!=0){
	valarr[0] = UNDEFCUT;// tells subroutine we have one value for all cut classes
	TH2F *map = std_arr[14]->GetDID()->GetMap(mychip,AVTOT);
	valarr[1] = 0;
	float nent=0;
	for(int bx=0;bx<map->GetNbinsX();bx++){
	  for(int by=0;by<map->GetNbinsY();by++){
	    if(map->GetBinContent(bx+1,by+1)!=0){
	      nent++;
	      valarr[1] += map->GetBinContent(bx+1,by+1);
	    }
	  }
	}
	if(nent>0) valarr[1]/=nent;
	cutname = std_arr[14]->GetDID()->GetName();
	cutname+= "(ToT chip "+QString::number(mychip)+")";
	plotid = (long int)std_arr[14]->findPlotItem("Plot raw avg. ToT");
      }
      ModuleClass(cutid, cutname.latin1(), valarr, allpassed, plotid);
      bool alldisen=true;
      CutClassItem *lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
      while(lvit!=0){
	m_cutpnl->ClassList->setCurrentItem(lvit);
	if(m_cutpnl->GetCutPars(m_cutpnl->npcuts+cutid)->enabled) alldisen = false;
	lvit = (CutClassItem*)lvit->nextSibling();
      }
      if(alldisen) break; // no need to print 16 times if cut is disabled anyway
    }
  } // end   if(m_parent->options->m_Opts->m_showChipCuts)

  // grand summary
  //rmain->m_psfile = NULL;
  //psfile->Close();
  ReportText->setText(ReportText->text() + "<br><br><b>SUMMARY</b><br>");
  ReportText->repaint();
  if(m_app!=0) m_app->processEvents();
  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  m_all_OK = allpassed[m_selcutID];
  for(i=0;i<ncutclasses;i++){
    title = lvit->text(0) + ":";
    if(allpassed[i])
      title += " <b><font color=\"#00FF00\">PASS</font></b><br><a name=\"summary\"></a>";
    else
      title += " <b><font color=\"#FF0000\">FAIL</font></b><br><a name=\"summary\"></a>";
    ReportText->setText(ReportText->text() + title);
    ReportText->repaint();
    if(m_app!=0) m_app->processEvents();
    lvit = (CutClassItem*)lvit->nextSibling();
  }

  // scroll to end of report
  ReportText->scrollToAnchor("summary");
  ReportText->repaint();
  repaint();
  if(m_app!=0) m_app->processEvents();

  // write summary into text file
  if(!basename.isEmpty()){
    fname = basename + ".html";
    FILE *out = fopen(fname.latin1(),"w");
    if(out!=0){
      QString txtcont = ReportText->text();
      // remove links to plots - won't work in file
      txtcont.replace(QRegExp("</a>"),"");
      int pos=0;
      while(pos>=0){
        pos = txtcont.find("<a href");
        if(pos>=0){
          int other_end = txtcont.find("\">",pos+1);
          if(other_end>=0)
            txtcont.remove(pos,other_end+2-pos);
          else
            pos = -1;
        }
      }
//       txtcont.replace(QRegExp("<br>"),"\n");
//       txtcont.replace(QRegExp("<b>"),"");
//       txtcont.replace(QRegExp("</b>"),"");
//       txtcont.replace(QRegExp("<font color=\"#FF0000\">"),"");
//       txtcont.replace(QRegExp("<font color=\"#00FF00\">"),"");
//       txtcont.replace(QRegExp("</font>"),"");
      fprintf(out,txtcont.latin1());
      fclose(out);
    }
  }

  // delete still existing maps, data items etc
  if(xtalk!=NULL) xtalk->Delete();
  /*
  for(i=0;i<=ncutclasses;i++){
    delete (m_badpixit[i]->GetDID());
    delete m_badpixit[i];
  }
  */

  lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
  while(lvit!=0){
    if(lvit->itemID()==m_selcutID){
      m_cutpnl->ClassList->setCurrentItem(lvit);
      break;
    }
    lvit = (CutClassItem*)lvit->nextSibling();
  }

  // fill quali table
  m_selcutID = ((CutClassItem*)m_cutpnl->ClassList->currentItem())->itemID();
  cutLabel->setText("electrical penalties based on \"" + 
                  m_cutpnl->ClassList->currentItem()->text(0)+"\"");
  fillTable();

  delete (m_badpixit[ncutclasses]->GetDID());
  delete m_badpixit[ncutclasses];
  //m_badpixit.resize(ncutclasses);

  // back to normal cursor
  QApplication::restoreOverrideCursor();

  CloseButt->setText("Close");
  return;
}

void QAReport::BadPixelTest(int cutid, const char *cutname, TH2F *hitocheck, float convert, float offset, TH2F *ignore,
                            bool zerosupp){ 
  QString cuttxt, fillchar;
  int i,j,ccl;
  float cutmin, cutmax;
  CutClassItem *lvit;
  int ncutclasses = m_cutpnl->ClassList->childCount();
  bool doignore;
  bool printed_header = false;
  
  cuttxt = ReportText->text();
  cuttxt+= m_cutpnl->GetCutPars(cutid)->label.c_str();
  if(hitocheck!=0){
    cuttxt += " - data set: ";
    cuttxt += cutname;
    cuttxt += "<br>";
    lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
    for(ccl=0;ccl<ncutclasses;ccl++){
      int nbad, nbadold = (int)m_badpix_hi[ccl]->GetEntries();
      nbad = nbadold;
      m_cutpnl->ClassList->setCurrentItem(lvit);
      cutmin = convert * (float)m_cutpnl->GetCutPars(cutid)->min + offset;
      cutmax = convert * (float)m_cutpnl->GetCutPars(cutid)->max + offset;
      if(m_cutpnl->GetCutPars(cutid)->enabled){
        if(!printed_header){
          cuttxt += "<pre>Cut class          min     max         tot.bad new bad\n";
          printed_header = true;
        }
        for(i=1;i<=hitocheck->GetNbinsX();i++){
          for(j=1;j<=hitocheck->GetNbinsY();j++){
            doignore = false;
            if(ignore!=NULL)
              if(ignore->GetBinContent(i,j)==1) doignore=true;
            if(((cutmin>hitocheck->GetBinContent(i,j) && m_cutpnl->GetCutPars(cutid)->min!=UNDEFCUT)
                || (cutmax<hitocheck->GetBinContent(i,j)&& m_cutpnl->GetCutPars(cutid)->max!=UNDEFCUT))
               && !doignore && (!zerosupp || hitocheck->GetBinContent(i,j)!=0)){
              if(m_badpix_hi[ccl]->GetBinContent(i,j)==0){
                nbad++;
                m_badpix_hi[ccl]->SetBinContent(i,j,pow((float)2,(float)cutid));
              } else{
                m_badpix_hi[ccl]->SetBinContent(i,j,
						(float)((int)m_badpix_hi[ccl]->GetBinContent(i,j)|(int)pow((float)2,(float)cutid)));
              }
            }
          }
        }
        nbadold = nbad - nbadold;
        m_badpix_hi[ccl]->SetEntries(nbad);
        cuttxt += lvit->text(0) + fillchar.fill(' ',18-lvit->text(0).length());
        if(m_cutpnl->GetCutPars(cutid)->min!=UNDEFCUT)
          cuttxt += QString("%1").arg(m_cutpnl->GetCutPars(cutid)->min,7,'f',2)+" ";
        else
          cuttxt += "    ... ";
        if(m_cutpnl->GetCutPars(cutid)->max!=UNDEFCUT)
          cuttxt += QString("%1").arg(m_cutpnl->GetCutPars(cutid)->max,7,'f',2)+" ";
        else
          cuttxt += "    ... ";
        cuttxt += m_cutpnl->GetCutPars(cutid)->unit.c_str() + fillchar.fill(' ',5-m_cutpnl->GetCutPars(cutid)->unit.length());
        cuttxt += QString("%1").arg(nbad,6);
        cuttxt += QString("%1").arg(nbadold,6);
        cuttxt += "\n";
      }
      lvit = (CutClassItem*)lvit->nextSibling();
    }
    if(!printed_header)
      cuttxt += "<pre><i>cut disabled</i>";
    cuttxt += "</pre><a name=\"bp"+QString::number(cutid)+"\"></a>";
  }else
    cuttxt += "<br><a name=\"bp"+QString::number(cutid)+"\"></a>";
  ReportText->setText(cuttxt);
  ReportText->scrollToAnchor(("bp"+QString::number(cutid)).latin1());
  ReportText->repaint();
  repaint();
  if(m_app!=0) m_app->processEvents();
  return;
}


bool QAReport::ModuleClass(int cutID, const char *cutname, std::vector<float> &valarr, std::vector<bool> &passed,
                           long int plotid){
  std::vector<long int>vpid;
  int i;
  for(i=0;i<(int)passed.size();i++)
    vpid.push_back(plotid);
  return ModuleClass(cutID, cutname, valarr, passed, vpid);
}

bool QAReport::ModuleClass(int cutID, const char *cutname, std::vector<float> &valarr, std::vector<bool> &passed,
                           std::vector<long int> plotid){
  QString cuttxt, fillchar;
  int ccl, cutid = cutID+m_cutpnl->npcuts;
  CutClassItem *lvit;
  int ncutclasses = m_cutpnl->ClassList->childCount();
  bool this_passed, sel_passed=true;
  float value;
  bool printed_header=false;

  cuttxt = ReportText->text();
  cuttxt+= m_cutpnl->GetCutPars(cutid)->label.c_str();
  if(valarr[0]!=10*UNDEFCUT){
    cuttxt += " - data set: ";
    cuttxt += cutname;
    cuttxt += "<br>";
    lvit = (CutClassItem*)m_cutpnl->ClassList->firstChild();
    for(ccl=0;ccl<ncutclasses;ccl++){
      m_cutpnl->ClassList->setCurrentItem(lvit);
      this_passed = true;
      if(valarr[0]==UNDEFCUT)
        value = valarr[1];
      else
        value = valarr[ccl];
      if(m_cutpnl->GetCutPars(cutid)->enabled){
        if(!printed_header){
          cuttxt += "<pre>Cut class          min    max         measured result<br>";
          printed_header = true;
        }
        if((value<m_cutpnl->GetCutPars(cutid)->min && m_cutpnl->GetCutPars(cutid)->min!=UNDEFCUT) ||
           (value>m_cutpnl->GetCutPars(cutid)->max && m_cutpnl->GetCutPars(cutid)->max!=UNDEFCUT)){
          passed[ccl] = false;
          this_passed = false;
        }
        cuttxt += lvit->text(0) + fillchar.fill(' ',18-lvit->text(0).length());
        if(m_cutpnl->GetCutPars(cutid)->min!=UNDEFCUT)
          cuttxt += QString("%1").arg(m_cutpnl->GetCutPars(cutid)->min,7,'f',2) + " ";
        else
          cuttxt += "    ... ";
        if(m_cutpnl->GetCutPars(cutid)->max!=UNDEFCUT)
          cuttxt += QString("%1").arg(m_cutpnl->GetCutPars(cutid)->max,7,'f',2) + " ";
        else
          cuttxt += "    ... ";
        cuttxt += m_cutpnl->GetCutPars(cutid)->unit.c_str() + fillchar.fill(' ',5-m_cutpnl->GetCutPars(cutid)->unit.length());
        cuttxt += QString("%1").arg(value,7,'f',2);
        if(this_passed){
          cuttxt += " <b><font color=\"#00FF00\">";
          if(plotid[lvit->itemID()]>0)
            cuttxt += "<a href=\"" + QString::number(plotid[lvit->itemID()])+ "\">";
          cuttxt += "PASS";
          if(plotid[lvit->itemID()]>0)
            cuttxt += "</a>";
          cuttxt += "</font></b><br>";
        } else{
          cuttxt += " <b><font color=\"#FF0000\">";
          if(plotid[lvit->itemID()]>0)
            cuttxt += "<a href=\"" + QString::number(plotid[lvit->itemID()])+ "\">";
          cuttxt += "FAIL";
          if(plotid[lvit->itemID()]>0)
            cuttxt += "</a>";
          cuttxt += "</font></b><br>";
        }
      }
      if(m_selcutID==lvit->itemID()) sel_passed = this_passed;
      lvit = (CutClassItem*)lvit->nextSibling();
    }
    if(!printed_header)
      cuttxt += "<pre><i>cut disabled</i>";
    cuttxt += "</pre><a name=\"mc"+QString::number(cutid)+"\"></a>";
  }else
    cuttxt += "<br><a name=\"mc"+QString::number(cutid)+"\"></a>";
  ReportText->setText(cuttxt);
  ReportText->scrollToAnchor(("mc"+QString::number(cutid)).latin1());
  ReportText->repaint();
  repaint();
  if(m_app!=0) m_app->processEvents();

  return sel_passed;
}

void QAReport::LoadPath(const char *havepath){
  QString path = QString::null;
  if(havepath==NULL){
    if(!FileName->text().isEmpty())
      path = FileName->text();
    path = Q3FileDialog::getExistingDirectory(path,this,"MAF QAReport",
                                      "select output dir",FALSE);
  }else
    path = havepath;

  if(!path.isEmpty()){
    FILE *out = fopen((path+"/test.txt").latin1(),"w");
    if(out!=NULL){
      FileName->setText(path);
      StartButton->setEnabled(true);
      fclose(out);
      remove((path+"/test.txt").latin1());
      return;
    }else if(havepath!=NULL){
      bool do_cre_dir = true;
      if(m_parent!=0 && m_parent->options->m_Opts->m_showwarn){
        QMessageBox credir("Create directory?", 
                           "Output directory " + path + " doesn't exist.\nDo you want to create the directory?",
                           QMessageBox::Information,QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton,
                           this, "overwrite_file",TRUE,Qt::WType_TopLevel);
        
        if(credir.exec()!=QMessageBox::Yes)
          do_cre_dir=false;
      }
      if(do_cre_dir){
        // check entire path and create directories if necessary
        int cp_err = MAEngine::checkPath(path.latin1());
        if(cp_err<0)
          QMessageBox::warning(this,"QARep","Part of folder " + path + " does not exist and can't be created.");
        FileName->setText(path);
        StartButton->setEnabled(true);
        return;
      } 
    }
    FileName->setText("");
    StartButton->setEnabled(false);
    QMessageBox::warning(this,"QARep","Can't create/open files in dir. "+path);
  }
  return;
}
float QAReport::CountChi(TH2F * chi_histo, TH2F* mask){
  int i,j, ntot=0, nfail=0;
  float retval;
  for(i=0;i<chi_histo->GetNbinsX();i++){
    for(j=0;j<chi_histo->GetNbinsY();j++){
      if(mask->GetBinContent(i+1,j+1)!=1){
        ntot++;
        if(chi_histo->GetBinContent(i+1,j+1)<=0)
          nfail++;
      }
    }
  }
  if(ntot>0)
    retval = 100*(float)nfail/(float)ntot;
  else
    retval = 0;
  return retval;
}
void QAReport::copyToCB(){
  int i,j, ncsel;
  QString cbText="";
  for(i=0;i<QualiTable->numRows();i++){
    ncsel=0;
    for(j=0;j<QualiTable->numCols();j++){
      if(QualiTable->isSelected(i,j)){
        cbText+=QualiTable->text(i,j)+"\t";
        ncsel++;
      }
    }
    if(ncsel>0){
      cbText.remove(cbText.length()-1,1);
      cbText+="\n";
    }
  }
  QClipboard *cb = QApplication::clipboard();
  cb->setText(cbText);
}
void QAReport::fillTable(){
  int i,j,con;
  int badDig=0, badAna=0, badSrc=0;
  TH2F *map = 0;
  if(m_selcutID<m_cutpnl->ClassList->childCount())
    map = m_badpixit[m_selcutID]->GetDID()->GetMap(-1,MAMASK);
  if(map==0) return;
  for(i=1;i<map->GetNbinsX()+1;i++){
    for(j=1;j<map->GetNbinsY()+1;j++){
      con = (int)map->GetBinContent(map->GetBin(i,j));
      if( con ){
        if(con&1 || con&1024) // digital: hits (1) and ToT (1024)
          badDig++;
        // 32=source hit, 64=source ToT, 128=masked, 65536=hot in source. -> others not used here
        else if(con&32 || con&64 || con&128 || con&65536)
          badSrc++;
        // 2=disc. bumps, 2048=bad thresh chi2, 16348= bad ToT chi2, 
        // 8,8192=merged&xtalk, 4096=monleak, 32768=intime
        else if(con&2 || con&2048 || con&16384 || con&8 || con&8192 || con&4096 || con&32768)
          badAna++;
//         else if(con&256 || con&512) // noise & threshold cut
//           ;
      }
    }
  }

  float neffBad = 0, badNse, badThr, badXT, badITT, badTOT, badVDD, neffLeak=0;
  badThr = calcNeffBad(m_sdThr,60,20,25);
  neffBad += badThr;
  badNse = calcNeffBad(m_avgNse,160,20,25);
  neffBad += badNse;
  badXT = calcNeffBad(m_avgXtalk,1,0.3f,25);
  neffBad += badXT; 
  badITT = calcNeffBad(m_avgITThresh,5550,250,50);
  neffBad += badITT;
  badTOT = calcNeffBad(m_avgTOT,34.5,1,25);
  neffBad += badTOT;
  badVDD = 0;
  if(m_minVDD>1760)
    badVDD = calcNeffBad(m_minVDD,1790,30,25);
  neffBad += badVDD; 
  neffLeak = (int)460.8*(m_IV_oper-2000)/90000;
  if(neffLeak<0)
    neffLeak = 0;
  neffBad += (float)neffLeak;
  // add "real" bad pixel
  neffBad += .5+(float)(badSrc+badDig+badAna);
  // store result
  m_neff = (int)neffBad;

  QPixmap pm(10,10);
  pm.fill(Qt::red);

  QualiTable->setText(0,1,QString::number(badSrc));
  QualiTable->setText(1,1,QString::number(badDig));
  QualiTable->setText(2,1,QString::number(badAna));
  QualiTable->setText(0,2,QString::number(badSrc));
  if(badSrc>10) QualiTable->setPixmap(0,2,pm);
  QualiTable->setText(1,2,QString::number(badDig));
  if(badDig>10) QualiTable->setPixmap(1,2,pm);
  QualiTable->setText(2,2,QString::number(badAna));
  if(badAna>10) QualiTable->setPixmap(2,2,pm);
  QualiTable->setText(3,1,QString::number((int)(m_avgNse+.5)));
  QualiTable->setText(3,2,QString::number((int)badNse));
  if(badNse>0) QualiTable->setPixmap(3,2,pm);
  QualiTable->setText(4,1,QString::number(m_sdThr,'f',1));
  QualiTable->setText(4,2,QString::number((int)badThr));
  if(badThr>0) QualiTable->setPixmap(4,2,pm);
  QualiTable->setText(5,1,QString::number((int)neffBad));
  QualiTable->setText(5,2,QString::number((int)neffBad));
  QualiTable->setText(6,1,QString::number(m_avgXtalk,'f',2));
  QualiTable->setText(6,2,QString::number((int)badXT));
  if(badXT>0) QualiTable->setPixmap(6,2,pm);
  QualiTable->setText(7,1,QString::number((int)m_avgITThresh));
  QualiTable->setText(7,2,QString::number((int)badITT));
  if(badITT>0) QualiTable->setPixmap(7,2,pm);
  QualiTable->setText(8,1,QString::number(m_avgTOT,'f',1));
  QualiTable->setText(8,2,QString::number((int)badTOT));
  if(badTOT>0) QualiTable->setPixmap(8,2,pm);
  QualiTable->setText(9,1,QString::number(m_minVDD/1000,'f',2));
  QualiTable->setText(9,2,QString::number((int)badVDD));
  if(badVDD>0) QualiTable->setPixmap(9,2,pm);
  QualiTable->setText(10,1,QString::number((int)(m_IV_oper*1000)));
  QualiTable->setText(10,2,QString::number((int)neffLeak));
  if(neffLeak>0) QualiTable->setPixmap(10,2,pm);

  recalcMech();

  return;
}
float QAReport::calcNeffBad(float val, float mean, float sigma, float weight){
  float retval=0;
  if(val<=0) return 0;

  if(sigma<=0) return 0;

  retval = ((val-mean)*(val-mean)/sigma/sigma-1)*weight;
  if(retval>0)
    return retval;
  else 
    return 0;
}
void  QAReport::recalcMech(){
  float neffBad = (float)m_neff;
  int nbow=0, nclust=0;

  nbow = (int)(0.5+460.8*pow((double)moduleBow->value()/50-1,3));
  if(nbow<0) nbow = 0;
  neffBad += nbow;

  if(bareReworked->isChecked())
    neffBad += 202;
  if(flexReworked->isChecked())
    neffBad += 46*(1+9*(int)fp_beforeAssy->isChecked());
  if(MCCReworked->isChecked())
    neffBad += 46*(1+9*(int)mcc_beforeAssy->isChecked());

  nclust = 280*discRegions->value();
  neffBad += nclust;

  QualiTable->setText(5,1,QString::number((int)neffBad));
  QualiTable->setText(5,2,QString::number((int)neffBad));
  return;
}

void QAReport::processLink(const QString &link_text){
  QString   cuttxt = ReportText->text();
  bool isOK;
  Q3ListViewItem* plotid = (Q3ListViewItem*)link_text.toInt(&isOK);
  ModItem *item=0;
  if(isOK)
    item = dynamic_cast<ModItem*> (plotid);
  if(item!=0)
    m_parent->ModuleList_itemselected(item);
  ReportText->setText(cuttxt);
  ReportText->repaint();
  if(m_app!=0) m_app->processEvents();
  return;
}
void QAReport::saveTable(const char *path){
  FILE *quali = fopen(path,"w");
  for(int j=0;j<QualiTable->numCols();j++){
    fprintf(quali,"%s",QualiTable->horizontalHeader()->label(j).latin1());
    if(j<QualiTable->numCols()-1) fprintf(quali,"\t");
    else                            fprintf(quali,"\n");
  }
  for(int k=0;k<QualiTable->numRows();k++){
    //fprintf(quali,"%s\t",QualiTable->verticalHeader()->label(j).latin1());
    for(int j=0;j<QualiTable->numCols();j++){
      fprintf(quali,"%s",QualiTable->text(k,j).latin1());
      if(j<QualiTable->numCols()-1) fprintf(quali,"\t");
      else                            fprintf(quali,"\n");
    }
  }
  fclose(quali);
}
