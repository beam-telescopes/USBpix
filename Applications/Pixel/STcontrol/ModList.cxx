#include "ModList.h"
#include "STControlEngine.h"
#include "STRodCrate.h"
#include "STPixModuleGroup.h"
#include "ConfigEdit.h"
#include "ConfigEditI4.h"
#include "USBCtrlEdit.h"
#include "GroupEditor.h"
#include "OptionsPanel.h"

#ifndef NOTDAQ
#include "RodEditor.h"
#endif

#include <PixDcs/SleepWrapped.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixDcs/PixDcs.h>
#include <GeneralDBfunctions.h>

#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QLayout>
#include <QAction>
#include <QTabWidget>
#include <QPoint>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTime>
#include <QTimer>
#include <QDateTime>

#include <string>
#include <sstream>
#include <math.h>

#ifdef WIN32
#include <stdlib.h>
#endif

using namespace PixLib;
using namespace SctPixelRod;

ModListItem::ModListItem(STControlEngine &engine_in, QTreeWidget *parent, const char *name) :
  QTreeWidgetItem(parent) , m_engine(engine_in)
{
  setText(0,name);
  setText(1," ");
  setText(2,"");
  m_type = tnone;
  m_crateId = -1;
  m_grpId = -1;
  m_modId = -1;
  m_enabled = true;
}
ModListItem::ModListItem(STControlEngine &engine_in, QTreeWidgetItem *parent, ContentType type_in, int id_in) :
  QTreeWidgetItem(parent), m_engine(engine_in)
{
  setText(0,"new module");
  setText(1,"untested");
  setText(2,"");
  m_type = type_in;;
  m_enabled = true;
  m_crateId = -1;
  m_grpId = -1;
  m_modId = -1;
  ModListItem *mlp = dynamic_cast<ModListItem*>(parent);
  switch(m_type){
  case tmodule:{
    if(mlp!=0){
      m_crateId = mlp->crateId();
      m_grpId = mlp->grpId();
    }
    m_modId = id_in;
    if(m_crateId<0 || m_grpId<0 || m_modId<0) return;
    if(getCrate()==0) return;
    // write index of module
    setText(3,QString::number(m_modId));
    // get the real name of this module
    std::string actname;
    try{
      actname = getCrate()->getModName(m_grpId, m_modId);
    } catch(...){
      std::cerr << "error getting name" << std::endl;
    }
    PixLib::getDecNameCore(actname);
    if(actname!="")
      setText(0,actname.c_str());
    // check module config if module is active 
    try{
      Config &conf = getCrate()->getPixModuleConf(m_grpId, m_modId);
      if(conf["general"].name()!="__TrashConfGroup__" &&
	 conf["general"]["Active"].name()!="__TrashConfObj__"){
	if(((ConfList&)conf["general"]["Active"]).sValue()=="TRUE"){
	  setText(2,"enabled");
	  m_enabled = true;
	} else{
	  setText(2,"disabled");
	  m_enabled = false;
	}
      }
    }catch(...){
      std::cerr << "error getting active status" << std::endl;
    }
    break;}
  case tgroup:
    if(mlp!=0){
      m_crateId = mlp->crateId();
    }
    m_grpId = id_in;
    if(m_crateId<0 || m_grpId<0) return;
    if(getCrate()==0) return;
    setText(0,getCrate()->getGrpName(m_grpId).c_str());
    setText(1," ");
    setText(2,"");
    // write index of module
    setText(3,QString::number(m_grpId));
    break;
  case tROD:
    if(mlp!=0){
      m_crateId = mlp->crateId();
      m_grpId = mlp->grpId();
    }
    setText(0,"ROD");
    setText(1,"uninit.");
    setText(2,"");
    setText(3,"");
    break;
  case tBOC:
    if(mlp!=0){
      m_crateId = mlp->crateId();
      m_grpId = mlp->grpId();
    }
    setText(0,"BOC");
    setText(1,"uninit.");
    setText(2,"");
    setText(3,"");
    break;
  case tUSB:
    if(mlp!=0){
      m_crateId = mlp->crateId();
      m_grpId = mlp->grpId();
    }
    setText(0,"USB-controller");
    setText(1,"uninit.");
    setText(2,"");
    setText(3,"");
    break;
  case tCTRL:
    if(mlp!=0){
      m_crateId = mlp->crateId();
      m_grpId = mlp->grpId();
    }
    setText(0,"Controller");
    setText(1,"uninit.");
    setText(2,"");
    setText(3,"");
    break;
  case tDCSC:
    setText(2,"---");
    setText(3,"---");
    setText(4,"---");
    setText(5,"---");
    setText(6,"---");
  case tDCS:
    if(mlp!=0) m_crateId = mlp->crateId();
    m_grpId = id_in;
    if(m_crateId<0 || m_grpId<0) return;
    if(getCrate()==0) return;
    setText(0,"PixDcsChan");
    setText(1,"uninit.");
    setText(7,QString::number(m_grpId));
    break;
  default:
    break;
  }
}
ModListItem::ModListItem(STControlEngine &engine_in, QTreeWidget *parent, STRodCrate *crate) :
  QTreeWidgetItem(parent) , m_engine(engine_in)
{
  QString baseName;
  switch(m_engine.whichIFtype()){
  case tRCCVME:
    baseName="RCC";
    break;
  case tUSBSys:
    baseName="USB";
    break;
  default:
    baseName="---";
  }
  m_type = tcrate;;
  m_enabled = true;
  m_grpId = -1;
  m_modId = -1;
  m_crateId = -1;
  if(crate==0) return;
  m_crateId = crate->getID();
  setText(0,crate->getName().c_str());
  if(baseName!="---")
    setText(1,crate->vmeOK()?(baseName+" OK"):("no "+baseName));
  else
    setText(1,baseName);
  setText(2,"");
}
ModListItem::~ModListItem(){
}
ModListItem* ModListItem::parentGroup(){
  ModListItem *modit, *parnt;
  parnt = dynamic_cast<ModListItem*>(this);
   modit = parnt;
  while(parnt->parent()!=0 && parnt->getType()!=tgroup){
    modit = parnt;                               
    parnt = dynamic_cast<ModListItem*>(modit->parent());    
  }
  if(parnt->getType()==tgroup)
    return parnt;
  else
    return 0;
}
ModListItem* ModListItem::parentCrate(){
  ModListItem *modit, *parnt;
  parnt = dynamic_cast<ModListItem*>(this);
  modit = parnt;
  while(parnt->parent()!=0 && parnt->getType()!=tcrate){
    modit = parnt;                               
    parnt = dynamic_cast<ModListItem*>(modit->parent());    
  }
  if(parnt->getType()==tcrate)
    return parnt;
  else
    return 0;
}

STRodCrate * ModListItem::getCrate()
{
  if(m_crateId>=0 && (int)m_engine.getSTRodCrates().size()>m_crateId)
    return m_engine.getSTRodCrates()[m_crateId];
  else
    return 0;
}
/** Read property of Bool enabled.*/
const bool& ModListItem::isEnabled(){
  return m_enabled;
}
/** Write property of Bool enabled. */
void ModListItem::setEnabled( const bool& _newVal){
  if( _newVal != m_enabled ) {
    m_enabled = _newVal;
    if( m_enabled ) {
      setText(2, "enabled");
    } else {
      setText(2, "disabled");
    }
    if(m_modId>=0 && m_grpId>=0 && parentCrate()!=0 && parentCrate()->getCrate()!=0)
      parentCrate()->getCrate()->setModuleActive(m_grpId, m_modId, m_enabled);
    ModList *ml = dynamic_cast<ModList*>(treeWidget()->parent());
    if(ml!=0)
      ml->emitStatusChanged();
  }
}
/** Toggles the enable flag */
void ModListItem::toggleEnable(){
  if( m_enabled ) {
    setEnabled( false );
  } else {
    setEnabled( true );
  }
}

void ModListItem::up()
{
  if(getType()!=tDCS) return; // action only possible for DCS devices
  int nextID = parent()->indexOfChild(this)-1;
  ModListItem *mli_prev = 0;
  if(nextID>=0) mli_prev = (ModListItem *)parent()->child(nextID);
  if(mli_prev!=0){
    STRodCrate *crt = getCrate();
    if(crt==0) return;

    std::vector<PixDcs*> dcsv = crt->getPixDcs();
    PixDcs *dcs_mine = 0, *dcs_prev=0;
    for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
      if((*dcsi)->getIndex()==grpId()){
	dcs_mine=*dcsi;
      }
      if((*dcsi)->getIndex()==mli_prev->grpId()){
	dcs_prev=*dcsi;
      }
    }
    if(dcs_prev!=0 && dcs_mine!=0){
      // swap indices
      int newId = dcs_prev->getIndex();
      dcs_prev->setIndex(dcs_mine->getIndex());
      mli_prev->m_grpId = dcs_mine->getIndex();
      dcs_mine->setIndex(newId);
      m_grpId = newId;
      // needed to get correct sorting
      mli_prev->setText(7, QString::number(dcs_prev->getIndex()));
      setText(7, QString::number(dcs_mine->getIndex()));
    }
  }
}
void ModListItem::down()
{
  if(getType()!=tDCS) return; // action only possible for DCS devices
  int nextID = parent()->indexOfChild(this)+1;
  ModListItem *mli_next = 0;
  if(nextID< parent()->childCount()) mli_next = (ModListItem *)parent()->child(nextID);
  if(mli_next!=0){
    STRodCrate *crt = getCrate();
    if(crt==0) return;

    std::vector<PixDcs*> dcsv = crt->getPixDcs();
    PixDcs *dcs_mine = 0, *dcs_next=0;
    for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
      if((*dcsi)->getIndex()==grpId()){
	dcs_mine=*dcsi;
      }
      if((*dcsi)->getIndex()==mli_next->grpId()){
	dcs_next=*dcsi;
      }
    }
    if(dcs_next!=0 && dcs_mine!=0){
      // swap indices
      int newId = dcs_next->getIndex();
      dcs_next->setIndex(dcs_mine->getIndex());
      mli_next->m_grpId = dcs_mine->getIndex();
      dcs_mine->setIndex(newId);
      m_grpId = newId;
      // needed to get correct sorting
      mli_next->setText(7, QString::number(dcs_next->getIndex()));
      setText(7, QString::number(dcs_mine->getIndex()));
    }
  }
}

// ModList class starts here

ModList::ModList( STControlEngine &engine_in, QWidget* parent, Qt::WindowFlags fl, MLmode mode) :
  QWidget( parent, fl ), m_engine( engine_in ), m_mode(mode), m_scanBusy(false)
{
  setupUi(this);
  QTreeWidgetItem *twHeader = ModListView->headerItem();
  ModListView->setColumnWidth(0, 180);
  switch(m_mode){
  case DAQList:
    textLabel1->setText("read-out configuration");
    ModListView->setColumnCount(4);
    twHeader->setText(1, "Status");
    twHeader->setText(2, "Enabled?");
    twHeader->setText(3, "Index");
    //twHeader->setClickEnabled(true, ModListView->header()->count() - 1);
    //twHeader->setResizeEnabled(true, ModListView->header()->count() - 1);
    ModListView->sortItems(3,Qt::AscendingOrder); // set ordering by last column (i.e. index)
    turnoffButton->hide();
    turnonButton->hide();
    readButton->hide();
    offReverse->hide();
    noUpdateInScan->hide();
    refrStatus->hide();
    upItemButton->hide();
    downItemButton->hide();
    break;
  case DCSList:{
    noUpdateInScan->setChecked(true);
    offReverse->setChecked(true);
    textLabel1->setText("monitoring configuration");
    ModListView->setColumnCount(8);
    ModListView->setColumnWidth(1, 60);
    ModListView->setColumnWidth(2, 70);
    twHeader->setText(1, "Status");
    twHeader->setText(2, "Enabled?");
    twHeader->setText(3, "Voltage (V)");
    twHeader->setText(4, "Current (A)");
    twHeader->setText(5, "Temp. (C)");
    twHeader->setText(6, "Pos. (a.u.)");
    twHeader->setText(7, "Index");
//     ModListView->addColumn(QApplication::translate("ModListBase", "Voltage (V)", 0, QApplication::UnicodeUTF8));
//     ModListView->header()->setClickEnabled(true, ModListView->header()->count() - 1);
//     ModListView->header()->setResizeEnabled(true, ModListView->header()->count() - 1);
//     ModListView->addColumn(QApplication::translate("ModListBase", "Current (A)", 0, QApplication::UnicodeUTF8));
//     ModListView->header()->setClickEnabled(true, ModListView->header()->count() - 1);
//     ModListView->header()->setResizeEnabled(true, ModListView->header()->count() - 1);
//     ModListView->addColumn(QApplication::translate("ModListBase", "Temp. (C)", 0, QApplication::UnicodeUTF8));
//     ModListView->header()->setClickEnabled(true, ModListView->header()->count() - 1);
//     ModListView->header()->setResizeEnabled(true, ModListView->header()->count() - 1);
//     ModListView->addColumn(QApplication::translate("ModListBase", "Pos. (a.u.)", 0, QApplication::UnicodeUTF8));
//     ModListView->header()->setClickEnabled(true, ModListView->header()->count() - 1);
//     ModListView->header()->setResizeEnabled(true, ModListView->header()->count() - 1);
//     ModListView->addColumn(QApplication::translate("ModListBase", "Index", 0, QApplication::UnicodeUTF8));
//     ModListView->header()->setClickEnabled(true, ModListView->header()->count() - 1);
//     ModListView->header()->setResizeEnabled(true, ModListView->header()->count() - 1);
    ModListView->sortItems(7,Qt::AscendingOrder); // set ordering by last column (i.e. index)
    QObject::connect(turnoffButton, SIGNAL(clicked()), this, SLOT(allOff()));
    QObject::connect(turnonButton, SIGNAL(clicked()), this, SLOT(allOn()));
    QObject::connect(readButton, SIGNAL(clicked()), this, SLOT(updateView()));
    QObject::connect(upItemButton, SIGNAL(clicked()), this, SLOT(upCurrItem()));
    QObject::connect(downItemButton, SIGNAL(clicked()), this, SLOT(downCurrItem()));
    // background check of status
    QTimer *timer = new QTimer(this);
    timer->start(2000);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkStatus()) );

    // get command from eudaq producer
    QObject::connect( &m_engine, SIGNAL(powerOn()), this, SLOT(allOn()));
    QObject::connect( &m_engine, SIGNAL(powerOff()), this, SLOT(allOff()));

    break;}
  default:
    textLabel1->setText("unknown view-mode");
  }
  m_edtItem.clear();
  m_lastDcsRead = new QTime();
  m_lastDcsRead->start();

  ModListView->setSelectionMode(QAbstractItemView::SingleSelection);
  itemSelected(0, 0);
  ModListView->setContextMenuPolicy(Qt::CustomContextMenu);

  QObject::connect(ModListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(editModule(QTreeWidgetItem*, int)));
  QObject::connect(ModListView, SIGNAL(customContextMenuRequested (const QPoint &)),  this, SLOT(rightClick(const QPoint &)));
  QObject::connect(ModListView, SIGNAL( itemClicked(QTreeWidgetItem*, int) ), this, SLOT( itemSelected(QTreeWidgetItem*, int) ) );  

  // get updates from engine
  QObject::connect( &m_engine, SIGNAL( crateListChanged() ), this, SLOT( updateView() ) );
  QObject::connect( &m_engine, SIGNAL( configChanged() ), this, SLOT( updateView() ) );
  QObject::connect( &m_engine, SIGNAL( statusChanged() ), this, SLOT( changeStatus() ) ); 
}
ModList::~ModList(){
  delete m_lastDcsRead;
}
void ModList::editModule(QTreeWidgetItem *in_item, int col){
  if(col==0) editModule(in_item,true);
}
void ModList::editModule(QTreeWidgetItem *in_item, bool doubleClick){
  // Make sure in_item is not 0.
  if(in_item==0) return;

  ModListItem* item = dynamic_cast<ModListItem*>(in_item);
  if(item!=0){

    int newId = -1;
    for(std::map< int, std::vector<int>* >::iterator mapit = m_edtItem.begin(); mapit!=m_edtItem.end(); mapit++){
      if(mapit->first > newId) newId = mapit->first;
    }
    newId++;// new index = max(exist. indices)+1

    std::vector<int> *tmpVec = new std::vector<int>;
    tmpVec->push_back(item->crateId());
    tmpVec->push_back(item->grpId());
    tmpVec->push_back(item->modId());
    m_edtItem[newId] = tmpVec;
    //  m_edtItem = dynamic_cast<ModListItem*>(in_item);
    
    // if in-item is module group, avoid change in open state
    if((item->getType()==ModListItem::tgroup || item->getType()==ModListItem::tDCS) && doubleClick)
      item->setExpanded(!item->isExpanded());

    // cast only works for ModuleList items
    if(item->getType()==ModListItem::tmodule ){

      STRodCrate *crate=0;
//       if(m_cfgEditOpen){
// 	QMessageBox::warning(this,"ModList::editModule()","Another config editor is already open, can't proceed");
// 	return;
//       }
      crate = item->getCrate();
      if(crate==0) return;
      if(crate->getPixCtrlStatus(item->grpId())==tbusy ||
	 crate->getPixCtrlStatus(item->grpId())==tscanning ||
	 item->text(1)=="testing"){ // module or ROD is doing something, do not disturb
	QMessageBox::information(this,"ModList::editModule()","ROD or module is busy, can't edit config.");
	return;
      }
      Config &mcfg = crate->getPixModuleConf(item->grpId(), item->modId());
      std::string fetype="unknown";
      if(mcfg["general"].name()!="__TrashConfGroup__" && mcfg["general"]["FE_Flavour"].name()!="__TrashConfObj__")
	fetype = ((ConfList&)mcfg["general"]["FE_Flavour"]).sValue();
      if(fetype=="FE_I2" || fetype=="FE_I1"){      // FE-I1, FE-I2/3
	Config *mcc_conf=0;
	std::vector<Config*> fe_conf;
	for(int chipID=0;chipID<16;chipID++) {
	  Config &tmpCfg = crate->getPixModuleChipConf(item->grpId(), item->modId(), chipID);
	  if(tmpCfg.name()=="dummychip") break; // end of FE list reached
	  else fe_conf.push_back(&tmpCfg);
	}
	Config &tmpCfg = crate->getPixModuleChipConf(item->grpId(), item->modId(), 16);
	if(tmpCfg.name()!="dummychip") mcc_conf = &tmpCfg; // have MCC
	ConfigEdit *cfgedt = new ConfigEdit(&mcfg, mcc_conf, fe_conf, newId, 0);
	cfgedt->setAttribute(Qt::WA_DeleteOnClose);
	// read module name
	std::string mname = crate->getModName(item->grpId(), item->modId());
	PixLib::getDecNameCore(mname);
	cfgedt->ModName->setText(mname.c_str());
	cfgedt->GroupName->setText(crate->getGrpName(item->grpId()).c_str());
	QObject::connect(cfgedt, SIGNAL(cfgSaved(int, int)), this, SLOT(saveFromEdit(int, int)));
	QObject::connect(cfgedt, SIGNAL(cfgDone(int)), this, SLOT(editClosed(int)));
	cfgedt->show();
      } 
      else if(fetype=="FE_I4A" || fetype=="FE_I4B"){ // FE-I4
	// create and open editor panel
	std::vector<Config*> conf;// first elements are FEs, last is module
	for(int chipID=0;chipID<99;chipID++) {
	  Config &tmpCfg = crate->getPixModuleChipConf(item->grpId(), item->modId(), chipID);
	  if(tmpCfg.name()=="dummychip") break; // end of FE list reached
	  else conf.push_back(&tmpCfg);
	}
	ConfigEditI4 *cfgedt = new ConfigEditI4(&mcfg,conf, newId, 0);
	cfgedt->setAttribute(Qt::WA_DeleteOnClose);
	// read module name
	std::string mname = crate->getModName(item->grpId(), item->modId());
	PixLib::getDecNameCore(mname);
	cfgedt->ModName->setText(mname.c_str());
	QObject::connect(cfgedt, SIGNAL(cfgSaved(int, int)), this, SLOT(saveFromEdit(int, int)));
	QObject::connect(cfgedt, SIGNAL(cfgDone(int)), this, SLOT(editClosed(int)));
	cfgedt->show();
      } else { // something unknown
	// use generic config editor
	optionsPanel op(mcfg,this,0, true, false, true);
	if(op.exec()==QDialog::Accepted) saveFromEdit(item->modId(), -1);
      }
    } else if(item->getType()==ModListItem::tBOC && item->parentGroup()!=0 &&
	      item->parentGroup()->getType()==ModListItem::tgroup){
#ifndef NOTDAQ
      STRodCrate *crt = item->getCrate();
      if(crt==0) return;
      
      optionsPanel op(crt->getPixBocConf(item->grpId()),this,0, true, false, true);
      if(op.exec()==QDialog::Accepted){
	// store globally and individ. that this BOC was edited
	m_engine.editedCfg();
	// store individ. that this BOC was edited
	crt->editedCfg(item->grpId(),42);
      }
#endif
      
    } else if(item->getType()==ModListItem::tROD && item->parentGroup()!=0 &&
	      item->parentGroup()->getType()==ModListItem::tgroup){
#ifndef NOTDAQ
      STRodCrate *crt = item->getCrate();
      if(crt==0) return;
      if(crt->getPixCtrlStatus(item->grpId())==tbusy ||
	 crt->getPixCtrlStatus(item->grpId())==tscanning) return; // ROD is doing something
      // do not disturb
      
      Config &cfgROD = crt->getPixCtrlConf(item->grpId());
      RodEditor *redt;
      
      if(crt->hasBoc(item->grpId()))
	redt = new RodEditor(cfgROD, crt->getPixBocConf(item->grpId()), m_engine, crt, this);
      else
	redt = new RodEditor(cfgROD, m_engine, crt, this);
      
      if(redt->exec()){
	m_engine.editedCfg(); 
	// store individ. that this ROD was edited
	crt->editedCfg(item->grpId(),41);
	
	// set group name as per ROD slot
	ModListItem *group;
	if((group=item->parentGroup())!=0)
	  group->setText(0,"SLOT "+QString::number(redt->loaded_RODslots[redt->RodSlotBox->currentIndex()]));
	Config &conf = crt->getGrpConf(item->grpId());
	std::string new_name = ("SLOT "+QString::number(redt->loaded_RODslots[redt->RodSlotBox->currentIndex()])).toLatin1().data();
	if(conf["general"].name()!="__TrashConfGroup__" &&
	   conf["general"]["ModuleGroupName"].name()!="__TrashConfObj__")
	  ((ConfString&)conf["general"]["ModuleGroupName"]).m_value = new_name;
	
	// set DTO speed of all modules
	STRodCrate *crt = item->getCrate();
	if(crt!=0){
	  std::vector< std::pair<std::string, int> > list;
	  crt->getModules(item->grpId(), list);
	  for(int iMod=0; iMod<(int)list.size(); iMod++){
	    Config &mccconf = crt->getPixModuleChipConf(item->grpId(), iMod, 16);
	    int mccmod = redt->SetupBox->currentIndex();
	    if(mccmod>0) mccmod++; // 1==40MHz dual is not used
	    if(mccconf["Registers"].name()!="__TrashConfGroup__" && 
	       mccconf["Registers"]["CSR_OutputMode"].name()!="__TrashConfObj__")
	      WriteIntConf((ConfInt&)mccconf["Registers"]["CSR_OutputMode"], mccmod);
	  }
	}
	
	// change ROD-status back to unitialised
	crt->setPixCtrlStatus(item->grpId(), tblocked);
	// NB: the following call is the last thing to do!!!!
	// crate list will be changed by this call, item pointer is nonsense!
	//      m_engine.changedConfigs(); // shouldn't be necessary anymore
      }
      delete redt;
#endif
    } else if(item->getType()==ModListItem::tgroup){
      STRodCrate *crt = item->getCrate();
      if(crt==0) return;
      if(crt->getPixCtrlStatus(item->grpId())==tbusy ||
	 crt->getPixCtrlStatus(item->grpId())==tscanning) return; // ROD is doing something
      // do not disturb
      bool wasEdited=false;
      std::vector<int> mods;
      if(m_engine.whichIFtype()==tRCCVME){
	GroupEditor gedt(m_engine,item->crateId(),item->grpId(),this);
	if(gedt.exec()==QDialog::Accepted){
	  mods = gedt.editedModules();
	  wasEdited = true;
	}
      } else{
	optionsPanel op(crt->getGrpConf(item->grpId()),this, 0,true, false, true);
	wasEdited = (op.exec()==QDialog::Accepted);
      }
      if(wasEdited){
	m_engine.editedCfg();
	// store individ. that this group was edited
	crt->editedCfg(item->grpId(),40);
	// set item name as it is in config
	Config &conf = crt->getGrpConf(item->grpId());
	if(conf["general"].name()!="__TrashConfGroup__" &&
	   conf["general"]["ModuleGroupName"].name()!="__TrashConfObj__")
	  item->setText(0,((ConfString&)conf["general"]["ModuleGroupName"]).value().c_str());
	for(std::vector<int>::iterator it=mods.begin(); it!=mods.end(); it++)
	  crt->editedCfg(item->grpId(), (*it)); // store individ. that this module was edited
	// send modified module config to PixController if that's initialised
	// and ask edit panel if anything was acutally changed
	if(crt->getPixCtrlStatus(item->grpId())==tOK && mods.size()>0){
	  for(std::vector<int>::iterator it=mods.begin(); it!=mods.end(); it++){
	    crt->editedCfg(item->grpId(), (*it)); // store individ. that this module was edited
	    try{
	      crt->writeConfig(item->grpId(), (*it));
	    } catch(...){
	      QMessageBox::warning(this,"ModList::editModule","Error sending edited configuration to PixController!");
	    }
	  }
	  m_engine.readRodBuff("Post-config:");
	}
	// NB: the following call is the last thing to do!!!!
	// crate list will be changed by this call, item pointer is nonsense!
	m_engine.changedConfigs();
      }
    } else if(item->getType()==ModListItem::tUSB && item->parentGroup()!=0 &&
	      item->parentGroup()->getType()==ModListItem::tgroup){
      STRodCrate *crt = item->getCrate();
      if(crt==0) return;
      if(crt->getPixCtrlStatus(item->grpId())==tbusy ||
	 crt->getPixCtrlStatus(item->grpId())==tscanning) return; // system is doing something
      // do not disturb
      
      Config &cfgUSB = crt->getPixCtrlConf(item->grpId());
      USBCtrlEdit *uce = new USBCtrlEdit(cfgUSB,this);
      if(uce->exec()){
	m_engine.editedCfg(); 
	// store individ. that this PixController was edited
	crt->editedCfg(item->grpId(),41);
	if(uce->recfgNeeded()) crt->setPixCtrlStatus(item->grpId(), tblocked);
	else                   crt->reloadCtrlCfg(item->grpId());
      }
      delete uce;
    } else if(item->getType()==ModListItem::tCTRL && item->parentGroup()!=0 &&
	      item->parentGroup()->getType()==ModListItem::tgroup){
      STRodCrate *crt = item->getCrate();
      if(crt==0) return;
      if(crt->getPixCtrlStatus(item->grpId())==tbusy ||
	 crt->getPixCtrlStatus(item->grpId())==tscanning) return; // system is doing something
      // do not disturb
      
      Config &cfgROD = crt->getPixCtrlConf(item->grpId());
      optionsPanel *op = new optionsPanel(cfgROD,this, 0,false, false, true);
      op->setWindowTitle("PixController config editor");
      if(op->exec()){
	m_engine.editedCfg(); 
	// store individ. that this PixController was edited
	crt->editedCfg(item->grpId(),41);
	crt->setPixCtrlStatus(item->grpId(), tblocked);
      }
      delete op;
    } else if(item->getType()==ModListItem::tDCSC){
      STRodCrate *crt = item->getCrate();
      if(crt==0 || item->parent()==0) return;
      
      std::vector<PixDcs*> dcsv = crt->getPixDcs();
      PixDcs *dcs = 0;
      for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
	if((*dcsi)->getIndex()==((ModListItem*)item->parent())->grpId()){
	  dcs=*dcsi;
	  break;
	}
      }
      if(dcs!=0 && dcs->getChan(item->grpId())!=0){
	//    if(((ModListItem*)item->parent())->grpId()>=0 && ((ModListItem*)item->parent())->grpId()<(int)dcs.size()){
	//if(item->grpId()>=0 && item->grpId()<dcs->nchan()){
	Config &cfgDcs = dcs->getChan(item->grpId())->config();
	optionsPanel *op = new optionsPanel(cfgDcs,this,0,false, false, true, false);
	op->setWindowTitle("PixDcs config editor - channel "+QString::number(item->grpId()));
	if(op->exec()){
	  try{
	    dcs->SetState("UPDATE");
	  }
	  catch(PixDcsExc& tException){
	    m_engine.toLog(std::string("WARNING: ModList::editModule: ").append(tException.getDescr()));
	  }
	  m_engine.editedCfg();
	}
	delete op;
	//}
      }
      updateViewDCS();
    } else if(item->getType()==ModListItem::tDCS){
      STRodCrate *crt = item->getCrate();
      if(crt==0) return;
      
      std::vector<PixDcs*> dcsv = crt->getPixDcs();
      PixDcs *dcs = 0;
      for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
	if((*dcsi)->getIndex()==item->grpId()){
	  dcs=*dcsi;
	  break;
	}
      }
      //    if(item->grpId()>=0 && item->grpId()<(int)dcs.size()){
      if(dcs!=0){
	Config &cfgDcs = dcs->config();
	optionsPanel *op = new optionsPanel(cfgDcs,this,0,false,false, true, false);
	op->setWindowTitle("PixDcs config editor");
	if(op->exec()) m_engine.editedCfg();
	delete op;
      }
      updateViewDCS();
    }
  }

  return;
}
void ModList::editClosed(int listId){
  std::vector<int>* tmpVec = m_edtItem[listId];
  delete tmpVec;
  m_edtItem[listId] = 0;
}
void ModList::saveFromEdit(int new_modID, int listId){
  //if(cfgedt.editState()){ // ask edit panel if anything was acutally changed
  if(m_edtItem[listId]!=0){
    // update module ID, otherwise identification will not work
    (*(m_edtItem[listId]))[2] = new_modID;
    m_engine.editedCfg();                         // store generally that something was edited
    STRodCrate *crt = m_engine.getSTRodCrates()[m_edtItem[listId]->at(0)];
    crt->editedCfg(m_edtItem[listId]->at(1), m_edtItem[listId]->at(2)); // store individ. that this module was edited
    // send modified module config to PixController if that's initialised
    if(crt->getPixCtrlStatus(m_edtItem[listId]->at(1))==tOK){
      try{
	crt->writeConfig(m_edtItem[listId]->at(1), m_edtItem[listId]->at(2));
      } catch(...){
	QMessageBox::warning(this,"ModList::saveFromEdit","Error sending edited configuration to PixController!");
      }
      m_engine.readRodBuff("Post-config:");
    }
    // update module lists
    updateView();
  } else
    QMessageBox::warning(this,"ModList::saveFromEdit","Function called, but needed member variables are NULL");
}
void ModList::rightClick(const QPoint &){
  ModListItem* item = dynamic_cast<ModListItem*>(ModListView->currentItem());
  int col = ModListView->currentColumn();
  if(item==0) return; // nothing to do
  if(item->text(2)=="busy" || item->text(2)=="---") return;  // must not touch busy modules or uninit. DCS objects
  if(((ModListItem*)item)->getType()!=ModListItem::tmodule && 
     ((ModListItem*)item)->getType()!=ModListItem::tDCS && 
     ((ModListItem*)item)->getType()!=ModListItem::tDCSC) return; // not a valid operation on RODs and crates
  switch(col){
  default:
    break;
  case 2: // toggle enable/disable
    toggleEnable(item);
    break;
  }
  return;
}
void ModList::toggleEnable(QTreeWidgetItem *in_item)
{
  //  Make sure that in_item was a ModListItem
  ModListItem *item = dynamic_cast<ModListItem*>(in_item);
  if(item==0) return;

  if(item->getType()==ModListItem::tmodule) item->toggleEnable();

  else if(item->getType()==ModListItem::tDCS && item->getCrate()!=0){
    std::vector<PixDcs*> dcsv = item->getCrate()->getPixDcs();
    PixDcs *dcs = 0;
    for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
      if((*dcsi)->getIndex()==((ModListItem*)item)->grpId()){
        dcs=*dcsi;
        break;
      }
    }
    if(dcs!=0){
      std::string state = dcs->ReadState("");
      if((state=="ON" || state=="ON*" || state =="OFF" || state=="ALL ON" || state =="ALL OFF" || state=="LIM") 
          && (
            (dcs->getDevType()==PixDcs::SUPPLY)
            || (dcs->getDevType()==PixDcs::PULSER)
            || (dcs->getDevType()==PixDcs::POSITION)
            || (dcs->getDevType()==PixDcs::VOLTAGE_SOURCE)
            || (dcs->getDevType()==PixDcs::CURRENT_SOURCE)
            || (dcs->getDevType()==PixDcs::CHILLER)
          )){
        if(state=="ON" || state=="ALL ON" || state=="LIM"){
          QApplication::setOverrideCursor(Qt::WaitCursor);
          dcs->SetState("OFF");
          QApplication::restoreOverrideCursor();
        }else if(state=="ON*"){
          QApplication::setOverrideCursor(Qt::WaitCursor);
          try{
            dcs->SetState("UPDATE");
          }catch(...){}
          QApplication::restoreOverrideCursor();
        }else{
          QApplication::setOverrideCursor(Qt::WaitCursor);
          dcs->SetState("ON");
          QApplication::restoreOverrideCursor();
        }
      }
    }
    PixLib::sleep(500); // make sure last channels have stabilised
    updateViewDCS();
  } 
  else if(item->getType()==ModListItem::tDCSC && item->getCrate()!=0){
    std::vector<PixDcs*> dcsv = item->getCrate()->getPixDcs();
    PixDcs *dev = 0;
    for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
      if((*dcsi)->getIndex()==((ModListItem*)item->parent())->grpId()){
        dev=*dcsi;
        break;
      }
    }
    if(dev!=0 && dev->getChan(item->grpId())!=0){
      std::string state = dev->getChan(item->grpId())->ReadState("");
      if((state=="ON" || state=="ON*" || state =="OFF" || state=="LIM")
          && (
            (dev->getDevType()==PixDcs::SUPPLY)
            || (dev->getDevType()==PixDcs::PULSER)
            || (dev->getDevType()==PixDcs::POSITION)
            || (dev->getDevType()==PixDcs::VOLTAGE_SOURCE)
            || (dev->getDevType()==PixDcs::CURRENT_SOURCE)
            || (dev->getDevType()==PixDcs::CHILLER)
          )){
        if(state=="ON" || state=="LIM"){
          QApplication::setOverrideCursor(Qt::WaitCursor);
          dev->getChan(item->grpId())->SetState("OFF");
          QApplication::restoreOverrideCursor();
        }else if(state=="ON*"){
          QApplication::setOverrideCursor(Qt::WaitCursor);
          try{
            dev->SetState("UPDATE");
          }catch(...){}
          QApplication::restoreOverrideCursor();
        }else{
          QApplication::setOverrideCursor(Qt::WaitCursor);
          dev->getChan(item->grpId())->SetState("ON");
          QApplication::restoreOverrideCursor();
        }
      }
    }
    PixLib::sleep(500); // make sure last channels have stabilised
    updateViewDCS();
  }

  return;
}
void ModList::allSwitch(bool in_on){
  std::map<int, PixDcs* > myDcs;
  for(int iTop=0; iTop<ModListView->topLevelItemCount();iTop++){
    ModListItem *tlitem = dynamic_cast<ModListItem*>(ModListView->topLevelItem(iTop));
    if(tlitem!=0){
      QApplication::setOverrideCursor(Qt::WaitCursor);
      for(int i=0;i<tlitem->childCount(); i++){
        ModListItem *item = dynamic_cast<ModListItem*>(tlitem->child(i));
        if( item->getType()==ModListItem::tDCS && item->getCrate()!=0){
          std::vector<PixDcs*> dcsv = item->getCrate()->getPixDcs();
          PixDcs *dcs = 0;
          for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
            if((*dcsi)->getIndex()==((ModListItem*)item)->grpId()){
              dcs=*dcsi;
              break;
            }
          }
          if(dcs!=0){
            if  ((dcs->getDevType()==PixDcs::SUPPLY)
              || (dcs->getDevType()==PixDcs::PULSER)
              || (dcs->getDevType()==PixDcs::POSITION)
              || (dcs->getDevType()==PixDcs::VOLTAGE_SOURCE)
              || (dcs->getDevType()==PixDcs::CURRENT_SOURCE)
              || (dcs->getDevType()==PixDcs::CHILLER))
            {
              int myInd = dcs->getIndex();
              if(!in_on && offReverse->isChecked()) myInd *= (-1);
              myDcs[myInd] = dcs;
            }
          }
        }
      }
      for(std::map<int, PixDcs* >::iterator IT=myDcs.begin(); IT!=myDcs.end();IT++){
        IT->second->SetState(in_on?"ON":"OFF");
        if(in_on){
          try{
            IT->second->SetState("UPDATE");
          }catch(...){}
        }
        //printf("turning %s device %s with ID %d\n", in_on?"ON":"OFF", IT->second->name().c_str(), IT->first);
      }
    }
  }
  updateViewDCS();
  QApplication::restoreOverrideCursor();
  return;
}
// void ModList::deleteModuleGroup(ModListItem *item){
//   // Make sure item is not 0.
//   if(item==0) {
//     return;
//   }
//   if( item->getType()==ModListItem::tgroup ) {
//     delete item;
//     return;
//   } else {
//     std::cerr << "[Note] ModList::deleteModuleGroup(ModListItem *item) :\n"
//       << "[Note] *item does not contain a PixModuleGroup according to getType()\n"
//       << "[Note] returning" << endl;
//     return;
//   }
// }
/** Return the currently selected Module as given by currentItem() */
ModListItem * ModList::currentModule() const{
  ModListItem *item = m_selItem;
  // We promised we'd return a module
  if( item->getType() == ModListItem::tmodule ) {
    return item;
  } else {
    return 0;
  }
}
/** Return the currently selected Module as given by currentItem() */
ModListItem * ModList::currentROD() const{
  ModListItem *item = m_selItem;
  // We promised we'd return a module
  if( item->getType() == ModListItem::tROD ) {
    return item;
  } else {
    return 0;
  }
}
/** Return the currently selected Module as given by currentItem() */
ModListItem * ModList::currentUSB() const{
  ModListItem *item = m_selItem;
  // We promised we'd return a module
  if( item->getType() == ModListItem::tUSB ) {
    return item;
  } else {
    return 0;
  }
}
/** Return the currently selected Module as given by currentItem() */
ModListItem * ModList::currentBOC() const{
  ModListItem *item = m_selItem;
  // We promised we'd return a module
  if( item->getType() == ModListItem::tBOC ) {
    return item;
  } else {
    return 0;
  }
}
/** Return the currently selected Group as given by currentItem(), or its parent Group */
ModListItem * ModList::currentGroup() const{
  ModListItem *item = m_selItem;
  if( item == 0 )
    return 0;

  switch( item->getType() ) {
  case ModListItem::tgroup:  
    return item; 
    break;
  case ModListItem::tmodule: 
    return item->parentGroup(); 
    break;
  default: 
    return 0;
  }

  return 0;
}
/** Return the currently selected crate as given by currentItem() */
ModListItem * ModList::currentCrate() const{
  ModListItem *item = m_selItem;
  if( item == 0 )
    return 0;

  switch( item->getType() ) {
  case ModListItem::tcrate:  
    return item; 
    break;
  default: 
    return 0;
  }

  return 0;
}
/** Update the ListView */
void ModList::updateView()
{
  if(m_scanBusy && noUpdateInScan->isChecked()) return;

  switch(m_mode){
  case DAQList:
    updateViewDAQ();
    break;
  case DCSList:
    updateViewDCS();
    break;
  default:
    QMessageBox::warning(this,"ModList::updateView","Mode is undefined, can't select correct function");
  }
}
void ModList::updateViewDCS(){
  // check time since last read and skip if too shortly ago -  otherwise GUI freezes during certain scans
  if(m_scanBusy && m_lastDcsRead->elapsed()<1500) return;
  m_lastDcsRead->restart();
  // We regenerate the whole view from scratch.
  // Delete all items
  ModListView->clear();
  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  unsigned int crateID=0;
  for( std::vector<STRodCrate *>::iterator crate = m_engine.getSTRodCrates().begin(); 
       crate != m_engine.getSTRodCrates().end(); crate++ ) {
    ModListItem *crateItem = new ModListItem( m_engine, ModListView, *crate);
    crateID++;
    crateItem->setExpanded( true );
    crateItem->setText(1,"DCS");
    std::vector<PixDcs*> dcsvec = (*crate)->getPixDcs();
    for(std::vector<PixDcs*>::iterator dcs = dcsvec.begin(); dcs!=dcsvec.end(); dcs++){
      // update error log - good occasion to do this here
      std::string errTxt;
      (*dcs)->ReadError(errTxt);
      if(errTxt!="") m_engine.toErrLog("from DCS device "+(*dcs)->name()+": "+errTxt);
      ModListItem *dcsItem = new ModListItem( m_engine, crateItem, ModListItem::tDCS, (*dcs)->getIndex() );
      dcsItem->setText(7,QString::number((*dcs)->getIndex()));
      dcsItem->setText(0,(*dcs)->name().c_str());
      std::string state = (*dcs)->ReadState("");
      if(state=="ON" || state=="ON*" || state =="OFF" || state=="ALL ON" || state =="ALL OFF"  || state =="LIM" ){
	dcsItem->setText(1,"OK");
	dcsItem->setText(2,state.c_str());
	if(state=="LIM")
	  m_engine.toLog(("ModList::updateViewDCS : device "+dcsItem->text(0) +" is in current or temperature limit state at "+
			  QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy")+".").toLatin1().data());
      }
      if(state =="OK" || state=="CHAN"){
	dcsItem->setText(1,"OK");
	dcsItem->setText(2,"");
      }
      if(state=="ERROR"){
	dcsItem->setText(1,"problem");
	dcsItem->setText(2,"--");
      }
      dcsItem->setExpanded( true );
      //      for(std::vector<PixDcsChan*>::iterator dcsc = (*dcs)->chanBegin(); dcsc!=(*dcs)->chanEnd(); dcsc++){
      for(int cID=0; cID<(*dcs)->nchan(); cID++){
	PixDcsChan *dcsc = (*dcs)->getChan(cID);
	ModListItem *dcscItem = new ModListItem( m_engine, dcsItem, ModListItem::tDCSC, cID );
	dcscItem->setText(0,dcsc->name().c_str());
	dcscItem->setText(7,QString::number(cID));
	dcscItem->setText(1,"---");
	dcscItem->setText(2,"---");
	dcscItem->setText(3,"---");
	dcscItem->setText(4,"---");
	dcscItem->setText(5,"---");
	dcscItem->setText(6,"---");
	if(state=="ON" || state=="ON*" || state =="OFF" || state=="OK" || state=="ALL ON" || state =="ALL OFF" || state=="CHAN" || state =="LIM"){
	  dcscItem->setText(1,"OK");
	  if(state=="ON" || state=="ON*" || state =="OFF" || state =="OK" || state =="LIM")
	    dcscItem->setText(2,"");
	  else{
	    std::string sttxt = dcsc->ReadState("");
	    if(sttxt=="LIM")
	      m_engine.toLog(("ModList::updateViewDCS : channel "+dcscItem->text(0)+" of device " +dcsItem->text(0)
			      +" is in current or temperature limit state at "+
			      QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy")+".").toLatin1().data());
	    dcscItem->setText(2,sttxt.c_str());
	  }
	  if(dcsc->getType()==PixDcsChan::VONLY || dcsc->getType()==PixDcsChan::VCBOTH || dcsc->getType()==PixDcsChan::VCT){
	    try{
	      float volts = dcsc->ReadParam("voltage");
	      if(fabs(volts)<1e-2)
		dcscItem->setText(3,QString::number(volts,'g',4));
	      else
		dcscItem->setText(3,QString::number(volts,'f',4));
	    }catch(...){
	      dcscItem->setText(3,"---");
	    }
	  }
	  if(dcsc->getType()==PixDcsChan::CONLY || dcsc->getType()==PixDcsChan::VCBOTH || dcsc->getType()==PixDcsChan::VCT){
	    try{
	      float curr = dcsc->ReadParam("current");
	      if(fabs(curr)<1e-2)
		dcscItem->setText(4,QString::number(curr,'g',4));
	      else
		dcscItem->setText(4,QString::number(curr,'f',4));
	    }catch(...){
	      dcscItem->setText(4,"---");
	    }
	  }
	  if(dcsc->getType()==PixDcsChan::TEMP || dcsc->getType()==PixDcsChan::VCT){
	    try{
	      float temprt = dcsc->ReadParam("temperature");
	      if(fabs(temprt)<1e-2)
		dcscItem->setText(5,QString::number(temprt,'g',4));
	      else
		dcscItem->setText(5,QString::number(temprt,'f',1));
	    }catch(...){
	      dcscItem->setText(5,"---");
	    }
	  }
	  if(dcsc->getType()==PixDcsChan::FE_GADC){
	    try{
	      float adccnt = dcsc->ReadParam("FE GADC");
	      if(fabs(adccnt)<1e-2)
		dcscItem->setText(3,QString::number(adccnt,'g',4));
	      else
		dcscItem->setText(3,QString::number(adccnt,'f',4));
	    }catch(...){
	      dcscItem->setText(3,"---");
	    }
	  }
	  if(dcsc->getType()==PixDcsChan::POSITION){
		try{
		  int tPosition = dcsc->ReadParam("position");
		  dcscItem->setText(6,QString::number(tPosition,'g',4));
		}catch(...){
		  dcscItem->setText(6,"---");
		}
	  }
	}
      }
    }
  }
  ModListView->sortItems(ModListView->columnCount()-1,Qt::AscendingOrder); // last col. is index
  QApplication::restoreOverrideCursor();
}
void ModList::updateViewDAQ()
{

  // We regenerate the whole view from scratch.
  // Delete all items
  ModListView->clear();

  unsigned int crateID=0;
  for( std::vector<STRodCrate *>::iterator crate = m_engine.getSTRodCrates().begin(); 
       crate != m_engine.getSTRodCrates().end(); crate++ ) {
    ModListItem *crateItem = new ModListItem( m_engine, ModListView, *crate);
    crateID++;
    crateItem->setExpanded( true );
    for(int iGrp=0; iGrp<(*crate)->nGroups(); iGrp++){
      ModListItem *grpItem = new ModListItem( m_engine, crateItem, ModListItem::tgroup, iGrp );
      grpItem->setExpanded( true );
      // add separate ROD and BOC items
      int ctrlType = (*crate)->hasCtrl(iGrp);
      if(ctrlType==1){
        ModListItem *rodItem = new ModListItem( m_engine, grpItem, ModListItem::tROD, iGrp);
        switch((*crate)->getPixCtrlStatus(iGrp)){
        case tOK:
          rodItem->setText(1,"OK");
          break;
        case tproblem:
          rodItem->setText(1,"problem");
          break;
        case tbusy:
          rodItem->setText(1,"busy");
          break;
        case tscanning:
          rodItem->setText(1,"scanning");
          break;
        case tblocked:
        default:
          rodItem->setText(1,"uninit.");
        }
	if((*crate)->hasBoc(iGrp)){
	  ModListItem *bocItem = new ModListItem( m_engine, grpItem, ModListItem::tBOC, iGrp);
          switch((*crate)->getPixBocStatus(iGrp)){
          case tOK:
            bocItem->setText(1,"OK");
            break;
          case tproblem:
            bocItem->setText(1,"problem");
            break;
          case tbusy:
            bocItem->setText(1,"busy");
            break;
          case tblocked:
          default:
            bocItem->setText(1,"uninit.");
          }
        }
      }
      // add separate item for USB board
      else if(ctrlType==2){
        ModListItem *rodItem = new ModListItem( m_engine, grpItem, ModListItem::tUSB, iGrp);
        switch((*crate)->getPixCtrlStatus(iGrp)){
        case tOK:
          rodItem->setText(1,"OK");
          break;
        case tproblem:
          rodItem->setText(1,"problem");
          break;
        case tbusy:
          rodItem->setText(1,"busy");
          break;
        case tscanning:
          rodItem->setText(1,"scanning");
          break;
        case tblocked:
        default:
          rodItem->setText(1,"uninit.");
        }
      }
      // add separate item for other PixControllers
      else if(ctrlType>0){
        ModListItem *rodItem = new ModListItem( m_engine, grpItem, ModListItem::tCTRL, iGrp);
        switch((*crate)->getPixCtrlStatus(iGrp)){
        case tOK:
          rodItem->setText(1,"OK");
          break;
        case tproblem:
          rodItem->setText(1,"problem");
          break;
        case tbusy:
          rodItem->setText(1,"busy");
          break;
        case tscanning:
          rodItem->setText(1,"scanning");
          break;
        case tblocked:
        default:
          rodItem->setText(1,"uninit.");
        }
      }
      // now add all modules
      //      for( PixModuleGroup::moduleIterator mod = (*group)->modBegin(); mod != (*group)->modEnd(); mod++ ){
      std::vector< std::pair<std::string, int> > list;
      (*crate)->getModules(iGrp, list);
      //      for(int iMod=0; iMod<(int)list.size(); iMod++){
      for(std::vector< std::pair<std::string, int> >::iterator IT=list.begin(); IT!=list.end(); IT++){
        ModListItem *modItem = new ModListItem( m_engine, grpItem, ModListItem::tmodule, IT->second);
        modStatus modS = (*crate)->getPixModuleStatus(iGrp,IT->second);
        switch(modS.modStat){
        case tOK:
          modItem->setText(1,"OK");
          break;
        case tbusy:
          modItem->setText(1,"testing");
          break;
        case tproblem:
          modItem->setText(1,"problem");
          break;
        case tfailed:
          modItem->setText(1,"failed");
          break;
        case tblocked:
        case tunknown:
        default:
          modItem->setText(1,"untested");
        }
      }
    }
  }
  ModListView->sortItems(ModListView->columnCount()-1,Qt::AscendingOrder); // last col. is index
  ModListItem *fc=(ModListItem*)ModListView->topLevelItem(0);
  itemSelected(fc);
  emit listUpdated(fc);

}

void ModList::changeStatus()
{
  if(m_scanBusy && noUpdateInScan->isChecked()) return;

  switch(m_mode){
  case DCSList:
    updateViewDCS();
    break;
  default:
    QMessageBox::warning(this,"ModList::updateView","Mode is undefined, can't select correct function");
    break;
    
  case DAQList:{
    
    for(int iTop=0; iTop<ModListView->topLevelItemCount();iTop++){
      ModListItem* crIt = dynamic_cast<ModListItem *>(ModListView->topLevelItem(iTop));
      if(crIt==0) continue;
      STRodCrate *crt = crIt->getCrate();
      for(int iGrp=0; iGrp<crIt->childCount(); iGrp++){
	ModListItem *grpIt = (ModListItem *)crIt->child(iGrp);
	for(int iMod=0; iMod<grpIt->childCount();iMod++){
	  ModListItem *objIt = (ModListItem *)grpIt->child(iMod);
	  if(objIt->getType()==ModListItem::tROD || objIt->getType()==ModListItem::tUSB || objIt->getType()==ModListItem::tCTRL){
	    switch(crt->getPixCtrlStatus(objIt->grpId())){
	    case tOK:
	      objIt->setText(1,"OK");
	      break;
	    case tproblem:
	      objIt->setText(1,"problem");
	      break;
	    case tbusy:
	      objIt->setText(1,"busy");
	      break;
	    case tscanning:
	      objIt->setText(1,"scanning");
	      break;
	    case tblocked:
	    default:
	      objIt->setText(1,"uninit.");
	    }
	  } else if(objIt->getType()==ModListItem::tBOC){
	    switch(crt->getPixBocStatus(objIt->grpId())){
	    case tOK:
	      objIt->setText(1,"OK");
	      break;
	    case tproblem:
	      objIt->setText(1,"problem");
	      break;
	    case tbusy:
	      objIt->setText(1,"busy");
	      break;
	    case tblocked:
	    default:
	      objIt->setText(1,"uninit.");
	    }
	  } else if(objIt->getType()==ModListItem::tmodule){
	    switch(crt->getPixModuleStatus(objIt->grpId(),objIt->modId()).modStat){
	    case tOK:
	      objIt->setText(1,"OK");
	      break;
	    case tbusy:
	      objIt->setText(1,"testing");
	      break;
	    case tproblem:
	      objIt->setText(1,"problem");
	      break;
	    case tfailed:
	      objIt->setText(1,"failed");
	      break;
	    case tblocked:
	    case tunknown:
	    default:
	      objIt->setText(1,"untested");
	    }
	    objIt->setEnabled(crt->getModuleActive(objIt->grpId(),objIt->modId()));
	  }
	}
      }
    }
    itemSelected((ModListItem *)ModListView->topLevelItem(0));
    emit statusChanged();
    
  }
  }

}

void ModList::checkStatus(){
  if(!refrStatus->isChecked()) return;
  ModListItem *top_item = dynamic_cast<ModListItem*>(ModListView->topLevelItem(0));
  if(top_item!=0){
    for(int i=0;i<top_item->childCount();i++){
      ModListItem *item = dynamic_cast<ModListItem*>(top_item->child(i));
      for(int j=0;j<item->childCount();j++){
	ModListItem *item2 = dynamic_cast<ModListItem*>(item->child(j));
	if(item2->getType()==ModListItem::tDCSC && item2->getCrate()!=0){
	  std::vector<PixDcs*> dcsv = item2->getCrate()->getPixDcs();
	  PixDcs *dev = 0;
	  for(std::vector<PixDcs*>::iterator dcsi = dcsv.begin(); dcsi!=dcsv.end(); dcsi++){
	    if((*dcsi)->getIndex()==item->grpId()){
	      dev=*dcsi;
	      break;
	    }
	  }
	  if(dev!=0 && dev->getChan(item2->grpId())!=0){
	    try{
	      std::string oldtxt = item2->text(2).toLatin1().data();
	      std::string sttxt = dev->getChan(item2->grpId())->ReadState("");
	      if(sttxt!="unknown") item2->setText(2,sttxt.c_str());
	      if(sttxt=="LIM" && oldtxt!="LIM")
		m_engine.toErrLog(("ModList::checkStatus : channel "+item2->text(0)+" of device " +item->text(0)
				   +" went into current or temperature limit state at "+
				   QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy")+".").toLatin1().data());
	    }catch(...) {}
	  }
	}
      }
    }
  }
}
void ModList::itemSelected(QTreeWidgetItem *item, int ){
  m_selItem = dynamic_cast<ModListItem*>(item);
  if(m_selItem==0){
    upItemButton->setEnabled(false);
    downItemButton->setEnabled(false);
  } else{
    upItemButton->setEnabled(m_selItem->getType()==ModListItem::tDCS);
    downItemButton->setEnabled(m_selItem->getType()==ModListItem::tDCS);
  }
}
void ModList::upCurrItem()
{
  int lastId = -1;
  ModListItem *mitem = m_selItem;
  ModListView->sortItems(ModListView->columnCount()-1,Qt::AscendingOrder); // last col. is index
  if(mitem!=0 && mitem->parent()!=0){
    int nextID = mitem->parent()->indexOfChild(mitem)-1;
    ModListItem *mli_prev = 0;
    if(nextID>=0) mli_prev = (ModListItem *)mitem->parent()->child(nextID);
    if(mli_prev==0)
      QMessageBox::warning(this,"ModList::upCurrItem","Item is at top, can't move");
    else{
      lastId = mitem->grpId()-1;
      mitem->up();
    }
    updateView();
    if(lastId>=0){
      ModListItem *top_item = dynamic_cast<ModListItem*>(ModListView->topLevelItem(0));
      if(top_item!=0){
	for(int i=0;i<top_item->childCount();i++){
	  mitem = dynamic_cast<ModListItem*>(top_item->child(i));
	  if(mitem!=0 && mitem->grpId()==lastId){
	    itemSelected(mitem);
	    break;
	  }
	}
      }      
    }
    ModListView->sortItems(ModListView->columnCount()-1,Qt::AscendingOrder); // last col. is index
  }else
    QMessageBox::warning(this,"ModList::upCurrItem","Please select an item before trying to move it");

  return;
}
void ModList::downCurrItem()
{
  int lastId = -1;
  ModListItem *mitem = m_selItem;
  ModListView->sortItems(ModListView->columnCount()-1,Qt::AscendingOrder); // last col. is index
  if(mitem!=0 && mitem->parent()!=0){
    int nextID = mitem->parent()->indexOfChild(mitem)+1;
    ModListItem *mli_next = 0;
    if(nextID< mitem->parent()->childCount()) mli_next = (ModListItem *)mitem->parent()->child(nextID);
    if(mli_next==0)
      QMessageBox::warning(this,"ModList::downCurrItem","Item is at bottom, can't move");
    else{
      lastId = mitem->grpId()+1;
      mitem->down();
    }
    updateView();
    if(lastId>=0){
      ModListItem *top_item = dynamic_cast<ModListItem*>(ModListView->topLevelItem(0));
      if(top_item!=0){
	for(int i=0;i<top_item->childCount();i++){
	  mitem = dynamic_cast<ModListItem*>(top_item->child(i));
	  if(mitem!=0 && mitem->grpId()==lastId){
	    itemSelected(mitem);
	    break;
	  }
	}
      }      
    }
    ModListView->sortItems(ModListView->columnCount()-1,Qt::AscendingOrder); // last col. is index
  }else
    QMessageBox::warning(this,"ModList::downCurrItem","Please select an item before trying to move it");

  return;
}
