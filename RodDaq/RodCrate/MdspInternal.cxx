// File: MdspInternal.cxx
// $Header$

//! Namespace for the common routines for SCT and PIXEL ROD software.

#include "RodModule.h"

namespace SctPixelRod {

// Constructors

// MdspInternal contains internal data for the master DSP
// Normally, users won't need to access this data
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//
  unsigned long MdspInternal::load(unsigned long mdspIntStart, VmePort* myVmePort) {
    int i;
    unsigned long nowAddr = mdspIntStart;
    myVmePort->write32(HPIA, mdspIntStart);
    m_structureSize = myVmePort->read32(HPID_AUTO);

    m_rodCfg = myVmePort->read32(HPID_AUTO);
    m_rodMode = myVmePort->read32(HPID_AUTO);

    m_rodCfgSize = myVmePort->read32(HPID_AUTO);
    m_rodModeSize = myVmePort->read32(HPID_AUTO);

    m_spStruct[0] = myVmePort->read32(HPID_AUTO);
    m_spStruct[1] = myVmePort->read32(HPID_AUTO);
    
    m_histoCtrl = myVmePort->read32(HPID_AUTO);
    m_moduleMaskData = myVmePort->read32(HPID_AUTO);
    m_maskCfg = myVmePort->read32(HPID_AUTO);

    m_histoCtrlSize = myVmePort->read32(HPID_AUTO);
    m_moduleMaskDataSize = myVmePort->read32(HPID_AUTO);
    m_maskCfgSize = myVmePort->read32(HPID_AUTO);
    
    for (i=0; i<N_PRIM_BFRS; i++) {
      m_msgList[i] = myVmePort->read32(HPID_AUTO);
    }
    nowAddr = myVmePort->read32(HPIA);
    
    return(nowAddr);
  };
} // End namespace SctPixelRod
