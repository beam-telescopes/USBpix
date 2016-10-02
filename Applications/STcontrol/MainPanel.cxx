#include <Config/Config.h>
#include "Config/ConfGroup.h"
#include "Config/ConfObj.h"
#include <verauth.h>
#include <OptionsPanel.h>
#include <DataViewer.h>
#include <RenameWin.h>
#include <RefWin.h>

#include "ModList.h"
#include "MainPanel.h"
#include "PixScanPanel.h"
#include "RegisterPanel.h"
#include "ConfigEdit.h"
#include "ConfigEditI4.h"
#include "STControlEngine.h"
#include "STRodCrate.h"
#include "ToolPanel.h"
#include "LogPanel.h"
#include "STCLog.h"
#include "STCLogContainer.h"
#include "SetAll.h"
#include "PrimlistPanel.h"
#include "WaferTestPanel.h"
#include "NewDcsDialog.h"
#include "MultiboardCfg.h"
#include "ConfigCreator.h"
#ifndef NOTDAQ
#include "BocAnalysisPanel.h"
#endif

// temp.
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/GeneralDBfunctions.h>
#include <PixModuleGroup/PixModuleGroup.h>
#include <PixDcs/PixDcs.h>

#include <TROOT.h>

#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QLayout>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QSplitter> 
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QUrl>
#include <QPixmap>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#ifndef WIN32
#include <unistd.h>
#endif

using namespace PixLib;
using namespace SctPixelRod;

MainPanel::MainPanel( STControlEngine &engine_in, STCLogContainer &log_in, QWidget* parent, Qt::WindowFlags fl, bool smallScreen) :
  QMainWindow(parent, fl), m_engine( engine_in ), m_sTCLogContainer( log_in )
{

  setupUi(this);
  QObject::connect(closePanelAction, SIGNAL(triggered()), this, SLOT(checkClose()));
  QObject::connect(fileCloseAction, SIGNAL(triggered()), this, SLOT(closeConfig()));
  QObject::connect(decremGDAC, SIGNAL(triggered()), this, SLOT(decrGDAC()));
  QObject::connect(decremGDACBy, SIGNAL(triggered()), this, SLOT(decrGDACBy()));
  QObject::connect(decremTDAC, SIGNAL(triggered()), this, SLOT(decrTDAC()));
  QObject::connect(decremTDACBy, SIGNAL(triggered()), this, SLOT(decrTDACBy()));
  QObject::connect(currModuleCfgEdit, SIGNAL(triggered()), this, SLOT(editCurrentModule()));
  QObject::connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(loadConfig()));
  QObject::connect(optionsReload_defaultAction, SIGNAL(triggered()), this, SLOT(loadOptions()));
  QObject::connect(actionNew_cfg, SIGNAL(triggered()), this, SLOT(newConfig()));
  QObject::connect(actionNew_cfg_generic, SIGNAL(triggered()), this, SLOT(newConfigGeneric()));
  QObject::connect(fileReopenAction, SIGNAL(triggered()), this, SLOT(reopenConfig()));
  QObject::connect(fileSaveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
  QObject::connect(fileSaveAction, SIGNAL(triggered()), this, SLOT(saveConfig()));
  QObject::connect(optionsSave_as_defaultAction, SIGNAL(triggered()), this, SLOT(saveOptions()));
  QObject::connect(editset_all_FDACs_from_scanAction, SIGNAL(triggered()), this, SLOT(setFDACs()));
  QObject::connect(editset_all_TDACs_from_scanAction, SIGNAL(triggered()), this, SLOT(setTDACs()));
  QObject::connect(actionSet_all_masks_from_scan, SIGNAL(triggered()), this, SLOT(setMasks()));
  QObject::connect(editset_all_VCALs_to_charge_ofAction, SIGNAL(triggered()), this, SLOT(setVcal()));
  QObject::connect(showAboutMen, SIGNAL(triggered()), this, SLOT(showAbout()));
#ifndef NOTDAQ
  QObject::connect(showBocAnalysisChoice, SIGNAL(triggered()), this, SLOT(showBocAnalysis()));
#endif
  QObject::connect(showDataChoice, SIGNAL(triggered()), this, SLOT(showData()));
  QObject::connect(currShowInfo, SIGNAL(triggered()), this, SLOT(showInfo()));
  QObject::connect(showLogPanelChoice, SIGNAL(triggered()), this, SLOT(showLogs()));
  QObject::connect(showModListChoice, SIGNAL(triggered()), this, SLOT(showModList()));
  QObject::connect(actionDCS_view, SIGNAL(triggered()), this, SLOT(showDcsView()));
  QObject::connect(optionsShow_PanelAction, SIGNAL(triggered()), this, SLOT(showOptions()));
  QObject::connect(showPixScanChoice, SIGNAL(triggered()), this, SLOT(showPixScan()));
  QObject::connect(showPrimlistPanelChoice, SIGNAL(triggered()), this, SLOT(showPrimList()));
  QObject::connect(showWafertestPanelChoice, SIGNAL(triggered()), this, SLOT(showWaferTest()));
  QObject::connect(showReadmeMen, SIGNAL(triggered()), this, SLOT(showReadme()));
  QObject::connect(showRegisterChoice, SIGNAL(triggered()), this, SLOT(showRegister()));
  QObject::connect(showToolChoice, SIGNAL(triggered()), this, SLOT(showTools()));
  QObject::connect(decrMccDelAction, SIGNAL(triggered()), this, SLOT(incrMccDelay()));
  QObject::connect(single_FE_Action, SIGNAL(triggered()), this, SLOT(setSingleFe()));

  currItemAdd->setEnabled(false);

  m_scanActive = false;

  // split main window into two sliding halves
  m_split = new QSplitter( this );
  vboxLayout->addWidget(m_split);
  // initial siye limits
  int xsizr = 550;
  int xsizl = 700;
  int ysizr = 800;
  int ysizl = 800;
  if(smallScreen){
    xsizr = 500;
    xsizl = 600;
    ysizr = 650;
    ysizl = 650;
  }

  // add set-up viewing panels to the left half
  if(m_engine.getRDSame()){
    m_cmnwid = new QWidget(m_split);
    QVBoxLayout *vboxLayout2 = new QVBoxLayout(m_cmnwid);
    m_split_left_panel = new QSplitter(m_cmnwid);
    m_split_left_panel->setOrientation(Qt::Vertical);
    vboxLayout2->addWidget(m_split_left_panel);
    m_modList = new ModList( m_engine, m_cmnwid, 0 , ModList::DAQList);
    m_split_left_panel->addWidget(m_modList);
    //vboxLayout2->addWidget(m_modList);
    m_dcsList = new ModList( m_engine, m_cmnwid, 0 , ModList::DCSList);
    m_split_left_panel->addWidget(m_dcsList);
    //vboxLayout2->addWidget(m_dcsList);
    ysizr /=2;
  } else{
    m_cmnwid = 0;
    m_modList = new ModList( m_engine, m_split, 0 , ModList::DAQList);
    m_dcsList = new ModList( m_engine, m_split, 0 , ModList::DCSList);
  }
  m_modList->setMinimumSize(QSize(xsizr, ysizr));
  m_dcsList->setMinimumSize(QSize(xsizr, ysizr));

  connect( m_modList->ModListView, SIGNAL( itemClicked(QTreeWidgetItem*, int) ), this, SLOT( setCurrentModule(QTreeWidgetItem*, int) ) );  
  connect( m_dcsList->ModListView, SIGNAL( itemClicked(QTreeWidgetItem*, int) ), this, SLOT( setCurrentModule(QTreeWidgetItem*, int) ) );  
  connect( m_modList, SIGNAL( listUpdated(QTreeWidgetItem*) ), this, SLOT( setCurrentModule(QTreeWidgetItem*) ) ); 
 
  // other panels to the right half
  m_dataPanel = new DataViewer(m_split, Qt::Widget);
  m_dataPanel->setMinimumSize(QSize(xsizl, ysizl));
  m_pixScanPanel = new PixScanPanel(m_engine, m_split);
  m_pixScanPanel->setMinimumSize(QSize(xsizl, ysizl));
  m_regPanel  = new RegisterPanel(m_engine, m_split, Qt::Widget);
  m_regPanel->setMinimumSize(QSize(xsizl, ysizl));
  m_toolPanel = new ToolPanel( m_engine, m_split );
  m_toolPanel->setMinimumSize(QSize(xsizl, ysizl));
  m_logPanel = new LogPanel( m_split );
  m_logPanel->setMinimumSize(QSize(xsizl, ysizl));
#ifndef NOTDAQ
  m_bocPanel = new BocAnalysisPanel(m_engine,m_split, "bocpanel", 0);
#endif
  m_prlPanel = new PrimlistPanel( m_engine, m_split );
  m_prlPanel->setMinimumSize(QSize(xsizl, ysizl));
  m_prbPanel = new WaferTestPanel( m_engine, m_split );
  m_prbPanel->setMinimumSize(QSize(xsizl, ysizl));
  showModList();
  setCurrentModule(0,0);
  //showPixScan();
  showTools();
  m_modList->updateView();
  m_dcsList->updateView();

  // item last clicked on module view
  m_currentListItem = 0;

  // manipulated layout of PixScanPanel in constructor, have to fit to new size
  adjustSize();

  Config &opts = m_engine.getOptions();
  std::string logPath = ((ConfString&)opts["paths"]["defLogPath"]).value();
  if(logPath!="") m_logPanel->browseLogFolder(logPath.c_str());

  // first-time set-up of config type in engine should change this panel's menu(es) and items in tool panel
  connect( &(m_engine), SIGNAL(configChanged()), this, SLOT(setShowMenues()));
  connect( &(m_engine), SIGNAL(cfgLabelChanged()),this, SLOT(setMyTitle()));
  connect( &(m_engine), SIGNAL(configChanged()), dynamic_cast<ToolPanel*>(m_toolPanel), SLOT( setIdle()));
  // initialize logging
  connect( &(m_sTCLogContainer.log())       , SIGNAL( added( const QString& ) ), m_logPanel, SLOT( logToSTC( const QString& ) ) );
  connect( &(m_sTCLogContainer.buffers())   , SIGNAL( added( const QString& ) ), m_logPanel, SLOT( logToROD( const QString& ) ) );
  connect( &(m_sTCLogContainer.errLog())    , SIGNAL( added( const QString& ) ), m_logPanel, SLOT( errorToSTC( const QString& ) ) );
  connect( &(m_sTCLogContainer.errBuffers()), SIGNAL( added( const QString& ) ), m_logPanel, SLOT( errorToROD( const QString& ) ) );
  connect( m_logPanel, SIGNAL(wantRodBuffers()), &m_engine, SLOT(readRodBuff()));

  // scan related signals
  connect( &(m_engine), SIGNAL(beganScanning()),    dynamic_cast<ModList*>(m_dcsList), SLOT( setBusy()));
  connect( &(m_engine), SIGNAL(finishedScanning()), dynamic_cast<ModList*>(m_dcsList), SLOT( setIdle()));
  connect( &(m_engine), SIGNAL(beganScanning()),    dynamic_cast<ToolPanel*>(m_toolPanel), SLOT( setBusy()));
  connect( &(m_engine), SIGNAL(finishedScanning()), dynamic_cast<ToolPanel*>(m_toolPanel), SLOT( setIdle()));
  connect( &(m_engine), SIGNAL(beganScanning()),    this, SLOT(setScanBusy()));
  connect( &(m_engine), SIGNAL(finishedScanning()), this, SLOT(setScanDone()));
  connect( &(m_engine), SIGNAL(breakloop()), dynamic_cast<ToolPanel*>(m_toolPanel), SLOT( setBreak()));

  connect( &(m_engine), SIGNAL( prlScanStarted(int) ), m_pixScanPanel, SLOT( scanRunning(int) ) );
  connect( &(m_engine), SIGNAL( scanConfigChanged() ), m_pixScanPanel, SLOT( updateConfigSel() ) );
  connect( &(m_engine), SIGNAL( gotPixScanData(const char *) ), m_dataPanel, SLOT( browseFile(const char *) ) );
  connect( &(m_engine), SIGNAL( currentModule(int, int, int)), m_regPanel, SLOT( setModule(int, int, int) ) );
  connect( &(m_engine), SIGNAL( blockCurrMod(bool)), m_regPanel, SLOT( setCurrModBlock(bool)));
  // connect PixScanPanel file name to Data Viewer file
  m_dataPanel->browseFile(m_pixScanPanel->scanFileName->text().toLatin1().data());
  connect( m_pixScanPanel, SIGNAL( scanFileChanged(const char*) ), m_dataPanel, SLOT( browseFile(const char *) ) );
  //connect( m_prlPanel, SIGNAL( scanFileChanged(const char*) ), m_pixScanPanel, SLOT( browseButton_clicked(const char *) ) );
  connect(&(m_engine), SIGNAL( scanFileChanged(const char *) ), m_pixScanPanel, SLOT( browseButton_clicked(const char *) ) );
  // connect engine to BOC analysis features of resp. panel
  connect(&(m_engine), SIGNAL( analyseBoc(const char *, const char *) ), this, SLOT (AnalyseBocData(const char* , const char*)) );
  // allow engine to open pop-up windows even though it's not a widget
  connect(&(m_engine), SIGNAL( popupWin(const char*) ), this, SLOT (showPopupWin(const char*)) );

  // BOC stuff only with TDAQ
#ifndef NOTDAQ
  connect( m_bocPanel, SIGNAL( errorMessage(std::string)  ),&(m_engine), SLOT ( toErrLog(std::string) ));
  connect( m_bocPanel, SIGNAL( bocScanRunning() ), m_pixScanPanel, SLOT( scanRunning() ) );
  connect( m_pixScanPanel, SIGNAL( enableBocAnaScan(bool) ), m_bocPanel, SLOT (enableAnaScan(bool) ) );
  connect( m_bocPanel, SIGNAL( sendPixScanStatus(int, int, int, int, int, int, int, int, int) ), m_pixScanPanel, SLOT(updateStatus(int, int, int, int, int, int, int, int, int)));
#endif

  setMyTitle();
}
MainPanel::~MainPanel(){
}
void MainPanel::showModList(){
  if(m_engine.getRDSame()){
    m_cmnwid->show();
  } else{
    m_modList->show();
    m_dcsList->hide();
  }
  showModListChoice->setEnabled(false);
  actionDCS_view->setEnabled(!m_engine.getRDSame());
  showGeographicalChoice->setEnabled(false);// true);
  setCurrentModule((ModListItem*)m_modList->ModListView->children().first());
}
void MainPanel::showDcsView(){
  if(m_engine.getRDSame()){
    m_cmnwid->show();
  } else{
    m_modList->hide();
    m_dcsList->show();
  }
  showModListChoice->setEnabled(true);
  actionDCS_view->setEnabled(false);
  showGeographicalChoice->setEnabled(false);// true);
  setCurrentModule((ModListItem*)m_dcsList->ModListView->children().first());
}
void MainPanel::showPixScan(){
  m_pixScanPanel->show();
  m_regPanel->hide();
  m_dataPanel->hide();
  m_toolPanel->hide();
  m_logPanel->hide();
  m_prlPanel->hide();
  m_prbPanel->hide();
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(false);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(true);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(true);
#ifndef NOTDAQ
  showBocAnalysisChoice->setEnabled(true);
  m_bocPanel->hide();
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}
void MainPanel::showData(){
  m_pixScanPanel->hide();
  m_regPanel->hide();
  m_dataPanel->show();
  m_toolPanel->hide();
  m_logPanel->hide();
  m_prlPanel->hide();
  m_prbPanel->hide();
  showDataChoice->setEnabled(false);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(true);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(true);

#ifndef NOTDAQ
  m_bocPanel->hide();
  showBocAnalysisChoice->setEnabled(true);
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}

void MainPanel::showRegister(){
  m_pixScanPanel->hide();
  m_regPanel->show();
  m_dataPanel->hide();
  m_toolPanel->hide();
  m_logPanel->hide();
  m_prlPanel->hide();
  m_prbPanel->hide();
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(false);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(true);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(true);
#ifndef NOTDAQ
  m_bocPanel->hide();
  showBocAnalysisChoice->setEnabled(true);
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}
void MainPanel::showTools(){
  m_pixScanPanel->hide();
  m_regPanel->hide();
  m_dataPanel->hide();
  m_toolPanel->show();
  m_logPanel->hide();
  m_prlPanel->hide();
  m_prbPanel->hide();
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(false);
  showLogPanelChoice->setEnabled(true);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(true);
#ifndef NOTDAQ
  m_bocPanel->hide();
  showBocAnalysisChoice->setEnabled(true);
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}
void MainPanel::showLogs(){
  m_pixScanPanel->hide();
  m_regPanel->hide();
  m_dataPanel->hide();
  m_toolPanel->hide();
  m_logPanel->show();
  m_prlPanel->hide();
  m_prbPanel->hide();
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(false);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(true);
#ifndef NOTDAQ
  m_bocPanel->hide();
  showBocAnalysisChoice->setEnabled(true);
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}

void MainPanel::showBocAnalysis(){
  m_pixScanPanel->hide();
  m_regPanel->hide();
  m_dataPanel->hide();
  m_toolPanel->hide();
  m_logPanel->hide();
#ifndef NOTDAQ
  m_bocPanel->show();
#endif
  m_prlPanel->hide();
  m_prbPanel->hide();
  showPixScanChoice->setEnabled(true);
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(true);
  showBocAnalysisChoice->setEnabled(false);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(true);
}
void MainPanel::showPrimList()
{
  m_pixScanPanel->hide();
  m_regPanel->hide();
  m_dataPanel->hide();
  m_toolPanel->hide();
  m_logPanel->hide();
  m_prlPanel->show();
  m_prbPanel->hide();
  showPixScanChoice->setEnabled(true);
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(true);
  showPrimlistPanelChoice->setEnabled(false);
  showWafertestPanelChoice->setEnabled(true);
#ifndef NOTDAQ
  m_bocPanel->hide();
  showBocAnalysisChoice->setEnabled(true);
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}
void MainPanel::showWaferTest(){
  m_pixScanPanel->hide();
  m_regPanel->hide();
  m_dataPanel->hide();
  m_toolPanel->hide();
  m_logPanel->hide();
  m_prlPanel->hide();
  m_prbPanel->show();
  showPixScanChoice->setEnabled(true);
  showDataChoice->setEnabled(true);
  showPixScanChoice->setEnabled(true);
  showRegisterChoice->setEnabled(true);
  showToolChoice->setEnabled(true);
  showLogPanelChoice->setEnabled(true);
  showPrimlistPanelChoice->setEnabled(true);
  showWafertestPanelChoice->setEnabled(false);
#ifndef NOTDAQ
  m_bocPanel->hide();
  showBocAnalysisChoice->setEnabled(true);
#else
  showBocAnalysisChoice->setEnabled(false);
#endif
}

void MainPanel::addDcs(ModListItem *mlitem){
  if(mlitem==0) return;
  STRodCrate *crt = mlitem->getCrate();
  std::vector<std::string> grpNames;
  for( std::vector<STRodCrate *>::iterator crate = m_engine.getSTRodCrates().begin(); 
       crate != m_engine.getSTRodCrates().end(); crate++ ) {
    for(int iGrp=0; iGrp<(*crate)->nGroups(); iGrp++){
      grpNames.push_back((*crate)->getGrpName(iGrp));
    }
  }
  NewDcsDialog ndd(grpNames, this);
  if(ndd.exec()){
    int devType = ndd.objectDevType->itemData(ndd.objectDevType->currentIndex()).toInt();
    QString genType = ndd.objectGenType->currentText();
    if(genType=="USB" && ndd.objectAssCtrlName->currentText()!=""){
      // special treatment of USBPix adapter boards
      int pGrp = -1;
      for(int iGrp=0; iGrp<crt->nGroups(); iGrp++){
	if(crt->getGrpName(iGrp)==std::string(ndd.objectAssCtrlName->currentText().toLatin1().data())) {
	  pGrp = iGrp;
	  break;
	}
      }
      //std::cout << "Ctrl ID " << pGrp << std::endl;
      // get controller cfg and check if it has BI or GPAC flag set
      if(pGrp>=0){
	if(crt->getPixCtrlConf(pGrp)["general"].name()!="__TrashConfGroup__" && 
	   crt->getPixCtrlConf(pGrp)["general"]["AdapterCardFlavor"].name()!="__TrashConfObj__"){
	  if(((ConfInt&)crt->getPixCtrlConf(pGrp)["general"]["AdapterCardFlavor"]).value()==1)
	    genType += "BI"; 
	  if(((ConfInt&)crt->getPixCtrlConf(pGrp)["general"]["AdapterCardFlavor"]).value()==2)
	    genType += "Gpac"; 
	}
      }
      //std::cout << "general type " << genType << ", dev. type " << devType << std::endl;
    }
    m_engine.addDcsToCurrDB(ndd.objectLabel->text().toLatin1().data(), genType.toLatin1().data(), devType, 
			    ndd.nChannels->value(), ndd.objectAssCtrlName->currentText().toLatin1().data(), 
			    mlitem->getCrate()->getName().c_str());
  }
}

void MainPanel::editCurrentModule(){
  ModListItem *item = m_currentListItem;
  if(item==0) return;
  ModList *list=m_modList;
  if(item->treeWidget()==m_dcsList->ModListView){
    list = m_dcsList;
  }

  if(item->getType()==ModListItem::tcrate){
    // editing actually means adding new devices
    if(item->text(1)=="DCS") // DCS panel -> add DCS item
      addDcs(item);
    else // DAQ panel -> add PixModuleGroup - still to be implemented
      QMessageBox::warning(this, "MainPanel::editCurrentModule()", "Adding module groups is not yet implemented.");
  } else{
    // normal editing functionality
    list->editModule(item,false);
  }
}
void MainPanel::setCurrentModule(QTreeWidgetItem *in_item, int col){
  // Make sure in_item is not 0.

  if(col!=0) return; // should click on module label, not on properties

  ModListItem *item = 0;
  if(in_item!=0)
    item = dynamic_cast<ModListItem*>(in_item);
  m_currentListItem = item;

  if(m_scanActive) return; // must not enable edit menu items when scan is running

  QString label = "current module: ";
  if(item!=0 && item->getType()==ModListItem::tmodule){
    // set register panel label
    label += item->text(0);
    m_regPanel->setModule(item->crateId(), item->grpId(),item->modId(),label.toLatin1().data());
    // edit cfg label
    label = "Edit cfg (module ";
    label += item->text(0);
    label += ")";
    currModuleCfgEdit->setText(label);
    currModuleCfgEdit->setEnabled(true);
    // HW info
    label = "Show Info for XXX";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(false);
    // FE select
    label = "select single &FE cfg (mod ";
    label += item->text(0);
    label += ")";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(true);
  }else if(item!=0 && item->getType()==ModListItem::tgroup){
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit cfg label
    currModuleCfgEdit->setEnabled(true);
    label = "Edit cfg (group ";
    label += item->text(0);
    label += ")";
    currModuleCfgEdit->setText( label );
    // HW info
    label = "Show Info for XXX";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(false);
    // FE select
    label = "select single &FE cfg (grp ";
    label += item->text(0);
    label += ")";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(true);
  }else if(item!=0 && item->getType()==ModListItem::tUSB){
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit cfg label
    currModuleCfgEdit->setEnabled( true );
    label = "Edit USB-&cfg (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currModuleCfgEdit->setText( label );
    // NEW: show HW interface instead of delete function
    label = "&Show USB HW Info (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(true);
    // FE select
    label = "select single &FE cfg (grp ";
    label += item->parentGroup()->text(0);
    label += ")";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(true);
  }else if(item!=0 && item->getType()==ModListItem::tCTRL){
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit cfg label
    currModuleCfgEdit->setEnabled( true );
    label = "Edit Ctrl-&cfg (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currModuleCfgEdit->setText( label );
    // NEW: show HW interface instead of delete function
    label = "&Show Ctrl HW Info (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(true);
    // FE select
    label = "select single &FE cfg (grp ";
    label += item->parentGroup()->text(0);
    label += ")";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(true);
  }else if(item!=0 && item->getType()==ModListItem::tROD){
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit cfg label
    currModuleCfgEdit->setEnabled( true );
    label = "Edit ROD-&cfg (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currModuleCfgEdit->setText( label );
    // NEW: show HW interface instead of delete function
    currShowInfo->setText(label);
    label = "&Show ROD HW Info (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(true);
    // FE select
    label = "select single &FE cfg (grp ";
    label += item->parentGroup()->text(0);
    label += ")";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(true);
  }else if(item!=0 && item->getType()==ModListItem::tBOC){
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit cfg label
    currModuleCfgEdit->setEnabled( true );
    label = "&Edit BOC-cfg (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currModuleCfgEdit->setText( label );
    // show HW info
    label = "&Show Info for XXX";
    //    label = "&Show BOC HW Info (group ";
    label += item->parentGroup()->text(0);
    label += ")";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(false);//true);
    // FE select
    label = "select single &FE cfg (grp ";
    label += item->parentGroup()->text(0);
    label += ")";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(true);
  }else if(item!=0 && item->getType()==ModListItem::tDCS){ // item is DCS device
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit item
    currModuleCfgEdit->setEnabled( true );
    label = "&Edit DCS-cfg (";
    label += item->text(0);
    label += ")";
    currModuleCfgEdit->setText(label);
    // HW info
    label = "Update DCS-readings";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(true);
    // FE select
    label = "select single &FE cfg";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(false);
  }else if(item!=0 && item->getType()==ModListItem::tDCSC){ // item is DCS channel
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // edit item
    currModuleCfgEdit->setEnabled( true );
    label = "&Edit channel-cfg (";
    label += item->text(0);
    label += ")";
    currModuleCfgEdit->setText(label);
    // HW info
    label = "Update DCS-readings";
    currShowInfo->setText(label);
    currShowInfo->setEnabled(true);
    // FE select
    label = "select single &FE cfg";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(false);
  }else if(item!=0 && item->getType()==ModListItem::tcrate){ // item is crate level
    // un-set register panel label
    m_regPanel->setModule(-1,-1,-1);
    // turn edit item into add item
    label = (item->text(1)=="DCS")?"&Add DCS item to ":"&Add module group to ";
    label += item->text(0);
    currModuleCfgEdit->setText(label);
    // add option for now only for DCS - fix later
    // (generating new cfg. is basically the same as adding a group)
    currModuleCfgEdit->setEnabled(item->text(1)=="DCS");
    // HW info
    label = "Show Info for "+item->text(0);
    currShowInfo->setText(label);
    currShowInfo->setEnabled(false);
    // FE select
    if(item->text(1)=="DCS")
      label = "---";
    else{
      label = "select single &FE cfg (";
      label += item->text(0);
      label += ")";
    }
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(item->text(1)!="DCS");
  } else{ // null-item
    // un-set register panel label
    m_regPanel->setModule(99999,-1,-1);
    // disable editing
    currModuleCfgEdit->setEnabled( false );
    currModuleCfgEdit->setText( "Edit cfg" );
    // FE select
    label = "select single &FE cfg (all mods)";
    single_FE_Action->setText(label);
    single_FE_Action->setEnabled(!showModListChoice->isEnabled());
  }
  return;
}
void MainPanel::checkClose(){
  if(b_closeConfig())
    close();
}

void MainPanel::reopenConfig(){
  if(std::string(m_engine.getPixConfDBFname())==""){
    QMessageBox::information(this,"STcontrol: MainPanel::reopenConfig()", "Can't find DB object in memory.");
    return;
  }

  //@ to do: do this properly once PixLib can life-update configs
  QString path = m_engine.getPixConfDBFname();
  if(b_closeConfig())
    loadConfig(path.toLatin1().data());
  
}
void MainPanel::loadConfig(const char *path){
  bool do_load=false;
  QString my_path;
  if(std::string(m_engine.getPixConfDBFname())!=""){
    QMessageBox::warning(this,"STcontrol::loadConfig","There is already a loaded config ("+QString(m_engine.getPixConfDBFname())+
			 ") in memory.\nPlease close it first before continuing." );
    return;
  }
  if(path==0){
    Config &opts = m_engine.getOptions();
    std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
    QString qpath = QString::null;
    if(defPath!=".") qpath = defPath.c_str();
    QStringList filter;
    filter += "DB ROOT-config file (*.cfg.root)";
    filter += "DB ascii-config file (*.cfg)";
    filter += "Any file (*.*)";
    QFileDialog fdia(this, "Specify name of RootDB config-file", qpath);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::ExistingFile);
    if(fdia.exec() == QDialog::Accepted){
      do_load = true;
      my_path = fdia.selectedFiles().at(0);
      my_path.replace("\\", "/");
    }
  } else{
    my_path = path;
    do_load = true;
  }    
  if(do_load){
    m_engine.loadDB(my_path.toLatin1().data());
  }
  
  return;
}
void MainPanel::saveAs(){
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT-config file (*.cfg.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of RootDB config-file", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);
  if(fdia.exec() == QDialog::Accepted){
    QString my_fname = fdia.selectedFiles().at(0);
    if(my_fname.right(9)!=".cfg.root")
      my_fname += ".cfg.root";
    my_fname.replace("\\", "/");
    saveConfig(my_fname.toLatin1().data());
  }
  return;
}
void MainPanel::saveConfig(const char *path){

  // tell engine to save
  m_engine.saveDB(path);

  return; 
}


void MainPanel::newConfigGeneric(){
  // can only start from scratch
  if(std::string(m_engine.getPixConfDBFname())!=""){
    QMessageBox::information(this,"STcontrol: MainPanel::newConfig()",
                             "A DB object already exists in memory, can't proceed.");
    return;
  }
  // get file name from user
  QString my_fname;
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT-config file (*.cfg.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of new RootDB cfg-file", qpath);
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  if(fdia.exec() == QDialog::Accepted){
    my_fname = fdia.selectedFiles().at(0);
    my_fname.replace("\\", "/");
    if(my_fname.right(9)!=".cfg.root")
      my_fname += ".cfg.root";

    ConfigCreator cc(this);
    if(cc.exec() == QDialog::Accepted){
      std::vector<PixModuleGroup*> grps = cc.getCfg();
      PixConfDBInterface *myDB = PixLib::createDefaultDB(my_fname.toLatin1().data(), 0);
      DBInquire *appInq = findAppInq(myDB, 0);
      for(unsigned int iGrp=0; iGrp<grps.size(); iGrp++){
	std::string name, decName;
	// new module group
	name="PixModuleGroup";
	decName = appInq->getDecName() + std::string(grps[iGrp]->getName());
	DBInquire *grpInq = myDB->makeInquire(name, decName);
	appInq->pushRecord(grpInq);
	myDB->DBProcess(appInq,COMMITREPLACE);
	myDB->DBProcess(grpInq,COMMIT);
	grps[iGrp]->config().write(grpInq);
      }
      // add regulators if requested
      std::vector<PixDcs*> dcs = cc.getDcs();
      for(std::vector<PixDcs*>::iterator it = dcs.begin(); it!=dcs.end(); it++){
	std::string name, decName;
	name="PixDcs";
	decName = appInq->getDecName() + (*it)->name();
	DBInquire *dcsInq = myDB->makeInquire(name, decName);
	appInq->pushRecord(dcsInq);
	myDB->DBProcess(appInq,COMMITREPLACE);
	myDB->DBProcess(dcsInq,COMMIT);
	(*it)->config().write(dcsInq);
      }
      delete myDB;
      m_engine.loadDB(my_fname.toLatin1().data());
    }
  }
}

void MainPanel::newConfig(){
  // can only start from scratch
  if(std::string(m_engine.getPixConfDBFname())!=""){
    QMessageBox::information(this,"STcontrol: MainPanel::newConfig()",
                             "A DB object already exists in memory, can't proceed.");
    return;
  }
  // get file name from user
  QString my_fname;
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT-config file (*.cfg.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this,"Specify name of new RootDB cfg-file", qpath);
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  if(fdia.exec() == QDialog::Accepted){
    my_fname = fdia.selectedFiles().at(0);
    my_fname.replace("\\", "/");
    if(my_fname.right(9)!=".cfg.root")
      my_fname += ".cfg.root";
    MultiboardCfg mbcfg(m_engine,this);
    if(mbcfg.exec() == QDialog::Accepted){
      IFtypes myIfType = tundefIF;
      std::vector<grpData> myGrpDataV;
      std::vector<MultiboardCfg::cfgInfo> cfgList = mbcfg.getCfgList();
      for(std::vector<MultiboardCfg::cfgInfo>::iterator it = cfgList.begin(); it!=cfgList.end(); it++){
        grpData myGrpData;
        myGrpData.myROD.slot = it->boardID[0];
        myGrpData.myBOC.mode = it->boardID[1];
        myGrpData.myBOC.haveBoc = false;
        myGrpData.cfgType = 1;
        myGrpData.myROD.IPfile = mbcfg.fpgaFwFile->text().toLatin1().data();
        myGrpData.myROD.IDfile = mbcfg.uCFwFile->text().toLatin1().data();
        myGrpData.myROD.Xfile = "";
        myIfType = tUSBSys;
        myGrpData.myROD.mode   = it->adapterType;
        if(it->FEflavour==2) // read cfg. from file (as is)
          myGrpData.cfgType = 2;
        else if(it->FEflavour==3 && it->fileName.size()>1) // read cfg. from file (nx 1-FE -> n-FE)
          myGrpData.cfgType = 3;
        for(unsigned int im=0;im<it->fileName.size(); im++){
          modData myModD;
          myModD.fname = it->fileName[im].toLatin1().data();
          myModD.modname= it->newFeName[im].toLatin1().data();
          myModD.connName= it->modDecName[im].toLatin1().data();
          myModD.modID   = (int) im;
          myModD.grpID   = 0;
          myModD.active  = true;
          myModD.roType  = 0;
          myModD.inLink  = 0;
          for(int olID=0;olID<4;olID++)
            myModD.outLink[olID]  = 0;
          myModD.slot = 0;
          myModD.pp0 = -1;
          myModD.assyType = it->FEnum;
          myModD.assyID = it->FEflavour+1;
          myModD.pos_on_assy = 0;
          myGrpData.myMods.push_back(myModD);
        }
        myGrpDataV.push_back(myGrpData);
      }
      m_engine.addCrateToDB("TestApp", myGrpDataV, my_fname.toLatin1().data(), myIfType, mbcfg.usbDcsBox->isChecked());
      if(mbcfg.addDcsBox->isChecked() && mbcfg.rootCfgFileDcs->text()!=""){
        std::string ctrlName="USB-board";
        std::stringstream b;
        b << myGrpDataV[0].myROD.slot;
        ctrlName += "_"+b.str();
        m_engine.copyDcsDb(my_fname.toLatin1().data(), mbcfg.rootCfgFileDcs->text().toLatin1().data(), ctrlName.c_str());
      }
      m_engine.loadDB(my_fname.toLatin1().data());
    }
  }
}
/** Close the current configuration file and clear STCEngine. */
bool MainPanel::b_closeConfig(){
  if(m_engine.cfgEdited()){
    QMessageBox overwrite(QMessageBox::Question, "cfg. was edited!", 
                          "The configuration has been edited\nDo you want to continue?\nThis will erase all changes!",
                          QMessageBox::Yes|QMessageBox::No, this);
    if(overwrite.exec()==QMessageBox::No)
      return false;
  }
  m_engine.clear();
  m_engine.setPixConfDBFname("");

  return true;
}

void MainPanel::decrGDACBy(){
  SetAll sti(this);
  sti.setWindowTitle("Set GDAC increment");
  sti.TextLabel1->setText("Set GDAC increment");
  sti.SpinBox->setMinimum(-31);
  sti.SpinBox->setMaximum(32);
  if(sti.exec()){
    decrGDAC(sti.SpinBox->value());
  }
}
void MainPanel::decrGDAC(int step){
  bool cfgOne=false;
  // loop over crates
  std::vector <STRodCrate*> myCrates = m_engine.getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
    // get groups in current crate and loop over them
    for(int iGrp=0; iGrp<(*crIT)->nGroups(); iGrp++){
      std::vector< std::pair<std::string, int> > list;
      (*crIT)->getModules(iGrp, list);
      for(int iMod=0; iMod<(int)list.size(); iMod++){
	std::vector<Config*> fe_conf;
	for(int chipID=0;chipID<16;chipID++) {
	  Config &tmpCfg = (*crIT)->getPixModuleChipConf(iGrp,iMod, chipID);
	  if(tmpCfg.name()=="dummychip") break; // end of FE list reached
	  else fe_conf.push_back(&tmpCfg);
	}
	Config &mod_conf = (*crIT)->getPixModuleConf(iGrp,iMod);
	std::string fetype="FEX";
	if(mod_conf["general"].name()!="__TrashConfGroup__" && mod_conf["general"]["FE_Flavour"].name()!="__TrashConfObj__")
	  fetype = ((ConfList&)mod_conf["general"]["FE_Flavour"]).sValue();
	if(fetype=="FE_I2" || fetype=="FE_I1"){      // FE-I1, FE-I2/3
	  ConfigEdit cfged(&mod_conf, 0, fe_conf, 0, this);
	  cfged.shiftValue(4, step);
	  cfged.StoreCfg();
	}else if(fetype=="FE_I4A" || fetype=="FE_I4B"){ // FE-I4
	  ConfigEditI4 cfged(&mod_conf, fe_conf, 0, this);
 	  cfged.shiftValue(4, step);
 	  cfged.storeCfg();
	} else{ // unknown
	  m_engine.toLog("MainPanel::decrGDAC() : Can't identify FE flavour ("+fetype+"), no GDAC action possible.");
	  continue;
	}
        // send modified module config to PixController if that's initialised
        m_engine.editedCfg();
        if((*crIT)->getPixCtrlStatus(iGrp)==tOK){
	  cfgOne = true;
          try{
	    (*crIT)->writeConfig(iGrp,iMod);
          } catch(...){
            m_engine.toLog("MainPanel::decrGDAC() : Error sending edited configuration to PixController!");
          }
	}
      }
    }
  }
  if(cfgOne) m_engine.readRodBuff("Post-config edit:");
  return;
}
void MainPanel::decrTDACBy(){
  SetAll sti(this);
  sti.setWindowTitle("Set TDAC increment");
  sti.TextLabel1->setText("Set TDAC increment");
  sti.SpinBox->setMinimum(-127);
  if(sti.exec()){
    decrTDAC(sti.SpinBox->value());
  }
}
void MainPanel::decrTDAC(int step){
  bool cfgOne=false;
  // loop over crates
  std::vector <STRodCrate*> myCrates = m_engine.getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
    // get groups in current crate and loop over them
    for(int iGrp=0; iGrp<(*crIT)->nGroups(); iGrp++){
      std::vector< std::pair<std::string, int> > list;
      (*crIT)->getModules(iGrp, list);
      for(int iMod=0; iMod<(int)list.size(); iMod++){
	std::vector<Config*> fe_conf;
	for(int chipID=0;chipID<16;chipID++) {
	  Config &tmpCfg = (*crIT)->getPixModuleChipConf(iGrp,iMod, chipID);
	  if(tmpCfg.name()=="dummychip") break; // end of FE list reached
	  else fe_conf.push_back(&tmpCfg);
	}
	Config &mod_conf = (*crIT)->getPixModuleConf(iGrp,iMod);
	std::string fetype="FEX";
	if(mod_conf["general"].name()!="__TrashConfGroup__" && mod_conf["general"]["FE_Flavour"].name()!="__TrashConfObj__")
	  fetype = ((ConfList&)mod_conf["general"]["FE_Flavour"]).sValue();
	if(fetype=="FE_I2" || fetype=="FE_I1"){      // FE-I1, FE-I2/3
	  ConfigEdit cfged(&mod_conf, 0, fe_conf, 0, this);
	  cfged.ShiftPTable(1,step);
	}else if(fetype=="FE_I4A" || fetype=="FE_I4B"){ // FE-I4
	  ConfigEditI4 cfged(&mod_conf, fe_conf, 0, this);
	  cfged.ShiftPTable(1,step);
	} else{ // unknown
	  m_engine.toLog("MainPanel::decrTDAC() : Con't identify FE flavour ("+fetype+"), no TDAC action possible.");
	  continue;
	}
        // send modified module config to PixController if that's initialised
        m_engine.editedCfg();
        if((*crIT)->getPixCtrlStatus(iGrp)==tOK){
	  cfgOne = true;
	  try{
	    (*crIT)->writeConfig(iGrp,iMod);
	  } catch(...){
            m_engine.toLog("MainPanel::decrTDAC() : Error sending edited configuration to PixController!");
	  }
	}
      }
    }
  }
  if(cfgOne) m_engine.readRodBuff("Post-config edit:");
  return;
}
void MainPanel::showAbout(){
  QString about = "This is the ATLAS Pixel Module/System-Test Control GUI\n\n"
    "Version " PLGUI_VERSION " built\nwith ROOT version " + QString(gROOT->GetVersion()) +
    "\nand QT version " + QString(qVersion()) + "."
    "\n\nCurrently active authors of GUI and libraries:\n" PLGUI_AUTHORS;
  QMessageBox habout(QMessageBox::Information,"Pixel DAQ GUI Help About", about, QMessageBox::Ok, this);
  habout.setIconPixmap(QPixmap(QString::fromUtf8(":/icons/images/GA_Si.jpg")));
  habout.exec();
}
void MainPanel::showReadme(){
  QUrl url("http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/STcontrolI4");
  QDesktopServices::openUrl(url);
}
void MainPanel::showInfo(){
  ModListItem *item = 0;
  item=m_modList->currentBOC();
  if(item==0)
    item=m_modList->currentROD();
  if(item==0)
    item = dynamic_cast<ModListItem*>(m_dcsList->ModListView->currentItem());

  STRodCrate *crt = 0;
  if(item!=0) crt = item->getCrate();
  if(crt==0){
    QMessageBox::warning(this,"MainPanel::showInfo()","Can't find crate for this item");
    return;
  }

  if (item!=0 && (item->getType()==ModListItem::tUSB || item->getType()==ModListItem::tROD || item->getType()==ModListItem::tCTRL)){
    if(crt->getPixCtrlStatus(item->grpId())!=tOK)
      QMessageBox::warning(this,"MainPanel::showInfo()","Controller hasn't been initialised yet or is not idle, can't proceed");
    else{
      std::string infoTxt;
      crt->ctrlHwInfo(item->grpId(), infoTxt);
      QMessageBox::information(this,"MainPanel::showInfo()",infoTxt.c_str());
    }
  } else if (item!=0 && (item->getType()==ModListItem::tDCS || item->getType()==ModListItem::tDCSC)){
    m_dcsList->updateView();
  } else
    QMessageBox::warning(this,"MainPanel::showInfo()","Only implemented for ROD/BOC and DCS.");

}
void MainPanel::showOptions(){
  optionsPanel op(m_engine.getOptions(),this);
  op.exec();
}
void MainPanel::saveOptions(){
  m_engine.saveOptions();
}
void MainPanel::loadOptions(){
  m_engine.loadOptions();
}
void MainPanel::setVcal(){
  RenameWin rnw(this);
  rnw.EnterLabel->setText("Enter charge (e) that VCAL should be set to");
  rnw.optBox->show();
  rnw.optBox->setText("Chigh");
  rnw.exec();
  if(!rnw.Name->text().isEmpty()){
    bool isOK;
    float chargeVcal = rnw.Name->text().toFloat(&isOK);
    if(isOK)
      m_engine.setVcal(chargeVcal, rnw.optBox->isChecked());
  }
}
void MainPanel::incrMccDelay()
{
  RenameWin rnw(this);
  rnw.EnterLabel->setText("Enter MCC delay in-/decrement");
  rnw.optBox->show();
  rnw.optBox->setText("delay/ns(ticked)");
  rnw.exec();
  if(!rnw.Name->text().isEmpty()){
    bool isOK;
    float delay = rnw.Name->text().toFloat(&isOK);
    if(isOK)
      m_engine.incrMccDelay(delay, rnw.optBox->isChecked());
  }
}
void MainPanel::setTFDACs(bool isTDAC)
{
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defDataPath"]).value();
  QString dlabel = isTDAC?"TDAC":"FDAC";
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT file (*.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of RootDB data-file", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif

  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
    RefWin sclist(this, 0, fname.toLatin1().data(), (dlabel+"_T").toLatin1().data());
    sclist.SelectorLabel->hide();
    sclist.DatSel->hide();
    sclist.FuncselLabel->setText("select a scan:");
    sclist.setWindowTitle("Select scans with "+dlabel+" histograms");

    if(sclist.FuncSel->count()>0){
      if(sclist.exec()){
	int not_found = m_engine.setTFDACs((fname+":/"+sclist.FuncSel->currentText()).toLatin1().data(), isTDAC);
	if(not_found>0)
	  QMessageBox::warning(this,"MainPanel::setTFDACs","Couldn't find "+dlabel+" entries for "+
			       QString::number(not_found)+((not_found==1)?" module":" modules"));
      }
    } else{
      QMessageBox::warning(this,"MainPanel::setTFDACs","No scans with histo "+dlabel+"_T found in file "+fname);
    }
  }  
}
void MainPanel::setMasks(){
  QString histoType = "HITOCC";
  int mask = 0;
  bool keepLoading=true;
  QString logicOper="";
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defDataPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT file (*.root)";
  filter += "Any file (*.*)";
  std::vector<std::string> files, histos;
  while(keepLoading){
    QFileDialog fdia(this, "Specify name of RootDB data-file", qpath);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::ExistingFile);
    if(fdia.exec() == QDialog::Accepted){
      QString fname = fdia.selectedFiles().at(0);
      fname.replace("\\", "/");
      RefWin sclist(this, 0, fname.toLatin1().data(), histoType.toLatin1().data());
      QCheckBox *cben = new QCheckBox("Apply to ENABLE", &sclist);
      sclist.comboLayout->addWidget(cben);
      QCheckBox *cbc0 = new QCheckBox("Apply to CAP0", &sclist);
      sclist.comboLayout->addWidget(cbc0);
      QCheckBox *cbc1 = new QCheckBox("Apply to CAP1", &sclist);
      sclist.comboLayout->addWidget(cbc1);
      QCheckBox *cbil = new QCheckBox("Apply to ILEAK", &sclist);
      sclist.comboLayout->addWidget(cbil);
      if(mask!=0){
	sclist.logicComboBox->setEnabled(false);
	for(int i=0; i<sclist.logicComboBox->count();i++){
	  if(logicOper==sclist.logicComboBox->itemText(i)){
	    sclist.logicComboBox->setCurrentIndex(i);
	    break;
	  }
	}
	cben->setEnabled(false);
	cbc0->setEnabled(false);
	cbc1->setEnabled(false);
	cbil->setEnabled(false);
	cben->setChecked(mask&1);
	cbc0->setChecked(mask&2);
	cbc1->setChecked(mask&4);
	cbil->setChecked(mask&8);
      }
      sclist.nextButton->show();
      sclist.logicComboBox->show();
      sclist.DatSel->hide();
      sclist.SelectorLabel->hide();
      if(sclist.FuncSel->count()>0){
	if(sclist.exec() == QDialog::Accepted){
	  files.push_back((fname+":/"+sclist.FuncSel->currentText()).toLatin1().data());
	  histos.push_back(histoType.toLatin1().data());
	  if(mask==0){
	    mask = (int) cben->isChecked() + (((int) cbc1->isChecked() )<<1) + (((int) cbc0->isChecked() )<<2) + 
	      (((int) cbil->isChecked() )<<3);
	    logicOper = sclist.logicComboBox->currentText();
	  }
	  if(!sclist.nextClicked()){
	    keepLoading = false;
	    int not_found = m_engine.setMasks(files, histos, mask, std::string(logicOper.toLatin1().data()));
	    if(not_found>0)
	      QMessageBox::warning(this,"MainPanel::setMasks","Couldn't find "+sclist.DatSel->currentText()+
				   " entries for "+QString::number(not_found)+((not_found==1)?" module":" modules"));
	  }
	} else
	  keepLoading = false;
      } else{
	keepLoading = false;
	QMessageBox::warning(this,"MainPanel::setMasks","No scans with histo "+histoType+" found in file "+fname);
      }
    } else
      keepLoading = false;
  }
}
void MainPanel::setSingleFe()
{
  ModListItem *item = 0;
  int modID = -1;
  int grpID = -1; 
  int crtID = -1;
  item=m_modList->currentModule();
  if(item==0){
    item=m_modList->currentGroup();
    if(item==0){
      item=m_modList->currentCrate();
      if(item!=0)
	crtID = item->crateId();
    }else{
      grpID = item->grpId();
      crtID = item->crateId();
    }
  }else{
    modID = item->modId();
    grpID = item->grpId();
    crtID = item->crateId();
  }
  RenameWin rnw(this);
  rnw.EnterLabel->setText("Enter FE index that should be selected (-1 restores cfg of all FEs)");
  rnw.Name->setText("-1");
  rnw.exec();
  if(!rnw.Name->text().isEmpty()){
    bool isOK;
    int iFE = rnw.Name->text().toInt(&isOK);
    if(isOK)
      m_engine.selectFe(iFE, crtID, grpID, modID);
  }
}

#ifndef NOTDAQ
void MainPanel::AnalyseBocData(const char* filename, const char* scanlabel ){
  m_bocPanel->Analyse(filename, scanlabel);
  m_bocPanel->WriteData();
#else
void MainPanel::AnalyseBocData(const char* , const char*  ){
#endif
}
void MainPanel::showPopupWin(const char *msg)
{
  QMessageBox::information(this,"STcontrol: MainPanel", msg);
}
void MainPanel::setShowMenues(){
  if(m_pixScanPanel->isVisible()) showPixScan();
  if(m_regPanel->isVisible()) showRegister();
  if(m_dataPanel->isVisible()) showData();
  if(m_toolPanel->isVisible()) showTools();
  if(m_logPanel->isVisible()) showLogs();
#ifndef NOTDAQ
  if(m_bocPanel->isVisible()) showBocAnalysis();
#endif
  if(m_prlPanel->isVisible()) showPrimList();
}
void MainPanel::setScanBusy(){
  // file menu
  menuNew_config->setEnabled(false);
  //actionNew_cfg->setEnabled(false);
  fileOpenAction->setEnabled(false);
  fileCloseAction->setEnabled(false);
  fileReopenAction->setEnabled(false);
  fileSaveAsAction->setEnabled(false);
  fileSaveAction->setEnabled(false);
  // edit menu
  currItemAdd->setEnabled(false);
  currModuleCfgEdit->setEnabled(false);
  currGroupDelete->setEnabled(false);
  currShowInfo->setEnabled(false);
  single_FE_Action->setEnabled(false);

  decremGDAC->setEnabled(false);
  decremGDACBy->setEnabled(false);
  decremTDAC->setEnabled(false);
  decremTDACBy->setEnabled(false);
  decrMccDelAction->setEnabled(false);
  editset_all_FDACs_from_scanAction->setEnabled(false);
  editset_all_TDACs_from_scanAction->setEnabled(false);
  editset_all_VCALs_to_charge_ofAction->setEnabled(false);

  m_scanActive = true;
}
void MainPanel::setScanDone(){
  m_scanActive = false;
  setCurrentModule(0); // takes care of top part of edit menu
  // file menu
  menuNew_config->setEnabled(true);
  //  actionNew_cfg->setEnabled(true);
  fileOpenAction->setEnabled(true);
  fileCloseAction->setEnabled(true);
  fileReopenAction->setEnabled(true);
  fileSaveAsAction->setEnabled(true);
  fileSaveAction->setEnabled(true);
  // edit menu - only bottom part
  decremGDAC->setEnabled(true);
  decremGDACBy->setEnabled(true);
  decremTDAC->setEnabled(true);
  decremTDACBy->setEnabled(true);
  decrMccDelAction->setEnabled(true);
  editset_all_FDACs_from_scanAction->setEnabled(true);
  editset_all_TDACs_from_scanAction->setEnabled(true);
  editset_all_VCALs_to_charge_ofAction->setEnabled(true);

}
void MainPanel::setMyTitle(){
  QString appAst="";
  if(m_engine.cfgEdited()) appAst = "*";
  std::string myFile = m_engine.getPixConfDBFname();
  PixLib::getDecNameCore(myFile);
  QString myTitle= "Pixel Test DAQ Control; ";
  if(myFile=="") myTitle += "no config loaded";
  else           myTitle += "current configuration: "+QString(myFile.c_str())+appAst;
  setWindowTitle(myTitle);
}
