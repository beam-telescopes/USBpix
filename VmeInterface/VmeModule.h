//File: VmeModule.h

#ifndef SCTPIXELROD_VMEMODULE_H 
#define SCTPIXELROD_VMEMODULE_H

//! VmeModule A pure virtual base class for VME modules. 
/*!

  This is the implementation of a VME base class for polymorphic classes of VME 
  modules.

  Contributors:	Tom Meyer (meyer@iastate.edu) - originator
*/

//typedef unsigned int   UINT32;
// clashes with definition in DSP
#include "VmeInterface.h"

namespace SctPixelRod {

class VmeModule {
public:
  // Constructor
  typedef unsigned int   UINT32;

  VmeModule( UINT32 baseAddress, UINT32 mapSize, VmeInterface & ourInterface) :
           m_ourInterface(ourInterface) { 
    m_baseAddress=baseAddress;
    m_mapSize=mapSize;
  }

  // Destructor
  virtual ~VmeModule() {};

  // Copy constructor
  VmeModule( const VmeModule & rhs);

  // Assignment operator
  VmeModule & operator=(const VmeModule &);

  //! Accessor function to set base address
  void setBaseAddress( UINT32 baseAddress) {
    m_baseAddress = baseAddress;
  };

  //! Accessor function to get base address
  UINT32 getBaseAddress() const { return m_baseAddress; };

  //! Accessor function to set interface
  void setVmeInterface(VmeInterface & ourInterface) { m_ourInterface = ourInterface; };

  //! Accessor function to get interface
  VmeInterface & getVmeInterface() const { return m_ourInterface; };

  //! Accessor function to set map size
  void setMapSize(UINT32 mapSize) { m_mapSize = mapSize;};
  
  //! Accessor function to get map size
  UINT32 getMapSize() const {return m_mapSize;};
  
  //! Pure virtual method for initialization
  virtual void initialize() = 0;
  
  //! Pure virtual method to reset module
  virtual void reset() = 0;

  //! Pure virtual method to report module status
  virtual void status() = 0;

  protected:
  
  //! VME base address of this module. 
  UINT32 m_baseAddress;
  
  //! VME map size
  UINT32 m_mapSize;
  
  //! VME interface for this module. 
  VmeInterface & m_ourInterface;

};       // End of VmeModule declaration

}; //  End namespace SctPixelRod
#endif // SCTPIXELROD_VMEMODULE_H
