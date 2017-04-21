#include "RootStuff.h"
#include "OptWin.h"
#include "TopWin.h"
#include "OptClass.h"
#include "FileTypes.h"
#include "StdTestWin.h"
#include "MAEngine.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <q3table.h>
#include <q3header.h>
#include <q3filedialog.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <QDoubleSpinBox>

OptWin::OptWin( QWidget* parent, const char* , bool , Qt::WFlags fl) 
  : QDialog(NULL,fl){

  setupUi(this);

  m_Opts = new optClass();

  new PathItem(DataPath,DataLabel,DataBrowse);
  new PathItem(CutsPath,CutsLabel,CutsBrowse);
  new PathItem(MacrosPath,MacrosLabel,MacrosBrowse);
  new PathItem(PDBPath,PDBLabel,PDBBrowse);

  int i, ntypes=0;

  Q3Header *hdr = StdTable->horizontalHeader();
  hdr->setLabel(0,"Data set");
  hdr->setLabel(1,"File name");
  StdTable->setColumnWidth(0,250);
  StdTable->setColumnWidth(1,250);
  StdTestWin sw(parent);
  StdTable->setNumRows(sw.NSLOTS);
  for(i=0;i<sw.NSLOTS;i++){
    StdTable->setText(i,0,sw.m_items[i]->m_label->text());
    StdTable->setText(i,1,"");
  }

  hdr = RangeTable->horizontalHeader();
  hdr->setLabel(0,"Name");
  hdr->setLabel(1,"Min");
  hdr->setLabel(2,"Max");
  hdr->setLabel(3,"Enab.");
  RangeTable->setColumnWidth(0,250);
  RangeTable->setColumnWidth(1,80);
  RangeTable->setColumnWidth(2,80);
  RangeTable->setColumnWidth(3,30);
  hdr = RangeTable->verticalHeader();
  RangeTable->setNumRows(1);
  for(i=0;i<DSET;i++){
    if(m_Opts->hinam[i]!=""){ // projections must have a name
      hdr->setLabel(ntypes,QString::number(i));
      RangeTable->setText(ntypes,0,m_Opts->hitit[i].c_str());
      RangeTable->setNumRows(RangeTable->numRows()+1);
      ntypes++;
    }
  }
  RangeTable->setNumRows(RangeTable->numRows()-1);

#ifdef PIXEL_ROD
  if(m_Opts->m_PLhistoExists[0]){ // only applied to PixLib histos, otherwise disable page
    TabWidget2->setTabEnabled(TabWidget2->page(2),true);
    hdr = PixLibRangeTable->horizontalHeader();
    hdr->setLabel(0,"Histogram");
    hdr->setLabel(1,"Min");
    hdr->setLabel(2,"Max");
    hdr->setLabel(3,"Enab.");
    hdr->setLabel(4,"Plot style (if appl.)");
    hdr->setLabel(5,"Plt <0");
    PixLibRangeTable->setColumnWidth(0,175);
    PixLibRangeTable->setColumnWidth(1,80);
    PixLibRangeTable->setColumnWidth(2,80);
    PixLibRangeTable->setColumnWidth(3,30);
    PixLibRangeTable->setColumnWidth(4,250);
    PixLibRangeTable->setColumnWidth(5,80);
    hdr = PixLibRangeTable->verticalHeader();
    PixLibRangeTable->setNumRows(1);
    ntypes=0;
    PixScan *ps = new PixScan();
    std::map<std::string, int> hiTypes = ps->getHistoTypes();
    QStringList stringList;
    for(std::vector<std::string>::iterator IT=m_Opts->m_PLplotTypeLabels.begin(); IT!=m_Opts->m_PLplotTypeLabels.end(); IT++)
      stringList << (*IT).c_str();
    for(std::map<std::string, int>::iterator it = hiTypes.begin(); it!=hiTypes.end(); it++){
      if(it->second<NPLHIMAX && m_Opts->m_PLhistoExists[it->second]){
	hdr->setLabel(ntypes,QString::number(it->second));
	PixLibRangeTable->setText(ntypes,0,it->first.c_str());
	PixLibRangeTable->setNumRows(PixLibRangeTable->numRows()+1);
	Q3ComboTableItem *comboItem = new Q3ComboTableItem ( PixLibRangeTable, stringList, false );
	comboItem->setCurrentItem( 2 );
	PixLibRangeTable->setItem( ntypes, 4, comboItem); 
	ntypes++;
      }
    }
    PixLibRangeTable->setNumRows(PixLibRangeTable->numRows()-1);
    delete ps;
  } else
#endif
    TabWidget2->setTabEnabled(TabWidget2->page(2),false);

  totcheckVar->clear();
  for(i=0;i<20;i++){
    QString tmpstr=MAEngine::getToTCheckLabel(i).c_str();
    if(tmpstr=="value not used") break;
    totcheckVar->insertItem(tmpstr);
  }

  return;
}

OptWin::~OptWin(){
}

void OptWin::SC_clicked(){
  Save_clicked();
  accept();
}

void OptWin::Save_clicked(){
  m_Opts->m_SCmode     = SCModeBox->isChecked();
  m_Opts->m_splitnsp   = SplitNoiseProj->isChecked();
  m_Opts->m_splitthr   = SplitThreshProj->isChecked();
  m_Opts->m_showwarn   = ShowWarn->isChecked();
  m_Opts->m_showchipid = (int) ChipLabelBox->isChecked();
  m_Opts->m_showAxisLabels = showAxisLabels->isChecked();
  m_Opts->m_projlogy   = (int) DoProjLogy->isChecked();
  m_Opts->m_hitcut     = (float) HITcut->value();
  m_Opts->m_totcut     = (float) TOTcut->value();
  m_Opts->m_noisecut   = (float) NOISEcut->value();
  m_Opts->m_showcutpix = ShowAllUnconn->isChecked();
  m_Opts->m_diffcut    = (float) DIFFcut->value();
  m_Opts->m_xtfrcut    = XTFRcut->value();
  m_Opts->m_xtnoisecut = XTNOISEcut->value();
  m_Opts->m_noisetab   = (int) NOISEtab->isChecked();
  m_Opts->m_zerotab    = (int) TabZeroBox->isChecked();
  m_Opts->m_masklong   = (int) LongBox->isChecked();
  m_Opts->m_maskgang   = (int) GangedBox->isChecked();
  m_Opts->m_QhighTW    = (int) Qt0TW->value();
  m_Opts->m_ChipTarget = (float) ChipTarget->value();
  m_Opts->m_doChipTarget=(int) doChipTarget->isChecked();
  m_Opts->m_doFixedTarget=(int) doFixedTarget->isChecked();
  m_Opts->m_rec2DFit   =  rec2DFit->isChecked();
  m_Opts->m_DACdoFit   =  DACdoFit->isChecked();
  m_Opts->m_FitGauss   = (int) FitGauss->isChecked();
  m_Opts->m_ToggleMask = (int) ToggleMask->isChecked();
  m_Opts->m_ToTPlotPnt = ToTPlotPnt->value();
  m_Opts->m_fixedToTSwitch = fixedTOTSwitch->isChecked();
  m_Opts->m_ToTSwitch  = TOTSwitchPnt->value();
  m_Opts->m_QPlotPnt = TOTIgnore->value();
  m_Opts->m_UsrMin = UsrMin->value();
  m_Opts->m_UsrMax = UsrMax->value();
  m_Opts->m_doUsrRg = UserRange->isChecked();
  m_Opts->m_doTotRelcal = TotRelcal->isChecked();
  m_Opts->m_runCombToTfit = runCombToTfit->isChecked();
  m_Opts->m_runBadFitsIntime = runBadFitsIntime->isChecked();
  m_Opts->m_runBadFitsTzero  = runBadFitsTzero ->isChecked();
  m_Opts->m_runBadFitsThresh = runBadFitsThresh->isChecked();
  m_Opts->m_runBadFitsXtalk  = runBadFitsXtalk ->isChecked();
  m_Opts->m_minToT = MinToT->value();
  m_Opts->m_peakMinHit = minHits->value();
  m_Opts->m_doBlackBg = doBlackBg->isChecked();
  m_Opts->m_srcMin = (float)srcMin->value();
  m_Opts->m_srcRestrict = srcRestrict->isChecked();
  m_Opts->m_srcRecalcPeak = srcRecalcPeak->isChecked();
  m_Opts->m_cfgUsrdefNames = cgfUserdefNames->isChecked();
  if(chiCutEn->isChecked())
    m_Opts->m_chiCutmT     = chiCut_MinThr->value();
  else
    m_Opts->m_chiCutmT     = -1;
  if(nseCutEn->isChecked()){
    m_Opts->m_nseCutmT        = nseCut_MinThr->value();
    m_Opts->m_nseCutmT_long   = nseCut_MinThr_long->value();
    m_Opts->m_nseCutmT_ganged = nseCut_MinThr_ganged->value();
    m_Opts->m_nseCutmT_inter  = nseCut_MinThr_inter->value();
  }else{
    m_Opts->m_nseCutmT        = -1;
    m_Opts->m_nseCutmT_long   = -1;
    m_Opts->m_nseCutmT_ganged = -1;
    m_Opts->m_nseCutmT_inter  = -1;
  }
  m_Opts->m_DefDataPath  = DataPath->text().latin1();
  m_Opts->m_UseMname     = GenFromMod->isChecked();
  m_Opts->m_DefCutPaths  = CutsPath->text().latin1();
  m_Opts->m_DefMacroPath = MacrosPath->text().latin1();
  m_Opts->m_showChipCuts = showChipCuts->isChecked();
  m_Opts->m_pdbpath      = PDBPath->text().latin1();
  m_Opts->m_pdbPlots     = storePlots->isChecked();
  m_Opts->m_pdbRemove    = removePDBFiles->isChecked();
  m_Opts->m_pdbClear     = removeDataPDB->isChecked();
  m_Opts->m_CutCreateRoot= storeROOT->isChecked();
  m_Opts->m_CutWebCopy   = copyWeb->isChecked();
  m_Opts->m_CutWebPath   = webServer->text().latin1();
  m_Opts->m_CutWebUser   = webUser->text().latin1();
  m_Opts->m_CutWebCmd    = webCommand->text().latin1();
  m_Opts->m_VbiasMin     = VbiasMin->value();
  m_Opts->m_VbiasMax     = VbiasMax->value();
  m_Opts->m_IbiasCorr    = IbiasCorr->isChecked();
  m_Opts->m_absValIV     = absIV->isChecked();
  m_Opts->m_ambushStart  = ambushStart->value();
  m_Opts->m_ambushStop   = ambushStop ->value();
  m_Opts->m_userPDB      = userPDB->text().latin1();
  m_Opts->m_passwdPDB    = passwordPDB->text().latin1();
  m_Opts->m_delT0_twalk  = deltaT0->value();
  m_Opts->m_totcheckVar  = totcheckVar->currentItem();
  m_Opts->m_totcheckValue= totcheckValue->text().toFloat();
  m_Opts->m_totcheckKeep = totcheckKeep->isChecked();
  m_Opts->m_corPlotOpt   = corEdit->text().latin1();
  m_Opts->m_occPlotHisto = occHisto->isChecked();
  if(SplitFitBox->isChecked())
    m_Opts->m_plotfitopt=0;
  else
    m_Opts->m_plotfitopt = PlotFitBox->currentItem()+1;
  m_Opts->m_colourScale = ColourComboBox->currentItem();
  RMain::ColourScale(m_Opts->m_colourScale);
  // voltage channels
  m_Opts->m_DDsupp   = VDD_Supp->value()-1;
  m_Opts->m_DDchan   = VDD_Chan->value()-1;
  m_Opts->m_DDAsupp  = VDDA_Supp->value()-1;
  m_Opts->m_DDAchan  = VDDA_Chan->value()-1;
  m_Opts->m_BIASsupp = Vbias_Supp->value()-1;
  //  m_Opts->m_BIASchan = Vbias_Chan->value()-1;
  // indiv. user ranges
  int i;
  for(i=0;i<RangeTable->numRows();i++){
    bool ok;
    int id = RangeTable->verticalHeader()->label(i).toInt(&ok);
    if(id>LOAD && id<DSET){
      m_Opts->m_UsrIMin[id] = RangeTable->text(i,1).toFloat(&ok);
      m_Opts->m_UsrIMax[id] = RangeTable->text(i,2).toFloat(&ok);
      m_Opts->m_UsrIUse[id] = (bool) RangeTable->text(i,3).toInt(&ok);
    }
  }
  for(i=0;i<(int)m_Opts->m_StdFname.size();i++)
    m_Opts->m_StdFname[i] = StdTable->text(i,1).latin1();

  for(i=0;i<PixLibRangeTable->numRows();i++){
    int id = PixLibRangeTable->verticalHeader()->label(i).toInt();
    if(id>=0 && id<NPLHIMAX && m_Opts->m_PLhistoExists[id]){
      m_Opts->m_PLUsrIMin[id]  = PixLibRangeTable->text(i,1).toFloat();
      m_Opts->m_PLUsrIMax[id]  = PixLibRangeTable->text(i,2).toFloat();
      m_Opts->m_PLUsrIUse[id]  = (bool) PixLibRangeTable->text(i,3).toInt();
      m_Opts->m_PLPltZero[id]  = (bool) PixLibRangeTable->text(i,5).toInt();
      Q3ComboTableItem *comboItem = dynamic_cast<Q3ComboTableItem *>(PixLibRangeTable->item(i,4));
      if(comboItem!=0)
	m_Opts->m_PLplotType[id] = comboItem->currentItem();
    }
  }

  m_Opts->m_webUploadOpt = webUploadMode->currentItem();

/*  switch(ModeComboBox->currentItem()){
  case 0: m_Opts->m_summode = 5;
		  break;
  case 1: m_opts->m_summode = 
  }*/
//  m_Opts->m_summode = ModeComboBox->currentItem();

  Reset_clicked();

  emit showCCclicked(m_Opts->m_showChipCuts);
}
void OptWin::Reset_clicked(){
  SCModeBox->setChecked(m_Opts->m_SCmode);
  SplitNoiseProj->setChecked(m_Opts->m_splitnsp);
  SplitThreshProj->setChecked(m_Opts->m_splitthr);
  ShowWarn->setChecked(m_Opts->m_showwarn);
  ChipLabelBox->setChecked((bool)m_Opts->m_showchipid);
  showAxisLabels->setChecked(m_Opts->m_showAxisLabels);
  DoProjLogy->setChecked((bool)m_Opts->m_projlogy);
  HITcut->setValue((int) m_Opts->m_hitcut);
  TOTcut->setValue((int) m_Opts->m_totcut);
  NOISEcut->setValue((int) m_Opts->m_noisecut);
  ShowAllUnconn->setChecked(m_Opts->m_showcutpix);
  DIFFcut->setValue((int) m_Opts->m_diffcut);
  XTFRcut->setValue(m_Opts->m_xtfrcut);
  XTNOISEcut->setValue(m_Opts->m_xtnoisecut);
  NOISEtab->setChecked((bool)m_Opts->m_noisetab);
  TabZeroBox->setChecked((bool)m_Opts->m_zerotab);
  LongBox->setChecked((bool)m_Opts->m_masklong);
  GangedBox->setChecked((bool)m_Opts->m_maskgang);
  Qt0TW->setValue(m_Opts->m_QhighTW);
  ChipTarget->setValue((int)m_Opts->m_ChipTarget);
  doChipTarget->setChecked((bool)m_Opts->m_doChipTarget);
  doFixedTarget->setChecked((bool)m_Opts->m_doFixedTarget);
  rec2DFit->setChecked(m_Opts->m_rec2DFit);
  DACdoFit->setChecked(m_Opts->m_DACdoFit);
  FitGauss->setChecked((bool)m_Opts->m_FitGauss);
  ToggleMask->setChecked((bool)m_Opts->m_ToggleMask);
  ToTPlotPnt->setValue(m_Opts->m_ToTPlotPnt);
  fixedTOTSwitch->setChecked(m_Opts->m_fixedToTSwitch);
  TOTSwitchPnt->setValue(m_Opts->m_ToTSwitch);
  TOTIgnore->setValue(m_Opts->m_QPlotPnt);
  UsrMin->setValue(m_Opts->m_UsrMin);
  UsrMax->setValue(m_Opts->m_UsrMax);
  UserRange->setChecked(m_Opts->m_doUsrRg);
  TotRelcal->setChecked(m_Opts->m_doTotRelcal);
  runCombToTfit->setChecked(m_Opts->m_runCombToTfit);
  runBadFitsIntime->setChecked(m_Opts->m_runBadFitsIntime);
  runBadFitsTzero->setChecked(m_Opts->m_runBadFitsTzero );
  runBadFitsThresh->setChecked(m_Opts->m_runBadFitsThresh);
  runBadFitsXtalk->setChecked(m_Opts->m_runBadFitsXtalk );
  MinToT->setValue(m_Opts->m_minToT);
  minHits->setValue(m_Opts->m_peakMinHit);
  doBlackBg->setChecked(m_Opts->m_doBlackBg);
  srcMin->setValue((int)m_Opts->m_srcMin);
  srcRestrict->setChecked(m_Opts->m_srcRestrict);
  srcRecalcPeak->setChecked(m_Opts->m_srcRecalcPeak);
  cgfUserdefNames->setChecked(m_Opts->m_cfgUsrdefNames);
  if(m_Opts->m_chiCutmT>0){
    chiCut_MinThr->setValue(m_Opts->m_chiCutmT);
    chiCutEn->setChecked(true);
  }else{
    chiCut_MinThr->setValue(0);
    chiCutEn->setChecked(false);
  }
  if(m_Opts->m_nseCutmT>0){
    nseCut_MinThr->setValue(m_Opts->m_nseCutmT);
    nseCut_MinThr_long->setValue(m_Opts->m_nseCutmT_long);
    nseCut_MinThr_ganged->setValue(m_Opts->m_nseCutmT_ganged);
    nseCut_MinThr_inter->setValue(m_Opts->m_nseCutmT_inter);
    nseCutEn->setChecked(true);
  }else{
    nseCut_MinThr->setValue(0);
    nseCut_MinThr_long->setValue(0);
    nseCut_MinThr_ganged->setValue(0);
    nseCut_MinThr_inter->setValue(0);
    nseCutEn->setChecked(false);
  }

  DataPath->setText(m_Opts->m_DefDataPath.c_str());
  GenFromMod->setChecked(m_Opts->m_UseMname);
  CutsPath->setText(m_Opts->m_DefCutPaths.c_str());
  MacrosPath->setText(m_Opts->m_DefMacroPath.c_str());
  showChipCuts->setChecked(m_Opts->m_showChipCuts);
  PDBPath->setText(m_Opts->m_pdbpath.c_str());
  storePlots->setChecked(m_Opts->m_pdbPlots);
  removePDBFiles->setChecked(m_Opts->m_pdbRemove);
  removeDataPDB->setChecked(m_Opts->m_pdbClear);
  storeROOT->setChecked(m_Opts->m_CutCreateRoot);
  copyWeb->setChecked(m_Opts->m_CutWebCopy);
  webServer->setText(m_Opts->m_CutWebPath.c_str());
  webUser->setText(m_Opts->m_CutWebUser.c_str());
  webCommand->setText(m_Opts->m_CutWebCmd.c_str());
  VbiasMin->setValue(m_Opts->m_VbiasMin);
  VbiasMax->setValue(m_Opts->m_VbiasMax);
  IbiasCorr->setChecked(m_Opts->m_IbiasCorr);
  absIV->setChecked(m_Opts->m_absValIV);
  ambushStart->setValue(m_Opts->m_ambushStart);
  ambushStop ->setValue(m_Opts->m_ambushStop);
  userPDB->setText(m_Opts->m_userPDB.c_str());
    passwordPDB->setText(m_Opts->m_passwdPDB.c_str());
  webUploadMode->setCurrentItem(m_Opts->m_webUploadOpt);
  deltaT0->setValue(m_Opts->m_delT0_twalk);
  totcheckVar->setCurrentItem(m_Opts->m_totcheckVar);
  totcheckValue->setText(QString::number(m_Opts->m_totcheckValue,'f',2));
  totcheckKeep->setChecked(m_Opts->m_totcheckKeep);
  corEdit->setText(m_Opts->m_corPlotOpt.c_str());
  occHisto->setChecked(m_Opts->m_occPlotHisto);

  ColourComboBox->setCurrentItem(m_Opts->m_colourScale);
  if(m_Opts->m_plotfitopt){
    PlotFitBox->setCurrentItem(m_Opts->m_plotfitopt-1);
    SplitFitBox->setChecked(FALSE);
  } else
    SplitFitBox->setChecked(TRUE);
  Split_clicked(0);
  //voltage channels
  VDD_Supp->setValue(m_Opts->m_DDsupp+1);
  VDD_Chan->setValue(m_Opts->m_DDchan+1);
  VDDA_Supp->setValue(m_Opts->m_DDAsupp+1);
  VDDA_Chan->setValue(m_Opts->m_DDAchan+1);
  Vbias_Supp->setValue(m_Opts->m_BIASsupp+1);
  //  Vbias_Chan->setValue(m_Opts->m_BIASchan+1);
  // indiv. user ranges
  int i;
  for(i=0;i<RangeTable->numRows();i++){
    bool ok;
    int id = RangeTable->verticalHeader()->label(i).toInt(&ok);
    if(id>LOAD && id<DSET){
      RangeTable->setText(i,1,QString::number(m_Opts->m_UsrIMin[id],'f',3));
      RangeTable->setText(i,2,QString::number(m_Opts->m_UsrIMax[id],'f',3));
      RangeTable->setText(i,3,QString::number((int)m_Opts->m_UsrIUse[id]));
    }
  }
  for(i=0;i<(int)m_Opts->m_StdFname.size();i++)
    StdTable->setText(i,1,m_Opts->m_StdFname[i].c_str());

  for(i=0;i<PixLibRangeTable->numRows();i++){
    int id = PixLibRangeTable->verticalHeader()->label(i).toInt();
    if(id>=0 && id<NPLHIMAX && m_Opts->m_PLhistoExists[id]){
      PixLibRangeTable->setText(i,1,QString::number(m_Opts->m_PLUsrIMin[id],'f',3));
      PixLibRangeTable->setText(i,2,QString::number(m_Opts->m_PLUsrIMax[id],'f',3));
      PixLibRangeTable->setText(i,3,QString::number((int)m_Opts->m_PLUsrIUse[id]));
      PixLibRangeTable->setText(i,5,QString::number((int)m_Opts->m_PLPltZero[id]));
      Q3ComboTableItem *comboItem = dynamic_cast<Q3ComboTableItem *>(PixLibRangeTable->item(i,4));
      if(comboItem!=0)
	comboItem->setCurrentItem(m_Opts->m_PLplotType[id]);
    }
  }

  return;
}
void OptWin::Split_clicked(int){
  PlotFitBox->setEnabled(!SplitFitBox->isChecked());
}

void OptWin::FT_clicked(int){
  doChipTarget->setEnabled(!doFixedTarget->isChecked());
}
void OptWin::FixSwitch_clicked(){
  TOTSwitchPnt->setEnabled(fixedTOTSwitch->isChecked());
  SwitchLabel->setEnabled(fixedTOTSwitch->isChecked());
}
PathItem::PathItem(QLineEdit* fname, QLabel* label, QPushButton* button) : QWidget(){
  m_fname  = fname;
  m_label  = label;
  m_button = button;
  // signals and slots connections
  connect( m_button, SIGNAL( clicked() ), this, SLOT( GetFile() ) );
}

PathItem::~PathItem(){
}
void PathItem::GetFile(){
  QString path=m_fname->text();
  path = Q3FileDialog::getExistingDirectory(path,this,"MAFQAREp",
					   "select output dir",FALSE);
  if(!path.isEmpty())
    m_fname->setText(path);
  return;
}
