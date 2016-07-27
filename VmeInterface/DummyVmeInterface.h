/////////////////////////////////////////////////////////////////////
//! DummyVmeInterface.h 
//! version 1.2
//!
//! Dummy VmeInterface implemetation 
//!                                 
/////////////////////////////////////////////////////////////////////
// 24/04/2002  V 1.1 PM - First implementation
// 19/02/2004  V 1.2 PM - Safe single word R/W methods added            
//             

#ifndef SCTPIXELROD_DUMMY_VMEINTERFACE_H
#define SCTPIXELROD_DUMMY_VMEINTERFACE_H

#include <vector>

#include "VmeInterface.h"
#include "VmePort.h"


namespace SctPixelRod {

class DummyVmeInterface : public VmeInterface {
public:
  //! Constructor
  DummyVmeInterface();
  //! Destructor  
  ~DummyVmeInterface();                                                                  

  //! Install an interrupt handler
  virtual void declareInterruptHandler(VmeInterruptHandler &handler);
  //! Remove an interrupt handler
  virtual void removeInterruptHandler(VmeInterruptHandler &handler);
  //! Set the interrupt subvector mask
  virtual void cleanInterruptHandlers();
  //! Remove an interrupt handler
  virtual inline void activateSoftVector(unsigned char vect, unsigned char subvect) {
    if (subvect>0 && subvect<=32) {
      m_softVectMask |= 1 << (subvect-1);
    }
  };
  //! Re-enable VME RORA levels
  virtual void reEnableInterrupt() { }; 
  //! Suspend interrupt notification
  virtual void blockInterruptNotification(); 
  //! Resume interrupt notification
  virtual void resumeInterruptNotification(); 
  //! Set the interrupt data word
  virtual void setInterruptData(long data) {
    m_interruptData = data;
  };
  //! Read the interrupt data word
  virtual long getInterruptData() {
    return m_interruptData;
  };
  
  //! Return the number of bus errors
  virtual long getBusErrors();
  //! Throw exceptions in case of bus errors
  virtual void busErrorReport();
  virtual void busErrorReport(const unsigned long handle);
  virtual void busErrorReport(VmePort &port);
  //! Return the last error code
  virtual long getLastErrcode() { return 0; }
  //! Print the message corresponding to an error code
  virtual std::string getErrorMessage(const long errcode);

  //! Byte read (automatic mapping)
  virtual unsigned char read8  (const unsigned long handle, const unsigned long offset);  
  virtual unsigned char readS8  (const unsigned long handle, const unsigned long offset);  
  // Word read (automatic mapping)
  virtual unsigned short read16 (const unsigned long handle, const unsigned long offset);  
  virtual unsigned short readS16 (const unsigned long handle, const unsigned long offset);  
  //! Long Word read (automatic mapping)
  virtual unsigned long read32 (const unsigned long handle, const unsigned long offset);  
  virtual unsigned long readS32 (const unsigned long handle, const unsigned long offset);  
  //! Byte write (automatic mapping)
  virtual void write8 (const unsigned long handle, const unsigned long offset, const unsigned char value);   
  virtual void writeS8 (const unsigned long handle, const unsigned long offset, const unsigned char value);   
  //! Word write (automatic mapping)
  virtual void write16(const unsigned long handle, const unsigned long offset, const unsigned short value);    
  virtual void writeS16(const unsigned long handle, const unsigned long offset, const unsigned short value);    
  //! Long word write (automatic mapping)
  virtual void write32(const unsigned long handle, const unsigned long offset, const unsigned long value);   
  virtual void writeS32(const unsigned long handle, const unsigned long offset, const unsigned long value);   

  //! 32 bits block read (automatic mapping)
  virtual void blockRead32 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len);  
  //! 64 bits block read (automatic mapping)
  virtual void blockRead64 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len);   
  //! 32 bits block write (automatic mapping)
  virtual void blockWrite32(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len);  
  //! 64 bits block write (automatic mapping)
  virtual void blockWrite64(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len);   

  //! Register a new VmePort object
  virtual unsigned long registerPort(VmePort &port);         
  //! Delete a VmePort
  virtual void deletePort(VmePort &port);
  //! Return a pointer to the port map
  //virtual void *getPortMap(const unsigned long handle);

private:
  std::vector<VmePort *> m_ports;
  sigset_t m_sigMask;                 // Mask for blocking signals

  static unsigned char m_softVectMask;                  // Interrupt subvector array
  static long m_interruptData;                          // Interrupt data word
  static DummyVmeInterface *m_if;                       // Static pointer to THE interface
  static std::vector<VmeInterruptHandler *> m_hardIH;        // Vector of basic IH (always executed)
  static std::vector<VmeInterruptHandler *> m_softIH[32];    // Vector of slaves IH (executed when the subvector mask set
                                                        // by the basic IH equals the one defined in the slave IH

  //! Interrupt handler
  static void m_interruptHandler(int signum);
};                                             // End of RCCVmeInterface declaration

} // End namespace SctPixelRod

#endif // SCTPIXELROD_RCCVMEINTERFACE_H
