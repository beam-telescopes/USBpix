// File: SdspInternal.h

#ifndef SCTPIXELROD_SDSPINTERNAL_H
#define SCTPIXELROD_SDSPINTERNAL_H

namespace SctPixelRod {

/*! 
 * @class SdspInternal  
 *
 * @brief This is a class that describes the internal data for the slave DSPs.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

class SdspInternal
{
public:

    SdspInternal(){};                                   // Constructor
    SdspInternal( const SdspInternal& );                // Copy constructor
    ~SdspInternal(){};                                  // Destructor
    SdspInternal& operator=( const SdspInternal&);      //Overload = operator

// Accessor functions 
    unsigned long structureSize() const { return m_structureSize; };
    
    unsigned long rodCfg() const { return m_rodCfg; };
    unsigned long rodMode() const { return m_rodMode; };
    
    unsigned long rodCfgSize() const { return m_rodCfgSize; };
    unsigned long rodModeSize() const { return m_rodModeSize; };
                             
    unsigned long histoCtrl() const { return m_histoCtrl; };
    unsigned long evtMgrCtrl() const { return m_evtMgrCtrl; };
    unsigned long eventQueue() const { return m_eventQueue; };
    unsigned long evtMgrRes() const { return m_evtMgrRes; };
                             
    unsigned long histoCtrlSize() const { return m_histoCtrlSize; };
    unsigned long evtMgrCtrlSize() const { return m_evtMgrCtrlSize; };
    unsigned long eventQueueSize() const { return m_eventQueueSize; };
    unsigned long evtMgrResSize() const { return m_evtMgrResSize; };

    unsigned long histoDefaultBase() const { return m_histoDefaultBase; };
 
    unsigned long msgList(long buffNo, long i) const {return m_msgList[buffNo][i]; };
                                
// Methods
    unsigned long load(unsigned long sdspIntStart, VmePort *myVmePort); 
              
private:
    unsigned long m_structureSize;                    //!< size of the struct
    
    unsigned long m_rodCfg;                           //!< ROD configuration
    unsigned long m_rodMode;                          //!< ROD mode

    unsigned long m_rodCfgSize;                       //!< ROD configuration size
    unsigned long m_rodModeSize;                      //!< ROD mode size
 
    unsigned long m_histoCtrl;                        //!< histogram control base
    unsigned long m_evtMgrCtrl;                       //!< event manager control base
    unsigned long m_eventQueue;                       //!< event queue base
    unsigned long m_evtMgrRes;                        //!< mask configuration base

    unsigned long m_histoCtrlSize;                    //!< histogram control size
    unsigned long m_evtMgrCtrlSize;                   //!< event manager control size
    unsigned long m_eventQueueSize;                   //!< event queue size
    unsigned long m_evtMgrResSize;                    //!< mask configuration size
    
    unsigned long m_histoDefaultBase;                 //!< histogram default base

    unsigned long m_msgList[N_PRIM_BFRS][2];          //!< Message list base
    
  };                                   // End of SdspInternal declaration
}; // End namespace SctPixelRod

#endif  // SCTPIXELROD_SDSPMEMORYMAP_H
