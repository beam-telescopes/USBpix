/////////////////////////////////////////////////////////////////////
// PixFeI1Config.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 15/03/04  Version 1.0 (CS)
//           Initial release
//

//! PixFeConfig implementation for FE-I1

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
#include "PixFe/PixFeConfig.h"
#include "PixFe/PixFeI1Config.h"

using namespace PixLib;


//! Constructor
PixFeI1Config::PixFeI1Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number) :
m_db(db), m_dbInquire(dbInquire), m_name(name), m_className("PixFeI1"), m_conf(0){
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
PixFeI1Config::PixFeI1Config(const PixFeConfig &c) : m_className("PixFeI1"), m_conf(0) {
  // Try dynamic casting
  try {
    const PixFeI1Config& cfg = dynamic_cast<const PixFeI1Config&>(c);
    
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
PixFeI1Config::~PixFeI1Config() {
  delete m_conf;
}


//! Assignment operator
PixFeConfig& PixFeI1Config::operator = (const PixFeConfig& c) {

  if(this != &c) {
    // Try dynamic casting
    try {
      const PixFeI1Config& cfg = dynamic_cast<const PixFeI1Config&>(c);
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
void PixFeI1Config::copyGlobReg(const PixFeConfig& source) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI1Config& cfg = dynamic_cast<const PixFeI1Config&>(source);
      m_data.copyGlobReg(cfg.m_data);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}

//! Pixel register copy method
void PixFeI1Config::copyPixReg(const PixFeConfig& source, std::string name) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI1Config& cfg = dynamic_cast<const PixFeI1Config&>(source);
      m_data.copyPixReg(cfg.m_data, name);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}

//! Trim copy method
void PixFeI1Config::copyTrim(const PixFeConfig& source, std::string name) {
  if(this != &source) {
    // Try dynamic casting
    try {
      const PixFeI1Config& cfg = dynamic_cast<const PixFeI1Config&>(source);
      m_data.copyTrim(cfg.m_data, name);
    }
    catch(bad_cast) {
      throw PixFeExc(PixFeExc::CASTING_MISMATCH, PixFeExc::ERROR, m_name);
      return;
    }
  }
}


//! Write a value in the Global Register mem copy
void PixFeI1Config::writeGlobRegister(std::string regName, int value) {
  // Check if register exists
  int length, offset;
  if(!m_structures.getGlobReg(regName, length, offset)) {
    throw PixFeExc(PixFeExc::GLOB_REG_NOT_FOUND, PixFeExc::ERROR, m_name);
    return;
  }
  
  // Write value to software register copy
  m_data.setGlobReg(regName, value);
}

//! Read a value from the Global Register mem copy 
int& PixFeI1Config::readGlobRegister(std::string regName) {
  // Read value from software register copy
  bool status;
  int& value = m_data.getGlobReg(regName, status);
  if(!status) {
    throw PixFeExc(PixFeExc::GLOB_REG_NOT_FOUND, PixFeExc::ERROR, m_name);
    int *temp;
    return *temp;
  }
  
  return value;
}


//! Write a value in the Pixel Register mem copy
void PixFeI1Config::writePixRegister(std::string regName, ConfMask<bool> &value) {
  // Check if register exists
  int address;
  if(!m_structures.getPixReg(regName, address)) {
    throw PixFeExc(PixFeExc::PIX_REG_NOT_FOUND, PixFeExc::ERROR, m_name);
    return;
  }
  
  // Write value to software register copy
  m_data.setPixReg(regName, value);
}

//! Read a value from the Pixel Register mem copy
ConfMask<bool>& PixFeI1Config::readPixRegister(std::string regName) {
  // Read value from software register copy
  bool ok;
  ConfMask<bool> &value = m_data.getPixReg(regName, ok);
  if(!ok) throw PixFeExc(PixFeExc::PIX_REG_NOT_FOUND, PixFeExc::ERROR, m_name);
  
  return value;
}


//! Write a value in the Trim mem copy
void PixFeI1Config::writeTrim(std::string trimName, ConfMask<unsigned short int> &value) {
  // Check if trim exists
  int address, number;
  if(!m_structures.getTrim(trimName, address, number)) {
    throw PixFeExc(PixFeExc::TRIM_NOT_FOUND, PixFeExc::ERROR, m_name);
    return;
  }
  
  // Write value to software register copy
  m_data.setTrim(trimName, value);
}

//! Read a value from the Trim mem copy
ConfMask<unsigned short int>& PixFeI1Config::readTrim(std::string trimName) {
  bool ok;
  ConfMask<unsigned short int> &value = m_data.getTrim(trimName, ok);
  if(!ok) throw PixFeExc(PixFeExc::TRIM_NOT_FOUND, PixFeExc::ERROR, m_name);
  
  return value;
}


//! Read the config from DB
void PixFeI1Config::loadConfig(std::string ){//configName) {
  m_conf->read(m_dbInquire);
}


//! Save the config to DB
void PixFeI1Config::saveConfig(std::string ){//configName) {
  m_conf->write(m_dbInquire);
}


void PixFeI1Config::dump(std::ostream &out) {
  // Dump configuration header
  out << "********************************************" << std::endl 
      << "*********** FE #" << std::dec << m_data.getIndex() << " CONFIGURATION ***********" << std::endl
      << "********************************************" << std::endl;

  // Dump Global Register contents
  out << "*********** GLOBAL REGISTER ***********" << std::endl;
  Bits glob;
  if(!m_data.getGlobReg(glob)) {
    PixFeExc exc(PixFeExc::GLOB_REG_NOT_FOUND, PixFeExc::ERROR, m_name);
    exc.dump(out);
    throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
  }
  out << glob << std::endl;
  out << std::endl;
  
  // Dump Pixel Registers contents
  ConfMask<bool> pixReg(m_structures.nCol(), m_structures.nRow(), 1);
  int i, row, col;
#ifdef WIN32
  std::vector<int> val[5];
  for(int iv=0;iv<5;iv++)
    val[iv].resize(m_structures.nCol());
#else
  int val[5][m_structures.nCol()];
#endif

  PixFeStructures::nameIterator n, nEnd=m_structures.pixRegEnd();
  for(n=m_structures.pixRegBegin(); n!=nEnd; n++) {
    out << "*********** " << *n << " REGISTER ***********" << std::endl;
    try {
      pixReg = readPixRegister(*n);
    } catch(PixFeExc &FeExc) {
      FeExc.dump(out);
      throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
    }
    
    for(col=0; col<m_structures.nCol(); col++)
      for(i=0; i<5; i++)
	val[i][col] = 0;
    
    for(row=0; row<m_structures.nRow(); row++)
      for(col=0; col<m_structures.nCol(); col++)
	val[row/32][col] |= pixReg[col][row]<<(row%32);
    
    for(col=0; col<m_structures.nCol(); col++) {
      for(row=0; row<5; row++)
	out << std::hex << std::setw(8) << std::setfill('0') << val[row][col] << " ";
      out << std::dec << std::endl;
    }
    out << std::endl;
  }

  // Dump Trim contents
  ConfMask<unsigned short int> *trim;

  nEnd=m_structures.trimEnd();
  for(n=m_structures.trimBegin(); n!=nEnd; n++) {
    out << "*********** " << *n << " TRIM ***********" << std::endl;
    try {
      int max;
      m_structures.getTrimMax(*n, max);
      trim = new ConfMask<unsigned short int>(m_structures.nCol(), m_structures.nRow(), max);
      (*trim) = readTrim(*n);
    } catch(PixFeExc &FeExc) {
      FeExc.dump(out);
      throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
    }
    
    for(row=0; row<m_structures.nRow(); row++) {
      for(col=0; col<m_structures.nCol(); col++) 
	out << (*trim)[col][row] << " ";
      out << std::endl;
    }
    out << std::endl;
    delete trim;
  }
}


//! Setup FE configuration structures elements
void PixFeI1Config::setupStructures() {
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
    m_structures.setGlobReg("MUX_TEST_PIXEL",         2, 126);
    m_structures.setGlobReg("DAC_MON_LEAK_ADC",       9, 128);
    m_structures.setGlobReg("DAC_VCAL",               9, 137);
    m_structures.setGlobReg("WIDTH_SELF_TRIGGER",     4, 146);
    m_structures.setGlobReg("MUX_DO",                 4, 150);
    m_structures.setGlobReg("MUX_MON_HIT",            4, 154);
    m_structures.setGlobReg("MUX_EOC",                2, 158);
    m_structures.setGlobReg("FREQUENCY_CEU",          2, 160); 
    m_structures.setGlobReg("MODE_TOT_THRESH",        2, 162);
    m_structures.setGlobReg("ENABLE_TIMESTAMP",       1, 164);
    m_structures.setGlobReg("ENABLE_SELF_TRIGGER",    1, 165);
    m_structures.setGlobReg("SPARE",                  1, 166);
    m_structures.setGlobReg("MON_MON_LEAK_ADC",       1, 167);
    m_structures.setGlobReg("MON_ADC_REF",            1, 168);
    m_structures.setGlobReg("ENABLE_MON_LEAK",        1, 169);
    m_structures.setGlobReg("STATUS_MON_LEAK",        1, 170);
    m_structures.setGlobReg("ENABLE_CAP_TEST",        1, 171);
    m_structures.setGlobReg("ENABLE_BUFFER",          1, 172);
    m_structures.setGlobReg("ENABLE_VCAL_MEASURE",    1, 173);
    m_structures.setGlobReg("ENABLE_LEAK_MEASURE",    1, 174);
    m_structures.setGlobReg("ENABLE_BUFFER_BOOST",    1, 175);
    m_structures.setGlobReg("ENABLE_CP8",             1, 176);
    m_structures.setGlobReg("MON_IVDD2",              1, 177);
    m_structures.setGlobReg("MON_ID",                 1, 178);
    m_structures.setGlobReg("ENABLE_CP7",             1, 179);
    m_structures.setGlobReg("MON_IP2",                1, 180);
    m_structures.setGlobReg("MON_IP",                 1, 181);
    m_structures.setGlobReg("ENABLE_CP6",             1, 182);
    m_structures.setGlobReg("MON_ITRIMTH",            1, 183);
    m_structures.setGlobReg("MON_IF",                 1, 184);
    m_structures.setGlobReg("ENABLE_CP5",             1, 185);
    m_structures.setGlobReg("MON_ITRIMIF",            1, 186);
    m_structures.setGlobReg("MON_VCAL",               1, 187);
    m_structures.setGlobReg("ENABLE_CP4",             1, 188);
    m_structures.setGlobReg("ENABLE_CINJ_HIGH",       1, 189);
    m_structures.setGlobReg("ENABLE_EXTERNAL",        1, 190);
    m_structures.setGlobReg("ENABLE_TEST_ANALOG_REF", 1, 191);
    m_structures.setGlobReg("ENABLE_DIGITAL",         1, 192);
    m_structures.setGlobReg("ENABLE_CP3",             1, 193);
    m_structures.setGlobReg("MON_ITH1",               1, 194);
    m_structures.setGlobReg("MON_ITH2",               1, 195);
    m_structures.setGlobReg("ENABLE_CP2",             1, 196);
    m_structures.setGlobReg("MON_IL",                 1, 197);
    m_structures.setGlobReg("MON_IL2",                1, 198);
    m_structures.setGlobReg("ENABLE_CP1",             1, 199);
    m_structures.setGlobReg("ENABLE_CP0",             1, 200);
    m_structures.setGlobReg("MON_SPARE",              1, 201);
  }
  
  // Setup FE Pixel Register
  m_structures.setPixReg("HITBUS", 0);
  m_structures.setPixReg("SELECT", 1);
  m_structures.setPixReg("PREAMP", 2);
  m_structures.setPixReg("ENABLE", 3);
  
  // Setup FE Trims
  m_structures.setTrim("TDAC", 0, 5, 0x1f);
  m_structures.setTrim("FDAC", 5, 5, 0x1f);
}

    
void PixFeI1Config::setupData() {
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


void PixFeI1Config::setupConfigObject() {
  // Create the Config object
  std::stringstream configName;
  configName << "PixFe_"<<m_data.getIndex()<<"/PixFe";
  m_conf = new Config(configName.str());
  Config &conf = *m_conf;

  // Group class info
  conf.addGroup("ClassInfo");
  conf["ClassInfo"].addString("ClassName", m_className, m_className, "Derived class name", false);

  // Global register subconfig
  conf.addConfig("GlobalRegister_0/GlobalRegister");
  conf.subConfig("GlobalRegister_0/GlobalRegister").addGroup("GlobalRegister");
  PixFeStructures::nameIterator name, nameEnd=m_structures.globRegEnd();
  for(name = m_structures.globRegBegin(); name != nameEnd; name++) {
    int def;
    try {
      bool status;
      def = readGlobRegister(*name);
      conf.subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"].addInt(*name, m_data.getGlobReg(*name, status), def, *name+" register", true);
    } catch(PixFeExc &FeExc) {
      FeExc.dump(std::cout);
      throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
    }
  }

  // Pixel register group
  conf.addConfig("PixelRegister_0/PixelRegister");
  conf.subConfig("PixelRegister_0/PixelRegister").addGroup("PixelRegister");
  nameEnd=m_structures.pixRegEnd();
  for(name = m_structures.pixRegBegin(); name != nameEnd; name++) {
    ConfMask<bool> def(m_structures.nCol(), m_structures.nRow(), 1, 1);
    try {
      bool status;
      def = readPixRegister(*name);
      conf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"].addMatrix(*name, m_data.getPixReg(*name, status), def, *name+" register", true);
    } catch(PixFeExc &FeExc) {
      FeExc.dump(std::cout);
      throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
    }
  }

  // Trim group
  conf.addConfig("Trim_0/Trim");
  conf.subConfig("Trim_0/Trim").addGroup("Trim");
  nameEnd=m_structures.trimEnd();
  for(name = m_structures.trimBegin(); name != nameEnd; name++) {
    ConfMask<unsigned short int> def(m_structures.nCol(), m_structures.nRow(), 32, 32);
    try {
      bool status;
      def = readTrim(*name);
      conf.subConfig("Trim_0/Trim")["Trim"].addMatrix(*name, m_data.getTrim(*name, status), def, *name+" trim", true);
    } catch(PixFeExc &FeExc) {
      FeExc.dump(std::cout);
      throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::WARNING, m_name);
    }
  }

  // Miscellanea group
  conf.addGroup("Misc");
  conf["Misc"].addInt("Index", m_data.getIndex(), m_data.getIndex(), "FE index", false);
  conf["Misc"].addInt("Address", m_data.getCmdAddr(), m_data.getCmdAddr(), "FE geographical address", true);
  conf["Misc"].addFloat("CInjLo", m_data.getCInjLo(), m_data.getCInjLo(), "Charge injection low", true);
  conf["Misc"].addFloat("CInjHi", m_data.getCInjHi(), m_data.getCInjHi(), "Charge injection high", true);
  conf["Misc"].addFloat("VcalGradient0", m_data.getVcalGradient(0), m_data.getVcalGradient(0), "VCAL cal. Par0", true);
  conf["Misc"].addFloat("VcalGradient1", m_data.getVcalGradient(1), m_data.getVcalGradient(1), "VCAL cal. Par1", true);
  conf["Misc"].addFloat("VcalGradient2", m_data.getVcalGradient(2), m_data.getVcalGradient(2), "VCAL cal. Par2", true);
  conf["Misc"].addFloat("VcalGradient3", m_data.getVcalGradient(3), m_data.getVcalGradient(3), "VCAL cal. Par3", true);
  conf["Misc"].addFloat("OffsetCorrection", m_data.getOffsetCorrection(), m_data.getOffsetCorrection(), "Internal injection offset correction", true);
  conf["Misc"].addBool("ConfigEnable", m_data.getCfgEn(), m_data.getCfgEn(), "FE configuration enable", true);
  conf["Misc"].addBool("ScanEnable", m_data.getScanEn(), m_data.getScanEn(), "FE scan/readout enable", true);
  conf["Misc"].addBool("DacsEnable", m_data.getDacsEn(), m_data.getDacsEn(), "FE DACs enable", true);

  // Select default values
  conf.reset();
}
