// File: SdspInternal.cxx
// $Header$

//! Namespace for the common routines for SCT and PIXEL ROD software.

#include "RodModule.h"

namespace SctPixelRod {

// Constructors

// MdspMemoryMap contains the internal data for the slave DSPs
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//
  unsigned long SdspInternal::load(unsigned long sdspIntStart, VmePort* myVmePort) {
    int i;
    unsigned long nowAddr = sdspIntStart;
    myVmePort->write32(HPIA, sdspIntStart);
    m_structureSize = myVmePort->read32(HPID_AUTO);
   
    m_rodCfg = myVmePort->read32(HPID_AUTO);
    m_rodMode = myVmePort->read32(HPID_AUTO);

    m_rodCfgSize = myVmePort->read32(HPID_AUTO);
    m_rodModeSize = myVmePort->read32(HPID_AUTO);

    m_histoCtrl = myVmePort->read32(HPID_AUTO);
    m_evtMgrCtrl = myVmePort->read32(HPID_AUTO);
    m_eventQueue = myVmePort->read32(HPID_AUTO);
    m_evtMgrRes = myVmePort->read32(HPID_AUTO);

    m_histoCtrlSize = myVmePort->read32(HPID_AUTO);
    m_evtMgrCtrlSize = myVmePort->read32(HPID_AUTO);
    m_eventQueueSize = myVmePort->read32(HPID_AUTO);
    m_evtMgrResSize = myVmePort->read32(HPID_AUTO);
    
    m_histoDefaultBase = myVmePort->read32(HPID_AUTO);

    for (i = 0; i< N_PRIM_BFRS; i++) {
      m_msgList[i][0] = myVmePort->read32(HPID_AUTO);
      m_msgList[i][1]= myVmePort->read32(HPID_AUTO);
    }
    nowAddr = myVmePort->read32(HPIA);
    
    return(nowAddr);
  };
} // End namespace SctPixelRod
