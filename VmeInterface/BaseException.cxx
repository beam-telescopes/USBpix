//File: baseException.cxx
// $Header: /local/reps/atlaspixeldaq/VmeInterface/BaseException.cxx,v 1.8 2006/04/18 15:48:27 sctrod Exp $

#include "BaseException.h"

//! Namespace for the common routines for SCT and PIXEL ROD software. 
using namespace SctPixelRod;

//***************************Class BaseException**************************
//
// Description:
//  This class is thrown if an error in a ROD operation is detected.
//   
//  Author(s):
//    Tom Meyer (meyer@iastate.edu) - originator

//	Constructors. Use defaults for destructor, copy, and assignment.

BaseException::BaseException( std::string descriptor) {
  m_descriptor = descriptor;
  m_numData = 0;
  m_type = BASE;
  };  

void BaseException::what(std::ostream& os) const {
  os << "BaseException: " << getDescriptor() << std::endl;
};

//! This overloaded operator lets us use cout to print Exception information
std::ostream& operator<<(std::ostream& os, const SctPixelRod::BaseException& except) {
  except.what(os);
  return os;
}
