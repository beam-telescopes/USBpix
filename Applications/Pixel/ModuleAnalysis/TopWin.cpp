#include <qradiobutton.h>
#include <qpushbutton.h> 
#include <qlabel.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <q3listview.h>
#include <q3listbox.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <q3table.h>
#include <q3popupmenu.h>
#include <qcursor.h>
#include <q3filedialog.h>
#include <qapplication.h>
#include <qmenu.h>
#include <qdir.h>
#include <qtimer.h>
#include <q3multilineedit.h>
#include <q3groupbox.h>
#include <qregexp.h>
#include <qdialog.h>
#include <qtabwidget.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qevent.h>
#include <q3process.h>
#include <q3textbrowser.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QPixmap>

#include <TSystem.h>
#include <TObjString.h>
#include <TInterpreter.h>
#include <TCint.h>
#include <TKey.h>
#include <TTimeStamp.h>
#include <TPaletteAxis.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TMath.h>

#ifdef HAVE_PLDB
#include <PixConfDBInterface.h>
#include <RootDB.h>
#include <Histo.h>
#include <Config/Config.h>
#ifdef PIXEL_ROD // can link to PixScan
#include <PixScan.h>
#include "ConfigViewer.h"
#include "DBtoMAData.h"
#include <PixDBData.h>
#include <FitClass.h>
#endif
#endif

#include <stdio.h>
#include <map>
#include <string>
#include <sstream>

#include "TopWin.h"
#include "ModWin.h"
#include "RootStuff.h"
#include "ModTree.h"
#include "PixelDataContainer.h"
#include "DataStuff.h"
#include "FileWin.h"
#include "FitWin.h"
#include "PSFitWin.h"
#include "RefWin.h"
#include "RenameWin.h"
#include "OptWin.h"
#include "CorrelSel.h"
#include "StdTestWin.h"
#include "LogDisp.h"
#include "LogClasses.h"
#include "BadPixel.h"
#include "CutPanel.h"
#include "MaskedSel.h"
#include "QAReport.h"
#include "TotRecal.h"
#include "PDBForm.h"
#include "RootDataList.h"
#include "SelectionPanel.h"
#include "CfgForm.h"
#include "Twodsel.h"
#include "MultiLoad.h"
#include "MultiLoadRDB.h"
#include "RegPanel.h"
#include "LoginPanel.h"
#include "CDatabase.h"
#include "LoadStave.h"
#include "Overviewselect.h"
#include "MAEngine.h"
#include "ParFitfun.h"
#include "MAFitting.h"
#include "MaskMaker.h"
#include <verauth.h>

TopWin::TopWin( QWidget* parent, const char* , Qt::WFlags , bool depends, QApplication *app)
  : QWidget(parent), m_app(app){
//  : TopWinBase( parent, name, fl ), m_app(app) {
  setupUi(this);
  QObject::connect(pixScanLevel, SIGNAL(valueChanged(int)), this, SLOT(setScaptBoxes(int)));
  //  QObject::connect(ChipOrMod, SIGNAL(stateChanged(int)), this, SLOT(ModChipSel()));
  //  QObject::connect(ChipOrMod2, SIGNAL(stateChanged(int)), this, SLOT(ModChipSel2()));
  QObject::connect(ChipSel, SIGNAL(valueChanged(int)), this, SLOT(UpdatePixVal()));
  QObject::connect(ColSel, SIGNAL(valueChanged(int)), this, SLOT(UpdatePixVal()));
  QObject::connect(RowSel, SIGNAL(valueChanged(int)), this, SLOT(UpdatePixVal()));
  QObject::connect(ModuleList, SIGNAL(rightButtonPressed(Q3ListViewItem*,QPoint,int)), this, SLOT(ModuleList_rpr(Q3ListViewItem*,QPoint,int)));
  QObject::connect(ModuleList, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(SetCurrMod(Q3ListViewItem*)));
  QObject::connect(ModuleList, SIGNAL(doubleClicked(Q3ListViewItem*)), this, SLOT(ModuleList_itemselected(Q3ListViewItem*)));
  QObject::connect(ModuleList, SIGNAL(returnPressed(Q3ListViewItem*)), this, SLOT(ModuleList_return(Q3ListViewItem*)));
  
  // print version number on the front panel
  m_verstrg = PLGUI_VERSION;
  VersionLabel->setText("");//Module Analysis Frame - version " + m_verstrg);// + ";   this: 0x" + QString::number((int)this,16));
  // some init stuff
  m_browser=QString::null;
  m_depends = depends;
  m_lastPitem = 0;
  m_lastPdbID = -1;
  m_lastPdbFPars.clear();
  m_lastPdbFID = -1;
  m_PlotIsActive = false;
  m_brwsproc = new Q3Process(this);

  // arranging of items in the tree view
  ModuleList->setSorting(0);//,FALSE);

  // options: set default and create window for user interaction
  options = 0;
  options = new OptWin(this);
  Options_load(); // load user's default if file exists (otherwise hard-coded is used)
  options->Reset_clicked();

  // cut panel
  m_cutpanel = new CutPanel(this);
  connect(options,SIGNAL(showCCclicked(bool)),m_cutpanel,SLOT(hideCCTab(bool)));

  // start root stuff
  rootmain = new RMain(options->m_Opts);
  ChipSel->setValue(rootmain->m_chip);
  ColSel->setValue(rootmain->m_col);
  RowSel->setValue(rootmain->m_row);
  m_lastchip  = -1;
  m_memdir = gDirectory;

  // fit window
  m_FitWin = new FitWin(this, Qt::WType_TopLevel);
  m_FitWin->autoCalNote->hide();
  m_FitWin->autoCalNote2->hide();

  // file loading window
  m_fwin = new FileWin(this,Qt::WType_TopLevel);
  // type-add functionality moved to MAEngine and FileWin
  
  // fitting routine class
  m_fitting = new MAFitting(m_app, options->m_Opts);
  connect(m_fitting, SIGNAL(sendText(QString)), this, SLOT(setText(QString)));

  // menus
  m_mainpop = new QMenu("&Main Menu", this);
  m_mainpop->addAction("&New Stave/sector",this,SLOT(newStave()));
  m_mainpop->addAction("&New Module",this,SLOT(ModuleButton_clicked()));
  m_mainpop->insertSeparator();
  m_mainpop->addAction("&Load data file for curr. module",this,SLOT(LoadDataCurrMod()));
  m_mainpop->addAction("Load file-l&ist for curr. module",this,SLOT(LoadListCurrMod()));
  m_mainpop->addAction("Load std &file-set for curr. mod.",this,SLOT(LoadStdCurrMod()));
  m_mainpop->addAction("Load MA &ROOT file (select items)",this,SLOT(LoadRoot_Menu()));
  m_mainpop->addAction("Load MA R&OOT file (all items)",this,SLOT(LoadRootAll()));
  m_mainpop->addAction("Load Root&DB data file",this,SLOT(LoadRootPixLib()));
  m_mainpop->addAction("List contents of Root&DB data file",this,SLOT(ListRootPixLib()));
  m_mainpop->addAction("&Save all for curr. module",this,SLOT(SaveDataCurrMod()));
  m_mainpop->addAction("Save &highlighted for curr. module",this,SLOT(SaveSelDataCurrMod()));
  m_mainpop->addAction("Save &all modules",this,SLOT(SaveAllData()));
  m_mainpop->insertSeparator();
  m_mainpop->addAction("Load, analyse, PD&B-upload several modules",this,SLOT(LoadMultiModule()));
  m_mainpop->addAction("Load and analyse several modules from a RootDB file",this,SLOT(LoadMultiModuleRDB()));
  m_mainpop->addAction("&Upload PDB mechanical test for stave/sector",this,SLOT(staveMechPDB()));
  m_mainpop->addAction("Get ROOT file of single module from RDB",this,SLOT(getModuleRootFromRDB()));
  m_mainpop->addAction("&Get ROOT files of stave/sector from RDB",this,SLOT(getStaveRootFromRDB()));
  m_mainpop->addAction("&Get ROOT files of list of modules from RDB",this,SLOT(getListRootFromRDB()));
  m_mainpop->addAction("Cr&eate config files for stave/sector",this,SLOT(getStaveConfig()));
  m_mainpop->insertSeparator();
  m_mainpop->addAction("Execute ROOT &command",this,SLOT(RootCommand()));
  m_mainpop->addAction("Execute ROOT &macro",this,SLOT(RootMacro()));
  m_mainpop->insertSeparator();
  m_mainpop->addAction("Save can&vas",this,SLOT(SaveCanvas()));
  m_mainpop->addAction("&Print canvas",this,SLOT(PrintCanvas()));
  m_mainpop->insertSeparator();
  if(depends)
    m_mainpop->addAction("&Close",this,SLOT(ExitButton_clicked()));
  else
    m_mainpop->addAction("&Quit",this,SLOT(ExitButton_clicked()));

  m_mainpop->setItemEnabled(m_mainpop->idAt(3),false);
  m_mainpop->setItemEnabled(m_mainpop->idAt(4),false);
  m_mainpop->setItemEnabled(m_mainpop->idAt(5),false);
#ifndef HAVE_PLDB
  m_mainpop->setItemEnabled(m_mainpop->idAt(8),false);
  m_mainpop->setItemEnabled(m_mainpop->idAt(9),false);
#endif
  m_mainpop->setItemEnabled(m_mainpop->idAt(10),false);
  m_mainpop->setItemEnabled(m_mainpop->idAt(11),false);
  m_mainpop->setItemEnabled(m_mainpop->idAt(25),false); // print menu, doesn't work for now

  m_editpop = new QMenu("&Edit", this);
  m_editpop->addAction("&Rename curr. module",this,SLOT(RenameCurrMod()));
  m_editpop->addAction("&Delete curr. module",this,SLOT(RemoveCurrMod()));
  m_editpop->addAction("Erase &fitted raw data (curr. mod.)",this,SLOT(RemRawCurrMod()));
  m_editpop->addAction("Compress data to &summaries (curr. module)",this,SLOT(ClearFitsCurrMod()));
  m_editpop->insertSeparator();
  m_editpop->addAction("Delete &all modules",this,SLOT(RemoveAllMod()));
  m_editpop->addAction("&Erase all fitted raw data",this,SLOT(RemoveAllRaw()));
  m_editpop->addAction("&Compress all data to summaries",this,SLOT(ClearAllFits()));
  m_editpop->setItemEnabled(m_editpop->idAt(0),false);
  m_editpop->setItemEnabled(m_editpop->idAt(1),false);
  m_editpop->setItemEnabled(m_editpop->idAt(2),false);
  m_editpop->setItemEnabled(m_editpop->idAt(3),false);

  m_optpop = new QMenu("&Options", this);
  m_optpop->addAction("Show &Panel",this,SLOT(Options_clicked()));
  m_optpop->addAction("&Reload default",this,SLOT(Options_load()));
  m_optpop->addAction("&Save as default",this,SLOT(Options_save()));
  m_optpop->addAction("Reload &from...",this,SLOT(Options_loadFrom()));
  m_optpop->addAction("Save &as...",this,SLOT(Options_saveAs()));

  m_anapop = new QMenu("&Analysis", this);
  m_anapop->addAction("Cut &Settings panel",this,SLOT(Cuts_open()));
  m_anapop->addAction("&Edit PDB Form profile",this,SLOT(PDBProf()));
  m_anapop->addAction("&PDB Form for curr. module",this,SLOT(CreatePDBCurrMod()));
  m_anapop->addAction("&QA Report for curr. module",this,SLOT(RunQAReportCurrModule()));
  m_anapop->insertSeparator();
  m_anapop->addAction("Plot &correlation",this,SLOT(PlotCorrel()));
  m_anapop->addAction("&Apply mask for curr. mod.",this,SLOT(PlotMaskedCurrMod()));
  m_anapop->addAction("&Overlay graphs for curr. mod.",this,SLOT(PlotMultiGraphCurrMod()));
  m_anapop->addAction("Compare &bad channels for curr. mod.",this,SLOT(CompareBadChansCurrMod()));
  m_anapop->addAction("&Ibias stability plot for curr. mod.",this,SLOT(BiasStabCurrMod()));
  m_anapop->insertSeparator();
  m_anapop->addAction("Fit all unfitted 1D-hit data",this,SLOT(FitScurves()));
  m_anapop->addAction("Fit all (any!) 1D-hit data",this,SLOT(FitAllScurves()));
  m_anapop->addAction("Fit scan entry in RootDB file",this,SLOT(rootdbFitAll()));
  m_anapop->addAction("Combine 1D-fits to 2D scan",this,SLOT(CombScurves()));
  m_anapop->addAction("Combine hit maps to 1D scan",this,SLOT(CombHitMaps()));
#ifdef MA_DEBUG
  m_anapop->insertSeparator();
  m_anapop->addAction("Memory test",this,SLOT(memoryTest()));
#endif

//****************** added by JW on 02/02/2005 *****************************************************
  m_submenu1 = new QMenu("Plot multiple distributions",this);
  m_submenu1->addAction("Thresholds", this, SLOT(PlotMultiMean()));
  m_submenu1->addAction("Noise", this, SLOT(PlotMultiSig()));
  m_anapop->addMenu(m_submenu1);

  m_submenu2 = new QMenu("Overlay multiple distributions", this);
  m_submenu2->addAction("Thresholds", this, SLOT(OverlayMultiMean()));
  m_submenu2->addAction("Noise", this, SLOT(OverlayMultiSig()));
  m_anapop->addMenu(m_submenu2);

  m_anapop->addAction("Overview plots dialog", this, SLOT(SelectOverview()));
//**************************************************************************************************
  m_anapop->insertSeparator();
  m_anapop->addAction("Check ToT of source data",this,SLOT(CheckToTsrc()));
  m_anapop->setItemEnabled(m_anapop->idAt(2),false);
  m_anapop->setItemEnabled(m_anapop->idAt(3),false);
  m_anapop->setItemEnabled(m_anapop->idAt(5),false);
  m_anapop->setItemEnabled(m_anapop->idAt(6),false);
  m_anapop->setItemEnabled(m_anapop->idAt(7),false);
  m_anapop->setItemEnabled(m_anapop->idAt(8),false);
  m_anapop->setItemEnabled(m_anapop->idAt(9),false);
  m_anapop->setItemEnabled(m_anapop->idAt(10),false);

  m_helppop = new QMenu("&Help", this);
  m_helppop->addAction("Online &user manual",this,SLOT(helpUserRef()));
  m_helppop->addAction("Online &coding help",this,SLOT(helpCoding()));
  m_helppop->addAction("&About MA",this,SLOT(helpAbout()));

  m_menubar->addMenu(m_mainpop);
  m_menubar->addMenu(m_editpop);
  m_menubar->addMenu(m_anapop);
  m_menubar->addMenu(m_optpop);
  m_menubar->addMenu(m_helppop);

  SystestMode = false;

  // process start-up options
  if(options->m_Opts->m_SCmode){
    ChipOrMod->setChecked(true);
    ChipSel->setValue(0);
  }
}

/*  
 *  Destroys the object and frees any allocated resources
 */
TopWin::~TopWin(){
  // destroy all data objects in memory
  ModItem *modit, *datit;
  std::vector<ModItem*> allMods = getAllModuleItems();
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    modit = *IT;
    datit = (ModItem*) modit->firstChild();
    while(datit!=0){
      delete (datit->GetDID());
      datit = (ModItem *) datit->nextSibling();
    }
  }
#ifdef PIXEL_ROD // can link to PixScan
  //obsolete
//   for(std::vector<wrapPs*>::iterator it=m_pixScans.begin();it!=m_pixScans.end();it++)
//     delete (*it);
//   m_pixScans.clear();
#endif
  delete m_fitting;
  delete rootmain; // ROOT object, not auto-destroyed by QT
}


void TopWin::ModChipSel(){
  ChipOrMod2->setChecked( !ChipOrMod->isChecked() );
  return;
}

void TopWin::ModChipSel2(){
  ChipOrMod->setChecked( !ChipOrMod2->isChecked() );
  return;
}

void TopWin::ExitButton_clicked(){
  TCanvas *tmpcan = (TCanvas*) gROOT->FindObject("pixcan");
  if(tmpcan!=0) tmpcan->Close();
  close();
  emit closed();
}

ModItem* TopWin::ModuleButton_clicked(const char *mod_name, ModItem *parent, bool isStave){
  ModItem *item = 0;
  QString text, mod_qname = QString::null;
  if(mod_name!=0)
    mod_qname = mod_name;
  else{
    ModWin getmod(this,"New_item", TRUE, Qt::WType_TopLevel);
    if(isStave){
      getmod.setCaption("New Stave");
      getmod.TextboxLabel->setText("Enter stave/sector name");
    }
    getmod.ModuleName->setFocus();
    if(getmod.exec()==1 && !getmod.ModuleName->text().isEmpty())
      mod_qname = getmod.ModuleName->text();
  }
  if(mod_qname != QString::null){
    if(parent==0)
      item = new ModItem(ModuleList, mod_qname, 0, isStave?STVITEM:MODITEM);
    else
      item = new ModItem(parent, mod_qname, 0, isStave?STVITEM:MODITEM);
    item->setOpen(TRUE);
    ModuleList->setCurrentItem(item);
    if(isStave)
      SetCurrMod(0);
    else
      SetCurrMod((Q3ListViewItem*)item);
    if(mod_name==0) ModuleList_rpr(item);
  }
  if(ModuleList->childCount()==1) // first entry, start sorting
    ModuleList->sort();
  return item;
}
void TopWin::ModuleList_rpr(ModItem* item, int lastPdbID){
  if(item==0) return;
  QString path, messg;
  PixelDataContainer *data=0;
  int seltype=0;
#ifdef PIXEL_ROD // can link to PixScan
  PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
#else
  void *pdbd = 0;
#endif
  textwin->setText("");
  messg = "Data is from ";
  if(item!=0){
    if(item->GetPID()>OPTS && item->GetPID()<DSET){
      messg = "Item refers to ";
      data = item->GetRefItem();
    } else
      data = item->GetDataItem();
  }
  // see if stave/sector items are around
  bool haveStave=false;
  ModItem *stItem = (ModItem*) ModuleList->firstChild();
  while(stItem!=0){
    if(stItem->GetPID()==STVITEM){
      haveStave = true;
      break;
    }
    stItem = (ModItem*) stItem->nextSibling();
  }

  if(item->GetPID()==MODITEM){
    // module level, allow to load and save
    Q3PopupMenu modmenu(this,"Module Menu");
    modmenu.insertItem("&Load new data set",0);
    modmenu.insertItem("Load data files from &list",1);
    modmenu.insertItem("Load standard &file-set",2);
    modmenu.insertItem("&Save all",3);
    modmenu.insertItem("Save &highlighted",4);
    modmenu.insertItem("&QA Report",5);
    modmenu.insertItem("&PDB Form",6);
    modmenu.insertItem("Plot &correlation",7);
    modmenu.insertItem("Apply &Mask",8);
    modmenu.insertItem("&Overlay graphs",9);
    modmenu.insertItem("Compare &bad chan's",10);
    modmenu.insertItem("&Rename",11);
    if(item->GetPID()==MODITEM && item->Parent()!=0)
      modmenu.insertItem("&Set pos. on stave/sector",12);
    else if(item->GetPID()==MODITEM && haveStave)
      modmenu.insertItem("Assign to stave/sector",12);
    else
      modmenu.insertItem("&Set pos. on stave/sector",12);
    modmenu.insertItem("&Delete",13);

    // trivial checks
    if(item->childCount()<=0)
      modmenu.setItemEnabled(3,0);
    if(item->childCount()<1){
      modmenu.setItemEnabled(7,0);
      modmenu.setItemEnabled(8,0);
    }
    if(item->GetPID()!=MODITEM || (item->Parent()==0 && !haveStave))
      modmenu.setItemEnabled(12,0);

    seltype = CheckChilds(item);
    modmenu.setItemEnabled(4,(seltype & 1));
    modmenu.setItemEnabled(9,(seltype & 2));
 
    switch(modmenu.exec(QCursor::pos())){
    case 0:
      LoadData(item);
      item->setOpen(TRUE);
      break;
    case 1:
      LoadList(item);
      item->setOpen(TRUE);
      break;
    case 2:
      LoadStd(item);
      item->setOpen(TRUE);
      break;
    case 3:
      SaveData(item,1);
      break;
    case 4:
      SaveData(item,0);
      break;
    case 5:{
      RunQAReport(item);
      break;}
    case 6:
      CreatePDB(item);
      break;
    case 7:
      PlotCorrel();
      break;
    case 8:
      PlotMasked(item);
      break;
    case 9:
      PlotMultiGraphs(item);
      break;
    case 10:
      CompareBadChans(item);
      break;
    case 11:
      RenameMod(item);
      break;
    case 12:
      if(item->GetPID()==MODITEM && item->Parent()!=0)
        setStavePos(item);
      else if(item->GetPID()==MODITEM && haveStave)
        assignToStave(item);
      break;
    case 13:
      RemoveMod(item);
      break;
    default:
      break;
    }
  } else if(item->GetPID()==STVITEM){
    // stave level - allow to rename, delete and add module
    Q3PopupMenu modmenu(this,"Module Menu");
    modmenu.insertItem("&Add new module",0);
    modmenu.insertItem("&Rename",1);
    modmenu.insertItem("&Delete",2);
    switch(modmenu.exec(QCursor::pos())){
    case 0:
      ModuleButton_clicked(0,item);
      break;
    case 1:
      RenameMod(item);
      break;
    case 2:
      RemoveMod(item);
      break;
    default:
      break;
    }
  } else if(item->GetPID()>=PIXDBTYPES){
#ifdef PIXEL_ROD // can link to PixScan
    int mychip = -1;
    if(ChipOrMod->isChecked())
      mychip = ChipSel->value();
    rootmain->m_chip = ChipSel->value();
    rootmain->m_col  = ColSel->value();
    rootmain->m_row  = RowSel->value();
    // remember last PID and DID
    m_lastPitem = item;
    m_lastPdbID = -1;
    if(lastPdbID<0){
      m_lastPdbFPars.clear();
      m_lastPdbFID = -1;
    }
    m_lastchip  = mychip;
    rootmain->ClearMemory();
    if(rootmain->m_currmap!=0) rootmain->m_currmap->Delete();
    rootmain->m_currmap = 0;

    if(pdbd==0){
      QMessageBox::information(this,"MA: Processing item right-click menu",
			       "Generic PixLib histo plotting not possible for non-PixLib data.");
    } else{
      int PdbID = lastPdbID;
      int sclvl = pdbd->getScanLevel();
      int spnts = pdbd->getScanSteps(pixScanLevel->value());

      std::vector<ModItem *> selIts = item->ModParent()->getAllSelItems();
//       for(std::vector<ModItem *>::iterator IT = selIts.begin(); IT!=selIts.end(); IT++){
// 	if((*IT)->GetPID()>=
//       }

      int nsel = selIts.size();
      bool scanpts=false;
      if(PdbID<0){
	Q3PopupMenu modmenu(this,"PixScan Menu");
	modmenu.insertItem("Plot vs scan par on selected level",0);
	modmenu.insertItem("Plot vs scan par with fit function",1);
	modmenu.insertItem("Plot avg./spread vs scan par on selected level",7);
	modmenu.insertItem("Fit as fct. of var. on selected level",2);
	modmenu.insertItem("Plot ratio/diff./... between two scan points",3);
	modmenu.insertItem("Plot ratio/diff./... between selected items",4);
	modmenu.insertItem("Generate mask from histogram",5);
	modmenu.insertItem("Generate mask from ratio/diff./... between selected items",6);
	modmenu.setItemEnabled(0,(pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
	modmenu.setItemEnabled(1,(pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
	modmenu.setItemEnabled(7,(pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
	modmenu.setItemEnabled(2,(pixScanLevel->value()<sclvl && pixScanLevel->value()>=0));
	modmenu.setItemEnabled(3,(pixScanLevel->value()<sclvl && pixScanLevel->value()>=0 && 
				  Scanpt0->value()<spnts && Scanpt0->value()>=0));
	modmenu.setItemEnabled(4,(nsel==2));
	modmenu.setItemEnabled(5,true);
	modmenu.setItemEnabled(6,(nsel==2));
	PdbID = modmenu.exec(QCursor::pos());
      }
      switch(PdbID){
      case 0:
	plotDBscan(item,false);
	break;
      case 1:
	plotDBscan(item,true);
	break;
      case 7:
	plotDBscan(item,false,true);
	break;
      case 2:{
	if(m_fitting->rootdbFit(item, pixScanLevel->value(), mychip)){
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  //re-read this item
	  ModItem *mod = item->ModParent();
	  ModItem *dat = item->DatParent();
	  bool isOpen = dat->isOpen();
	  DBtoMAData *new_data = new DBtoMAData(item->GetDataItem()->GetName(), 
						(QString(item->GetDataItem()->GetFullPath())+
						 QString(item->GetDataItem()->GetPath())).latin1(),
						mod->text(0).latin1());
	  RemData(dat, false);
	  item = DataToTree(mod, new_data);
	  item->setOpen(isOpen);
	  QApplication::restoreOverrideCursor();
	}
	break;}
      case 3:
	scanpts=true;
      case 4:{
	PixDBData *pdbd1 = dynamic_cast<PixDBData*>(selIts[0]->GetDataItem());
	PixDBData *pdbd2 = dynamic_cast<PixDBData*>((scanpts?selIts[0]:selIts[1])->GetDataItem());
	if(pdbd1==0 || pdbd2==0) QMessageBox::warning(this,"Module Analysis","Can't get data");
	// ask user for type of operation
	RefWin rw(item, this,"Reference Data Set", TRUE, Qt::WType_TopLevel, 100);
	std::map<std::string, int> pdb_oper = pdbd1->getOperTypes();
	for(std::map<std::string, int>::iterator IT=pdb_oper.begin(); IT!=pdb_oper.end();IT++)
	  rw.AddToList(IT->first.c_str(),IT->second);
	rw.setCaption("Options for plotting");
 	if(!scanpts){
	  rw.SelectorLabel->hide();
	  rw.DatSel->hide();
	} else {
	  rw.SelectorLabel->setText("Select 2nd scan point (1st is "+QString::number(Scanpt0->value())+")");
	  for(int is=0;is<spnts;is++)
	    if(is!=Scanpt0->value()) rw.DatSel->insertItem(QString::number(is));
	}
	rw.OKButt->setEnabled(true);
	if(rw.exec()==QDialog::Accepted){
	  int ps_type1 = selIts[0]->GetPID()-PIXDBTYPES;
	  int ps_type2 = (scanpts?selIts[0]:selIts[1])->GetPID()-PIXDBTYPES;
 	  int error, pt2=-1;
 	  if(scanpts)
 	    pt2 = rw.DatSel->currentText().toInt();
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  pdbd1->getOperHisto((PixLib::PixScan::HistogramType)ps_type1, scanpts?0:pdbd2, (PixLib::PixScan::HistogramType)ps_type2, 
			      (PixDBData::OperType) rw.func, mychip, 0, 0, Scanpt0->value(), pt2, pixScanLevel->value());
	  TH2F *hi = pdbd1->getChipMap(mychip);
	  // make copy of histo to be independent of any internal cleaning of pdbd1
	  gROOT->cd(); // make sure histo is created in memory, not file
	  hi = new TH2F(*hi);
	  hi->SetName((std::string(hi->GetName())+"_2").c_str());
	  if(hi!=0){
	    error = rootmain->PlotMapProjScat(selIts[0], mychip, -1, hi);
	    if(rootmain->m_currmap!=0){
	      rootmain->m_currmap->SetWidget(this);
	      rootmain->m_currmap->m_dodclk=false;
	    }
	    UpdatePixVal();
	    // don't need histo any longer
	    hi->Delete();
	    QApplication::restoreOverrideCursor();
	    if(error){
	      messg.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
	      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",messg);
	    }
	  } else
	    QMessageBox::warning(this,"Module Analysis","Error while generating result histogram");
	}
	break;}
      case 5:{
	PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
	if(pdbd==0) QMessageBox::warning(this,"Module Analysis","Can't get data");
 	int ps_type = item->GetPID()-PIXDBTYPES;
 	MaskMaker mm(this);
 	mm.setHistoLabel(ps_type);
	if(mm.exec()==QDialog::Accepted){
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  PixLib::Histo *h = pdbd->GenMask((PixLib::PixScan::HistogramType)ps_type, Scanpt0->value(), pixScanLevel->value(), 
					   mm.getMin(),mm.getMax());
	  if(h!=0){
	    // store mask in file and remove histo from memory
	    pdbd->writePixLibHisto(h, PixLib::PixScan::HITOCC, 0,0);
	    delete h;
	    //re-read this item
	    ModItem *mod = item->ModParent();
	    ModItem *dat = item->DatParent();
	    bool isOpen = dat->isOpen();
	    DBtoMAData *new_data = new DBtoMAData(item->GetDataItem()->GetName(), 
						  (QString(item->GetDataItem()->GetFullPath())+
						   QString(item->GetDataItem()->GetPath())).latin1(),
						  mod->text(0).latin1());
	    RemData(dat, false);
	    item = DataToTree(mod, new_data);
	    item->setOpen(isOpen);
	    ModuleList->setSelectionMode(Q3ListView::NoSelection);
	    ModuleList->setSelectionMode(Q3ListView::Extended);
	    for(ModItem *it = (ModItem*)item->firstChild();it!=0;it = (ModItem*)it->nextSibling()){
	      if(PixLib::PixScan::HITOCC == (PixLib::PixScan::HistogramType)(it->GetPID()-PIXDBTYPES)){
		it->setSelected(true);
		ModuleList_itemselected(it);
		break;
	      }
	    }
	    QApplication::restoreOverrideCursor();
	  } else{
	    QApplication::restoreOverrideCursor();
	    QMessageBox::warning(this,"Module Analysis","Mask making didn't return valid Histo object");
	  }
	}
	break;}
      case 6:{
	PixDBData *pdbd1 = dynamic_cast<PixDBData*>(selIts[0]->GetDataItem());
	PixDBData *pdbd2 = dynamic_cast<PixDBData*>(selIts[1]->GetDataItem());
	if(pdbd1==0 || pdbd2==0) QMessageBox::warning(this,"Module Analysis","Can't get data");
	// ask user for type of operation
	RefWin rw(item, this,"Reference Data Set", TRUE, Qt::WType_TopLevel, 100);
	std::map<std::string, int> pdb_oper = pdbd1->getOperTypes();
	for(std::map<std::string, int>::iterator IT=pdb_oper.begin(); IT!=pdb_oper.end();IT++)
	  rw.AddToList(IT->first.c_str(),IT->second);
	rw.setCaption("Options for plotting");
	rw.SelectorLabel->hide();
	rw.DatSel->hide();
	rw.OKButt->setEnabled(true);
	if(rw.exec()==QDialog::Accepted){
	  int ps_type1 = selIts[0]->GetPID()-PIXDBTYPES;
	  int ps_type2 = selIts[1]->GetPID()-PIXDBTYPES;
	  MaskMaker mm(this);
	  mm.histoLabel->setText(rw.FuncSel->currentText());
	  if(mm.exec()==QDialog::Accepted){
	    QApplication::setOverrideCursor(Qt::waitCursor);
	    PixLib::Histo *h = pdbd1->GenMask((PixLib::PixScan::HistogramType)ps_type1, pdbd2, (PixLib::PixScan::HistogramType)ps_type2, 
					     (PixDBData::OperType) rw.func, Scanpt0->value(), pixScanLevel->value(), 
					     mm.getMin(),mm.getMax());
	    if(h!=0){
	      // store mask in file and remove histo from memory
	      pdbd1->writePixLibHisto(h, PixLib::PixScan::HITOCC, 0,0);
	      delete h;
	      //re-read this item
	      ModItem *mod = selIts[0]->ModParent();
	      ModItem *dat = selIts[0]->DatParent();
	      bool isOpen = dat->isOpen();
	      DBtoMAData *new_data = new DBtoMAData(selIts[0]->GetDataItem()->GetName(), 
						    (QString(selIts[0]->GetDataItem()->GetFullPath())+
						     QString(selIts[0]->GetDataItem()->GetPath())).latin1(),
						    mod->text(0).latin1());
	      RemData(dat, false);
	      item = DataToTree(mod, new_data);
	      item->setOpen(isOpen);
	      ModuleList->setSelectionMode(Q3ListView::NoSelection);
	      ModuleList->setSelectionMode(Q3ListView::Extended);
	      for(ModItem *it = (ModItem*)item->firstChild();it!=0;it = (ModItem*)it->nextSibling()){
		if(PixLib::PixScan::HITOCC == (PixLib::PixScan::HistogramType)(it->GetPID()-PIXDBTYPES)){
		  it->setSelected(true);
		  ModuleList_itemselected(it);
		  break;
		}
	      }
	      QApplication::restoreOverrideCursor();
	    } else{
	      QApplication::restoreOverrideCursor();
	      QMessageBox::warning(this,"Module Analysis","Mask making didn't return valid Histo object");
	    }
	  }
	}
	break;}
      default:
	break;
      }
    }
  } else if(pdbd!=0 && item->GetPID()==IVPLOT){ // PixDBData graph item
    Q3PopupMenu modmenu(this,"PixScan Menu");
    modmenu.insertItem("Fit graph",0);
    modmenu.insertItem("Fit graph and plot residual graph",1);
    modmenu.insertItem("Fit graph and histogram residual",2);
    modmenu.insertItem("Plot and fit FE-I3-CapMeasure graphs separately",3);
    // CapMeasure plot style only makes sense if this type of scan was done
    if(pdbd->getScanPar(0)!="CAPMEAS") modmenu.setItemEnabled(3,false);
    int ichosen = modmenu.exec(QCursor::pos());
    if(ichosen>=0 && ichosen<3){
      // reset calib matters in fitwindow
      m_FitWin->ChipCalBox->setChecked(true);
      m_FitWin->ChipCalBox->setEnabled(true);
      m_FitWin->CfacEdit->setEnabled(true);
      m_FitWin->ChipCfac->setEnabled(true);
      m_FitWin->LoadButt->setEnabled(true);
      m_FitWin->autoCalNote->hide();
      m_FitWin->autoCalNote2->hide();
      if(m_FitWin->exec()){
	// run fit
	int error;
	QString tmp;
	textwin->setText("Performing fit now, please be patient...");
	textwin->repaint();
	if(m_app!=0) m_app->processEvents();
	QApplication::setOverrideCursor(Qt::waitCursor);
	gSystem->Sleep(500);
	double xmin=0., xmax=-1.;
	if(m_FitWin->FitXmin->value()!= m_FitWin->FitXmax->value()){
	  xmin = (double) m_FitWin->FitXmin->value();
	  xmax = (double) m_FitWin->FitXmax->value();
	}
	if((error=rootmain->FitGraph(m_FitWin->FitType->currentItem(), item, ichosen, xmin, xmax))){
	  tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
	  textwin->setText("");
	  QApplication::restoreOverrideCursor();
	  if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
	} else{
	  textwin->setText("");
	  QApplication::restoreOverrideCursor();
	}
      }
    } else if (ichosen==3){
      int error;
      if ((error=rootmain->PlotCM3Graph(item))!=0){
	QString tmp;
	tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
	if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
      }
    }
#endif
  } else{
    // data level: path info, remove, rename
    path = messg + item->GetDataItem()->GetFullPath() + " .. pointer 0x" + QString::number((long int)data,16);
    textwin->setText(path);

    if((item->GetPID() > DSET  && item->GetPID()<PIXDBTYPES)|| item->GetRID()!=0){
      // popup menu
      Q3PopupMenu popmenu(this,"Data set menu");
      popmenu.insertItem("Rename",0);
      popmenu.insertItem("Delete",1);
      popmenu.insertItem("Remove raw",2);
      if(item->GetPID()==BRIEFSUM || item->GetPID()==BRIEFDIG)
        popmenu.insertItem("Retrieve org. data",3);
      else
        popmenu.insertItem("Reduce to summary",3);
      popmenu.insertItem("Assign test-ID",4);
      popmenu.setItemEnabled(2,(item->GetPID()==ONEDFIT || item->GetPID()==TOTFR));
      popmenu.setItemEnabled(3,(item->GetPID()==ONEDFIT || item->GetPID()==ONEDSCAN || 
                                item->GetPID()==DIG || 
                                item->GetPID()==BRIEFSUM || item->GetPID()==BRIEFDIG));
      popmenu.setItemEnabled(4,(item->GetPID() > DSET && item->GetPID()<PIXDBTYPES));
      switch(popmenu.exec(QCursor::pos())){
      case 0:{
        RenameWin rnw(this," ", TRUE, Qt::WType_TopLevel);
        rnw.Name->validateAndSet(item->text(0),item->text(0).length(),
                                 0,item->text(0).length());
        rnw.Name->setFocus();
        if(rnw.exec()){
          item->setText(0,rnw.Name->text());
          if(item->GetPID() > DSET && item->GetPID() < PIXDBTYPES) // only change name of data if this is a data item
            item->GetDataItem()->ChangeName(rnw.Name->text().latin1());
        }
        break;}
      case 1:
        if(item->GetRID()==0) RemData(item);
        else                  delete item;
        break;
      case 2:
        RemRaw(item);
        break;
      case 3:
        if(item->GetPID()==BRIEFSUM || item->GetPID()==BRIEFDIG)
          RetrieveOrgData(item);
        else
          RemFit(item);
        break;
      case 4:
        MakeStd(item);
        break;
      default:
        break;
      }
    }
  }
  return;
}

void TopWin::ModuleList_itemselected(ModItem* item){
  if(item==0) return;
  ModItem *plotit, *datit, *parnt, *test1, *test2;
  QString tmp;
  int error, chip=-1;
  if(ChipOrMod->isOn())
    chip = ChipSel->value();
  rootmain->m_chip = ChipSel->value();
  rootmain->m_col  = ColSel->value();
  rootmain->m_row  = RowSel->value();
  //item->setOpen(TRUE);
  if(item->GetPID()<=MODITEM || item->GetPID()==NONE || 
     (item->GetPID()>DSET && item->GetPID()<PIXDBTYPES) ||
     item->ModParent()==0) return;
  datit = item->DatParent();
  parnt = item->ModParent();
  /*
  while(parnt->Parent()!=0){ // find highest level item (=module)
    datit = parnt;              // the actual data set item
    parnt = parnt->Parent();    // and the module item
  }
  */
  // remember last PID and DID
  m_lastPitem = item;
  m_lastPdbID = -1;
  m_lastPdbFPars.clear();
  m_lastPdbFID = -1;
  m_lastchip  = chip;
  rootmain->ClearMemory();
  if(rootmain->m_currmap!=0) rootmain->m_currmap->Delete();
  rootmain->m_currmap = 0;

  // plotting
  textwin->setText("");
  switch(item->GetPID()){
  case PSCONFIG:{
    if(item->GetDataItem()==0) break;
#ifdef PIXEL_ROD // can link to PixScan
    DatSet *maData = dynamic_cast<DatSet*>(item->GetDataItem());
    DBtoMAData *plData = dynamic_cast<DBtoMAData*>(item->GetDataItem());
    if(maData!=0){
      PixLib::PixScan *ps=0;
      if((ps=item->GetDID()->getPixScan())!=0){
	QApplication::setOverrideCursor(Qt::waitCursor);
	configViewer* cv = new configViewer(ps->config(),this,"scancfgdisp",FALSE, Qt::WDestructiveClose);
	//cv->setCaption("PixScan config viewer");
	QApplication::restoreOverrideCursor();
	cv->show();
      }
    } else if(plData!=0){
      QApplication::setOverrideCursor(Qt::waitCursor);
      configViewer* cv = new configViewer(plData->getScanConfig(),this,
					  "scancfgdisp",FALSE, Qt::WDestructiveClose);
      QApplication::restoreOverrideCursor();
      cv->show();
    }
#endif
    break;}
  case PMCONFIG:{
    if(item->GetDataItem()==0) break;
#ifdef PIXEL_ROD // can link to PixScan
    DBtoMAData *plData = dynamic_cast<DBtoMAData*>(item->GetDataItem());
    if(plData!=0){
      QApplication::setOverrideCursor(Qt::waitCursor);
      configViewer* cv = new configViewer(plData->getModConfig(),this,
					  "modcfgdisp",FALSE, Qt::WDestructiveClose);
      QApplication::restoreOverrideCursor();
      std::string ctrlInfo = plData->getCtrlInfo();
      if(ctrlInfo!=""){
	QWidget *TabPage = new QWidget( cv->m_tabwidget, "TabPageInfo" );
	QLabel *qtxt = new QLabel(TabPage);
	qtxt->setText(ctrlInfo.c_str());
	cv->m_tabwidget->insertTab( TabPage,  "ControllerInfo");
	//	printf("USBPix Info: %s\n", ctrlInfo.c_str());
      }
      cv->show();
    }
#endif
    break;}
  case PBCONFIG:{
    if(item->GetDataItem()==0) break;
#ifdef PIXEL_ROD // can link to PixScan
    DBtoMAData *plData = dynamic_cast<DBtoMAData*>(item->GetDataItem());
    if(plData!=0){
      QApplication::setOverrideCursor(Qt::waitCursor);
      configViewer* cv = new configViewer(plData->getBocConfig(),this,
					  "boccfgdisp",FALSE, Qt::WDestructiveClose);
      QApplication::restoreOverrideCursor();
      cv->show();
    }
#endif
    break;}
  case LOGFILE:{
    if(item->GetDID()==0) break;
    bool showScanCfg=true;
#ifdef PIXEL_ROD // can link to PixScan
    //    showScanCfg=(item->GetDID()->getPixScan()==0);
#endif
    LogDisp *ldsp = new LogDisp(this,"Log File Display",FALSE, Qt::WDestructiveClose,item,options->m_Opts,
				showScanCfg);
    QString tmpstr;
    tmpstr.sprintf("Log File for data set \"%s\"",item->GetDID()->GetName());
    ldsp->LabelLabel->setText(tmpstr);
    ldsp->FileLabel->setText(item->GetDID()->GetPath());
    ldsp->show();//exec();
    break;}
  // register test: just open a panel for now
  case REGPANEL:{
    RegPanel rp(item, this,"regpanel");
    rp.exec();
    break;}
  // for the profile histos
  case TWLK_NORMAL_OVPROF:
  case TWLK_LONG_OVPROF:
  case TWLK_GANGED_OVPROF:
  case TWLK_LGANGED_OVPROF:
  case TWLK_ALL_OVPROF:
    textwin->setText("");
    if ((error=rootmain->PlotProfile(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case TOGMASK:{
    int binx, biny;
    item->GetDID()->PixCCRtoBins(ChipSel->value(),ColSel->value(),RowSel->value(),binx,biny);
    float con;
    TH2F *map = item->GetDID()->GetMap(-1,MAMASK);
    if(map==0)
      if(options->m_Opts->m_showwarn) 
        QMessageBox::warning(this,"Module Analysis","No mask available for this item");
    con = map->GetBinContent(binx,biny);
    if(con) con=0;
    else    con=1;
    map->SetBinContent(binx,biny,con);
  }
  case CLTSIMP:
  case TOT_HIT:
  case MAMASK:
  case RMASKP:
  case HMASKP:
  case SMASKP:
  case PMASKP:
  case INJCAPH:
  case INJCAPL:
  case ILEAK:
  case DEAD_CHAN:
    textwin->setText("");
    if ((error=rootmain->Plot2DColour(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case BADSEL:{
    textwin->setText("");
    bool use_defcuts=false;
    int sel_cuts=0;
    if(m_cutpanel->ClassList->childCount()<=0){
      m_cutpanel->NewClass();
      m_cutpanel->RenameEdit->setText("temp cuts");
      m_cutpanel->RenameClass();
      use_defcuts = true;
    }
    SelectionPanel selpan(this,"selpan");
    for(int i=0; i<NCUTS;i++){
      selpan.AddCheckBox(m_cutpanel->GetCutPars(i)->label.c_str());
    }
    if((sel_cuts=selpan.exec())>=0){
      if ((error=rootmain->PlotBadSel(item, chip, sel_cuts, selpan.buttonANDOR->isOn()))){
        tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
      }
    }
    if(use_defcuts) // if we created default cuts because nothing was there we have to clean up
      m_cutpanel->DeleteClass();
    break;}
  case MASKTAB:
  case DEADTAB:
    textwin->setText("");
    QApplication::setOverrideCursor(Qt::waitCursor);
    rootmain->MaskTable(item, chip);
    QApplication::restoreOverrideCursor();
    break;
  case TOT_DIST:
  case TOT_FDIS:
  case CAL_DIST:
  case CAL_FDIS:
  case TOTRAW:
  case TOTSD:
  case RAWSLC:
  case TWODRAW:
  case TWODRAWS:
  case TWODRAWC:
    textwin->setText("");
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->PlotMapScanhi(item, chip, Scanpt0->value()))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    QApplication::restoreOverrideCursor();
    break;
  case TWODMINT:
    textwin->setText("");
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->PlotMinThresh(item, chip))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    } else if(item->Parent()->findPlotItem(TDACP)==0 && item->GetDataItem()->GetMap(-1,TDACLOG)!=0)
      new ModItem(item->Parent(),"Plot TDAC at min. threshold",item->GetDataItem(),TDACP);
    QApplication::restoreOverrideCursor();
    break;
  case TWODMINTN:
    textwin->setText("");
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->PlotMinThresh(item, chip, SIG))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    } else if(item->Parent()->findPlotItem(TDACP)==0 && item->GetDataItem()->GetMap(-1,TDACLOG)!=0)
      new ModItem(item->Parent(),"Plot TDAC at min. threshold",item->GetDataItem(),TDACP);
    QApplication::restoreOverrideCursor();
    break;
  case TWODMINTC:
    textwin->setText("");
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->PlotMinThresh(item, chip, CHI))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    } else if(item->Parent()->findPlotItem(TDACP)==0 && item->GetDataItem()->GetMap(-1,TDACLOG)!=0)
      new ModItem(item->Parent(),"Plot TDAC at min. threshold",item->GetDataItem(),TDACP);
    QApplication::restoreOverrideCursor();
    break;
  case DIGSUMM:
    if((error=rootmain->nBadPixs(item,true))<0){
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis","Error "+QString::number(error)+" creating summary");
    }    
    break;
  case THSUMM:
    if(rootmain->ThreshSumm(item)==0){
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis","Error creating summary");
    }    
    break;
  case ITHSUMM:
    if(rootmain->ITHSumm(item)==0){
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis","Error creating summary");
    }    
    break;
  case CAL_TAB:{
    RenameWin getnom(this,"getnom", TRUE, Qt::WType_TopLevel);
    getnom.EnterLabel->setText("Enter nominal position (e):");
    getnom.Name->validateAndSet("16600",9,0,9);
    getnom.Name->setFocus();
    getnom.exec();
    bool isok;
    float nom_pos = getnom.Name->text().toFloat(&isok);
    if(!isok) nom_pos=16600;
    if(rootmain->PeakSumm(item,-1,nom_pos)==0){
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis","Error creating summary");
    }    
    break;}
  case PARCOMB:{
    RenameWin rnw(this," ", TRUE, Qt::WType_TopLevel);
    rnw.Name->validateAndSet("[0]+2*[1]",9,0,9);
    rnw.Name->setFocus();
    if(rnw.exec()){    
      // do analysis
      rootmain->ParComb(rnw.Name->text().latin1(), item->GetDID());
    }}
    // then plot like the others
    textwin->setText("");
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->PlotMapProjScat(item, chip, Scanpt0->value()))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    QApplication::restoreOverrideCursor();
    break;
  case TOT_SUMM:
  case CAL_SUMM:
    if(item->text(0)=="Run cal. peak fit (slow!)" || item->text(0)=="Run peak fit (slow!)"){
      textwin->setText("Fitting spectra of all pixels, might take some time...");
      textwin->repaint();
      if(m_app!=0)
        m_app->processEvents();
      if(item->GetPID()==CAL_SUMM)
        item->setText(0,"Plot position of cal. peak & distribution (fitted)");
      else
        item->setText(0,"Plot position of peak & distribution (fitted)");
    }
  case TOT_FAST:
  case CAL_FAST:
    if(item->GetDID()->GetMap(-1,item->GetPID())==0 || options->m_Opts->m_srcRecalcPeak){
      QApplication::setOverrideCursor(Qt::waitCursor);
      if(options->m_Opts->m_srcRestrict)
        item->GetDID()->PeakAna((item->GetPID()==TOT_SUMM || item->GetPID()==CAL_SUMM), item->GetRID(),
                                options->m_Opts->m_peakMinHit, (float)options->m_Opts->m_srcMin);
      else
        item->GetDID()->PeakAna((item->GetPID()==TOT_SUMM || item->GetPID()==CAL_SUMM), item->GetRID());
      QApplication::restoreOverrideCursor();
      bool need_entry = true;
      test1 = (ModItem*) item->parent()->firstChild();
      while(test1!=0){
        if(test1->text(0)=="Table of avg. position of peak (fast ana)"){
          need_entry=false;
          break;
        }
        test1 = (ModItem*) test1->nextSibling();
      }
      if(need_entry)
        new ModItem(item->Parent(), "Table of avg. position of peak (fast ana)",item->GetDID(),CAL_TAB); 
    }
  case RAW:{
    QApplication::setOverrideCursor(Qt::waitCursor);
    if(options->m_Opts->m_occPlotHisto)
      error=rootmain->PlotMapProjScat(item, chip, Scanpt0->value());
    else
      error=rootmain->PlotMapScanhi(item, chip, Scanpt0->value());
    if(error){
      textwin->setText("");
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    textwin->setText("");
    QApplication::restoreOverrideCursor();
    break;}
  case AVTOT:
  case SIGTOT:
  case MEAN:
  case SIG:
  case CHI:
  case T20K:
  case TDACP:
  case FDACP:
  case TDACLOG:
  case FDACLOG:
  case LEAK_PLOT:
  case MLPLOT:
  case TIME:
  case TWLK_TIME20NS_2D:
  case TWLK_Q20NS_2D:
  case TWLK_OVERDRV_2D:
  case TWODMEAN:
  case TWODSIG:
  case TWODCHI:
  case PARA:
  case PARB:
  case PARC:
  case PARD:
  case PARE:
  case ITH:
  case TOT_MAX:
  case TOT_RATE:
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->PlotMapProjScat(item, chip, Scanpt0->value()))){
//      if((error=rootmain->PlotSingleProjection(item, chip, Scanpt0->value()))){                        //by JW
      textwin->setText("");
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    textwin->setText("");
    QApplication::restoreOverrideCursor();
    break;
  case LOOSE:
  case SHORT:
  case XTFR:
  case XTNOISE:
  case NRAT:
  case TRAT:
  case NDIF:
  case NSQDIF:
  case TDIF:
  case TIMEDIF:
  case TCALDIF:
  case DELCAL:
  case MINTDIF:
  case MINTRAT:
    if((error=rootmain->PlotComp(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case TDNRAT:
  case TDTRAT:
  case TDNDIF:
  case TDTDIF:
    if((error=rootmain->Plot2DComp(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case PLTCCAL:
    if((error=rootmain->PlotCombCal(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case PLTCCPR:
    if((error=rootmain->PlotCombCalProj(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case CLTOT:
    if((error=rootmain->PlotTOT(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case IVPLOT:
  case IVCORR:{
    int scanpt[3]= {Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
    int pslevel = pixScanLevel->value();
    if(dynamic_cast<PixDBData*>(item->GetDataItem())==0){
      for(int is=0;is<3;is++) scanpt[is] = -1;
      pslevel = -1;
    }
    if((error=rootmain->PlotGraph(item, (pslevel>=0)?scanpt[pslevel]:0, pslevel))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;}
  case TWLKGR:
    if((error=rootmain->TwalkGraphs(item, chip))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
    
  case GETMASK:{
    bool doit=true, newmask=(item->GetDID()->GetMap(-1,MAMASK)==0);
    if(!newmask){
      QMessageBox ays("Delete item?", 
                      "This will overwrite the existing mask - continue?",
                      QMessageBox::Warning,QMessageBox::Yes, QMessageBox::No, Qt::NoButton,
                      this, "ays",TRUE,Qt::WType_TopLevel);
      if(ays.exec()==QMessageBox::No)
        doit = false;
    }
    if(!doit) break;
    QStringList filter("Mask file (*.dat)");
    filter += "Any file (*.*)";
    Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
    fdia.setFilters(filter);
    if(fdia.exec() == QDialog::Accepted){
      QApplication::setOverrideCursor(Qt::waitCursor);
      if((error=item->GetDID()->ReadMaskSet(fdia.selectedFile().latin1()))){
        tmp.sprintf("Error: %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
        QApplication::restoreOverrideCursor();
      } else {
        if(newmask){
          new ModItem(item->Parent(),"Plot map of masked pixels",     item->GetDID(),MAMASK);
          new ModItem(item->Parent(),"Toggle selected pixel in mask", item->GetDID(),TOGMASK);
          new ModItem(item->Parent(),"Write mask to file",            item->GetDID(),SAVEMASK);
          new ModItem(item->Parent(),"Apply mask to all other scans", item->GetDID(),APPLMASK);
        }
      }
      QApplication::restoreOverrideCursor();
    }
    break;}
  case APPLMASK:{
    bool doit=true, newmask=(item->GetDID()->GetMap(-1,MAMASK)==0);
    if(newmask){
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"TopWin::ApplyMask","Item has no mask data, can't continue.");
      break;
    }
    ModItem *otherScans = (ModItem*) item->ModParent()->firstChild();
    while(otherScans!=0){
      if(otherScans!=item->DatParent() && otherScans->GetDID()->m_istype==DIG &&
         otherScans->GetDID()->GetLog()->MyModule->GetInjection()=="Digital" && 
         otherScans->GetDID()->m_isscan>0){
        doit=true;
        newmask=(otherScans->GetDID()->GetMap(-1,MAMASK)==0);
        if(!newmask){
          QMessageBox ays("Delete item?", 
                          "This will overwrite the existing mask for data\n"+
                          otherScans->text(0) + " - continue?",
                          QMessageBox::Warning,QMessageBox::Yes, QMessageBox::No, Qt::NoButton,
                          this, "ays",TRUE,Qt::WType_TopLevel);
          if(ays.exec()==QMessageBox::No)
            doit = false;
        }
        if(doit){
          QApplication::setOverrideCursor(Qt::waitCursor);
          textwin->setText("Copying mask to "+otherScans->text(0));
          textwin->repaint();
          if(m_app!=0)
            m_app->processEvents();
          otherScans->GetDID()->CreateMask(item->GetDID()->GetMap(-1,MAMASK),0);
           if(newmask){
            ModItem *excPlot = otherScans->findPlotItem("Excess/missing hits");
            if(excPlot!=0){
              new ModItem(excPlot,"Plot map of masked pixels",     otherScans->GetDID(),MAMASK);
              new ModItem(excPlot,"Toggle selected pixel in mask", otherScans->GetDID(),TOGMASK);
              new ModItem(excPlot,"Write mask to file",            otherScans->GetDID(),SAVEMASK);
              new ModItem(excPlot,"Apply mask to all other scans", otherScans->GetDID(),APPLMASK);
            }
           }
          QApplication::restoreOverrideCursor();
        }
      }
      otherScans = (ModItem*) otherScans->nextSibling();
    }
    break;}
  case DIGEMP:{
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->DigExcMiss(item, chip, 0))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    bool newmask=(item->GetDID()->GetMap(-1,MAMASK)!=0 && 
                  ((ModItem*)item->parent())->findPlotItem("Plot map of masked pixels")==0);
    if(newmask){
      new ModItem(item->Parent(),"Plot map of masked pixels",     item->GetDID(),MAMASK);
      new ModItem(item->Parent(),"Toggle selected pixel in mask", item->GetDID(),TOGMASK);
      new ModItem(item->Parent(),"Write mask to file",            item->GetDID(),SAVEMASK);
      new ModItem(item->Parent(),"Apply mask to all other scans", item->GetDID(),APPLMASK);
    }
    QApplication::restoreOverrideCursor();
    break;}
  case DIGEMPR:{
    // prompt user for min. voltage
    RenameWin getvolts(this,"getvolts", TRUE, Qt::WType_TopLevel);
    getvolts.EnterLabel->setText("Enter min. voltage (mV):");
    getvolts.Name->validateAndSet("2000",4,0,4);
    getvolts.Name->setFocus();
    getvolts.exec();    
    bool isok;
    float volts = getvolts.Name->text().toFloat(&isok);
    if(!isok) volts=0;
    // plot
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->DigExcMiss(item, chip, 0, volts))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    bool newmask=(item->GetDID()->GetMap(-1,MAMASK)!=0 && 
                  ((ModItem*)item->parent())->findPlotItem("Plot map of masked pixels")==0);
    if(newmask){
      new ModItem(item->Parent(),"Plot map of masked pixels", item->GetDID(),MAMASK);
      new ModItem(item->Parent(),"Toggle selected pixel in mask", item->GetDID(),TOGMASK);
      new ModItem(item->Parent(),"Write mask to file",            item->GetDID(),SAVEMASK);
      new ModItem(item->Parent(),"Apply mask to all other scans", item->GetDID(),APPLMASK);
    }
    QApplication::restoreOverrideCursor();
    break;
  }
  case DIGEMN:{
    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error=rootmain->DigExcMiss(item, chip, 1))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    bool newmask=(item->GetDID()->GetMap(-1,MAMASK)!=0 && 
                  ((ModItem*)item->parent())->findPlotItem("Plot map of masked pixels")==0);
    if(newmask){
      new ModItem(item->Parent(),"Plot map of masked pixels", item->GetDID(),MAMASK);
      new ModItem(item->Parent(),"Toggle selected pixel in mask", item->GetDID(),TOGMASK);
      new ModItem(item->Parent(),"Write mask to file",            item->GetDID(),SAVEMASK);
      new ModItem(item->Parent(),"Apply mask to all other scans", item->GetDID(),APPLMASK);
    }
    QApplication::restoreOverrideCursor();
    break;}
  case REFD:{
    // make sure there's more than one data set
    int refforwhat = 0;
    if(datit->GetDID()->m_istype==TWODSCAN || datit->GetDID()->m_istype==TWODFIT)
      refforwhat = 7;
    RefWin rdw(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel, refforwhat);
    if(rdw.DatSel->count()<=0)
     textwin->setText("Load more than this sfit-data set to enable options");
    else{
      // open selection window and get reference data set
      if(rdw.exec()==1){
        if(rdw.selit==0)
          textwin->setText("selit-pointer is 0");
        else{
          tmp = "Plot " + rdw.FuncSel->currentText();
          if(refforwhat==0)
            plotit = new ModItem(datit,tmp, item->GetDID(),
                                 rdw.func, rdw.selit->GetDID());
          else
            plotit = new ModItem(item->Parent(),tmp, item->GetDID(),
                                 rdw.func, rdw.selit->GetDID());
        }
      }
    }
    break;}
  case TWODOPT:{
    int acttype=NONE;
    if(datit->GetDID()->GetLog()==0){ // find type of scans
      RefWin rdw(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel,4);
      // open selection window and get action type
      if(rdw.exec()==1) acttype = rdw.func;
    } else{
      if(datit->GetDID()->GetLog()->MyScan->GetOuter()=="All TDACs")
        acttype = TFDAC;
      else if((datit->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" ||
               datit->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff")&&
        (datit->GetDID()->GetLog()->MyScan->GetOuter()=="DAC 8" ||
         datit->GetDID()->GetLog()->MyScan->GetOuter()=="DAC 7"))
        acttype = ITHSCAN;
      else if(datit->GetDID()->GetLog()->MyScan->GetOuter()=="VCal FE" &&
        (datit->GetDID()->GetLog()->MyScan->GetInner()=="MCC strobe delay"   ||
         datit->GetDID()->GetLog()->MyScan->GetInner()=="Strobe LE & TE" )&&
        !datit->GetDID()->GetLog()->MyModule->CHigh)
        acttype = TWLK;
    }
    switch(acttype){
    case NONE:
      textwin->setText("no action selected");
      break;
    case TFDAC:{
      ModItem *temp, *temp2=0;
      int error;
      float fixed_target=-1;
      // fit if requested
      if((datit->GetDID()->m_istype==TWODSCAN || !options->m_Opts->m_rec2DFit) && options->m_Opts->m_DACdoFit){
        m_FitWin->FitType->setCurrentItem(5);
        m_FitWin->CfacEdit->setText("1.0");
        m_FitWin->SetPar[0]->setText("3000");
        m_FitWin->SetPar[1]->setText("6000");
        m_FitWin->SetPar[2]->setText("0.05");
        m_FitWin->FixPar[3]->setChecked(true);
        if(item->GetDID()->GetLog()!=0 &&
           item->GetDID()->GetLog()->MyModule->FEflavour==0) // FE-I1
          m_FitWin->SetPar[3]->setText("31");// force to end of TDAC range
        else  // FE-I2
          m_FitWin->SetPar[3]->setText("64");// force to centre of TDAC range
	int chip_s=-1, chip_e=-1;
	if(ChipOrMod->isOn()){
	  chip_s = ChipSel->value();
	  chip_e = ChipSel->value();
	}
        datit = MAEngine::PerformFit(item,false,true,true,this,m_app, chip_s, chip_e);
        if(datit==0) break;
        //temp = new ModItem(datit,"temp fit item",datit->GetDID(),FIT);
        //ModuleList_itemselected(temp);
        //datit = temp;
        // delete temp;  already deleted by fitting
      } 
      // find TDAC corresponding to target
      int fittype = datit->GetDID()->m_fittype;
      if(!options->m_Opts->m_DACdoFit)
        datit->GetDID()->m_fittype = -1;
      QApplication::setOverrideCursor(Qt::waitCursor);
      if(options->m_Opts->m_doFixedTarget)
        fixed_target = options->m_Opts->m_ChipTarget;
      error = datit->GetDID()->TDACAna(options->m_Opts->m_doChipTarget, fixed_target);
      if(!options->m_Opts->m_DACdoFit)
        datit->GetDID()->m_fittype = fittype;
      if(error){
        QApplication::restoreOverrideCursor();
        QString tmpstr;
        tmpstr.sprintf("Error doing TDAC calculation: %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmpstr);
        break;
      }
      /*
      error = datit->GetDID()->ThreshPred(0);
      if(error){
        QApplication::restoreOverrideCursor();
        QString tmpstr;
        tmpstr.sprintf("Threshold/noise prediction failed with error %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::information(this,"Module Analysis",tmpstr);
        break;
      }  
      */    
      // remove existing stuff
      temp = (ModItem*)datit->firstChild();
      while(temp!=0){
        temp2 = temp;
        temp = (ModItem*) temp2->nextSibling();
        if(temp2->text(0)=="TDAC tuning results")
          delete temp2;
      }
      temp = new ModItem(datit,  "TDAC tuning results",  datit->GetDID(),NONE);
      //new ModItem(temp,"Plot pred. threshold", datit->GetDID(),MEAN);
      //new ModItem(temp,"Plot pred. noise",     datit->GetDID(),SIG);
      new ModItem(temp,"Plot TDAC map",   datit->GetDID(),TDACP);
      new ModItem(temp,"Write TDAC map to file",    datit->GetDID(),TDACW);
      QApplication::restoreOverrideCursor();
      break;}
    case ITHSCAN:{
      ModItem *temp, *temp2;
      int error;
      //int twodpts[NCHIP];
      // fit if requested
      if(datit->GetDID()->m_istype==TWODSCAN || !options->m_Opts->m_rec2DFit){
        m_FitWin->FitType->setCurrentItem(2);
        m_FitWin->CfacEdit->setText("1.0");
	int chip_s=-1, chip_e=-1;
	if(ChipOrMod->isOn()){
	  chip_s = ChipSel->value();
	  chip_e = ChipSel->value();
	}
        datit = MAEngine::PerformFit(item,false,true,true,this,m_app, chip_s, chip_e);
        if(datit==0) break;
      } 
      QApplication::setOverrideCursor(Qt::waitCursor);
      // create target ITH 
      error = datit->GetDID()->ITHAna(options->m_Opts->m_ChipTarget);
      if(error){
        QApplication::restoreOverrideCursor();
        QString tmpstr;
        tmpstr.sprintf("Error doing ITH calculation: %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmpstr);
        break;
      }
      // remove existing stuff
      temp = (ModItem*)datit->firstChild();
      while(temp!=0){
        temp2 = temp;
        temp = (ModItem*) temp2->nextSibling();
        if(temp2->text(0)=="ITH fit results")
          delete temp2;
      }
      temp = new ModItem(datit,  "ITH fit results",  datit->GetDID(),NONE);
      //new ModItem(temp,"Plot pred. threshold", datit->GetDID(),MEAN);
      //new ModItem(temp,"Plot pred. noise",     datit->GetDID(),SIG);
      new ModItem(temp,"Plot ITH at target",   datit->GetDID(),ITH);
      new ModItem(temp,"Show ITH summary",     datit->GetDID(),ITHSUMM);
      QApplication::restoreOverrideCursor();
      break;}
    case TWLK:{
      DatSet *rawdat[3], *tw;
      int err=-99;
      rawdat[1] = datit->GetDID();
      // open selection window and get 2nd 2d scan and threshold scan
      RefWin rdw2(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel,6);
      if(rdw2.DatSel->count()<=0)
        textwin->setText("Load timwalk(Chigh) data set to enable analysis");
      else if(rdw2.exec()==1){
        rawdat[2] = rdw2.selit->GetDID();
        RefWin rdw3(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel,5);
        if(rdw3.DatSel->count()<=0)
          textwin->setText("Load threshold scan data set to enable analysis");
        else if(rdw3.exec()==1){
          rawdat[0] = rdw3.selit->GetDID();
          float anapar[2]={(float)options->m_Opts->m_QhighTW,
                           (float)options->m_Opts->m_delT0_twalk};
          tw = new DatSet("Time walk analysis",rawdat,3,TWLK,&err,anapar,2);
          if(err){
            QString tmp;
            tmp.sprintf("Error doing time-walk analysis: %s",DatSet::ErrorToMsg(err).c_str());
            if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
          } else{
            test1 = DataToTree(parnt,tw);
            test1 = (ModItem*) test1->firstChild();
            while(test1!=0){
              if(test1->GetPID()==TWLKGR)
                test1->SetRefArr(rawdat,3);
              test1 = (ModItem*) test1->nextSibling();
            }
          }
        }
      }
      break;}
    default:
      break;}
    break;}
  case GETFDAC:{
    ModItem *temp, *temp2;
    int error;
    float fixed_target=-1;
    // fit if requested
    if((datit->GetDID()->m_fittype<0 || !options->m_Opts->m_rec2DFit) && options->m_Opts->m_DACdoFit){
      m_FitWin->FitType->setCurrentItem(2);
      m_FitWin->CfacEdit->setText("1.0");
      int chip_s=-1, chip_e=-1;
      if(ChipOrMod->isOn()){
	chip_s = ChipSel->value();
	chip_e = ChipSel->value();
      }
      datit = MAEngine::PerformFit(item,false,true,true,this,m_app, chip_s, chip_e);
      if(datit==0) break;
    }
    // get FDAC values
    int fittype = datit->GetDID()->m_fittype;
    if(!options->m_Opts->m_DACdoFit)
      datit->GetDID()->m_fittype = -1;
    QApplication::setOverrideCursor(Qt::waitCursor);
    if(options->m_Opts->m_doFixedTarget)
      fixed_target = options->m_Opts->m_ChipTarget;
    error = datit->GetDID()->TDACAna(options->m_Opts->m_doChipTarget, fixed_target);
    if(!options->m_Opts->m_DACdoFit)
      datit->GetDID()->m_fittype = fittype;
    if(error){
      QApplication::restoreOverrideCursor();
      QString tmpstr;
      tmpstr.sprintf("Error doing FDAC calculation: %s",DatSet::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmpstr);
      break;
    }      
    QApplication::restoreOverrideCursor();
    // remove existing stuff
    temp = (ModItem*)datit->firstChild();
    while(temp!=0){
      temp2 = temp;
      temp = (ModItem*) temp2->nextSibling();
      if(temp2->text(0)=="FDAC tuning results")
        delete temp2;
    }
    temp = new ModItem(datit,  "FDAC tuning results",  datit->GetDID(),NONE);
    new ModItem(temp,"Plot FDAC map",   datit->GetDID(),FDACP);
    //new ModItem(temp,"Plot pred. TOT(20ke)", datit->GetDID(),T20K);
    new ModItem(temp,"Write FDAC map to file",    datit->GetDID(),TDACW);
      break;}
  case TIMEREF:{
    // make sure there's more than one data set
    RefWin rdw(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel, 3);
    if(rdw.DatSel->count()<=0)
      textwin->setText("Load more than this delay-data set to enable options");
    else{
      // open selection window and get reference data set
      if(rdw.exec()==1){
        if(rdw.selit==0)
          textwin->setText("selit-pointer is 0");
        else{
          plotit = new ModItem(item->Parent()->Parent(),"Plot delay scale factor", item->GetDID(),
                               DELCAL, rdw.selit->GetDID());
          plotit = new ModItem(item->Parent()->Parent(),"Plot delay difference", item->GetDID(),
                               TIMEDIF, rdw.selit->GetDID());
          plotit = new ModItem(item->Parent()->Parent(),"Plot delay difference (calib'ed)", item->GetDID(),
                               TCALDIF, rdw.selit->GetDID());
        }
      }
    }
    break;}
  case COMBCAL:{
    RefWin rdw(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel,1);
    if(rdw.DatSel->count()>0){
      if(rdw.exec()==1){
        DatSet *refdid = rdw.selit->GetDID();
        if(refdid->GetLog()==0){
          QMessageBox::warning(this,"ModuleAnalysis","Can't find logfile in ref. data");
          break;
        }
        if(!refdid->GetLog()->MyModule->CHigh){
          QMessageBox::warning(this,"ModuleAnalysis","Selected data is a Clow scan - can't proceed");
          break;
        }
        // everything is ok, let's create a new item with the combined calib.
        //delete item;
        QApplication::setOverrideCursor(Qt::waitCursor);
        //textwin->setText("Re-fitting ToT-calib. for each pixel, please remain patient...");
        DatSet *rawdat[2], *combcal;
        error = -99;
        rawdat[0] = datit->GetDID();
        rawdat[1] = refdid;
        combcal = new DatSet("ToT calibration - combined",rawdat,2,TOTCOMB,&error,0,0);
        if(error){
          QApplication::restoreOverrideCursor();
          tmp.sprintf("Error in analysis: %s",DatSet::ErrorToMsg(error).c_str());
          if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
          textwin->setText("...aborted.");
          QApplication::restoreOverrideCursor();
          break;
        }
        //textwin->setText("...done!");
        QApplication::restoreOverrideCursor();

        test1 = DataToTree(parnt,combcal);
        test1 = (ModItem*) test1->firstChild();
        while(test1!=0){
          if(test1->text(0)=="Plot Q vs ToT"  || test1->text(0)=="Plot ToT distributions" 
             || test1->text(0)=="Fit combined data" || test1->text(0)=="Modify Chi/Clo calibration")
            test1->SetRefArr(rawdat,2);
          test1 = (ModItem*) test1->nextSibling();
        }
      }
    }
    break;}
  case RECALTOT:{
    bool isOK;
    TotRecal trwin(this,"TotRecal", TRUE, Qt::WType_TopLevel,item);
    if(trwin.exec()){
      for(int i=0;i<NCHIP;i++){
        if(trwin.OneForAll->isChecked())
          item->GetDID()->m_ChiCloCal[i] = trwin.GlobalCal->text().toFloat(&isOK);
        else
          item->GetDID()->m_ChiCloCal[i] = trwin.ChipCalib->text(i,0).toFloat(&isOK);
      }
    }
    break;}
  case RECALFIT:{
    /*
    RenameWin getcal(this,"getcal", TRUE, Qt::WType_TopLevel);
    getcal.EnterLabel->setText("Enter calib. correction:");
    getcal.Name->validateAndSet("1.0",4,0,4);
    getcal.Name->setFocus();
    getcal.exec();
    */
    bool isOK, runcal=false;
    int i;
    float cal[NCHIP];
    TotRecal trwin(this,"TotRecal", TRUE, Qt::WType_TopLevel,item);
    trwin.FitButton->hide();
    for(i=0;i<NCHIP;i++)
      trwin.ChipCalib->setText(i,0,"1.0");
    trwin.GlobalCal->setText("1.0");
    if(trwin.exec()){
      runcal = true;
      for(i=0;i<NCHIP;i++){
        if(trwin.OneForAll->isChecked())
          cal[i] = trwin.GlobalCal->text().toFloat(&isOK);
        else
          cal[i] = trwin.ChipCalib->text(i,0).toFloat(&isOK);
        runcal &= isOK;
      }
    }
    if(runcal) item->GetDID()->ModifyToTCal(cal);
    break;}
  case FITTOTC:
    FitToTComb(item);
    break;
  case FIT:{
    int chip_s=-1, chip_e=-1;
    if(ChipOrMod->isOn()){
      chip_s = ChipSel->value();
      chip_e = ChipSel->value();
    }
    MAEngine::PerformFit(item,false,true,true,this,m_app, chip_s, chip_e);
    break;}
  case REFIT:{
    int chip_s=-1, chip_e=-1;
    if(ChipOrMod->isOn()){
      chip_s = ChipSel->value();
      chip_e = ChipSel->value();
    }
    MAEngine::PerformFit(item,true,true,true,this,m_app, chip_s, chip_e);
    break;}
  case TOTCAL:
    CalibrateToT(datit);
    break;
  case DOSRC:
    textwin->setText("Performing source analysis...");
    QApplication::setOverrideCursor(Qt::waitCursor);
    if(item->text(0)!="Re-determine dead pixels"){
      // load mask & perform analysis
      if((error=datit->GetDID()->LoadMask())<0){
        QApplication::restoreOverrideCursor();
        tmp.sprintf("Error doing source analysis: %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
        textwin->setText("...aborted.");
        break;
      }
      // remove the option for this analysis
      delete item;
      item  = datit;
      // remove hot pixel analysis option (interferes with this one)
      test1 = (ModItem*) datit->firstChild();
      while(test1!=0){
        test2 = test1;
        test1 = (ModItem*)test1->nextSibling();
        if(test2->text(0)=="Hot pixel analysis") delete test2;
      }
      datit->GetDID()->FindDeadChannels(options->m_Opts->m_minToT);
      test1 = new ModItem(datit,"Source Analysis",                                datit->GetDID(),NONE);
      test2 = new ModItem(test1,"Plot of masked channels and disabled col pairs", datit->GetDID(),MAMASK);
      test2 = new ModItem(test1,"Tabulate masked channels and disabled col pairs",datit->GetDID(),MASKTAB);
      test2 = new ModItem(test1,"Plot dead channels",                             datit->GetDID(),DEAD_CHAN);
      test2 = new ModItem(test1,"Tabulate dead channels",                         datit->GetDID(),DEADTAB);
      test2 = new ModItem(test1,"Re-determine dead pixels",                       datit->GetDID(),DOSRC);
      QApplication::restoreOverrideCursor();
      textwin->setText("...done");
    } else{
      // only perform dead pixel analysis
      if(datit->GetDID()->GetMap(-1,DEAD_CHAN)!=0) datit->GetDID()->GetMap(-1,DEAD_CHAN)->Reset();
      datit->GetDID()->FindDeadChannels(options->m_Opts->m_minToT);
      QApplication::restoreOverrideCursor();
      textwin->setText("...done");
    }
    break;
  case MAKEMASK:
    // remove source analysis (interferes with this)
    test1 = (ModItem*) datit->firstChild();
    while(test1!=0){
      test2 = test1;
      test1 = (ModItem*)test1->nextSibling();
      if(test2->GetPID()==DOSRC) delete test2;
    }
    // run analysis
    item->GetDID()->GetHotPix(options->m_Opts->m_hitcut-1,options->m_Opts->m_totcut);
    item->GetDID()->MaskGLSet(options->m_Opts->m_masklong, options->m_Opts->m_maskgang);
    test1 = (ModItem*)item->Parent()->firstChild();
    // remove old mask-plot items
    while(test1!=0){
      int dodel=0;
      if(test1->GetPID()==MAMASK || test1->GetPID()==SAVEMASK || test1->GetPID()==INVMASK) dodel=1;
      test2 = test1;
      test1 = (ModItem*)test1->nextSibling();
      if(dodel) delete test2;
    }
    // add new mask-plot items
    test1 = new ModItem(item->Parent(),"Plot of channels to be masked", item->GetDID(),MAMASK);
    test1 = new ModItem(item->Parent(),"Toggle selected pixel in mask", item->GetDID(),TOGMASK);
    test1 = new ModItem(item->Parent(),"Write mask to file",            item->GetDID(),SAVEMASK);
    test1 = new ModItem(item->Parent(),"Write inverse mask to file",    item->GetDID(),INVMASK);
    break;
  case COMBMASK:{
    // offer a list of other masks
    TH2F *ormask=0;
    RefWin rdw(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel,2);
    if(rdw.DatSel->count()>0){
      if(rdw.exec()==1)
        ormask = rdw.selit->GetDID()->GetMap(-1,MAMASK);
    } else{
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"ModuleAnalysis","Can't find any other masks");
      break;
    }
    if(ormask!=0){
      // determine new mask
      item->GetDID()->GetHotPix(options->m_Opts->m_hitcut-1,options->m_Opts->m_totcut,ormask);
      test1 = (ModItem*)item->Parent()->firstChild();
      while(test1!=0){
        int dodel=0;
        if(test1->GetPID()==MAMASK || test1->GetPID()==SAVEMASK || test1->GetPID()==INVMASK) dodel=1;
        test2 = test1;
        test1 = (ModItem*)test1->nextSibling();
        if(dodel) delete test2;
      }
      test1 = new ModItem(item->Parent(),"Plot of channels to be masked", item->GetDID(),MAMASK);
      test1 = new ModItem(item->Parent(),"Write mask to file",            item->GetDID(),SAVEMASK);
      test1 = new ModItem(item->Parent(),"Toggle selected pixel in mask", item->GetDID(),TOGMASK);
      test1 = new ModItem(item->Parent(),"Write inverse mask to file",    item->GetDID(),INVMASK);
    }
    break;
  }
  case SAVEMASK:{
    Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    if(fdia.exec() == QDialog::Accepted) 
      item->GetDID()->WriteHotPix(fdia.selectedFile().latin1());
    break;}
  case INVMASK:{
    Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    if(fdia.exec() == QDialog::Accepted) 
      item->GetDID()->WriteHotPix(fdia.selectedFile().latin1(),1);
    break;}
  case TDACW:{
    Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    if(fdia.exec() == QDialog::Accepted) 
      item->GetDID()->WriteDAC(fdia.selectedFile().latin1());
    break;}
  case TDACCOR:{
    std::vector<ModItem*> loadedList;
    std::vector<ModItem*>::iterator IT;
    LoadList(item->ModParent(), loadedList);
    int error;
    float result;

    BadPixel *badpix = new BadPixel(0,0,"badpix",FALSE,Qt::WDestructiveClose);
    badpix->setCaption("Correlation summary");
    badpix->Init("not used","Correlation to "+item->DatParent()->text(0));
    // do non-standard settings by hand
    badpix->PixelList->horizontalHeader()->setLabel(0,"Data item");
    badpix->PixelList->horizontalHeader()->setLabel(1,"correl. coeff.");
    badpix->PixelList->setNumCols(2);
    badpix->PixelList->setNumRows(0);
    badpix->MaskButt->hide();

    for(IT=loadedList.begin();IT!=loadedList.end();IT++){
      error = rootmain->PlotCorrel(item->ModParent()->text(0).latin1(),item, *IT,
                                   TDACP, TDACP,1,0, chip, true, pixScanLevel->value());
      result = -1;
      if(!error){
        int nRows = badpix->PixelList->numRows();
        TH2F *tmphi = (TH2F*) gROOT->FindObject("corrplt");
        if(tmphi){
          result = tmphi->GetCorrelationFactor();
          badpix->PixelList->setNumRows(nRows+1);
          badpix->PixelList->setText(nRows,0,(*IT)->text(0));
          badpix->PixelList->setText(nRows,1,QString::number(result,'f',2));
        }
      }
      RemData(*IT,false);
    }
    badpix->show();
    break;}
  case WRITECFG:{
    if(options->m_Opts->m_cfgUsrdefNames){
      CfgForm cf(this,"cfgform",TRUE);
      if(cf.exec()==QDialog::Accepted){
        const char *auxnames[6]={cf.m_items[1]->m_FileName->text().latin1(),
                                 cf.m_items[2]->m_FileName->text().latin1(),
                                 cf.m_items[3]->m_FileName->text().latin1(),
                                 cf.m_items[4]->m_FileName->text().latin1(),
                                 cf.m_items[5]->m_FileName->text().latin1(),
                                 cf.m_items[6]->m_FileName->text().latin1()};
        item->GetDID()->WriteConfig(cf.m_items[0]->m_FileName->text().latin1(),auxnames);
      }
    } else{
      QStringList filter("Config file (*.cfg)");
#ifdef HAVE_PLDB
      filter += "RootDB cfg file (*.cfg.root)";
#endif
      filter += "Any file (*.*)";
      Q3FileDialog fdia(QString::null, QString::null, this,"select data file",TRUE);
      fdia.setFilters(filter);
      fdia.setMode(Q3FileDialog::AnyFile);
      if(fdia.exec() == QDialog::Accepted){
        if(fdia.selectedFile().right(5)==".root" || fdia.selectedFilter()=="RootDB cfg file (*.cfg.root)"){
#ifdef HAVE_PLDB
          QString fname = fdia.selectedFile();
          if(fname.right(9)!=".cfg.root")
            fname += ".cfg.root";
          if(item->GetDID()->GetLog()!=0){
            try{
              QApplication::setOverrideCursor(Qt::waitCursor);
              item->GetDID()->GetLog()->MyModule->createRootDB(fname.latin1());
              QApplication::restoreOverrideCursor();
            }catch(SctPixelRod::BaseException& exc){
              std::stringstream msg;
              msg << exc;
              QApplication::restoreOverrideCursor();
              QMessageBox::warning(this,"exception",("Error during RootDB writing:\n" + msg.str()).c_str());
              return;
            }catch(...){
              QApplication::restoreOverrideCursor();
              QMessageBox::warning(this,"exception","Unknown error during RootDB writing");
              return;
            }
          }
#else
          QMessageBox::warning(this,"nopixlib","This version comes without RootDB, "
                               "saving in this format not possible");
#endif
        }else{
          QApplication::setOverrideCursor(Qt::waitCursor);
          item->GetDID()->WriteConfig(fdia.selectedFile().latin1());
          QApplication::restoreOverrideCursor();
        }
      }
    }
    break;}
  case GETLOG:{
    Q3FileDialog fdia(QString::null, "Log file (*.logfile)",this,"select data file",TRUE);
    if(fdia.exec() == QDialog::Accepted){ 
      DatSet *did = item->GetDID();
      if((error=did->LoadLog(fdia.selectedFile().latin1()))==0){
        // re-create everything and delete old items
        DataToTree(parnt,datit->GetDID());
        delete datit;
      } else{
        tmp.sprintf("Error loading logfile %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Modunal Analysis",tmp);
      }
    }
    break;}
  case BI_SCANS_PLOT:                                   //(added 27/4/04 by LT)                
    if((error=rootmain->PlotBI_scans(item))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case BI_VIT_VS_TIME_PLOT:                                   //(added 27/4/04 by LT)                
    if((error=rootmain->PlotBI_vitVsTime(item))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case BI_I_VS_T_PLOT:                                   //(added 27/4/04 by LT)                
    if((error=rootmain->PlotBI_currentVsTemp(item))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;
  case LVL1DIST:{
    int scanpt = 0;
    if(item->GetDID()!=0 && item->GetDID()->m_isscan)
      scanpt = Scanpt0->value();
    if((error=rootmain->PlotLvl1(item, chip, scanpt))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;}
  case LVL12DPLT:{
    if((error=rootmain->PlotLvl1(item, chip, -999))){
      tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    }
    break;}
  default:
#ifdef PIXEL_ROD // can link to PixScan
    if(item->GetPID()>=PIXDBTYPES){
      rootmain->m_currmap = 0;
      int ps_type = item->GetPID()-PIXDBTYPES;
      int scanpt = Scanpt0->value(), pslevel = pixScanLevel->value();
      error = 0;
      QApplication::setOverrideCursor(Qt::waitCursor);
      int ptarr[3]={ Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
      if(ps_type<PixLib::PixScan::MAX_HISTO_TYPES && (options->m_Opts->m_PLplotType[ps_type])<=1)
	error=rootmain->PlotMapScanhi(item, chip, ptarr, 0, 0, pslevel, 0);
      else
	error=rootmain->PlotMapProjScat(item, chip, ptarr, 0, pslevel);
      if(error==-2) // no 2D histo found, might have been 1D histo - try this
	error=rootmain->PlotHisto(item, scanpt, pslevel, 0);
      QApplication::restoreOverrideCursor();
      if(error){
	tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
	if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
      }
    } else {
#else
    {
#endif
      QMessageBox::warning(this,"MA: Processing item action",
			   "Uknown action for type "+QString::number(item->GetPID()));
    break;
    }
  }
  //TOT distribution special:
  if(item->GetPID()==TOT_DIST)
    item->ChgPID(TOT_SUMM);
  if(item->GetPID()==TOT_FDIS)
    item->ChgPID(TOT_FAST);
  if(item->GetPID()==CAL_DIST)
    item->ChgPID(CAL_SUMM);
  if(item->GetPID()==CAL_FDIS)
    item->ChgPID(CAL_FAST);
  // store this object in current map for reference
  // and update bin content indicator
  if(rootmain->m_currmap!=0)
    rootmain->m_currmap->SetWidget(this);
  UpdatePixVal();

  return;
}
void TopWin::SaveData(ModItem* item, int doall, const char *in_path, const char *in_comment){
  //  ModItem *tmp, *dset_child, *sub_child;
  if(item==0) return;
  textwin->setText(" ");
  // open a file dialog to get path
  //  int i,j,k;
  int ndat=0, oachoice;
  char cndat[5];//, dirnam[20];
  //  DatSet *did=0;
  TFile *output;
  QString path, messg;
  QStringList filter("ROOT file (*.root)");
  filter += "Any file (*.*)";
  // load default dir. if requested to
  if(in_path!=0)
    path = in_path;
  else
    path = QString::null;
  if(path==QString::null && options->m_Opts->m_DefDataPath!="."){
    if(options->m_Opts->m_UseMname){
      path = options->m_Opts->m_DefDataPath.c_str();
      path+= "M" + item->text(0) + "/data/";
    }else
      path = options->m_Opts->m_DefDataPath.c_str();
  }
  if(in_path==0){
    Q3FileDialog fdia(path, QString::null,this,"getfilename",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    fdia.setFilters(filter);
    if(fdia.exec() == QDialog::Accepted && !fdia.selectedFile().isEmpty())
      path = fdia.selectedFile();
    else
      path = QString::null;
  }
  if(path==QString::null) return;

  // check if filename ends with root, otherwise attach
  if(path.find(".root",-5)<0)
    path += ".root";
  // see if file exists; if yes, ask user whether to overwrite or append
  oachoice = QMessageBox::Yes;
  FILE *testfile = fopen(path.latin1(),"r");
  if(testfile!=0){
    fclose(testfile);
    QMessageBox overwrt("Overwrite file?", 
                        "File exists:" + path + "\nDo you want to replace or append?",
                        QMessageBox::Warning,QMessageBox::Yes, QMessageBox::No, QMessageBox::Abort,
                        this, "overwrite_file",TRUE,Qt::WType_TopLevel);
    overwrt.setButtonText(QMessageBox::Yes,QString("Replace"));
    overwrt.setButtonText(QMessageBox::No ,QString("Append"));
    oachoice = overwrt.exec();
  }
  // hour glass cursor
  // open root file and write DatSet as TObject
  if(oachoice==QMessageBox::Yes){
    output = new TFile(path.latin1(),"RECREATE");
    TObjString mn(item->text(0).latin1());
    mn.Write("module name");
  }else if(oachoice==QMessageBox::No){
    output = new TFile(path.latin1(),"UPDATE");
    TObject *tmpto;
    tmpto = output->Get("ndat");
    sscanf(tmpto->GetName(),"%d",&ndat);
  } else
    return;
  
  QApplication::setOverrideCursor(Qt::waitCursor);
  ndat = saveDataCore(item, doall, gDirectory);
  sprintf(cndat,"%d",ndat);
  TObjString nd(cndat);
  gDirectory->cd("..");
  nd.Write("ndat");
  TObjString ver_to(m_verstrg.latin1());
  ver_to.Write("version");
  QString qcmt = "";
  if(in_comment!=0){
    qcmt = in_comment;
    qcmt += "\n";
  }
  qcmt += "Saved on ";
  qcmt += QDateTime::currentDateTime().toString();
  TObjString comment(qcmt.latin1());
  comment.Write("comment");
  output->Close();

  QApplication::restoreOverrideCursor();
  return;
}
void TopWin::RenameMod(ModItem *item){
  if(item==0) return;
  RenameWin rnw(this," ", TRUE, Qt::WType_TopLevel);
  rnw.Name->validateAndSet(item->text(0),item->text(0).length(),
                           0,item->text(0).length());
  rnw.Name->setFocus();
  if(rnw.exec()){
    item->setText(0,rnw.Name->text());
    SetCurrMod((Q3ListViewItem*)item);
  }
  return;
}
void TopWin::RemoveMod(ModItem *item, bool ask){
  bool go_for_it = false;
  if(item==0) return;
  if(item->m_lock){
    QMessageBox::warning(this,"TopWin::RemoveMod","Item "+item->text(0)+" is in use\n"
                         "and can't be deleted");
    return;
  }
  if(ask){
    QMessageBox ays("Delete item?", 
                    "This will delete all data of module " + item->text(0)+"\nAre you sure?",
                    QMessageBox::Warning,QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton,
                    this, "ays",TRUE,Qt::WType_TopLevel);
    if(ays.exec()==QMessageBox::Yes)
      go_for_it = true;
  } else
    go_for_it = true;

  if(go_for_it){
    bool allData_gone=true;
    ModItem *tmpit, *datits = (ModItem*) item->firstChild();
    while(datits!=0){
      tmpit = datits;
      datits = (ModItem *) datits->nextSibling();
      if(tmpit->m_lock){
        QMessageBox::warning(this,"TopWin::RemData","Data-item "+tmpit->text(0)+"\n"
                             "is in use and can't be deleted");
        allData_gone = false;
      } else{
        DatSet *mydata = tmpit->GetDID();
        delete tmpit;
        delete mydata;
      }
    }
    if(allData_gone){
      delete item;
      ModuleList->setCurrentItem(0);
      SetCurrMod(0);
    }
  }
  return;
}
void TopWin::CreatePDB(ModItem *item){
  if(item==0) return;
  PDBForm *pdb = new PDBForm(this,"PDB Form Manager",FALSE,Qt::WDestructiveClose,item,m_app,0);
  if(options->m_Opts->m_userPDB!="" && options->m_Opts->m_passwdPDB!=""){
    pdb->m_User = options->m_Opts->m_userPDB.c_str();
    pdb->m_Passwd = options->m_Opts->m_passwdPDB.c_str();
  }
//   if(pdb->retValue==1){
//     QMessageBox::warning(this,"Module Analysis","PDBForm: No Log File loaded!");
//  } else if(pdb->retValue==2){
  if(pdb->retValue==2){
    QMessageBox::warning(this,"Module Analysis","PDBForm: Unknown module type!");
//   } else if(pdb->retValue==3){
//     QMessageBox::warning(this,"Module Analysis","PDBForm: Missing data sets!");
  } else if(pdb->retValue==4){
    QMessageBox::warning(this,"Module Analysis","PDBForm: Module item is NULL!");
  } else pdb->show();
  return;
}
void TopWin::PDBProf(){
  PDBForm pdb(this,"PDB Form Manager",TRUE,Qt::WType_TopLevel);
  pdb.savePlots->setEnabled(false);
  pdb.removeFiles->setEnabled(false);
  pdb.exec();
}
ModItem* TopWin::GetCurrMod(){
  ModItem *temp=0, *currModule;
  currModule = (ModItem*)ModuleList->currentItem();
  if(currModule!=0){
    if(currModule->GetPID()==MODITEM)
      return currModule;
    else{
      temp = currModule->ModParent();
      if(temp!=0 && temp->GetPID()==MODITEM)
        return temp;
      else
        return 0;
    }
  }
  return 0;
}
void TopWin::BiasStabCurrMod(){
  BiasStab(GetCurrMod());
}
void TopWin::CompareBadChansCurrMod(){
  CompareBadChans(GetCurrMod());
}
void TopWin::PlotCorrelCurrMod(){
  // obsolete
  //  PlotCorrel(GetCurrMod());
}
void TopWin::PlotMaskedCurrMod(){
  PlotMasked(GetCurrMod());
}
void TopWin::PlotMultiGraphCurrMod(){
  PlotMultiGraphs(GetCurrMod());
}
void TopWin::CreatePDBCurrMod(){
  CreatePDB(GetCurrMod());
}
void TopWin::RenameCurrMod(){
  RenameMod(GetCurrMod());
}
void TopWin::RemRawCurrMod(){
  RemoveAllRaw(GetCurrMod(),true);
}
void TopWin::ClearFitsCurrMod(){
  ClearAllFits(GetCurrMod(),true);
}
void TopWin::RemoveCurrMod(){
  RemoveMod(GetCurrMod());
}
void TopWin::SaveDataCurrMod(){
  SaveData(GetCurrMod(), 1);
  return;
}
void TopWin::SaveSelDataCurrMod(){
  SaveData(GetCurrMod(), 0);
  return;
}
void TopWin::LoadDataCurrMod(){
  LoadData(GetCurrMod());
  return;
}
void TopWin::LoadListCurrMod(){
  LoadList(GetCurrMod());
  return;
}
void TopWin::LoadStdCurrMod(){
  LoadStd(GetCurrMod());
  return;
}
ModItem* TopWin::LoadRoot_Menu(bool interactive, const char *in_path, ModItem *in_parent, bool *loadData){
  ModItem *item=0, *temp_parent=0, *parent=0;
  QString path=QString::null, mname;
  int i;
  std::vector<std::string> moddir_names;
  std::vector<QString> staves;
  bool stv_exist=false;
  std::vector<int> moddir_pos;
  if(in_path==0){
    if(options->m_Opts->m_DefDataPath!=".")
      path = options->m_Opts->m_DefDataPath.c_str();
    QStringList filter("ROOT file (*.root)");
    filter += "Any file (*.*)";
    Q3FileDialog fdia(path, QString::null,this,"select data file",TRUE);
    fdia.setFilters(filter);
    path=QString::null;
    if(fdia.exec() == QDialog::Accepted) 
      path = fdia.selectedFile();
  } else
    path = in_path;

  if(path.isEmpty()) return 0;
  // change Windows style if necessary
  path.replace(QRegExp("\\"),"/");

  TObject *tmpro;
  TFile *f = new TFile(path.latin1());

  // check if it's one module per file or multi-module file
  tmpro = f->Get("ModuleFolder");
  if(tmpro!=0){
    // multi module
    TIter next(f->GetListOfKeys());
    TKey *key;
    if(interactive){
      //allow to select modules from list
      RootDataList dl(this,"rootdl",TRUE);
      dl.moduleLabel->setText("List of available modules in current file");
      while((key=(TKey*)next())){
        std::string class_name = key->GetClassName();
        if(class_name=="TDirectory")
          dl.LoadName(key->GetName(), (long int)gDirectory);//lists available modules
      }
      Q3ListViewItem *modIT = dl.FileList->firstChild();
      if(dl.exec()==0 || modIT==0){
        f->Close();
        return 0;
      } else{
        while(modIT!=0){
		  if(modIT->isSelected())
            moddir_names.push_back(modIT->text(0).latin1());
          modIT = modIT->nextSibling();
        }
      }
    } else{ // select all modules
      while((key=(TKey*)next())){
        std::string class_name = key->GetClassName();
        if(class_name=="TDirectory")
          moddir_names.push_back(key->GetName());
      }
    }
  } else
    moddir_names.push_back("no module dirs");
  f->Close();

  staves.resize(moddir_names.size());
  moddir_pos.resize(moddir_names.size());

  for(i=0;i<(int)moddir_names.size();i++){

    TFile *f = new TFile(path.latin1());
    //in case it's a multi-module file we have to cd to module folder first
    if(moddir_names[i]!="no module dirs"){
      f->cd(moddir_names[i].c_str());
	  //*********************************************************************************************
	  TObjString *pos = (TObjString*) gDirectory->Get("position");
      if(pos!=0){
	    QString stpos = (pos->GetString()).Data();
		moddir_pos[i]=(stpos.toInt());
      }else moddir_pos[i]=0;
	  TObjString *stv = (TObjString*) gDirectory->Get("Stave");
	  if(stv!=0) staves[i]=(QString)stv->GetString();
	  //*********************************************************************************************
	  mname = moddir_names[i].c_str();
    } else{
      // find out module name
      tmpro = f->Get("module name");
      if(tmpro==0){
        QMessageBox::warning(this,"MAF","Couldn't find a module tag in the file/directory.");
	mname="";
      }else{
	mname = tmpro->GetName();
	moddir_pos[i] = 0;
	staves[i] = "";
      }
    }
    f->Close();
    
    ModItem *temp, *currModule=0;
    std::vector<ModItem*> allMods = getAllModuleItems();
    if(mname!=""){
      textwin->setText("loading module "+mname);
      textwin->repaint();
      if(m_app!=0)
	m_app->processEvents();
      
      for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
	temp = *IT;
	if(temp->text(0)==mname){
	  currModule = temp;
	  break;
	}
      }
      item = currModule;

      if(currModule==0){ // module not found, create item
	if(interactive && moddir_names[i]=="no module dirs"){
	  QMessageBox info("New module", 
			   "Module " + mname + " not found in list.\nNew according item is created now.",
			   QMessageBox::Information,QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
			   NULL, "newmod",FALSE,Qt::WType_TopLevel);
	  info.exec();
	}
	if(in_parent==0){
	  //****find the right parent stave ***********************************************************
	    if(staves[i]!="" && moddir_pos[i]!=0){
	      temp_parent=(ModItem*)(ModuleList->firstChild());
	      stv_exist=false;
	      while(temp_parent!=0){
		if((temp_parent->text(0)).compare(staves[i])==0){
		  parent = temp_parent;
		  stv_exist = true;
		}
		temp_parent = (ModItem*)temp_parent->nextSibling();
	      }
	      if(!stv_exist)
		parent = new ModItem(ModuleList, staves[i], 0, STVITEM);
	    }else parent = 0;
	  //*******************************************************************************************
	      }else
	  parent = in_parent;
	if(parent==0)
	  item = new ModItem(ModuleList, mname, 0, MODITEM);
	else
	  item = new ModItem(parent, mname, 0, MODITEM);
	
	ModuleList->setCurrentItem(item);
	currModule = item;
      }
      
      if(moddir_names[i]!="no module dirs")
	LoadRoot(item, path.latin1(), interactive, moddir_names[i].c_str(), loadData);
      else
	LoadRoot(item, path.latin1(), interactive, 0, loadData);
      
      currModule->setOpen(TRUE);
      if(parent!=0 && parent->GetPID()==STVITEM){
	currModule->setOpen(FALSE);
	currModule->SetModPos(moddir_pos[i]);
      }
      SetCurrMod(currModule);
    }
  }

  return item;
}
void TopWin::LoadList(ModItem* item){
  std::vector<ModItem*> loadedList;
  LoadList(item, loadedList);
}
void TopWin::LoadList(ModItem *item, std::vector<ModItem*> &loadedList){
  if(item==0) return;
  QString label, path=QString::null, fname;
  char line[3001], *tmpc;
  int i, type=NONE;
  // load default dir. if requested to
  if(options->m_Opts->m_DefDataPath!="."){
    if(options->m_Opts->m_UseMname){
      path = options->m_Opts->m_DefDataPath.c_str();
      path+= "M" + item->text(0) + "/data/";
    }else
      path = options->m_Opts->m_DefDataPath.c_str();
  }
  Q3FileDialog fdia(path, "File list (*.txt)",this,"select data file",TRUE);
  if(fdia.exec() == QDialog::Accepted){
    FILE *list = fopen(fdia.selectedFile().latin1(),"r");
    if(list!=0){
      tmpc = fgets(line,3000,list);
      line[strlen(line)-1] = '\0';
      if(tmpc!=0) 
        path = line;
      else{
        fclose(list);
        return;
      }   
      i = path.find(" all-type");
      if(i>0){
        sscanf(path.right(path.length()-i-10).latin1(),"%d",&type);
        path = path.left(i);
      }
      i=0;
      while((tmpc=fgets(line,3000,list))!=0){
        line[strlen(line)-1] = '\0';
        if((i/2)*2==i)
          fname = line;
        else{
          label = line;
          fname = path + "/" + fname;
          m_fwin->SetType(m_fwin->GuessType(fname));
          if(type==NONE)
            type = m_fwin->m_seltype;
          ModItem *tmpit = LoadData(item,type,fname.latin1(), label.latin1());
          loadedList.push_back(tmpit);
          printf("loaded %s\n",tmpit->text(0).latin1());
        }
        i++;
      }
    }
    SetCurrMod((Q3ListViewItem*)item);
  }
}
void TopWin::LoadData(ModItem* item){
  if(item==0) return;
  // open a file dialog to get path and some info on content
  QString label, path=QString::null;
  int type, i;
  
  label = "Load file(s) for module \"" + item->text(0) + "\"";
  m_fwin->ModuleLabel->setText(label);
  m_fwin->FileName->setText("No file has been selected");
  m_fwin->FileType->setCurrentItem(0);
  m_fwin->SetType(0);
  // load default dir. if requested to
  if(options->m_Opts->m_DefDataPath!="."){
    if(options->m_Opts->m_UseMname){
      path = options->m_Opts->m_DefDataPath.c_str();
      path+= "M" + item->text(0) + "/data/";
    }else
      path = options->m_Opts->m_DefDataPath.c_str();
  }
  m_fwin->SetDefDir(path);

  if(m_fwin->exec()==1 && !m_fwin->FileName->text().isEmpty()
     && m_fwin->m_seltype>0){
    type = m_fwin->m_seltype;
    // data label; default: new data
    if(!m_fwin->Comments->text().isEmpty())
      label = m_fwin->Comments->text();
    else
      label = "new data";
    
    if(type!=MULTIF)
      LoadData(item,type,m_fwin->FileName->text().latin1(), label.latin1());
    else{
      for(i=0;i<(int)m_fwin->m_pathlist.count();i++){
        std::string slabel = m_fwin->m_pathlist[i].latin1();
        type = slabel.find_last_of("/");
        if(type>=0) slabel.erase(0,type+1); 
        m_fwin->SetType(m_fwin->GuessType(m_fwin->m_pathlist[i]));
        type = m_fwin->m_seltype;
        LoadData(item,type,m_fwin->m_pathlist[i].latin1(),slabel.c_str());
      }
    }
    SetCurrMod((Q3ListViewItem*)item);
  }
  return;
}
ModItem* TopWin::LoadData(ModItem* item, int type, const char *path, const char *label){
  int pos;
  textwin->setText("");
  if(item==0) return 0;
  QString logpath="";
  std::string lpath = path;
  // first, check if the logfile still exists; if not ask user
  // except those file for which we never expect a logfile or for which no user
  // interaction is desired
  if(type!=BI_VIT_DATA && type!=SENSORIV && type!=ROOTF && type!= TFDAC && 
     type !=MASKDAT && type!=REGTEST && DatSet::LogPath(lpath)<0){
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",
                     "Can't find default logfile. Search for another logfile or config file \n"
                     "in the following browser (press cancel to skip).");
    QStringList filter("Log file (*.logfile)");
    filter += "Config file (*.cfg)";
    filter += "Any file (*.*)";
    Q3FileDialog fdia(QString::null, QString::null,this,"select data file",TRUE);
    fdia.setFilters(filter);
    if(fdia.exec() == QDialog::Accepted) 
      logpath = fdia.selectedFile();
  }
  
  // for DIG or TOTR types, check if fit output exists;
  // if yes, change file type to load both raw and fit data
  if(type==DIG || type==TOTR){
    FILE *testfile;
    pos = lpath.find(".logfile");
    lpath.erase(pos,lpath.length());
    if(type==DIG)  lpath += "_sfits.out";
    if(type==TOTR) lpath += "_cal.out";
    testfile = fopen(lpath.c_str(),"r");
    if(testfile!=0){
      if(type==DIG)  type = ONEDFIT;
      if(type==TOTR) type = TOTFR;
      fclose(testfile);
    }
  }
  
  // create DatSet object and read datafile(s)
  if(type==ROOTF){
    LoadRoot(item, path);
    return 0;
  }else
    return LoadData(item, path, label,type,logpath.latin1());
}
ModItem* TopWin::LoadData(ModItem* item, const char *filename, const char *name, 
                         int intype, const char *cfgname){
  if(item==0) return 0;
  QString label;
  int retval, type=TMath::Abs(intype);
  DatSet *did=0;
  QApplication::setOverrideCursor(Qt::waitCursor);
  if(type==CHIPCURR){
    int pars[4]={options->m_Opts->m_DDsupp,
                 options->m_Opts->m_DDchan,
                 options->m_Opts->m_DDAsupp,
                 options->m_Opts->m_DDAchan};
    did = new DatSet(name, filename, type, &retval, cfgname, pars);
  }else if(type==IVSCAN){
    int pars[1]={(int)options->m_Opts->m_absValIV};
    did = new DatSet(name, filename, type, &retval, cfgname, pars);
  }else
    did = new DatSet(name, filename, type, &retval, cfgname);
  QApplication::restoreOverrideCursor();
  if(type==IVSCAN && retval==-21){
    if(options->m_Opts->m_showwarn){
      QMessageBox tempcorr("Set temperature?", 
                           "Temperature in IV-data (" + QString(name) +
                           ") is too low to be meaningful.\n"
                           "Do you want to correct this?",
                           QMessageBox::Question,QMessageBox::Yes, QMessageBox::No, Qt::NoButton,
                           this, "setT",TRUE,Qt::WType_TopLevel);
      if(tempcorr.exec()==QMessageBox::Yes){
        RenameWin gettemp(this,"gettemp", TRUE, Qt::WType_TopLevel);
        gettemp.EnterLabel->setText("Enter ambient temperature:");
        gettemp.Name->validateAndSet("20.0",4,0,4);
        gettemp.Name->setFocus();
        gettemp.exec();
        bool isok;
        float temper = gettemp.Name->text().toFloat(&isok);
        if(isok) did->CorrectIVTemp(temper);
      }
    }
    retval = 0;
  }
  
  if(retval<0){
    if(did!=0){
      delete did;
      did=0;
    }
    label = filename;
    // a few special cases in the returned error value:
    // 1. error happended in the config file
    if(retval<-100000){
      int ind;
      ind = retval/100000;
      label.sprintf("%s (file %s)",DatSet::ErrorToMsg(ind).c_str(),cfgname);
    }
    // 2. error happended in one of the 16 tried files, find out which
    else if(retval<-100){
      int ind;
      ind = label.find("_",label.length()-7);
      QString ext = label.right(4);
      label.truncate(ind+1);
      ind = retval/(-100)-1;
      retval += 100*(ind+1);
      label.sprintf("%s (file %s%d%s)",DatSet::ErrorToMsg(retval).c_str(),label.latin1(),ind,ext.latin1());
    }
    // 3. read just one file
    else
      label.sprintf("%s (file %s)",DatSet::ErrorToMsg(retval).c_str(),label.latin1());
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",label);
    return 0;
  }
  label = filename;
  item->SetLastpath(label);
  if(did==0){
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis","Data pointer is 0");
    return 0;
  }
  return DataToTree(item,did);
}
ModItem* TopWin::DataToTree(ModItem* item, PixelDataContainer *data_in){
  if(data_in==0 || item==0) return 0;
  QString label;
  int type=NONE, isscan=0, iscal=0, npar;
  ModItem *plotit, *subplt;
  // get some DatSet properties to be added to ListView entry/needed for set-up
  isscan = data_in->getScanLevel();
  DatSet *did = dynamic_cast<DatSet*>(data_in);
  if(did!=0){
    type   = did->m_istype;
    iscal  = did->m_iscal;
  }
#ifdef PIXEL_ROD // can link to PixScan
  DBtoMAData *dbData = dynamic_cast<DBtoMAData*>(data_in);
  if(dbData!=0)
    type = PIXDBF;
#endif
  label.sprintf("%s",data_in->GetName());
  // create new entry in the ListView
  ModItem *datit = new ModItem(item, label, data_in,type);
  // log file if there
  if(did!=0)
    LogToTree(did,datit);
  // allow to plot
  switch(type){
  case ITHSCAN:
    plotit = new ModItem(datit, "ITH fit results",  did,NONE);
    subplt = new ModItem(plotit,"Plot chi^2",       did,CHI);
    subplt = new ModItem(plotit,"Plot threshold",   did,MEAN);
    break;
  case BRIEFSUM:
    plotit = new ModItem(datit, "Show thresh./noise summary",did,THSUMM);
    break;
  case BRIEFDIG:
    plotit = new ModItem(datit, "Show ineff. pixel summary",did,DIGSUMM);
    break;
  case ONEDFIT: // combined hit+sfit info
    plotit = new ModItem(datit, "Raw data",             did,NONE);
    subplt = new ModItem(plotit,"Plot hits vs scan par",did,RAW);
    subplt = new ModItem(plotit,"Fit all data",         did,FIT);
    subplt = new ModItem(plotit,"Re-fit only bad chi^2",did,REFIT);
  case ONEDSCAN:  // slightly reduced programme if no raw data
    plotit = new ModItem(datit, "S-fit results",    did,NONE);
    subplt = new ModItem(plotit,"Plot chi^2",       did,CHI);
    if(did->GetLog()!=0){ // find type of scans
      if(did->GetLog()->MyScan->GetInner()=="VCal FE" ||
         did->GetLog()->MyScan->GetInner()=="VCal diff"){ // give mean, noise special names
        subplt = new ModItem(plotit,"Plot noise",       did,SIG);
        subplt = new ModItem(plotit,"Plot threshold",   did,MEAN);
        subplt = new ModItem(plotit,"Options",          did,REFD);
        subplt = new ModItem(plotit,"Show thresh./noise summary",did,THSUMM);
      } else if(did->GetLog()->MyScan->GetInner()=="MCC strobe delay"){ // trigger delay: special options
        subplt = new ModItem(plotit,"Plot delay time",  did,TIME);
        subplt = new ModItem(plotit,"Get delay reference scan", did,TIMEREF);
      } else{ // general set-up
        subplt = new ModItem(plotit,"Plot sigma",     did,SIG);
        subplt = new ModItem(plotit,"Plot mean",      did,MEAN);
        subplt = new ModItem(plotit,"Options",          did,REFD);
      } 
    } else{
      subplt = new ModItem(plotit,"Plot sigma",       did,SIG);
      subplt = new ModItem(plotit,"Plot mean",        did,MEAN);
      subplt = new ModItem(plotit,"Options",          did,REFD);
    }
    break;
  case DIG:  // simple hit map, only gets simple raw data plot
    if(isscan>0){
      plotit = new ModItem(datit, "Raw data",         did,NONE);
      if(isscan==1){
        subplt = new ModItem(plotit,"Plot hits vs scan par",did,RAW);
        subplt = new ModItem(plotit,"Fit all data",         did,FIT);
      }else if(isscan==2){
        subplt = new ModItem(plotit,"Plot hits vs outer&inner",   did,RAW);
        subplt = new ModItem(plotit,"Plot 2D-slice vs inner",did,RAWSLC);
      }
    } else
      plotit = new ModItem(datit,  "Plot raw data",    did,RAW);
    if(did->GetLog()->MyModule->GetInjection()=="Digital" && isscan>0){
      QString scanVar = did->GetLog()->MyScan->GetInner().c_str();
      plotit = new ModItem(datit, "Excess/missing hits", did,NONE);
      subplt = new ModItem(plotit,"Plot #pixels vs "+scanVar+" and maps",  did,DIGEMP);
      subplt = new ModItem(plotit,"Plot maps ("+scanVar+">"+scanVar+"min)",  did,DIGEMPR);
      subplt = new ModItem(plotit,"Plot #hits vs "+scanVar+" and maps",  did,DIGEMN);
      subplt = new ModItem(plotit,"Upload channels to be masked", did,GETMASK);
      if(did->GetMap(-1,MAMASK)!=0){
        subplt = new ModItem(plotit,"Plot of channels to be masked", did,MAMASK);
        subplt = new ModItem(plotit,"Toggle selected pixel in mask", did,TOGMASK);
        subplt = new ModItem(plotit,"Write mask to file",            did,SAVEMASK);
        subplt = new ModItem(plotit,"Apply mask to all other scans", did,APPLMASK);
      }
    }else if(did->GetLog()->MyModule->GetInjection()!="Digital" && isscan==0){
      // hot pixel detection
      plotit = new ModItem(datit, "Hot pixel analysis",    did,  NONE);
      subplt = new ModItem(plotit,"Determine channels to be masked", did,MAKEMASK);
      subplt = new ModItem(plotit,"Determine mask & combine with other mask", did,COMBMASK);
    }
    break;
  case TOTR:
    if(isscan==1){
      if(did->GetLog()!=0){ // find type of scans
        if(did->GetLog()->MyScan->GetInner()=="All FDACs"){
          plotit = new ModItem(datit, "ToT vs FDAC",          did,NONE);
          subplt = new ModItem(plotit,"Plot ToT vs FDAC",     did,TOTRAW);
          subplt = new ModItem(plotit,"Fit all data",         did,FIT);
          plotit = new ModItem(datit, "Calculate FDAC",       did,GETFDAC);
        }else if(did->GetLog()->MyScan->GetInner()=="VCal FE" ||
                did->GetLog()->MyScan->GetInner()=="VCal diff"){
          plotit = new ModItem(datit, "ToT vs VCAL",          did,NONE);
          subplt = new ModItem(plotit,"Plot ToT vs VCAL",     did,TOTRAW);
          subplt = new ModItem(plotit,"Fit all data",         did,FIT);
        } else{
          plotit = new ModItem(datit, "Raw ToT data",         did,NONE);
          subplt = new ModItem(plotit,"Plot ToT vs scan par", did,TOTRAW);
          subplt = new ModItem(plotit,"Fit all data",         did,FIT);
          plotit = new ModItem(datit, "Hit data",             did,NONE);
          subplt = new ModItem(plotit,"Plot hits vs scan par",did,RAW);
        }
      }
    } else if(isscan==2){
      plotit = new ModItem(datit, "Raw ToT data",            did,NONE);
      subplt = new ModItem(plotit,"Plot avg. ToT vs outer&inner", did,TOTRAW);
      subplt = new ModItem(plotit,"Plot ToT sd. vs outer&inner", did,TOTSD);
      subplt = new ModItem(plotit,"Plot hits vs outer&inner",did,RAW);
    } else{
      // general stuff
      plotit = new ModItem(datit,  "ToT data",                did,NONE);
      subplt = new ModItem(plotit, "Plot hits from ToT",      did,TOT_HIT);
      subplt = new ModItem(plotit, "Plot ratio of hits to neighbours",did,TOT_RATE);
      subplt = new ModItem(plotit, "Plot raw avg. ToT",       did,AVTOT);
      subplt = new ModItem(plotit, "Plot raw ToT std. deviation",did,SIGTOT);
      subplt = new ModItem(plotit, "Plot max. ToT",           did,TOT_MAX);
      subplt = new ModItem(plotit, "Plot raw ToT dist.", did,TOTRAW);
      // source specific: dead chan. analysis and peak position fitter
      // -> request self-triggering mode
      if(did->GetLog()!=0 && did->GetLog()->MyScan->TrgMode==2){
        //  subplt = new ModItem(plotit, "Plot raw ToT dist.", did,TOTRAW);
        if(iscal){
          subplt = new ModItem(plotit, "Re-calibrate ToT", did,TOTCAL);
          subplt = new ModItem(plotit, "Plot calib'ed ToT dist.", did,CLTOT);
        } else
          subplt = new ModItem(plotit,"Calibrate ToT", did,TOTCAL);
        plotit = new ModItem(datit,   "ToT peak finders",did,NONE);
        subplt = new ModItem(plotit,  "Run peak fit (slow!)",did,TOT_SUMM);
        subplt = new ModItem(plotit,  "Plot position of peak & distribution (fast ana)",did,TOT_FAST);
        if(iscal){
          subplt = new ModItem(plotit,  "Run cal. peak fit (slow!)", did,CAL_SUMM);
          subplt = new ModItem(plotit,  "Plot position of cal. peak & distribution (fast ana)", did,CAL_FAST);
        }
        plotit = new ModItem(datit,  "Run Source Analysis",    did,  DOSRC);
      }
      // hot pixel detection
      plotit = new ModItem(datit, "Hot pixel analysis",    did,  NONE);
      subplt = new ModItem(plotit,"Determine channels to be masked", did,MAKEMASK);
      subplt = new ModItem(plotit,"Determine mask & combine with other mask", did,COMBMASK);
      if(did->GetMap(-1,MAMASK)!=0){
        subplt = new ModItem(plotit,"Plot of channels to be masked", did,MAMASK);
        subplt = new ModItem(plotit,"Toggle selected pixel in mask", did,TOGMASK);
        subplt = new ModItem(plotit,"Write mask to file",            did,SAVEMASK);
      }
    }
    break;
  case MASKDAT:
    plotit = new ModItem(datit,  "Plot of masked channels",   did,MAMASK);
    plotit = new ModItem(datit,  "Toggle selected pixel in mask", did,TOGMASK);
    if(did->GetMap(-1,MAMASK)!=0 && did->GetMap(-1,MAMASK)->GetMaximum()>1)
      plotit = new ModItem(datit,  "Plot of selected defects",   did,BADSEL);
    break;
  case LEAK:
    plotit = new ModItem(datit,  "Plot leakage currents",    did,  LEAK_PLOT);
    plotit = new ModItem(datit,  "Plot MonLeak readings",    did,  MLPLOT);
    break;
  case TOTFR:
    if((did->GetLog()!=0 && did->GetLog()->MyScan->GetInner()=="VCal FE") ||
       did->GetLog()->MyScan->GetInner()=="VCal diff"){
      plotit = new ModItem(datit, "ToT vs VCAL",          did,NONE);
      subplt = new ModItem(plotit,"Plot ToT vs VCAL",     did,TOTRAW);
      subplt = new ModItem(plotit,"Fit all data",         did,FIT);
    }else{
      if(did->GetLog()!=0 && did->GetLog()->MyScan->GetInner()=="All FDACs"){
        plotit = new ModItem(datit, "ToT vs FDAC",          did,NONE);
        subplt = new ModItem(plotit,"Plot ToT vs FDAC",     did,TOTRAW);
        subplt = new ModItem(plotit,"Fit all data",         did,FIT);
        subplt = new ModItem(plotit,"Calculate FDAC",       did,GETFDAC);
      } else{
        plotit = new ModItem(datit, "Raw ToT data",         did,NONE);
        subplt = new ModItem(plotit,"Plot ToT vs scan par", did,TOTRAW);
        subplt = new ModItem(plotit,"Fit all data",         did,FIT);
        plotit = new ModItem(datit, "Hit data",             did,NONE);
        subplt = new ModItem(plotit,"Plot hits vs scan par",did,RAW);
      }
      int npar = 1;
      TF1 *f = RMain::GetFitFunc(did->m_fittype);
      if(f!=0) npar = f->GetNpar();
      plotit = new ModItem(datit, "Fit results",                      did,NONE);
      subplt = new ModItem(plotit,"Plot combination",                 did,PARCOMB);
      for(int i=0;i<npar;i++)
        subplt = new ModItem(plotit,"Plot par. " + QString::number(i),did,PARA+i);
      subplt = new ModItem(plotit,"Plot chi^2",                       did,CHI);
      break;
    } // continue with ToT cal fit
  case TOTF: 
    plotit = new ModItem(datit, "ToT-fit results",  did,NONE);
    subplt = new ModItem(plotit,"Plot chi^2",       did,CHI);
    subplt = new ModItem(plotit,"Plot ToT(Q_target) from fit par\'s",  did,T20K);
    if(did->GetLog()!=0 && !did->GetLog()->MyModule->CHigh)
      subplt = new ModItem(datit,"Combine with Chigh calib.",  did,COMBCAL);
    break;
  case TOTCOMB:
    //plotit = new ModItem(datit,  "ToT-fit results",  did,NONE);
    plotit = new ModItem(datit,"Plot Q vs ToT",                     did,PLTCCAL);
    plotit = new ModItem(datit,"Plot ToT distributions",            did,PLTCCPR);
    plotit = new ModItem(datit,"Fit combined data",                 did,FITTOTC);
    plotit = new ModItem(datit,"Modify Chi/Clo calibration",        did,RECALTOT);
    if(did->GetMap(-1,PARD)!=0)
      plotit = new ModItem(datit,"Plot Chi/Clo rel. cal. fit result", did,PARD);
    break;
  case TOTCF:
    //plotit = new ModItem(datit,  "ToT-fit results",  did,NONE);
    plotit = new ModItem(datit,"Plot chi^2",                        did,CHI);
    plotit = new ModItem(datit,"Plot ToT(Q_target) from fit par\'s",did,T20K);
    plotit = new ModItem(datit,"Plot Q vs ToT",                     did,PLTCCAL);
    plotit = new ModItem(datit,"Plot ToT distributions",            did,PLTCCPR);
    plotit = new ModItem(datit,"Re-fit combined data",              did,FITTOTC);
    plotit = new ModItem(datit,"Modify Chi/Clo calibration",        did,RECALTOT);
    if(did->GetMap(-1,PARD)!=0)
      plotit = new ModItem(datit,"Plot Chi/Clo rel. cal. fit result", did,PARD);
    plotit = new ModItem(datit,"Modify total calibration",          did,RECALFIT);
    break;
  case TFDAC:
    plotit = new ModItem(datit,  "Plot DAC map",         did,TDACP);
    plotit = new ModItem(datit,  "Write DAC map to file",did,TDACW);
    plotit = new ModItem(datit,  "Correlate to list of TDAC maps",   did,TDACCOR);
    break;
  case IVSCAN:
  case SENSORIV:
    plotit = new ModItem(datit,  "Plot IV curve (raw data)",  did,IVPLOT);
    plotit = new ModItem(datit,  "Plot T-corrected IV curve",  did,IVCORR);
    //plotit = new ModItem(datit,  "PDB File", did, PDB);
    break;
  case TWODFIT:  // combined inner + outer fit results
    {npar = 1;
    TF1 *f = RMain::GetFitFunc(did->m_fittype);
    if(f!=0) npar = f->GetNpar();
    plotit = new ModItem(datit,"Outer-fit results",                  did,NONE);
    subplt = new ModItem(plotit,"Plot combination",                 did,PARCOMB);
    for(int i=0;i<npar;i++)
      subplt = new ModItem(plotit,"Plot par. " + QString::number(i),did,PARA+i);
    subplt = new ModItem(plotit,"Plot chi^2",                       did,CHI);}
  case TWODSCAN:
    plotit = new ModItem(datit,  "S-fit results",         did,NONE);
    subplt = new ModItem(plotit,"Plot chi^2",            did,TWODCHI);
    subplt = new ModItem(plotit,"Plot sigma",            did,TWODSIG);
    subplt = new ModItem(plotit,"Plot mean",             did,TWODMEAN);
    label = "Options";
    if(did->GetLog()!=0){ // find type of scans
      if(did->GetLog()->MyScan->GetOuter()=="All TDACs")
        label = "Calculate TDAC";
      else if(did->GetLog()->MyScan->GetOuter()=="VCal FE" &&
        (did->GetLog()->MyScan->GetInner()=="MCC strobe delay" ||
         did->GetLog()->MyScan->GetInner()=="Strobe LE & TE" )&&
        !did->GetLog()->MyModule->CHigh)
        label = "Timewalk analysis";
      else if((did->GetLog()->MyScan->GetInner()=="VCal FE" ||
               did->GetLog()->MyScan->GetInner()=="VCal diff")&&
        (did->GetLog()->MyScan->GetOuter()=="DAC 8" ||
         did->GetLog()->MyScan->GetOuter()=="DAC 7"))
        label = "ITH-scan analysis";
      else if(did->GetLog()->MyScan->GetOuter()=="VCal FE" &&
        (did->GetLog()->MyScan->GetInner()=="MCC strobe delay"  ||
         did->GetLog()->MyScan->GetInner()=="Strobe LE & TE" )&&
        did->GetLog()->MyModule->CHigh)
        label = "";
    }
    if(label!="") plotit = new ModItem(datit,label,did,TWODOPT);
    plotit = new ModItem(datit,"Inner scan fit results vs outer par",did,NONE);
    subplt = new ModItem(plotit,"Fit",    did,FIT);
    if(did->GetLog()->MyScan->GetInner()=="VCal FE" ||
       did->GetLog()->MyScan->GetInner()=="VCal diff"){ // allow threshold/noise comp.
      subplt = new ModItem(plotit,"Options",did,REFD);
      subplt = new ModItem(plotit,"Plot threshold",    did,TWODRAW);
      subplt = new ModItem(plotit,"Plot noise",        did,TWODRAWS);
      subplt = new ModItem(plotit,"Plot chi^2",        did,TWODRAWC);
      plotit = new ModItem(datit,"Minimum threshold analysis",did,NONE);
      subplt = new ModItem(plotit,"Plot min. threshold",did,TWODMINT);
      subplt = new ModItem(plotit,"Plot noise at min. threshold",did,TWODMINTN);
      subplt = new ModItem(plotit,"Plot chi^2 at min. threshold",did,TWODMINTC);
    }else
      subplt = new ModItem(plotit,"Plot",   did,TWODRAW);
    break;
  case TWLK:{
    float dt0=20;
    if(datit->GetDID()->getAnaPar(1)>-999)
      dt0 = datit->GetDID()->getAnaPar(1);
    char label[300];
    sprintf(label,"Plot t0-%dns",(int)dt0);
    plotit = new ModItem(datit,label,      did,TWLK_TIME20NS_2D);
    sprintf(label,"Plot Q(t0-%dns)",(int)dt0);
    plotit = new ModItem(datit,label,  did,TWLK_Q20NS_2D);
    plotit = new ModItem(datit,"Plot overdrive (Q)",did,TWLK_OVERDRV_2D);
    plotit = new ModItem(datit,"Overdrive Profiles", did, NONE);
    subplt = new ModItem(plotit,"Normal pixels", did, TWLK_NORMAL_OVPROF);
    subplt = new ModItem(plotit,"Long pixels",did,TWLK_LONG_OVPROF);
    subplt = new ModItem(plotit,"Ganged pixels",did,TWLK_GANGED_OVPROF);
    subplt = new ModItem(plotit,"Long ganged pixels",did,TWLK_LGANGED_OVPROF);
    subplt = new ModItem(plotit,"All pixels",did,TWLK_ALL_OVPROF);
    plotit = new ModItem(datit,"View Clow and Chigh curve", did,TWLKGR);
    break;}
  case CHIPCURR:
    plotit = new ModItem(datit, "Plot FE currents",  did,IVPLOT);
    break;
  case BI_VIT_DATA: // (added 27/4/04 by LT)
    plotit = new ModItem(datit, "Plot V,I,T vs Time",  did, BI_VIT_VS_TIME_PLOT);
    plotit = new ModItem(datit, "Plot V,I vs NTC Temp",  did, BI_I_VS_T_PLOT);
    plotit = new ModItem(datit, "Plot summary of scans",  did, BI_SCANS_PLOT);
    break;
  case REGTEST:
    plotit = new ModItem(datit, "Show register info",  did, REGPANEL);
    break;
  case LVL1SHOT:
    plotit = new ModItem(datit, "Plot LVL1 distribution",         did,LVL1DIST);
    break;
  case LVL1SCAN:
    plotit = new ModItem(datit, "Plot LVL1 distribution",         did,LVL1DIST);
    plotit = new ModItem(datit, "Plot LVL1 dist vs scan par",     did,LVL12DPLT);
    break;
  case PIXDBF:{
#ifdef PIXEL_ROD // can link to PixScan
    PixDBData *pdbd = dynamic_cast<PixDBData*>(data_in);
    plotit = new ModItem(datit,  "Display config", data_in,NONE);
    subplt = new ModItem(plotit, "Display scan cfg", data_in,PSCONFIG);
    subplt = new ModItem(plotit, "Display module cfg", data_in,PMCONFIG);
    if(pdbd!=0 && pdbd->getBocConfig().name()!="blabla")
      subplt = new ModItem(plotit, "Display BOC cfg", data_in,PBCONFIG);
    subplt = new ModItem(plotit, "Plot TDAC map",   data_in,TDACLOG);
    subplt = new ModItem(plotit, "Plot FDAC map",   data_in,FDACLOG);
    subplt = new ModItem(plotit,"Plot readout mask",data_in,RMASKP);
    int nCol, nRow, nFe;
    pdbd->getGeom(nRow, nCol, nFe);
    if(nFe==16 && nCol==18 && nRow==160){ // FE-I3
      subplt = new ModItem(plotit,"Plot hitbus mask", data_in,HMASKP);
      subplt = new ModItem(plotit,"Plot strobe mask", data_in,SMASKP);
      subplt = new ModItem(plotit,"Plot preamp mask", data_in,PMASKP);
    } else{
      subplt = new ModItem(plotit,"Plot CAP1 mask", data_in,INJCAPH);
      subplt = new ModItem(plotit,"Plot CAP0 mask", data_in,INJCAPL);
      subplt = new ModItem(plotit,"Plot ILEAK mask", data_in,ILEAK);
    }
    PixLib::PixScan *ps = new PixLib::PixScan();
    for(std::map<std::string, int>::iterator it = ps->getHistoTypes().begin();
	it!=ps->getHistoTypes().end(); it++){
      if(pdbd!=0 && pdbd->haveHistoType((PixLib::PixScan::HistogramType)it->second) && 
	 (PixLib::PixScan::HistogramType)it->second!=PixLib::PixScan::DCS_DATA) // exclude DCS_DATA type, dealt with via graphs below
	plotit = new ModItem(datit,("Plot "+it->first).c_str(),data_in,(PIXDBTYPES+it->second));
    }
    if(pdbd->GetGraph()!=0) plotit = new ModItem(datit, "Plot Graph", data_in,IVPLOT);
    delete ps;
#endif
    break;}
  default:
    break;
  }
  return datit;
}
void TopWin::LoadRoot(ModItem* item, const char* path, bool showlist, const char *subpath, bool *loadData){
  if(item==0) return;
  // get NSLOTS from StdTestWin
  StdTestWin tmpwin(this,"tmptestwin");
  int NSLOTS = tmpwin.NSLOTS;
  int i,j,k,refid, reftype, n, error;
  std::vector<int> exists;
  exists.resize(NSLOTS);
  for(i=0;i<NSLOTS;i++) exists[i]=1;
  exists[14] = 0; // want to run source analysis & calib in any case
  DatSet *did;
  ModItem *Mit;
  std::vector<ModItem*> all_arr, Mit_arr;
  Mit_arr.resize(NSLOTS);
  Q3ListViewItem *dlitem;
  const int nRefs=10;
  DatSet *refs[nRefs];
  for(i=0;i<NSLOTS;i++)
    Mit_arr[i] = 0;
  char fname[20];
  QString mname, orgpath, lastname, currname;
  TDirectory *moddir;

  textwin->setText("");
  // store original directory to root memory
  orgpath = gDirectory->GetPath();
  TFile *f = 0;
  try{
    f = new TFile(path);
  }catch(...){
    if(options->m_Opts->m_showwarn) 
      QMessageBox::warning(this,"MA TopWin::LoadRoot","File "+QString(path)+
			   " seems to be corrupt, can't load.");
    //    f->Close();
    return;
  }
  if(subpath!=0) f->cd(subpath); //muli-module file, cd to module folder first
  moddir = gDirectory;
  TObject *tmpro, *str1, *str2, *str3;
  tmpro = moddir->Get("ndat");
  if(tmpro!=0)
    sscanf(tmpro->GetName(),"%d",&n);
  else{
    n=0;
    TIter next(moddir->GetListOfKeys());
    TKey *key;
    while((key=(TKey*)next())){
      std::string class_name = key->GetClassName();
      if(class_name=="TDirectory"){
        n++;
      }
    }
  }
  all_arr.resize(n);
  tmpro = moddir->Get("module name");
  if(tmpro!=0)
    mname = tmpro->GetName();
  else
    mname = moddir->GetName();
  if(mname!=item->text(0).latin1()){
    mname = "Module name is different in list and data file:\n" +
                item->text(0) + ", " + mname;
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",mname);
  }
  tmpro = f->Get("version");
  bool mod_index = false;
  if(tmpro!=0) {
    mod_index = DatSet::OlderVersion(tmpro->GetName(),"2.4.9"); // changed FileTypes with version 2.4.9
    if(DatSet::OlderVersion(tmpro->GetName(),"3.5.7"))
      exists[21] = 0; // want to create overdrive item for versions before 3.5.7
  }

  RootDataList dl(this,"rootdl",TRUE);
  dl.moduleLabel->setText("Data for module "+item->text(0));
  TObject *tmpobj;
  tmpobj = moddir->Get("comment");
  if(tmpobj!=0)
    dl.Comments->setText(tmpobj->GetName());
  for(i=0;i<n;i++){
    sprintf(fname,"data-%d",i);
    tmpobj=0;
    try{
      moddir->cd(fname);
      tmpobj = gDirectory->Get("dtname");
    }catch(...){
      tmpobj=0;
    }
    if(tmpobj==0){
      if(options->m_Opts->m_showwarn) 
	QMessageBox::warning(this,"MA TopWin::LoadRoot","File "+QString(path)+
			     " seems to be corrupt, can't load.");
      //f->Close();
      return;
    }
    dl.LoadName(tmpobj->GetName(), (long int)gDirectory);
  }

  if(showlist){
    if(dl.exec()==0){
      f->Close();
      return;
    }
    dlitem = dl.FileList->firstChild();    
    while(dlitem!=0){
      if(dlitem->isSelected()){ // check for references
        ((TDirectory*)dl.loadedItems[(long int)dlitem])->cd();
        for(j=0;j<500;j++){
          sprintf(fname,"refid;%d",1+j);
          str1 = gDirectory->Get(fname);
          if(str1!=0){
            Q3ListViewItem *ritem = dl.FileList->firstChild();
            while(ritem!=0){
              if(ritem->text(1)==str1->GetName()) break;
              ritem = ritem->nextSibling();
            }
            if(ritem!=0 && !ritem->isSelected() && options->m_Opts->m_showwarn) {
              QMessageBox select("Select reference",
                                 "Item\"" + dlitem->text(0) + "\" refers to data set\n\""
                                 + ritem->text(0) + "\" which has not been selected.\n Select now? ",
                                 QMessageBox::Question,QMessageBox::Yes, QMessageBox::No, Qt::NoButton,
                                 this, "selectref",TRUE,Qt::WType_TopLevel);
              if(select.exec()==QMessageBox::Yes){
                dl.FileList->setSelected(ritem,true);
              }
            }
          }
        }
      }
      dlitem = dlitem->nextSibling();
    }
  } else
    dl.SelectAll(true);

  QApplication::setOverrideCursor(Qt::waitCursor);
  for(i=0;i<n;i++){
    sprintf(fname,"data-%d",i);
    moddir->cd(fname);
    dlitem = (Q3ListViewItem*) dl.loadedItemsInv[(long int)gDirectory];
    bool isInList = true;
    if(loadData!=0){
      int stdType=-1;
      TObject *tmpobj = gDirectory->Get("stdtest");
      isInList = false;
      if(tmpobj!=0){
        sscanf(tmpobj->GetName(),"%d",&stdType);
        stdType = stdType%100; // strip off test type, just keep data type
        if(stdType<NSLOTS && stdType>=0)
          isInList = loadData[stdType];
      }
    }
    if((!showlist || (dlitem !=0 && dlitem->isSelected())) && isInList){
      did = new DatSet(orgpath.latin1(),&error);
      if(error<0 && options->m_Opts->m_showwarn)
        QMessageBox::warning(this,"Module Analysis",DatSet::ErrorToMsg(error).c_str());
      if(error==0){
        QString mytxt = "loaded from ROOT: ";
        mytxt += +did->GetName();
        textwin->setText(mytxt);
        textwin->repaint();
        if(m_app!=0)
          m_app->processEvents();
      }
      Mit = DataToTree(item, did);
      all_arr[i] = Mit;
      int type = did->GetStdtest()%100;
      if(type>=0 && type<NSLOTS)
        Mit_arr[type] = Mit;
      if(m_app!=0)
        m_app->processEvents();
    }
  }

  // restore reference info if there is any
  
  textwin->setText("...implementing internal references...");
  textwin->repaint();
  if(m_app!=0)
    m_app->processEvents();

  for(i=0;i<n;i++){
    sprintf(fname,"data-%d",i);
    moddir->cd(fname);
    lastname="";
    k=0;
    if(all_arr[i]!=0){
      for(j=0;j<500;j++){
        sprintf(fname,"refid;%d",j+1);
        str1 = gDirectory->Get(fname);
        //if(str1==0) break;
        sprintf(fname,"reftype;%d",j+1);
        str3 = gDirectory->Get(fname);
        sprintf(fname,"refnam;%d",j+1);
        str2 = gDirectory->Get(fname);
        if(str2!=0)
          currname = str2->GetName();
        else 
          currname = "";
        if(lastname!=currname && j>0){
          if(lastname=="View Clow and Chigh curve" || lastname=="Plot Q vs ToT" || lastname=="Plot ToT distributions"
             || lastname=="Fit combined data" || lastname=="Re-fit combined data" || lastname=="Modify Chi/Clo calibration"
             || lastname=="Plot calib'ed ToT dist." || lastname=="Plot position of cal. peak & distribution (fast ana)" 
             || lastname=="Plot position of cal. peak & distribution (fitted)" || lastname=="Run cal. peak fit (slow!)"){
            //find already exisiting item
            Mit = (ModItem*) all_arr[i]->firstChild();
            while(Mit!=0){
              // check sub-items too
              ModItem *subMit = (ModItem*) Mit->firstChild();
              while(subMit!=0){
                if(subMit->text(0)==lastname){
                  Mit = subMit;
                  break;
                }
                subMit = (ModItem*)subMit->nextSibling();
              }
              if(Mit->text(0)==lastname) break;
              Mit = (ModItem*)Mit->nextSibling();
            }
            if(Mit==0)
              Mit = new ModItem(all_arr[i],lastname.latin1(),all_arr[i]->GetDID(),reftype);
          }else
            Mit = new ModItem(all_arr[i],lastname.latin1(),all_arr[i]->GetDID(),reftype);
          Mit->SetRefArr(refs,k);
          k=0;
        }
        if(str1==0) break;
        sscanf(str1->GetName(),"%d",&refid);
        sscanf(str3->GetName(),"%d",&reftype);
        if(mod_index) reftype += 500;
        if(all_arr[refid]!=0){
          refs[k] = all_arr[refid]->GetDID();
          k++;
        }
        lastname=currname;
      }
    }
  }
  
  //close file and process stored structure
  f->Close();
  textwin->setText("...done with loading");
  textwin->repaint();
  if(m_app!=0)
    m_app->processEvents();
  ProcStd(Mit_arr,exists);
  QApplication::restoreOverrideCursor();
  return;
}
void TopWin::RemData(ModItem *datit, bool do_confirm){
  if(datit==0) return;
  ModItem *parnt = datit->ModParent();
  if(parnt!=0 && parnt->m_lock){
    QMessageBox::warning(this,"TopWin::RemData","Module-item "+parnt->text(0)+" is in use\n"
                         "and none of its data can be deleted");
    return;
  }
  if(datit->m_lock){
    QMessageBox::warning(this,"TopWin::RemData","Data-item "+datit->text(0)+"\n"
                         "is in use and can't be deleted");
    return;
  }
  //  ModItem *test1, *test2, *test3;
  //  int error;
  if(datit==0 || parnt==0) return;
  textwin->setText("");
  // check for plot items that refer to this data set
//   error=0;
//   test1 = (ModItem*) parnt->firstChild();
//   while(test1!=0){
//     if(test1!=datit){
//       if(test1->GetRID() == datit->GetDID())
//         error++;
//       test2 = (ModItem*) test1->firstChild();
//       while(test2!=0){
//         if(test2->GetRID() == datit->GetDID())
//           error++;
//         test3 = (ModItem*) test2->firstChild();
//         while(test3!=0){
//           if(test3->GetRID() == datit->GetDID())
//             error++;
//           test3 = (ModItem*) test3->nextSibling();
//         }
//         test2 = (ModItem*) test2->nextSibling();
//       }
//     }
//     test1 = (ModItem*) test1->nextSibling();
//   }
  if(datit->checkForRefs().size()>0)   // error>0)
    QMessageBox::warning(this,"Module Analysis",
                         "Can't delete this data set, it is referred to by others");
  else{
    // warn user and remove item if confirmed
    bool go_ahead=true;
    if(do_confirm){
      QMessageBox ays("Confirm delete",
                      "This will delete data set\n\"" + datit->text(0) + "\"\nAre you sure?",
                      QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, Qt::NoButton,
                      this, "ays",TRUE,Qt::WType_TopLevel);
      go_ahead = (ays.exec()==QMessageBox::Ok);
    }
    if(go_ahead){
      QApplication::setOverrideCursor(Qt::waitCursor);
      DatSet *mydata = datit->GetDID();
      delete datit;
      delete mydata;
      QApplication::restoreOverrideCursor();
    }
  }
  QApplication::restoreOverrideCursor();
  return;
}
void TopWin::RemRaw(ModItem *datit, bool do_prompt){
  if(datit==0) return;
  ModItem *test;
  bool go_for_it=true;
  // warn user and remove item if confirmed
  if(do_prompt){
    QMessageBox ays("Confirm delete",
                    "This will remove raw data from\n\"" + datit->text(0) + "\"\nAre you sure?",
                    QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, Qt::NoButton,
                    this, "ays",TRUE,Qt::WType_TopLevel);
  if(ays.exec()!=QMessageBox::Ok)
    go_for_it = false;
  }
  if(go_for_it){
    QApplication::setOverrideCursor(Qt::waitCursor);
    datit->GetDID()->ClearRaw();
    // remove raw-data item in list
    test = (ModItem*)datit->firstChild();
    while(test!=0){
      if(test->text(0)=="Raw data" || 
         test->text(0)=="Raw ToT data" ||
         test->text(0)=="Scan data" ||
         test->text(0)=="ToT vs VCAL" ||
         test->text(0)=="ToT vs FDAC" ||
         test->text(0)=="DAC fit histos") {
        delete test;
        test = 0;
      } else
        test = (ModItem*)test->nextSibling();
    }
    datit->GetDID()->m_isscan = 0;
    switch(datit->GetPID()){
    case ONEDFIT:
      datit->ChgPID(ONEDSCAN);
      break;
    case TOTFR:
      datit->ChgPID(TOTF);
      break;
    default:
      break;
    }
  }
  QApplication::restoreOverrideCursor();
  return;
}
void TopWin::Options_clicked(){
  options->show();
  return;
}
void TopWin::BiasStab(ModItem *item){
  if(item==0) return;
  int supp = options->m_Opts->m_BIASsupp;
  int chan = options->m_Opts->m_BIASchan;
  std::vector<ModItem*> data_items;
  ModItem *loopit = (ModItem*)item->firstChild();
  while(loopit!=0){
    if(loopit->GetDID()->GetLog()!=0 && loopit->GetDID()->GetLog()->MyDCS->HVvolt[supp][chan]>50)
      data_items.push_back(loopit);
    loopit = (ModItem*) loopit->nextSibling();
  }
  int error = rootmain->PlotBiasStab(data_items);
  if(error){
    QString tmp;
    tmp =  "Error plotting: ";
    tmp += RMain::ErrorToMsg(error).c_str();
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
  }
  return;
}
void TopWin::CompareBadChans(ModItem* item){
  if(item==0) return;
  int i,j, k, error, chip=-1;
  if(ChipOrMod->isOn()) chip = ChipSel->value();;
  MaskedSel selwin(item, this,"badchannel", TRUE, Qt::WType_TopLevel, 1);
  selwin.MaskTypeBox->setChecked(false);
  if(selwin.exec()==1){
    // get selection
    i = selwin.FirstSel->currentItem();
    k = selwin.SecondSel->currentItem();
    // create mask for item k
    DatSet *tmpdat;
    ModItem *tmpit;
    tmpdat = new DatSet(selwin.alist[k]->GetDID()->GetName(),
                        MASKDAT,false,0,&error);
    int  tmpBPT = options->m_Opts->m_noisetab;
    bool tmpNC  = options->m_Opts->m_showcutpix;
    options->m_Opts->m_showcutpix = false;
    options->m_Opts->m_noisetab   = 0;
    //    ModuleList_itemselected(selwin.alist[k]);
    TH2F *tmphi = 0;
    j = selwin.alist[k]->GetPID();
    int sel_cuts=0;
    if(j==BADSEL || selwin.aplotid[i]==BADSEL){
      bool use_defcuts=false;
      if(m_cutpanel->ClassList->childCount()<=0){
        m_cutpanel->NewClass();
        m_cutpanel->RenameEdit->setText("temp cuts");
        m_cutpanel->RenameClass();
        use_defcuts = true;
      }
      SelectionPanel selpan(this,"selpan");
      for(int i=0; i<NCUTS;i++){
        selpan.AddCheckBox(m_cutpanel->GetCutPars(i)->label.c_str());
      }
      sel_cuts=selpan.exec();
      if(sel_cuts<0) sel_cuts=0;
      if(use_defcuts) // if we created default cuts because nothing was there we have to clean up
        m_cutpanel->DeleteClass();
    }
    if(j==LOOSE)
      tmphi = rootmain->GetComp(selwin.alist[k],-1,-1,-1,-1);
    else if(j==DEAD_CHAN)
      tmphi = selwin.alist[k]->GetDID()->GetMap(-1,DEAD_CHAN);
    else if(j==BADSEL)
      tmphi = rootmain->GetBadSelMap(selwin.alist[k]->GetDID()->GetMap(-1,MAMASK),sel_cuts);
    else{
      QMessageBox::warning(this,"bad selection","Bad channel comparison only\n"
                           "works for bad-channel masks, source and HV-off data");
      return;
    }
    tmpdat->CreateMask(tmphi,2);
    options->m_Opts->m_showcutpix = tmpNC;
    options->m_Opts->m_noisetab   = tmpBPT;
    tmpit = DataToTree(item, tmpdat);
    // reset last PID and DID
    m_lastPitem = 0;
    m_lastPdbID = -1;
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
    rootmain->ClearMemory();
    // plot with mask applied
    tmpBPT = options->m_Opts->m_zerotab;
    options->m_Opts->m_zerotab = false;
    error = rootmain->PlotMasked(selwin.alist[i], tmpit, selwin.aplotid[i], MAMASK, 
                                 selwin.MaskTypeBox->isChecked(),chip,sel_cuts);
    options->m_Opts->m_zerotab = tmpBPT;
    delete tmpit;
    delete tmpdat;
    if(error){
      QString tmp;
      tmp =  "Error plotting: ";
      tmp += RMain::ErrorToMsg(error).c_str();
      if(options->m_Opts->m_showwarn)
        QMessageBox::warning(this,"Module Analysis",tmp);
    }
    rootmain->m_lastmap = rootmain->m_currmap;
    rootmain->m_currmap = 0;
  }
  return;
}
void TopWin::PlotCorrel(){
  int i,k, error;
  int chip=-1;
  if(ChipOrMod->isOn())
    chip = ChipSel->value();
  std::vector<ModItem*> moditems = getAllModuleItems();
  CorrelSel crlwin(moditems, this,"Correlation Data Sets", TRUE, Qt::WType_TopLevel);
  if(crlwin.exec()==1){
    // reset last PID and DID
    m_lastPitem = 0;
    m_lastPdbID = -1;
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
    rootmain->ClearMemory();
    rootmain->m_lastmap = rootmain->m_currmap;
    rootmain->m_currmap = 0;
    // now the actual correl. plot
    i = crlwin.FirstSel->currentItem();
    k = crlwin.SecondSel->currentItem();
    bool doScatter = !(bool)crlwin.PlotType->currentItem();
    int plotType = crlwin.PlotType->currentItem();
    if(plotType>1)
      plotType--;
    if(crlwin.SecondMod->currentText()=="All other modules"){
      // prepare table
      QString toptitle;
      toptitle = "List of correlation coefficients to module "+crlwin.FirstMod->currentText()+" - "+
        crlwin.m_inlistA[i]->text(0);
      BadPixel *badpix = new BadPixel(0,0,"badpix",FALSE,Qt::WDestructiveClose);
      badpix->setCaption("Correlation summary");
      badpix->Init("not used",toptitle.latin1());
      // do non-standard settings by hand
      badpix->PixelList->setNumCols(1);
      badpix->PixelList->setNumRows(17);
      badpix->MaskButt->hide();
      int chipID;
      for(chipID=0;chipID<NCHIP;chipID++)
        badpix->PixelList->setText(chipID,0,QString::number(chipID));
      badpix->PixelList->setText(NCHIP,0,"module");
      // loop over all modules
      int nMods=0;
      std::vector<ModItem*> allMods = getAllModuleItems();
      for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
        if((*IT)->text(0)!=crlwin.FirstMod->currentText()){ // different module from the selected one
          ModItem *simDID = getSimilarData((*IT),crlwin.m_inlistA[i]->GetDID()->GetStdtest());
          if(simDID!=0){
            nMods++;
            badpix->PixelList->setNumCols(1+nMods);
            badpix->PixelList->horizontalHeader()->setLabel(nMods,(*IT)->text(0));
            // get correlation/RMS per chip
            float result;
            for(chipID=0;chipID<NCHIP+1;chipID++){
              if(chipID<NCHIP)
                chip = chipID;
              else
                chip = -1;
              error = rootmain->PlotCorrel(crlwin.FirstMod->currentText().latin1(),crlwin.m_inlistA[i],simDID,
                                           crlwin.m_plotidA[i], crlwin.m_plotidA[i],
                                           1,Scanpt0->value(), chip, false, pixScanLevel->value());
              result = -1;
              if(!error){
                TH2F *tmphi = (TH2F*) gROOT->FindObject("corrplt");
                if(tmphi) result = tmphi->GetCorrelationFactor();
              } else
                printf("Error in RMain::PlotCorrel: %d\n",error);
              badpix->PixelList->setText(chipID,nMods,QString::number(result,'f',3));
            }
          }
        }
      }
      badpix->show();
    } else if(crlwin.PlotType->currentItem()!=2 && crlwin.PlotType->currentItem()!=4){ //plot
      if(crlwin.m_plotidA[i]==OUTPAR || crlwin.m_plotidB[k]==OUTPAR){ // special treatment for this case
        if((crlwin.m_plotidA[i]==OUTPAR && (crlwin.m_plotidB[k]<TWODMEAN || crlwin.m_plotidB[k]>TWODCHI))||
           (crlwin.m_plotidB[k]==OUTPAR && (crlwin.m_plotidA[i]<TWODMEAN || crlwin.m_plotidA[i]>TWODCHI)))
          if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",
                                                               "Outer scan parameter can only be plotted against 2d mean or sigma");
        if(crlwin.m_plotidA[i]==OUTPAR)
          error = rootmain->Plot2DCorrel(crlwin.FirstMod->currentText().latin1(),crlwin.m_inlistB[k]->GetDID(),crlwin.m_plotidB[k],
                                         doScatter);
        else
          error = rootmain->Plot2DCorrel(crlwin.FirstMod->currentText().latin1(),crlwin.m_inlistA[i]->GetDID(),crlwin.m_plotidA[i],
                                         doScatter);
        if(error){
          QString tmp;
          tmp =  "Error plotting: ";
          tmp += RMain::ErrorToMsg(error).c_str();
          if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
        }
      } else{
        QString modLabel = crlwin.FirstMod->currentText();
        if(crlwin.SecondMod->currentText()!=modLabel)
          modLabel += " vs " + crlwin.SecondMod->currentText();
        error = rootmain->PlotCorrel(modLabel.latin1(),crlwin.m_inlistA[i],crlwin.m_inlistB[k], 
                                     crlwin.m_plotidA[i], crlwin.m_plotidB[k],
                                     plotType,Scanpt0->value(), chip, true, pixScanLevel->value());
        if(error){
          QString tmp;
          tmp =  "Error plotting: ";
          tmp += RMain::ErrorToMsg(error).c_str();
          if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
        }
      }
    } else { // tabulate correl coeff. or RMS
      if(plotType>2) plotType--; // plotType = 1:correl, 2: RMS
      // prepare table
      QString toptitle;
      if(plotType==1)
        toptitle = "List of correlation coefficients per chip, data sets \n\"";
      else
        toptitle = "List of difference RMS per chip, data sets \n\"";
      toptitle += crlwin.m_inlistA[i]->GetDID()->GetName();
      toptitle += "\", module ";
      toptitle += crlwin.FirstMod->currentText();
      toptitle += " and ";
      toptitle += crlwin.m_inlistB[k]->GetDID()->GetName();
      toptitle += "\", module ";
      toptitle += crlwin.SecondMod->currentText();
      BadPixel *badpix = new BadPixel(0,0,"badpix",FALSE,Qt::WDestructiveClose);
      badpix->setCaption("Correlation summary");
      badpix->Init("not used",toptitle.latin1());
      // do non-standard settings by hand
      if(plotType==1)
        badpix->PixelList->horizontalHeader()->setLabel(1,"corr.coeff");
      else
        badpix->PixelList->horizontalHeader()->setLabel(1,"diff.RMS");
      badpix->PixelList->horizontalHeader()->setLabel(2,"not used");
      badpix->PixelList->setNumRows(17);
      badpix->MaskButt->hide();
      // get correlation/RMS per chip
      float result;
      QString modLabel = crlwin.FirstMod->currentText();
      if(crlwin.SecondMod->currentText()!=modLabel)
        modLabel += " vs " + crlwin.SecondMod->currentText();
      for(int chipID=0;chipID<NCHIP+1;chipID++){
        if(chipID<NCHIP)
          chip = chipID;
        else
          chip = -1;
        error = rootmain->PlotCorrel(modLabel.latin1(),crlwin.m_inlistA[i],crlwin.m_inlistB[k], 
                                     crlwin.m_plotidA[i], crlwin.m_plotidB[k],
                                     plotType,Scanpt0->value(), chip, false);
        result = -1;
        if(!error){
          if(plotType==1){
            TH2F *tmphi = (TH2F*) gROOT->FindObject("corrplt");
            if(tmphi) result = tmphi->GetCorrelationFactor();
          }else{
            TH1F *tmphi = (TH1F*) gROOT->FindObject("diffplt");
            if(tmphi) result = tmphi->GetRMS();
          }
        }
        if(chipID<NCHIP)
          badpix->PixelList->setText(chipID,0,QString::number(chip,'f',0));
        else
          badpix->PixelList->setText(chipID,0,"module");
        badpix->PixelList->setText(chipID,1,QString::number(result,'f',3));
      }
      badpix->PixelList->setNumCols(2);
      badpix->show();
    }
  }
  return;
}
void TopWin::LoadStd(ModItem *item){
  if(item==0) return;
  StdTestWin stdwin(this,"Load Std Files", TRUE, Qt::WType_TopLevel, item);
  stdwin.ModuleName->setText(item->text(0));

  // show panel and get file names
  if(stdwin.exec()==1){
    MAEngine::LocalLoadStd(this, item,stdwin,m_app, textwin);
  }
  return;
}
void TopWin::ProcStd(std::vector<ModItem*> itemarr, std::vector<int> exists){
  if(itemarr.size()<22) return;

  ModItem *intVCAL, *extVCAL, *XTalk, *noHV, *wHV, *HotPix, *Src, *Mit;
  ModItem *Twlklo, *Twlkhi, *tmpit=0, *Intscan=0;
  noHV    = itemarr[1];  // scan w/o HV
  wHV     = itemarr[2];  // untuned scan w/ HV
  intVCAL = itemarr[5];  // internal tuned scan
  extVCAL = itemarr[6];  // external tuned scan
  XTalk   = itemarr[7];  // xtalk scan
  HotPix  = itemarr[13]; // hot pixel scan
  Src     = itemarr[14]; // source scan
  Twlklo  = itemarr[11]; //time walk (Clo)
  Twlkhi  = itemarr[12]; //time walk (Clo)
  Intscan = itemarr[21]; // in-time threshold scan

  // alternatives for HVon scan:
  if(wHV==0){
    if(extVCAL!=0 && extVCAL->GetDID()->GetLog()!=0 && extVCAL->GetDID()->GetLog()->MyModule->GetInjection()=="Internal")
      wHV = extVCAL;
    else if(intVCAL!=0)
      wHV = intVCAL;
  }

  // process cross-references
  if(XTalk!=0 && XTalk->GetDID()->GetLog()!=0 && XTalk->GetDID()->GetLog()->MyModule->GetInjection()=="Internal"
     && intVCAL!=0 && exists.size()>0 && (exists[7]==0 || exists[5]==0)){
    Mit = new ModItem(XTalk,"Plot cross-talk fraction", XTalk->GetDID(),
                      XTFR,intVCAL->GetDID());
    Mit = new ModItem(XTalk,"Plot merged bumps map (noise)", XTalk->GetDID(),
                      SHORT,intVCAL->GetDID());
  }
  if(Intscan!=0 && Intscan->GetDID()->GetLog()!=0 &&
     intVCAL!=0 && exists.size()>0 && (exists[21]==0 || exists[5]==0)){
    Mit = new ModItem(Intscan,"Plot overdrive", Intscan->GetDID(),
                      TDIF,intVCAL->GetDID());
  }
  if(extVCAL!=0 && extVCAL->GetDID()->GetLog()->MyModule->GetInjection()=="External" && intVCAL!=0
     && exists.size()>0 && (exists[5]==0 || exists[6]==0)){
    Mit = new ModItem(extVCAL,"Plot noise ratio to int", 
                      extVCAL->GetDID(),NRAT,intVCAL->GetDID());
    Mit = new ModItem(extVCAL,"Plot threshold ratio to int", 
                      extVCAL->GetDID(),TRAT,intVCAL->GetDID());
  }
  if(noHV!=0 && wHV!=0 && exists.size()>0 && (exists[1]==0 || exists[2]==0))
    Mit = new ModItem(noHV,"Plot unconn. bump map (noise diff.)", 
                      noHV->GetDID(),LOOSE,wHV->GetDID());

  if(noHV!=0 && wHV==0 && intVCAL==0 && exists.size()>0 && (exists[1]==0))
    Mit = new ModItem(noHV,"Plot unconn. bump map (low noise)", 
                      noHV->GetDID(),LOOSE,noHV->GetDID());

  // make item with combined ToT calib. if Clow *and* Chigh exist
  if(itemarr[8]!=0 && itemarr[9]!=0 && itemarr[19]==0){
    DatSet *rawdat[2]={itemarr[8]->GetDID(),itemarr[9]->GetDID()}, *combcal;
    int error=-15;
    int test_type = itemarr[8]->GetDID()->GetStdtest()/100;
    QString tmp;
    combcal = new DatSet("ToT calibration - combined",rawdat,2,TOTCOMB,&error,0,0);
    if(error){
      tmp.sprintf("Error in analysis: %s",DatSet::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    } else{
      itemarr[19] = DataToTree((ModItem*)itemarr[8]->parent(),combcal);
      Mit = (ModItem*) itemarr[19]->firstChild();
      while(Mit!=0){
        if(Mit->text(0)=="Plot Q vs ToT"  || Mit->text(0)=="Plot ToT distributions" 
             || Mit->text(0)=="Fit combined data" || Mit->text(0)=="Modify Chi/Clo calibration")
          Mit->SetRefArr(rawdat,2);
        if(Mit->text(0)=="Fit combined data") tmpit = Mit;
        Mit = (ModItem*) Mit->nextSibling();
      }
      if(itemarr[19]!=0) itemarr[19]->GetDID()->SetStdtest(19+100*test_type);
    }
  }
  // run fit if not done yet and required to do so
  if(itemarr[19]!=0 && itemarr[19]->GetDID()->m_istype==TOTCOMB && exists[19]==-1){
    DatSet *rawdat[2]={itemarr[8]->GetDID(),itemarr[9]->GetDID()};
    int error;
    int test_type = itemarr[8]->GetDID()->GetStdtest()/100;
    QString tmp;
    if(options->m_Opts->m_doTotRelcal){
      textwin->setText("Fitting Chi/Clo rel. ToT calib. now, please be patient...");
      textwin->repaint();
      if(m_app!=0)
        m_app->processEvents();
      QApplication::setOverrideCursor(Qt::waitCursor);
      gSystem->Sleep(300);
      float totmin=0, totmax=-1;
      if(options->m_Opts->m_fixedToTSwitch){
        totmax = 1e3*(float)options->m_Opts->m_ToTSwitch;
        totmin = 1e3*(float)options->m_Opts->m_QPlotPnt;
      }
      if((error = itemarr[19]->GetDID()->TotRelCalFit(rawdat, itemarr[19]->GetDID()->m_ChiCloCal,totmin,totmax))){
        QApplication::restoreOverrideCursor();
        tmp.sprintf("Error while fitting: %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
        textwin->setText("...aborted.");
      } else{
        QApplication::restoreOverrideCursor();
        textwin->setText("...done!");
      }
    }
    itemarr[19] = FitToTComb(tmpit);
    if(itemarr[19]!=0) itemarr[19]->GetDID()->SetStdtest(19+100*test_type);
  }
  
  // calibrate source scan
  if(Src!=0 && Src->GetDID()->m_iscal==0){
    if(itemarr[19]!=0 && itemarr[19]->GetDID()->m_istype==TOTCF && exists.size()>0 && exists[19]<=0)
      CalibrateToT(Src,itemarr[19]);
    else if(itemarr[8]!=0 && exists.size()>0 && exists[8]==0)
      CalibrateToT(Src,itemarr[8]);
    else if(itemarr[9]!=0 && exists.size()>0 && exists[9]==0)
      CalibrateToT(Src,itemarr[9]);
  }
  

  // make mask
  if(HotPix!=0 && exists.size()>0 && exists[13]==0){
    Mit = (ModItem*)HotPix->firstChild();
    if(Mit!=0) Mit = (ModItem*)Mit->firstChild();
    while(Mit!=0){
      if(Mit->GetPID()==MAKEMASK) ModuleList_itemselected(Mit);
      Mit = (ModItem*)Mit->nextSibling();
    }
  }
  // run source analysis
  if(Src!=0 && exists.size()>0 && exists[14]==0){
    ModItem *tmpit;
    Mit = (ModItem*)Src->firstChild();
    while(Mit!=0){
      tmpit = Mit;
      Mit = (ModItem*)Mit->nextSibling();
      if(tmpit->GetPID()==DOSRC){
        ModuleList_itemselected(tmpit);
        break;
      }
    }
  }
  // do time walk analysis if all three data sets are there an it hasn't been done yet
  if(Twlklo!=0 && Twlkhi!=0 && itemarr[15]==0){
    textwin->setText("Performing timewak analysis...");
    DatSet *rawdat[3];
    int err=-15;
    int test_type = Twlklo->GetDID()->GetStdtest()/100;
    rawdat[1] = Twlklo->GetDID();
    rawdat[2] = Twlkhi->GetDID();
    for(int i=1;i<3;i++)
      if(rawdat[i]->GetLog()==0) return;
    DatSet *tw=0;
    rawdat[0]=0;
    if(itemarr[18]!=0)
      rawdat[0] = itemarr[18]->GetDID();
    else{
      if(rawdat[1]->GetLog()->MyModule->GetInjection()=="Internal" && intVCAL!=0){
        rawdat[0] = intVCAL->GetDID();
      }
    }
    float anapar[1]={(float)options->m_Opts->m_QhighTW};
    if(rawdat[0]!=0) tw = new DatSet("Time walk analysis",rawdat,3,TWLK,&err,anapar,1);
    if(err){
      textwin->setText("...aborted");
      QString tmp;
      tmp.sprintf("Error doing time-walk analysis: %s",DatSet::ErrorToMsg(err).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
    } else{
      tw->SetStdtest(15+100*test_type);
      Mit = DataToTree((ModItem*) Twlklo->parent(),tw);
      Mit = (ModItem*) Mit->firstChild();
      while(Mit!=0){
        if(Mit->GetPID()==TWLKGR)
          Mit->SetRefArr(rawdat,3);
        Mit = (ModItem*) Mit->nextSibling();
      }
      textwin->setText("...done");
    }
  }

  // T0 analysis
  if(itemarr[34]!=0 && itemarr[35]!=0){
    Mit = new ModItem(itemarr[34],"Plot delay scale factor", itemarr[34]->GetDID(),
                      DELCAL, itemarr[35]->GetDID());
    Mit = new ModItem(itemarr[34],"Plot delay difference", itemarr[34]->GetDID(),
                      TIMEDIF, itemarr[35]->GetDID());
    Mit = new ModItem(itemarr[34],"Plot delay difference (calib'ed)", itemarr[34]->GetDID(),
                      TCALDIF, itemarr[35]->GetDID());    
  }

  return;
}
void TopWin::MapAction(int chip, int col, int row, float con, int event){
  QString txtstr;
  //  int chip,col, row;
  if(m_lastPitem==0) return;
  switch(event){
  case kButton1Down:
    if(m_lastchip<0) ChipSel->setValue(chip);
    ColSel->setValue(col);
    RowSel->setValue(row);
    txtstr.sprintf("%.3f",con);
    PixVal->setText(txtstr);
    break;
  case kButton1Double:{
    if(m_lastPitem->GetPID()==TOT_SUMM)
      m_lastPitem->ChgPID(TOT_DIST);
    if(m_lastPitem->GetPID()==TOT_FAST)
      m_lastPitem->ChgPID(TOT_FDIS);
    if(m_lastPitem->GetPID()==CAL_SUMM)
      m_lastPitem->ChgPID(CAL_DIST);
    if(m_lastPitem->GetPID()==CAL_FAST)
      m_lastPitem->ChgPID(CAL_FDIS);
    //ModuleList_itemselected(m_lastPitem);
    QTimer::singleShot(0,this,SLOT(Replot()) );
    break;}
  default:
    break;
  }
  return;
}
void TopWin::LogToTree(DatSet *did, ModItem *dataitem){
  ModItem *plotit, *subplt;
  int Chip, dot=0, dof=0, dom[4]={0,0,0,0};
  if(did->GetLog()!=0){
    for(Chip=0;Chip<NCHIP;Chip++){
      if(did->GetLog()->MyModule->Chips[Chip]->TDACs->m_mode==32 ||
         ((did->GetLog()->TDAQversion>=4 || did->GetLog()->TDAQversion<0)
        && did->GetLog()->MyModule->Chips[Chip]->TDACs->m_mode==128))  dot =1;
      if(did->GetLog()->MyModule->Chips[Chip]->FDACs->m_mode==32 ||
         ((did->GetLog()->TDAQversion>=4 || did->GetLog()->TDAQversion<0)
        && did->GetLog()->MyModule->Chips[Chip]->FDACs->m_mode==128))  dof =1;
      if(did->GetLog()->MyModule->Chips[Chip]->ReadoutMask->m_filename.find("ALL")== std::string::npos &&
         did->GetLog()->MyModule->Chips[Chip]->ReadoutMask->m_filename!="" ) dom[0] =1;
      if(did->GetLog()->MyModule->Chips[Chip]->HitbusMask->m_filename.find("ALL") == std::string::npos &&
         did->GetLog()->MyModule->Chips[Chip]->HitbusMask->m_filename!="" ) dom[1] =1;
      if(did->GetLog()->MyModule->Chips[Chip]->StrobeMask->m_filename.find("ALL") == std::string::npos &&
         did->GetLog()->MyModule->Chips[Chip]->StrobeMask->m_filename!="" ) dom[2] =1;
      if(did->GetLog()->MyModule->Chips[Chip]->PreampMask->m_filename.find("ALL") == std::string::npos &&
         did->GetLog()->MyModule->Chips[Chip]->PreampMask->m_filename!="" ) dom[3] =1;
    }
    plotit = new ModItem(dataitem,"Log File",did,NONE);
#ifdef PIXEL_ROD // can link to PixScan
    if(did->getPixScan()!=0){
      subplt = new ModItem(plotit,  "Display module cfg",did,LOGFILE);
      subplt = new ModItem(plotit,  "Display scan cfg",did,PSCONFIG);
    } else
#endif
    subplt = new ModItem(plotit,  "Display",did,LOGFILE);
    subplt = new ModItem(plotit,  "Create cfg file",did,WRITECFG);
    if(dot) subplt = new ModItem(plotit,"Plot TDAC map",did,TDACLOG);
    if(dof) subplt = new ModItem(plotit,"Plot FDAC map",did,FDACLOG);
    if(dom[0]) subplt = new ModItem(plotit,"Plot readout mask",did,RMASKP);
    if(dom[1]) subplt = new ModItem(plotit,"Plot hitbus mask",did,HMASKP);
    if(dom[2]) subplt = new ModItem(plotit,"Plot strobe mask",did,SMASKP);
    if(dom[3]) subplt = new ModItem(plotit,"Plot preamp mask",did,PMASKP);
  } else if(did->m_istype!=TFDAC && did->m_istype!=TWLK && did->m_istype!=MASKDAT
            && did->m_istype!=TOTCOMB && did->m_istype!=TOTCF && did->m_istype!=CHIPCURR
            && did->m_istype!=BI_VIT_DATA && did->m_istype!=REGTEST)
    plotit = new ModItem(dataitem,"Load Log File",did,GETLOG);

  return;
}


void TopWin::Options_save(){
  QString file = QDir::homeDirPath() + "/.mafrc";
  Options_saveAs(file.latin1());
  return;
}
void TopWin::Options_saveAs(const char *path){
  QString qspath;
  if(path!=0) qspath = path;
  else{
    QStringList filter("Options file (*.opt)");
    filter += "Any file (*.*)";
    Q3FileDialog fdia(QString::null, QString::null,this,"select data file",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    fdia.setFilters(filter);
    if(fdia.exec() == QDialog::Accepted && !fdia.selectedFile().isEmpty())
      qspath = fdia.selectedFile();
    else
      return;
  }
  /*
  TFile *output;
  output = new TFile(qspath.latin1(),"RECREATE");
  if(output==0) return;
  options->m_Opts->Write("options");
  output->Close();
  */
  FILE *outfile = fopen(qspath.latin1(),"w");
  if(outfile==0) return;
  options->m_Opts->Write(outfile);
  fclose(outfile);
  return;
}
void TopWin::Options_load(){
  Options_loadFrom(QDir::homeDirPath() + "/.mafrc");
  return;
}
void TopWin::Options_loadFrom(const char *path){
  QString qspath;
  if(path!=0) qspath = path;
  else{
    QStringList filter("Options file (*.opt)");
    filter += "Any file (*.*)";
    Q3FileDialog fdia(QString::null, QString::null,this,"select data file",TRUE);
    fdia.setFilters(filter);
    if(fdia.exec() == QDialog::Accepted && !fdia.selectedFile().isEmpty())
      qspath = fdia.selectedFile();
    else
      return;
  }
  /*
  // test if file exists (avoids annoying error msg. from TFile)
  FILE *test_if_exist = fopen(qspath.latin1(),"r");
  if(test_if_exist==0) return;
  fclose(test_if_exist);
  // then really open it
  TFile *input = new TFile(qspath.latin1());
  if(input==0) return;
  optClass *tmp = (optClass*)input->Get("options");
  if(tmp!=0)
    *options->m_Opts = *tmp;
  //    options->m_Opts = (optClass*) tmp->Clone("kuytdf");
  input->Close();
  */
  FILE *infile = fopen(qspath.latin1(),"r");
  if(infile==0) return;
  options->m_Opts->Read(infile);
  fclose(infile);
  options->Reset_clicked();
  return;
}

void TopWin::RootCommand(){
  int error;
  RenameWin rnw(this," ", TRUE, Qt::WType_TopLevel);
  rnw.Name->validateAndSet("enter root command here",30,0,30);
  rnw.Name->setFocus();
  rnw.exec();
  gROOT->ProcessLineSync(rnw.Name->text().latin1(), &error);
  switch(error){
  case TInterpreter::kNoError:
    break;
  case TInterpreter::kRecoverable:
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"MAF","Recoverable error while executing command");
    break;
  default:
  case TInterpreter::kDangerous:
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"MAF","Dangerous error while executing command");
    break;
  case TInterpreter::kFatal:
    if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"MAF","Fatal error while executing command");
    break;
  }
  return;
}
void TopWin::RootMacro(){
  TInterpreter::EErrorCode error;
  // load default dir. if requested to
  QString path = QString::null;
  if(options->m_Opts->m_DefMacroPath!=".")
    path = options->m_Opts->m_DefMacroPath.c_str();
  Q3FileDialog fdia(path, "ROOT macro (*.cpp)",this,"select data file",TRUE);
  if(fdia.exec() == QDialog::Accepted){
    gInterpreter->ExecuteMacro(fdia.selectedFile().latin1(), &error);
    switch(error){
    case TInterpreter::kNoError:
      break;
    case TInterpreter::kRecoverable:
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"MAF","Recoverable error while executing macro");
      break;
    default:
    case TInterpreter::kDangerous:
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"MAF","Dangerous error while executing macro");
      break;
    case TInterpreter::kFatal:
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"MAF","Fatal error while executing macro");
      break;
    }
  }
  return;
}
void TopWin::ModuleList_return(Q3ListViewItem* item){
  if(item==0) return;
 // process, depending on wether it's a module or data
  if(item->parent()==0){ // it's a module item
    item->setOpen(true);
    ModuleList_rpr((ModItem*)item);
  }else{
    // toggle open-state
    item->setOpen(!item->isOpen());
    if(item->isOpen())
      ModuleList_itemselected((ModItem*)item);
  }
  return;
}

void TopWin::PlotMasked(ModItem* item){
  if(item==0) return;
  int i,k, error, chip=-1;
  if(ChipOrMod->isOn()) chip = ChipSel->value();
  MaskedSel mskwin(item, this,"Masks application", TRUE, Qt::WType_TopLevel);
  if(mskwin.exec()==1){
    // reset last PID and DID
    m_lastPitem = 0;
    m_lastPdbID = -1;
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
    //if(rootmain->m_lastmap!=0) rootmain->m_lastmap->Delete();
    rootmain->ClearMemory();
    rootmain->m_lastmap = rootmain->m_currmap;
    rootmain->m_currmap = 0;
    // now the actual masked plot
    i = mskwin.FirstSel->currentItem();
    k = mskwin.SecondSel->currentItem();
    
    error = rootmain->PlotMasked(mskwin.alist[i], mskwin.blist[k], mskwin.aplotid[i], mskwin.bplotid[k],
                                 mskwin.MaskTypeBox->isChecked(),chip);
    
    if(error)
      {
        QString tmp;
        tmp =  "Error plotting: ";
        tmp += RMain::ErrorToMsg(error).c_str();
        
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
      } else{
        if(rootmain->m_currmap!=0){
          rootmain->m_currmap->SetWidget(this);
          int chip=0,col, row, binx, biny;
          if(m_lastchip<0) chip = ChipSel->value();
          col = ColSel->value();
          row = RowSel->value();
          mskwin.alist[i]->GetDataItem()->PixCCRtoBins(chip,col,row,binx,biny);
          QString txtstr;
          txtstr.sprintf("%.3f",rootmain->m_currmap->GetBinContent(binx,biny));
          PixVal->setText(txtstr);
        } else
          PixVal->setText("unknown");
      }
  }
}
void TopWin::Cuts_open(){
  m_cutpanel->show();
}
void TopWin::MakeStd(ModItem *item){
  if(item==0) return;
  if(item->GetDID()==0){
    QMessageBox::information(this,"ModuleAnalysis::MakeStd",
			     "Set std. type is currently not implemented for non-MA data types");
    return;
  }
  int i, type, altt;
  std::vector<int> inlist;
  std::map<int,int> inv_list;
  StdTestWin stdwin(this,"Load Std Files", TRUE, Qt::WType_TopLevel);

  RefWin stdsel(item, this,"stdselector", TRUE, Qt::WType_TopLevel,10);
  stdsel.FuncselLabel->setText("Select type of measurement");
  stdsel.FuncSel->insertItem("undefined");
  for(i=1;i<NTYPES;i++){
    stdsel.FuncSel->insertItem(stdwin.prefix[i]);
  }
  stdsel.SelectorLabel->setText("Select type of standard test");
  stdsel.DatSel->insertItem("none");
  inlist.push_back(-1);
  for(i=0;i<stdwin.NSLOTS;i++){
    type = stdwin.m_items[i]->m_type;
    altt = type;
    if(altt==TOTF) altt = TOTFR;
    if(item->GetDID()->m_istype==type || item->GetDID()->m_istype==altt){
      inv_list.insert(std::make_pair(i,inlist.size()));
      inlist.push_back(i);
      stdsel.DatSel->insertItem(stdwin.m_items[i]->m_label->text());
    }
  }
  if(inlist.size()>0) stdsel.OKButt->setEnabled(true);
  int tt, mt = item->GetDID()->GetStdtest();
  tt  = mt%100;
  mt /= 100;
  stdsel.DatSel->setCurrentItem(inv_list[tt]);
  stdsel.FuncSel->setCurrentItem(mt);
  if(stdsel.exec()==1)
    item->GetDID()->SetStdtest(100*stdsel.FuncSel->currentItem()+inlist[stdsel.DatSel->currentItem()]);
}
void TopWin::CalibrateToT(ModItem *item, ModItem *ToTcal){
  bool recre = item->GetDID()->m_iscal==0;
  DatSet *tc;
  ModItem *myToTcal=ToTcal, *temp, *temp2, *calprnt=0, *peakpnt=0;
  if(myToTcal==0){
    RefWin rdw(item, this,"Reference Data Set", TRUE, Qt::WType_TopLevel,1);
    if(rdw.DatSel->count()>0){
      if(rdw.exec()==1){
        myToTcal = rdw.selit;
      }
    }
  }
  if(myToTcal==0) return;

  textwin->setText("Calibrating " + item->text(0) + " with " + myToTcal->text(0) + "...");
  QApplication::setOverrideCursor(Qt::waitCursor);
  tc = myToTcal->GetDID();
  item->GetDID()->CalibrateToT(tc);
  temp = (ModItem*)item->firstChild();
  while(temp!=0){
    if(temp->text(0)=="ToT peak finders")
      peakpnt = temp;
    if(temp->text(0)=="ToT data")
          calprnt = temp;
    temp = (ModItem*) temp->nextSibling();
        if(peakpnt!=0 && calprnt!=0) break;
  }
  if(calprnt==0) return;
  temp = (ModItem*)calprnt->firstChild();
  while(temp!=0){
    temp2 = (ModItem*)temp->nextSibling();
    if(temp->text(0)=="Calibrate ToT"){
      //delete temp;
      temp->setText(0,"Re-calibrate ToT");
    }
        temp=temp2;
  }
  if(!recre){
    //    new ModItem(calprnt,  "Re-calibrate ToT", item->GetDID(),TOTCAL);
    //  } else{
    temp = (ModItem*)calprnt->firstChild();
    while(temp!=0){
      temp2 = (ModItem*)temp->nextSibling();
      if(temp->text(0)=="Plot calib'ed ToT dist."){
        delete temp;
      }
      temp=temp2;
    }
    temp = 0;
    if(peakpnt!=0) temp = (ModItem*)peakpnt->firstChild();
    while(temp!=0){
      temp2 = (ModItem*)temp->nextSibling();
      if(temp->text(0)=="Plot position of cal. peak & distribution (fast ana)" 
         || temp->text(0)=="Plot position of cal. peak & distribution (fitted)"
         || temp->text(0)=="Run cal. peak fit (slow!)")  delete temp;
      temp=temp2;
    }
  }
  if(calprnt!=0) new ModItem(calprnt,  "Plot calib'ed ToT dist.", item->GetDID(),CLTOT,tc);
  if(peakpnt!=0){
    new ModItem(peakpnt,  "Run cal. peak fit (slow!)", item->GetDID(),CAL_SUMM,tc);
    new ModItem(peakpnt,  "Plot position of cal. peak & distribution (fast ana)", item->GetDID(),CAL_FAST,tc);
  }
  textwin->setText("...done");
  QApplication::restoreOverrideCursor();

  return;
}
void TopWin::SetCurrMod(Q3ListViewItem *item){
  ModItem *currModule;
  currModule = dynamic_cast<ModItem*>(item);
  int seltype = 0;

  if(currModule!=0){
    if(currModule->GetPID()!=MODITEM)
      currModule = currModule->ModParent();
  }

  if(currModule!=0) seltype = CheckChilds((ModItem*)currModule);

  m_mainpop->setItemEnabled(m_mainpop->idAt(3),currModule!=0);
  m_mainpop->setItemEnabled(m_mainpop->idAt(4),currModule!=0);
  m_mainpop->setItemEnabled(m_mainpop->idAt(5),currModule!=0);
  m_mainpop->setItemEnabled(m_mainpop->idAt(10),(currModule!=0 && currModule->childCount()>0));
  m_mainpop->setItemEnabled(m_mainpop->idAt(11),(currModule!=0 && (seltype&1)));
  m_editpop->setItemEnabled(m_editpop->idAt(0),currModule!=0);
  m_editpop->setItemEnabled(m_editpop->idAt(1),currModule!=0);
  m_editpop->setItemEnabled(m_editpop->idAt(2),currModule!=0);
  m_editpop->setItemEnabled(m_editpop->idAt(3),currModule!=0);
  m_anapop->setItemEnabled(m_anapop->idAt(2),currModule!=0);
  m_anapop->setItemEnabled(m_anapop->idAt(3),currModule!=0);
  m_anapop->setItemEnabled(m_anapop->idAt(5),ModuleList->childCount()>0);
  m_anapop->setItemEnabled(m_anapop->idAt(6),ModuleList->childCount()>0);
  m_anapop->setItemEnabled(m_anapop->idAt(7),(currModule!=0 && (seltype & 2)));
  m_anapop->setItemEnabled(m_anapop->idAt(8),(currModule!=0 && currModule->childCount()>1));
  m_anapop->setItemEnabled(m_anapop->idAt(9),(currModule!=0 && currModule->childCount()>1));
  m_anapop->setItemEnabled(m_anapop->idAt(10),currModule!=0);

  if(currModule!=0){
    QString label;
    label = "&Load data for " + currModule->text(0);
    m_mainpop->changeItem(m_mainpop->idAt(3),label);
    label = "Load file-&list for " + currModule->text(0);
    m_mainpop->changeItem(m_mainpop->idAt(4),label);
    label = "Load std &file-set for " + currModule->text(0);
    m_mainpop->changeItem(m_mainpop->idAt(5),label);
    label = "&Save all for " + currModule->text(0);
    m_mainpop->changeItem(m_mainpop->idAt(10),label);
    label = "Save &highlighted for " + currModule->text(0);
    m_mainpop->changeItem(m_mainpop->idAt(11),label);
    label = "&Rename " + currModule->text(0);
    m_editpop->changeItem(m_editpop->idAt(0),label);
    label = "&Delete " + currModule->text(0);
    m_editpop->changeItem(m_editpop->idAt(1),label);
    label = "Erase &fitted raw data (" + currModule->text(0)+")";
    m_editpop->changeItem(m_editpop->idAt(2),label);
    label = "Compress data to &summaries  (" + currModule->text(0)+")";
    m_editpop->changeItem(m_editpop->idAt(3),label);
    label = "&PDB Form for " + currModule->text(0);
    m_anapop->changeItem(m_anapop->idAt(2),label);
    label = "&QA Report for " + currModule->text(0);
    m_anapop->changeItem(m_anapop->idAt(3),label);
    label = "&Apply mask for " + currModule->text(0);
    m_anapop->changeItem(m_anapop->idAt(6),label);
    label = "&Overlay graphs for " + currModule->text(0);
    m_anapop->changeItem(m_anapop->idAt(7),label);
    label = "Compare &bad channels for " + currModule->text(0);
    m_anapop->changeItem(m_anapop->idAt(8),label);
    label = "&Ibias stability plot for " + currModule->text(0);
    m_anapop->changeItem(m_anapop->idAt(9),label);
  } else{
    m_mainpop->changeItem(m_mainpop->idAt(3),"&Load data for curr. module");
    m_mainpop->changeItem(m_mainpop->idAt(4),"Load file-&list for curr. module");
    m_mainpop->changeItem(m_mainpop->idAt(5),"Load std &file-set for curr. module");
    m_mainpop->changeItem(m_mainpop->idAt(10),"&Save all for curr. module");
    m_mainpop->changeItem(m_mainpop->idAt(11),"Save &highlighted for curr. module");
    m_editpop->changeItem(m_editpop->idAt(0),"&Rename curr. module");
    m_editpop->changeItem(m_editpop->idAt(1),"&Delete curr. module");
    m_editpop->changeItem(m_editpop->idAt(2),"Erase &fitted raw data (curr. mod.)");
    m_editpop->changeItem(m_editpop->idAt(3),"Compress data to &summaries (curr. module)");
    m_anapop->changeItem(m_anapop->idAt(2),"&PDB Form for curr. module");
    m_anapop->changeItem(m_anapop->idAt(3),"&QA Report for curr. module");
    m_anapop->changeItem(m_anapop->idAt(6),"&Apply mask for curr. mod.");
    m_anapop->changeItem(m_anapop->idAt(7),"&Overlay graphs for curr. mod.");
    m_anapop->changeItem(m_anapop->idAt(8),"Compare &bad channels for curr. mod.");
    m_anapop->changeItem(m_anapop->idAt(9),"&Ibias stability plot for curr. mod.");
  }
  return;
}

void TopWin::Replot(){
  int i=0, nmax=100;
  // wait till the last root-call has finished
  while(m_PlotIsActive && i<nmax){
    gSystem->Sleep(40);
    i++;
  }
  if(i==nmax){
    QMessageBox::warning(this,"MAF","Waiting for double-click routine to finish timed out;\nwon't plot then.");
    return;
  }      
  // then execute the plot command
  if(m_lastPdbID>=0)
    ModuleList_rpr(m_lastPitem, m_lastPdbID);
  else
    ModuleList_itemselected(m_lastPitem);
  return;
}
void TopWin::PlotMultiGraphs(ModItem *item){
  ModItem *itarr[20], *child, *subchild, *sschild;
  int j=0, error;
  bool isiv=true;
  child = (ModItem*)item->firstChild();
  while(child!=0){
    PixDBData *pdbd = dynamic_cast<PixDBData*>(child->GetDataItem());
    if(child->isSelected() && ((pdbd!=0 && child->GetPID()==IVPLOT) ||
				(child->GetDID()->m_istype==IVSCAN || child->GetDID()->m_istype==SENSORIV))){
      itarr[j] = child;
      j++;
    }
    else if(child->isSelected()){
      itarr[j] = child;
      j++;
      isiv = false;
    }
    subchild = (ModItem*)child->firstChild();
    while(subchild!=0){
      PixDBData *pdbds = dynamic_cast<PixDBData*>(subchild->GetDataItem());
      if(subchild->isSelected() && ((pdbds!=0 && subchild->GetPID()==IVPLOT) ||
				    (subchild->GetDID()->m_istype==IVSCAN || subchild->GetDID()->m_istype==SENSORIV))){
	itarr[j] = subchild;
	j++;
      }
      else if(subchild->isSelected()){
	itarr[j] = subchild;
	j++;
	isiv = false;
      }
      sschild = (ModItem*)subchild->firstChild();
      while(sschild!=0){
	PixDBData *pdbdss = dynamic_cast<PixDBData*>(sschild->GetDataItem());
	if(sschild->isSelected() && ((pdbdss!=0 && sschild->GetPID()==IVPLOT) ||
				     (sschild->GetDID()->m_istype==IVSCAN || sschild->GetDID()->m_istype==SENSORIV))){
	  itarr[j] = sschild;
	  j++;
	}
	else if(sschild->isSelected()){
	  itarr[j] = sschild;
	  j++;
	  isiv = false;
	}
	sschild = (ModItem*)sschild->nextSibling();
      }
      subchild = (ModItem*)subchild->nextSibling();
    }
    child = (ModItem*)child->nextSibling();
  }
  if(j>0){
    if(isiv) error = rootmain->PlotGraph(itarr,j);
    else     error = rootmain->PlotScan(itarr,j);
    QString tmp;
    tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
    if(options->m_Opts->m_showwarn && error<0) QMessageBox::warning(this,"Module Analysis",tmp);
  } else
    QMessageBox::warning(this,"MAF","No items selected");
}

ModItem* TopWin::FitToTComb(ModItem *item){
    QString tmp;
    DatSet *rawdat[2];
    ModItem *test1, *test2;
    ModItem *parnt, *datit;
    datit = item->DatParent();
    parnt = item->ModParent();
    /*
    datit = item;
    parnt = item->Parent();
    while(parnt->Parent()!=0){ // find highest level item (=module)
      datit = parnt;              // the actual data set item
      parnt = parnt->Parent();    // and the module item
    }
    */
    rawdat[0] = item->GetRID(0);
    rawdat[1] = item->GetRID(1);
    float anapar[2]={0,-1};
    int chip, error=0;
    if(options->m_Opts->m_fixedToTSwitch){
      anapar[0] = (float)options->m_Opts->m_ToTSwitch;
      anapar[1] = (float)options->m_Opts->m_QPlotPnt;
    }
    // check if fit data exists, and ask user if it should be overwritten
    if(item->GetDID()->m_istype==TOTCF){
      QMessageBox ays("Confirm delete",
                      "This will overwrite existing fit data\nAre you sure?",
                      QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
                      this, "ays",TRUE,Qt::WType_TopLevel);
      /*
      SureDia ays( this, datit->text(0).latin1(), TRUE, Qt::WType_TopLevel);
      ays.AppendButt->hide();
      ays.TextboxLabel1->setText( "This will overwrite existing fit data");
      ays.TextboxLabel2->setText( " ");
      */
      error = (ays.exec()!=QMessageBox::Ok);
    }
    if(error) return 0;
    // initialise fitting
    textwin->setText("Performing ToT-combined fit now, please be patient...");
    textwin->repaint();
    if(m_app!=0)
      m_app->processEvents();
    QApplication::setOverrideCursor(Qt::waitCursor);
    gSystem->Sleep(1000);
    error = item->GetDID()->TotCombFit(-2,rawdat,anapar,2);
    if(error<0){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error while initialising fit: %s",DatSet::ErrorToMsg(error).c_str());
      if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
      textwin->setText("...aborted.");
      return 0;
    }
    // run fit chip by chip
    for(chip=0;chip<NCHIP;chip++){
      tmp.sprintf("...fitting chip %d...",chip);
      textwin->setText(tmp);
      textwin->repaint();
      if(m_app!=0)
        m_app->processEvents();
      error = item->GetDID()->TotCombFit(chip,rawdat,anapar,2);
      if(error<0){
        QApplication::restoreOverrideCursor();
        tmp.sprintf("Error while fitting: %s",DatSet::ErrorToMsg(error).c_str());
        if(options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
        textwin->setText("...aborted.");
        return 0;
      }
    }
    QApplication::restoreOverrideCursor();
    textwin->setText("...done!");
    test1 = DataToTree(parnt,datit->GetDID());
    delete datit;
    datit = test1;
    test1 = (ModItem*) test1->firstChild();
    while(test1!=0){
      if(test1->text(0)=="Plot Q vs ToT"  || test1->text(0)=="Plot ToT distributions" 
         || test1->text(0)=="Re-fit combined data" || test1->text(0)=="Modify Chi/Clo calibration")
        test1->SetRefArr(rawdat,2);
      test2 = test1;
      test1 = (ModItem*) test1->nextSibling();
      if(test2->text(0)=="Load Log File") delete test2;
    }
    return datit;
}
void TopWin::RunQAReport(ModItem *item){
  if(item==0) return;
  QAReport rep(item, this, "QAReport", TRUE, Qt::WType_TopLevel);
  rep.exec();
  return;
}
void TopWin::RunQAReportCurrModule(){
  RunQAReport(GetCurrMod());
  return;
}
void TopWin::UpdatePixVal(){
  int chip=0,col, row, binx, biny;
  if(rootmain->m_currmap!=0){
    if(m_lastchip<0) chip = ChipSel->value();
    col = ColSel->value();
    row = RowSel->value();
    // flip tmphi for FE-I4; can have 80 or 160 col's (PPS module plot), so can only cut on # rows
    if(rootmain->m_currmap->GetNbinsY()==336) row = 335 - row;

    if(m_lastPitem!=0 && m_lastPitem->GetDataItem()!=0){
      m_lastPitem->GetDataItem()->PixCCRtoBins(chip,col,row,binx,biny);
      QString txtstr;
      txtstr.sprintf("%.3f",rootmain->m_currmap->GetBinContent(binx,biny));
      PixVal->setText(txtstr);
    } else
      PixVal->setText("unknown");
  } else
    PixVal->setText("unknown");
}
void TopWin::helpAbout(){
  QString about = "This is ATLAS Pixel Module Analysis\n\n"
    "Version " + m_verstrg + ", built\nwith ROOT version " + QString(gROOT->GetVersion()) +
    "\nand QT version " + QString(qVersion()) + "."
#ifdef HAVE_PLDB
    "\nincluding PixLib::RootDB functionality."
#endif
    "\n\nCurrently active authors of GUI and libraries:\n" PLGUI_AUTHORS;
  QMessageBox::information(this,"MAF about",about);
}
void TopWin::helpUserRef(){
  QString url="http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/ModuleAnalysis";
  openUrl(url.latin1());
}
void TopWin::helpCoding(){
  QString url="http://home.cern.ch/jgrosse/ModuleAnalysis/";
  openUrl(url.latin1());
}
void TopWin::openUrl(const char *url){
  QString exe_brws;
  QString sysname = gSystem->GetName();

  if(m_browser==QString::null){
    Q3Process *getbrws = new Q3Process(this);
    if(sysname=="WinNT"){       // windows
      QString execstr;
      char a[100], b[100], c[100], htmlrc[1000]={'\0'}, browserc[1000]={'\0'};
      QString qline;
      getbrws->addArgument("reg");
      getbrws->addArgument("query");
      getbrws->addArgument("\"HKEY_CLASSES_ROOT\\.html\"");
      getbrws->start();
      while(getbrws->isRunning()){};
      if(getbrws->exitStatus()==0){
        while(getbrws->canReadLineStdout()){
          qline = getbrws->readLineStdout();
          if(qline.find("NO NAME")>=0){
            sscanf(qline.latin1(),"%s %s %s %s",a,b,c, htmlrc);
            break;
          }
        }
      }
      getbrws->clearArguments();
      if(strlen(htmlrc)>0){
        getbrws->addArgument("reg");
        getbrws->addArgument("query");
        execstr = "\"HKEY_CLASSES_ROOT\\";
        execstr+= htmlrc;
        execstr+= "\\shell\\open\\command\"";
        getbrws->addArgument(execstr);
        getbrws->start();
        while(getbrws->isRunning()){};
        if(getbrws->exitStatus()==0){
          while(getbrws->canReadLineStdout()){
            qline = getbrws->readLineStdout();
            if(qline.find("NO NAME")>=0){
              sscanf(qline.latin1(),"%s %s %s %s",a,b,c, browserc);
              break;
            }
          }
        }
        if(strlen(browserc)>0)
          m_browser = browserc;
      }
    } else if(sysname=="Unix"){ // linux/unix
      const int nbmax=3;
      int nb;
      QString bname[nbmax]={"mozilla","konqueror","netscape"};
      for(nb=0;m_browser.isEmpty() && nb<nbmax;nb++){
        getbrws->clearArguments();
        getbrws->addArgument(bname[nb]);
        getbrws->addArgument("--version");
        if(getbrws->start()){
          while(getbrws->isRunning()){}; // wait for cmd to finish
          if(!getbrws->exitStatus()){
            m_browser = bname[nb];
          }
        }
      }
    }
    delete getbrws;
  }

  if(m_browser==QString::null){
    QString message="Can't find internet browser;\n"
      "open the following URL to browse the help pages:\n";
    message += url;
    QMessageBox::warning(this,"browser not found",message);
  } else{
    if(m_brwsproc->isRunning()) // terminate previous stuff
      m_brwsproc->kill();
    m_brwsproc->clearArguments();
    m_brwsproc->addArgument(m_browser);
    m_brwsproc->addArgument(url);
    if(!m_brwsproc->start())
      QMessageBox::warning(this,"browser not executed","Could not launch browser");
  }
}
int TopWin::CheckChilds(ModItem *item){
  if(item==0) return 0;
  int i=0, j=0, seltype=-1;
  ModItem *child, *subchild, *sschild;

  child = (ModItem*)item->firstChild();
  while(child!=0){
    if(child->isSelected()){
      i++;
      if((seltype==-1 || child->GetPID()==seltype) && 
         (child->GetPID()==IVCORR || child->GetPID()==IVPLOT
          || child->GetPID()==RAW || child->GetPID()==TWODRAW
          || child->GetPID()==TOTRAW)) j++;
      seltype = child->GetPID();
    }
    subchild = (ModItem*)child->firstChild();
    while(subchild!=0){
      if(subchild->isSelected()){
        if((seltype==-1 || subchild->GetPID()==seltype) && 
           (subchild->GetPID()==IVCORR || subchild->GetPID()==IVPLOT
            || subchild->GetPID()==RAW || subchild->GetPID()==TWODRAW
            || subchild->GetPID()==TOTRAW)) j++;
        seltype = subchild->GetPID();
      }
      sschild = (ModItem*)subchild->firstChild();
      while(sschild!=0){
        if(sschild->isSelected()){
          if((seltype==-1 || sschild->GetPID()==seltype) && 
             (sschild->GetPID()==IVCORR || sschild->GetPID()==IVPLOT
              || sschild->GetPID()==RAW || sschild->GetPID()==TWODRAW
              || sschild->GetPID()==TOTRAW)) j++;
          seltype = sschild->GetPID();
        }
        sschild = (ModItem*)sschild->nextSibling();
      }
      subchild = (ModItem*)subchild->nextSibling();
    }
    child = (ModItem*)child->nextSibling();
  }
  seltype = 0;
  if(i>0)  seltype |= 1;
  if(j>1)  seltype |= 2;
  return seltype;
}
void TopWin::LoadRootPixLib(bool link){
#ifdef HAVE_PLDB
  int rootID=1;
  std::vector<Q3ListViewItem*> scan_items;
  QStringList filter("ROOT data file (*.root)");
  filter += "Any file (*.*)";
  QString path = QString::null;
  if(options->m_Opts->m_DefDataPath!=".")
    path = options->m_Opts->m_DefDataPath.c_str();
  Q3FileDialog fdia(path, QString::null, this,"select data file",TRUE);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted) {
    QApplication::setOverrideCursor(Qt::waitCursor);
    std::string fname = fdia.selectedFile().latin1();
    RootDataList dl(this,"rootdl",TRUE);
    dl.Comments->hide(); // do we need a comment window for these guys?
    // now get the objects in the DB and load the module names
    PixLib::RootDB *myrdb=0;
    PixLib::DBInquire *root, *DBscan=0;
    std::map<int, std::vector<PixLib::DBInquire*> > DBmods;
    PixLib::recordIterator baseScanIter;
    // get the actual DB object - desparately needed for further use
    try{
      myrdb  = new PixLib::RootDB(fname);
    } catch(...){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"ModuleAnalysis Error","Error opening file "+fdia.selectedFile());
      return;
    }
    if(myrdb==0){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"ModuleAnalysis Error","Error reading a DB object from file");
      return;
    }
    // then loop over the root records until no more is to be found
    // assumption: one record == one scan with all its config and data
    try{
      root = myrdb->readRootRecord(rootID);
    } catch(...){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"ModuleAnalysis Error","No root record found in file");
      return;
    }
    while(root!=0){
      // let's see if any scan info is around
      for(baseScanIter = root->recordBegin(); baseScanIter!=root->recordEnd(); baseScanIter++){
        if((*baseScanIter)->getName()=="PixScanData" ||
           (*baseScanIter)->getName()=="PixScanResult" ||
	   (*baseScanIter)->getName()=="PixScan" ){
          std::string name, time_stamp="";
#ifdef PIXEL_ROD // can link to PixScan
	  if((*baseScanIter)->getName()=="PixScanResult"){
	    PixLib::fieldIterator tmpit = (*baseScanIter)->findField("TimeStamp");
	    std::string timestamp="";
	    if((*tmpit)!=0 && tmpit!=(*baseScanIter)->fieldEnd())
	    myrdb->DBProcess(tmpit,PixLib::READ, time_stamp);
	    int pos = (int)time_stamp.find("Scan commenced at ");
	    if(pos!=(int)std::string::npos)
	      time_stamp.erase(pos,18);
	  }
#endif
          name = TLogFile::GetNameFromPath((*baseScanIter)->getDecName().c_str());
          dl.LoadName(name.c_str(), (long int)*baseScanIter, time_stamp.c_str());
        }
      }
      rootID++;
      try{
        root = myrdb->readRootRecord(rootID);
      } catch(...){
        break;
      }
    }
    // finished fishing in the root file, let's see if we caught something
    if(dl.FileList->firstChild()==0){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"ModuleAnalysis Error","No scan info found in "+fdia.selectedFile());
      return;
    }
    QApplication::restoreOverrideCursor();
    // then proudly present our big fishes in a panel, have the user select some
    if(dl.exec()==1){
      QApplication::setOverrideCursor(Qt::waitCursor);
      // and get them into MA somehow
      Q3ListViewItem *item = dl.FileList->firstChild();
      while(item!=0){
        if(item->isSelected()){
	  DBscan = (PixLib::DBInquire*) dl.loadedItems[(long int)item]; // scan object
	  loadPixScanData(DBscan,0,link,fname.c_str());
	}
	item = item->nextSibling();
      }
    }
    delete myrdb; // closes the file
    QApplication::restoreOverrideCursor();
  }
#else // HAVE_PLDB
  QMessageBox::warning(this,"MA","This version comes without PixLib\n -> function is not implemented!");
#endif // HAVE_PLDB
}
void TopWin::loadPixScanData(void *DBscan_in, int summary,bool link, const char *fname){
  std::vector<ModItem*> data;
  loadPixScanData(data, DBscan_in, summary, link, fname, 0);
  data.clear();
}
void TopWin::loadPixScanData(std::vector<ModItem*> &data, void *DBscan_in, int summary,bool link, 
			     const char *fname,  const char *mname){
#ifndef HAVE_PLDB
  QMessageBox::warning(this,"MA","This version comes without PixLib\n -> function is not implemented!");
}
#else
  PixLib::DBInquire *DBscan = (PixLib::DBInquire *) DBscan_in;
  ModItem *modit;
//DatSet *datit;
  PixelDataContainer* datit;
  TLogFile *mylog;
  PixLib::DBInquire *DBscfg;
  //  std::map<int, std::vector<PixLib::DBInquire*> > DBmods;
  PixLib::recordIterator baseIter, modIter, cfgIter;
  if(DBscan==0) return;
  PixLib::RootDB *myrdb=dynamic_cast<PixLib::RootDB*>(DBscan->getDB());
  if(myrdb==0) return;
#ifndef PIXEL_ROD // can't link to PixScan
  bool noPixScan_warning=true;
#endif
  if(DBscan->getName()=="PixScanResult" ||
     DBscan->getName()=="PixScan" ){
#ifdef PIXEL_ROD // can link to PixScan
    // load scan data & config for each module - new structure using PixScan
    PixLib::fieldIterator fit;
    fit = DBscan->findField("TimeStamp");
    std::string timestamp="";
    if((*fit)!=0 && fit!=DBscan->fieldEnd())
      myrdb->DBProcess(fit,PixLib::READ, timestamp);
    //printf("Processing scan %s (%s) ... \n",DBscan->getDecName().c_str(), timestamp.c_str());
    std::string modNam, grpNam;
    recordIterator bStart = DBscan->recordBegin();
    recordIterator bEnd = DBscan->recordEnd();
    if(mname!=0){
      modNam = mname;
      grpNam = modNam;
      int pos = (int)modNam.find("/");
      if(pos!=(int)std::string::npos){
	modNam.erase(0,pos+1);
	grpNam.erase(pos, grpNam.length()-pos);
      }
      bStart = DBscan->findRecord(grpNam+"/PixModuleGroup");
      if(bStart!=bEnd){
	bEnd = bStart;
	bEnd++;
      }
    }
    for(baseIter = bStart; baseIter!=bEnd; baseIter++){
      if((*baseIter)->getName()==("PixModuleGroup") ||
	 ((*baseIter)->getDecName().find((*baseIter)->getName())!=std::string::npos &&
	  DBscan->getName()=="PixScan")){
	fit = (*baseIter)->findField("InfoROD");
	std::string infoROD="";
	int rodRev=-1, rodSN=-1, mdspVer=-1, mdspCode=-1;
	if((*fit)!=0 && fit!=(*baseIter)->fieldEnd()){
	  myrdb->DBProcess(fit,PixLib::READ, infoROD);
	  int pos = (int)infoROD.find("ROD rev");
	  if(pos!=(int)std::string::npos)
	    sscanf(infoROD.substr(pos+8,1).c_str(),"%x",&rodRev);
	  pos = infoROD.find("SN");
	  if(pos!=(int)std::string::npos)
	    sscanf(infoROD.substr(pos+2,5).c_str(),"%d",&rodSN);
	  pos = infoROD.find("MDSP ver.");
	  if(pos!=(int)std::string::npos)
	    sscanf(infoROD.substr(pos+9,4).c_str(),"%d",&mdspVer);
	  pos = infoROD.find("code ver.");
	  if(pos!=(int)std::string::npos)
	    sscanf(infoROD.substr(pos+9,4).c_str(),"%x",&mdspCode);
	}
	DBscfg = 0;
	//	printf("Processing group %s (%s) ... \n",(*baseIter)->getDecName().c_str(), infoROD.c_str());
	if(DBscan->getName()=="PixScanResult"){
	  for(cfgIter = (*baseIter)->recordBegin(); cfgIter!=(*baseIter)->recordEnd(); cfgIter++){
	    if((*cfgIter)->getName().find("PixScanData")!=std::string::npos){
	      DBscfg = *cfgIter;
	      break;
	    }
	  }
	} else // simple directory stucture: it's all in the group diretory, no module data stored
	  DBscfg = (*baseIter);
	if(DBscfg!=0){
	  //	  printf("Processing scan %s ... \n",DBscfg->getDecName().c_str());
	  PixScan *ps = 0;
	  wrapPs *cfgPs = 0;
	  if(!link){
	    ps = new PixScan();
	    try{
	      ps->readConfig(DBscfg);
	    }catch(...){
	      QMessageBox::warning(this,"MA",("Error reading config for PixScan "+DBscfg->getDecName()).c_str());
	      delete ps;
	      return;
	    }
	    //ps->config().dump(std::cout);
	    try{
	      ps->readHisto(DBscfg);
	    }catch(SctPixelRod::BaseException& exc){
	      std::stringstream msg;
	      msg << exc;
	      QApplication::restoreOverrideCursor();
	      QMessageBox::warning(this,"MA TopWin::loadPixScanData",
				   ("Exception ("+msg.str()+") reading histos for PixScan "+DBscfg->getDecName()).c_str());
	      delete ps;
	      return;
	    }catch(...){
	      QApplication::restoreOverrideCursor();
	      QMessageBox::warning(this,"MA TopWin::loadPixScanData",
				   ("Exception reading histos for PixScan "+DBscfg->getDecName()).c_str());
	      delete ps;
	      return;
	    }
	    cfgPs = new wrapPs();
	    cfgPs->config()=ps->config();
	    //	    m_pixScans.push_back(cfgPs);
	  }
	  if(DBscan->getName()=="PixScanResult"){
	    std::string db_path = fname;//gDirectory->GetPath();
	    //db_path.erase(db_path.length()-1,1);
	    db_path += ":";
	    db_path += DBscfg->getDecName();
	    db_path.erase(db_path.length()-1,1);
	    int slpos = db_path.find_last_of("/");
	    if(slpos!=(int)std::string::npos) 
	      db_path.erase(slpos, db_path.length()-slpos);
	    recordIterator mStart = (*baseIter)->recordBegin();
	    recordIterator mEnd   = (*baseIter)->recordEnd();
	    if(mname!=0){
	      mStart = (*baseIter)->findRecord(modNam+"/PixModule");
	      if(mStart!=mEnd){
		mEnd = mStart;
		mEnd++;
	      }
	    }
	    for(modIter = mStart; modIter!=mEnd; modIter++){
	      if((*modIter)->getName().find("PixModule")!=std::string::npos){
		//		printf("checking module %s\n",(*modIter)->getDecName().c_str());
		std::string mname = TLogFile::GetNameFromPath((*modIter)->getDecName().c_str());
		// check if module sits on a stave/sector
		ModItem *staveIt = 0;
		PixLib::fieldIterator fit = (*modIter)->findField("geometry_Type");
		if(fit!=(*modIter)->fieldEnd()){
		  std::string ass_type;
		  int ass_SN=-1, ass_pos=-1;
		  myrdb->DBProcess(fit,PixLib::READ,ass_type);
		  if(ass_type!="unknown"){
		    fit = (*modIter)->findField("geometry_staveID");
		    if(fit!=(*modIter)->fieldEnd())
		      myrdb->DBProcess(fit,PixLib::READ,ass_SN);
		    fit = (*modIter)->findField("geometry_position");
		    if(fit!=(*modIter)->fieldEnd())
		      myrdb->DBProcess(fit,PixLib::READ,ass_pos);
		    ModItem *temp_parent=(ModItem*)(ModuleList->firstChild());
		    while(temp_parent!=0){
		      if(temp_parent->text(0)==QString::number(ass_SN))
			staveIt = temp_parent;
		      temp_parent = (ModItem*)temp_parent->nextSibling();
		    }
		    if(staveIt==0){
		      staveIt = new ModItem(ModuleList, QString::number(ass_SN), 0, STVITEM);
		      staveIt->setOpen(true);
		    }
		  }
		}
		// create new module item in tree view if neccessary
		modit = 0;
		std::vector<ModItem*> allMods = getAllModuleItems();
		for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
		  ModItem* test1 = *IT;
		  if(test1->text(0)==QString(mname.c_str())){
		    modit = test1;
		    break;
		  }
		}
		if(modit==0){
		  // to do: implement stave level if specified in RootDB file
		  if(staveIt==0)
		    modit = new ModItem(ModuleList,mname.c_str(), 0, MODITEM); 
		  else
		    modit = new ModItem(staveIt,mname.c_str(), 0, MODITEM); 
		  modit->setOpen(true);
		  SetCurrMod(modit);
		}
		//printf("DB path = %s, %s, %s\n",TLogFile::GetNameFromPath(DBscan->getDecName().c_str()).c_str(),db_path.c_str(), gDirectory->GetPath());
		if(link){
		  // just link to data entry, but only if not yet listed
		  DBtoMAData* dmd = new DBtoMAData(TLogFile::GetNameFromPath(DBscan->getDecName().c_str(),false).c_str(),
					 db_path.c_str(),mname.c_str());
		  if(checkDBLinkExists(dmd->GetName(), dmd->GetFullPath(), dmd->GetModName())){
		    delete dmd;
		    return;
		  } else{
		    datit = dmd;
		  }
		} else {
		  m_memdir->cd(); // make sure MA histos are created in memory
		  int error = 0;
		  datit = MAEngine::readFromPixScan(ps, *modIter, TLogFile::GetNameFromPath(DBscan->getDecName().c_str()).c_str(), &error, -1, db_path.c_str());
		  if(error){
		    QApplication::restoreOverrideCursor();
		    QMessageBox::warning(this,"Module Analysis","Error loading data for module " +
					 modit->text(0) + ":\n" + DatSet::ErrorToMsg(error).c_str());
		    QApplication::setOverrideCursor(Qt::waitCursor);
		  }
		}
		if(datit!=0){
		  if(!link){
		    // manually add ROD info and scan start time stamp
		    mylog = ((DatSet*)datit)->GetLog();
		    mylog->comments = timestamp;
		    mylog->PLLver = (-1)*(1000*rodRev+rodSN);
		    mylog->PCCver = (-1)*(1000*mdspVer+mdspCode);
		    // link to PixScan config object
		    ((DatSet*)datit)->setPixScan(cfgPs);
		  }
		  // add to tree view
		  ModItem *newItem = DataToTree(modit,datit);
		  data.push_back(newItem);
		  // compress if requested
		  if(!link && summary)
		    RemFit(newItem,false);
		}
		if(m_app!=0)
		  m_app->processEvents();
	      }
	    }
	  } else{
	    // load 32 dummy modules - for a start, fix later
	    for(int mod=0;mod<32;mod++){
	      modit = 0;
	      bool newGen=false;
	      std::vector<ModItem*> allMods = getAllModuleItems();
	      for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
		ModItem* test1 = *IT;
		if(test1->text(0)=="Module "+QString::number(mod)){
		  modit = test1;
		  break;
		}
	      }
	      if(modit==0){
		newGen = true;
		modit = new ModItem(ModuleList,"Module "+QString::number(mod), 0, MODITEM); 
		modit->setOpen(true);
	      }
	      m_memdir->cd(); // make sure MA histos are created in memory
	      int error = 0;
	      datit = MAEngine::readFromPixScan(ps, 0, TLogFile::GetNameFromPath(DBscan->getDecName().c_str()).c_str(), &error, mod);
	      if(error){
		std::cout << "Error loading data: " << DatSet::ErrorToMsg(error) << std::endl;
		// 			QApplication::restoreOverrideCursor();
		// 			QMessageBox::warning(this,"Module Analysis","Error loading data for module" +
		// 					     modit->text(0) + ":\n" + DatSet::ErrorToMsg(error).c_str());
		// 			QApplication::setOverrideCursor(Qt::waitCursor);
		delete datit;
		datit = 0;
	      }
	      if(datit!=0 && !link){
		// link to PixScan config object
		((DatSet*)datit)->setPixScan(cfgPs);
		// add to tree view
		ModItem *newItem = DataToTree(modit,datit);
		// compress if requested
		if(summary && !link)
		  RemFit(newItem,false);
	      } else if(newGen)
		delete modit;
	      if(m_app!=0)
		m_app->processEvents();
	    }
	  }
	  delete ps;
	}
      }
    }
#else // ifdef PIXEL_ROD - can't link to PixScan
    if(noPixScan_warning){
      noPixScan_warning=false;
      QMessageBox::warning(this,"MA","This version comes without PixLib::PixScan\n"
			   "-> can't process PixScan type RootDB data format.");
    }
#endif // ifdef PIXEL_ROD
  } else {
    // load scan data & config for each module - old structure using PixScanConfig
    // get scan config
    DBscfg = 0;
    for(baseIter = DBscan->recordBegin(); baseIter!=DBscan->recordEnd(); baseIter++){
      if((*baseIter)->getName().find("PixScanConfig")!=std::string::npos){
	DBscfg = *baseIter;
	break;
      }
    }
    if(DBscfg!=0){
      // get the associated modules
      for(baseIter = DBscan->recordBegin(); baseIter!=DBscan->recordEnd(); baseIter++){
	if((*baseIter)->getName().find("PixModule")!=std::string::npos){
	  std::string mname = TLogFile::GetNameFromPath((*baseIter)->getDecName().c_str());
	  //              printf("Processing module %s ... \n",mname.c_str());
	  modit = 0;
	  std::vector<ModItem*> allMods = getAllModuleItems();
	  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
	    ModItem* test1 = *IT;
	    if(test1->text(0)==QString(mname.c_str())){
	      modit = test1;
	      break;
	    }
	  }
	  if(modit==0)
	    modit = new ModItem(ModuleList,mname.c_str(), 0, MODITEM); 
	  // tmp histo objects etc for data reading
	  PixLib::Histo *myHi, tmpHi;
	  PixLib::fieldIterator hiField;
	  std::string decName;
	  // get S-curve fit histos for this module
	  std::vector<Histo*> thrFit;
	  try{
	    decName = "HistoThresh";
	    hiField = (*baseIter)->findField(decName);
	    if((*hiField)!=0 && hiField!=(*baseIter)->fieldEnd()){
	      myrdb->DBProcess(hiField,PixLib::READ, tmpHi);
	      myHi = new Histo(tmpHi);
	      thrFit.push_back(myHi);
	    }
	    decName = "HistoNoise";
	    hiField = (*baseIter)->findField(decName);
	    if((*hiField)!=0 && hiField!=(*baseIter)->fieldEnd()){
	      myrdb->DBProcess(hiField,PixLib::READ, tmpHi);
	      myHi = new Histo(tmpHi);
	      thrFit.push_back(myHi);
	    }
	    decName = "HistoChi2";
	    hiField = (*baseIter)->findField(decName);
	    if((*hiField)!=0 && hiField!=(*baseIter)->fieldEnd()){
	      myrdb->DBProcess(hiField,PixLib::READ, tmpHi);
	      myHi = new Histo(tmpHi);
	      thrFit.push_back(myHi);
	    }
	  }catch(...){
	    for(unsigned int k=0;k<thrFit.size();k++)
	      delete (thrFit[k]);
	    thrFit.clear();
	  }
	  // get raw data histogram for this module
	  std::vector<PixLib::Histo*> dataHisto;
	  int k;
	  for(k=0;k<1000;k++){
	    //                if((k/100)*100==k) printf("Checking histo %d\n",k);
	    decName = "HistoOcc_";
	    std::stringstream b;
	    b << k;
	    decName+= b.str();
	    try{
	      hiField = (*baseIter)->findField(decName);
	    }catch(...){
	      break;
	    }
	    if((*hiField)==0||hiField==(*baseIter)->fieldEnd()) break;
	    myrdb->DBProcess(hiField,PixLib::READ, tmpHi);
	    myHi = new Histo(tmpHi);
	    dataHisto.push_back(myHi);
	  }
	  // get ToT mean histograms for this module
	  std::vector<PixLib::Histo*> meanToTHisto;
	  for(k=0;k<1000;k++){
	    //                if((k/100)*100==k) printf("Checking histo %d\n",k);
	    decName = "HistoToTMean_";
	    std::stringstream b;
	    b << k;
	    decName+= b.str();
	    try{
	      hiField = (*baseIter)->findField(decName);
	    }catch(...){
	      break;
	    }
	    if((*hiField)==0||hiField==(*baseIter)->fieldEnd()) break;
	    myrdb->DBProcess(hiField,PixLib::READ, tmpHi);
	    myHi = new Histo(tmpHi);
	    meanToTHisto.push_back(myHi);
	  }
	  // get ToT sigma histograms for this module
	  std::vector<PixLib::Histo*> sigToTHisto;
	  for(k=0;k<1000;k++){
	    //                if((k/100)*100==k) printf("Checking histo %d\n",k);
	    decName = "HistoToTSigma_";
	    std::stringstream b;
	    b << k;
	    decName+= b.str();
	    try{
	      hiField = (*baseIter)->findField(decName);
	    }catch(...){
	      break;
	    }
	    if((*hiField)==0||hiField==(*baseIter)->fieldEnd()) break;
	    myrdb->DBProcess(hiField,PixLib::READ, tmpHi);
	    myHi = new Histo(tmpHi);
	    sigToTHisto.push_back(myHi);
	  }
	  //              printf("found %d raw-data and %d fit histos for module %s\n",dataHisto.size(),thrFit.size(),mname.c_str());
	  // create logfile object from DB
	  mylog = new TLogFile(*baseIter,DBscfg);
	  // load data into MA
	  m_memdir->cd(); // make sure MA histos are created in memory
	  //
	  bool isscan = mylog->MyScan->Inner->Scan>0;
	  // current data types:
	  // DIG: occupancy raw data (with and without scan)
	  // ONDEFIT: occupancy + S-curve fit data
	  // ONDESCAN: S-curve fit data only
	  // TOTR: ToT data (with and without scan)
	  int error;
	  int datType = NONE;
	  if(meanToTHisto.size()>0){
	    datType = TOTR;
	  } else if(dataHisto.size()>0){
	    datType = DIG;
	    if(thrFit.size()>0)
	      datType = ONEDFIT; // occupancy plus fit data
	  } else if(thrFit.size()>0)
	    datType = ONEDSCAN;
	  if(datType==ONEDSCAN){
	    datit = new DatSet(TLogFile::GetNameFromPath(DBscan->getDecName().c_str()).c_str(), datType, false, mylog, thrFit, &error);
	  }else{
	    datit = new DatSet(TLogFile::GetNameFromPath(DBscan->getDecName().c_str()).c_str(), datType, isscan, mylog, dataHisto, &error);
	    if(!error && datType==ONEDFIT)
	      error = ((DatSet*)datit)->LoadThrHisto(thrFit);
	    if(!error && datType==TOTR)
	      error = ((DatSet*)datit)->LoadToTHisto(meanToTHisto, sigToTHisto);
	  }
	  for(k=0;k<(int)dataHisto.size();k++)
	    delete (dataHisto[k]);
	  dataHisto.clear();
	  for(k=0;k<(int)thrFit.size();k++)
	    delete (thrFit[k]);
	  thrFit.clear();
	  if(error){
	    QApplication::restoreOverrideCursor();
	    QMessageBox::warning(this,"Module Analysis","Error loading data for module" +
				 modit->text(0) + ":\n" + DatSet::ErrorToMsg(error).c_str());
	    QApplication::setOverrideCursor(Qt::waitCursor);
	  } //else
	  ModItem *newItem = DataToTree(modit,datit);
	  data.push_back(newItem);
	}
      }
    }
  }
}
#endif
void TopWin::DateFromLog(const char *log_comments, QString &theDate, QString &theTime){
  QStringList date = QStringList::split("\n",log_comments,TRUE);
  date = date.grep("Scan commenced");
  QStringList::Iterator it = date.end(); --it;
  date = date.split(' ',*it,FALSE);
  it = date.end(); --it;
  theDate = *it;
  --it; --it;
  theTime = *it;
}

/***************************************************************************/
//
double TopWin::BI_getTimeFromStrings(QString &theDate, QString &theTime){

  if(theDate.isEmpty() || theTime.isEmpty()) return -999;

        int month, day, year, hours, mins, secs;
                char strmonth[3];
        char str[100];

        strcpy(str, theDate.latin1());
        sscanf(str, "%2d-%2d-%4d", &month, &day, &year);
                if(month<0 || day<0 || year<0){
                  sscanf(str, "%3c-%2d-%4d", strmonth, &day, &year);
                  if(strncmp(strmonth,"Jan",3)==0) month=1;        
                  if(strncmp(strmonth,"Feb",3)==0) month=2;
                  if(strncmp(strmonth,"Mar",3)==0) month=3;
                  if(strncmp(strmonth,"Apr",3)==0) month=4;
                  if(strncmp(strmonth,"May",3)==0) month=5;
                  if(strncmp(strmonth,"Jun",3)==0) month=6;
                  if(strncmp(strmonth,"Jul",3)==0) month=7;
                  if(strncmp(strmonth,"Aug",3)==0) month=8;
                  if(strncmp(strmonth,"Sep",3)==0) month=9;
                  if(strncmp(strmonth,"Oct",3)==0) month=10;
                  if(strncmp(strmonth,"Nov",3)==0) month=11;
                  if(strncmp(strmonth,"Dec",3)==0) month=12;
                }
        strcpy(str, theTime.latin1());
        sscanf(str, "%2d:%2d:%2d", &hours, &mins, &secs);

        ////QDateTime dt, dtZero;
        ////dtZero.setTime_t(0);
        ////dt.setTime(QTime(hours, mins, secs));
        ////dt.setDate(QDate(year, month, day));
        ////return(dtZero.secsTo(dt)-3600); // one hour summer time shift??

//         struct tm gmt;
        
//         gmt.tm_mday=day; 
//         gmt.tm_mon=month-1; 
//         gmt.tm_year=year-1900;  
//         gmt.tm_hour=hours;  // 1h shift on some PCs ??????
//         gmt.tm_min=mins; 
//         gmt.tm_sec=secs;

//         return((double)mktime(&gmt));
        TTimeStamp s(year-1900,month,day,hours,mins,secs,0,false,0);
        return (double)s.GetSec();
                     
}
/***************************************************************************/

void TopWin::SaveCanvas(const char *inpath){
  QString path=QString::null;

  if(inpath!=0){
    path = inpath;
    if(path.right(3)!=".ps" && path.right(4)!=".gif")
      path += ".ps";
  } else{
    QStringList filter("PS file (*.ps)");
    filter +=  "GIF file (*.gif)";
    filter += "Any file (*.*)";
    
    Q3FileDialog fdia(QString::null, QString::null,this,"select output file",TRUE);
    fdia.setFilters(filter);
    fdia.setMode(Q3FileDialog::AnyFile);
    fdia.setSelection(path);
    
    if(fdia.exec() == QDialog::Accepted){
      path = fdia.selectedFile();
      if(fdia.selectedFilter()=="PS file (*.ps)"){
        if(path.right(3)!=".ps")
          path += ".ps";
      } else if(fdia.selectedFilter()=="GIF file (*.gif)"){
        if(path.right(4)!=".gif")
          path += ".gif";
      }
    }
  }

  if(!path.isEmpty())
    rootmain->SaveCan(path.latin1());

  return;
}
void TopWin::PrintCanvas(){
  /* problems under QT4, never really worked anyway...
  QPrinter myprinter(QPrinter::Compatible);
  QString path;
  QString sysname = gSystem->GetName();
  if(sysname=="WinNT")       // windows
    path = "C:/MAtempcanvas.ps";
  else                       // linux
    path = "/tmp/MAtempcanvas.ps";
  SaveCanvas(path.latin1());
  // print
  myprinter.setDocName(path);
  myprinter.setup(this);
  // delete temp file
  if(sysname=="WinNT")       // windows
    path = "del C:\\MAtempcanvas.ps";
  else                       // linux
    path = "rm /tmp/MAtempcanvas.ps";
  system(path.latin1());
  */
}
void TopWin::SaveAllData(const char *in_path, const char *in_comment){
  ModItem *item;// = (ModItem*)ModuleList->firstChild();
  std::vector<ModItem*> allMods = getAllModuleItems();
  if(allMods.size()==0) return;
  textwin->setText(" ");
  int oachoice;
  TFile *output;
  QString path, messg;
  QStringList filter("ROOT file (*.root)");
  filter += "Any file (*.*)";
  // load default dir. if requested to
  if(in_path!=0)
    path = in_path;
  else
    path = QString::null;
  if(path==QString::null && options->m_Opts->m_DefDataPath!=".")
    path = options->m_Opts->m_DefDataPath.c_str();

  if(in_path==0){
    Q3FileDialog fdia(path, QString::null,this,"getfilename",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    fdia.setFilters(filter);
    if(fdia.exec() == QDialog::Accepted && !fdia.selectedFile().isEmpty())
      path = fdia.selectedFile();
    else
      path = QString::null;
  }
  if(path==QString::null) return;

  // check if filename ends with root, otherwise attach
  if(path.find(".root",-5)<0)
    path += ".root";
  // see if file exists; if yes, ask user whether to overwrite or append
  oachoice = QMessageBox::Yes;
  FILE *testfile = fopen(path.latin1(),"r");
  if(testfile!=0){
    fclose(testfile);
    QMessageBox overwrt("Overwrite file?", 
                        "File exists:" + path + "\nDo you want to replace or append?",
                        QMessageBox::Warning,QMessageBox::Yes, QMessageBox::No, QMessageBox::Abort,
                        this, "overwrite_file",TRUE,Qt::WType_TopLevel);
    overwrt.setButtonText(QMessageBox::Yes,QString("Replace"));
    overwrt.setButtonText(QMessageBox::No ,QString("Append"));
    oachoice = overwrt.exec();
  }
  // hour glass cursor
  // open root file and write DatSet as TObject
  if(oachoice==QMessageBox::Yes){
    output = new TFile(path.latin1(),"RECREATE");
  }else if(oachoice==QMessageBox::No){
    output = new TFile(path.latin1(),"UPDATE");
    TObject *tmpto;
    tmpto = output->Get("ModuleFolder");
    if(tmpto==0){
      QMessageBox::warning(this,"Module Analysis","SaveAll: existing file does not"
                           "have module structure\n->can't save.");
      return;
    }
  } else
    return;

  TObjString mn("ModuleFolder");
  mn.Write("ModuleFolder");

  TObjString ver_to(m_verstrg.latin1());
  ver_to.Write("version");
  QString qcmt = "";
  if(in_comment!=0){
    qcmt = in_comment;
    qcmt += "\n";
  }
  qcmt += "Saved on ";
  qcmt += QDateTime::currentDateTime().toString();
  TObjString comment(qcmt.latin1());
  comment.Write("comment");

  QApplication::setOverrideCursor(Qt::waitCursor);
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    item = *IT;
    textwin->setText("Saving module "+item->text(0));
    textwin->repaint();
    if(m_app!=0)
      m_app->processEvents();
    output->mkdir(item->text(0).latin1(),"MA module");
    output->cd(item->text(0).latin1());
	//******************************************************************************************
    if(item->Parent()!=0 && item->Parent()->GetPID()==STVITEM){
      QString stave   = "";
	  stave = (item->Parent()->text(0).latin1());
	  TObjString st(stave.latin1());
	  st.Write("Stave");

      QString pos="";
	  pos.setNum(item->GetModID());
	  TObjString position(pos);
	  position.Write("position");
	}
	//******************************************************************************************
    saveDataCore(item,true,gDirectory);
    output->cd();
  }
  output->Close();
  textwin->setText("done");
  QApplication::restoreOverrideCursor();
  return;
}

int TopWin::saveDataCore(ModItem *item, bool doall, TDirectory *outdir){
  if(item==0) return 0;
  if(outdir==0) return 0;
  int i, j, k, ndat=0;
  char cndat[5], dirnam[20];
  DatSet *did;
  ModItem *dset_child, *sub_child, *tmp;
  ModItem *dsetit = (ModItem*)item->firstChild();
  for(i=0;i<item->childCount() && dsetit!=0;i++){
    did = dsetit->GetDID();
    if(did!=0 && (doall || dsetit->isSelected())){
      sprintf(dirnam,"data-%d",ndat);
      outdir->mkdir(dirnam,"ModuleAnalysis data");
      outdir->cd(dirnam);
      did->Write();
      
      // store reference info if there is any
      dset_child = (ModItem*) dsetit->firstChild();
      while(dset_child!=0){
        for(j=0;j<100 && dset_child->GetRID(j)!=0;j++){
          tmp = (ModItem*)item->firstChild();
          for(k=0;k<item->childCount() && tmp!=0;k++){
            if(tmp->GetDID()==dset_child->GetRID(j)){
              sprintf(cndat,"%d",k);
              TObjString refid_to(cndat);
              refid_to.Write("refid");
              sprintf(cndat,"%d",dset_child->GetPID());
              TObjString reft_to(cndat);
              reft_to.Write("reftype");
              TObjString refnam_to(dset_child->text(0).latin1());
              refnam_to.Write("refnam");
              break;
            }
            tmp = (ModItem*)tmp->nextSibling();
          }
        }
        // check sub-items too
        sub_child = (ModItem*) dset_child->firstChild();
        while(sub_child!=0){
          for(j=0;j<100 && sub_child->GetRID(j)!=0;j++){
            tmp = (ModItem*)item->firstChild();
            for(k=0;k<item->childCount() && tmp!=0;k++){
              if(tmp->GetDID()==sub_child->GetRID(j)){
                sprintf(cndat,"%d",k);
                TObjString refid_to(cndat);
                refid_to.Write("refid");
                sprintf(cndat,"%d",sub_child->GetPID());
                TObjString reft_to(cndat);
                reft_to.Write("reftype");
                TObjString refnam_to(sub_child->text(0).latin1());
                refnam_to.Write("refnam");
                break;
              }
              tmp = (ModItem*)tmp->nextSibling();
            }
          }
          sub_child = (ModItem*) sub_child->nextSibling();
        }
        // end sub-childs
        dset_child = (ModItem*) dset_child->nextSibling();
      }
      ndat++;
    }
    dsetit = (ModItem*)dsetit->nextSibling();
  }

  return ndat;
}
void TopWin::RemoveAllMod(bool warn){
  std::vector<ModItem*> allMods = getAllModuleItems();
  if(allMods.size()==0) return;

  QMessageBox ays("Delete items?", 
                  "This will delete all data of all modules.\nAre yo sure?",
                  QMessageBox::Warning,QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton,
                  this, "ays",TRUE,Qt::WType_TopLevel);
  bool contExe = !warn;
  if(warn && ays.exec()==QMessageBox::Yes) contExe = true;
  if(contExe){
    QApplication::setOverrideCursor(Qt::waitCursor);
    for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
      textwin->setText("Deleting module "+(*IT)->text(0));
      textwin->repaint();
      if(m_app!=0)
        m_app->processEvents();
      RemoveMod((*IT),false);
    }
    textwin->setText("done");
    QApplication::restoreOverrideCursor();
  }
  return;
}
void TopWin::rootdbFitAll(){
  int mychip = -1;
  if(ChipOrMod->isChecked())
    mychip = ChipSel->value();
  m_fitting->rootdbFitAll(mychip);
}
void TopWin::FitScurves(bool do_all){
  FitAll(0,0,0,-1, do_all);
}
void TopWin::FitAll(int fit_type, int fit_min, int fit_max, int chi_max, bool do_all){
  ModItem *ditem;//, *mitem = (ModItem*)ModuleList->firstChild();
  std::vector<ModItem*> allMods = getAllModuleItems();
  if(allMods.size()==0){
    textwin->setText("Can't find any data for fitting");
    return;
  }
  // get fit options from user, load calib with what's in the log file
  int chip;
  float **calfac, capfac;
  calfac = new float*[NCHIP];
  for(chip=0;chip<NCHIP;chip++)
    calfac[chip] = new float[4];
  // suggest choice of fit function
  m_FitWin->FitType->setCurrentItem(fit_type);
  // apply settings
  m_FitWin->FitSelected(m_FitWin->FitType->currentItem());
  // disable calib matters
  m_FitWin->ChipCalBox->setEnabled(false);
  m_FitWin->CfacEdit->setEnabled(false);
  m_FitWin->ChipCfac->setEnabled(false);
  m_FitWin->LoadButt->setEnabled(false);
  m_FitWin->autoCalNote->show();
  m_FitWin->autoCalNote2->show();
  // show fit window
  if(fit_min==fit_max || chi_max>0){
    if(m_FitWin->exec()==0) return;
  }else{
    m_FitWin->FitXmin->setValue(fit_min);
    m_FitWin->FitXmax->setValue(fit_max);
    m_FitWin->ChiCut->setValue(chi_max);
  }
  // reset calib matters in fitwindow
  m_FitWin->ChipCalBox->setChecked(true);
  m_FitWin->ChipCalBox->setEnabled(true);
  m_FitWin->CfacEdit->setEnabled(true);
  m_FitWin->ChipCfac->setEnabled(true);
  m_FitWin->LoadButt->setEnabled(true);
  m_FitWin->autoCalNote->hide();
  m_FitWin->autoCalNote2->hide();
  std::vector<ModItem *> dat_list;
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    ditem = (ModItem*) (*IT)->firstChild();
    while(ditem!=0){
      ModItem *tmpIt = (ModItem*)ditem->nextSibling();
      if((ditem->GetDID()->m_istype==DIG ||
          (ditem->GetDID()->m_istype==ONEDFIT && do_all))&& 
         ditem->GetDID()->m_isscan==1){
        if(ditem->GetDID()->GetLog()!=0){
          if((ditem->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" ||
              ditem->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff") &&
             ditem->GetDID()->GetLog()->MyScan->GetOuter()=="No Scan"){
            m_FitWin->ChipCalBox->setChecked(true);
            m_FitWin->CPCButt_pressed();
            for(chip=0;chip<NCHIP;chip++){
              calfac[chip] = new float[4];
              if(ditem->GetDID()->GetLog()->MyModule->CHigh)
                capfac  = ditem->GetDID()->GetLog()->MyModule->Chips[chip]->Chi;
              else
                capfac  = ditem->GetDID()->GetLog()->MyModule->Chips[chip]->Clo;
              if(ditem->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE"){
                calfac[chip][0] = capfac*ditem->GetDID()->GetLog()->MyModule->Chips[chip]->Vcal/0.160218;
                calfac[chip][2] = ditem->GetDID()->GetLog()->MyModule->Chips[chip]->VcalQu;
                calfac[chip][2] *= capfac/0.160218;
                calfac[chip][3] = ditem->GetDID()->GetLog()->MyModule->Chips[chip]->VcalCu;
                calfac[chip][3] *= capfac/0.160218;
              }else if(ditem->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff"){
                calfac[chip][0]  = ditem->GetDID()->GetLog()->MyModule->Chips[chip]->Clo*
                  ditem->GetDID()->GetLog()->extcal/0.160218;
                calfac[chip][2] = 0;
                calfac[chip][3] = 0;
              }else{
                calfac[chip][0] = 1;
                calfac[chip][2] = 0;
                calfac[chip][3] = 0;
              }
              calfac[chip][1] = 0;
            }
            m_FitWin->setCalTable(calfac);
          } else{
            m_FitWin->ChipCalBox->setChecked(false);
            m_FitWin->CPCButt_pressed();
            m_FitWin->CfacEdit->setText("1.0");
          }
        }
        // do the fit
        textwin->setText("Fitting data "+ditem->text(0)+" from "+
                         ditem->ModParent()->text(0));
        textwin->repaint();
        if(m_app!=0)
          m_app->processEvents();
        if(MAEngine::PerformFit(ditem,false,(ditem->GetDID()->GetLog()==0), !do_all, this, m_app)==0) // uses fitwin settings from above!
          QMessageBox::warning(this,"TopWin::FitAll","Error fitting data " + ditem->text(0) + " from module "
                               + ditem->ModParent()->text(0));
        
      }
      ditem = tmpIt;//(ModItem*) ditem->nextSibling();
    }
  }
  for(chip=0;chip<NCHIP;chip++){
    delete[] calfac[chip];
    calfac[chip] = 0;
  }
  delete[] calfac;
  textwin->setText("done");
  return;
}
void TopWin::RemoveAllRaw(ModItem *in_item, bool do_prompt){
  ModItem *ditem;
  std::vector<ModItem*> dataIt;

  if(in_item==0){
    std::vector<ModItem*> allMods = getAllModuleItems();
    for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
      ditem = (ModItem*) (*IT)->firstChild();
      while(ditem!=0){
        dataIt.push_back(ditem);
        ditem = (ModItem*) ditem->nextSibling();
      }
    }
  } else if(in_item->GetPID()==MODITEM){
    ditem = (ModItem*) in_item->firstChild();
    while(ditem!=0){
      dataIt.push_back(ditem);
      ditem = (ModItem*) ditem->nextSibling();
    }
  }
  
  if(dataIt.size()==0){
    textwin->setText("Can't find any data for removal");
    return;
  }
  
  bool go_for_it=true;
  // warn user and remove item if confirmed
  if(do_prompt){
    QString prmp_txt = "This will remove raw data from all modules\nAre you sure?";
    if(in_item!=0)
      prmp_txt = "This will remove all raw data\nfrom module "+in_item->text(0)+"\nAre you sure?";
    QMessageBox ays("Confirm delete", prmp_txt,
                    QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
                    this, "ays",TRUE,Qt::WType_TopLevel);
    if(ays.exec()!=QMessageBox::Ok)
      go_for_it = false;
  }

  if(go_for_it){
    for(std::vector<ModItem*>::iterator IT=dataIt.begin(); IT!=dataIt.end();IT++){
      ditem = *IT;
      if(ditem->GetDID()->m_istype==ONEDFIT){
        textwin->setText("Removing raw from data "+ditem->text(0)+" from "+
                         ditem->ModParent()->text(0));
        textwin->repaint();
        if(m_app!=0)
          m_app->processEvents();
        RemRaw(ditem, false);
      }
    }
  }
  textwin->setText("done");
  return;
}

void TopWin::CombHitMaps(){

  TwodSel tds(getAllModuleItems(),1,this,"blabla");
  if(tds.exec()!=0){
//     int scanpars[3]={tds.startBox->value(),tds.stopBox->value(),tds.stepBox->value()};
//     CombScurves(tds.getSelScan(),dataVec, scanpars, tds.delete1D->isChecked(), tds.scanLabel->text().latin1());
    CombHitMaps(tds.getSelScan(),tds.m_dataList, tds.m_scanPts, (tds.reverseScan->isChecked()&&tds.reverseScan->isEnabled()), 
                tds.delete1D->isChecked(), tds.scanLabel->text().latin1());
  }
  return;
}
void TopWin::CombHitMaps(int scanType, std::vector< std::vector<ModItem*> >dataVec, std::vector< std::vector<int> > scanPars, 
                         bool reverse, bool clear, const char *label){
  int i,j;
  //  int minPt = -1, mapPt = 22, offset=0;
  // run the actual work per module
  for(i=0;i<(int)dataVec.size();i++){
    if((int)dataVec[i].size()!=0 && dataVec[i][0]!=0){
      int rderr;
      std::string scan_label;
      if(label!=0)
        scan_label = label;
      else
        scan_label = "new 1D scan - generated";    
      int scanPts[500], scanInd[500];
      for(j=0;j<(int) dataVec[i].size();j++){
        scanPts[j]=scanPars[i][j];
      }
      TMath::Sort((int) dataVec[i].size(),scanPts,scanInd,reverse);
      TLogFile *log = new TLogFile(*(dataVec[i][scanInd[0]]->GetDID()->GetLog()));
      if(log!=0){
        log->MyScan->Inner->Scan  = scanType;
        log->MyScan->Inner->Step  = (int) dataVec[i].size();
        for(j=0;j<(int) dataVec[i].size();j++){
          log->MyScan->Inner->Pts[j]=scanPars[i][scanInd[j]];
        }
        log->MyScan->Inner->Pts[j] = -99999;
        log->MyScan->Inner->Start = log->MyScan->Inner->Pts[0];
        log->MyScan->Inner->Stop  = log->MyScan->Inner->Pts[j-1];
        
        // create data
        std::vector<TH2F*> myMaps;
        for(j=0;j<(int)dataVec[i].size();j++){
          if(dataVec[i][j]!=0)
            myMaps.push_back(dataVec[i][j]->GetDID()->GetMap(-1,RAW));
          else
            myMaps.push_back(0);
        }
        DatSet *dset = new DatSet(scan_label.c_str(), DIG, true, log, myMaps, &rderr);
        if(rderr){
          delete dset;
          QMessageBox::warning(this,"TopWin::CombHitMaps","Error creating 1D data for module "+
                               dataVec[i][0]->ModParent()->text(0)+"\n"+QString(DatSet::ErrorToMsg(rderr).c_str()));
        } else {
          DataToTree(dataVec[i][0]->ModParent(),dset);
        }
      }
    }
  }
  
  if(clear){
    for(i=0;i<(int)dataVec.size();i++){
      for(j=0;j<(int)dataVec[i].size();j++){
        RemData(dataVec[i][j],false);
      }
    }
  }
  
  return;
}
void TopWin::CombScurves(){
//   int i, val;
//   bool isOK;

  TwodSel tds(getAllModuleItems(),0,this,"blabla");
  if(tds.exec()!=0)
    CombScurves(tds.getSelScan(),tds.m_dataList, tds.m_scanPts, (tds.reverseScan->isChecked()&&tds.reverseScan->isEnabled()), 
                tds.delete1D->isChecked(), tds.scanLabel->text().latin1());
  return;
}
void TopWin::CombScurves(int scanType, std::vector< std::vector<ModItem*> >dataVec, std::vector< std::vector<int> > scanPars, 
                         bool reverse, bool clear, const char *label){
  int i,j;
  //  int minPt = -1, mapPt = 22, offset=0;
  // run the actual work per module
  for(i=0;i<(int)dataVec.size();i++){
    if((int)dataVec[i].size()!=0 && dataVec[i][0]!=0){
      int rderr;
      std::string scan_label;
      if(label!=0)
        scan_label = label;
      else
        scan_label = "new 2D scan - generated";    
      int scanPts[500], scanInd[500];
      for(j=0;j<(int) dataVec[i].size();j++){
        scanPts[j]=scanPars[i][j];
      }
      TMath::Sort((int) dataVec[i].size(),scanPts,scanInd,reverse);
      TLogFile *log = new TLogFile(*(dataVec[i][scanInd[0]]->GetDID()->GetLog()));
      if(log!=0){
        log->MyScan->Outer->Scan  = scanType;
        log->MyScan->Outer->Step  = (int) dataVec[i].size();
        for(j=0;j<(int) dataVec[i].size();j++){
          log->MyScan->Outer->Pts[j]=scanPars[i][scanInd[j]];
        }
        log->MyScan->Outer->Pts[j] = -99999;
        log->MyScan->Outer->Start = log->MyScan->Outer->Pts[0];
        log->MyScan->Outer->Stop  = log->MyScan->Outer->Pts[j-1];
        
        // create data
        DatSet *dset = new DatSet(scan_label.c_str(), TWODSCAN, false, log, &rderr);
        if(rderr){
          delete dset;
          QMessageBox::warning(this,"TopWin::CombScurves","Error creating 2D data for module "+
                               dataVec[i][0]->ModParent()->text(0)+"\n"+QString(DatSet::ErrorToMsg(rderr).c_str()));
        } else {
          for(j=0;j<(int)dataVec[i].size();j++){
            if(dataVec[i][j]!=0){
              dset->Load2DHisto(scanInd[j], dataVec[i][j]->GetDID()->GetMap(-1,MEAN), 
                                dataVec[i][j]->GetDID()->GetMap(-1,SIG), 
                                dataVec[i][j]->GetDID()->GetMap(-1,CHI));
            }
          }
          DataToTree(dataVec[i][0]->ModParent(),dset);
        }
      }
    }
  }
  
  if(clear){
    for(i=0;i<(int)dataVec.size();i++){
      for(j=0;j<(int)dataVec[i].size();j++){
        RemData(dataVec[i][j],false);
      }
    }
  }
  
  return;
}
int TopWin::getDacVal(TLogFile *log1D, TLogFile *logOut, int offset, int mapPt){
  if(log1D==0 || logOut==0)
    return -99999; // error

  //check for first fully operational and scanned chip
  int chip=0;
  for(chip=0;chip<NCHIP;chip++){
    if(log1D->MyModule->Chips[chip]->Switches[0] &&
       log1D->MyModule->Chips[chip]->Switches[1] &&
       log1D->MyModule->Chips[chip]->Switches[2]) break;
  }

  if(chip==NCHIP) return -99999; // no chip used for scanning

  TScanPars *myScan = logOut->MyScan->Outer;
  if(myScan->Scan==0) myScan = logOut->MyScan->Inner;

  int dacVal;
  switch(myScan->Scan){
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:  // global DACs, handled by array
    dacVal = log1D->MyModule->Chips[chip]->DACs[myScan->Scan-1]-offset;
    break;
  case 13: //trigger latency
    dacVal = log1D->MyScan->FElatency;
    break;
  case 14: // TDAC scan
  case 18: // TDAC relative scan
    dacVal = log1D->MyModule->Chips[chip]->TDACs->m_DAC[mapPt]-offset;
    break;
  case 15: // FDAC scan
    dacVal = log1D->MyModule->Chips[chip]->FDACs->m_DAC[mapPt]-offset;
    break;
  case 16: // GDAC scan
  case 19: // GDAC relative scan
    dacVal = log1D->MyModule->Chips[chip]->GDAC-offset;
    break;
  case 21: // trigger delay
    dacVal = log1D->MyScan->TrgDelay-offset;
    break;
  case 22: // strobe delay
    dacVal = log1D->MyModule->MCC->StrbDel-offset;
    break;
  case 23: // strobe duration
    dacVal = log1D->MyScan->StrbLength-offset;
    break;
  case 25: // MCC strobe delay
    dacVal = log1D->MyModule->MCC->StrbDel-offset;
    break;
  default:
    dacVal = -99999;
  }

  return dacVal;
}
int TopWin::CheckScanpt(TLogFile *logIn, TLogFile *logOut, int offset, int mapPt){
  if(logIn==0 || logOut==0)
    return -1; // error

  //check for first fully operational and scanned chip
  int chip=0;
  for(chip=0;chip<NCHIP;chip++){
    if(logIn->MyModule->Chips[chip]->Switches[0] &&
       logIn->MyModule->Chips[chip]->Switches[1] &&
       logIn->MyModule->Chips[chip]->Switches[2]) break;
  }

  if(chip==NCHIP) return -2; // no chip used for scanning

  int dacVal = getDacVal(logIn,logOut,offset,mapPt);
  if(dacVal==-99999) return -3;

  TScanPars *myScan = logOut->MyScan->Outer;
  if(myScan->Scan==0) myScan = logOut->MyScan->Inner;

  int scanPts[1000];
  bool reverse = (myScan->Stop<myScan->Start);
  for(int i=0;i<myScan->Step;i++){
    if(reverse)
      scanPts[i] = myScan->Pts[myScan->Step-i-1];
    else
      scanPts[i] = myScan->Pts[i];
  }
  int scanpt = TMath::BinarySearch(myScan->Step, scanPts, dacVal);
  if(reverse)
    scanpt = myScan->Step-scanpt-1;
  return scanpt;
}
int TopWin::getMinDAC(std::vector<ModItem*> datList, int mapPt, bool reverse){
  if(datList.size()==0) return -1;
  int dacArr[100], indArr[100], nArr=0;
  unsigned int i;

  int chip=0;
  for(chip=0;chip<NCHIP;chip++){
    if(datList[0]->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[0] &&
       datList[0]->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[1] &&
       datList[0]->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[2]) break;
  }

  if(chip==NCHIP) return -1;

  for(i=0; i<datList.size();i++){

    if(mapPt>0){
      dacArr[nArr] = datList[i]->GetDID()->GetLog()->MyModule->Chips[chip]->TDACs->m_DAC[mapPt];
    } else {
      dacArr[nArr] = datList[i]->GetDID()->GetLog()->MyModule->Chips[chip]->GDAC;
    }
    indArr[nArr] = nArr;
    nArr++;

  }

  TMath::Sort(nArr,dacArr,indArr,reverse);

  return indArr[0];
}
void TopWin::LoadMultiModule(){
  MultiLoad ml(this,"multi-load-panel");
  CutClassItem* cutClass=0;
  ModItem *tmpIt;
  std::vector<std::string> pdbPaths, modNames;
  std::vector<QString> anaDate, reportText, staveSNs;
  std::vector<float> allThermCond;
  std::vector<bool> isCold;
  QString stvLabels[13]={"M6C", "M5C", "M4C", "M3C", "M2C", "M1C" , "M0",
			 "M1A", "M2A", "M3A", "M4A", "M5A", "M6A"};
  int i, measType=0;
  if(ml.exec()){
    bool getVolts = (ml.cutsBox->currentText()=="BIST cuts");
    rootmain->CheckCan(); // somehow neccessary if lots of data loaded - memory problem?
    bool tmp_genpath = options->m_Opts->m_UseMname;
    options->m_Opts->m_UseMname = true;
    QString tmpPath = options->m_Opts->m_DefDataPath.c_str();
    int nRows=ml.moduleTable->numRows();
    Q3Table *qualitab = 0;
    BadPixel *bp = 0;
    float thermCond, onTemp;
    if(ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
      // prepare table output
      // electrical stuff first
      bp = new BadPixel(0,0,"qualitab_multi",FALSE,Qt::WDestructiveClose);
      bp->setCaption("Qualification table");
      bp->resize( 584, 650);
      bp->Headline->setText("");
      qualitab = bp->PixelList;
      qualitab->setNumRows(27+2*(int)getVolts);
      qualitab->setNumCols(nRows);
      // qrite headers etc
      qualitab->verticalHeader()->setLabel(0 ,"Module");
      qualitab->verticalHeader()->setLabel(1 ,"Meas. type");
      qualitab->verticalHeader()->setLabel(2 ,"Test Date");
      qualitab->verticalHeader()->setLabel(3 ,"Sensor T");
      qualitab->verticalHeader()->setLabel(4 ,"Sensor Ileak");
      qualitab->verticalHeader()->setLabel(5 ,"NTC");
      qualitab->verticalHeader()->setLabel(6 ,"Source scan bad pixel");
      qualitab->verticalHeader()->setLabel(7 ,"Source scan masked pixel");
      qualitab->verticalHeader()->setLabel(8 ,"Digital bad pixel");
      qualitab->verticalHeader()->setLabel(9 ,"ANA_BAD");
      qualitab->verticalHeader()->setLabel(10,"DISC_BAD");
      qualitab->verticalHeader()->setLabel(11,"XTALK_BAD");
      qualitab->verticalHeader()->setLabel(12,"MONLEAK_BAD");
      qualitab->verticalHeader()->setLabel(13,"INTIME_BAD");
      qualitab->verticalHeader()->setLabel(14,"TOT_BAD");
      qualitab->verticalHeader()->setLabel(15,"Analog bad pixel");
      qualitab->verticalHeader()->setLabel(16,"Total bad pixel");
      qualitab->verticalHeader()->setLabel(17,"Average noise");
      qualitab->verticalHeader()->setLabel(18,"Threshold dispersion");
      qualitab->verticalHeader()->setLabel(19,"T power off");
      qualitab->verticalHeader()->setLabel(20,"IDDA");
      qualitab->verticalHeader()->setLabel(21,"IDD");
      qualitab->verticalHeader()->setLabel(22,"VDDA");
      qualitab->verticalHeader()->setLabel(23,"VDD");
      qualitab->verticalHeader()->setLabel(24,"Module power");
      qualitab->verticalHeader()->setLabel(25,"T power on");
      qualitab->verticalHeader()->setLabel(26,"dT/P (degC/W)");
      if(getVolts){
	qualitab->verticalHeader()->setLabel(27,"VDD  (dig.sc.)");
	qualitab->verticalHeader()->setLabel(28,"VDDA (dig.sc.)");
      }
      qualitab->verticalHeader()->adjustHeaderSize ();
      if(ml.cutsBox->currentItem()<ml.cutsBox->count()-1){
        // make sure we have default cuts loaded
        //        m_cutpanel->NewClass();
        MAEngine::defineSpecialCuts(*m_cutpanel,ml.cutsBox->currentItem());
        cutClass = (CutClassItem*)m_cutpanel->ClassList->currentItem();
        //CutParam *cp = m_cutpanel->GetCutPars(cutClass->itemID());
        // check if we need special cuts
        //        switch(ml.cutsBox->currentItem()){
        //        }
      }
    }
    // show a panel which allows user-interrupt
    QMessageBox *stop_win = new QMessageBox("Abort","Abort upload/analysis",
                                            QMessageBox::NoIcon,QMessageBox::Abort,0,0,
                                            NULL,"abort_upload",FALSE);
    stop_win->show();
    for(i=0;i<nRows;i++){
      QString theDate, theTime;
      ModItem *ambData = 0;
      ModItem *item = 0;
      thermCond = -99;
      onTemp = 1;
      if(!stop_win->isVisible()){
        delete stop_win;
        //        delete wait_win;
        if(ml.runPDB->isChecked() && ml.runPDB->isEnabled())
          delete bp;
        QMessageBox::warning(this,"ModuleAnalysis","Upload/analysis was aborted successfully.");
        return;
      }
      ModItem *staveItem = 0;
      if(!ml.moduleTable->text(i,1).isEmpty()){
//         bool newEntry = true;
//         for(int nn=0;nn<staveSNs.size();nn++)
//           if(staveSNs[nn]==ml.moduleTable->text(i,1))
//             newEntry = false;
//         if(newEntry)
//           staveSNs.push_back(ml.moduleTable->text(i,1));
        ModItem* tempItem = (ModItem*)ModuleList->firstChild();
        while(tempItem!=0){
          if(tempItem->GetPID()==STVITEM && tempItem->text(0)==ml.moduleTable->text(i,1).right(4))
            staveItem = tempItem;
          tempItem = (ModItem*)tempItem->nextSibling();
        }
        if(staveItem==0){
          staveItem = new ModItem(ModuleList, ml.moduleTable->text(i,1).right(4), 0, STVITEM);
          staveItem->setOpen(true);
        }
      }
      if(ml.moduleTable->text(i,0).right(5)==".root"){ // load ROOT file
        StdTestWin stdwin(this,"tmptestwin",false,0,item);
        bool* loadData = new bool[stdwin.NSLOTS];
        if(ml.multiPlots->isChecked() && ml.multiPlots->isEnabled()){
          stdwin.ModuleName->setText(ml.moduleTable->text(i,0));
          int testType=NTYPES-1;
          stdwin.TestTypeSel->setCurrentItem(testType); // set to BIST type
          int count=0;
          for(std::vector<StdTestItem*>::iterator STD_IT=stdwin.m_items.begin();
              STD_IT!=stdwin.m_items.end(); STD_IT++){
            loadData[count] = (*STD_IT)->m_testtp[testType];
            count++;
          }
        } else{
          int cuttype = ml.cutsBox->currentItem();
          if(!ml.loadNeededFiles->isChecked())
            cuttype = -1; // will enable all data
          MAEngine::setNeededData(loadData,cuttype);
        }
        item = LoadRoot_Menu(false,ml.moduleTable->text(i,0).latin1(), staveItem, loadData);
	item->SetModPos(ml.moduleTable->text(i,2).toInt());
        delete loadData;
        if(ml.runPDB->isChecked()  && ml.runPDB->isEnabled() && item!=0){
	  int sID=item->GetModID();
	  if(sID>0 && sID<14)
	    qualitab->horizontalHeader()->setLabel(i,stvLabels[sID-1]);
	  else
	    qualitab->horizontalHeader()->setLabel(i,"Module "+QString::number(i));
	  qualitab->setText(0 ,i,item->text(0));
	}
        ModItem *tmpit = (ModItem*)item->firstChild();
        while(tmpit!=NULL){
          if((tmpit->GetDID()->GetStdtest()%100)==39)
            ambData = tmpit; // found std-ambush item
          tmpit = (ModItem*) tmpit->nextSibling();
        }
      }else if(!ml.pathToModules->text().isEmpty()){ // create new module and load std. data
        item = ModuleButton_clicked(ml.moduleTable->text(i,0).latin1(), staveItem);
        if(ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
          int sID=ml.moduleTable->text(i,2).toInt();
	  if(sID>0 && sID<14)
	    qualitab->horizontalHeader()->setLabel(i,stvLabels[sID-1]);
	  else
	    qualitab->horizontalHeader()->setLabel(i,"Module "+QString::number(i));
	  qualitab->setText(0 ,i,ml.moduleTable->text(i,0));
	}
        StdTestWin stdwin(this,"tmptestwin",false,0,item);
        stdwin.ModuleName->setText(item->text(0));
        int testType=ml.meastypeBox->currentItem();
        stdwin.TestTypeSel->setCurrentItem(testType);
        stdwin.Index->setValue(ml.measIndex->value());
        options->m_Opts->m_DefDataPath = ml.pathToModules->text().latin1();
        stdwin.TypeSet();
        stdwin.GenerateNames();
        // debug
        //stdwin.exec();
        // end debug
        // disable data sets the user chose not to load
        for(int nstd=0;nstd<stdwin.NSLOTS;nstd++){
          if(!ml.m_loadStd[nstd])
            stdwin.m_items[nstd]->m_fname->setText("");
        }
        //stdwin.OKButt->setEnabled(false);
        //stdwin.show();
        MAEngine::LocalLoadStd(this, item,stdwin,m_app, textwin);
        if(ml.getAmbush->isChecked() && !ml.ambushDir->text().isEmpty())
          ambData = LoadData(item,(ml.pathToModules->text()+"M"+ml.moduleTable->text(i,0)+"/"
                                   +ml.ambushDir->text()+"curra.log").latin1(), 
                             "Ambush logs",BI_VIT_DATA,0);
      }
      if(!stop_win->isVisible()){
        delete stop_win;
        //        delete wait_win;
        if(ml.runPDB->isChecked() && ml.runPDB->isEnabled())
          delete bp;
        QMessageBox::warning(this,"ModuleAnalysis","Upload/analysis was aborted successfully.");
        return;
      }
      bool isOK;
      int stave_pos;
      if(!ml.moduleTable->text(i,2).isEmpty()){
        stave_pos = ml.moduleTable->text(i,2).toInt(&isOK);
        if(isOK && stave_pos>0)
          item->SetModPos(stave_pos);
      }
      if(ambData!=0){
        ambData->GetDID()->SetStdtest(100*ml.meastypeBox->currentItem()+39);
        // update logfile info from ambush data
        int aSP, aCH, dSP, dCH, hSP, hCH;
        aSP = options->m_Opts->m_DDAsupp;
        aCH = options->m_Opts->m_DDAchan;
        dSP = options->m_Opts->m_DDsupp;
        dCH = options->m_Opts->m_DDchan;
        hSP = options->m_Opts->m_BIASsupp;
        hCH = options->m_Opts->m_BIASchan;
        ModItem *datit = (ModItem*) item->firstChild();
        while(datit!=0){
          if(datit!=ambData && datit->GetDID()->GetLog()!=0 && datit->GetDID()->GetLog()->MyDCS->Tntc<-98)
            // only modifiy if nothing usefule exists already
            datit->GetDID()->updateLogFromAmbush(ambData->GetDID(),aSP, aCH, dSP, dCH, hSP, hCH);
          datit = (ModItem*)datit->nextSibling();
        }
      }
      if(!stop_win->isVisible()){
        delete stop_win;
        //        delete wait_win;
        if(ml.runPDB->isChecked() && ml.runPDB->isEnabled())
          delete bp;
        QMessageBox::warning(this,"ModuleAnalysis","Upload/analysis was aborted successfully.");
        return;
      }
      if(item==0){
        if(options->m_Opts->m_showwarn) 
          QMessageBox::warning(this,"Module Analysis",
                               "Error reading/loading " + ml.moduleTable->text(i,0));
      } else if(ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
        PDBForm *myPDBForm = new PDBForm(this,"PDB Form Manager",TRUE,Qt::WType_TopLevel,item,m_app,
					 ml.m_PDBForm->outputPath->text().latin1());
        myPDBForm->LoadProfile();
        myPDBForm->pdbCombo->setCurrentItem(ml.m_PDBForm->pdbCombo->currentItem());
        myPDBForm->savePlots->setChecked(ml.m_PDBForm->savePlots->isChecked());
        myPDBForm->outputPath->setText(ml.m_PDBForm->outputPath->text());
        myPDBForm->SaveButton->setEnabled(false);
        myPDBForm->CreateButton->setEnabled(false);
        myPDBForm->CancelButton->setEnabled(false);
        measType = myPDBForm->pdbMeasType->currentItem();
        tmpIt = myPDBForm->ANAitem;
        theDate = "";
        if(tmpIt!=0)
          DateFromLog(tmpIt->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
        anaDate.push_back(theDate);
        myPDBForm->show();
        myPDBForm->CreatePDB();
        int allAna = myPDBForm->m_anaBadPixs+myPDBForm->m_discBadPixs+myPDBForm->m_xtBadPixs+
          myPDBForm->m_mlBadPixs+myPDBForm->m_intBadPixs+myPDBForm->m_totBadPixs;
        qualitab->setText(1 ,i,myPDBForm->pdbMeasType->currentText());
        theDate = "";
        if(myPDBForm->IVitem!=0 && myPDBForm->IVitem->GetDID()->GetLog()!=0)
          DateFromLog(myPDBForm->IVitem->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
        if(myPDBForm->DIGitem!=0 && myPDBForm->DIGitem->GetDID()->GetLog()!=0)
          DateFromLog(myPDBForm->DIGitem->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
        if(myPDBForm->ANAitem!=0 && myPDBForm->ANAitem->GetDID()->GetLog()!=0)
          DateFromLog(myPDBForm->ANAitem->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
        theDate = PDBForm::Date(theDate); // convert to PDB standard format
        qualitab->setText(2 ,i,theDate);
        qualitab->setText(3 ,i,QString::number(myPDBForm->m_IV_T,'f',1));
        qualitab->setText(4 ,i,QString::number(myPDBForm->m_rep->m_IV_oper,'f',3));
        qualitab->setText(5 ,i,QString::number(myPDBForm->m_NTC,'f',1));
        qualitab->setText(6 ,i,QString::number(myPDBForm->m_srcBadPixs));
        qualitab->setText(7 ,i,QString::number(myPDBForm->m_srcMasked));
        qualitab->setText(8 ,i,QString::number(myPDBForm->m_digBadPixs));
        qualitab->setText(9 ,i,QString::number(myPDBForm->m_anaBadPixs));
        qualitab->setText(10,i,QString::number(myPDBForm->m_discBadPixs));
        qualitab->setText(11,i,QString::number(myPDBForm->m_xtBadPixs));
        qualitab->setText(12,i,QString::number(myPDBForm->m_mlBadPixs));
        qualitab->setText(13,i,QString::number(myPDBForm->m_intBadPixs));
        qualitab->setText(14,i,QString::number(myPDBForm->m_totBadPixs));
        qualitab->setText(15,i,QString::number(allAna));
        qualitab->setText(16,i,QString::number(allAna+myPDBForm->m_srcBadPixs+myPDBForm->m_digBadPixs));
        qualitab->setText(17,i,QString::number(myPDBForm->m_rep->m_avgNse,'f',1));
        qualitab->setText(18,i,QString::number(myPDBForm->m_rep->m_sdThr,'f',1));
        if(!stop_win->isVisible()){
          delete stop_win;
          //        delete wait_win;
          if(ml.runPDB->isChecked() && ml.runPDB->isEnabled())
            delete bp;
          QMessageBox::warning(this,"ModuleAnalysis","Upload/analysis was aborted successfully.");
          return;
        }
        float offTemp=-99, Idda=0, Idd=0, Vdda=0, Vdd=0, ePow=0;
        if(myPDBForm->m_IV_T>-99)
          offTemp = myPDBForm->m_IV_T;
        onTemp =-99;
        tmpIt = myPDBForm->ANAitem;
        if(tmpIt!=0){
          // if logfile info is good then take V,I,T data from there
          if(tmpIt->GetDID()->GetLog()->MyDCS->Tntc>-98)
            onTemp = tmpIt->GetDID()->GetLog()->MyDCS->Tntc;
          for(int ch=0;ch<3;ch++){
            for(int spn=0;spn<10;spn++){
              if(tmpIt->GetDID()->GetLog()->MyDCS->LVvolt[spn][ch]>1.4
                 && tmpIt->GetDID()->GetLog()->MyDCS->LVvolt[spn][ch]<1.85
                 && tmpIt->GetDID()->GetLog()->MyDCS->LVcurr[spn][ch]>0.01){
                Vdda = tmpIt->GetDID()->GetLog()->MyDCS->LVvolt[spn][ch];
                Idda = tmpIt->GetDID()->GetLog()->MyDCS->LVcurr[spn][ch]/1e3;
              }
              if(tmpIt->GetDID()->GetLog()->MyDCS->LVvolt[spn][ch]>1.8
                 && tmpIt->GetDID()->GetLog()->MyDCS->LVvolt[spn][ch]<2.5
                 && tmpIt->GetDID()->GetLog()->MyDCS->LVcurr[spn][ch]>0.01){
                Vdd = tmpIt->GetDID()->GetLog()->MyDCS->LVvolt[spn][ch];
                Idd = tmpIt->GetDID()->GetLog()->MyDCS->LVcurr[spn][ch]/1e3;
              }
            }
          }
        }
        if(ambData!=0){
          // calculate thermal conductivity from ambush data
          double time, timeStart=1999999999, timeStop=0;
          tmpIt=(ModItem*)item->firstChild();
          while(tmpIt!=0){
            if(tmpIt->GetDID()->m_istype==IVSCAN){
              const char* cm = tmpIt->GetDID()->GetLog()->comments.c_str();        
              DateFromLog(cm, theDate, theTime);
              time= BI_getTimeFromStrings(theDate, theTime);
              if(offTemp<-98)
                offTemp = (float) ambData->GetDID()->BI_getAvgValueOnInterval(BI_NTC,time+30,time+120);
            }
            if(tmpIt->GetDID()!=0 && tmpIt->GetDID()->GetLog()!=0 && tmpIt->GetDID()->m_istype!=IVSCAN){
              const char* cm = tmpIt->GetDID()->GetLog()->comments.c_str();        
              DateFromLog(cm, theDate, theTime);
              time= BI_getTimeFromStrings(theDate, theTime);
              if(time>timeStop)
                timeStop = time;
              if(time<timeStart)
                timeStart = time;
            }
            tmpIt = (ModItem*) tmpIt->nextSibling();
          }
          int nTiter = 0;
          while(offTemp<-98 && nTiter<100){  // check for time before all scans started to get off status
            offTemp = (float) ambData->GetDID()->BI_getAvgValueOnInterval(BI_NTC,timeStart-180-60*(float)nTiter,
                                                                          timeStart-120-60*(float)nTiter);
            nTiter++;
          }
          if(offTemp<-98) // last try: closer to power on
            offTemp = (float) ambData->GetDID()->BI_getAvgValueOnInterval(BI_NTC,timeStart-120,timeStart-60);
          tmpIt = myPDBForm->ANAitem;
          if(tmpIt!=0){
            const char* cm = tmpIt->GetDID()->GetLog()->comments.c_str();        
            DateFromLog(cm, theDate, theTime);
            time= BI_getTimeFromStrings(theDate, theTime);
            timeStart = time+options->m_Opts->m_ambushStart;
            timeStop = time+options->m_Opts->m_ambushStop;
          }
          if(onTemp<-98)
            onTemp = (float)ambData->GetDID()->BI_getAvgValueOnInterval(BI_NTC,timeStart,timeStop);
          if(Idda<.1){
            Idda = ambData->GetDID()->BI_getAvgValueOnInterval(BI_ANALOG_CURR,timeStart,timeStop);
            Vdda = ambData->GetDID()->BI_getAvgValueOnInterval(BI_ANALOG_VOLT,timeStart,timeStop);
          }
          if(Idd<0.1){
            Idd  = ambData->GetDID()->BI_getAvgValueOnInterval(BI_DIG_CURR,timeStart,timeStop);
            Vdd  = ambData->GetDID()->BI_getAvgValueOnInterval(BI_DIG_VOLT,timeStart,timeStop);
          }
        }
        ePow = Vdd*Idd + Vdda*Idda;
        if(ePow!=0)
          thermCond = (onTemp-offTemp)/ePow;
        else
          thermCond = -99;
        qualitab->setText(19,i,QString::number(offTemp,'f',1));
        qualitab->setText(20,i,QString::number(Idda,'f',3));
        qualitab->setText(21,i,QString::number(Idd,'f',3));
        qualitab->setText(22,i,QString::number(Vdda,'f',3));
        qualitab->setText(23,i,QString::number(Vdd,'f',3));
        qualitab->setText(24,i,QString::number(ePow,'f',3));
        qualitab->setText(25,i,QString::number(onTemp,'f',1));
        qualitab->setText(26,i,QString::number(thermCond,'f',2));
        allThermCond.push_back(thermCond);
	if(getVolts){
	  qualitab->setText(27,i,QString::number(myPDBForm->m_rep->m_VDD,'f',2));
	  qualitab->setText(28,i,QString::number(myPDBForm->m_rep->m_VDDA,'f',2));
	}
        isCold.push_back((onTemp<0));
        //            qualitab->setText(15,i+1,QString::number(myPDBForm->m_rep->m_minVDD/1000,'f',2));
        //        if(ml.uploadBox->isChecked()){
          pdbPaths.push_back(myPDBForm->m_path.latin1());
          modNames.push_back(item->text(0).latin1());
          reportText.push_back(myPDBForm->ResultWindow->text());
          //        }
        myPDBForm->close();
        delete myPDBForm;
        if(!stop_win->isVisible()){
          delete stop_win;
          //        delete wait_win;
          delete bp;
          QMessageBox::warning(this,"ModuleAnalysis","Upload/analysis was aborted successfully.");
          return;
        }
      }
      if(ml.savetoRoot->isChecked() && !ml.pathToRoot->text().isEmpty() && ml.moduleTable->text(i,0).right(5)!=".root"){
        // generate real path from wild cards
        QString rpath = ml.pathToRoot->text();
        QString sind;
        sind.sprintf("%02d",ml.measIndex->value());
        rpath.replace("M%m","M"+item->text(0));
        rpath.replace("%m","M"+item->text(0));
        QString modsn = "20210020" + item->text(0);
        rpath.replace("%s",modsn);
        rpath.replace("%t",ml.meastypeBox->currentText());
        rpath.replace("%i",sind);
        // check if all folders in path exist
        QAReport tmp_rep(item,this,"tmp report");
        tmp_rep.LoadPath(rpath.latin1());
        // save to this path under a default name
        SaveData(item,true,(rpath + item->text(0) + "_" + 
                            ml.meastypeBox->currentText() + sind + ".root").latin1(),
                 "Auto-saved from multi-module analysis");
      }
      if(ml.runPDB->isChecked() && ml.runPDB->isEnabled() &&
         ml.removeAfterPDB->isChecked() && ml.removeAfterPDB->isEnabled()){
        RemoveMod(item,false);
        delete staveItem; // deletes stave/sector item if it was there
      }
      // bistave ultra-quick "analysis":
      // compress digital and threshold scan data, 
      // discard rest, and keep compr. data for summary plots
      if(ml.multiPlots->isChecked() && ml.multiPlots->isEnabled()){
        ModItem *oldItem, *datItem = (ModItem*)item->firstChild();
        while(datItem!=0){
          // produce summary or remove unused data
          oldItem = datItem;
          datItem = (ModItem*)oldItem->nextSibling();
          if((oldItem->GetDID()->GetStdtest()%100)==6 || (oldItem->GetDID()->GetStdtest()%100)==5){
            delete rootmain->ThreshSumm(oldItem,0);
          } else if((oldItem->GetDID()->GetStdtest()%100)==1){
            // produce threshold summary
            delete rootmain->ThreshSumm(oldItem,0);
          } else if((oldItem->GetDID()->GetStdtest()%100)==0){
            // produce digital ineff. summary
            rootmain->nBadPixs(oldItem);
          }else{ // not interested in the rest
            DatSet *mydata = oldItem->GetDID();
            delete mydata;
            delete oldItem;
          }
        }
        // then clear full info
        ClearAllFits(item,false);
        // close module tree item for better visibility
        item->setOpen(false);
      }
      if(!stop_win->isVisible()){
        delete stop_win;
        //        delete wait_win;
        if(ml.runPDB->isChecked() && ml.runPDB->isEnabled())
          delete bp;
        QMessageBox::warning(this,"ModuleAnalysis","Upload/analysis was aborted successfully.");
        return;
      }
    }
    delete stop_win;
    //    delete wait_win;
    if(ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
      bp->show();
      if(cutClass!=0)
        m_cutpanel->DeleteClass();
      //    if(ml.uploadBox->isChecked() && ml.m_PDBForm->m_uploadJava){
      QString qboxLabel;
      if(staveSNs.size()>0){
        qboxLabel = "Launch PDB upload for stave/sector(s) ";
        for(unsigned int kk=0;kk<staveSNs.size();kk++)
          qboxLabel+= staveSNs[kk];
        qboxLabel+= "?";
      }else{
        qboxLabel = "Launch PDB upload for module(s) ";
        qboxLabel+= modNames[0].c_str();
        if(modNames.size()>1){
          qboxLabel+= ", ";
          qboxLabel+= modNames[1].c_str();
          qboxLabel+= ",...?";
        } else
          qboxLabel+= "?";
      }
      QMessageBox *upload_win = new QMessageBox("PDB Upload?",qboxLabel,
                                                QMessageBox::Question,QMessageBox::YesAll,QMessageBox::Yes,QMessageBox::No,
                                                NULL,"PDBupload",FALSE);
      upload_win->setButtonText( QMessageBox::Yes, "Yes (module by module)" );
      upload_win->setButtonText( QMessageBox::YesAll, "Yes (all mod's in one go)" );
      upload_win->show();
      while(upload_win->isVisible()){
        if(m_app!=0)
          m_app->processEvents();
      }
      if(upload_win->result()==QMessageBox::Yes || upload_win->result()==QMessageBox::YesAll){ 
        QString uplRDBcmds="";
        bool storedDoWWW = ml.m_PDBForm->WWWcheckBox->isChecked();
        if(ml.m_PDBForm->webUploadMode->currentItem()>0) // upload is done at the end
          ml.m_PDBForm->WWWcheckBox->setChecked(false);
        if(!ml.m_PDBForm->m_uploadJava)
          QMessageBox::warning(this,"TopWin","Can't find java or PDB installation\nPDB upload not permitted");
        else{
          QString userPDB,passwdPDB;
          userPDB = options->m_Opts->m_userPDB.c_str();
          passwdPDB = options->m_Opts->m_passwdPDB.c_str();
          if(userPDB.isEmpty() || passwdPDB.isEmpty()){
            ml.m_lp->exec();
            userPDB = ml.m_lp->UserID->text();
            passwdPDB = ml.m_lp->Password->text();
          }
          std::string myName,pdbNames[4]={"iv","dig","ana","src"};
          QCheckBox *pdbBoxes[4] = {ml.m_PDBForm->IVcheckBox,
                                    ml.m_PDBForm->DIGcheckBox,
                                    ml.m_PDBForm->ANAcheckBox,
                                    ml.m_PDBForm->SRCcheckBox};
          for(i=0;i<(int)pdbPaths.size();i++){
            if(ml.m_PDBForm->webUploadMode->currentItem()==1){ // RDB upload is done at the end
              QString replCmd = ml.m_PDBForm->WebserverCmd->text();
              replCmd.replace("%m",modNames[i].c_str());
              replCmd.replace("%s",("20210020"+modNames[i]).c_str());
              if(uplRDBcmds!="")
                uplRDBcmds += ";";
              uplRDBcmds += replCmd;
            }
            ml.m_PDBForm->setCaption(("PDB File Manager (module "+modNames[i]+")").c_str());
            ml.m_PDBForm->modsn = ("20210020"+modNames[i]).c_str();
            ml.m_PDBForm->m_path = pdbPaths[i].c_str();
            ml.m_PDBForm->pdbMeasType->setCurrentItem(measType);
            ml.m_PDBForm->ResultWindow->setText(reportText[i]);
            ml.m_PDBForm->IcheckType();
            for(int j=0;j<4;j++){
              myName = pdbPaths[i]+"/PDB"+pdbNames[j]+".txt";
              FILE *testFile = fopen(myName.c_str(),"r");
              if(testFile!=0){
                pdbBoxes[j]->setEnabled(true);
                pdbBoxes[j]->setChecked(true);
                fclose(testFile);
              } else{
                pdbBoxes[j]->setEnabled(false);
                pdbBoxes[j]->setChecked(false);
              }
            }
            ml.m_PDBForm->m_User = userPDB;
            ml.m_PDBForm->m_Passwd = passwdPDB;
            if(upload_win->result()==QMessageBox::Yes){
              ml.m_PDBForm->SaveButton->setEnabled(false);
              ml.m_PDBForm->CreateButton->setText("Upload");
              ml.m_PDBForm->CreateButton->setEnabled(false);
              ml.m_PDBForm->mechDate->setText(ml.m_PDBForm->Date(anaDate[i]));
              if(isCold[i]){
                ml.m_PDBForm->thermCondCold->setText(QString::number(allThermCond[i],'f',2));
                ml.m_PDBForm->thermCondWarm->setText("");
              }else{
                ml.m_PDBForm->thermCondWarm->setText(QString::number(allThermCond[i],'f',2));
                ml.m_PDBForm->thermCondCold->setText("");
              }
              ml.m_PDBForm->TabWidget->setTabEnabled(ml.m_PDBForm->TabWidget->page(0),false);
              ml.m_PDBForm->TabWidget->setTabEnabled(ml.m_PDBForm->TabWidget->page(1),false);
              ml.m_PDBForm->TabWidget->setTabEnabled(ml.m_PDBForm->TabWidget->page(2),true);
              ml.m_PDBForm->TabWidget->setTabEnabled(ml.m_PDBForm->TabWidget->page(3),true);
              ml.m_PDBForm->TabWidget->setTabEnabled(ml.m_PDBForm->TabWidget->page(4),true);
              ml.m_PDBForm->TabWidget->setCurrentPage(3);
              ml.m_PDBForm->TabWidget->repaint();
              connect( ml.m_PDBForm->TabWidget, SIGNAL( currentChanged(QWidget*) ), ml.m_PDBForm, SLOT( enableUploadButt() ) );
              disconnect( ml.m_PDBForm->CreateButton, SIGNAL( clicked() ), ml.m_PDBForm, SLOT( CreatePDB() ) );
              connect( ml.m_PDBForm->CreateButton, SIGNAL( clicked() ), ml.m_PDBForm, SLOT( launchJavaUpload() ) );
              ml.m_PDBForm->PASScheckBox->setChecked(false);
              ml.m_PDBForm->ROOTcheckBox->setChecked(false);
              ml.m_PDBForm->PASScheckBox->setEnabled(false);
              ml.m_PDBForm->ROOTcheckBox->setEnabled(false);
//               ml.m_PDBForm->WWWcheckBox->setChecked(false);
//               if(ml.m_PDBForm->m_uploadWeb==0)
//                 ml.m_PDBForm->groupBoxWWW->setTitle("Web upload: NOT POSSIBLE, CAN'T FIND SSH!");
//               ml.m_PDBForm->WWWcheckBox->setEnabled(ml.m_PDBForm->m_uploadWeb);
//               ml.m_PDBForm->WWWcheckBox->setChecked(ml.m_PDBForm->m_uploadWeb && options->m_Opts->m_CutWebCopy);
//               ml.m_PDBForm->WWWclearBefore->setEnabled(ml.m_PDBForm->m_uploadWeb);
//               ml.m_PDBForm->WWWclearBefore->setChecked(ml.m_PDBForm->m_uploadWeb && options->m_Opts->m_pdbClear);
//               if(options->m_Opts->m_CutWebPath=="")
//                 ml.m_PDBForm->WebserverName->setText("");
//               else
//                 ml.m_PDBForm->WebserverName->setText((options->m_Opts->m_CutWebUser+"@"+
//                                                       options->m_Opts->m_CutWebPath).c_str());
//               ml.m_PDBForm->WebserverCmd->setText(options->m_Opts->m_CutWebCmd.c_str());
              ml.m_PDBForm->CreateButton->show();
              ml.m_PDBForm->checkMech();
              ml.m_PDBForm->exec();
            } else {
              ml.m_PDBForm->MECHcheckBox->setChecked(false);
              ml.m_PDBForm->PASScheckBox->setChecked(false);
              ml.m_PDBForm->ROOTcheckBox->setChecked(false);
              ml.m_PDBForm->m_pauseAfterWWW = false;
              ml.m_PDBForm->MECHcheckBox->setEnabled(false);
              ml.m_PDBForm->PASScheckBox->setEnabled(false);
              ml.m_PDBForm->ROOTcheckBox->setEnabled(false);
              ml.m_PDBForm->TabWidget->setCurrentPage(2);
              ml.m_PDBForm->TabWidget->repaint();
              ml.m_PDBForm->CreateButton->hide();
              ml.m_PDBForm->show();
              // run upload
              ml.m_PDBForm->launchJavaUpload();
              ml.m_PDBForm->close();
            }
          } // end module loop
        } // end java if
        // RDB upload - once at the end
        ml.m_PDBForm->WWWcheckBox->setChecked(storedDoWWW);
        ml.m_PDBForm->ROOTcheckBox->setChecked(false);
        switch(ml.m_PDBForm->webUploadMode->currentItem()){
        case 1:  // collection of commands
          if(uplRDBcmds!=""){
            ml.m_PDBForm->WebserverCmd->setText(uplRDBcmds);
            ml.m_PDBForm->m_pauseAfterWWW = true;
            ml.m_PDBForm->miscUpload();
          }
          break;
        case 2:  // one command, independend of module name
          ml.m_PDBForm->m_pauseAfterWWW = true;
          ml.m_PDBForm->modsn=""; // doesn't exist in this mode
          ml.m_PDBForm->miscUpload();
          break;
        default: // do nothing
          break;
        }
      }
      delete upload_win;
    }
    if(ml.multiPlots->isChecked() && ml.multiPlots->isEnabled()){
      const int nplots=5;
      int iplot;
      int data_type[nplots]={5,5,5,1,0};
      int array_item[nplots]={1,2,3,4,4};
      QString plotlabel[nplots]={"threshold dispersion (internal thresh.)",
                                 "avg. noise (internal thresh.)",
                                 "good analog pixels (internal thresh.)",
                                 "good bumps (HV off thresh. scan)",
                                 "digital good pixels (dig. SRAM)"};
      QString fplname[nplots] = {"bistsum_tdsip.ps", "bistsum_noise.ps", "bistsum_goodana.ps",
                                 "bistsum_goodbumps.ps","bistsum_gooddig.ps"};
      for(iplot=0;iplot<nplots;iplot++){
        PlotModuleGraphSummary(data_type[iplot] ,array_item[iplot], plotlabel[iplot].latin1());
        if(iplot<nplots-1)
          QMessageBox::question(this,"Module Analysis","Continue to next plot?");
        else
          QMessageBox::question(this,"Module Analysis","Finish?");
        if(ml.savePlots->isChecked() && !ml.plotsPath->text().isEmpty())
          // save canvas
          SaveCanvas((ml.plotsPath->text()+fplname[iplot]).latin1());
      }
    }
      //    } else if (ml.uploadBox->isChecked() && !ml.m_PDBForm->m_uploadJava)
    options->m_Opts->m_DefDataPath = tmpPath.latin1();
    options->m_Opts->m_UseMname = tmp_genpath;
  }
  return;
}
void TopWin::memoryTest(){
  system("export currmem=`ps lx | grep ModuleA | grep -v grep | awk '{print $7}'`;"
         " echo curr. memory consumed $currmem");
}

//**********SLOTs by JW ***************************************************************************
void TopWin::PlotMultiMean()
{
  PlotMultiProjection(MEAN,false);
}
void TopWin::PlotMultiSig()
{
  PlotMultiProjection(SIG,false);
}
void TopWin::OverlayMultiMean()
{
  PlotMultiProjection(MEAN,true);
}
void TopWin::OverlayMultiSig()
{
  PlotMultiProjection(SIG,true);
}
void TopWin::PlotGraphThrSummary()
{
  PlotModuleGraphSummary(5 ,0, "avg. threshold");
}
void TopWin::PlotGraphThrDispSummary()
{
  PlotModuleGraphSummary(5 ,1, "threshold dispersion");
}
void TopWin::PlotGraphNoiseSummary()
{
  PlotModuleGraphSummary(5 ,2, "avg. noise");
}
void TopWin::PlotGraphAnaGoodSummary()
{
  PlotModuleGraphSummary(5 ,3, "analog good pixels");
}
void TopWin::PlotGraphGoodBumpSummary()
{
  PlotModuleGraphSummary(1 ,4, "good bumps (HV off thresh. scan)");
}
void TopWin::PlotGraphDigSummary()
{
  PlotModuleGraphSummary(0, 4, "digital scan (No. good pixels)");
}
void TopWin::SelectOverview()
{
int plot, status;

  OverviewSelect os(this, "overview-selection");
  os.SystestChkBox->setChecked(SystestMode);
  os.GotoSystestMode();
  if(os.exec()){
    if(!os.SystestChkBox->isChecked()){
      plot = os.PlotComboBox->currentItem();
      status = os.StatusComboBox->currentItem();
      if(status==0)
        status=-1;
      switch(plot){
        case 0: PlotModuleGraphSummary(5 ,0, "avg. threshold", status);
                        break;
        case 1: PlotModuleGraphSummary(5 ,1, "threshold dispersion", status);
                            break;
        case 2: PlotModuleGraphSummary(5 ,2, "avg. noise", status);
                            break;
                case 3: PlotModuleGraphSummary(5 ,3, "analog good pixels", status);
                            break;
                case 4: PlotModuleGraphSummary(1 ,4, "good bumps (HV off thresh. scan)", status);
                            break;
                case 5: PlotModuleGraphSummary(0, 4, "digital scan (No. good pixels)", status);
                            break;
                default:break;
          }
        }
    else if(os.SystestChkBox->isChecked()){
          plot = os.AvailableComboBox->currentItem();
      if(os.ScanLabel->text() == "digital scan")
        PlotSystestSummary(4, "Digital scan info", plot);
      if(os.ScanLabel->text() == "analog scan"){
        switch(os.OptionsComboBox->currentItem()){
        case 0: PlotSystestSummary(0, "avg. threshold", plot);
                break;
        case 1: PlotSystestSummary(1, "threshold dispersion", plot);
                break;
        case 2: PlotSystestSummary(2, "avg. noise", plot);
                break;
        case 3: PlotSystestSummary(3, "analog good pixels", plot);
                break;
		case 4: //get differences!
        default:break;
        }
       }
     }
  }
}
//**************************FUNCTIONs by JW ****************************************************
void TopWin::PlotMultiProjection(int type, bool overlay)         
{
  ModItem *itarr[20],*item, *child, *temp;
  //  ModItem *module, *subchild;
  int j=0, error;
  //  int i=0;
  //  bool found=false;
  //  int pixel_type;        // 0=ALL, 1=NORMAL, 2=GANGED, 3=INTERGANGED, 4=LONG
  std::string oldname, tempname;

  std::vector<ModItem*> allMods = getAllModuleItems();
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    item = *IT;
    tempname = item->text(0).latin1();
    if(item->isSelected() && item->parent() == 0)        //Module selected->loop through all children
      {
        child = (ModItem*)item->firstChild();
        while(child!=0){
          temp = child->findPlotItem(type);
          if(temp != NULL){
            itarr[j] = temp;
            j++;
          }
          child = (ModItem*)child->nextSibling();
        }
      }
    else
      {
        child = (ModItem*)item->firstChild();
        while(child != 0)
          {
            if(child->isSelected()){
              temp = child->findPlotItem(type);
              if(temp != NULL){
                itarr[j] = temp;
                j++;
              }
            }
            child = (ModItem*)child->nextSibling();
          }
      }
  }
  if(j>0){
    error = rootmain->PlotMultipleProjections(itarr, j, overlay);        // hier muss noch der Pixeltyp übergeben werden
    QString tmp;
    tmp.sprintf("Error plotting: %s",RMain::ErrorToMsg(error).c_str());
    if(options->m_Opts->m_showwarn && error<0) QMessageBox::warning(this,"Module Analysis",tmp);
  } else
    QMessageBox::warning(this,"MAF","No items selected");
}

void TopWin::PlotModuleGraphSummary(int scantype, int sumtype, QString label, int meastype, bool )
{
  ModItem *child;
//  std::map<double, std::vector<ModItem*> > itemmap;
  int parentid;

  delete rootmain->m_currmap;
  rootmain->m_currmap = 0;

  std::vector<ModItem*> allMods = getAllModuleItems();
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    child = (ModItem*) ((*IT)->firstChild());
    while(child != 0){
          if(child->ModParent()->parent() != 0)
            parentid=atoi(child->ModParent()->parent()->text(0));        //stave-id
          else
                parentid=atoi(child->ModParent()->text(0));                                //module-id

      if((sumtype != -1 && (child->GetDID()->GetStdtest()%100) == scantype
                  &&((child->GetDID()->GetStdtest()/100)==meastype || meastype==-1))
				  || meastype==-1){  //should select the test-status
                m_itemmap[parentid].push_back(child);
        if(child->GetDID()->m_istype==ONEDSCAN || 
           child->GetDID()->m_istype==ONEDFIT)
          delete rootmain->ThreshSumm(child, 0);
        else if(child->GetDID()->m_istype==DIG)
          rootmain->nBadPixs(child);
      }
      child = (ModItem*)child->nextSibling();
    }
  }
  if(m_itemmap.size()==0)//is warning flag set?
          QMessageBox::warning(this, "Test status not found", "No data for selected test status available", QMessageBox::Ok, 
            Qt::NoButton);
  if(m_itemmap.size()>0){
    rootmain->PlotModuleSummary(m_itemmap, sumtype, this, label);                
    m_lastPitem = (m_itemmap.begin()->second)[0];                                                //necessary for ExecuteEvent();
    m_lastPdbID = -1;
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
    rootmain->m_currmap->SetWidget(this);
  } else{
    m_lastPitem = 0;
    m_lastPdbID = -1;
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
  }
  UpdateChipVal();
}

void TopWin::CreateDifferencePlots()
{
  //  ModItem* item;
  std::map<int, std::vector<ModItem*> >::iterator It;
  //  std::vector<ModItem*>::iterator VecIt;

  It=m_itemmap.begin();
/*  for(int i=0; i<ComboIndex;i++)
          ++It;                                                //set Iterator to selected dataset

  for(VecIt=(It->second).begin(); VecIt!=(It->second).end(); ++VecIt){
    int refforwhat = 0;
    if(datit->GetDID()->m_istype==TWODSCAN || datit->GetDID()->m_istype==TWODFIT)
      refforwhat = 7;
    RefWin rdw(datit, this,"Reference Data Set", TRUE, Qt::WType_TopLevel, refforwhat);
    if(rdw.DatSel->count()<=0)
     textwin->setText("Load more than this sfit-data set to enable options");
    else{
      // open selection window and get reference data set
      if(rdw.exec()==1){
        if(rdw.selit==0)
          textwin->setText("selit-pointer is 0");
        else{
          tmp = "Plot " + rdw.FuncSel->currentText();
          if(refforwhat==0)
            plotit = new ModItem(datit,tmp, item->GetDID(),
                                 rdw.func, rdw.selit->GetDID());
          else
            plotit = new ModItem(item->Parent(),tmp, item->GetDID(),
                                 rdw.func, rdw.selit->GetDID());
        }
      }
    }
//    ModuleList_itemselected(*VecIt);					//rufe Referenz-Fenster auf!
  }*/
}

void TopWin::PlotSystestSummary(int sumtype, QString Plot_label, int ComboIndex)
{
  QString label;
  int parentid;
  std::map<int, std::vector<ModItem*> > selected_map;
  std::map<int, std::vector<ModItem*> >::iterator It;
  std::vector<ModItem*>::iterator VecIt;

  It=m_itemmap.begin();
  for(int i=0; i<ComboIndex;i++)
          ++It;                                                //set Iterator to selected dataset

  for(VecIt=(It->second).begin(); VecIt!=(It->second).end(); ++VecIt){
    if((*VecIt)->ModParent()->parent() != 0)
      parentid=atoi((*VecIt)->ModParent()->parent()->text(0));        //stave-id
    else
      parentid=9999;						                              //fake stave-id 
	selected_map[parentid].push_back((*VecIt));
  }
  rootmain->PlotModuleSummary(selected_map, sumtype, this, Plot_label);
 
  m_lastPitem = (m_itemmap.begin()->second)[0];
  m_lastPdbID = -1;
  m_lastPdbFPars.clear();
  m_lastPdbFID = -1;
  rootmain->m_currmap->SetWidget(this);
  UpdateChipVal();
}

void TopWin::DateFromSTLog(const char *log_comments, QString &theDate, QString &theTime){
  QStringList date = QStringList::split("\n",log_comments,TRUE);
  date = date.grep("Scan saved");
  QStringList::Iterator it = date.end(); --it;
  date = date.split(' ',*it,FALSE);
  it = date.end(); --it;--it;
  theTime = *it;
  --it; --it;
  theDate = *it+"-";
  ++it;
  theDate = theDate+*it+"-";
  it = date.end();--it;
  theDate = theDate+*it;
}

void TopWin::GetLabelmap()
{
  ModItem *item;
  std::vector<ModItem*> allMods = getAllModuleItems();
  QString ItemLabel;

  if(m_labelmap.size()!=0) m_labelmap.clear();
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    item = (ModItem*) ((*IT)->firstChild());                                                                                //this is definitely a scan item!
        while(item!=0){                                                                                                                                        //loop over all scan items
      m_labelmap[item->text(0)].push_back(item);
          if(item->GetDID()->m_istype==ONEDSCAN || 
           item->GetDID()->m_istype==ONEDFIT)
        delete rootmain->ThreshSumm(item, 0);
      else if(item->GetDID()->m_istype==DIG)
        rootmain->nBadPixs(item);

          item = (ModItem*)item->nextSibling();
        }
  }
  if(m_labelmap.size()==0){
    QMessageBox::warning(this,"No data", "No data found!", QMessageBox::Ok, QMessageBox::NoButton);
    m_lastPitem = 0;
    m_lastPdbID = -1;
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
    return;
  }
}

void TopWin::GetItemmapByTimestamp()
{
  ModItem *item;
  std::vector<ModItem*> allMods = getAllModuleItems();
  QString theDate, theTime;
  int theTimestamp;

  if(m_itemmap.size()!=0) m_itemmap.clear();
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    item = (ModItem*) ((*IT)->firstChild());                                                                                //this is definitely a scan item!
        while(item!=0){                                                                                                                                        //loop over all scan items
//          DateFromSTLog(item->GetDID()->GetLog()->comments.c_str(), theDate, theTime);        //get timestamp
          DateFromLog(item->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
	  //          if(theDate==0 || theTime==0)
	  //                DateFromLog(item->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
          
          theTimestamp = int(BI_getTimeFromStrings(theDate, theTime));
          if(item->GetDID()->m_istype==ONEDSCAN || 
           item->GetDID()->m_istype==ONEDFIT)
        delete rootmain->ThreshSumm(item, 0);
      else if(item->GetDID()->m_istype==DIG)
        rootmain->nBadPixs(item);
          m_itemmap[theTimestamp].push_back(item);                                //map scan items by timestamp

          item = (ModItem*)item->nextSibling();
        }
  }
  if(m_itemmap.size()==0){
          QMessageBox::warning(this,"No data", "No data found!", QMessageBox::Ok, QMessageBox::NoButton);
          m_lastPitem = 0;
	  m_lastPdbID = -1;
	  m_lastPdbFPars.clear();
	  m_lastPdbFID = -1;
          return;
  }
}

void TopWin::UpdateChipVal(){
  int chip=0,binx, biny;
  if(rootmain->m_currmap!=0){
    if(m_lastchip<0) chip = ChipSel->value();
    ChipSel->setValue(chip);
    if(chip < 8){
      biny=1;
      binx=1+chip;
    }else{
      biny=2;
      binx=16-chip;
    }
    QString txtstr;
    txtstr.sprintf("%.3f",rootmain->m_currmap->GetBinContent(binx,biny));
    PixVal->setText(txtstr);
  } else
    PixVal->setText("unknown");
}
//***********************************************************************************************
void TopWin::getModuleRootFromRDB(){
  LoadStave getSN(this," ", TRUE, Qt::WType_TopLevel);
  getSN.snLabel->setText("SN of module");
  getSN.staveSN->setText("20210020");
  getSN.setCaption("Get SN of module to download");
  StdTestWin stdwin(this,"tmptestwin");
  for(int i=1;i<NTYPES;i++){
    getSN.measType->insertItem(stdwin.prefix[i]);
  }  
  getSN.loadRoot->setChecked(true);
  getSN.loadRoot->setText("Retain name of ROOT file as on RDB");
  getSN.rootVarName->setText("M%m/%t/%i/%s_%t%i.root");
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootFixedName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.browseButton, SLOT( setEnabled(bool) ) );
  connect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setDisabled(bool) ) );
  getSN.rootVarName->setEnabled(false);
  getSN.browseButton->setEnabled(true);
  getSN.rootFixedName->setEnabled(true);
  if(getSN.exec()){
    QString fname = getSN.rootFixedName->text();
    if(!getSN.loadRoot->isChecked())
      fname += getSN.rootVarName->text();
    MAEngine::getRootFromRDB(getSN.staveSN->text(), getSN.measType->currentText(),fname, getSN.loadRoot->isChecked(), getApp());
  }
}
void TopWin::getStaveRootFromRDB(){
  LoadStave getSN(this," ", TRUE, Qt::WType_TopLevel);
  getSN.setCaption("Get SN of stave/sector to download");
  StdTestWin stdwin(this,"tmptestwin");
  for(int i=1;i<NTYPES;i++){
    getSN.measType->insertItem(stdwin.prefix[i]);
  }  
  getSN.loadRoot->setChecked(true);
  getSN.loadRoot->setText("Retain name of ROOT file as on RDB");
  getSN.rootVarName->setText("M%m/%t/%i/%s_%t%i.root");
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootFixedName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.browseButton, SLOT( setEnabled(bool) ) );
  connect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setDisabled(bool) ) );
  getSN.rootVarName->setEnabled(false);
  getSN.browseButton->setEnabled(true);
  getSN.rootFixedName->setEnabled(true);
  if(getSN.exec()){
    QString userPDB = options->m_Opts->m_userPDB.c_str();
    QString passwdPDB = options->m_Opts->m_passwdPDB.c_str();
    if(userPDB.isEmpty() || passwdPDB.isEmpty()){
      LoginPanel lp(this,"loginpanel");
      lp.HostLabel->setText("Login details for PDB:");
      lp.HostName->hide();
      lp.UserID->setText("");
      lp.Password->setText("");
      lp.exec();
      userPDB = lp.UserID->text();
      passwdPDB = lp.Password->text();
    }
    MAEngine::getStaveRootFromRDB(getSN.staveSN->text(), userPDB, passwdPDB, getSN.loadRoot->isChecked(), 
                        getSN.measType->currentText(), getSN.rootFixedName->text(), getSN.rootVarName->text(), 
                                  getApp(), this);
    textwin->setText("Finished downloading of stave data.");
    textwin->repaint();
  }
}
void TopWin::getListRootFromRDB(){
  LoadStave getSN(this," ", TRUE, Qt::WType_TopLevel);
  getSN.snLabel->setText("SN of modules");
  getSN.staveSN->setText("");
  getSN.setCaption("Get SN of modules to download");
  StdTestWin stdwin(this,"tmptestwin");
  for(int i=1;i<NTYPES;i++){
    getSN.measType->insertItem(stdwin.prefix[i]);
  }  
  getSN.loadRoot->setChecked(true);
  getSN.loadRoot->setText("Retain name of ROOT file as on RDB");
  getSN.rootVarName->setText("M%m/%t/%i/%s_%t%i.root");
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootFixedName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.browseButton, SLOT( setEnabled(bool) ) );
  connect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setDisabled(bool) ) );
  getSN.rootVarName->setEnabled(false);
  getSN.browseButton->setEnabled(true);
  getSN.rootFixedName->setEnabled(true);
  getSN.browseListButton->show();
  if(getSN.exec()){
    QString fname = getSN.rootFixedName->text();
    if(!getSN.loadRoot->isChecked())
      fname += getSN.rootVarName->text();
    QString modSN = getSN.staveSN->text();
    int pos = modSN.find("; ");
    while(pos>=0){
      textwin->setText("Downloading file for module "+modSN.left(pos));
      textwin->repaint();
      if(m_app!=0)
        m_app->processEvents();      
      MAEngine::getRootFromRDB("20210020"+modSN.left(pos), getSN.measType->currentText(),fname, getSN.loadRoot->isChecked(), getApp());
      modSN.remove(0,pos+2);
      pos = modSN.find("; ");
    }
    textwin->setText("Finished downloading.");
    textwin->repaint();
  }
}
void TopWin::getStaveConfig(){
  LoadStave getSN(this," ", TRUE, Qt::WType_TopLevel);
  getSN.outputName->show();
  getSN.outputLabel->show();
  getSN.browseOutButton->show();
  getSN.outputFile->show();
  getSN.outputFileLabel->show();
  getSN.phaseTable->show();
  getSN.phaseLabel->show();
  getSN.loadPhaseButton->show();
  getSN.okButton->setEnabled(false);
  getSN.setCaption("Get SN of stave/sector");
  StdTestWin stdwin(this,"tmptestwin");
  for(int i=1;i<NTYPES;i++){
    getSN.measType->insertItem(stdwin.prefix[i]);
  }  
  getSN.loadRoot->setChecked(true);
  getSN.loadRoot->setText("Get config from source data");
  getSN.rootVarName->setText("M%m/%t/%i/%s_%t%i.root");
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootFixedName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.rootVarName, SLOT( setEnabled(bool) ) );
  disconnect( getSN.loadRoot, SIGNAL( toggled(bool) ), getSN.browseButton, SLOT( setEnabled(bool) ) );
  getSN.rootVarName->setEnabled(true);
  getSN.browseButton->setEnabled(true);
  getSN.rootFixedName->setEnabled(true);
  if(getSN.exec()){
    QString userPDB = options->m_Opts->m_userPDB.c_str();
    QString passwdPDB = options->m_Opts->m_passwdPDB.c_str();
    if(userPDB.isEmpty() || passwdPDB.isEmpty()){
      LoginPanel lp(this,"loginpanel");
      lp.HostLabel->setText("Login details for PDB:");
      lp.HostName->hide();
      lp.UserID->setText("");
      lp.Password->setText("");
      lp.exec();
      userPDB = lp.UserID->text();
      passwdPDB = lp.Password->text();
    }
    QApplication::setOverrideCursor(Qt::waitCursor);
    CDatabaseCom cdb(userPDB.latin1(),passwdPDB.latin1(),
                     getSN.staveSN->text().latin1());
    std::vector<std::string> modSNs = cdb.getStaveSNs(*m_app);
    int nMod=0;
    if(modSNs.size()==0){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"no modules","No modules found...PDB error or wrong sector SN");
      return;
    }
    for(std::vector<std::string>::iterator it=modSNs.begin(); it!=modSNs.end();it++){
      QString mname  = (*it).c_str();
      QString inPath, indStrg;
      int maxInd = 0;
      for(int ind=1;ind<20;ind++){
        inPath = getSN.rootFixedName->text()+getSN.rootVarName->text();
        inPath.replace("%m",mname);
        inPath.replace("%s","20210020"+mname);
        inPath.replace("%t",getSN.measType->currentText());
        indStrg.sprintf("%02d",ind);
        inPath.replace("%i",indStrg);
        FILE *test = fopen(inPath.latin1(),"r");
        if(test!=0){
          maxInd = ind;
          fclose(test);
        }
      }
      if(maxInd>0){
        inPath = getSN.rootFixedName->text()+getSN.rootVarName->text();
        inPath.replace("%m",mname);
        inPath.replace("%s","20210020"+mname);
        inPath.replace("%t",getSN.measType->currentText());
        indStrg.sprintf("%02d",maxInd);
        inPath.replace("%i",indStrg);
        textwin->setText("Creating config for module " + mname);
        textwin->repaint();
        if(m_app!=0)
          m_app->processEvents();
        int type = 5;
        //        QString cfgName = getSN.measType->currentText()+"Tuned.cfg";
        if(getSN.loadRoot->isChecked()){
          type = 14;
          //cfgName = getSN.measType->currentText()+"Source.cfg";
        }
        QString cfgName="config.cfg", cfgPath=getSN.outputName->text();
        if(!getSN.outputFile->text().isEmpty())
          cfgName = getSN.outputFile->text();
        cfgName.replace("%m",mname);
        cfgName.replace("%s","20210020"+mname);
        int phase1=-999, phase2=-999;
        bool isOK;
        if(getSN.phaseTable->numRows()>nMod){
          phase1 = getSN.phaseTable->text(nMod,0).toInt(&isOK);
          if(!isOK) phase1 = -999;
          phase2 = getSN.phaseTable->text(nMod,1).toInt(&isOK);
          if(!isOK) phase2 = -999;
        }
        int error = MAEngine::WriteConfig(type, inPath.latin1(), cfgPath.latin1(), 
                                          cfgName.latin1(),true, phase1, phase2);
        
        if(error<0){
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this,"cfg error","Error creating config file for module "+mname);
          QApplication::restoreOverrideCursor();
        }
      } else{
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this,"no ROOT file","Can't find ROOT file for module "+mname);
        QApplication::restoreOverrideCursor();
      }
      nMod++;
    }
    textwin->setText("Finished creation of stave cfg data.");
    textwin->repaint();
  }
}
void TopWin::staveMechPDB(){
  MultiLoad ml(this,"multi-load-panel");
  ml.setCaption("Multi-module load - PDB mechanical test");
  ml.textLabel1->setEnabled(false);
  ml.pathToModules->setEnabled(false);
  ml.browseButton->setEnabled(false);
  ml.meastypeBox->setEnabled(false);
  ml.measIndex->setEnabled(false);
  ml.getAmbush->setEnabled(false);
  ml.ambushDir->setEnabled(false);
  ml.savetoRoot->setEnabled(false);
  ml.pathToRoot->setEnabled(false);
  ml.runPDB->setEnabled(false);
  ml.multiPlots->setEnabled(false);
  ml.textLabel1_2->setEnabled(false);
  ml.cutsBox->setEnabled(false);
  ml.uploadBox->setEnabled(false);
  ml.removeAfterPDB->setEnabled(false);

  ModItem *modItem;
  int i;
  if(ml.exec()){
    QString userPDB,passwdPDB;
    userPDB = options->m_Opts->m_userPDB.c_str();
    passwdPDB = options->m_Opts->m_passwdPDB.c_str();
    if(userPDB.isEmpty() || passwdPDB.isEmpty()){
      ml.m_lp->exec();
      userPDB = ml.m_lp->UserID->text();
      passwdPDB = ml.m_lp->Password->text();
    }
    int nRows=ml.moduleTable->numRows();
    for(i=0;i<nRows;i++){
      modItem = ModuleButton_clicked(ml.moduleTable->text(i,0).latin1());
      PDBForm pdb(this,"PDB Form Manager",FALSE,0,modItem,m_app,0);
      pdb.pdbMeasType->setCurrentItem(7);
      pdb.IcheckType();
      pdb.m_User = userPDB;
      pdb.m_Passwd = passwdPDB;
      pdb.exec();
      delete modItem;
    }
  }
}
std::vector<ModItem*> TopWin::getAllModuleItems(){
  std::vector<ModItem*> allMods;
  ModItem *mainIt = (ModItem*)ModuleList->firstChild();
  while(mainIt!=0){
    if(mainIt->GetPID()==MODITEM){
      allMods.push_back(mainIt);
    } else{
      ModItem *subIt = (ModItem*)mainIt->firstChild();
      while(subIt!=0){
        if(subIt->GetPID()==MODITEM){
          allMods.push_back(subIt);
        }
        subIt = (ModItem*) subIt->nextSibling();
      }
    }
    mainIt = (ModItem*) mainIt->nextSibling();
  }
  return allMods;
}
void TopWin::setStavePos(ModItem *item){
  if(item==0) return;
  if(item->GetPID()!=MODITEM || item->Parent()==0) return;
  if(item->Parent()->GetPID()!=STVITEM) return;

  RenameWin rnw(this," ", TRUE, Qt::WType_TopLevel);
  QString oldPos = QString::number(item->GetModID());
  rnw.Name->validateAndSet(oldPos,oldPos.length(),
                           0,oldPos.length());
  rnw.Name->setFocus();
  rnw.EnterLabel->setText("Enter position of " + item->text(0) + " on stave");
  if(rnw.exec()){
    bool isOK;
    int newPos = rnw.Name->text().toInt(&isOK);
    if(isOK && newPos>0 && 
       ((newPos<14&&item->Parent()->text(0).left(1)=="4") || // stave
        (newPos<7&&item->Parent()->text(0).left(1)=="9")))   // sector
       item->SetModPos(newPos);
  }
}
void TopWin::RemFit(ModItem *item, bool do_prompt){
  if(item==0) return;

  bool go_for_it=true;
  // warn user and remove item if confirmed
  if(do_prompt){
    QMessageBox ays("Confirm delete",
                    "This will remove all but summary data from\n\"" + item->text(0) + 
                    "\"\nAre you sure?",
                    QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
                    this, "ays",TRUE,Qt::WType_TopLevel);
  if(ays.exec()!=QMessageBox::Ok)
    go_for_it = false;
  }
  if(go_for_it){
    QApplication::setOverrideCursor(Qt::waitCursor);

    if(item->GetPID()==ONEDSCAN || item->GetPID()==ONEDFIT){
      // produce summary
      delete rootmain->ThreshSumm(item,0);
      // clear full data
      item->GetDID()->ClearFit();
      DataToTree(item->ModParent(),item->GetDID());
      delete item;
    } 
    else if(item->GetPID()==DIG){
      // produce summary
      rootmain->nBadPixs(item);
      // clear full data
      item->GetDID()->ClearFit();
      DataToTree(item->ModParent(),item->GetDID());
      delete item;
    }
  }

  QApplication::restoreOverrideCursor();
  return;
}
ModItem* TopWin::RetrieveOrgData(ModItem *item){
  if(item==0) return 0;
  if(item->GetDID()==0) return 0;

  QString orgPath = item->GetDID()->GetFullPath();
  if(orgPath==""){
    QMessageBox::warning(this,"insuff. info","Can't get information about org. data");
    return 0;
  }

  ModItem *newIt=0;
  // first case: data loaded from TurboDAQ file
  if(item->GetDID()->GetLog()!=0 && item->GetDID()->GetLog()->MAFVersion=="none"
     && strcmp(item->GetDID()->GetPath(),"RootDB file")!=0){ 
    orgPath += item->GetDID()->GetPath();
    m_fwin->SetType(m_fwin->GuessType(orgPath));
    int type = m_fwin->m_seltype;
    newIt = LoadData(item->ModParent(),orgPath.latin1(),item->text(0),type);
  }else if(item->GetDID()->GetLog()!=0 && item->GetDID()->GetLog()->MAFVersion=="none"
	   && strcmp(item->GetDID()->GetPath(),"RootDB file")==0){ // RootDB file
#ifdef HAVE_PLDB
    //    printf("loading RootDB file %s\n",orgPath.latin1());
    QString subFolder = orgPath;
    int pos = orgPath.find("root:/");
    if(pos>=0){
      subFolder.remove(0,pos+6);
      orgPath.remove(pos+4,orgPath.length()-pos-4);
      FILE *testFile = fopen(orgPath.latin1(),"r");
      if(testFile==0){
        QMessageBox::warning(this,"insuff. info","Can't open file related to org. data");
        return 0;
      } else
        fclose(testFile);
      QString scanName, grpName;
      int spos = subFolder.find("/");
      if(spos>=0){
	scanName = subFolder.left(spos);
	grpName  = subFolder.right(subFolder.length()-spos-1);
	//	printf("%s - %s \n",scanName.latin1(), grpName.latin1());
	m_memdir->cd(); // make sure MA histos are created in memory
	QApplication::setOverrideCursor(Qt::waitCursor);
	DatSet *newData = MAEngine::findPixScan(orgPath.latin1(), scanName.latin1(), grpName.latin1(), item->ModParent()->text(0).latin1());
	if(newData!=0){
	  //	  m_pixScans.push_back(newData->getPixScan());
	  newIt = DataToTree(item->ModParent(), newData);
	}else{
	  QApplication::restoreOverrideCursor();
	  QMessageBox::warning(this,"insuff. info","Error retrieving org. data");
	  return 0;
	}  
	QApplication::restoreOverrideCursor();
      } else{
	QMessageBox::warning(this,"insuff. info","Can't open file related to org. data");
	return 0;
      }
    } else{
      QMessageBox::warning(this,"insuff. info","Can't open file related to org. data");
      return 0;
    }
#else
    QMessageBox::warning(this,"nopixlib","This version comes without RootDB, "
			 "reading in this format not possible");
#endif
  }else if(item->GetDID()->GetLog()!=0){ // so must be from a ROOT file
    //    printf("loading MA Root file %s\n",orgPath.latin1());
    QString subFolder = orgPath;
    int pos = orgPath.find("root:/");
    if(pos>=0){
      orgPath.remove(pos+4,orgPath.length()-pos-4);
      subFolder.remove(0,pos+6);
      FILE *testFile = fopen(orgPath.latin1(),"r");
      if(testFile==0){
        QMessageBox::warning(this,"insuff. info","Can't open file related to org. data");
        return 0;
      } else
        fclose(testFile);
      QString basePath = gDirectory->GetPath();
      TFile *f = new TFile(orgPath.latin1());
      if(f==0)
        return 0;
      f->cd(subFolder.latin1());
      int rderr;
      DatSet *newData = new DatSet(basePath.latin1(),&rderr);
      f->Close();
      newIt = DataToTree(item->ModParent(), newData);
    } else
      QMessageBox::warning(this,"insuff. info","Can't open file related to org. data");
  } else { // no idea what to do
    QMessageBox::warning(this,"insuff. info","Can't get information about org. data");
    return 0;
  }
  int stdType = item->GetDID()->GetStdtest();
  // check if original data was referred to and if so replace references
  std::vector<ModItem*> refItems = item->checkForRefs();
  for(std::vector<ModItem*>::iterator IT=refItems.begin(); IT!=refItems.end(); IT++){
    DatSet* array[NREFS];
    int nRefElms=0;
    for(int kk=0;kk<NREFS;kk++){
      if((*IT)->GetRID(kk)!=0){
        nRefElms++;
        if((*IT)->GetRID(kk)==item->GetDID())
          array[kk] = newIt->GetDID();
        else
          array[kk] = (*IT)->GetRID(kk);
      }
    }
    (*IT)->SetRefArr(array, nRefElms);
  }
  RemData(item, false);
  newIt->GetDID()->SetStdtest(stdType);

  // if approp.: check if other standard items are around and reprocess std. item accordingly
  if(stdType>=0){
    StdTestWin tmpstd(this,"tmpstd",0,0,newIt->ModParent());
    std::vector<ModItem*> stdArr;
    stdArr.resize(tmpstd.NSLOTS,0);
    std::vector<int> exists;
    exists.resize(tmpstd.NSLOTS,0);
    int itemId, thisType = newIt->GetDID()->GetStdtest()/100;
    ModItem *getStd = (ModItem*) newIt->ModParent()->firstChild();
    while(getStd!=0){
      if((itemId=getStd->GetDID()->GetStdtest()%100)>=0 && 
	 itemId < tmpstd.NSLOTS &&                               // std. ID is within range
	 (getStd->GetDID()->GetStdtest()/100)==thisType &&       // same meas. type as this data
	 tmpstd.m_items[itemId]->m_type==getStd->GetDID()->m_istype){ // is of correct data type (i.e. not compressed)
	stdArr[itemId] = getStd;                                 // -> load!
	exists[itemId] = 1;
      }
      getStd = (ModItem*) getStd->nextSibling();
    }
    stdArr[stdType] = newIt;
    ProcStd(stdArr,exists);
  }

  return newIt;
}
void TopWin::ClearAllFits(ModItem *in_item, bool do_prompt){
  ModItem *ditem;
  std::vector<ModItem*> dataIt;

  if(in_item==0){
    std::vector<ModItem*> allMods = getAllModuleItems();
    for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
      ditem = (ModItem*) (*IT)->firstChild();
      while(ditem!=0){
        dataIt.push_back(ditem);
        ditem = (ModItem*) ditem->nextSibling();
      }
    }
  } else if(in_item->GetPID()==MODITEM){
    ditem = (ModItem*) in_item->firstChild();
    while(ditem!=0){
      dataIt.push_back(ditem);
      ditem = (ModItem*) ditem->nextSibling();
    }
  }
  
  if(dataIt.size()==0){
    textwin->setText("Can't find any data for summaries");
    return;
  }
  
  bool go_for_it=true;
  // warn user and remove item if confirmed
  if(do_prompt){
    QString prmp_txt = "This will remove all but summary data from all modules\nAre you sure?";
    if(in_item!=0)
      prmp_txt = "This will remove all but summary data\nfrom module "+in_item->text(0)+"\nAre you sure?";
    QMessageBox ays("Confirm delete", prmp_txt,
                    QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
                    this, "ays",TRUE,Qt::WType_TopLevel);
    if(ays.exec()!=QMessageBox::Ok)
      go_for_it = false;
  }

  if(go_for_it){
    for(std::vector<ModItem*>::iterator IT=dataIt.begin(); IT!=dataIt.end();IT++){
      ditem = *IT;
      textwin->setText("Compressing data "+ditem->text(0)+" from "+
                       ditem->ModParent()->text(0));
      textwin->repaint();
      if(m_app!=0)
        m_app->processEvents();
      RemFit(ditem,false);
    }
  }
  textwin->setText("done");
  return;
}
ModItem* TopWin::getSimilarData(ModItem *module, int stdType){
  if(module==0) return 0;
  ModItem *child = (ModItem*)module->firstChild();
  while(child!=0){
    if(child->GetDID()!=0 && (child->GetDID()->GetStdtest()%100)==(stdType%100))
      return child;
    child = (ModItem*)child->nextSibling();
  }
  return 0;
}
void TopWin::assignToStave(ModItem *module){
  if(module==0) return;
  RefWin selStave(0,this,"selst",0,0,100);
  selStave.FuncSel->hide();
  selStave.FuncselLabel->hide();
  selStave.setCaption("select stave to assign module "+module->text(0)+" to");
  selStave.SelectorLabel->setText("select stave");
  disconnect( selStave.OKButt, SIGNAL( clicked() ), &selStave, SLOT( RDAccept() ) );
  connect( selStave.OKButt, SIGNAL( clicked() ), &selStave, SLOT( accept() ) );

  std::vector<ModItem*> staveList;
  ModItem *stItem = (ModItem*) ModuleList->firstChild();
  while(stItem!=0){
    if(stItem->GetPID()==STVITEM){
      staveList.push_back(stItem);
      selStave.DatSel->insertItem(stItem->text(0));
    }
    stItem = (ModItem*) stItem->nextSibling();
  }
  if(selStave.exec()==QDialog::Accepted){
    int id = selStave.DatSel->currentItem();
    if(id>=0 && id<(int)staveList.size())
      assignToStave(module,staveList[id]);
  }
  return;
}
void TopWin::assignToStave(ModItem *module, ModItem *stave){
  if(module==0 || stave==0) return;
  Q3ListViewItem *item = (Q3ListViewItem*) module;
  if(item->parent()!=0)
    item->parent()->takeItem(item);
  else
    ModuleList->takeItem(item);
  stave->insertItem(item);
  return;
}
void TopWin::CheckToTsrc(){
  QString path = Q3FileDialog::getExistingDirectory(QString::null,this, "getdir", "Get directory where modules data are");
  if(path==QString::null) return;
  BadPixel *badpix = new BadPixel(0,0,"badpix",FALSE,Qt::WDestructiveClose);
  badpix->setCaption("ToT(source) summary - retrieving data");
  badpix->Init("...","ToT(source) summary");
  // do non-standard settings by hand
  badpix->PixelList->setNumCols(1);
  badpix->PixelList->setNumRows(0);
  badpix->PixelList->horizontalHeader()->setLabel(0,"Module");
  for(int jj=0;jj<20;jj++){
    std::string label=MAEngine::getToTCheckLabel(jj);
    if(label=="value not used") break;
    badpix->PixelList->setNumCols(jj+2);
    badpix->PixelList->horizontalHeader()->setLabel(jj+1,label.c_str());
  }
  badpix->MaskButt->hide();
  badpix->CloseButt->setEnabled(false);
  badpix->WrtieButt->setEnabled(false);
  badpix->show();

  QPixmap pm(10,10);
  pm.fill(Qt::red);

  QDir mydir(path, "20210020*");
  QStringList mainlist = mydir.entryList();
  for(QStringList::Iterator it=mainlist.begin(); it!=mainlist.end(); it++){
    mydir.cd(*it);
    if(mydir.cd("FLEX")){
      mydir.setNameFilter("0*");
      QStringList indlist = mydir.entryList();
      int maxind=-1;
      for(QStringList::Iterator iit=indlist.begin(); iit!=indlist.end(); iit++){
        bool isOK;
        int tmpind = (*iit).toInt(&isOK);
        if(tmpind>maxind) maxind=tmpind;
      }
      if(maxind>=0){
        QString sind;
        sind.sprintf("%02d",maxind);
        mydir.cd(sind);
        QString fname=(*it)+"_FLEX"+sind+".root";
        FILE *test=fopen((mydir.path()+"/"+fname).latin1(),"r");
        if(test==0){
          QString short_SN=(*it).right(6);
          fname=short_SN+"_FLEX"+sind+".root";
          test=fopen((mydir.path()+"/"+fname).latin1(),"r");
        }
        if(test!=0){
          fclose(test);
          DatSet *data = MAEngine::LoadStdFromRoot(14, (mydir.path()+"/"+fname).latin1());
          if(data!=0){
            std::vector<float> result;
            MAEngine::checkToT(data,result);
            int nRows = badpix->PixelList->numRows();
            badpix->PixelList->setNumRows(nRows+1);
            badpix->PixelList->setText(nRows,0,data->GetLog()->MyModule->modname.c_str());
            for(unsigned int kk=0;kk<result.size();kk++)
              badpix->PixelList->setText(nRows,kk+1, QString::number(result[kk],'f',2));
            if(result[options->m_Opts->m_totcheckVar]>options->m_Opts->m_totcheckValue){
              badpix->PixelList->setPixmap(nRows,0,pm);
              if(options->m_Opts->m_totcheckKeep){
                // create module if necessary
                ModItem *mod = 0;
                std::vector<ModItem*> allMods = getAllModuleItems();
                for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
                  if((*IT)->text(0)==data->GetLog()->MyModule->modname.c_str())
                    mod = *IT;
                }
                if(mod==0)
                  mod = ModuleButton_clicked(data->GetLog()->MyModule->modname.c_str(),0);
                DataToTree(mod, data);
              } else
                delete data;
            } else
              delete data;
            if(m_app!=0)
              m_app->processEvents();
          } else if(options->m_Opts->m_showwarn) 
	    QMessageBox::warning(this,"MA TopWin::CheckToTsrc","File "+fname+
				 " seems to be corrupt, can't load.");
        }
        mydir.cdUp();
      }
      mydir.cdUp();
    }
    mydir.cdUp();
  }
  badpix->setCaption("ToT(source) summary - complete");
  badpix->CloseButt->setEnabled(true);
  badpix->WrtieButt->setEnabled(true);
}
void TopWin::plotDBscan(ModItem *item, bool plot_func, bool pltAvg)
{
  // remember last PID and DID
  int error, my_chip = -1;
  if(ChipOrMod->isOn())
    my_chip = ChipSel->value();
  rootmain->m_chip = ChipSel->value();
  rootmain->m_col  = ColSel->value();
  rootmain->m_row  = RowSel->value();
  int ptarr[3]={ Scanpt0->value(), Scanpt1->value(), Scanpt2->value()};
  
  m_lastPitem = item;
  m_lastPdbID = (int)plot_func;
  m_lastchip  = my_chip;
  rootmain->ClearMemory();
  if(rootmain->m_currmap!=0) rootmain->m_currmap->Delete();
  rootmain->m_currmap = 0;

  int fid = -1;
  std::vector<int> pars;
  if(plot_func){
    if(m_lastPdbFPars.size()==0){
      PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
      if(pdbd==0) return;
      ParFitfun pff(this, "pff", false, 0, pdbd);
      if(pff.exec()!=QDialog::Accepted) return;
      fid = pff.getPars(pars);
      m_lastPdbFPars = pars;
      m_lastPdbFID = fid;
    } else{
      pars = m_lastPdbFPars;
      fid = m_lastPdbFID;
    }
  } else{
    m_lastPdbFPars.clear();
    m_lastPdbFID = -1;
  }
  
  QApplication::setOverrideCursor(Qt::waitCursor);
  if((error = rootmain->plotPS_ValVSPar(pars,item, pixScanLevel->value(), ptarr, my_chip, fid, pltAvg))){
    QApplication::restoreOverrideCursor();
    if(options->m_Opts->m_showwarn) 
      QMessageBox::warning(this,"Module Analysis",
			   ("Error plotting: "+RMain::ErrorToMsg(error)).c_str());
  } else
    QApplication::restoreOverrideCursor();
  // store this object in current map for reference
  // and update bin content indicator
  if(rootmain->m_currmap!=0){
    rootmain->m_currmap->SetWidget(this);
    rootmain->m_currmap->m_dodclk=true;
  }
  UpdatePixVal();
  return;
}

void TopWin::LoadMultiModuleRDB()
{
  MultiLoadRDB ml(this,"ml", false, 0, options->m_Opts->m_DefDataPath.c_str());
  ml.plotsPath->setText(options->m_Opts->m_pdbpath.c_str());
  if(ml.exec()==QDialog::Accepted){
    rootmain->CheckCan(); // somehow neccessary if lots of data loaded - memory problem?
    // get selected scans and modules
    std::vector<std::string> mods;
    std::map<std::string, int> scans;
    ml.getMods(mods);
    ml.getScans(scans);
    Q3Table *qualitab = 0;
    BadPixel *bp = 0;
    if(ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
      // prepare table output
      bp = new BadPixel(0,0,"summ_tab_multi",FALSE,Qt::WDestructiveClose);
      bp->setCaption("Summary");
      bp->resize( 600, 450);
      bp->Headline->setText("");
      qualitab = bp->PixelList;
      qualitab->setNumRows(10);
      qualitab->setNumCols(0);
      // write headers etc
      qualitab->verticalHeader()->setLabel(0 ,"Test Date");
      qualitab->verticalHeader()->setLabel(1 ,"Total bad pixel");
      qualitab->verticalHeader()->setLabel(2 ,"Average noise");
      qualitab->verticalHeader()->setLabel(3 ,"Average threshold");
      qualitab->verticalHeader()->setLabel(4 ,"Threshold dispersion");
      qualitab->verticalHeader()->setLabel(5 ,"Average in-time thresh.");
      qualitab->verticalHeader()->setLabel(6 ,"In-time thr. dispersion");
      qualitab->verticalHeader()->setLabel(7 ,"Average ToT(mip)");
      qualitab->verticalHeader()->setLabel(8 ,"ToT(mip) dispersion");
      qualitab->verticalHeader()->setLabel(9 ,"PASS/FAIL");
    }
    // loop over modules (dummy for now)
    for(std::vector<std::string>::iterator IT=mods.begin(); IT!=mods.end(); IT++){
      //printf("LoadMultiModuleRDB(): module %s\n",(*IT).c_str());
      // open RootDB files
      PixLib::RootDB *file = 0;
      try{
	file = new PixLib::RootDB(ml.fileName->text().latin1());
      }catch(...){
	file = 0;
      }
      if(file==0){
	QMessageBox::warning(this,"ModuleAnalysis Error","Error opening file "+ml.fileName->text());
	return;
      }
      PixLib::DBInquire *root = 0;
      try{
	root = file->readRootRecord(1);
      } catch(...){
	root = 0;
      }
      if(root==0){
	QMessageBox::warning(this,"ModuleAnalysis Error","Error reading file "+ml.fileName->text());
	return;
      }
      // get scan data
      std::vector<ModItem*> data;
      int di=-1;
      for(std::map<std::string, int>::iterator IT3=scans.begin(); IT3!=scans.end(); IT3++){
	//printf("LoadMultiModuleRDB(): scan %s %d\n",IT3->first.c_str(), IT3->second);
	// get inquire for that scan
	PixLib::recordIterator scan = root->findRecord(IT3->first+"/PixScanResult");
	if(scan!=root->recordEnd()){
	  di = data.size(); // remember old starting point
	  loadPixScanData(data, (*scan), false, false, ml.fileName->text().latin1(), (*IT).c_str());
	  for(; di<(int)data.size(); di++){
	    DatSet *datset = data[di]->GetDID();
	    if(datset!=0)
	      datset->SetStdtest(11*100+IT3->second);
	  }
	}
      }
      // close file
      delete file;
      // run analysis if possible and requested
      if(data.size()>0 && ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
	ModItem *mi = data[0]->ModParent();
	qualitab->setNumCols(qualitab->numCols()+1);
	qualitab->horizontalHeader()->setLabel( qualitab->numCols()-1, mi->text(0));
	QString theDate, theTime;
	DateFromLog(data[0]->GetDID()->GetLog()->comments.c_str(), theDate, theTime);
	qualitab->setText(0 , qualitab->numCols()-1, theDate);
	// run analysis
	if(ml.plotsPath->text()==""){ // must have output path -> force user to enter
	}
	// create and execute QA report via GUI window
	QAReport qar(mi, this, "qar", false, 0, ml.plotsPath->text().latin1(), m_app);
	// disable buttons - the user is not supposed to use them
	qar.BrowseButton->setEnabled(false);
	qar.StartButton->setEnabled(false);
	qar.CloseButt->setEnabled(false);
	qar.show();
	qar.StartReport();
	// store test report text
	QString rtxt = qar.ReportText->text();
	if(ml.removeAfterPDB->isChecked() && ml.removeAfterPDB->isEnabled()){
	  // remove links to plots - links won't work if data is deleted
	  rtxt.replace(QRegExp("</a>"),"");
	  int pos=0;
	  while(pos>=0){
	    pos = rtxt.find("<a href");
	    if(pos>=0){
	      int other_end = rtxt.find("\">",pos+1);
	      if(other_end>=0)
		rtxt.remove(pos,other_end+2-pos);
	      else
		pos = -1;
	    }
	  }
	}
	m_reports.push_back(rtxt);
	// retrieve information from analysis
	qualitab->setText(1 , qualitab->numCols()-1, QString::number(qar.m_nBadPix));
	qualitab->setText(2 , qualitab->numCols()-1, QString::number(qar.m_avgNse, 'f',1));
	qualitab->setText(3 , qualitab->numCols()-1, QString::number(qar.m_avgThr, 'f',1));
	qualitab->setText(4 , qualitab->numCols()-1, QString::number(qar.m_sdThr, 'f',1));
	qualitab->setText(5 , qualitab->numCols()-1, QString::number(qar.m_avgITThresh, 'f',1));
	qualitab->setText(6 , qualitab->numCols()-1, QString::number(qar.m_sdITThresh, 'f',1));
	qualitab->setText(7 , qualitab->numCols()-1, QString::number(qar.m_avgTOT, 'f',1));
	qualitab->setText(8 , qualitab->numCols()-1, QString::number(qar.m_sdTOT, 'f',1));
	qualitab->setText(9 , qualitab->numCols()-1, (qar.m_all_OK)?"PASS":"FAIL");
 	// remove data if requested
	if(ml.removeAfterPDB->isChecked() && ml.removeAfterPDB->isEnabled())
	  RemoveMod(mi, false);
      }
    }
    if(ml.runPDB->isChecked() && ml.runPDB->isEnabled()){
      connect( qualitab, SIGNAL( clicked(int,int,int,const QPoint&) ),
	       this, SLOT( qTableClicked(int,int, int) ) );
      m_endRow = qualitab->numRows()-1;
      bp->exec();
    }
  }
}
void TopWin::showReport(int irep)
{
  if(irep>=(int)m_reports.size()) return;

  QAReport qar(0, this, "qar");
  // disable buttons - the user is not supposed to use them
  qar.BrowseButton->setEnabled(false);
  qar.StartButton->setEnabled(false);
  qar.TabWidget->setTabEnabled(qar.TabWidget->page(1), false);
  qar.ReportText->setText(m_reports[irep]);
  qar.exec();
    
}
void TopWin::qTableClicked( int row, int col, int button)
{
  if(button==2 && row==m_endRow) // right mouse click on last row only
    showReport(col);
}
bool TopWin::checkDBLinkExists(const char *name, const char *path, const char *modName){
  ModItem *modit, *datit;
  std::vector<ModItem*> allMods = getAllModuleItems();
  for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
    modit = *IT;
    datit = (ModItem*) modit->firstChild();
    while(datit!=0){
      PixDBData *pdd = dynamic_cast<PixDBData*>(datit->GetDataItem());
      if(pdd!=0 && std::string(pdd->GetName())==std::string(name) && 
	 std::string(pdd->GetFullPath())==std::string(path) && 
	 std::string(pdd->GetModName())==std::string(modName))	return true;
      datit = (ModItem *) datit->nextSibling();
    }
  }
  return false;
}
void TopWin::setScaptBoxes(int loop_level){
  Scanpt0->setEnabled(loop_level>=0 && loop_level<1);
  Scanpt1->setEnabled(loop_level>=0 && loop_level<2);
  Scanpt2->setEnabled(loop_level>=0 && loop_level<3);
}
