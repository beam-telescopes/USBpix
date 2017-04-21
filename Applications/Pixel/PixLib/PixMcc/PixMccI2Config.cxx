/////////////////////////////////////////////////////////////////////
// PixMccI2Config.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 16/03/04  Version 1.0 (CS)
//           Initial release
//

//! PixMccConfig implementation for MCC-I2

#include <math.h>
#include <typeinfo>

#include "Bits/Bits.h"
#include "Config/Config.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMccExc.h"
#include "PixMcc/PixMccStructures.h"
#include "PixMcc/PixMccData.h"
#include "PixMcc/PixMcc.h"
#include "PixMcc/PixMccConfig.h"
#include "PixMcc/PixMccI1Config.h"
#include "PixMcc/PixMccI2Config.h"


using namespace PixLib;


//! Constructor
PixMccI2Config::PixMccI2Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name) {
  // Members initializiation
  m_db = db;
  m_dbInquire = dbInquire;
  m_name = name;
  m_className = "PixMccI2";

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
PixMccI2Config::PixMccI2Config(const PixMccConfig &c) {
  // Members initializiation
  m_className = "PixMccI2";

  // Try dynamic casting
  try {
    const PixMccI2Config& cfg = dynamic_cast<const PixMccI2Config&>(c);
    
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
PixMccI2Config::~PixMccI2Config() {
}


//! Assignment operator
PixMccConfig& PixMccI2Config::operator = (const PixMccConfig& c) {

  if(this != &c) {
    // Try dynamic casting
    try {
      const PixMccI2Config& cfg = dynamic_cast<const PixMccI2Config&>(c);
 
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


//! Setup MCC configuration structures elements
void PixMccI2Config::setupStructures() {
  // Setup MCC commands
  if(m_structures.getCmdSize() == 0) {
    m_structures.setCmd("TRIGGER",          "1x1d",   -1,  0,  501);
    m_structures.setCmd("BCR",              "1x161",  -1,  0,    0);
    m_structures.setCmd("ECR",              "1x162",  -1,  0,    0);
    m_structures.setCmd("CAL",              "1x164",  -1,  0,    0);
    m_structures.setCmd("SYNC",             "1x168",  -1,  0,    0);
    m_structures.setCmd("WRITE_REGISTER",   "1x16b0",  1, 20,    0);
    m_structures.setCmd("READ_REGISTER",    "1x16b1",  1, 20,  516);
    m_structures.setCmd("WRITE_FIFO",       "1x16b2",  0, 31,    0);
    m_structures.setCmd("READ_FIFO",        "1x16b3",  2, 31,  527);
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
    m_structures.setReg("CSR",                     "0x0", 0x0000, 0x0000);
    m_structures.setReg("CSR_OutputMode",          "0x0", 0x0003, 0x0003);
    m_structures.setReg("CSR_ReceiverDisable",     "0x0", 0x0004, 0x0004);
    m_structures.setReg("CSR_OutputPattern",       "0x0", 0x0020, 0x0020);
    m_structures.setReg("CSR_Playback",            "0x0", 0x0040, 0x0040);
    m_structures.setReg("CSR_EventBuilderDisable", "0x0", 0x0080, 0x0080);
    m_structures.setReg("CSR_WarningLVL1",         "0x0", 0x0000, 0x0100);
    m_structures.setReg("CSR_WarningFast",         "0x0", 0x0000, 0x0200);
    m_structures.setReg("CSR_WarningSlow",         "0x0", 0x0000, 0x0400);
    m_structures.setReg("CSR_BCIdCheckFail",       "0x0", 0x0000, 0x0800);
    m_structures.setReg("CSR_LVL1CheckFail",       "0x0", 0x0000, 0x1000);
    m_structures.setReg("CSR_RegisterSEU",         "0x0", 0x0000, 0x2000);
    m_structures.setReg("CSR_CommandSEU",          "0x0", 0x0000, 0x4000);
    m_structures.setReg("CSR_EventBuilderSEU",     "0x0", 0x0000, 0x8000);

    m_structures.setReg("LV1",                     "0x1", 0x0000, 0x0000);
    m_structures.setReg("LV1_Counter",             "0x1", 0x0000, 0x00ff);
    m_structures.setReg("LV1_Contiguous",          "0x1", 0x0f00, 0x0f00);

    m_structures.setReg("FEEN",                    "0x2", 0xffff, 0xffff);

    m_structures.setReg("WFE",                     "0x3", 0xffff, 0xffff);
    
    m_structures.setReg("WMCC",                    "0x4", 0xffff, 0xffff);

    m_structures.setReg("CNT",                     "0x5", 0x0000, 0x0000);
    m_structures.setReg("CNT_FEControlBits",       "0x5", 0x0007, 0x0007);
    m_structures.setReg("CNT_FEDataBits",          "0x5", 0xfff8, 0xfff8);

    m_structures.setReg("CAL",                     "0x6", 0x0000, 0x0000);
    m_structures.setReg("CAL_Delay",               "0x6", 0x003f, 0x003f);
    m_structures.setReg("CAL_Range",               "0x6", 0x03c0, 0x03c0);
    m_structures.setReg("CAL_Enable",              "0x6", 0x0400, 0x0400);

    m_structures.setReg("PEF",                     "0x7", 0x0000, 0xffff);

    m_structures.setReg("SBSR",                    "0x8", 0x0000, 0xffff);

    m_structures.setReg("WBITD",                   "0x9", 0x0000, 0x0000);
    m_structures.setReg("WBITD_FEFlags",           "0x9", 0x00ff, 0x00ff);
    m_structures.setReg("WBITD_HitOverflow",       "0x9", 0x0100, 0x0100);
    m_structures.setReg("WBITD_EoEOverflow",       "0x9", 0x0200, 0x0200);
    m_structures.setReg("WBITD_ReceiverBCId",      "0x9", 0x0400, 0x0400);
    m_structures.setReg("WBITD_EventBuilderBCId",  "0x9", 0x0800, 0x0800);
    m_structures.setReg("WBITD_EventBuilderLVL1",  "0x9", 0x1000, 0x1000);

    m_structures.setReg("WRECD",                   "0xa", 0xffff, 0xffff);
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


void PixMccI2Config::setupData() {
  // Setup register default values
  writeRegister("CSR_OutputMode",          0); // 40 Mbit/s
  writeRegister("CSR_ReceiverDisable",     0); // WRECD mode 0
  writeRegister("CSR_OutputPattern",       0); // Output pattern disabled
  writeRegister("CSR_Playback",            0); // Playback disabled
  writeRegister("CSR_EventBuilderDisable", 0); // Event builder enabled
  
  writeRegister("LV1_Contiguous",          0); // Single LVL1 mode

  writeRegister("FEEN",                    0xffff); // All FE enabled

  writeRegister("WFE",                     0); // All FE warnings cleared
    
  writeRegister("WMCC",                    0); // All MCC warnings cleared
  
  writeRegister("CNT_FEControlBits",       0);
  writeRegister("CNT_FEDataBits",          0);
  
  writeRegister("CAL_Delay",               0);
  writeRegister("CAL_Range",               0);
  writeRegister("CAL_Enable",              0);

  writeRegister("WBITD_FEFlags",           0);
  writeRegister("WBITD_HitOverflow",       0);
  writeRegister("WBITD_EoEOverflow",       0);
  writeRegister("WBITD_ReceiverBCId",      0);
  writeRegister("WBITD_EventBuilderBCId",  0);
  writeRegister("WBITD_EventBuilderLVL1",  0);

  writeRegister("WRECD",                   0);

  // Setup strobe default values
  for(int i=0; i<16; i++)
    m_data.getDelayRange(i) = 0.0;
}

