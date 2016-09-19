#/***************************************************************************
                          STcontrolEngine.cxx  -  description
                             -------------------
    begin                : Fri Apr 16 2004
    copyright            : (C) 2004 by jschumac
    email                : jschumac@silabsbc
    modifications        : 2005 by rottlaen
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <PixConfDBInterface/RootDB.h>
#include <PixConfDBInterface/TurboDaqDB.h>
#include <PixController/PixScan.h>
#include <PixModule/PixModule.h>
#include <Config/Config.h>
#include <PixDcs/PixDcs.h>
#include <PixDcs/SleepWrapped.h>
#include <DBEdtEngine.h>
#include <GeneralDBfunctions.h>
#include <DataContainer/PixDBData.h>
#include <SmtpMime>

#include "STCLogContainer.h"
#include "STCLog.h"
#include "STControlEngine.h"
#include "STRodCrate.h"
#include "ChipTest.h"
#include "PrimListItem.h"

#include <TGraph.h>

#include <QApplication>
#include <QTimer>
#include <QTime>
#include <QDateTime>
#include <QString>
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>

#include <iostream>
#include <stdlib.h>
#ifdef WIN32
#include <stdlib.h>
#else
#include <unistd.h>
#endif


using namespace PixLib;
using namespace SctPixelRod;

STControlEngine::STControlEngine( QApplication *app, STCLogContainer& log_in, QObject * parent) :
  QObject( parent ), m_app(app), m_log(log_in), m_singleCrateMode(true), m_PixConfDBFname("") {
#ifdef WITHEUDAQ
  m_STControlProducer = nullptr;
#endif

  m_PixConfDBFname = "";
  m_IFtype = tundefIF;

  m_lastPixScanOpts.scanConfig=0;
  m_lastPixScanOpts.loadToAna=0;
  m_lastPixScanOpts.anaLabel="";
  m_lastPixScanOpts.writeToFile=false;
  m_lastPixScanOpts.fileName="";
  m_lastPixScanOpts.timestampStart="";
  m_lastPixScanOpts.stdTestID = -1;
  m_lastPixScanOpts.timeToAbort = -1;

  // create single-shot and permanent timer for scan monitoring and prim.list processing
  m_scanTimer = new QTimer(this);
  m_scanTimer->setSingleShot(true);
  m_scanTimer2 = new QTimer(this);
  m_scanTimer2->setSingleShot(false);
  m_cfgEdited=false;
  m_scanPtsPtr[0] = 0;
  m_scanPtsPtr[1] = 0;
  m_checkScanCount = 0;
  m_pixScanStartTime = new QTime();
  
  //m_finishScanRunning = false;
  //  m_observedGroup = 0;
  m_obsCrate = -1;
  m_obsGrp = -1;

  // chip test option classes
  m_ctCfg = new Config("ChipTest");
  m_ctCfg->addGroup("general");
  (*m_ctCfg)["general"].addBool("resetAfterTest", m_resetAfterTest, false, 
			      "Reset Module/FEs after any of the tests", true);
  m_ctCfg->reset();
  ChipTest ctdummy(ChipTest::CTDUMMY);
  std::map<std::string, int> types = ctdummy.getTypeList();
  for(std::map<std::string, int>::iterator it=types.begin(); it!=types.end(); it++){
    ChipTest *ct = ChipTest::make((ChipTest::chipTestType) it->second);
    m_chipTests[it->first] = ct;
    m_ctCfg->addConfig(&(ct->config()));
  }

  // need $PIX_LIB to find base config files
  //  m_plPath = gSystem->Getenv("PIX_LIB");
  m_plPath = QProcessEnvironment::systemEnvironment ().value("PIX_LIB");

  // set up options
  m_options = new Config("STControlConfig");
  Config &conf = *m_options;
  // Group layout
  conf.addGroup("layout");
  conf["layout"].addBool("RoDcsSamePanel", m_RoDcsSamePanel, true,
			"Show read-out and monitoring/regulator config on same panel", true);
  // Group paths
  conf.addGroup("paths");
  conf["paths"].addString("defCfgPath", m_cfgPath, ".",
			  "default config file path", true, 2);
  conf["paths"].addString("defLogPath", m_logPath, QDir::homePath().toLatin1().data(),
			  "default logfile file path", true, 2);
  conf["paths"].addString("defDataPath", m_dataPath, ".",
			  "default data file path", true, 2);
  conf["paths"].addString("defDataName", m_dataName, "",
			"default data file name", true);
  conf["paths"].addBool("addDatetoDataFile", m_addDatetoDataFile, true,
			"append current date to default data file name", true);

  // Group module tests

  conf.addGroup("autoload");
  conf["autoload"].addString("defCfgName", m_cfgName, "",
			     "start-up loading of cfg.-file", true, 3);
  conf["autoload"].addBool("initRods", m_initRods, false,
			   "init all PixControllers after auto-config loading", true);
  conf["autoload"].addBool("initDcs", m_initDcs, false,
			   "init all PixDcs objects after auto-config loading", true);
  conf["autoload"].addBool("dcsOn", m_dcsOn, false,
			   "turn on all supply-type PixDcs objects after auto-DCS-init", true);
  conf["autoload"].addString("defPixScanName", m_psName, "",
			     "start-up loading of PixScan-file", true, 3);
  conf["autoload"].addBool("doPixScanAutoLabel", m_psAutoLabel, true,
			   "automatic setting of scan label from PixScan name", true);

  conf.addGroup("errors");
  conf["errors"].addBool("showRodWin", m_showRodWin, true, 
			 "Show warning panel in case of ROD error buffer messages", true);
  conf["errors"].addBool("showErrWin", m_showErrWin, true, 
			 "Show warning panel in case of general error buffer messages", true);
  conf["errors"].addInt("scanTimeout", m_scanTimeout, 1, 
			 "Timeout in minutes for checking that a scan started correctly", true);

  conf.addGroup("modtest");
  conf["modtest"].addString("defPath", m_modPath, ".",
			    "default path for module testing", true, 2);
//   conf["modtest"].addVector("testTypes", m_testTypes, types,
// 			"default path for module testing", true);
  conf["modtest"].addBool("resetAfterTest", m_resetAfterTest, false,
			 "Reset Module/FEs after any of the tests", true);

  // initialise options
  m_options->reset();
  try{
    loadOptions();
    // must have a valid log file dir - use home if provided path doesn't exist
    QDir logPath(m_logPath.c_str());
    if(!logPath.exists()) m_logPath = QDir::homePath().toLatin1().data();
  }catch(...){
    m_options->reset();
  }

#ifdef WITHEUDAQ
  //Checking command line for producer id
  QString eudaq_producer_id;
  int rc_pos = m_app->arguments ().indexOf ("-pid");
  if (rc_pos>=0 && m_app->arguments ().size() > rc_pos+1)
	  eudaq_producer_id = m_app->arguments ().at(rc_pos + 1).toLocal8Bit().constData();

  if (eudaq_producer_id.trimmed().left(1)=="-") // another option
	eudaq_producer_id="0";

  //TB:TODO producer ID?
  bool isInt=false;
  int producer_id=eudaq_producer_id.trimmed().toInt(&isInt);
  //if (isInt) m_STeudaq -> setProducerId(producer_id);

  // Checking Command Line for Run Control address
  QString eudaq_rc_address;
  rc_pos = m_app->arguments ().indexOf ("-r");
  if (rc_pos>=0 && m_app->arguments ().size() > rc_pos+1)
	  eudaq_rc_address = m_app->arguments ().at(rc_pos + 1).toLocal8Bit().constData();

  if (eudaq_rc_address.trimmed().left(1)=="-") // another option
	eudaq_rc_address="";

  if (rc_pos>=0 && eudaq_rc_address!="") {
	  m_STControlProducer = std::unique_ptr<STControlProducer>(new STControlProducer(*this, "name", eudaq_rc_address.toStdString()) );
  }
#endif
}
STControlEngine::~STControlEngine(){
  m_singleCrateMode=false; // make sure crates are deleted this time
  clear();

  // remove user PixScan configs
  m_lastPixScanOpts.scanConfig=0;
  for(std::vector<PixScan*>::iterator scfgIter=m_knownPixScanCfgs.begin();
      scfgIter!=m_knownPixScanCfgs.end();scfgIter++)
    delete (*scfgIter);

  delete m_options;

  for(std::map<std::string, ChipTest*>::iterator it=m_chipTests.begin(); it!=m_chipTests.end(); it++)
    delete (it->second);

}
/** Read property of vector <STRodCrate *> m_sTRodCrate. */
std::vector <STRodCrate *> & STControlEngine::getSTRodCrates(){
  return m_sTRodCrates;
}
/** open DB file and remember its name */
void STControlEngine::setPixConfDBFname(const char *in_path){
  m_PixConfDBFname = in_path;
  emit cfgLabelChanged();
  PixConfDBInterface *myDB=0;
  if(m_PixConfDBFname!=""){  // open new file
    try{
      myDB = DBEdtEngine::openFile(m_PixConfDBFname.c_str(), false);
    }catch(SctPixelRod::BaseException& exc){
      std::stringstream msg;
      msg << "STControlEngine::setPixConfDBFname : exception while opening DB file " 
          << m_PixConfDBFname << ": " << exc;
      toErrLog(msg.str().c_str());
    }catch(...){
      toErrLog(("STControlEngine::setPixConfDBFname : unknown exception while opening DB file " 
                + m_PixConfDBFname).c_str());
    }
    delete myDB;
  }
}
std::string STControlEngine::getPixScanCfgDBName(PixScan *cfg){
  std::string defVal = "unknown";
  if(m_knownPSDBcfgs.lower_bound((long int)cfg)==m_knownPSDBcfgs.end())
    return defVal;
  return (*m_knownPSDBcfgs.lower_bound((long int)cfg)).second;
}
void STControlEngine::setPixScanCfgDBName(PixScan *cfg, const char *name){
  if(m_knownPSDBcfgs.lower_bound((long int)cfg)!=m_knownPSDBcfgs.end())
    (*m_knownPSDBcfgs.lower_bound((long int)cfg)).second = name;
}
/** Adds a STRodCrate */
STRodCrate& STControlEngine::addCrate(const char *SBCaddress){
  if(m_singleCrateMode && m_sTRodCrates.size()!=0){
    toErrLog("STControlEngine::addCrate : a crate already exists and we are in special single-crate mode -> not creating new crate.");
    static STRodCrate tmpCrate;
    return tmpCrate;
  }
  int crID = m_sTRodCrates.size();
  m_sTRodCrates.push_back( new STRodCrate(crID, getOptions(), m_app, this,SBCaddress) );
  // forward signals from STRodCrate
  connect(m_sTRodCrates[crID],SIGNAL(groupListChanged()), this, SIGNAL(crateListChanged()));
  //  connect(m_sTRodCrates[crID],SIGNAL(updateModList()), this, SIGNAL(updateModList()));
  // forward uncritical messages to log container
  connect(m_sTRodCrates[crID],SIGNAL(logMessage(std::string)), this, SLOT(toLog(std::string)));
  // forward error messages to log container
  connect(m_sTRodCrates[crID],SIGNAL(errorMessage(std::string)), this, SLOT(toErrLog(std::string)));
  // forward ROD messages to log container
  connect(m_sTRodCrates[crID],SIGNAL(rodMessage(std::string, bool)), this, SLOT(toRodBuff(std::string, bool)));
  // forward module test signals
  connect(m_sTRodCrates[crID],SIGNAL(sendRTStatus(const char*)), this, SIGNAL(sendRTStatus(const char*)));
  connect(m_sTRodCrates[crID],SIGNAL(currentModule(int, int, int)), this, SIGNAL(currentModule(int, int, int)));
  connect(m_sTRodCrates[crID],SIGNAL(statusChanged()), this, SIGNAL(statusChanged()));
  // get it working and add to list view
  m_sTRodCrates[crID]->setupVme();
  emit crateListChanged();
  return *(m_sTRodCrates[crID]);
}
/** Deletes all STRodCrates */
void STControlEngine::clear(){
  m_cfgEdited=false;

  if(!m_singleCrateMode){ // must not delete single crate if in single-crate mode!
    for(std::vector<STRodCrate *>::iterator i=m_sTRodCrates.begin(); i != m_sTRodCrates.end(); i++) {
      // disable these signals, after deletion the crate list is not sane any more
      disconnect((*i),SIGNAL(groupListChanged()), this, SIGNAL(crateListChanged()));
      //      disconnect((*i),SIGNAL(updateModList()), this, SIGNAL(updateModList()));
      disconnect((*i),SIGNAL(statusChanged()), this, SIGNAL(statusChanged()));
      delete *i; *i = 0;
    }
    m_sTRodCrates.clear();
    emit crateListChanged(); // call this only once for all crates
  } else if ( m_sTRodCrates.size() == 1 ) // in single-crate mode, just clear that crate
    m_sTRodCrates[0]->clear();
  else
    toErrLog("STControlEngine::clear: can't clear because we don't have any crates but we're supposed to have exactly one.");
}
void STControlEngine::addCrateToDB(const char *crateName, std::vector<grpData> inGrpData, const char *DBfilename, 
				   IFtypes crtIFtype, bool add_UsbDcs)
{
  PixConfDBInterface *myDB = 0;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  try{
    // open file, create app
    myDB = PixLib::addAppInq(DBfilename, crateName);
  }catch(PixDBException &dbexc){
    std::stringstream txt, exmsg;
    dbexc.what(exmsg);
    txt << "STControlEngine::addCrateToDB : PixDB-exception " <<exmsg.str() << " while creating new DB file "<< DBfilename;
    toErrLog(txt.str());
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addCrateToDB : exception while creating new DB file " 
        << DBfilename << ": " << exc;
    toErrLog(msg.str());
    //remove(DBfilename);
  } catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::addCrateToDB : unknown exception while creating new DB file" 
	      + QString(DBfilename)).toLatin1().data());
    //remove(DBfilename);
  }
  try{
    // close and re-open file
    delete myDB;
    myDB = DBEdtEngine::openFile(DBfilename, true);
    // get crate inquire again
    PixLib::DBInquire *startInq = PixLib::findAppInq(myDB, crateName);
    if(startInq==0){
      toErrLog(("STControlEngine::addCrateToDB : unable to create crate inquire in DB file" + QString(DBfilename)).toLatin1().data());
      return;
    }
    // add groups
    int usbInd=0;
    int usbDcsInd=0;
    for(std::vector<grpData>::iterator IT=inGrpData.begin(); IT!=inGrpData.end(); IT++){
      addGrouptoDB(startInq, *IT, crtIFtype);
      if(crtIFtype==tUSBSys && add_UsbDcs && IT->myMods.size()>0){
		// USBPix regulators and NTC created from scratch
		std::stringstream b;
		b << IT->myROD.slot;
		addUsbDcs(((IT->myROD.slot>=0)?("USB-board_"+b.str()):"USB-board"), IT->myROD.slot, IT->myROD.mode, usbDcsInd, usbInd, startInq, myDB);
      }
    }
    // get rid of temporary file
    delete myDB;
  }catch(PixDBException &dbexc){
    std::stringstream txt, exmsg;
    dbexc.what(exmsg);
    txt << "STControlEngine::addCrateToDB : PixDB-exception " <<exmsg.str() << " while working on DB file "<< DBfilename;
    toErrLog(txt.str());
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addCrateToDB : exception while working on DB file " 
        << DBfilename << ": " << exc;
    toErrLog(msg.str().c_str());
    //remove(DBfilename);
  } catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::addCrateToDB : unknown exception while working on DB file" 
	      + QString(DBfilename)).toLatin1().data());
    //remove(DBfilename);
  }
  QApplication::restoreOverrideCursor();
  return;
}
/** adds USBPix regulators and NTC from scratch with default settings */
void STControlEngine::addUsbDcs(std::string ctrlName_in, int USB_id, int adapterType, int &usbDcsInd, int &usbInd, DBInquire *startInq, PixConfDBInterface *myDB){
  std::stringstream a;
  a << usbDcsInd;
  usbDcsInd++;
  std::stringstream b;
  b << USB_id;
  DBInquire *new_inq = myDB->makeInquire("PixDcs","USBPixDcs_"+a.str());
  startInq->pushRecord(new_inq);
  myDB->DBProcess(startInq,COMMITREPLACE);
  myDB->DBProcess(new_inq,COMMIT);
  DBField *fld = myDB->makeField("USBPixController");
  std::string ctrlName=ctrlName_in;
  myDB->DBProcess(fld,COMMIT,ctrlName);
  new_inq->pushField(fld);
  delete fld;
  myDB->DBProcess(new_inq,COMMITREPLACE);
  fld = myDB->makeField("general_Index");
  myDB->DBProcess(fld,COMMIT,usbInd);
  new_inq->pushField(fld);
  delete fld;
  myDB->DBProcess(new_inq,COMMITREPLACE);
  ctrlName = (adapterType == 4)?"GPAC-Supplies":"USB-regulators";
  if(USB_id>=0)
	ctrlName += "_"+b.str();
  fld = myDB->makeField("general_DeviceName");
  myDB->DBProcess(fld,COMMIT,ctrlName);
  new_inq->pushField(fld);
  delete fld;
  myDB->DBProcess(new_inq,COMMITREPLACE);
  ctrlName = "SUPPLY";
  fld = myDB->makeField("general_DeviceType");
  myDB->DBProcess(fld,COMMIT,ctrlName);
  new_inq->pushField(fld);
  delete fld;
  myDB->DBProcess(new_inq,COMMITREPLACE);
  std::string clName = "USBPixDcs";
  if((adapterType==1) || (adapterType==2)) clName = "USBBIPixDcs";
  if(adapterType==4) clName = "USBGpacPixDcs";
  fld = myDB->makeField("ActualClassName");
  myDB->DBProcess(fld,COMMIT,clName);
  new_inq->pushField(fld);
  delete fld;
  myDB->DBProcess(new_inq,COMMITREPLACE);
  // DCS channels
  std::string descr[4]={"VDDD1", "VDDA1", "VDDD2", "VDDA2"};
  std::string descrB[4]={"VDDA1", "VDDD1", "VDDA2", "VDDD2"};
  float nomvolts[4]={1.2f,1.5f,1.2f,1.5f};
  float currLim = 1.0f;
  if((adapterType == 1) || (adapterType == 2) || (adapterType == 4)){
	for(int i=0;i<4;i++){
	  std::stringstream a;
	  a << (i+1);
	  descr[i]=((adapterType == 4)?"PWR":"CH")+a.str();
	  nomvolts[i]=(adapterType == 4)?0.:2.0f;
	}
  }
  for(int iCh=0;iCh<4;iCh++){
	std::stringstream c;
	c<<iCh;
	DBInquire *ch_inq = myDB->makeInquire("PixDcsChan","USBPixDcsChan_"+c.str());
	new_inq->pushRecord(ch_inq);
	myDB->DBProcess(new_inq,COMMITREPLACE);
	myDB->DBProcess(ch_inq,COMMIT);
	fld = myDB->makeField((adapterType == 4)?"general_GpacChannel":"general_ChannelDescr");
	if(adapterType == 4)
	  myDB->DBProcess(fld,COMMIT,iCh);
	else
	  myDB->DBProcess(fld,COMMIT,descr[iCh]);
	ch_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(ch_inq,COMMITREPLACE);
	fld = myDB->makeField("general_ChannelName");
	myDB->DBProcess(fld,COMMIT,descr[iCh]);
	ch_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(ch_inq,COMMITREPLACE);
	fld = myDB->makeField("settings_NomVolts");
	myDB->DBProcess(fld,COMMIT,nomvolts[iCh]);
	ch_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(ch_inq,COMMITREPLACE);
  }
  if(adapterType>1){
	fld = myDB->makeField((adapterType == 4)?"general_CurrentLimit":"general_CurrLim");
	myDB->DBProcess(fld,COMMIT,currLim);
	new_inq->pushField(fld);
	delete fld;
  }
  myDB->DBProcess(new_inq,COMMITREPLACE);
  usbInd++;
  if(adapterType == 4){
	// GPAC non-pwr channels
	// voltage-source
	std::stringstream d;
	d << "_" << usbDcsInd;
	usbDcsInd++;
	new_inq = myDB->makeInquire("PixDcs","USBGpacPixDcs"+d.str());
	startInq->pushRecord(new_inq);
	myDB->DBProcess(startInq,COMMITREPLACE);
	myDB->DBProcess(new_inq,COMMIT);
	fld = myDB->makeField("USBPixController");
    ctrlName=ctrlName_in;
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	fld = myDB->makeField("general_Index");
	myDB->DBProcess(fld,COMMIT,usbInd);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "GPAC-Vsrc";
	if(USB_id>=0)
	  ctrlName += "_"+b.str();
	fld = myDB->makeField("general_DeviceName");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "VOLTAGE-SOURCE";
	fld = myDB->makeField("general_DeviceType");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	clName = "USBGpacPixDcs";
	fld = myDB->makeField("ActualClassName");
	myDB->DBProcess(fld,COMMIT,clName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	// DCS channels
	for(int ich=0;ich<4;ich++){
	  float nomVolts = 0.f;
	  std::stringstream ichs;
	  ichs << ich;
	  DBInquire *ch_inq = myDB->makeInquire("PixDcsChan","USBPixDcsChan_"+ichs.str());
	  new_inq->pushRecord(ch_inq);
	  myDB->DBProcess(new_inq,COMMITREPLACE);
	  myDB->DBProcess(ch_inq,COMMIT);
	  fld = myDB->makeField("general_GpacChannel");
	  myDB->DBProcess(fld,COMMIT,ich);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("general_ChannelName");
	  descr[0] = "VSRC"+ichs.str();
	  myDB->DBProcess(fld,COMMIT,descr[0]);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("general_NomVolts");
	  myDB->DBProcess(fld,COMMIT,nomVolts);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	}
	myDB->DBProcess(new_inq,COMMITREPLACE);
	usbInd++;
	// current-source
	std::stringstream e;
	e << "_" << usbDcsInd;
	usbDcsInd++;
	new_inq = myDB->makeInquire("PixDcs","USBGpacPixDcs"+e.str());
	startInq->pushRecord(new_inq);
	myDB->DBProcess(startInq,COMMITREPLACE);
	myDB->DBProcess(new_inq,COMMIT);
	fld = myDB->makeField("USBPixController");
	ctrlName=ctrlName_in;
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	fld = myDB->makeField("general_Index");
	myDB->DBProcess(fld,COMMIT,usbInd);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "GPAC-Isrc";
	if(USB_id>=0)
	  ctrlName += "_"+b.str();
	fld = myDB->makeField("general_DeviceName");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "CURRENT-SOURCE";
	fld = myDB->makeField("general_DeviceType");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	clName = "USBGpacPixDcs";
	fld = myDB->makeField("ActualClassName");
	myDB->DBProcess(fld,COMMIT,clName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	// DCS channels
	for(int ich=0;ich<12;ich++){
	  float nomCurr = 0.f;
	  std::stringstream ichs;
	  ichs << ich;
	  DBInquire *ch_inq = myDB->makeInquire("PixDcsChan","USBPixDcsChan_"+ichs.str());
	  new_inq->pushRecord(ch_inq);
	  myDB->DBProcess(new_inq,COMMITREPLACE);
	  myDB->DBProcess(ch_inq,COMMIT);
	  fld = myDB->makeField("general_GpacChannel");
	  myDB->DBProcess(fld,COMMIT,ich);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("general_ChannelName");
	  descr[0] = "ISRC"+ichs.str();
	  myDB->DBProcess(fld,COMMIT,descr[0]);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("general_NomCurrent");
	  myDB->DBProcess(fld,COMMIT,nomCurr);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	}
	myDB->DBProcess(new_inq,COMMITREPLACE);
	usbInd++;
	// ADC
	std::stringstream x;
	x << "_" << usbDcsInd;
	usbDcsInd++;
	new_inq = myDB->makeInquire("PixDcs","USBGpacPixDcs"+x.str());
	startInq->pushRecord(new_inq);
	myDB->DBProcess(startInq,COMMITREPLACE);
	myDB->DBProcess(new_inq,COMMIT);
	fld = myDB->makeField("USBPixController");
	ctrlName=ctrlName_in;
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	fld = myDB->makeField("general_Index");
	myDB->DBProcess(fld,COMMIT,usbInd);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "GPAC-ADC";
	if(USB_id>=0)
	  ctrlName += "_"+b.str();
	fld = myDB->makeField("general_DeviceName");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "ADC-METER";
	fld = myDB->makeField("general_DeviceType");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	clName = "USBGpacPixDcs";
	fld = myDB->makeField("ActualClassName");
	myDB->DBProcess(fld,COMMIT,clName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	// DCS channels
	for(int ich=0;ich<4;ich++){
	  //float nomCurr = 0.f;
	  std::stringstream ichs;
	  ichs << ich;
	  DBInquire *ch_inq = myDB->makeInquire("PixDcsChan","USBPixDcsChan_"+ichs.str());
	  new_inq->pushRecord(ch_inq);
	  myDB->DBProcess(new_inq,COMMITREPLACE);
	  myDB->DBProcess(ch_inq,COMMIT);
	  fld = myDB->makeField("general_GpacChannel");
	  myDB->DBProcess(fld,COMMIT,ich);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("general_ChannelName");
	  descr[0] = "ISRC"+ichs.str();
	  myDB->DBProcess(fld,COMMIT,descr[0]);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	}
	myDB->DBProcess(new_inq,COMMITREPLACE);
	usbInd++;
	// injection
	std::stringstream y;
	y << "_" << usbDcsInd;
	usbDcsInd++;
	new_inq = myDB->makeInquire("PixDcs","USBGpacPixDcs"+y.str());
	startInq->pushRecord(new_inq);
	myDB->DBProcess(startInq,COMMITREPLACE);
	myDB->DBProcess(new_inq,COMMIT);
	fld = myDB->makeField("USBPixController");
	ctrlName=ctrlName_in;
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	fld = myDB->makeField("general_Index");
	myDB->DBProcess(fld,COMMIT,usbInd);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "GPAC-Injection";
	if(USB_id>=0)
	  ctrlName += "_"+b.str();
	fld = myDB->makeField("general_DeviceName");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "PULSER";
	fld = myDB->makeField("general_DeviceType");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	clName = "USBGpacPixDcs";
	fld = myDB->makeField("ActualClassName");
	myDB->DBProcess(fld,COMMIT,clName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	// DCS channels
	for(int ich=0;ich<1;ich++){
	  float nomHi = 0.5f;
	  float nomLo = 1.f;
	  std::stringstream ichs;
	  ichs << ich;
	  DBInquire *ch_inq = myDB->makeInquire("PixDcsChan","USBPixDcsChan_"+ichs.str());
	  new_inq->pushRecord(ch_inq);
	  myDB->DBProcess(new_inq,COMMITREPLACE);
	  myDB->DBProcess(ch_inq,COMMIT);
	  fld = myDB->makeField("general_GpacChannel");
	  myDB->DBProcess(fld,COMMIT,ich);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("general_ChannelName");
	  descr[0] = "INJECT"+ichs.str();
	  myDB->DBProcess(fld,COMMIT,descr[0]);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("settings_High");
	  myDB->DBProcess(fld,COMMIT,nomHi);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	  fld = myDB->makeField("settings_Low");
	  myDB->DBProcess(fld,COMMIT,nomLo);
	  ch_inq->pushField(fld);
	  delete fld;
	  myDB->DBProcess(ch_inq,COMMITREPLACE);
	}
	myDB->DBProcess(new_inq,COMMITREPLACE);
	usbInd++;
  }
  if((adapterType == 0) || (adapterType == 3)){
	// USBPix NTC ADC
	std::stringstream d;
	d << "_" << usbDcsInd;
	usbDcsInd++;
	new_inq = myDB->makeInquire("PixDcs","USBPixDcs"+d.str());
	startInq->pushRecord(new_inq);
	myDB->DBProcess(startInq,COMMITREPLACE);
	myDB->DBProcess(new_inq,COMMIT);
	fld = myDB->makeField("USBPixController");
	ctrlName=ctrlName_in;
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	fld = myDB->makeField("general_Index");
	myDB->DBProcess(fld,COMMIT,usbInd);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "USB-ADC";
	if(USB_id>=0)
	  ctrlName += "_"+b.str();
	fld = myDB->makeField("general_DeviceName");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	ctrlName = "ADC-METER";
	fld = myDB->makeField("general_DeviceType");
	myDB->DBProcess(fld,COMMIT,ctrlName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	clName = "USBPixDcs";
	// clName = "USBBIPixDcs";
	fld = myDB->makeField("ActualClassName");
	myDB->DBProcess(fld,COMMIT,clName);
	new_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(new_inq,COMMITREPLACE);
	// DCS channel
	DBInquire *ch_inq = myDB->makeInquire("PixDcsChan","USBPixDcsChan_0");
	new_inq->pushRecord(ch_inq);
	myDB->DBProcess(new_inq,COMMITREPLACE);
	myDB->DBProcess(ch_inq,COMMIT);
	fld = myDB->makeField("general_ChannelDescr");
	descr[0] = "Temp";
	myDB->DBProcess(fld,COMMIT,descr[0]);
	ch_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(ch_inq,COMMITREPLACE);
	fld = myDB->makeField("general_ChannelName");
	descr[0] = "FE on-board NTC";
	myDB->DBProcess(fld,COMMIT,descr[0]);
	ch_inq->pushField(fld);
	delete fld;
	myDB->DBProcess(ch_inq,COMMITREPLACE);
	usbInd++;
  }
}
/** adds a PixModuleGroup to currently loaded DB file */
void STControlEngine::addGroupToCurrDB(grpData inGrpData, const char *crateName, IFtypes crtIFtype){
  std::string fname = getPixConfDBFname();
  if(fname==""){
    toErrLog("STControlEngine::addGrouptoCurrDB : Can't find existing DB file in memory, so can't edit.");
    return;
  }

  PixConfDBInterface *myDB = reopenDBforEdit();
  if(myDB==0){
    toErrLog("STControlEngine::addGrouptoCurrDB : error reopening existing DB file, so can't edit.");
    reloadDBafterEdit(false);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);    
  DBInquire *startInq=0;
  try{
    if(m_singleCrateMode)
      startInq = PixLib::findAppInq(myDB);
    else
      startInq = PixLib::findAppInq(myDB,crateName);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addGrouptoCurrDB : exception while re-reading new DB file " 
        << fname << ": " << exc;
    toErrLog(msg.str().c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::addGrouptoCurrDB : unknown exception while re-reading new DB file" 
              + fname).c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }
  QApplication::restoreOverrideCursor();

  addGrouptoDB(startInq, inGrpData, crtIFtype);

  // close and remove old file and open new one
  delete myDB;
  reloadDBafterEdit(true);
  return;
}
/** adds PixModules to a PixModuleGroup in currently loaded DB file */
void STControlEngine::addModulesToCurrDB(int crateID, int grpID, std::vector<modData> inModData){
  std::string fname = getPixConfDBFname();
  if(fname==""){
    toErrLog("STControlEngine::addModulesCurrDB : Can't find existing DB file in memory, so can't save.");
    return;
  }

  STRodCrate *crate = getSTRodCrates()[crateID];
  // get name/decorated name of group
  std::string org_name = crate->getGrpName(grpID);
  std::string find_name = crate->getGrpDecName(grpID);
  if(org_name==""|| find_name==""){
    toErrLog("STControlEngine::addModulesCurrDB : error retrieving DB names for group");
    return;
  }
  // remove module name
  int pos = (int)find_name.find_last_of("/"); // remove trailing "/"
  if(pos!=(int)std::string::npos){ // extract group name from "path"
    find_name.erase(pos,find_name.length()-pos);
    pos = find_name.find_last_of("/");
    if(pos!=(int)std::string::npos)
      find_name.erase(0,pos+1);
  }

  // must re-open in rw mode
  PixConfDBInterface *myDB = reopenDBforEdit();
  if(myDB==0){
    toErrLog("STControlEngine::addModulesCurrDB : error reopening existing DB file, so can't edit.");
    reloadDBafterEdit(false);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);    
  DBInquire *startInq=0, *grpInq=0;
  try{
    if(m_singleCrateMode)
      startInq = PixLib::findAppInq(myDB);
    else{
      // get inquire for this crate
      std::string name = "TestApp";
      if(crate!=0) name = crate->getName();
      startInq = PixLib::findAppInq(myDB, name.c_str());
    }
    grpInq  = *(startInq->findRecord(find_name+"/PixModuleGroup;1"));
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addModulesCurrDB : exception while searching for inquire for group "
        << org_name << " from DB file " << fname << ": " << exc;
    toErrLog(msg.str().c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::addModulesCurrDB : unknown exception while searching for inquire for group "
              + org_name + " from DB file "+ fname).c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }
  QApplication::restoreOverrideCursor();

  if(grpInq==0){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::addModulesCurrDB : can't find inquire for group "
              + org_name + " in DB file " + fname).c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }

  // add modules
  try{
    newModWiz(inModData, grpInq, m_plPath.toLatin1().data());
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addModulesCurrDB : exception while creating modules in module group DB entry: " << exc;
    toErrLog(msg.str().c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog("STControlEngine::addModulesCurrDB : unknown exception while creating modules in module group DB entry");
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }
  QApplication::restoreOverrideCursor();
 
  // close and remove old file and load new one
  delete myDB;
  reloadDBafterEdit(true);
  return;
}
/** adds a PixModuleGroup to a loaded DB file */
void STControlEngine::addGrouptoDB(DBInquire *startInq, grpData inGrpData, IFtypes crtIFtype){
  // create group inquire with ROD entries
  QApplication::setOverrideCursor(Qt::WaitCursor);    
  PixLib::DBInquire *grpInq=0;
  try{
    if(crtIFtype==tRCCVME)
      grpInq   = PixLib::newGroupWiz(inGrpData.myROD,startInq);
    else if(crtIFtype==tUSBSys){
      QString grpName="USB-board";
      if(inGrpData.myROD.slot>=0) grpName += "_"+QString::number(inGrpData.myROD.slot);
      grpInq   = PixLib::newGroupWiz("USBPixController", grpName.toLatin1().data(), startInq);
      recordIterator r=grpInq->recordBegin();
      for( ; (r!=grpInq->recordEnd()) && ((*r)->getName()!="PixController"); r++) {}
      if(r!=grpInq->recordEnd()) {
	if(inGrpData.myROD.slot>=0){ // ID specified in wizard, must write to cfg.
	  DBField *ctrlField = grpInq->getDB()->makeField("general_BoardID");
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,inGrpData.myROD.slot);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	}
	if(inGrpData.myBOC.mode>=0){ // 2-FE-module: 2nd board must be declared
	  DBField *ctrlField = grpInq->getDB()->makeField("general_Board2ID");
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,inGrpData.myBOC.mode);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	}
	if(inGrpData.myROD.IPfile!=""){
	  DBField *ctrlField = grpInq->getDB()->makeField("general_FirmwareFile");
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,inGrpData.myROD.IPfile);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	}
	if(inGrpData.myROD.IDfile!=""){
	  DBField *ctrlField = grpInq->getDB()->makeField("general_uCFirmwareFile");
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,inGrpData.myROD.IDfile);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	}
	if(inGrpData.myROD.mode>=0){ // must set adapter board type in cfg. item
	  int actype = 0;
    switch (inGrpData.myROD.mode)
    {
      case 0:
      case 3:
        actype = 0;
        break;
      case 1:
      case 2:
        actype = 1;
        break;
      case 4:
        actype = 2;
        break;
    }
	  DBField *ctrlField = grpInq->getDB()->makeField("general_AdapterCardFlavor");
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,actype);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	  bool multiFe = false;
	  if(inGrpData.myROD.mode>1){
	    multiFe = true;
	    for(int iFe=0; iFe<4; iFe++){
	      // to do: set BI channels on for present FEs
	      std::stringstream sv;
	      sv << "general_readoutChannelsInput";
	      sv << iFe;
	      ctrlField = grpInq->getDB()->makeField(sv.str());
	      int svval = (iFe<inGrpData.myMods[0].assyType)?1:0;
	      grpInq->getDB()->DBProcess(ctrlField,COMMIT,svval);
	      (*r)->pushField(ctrlField);
	      delete ctrlField;
	      grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	    }
	  }
	  ctrlField = grpInq->getDB()->makeField("general_MultiChipWithSingleBoard");
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,multiFe);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
	  
    ctrlField = grpInq->getDB()->makeField("general_OverrideEnableDemux");
    bool enable_demux = (inGrpData.myROD.mode == 3);
	  grpInq->getDB()->DBProcess(ctrlField,COMMIT,enable_demux);
	  (*r)->pushField(ctrlField);
	  delete ctrlField;
	  grpInq->getDB()->DBProcess(*r,COMMITREPLACE);
    
    for(int iFe=0; iFe<4; iFe++){
      // to do: set BI channels on for present FEs
      std::stringstream sv;
      sv << "general_readoutChannelReadsChip";
      sv << iFe;
      ctrlField = grpInq->getDB()->makeField(sv.str());
      int assoc = 0;
      if (inGrpData.myROD.mode <= 1)
      {
        assoc = iFe;
      }
      else
      {
        assoc = -2;
      }
      grpInq->getDB()->DBProcess(ctrlField, COMMIT, assoc);
      (*r)->pushField(ctrlField);
      delete ctrlField;
      grpInq->getDB()->DBProcess(*r, COMMITREPLACE);
    }
	}
      }
    } else {
      QString grpName="Group";
      if(inGrpData.myROD.slot>=0) grpName += "_"+QString::number(inGrpData.myROD.slot);
      grpInq   = PixLib::newGroupWiz(inGrpData.myROD.Xfile.c_str(), grpName.toLatin1().data(), startInq);
//       QApplication::restoreOverrideCursor();
//       std::stringstream msg;
//       msg << "STControlEngine::addGrouptoDB : unknown interface tpye encountered while creating module group DB entry. ";
//       toErrLog(msg.str().c_str());
//       return;
    }
  }catch(PixDBException &dbexc){
    std::stringstream txt, exmsg;
    dbexc.what(exmsg);
    txt << "STControlEngine::addGrouptoDB : PixDB-exception " <<exmsg.str() << " while creating module group DB entry";
    toErrLog(txt.str());
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addGrouptoDB : exception while creating module group DB entry: " << exc;
    toErrLog(msg.str().c_str());
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog("STControlEngine::addGrouptoDB : unknown exception while creating module group DB entry");
    return;
  }
  // add BOC if desired
  if(inGrpData.myBOC.haveBoc){
    try{
      PixLib::addBocToGroup(inGrpData.myBOC, grpInq);
    }catch(SctPixelRod::BaseException& exc){
      QApplication::restoreOverrideCursor();
      std::stringstream msg;
      msg << "STControlEngine::addGrouptoDB : exception while creating BOC in module group DB entry: " << exc;
      toErrLog(msg.str());
      return;
    }catch(...){
      QApplication::restoreOverrideCursor();
      toErrLog("STControlEngine::addGrouptoDB : unknown exception while creating BOC in module group DB entry");
      return;
    }
  }
  // add modules
  int incr=1;
  if(inGrpData.cfgType==3) incr = inGrpData.myMods[0].assyType; // i-th module contains info about i-th FE to be added to 1st module
  for(unsigned int i=0;i<inGrpData.myMods.size();i+=incr){
    try{
      PixConfDBInterface *myDB = grpInq->getDB();
      if(inGrpData.cfgType<0 || (inGrpData.cfgType==1 && inGrpData.myMods[i].connName=="")){ // default, empty config
	std::string decName;
	decName = grpInq->getDecName() + inGrpData.myMods[i].modname;
	DBInquire *modInq = myDB->makeInquire("PixModule",decName);
	grpInq->pushRecord(modInq);
	myDB->DBProcess(grpInq,COMMITREPLACE);
	myDB->DBProcess(modInq,COMMIT);
	// define FE flavour and set PP0-type to USB
	DBField *newField = myDB->makeField("general_FE_Flavour");
	std::string name, cname;
	switch(inGrpData.myMods[i].assyID){
	case 0:
	  name = "FE_I2";
	  cname= "PixFeI2";
	  break;
	case 1:
	  name = "FE_I4A";
	  cname= "PixFeI4A";
	  break;
	case 2:
	  name = "FE_I4B";
	  cname= "PixFeI4B";
	  break;
	default:
	  name = "???";
	  cname= "PixFeI1";
	  break;
	}
	myDB->DBProcess(newField,COMMIT,name);
	modInq->pushField(newField);
	delete newField;
	myDB->DBProcess(modInq,COMMITREPLACE);
	if(crtIFtype==tUSBSys){
	  //PixModule::USB;
	  newField = myDB->makeField("pp0_Type");
	  name="USB";
	  myDB->DBProcess(newField,COMMIT,name);
	  modInq->pushField(newField);
	  delete newField;
	  myDB->DBProcess(modInq,COMMITREPLACE);
	}
	// set module ID correctly
	int modId = inGrpData.myMods[i].modID;
	newField = myDB->makeField("general_ModuleId");
	myDB->DBProcess(newField,COMMIT, modId);
	modInq->pushField(newField);
	delete newField;
	myDB->DBProcess(modInq,COMMITREPLACE);
	// add FE inquire(s) and label its/their type
	for(int iFe=0; iFe<inGrpData.myMods[i].assyType; iFe++){
	  std::stringstream a;
	  a<< iFe;
	  decName = modInq->getDecName() + "PixFe_"+a.str();
	  DBInquire *feInq = myDB->makeInquire("PixFe",decName);
	  modInq->pushRecord(feInq);
	  myDB->DBProcess(modInq,COMMITREPLACE);
	  myDB->DBProcess(feInq,COMMIT);
	  newField = myDB->makeField("ClassInfo_ClassName");
	  myDB->DBProcess(newField,COMMIT,cname);
	  feInq->pushField(newField);
	  delete newField;
	  myDB->DBProcess(feInq,COMMITREPLACE);
	  newField = myDB->makeField("Misc_Index");
	  myDB->DBProcess(newField,COMMIT,iFe);
	  feInq->pushField(newField);
	  delete newField;
	  myDB->DBProcess(feInq,COMMITREPLACE);
	  newField = myDB->makeField("Misc_Address");
	  myDB->DBProcess(newField,COMMIT,iFe);
	  feInq->pushField(newField);
	  delete newField;
	  myDB->DBProcess(feInq,COMMITREPLACE);
	  // add inquires for global, pixel and trim registers
	  decName = feInq->getDecName() + "GlobalRegister_0";
	  DBInquire *regInq = myDB->makeInquire("GlobalRegister",decName);
	  feInq->pushRecord(regInq);
	  myDB->DBProcess(feInq,COMMITREPLACE);
	  myDB->DBProcess(regInq,COMMIT);
	  decName = feInq->getDecName() + "PixelRegister_0";
	  regInq = myDB->makeInquire("PixelRegister",decName);
	  feInq->pushRecord(regInq);
	  myDB->DBProcess(feInq,COMMITREPLACE);
	  myDB->DBProcess(regInq,COMMIT);
	  decName = feInq->getDecName() + "Trim_0";
	  regInq = myDB->makeInquire("Trim",decName);
	  feInq->pushRecord(regInq);
	  myDB->DBProcess(feInq,COMMITREPLACE);
	  myDB->DBProcess(regInq,COMMIT);
	}
      } else if(inGrpData.cfgType==0){
	// create config from TurboDAQ file
	std::vector<DBInquire*> modInq = newModWiz(inGrpData.myMods, grpInq, m_plPath.toLatin1().data());
	for(std::vector<DBInquire*>::iterator IT = modInq.begin(); IT!=modInq.end();IT++){
	  fieldIterator f;
	  f = (*IT)->findField("pp0_Type");
	  std::string new_value = "USB";
	  if(f!=(*IT)->fieldEnd()) myDB->DBProcess(*f,COMMIT,new_value);
	  // final commit
	  myDB->DBProcess((*IT),COMMITREPLACE);
	}
      } else if(inGrpData.cfgType==2 || (inGrpData.cfgType==3 && inGrpData.myMods[i+1].connName!="")){
	// copy config from RootDB file
	std::string decName;
	decName = grpInq->getDecName() + inGrpData.myMods[i].modname;
	DBInquire *modInq = myDB->makeInquire("PixModule",decName);
	grpInq->pushRecord(modInq);
	myDB->DBProcess(grpInq,COMMITREPLACE);
	myDB->DBProcess(modInq,COMMIT);
	// open file from which to copy and get module inquire
	DBInquire *cpModInq=0;
	PixConfDBInterface *modDB = DBEdtEngine::openFile(inGrpData.myMods[i].fname.c_str());
	std::vector<DBInquire*> ti = modDB->DBFindRecordByName(PixLib::BYDECNAME,inGrpData.myMods[i].connName+"PixModule");
	if(ti.size()==1) cpModInq=ti[0];
	if(cpModInq!=0){
	  // copy all inquires and fields into new module inquire
	  for(fieldIterator fit=cpModInq->fieldBegin(); fit!=cpModInq->fieldEnd(); fit++)
	    if((*fit)->getName()!="general_ModuleId") insertDBField(*fit, modInq);
	  // set module ID correctly
	  int modId = inGrpData.myMods[i].modID;
	  DBField *newField = myDB->makeField("general_ModuleId");
	  myDB->DBProcess(newField,COMMIT, modId);
	  modInq->pushField(newField);
	  delete newField;
	  myDB->DBProcess(modInq,COMMITREPLACE);
	  int nFeRec=0;
	  for(recordIterator rit=cpModInq->recordBegin();rit!=cpModInq->recordEnd();rit++){
	    if((*rit)->getName() == "PixFe"){
	      if(inGrpData.cfgType!=3 || nFeRec<1) // if 2 FE cfg. are combined, must not have more than 1 FE from this cfg.
		insertDBInquire(*rit, modInq);
	      nFeRec++;
	    } else
	      insertDBInquire(*rit, modInq);
	  }
	}
	delete modDB;
	if(inGrpData.cfgType==3){ // add cfg. of 2nd FE to existing 
	  for(int j=1; j<inGrpData.myMods[i].assyType; j++){
	    PixConfDBInterface *modDB = DBEdtEngine::openFile(inGrpData.myMods[i+j].fname.c_str());
	    std::vector<DBInquire*> ti = modDB->DBFindRecordByName(PixLib::BYDECNAME,inGrpData.myMods[i+j].connName+"PixModule");
	    if(ti.size()==1) cpModInq=ti[0];
	    if(cpModInq!=0){
	      // copy FE inquire into new module's 2nd FE inquire
	      for(recordIterator rit=cpModInq->recordBegin();rit!=cpModInq->recordEnd();rit++){
		if((*rit)->getName() == "PixFe"){
		  std::stringstream a;
		  a << j;
		  insertDBInquire(*rit, modInq, true, ("PixFe_"+a.str()).c_str());
		  ti = myDB->DBFindRecordByName(PixLib::BYDECNAME,modInq->getDecName()+"PixFe_"+a.str()+"/PixFe");
		  if(ti.size()==1){
		    // creect j-th FE's index and geographical address
		    fieldIterator fit = ti[0]->findField("Misc_Index");
		    if(fit!=ti[0]->fieldEnd()) myDB->DBProcess(*fit,COMMIT,j);			      
		    fit = ti[0]->findField("Misc_Address");
		    if(fit!=ti[0]->fieldEnd()) myDB->DBProcess(*fit,COMMIT,j);			      
		    myDB->DBProcess(ti[0],COMMITREPLACE);
		  }
		  break;
		}
	      }
	    }
	    delete modDB;
	  }
	}
      } else{
	std::stringstream txt;
	txt << inGrpData.cfgType;
	toErrLog("STControlEngine::addGrouptoDB : unknown config type in USBPix mode: "+txt.str());
      }
    }catch(PixDBException &dbexc){
      std::stringstream txt;
      txt << "STControlEngine::addGrouptoDB : PixDB-exception " << dbexc.message << " while handling file " + inGrpData.myMods[i].fname +
	", adding module group DB entry.";
      toErrLog(txt.str());
    }catch(SctPixelRod::BaseException& exc){
      QApplication::restoreOverrideCursor();
      std::stringstream msg;
      msg << "STControlEngine::addGrouptoDB : exception while handling file " + inGrpData.myMods[i].fname +
	", adding module group DB entry: " << exc;
	toErrLog(msg.str());
	return;
    }catch(...){
      QApplication::restoreOverrideCursor();
      toErrLog("STControlEngine::addGrouptoDB : unknown exception handling file " + inGrpData.myMods[i].fname +
	       ", adding module group DB entry");
      return;
    }
  }

//   }else{
//     if(inGrpData.cfgType){ // RootDB type input
//       for(unsigned int i=0;i<inGrpData.myMods.size();i++){
// 	try{
// 	  PixConfDBInterface *modDB = DBEdtEngine::openFile(inGrpData.myMods[i].fname.c_str());
// 	  PixConfDBInterface *myDB = grpInq->getDB();
// 	  DBInquire *root    = modDB->readRootRecord(1);
// 	  DBInquire *appInq  = *(root->recordBegin());
// 	  DBInquire *grpInq2 = *(appInq->recordBegin());
// 	  DBInquire *modInq  = *(grpInq2->recordBegin());
// 	  std::string decName;
// 	  decName = grpInq->getDecName() + inGrpData.myMods[i].modname;
// 	  DBInquire *modInq2 = myDB->makeInquire("PixModule",decName);
// 	  grpInq->pushRecord(modInq2);
// 	  myDB->DBProcess(grpInq,COMMITREPLACE);
// 	  myDB->DBProcess(modInq2,COMMIT);
// 	  // call resp. function for all depending fields
// 	  for(fieldIterator fit=modInq->fieldBegin(); fit!=modInq->fieldEnd(); fit++)
// 	    insertDBField(*fit, modInq2);
// 	  // call this function for all depending inquires
// 	  for(recordIterator rit=modInq->recordBegin();rit!=modInq->recordEnd();rit++)
// 	    insertDBInquire(*rit, modInq2);
// 	  fixModule(modInq2, inGrpData.myMods[i]);
// 	  delete modDB;
// 	}catch(SctPixelRod::BaseException& exc){
// 	  QApplication::restoreOverrideCursor();
// 	  std::stringstream msg;
// 	  msg << "STControlEngine::addGrouptoDB : exception while handling file " + inGrpData.myMods[i].fname +
// 	    " while adding module group DB entry: " << exc;
// 	  toErrLog(msg.str().c_str());
// 	  return;
// 	}catch(...){
// 	  QApplication::restoreOverrideCursor();
// 	  toErrLog("STControlEngine::addGrouptoDB : unknown exception handling file " + inGrpData.myMods[i].fname +
// 		   " while adding module group DB entry");
// 	  return;
// 	}
//       }
//     } else{ // TurboDAQ input
//       try{
// 	newModWiz(inGrpData.myMods, grpInq, m_plPath.toLatin1().data());
//       }catch(SctPixelRod::BaseException& exc){
// 	QApplication::restoreOverrideCursor();
// 	std::stringstream msg;
// 	msg << "STControlEngine::addGrouptoDB : exception while creating modules in module group DB entry: " << exc;
// 	toErrLog(msg.str().c_str());
// 	return;
//       }catch(...){
// 	QApplication::restoreOverrideCursor();
// 	toErrLog("STControlEngine::addGrouptoDB : unknown exception while creating modules in module group DB entry");
// 	return;
//       }
//     }
//   }
  QApplication::restoreOverrideCursor();
}
/** loads objects according to entries in DB file */
void STControlEngine::loadDB(const char *DBfilename){
  QString my_path = DBfilename;
  QApplication::setOverrideCursor(Qt::WaitCursor);    
  // load DB object
  try{
    PixConfDBInterface *tmp_DB = 0;
    if(my_path.right(8)!="cfg.root"){ //clumsy way to identify a root file, but OK for a start
      // assume this is an ascii DB file (TurboDaqDB), so must convert to RootDB
      std::string openopt = "CREATE";
      FILE *testf = fopen((my_path+".root").toLatin1().data(),"r");
      if(testf!=0){
        fclose(testf);
        openopt = "RECREATE";
      }
      tmp_DB = new RootDB(my_path.toLatin1().data(),openopt);
      // re-open in read-only mode to avoid corrupting the file
      delete tmp_DB;
      my_path += ".root";
    }
    // store file name
    QApplication::restoreOverrideCursor();
    setPixConfDBFname(my_path.toLatin1().data());
  }
  catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::loadDB : exception while converting TurboDaqDB file to RootDB file: " << exc;
    toErrLog(msg.str().c_str());
    setPixConfDBFname("");
    return;
  }
  catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog("STControlEngine::loadDB : unknown exception while converting TurboDaqDB file to RootDB file.");
    setPixConfDBFname("");
    return;
  }
  QApplication::restoreOverrideCursor();
  
  // load the crates, module group(s) etc
  loadCrates();

  // for backward compatibility, load san config, too
  // don't support any longer, out of use for way too long
  //  loadScanCfg();
  
}
PixConfDBInterface * STControlEngine::reopenDBforEdit(){
  QString fname = getPixConfDBFname();
  if(fname==""){
    toErrLog("STControlEngine::reopenDBforEdit : Can't find existing DB file in memory, so can't save.");
    return 0;
  }

  fname = getPixConfDBFname();
  setPixConfDBFname(""); // close current (read-only) file
  QFile::copy(fname, fname + ".temp.cfg.root");
  QApplication::setOverrideCursor(Qt::WaitCursor);
  PixConfDBInterface *myDB;
  try{
    myDB = DBEdtEngine::openFile((fname + ".temp.cfg.root").toLatin1().data(),true);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::reopenDBforEdit : exception while re-opening DB file "
        << std::string(fname.toLatin1().data()) << ": " << exc;
    toErrLog(msg.str().c_str());
    remove((fname + ".temp.cfg.root").toLatin1().data());
    return 0;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::reopenDBforEdit : unknown exception while re-opening DB file "+fname ).toLatin1().data());
    remove((fname + ".temp.cfg.root").toLatin1().data());
    return 0;
  }
  QApplication::restoreOverrideCursor();
  m_PixConfDBFname = (fname + ".temp.cfg.root").toLatin1().data();
  return myDB;
}
void STControlEngine::reloadDBafterEdit(bool getNew){
  // retrieve original filename and check if current name has ".temp" extension (mandatory in editing mode)
  QString fname = getPixConfDBFname();
  int pos = (int)fname.indexOf(".temp.cfg.root");
  if(pos>=0){
    fname.remove(pos, 14);
  } else {
    toErrLog("STControlEngine::reloadDBafterEdit : we are not in DB-file editing mode, no action performed.");
    return;
  }

  if(getNew){ 
    // remove old file, rename new(.temp) to old name
    QFile::remove(fname);
    rename((fname + ".temp.cfg.root").toLatin1().data(), fname.toLatin1().data());
    //remove((fname+".temp.cfg.root").c_str());
  } else{
    // remove .temp file, not needed any longer
    remove((fname+".temp.cfg.root").toLatin1().data());
  }
  setPixConfDBFname(fname.toLatin1().data());
  // reload DB file
  clear();
  loadDB(fname.toLatin1().data());
}
void STControlEngine::saveDB(const char *DBfilename, bool activeOnly){
  QString fname = getPixConfDBFname();

  QString path=fname + ".temp.cfg.root";
  if(DBfilename!=0) path=DBfilename;
  else if(fname==""){
    toErrLog("STControlEngine::saveDB : Can't find name of existing DB file in memory, so can't save.");
    return;
  }

  QFile::copy(fname, path);

  PixConfDBInterface *myDB=0;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  try{
    myDB = DBEdtEngine::openFile(path.toLatin1().data(),true);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::saveDB : exception while re-opening DB file " << std::string(path.toLatin1().data()) << ": " << exc;
    toErrLog(msg.str().c_str());
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::saveDB : unknown exception while re-opening DB file " + std::string(path.toLatin1().data())).c_str());
    return;
  }
  delete myDB; myDB = 0;

  // retrieve inquires and save configs
  bool gotErr = false;
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
    gotErr |= (*crIT)->saveDB(path.toLatin1().data(), activeOnly);

  QApplication::restoreOverrideCursor();
  if(gotErr) 
    toErrLog("Errors occured while saving, please check log for details.");
  else{
    // clear edit flag
    savedCfg();
    if(DBfilename==0){
      // remove old file, and rename temp. file to old file name
      QFile::remove(fname);
      rename(path.toLatin1().data(),fname.toLatin1().data());
    } else{
      // we're actually in "save as..." mode, so just change the filename used here
      fname = path;
    }
    m_PixConfDBFname = fname.toLatin1().data();
    emit cfgLabelChanged();
  }
  return;
}
void STControlEngine::savePixScan(const char *DBfilename){
  if(DBfilename==0) return;

  std::string path=DBfilename;

  PixConfDBInterface *myDB=0;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  // create file if necessary - function will throw an exception when file exists, so just ignore that
  try{
    DBEdtEngine::createFile(path.c_str());
  }catch(...){
  }
  // open file
  try{
    myDB = DBEdtEngine::openFile(path.c_str(),true);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::savePixScan : exception while opening DB file " << path << ": " << exc;
    toErrLog(msg.str().c_str());
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::savePixScan : unknown exception while opening DB file " + path).c_str());
    return;
  }

  // Save PixScan config - loop over what the engine has got
  for(std::vector<PixScan*>::const_iterator it = getPixScanCfg().begin(); 
      it != getPixScanCfg().end(); it++) {
    std::string find_name = getPixScanCfgDBName(*it);
    if( (*it) != 0 && find_name!="unknown" ){
      find_name += "/PixScan;1";
      std::vector<DBInquire*> scanInq;
      try{
        scanInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name);
      }catch(...){
        // everything is caught by the statement below
	scanInq.clear();
      }
      if(scanInq.size()==0){
        // add new inquire for this config
        std::string name="PixScan";
        try{
          PixLib::DBInquire *root = myDB->readRootRecord(1);
          std::string decName = root->getDecName() + getPixScanCfgDBName(*it);
          scanInq.push_back(myDB->makeInquire(name, decName));
          root->pushRecord(scanInq[0]);
          myDB->DBProcess(root,COMMITREPLACE);
          myDB->DBProcess(scanInq[0],COMMIT);
          setPixScanCfgDBName(*it, decName.c_str());
        } catch(...){
          QApplication::restoreOverrideCursor();
          toErrLog(("STControlEngine::savePixScan : could not find nor create DB object for PixScan "
                    +find_name+" -> can't save.").c_str());
        }
        try{
          scanInq.clear();
          // must close and re-open file to make this work
          delete myDB;
          myDB = DBEdtEngine::openFile(path.c_str(),true);
          find_name = getPixScanCfgDBName(*it);
          if( (*it) != 0 && find_name!="unknown" ){
            find_name += "/PixScan;1";
            scanInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name);
          }
        } catch(SctPixelRod::BaseException& exc){
          QApplication::restoreOverrideCursor();
          std::stringstream msg;
          msg << "STControlEngine::savePixScan : exception while re-opening DB file " << path
              << " after PixScan config creation: " << exc;
          toErrLog(msg.str().c_str());
        } catch(...){
          QApplication::restoreOverrideCursor();
          toErrLog(("STControlEngine::savePixScan : unknown exception while re-opening DB file "
                    +path+" after PixScan config creation.").c_str());
        }
      } else{
        if(scanInq.size()!=1)
          toLog(("STControlEngine::savePixScan : got several PixScan configs for "
                 +find_name+".Will use first.").c_str());
      }
      if(scanInq.size()>0){
        try{
          (*it)->writeConfig( scanInq[0] );
        }
        catch(SctPixelRod::BaseException& exc){
          QApplication::restoreOverrideCursor();
          std::stringstream msg;
          msg << "STControlEngine::savePixScan : exception while saving PixScan config: "
              << exc;
          toErrLog(msg.str().c_str());
          return;
        }
        catch(...){
          QApplication::restoreOverrideCursor();
          toErrLog("STControlEngine::savePixScan : unknown exception while saving PixScan config.");
          return;
        }
      }
    }
  }
  // close temporary file
  delete myDB;
  QApplication::restoreOverrideCursor();
  
}
/** Loads PixModuleGroup */
void STControlEngine::loadCrates()
{
  // if in single crate mode, we only have one crate for now, but need to make sure it is initialized
  if( m_singleCrateMode && m_sTRodCrates.size() < 1 ) {
    toErrLog("STControlEngine::loadModuleGroup : can't load module group because we don't have any crates.");
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);    
  PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(getPixConfDBFname(), false); 
  DBInquire* root=0;
  try{
    root = confDBInterface->readRootRecord(1);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << exc;
    toErrLog(("STControlEngine::loadModuleGroup : can't get root record: "+msg.str()).c_str());
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog("STControlEngine::loadModuleGroup : can't get root record");
    return;
  }
  if( root == 0 ){
    QApplication::restoreOverrideCursor();
    toErrLog("STControlEngine::loadModuleGroup: can't find root record in DB" );
    return;
  }
  QApplication::restoreOverrideCursor();

  // loop over DB entries and create a PixModuleGroup when an according entry is found
  for(recordIterator appIter = root->recordBegin();appIter!=root->recordEnd();appIter++){
    if((int)(*appIter)->getName().find("application")!=(int)std::string::npos){
      if(m_singleCrateMode){
	m_sTRodCrates[0]->loadDB(getPixConfDBFname(), (*appIter)->getDecName().c_str());// gets all module groups from *appIter
	m_IFtype = m_sTRodCrates[0]->getIFtype();
      }else{ // create crate according to DB name
	std::string cname = (*appIter)->getDecName();
	// remove heading and trailing "/"
	cname.erase(0,1);
	cname.erase(cname.length()-1,1);
	//	addCrate(cname.c_str()).loadDB(*appIter);// gets all module groups from *appIter
	addCrate(cname.c_str()).loadDB(getPixConfDBFname(), (*appIter)->getDecName().c_str());// gets all module groups from *appIter
      }
    }
  }
  emit configChanged();

  // close file
  delete confDBInterface;
  return;
}
void STControlEngine::readRodBuff(std::string msg){
  // user message
  if(msg!="") m_log.buffers() << msg.c_str();

  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ )
    (*crate)->readRodBuff();

  return;
}
void STControlEngine::toRodBuff(std::string msg, bool error_in){
  if(error_in && m_showRodWin)
    m_log.errBuffers() << (msg+"\n").c_str();
  else
    m_log.buffers() << (msg+"\n").c_str();
  if(error_in){
    std::string eudaq_msg = "STControlEngine::toRodBuff: error message from PixController";
    emit error(eudaq_msg);
  }
}
void STControlEngine::toLog(std::string msg){
  // write to log container
  m_log.log() << (msg+"\n").c_str();
}
void STControlEngine::toErrLog(std::string msg){
  // write to log container as error
  if(m_showErrWin)
    m_log.errLog() << (msg+"\n").c_str();
  else
    m_log.log() << (msg+"\n").c_str();
  //stopping toolpanel-loop.
  emit breakloop();
  // emit additional error signal for futher processing e.g. for eudaq
  emit error( msg );
}

void STControlEngine::loadScanCfg(const char *fname){

  PixConfDBInterface *tmpDB = 0;

  QString file_name;
  if(fname==0)
    file_name = getPixConfDBFname();
  else{
    file_name = fname;
  }
  tmpDB = DBEdtEngine::openFile(file_name.toLatin1().data());

  if( tmpDB == 0 ) {
    toLog("[WARNING] STControlEngine::loadScanCfg() :\n"
                      "[WARNING] No PixConfDBInterface present. Returning. \n");
    return;
  }

  // clear existing PixScan records
  for(std::vector<PixScan*>::const_iterator pcfgIT = m_knownPixScanCfgs.begin(); pcfgIT!=m_knownPixScanCfgs.end();pcfgIT++)
    delete (*pcfgIT);
  m_knownPixScanCfgs.clear();
  m_knownPSDBcfgs.clear();

  // get root record
  DBInquire* root=0;
  try{
    root = tmpDB->readRootRecord(1);
  }catch(SctPixelRod::BaseException& exc){
    std::stringstream msg;
    msg << exc;
    toErrLog("STControlEngine::loadScanCfg() : Can't get root record: "+msg.str());
    if(fname!=0)
      delete tmpDB;
    return;
  }catch(...){
    toErrLog("STControlEngine::loadScanCfg() : Can't get root record");
    if(fname!=0)
      delete tmpDB;
    return;
  }
  std::stringstream msg;
  if( root == 0 ){
    toLog("[WARNING] STControlEngine::loadScanCfg() :\n"
                      "[WARNING] Can not find root record in database. Returning. \n");
    if(fname!=0)
      delete tmpDB;
    return;
  }


  // get a record iterator
  recordIterator it = root->recordBegin();
  if( *it == 0 ){
    toLog("[WARNING] STControlEngine::loadScanCfg() :\n"
                      "[WARNING] Can not find any records in database. Returning. \n");
    if(fname!=0)
      delete tmpDB;
    return;
  }


  // load the scan config items into vetor containers
  recordIterator itEnd = root->recordEnd();
  for( it = root->recordBegin(); it != itEnd; it++) {
    if( (*it)->getName() == "PixScan" ) {
      PixScan *scanCfg = new PixScan(*it);
      m_knownPixScanCfgs.push_back(scanCfg);
      m_knownPSDBcfgs.insert(std::make_pair((long int)scanCfg,(*it)->getDecName()));
    }
  }

  delete tmpDB;

  emit scanConfigChanged();
    
  return;
}
void STControlEngine::newPixScanCfg(const char *cfgName, PixScan *copy_from_this){
  std::string name = cfgName;
  PixScan *scanCfg = new PixScan();
  if(copy_from_this!=0)
    scanCfg->config() = copy_from_this->config();
  m_knownPixScanCfgs.push_back(scanCfg);
  m_knownPSDBcfgs.insert(std::make_pair((long int)scanCfg,name));
  m_cfgEdited=true;

  emit configChanged();
    
  return;
}
void STControlEngine::clearModuleInfo(){
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate != getSTRodCrates().end(); crate++ ) 
    (*crate)->clearModuleInfo();

  return;
}

void STControlEngine::processExecute()
{
  PixLib::sleep(20);
  do{   
    PixLib::sleep(10);
    m_app->processEvents();
  }
  while (RodProcessing());
						     
  m_app->processEvents();

}
void STControlEngine::initRods()
{
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->initRods();
    } else{
      toLog("STControlEngine::initRods() : no correctly initialised base-interface found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  readRodBuff("Post-controler-init info:");	   

  return;
}
void STControlEngine::resetRods()
{
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->initRods();
    } else{
      toLog("STControlEngine::resetRods() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}
void STControlEngine::initBocs()
{
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->initBocs();
    } else{
      toLog("STControlEngine::initBocs() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  readRodBuff("Post-BOC-init info:");

  return;
}
void STControlEngine::initDcs(){
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
      (*crate)->initDcs();
  }

  emit statusChanged();

  processExecute();

  // readRodBuff("Post-DCS-init info:");

  return;
}
void STControlEngine::configModules(int cfgType, int patternType, int DCs, std::string latch){
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->configModules(cfgType, patternType, DCs, latch);
    } else{
      toLog("STControlEngine::configModules() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  //readRodBuff("Post-module-cfg info:");

  return;
}
void STControlEngine::resetModules(int type)
{
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->resetModules(type);
    } else{
      toLog("STControlEngine::resetModules() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}

void STControlEngine::triggerModules()
{
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->triggerModules();
    } else{
      toLog("STControlEngine::triggerModules() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}
void STControlEngine::getSrvRec(){
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->getSrvRec();
    } else{
      toLog("STControlEngine::getSrvRec() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}

void STControlEngine::readEPROM(){
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->readEPROM();
    } else{
      toLog("STControlEngine::readEPROM() : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}


void STControlEngine::readGADC(int type){
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->readGADC(type);
    } else{
      toLog("STControlEngine::readGADC() : no interface found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}

void STControlEngine::sendGlobalPulse(int length){
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->sendGlobalPulse(length);
    } else{
      toLog("STControlEngine::readEPROM() : no interface found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }
}

void STControlEngine::setMcc(int opt)
{
  // Loop over crates
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) {
    if((*crate)->vmeOK()){
      (*crate)->setMcc(opt);
    } else{
      toLog("STControlEngine::setMcc(int) : no VME interfaces found for crate "+(*crate)->getName()+
	    ", can't proceed");
    }
  }

  processExecute();

  return;
}

void STControlEngine::updateGUI(){
  if(m_app!=0) 
    m_app->processEvents();
}

/** Start a scan using the configuration in the function argument. */
int STControlEngine::pixScan(pixScanRunOptions scanOpts, bool start_monitor){
  
  emit sendPixScanStatus(0,0,0,0,-1,0,0,0,10);
  m_app->processEvents();

  m_checkScanCount = 0;
  for(int ic=0;ic<(int)m_sTRodCrates.size();ic++) m_sTRodCrates[ic]->m_readyForDcs=0;

  if(scanOpts.scanConfig==0) return -1; // can't work without config
     
  m_app->processEvents();               // make sure GUI really updates
  // set wait cursor
  QApplication::setOverrideCursor(Qt::WaitCursor);

  // if in source scan mode and auto-indexing of raw file is requested, 
  // check for existing file
  std::string orgRawName = scanOpts.scanConfig->getSourceRawFile().c_str();;
  if(scanOpts.indexRawFile && scanOpts.scanConfig->getSourceScanFlag()){
    QString newRname = scanOpts.fileName.c_str();
    // strip off extension
    QString rExt="";
    int spos = newRname.lastIndexOf(".");
    if(spos){
      rExt = newRname.right(newRname.length()-spos);
      newRname = newRname.left(spos);
    }
    // assemble all parts and save in scan config
    QString appName = scanOpts.anaLabel.c_str();
    appName.replace(" ","_");
    newRname += "_"+appName+".raw";
    ((ConfString&)scanOpts.scanConfig->config()["general"]["sourceRawFile"]).m_value = std::string(newRname.toLatin1().data());
  }

  // store scan options
  m_lastPixScanOpts = scanOpts;

  // copy config into all module groups
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ )
    if((*crate)->setPixScan(*(scanOpts.scanConfig))) return -2;

  // then tell the ROD about it
  // ROD text buffer dump
  readRodBuff("Pre-scan-init info (scan "+scanOpts.anaLabel+"):");
  
  // set ToolPanel inactive
  emit beganScanning();
  
  // save starting time for logfiles later
  m_lastPixScanOpts.timestampStart = ("Scan commenced at "+
                                   QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy")).toLatin1().data();
  *m_pixScanStartTime = QTime::currentTime();
  // TurboDAQ format: Scan commenced at 11:58:45 on 05-28-2004

  // if scan modifies and does *not* restore module config,
  // set edited flag accordingly
  if(!(scanOpts.scanConfig->getRestoreModuleConfig())) editedCfg();

  //m_log.buffers() << "Post-scan-init info:\n";
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ )
    (*crate)->scan(m_lastPixScanOpts);

  PixLib::sleep(10);
  
  // restore original raw name - important for iterative calls from prim. list
  ((ConfString&)scanOpts.scanConfig->config()["general"]["sourceRawFile"]).m_value = orgRawName;
  
  if(start_monitor){
    connect( m_scanTimer2, SIGNAL(timeout()), this, SLOT(FinishScan()) );
    m_scanTimer2->start( 500 );  
  }
  
  QApplication::restoreOverrideCursor();  
  m_app->processEvents();
  return 0;
}


void STControlEngine::FinishScan()
{
  //m_finishScanRunning = true;
  disconnect( m_scanTimer2, SIGNAL(timeout()), this, SLOT(FinishScan()) );
  m_scanTimer2->stop();
  m_app->processEvents();
  
  // ensure all Rods have finished.  
  if (RodProcessing()) // some scanning still in progress, wait till finished
    {
      // update GUI
      if(m_obsCrate>=0 && m_obsGrp>=0 && m_obsCrate<(int)m_sTRodCrates.size() && m_obsGrp<m_sTRodCrates[m_obsCrate]->nGroups()){
	  if (m_sTRodCrates[m_obsCrate]->getGrpScanStatus(m_obsGrp) == 1)  // set Status: waiting for other Rods
	    emit sendPixScanStatus(m_sTRodCrates[m_obsCrate]->getGrpNSteps(m_obsGrp,0),m_sTRodCrates[m_obsCrate]->getGrpNSteps(m_obsGrp,1),
				   m_sTRodCrates[m_obsCrate]->getGrpNSteps(m_obsGrp,2),m_sTRodCrates[m_obsCrate]->getGrpNMasks(m_obsGrp),
				   m_sTRodCrates[m_obsCrate]->getGrpCurrFe(m_obsGrp),m_sTRodCrates[m_obsCrate]->getGrpSRAMFillLevel(m_obsGrp),
				   m_sTRodCrates[m_obsCrate]->getGrpTriggerRate(m_obsGrp),m_sTRodCrates[m_obsCrate]->getGrpEvtRate(m_obsGrp),8);
	  else  // update ScanPanel according to Status of observed Rod
	    emit sendPixScanStatus(m_sTRodCrates[m_obsCrate]->getGrpNSteps(m_obsGrp,0),m_sTRodCrates[m_obsCrate]->getGrpNSteps(m_obsGrp,1),
				   m_sTRodCrates[m_obsCrate]->getGrpNSteps(m_obsGrp,2),m_sTRodCrates[m_obsCrate]->getGrpNMasks(m_obsGrp),
				   m_sTRodCrates[m_obsCrate]->getGrpCurrFe(m_obsGrp),m_sTRodCrates[m_obsCrate]->getGrpSRAMFillLevel(m_obsGrp),
				   m_sTRodCrates[m_obsCrate]->getGrpTriggerRate(m_obsGrp),m_sTRodCrates[m_obsCrate]->getGrpEvtRate(m_obsGrp),
				   m_sTRodCrates[m_obsCrate]->getGrpScanStatus(m_obsGrp));
	}
      else
	emit sendPixScanStatus(0,0,0,0,-1,0,0,0,9);

      // check if scan needs to be aborted
      if(m_pixScanStartTime->secsTo(QTime::currentTime())>(60*m_lastPixScanOpts.timeToAbort) && m_lastPixScanOpts.timeToAbort>0){
	m_lastPixScanOpts.timeToAbort = -1; // to mark sure abort is triggered only once
	stopPixScan();
	toLog("STControlEngine::FinishScan : Scan execution exceeded time limit, scan was thus aborted.");
      }

      // check DCS error states
      PixScan *cfg = m_lastPixScanOpts.scanConfig;
      bool anyDcsScanned =false;
      for(int il=0;il<3;il++){
	if(cfg->getLoopActive(il) && cfg->getLoopParam(il)==PixLib::PixScan::DCS_VOLTAGE) anyDcsScanned = true;
      }
      if(anyDcsScanned)
	for(std::vector<STRodCrate*>::iterator crIT = getSTRodCrates().begin(); crIT != getSTRodCrates().end(); crIT++) (*crIT)->readPixDcsErr();

      // read DCS if requested
      if(m_lastPixScanOpts.readDcs>0)
	for(std::vector<STRodCrate*>::iterator crIT = getSTRodCrates().begin(); crIT != getSTRodCrates().end(); crIT++) (*crIT)->readUsbPixDcs();

      // process scan to file writing from here for windows due to problems with ROOT file writing from threads
#ifdef WIN32
      for(std::vector<STRodCrate *>::iterator i=m_sTRodCrates.begin(); i != m_sTRodCrates.end(); i++)
	(*i)->saveScanIfDone(m_lastPixScanOpts);
#endif

      connect( m_scanTimer2, SIGNAL(timeout()), this, SLOT(FinishScan()) );
      m_scanTimer2->start( 500 );       
      
      return;
    }
  else{  // all scanning done
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    //send status "post-scan actions"
    emit sendPixScanStatus(0,0,0,2147483647,-1,0,0,0,4);
    m_app->processEvents();
    // restore DCS settings if they were altered
    for(std::vector<STRodCrate *>::iterator i=m_sTRodCrates.begin(); i != m_sTRodCrates.end(); i++)
      (*i)->restoreAfterScan();
    QApplication::restoreOverrideCursor(); 

    //load data from file to DataPanel if requested
    if(m_lastPixScanOpts.writeToFile==true && m_lastPixScanOpts.fileName!=""){// && m_lastPixScanOpts.loadToAna){
      QApplication::setOverrideCursor(Qt::WaitCursor);
      
      FILE *testDB = fopen(m_lastPixScanOpts.fileName.c_str(),"r");
      
      if(testDB!=0){

	fclose(testDB);
	
	// tell the data viewer to re-load display
	emit gotPixScanData(m_lastPixScanOpts.fileName.c_str());
	m_app->processEvents();
	
      } else{
	m_log.errLog() << ("STControlEngine::FinishScan() : ERROR loading file " 
			   + m_lastPixScanOpts.fileName + " to Data Panel \n").c_str();
      }
      QApplication::restoreOverrideCursor(); 
    }
    
    //send status "finished" 
    emit sendPixScanStatus(0,0,0,2147483647,-1,0,0,0,1);
    
    m_app->processEvents();
    
    readRodBuff("Post-scan info:");
    // m_finishScanRunning = false;
    m_app->processEvents();
    
    // set ToolPanel active again
    emit finishedScanning();
    m_app->processEvents();
    
    return;
  }
}

void STControlEngine::saveOptions(){
  QString openOpt, file = QDir::homePath() + "/.stcrc";
  FILE *f = fopen(file.toLatin1().data(),"r");
  if(f!=0){
    openOpt = "UPDATE";
    fclose(f);
  }else
    openOpt = "NEW";
  PixLib::RootDB *optf = new RootDB(file.toLatin1().data(),openOpt.toLatin1().data());
  m_options->write(optf->readRootRecord(1));
  delete optf;
}
void STControlEngine::loadOptions(){
  QString file = QDir::homePath() + "/.stcrc";
  FILE *f = fopen(file.toLatin1().data(),"r");
  if(f!=0)
    fclose(f);
  else
    return;
  PixLib::RootDB *optf = new RootDB(file.toLatin1().data());
  m_options->read(optf->readRootRecord(1));
  delete optf;
}

void STControlEngine::changedConfigs()
{
  emit crateListChanged();
  m_app->processEvents();  // make sure GUI really updates
}
int STControlEngine::CtrlStatusSummary()
{
  bool existflag = false;
  int nRods = 0;
  
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) 
    {
      if((*crate)->CtrlStatusSummary()!=-1)
	{
	  nRods+=(*crate)->CtrlStatusSummary(); // number of initialised Rods
	  existflag = true; // at least one Rod existing
	}
    }
    if (!existflag)
    return -1;
  else
    return nRods;	
}

// returns a vecor with the ids of the initialized boards
std::vector<int> STControlEngine::GetBoardIDs()
{
  std::vector<int> boards;
  std::vector<int> t_boardids;
  boards.clear();
  
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) 
    {
      if((*crate)->CtrlStatusSummary()!=-1)
		{
		  t_boardids = (*crate)->GetBoardIDs(); // IDs of board in this Rods
		  boards.insert(boards.end(), t_boardids.begin(), t_boardids.end()); 
		}
    }
  for(int i=0;i<(int)boards.size();i++)
    std::cout << "STControlEngine: initialised board " << boards[i] << std::endl;
  return boards;
}

// returns a vecor with the ids of the initialized boards
int STControlEngine::GetHitDiscCnfg()
{
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) 
    {
      if((*crate)->CtrlStatusSummary()!=-1)
		{
			return (*crate)->GetHitDiscCnfg();
		}
    }
  return 0;
}

bool STControlEngine::RodProcessing()
{
  for( std::vector<STRodCrate *>::iterator crate = getSTRodCrates().begin(); 
       crate !=  getSTRodCrates().end(); crate++ ) 
    {
      if( (*crate)->RodProcessing()==true)
	return true;
    }
	
  return false;

}
void STControlEngine::init(const char *cfg_file, bool initRODs, const char *ps_file)
{
  m_app->processEvents();               // make sure GUI really updates
  std::string cfname="", pfname="";
  if(cfg_file!=0)
    cfname = cfg_file;
  else if(m_cfgName!="")
    cfname = m_cfgPath+"/"+m_cfgName;
  if(ps_file!=0)
    pfname = ps_file;
  else if(m_psName!="")
    pfname = m_cfgPath+"/"+m_psName;

  if(cfname!=""){
    FILE *cf = fopen(cfname.c_str(), "r");
    if(cf!=0){
      fclose(cf);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      toLog("STControlEngine::init : loading default config. file " + cfname + "\n");
      loadDB(cfname.c_str());
      QApplication::restoreOverrideCursor();
      m_app->processEvents();               // make sure GUI really updates
      if(initRODs || m_initRods){
	QApplication::setOverrideCursor(Qt::WaitCursor);
	initRods();
	QApplication::restoreOverrideCursor();
      }
      if(m_initDcs){
	QApplication::setOverrideCursor(Qt::WaitCursor);
	initDcs();
	if(m_dcsOn){
	  std::vector <STRodCrate*> myCrates = getSTRodCrates();
	  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	    std::vector <PixDcs *> pd = (*crIT)->getPixDcs();
	    for(std::vector <PixDcs *>::iterator pdIT = pd.begin(); pdIT!=pd.end(); pdIT++)
	      (*pdIT)->SetState("ON");
	  }
	  emit statusChanged();
	}
	QApplication::restoreOverrideCursor();
      }
      m_app->processEvents();               // make sure GUI really updates
    } else
	toErrLog("STControlEngine::init : ERROR loading default config. file " + cfname + "\n");
  }
  if(pfname!=""){
    FILE *psf = fopen(pfname.c_str(), "r");
    if(psf!=0){
      fclose(psf);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      toLog("STControlEngine::init : loading user-def. scan config. file " + pfname + "\n");
      loadScanCfg(pfname.c_str());
      QApplication::restoreOverrideCursor();
      m_app->processEvents();               // make sure GUI really updates
    } else
	toErrLog("STControlEngine::init : ERROR loading user-def. scan config. file " + pfname + "\n");
  }
  return;
}


int STControlEngine::setTFDACs(const char *fname, bool isTDAC)
{
  int not_found=0;
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
    not_found += (*crIT)->setTFDACs(fname, isTDAC);
  if(not_found>0) editedCfg();
  return not_found;
}
int STControlEngine::setMasks(std::vector<std::string> files, std::vector<std::string> histos, int mask, std::string logicOper){
//setMasks(const char *fname, const char *hname, int mask){
  int not_found=0;
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
    not_found += (*crIT)->setMasks(files, histos, mask, logicOper);
  if(not_found>0) editedCfg();
  return not_found;
}
void STControlEngine::setVcal(float charge, bool Chigh)
{
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
    (*crIT)->setVcal(charge, Chigh);
  editedCfg();
  return;
}
void STControlEngine::incrMccDelay(float delay, bool calib)
{
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
    (*crIT)->incrMccDelay(delay, calib);
  editedCfg();
  return;
}
void STControlEngine::disableFailed()
{
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
    (*crIT)->disableFailed();

  emit statusChanged();
  m_app->processEvents();

  return;
}
void STControlEngine::launchPrimList(std::vector<PrimListItem*> list, const char *outname, int labelIndex, bool indexMode, int modTestType)
{

  if(outname==0){
    toErrLog("STControlEngine::launchPrimList : no output file name specified");
    return;
  }

  m_prlOutname = outname;
  m_prlModTestType = "";
  // temporary solution, needs fix for cfg. with >1 module
  std::string mname = "123456";
  if(modTestType>0){
    if(modTestType>1) setBurnInChan(labelIndex-1, false, false, true);
    m_prlModTestType = outname;
    std::vector <STRodCrate *> rcs = getSTRodCrates();
    if(rcs.size()>0){
      for(int igrp = 0; igrp < rcs[0]->nGroups(); igrp++){
	std::vector< std::pair<std::string, int> > modList;
	rcs[0]->getModules(igrp, modList);
	for(unsigned int im=0; im<modList.size();im++){
	  if(rcs[0]->getModuleActive(igrp, modList[im].second)){
	    mname = modList[im].first;
	    break;
	  }
	}
	getDecNameCore(mname);
	if(mname!="123456") break; // exit group loop, found active module in this grp.
      }
    }
    // make sure all needed dir's exist
    QDir mainDir(m_modPath.c_str());
    mainDir.mkpath(("M"+mname+"/data").c_str());
    mainDir.mkpath(("M"+mname+"/configs").c_str());
	mainDir.mkpath(("M"+mname+"/results").c_str());
	mainDir.mkpath(("M"+mname+"/traveller").c_str());
    mainDir.cd(("M"+mname+"/data").c_str());
    // determine lowest unused index and save into file name and test type
    int itest;
    char sitest[3];
    for(itest=99;itest>0;itest--){
      sprintf(sitest,"%02d", itest);
      m_prlOutname = mname + "_" + m_prlModTestType + std::string(sitest) + ".root";
      if(mainDir.exists(m_prlOutname.c_str())){
	int myind = itest+1; // previous index is the last unused
	sprintf(sitest,"%02d", myind);
	m_prlModTestType += std::string(sitest);
	m_prlOutname = m_modPath + "/M" + mname + "/data/" + mname + "_" + m_prlModTestType + ".root";
	break;
      }
    }
    if(itest==0){
      int myind = itest+1; // previous index is the last unused
      sprintf(sitest,"%02d", myind);
      m_prlModTestType += std::string(sitest);
      m_prlOutname = m_modPath + "/M" + mname + "/data/" + mname + "_" + m_prlModTestType + ".root";
    }
  }
  
  m_prlList.clear();
  m_prlList = list;
  m_prlIndex = -1;
  m_prlLabelIndex = labelIndex;
  m_prlIndMode = indexMode;

  // set ToolPanel inactive
  emit beganScanning();
  // set BOC and PixScan panel inactive
  emit prlScanStarted(0);

  // check if file exists, and append new data if so
  // only do first time if we're in a repeating loop
  if((m_prlLabelIndex<2 || !m_prlIndMode) && m_prlOutname!=""){
    FILE *testDB = fopen(m_prlOutname.c_str(),"r");
    if(testDB!=0){
      fclose(testDB);
	} else{
      // need new file
      try{
	RootDB *myData = new RootDB(m_prlOutname.c_str(),"NEW");
	delete myData;
      }catch(...){
	toErrLog("STControlEngine::launchPrimList : ERROR creating file " + m_prlOutname + "\n");
	// re-set scan panel
	emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
	// re-activate tool panel
	emit finishedScanning();
	m_app->processEvents();
	emit prlDone();
	return;
      }
    }
    if(modTestType>0){
      // save test type and index in case of module test mode
      RootDB *myDB = new RootDB(m_prlOutname.c_str(),"UPDATE");
      PixLib::DBInquire *root = 0;
      root = myDB->readRootRecord(1);
      fieldIterator f;
      f = root->findField("ModuleTestType");
      DBField *fi=0;
      if (f==root->fieldEnd()) {
	fi = myDB->makeField("ModuleTestType");
	f.pointsTo(fi);
	f = root->pushField(*f);
      }
      myDB->DBProcess(f,COMMIT,m_prlModTestType);
      delete fi;
      myDB->DBProcess(root,COMMITREPLACE);
      delete myDB;
    }
  }

  m_prlSkipToEnd = false;
  m_stopProcessing = false;
  connect( m_scanTimer, SIGNAL(timeout()), this, SLOT(processPrimList()) );
  m_scanTimer->start( 500 );  

  return;
}
void STControlEngine::processPrimList()
{
  // only needed for start-up, stop for the rest of this list
  disconnect( m_scanTimer, SIGNAL(timeout()), this, SLOT(processPrimList()) );
  //m_scanTimer->stop();

  // remove connection of this functuion to scanning end (will be restored if nec.)
  disconnect(this, SIGNAL(finishedScanning()), this, SLOT(processPrimList()));

  // go to next item
  m_prlIndex++;

  // at end of list? -> finish
  if(m_prlIndex>=(int)m_prlList.size() || m_stopProcessing){
    // re-set scan panel
    emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
    // re-activate tool panel
    emit finishedScanning();
    m_app->processEvents();
    emit prlDone();
    return;
  }

  if(m_prlList[m_prlIndex]==0){
    toLog("STControlEngine::processPrimList : PrimListItem-pointer is NULL, can't process.");
    processPrimList(); // needed to keep going
    return;
  }    

  // keep BOC and PixScan panels inactive - might have been set to idle by FinishScan routine
  m_app->processEvents(); // make sure GUI really updates
  emit prlScanStarted(0);
  m_app->processEvents(); // make sure GUI really updates

  emit sendPixScanStatus(0,0,0,0,-1,0,0,0,11);
  m_app->processEvents(); // make sure GUI really updates

  // send current index to prim. list panel
  emit prlInProgress(m_prlIndex);
  m_app->processEvents(); // make sure GUI really updates

  // load data or update in data viewer
  emit scanFileChanged(m_prlOutname.c_str());
  m_app->processEvents(); // make sure GUI really updates

  // skip flag raised? then process item only if explicitly set so
  if(m_prlSkipToEnd && !m_prlList[m_prlIndex]->getSkipFlag()){
    processPrimList(); // needed to keep going
    return;
  }

  bool error = false;
  std::stringstream a;
  a << m_prlLabelIndex;
  switch(m_prlList[m_prlIndex]->getType()){
  case PrimListItem::DEF_SCAN:
    error = launchPrlScans((m_prlList[m_prlIndex]->getScanLabel()+(m_prlIndMode?(" "+a.str()):"")).c_str(),m_prlOutname.c_str(),
			   0, m_prlList[m_prlIndex]->getSubType(),    m_prlList[m_prlIndex]->getScanIsI4(),
			   m_prlList[m_prlIndex]->getReadDcs(), m_prlList[m_prlIndex]->getTimeout());
    break;
  case PrimListItem::CUST_SCAN:
    error = launchPrlScans((m_prlList[m_prlIndex]->getScanLabel()+(m_prlIndMode?(" "+a.str()):"")).c_str(),m_prlOutname.c_str(), 
			   m_prlList[m_prlIndex]->getPS(),0, true, m_prlList[m_prlIndex]->getReadDcs(), 
			   m_prlList[m_prlIndex]->getTimeout());
    break;
  case PrimListItem::TOOL:{
    error = launchPrlTools();
	break;}
  case PrimListItem::CHIP_TEST:{
    clearModuleInfo();
    runChipTest(m_prlList[m_prlIndex]->getCT(), -1,-1,-1);
    // save result to file
    PixConfDBInterface *myData=0;
    DBInquire *root=0, *testInq=0, *grpInq=0, *modInq=0;
    if(m_prlOutname!=""){
      // open data file
      myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
      root = myData->readRootRecord(1); 
      // check if this scan already has an inquire, and create one if not
      testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel()+ (m_prlIndMode?(" "+a.str()):"") + "/ChipTestResult"));
      if(testInq==0){
	std::string name, decName, myDecName;
	name="ChipTestResult";
	decName = root->getDecName() + m_prlList[m_prlIndex]->getLabel()+ (m_prlIndMode?(" "+a.str()):"");
	testInq = myData->makeInquire(name, decName);
	root->pushRecord(testInq);
	// process
	myData->DBProcess(root,COMMITREPLACE);
	myData->DBProcess(testInq,COMMIT);
	delete myData;
	myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	root = myData->readRootRecord(1);
	testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ChipTestResult"));
      }
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	// loop over groups
	for(int iGrp=0;iGrp<(*crIT)->nGroups();iGrp++){
	  // create inquire for this group
	  std::string name, decName, myDecName;
	  name="PixModuleGroup";
	  decName = testInq->getDecName() + (*crIT)->getGrpName(iGrp);
	  grpInq = myData->makeInquire(name, decName);
	  testInq->pushRecord(grpInq);
	  // process
	  myData->DBProcess(testInq,COMMITREPLACE);
	  myData->DBProcess(grpInq,COMMIT);
	  delete myData;
	  myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	  root = myData->readRootRecord(1);
	  testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ChipTestResult"));
	  grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	  // loop over modules
	  std::vector< std::pair<std::string, int> > modList;
	  (*crIT)->getModules(iGrp, modList);
	  for(std::vector< std::pair<std::string, int> >::iterator modIT=modList.begin(); modIT!=modList.end();modIT++){
	    if((*crIT)->getModuleActive(iGrp, modIT->second)){
	      // create inquire for this module
	      std::stringstream b;
	      b << modIT->second;
	      name="Mod"+b.str();
	      decName = testInq->getDecName() + name;
	      modInq = myData->makeInquire(name, decName);
	      grpInq->pushRecord(modInq);
	      // process
	      myData->DBProcess(grpInq,COMMITREPLACE);
	      myData->DBProcess(modInq,COMMIT);
	      delete myData;
	      myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	      root = myData->readRootRecord(1);
	      testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ChipTestResult"));
	      grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	      modInq = *(grpInq->findRecord(name+"/"+name));
	      // get chip test info
	      modStatus ms = (*crIT)->getPixModuleStatus(iGrp, modIT->second);
	      // dump info and pass/fail flag to DB file
	      RootDBField *infoField = new RootDBField("Result",myData);
	      std::string femsg = "";
	      int nFe=0;
	      for(int chipID=0;chipID<20;chipID++) {
		Config &conf = (*crIT)->getPixModuleChipConf(iGrp, modIT->second, chipID);
		if(conf.name()=="dummychip") break; // end of chips reached
		nFe++;
	      }
	      for(int chipID=0;chipID<nFe;chipID++) {
		std::stringstream c;
		c << chipID;
		if(nFe>1) femsg += "*** FE " + c.str() + "\n";
		femsg += ms.feMsg[chipID];
	      }
	      myData->DBProcess(infoField,COMMIT,femsg);
	      modInq->pushField(infoField);
	      delete infoField;
	      myData->DBProcess(modInq,COMMITREPLACE);    
	      bool modPass = (ms.feStat[0]==tOK);
	      if(!modPass && ((ConfBool&)m_prlList[m_prlIndex]->config()["specific"]["raiseSkipIfFailed"]).value()) 
		m_prlSkipToEnd = true;
	      RootDBField *passField = new RootDBField("PassFlag",myData);
	      myData->DBProcess(passField,COMMIT,modPass);
	      modInq->pushField(passField);
	      delete passField;
	      myData->DBProcess(modInq,COMMITREPLACE);    
	      delete myData;
	      myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	      root = myData->readRootRecord(1);
	      testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ChipTestResult"));
	      grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	    }
	  }
	}
      }
      delete myData;
    }
    // done, process next prl item
    processPrimList();
    break;}
  default:
    toLog("STControlEngine::processPrimList : unknown/unhandled type for item "+m_prlList[m_prlIndex]->getLabel()+".");
    processPrimList(); // needed to keep going
    return;
  }    

  // handle errors
  if(error && m_prlList[m_prlIndex]->getAbort()){
    toErrLog("STControlEngine::processPrimList : error while starting item \""+ m_prlList[m_prlIndex]->getLabel() + 
	     "\" - aborting prim. list execution.");
    // re-set scan panel
    emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
    // re-activate tool panel
    emit finishedScanning();
    m_app->processEvents();
    emit prlDone();
    return;
  } else if(error){
    toLog("STControlEngine::processPrimList : error while starting item \""+m_prlList[m_prlIndex]->getLabel()+"\".");
    processPrimList(); // needed to keep going
  }

  return;
}
bool STControlEngine::launchPrlScans(const char *label, const char *fname, PixScan *cfg, int type, bool scanIsI4, 
				     int readDcs, int timeToAbort)
{

  std::string scan_label = label;

  if(fname==0 || (fname!=0 && std::string(fname)=="")){
    toLog("STControlEngine::processPrlScans : no output file name specified, won't start scan "+scan_label);
    return true;
  }

  PixScan *myCfg = cfg;
  if(cfg==0)
    myCfg = new PixScan((PixLib::PixScan::ScanType)type, scanIsI4?PixModule::PM_FE_I4A:PixModule::PM_FE_I2);
  
  if(CtrlStatusSummary()==0){
    toLog("STControlEngine::processPrlScans : can't find any initialised RODs, won't start scan "+scan_label);
    if(cfg==0)
      delete myCfg;
    return true;
  }

  // set PixScanPanel view correctly
  int sctype = 2*(int)myCfg->getSourceScanFlag();
  if(sctype==2) sctype += (int)myCfg->getLoopActive(0);
  // type = 4 foreseen for TB in scanRunning - where do we get that info from?  - why was this never implemented?
  if(sctype==0 && !myCfg->getDspMaskStaging()) sctype = 1;
  emit prlScanStarted(sctype);

  // simple stuff to start with - needs to take care of a few more options
  pixScanRunOptions myscopt;
  myscopt.scanConfig = myCfg;
  myscopt.loadToAna = false;
  myscopt.anaLabel = scan_label;
  myscopt.writeToFile = (fname!=0);
  if(myscopt.writeToFile)
    myscopt.fileName = fname;
  else
    myscopt.fileName = "";
  myscopt.determineSDSPcfg = true;
  myscopt.stdTestID = -1;
  if(cfg==0)
    myscopt.stdTestID = type;
  myscopt.runFEbyFE = false;
  myscopt.indexRawFile = myCfg->getSourceScanFlag() && (myCfg->getSrcTriggerType()!=PixScan::STROBE_SCAN ||
							myCfg->getSourceRawFile()=="generate");
  myscopt.readDcs = readDcs;
  myscopt.timeToAbort = myCfg->getSourceScanFlag()?timeToAbort:(-1);

  // start scan - in monitoring
  pixScan(myscopt); 
  if(cfg==0)
    delete myCfg;
  
  // after scan: go to next prl item
  connect(this, SIGNAL(finishedScanning()), this, SLOT(processPrimList()));
  return false;
}
bool STControlEngine::launchPrlTools()
{
  PrimListItem::PrimListTools type = (PrimListItem::PrimListTools) m_prlList[m_prlIndex]->getSubType();
  std::string fileName = m_prlOutname;
  Config &argCfg = m_prlList[m_prlIndex]->config();
  switch(type){
  case PrimListItem::INIT_ROD:
    initRods();
    break;
  case PrimListItem::INIT_DCS:
    initDcs();
    break;
  case PrimListItem::RESET_MODS:{
    int type = ((ConfInt&)argCfg["arguments"]["resetType"]).getValue();
    resetModules(type);
    break;}
  case PrimListItem::CFG_MODS:{
    int cfgType = ((ConfInt&)argCfg["arguments"]["cfgtype"]).getValue();
    int DCs     = ((ConfInt&)argCfg["arguments"]["DCtoWrite"]).getValue();
    int patType = ((ConfInt&)argCfg["arguments"]["patternType"]).getValue();
    std::string latch = ((ConfList&)argCfg["arguments"]["latch"]).sValue();
    configModules(cfgType, patType, DCs, latch);
    break;}
  case PrimListItem::SET_VCAL:
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["charge"].name()!="__TrashConfObj__" && argCfg["arguments"]["useChigh"].name()!="__TrashConfObj__")
      setVcal(((ConfFloat&)argCfg["arguments"]["charge"]).value(), ((ConfBool&)argCfg["arguments"]["useChigh"]).value());
    else
      return true; // arguments not available somehow, return an error
    break;
  case PrimListItem::LINK_CHECK:
    runLinkCheck();
    break;
  case PrimListItem::DISABLE_FAILED:
    disableFailed();
    break;
  case PrimListItem::RELOAD_CFG:
    clear();
    loadDB(m_PixConfDBFname.c_str());
    break;
  case PrimListItem::LOAD_NEW_CFG:
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["fileName"].name()!="__TrashConfObj__"){
      std::string cfgName = ((ConfString&)argCfg["arguments"]["fileName"]).value();
      clear();
      loadDB(cfgName.c_str());
    } else
      return true; // arguments not available somehow, return an error
    break;
  case PrimListItem::GET_SRVREC:{
    getSrvRec();
    std::stringstream a;
    a << m_prlLabelIndex;
    std::vector<int> srvRec;
    std::vector <STRodCrate*> myCrates = getSTRodCrates();
    // save result to file
    PixConfDBInterface *myData=0;
    DBInquire *root=0, *testInq=0, *grpInq=0, *modInq=0;
    if(m_prlOutname!=""){
      myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
      root = myData->readRootRecord(1); 
      // check if this scan already has an inquire, and create one if not
      testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel()+ (m_prlIndMode?(" "+a.str()):"") + (m_prlIndMode?(" "+a.str()):"") + "/ServiceRecord"));
      if(testInq==0){
	std::string name, decName, myDecName;
	name="ServiceRecord";
	decName = root->getDecName() + m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"");
	testInq = myData->makeInquire(name, decName);
	root->pushRecord(testInq);
	// process
	myData->DBProcess(root,COMMITREPLACE);
	myData->DBProcess(testInq,COMMIT);
	delete myData;
	myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	root = myData->readRootRecord(1);
	testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ServiceRecord"));
      }
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	// loop over groups
	for(int iGrp=0;iGrp<(*crIT)->nGroups();iGrp++){
	  (*crIT)->getSrvRecRes(iGrp, srvRec);
// 	  cout << "Srv.Rec. for group " << iGrp << endl;
// 	  for(unsigned int k=0;k<srvRec.size();k++)
// 	    cout << k << ": "<<(srvRec[k]) << endl;
	  // create inquire for this group
	  std::string name, decName, myDecName;
	  name="PixModuleGroup";
	  decName = testInq->getDecName() + (*crIT)->getGrpName(iGrp);
	  grpInq = myData->makeInquire(name, decName);
	  testInq->pushRecord(grpInq);
	  // process
	  myData->DBProcess(testInq,COMMITREPLACE);
	  myData->DBProcess(grpInq,COMMIT);
	  delete myData;
	  myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	  root = myData->readRootRecord(1);
	  testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ServiceRecord"));
	  grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	  // temporary: have only one module per group (needs to be fixed?)
	  // loop over modules
// 	  std::vector< std::pair<std::string, int> > modList;
// 	  (*crIT)->getModules(iGrp, modList);
// 	  for(std::vector< std::pair<std::string, int> >::iterator modIT=modList.begin(); modIT!=modList.end();modIT++){
	    // create inquire for this module
	    std::stringstream b;
	    b << 0;//modIT->second;
	    name="Mod"+b.str();
	    decName = testInq->getDecName() + name;
	    modInq = myData->makeInquire(name, decName);
	    grpInq->pushRecord(modInq);
	    // process
	    myData->DBProcess(grpInq,COMMITREPLACE);
	    myData->DBProcess(modInq,COMMIT);
	    delete myData;
	    myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	    root = myData->readRootRecord(1);
	    testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ServiceRecord"));
	    grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	    modInq = *(grpInq->findRecord(name+"/"+name));
	    // dump service record vector DB file
	    RootDBField *infoField = new RootDBField("Result",myData);
	    myData->DBProcess(infoField,COMMIT,srvRec);
	    modInq->pushField(infoField);
	    delete infoField;
	    myData->DBProcess(modInq,COMMITREPLACE);    
	    delete myData;
	    myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	    root = myData->readRootRecord(1);
	    testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/ServiceRecord"));
	    //	    grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	    //	  }
	}
      }
      delete myData;
    }
    break;}
  case PrimListItem::SET_FE_GR:{
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["RegName"].name()!="__TrashConfObj__" && argCfg["arguments"]["RegVal"].name()!="__TrashConfObj__"){
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	(*crIT)->setGR(((ConfList&)argCfg["arguments"]["RegName"]).sValue(),((ConfInt&)argCfg["arguments"]["RegVal"]).getValue());
	for(int iG=0;iG<(*crIT)->nGroups();iG++){
	  std::vector< std::pair<std::string, int> > mlist;
	  (*crIT)->getModules(iG, mlist);
	  for(std::vector< std::pair<std::string, int> >::iterator mIT=mlist.begin(); mIT!=mlist.end();mIT++)
	    (*crIT)->writeConfig(iG, mIT->second);
	}
      }
      editedCfg();
    } else
      return true; // arguments not available somehow, return an error
    break;}
  case PrimListItem::SET_FE_PR:{
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["RegName"].name()!="__TrashConfObj__" && argCfg["arguments"]["RegVal"].name()!="__TrashConfObj__"){
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
	(*crIT)->setPR(((ConfList&)argCfg["arguments"]["RegName"]).sValue(),((ConfInt&)argCfg["arguments"]["RegVal"]).getValue());
      editedCfg();
    } else
      return true; // arguments not available somehow, return an error
    break;}
  case PrimListItem::SEND_DCS_CMD:{
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["dcsName"].name()!="__TrashConfObj__" && argCfg["arguments"]["dcsCmd"].name()!="__TrashConfObj__"
       && argCfg["arguments"]["readResponse"].name()!="__TrashConfObj__"){
      std::string dcsName = ((ConfString&)argCfg["arguments"]["dcsName"]).value();
      std::string dcsCmd  = ((ConfString&)argCfg["arguments"]["dcsCmd"]).value();

	  bool readResp  = ((ConfBool&)argCfg["arguments"]["readResponse"]).value();
      bool devFound=false;

      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	std::vector <PixDcs *> pd = (*crIT)->getPixDcs();
	for(std::vector <PixDcs *>::iterator pdIT = pd.begin(); pdIT!=pd.end(); pdIT++)
	  if((*pdIT)->name()==dcsName){
	    devFound = true;
	    if((*pdIT)->sendCommand(dcsCmd, readResp)){
	      toLog(("STControlEngine::launchPrlTools : DCS device  "+dcsName+ " returned error on cmd; return string: " + dcsCmd ).c_str());
	      return true;
	    }
	    break;
	  }
      }
      if(!devFound){
	toLog(("STControlEngine::launchPrlTools : DCS device which should have been turned on/off wasn't found: "+dcsName ).c_str());
	return true;
      }
    }else
      return true; // arguments not available somehow, return an error
    break;
  }
  case PrimListItem::SEND_MAIL:{
	  //std::cout << "entering send mail" << std::endl;
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["smtpServerName"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["userName"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["password"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["subject"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["message"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["sender"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["senderName"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["recipient"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["recipientName"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["port"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["connectionType"].name()!="__TrashConfObj__"&&
       argCfg["arguments"]["authentificationType"].name()!="__TrashConfObj__" ){
      QString smtpServerName = QString::fromStdString(((ConfString&)argCfg["arguments"]["smtpServerName"]).value());
      QString userName  = QString::fromStdString(((ConfString&)argCfg["arguments"]["userName"]).value());
      QString password  = QString::fromStdString(((ConfString&)argCfg["arguments"]["password"]).value());
      QString subject  = QString::fromStdString(((ConfString&)argCfg["arguments"]["subject"]).value());
      QString messageText  = QString::fromStdString(((ConfString&)argCfg["arguments"]["message"]).value());
      QString sender = QString::fromStdString(((ConfString&)argCfg["arguments"]["sender"]).value());
      QString senderName  = QString::fromStdString(((ConfString&)argCfg["arguments"]["senderName"]).value());
      QString recipient  = QString::fromStdString(((ConfString&)argCfg["arguments"]["recipient"]).value());
      QString recipientName  = QString::fromStdString(((ConfString&)argCfg["arguments"]["recipientName"]).value());
      int connectionType  = ((ConfInt&)argCfg["arguments"]["connectionType"]).value();
      int port = ((ConfInt&)argCfg["arguments"]["port"]).value();
      int authentificationType  = ((ConfInt&)argCfg["arguments"]["authentificationType"]).value();

      //std::cout << "smtpServerName" << smtpServerName.toStdString() << std::endl;
      //std::cout << "userName" << userName.toStdString() << std::endl;
      //std::cout << "password" << password.toStdString() << std::endl;
      //std::cout << "subject" << subject.toStdString() << std::endl;
      //std::cout << "messageText" << messageText.toStdString() << std::endl;
      //std::cout << "sender" << sender.toStdString() << std::endl;
      //std::cout << "senderName" << senderName.toStdString() << std::endl;
      //std::cout << "recipient" << recipient.toStdString() << std::endl;
      //std::cout << "recipientName" << recipientName.toStdString() << std::endl;
      //std::cout << "connectionType" << connectionType << std::endl;
      //std::cout << "port" << port << std::endl;
      //std::cout << "authentificationType" << authentificationType << std::endl;

      bool sendingSuccessfull = true;
      int sendingError = 0;
      //std::cout << "creating object" << std::endl;
      SmtpClient smtp(smtpServerName, port, (SmtpClient::ConnectionType)connectionType);

      smtp.setAuthMethod((SmtpClient::AuthMethod)authentificationType);


      smtp.setUser(userName);
      smtp.setPassword(password);

      MimeMessage message;

      message.setSender(new EmailAddress(sender, senderName));
      message.addRecipient(new EmailAddress(recipient, recipientName));
      message.setSubject(subject);

      // Now add some text to the email.
      // First we create a MimeText object.

      MimeText text;

      text.setText(messageText);

      // Now add it to the mail

      message.addPart(&text);

      // Now we can send the mail
      //std::cout << "sending stuff" << std::endl;
      try{
    	  sendingSuccessfull = smtp.connectToHost();
    	  //std::cout << "connected to host: " << sendingSuccessfull << std::endl;
    	  sendingSuccessfull = smtp.login();
    	  //std::cout << "logged in to host: " << sendingSuccessfull << std::endl;
    	  sendingSuccessfull = smtp.sendMail(message);
    	  //std::cout << "sent message: " << sendingSuccessfull << std::endl;
    	  smtp.quit();
      }
      catch(int error){
    	  sendingSuccessfull = false;
    	  sendingError = error;
    	  //std::cout << "error has occurred: " << error << std::endl;
      }
      //std::cout << "stuff has been sent" << std::endl;
      if(!sendingSuccessfull){
    	  std::stringstream logMessage;
    	  logMessage << "STControlEngine::launchPrlTools : Sending of mail not successfull(Error: " << sendingError << ")";
    	  toLog(logMessage.str());
    	  return true;
      } else
    	  toLog("STControlEngine::launchPrlTools : Mail has been sent successfully.");
    }else
    	return true; // arguments not available somehow, return an error
    break;
  }
  case PrimListItem::DCS_PWR:
  case PrimListItem::DCS_ONOFF:
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["dcsName"].name()!="__TrashConfObj__" &&
			 argCfg["arguments"]["on_or_off"].name()!="__TrashConfObj__" && 
       argCfg["arguments"]["dcsChan"].name()!="__TrashConfObj__")
		
		{

      std::string dcsName = ((ConfString&)argCfg["arguments"]["dcsName"]).value();
      int dcsChan = ((ConfInt&)argCfg["arguments"]["dcsChan"]).getValue();
      if(dcsChan==-1) dcsChan = m_prlLabelIndex-1;
      bool on_or_off = ((ConfBool&)argCfg["arguments"]["on_or_off"]).value();
      bool devFound=false;
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	std::vector <PixDcs *> pd = (*crIT)->getPixDcs();
	for(std::vector <PixDcs *>::iterator pdIT = pd.begin(); pdIT!=pd.end(); pdIT++)
	  if((*pdIT)->name()==dcsName){
	    devFound = true;
	    if(dcsChan==-99)
	      (*pdIT)->SetState(on_or_off?"ON":"OFF");
	    else{
	      PixDcsChan *pdchan = (*pdIT)->getChan(dcsChan);
	      if(pdchan!=0) pdchan->SetState(on_or_off?"ON":"OFF");
	    }
	    if(on_or_off) {
	      try{
		(*pdIT)->SetState("UPDATE");
	      }catch(...){}
	    }
	    break;
	  }
      }
      if(!devFound){
	toLog(("STControlEngine::launchPrlTools : DCS device which should have been turned on/off wasn't found: "+dcsName ).c_str());
	return true;
      }
      emit statusChanged();
    } else
      return true; // arguments not available somehow, return an error
    break;
  case PrimListItem::DCS_SET:
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["dcsName"].name()!="__TrashConfObj__" &&
			 argCfg["arguments"]["dcsSetType"].name()!="__TrashConfObj__" && 
			 argCfg["arguments"]["dcsChan"].name()!="__TrashConfObj__" &&
			 argCfg["arguments"]["setOffset"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["setVal"].name()!="__TrashConfObj__"
			 ){

			std::string dcsName = ((ConfString&)argCfg["arguments"]["dcsName"]).value();
      std::string setType = ((ConfList&)argCfg["arguments"]["dcsSetType"]).sValue();
      float setVal = ((ConfFloat&)argCfg["arguments"]["setVal"]).value();
			float setOffset = ((ConfFloat&)argCfg["arguments"]["setOffset"]).value();
      int dcsChan = ((ConfInt&)argCfg["arguments"]["dcsChan"]).getValue();

			if(dcsChan==-1) {
						dcsChan = m_prlLabelIndex-1;
					  int PrimListIter = m_prlLabelIndex-1;
					  setVal = setVal * (PrimListIter) + setOffset;
			}else{setVal = setVal + setOffset;
			}

	  //THERESA
	  //if(dcsChan==-1) dcsChan

//			int PrimListIter = m_prlLabelIndex-1;
//	  setVal = setVal * (PrimListIter+1);

		//+ setOffset;
      
		  
		  
		  
		  
		  
		  
		  
		  
      // loop over crates
      bool chanFound = false;
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	std::vector <PixDcs *> pd = (*crIT)->getPixDcs();
	for(std::vector <PixDcs *>::iterator pdIT = pd.begin(); pdIT!=pd.end(); pdIT++){
	  for(std::vector<PixDcsChan*>::iterator dcsc = (*pdIT)->chanBegin(); dcsc!=(*pdIT)->chanEnd(); dcsc++){
	    if((*dcsc)->name()==dcsName){
	      chanFound = true;
	      Config &cfg = (*dcsc)->config();
	      std::string cfgStype = "unknown";
	      if(setType=="voltage") cfgStype = "NomVolts";
	      else if(setType=="currentlim") cfgStype = "CurrLim";
	      else if(setType=="position") cfgStype = "Position";
	      else if(setType=="maxspeed") cfgStype = "MaxSpeed";
	      else if(setType=="settemperature") cfgStype = "NomTemp";
	      else if(setType=="useexternaltempsensor") cfgStype = "ExternalTempSensor";
	      if(cfg["settings"].name()!="__TrashConfGroup__" && cfg["settings"][cfgStype].name()!="__TrashConfObj__"){
		((ConfFloat&)cfg["settings"][cfgStype]).m_value = setVal;
		(*dcsc)->SetParam(setType, setVal);
		if((*pdIT)->ReadState("")=="ON"){
		  // if device is already on, update settings on supply
		  (*pdIT)->SetState("ON");
		  try{
		    (*pdIT)->SetState("UPDATE");
		  }catch(...){}
		}
	      } else{
		toLog(("STControlEngine::launchPrlTools : can't find ConfObj for setting type "+setType+" in DCS channel "+dcsName ).c_str());
		return true;
	      }
	    }
	  }
	}
      }
      if(!chanFound){
	toLog(("STControlEngine::launchPrlTools : DCS channel from which should have been read wasn't found: "+dcsName ).c_str());
	return true;
      }
      emit statusChanged();
    } else
      return true; // arguments not available somehow, return an error
    break;
  case PrimListItem::DCS_GET:
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["dcsName"].name()!="__TrashConfObj__" && argCfg["arguments"]["dcsGetType"].name()!="__TrashConfObj__" && 
       argCfg["arguments"]["storeName"].name()!="__TrashConfObj__" && argCfg["arguments"]["minCut"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["maxCut"].name()!="__TrashConfObj__" && argCfg["arguments"]["dcsChan"].name()!="__TrashConfObj__"){
      std::stringstream a;
      a << m_prlLabelIndex;
      std::string dcsName = ((ConfString&)argCfg["arguments"]["dcsName"]).value();
      int dcsChan = ((ConfInt&)argCfg["arguments"]["dcsChan"]).getValue();
      if(dcsChan==-1) dcsChan = m_prlLabelIndex-1;
      std::string getType = ((ConfList&)argCfg["arguments"]["dcsGetType"]).sValue();
      std::string storeName = ((ConfString&)argCfg["arguments"]["storeName"]).value()+
	(m_prlIndMode?(" "+a.str()):"");
      float minCut = ((ConfFloat&)argCfg["arguments"]["minCut"]).value();
      float maxCut = ((ConfFloat&)argCfg["arguments"]["maxCut"]).value();
      PixConfDBInterface *myDB = 0;
      PixLib::DBInquire *dcsRec= 0;
      if(storeName!="" && fileName!=""){
	try{
	  myDB = new RootDB(fileName.c_str(),"UPDATE");
	}catch(SctPixelRod::BaseException& exc){
	  std::stringstream msg;
	  msg << "STControlEngine::launchPrlTools : exception while opening DB file "
	      << fileName << ": " << exc;
	  toLog(msg.str().c_str());
	  return true;
	}catch(...){
	  toLog(("STControlEngine::launchPrlTools : unknown exception while opening DB file "+fileName ).c_str());
	  return true;
	}
	std::string rname = "DCS_readings/DCS_readings";
	PixLib::DBInquire *root = myDB->readRootRecord(1);
	if(root->findRecord(rname)==root->recordEnd()){
	  try{
	    dcsRec = myDB->makeInquire("DCS_readings", "DCS_readings");
	    root->pushRecord(dcsRec);
	    myDB->DBProcess(root,COMMITREPLACE);
	    myDB->DBProcess(dcsRec,COMMIT);
	    delete myDB;
	    myDB = new RootDB(fileName.c_str(),"UPDATE");
	    root = myDB->readRootRecord(1);
	    dcsRec = *(root->findRecord(rname));
	  }catch(...){
	    toLog("STControlEngine::launchPrlTools : exception while creating a record for DCS reading in DB file");
	    delete myDB;
	    return true;
	  }
	} else
	  dcsRec = *(root->findRecord(rname));
      }
      // loop over crates
      bool chanFound = false;
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	std::vector <PixDcs *> pd = (*crIT)->getPixDcs();
	PixDcsChan *pdchan = 0;
	if(dcsChan==-99){
	  for(std::vector <PixDcs *>::iterator pdIT = pd.begin(); pdIT!=pd.end(); pdIT++){
	    for(std::vector<PixDcsChan*>::iterator dcsc = (*pdIT)->chanBegin(); dcsc!=(*pdIT)->chanEnd(); dcsc++){
	      if((*dcsc)->name()==dcsName){
		pdchan = *dcsc;
		break;
	      }
	    }
	  }
	} else{
	  for(std::vector <PixDcs *>::iterator pdIT = pd.begin(); pdIT!=pd.end(); pdIT++){
	    if((*pdIT)->name()==dcsName){
	      pdchan = (*pdIT)->getChan(dcsChan);
	      break;
	    }
	  }
	}
	if(pdchan!=0){
	  chanFound = true;
	  float readVal = pdchan->ReadParam(getType);
	  // apply cut to value and write into m_prlSkipToEnd
	  if(readVal < minCut || readVal > maxCut) m_prlSkipToEnd = true;
	  if(dcsRec!=0){
	    if(dcsRec->findField(storeName)==dcsRec->fieldEnd()){
	      try{
		DBField * newField = myDB->makeField(storeName);
		myDB->DBProcess(newField,COMMIT,readVal);
		dcsRec->pushField(newField);
		delete newField;
		myDB->DBProcess(dcsRec,COMMITREPLACE);
	      }catch(...){
		toLog("STControlEngine::launchPrlTools : exception while creating a field for DCS reading in DB file");
		delete myDB;
		return true;
	      }
	    } else{
	      toLog(("STControlEngine::launchPrlTools : field to which should be written already exists: "+storeName ).c_str());
	      delete myDB;
	      return true;
	    }
	  }
	  break;
	}
      }
      if(!chanFound){
	std::stringstream a;
	a << dcsChan;
	toLog(("STControlEngine::launchPrlTools : DCS channel from which should have been read wasn't found: "+dcsName 
	       + ", channel ID: " + a.str()).c_str());
	delete myDB;
	return true;
      }
      delete myDB;
    } else
      return true; // arguments not available somehow, return an error
    break;
  case PrimListItem::GET_TIME:{
    std::stringstream a;
    a << m_prlLabelIndex;
    std::string storeName = m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"");
    PixConfDBInterface *myDB = 0;
    PixLib::DBInquire *dcsRec= 0;
    if(storeName!="" && fileName!=""){
      try{
	myDB = new RootDB(fileName.c_str(),"UPDATE");
      }catch(SctPixelRod::BaseException& exc){
	std::stringstream msg;
	msg << "STControlEngine::launchPrlTools : exception while opening DB file "
	    << fileName << ": " << exc;
	toLog(msg.str().c_str());
	return true;
      }catch(...){
	toLog(("STControlEngine::launchPrlTools : unknown exception while opening DB file "+fileName ).c_str());
	return true;
      }
      std::string rname = "Timestamps/DCS_readings";
      PixLib::DBInquire *root = myDB->readRootRecord(1);
      if(root->findRecord(rname)==root->recordEnd()){
	try{
	  dcsRec = myDB->makeInquire("DCS_readings", "Timestamps");
	  root->pushRecord(dcsRec);
	  myDB->DBProcess(root,COMMITREPLACE);
	  myDB->DBProcess(dcsRec,COMMIT);
	  delete myDB;
	  myDB = new RootDB(fileName.c_str(),"UPDATE");
	  root = myDB->readRootRecord(1);
	  dcsRec = *(root->findRecord(rname));
	}catch(...){
	  toLog("STControlEngine::launchPrlTools : exception while creating a record for DCS reading in DB file");
	  delete myDB;
	  return true;
	}
      } else
	dcsRec = *(root->findRecord(rname));
      if(dcsRec->findField(storeName)==dcsRec->fieldEnd()){
	try{
	  std::string ts = (QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy")).toLatin1().data();
	  DBField * newField = myDB->makeField(storeName);
	  myDB->DBProcess(newField,COMMIT,ts);
	  dcsRec->pushField(newField);
	  delete newField;
	  myDB->DBProcess(dcsRec,COMMITREPLACE);
	  delete myDB;
	}catch(...){
	  toLog("STControlEngine::launchPrlTools : exception while creating a field for DCS reading in DB file");
	  delete myDB;
	  return true;
	}
      } else{
	toLog(("STControlEngine::launchPrlTools : field to which should be written already exists: "+storeName ).c_str());
	delete myDB;
	return true;
      }
    }
    break;}
  case PrimListItem::FIT_SCAN:
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["scanName"].name()!="__TrashConfObj__" && argCfg["arguments"]["histoType"].name()!="__TrashConfObj__" && 
       argCfg["arguments"]["funcType"].name()!="__TrashConfObj__" && argCfg["arguments"]["storeType"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["fitParPreset"].name()!="__TrashConfObj__" && argCfg["arguments"]["conversion"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["offset"].name()!="__TrashConfObj__" && argCfg["arguments"]["fitRangeMin"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["fitRangeMax"].name()!="__TrashConfObj__"){

      std::string scanName = ((ConfString&)argCfg["arguments"]["scanName"]).value();
      std::string storeType = ((ConfList&)argCfg["arguments"]["storeType"]).sValue();
      int histoType = ((ConfList&)argCfg["arguments"]["histoType"]).getValue();
      int funcType = ((ConfList&)argCfg["arguments"]["funcType"]).getValue();
      float offset = ((ConfFloat&)argCfg["arguments"]["offset"]).value();
      float conversion = ((ConfFloat&)argCfg["arguments"]["conversion"]).value();
      float fitRgMin = ((ConfFloat&)argCfg["arguments"]["fitRangeMin"]).value();
      float fitRgMax = ((ConfFloat&)argCfg["arguments"]["fitRangeMax"]).value();
      // loop over modules for fitting
      for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ){
	for(int iGrp=0; iGrp<(*crate)->nGroups(); iGrp++){
	  std::vector< std::pair<std::string, int> > list;
	  (*crate)->getModules(iGrp, list);
	  for(std::vector< std::pair<std::string, int> >::iterator it=list.begin(); it!=list.end(); it++){
	    if((*crate)->getModuleActive(iGrp, it->second)){ // ignore inactive modules
	      PixDBData *data = 0;
	      try{
		std::string mname = it->first;
		PixLib::getDecNameCore(mname);
		data = new PixDBData("fitting", (fileName+":/"+scanName+"/"+(*crate)->getGrpName(iGrp)).c_str(),
				     mname.c_str());
		if(histoType == (int)PixScan::DCS_DATA){
		  std::vector<float> fitPars;
		  float chi2;
		  if(((ConfList&)argCfg["arguments"]["funcType"]).sValue()=="FEI3 CapMeasure"){
		    fitPars = ((ConfVector&)argCfg["arguments"]["fitParPreset"]).valueVFloat();
		    double Uload = 2.0;
		    if(fitPars.size()>0) Uload = (double)fitPars[0];
		    std::vector<float> intermedFitPars;
		    fitPars.clear();
		    data->fitFEI3CapMeasure(fitPars, intermedFitPars, Uload);
		  } else{
		    fitPars = ((ConfVector&)argCfg["arguments"]["fitParPreset"]).valueVFloat();
		    data->fitGraph(fitPars, chi2, funcType, fitRgMin, fitRgMax,false);
		  }
		  if(storeType=="VCAL parameters"){
		    // get cfg. of FE 0 - won't work for multi-FE-modules since alwas FE index 0 is assumed
		    Config &cfg = (*crate)->getPixModuleChipConf(iGrp, it->second, 0);
		    for(unsigned int ip=0;ip<fitPars.size(); ip++) {
		      std::stringstream vcname;
		      vcname << "VcalGradient" << ip;
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"][vcname.str()].name()!="__TrashConfObj__"){
			if(ip==0) // ignore offset, this is just VDDA...
			  ((ConfFloat&)cfg["Misc"][vcname.str()]).m_value = 0.f;
			else
			  ((ConfFloat&)cfg["Misc"][vcname.str()]).m_value = fitPars[ip]*(1.e3);
			// result in V, cfg. need mV
		      }
		    }
		  } else if(storeType=="FE-I3 injection capacitances"){
		    // get cfg. of FE 0 - won't work for multi-FE-modules since alwas FE index 0 is assumed
		    Config &cfg = (*crate)->getPixModuleChipConf(iGrp, it->second, 0);
		    if(fitPars.size()==3){ // must return 3 cap. values, otherwise undef. result
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"]["CInjLo"].name()!="__TrashConfObj__")
			((ConfFloat&)cfg["Misc"]["CInjLo"]).m_value = fitPars[0];
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"]["CInjHi"].name()!="__TrashConfObj__")
			((ConfFloat&)cfg["Misc"]["CInjHi"]).m_value = fitPars[2];
		    } else{
		      delete data;
		      return true;
		    }
		  } else if(storeType=="CAP0" || storeType=="CAP1" || storeType=="CAP0+CAP1" || storeType=="all CAPx"){
		    float cap_res = 5.7f;
		    if(fitPars.size()>1) // must have used linear fit, so ar least 2 scan par's
		      cap_res = fitPars[1]*conversion+offset;
		    else{
		      delete data;
		      return true;
		    }
		    // get cfg. of FE 0 - won't work for multi-FE-modules since alwas FE index 0 is assumed
		    Config &cfg = (*crate)->getPixModuleChipConf(iGrp, it->second, 0);
		    if(storeType=="all CAPx"){ // CAP0+CAP1 was measured, scale result to all CAP's
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"]["CInjHi"].name()!="__TrashConfObj__")
			((ConfFloat&)cfg["Misc"]["CInjHi"]).m_value = cap_res;
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"]["CInjMed"].name()!="__TrashConfObj__")
			((ConfFloat&)cfg["Misc"]["CInjMed"]).m_value = cap_res/5.7*3.9;
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"]["CInjLo"].name()!="__TrashConfObj__")
			((ConfFloat&)cfg["Misc"]["CInjLo"]).m_value = cap_res/5.7*1.9;
		    } else{
		      std::string cfgName="none";
		      if(storeType=="CAP0") cfgName="CInjMed";
		      if(storeType=="CAP1") cfgName="CInjLo";
		      if(storeType=="CAP0+CAP1") cfgName="CInjHi";
		      if(cfg["Misc"].name()!="__TrashConfGroup__" && cfg["Misc"][cfgName].name()!="__TrashConfObj__")
			((ConfFloat&)cfg["Misc"][cfgName]).m_value = cap_res;
		    }
		  } // else: not stored or unknown
		} else{
		  // not yet implemented
		}
	      }catch(...){
		data = 0;
	      }
	      delete data;
	    }
	  }
	}
      }
    } else
      return true; // arguments not available somehow, return an error
    break;
  case PrimListItem::ANA_GRAPH:{
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["scanName"].name()!="__TrashConfObj__" && argCfg["arguments"]["anaAction"].name()!="__TrashConfObj__" && 
       argCfg["arguments"]["anaArgs"].name()!="__TrashConfObj__" && argCfg["arguments"]["storeType"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["FEindex"].name()!="__TrashConfObj__"){
      
      std::string scanName = ((ConfString&)argCfg["arguments"]["scanName"]).value();
      int storeType = ((ConfList&)argCfg["arguments"]["storeType"]).getValue();
      int anaAction = ((ConfList&)argCfg["arguments"]["anaAction"]).getValue();
      int feIndex =   ((ConfInt&)argCfg["arguments"]["FEindex"]).getValue();
      std::vector<float> anaArgs = ((ConfVector&)argCfg["arguments"]["anaArgs"]).valueVFloat();
      // loop over modules for analysis
      for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ){
	for(int iGrp=0; iGrp<(*crate)->nGroups(); iGrp++){
	  std::vector< std::pair<std::string, int> > list;
	  (*crate)->getModules(iGrp, list);
	  for(std::vector< std::pair<std::string, int> >::iterator it=list.begin(); it!=list.end(); it++){
	    if((*crate)->getModuleActive(iGrp, it->second)){ // ignore inactive modules
	      PixDBData *data = 0;
	      try{
		double result = 0.;
		bool bresult=true;
		std::string mname = it->first;
		PixLib::getDecNameCore(mname);
		data = new PixDBData("anagraph", (fileName+":/"+scanName+"/"+(*crate)->getGrpName(iGrp)).c_str(),
				     mname.c_str());
		if(data->validPath()){
		  TGraph *gr = data->GetGraph();
		  int npts = gr->GetN();
		  double *x = gr->GetX(), *y = gr->GetY();
		  switch(anaAction){
		  default:
		  case 1:{ // find best match
		    if(anaArgs.size()!=1){
		      // wrong # of arg's, can't proceed
		      toLog("STControlEngine::launchPrlTools : wrong number of arguments given to best match analysis, should have been 1.");
		      delete data;
		      return true;
		    }
		    int ibest = -1;
		    double target = (double)anaArgs[0], dbest=1.e10;
		    for(int ip=0;ip<npts;ip++){
		      if(fabs(dbest-target)>fabs(y[ip]-target)){
			ibest = ip;
			dbest = y[ip];
		      }
		    }
		    if(ibest>=0) result = x[ibest];
		    else{
		      toLog("STControlEngine::launchPrlTools : can't match target value in best match analysis.");
		      delete data;
		      return true;
		    }
		    break;}
		  case 2:{ // check if value is exceeded
		    if(anaArgs.size()!=1){
		      // wrong # of arg's, can't proceed
		      toLog("STControlEngine::launchPrlTools : wrong number of arguments given to best max. exceed analysis, should have been 1.");
		      delete data;
		      return true;
		    }
		    double maxval = (double)anaArgs[0];
		    for(int ip=0;ip<npts;ip++){
		      if(fabs(y[ip])>maxval){
			bresult = false;
			break;
		      }
		    }
		    break;}
		  }
		  // store result
		  switch(storeType){
		  default:
		  case 1:{
		    std::string objN;
		    if(data->getScanPar(0)=="FEI4_GR"){
		      objN = ((ConfString&)(data->getScanConfig())["loops"]["feGlobRegNameLoop_0"]).value();
		      // FE index currently hard-coded in cfg., how to make it automatic?
		      Config &feconf = (*crate)->getPixModuleChipConf(iGrp, it->second, feIndex).subConfig("GlobalRegister_0/GlobalRegister");
		      if(feconf["GlobalRegister"].name()!="__TrashConfGroup__" && feconf["GlobalRegister"][objN].name()!="__TrashConfObj__"){
			WriteIntConf(((ConfInt&) feconf["GlobalRegister"][objN]), (int)result);
		      } else{
			toLog("STControlEngine::launchPrlTools : can't find FE global register "+objN);
			delete data;
			return true;
		      }
		    }else if(data->getScanPar(0)=="IREF_PAD"){
		      Config &cfgUSB = (*crate)->getPixCtrlConf(iGrp);
		      if(cfgUSB["general"].name()!="__TrashConfGroup__" && cfgUSB["general"]["IrefPads"].name()!="__TrashConfObj__"){
			WriteIntConf((ConfInt&) cfgUSB["general"]["IrefPads"], (int)result);
			(*crate)->reloadCtrlCfg(iGrp);
		      } else{
			toLog("STControlEngine::launchPrlTools : can't find USBPixController cfg. item IrefPads");
			delete data;
			return true;
		      }
		    }else{
		      // other scan var's not yet implemented
		      toLog("STControlEngine::launchPrlTools : saving of scan variabled "+data->getScanPar(0)+" not yet implemented, sorry");
		      delete data;
		      return true;
		    }
		    // do not break, saving to file should always be executed
		  }
		  case 2:{
		    // store in file
		    std::string storeName = scanName+"_ana";
		    PixConfDBInterface *myDB = 0;
		    PixLib::DBInquire *anaRec= 0;
		    try{
		      myDB = new RootDB(fileName.c_str(),"UPDATE");
		    }catch(SctPixelRod::BaseException& exc){
		      std::stringstream msg;
		      msg << "STControlEngine::launchPrlTools : exception while opening DB file "
			  << fileName << ": " << exc;
		      toLog(msg.str().c_str());
		      delete data;
		      return true;
		    }catch(...){
		      toLog(("STControlEngine::launchPrlTools : unknown exception while opening DB file "+fileName ).c_str());
		      delete data;
		      return true;
		    }
		    std::string rname = "Graph_ana/DCS_readings";
		    PixLib::DBInquire *root = myDB->readRootRecord(1);
		    if(root->findRecord(rname)==root->recordEnd()){
		      try{
			anaRec = myDB->makeInquire("DCS_readings", "Graph_ana");
			root->pushRecord(anaRec);
			myDB->DBProcess(root,COMMITREPLACE);
			myDB->DBProcess(anaRec,COMMIT);
			delete myDB;
			myDB = new RootDB(fileName.c_str(),"UPDATE");
			root = myDB->readRootRecord(1);
			anaRec = *(root->findRecord(rname));
		      }catch(...){
			toLog("STControlEngine::launchPrlTools : exception while creating a record for graph analysis result in DB file");
			delete myDB;
			delete data;
			return true;
		      }
		    } else
		      anaRec = *(root->findRecord(rname));
		    if(anaRec->findField(storeName)==anaRec->fieldEnd()){
		      try{
			DBField * newField = myDB->makeField(storeName);
			myDB->DBProcess(newField,COMMIT, result);
			anaRec->pushField(newField);
			delete newField;
			myDB->DBProcess(anaRec,COMMITREPLACE);
			delete myDB;
		      }catch(...){
			toLog("STControlEngine::launchPrlTools : exception while creating a field for graph analysis result in DB file");
			delete myDB;
			delete data;
			return true;
		      }
		    } else{
		      toLog(("STControlEngine::launchPrlTools : field to which should be written already exists: "+storeName ).c_str());
		      delete myDB;
		      return true;
		    }
		    break;}
		  case 3:{ 
		    // store in HV mod. mask
		    (*crate)->setModHVmask(iGrp, it->second, bresult);
		    // load entire mask to controller
		    int iref = (*crate)->getModHVmask(iGrp);
		    Config &pccfg = (*crate)->getPixCtrlConf(iGrp);
		    if(pccfg["general"].name()!="__TrashConfGroup__" && pccfg["general"]["IrefPads"].name()!="__TrashConfObj__")
		      ((ConfInt&)pccfg["general"]["IrefPads"]).setValue(iref);
		    // load modified config to board
		    (*crate)->reloadCtrlCfg(iGrp);
		    break;}
		  }
		}
	      }catch(...){
		data = 0;
	      }
	      delete data;
	    }
	  }
	}
      }
    } else
      return true; // arguments not available somehow, return an error
    break;}
  case PrimListItem::GEN_PIXMASK: {
    if(argCfg.name()!="__TrashConfig__" && argCfg["arguments"].name()!="__TrashConfGroup__" &&
       argCfg["arguments"]["scanName"].name()!="__TrashConfObj__" && argCfg["arguments"]["histoType"].name()!="__TrashConfObj__" && 
       argCfg["arguments"]["minCut"].name()!="__TrashConfObj__" && argCfg["arguments"]["maxCut"].name()!="__TrashConfObj__" &&
       argCfg["arguments"]["mask"].name()!="__TrashConfObj__" && argCfg["arguments"]["maskComb"].name()!="__TrashConfObj__"){
      std::string scanName = ((ConfString&)argCfg["arguments"]["scanName"]).value();
      int histoType = ((ConfList&)argCfg["arguments"]["histoType"]).getValue();
      std::string maskType = ((ConfList&)argCfg["arguments"]["mask"]).sValue();
      std::string maskComb = ((ConfList&)argCfg["arguments"]["maskComb"]).sValue();
      float minCut = ((ConfFloat&)argCfg["arguments"]["minCut"]).value();
      float maxCut = ((ConfFloat&)argCfg["arguments"]["maxCut"]).value();
      // loop over modules for analysis
      for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ){
	for(int iGrp=0; iGrp<(*crate)->nGroups(); iGrp++){
	  std::vector< std::pair<std::string, int> > list;
	  (*crate)->getModules(iGrp, list);
	  for(std::vector< std::pair<std::string, int> >::iterator it=list.begin(); it!=list.end(); it++){
	    if((*crate)->getModuleActive(iGrp, it->second)){ // ignore inactive modules
	      PixDBData *data = 0;
	      try{
		std::string mname = it->first;
		PixLib::getDecNameCore(mname);
		data = new PixDBData("maskgen", (fileName+":/"+scanName+"/"+(*crate)->getGrpName(iGrp)).c_str(),
				     mname.c_str());
		if(data->validPath()){
		  PixLib::Histo *h = data->GenMask((PixLib::PixScan::HistogramType)histoType, -1, -1, minCut, maxCut);
		  if(h!=0){
		    // upload histo to mask
		    for (int chip=0; chip<50; chip++){
		      Config &feconf = (*crate)->getPixModuleChipConf(iGrp, it->second, chip);
		      if(feconf.name()=="dummychip") break;
		      Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
		      if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
			 pixreg["PixelRegister"][maskType].name()!="__TrashConfObj__" &&
			 pixreg["PixelRegister"][maskType].type()==ConfObj::MATRIX &&
			 ((ConfMatrix&)pixreg["PixelRegister"][maskType]).subtype()==ConfMatrix::U1){
			ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][maskType];
			ConfMask<bool> &myMask = mapobj.valueU1();
			int nCol = myMask.get().size();
			int nRow = myMask.get().front().size();	
			for(int col=0;col<nCol;col++){
			  for(int row=0;row<nRow;row++){
			    int binx, biny;
			    data->PixCCRtoBins(chip,col,row,binx,biny);
			    // function was designed for root, so need offset by 1!
			    binx--;
			    biny--;
			    bool new_mask_val = (bool)((*h)(binx,biny));
			    bool old_mask_val = myMask[col][row];
			    // maskComb=="overwrite" - nothing else to do
			    if(maskComb=="AND with existing mask"){
			      new_mask_val &= old_mask_val;
			    } else if(maskComb=="OR with existing mask"){
			      new_mask_val |= old_mask_val;
			    }
			    myMask.set(col, row, new_mask_val);
			  }
			}
		      }
		    }
		    delete h;
		  }
		}
		delete data;
	      }catch(...){}
	    }
	  }
	}
      }
    } else
      return true; // arguments not available somehow, return an error
    break;}
  case PrimListItem::GEN_WAIT:{
    int wt = ((ConfInt&)argCfg["arguments"]["waitTime"]).getValue();
    for(int iw = 0; iw<wt; iw++){
      // loop to wait 60 sec. but still updating GUI inbetween
      for(int iws=0;iws<600;iws++){
	updateGUI();
	PixLib::sleep(100);
      }
    }
    break;}
  case PrimListItem::GEN_WAIT_MSEC:{
    int wt = ((ConfInt&)argCfg["arguments"]["waitTime"]).getValue();
    for(int iw = 0; iw<(wt/100); iw++){
      // loop to wait 1 sec. but still updating GUI inbetween
      updateGUI();
      PixLib::sleep(100);
    }
    updateGUI();
    PixLib::sleep(wt%100);
    break;}
  case PrimListItem::SIG_WAIT:{
    int wt = ((ConfInt&)argCfg["arguments"]["channel"]).getValue();
    int lc = ((ConfInt&)argCfg["arguments"]["combination"]).getValue();
    bool waitForHigh = ((ConfBool&)argCfg["arguments"]["waitOnOne"]).value();
    bool doWait=true;
    while(doWait){
      if(lc==0) doWait = false; // using OR, so true would remain
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	// loop over module groups
	for(int iGrp=0; iGrp < (*crIT)->nGroups(); iGrp++){
	  bool retval = (*crIT)->getCtrlInputStatus(iGrp, wt);
	  if(!waitForHigh) retval = !retval;
	  if(lc==0) // OR
	    doWait |= retval;
	  else if(lc==1) // AND
	    doWait &= retval;
	  else // not implemented
	    return true;
	}
      }
      updateGUI();
    }
    break;}
  case PrimListItem::SAVE_CFG:{
    std::string fnExt = ((ConfString&)argCfg["arguments"]["extString"]).value();
    bool activeOnly = ((ConfBool&)argCfg["arguments"]["activeOnly"]).value();
    std::string cfgFname = fileName;
    if(m_prlModTestType==""){
      int pos = cfgFname.find(".root"); 
      if(pos!=(int)std::string::npos){
	cfgFname.erase(pos,cfgFname.length()-pos);
	cfgFname += fnExt+".cfg.root";
      }
    } else{
      // temporary, need to re-write once cfg. can be saved module by module
      std::string mname = "123456";
      std::vector <STRodCrate *> rcs = getSTRodCrates();
      if(rcs.size()>0){
	for(int igrp = 0; igrp < rcs[0]->nGroups(); igrp++){
	  std::vector< std::pair<std::string, int> > modList;
	  rcs[0]->getModules(igrp, modList);
	  for(unsigned int im=0; im<modList.size();im++){
	    if(rcs[0]->getModuleActive(igrp, modList[im].second)){
	      mname = modList[im].first;
	      break;
	    }
	  }
	  getDecNameCore(mname);
	  if(mname!="123456") break; // exit group loop, found active module in this grp.
	}
      }
      getDecNameCore(mname);
      if(fnExt=="")
	cfgFname = m_modPath + "/M" + mname + "/configs/" + mname + "_" + m_prlModTestType + ".cfg.root";
      else
	cfgFname = m_modPath + "/M" + mname + "/configs/" + mname + "_" + fnExt + "_" + m_prlModTestType + ".cfg.root";
    }
    if(fileName.length()>0){
      std::string orgName = getPixConfDBFname();
      remove(cfgFname.c_str());
      saveDB(cfgFname.c_str(), activeOnly);
      m_PixConfDBFname = orgName;
      emit cfgLabelChanged();
    }
    break;}
  case PrimListItem::LOAD_MOD_CFG:{
    bool loadSN = ((ConfBool&)argCfg["arguments"]["SetChipSN"]).value();    
    int ChipSN = ((ConfInt&)argCfg["arguments"]["Chip_SN"]).getValue();
    // loop over crates
    bool gotErr = false;
    for(std::vector<STRodCrate*>::iterator crIT = getSTRodCrates().begin(); crIT != getSTRodCrates().end(); crIT++){
      gotErr |= (*crIT)->reloadModCfg(getPixConfDBFname());
      if(loadSN) (*crIT)->setGR("GlobalRegister_Chip_SN", ChipSN);
    }
    if(gotErr){
      toErrLog("STControlEngine::launchPrlTools : error loading config for at least one module");
      return true;
    }
    break;}
  case PrimListItem::SET_FE_MODE:{
    int mode = ((ConfInt&)argCfg["arguments"]["mode"]).getValue();    
    for(std::vector<STRodCrate*>::iterator crIT = getSTRodCrates().begin(); crIT != getSTRodCrates().end(); crIT++)
      (*crIT)->setFeMode(mode);
    break;}
  case PrimListItem::RD_EPROM:{	// M.B.: implemented new PrlTool for read EPROM. Implementation correct?
	  for(std::vector<STRodCrate*>::iterator crIT = getSTRodCrates().begin(); crIT != getSTRodCrates().end(); crIT++){
      (*crIT)->readEPROM();
	  }
	break;}
  case PrimListItem::BURN_EPROM:{	// M.B.: implemented new PrlTool for burn EPROM. Implementation correct?
	  for(std::vector<STRodCrate*>::iterator crIT = getSTRodCrates().begin(); crIT != getSTRodCrates().end(); crIT++){
      (*crIT)->burnEPROM();
	  }
	break;}
  case PrimListItem::READ_GADC:{	// M.B.: implemented new PrlTool for read GADC. Implementation correct? Error Handling for fail (esp. in FE-I4A case) missing...
    int type = ((ConfInt&)argCfg["arguments"]["readType"]).getValue();
    readGADC(type);
    std::stringstream a;
    a << m_prlLabelIndex;
    //std::vector<int> srvRec;
	std::vector<int> result;
    std::vector <STRodCrate*> myCrates = getSTRodCrates();
    // save result to file
    PixConfDBInterface *myData=0;
    DBInquire *root=0, *testInq=0, *grpInq=0, *modInq=0;
    if(m_prlOutname!=""){
      myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
      root = myData->readRootRecord(1); 
      // check if this scan already has an inquire, and create one if not
      testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel()+ (m_prlIndMode?(" "+a.str()):"") + (m_prlIndMode?(" "+a.str()):"") + "/GenericADCvalue"));
      if(testInq==0){
	std::string name, decName, myDecName;
	name="GenericADCvalue";
	decName = root->getDecName() + m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"");
	testInq = myData->makeInquire(name, decName);
	root->pushRecord(testInq);
	// process
	myData->DBProcess(root,COMMITREPLACE);
	myData->DBProcess(testInq,COMMIT);
	delete myData;
	myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	root = myData->readRootRecord(1);
	testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/GenericADCvalue"));
      }
      // loop over crates
      std::vector <STRodCrate*> myCrates = getSTRodCrates();
      for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
	// loop over groups
	for(int iGrp=0;iGrp<(*crIT)->nGroups();iGrp++){
	  (*crIT)->getGADCRes(iGrp, result);
//  	  cout << "GADC for group " << iGrp << endl;
//  	  for(unsigned int k=0;k<result.size();k++)
//  	    cout << k << ": "<<(result[k]) << endl;
	  // create inquire for this group
	  std::string name, decName, myDecName;
	  name="PixModuleGroup";
	  decName = testInq->getDecName() + (*crIT)->getGrpName(iGrp);
	  grpInq = myData->makeInquire(name, decName);
	  testInq->pushRecord(grpInq);
	  // process
	  myData->DBProcess(testInq,COMMITREPLACE);
	  myData->DBProcess(grpInq,COMMIT);
	  delete myData;
	  myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	  root = myData->readRootRecord(1);
	  testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/GenericADCvalue"));
	  grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	  // temporary: have only one module per group (needs to be fixed?)
	  // loop over modules
// 	  std::vector< std::pair<std::string, int> > modList;
// 	  (*crIT)->getModules(iGrp, modList);
// 	  for(std::vector< std::pair<std::string, int> >::iterator modIT=modList.begin(); modIT!=modList.end();modIT++){
	    // create inquire for this module
	    std::stringstream b;
	    b << 0;//modIT->second;
	    name="Mod"+b.str();
	    decName = testInq->getDecName() + name;
	    modInq = myData->makeInquire(name, decName);
	    grpInq->pushRecord(modInq);
	    // process
	    myData->DBProcess(grpInq,COMMITREPLACE);
	    myData->DBProcess(modInq,COMMIT);
	    delete myData;
	    myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	    root = myData->readRootRecord(1);
	    testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/GenericADCvalue"));
	    grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	    modInq = *(grpInq->findRecord(name+"/"+name));
	    // dump service record vector DB file
	    RootDBField *infoField = new RootDBField("Result",myData);
	    myData->DBProcess(infoField,COMMIT,result);
	    modInq->pushField(infoField);
	    delete infoField;
	    myData->DBProcess(modInq,COMMITREPLACE);    
	    delete myData;
	    myData = new RootDB(m_prlOutname.c_str(),"UPDATE");
	    root = myData->readRootRecord(1);
	    testInq = *(root->findRecord(m_prlList[m_prlIndex]->getLabel() + (m_prlIndMode?(" "+a.str()):"") + "/GenericADCvalue"));
	    //	    grpInq = *(testInq->findRecord((*crIT)->getGrpName(iGrp)+"/PixModuleGroup"));
	    //	  }
	}
      }
      delete myData;
    }
    break;}
  case PrimListItem::SEND_GLOBPLS:{
    int length = ((ConfInt&)argCfg["arguments"]["length"]).getValue();
    sendGlobalPulse(length);
    break;}
  case PrimListItem::SELECT_BI_MOD:{
    int chan = ((ConfInt&)argCfg["arguments"]["channel"]).getValue();
    bool setIref = ((ConfBool&)argCfg["arguments"]["setIref"]).value();    
    bool skipIfNotIref = ((ConfBool&)argCfg["arguments"]["skipIfNotIref"]).value();
    bool wrapGrpId = ((ConfBool&)argCfg["arguments"]["WrapGrpId"]).value();
    if(chan<0) chan = m_prlLabelIndex-1;
    setBurnInChan(chan, setIref, skipIfNotIref, wrapGrpId);
    emit configChanged();
    break;}
  default:
    // not implemented, return an error
    return true;
  }

  // done, process next prl item
  processPrimList();
  return false;
}
void STControlEngine::runLinkCheck(int crateID, int grpID, int modID)
{
  if(CtrlStatusSummary()<=0){
    sendRTStatus("can't find any initialised RODs, not testing");
    return;
  }

  // in some cases, it seems to be necessary to reset FEs and MCC before starting...
  sendRTStatus("resetting modules");
  m_app->processEvents();               // make sure GUI really updates
  resetModules(true);
  resetModules(false);

  // set ToolPanel inactive
  emit beganScanning();
  sendRTStatus("running link check");
  m_app->processEvents();               // make sure GUI really updates
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if(crateID<0){
    // run link test on all crates
    std::vector <STRodCrate*> myCrates = getSTRodCrates();
    for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
      (*crIT)->runLinkTest(-1,-1);
    processExecute();
  }else if(crateID<(int)m_sTRodCrates.size()){ 
    // just test one module, no need to run multi-threaded
    m_sTRodCrates[crateID]->runLinkTest(grpID, modID);
  }
  QApplication::restoreOverrideCursor();
  emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
  emit finishedScanning();
  m_app->processEvents();               // make sure GUI really updates

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_app->processEvents();               // make sure GUI really updates

  sendRTStatus("re-initialising RODs");
  m_app->processEvents();               // make sure GUI really updates
  // temporary "fix": ROD doesn't scan properly after this excercise -> reset
  initRods();

  sendRTStatus("Done.");
  QApplication::restoreOverrideCursor();
  return;
}

void STControlEngine::runChipTest(const char *label, int crateID, int grpID, int modID){

  // check if we have a ChipTest object with the requested name
  ChipTest *ct = m_chipTests[std::string(label)];
  if(ct==0){
    sendRTStatus(("STControlEngine: can't find ChipTest object with name "+std::string(label)).c_str());
    return;
  }
  
  runChipTest(ct, crateID, grpID, modID);
}
void STControlEngine::runChipTest(ChipTest *ct_in, int crateID, int grpID, int modID){

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // set ToolPanel inactive
  emit beganScanning();

  if(crateID<0){
    // run scan on all crates
    std::vector <STRodCrate*> myCrates = getSTRodCrates();
    for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++)
      (*crIT)->runChipTest(ct_in, m_resetAfterTest, -1,-1);
    processExecute();
  }else if(crateID<(int)m_sTRodCrates.size()){ 
    // just test one module, no need to run multi-threaded
    m_sTRodCrates[crateID]->runChipTest(ct_in, m_resetAfterTest, grpID, modID);
  }
  sendRTStatus("Done.");

  // re-activate tool panel
  emit finishedScanning();
  m_app->processEvents();

  QApplication::restoreOverrideCursor();  

  return;
}
void STControlEngine::stopPixScan()
{
  if(!m_lastPixScanOpts.scanConfig->getSourceScanFlag()){
    // aborted data don't need saving - unless in source mode, then "abort" actually means "stop"
    m_lastPixScanOpts.writeToFile = false; 
    m_lastPixScanOpts.loadToAna   = false;
  }
  // stop scan over all crates and rods
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) 
      (*crate)->abortScan();
}
float STControlEngine::readCurrent(float cutval){ // dummy routine
  float retval = 1.1*cutval;
  retval += (float)rand()/(float)RAND_MAX*.2*cutval;
  for(int i=0;i<5;i++){
    PixLib::sleep(10);
    m_app->processEvents();
  }
  return retval;
}
void STControlEngine::selectFe(int iFE, int crateID, int grpID, int modID)
{
  int iStart = 0;
  int iStop = m_sTRodCrates.size();
  if(crateID>=0 && crateID<(int)m_sTRodCrates.size()){
    iStart = crateID;
    iStop = crateID+1;
  }
  for(int ci=iStart; ci<iStop;ci++)
    m_sTRodCrates[ci]->selectFe(iFE, grpID, modID);
}
void STControlEngine::addDcsToCurrDB(const char *objName, const char *classNameHead, int devType, int nChan, 
				     const char *ctrlName, const char *crateName){
  std::string fname = getPixConfDBFname();
  if(fname==""){
    toErrLog("STControlEngine::addGrouptoCurrDB : Can't find existing DB file in memory, so can't edit.");
    return;
  }

  PixConfDBInterface *myDB = reopenDBforEdit();
  if(myDB==0){
    toErrLog("STControlEngine::addGrouptoCurrDB : error reopening existing DB file, so can't edit.");
    reloadDBafterEdit(false);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);    
  DBInquire *startInq=0;
  try{
    if(m_singleCrateMode)
      startInq = PixLib::findAppInq(myDB);
    else
      startInq = PixLib::findAppInq(myDB,crateName);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addDcstoCurrDB : exception while re-reading new DB file " 
        << fname << ": " << exc;
    toErrLog(msg.str().c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog(("STControlEngine::addDcstoCurrDB : unknown exception while re-reading new DB file" 
              + fname).c_str());
    delete myDB;
    reloadDBafterEdit(false);
    return;
  }
  QApplication::restoreOverrideCursor();

  addDcsToDB(startInq, objName, classNameHead, devType, nChan, ctrlName);

  // close and remove old file and open new one
  delete myDB;
  reloadDBafterEdit(true);
  return;
}
void STControlEngine::addDcsToDB(DBInquire *baseInq, const char *objName, const char *classNameHead, int devType, 
				 int nChan, const char *ctrlName){
  QApplication::setOverrideCursor(Qt::WaitCursor);    
  try{
    PixConfDBInterface *myDB = baseInq->getDB();
    // count existing PixDcs objects and remember highest index used
    int nDcs=0;
    int lastIndex = -1;
    for(recordIterator dcsit = baseInq->recordBegin();dcsit!=baseInq->recordEnd();dcsit++){
      if((*dcsit)->getName().find("PixDcs")!=std::string::npos){
	nDcs++;
	fieldIterator fit = (*dcsit)->findField("general_Index");
	if(fit!=(*dcsit)->fieldEnd()){
	  int devId;
	  if(fit!=(*dcsit)->fieldEnd()){
	    myDB->DBProcess(*fit,READ,devId);
	    if(devId>lastIndex) lastIndex = devId;
	  }
	}
      }
    }
    std::stringstream a;
    a << nDcs;
    // create main inquire
    std::string decName, className=classNameHead;
    decName = baseInq->getDecName();
    className += "PixDcs";
    decName += className + "_" + a.str();
    DBInquire *dcsInq = myDB->makeInquire("PixDcs",decName);
    baseInq->pushRecord(dcsInq);
    myDB->DBProcess(baseInq,COMMITREPLACE);
    myDB->DBProcess(dcsInq,COMMIT);
    // define device type and name
    DBField *newField = myDB->makeField("general_DeviceName");
    std::string name=objName;
    myDB->DBProcess(newField,COMMIT,name);
    dcsInq->pushField(newField);
    delete newField;
    myDB->DBProcess(dcsInq,COMMITREPLACE);
    newField = myDB->makeField("general_DeviceType");
    name = "SUPPLY";
    DummyPixDcs *pd = new DummyPixDcs();
    Config &conf = pd->config();
    std::map<std::string, int> typeMap = ((ConfList&)conf["general"]["DeviceType"]).symbols();
    for(std::map<std::string, int>::iterator tit=typeMap.begin(); tit!=typeMap.end(); tit++)
      if(tit->second==devType) name = tit->first;
    delete pd;
    myDB->DBProcess(newField,COMMIT,name);
    dcsInq->pushField(newField);
    delete newField;
    myDB->DBProcess(dcsInq,COMMITREPLACE);
    // define device type by class name
    newField = myDB->makeField("ActualClassName");
    myDB->DBProcess(newField,COMMIT,className);
    dcsInq->pushField(newField);
    delete newField;
    myDB->DBProcess(dcsInq,COMMITREPLACE);
    // set index to last one used +1 to allow for correct ordering
    int newIndex = lastIndex + 1;
    newField = myDB->makeField("general_Index");
    myDB->DBProcess(newField,COMMIT,newIndex);
    dcsInq->pushField(newField);
    delete newField;
    myDB->DBProcess(dcsInq,COMMITREPLACE);
    // add name of USBPixController if this is a USBPixDcs object
    if(className.substr(0,3)=="USB"){
      newField = myDB->makeField("USBPixController");
      name =ctrlName;
      myDB->DBProcess(newField,COMMIT,name);
      dcsInq->pushField(newField);
      delete newField;
      myDB->DBProcess(dcsInq,COMMITREPLACE);
    }
    // add channel inquire(s)
    for(int iCh=0;iCh<nChan;iCh++){
      std::stringstream b;
      b << iCh;
      decName = dcsInq->getDecName() + className+"Chan_"+b.str();
      DBInquire *chanInq = myDB->makeInquire("PixDcsChan",decName);

      dcsInq->pushRecord(chanInq);
      myDB->DBProcess(dcsInq,COMMITREPLACE);
      myDB->DBProcess(chanInq,COMMIT);
    }
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STControlEngine::addDcsToDB : exception while creating DCS inquire: " << exc;
    toErrLog(msg.str().c_str());
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    toErrLog("STControlEngine::addDcsToDB : unknown exception while creating DCS inquire." );
    return;
  }
  QApplication::restoreOverrideCursor();
  return;
}
bool STControlEngine::checkScanLabel(std::vector<QString> existingLabels, QString newLabel, QString &suggLabel, QString compExpr){
  bool is_dupl = false;
  QString test_txt = newLabel;
  suggLabel = newLabel;
  // if user appended a number to scan label, strip it off
  int lind=1, tind = test_txt.lastIndexOf(compExpr);
  // the following avoids stripping off more than just an index at the end
  if(tind>test_txt.length()-5) test_txt = test_txt.left(tind);
  for(std::vector<QString>::iterator it=existingLabels.begin(); it!=existingLabels.end();it++){
    if(newLabel==(*it)) is_dupl = true;
    if((*it).left(test_txt.length())==test_txt){ // found a label similar to ours - check if it has an index
      bool isOK;
      tind = (*it).right((*it).length()-test_txt.length()-1).toInt(&isOK);
      if(isOK && tind>=lind) lind = tind+1;
    }
  }
  test_txt += compExpr + QString::number(lind);
  if(is_dupl) suggLabel = test_txt;
  return is_dupl;
}
int STControlEngine::currFEFlavour(){
  int retval = 0;
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) {
    for(int i=0; i<(*crate)->nGroups(); i++){
      Config &cfg = (*crate)->getPixModuleConf(i, 0);
      if(cfg.name()!="__TrashConfig__" && cfg["general"].name()!="__TrashConfGroup__" &&
	 cfg["general"]["FE_Flavour"].name()!="__TrashConfObj__"){
	std::string feflv = ((ConfList&)cfg["general"]["FE_Flavour"]).sValue();
	if(feflv=="FE_I4A"){
	  if(retval==0) retval = 1;
	  else if(retval!=1) retval = 10;
	}
	else if(feflv=="FE_I4B"){
	  if(retval==0) retval = 2;
	  else if(retval!=2) retval = 10;
	}
      }
    }
  }
  return retval;
}
std::map<int, std::string> STControlEngine::GetFeFlavours(){
  std::map<int, std::string> retval, t_flv;
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) {
    t_flv = (*crate)->GetFeFlavours(); // FE flavours of boards in this crate
    retval.insert(t_flv.begin(), t_flv.end()); 
  }
  return retval;
}

void STControlEngine::setBurnInChan(int chan, bool setIref, bool skipIfNotIref, bool wrapGrpId){
  // loop over crates
  std::vector <STRodCrate*> myCrates = getSTRodCrates();
  for(std::vector<STRodCrate*>::iterator crIT = myCrates.begin(); crIT != myCrates.end(); crIT++){
    int grpWrap = 0, grpWrapId = 0;
    // loop over groups
    for(int iGrp=0;iGrp<(*crIT)->nGroups();iGrp++){
      int gChan = chan;
      std::vector< std::pair<std::string, int> > modlist;
      (*crIT)->getModules(iGrp, modlist);
      if(wrapGrpId){
	if(iGrp!=grpWrapId)
	  gChan = -1;
	else{
	  gChan -= grpWrap;
	  if(gChan>=(int)modlist.size()){
	    gChan = -1;
	    grpWrap += modlist.size();
	    grpWrapId++;
	  }
	}
      } 
      if(gChan>=0){
	Config &pccfg = (*crIT)->getPixCtrlConf(iGrp);
	bool haveIrefCfg = pccfg["general"].name()!="__TrashConfGroup__" && pccfg["general"]["IrefPads"].name()!="__TrashConfObj__";
	// if Iref is used as HV switch, check if module is enabled and skip if not
	if(!setIref && skipIfNotIref && haveIrefCfg){
	  int iref = ((ConfInt&)pccfg["general"]["IrefPads"]).value();
	  for(int ic=0;ic<chan;ic++){
	    if((iref>>ic)&0x1) gChan++;
	  }
	}
	// set channel on burn-in card - chan+1, since it starts counting at 1
	if(gChan>=0 && gChan<4){
	  ((ConfInt&)(*crIT)->getPixCtrlConf(iGrp)["general"]["FEToRead"]).setValue(gChan+1);
	  int iref = 1<<gChan;
	  if(setIref && haveIrefCfg)((ConfInt&)(*crIT)->getPixCtrlConf(iGrp)["general"]["IrefPads"]).setValue(iref);
	} else {
	  ((ConfInt&)(*crIT)->getPixCtrlConf(iGrp)["general"]["FEToRead"]).setValue(4);
	  if(setIref && haveIrefCfg) ((ConfInt&)(*crIT)->getPixCtrlConf(iGrp)["general"]["IrefPads"]).setValue(0);
	}
	// load modified config to board
	(*crIT)->reloadCtrlCfg(iGrp);
      }
      // disable all unused modules
      for(unsigned int iMod=0;iMod<modlist.size();iMod++){
	int modId = modlist[iMod].second;
	*((int *)((ConfList&)(*crIT)->getPixModuleConf(iGrp, modId)["general"]["Active"]).m_value)=
	  (int)((ConfList&)(*crIT)->getPixModuleConf(iGrp, modId)["general"]["Active"]).symbols()[((modId==gChan)?"TRUE":"FALSE")];
      }
    }
  }
  return;
}
void STControlEngine::copyDcsDb(const char *fname_currDB, const char *fname_cpDB, const char *ctrlName){
  PixConfDBInterface *myDB=0, *inDB;
  try{
    myDB = DBEdtEngine::openFile(fname_currDB, true);
  }catch(SctPixelRod::BaseException& exc){
    std::stringstream msg;
    msg << "STControlEngine::copyDcsDb : exception while opening DB file " 
	<< std::string(fname_currDB) << ": " << exc;
    toErrLog(msg.str().c_str());
    myDB=0;
  }catch(...){
    toErrLog(("STControlEngine::copyDcsDb : unknown exception while opening DB file " 
	      + std::string(fname_currDB)));
    myDB=0;
  }
  try{
    inDB = DBEdtEngine::openFile(fname_cpDB, false);
  }catch(SctPixelRod::BaseException& exc){
    std::stringstream msg;
    msg << "STControlEngine::copyDcsDb : exception while opening DB file " 
	<< std::string(fname_cpDB) << ": " << exc;
    toErrLog(msg.str().c_str());
    inDB=0;
  }catch(...){
    toErrLog(("STControlEngine::copyDcsDb : unknown exception while opening DB file " 
	      + std::string(fname_cpDB)));
    inDB=0;
  }

  if(myDB==0 || inDB==0) return;

  DBInquire *dcsRoot = inDB->readRootRecord(1);
  DBInquire *dcsAppInq  = *(dcsRoot->recordBegin());
  DBInquire *myRoot = myDB->readRootRecord(1);
  DBInquire *myAppInq  = *(myRoot->recordBegin());

  // get highest DCS index first to insert copied devices correctly
  std::map<std::string,int> existDcs;
  for(recordIterator dcsit=myAppInq->recordBegin();dcsit!=myAppInq->recordEnd();dcsit++){
    if((*dcsit)->getName()=="PixDcs"){
      fieldIterator fit = (*dcsit)->findField("general_Index");
      if(fit!=(*dcsit)->fieldEnd()){
	int devId;
	if(fit!=(*dcsit)->fieldEnd()){
	  myDB->DBProcess(*fit,READ,devId);
	  std::string dnc = (*dcsit)->getDecName();
	  getDecNameCore(dnc);
	  existDcs[dnc] = devId+1;
	}
      }
    }
  }
  // now copy devices from separate file
  for(recordIterator dcsit=dcsAppInq->recordBegin();dcsit!=dcsAppInq->recordEnd();dcsit++){
    if((*dcsit)->getName()=="PixDcs"){
      fieldIterator fit = (*dcsit)->findField("ActualClassName");
      if(fit!=(*dcsit)->fieldEnd()){
	std::string clname;
	inDB->DBProcess(*fit,READ, clname);
	bool copyRec = false;
	if(clname!="USBPixDcs" && clname!="USBBIPixDcs"){
	  copyRec = true;
	} else{ // copy only FE-ADC entries, rest should be covered by creation from scratch
	  fit = (*dcsit)->findField("general_DeviceType");
	  std::string devType = "NONE";
	  if(fit!=(*dcsit)->fieldEnd())  myDB->DBProcess(*fit,READ,devType);
	  if(devType=="ADC-METER"){
	    for(recordIterator dcsiit=(*dcsit)->recordBegin();dcsiit!=(*dcsit)->recordEnd();dcsiit++){
	      if((*dcsiit)->getName()=="PixDcsChan"){
		fit = (*dcsiit)->findField("general_ChannelDescr");
		std::string chType = "NONE";
		if(fit!=(*dcsiit)->fieldEnd())  myDB->DBProcess(*fit,READ,chType);
		if(chType=="FE ADC"){
		  copyRec = true;
		  break;
		}
	      }
	    }
	  }
	}
	if(copyRec){
	  std::string newName = (*dcsit)->getDecName();
	  getDecNameCore(newName);
	  if(existDcs[newName]>0) newName+="_new";
	  insertDBInquire(*dcsit, myAppInq, true, newName.c_str());
	  std::vector<DBInquire*> div = myDB->DBFindRecordByName(PixLib::BYDECNAME,myAppInq->getDecName()+newName+"/PixDcs");
	  if(div.size()==1){
	    // fix index if needed, otherwise preserve exist. index
	    fit = div[0]->findField("general_Index");
	    if(fit!=div[0]->fieldEnd()){
	      int currId;
	      myDB->DBProcess(*fit,READ,currId);
	      std::string dnc = (*dcsit)->getDecName();
	      getDecNameCore(dnc);
	      bool needNewId=false;
	      int maxId = -1;
	      // check existing indices - NB: existDcs is shifted by 1
	      for(std::map<std::string,int>::iterator mit = existDcs.begin(); mit!=existDcs.end();mit++){
		if((mit->second-1)>maxId) maxId=mit->second-1;
		if((mit->second-1) == currId) needNewId=true;
	      }
	      if(needNewId){
		currId = maxId+1;
		myDB->DBProcess(*fit,COMMIT,currId);
		myDB->DBProcess(div[0],COMMITREPLACE);
	      }
	      existDcs[dnc] = currId+1;
	    }
	    // if USB(BI)PixDcs, fix controller ref.
	    if(clname=="USBPixDcs" || clname=="USBBIPixDcs"){
	      fit = (*dcsit)->findField("USBPixController");
	      if(fit!=(*dcsit)->fieldEnd()){
		std::string newName = ctrlName;
		myDB->DBProcess(*fit,COMMIT,newName);
		myDB->DBProcess(div[0],COMMITREPLACE);
	      }
	    }
	  }
	}
      }
    }
  }
  delete inDB;
  delete myDB;
}
void STControlEngine::forceRead(){
  for( std::vector<STRodCrate *>::iterator crate = m_sTRodCrates.begin(); crate != m_sTRodCrates.end(); crate++ ) 
    (*crate)->forceRead();
  return;
}
int STControlEngine::getConsecutiveLvl1TrigA(int superGroup){
  return m_lastPixScanOpts.scanConfig->getConsecutiveLvl1TrigA(superGroup);
}
int STControlEngine::getConsecutiveLvl1TrigB(int superGroup){
  return m_lastPixScanOpts.scanConfig->getConsecutiveLvl1TrigB(superGroup);
}


void STControlEngine::createMultiBoardConfig(QString filename, extScanOptions ScanParameters)
{
  //  std::cout << "STControlEngine::createMultiBoardConfig: filename = " << std::string(filename.toLatin1().data()) << std::endl;

  // create config
  std::vector<grpData> myGrpDataV;
  std::string fullModDecName;
  std::string requestedModDecName;
  QStringList modNames;
  bool found;
  std::map< int, std::vector<int> > roChanInput;
  std::map< int, std::vector<int> > roChanFeid;
  for (int i=0; i<ScanParameters.boards.count(); i++)
    {
      // Search Module
      requestedModDecName = "";
      found=false;
      std::string flvValue = "FE_I4A";

      PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(ScanParameters.config_files[i].toLatin1().data(), false); 
      DBInquire *root = confDBInterface->readRootRecord(1);
      for(recordIterator appIter = root->recordBegin();appIter!=root->recordEnd();appIter++){
	if((int)(*appIter)->getName().find("application")!=(int)std::string::npos){
	  // loop over inquires in crate inquire and create a PixModuleGroup when an according entry is found
	  for(recordIterator pmgIter = (*appIter)->recordBegin();pmgIter!=(*appIter)->recordEnd();pmgIter++){
	    if((*pmgIter)->getName().find("PixModuleGroup")!=std::string::npos){
	      for(recordIterator pmIter = (*pmgIter)->recordBegin();pmIter!=(*pmgIter)->recordEnd();pmIter++){
		if((*pmIter)->getName().find("PixController")!=std::string::npos && ScanParameters.adapterCardFlavour==1){
		  std::vector<int> storeval_in(4,0);
		  std::vector<int> storeval_fe(4,0);
		  for(int iFe=0; iFe<4; iFe++){
		    std::stringstream sv;
		    sv << "general_readoutChannelsInput";
		    sv << iFe;
		    fieldIterator field = (*pmIter)->findField(sv.str());
		    int dbval = 0;
		    if (field!=(*pmIter)->fieldEnd()) {
		      confDBInterface->DBProcess((*field),PixLib::READ,dbval);
		      storeval_in.at(iFe)=dbval;
		    }
		    std::stringstream sv2;
		    sv2 << "general_readoutChannelReadsChip";
		    sv2 << iFe;
		    field = (*pmIter)->findField(sv2.str());
		    if (field!=(*pmIter)->fieldEnd()) {
		      confDBInterface->DBProcess((*field),PixLib::READ,dbval);
		      storeval_fe.at(iFe)=dbval;
		    }
		  }
		  roChanInput[ScanParameters.boards[i].toInt()]=storeval_in;
		  roChanFeid [ScanParameters.boards[i].toInt()]=storeval_fe;
		}
		if((*pmIter)->getName().find("PixModule")!=std::string::npos && !found){
		  fullModDecName = (*pmIter)->getDecName();
		  requestedModDecName = (*pmIter)->getDecName();
		  getDecNameCore(requestedModDecName);
		  // if specific module was requested, just accept this
		  // otherwise use first module found
		  if (requestedModDecName==ScanParameters.config_modules[i].toStdString() || ScanParameters.config_modules[i]==""){
		    found=true;
		    // read FE flavour to use it for USBPixDcs channel settings
		    fieldIterator f;
		    f = (*pmIter)->findField("general_FE_Flavour");
		    if(f!=(*pmIter)->fieldEnd()) confDBInterface->DBProcess(*f,READ,flvValue);
		    break;
		  }
		}
	      }
	    }
	  }
	}
      }
      delete confDBInterface; //closes file
      
      if (requestedModDecName=="") return;
      
      bool new_mod_name=false;
      int mod_counter=1;
      std::string modBaseName=requestedModDecName;
      while (!new_mod_name)
	{
	  new_mod_name = true;
	  for (int i = 0; i < modNames.size(); ++i)
	    {
	      if (modNames.at(i).toStdString() == requestedModDecName) 
		{
		  new_mod_name=false;
		  break;
		}
	    }
	  
	  if (!new_mod_name)
	    {
	      requestedModDecName=modBaseName + QString::number(mod_counter).toStdString();
	      mod_counter++;
	    }
	}
      
      modNames.push_back(QString::fromStdString(requestedModDecName));
      
      grpData myGrpData;
      myGrpData.myROD.slot = ScanParameters.boards[i].toInt();
      myGrpData.myROD.mode = (ScanParameters.adapterCardFlavour==1)?2:0;
      myGrpData.myBOC.mode = -1;
      myGrpData.myBOC.haveBoc = false;
      myGrpData.cfgType = 1;
      myGrpData.myROD.IPfile = ScanParameters.fpga_firmware.toLatin1().data();
      myGrpData.myROD.IDfile = ScanParameters.uc_firmware.toLatin1().data();
      modData myModD;
      myModD.fname = ScanParameters.config_files[i].toLatin1().data();
      myModD.modname= requestedModDecName;
      myModD.connName= fullModDecName;
      myGrpData.cfgType = 2; // read cfg. from file as is
      myModD.modID   = 0;
      myModD.grpID   = 0;
      myModD.active  = true;
      myModD.roType  = 0;
      myModD.inLink  = 0;
      for(int olID=0;olID<4;olID++)
	myModD.outLink[olID]  = 0;
      myModD.slot = 0;
      myModD.pp0 = -1;
      myModD.assyType = 1;
      myModD.assyID = 1;
      if(flvValue == "FE_I4B")
	myModD.assyID = 2;
      myModD.pos_on_assy = 0;
      myGrpData.myMods.push_back(myModD);
      myGrpDataV.push_back(myGrpData);
    }
  
  // Create Multiboard Config
  addCrateToDB("TestApp", myGrpDataV, filename.toLatin1().data(), tUSBSys);
  loadDB(filename.toLatin1().data());
  
  QApplication::processEvents();
  
  // unload config
  clear();
  setPixConfDBFname("");

  // alter settings as requested from scan config
  std::string channel;
  PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(filename.toLatin1().data(), true); 
  DBInquire *root = confDBInterface->readRootRecord(1);
  for(recordIterator appIter = root->recordBegin();appIter!=root->recordEnd();appIter++){
    if((int)(*appIter)->getName().find("application")!=(int)std::string::npos){
      // loop over inquires in crate inquire and create a PixModuleGroup when an according entry is found
      for(recordIterator pmgIter = (*appIter)->recordBegin();pmgIter!=(*appIter)->recordEnd();pmgIter++){
	if((*pmgIter)->getName().find("PixModuleGroup")!=std::string::npos)
	  {
	    // Add TriggerDelay if neccessary
	    fieldIterator field = (*pmgIter)->findField("general_TriggerDelay");
	    if (field==(*pmgIter)->fieldEnd()) {
	      int dbval=0;
	      PixLib::DBField *new_field = confDBInterface->makeField("general_TriggerDelay");
	      confDBInterface->DBProcess(new_field,PixLib::COMMIT,dbval);
	      (*pmgIter)->pushField(new_field);
	      confDBInterface->DBProcess((*pmgIter), PixLib::COMMITREPLACE);
	    }
	    
	    for(recordIterator pmIter = (*pmgIter)->recordBegin();pmIter!=(*pmgIter)->recordEnd();pmIter++){
	      if((*pmIter)->getName().find("PixController")!=std::string::npos){
		
		// get board id
		fieldIterator field = (*pmIter)->findField("general_BoardID");
		int boardid;
		int board_index=0;
		if (field!=(*pmIter)->fieldEnd())
		  {
		    confDBInterface->DBProcess(field, PixLib::READ, boardid);
		    
		    // board index for this id
		    while (board_index < ScanParameters.boards.count() && ScanParameters.boards[board_index]!=QString::number(boardid))
		      board_index++;
		    //std::cout << "boardid: " << boardid << " with index " << board_index << std::endl;
		  }
		
		// Set RJ45 to on
		field = (*pmIter)->findField("general_enableRJ45");
		bool dbval=true;
		if (field==(*pmIter)->fieldEnd()) {
		  PixLib::DBField *new_field = confDBInterface->makeField("general_enableRJ45");
		  confDBInterface->DBProcess(new_field,PixLib::COMMIT,dbval);
		  (*pmIter)->pushField(new_field);
		} else {
		  confDBInterface->DBProcess(field, COMMIT, dbval);
		}
		
		// Set trigger replication mode
		field = (*pmIter)->findField("general_TriggerReplication");
		std::string rep_mode;
		if (ScanParameters.trigger_replication_modes[board_index]=="master")
		  rep_mode="Master";
		else if (ScanParameters.trigger_replication_modes[board_index]=="slave")
		  rep_mode="Slave";
		else
		  rep_mode="Disabled";
		
		if (field==(*pmIter)->fieldEnd()) {
		  PixLib::DBField *new_field = confDBInterface->makeField("general_TriggerReplication");
		  confDBInterface->DBProcess(new_field,PixLib::COMMIT,rep_mode);
		  (*pmIter)->pushField(new_field);
		} else {
		  confDBInterface->DBProcess(field, COMMIT, rep_mode);
		}
		
		// Add general_enableCmdLvl1 if needed
		field = (*pmIter)->findField("general_enableCmdLvl1");
		if (field==(*pmIter)->fieldEnd()) {
		  // field not existing!
		  bool dbval=true;
		  //		  std::cout << "Add general_enableCmdLvl1" << std::endl;
		  PixLib::DBField *new_field = confDBInterface->makeField("general_enableCmdLvl1");
		  confDBInterface->DBProcess(new_field,PixLib::COMMIT,dbval);
		  (*pmIter)->pushField(new_field);
		} else {
		  bool dbval=true;
		  confDBInterface->DBProcess(field, COMMIT, dbval);
		}

		// set channel mode for BIC config
		if(ScanParameters.adapterCardFlavour==1){
		  for(int iFe=0; iFe<4; iFe++){
		    int dbval = 0;
		    if(roChanInput.find(boardid)!=roChanInput.end())
		      dbval = roChanInput[boardid].at(iFe);
		    std::stringstream sv;
		    sv << "general_readoutChannelsInput";
		    sv << iFe;
		    field = (*pmIter)->findField(sv.str());
		    if (field==(*pmIter)->fieldEnd()) {
		      // field not existing!
		      PixLib::DBField *new_field = confDBInterface->makeField(sv.str());
		      confDBInterface->DBProcess(new_field,PixLib::COMMIT,dbval);
		      (*pmIter)->pushField(new_field);
		    } else{
		      confDBInterface->DBProcess((*field),PixLib::COMMIT,dbval);
		    }
		    dbval=0;
		    if(roChanFeid.find(boardid)!=roChanFeid.end())
		      dbval = roChanFeid[boardid].at(iFe);
		    std::stringstream sv2;
		    sv2 << "general_readoutChannelReadsChip";
		    sv2 << iFe;
		    field = (*pmIter)->findField(sv2.str());
		    if (field==(*pmIter)->fieldEnd()) {
		      // field not existing!
		      PixLib::DBField *new_field = confDBInterface->makeField(sv2.str());
		      confDBInterface->DBProcess(new_field,PixLib::COMMIT,dbval);
		      (*pmIter)->pushField(new_field);
		    } else{
		      confDBInterface->DBProcess((*field),PixLib::COMMIT,dbval);
		    }
		  }
		}
		confDBInterface->DBProcess((*pmIter), PixLib::COMMITREPLACE);
	      }
	    }
	  }
	
      }
    }
  }

  delete confDBInterface;
  loadDB(filename.toLatin1().data());
}
void STControlEngine::setShowErrorPopups( bool showErr){
  m_showRodWin = showErr;
  m_showErrWin = showErr;
}

