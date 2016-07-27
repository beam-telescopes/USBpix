#include "CfgWizardDB.h"
#include <string>
#include <sstream>

CfgWizardDB::CfgWizardDB() :  TurboDaqDB(), m_currGroup(0){
  CfgWizardDBInquire *tmp_rootRecord = new CfgWizardDBInquire("rootRecord");
  tmp_rootRecord->getDecName() = "/rootRecord";
  CfgWizardDBField * db = new CfgWizardDBField("CfgWizardDB Version", STRING);
  db->m_stringContent = m_version;
  insertField(tmp_rootRecord,db);
  tmp_rootRecord->m_myDB = this;
  m_rootRecord = tmp_rootRecord;
  // add application
  m_app = new CfgWizardDBInquire("application");
  m_app->getDecName() = "RCC-Control";
  insertInquire(m_rootRecord,m_app);
  // have no groups yet
  m_currGroup = NULL;
}
void CfgWizardDB::newModuleGroup(const char *name, int slot, const char *ipram,
				 const char *idram, const char *extfile){
  CfgWizardDBInquire *grp, *ctrl;
  CfgWizardDBField *grpfi;
  // create entry
  grp = new CfgWizardDBInquire("PixModuleGroup");
  m_currGroup = grp;
  m_currGroup->getDecName() = name;
  // add fields
  grpfi = new CfgWizardDBField("ModuleGroupName", STRING);
  grpfi->m_stringContent = name;
  insertField(grp, grpfi);
  // just to make things work
  grpfi = new CfgWizardDBField("duration", INT);
  grpfi->m_integerContent =   750;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("delay", INT);
  grpfi->m_integerContent =  255    ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("delayRange", INT);
  grpfi->m_integerContent =   23;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("accepts", INT);
  grpfi->m_integerContent =  16    ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("self", INT);
  grpfi->m_integerContent =   0       ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("latency", INT);
  grpfi->m_integerContent =  255   ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("calL1ADelay", INT);
  grpfi->m_integerContent =  240;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("binReset", INT);
  grpfi->m_integerContent =  0;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("softFE", INT);
  grpfi->m_integerContent =  10;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("shortSync", INT);
  grpfi->m_integerContent =  8;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("mediumSync", INT);
  grpfi->m_integerContent =  4;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("longSync", INT);
  grpfi->m_integerContent = 32;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("ECR", INT);
  grpfi->m_integerContent =  1;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("BCR", INT);
  grpfi->m_integerContent =  2;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("moduleInit", INT);
  grpfi->m_integerContent =  0  ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("vCal", INT);
  grpfi->m_integerContent =  255;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("cInject", INT);
  grpfi->m_integerContent = 1 ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("phi", INT);
  grpfi->m_integerContent =   40            ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("totThresholdMode", INT);
  grpfi->m_integerContent =  0 ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("totMinimum", INT);
  grpfi->m_integerContent =  0       ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("totTwalk", INT);
  grpfi->m_integerContent =   250      ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("totLeMode", INT);
  grpfi->m_integerContent =  20       ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("hitbus", INT);
  grpfi->m_integerContent =  1         ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("digitalInject", INT);
  grpfi->m_integerContent =  1;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("checkRegisters", INT);
  grpfi->m_integerContent =  1  ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("bandwidth", INT);
  grpfi->m_integerContent =  0;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("enableFEContinuityCheck", INT);
  grpfi->m_integerContent =  255 ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("enableFECrosscheck", INT);
  grpfi->m_integerContent =  255;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("sCurves", INT);
  grpfi->m_integerContent =  1        ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("totCalibration", INT);
  grpfi->m_integerContent =  1 ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("tdacTune", INT);
  grpfi->m_integerContent =  1       ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("tdacAlgorithm", INT);
  grpfi->m_integerContent =  1  ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("fdacTune", INT);
  grpfi->m_integerContent =  1       ;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("calibrationOption", INT);
  grpfi->m_integerContent =  1;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("chi2Cut", FLOAT);
  grpfi->m_floatContent = 2.54f;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("customScale", FLOAT);
  grpfi->m_floatContent = 3.45f;
  insertField(grp, grpfi);
  grpfi = new CfgWizardDBField("customOffset", FLOAT);
  grpfi->m_floatContent = 5.25f;
  insertField(grp, grpfi);
  // end
  // PixController
  ctrl = new CfgWizardDBInquire("PixController");
  ctrl->getDecName() = "RodPixController";
  insertInquire(grp, ctrl);
  grpfi = new CfgWizardDBField("ActualClassName", STRING);
  grpfi->m_stringContent = "RodPixController";
  insertField(ctrl, grpfi);
  grpfi = new CfgWizardDBField("Slot", INT);
  grpfi->m_integerContent = slot;
  insertField(ctrl, grpfi);
  grpfi = new CfgWizardDBField("IPRAMFile", STRING);
  grpfi->m_stringContent = ipram;
  insertField(ctrl, grpfi);
  grpfi = new CfgWizardDBField("IDRAMFile", STRING);
  grpfi->m_stringContent = idram;
  insertField(ctrl, grpfi);
  grpfi = new CfgWizardDBField("EXTFile", STRING);
  grpfi->m_stringContent = extfile;
  insertField(ctrl, grpfi);
  // insert group entry
  insertInquire(m_app, grp);
}
void CfgWizardDB::newModule(const char *fname, int in_link, int *out_link,
			    int latency, int delay, int bwidth, 
			    int modID,int grpID, bool active){
  CfgWizardDBField *modfi;

  // check if we have a group record
  if(m_currGroup==NULL)
    throw PixDBException("CfgWizardDB: No PixGroup record found.");

  // the actual record
  CfgWizardDBInquire *cfg = new CfgWizardDBInquire("PixModule");
  // open TurboDAQ config file and read module name
  openTurboDaqFiles(fname);
  std::string readString="";
  int i=0, imax=100000000;
  while(readString.find("Module string identifier")==std::string::npos
	&& i<imax){
    getLineDos(*m_moduleCfgFile,readString);
    i++;
  }
  if(i==imax)
    throw PixDBException("CfgWizardDB: module name not found in config file.");
  getLineDos(*m_moduleCfgFile,readString);
  cfg->getDecName() = readString;
  m_moduleCfgFile->close();
  // add to module group
  insertInquire(m_currGroup, cfg);

  // add general info
  modfi = new CfgWizardDBField("InputLink", INT);
  modfi->m_integerContent = in_link;
  insertField(cfg, modfi);
  for(int i=0; i<4;i++){
    std::string label = "OutputLink";
    std::stringstream id;
    id << (i+1);
    label += id.str();
    modfi = new CfgWizardDBField(label.c_str(), INT);
    modfi->m_integerContent = out_link[i];
    insertField(cfg, modfi);
  }
  modfi = new CfgWizardDBField("Latency", INT);
  modfi->m_integerContent = latency;
  insertField(cfg, modfi);
  modfi = new CfgWizardDBField("Delay", INT);
  modfi->m_integerContent = delay;
  insertField(cfg, modfi);
  modfi = new CfgWizardDBField("MCCOutputBandwidth", INT);
  modfi->m_integerContent = bwidth;
  insertField(cfg, modfi);
  modfi = new CfgWizardDBField("ModuleId", INT);
  modfi->m_integerContent = modID;
  insertField(cfg, modfi);
  modfi = new CfgWizardDBField("GroupId", INT);
  modfi->m_integerContent = grpID;
  insertField(cfg, modfi);
  modfi = new CfgWizardDBField("Active", INT);
  modfi->m_integerContent =  (int) active;
  insertField(cfg, modfi);

  // read TurboDAQ config file
  openTurboDaqFiles(fname);
  getLineDos(*m_moduleCfgFile,readString);
  getLineDos(*m_moduleCfgFile,readString);
  getLineDos(*m_moduleCfgFile,readString);
  getLineDos(*m_moduleCfgFile,readString);
  if(readString == "1"){ // module
    readModuleConfig(cfg, 16);
    // add extra MCC stuff
    recordIterator it;
    it = cfg->recordBegin();  
    modfi = new CfgWizardDBField("FEEN", INT);
    modfi->m_integerContent = 65535;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("CSR", INT);
    modfi->m_integerContent =  16 ;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("LV1", INT);
    modfi->m_integerContent =  3840 ;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("WFE", INT);
    modfi->m_integerContent =  0;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("WMCC", INT);
    modfi->m_integerContent = 0;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("CNT", INT);
    modfi->m_integerContent =  50;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("CAL", INT);
    modfi->m_integerContent =  1344;
    insertField(*it, modfi);
    modfi = new CfgWizardDBField("PEF", INT);
    modfi->m_integerContent =  0;
    insertField(*it, modfi);
    // add extra FE stuff
    it = cfg->recordBegin();it++;  
    for(int k = 0; k < 16; k++, it++){
      // first two not in cfg file???
      modfi = new CfgWizardDBField("FREQUENCY_CEU", INT);
      modfi->m_integerContent = 2;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("LATENCY", INT);
      modfi->m_integerContent = 255;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("CAP_MEASURE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("DAC_MON_LEAK_ADC", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("DAC_SPARE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("DAC_VCAL", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_BUFFER", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_BUFFER_BOOST", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_CINJ_HIGH", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_DIGITAL", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_EXTERNAL", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_LEAK_MEASURE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_MON_LEAK", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_SELF_TRIGGER", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_TEST_ANALOG_REF", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("ENABLE_VCAL_MEASURE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MODE_TOT_THRESH", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_ADC_REF", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_ID", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_IF", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_IL", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_IL2", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_IP", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_IP2", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_ITH1", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_ITH2", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_ITRIMIF", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_ITRIMTH", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_IVDD2", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_MON_LEAK_ADC", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_SPARE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MON_VCAL", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MUX_DO", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MUX_EOC", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MUX_MON_HIT", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("MUX_TEST_PIXEL", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("SPARE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("STATUS_MON_LEAK", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("THRESH_TOT_DOUBLE", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("THRESH_TOT_MINIMUM", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
      modfi = new CfgWizardDBField("WIDTH_SELF_TRIGGER", INT);
      modfi->m_integerContent = 0;
      insertField(*it, modfi);
    }
    // done!
  }
  else if(readString == "0"){ // single chip
    readSingleChipConfig(cfg);
  }
  else throw PixDBException("Module type decoding info not found");	
  m_moduleCfgFile->close();

  return;
}
