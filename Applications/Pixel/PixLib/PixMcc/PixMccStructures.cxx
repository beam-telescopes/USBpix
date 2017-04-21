/////////////////////////////////////////////////////////////////////
// PixMccStructures.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0.0 (CS)
//           Initial release
//

//! MCC structures: commands and data structures

#include "Bits/Bits.h"
#include "PixMcc/PixMccStructures.h"

using namespace PixLib;


PixMccStructures::PixMccStructures() {
  // Initialize some variables
  m_updateNames = false;
}

PixMccStructures::~PixMccStructures() {
  // Empty command maps
  m_commands.clear();
  m_registers.clear();
  m_fifos.clear();
}


void PixMccStructures::setCmd(std::string name, std::string pattern, int nPar, int dataLen, int outLen) {
  // Check if this command has already been inserted in the map
  std::map<std::string, MccCmd>::iterator cmd=m_commands.find(name);
  // If the command already exists update the parameters
  if(cmd != m_commands.end()) {
    cmd->second.pattern = Bits(pattern);
    cmd->second.nPar    = nPar;
    cmd->second.dataLen = dataLen;
    cmd->second.outLen  = outLen;
  }
  // Otherwise insert a new entry in the command map
  else {
    struct MccCmd newCmd(pattern, nPar, dataLen, outLen);
    m_commands.insert(make_pair(name,newCmd));
    m_updateNames = true;
  }
}

bool PixMccStructures::getCmd(std::string name, Bits &pattern, int &nPar, int &dataLen, int &outLen) {
  // Check if this command is stored in the map
  std::map<std::string, MccCmd>::iterator cmd=m_commands.find(name);
  // If the command already exists retrieve the parameters and return true
  if(cmd != m_commands.end()) {
    pattern = cmd->second.pattern;
    nPar    = cmd->second.nPar;
    dataLen = cmd->second.dataLen;
    outLen  = cmd->second.outLen;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getCmd(std::string name, Bits &pattern) {
  // Check if this command has already been inserted in the map
  std::map<std::string, MccCmd>::iterator cmd=m_commands.find(name);
  // If the command already exists retrieve the parameter and return true
  if(cmd != m_commands.end()) {
    pattern = cmd->second.pattern;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getCmdPar(std::string name, int &nPar) {
  // Check if this command has already been inserted in the map
  std::map<std::string, MccCmd>::iterator cmd=m_commands.find(name);
  // If the command already exists retrieve the parameter and return true
  if(cmd != m_commands.end()) {
    nPar = cmd->second.nPar;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getCmdData(std::string name, int &dataLen) {
  // Check if this command has already been inserted in the map
  std::map<std::string, MccCmd>::iterator cmd=m_commands.find(name);
  // If the command already exists retrieve the parameter and return true
  if(cmd != m_commands.end()) {
    dataLen = cmd->second.dataLen;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getCmdOut(std::string name, int &outLen) {
  // Check if this command has already been inserted in the map
  std::map<std::string, MccCmd>::iterator cmd=m_commands.find(name);
  // If the command already exists retrieve the parameter and return true
  if(cmd != m_commands.end()) {
    outLen = cmd->second.outLen;
    return true;
  }
  // Otherwise return false
  else return false;
}

PixMccStructures::nameIterator PixMccStructures::cmdBegin() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return begin iterator
  return m_cmdNames.begin();
}

PixMccStructures::nameIterator PixMccStructures::cmdEnd() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return end iterator
  return m_cmdNames.end();
}


void PixMccStructures::setReg(std::string name, std::string regId, int writeMask, int readMask) {
  // Check if this register has already been inserted in the map
  std::map<std::string, MccReg>::iterator reg=m_registers.find(name);
  // If the register already exists update the parameters
  if(reg != m_registers.end()) {
    reg->second.id        = Bits(regId);
    reg->second.writeMask = writeMask;
    reg->second.readMask  = readMask;
  }
  // Otherwise insert a new entry in the register map
  else {
    struct MccReg newReg(regId, writeMask, readMask);
    m_registers.insert(make_pair(name,newReg));
    m_updateNames = true;
  }
}

bool PixMccStructures::getReg(std::string name, Bits &regId, int &writeMask, int &readMask) {
  // Check if this register has already been inserted in the map
  std::map<std::string, MccReg>::iterator reg=m_registers.find(name);
  // If the register already exists retrieve the parameters and return true
  if(reg != m_registers.end()) {
    regId     = reg->second.id;
    writeMask = reg->second.writeMask;
    readMask  = reg->second.readMask;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getReg(std::string name, Bits &regId) {
  // Check if this register has already been inserted in the map
  std::map<std::string, MccReg>::iterator reg=m_registers.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_registers.end()) {
    regId = reg->second.id;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getRegWMask(std::string name, int &writeMask) {
  // Check if this register has already been inserted in the map
  std::map<std::string, MccReg>::iterator reg=m_registers.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_registers.end()) {
    writeMask = reg->second.writeMask;
    return true;
  }
  // Otherwise return false
  else return false;
}

bool PixMccStructures::getRegRMask(std::string name, int &readMask) {
  // Check if this register has already been inserted in the map
  std::map<std::string, MccReg>::iterator reg=m_registers.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_registers.end()) {
    readMask = reg->second.readMask;
    return true;
  }
  // Otherwise return false
  else return false;
}

PixMccStructures::nameIterator PixMccStructures::regBegin() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return begin iterator
  return m_regNames.begin();
}

PixMccStructures::nameIterator PixMccStructures::regEnd() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return end iterator
  return m_regNames.end();
}


void PixMccStructures::setFifo(std::string name, std::string fifoId) {
  // Check if this FIFO has already been inserted in the map
  std::map<std::string, MccFifo>::iterator fifo=m_fifos.find(name);
  // If the FIFO already exists update the parameters
  if(fifo != m_fifos.end()) {
    fifo->second.id = Bits(fifoId);
  }
  // Otherwise insert a new entry in the FIFO map
  else {
    struct MccFifo newFifo(fifoId);
    m_fifos.insert(make_pair(name,newFifo));
    m_updateNames = true;
  }
}

bool PixMccStructures::getFifo(std::string name, Bits &fifoId) {
  // Check if this FIFO has already been inserted in the map
  std::map<std::string, MccFifo>::iterator fifo=m_fifos.find(name);
  // If the FIFO already exists retrieve the parameter and return true
  if(fifo != m_fifos.end()) {
    fifoId = fifo->second.id;
    return true;
  }
  // Otherwise return false
  else return false;
}

PixMccStructures::nameIterator PixMccStructures::fifoBegin() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return begin iterator
  return m_fifoNames.begin();
}

PixMccStructures::nameIterator PixMccStructures::fifoEnd() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return end iterator
  return m_fifoNames.end();
}


void PixMccStructures::updateNames() {
  // Update command names set
  {
    m_cmdNames.clear();
    std::map<std::string, MccCmd>::iterator m, mEnd = m_commands.end();
    for(m = m_commands.begin(); m != mEnd; m++)
      m_cmdNames.push_back(m->first);
  }
  // Update register names set
  {
    m_regNames.clear();
    std::map<std::string, MccReg>::iterator m, mEnd = m_registers.end();
    for(m = m_registers.begin(); m != mEnd; m++)
      m_regNames.push_back(m->first);
  }
  // Update fifo names set
  {
    m_fifoNames.clear();
    std::map<std::string, MccFifo>::iterator m, mEnd = m_fifos.end();
    for(m = m_fifos.begin(); m != mEnd; m++)
      m_fifoNames.push_back(m->first);
  }
}
