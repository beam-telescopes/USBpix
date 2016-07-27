/////////////////////////////////////////////////////////////////////
//! VmeInterface.h 
//! version 2.5
//!
//! VmeInterface abstract base class 
//!                                 
/////////////////////////////////////////////////////////////////////
//  8/11/2001  V 1.0 PM - Session management and error handling 
// 29/11/2001  V 1.1 PM - ATLAS coding rules compliance 
// 20/02/2002  V 2.0 PM - First implementation after the splitting
//                        VmeInterface/VmePort 
// 25/03/2002  V 2.1 TM - Data types unsigned, addr_mod->AddrMod, 
//                        namespace added            
//  4/04/2002  V 2.2 PM - Modification in the error handling methods
// 18/04/2002  V 2.3 PM - Modifications in the interrupt handling
//                        and in the error reporting methods
// 22/04/2002  V 2.4 PM - New methods for blocking and resuming 
//                        interrupt notification and for reading and
//                        setting an interrupt data word           
// 19/02/2004  V 2.5 PM - Safe single word R/W methods added            
//

/*!

   VmeInterface is a pure abstract class implementing the
   interface of a generic VME bus access library.
   There will be a derivation of VmeInterface for every
   VME access library. Class derived from VmeInterface
   must not be called directly by the user, but only via
   an instance of VmePort. The user has only to create a
   single instance of a VmeInterface derived class and pass
   it to the VmePort constructor.

*/

#ifndef SCTPIXELROD_VMEINTERFACE_H
#define SCTPIXELROD_VMEINTERFACE_H

#include <vector>
#include <string>

#include "BaseException.h"

namespace SctPixelRod {

class VmePort;
class VmeInterruptHandler;

#define VMEADD_MISALIGNED      11001
#define PCIADD_MISALIGNED      11002
#define PCI_VME_MISALIGNED     11003
#define DMAMEM_ALLOC_ERROR     11004
#define BT_INIT_ERROR          12000
#define BT_START_ERROR         13000
#define BT_END_ERROR           14000
#define BT_WAIT_ERROR          15000

//! Vme Exception class; an object of this type is thrown in case of a VME error
class VmeException : public BaseException {
public:
  enum ErrorClass {OK, INIT_ERROR, BUS_ERROR, MAP_ERROR, DMA_ERROR, BT_READ_ERROR, BT_WRITE_ERROR};
  //! Constructor
  VmeException(ErrorClass ec, long ecod, VmePort *port=NULL) : 
     BaseException("VME Error"), m_errorClass(ec), m_errorCode(ecod), m_port(port) {std::cerr << "VME: ErrCode= " << ecod << std::endl;} 
  VmeException(std::string etext, ErrorClass ec, long ecod, VmePort *port=NULL) : 
     BaseException(etext), m_errorClass(ec), m_errorCode(ecod), m_port(port) {std::cerr << "VME Error: ErrCode: " << ecod << " ErrText: " << etext << std::endl;}

  //! m_errorClass accessor
  ErrorClass getErrorClass() { return m_errorClass; };
  //! m_errorCode accessor
  long getErrorCode() { return m_errorCode; };
  //! m_port accessor
  VmePort *getPort() { return m_port; };
private:
  ErrorClass m_errorClass;
  int m_errorCode;
  VmePort *m_port;
};

class VmeInterface {
public:
  enum AddrMod { A16, A24, A32 };

  //! Constructor
  VmeInterface() {};
  //! Destructor  
  virtual ~VmeInterface() {};                                                                  

  //! Install an interrupt handler
  virtual void declareInterruptHandler(VmeInterruptHandler &handler) = 0;
  //! Uninstall all the handlers
  virtual void cleanInterruptHandlers() = 0;
  //! Remove an interrupt handler
  virtual void removeInterruptHandler(VmeInterruptHandler &handler) = 0;
  //! Set the interrupt subvector mask
  virtual void activateSoftVector(unsigned char vect, unsigned char subvect) = 0; 
  //! Re-enable VME RORA levels
  virtual void reEnableInterrupt() = 0; 
  //! Suspend interrupt notification
  virtual void blockInterruptNotification() = 0; 
  //! Resume interrupt notification
  virtual void resumeInterruptNotification() = 0; 
  //! Set the interrupt data word
  virtual void setInterruptData(long data) = 0;
  //! Read the interrupt data word
  virtual long getInterruptData() = 0;
  
   //! Return the number of bus errors
  virtual long getBusErrors() = 0;
  //! Throw exceptions in case of bus errors
  virtual void busErrorReport() = 0;
  virtual void busErrorReport(const unsigned long handle) = 0;
  virtual void busErrorReport(VmePort &port) = 0;
  //! Return the last error code
  virtual long getLastErrcode() = 0;
  //! Print the message corresponding to an error code
  virtual std::string getErrorMessage(const long errcode) = 0;

  //! Byte read (automatic mapping)
  virtual unsigned char read8  (const unsigned long handle, const unsigned long offset) = 0;  
  virtual unsigned char readS8  (const unsigned long handle, const unsigned long offset) = 0;  
  // Word read (automatic mapping)
  virtual unsigned short read16 (const unsigned long handle, const unsigned long offset) = 0;  
  virtual unsigned short readS16 (const unsigned long handle, const unsigned long offset) = 0;  
  //! Long Word read (automatic mapping)
  virtual unsigned long read32 (const unsigned long handle, const unsigned long offset) = 0;  
  virtual unsigned long readS32 (const unsigned long handle, const unsigned long offset) = 0;  
  //! Byte write (automatic mapping)
  virtual void write8 (const unsigned long handle, const unsigned long offset, const unsigned char value) = 0;   
  virtual void writeS8 (const unsigned long handle, const unsigned long offset, const unsigned char value) = 0;   
  //! Word write (automatic mapping)
  virtual void write16(const unsigned long handle, const unsigned long offset, const unsigned short value)= 0;    
  virtual void writeS16(const unsigned long handle, const unsigned long offset, const unsigned short value)= 0;    
  //! Long word write (automatic mapping)
  virtual void write32(const unsigned long handle, const unsigned long offset, const unsigned long value) = 0;   
  virtual void writeS32(const unsigned long handle, const unsigned long offset, const unsigned long value) = 0;   

  //! 32 bits block read (automatic mapping)
  virtual void blockRead32 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len) = 0;  
  //! 64 bits block read (automatic mapping)
  virtual void blockRead64 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len) = 0;   
  //! 32 bits block write (automatic mapping)
  virtual void blockWrite32(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len) = 0;  
  //! 64 bits block write (automatic mapping)
  virtual void blockWrite64(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len) = 0;   

  //! Register a new VmePort object
  virtual unsigned long registerPort(VmePort &port) = 0;         
  //! Delete a VmePort
  virtual void deletePort(VmePort &port) = 0;
  //! Return a pointer to the port map
  //virtual void *getPortMap(const unsigned long handle) = 0;

  //! Return interface status
  bool ok() { return m_ok; }; 

protected:  
  bool m_ok;  //! Interface status

};                                             // End of VmeInterface declaration

} // End namespace SctPixelRod

#endif // SCTPIXELROD_VMEINTERFACE_H






