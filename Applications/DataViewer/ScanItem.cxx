#include "ScanItem.h"
#include "PlotFactory.h"
#include "OptionsPanel.h"

#include <DataContainer/PixDBData.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <Config/Config.h>
#include <PixController/PixScan.h>
#include <DBTreeItem.h>
#include <GeneralDBfunctions.h>

#include <QApplication>

#include <string>
#include <map>

ModuleAction::ModuleAction(QTreeWidgetItem* item, PixDBData &dbData, int type)
  : QTreeWidgetItem(item), m_dbData(dbData), m_type(type) {}
ModuleAction::~ModuleAction(){}
bool ModuleAction::isHistoPlot(){
  return (m_type < (int)PixLib::PixScan::MAX_HISTO_TYPES);
}

ModuleItem::ModuleItem(QTreeWidgetItem* item, PixLib::DBInquire *inquire, const char *fname, const char *modname)
  : QTreeWidgetItem(item) {
  std::string path = fname;
  path += ":"+inquire->getDecName();
  path = path.erase(path.length()-1,1);
  m_dbData = new PixDBData(item->text(0).toLatin1().data(), path.c_str(), modname);
  loadContent();
}
ModuleItem::~ModuleItem(){
  delete m_dbData;
}
void ModuleItem::loadContent(){
  // get module geometry
  int nCol, nRow, nFe;
  m_dbData->getGeom(nRow, nCol, nFe);
  setText(0, QString("Module ")+QString(m_dbData->GetModName()));
  QTreeWidgetItem *cfg = new QTreeWidgetItem(this);
  cfg->setText(0, QString("Module Configuration"));
  // add sub-items for options panel and map plotting
  ModuleAction *act = new ModuleAction(cfg, (*m_dbData), (10+(int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString("Show Config."));
  act = new ModuleAction(cfg, (*m_dbData), ((int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString("Plot TDAC map"));
  act = new ModuleAction(cfg, (*m_dbData), (1+(int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString("Plot FDAC map"));
  act = new ModuleAction(cfg, (*m_dbData), (2+(int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString("Plot ENABLE map"));
  act = new ModuleAction(cfg, (*m_dbData), (3+(int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString((nRow%336==0)?"Plot CAP0 map":"Plot SELECT map"));
  act = new ModuleAction(cfg, (*m_dbData), (4+(int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString((nRow%336==0)?"Plot CAP1 map":"Plot PREAMP map"));
  act = new ModuleAction(cfg, (*m_dbData), (5+(int)PixLib::PixScan::MAX_HISTO_TYPES));
  act->setText(0, QString((nRow%336==0)?"Plot ILEAK map":"Plot HITBUS map"));
  PixScan psc;
  std::map<std::string, int> htypes = psc.getHistoTypes();
  PixLib::DBInquire *inquire = m_dbData->openDBFile();
  for(PixLib::recordIterator hit=inquire->recordBegin(); hit!=inquire->recordEnd();hit++){
    int htype = htypes[(*hit)->getName()];
    if(htype>0 || (*hit)->getName()=="OCCUPANCY"){
      act = new ModuleAction(this, (*m_dbData), htype);
      act->setText(0, QString(("Plot "+(*hit)->getName()).c_str()));
    }
  }
}
void ModuleItem::reLoadContent(){
  for(int i=childCount()-1; i>=0;i--)
    removeChild(child(i));
  loadContent();
}

ScanItem::ScanItem(QTreeWidgetItem* item, PixLib::DBInquire *inquire, const char *fname)
  : QTreeWidgetItem(item),m_fname(fname) {
  setupContent(inquire);
}
ScanItem::ScanItem(QTreeWidget* list, PixLib::DBInquire *inquire, const char *fname)
  : QTreeWidgetItem(list),m_fname(fname) {
  setupContent(inquire);
}
ScanItem::~ScanItem(){
}
void ScanItem::setupContent(PixLib::DBInquire *inquire){
  // general scan information
  DBTreeItem *info = new DBTreeItem(this, inquire, m_fname.c_str(), false, true);
  info->setText(0, QString("Scan Info"));
  for(int i=0;i<info->childCount(); i++){
    if(info->child(i)->text(0)=="Comment")
      ((DBTreeItem*)info->child(i))->setWritable();
  }
  // add modules and their scan results
  bool firstMod=true;
  std::string decName = inquire->getDecName();
  m_decName = decName+inquire->getName();
  QString itemName = inquire->getDecName().c_str();
  // remove heading,trailing "/"
  itemName = itemName.mid(1,itemName.length()-2);
  setText(0, itemName);
  for(PixLib::recordIterator rit=inquire->recordBegin(); rit!=inquire->recordEnd();rit++){
    if((*rit)->getName()=="PixModuleGroup"){
      for(PixLib::recordIterator mit=(*rit)->recordBegin(); mit!=(*rit)->recordEnd();mit++){
	if((*mit)->getName()=="PixModule"){
	  std::string modName = (*mit)->getDecName();
	  PixLib::getDecNameCore(modName);
	  ModuleItem *newMod = new ModuleItem(this, (*rit), m_fname.c_str(), modName.c_str());
	  if(firstMod){ // add item for scan cfg. with Config object of first module
	    ModuleAction *act = new ModuleAction(info, newMod->getData(), (11+(int)PixLib::PixScan::MAX_HISTO_TYPES));
	    act->setText(0, QString("Show Scan Config."));
	    firstMod = false;
	  }
	}
      }
    }
  }
}
