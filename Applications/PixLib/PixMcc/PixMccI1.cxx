//////////////////////////////////////////////////////////////////////
// PixMccI1.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//
// 15/03/04  Version 1.1 (CS)
//           Revised version using PixMccI1Config
//

//! MCC-I1 implementation

#include <math.h>
#include <algorithm>
#include <vector>

#include "Bits/Bits.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/PixController.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMccExc.h"
#include "PixMcc/PixMccStructures.h"
#include "PixMcc/PixMccData.h"
#include "PixMcc/PixMccI1Config.h"
#include "PixMcc/PixMccI1.h"


using namespace PixLib;

//! Constructor
PixMccI1::PixMccI1(DBInquire *dbInquire, PixModule *mod, std::string name) :
  m_dbInquire(dbInquire), m_module(mod), m_name(name) {
  // Create MCC config
  m_data = new PixMccI1Config((m_dbInquire!=0)?m_dbInquire->getDB():0, m_dbInquire, m_name);
}


//! Destructor
PixMccI1::~PixMccI1() {
}


//! Config object accessor
Config &PixMccI1::config() {
  return m_data->config();
} 


//! Write into a register mem copy
void PixMccI1::writeRegister(std::string regName, int value) {
  m_data->writeRegister(regName, value);
}

//! Read a register from mem copy
int PixMccI1::readRegister(std::string regName) {
  return m_data->readRegister(regName);
}

//! Set register value into mem copy and device
void PixMccI1::setRegister(std::string regName, int value) {
  // Check if register exists
  Bits wrCommand, cmdPattern, regId;
  if(!m_data->structures().getReg(regName, regId)) {
    throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Write value to software register copy
  m_data->writeRegister(regName, value);

  // Build WriteRegister command
  int nPar, dataLen, outLen;
  if(!m_data->structures().getCmd("WRITE_REGISTER", cmdPattern, nPar, dataLen, outLen)) {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Get entire register value
  int position = regName.find("_");
  std::string mainReg(regName, 0, position);
  int mainVal = m_data->readRegister(mainReg);

  Bits val(dataLen - regId.size(), mainVal);
  wrCommand = cmdPattern + regId + val;

  // Send command
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(wrCommand, moduleMask);
}

//! Get register value from device to mem
int PixMccI1::getRegister(std::string regName) {
  // Build ReadRegister command
  Bits rrCommand, cmdPattern, regId;
  int wrMask, rdMask, nPar, dataLen, outLen;
  if(!m_data->structures().getCmd("READ_REGISTER", cmdPattern, nPar, dataLen, outLen))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return -1;
  }
  if(!m_data->structures().getReg(regName, regId, wrMask, rdMask)) {
    throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return -1;
  }

  rrCommand = cmdPattern + regId;

  // Send command and get return value
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
//   std::vector<Bits> output;
//   m_module->getPixModGroup()->getPixController()->sendCommandOutSync(rrCommand, moduleMask, output);
  m_module->getPixModGroup()->getPixController()->sendCommand(rrCommand, moduleMask);

  // Check output length
//   int headerLength, dataOutLength;
//   headerLength  = (int)floor(outLen/100.0);
//   dataOutLength = outLen-headerLength*100;
//   if(output.size()!=1 || output[0].size()!=(headerLength+dataOutLength)) {
//     throw PixMccExc(PixMccExc::WRONG_MCC_OUTPUT, PixMccExc::ERROR, m_name); 
//     return -1;
//   }

  // Check header
//   Bits header(headerLength);
//   header = output[0].range(0, headerLength-1);
//   if(header != m_data->structures().getHeader()) {
//     throw PixMccExc(PixMccExc::WRONG_MCC_OUTPUT, PixMccExc::ERROR, m_name); 
//     return -1;
//   }

  // Get entire register value
//   int value = output[0].get(headerLength, output.size()-1);

  // Update software register copy
//   int position = regName.find("_");
//   std::string mainReg(regName, 0, position);
//   writeRegister(mainReg, value);

  // Return (sub)register value
  int retValue=0;

//   if(wrMask==0x0 && rdMask==0x0) { // Read-only register case
//     retValue = value & rdMask;
//     for( ; (rdMask&0x1)==0x0; ) {rdMask=rdMask>>1; retValue=retValue>>1;}
//   }
//   else { // Read/write register case
//     retValue = readRegister(regName);
//   }

  return retValue;
}


//! Write into a FIFO (directly into device)
void PixMccI1::setFifo(std::string fifoName, int value) {

  // Build WriteFIFO command
  Bits wfCommand, cmdPattern, fifoId;
  int nPar, dataLen, outLen;
  if(!m_data->structures().getCmd("WRITE_FIFO", cmdPattern, nPar, dataLen, outLen))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }
  if(!m_data->structures().getFifo(fifoName, fifoId)) {
    throw PixMccExc(PixMccExc::FIFO_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }
  Bits val(dataLen - fifoId.size(), value);
  wfCommand = cmdPattern + fifoId + val;

  // Send command
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(wfCommand, moduleMask);
}

//! Read from a FIFO (directly from device)
int PixMccI1::getFifo(std::string fifoName) {

  // Build ReadFIFO command
  Bits rfCommand, cmdPattern, fifoId;
  int nPar, dataLen, outLen;
  if(!m_data->structures().getCmd("READ_FIFO", cmdPattern, nPar, dataLen, outLen))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return -1;
  }
  if(!m_data->structures().getFifo(fifoName, fifoId)) {
    throw PixMccExc(PixMccExc::FIFO_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return -1;
  }
  rfCommand = cmdPattern + fifoId;

  // Send command and get return value
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
//   std::vector<Bits> output;
//   m_module->getPixModGroup()->getPixController()->sendCommandOutSync(rfCommand, moduleMask, output);
  m_module->getPixModGroup()->getPixController()->sendCommand(rfCommand, moduleMask);

  // Check output length
//   int headerLength, dataOutLength;
//   headerLength  = (int)floor(outLen/100.0);
//   dataOutLength = outLen-headerLength*100;
//   if(output.size()!=1 || output[0].size()!=(headerLength+dataOutLength)) {
//     throw PixMccExc(PixMccExc::WRONG_MCC_OUTPUT, PixMccExc::ERROR, m_name); 
//     return -1;
//   }

  // Check header
//   Bits header(headerLength);
//   header = output[0].range(0, headerLength-1);
//   if(header != m_data->structures().getHeader()) {
//     throw PixMccExc(PixMccExc::WRONG_MCC_OUTPUT, PixMccExc::ERROR, m_name); 
//     return -1;
//   }

  // Check return value
  int retValue = 0; //output[0].get(headerLength, output.size()-1);
  return retValue;
}


//! Configure the MCC with the current configuration
void PixMccI1::configure() {
  
  // Write all register values
  std::string lastReg = "";
  PixMccData::MccRegConstIterator reg, regEnd=m_data->data().endReg();
  for(reg=m_data->data().beginReg(); reg!=regEnd; reg++) {
    // Extract main register name
    int position = reg->first.find("_");
    std::string mainReg(reg->first, 0, position);
    // Check if the register has already been loaded
    if(mainReg != lastReg)
      writeRegister(reg->first, reg->second);
    lastReg = mainReg;
  }
}


//! Restore a configuration from map 
bool PixMccI1::restoreConfig(std::string configName) {
  // Look for the requested configuration inside config map
  if (m_configs.find(configName) != m_configs.end()) {
    // Restore the selected configuration
    *m_data = *(m_configs[configName]);
    return true;
  }
  return false;
}


//! Store a configuration into map
void PixMccI1::storeConfig(std::string configName) {  
  // Look for the requested configuration inside config map
  PixMccI1Config *cfg = new PixMccI1Config(*m_data);
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
  }
  m_configs[configName] = cfg;
}


//! Remove a configuration from the map
void PixMccI1::deleteConfig(std::string configName) {
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
    m_configs.erase(configName);
  }
}


//! Read the config from DB
void PixMccI1::loadConfig(std::string configName) {
  m_data->loadConfig(configName);
}


//! Save the config to DB
void PixMccI1::saveConfig(std::string /*configName*/) {
}


//! Enable a FE (mem copy)
void PixMccI1::writeEnableFE(int feNum) {

  // Read value from software FEEN copy
  int regValue = readRegister("FEEN");

  // Update FEEN value
  regValue |= (0x1<<feNum);

  // Write new FEEN value
  writeRegister("FEEN", regValue);
}

//! Enable a FE (mem and device)
void PixMccI1::setEnableFE(int feNum) {

  // Write into mem
  try {
    writeEnableFE(feNum);
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Write into device
  setRegister("FEEN");
}


//! Disable a FE (mem copy)
void PixMccI1::writeDisableFE(int feNum) {

  // Read value from software FEEN copy
  int regValue = readRegister("FEEN");

  // Update FEEN value
  regValue &= (~(0x1<<feNum));

  // Write new FEEN value
  writeRegister("FEEN", regValue);
}

//! Disable a FE (mem and device)
void PixMccI1::setDisableFE(int feNum) {

  // Write into mem
  try{ 
    writeDisableFE(feNum);
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Write into device
  setRegister("FEEN");
}


//! Enable all FEs (mem copy)
void PixMccI1::writeEnableAllFE() {

  // Write new FEEN value
  int regValue = 0xffff;
  writeRegister("FEEN", regValue);
}

//! Enable all FEs (mem and device)
void PixMccI1::setEnableAllFE() {

  // Write into mem
  try {
    writeEnableAllFE();
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Write into device
  setRegister("FEEN");
}


//! Disable all FEs (mem copy)
void PixMccI1::writeDisableAllFE() {

  // Write new FEEN value
  int regValue = 0x0000;
  writeRegister("FEEN", regValue);
}

//! Disable all FEs (mem and device)
void PixMccI1::setDisableAllFE() {

  // Write into mem
  try {
    writeDisableAllFE();
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Write into device
  setRegister("FEEN");
}


//! Check if a FE is enabled (looking at mem copy)
bool PixMccI1::feEnabled(int feNum) {

  // Read FEEN value
  int regValue = readRegister("FEEN");

  // Check if the selected FE is enabled
  return((regValue>>feNum)&0x1);
}


//! Set MCC output speed (mem copy)
void PixMccI1::writeOutSpeed(MccOutputSpeed speed) {

  // Update CSR value
  switch (speed) {
  case SINGLE_LINK_40 : 
    writeRegister("CSR_OutputMode", 0x0);
  case DOUBLE_LINK_40 :
    writeRegister("CSR_OutputMode", 0x1);
  case SINGLE_LINK_80 :
    writeRegister("CSR_OutputMode", 0x2);
  case DOUBLE_LINK_80 :
    writeRegister("CSR_OutputMode", 0x3);
  default :
    writeRegister("CSR_OutputMode", 0x0);
  }
}

//! Set MCC output speed (mem and device)
void PixMccI1::setOutSpeed(MccOutputSpeed speed) {

  // Write into mem
  try {
    writeOutSpeed(speed);
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Write into device
  setRegister("CSR");
}


//! Write strobe parameters (mem copy)
void PixMccI1::writeStrobe(int width, int delay, int range, bool calEnable) {

  // Check if calibration is enabled
  if(calEnable) {
    // Setup all calibration parameters and set calibration enable bit to 1
    try {
      writeRegister("CNT", (width & 0xffff));
      writeRegister("CAL_Enable", 0x1);
      writeRegister("CAL_Range",  range);
      writeRegister("CAL_Delay",  delay);
    } catch(PixMccExc &e) {
      throw e;
      return;
    }
  } else {
    // Set calibration enable bit to 0
    try {
      writeRegister("CNT_Enable", 0x0);
    } catch(PixMccExc &e) {
      throw e;
      return;
    }
  }
} 

//! Write strobe parameters (mem and device)
void PixMccI1::setStrobe(int width, int delay, int range, bool calEnable) {

  // Write into mem
  try {
    writeStrobe(width, delay, range, calEnable);
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Write into device
  setRegister("CNT");
  setRegister("CAL");
}		


//! Test the event builder
void PixMccI1::testEventBuilder() {
}


//! Test the FIFOs
void PixMccI1::testFifo() {
}


//! Issue a full MCC reset
void PixMccI1::globalResetMCC() {

  // Reset software registers
  m_data->data().resetReg();	

  // Build GlobalResetMCC command
  Bits resetCommand;
  if(!m_data->structures().getCmd("GLOBAL_RESET_MCC", resetCommand))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Send command
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(resetCommand, moduleMask);
}


//! Issue a FE reset
void PixMccI1::globalResetFE(int nCK) {

  // Build GlobalResetFE command
  Bits cmdPattern, resetCommand;
  int nPar, dataLen, outLen;
  if(!m_data->structures().getCmd("GLOBAL_RESET_FE", cmdPattern, nPar, dataLen, outLen)) {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  unsigned int syncw = abs(nCK)/2;
  if (syncw > 0xf) syncw = 0xf;
  Bits val(dataLen - 4, syncw);
  Bits addr(4,0x0);
  resetCommand = cmdPattern + addr + val;
  // Send command
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(resetCommand, moduleMask);
}


//! Send an ECR command
void PixMccI1::ecr() {

  // Reset LVL1ID software value
  int regValue = m_data->readRegister("LV1");
  regValue &= 0xff00;
  writeRegister("LV1", regValue);
	
  // Build ECR command
  Bits resetCommand;
  if(!m_data->structures().getCmd("ECR", resetCommand))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Send command
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(resetCommand, moduleMask);
}


//! Send a BCR command
void PixMccI1::bcr() {

  // Build BCR command
  Bits resetCommand;
  if(!m_data->structures().getCmd("BCR", resetCommand))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Send command
  // TO DO: Build module mask
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(resetCommand, moduleMask);
}


//! Send a SyncFE command
void PixMccI1::syncFE() {

  // Build SYNC command
  Bits resetCommand;
  if(!m_data->structures().getCmd("SYNC", resetCommand))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Send command
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(resetCommand, moduleMask);
}

//! Issue a trigger command
void PixMccI1::trigger() {
  // Build GlobalResetMCC command
  Bits triggerCommand;
  if(!m_data->structures().getCmd("TRIGGER", triggerCommand))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Send command
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(triggerCommand, moduleMask);
}


//! Start MCC event builder
void PixMccI1::enableDataTaking() {

  // Build EnableDataTaking command
  Bits command;
  if(!m_data->structures().getCmd("ENABLE_DATATAKE", command))  {
    throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
    return;
  }

  // Send command
  int moduleMask = (0x1<<m_module->moduleId());
  m_module->getPixModGroup()->getPixController()->sendCommand(command, moduleMask);
}


//! Send FE config
void PixMccI1::writeFE(const Bits &/*cmd*/, int /*dataLen*/) {
}


//! Get FE config
void PixMccI1::readFE(const Bits &/*cmd*/, int /*dataLen*/, Bits &/*out*/) {
}


//! Dump MCC info to output stream
void PixMccI1::dump(std::ostream &out) {
  m_data->dump(out);
}


//! Set register value into device from mem copy
void PixMccI1::setRegister(std::string regName) {
  // Read mem value
  int value;
  try {
    value = readRegister(regName);
  } catch(PixMccExc &e) {
    throw e;
    return;
  }

  // Set mem value into device
  setRegister(regName, value);
}

//! Ask the MCC to generate a 40 Mb/s test pattern
void PixMccI1::prepareTestPattern(std::vector<unsigned int> &val, Bits &prep, Bits &exec, Bits &ret) {
  if (val.size() > 128) {
    throw PixMccExc(PixMccExc::FIFO_OVERFLOW, PixMccExc::ERROR, m_name); 
    return;
  } else {
    // Build WriteFIFO/ReadFIFO sequence
    Bits wCmd, rCmd, fifoId;
    Bits resetCommand;
    Bits writeRegister, regId;
    int nPar, dataLen, outLen;
    prep.resize(0);
    exec.resize(0);
    ret.resize(0);
    Bits zero(11, 0x0);
    Bits zeroE(17, 0x0);
    Bits zeroO(19, 0x0);
    Bits head(5, 0x1d);

    Bits triggerCommand;
    if(!m_data->structures().getCmd("TRIGGER", triggerCommand))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }

    if(!m_data->structures().getCmd("GLOBAL_RESET_MCC", resetCommand))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    prep = resetCommand + zero;

    if(!m_data->structures().getCmd("WRITE_REGISTER", writeRegister, nPar, dataLen, outLen)) {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    if(!m_data->structures().getReg("FEEN", regId)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits valFEEN(dataLen - regId.size(), 0xffff);
    prep += writeRegister + regId + valFEEN + zero;

    if(!m_data->structures().getReg("CSR", regId)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits valCSR(dataLen - regId.size(), 0x1c);
    prep += writeRegister + regId + valCSR + zero;

    if(!m_data->structures().getCmd("READ_FIFO", rCmd, nPar, dataLen, outLen))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    if(!m_data->structures().getCmd("WRITE_FIFO", wCmd, nPar, dataLen, outLen))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    if(!m_data->structures().getFifo("FIFO00", fifoId)) {
      throw PixMccExc(PixMccExc::FIFO_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    unsigned int i;
    for (i=0; i<val.size(); i++) {
      int n = dataLen - (int)fifoId.size();
      int d = (int)(val[i]&0x7ffffff);
      Bits val(n, d);
      prep += wCmd + fifoId + val + zero;
      ret += head + val;
      if (i%2 == 0) {
        ret += zeroE;
      } else {
        ret += zeroO;
      } 
   }

    exec = triggerCommand + zero;

    exec += resetCommand + zero;

    for (i=0; i<val.size(); i++) {
      exec += rCmd + fifoId + zero + zero + zero;
    }
  }
}

//! Ask the MCC to generate a test event
void PixMccI1::prepareTestEvent(std::vector<unsigned int> &val, Bits &prep, Bits &exec, Bits &ret) {
  if (val.size() > 128) {
    throw PixMccExc(PixMccExc::FIFO_OVERFLOW, PixMccExc::ERROR, m_name); 
    return;
  } else {
    // Reset bits
    prep.resize(0);
    exec.resize(0);
    ret.resize(0);

    // Build WriteFIFO/ReadFIFO sequence
    int nPar, dataLen, outLen;
    if (val.size() < 2 || val[0] == 0) {
      val.clear();
      val.push_back(0xffff);
      val.push_back(0x3e5aa);
      val.push_back(0x23f55);
      val.push_back(0x13f55);
    }
    unsigned int feMask = val[0];
    unsigned int csrVal = 0x50 + (readRegister("CSR_OutputMode")&0xf);

    Bits zero(1, 0x0);
    Bits one(1, 0x1);
    Bits trailer(22, 0x0);
    Bits header(5, 0x1d);
    Bits space(32, 0x0);

    Bits triggerCommand;
    if(!m_data->structures().getCmd("TRIGGER", triggerCommand))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits bcrCommand;
    if(!m_data->structures().getCmd("BCR", bcrCommand))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits resetCommand;
    if(!m_data->structures().getCmd("GLOBAL_RESET_MCC", resetCommand))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits readRegister;
    if(!m_data->structures().getCmd("READ_REGISTER", readRegister))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits enableDatatake;
    if(!m_data->structures().getCmd("ENABLE_DATATAKE", enableDatatake))  {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits writeReceiver;
    if(!m_data->structures().getCmd("WRITE_RECEIVER", writeReceiver, nPar, dataLen, outLen)) {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits writeRegister;
    if(!m_data->structures().getCmd("WRITE_REGISTER", writeRegister, nPar, dataLen, outLen)) {
      throw PixMccExc(PixMccExc::CMD_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits feenId;
    if(!m_data->structures().getReg("FEEN", feenId)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits csrId;
    if(!m_data->structures().getReg("CSR", csrId)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits lvl1Id;
    if(!m_data->structures().getReg("LV1", lvl1Id)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    Bits cntId;
    if(!m_data->structures().getReg("CNT", cntId)) {
      throw PixMccExc(PixMccExc::REG_NOT_FOUND, PixMccExc::ERROR, m_name); 
      return;
    }
    
    prep = resetCommand + space;
    Bits valFEEN(dataLen - feenId.size(), feMask);
    prep += writeRegister + feenId + valFEEN + space;
    Bits valCSR(dataLen - csrId.size(), csrVal);
    prep += writeRegister + csrId + valCSR + space;
    Bits valLVL1(dataLen - lvl1Id.size(), 0x0);
    prep += writeRegister + lvl1Id + valLVL1 + space;

    exec = triggerCommand + space + enableDatatake + space + bcrCommand + space;
    Bits valCNT(dataLen - cntId.size(), val.size()*4);
    exec += writeRegister + cntId + valCNT + space;
    exec += writeReceiver + Bits(4,0x0);
    ret = header+Bits(8,0x1)+one+Bits(8,0x5+0x20*val.size())+one;

    bool first = true;
    for (int fe=0; fe<16; fe++) {
      if ((feMask&(0x1<<fe)) != 0) {
	ret += Bits(8, 0xe0+fe) + one;
	for (unsigned int i=1; i<val.size(); i++) {
	  int tot = val[i] & 0xff;
	  int col = ((val[i] & 0x1f00) >> 8);
	  int row = ((val[i] & 0x1fe00) >> 13);
	  if (col > 17) col = 17;
	  if (row > 159) row = 159;
	  if (first) {
	    exec += Bits(5, 0x11) + Bits(8, row) + Bits(5, col) + Bits(8, tot); 
	  }
	  ret += Bits(8, row) + Bits(5, col) + Bits(8, tot) + one;
	}
	first = false;
      }
    }
    exec += Bits(26, 0x23c0000) + Bits(6*val.size()) + space + space;
    //exec += readRegister + feenId + space + space + space;
    //exec += readRegister + cntId + space + space + space;
    //exec += writeRegister + feenId + valFEEN + space;
    exec += enableDatatake + space + space;
    exec += triggerCommand + space + space + space;
    ret += trailer;
  }
}
