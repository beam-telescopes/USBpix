// File: MdspInternal.h

#ifndef SCTPIXELROD_MDSPINTERNAL_H
#define SCTPIXELROD_MDSPINTERNAL_H

namespace SctPixelRod {

/*! 
 * @class MdspInternal  
 *
 * @brief This is a class that holds internal parameters for the master DSP.
 * In the normal course of things, users should not need to access this data.
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

class MdspInternal
{
public:

    MdspInternal() {};                                  // Constructor
    MdspInternal( const MdspInternal& );                // Copy constructor
    ~MdspInternal() {};                                 // Destructor
    MdspInternal& operator=( const MdspInternal&);      //Overload = operator

// Accessor functions 
    unsigned long structureSize() const { return m_structureSize; };
                              
// Methods
    unsigned long load(unsigned long mdspIntStart, VmePort *myVmePort); 
              
private:
    unsigned long m_structureSize;                    //!< size of the MdspMemoryStruct
    
    unsigned long m_rodCfg;                           //!< ROD configuration
    unsigned long m_rodMode;                          //!< ROD mode

    unsigned long m_rodCfgSize;                       //!< ROD configuration size
    unsigned long m_rodModeSize;                      //!< ROD mode size
 
    unsigned long m_spStruct[2];                      //!< serial port structs

    unsigned long m_histoCtrl;                        //!< histogram control base
    unsigned long m_moduleMaskData;                   //!< module mask data base
    unsigned long m_maskCfg;                          //!< mask configuration base

    unsigned long m_histoCtrlSize;                    //!< histogram control size
    unsigned long m_moduleMaskDataSize;               //!< module mask data size
    unsigned long m_maskCfgSize;                      //!< mask configuration size
    
    unsigned long m_msgList[N_PRIM_BFRS];             //!< Message list base
    
  };                                   // End of MdspMemoryMap declaration
}; // End namespace SctPixelRod

#endif  // SCTPIXELROD_MDSPMEMORYMAP_H
