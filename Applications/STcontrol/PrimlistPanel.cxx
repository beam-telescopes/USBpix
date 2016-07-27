#include "PrimlistPanel.h"
#include "STControlEngine.h"
#include "OptionsPanel.h"
#include "PrlItemInit.h"
#include "PixScanPanel.h"
#include "ChipTest.h"
#include <RenameWin.h>

#include <PixDcs/SleepWrapped.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixConfDBInterface/RootDB.h>
#include <PixController/PixScan.h>
#include <PixModule/PixModule.h>

#include <QString>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QVariant>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <string>
#include <sstream>
#ifdef WIN32
#include <stdlib.h>
#else
#include <unistd.h>
#endif

PrimListViewItem::PrimListViewItem( QTreeWidget* parent, PrimListTypes type, int index, const char *label, PixScan *in_ps)
  : QTreeWidgetItem(parent), PrimListItem(type, index, label, in_ps)
{
  setText(0, getLabel().c_str());
  setText(1, getTypeLabel(getType()).c_str());
  // needed to get correct sorting for different # of digits
  QString strgind;
  strgind.sprintf("%05d",getIndex());
  setText(2, strgind);

  m_tmpWidget = 0;
  m_copyItem = 0;
}
PrimListViewItem::PrimListViewItem( QTreeWidget* parent, DBInquire *dbi)
  : QTreeWidgetItem(parent), PrimListItem(dbi)
{
  setText(0, getLabel().c_str());
  setText(1, getTypeLabel(getType()).c_str());
  // needed to get correct sorting for different # of digits
  QString strgind;
  strgind.sprintf("%05d",getIndex());
  setText(2, strgind);

  m_tmpWidget = 0;
  m_copyItem = 0;
}
PrimListViewItem::PrimListViewItem( QTreeWidget* parent, PrimListViewItem &inItem, int index) : 
  QTreeWidgetItem(parent), PrimListItem((PrimListItem&)inItem, index){
  setText(0, getLabel().c_str());
  setText(1, getTypeLabel(getType()).c_str());
  // needed to get correct sorting for different # of digits
  QString strgind;
  strgind.sprintf("%05d",getIndex());
  setText(2, strgind);

  m_tmpWidget = 0;
  m_copyItem = 0;
}
PrimListViewItem::~PrimListViewItem()
{
  delete m_copyItem;
}
void PrimListViewItem::edit(bool isNew)
{
  if(isNew) m_copyItem = new PrimListItem(*this, getIndex());
  ((ConfString&)m_copyItem->config()["general"]["label"]).m_value = m_label; // otherwise, we get "Copy of ..."
  optionsPanel op(m_copyItem->config(), treeWidget(), 0, (getType()==CHIP_TEST));
  m_tmpWidget = &op;
  op.setWindowTitle(("Edit primlist item "+getLabel()).c_str());
  if( m_copyItem->getPS()!=0){
    QWidget *TabPage = new QWidget( (QWidget*)op.m_tabwidget);
    QVBoxLayout *layout1 = new QVBoxLayout( TabPage);
    QHBoxLayout *layout2 = new QHBoxLayout( 0 );
    QSpacerItem *spacer1 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer1 );
    QPushButton *b = new QPushButton("Edit PixScan config", TabPage);
    layout2->addWidget(b);
    QSpacerItem *spacer2 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer2 );
    layout1->addLayout(layout2);
    connect(b,SIGNAL(clicked()), this, SLOT(editPS()));
    op.m_tabwidget->addTab( TabPage,  "PixScan config");
  }
  // automatically handle new config. structure to be displayed when changing TOOL or CHIP_TEST sub-type
  if(op.m_handles["specific_toolType"]!=0) 
    connect(((QWidget*)op.m_handles["specific_toolType"]), SIGNAL(activated(int)), this, SLOT(toolTypeChanged()));
  if(op.m_handles["specific_chipTestType"]!=0) 
    connect(((QWidget*)op.m_handles["specific_chipTestType"]), SIGNAL(activated(int)), this, SLOT(toolTypeChanged()));
  // select specific tab, was started from there
  if(!isNew){
    QWidget *tab = op.m_tabwidget->findChild<QWidget*>("TabPageFillBox_specific");
    if(tab!=0) op.m_tabwidget->setCurrentWidget(tab);
  }
  // connect save/cancel functions
  connect(&op, SIGNAL(accepted()), this, SLOT(saveEdit()));
  connect(&op, SIGNAL(rejected()), this, SLOT(cleanEdit()));
  op.exec();
}
void PrimListViewItem::toolTypeChanged(){
  disconnect(m_tmpWidget, SIGNAL(accepted()), this, SLOT(saveEdit()));
  ((optionsPanel*)m_tmpWidget)->save();
  m_copyItem->setupSubcfg(0);
  //open new panel
  edit(false);
}
void PrimListViewItem::editPS()
{
  emit editThisPS(m_copyItem->getPS(), m_tmpWidget);
}
void PrimListViewItem::saveEdit(){
  if(m_copyItem->getSubType()>=0 && m_copyItem->getSubType()!=getSubType()){ 
    // sub-type for a TOOL or CHIP_TEST changed, make sure list of arguments is updated
    (*m_config) = m_copyItem->config();
    setupSubcfg(0);
  }
  (*m_config) = m_copyItem->config();
  if(m_ps!=0 && m_copyItem->getPS()!=0) m_ps->config() = m_copyItem->getPS()->config();
  cleanEdit();
}
void PrimListViewItem::cleanEdit(){
  setText(0, getLabel().c_str());
  delete m_copyItem; m_copyItem=0;
}
void PrimListViewItem::up()
{
  PrimListViewItem *prli_prev = (PrimListViewItem *)treeWidget()->itemAbove(this);
  if(prli_prev!=0){
    // increment index of previous item
    prli_prev->setIndex(prli_prev->getIndex()+1);
    // needed to get correct sorting for different # of digits
    QString strgind;
    strgind.sprintf("%05d",prli_prev->getIndex());
    prli_prev->setText(2, strgind);
    // decrement index of this item
    setIndex(getIndex()-1);
    // needed to get correct sorting for different # of digits
    strgind.sprintf("%05d",getIndex());
    setText(2, strgind);
  }
}
void PrimListViewItem::down()
{
  PrimListViewItem *prli_next = (PrimListViewItem *)treeWidget()->itemBelow(this);
  if(prli_next!=0){
    // decrement index of next item
    prli_next->setIndex(prli_next->getIndex()-1);
    // needed to get correct sorting for different # of digits
    QString strgind;
    strgind.sprintf("%05d",prli_next->getIndex());
    prli_next->setText(2, strgind);
    // increment index of this item
    setIndex(getIndex()+1);
    // needed to get correct sorting for different # of digits
    strgind.sprintf("%05d",getIndex());
    setText(2, strgind);
  }
}
PrimlistPanel::PrimlistPanel( STControlEngine &engine_in, QWidget* parent , bool extCtrl)
  : QWidget(parent), m_engine(engine_in), m_extCtrl(extCtrl)
{
  setupUi(this);
  QObject::connect(LoadPrlButton, SIGNAL(clicked()), this, SLOT(loadPrl()));
  QObject::connect(MergePrlButton, SIGNAL(clicked()), this, SLOT(mergePrl()));
  QObject::connect(SavePrlButton, SIGNAL(clicked()), this, SLOT(savePrl()));
  if(!m_extCtrl) QObject::connect(runButton, SIGNAL(clicked()), this, SLOT(startPrl()));
  QObject::connect(ClearPrlButton, SIGNAL(clicked()), this, SLOT(clearPrl()));
  QObject::connect(newItemButton, SIGNAL(clicked()), this, SLOT(addItem()));
  QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(browseOutput()));
  QObject::connect(prmlistView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),  this, SLOT(editItem(QTreeWidgetItem*, int)));
  QObject::connect(delItemButton, SIGNAL(clicked()), this, SLOT(deleteCurrItem()));
  QObject::connect(up100ItemButton, SIGNAL(clicked()), this, SLOT(up100CurrItem()));
  QObject::connect(up10ItemButton, SIGNAL(clicked()), this, SLOT(up10CurrItem()));
  QObject::connect(upItemButton, SIGNAL(clicked()), this, SLOT(upCurrItem()));
  QObject::connect(downItemButton, SIGNAL(clicked()), this, SLOT(downCurrItem()));
  QObject::connect(down10ItemButton, SIGNAL(clicked()), this, SLOT(down10CurrItem()));
  QObject::connect(down100ItemButton, SIGNAL(clicked()), this, SLOT(down100CurrItem()));
  QObject::connect(copyButton, SIGNAL(clicked()), this, SLOT(copyItem()));
  QObject::connect(pasteButton, SIGNAL(clicked()), this, SLOT(pasteItem()));
  QObject::connect(modtestMode, SIGNAL(toggled(bool)), this, SLOT(modTestMode(bool)));
  if(!m_extCtrl) QObject::connect(this, SIGNAL(changeStartState(bool)), runButton, SLOT(setEnabled(bool)));
  QObject::connect(burninBox, SIGNAL(toggled(bool)), this, SLOT(setBirep(bool)));
  QObject::connect(&(m_engine), SIGNAL(beganScanning()),    this, SLOT(disableStart()));
  QObject::connect(&(m_engine), SIGNAL(finishedScanning()), this, SLOT(finishedScan()));
  QObject::connect(&(m_engine), SIGNAL(prlScanStarted(int)),this, SLOT(freezePanel()) );
  QObject::connect(&(m_engine), SIGNAL(prlDone()),          this, SLOT(finishedPrl()) );

  m_currItem = 0;
  m_copyItem = 0;
  m_iiter = 0;
  m_lastProcInd = -1;

  prmlistView->setColumnWidth(0, 300);
  prmlistView->setColumnWidth(1, 150);
  prmlistView->sortItems(2,Qt::AscendingOrder);
  prmlistView->setSelectionMode(QAbstractItemView::SingleSelection);
}
PrimlistPanel::~PrimlistPanel()
{
  clearPrl();
}
void PrimlistPanel::mergePrl(){
  loadPrl(0,mergeSpinBox->value());
}
void PrimlistPanel::loadPrl(const char *extFile, int index)
{
  QString fname="";
  if(extFile!=0){
    fname = extFile;
  }else{
    Config &opts = m_engine.getOptions();
    std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
    QString qpath = QString::null;
    if(defPath!=".") qpath = defPath.c_str();
    QStringList filter;
    filter += "DB ROOT primlist file (*.prl.root)";
    filter += "Any file (*.*)";
    QFileDialog fdia(this, "Specify name of DB file with primlist entries", qpath);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::ExistingFile);

    if(fdia.exec() == QDialog::Accepted){
      fname = fdia.selectedFiles().at(0);
      fname.replace("\\", "/");
    }
  }
  if(fname=="") return;

  if(fname.right(8)!="prl.root"){
    m_engine.toErrLog(("PrimlistPanel::loadPrl : file extension "+fname.right(8)+
		       " unknown, can't load primitive list.").toLatin1().data());
    return;
  }
  PixLib::PixConfDBInterface *myDB = new RootDB(fname.toLatin1().data());
  if(myDB==0){
    m_engine.toErrLog(("PrimlistPanel::loadPrl : error opening file "+fname).toLatin1().data());
    return;
  }
  
  PixLib::DBInquire *root = 0;
  try{
    root = myDB->readRootRecord(1);
  } catch(...){    
    delete myDB;
    m_engine.toErrLog(("PrimlistPanel::loadPrl : error reading root record in file "+fname).toLatin1().data());
    return;
  }
  
  const int tmp_offs=100000; // list is set up for 5-digit index, so this should be safe
  if(index<0) clearPrl();
  else{ // add temp offset of 1000 to all items from index on
    for(int i=0;i<prmlistView->topLevelItemCount();i++){
      PrimListViewItem *lvItem = dynamic_cast<PrimListViewItem*>(prmlistView->topLevelItem(i));
      if(lvItem!=0){
	// adjust index if needed
	int old_ind = lvItem->getIndex();
	if(old_ind>=index) lvItem->setIndex(tmp_offs+old_ind);
      }
    }
  }
  
  // load general settings only of loaded from scratch
  if(index<0){
    // get general output file name from DB
    fieldIterator  fi;
    fi  = root->findField("outfile");
    std::string outname;
    if(fi!=root->fieldEnd()){
      myDB->DBProcess((*fi),READ,outname);
      outFname->setText(outname.c_str());
    }
    
    // get module test type
    fi  = root->findField("modTestType");
    std::string mtt;
    if(fi!=root->fieldEnd()){
      myDB->DBProcess((*fi),READ,mtt);
      if(mtt!="NONE"){
	modtestMode->setChecked(true);
	int cbind = modtestType->findText(mtt.c_str());
	if(cbind >=0) modtestType->setCurrentIndex(cbind);
	else m_engine.toErrLog("PrimlistPanel::loadPrl : unknown module test type requested: "+ mtt);
      } else
	modtestMode->setChecked(false);
    } else
      modtestMode->setChecked(false);
  }
    
  // loop over DB entries and create a PrimListItem when an according entry is found
  int nitems=0;
  for(recordIterator objIter = root->recordBegin();objIter!=root->recordEnd();objIter++){
    if((*objIter)->getName()=="PrimListItem"){
      PrimListViewItem *lvItem = new PrimListViewItem(prmlistView,*objIter);
      if(index>=0){
	lvItem->setIndex(nitems+index);
	QString strgind;
	strgind.sprintf("%05d",lvItem->getIndex());
	lvItem->setText(2, strgind);
      }
      nitems++;
    }
  }
  
  delete myDB;

  if(index>=0){ // re-calculate index of items after insertion
    for(int i=0;i<prmlistView->topLevelItemCount();i++){
      PrimListViewItem *lvItem = dynamic_cast<PrimListViewItem*>(prmlistView->topLevelItem(i));
      if(lvItem!=0){
	// adjust index if needed
	int new_ind = lvItem->getIndex()-tmp_offs+nitems;
	if(lvItem->getIndex()>=tmp_offs){
	  lvItem->setIndex(new_ind);
	  QString strgind;
	  strgind.sprintf("%05d",lvItem->getIndex());
	  lvItem->setText(2, strgind);
	}
      }
    }
  }

  prmlistView->sortItems(2,Qt::AscendingOrder);

  emit changeStartState(prmlistView->topLevelItemCount()>0);
  
  m_copyItem = 0;
  pasteButton->setEnabled(false);  
  MergePrlButton->setEnabled(prmlistView->topLevelItemCount()>0);
  return;
}
void PrimlistPanel::disableStart(){
  // just disable start button, no problem to edit list
   emit changeStartState(false);
}
void PrimlistPanel::freezePanel(){
  // make sure prim. list panel isn't re-activated when scans in our list get executed
  QObject::disconnect(&(m_engine), SIGNAL(beganScanning()),    this, SLOT(disableStart()));
  QObject::disconnect(&(m_engine), SIGNAL(finishedScanning()), this, SLOT(finishedScan()));
  // disable panel functionality
  runButton->setEnabled(false);
  prmlistView->setEnabled(false);
  prmlistView->sortItems(2,Qt::AscendingOrder);
  LoadPrlButton->setEnabled(false);
  SavePrlButton->setEnabled(false);
  ClearPrlButton->setEnabled(false);
  MergePrlButton->setEnabled(false);
  newItemButton->setEnabled(false);
  delItemButton->setEnabled(false);
  up100ItemButton->setEnabled(false);
  up10ItemButton->setEnabled(false);
  upItemButton->setEnabled(false);
  downItemButton->setEnabled(false);
  down10ItemButton->setEnabled(false);
  down100ItemButton->setEnabled(false);
  browseButton->setEnabled(false);
  outFname->setEnabled(false);
  copyButton->setEnabled(false);
  pasteButton->setEnabled(false);
  repeatBox->setEnabled(false);
  nrepBox->setEnabled(false);
  indexToFile->setEnabled(false);
  indexToItem->setEnabled(false);
  modtestMode->setEnabled(false);
  modtestType->setEnabled(false);
  burninBox->setEnabled(false);
}
void PrimlistPanel::startPrl()
{
  m_abortPressed = false;

  if(modtestMode->isChecked()){ // module test mode - generate file name from type and index
    outFname->setText(modtestType->currentText());
  }

  if(outFname->text()=="")
    if(QMessageBox::question(this,"PrimlistPanel::startPrl",
			     "No output file name given, \nscan data won't be saved.\n Continue?",
			     QMessageBox::Yes, QMessageBox::No)==QMessageBox::No) return;

  // check consistency of primitive list items
  m_items.clear();
  std::vector<QString> scanLabels;
  bool labelsOK = true;
  for(int i=0;i<prmlistView->topLevelItemCount();i++){
    PrimListViewItem *lvItem = dynamic_cast<PrimListViewItem*>(prmlistView->topLevelItem(i));
    if(lvItem!=0 && labelsOK){
      m_items.push_back((PrimListItem*)lvItem);
      // if it's a scan: check scan label for uniqueness or if its empty and don't start in case of problems
      if(lvItem->getType()==PrimListItem::DEF_SCAN || lvItem->getType()==PrimListItem::CUST_SCAN){
	QString oldLabel, newLabel;
	oldLabel = lvItem->getScanLabel().c_str();
	if(oldLabel==""){
	  QMessageBox::warning(this,"PrimlistPanel::startPrl",
			       ("Primlist item "+lvItem->getLabel()+" has empty scan label\n"+
				"Please enter a new label.").c_str());
	  newLabel="";
	  while(newLabel.isEmpty()){
	    RenameWin rnw(this);
	    rnw.EnterLabel->setText("Enter Scan Label");
	    rnw.Name->setText("");
	    rnw.setWindowTitle("Enter Scan Label");
	    rnw.exec();
	    newLabel = rnw.Name->text();
	  }
	  lvItem->setScanLabel(newLabel.toLatin1().data());
	} else if(STControlEngine::checkScanLabel(scanLabels, oldLabel, newLabel)){
	  QMessageBox::StandardButton user_answer = QMessageBox::information(this, "PrimlistPanel",
									     "Scan label of item "+QString(lvItem->getLabel().c_str())+" has been used before.\n"
									     "RootDB does not support this.\n"
									     "Do you want to use \""+newLabel+"\" instead?",
									     QMessageBox::Yes, QMessageBox::No);
	  if(user_answer == QMessageBox::Yes){
	    lvItem->setScanLabel(newLabel.toLatin1().data());
	    oldLabel = newLabel;
	  }else{
	    QMessageBox::information(this, "PrimlistPanel", 
				     "Please change scan label manually to something different before "
				     "trying to start primitive list execution again.");
	    labelsOK = false;
	  }
	}
	scanLabels.push_back(oldLabel);
      }
    }
  }
  
  if(!labelsOK) return;
  
  if(!m_extCtrl){
    disconnect( runButton, SIGNAL( clicked() ), this, SLOT( startPrl() ) );
    runButton->setText("Abort");
    runButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    freezePanel();
    runButton->setEnabled(true);
  }
  disconnect( &(m_engine), SIGNAL( prlScanStarted(int) ), this, SLOT( freezePanel() ) );
  connect( &(m_engine), SIGNAL( prlInProgress(int) ), this, SLOT( currentProcItem(int) ) );
  disconnect(prmlistView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),  this, SLOT(editItem(QTreeWidgetItem*, int)));
//  connect(prmlistView, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(currentProcItem()));
  if(!m_extCtrl) connect( runButton, SIGNAL( clicked() ), this, SLOT( abortPrl() ) );
  
  // process list - separate from this in case >1 iteration was requested
  m_iiter = 0;
  m_iterMask = 0xff;
  if(burninBox->isChecked()){
    m_iterMask = 0;
    m_iterMask += ((int) checkBoxM0->isChecked()) << 0;
    m_iterMask += ((int) checkBoxM1->isChecked()) << 1;
    m_iterMask += ((int) checkBoxM2->isChecked()) << 2;
    m_iterMask += ((int) checkBoxM3->isChecked()) << 3;
  }
  if(repeatBox->isChecked()){
    digitLabel->show();
    repStep->show();
  } else {
    digitLabel->hide();
    repStep->hide();
  }
  startPrlIter();
}
void PrimlistPanel::startPrlIter()
{

  if(m_abortPressed){
    finishedPrl();
    return;
  }

  int niter=1;
  if(repeatBox->isChecked()) niter=nrepBox->value();

  QString fname = outFname->text();

  if(niter>1){
    repStep->display(m_iiter);
    if(indexToFile->isChecked() && fname!="" && !burninBox->isChecked()){
      // add index to file name
      int extInd = fname.lastIndexOf(".");
      if(extInd>=0)
	fname = fname.left(extInd)+"_"+QString::number(m_iiter+1)+fname.right(fname.length()-extInd);
      else
	fname += "_"+QString::number(m_iiter+1);
    }
  }
  
  FILE *chkexf = fopen(fname.toLatin1().data(),"r");
  if(chkexf!=0 && m_iiter==0){
    fclose(chkexf);
    QMessageBox::warning(this,"PrimlistPanel::startPrl",
			 "Output file already exists.\nPlease browse to a new file before continuing.");
    m_abortPressed = true;
    finishedPrl();
    return;
  }
  
  // tell data viewer to update with scan file
  //  emit scanFileChanged(fname.toLatin1().data());  
  
  int modTestType = 0;
  if(modtestMode->isChecked()){
    modTestType = 1;
    if(burninBox->isChecked()){
       modTestType = 2;
       int imod = 1 << m_iiter;
       if(!(imod & m_iterMask)){ // current index was masked, jump to next
	 finishedPrl();
	 return;
       }
    }
  }

  m_engine.launchPrimList(m_items, fname.toLatin1().data(), m_iiter+1, 
			  indexToItem->isChecked()&&repeatBox->isChecked(), modTestType);
}

void PrimlistPanel::finishedPrl()
{
  // check if more than one iteration was requested and if we have to start again
  m_iiter++;
  int niter=1;
  if(repeatBox->isChecked()) niter=nrepBox->value();
  if(m_iiter<niter && !m_abortPressed){
    startPrlIter();
  } else {
    // otherwise, go into idle state
    finishedScan();
    QObject::connect(&(m_engine), SIGNAL(beganScanning()),    this, SLOT(disableStart()));
    QObject::connect(&(m_engine), SIGNAL(finishedScanning()), this, SLOT(finishedScan()));

    prmlistView->clearSelection();
    prmlistView->setEnabled(true);
    LoadPrlButton->setEnabled(true);
    SavePrlButton->setEnabled(true);
    ClearPrlButton->setEnabled(true);
    MergePrlButton->setEnabled(prmlistView->topLevelItemCount()>0);
    newItemButton->setEnabled(true);
    delItemButton->setEnabled(true);
    up100ItemButton->setEnabled(true);
    up10ItemButton->setEnabled(true);
    upItemButton->setEnabled(true);
    downItemButton->setEnabled(true);
    down10ItemButton->setEnabled(true);
    down100ItemButton->setEnabled(true);
    browseButton->setEnabled(!(modtestMode->isChecked()));
    outFname->setEnabled(!(modtestMode->isChecked() || m_extCtrl));
    copyButton->setEnabled(true);
    pasteButton->setEnabled(m_copyItem!=0);
    repeatBox->setEnabled(true);
    nrepBox->setEnabled(repeatBox->isChecked());
    indexToFile->setEnabled(repeatBox->isChecked());
    indexToItem->setEnabled(repeatBox->isChecked());
    modtestMode->setEnabled(true);
    modtestType->setEnabled(modtestMode->isChecked());
    if(modtestMode->isChecked()){
      burninBox->setEnabled(true);
      if(burninBox->isChecked()) setBirep(true);
    }
    emit finishedList();
  }

}
void PrimlistPanel::finishedScan(){
  // go into idle state
  m_iiter = 0;
  m_lastProcInd = -1;
  disconnect( &(m_engine), SIGNAL( prlInProgress(int) ), this, SLOT( currentProcItem(int) ) );
  connect( &(m_engine), SIGNAL( prlScanStarted(int) ), this, SLOT( freezePanel() ) );
  connect(prmlistView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),  this, SLOT(editItem(QTreeWidgetItem*, int)));
//  disconnect(prmlistView, SIGNAL(itemClicked(QTreeWidgetItem*)), this, SLOT(currentProcItem()));
  if(!m_extCtrl){
    disconnect( runButton, SIGNAL( clicked() ), this, SLOT( abortPrl() ) );
    connect( runButton, SIGNAL( clicked() ), this, SLOT( startPrl() ) );
    runButton->setText("Start list");
    runButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
  }
  emit changeStartState(prmlistView->topLevelItemCount()>0);
}
void PrimlistPanel::currentProcItem(int index)
{
  if(index < prmlistView->topLevelItemCount() && index>=0){
    m_lastProcInd = index;
    prmlistView->clearSelection();
    for(int i=0;i<prmlistView->topLevelItemCount();i++){
      PrimListViewItem *plvi = dynamic_cast<PrimListViewItem*>(prmlistView->topLevelItem(i));
      if(plvi!=0 && plvi->getIndex()==index){
	prmlistView->setCurrentItem(plvi);
	break;
      }
    }
  }
}
void PrimlistPanel::savePrl()
{
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT primlist file (*.prl.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of DB file with primlist entries", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);

  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().at(0);
    qpath.replace("\\", "/");
    
    if(qpath.right(8)!="prl.root") qpath += ".prl.root";

    // check if file exists -> remove
    FILE *test = fopen(qpath.toLatin1().data(),"r");
    if(test!=0){
      fclose(test);
      remove(qpath.toLatin1().data());
    }

    PixLib::PixConfDBInterface *myDB = new RootDB(qpath.toLatin1().data(),"NEW");
    if(myDB==0){
      m_engine.toErrLog(("PrimlistPanel::savePrl : error opening file "+qpath).toLatin1().data());
      return;
    }
    
    PixLib::DBInquire *root = 0;
    try{
      root = myDB->readRootRecord(1);
    } catch(...){    
      delete myDB;
      m_engine.toErrLog(("PrimlistPanel::savePrl : error reading root record in file "+qpath).toLatin1().data());
      return;
    }

    // save output file name
    DBField *fi = 0;
    if(outFname->text()!=""){
      fieldIterator f;
      f = root->findField("outfile");
      if (f==root->fieldEnd()) {
	fi = myDB->makeField("outfile");
	f.pointsTo(fi);
	f = root->pushField(*f);
      }
      std::string fname = outFname->text().toLatin1().data();
      myDB->DBProcess(f,COMMIT,fname);
      myDB->DBProcess(root,COMMITREPLACE);
      delete fi; fi=0;
    }

    // save module test type (set to NONE if not in module test mode)
    fieldIterator f;
    f = root->findField("modTestType");
    if (f==root->fieldEnd()) {
      DBField *fi = myDB->makeField("modTestType");
      f.pointsTo(fi);
      f = root->pushField(*f);
    }
    std::string mttype = "NONE";
    if(modtestMode->isChecked()) mttype = modtestType->currentText().toLatin1().data();
    myDB->DBProcess(f,COMMIT,mttype);
    myDB->DBProcess(root,COMMITREPLACE);
    delete fi; fi=0;
    
    for(int i=0;i<prmlistView->topLevelItemCount();i++){
      PrimListViewItem *lvItem = dynamic_cast<PrimListViewItem*>(prmlistView->topLevelItem(i));
      if(lvItem!=0){
	std::stringstream a;
	a << lvItem->getIndex();
	std::string name = "PrimListItem";
	std::string decName = name+a.str();
	DBInquire *inq = root->addInquire(name, decName);
	if(lvItem->getType()==PrimListItem::CUST_SCAN) // need record for PixScan config
	  inq->addInquire("ScanConfig","ScanConfig");
	if(lvItem->getType()==PrimListItem::CHIP_TEST) // need record for ChipTest config
	  inq->addInquire("ChipTest","ChipTest");
	if(!lvItem->config().write(inq))
	  m_engine.toErrLog("PrimlistPanel::savePrl : error writing Config of item "+lvItem->getLabel());
      }
    }

    delete myDB;
  }
}
void PrimlistPanel::abortPrl()
{
  m_abortPressed = true;
  m_engine.abortPrlExec();
  runButton->setEnabled(false);
}
void PrimlistPanel::clearPrl()
{
  // somehow the simple clear command crashes on linux, so clear manually (which works)
  //prmlistView->clear();
  for( int i = prmlistView->topLevelItemCount(); i >= 0; --i ){
    delete prmlistView->topLevelItem( i );
  }
  m_copyItem = 0;
  pasteButton->setEnabled(false);  
  MergePrlButton->setEnabled(false);
  emit changeStartState(false);
}
void PrimlistPanel::addItem()
{
  PrlItemInit iniPri(m_engine, this);

  if(iniPri.exec()==QDialog::Accepted){
    PixScan *tmpPs = new PixScan(PixScan::DIGITAL_TEST, PixModule::PM_FE_I4A);
    PrimListItem::PrimListTypes pl_type = (PrimListItem::PrimListTypes)iniPri.prliType->currentIndex();
    PixScan *ps = 0;
    if(pl_type==PrimListItem::CUST_SCAN){ // get PixScan object for init.
      int maxDef = tmpPs->getScanTypes().size();
      int index = iniPri.prliCustType->currentIndex();
      if(index >= maxDef)
	ps = m_engine.getPixScanCfg()[(index-maxDef)];
      else{
	ps = tmpPs;
	QString scan_type = iniPri.prliCustType->currentText();
	scan_type.remove(0,9); // remove "DEFAULT: "
	ps->preset((PixScan::ScanType) ps->getScanTypes()[scan_type.toLatin1().data()], 
		   iniPri.flavourI4Button->isChecked()?PixModule::PM_FE_I4A:PixModule::PM_FE_I2);
      }
    }
    PrimListViewItem *prli = new PrimListViewItem(prmlistView,pl_type, prmlistView->topLevelItemCount(), 
						  iniPri.Comments->text().toLatin1().data(), ps);
    delete tmpPs;
    connect(prli, SIGNAL(editThisPS(PixLib::PixScan*, QWidget*)), this, SLOT(editPS(PixLib::PixScan*, QWidget*)));
    prli->edit();
    disconnect(prli, SIGNAL(editThisPS(PixLib::PixScan*, QWidget*)), this, SLOT(editPS(PixLib::PixScan*, QWidget*)));
  }

  MergePrlButton->setEnabled(prmlistView->topLevelItemCount()>0);
  emit changeStartState(prmlistView->topLevelItemCount()>0);

  return;
}
void PrimlistPanel::browseOutput()
{
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defDataPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT data file (*.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of DB file for scan output", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);
  
  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
    // auto-add root-extension
    if(fname.right(5)!=".root" && fname!="") fname+=".root";
    outFname->setText(fname);
  }
}
void PrimlistPanel::editItem(QTreeWidgetItem* item, int)
{
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(item);
  if(prli!=0){
    connect(prli, SIGNAL(editThisPS(PixLib::PixScan*, QWidget*)), this, SLOT(editPS(PixLib::PixScan*, QWidget*)));
    prli->edit();
    disconnect(prli, SIGNAL(editThisPS(PixLib::PixScan*, QWidget*)), this, SLOT(editPS(PixLib::PixScan*, QWidget*)));
  }
  return;
}
void PrimlistPanel::deleteCurrItem()
{
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to delete it");
  else{
    prmlistView->sortItems(2,Qt::AscendingOrder);
    PrimListViewItem *prli_next = (PrimListViewItem *)prmlistView->itemBelow(prli);
    PrimListViewItem *prli_prev = (PrimListViewItem *)prmlistView->itemAbove(prli);
    delete prli;
    if(prli_next!=0) prmlistView->setCurrentItem(prli_next);
    else if(prli_prev!=0) prmlistView->setCurrentItem(prli_prev);
    while(prli_next!=0){
      // decrement index of following items
      prli_next->setIndex(prli_next->getIndex()-1);
      // needed to get correct sorting for different # of digits
      QString strgind;
      strgind.sprintf("%05d",prli_next->getIndex());
      prli_next->setText(2, strgind);
      prli_next = (PrimListViewItem *)prmlistView->itemBelow(prli_next);
    }
  }
  m_copyItem = 0;
  pasteButton->setEnabled(false);  
  MergePrlButton->setEnabled(prmlistView->topLevelItemCount()>0);
  emit changeStartState(prmlistView->topLevelItemCount()>0);
}
void PrimlistPanel::up100CurrItem()
{
  prmlistView->sortItems(2,Qt::AscendingOrder);
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to move it");
  else{
    if(prmlistView->itemAbove(prli)==0)
      QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Item is at top, can't move");
    else
      for(int i = 0; i < 100; i++){
	prli->up();
	prmlistView->sortItems(2,Qt::AscendingOrder);
      }
  }
  prmlistView->sortItems(2,Qt::AscendingOrder);
  return;
}
void PrimlistPanel::up10CurrItem()
{
  prmlistView->sortItems(2,Qt::AscendingOrder);
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to move it");
  else{
    if(prmlistView->itemAbove(prli)==0)
      QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Item is at top, can't move");
	else
      for(int i = 0; i < 10; i++){
	prli->up();
	prmlistView->sortItems(2,Qt::AscendingOrder);
      }
  }
  prmlistView->sortItems(2,Qt::AscendingOrder);
  return;
}
void PrimlistPanel::upCurrItem()
{
  prmlistView->sortItems(2,Qt::AscendingOrder);
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to move it");
  else{
    if(prmlistView->itemAbove(prli)==0)
      QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Item is at top, can't move");
    else
      prli->up();
  }
  prmlistView->sortItems(2,Qt::AscendingOrder);
  return;
}
void PrimlistPanel::downCurrItem()
{
  prmlistView->sortItems(2,Qt::AscendingOrder);
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to move it");
  else{
    if(prmlistView->itemBelow(prli)==0)
      QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Item is at bottom, can't move");
    else
      prli->down();
  }
  prmlistView->sortItems(2,Qt::AscendingOrder);
  return;
}
void PrimlistPanel::down10CurrItem()
{
  prmlistView->sortItems(2,Qt::AscendingOrder);
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to move it");
  else{
    if(prmlistView->itemBelow(prli)==0)
      QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Item is at bottom, can't move");
    else
      for(int i = 0; i < 10; i++){
	prli->down();
	prmlistView->sortItems(2,Qt::AscendingOrder);
      }
  }
  prmlistView->sortItems(2,Qt::AscendingOrder);
  return;
}
void PrimlistPanel::down100CurrItem()
{
  prmlistView->sortItems(2,Qt::AscendingOrder);
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to move it");
  else{
    if(prmlistView->itemBelow(prli)==0)
      QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Item is at bottom, can't move");
    else
      for(int i = 0; i < 100; i++){
	prli->down();
	prmlistView->sortItems(2,Qt::AscendingOrder);
      }
  }
  prmlistView->sortItems(2,Qt::AscendingOrder);
  return;
}
void PrimlistPanel::editPS(PixScan *ps, QWidget *winParent)
{
  m_engine.getPixScanCfg().push_back(ps); // temporarily add our PixScan object so that the panel knows it
  PixScanPanel *psp = new PixScanPanel(m_engine, winParent, Qt::Window);
  winParent->setEnabled(false);
  psp->setEnabled(true);
  psp->setMaximumHeight(400);
  psp->tabwidget->setTabEnabled(0, false);
  psp->tabwidget->setCurrentIndex(1);
  int index = ps->getScanTypes().size() + m_engine.getPixScanCfg().size()-1;
  psp->scanConfigSel->setCurrentIndex(index);
  psp->updateConfig();
  psp->show();
  while(psp->isVisible()){
    m_engine.updateGUI();
    PixLib::sleep(100);
  }
  delete psp;
  m_engine.getPixScanCfg().erase(m_engine.getPixScanCfg().end()-1); // remove temp. list item
  winParent->setEnabled(true);
}
void PrimlistPanel::copyItem(){
  PrimListViewItem *prli = dynamic_cast<PrimListViewItem *>(prmlistView->currentItem());
  if(prli==0)
    QMessageBox::warning(this,"PrimlistPanel::deleteCurrItem","Please select an item before trying to copy it");
  else{
    m_copyItem = prli;
    pasteButton->setEnabled(true);  
  }
}
void PrimlistPanel::pasteItem(){
  if(m_copyItem!=0){
    PrimListViewItem *prli = new PrimListViewItem( prmlistView, *m_copyItem, prmlistView->topLevelItemCount());
    connect(prli, SIGNAL(editThisPS(PixLib::PixScan*, QWidget*)), this, SLOT(editPS(PixLib::PixScan*, QWidget*)));
    m_copyItem = 0;
    pasteButton->setEnabled(false);
    prmlistView->setCurrentItem(prli);
  }
}
void PrimlistPanel::modTestMode(bool){
  setBirep(burninBox->isChecked());
}
std::vector<PrimListItem*>& PrimlistPanel::getPrlItems(){
  m_items.clear();
  for(int i=0;i<prmlistView->topLevelItemCount();i++){
    PrimListViewItem *lvItem = dynamic_cast<PrimListViewItem*>(prmlistView->topLevelItem(i));
    if(lvItem!=0){
      m_items.push_back((PrimListItem*)lvItem);
    }
  }
  return m_items;
}
void PrimlistPanel::setBirep(bool isBi){
  repeatBox->setChecked(isBi && modtestMode->isChecked());
  repeatBox->setEnabled(!(isBi && modtestMode->isChecked()));
  if(isBi){
    indexToFile->setChecked(true); 
    indexToFile->setEnabled(false);
    indexToItem->setEnabled(false);
    nrepBox->setValue(4);
  }
  checkBoxM0->setEnabled(isBi && modtestMode->isChecked());
  checkBoxM1->setEnabled(isBi && modtestMode->isChecked());
  checkBoxM2->setEnabled(isBi && modtestMode->isChecked());
  checkBoxM3->setEnabled(isBi && modtestMode->isChecked());
}
