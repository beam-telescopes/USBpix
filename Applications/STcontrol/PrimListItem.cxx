#include "PrimListItem.h"
#include "ChipTest.h"

#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixController/PixScan.h>
#include <PixFe/PixFeStructures.h>
#include <PixFe/PixFeData.h>
#include <PixFe/PixFeI2.h>
#include <PixFe/PixFeI4A.h>
#include <PixFe/PixFeI4B.h>
#include <PixModule/PixModule.h>
#include <Fitting/FitClass.h>

#include <QComboBox>
#include <TMath.h>

PixFeI2  fei2 (0,0, "tmpfe2", 0);
PixFeI4A fei4a(0,0, "tmpfe4A", 0);
PixFeI4B fei4b(0,0, "tmpfe4B", 0);

PrlToolSubcfg::PrlToolSubcfg(DBInquire *, Config &cfg_in) : m_config(cfg_in){
}
PrlToolSubFEGR::PrlToolSubFEGR(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubFEGR::~PrlToolSubFEGR(){}
void PrlToolSubFEGR::setupGenConfig(){
  std::map<std::string, int> regmap;
  ConfGroup &grgrpa = fei4a.config().subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"];
  for(int i=0;i<grgrpa.size();i++) regmap[grgrpa[i].name()] = i;
  ConfGroup &grgrpb = fei4b.config().subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"];
  for(int i=0;i<grgrpb.size();i++){
    if(regmap[grgrpb[i].name()]==0 && grgrpb[i].name()!=grgrpa[0].name()){ // name doesn't exist, so only for FE-I4B
      regmap[grgrpb[i].name()] = 1000+i;
    }
  }
  // to do: use 1000+i items in execution of prim list
  m_config.addGroup("arguments");
  m_config["arguments"].addList("RegName", m_cfgType, 0, regmap, "Name of register to be written", true);
  m_config["arguments"].addInt("RegVal", m_cfgVal, 0, "Value to be written to register", true);
  m_config["arguments"].reset();
}

PrlToolSubFEPR::PrlToolSubFEPR(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubFEPR::~PrlToolSubFEPR(){}
void PrlToolSubFEPR::setupGenConfig(){
  std::map<std::string, int> regmap;
  Config &cfg = fei4a.config(); // pixel registers are identical in I4A and I4B, so doesn't matter which is used
  ConfGroup &grgrppr = cfg.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
  for(int i=0;i<grgrppr.size();i++) regmap[grgrppr[i].name()] = i;
  int offset = (int)regmap.size();
  ConfGroup &grgrptr = cfg.subConfig("Trim_0/Trim")["Trim"];
  for(int i=0;i<grgrptr.size();i++) regmap[grgrptr[i].name()] = i+offset;
  m_config.addGroup("arguments");
  m_config["arguments"].addList("RegName", m_cfgType, 0, regmap, "Name of register to be written", true);
  m_config["arguments"].addInt("RegVal", m_cfgVal, 0, "Value to be written to register", true);
  m_config["arguments"].reset();
}

PrlToolSubVcal::PrlToolSubVcal(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubVcal::~PrlToolSubVcal(){}
void PrlToolSubVcal::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addFloat("charge", m_charge, 20.e3, "charge in e to which VCAL should be set", true);
  m_config["arguments"].addBool("useChigh", m_chigh, false, "use Chigh for conversion (Clow used otherw.)", true);
  m_config["arguments"].reset();
}

PrlToolSubPwr::PrlToolSubPwr(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubPwr::~PrlToolSubPwr(){}
void PrlToolSubPwr::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addString("dcsName", m_dcsName, "VDDD", "Full name of DCS device", true);
  m_config["arguments"].addInt("dcsChan", m_dcsChan, -99, "DCS channel ID (whole device if -99, set to index if -1)", true);
  m_config["arguments"].addBool("on_or_off", m_on_or_off, false, "false: turn off; true: turn on", true);
  m_config["arguments"].reset();
}

PrlToolSubSetDcs::PrlToolSubSetDcs(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubSetDcs::~PrlToolSubSetDcs(){}
void PrlToolSubSetDcs::setupGenConfig(){
  std::map<std::string, int> setmap;

  //THERESA

  setmap["voltage"] = 0;
  setmap["currentlim"] = 1;
  setmap["position"] = 2;
  setmap["maxspeed"] = 3;
  setmap["settemperature"] = 4;
  setmap["useexternaltempsensor"] = 5;
  m_config.addGroup("arguments");
  m_config["arguments"].addString("dcsName", m_dcsName, "VDDD", "Full name of DCS channel", true);
  m_config["arguments"].addList("dcsSetType", m_dcsSetType, 0, setmap, "Type of setting to be done", true);
  m_config["arguments"].addFloat("setVal", m_setVal, 0., "Value to be set on channel/device", true);

  m_config["arguments"].addFloat("setOffset", m_setOffset, 0., "Offset to be set on channel/device", true);

	m_config["arguments"].addInt("dcsChan", m_dcsChan, -99, "DCS channel ID (whole device if -99, set to index if -1)", true);

  m_config["arguments"].reset();
}

PrlToolSubGetDcs::PrlToolSubGetDcs(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubGetDcs::~PrlToolSubGetDcs(){}

void PrlToolSubGetDcs::setupGenConfig(){
  std::map<std::string, int> setmap;
  setmap["voltage"] = 0;
  setmap["current"] = 1;
  setmap["temperature"] = 2;
  m_config.addGroup("arguments");
  m_config["arguments"].addString("dcsName", m_dcsName, "VDDD", "Full name of DCS channel (of device if channel ID != -99)", true);
  m_config["arguments"].addInt("dcsChan", m_dcsChan, -99, "DCS channel ID (ignored if -99, set to index if -1)", true);
  m_config["arguments"].addList("dcsGetType", m_dcsGetType, 0, setmap, "Type of reading to be done", true);
  m_config["arguments"].addString("storeName", m_storeName, "VDDD_READ 0", "Name of data field into which to write result", true);
  m_config["arguments"].addFloat("minCut", m_minCut, -9999999999.f,
				 "If read value is below this, the \"skip rest of list\"-flag is raised", true);
  m_config["arguments"].addFloat("maxCut", m_maxCut, 9999999999.f,
				 "If read value is above this, the \"skip rest of list\"-flag is raised", true);
  m_config["arguments"].reset();
}

PrlToolSubSendDcsCmd::PrlToolSubSendDcsCmd(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubSendDcsCmd::~PrlToolSubSendDcsCmd(){
}
void PrlToolSubSendDcsCmd::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addString("dcsName", m_dcsName, "none", "Full name of DCS device", true);
  m_config["arguments"].addString("dcsCmd", m_command, "*IDN?", "Command string to be sent to device", true);
  m_config["arguments"].addBool("readResponse", m_readResp, false, "Read response from DCS device after command was sent", true);
  m_config["arguments"].reset();
}

PrlToolSubFitScan::PrlToolSubFitScan(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubFitScan::~PrlToolSubFitScan(){}
void PrlToolSubFitScan::setupGenConfig(){
  FitClass fc;
  std::map<std::string, int> funcmap;
  for(int i=0;i<999;i++){
    std::string fname = fc.getFuncTitle(i);
    if(fname=="") break;
    funcmap[fname] = i;
  }
  funcmap["FEI3 CapMeasure"];
  PixLib::PixScan ps;
  //std::map<std::string, int> histoMap = ps.getHistoTypes();
  std::map<std::string, int> storeMap;
  storeMap["not stored"] = 0;
  storeMap["VCAL parameters"] = 1;
  storeMap["FE-I3 injection capacitances"] = 2;
  storeMap["CAP0"] = 3;
  storeMap["CAP1"] = 4;
  storeMap["CAP0+CAP1"] = 5;
  storeMap["all CAPx"] = 6;

  m_fitParPreset.push_back(1.6f);
  m_fitParPreset.push_back(-0.0009f);

  m_config.addGroup("arguments");
  m_config["arguments"].addString("scanName", m_scanName, "unknown", "Label of scan of which a hist/graph should be fitted", true);
  m_config["arguments"].addList("funcType", m_funcType, 0, funcmap, "Function to be fitted", true);
  m_config["arguments"].addList("histoType", m_histoType, 0, ps.getHistoTypes(), "Histogram/graph to be fitted", true);
  m_config["arguments"].addVector("fitParPreset", m_fitParPreset, m_fitParPreset, "Initial guesses of fit parameters", true);
  m_config["arguments"].addFloat("fitRangeMin", m_fitRgMin, 0., "Lower limit of fit rage; min=max=0. -> no limit", true);
  m_config["arguments"].addFloat("fitRangeMax", m_fitRgMax, 0., "Upper limit of fit rage; min=max=0. -> no limit", true);
  m_config["arguments"].addList("storeType", m_storeType, 0, storeMap, "Write the fit parameters to FE config?", true);
  m_config["arguments"].addFloat("offset", m_offset, 0., "Offset to add to stored val (only CAP0, CAP1)", true);
  m_config["arguments"].addFloat("conversion", m_conversion, 1., "Conversion of grad to stored val (only CAP0, CAP1)", true);
  m_config["arguments"].reset();
}
PrlToolSubAnaGraph::PrlToolSubAnaGraph(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubAnaGraph::~PrlToolSubAnaGraph(){
}
void PrlToolSubAnaGraph::setupGenConfig(){
  // prepare lists/default arg's
  std::map<std::string, int> storemap;
  storemap["scanned register"] = 1;
  storemap["file"] = 2;
  storemap["mod. HV mask"] = 3;
  std::map<std::string, int> anamap;
  anamap["Find best match"] = 1;
  anamap["Any |value|>limit?"] = 2;
  m_anaArgs.push_back(0.);

  m_config.addGroup("arguments");
  m_config["arguments"].addString("scanName", m_scanName, "unknown", "Label of scan of which a hist/graph should be fitted", true);
  m_config["arguments"].addList("anaAction", m_anaAction, 0, anamap, "Analysis action to be used", true);
  m_config["arguments"].addVector("anaArgs", m_anaArgs, m_anaArgs, "List of arguments (e.g. target values)", true);
  m_config["arguments"].addList("storeType", m_storeType, 0, storemap, "Write analysis result to ...", true);
  m_config["arguments"].addInt("FEindex", m_feIndex, 0, "Index of FE to which register is written (if reg. write selected)", true);
  m_config["arguments"].reset();
}

PrlToolSubGenPixMask::PrlToolSubGenPixMask(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubGenPixMask::~PrlToolSubGenPixMask(){}
void PrlToolSubGenPixMask::setupGenConfig(){
  PixLib::PixScan ps;
  std::map<std::string, int> histoMap = ps.getHistoTypes();
  std::map<std::string, int> maskMap, combMap;
  maskMap["ENABLE"] = 0;
  maskMap["ILEAK"] = 1;
  maskMap["CAP0"] = 2;
  maskMap["CAP1"] = 3;

  combMap["overwrite"] = 0;
  combMap["AND with existing mask"] = 1;
  combMap["OR with existing mask"] = 2;

  m_config.addGroup("arguments");
  m_config["arguments"].addString("scanName", m_scanName, "unknown", "Label of scan of with histogram for mask generation", true);
  m_config["arguments"].addList("histoType", m_histType, (int)PixScan::OCCUPANCY, histoMap, "Histogram type to be analysed", true);
  m_config["arguments"].addFloat("minCut", m_cutMin, 1.f, "Min. cut value (cut passed -> mask=1)", true);
  m_config["arguments"].addFloat("maxCut", m_cutMax, 1.f, "Max. cut value (cut passed -> mask=1)", true);
  m_config["arguments"].addList("mask", m_maskType, 0, maskMap, "FE mask to write to", true);
  m_config["arguments"].addList("maskComb", m_maskComb, 0, combMap, "Type of writing to mask", true);
  m_config["arguments"].reset();
}

PrlToolSubWait::PrlToolSubWait(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubWait::~PrlToolSubWait(){}
void PrlToolSubWait::setupGenConfig(){
  int type = ((ConfInt&)m_config["specific"]["toolType"]).value();
  m_config.addGroup("arguments");
  m_config["arguments"].addInt("waitTime", m_waitTime, 1, (type==PrimListItem::GEN_WAIT)?"time in min. to wait":"time in msec. to wait", true);
  m_config["arguments"].reset();
}

PrlToolSubSendMail::PrlToolSubSendMail(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubSendMail::~PrlToolSubSendMail(){
}
void PrlToolSubSendMail::setupGenConfig(){
	
  std::map<std::string, int> connectionTypeList;
  connectionTypeList["TcpConnection"]     = 0;
  connectionTypeList["SslConnection"] = 1;
  connectionTypeList["TlsConnection"] = 2;

  std::map<std::string, int> authentificationTypeList;
  authentificationTypeList["AuthPlain"]     = 0;
  authentificationTypeList["AuthLogin"] = 1;
	
  m_config.addGroup("arguments");
  m_config["arguments"].addString("smtpServerName", m_smtpServerName, "none", "SMTP server name", true);
  m_config["arguments"].addString("userName", m_userName, "none", "user name", true);
  m_config["arguments"].addString("password", m_password, "none", "password", true);
  m_config["arguments"].addString("subject", m_subject, "none", "subject", true);
  m_config["arguments"].addString("message", m_message, "none", "message", true);
  m_config["arguments"].addString("sender", m_sender, "none", "sender address", true);
  m_config["arguments"].addString("senderName", m_senderName, "none", "sender name", true);
  m_config["arguments"].addString("recipient", m_recipient, "none", "recipient address", true);
  m_config["arguments"].addString("recipientName", m_recipientName, "none", "recipient name", true);	
  m_config["arguments"].addInt("port", m_port, 1, "SMTP server port", true);	
  m_config["arguments"].addList("connectionType", m_connectionType, 0, connectionTypeList, "connection type", true);
  m_config["arguments"].addList("authentificationType", m_authentificationType, 0, authentificationTypeList, "authentification type", true);
  m_config["arguments"].reset();
}

PrlToolSubWaitForSignal::PrlToolSubWaitForSignal(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubWaitForSignal::~PrlToolSubWaitForSignal(){}
void PrlToolSubWaitForSignal::setupGenConfig(){
  std::map<std::string, int> channelMap, logicMap;
  channelMap["RX0"] = 0;
  channelMap["RX1"] = 1;
  channelMap["RX2"] = 2;
  channelMap["EXT_TRG"] = 3;
  logicMap["OR"] = 0;
  logicMap["AND"] = 1;

  m_config.addGroup("arguments");
  m_config["arguments"].addList("channel", m_waitChannel, 0, channelMap, "USBPix-channel from which to read", true);
  m_config["arguments"].addList("combination", m_ctrlCombi, 0, logicMap, "Logical combination if more than 1 USBPix system", true);
  m_config["arguments"].addBool("waitOnOne", m_waitOnOne, true, "Wait if signal is high if true, low if false", true);
  m_config["arguments"].reset();
}
PrlToolSubConfigureMods::PrlToolSubConfigureMods(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubConfigureMods::~PrlToolSubConfigureMods(){}
void PrlToolSubConfigureMods::setupGenConfig(){
  std::map<std::string, int> cfgTypeList;
  cfgTypeList["ALL"]     = 0;
  cfgTypeList["GR only"] = 1;
  cfgTypeList["PR only"] = 2;
  
  std::map<std::string,int> typeMap;
  typeMap["current configuration"] = ChipTest::CURRENTCFG;
  typeMap["all 0's"]               = ChipTest::ALLZERO;
  typeMap["all 1's"]               = ChipTest::ALLONE;
  typeMap["odd rows 1"]            = ChipTest::ALLODD;
  typeMap["even rows 1"]           = ChipTest::ALLEVEN;
  typeMap["all 1's part A"]        = ChipTest::ALLONE_A;
  typeMap["all 1's part B"]        = ChipTest::ALLONE_B;
  typeMap["all 1's part C"]        = ChipTest::ALLONE_C;
  typeMap["all 1's part D"]        = ChipTest::ALLONE_D;
  typeMap["all 1's part E"]        = ChipTest::ALLONE_E;
  typeMap["all 1's part F"]        = ChipTest::ALLONE_F;
  typeMap["random"]                = ChipTest::RANDOM;

  std::map<std::string,int> pixNames;
  pixNames["ALL"] = 0;
  Config &feconf = fei4a.config(); // pixel registers are identical in I4A and I4B, so doesn't matter which is used
  ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];
  ConfGroup &maskgrp = feconf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
  for(int i=0;i<(int)maskgrp.size();i++){
    std::string prname = maskgrp[i].name();
    prname.erase(0,std::string("PixelRegister_").length());
    pixNames[prname] = i+1;
  }
  // trim registers: >1 bit, must add bit index to name
  int maxval, nmask=pixNames.size()+1;
  for(int i=0;i<(int)trimgrp.size();i++){
    std::string prname = trimgrp[i].name();
    prname.erase(0,std::string("Trim_").length());
    if(!fei4a.getTrimMax(prname,maxval)) maxval=0;
    maxval = (int)TMath::Log2((double)(maxval+1));
    for(int j=0;j<maxval;j++){
      std::stringstream a;
      a << j;
      pixNames[prname+a.str()] = nmask;
      nmask++;
    }
  }

  m_config.addGroup("arguments");
  m_config["arguments"].addList("cfgtype", m_configType, 0, cfgTypeList, "Selects which part of registers is configured", true);
  m_config["arguments"].addList("patternType", m_patternType, 0, typeMap, "Pattern to be written to register", true);
  m_config["arguments"].addInt ("DCtoWrite", m_DCtoWrite, -1, "PR only: DC to which pixel reg. is written (-1: all DCs)", true);
  m_config["arguments"].addList("latch", m_latch, 0, pixNames, "PR only: latch to be written to register", true);
  m_config["arguments"].reset();
}
PrlToolSubSaveCfg::PrlToolSubSaveCfg(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubSaveCfg::~PrlToolSubSaveCfg(){}
void PrlToolSubSaveCfg::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addString("extString", m_extString, "", "String added to data file name when creating cfg. file name", true);
  m_config["arguments"].addBool("activeOnly", m_activeOnly, true, "Save only cfg. of active modules", true);  
  m_config["arguments"].reset();
}
PrlToolSubSetFeMode::PrlToolSubSetFeMode(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubSetFeMode::~PrlToolSubSetFeMode(){}
void PrlToolSubSetFeMode::setupGenConfig(){
  m_config.addGroup("arguments");
  std::map<std::string,int> typeMap;
  typeMap["Conf. mode"] = 0;
  typeMap["Run mode"] = 1;
  m_config["arguments"].addList("mode", m_mode, 0, typeMap, "Set FE operation mode", true);
  m_config["arguments"].reset();
}
PrlToolSubLoadCfg::PrlToolSubLoadCfg(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubLoadCfg::~PrlToolSubLoadCfg(){}
void PrlToolSubLoadCfg::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addString("fileName", m_fname, "", "Path to config file to be loaded", true);
  m_config["arguments"].reset();
}
PrlToolSubLoadModCfg::PrlToolSubLoadModCfg(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
}
PrlToolSubLoadModCfg::~PrlToolSubLoadModCfg(){}
void PrlToolSubLoadModCfg::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addBool("SetChipSN", m_setChipSN, true, "Set Chip SN from wafer probing info after reloading cfg.", true);
  m_config["arguments"].addInt("Chip_SN", m_chipSN, -1, "Chip Serial Number (to be set externally)", false);
  m_config["arguments"].reset();
}

PrlToolSubResetMods::PrlToolSubResetMods(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
  //m_config["arguments"].reset();
}
PrlToolSubResetMods::~PrlToolSubResetMods(){}
void PrlToolSubResetMods::setupGenConfig(){
  m_config.addGroup("arguments");
  std::map<std::string,int> typeMap;
  typeMap["soft global reset"] = 0;
  typeMap["hard reset (pulse)"] = 1;  
  typeMap["hard reset (static ON)"] = 2;  
  typeMap["hard reset (static OFF)"] = 3;
  typeMap["event counter reset"] = 4;
  typeMap["bunch counter reset"] = 5;
  m_config["arguments"].addList("resetType", m_resetType, 1, typeMap, "Type of reset to be sent", true);
  m_config["arguments"].reset();
}

PrlToolSubReadGADC::PrlToolSubReadGADC(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
  //m_config["arguments"].reset();
}
PrlToolSubReadGADC::~PrlToolSubReadGADC(){}
void PrlToolSubReadGADC::setupGenConfig(){
  m_config.addGroup("arguments");
  std::map<std::string,int> typeMap;
  typeMap["Temperature"] = 0;
  typeMap["GADC reference voltage"] = 1;  
  typeMap["Analog GND"] = 2;  
  typeMap["Analog MUX output"] = 3;
  typeMap["Analog regulator current"] = 4;
  typeMap["10 bit DAC output (Pulser input)"] = 5;
  typeMap["1/2 regulated analog voltage"] = 6;  
  typeMap["Leakage current"] = 7;  
  m_config["arguments"].addList("readType", m_ReadType, 1, typeMap, "Type of value to be read", true);
  m_config["arguments"].reset();
}
PrlToolSubSendGlobpls::PrlToolSubSendGlobpls(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
  //m_config["arguments"].reset();
}
PrlToolSubSendGlobpls::~PrlToolSubSendGlobpls(){}
void PrlToolSubSendGlobpls::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addInt("length", m_length, 10, "Length of global pulse", true);
  m_config["arguments"].reset();
}

PrlToolSubSelBiMod::PrlToolSubSelBiMod(DBInquire *inq, Config &cfg_in) : PrlToolSubcfg(inq, cfg_in){
  setupGenConfig();
  if(inq!=0) m_config.read(inq);
  m_config["arguments"].reset();
}
PrlToolSubSelBiMod::~PrlToolSubSelBiMod(){}
void PrlToolSubSelBiMod::setupGenConfig(){
  m_config.addGroup("arguments");
  m_config["arguments"].addInt("channel", m_channel, -1, "Channel on burn-in card; if <0, prim. list iteration index is used", true);
  m_config["arguments"].addBool("setIref", m_setIref, false, 
				"Set Iref to selected channel automatically (intended for HV switcher usage)", true);
  m_config["arguments"].addBool("skipIfNotIref", m_skipIfNotIref, false, 
			"If in index mode, skip to next ID if current index is not set in Iref", true);
  m_config["arguments"].addBool("WrapGrpId", m_wrapGrpId, false, "if channel from prim.list index, wrap to next grp. if > last mod. id",
				true);
  m_config["arguments"].reset();
}


PrimListItem::PrimListItem(PrimListTypes type, int index, const char *label, PixScan *in_ps)
  : m_type((int)type), m_index(index)
{
  m_label=label;
  m_subCfg = 0;
  m_scanLabel = "";
  m_subType = -1;
  setupGenConfig();
  setupSpecConfig(in_ps);
  m_scanLabel = m_label;
  setupSubcfg(0);
}
PrimListItem::PrimListItem(DBInquire *inq)
{
  m_index = 0;
  m_type = (int)DEF_SCAN;
  m_label="";
  m_scanLabel = "";
  m_subType = -1;
  m_subCfg = 0;

  setupGenConfig();
  m_config->read(inq);
  setupSpecConfig();
  m_config->read(inq);
  setupSubcfg(inq);
  m_config->read(inq);
}
PrimListItem::PrimListItem(PrimListItem &inItem, int index) : m_type(inItem.m_type), m_index(index), m_label("Copy of "+inItem.m_label),
							      m_scanLabel(inItem.m_scanLabel), m_subType(inItem.m_subType){
  m_subCfg = 0;
  setupGenConfig();
  *m_config = inItem.config();
  setupSpecConfig(inItem.m_ps, false);
  *m_config = inItem.config();
  setupSubcfg(0);
  *m_config = inItem.config();
  m_label = "Copy of "+inItem.m_label;
  m_index = index;
}
PrimListItem::~PrimListItem()
{
  // must erase PixScan's config from our config, otherwise this would get
  // deleted again when deleting PixScan
  if(m_config!=0) m_config->removeConfig(std::string("ScanConfig/ScanConfig"));
  delete m_config;
  delete m_ps;
  delete m_subCfg;

}
std::string PrimListItem::getTypeLabel(PrimListTypes type)
{
  const std::string dummy="none";
  for(std::map<std::string, int>::iterator it=m_typeDef.begin(); it!=m_typeDef.end(); it++){
    if(it->second==(int)type)
      return it->first;
  }
  return dummy;
}
std::string PrimListItem::getToolLabel(PrimListTools type)
{
  const std::string dummy="none";
  for(std::map<std::string, int>::iterator it=m_prlTools.begin(); it!=m_prlTools.end(); it++){
    if(it->second==(int)type)
      return it->first;
  }
  return dummy;
}
void PrimListItem::setupGenConfig(bool  doreset)
{

  m_typeDef["default PixScan"] = DEF_SCAN;
  m_typeDef["custom PixScan"]  = CUST_SCAN;
  m_typeDef["chip test"]       = CHIP_TEST;
  m_typeDef["tool"]            = TOOL;

  // fill list of primlist tools
  m_prlTools["init. controller"]     = INIT_ROD;
  m_prlTools["initialise DCS"]       = INIT_DCS;
  m_prlTools["reset modules"]        = RESET_MODS;
  m_prlTools["configure modules"]    = CFG_MODS;
  m_prlTools["set VCAL from Q"]      = SET_VCAL;
  m_prlTools["link test"]            = LINK_CHECK;
  m_prlTools["disable failed"]       = DISABLE_FAILED;
  m_prlTools["reload entire cfg."]   = RELOAD_CFG;
  m_prlTools["load cfg. from file"]  = LOAD_NEW_CFG;
  m_prlTools["set FE globreg"]       = SET_FE_GR;
  m_prlTools["set FE pixreg"]        = SET_FE_PR;
  m_prlTools["turn on/off supply"]   = DCS_PWR;
  // can't replace above item, otherwise not backward compatible
  m_prlTools["turn on/off device"]   = DCS_ONOFF; 
  m_prlTools["set DCS value"]        = DCS_SET;
  m_prlTools["get DCS value"]        = DCS_GET;
  m_prlTools["get time stamp"]       = GET_TIME;
  m_prlTools["fit scan result"]      = FIT_SCAN;
  m_prlTools["analyse scan graph"]   = ANA_GRAPH;
  m_prlTools["generate pixel mask"]  = GEN_PIXMASK;
  m_prlTools["wait (minutes)"]       = GEN_WAIT;
  m_prlTools["wait (msec)"]          = GEN_WAIT_MSEC;
  m_prlTools["wait for ext. signal"] = SIG_WAIT;
  m_prlTools["request serv. rec's"]  = GET_SRVREC;
  m_prlTools["save config to file"]  = SAVE_CFG;
  m_prlTools["reload module cfg."]   = LOAD_MOD_CFG;
  m_prlTools["set FE oper. mode"]    = SET_FE_MODE;
  m_prlTools["send cmd. to DCS device"] = SEND_DCS_CMD;
  m_prlTools["Read EPROM"]           = RD_EPROM;
  m_prlTools["burn GR values to EPROM"] = BURN_EPROM;
  m_prlTools["read GADC to GR"]      = READ_GADC;
  m_prlTools["send global pulse"]    = SEND_GLOBPLS;
  m_prlTools["select burn-in channel"] = SELECT_BI_MOD;
  m_prlTools["send mail"] = SEND_MAIL;
  m_abortIfError = false;

  m_config = new Config("prlcfg");
  Config &conf = *m_config;
  conf.addGroup("general");
//  conf["general"].addList("type", (int)m_type, (int)m_type, m_typeDef, "type of action of primlist item", false);
  conf["general"].addList("type", m_type, m_type, m_typeDef, "type of action of primlist item", false);
  conf["general"].addInt("index", m_index, m_index, "index of item in primlist", false);
  conf["general"].addBool("abortIfError", m_abortIfError, false, "abort execution of primlist if this item caused an error", true);
  conf["general"].addBool("executeIfSkip", m_executeIfSkip, false, 
			  "Still execute this item if a previous tool indicates to skip rest of list", true);
  conf["general"].addString("label", m_label, m_label, "label of action as shown in the list", true);
  if(doreset) conf.reset();

  if(doreset) {
    m_subType = -1;
    m_ps = 0;
    m_ct = 0;
  }

  return;
}
void PrimListItem::setupSpecConfig(PixScan *in_ps, bool  doreset)
{
  Config &conf = *m_config;

  switch(m_type){
  case DEF_SCAN:{
    conf.addGroup("specific");
    // create temporary PixScan object to get list of scan types
    m_ps = new PixScan(PixScan::DIGITAL_TEST, PixModule::PM_FE_I4A);
    m_scanTypes = m_ps->getScanTypes();
    delete m_ps; m_ps=0;
    conf["specific"].addList("scanType", m_subType, PixScan::DIGITAL_TEST, m_scanTypes, "PixScan default config type", true);
    conf["specific"].addBool("scanIsI4", m_scanIsI4, true, "Set: FE-I4A/B, unset: FE-I2/3", true);
    conf["specific"].addString("scanLabel", m_scanLabel, "Primlist Scan 0", "Label used for storing scan result in file", true);
    // NB: must be identical to options in PixScanPanel - better use common def. via engine?
    std::map<std::string, int> readDcsOpts;
    readDcsOpts["never"] = 0;
    readDcsOpts["after start"] = 1;
    conf["specific"].addList("readDCS", m_readDcs, 0, readDcsOpts, "Type of DCS readings during scan", true);
    conf["specific"].addInt("timeout", m_timeout, -1, "Time in min. after which scan w/ src-flag=true is aborted (if <0: no abort)", true);
    if(doreset) conf["specific"].reset();
    break;}
  case CUST_SCAN:{
    conf.addGroup("specific");
    conf["specific"].addString("scanLabel", m_scanLabel, "Primlist Scan 0", "Label used for storing scan result in file", true);
    // NB: must be identical to options in PixScanPanel - better use common def. via engine?
    std::map<std::string, int> readDcsOpts;
    readDcsOpts["never"] = 0;
    readDcsOpts["after start"] = 1;
    conf["specific"].addList("readDCS", m_readDcs, 0, readDcsOpts, "Type of DCS readings during scan", true);
    conf["specific"].addInt("timeout", m_timeout, -1, "Time in min. after which scan w/ src-flag=true is aborted (if <0: no abort)", true);
    if(doreset) conf["specific"].reset();

    if(in_ps==0)
      m_ps = new PixScan(PixScan::DIGITAL_TEST, PixModule::PM_FE_I4A);
    else
      m_ps = new PixScan(*in_ps);
    // needed to find this sub-config in a DB file later
    m_ps->config().m_confName = "ScanConfig/ScanConfig";
    conf.addConfig(&(m_ps->config()));
    break;}
  case TOOL:
    conf.addGroup("specific");
    conf["specific"].addList("toolType", m_subType, INIT_ROD, m_prlTools, "tool action type", true);
    if(doreset) conf["specific"].reset();
    break;
  case CHIP_TEST:{
    conf.addGroup("specific");
    ChipTest ctdummy(ChipTest::CTDUMMY);
    std::map<std::string, int> types = ctdummy.getTypeList();
    conf["specific"].addList("chipTestType", m_subType, ChipTest::GLOBALREG, ctdummy.getTypeList(), "type of chip test", true);
    conf["specific"].addBool("raiseSkipIfFailed", m_raiseSkipIfFailed, true, 
			  "Raise flag to skip remaining primitive list items if test fails", true);
    if(doreset) conf["specific"].reset();
    break;}
  default:
    break;
  }
  return;
}
void PrimListItem::setupSubcfg(DBInquire *inq){
  if(m_type!=(int)TOOL && m_type!=(int)CHIP_TEST) return; // only valid for tools and chip tests
  Config *oldcfg = m_config;
  PixScan *oldps = m_ps;
  if(m_ct!=0) m_config->m_config.clear();
  setupGenConfig(false);
  (*oldcfg)["general"].copy((*m_config)["general"]);
  setupSpecConfig(oldps, false);
  (*oldcfg)["specific"].copy((*m_config)["specific"]);
  delete m_subCfg; 
  delete m_ct; m_ct=0;
  delete oldps;
  delete oldcfg;
  if(m_type==(int)TOOL){
    switch(m_subType){
    case SET_VCAL:
      m_subCfg = new PrlToolSubVcal(inq, *m_config);
      break;
    case SET_FE_GR:
      m_subCfg = new PrlToolSubFEGR(inq, *m_config);
      break;
    case SET_FE_PR:
      m_subCfg = new PrlToolSubFEPR(inq, *m_config);
      break;
    case DCS_PWR:
      m_subCfg = new PrlToolSubPwr(inq, *m_config);
      break;
    case DCS_SET:
      m_subCfg = new PrlToolSubSetDcs(inq, *m_config);
      break;
    case DCS_GET:
      m_subCfg = new PrlToolSubGetDcs(inq, *m_config);
      break;
    case FIT_SCAN:
      m_subCfg = new PrlToolSubFitScan(inq, *m_config);
      break;
    case ANA_GRAPH:
      m_subCfg = new PrlToolSubAnaGraph(inq, *m_config);
      break;
    case GEN_PIXMASK:
      m_subCfg = new PrlToolSubGenPixMask(inq, *m_config);
      break;
    case GEN_WAIT:
    case GEN_WAIT_MSEC:
      m_subCfg = new PrlToolSubWait(inq, *m_config);
      break;
    case SIG_WAIT:
      m_subCfg = new PrlToolSubWaitForSignal(inq, *m_config);
      break;
    case CFG_MODS:
      m_subCfg = new PrlToolSubConfigureMods(inq, *m_config);
      break;
    case SAVE_CFG:
      m_subCfg = new PrlToolSubSaveCfg(inq, *m_config);
      break;
    case SET_FE_MODE:
      m_subCfg = new PrlToolSubSetFeMode(inq, *m_config);
      break;
    case LOAD_NEW_CFG:
      m_subCfg = new PrlToolSubLoadCfg(inq, *m_config);
      break;
    case RESET_MODS:
      m_subCfg = new PrlToolSubResetMods(inq, *m_config);
      break;
    case SEND_DCS_CMD:
      m_subCfg = new PrlToolSubSendDcsCmd(inq, *m_config);
      break;
    case LOAD_MOD_CFG:
      m_subCfg = new PrlToolSubLoadModCfg(inq, *m_config);
      break;
    case READ_GADC:
      m_subCfg = new PrlToolSubReadGADC(inq, *m_config);
      break;
    case SEND_GLOBPLS:
      m_subCfg = new PrlToolSubSendGlobpls(inq, *m_config);
      break;
    case SELECT_BI_MOD:
      m_subCfg = new PrlToolSubSelBiMod(inq, *m_config);
      break;
    case SEND_MAIL:
      m_subCfg = new PrlToolSubSendMail(inq, *m_config);
      break;
    default: // any other type: no argument
      m_subCfg=0;
    }
  } else{
    m_ct = ChipTest::make((ChipTest::chipTestType) m_subType);
    // needed to find this in a DB file later
    m_ct->config().m_confName = "ChipTest/ChipTest";
    m_config->addConfig(&(m_ct->config()));
  }
}
