// File: MdspMemoryMap.h

#ifndef SCTPIXELROD_MDSPMEMORYMAP_H
#define SCTPIXELROD_MDSPMEMORYMAP_H

#include <iostream>
#include <iomanip>
namespace SctPixelRod {
  /*! 
   * @class MdspMemoryMap  
   *
   * @brief This is a class that describes the memory map for the master DSP
   * @author Tom Meyer (meyer@iastate.edu) - originator
   */

  class MdspMemoryMap
  {
  public:

    MdspMemoryMap() {};                                  // Constructor
    MdspMemoryMap( const MdspMemoryMap& );               // Copy constructor
    ~MdspMemoryMap() {};                                 // Destructor
    MdspMemoryMap& operator=( const MdspMemoryMap&);     //Overload = operator
    // Accessor functions 
    unsigned long structureSize() const { return m_structureSize; };
    unsigned long revision() const { return m_revision; };
    unsigned long rodRevision() const { return m_rodRevision; };
    unsigned long dspVersion() const { return m_dspVersion; };
    unsigned long fmtVersion() const { return m_fmtVersion; };
    unsigned long efbVersion() const { return m_efbVersion; };
    unsigned long rtrVersion() const { return m_rtrVersion; };
    unsigned long rcfVersion() const { return m_rcfVersion; };
  
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

    unsigned long flash() const { return m_flash; };
    unsigned long flashBase() const {return m_flash; }; // An alias for flash()
    unsigned long flashSize() const { return m_flashSize; };
    unsigned long rodRegisterBase() const { return m_rodRegisterBase; };
    
    unsigned long sdram(long ramNum) { return m_sdram[ramNum]; };
    unsigned long sdramSize(long ramNum) { return m_sdramSize[ramNum]; };

    unsigned long moduleCfgArray(long index) const { return m_moduleCfgArray[index]; };
    unsigned long spBuff(long index) const { return m_spBuff[index]; };
    unsigned long inmemCtrlStruct() const {return m_inmemCtrlStruct; };
    unsigned long moduleCfgSize() const { return m_moduleCfgSize; };
    unsigned long moduleCfgArraySize() const { return m_moduleCfgArraySize; };
    unsigned long spBuffSize() const { return m_spBuffSize; };

    unsigned long primBuffer() const { return m_primBuffer; };
    unsigned long replyBuffer() const { return m_replyBuffer; };
    unsigned long primBufferSize() const { return m_primBufferSize; };
    unsigned long replyBufferSize() const { return m_replyBufferSize; };
    
    unsigned long txtBufferSize(long buffNo) {return m_txtBufferSize[buffNo]; };
    unsigned long txtBuffer(long buffno) { return m_txtBuffer[buffno]; };
    
    unsigned long txtBuff(long buffno) { return m_txtBuff[buffno]; };
    
    unsigned long primParams() {return m_primParams; };
    unsigned long nPrimitives() {return m_nPrimitives; };

    unsigned long internal() {return m_internal; };
                              
    // Methods
    unsigned long load(unsigned long mdspMemStart, VmePort *myVmePort);
              
  private:
    unsigned long m_structureSize;                    //!< size of the MdspMemoryStruct
    unsigned long m_revision;                         //!< structure revision number 
    unsigned long m_rodRevision;                      //!< ROD revision number 
    unsigned long m_dspVersion;                       //!< MDSP program version
    unsigned long m_fmtVersion;                       //!< FMT program version
    unsigned long m_efbVersion;                       //!< EFB program version
    unsigned long m_rtrVersion;                       //!< RTR program version
    unsigned long m_rcfVersion;                       //!< RFC program version
    
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

    unsigned long m_flash;                            //!< flash RAM base 
    unsigned long m_flashSize;                        //!< flash RAM size 
    unsigned long m_rodRegisterBase;                  //!< register base 

    unsigned long m_sdram[2];                         //!< SDRAM bases
    unsigned long m_sdramSize[2];                     //!< SDRAM sizes
    
    unsigned long m_moduleCfgArray[3];                //!< module configuration arrays
    unsigned long m_spBuff[2];                        //!< serial port buffers
    unsigned long m_inmemCtrlStruct;                  //!< inmem Control structure

    unsigned long m_moduleCfgSize;                    //!< module config size
    unsigned long m_moduleCfgArraySize;               //!< module config array size
    unsigned long m_spBuffSize;                       //!< mask configuration base

    unsigned long m_primBuffer;                       //!< PrimBuffer base
    unsigned long m_replyBuffer;                      //!< ReplyBuffer base
    unsigned long m_primBufferSize;                   //!< PrimBuffer size
    unsigned long m_replyBufferSize;                  //!< ReplyBuffer size
    unsigned long m_txtBufferSize[N_TXT_BUFFS];       //!< Text buffer size
    unsigned long m_txtBuffer[N_TXT_BUFFS];           //!< Text buffer bases    
    unsigned long m_txtBuff[N_TXT_BUFFS];             //!< New text buffers

    unsigned long m_primParams;                       //!< primitive parameters
    unsigned long m_nPrimitives;                      //!< Number of primitives
    
    unsigned long m_internal;                         //!< MDSP Address of MdspInternal struct
    
  };                                   // End of MdspMemoryMap declaration

}; // End namespace SctPixelRod

std::ostream& operator<<(std::ostream& os, SctPixelRod::MdspMemoryMap& mdsp);

#endif  // SCTPIXELROD_MDSPMEMORYMAP_H
