#include "MultiboardCfg.h"
#include "STControlEngine.h"
#include <SiLibUSB.h>
#include <Config/Config.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <DBEdtEngine.h>
#include <GeneralDBfunctions.h>
#include <PixController.h>

#include <QComboBox>
#include <QVariant>
#include <QFileDialog>
#include <QGroupBox>

#include <TSystem.h>

#include <stdio.h>

MultiboardCfg::MultiboardCfg(STControlEngine &engine_in, QWidget* parent, Qt::WindowFlags fl) : 
  QDialog(parent, fl), m_engine(engine_in){
  setupUi(this);

  QObject::connect(finishButton, SIGNAL(clicked()), this, SLOT(acceptCfg()));
  QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(nextBoard()));
  QObject::connect(cfgBrowseButton0, SIGNAL(clicked()), this, SLOT(browseFeConfig0()));
  QObject::connect(cfgBrowseButton0_1, SIGNAL(clicked()), this, SLOT(browseFeConfig4()));
  QObject::connect(cfgBrowseButton0_2, SIGNAL(clicked()), this, SLOT(browseFeConfig5()));
  QObject::connect(cfgBrowseButton0_3, SIGNAL(clicked()), this, SLOT(browseFeConfig6()));
  QObject::connect(cfgBrowseButton1, SIGNAL(clicked()), this, SLOT(browseFeConfig1()));
  QObject::connect(cfgBrowseButton2, SIGNAL(clicked()), this, SLOT(browseFeConfig2()));
  QObject::connect(cfgBrowseButton3, SIGNAL(clicked()), this, SLOT(browseFeConfig3()));
  QObject::connect(fpgaBrowseButton, SIGNAL(clicked()), this, SLOT(browsefpgaFW()));
  QObject::connect(uCBrowseButton, SIGNAL(clicked()), this, SLOT(browseuCFW()));
  QObject::connect(FEcfgTypeBox, SIGNAL(activated(int)), this, SLOT(setCfgBrws()));
  QObject::connect(FEcfgTypeBox, SIGNAL(activated(int)), this, SLOT(enaButtons()));
  QObject::connect(feCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateDevList()));
  QObject::connect(feCheckBox, SIGNAL(toggled(bool)), this, SLOT(set2FeMode()));
  QObject::connect(boardComboBox, SIGNAL(activated(int)), this, SLOT(set2FeMode()));
  QObject::connect(modIdBox, SIGNAL(valueChanged(int)), this, SLOT(selMod(int)));
  QObject::connect(biAdapter, SIGNAL(activated(int)), this, SLOT(set2FeMode()));
  QObject::connect(biAdapter, SIGNAL(activated(int)), this, SLOT(setSelMod(int)));
  QObject::connect(biAdapter, SIGNAL(activated(int)), this, SLOT(enaButtons()));
  QObject::connect(cfgDcsBrowseButton, SIGNAL(clicked()), this, SLOT(browseDcsCfg()));
  QObject::connect(bi_nFEs, SIGNAL(valueChanged(int)), this, SLOT(setCfgBrws()));

  m_feName[0] = feName0;
  m_feName[1] = feName1;
  m_feName[2] = feName2;
  m_feName[3] = feName3;
  m_rootCfgFile[0] = rootCfgFile0;
  m_rootCfgFile[1] = rootCfgFile1;
  m_rootCfgFile[2] = rootCfgFile2;
  m_rootCfgFile[3] = rootCfgFile3;
  m_rootCfgFile[4] = rootCfgFile0_1;
  m_rootCfgFile[5] = rootCfgFile0_2;
  m_rootCfgFile[6] = rootCfgFile0_3;
  m_rootFeCfgName[0] = rootFeCfgName0;
  m_rootFeCfgName[1] = rootFeCfgName1;
  m_rootFeCfgName[2] = rootFeCfgName2;
  m_rootFeCfgName[3] = rootFeCfgName3;
  m_rootFeCfgName[4] = rootFeCfgName0_1;
  m_rootFeCfgName[5] = rootFeCfgName0_2;
  m_rootFeCfgName[6] = rootFeCfgName0_3;
  m_modFileLabel[0] = modFileLabel0;
  m_modFileLabel[1] = modFileLabel0_1;
  m_modFileLabel[2] = modFileLabel0_2;
  m_modFileLabel[3] = modFileLabel0_3;
  m_modCfgLabel[0] = modCfgLabel0;
  m_modCfgLabel[1] = modCfgLabel0_1;
  m_modCfgLabel[2] = modCfgLabel0_2;
  m_modCfgLabel[3] = modCfgLabel0_3;
  m_cfgBrowseButton[0] = cfgBrowseButton0;
  m_cfgBrowseButton[1] = cfgBrowseButton0_1;
  m_cfgBrowseButton[2] = cfgBrowseButton0_2;
  m_cfgBrowseButton[3] = cfgBrowseButton0_3;
  m_modGrp[0] = mod0GroupBox;
  m_modGrp[1] = mod1GroupBox;
  m_modGrp[2] = mod2GroupBox;
  m_modGrp[3] = mod3GroupBox;

  InitUSB();
  set2FeMode();
  setSelMod(biAdapter->currentIndex());
  updateDevList();
  selMod(modIdBox->value());
  resize(650,450);

  // need $PIX_LIB to find base config files
  QString basePath = gSystem->Getenv("PIX_LIB");
  if(basePath!=""){
    QDir baseDir(basePath);
    for(int i=0;i<3;i++) baseDir.cdUp();
    basePath = baseDir.path();
    fpgaFwFile->setText(basePath+"/config/usbpixi4.bit");
    uCFwFile->setText("");//basePath+"/config/usbpix.bix");
    //    browseFeConfig(basePath+"/config/std.cfg.root");
  }

  for(int i=0;i<4;i++)
    m_feName[i]->setText("SC"+QString::number(i+1));
}
MultiboardCfg::~MultiboardCfg(){
}
void MultiboardCfg::updateDevList(){
  boardComboBox->clear();
  
  OnDeviceChange();

  // if first board, allow to use sigle USBPix cfg.
  if(m_cfgList.size()==0 && !feCheckBox->isChecked())
    boardComboBox->addItem("single board cfg. (ID=-1)", -1);

  SiUSBDevice tmpDev(NULL);
  for (int i = 0; i < GetMaxNumberOfUSBBoards(); i++)	{
    if (GetUSBDeviceIndexed(i) != 0){
      tmpDev.SetDeviceHandle(GetUSBDeviceIndexed(i));
      bool stillFree=true;
      for(std::vector<cfgInfo>::iterator it = m_cfgList.begin(); it!=m_cfgList.end(); it++){
	if((*it).boardID[0]==(int)tmpDev.GetId()){
	  stillFree = false;
	  break;
	}
      }
      if(stillFree){
	QString usblabel;
	usblabel  = tmpDev.GetName();
	usblabel += " - ID " + QString::number((int)tmpDev.GetId()) + " ";
	QVariant vid((int)tmpDev.GetId());
	boardComboBox->addItem(usblabel , vid);
      }
    }
  }
  enaButtons();
}
void MultiboardCfg::enaButtons(){
  warningLabel->setText(" ");
  bool enab = boardComboBox->count()>0 && (boardComboBox->count()>1 || !feCheckBox->isChecked());
  bool enab2 = true;
  if(!enab) warningLabel->setText("Can't find sufficient no. of USB boards -> can't proceed");
  else{
    if(FEcfgTypeBox->currentIndex()>1){
      int imod = 1;
      if(biAdapter->currentIndex()==1) imod = 4;
      for(int i=0;i<imod;i++)
	enab2 &= m_rootCfgFile[i]->text()!="";
      imod = 3;
      if(FEcfgTypeBox->currentIndex()>2 && biAdapter->currentIndex() >= 2) imod += bi_nFEs->value();
      for(int i=4;i<imod;i++)
	enab2 &= m_rootCfgFile[i]->text()!="";
    }
    if(!enab2) warningLabel->setText("Not all modules have a RootDB-cfg. file specified -> can't proceed");
  }
  finishButton->setEnabled(enab&&enab2);
  enab = boardComboBox->count()>((m_cfgList.size()==0)?2:1) && enab2;
  addButton->setEnabled(enab);
}
void MultiboardCfg::acceptCfg(){
  nextBoard();
  accept();
}
void MultiboardCfg::nextBoard(){
  cfgInfo mycfg;
  mycfg.boardID[0] = boardComboBox->itemData(boardComboBox->currentIndex()).toInt();
  mycfg.boardID[1] = -1;
  if(feCheckBox->isChecked()) 
    mycfg.boardID[1] = board2ComboBox->itemData(board2ComboBox->currentIndex()).toInt();
  mycfg.FEflavour = FEcfgTypeBox->currentIndex();
  mycfg.FEnum = (feCheckBox->isChecked()?2:1);
  mycfg.adapterType = biAdapter->currentIndex();
  if(biAdapter->currentIndex()>=2) mycfg.FEnum = bi_nFEs->value();
  
  int imod = 1;
  if(biAdapter->currentIndex()==1) imod = 4;
  for(int k=0;k<imod;k++){
    mycfg.newFeName.push_back(m_feName[k]->text());
    if(FEcfgTypeBox->currentIndex()>1){
      mycfg.fileName.push_back(m_rootCfgFile[k]->text());
      mycfg.modDecName.push_back(m_rootFeCfgName[k]->itemData(m_rootFeCfgName[k]->currentIndex()).toString());
    } else {
      mycfg.fileName.push_back("");
      mycfg.modDecName.push_back("");
    }
  }
  if(FEcfgTypeBox->currentIndex()==3){
    for(int i=4; i<(mycfg.FEnum+3); i++){
      mycfg.fileName.push_back(m_rootCfgFile[i]->text());
      mycfg.modDecName.push_back(m_rootFeCfgName[i]->itemData(m_rootFeCfgName[i]->currentIndex()).toString());
      mycfg.newFeName.push_back("dummy");
    }
  }

  m_cfgList.push_back(mycfg);

  uCFwFile->setEnabled(false);
  fpgaFwFile->setEnabled(false);
  fpgaBrowseButton->setEnabled(false);
  uCBrowseButton->setEnabled(false);

  for(int i=0;i<7;i++){
    m_rootFeCfgName[i]->clear();
    m_rootCfgFile[i]->setText("");
  }

  QString indstr = "2nd";
  if(m_cfgList.size()==2) indstr = "3rd";
  if(m_cfgList.size()>2) indstr = QString::number(m_cfgList.size()+1)+"th";
  topLabel->setText("Configuration for "+indstr+" USB board");
  int indOffs=0;
  for(unsigned int i=0;i<m_cfgList.size();i++)
    indOffs += (int)m_cfgList[i].fileName.size();
  for(int i=0;i<4;i++)
    m_feName[i]->setText("SC"+QString::number(i+indOffs+1));
  updateDevList();
}
void MultiboardCfg::browseFeConfig(const char *path_in, int modId){
  bool accepted=false;

  QString cfgName="";

  if(path_in!=0){
    cfgName = path_in;
    accepted = true;
  }else{
    std::string defPath = ".";
    if(!m_engine.plPath().isEmpty()){
      Config &opts = m_engine.getOptions();
      defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
    }
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
      accepted = true;
    }
  }

  if(accepted){
    m_rootCfgFile[modId]->setText(cfgName);
    enaButtons();

    m_rootFeCfgName[modId]->clear();
    PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(cfgName.toLatin1().data(), false); 
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
		m_rootFeCfgName[modId]->addItem(modName.c_str(), vdn);
	      }
	    }
	  }
	}
      }
    }
    delete confDBInterface; //closes file
  }
}
void MultiboardCfg::browseuCFW(){
  QString qpath = QString::null;
  if(uCFwFile->text()!="") qpath = uCFwFile->text();
  QFileDialog fdia(this, "Specify name of uC firmware file", qpath,"uC firmware file (*.bix);;Any file (*.*)");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().first();
    qpath.replace("\\", "/");
    uCFwFile->setText(qpath);
  }
}
void MultiboardCfg::browsefpgaFW(){
  QString qpath = QString::null;
  if(fpgaFwFile->text()!="") qpath = fpgaFwFile->text();
  QFileDialog fdia(this, "Specify name of FPGA firmware file", qpath,"FPGA firmware file (*.bit);;Any file (*.*)");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().first();
    qpath.replace("\\", "/");
    fpgaFwFile->setText(qpath);
  }
}
void MultiboardCfg::setCfgBrws(){
  int cfgType = FEcfgTypeBox->currentIndex();
  int imod = 1;
  if(biAdapter->currentIndex()==1) imod = 4;
  for(int i=0;i<imod;i++){
    m_rootCfgFile[i]->setEnabled(cfgType>1);
    m_rootFeCfgName[i]->setEnabled(cfgType>1);
  }
  cfgBrowseButton0->setEnabled(cfgType>1);
  cfgBrowseButton1->setEnabled(cfgType==2 && imod==4);
  cfgBrowseButton2->setEnabled(cfgType==2 && imod==4);
  cfgBrowseButton3->setEnabled(cfgType==2 && imod==4);
  int nFe=1;
  if(cfgType==3){
    m_modFileLabel[0]->setText("ROOT cfg. file FE0");
    nFe = 2;
    if(biAdapter->currentIndex()>=2)
      nFe = bi_nFEs->value();
  } else{
    m_modFileLabel[0]->setText("ROOT cfg. file");
  }
  for(int i=1;i<4;i++){
    if(i<nFe){
      m_rootCfgFile[i+3]->show();
      m_rootFeCfgName[i+3]->show();
      m_cfgBrowseButton[i]->show();
      m_modFileLabel[i]->show();
      m_modCfgLabel[i]->show();
    } else{
      m_rootCfgFile[i+3]->hide();
      m_rootFeCfgName[i+3]->hide();
      m_cfgBrowseButton[i]->hide();
      m_modFileLabel[i]->hide();
      m_modCfgLabel[i]->hide();
    }
  }
}
void MultiboardCfg::set2FeMode(){
  board2ComboBox->clear();
  board2ComboBox->setEnabled(feCheckBox->isChecked());
  FEcfgTypeBox->clear();
  FEcfgTypeBox->addItem("generate FE-I4A from scratch");
  FEcfgTypeBox->addItem("generate FE-I4B from scratch");
  FEcfgTypeBox->addItem("read from file (cfg. as is)");
  if(feCheckBox->isChecked() && biAdapter->currentIndex()<1)
    FEcfgTypeBox->addItem("read from file (2x1-FE -> 2-FE)");
  if((biAdapter->currentIndex()>=2))
    FEcfgTypeBox->addItem("read from file (nx1-FE -> n-FE)");
  if(feCheckBox->isChecked()){
    for(int ib1=0; ib1<boardComboBox->count(); ib1++){
      if(ib1!=boardComboBox->currentIndex())
	board2ComboBox->addItem( boardComboBox->itemText(ib1), boardComboBox->itemData(ib1));
    }
  }
  FEcfgTypeBox->setCurrentIndex(1);
  setCfgBrws();
  enaButtons();
}
void MultiboardCfg::selMod(int modID){
  for(int i=0;i<4;i++)
    if(i!=modID) m_modGrp[i]->hide();

  m_modGrp[modID]->show();
}
void MultiboardCfg::setSelMod(int biMode){
  switch (biMode)
  {
    case 4:
    case 0:
      modIdBox->setMaximum(0);
      feCheckBox->setEnabled(true);
      bi_nFEs->setEnabled(false);
      bi_nFEs->setValue(1);
      break;
    case 1:
      modIdBox->setMaximum(3);
      feCheckBox->setEnabled(true);
      bi_nFEs->setEnabled(false);
      bi_nFEs->setValue(1);
      break;
    case 2:
      modIdBox->setMaximum(0);
      feCheckBox->setEnabled(false);
      bi_nFEs->setEnabled(true);
      feCheckBox->setChecked(false);
      break;
    case 3:
      modIdBox->setMaximum(0);
      feCheckBox->setEnabled(false);
      bi_nFEs->setEnabled(true);
      break;
  }
}
void MultiboardCfg::browseDcsCfg(){
  QString qpath = QString::null;
  if(rootCfgFileDcs->text()!="") qpath = rootCfgFileDcs->text();
  QFileDialog fdia(this, "Specify name of DCS cfg. file", qpath,"RootDB config file (*.cfg.root);;Any file (*.*)");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().first();
    qpath.replace("\\", "/");
    rootCfgFileDcs->setText(qpath);
  }
}
