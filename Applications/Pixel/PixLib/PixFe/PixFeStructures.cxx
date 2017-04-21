/////////////////////////////////////////////////////////////////////
// PixFeStructures.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 27/06/03  Version 1.0.0 (CS)
//           Initial release
//

//! FE structures: data structures

#include <iostream>

#include "Bits/Bits.h"
#include "PixFe/PixFeStructures.h"

using namespace PixLib;


PixFeStructures::PixFeStructures() {
  // Initialize name updating flag
  m_updateNames = false;

  // Empty Global Register
  m_globRegisters.clear();
  m_globRegisterLen = 0;

  // Empty Pixel Register
  m_pixRegisters.clear();
}

PixFeStructures::~PixFeStructures() {
  // Empty Global Register
  m_globRegisters.clear();

  // Empty Pixel Register
  m_pixRegisters.clear();
}


void PixFeStructures::setGlobReg(std::string name, int length, int offset) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FeGlobReg>::iterator reg=m_globRegisters.find(name);
  // If the register already exists update the parameters
  if(reg != m_globRegisters.end()) {
    m_globRegisterLen += (length - reg->second.length);
    reg->second.length = length;
    reg->second.offset = offset;
  }
  // Otherwise insert a new entry in the register map
  else {
    struct FeGlobReg newReg(length, offset);
    m_globRegisters.insert(make_pair(name,newReg));
    m_globRegisterLen += length;
    m_updateNames = true;
  }
}

bool PixFeStructures::getGlobReg(std::string name, int &length, int &offset) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FeGlobReg>::iterator reg=m_globRegisters.find(name);
  // If the register already exists retrieve the parameters and return true
  if(reg != m_globRegisters.end()) {
    length = reg->second.length;
    offset = reg->second.offset;
    return true;
  }
  // Otherwise return false
  else{
    std::cout << name << " not found pixfestructures" << std::endl;
    return false;
  }
}

bool PixFeStructures::getGlobRegLength(std::string name, int &length) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FeGlobReg>::iterator reg=m_globRegisters.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_globRegisters.end()) {
    length = reg->second.length;
    return true;
  }
  // Otherwise return false
  else{
    std::cout << name << " not found pixfestructures" << std::endl;
    return false;
  }
}

bool PixFeStructures::getGlobRegOffset(std::string name, int &offset) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FeGlobReg>::iterator reg=m_globRegisters.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_globRegisters.end()) {
    offset = reg->second.offset;
    return true;
  }
  // Otherwise return false
  else return false;
}

int PixFeStructures::getGlobRegTotLength() {
  // Return Global Register total length
  return m_globRegisterLen;
}

PixFeStructures::nameIterator PixFeStructures::globRegBegin() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return begin iterator
  return m_globRegNames.begin();
}

PixFeStructures::nameIterator PixFeStructures::globRegEnd() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return end iterator
  return m_globRegNames.end();
}


void PixFeStructures::setPixReg(std::string name, int address) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FePixReg>::iterator reg=m_pixRegisters.find(name);
  // If the register already exists update the parameters
  if(reg != m_pixRegisters.end()) {
    reg->second.address = address;
  }
  // Otherwise insert a new entry in the register map
  else {
    struct FePixReg newReg(address);
    m_pixRegisters.insert(make_pair(name,newReg));
    m_updateNames = true;
  }
}
		
bool PixFeStructures::getPixReg(std::string name, int &address) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FePixReg>::iterator reg=m_pixRegisters.find(name);
  // If the register already exists retrieve the parameters and return true
  if(reg != m_pixRegisters.end()) {
    address = reg->second.address;
    return true;
  }
  // Otherwise return false
  else return false;
}

PixFeStructures::nameIterator PixFeStructures::pixRegBegin() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return begin iterator
  return m_pixRegNames.begin();
}

PixFeStructures::nameIterator PixFeStructures::pixRegEnd() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return end iterator
  return m_pixRegNames.end();
}

		
void PixFeStructures::setTrim(std::string name, int address, int number, int maximum) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FeTrim>::iterator reg=m_trims.find(name);
  // If the register already exists update the parameters
  if(reg != m_trims.end()) {
    reg->second.address = address;
    reg->second.number  = number;
    reg->second.maximum  = maximum;
  }
  // Otherwise insert a new entry in the register map
  else {
    struct FeTrim newReg(address,number,maximum);
    m_trims.insert(make_pair(name,newReg));
    m_updateNames = true;
  }
}
		
bool PixFeStructures::getTrim(std::string name, int &address, int &number) {
  // Check if this register has already been inserted in the map
  std::map<std::string, FeTrim>::iterator reg=m_trims.find(name);
  // If the register already exists retrieve the parameters and return true
  if(reg != m_trims.end()) {
    address = reg->second.address;
    number  = reg->second.number;
    return true;
  }
  // Otherwise return false
  else return false;
}
		
bool PixFeStructures::getTrimMax(std::string name, int &maximum) {
  // Check if this register has already been inserted in the map
  maximum = 0;
  std::map<std::string, FeTrim>::iterator reg=m_trims.find(name);
  // If the register already exists retrieve the parameters and return true
  if(reg != m_trims.end()) {
    maximum = reg->second.maximum;
    return true;
  }
  // Otherwise return false
  else return false;
}
		
PixFeStructures::nameIterator PixFeStructures::trimBegin() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return begin iterator
  return m_trimNames.begin();
}

PixFeStructures::nameIterator PixFeStructures::trimEnd() {
  // Check if an update is needed
  if(m_updateNames) {
    updateNames();
    m_updateNames = false;
  }

  // Return end iterator
  return m_trimNames.end();
}


void PixFeStructures::updateNames() {
  // Update global registers names set
  m_globRegNames.clear();
  std::map<std::string, FeGlobReg>::iterator m1, mEnd1 = m_globRegisters.end();
  for(m1 = m_globRegisters.begin(); m1 != mEnd1; m1++)
    m_globRegNames.push_back(m1->first);

  // Update pixel registers names set
  m_pixRegNames.clear();
  std::map<std::string, FePixReg>::iterator m2, mEnd2 = m_pixRegisters.end();
  for(m2 = m_pixRegisters.begin(); m2 != mEnd2; m2++)
    m_pixRegNames.push_back(m2->first);

  // Update trim names set
  m_trimNames.clear();
  std::map<std::string, FeTrim>::iterator m3, mEnd3 = m_trims.end();
  for(m3 = m_trims.begin(); m3 != mEnd3; m3++)
    m_trimNames.push_back(m3->first);
}
