/////////////////////////////////////////////////////////////////////
//! RCCVmeInterface.cxx 
//! version 1.6
//!
//! VmeInterface implemetation on the RCC API 
//!                                 
/////////////////////////////////////////////////////////////////////
// 20/02/2002  V 1.0 PM - First implementation
//  4/04/2002  V 1.1 PM - Block transfer not working, Interrupt 
//                        Handling not implemented
// 18/04/2002  V 1.2 PM - Block transfer and interrupts ok. Bus error
//                        handling ok. Other ecxeptions still to be
//                        implemented           
// 22/04/2002  V 1.3 PM - New methods for blocking and resuming 
//                        interrupt notification
//                        Support for RORA interrupts on level 6
//                        New method for passing an int to soft
//                        interrupts.           
// 13/08/2002  V 1.4 PM - Modifications for RCC v3
// 19/02/2004  V 1.5 PM - Safe single word R/W methods added            
// 19/07/2004  V 1.6 JL - Throw VmeException if VME_Open fails
//

#include <signal.h>
#include <sys/types.h>

#include "RCCVmeInterface.h"
#include "cmem_rcc/cmem_rcc.h"

using namespace SctPixelRod;

long RCCVmeInterface::m_busErrors = 0;
int  RCCVmeInterface::m_irqId[8] = { -1, -1, -1, -1, -1, -1, -1, -1};       
sigset_t RCCVmeInterface::m_sigMask;
        
long RCCVmeInterface::m_interruptData = 0;
RCCVmeInterface* RCCVmeInterface::m_if = NULL;
unsigned char  RCCVmeInterface::m_softVectMask[256];                      
VmeInterruptHandler*  RCCVmeInterface::m_hardIH[8][256][10];   
VmeInterruptHandler*  RCCVmeInterface::m_softIH[32][10];  

RCCVmeInterface* RCCVmeInterface::makeRCCVmeInterface() {
  if (m_if == NULL) {
    try {
      m_if = new RCCVmeInterface();
      return m_if;
    }
    catch (...) {
      m_if = NULL;
      return m_if;
    }
  } else {
    return m_if;
  }
}

void RCCVmeInterface::m_exitHandler(int signum) {
  delete m_if; 
  exit(0);
}

std::string RCCVmeInterface::getCmemErrorString(unsigned long ret)
{
  char pid[255], code[255];
  CMEM_err_get(ret, pid, code);
  std::string errText(code);
  return errText;
}

void RCCVmeInterface::m_busErrorHandler(int signum) {
  RCCVmeInterface::m_busErrors++; 
}

void RCCVmeInterface::m_interruptHandler(int signum) {
  int is, smask;
  unsigned int i;
  static VME_InterruptInfo_t info[8];
  
  // Get interrupt info  
  int level = SIGRTMAX - signum;
  VME_InterruptInfoGet(m_irqId[level],&info[level]);
  int vec = info[level].vector;


  // Execute hard handlers
  m_softVectMask[vec] = 0;
  m_if->m_interruptData = 0;
  for (i=0; i<10; i++) {
    if (m_hardIH[level][vec][i] != 0) {
      if (m_hardIH[level][vec][i]->getActive()) {
        m_hardIH[level][vec][i]->interruptHandler(vec, 0, info[level].multiple);
      }
    }
  }
  
  // Execute soft handlers
  smask = m_softVectMask[vec];
  for (is=0; is<32; is++) {
    if ((smask & (1 << is)) != 0) { 
      for (i=0; i<10; i++) {
        if (m_softIH[is][i] != 0) {
	  if (m_softIH[is][i]->getActive()) {
	    m_softIH[is][i]->interruptHandler(vec, is, info[level].multiple);
	  }
	}
      }
    }
  }

  // Re-enable interrupts
  if (info[level].type == VME_INT_RORA) {
    VME_InterruptReenable(m_irqId[level]);
  }
}

RCCVmeInterface::RCCVmeInterface() {
  int ret;
  int i,j,k;

  // Init status variable
  m_ok = false;

  // Open the package
  m_lastErrcode = VME_Open();

  // Create the mutex
  m_mutex = new boost::mutex();

  if (m_lastErrcode != 0)
      throw VmeException(getErrorMessage(m_lastErrcode), VmeException::INIT_ERROR, m_lastErrcode);

  // Pre-allocate a DMA buffer
  for (int i=0; i<8; i++) {
    m_btListUse[i] = false;
    m_dmaBuf[i] = 0;
    m_dmaBufSize[i] = 0;
  }
  ret = CMEM_Open();
  char dma_name[17] = "VmeInterface DMA";
  ret = CMEM_SegmentAllocate(0x10000, dma_name, &m_dmaBuf[0]); 
  if (ret) {
    m_dmaBufSize[0] = 0;
  } else {
    m_dmaBufSize[0] = 0x10000;
  }
  // Install the bus error handler
  struct sigaction sa;
  sigemptyset(&sa.sa_mask); 
  sa.sa_flags = 0;
  sa.sa_handler = m_busErrorHandler; 
  ret = sigaction(SIGBUS, &sa, NULL);
  m_lastErrcode = VME_BusErrorRegisterSignal(SIGBUS);
  m_lastErrcode = VME_BusErrorInfoGet(&m_busErrInfo);
  // Install the global interrupt handler
  sigemptyset(&sa.sa_mask); 
  sa.sa_flags = 0;
  sa.sa_handler = m_interruptHandler; 
  for (i=0; i<8; i++) sigaction(SIGRTMAX-i, &sa, NULL);
  // Install the exit handler
  sigemptyset(&sa.sa_mask); 
  sa.sa_flags = 0;
  sa.sa_handler = m_exitHandler; 
  ret = sigaction(SIGTERM, &sa, NULL);
  ret = sigaction(SIGINT, &sa, NULL);
  // Pepare the mask for blocking interrupt handlers
  sigemptyset(&m_sigMask);
  for (i=0; i<8; i++) sigaddset(&m_sigMask, SIGRTMAX-i);
  // Clear the list of interrupts
  for (i=0; i<8; i++) m_irq[i].number_of_items = 0;
  for (i=0; i<10; i++) {
    for (k=0; k<256; k++) {
      for (j=0; j<8; j++) {
	m_hardIH[j][k][i] = 0;
      }
    } 
    for (k=0; k<32; k++) {
      m_softIH[k][i] = 0;
    }
  }
  // Set the static pointer to this object
  m_if = this;
  // Set status variable to ok
  m_ok = true;
};


RCCVmeInterface::~RCCVmeInterface() {
  std::cerr << "Deleting VmeInterface..." << std::endl;
  // Release the DMA buffer
  for (int i=0; i<8; i++) {
    if (m_dmaBufSize[i] != 0) {
      std::cout << "  Releasing DMA buffer" << std::endl;
      CMEM_SegmentFree(m_dmaBuf[i]);
    }
  }
  CMEM_Close();
  // Unlink the interrupts
  int i;
  for (i=0; i<8; i++) {
    if (m_irq[i].number_of_items > 0) {
      std::cout << "  Unlinking interrupt handlers for level " << i << std::endl;
      m_lastErrcode = VME_InterruptRegisterSignal(m_irqId[i], 0);
      m_lastErrcode = VME_InterruptUnlink(m_irqId[i]);
      m_irq[i].number_of_items = 0;
    }
  }
  // Remove existing port mappings
  unsigned int ip;
  for (ip=0; ip<m_ports.size(); ip++) {
    if (m_ports[ip] != NULL) {
      std::cout << "  Unmapping port " << ip  << std::endl;
      VME_MasterUnmap(m_ports[ip]->getHandle());
      m_ports[ip] = NULL;
    }
  }
  // Close the package
  VME_Close();
  m_ok = false;
};

void RCCVmeInterface::declareInterruptHandler(VmeInterruptHandler &handler) {
  unsigned char vect = handler.getInterruptVector();
  unsigned char softvec = handler.getInterruptSoftVect();
  unsigned int level = handler.getInterruptLevel();
  if (softvec == 0) {
    if (level <= 0 || level > 7) return;
    // Hard IH definition
    unsigned int i,nh=0;
    for (i=0; i<10; i++) {
      if (&handler == m_hardIH[level][vect][i]) return;
      if (m_hardIH[level][vect][i] != 0) nh++;
    }
    for (i=0; i<10; i++) {
      if (m_hardIH[level][vect][i] == 0) {
	m_hardIH[level][vect][i] = &handler; 
        nh++;
	break;
      }
    }
    if (nh == 1) {
      // Set up the glodal handler
      if (m_irq[level].number_of_items > 0) {      
        m_lastErrcode = VME_InterruptRegisterSignal(m_irqId[level], 0);
        m_lastErrcode = VME_InterruptUnlink(m_irqId[level]);
      }
      m_irq[level].list_of_items[m_irq[level].number_of_items].vector = vect;
      m_irq[level].list_of_items[m_irq[level].number_of_items].level = level;
      if (level == 6) {
        m_irq[level].list_of_items[m_irq[level].number_of_items].type = VME_INT_RORA;
      } else {
        m_irq[level].list_of_items[m_irq[level].number_of_items].type = VME_INT_ROAK;
      }
      m_irq[level].number_of_items++;
      m_lastErrcode = VME_InterruptLink(&m_irq[level], &m_irqId[level]);
      m_lastErrcode = VME_InterruptRegisterSignal(m_irqId[level], SIGRTMAX-level);      
    }
  } else if (softvec > 0 && softvec <= 32) {
    // Soft IH definition
    unsigned int i;
    for (i=0; i<10; i++) {
      if (&handler == m_softIH[softvec-1][i]) return;
    }
    for (i=0; i<10; i++) {
      if (m_softIH[softvec-1][i] == 0) {
	m_softIH[softvec-1][i] = &handler; 
	break;
      }
    }
  }
}

void RCCVmeInterface::cleanInterruptHandlers() {
  // Unlink the global handler
  int i,j,k;
  for (i=0; i<8; i++) {
    if (m_irq[i].number_of_items > 0) {      
      m_lastErrcode = VME_InterruptUnlink(m_irqId[i]);
      m_lastErrcode = VME_InterruptRegisterSignal(m_irqId[i], 0);
      m_irq[i].number_of_items = 0;
    }
  }
  for (i=0; i<10; i++) {
    for (k=0; k<256; k++) {
      for (j=0; j<8; j++) {
	m_hardIH[j][k][i] = 0;
      }
    } 
    for (k=0; k<32; k++) {
      m_softIH[k][i] = 0;
    }
  }
}

void RCCVmeInterface::removeInterruptHandler(VmeInterruptHandler &handler) {
  int vect = handler.getInterruptVector();
  char softvec = handler.getInterruptSoftVect();
  int level = handler.getInterruptLevel();
  if (softvec == 0) {   
    int i, nh=0;
    // Basic IH cancellation
    for (i=0; i<10; i++) {
      if (&handler == m_hardIH[level][vect][i]) {      
        m_hardIH[level][vect][i] = 0;
        break;
      }
      for (i=0; i<10; i++) {
        if (m_hardIH[level][vect][i] !=0) nh++;
      }
    }
    if (nh == 0) {
      // Check if vect has installed handlers
      int iv;
      bool found = false;
      for (iv=0; iv<m_irq[level].number_of_items; iv++) {
        if (found) m_irq[level].list_of_items[iv-1] = m_irq[level].list_of_items[iv];
        if (vect == m_irq[level].list_of_items[iv].vector) found = true;
      }
      if (found) {
        // Unlink the global handler
        if (m_irq[level].number_of_items > 0) {      
          m_lastErrcode = VME_InterruptUnlink(m_irqId[level]);
          m_lastErrcode = VME_InterruptRegisterSignal(m_irqId[level], 0);
        }
        
        if (--m_irq[level].number_of_items == 0) {
          m_irqId[level] = -1;
        } else {     
          m_lastErrcode = VME_InterruptLink(&m_irq[level], &m_irqId[level]);
          m_lastErrcode = VME_InterruptRegisterSignal(m_irqId[level], 0);
	}
      }
    }
  } else if (softvec > 0 && softvec <= 32) {
    int i;
    // Soft IH cancellation
    for (i=0; i<10; i++) {
      if (&handler == m_softIH[softvec-1][i]) {
        m_softIH[softvec-1][i]=0;
        return;
      }
    }
  }  
}

void RCCVmeInterface::reEnableInterrupt() {
  int i;
  for (i=0; i<8; i++) VME_InterruptReenable(m_irqId[i]);
}
 
void RCCVmeInterface::blockInterruptNotification() {
  pthread_sigmask(SIG_BLOCK, &m_sigMask, NULL);
} 

void RCCVmeInterface::resumeInterruptNotification() {
  pthread_sigmask(SIG_UNBLOCK, &m_sigMask, NULL);
}

unsigned char RCCVmeInterface::read8  (const unsigned long handle, const unsigned long offset) {
  u_char ldata;       
  VME_ReadFastUChar(handle ,offset, &ldata);
  busErrorReport(handle);
  return ldata;
}  

unsigned char RCCVmeInterface::readS8 (const unsigned long handle, const unsigned long offset) {
  u_char ldata;
  VME_ErrorCode_t ret;       
  ret = VME_ReadSafeUChar(handle ,offset, &ldata);
  if (ret != VME_SUCCESS) throw VmeException(getErrorMessage(ret), VmeException::BUS_ERROR, VME_BUSERROR); 
  return ldata;
}

unsigned short RCCVmeInterface::read16 (const unsigned long handle, const unsigned long offset) {
  u_short ldata;       
  VME_ReadFastUShort(handle ,offset, &ldata);
  busErrorReport(handle);
  return ldata;
}

unsigned short RCCVmeInterface::readS16 (const unsigned long handle, const unsigned long offset) {
  u_short ldata;
  VME_ErrorCode_t ret;       
  ret = VME_ReadSafeUShort(handle ,offset, &ldata);
  if (ret != VME_SUCCESS) throw VmeException(getErrorMessage(ret), VmeException::BUS_ERROR, VME_BUSERROR); 
  return ldata;
}

unsigned long RCCVmeInterface::read32 (const unsigned long handle, const unsigned long offset) {
  u_int ldata;       
  VME_ReadFastUInt(handle ,offset, &ldata);
  busErrorReport(handle);
  return ldata;
}

unsigned long RCCVmeInterface::readS32 (const unsigned long handle, const unsigned long offset) {
  u_int ldata;
  VME_ErrorCode_t ret;       
  ret = VME_ReadSafeUInt(handle ,offset, &ldata);
  if (ret != VME_SUCCESS) throw VmeException(getErrorMessage(ret), VmeException::BUS_ERROR, VME_BUSERROR); 
  return ldata;
}

void RCCVmeInterface::write8 (const unsigned long handle, const unsigned long offset, const unsigned char value) {
  VME_WriteFastUChar(handle ,offset, value);
  busErrorReport(handle);
}  

void RCCVmeInterface::writeS8 (const unsigned long handle, const unsigned long offset, const unsigned char value) {
  VME_ErrorCode_t ret;       
  ret = VME_WriteSafeUChar(handle ,offset, value);
  if (ret != VME_SUCCESS) throw VmeException(getErrorMessage(ret), VmeException::BUS_ERROR, VME_BUSERROR); 
}  

void RCCVmeInterface::write16(const unsigned long handle, const unsigned long offset, const unsigned short value) {
  VME_WriteFastUShort(handle ,offset, value);
  busErrorReport(handle);
}    

void RCCVmeInterface::writeS16 (const unsigned long handle, const unsigned long offset, const unsigned short value) {
  VME_ErrorCode_t ret;       
  ret = VME_WriteSafeUShort(handle ,offset, value);
  if (ret != VME_SUCCESS) throw VmeException(getErrorMessage(ret), VmeException::BUS_ERROR, VME_BUSERROR); 
}  

void RCCVmeInterface::write32(const unsigned long handle, const unsigned long offset, const unsigned long value) {
  VME_WriteFastUInt(handle ,offset, value);
  busErrorReport(handle);
}   

void RCCVmeInterface::writeS32 (const unsigned long handle, const unsigned long offset, const unsigned long value) {
  VME_ErrorCode_t ret;       
  ret = VME_WriteSafeUInt(handle ,offset, value);
  if (ret != VME_SUCCESS) throw VmeException(getErrorMessage(ret), VmeException::BUS_ERROR, VME_BUSERROR); 
}  

unsigned int RCCVmeInterface::getBtBuf() {
  unsigned int bufN=99;
  unsigned int count=1000;
  do {
    m_mutex->lock();
    for (unsigned int i=0; i<7; i++) {
      if (!m_btListUse[i]) {
	bufN = i;
	m_btListUse[i] = true;
	break;
      }
    }
    m_mutex->unlock();
    count--;
    if (bufN == 99) boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  } while (bufN == 99 && count > 0);
  if (count == 0) {
    throw VmeException(std::string("Could not get BlockTransferBuffer inside RCCVmeInterface"), VmeException::MAP_ERROR, DMAMEM_ALLOC_ERROR); 
  }
  return bufN;
}

void RCCVmeInterface::blockRead32 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len) {
  if (port.getAddrMod() == A16) {
    // Block moving not implemented in A16;
    return;
  }
  unsigned int bufN = getBtBuf();
  m_btList[bufN].number_of_items = 1;
  m_btList[bufN].list_of_items[0].vmebus_address = port.getBaseAddress()+offset;
  m_btList[bufN].list_of_items[0].size_requested = len;
  m_btList[bufN].list_of_items[0].control_word = VME_DMA_D32R;
  m_blockTransferRead(buf, bufN);      
  busErrorReport(port);
} 

void RCCVmeInterface::blockRead64 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len) {
  if (port.getAddrMod() == A16) {
    // Block moving not implemented in A16;
    return;
  }
  unsigned int bufN = getBtBuf();
  m_btList[bufN].number_of_items = 1;
  m_btList[bufN].list_of_items[0].vmebus_address = port.getBaseAddress()+offset;
  m_btList[bufN].list_of_items[0].size_requested = len;
  m_btList[bufN].list_of_items[0].control_word = VME_DMA_D64R;
  m_blockTransferRead(buf, bufN);      
  busErrorReport(port);
}

void RCCVmeInterface::blockWrite32(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len) {
  if (port.getAddrMod() == A16) {
    // Block moving not implemented in A16;
    return;
  }
  unsigned int bufN = getBtBuf();
  m_btList[bufN].number_of_items = 1;
  m_btList[bufN].list_of_items[0].vmebus_address = port.getBaseAddress()+offset;
  m_btList[bufN].list_of_items[0].size_requested = len;
  m_btList[bufN].list_of_items[0].control_word = VME_DMA_D32W;
  m_blockTransferWrite(buf, bufN);      
  busErrorReport(port);
}  

void RCCVmeInterface::blockWrite64(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len) {
  if (port.getAddrMod() == A16) {
    // Block moving not implemented in A16;
    return;
  }
  unsigned int bufN = getBtBuf();
  m_btList[bufN].number_of_items = 1;
  m_btList[bufN].list_of_items[0].vmebus_address = port.getBaseAddress()+offset;
  m_btList[bufN].list_of_items[0].size_requested = len;
  m_btList[bufN].list_of_items[0].control_word = VME_DMA_D64W;
  m_blockTransferWrite(buf, bufN);      
  busErrorReport(port);
}   

void RCCVmeInterface::m_blockTransferRead(unsigned long *buf, unsigned int bufN) {
  unsigned long ret;
  
  if (bufN >=0 && bufN <8) {
    long vmebusAddr = m_btList[bufN].list_of_items[0].vmebus_address;
    if (vmebusAddr%4 != 0) {
      m_btListUse[bufN] = false;
      throw VmeException(std::string("VME Address is misaligned"), VmeException::MAP_ERROR, VMEADD_MISALIGNED); 
    }
    // Check if the dma buffer needs to be enlarged
    long size = m_btList[bufN].list_of_items[0].size_requested;
    if (size%8 != 0) m_btList[bufN].list_of_items[0].size_requested+=8-size%8;
    if (size > m_dmaBufSize[bufN]) {
      if (m_dmaBufSize[bufN]) CMEM_SegmentFree(m_dmaBuf[bufN]);
      char dma_name[17] = "VmeInterface DMA";
      ret = CMEM_SegmentAllocate(size+0x1000, dma_name, &m_dmaBuf[bufN]); 
      if (ret) {
	m_dmaBufSize[bufN] = 0;
	m_btListUse[bufN] = false;
	throw VmeException(getCmemErrorString(ret), VmeException::MAP_ERROR, DMAMEM_ALLOC_ERROR); 
      } else {
	m_dmaBufSize[bufN] = size+0x1000;
      }
    }
    // Perform DMA transfer
    unsigned long pcidma;
    unsigned long *bdma;
    CMEM_SegmentPhysicalAddress(m_dmaBuf[bufN], &pcidma);
    CMEM_SegmentVirtualAddress (m_dmaBuf[bufN], (unsigned long*)&bdma);
    if (pcidma%4 != 0) {
      m_btListUse[bufN] = false;
      throw VmeException(VmeException::MAP_ERROR, PCIADD_MISALIGNED);
    } 
    if ((pcidma-vmebusAddr)%8 != 0) {
      m_btListUse[bufN] = false;
      throw VmeException(VmeException::MAP_ERROR, PCI_VME_MISALIGNED); 
    }
    m_btList[bufN].list_of_items[0].system_iobus_address = pcidma;
    int btId;
    ret = 0;  
    m_mutex->lock();

    int count=0;
    do {
      count++;
      ret = VME_BlockTransferInit(&m_btList[bufN], &btId);
      if (ret) usleep(100);
    } while(ret && count<10);
    if (count > 1) std::cout << "Read - Init - " << count << std::endl; 
    if (count >= 10) {
      m_btListUse[bufN] = false;
      m_mutex->unlock();
      throw VmeException(getErrorMessage(ret), VmeException::BT_READ_ERROR, BT_INIT_ERROR+count); 
    }

    count = 0;
    do {
      count++;
      ret = VME_BlockTransferStart(btId);
      if (ret) usleep(100);
    } while (ret && count<10);
    if (count > 1) std::cout << "Read - Start - " << count << std::endl; 
    if (count >= 10) {
      m_btListUse[bufN] = false;
      VME_BlockTransferEnd(btId);
      m_mutex->unlock();
      throw VmeException(getErrorMessage(ret), VmeException::BT_READ_ERROR, BT_START_ERROR+count); 
    }

    count=0;
    do {
      count++;
      usleep(1);
      //usleep(m_dmaBufSize[bufN]/500);
      ret = VME_BlockTransferWait(btId, 0, &m_btList[bufN]);
    } while(ret && count<100);
    if (count > 1) std::cout << "Read - Wait - " << count << std::endl; 
    if (count >= 100) {
      m_btListUse[bufN] = false;
      VME_BlockTransferEnd(btId);
      m_mutex->unlock();
      throw VmeException(getErrorMessage(ret), VmeException::BT_READ_ERROR, BT_WAIT_ERROR+count); 
    }

    ret = VME_BlockTransferEnd(btId);
    m_mutex->unlock();
    if (ret) {
      m_btListUse[bufN] = false;
      throw VmeException(getErrorMessage(ret), VmeException::BT_READ_ERROR, BT_END_ERROR+ret); 
    }

    // Copy the output to the user buffer
    int i;
    for (i=0; i<(size>>2); i++) {
      buf[i] = bdma[i];
    }
    m_btListUse[bufN] = false;   
  } else {
    throw VmeException(VmeException::MAP_ERROR, DMAMEM_ALLOC_ERROR); 
  }
}

void RCCVmeInterface::m_blockTransferWrite(const unsigned long *buf, unsigned int bufN) {
  unsigned long ret;
  
  if (bufN >=0 && bufN <8) {
    long vmebusAddr = m_btList[bufN].list_of_items[0].vmebus_address;
    if (vmebusAddr%4 != 0) {
      m_btListUse[bufN] = false;
      throw VmeException(VmeException::MAP_ERROR, VMEADD_MISALIGNED); 
    }
    // Check if the dma buffer needs to be enlarged
    long size = m_btList[bufN].list_of_items[0].size_requested;
    if (size%8 != 0) m_btList[bufN].list_of_items[0].size_requested+=8-size%8;
    if (size > m_dmaBufSize[bufN]) {
      if (m_dmaBufSize[bufN]) CMEM_SegmentFree(m_dmaBuf[bufN]);
      char dma_name[17] = "VmeInterface DMA";
      ret = CMEM_SegmentAllocate(size+0x1000, dma_name, &m_dmaBuf[bufN]); 
      if (ret) {
	m_dmaBufSize[bufN] = 0;
	m_btListUse[bufN] = false;
        throw VmeException(getCmemErrorString(ret), VmeException::MAP_ERROR, DMAMEM_ALLOC_ERROR); 
      } else {
	m_dmaBufSize[bufN] = size+0x1000;
      }
    }
    // Copy the input to the dma buffer
    int i;
    unsigned long pcidma;
    unsigned long *bdma;
    CMEM_SegmentPhysicalAddress(m_dmaBuf[bufN], &pcidma);
    CMEM_SegmentVirtualAddress(m_dmaBuf[bufN],  (unsigned long *)&bdma);
    if (pcidma%4 != 0) {
      m_btListUse[bufN] = false;
      throw VmeException(VmeException::MAP_ERROR, PCIADD_MISALIGNED); 
    }
    if ((pcidma-vmebusAddr)%8 != 0) {
      m_btListUse[bufN] = false;
      throw VmeException(VmeException::MAP_ERROR, PCI_VME_MISALIGNED); 
    }
    for (i=0; i<(size>>2); i++) {
      bdma[i] = buf[i];
    }   
    // Perform DMA transfer
    m_btList[bufN].list_of_items[0].system_iobus_address = pcidma;
    int btId;
    ret = 0;  
    m_mutex->lock();

    int count=0;
    do {
      count++;
      ret = VME_BlockTransferInit(&m_btList[bufN], &btId);
      if (ret) usleep(100);
    } while(ret && count<10);
    if (count >= 10) {
      m_btListUse[bufN] = false;
      m_mutex->unlock();
      throw VmeException(getErrorMessage(ret), VmeException::BT_WRITE_ERROR, BT_INIT_ERROR+count); 
    }

    count = 0;
    do {
      count++;
      ret = VME_BlockTransferStart(btId);
      if (ret) usleep(100);
    } while (ret && count<10);
    if (count >= 10) {
      m_btListUse[bufN] = false;
      VME_BlockTransferEnd(btId);
      m_mutex->unlock();
      throw VmeException(getErrorMessage(ret), VmeException::BT_WRITE_ERROR, BT_START_ERROR+count); 
    }

    count=0;
    do {
      count++;
      usleep(m_dmaBufSize[bufN]/50000);
      ret = VME_BlockTransferWait(btId, 0, &m_btList[bufN]);
    } while(ret && count<100);
    if (count >= 100) {
      m_btListUse[bufN] = false;
      VME_BlockTransferEnd(btId);
      m_mutex->unlock();
      throw VmeException(getErrorMessage(ret), VmeException::BT_WRITE_ERROR, BT_WAIT_ERROR+count); 
    }

    ret = VME_BlockTransferEnd(btId);
    m_mutex->unlock();
    if (ret) {
      m_btListUse[bufN] = false;
      throw VmeException(getErrorMessage(ret), VmeException::BT_WRITE_ERROR, BT_END_ERROR+ret); 
    }

    m_btListUse[bufN] = false;
  } else {
    throw VmeException(VmeException::MAP_ERROR, DMAMEM_ALLOC_ERROR); 
  }
}

unsigned long RCCVmeInterface::registerPort(VmePort &port) {
  int ret;
  unsigned int ip;
  VME_MasterMap_t master_map = {0x0, 0x1000, VME_A32, 0};

  m_mutex->lock();
  if (port.m_nExcl == 0) {
    master_map.vmebus_address = port.getBaseAddress();
    master_map.window_size = port.getMapSize();
    master_map.address_modifier =  m_RCCAddrMod(port.getAddrMod());
    int pp = port.m_handle[0];
    ret = VME_MasterMap(&master_map, &pp);
    port.m_handle[0] = pp;
  } else {
//    for (unsigned long j=0; j<port.m_nExcl+1; j++) {
//      std::cout << std::hex << "Start = 0x" << port.m_exclStart[j];
//      std::cout << " End = 0x" << port.m_exclEnd[j] << std::dec << std::endl;
//    }
    master_map.vmebus_address = port.m_baseAddress;
    master_map.window_size = port.m_exclStart[0];
    master_map.address_modifier =  m_RCCAddrMod(port.m_addrMod);
    int pp = port.m_handle[0];
    ret = VME_MasterMap(&master_map, &pp);
    port.m_handle[0] = pp;
    std::cout << std::hex << "Base = 0x" << master_map.vmebus_address;
    std::cout << " Size = 0x" << master_map.window_size;
    std::cout << " Ret = " << ret << std::dec << std::endl;
    for (unsigned long i=0; i<port.m_nExcl; i++) {
      master_map.vmebus_address = port.m_exclEnd[i]+1+port.m_baseAddress;
      master_map.window_size = port.m_exclStart[i+1]-port.m_exclEnd[i]-1;
      int pp = port.m_handle[i+1];
      ret = VME_MasterMap(&master_map, &pp);
      port.m_handle[i+1] = pp;
      std::cout << std::hex << "Base = 0x" << master_map.vmebus_address;
      std::cout << " Size = 0x" << master_map.window_size;
      std::cout << " Ret = " << ret << std::dec << std::endl;
    }
  }
  for (ip=0; ip<m_ports.size(); ip++) {
    if (m_ports[ip] == NULL) {
      m_ports[ip] = &port;
      m_mutex->unlock();
      return port.m_handle[0];
    }
  }
  m_ports.push_back(&port);
  m_mutex->unlock();
  return port.m_handle[0];
}         

void RCCVmeInterface::deletePort(VmePort &port) {
  unsigned int ip;

  m_mutex->lock();
  std::cout << "Deleting VmePort, start = " << std::hex << port.getBaseAddress() << std::dec << std::endl; 
  for (unsigned long i=0; i<port.m_nExcl+1; i++) {
    VME_MasterUnmap(port.m_handle[i]);
  }
  int count = 0;
  for (ip=0; ip<m_ports.size(); ip++) {
    if (m_ports[ip] == &port) {
      m_ports[ip] = NULL;
    } else {
      if (m_ports[ip] != NULL) {
	std::cout << std::hex << m_ports[ip]->getBaseAddress() << " " << std::dec;
	count++;
      }
    }
  }
  std::cout << " " << count << " VmePorts still allocated" << std::endl;
  m_mutex->unlock();
}

//void *RCCVmeInterface::getPortMap(const unsigned long handle) {
//  u_int value;
//  VME_MasterMapVirtualAddress(handle, &value);
//  return (void *)value;
//}

int RCCVmeInterface::m_RCCAddrMod(VmeInterface::AddrMod mod) {
  switch (mod) {
  case A16:
    return VME_A16;
    break;
  case A24:
    return VME_A24;
    break;
  case A32:
  default:
    return VME_A32;
    break;
  };
}

void RCCVmeInterface::busErrorReport() {

  if (getBusErrors() != 0) throw VmeException(VmeException::BUS_ERROR, VME_BUSERROR); 
}

void RCCVmeInterface::busErrorReport(const unsigned long handle) {
  if (getBusErrors() != 0) {
    // Loop over the registered ports
    unsigned int ip;
    for (ip=0; ip<m_ports.size(); ip++) {
      if (handle == m_ports[ip]->getHandle()) {  
        if (m_ports[ip]->getExceptionTrapping()) {
	  throw VmeException(VmeException::BUS_ERROR, VME_BUSERROR, m_ports[ip]);
	}
      }
    }
  }
}

void RCCVmeInterface::busErrorReport(VmePort &port) {
  if (getBusErrors() != 0) throw VmeException(VmeException::BUS_ERROR, VME_BUSERROR, &port); 
}

std::string RCCVmeInterface::getErrorMessage(const long errcod) {
  char err[256];
  VME_ErrorString(errcod, err);
  return err;
}

long RCCVmeInterface::getBusErrors() {
  long be = m_busErrors;
  m_busErrors = 0;
  VME_BusErrorInfoGet(&m_busErrInfo);
  return be;
}
