#include <qvariant.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <q3table.h>
#include <qstring.h>
#include <q3filedialog.h>
#include <qlabel.h>
#include <q3header.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <q3multilineedit.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <qcombobox.h>
#include <q3groupbox.h>
#include <qspinbox.h>
#include <qtooltip.h>

#include "LogDisp.h"
#include "DataStuff.h"
#include "LogClasses.h"
#include "OptClass.h"
#include "ModTree.h"

#include <math.h>

LogDisp::LogDisp(QWidget* parent, const char* , bool , Qt::WFlags fl,TLogFile *logptr, optClass *optptr,
		 bool showScanCfg)
  : QDialog(parent, fl){
  //  : LogDispBase(parent,name,modal, fl){

  setupUi(this);
  QObject::connect(CloseButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(ResetButt, SIGNAL(clicked()), this, SLOT(ResetLog()));
  QObject::connect(SaveButt, SIGNAL(clicked()), this, SLOT(SaveLog()));
  QObject::connect(TabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(SetButtons(QWidget*)));

  m_CurrentFile = logptr;
  m_opts = optptr;
  m_dataItem = 0;

  InitPanel(showScanCfg);
  return;
}
LogDisp::LogDisp(QWidget* parent, const char* , bool , Qt::WFlags fl,ModItem *dataItem, optClass *optptr,
		 bool showScanCfg)
  : QDialog(parent, fl){
//  : LogDispBase(parent,name,modal, fl){

  setupUi(this);
  QObject::connect(CloseButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(ResetButt, SIGNAL(clicked()), this, SLOT(ResetLog()));
  QObject::connect(SaveButt, SIGNAL(clicked()), this, SLOT(SaveLog()));
  QObject::connect(TabWidget, SIGNAL(currentChanged(int)), this, SLOT(SetButtons(int)));

  m_opts = optptr;
  m_dataItem = dataItem;
  m_CurrentFile = 0;
  if(m_dataItem!=0 && m_dataItem->GetDID()!=0){
    m_CurrentFile = m_dataItem->GetDID()->GetLog();
    if(m_dataItem->DatParent()!=0)
      m_dataItem->DatParent()->m_lock=true;
  }

  InitPanel(showScanCfg);
  return;
}
void LogDisp::InitPanel(bool showScanCfg){
  Q3Header *hdr;
  int i;
  QString chip;
  // set up global DAC table
  hdr = GlobalTable->verticalHeader();
  for(i=0;i<16;i++){
    chip.sprintf("%d",i);
    hdr->setLabel(i,chip);
  }
  hdr = GlobalTable->horizontalHeader();
  hdr->setLabel(0,"Config");
  hdr->setLabel(1,"Scan");
  hdr->setLabel(2,"DACs");
  hdr->setLabel(3,"GDAC");
  hdr->setLabel(4,"IVDD2");
  hdr->setLabel(5,"ID");
  hdr->setLabel(6,"IP");
  hdr->setLabel(7,"IP2");
  hdr->setLabel(8,"TrimT");
  hdr->setLabel(9,"IF");
  hdr->setLabel(10,"TrimF");
  hdr->setLabel(11,"ITh1");
  hdr->setLabel(12,"ITh2");
  hdr->setLabel(13, "IL");
  hdr->setLabel(14, "IL2");
  hdr->setLabel(15, "TSP");
  hdr->setLabel(16, "CTST");
  for(i=0;i<GlobalTable->numCols();i++)
    GlobalTable->setColumnWidth(i,50);

  // set up pixel DAC table
  hdr = PixelTable->verticalHeader();
  for(i=0;i<16;i++){
    chip.sprintf("%d",i);
    hdr->setLabel(i,chip);
  }
  hdr = PixelTable->horizontalHeader();
  hdr->setLabel(0,"ColMask");
  hdr->setLabel(1,"TDAC");
  hdr->setLabel(2,"FDAC");
  hdr->setLabel(3,"R/o Mask");
  hdr->setLabel(4,"Hitbus Mask");
  hdr->setLabel(5,"Strobe Mask");
  hdr->setLabel(6,"Preamp Mask");
  for(i=0;i<PixelTable->numCols();i++)
    PixelTable->setColumnWidth(i,100);

  // set up calibration table
  hdr = CalibTable->verticalHeader();
  for(i=0;i<16;i++){
    chip.sprintf("%d",i);
    hdr->setLabel(i,chip);
  }
  hdr = CalibTable->horizontalHeader();
  hdr->setLabel(0,"VCAL");
  hdr->setLabel(1,"C_low");
  hdr->setLabel(2,"C_high");
  hdr->setLabel(3,"Q_low");
  hdr->setLabel(4,"Q_high");
  hdr->setLabel(5,"VCAL off");
  hdr->setLabel(6,"VCAL Qu");
  hdr->setLabel(7,"VCAL Cu");
  hdr->setLabel(8,"MLcal");
  for(i=0;i<CalibTable->numCols();i++)
    CalibTable->setColumnWidth(i,70);
  
  // set up FE error table
  hdr = FEErrTable->verticalHeader();
  for(i=0;i<16;i++){
    chip.sprintf("%d",i);
    hdr->setLabel(i,chip);
  }
  hdr = FEErrTable->horizontalHeader();
  hdr->setLabel(0,"Potential buffer Overflows");
  hdr->setLabel(1,"Illegal column address");
  hdr->setLabel(2,"Illegal row address");
  hdr->setLabel(3,"Instances of missing EoE");
  hdr->setLabel(4,"Unsequential LV1s");
  hdr->setLabel(5,"Unsequential BCIDs");
  hdr->setLabel(6,"Unmatched BCIDs");
  hdr->setLabel(7,"Trigger FIFO Hamming code errors");
  hdr->setLabel(8,"Parity upset in register");
  hdr->setLabel(9,"Parity flags in EoEs");
  hdr->setLabel(10,"Parity upsets in hits");
  hdr->setLabel(11,"Parity upsets in EoEs");
  hdr->setLabel(12,"Bit flip warnings");
  hdr->setLabel(13,"MCC Hit Overflows");
  hdr->setLabel(14,"MCC EoE Overflows");
  hdr->setLabel(15,"MCC BCID within FE");
  hdr->setLabel(16,"MCC BCID between FEs");
  hdr->setLabel(17,"MCC WFE register instances");
  hdr->setLabel(18,"MCC WMCC register instances");
  for(i=0;i<FEErrTable->numCols();i++)
    FEErrTable->setColumnWidth(i,150);
  
  // set up MCC strobe cal table
  hdr = MCCStrCal->horizontalHeader();
  for(i=0;i<16;i++){
    chip.sprintf("%d",i);
    hdr->setLabel(i,chip);
    MCCStrCal->setColumnWidth(i,50);
  }
  MCCStrCal->verticalHeader()->setLabel(0," ");

  // if requested, hide scan config table (dealt with in other panels then)
  TabWidget->setTabEnabled(TabWidget->page(4),showScanCfg);
  TabWidget->setTabEnabled(TabWidget->page(6),showScanCfg);

  // load data if we've got a pointer
  if(m_CurrentFile!=NULL){
    DisplayGDACs();
    DisplayPDACs();
    DisplayCalib();
    DisplayTypeOfMeasurement();
    DisplayInjection();
    DisplayErr();
    edComments->setText(m_CurrentFile->comments.c_str());
    QString verlab = "";
    if(m_CurrentFile->TDAQversion>0){
      verlab = "TurboDAQ version " + QString::number(m_CurrentFile->TDAQversion,'f',2);
      if(m_CurrentFile->MAFVersion!="none"){
	verlab += " - from file created with MAF version ";
	verlab += m_CurrentFile->MAFVersion.c_str();
      }
    }
    VerLabel->setText(verlab);
    if(m_CurrentFile->PLLver>0){
      PLLlabel->setText("PLL ver "+QString::number(m_CurrentFile->PLLver%1000)+
			" SN " + QString::number(m_CurrentFile->PLLver/1000));
      phaseLabel->show();
      pllPhase1->show();
      pllPhase2->show();
      int phase1, phase2;
      m_CurrentFile->getPLLPhases(phase1,phase2);
      pllPhase1->setText(QString::number(phase1));
      pllPhase2->setText(QString::number(phase2));
    }else if(m_CurrentFile->PLLver<0){
      PLLlabel->setText("ROD rev "+QString::number((-m_CurrentFile->PLLver)/1000,16)+
    			" SN " + QString::number((-m_CurrentFile->PLLver)%1000));
      phaseLabel->hide();
      pllPhase1->hide();
      pllPhase2->hide();
    }else{
      PLLlabel->setText("");
      phaseLabel->hide();
      pllPhase1->hide();
      pllPhase2->hide();
    }
    if(m_CurrentFile->PCCver>0)
      PCClabel->setText("PCC ver "+QString::number(m_CurrentFile->PCCver%1000)+
			" SN " + QString::number(m_CurrentFile->PCCver/1000));
    else if(m_CurrentFile->PCCver<0)
      PCClabel->setText("MDSP rev "+QString::number((-m_CurrentFile->PCCver)/1000)+
    			" code ver " + QString::number((-m_CurrentFile->PCCver)%1000,16));
    else
      PCClabel->setText("");
  }

}

LogDisp::~LogDisp(){
  if(m_dataItem!=0 && m_dataItem->DatParent()!=0)
    m_dataItem->DatParent()->m_lock=false;
  //if(m_CurrentFile!=NULL)
  //  delete m_CurrentFile;
}
void LogDisp::LoadFile(const char *path){
  m_CurrentFile = new TLogFile();
  m_CurrentFile->ReadoutLog(path);
  InitPanel(true);
  DisplayGDACs();
  DisplayPDACs();
  DisplayCalib();
  DisplayTypeOfMeasurement();
  DisplayInjection();
  DisplayErr();
  edComments->setText(m_CurrentFile->comments.c_str());
  VerLabel->setText("TurboDAQ version " + QString::number(m_CurrentFile->TDAQversion,'f',1));

  return;
}
void LogDisp::DisplayPDACs(){
  for (int Chip = 0; Chip < PixelTable->numRows(); Chip++){
    char cpsw[10];
    //    sprintf(cpsw,"");
    cpsw[0]='\0';
    for(int i=0;i<9;i++){
      int j = m_CurrentFile->MyModule->Chips[Chip]->ColMask>>i;
      sprintf(cpsw,"%s%d",cpsw,(j&1));
    }
    QString cpswstr(cpsw);
    PixelTable->setText(Chip,0,cpswstr);
    QString DACstr;
    DACstr.sprintf("%d",m_CurrentFile->MyModule->Chips[Chip]->TDACs->m_mode);
    if(m_CurrentFile->MyModule->Chips[Chip]->TDACs->m_mode==32 ||
       ((m_CurrentFile->TDAQversion>=4 || m_CurrentFile->TDAQversion<0)
	&& m_CurrentFile->MyModule->Chips[Chip]->TDACs->m_mode==128)) 
      DACstr = m_CurrentFile->MyModule->Chips[Chip]->TDACs->m_filename.c_str();
    PixelTable->setText(Chip,1,DACstr);
    DACstr.sprintf("%d",m_CurrentFile->MyModule->Chips[Chip]->FDACs->m_mode);
    if(m_CurrentFile->MyModule->Chips[Chip]->FDACs->m_mode==32||
       ((m_CurrentFile->TDAQversion>=4 || m_CurrentFile->TDAQversion<0)
	 && m_CurrentFile->MyModule->Chips[Chip]->FDACs->m_mode==128)) 
      DACstr = m_CurrentFile->MyModule->Chips[Chip]->FDACs->m_filename.c_str();
    PixelTable->setText(Chip,2,DACstr);
    PixelTable->setText(Chip,3,m_CurrentFile->MyModule->Chips[Chip]->ReadoutMask->m_filename.c_str());
    PixelTable->setText(Chip,4,m_CurrentFile->MyModule->Chips[Chip]->HitbusMask->m_filename.c_str());
    PixelTable->setText(Chip,5,m_CurrentFile->MyModule->Chips[Chip]->StrobeMask->m_filename.c_str());
    PixelTable->setText(Chip,6,m_CurrentFile->MyModule->Chips[Chip]->PreampMask->m_filename.c_str());
  }
}
void LogDisp::DisplayGDACs(){
  QString DACstr;
  PhiClock->setCurrentItem(m_CurrentFile->MyModule->PhiClk+1);
  HitbusEnabled->setChecked(m_CurrentFile->MyModule->HitbusEnabled);
  FEflavour->setCurrentItem(m_CurrentFile->MyModule->FEflavour+1);
  buffEnabled->setChecked((bool)m_CurrentFile->MyModule->BuffEnabled);
  buffBoost->setChecked((bool)m_CurrentFile->MyModule->buffboost);
  BufferMUX->setCurrentItem(m_CurrentFile->MyModule->BuffMux+1);
  TOTmode->setCurrentItem(m_CurrentFile->MyModule->TOTmode+1);
  minTOT->setText(QString::number(m_CurrentFile->MyModule->minTOT));
  dblTOT->setText(QString::number(m_CurrentFile->MyModule->dblTOT));
  TwalkTOT->setText(QString::number(m_CurrentFile->MyModule->TwalkTOT));
  if(m_CurrentFile->MyModule->isMCCmod){
    isMCCmodBox->setChecked((bool)m_CurrentFile->MyModule->isMCCmod);
    PixelTable->setNumRows(16);
    GlobalTable->setNumRows(16);
    CalibTable->setNumRows(16);
    MCC_Bwidth->setCurrentItem(m_CurrentFile->MyModule->MCC->OutputBandwidth+1);
    MCC_ErrorFlag->setChecked(m_CurrentFile->MyModule->MCC->ErrorFlag);
    MCC_FECheck->setChecked(m_CurrentFile->MyModule->MCC->FECheck);
    MCC_TimeStampComp->setChecked(m_CurrentFile->MyModule->MCC->TimeStampComp);
    MCCFlavour->setCurrentItem(m_CurrentFile->MyModule->MCC->flavour);
    for(int i=0;i<16;i++)
      MCCStrCal->setText(0,i,QString::number(m_CurrentFile->MyModule->MCC->strobe_cal[i],'f',3));
  } else{
    PixelTable->setNumRows(1);
    GlobalTable->setNumRows(1);
    CalibTable->setNumRows(1);
    BwidthLabel->hide();
    MCC_Bwidth->hide();
    MCC_ErrorFlag->hide();
    MCC_FECheck->hide();
    MCC_TimeStampComp->hide();
    MCCStrCal->hide();
  }

  ModName->setText(m_CurrentFile->MyModule->modname.c_str());

  int Chip, DAC;

  for (Chip = 0; Chip < GlobalTable->numRows(); Chip++){
    for (DAC = 0; DAC < 3; DAC++){
      if(m_CurrentFile->MyModule->Chips[Chip]->Switches[DAC])
	GlobalTable->setText(Chip,DAC,"ON");
      else
	GlobalTable->setText(Chip,DAC,"OFF");
    }
    if(m_CurrentFile->MyModule->FEflavour==1){
      DACstr.sprintf("%d",m_CurrentFile->MyModule->Chips[Chip]->GDAC);
      GlobalTable->setText(Chip,3,DACstr);
    }
    for (DAC = 0; DAC < 11; DAC++){
      DACstr.sprintf("%d",m_CurrentFile->MyModule->Chips[Chip]->DACs[DAC]);
      GlobalTable->setText(Chip,DAC+4,DACstr);
    }
    for (DAC = 0; DAC < 2; DAC++){
      if(m_CurrentFile->MyModule->Chips[Chip]->Switches[DAC+3])
	GlobalTable->setText(Chip,DAC+15,"ON");
      else
	GlobalTable->setText(Chip,DAC+15,"OFF");
    }
  }
}

void LogDisp::DisplayCalib(){
  QString Calstr;
  float qcal;
  for (int Chip = 0; Chip < CalibTable->numRows(); Chip++){
    CalibTable->setText(Chip,0,QString::number(m_CurrentFile->MyModule->Chips[Chip]->Vcal,'f',4));
    CalibTable->setText(Chip,1,QString::number(m_CurrentFile->MyModule->Chips[Chip]->Clo,'f',4));
    CalibTable->setText(Chip,2,QString::number(m_CurrentFile->MyModule->Chips[Chip]->Chi,'f',4));
    qcal = m_CurrentFile->MyModule->Chips[Chip]->Clo*m_CurrentFile->MyModule->Chips[Chip]->Vcal/0.160218;
    CalibTable->setText(Chip,3,QString::number(qcal,'f',2));
    qcal = m_CurrentFile->MyModule->Chips[Chip]->Chi*m_CurrentFile->MyModule->Chips[Chip]->Vcal/0.160218;
    CalibTable->setText(Chip,4,QString::number(qcal,'f',2));
    qcal = m_CurrentFile->MyModule->Chips[Chip]->VcalOff;
    if(fabs(qcal)>.0001 || qcal==0)
      Calstr.sprintf("%.4f",qcal);
    else
      Calstr.sprintf("%.2e",qcal);
    CalibTable->setText(Chip,5,Calstr);
    qcal = m_CurrentFile->MyModule->Chips[Chip]->VcalQu;
    if(fabs(qcal)>.0001 || qcal==0)
      Calstr.sprintf("%.4f",qcal);
    else
      Calstr.sprintf("%.2e",qcal);
    CalibTable->setText(Chip,6,Calstr);
    qcal = m_CurrentFile->MyModule->Chips[Chip]->VcalCu;
    if(fabs(qcal)>.0001 || qcal==0)
      Calstr.sprintf("%.4f",qcal);
    else
      Calstr.sprintf("%.2e",qcal);
    CalibTable->setText(Chip,7,Calstr);
    CalibTable->setText(Chip,8,QString::number(m_CurrentFile->MyModule->Chips[Chip]->MLcal,'f',4));
  }
  // do voltage etc here for the moment
  if(m_opts!=NULL && m_CurrentFile->MyDCS->LVvolt[m_opts->m_DDsupp][m_opts->m_DDchan]>=0)
    edVDD->setText(QString::number(m_CurrentFile->MyDCS->LVvolt[m_opts->m_DDsupp][m_opts->m_DDchan],'f',3));
  else
    edVDD->setText("unknown");
  if(m_opts!=NULL && m_CurrentFile->MyDCS->LVcurr[m_opts->m_DDsupp][m_opts->m_DDchan]>=0)
    edIDD->setText(QString::number(m_CurrentFile->MyDCS->LVcurr[m_opts->m_DDsupp][m_opts->m_DDchan],'f',3));
  else
    edIDD->setText("unknown");
  if(m_opts!=NULL && m_CurrentFile->MyDCS->LVvolt[m_opts->m_DDAsupp][m_opts->m_DDAchan]>=0)
    edVDDA->setText(QString::number(m_CurrentFile->MyDCS->LVvolt[m_opts->m_DDAsupp][m_opts->m_DDAchan],'f',3));
  else
    edVDDA->setText("unknown");
  if(m_opts!=NULL && m_CurrentFile->MyDCS->LVcurr[m_opts->m_DDAsupp][m_opts->m_DDAchan]>=0)
    edIDDA->setText(QString::number(m_CurrentFile->MyDCS->LVcurr[m_opts->m_DDAsupp][m_opts->m_DDAchan],'f',3));
  else
    edIDDA->setText("unknown");
  if(m_opts!=NULL && m_CurrentFile->MyDCS->HVvolt[m_opts->m_BIASsupp][m_opts->m_BIASchan]>=0)
    edVbias->setText(QString::number(m_CurrentFile->MyDCS->HVvolt[m_opts->m_BIASsupp][m_opts->m_BIASchan],'f',3));
  else
    edVbias->setText("unknown");
  if(m_opts!=NULL && m_CurrentFile->MyDCS->HVcurr[m_opts->m_BIASsupp][m_opts->m_BIASchan]>=0)
    edIbias->setText(QString::number(m_CurrentFile->MyDCS->HVcurr[m_opts->m_BIASsupp][m_opts->m_BIASchan],'f',3));
  else
    edIbias->setText("unknown");
  if(m_CurrentFile->MyDCS->Tntc>-1000)
    edTNTC->setText(QString::number(m_CurrentFile->MyDCS->Tntc,'f',3));
  else
    edTNTC->setText("unknown");
  return;
}

void LogDisp::DisplayTypeOfMeasurement(){
 QString tmpstr;
 int i;

 tmpstr = "Inner scan - ";
 tmpstr += m_CurrentFile->MyScan->GetInner().c_str();
 InnerBox->setTitle(tmpstr);
 tmpstr.sprintf("%d",m_CurrentFile->MyScan->Inner->Step);
 edInnerStep->setText(tmpstr);
 if(m_CurrentFile->MyScan->Inner->Mode==1){
   tmpstr.sprintf("%d",m_CurrentFile->MyScan->Inner->Start);
   edInnerStart->setText(tmpstr);
   tmpstr.sprintf("%d",m_CurrentFile->MyScan->Inner->Stop);
   edInnerStop->setText(tmpstr);
 }else{
   tmpstr = "from file";
   edInnerStart->setText(tmpstr);
   edInnerStop->setText(tmpstr);
 }
 InnerPtsBox->clear();
 for(i=0;i<500 && m_CurrentFile->MyScan->Inner->Pts[i]!=-99999;i++){
   tmpstr.sprintf("%d",m_CurrentFile->MyScan->Inner->Pts[i]);
   InnerPtsBox->insertItem(tmpstr,i);
 }

 tmpstr = "Outer scan - ";
 tmpstr += m_CurrentFile->MyScan->GetOuter().c_str();
 OuterBox->setTitle(tmpstr);
 tmpstr.sprintf("%d",m_CurrentFile->MyScan->Outer->Step);
 edOuterStep->setText(tmpstr);
 if(m_CurrentFile->MyScan->Outer->Mode==1){
   tmpstr.sprintf("%d",m_CurrentFile->MyScan->Outer->Start);
   edOuterStart->setText(tmpstr);
   tmpstr.sprintf("%d",m_CurrentFile->MyScan->Outer->Stop);
   edOuterStop->setText(tmpstr);
 }else{
   tmpstr = "from file";
   edOuterStart->setText(tmpstr);
   edOuterStop->setText(tmpstr);
 }
 OuterPtsBox->clear();
 for(i=0;i<500 && m_CurrentFile->MyScan->Outer->Pts[i]!=-99999;i++){
   tmpstr.sprintf("%d",m_CurrentFile->MyScan->Outer->Pts[i]);
   OuterPtsBox->insertItem(tmpstr,i);
 }
}
void LogDisp::DisplayInjection(){
  QString tmpstr;
  edInjection->setText(m_CurrentFile->MyModule->GetInjection().c_str());
  edNevents->setText(QString::number(m_CurrentFile->MyScan->Nevents));
  switch (m_CurrentFile->MyModule->Injection){
  case 0:
  default:
    edVCal->setText("");
    cbCHigh->hide();
    extCal->hide();
    extCalLabel->hide();
    break;
  case 1:
    tmpstr.sprintf("%d",m_CurrentFile->MyModule->FE_VCal);
    edVCal->setText(tmpstr);
    cbCHigh->show();
    extCal->hide();
    extCalLabel->hide();
    break;
  case 2:
    tmpstr.sprintf("%d ; %d",m_CurrentFile->MyScan->TPCC_VCal_L,
		   m_CurrentFile->MyScan->TPCC_VCal_H);
    edVCal->setText(tmpstr);
    cbCHigh->hide();
    extCal->show();
    extCalLabel->show();
    extCal->setText(QString::number(m_CurrentFile->extcal,'f',4));
    break;
  }
  cbChipByChip->setCurrentItem(m_CurrentFile->MyScan->ChipByChip);
  cbCHigh->setCurrentItem(m_CurrentFile->MyModule->CHigh);
  MCCdelrg->setText(QString::number(m_CurrentFile->MyModule->MCC->StrbDelrg));
  MCCdel->setText(QString::number(m_CurrentFile->MyModule->MCC->StrbDel));
  StrbInterval->setText(QString::number(m_CurrentFile->MyScan->StrbInterval));
  StrbLength->setText(QString::number(m_CurrentFile->MyScan->StrbLength));

  TrgDelay->setText(QString::number(m_CurrentFile->MyScan->TrgDelay));
  FElatency->setText(QString::number(m_CurrentFile->MyModule->FElatency));
  edNaccepts->setText(QString::number(m_CurrentFile->MyModule->MCC->Naccepts));
  TrgMode->setCurrentItem(m_CurrentFile->MyScan->TrgMode+1);
  TrgType->setCurrentItem(m_CurrentFile->MyScan->TrgType+1);

  maskIsStatic->setChecked((bool)m_CurrentFile->MyScan->MaskScanMode);
  if(m_CurrentFile->MyScan->MaskScanMode){
    maskShift->hide();
    maskSteps->hide();
    maskStepLabel1->hide();
    maskStepLabel2->hide();
    maskTypeLabel->hide();
    if(m_CurrentFile->MyScan->MaskStaticMode==0)
      maskShiftMode->setText("Regular mode");
    else
      maskShiftMode->setText("FEs all off when not scanned");
  } else{
    maskShift->show();
    maskSteps->show();
    maskStepLabel1->show();
    maskStepLabel2->show();
    maskTypeLabel->show() ;
    maskShift->setText(m_CurrentFile->MyScan->GetMaskShift().c_str());
    maskShiftMode->setText(m_CurrentFile->MyScan->GetMaskMode().c_str());
    maskSteps->setText(QString::number(m_CurrentFile->MyScan->MaskSteps));
  }
}
void LogDisp::DisplayErr(){
  for (int Chip = 0; Chip < FEErrTable->numRows(); Chip++){
    FEErrTable->setText(Chip,0,QString::number(m_CurrentFile->MyErr->buff_ovfl[Chip]));
    FEErrTable->setText(Chip,1,QString::number(m_CurrentFile->MyErr->illg_col[Chip]));
    FEErrTable->setText(Chip,2,QString::number(m_CurrentFile->MyErr->illg_row[Chip]));
    FEErrTable->setText(Chip,3,QString::number(m_CurrentFile->MyErr->miss_eoe[Chip]));
    FEErrTable->setText(Chip,4,QString::number(m_CurrentFile->MyErr->unseq_lv1[Chip]));
    FEErrTable->setText(Chip,5,QString::number(m_CurrentFile->MyErr->unseq_bcid[Chip]));
    FEErrTable->setText(Chip,6,QString::number(m_CurrentFile->MyErr->unm_bcid[Chip]));
    FEErrTable->setText(Chip,7,QString::number(m_CurrentFile->MyErr->hamming_err[Chip]));
    FEErrTable->setText(Chip,8,QString::number(m_CurrentFile->MyErr->par_ups[Chip]));
    FEErrTable->setText(Chip,9,QString::number(m_CurrentFile->MyErr->par_flags[Chip]));
    FEErrTable->setText(Chip,10,QString::number(m_CurrentFile->MyErr->par_hits[Chip]));
    FEErrTable->setText(Chip,11,QString::number(m_CurrentFile->MyErr->par_eoe[Chip]));
    FEErrTable->setText(Chip,12,QString::number(m_CurrentFile->MyErr->bitflip[Chip]));
    FEErrTable->setText(Chip,13,QString::number(m_CurrentFile->MyErr->mcc_hovfl[Chip]));
    FEErrTable->setText(Chip,14,QString::number(m_CurrentFile->MyErr->mcc_eovfl[Chip]));
    FEErrTable->setText(Chip,15,QString::number(m_CurrentFile->MyErr->mcc_bcin[Chip]));
    FEErrTable->setText(Chip,16,QString::number(m_CurrentFile->MyErr->mcc_bcbw[Chip]));
    FEErrTable->setText(Chip,17,QString::number(m_CurrentFile->MyErr->mcc_wfe[Chip]));
    FEErrTable->setText(Chip,18,QString::number(m_CurrentFile->MyErr->mcc_wmcc[Chip]));
  }
  LV1bf->setValue(m_CurrentFile->MyErr->mcc_lvbf);
  Badfc->setValue(m_CurrentFile->MyErr->mcc_badfc);
  Badsc->setValue(m_CurrentFile->MyErr->mcc_badsc);
  Error0->setValue(m_CurrentFile->MyErr->mcc_err0);
  Error1->setValue(m_CurrentFile->MyErr->mcc_err1);
}
void LogDisp::SaveLog(){
  m_CurrentFile->comments = edComments->text().latin1();
}
void LogDisp::ResetLog(){
  edComments->setText(m_CurrentFile->comments.c_str());
}
void LogDisp::SetButtons(int){
  if(TabWidget->tabLabel(TabWidget->currentPage())=="Comments"){
    SaveButt->show();
    ResetButt->show();
  } else{
    SaveButt->hide();
    ResetButt->hide();
  }
  return;
}

