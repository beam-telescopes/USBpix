#include "DataViewer.h"
#include "ScanItem.h"
#include "PlotFactory.h"
#include "OptionsPanel.h"
#include "ParFitfun.h"
#include "RefWin.h"
#include "MaskMaker.h"
#include "FitWin.h"
#include "FitStatus.h"

#include <PixDcs/SleepWrapped.h>
#include <PixConfDBInterface.h>
#include <Config/Config.h>
#include <DataContainer/PixDBData.h>
#include <Fitting/FitClass.h>
#include <GeneralDBfunctions.h>
#include <DBTreeItem.h>
#include <DBEdtEngine.h>

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QTreeView>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>

  
DataViewer::DataViewer(QWidget * parent, Qt::WindowFlags flags , bool waferView)
  : QWidget(parent, flags), m_waferView(waferView), m_file(0)
{
  setupUi(this);
  
  if(flags==Qt::Widget){ // embedded window
    browseButton->hide();
    updateButton->hide();
    fileName->setEnabled(false);
    fileLabel->hide();
    closeButton->hide();
  } else{
    mainLabel->hide();
    setWindowTitle(m_waferView?"Wafer Viewer":"Data Viewer");
  }
  
  closeFile();
  if(m_waferView) m_treeView->headerItem()->setText(0, QString("List of  Items"));
  
  m_pfact = new PlotFactory(this);
  // load options if according file exists
  QString file = QDir::homePath() + "/.dataviewer.root";
  FILE *ftest = fopen(file.toLatin1().data(),"r");
  if(ftest!=0){
    fclose(ftest);
    PixLib::PixConfDBInterface *optFile = DBEdtEngine::openFile(file.toLatin1().data());
    PixLib::DBInquire *root = optFile->readRootRecord(1);
    m_pfact->config().read(root);
    delete optFile; optFile=0;
  }
  optionsPanel *opt = new optionsPanel(m_pfact->config(), m_tabWidget, Qt::Widget, false, true, true, false);
  optVerticalLayout->addWidget(opt);
  m_lastAction = 0;
  m_lastQactID = -1;
  if(m_pfact->getChipMode() || m_waferView) ChipOrMod_2->setChecked(true);
  setChipToPlot();

  m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_lastPdbFID = -1;
  m_lastPdbFPars.clear();

  QObject::connect(closeButton, SIGNAL(clicked()),   this, SLOT(close()));
  QObject::connect(browseButton, SIGNAL(clicked()),  this, SLOT(browseFile()));
  QObject::connect(updateButton, SIGNAL(clicked()),  this, SLOT(updateView()));
  QObject::connect(saveOptButton, SIGNAL(clicked()),  this, SLOT(saveOpt()));
  QObject::connect(m_treeView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),  this, SLOT(processDoubleClick(QTreeWidgetItem*, int)));
  QObject::connect(m_treeView, SIGNAL(customContextMenuRequested (const QPoint &)),  this, SLOT(processRightClick(const QPoint &)));
  QObject::connect(ChipOrMod, SIGNAL(toggled(bool)), this, SLOT(setChipToPlot(bool)));
  QObject::connect(ChipSel, SIGNAL(valueChanged(int)), this, SLOT(setChipToPlot()));
  QObject::connect(pixScanLevel, SIGNAL(valueChanged(int)), this, SLOT(setScanptBoxes(int)));

}
DataViewer::~DataViewer(){
  delete m_pfact;
}

void DataViewer::browseFile(const char *fname){
  QString qpath = fileName->text();
  if(qpath=="" && m_pfact->getDataPath()!=".")
     qpath = m_pfact->getDataPath().c_str();
  if(fname!=0){
    fileName->setText(fname);
    updateView();
  } else{
    QStringList filter;
    filter += "RootDB data file (*.root)";
    filter += "Any file (*.*)";
    QFileDialog fdia(this, "Specify name of data file", qpath);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::ExistingFile);
    if(fdia.exec() == QDialog::Accepted){
      qpath = fdia.selectedFiles().at(0);
      qpath.replace("\\", "/");
      fileName->setText(qpath);
      updateView();
    }
  }
  return;
}
void DataViewer::updateView(){
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString name = fileName->text();
  closeFile();
  fileName->setText(name); // restore name since this is erased after closing
  // check if file exists, only proceed if yes
  FILE *ft = fopen(fileName->text().toLatin1().data(), "r");
  if(ft==0){
    //fileName->setText("");
    QApplication::restoreOverrideCursor();
    return;
  }else
    fclose(ft);
  // then open as DB file
  try{
    m_file = DBEdtEngine::openFile(fileName->text().toLatin1().data());
  }catch(...){
    m_file = 0;
    fileName->setText("");
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,"DataView::updateView","Error opening file "+name);
    return;
  }
  PixLib::DBInquire *root = m_file->readRootRecord(1);
  for(PixLib::recordIterator rit=root->recordBegin(); 
      rit!=root->recordEnd();rit++){
    if((*rit)->getName()=="PixScanResult"){
      //      ScanItem *newItem;
      if(m_waferView)
	new ScanItem(m_scans, (*rit), fileName->text().toLatin1().data());
      else
	new ScanItem(m_treeView, (*rit), fileName->text().toLatin1().data());
    } else if((*rit)->getName()=="ChipTestResult" && m_waferView){
      DBTreeItem *newItem = new DBTreeItem(m_chipTests, (*rit), fileName->text().toLatin1().data(), false, true);
      // rename to test label
      std::string testName = (*rit)->getDecName();
      PixLib::getDecNameCore(testName);
      bool allPass=true, mixPass=false;
      for(PixLib::recordIterator riit=(*rit)->recordBegin(); 
	  riit!=(*rit)->recordEnd();riit++){
	if((*riit)->getName()=="PixModuleGroup"){
	  for(PixLib::recordIterator riiit=(*riit)->recordBegin(); 
	      riiit!=(*riit)->recordEnd();riiit++){
	    fieldIterator f;
	    f = (*riiit)->findField("PassFlag");
	    if(f!=(*riiit)->fieldEnd()){
	      bool value;
	      m_file->DBProcess(*f,READ,value);
	      if(value){
		if(!allPass) mixPass=true;
	      } else
		allPass = false;
	    }
	    f = (*riiit)->findField("Result");
	    if(f!=(*riiit)->fieldEnd()){
	      std::string value;
	      m_file->DBProcess(*f,READ,value);
	      int type = DBTreeItem::none;
	      if(value.substr(0,14)=="Passed GR test" || value.substr(0,14)=="Failed GR test")
		type = DBTreeItem::GR;
	      else if(value.substr(0,23)=="*** FE 0\nPassed GR test" || value.substr(0,23)=="*** FE 0\nFailed GR test")
		type = DBTreeItem::GR+DBTreeItem::DCdata;
	      else if(value.substr(0,25)=="Passed test of scan chain" || value.substr(0,25)=="Failed test of scan chain")
		type = DBTreeItem::SC;
	      else if(value.substr(0,34)=="*** FE 0\nPassed test of scan chain" || value.substr(0,34)=="*** FE 0\nFailed test of scan chain")
		type = DBTreeItem::SC+DBTreeItem::DCdata;
	      else if(value.substr(0,9)=="*** FE 0\n")
		type = DBTreeItem::PR+DBTreeItem::DCdata;
	      else
		type = DBTreeItem::PR;
	      DBTreeItem *newMod = new DBTreeItem(newItem, (*f), fileName->text().toLatin1().data(), (DBTreeItem::ChipTestType)type, true);
	      // rename to module + group label
	      std::string mlabel, decName = (*riit)->getDecName();
	      PixLib::getDecNameCore(decName);
	      mlabel = "Result for " + decName;
	      decName = (*riiit)->getDecName();
	      PixLib::getDecNameCore(decName);
	      mlabel += " - " +decName;
	      newMod->setText(0,mlabel.c_str());
	    }
	  }
	}
      }
      if(allPass)
	testName += " - ALL PASSED";
      else if(mixPass)
	testName += " - some FAILED";
      else
	testName += " - ALL FAILED";
      newItem->setText(0,testName.c_str());
    } else if((*rit)->getName()=="DCS_readings" && m_waferView){
      DBTreeItem *newItem = new DBTreeItem(m_dcsReadings, (*rit), fileName->text().toLatin1().data(), true, true);
      // rename to reading label
      std::string decName = (*rit)->getDecName();
      PixLib::getDecNameCore(decName);
      newItem->setText(0,decName.c_str());
    } else if((*rit)->getName()=="ServiceRecord" && m_waferView){
      DBTreeItem *newItem = new DBTreeItem(m_srvRecs, (*rit), fileName->text().toLatin1().data(), false, true);
      // rename to reading label
      std::string decName = (*rit)->getDecName();
      PixLib::getDecNameCore(decName);
      newItem->setText(0,decName.c_str());
      for(PixLib::recordIterator riit=(*rit)->recordBegin(); 
	  riit!=(*rit)->recordEnd();riit++){
	if((*riit)->getName()=="PixModuleGroup"){
	  for(PixLib::recordIterator riiit=(*riit)->recordBegin(); 
	      riiit!=(*riit)->recordEnd();riiit++){
	    fieldIterator f = (*riiit)->findField("Result");
	    if(f!=(*riiit)->fieldEnd()){
	      DBTreeItem *newMod = new DBTreeItem(newItem, (*f), fileName->text().toLatin1().data(), DBTreeItem::none, true);
	      // rename to module + group label
	      std::string mlabel, decName = (*riit)->getDecName();
	      PixLib::getDecNameCore(decName);
	      mlabel = "Result for " + decName;
	      decName = (*riiit)->getDecName();
	      PixLib::getDecNameCore(decName);
	      mlabel += " - " +decName;
	      newMod->setText(0,mlabel.c_str());
	    }
	  }
	}
      }
    } else if((*rit)->getName()=="GenericADCvalue" && m_waferView){
      DBTreeItem *newItem = new DBTreeItem(m_gadcVals, (*rit), fileName->text().toLatin1().data(), false, true);
      // rename to reading label
      std::string decName = (*rit)->getDecName();
      PixLib::getDecNameCore(decName);
      newItem->setText(0,decName.c_str());
      for(PixLib::recordIterator riit=(*rit)->recordBegin(); 
	  riit!=(*rit)->recordEnd();riit++){
	if((*riit)->getName()=="PixModuleGroup"){
	  for(PixLib::recordIterator riiit=(*riit)->recordBegin(); 
	      riiit!=(*riit)->recordEnd();riiit++){
	    fieldIterator f = (*riiit)->findField("Result");
	    if(f!=(*riiit)->fieldEnd()){
	      DBTreeItem *newMod = new DBTreeItem(newItem, (*f), fileName->text().toLatin1().data(), DBTreeItem::none, true);
	      // rename to module + group label
	      std::string mlabel, decName = (*riit)->getDecName();
	      PixLib::getDecNameCore(decName);
	      mlabel = "Result for " + decName;
	      decName = (*riiit)->getDecName();
	      PixLib::getDecNameCore(decName);
	      mlabel += " - " +decName;
	      newMod->setText(0,mlabel.c_str());
	    }
	  }
	}
      }
    }
  }
  m_treeView->scrollToBottom();
  QApplication::restoreOverrideCursor();

}
void DataViewer::closeFile(){
  m_treeView->clear();
  m_treeView->setSortingEnabled(false);
  if(m_waferView){
    m_scans = new QTreeWidgetItem(m_treeView, QStringList("Scans"));
    m_chipTests = new QTreeWidgetItem(m_treeView, QStringList("Chip Tests"));
    m_dcsReadings = new QTreeWidgetItem(m_treeView, QStringList("DCS Readings"));
    m_srvRecs = new QTreeWidgetItem(m_treeView, QStringList("Service Records"));
    m_gadcVals = new QTreeWidgetItem(m_treeView, QStringList("Generic ADC readings"));
  } else{
    m_scans = 0;
    m_chipTests = 0;
    m_dcsReadings = 0;
    m_srvRecs = 0;
    m_gadcVals = 0;
  }
  fileName->setText("");
  delete m_file; m_file = 0;

}
void DataViewer::processRightClick(const QPoint &){
  ModuleAction *mac = dynamic_cast<ModuleAction*>(m_treeView->currentItem());
  if(mac!=0 && mac->isHistoPlot()){
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
    int sclvl = mac->getData().getScanLevel();
    int spnts = mac->getData().getScanSteps(pixScanLevel->value());
    int nsel = m_treeView->selectedItems().count();

    QAction *act;
    QMenu modmenu(this);
    act = modmenu.addAction("Plot vs scan par. on selected level", this, SLOT(plotScanPlain()));
    act->setEnabled((pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
    act = modmenu.addAction("Plot vs scan par. with fit function", this, SLOT(plotScanFit()));
    act->setEnabled((pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
    act = modmenu.addAction("Plot avg./spread vs scan par. on selected level", this, SLOT(plotScanAvg()));
    act->setEnabled((pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
    modmenu.addSeparator();
    act = modmenu.addAction("Fit as fct. of scan par. on selected level", this, SLOT(fitScan()));
    act->setEnabled((pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
    modmenu.addSeparator();
    act = modmenu.addAction("Plot ratio/diff./... between two scan points", this, SLOT(plotOperScan()));
    act->setEnabled((pixScanLevel->value()<sclvl && pixScanLevel->value()>=0 && 
		     Scanpt0->value()<spnts && Scanpt0->value()>=0));
    act = modmenu.addAction("Plot ratio/diff./... between selected items", this, SLOT(plotOperItems()));
    act->setEnabled(nsel==2);
    act = modmenu.addAction("Plot correlation between selected items", this, SLOT(plotCorrel()));
    act->setEnabled(nsel==2);
    modmenu.addSeparator();
    act = modmenu.addAction("Generate mask from histogram", this, SLOT(genMaskPlain()));
    act->setEnabled(true);
    act = modmenu.addAction("Generate mask from ratio/diff./... between selected items", this, SLOT(genMaskOper()));
    act->setEnabled(nsel==2);
    modmenu.exec(QCursor::pos());
  }
}
void DataViewer::plotScan(bool doFit){
  m_lastQactID = doFit?2:1;
  m_pfact->ClearMemory();
  int scanpts[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
  int fid = -1;
  std::vector<int> pars;
  ModuleAction *mac = dynamic_cast<ModuleAction*>(m_treeView->currentItem());
  if(mac!=0 && mac->isHistoPlot()){
    m_lastAction = mac;
    if(doFit){
      if(m_lastPdbFPars.size()==0){
	ParFitfun pff(mac->getData(), this);
	if(pff.exec()!=QDialog::Accepted) return;
	fid = pff.getPars(pars);
	m_lastPdbFPars = pars;
	m_lastPdbFID = fid;
      }else{
	pars = m_lastPdbFPars;
	fid = m_lastPdbFID;
      }
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int retVal = m_pfact->plotValVsPar(mac->getData(), mac->getType(), pixScanLevel->value(), scanpts,  m_plotChip, 
				       ChipSel->value(), ColSel->value(), RowSel->value(), pars, fid);
    QApplication::restoreOverrideCursor();
    if(retVal<0) QMessageBox::warning(this,"DataView plot action","Error while plotting; code "+QString::number(retVal));
  }
}
void DataViewer::plotScanAvg(){
  m_lastQactID = 3;
  m_pfact->ClearMemory();
  int scanpts[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
  ModuleAction *mac = dynamic_cast<ModuleAction*>(m_treeView->currentItem());
  if(mac!=0 && mac->isHistoPlot()){
    m_lastAction = mac;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int retVal = m_pfact->plotAvgVsPar(mac->getData(), mac->getType(), pixScanLevel->value(), scanpts,  m_plotChip);
    QApplication::restoreOverrideCursor();
    if(retVal<0) QMessageBox::warning(this,"DataView plot action","Error while plotting; code "+QString::number(retVal));
  }
}
void DataViewer::plotOper(bool isScan){
  m_lastQactID = 0;
  m_pfact->ClearMemory();
  ModuleAction *mac1, *mac2;
  mac1 = dynamic_cast<ModuleAction*>(m_treeView->selectedItems()[0]);
  if(isScan) mac2 = mac1;
  else
    mac2 = dynamic_cast<ModuleAction*>(m_treeView->selectedItems()[1]);
  if(mac1!=0 && mac2!=0){
    int scanpts1[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
    int scanpts2[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
    int spnts = mac1->getData().getScanSteps(pixScanLevel->value());
    m_lastAction = mac1;
    RefWin rw(this);
    std::map<std::string, int> pdb_oper = mac1->getData().getOperTypes();
    for(std::map<std::string, int>::iterator IT=pdb_oper.begin(); IT!=pdb_oper.end();IT++)
      rw.addToList(IT->first.c_str(),IT->second);
    rw.setWindowTitle("Options for plotting");
    if(!isScan){
      rw.SelectorLabel->hide();
      rw.DatSel->hide();
    } else {
      rw.SelectorLabel->setText("Select 2nd scan point (1st is "+QString::number(Scanpt0->value())+")");
      for(int is=0;is<spnts;is++)
	if(is!=scanpts1[pixScanLevel->value()]) rw.DatSel->addItem(QString::number(is), QVariant(is));
    }
    rw.OKButt->setEnabled(true);
    if(rw.exec()==QDialog::Accepted){
      if(isScan) scanpts2[pixScanLevel->value()] = rw.DatSel->currentText().toInt();
      int retVal = m_pfact->PlotMapProjScat(mac1->getData(), mac2->getData(), m_plotChip, scanpts1, pixScanLevel->value(), mac1->getType(),
					    scanpts2, mac2->getType(), rw.getSelVal());
      if(retVal<0) QMessageBox::warning(this,"DataView plot action","Error while plotting; code "+QString::number(retVal));
    }
  }
}

void DataViewer::plotCorrel(){
  m_lastQactID = 0;
  m_pfact->ClearMemory();
  ModuleAction *mac1, *mac2;
  mac1 = dynamic_cast<ModuleAction*>(m_treeView->selectedItems()[0]);
  mac2 = dynamic_cast<ModuleAction*>(m_treeView->selectedItems()[1]);
  if(mac1!=0 && mac2!=0){
    int scanpts[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
    int retVal = m_pfact->PlotCorrel(mac1->getData(), mac2->getData(), m_plotChip, scanpts, pixScanLevel->value(), mac1->getType(),
				     mac2->getType());
    if(retVal<0) QMessageBox::warning(this,"DataView plot action","Error while plotting; code "+QString::number(retVal));
  }
}

void DataViewer::fitScan(){
  m_lastQactID = 0;
  m_pfact->ClearMemory();
  //int scanpts[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
  ModuleAction *mac=dynamic_cast<ModuleAction*>(m_treeView->currentItem());
  if(mac!=0 && mac->isHistoPlot()){
    FitClass &fc = m_pfact->getFitClass();
    bool reload = false, saveData=false;
    QString tmptxt;
    
    FitWin *fw = new FitWin(mac->getData(), fc, this, 0, mac->getType(), pixScanLevel->value());
    
    if(fw->exec()==QDialog::Accepted){
      QString tmptxt;
      int npar = fc.getNPar(fw->FitType->currentIndex());
      float *pars;
      bool *pfix;
      int *phis;
      pars = new float[npar];
      pfix = new bool[npar];
      phis = new int[npar];
      for(int i=0;i<npar; i++){
	if(fw->guessParBox->isChecked()){
	  pars[i] = 0.;
	  pfix[i] = fw->m_parFixed[i]->isChecked();
	} else{
	  pars[i] = (float)fw->m_parValues[i]->value();
	  pfix[i] = fw->m_parFixed[i]->isChecked();
	}
      }
      PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType)mac->getType();
      PixLib::PixScan::HistogramType errType = PixLib::PixScan::MAX_HISTO_TYPES;
      if(fw->fromHistoBox->isChecked()){
	PixLib::PixScan ps;
	errType = (PixLib::PixScan::HistogramType)(ps.getHistoTypes()[fw->errorCBox->currentText().toLatin1().data()]);
      }

      // open dialog for save fit results into RootDB later on
      ParFitfun *pff = new ParFitfun(mac->getData(), this);
      pff->setWindowTitle("Saving parameters to RootDB file");
      pff->funcBox->setCurrentIndex(fw->FitType->currentIndex());
      pff->funcBox->setEnabled(false);
      pff->genHistoBoxes(fw->FitType->currentIndex(), false, true);
      
      
      std::vector<int> parhis;
      PixScan::HistogramType oldChiHi=PixScan::MAX_HISTO_TYPES;
      if(pff->exec()==QDialog::Accepted){
	saveData = true;
	pff->getPars(parhis, true);
	for(unsigned int k=0;k<parhis.size();k++){
	  if(k<(unsigned int)npar) phis[k] = parhis[k];
	}
	if(fw->refitBox->isChecked())
	  oldChiHi = (PixScan::HistogramType)parhis[parhis.size()-1]; // last argument is old chi2 histo
      }
//       tmptxt = "Initialising fit for data "+QString(mac->getData().GetName())+
// 	", module "+QString(mac->getData().GetModName());
//       printf("%s\n", tmptxt.toLatin1().data());
      mac->getData().initFit(type, pixScanLevel->value(), pars, pfix, phis, fw->FitType->currentIndex(),errType, oldChiHi, fw->vcalBox->isChecked());
      
      // determine no. of FEs via module config.
      int nFe=0;
      PixLib::Config &mcfg = mac->getData().getModConfig();
      for(int isc=0; isc<mcfg.subConfigSize(); isc++)
	if(mcfg.subConfig(isc).name().substr(0,5)=="PixFe") nFe++;

      QApplication::setOverrideCursor(Qt::WaitCursor);
      int minc = 0; 
      int maxc = nFe;
      if(m_plotChip>=0 && m_plotChip<nFe){
	minc = m_plotChip;
	maxc = minc+1;
      }
      bool noAbort=true;
      for(int chip=minc;chip<maxc && noAbort;chip++){
	QApplication::processEvents();
	float frErr = 0;
	if(fw->fractBox->isChecked())
	  frErr = fw->fractEdit->text().toFloat();
	FitThread fitthr(mac->getData(), fw->FitType->currentIndex(), chip,(float)fw->ChiCut->value(),(float)fw->FitXmin->value(),
		  (float)fw->FitXmax->value(),frErr,fw->divndfBox->isChecked());
	fitthr.start();
	QApplication::restoreOverrideCursor();
	QMessageBox waitwin(QMessageBox::Information, "wait", "wait for fit abort",
			     QMessageBox::NoButton, 0, Qt::Dialog);
	FitStatus abortwin(this);
	abortwin.show();
	tmptxt = "Fitting chip "+QString::number(chip)+" from data "+QString(mac->getData().GetName())+
	  ", module "+QString(mac->getData().GetModName());
	if(oldChiHi<PixScan::MAX_HISTO_TYPES)
	  tmptxt += " (bad fits only)";
	//	printf("%s\n", tmptxt.toLatin1().data());
	abortwin.logBrowser->append(tmptxt+"\n");
	QApplication::processEvents();
	while(fitthr.isRunning()){
	  if(!abortwin.isVisible() && noAbort){
	    mac->getData().abortFit(); // controlled abort, loop must wait till finished
	    noAbort = false;
	    waitwin.show();
	    saveData = (abortwin.result()==QDialog::Accepted);
	  }
	  int col, row;
	  mac->getData().getPix(col, row);
	  tmptxt = "Fitting chip "+QString::number(chip)+", col "+QString::number(col)+", row "+QString::number(row)+
	    " from data "+QString(mac->getData().GetName())+", module "+QString(mac->getData().GetModName());
	  if(oldChiHi<PixScan::MAX_HISTO_TYPES)
	    tmptxt += " (bad fits only)";
	  //printf("%s\n", tmptxt.toLatin1().data());
	  abortwin.logBrowser->append(tmptxt+"\n");
	  QApplication::processEvents();
	  PixLib::sleep(200);
	}
	QApplication::setOverrideCursor(Qt::WaitCursor);
      }
      tmptxt = noAbort?"Finished fit!":"Fit was aborted";
      //printf("%s\n", tmptxt.toLatin1().data());
      
      if(saveData){
	tmptxt = "Storing results";
	//printf("%s\n", tmptxt.toLatin1().data());
	int loop = pixScanLevel->value()+1;
	for(int k=0;k<(int)parhis.size();k++){
	  PixLib::PixScan::HistogramType htype = (PixLib::PixScan::HistogramType)parhis[k];
	  if(htype<PixScan::MAX_HISTO_TYPES)
	    mac->getData().writePixLibHisto(mac->getData().getParHisto(k), htype, 0, loop);
	}
	// store fit function name for later reference
	mac->getData().writeFitType(fw->FitType->currentIndex(), loop);
	// clear all, including histos
	mac->getData().clearFit(true);
	reload = true;
	tmptxt = "Done!";
	//printf("%s\n", tmptxt.toLatin1().data());
	QApplication::processEvents();
      }
      QApplication::restoreOverrideCursor();
      delete pff;
      delete[] pars;
      delete[] pfix;
      delete[] phis;
    }
    delete fw;

    if(reload){
      QTreeWidgetItem *item = mac;
      while(dynamic_cast<ModuleItem*>(item)==0){
	item = item->parent();
	if(item==0) break;
      }
      ModuleItem *mit = dynamic_cast<ModuleItem*>(item);
      if(mit!=0) mit->reLoadContent();
    }
  }

}
void DataViewer::generateMask(bool isOperType){
  m_lastQactID = 0;
  m_pfact->ClearMemory();
  ModuleAction *mac1=0, *mac2=0;
  mac1 = dynamic_cast<ModuleAction*>(m_treeView->selectedItems()[0]);
  int rwFunc = -1;
  if(isOperType){
    mac2 = dynamic_cast<ModuleAction*>(m_treeView->selectedItems()[1]);
    // ask user for type of operation
    RefWin rw(this);
    std::map<std::string, int> pdb_oper = mac1->getData().getOperTypes();
    for(std::map<std::string, int>::iterator IT=pdb_oper.begin(); IT!=pdb_oper.end();IT++)
      rw.addToList(IT->first.c_str(),IT->second);
    rw.setWindowTitle("Options for mask generation");
    rw.SelectorLabel->hide();
    rw.DatSel->hide();
    if(rw.exec()==QDialog::Accepted){
      rwFunc = rw.getSelVal();
    }
  }
  int ps_type = mac1->getType();
  MaskMaker mm(this);
  mm.setHistoLabel(ps_type);
  if(mm.exec()==QDialog::Accepted){
    QApplication::setOverrideCursor(Qt::WaitCursor);
    PixLib::Histo *h = 0;
    if(isOperType){
      int ps_type2 = mac2->getType();
      h = mac1->getData().GenMask((PixLib::PixScan::HistogramType)ps_type, &(mac2->getData()), (PixLib::PixScan::HistogramType)ps_type2, 
				  (PixDBData::OperType) rwFunc, Scanpt0->value(), pixScanLevel->value(), 
				  mm.getMin(),mm.getMax());
    }else{
      h = mac1->getData().GenMask((PixLib::PixScan::HistogramType)ps_type, Scanpt0->value(), pixScanLevel->value(), 
			      mm.getMin(),mm.getMax());
    }
    if(h!=0){
      // store mask in file and remove histo from memory
      mac1->getData().writePixLibHisto(h, PixLib::PixScan::HITOCC, 0,0);
      delete h;
      //re-read this item
      ModuleItem *mit = dynamic_cast<ModuleItem*>(mac1->parent());
      if(mit!=0) mit->reLoadContent();
      QApplication::restoreOverrideCursor();
    } else{
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"DataView plot action","Mask making didn't return valid Histo object");
    }
  }
}
void DataViewer::processDoubleClick(QTreeWidgetItem *item, int){
  m_lastQactID = 0;
  m_pfact->ClearMemory();
  ModuleAction *mac = dynamic_cast<ModuleAction*>(item);
  DBTreeItem *dti = dynamic_cast<DBTreeItem*>(item);
  if(mac!=0){
    int scanpts[3]={Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
    m_lastAction = mac;
    int retVal = 0;
    if(mac->getType() < (6+(int)PixLib::PixScan::MAX_HISTO_TYPES)){
      QApplication::setOverrideCursor(Qt::WaitCursor);
      retVal = m_pfact->GenericPlotting(mac->getData(), m_plotChip, scanpts, pixScanLevel->value(), mac->getType());
      QApplication::restoreOverrideCursor();
    }else{
      int nonHtype = mac->getType() - (int)PixLib::PixScan::MAX_HISTO_TYPES;
      switch(nonHtype){
      case 10:{ // show module config
	Config &cfg = mac->getData().getModConfig();
	optionsPanel op(cfg, 0, 0, true, false, true, true);
	op.setWindowTitle("Module Configuration");
	op.exec();
	break;}
      case 11:{ // show scan config
	Config &cfg = mac->getData().getScanConfig();
	optionsPanel op(cfg, 0, 0, true, false, true, true);
	op.setWindowTitle("Scan Configuration");
	op.exec();
	break;}
      default: // none
	retVal = -3;
      }
    }
    if(retVal<0) QMessageBox::warning(this,"DataView plot action","Error while plotting; code "+QString::number(retVal));
  } else if(dti!=0 && dti->isField())
    dti->edit();
}
void DataViewer::getHistoPixel(float &x, float &y){
  x = -1.;
  y = -1.;
  if(m_lastAction!=0){
    PixDBData &data = m_lastAction->getData();
    data.PixXY(ChipSel->value(), ColSel->value(), RowSel->value(), &x, &y);
  }
}
void DataViewer::processHistoClick(int x, int y, float cont, bool dblClick){
  if(m_lastAction!=0){
    PixDBData &data = m_lastAction->getData();
    int col, row, chip;
    data.PixXYtoCCR(x,y,&chip,&col,&row);
    ChipSel->setValue(chip);
    ColSel->setValue(col);
    RowSel->setValue(row);
    PixVal->setText(QString::number(cont, 'f',2));
    if(dblClick && m_lastQactID > 0){
      m_treeView->setCurrentItem(m_lastAction);
      switch(m_lastQactID){
      case 1:
	plotScan(false);
	break;
      case 2:
	plotScan(true);
	break;
      case 3:
	plotScanAvg();
	break;
      default:
	break;
      }
    }
  }
}
void DataViewer::setChipToPlot(){
  setChipToPlot(ChipOrMod->isChecked());
}
void DataViewer::setChipToPlot(bool modMode){
  if(modMode) m_plotChip = -1;
  else        m_plotChip = ChipSel->value();
}
void DataViewer::setScanptBoxes(int loop_level){
  Scanpt0->setEnabled(loop_level>=0 && loop_level<1);
  Scanpt1->setEnabled(loop_level>=0 && loop_level<2);
  Scanpt2->setEnabled(loop_level>=0 && loop_level<3);
}
void DataViewer::saveOpt(){
  QString file = QDir::homePath() + "/.dataviewer.root";
  PixLib::PixConfDBInterface *optFile = DBEdtEngine::openFile(file.toLatin1().data(), true);
  if(optFile==0){
    QMessageBox::warning(this,"DataView::saveOpt","Error opening file $HOME/.dataviewer");
    return;
  }
  PixLib::DBInquire *root = optFile->readRootRecord(1);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool retVal = m_pfact->config().write(root);
  QApplication::restoreOverrideCursor();
  if(!retVal) QMessageBox::warning(this,"DataView::saveOpt","Error saving DataViewer options.");
  delete optFile;

  return;
}
DataViewer::FitThread::FitThread(PixDBData &data, int fittype, int chip, float chicut, 
				float xmin, float xmax, float fracErr, bool useNdof) :
  m_data(data), m_fittype(fittype), m_chip(chip), m_chicut(chicut), m_xmin(xmin), m_xmax(xmax), 
  m_fracErr(fracErr), m_useNdof(useNdof){
}
void DataViewer::FitThread::run(){
  m_data.fitHisto(m_fittype, m_chip, m_chicut, m_xmin, m_xmax, m_fracErr, m_useNdof);
}
