#include "ConfigCreator.h"

#include <PixModuleGroup/PixModuleGroup.h>
#include <PixController/PixController.h>
#include <PixModule/PixModule.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>

#include <QComboBox>
#include <QVariant>
#include <QSpinBox>
#include <QListWidget>
#include <QRadioButton>

ConfigCreator::ConfigCreator(QWidget *parent, Qt::WindowFlags f) 
  : QDialog(parent, f) {

  setupUi(this);

  // add controller types
  std::vector<std::string> pclist;
  PixLib::PixController::listTypes(pclist);
  for(std::vector<std::string>::iterator it=pclist.begin(); it!=pclist.end(); it++)
    controllerTypeBox->addItem(QString((*it).c_str()));

  // load list of FE and MCC flavours - make IBL-SCC (no MCC, 1x FE-I4B) default
  PixLib::PixModule mod(0,0,"dummy");
  PixLib::Config &modCfg = mod.config();
  PixLib::ConfList mccFlv = (ConfList&)modCfg["general"]["MCC_Flavour"];
  PixLib::ConfList feFlv  = (ConfList&)modCfg["general"]["FE_Flavour"];
  std::map<std::string, int> flvList = mccFlv.symbols();
  for(std::map<std::string, int>::iterator it=flvList.begin(); it!=flvList.end(); it++){
    mccTypeBox->addItem(QString(it->first.c_str()), QVariant(it->second));
    if(it->second==PixLib::PixModule::PM_NO_MCC) 
      mccTypeBox->setCurrentIndex(mccTypeBox->count()-1);
  }
  flvList = feFlv.symbols();
  for(std::map<std::string, int>::iterator it=flvList.begin(); it!=flvList.end(); it++){
    feTypeBox->addItem(QString(it->first.c_str()), QVariant(it->second));
    if(it->second==PixLib::PixModule::PM_FE_I4B) 
      feTypeBox->setCurrentIndex(feTypeBox->count()-1);
  }

  connect(finishButton, SIGNAL(pressed()), this, SLOT(createCfgDone()));
  connect(addButton, SIGNAL(pressed()), this, SLOT(createCfgNext()));
  connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));
  connect(nmodSpinBox, SIGNAL(valueChanged(int)), this, SLOT(mnameListSetup(int)));
  connect(presetButton, SIGNAL(pressed()), this, SLOT(preset()));
}

ConfigCreator::~ConfigCreator(){
  for(unsigned int i=0;i<m_pmg.size(); i++){
    // remove sub-configs, otherwise confusion about who is going to delete them
    m_pmg[i]->config().removeConfig(m_pmg[i]->getPixController()->config().name());
    for(PixLib::PixModuleGroup::moduleIterator it = m_pmg[i]->modBegin(); it!=m_pmg[i]->modEnd(); it++)
      m_pmg[i]->config().removeConfig((*it)->moduleName()+"/PixModule");
    delete m_pmg[i]; m_pmg[i]=0;
  }
  m_pmg.clear();
}

void ConfigCreator::createCfg(bool finished){
  std::vector<std::string> mnames;
  for(int i=0;i<nmodSpinBox->value(); i++) 
    mnames.push_back(mnameList->item(i)->text().toLatin1().data());

  PixLib::PixModuleGroup *pmg = new PixLib::PixModuleGroup(std::string(groupNameEdit->text().toLatin1().data()),
				     std::string(controllerTypeBox->currentText().toLatin1().data()), mnames, 
				     std::string(feTypeBox->currentText().toLatin1().data()), nfeSpinBox->value(), 
					     nfeRowSpinBox->value(), std::string (mccTypeBox->currentText().toLatin1().data()));
  m_pmg.push_back(pmg);
  pmg->config().addConfig(&(pmg->getPixController()->config()));
  for(PixLib::PixModuleGroup::moduleIterator it = pmg->modBegin(); it!=pmg->modEnd(); it++)
    pmg->config().addConfig(&((*it)->config()));
 
  if(finished) accept();
  else{
    controllerTypeBox->setEnabled(false);
    groupNameEdit->setText("ModuleGroup_"+QString::number(m_pmg.size()));
  }
}

void ConfigCreator::mnameListSetup(int nMod){
  if(nMod>mnameList->count()){
    for(int i=mnameList->count(); i<nMod; i++){
      mnameList->addItem("Module_"+QString::number(i));
      mnameList->item(i)->setFlags(mnameList->item(i)->flags() | Qt::ItemIsEditable);
    }
  }
  if(nMod<mnameList->count()){
    for(int i=(mnameList->count()-1); i>=nMod; i--){
      QListWidgetItem *item = mnameList->takeItem(i);
      delete item;
    }
  }
}
void ConfigCreator::preset(){
  int nFe, nFeRows, mccFlv, feFlv;
  nFe = 1;
  nFeRows = 1;
  mccFlv = PixLib::PixModule::PM_NO_MCC;
  feFlv = PixLib::PixModule::PM_FE_I4B;
  if(fei4Choice->isChecked()){
    nFe = 2;
    nFeRows = 1;
    mccFlv = PixLib::PixModule::PM_NO_MCC;
    feFlv = PixLib::PixModule::PM_FE_I4B;
  } else if(fei3Choice->isChecked()){
    nFe = 16;
    nFeRows = 2;
    mccFlv = PixLib::PixModule::PM_MCC_I2;
    feFlv = PixLib::PixModule::PM_FE_I2;
  }
  for(int i=0;i<mccTypeBox->count();i++){
    if(mccTypeBox->itemData(i).toInt()==mccFlv){
      mccTypeBox->setCurrentIndex(i);
      break;
    }
  }
  for(int i=0;i<feTypeBox->count();i++){
    if(feTypeBox->itemData(i).toInt()==feFlv){
      feTypeBox->setCurrentIndex(i);
      break;
    }
  }
  nfeSpinBox->setValue(nFe);
  nfeRowSpinBox->setValue(nFeRows);
}
