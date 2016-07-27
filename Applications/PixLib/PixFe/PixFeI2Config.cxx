/////////////////////////////////////////////////////////////////////
// PixFeI2Config.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//           Initial release
//

//! PixFeConfig implementation for FE-I2

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
#include "PixFe/PixFeI2Config.h"

using namespace PixLib;


//! Constructor
PixFeI2Config::PixFeI2Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number) {
  // Members initialization
  m_db = db;
  m_dbInquire = dbInquire;
  m_name = name;
  m_className = "PixFeI2";

  // Setup config structures
  setupStructures();
    
  // Setup default data
  m_data.setFeStructures(&m_structures);
  m_data.setIndex(number);
  m_data.setCmdAddr(number);
  setupData();

  // Setup config object
  setupConfigObject();

  // Load initial config from database
  if(m_db!=0 && m_dbInquire!=0) loadConfig("PHYSICS");
}

//! Copy constructor
PixFeI2Config::PixFeI2Config(const PixFeConfig &c) {
  // Members initialization
  m_className = "PixFeI2";

  // Try dynamic casting
  try {
    const PixFeI2Config& cfg = dynamic_cast<const PixFeI2Config&>(c);
    
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
  }
  catch(bad_cast) {
    throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
    return;
  }
}

//! Destructor
PixFeI2Config::~PixFeI2Config() {
}


//! Assignment operator
PixFeConfig& PixFeI2Config::operator = (const PixFeConfig& c) {

  if(this != &c) {
    // Try dynamic casting
    try {
      const PixFeI2Config& cfg = dynamic_cast<const PixFeI2Config&>(c);
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
void PixFeI2Config::copyGlobReg(const PixFeConfig& source) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI2Config& cfg = dynamic_cast<const PixFeI2Config&>(source);
      m_data.copyGlobReg(cfg.m_data);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}

//! Pixel register copy method
void PixFeI2Config::copyPixReg(const PixFeConfig& source, std::string name) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI2Config& cfg = dynamic_cast<const PixFeI2Config&>(source);
      m_data.copyPixReg(cfg.m_data, name);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}

//! Trim copy method
void PixFeI2Config::copyTrim(const PixFeConfig& source, std::string name) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI2Config& cfg = dynamic_cast<const PixFeI2Config&>(source);
      m_data.copyTrim(cfg.m_data, name);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}


//! Setup FE configuration structures elements
void PixFeI2Config::setupStructures() {
  // Setup FE general parameters
  m_structures.nCol(18);
  m_structures.nRow(160);
  
  // set up general stuff
  m_data.setCInjLo(7.6f); 
  m_data.setCInjHi(44.0f);
  m_data.setVcalGradient(0,0.f);
  m_data.setVcalGradient(1,0.9f);
  m_data.setVcalGradient(2,0.f);
  m_data.setVcalGradient(3,0.f);
  m_data.setOffsetCorrection(0.f);

  m_data.setCfgEn(true);
  m_data.setScanEn(true);
  m_data.setDacsEn(true);

  // Setup FE Global Register
  if(m_structures.getGlobRegSize() == 0) {
    m_structures.setGlobReg("LATENCY",                8, 0);
    m_structures.setGlobReg("DAC_IVDD2",              8, 8);
    m_structures.setGlobReg("DAC_IP2",                8, 16);
    m_structures.setGlobReg("DAC_ID",                 8, 24);
    m_structures.setGlobReg("DAC_IP",                 8, 32);
    m_structures.setGlobReg("DAC_ITRIMTH",            8, 40);
    m_structures.setGlobReg("DAC_IF",                 8, 48);
    m_structures.setGlobReg("DAC_ITH1",               8, 56);
    m_structures.setGlobReg("DAC_ITH2",               8, 64);
    m_structures.setGlobReg("DAC_IL",                 8, 72);    
    m_structures.setGlobReg("DAC_IL2",                8, 80);    
    m_structures.setGlobReg("DAC_ITRIMIF",            8, 88);
    m_structures.setGlobReg("DAC_SPARE",              8, 96);
    m_structures.setGlobReg("THRESH_TOT_MINIMUM",     8, 104);
    m_structures.setGlobReg("THRESH_TOT_DOUBLE",      8, 112);
    m_structures.setGlobReg("CAP_MEASURE",            6, 120);
    m_structures.setGlobReg("GLOBAL_DAC",             5, 120); // Every address has to be rearranged!!! Things have changed a lot!!!
    m_structures.setGlobReg("MUX_TEST_PIXEL",         2, 126);
    m_structures.setGlobReg("DAC_MON_LEAK_ADC",       9, 128);
    m_structures.setGlobReg("DAC_VCAL",              10, 137);
    m_structures.setGlobReg("WIDTH_SELF_TRIGGER",     4, 147);
    m_structures.setGlobReg("MUX_DO",                 4, 151);
    m_structures.setGlobReg("MUX_MON_HIT",            4, 155);
    m_structures.setGlobReg("MUX_EOC",                2, 159);
    m_structures.setGlobReg("FREQUENCY_CEU",          2, 161); 
    m_structures.setGlobReg("MODE_TOT_THRESH",        2, 163);
    m_structures.setGlobReg("ENABLE_TIMESTAMP",       1, 165);
    m_structures.setGlobReg("ENABLE_SELF_TRIGGER",    1, 166);
    m_structures.setGlobReg("SPARE",                  1, 167);
    m_structures.setGlobReg("MON_MON_LEAK_ADC",       1, 168);
    m_structures.setGlobReg("MON_ADC_REF",            1, 169);
    m_structures.setGlobReg("ENABLE_MON_LEAK",        1, 170);
    m_structures.setGlobReg("STATUS_MON_LEAK",        1, 171);
    m_structures.setGlobReg("ENABLE_CAP_TEST",        1, 172);
    m_structures.setGlobReg("ENABLE_BUFFER",          1, 173);
    m_structures.setGlobReg("ENABLE_VCAL_MEASURE",    1, 174);
    m_structures.setGlobReg("ENABLE_LEAK_MEASURE",    1, 175);
    m_structures.setGlobReg("ENABLE_BUFFER_BOOST",    1, 176);
    m_structures.setGlobReg("ENABLE_CP8",             1, 177);
    m_structures.setGlobReg("MON_IVDD2",              1, 178);
    m_structures.setGlobReg("MON_ID",                 1, 179);
    m_structures.setGlobReg("ENABLE_CP7",             1, 180);
    m_structures.setGlobReg("MON_IP2",                1, 181);
    m_structures.setGlobReg("MON_IP",                 1, 182);
    m_structures.setGlobReg("ENABLE_CP6",             1, 183);
    m_structures.setGlobReg("MON_ITRIMTH",            1, 184);
    m_structures.setGlobReg("MON_IF",                 1, 185);
    m_structures.setGlobReg("ENABLE_CP5",             1, 186);
    m_structures.setGlobReg("MON_ITRIMIF",            1, 187);
    m_structures.setGlobReg("MON_VCAL",               1, 188);
    m_structures.setGlobReg("ENABLE_CP4",             1, 189);
    m_structures.setGlobReg("ENABLE_CINJ_HIGH",       1, 190);
    m_structures.setGlobReg("ENABLE_EXTERNAL",        1, 191);
    m_structures.setGlobReg("ENABLE_TEST_ANALOG_REF", 1, 192);
    m_structures.setGlobReg("ENABLE_DIGITAL",         1, 193);
    m_structures.setGlobReg("ENABLE_CP3",             1, 194);
    m_structures.setGlobReg("MON_ITH1",               1, 195);
    m_structures.setGlobReg("MON_ITH2",               1, 196);
    m_structures.setGlobReg("ENABLE_CP2",             1, 197);
    m_structures.setGlobReg("MON_IL",                 1, 198);
    m_structures.setGlobReg("MON_IL2",                1, 199);
    m_structures.setGlobReg("ENABLE_CP1",             1, 200);
    m_structures.setGlobReg("ENABLE_CP0",             1, 201);
    m_structures.setGlobReg("MON_SPARE",              1, 202);
    m_structures.setGlobReg("ENABLE_BIASCOMP",        1, 203);
    m_structures.setGlobReg("ENABLE_AUTOTUNE",        1, 204);
    m_structures.setGlobReg("ENABLE_HITBUS",          1, 205);
    m_structures.setGlobReg("SELF_WIDTH",             4, 206);
    m_structures.setGlobReg("SELF_LATENCY",           4, 210);
  }
  
  // Setup FE Pixel Register
  m_structures.setPixReg("HITBUS", 0);
  m_structures.setPixReg("SELECT", 1);
  m_structures.setPixReg("PREAMP", 2);
  m_structures.setPixReg("ENABLE", 3);
  
  // Setup FE Trims
  m_structures.setTrim("TDAC", 0, 5, 0x7f);
  m_structures.setTrim("FDAC", 5, 5, 0x7);
}

    
void PixFeI2Config::setupData() {
  // Setup global register default values
  PixFeStructures::nameIterator name, nameEnd=m_structures.globRegEnd();
  for(name = m_structures.globRegBegin(); name != nameEnd; name++)
    writeGlobRegister(*name, 0x0);
		
  writeGlobRegister("LATENCY",               210);
  writeGlobRegister("DAC_IVDD2",              64);
  writeGlobRegister("DAC_IP2",                64);
  writeGlobRegister("DAC_ID",                 64);
  writeGlobRegister("DAC_IP",                 64);
  writeGlobRegister("DAC_ITRIMTH",            64);
  writeGlobRegister("DAC_IF",                 32);
  writeGlobRegister("DAC_IL",                 64);
  writeGlobRegister("DAC_IL2",                64);
  writeGlobRegister("DAC_ITRIMIF",            16);
  writeGlobRegister("GLOBAL_DAC",              8);
  writeGlobRegister("FREQUENCY_CEU",           3);
  writeGlobRegister("ENABLE_TIMESTAMP",        1);
  writeGlobRegister("ENABLE_CP8",              1);
  writeGlobRegister("ENABLE_CP7",              1);
  writeGlobRegister("ENABLE_CP6",              1);
  writeGlobRegister("ENABLE_CP5",              1);
  writeGlobRegister("ENABLE_CP4",              1);
  writeGlobRegister("ENABLE_CP3",              1);
  writeGlobRegister("ENABLE_CP2",              1);
  writeGlobRegister("ENABLE_CP1",              1);
  writeGlobRegister("ENABLE_CP0",              1);

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
}
