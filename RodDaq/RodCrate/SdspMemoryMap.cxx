// File: SdspMemoryMap.cxx
// $Header$

//! Namespace for the common routines for SCT and PIXEL ROD software.

#include "RodModule.h"

namespace SctPixelRod {

// Constructors

// MdspMemoryMap contains the memory map data for the slave DSPs
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//
  unsigned long SdspMemoryMap::load(unsigned long sdspMemStart, VmePort* myVmePort) {
    int i;
    unsigned long nowAddr = sdspMemStart;
    myVmePort->write32(HPIA, sdspMemStart);
    m_structureSize = myVmePort->read32(HPID_AUTO);
    m_revision = myVmePort->read32(HPID_AUTO);
    if (m_revision > 100) {
      m_dspVersion = myVmePort->read32(HPID_AUTO);
      m_unused = myVmePort->read32(HPID_AUTO);
    }
    else {
      m_dspVersion = 120;
      m_unused = 0;
    }
    
    m_ipram = myVmePort->read32(HPID_AUTO);
    m_idram = myVmePort->read32(HPID_AUTO);
    m_commRegs = myVmePort->read32(HPID_AUTO);
    m_idata = myVmePort->read32(HPID_AUTO);
    m_idramSec = myVmePort->read32(HPID_AUTO);
    m_xcode = myVmePort->read32(HPID_AUTO);

    m_ipramSize = myVmePort->read32(HPID_AUTO);
    m_idramSize = myVmePort->read32(HPID_AUTO);
    m_commRegsSize = myVmePort->read32(HPID_AUTO);
    m_idataSize = myVmePort->read32(HPID_AUTO);
    m_idramSecSize = myVmePort->read32(HPID_AUTO);
    m_xcodeSize = myVmePort->read32(HPID_AUTO);

    m_sdram[0] = myVmePort->read32(HPID_AUTO);
    m_sdram[1] = myVmePort->read32(HPID_AUTO);
    m_sdramSize[0] = myVmePort->read32(HPID_AUTO);
    m_sdramSize[1] = myVmePort->read32(HPID_AUTO);

    m_burst = myVmePort->read32(HPID_AUTO);
    m_burstSize = myVmePort->read32(HPID_AUTO);

    m_primBuffer = myVmePort->read32(HPID_AUTO);
    m_replyBuffer = myVmePort->read32(HPID_AUTO);
    m_primBufferSize = myVmePort->read32(HPID_AUTO);
    m_replyBufferSize = myVmePort->read32(HPID_AUTO);
    
    for (i = 0; i< N_SDSP_TXT_BUFFS; i++) {
      m_txtBuffer[i] = myVmePort->read32(HPID_AUTO);
      nowAddr = myVmePort->read32(HPIA);
      myVmePort->write32(HPIA,m_txtBuffer[i]);
      m_txtBufferSize[i] = myVmePort->read32(HPID_AUTO);
      myVmePort->write32(HPIA, nowAddr);
    }

    for (i = 0; i< N_SDSP_TXT_BUFFS; i++) {
      m_txtBuff[i] = myVmePort->read32(HPID_AUTO);
    }
    
    m_primParams = myVmePort->read32(HPID_AUTO);
    m_nPrimitives = myVmePort->read32(HPID_AUTO);
    
    m_internal = myVmePort->read32(HPID_AUTO);
    nowAddr = myVmePort->read32(HPIA);
    
    return(nowAddr);
  };
} // End namespace SctPixelRod

std::ostream& operator<<(std::ostream& os, SctPixelRod::SdspMemoryMap& sdsp) {
  int i;
  int nw = sizeof(unsigned long)*2;
  std::_Ios_Fmtflags oldOsFlags = os.flags();
  os << "\n================ SDSP Memory Map ================" << std::endl;    
  std::dec(os);
  os << "Structure size             : " << sdsp.structureSize() << std::endl;
  os << "Revision                   : " << sdsp.revision() << std::endl;
  std::hex(os);
  os << "DSP version                : " << sdsp.dspVersion() << std::endl;
  os << "Unused                     : " << sdsp.unused() << std::endl;    
  os << "-------------------------------------------------\n";
  os.fill('0');
  os << "IPRAM base                 : 0x" << std::setw(nw) << sdsp.ipram() << std::endl;
  os << "IPRAM size                 : 0x" << std::setw(nw) << sdsp.ipramSize() << std::endl;
  os << "IDRAM base                 : 0x" << std::setw(nw) << sdsp.idram() << std::endl;
  os << "IDRAM size                 : 0x" << std::setw(nw) << sdsp.idramSize() << std::endl;
  os << "IDATA base                 : 0x" << std::setw(nw) << sdsp.idata() << std::endl;
  os << "IDATA size                 : 0x" << std::setw(nw) << sdsp.idataSize() << std::endl;
  os << "IDRAMSEC base              : 0x" << std::setw(nw) << sdsp.idramSec() << std::endl;
  os << "IDRAMSEC size              : 0x" << std::setw(nw) << sdsp.idramSecSize() << std::endl;
  os << "Comm. regs base            : 0x" << std::setw(nw) << sdsp.commRegs() << std::endl;
  os << "Comm. regs. size           : 0x" << std::setw(nw) << sdsp.commRegsSize() << std::endl;
  os << "-------------------------------------------------\n";
  os << "SDRAM 0 base               : 0x" << std::setw(nw) << sdsp.sdram(0) << std::endl;
  os << "SDRAM 0 size               : 0x" << std::setw(nw) << sdsp.sdramSize(0) << std::endl;
  os << "SDRAM 1 base               : 0x" << std::setw(nw) << sdsp.sdram(1) << std::endl;
  os << "SDRAM 1 size               : 0x" << std::setw(nw) << sdsp.sdramSize(1) << std::endl;
  os << "-------------------------------------------------\n";
  os << "Burst memory base          : 0x" << std::setw(nw) << sdsp.burst() << std::endl;
  os << "Burst memory size          : 0x" << std::setw(nw) << sdsp.burstSize() << std::endl;
  os << "Primitive buffer base      : 0x" << std::setw(nw) << sdsp.primBuffer() << std::endl;
  os << "Primitive buffer size      : 0x" << std::setw(nw) << sdsp.primBufferSize() << std::endl;
  os << "Reply buffer base          : 0x" << std::setw(nw) << sdsp.replyBuffer() << std::endl;
  os << "Reply buffer size          : 0x" << std::setw(nw) << sdsp.replyBufferSize() << std::endl;
  os << "-------------------------------------------------\n";
  for (i = 0; i< N_SDSP_TXT_BUFFS; i++) {
    os << "Text buffer          " << i << "     : 0x" << std::setw(nw) << sdsp.txtBuffer(i) << std::endl;
    os << "Text buffer new      " << i << "     : 0x" << std::setw(nw) << sdsp.txtBuff(i) << std::endl;
    os << "Text buffer size     " << i << "     : 0x" << std::setw(nw) << sdsp.txtBufferSize(i) << std::endl;
  }
  os << "-------------------------------------------------\n";
  std::hex(os);
  os << "Address of internal struct : 0x" << std::setw(nw) << sdsp.internal() << std::endl;
  os << "Primitive params           : 0x" << std::setw(nw) << sdsp.primParams() << std::endl;
  std::dec(os);
  os << "Number of primitives       : " << sdsp.nPrimitives() << std::endl;
  os << "=================================================\n" << std::endl;
  //
  os.flags(oldOsFlags); // reset stream flags
  //
  return os;
}
