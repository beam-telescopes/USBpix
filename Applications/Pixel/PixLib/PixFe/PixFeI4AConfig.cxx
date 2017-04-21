/////////////////////////////////////////////////////////////////////
// PixFeI4AConfig.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 09/02/10  Version 0.1 (JGK)
// 04/11/11  Version 1.0 (JGK) - renamed from FeI4 to FeI4A
//

//! PixFeConfig implementation for FE-I4A

#include <math.h>
#include <iomanip>
#include <sstream>
#include <typeinfo>

#include "Bits/Bits.h"
#include "Config/Config.h"
#include "PixFe/PixFeExc.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeData.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixFe/PixFeI4AConfig.h"

using namespace PixLib;


//! Constructor
PixFeI4AConfig::PixFeI4AConfig(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number) {
  // Members initialization
  m_db = db;
  m_dbInquire = dbInquire;
  m_name = name;
  m_className = "PixFeI4A";

  // Setup config structures
  setupStructures();
    
  // Setup default data
  m_data.setFeStructures(&m_structures);
  m_data.setIndex(number);
  m_data.setCmdAddr(number);
  setupData();

  // Setup config object
  setupConfigObject();
  // extra for FE-I4A
  Config &conf = *m_conf;
  conf["Misc"].addFloat("CInjMed", m_data.getCInjMed(), m_data.getCInjMed(), "Charge injection medium", true);
  conf["Misc"].addFloat("DelayCalib", m_delayCalib, 1., "Calibration of PlsrDelay to ns", true);

  ConfMask<float> tSettings(m_structures.nCol(), m_structures.nRow(), 1e5, 0);

  try {
  bool status = false;
  for(PixFeData::nameVector::iterator it = m_data.calibBegin(); it != m_data.calibEnd(); ++it)	//set the TOT calibration histos
  	conf["Misc"].addMatrix(*it, m_data.getCalib(*it, status), tSettings, "TOT calibration", true);
  } catch(PixFeExc &FeExc) {
  	FeExc.dump(std::cout);
  	throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
  }

  conf.reset();

  // Load initial config from database
  if(m_db!=0 && m_dbInquire!=0) loadConfig("PHYSICS");
}

//! Copy constructor
PixFeI4AConfig::PixFeI4AConfig(const PixFeConfig &c) {
  // Members initialization
  m_className = "PixFeI4A";

  // Try dynamic casting
  try {
    const PixFeI4AConfig& cfg = dynamic_cast<const PixFeI4AConfig&>(c);
    
    // Setup config structures
    setupStructures();
    m_data.setFeStructures(&m_structures);

    // Copy database information
    m_db = cfg.m_db;
    m_dbInquire = cfg.m_dbInquire;
    m_name = cfg.m_name;

    // Copy config data
    m_data = cfg.m_data;

    // Setup config object
    setupConfigObject();
    // extra for FE-I4A
    Config &conf = *m_conf;
    conf["Misc"].addFloat("CInjMed", m_data.getCInjMed(), m_data.getCInjMed(), "Charge injection medium", true);
    conf["Misc"].addFloat("DelayCalib", m_delayCalib, 1., "Calibration of PlsrDelay to ns", true);

    ConfMask<float> tSettings(m_structures.nCol(), m_structures.nRow(), 1e5, 0);
	try {
		bool status = false;
		for(PixFeData::nameVector::iterator it = m_data.calibBegin(); it != m_data.calibEnd(); ++it)	//set the TOT calibration histos
			conf["Misc"].addMatrix(*it, m_data.getCalib(*it, status), tSettings, "TOT calibration", true);
	} catch(PixFeExc &FeExc) {
		FeExc.dump(std::cout);
		throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
	}

    conf.reset();
  }
  catch(bad_cast) {
    throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
    return;
  }
}

//! Destructor
PixFeI4AConfig::~PixFeI4AConfig() {
}


//! Assignment operator
PixFeConfig& PixFeI4AConfig::operator = (const PixFeConfig& c) {

  if(this != &c) {
    // Try dynamic casting
    try {
      const PixFeI4AConfig& cfg = dynamic_cast<const PixFeI4AConfig&>(c);
      m_data = cfg.m_data;
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return *this;
    }
  }

  return *this;
}


//! Global register copy method
void PixFeI4AConfig::copyGlobReg(const PixFeConfig& source) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI4AConfig& cfg = dynamic_cast<const PixFeI4AConfig&>(source);
      m_data.copyGlobReg(cfg.m_data);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}

//! Pixel register copy method
void PixFeI4AConfig::copyPixReg(const PixFeConfig& source, std::string name) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI4AConfig& cfg = dynamic_cast<const PixFeI4AConfig&>(source);
      m_data.copyPixReg(cfg.m_data, name);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}

//! Trim copy method
void PixFeI4AConfig::copyTrim(const PixFeConfig& source, std::string name) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI4AConfig& cfg = dynamic_cast<const PixFeI4AConfig&>(source);
      m_data.copyTrim(cfg.m_data, name);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}


//! Setup FE configuration structures elements
void PixFeI4AConfig::setupStructures() {
  // Setup FE general parameters
  m_structures.nCol(80);
  m_structures.nRow(336);
  
  // set up general stuff
  m_data.setCInjLo(1.9f); 
  m_data.setCInjMed(3.9f); 
  m_data.setCInjHi(5.7f);
  m_data.setVcalGradient(0,0.f);
  m_data.setVcalGradient(1,1.5f);
  m_data.setVcalGradient(2,0.f);
  m_data.setVcalGradient(3,0.f);
  m_data.setOffsetCorrection(0.f);

  m_data.setCfgEn(true);
  m_data.setScanEn(true);
  m_data.setDacsEn(true);

  // Setup FE Global Register
  if(m_structures.getGlobRegSize() == 0) {
    
    m_structures.setGlobReg("TrigCnt",         4,  0);
    m_structures.setGlobReg("Conf_AddrEnable", 1,  4);
    m_structures.setGlobReg("Reg2Spare",      11,  5);
    m_structures.setGlobReg("ErrMask0",       16, 16);
    m_structures.setGlobReg("ErrMask1",       16, 32);
    m_structures.setGlobReg("PrmpVbpRight",    8, 48);
    m_structures.setGlobReg("Vthin",           8, 56);
    m_structures.setGlobReg("DisVbn_CPPM",     8, 64);
    m_structures.setGlobReg("PrmpVbp",         8, 72);
    m_structures.setGlobReg("TdacVbp",         8, 80);
    m_structures.setGlobReg("DisVbn",          8, 88);
    m_structures.setGlobReg("Amp2Vbn",         8, 96);
    m_structures.setGlobReg("Amp2VbpFol",      8,104);
    m_structures.setGlobReg("PrmpVbpTop",      8,112);
    m_structures.setGlobReg("Amp2Vbp",         8,120);
    m_structures.setGlobReg("FdacVbn",         8,128);
    m_structures.setGlobReg("Amp2Vbpf",        8,136);
    m_structures.setGlobReg("PrmpVbnFol",      8,144);
    m_structures.setGlobReg("PrmpVbpLeft",     8,152);
    m_structures.setGlobReg("PrmpVbpf",        8,160);
    m_structures.setGlobReg("PrmpVbnLcc",      8,168);
    m_structures.setGlobReg("Reg13Spare",      1,176);
    m_structures.setGlobReg("PxStrobes",      13,177);
    m_structures.setGlobReg("S0",              1,190);
    m_structures.setGlobReg("S1",              1,191);
    m_structures.setGlobReg("LVDSDrvIref",     8,192);
    m_structures.setGlobReg("BonnDac",         8,200);
    m_structures.setGlobReg("PllIbias",        8,208);
    m_structures.setGlobReg("LVDSDrvVos",      8,216);
    m_structures.setGlobReg("TempSensBias",    8,224);
    m_structures.setGlobReg("PllIcp",          8,232);
    m_structures.setGlobReg("Reg17Spare",      8,240);
    m_structures.setGlobReg("PlsrIdacRamp",    8,248);
    m_structures.setGlobReg("Reg18Spare",      8,256);
    m_structures.setGlobReg("PlsrVgOPamp",     8,264);
    m_structures.setGlobReg("PlsrDacBias",     8,272);
    m_structures.setGlobReg("Reg19Spare",      8,280);
    m_structures.setGlobReg("Vthin_AltCoarse", 8,288);
    m_structures.setGlobReg("Vthin_AltFine",   8,296);
    m_structures.setGlobReg("PlsrDAC",        10,304);
    m_structures.setGlobReg("DIGHITIN_Sel",    1,314);
    m_structures.setGlobReg("DINJ_Override",   1,315);
    m_structures.setGlobReg("HITLD_In",        1,316);
    m_structures.setGlobReg("Reg21Spare",      3,317);
    m_structures.setGlobReg("Reg22Spare2",     2,320);
    m_structures.setGlobReg("Colpr_Addr",      6,322);
    m_structures.setGlobReg("Colpr_Mode",      2,328);
    m_structures.setGlobReg("Reg22Spare1",     6,330);
    m_structures.setGlobReg("DisableColumnCnfg0",16,336);
    m_structures.setGlobReg("DisableColumnCnfg1",16,352);
    m_structures.setGlobReg("DisableColumnCnfg2", 8,368);
    m_structures.setGlobReg("TrigLat",         8,376);
    m_structures.setGlobReg("CMDcnt",         14,384);
    m_structures.setGlobReg("StopModeCnfg",    1,398);
    m_structures.setGlobReg("HitDiscCnfg",     2,399);
    m_structures.setGlobReg("EN_PLL",          1,401);
    m_structures.setGlobReg("Efuse_sense",     1,402);
    m_structures.setGlobReg("Stop_Clk",        1,403);
    m_structures.setGlobReg("ReadErrorReq",    1,404);
    m_structures.setGlobReg("ReadSkipped",     1,405);
    m_structures.setGlobReg("Reg27Spare",      5,406);
    m_structures.setGlobReg("GateHitOr",       1,411);
    m_structures.setGlobReg("CalEn",           1,412);
    m_structures.setGlobReg("SR_clr",          1,413);
    m_structures.setGlobReg("Latch_en",        1,414);
    m_structures.setGlobReg("SR_Clock",        1,415);
    m_structures.setGlobReg("LVDSDrvSet06",    1,416);
    m_structures.setGlobReg("Reg28Spare",      5,417);
    m_structures.setGlobReg("EN40M",           1,422);
    m_structures.setGlobReg("EN80M",           1,423);
    m_structures.setGlobReg("CLK1",            3,424);
    m_structures.setGlobReg("CLK0",            3,427);
    m_structures.setGlobReg("EN160M",          1,430);
    m_structures.setGlobReg("EN320M",          1,431);
    m_structures.setGlobReg("Reg29Spare1",     2,432);
    m_structures.setGlobReg("no8b10b",         1,434);
    m_structures.setGlobReg("Clk2OutCnfg",     1,435);
    m_structures.setGlobReg("EmptyRecord",     8,436);
    m_structures.setGlobReg("Reg29Spare2",     1,444);
    m_structures.setGlobReg("LVDSDrvEn",       1,445);
    m_structures.setGlobReg("LVDSDrvSet30",    1,446);
    m_structures.setGlobReg("LVDSDrvSet12",    1,447);
    m_structures.setGlobReg("PlsrRiseUpTau",   3,448);
    m_structures.setGlobReg("PlsrPwr",         1,451);
    m_structures.setGlobReg("PlsrDelay",       6,452);
    m_structures.setGlobReg("ExtDigCalSW",     1,458);
    m_structures.setGlobReg("ExtAnaCalSW",     1,459);
    m_structures.setGlobReg("Reg31Spare",      4,460);
    m_structures.setGlobReg("SELB0",          16,464);
    m_structures.setGlobReg("SELB1",          16,480);
    m_structures.setGlobReg("SELB2",           8,496);
    m_structures.setGlobReg("EfuseCref",       4,504);
    m_structures.setGlobReg("EfuseVref",       4,508);
    m_structures.setGlobReg("Chip_SN",        16,512);

  }
  
  // Setup FE Pixel Register
  m_structures.setPixReg("ENABLE",  0);
  m_structures.setPixReg("CAP1", 1);
  m_structures.setPixReg("CAP0", 2);
  m_structures.setPixReg("ILEAK",   3);
  m_structures.setPixReg("DIGINJ",4);
  
  // Setup FE Trims
  m_structures.setTrim("TDAC", 0, 5, 0x1f);
  m_structures.setTrim("FDAC", 5, 4, 0xf);
}

    
void PixFeI4AConfig::setupData() {
  // Setup global register default values
  PixFeStructures::nameIterator name, nameEnd=m_structures.globRegEnd();
  for(name = m_structures.globRegBegin(); name != nameEnd; name++)
    writeGlobRegister(*name, 0x0);
		
  writeGlobRegister("Amp2Vbn", 79);
  writeGlobRegister("Amp2Vbp", 85);
  writeGlobRegister("Amp2Vbpf", 13);
  writeGlobRegister("Amp2VbpFol", 26);
  writeGlobRegister("BonnDac", 237);
  writeGlobRegister("CalEn", 0);
  writeGlobRegister("Chip_SN", 0);
  writeGlobRegister("CLK0", 4);
  writeGlobRegister("CLK1", 0);
  writeGlobRegister("Clk2OutCnfg", 0);
  writeGlobRegister("CMDcnt", 11);
  writeGlobRegister("Colpr_Addr", 0);
  writeGlobRegister("Colpr_Mode", 0);
  writeGlobRegister("Conf_AddrEnable", 1);
  writeGlobRegister("DIGHITIN_Sel", 0);
  writeGlobRegister("DINJ_Override", 0);
  writeGlobRegister("DisVbn", 26);
  writeGlobRegister("DisVbn_CPPM", 62);
  writeGlobRegister("Efuse_sense", 0);
  writeGlobRegister("EfuseCref", 15);
  writeGlobRegister("EfuseVref", 0);
  writeGlobRegister("EmptyRecord", 0);
  writeGlobRegister("EN_PLL", 1);
  writeGlobRegister("EN160M", 1);
  writeGlobRegister("EN320M", 0);
  writeGlobRegister("EN40M", 1);
  writeGlobRegister("EN80M", 0);
  writeGlobRegister("ErrMask0", 0xffff);
  writeGlobRegister("ErrMask1", 0xffff);
  writeGlobRegister("ExtAnaCalSW", 0);
  writeGlobRegister("ExtDigCalSW", 0);
  writeGlobRegister("FdacVbn", 50);
  writeGlobRegister("GateHitOr", 0);
  writeGlobRegister("HitDiscCnfg", 0);
  writeGlobRegister("HITLD_In", 0);
  writeGlobRegister("Latch_en", 0);
  writeGlobRegister("LVDSDrvEn", 1);
  writeGlobRegister("LVDSDrvIref", 171);
  writeGlobRegister("LVDSDrvSet06", 1);
  writeGlobRegister("LVDSDrvSet12", 1);
  writeGlobRegister("LVDSDrvSet30", 1);
  writeGlobRegister("LVDSDrvVos", 105);
  writeGlobRegister("PllIbias", 88);
  writeGlobRegister("PllIcp", 28);
  writeGlobRegister("PlsrDAC", 0);
  writeGlobRegister("PlsrDacBias", 96);
  writeGlobRegister("PlsrDelay", 2);
  writeGlobRegister("PlsrIdacRamp", 213);
  writeGlobRegister("PlsrPwr", 1);
  writeGlobRegister("PlsrRiseUpTau", 7);
  writeGlobRegister("PlsrVgOPamp", 255);
  writeGlobRegister("PrmpVbnFol", 106);
  writeGlobRegister("PrmpVbnLcc", 0);
  writeGlobRegister("PrmpVbp", 43);
  writeGlobRegister("PrmpVbpf", 20);
  writeGlobRegister("PrmpVbpLeft", 43);
  writeGlobRegister("PrmpVbpRight", 43);
  writeGlobRegister("PrmpVbpTop", 0);
  writeGlobRegister("PxStrobes", 0);
  writeGlobRegister("ReadErrorReq", 0);
  writeGlobRegister("ReadSkipped", 0);
  writeGlobRegister("Reg13Spare", 0);
  writeGlobRegister("Reg17Spare", 0);
  writeGlobRegister("Reg18Spare", 0);
  writeGlobRegister("Reg19Spare", 0);
  writeGlobRegister("Reg21Spare", 0);
  writeGlobRegister("Reg22Spare1", 0);
  writeGlobRegister("Reg22Spare2", 0);
  writeGlobRegister("Reg27Spare", 0);
  writeGlobRegister("Reg28Spare", 0);
  writeGlobRegister("Reg29Spare1", 0);
  writeGlobRegister("Reg29Spare2", 0);
  writeGlobRegister("Reg2Spare", 0);
  writeGlobRegister("Reg31Spare", 0);
  writeGlobRegister("S0", 0);
  writeGlobRegister("S1", 0);
  writeGlobRegister("SR_Clock", 0);
  writeGlobRegister("SR_clr", 0);
  writeGlobRegister("Stop_Clk", 0);
  writeGlobRegister("StopModeCnfg", 0);
  writeGlobRegister("TdacVbp", 255);
  writeGlobRegister("TempSensBias", 0);
  writeGlobRegister("TrigCnt", 15);
  writeGlobRegister("TrigLat", 210);
  writeGlobRegister("Vthin", 255);
  writeGlobRegister("Vthin_AltCoarse", 0);
  writeGlobRegister("Vthin_AltFine", 130);
  writeGlobRegister("no8b10b", 0);

  // Setup pixel registers default values
  ConfMask<bool> pixDefault(m_structures.nCol(), m_structures.nRow(), 1, 1);
  nameEnd=m_structures.pixRegEnd();
  for(name = m_structures.pixRegBegin(); name != nameEnd; name++)
    writePixRegister(*name, pixDefault);
		
  // Setup trim default values
  nameEnd=m_structures.trimEnd();
  for(name = m_structures.trimBegin(); name != nameEnd; name++) {
    int max;
    m_structures.getTrimMax(*name, max);
    ConfMask<unsigned short int> trimDefault(m_structures.nCol(), m_structures.nRow(), max, max/2);
    writeTrim(*name, trimDefault);
  }

  //setup calib default values
  for(int itot=0;itot<14;itot++){
    std::stringstream a;
    a << itot;
    m_data.setCalib("TOT"+a.str(), 1e5, 0x0);
  }

}
