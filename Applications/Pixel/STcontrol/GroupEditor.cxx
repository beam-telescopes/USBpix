#include <PixController/PixScan.h>
#include "GroupEditor.h"
#include "STRodCrate.h"
#include "STControlEngine.h"
#include "OptionsPanel.h"

#include <GeneralDBfunctions.h>

#include <PixModule/PixModule.h>
#include <PixController/PixController.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>

#include <QTableWidget>
#include <QString>
#include <QClipboard>
#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>
#include <QLayout>
#include <QLineEdit>
#include <QComboBox>

#include <QKeySequence>

#include <stdio.h>

GroupEditor::GroupEditor( STControlEngine &in_engine, int iCrate, int iGrp, QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent,fl), m_engine(in_engine), m_grpId(iGrp)
{
  setupUi(this);

  m_crate = m_engine.getSTRodCrates()[iCrate];

  PP0Box->clear();

  moduleTable->setColumnWidth(0,90);
  moduleTable->setColumnWidth(1,75);
  moduleTable->setColumnWidth(2,75);
  moduleTable->setColumnWidth(3,70);
  moduleTable->setColumnWidth(4,75);
  moduleTable->setColumnWidth(5,55);
  moduleTable->setColumnWidth(6,60);
  for(int i=7;i<moduleTable->columnCount();i++)
    moduleTable->setColumnWidth(i,75);
  setupTable();

  groupLabel->setText(("Configuration of module group "+m_crate->getGrpName(m_grpId)).c_str());

  m_pasteAction = new QAction("paste",this);
  m_pasteAction->setShortcut(QKeySequence("Ctrl+V"));
  m_copyAction  = new QAction("copy",this);
  m_copyAction->setShortcut(QKeySequence("Ctrl+C"));

  QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
  QObject::connect(moduleTable, SIGNAL(cellChanged(int,int)), this, SLOT(setEdited(int,int)));
  QObject::connect(m_copyAction,SIGNAL(triggered()),this,SLOT(copyToCB()));
  QObject::connect(m_pasteAction,SIGNAL(triggered()),this,SLOT(pasteFromCB()));

  // deal with the group config via the default panel
  m_op = new optionsPanel(m_crate->getGrpConf(m_grpId),this,0,true);
  // display optionPanel's tabs in our own display
  for (int i=0; i<m_op->m_tabwidget->count();i++)
    tabWidget->addTab( m_op->m_tabwidget->widget(i), m_op->m_tabwidget->tabText(i));
  // connect change of PP0 type to re-calculation of links
  connect( PP0Box, SIGNAL( activated(int) ), this, SLOT( setAllLinks() ) );
}
GroupEditor::~GroupEditor()
{
}
void GroupEditor::saveConfig()
{
  m_op->save();
  for(std::vector<int>::iterator it=m_edited.begin(); it!=m_edited.end(); it++){
    Config &conf = m_crate->getPixModuleConf(m_grpId, (*it));
    //m_modules[(*it)+1]->config();
    // write group ID
    bool isOK;
    int value = moduleTable->item((*it),3)->text().toInt(&isOK);
    if(conf["general"]["GroupId"].name()!="__TrashConfObj__" && isOK)
      *((int *)((ConfInt&)conf["general"]["GroupId"]).m_value) = value;
    value = moduleTable->item((*it),4)->text().toInt(&isOK);
    if(conf["general"]["ModuleId"].name()!="__TrashConfObj__" && isOK)
      *((int *)((ConfInt&)conf["general"]["ModuleId"]).m_value) = value;
    value = moduleTable->item((*it),5)->text().toInt(&isOK);
    if(conf["pp0"]["position"].name()!="__TrashConfObj__" && isOK)
      *((int *)((ConfInt&)conf["pp0"]["position"]).m_value) = value;
    value = PP0Box->currentIndex();
    if(value<=2)
      *((int *)((ConfInt&)conf["pp0"]["Type"]).m_value) = value;
    value = moduleTable->item((*it),6)->text().toInt(&isOK);
    if(conf["general"]["InputLink"].name()!="__TrashConfObj__" && isOK)
      *((int *)((ConfInt&)conf["general"]["InputLink"]).m_value) = value;
    for(int j=1;j<5;j++){
      char llabel[30];
      sprintf(llabel,"OutputLink%d",j);
      value = 16*(moduleTable->item((*it),6+j)->text().left(1).toInt(&isOK))+
	moduleTable->item((*it),6+j)->text().right(1).toInt(&isOK);
      //16*(moduleTable->text((*it),6+j).left(1).toInt(&isOK))+tmpStrg.toInt(&isOK)
      if(conf["general"][llabel].name()!="__TrashConfObj__" && isOK)
	*((int *)((ConfInt&)conf["general"][llabel]).m_value) = value;
    }
    if(moduleTable->item((*it),1)->text().isEmpty() && conf["geometry"]["Type"].name()!="__TrashConfObj__"){
      value = 0;
      *((int *)((ConfInt&)conf["geometry"]["Type"]).m_value) = value;
      if(conf["geometry"]["staveID"].name()!="__TrashConfObj__")
	*((int *)((ConfInt&)conf["geometry"]["staveID"]).m_value) = value; 
      if(conf["geometry"]["position"].name()!="__TrashConfObj__")
	*((int *)((ConfInt&)conf["geometry"]["position"]).m_value) = value;
    } else {
      int aSN = moduleTable->item((*it),1)->text().toInt(&isOK);
      if(isOK){
	if(aSN<9000)
	  value = 1; // stave
	else
	  value = 2; // sector
	*((int *)((ConfInt&)conf["geometry"]["Type"]).m_value) = value;
	if(conf["geometry"]["position"].name()!="__TrashConfObj__")
	  *((int *)((ConfInt&)conf["geometry"]["staveID"]).m_value) = aSN;
	value = moduleTable->item((*it),2)->text().toInt(&isOK);
	if((conf["geometry"]["position"].name()!="__TrashConfObj__") && isOK)
	  *((int *)((ConfInt&)conf["geometry"]["position"]).m_value) = value;
      }
    }
  }
  accept();
}
void GroupEditor::setupTable()
{
  std::vector< std::pair<std::string, int> > list;
  m_crate->getModules(m_grpId, list);
  int allPP0 = -1;
  moduleTable->setRowCount((int)list.size());
  for(int iMod=0; iMod<(int)list.size(); iMod++){
    for(int iCol=0; iCol<moduleTable->columnCount();iCol++)
      moduleTable->setItem(iMod,iCol,new QTableWidgetItem("???"));
    std::string mname = m_crate->getModName(m_grpId, iMod);
    getDecNameCore(mname);
    moduleTable->item(iMod,0)->setText(mname.c_str());
    Config &conf = m_crate->getPixModuleConf(m_grpId, iMod);
    // assembly info
    if(conf["geometry"]["Type"].name()!="__TrashConfObj__"){
      int assy_type = ((ConfInt&)conf["geometry"]["Type"]).getValue();
      if(assy_type>0){
	if(conf["geometry"]["staveID"].name()!="__TrashConfObj__")
	  moduleTable->item(iMod,1)->setText(QString::number(((ConfInt&)conf["geometry"]["staveID"]).getValue()));
	if(conf["geometry"]["position"].name()!="__TrashConfObj__")
	  moduleTable->item(iMod,2)->setText(QString::number(((ConfInt&)conf["geometry"]["position"]).getValue()));
      }
    }
    // read group ID
    if(conf["general"]["GroupId"].name()!="__TrashConfObj__")
      moduleTable->item(iMod,3)->setText(QString::number(((ConfInt&)conf["general"]["GroupId"]).getValue()));
    // read module ID
    if(conf["general"]["ModuleId"].name()!="__TrashConfObj__")
      moduleTable->item(iMod,4)->setText(QString::number(((ConfInt&)conf["general"]["ModuleId"]).getValue()));
    // read PP0-slot
    if(conf["pp0"]["position"].name()!="__TrashConfObj__")
      moduleTable->item(iMod,5)->setText(QString::number(((ConfInt&)conf["pp0"]["position"]).getValue()));
    if(PP0Box->count()==0){ // fill PP0 type box
      std::map<std::string, int> syms = ((ConfList&)conf["pp0"]["Type"]).symbols();
      for(std::map<std::string, int>::iterator it = syms.begin(); it!=syms.end(); it++)
	PP0Box->addItem("dummy"+QString::number(it->second));
      for(std::map<std::string, int>::iterator it = syms.begin(); it!=syms.end(); it++){
	PP0Box->setCurrentIndex(it->second);
	PP0Box->setItemText(it->second, (it->first).c_str());
      }
      PP0Box->addItem("keep module settings");
    }
    int pp0 = ((ConfInt&)conf["pp0"]["Type"]).getValue();
    if(allPP0<0) allPP0 = pp0;
    else if(allPP0!=pp0) allPP0=3;
    // read links
    if(conf["general"]["InputLink"].name()!="__TrashConfObj__")
      moduleTable->item(iMod,6)->setText(QString::number(((ConfInt&)conf["general"]["InputLink"]).getValue()));
    for(int j=1;j<5;j++){
      char llabel[30];
      sprintf(llabel,"OutputLink%d",j);
      if(conf["general"][llabel].name()!="__TrashConfObj__"){
	int olink = ((ConfInt&)conf["general"][llabel]).getValue();
	moduleTable->item(iMod,6+j)->setText(QString::number(olink/16)+":"+QString::number(olink%16));
      }
    }
  }
  PP0Box->setCurrentIndex(allPP0);
  for(int iRow=0; iRow<moduleTable->rowCount();iRow++){
    Qt::ItemFlags flags = moduleTable->item(iRow,0)->flags();
    flags &= ~Qt::ItemIsEnabled;
    moduleTable->item(iRow,0)->setFlags(flags);
  }
}
void GroupEditor::setEdited(int row, int col)
{
  if(col==5){
    Config &modConf = m_crate->getPixModuleConf(m_grpId, row+1);
    int pp0 = PP0Box->currentIndex();
    if(pp0>2) pp0 = ((ConfList&)modConf["pp0"]["Type"]).getValue();
    Config &cfgROD = m_crate->getPixCtrlConf(m_grpId);
    int type = ((ConfInt&)cfgROD["fmt"]["linkMap_0"]).getValue() & 0x3;

    std::vector<int> in, out1, out2, out3, out4;
    
    PixLib::listLinks(in,out1,out2,out3,out4,type, pp0);
    
    int slot = moduleTable->item(row,5)->text().toInt();

    if(slot>0 && slot<29){
      moduleTable->item(row, 6)->setText(QString::number(in.at(slot-1)));
      moduleTable->item(row, 7)->setText(QString::number(out1.at(slot-1)/16)+":"+QString::number(out1.at(slot-1)%16));
      moduleTable->item(row, 8)->setText(QString::number(out2.at(slot-1)/16)+":"+QString::number(out2.at(slot-1)%16));
      moduleTable->item(row, 9)->setText(QString::number(out3.at(slot-1)/16)+":"+QString::number(out3.at(slot-1)%16));
      moduleTable->item(row,10)->setText(QString::number(out4.at(slot-1)/16)+":"+QString::number(out4.at(slot-1)%16)); 
	//      }
    } else{
      moduleTable->item(row, 6)->setText("");
      moduleTable->item(row, 7)->setText(""); 
      moduleTable->item(row, 8)->setText("");
      moduleTable->item(row, 9)->setText("");
      moduleTable->item(row,10)->setText("");
    }
    
  }

  // rest (forgotten what)

  for(std::vector<int>::iterator it=m_edited.begin(); it!=m_edited.end(); it++)
    if((*it)==row) return;
  m_edited.push_back(row);
  //  m_editedMod.push_back(m_modules[row+1]);
}
void GroupEditor::copyToCB()
{
  int i,j, ncsel;
  QString cbText="";
  for(i=0;i<moduleTable->rowCount();i++){
    ncsel=0;
    for(j=0;j<moduleTable->columnCount();j++){
      if(moduleTable->item(i,j)->isSelected()){
	cbText+=moduleTable->item(i,j)->text()+"\t";
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
void GroupEditor::pasteFromCB()
{
  int row=-1, col=-1;
  for(int i=0;i<moduleTable->rowCount();i++){
    for(int j=0;j<moduleTable->columnCount();j++){
      if(moduleTable->item(i,j)->isSelected()){
	row = i;
	col = j;
	break;
      }
    }
  }
  if(row<0 || col<0){
    QMessageBox::warning(this,"STControl GroupEditor::pasteFromCB()",
			 "No table cell was selected, can't paste.");
    return;
  }
  QClipboard *cb = QApplication::clipboard();
  QString rowText,cellText,cbText=cb->text();
  int pos1=0, pos2=0;
  int currRow = row, currCol = col;
  // pick CB text into lines - 1 line per table row
  while(pos1>=0){
    pos1 = cbText.indexOf("\n");
    if(pos1>=0){
      rowText = cbText.left(pos1);
      pos2 = 0;
      currCol = col;
      // pick row text into tab-separated cell entries
      while(pos2>=0){
	pos2 = rowText.indexOf("\t");
	if(pos2>=0){
	  cellText = rowText.left(pos2);
	  rowText.remove(0,pos2+1);
	  if(currCol<moduleTable->columnCount() && currRow<moduleTable->rowCount())
	    moduleTable->item(currRow, currCol)->setText(cellText);
	  currCol++;
	}
      }
      // last part of line has not tab at the end, must paste explicitly
      if(currCol<moduleTable->columnCount() && currRow<moduleTable->rowCount())
	moduleTable->item(currRow, currCol)->setText(cellText);
      // remove processed line from CB text
      cbText.remove(0,pos1+1);
      currRow++;
    }
  }
}
void GroupEditor::setAllLinks()
{
  if(PP0Box->currentText()!="keep module settings"){
    for(int i=0;i<moduleTable->rowCount();i++){
      setEdited(i, 5);
    }
  }
}
