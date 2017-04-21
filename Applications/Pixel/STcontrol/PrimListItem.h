/***************************************************************************
                          PrimListItem.h  -  description
                             -------------------
    begin                : Wed 26 Jan 2006
    copyright            : (C) 2006 by jgrosse
    email                : joern.grosse-knetter@uni-bonn.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRIMLISTITEM_H
#define PRIMLISTITEM_H

#include <string>
#include <map>
#include <vector>

namespace PixLib{
class DBInquire;
class Config;
class PixScan; 
}

class ChipTest;

class PrlToolSubcfg {

 public:
  PrlToolSubcfg(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  virtual ~PrlToolSubcfg(){};

  PixLib::Config& config(){return m_config;};

 protected:

  virtual void setupGenConfig()=0;
  PixLib::Config &m_config;

};

class PrlToolSubVcal : public PrlToolSubcfg {

 public:
  PrlToolSubVcal(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubVcal();

 protected:

  void setupGenConfig();
  float m_charge;
  bool m_chigh;

};

class PrlToolSubFEGR : public PrlToolSubcfg {

 public:
  PrlToolSubFEGR(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubFEGR();

 protected:

  void setupGenConfig();
  int m_cfgType, m_cfgVal;

};

class PrlToolSubFEPR : public PrlToolSubcfg {

 public:
  PrlToolSubFEPR(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubFEPR();

 protected:

  void setupGenConfig();
  int m_cfgType, m_cfgVal;

};

class PrlToolSubPwr : public PrlToolSubcfg {

 public:
  PrlToolSubPwr(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubPwr();

 protected:

  void setupGenConfig();
  std::string m_dcsName;
  int m_dcsChan;
  bool m_on_or_off;

};

class PrlToolSubSetDcs : public PrlToolSubcfg {

 public:
  PrlToolSubSetDcs(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubSetDcs();

 protected:

  void setupGenConfig();
  std::string m_dcsName;
  int m_dcsSetType;
  float m_setVal;
	int m_dcsChan;  // new theresa
  float m_setOffset;
	
};

class PrlToolSubGetDcs : public PrlToolSubcfg {

 public:
  PrlToolSubGetDcs(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubGetDcs();

 protected:

  void setupGenConfig();
  std::string m_dcsName;
  int m_dcsChan;
  int m_dcsGetType;
  std::string m_storeName;
  float m_minCut;
  float m_maxCut;

};

class PrlToolSubFitScan : public PrlToolSubcfg {

 public:
  PrlToolSubFitScan(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubFitScan();

 protected:

  void setupGenConfig();
  std::string m_scanName;
  int m_funcType, m_histoType, m_storeType;
  std::vector<float> m_fitParPreset;
  float m_fitRgMin, m_fitRgMax;
  float m_conversion, m_offset;
};

class PrlToolSubWait : public PrlToolSubcfg {

 public:
  PrlToolSubWait(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubWait();

 protected:

  void setupGenConfig();
  int m_waitTime;
};

class PrlToolSubWaitForSignal : public PrlToolSubcfg {

 public:
  PrlToolSubWaitForSignal(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubWaitForSignal();

 protected:
  void setupGenConfig();
  int m_waitChannel;
  int m_ctrlCombi;
  bool m_waitOnOne;
};

class PrlToolSubConfigureMods : public PrlToolSubcfg {

 public:
  PrlToolSubConfigureMods(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
 ~PrlToolSubConfigureMods();

 protected:
  void setupGenConfig();
  int m_configType;
  int m_DCtoWrite;
  int m_patternType;
  int m_latch;
};

class PrlToolSubSaveCfg : public PrlToolSubcfg {

 public:
  PrlToolSubSaveCfg(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
 ~PrlToolSubSaveCfg();

 protected:
  void setupGenConfig();
  std::string m_extString;
  bool m_activeOnly;
};

class PrlToolSubSetFeMode : public PrlToolSubcfg {

 public:
  PrlToolSubSetFeMode(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
 ~PrlToolSubSetFeMode();

 protected:
  void setupGenConfig();
  int m_mode;
};

class PrlToolSubLoadCfg : public PrlToolSubcfg {

 public:
  PrlToolSubLoadCfg(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubLoadCfg();

 protected:
  void setupGenConfig();
  std::string m_fname;
};
class PrlToolSubLoadModCfg : public PrlToolSubcfg {

 public:
  PrlToolSubLoadModCfg(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubLoadModCfg();

 protected:
  void setupGenConfig();
  bool m_setChipSN;
  int m_chipSN;
};

class PrlToolSubResetMods : public PrlToolSubcfg {

 public:
  PrlToolSubResetMods(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubResetMods();

 protected:
  void setupGenConfig();
  int m_resetType;
};

class PrlToolSubReadGADC : public PrlToolSubcfg {

 public:
  PrlToolSubReadGADC(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubReadGADC();

 protected:
  void setupGenConfig();
  int m_ReadType;
};

class PrlToolSubSendGlobpls : public PrlToolSubcfg {

 public:
  PrlToolSubSendGlobpls(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubSendGlobpls();

 protected:
  void setupGenConfig();
  int m_length;
};

class PrlToolSubSendDcsCmd : public PrlToolSubcfg {

 public:
  PrlToolSubSendDcsCmd(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubSendDcsCmd();

 protected:
  void setupGenConfig();
  std::string m_dcsName;
  std::string m_command;
  bool m_readResp;
};

class PrlToolSubSendMail : public PrlToolSubcfg {

 public:
  PrlToolSubSendMail(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubSendMail();

 protected:
  void setupGenConfig();
  std::string m_smtpServerName;
  std::string m_userName;
  std::string m_password;
  std::string m_subject;
  std::string m_message;
  std::string m_sender;
  std::string m_recipient;
  std::string m_senderName;
  std::string m_recipientName;
  int m_port;
  int m_connectionType;
  int m_authentificationType;
};

class PrlToolSubAnaGraph : public PrlToolSubcfg {

 public:
  PrlToolSubAnaGraph(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubAnaGraph();

 protected:

  void setupGenConfig();
  std::string m_scanName;
  int m_anaAction, m_storeType, m_feIndex;
  std::vector<float> m_anaArgs;
};

class PrlToolSubGenPixMask : public PrlToolSubcfg {

 public:
  PrlToolSubGenPixMask(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubGenPixMask();

 protected:

  void setupGenConfig();
  std::string m_scanName;
  int m_histType, m_maskType, m_maskComb;
  float m_cutMin, m_cutMax;
};


class PrlToolSubSelBiMod : public PrlToolSubcfg {

 public:
  PrlToolSubSelBiMod(PixLib::DBInquire *inq, PixLib::Config &cfg_in);
  ~PrlToolSubSelBiMod();

 protected:

  void setupGenConfig();
  int m_channel;
  bool m_setIref, m_skipIfNotIref, m_wrapGrpId;
};

class PrimListItem {

  friend class PrlToolSubcfg;

 public:

  enum PrimListTypes {DEF_SCAN, CUST_SCAN, TOOL, CHIP_TEST, MAX_PRL_TYPES};
  enum PrimListTools { INIT_ROD, RESET_MODS, CFG_MODS, SET_VCAL, LINK_CHECK, DISABLE_FAILED, RELOAD_CFG, 
		       SET_FE_GR, SET_FE_PR, INIT_DCS, DCS_PWR, DCS_SET, DCS_GET, 
		       FIT_SCAN, GEN_WAIT, SIG_WAIT, GET_SRVREC, GET_TIME, SAVE_CFG, LOAD_MOD_CFG, SET_FE_MODE, 
		       LOAD_NEW_CFG, GEN_WAIT_MSEC, SEND_DCS_CMD, ANA_GRAPH, RD_EPROM, BURN_EPROM, READ_GADC, SEND_GLOBPLS,
		       SELECT_BI_MOD, DCS_ONOFF, GEN_PIXMASK, MAX_PRL_TOOLS, SEND_MAIL};

  PrimListItem(PrimListTypes, int, const char*, PixLib::PixScan *in_ps=0);
  PrimListItem(PixLib::DBInquire*);
  PrimListItem(PrimListItem&, int);
  ~PrimListItem();

  PixLib::Config& config(){return *m_config;};

  PrimListTypes getType(){return (PrimListTypes)m_type;};
  int getIndex(){return m_index;};
  void setIndex(int new_index){m_index = new_index;};
  std::string getLabel(){return m_label;};
  std::string getScanLabel(){return m_scanLabel;};
  void setScanLabel(const char *newLabel){m_scanLabel = newLabel;};
  bool getAbort(){return m_abortIfError;};
  bool getSkipFlag(){return m_executeIfSkip;};
  std::string getTypeLabel(PrimListTypes type);
  std::string getToolLabel(PrimListTools type);
  int getSubType(){return m_subType;};
  PixLib::PixScan* getPS(){return m_ps;};
  ChipTest* getCT(){return m_ct;};
  void setupSubcfg(PixLib::DBInquire *inq);
  int getReadDcs(){return m_readDcs;};
  int getTimeout(){return m_timeout;};
  
 protected:

  void setupGenConfig(bool doreset=true);
  void setupSpecConfig(PixLib::PixScan *in_ps=0, bool doreset=true);

  // general members
  int m_type;
  int m_index;
  bool m_abortIfError;
  bool m_executeIfSkip;
  bool m_raiseSkipIfFailed;
  std::string m_label, m_scanLabel;
  int m_readDcs;
  int m_timeout;
  PixLib::Config *m_config;
  std::map<std::string, int> m_typeDef;
  std::map<std::string, int> m_prlTools;
  std::map<std::string, int> m_scanTypes;
 // type-specific members
  int m_subType;
  PixLib::PixScan *m_ps;
  std::vector<int> m_toolArgs;
  PrlToolSubcfg *m_subCfg;
  ChipTest *m_ct;
};

#endif // PRIMLISTITEM_H
