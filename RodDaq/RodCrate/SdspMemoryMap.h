// File: SdspMemoryMap.h

#ifndef SCTPIXELROD_SDSPMEMORYMAP_H
#define SCTPIXELROD_SDSPMEMORYMAP_H

#include <iostream>
#include <iomanip>
namespace SctPixelRod {

/*! 
 * @class SdspMemoryMap  
 *
 * @brief This is a class that describes the memory map for the slave DSPs.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

class SdspMemoryMap
{
public:

    SdspMemoryMap(){};                                   // Constructor
    SdspMemoryMap( const SdspMemoryMap& );               // Copy constructor
    ~SdspMemoryMap(){};                                  // Destructor
    SdspMemoryMap& operator=( const SdspMemoryMap&);     //Overload = operator

// Accessor functions 
    unsigned long structureSize() const { return m_structureSize; };
    unsigned long revision() const { return m_revision; };
    unsigned long dspVersion() const { return m_dspVersion; };
    unsigned long unused() const { return m_unused; };
    
    unsigned long ipram() const { return m_ipram; };
    unsigned long idram() const { return m_idram; };
    unsigned long commRegs() const { return m_commRegs; };
    unsigned long idata() const { return m_idata; };
    unsigned long idramSec() const { return m_idramSec; };
    unsigned long xcode() const { return m_xcode; };
    
    unsigned long ipramSize() const { return m_ipramSize; };
    unsigned long idramSize() const { return m_idramSize; };
    unsigned long commRegsSize() const { return m_commRegsSize; };
    unsigned long idataSize() const { return m_idataSize; };
    unsigned long idramSecSize() const { return m_idramSecSize; };
    unsigned long xcodeSize() const { return m_xcodeSize; };
    
    unsigned long sdram(long index) const { return m_sdram[index]; };
    unsigned long sdramSize(long index) const { return m_sdramSize[index]; };
    
    unsigned long burst() const { return m_burst; };
    unsigned long burstSize() const { return m_burstSize; };
    
    unsigned long primBuffer() const { return m_primBuffer; };
    unsigned long replyBuffer() const { return m_replyBuffer; };
    unsigned long primBufferSize() const { return m_primBufferSize; };
    unsigned long replyBufferSize() const { return m_replyBufferSize; };

    unsigned long txtBufferSize(long buffno) const {return m_txtBufferSize[buffno]; };
    unsigned long txtBuffer(long buffno) { return m_txtBuffer[buffno]; };
    
    unsigned long txtBuff(long buffno) { return m_txtBuff[buffno]; };
    
    unsigned long primParams() {return m_primParams; };
    unsigned long nPrimitives() {return m_nPrimitives; };

    unsigned long internal() {return m_internal; };
                              
// Methods
    unsigned long load(unsigned long sdspStart, VmePort *myVmePort); 
              
private:
    unsigned long m_structureSize;                    //!< size of the MdspMemoryStruct
    unsigned long m_revision;                         //!< revision number 
    unsigned long m_dspVersion;                       //!< DSP version number 
    unsigned long m_unused;                           //!< future development 
    
    unsigned long m_ipram;                            //!< ipram base 
    unsigned long m_idram;                            //!< idram base 
    unsigned long m_commRegs;                         //!< communication registers base 
    unsigned long m_idata;                            //!< idata base 
    unsigned long m_idramSec;                         //!< idramSec base 
    unsigned long m_xcode;                            //!< xcode base 
    
    unsigned long m_ipramSize;                        //!< ipram size 
    unsigned long m_idramSize;                        //!< idram size 
    unsigned long m_commRegsSize;                     //!< commRegs size 
    unsigned long m_idataSize;                        //!< idata size 
    unsigned long m_idramSecSize;                     //!< idramSec size 
    unsigned long m_xcodeSize;                        //!< xcode size 

    unsigned long m_sdram[2];                         //!< SDRAM bases
    unsigned long m_sdramSize[2];                     //!< SDRAM sizes
    
    unsigned long m_burst;                            //!< trapped events buffer base
    unsigned long m_burstSize;                        //!< trapped evts bfr size(dynamic)
    
    unsigned long m_primBuffer;                       //!< PrimBuffer base
    unsigned long m_replyBuffer;                      //!< ReplyBuffer base
    unsigned long m_primBufferSize;                   //!< PrimBuffer size
    unsigned long m_replyBufferSize;                  //!< ReplyBuffer size

    unsigned long m_txtBufferSize[N_SDSP_TXT_BUFFS];  //!< Text buffer size
    unsigned long m_txtBuffer[N_SDSP_TXT_BUFFS];      //!< Text buffer bases    
    unsigned long m_txtBuff[N_SDSP_TXT_BUFFS];        //!< New text buffer bases    

    unsigned long m_primParams;                       //!< primitive parameters
    unsigned long m_nPrimitives;                      //!< Number of primitives

    unsigned long m_internal;                         //!< DSP addr of internal data 
       
  };                                   // End of SdspMemoryMap declaration
}; // End namespace SctPixelRod

std::ostream& operator<<(std::ostream& os, SctPixelRod::SdspMemoryMap& mdsp);

#endif  // SCTPIXELROD_SDSPMEMORYMAP_H
