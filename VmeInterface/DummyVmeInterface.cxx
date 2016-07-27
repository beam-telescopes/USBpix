/////////////////////////////////////////////////////////////////////
//! DummyVmeInterface.cxx 
//! version 1.2
//!
//! Dummy VmeInterface implemetation 
//!                                 
/////////////////////////////////////////////////////////////////////
// 20/02/2002  V 1.0 PM - First implementation
// 19/02/2004  V 1.2 PM - Safe single word R/W methods added            
//

#include <signal.h>
#include <sys/types.h>

#include "DummyVmeInterface.h"

using namespace SctPixelRod;

unsigned char  DummyVmeInterface::m_softVectMask;             
long DummyVmeInterface::m_interruptData = 0;
DummyVmeInterface *DummyVmeInterface::m_if = NULL;
std::vector<VmeInterruptHandler *>  DummyVmeInterface::m_hardIH;   
std::vector<VmeInterruptHandler *>  DummyVmeInterface::m_softIH[32];  
                                                        
void DummyVmeInterface::m_interruptHandler(int signum) {
  static int is, smask;
  static unsigned int i;

  // Execute hard handlers
  m_softVectMask = 0;
  m_if->m_interruptData = 0;
  for (i=0; i<m_hardIH.size(); i++) {
    if (m_hardIH[i]->getActive()) {
      m_hardIH[i]->interruptHandler(0, 0, 0);
    }
  }

  // Execute soft handlers
  smask = m_softVectMask;
  for (is=0; is<32; is++) {
    if ((smask & (1 << is)) != 0) { 
      for (i=0; i<m_softIH[is].size(); i++) {
        if (m_softIH[is][i]->getActive()) {
          m_softIH[is][i]->interruptHandler(0, is, 0);
	}
      }
    }
  }
}

DummyVmeInterface::DummyVmeInterface() {
  int ret;
  // Install the global interrupt handler
  struct sigaction sa;
  sigemptyset(&sa.sa_mask); 
  sa.sa_flags = 0;
  sa.sa_handler = m_interruptHandler; 
  ret = sigaction(SIGRTMAX-1, &sa, NULL);
  // Pepare the mask for blocking interrupt handlers
  sigemptyset(&m_sigMask);
  sigaddset(&m_sigMask, SIGRTMAX-1);
  // Set the pointer to this object
  m_if = this;
};

DummyVmeInterface::~DummyVmeInterface() {
};

void DummyVmeInterface::declareInterruptHandler(VmeInterruptHandler &handler) {
  unsigned char softvec = handler.getInterruptSoftVect();
  if (softvec == 0) {   
    // Hard IH definition
    unsigned int i;
    for (i=0; i<m_hardIH.size(); i++) {
      if (&handler == m_hardIH[i]) return;
    }
    m_hardIH.push_back(&handler);
  } else if (softvec > 0 && softvec <= 32) {
    // Soft IH definition
    unsigned int i;
    for (i=0; i<m_softIH[softvec-1].size(); i++) {
      if (&handler == m_softIH[softvec-1][i]) return;
    }
    m_softIH[softvec-1].push_back(&handler);
  }
}

void DummyVmeInterface::removeInterruptHandler(VmeInterruptHandler &handler) {
  char softvec = handler.getInterruptSoftVect();
  std::vector<VmeInterruptHandler *>::iterator i;
  if (softvec == 0) {   
    // Basic IH cancellation
    for (i=m_hardIH.begin(); i != m_hardIH.end(); i++) {
      if (&handler == *i) {
        m_hardIH.erase(i);
        break;
      }
    }
  } else if (softvec > 0 && softvec <= 32) {
    // Slave IH cancellation
    for (i=m_softIH[softvec-1].begin(); i != m_softIH[softvec-1].end(); i++) {
      if (&handler == *i) {
        m_softIH[softvec-1].erase(i);
        return;
      }
    }
  }  
}

void DummyVmeInterface::cleanInterruptHandlers() {
}

void DummyVmeInterface::blockInterruptNotification() {
  sigprocmask(SIG_BLOCK, &m_sigMask, NULL);
} 

void DummyVmeInterface::resumeInterruptNotification() {
  sigprocmask(SIG_UNBLOCK, &m_sigMask, NULL);
}

unsigned char DummyVmeInterface::read8  (const unsigned long handle, const unsigned long offset) {
  return 0;
}  

unsigned char DummyVmeInterface::readS8  (const unsigned long handle, const unsigned long offset) {
  return 0;
}  

unsigned short DummyVmeInterface::read16 (const unsigned long handle, const unsigned long offset) {
  return 0;
}

unsigned short DummyVmeInterface::readS16 (const unsigned long handle, const unsigned long offset) {
  return 0;
}

unsigned long DummyVmeInterface::read32 (const unsigned long handle, const unsigned long offset) {
  return 0;
}

unsigned long DummyVmeInterface::readS32 (const unsigned long handle, const unsigned long offset) {
  return 0;
}

void DummyVmeInterface::write8 (const unsigned long handle, const unsigned long offset, const unsigned char value) {
}  

void DummyVmeInterface::writeS8 (const unsigned long handle, const unsigned long offset, const unsigned char value) {
}  

void DummyVmeInterface::write16(const unsigned long handle, const unsigned long offset, const unsigned short value) {
}    

void DummyVmeInterface::writeS16(const unsigned long handle, const unsigned long offset, const unsigned short value) {
}    

void DummyVmeInterface::write32(const unsigned long handle, const unsigned long offset, const unsigned long value) {
}   

void DummyVmeInterface::writeS32(const unsigned long handle, const unsigned long offset, const unsigned long value) {
}   

void DummyVmeInterface::blockRead32 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len) {
} 

void DummyVmeInterface::blockRead64 (VmePort &port, const unsigned long offset, unsigned long *buf, const long len) {
}

void DummyVmeInterface::blockWrite32(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len) {
}  

void DummyVmeInterface::blockWrite64(VmePort &port, const unsigned long offset, const unsigned long *buf, const long len) {
}   

unsigned long DummyVmeInterface::registerPort(VmePort &port) {
  unsigned int ip;
 
  for (ip=0; ip<m_ports.size(); ip++) {
    if (m_ports[ip] == NULL) {
      m_ports[ip] = &port;
      return ip;
    }
  }
  m_ports.push_back(&port);
  return m_ports.size()-1;
}         

void DummyVmeInterface::deletePort(VmePort &port) {
  unsigned int ip;

  for (ip=0; ip<m_ports.size(); ip++) {
    if (m_ports[ip] == &port) {
      m_ports[ip] = NULL;
    }
  }
}

//void *DummyVmeInterface::getPortMap(const unsigned long handle) {
//  return NULL;
//}
  
void DummyVmeInterface::busErrorReport() { 
}

void DummyVmeInterface::busErrorReport(const unsigned long handle) {
}

void DummyVmeInterface::busErrorReport(VmePort &port) {
}

std::string DummyVmeInterface::getErrorMessage(const long errcod) {
  return "";
}

long DummyVmeInterface::getBusErrors() {
  return 0;
}
