#include "WaferTestPanel.h"
#include "OptionsPanel.h"
#include "STControlEngine.h"
#include "PrimlistPanel.h"

#include <PixProber/PixProber.h>
#include <Config/Config.h>
#include <PixDcs/SleepWrapped.h>
#include <PixConfDBInterface/RootDB.h>
#include <GeneralDBfunctions.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <iostream>

using namespace PixLib;
using namespace std;

const bool STCONTROL_WAFERTESTPANEL_DEBUG = false;

WaferTestPanel::WaferTestPanel(STControlEngine &engine_in, QWidget* parent) :
  QWidget(parent), m_engine(engine_in)
{
  setupUi(this);

  m_prober = 0;
  m_nChips = 0;
  m_abortFlag = false;

  vector<string> prbList;
  PixProber::listTypes(prbList);
  for(vector<string>::iterator it=prbList.begin(); it!=prbList.end(); it++)
    proberTypesBox->addItem(it->c_str());

  QObject::connect(newEditButton, SIGNAL(clicked()), this, SLOT(newProber()));  
  QObject::connect(loadButton, SIGNAL(clicked()), this, SLOT(loadProber()));  
  QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(saveProber()));  
  QObject::connect(browseDirButton, SIGNAL(clicked()), this, SLOT(browseOutput()));  
  QObject::connect(indexLabelType, SIGNAL(currentIndexChanged(int)), this, SLOT(enableSDbox(int)));
  saveButton->setEnabled(false);

  // incorporate prim. list panel
  m_prlPan = new PrimlistPanel( m_engine, this , true);
  verticalLayout->addWidget(m_prlPan);
  m_prlPan->runButton->setText("Start Probing");
  m_prlPan->topLabel->setText("Primitive list executed during probing");
  m_prlPan->outFname->setEnabled(false);
  m_prlPan->browseButton->hide();
  m_prlPan->repeatBox->hide();
  m_prlPan->repeatBox->setChecked(false);
  m_prlPan->nrepBox->hide();
  m_prlPan->repLabel->hide();
  m_prlPan->repStep->hide();
  m_prlPan->digitLabel->hide();
  m_prlPan->indexToFile->hide();
  m_prlPan->indexToItem->hide();
  m_prlPan->modtestMode->hide();
  m_prlPan->modtestType->hide();
  m_prlPan->burninBox->hide();
  m_prlPan->checkBoxM0->hide();
  m_prlPan->checkBoxM1->hide();
  m_prlPan->checkBoxM2->hide();
  m_prlPan->checkBoxM3->hide();
  QObject::connect(m_prlPan->runButton, SIGNAL(clicked()), this, SLOT(startProbing()));  
  QObject::connect(m_prlPan, SIGNAL(changeStartState(bool)), this, SLOT(checkReadyForProbing(bool)));
  connect( &(m_engine), SIGNAL( prlDone() ), this, SLOT(checkReadyForProbing()) );
}
WaferTestPanel::~WaferTestPanel()
{
  delete m_prober;
}
void WaferTestPanel::newProber()
{
  m_prober = PixProber::make(0,string(proberTypesBox->currentText().toLatin1().data()));
  QObject::disconnect(newEditButton, SIGNAL(clicked()), this, SLOT(newProber()));  
  QObject::disconnect(loadButton, SIGNAL(clicked()), this, SLOT(loadProber()));  
  QObject::connect(newEditButton, SIGNAL(clicked()), this, SLOT(editPrbCfg()));
  QObject::connect(loadButton, SIGNAL(clicked()), this, SLOT(deleteProber()));  
  newEditButton->setText("Edit Prober Cfg.");
  proberTypesBox->setEnabled(false);
  loadButton->setText("Delete Prober");
  saveButton->setEnabled(true);
  editPrbCfg();
}
void WaferTestPanel::editPrbCfg()
{
  optionsPanel op(m_prober->config(), this);
  op.setWindowTitle("Edit "+proberTypesBox->currentText());
  op.exec();

  checkReadyForProbing();
}
void WaferTestPanel::deleteProber(){
  delete m_prober; m_prober=0;
  QObject::connect(newEditButton, SIGNAL(clicked()), this, SLOT(newProber()));  
  QObject::connect(loadButton, SIGNAL(clicked()), this, SLOT(loadProber()));  
  QObject::disconnect(newEditButton, SIGNAL(clicked()), this, SLOT(editPrbCfg()));
  QObject::disconnect(loadButton, SIGNAL(clicked()), this, SLOT(deleteProber()));  
  newEditButton->setText("New Prober");
  proberTypesBox->setEnabled(true);
  loadButton->setText("Load Prober");
  saveButton->setEnabled(false);  
  checkReadyForProbing();
}
void WaferTestPanel::loadProber(){
  Config &opts = m_engine.getOptions();
  string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT prober file (*.prb.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify output folder for data files", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::ExistingFile);

  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().at(0);
    qpath.replace("\\", "/");

    PixConfDBInterface *myDB = new RootDB(qpath.toLatin1().data());
    if(myDB==0){
      m_engine.toErrLog(("WaferTestPanel::saveProber : error opening file "+qpath).toLatin1().data());
      return;
    }
    
    DBInquire *root = 0;
    try{
      root = myDB->readRootRecord(1);
    } catch(...){    
      delete myDB;
      m_engine.toErrLog(("WaferTestPanel::saveProber : error reading root record in file "+qpath).toLatin1().data());
      return;
    }

    delete m_prober; m_prober=0;
    for(recordIterator pbIter = root->recordBegin();pbIter!=root->recordEnd();pbIter++){
      if((*pbIter)->getName().find("PixProber")!=std::string::npos){
	string pbclass = (*pbIter)->getDecName();
	// remove heading and trailing "/"
	pbclass.erase(0,1);
	pbclass.erase(pbclass.length()-1,1);
	//cout << "found " << pbclass << endl;
	// check if class type is available and if so, fix it and create prober
	for(int i=0;i<proberTypesBox->count();i++){
	  if(string(proberTypesBox->itemText(i).toLatin1().data())==pbclass){
	    m_prober = PixProber::make(*pbIter,pbclass);
	    proberTypesBox->setCurrentIndex(i);
	    break;
	  }
	}
      }
      if(m_prober!=0) break;
    }

    if(m_prober!=0){
      QObject::disconnect(newEditButton, SIGNAL(clicked()), this, SLOT(newProber()));  
      QObject::disconnect(loadButton, SIGNAL(clicked()), this, SLOT(loadProber()));  
      QObject::connect(newEditButton, SIGNAL(clicked()), this, SLOT(editPrbCfg()));
      QObject::connect(loadButton, SIGNAL(clicked()), this, SLOT(deleteProber()));  
      newEditButton->setText("Edit Prober Cfg.");
      proberTypesBox->setEnabled(false);
      loadButton->setText("Delete Prober");
      saveButton->setEnabled(true);
      checkReadyForProbing();
    }
  }
}
void WaferTestPanel::saveProber(){
  if(m_prober==0) return;

  Config &opts = m_engine.getOptions();
  string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT prober file (*.prb.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this,"Specify output folder for data files", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);

  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().at(0);
    qpath.replace("\\", "/");
    if(qpath.right(8)!="prb.root") qpath += ".prb.root";

    // check if file exists -> remove
    FILE *test = fopen(qpath.toLatin1().data(),"r");
    if(test!=0){
      fclose(test);
      remove(qpath.toLatin1().data());
    }

    PixConfDBInterface *myDB = new RootDB(qpath.toLatin1().data(),"NEW");
    if(myDB==0){
      m_engine.toErrLog(("WaferTestPanel::saveProber : error opening file "+qpath).toLatin1().data());
      return;
    }
    
    DBInquire *root = 0;
    try{
      root = myDB->readRootRecord(1);
    } catch(...){    
      delete myDB;
      m_engine.toErrLog(("WaferTestPanel::saveProber : error reading root record in file "+qpath).toLatin1().data());
      return;
    }

    string name = "PixProber";
    string decName = proberTypesBox->currentText().toLatin1().data();
    DBInquire *inq = root->addInquire(name, decName);
    m_prober->config().write(inq);
    delete myDB;
  }
}
void WaferTestPanel::browseOutput(){
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defDataPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QFileDialog fdia(this, "Specify output folder for data files", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setFileMode(QFileDialog::DirectoryOnly);

  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
    outdirName->setText(fname);
  }
  checkReadyForProbing();
}
void WaferTestPanel::startProbing(){
  disconnect( &(m_engine), SIGNAL( prlDone() ), this, SLOT(checkReadyForProbing()) );
  disconnect(m_prlPan->runButton, SIGNAL(clicked()), this, SLOT(startProbing()));  
  connect(m_prlPan->runButton, SIGNAL(clicked()), this, SLOT(abortProbing()));  
  m_prlPan->runButton->setText("Abort");
  m_prlPan->runButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
  m_prlPan->freezePanel();
  m_prlPan->runButton->setEnabled(true);

  newEditButton->setEnabled(false);
  loadButton->setEnabled(false);
  saveButton->setEnabled(false);  
  browseDirButton->setEnabled(false);  
  outdirName->setEnabled(false);
  colField->setEnabled(false);
  rowField->setEnabled(false);
  indexField->setEnabled(false);
  indexLabelType->setEnabled(false);
  subdieBox->setEnabled(false);
  waferLabel->setEnabled(false);
  waferSN->setEnabled(false);
  firstChipBox->setEnabled(false);
  nChips->setEnabled(false);
  m_abortFlag = false;
  statusLabel->setText(" ");
  m_engine.getApp().processEvents();

  // make sure all needed dir's exist
  QDir mainDir(outdirName->text());
  mainDir.mkdir("wafer_"+waferLabel->text());
  if(indexLabelType->currentIndex()==2){
    mainDir.mkdir("wafer_"+waferLabel->text()+"/chipA");
    mainDir.mkdir("wafer_"+waferLabel->text()+"/chipB");
  }

  // move chuck to first chip
  try{
    m_prober->chuckSeparate();
  } catch(PixProberExc &exc){
    if (STCONTROL_WAFERTESTPANEL_DEBUG)
    {
      exc.dump(std::cerr);
    }
    m_engine.toErrLog("WaferTestPanel::startProbing : Exception while going into separate:\n"+exc.getDescr());
    statusLabel->setText("Abnormal termination of probing, see log for details");
    finishProbing();
    return;
  }catch(...){
    m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while going into separate");
    statusLabel->setText("Abnormal termination of probing, see log for details");
    finishProbing();
    return;
  }
  if(firstChipBox->isChecked()){
    try{
      statusLabel->setText("Moving chuck to home position");
      m_prober->chuckGotoFirst();
    } catch(PixProberExc &exc){
      if (STCONTROL_WAFERTESTPANEL_DEBUG)
      {
        exc.dump(std::cerr);
      }
      m_engine.toErrLog("WaferTestPanel::startProbing : Exception while moving to first die:\n"+exc.getDescr());
      statusLabel->setText("Abnormal termination of probing, see log for details");
      finishProbing();
      return;
    }catch(...){
      m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while moving to first die");
      statusLabel->setText("Abnormal termination of probing, see log for details");
      finishProbing();
      return;
    }
  }else{
    statusLabel->setText("Moving chuck to requested position");
    try{
      if(indexLabelType->currentIndex()==0){
	int index = indexField->text().toInt();
	m_prober->chuckGotoIndex(index);
      } else{
	int row = rowField->text().toInt();
	int col = colField->text().toInt();
	int sd  = (indexLabelType->currentIndex()==2)?subdieBox->currentIndex():0;
	m_prober->chuckGotoPosition(col,row,sd);
      }
    } catch(PixProberExc &exc){
      if (STCONTROL_WAFERTESTPANEL_DEBUG)
      {
        exc.dump(std::cerr);
      }
      m_engine.toErrLog("WaferTestPanel::startProbing : Exception while stepping to requested die:\n"+exc.getDescr());
      statusLabel->setText("Abnormal termination of probing, see log for details");
      finishProbing();
      return;
    }catch(...){
      m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while stepping to requested die");
      statusLabel->setText("Abnormal termination of probing, see log for details");
      finishProbing();
      return;
    }
  }

  m_nChips=0;
  nTestedChips->display(m_nChips);
  probeStepInit(true);
}
void WaferTestPanel::probeStepInit(bool firstStep){
  QObject::disconnect(m_prlPan, SIGNAL(finishedList()), this, SLOT(probeStepInit()));  
  if(m_abortFlag || (m_nChips>=(nChips->value()-1) && nChips->value()>0 && !firstStep)){
    try{
      m_prober->chuckSeparate();
    } catch(PixProberExc &exc){
      if (STCONTROL_WAFERTESTPANEL_DEBUG)
      {
        exc.dump(std::cerr);
      }
      m_engine.toErrLog("WaferTestPanel::startProbing : Exception while going into separate:\n"+exc.getDescr());
      statusLabel->setText("Abnormal termination of probing, see log for details");
      finishProbing();
      return;
    }catch(...){
      m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while going into separate");
      statusLabel->setText("Abnormal termination of probing, see log for details");
      finishProbing();
      return;
    }
    if(m_abortFlag)
      statusLabel->setText("User abort");
    else{
      nTestedChips->display(m_nChips+1);
      statusLabel->setText("Reached number of requested chips to be probed");
    }
    finishProbing();
    return;
  }
  if(!firstStep){
    m_nChips++;
    nTestedChips->display(m_nChips);
    statusLabel->setText("Moving chuck to next die");
    bool endOfWafer = false;
    try{
      try{
        m_prober->chuckSeparate();
      } catch(PixProberExc &exc){
        if (STCONTROL_WAFERTESTPANEL_DEBUG)
        {
          exc.dump(std::cerr);
        }
        m_engine.toErrLog("WaferTestPanel::startProbing : Exception while going into separate:\n"+exc.getDescr());
        statusLabel->setText("Abnormal termination of probing, see log for details");
        finishProbing();
        return;
      }catch(...){
        m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while going into separate");
        statusLabel->setText("Abnormal termination of probing, see log for details");
        finishProbing();
        return;
      }
      try{
        endOfWafer = m_prober->chuckNextDie();
      } catch(PixProberExc &exc){
        if (STCONTROL_WAFERTESTPANEL_DEBUG)
        {
          exc.dump(std::cerr);
        }
        m_engine.toErrLog("WaferTestPanel::startProbing : Exception while moving to next die:\n"+exc.getDescr());
        statusLabel->setText("Abnormal termination of probing, see log for details");
        finishProbing();
        return;
      }catch(...){
        m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while moving to next die");
        statusLabel->setText("Abnormal termination of probing, see log for details");
        finishProbing();
        return;
      }
      if(endOfWafer){
        statusLabel->setText("Successfully reached end of wafer");
      }
    } catch(PixProberExc &exc){
      if (STCONTROL_WAFERTESTPANEL_DEBUG)
      {
        exc.dump(std::cerr);
      }
      m_engine.toErrLog("WaferTestPanel::probeStepInit : Exception while stepping to next die:\n"+exc.getDescr());
      statusLabel->setText("Abnormal termination of probing, see log for details");
      endOfWafer = true;
    }catch(...){
      m_engine.toErrLog("WaferTestPanel::probeStepInit : Unknown exception while stepping to next die");
      statusLabel->setText("Abnormal termination of probing, see log for details");
      endOfWafer = true;
    }
    if(endOfWafer){
      finishProbing();
      return;
    }
  }
  try{
    m_prober->chuckContact();
  } catch(PixProberExc &exc){
    if (STCONTROL_WAFERTESTPANEL_DEBUG)
    {
      exc.dump(std::cerr);
    }
    m_engine.toErrLog("WaferTestPanel::startProbing : Exception while going into contact:\n"+exc.getDescr());
    statusLabel->setText("Abnormal termination of probing, see log for details");
    finishProbing();
    return;
  }catch(...){
    m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while going into contact");
    statusLabel->setText("Abnormal termination of probing, see log for details");
    finishProbing();
    return;
  }

  // get coordinates
  int col, row, sd, index;
  try{
    if(indexLabelType->currentIndex()==0){
      m_prober->chuckGetIndex(index);
      indexField->setText(QString::number(index));
      colField->setText("");
      rowField->setText("");
    } else{
      m_prober->chuckGetPosition(col,row,sd);
      colField->setText(QString::number(col));
      rowField->setText(QString::number(row));
      subdieBox->setCurrentIndex(sd);
      indexField->setText("");
    }
  } catch(PixProberExc &exc){
    if (STCONTROL_WAFERTESTPANEL_DEBUG)
    {
      exc.dump(std::cerr);
    }
    m_engine.toErrLog("WaferTestPanel::startProbing : Exception while retrieving chuck position:\n"+exc.getDescr());
    statusLabel->setText("Abnormal termination of probing, see log for details");
    finishProbing();
    return;
  }catch(...){
    m_engine.toErrLog("WaferTestPanel::startProbing : Unknown exception while retrieving chuck position");
    statusLabel->setText("Abnormal termination of probing, see log for details");
    finishProbing();
    return;
  }

  QString fname = outdirName->text()+"/wafer_"+waferLabel->text();
  if(indexLabelType->currentIndex()==2) fname += "/chip"+subdieBox->currentText();
  if(indexLabelType->currentIndex()==0)
    fname += "/wafer_"+waferLabel->text()+"_"+indexField->text();
  else
    fname += "/wafer_"+waferLabel->text()+"_"+colField->text()+"_"+rowField->text();
  if(indexLabelType->currentIndex()==2) fname += "_"+subdieBox->currentText();
  fname += ".root";
  // temporary: remove existing file - have to come up with something better later
  if(QFile::exists(fname)) QFile::remove(fname);
  m_prlPan->outFname->setText(fname);

  // check for cfg. reload item and write wafer/chip SN to it
  // bits 14...6: wafer SN, bits 5..0: chip SN
  int waferChipSN = (waferSN->text().toInt()&0x1ff)<<6; // shift by 6 bits and limit to range of 9 bits
  if(indexLabelType->currentIndex()==0) waferChipSN += (indexField->text().toInt())&0x3f;
  else                                 waferChipSN += m_nChips&0x3f; // don't know anything better to do
  std::vector<PrimListItem*>prl_items = m_prlPan->getPrlItems();
  for(std::vector<PrimListItem*>::iterator pit=prl_items.begin(); pit!=prl_items.end(); pit++){
    if((*pit)->getSubType()==PrimListItem::LOAD_MOD_CFG) // update chip SN cfg. obj. in item
      WriteIntConf(((ConfInt&) (*pit)->config()["arguments"]["Chip_SN"]), waferChipSN);
  }
  // start primitive list
  connect(m_prlPan, SIGNAL(finishedList()), this, SLOT(probeStepInit()));  
  statusLabel->setText("Executing primitive list");
  m_prlPan->startPrl();
}
void WaferTestPanel::abortProbing(){
  m_prlPan->runButton->setEnabled(false);
  m_abortFlag = true;
  m_prlPan->abortPrl();
}
void WaferTestPanel::finishProbing(){
  disconnect(m_prlPan->runButton, SIGNAL(clicked()), this, SLOT(abortProbing()));  
  connect(m_prlPan->runButton, SIGNAL(clicked()), this, SLOT(startProbing()));  
  connect( &(m_engine), SIGNAL( prlDone() ), this, SLOT(checkReadyForProbing()) );
  m_prlPan->runButton->setEnabled(true);
  m_prlPan->runButton->setText("Start Probing");
  m_prlPan->runButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
  m_prlPan->finishedPrl();

  newEditButton->setEnabled(true);
  loadButton->setEnabled(true);
  saveButton->setEnabled(true);  
  browseDirButton->setEnabled(true);  
  outdirName->setEnabled(true);
  indexLabelType->setEnabled(true);
  waferLabel->setEnabled(true);
  waferSN->setEnabled(true);
  enableSDbox(indexLabelType->currentIndex());
  firstChipBox->setEnabled(true);
  nChips->setEnabled(true);
}
void WaferTestPanel::checkReadyForProbing(bool extFlag){
  if(!extFlag) {
    m_prlPan->runButton->setEnabled(false);
    return;
  }
  if(outdirName->text()!="" && m_prlPan->prmlistView->topLevelItemCount()>0 && m_prober!=0){
    try
    {
      m_prober->initHW();
    }
    catch (PixProberExc& e)
    {
      if (STCONTROL_WAFERTESTPANEL_DEBUG)
      {
        std::cerr << __FILE__ << ":" << __LINE__ << ": PixProberException: ";
        e.dump(std::cerr);
	m_engine.toErrLog("WaferTestPanel::checkReadyForProbing : Exception while initialising:\n"+e.getDescr());
      }
      m_prlPan->runButton->setEnabled(false);
    } 
    catch(...)
    {
      if (STCONTROL_WAFERTESTPANEL_DEBUG)
      {
        std::cerr << __FILE__ << ":" << __LINE__ << ": Unhandled Exception";
	m_engine.toErrLog("WaferTestPanel::checkReadyForProbing : Unhandled exception while initialising");
      }
      m_prlPan->runButton->setEnabled(false);
      return;
    }
    m_prlPan->runButton->setEnabled(true);
  } else{
    m_prlPan->runButton->setEnabled(false);
  }
}
void WaferTestPanel::enableSDbox(int labelType){
  subdieBox->setEnabled(labelType==2);
  colField->setEnabled(labelType>0);
  rowField->setEnabled(labelType>0);
  indexField->setEnabled(labelType==0);
}
