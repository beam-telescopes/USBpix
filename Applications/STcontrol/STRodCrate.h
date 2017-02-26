/***************************************************************************
                          STRodCrate.h  -  description
                             -------------------
    begin                : Fri Apr 16 2004
    copyright            : (C) 2004 by Jan Schumacher
    email                : jschumac@physik.uni-bonn.de
    modifications        : 2005 by Iris Rottlaender
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STRODCRATE_H
#define STRODCRATE_H

#include <vector>
#include <map>
#include <string>
#include <qapplication.h>
#include <qobject.h>
#include "STCdefines.h"

class STPixModuleGroup;
class QTimer;
class ChipTest;

namespace PixLib{
  class PixModuleGroup;
  class PixModule;
  class PixConfDBInterface;
  class DBInquire;
  class Config;
  class PixScan;
  class PixDcs;
  class PixDcsChan;
}
namespace SctPixelRod{
  class VmeInterface;
}


/**This class represents one rod crate, i.e. it contains all PixModuleGroups of one crate
  *@author Jan Schumacher
  */

class STRodCrate : public QObject {

  Q_OBJECT

 public:

  STRodCrate();
  STRodCrate( int ID, PixLib::Config &options, QApplication* application, QObject * parent=0, const char *SBCaddr=0);
  virtual ~STRodCrate();

  /** returns the full IP address of this SBC */
  std::string getAddress(){return m_SBCaddr;};
  /** returns the top part of the IP address of this SBC (i.e. its name)*/
  std::string getName();
  /** return the ID as provided to the constructor */
  int getID(){return m_ID;};
  /** return the type of the interface of this crate */
  IFtypes getIFtype(){return m_IFtype;};
  /** creates a new VME interface; can't put this into constructor, otherwise the log messages get lost */
  void setupVme();
  /** Read property of VmeInterface * m_vmeInterface. */
  bool vmeOK();
  // obsolete  VmeInterface * getVmeInterface() { return m_vmeInterface; };
  /** Read property of vector <PixModuleGroup *> m_pixModuleGroups. */
  std::vector <STPixModuleGroup *> &getSTPixModuleGroups();
  /** Read property of vector <PixDcs *> m_pixDcs. */
  std::vector <PixLib::PixDcs *> &getPixDcs();
  /** initialises the PixScan objects of all module groups */
  int setPixScan(PixLib::PixScan&);
  /** This deletes all PixModuleGroups and PixDcs*/
  void clear();
  /** load all module(group)s found in the DB inquire named "decName" in file "fname" */
  void loadDB( const char *fname, const char *decName );
  /** save all edited configs to its DB inquire in file fname */
  bool saveDB(const char *fname, bool activeOnly=false);
  /** re-load module config from DB file */
  bool reloadModCfg(const char *fname);

  // auxilliary functions to avoid direct access to STPixModuleGroup objects

  /** gives back number of initialised Rods, -1 if no Rod present */
  int CtrlStatusSummary();
  /** gives back true while any thread from tool panel or scan is running */
  bool RodProcessing();
  /** user-trigger reading of ROD buffers */
  void readRodBuff();
  /* set GR of all FEs of all modules to a new value; GR identified by name */
  void setGR(std::string GRname, int GRval);
  /* set PR of all FEs of all modules to a new value; PR identified by name */
  void setPR(std::string PRname, int PRval);
  /** set TDACs of all modules in all groups to what is found in specified file */
  int setTFDACs(const char*, bool);
  /*! set selected masks from a histogram from a RootDB file */
  int setMasks(std::vector<std::string> files, std::vector<std::string> histos, int, std::string logicOper);
  /** set VCAL of all FEs of all modules in all groups to a value corresponding to requested charge */
  void setVcal(float charge, bool Chigh);
  /*! de-/increment MCC delay of all modules in all groups to a value corresponding to requested value
   *  if "calib" is true, the value will be converted from ns to MCC delay; if false, the value as such
   *  will be used
   */
  void incrMccDelay(float delay, bool calib);
  /** disable all modules that have a "failed" state */
  void disableFailed();
  /** initialise all DCS objects */
  void initDcs();  
  /** initialise all RODs and their BOCs */
  void initRods();  
  /** reset all RODs */
  void resetRods();  
  /** initialise all BOCs */
  void initBocs();  
  /** send config from RODs to all active modules */
  void configModules(int cfgType=0, int patternType=0, int DCs=-1, std::string latch="ALL");
  /** reset all modules */
  void resetModules(int type);
  /** send a L1 trigger to all modules */
  void triggerModules();
  /** get service records from FE-I4 on demand */
  void getSrvRec();
  /** make all MCCs send a pattern (XCK/2, XCK etc) */
  void setMcc(int opt);
  /** clear module status info */
  void clearModuleInfo();
  /** abort scan on all module groups */
  void abortScan();
  /** returns two lists, one with slots of existing RODs, one with their rev. no's */
  void listRODs(std::vector<int> &rod_slots, std::vector<unsigned int> &revs);
  /** return number of module groups in this crate */
  int nGroups(){return m_pixModuleGroups.size();};
  /** returns STPixModuleGroup::getNSteps(int loop) for group iGrp */
  int getGrpNSteps(int iGrp, int i);
  /** returns STPixModuleGroup::getNMasks for group iGrp */
  int getGrpNMasks(int iGrp);
  /** returns STPixModuleGroup::getSRAMFillLevel for group iGrp */
  int getGrpSRAMFillLevel(int iGrp);
  /** returns STPixModuleGroup::getTriggerRate for group iGrp */
  int getGrpTriggerRate(int iGrp);
  /** returns STPixModuleGroup::getEvtRate for group iGrp */
  int getGrpEvtRate(int iGrp);
  /** returns STPixModuleGroup::getCurrFe for group iGrp */
  int getGrpCurrFe(int iGrp);
  /** returns STPixModuleGroup::getScanStatus for group iGrp */
  int getGrpScanStatus(int iGrp);
  /** returns PixModuleGroup name for group iGrp */
  std::string getGrpName(int iGrp);
  /** returns PixModuleGroup decor. name for group iGrp */
  std::string getGrpDecName(int iGrp);
  /** returns >0 if group of index iGrp has a PixController: 1=ROD, 2=USB, 3=TPLL */
  int hasCtrl(int iGrp);
  /** returns true if group of index iGrp has a PixBoc */
  bool hasBoc(int iGrp);
  /** Read status of PixBoc of PixModuleGroup iGrp if found. */
  StatusTag getPixBocStatus(int iGrp);
  /** Read status of PixCOntroller of PixModuleGroup iGrp if found. */
  StatusTag getPixCtrlStatus(int iGrp);
  /** Read hardware info string from PixController */
  void ctrlHwInfo(int iGrp, std::string &txt);
  /** Set status of PixCOntroller of PixModuleGroup iGrp if found. */
  void setPixCtrlStatus(int iGrp, StatusTag status);
  /** only for USBPix: get input status of ctrl */
  bool getCtrlInputStatus(int iGrp, int input);
  /** returns a list modules (moduleId() and its name) of PixModuleGroup iGrp if found */
  void getModules(int iGrp, std::vector< std::pair<std::string, int> > &list);
  /** returns the name of module iMod of PixModuleGroup iGrp if found */
  std::string getModName(int iGrp, int iMod);
  /** send config of selected module to its ROD */
  void writeConfig(int iGrp, int iMod);
  /** Read status of module iMod of PixModuleGroup iGrp if found. */
  modStatus getPixModuleStatus(int iGrp, int modID);
  /** Change property of entry in map <StatusTag> m_pixModuleSTatus(Info). */
  void setPixModuleStatus(int iGrp, int modID, modStatus newStatus, bool emitSignal=true);
  /** set HV mask of specific module in group */
  void setModHVmask(int iGrp, int modId, bool on);
  /** set HV mask of all modules in group */
  int getModHVmask(int iGrp);
  /** Read local config of a module if found. */
  PixLib::Config& getPixModuleConf(int iGrp, int modID);
  /** Read local config of a FE (ID=0...15) or MCC (ID=16) of a module if found. */
  PixLib::Config& getPixModuleChipConf(int iGrp, int modID, int chipID);
  /** Read local config of a PixCOntroller if found. */
  PixLib::Config& getPixCtrlConf(int iGrp);
  /** Read local config of a PixBoc if found. */
  PixLib::Config& getPixBocConf(int iGrp);
  /** Read local config of group iGrp */
  PixLib::Config& getGrpConf(int iGrp);
  /** tell us that cfg of object was edited  - ID is module ID or 40,41,42 for group, ROD, BOC */
  void editedCfg(int iGrp, int ID);
  /** tell us that cfg of all modules was edited */
  void editedCfgAllMods(int iGrp);
  /** Change local config of a module. */
  void setPixModuleConf(int iGrp, int modID, PixLib::Config &conf);
  /* perform link check via MCC pattern on specified module 
   * if grpID<0, run on all groups and active modules  */
  void runLinkTest(int grpID, int modID);
  /* perform chip test  on specified module
   * if grpID<0, run on all groups and active modules  */
  void runChipTest(ChipTest *ct, bool resetMods, int grpID, int modID);
  /** launch scan */
  void scan(pixScanRunOptions scanOpts);
  /** save scan results - only used for windows */
  void saveScanIfDone(pixScanRunOptions scanOpts);
  /** restore DCS settings after scan - to be called from STControlEngine */
  void restoreAfterScan();
  /** set config of all but specified FE on selected module (all modules if ID<0) 
   *  on selected group (all groupd if ID<0) to zero DACs if FE==-1, then full config 
   *  will be restored and FE-zero config will be deleted
   */
  void selectFe(int FE, int grpID, int modID);
  /** configure individual module of group grpID, and only FEs that are on in binary mask */
  void configSingleMod(int grpID, int modID, int maskFE);
  /** get active status of module of specified ID */
  bool getModuleActive(int grpID, int modID);
  /** set active status of module of specified ID */
  void setModuleActive(int grpID, int modID, bool active);
  /** get service record reply from specific group */
  void getSrvRecRes(int iGrp, std::vector<int> &srvRec);
  /** get GADC reply from specific group */
  void getGADCRes(int iGrp, std::vector<int> &values);
  /** reload controller cfg. for given group */
  void reloadCtrlCfg(int iGrp);
  /** set all FEs to one of their operating modes */
  void setFeMode(int mode);
  /** read the FEs EPROM value to the GR */
  void readEPROM();
  /** burn the current GR values to the FEs EPROM */
  void burnEPROM();
  /** digitize some value using GADC and write it to the GR */
  void readGADC(int type);
  /** read UsbPixDcs in crate and try to associate to module groups */
  void readUsbPixDcs();
  /** read error messages from PixDcs devices in crate */
  void readPixDcsErr();
  /* tell all groups to send a global pulse of defined length */
  void sendGlobalPulse(int length);

  // counter for DCS reading during scans
  int m_readyForDcs;

  // Returns a Vector with all boardids connected to zhis ROD
  std::vector<int> GetBoardIDs();
  std::map<int, std::string> GetFeFlavours();
  int GetHitDiscCnfg();

 public slots:
  void recCurrentModule(PixLib::PixModule *);
  void setDcs(std::string name, std::string parName, double value, int cmdId, STPixModuleGroup *grp);
  void getDcs(std::string name, int rtype, int cmdId, STPixModuleGroup *grp);
  void checkUsbEvent();

 private: // Private attributes/functions
  /** Loads PixelModuleGroups from file */
  void loadModuleGroup( PixLib::DBInquire *grpInq );
  /** Loads PixelDcs from file */
  void loadDcs( PixLib::DBInquire *dcsInq );
  /** deletes VME interface if requested, or resets USBPix interface */
  void clearVme();
  /** This vector contains the PixModuleGroups belonging to this crate.
  STRodCrate owns the pointers and will destroy them when necessary. */
  std::vector <STPixModuleGroup *> m_pixModuleGroups;
  std::vector <PixLib::PixDcs*> m_pixDcs;
  std::vector<PixLib::PixDcsChan*> m_dcsPrescanCfg;
  std::vector<std::string> m_dcsState;
  int m_ID;
  IFtypes m_IFtype;
  std::string m_SBCaddr;
  SctPixelRod::VmeInterface *m_vmeInterface;
  bool m_USBinit;
  QApplication* m_app;
  QTimer *m_timer;
  bool m_filebusy;
  bool m_comInit;
  PixLib::Config& m_options;

 signals:
  /** Emitted, when the m_pixModuleGroups vector changes */
  void groupListChanged();
  /** Emitted, when ModGroupList needs to be updated. */
  //  void updateModList();
  /** emitted when a log message should be forwarded to somewhere */
  void logMessage(std::string);
  /** emitted when an error message should be forwarded to somewhere */
  void errorMessage(std::string);
  /** emitted when a ROD-related message should be forwarded to somewhere */
  void rodMessage(std::string, bool);
  /** emitted when a module test is proceeding */
  void sendRTStatus(const char*);
  /** emitted when a module is under test */
  void currentModule(int, int, int);
  /** emitted, when the status of a sub-object (module, ROD,...) changed */
  void statusChanged();
  // emitted when data arrived from USBPixController via STPixModuleGroup
  void dataPending(std::vector<unsigned int *>* data, int boardid);
  void eudaqScanStatus(int boardid, bool SRAMFullSignal, int SRAMFillingLevel, int TriggerRate);
};

#endif
