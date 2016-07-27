///////////////////////////////////////////////////////////////////
// PixGPIBError.cxx
//   Implementation file for class PixGPIBError
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//  Version 00-00-01 21/11/2008 Daniel Dobos
///////////////////////////////////////////////////////////////////

#include "PixGPIBError.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

#if defined(USE_LINUX_GPIB)
#include "gpib/ib.h"
#else
#include "ni488.h"
#endif


using namespace std;

PixGPIBError::PixGPIBError(bool useStdout):m_useStdout(useStdout){
  string estr="";
  m_errMsg.str(estr);
}
////////////////////////
// printGPIBError()
////////////////////////
void PixGPIBError::printGPIBError(const char *msg) {

  (m_useStdout?cout:m_errMsg) << string(msg) << endl;

  (m_useStdout?cout:m_errMsg) << "* ibsta = 0x" << std::hex << ibsta << std::dec << " <";
  if (ibsta & ERR ) (m_useStdout?cout:m_errMsg) << " ERR";
  if (ibsta & TIMO) (m_useStdout?cout:m_errMsg) << " TIMO";
  if (ibsta & END ) (m_useStdout?cout:m_errMsg) << " END";
  if (ibsta & SRQI) (m_useStdout?cout:m_errMsg) << " SRQI";
  if (ibsta & RQS ) (m_useStdout?cout:m_errMsg) << " RQS";
  if (ibsta & CMPL) (m_useStdout?cout:m_errMsg) << " CMPL";
  if (ibsta & LOK ) (m_useStdout?cout:m_errMsg) << " LOK";
  if (ibsta & REM ) (m_useStdout?cout:m_errMsg) << " REM";
  if (ibsta & CIC ) (m_useStdout?cout:m_errMsg) << " CIC";
  if (ibsta & ATN ) (m_useStdout?cout:m_errMsg) << " ATN";
  if (ibsta & TACS) (m_useStdout?cout:m_errMsg) << " TACS";
  if (ibsta & LACS) (m_useStdout?cout:m_errMsg) << " LACS";
  if (ibsta & DTAS) (m_useStdout?cout:m_errMsg) << " DTAS";
  if (ibsta & DCAS) (m_useStdout?cout:m_errMsg) << " DCAS";
  (m_useStdout?cout:m_errMsg) << endl;

  (m_useStdout?cout:m_errMsg) << "* iberr = " << iberr << endl;
  if (iberr == EDVR) (m_useStdout?cout:m_errMsg) << " EDVR <Driver Error>" << endl;
  if (iberr == ECIC) (m_useStdout?cout:m_errMsg) << " ECIC <Not Controller-In-Charge>" << endl;
  if (iberr == ENOL) (m_useStdout?cout:m_errMsg) << " ENOL <No Listener>" << endl;
  if (iberr == EADR) (m_useStdout?cout:m_errMsg) << " EADR <Address error>" << endl;
  if (iberr == EARG) (m_useStdout?cout:m_errMsg) << " EARG <Invalid argument>" << endl;
  if (iberr == ESAC) (m_useStdout?cout:m_errMsg) << " ESAC <Not System Controller>" << endl;
  if (iberr == EABO) (m_useStdout?cout:m_errMsg) << " EABO <Operation aborted>" << endl;
  if (iberr == ENEB) (m_useStdout?cout:m_errMsg) << " ENEB <No GPIB board>" << endl;
  if (iberr == EDMA) (m_useStdout?cout:m_errMsg) << " EDMA <DMA Error>" << endl;   
  if (iberr == EOIP) (m_useStdout?cout:m_errMsg) << " EOIP <Async I/O in progress>" << endl;
  if (iberr == ECAP) (m_useStdout?cout:m_errMsg) << " ECAP <No capability>" << endl;
  if (iberr == EFSO) (m_useStdout?cout:m_errMsg) << " EFSO <File system error>" << endl;
  if (iberr == EBUS) (m_useStdout?cout:m_errMsg) << " EBUS <Command error>" << endl;
  if (iberr == ESRQ) (m_useStdout?cout:m_errMsg) << " ESRQ <SRQ stuck on>" << endl;
  if (iberr == ETAB) (m_useStdout?cout:m_errMsg) << " ETAB <Table Overflow>" << endl;
#if !defined(USE_LINUX_GPIB)
  if (iberr == ELCK) (m_useStdout?cout:m_errMsg) << " ELCK <Interface is locked>" << endl;
  if (iberr == EARM) (m_useStdout?cout:m_errMsg) << " EARM <ibnotify callback failed to rearm>" << endl;
  if (iberr == EHDL) (m_useStdout?cout:m_errMsg) << " EHDL <Input handle is invalid>" << endl;
  if (iberr == EWIP) (m_useStdout?cout:m_errMsg) << " EWIP <Wait in progress on specified input handle>" << endl;
  if (iberr == ERST) (m_useStdout?cout:m_errMsg) << " ERST <The event notification was cancelled due to a reset of the interface>" << endl;
  if (iberr == EPWR) (m_useStdout?cout:m_errMsg) << " EPWR <The interface lost power>" << endl;
  (m_useStdout?cout:m_errMsg) << "* ibcntl = " << ibcntl << endl;
#endif
}
void PixGPIBError::getError(string &errMsg){
  // return current content of error string and reset it afterwards
  // NB: if errors are always written to std.out, this will always be empty!
  errMsg = m_errMsg.str();
  string estr="";
  m_errMsg.str(estr);
  return;
}
