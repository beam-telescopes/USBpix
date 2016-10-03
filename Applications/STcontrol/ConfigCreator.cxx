#include "ConfigCreator.h"
#include "ConfigCreatorHelper.h"

#include <PixModuleGroup/PixModuleGroup.h>
#include <PixController/PixController.h>
#include <PixModule/PixModule.h>
#include <PixFe/PixFe.h>
#include <PixDcs/PixDcs.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <DBEdtEngine.h>
#include <GeneralDBfunctions.h>

#include <QComboBox>
#include <QVariant>
#include <QSpinBox>
#include <QListWidget>
#include <QRadioButton>
#include <QFileDialog>

ConfigCreator::ConfigCreator(QWidget *parent, Qt::WindowFlags f) 
  : QDialog(parent, f) {

  setupUi(this);

  // add controller types
  std::vector<PixControllerInfo> pclist;
  PixLib::PixController::listTypes(pclist);
  for(std::vector<PixControllerInfo>::iterator it=pclist.begin(); it!=pclist.end(); it++){
    QList<QVariant> vlist;
    QVariant ctrlClass(QString(it->className.c_str()));
    vlist.append(ctrlClass);
    QVariant extraOpt(it->extraOption);
    vlist.append(extraOpt);
    controllerTypeBox->addItem(QString(it->decName.c_str()), QVariant(vlist));
  }
  
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
  connect(FEcfgTypeBox, SIGNAL(activated(int)), this, SLOT(cfgLoadSelect(int)));
  connect(cfgBrowseButton, SIGNAL(pressed()), this, SLOT(browseConfigFile()));
  connect(rootModCfgName, SIGNAL(activated(int)), this, SLOT(setFromFile(int)));
  connect(mnameList, SIGNAL(currentRowChanged(int)), this, SLOT(setDbInfo(int)));
  connect(nfeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setFecombMax(int)));
  connect(iFeComb, SIGNAL(valueChanged(int)), this, SLOT(selectFileDisp(int)));
  connect(controllerTypeBox, SIGNAL(activated(const QString &)), this, SLOT(ctrlTypeSelected(const QString &)));

  mnameList->setCurrentRow(0);
  cfgLoadSelect(0);
  ctrlTypeSelected(controllerTypeBox->currentText());
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
  for(unsigned int i=0;i<m_dcs.size(); i++){
    delete m_dcs[i]; m_dcs[i]=0;
  }
  m_dcs.clear();
}

void ConfigCreator::createCfg(bool finished){
  std::vector<std::string> mnames;
  for(int i=0;i<nmodSpinBox->value(); i++) 
    mnames.push_back(mnameList->item(i)->text().toLatin1().data());

  QList<QVariant> vlist = controllerTypeBox->currentData().toList();

  // create module group from default constructor
  PixLib::PixModuleGroup *pmg = new PixLib::PixModuleGroup(std::string(groupNameEdit->text().toLatin1().data()),
							   std::string(vlist.at(0).toString().toLatin1().data()), mnames, 
							   std::string(feTypeBox->currentText().toLatin1().data()), nfeSpinBox->value(), 
							   nfeRowSpinBox->value(), std::string (mccTypeBox->currentText().toLatin1().data()),
							   vlist.at(1).toInt());

  // add individual modules to group as per chip layout requested
  if(FEcfgTypeBox->currentIndex()>0){
    for(int i=0;i<nmodSpinBox->value(); i++) {
		PixLib::PixModule *mod = pmg->module(i);
		if(mod!=0){
			unsigned int jmax = m_dbFnames[i].size();
			if(FEcfgTypeBox->currentIndex()==1 && jmax>1){
				jmax = 1;
				std::cout << "Inconsistent FE file reading mode information for module " << i << std::endl;
			}
			// remember for later - will be overwritten when reading from DB, but we need this group's IDs
			int mid = mod->moduleId();
			int gid = mod->groupId();
			for(unsigned int j = 0; j<jmax; j++){
				// open and read from DB file
				PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(m_dbFnames[i][j].toLatin1().data(), false); 
				DBInquire *root = confDBInterface->readRootRecord(1);
				std::vector<DBInquire*> inqVec;
				try{
				  inqVec = confDBInterface->DBFindRecordByName(PixLib::BYDECNAME, root->getDecName()+
										   std::string(m_dbMnames[i][j].toLatin1().data())+"/PixModule");
				}catch(...){
				  inqVec.clear();
				}
				if(inqVec.size()==1){
					if(FEcfgTypeBox->currentIndex()==1){
						// read DB info into module config.
						mod->config().read(inqVec[0]);
						// problem with old cfg. files: MCC_I2 set for 1 FE_I2, which is wrong
						if(mod->getMCCFlavour()==PixModule::PM_MCC_I2 && mod->getFEFlavour()==PixModule::PM_FE_I2 &&
							mod->getFECount()==1){
							PixLib::ConfList mccFlv = (ConfList&)mod->config()["general"]["MCC_Flavour"];
							mccFlv.setValue((int)PixModule::PM_NO_MCC);
						}
					} else if(FEcfgTypeBox->currentIndex()==2){
						for(recordIterator feIter = inqVec[0]->recordBegin();feIter!=inqVec[0]->recordEnd();feIter++){
							if((*feIter)->getName().find("PixFe")!=std::string::npos){
								// read DB info into module's j-th FE config.
								mod->pixFE(j)->config().read(*feIter);
							}
						}
					}
				} else{
					std::cout << "Can't find module " << std::string(m_dbMnames[i][j].toLatin1().data()) << " in file " << 
								std::string(m_dbFnames[i][j].toLatin1().data()) << std::endl;
				}
				delete confDBInterface; //closes file// restore IDs
				((ConfInt&)mod->config()["general"]["ModuleId"]).setValue(mid);
				((ConfInt&)mod->config()["general"]["GroupId"]).setValue(gid);
			}
		}
	}
  }

  // polish and store configuration
  m_pmg.push_back(pmg);
  pmg->config().addConfig(&(pmg->getPixController()->config()));
  for(PixLib::PixModuleGroup::moduleIterator it = pmg->modBegin(); it!=pmg->modEnd(); it++)
    pmg->config().addConfig(&((*it)->config()));

  // create regulators as PixDcs device if requested
  if(addUsbDcs->isChecked()){
    std::string dcsType = "";
    switch(vlist.at(1).toInt()){
    case 0:
      dcsType = "USBPixDcs";
      break;
    case 1:
    case 3:
      dcsType = "USBBIPixDcs";
      break;
    case 2:
      dcsType = "USBGpacPixDcs";
      break;
    default:
      dcsType = "";
    }	    
    if(dcsType != ""){
      // new DCS object for regulators
      PixDcs *dcs = PixDcs::make((DBInquire*) 0, (void*) pmg->getPixController(), dcsType);
      Config &conf = dcs->config();
      std::stringstream a;
      a << m_dcs.size();
      ((ConfString&)conf["general"]["DeviceName"]).m_value = ("USB regulators "+a.str());
      ((ConfInt&)conf["general"]["Index"]).setValue(m_dcs.size());
      m_dcs.push_back(dcs);
    }
  }
 
  if(finished) accept();
  else{
    //controllerTypeBox->setEnabled(false);
    groupNameEdit->setText("ModuleGroup_"+QString::number(m_pmg.size()));
    mnameList->clear();
    m_dbFnames.clear();
    m_dbMnames.clear();
    mnameListSetup(1);
    mnameList->setCurrentRow(0);
    FEcfgTypeBox->setCurrentIndex(0);
    cfgLoadSelect(0);
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
void ConfigCreator::cfgLoadSelect(int type){
  switch(type){
  case 0: // generate from scratch
  default:
    modFileLabel->hide();
    rootCfgFile->hide();
    cfgBrowseButton->hide();
    rootModCfgName->hide();
    modCfgLabel->hide();
    flavourGroupBox->show();
    nfeSpinBox->setEnabled(1);
    nfeRowSpinBox->setEnabled(1);
    mccTypeBox->setEnabled(1);
    feTypeBox->setEnabled(1);
	feCombLabel->hide();
	iFeComb->hide();
	iFeComb->setMaximum(0);
    break;
  case 1: // load from file
    modFileLabel->show();
    rootCfgFile->show();
    cfgBrowseButton->show();
    rootModCfgName->show();
    modCfgLabel->show();
    flavourGroupBox->hide();
    nfeSpinBox->setEnabled(0);
    nfeRowSpinBox->setEnabled(0);
    mccTypeBox->setEnabled(0);
    feTypeBox->setEnabled(0);
	feCombLabel->hide();
	iFeComb->hide();
	iFeComb->setMaximum(0);
    break;
  case 2: // load N FE from file and combine
    modFileLabel->show();
    rootCfgFile->show();
    cfgBrowseButton->show();
    rootModCfgName->show();
    modCfgLabel->show();
    flavourGroupBox->hide();
    nfeSpinBox->setEnabled(1);
    nfeRowSpinBox->setEnabled(1);
    mccTypeBox->setEnabled(0);
    feTypeBox->setEnabled(0);
	feCombLabel->show();
	iFeComb->show();
	iFeComb->setMaximum(nfeSpinBox->value()-1);
    break;
  }
}
void ConfigCreator::browseConfigFile(){
  QString cfgName="";

  std::string defPath = ".";
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList cfgFilter;
  cfgFilter += "RootDB config file (*.cfg.root)";
  cfgFilter += "Any file (*.*)";
  QFileDialog CfgDia(this, "Specify name of RootDB cfg-file", qpath);
#if defined(QT5_FIX_QDIALOG)
  CfgDia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  CfgDia.setNameFilters(cfgFilter);
  CfgDia.setFileMode(QFileDialog::ExistingFile);
  if(CfgDia.exec() == QDialog::Accepted){
    cfgName = CfgDia.selectedFiles().at(0);
    cfgName.replace("\\", "/");

    rootCfgFile->setText(cfgName);
	if(m_dbFnames.find(mnameList->currentRow())==m_dbFnames.end()){ // first time this module stores information
		std::vector<QString> emptyNames;
		emptyNames.resize(nfeSpinBox->value());
		m_dbFnames[mnameList->currentRow()] = emptyNames;
		m_dbMnames[mnameList->currentRow()] = emptyNames;
	} else if((int)m_dbFnames[mnameList->currentRow()].size()!=nfeSpinBox->value()){
		m_dbFnames[mnameList->currentRow()].resize(nfeSpinBox->value());
		m_dbMnames[mnameList->currentRow()].resize(nfeSpinBox->value());
	}
    m_dbFnames[mnameList->currentRow()][iFeComb->value()] = cfgName;

    rootModCfgName->clear();
    std::vector<std::string> mnames;
    ConfigCreatorHelper::listModuleNames(std::string(cfgName.toLatin1().data()), mnames);
    for(std::vector<std::string>::iterator it = mnames.begin(); it!=mnames.end(); it++){
      QVariant vdn(QString(it->c_str()));
      rootModCfgName->addItem(it->c_str(), vdn);
    }
  }
  setFromFile(0);
}
void ConfigCreator::setFromFile(int modIt){
  m_dbMnames[mnameList->currentRow()][iFeComb->value()] = rootModCfgName->itemData(rootModCfgName->currentIndex()).toString();
  PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(rootCfgFile->text().toLatin1().data(), false); 
  DBInquire *root = confDBInterface->readRootRecord(1);
  std::vector<DBInquire*> inqVec;
  try{
    inqVec = confDBInterface->DBFindRecordByName(PixLib::BYDECNAME, root->getDecName()+
						 std::string(rootModCfgName->itemData(modIt).toString().toLatin1().data())+"/PixModule");
  }catch(...){
    inqVec.clear();
  }
  if(inqVec.size()==1){
    PixModule mod(inqVec[0], 0, "tmpmod");
    // problem with old cfg. files: MCC_I2 set for 1 FE_I2, which is wrong
    if(mod.getMCCFlavour()==PixModule::PM_MCC_I2 && mod.getFEFlavour()==PixModule::PM_FE_I2 &&
       mod.getFECount()==1){
      PixLib::ConfList mccFlv = (ConfList&)mod.config()["general"]["MCC_Flavour"];
      mccFlv.setValue((int)PixModule::PM_NO_MCC);
    }
	if(FEcfgTypeBox->currentIndex()==1){ 
		nfeSpinBox->setValue(mod.getFECount());
		nfeRowSpinBox->setValue((mod.nRowsMod()/mod.nRowsFe()));
		QVariant mccData((int)mod.getMCCFlavour());
		mccTypeBox->setCurrentIndex(mccTypeBox->findData(mccData));
	}
    QVariant feData((int)mod.getFEFlavour());
    feTypeBox->setCurrentIndex(feTypeBox->findData(feData));
  }
  
  delete confDBInterface; //closes file
}

void ConfigCreator::setDbInfo(int modListRow){
  rootModCfgName->clear();
  rootCfgFile->setText("");
  if(m_dbFnames.find(modListRow)!=m_dbFnames.end()){
    rootCfgFile->setText(m_dbFnames[modListRow][iFeComb->value()]);
    if(m_dbFnames[modListRow][iFeComb->value()]!=""){
      try{
	PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(m_dbFnames[modListRow][iFeComb->value()].toLatin1().data(), false); 
	DBInquire *root = confDBInterface->readRootRecord(1);
	for(recordIterator appIter = root->recordBegin();appIter!=root->recordEnd();appIter++){
	  if((int)(*appIter)->getName().find("application")!=(int)std::string::npos){
	    // loop over inquires in crate inquire and create a PixModuleGroup when an according entry is found
	    for(recordIterator pmgIter = (*appIter)->recordBegin();pmgIter!=(*appIter)->recordEnd();pmgIter++){
	      if((*pmgIter)->getName().find("PixModuleGroup")!=std::string::npos){
		for(recordIterator pmIter = (*pmgIter)->recordBegin();pmIter!=(*pmgIter)->recordEnd();pmIter++){
		  if((*pmIter)->getName().find("PixModule")!=std::string::npos){
		    std::string modName = (*pmIter)->getDecName();
		    QVariant vdn(QString(modName.c_str()));
		    getDecNameCore(modName);
		    rootModCfgName->addItem(modName.c_str(), vdn);
		  }
		}
	      }
	    }
	  }
	}
	delete confDBInterface; //closes file
      }catch(...){}
    }
    int imcn = rootModCfgName->findData(m_dbMnames[modListRow][iFeComb->value()]);
    if(imcn>=0){
		disconnect(rootModCfgName, SIGNAL(activated(int)), this, SLOT(setFromFile(int)));
		rootModCfgName->setCurrentIndex(imcn);
		connect(rootModCfgName, SIGNAL(activated(int)), this, SLOT(setFromFile(int)));
	}
  }
}
void ConfigCreator::selectFileDisp(int){
	setDbInfo(mnameList->currentRow());
}
void ConfigCreator::setFecombMax(int val){
	iFeComb->setMaximum(val);
}
void ConfigCreator::ctrlTypeSelected(const QString & text){
	// enable USB regulator creation tick box if adapter card is part of controller
	bool hasRegs = (((text.indexOf("SCA")!=-1)||(text.indexOf("BIC")!=-1)||(text.indexOf("GPAC")!=-1))&&text.indexOf("FE-I4")!=-1);
	addUsbDcs->setEnabled(hasRegs);
	addUsbDcs->setChecked(hasRegs);
}
