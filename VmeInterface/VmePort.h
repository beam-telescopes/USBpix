/////////////////////////////////////////////////////////////////////
//! VmePort.h 
//! version 1.4
//!
//! VmePort class 
//!                                 
/////////////////////////////////////////////////////////////////////
// 25/03/2002  V 1.0 TM - Data types unsigned, addr_mod->AddrMod, 
//                        namespace added            
//  3/04/2002  V 1.1 PM - Few modifications in the block transfer
//                        methods 
// 18/04/2002  V 1.2 PM - Modification in the interrupt handling
//                        and in the error reporting methods
// 22/04/2002  V 1.3 PM - New methods for blocking and resuming 
//                        interrupt notification and for reading
//                        and setting an interrupt data word           
// 19/02/2004  V 1.4 PM - Safe single word R/W methods added            
//

/*!

  VmePort is a class handling the interface of a generic
  VME medule. It's intended for multiple instantiations
  so every VME card can have its own instance of a VmePort.
  VmePort uses a VmeInterface to access to the VME bus.
  
  Three types of VME access are supported
   - Single word read/write with automatic mapping
   - Block transfers with automatic mapping
   - Manual mapping delivering a pointer to the user for direct
     memory access

  Errors are signaled via try/catch mechanism; the
  exceptions can be disabled.
  Bus errors are signaled via C++ exceptions if they occur
  inside a class method. When using VME access via memory
  pointer, the user is responsible of checking for bus errors
  using getBusErrors() or busErrorsReport() (the first returns the
  number of bus erros since last call, the second trow an exceprion
  if the bus error counter id non-zero).

  VmePort allows the user to declare a VME interrupt handler.
  The interrupt handler is an object of a class derived from
  VmeInterruptHandler. The interruptHandler() method of this class
  will be called during the interrupt handling. There are two 
  types of handlers: hard (HIH) and soft (SIH). The HIH have
  soft-vector=0 and are linked to a physical VME interrupt with
  a given vector. During the execution of interruptHandler(),
  HIH can call activateSoftVector() enabling one or more soft
  vector. SIH, declared with soft-vector != 0) are called during
  the handling of a VME interrupt if one of the HIH linked to
  the interrupt vector has activated the soft-vector. SIH are
  useful to link different components to a single VME interrupt:
  a HIH will check the hardware, detect the source of the interrupt
  and activate the corresponding SIH.

*/

#ifndef SCTPIXELROD_VMEPORT_H
#define SCTPIXELROD_VMEPORT_H

#include <string>

#include "VmeInterface.h"

namespace SctPixelRod {

class VmePort {
  friend class RCCVmeInterface;

public:
  //! Constructor
  VmePort(unsigned long base, unsigned long size, VmeInterface::AddrMod mod,
          VmeInterface &interf) : 
          m_baseAddress(base), m_mapSize(size), m_addrMod(mod),
          m_interface(interf),
          m_nExcl(0) {
     m_interface.registerPort(*this);
     m_exceptionTrapping = true;
   };
  VmePort(unsigned long base, unsigned long size, 
          unsigned long nexcl, unsigned long *excls, unsigned long *excle,
          VmeInterface::AddrMod mod,
          VmeInterface &interf) : 
          m_baseAddress(base), m_mapSize(size), m_addrMod(mod),
          m_interface(interf) {
     if (nexcl > 4) nexcl = 4;
     for (unsigned long i=0; i<nexcl; i++) {
       m_exclStart[i] = excls[i];
       m_exclEnd[i] = excle[i];
       if (excls[i]>size) nexcl = 0;
       if (excle[i]>size) nexcl = 0;
       if (excle[i]-excls[i]<0x1000) nexcl = 0;
       if (i>0 && excls[i]<= excle[i-1]) nexcl = 0;
     }
     m_nExcl = nexcl;
     m_exclStart[m_nExcl] = size;
     m_interface.registerPort(*this);
     m_exceptionTrapping = true;
   };
  //! Destructor  
  ~VmePort() {
    m_interface.deletePort(*this);
  };                                                                  

  //! Get the pointer to VmeInterface
  inline VmeInterface &getInterface() { return m_interface; };         
  //! Get the handle
  inline unsigned long getHandle() { return m_handle[0]; };
  inline unsigned long getHandle(unsigned long &addr) { 
    for (unsigned long i=0; i<m_nExcl+1; i++) {
      if (addr < m_exclStart[i]) {
	if (i>0) addr -= (m_exclEnd[i-1]+1);
        return m_handle[i];
      }
    }
    return m_handle[0];
  };         

  //! Get the address of the map in VMEM
  //inline void *getMap() { return m_interface.getPortMap(m_handle[0]); };         
  //inline void *getMap(unsigned long i) { 
  //  if (i<4) {
  //    return m_interface.getPortMap(m_handle[i]); 
  //  } else {
  //    return m_interface.getPortMap(m_handle[0]); 
  //  }
  //};         
  //! Get the base address
  inline unsigned long getBaseAddress() { return m_baseAddress; };         
  //! Get the map size
  inline unsigned long getMapSize() { return m_mapSize; };
  //! Get the address modifier
  inline VmeInterface::AddrMod getAddrMod() { return m_addrMod; };
  //! Enable or disable exception trapping
  inline void setExceptionTrapping(bool act) { m_exceptionTrapping = act; };         
  //! Returns current exception trapping
  inline bool getExceptionTrapping() { return m_exceptionTrapping; };                
  //! Return the last error code
  inline long getLastErrcode() { return m_interface.getLastErrcode(); };
  //! Print the message corresponding to an error code
  inline std::string getErrorMessage(const long errcode) { return m_interface.getErrorMessage(errcode); };

  //! Return the number of bus errors
  inline long getBusErrors() { return m_interface.getBusErrors(); };
  //! Throw exceptions in case of bus errors
  void busErrorReport() { m_interface.busErrorReport(*this); };

  //! Byte read (automatic mapping)
  inline unsigned char read8(unsigned long offset) { 
    int h = getHandle(offset);
    return m_interface.read8(h, offset);
  };  
  inline unsigned char readS8(unsigned long offset) { 
    int h = getHandle(offset);
    return m_interface.readS8(h, offset);
  };  
  
  // Word read (automatic mapping)
  inline unsigned short read16(unsigned long offset){ 
    int h = getHandle(offset);
    return m_interface.read16(h, offset);
  };  
  inline unsigned short readS16(unsigned long offset){ 
    int h = getHandle(offset);
    return m_interface.readS16(h, offset);
  };  
  
  //! Long Word read (automatic mapping)
  inline unsigned long read32(unsigned long offset){
    int h = getHandle(offset);
    return m_interface.read32(h, offset);
  };  
  inline unsigned long readS32(unsigned long offset){
    int h = getHandle(offset);
    return m_interface.readS32(h, offset);
  };  
  
  //! Byte write (automatic mapping)
  inline void write8(unsigned long offset, const unsigned char value) {
    int h = getHandle(offset);
    m_interface.write8(h, offset, value);
  };   
  inline void writeS8(unsigned long offset, const unsigned char value) {
    int h = getHandle(offset);
    m_interface.writeS8(h, offset, value);
  };   
  
  //! Word write (automatic mapping)
  inline void write16(unsigned long offset, const unsigned short value) {
    int h = getHandle(offset);
    m_interface.write16(h, offset, value);
  };    
  inline void writeS16(unsigned long offset, const unsigned short value) {
    int h = getHandle(offset);
    m_interface.writeS16(h, offset, value);
  };    
  
  //! Long word write (automatic mapping)
  inline void write32(unsigned long offset, const unsigned long value) {
    int h = getHandle(offset);
    m_interface.write32(h, offset, value);
  };   
  inline void writeS32(unsigned long offset, const unsigned long value) {
    int h = getHandle(offset);
    m_interface.writeS32(h, offset, value);
  };   

  //! 32 bits block read (automatic mapping)
  inline void blockRead32(const unsigned long offset, unsigned long *buf, const long len) {
    m_interface.blockRead32(*this, offset, buf, len);
  };  
  //! 64 bits block read (automatic mapping)
  inline void blockRead64(const unsigned long offset, unsigned long *buf, const long len) {
    m_interface.blockRead64(*this, offset, buf, len);
  };   
  //! 32 bits block write (automatic mapping)
  inline void blockWrite32(const unsigned long offset, const unsigned long *buf, const long len) {
    m_interface.blockWrite32(*this, offset, buf, len);
  };  
  //! 64 bits block write (automatic mapping)
  inline void blockWrite64(const unsigned long offset, const unsigned long *buf, const long len) {
    m_interface.blockWrite64(*this, offset, buf, len);
  };   
  
  // Interrupt handling
  inline void declareInterruptHandler(VmeInterruptHandler &ih) {
    m_interface.declareInterruptHandler(ih);
  };
  inline void cleanInterruptHandlers() {
    m_interface.cleanInterruptHandlers();
  };
  inline void removeInterruptHandler(VmeInterruptHandler &ih) {
    m_interface.removeInterruptHandler(ih);
  };
  //! Re-enable a VME RORA levels (at the end of a RORA interrupt handling)
  inline void reEnableInterrupt() { 
    m_interface.reEnableInterrupt();
  }; 
  //! Suspend interrupt notification
  void blockInterruptNotification() {
    m_interface.blockInterruptNotification();
  };
  //! Resume interrupt notification
  void resumeInterruptNotification() {
    m_interface.resumeInterruptNotification();
  };
  
private:
  unsigned long m_handle[5];
  unsigned long m_baseAddress;
  unsigned long m_mapSize;
  VmeInterface::AddrMod m_addrMod;
  VmeInterface &m_interface;
  bool m_exceptionTrapping;
  unsigned long m_nExcl;
  unsigned long m_exclStart[5];
  unsigned long m_exclEnd[5];
};                                             // End of VmePort declaration

//! Vme Interrupt Handler class; abstract base class for Interrupt Handlers
class VmeInterruptHandler {
public:
  //! Constructor
  VmeInterruptHandler(VmePort &port, char vect, int level) : 
      m_port(port), m_interruptLevel(level) {
    if (level == -1) {
      m_interruptVector = 0;
      m_interruptSoftVect = vect;
    } else {
      m_interruptVector = vect;
      m_interruptSoftVect = 0;
    } 
    m_port.declareInterruptHandler(*this); 
    m_active = true;
  };
  //! Destructor
  virtual ~VmeInterruptHandler() {
    m_active = false;
    m_port.removeInterruptHandler(*this);
  };
  //! Interrupt handling function
  virtual void interruptHandler(unsigned char vect, unsigned char softvect, int count) = 0;
  //! Activate a soft-vector
  inline void activateSoftVector(unsigned char softvect) { 
    if (m_interruptSoftVect == 0) m_port.getInterface().activateSoftVector(m_interruptVector, softvect);
  };
  //! Set the interrupt data word
  inline void setInterruptData(long data) {
    if (m_interruptSoftVect == 0) m_port.getInterface().setInterruptData(data);
  }
  //! Read the interrupt data word
  inline long getInterruptData() {
    return m_port.getInterface().getInterruptData();
  }
  //! Re-enable VME RORA levels (at the end of a RORA interrupt handling)
  inline void reEnableInterrupt() { m_port.reEnableInterrupt(); }; 
  //! m_interruptVector accessor
  inline char getInterruptVector() { return m_interruptVector; }
  //! m_interruptSoftVect accessor 
  inline char getInterruptSoftVect() { return m_interruptSoftVect; }
  //! m_interruptLevel accessor 
  inline int getInterruptLevel() { return m_interruptLevel; }
  //! m_active accessors
  inline void setActive(bool active) { m_active = active; };
  inline bool getActive() { return m_active; };
  
protected:
  VmePort &m_port;
  unsigned char m_interruptVector;
  unsigned char m_interruptSoftVect;
  unsigned int m_interruptLevel;
  bool m_active;
};

} // End namespace SctPixelRod

#endif // SCTPIXELROD_VMEPORT_H
