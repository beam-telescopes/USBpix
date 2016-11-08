#include "STPixModuleGroup.h"
#include "STRodCrate.h"
#include "ChipTest.h"

#include <VmeInterface.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixDcs/SleepWrapped.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/RootDB.h>
#include <PixController/PixScan.h>
#include <PixController/PixController.h>
#ifndef NOTDAQ
#include <PixController/RodPixController.h>
#include <PixBoc/PixBoc.h>
#endif
#include <PixController/USBPixController.h>
#include <PixController/PixScan.h>
#include <PixDcs/PixDcs.h>
#include <PixMcc/PixMcc.h>
#include <PixFe/PixFe.h>
#include <PixMcc/PixMccExc.h>
#include <PixFe/PixFeExc.h>
#include <PixFe/PixFeI4A.h>
#include <PixFe/PixFeI4B.h>
#include <PixModule/PixModule.h>
#include <Bits/Bits.h>
#include <BaseException.h>
#include <qeventloop.h>
#include <DataContainer/PixDBData.h>

#include <exception>
#include <sstream>
#include <iostream>
#include <string>
#include <sstream>
#include <functional>

#include <ctime>
#ifdef WIN32
#include <stdlib.h>
#else
#include <unistd.h>
#endif

#include <QDateTime>
#include <QDebug>
#include <QFuture>
#include <QtConcurrentRun>

#include <TMath.h>

#define STEP_DEBUG 0

using namespace PixLib;
using namespace SctPixelRod;

// temporary place, should go somewhere more general
int solveCubic(double par[], double *res){
  if(par[3]==0 && par[2]==0 && par[1]==0 ) return 0;

  if(par[3]==0 && par[2]==0 ){
    double tmp1 = -par[0]/par[1];
    res[0] = tmp1;
    return 1;
  }

  if(par[3]==0){
    double tmp1 = par[0]/par[2]-par[1]*par[1]/(4*par[2]*par[2]);
    if(tmp1<0) return 0;
    res[0] = sqrt(tmp1)-par[1]/2/par[2];
    res[1] = -sqrt(tmp1)-par[1]/2/par[2];
    return 2;
  }

  double p,q,D;

  double a = par[2]/par[3];
  double b = par[1]/par[3];
  double c = par[0]/par[3];

  p = b - pow(a,2)/3;
  q = c + 2*pow(a,3)/27 - a*b/3;

  D = q*q/4 + p*p*p/27;

  if(D<-1e-10){
    double tmp1 = sqrt(-p/3), tmp2 = acos(-q/2*sqrt(-27/p/p/p));
    double pi=3.1415926535897932384626433832795;
    res[0] =  2*tmp1*cos(tmp2/3) - a/3;
    res[1] = -2*tmp1*cos(tmp2/3+pi/3) - a/3;
    res[2] = -2*tmp1*cos(tmp2/3-pi/3) - a/3;
    return 3;
  } else if(D>1e-10){
    double tmp1, tmp2;
    tmp1 = -q/2+sqrt(D);
    if(tmp1<0) 
      tmp1 = -pow(-tmp1,0.33333333333);
    else
      tmp1 = pow(tmp1,.33333333333);
    tmp2 = -q/2-sqrt(D);
    if(tmp2<0) 
      tmp2 = -pow(-tmp2,0.3333333333);
    else
      tmp2 = pow(tmp2,.33333333);
    res[0] = tmp1 + tmp2 - a/3;
    return 1;
  } else{
    if(p==q){
      res[0] = 0;
      return 1;
    } else{
      res[0] = -pow((4*q),.33333333)-b/3/a;
      res[1] = pow((q/2),.33333333)-a/3;
      return 2;
    }
  }
  
}

QMutex* STPixModuleGroup::writeToFileMutex =  new QMutex();

STPixModuleGroup::STPixModuleGroup(int ID, PixConfDBInterface *db, DBInquire *dbInquire, QApplication* application, bool &filebusy, 
				   PixLib::Config &options, QObject *parent)
  : QObject(parent), PixModuleGroup(db, dbInquire), m_ID(ID), m_filebusy(filebusy), m_options(options), m_decName(""){
  m_CtrlStatus=tblocked;
  m_BocStatus=tblocked;
  m_PixScan = new PixScan();
  for(int i=0;i<3;i++)
    m_pixScanLoop[i] = false;  
  m_app = application;
  m_ctrlThr = new STPixModuleGroup::CtrlThread(*this, m_app);
  processing = false;
  m_nSteps[0] = 0;
  m_nSteps[1] = 0;
  m_nSteps[2] = 0;
  m_nMasks = 0;
  m_currSRAMFillLevel = 0;
  m_currTriggerRate = 0;
  m_currEvtRate = 0;
  m_currTluVeto = false;
  m_currFe = -1;
  ScanStatus = 1;
  m_srvCounts.clear();
  if(dbInquire!=0)
    m_decName = dbInquire->getDecName();

  // edited status of group, ROD and BOC
  m_grpEdited = false;
  m_rodEdited = false;
  m_bocEdited = false;
  // special storage of module status and mod cfg edited status
  for(moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    modStatus modS;
    modS.modStat = tunknown;
    modS.mccStat = ((*mi)->pixMCC()!=0)?tunknown:tblocked;
    modS.modMsg = "";
    modS.mccMsg = "";
    for(int chip=0;chip<16;chip++){
      modS.feStat[chip] = ((*mi)->pixFE(chip)!=0)?tunknown:tblocked;
      modS.feMsg[chip] = "";
    }
    m_pixModuleStatus.insert(std::make_pair((*mi)->moduleId(),modS));
    m_modEdited.insert(std::make_pair((*mi)->moduleId(),1));
    // temporary fix: read connectivity name
    std::string fval="";
    Config &conf = (*mi)->config();
    std::string find_name = (*mi)->moduleName();
    find_name += "PixModule;1";
    std::vector<DBInquire*> modInq;
    try{
      modInq = db->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
    }catch(...){
      modInq.clear();
    }
    if(modInq.size()==1){
      fieldIterator f = modInq[0]->findField("geometry_connName");
      if(f!=modInq[0]->fieldEnd()){
	db->DBProcess(*f,READ,fval);
      }
    }
    if(conf["geometry"].name()!="__TrashConfGroup__"){
      // add connectivity name
      m_pixModConnName[(*mi)->moduleId()] = fval;
      conf["geometry"].addString("connName", m_pixModConnName[(*mi)->moduleId()], fval,
				 "connectivity table name of this module", true);
    }
  }
}

STPixModuleGroup::STPixModuleGroup(int ID, QApplication* application, bool &filebusy, PixLib::Config &options, QObject *parent)
  : QObject(parent), PixModuleGroup(), m_ID(ID), m_filebusy(filebusy), m_options(options), m_decName(""){
  m_CtrlStatus=tblocked;
  m_BocStatus=tblocked;
  m_PixScan = new PixScan();  
  for(int i=0;i<3;i++)
    m_pixScanLoop[i] = false;
  m_app=application;
  m_ctrlThr = new STPixModuleGroup::CtrlThread(*this, m_app);
  processing = false;
  m_nSteps[0] = 0;
  m_nSteps[1] = 0;
  m_nSteps[2] = 0;
  m_nMasks = 0;
  m_currSRAMFillLevel = 0;
  m_currTriggerRate = 0;
  m_currEvtRate = 0;
  m_currTluVeto = false;
  m_currFe = -1;
  ScanStatus = 1;
  m_srvCounts.clear();
 
  // special storage of module status
  for(moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    modStatus modS;
    modS.modStat = tunknown;
    modS.mccStat = ((*mi)->pixMCC()!=0)?tunknown:tblocked;
    modS.modMsg = "";
    modS.mccMsg = "";
    for(int chip=0;chip<16;chip++){
      modS.feStat[chip] = ((*mi)->pixFE(chip)!=0)?tunknown:tblocked;
      modS.feMsg[chip] = "";
    }
    m_pixModuleStatus.insert(std::make_pair((*mi)->moduleId(),modS));
    m_modEdited.insert(std::make_pair((*mi)->moduleId(),1));
  }
}

STPixModuleGroup::~STPixModuleGroup(){
  delete m_PixScan;
  delete m_ctrlThr;
}

void STPixModuleGroup::readRodBuff(std::string msg){
  std::stringstream out_msg;
  std::string txt;
  bool err=false;
  if(getPixController() && (getPixCtrlStatus()==tOK
	     || getPixCtrlStatus()==tbusy
	     || getPixCtrlStatus()==tscanning)){
    out_msg << msg << "\nController of grp. "<< getName() << "\n";
    out_msg << "============================= Err Buffer" << "\n";
    while (getPixController()->getGenericBuffer("err",txt)){
      out_msg << txt << "\n";
      err = true;
    }
    out_msg << "============================= Info Buffer" << "\n";
    while (getPixController()->getGenericBuffer("info",txt)) out_msg << txt << "\n";
  } else
    out_msg << "Controller-hardware is not available." << "\n";
  emit rodMessage(out_msg.str(), err);
}
/** Read status of a module if found. */
modStatus STPixModuleGroup::getPixModuleStatus(int modID){
  return m_pixModuleStatus[modID];
}
/** Change property of entry in map <StatusTag> m_pixModuleSTatus(Info). */

void STPixModuleGroup::setPixModuleStatus(int modID, modStatus newStatus, bool emitSignal){
  m_pixModuleStatus[modID] = newStatus;
  //  emit groupChanged();
  if(emitSignal)
    emit statusChanged();
  return;
}
/** copy the content of a PixScan object to the one owned by this group */
int STPixModuleGroup::setPixScan(PixScan *inPixScan){

  m_PixScan->config() = inPixScan->config();


  // S-curve fitting enabled? must set histogram keep/fill flags properly!
  bool doSFit = false;
  for(int nl=0;nl<MAX_LOOPS;nl++)
    doSFit |= ((m_PixScan->getLoopAction(nl)==PixScan::SCURVE_FIT || m_PixScan->getLoopAction(nl)==PixScan::SCURVE_FAST)
	       && m_PixScan->getLoopActive(nl));

  m_PixScan->setHistogramFilled(PixScan::SCURVE_MEAN, doSFit);
  m_PixScan->setHistogramKept(  PixScan::SCURVE_MEAN, doSFit);
  m_PixScan->setHistogramFilled(PixScan::SCURVE_SIGMA,doSFit);
  m_PixScan->setHistogramKept(  PixScan::SCURVE_SIGMA,doSFit);
  m_PixScan->setHistogramFilled(PixScan::SCURVE_CHI2, doSFit);
  m_PixScan->setHistogramKept(  PixScan::SCURVE_CHI2, doSFit);

  return 0;
}
 
STPixModuleGroup::CtrlThread::CtrlThread(STPixModuleGroup& group, QApplication * application):m_STPixModuleGroup(&group)
{
  threadtag = MTundecided;
  m_app = application;
};

void STPixModuleGroup::ThreadExecute(ThreadTag tag)
{
  while((m_ctrlThr->isRunning()))  PixLib::sleep(10);
  
  if (tag == MTsetmcc)
    emit logMessage("STPixModuleGroup::ThreadExecute() started MTsetmcc without specifying fullXCK");
  
  if(tag == MTscan)
    emit logMessage("STPixModuleGroup::ThreadExecute() startet MTscan without specifying ScanOptions"); 

  if (tag == MTchiptest)
    emit logMessage("STPixModuleGroup::ThreadExecute() started MTchiptest without specifying ChipTest object");

  if (tag == MTresetmods)
    emit logMessage("STPixModuleGroup::ThreadExecute() started MTresetmods without specifying hard/soft option");

  std::vector<int> opts;
  opts.push_back(0);
  m_ctrlThr->setTagOpt(opts); // some functions can be called w/o argument, 0 is then default
  m_ctrlThr->setThreadTag(tag);
  m_ctrlThr->start();
  return;
}

void STPixModuleGroup::ThreadExecute(ThreadTag tag, std::vector<int> opts)
{
  while((m_ctrlThr->isRunning())) PixLib::sleep(10);
  
  if (tag != MTsetmcc && tag != MTconfig && tag != MTresetmods)
    emit logMessage("STPixModuleGroup::ThreadExecute() : option-int set although not needed by performed operation");

  m_ctrlThr->setTagOpt(opts); // some functions can be called w/o argument, 0 is then default
  m_ctrlThr->setTagStrg("ALL");
  m_ctrlThr->setThreadTag(tag);
  m_ctrlThr->start();
  return;
}
void STPixModuleGroup::ThreadExecute(ThreadTag tag, std::vector<int> opts, std::string latch)
{
  while((m_ctrlThr->isRunning())) PixLib::sleep(10);
  
  if (tag != MTconfig)
    emit logMessage("STPixModuleGroup::ThreadExecute() : option-int&string set although not needed by performed operation");
  
  m_ctrlThr->setTagOpt(opts); // some functions can be called w/o argument, 0 is then default
  m_ctrlThr->setTagStrg(latch);
  m_ctrlThr->setThreadTag(tag);
  m_ctrlThr->start();
  return;
}
void STPixModuleGroup::ThreadExecute(ThreadTag tag, pixScanRunOptions runOpts)
{
  while((m_ctrlThr->isRunning())) PixLib::sleep(10);
  
  if (tag != MTscan)
    emit logMessage("STPixModuleGroup::ThreadExecute() : runOpts set although not needed by performed operation");

  m_ctrlThr->setScanOpts(runOpts);
  m_abortScan = false;
  m_ctrlThr->setThreadTag(tag);
  m_ctrlThr->start();
  return;
}
void STPixModuleGroup::ThreadExecute(ThreadTag tag, ChipTest *ct, bool resetMods){
  while((m_ctrlThr->isRunning())) PixLib::sleep(10);
  
  if (tag != MTchiptest)
    emit logMessage("STPixModuleGroup::ThreadExecute() : ChipTest object set although not needed by performed operation");

  m_ctrlThr->setCtOpts(ct, resetMods);
  m_abortScan = false;
  m_ctrlThr->setThreadTag(tag);
  m_ctrlThr->start();
  return;
}
void STPixModuleGroup::CtrlThread::setTagOpt(std::vector<int> opt){
  m_tagOpt.clear();
  m_tagOpt=opt; 
  return;
}
void STPixModuleGroup::CtrlThread::run()
{

  std::stringstream msg;
  // try to catch any kind of problem - avoids uncontrolled crashes (hopefully)
  try{
    switch(threadtag){
    case MTscan:
      scan();
      break;
    case MTinitrods: 
      initrods();
      break;
    case MTinitbocs:
      initbocs();
      break;
    case MTtrigger:
      trigger();
      break;
    case MTsrvrec:
      getSrvRec();
      break;
    case MTconfig:
      config();
      break;
    case MTresetmods:
      resetmods();
      break;
    case MTsetmcc: 
      setmcc();
      break;
    case MTresetrods:
      resetrods();
      break;
    case MTlinktest:
      runLinkTest();
      break;
    case MTchiptest:
      runChipTest();
      break;
    case MTundecided:{
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::run():  Error in arguments (threadtag = MTundecided) with group "
						   + getSTPixModuleGroup()->getName()+"\n" );
      m_app->postEvent(getSTPixModuleGroup(), tee);
      break;}
    default:{
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::run():  Error in arguments (threadtag has unknown value) with group "
						   + getSTPixModuleGroup()->getName()+ "\n" );
      m_app->postEvent(getSTPixModuleGroup(), tee);
    }
    }
  } catch (SctPixelRod::VmeException& v) {
    msg << "VME-exception ";
    msg << v;
  } catch(SctPixelRod::BaseException& b){
    msg << "Base exception ";
    msg << b;
  } catch(std::exception& s){
    msg << "Std-lib exception ";
    msg << s.what();
  } catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    std::stringstream a;
    a << (int)threadtag;
    ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::run(): Exception " + msg.str() +
					     " not caught during execution of tag " +a.str() + " with group "+
					     getSTPixModuleGroup()->getName() + " at " + 
					     std::string(QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy").toLatin1().data())+"\n" );
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->ScanStatus=1;
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
  }
}

void STPixModuleGroup::CtrlThread::readRodBuff(std::string msg)
{
  std::stringstream out_msg;
  std::string txt;
  bool haveErr=false;
  out_msg << msg << "\nController of grp. "<< getSTPixModuleGroup()->getName() << "\n";
  if(getSTPixModuleGroup()->getPixController() && (getSTPixModuleGroup()->getPixCtrlStatus()==tOK
	     || getSTPixModuleGroup()->getPixCtrlStatus()==tbusy
	     || getSTPixModuleGroup()->getPixCtrlStatus()==tscanning)){
    out_msg << "============================= Err Buffer" << "\n";
    while (getSTPixModuleGroup()->getPixController()->getGenericBuffer("err",txt)){
      out_msg << txt << "\n";
      haveErr = true;
    }
    out_msg << "============================= Info Buffer" << "\n";
    while (getSTPixModuleGroup()->getPixController()->getGenericBuffer("info",txt)) out_msg << txt << "\n";
  } else
    out_msg << "Controller-hardware is not available." << "\n";
  ThreadBufferEvent* tlb = new ThreadBufferEvent(out_msg.str(), haveErr);
  m_app->postEvent(getSTPixModuleGroup(), tlb);
}

void STPixModuleGroup::CtrlThread::scan()
{
  getSTPixModuleGroup()->processing = true;
  
  // see whether there is at least one enabled module, otherwise scan runs into endless loop.
  bool haveValidMod = false;
  for(PixModuleGroup::moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
    if(((*mi)->config())["general"].name()!="__TrashConfGroup__" &&
       ((*mi)->config())["general"]["Active"].name()!="__TrashConfObj__" &&
       (((ConfList&)((*mi)->config())["general"]["Active"]).sValue()!="FALSE")){ 
      haveValidMod = true;
      break;
    }
  }
  if(!haveValidMod)	{
    ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::scan(): No enabled Module in group "  
					     + getSTPixModuleGroup()->getName() + " - scan not started\n" );
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }

  // init counter which is used later to check if ROD buffer should be read
  m_checkScanCount = 0;
  getSTPixModuleGroup()->m_scanDcsReadings.clear();
  getSTPixModuleGroup()->m_forceRead = false;

  bool onlyDcsScanned=true;
  std::map<int, double> modDcsSettings;
  try{
    PixController *pc = getSTPixModuleGroup()->getPixController();
   
    for (int i =0; i<3; i++)
      getSTPixModuleGroup()->m_nSteps[i] = 0;
    
    getSTPixModuleGroup()->m_nMasks = 0;
    getSTPixModuleGroup()->m_currSRAMFillLevel = 0;
    getSTPixModuleGroup()->m_currTriggerRate = 0;
    getSTPixModuleGroup()->m_currEvtRate = 0;
    getSTPixModuleGroup()->m_currTluVeto = false;
    getSTPixModuleGroup()->ScanStatus = 10;
    getSTPixModuleGroup()->m_currFe = -1;

    PixScan &cfg = *(getSTPixModuleGroup()->getPixScan());

    cfg.resetScan();
    for(int il=0;il<3;il++){
      PixScan::ScanParam par = cfg.getLoopParam(il);
      // set DCS value if scanned
      if(cfg.getLoopActive(il) && (par!=PixScan::DCS_VOLTAGE || cfg.getDcsChan()=="") &&
	 (par!=PixScan::DCS_PAR1 || cfg.getDcsScanPar1ChannelName()=="" ) &&
	 (par!=PixScan::DCS_PAR2 || cfg.getDcsScanPar2ChannelName()=="" ) &&
	 (par!=PixScan::DCS_PAR3 || cfg.getDcsScanPar3ChannelName()=="" ) ) onlyDcsScanned = false;
    }

    // Make sure we have a valid PixController
    if (pc) {
      // and that it is initialised
      if(getSTPixModuleGroup()->getPixCtrlStatus()==tOK) {
	
	// variables for SDSP settings
	bool sdspUsed[4]={false,false,false,false};
	int modMask[4]={0,0,0,0};
	// Module configuration
	//            std::cout << "Configuring Modules: ";
	for(PixModuleGroup::moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
	  //              std::cout << (*mi)->moduleName() << "; " << " Id: " << (*mi)->moduleId();
	  //(*mi)->writeConfig(); // send config from SBC to ROD
	  if(scanOpts.determineSDSPcfg){ 
	    // determine SDSP setup
	    Config &conf = (*mi)->config();
	    if(conf["general"].name()!="__TrashConfGroup__"){
	      if(conf["general"]["Active"].name()!="__TrashConfObj__" &&
		 conf["general"]["GroupId"].name()!="__TrashConfObj__"){
		unsigned int gID = ((ConfInt&)conf["general"]["GroupId"]).value();
		if(gID<4){
		  if(((ConfList&)conf["general"]["Active"]).sValue()=="TRUE")
		    sdspUsed[gID] = true;
		  if(((ConfList&)conf["general"]["Active"]).sValue()=="TRUE" &&
		     conf["general"]["ModuleId"].name()!="__TrashConfObj__")
		    modMask[gID] += 1<<(((ConfInt&)conf["general"]["ModuleId"]).value());
		}
	      }
	    }
	  }
	}
	if(scanOpts.determineSDSPcfg){ // user requested to have SDSP config to be calculated
	  // write into scan config
	  Config &conf = cfg.config();
	  if(conf["modGroups"].name()!="__TrashConfGroup__"){
	    for(int i=0;i<4;i++){
	      std::string subName;
	      std::stringstream b;
	      b << i;
	      subName = "moduleMask_";
	      subName += b.str();
	      if(conf["modGroups"][subName].name()!="__TrashConfObj__"){
		WriteIntConf(((ConfInt&)conf["modGroups"][subName]), modMask[i]);
		//printf("Mask for SDSP%d = %d\n",i,((ConfInt&)conf["modGroups"][subName]).value());
	      }else{
		ThreadLogEvent* tle = new ThreadLogEvent("cannot set modGroups_"+subName+"\n");
		m_app->postEvent(getSTPixModuleGroup(), tle);
	      }
	      subName = "configEnabled_";
	      subName += b.str();
	      if(conf["modGroups"][subName].name()!="__TrashConfObj__")
		((ConfBool&)conf["modGroups"][subName]).m_value = sdspUsed[i];
	      else{
		ThreadLogEvent* tle = new ThreadLogEvent("cannot set modGroups_"+subName+"\n");
		m_app->postEvent(getSTPixModuleGroup(), tle);
	      }
	      subName = "strobeEnabled_";
	      subName += b.str();
	      if(conf["modGroups"][subName].name()!="__TrashConfObj__")
		((ConfBool&)conf["modGroups"][subName]).m_value = sdspUsed[i];
	      else{
		ThreadLogEvent* tle = new ThreadLogEvent("cannot set modGroups_"+subName+"\n");
		m_app->postEvent(getSTPixModuleGroup(), tle);
	      }
		subName = "triggerEnabled_";
	      subName += b.str();
	      if(conf["modGroups"][subName].name()!="__TrashConfObj__")
		((ConfBool&)conf["modGroups"][subName]).m_value = sdspUsed[i];
	      else{
		ThreadLogEvent* tle = new ThreadLogEvent("cannot set modGroups_"+subName+"\n");
	      	m_app->postEvent(getSTPixModuleGroup(), tle);
	      }
	      subName = "readoutEnabled_";
	      subName += b.str();
	      if(conf["modGroups"][subName].name()!="__TrashConfObj__")
		((ConfBool&)conf["modGroups"][subName]).m_value = sdspUsed[i];
	      else{
		ThreadLogEvent* tle = new ThreadLogEvent("cannot set modGroups_"+subName+"\n");
		m_app->postEvent(getSTPixModuleGroup(), tle);
	      }
	    }
	  } else{
	    ThreadLogEvent* tle = new ThreadLogEvent("cannot set modGroups\n");
	    m_app->postEvent(getSTPixModuleGroup(), tle);
	  }
	}

	// tmp FEbyFE
	if(scanOpts.runFEbyFE){ // temporary way to scan FE by FE
	  getSTPixModuleGroup()->selectFe(0,-1);
	  getSTPixModuleGroup()->downloadConfig();
	}
	// end tmp FEbyFE
	getSTPixModuleGroup()->initScan(&cfg);
	
      } else if (cfg.getDcsChan()=="" || !onlyDcsScanned){
	ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::scan() : PixModuleGroup " + getSTPixModuleGroup()->getName() + " has an uninitialised PixController, not proceeding\n");
	m_app->postEvent(getSTPixModuleGroup(), tle);
	return;
      }
    } else {
      ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::scan() : Couldn't find any valid PixController in PixModuleGroup " + getSTPixModuleGroup()->getName() + "\n");
      m_app->postEvent(getSTPixModuleGroup(), tle);
      return;
    }
  }
  catch (PixScanExc &psExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixScan-exception level " << psExc.dumpLevel() << 
      " while initialising scan " << psExc << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (USBPixControllerExc &upcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    std::stringstream msg;
    upcExc.dump(msg);
    txt << "STPixModuleGroup::CtrlThread::scan() : USBPixController-exception (grp. " << getSTPixModuleGroup()->getName() << 
      ") while initialising scan: " << msg.str() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
#ifndef NOTDAQ
  catch (RodPixControllerExc &rpcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    std::stringstream msg;
    rpcExc.dump(msg);
    txt << "STPixModuleGroup::CtrlThread::scan() : RodPixController-exception (grp. " << getSTPixModuleGroup()->getName() << 
      ") while initialising scan: " << msg.str() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
#endif
  catch (PixControllerExc &pcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    std::stringstream msg;
    pcExc.dump(msg);
    txt << "STPixModuleGroup::CtrlThread::scan() : PixController-exception (grp. " << getSTPixModuleGroup()->getName() << 
      ") while initialising scan: " << msg.str() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixModuleExc &mExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixModule-exception level " << mExc.dumpLevel() << 
      " while initialising scan " << mExc << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixMccExc &mcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixMcc-exception level " << mcExc.dumpLevel() << 
      " while initialising scan " << mcExc.dumpType() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixFeExc &fExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixFe-exception level " << fExc.dumpLevel() << 
      " while initialising scan " << fExc.dumpType() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (ConfigExc & cfExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : Config-exception while initialising scan " << cfExc << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (VmeException & vexc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : VME-exception while initialising scan " << vexc << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (BaseException & exc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : SCT-pixel base-exception while initialising scan " << exc << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch(std::exception& sexc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : Std-lib-exception while initialising scan " << sexc.what() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (...){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::scan() : Unexpected exception while initialising scan\n");
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }

  readRodBuff("Post-scan-init info:");

  try{  
    PixController *pc = getSTPixModuleGroup()->getPixController();
    PixScan &cfg = *(getSTPixModuleGroup()->getPixScan());
    cfg.resetScan();
    // check if histograms from controller are requested
    bool histoOnCtrl=false;
    std::map<std::string, int> htv = cfg.getHistoTypes();
    for(std::map<std::string, int>::iterator IT = htv.begin(); IT!=htv.end();IT++){
      if(PixLib::PixScan::DCS_DATA != (PixLib::PixScan::HistogramType)IT->second && 
	 PixLib::PixScan::HIT_RATE != (PixLib::PixScan::HistogramType)IT->second &&
	 PixLib::PixScan::TRG_RATE != (PixLib::PixScan::HistogramType)IT->second)
	histoOnCtrl |= cfg.getHistogramFilled((PixLib::PixScan::HistogramType)IT->second);
    }

    // make sure we have a valid and initialized Pixcontroller  
    if(pc){
      
      getSTPixModuleGroup()->ScanStatus = 0;
      
      if(getSTPixModuleGroup()->getPixCtrlStatus()==tOK && !onlyDcsScanned){
	setCtrlStatusEvent* cse = new setCtrlStatusEvent(tbusy,getSTPixModuleGroup());
	m_app->postEvent(getSTPixModuleGroup(), cse);
      }
      // if scan modifies and does *not* restore module config,
      // set edited flags accordingly
      if(!(cfg.getRestoreModuleConfig())){
	getSTPixModuleGroup()->editedCfgAllMods(); // all modules
	getSTPixModuleGroup()->editedCfg(40); // module group
	getSTPixModuleGroup()->editedCfg(41); // PixController
      }
      
      //start scan
      getSTPixModuleGroup()->scanLoopStart(2, &cfg);
      
	unsigned int size_of_Raw_file_name = 0;				//later used for variation of the raw file name in order to extract each raw file during a scan with dcs parameter variation

      while(cfg.loop(2) && !getSTPixModuleGroup()->doAbortScan()){   
	
	getSTPixModuleGroup()->prepareStep(2, &cfg);
	PixScan::ScanParam par = cfg.getLoopParam(2);
	// set DCS value if scanned
	if(cfg.getLoopActive(2) && (par==PixScan::DCS_VOLTAGE||par==PixScan::DCS_PAR1||par==PixScan::DCS_PAR2||par==PixScan::DCS_PAR3))
	  setDcsScanVal(2, &cfg);
	getSTPixModuleGroup()->scanLoopStart(1, &cfg);
	
	while(cfg.loop(1) && !getSTPixModuleGroup()->doAbortScan()){
	  getSTPixModuleGroup()->prepareStep(1, &cfg);
	  par = cfg.getLoopParam(1);
	  // set DCS value if scanned
	  if(cfg.getLoopActive(1) && (par==PixScan::DCS_VOLTAGE||par==PixScan::DCS_PAR1||par==PixScan::DCS_PAR2||par==PixScan::DCS_PAR3))
	    setDcsScanVal(1, &cfg);
	  getSTPixModuleGroup()->scanLoopStart(0, &cfg);
	


	  // tmp FEbyFE
	  int feStart=0;
	  int feEnd=1;
	  if(scanOpts.runFEbyFE) feEnd = 16;
	  PixScan *tmpScans[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	  
	  for(int ife=feStart; ife<feEnd; ife++){
	    if(ife>0){
	      tmpScans[ife] = new PixScan();
	      tmpScans[ife]->config() = getSTPixModuleGroup()->getPixScan()->config();
	    }else
	      tmpScans[ife] = getSTPixModuleGroup()->getPixScan();
	  }
	  for(int ife=feStart; ife<feEnd && !getSTPixModuleGroup()->doAbortScan(); ife++){
	    
	    if(scanOpts.runFEbyFE){
	      getSTPixModuleGroup()->m_currFe = ife;
	      for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
		if((*mi)->pixFE(ife)!=0) (*mi)->pixFE(ife)->restoreConfig("PreFEbyFEConfig");
	      }
	    }
	    // configure module
	    if(scanOpts.runFEbyFE && !onlyDcsScanned){
	      getSTPixModuleGroup()->downloadConfig();
	      pc->setConfigurationMode();
	      pc->sendModuleConfig(0);
	    }
	    
	    // tmp FEbyFE
	    while(tmpScans[ife]->loop(0) && !getSTPixModuleGroup()->doAbortScan()){
	      // org code
	      //while(cfg.loop(0) && !getSTPixModuleGroup()->doAbortScan())
	      // end tmp FEbyFE
	      

		if(!tmpScans[ife]->getSourceRawFile().empty())										//look whether the string is used or not
		{
			std::string SourceRawFileNameTmp = tmpScans[ife]->getSourceRawFile();						//put file name in a temporary string			
			if(getSTPixModuleGroup()->getPixScan()->scanIndex(0)+getSTPixModuleGroup()->getPixScan()->scanIndex(1)+getSTPixModuleGroup()->getPixScan()->scanIndex(2)==0) 
			  size_of_Raw_file_name = SourceRawFileNameTmp.size()-4;					//determine the size of the string in the first round	
			SourceRawFileNameTmp.resize(size_of_Raw_file_name);								//erase the ".raw" at the end in the first round and afterwards also the previous changes
			SourceRawFileNameTmp.insert(SourceRawFileNameTmp.size(),"_"+std::to_string(getSTPixModuleGroup()->getPixScan()->scanIndex(0))+"_"+std::to_string(getSTPixModuleGroup()->getPixScan()->scanIndex(1))+"_"+std::to_string(getSTPixModuleGroup()->getPixScan()->scanIndex(2))+".raw");											//insert "loop0_loop1_loop2.raw" at the end of the file name
			tmpScans[ife]->setSourceRawFile(SourceRawFileNameTmp);  							//rename .raw file to output all individual files in case of loops during source scans
		}
	      
	      // tmp FEbyFE
	      getSTPixModuleGroup()->prepareStep(0, tmpScans[ife]);
	      par = tmpScans[ife]->getLoopParam(0);
	      // if DCS variables are scanned, talk to DCS
	      if(tmpScans[ife]->getLoopActive(0) && (par==PixScan::DCS_VOLTAGE||par==PixScan::DCS_PAR1||par==PixScan::DCS_PAR2||par==PixScan::DCS_PAR3)) 
		setDcsScanVal(0, tmpScans[ife]);
	      // update scan counters
	      for(int i=2;i>=0;i--)
		getSTPixModuleGroup()->m_nSteps[i]=tmpScans[ife]->scanIndex(i);
	      getSTPixModuleGroup()->m_nMasks=tmpScans[ife]->getMaskStageIndex();
	      
	      // run all scans as usual - requires PixController to return immediately after scan init
	      if(STEP_DEBUG) qDebug() << "Running scan in main thread!";
	      getSTPixModuleGroup()->scanExecute(tmpScans[ife]);

	      // wait till the controller knows it is in scanning mode
	      int timeoutCnt=0; 
	      // get timeout cfg. par. - is in minutes, so convert to 0.1 s
	      int tMax=600*((ConfInt&)getSTPixModuleGroup()->getOptions()["errors"]["scanTimeout"]).getValue();
	      
	      // the following checks that the PixController really started the scan
	      // in case it has to do the mask staging and fills some histograms 
	      // (otherwise there isn't much to be done here and thus nothing to be checked)
	      if (cfg.getRunType() == PixScan::NORMAL_SCAN && tmpScans[ife]->getDspMaskStaging() && histoOnCtrl){
		int status;
		while ((status=pc->runStatus()==0) &&timeoutCnt<tMax){
		  PixLib::sleep(100);
		  timeoutCnt++;
		}
	      }
	      if(timeoutCnt<tMax){ //ok, it is scanning
		setCtrlStatusEvent* cse = new setCtrlStatusEvent(tscanning,getSTPixModuleGroup());
		m_app->postEvent(getSTPixModuleGroup(), cse);
		
	      }else{  //timeout, skip this controller
		setCtrlStatusEvent* cse3 = new setCtrlStatusEvent(tOK,getSTPixModuleGroup());
		m_app->postEvent(getSTPixModuleGroup(), cse3);
		
		std::stringstream msg;
		msg<<"STPixModuleGroup::CtrlThread::scan() : PixController for group " + getSTPixModuleGroup()->getName() + 
		  " failed to start scan in time. \n";
		ThreadErrorEvent* tee = new ThreadErrorEvent(msg.str());
		m_app->postEvent(getSTPixModuleGroup(), tee);
		
		getSTPixModuleGroup()->processing = false;
		getSTPixModuleGroup()->ScanStatus=1;
		setThreadTag(MTundecided);
		return;
	      }
	      
	      // wait until controller has finished scanning
	      int nloop=0;
	      int nAbortWait=0;

		
	      //For console output merely
	      int debug_counter1 = 5;
	      int debug_counter2 = 10;


	      do{

	
		if(getSTPixModuleGroup()->doAbortScan()) nAbortWait++;
		
		unsigned long tsleep = (tmpScans[ife]->getSourceScanFlag())?200:1000;
		if (cfg.getRunType() == PixScan::NORMAL_SCAN && cfg.getDspMaskStaging() && histoOnCtrl)
		  PixLib::sleep(tsleep);

		nloop = pc -> nTrigger();
		
		if(cfg.getSourceScanFlag()) {
		  getSTPixModuleGroup()->m_nMasks=nloop;
		  
		  // USBpix Source Scan
		    
		    getSTPixModuleGroup()->m_currSRAMReadoutReady = pc->getSRAMReadoutReady();
		    getSTPixModuleGroup()->m_currSRAMFull = pc->getSRAMFull();
		    getSTPixModuleGroup()->m_currMeasurementPause = pc->getMeasurementPause();
		    getSTPixModuleGroup()->m_currMeasurementRunning = pc->getMeasurementRunning();
		    getSTPixModuleGroup()->m_currSRAMFillLevel = pc->getSRAMFillLevel();
		    getSTPixModuleGroup()->m_currTluVeto = pc->getTluVeto();
		    getSTPixModuleGroup()->m_currTriggerRate = pc->getTriggerRate();
		    getSTPixModuleGroup()->m_currEvtRate = pc->getEventRate();
		   
		    if(cfg.getTestBeamFlag()) {	
				if(STEP_DEBUG) {
			    	if(debug_counter1%5==0) {
						qDebug() << "Board("<< pc->getBoardID() << ") SRAM_READOUT_AT: " <<cfg.getSramReadoutAt()
						<< "%, Currently at: " <<getSTPixModuleGroup()->m_currSRAMFillLevel<<"%";
					}
			    	debug_counter1++;	
				}
			}
		}

		else if(cfg.getDspMaskStaging()) {
		  getSTPixModuleGroup()->m_nMasks=(nloop>>12);
		}
		
		for(int i=2;i>=0;i--){
		  // tmp FEbyFE
		  int measInd = tmpScans[ife]->scanIndex(i);
		  if((i==0)&&tmpScans[ife]->getDspProcessing(0))
		    //int measInd = cfg.scanIndex(i);
		    //if((i==0)&&cfg.getDspProcessing(0))
		    // end tmp FEbyFE
		    measInd = nloop&0xfff;
		  
		  getSTPixModuleGroup()->m_nSteps[i]=measInd;
		}
		
		// abort ROD scan if requested by user
		if( (nloop>0||pc!=0) && getSTPixModuleGroup()->doAbortScan() ){
		  pc->stopScan();
		  // source scan without loop: onlz stop data taking, but still store data taken so far
		  if(tmpScans[ife]->getSourceScanFlag() && !tmpScans[ife]->getLoopActive(0)) 
		    getSTPixModuleGroup()->m_abortScan = false;
		}
		
		// read buffer every now and then - only ROD
		m_checkScanCount++;
		int freq=10;
		if(m_checkScanCount>100) freq=100;   //check less often for successful scans
		if(m_checkScanCount>1000) freq=1000; //check even less often for long scans
		if((m_checkScanCount%freq)==0 && pc==0){
		  std::string msg = "Info while scanning (#";
		  std::stringstream a;
		  int ncheck = m_checkScanCount/freq;
		  if(freq==100) ncheck += 9;
		  if(freq==1000) ncheck += 18;
		  a << ncheck;
		  msg += a.str();
		  msg += "):";
		  readRodBuff(msg);

		
		}
	      } while ((pc->runStatus()==1 && nAbortWait<100));
	      // end controller scan loop
	      
	      if(nAbortWait>=100){
		// stopScan  didn't work, notify user
		setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
		m_app->postEvent(getSTPixModuleGroup(), cse);
		ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::scan(): clean stop of scan after abort signal didn't succeed");
		m_app->postEvent(getSTPixModuleGroup(), tee);
		
	      }
	      
	      if (cfg.getRunType() == PixScan::NORMAL_SCAN && cfg.getDspMaskStaging() && histoOnCtrl){
		msleep(500);	
	      }
	      
	      getSTPixModuleGroup()->scanTerminate(tmpScans[ife]);
	      tmpScans[ife]->next(0);
	    }
	    
	    getSTPixModuleGroup()->ScanStatus=5;
	    if(!getSTPixModuleGroup()->doAbortScan() ){
	      // tmp FEbyFE
	      getSTPixModuleGroup()->scanLoopEnd(0, tmpScans[ife]);
	    }
	    // tmp FEbyFE
	    if(scanOpts.runFEbyFE){
	      getSTPixModuleGroup()->ScanStatus=0; 
	      for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
		if((*mi)->pixFE(ife)!=0) (*mi)->pixFE(ife)->restoreConfig("ResetConfig");
	      }
	    }
	  }
	  if(scanOpts.runFEbyFE){
	    getSTPixModuleGroup()->mergeFeHistos(tmpScans);
	    tmpScans[0]=0;
	    for(int i=1;i<16;i++){
	      delete (tmpScans[i]);
	      tmpScans[i]=0;
	    }
	  }
	  // end tmp FEbyFE
	  if(!getSTPixModuleGroup()->doAbortScan() ){
	    cfg.next(1);
	  }
	  getSTPixModuleGroup()->ScanStatus=0; 
	}
	
	getSTPixModuleGroup()->ScanStatus=6;
	if(!getSTPixModuleGroup()->doAbortScan() ){
	  getSTPixModuleGroup()->scanLoopEnd(1, &cfg);
	  cfg.next(2);	  
	}
	getSTPixModuleGroup()->ScanStatus=0;
      }
      
      getSTPixModuleGroup()->ScanStatus=7;
      
      if(!getSTPixModuleGroup()->doAbortScan() ){
	getSTPixModuleGroup()->scanLoopEnd(2, &cfg);  
	if(!onlyDcsScanned) getSTPixModuleGroup()->terminateScan(&cfg);
      }
      // tmp FEbyFE
      if(scanOpts.runFEbyFE && !onlyDcsScanned){ // restore org config
	getSTPixModuleGroup()->selectFe(-1,-1);
	getSTPixModuleGroup()->downloadConfig();
      }
      // end tmp FEbyFE
      if(getSTPixModuleGroup()->getPixCtrlStatus()!=tOK){
	setCtrlStatusEvent* cse = new setCtrlStatusEvent(tOK,getSTPixModuleGroup());
	m_app->postEvent(getSTPixModuleGroup(), cse);
      } else if (!onlyDcsScanned && !getSTPixModuleGroup()->doAbortScan()){
	ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::scan(): PixController has not scanned - should not have happened");
	m_app->postEvent(getSTPixModuleGroup(), tee);
      }
      
      // if requested, write to file - do not so when scan was aborted
      if(scanOpts.writeToFile && !scanOpts.fileName.empty() && !getSTPixModuleGroup()->doAbortScan()){
	getSTPixModuleGroup()->ScanStatus=3; 
	// write histograms to file
	// NB: preferrably do this also within this thread, but for windows this doesn't work somehow
#ifdef WIN32
	// writing called externally to be outside of thread, wait for it to finish
	while(getSTPixModuleGroup()->ScanStatus==3) PixLib::sleep(100);
#else
	getSTPixModuleGroup()->processPixScanHistos(scanOpts);
#endif
      }
      
      if(getSTPixModuleGroup()->doAbortScan()){
	// set status back to OK, since lines above were not executed
	setCtrlStatusEvent* cse4 = new setCtrlStatusEvent(tOK, getSTPixModuleGroup());
	m_app->postEvent(getSTPixModuleGroup(), cse4);
      }
      
    }
  }
  catch(PixDBException &dbexc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt, exmsg;
    dbexc.what(exmsg);
    txt << "STPixModuleGroup::CtrlThread::scan() : PixDB-exception " <<exmsg.str() << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixScanExc &psExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixScan-exception level " << psExc.dumpLevel() << 
      " " << psExc << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (USBPixControllerExc &upcExc){
    bool setProblem = (upcExc.getErrorLevel()==PixControllerExc::WARNING || upcExc.getErrorLevel()==PixControllerExc::INFO);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(setProblem?tOK:tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    std::stringstream msg;
    upcExc.dump(msg);
    txt << "STPixModuleGroup::CtrlThread::scan() : USBPixController-exception (grp. " << getSTPixModuleGroup()->getName() << "): " 
	<< msg.str() << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
#ifndef NOTDAQ
  catch (RodPixControllerExc &rpcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    std::stringstream msg;
    rpcExc.dump(msg);
    txt << "STPixModuleGroup::CtrlThread::scan() : RodPixController-exception (grp. " << getSTPixModuleGroup()->getName() << "): " 
	<< msg.str() << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
#endif
  catch (PixControllerExc &pcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    std::stringstream msg;
    pcExc.dump(msg);
    txt << "STPixModuleGroup::CtrlThread::scan() : PixController-exception (grp. " << getSTPixModuleGroup()->getName() << "): " 
	<< msg.str() << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixModuleExc &mExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixModule-exception level " << mExc.dumpLevel() << 
      " " << mExc << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixMccExc &mcExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixMcc-exception level " << mcExc.dumpLevel() << 
      " " << mcExc.dumpType() << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixFeExc &fExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : PixFe-exception level " << fExc.dumpLevel() << 
      " " << fExc.dumpType() << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (ConfigExc & cfExc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : Config-exception " << cfExc << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch (VmeException & vexc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : VME-exception " << vexc << "\n";
    ThreadErrorEvent* tle = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }
  catch(BaseException & exc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt<<"STPixModuleGroup::CtrlThread::scan() : "<<exc<<"\n";
    ThreadErrorEvent* tee = new ThreadErrorEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tee);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus=1;
    setThreadTag(MTundecided);
    return;
  }
  catch(std::exception& sexc){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    std::stringstream txt;
    txt << "STPixModuleGroup::CtrlThread::scan() : Std-lib-exception while scanning " << sexc.what() << "\n";
    ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
    m_app->postEvent(getSTPixModuleGroup(), tle);
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus = 1;
    setThreadTag(MTundecided);
    return;
  }catch(...){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem, getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::scan() :Unexpected exception. \n");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    
    getSTPixModuleGroup()->processing = false;
    getSTPixModuleGroup()->ScanStatus=1;
    setThreadTag(MTundecided);
    return;
  }
  
  getSTPixModuleGroup()->ScanStatus=1;
  getSTPixModuleGroup()->processing = false;
  setThreadTag(MTundecided);
  return;
}

void STPixModuleGroup::CtrlThread::initrods()
{
  getSTPixModuleGroup()->processing = true;

  try{
    getSTPixModuleGroup()->initHW();
    getSTPixModuleGroup()->downloadConfig();

    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tOK,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);

#ifndef NOTDAQ
    PixBoc *pb = getSTPixModuleGroup()->getPixBoc();    
    if(pb!=0 && getSTPixModuleGroup()->getPixBocStatus()!=tOK
       && getSTPixModuleGroup()->getPixBocStatus()!=tbusy){
      setBocStatusEvent* bse = new setBocStatusEvent(tOK,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), bse);
    }
#endif
    
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
    
#ifndef NOTDAQ
  }catch (RodPixControllerExc & rpcexc){
    std::stringstream msg;
    rpcexc.dump(msg);
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initrods() : RodPixController-exception (grp. " + 
						 getSTPixModuleGroup()->getName() + "): " + msg.str());
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);

    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
#endif
  }catch (USBPixControllerExc & upcexc){
    std::stringstream msg;
    upcexc.dump(msg);
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initrods() : USBPixController-exception (grp. " + 
						 getSTPixModuleGroup()->getName() + "): "  + msg.str());
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);

    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  catch (PixModuleExc &mExc){
    std::stringstream txt;
    txt << mExc << " (level " << mExc.dumpLevel() << ")";
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initrods() : PixModule exception (" + txt.str()+ ") from grp. "
						 + getSTPixModuleGroup()->getName() + " during init. of PixController");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);

    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }catch (BaseException & exc){
    std::stringstream msg;
    msg << exc;
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initrods() : Base exception (" + msg.str()+ ") from grp. "
						 + getSTPixModuleGroup()->getName() + " during init. of PixController");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);

    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  catch(std::exception& sexc){
    std::stringstream txt;
    txt << sexc.what();
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initrods() : std.-lib. exception (" + txt.str() + ") from grp. "
						 + getSTPixModuleGroup()->getName() + ".");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);

    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  catch (...){
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initrods : Unexpected exception while performing PixController init (grp. " +  
						 getSTPixModuleGroup()->getName() + ")");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }  
  return;
} 

void STPixModuleGroup::CtrlThread::trigger()
{  
  getSTPixModuleGroup()->processing = true;

  if (getSTPixModuleGroup()->getPixController()!=0){
    try{
      getSTPixModuleGroup()->sendCommand(PMG_CMD_TRIGGER, 0);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
    } catch (BaseException & exc){
      std::stringstream msg;
      msg << exc;
      ThreadErrorEvent* tee = new ThreadErrorEvent(msg.str());
      m_app->postEvent(getSTPixModuleGroup(), tee);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
    catch (...){
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup:CtrlThread:trigger() : Unexpected exception while triggering module in group " + 
						   getSTPixModuleGroup()->getName());
      m_app->postEvent(getSTPixModuleGroup(), tee);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
  } else{
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup:CtrlThread:trigger() : Found no valid PixController in PixModuleGroup for group " + 
						 getSTPixModuleGroup()-> getName() + "\n");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  return;
}

void STPixModuleGroup::CtrlThread::getSrvRec(){
  getSTPixModuleGroup()->processing = true;

  if(getSTPixModuleGroup()->getPixCtrlStatus()==tOK) {
    try{
      std::string srvrec;
      getSTPixModuleGroup()->getPixController()->getServiceRecords(srvrec, (getSTPixModuleGroup()->m_srvCounts));
      ThreadBufferEvent* tlb = new ThreadBufferEvent("Service record for "+getSTPixModuleGroup()->getName()+":\n"+srvrec, false);
      m_app->postEvent(getSTPixModuleGroup(), tlb);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
    } catch (BaseException & exc){
      std::stringstream msg;
      msg << exc;
      ThreadErrorEvent* tee = new ThreadErrorEvent(msg.str());
      m_app->postEvent(getSTPixModuleGroup(), tee);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
    catch (...){
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup:CtrlThread:trigger() : Unexpected exception while triggering module in group " + getSTPixModuleGroup()->getName());
      m_app->postEvent(getSTPixModuleGroup(), tee);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
  } else{ 
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup:CtrlThread:trigger() : PixModuleGroup " +  getSTPixModuleGroup()->getName() + " has an uninitialised PixController\n");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }

  return;
}

void STPixModuleGroup::CtrlThread::config()  
{  
  if(m_tagOpt.size()!=3){
    std::stringstream a;
    a << m_tagOpt.size();
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::config() : need three config options, but got "+
						 a.str()+ " (grp. "+ getSTPixModuleGroup()->getName() + ")");
    m_app->postEvent(getSTPixModuleGroup(), tee);      
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tOK,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }

  getSTPixModuleGroup()->processing = true;
  // set module mask according to enabled flags
  bool noneCfg=true;
  for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
    Config &conf = (*mi)->config();
    if(conf["general"]["Active"].name()!="__TrashConfObj__"){
      if(((ConfList&)conf["general"]["Active"]).sValue()=="TRUE")
	noneCfg = false;
    }
  }
  if(noneCfg){ // no active module, nothing to do
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  // check if ROD has been initialised, and do so if not
  if(getSTPixModuleGroup()->getPixCtrlStatus()==tunknown){
    initrods();
    getSTPixModuleGroup()->processing = true;
  }

  if(getSTPixModuleGroup()->getPixCtrlStatus()==tOK){
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tbusy,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    // Module configuration
    try{
      // load regular pattern if requested
      if(m_tagOpt[1]!=ChipTest::CURRENTCFG && m_tagOpt[1]!=ChipTest::READONLY){
	for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
	  // store current cfg. for restoring it later
	  (*mi)->storeConfig("PreFixpatConfig");
	  for(PixModule::feIterator fei = (*mi)->feBegin(); fei!=(*mi)->feEnd(); fei++){
	    switch(m_tagOpt[0]){
	    case 1:
	      getSTPixModuleGroup()->setGlobReg(m_tagOpt[1], *fei);
	      break;
	    case 2:
	      getSTPixModuleGroup()->setPixlReg(m_tagOpt[1], *fei);
	      break;
	    default:
	      // do nothing
	      break;
	    }
	  }
	}
      }
      switch(m_tagOpt[0]){
      case 0:
      default:
	getSTPixModuleGroup()->downloadConfig();
	getSTPixModuleGroup()->getPixController()->setConfigurationMode();
	getSTPixModuleGroup()->getPixController()->sendModuleConfig(0);
	break;
      case 1:
	getSTPixModuleGroup()->downloadConfig();
	getSTPixModuleGroup()->getPixController()->setConfigurationMode();
	getSTPixModuleGroup()->getPixController()->sendGlobal(0);
	break;
      case 2:
	getSTPixModuleGroup()->downloadConfig();
	getSTPixModuleGroup()->getPixController()->setConfigurationMode();
	if(m_tagOpt[2]>=0 && m_tagOpt[2]<40 && m_tagStrg!="ALL")
	  getSTPixModuleGroup()->getPixController()->sendPixel(0, m_tagStrg, m_tagOpt[2]);
	else if(m_tagStrg!="ALL")
	  getSTPixModuleGroup()->getPixController()->sendPixel(0, m_tagStrg, -1);
	else{
	  if(m_tagOpt[2]>=0 && m_tagOpt[2]<40){
	    PixFeI4A fe(0,0,"DummyFE",0); // pixel registers are identical in I4A and I4B, so doesn't matter which is used
	    Config &feconf = fe.config();
	    ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];
	    ConfGroup &maskgrp = feconf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
	    for(int i=0;i<(int)maskgrp.size();i++){
	      std::string prname = maskgrp[i].name();
	      prname.erase(0,std::string("PixelRegister_").length());
	      getSTPixModuleGroup()->getPixController()->sendPixel(0, prname, m_tagOpt[2]);
	    }
	    // trim registers: >1 bit, must add bit index to name
	    int maxval, nmask=maskgrp.size()+1;
	    for(int i=0;i<(int)trimgrp.size();i++){
	      std::string prname = trimgrp[i].name();
	      prname.erase(0,std::string("Trim_").length());
	      if(!fe.getTrimMax(prname,maxval)) maxval=0;
	      maxval = (int)TMath::Log2((double)(maxval+1));
	      for(int j=0;j<maxval;j++){
		std::stringstream a;
		a << j;
		getSTPixModuleGroup()->getPixController()->sendPixel(0, prname+a.str(), m_tagOpt[2]);
		nmask++;
	      }
	    }
	  }else
	    getSTPixModuleGroup()->getPixController()->sendPixel(0);
	}
	break;
      case 3:
	getSTPixModuleGroup()->getPixController()->setConfigurationMode();
	getSTPixModuleGroup()->getPixController()->setFEConfigurationMode();
	break;
      case 4:
	getSTPixModuleGroup()->getPixController()->setRunMode();
	getSTPixModuleGroup()->getPixController()->setFERunMode();
	break;
      }
      if(m_tagOpt[1]!=ChipTest::CURRENTCFG && m_tagOpt[1]!=ChipTest::READONLY){
	for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
	  // restore old config
	  (*mi)->restoreConfig("PreFixpatConfig");
	  (*mi)->deleteConfig("PreFixpatConfig");
	}
      }
      cse = new setCtrlStatusEvent(tOK,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), cse);
#ifndef NOTDAQ
    }catch (RodPixControllerExc & rpcexc){
      std::stringstream msg;
      rpcexc.dump(msg);
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::config() : " + msg.str()+ " (grp. "
						   + getSTPixModuleGroup()->getName() + ")");
      m_app->postEvent(getSTPixModuleGroup(), tee);
      // getSTPixModuleGroup()->setPixCtrlStatus(tproblem);
      setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), cse);
   
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
#endif
    }catch (USBPixControllerExc & upcexc){
      std::stringstream msg;
      upcexc.dump(msg);
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::config() : " + msg.str()+ " (grp. "
						   + getSTPixModuleGroup()->getName() + ")");
      m_app->postEvent(getSTPixModuleGroup(), tee);
      // getSTPixModuleGroup()->setPixCtrlStatus(tproblem);
      setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), cse);
      
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }catch (BaseException & exc){
      std::stringstream msg;
      msg << exc;
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::config() : " + msg.str()+ " (grp. "
	       + getSTPixModuleGroup()->getName() + ")");
      m_app->postEvent(getSTPixModuleGroup(), tee);
      // getSTPixModuleGroup()->setPixCtrlStatus(tproblem);
      setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), cse);

      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
    catch (...){
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::config() : Unexpected exception while performing module init (grp. "
	       + getSTPixModuleGroup()->getName() + ")");
      m_app->postEvent(getSTPixModuleGroup(), tee);
      // getSTPixModuleGroup()->setPixCtrlStatus(tproblem);
      setCtrlStatusEvent* cse = new setCtrlStatusEvent(tproblem,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), cse);

      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
  }
  getSTPixModuleGroup()->processing = false;
  setThreadTag(MTundecided);
  return;
}

void STPixModuleGroup::CtrlThread::resetmods()  
{ 
  if(m_tagOpt.size()!=1){
    std::stringstream a;
    a << m_tagOpt.size();
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetmods() : need one config option, but got "+
						 a.str()+ " (grp. "+ getSTPixModuleGroup()->getName() + ")");
    m_app->postEvent(getSTPixModuleGroup(), tee);      
    setCtrlStatusEvent* cse = new setCtrlStatusEvent(tOK,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), cse);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }

  getSTPixModuleGroup()->processing = true;
  
  // Make sure we have PixController ready to do some work
  if(getSTPixModuleGroup()->getPixCtrlStatus()==tOK) {
    try{
      getSTPixModuleGroup()->sendCommand((PixLib::PixModuleGroup::CommandType)m_tagOpt[0], 0);
    } catch (BaseException & exc){
      std::stringstream msg;
      msg << exc;
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetmods() : " + msg.str()+ "(group "+  getSTPixModuleGroup()->getName()+")");
      m_app->postEvent(getSTPixModuleGroup(), tee);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
    catch (...){
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetmods() : Unexpected exception while resetting module in group "+  
						   getSTPixModuleGroup()->getName());
      m_app->postEvent(getSTPixModuleGroup(), tee);
      getSTPixModuleGroup()->processing = false;
      setThreadTag(MTundecided);
      return;
    }
  } else{
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetmods() : PixModuleGroup " + getSTPixModuleGroup()->getName() + 
						 " has an uninitialised PixController\n");
    m_app->postEvent(getSTPixModuleGroup(), tee);
  }
  
  getSTPixModuleGroup()->processing = false;
  setThreadTag(MTundecided);
  return;
}

void STPixModuleGroup::CtrlThread::resetrods()  
{ 
  getSTPixModuleGroup()->processing = true;
#ifndef NOTDAQ
  
//   RodPixController *rod = dynamic_cast<RodPixController *>( getSTPixModuleGroup()->getPixController() );
//   // Make sure we have a valid PodPixController
//   if (rod && rod->rodMod()!=0 && getSTPixModuleGroup()->getPixCtrlStatus()!=tbusy) {
//     try{
//       rod->rodMod()->reset();
//     }catch (BaseException & exc){
//       std::stringstream msg;
//       msg << exc;
//       ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetrods() : "+msg.str());
//       m_app->postEvent(getSTPixModuleGroup(), tee);
//       //  getSTPixModuleGroup()->setPixCtrlStatus(tblocked);
//        setCtrlStatusEvent* cse = new setCtrlStatusEvent(tblocked,getSTPixModuleGroup());
//        m_app->postEvent(getSTPixModuleGroup(), cse);
      
//       getSTPixModuleGroup()->processing = false;
//       setThreadTag(MTundecided);
//       return;					   
//     }
//     catch (...){
//       ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetrods() : Unexpected exception while performing ROD reset");
//       m_app->postEvent(getSTPixModuleGroup(), tee);
//       getSTPixModuleGroup()->processing = false;
//       //getSTPixModuleGroup()->setPixCtrlStatus(tblocked);
//       setCtrlStatusEvent* cse = new setCtrlStatusEvent(tblocked,getSTPixModuleGroup());
//       m_app->postEvent(getSTPixModuleGroup(), cse);
//       setThreadTag(MTundecided);
//       return;
//     }
//     // getSTPixModuleGroup()->setPixCtrlStatus(tblocked);
//     setCtrlStatusEvent* cse = new setCtrlStatusEvent(tblocked,getSTPixModuleGroup());
//     m_app->postEvent(getSTPixModuleGroup(), cse);
// } else{
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetrods() : Found no valid ROD Controller in PixModuleGroup " + getSTPixModuleGroup()->getRodName() + "\n");
    m_app->postEvent(getSTPixModuleGroup(), tee);
//   }
  
#else
  ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::resetrods() : Code compiled w/o ROD functionality");
  m_app->postEvent(getSTPixModuleGroup(), tee);
#endif

  getSTPixModuleGroup()->processing = false;
  setThreadTag(MTundecided);
  return;
}


void STPixModuleGroup::CtrlThread::initbocs()  
{
  getSTPixModuleGroup()->processing = true;
#ifndef NOTDAQ
  try{
    PixBoc *pb = getSTPixModuleGroup()->getPixBoc();
    if(pb!=0 && getSTPixModuleGroup()->getPixBocStatus()!=tbusy){
      pb->BocConfigure();
      setBocStatusEvent* bse = new setBocStatusEvent(tOK,getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), bse);
    }
  }catch (BaseException & exc){
    std::stringstream msg;
    msg << exc;
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initbocs() : "+ msg.str());
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setBocStatusEvent* bse = new setBocStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), bse);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  catch (...){
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::initbocs() : Unexpected exception while performing ROD reset");
    m_app->postEvent(getSTPixModuleGroup(), tee);
    setBocStatusEvent* bse = new setBocStatusEvent(tproblem,getSTPixModuleGroup());
    m_app->postEvent(getSTPixModuleGroup(), bse);
    getSTPixModuleGroup()->processing = false;
    setThreadTag(MTundecided);
    return;
  }
  
#else
  ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::rinitbocs() : Code compiled w/o ROD functionality");
  m_app->postEvent(getSTPixModuleGroup(), tee);
#endif

  getSTPixModuleGroup()->processing = false;
  setThreadTag(MTundecided);
  return;
}

void STPixModuleGroup::CtrlThread::setmcc()
{  
//  int OMval = 0; 
//  if(m_tagOpt[0]) OMval = 2;
  getSTPixModuleGroup()->processing = true;
  
//   RodPixController *rod = dynamic_cast<RodPixController *>( getSTPixModuleGroup()->getPixController() );
//   // Make sure we have a valid RodPixController
//   if (rod){
//     if(getSTPixModuleGroup()->getPixCtrlStatus()==tOK) {
//       try{
// 	// Set Mode
// 	rod->setCalibrationDebugMode();
// 	// Loop over Modules
// 	for( PixModuleGroup::moduleIterator module = getSTPixModuleGroup()->modBegin(); 
// 	     module !=  getSTPixModuleGroup()->modEnd(); module++) {
// 	  // set MCC CSR OM to 0 (40Mb/s mode) or 2 (80Mb/s mode) and OPAT to 1 (1010... mode)
// 	  if((*module)->pixMCC()!=0){
// 	    (*module)->pixMCC()->setRegister("CSR_OutputMode",    OMval); // data speed
// 	    (*module)->pixMCC()->setRegister("CSR_OutputPattern", 1);     // 1010...
// 	  }
// 	}
//       } catch (BaseException & exc){
// 	std::stringstream msg;
// 	msg << exc;
// 	ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::setmcc() : " + msg.str());
// 	m_app->postEvent(getSTPixModuleGroup(), tee);
// 	getSTPixModuleGroup()->processing = false;
// 	setThreadTag(MTundecided);
// 	return;
//       }
//       catch (...){
// 	ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::setmcc() : Unexpected exception while resetting module in ROD " + getSTPixModuleGroup()->getName());
// 	m_app->postEvent(getSTPixModuleGroup(), tee);
// 	getSTPixModuleGroup()->processing = false;
// 	setThreadTag(MTundecided);
// 	return;
//       }
//     } else{
//       ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::setmcc() : PixModuleGroup " + getSTPixModuleGroup()->getName() + " has an uninitialised ROD\n");
//       m_app->postEvent(getSTPixModuleGroup(), tee);
//     }
//   } else{
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::CtrlThread::setmcc() : Found no valid ROD Controller in PixModuleGroup for Module " + getSTPixModuleGroup()->getRodName() + "\n");
    m_app->postEvent(getSTPixModuleGroup(), tee);
//   }
  
  getSTPixModuleGroup()->processing = false;
  setThreadTag(MTundecided);
  return;
}

void STPixModuleGroup::CtrlThread::runLinkTest()
{
  getSTPixModuleGroup()->processing = true;
  getSTPixModuleGroup()->runLinkTest(0,this); // PixModule ptr.==NULL -> run over all modules
  getSTPixModuleGroup()->processing = false;
}
void STPixModuleGroup::CtrlThread::runChipTest()
{
  getSTPixModuleGroup()->processing = true;
  getSTPixModuleGroup()->runChipTest(m_ct, m_resetMods, 0); // PixModule ptr.==NULL -> run over all modules
  getSTPixModuleGroup()->processing = false;
}
void STPixModuleGroup::CtrlThread::getDcsReadings(PixScan *scn){
  int rtype = (int)scn->getDcsMode();
  std::string chanName = scn->getDcsChan();
  int cmdId=0;
  std::vector<int> myCmdIds, modIds;
  // get next unused command ID
  for(std::map<int,std::pair<int,double> >::iterator it=getSTPixModuleGroup()->m_dcsRStates.begin(); 
      it!=getSTPixModuleGroup()->m_dcsRStates.end(); it++)
    if(it->first>cmdId) cmdId = it->first;
  cmdId++;
  // send DCS commands
  for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
    if(((*mi)->config())["general"].name()!="__TrashConfGroup__" &&
       ((*mi)->config())["general"]["Active"].name()!="__TrashConfObj__" &&
       (((ConfList&)((*mi)->config())["general"]["Active"]).sValue()=="TRUE")){  // process only active modules
      myCmdIds.push_back(cmdId);
      modIds.push_back((*mi)->moduleId());
      // setting DCS status
      getSTPixModuleGroup()->m_dcsRStates[cmdId] = std::make_pair(3,0.);
      // send actual command
      std::string mname = (*mi)->moduleName();
      getDecNameCore(mname);
      getDcsEvent* gdcs = new getDcsEvent(chanName+(scn->getAddModNameToDcs()?("_"+mname):""), rtype, cmdId,
					  getSTPixModuleGroup());
      m_app->postEvent(getSTPixModuleGroup(), gdcs);
      cmdId++;
    }
  }
  // wait until commands have been processed
  bool dcsProc=true;
  int nDcsIter=0;
  while(dcsProc && nDcsIter<100000){
    dcsProc = false;
    for(int ic=0;ic<(int) myCmdIds.size(); ic++){
      if(getSTPixModuleGroup()->m_dcsRStates[myCmdIds[ic]].first==3) dcsProc=true;
      else{
	std::stringstream a;
	a << modIds[ic];
	// temporary, need some way to store data into file later on
	std::string mname = getSTPixModuleGroup()->module(modIds[ic])->moduleName();
	getDecNameCore(mname);
	(getSTPixModuleGroup()->m_dcsRead[modIds[ic]])->push_back(getSTPixModuleGroup()->m_dcsRStates[myCmdIds[ic]].second);
      }
    }
    if(dcsProc) PixLib::sleep(10);
    nDcsIter++;
  }
  // clean up, error processing
  for(int ic = 0; ic < (int)myCmdIds.size(); ic++){
    if(getSTPixModuleGroup()->m_dcsRStates[myCmdIds[ic]].first==2) {
      std::stringstream a;
      a << modIds[ic];
      ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::getDcsReadings(): reading of DCS channel "+
					       chanName+" on module ID "+a.str()+
					       " of group "+getSTPixModuleGroup()->getName()+" failed.\n" );
      m_app->postEvent(getSTPixModuleGroup(), tle);
    }
    getSTPixModuleGroup()->m_dcsRStates.erase(myCmdIds[ic]);
  }
}
void STPixModuleGroup::CtrlThread::setDcsScanVal(int loop, PixScan *scn)
{
	//error handling if parameter strings are set
	if(scn->getLoopParam(loop) == PixLib::PixScan::DCS_VOLTAGE && scn->getScanDcsChan().compare("") == 0){
		ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): DCS_VOLTAGES is scanned but DCS channel name not specified.\n" );
		m_app->postEvent(getSTPixModuleGroup(), tle);
		return;
	}
	else if(scn->getLoopParam(loop) == PixLib::PixScan::DCS_PAR1 && (scn->getDcsScanPar1ChannelName().compare("")==0 || scn->getDcsScanPar1Name().compare("") == 0) ){
		ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): DCS_PAR1 is scanned but no DCS channel name or parameter name specified.\n" );
		m_app->postEvent(getSTPixModuleGroup(), tle);
		return;
	}
	else if(scn->getLoopParam(loop) == PixLib::PixScan::DCS_PAR2 && (scn->getDcsScanPar2ChannelName().compare("")==0 || scn->getDcsScanPar2Name().compare("") == 0) ){
		ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): DCS_PAR2 is scanned but no DCS channel name or parameter name specified.\n" );
		m_app->postEvent(getSTPixModuleGroup(), tle);
		return;
	}
	else if(scn->getLoopParam(loop) == PixLib::PixScan::DCS_PAR3 && (scn->getDcsScanPar3ChannelName().compare("")==0 || scn->getDcsScanPar3Name().compare("") == 0) ){
		ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): DCS_PAR3 is scanned but no DCS channel name or parameter name specified.\n" );
		m_app->postEvent(getSTPixModuleGroup(), tle);
		return;
	}
	else if(scn->getLoopParam(loop) != PixLib::PixScan::DCS_VOLTAGE && scn->getLoopParam(loop) != PixLib::PixScan::DCS_PAR1 && scn->getLoopParam(loop) != PixLib::PixScan::DCS_PAR2 && scn->getLoopParam(loop) != PixLib::PixScan::DCS_PAR3){
		ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): PixLib::PixScan is unknown. Dont know what to do. Please implement\n" );
		m_app->postEvent(getSTPixModuleGroup(), tle);
		return;
	}

	std::string chanName = "";
	int cmdId=0;
	std::vector<int> myCmdIds, modIds;
	// get next unused command ID
	for(std::map<int,int>::iterator it=getSTPixModuleGroup()->m_dcsStates.begin(); it!=getSTPixModuleGroup()->m_dcsStates.end(); it++)
		if(it->first>cmdId) cmdId = it->first;
	cmdId++;
	// send DCS commands
	// debug printout
	bool dcsProc=true;
	int nDcsIter=0;
	for(moduleIterator mi = getSTPixModuleGroup()->modBegin(); mi != getSTPixModuleGroup()->modEnd(); mi++){
		if(((*mi)->config())["general"].name()!="__TrashConfGroup__" &&((*mi)->config())["general"]["Active"].name()!="__TrashConfObj__" &&(((ConfList&)((*mi)->config())["general"]["Active"]).sValue()=="TRUE")){  // process only active modules
			myCmdIds.push_back(cmdId);
		modIds.push_back((*mi)->moduleId());
		std::string mname = (*mi)->moduleName();
		getDecNameCore(mname);
		// setting DCS status
		getSTPixModuleGroup()->m_dcsStates[cmdId] = 3;
		// send actual command
		setDcsEvent* sdcs;
		switch(scn->getLoopParam(loop)){
			case PixLib::PixScan::DCS_VOLTAGE:
				sdcs = new setDcsEvent(scn->getScanDcsChan()+(scn->getAddModNameToDcs()?("_"+mname):""), "voltage", (scn->getLoopVarValues(loop))[scn->scanIndex(loop)], cmdId, getSTPixModuleGroup());
				chanName = scn->getScanDcsChan();
				break;
			case PixLib::PixScan::DCS_PAR1:
				sdcs = new setDcsEvent(scn->getDcsScanPar1ChannelName()+(scn->getAddModNameToDcs()?("_"+mname):""), scn->getDcsScanPar1Name(), (scn->getLoopVarValues(loop))[scn->scanIndex(loop)], cmdId, getSTPixModuleGroup());
				chanName = scn->getDcsScanPar1ChannelName();
				break;
			case PixLib::PixScan::DCS_PAR2:
				sdcs = new setDcsEvent(scn->getDcsScanPar2ChannelName()+(scn->getAddModNameToDcs()?("_"+mname):""), scn->getDcsScanPar2Name(), (scn->getLoopVarValues(loop))[scn->scanIndex(loop)], cmdId, getSTPixModuleGroup());
				chanName = scn->getDcsScanPar2ChannelName();
				break;
			case PixLib::PixScan::DCS_PAR3:
				sdcs = new setDcsEvent(scn->getDcsScanPar3ChannelName()+(scn->getAddModNameToDcs()?("_"+mname):""), scn->getDcsScanPar3Name(), (scn->getLoopVarValues(loop))[scn->scanIndex(loop)], cmdId, getSTPixModuleGroup());
				chanName = scn->getDcsScanPar3ChannelName();
				break;
			default:
				ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): PixLib::PixScan is unknown. Dont know what to do. Please implement\n" );
				m_app->postEvent(getSTPixModuleGroup(), tle);
				return;
		}
		m_app->postEvent(getSTPixModuleGroup(), sdcs);
		cmdId++;
		}
	}

	// wait until commands have been processed
	dcsProc=true;
	nDcsIter=0;
	while(dcsProc && nDcsIter<10000){
		dcsProc = false;
		for(std::vector<int>::iterator itv = myCmdIds.begin(); itv!=myCmdIds.end(); itv++)
		if(getSTPixModuleGroup()->m_dcsStates[(*itv)]==3)
			dcsProc=true;
		if(dcsProc)
			PixLib::sleep(10);
		nDcsIter++;
	}
	// clean up, error processing
	for(int ic=0;ic<(int) myCmdIds.size(); ic++){
		//std::cout << "STPixModuleGroup: Setting DCS channel; cmdId " << (*itv) << " returns state " << getSTPixModuleGroup()->m_dcsStates[(*itv)] << std::endl;
		if(getSTPixModuleGroup()->m_dcsStates[myCmdIds[ic]]==2){
			std::stringstream a;
			a << modIds[ic];
			ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::CtrlThread::setDcsScanVal(): setting of DCS channel "+chanName+" on module ID "+a.str()+" of group "+getSTPixModuleGroup()->getName()+" failed.\n" );
			m_app->postEvent(getSTPixModuleGroup(), tle);
		}
		getSTPixModuleGroup()->m_dcsStates.erase(myCmdIds[ic]);
	}
}

int STPixModuleGroup::getNSteps(int i)
{
  if (i>-1 && i<3)
    return  m_nSteps[i];
  else
    return -1;
}


bool STPixModuleGroup::initrods()
{
  processing = true;

  try{
    initHW();
    downloadConfig();
    setPixCtrlStatus(tOK);
#ifndef NOTDAQ
    PixBoc *pb = getPixBoc();    
    if(pb!=0 && getPixBocStatus()!=tOK && getPixBocStatus()!=tbusy)
      setPixBocStatus(tOK);
#endif

    processing = false;
    return true;
    
#ifndef NOTDAQ
  }catch (RodPixControllerExc & rpcexc){
    std::stringstream msg;
    rpcexc.dump(msg);
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::initrods() : RodPixController-exception (grp. " + 
						 getName() + "): " + msg.str());
    m_app->postEvent(this, tee);
    setPixCtrlStatus(tproblem);
    processing = false;
    return false;
#endif
  }catch (USBPixControllerExc & upcexc){
    std::stringstream msg;
    upcexc.dump(msg);
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::initrods() : USBPixController-exception (grp. " + 
						 getName() + "): "  + msg.str());
    m_app->postEvent(this, tee);
    setPixCtrlStatus(tproblem);
    processing = false;
    return false;
  }
  catch (PixModuleExc &mExc){
    std::stringstream txt;
    txt << mExc << " (level " << mExc.dumpLevel() << ")";
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::initrods() : PixModule exception (" + txt.str()+ ") from grp. "
						 + getName() + " during init. of PixController");
    m_app->postEvent(this, tee);
    setPixCtrlStatus(tproblem);
    processing = false;
    return false;
  }
  catch (BaseException & exc){
    std::stringstream msg;
    msg << exc;
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::initrods() : " + msg.str()+ " (grp. "
						 + getName() + ")");
    m_app->postEvent(this, tee);
    setPixCtrlStatus(tproblem);
    processing = false;
    return false;
  }
  catch(std::exception& sexc){
    std::stringstream txt;
    txt << sexc.what();
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::initrods() : std.-lib. exception (" + txt.str() + ") from grp. "
						 + getName() + ".");
    m_app->postEvent(this, tee);
    setPixCtrlStatus(tproblem);
    processing = false;
    return false;
  }catch (...){
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::initrods : Unexpected exception while performing PixController reinit (grp. " +  
						 getName() + ")");
    m_app->postEvent(this, tee);
    setPixCtrlStatus(tproblem);
    processing = false;
    return false;
  }  
  // should never get here
  return false;
} 

void STPixModuleGroup::runLinkTest(PixModule *in_mod, CtrlThread *thread)
{
// currently not supported
  return;

  if(getPixCtrlStatus()!=tOK) return;

  m_abortScan = false;

  // define "scan" parameters, retrieving MCC pattern from links
//   m_PixScan->preset(PixScan::DIGITAL_TEST);
//   m_PixScan->setRepetitions(10); 
//   m_PixScan->setHistogramFilled(PixScan::OCCUPANCY,false);
//   m_PixScan->setHistogramKept(PixScan::OCCUPANCY,false);
//   m_PixScan->setHistogramFilled(PixScan::RAW_DATA_0,true);
//   m_PixScan->setHistogramKept(PixScan::RAW_DATA_0,false);
//   m_PixScan->setHistogramFilled(PixScan::RAW_DATA_1,true);
//   m_PixScan->setHistogramKept(PixScan::RAW_DATA_1,true);
//   m_PixScan->setHistogramFilled(PixScan::RAW_DATA_DIFF_1,true);
//   m_PixScan->setHistogramKept(PixScan::RAW_DATA_DIFF_1,true);
//   m_PixScan->setRunType(PixScan::RAW_PATTERN);
//   m_PixScan->setLoopParam(0,PixScan::BOC_RX_DELAY);
//   m_PixScan->setLoopVarValues(0, 0, 0, 1);
//   m_PixScan->setLoopVarValues(1, 0, 0, 1);

  // no data storage, do analysis on the fly
  pixScanRunOptions opts;
  opts.scanConfig = m_PixScan;
  opts.loadToAna = false;
  opts.anaLabel = "tmp MCC pattern scan";
  opts.writeToFile = false;
  opts.fileName = "";
  opts.determineSDSPcfg = true;
  opts.runFEbyFE = false;

  // if only one module is requested, disable rest
  std::map <long int,bool> modActives;
  if(in_mod!=0){
    // loop over modules
    for(moduleIterator mi = modBegin(); mi != modEnd(); mi++){
      Config &modconf = (*mi)->config();
      // store current setting
      bool isAct = ((ConfList&)modconf["general"]["Active"]).sValue()=="TRUE";
      modActives.insert(std::make_pair((long int)(*mi),isAct));
      // set to inactive except for this module
      WriteIntConf((ConfInt&)modconf["general"]["Active"],((*mi)==in_mod));
    }
    // set status to testing
    modStatus modS = getPixModuleStatus(in_mod->moduleId());
    if(modS.modStat != tfailed)
      modS.modStat = tbusy;
    setModStatusEvent *mse = new setModStatusEvent(modS, in_mod);
    m_app->postEvent(this, mse);
    if(thread==0) // this is only possible in single-threaded mode
      m_app->processEvents();
  }

  if(thread!=0){ // this was called from a thread, so use that thread for executing scan
    thread->setScanOpts(opts);
//     thread->scaninit();
//     processing = true;
    thread->scan();
    processing = true;
  } else{ // called from the main thread, execute scan in a new thread
    processing = true;
//     ThreadExecute(MTscaninit, opts);
//     do{
//       usleep(10000);
//       m_app->processEvents();
//     }
//     while(getProcessing());
//     processing = true;
    ThreadExecute(MTscan, opts);
    do{
      PixLib::sleep(10);
      m_app->processEvents();
    }
    while(getProcessing());
  }

  // get PixScan object associated with this ROD
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    if(((*mi)->config())["general"].name()!="__TrashConfGroup__" &&
       ((*mi)->config())["general"]["Active"].name()!="__TrashConfObj__" &&
       (((ConfList&)((*mi)->config())["general"]["Active"]).sValue()!="FALSE")){
      
      modStatus modS = getPixModuleStatus((*mi)->moduleId());
      if(modS.modStat == tbusy)
	modS.modStat = tunknown;
      PixLib::Histo &hidif = m_PixScan->getHisto(PixScan::RAW_DATA_DIFF_1,(*mi)->moduleId(),0,0,0);
      PixLib::Histo &hiraw = m_PixScan->getHisto(PixScan::RAW_DATA_1,(*mi)->moduleId(),0,0,0);
      if(hidif.name()!="Not Found" && hiraw.name()!="Not Found"){
	int nbfl=0, nraw=0, ibin;
	for(ibin=0;ibin<hidif.nBin(0);ibin++)
	  nbfl+=(int)hidif(ibin,0);
	for(ibin=0;ibin<hiraw.nBin(0);ibin++)
	  nraw+=(int)hiraw(ibin,0);
	if(nbfl>0 || nraw==0){
	  std::stringstream a;
	  a << nbfl;
	  modS.modStat = tfailed;
	  modS.mccStat =  tfailed;
	  if(nraw==0)
	    modS.mccMsg  += "Link test: no signal observed\n";
	  else
	    modS.mccMsg  += "Link test: " + a.str() + " bit flips\n";
	  modS.modMsg += "Link test failed\n";
	} else{
	  if(modS.modStat==tunknown || modS.modStat==tblocked)
	    modS.modStat = tOK;
	  modS.mccStat = tOK;
	  modS.modMsg += "Successful link test\n";
	  modS.mccMsg += "Successful link test\n";
	}
      }
      setModStatusEvent *mse = new setModStatusEvent(modS, (*mi));
      m_app->postEvent(this, mse);
      if(thread==0) // this is only possible in single-threaded mode
	m_app->processEvents();
    }
  }
  // reset to clear memory
  m_PixScan->resetScan();

  // re-set module config if this was modified earlier on
  if(in_mod!=0){
    // loop over modules
    for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
      if((*mi)!=in_mod){
	Config &modconf = (*mi)->config();
	// set to original avtive settings
	WriteIntConf((ConfInt&)modconf["general"]["Active"],modActives[(long int)(*mi)]);
      }
    }
  }

}
void STPixModuleGroup::runChipTest(ChipTest*ct, bool resetMods, PixModule *mod){
  std::string txt, allTxt;
  if(getPixController()!=0 && getPixCtrlStatus()==tOK){
    setCtrlStatusEvent* cseb = new setCtrlStatusEvent(tbusy,this);
    m_app->postEvent(this, cseb);
    if(mod!=0) // this is only possible in single-threaded mode
      m_app->processEvents();
    // set all modules inactive, store prev. active/inactive state
    std::map<long int, bool> activeStates;
    for(moduleIterator mia = modBegin(); mia != modEnd(); mia++){
      Config &conf = (*mia)->config(); // 
      activeStates.insert(std::make_pair((long int)(*mia),((ConfList&)conf["general"]["Active"]).sValue()=="TRUE"));
      WriteIntConf((ConfInt&)conf["general"]["Active"],0);
    }
    // loop over modules
    moduleIterator mStart = modBegin();
    moduleIterator mStop  = modEnd();
    if(mod!=0){
      while((*mStart)!=mod)
	mStart++;
      if(mStart!=modEnd())
	mStop = mStart + 1;
    }
    for(moduleIterator mi = mStart; mi != mStop; mi++){
      Config &conf = (*mi)->config();
      // skip inactive modules
      if(!(activeStates[(long int)(*mi)])) continue;
      // enable this module temporarily
      WriteIntConf((ConfInt&)conf["general"]["Active"],1); 
      //downloadConfig();
      // check FE flavour
      int feflv = ((ConfInt&)conf["general"]["FE_Flavour"]).valueInt();
      bool isFeI4 = true;
      if(feflv==PixLib::PixModule::PM_FE_I1 ||  feflv==PixLib::PixModule::PM_FE_I2)
	isFeI4 = false;
      // get current module status
      modStatus modS;
      modS = getPixModuleStatus((*mi)->moduleId());
      StatusTag orgStat = modS.modStat;
      modS.modStat = tbusy;
      setModStatusEvent *mse = new setModStatusEvent(modS, (*mi));
      m_app->postEvent(this, mse);
      if(mod!=0) // this is only possible in single-threaded mode
	m_app->processEvents();
      
      // run the test for selected module
      switch(ct->getType()){
      case ChipTest::GLOBALREG:
      case ChipTest::PIXELREG:{
	if(((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::CURRENTCFG &&
	   ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY){
	  // store current cfg. for restoring it later
	  (*mi)->storeConfig("PreChiptestConfig");
	}
	for(std::vector<PixFe*>::iterator feit = (*mi)->feBegin(); feit != (*mi)->feEnd(); feit++){
	  PixFe *myFe = *feit;
	  // prepare config - still to do
	  bool allDcsIdent = false;
	  if(((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::CURRENTCFG &&
	     ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY){
	    int cttype = ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue();
	    allDcsIdent = cttype!=ChipTest::RANDOM && cttype!=ChipTest::CURRENTCFG;
	    if(ct->getType()==ChipTest::GLOBALREG) setGlobReg(cttype, myFe);
	    else                                   setPixlReg(cttype, myFe);
	  }
	  // run test
	  // configure module
	  if(((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY) downloadConfig();
	  // sending config is done as part of test now, just send global reg. for pixel reg. test
	  getPixController()->setConfigurationMode();
	  if(ct->getType()==ChipTest::PIXELREG && 
	     ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY) 
	    getPixController()->sendGlobal(0);
	  // run test
	  //PixLib::sleep(100);
	  bool tpassed = false;

	  // run the requested register test
	  if(ct->getType()==ChipTest::GLOBALREG){
	    std::vector<int> data_in, data_out;
	    std::vector<std::string> regLabels;
	    tpassed = getPixController()->testGlobalRegister((*mi)->moduleId(),data_in, data_out, regLabels, 
							   ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY,
							   myFe->number());
	    allTxt = "Register\tSent\tRead\n";
	    for(int i=0;i<(int)data_in.size(); i++){
	      stringstream a,b;
	      a << data_in[i];
	      b << data_out[i];
	      allTxt += regLabels[i] + ":\t"+ a.str() + "\t" + b.str()+"\n";
	    }
	    // store status
	    std::stringstream a;
	    a << std::hex << myFe->number();
	    if(tpassed){
	      modS.feStat[myFe->number()] = tOK;
	      modS.feMsg[myFe->number()] += "Passed GR test:\n"+allTxt+"\n";
	      if(orgStat==tunknown) modS.modStat = tOK;
	      else                  modS.modStat = orgStat;
	      modS.modMsg += "FE"+a.str()+" passed GR test\n";
	    } else{
	      modS.modMsg += "FE"+a.str()+" failed GR test\n";
	      modS.feStat[myFe->number()] = tfailed;
	      modS.feMsg[myFe->number()] += "Failed GR test:\n"+allTxt+"\n";
	      modS.modStat = tfailed;
	      orgStat = tfailed;
	    }
	  }else{ // i.e. is PIXELREG
	    tpassed = true;
	    // generate a list of register names from FE config
	    std::string cfgName = "Latch";
	    if(!isFeI4) cfgName = "LatchI3";
	    std::map<std::string,int> pixNames;
	    pixNames= ((ConfList&)ct->config()["options"][cfgName]).symbols();
	    if(((ConfList&)ct->config()["options"][cfgName]).sValue()!="ALL"){
	      pixNames.clear();
	      pixNames[((ConfList&)ct->config()["options"][cfgName]).sValue()] = 0;
	    }
	    bool ignoreDCsOff = ((ConfBool&)ct->config()["options"]["IgnoreDCsOff"]).value();
	    bool bypass = ((ConfBool&)ct->config()["options"]["BypassMode"]).value();
	    int myDC = -1;
	    int cfgDC = ((ConfInt&)ct->config()["options"]["DCtoTest"]).getValue();
	    if(cfgDC>=0 && cfgDC<40)
	      myDC = cfgDC;
	    else if(allDcsIdent)
	      myDC = 40;
	    for(std::map<std::string,int>::iterator pnIT=pixNames.begin(); pnIT!=pixNames.end();pnIT++){
	      if(pnIT->first!="ALL"){ // skip ALL label from ChipTest list, doesn't correspond to register
		// 	      cout << "STPixModuleGroup::runChipTest: testing pixel reg. " << (pnIT->first) << 
		// 		" with arguments " << (ignoreDCsOff?"TRUE":"FALSE") << " "<< myDC << endl;
		//PixLib::sleep(100);
		bool this_passed;
		std::vector<int> data_in, data_out;
		this_passed = getPixController()->testPixelRegister((*mi)->moduleId(),pnIT->first, data_in, data_out,ignoreDCsOff,myDC, 
						       ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY,
						       myFe->number(), bypass);
		allTxt = "Sent\tRead\n";
		for(int i=0;i<(int)data_in.size(); i++){
		  stringstream a,b,c;
		  if(isFeI4)
		    c << "DC"<<(((i/21)<10)?" ":"")<<std::dec<<(i/21)<<", word "<<(i%21);
		  else
		    c << "DC"<<(((i/10)<10)?" ":"")<<std::dec<<(i/10)<<", word "<<(i%10);
		  a << std::hex << (unsigned int) data_in[i];
		  b << std::hex << (unsigned int) data_out[i];
		  allTxt += c.str()+":\t0x" + a.str() + "\t0x" + b.str()+"\n";
		}
		tpassed &= this_passed;
		if(this_passed){
		  modS.feMsg[myFe->number()] += "Passed test of "+(pnIT->first)+":\n"+allTxt+"\n";
		  if(orgStat==tunknown) modS.modStat = tOK;
		  else                  modS.modStat = orgStat;
		} else{
		  modS.feMsg[myFe->number()] += "Failed test of "+(pnIT->first)+":\n"+allTxt+"\n";
		  modS.modStat = tfailed;
		  orgStat = tfailed;
		}
	      }
	    }
	    std::stringstream a;
	    a << std::hex << myFe->number();
	    if(tpassed){
	      modS.feStat[myFe->number()] = tOK;
	      modS.modMsg += "FE"+a.str()+" passed PR test\n";
	    }else{
	      modS.feStat[myFe->number()] = tfailed;
	      modS.modMsg += "FE"+a.str()+" failed PR test\n";
	    }
	  }
	}
	if(((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::CURRENTCFG &&
	   ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY){
	  // restore old config
	  (*mi)->restoreConfig("PreChiptestConfig");
	  (*mi)->deleteConfig("PreChiptestConfig");
	}
	break;}
      case ChipTest::SCANCHAIN:{
	modS.modStat = orgStat;
	std::string fileName = ((ConfString&)ct->config()["options"]["SteeringFile"]).value();
	std::string chainName = ((ConfList&)ct->config()["options"]["ChainType"]).sValue();
	std::string dcsName = ((ConfString&)ct->config()["options"]["DscName"]).value();
	PixDcs *dcs=0;
	// where do we get this from ???
	STRodCrate *crt = dynamic_cast<STRodCrate*>(parent());
	std::vector<PixDcs*> dcsv = crt->getPixDcs();
	for(std::vector<PixDcs*>::iterator it=dcsv.begin(); it!=dcsv.end(); it++){
	  if((*it)->name()==dcsName){
	    dcs=*it;
	    break;
	  }
	}

	allTxt="";
	for(std::vector<PixFe*>::iterator feit = (*mi)->feBegin(); feit != (*mi)->feEnd(); feit++){
	  PixFe *myFe = *feit;
	  if(dynamic_cast<PixFeI4B*>(myFe)==0 && dynamic_cast<PixFeI4A*>(myFe)==0){
	      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::runChipTest() : "
						       "FE's in the cfg. are neither FE-I4A nor -I4B, can't run scan chain.\n");
	      m_app->postEvent(this, tee);
	      break;
	  }
	  double curr_bef, curr_after;
	  bool tpassed = true;
	  // to do: open file
	  char line[2000];
	  FILE *stil = fopen(fileName.c_str(), "r");
	  if(stil==0){
	    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::runChipTest() : "
							 "stil-file provided for scan chain tests doesn't exist: "+fileName+"\n");
	    m_app->postEvent(this, tee);
	    break;
	  }
	  bool gotSi=false, gotP0=false;
	  QString patternI, patternO;
	  int ipat=0;
	  bool se_while_pulse = false;
	  bool si_while_pulse = false;
	  bool shift_only = false;
	  while(fgets(line,2000,stil)!=0){
	    QString qsline = line;
	    if(qsline.indexOf("Pattern 0", 0, Qt::CaseInsensitive) != -1) gotP0=true;
	    if (qsline.indexOf("si\"=", 0, Qt::CaseInsensitive) != -1 && gotP0){
	      patternI = qsline;
	      bool keeprd=true;
	      while(keeprd){
		if (!(patternI.indexOf(";", 0) != -1)) {
		  keeprd = (fgets(line,2000,stil)!=0);
		  patternI += QString(line);
		} else
		  break;
	      }
	      gotSi = true;
	      patternI.replace(QString("\n"), QString(""));
	      patternI = patternI.mid(patternI.indexOf("=", 0)+1, patternI.indexOf(";", 0)-1 - patternI.indexOf("=", 0));
	    }
	    QString pi_Definition = "";
	    if (qsline.indexOf("_pi\"=", 0, Qt::CaseInsensitive) != -1){
	      pi_Definition = qsline;
	      bool keeprd=true;
	      while(keeprd){
		if (!(pi_Definition.indexOf(";", 0) != -1)) {
		  keeprd = (fgets(line,2000,stil)!=0);
		  pi_Definition += QString(line);
		} else
		  break;
	      }
	      //keeprd = (fgets(line,2000,stil)!=0);
	      QString checkline = line;
	      while(keeprd){
		if (checkline.indexOf("_po\"=", 0) != -1) {
		  break;
		} else {
		  //cout << "Debugging STPixModuleGroup: Checking for second _pi definition." << endl;
		  keeprd = (fgets(line,2000,stil)!=0);
		  checkline = line;
		  //cout << checkline << endl;
		  if (checkline.indexOf("_pi\"=", 0, Qt::CaseInsensitive) != -1)
		    {
		      pi_Definition = checkline;
		      bool keeprd=true;
		      while(keeprd){
			if (!(pi_Definition.indexOf(";", 0) != -1)) {
			  keeprd = (fgets(line,2000,stil)!=0);
			  pi_Definition += QString(line);
			} else
			  break;
		      }
		    }
		}
	      }
	      
	      int se_select_decr = 0;
	      int si_select_decr = 0;
	      int clk_select_decr = 0;
	      if(chainName=="DOB") {se_select_decr = 2; si_select_decr = 1, clk_select_decr = 4;}
	      if(chainName=="CMD") {se_select_decr = 3; si_select_decr = 2, clk_select_decr = 4;}
	      if(chainName=="ECL") {se_select_decr = 4; si_select_decr = 1, clk_select_decr = 3;}
	      QString SEstateString = pi_Definition.mid(pi_Definition.indexOf(";", 0)-se_select_decr, 1);
	      QString SIstateString = pi_Definition.mid(pi_Definition.indexOf(";", 0)-si_select_decr, 1);
	      QString CLKstateString = pi_Definition.mid(pi_Definition.indexOf(";", 0)-clk_select_decr, 1);
	      //SEstateString = SEstateString.mid(SEstateString.indexOf(";", 0)-se_select_decr, 1);
	      //SEstateString = SEstateString.mid(SEstateString.indexOf(";", 0)-2, 1);
	      se_while_pulse = SEstateString.toInt();
	      if (SIstateString != "0")
		SIstateString = "1";
	      si_while_pulse = SIstateString.toInt();
	      if (CLKstateString != "0")
		CLKstateString = "1";
	      shift_only = !CLKstateString.toInt();
	    }
	    if (qsline.indexOf("so\"=", 0, Qt::CaseInsensitive) != -1 && gotP0){
	      patternO = qsline;
	      bool keeprd2=true;
	      while(keeprd2){
		if (!(patternO.indexOf(";", 0) != -1)) {
		  keeprd2 = (fgets(line,2000,stil)!=0);
		  patternO += QString(line);
		} else
		  break;
	      }
	      patternO.replace(QString("\n"), QString(""));
	      patternO = patternO.mid(patternO.indexOf("=", 0)+1, patternO.indexOf(";", 0)-1 - patternO.indexOf("=", 0));
	      patternO.replace(QString("H"), QString("1"));
	      patternO.replace(QString("L"), QString("0"));
	      if(gotSi){
		gotSi = false;
		if(patternI.length()!=patternO.length()){
		  std::cerr << "different length of in and exp. out data" << std::endl;
		  tpassed = false;
		}else{
		  std::vector<int> data_in;
		  std::string data_out, data_cmp=patternO.toLatin1().data();
		  // turn bit-strings into 32-bit words
		  int is;
		  Bits bi(patternI.length());
		  for(is=0;is<bi.size();is++) bi.set(is, (patternI.mid(is,1)=="1"));
		  for(is=0;is<bi.size()/32;is++) data_in.push_back(bi.pack(is));
		  if((bi.size()-32*is)>0) data_in.push_back(bi.get(is*32, bi.size()-1));
		  // 		  Bits bo(patternO.length());
		  // 		  for(is=0;is<bo.size();is++) bo.set(is, (patternO.mid(is,1)=="1"));
		  // 		  for(is=0;is<bo.size()/32;is++) data_cmp.push_back(bo.pack(is));
		  // 		  if((bo.size()-32*is)>0) data_cmp.push_back(bo.get(is*32,bo.size()-1));
		  std::stringstream msg;
		  try{
		    //cout << "Running SC pattern " << ipat << " with se-flag " << (se_while_pulse?"ON":"OFF") << endl;
		    tpassed &= getPixController()->testScanChain(chainName, data_in, data_out, data_cmp, shift_only/*(ipat==0)*/, se_while_pulse, si_while_pulse,
								 dcs, curr_bef, curr_after, myFe->number());
		  }catch (USBPixControllerExc &upcExc){
		    upcExc.dump(msg);
		  } catch(std::exception& s){
		    msg << "Std-lib exception ";
		    msg << s.what();
		  } catch(...){
		    msg << "Unknown exception ";
		  }
		  if(msg.str()!=""){
		    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::runChipTest(): Exception " + msg.str() +
								 " not caught during execution of scan chain with group "+
								 getName() + "\n" );
		    m_app->postEvent(this, tee);
		  }
		  
		  std::stringstream cpat;
		  cpat << ipat;
		  ipat++;
		  allTxt += "Pattern "+cpat.str()+"\nExp.: "+data_cmp+"\nRead: "+data_out+"\n";
		  stringstream a,b;
		  a << curr_bef;
		  b << curr_after;
		  allTxt += "Curr. before: " +a.str() + "\nCurr. after: " + b.str()+"\n";
		}
	      }
	    }
	    fclose(stil);
	    std::stringstream a;
	    a << std::hex << myFe->number();
	    if(tpassed){
	      modS.feStat[myFe->number()] = tOK;
	      modS.feMsg[myFe->number()] += "Passed test of scan chain "+chainName+":\n"+allTxt+"\n";
	      if(orgStat==tunknown) modS.modStat = tOK;
	      else                  modS.modStat = orgStat;
	      modS.modMsg += "FE"+a.str()+" passed test of scan chain "+chainName+"\n";
	    }else{
	      modS.feStat[myFe->number()] = tfailed;
	      modS.feMsg[myFe->number()] += "Failed test of scan chain "+chainName+":\n"+allTxt+"\n";
	      modS.modMsg += "FE"+a.str()+" failed test of scan chain "+chainName+"\n";
	      modS.modStat = tfailed;
	      orgStat=tfailed;
	    }
	  }
	}
	break;}
      default:
	break;
      }
      // set module status
      mse = new setModStatusEvent(modS, (*mi));
      m_app->postEvent(this, mse);
      if(mod!=0) // this is only possible in single-threaded mode
	m_app->processEvents();
      // disable this module temporarily
      WriteIntConf((ConfInt&)conf["general"]["Active"],0); 
      if(ct->config()["options"]["Bitpattern"].name()!="__TrashConfObj__" && 
	 ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY) downloadConfig();
      // reset this module if requested
      if(resetMods){
	getPixController()->setCalibrationMode();
	(*mi)->reset(0);
	(*mi)->reset(1);
      }
    }
    // set all modules back to their org. active state
    for(moduleIterator mi = modBegin(); mi != modEnd(); mi++){
      Config &conf = (*mi)->config();
      WriteIntConf((ConfInt&)conf["general"]["Active"],(int)activeStates[(long int)(*mi)]);
      if(ct->config()["options"]["Bitpattern"].name()!="__TrashConfObj__" && 
	 ((ConfList&)ct->config()["options"]["Bitpattern"]).getValue()!=ChipTest::READONLY) downloadConfig();
    }
    // fully reset modules
    if(resetMods){
      getPixController()->setCalibrationMode();
      // Loop over modules
      for( moduleIterator module = modBegin(); module !=  modEnd(); module++) {
	(*module)->reset(0);
	(*module)->reset(1);
      }
    }
    // release this ROD
    setCtrlStatusEvent* cseo = new setCtrlStatusEvent(tOK,this);
    m_app->postEvent(this, cseo);
    if(mod!=0) // this is only possible in single-threaded mode
      m_app->processEvents();
  } else{
    ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::runChipTest() : "
                                             "Could not find operational PixController for group "+
                                             getName()+".\n");
    m_app->postEvent(this, tle);
  }
}
void STPixModuleGroup::customEvent( QEvent * event )
{
  if(event==0){
    emit errorMessage("STPixModuleGroup::customEvent called with a NULL-pointer");
    return;
  }
  
  if ( event->type() == 2000 ) {           // It must be a ThreadErrorEvent
    emit errorMessage(dynamic_cast<ThreadErrorEvent*>(event)->getText() );
  }
  if (event->type() == 2001){             // Or a ThreadLogEvent
    emit logMessage(dynamic_cast<ThreadLogEvent*>(event)->getText() );
  }
  if(event->type()==2002){ // set ROD status
    dynamic_cast<setCtrlStatusEvent*>(event)->getGroup()->setPixCtrlStatus(dynamic_cast<setCtrlStatusEvent*>(event)->getStatus());
  }
  if(event->type()==2003){ // set module status
    PixModule *mod = dynamic_cast<setModStatusEvent*>(event)->getModule();
    STPixModuleGroup *grp = (STPixModuleGroup*) mod->getPixModGroup();
    grp->setPixModuleStatus(mod->moduleId(),dynamic_cast<setModStatusEvent*>(event)->getStatus());
  }
  if (event->type() == 2004){             // an event update module test panel
    emit sendRTStatus((dynamic_cast<setMTestStatusEvent*>(event)->getText()).c_str() );
  }
  if (event->type() == 2005){             // Or a ThreadBufferEvent
    ThreadBufferEvent *tbe = dynamic_cast<ThreadBufferEvent*>(event);
    emit rodMessage(tbe->getText(), tbe->getErr() );
  }
  if (event->type() == 2006){ // set ROD status
    dynamic_cast<setBocStatusEvent*>(event)->getGroup()->setPixBocStatus(dynamic_cast<setBocStatusEvent*>(event)->getStatus());
  }
  if (event->type() == 2007){ // set DCS value
    emit setDcs(dynamic_cast<setDcsEvent*>(event)->getChanName(), dynamic_cast<setDcsEvent*>(event)->getParName(), dynamic_cast<setDcsEvent*>(event)->getValue(),
		dynamic_cast<setDcsEvent*>(event)->getCmdId(), dynamic_cast<setDcsEvent*>(event)->getGroup());
  }
}
int STPixModuleGroup::setTFDACs(const char *fname, bool isTDAC)
{
  int not_found=0;
  std::string DACname = isTDAC?"TDAC":"FDAC";
  //printf("loading %s\n",DACname.c_str());
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    std::string path = fname;
    path += "/" + getName();
    Histo *hi = 0;
    PixDBData *DataObj=0;
    try{
      std::string mname = (*mi)->moduleName();
      getDecNameCore(mname);
      DataObj = new PixDBData("Name",path.c_str(),mname.c_str());
      if(isTDAC && DataObj->haveHistoType(PixScan::TDAC_T))
	hi = DataObj->getGenericPixLibHisto(PixScan::TDAC_T);
      if(!isTDAC && DataObj->haveHistoType(PixScan::FDAC_T))
	hi = DataObj->getGenericPixLibHisto(PixScan::FDAC_T);
    } catch(...){
      DataObj = 0;
      hi = 0;
    }
    if(hi!=0){
      int chip=0;
      for(PixModule::feIterator fei = (*mi)->feBegin(); fei!=(*mi)->feEnd(); fei++){
	Config &feconf = (*fei)->config();      
	Config &trimreg = feconf.subConfig("Trim_0/Trim");
	if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
	   trimreg["Trim"][DACname].name()!="__TrashConfObj__" &&
	   trimreg["Trim"][DACname].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)trimreg["Trim"][DACname]).subtype()==ConfMatrix::U16){
	  ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][DACname];
	  ConfMask<unsigned short int> &myMask = mapobj.valueU16();
	  int nCol = myMask.get().size();
	  int nRow = myMask.get().front().size();	
	  for(int col=0;col<nCol;col++){
	    for(int row=0;row<nRow;row++){
	      int binx, biny;
	      DataObj->PixCCRtoBins(chip,col,row,binx,biny);
	      // function was designed for root, so need offset by 1!
	      binx--; biny--;
	      myMask.set(col, row, (unsigned short int)((*hi)(binx,biny)));
	    }
	  }
	}
	chip++;
      }
      // set edited flag accordingly
      editedCfg((*mi)->moduleId());
    } else {
      not_found++;
      std::string errMsg = "STPixModuleGroup::setTFDACs : Couldn't find ";
      errMsg += DACname;
      errMsg += " histo for module "+(*mi)->moduleName()+" in file ";
      errMsg += fname;
      emit logMessage(errMsg);
    }
    delete DataObj;
  }
  return not_found;
}
int STPixModuleGroup::setMasks(std::vector<std::string> files, std::vector<std::string> histos, int mask, std::string logicOper){
//setMasks(const char *fname, const char *hname, int mask){
  int not_found=0;
  if(files.size()!=histos.size()) return 0;

  std::string DACname[4]={"ENABLE","CAP1","CAP0","ILEAK"};
  PixScan ps;

  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    Histo *hi=0;
    PixDBData *DataObj=0;
    for(unsigned int i=0;i<histos.size(); i++){
      std::string path = files[i];
      path += "/" + getName();
      int htype = ps.getHistoTypes()[histos[i]];
      try{
	std::string mname = (*mi)->moduleName();
	getDecNameCore(mname);
	DataObj = new PixDBData("Name",path.c_str(),mname.c_str());
	if(DataObj!=0 && DataObj->haveHistoType(htype)){
	  Histo *tmphi = DataObj->getGenericPixLibHisto((PixLib::PixScan::HistogramType)htype);
	  if(tmphi!=0){
	    if(hi!=0){
	      for(int j=0;j<hi->nBin(0);j++){
		for(int k=0;k<hi->nBin(1);k++){
		  if(logicOper=="OR")
		    hi->set(j,k,(double)(((*tmphi)(j,k)>0.5) || ((*hi)(j,k)>0.5)));
		  else if (logicOper=="AND")
		    hi->set(j,k,(double)(((*tmphi)(j,k)>0.5) && ((*hi)(j,k)>0.5)));
		  else // don't know what to do
		    hi->set(j,k,0.);
		}
	      }
	    } else
	      hi = new Histo(*tmphi);
	  }
	}
      } catch(...){
	DataObj = 0;
      }
      // clean up, but leave last pixDBData-object for later use, to be deleted there
      if(i<(histos.size()-1)){
	//std::cout << "Iteration " << i << " of " << histos.size() << ": deleting" << std::endl;
	delete DataObj; DataObj=0;
      }
    }
    if(hi!=0 && DataObj!=0){
      int chip=0;
      for(PixModule::feIterator fei = (*mi)->feBegin(); fei!=(*mi)->feEnd(); fei++){
	for(int iMask=0;iMask<4;iMask++){
	  if(mask&(1<<iMask)){ // mask was selected
	    Config &feconf = (*fei)->config();      
	    Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
	    if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
	       pixreg["PixelRegister"][DACname[iMask]].name()!="__TrashConfObj__" &&
	       pixreg["PixelRegister"][DACname[iMask]].type()==ConfObj::MATRIX &&
	       ((ConfMatrix&)pixreg["PixelRegister"][DACname[iMask]]).subtype()==ConfMatrix::U1){
	      ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][DACname[iMask]];
	      ConfMask<bool> &myMask = mapobj.valueU1();
	      int nCol = myMask.get().size();
	      int nRow = myMask.get().front().size();	
	      for(int col=0;col<nCol;col++){
		for(int row=0;row<nRow;row++){
		  int binx, biny;
		  DataObj->PixCCRtoBins(chip,col,row,binx,biny);
		  // function was designed for root, so need offset by 1!
		  binx--;
		  biny--;
		  myMask.set(col, row, ((*hi)(binx,biny)>0.5));
		}
	      }
	    }
	  }
	}
	chip++;
      }
      delete hi;
      delete DataObj;
      // set edited flag accordingly
      editedCfg((*mi)->moduleId());
    } else {
      not_found++;
      std::string errMsg = "STPixModuleGroup::setMasks : Couldn't find ";
      errMsg += std::string(histos[0]);
      errMsg += " histo for module "+(*mi)->moduleName()+" in file ";
      errMsg += files[0];
      emit logMessage(errMsg);
    }
  }
  return not_found;
}
void STPixModuleGroup::setGR(std::string GRname, int GRval){
  std::string grpN, objN;
  grpN = GRname;
  objN = GRname;
  unsigned int pos = (unsigned int)grpN.find("_");
  if(pos!=(unsigned int)std::string::npos){
    grpN.erase(pos,grpN.length()-pos);
    objN.erase(0,pos+1);
  }
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    for(PixModule::feIterator fei = (*mi)->feBegin(); fei!=(*mi)->feEnd(); fei++){
      Config &feconf = (*fei)->config().subConfig("GlobalRegister_0/GlobalRegister");
      if(feconf[grpN].name()!="__TrashConfGroup__" && feconf[grpN][objN].name()!="__TrashConfObj__"){
	WriteIntConf(((ConfInt&) feconf[grpN][objN]), GRval);
      } else{
	std::stringstream fen;
	fen << (*fei)->number();
	std::string errMsg = "STPixModuleGroup::setGR : Couldn't find " + GRname + " in FE " + fen.str() + " of module " + (*mi)->moduleName();
	emit logMessage(errMsg);
      }
    }
    editedCfg((*mi)->moduleId());
  }
}
void STPixModuleGroup::setPR(std::string PRname, int PRval){
  std::string grpN, objN;
  grpN = PRname;
  objN = PRname;
  unsigned int pos = (unsigned int)grpN.find("_");
  if(pos!=(unsigned int)std::string::npos){
    grpN.erase(pos,grpN.length()-pos);
    objN.erase(0,pos+1);
  }
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    for(PixModule::feIterator fei = (*mi)->feBegin(); fei!=(*mi)->feEnd(); fei++){
      Config &feconf = (*fei)->config().subConfig(grpN+"_0/"+grpN);
//       if(grpN!="PixelRegister" && grpN!="Trim"){
// 	emit logMessage("STPixModuleGroup::setPR : unknown register group name: "+ grpN);
// 	return;
//       }
      if(feconf.name()=="__TrashConfig__"){
	emit logMessage("STPixModuleGroup::setPR : unknown register group name: "+ grpN);
	return;
      }
      if(feconf[grpN].name()!="__TrashConfGroup__" && feconf[grpN][objN].name()!="__TrashConfObj__"){
	if(feconf[grpN][objN].type()==ConfObj::MATRIX && ((ConfMatrix&)feconf[grpN][objN]).subtype()==ConfMatrix::U16){
	  ConfMatrix &mapobj = (ConfMatrix&)feconf[grpN][objN];
	  std::vector<unsigned short int> temp;
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
	  int imax = (int)temp.size();
	  temp.clear();
	  for(int i=0;i<imax;i++){
	    temp.push_back((unsigned short int)PRval);
	  }
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
	} else if(feconf[grpN][objN].type()==ConfObj::MATRIX && ((ConfMatrix&)feconf[grpN][objN]).subtype()==ConfMatrix::U1){
	  ConfMatrix &mapobj = (ConfMatrix&)feconf[grpN][objN];
	  std::vector<bool> temp;
	  ((ConfMask<bool> *)mapobj.m_value)->get(temp);
	  int imax = (int)temp.size();
	  temp.clear();
	  for(int i=0;i<imax;i++){
	    temp.push_back((bool)PRval);
	  }
	  ((ConfMask<bool> *)mapobj.m_value)->set(temp);
	}
      } else{
	std::stringstream fen;
	fen << (*fei)->number();
	std::string errMsg = "STPixModuleGroup::setPR : Couldn't find " + objN + " in group " + grpN + 
	                     " in FE " + fen.str() + " of module " + (*mi)->moduleName();
	emit logMessage(errMsg);
      }
    }
    editedCfg((*mi)->moduleId());
  }
}
void STPixModuleGroup::setVcal(float charge, bool Chigh)
{
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    for(PixModule::feIterator fei = (*mi)->feBegin(); fei!=(*mi)->feEnd(); fei++){
      Config &feconf = (*fei)->config();
      if(feconf["Misc"].name()!="__TrashConfGroup__"){
	double Cval, par[4], result[3];
	if(Chigh)
	  Cval = (double)((ConfFloat&)feconf["Misc"]["CInjHi"]).value();
	else
	  Cval = (double)((ConfFloat&)feconf["Misc"]["CInjLo"]).value();
	char vcalstr[30];
	for(int k=0;k<4;k++){
	  sprintf(vcalstr,"VcalGradient%d",k);
	  par[k] = (double)((ConfFloat&)feconf["Misc"][vcalstr]).value();
	}
	par[0] -= (double)charge*0.160218/Cval;
	int myval = -1;
	int nsol = solveCubic(par,result);
	int iV;
	for(iV=0;iV<nsol;iV++){
	  //	      printf("%lf\n",result[iV]);
	  if(result[iV]>0 && result[iV]<1023)
	    myval = (int)(result[iV]+0.5);
	}
	Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
	if(globreg["GlobalRegister"].name()!="__TrashConfGroup__" &&
	   myval>0){
	  PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fei);
	  if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fei);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"][(fei4==0)?"DAC_VCAL":"PlsrDAC"],myval);
	}
      }
    }
    // send modified module config to PixController if that's initialised
    if(getPixCtrlStatus()==tOK){
      try{
	//(*mi)->writeConfig();
	getPixController()->writeModuleConfig(**mi);
      } catch(...){
	emit errorMessage("STPixModuleGroup::setVcal : Error sending edited configuration of "+
			  (*mi)->moduleName()+" to PixController!");
      }
      //      readRodBuff("Post-config edit:");
    }
    // set edited flags accordingly
    editedCfg((*mi)->moduleId());
  }
}
void STPixModuleGroup::incrMccDelay(float in_delay, bool calib)
{
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    if((*mi)->pixMCC()!=0){
      float delay, delIncr = in_delay;
      int range = (*mi)->pixMCC()->readRegister("CAL_Range");
      if(calib){
	Config &conf = (*mi)->pixMCC()->config();
	std::stringstream a;
	a << range;
	float calfac = ((ConfFloat&)conf["Strobe"]["DELAY_"+a.str()]).m_value;
	if(calfac!=0) delIncr /= calfac;
      }
      delay = (float)(*mi)->pixMCC()->readRegister("CAL_Delay");
      delay += delIncr;
      if(delay<0) delay = 0;
      if(delay>63) delay = 63;
    (*mi)->pixMCC()->writeRegister("CAL_Delay", (int)delay);
    }
    // set edited flags accordingly
    editedCfg((*mi)->moduleId());
  }
}
void STPixModuleGroup::disableFailed()
{
  for(PixModuleGroup::moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    modStatus ms = getPixModuleStatus((*mi)->moduleId());
    if(ms.modStat==tfailed){
      try{
	Config &conf = (*mi)->config();
	if(conf["general"].name()!="__TrashConfGroup__" &&
	   conf["general"]["Active"].name()!="__TrashConfObj__") 
	  *((int *)((ConfList&)conf["general"]["Active"]).m_value)=
	    (int)((ConfList&)conf["general"]["Active"]).m_symbols["FALSE"];
      }catch(...){
	ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::disableFailed() : error disabling moduel "
						 + (*mi)->moduleName() + ".\n");
	m_app->postEvent(this, tle);
      }
    }
  }
}
void STPixModuleGroup::processPixScanHistos(pixScanRunOptions scanOpts){ 
#ifndef WIN32
  // only one thread should execute this function at a time
  // locker automatically unlocks mutex when destroyed.
  QMutexLocker locker(writeToFileMutex);
#endif
	      
  //  PixController *pc = getPixController();

  PixConfDBInterface *myData=0;
  DBInquire *root=0, *testInq=0, *dataInq=0, *grpInq=0, *modInq=0;
 
  if(scanOpts.writeToFile && !scanOpts.fileName.empty()){
    // check if file exists, and append new data if so
    FILE *testDB = fopen(scanOpts.fileName.c_str(),"r");
    if(testDB!=0){
      fclose(testDB);
      try{
        myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
      }catch(...){
        myData = 0;
      }
    }
    
    if(myData==0){
      // need new file
      try{
        myData = new RootDB(scanOpts.fileName.c_str(),"NEW");//createEmptyDB(scanOpts.fileName.c_str());
        delete myData;
        myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
      }catch(...){
	ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::processPixScanHistos() : ERROR creating file " + scanOpts.fileName + "\n");
	m_app->postEvent(this, tle);
	ScanStatus=1;
        return;
      }
    }
  
#ifdef WIN32
    m_app->processEvents(); 
#endif
    root = myData->readRootRecord(1);  
    
    if(root==0){
      ThreadLogEvent* tle = new ThreadLogEvent("STPixModuleGroup::processPixScanHistos() : Can't find root record in file" + scanOpts.fileName + ".\n");
      m_app->postEvent(this, tle);
      ScanStatus=1;
      return;
    }
    // check if this scan already has an inquire, and create one if not
    testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
    if(testInq==0){
      std::string name, decName, myDecName;
      name="PixScanResult";
      decName = root->getDecName() + scanOpts.anaLabel.c_str();
      testInq = myData->makeInquire(name, decName);
      root->pushRecord(testInq);
      // process
      myData->DBProcess(root,COMMITREPLACE);
      myData->DBProcess(testInq,COMMIT);
      // close and re-open file to avoid problems
      delete myData;
      myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
      root = myData->readRootRecord(1);
      testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
      // @todo: Handle 0 return. (For example if anaLabel contains a '/' m(
#ifdef WIN32
      m_app->processEvents(); 
#endif
      // add start time stamp to scan config
      DBField *infoField = myData->makeField("TimeStamp");
      std::string ts = scanOpts.timestampStart;
      myData->DBProcess(infoField,COMMIT,ts);
      testInq->pushField(infoField);
      delete infoField;
      // add finish time stamp to scan config
      infoField = myData->makeField("TimeStampEnd");
      std::string tsend = ("Scan terminated at "+QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy")).toLatin1().data();
      myData->DBProcess(infoField,COMMIT,tsend);
      testInq->pushField(infoField);
      delete infoField;
      // add empty comment field
      infoField = myData->makeField("Comment");
      std::string cmnt = "";
      myData->DBProcess(infoField,COMMIT,cmnt);
      testInq->pushField(infoField);
      delete infoField;
      // add std. test ID for later analysis
      DBField *stdidField = myData->makeField("PixScanID");
      myData->DBProcess(stdidField,COMMIT,scanOpts.stdTestID);
      testInq->pushField(stdidField);
      delete stdidField;
      myData->DBProcess(testInq,COMMITREPLACE);    
      // close and re-open file to avoid problems
      delete myData;
      myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
      root = myData->readRootRecord(1);
      testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
    }
	
#ifdef WIN32
    m_app->processEvents(); 
#endif
    // get PixScan object associated with this ROD
    PixScan &cfg = *(getPixScan());
    // create folder for this group
    std::string name, decName, myDecName;
    name="PixModuleGroup";
    decName = testInq->getDecName() + getName();
    grpInq = myData->makeInquire(name, decName);
    testInq->pushRecord(grpInq);
    myData->DBProcess(testInq,COMMITREPLACE);
    myData->DBProcess(grpInq,COMMIT);
    // close and re-open file to avoid problems
    delete myData;
    myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
    root = myData->readRootRecord(1);
    testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
    grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
    if(getPixCtrlStatus()==tOK && getPixController()!=0){
      RootDBField *UPCField = new RootDBField("InfoPixController",myData);
      std::string infostr;
      getPixController()->hwInfo(infostr);
      myData->DBProcess(UPCField,COMMIT,infostr);
      grpInq->pushField(UPCField);
      myData->DBProcess(grpInq,COMMITREPLACE);    
      delete UPCField;
    }
    delete myData;
#ifdef WIN32
    m_app->processEvents(); 
#endif

    // need to close and re-open, otherwise FE subrecords not seen by Config::write...
    myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
    root = myData->readRootRecord(1);
    // save scan config for this group and its histo data
    testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
    grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
    name="PixScanData";
    decName = grpInq->getDecName() + "Data_Scancfg";
    dataInq = myData->makeInquire(name, decName);
    grpInq->pushRecord(dataInq);
    myData->DBProcess(grpInq,COMMITREPLACE);
    myData->DBProcess(dataInq,COMMIT);
    // close and re-open file to avoid problems
    delete myData;
    myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
    root = myData->readRootRecord(1);
    testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
    grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
    dataInq  = *(grpInq->findRecord("Data_Scancfg/PixScanData"));
    cfg.write(dataInq);
    // close and re-open file to avoid problems
    delete myData;
    myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
    root = myData->readRootRecord(1);
    testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
    grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
    if(grpInq!=0){
#ifndef NOTDAQ
      // save boc config if we have one
      if(getPixBoc()!=0){
	DBInquire *bocInq=createEmptyBoc(grpInq,"OpticalBoc");
	// need to close and re-open, otherwise Tx/Rx subrecords not seen by Config::write...
	delete myData;
	myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
	root = myData->readRootRecord(1);
	testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
	grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
	bocInq  = *(grpInq->findRecord("OpticalBoc/PixBoc"));
	// dump boc config
	getPixBoc()->getConfig()->write(bocInq);
	for (int i =0; i<4; i++)
	  {
	    stringstream nr;
	    nr<<"RxNr"<<i;
	    DBInquire *rxInq=*(bocInq->findRecord(nr.str()+"/PixRx"));
	      getPixBoc()->getRx(i)->getConfigRx()->write(rxInq);
	  }
	for (int i =0; i<4; i++)
	  {
	    stringstream nr;
	    nr<<"TxNr"<<i;
	    DBInquire *txInq=*(bocInq->findRecord(nr.str()+"/PixTx"));
	    getPixBoc()->getTx(i)->getConfigTx()->write(txInq);
	  }
	
	myData->DBProcess(bocInq,COMMITREPLACE);
	myData->DBProcess(grpInq,COMMITREPLACE);    
      }
#endif
    }
    delete myData;  
#ifdef WIN32
    m_app->processEvents(); 
#endif
  
    // write DCS readings if acquired during scan
    if(scanOpts.readDcs>0){
      myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
      root = myData->readRootRecord(1);
      testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
      grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
      name="PixDcsReadings";
      decName = grpInq->getDecName() + "UsbPix-DCS";
      dataInq = myData->makeInquire(name, decName);
      grpInq->pushRecord(dataInq);
      myData->DBProcess(grpInq,COMMITREPLACE);
      myData->DBProcess(dataInq,COMMIT);
      // close and re-open file to avoid problems
      delete myData;
      myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
      root = myData->readRootRecord(1);
      testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
      grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
      dataInq  = *(grpInq->findRecord("UsbPix-DCS/PixDcsReadings"));
      for(int id=0;id<(int)m_scanDcsReadings.size();id++){
	RootDBField *DcsField = new RootDBField(m_scanDcsReadings[id].chanName+"_V",myData);
	myData->DBProcess(DcsField,COMMIT,m_scanDcsReadings[id].volts);
	dataInq->pushField(DcsField);
	myData->DBProcess(dataInq,COMMITREPLACE);    
	delete DcsField;
	DcsField = new RootDBField(m_scanDcsReadings[id].chanName+"_I",myData);
	myData->DBProcess(DcsField,COMMIT,m_scanDcsReadings[id].curr);
	dataInq->pushField(DcsField);
	myData->DBProcess(dataInq,COMMITREPLACE);    
	delete DcsField;
      }
      delete myData;
    }

    // save module config
    for(PixModuleGroup::moduleIterator mi = modBegin(); 
	mi != modEnd(); mi++){
      if(((*mi)->config())["general"].name()!="__TrashConfGroup__" &&
	 ((*mi)->config())["general"]["Active"].name()!="__TrashConfObj__" &&
	 (((ConfList&)((*mi)->config())["general"]["Active"]).sValue()!="FALSE")){
	if(scanOpts.writeToFile && !scanOpts.fileName.empty()){
	  std::string mname = (*mi)->moduleName();
	  getDecNameCore(mname);
	  myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
	  root = myData->readRootRecord(1);
	  testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
	  grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
	  // new module entry
	  int nFe = 0;
	  for(PixLib::PixModule::feIterator it = (*mi)->feBegin(); it!=(*mi)->feEnd(); it++) nFe++;
	  modInq  = createEmptyModule(grpInq, mname, nFe, (((*mi)->pixMCC()!=0)?1:0));
	  // need to close and re-open, otherwise FE subrecords not seen by Config::write...
	  // @ to do: complain to Guido about missing update in higher record
	  //   levels when creating stuff from scratch
	  delete myData;
	  myData = new RootDB(scanOpts.fileName.c_str(),"UPDATE");
	  root = myData->readRootRecord(1);
	  testInq = *(root->findRecord(scanOpts.anaLabel + "/PixScanResult"));
	  grpInq  = *(testInq->findRecord(getName()+"/PixModuleGroup"));
	  modInq  = *(grpInq->findRecord(mname+"/PixModule"));
	  // dump module config
	  (*mi)->config().write(modInq);
	  myData->DBProcess(modInq,COMMITREPLACE);
	  myData->DBProcess(grpInq,COMMITREPLACE);    
	  delete myData;
	  m_app->processEvents(); 
	}
      }
    }
    // clear all PixScan objects (will clear all histos, too)
    cfg.resetScan();
  }
  ScanStatus=1;
  return;
}
bool STPixModuleGroup::cfgEdited(int ID)
{
  if(ID==40)      // status of group
    return m_grpEdited;
  else if(ID==41) // status of ROD
    return m_rodEdited;
  else if(ID==42) // status of BOC
    return m_bocEdited;
  else if(module(ID)!=0){ // see if object is a module
    if(m_modEdited[ID]==2)
      return true;
    else
      return false;
  } else
    return false;
}
void STPixModuleGroup::editedCfg(int ID)
{
  if(ID==40) // status of group
    m_grpEdited = true;
  else if(ID==41)// status of ROD
    m_rodEdited = true;
  else if(ID==42) // status of BOC
    m_bocEdited = true;
  else if(module(ID)!=0) // see if object is a module
    m_modEdited[ID]=2;
  return;
}
void STPixModuleGroup::editedCfgAllMods()
{
  for(std::map<int, int>::iterator it=m_modEdited.begin(); it!=m_modEdited.end(); it++)
    it->second = 2;
}
void STPixModuleGroup::savedCfg()
{
  m_grpEdited = false;
  m_rodEdited = false;
  m_bocEdited = false;
  for(std::map<int, int>::iterator it=m_modEdited.begin(); it!=m_modEdited.end(); it++)
    it->second = 1;
  return;
}
void STPixModuleGroup::mergeFeHistos(PixScan **)//cfgs)
{
  return; // not converted to FE-I4, so do not use

  // assume cfgs is a 16-element array, element 0 will be merged in, all other contain info for FEs 1 to 15
//   std::map<std::string, int> types = cfgs[0]->getHistoTypes();
//   for(moduleIterator mi = modBegin(); mi != modEnd(); mi++){
//     for(std::map<std::string, int>::iterator it=types.begin(); it!=types.end(); it++){
//       bool found = false;
//       try{
// 	for(int chip=0;chip<16;chip++)
// 	  cfgs[chip]->getHisto((PixScan::HistogramType)it->second);
// 	found = true;
//       }catch(...){
// 	found = false;
//       }
//       if(found){
// 	PixScanHisto &hia = cfgs[0]->getHisto((PixScan::HistogramType)it->second);
// 	found = false;
// 	try{
// 	  hia[(*mi)->moduleId()][0][0][0].histo();
// 	  found = true;
// 	}catch(...){
// 	  found = false;
// 	}
// 	if(found){
// 	  Histo &hi = hia[(*mi)->moduleId()][0][0][0].histo();
// 	  for(int chip=1;chip<16;chip++){
// 	    Histo &hi_chip = cfgs[chip]->getHisto((PixScan::HistogramType)it->second)[(*mi)->moduleId()][0][0][0].histo();
// 	    for(int row=0;row<160;row++){
// 	      for(int col=0;col<18;col++){
// 		int rowc, colc;
// 		float xval, yval;
// 		PixelDataContainer::PixXY(chip, col, row, &xval, &yval);
// 		colc = (int) xval;
// 		rowc = (int) yval;
// 		double val = hi_chip(colc,rowc);
// 		hi.set(colc, rowc, val);
// 	      }
// 	    }
// 	  }
// 	} else{
// 	  found = false;
// 	  try{
// 	    hia[(*mi)->moduleId()][0][0].histo();
// 	    found = true;
// 	  }catch(...){
// 	    found = false;
// 	  }
// 	  if(found){
// 	    Histo &hi = hia[(*mi)->moduleId()][0][0].histo();
// 	    for(int chip=1;chip<16;chip++){
// 	      Histo &hi_chip = cfgs[chip]->getHisto((PixScan::HistogramType)it->second)[(*mi)->moduleId()][0][0].histo();
// 	      for(int row=0;row<160;row++){
// 		for(int col=0;col<18;col++){
// 		  int rowc, colc;
// 		  float xval, yval;
// 		  PixelDataContainer::PixXY(chip, col, row, &xval, &yval);
// 		  colc = (int) xval;
// 		  rowc = (int) yval;
// 		  double val = hi_chip(colc,rowc);
// 		  hi.set(colc, rowc, val);
// 		}
// 	      }
// 	    }
// 	  }
// 	}
//       }
//     }
//   }
}
void STPixModuleGroup::selectFe(int iFE, int modID)
{
  moduleIterator mBegin = modBegin();
  moduleIterator mEnd = modEnd();
  if(modID>=0 && module(modID)!=0){
    while((*mBegin)!=module(modID))
      mBegin++;
    if(mBegin!=modEnd())
      mEnd = mBegin + 1;
  }
  for(moduleIterator mi = mBegin; mi != mEnd; mi++){
    if(iFE>=0 && iFE<16){
      if(!(*mi)->restoreConfig("PreFEbyFEConfig")){ // full config has not been stored yet
	// store current cfg. for restoring it later
	(*mi)->storeConfig("PreFEbyFEConfig");
	// set all (relevant) DACs of all FE to zero
	for(std::vector<PixFe*>::iterator fea = (*mi)->feBegin(); fea != (*mi)->feEnd(); fea++){
	  // set all FEs to DAC=0
	  /* the following will not work for FE-I4
	  Config &cfg = (*fea)->config();
	  Config &globreg = cfg.subConfig("GlobalRegister_0/GlobalRegister");
	  int resVal = 0;
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_IVDD2"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_ID"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_IP"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_IP2"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_ITRIMTH"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_IF"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_ITRIMIF"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_ITH1"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_ITH2"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_IL"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["DAC_IL2"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["ENABLE_TIMESTAMP"], resVal);
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"]["ENABLE_CAP_TEST"], resVal);
	  for(int i=0;i<9;i++){
	    std::stringstream a;
	    a << i;
	    std::string cpen = "ENABLE_CP"+a.str();
	    WriteIntConf((ConfInt&)globreg["GlobalRegister"][cpen], resVal);
	  }
	  */
	}
	// store this config, too
	(*mi)->storeConfig("ResetConfig");
      }
      for(moduleIterator mi = mBegin; mi != mEnd; mi++){
	for(int i=0;i<16;i++){
	  if(i==iFE && (*mi)->pixFE(i)!=0)
	    (*mi)->pixFE(i)->restoreConfig("PreFEbyFEConfig");
	  else if ((*mi)->pixFE(i)!=0)
	    (*mi)->pixFE(i)->restoreConfig("ResetConfig");
	}
      }
    } else {
      for(moduleIterator mi = mBegin; mi != mEnd; mi++){
	(*mi)->deleteConfig("ResetConfig");
	(*mi)->restoreConfig("PreFEbyFEConfig");
	(*mi)->deleteConfig("PreFEbyFEConfig");
      }
    }
  }
}
void STPixModuleGroup::configSingleMod(int modID, int maskFE)
{
  PixModule *selMod = module(modID);
  Config &conf = selMod->config();
  if(conf["general"]["Active"].name()!="__TrashConfObj__")
    if(((ConfList&)conf["general"]["Active"]).sValue()!="TRUE") return; // module not active, nothing to do
  
  // set all modules except request inactive, store prev. active/inactive state
  std::map<int, bool> activeStates;
  for(moduleIterator mia = modBegin(); mia != modEnd(); mia++){
    Config &conf = (*mia)->config();
    activeStates.insert(std::make_pair((*mia)->moduleId(),((ConfList&)conf["general"]["Active"]).sValue()=="TRUE"));
    if((*mia)->moduleId()!=modID)
      WriteIntConf((ConfInt&)conf["general"]["Active"],0);
  }

  // set config for requested FEs
  selectFe(0,modID);
  selMod->pixFE(0)->restoreConfig("ResetConfig");
  for(int iFE=0; iFE<16; iFE++){
    if((maskFE>>iFE)&1 && selMod->pixFE(iFE)!=0)
      selMod->pixFE(iFE)->restoreConfig("PreFEbyFEConfig");
  }
  if(getPixCtrlStatus()==tOK){
    downloadConfig();
    
    // configure
    ThreadExecute(MTconfig);
    // wait till thread has finished
    while((m_ctrlThr->isRunning())){
      PixLib::sleep(10);
      m_app->processEvents();
    }
  }

  // set all modules back to their org. active state
  for(moduleIterator mi = modBegin(); mi != modEnd(); mi++){
    Config &conf = (*mi)->config();
    if((*mi)->moduleId()!=modID)
      WriteIntConf((ConfInt&)conf["general"]["Active"],(int)activeStates[(*mi)->moduleId()]);
  }
  if(getPixCtrlStatus()==tOK)
    downloadConfig();

  // restore org. FE config
  selectFe(-1,modID);
}
bool STPixModuleGroup::getModuleActive(int modID)
{
  PixModule *selMod = module(modID);
  if(selMod==0) return false;
  Config &conf = selMod->config();
  if(conf["general"]["Active"].name()!="__TrashConfObj__")
    return ((ConfList&)conf["general"]["Active"]).sValue()=="TRUE";
  else
    return false;
}
void STPixModuleGroup::setModuleActive(int modID, bool active)
{
  PixModule *selMod = module(modID);
  if(selMod==0) return;
  Config &conf = selMod->config();
  WriteIntConf((ConfInt&)conf["general"]["Active"],(int)active);
}
void STPixModuleGroup::setDcsState(int cmdId, int state){
  if(m_dcsStates[cmdId]!=0) m_dcsStates[cmdId] = state;
}
void STPixModuleGroup::setDcsRState(int cmdId, int state, double value){
  m_dcsRStates[cmdId] = std::make_pair(state, value);
}
void STPixModuleGroup::setGlobReg(int type, PixFe *fe){
  int length;
  bool unhandledType=false;
  Config &feconf = fe->config();
  ConfGroup &grgrp = feconf.subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"];
  // make sure we have a new seed for the random numbers
  srand ( time(NULL) );
  for(int i=0;i<grgrp.size();i++){
    std::string oName = grgrp[i].name();
    if(grgrp[i].type()==ConfObj::INT){
      oName.erase(0,std::string("GlobalRegister_").length());
      fe->getGlobRegisterLength(oName,length);
      int newVal=0; // that covers already ChipTest::ALLZERO
      switch(type){
      case ChipTest::RANDOM:
	newVal = (int)((double)rand()/(double)RAND_MAX*pow(2,(double)length));
	// don't break since action for ALLZERO is also applied to random
      case ChipTest::ALLZERO:
	// exclude registers that alter the i/o behaviour
	if(oName!="EN_PLL" && oName!="EN40M" && oName!="EN80M" && oName!="EN160M" && oName!="EN320M" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      case ChipTest::ALLONE_A:
	newVal = ((int)pow(2,(double)length))-1;
	if(oName=="PrmpVbp" || oName=="DisVbn")
	  newVal = 0;
	if(oName=="TdacVbp" || oName=="PrmpVbpRight" || oName=="Vthin" 
	   || oName=="DisVbn_CPPM" || oName=="Amp2Vbn" || oName=="Amp2VbpFol" 
	   || oName=="PrmpVbpTop" || oName=="Amp2Vbp" || oName=="FdacVbn" 
	   || oName=="Amp2Vbpf" || oName=="PrmpVbnFol" || oName=="PrmpVbpLeft" 
	   || oName=="PrmpVbpf" || oName=="PrmpVbnLcc" || oName=="Vthin_AltCoarse" || oName=="Vthin_AltFine")
	  newVal = 128;
	if(oName!="EN_PLL" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      case ChipTest::ALLONE_B:
	newVal = ((int)pow(2,(double)length))-1;
	if(oName=="PrmpVbp" || oName=="DisVbn")
	  newVal = 0;
	if(oName=="TdacVbp" || oName=="PrmpVbpRight" || oName=="Vthin" 
	   || oName=="DisVbn_CPPM" || oName=="Amp2Vbn" || oName=="Amp2VbpFol" 
	   || oName=="PrmpVbpTop" || oName=="Amp2Vbp" || oName=="FdacVbn" 
	   || oName=="Amp2Vbpf" || oName=="PrmpVbnFol" || oName=="PrmpVbpLeft" 
	   || oName=="PrmpVbpf" || oName=="PrmpVbnLcc" || oName=="Vthin_AltCoarse" || oName=="Vthin_AltFine")
	  newVal = 127;
	if(oName!="EN_PLL" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      case ChipTest::ALLONE_C:
	newVal = 0;
	if(oName=="PrmpVbp")
	  newVal = 128;
	if(oName!="EN_PLL" && oName!="EN40M" && oName!="EN80M" && oName!="EN160M" && oName!="EN320M" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      case ChipTest::ALLONE_D:
	newVal = 0;
	if(oName=="PrmpVbp")
	  newVal = 127;
	if(oName!="EN_PLL" && oName!="EN40M" && oName!="EN80M" && oName!="EN160M" && oName!="EN320M" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      case ChipTest::ALLONE_E:
	newVal = 0;
	if(oName=="DisVbn")
	  newVal = 128;
	if(oName!="EN_PLL" && oName!="EN40M" && oName!="EN80M" && oName!="EN160M" && oName!="EN320M" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      case ChipTest::ALLONE_F:
	newVal = 0;
	if(oName=="DisVbn")
	  newVal = 127;
	if(oName!="EN_PLL" && oName!="EN40M" && oName!="EN80M" && oName!="EN160M" && oName!="EN320M" 
	   && oName!="CLK1" && oName!="CLK0" && oName!="Clk2OutCnfg" && oName!="Stop_Clk"
	   && oName!="LVDSDrvEn" && oName!="LVDSDrvIref" && oName!="LVDSDrvVos" && oName!="no8b10b" 
	   && oName!="Conf_AddrEnable" && oName!="EmptyRecord" && oName!="PllIbias"  && oName!="PllIcp" 
	   && oName!="LVDSDrvSet06" && oName!="LVDSDrvSet12" && oName!="LVDSDrvSet30")
	  ((ConfInt&)grgrp[i]).setValue(newVal);
	break;
      default:
	unhandledType=true;
	break;
      }
    } else { // don't know what to do
      ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::setGlobReg : config item "+grgrp[i].name()+" is not of type INT");
      m_app->postEvent(this, tee);
    }
  }
  if(unhandledType){
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::setGlobReg : unhandled configuration type");
    m_app->postEvent(this, tee);
  }
}
void STPixModuleGroup::setPixlReg(int type, PixFe *fe){
  int maxval;
  bool unhandledType=false;
  Config &feconf = fe->config();
  ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];
  ConfGroup &maskgrp = feconf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
  // make sure we have a new seed for the random numbers
  srand ( time(NULL) );
  for(int i=0;i<trimgrp.size();i++){
    std::string oName = trimgrp[i].name();
    oName.erase(0,std::string("Trim_").length());
    if(trimgrp[i].type()==ConfObj::MATRIX && ((ConfMatrix&)trimgrp[i]).subtype()==ConfMatrix::U16){
      if(!fe->getTrimMax(oName,maxval)) maxval=1;
      ConfMatrix &mapobj = (ConfMatrix&)trimgrp[i];
      std::vector<unsigned short int> temp;
      ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
      std::vector<unsigned short int>::iterator it;
      int index=0;
      for(it=temp.begin(); it!=temp.end(); it++){
	switch(type){
	case ChipTest::ALLZERO:
	  (*it) = 0;
	  break;
	case ChipTest::ALLONE:
	  (*it) = maxval;
	  break;
	case ChipTest::ALLODD:
	  if(((index/80)%2)!=0)
	    (*it) = maxval;
	  else
	    (*it) = 0;
	  break;
	case ChipTest::ALLEVEN:
	  if(((index/80)%2)==0)
	    (*it) = maxval;
	  else
	    (*it) = 0;
	  break;
	case ChipTest::RANDOM:
	  (*it) = (int)((double)rand()/(double)RAND_MAX*(double)maxval);
	  break;
	default:
	  unhandledType=true;
	  (*it) = 0;
	  break;
	}
	index++;
      }
      ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
    }
  }
  for(int i=0;i<(int)maskgrp.size();i++){
    if(maskgrp[i].type()==ConfObj::MATRIX && ((ConfMatrix&)maskgrp[i]).subtype()==ConfMatrix::U1){
      ConfMatrix &mapobj = (ConfMatrix&)maskgrp[i];
      std::vector<bool> temp;
      ((ConfMask<bool> *)mapobj.m_value)->get(temp);
      std::vector<bool>::iterator it;
      int index=0;
      for(it=temp.begin(); it!=temp.end(); it++){
	switch(type){
	case ChipTest::ALLZERO:
	  (*it) = false;
	  break;
	case ChipTest::ALLONE:
	  (*it) = true;
	  break;
	case ChipTest::ALLODD:
	  (*it) = (((index/80)%2)!=0);
	  break;
	case ChipTest::ALLEVEN:
	  (*it) =(((index/80)%2)==0);
	  break;
	case ChipTest::RANDOM:
	  (*it) = (rand()>RAND_MAX/2);
	  break;
	default:
	  unhandledType=true;
	  (*it) = false;
	  break;
	}
	index++;
      }
      ((ConfMask<bool> *)mapobj.m_value)->set(temp);
    }
  }
  if(unhandledType){
    ThreadErrorEvent* tee = new ThreadErrorEvent("STPixModuleGroup::setPixlReg : unhandled configuration type");
    m_app->postEvent(this, tee);
  }
}
bool STPixModuleGroup::getCtrlInputStatus(int input){
  std::map<int, PixController::rxTypes> input_transl;
  input_transl[0] = PixController::RX0;
  input_transl[1] = PixController::RX1;
  input_transl[2] = PixController::RX2;
  input_transl[3] = PixController::EXT_TRG;
  
  if(input<0 || input >3) return false;
  
  return getPixController()->checkRxState(input_transl[input]);
}
void STPixModuleGroup::reloadCtrlCfg(){
  USBPixController *upc = dynamic_cast<USBPixController*>(getPixController());
  if(upc!=0)
  {
	  try{
		  upc->updateRegs();
	  }
	  catch (USBPixControllerExc &upcExc){
		  std::stringstream txt;
		  std::stringstream msg;
		  upcExc.dump(msg);
		  txt << "STPixModuleGroup::reloadCtrlCfg() : USBPixController-exception (grp. " << m_ctrlThr->getSTPixModuleGroup()->getName() <<") while updating regs. Reason: controller not initialized. Message: " << msg.str() << "\n";
		  ThreadLogEvent* tle = new ThreadLogEvent(txt.str());
		  m_app->postEvent(m_ctrlThr->getSTPixModuleGroup(), tle);
	  }
  }
}
void STPixModuleGroup::ctrlHwInfo(std::string &txt){
  if(getPixController()!=0) getPixController()->hwInfo(txt);
}

