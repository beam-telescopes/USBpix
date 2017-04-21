/////////////////////////////////////////////////////////////////////
// PixMccI1Config.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 16/03/04  Version 1.0 (CS)
//           Initial release
//

//! PixMccConfig implementation for MCC-I1

#include <math.h>
#include <typeinfo>
#include <sstream>

#include "Bits/Bits.h"
#include "Config/Config.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMccExc.h"
#include "PixMcc/PixMccStructures.h"
#include "PixMcc/PixMccData.h"
#include "PixMcc/PixMcc.h"
#include "PixMcc/PixMccI1Config.h"


using namespace PixLib;


//! Constructor
PixMccI1Config::PixMccI1Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name) :
  m_db(db), m_dbInquire(dbInquire), m_name(name), m_className("PixMccI1") {

  // Setup config structures
  setupStructures();

  // Setup default data
  m_data.setMccStructures(&m_structures);
  setupData();

  // Setup config object
  setupConfigObject();

  // Load initial config from database
  if(m_db!=0 && m_dbInquire!=0) loadConfig("PHYSICS");
}

//! Copy constructor
PixMccI1Config::PixMccI1Config(const PixMccConfig &c) : m_className("PixMccI1") {
  // Try dynamic casting
  try {
    const PixMccI1Config& cfg = dynamic_cast<const PixMccI1Config&>(c);
    
    // Setup config structures
    setupStructures();
    m_data.setMccStructures(&m_structures);

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
    throw PixMccExc(PixMccExc::CASTING_MISMATCH, PixMccExc::ERROR, m_name);
    return;
  }
}

//! Destructor
PixMccI1Config::~PixMccI1Config() {
}


//! Assignment operator
PixMccConfig& PixMccI1Config::operator = (const PixMccConfig& c) {

  if(this != &c) {
    // Try dynamic casting
    try {
      const PixMccI1Config& cfg = dynamic_cast<const PixMccI1Config&>(c);
 
      // Copy config data
      m_data = cfg.m_data;
    }
    catch(bad_cast) {
      throw PixMccExc(PixMccExc::CASTING_MISMATCH, PixMccExc::ERROR, m_name);
      return *this;
    }
  }

  return *this;
}


//! Write into a register mem copy
void PixMccI1Config::writeRegister(std::string regName, int value) {
  // Get register structure info
  Bits id; int wrMask, rdMask;
  if(!m_structures.getReg(regName, id, wrMask, rdMask)) {
    throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }
  
  // Write value to software register copy (single register)
  if(wrMask!=0)
    m_data.setReg(regName, value);
  // Write value to software register copy (multiple register)
  else {
    PixMccStructures::nameIterator n, nEnd=m_structures.regEnd();
    for(n = m_structures.regBegin(); n != nEnd; n++) {
      // Find registers starting with regName
      if(n->compare(0, regName.length(), regName)==0) {
	if(m_structures.getRegWMask(*n, wrMask)) {
	  // Select writable registers
	  if(wrMask!=0) {
	    // Count positions to shift
	    int shift; for(shift=0; (wrMask & 0x1)==0; shift++, wrMask=(wrMask>>1)) {}
	    // Update value
	    writeRegister(*n, (value>>shift)&wrMask);
	  }
	}
      }
    }
  }
}

//! Read a register from mem copy
int PixMccI1Config::readRegister(std::string regName) {
  // Get register structure info
  Bits id; int wrMask, rdMask;
  if(!m_structures.getReg(regName, id, wrMask, rdMask)) {
    throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return -1;
  }
  
  // Read value from software register copy (single register)
  int value=0;
  if(wrMask!=0) {
    if(!m_data.getReg(regName, value)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return -1;
    }
  }
  // Read value from software register copy (multiple register)
  else {
    PixMccStructures::nameIterator n, nEnd=m_structures.regEnd();
    for(n = m_structures.regBegin(); n != nEnd; n++) {
      // Find registers starting with regName
      if(n->compare(0, regName.length(), regName)==0) {
	if(m_structures.getReg(*n, id, wrMask, rdMask)) {
	  // Select writable registers
	  if(wrMask!=0) {
	    // Count positions to shift
	    int shift; for(shift=0; (rdMask & 0x1)==0; shift++, rdMask=(rdMask>>1)) {}
	    // Update value
	    value += (((readRegister(*n))&rdMask)<<shift);
	  }
	}
      }
    }
  }

  return value;
}


//! Read the config from DB
void PixMccI1Config::loadConfig(std::string /*configName*/) {
  m_conf->read(m_dbInquire);
}


//! Save the config to DB
void PixMccI1Config::saveConfig(std::string /*configName*/) {
  m_conf->write(m_dbInquire);
}


//! Dump MCC info to output stream
void PixMccI1Config::dump(std::ostream &out) {
  // Dump configuration header
  out << "********************************************" << std::endl 
      << "************* MCC CONFIGURATION ************" << std::endl
      << "********************************************" << std::endl;

  // Dump Register contents		
  try {
    PixMccStructures::nameIterator n, nEnd=m_structures.regEnd();
    for(n=m_structures.regBegin(); n!=nEnd; n++)
      out << *n << " REGISTER VALUE: " << std::hex << readRegister(*n) << std::endl;
  } catch(PixMccExc &MccExc) {
    MccExc.dump(out);
    throw PixMccExc(PixMccExc::INCOMPLETE_CONFIG, PixMccExc::WARNING, m_name);
  }

  // Dump strobe related parameters
  for(int i=0; i<16; i++) {
    out << "CAL strobe-delay range " << i << m_data.getDelayRange(i) << std::endl;
  }
}

 
//! Setup MCC configuration structures elements
void PixMccI1Config::setupStructures() {
  // Setup MCC commands
  if(m_structures.getCmdSize() == 0) {
    m_structures.setCmd("TRIGGER",          "1x1d",   -1,  0,  501);
    m_structures.setCmd("BCR",              "1x161",  -1,  0,    0);
    m_structures.setCmd("ECR",              "1x162",  -1,  0,    0);
    m_structures.setCmd("CAL",              "1x164",  -1,  0,    0);
    m_structures.setCmd("SYNC",             "1x168",  -1,  0,    0);
    m_structures.setCmd("WRITE_REGISTER",   "1x16b0",  1, 20,    0);
    m_structures.setCmd("READ_REGISTER",    "1x16b1",  1, 20,  516);
    m_structures.setCmd("WRITE_FIFO",       "1x16b2",  0, 25,    0);
    m_structures.setCmd("READ_FIFO",        "1x16b3",  2, 25,  521);
    m_structures.setCmd("WRITE_FRONTEND",   "1x16b4",  0, -1,    0);
    m_structures.setCmd("READ_FRONTEND",    "1x16b5",  0, -1,  501);
    m_structures.setCmd("WRITE_RECEIVER",   "1x16b6",  0, -1,    0);
    m_structures.setCmd("ENABLE_DATATAKE",  "1x16b8",  0, -1,    0);
    m_structures.setCmd("SOFT_RESET",       "1x16b9",  0, -1,    0);
    m_structures.setCmd("GLOBAL_RESET_MCC", "1x16b9",  0, -1,    0);
    m_structures.setCmd("GLOBAL_RESET_FE",  "1x16ba",  0,  8,    0);
  }

  // Setup MCC registers
  if(m_structures.getRegSize() == 0) {
    m_structures.setReg("CSR",               "0x0", 0x0000, 0x0000);
    m_structures.setReg("CSR_OutputMode",    "0x0", 0x0003, 0x0003);
    m_structures.setReg("CSR_ErrorReport",   "0x0", 0x0004, 0x0004);
    m_structures.setReg("CSR_ErrorCheck",    "0x0", 0x0008, 0x0008);
    m_structures.setReg("CSR_TOT",           "0x0", 0x0010, 0x0010);
    m_structures.setReg("CSR_OutputPattern", "0x0", 0x0020, 0x0020);
    m_structures.setReg("CSR_Playback",      "0x0", 0x0040, 0x0040);
    m_structures.setReg("CSR_WarningLVL1",   "0x0", 0x0000, 0x0100);
    m_structures.setReg("CSR_WarningFast",   "0x0", 0x0000, 0x0200);
    m_structures.setReg("CSR_WarningSlow",   "0x0", 0x0000, 0x0400);
    m_structures.setReg("CSR_LVL1CheckFail", "0x0", 0x0000, 0x0800);
    m_structures.setReg("CSR_EoECheckFail",  "0x0", 0x0000, 0x1000);

    m_structures.setReg("LV1",               "0x1", 0x0000, 0x0000);
    m_structures.setReg("LV1_Counter",       "0x1", 0x0000, 0x00ff);
    m_structures.setReg("LV1_Contiguous",    "0x1", 0x0f00, 0x0f00);

    m_structures.setReg("FEEN",              "0x2", 0xffff, 0xffff);

    m_structures.setReg("WFE",               "0x3", 0xffff, 0xffff);
    
    m_structures.setReg("WMCC",              "0x4", 0xffff, 0xffff);

    m_structures.setReg("CNT",               "0x5", 0x0000, 0x0000);
    m_structures.setReg("CNT_FEControlBits", "0x5", 0x0007, 0x0007);
    m_structures.setReg("CNT_FEDataBits",    "0x5", 0xfff8, 0xfff8);

    m_structures.setReg("CAL",               "0x6", 0x0000, 0x0000);
    m_structures.setReg("CAL_Delay",         "0x6", 0x003f, 0x003f);
    m_structures.setReg("CAL_Range",         "0x6", 0x03c0, 0x03c0);
    m_structures.setReg("CAL_Enable",        "0x6", 0x0400, 0x0400);

    m_structures.setReg("PEF",               "0x7", 0x0000, 0xffff);
  }
  m_data.resetReg();

  // Setup MCC fifos
  if(m_structures.getFifoSize() == 0) {
    m_structures.setFifo("FIFO00", "0x0");
    m_structures.setFifo("FIFO01", "0x1");
    m_structures.setFifo("FIFO02", "0x2");
    m_structures.setFifo("FIFO03", "0x3");
    m_structures.setFifo("FIFO04", "0x4");
    m_structures.setFifo("FIFO05", "0x5");
    m_structures.setFifo("FIFO06", "0x6");
    m_structures.setFifo("FIFO07", "0x7");
    m_structures.setFifo("FIFO08", "0x8");
    m_structures.setFifo("FIFO09", "0x9");
    m_structures.setFifo("FIFO10", "0xa");
    m_structures.setFifo("FIFO11", "0xb");
    m_structures.setFifo("FIFO12", "0xc");
    m_structures.setFifo("FIFO13", "0xd");
    m_structures.setFifo("FIFO14", "0xe");
    m_structures.setFifo("FIFO15", "0xf");
    m_structures.setFifoMask(0xffffff);
  }

  // Setup data header
  m_structures.setHeader("1x1d");
} 


void PixMccI1Config::setupData() {
  // Setup register default values
  writeRegister("CSR_OutputMode",    0); // 40 Mbit/s
  writeRegister("CSR_ErrorReport",   1); // Report errors
  writeRegister("CSR_ErrorCheck",    1); // Check for errors
  writeRegister("CSR_TOT",           1); // TOT enabled
  writeRegister("CSR_OutputPattern", 0); // Output pattern disabled
  writeRegister("CSR_Playback",      0); // Playback disabled
  
  writeRegister("LV1_Contiguous",    0); // Single LVL1 mode

  writeRegister("FEEN",              0xffff); // All FE enabled

  writeRegister("WFE",               0); // All FE warnings cleared
    
  writeRegister("WMCC",              0); // All MCC warnings cleared
  
  writeRegister("CNT_FEControlBits", 0);
  writeRegister("CNT_FEDataBits",    0);
  
  writeRegister("CAL_Delay",         0);
  writeRegister("CAL_Range",         0);
  writeRegister("CAL_Enable",        0);

  // Setup strobe default values
  for(int i=0; i<16; i++)
    m_data.getDelayRange(i) = 0.0;
}


void PixMccI1Config::setupConfigObject() {
  // Create the Config object
  m_conf = new Config("PixMcc_0/PixMcc");
  Config &conf = *m_conf;

  // Group class info
  conf.addGroup("ClassInfo");
  conf["ClassInfo"].addString("ClassName", m_className, m_className, "Derived class name", true);

  // Group registers
  conf.addGroup("Registers");
  PixMccStructures::nameIterator name, nameEnd=m_structures.regEnd();
  for(name = m_structures.regBegin(); name != nameEnd; name++) {
    int mask, def;
    if(m_structures.getRegWMask(*name, mask))
      if(mask != 0)
	if(m_data.getReg(*name, def))
	  conf["Registers"].addInt(*name, m_data.getReg(*name), def, *name+" register", true);
  }

  // Group strobe
  conf.addGroup("Strobe");
  for(int i=0; i<16; i++) {
    std::stringstream fieldNum;
    std::string fieldName = "DELAY_";
    fieldNum << i;
    fieldName += fieldNum.str(); 
    conf["Strobe"].addFloat(fieldName, m_data.getDelayRange(i), m_data.getDelayRange(i), "Strobe delay range "+fieldNum.str(), true);
  }

  // Select default values
  conf.reset();
}
