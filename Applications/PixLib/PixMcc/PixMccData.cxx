/////////////////////////////////////////////////////////////////////
// PixMccData.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//

//! Image of internal MCC data structures

#include "Bits/Bits.h"
#include "PixMcc/PixMccStructures.h"
#include "PixMcc/PixMccData.h"

using namespace PixLib;


PixMccData::PixMccData() {
}

PixMccData::PixMccData(const PixMccData& d) {
  // Copy structures pointer
  m_structures = d.m_structures;

  // Copy register data
  m_registers.clear();
  m_registers = d.m_registers;
} 

PixMccData::~PixMccData() {
  // Empty register value map
  m_registers.clear();
}


PixMccData& PixMccData::operator = (const PixMccData& d) {
  if(this != &d) {
    // Copy register data
    //m_registers.clear();
    //m_registers = d.m_registers;
    std::map<std::string, int >::iterator ct1;
    std::map<std::string, int >::const_iterator ct2;
    for (ct2 = d.m_registers.begin(); ct2 != d.m_registers.end(); ++ct2) {
      ct1 = m_registers.find(ct2->first);
      if (ct1 != m_registers.end()) {
	ct1->second = ct2->second;
      } else {
	m_registers[ct2->first] = ct2->second;
      }
    }
  }
  return *this;
}


void PixMccData::setReg(std::string name, int value) {
  // Check if this register has already been inserted in the map
  std::map<std::string, int>::iterator reg=m_registers.find(name);

  // Get register write mask
  int writeMask;
  if(!m_structures->getRegWMask(name, writeMask)) return;
  for(; (writeMask&0x1)==0; writeMask=(writeMask>>1)) {}

  // If the register already exists update the value and return true
  if(reg != m_registers.end()) {
    reg->second = (value & writeMask);
  }
  // Otherwise create a new map entry
  else {
    m_registers.insert(make_pair(name, (value & writeMask)));
  }
}

bool PixMccData::getReg(std::string name, int &value) {
  // Check if this register has already been inserted in the map
  std::map<std::string, int>::iterator reg=m_registers.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_registers.end()) {
    value = reg->second;
    return true;
  }
  // Otherwise return false
  else return false;
}

int& PixMccData::getReg(std::string name) {
  // Check if this register has already been inserted in the map
  std::map<std::string, int>::iterator reg=m_registers.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_registers.end()) {
    return reg->second;
  }
  // Otherwise return reference to the first map entry
  else return m_registers.begin()->second;
}

void PixMccData::resetReg() {
  // Loop on all registers
  std::map<std::string, int>::iterator reg, regEnd=m_registers.end();
  for(reg=m_registers.begin(); reg!=regEnd; reg++)
    // Set to zero each register's content
    reg->second = 0x0000;
}
