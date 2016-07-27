// File: MdspMemoryMap.cxx
// $Header$

//! Namespace for the common routines for SCT and PIXEL ROD software.

#include "RodModule.h"

namespace SctPixelRod {

// Constructors

// MdspMemoryMap contains the memory map data for the master DSP
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//
  unsigned long MdspMemoryMap::load(unsigned long mdspMemStart, VmePort* myVmePort) {
    int i;
    
/* Set MAXREVISION to max DSP software revision number supported in this code */
    const unsigned int MAXREVISION = 101; 
       
    unsigned long nowAddr = mdspMemStart;
    myVmePort->write32(HPIA, mdspMemStart);
    m_structureSize = myVmePort->read32(HPID_AUTO);
    m_revision = myVmePort->read32(HPID_AUTO);
    if (m_revision > MAXREVISION) throw RodException(
        "DSP program version newer than MdspMemoryMap allows, m_revision, MAXREVISION =",
         m_revision, MAXREVISION);
    m_rodRevision = myVmePort->read32(HPID_AUTO);
    if (m_revision == 100) {
      m_dspVersion = 120;
      m_fmtVersion = m_rodRevision*256 + 0x19;
      m_efbVersion = m_rodRevision*256 + 0x14;
      m_rtrVersion = m_rodRevision*256 + 0x11;
      m_rcfVersion = m_rodRevision*256 + 0x16;
    }
    else {
      m_dspVersion = myVmePort->read32(HPID_AUTO);
      m_fmtVersion = myVmePort->read32(HPID_AUTO);
      m_efbVersion = myVmePort->read32(HPID_AUTO);
      m_rtrVersion = myVmePort->read32(HPID_AUTO);
      m_rcfVersion = myVmePort->read32(HPID_AUTO);
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
    m_flash = myVmePort->read32(HPID_AUTO);
    m_flashSize = myVmePort->read32(HPID_AUTO);

    if (m_revision == 100) {
      m_rodRegisterBase = 0x00400000;
    }
    else {
      m_rodRegisterBase = myVmePort->read32(HPID_AUTO);
    }
    
    m_sdram[0] = myVmePort->read32(HPID_AUTO);
    m_sdram[1] = myVmePort->read32(HPID_AUTO);
    m_sdramSize[0] = myVmePort->read32(HPID_AUTO);
    m_sdramSize[1] = myVmePort->read32(HPID_AUTO);

    m_moduleCfgArray[0] = myVmePort->read32(HPID_AUTO);
    m_moduleCfgArray[1] = myVmePort->read32(HPID_AUTO);
    m_moduleCfgArray[2] = myVmePort->read32(HPID_AUTO);
    m_spBuff[0] = myVmePort->read32(HPID_AUTO);
    m_spBuff[1] = myVmePort->read32(HPID_AUTO);
    if (m_revision == 100) {
      m_inmemCtrlStruct = 0;
    }
    else {
      m_inmemCtrlStruct = myVmePort->read32(HPID_AUTO);
    }
    
    
    m_moduleCfgSize = myVmePort->read32(HPID_AUTO);
    m_moduleCfgArraySize = myVmePort->read32(HPID_AUTO);
    m_spBuffSize = myVmePort->read32(HPID_AUTO);
    
    m_primBuffer = myVmePort->read32(HPID_AUTO);
    m_replyBuffer = myVmePort->read32(HPID_AUTO);
    m_primBufferSize = myVmePort->read32(HPID_AUTO);
    m_replyBufferSize = myVmePort->read32(HPID_AUTO);
    
    for (i = 0; i< N_TXT_BUFFS; i++) {
      m_txtBuffer[i] = myVmePort->read32(HPID_AUTO);
    }
    
    for (i = 0; i< N_TXT_BUFFS; i++) {
      m_txtBuff[i] = myVmePort->read32(HPID_AUTO);
    }
    nowAddr = myVmePort->read32(HPIA);
    for (i = 0; i< N_TXT_BUFFS; i++) {
      myVmePort->write32(HPIA,m_txtBuffer[i]);
      m_txtBufferSize[i] = myVmePort->read32(HPID_NOAUTO);
    }
    myVmePort->write32(HPIA, nowAddr);
    
    m_primParams = myVmePort->read32(HPID_AUTO);
    m_nPrimitives = myVmePort->read32(HPID_AUTO);
    
    m_internal = myVmePort->read32(HPID_AUTO);
    nowAddr = myVmePort->read32(HPIA);

    return(nowAddr);
  }

} // End namespace SctPixelRod

std::ostream& operator<<(std::ostream& os, SctPixelRod::MdspMemoryMap& mdsp) {
  int i;
  int nw = sizeof(unsigned long)*2;
  std::_Ios_Fmtflags oldOsFlags = os.flags();
  os << "\n================ MDSP Memory Map ================" << std::endl;    
  std::dec(os);
  os << "Structure size             : " << mdsp.structureSize() << std::endl;
  os << "Revision                   : " << mdsp.revision() << std::endl;
  std::hex(os);
  os << "ROD revision               : " << mdsp.rodRevision() << std::endl;
  os << "DSP version                : " << mdsp.dspVersion() << std::endl;
  os << "FMT version                : " << mdsp.fmtVersion() << std::endl;
  os << "EFB version                : " << mdsp.efbVersion() << std::endl;
  os << "RTR version                : " << mdsp.rtrVersion() << std::endl;
  os << "RCF version                : " << mdsp.rcfVersion() << std::endl;
  os << "-------------------------------------------------\n";
  os.fill('0');
  os << "IPRAM base                 : 0x" << std::setw(nw) << mdsp.ipram() << std::endl;
  os << "IPRAM size                 : 0x" << std::setw(nw) << mdsp.ipramSize() << std::endl;
  os << "IDRAM base                 : 0x" << std::setw(nw) << mdsp.idram() << std::endl;
  os << "IDRAM size                 : 0x" << std::setw(nw) << mdsp.idramSize() << std::endl;
  os << "IDATA base                 : 0x" << std::setw(nw) << mdsp.idata() << std::endl;
  os << "IDATA size                 : 0x" << std::setw(nw) << mdsp.idataSize() << std::endl;
  os << "IDRAMSEC base              : 0x" << std::setw(nw) << mdsp.idramSec() << std::endl;
  os << "IDRAMSEC size              : 0x" << std::setw(nw) << mdsp.idramSecSize() << std::endl;
  os << "Comm. regs base            : 0x" << std::setw(nw) << mdsp.commRegs() << std::endl;
  os << "Comm. regs. size           : 0x" << std::setw(nw) << mdsp.commRegsSize() << std::endl;
  os << "Flash RAM base             : 0x" << std::setw(nw) << mdsp.flash() << std::endl;
  os << "Flash RAM size             : 0x" << std::setw(nw) << mdsp.flashSize() << std::endl;
  os << "XCODE base                 : 0x" << std::setw(nw) << mdsp.xcode() << std::endl;
  os << "XCODE size                 : 0x" << std::setw(nw) << mdsp.xcodeSize() << std::endl;
  os << "-------------------------------------------------\n";
  os << "ROD register base          : 0x" << std::setw(nw) << mdsp.rodRegisterBase() << std::endl;
  os << "SDRAM 0 base               : 0x" << std::setw(nw) << mdsp.sdram(0) << std::endl;
  os << "SDRAM 0 size               : 0x" << std::setw(nw) << mdsp.sdramSize(0) << std::endl;
  os << "SDRAM 1 base               : 0x" << std::setw(nw) << mdsp.sdram(1) << std::endl;
  os << "SDRAM 1 size               : 0x" << std::setw(nw) << mdsp.sdramSize(1) << std::endl;
  os << "Module cfg. array 0        : 0x" << std::setw(nw) << mdsp.moduleCfgArray(0) << std::endl;
  os << "Module cfg. array 1        : 0x" << std::setw(nw) << mdsp.moduleCfgArray(1) << std::endl;
  os << "Module cfg. array 2        : 0x" << std::setw(nw) << mdsp.moduleCfgArray(2) << std::endl;
  os << "Serial port buff. 0        : 0x" << std::setw(nw) << mdsp.spBuff(0) << std::endl;
  os << "Serial port buff. 1        : 0x" << std::setw(nw) << mdsp.spBuff(1) << std::endl;
  os << "Serial port buff. size     : 0x" << std::setw(nw) << mdsp.spBuffSize() << std::endl;
  os << "INMEM control struct       : 0x" << std::setw(nw) << mdsp.inmemCtrlStruct() << std::endl;
  os << "Module cfg. size           : 0x" << std::setw(nw) << mdsp.moduleCfgSize() << std::endl;
  os << "Module cfg. array size     : 0x" << std::setw(nw) << mdsp.moduleCfgArraySize() << std::endl;
  os << "PrimBuffer base            : 0x" << std::setw(nw) << mdsp.primBuffer() << std::endl;
  os << "PrimBuffer size            : 0x" << std::setw(nw) << mdsp.primBufferSize() << std::endl;
  os << "ReplyBuffer base           : 0x" << std::setw(nw) << mdsp.replyBuffer() << std::endl;
  os << "ReplyBuffer size           : 0x" << std::setw(nw) << mdsp.replyBufferSize() << std::endl;
  os << "-------------------------------------------------\n";
  for (i = 0; i< N_TXT_BUFFS; i++) {
    os << "Text buffer          " << i << "     : 0x" << std::setw(nw) << mdsp.txtBuffer(i) << std::endl;
    os << "Text buffer new      " << i << "     : 0x" << std::setw(nw) << mdsp.txtBuff(i) << std::endl;
    os << "Text buffer size     " << i << "     : 0x" << std::setw(nw) << mdsp.txtBufferSize(i) << std::endl;
  }    
  os << "-------------------------------------------------\n";
  std::hex(os);
  os << "Address of internal struct : 0x" << std::setw(nw) << mdsp.internal() << std::endl;
  os << "Primitive params           : 0x" << std::setw(nw) << mdsp.primParams() << std::endl;
  std::dec(os);
  os << "Number of primitives       : " << mdsp.nPrimitives() << std::endl;
  os << "=================================================\n" << std::endl;    
  //
  os.flags(oldOsFlags); // reset stream flags
  //
  return os;
}
