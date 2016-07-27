// File: RodStatus.cxx
// $Header$

//! Namespace for the common routines for SCT and PIXEL ROD software.

#include "RodStatus.h"
#include <iomanip>

namespace SctPixelRod {
//------------------------------RodStatus--------------------------------- 
//  This class collects the status of a given ROD
//  
//  Authors Tom Meyer (meyer@iastate.edu) - originator,
//          Fredrik Tegenfeldt (fredrik.tegenfeldt@cern.ch)
RodStatus::RodStatus() {
  clear();
}

RodStatus::RodStatus( RodModule & rod ) {
  try {
    snapShot(rod);
  }
  catch (...) {
    throw;
  }
}
  
RodStatus::RodStatus( const RodStatus & other ) {
  copy(other);
}

RodStatus::~RodStatus() {
}

RodStatus & RodStatus::operator=(const RodStatus & other) {
  copy(other);
  return *this;
}

void RodStatus::copy(const RodStatus & other) {
  m_rod = other.getRod();
  m_sane = other.isSane();
  m_revision = other.getRevision();
  m_mdspMapRev   = other.getMdspMapRev();
  m_mdspProgRev  = other.getMdspProgRev();
  m_fpgaFmtRev   = other.getFpgaFmtRev();
  m_fpgaEfbRev   = other.getFpgaEfbRev();
  m_fpgaRtrRev   = other.getFpgaRtrRev();
  m_fpgaRcfRev   = other.getFpgaRcfRev();
  m_serialNumber = other.getSerialNumber();
  m_slotNumber = other.getSlotNumber();
  m_numSlaves = other.getNumSlaves();
  m_statusReg[0] = other.getStatusReg(0);
  m_statusReg[1] = other.getStatusReg(1);
  m_statusReg[2] = other.getStatusReg(2);
  m_commandReg[0] = other.getCommandReg(0);
  m_commandReg[1] = other.getCommandReg(1);
  m_primState = other.getPrimState();
}

void RodStatus::clear() {
  m_rod = 0;
  m_sane = false;
  m_serialNumber = 0;
  m_revision = 0;
  m_mdspMapRev = 0;
  m_mdspProgRev = 0;
  m_fpgaFmtRev = 0;
  m_fpgaEfbRev = 0;
  m_fpgaRtrRev = 0;
  m_fpgaRcfRev = 0;
  m_slotNumber = 0;
  m_numSlaves = 0;
  //
  m_statusReg[0] = 0;
  m_statusReg[1] = 0;
  m_statusReg[2] = 0;
  m_commandReg[0] = 0;
  m_commandReg[1] = 0;
  //
  m_primState = RodPrimList::IDLE;
  //  m_textState = TEXT_IDLE;  
}

inline const char *RodStatus::getPrimStateTxt() const {
  return getPrimStateTxt(m_primState);
}

const char *RodStatus::getPrimStateTxt(RodPrimList::PrimState st) const {
  static const char *str[] = {"Idle","Loaded","Executing","Waiting","Paused","Unknown"};
  const char *rval;
  switch (st) {
  case RodPrimList::IDLE:
    rval = str[0];
    break;
  case RodPrimList::LOADED:
    rval = str[1];
    break;
  case RodPrimList::EXECUTING:
    rval = str[2];
    break;
  case RodPrimList::WAITING:
    rval = str[3];
    break;
  case RodPrimList::PAUSED:
    rval = str[4];
    break;
  default:
    rval = str[5];
    break;
  }
  return rval;
}


void RodStatus::snapShot(RodModule &rod) {
  try {
    m_sane = false;
    m_rod = &rod;
    m_sane = rod.isSane();
    VmePort* vmePort = rod.getVmePort();
    unsigned long fpgaStatus6 = vmePort->read32(FPGA_STATUS_REG_REL_ADDR[6]);
    unsigned long maskedCor = fpgaStatus6 & 0xff000000; // correct byte order
    m_correctByteOrder = (maskedCor == 0xad000000);
    m_slotNumber = rod.getSlot();
    m_revision   = rod.getRevision();
    m_mdspMapRev = 0;
    m_mdspProgRev = 0x00c0ffee;
    //AKDEBUG
    //    m_mdspProgRev = mdspMap->dspVersion();
    m_fpgaFmtRev = rod.fmtVersion();
    m_fpgaEfbRev = rod.efbVersion();
    m_fpgaRtrRev = rod.rtrVersion();
    m_fpgaRcfRev = rod.rcfVersion();

    //
    m_serialNumber = rod.getSerialNumber();
    m_numSlaves = rod.getNumSlaves();
    m_primState = rod.getPrimState();
  }
  catch (...) {
    //    std::cout << "Some exception occured in snapShot!" << std::endl;
    throw;
  }
  return;
}

void RodStatus::writeToXml(std::ofstream& os) const {
  if (!m_rod) {return;}        // should throw an exception
  if (!os.is_open()) {return;} // idem
  os << "<?xml version=\"1.0\"?>" << std::endl;
  os << "<!DOCTYPE RODSTATUS SYSTEM>" << std::endl;
  os << "<RODSTATUS>" << std::endl;
  os << "  <SLOT> "       << m_slotNumber    << "  </SLOT>"      << std::endl;
  os << "  <BYTEORDER> "  << (m_correctByteOrder ? 1:-1) << "  </BYTEORDER>" << std::endl;
  os << "  <SERIALNUM> "  << m_serialNumber  <<  " </SERIALNUM>" << std::endl;
  os << "  <BOARDREV> "   << m_revision      <<  " </BOARDREV>"  << std::endl;
  //  os << "  <MDSPMAPREV> " << m_mdspMapRev    <<  " </MDSPMAPREV>" << std::endl;
  os << "  <MDSPPROGREV> " << m_mdspProgRev  <<  " </MDSPPROGREV>" << std::endl;
  os << "  <FPGAFMTREV> " << m_fpgaFmtRev    <<  " </FPGAFMTREV>" << std::endl;
  os << "  <FPGAEFBREV> " << m_fpgaEfbRev    <<  " </FPGAEFBREV>" << std::endl;
  os << "  <FPGARTRREV> " << m_fpgaRtrRev    <<  " </FPGARTRREV>" << std::endl;
  os << "  <FPGARCFREV> " << m_fpgaRcfRev    <<  " </FPGARCFREV>" << std::endl;
  os << "  <NSLAVES> "    << m_numSlaves     <<  " </NSLAVES>"   << std::endl;
//   os << "  <STATREG0> "   << m_statusReg[0]  <<  " </STATREG0>"  << std::endl;
//   os << "  <STATREG1> "   << m_statusReg[1]  <<  " </STATREG1>"  << std::endl;
//   os << "  <STATREG2> "   << m_statusReg[2]  <<  " </STATREG2>"  << std::endl;
//   os << "  <CMDREG0> "    << m_commandReg[0] <<  " </CMDREG0>"   << std::endl;
//   os << "  <CMDREG1> "    << m_commandReg[1] <<  " </CMDREG1>"   << std::endl;
//   os << "  <PRIMSTAT> "   << m_primState     <<  " </PRIMSTAT>"  << std::endl;
  //  os << "  <TEXTSTAT> "   << m_textState     <<  " </TEXTSTAT>"  << std::endl;
  os << "</RODSTATUS>" << std::endl;
}

void RodStatus::writeToXml(std::string& fileName) const {
  std::ofstream fout;
  fout.open(fileName.c_str(),std::ios::out);
  writeToXml(fout);
}
// overloaded << operator
std::ostream& operator<<(std::ostream& os, RodStatus& status) {
  std::_Ios_Fmtflags oldOsFlags = os.flags();
  os << std::endl;
  os << "================== R O D  S T A T U S ====================" << std::endl;
  os << " Slot                  : " << status.getSlotNumber() << std::endl;
  os << " Base adress           : " << std::hex << std::setfill('0') << "0x" << std::setw(8)
                                    << (status.getSlotNumber()<<24) << std::endl;
  os << " Byte order OK         : " << (status.isByteOrderOK() ? "Yes":"No") << std::endl;
  os << " Serial Number         : " << std::dec << status.getSerialNumber() << std::endl;
  os << " ROD rev.              : " << std::hex << status.getRevision() << std::endl;
  os << " MDSP program rev.     : " << std::hex << status.getMdspProgRev() << std::endl;
  os << " FMT program rev.      : " << std::hex << status.getFpgaFmtRev() << std::endl;
  os << " EFB program rev.      : " << std::hex << status.getFpgaEfbRev() << std::endl;
  os << " RTR program rev.      : " << std::hex << status.getFpgaRtrRev() << std::endl;
  os << " RCF program rev.      : " << std::hex << status.getFpgaRcfRev() << std::endl;
  os << " Number of slave DSPs  : " << status.getNumSlaves() << std::endl;
  os << std::endl;
//   os << " Status registers[0-2] : " << std::hex << std::setfill('0')
//      << "0x" << std::setw(8) << status.getStatusReg(0) << " "
//      << "0x" << std::setw(8) << status.getStatusReg(1) << " "
//      << "0x" << std::setw(8) << status.getStatusReg(2) << std::endl;
//   os << " Command registers[0-1]: " << std::hex << std::setfill('0')
//      << "0x" << std::setw(8) << status.getCommandReg(0) << " "
//      << "0x" << std::setw(8) << status.getCommandReg(1) << std::endl;
  dec(os);
  os << " Primitive state       : " << status.getPrimStateTxt() << std::endl;
  os << "==========================================================" << std::endl;
  os << std::endl;
  os.flags(oldOsFlags); // reset stream flags
  return os;
}

} // End namespace SctPixelRod
