//File: VmeModule.cxx

#include "VmeModule.h"

//! Namespace for the common routines for SCT and PIXEL ROD software. 
namespace SctPixelRod {

//	Copy Constructor

  VmeModule::VmeModule( const VmeModule & rhs) : 
                       m_ourInterface(rhs.getVmeInterface()) { 
    m_baseAddress = rhs.getBaseAddress();
    m_mapSize = rhs.getMapSize();
  }

// Assignment operator
  VmeModule & VmeModule::operator=( const VmeModule & rhs) {
    if (this == &rhs) return *this;
    m_baseAddress = rhs.getBaseAddress();
    m_mapSize = rhs.getMapSize();
    m_ourInterface = rhs.getVmeInterface();
    return *this;
  }

} // End namespace SctPixelRod
