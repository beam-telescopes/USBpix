/***************************************************************************
                      STPixModuleGroup.h  -  description
                             -------------------
    begin                : Fri 12 Aug 2005
    copyright            : (C) 2005 by Joern Grosse-Knetter
    email                : Joern.Grosse-Knetter@uni-bonn.de
    modifications        : Iris Rottlaender
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STPIXMODULEGROUP_H
#define STPIXMODULEGROUP_H

#include <PixModuleGroup/PixModuleGroup.h>
#include "STControlEngine.h"

#include <QObject>
#include <QEvent>
#include <QThread>
#include <QApplication>
#include <QMutex>

#include <string>

struct modGeometry{
  int type;
  int staveID;
  int position;
};

struct dcsReading{
  double volts;
  double curr;
  std::string chanName;
};

enum ThreadTag {

  MTundecided,
  MTscan,
  //  MTscaninit,
  MTinitrods,
  MTinitbocs,
  MTtrigger,
  MTconfig,
  MTresetmods,
  MTsetmcc,
  MTresetrods,
  MTlinktest,
  MTchiptest,
  MTsrvrec
};

namespace PixLib {
  
  class PixConfDBInterface;
  class DBInquire;
  class PixScan;
  class PixBoc;
  class PixFe;
}

class ChipTest;

/**This class represents one PixModuleGroup with some functional extension needed for STcontrol usage only
  *@author Joern Grosse-Knetter
  */

class STPixModuleGroup : public QObject, public PixLib::PixModuleGroup {

  friend class CtrlThread;
  friend class ScanThread;

  Q_OBJECT

 public:

  // Subclass using QThread used for parallel execution of a USBPixController-scan to allow for checks
  // of scan progress during scan execution
  // Author: Joern Grosse-Knetter
  
  class ScanThread : public QThread {

  friend class CtrlThread;
  friend class STPixModuleGroup;

  public:
    ScanThread(STPixModuleGroup& group, QApplication* application); 
    void setPixScan(PixLib::PixScan *ps){m_ps = ps;};
    void run() Q_DECL_OVERRIDE ;

  private:
    STPixModuleGroup& m_STPixModuleGroup;
    PixLib::PixScan *m_ps;
    QApplication* m_app;

  };
  
  // Subclass using QThread used for parallel Scan support, Rod initialising and other Tasks from Tool panel
  // Author: Iris Rottlaender
  
  class CtrlThread : public QThread {

  friend class STPixModuleGroup;
  friend class ScanThread;

  public:
    CtrlThread(STPixModuleGroup& group, QApplication* application); 
    void run() Q_DECL_OVERRIDE ;
    STPixModuleGroup* getSTPixModuleGroup(){return m_STPixModuleGroup;};
    void setThreadTag(ThreadTag tag){threadtag=tag; return;};
    void setTagOpt(std::vector<int> opts);
    void setTagStrg(std::string strg){m_tagStrg = strg; return;};
    void setScanOpts(pixScanRunOptions runOpts){scanOpts = runOpts;};
    void setCtOpts(ChipTest *ct, bool resetMods){m_ct = ct; m_resetMods=resetMods;};

  private:
    STPixModuleGroup* m_STPixModuleGroup;
    ThreadTag threadtag;
    std::vector<int> m_tagOpt;
    std::string m_tagStrg;
    int m_checkScanCount;
    QApplication* m_app;
    pixScanRunOptions scanOpts;
    ChipTest *m_ct;
    bool m_resetMods;
    ScanThread *m_scanThr;

  protected:
    void scan();
    //    void scaninit();
    void initrods();
    void initbocs();
    void trigger();
    void getSrvRec();
    void config();
    void resetmods();
    void setmcc();
    void resetrods();
    void runLinkTest();
    void runChipTest();
    void readRodBuff(std::string msg);
    // handling of DCS communication
    void setDcsScanVal(int loop, PixLib::PixScan *scn);
    void getDcsReadings(PixLib::PixScan *scn);
  
  };

  STPixModuleGroup(int ID, PixLib::PixConfDBInterface *db, PixLib::DBInquire *dbInquire, QApplication* application, bool &filebusy, 
		   PixLib::Config &options, QObject * parent=0);
  STPixModuleGroup(int ID, QApplication* application, bool &filebusy, PixLib::Config &options, QObject * parent=0); 
  ~STPixModuleGroup(); 
  
  // holding pointer to thread object, so it can be deleted properly
  CtrlThread *m_ctrlThr;  
  // contains service records
  std::vector<int> m_srvCounts;
  // containes GADC read values (GR40): status, select and GADCout
  std::vector<int> m_GADCValues;
  // for storage of dcs readings
  std::vector<dcsReading> m_scanDcsReadings;
  
  int  getID(){return m_ID;};
  int  getNSteps(int i);
  int  getNMasks(){return m_nMasks;}
  bool getSRAMReadoutReady(){return m_currSRAMReadoutReady;}
  bool getSRAMFull(){return m_currSRAMFull;}
  bool getMeasurementPause(){return m_currMeasurementPause;}
  bool getMeasurementRunning(){return m_currMeasurementRunning;}
  int  getSRAMFillLevel(){return m_currSRAMFillLevel;}
  int  getTriggerRate(){return m_currTriggerRate;}
  int  getEvtRate(){return m_currEvtRate;}
  int  getCurrFe(){return m_currFe;}
  int  getScanStatus(){return ScanStatus;};
  void setScanStatus(int status){ScanStatus=status;};
  bool getProcessing(){return processing;};
  std::string getDecName()     { return m_decName; };
  void setDcsState(int cmdId, int state);
  void setDcsRState(int cmdId, int state, double value);

  PixLib::Config& getOptions(){return m_options;};

  //static functions used for global testbeam readout
  static bool getGlobalReadout() {return STPixModuleGroup::global_readout; }
  static void disableGlobalReadout(){STPixModuleGroup::global_readout=false;}
  static QMutex* readoutFlagMutex;
  // needed for handling of ThreadErrorEvents and ThreadLogEvents
  void customEvent( QEvent * e );

  // for serialization of writing to file.
  static QMutex* writeToFileMutex;
  
 public slots:

  /** same as initrods in the thread to allow un-threaded calls which
      is needed for USBPix in case the uC code is written */
  bool initrods();
  /** Read status of controller associated to this group */
  StatusTag getPixCtrlStatus(){ return m_CtrlStatus;};
  /** Read hardware info string from PixController */
  void ctrlHwInfo(std::string &txt);
  /** only for USBPix: get input status of ctrl */
  bool getCtrlInputStatus(int input);
  /** Change status of controller associated to this group */
  void setPixCtrlStatus(StatusTag newStatus){m_CtrlStatus=newStatus; emit statusChanged();};
  /** Read status of BOC associated to this group */
  StatusTag getPixBocStatus(){return m_BocStatus;};
  /** Change status of BOC associated to this group */
  void setPixBocStatus(StatusTag newStatus){m_BocStatus=newStatus; emit statusChanged();};
  /** read buffers of the ROD */
  void readRodBuff(std::string msg);
  /** copy the content of a PixScan object to the one owned by this group */
  int setPixScan(PixLib::PixScan *inPixScan);
  /** returns a ref. to the PixScan object to the one owned by this group */
  PixLib::PixScan* getPixScan(){return m_PixScan;};
  /** returns pointer to the loop status array */
  bool* getLoopStatus(){return m_pixScanLoop;};
  // start Thread
  void ThreadExecute(ThreadTag tag);
  void ThreadExecute(ThreadTag tag, std::vector<int> opts);
  void ThreadExecute(ThreadTag tag, std::vector<int> opts, std::string latch);
  void ThreadExecute(ThreadTag tag, pixScanRunOptions runOpts);
  void ThreadExecute(ThreadTag tag, ChipTest *ct, bool resetMod);
  /* set GR of all FEs of all modules to a new value; GR identified by name */
  void setGR(std::string GRname, int GRval);
  /* set PR of all FEs of all modules to a new value; PR identified by name */
  void setPR(std::string PRname, int PRval);
  /* set TDACs of all modules in this groups to what is found in specified file */
  int setTFDACs(const char*, bool isTDAC);
  /*! set selected masks from a histogram from a RootDB file */
  int setMasks(std::vector<std::string> files, std::vector<std::string> histos, int, std::string logicOper);
  /* set VCAL of all FEs on all modules to a value corresponding to requested charge */
  void setVcal(float charge, bool Chigh);
  /*! de-/increment MCC delay of all modules in all groups to a value corresponding to requested value
   *  if "calib" is true, the value will be converted from ns to MCC delay; if false, the value as such
   *  will be used
   */
  void incrMccDelay(float delay, bool calib);
  /* disable all modules that have a "failed" state */
  void disableFailed();
  // write scan results to file
  void processPixScanHistos(pixScanRunOptions scanOpts);
  /* allows scan abort from outside */
  void abortScan(){m_abortScan = true;};
  /* allows scan abort from outside */
  bool doAbortScan(){return m_abortScan;};
  /* allows readout initiation from outside */
  void forceRead(){m_forceRead = true;};
  /* allows scan abort from outside */
  bool doForceRead(){return m_forceRead;};
  /** Read status of a module if found. */
  modStatus getPixModuleStatus(int modAddr);
  /** Change property of entry in map <StatusTag> m_pixModuleSTatus(Info). */
  void setPixModuleStatus(int modAddr, modStatus newStatus, bool emitSignal=true);
  /** run link test; if mod is not NULL, then just run on that module */
  void runLinkTest(PixLib::PixModule *mod, CtrlThread *thread);
  /** run a chip test; if mod is not NULL, then just run on that module */
  void runChipTest(ChipTest*ct, bool resetMods, PixLib::PixModule *mod);
  /** check if cfg was edited - ID is module ID or 40,41,42 for group, ROD, BOC */
  bool cfgEdited(int ID);
  /** tell us that cfg of object was edited  - ID is module ID or 40,41,42 for group, ROD, BOC */
  void editedCfg(int ID);
  /** tell us that cfg of all modules was edited */
  void editedCfgAllMods();
  /** cfg was saved, so clear edit flag */
  void savedCfg();
  /** set config of all but specified FE on selected module (all modules if modID<0) to zero DACs 
   *  if FE==-1, then full config will be restored and FE-zero config will be deleted
   */
  void selectFe(int FE, int modID);
  /** configure individual module, and only FEs that are on in binary mask */
  void configSingleMod(int modID, int maskFE);
  /** get active status of module of specified ID */
  bool getModuleActive(int modID);
  /** set active status of module of specified ID */
  void setModuleActive(int modID, bool active);
  /** reload controller configs - will only work for USBPixController registers */
  void reloadCtrlCfg();
  // connectivity test helper functions - obsolete
/*   bool runConnPart1(STControlEngine::connTestOpts opts, PixModule *mod); */
/*   bool runConnPart2(STControlEngine::connTestOpts opts, PixModule *mod); */

 private:
  int m_ID;
  bool &m_filebusy;
  /* stores status of the ROD and BOC owned by us */
  StatusTag m_CtrlStatus, m_BocStatus;
  // @TODO move module status from STcontrolEngine to this class
  /* stores the PixScan object to run a scan on this group (see set/getPixScan) */
  PixLib::PixScan *m_PixScan;
  /** stores the state of a scan loop (necessary because we don't have a continuously
      running loop, but something that's called by QTimer */
  // @TODO move scan loop from STControlEngine to this class so that
  //       the loop can be run as one set of nested loops in the background:
  //       the m_pixScanLoop variable would be obsoleted then
  bool m_pixScanLoop[3];  
  bool m_abortScan, m_forceRead;
  PixLib::Config &m_options;

//global readout flag and local pause flag
  bool m_testbeam_pause;
  static bool global_readout;

  QApplication* m_app;

  // variables holding information on scan progress.
  int m_nSteps[3];
  int m_nMasks;
  bool m_currSRAMReadoutReady;
  bool m_currSRAMFull;
  bool m_currMeasurementPause;
  bool m_currMeasurementRunning;
  int m_currSRAMFillLevel;
  int m_currTriggerRate;
  int m_currEvtRate;
  int m_currFe;
  bool m_readSram;
  // 0: scanning, 1: finished, 2: downloading histos from Rod, 5: fitting/downloading loop 0, 6: fitting/downloading loop 1, 7: fitting/downloading loop 2
  int ScanStatus; 
     
  // true only when ctrlthr::run() is running!
  bool processing;

  // need decorated for DB operation - it's not in PixModuleGroup...
  std::string m_decName;

  // contains status ifo (link/reg. tests) for modules
  std::map <int,modStatus> m_pixModuleStatus;
  std::string m_pixModConnName[32];

  // contains editing info for group, ROD, BOC and all modules
  bool m_grpEdited, m_rodEdited, m_bocEdited;
  std::map <int, int> m_modEdited;

  // purely temporary function to get histograms from FE by FE scanning merged
  void mergeFeHistos(PixLib::PixScan **cfgs);
  // prepare global register for register test
  void setGlobReg(int type, PixLib::PixFe *fe);
  // prepare pixel register for register test
  void setPixlReg(int type, PixLib::PixFe *fe);

 protected:
  // temp. storage of DCS status tags
  std::map<int, int> m_dcsStates;
  std::map<int, std::pair<int,double> > m_dcsRStates;
  std::map<int, std::vector<double>* > m_dcsRead;

 signals:
  //  void groupChanged();
  /** Emitted, when any of the status flags changes */
  //  void updateModList();
  /** emitted when a log message should be forwarded to somewhere */
  void logMessage(std::string);
  /** emitted when an error message should be forwarded to somewhere */
  void errorMessage(std::string);
  /** emitted when a ROD-related message should be forwarded to somewhere */
  void rodMessage(std::string, bool);
  /** Emitted, when the PixScan status has been read for this group */
  void sendPixScanStatus(int, int, int, int, int, int, int, int, int);
  /** emitted when a module test is proceeding */
  void sendRTStatus(const char*);
  /** emitted when a module is under test */
  void currentModule(PixLib::PixModule *mod);
  /** emitted, when the status of a sub-object (module, ROD,...) changed */
  void statusChanged();
  /** emitted when change of DCS channel is requested */
  void setDcs(std::string name, std::string parName, double value, int cmdId, STPixModuleGroup *grp);
  void getDcs(std::string name, int rtype, int cmdId, STPixModuleGroup *grp);
  // emitted when data arrived from USBPixController on nTrigger(unsigned int*) call
  void dataPending(std::vector<unsigned int *>* data, int boardid);
  void eudaqScanStatus(int boardid, bool SRAMFullSignal, int SRAMFillingLevel, int TriggerRate);
};

class ThreadErrorEvent : public QEvent
{
 public:
  ThreadErrorEvent( std::string  message )
    : QEvent((QEvent::Type)2000){text = message;};
  
  std::string getText(){return text;};

 private:
  std::string text;
};

class ThreadLogEvent : public QEvent
{
 public:
  ThreadLogEvent( std::string  message )
    : QEvent((QEvent::Type)2001){text = message;};
  
  std::string getText(){return text;};

 private:
  std::string text;
};


class setCtrlStatusEvent : public QEvent
{
 public:
  setCtrlStatusEvent(StatusTag ns, STPixModuleGroup * mg)
    : QEvent((QEvent::Type)2002){ newstatus =  ns; m_group = mg;};
  
  STPixModuleGroup * getGroup(){ return m_group;};
  StatusTag getStatus(){return newstatus;};

 private:
  StatusTag newstatus;
  STPixModuleGroup * m_group;
};

class setModStatusEvent : public QEvent
{
 public:
  setModStatusEvent(modStatus ns, PixLib::PixModule * mod)
    : QEvent((QEvent::Type)2003){ newstatus =  ns; m_mod = mod;};
  
  PixLib::PixModule * getModule(){ return m_mod;};
  modStatus getStatus(){return newstatus;};

 private:
  modStatus newstatus;
  PixLib::PixModule * m_mod;
};

class setMTestStatusEvent : public QEvent
{
 public:
  setMTestStatusEvent(std::string  message)
    : QEvent((QEvent::Type)2004){ m_text = message;};
  
  std::string getText(){return m_text;};

 private:
  std::string m_text;
};

class ThreadBufferEvent : public QEvent
{
 public:
  ThreadBufferEvent( std::string  message , bool err_in)
    : QEvent((QEvent::Type)2005), text(message), haveErr(err_in){};
  
  std::string getText(){return text;};
  bool getErr(){return haveErr;};

 private:
  std::string text;
  bool haveErr;
};

class setBocStatusEvent : public QEvent
{
 public:
  setBocStatusEvent(StatusTag ns, STPixModuleGroup * mg)
    : QEvent((QEvent::Type)2006){ newstatus =  ns; m_group = mg;};
  
  STPixModuleGroup * getGroup(){ return m_group;};
  StatusTag getStatus(){return newstatus;};

 private:
  StatusTag newstatus;
  STPixModuleGroup * m_group;
};

class setDcsEvent : public QEvent
{
 public:
  setDcsEvent(std::string chanName, std::string parName, double newValue, int cmdId, STPixModuleGroup * mg)
    : QEvent((QEvent::Type)2007), m_chanName(chanName), m_parName(parName), m_value(newValue), m_cmdId(cmdId) , m_group(mg){
    //std::cout<<"setDcsEvent"<<std::endl;
  };
  
 public:
  std::string getChanName(){return m_chanName;};
  std::string getParName(){return m_parName;};
  double getValue(){return m_value;};
  STPixModuleGroup * getGroup(){ return m_group;};
  int getCmdId(){return m_cmdId;};

 private:
  std::string m_chanName;
  std::string m_parName;
  double m_value;
  int m_cmdId;
  STPixModuleGroup * m_group;
};

class getDcsEvent : public QEvent
{
 public:
  getDcsEvent(std::string chanName, int rtype, int cmdId, STPixModuleGroup * mg)
    : QEvent((QEvent::Type)2008), m_chanName(chanName), m_rtype(rtype), m_cmdId(cmdId) , m_group(mg){};

 public:
  std::string getChanName(){return m_chanName;};
  int getRType(){return m_rtype;};
  STPixModuleGroup * getGroup(){ return m_group;};
  int getCmdId(){return m_cmdId;};

 private:
  std::string m_chanName;
  int m_rtype;
  int m_cmdId;
  STPixModuleGroup * m_group;
};

class dataPendingEvent : public QEvent
{
 public:
  dataPendingEvent(std::vector<unsigned int *>* data, int boardid /*STPixModuleGroup * mg, pixScanRunOptions opts*/)
    : QEvent((QEvent::Type)2010), m_data(data), m_boardid(boardid) {};
  
 public:
  std::vector<unsigned int *>* m_data;
  int m_boardid;
};

class eudaqScanStatusEvent : public QEvent
{
 public:
  eudaqScanStatusEvent(int boardid, bool SRAMFullSignal, int SRAMFillingLevel, int TriggerRate)
    : QEvent((QEvent::Type)2015), m_boardid(boardid), m_SRAMFullSignal(SRAMFullSignal), m_SRAMFillingLevel(SRAMFillingLevel), m_TriggerRate(TriggerRate) {};
  
 public:
  int m_boardid;
  bool m_SRAMFullSignal;
  int m_SRAMFillingLevel;
  int m_TriggerRate;
};
#endif // STPIXMODULEGROUP_H
