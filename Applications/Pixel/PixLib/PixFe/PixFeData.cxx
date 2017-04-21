/////////////////////////////////////////////////////////////////////
// PixFeData.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//
//! Image of internal FE data structures

#include <iostream>

#include "Bits/Bits.h"
#include "Config/ConfMask.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeData.h"

using namespace PixLib;


PixFeData::PixFeData() {
}

PixFeData::~PixFeData() {
  // Empty Global Register value map
  m_globRegisters.clear();

  // Clear Pixel Register value map
  m_pixRegisters.clear();

  // Clear Trim value map
  m_trims.clear();
}


PixFeData& PixFeData::operator = (const PixFeData& d) {

  if(this != &d) {
    // Copy indexing parameters
    m_index   = d.m_index;
    m_cmdAddr = d.m_cmdAddr;
    
    // Copy enable parameters
    m_cfgEnable  = d.m_cfgEnable; 
    m_scanEnable = d.m_scanEnable;
    m_dacsEnable = d.m_dacsEnable;
    
    // Copy uniform DAC values
    m_defPixelFdac = d.m_defPixelFdac;
    m_defPixelTdac = d.m_defPixelTdac;
    
    // Copy injection parameters
    m_cInjLo	       = d.m_cInjLo;	    
    m_cInjHi	       = d.m_cInjHi;	    
    m_cInjMed	       = d.m_cInjMed;
    m_vcalGradient[0]  = d.m_vcalGradient[0];    
    m_vcalGradient[1]  = d.m_vcalGradient[1];    
    m_vcalGradient[2]  = d.m_vcalGradient[2];    
    m_vcalGradient[3]  = d.m_vcalGradient[3];    
    m_offsetCorrection = d.m_offsetCorrection;
    
    // Copy Global Register data
    //m_globRegisters.clear();
    //m_globRegisters = d.m_globRegisters;
    std::map<std::string, int>::iterator it1;
    std::map<std::string, int>::const_iterator it2;
    for (it2 = d.m_globRegisters.begin(); it2 != d.m_globRegisters.end(); ++it2) {
      it1 = m_globRegisters.find(it2->first);
      if (it1 != m_globRegisters.end()) {
	it1->second = it2->second;
      } else {
	m_globRegisters[it2->first] = it2->second;
      }
    }
    
    // Copy Pixel Register data
    //m_pixRegisters.clear();
    //m_pixRegisters = d.m_pixRegisters;
    std::map<std::string, ConfMask<bool> >::iterator bt1;
    std::map<std::string, ConfMask<bool> >::const_iterator bt2;
    for (bt2 = d.m_pixRegisters.begin(); bt2 != d.m_pixRegisters.end(); ++bt2) {
      bt1 = m_pixRegisters.find(bt2->first);
      if (bt1 != m_pixRegisters.end()) {
	bt1->second = bt2->second;
      } else {
	m_pixRegisters[bt2->first] = bt2->second;
      }
    }

    // Copy Trim data
    //m_trims.clear();
    //m_trims = d.m_trims;
    std::map<std::string, ConfMask<unsigned short int> >::iterator ct1;
    std::map<std::string, ConfMask<unsigned short int> >::const_iterator ct2;
    for (ct2 = d.m_trims.begin(); ct2 != d.m_trims.end(); ++ct2) {
      ct1 = m_trims.find(ct2->first);
      if (ct1 != m_trims.end()) {
	ct1->second = ct2->second;
      } else {
	m_trims[ct2->first] = ct2->second;
      }
    }
  }
  
  return *this;
}


void PixFeData::copyGlobReg(const PixFeData& source) {
    // Copy Global Register data
    m_globRegisters.clear();
    m_globRegisters = source.m_globRegisters;
}

void PixFeData::copyPixReg(const PixFeData& source, std::string name) {
  // Check if all registers must be copied
  if(name == "ALL") {
    // Copy all Pixel Register data
    m_pixRegisters.clear();
    m_pixRegisters = source.m_pixRegisters;
  } else {
    // Copy a single Pixel Register data
    std::map<std::string, ConfMask<bool> >::iterator p;
    std::map<std::string, ConfMask<bool> >::const_iterator sourcep;
    sourcep = (source.m_pixRegisters).find(name);
    if(sourcep!=source.m_pixRegisters.end()) {
      p = m_pixRegisters.find(name);
      if(p!=m_pixRegisters.end())
	p->second = sourcep->second;
      else
	m_pixRegisters.insert(std::make_pair(name, sourcep->second));
    }
  }
}

void PixFeData::copyTrim(const PixFeData& source, std::string name) {
  // Check if all registers must be copied
  if(name == "ALL") {
    // Copy all Trim data
    m_trims.clear();
    m_trims = source.m_trims;
  } else {
    // Copy a single Trim data
    std::map<std::string, ConfMask<unsigned short int> >::iterator p;
    std::map<std::string, ConfMask<unsigned short int> >::const_iterator sourcep;
    sourcep = source.m_trims.find(name);
    if(sourcep!=source.m_trims.end()) {
      p = m_trims.find(name);
      if(p!=m_trims.end())
	p->second = sourcep->second;
      else
	m_trims.insert(std::make_pair(name, sourcep->second));
    }
  }
}


void PixFeData::setGlobReg(std::string name, int value) {
  // Check if this register has already been inserted in the map
  std::map<std::string, int>::iterator reg=m_globRegisters.find(name);

  // Get register write mask
  int len, writeMask=0;
  if(!m_structures->getGlobRegLength(name, len)) return;
  for(int i=0; i<len; i++)
    writeMask |= (0x1 << i);

  // If the register already exists update the value and return true
  if(reg != m_globRegisters.end())
    reg->second = (value & writeMask);
  // Otherwise create a new map entry
  else
    m_globRegisters.insert(make_pair(name, (value & writeMask)));
}

int& PixFeData::getGlobReg(std::string name, bool &ok) {
  // Check if this register has already been inserted in the map
  std::map<std::string, int>::iterator reg=m_globRegisters.find(name);
  // If the register already exists retrieve the parameter and return true
  if(reg != m_globRegisters.end()) {
    ok = true;
    return reg->second;
  }
  // Otherwise return false
  else {
    ok = false;
    int *temp=0;
    return *temp;
  }
}

bool PixFeData::getGlobReg(Bits &value) {
  // Empty Global Register
  value.resize(0);

  // Convert to Bits all Global Registers
  int len, off;
  std::map<int, Bits> sortedReg;
  std::map<std::string, int>::iterator reg, regEnd=m_globRegisters.end();
  for(reg=m_globRegisters.begin(); reg!=regEnd; reg++) {
    if(!m_structures->getGlobReg(reg->first, len, off)) {
      std::cout << reg->first.c_str() << "caused error" << std::endl;
      return false;
    }
    Bits regVal(len, reg->second);
    sortedReg.insert(std::make_pair(off, regVal));
  }

  // Fill Global Register
  std::map<int, Bits>::iterator regBits, regBitsEnd=sortedReg.end();
  for(regBits=sortedReg.begin(); regBits!=regBitsEnd; regBits++)
    value += regBits->second;

  // Check Global Register length
  if(value.size() != m_structures->getGlobRegTotLength()) 
    {
      std::cout << value.size() << " not equal to getglobregtotlenght " << m_structures->getGlobRegTotLength() << std::endl;
      //    return false;
    }
  return true;
}

void PixFeData::resetGlobReg() {
  // Loop on all Global registers
  std::map<std::string, int>::iterator reg, regEnd=m_globRegisters.end();
  for(reg=m_globRegisters.begin(); reg!=regEnd; reg++)
    // Set to zero each register's content
    reg->second = 0;
}


void PixFeData::setPixReg(std::string name, ConfMask<bool> &value) {
  // Check if this register has already been inserted in the map
  int addr;
  if(!m_structures->getPixReg(name, addr)) return;
  std::map<std::string, ConfMask<bool> >::iterator reg=m_pixRegisters.find(name);

  // If the register doesn't exists create it
  if(reg == m_pixRegisters.end())
    m_pixRegisters.insert(std::make_pair(name, value));
  // Update register value
  else
    reg->second = value;
}

void PixFeData::setPixReg(std::string name, int row, int col, bool value) {
  // Check row and col number
  if(!(row>=0 && row<m_structures->nRow() && col>=0 && col<m_structures->nCol())) return;

  // Check if this register has already been inserted in the map
  int addr;  
  if(!m_structures->getPixReg(name, addr)) return;
  std::map<std::string, ConfMask<bool> >::iterator reg=m_pixRegisters.find(name);

  // If the register doesn't exists create it
  if(reg == m_pixRegisters.end())
    m_pixRegisters.insert(std::make_pair(name, ConfMask<bool>(m_structures->nCol(), m_structures->nRow(), 1, 1)));
  // Update register value
  else
    reg->second.set(col, row, value);
}

void PixFeData::setPixRegCol(std::string name, int col, bool value) {
  // Check col number
  if(!(col>=0 && col<m_structures->nCol())) return;

  // Check if this register has already been inserted in the map
  int addr;  
  if(!m_structures->getPixReg(name, addr)) return;
  std::map<std::string, ConfMask<bool> >::iterator reg=m_pixRegisters.find(name);

  // If the register doesn't exists create it
  if(reg == m_pixRegisters.end()) {
    ConfMask<bool> temp(m_structures->nCol(), m_structures->nRow(), 1, 1);
    temp.setCol(col, value);
    m_pixRegisters.insert(std::make_pair(name, temp));
  }
  // Update register value
  else
    reg->second.setCol(col, value);
}

void PixFeData::setPixRegRow(std::string name, int row, bool value) {
  // Check row number
  if(!(row>=0 && row<m_structures->nRow())) return;

  // Check if this register has already been inserted in the map
  int addr;  
  if(!m_structures->getPixReg(name, addr)) return;
  std::map<std::string, ConfMask<bool> >::iterator reg=m_pixRegisters.find(name);

  // If the register doesn't exists create it
  if(reg == m_pixRegisters.end()) {
    ConfMask<bool> temp(m_structures->nCol(), m_structures->nRow(), 1, 1);
    temp.setRow(row, value);
    m_pixRegisters.insert(std::make_pair(name, temp));
  }
  // Update register value
  else
    reg->second.setRow(row, value);
}

ConfMask<bool>& PixFeData::getPixReg(std::string name, bool &ok) {
  // Check if register really exists
  int addr;
  if(m_structures->getPixReg(name, addr)) {
    std::map<std::string, ConfMask<bool> >::iterator reg=m_pixRegisters.find(name);
    if(reg != m_pixRegisters.end()) {
      // Return register
      ok = true;
      return reg->second;
    }
  }
  ok = false;
  ConfMask<bool> *temp=0;
  return *temp;
}

bool PixFeData::getPixReg(std::string name, int row, int col, bool &value) {
  // Check if register really exists
  int addr;
  if(!m_structures->getPixReg(name, addr)) return false;
  std::map<std::string, ConfMask<bool> >::iterator reg=m_pixRegisters.find(name);
  if(reg == m_pixRegisters.end()) return false;

  // Fill output
  value = reg->second[col][row];
  return true;
}

void PixFeData::resetPixReg(std::string name) {
  // Clear any existing mask map
  std::map<std::string, ConfMask<bool> >::iterator p=m_pixRegisters.find(name);
  if(p!=m_pixRegisters.end())
    p->second = ConfMask<bool>(m_structures->nCol(), m_structures->nRow(), 1, 1);
  else 
    m_pixRegisters.insert(std::make_pair(name, ConfMask<bool>(m_structures->nCol(), m_structures->nRow(), 1, 1)));
}


void PixFeData::setTrim(std::string name, ConfMask<unsigned short int> &value) {
  // Check if this trim has already been inserted in the map
  int addr, number;
  if(!m_structures->getTrim(name, addr, number)) return;
  std::map<std::string, ConfMask<unsigned short int> >::iterator trim=m_trims.find(name);

  // If the trim doesn't exists create it
  if(trim == m_trims.end())
    m_trims.insert(std::make_pair(name, value));
  // Update trim value
  else
    trim->second = value;
}

void PixFeData::setTrim(std::string name, int row, int col, unsigned short int value) {
  // Check row and col number
  if(!(row>=0 && row<m_structures->nRow() && col>=0 && col<m_structures->nCol())) return;

  // Check if this trim has already been inserted in the map
  int addr, number;  
  if(!m_structures->getTrim(name, addr, number)) return;
  std::map<std::string, ConfMask<unsigned short int> >::iterator trim=m_trims.find(name);

  // If the trim doesn't exists create it
  if(trim == m_trims.end())
    m_trims.insert(std::make_pair(name, ConfMask<unsigned short int>(m_structures->nCol(), m_structures->nRow(), 0xff, 0xff)));
  // Update trim value
  else
    trim->second.set(col, row, value);
}

void PixFeData::setTrimCol(std::string name, int col, unsigned short int value) {
  // Check col number
  if(!(col>=0 && col<m_structures->nCol())) return;

  // Check if this trim has already been inserted in the map
  int addr, number;  
  if(!m_structures->getTrim(name, addr, number)) return;
  std::map<std::string, ConfMask<unsigned short int> >::iterator trim=m_trims.find(name);

  // If the trim doesn't exists create it
  if(trim == m_trims.end()) {
    ConfMask<unsigned short int> temp(m_structures->nCol(), m_structures->nRow(), 0xff, 0xff);
    temp.setCol(col, value);
    m_trims.insert(std::make_pair(name, temp));
  }
  // Update trim value
  else
    trim->second.setCol(col, value);
}

void PixFeData::setTrimRow(std::string name, int row, unsigned short int value) {
  // Check row number
  if(!(row>=0 && row<m_structures->nRow())) return;

  // Check if this trim has already been inserted in the map
  int addr, number;  
  if(!m_structures->getTrim(name, addr, number)) return;
  std::map<std::string, ConfMask<unsigned short int> >::iterator trim=m_trims.find(name);

  // If the trim doesn't exists create it
  if(trim == m_trims.end()) {
    ConfMask<unsigned short int> temp(m_structures->nCol(), m_structures->nRow(), 0xff, 0xff);
    temp.setRow(row, value);
    m_trims.insert(std::make_pair(name, temp));
  }
  // Update trim value
  else
    trim->second.setRow(row, value);
}

void PixFeData::setCalib(std::string name, ConfMask<float> &value) {
  // Check if this calib has already been inserted in the map
  std::map<std::string, ConfMask<float> >::iterator tCalib=m_calibs.find(name);
  // If the trim doesn't exists create it
  if(tCalib == m_calibs.end()){
	  m_calibs.insert(std::make_pair(name, value));
	  m_calibNames.push_back(name);
  }
  // Update calib value
  else
	  tCalib->second = value;
}

void PixFeData::setCalib(std::string name, float maxValue, float defValue){
  // Check if this calib has already been inserted in the map
  std::map<std::string, ConfMask<float> >::iterator tCalib=m_calibs.find(name);
  // If the trim doesn't exists create it
  if(tCalib == m_calibs.end()){
    m_calibs.insert(std::make_pair(name, ConfMask<float>(m_structures->nCol(), m_structures->nRow(), maxValue, defValue)));
    m_calibNames.push_back(name);
  }
}

ConfMask<unsigned short int>& PixFeData::getTrim(std::string name, bool &ok) {
  // Check if trim really exists
  int addr, number;
  if(m_structures->getTrim(name, addr, number)) {
    std::map<std::string, ConfMask<unsigned short int> >::iterator trim=m_trims.find(name);
    if(trim != m_trims.end()) {
      // Return trim
      ok = true;
      return trim->second;
    }
  }
  ok = false;
  ConfMask<unsigned short int> *temp=0;
  return *temp;
}

ConfMask<float>& PixFeData::getCalib(std::string name, bool &ok) {
    std::map<std::string, ConfMask<float> >::iterator tCalib=m_calibs.find(name);
    if(tCalib != m_calibs.end()) {
      ok = true;
      return tCalib->second;
    }
  ok = false;
  ConfMask<float> *temp=0;
  return *temp;
}

bool PixFeData::getTrim(std::string name, int row, int col, unsigned short int &value) {
  // Check if trim really exists
  int addr, number;
  if(!m_structures->getTrim(name, addr, number)) return false;
  std::map<std::string, ConfMask<unsigned short int> >::iterator trim=m_trims.find(name);
  if(trim == m_trims.end()) return false;

  // Fill output
  value = trim->second[col][row];
  return true;
}

void PixFeData::resetTrim(std::string name) {
  // Clear any existing mask map
  std::map<std::string, ConfMask<unsigned short int> >::iterator p=m_trims.find(name);
  if(p!=m_trims.end())
    p->second = ConfMask<unsigned short int>(m_structures->nCol(), m_structures->nRow(), 0xff, 0xff);
  else 
    m_trims.insert(std::make_pair(name, ConfMask<unsigned short int>(m_structures->nCol(), m_structures->nRow(), 0xff, 0xff)));
}

PixFeData::nameIterator PixFeData::calibBegin() {
  // Return begin iterator
  return m_calibNames.begin();
}

PixFeData::nameIterator PixFeData::calibEnd() {
  // Return end iterator
  return m_calibNames.end();
}



