/***************************************************************************
                          STControlEngine.h  -  description
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

#ifndef STCONTROLENGINE_H
#define STCONTROLENGINE_H

#include <vector>

#include <map>
#include <string> 

#include <qobject.h>
#include <QString>
#include <QStringList>
#include <QMetaType>

#include <GeneralDBfunctions.h>
#include "STCdefines.h"

namespace PixLib{
class PixModuleGroup;
class PixModule;
class PixConfDBInterface;
class PixScanConfig;
class PixScan;
class Histo;
class DBInquire;
class Config;
}
namespace SctPixelRod{
class VmeInterface;
}
class STRodCrate;
class STCLogContainer;
class PrimListItem;
class QTimer;
class QApplication;
class QString;
class QTime;
class ChipTest;

#ifdef WITHEUDAQ
#include "STControlProducer.h"
#endif

/**This class encapsulates most of the actual work of Stcontrol. This includes in particular the interaction with PixLib.
  *@author jschumac
  */

class STControlEngine : public QObject {

  Q_OBJECT

public: 
  STControlEngine( QApplication *app, STCLogContainer& log_in, QObject * parent=0);
  ~STControlEngine();
  /** initialise config, set-up etc as per options or explicit argumens */
  void init(const char *cfg_file, bool initRODs, const char *ps_file);
  /** Read property of vector <STRodCrate *> m_sTRodCrates. */
  std::vector <STRodCrate *> & getSTRodCrates();
  /** Read property of vector m_knownScanCfgs */
  std::vector<PixLib::PixScan*>  & getPixScanCfg(){return m_knownPixScanCfgs;};
  /** Return DB-name of a PixScan config */
  std::string getPixScanCfgDBName(PixLib::PixScan *cfg);
  /** Change DB-name of a scan config */
  void setPixScanCfgDBName(PixLib::PixScan *cfg, const char *name);
  /** Adds a STRodCrate */
  STRodCrate& addCrate(const char *SBCaddress);
  /** Deletes all STRodCrates */
  void clear();
  /** adds a crate to a DB file (which is created if it doesn't exist) and adds info from a vector of group/modules */
  void addCrateToDB(const char *crateName, std::vector<PixLib::grpData> inGrpData, const char *DBfilename, IFtypes crtIFtype=tRCCVME,
		    bool addUsbDcs=true);
  /** adds PixModules to a PixModuleGroup in currently loaded DB file */
  void addModulesToCurrDB(int crateID, int grpID, std::vector<PixLib::modData> inModData);
  /** adds a PixModuleGroup to currently loaded DB file */
  void addGroupToCurrDB(PixLib::grpData inGrpData, const char *crateName, IFtypes crtIFtype=tRCCVME);
  /** adds a PixModuleGroup to a DB inquire */
  void addGrouptoDB(PixLib::DBInquire *baseInq, PixLib::grpData inGrpData, IFtypes crtIFtype=tRCCVME);
  /** adds a PixDcs object to currently loaded DB file */
  void addDcsToCurrDB(const char *objName, const char *classNameHead, int devType, int nChan, const char *ctrlName, const char *crateName);
  /** adds a PixDcs object to DB file */
  void addDcsToDB(PixLib::DBInquire *baseInq, const char *objName, const char *classNameHead, int devType, int nChan, const char *ctrlName);
  /** adds a USB*PixDcs object with default settings to DB file 
   *  def (for backward-compatibility): adapterType=0: SCA, =1: BIC single-FE, =2: BIC multi-FE, =3: SCA+demux(???), =4: GPAC
   */
  void addUsbDcs(std::string ctrlName_in, int USB_id, int adapterType, int &usbDcsInd, int &usbInd, PixLib::DBInquire *startInq, PixLib::PixConfDBInterface *myDB);
  /** loads objects according to entries in DB file */
  void loadDB(const char *DBfilename);
  /** reopens current DB file in update mode so it can be edited */
  PixLib::PixConfDBInterface * reopenDBforEdit();
  /** reverts action from reopenDBforEdit and reloads edited (getNew==true) or old (false) DB file */
  void reloadDBafterEdit(bool getNew);
  /** saves object configs as entries in DB file */
  void saveDB(const char *DBfilename, bool activeOnly=false);
  /** copies non-USBPixDcs-entries from 2nd file into 1st file */
  void copyDcsDb(const char *fname_currDB, const char *fname_cpDB, const char *ctrlName);
  /** saves PixScan configs as entries in DB file */
  void savePixScan(const char *DBfilename);
  /** Loads crates, PixModuleGroups, ... from current DB file */
  void loadCrates();
  /** loads all scan configurations in DB */
  void loadScanCfg(const char *fname=0);
  /** creates a new PixScan configurations in memory (not in DB!) */
  void newPixScanCfg(const char *cfgName, PixLib::PixScan *copy_from_this);
  /** Start a scan using PixScan */
  int pixScan(pixScanRunOptions, bool start_monitor=true);
  /** stop all running PixScans */
  void stopPixScan();
  /** force data reading on all crates (for test beam usage) */
  void forceRead();
  /** get reference to our options Config object */
  PixLib::Config &getOptions(){return *m_options;};
  /** access to global chip test configuration */
  PixLib::Config& chipTestCfg(){return *m_ctCfg;};
  /** Options of the current Scan, for EUDAQ to not use PixScan directly */
  int getConsecutiveLvl1TrigA(int);
  int getConsecutiveLvl1TrigB(int);
  
  const char* getPixConfDBFname() { return m_PixConfDBFname.c_str(); };
  void setPixConfDBFname(const char *in_path);

  /* see if we know where PixLib is */
  QString& plPath(){return m_plPath;};
  /* check if cfg was edited */
  bool cfgEdited(){return m_cfgEdited;};
  /* tell us that cfg was edited */
  void editedCfg(){m_cfgEdited=true; emit cfgLabelChanged();};
  /* cfg was saved, so clear edit flag */
  void savedCfg(){m_cfgEdited=false; emit cfgLabelChanged();};
  /* clears register and quick digital test info for all modules */
  void clearModuleInfo();
  /* running in single or multi-crate mode? */
  bool singleCrateMode(){return m_singleCrateMode;};
  /* which interface type are we using? 0=-RCC-VME, 1=Bonn-USB-board */
  IFtypes whichIFtype(){return m_IFtype;};

  /* perform link check via MCC pattern */
  void runLinkCheck(int crateID, int grpID, int modID);
  void runLinkCheck(){runLinkCheck(-1,-1,-1);};
  /* perform chip test test, type identified by label */
  void runChipTest(ChipTest *ct_in, int crateID, int grpID, int modID);
  void runChipTest(const char *label, int crateID, int grpID, int modID);
  void runChipTest(const char *label){runChipTest(label, -1,-1,-1);};
  /** set config of all but specified FE on selected module (all modules if ID<0) 
   *  on selected group (all groups if ID<0) on selected crate (all crates if ID<0) 
   *  to zero DACs if FE==-1, then full config will be restored and FE-zero 
   *  config will be deleted
   */
  void selectFe(int FE, int crateID=-1, int grpID=-1, int modID=-1);
  /* set TDACs of all modules to what is found in specified file */
  int setTFDACs(const char*, bool);
  /*! set selected masks from a histogram from a RootDB file */
  int setMasks(std::vector<std::string> files, std::vector<std::string> histos, int, std::string logicOper);
  /*! set VCAL of all FEs of all modules in all groups to a value corresponding to requested charge */
  void setVcal(float charge, bool Chigh);
  /*! de-/increment MCC delay of all modules in all groups to a value corresponding to requested value
   *  if "calib" is true, the value will be converted from ns to MCC delay; if false, the value as such
   *  will be used
   */
  void incrMccDelay(float delay, bool calib);
  /* disable all modules that have a "failed" state */
  void disableFailed();

  /** initialise all RODs and their BOCs */
  void initRods();  
  /** reset all RODs */
  void resetRods();  
  /** initialise all BOCs */
  void initBocs();  
  /** initialise all DCS objects */
  void initDcs();
  /** send config from RODs to all active modules 
      0: all reg's, 1: GR only, 2: PR only */
  void configModules(int cfgType=0, int patternType=0, int DCs=-1, std::string latch="ALL");
  /** reset all modules */
  void resetModules(int type);
  /** send a L1 trigger to all modules */
  void triggerModules();
  /** get service records from FE-I4 on demand */
  void getSrvRec();
  /** send global pulse to read FEs EPROM to GR */
  void readEPROM();
  /** send global pulse to read value using GADC to GR */
  void readGADC(int type);
  /** make all MCCs send a pattern (XCK/2, XCK etc) */
  void setMcc(int opt);
  /* explicit function to make all modules' MCCs send clk(/2) */
  void clkhalfAllMods(){setMcc(0);};
  void clkfullAllMods(){setMcc(1);};
  /* tell all crates/groups to send a global pulse of defined length */
  void sendGlobalPulse(int length);

  /** gives back true if one of the Rods has still running processes in thread */
  bool RodProcessing();
  // 
  void setObservedGroup(int crate, int grp){m_obsCrate=crate; m_obsGrp=grp;};
  //
  int CtrlStatusSummary(); 

  /** update GUI: calls main QApplication 
   *  to process pending GUI events */
  void updateGUI();

  /* static function to compare scan labels for uniqueness and suggest alternatives
   */
  static bool checkScanLabel(std::vector<QString> existingLabels, QString newLabel, QString &suggLabel, QString compExpr=" ");
  std::vector<int> GetBoardIDs();
  std::map<int, std::string> GetFeFlavours();
  int GetHitDiscCnfg();

  // Returns last Scan config
  pixScanRunOptions getLastPxScanConfig() {return m_lastPixScanOpts;};

  // tell the current FE flavour being in use
  int currFEFlavour();

  // set brun-in mode (typically called from prim.list)
  void setBurnInChan(int chan, bool setIref, bool skipIfNotIref, bool wrapGrpId);

public: // Public attributes
  // get application
  QApplication& getApp(){return *m_app;};

public slots:
  /** read ROD text buffers and write to log display */
  void readRodBuff(std::string msg);
  void readRodBuff(){readRodBuff("");};
  /** write message to STcontrol ROD buffer log */
  void toRodBuff(std::string msg, bool error);
  /** write message to STcontrol info log */
  void toLog(std::string msg);
  /** write message to STcontrol error log */
  void toErrLog(std::string msg);
  /** load STcontrol's options from .stcrc file */
  void loadOptions();
  /** write STcontrol's options to .stcrc file */
  void saveOptions();
  /** decides whether scanning has finished, performs some end-of-scan actions */
  void FinishScan();
  /** initialises execution of a primitive list */
  void launchPrimList(std::vector<PrimListItem*> list, const char *outname, int labelIndex=0, bool indexMode=false, int modTestType=0);
  /** processes entries from DB entries and takes according scan, init etc actions */
  void processPrimList();
  /** processes PixScans found in a prim list */
  bool launchPrlScans(const char *label, const char *fname, PixLib::PixScan *cfg=0, int type=0, bool scanIsI4=true, int readDcs=0, int timeToAbort=-1);
  /** processes Tools found in a prim list */
  bool launchPrlTools();
  /** signals an abort from outside the engine */
  void abortPrlExec(){m_stopProcessing=true;};
  /** processes crate list changes triggered from outside */
  void changedConfigs();
  /** display read-out view and monitoring/regulator (DCS) view in same panel */
  bool getRDSame(){return m_RoDcsSamePanel;}
  /** EUDAQ special function to create new cfg. file from several individual ones */
  void createMultiBoardConfig(QString, extScanOptions);
  /** allows EUDAQ producer to turn off error pop-up windows to avoid pending STcontrol */
  void setShowErrorPopups( bool );

protected:
  /* stores temporarily (ie during a scan) the settings used for that scan */
  pixScanRunOptions m_lastPixScanOpts;
  /* main application item, needed for controlled GUI updates */
  QApplication *m_app;

private: // Private attributes

  /** This contains the STRodCrates that we own. They are deleted when necessary. */
  std::vector <STRodCrate *> m_sTRodCrates;
  /* container for PixScan config DB interfaces */
  std::map<long int,std::string> m_knownPSDBcfgs;
  /* container for PixScan configs */
  std::vector<PixLib::PixScan*> m_knownPixScanCfgs;
  /* timer calling the run status checker */
  QTimer *m_scanTimer;
  // timer calling the finishScan-routine
  QTimer *m_scanTimer2;
  /** Logging Facility */
  STCLogContainer& m_log;
  /* vectors to hold histograms from ROD SDSP */
  /* fist level: crate, 2nd: ROD (module group), 3rd: module, 4th: scan pt.*/
  /* m_occHisto: occupancy raw data */
  std::vector< std::vector< std::vector< std::vector<PixLib::Histo*> > > > m_occHisto;
  /* flag whether something in the config has been edited */
  bool m_cfgEdited;
  /* this decides whether we run in single crate mode (no crate addition or delete allowed
     or in multi crate mode */
  bool m_singleCrateMode;
  // type of interface used
  IFtypes m_IFtype;
  // options related stuff
  PixLib::Config *m_options;

  // variables for option config
  std::string m_dataPath, m_dataName, m_cfgPath, m_cfgName, m_psName, m_logPath, m_modPath;
  bool m_addDatetoDataFile, m_initRods, m_psAutoLabel, m_showRodWin, m_showErrWin;
  bool m_resetAfterTest, m_initDcs, m_dcsOn, m_RoDcsSamePanel;
  unsigned int m_scanTimeout;

  // bool m_finishScanRunning;

  std::string m_PixConfDBFname;
  int *m_scanPtsPtr[2];
  int m_checkScanCount; // counts how often we've checked the scan status

  QString m_plPath;

  // variables holding the crate/rod  to be observed in GUI during scanning
  int m_obsCrate, m_obsGrp;

  // for prim. list operation
  bool m_stopProcessing;
  bool m_prlSkipToEnd;
  std::string m_prlOutname, m_prlModTestType;
  std::vector<PrimListItem *> m_prlList;
  int m_prlIndex, m_prlLabelIndex;
  bool m_prlIndMode;
  QTime *m_pixScanStartTime;

  // global chip test options
  std::map<std::string, ChipTest*> m_chipTests;
  PixLib::Config *m_ctCfg;

  std::unique_ptr<STControlProducer> m_STControlProducer;

  float readCurrent(float cutval); // dummy routine
  void processExecute();

 signals: // Signals
  /** Emitted, when the edit state of the config file or its changed */
  void cfgLabelChanged();
  /** Emmitted when the configuration connected to a (all) STRodCrate list(s) change(s) **/
  void configChanged();
  /** Emmitted when the STRodCrate list changes, not necessarily when its members change internally. */
  void crateListChanged();
  /** emitted when a new user-def. scan config. is loaded */
  void scanConfigChanged();
  // emitted when the Mod LIst view needs to be updated
  //  void updateModList();
  /** Emitted, when a scan has been completed and return data is ready to be accessed */
  void scanCompleted();
  /** Emitted, when a histogram has been read from the ROD and is ready for processing */
  void gotData(PixLib::PixModule &mod);
  /** Emitted, when PixScan results have been written to file and are ready for processing */
  void gotPixScanData(const char *fname);
  void gotPixScanData(PixLib::DBInquire *dataInq, int summary, const char *fname);
  /** Emitted, when PixScan file was changed externally */
  void scanFileChanged(const char *fname);
  /** Emitted, when the scan status has been read */
  void sendScanStatus(int, int, int);
  /** Emitted, when the PixScan status has been read */
  void sendPixScanStatus(int, int, int, int, int, int, int, int, int);
  /** Emitted while a register test or a quick digital test is performed */
  void sendRTStatus(const char*);
  /** for disabling Tool Panel when Scan is running*/
  void beganScanning();
  /** for enabling Tool Panel when Scan is running*/
  void finishedScanning();
  /** for interrupting Tool Panel Loop when errors occur **/
  void breakloop();
  /** emitted when a primitive list scan is launched; argument distinguishes strobe/src/TB scan */
  void prlScanStarted(int);
  /** emitted when a new item of a primitive list is in progress */
  void prlInProgress(int);
  /** emitted when a primitive list has fully been processed */
  void prlDone();
  /** emitted when a module is under test */
  void currentModule(int, int, int);
  /** emitted when module under test is not to be changed to none */
  void blockCurrMod(bool);
  /** emitted when a BOC data analysis is requested */
  void analyseBoc(const char *filename, const char *scanlabel);
  /** emitted, when the status of a crate/group sub-object (module, ROD,...) changed */
  void statusChanged();
  /** emitted when main panel should open message window */
  void popupWin(const char*);
  /** emitted to switch on all DCS Objects */
  void powerOn();
  /** emitted to switch off all DCS Objects */
  void powerOff();
  /** emitted to set scan options from eudaq producer*/
  void setScanOptions(extScanOptions);
  /** emitted to start the currently selected scan with the submitted scan label */
  void startCurrentScan(QString, QString);
  void error( std::string msg );
};



#endif
