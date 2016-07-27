/***************************************************************************
                          STRodCrate.cpp  -  description
                             -------------------
    begin                : Fri Apr 16 2004
    copyright            : (C) 2004 by jschumac
    email                : jschumac@silabsbc
    modifications        : 2005 by rottlaen, 2005 and 2006 by jgrosse
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <PixController/PixScan.h>
#include "STRodCrate.h"
#include "STPixModuleGroup.h"

#include <VmeInterface.h>
#ifndef NOTDAQ
#include <RCCVmeInterface.h>
#include <PixController/RodPixController.h>
#include <RodCrate/RodModule.h>
#endif
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/TurboDaqDB.h>
#include <PixController/PixScan.h>
#include <PixModuleGroup/PixModuleGroup.h>
#include <PixModule/PixModule.h>
#include <PixMcc/PixMcc.h>
#include <PixFe/PixFe.h>
#include <PixFe/PixFeExc.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixController/PixController.h>
#include <PixController/USBPixController.h>
#include <PixController/USBI3PixController.h>
#include <PixController/USB3PixController.h>
#include <PixBoc/PixBoc.h>
#include <PixDcs/SleepWrapped.h>
#include <PixDcs/PixDcs.h>
#include <PixDcs/USB_PixDcs.h>
#include <PixDcs/USBBIPixDcs.h>
#include <PixDcs/RS232PixDcs.h>
#ifdef HAVE_GPIB
#include <PixDcs/GPIBPixDcs.h>
#endif
#include <DBEdtEngine.h>

#include <SiLibUSB.h>
#include <ComTools.h>

#include <qmessagebox.h>
#include <qapplication.h>
#include <qthread.h>
#include <QTimer>
#include <qdatetime.h>
#include <TFile.h>

#include <sstream>
#ifdef WIN32
 #include <stdlib.h>
#else
 #include <unistd.h>
#endif

using namespace PixLib;
using namespace SctPixelRod;

Config trashConf("__TrashConfig__");

STRodCrate::STRodCrate() : m_options(trashConf){
  m_ID=-1; 
  m_vmeInterface=0; 
  m_SBCaddr=""; 
  m_app=0;
}
STRodCrate::STRodCrate( int ID, PixLib::Config &options, QApplication* application, QObject * parent, const char *SBCaddr )
  : QObject( parent ), m_ID(ID), m_SBCaddr(SBCaddr), m_options(options){
  m_vmeInterface=0;
  m_app = application;
  m_IFtype = tundefIF;
  m_USBinit = false;
  m_timer = new QTimer( this );
  m_filebusy = false;
  m_comInit = false;
  // the following doesn't work for linux at the moment, so don't bother to try
  //#ifdef WIN32
  connect(m_timer, SIGNAL(timeout()), this, SLOT(checkUsbEvent()) );
  //#endif
}
void STRodCrate::setupVme(){
  // if there is already an interface, delete it
  clearVme();

  try {
    // get VME interface or init. USB
    switch(m_IFtype){
    default:
    case tundefIF:
      break;
    case tUSBSys:
      if(!m_USBinit){
	InitUSB();
	m_USBinit=true;
      }
      m_timer->start(50);
      break;
    case tRCCVME:
#ifndef NOTDAQ
      m_vmeInterface = new RCCVmeInterface();
#endif
      break;
    }
    // init. RS232 interface
    if(!m_comInit){
      ComInit();
      m_comInit = true;
    }
  }
  catch (VmeException &v) {
    std::stringstream msg;
    msg << "STRodCrate::setupVme : VmeException:";
    msg << "  ErrorClass = " << v.getErrorClass();
    msg << "  ErrorCode = " << v.getErrorCode();
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    m_vmeInterface = 0;
  }
  catch (BaseException & exc){
    std::stringstream msg;
    msg << "STRodCrate::setupVme : Base exception during creation of VME interface: "<< exc; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    m_vmeInterface = 0;
  }
  catch (...){
    std::stringstream msg;
    msg << "STRodCrate::setupVme : Unknown exception during creation of VME interface."; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    m_vmeInterface = 0;
  }

}
void STRodCrate::clearVme(){
  // RS232 stuff
  if(m_comInit){
    ComExit();
    m_comInit = false;
  }
  if(m_USBinit){
    TermUSB();
  }
  delete m_vmeInterface; m_vmeInterface=0;
}
STRodCrate::~STRodCrate(){
  clear();
  clearVme();
  m_timer->stop();
}
bool STRodCrate::vmeOK(){
  switch(m_IFtype){
  default:
  case tundefIF:
    return true;
  case tUSBSys:
    return m_USBinit;
  case tRCCVME:
    return (m_vmeInterface!=0);
  }
}
/** returns the top part of the IP address of this SBC (i.e. its name)*/
std::string STRodCrate::getName(){
  std::string tmpStr = m_SBCaddr;
  int pos = tmpStr.find(".");
  if(pos!=(int)std::string::npos)
    tmpStr.erase(pos,tmpStr.length()-pos);
  return tmpStr;
}
/** Read property of vector <STPixModuleGroup *> m_pixModuleGroups. */
std::vector <STPixModuleGroup *> & STRodCrate::getSTPixModuleGroups(){
  return m_pixModuleGroups;
}
/** Read property of vector <STPixModuleGroup *> m_pixModuleGroups. */
std::vector <PixDcs *> & STRodCrate::getPixDcs(){
  return m_pixDcs;
}
/** This deletes all PixModuleGroups */
void STRodCrate::clear(){
  for(std::vector<PixDcsChan*>::iterator IT=m_dcsPrescanCfg.begin();IT!=m_dcsPrescanCfg.end();IT++){
    delete (*IT);
    *IT = 0;
  }
  m_dcsPrescanCfg.clear();

  // Only do anything if we have any module groups or DCS objects - otherwise we might create QT signal loops.

  // delete DCS first since USB devices might rely on module groups
  if(m_pixDcs.size()>0){
    for(std::vector<PixDcs *>::iterator i=m_pixDcs.begin(); i != m_pixDcs.end(); i++) {
      if((*i)->getDevType()==PixDcs::SUPPLY && (*i)->ReadState("")!="uninit." && 
	 (*i)->ReadState("")!="unknown" && (*i)->ReadState("")!="ERROR")
	(*i)->SetState("OFF"); // turn off everything before leaving...
      delete *i; *i = 0;
    }
    m_pixDcs.clear();
    emit groupListChanged();
  }

  if( m_pixModuleGroups.size() > 0 ) {
    for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
      delete *i; *i = 0;
    }
    m_pixModuleGroups.clear();
    emit groupListChanged();
  }
}
/** Loads PixelModuleGroups from inquire "decName in DB file "fname" */
void STRodCrate::loadDB( const char *fname, const char *decName )
{
  PixConfDBInterface * confDBInterface = DBEdtEngine::openFile(fname, false); 
  DBInquire *appInq=0, *root = confDBInterface->readRootRecord(1);
  try{
    std::vector<DBInquire*> inqVec;
    inqVec = confDBInterface->DBFindRecordByName(PixLib::BYDECNAME, root->getDecName()+
						 std::string(decName)+"/application");
    if(inqVec.size()==1)
      appInq = inqVec[0];
    else{
      emit errorMessage("STRodCrate::loadDB : couldn't find unique crate inquire.");
      delete confDBInterface;
      return;
    }
  }catch (BaseException & exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadDB : Base exception: "<< exc; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    delete confDBInterface;
    return;
  }catch(...){
    emit errorMessage("STRodCrate::loadDB : unknown exception while searching for crate inquire.");
    delete confDBInterface;
    return;
  }
  // loop over inquires in crate inquire and create a PixModuleGroup when an according entry is found
  for(recordIterator pmgIter = appInq->recordBegin();pmgIter!=appInq->recordEnd();pmgIter++){
    if((*pmgIter)->getName().find("PixModuleGroup")!=std::string::npos){
      loadModuleGroup(*pmgIter);
    }
    if((*pmgIter)->getName().find("PixDcs")!=std::string::npos){
      loadDcs(*pmgIter);
    }
  }
  delete confDBInterface;
  
  return;
}
bool STRodCrate::saveDB(const char *fname, bool activeOnly)
{
  PixConfDBInterface *myDB=0;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  try{
    myDB = DBEdtEngine::openFile(fname,true);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::saveDB : exception while re-opening DB file " << std::string(fname) << ": " << exc;
    emit errorMessage(msg.str());
    return false;
  }catch(...){
    QApplication::restoreOverrideCursor();
    emit errorMessage("STRodCrate::saveDB : unknown exception while re-opening DB file " + std::string(fname));
    return false;
  }

  bool gotErr = false;
  // get groups in current crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++){
    // have to get decorated name from module...
    std::string find_name = (*(*mgrIT)->modBegin())->moduleName();
    // remove module name
    find_name.erase(find_name.length()-1,1);
    int pos = (int)find_name.find_last_of("/");
    if(pos!=(int)std::string::npos)
      find_name.erase(pos+1,find_name.length()-pos);
    // then add the PixModuleGroup-specific bits for the full decorated name
    //      std::cout << "Saving group " << find_name << std::endl;
    find_name += "PixModuleGroup;1";
    std::vector<DBInquire*> grpInq;
    try{
      grpInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
    }catch(...){
      // dealt with later
      grpInq.clear();
    } 
    if(grpInq.size()==0){
      emit logMessage("STRodCrate::saveDB : Could not find group " + 
		      (*mgrIT)->getName() + " in DB object.");
      gotErr = true;
    } 
    if(grpInq.size()>1){
      emit logMessage("STRodCrate::saveDB : Got several DB objects for group " + 
		      (*mgrIT)->getName() + ". Will use first.");
      gotErr = true;
    }
    if((*mgrIT)->cfgEdited(40)){  // this group cfg was edited -> save
      Config &cfgGrp = (*mgrIT)->config();
      if(grpInq.size()>0){
	try{
	  cfgGrp.write(grpInq[0]);
	}catch(...){
	  emit logMessage("STRodCrate::saveDB : error writing config for group " + 
			  (*mgrIT)->getName() + " to DB object.");
	  gotErr = true;
	} 
      }
    }
    // save PixController config
    PixController *myROD = (*mgrIT)->getPixController();
    if(myROD!=0 && (*mgrIT)->cfgEdited(41)){
      Config &cfgROD = myROD->config();
      // have to get decorated name from module...
      std::string find_name = (*(*mgrIT)->modBegin())->moduleName();
      // remove module name
      find_name.erase(find_name.length()-1,1);
      int pos = (int)find_name.find_last_of("/");
      if(pos!=(int)std::string::npos)
	find_name.erase(pos+1,find_name.length()-pos);
      // then add the ROD-specific bits for the full decorated name
      find_name += (*mgrIT)->getRodName();
      //      std::cout << "Saving PixController " << find_name << std::endl;
      find_name += "/PixController;1";
      std::vector<DBInquire*> rodInq;
      try{
	rodInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
      }catch(...){
	// dealt with later
	rodInq.clear();
      } 
      if(rodInq.size()==0){
	emit logMessage("STRodCrate::saveDB : Could not find PixController " + 
			(*mgrIT)->getName() + " in DB object.");
	gotErr = true;
      } 
      if(rodInq.size()>1){
	emit logMessage("STRodCrate::saveDB : Got several DB objects for PixController " + 
			(*mgrIT)->getName() + ". Will use first.");
	gotErr = true;
      }
      if(rodInq.size()>0){
	try{
	  cfgROD.write(rodInq[0]);
	}catch(...){
	  emit logMessage("STRodCrate::saveDB : error writing config for PixController " + 
			  (*mgrIT)->getName() + " to DB object.");
	  gotErr = true;
	} 
      }
    }
#ifndef NOTDAQ
    PixBoc* myBOC = dynamic_cast<PixBoc*>((*mgrIT)->getPixBoc());
    if(myBOC!=0 && (*mgrIT)->cfgEdited(42)){
      Config &cfgBOC = *(myBOC->getConfig());
      // have to get decorated name from module...
      std::string find_name = (*(*mgrIT)->modBegin())->moduleName();
      // remove module name
      find_name.erase(find_name.length()-1,1);
      int pos = (int)find_name.find_last_of("/");
      if(pos!=(int)std::string::npos)
	find_name.erase(pos+1,find_name.length()-pos);
      // the add the BOC-specific bits for the full decorated name
      find_name += "OpticalBoc";
      //      std::cout << "Saving BOC " << find_name << std::endl;
      find_name += "/PixBoc;1";
      std::vector<DBInquire*> bocInq;
      try{
	bocInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
      }catch(...){
	// dealt with later
	bocInq.clear();
      } 
      if(bocInq.size()==0){
	emit logMessage("STRodCrate::saveDB : Could not find BOC " + 
			(*mgrIT)->getName() + " in DB object.");
	gotErr = true;
      } 
      if(bocInq.size()>1){
	emit logMessage("STRodCrate::saveDB : Got several DB objects for BOC " + 
			(*mgrIT)->getName() + ". Will use first.");
	gotErr = true;
      }
      if(bocInq.size()>0){
	try{
	  cfgBOC.write(bocInq[0]);
	}catch(...){
	  emit logMessage("STRodCrate::saveDB : error writing config for BOC " + 
			  (*mgrIT)->getName() + " to DB object.");
	  gotErr = true;
	} 
      }
      PixBoc::PixTx* myTx[4];
      std::string txnr;
      char txnumber[3];
      for(int i =0; i<4; i++)
	{
	  myTx[i] = dynamic_cast<PixBoc::PixTx*>((*mgrIT)->getPixBoc()->getTx(i));
	  if(myTx[i]!=0){
	    Config &cfgTx = *(myTx[i]->getConfigTx());
	    // have to get decorated name from module...
	    std::string find_name = (*(*mgrIT)->modBegin())->moduleName();
	    // remove module name
	    find_name.erase(find_name.length()-1,1);
	    int pos = (int)find_name.find_last_of("/");
	    if(pos!=(int)std::string::npos)
	      find_name.erase(pos+1,find_name.length()-pos);
	    // the add the BOC-specific bits for the full decorated name
	    find_name += "OpticalBoc";
	    sprintf(txnumber,"%d",i);
	    txnr=txnumber;
	    find_name += "/TxNr"+txnr;
	    find_name += "/PixTx;1";
	    std::vector<DBInquire*> txInq;
	    try{
	      txInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
	    }catch(...){
	      // dealt with later
	      txInq.clear();
	    } 
	    if(txInq.size()==0){
	      emit logMessage("STRodCrate::saveDB : Could not find Tx board for BOC of group " + 
			      (*mgrIT)->getName() + " in DB object.");
	      gotErr = true;
	    } 
	    if(txInq.size()>1){
	      emit logMessage("STRodCrate::saveDB : Got several DB objects for Tx board for BOC of group  " + 
			      (*mgrIT)->getName() + ". Will use first.");
	      gotErr = true;
	    }
	    if(txInq.size()>0){
	      try{
		cfgTx.write(txInq[0]);
	      }catch(...){
		emit logMessage("STRodCrate::saveDB : error writing config for Tx board for BOC " + 
				(*mgrIT)->getName() + " to DB object.");
		gotErr = true;
	      }
	    }
	    
	  }
	}
      PixBoc::PixRx* myRx[4];
      std::string rxnr;
      char rxnumber[3];
      for(int i =0; i<4; i++)
	{
	  myRx[i] = dynamic_cast<PixBoc::PixRx*>((*mgrIT)->getPixBoc()->getRx(i));
	  if(myRx[i]!=0){
	    Config &cfgRx = *(myRx[i]->getConfigRx());
	    // have to get decorated name from module...
	    std::string find_name = (*(*mgrIT)->modBegin())->moduleName();
	    // remove module name
	    find_name.erase(find_name.length()-1,1);
	    int pos = (int)find_name.find_last_of("/");
	    if(pos!=(int)std::string::npos)
	      find_name.erase(pos+1,find_name.length()-pos);
	    // the add the BOC-specific bits for the full decorated name
	    find_name += "OpticalBoc";
	    sprintf(rxnumber,"%d",i);
	    rxnr=rxnumber;
	    find_name += "/RxNr"+rxnr;
	    find_name += "/PixRx;1";
	    std::vector<DBInquire*> rxInq;
	    try{
	      rxInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
	    }catch(...){
	      // dealt with later
	      rxInq.clear();
	    } 
	    if(rxInq.size()==0){
	      emit logMessage("STRodCrate::saveDB : Could not find Rx board for BOC of group " + 
			      (*mgrIT)->getName() + " in DB object.");
	      gotErr = true;
	    } 
	    if(rxInq.size()>1){
	      emit logMessage("STRodCrate::saveDB : Got several DB objects for Rx board for BOC of group  " + 
			      (*mgrIT)->getName() + ". Will use first.");
	      gotErr = true;
	    }
	    if(rxInq.size()>0){
	      try{
		cfgRx.write(rxInq[0]);
	      }catch(...){
		emit logMessage("STRodCrate::saveDB : error writing config for Rx board for BOC " + 
				(*mgrIT)->getName() + " to DB object.");
		gotErr = true;
	      }
	    }
	  }
	}
    }
#endif
    for(PixModuleGroup::moduleIterator mi = (*mgrIT)->modBegin(); mi != (*mgrIT)->modEnd(); mi++){
      std::string find_name = (*mi)->moduleName();
      bool foundRec = false;
      for(recordIterator modIt = grpInq[0]->recordBegin(); modIt!=grpInq[0]->recordEnd(); modIt++){
	if((*modIt)->getName()=="PixModule" && (*modIt)->getDecName()==find_name){
	  if(activeOnly && !(*mgrIT)->getModuleActive((*mi)->moduleId())){
	    // erase records from inactive modules
	    grpInq[0]->eraseRecord(modIt);
	    myDB->DBProcess(grpInq[0],PixLib::COMMITREPLACE);
	  } else if((*mgrIT)->cfgEdited((*mi)->moduleId())){
	    // save module config
	    Config &modconf = (*mi)->config();
	    //	std::cout << "Saving module " << find_name << std::endl;
	    try{
	      //std::cout << "saving module " << (*mi)->moduleName() << std::endl;
	      modconf.write(*modIt);
	    }catch(...){
	      emit logMessage("STRodCrate::saveDB : error writing config for module " + 
			      (*mi)->moduleName() + " to DB object.");
	      gotErr = true;
	    } 
	  }
	  foundRec = true;
	  break;// found record, can stop loop
	}
      }
      if(!foundRec){
	emit logMessage("STRodCrate::saveDB : Could not find module " + 
			(*mi)->moduleName() + " in DB object.");
	gotErr = true;
      }
    }
    if(!gotErr){
      // clear all edit flags for this group
      (*mgrIT)->savedCfg();
    }
  }
  // save DCS objects
  // to do: use edit flag and save only when set, like for PixModule(Group) etc.
  for(std::vector <PixDcs*>::iterator di=m_pixDcs.begin(); di!=m_pixDcs.end(); di++){
    Config &dcsconf = (*di)->config();
    std::string find_name = (*di)->decName();
    //    std::cout << "Saving DCS object " << (*di)->name() << " into " << find_name<< std::endl;
    find_name += "PixDcs;1";
    std::vector<DBInquire*> dcsInq;
    try{
      dcsInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
    }catch(...){
      dcsInq.clear();
    } 
    if(dcsInq.size()==0){
      emit logMessage("STRodCrate::saveDB : Could not find DCS object " + 
		      (*di)->name() + " in DB object.");
      gotErr = true;
    } 
    if(dcsInq.size()>1){
      emit logMessage("STRodCrate::saveDB : Got several DB objects for DCS object " + 
		      (*di)->name() + ". Will use first.");
      gotErr = true;
    }
    if(dcsInq.size()>0){
      try{
	dcsconf.write(dcsInq[0]);
      }catch(...){
	emit logMessage("STRodCrate::saveDB : error writing config for DCS object " + 
			(*di)->name() + " to DB object.");
	gotErr = true;
      } 
    }
  }
  // close temporary file
  delete myDB;

  QApplication::restoreOverrideCursor();

  return gotErr;
}
bool STRodCrate::reloadModCfg(const char *fname){
  PixConfDBInterface *myDB=0;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  try{
    myDB = DBEdtEngine::openFile(fname);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::reloadModCfg : exception while opening DB file " << std::string(fname) << ": " << exc;
    emit errorMessage(msg.str());
    return false;
  }catch(...){
    QApplication::restoreOverrideCursor();
    emit errorMessage("STRodCrate::reloadModCfg : unknown exception while opening DB file " + std::string(fname));
    return false;
  }

  bool gotErr = false;
  // get groups in current crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++){
    for(PixModuleGroup::moduleIterator mi = (*mgrIT)->modBegin(); mi != (*mgrIT)->modEnd(); mi++){
      Config &modconf = (*mi)->config();
      std::string find_name = (*mi)->moduleName();
      //std::cout << "re-loading cfg. for module " << find_name << std::endl;
      find_name += "PixModule;1";
      std::vector<DBInquire*> modInq;
      try{
	modInq = myDB->DBFindRecordByName(PixLib::BYDECNAME, find_name); 
      }catch(...){
	modInq.clear();
      } 
      if(modInq.size()==0){
	emit logMessage("STRodCrate::reloadModCfg : Could not find module " + 
			(*mi)->moduleName() + " in DB object.");
	gotErr = true;
      } 
      if(modInq.size()>1){
	emit logMessage("STRodCrate::reloadModCfg : Got several DB objects for module " + 
			(*mi)->moduleName() + ". Will use first.");
	gotErr = true;
      }
      if(modInq.size()>0){
	try{
	  //std::cout << "loading inquire " << modInq[0]->getDecName() << std::endl;
	  if(!modconf.read(modInq[0])){
	    emit logMessage("STRodCrate::reloadModCfg : Config::read returns error for module " + 
			    (*mi)->moduleName() + ".");
	    gotErr = true;
	  }
	}catch(...){
	  emit logMessage("STRodCrate::reloadModCfg : error reading config for module " + 
			  (*mi)->moduleName() + " to DB object.");
	  gotErr = true;
	} 
      }
    }
  }
  delete myDB;
  QApplication::restoreOverrideCursor();
  return gotErr;
}
void STRodCrate::loadDcs( DBInquire *dcsInq ){
  PixDcs *dcs = 0;
  std::string dcsName;
  QApplication::setOverrideCursor( Qt::WaitCursor );
  try{
    // Create PixDcs object
    fieldIterator f = dcsInq->findField("ActualClassName");
    dcsInq->getDB()->DBProcess(f,READ, dcsName);
    void *interface = 0;
    // check if name  of a group with a USB controller was specified
    // if so, get the pointer and give it to PixDcs constructor
    f = dcsInq->findField("USBPixController");
    std::string ctrlName="???";
    if(f!=dcsInq->fieldEnd()){
      dcsInq->getDB()->DBProcess(f,READ, ctrlName);
      for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	   group != m_pixModuleGroups.end(); group++ ) {
	if((*group)->getName()==ctrlName) interface = (void*)(*group)->getPixController();
      }
    }
    if(interface!=0 || ctrlName=="???") 
      dcs = PixDcs::make(dcsInq, interface, dcsName);
    else
      emit errorMessage("STRodCrate::loadDcs : Can't find any USB interface for PixController "+ctrlName);
  }
  catch (PixDcsExc & pdexc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadDcs : PixDcs exception: ";
    pdexc.dump(msg); 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    return;
  }
  catch (BaseException & exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadDcs : Base exception: "<< exc; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    return;
  }
  catch (...){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadDcs : Unknown exception."; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    return;
  }
  m_pixDcs.push_back(dcs);
  QApplication::restoreOverrideCursor();
  emit groupListChanged();
  
  return;
}
void STRodCrate::loadModuleGroup( DBInquire *grpInq ){

  std::map <std::string, int> geomType;
  geomType["unknown"] = 0;
  geomType["Stave"]   = 1;
  geomType["Sector"]  = 2;

  STPixModuleGroup *grp = 0;
  QApplication::setOverrideCursor( Qt::WaitCursor );
  try{
    grp = new STPixModuleGroup(m_pixModuleGroups.size(), grpInq->getDB(), grpInq, m_app,
			       m_filebusy, m_options, this);
//     PixController *pc = grp->getPixController();
//     USBPixController *upc = dynamic_cast<USBPixController*>(pc);
//     USBI3PixController *ui3pc = dynamic_cast<USBI3PixController*>(pc);
//     USB3PixController *u3pc = dynamic_cast<USB3PixController*>(pc);
#ifndef NOTDAQ
    RodPixController *rpc = dynamic_cast<RodPixController*>(grp->getPixController());
    if(rpc!=0){
      if(m_IFtype!=tRCCVME){
	// VME hasn been initialised yet
	m_IFtype=tRCCVME;
	setupVme();
      }
    } else
#endif
      //if(upc!=0 || u3pc!=0 || ui3pc!=0){
    if(grp->getRodName().substr(0,3)=="USB"){
      if(m_IFtype != tUSBSys){
	m_IFtype = tUSBSys;
	setupVme();
      }
    }
    else
      m_IFtype = tundefIF;
  }
  catch (PixFeExc &fExc){
    std::stringstream txt;
    txt << "STRodCrate::loadModuleGroup : PixFe-exception level " << fExc.dumpLevel() << 
      ", type " << fExc.dumpType() << "\n";
    emit errorMessage(txt.str()); // sends to log container and makes engine open a warning box
    return;
  }
  catch (ConfigExc & cfExc){
    std::stringstream txt;
    txt << "STRodCrate::loadModuleGroup : Config-exception " << cfExc << "\n";
    emit errorMessage(txt.str()); // sends to log container and makes engine open a warning box
    return;
  }
  catch (VmeException &v) {
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadModuleGroup : VmeException:";
    msg << "  ErrorClass = " << v.getErrorClass();
    msg << "  ErrorCode = " << v.getErrorCode();
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    return;
  }
  catch (BaseException & exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadModuleGroup : Base exception: "<< exc; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    return;
  }
  catch (...){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << "STRodCrate::loadModuleGroup : Unknown exception."; 
    emit errorMessage(msg.str()); // sends to log container and makes engine open a warning box
    return;
  }
//   // forward groupChange signal
//   connect(grp, SIGNAL(groupChanged()), this, SIGNAL(groupListChanged()));
//   // forward updateModList signal
//   connect(grp, SIGNAL(updateModList()), this, SIGNAL(updateModList()));
  // forward log messages to log container
  connect(grp,SIGNAL(logMessage(std::string)), this, SIGNAL(logMessage(std::string)));
  // forward error messages to log container
  connect(grp,SIGNAL(errorMessage(std::string)), this, SIGNAL(errorMessage(std::string)));
  // forward error messages to log container
  connect(grp,SIGNAL(rodMessage(std::string, bool)), this, SIGNAL(rodMessage(std::string, bool)));
  // forward module test signals
  connect(grp,SIGNAL(sendRTStatus(const char*)), this, SIGNAL(sendRTStatus(const char*)));
  connect(grp,SIGNAL(currentModule(PixLib::PixModule *)), this, SLOT(recCurrentModule(PixLib::PixModule *)));
  connect(grp, SIGNAL(statusChanged()), this, SIGNAL(statusChanged()));
  // DCS hand shake-communication:
  // setting from group to crate
  connect(grp, SIGNAL(setDcs(std::string, std::string, double, int, STPixModuleGroup*)), this, SLOT(setDcs(std::string, std::string, double, int, STPixModuleGroup*)));
  // reading from group to crate
  connect(grp, SIGNAL(getDcs(std::string, int, int, STPixModuleGroup*)), this, 
	  SLOT(getDcs(std::string, int, int, STPixModuleGroup*)));
  // data SRAM testbeam mode
  connect(grp, SIGNAL(dataPending(std::vector<unsigned int *>*,int )), this, 
	  SIGNAL(dataPending(std::vector<unsigned int *>*, int )) );
  connect(grp, SIGNAL(eudaqScanStatus(int, bool, int, int)), this, 
	  SIGNAL(eudaqScanStatus(int, bool, int, int)) );
  // store group in resp. vector
  m_pixModuleGroups.push_back( grp );
  QApplication::restoreOverrideCursor();

  emit groupListChanged();
  
  return;
}
/** initialises the PixScan objects of all module groups */
int STRodCrate::setPixScan(PixScan &inPixScan){
  int error=0;
  // store raw file name in case it is needed below
  std::string orgRawFile = inPixScan.getSourceRawFile();
  for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
       group != m_pixModuleGroups.end(); group++ ) {
    // source scan: must add group name to raw file uniquely identify it in case several boards are being used
    // skip if no raw data file name if provided, otherwise this will accidentally be written
    if(m_pixModuleGroups.size()>1 && orgRawFile!=""){
      std::string rawFile = orgRawFile;
      // strip off extension and insert group name
      int epos = rawFile.find_last_of(".raw");
      if(epos != (int)std::string::npos){
	rawFile.erase(epos-3, rawFile.length()-epos+4);
      }
      rawFile += "_grp_"+(*group)->getName()+".raw";
      ((ConfString&)inPixScan.config()["general"]["sourceRawFile"]).m_value = rawFile;
    }
    error = (*group)->setPixScan(&inPixScan);
    if(error) return error;
  }
  // restore original raw name
  ((ConfString&)inPixScan.config()["general"]["sourceRawFile"]).m_value = orgRawFile;
  return error;
}


int STRodCrate::CtrlStatusSummary()
{
  if( m_pixModuleGroups.size() == 0 ) 
    {
      return -1;
    }
  else
    {
      int nRods = 0;
      for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) 
	{
	  if((*i)->getPixCtrlStatus()==tOK)
	    nRods++;
	}
      return nRods;
    }
}

vector<int> STRodCrate::GetBoardIDs()
{
  vector<int> boards;
  
  if( m_pixModuleGroups.size() == 0 ) 
    {
      return boards;
    }
  else
    {
      for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) 
		{
		  if((*i)->getPixCtrlStatus()==tOK)
		  {
			  boards.push_back( (*i) -> getPixController() -> getBoardID() );
		  }
		}
      return boards;
    }
}

std::map<int, std::string> STRodCrate::GetFeFlavours(){
  std::map<int, std::string> retval;
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
    Config &cfg = (*i)->module(0)->config();
    std::string feflv;
    if(cfg.name()!="__TrashConfig__" && cfg["general"].name()!="__TrashConfGroup__" &&
       cfg["general"]["FE_Flavour"].name()!="__TrashConfObj__")
      feflv = ((ConfList&)cfg["general"]["FE_Flavour"]).sValue();
    else
      feflv = "unnown";
    retval[(*i)->getPixController()->getBoardID()] = feflv;
  }
  return retval;
}

int STRodCrate::GetHitDiscCnfg()
{ 
  if( m_pixModuleGroups.size() == 0 ) 
    {
      return 0;
    }
  else
    {
      for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) 
		{
		  if((*i)->getPixCtrlStatus()==tOK)
		  {
			  // loop over modules
			  PixModuleGroup::moduleIterator mBegin = (*i) -> modBegin();
			  PixModuleGroup::moduleIterator mEnd = (*i) -> modEnd();

			  for(PixModuleGroup::moduleIterator mi = mBegin; mi != mEnd; mi++){
				  PixLib::PixModule::feIterator fe_it = (*mi)->feBegin();
				  try{
				    return (*fe_it)->readGlobRegister("HitDiscCnfg");
				  }catch(...){ //exception thrown on FEs w/o this register
				    return 0;
				  }
				  
				  //PixLib::PixFeI4Config	fei4conf = dynamic_cast <PixLib::PixFeI4Config> ((*fe_it)->feConfig());
			  }
		  }
		}
      return 0;
    }
}

void STRodCrate::readRodBuff(){
  if(vmeOK()){ // can't work w/o interface
    // Loop over module groups
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ) {
      (*group)->readRodBuff("");
    }
  } else{
    emit logMessage("STRodCrate::readRodBuff() : no VME interfaces found for crate "+getName()+
		    ", can't proceed");
  }
  return;
}

bool STRodCrate::RodProcessing()
{
  bool processing = false;
  if( m_pixModuleGroups.size() == 0 || (m_vmeInterface==0 && m_IFtype==tRCCVME)) 
    {
      return false;
    }

  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) 
    {  
      if((*i)->getProcessing()==true)
	{	 
	  processing = true;
	}   
    }
  
  return processing;
}


void STRodCrate::initRods()
{
  // send init command
  if( (m_vmeInterface!=0 && m_IFtype!=tUSBSys) || m_IFtype==tundefIF) { 
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTinitrods);  
      }
    }
  }
  if(m_IFtype==tUSBSys){
    // stop USB background checking, might interfere with what the driver is doing
    m_timer->stop();
    // make sure that pending calls don't interfere
    std::map<std::string, bool> grpState;
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	grpState[(*group)->getName()] = (*group)->initrods();
      }
    }
    // if uC code was loaded, this will scramble the handles, so update them
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      USBPixController *upc = dynamic_cast<USBPixController*>((*group)->getPixController());
      if(upc!=0){
	try{
	  // only call update if init was successful, otherwise we get a crash
	  if(grpState[(*group)->getName()]) upc->updateDeviceHandle();
	}catch(...){
	}
      }
      USBI3PixController *u3pc = dynamic_cast<USBI3PixController*>((*group)->getPixController());
      if(u3pc!=0){
	try{
	  // only call update if init was successful, otherwise we get a crash
	  if(grpState[(*group)->getName()]) u3pc->updateDeviceHandle();
	}catch(...){
	}
      }
    }
    // re-start timer if we're in USBPix mode
    m_timer->start(50);
  }
  return;
}
void STRodCrate::resetRods()
{
  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTresetrods);  
      }
    }
  }
  
  return;
}
void STRodCrate::initBocs()
{
  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTinitbocs);  
      }
    }
  }
  
  return;
}
void STRodCrate::configModules(int cfgType, int patternType, int DCs, std::string latch)
{
  std::vector<int> opts;
  opts.push_back(cfgType);
  opts.push_back(patternType);
  opts.push_back(DCs);

  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTconfig, opts, latch);  
      }
    }
  }
  
  return;
}
void STRodCrate::resetModules(int type)
{
  std::vector<int> opts;
  opts.push_back(type);

  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTresetmods, opts);  
      }
    }
  }
  
  return;
}
void STRodCrate::triggerModules()
{
  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTtrigger);  
      }
    }
  }
  
  return;
}
void STRodCrate::getSrvRec(){
  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTsrvrec);  
      }
    }
  }
  
  return;
}

void STRodCrate::setMcc(int opt)
{
  std::vector<int> opts;
  opts.push_back(opt);

  if(vmeOK()){
    
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      if((*group)->getPixCtrlStatus()!=tbusy){
	(*group)->ThreadExecute(MTsetmcc, opts);  
      }
    }
  }
  
  return;
}
void STRodCrate::scan(pixScanRunOptions scanOpts)
{
  if(vmeOK()){
    
    PixScan *cfg = scanOpts.scanConfig;
    bool anyDcsScanned =false;
    for(int il=0;il<3;il++){
      PixScan::ScanParam par = cfg->getLoopParam(il);
      std::string dcs_name="";
      // check if DCS is in use exclusively
      if(par==PixScan::DCS_VOLTAGE && cfg->getDcsChan()!="") dcs_name = cfg->getDcsChan();
      else if(par==PixScan::DCS_PAR1 && cfg->getDcsScanPar1ChannelName()!="" ) dcs_name = cfg->getDcsScanPar1ChannelName();
      else if(par==PixScan::DCS_PAR2 && cfg->getDcsScanPar2ChannelName()!="" ) dcs_name = cfg->getDcsScanPar2ChannelName();
      else if(par==PixScan::DCS_PAR3 && cfg->getDcsScanPar3ChannelName()!="" ) dcs_name = cfg->getDcsScanPar3ChannelName();
      if(cfg->getLoopActive(il) && dcs_name!="") anyDcsScanned = true;
    }
    // store DCS config before scan for restoring it later
    if(cfg->getRestoreModuleConfig() && anyDcsScanned){
      for(std::vector <PixDcs*>::iterator IT=m_pixDcs.begin(); IT!=m_pixDcs.end(); IT++){
	m_dcsState.push_back((*IT)->ReadState("xx"));
	for(std::vector <PixDcsChan*>::iterator ITC=(*IT)->chanBegin(); ITC!=(*IT)->chanEnd(); ITC++){
	  if((*IT)->getDevType()==PixDcs::SUPPLY){
	    m_dcsPrescanCfg.push_back(PixDcs::copyChan(*ITC));
	  } else
	    m_dcsPrescanCfg.push_back(0);
	}
      }
    }
    // start scan over all rods
    for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	 group != m_pixModuleGroups.end(); group++ ){
      // load DCS info if needed
      if(cfg->getDcsChan()!=""){
	for(PixModuleGroup::moduleIterator mit=(*group)->modBegin(); mit!=(*group)->modEnd(); mit++){
	  (*group)->setDcsChan((*mit)->moduleId(), 0); // set to NULL-pointer initially
	  if(((*mit)->config())["general"].name()!="__TrashConfGroup__" &&
	     ((*mit)->config())["general"]["Active"].name()!="__TrashConfObj__" &&
	     (((ConfList&)((*mit)->config())["general"]["Active"]).sValue()=="TRUE")){ 
	    std::string dcsName, modName = (*mit)->moduleName();
	    getDecNameCore(modName);
	    dcsName = cfg->getDcsChan();
	    if(cfg->getAddModNameToDcs()) dcsName += "_"+modName;
	    bool chanFnd = false;
	    for(std::vector <PixDcs*>::iterator IT=m_pixDcs.begin(); IT!=m_pixDcs.end(); IT++){
	      for(std::vector <PixDcsChan*>::iterator ITC=(*IT)->chanBegin(); ITC!=(*IT)->chanEnd(); ITC++){
		if((*ITC)->name()==dcsName){
		  (*group)->setDcsChan((*mit)->moduleId(), *ITC);
		  chanFnd = true;
		  break;
		}
	      }
	      if(chanFnd) break;
	    }
	    if(!chanFnd)
	      emit logMessage("STRodCrate::scan : DCS reading required, but could not find channel " + 
			dcsName + " for module " + modName);
	  }
	}
      }
      // actual scan start
      if((*group)->getPixCtrlStatus()!=tbusy) (*group)->ThreadExecute(MTscan, scanOpts);   
    }
  }
}
void STRodCrate::restoreAfterScan()
{
  // restore DCS config - if m_dcsPrescanCfg.size()>0 then DCS config was altered during scan
  // and org. config stored in STRodCrate::scan
  if(m_dcsPrescanCfg.size()>0){
    std::vector<std::string>::iterator ITS=m_dcsState.begin();
    for(std::vector <PixDcs*>::iterator IT=m_pixDcs.begin(); IT!=m_pixDcs.end(); IT++){
      for(std::vector <PixDcsChan*>::iterator ITC=(*IT)->chanBegin(); ITC!=(*IT)->chanEnd(); ITC++){
	for(std::vector<PixDcsChan*>::iterator ITD=m_dcsPrescanCfg.begin(); ITD!=m_dcsPrescanCfg.end(); ITD++){
	  if((*ITD)!=0 && (*ITD)->name()==(*ITC)->name()){
	    (*ITC)->config() = (*ITD)->config();
	    delete (*ITD);
	    (*ITD) = 0;
	  }
	}
      }
      try{
	if((*IT)->getDevType()==PixDcs::SUPPLY || (*IT)->getDevType()==PixDcs::POSITION) (*IT)->SetState("UPDATE");
      }catch(...){
      }
      if((*ITS)=="ON" || (*ITS)=="OFF") (*IT)->SetState(*ITS);
      ITS++;
    }
    m_dcsPrescanCfg.clear();
    m_dcsState.clear();
  }
}
void STRodCrate::setGR(std::string GRname, int GRval){
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++)
    (*mgrIT)->setGR(GRname, GRval);
  return;
}
void STRodCrate::setPR(std::string PRname, int PRval){
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++)
    (*mgrIT)->setPR(PRname, PRval);
  return;
}
int STRodCrate::setTFDACs(const char *fname, bool isTDAC)
{
  int not_found=0;
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++)
    not_found += (*mgrIT)->setTFDACs(fname, isTDAC);
  return not_found;
}
int STRodCrate::setMasks(std::vector<std::string> files, std::vector<std::string> histos, int mask, std::string logicOper){
//setMasks(const char *fname, const char *hname, int mask){
  int not_found=0;
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++)
    not_found += (*mgrIT)->setMasks(files, histos, mask, logicOper);
  return not_found;
}
void STRodCrate::setVcal(float charge, bool Chigh)
{
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++)
    (*mgrIT)->setVcal(charge,Chigh);
  return;
}
void STRodCrate::incrMccDelay(float delay, bool calib)
{
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++)
    (*mgrIT)->incrMccDelay(delay, calib);
  return;
}
void STRodCrate::disableFailed()
{
  // get groups in this crate and loop over them
  for(std::vector<STPixModuleGroup *>::iterator mgrIT = m_pixModuleGroups.begin(); mgrIT != m_pixModuleGroups.end();mgrIT++){
    (*mgrIT)->disableFailed();
  }
  return;
}
void STRodCrate::listRODs(std::vector<int> &rod_slots, std::vector<unsigned int> &revs){
  rod_slots.clear();
  revs.clear();
  if(m_vmeInterface==0) return; // can't work w/o VME interface
#ifndef NOTDAQ
  int i, address;
  for(i=5;i<22;i++){
    if (i!=13){ // don't even bother to try the TIM slot
      address= i <<24  ;
      try{
	SctPixelRod::RodModule rod(address,0x1000000, *m_vmeInterface, 4);
	rod.hpiLoad(SctPixelRod::HPIC, 0x10001);
	if( rod.hpiFetch(SctPixelRod::HPIC)==0x90009){
	  unsigned long rodserial=rod.hpiFetch(FPGA_STATUS_REG_6_REL_ADDR); // ROD Serial Number
	  rod_slots.push_back(i);
	  revs.push_back((rodserial>>16)&0xFF);
	}
      } catch(...){
      }
    }
  }
#endif
  return;
}
void STRodCrate::clearModuleInfo()
{
  modStatus modS;
  std::vector <STPixModuleGroup*> tmpGrp = m_pixModuleGroups;
  for( std::vector<STPixModuleGroup*>::iterator gi = tmpGrp.begin();
       gi!=tmpGrp.end(); gi++){
    for(PixModuleGroup::moduleIterator mi = (*gi)->modBegin(); mi != (*gi)->modEnd(); mi++){
      modS = (*gi)->getPixModuleStatus((*mi)->moduleId());
      if(modS.modStat!=tOK)
	modS.modStat = tunknown;
      if(modS.mccStat!=tOK && modS.mccStat!=tblocked)
	modS.mccStat = tunknown;
      modS.modMsg = "";
      modS.mccMsg = "";
      for(int chip=0;chip<16;chip++){
	if(modS.feStat[chip]!=tOK && modS.feStat[chip]!=tblocked)
	  modS.feStat[chip] = tunknown;
	modS.feMsg[chip] = "";
      }
      (*gi)->setPixModuleStatus((*mi)->moduleId(),modS);
    }
  }
}
void STRodCrate::abortScan()
{
  for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin();
       group != m_pixModuleGroups.end(); group++ ) {
    (*group)->abortScan();
  }
}
void STRodCrate::forceRead()
{
  for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin();
       group != m_pixModuleGroups.end(); group++ ) {
    (*group)->forceRead();
  }
}
int STRodCrate::getGrpNSteps(int iGrp, int i)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getNSteps(i);
  else
    return -1;
}
int STRodCrate::getGrpNMasks(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getNMasks();
  else
    return -1;
}
int STRodCrate::getGrpSRAMFillLevel(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getSRAMFillLevel();
  else
    return -1;
}
int STRodCrate::getGrpTriggerRate(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getTriggerRate();
  else
    return -1;
}
int STRodCrate::getGrpEvtRate(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getEvtRate();
  else
    return -1;
}
int STRodCrate::getGrpCurrFe(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getCurrFe();
  else
    return -1;
}
int STRodCrate::getGrpScanStatus(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getScanStatus();
  else
    return -1;
}
std::string STRodCrate::getGrpName(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getName();
  else
    return "";
}
std::string STRodCrate::getGrpDecName(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getDecName();
  else
    return "";
}
std::string STRodCrate::getModName(int iGrp, int iMod)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size() && m_pixModuleGroups[iGrp]->module(iMod)!=0)
    return m_pixModuleGroups[iGrp]->module(iMod)->moduleName();
  else
    return "";
}
void STRodCrate::getModules(int iGrp, std::vector< std::pair<std::string, int> > &list)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    for(PixModuleGroup::moduleIterator mi = m_pixModuleGroups[iGrp]->modBegin(); mi != m_pixModuleGroups[iGrp]->modEnd(); mi++)
      list.push_back(std::make_pair((*mi)->moduleName(), (*mi)->moduleId()));
    return;
  }else
    return; 
}
void STRodCrate::writeConfig(int iGrp, int iMod)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size() && getPixCtrlStatus(iGrp)==tOK){
    PixLib::PixModule *mod = m_pixModuleGroups[iGrp]->module(iMod);
    if(mod!=0) m_pixModuleGroups[iGrp]->getPixController()->writeModuleConfig(*mod);
      //mod->writeConfig();
  }
}
modStatus STRodCrate::getPixModuleStatus(int iGrp, int modID)
{
  static modStatus modS;
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getPixModuleStatus(modID);
  else
    return modS;
}
void STRodCrate::setPixModuleStatus(int iGrp, int modID, modStatus newStatus, bool emitSignal)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->setPixModuleStatus(modID, newStatus, emitSignal);
}
void STRodCrate::recCurrentModule(PixLib::PixModule *mod)
{
  if(mod==0) return;
  int grpID = ((STPixModuleGroup*)mod->getPixModGroup())->getID();
  emit currentModule(m_ID, grpID, mod->moduleId());
}
void STRodCrate::runLinkTest(int iGrp, int modID)
{
  if(vmeOK()){
    if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
      m_pixModuleGroups[iGrp]->runLinkTest(m_pixModuleGroups[iGrp]->module(modID),0);
    else{
      for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	   group != m_pixModuleGroups.end(); group++ ){
	if((*group)->getPixCtrlStatus()!=tbusy){
	  (*group)->ThreadExecute(MTlinktest);  
	}
      }
    }
  }
}
void STRodCrate::runChipTest(ChipTest *ct, bool resetMods, int iGrp, int modID){
  if(vmeOK()){
    if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
      m_pixModuleGroups[iGrp]->runChipTest(ct, resetMods, m_pixModuleGroups[iGrp]->module(modID));
    else{
      for( std::vector<STPixModuleGroup *>::iterator group = m_pixModuleGroups.begin(); 
	   group != m_pixModuleGroups.end(); group++ ){
	if((*group)->getPixCtrlStatus()!=tbusy){
	  (*group)->ThreadExecute(MTchiptest, ct, resetMods);  
	}
      }
    }
  }
}
void STRodCrate::selectFe(int iFE, int grpID, int modID)
{
  int iGrpStart = 0;
  int iGrpStop = m_pixModuleGroups.size();
  if(grpID>=0 && grpID<(int)m_pixModuleGroups.size()){
    iGrpStart = grpID;
    iGrpStop = grpID+1;
  }
  for(int iGrp = iGrpStart; iGrp<iGrpStop; iGrp++){
    m_pixModuleGroups[iGrp]->selectFe(iFE, modID);
    if(m_pixModuleGroups[iGrp]->getPixCtrlStatus()==tOK)
      m_pixModuleGroups[iGrp]->downloadConfig();
  }
}
void STRodCrate::configSingleMod(int iGrp, int modID, int maskFE)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->configSingleMod(modID, maskFE);
}
bool STRodCrate::getModuleActive(int iGrp, int modID)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getModuleActive(modID);
  else
    return false;
}
void STRodCrate::setModuleActive(int iGrp, int modID, bool active)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->setModuleActive(modID, active);
}
int STRodCrate::hasCtrl(int iGrp)
{ 
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    PixController *pc = m_pixModuleGroups[iGrp]->getPixController();
    if(pc!=0){
#ifndef NOTDAQ
      if(dynamic_cast<RodPixController *>(pc)!=0)
 	return 1;
#endif
      if(dynamic_cast<USBPixController *>(pc)!=0)
	return 2;
      // any other type
      return 3;
    }
  }
  return 0;
}
#ifndef NOTDAQ
bool STRodCrate::hasBoc(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return (m_pixModuleGroups[iGrp]->getPixBoc()!=0);
  else
#else
bool STRodCrate::hasBoc(int)
{
#endif
    return false;
}
#ifndef NOTDAQ
StatusTag STRodCrate::getPixBocStatus(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getPixBocStatus();
  else
#else
StatusTag STRodCrate::getPixBocStatus(int)
{
#endif
    return tunknown;
}
StatusTag STRodCrate::getPixCtrlStatus(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getPixCtrlStatus();
  else
    return tunknown;
}
void STRodCrate::ctrlHwInfo(int iGrp, std::string &txt){
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->ctrlHwInfo(txt);
  return;
}
void STRodCrate::setPixCtrlStatus(int iGrp, StatusTag status)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->setPixCtrlStatus(status);
}
bool STRodCrate::getCtrlInputStatus(int iGrp, int input){
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->getCtrlInputStatus(input);
  else
    return false;
}
PixLib::Config& STRodCrate::getPixModuleConf(int iGrp, int modID)
{
  static Config dummyCfg("dummymod");
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    PixLib::PixModule *mod = m_pixModuleGroups[iGrp]->module(modID);
    if(mod!=0) return mod->config();
    return dummyCfg;
  }
  return dummyCfg;
}
PixLib::Config& STRodCrate::getPixModuleChipConf(int iGrp, int modID, int chipID)
{
  static Config dummyCfg("dummychip");
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    PixLib::PixModule *mod = m_pixModuleGroups[iGrp]->module(modID);
    if(mod!=0){
      if(chipID>=0 && chipID<16){
	if(mod->pixFE(chipID)!=0)
	  return mod->pixFE(chipID)->config();
	else
	  return dummyCfg;
      }else if(chipID==16){
	if(mod->pixMCC()!=0)
	  return mod->pixMCC()->config();
	else
	  return dummyCfg;
      }else
	return dummyCfg;
    }
    return dummyCfg;
  }
  return dummyCfg;
}
PixLib::Config& STRodCrate::getPixCtrlConf(int iGrp)
{
  static Config dummyCfg("dummyctrl");
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    PixLib::PixController *ctrl = m_pixModuleGroups[iGrp]->getPixController();
    if(ctrl!=0) return ctrl->config();
    return dummyCfg;
  }
  return dummyCfg;
}
#ifndef NOTDAQ
PixLib::Config& STRodCrate::getPixBocConf(int iGrp)
{
  static Config dummyCfg("dummyboc");
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    PixLib::PixBoc *boc = m_pixModuleGroups[iGrp]->getPixBoc();
    if(boc!=0) return *(boc->getConfig());
    return dummyCfg;
  }
#else
PixLib::Config& STRodCrate::getPixBocConf(int)
{
  static Config dummyCfg("dummyboc");
#endif
  return dummyCfg;
}
PixLib::Config& STRodCrate::getGrpConf(int iGrp)
{
  static Config dummyCfg("dummygrp");
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return m_pixModuleGroups[iGrp]->config();
  return dummyCfg;
}
void STRodCrate::editedCfg(int iGrp, int ID)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->editedCfg(ID);
}
void STRodCrate::editedCfgAllMods(int iGrp)
{
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->editedCfgAllMods();
}
void STRodCrate::initDcs(){
  for(std::vector <PixDcs*>::iterator IT=m_pixDcs.begin(); IT!=m_pixDcs.end(); IT++){
    try{
      (*IT)->initHW();
    }catch(PixDcsExc &pde){
      std::stringstream msg;
      pde.dump(msg);
      emit errorMessage("STRodCrate::initDcs : PixDcs exception while initialising "+(*IT)->name()+": "+msg.str());
    }catch(...){
      emit errorMessage("STRodCrate::initDcs : unknown exception while initialising " + (*IT)->name());
    }
  }
}
void STRodCrate::setDcs(std::string chanName, std::string parName, double value, int cmdId, STPixModuleGroup *grp){
	bool foundIt = false;	//channel with the name chanName was found
	for(std::vector <PixDcs*>::iterator IT=m_pixDcs.begin(); IT!=m_pixDcs.end(); IT++){
		for(std::vector<PixDcsChan*>::iterator CIT=(*IT)->chanBegin(); CIT!=(*IT)->chanEnd(); CIT++){
			if(chanName==(*CIT)->name()){
				foundIt = true;
				//only send parameter if DCS device is in a "healthy" state
				std::string state = (*IT)->ReadState("");
				if(        state != "ON" 
					&& state != "ON*" 
					&& state != "OFF" 
					&& state != "OK"
					&& state != "ALL OFF"
					&& state != "ALL ON")
				{	
					grp->setDcsState(cmdId, 2);
				}else{
					try{
						(*CIT)->SetParam(parName, value);
						grp->setDcsState(cmdId, 1);
					}
					catch(PixDcsExc){
						grp->setDcsState(cmdId, 2);
					}
				}
				return;
			}
		}
	}
	if(!foundIt){
		grp->setDcsState(cmdId, 2);
	}
	return;
}
void STRodCrate::getDcs(std::string chanName, int rtype_in, int cmdId, STPixModuleGroup *grp){
  PixScan::DcsReadMode rtype = (PixScan::DcsReadMode) rtype_in;
  for(std::vector <PixDcs*>::iterator IT=m_pixDcs.begin(); IT!=m_pixDcs.end(); IT++){
    for(std::vector<PixDcsChan*>::iterator CIT=(*IT)->chanBegin(); CIT!=(*IT)->chanEnd(); CIT++){
      if(chanName==(*CIT)->name()){
	double val = 0.;
	try{
	  std::string rtypes;
	  switch(rtype){
	  case PixScan::VOLTAGE:
	    rtypes = "voltage";
	    break;
	  case PixScan::CURRENT:
	    rtypes = "current";
	    break;
	  default:
	    rtypes = "unknown";
	  }
	  val = (*CIT)->ReadParam(rtypes);
	}catch(...){
	  grp->setDcsRState(cmdId, 2, val);
	}
	grp->setDcsRState(cmdId, 1, val);
	return;
      }
    }
  }
  return;
}
void STRodCrate::checkUsbEvent(){
  if(OnDeviceChange()){
    cout << "INFO: STRodCrate::checkUsbEvent: USB update event detected " << endl;
    for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
      try{
	USBPixController *upc = dynamic_cast<USBPixController*>((*i)->getPixController());
	if(upc!=0) upc->updateDeviceHandle();
	USBI3PixController *ui3pc = dynamic_cast<USBI3PixController*>((*i)->getPixController());
	if(ui3pc!=0) ui3pc->updateDeviceHandle();
	USB3PixController *u3pc = dynamic_cast<USB3PixController*>((*i)->getPixController());
	if(u3pc!=0) u3pc->updateDeviceHandle();
	if((*i)->getPixCtrlStatus()==tproblem) (*i)->setPixCtrlStatus(tblocked);
      }catch (USBPixControllerExc & upcexc){
	std::stringstream msg;
	upcexc.dump(msg);
	emit logMessage("STRodCrate::checkUsbEvent() : USBPixController-exception (grp. " + 
			(*i)->getName() + "): "  + msg.str());
	(*i)->setPixCtrlStatus(tproblem);
      }catch (USBI3PixControllerExc & u3pcexc){
	std::stringstream msg;
	u3pcexc.dump(msg);
	emit logMessage("STRodCrate::checkUsbEvent() : USBI3PixController-exception (grp. " + 
			(*i)->getName() + "): "  + msg.str());
	(*i)->setPixCtrlStatus(tproblem);
      }catch (...){
	emit logMessage("STRodCrate::checkUsbEvent() : unknown exception (grp. " + 
			(*i)->getName() + ")");
	(*i)->setPixCtrlStatus(tproblem);
      }
    }
  }
}
void STRodCrate::getSrvRecRes(int iGrp, std::vector<int> &srvRec){
  srvRec.clear();
  //srvRec.resize(m_pixModuleGroups[iGrp]->m_srvCounts.size()); // service records have 32 elements
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
    //for(unsigned int i=0;i<m_pixModuleGroups[iGrp]->m_srvCounts.size();i++)
    srvRec = m_pixModuleGroups[iGrp]->m_srvCounts;
  }
  return;
}
void STRodCrate::getGADCRes(int iGrp, std::vector<int> &values){

  values.clear();
//   values.resize(m_pixModuleGroups[iGrp]->m_GADCValues.size()); // GADC result has currently 3 elements: status, select and GADCout
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size()){
//     for(unsigned int i=0;i<m_pixModuleGroups[iGrp]->m_GADCValues.size();i++)
      values = m_pixModuleGroups[iGrp]->m_GADCValues;
  }
  return;
}
void STRodCrate::reloadCtrlCfg(int iGrp){
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->reloadCtrlCfg();
  return;
}
void STRodCrate::setFeMode(int mode){
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
    switch(mode){
    case 0:
    default:
      (*i)->getPixController()->setFEConfigurationMode();
      break;
    case 1:
      (*i)->getPixController()->setFERunMode();
      break;
    }
  }
  return;
}

void STRodCrate::readEPROM(){
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
    (*i)->getPixController()->readEPROM();
  }
  return;
}

void STRodCrate::burnEPROM(){
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
    (*i)->getPixController()->burnEPROM();
  }
  return;
}

void STRodCrate::readGADC(int type){
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
    (*i)->getPixController()->readGADC(type, (*i)->m_GADCValues, -1);
  }
  return;
}

void STRodCrate::sendGlobalPulse(int length){
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++) {
    (*i)->getPixController()->sendGlobalPulse(length);
  }
  return;
}

#ifdef WIN32 // linux is using a differen storage mechanism, so don't allow to execute
void STRodCrate::saveScanIfDone(pixScanRunOptions scanOpts){
  for(std::vector<STPixModuleGroup *>::iterator i=m_pixModuleGroups.begin(); i != m_pixModuleGroups.end(); i++){
    if((*i)->getScanStatus()==3){ // grp. is ready for saving; will be set to 1 after call to processPixScanHistos
      std::stringstream msg;
      // try to catch any kind of problem - avoids uncontrolled crashes (hopefully)
      try{
	(*i)->processPixScanHistos(scanOpts);
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
	emit logMessage("STRodCrate::saveScanIfDone: Exception " + msg.str() +
			" not caught during execution with group "+
			(*i)->getName()+ " at " + 
			std::string(QDateTime::currentDateTime().toString("hh:mm:ss on MM-dd-yyyy").toLatin1().data())+"\n" );
	// TFile might still be open - close if gFile is not NULL
	try{
	  if(gFile!=0) gFile->Close();
	}catch(...){} // nothing else to try, give up...
	// call to processPixScanHistos failed, set status to 1=finished manually
	(*i)->setScanStatus(1); 
      }
    }
  }
#else
void STRodCrate::saveScanIfDone(pixScanRunOptions){
#endif
}
void STRodCrate::readUsbPixDcs(){
  bool scanRuns = true;
  for(int iGrp=0;iGrp<nGroups();iGrp++) if(getGrpScanStatus(iGrp)!=0) scanRuns = false;
  if(scanRuns){
    m_readyForDcs++;
    if(m_readyForDcs==3){
      for(std::vector <PixDcs *>::iterator pdIT = m_pixDcs.begin(); pdIT!=m_pixDcs.end(); pdIT++){
	USBPixDcs *upd = dynamic_cast<USBPixDcs*>(*pdIT);
	if(upd!=0){
	  STPixModuleGroup &mgr = (STPixModuleGroup&)(upd->getCtrl()->getModGroup());
	  for(std::vector<PixDcsChan*>::iterator dcsc = (*pdIT)->chanBegin(); dcsc!=(*pdIT)->chanEnd(); dcsc++){
	    try{
	      dcsReading readings;
	      readings.chanName = (*dcsc)->name();
	      readings.volts = (*dcsc)->ReadParam("voltage");
	      readings.curr = (*dcsc)->ReadParam("current");
	      mgr.m_scanDcsReadings.push_back(readings);
	    }catch(...){
	    }
	  }
	}
      }
    }
  }
}
void STRodCrate::readPixDcsErr(){
  for(std::vector<PixDcs *>::iterator dcs=m_pixDcs.begin(); dcs != m_pixDcs.end(); dcs++) {
    std::string errTxt;
    (*dcs)->ReadError(errTxt);
    if(errTxt!="") emit errorMessage("from DCS device "+(*dcs)->name()+": "+errTxt);
  }
}
void STRodCrate::setModHVmask(int iGrp, int modId, bool on){
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    m_pixModuleGroups[iGrp]->setModHVmask(modId, on);
}
int STRodCrate::getModHVmask(int iGrp){
  if(iGrp>=0 && iGrp<(int)m_pixModuleGroups.size())
    return     m_pixModuleGroups[iGrp]->getModHVmask();
  else
    return 0;
}
