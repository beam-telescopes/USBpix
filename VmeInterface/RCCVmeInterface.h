/////////////////////////////////////////////////////////////////////
//! RCCVmeInterface.h 
//! version 1.5
//!
//! VmeInterface implemetation on the RCC API 
//!                                 
/////////////////////////////////////////////////////////////////////
// 20/02/2002  V 1.0 PM - First implementation
//  3/04/2002  V 1.1 PM - Modifications in the error handling sector
// 18/04/2002  V 1.2 PM - Modifications in the interrupt handling
// 22/04/2002  V 1.3 PM - New methods for blocking and resuming 
//                        interrupt notification and for setting and
//                        reading an interrupt data word          
// 13/08/2002  V 1.4 PM - Modifications for RCC v3
// 19/02/2004  V 1.5 PM - Safe single word R/W methods added            
//             

#ifndef SCTPIXELROD_RCCVMEINTERFACE_H
#define SCTPIXELROD_RCCVMEINTERFACE_H

#include <vector>
#include <map>

#include "VmeInterface.h"
#include "VmePort.h"
#include "rcc_error/rcc_error.h"
#include "vme_rcc/vme_rcc.h"

#include <boost/thread/thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/mutex.hpp>
 
namespace SctPixelRod {

class RCCVmeInterface : public VmeInterface {
public:
  //! Constructor
  RCCVmeInterface();
  //! Destructor  
  ~RCCVmeInterface();
  //! Factory
  static RCCVmeInterface* makeRCCVmeInterface();                                                                  

  //! Install an interrupt handler
  virtual void declareInterruptHandler(VmeInterruptHandler &handler);
  //! Uninstall all the handlers
  virtual void cleanInterruptHandlers();
  //! Remove an interrupt handler
  virtual void removeInterruptHandler(VmeInterruptHandler &handler);
  //! Set the interrupt subvector mask
  virtual inline void activateSoftVector(unsigned char vect, unsigned char subvect) {
    if (subvect>0 && subvect<=32) {
      m_softVectMask[vect] |= 1 << (subvect-1);
    }
  };
  //! Re-enable VME RORA levels
  virtual void reEnableInterrupt(); 
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
  virtual long getLastErrcode() { return m_lastErrcode; }
  //! Print the message corresponding to an error code
  virtual std::string getErrorMessage(const long errcode);

  //! Byte read (automatic mapping)
  virtual unsigned char read8  (const unsigned long handle, const unsigned long offset);  
  virtual unsigned char readS8  (const unsigned long handle, const unsigned long offset);  
  //! Word read (automatic mapping)
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
  VME_BlockTransferList_t m_btList[8];
  bool m_btListUse[8];
  int m_dmaBuf[8];
  long m_dmaBufSize[8];
  VME_BusErrorInfo_t m_busErrInfo;
  static long m_busErrors;                       // Number of bus error since last call to getBusErrors()
  int  m_lastErrcode;                            // Return code of the last operation 
  VME_InterruptList_t  m_irq[8];                 // Enabled interrupt vectors
  boost::mutex *m_mutex;                         // Mutex

  static int m_irqId[8];                                // Interrupt idetifier
  static long m_interruptData;                          // Interrupt data word
  static sigset_t m_sigMask;                            // Mask for blocking signals
  static RCCVmeInterface *m_if;                         // Static pointer to THE interface
  static unsigned char m_softVectMask[256];             // Interrupt subvector array
  static VmeInterruptHandler* m_hardIH[8][256][10];     // Vector of basic IH (always executed)
  static VmeInterruptHandler* m_softIH[32][10];         // Vector of slaves IH (executed when the subvector mask set
                                                        // by the basic IH equals the one defined in the slave IH

  // Covversion from generic to specific address modifiers codes
  int m_RCCAddrMod(VmeInterface::AddrMod mod);
  //! Block transfer buffer allocation
  unsigned int getBtBuf();
  //! Generic block transfer read
  void m_blockTransferRead(unsigned long *buf, unsigned int bufN);
  //! Generic block transfer write
  void m_blockTransferWrite(const unsigned long *buf, unsigned int bufN);
  //! SIGBUS handler
  static void m_busErrorHandler(int signum);
  //! Exit handler
  static void m_exitHandler(int signum);
  //! Interrupt handler
  static void m_interruptHandler(int signum);
  static void m_interruptSignal(int signum);

  std::string getCmemErrorString(unsigned long ret);

};                                             // End of RCCVmeInterface declaration

} // End namespace SctPixelRod

#endif // SCTPIXELROD_RCCVMEINTERFACE_H








