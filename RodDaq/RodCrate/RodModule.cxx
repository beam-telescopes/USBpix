//File: RodModule.cxx
// $Header$

#define R_BOGUS 0xFFFF

#include "RodModule.h"
#include <sys/time.h>
#include "BocCard.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//! Namespace for the common routines for SCT and PIXEL ROD software. 
namespace SctPixelRod {

// Flag to force endian-ness swap before calls to HPI registers.  This should be
// a temporary patch.

static long endianSwap = 0;
static const long rodInitTimeout = 10;

//******************************Class NoImageFile**************************
//
// Description:
//  This class is thrown if If we try to load a binary image for a master or 
//  slave DSP and the file is not found
//   
//  Author(s):
//    Tom Meyer (meyer@iastate.edu) - originator
//	Constructors. Use defaults for destructor, copy, and assignment.

NoImageFile::NoImageFile(std::string descriptor, std::string fileName) : BaseException(descriptor) {
  m_fileName = fileName;
  setType(NOIMAGEFILE);
  }
  
void NoImageFile::what(std::ostream& os) {
  os << "NoImageFile Exception. Descriptor, File name: " << getDescriptor() << ";" << m_fileName << std::endl;
}
  
//******************************Class HpiException**************************
//
// Description:
//  This class is thrown if an error in an HPI read/write operation is detected.
//   
//  Author(s):
//    Tom Meyer (meyer@iastate.edu) - originator

//	Constructors. Use defaults for destructor, copy, and assignment.

HpiException::HpiException( std::string descriptor, unsigned long calcAddr,
                            unsigned long readAddr) : BaseException(descriptor) {
  m_calcAddr = calcAddr;
  m_readAddr = readAddr;
  setType(HPI);
  }

void HpiException::what(std::ostream& os) {
    os << "HpiException: " << getDescriptor() << std::endl;
    os << "Calculated Address:" << std::hex << getCalcAddr() << std::endl;
    os << "Read Address:" << std::hex << getReadAddr() << std::endl;
}  
//***************************Class RodException**************************
//
// Description:
//  This class is thrown if an error in a ROD operation is detected.
//   
//  Author(s):
//    Tom Meyer (meyer@iastate.edu) - originator

//	Constructors. Use defaults for destructor, copy, and assignment.

RodException::RodException( std::string descriptor) : BaseException(descriptor) {
  m_numData = 0;
  m_data1 = m_data2 = 0;
  setType(ROD);
  }
RodException::RodException( std::string descriptor, unsigned long data1) :
                            BaseException(descriptor) {
  m_numData = 1;
  m_data1 = data1;
  m_data2 = 0;
  setType(ROD);
  }
RodException::RodException( std::string descriptor, unsigned long data1,
                            unsigned long data2) : BaseException(descriptor) {
  m_numData = 2;
  m_data1 = data1;
  m_data2 = data2;
  setType(ROD);
  }

void RodException::what(std::ostream& os) {
  unsigned long numData;
  numData = getNumData();
  os << "RodException: " << getDescriptor() << std::endl;
  if (0 == numData) return;
  os << "Data1:" << getData1() << std::endl;
  if (1 == numData) return;
  os << "Data2:" << getData2() << std::endl;
}  

//********************************Class RodModule***************************
//
// Description:
//  This is a derived class providing the software interface for VME ROD modules.
//   
//  Author(s):
//    Tom Meyer (meyer@iastate.edu) - originator

//---------------------------------Constructor------------------------------                                    
/*
This is the only constructor to use.
*/

RodModule::RodModule( unsigned long baseAddr, unsigned long mapSize, 
     VmeInterface & ourInterface, long numSlaves) throw (RodException&,
     VmeException&) :
     VmeModule(baseAddr, mapSize, ourInterface) {
  m_slot = baseAddr>>24;
  m_serialNumber = 0xFFFFFFFF;  // Will be overwritten during init
  m_revision = 0;               // Will be overwritten during init
  m_numSlaves = numSlaves; 
  m_finBufferSize = 4096; 
  m_masterImageName = std::string("");
  m_myOutList = 0;
  m_vmeCommandReg = 0;
  m_rodStatusReg[0] = 0;
  m_rodStatusReg[1] = 0;
  m_debug = 0;
  m_debugFile = "rodprimitives.bin";
  m_textData = 0;
  m_btFailure = false;

  try {
    if (m_mapSize > 0xc00000) {
      unsigned long nexcl = 3;
      unsigned long exclS[3] = {   0x1000, 0x201000, 0x800000 };
      unsigned long exclE[3] = { 0x1fffff, 0x3fffff, 0xbfffff };
      m_myVmePort = new VmePort(m_baseAddress, m_mapSize, nexcl, exclS, exclE, 
				VmeInterface::A32, m_ourInterface); 
    } else {
      m_myVmePort = new VmePort(m_baseAddress, m_mapSize, VmeInterface::A32, 
				m_ourInterface); 
    }
  }
  catch (std::bad_alloc & ba) {
    throw RodException("Failed to get VME Port.");
  }

  m_myOutList = 0;

  memset(&m_masterXface, 0, sizeof(m_masterXface));

  m_myPrimState = RodPrimList::IDLE;

  // Make the vme library generate exceptions on bus errors
  getVmePort()->setExceptionTrapping(true);

  m_myBoc = NULL;
  if (bocCheck()==1) {
    //m_myBoc = new BocCard(*this);
  }
}

//---------------------------------Destructor------------------------------                                    
/*
Be sure to delete all VME Ports and other "new" things in constructor.
*/

RodModule::~RodModule() {
  delete(m_myVmePort); m_myVmePort = 0;
  if (m_myBoc != NULL) delete m_myBoc; 
    
  if (m_textData){ 
      for (unsigned int i=0; i< n_textbuffers; i++) {
          if (m_textData[i]) delete [](m_textData[i]);
      }  
      delete []m_textData;
      m_textData=NULL;
  }
  return;
}

//  Member methods


//--------------------------------initialize---------------------------------                                 

/* This method performs all actions necessary to place a properly loaded ROD 
in its initialized state, except for initializing the slave DSPs. This must be done
separately using initSlaveDsp().

Activities:
    Reset the ROD
    Read ROD serial number
    Load 0x000l000l into Master DSP HPIC register. (Says low order 16-bit word
                                                    comes first)
    Read back HPIC to verify
    Load MDSP memory map
    Retrieve pointers to text buffer structs
    Initialize PrimList and TextBuff state variables
*/
void RodModule::initialize() {initialize(false); return;};

void RodModule::initialize(bool resetFlag) throw (RodException &, VmeException &) {

  unsigned long hpicReadback;   // for debugging
  unsigned long mdspStart;
  

  std::cout << "RodModule::initialize" << std::endl;

  
  int i_count = 0;
  bool i_done = false;
  while(!i_done) {
    i_count++;
    // If reset flag is set reset the ROD so it starts in a known state
    if (resetFlag) {
      int r_count = 0;
      bool r_done = false;
      while (!r_done) {
	try {
	  r_count++;
	  reset();
	  r_done = true;
	}
	catch (RodException &r) {
	  if (r_count > 2) {
	    throw r;
	  }
	  std::cout << "S" << m_slot << " retry ROD reset (count = " << r_count << ")" << std::endl;
	}
      }
      // Wait an extra half a second
      sleep(500);
    }
    
    // Read ROD serial number 
    m_serialNumber = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[6]);
    if (endianSwap) m_serialNumber = endianReverse32(m_serialNumber); // Temporary (I hope)
    m_revision = (m_serialNumber&0x00ff0000)>>16;
    m_serialNumber = m_serialNumber & 0x3ff;   // Mask to get 10 least significant bits
    
    //	Initialize Master HPIC register to set half-word order
    
    unsigned long hpicValue = 0x00010001;
    
    hpiLoad(HPIC, hpicValue);
    hpicReadback = hpiFetch(HPIC);         // for debugging
    sleep(100);
    
    mdspStart = mdspSingleRead(MEMORYMAP_REG);
    std::cout << "S" << m_slot << " reading MEMORYMAP_REG: " << std::hex << MEMORYMAP_REG 
	      << " start " << std::hex << mdspStart << std::endl;
    
    // read interface structure
    mdspBlockRead(mdspStart, (long unsigned int*)&m_masterXface, 
		  sizeof(MasterXface)/4);

    if (m_masterXface.xface.nTextBuffers > 10) {
      if (i_count > 2) {
	throw RodException("Invalid number of text buffers returned by the ROD, aborting", m_slot);
      } else {
	resetFlag = true;
	std::cout << "S" << m_slot << " retry ROD initialize (count = " << i_count << ")" << std::endl;
        continue;
      }
    }
    std::cout << "S" << m_slot << " nTextBuffers: " << m_masterXface.xface.nTextBuffers 
	      << std::endl;
    std::cout << "S" << m_slot << " nTextBufferSlots: " << m_masterXface.xface.nTextBufferSlots 
	      << std::endl;
    
    // number of textbuffers per master and 4 slaves
    n_textbuffers = m_masterXface.xface.nTextBuffers*m_masterXface.xface.nTextBufferSlots;
    
    m_textBuffer     = new UINT32[n_textbuffers];
    m_textBufferSize = new UINT32[n_textbuffers];
    
    // read pointers to text buffer queue structures
    mdspBlockRead(m_masterXface.xface.textBuffer, 
		  (long unsigned int*)m_textBuffer, 
		  n_textbuffers);
    
    TextQueue textQueue;
    if(m_textData){
      for (unsigned int i=0; i< n_textbuffers; i++) {
	if (m_textData[i]) delete [](m_textData[i]);
      }  
      delete []m_textData;
      m_textData=NULL;
    }
    m_textData = new char*[n_textbuffers];
    for (unsigned int i=0; i < n_textbuffers; i++) m_textData[i] = 0;
    
    for (unsigned int i=0; i< n_textbuffers; i++) {
      mdspBlockRead(m_textBuffer[i], (long unsigned int*)&textQueue, 
		    sizeof(TextQueue)/4);
      m_textBufferSize[i] = textQueue.length;
      m_textData[i] = new char[sizeof(UINT32)*m_textBufferSize[i]];
      std::cout << "S" << m_slot << " RodModule TextBuffer " << i << " size: " << m_textBufferSize[i] << std::endl;
    }
    i_done = true;
  }
  
  //  Reinitialize state variables
  m_myPrimState = RodPrimList::IDLE;

  std::cout << "S" << m_slot << " RodModule::initialized finished" << std::endl;
return;
}

bool RodModule::isSane() { 
  bool sane = false;

  unsigned long fpgaStatus6 = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[6]);
  unsigned long maskedCor   = fpgaStatus6 & 0xff000000; // correct byte order
  unsigned long maskedRev   = fpgaStatus6 & 0x000000ff; // wrong byte order
  sane = ((maskedCor == 0xad000000) || (maskedRev == 0x000000ad));
 
  return sane;
}

unsigned long RodModule::fmtVersion(){
  unsigned long version = mdspSingleRead(0x00400088);
  return ((version & 0xFFF)<<4) + ((version & 0xf00000)>>20);
}
  
unsigned long RodModule::efbVersion(){
  unsigned long version = mdspSingleRead(0x0040221C);
  return (version & 0x00008FFF);
}
  
unsigned long RodModule::rtrVersion(){
  unsigned long version = mdspSingleRead(0x0040250C);
  return (version & 0x00008FFF);
}

unsigned long RodModule::rcfVersion(){
  unsigned long version = mdspSingleRead(0x0040440C);
  return (version & 0x00000FFF);
}


//---------------------------------reset------------------------------------                                
/* */
/* This method resets the ROD. */
/* */

void RodModule::reset() throw (RodException&, VmeException &) {
  // Reset all FPGAs and DSPs (bit 6 = 0x40)
  const unsigned long rodResetValue = 0x00000040;
  unsigned long rodReset;
  unsigned long rodRegValue, dspRegValue;
  clock_t startTime;
  unsigned int count;
  
  std::cout << "S" << m_slot << " RodModule::reset " << std::endl;
  if (endianSwap) {
    rodReset = endianReverse32(rodResetValue);
  }
  else {
    rodReset = rodResetValue;
  }

  // switch to little endian
  unsigned long hpicValue = 0x00010001;
  hpiLoad(HPIC, hpicValue);

  // mark MDSP as in unknown state
  mdspSingleWrite(MAGIC_LOCATION, 0xDEADDEAD);
  std::cout << "S" << m_slot << " RodModule::reset MAGIC_LOCATION" << std::hex 
       << MAGIC_LOCATION << std::endl;

 // reset all
  m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[2], rodReset);
  sleep(1000);

  std::cout << "S" << m_slot << " RodMOdule::reset RESET" << std::endl;
  // check  for finish of FPGA reset  
  startTime = time(0);
  count = 0;
  do {
    rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[0]);
    if ((time(0)-startTime) > 5 && count > 10) {
      throw RodException("FPGA reset timeout in reset(), DSP_RESET_TIMEOUT=5", m_slot);
    }
    count++;
    // Check that reset has finished
    // leave if PRM configuration mode active
  } while (((rodRegValue&0x3f) != FPGA_RESET_COMPLETE) && 
	   ((rodRegValue&0x20) != 0x20));

  // check  for finish of DSP reset  
  rodRegValue = 0;
  startTime = time(0);
  count = 0;
  do {
    rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[2]);
    if ((time(0)-startTime) > 5 && count > 10) {
      throw RodException("DSP reset timeout in reset(), DSP_RESET_TIMEOUT=5", m_slot);
    }
    count++;
  }
  while (rodRegValue != ALLDSP_RESET_COMPLETE);
   
  // ensure little endian DSP access via HostPortInterface 
  hpiLoad(HPIC, hpicValue);

 // wait for MDSP boot indication
  startTime = time(0);
  count = 0;
  do {
    dspRegValue = mdspSingleRead(MAGIC_LOCATION);
    if ((time(0)-startTime) > 15 && count > 10) {
      throw RodException("MDSP boot timeout in reset(), DSP_RESET_TIMEOUT=15", m_slot);
    }  
    count++; 
  } while(dspRegValue != I_AM_ALIVE);

  //  Reinitialize state variables
  m_myPrimState = RodPrimList::IDLE;
  
return;
}

//---------------------------------fpgaReset------------------------------------                                

/*! This method resets individual FPGAs on the ROD. The ROD can reset only 
    one FPGA or else all FPGAs at a time.
 */
void RodModule::fpgaReset(FpgaType targetFpga) throw (RodException&, VmeException &) {
  /* Reset one FPGA via FPGA Reset Control Register (0xC00004)
     The values for the targetFpga are set in an enum called FpgaType. The values are:
       0 = Controller 
       1 = Formatter
       2 = Formatter
       3 = EFB
       4 = Router
       5 = All
  */
  unsigned long resetValue;
  unsigned long reset;
  unsigned long rodRegValue;
  clock_t startTime;
  
if ((targetFpga != 0) && (targetFpga != 1) && (targetFpga != 2) && (targetFpga != 3) &&
      (targetFpga != 4) && (targetFpga != 5)) {
    std::cout << "Invalid argument to fpgaReset. Only one bit can be set." << std::endl;
    return;
  }
  resetValue = 0x00000001<<targetFpga;
  std::cout << "RodModule::fpgaReset " << std::endl;
  if (endianSwap) {
    reset = endianReverse32(resetValue);
  }
  else {
    reset = resetValue;
  }

  // Set the reset bit (it is self-clearing)
  m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[1], reset);
  
  // check  for finish of FPGA reset. The FPGA Reset Status Register is at VME 0xc00024.
  startTime = time(0);
  do {
    rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[1]);
    if ((time(0)-startTime) > DSP_RESET_TIMEOUT) {
      throw RodException("FPGA reset timeout in reset(), DSP_RESET_TIMEOUT=", 
			 DSP_RESET_TIMEOUT);
    }

  // leave if PRM configuration mode active (check bit 5 in FPGA Configuration Status
  // Register at 0xc00020.
  } while (((rodRegValue&0x3f) != FPGA_RESET_COMPLETE) && 
	   ((rodRegValue&0x20) != 0x20));

  //  Reinitialize state variables
  m_myPrimState = RodPrimList::IDLE;
  
return;
}

//---------------------------------dspReset------------------------------------ 
                              
/*! This method resets individual DSPs on the ROD. The ROD can reset only one DSP
    or else all DSPs at a time.
 */
void RodModule::dspReset(DspType targetDsp) throw (RodException&, VmeException &) {
  /* Reset one DSP via DSP Reset Control Register (0xC00008)
     The values for the targetDsp are set in an enum called DspType. The values are::
       1 = Master 
       2 = Slave 0
       3 = Slave 1
       4 = Slave 2
       5 = Slave 3
  */
  unsigned long resetValue;
  unsigned long reset;
  unsigned long rodRegValue;
  clock_t startTime;
  
  if ((targetDsp != 0) && (targetDsp != 1) && (targetDsp != 2) && (targetDsp != 3) &&
      (targetDsp != 4) && (targetDsp != 5)) {
    std::cout << "Invalid argument to dspReset. Only one bit can be set." << std::endl;
    return;
  }
  resetValue = 0x00000001<<targetDsp;
  std::cout << "RodModule::fpgaReset " << std::endl;
  if (endianSwap) {
    reset = endianReverse32(resetValue);
  }
  else {
    reset = resetValue;
  }

  // Set the reset bit (it is self-clearing)
  m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[2], reset);
  
  // check  for finish of DSP reset.  The DSP Reset Status Register is at VME 0xc00028.
  rodRegValue = 0;
  startTime = clock();
  do {
    rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[2]);
    if ((clock()-startTime)/CLOCKS_PER_SEC > DSP_RESET_TIMEOUT) {
      throw RodException("DSP reset timeout in reset(), DSP_RESET_TIMEOUT=",
      (long)DSP_RESET_TIMEOUT);
    }
  }
  while (rodRegValue != ALLDSP_RESET_COMPLETE);
   
  //  Reinitialize state variables
  m_myPrimState = RodPrimList::IDLE;
  
return;
}

//----------------------------------verify----------------------------------                                

/* This method checks the value of the h.o. byte of the 
*/

bool RodModule::verify() {
  unsigned long regVal;
  
// Read status register 6 
  regVal = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[6]);
  if (endianSwap) regVal = endianReverse32(regVal); 
  regVal = regVal & 0xff000000;            // Mask to get most significant byte
  if (regVal!=0xad000000) return false;
  return true;
}

//---------------------------------status-----------------------------------                                

/* This method reports the status of the ROD.
For now, it simply prints to standard output.
*/
void RodModule::status() {
    std::cout << "Slot: "                   << m_slot;
    std::cout << "; Serial Number:"         << m_serialNumber;
    std::cout << "; Number of slave DSPs: " << m_numSlaves;
    std::cout << std::endl;

    std::cout << std::endl;
    
    std::dec(std::cout);
    std::cout << "Primitive state: " << getPrimState() ;
    std::cout << std::endl;
    return;
}


// based on Joes routine
int RodModule::loadAndStartSlaves(const std::string & fileName, const int slaveMask) 
  throw (NoImageFile&, RodException&, VmeException &) {
  int size, xferSize, nWords, islaveMask;
  DldHeader dldHeader;
  FILE *fp;
  char *ptr;
  
  islaveMask = slaveMask;
  if(islaveMask == 0) islaveMask = 0xf; /* default = all dsps */

/* first initialize the dsp after the hardware reset */
  SlaveCtrlIn slaveCtrlInI ;
  slaveCtrlInI.flag = SLAVE_INIT;
  slaveCtrlInI.slaveMask = islaveMask;

  RodPrimitive *slvRWPrimI = new RodPrimitive(sizeof(SlaveCtrlIn)/sizeof(UINT32),
					      1, SLAVE_CTRL, R_BOGUS, (long int*)&slaveCtrlInI);
  executeMasterPrimitiveSync(*slvRWPrimI);
  delete slvRWPrimI;

  RodPrimList *primList = new RodPrimList();
/* the largest primitive we can send */
  UINT32 accMax = m_masterXface.xface.requestChannelLen[0] * sizeof(UINT32); // jsv words or bytes
  if((fp = fopen(fileName.c_str(), "rb")) == NULL) {
    throw NoImageFile("NoImageFile thrown from loadAndStartSlaves", fileName);
    return -1; /* no such file */
  }
/* load the code into the slaves */

  UINT32 acc = 0; 
  int nsections = 0;
  while(fread(&dldHeader, sizeof(DldHeader), 1, fp)) {
    size     = dldHeader.length;
    //AKDEBUG    std::cout << " Header " << size << std::endl;
   xferSize = size + sizeof(dldHeader) + sizeof(SlaveCtrlIn);
    /* accumulative length of what's going into current primitive list */
    acc += xferSize;
    if(acc > accMax) {
      if(nsections>0){
	//AKDEBUG check and clean memory usage ....
 	sendPrimList(primList);       /* send what we've got thus far */
	RodPrimList::iterator it = primList->begin();
	for(;it != primList->end(); it++) free(it->getBody());
	delete primList;
	primList = new RodPrimList(); /*start this guy out from scratch*/
	acc = xferSize;
      }else{
	throw NoImageFile("ImageFile to large/bad format thrown from loadAndStartSlaves", fileName);
	return -1; /* bad file */	
      }
    }
    nWords = (xferSize + 3) /sizeof(UINT32) ;
    SlaveCtrlIn *slaveCtrlInL = (SlaveCtrlIn *) malloc(nWords*sizeof(UINT32));
    slaveCtrlInL->flag      = LOAD_COFF;
    slaveCtrlInL->slaveMask = islaveMask;
    slaveCtrlInL->nSections = 1;
    nsections++;
    /* go to the end of the structure */
    ptr = (char*)&slaveCtrlInL[1];
    memcpy(ptr, &dldHeader, sizeof(dldHeader)); /* read dld header into buffer */
    ptr += sizeof(dldHeader);  
    fread(ptr, size, 1, fp); /* read dld data into buffer */
    
    RodPrimitive *slvCtrlPrimL = 
      new RodPrimitive(nWords, 1, SLAVE_CTRL, R_BOGUS, (long int *)slaveCtrlInL);

    primList->push_back(*slvCtrlPrimL);
    delete slvCtrlPrimL;
    //free(slaveCtrlInL);
  }

  fclose(fp);

  std::cout << "nsections: " << nsections
       << " primListsize: " << primList->size()
       << " combined size: " << acc << std::endl;
  sendPrimList(primList);       /* send what we've got thus far */
  waitPrimitiveCompletion();
  RodPrimList::iterator it = primList->begin();
  for(;it != primList->end(); it++) free(it->getBody());
  
  delete primList;

  /* start the slaves */

  SlaveCtrlIn slaveCtrlInS;
  slaveCtrlInS.flag      = LOAD_DONE;
  slaveCtrlInS.slaveMask = islaveMask;
  RodPrimitive *slvRWPrimS = 
    new RodPrimitive(sizeof(SlaveCtrlIn)/sizeof(UINT32), 1, 
		     SLAVE_CTRL, R_BOGUS, (long int *)&slaveCtrlInS);
  executeMasterPrimitiveSync(*slvRWPrimS);
  delete slvRWPrimS;

/* wait for them to boot */
  
  SlaveCtrlIn slaveCtrlInB;
  slaveCtrlInB.flag      = WAIT_BOOT;
  slaveCtrlInB.slaveMask = islaveMask;
  RodPrimitive *slvRWPrimB = 
    new RodPrimitive(sizeof(SlaveCtrlIn)/sizeof(UINT32), 1, 
		     SLAVE_CTRL, R_BOGUS, (long int *)&slaveCtrlInB);
  executeMasterPrimitiveSync(*slvRWPrimB);
  delete slvRWPrimB;

  return 0;
}


// modified function used for IBL-ROD slaves - still needed?
int RodModule::loadAndStartFpgaSlaves(const std::string & fileName, const int slaveMask)
  throw (NoImageFile&, RodException&, VmeException &) {
  int nWords, islaveMask;
  UINT32 size, offs;
  FILE *fp;
  islaveMask = 0 == slaveMask ? 0xf : slaveMask; // default to all if 0
  for (int mask = 1; mask < 1 << m_numSlaves; mask <<= 1){
	  if (0 == (mask & islaveMask)) continue;

/* first initialize the dsp after the hardware reset */
  SlaveCtrlIn slaveCtrlInI ;
  slaveCtrlInI.flag = SLAVE_INIT;
  slaveCtrlInI.slaveMask = mask;

  RodPrimitive *slvRWPrimI = new RodPrimitive(sizeof(SlaveCtrlIn)/sizeof(UINT32),
					      1, SLAVE_CTRL, R_BOGUS, (long int*)&slaveCtrlInI);
  executeMasterPrimitiveSync(*slvRWPrimI);
  delete slvRWPrimI;

	// AKU: test for NULL file
	if (fileName.empty()){
		throw NoImageFile("Missing file name thrown from loadAndStartSlaves", fileName);
		return -1; /* no such file */
	}

	/* the largest primitive we can send */
	if((fp = fopen(fileName.c_str(), "r1")) == NULL) {
		throw NoImageFile("NoImageFile thrown from loadAndStartSlaves", fileName);
		return -1; /* no such file */
	}
	/* load the code into the slaves */

	// AKU: the file is raw binary . no dld headers etc
	struct stat fileStat;
	fstat(fileno(fp), &fileStat);
	size = fileStat.st_size;
	if (0 == size) {
		throw NoImageFile("No data in ImageFile", fileName);
		fclose(fp);
		return -1; /* bad file */
	}
	// read and load file in portions
	const UINT32 maxBlkSize = 1024; // must be power of 2
	UINT32 accMax = m_masterXface.xface.requestChannelLen[0] * sizeof(UINT32); // jsv words or bytes
	UINT32 blkSize = maxBlkSize < accMax ? maxBlkSize : accMax;
	static unsigned char fBuf[sizeof(SlaveCtrlIn) + sizeof(DldHeader) + maxBlkSize];
	SlaveCtrlIn *svlCtrlIn = (SlaveCtrlIn*)&fBuf[0];
	DldHeader *dldHeader = (DldHeader*)&svlCtrlIn[1];
	UINT32 *data = (UINT32*)&dldHeader[1];

	RodPrimList *primList = new RodPrimList();

	offs = 0;
	while (offs < size){
		fread(data, blkSize, 1, fp);
		dldHeader->length = size - offs > blkSize ? blkSize : size - offs;	// set size: max is blkSize
		dldHeader->addr = offs;
		dldHeader->fInternal = 0;
		svlCtrlIn->flag      = LOAD_COFF;
		svlCtrlIn->slaveMask = mask;
		svlCtrlIn->nSections = 1; // only 1 section always

		nWords = (dldHeader->length + sizeof(DldHeader) + sizeof(SlaveCtrlIn)) / sizeof(long); //  + 3 ????

		RodPrimitive *slvCtrlPrimL =
		  new RodPrimitive(nWords, 1, SLAVE_CTRL, R_BOGUS, (long int *)svlCtrlIn);

		primList->clear();
		primList->insert(primList->begin(), *slvCtrlPrimL);
		sendPrimList(primList);       /* send what we've got thus far */
		waitPrimitiveCompletion();

		delete slvCtrlPrimL;

		offs += dldHeader->length;
	}
	fclose(fp);
	delete primList;

	std::cout << "File loaded" << std::endl;

		startSlave(mask);

	}
  return 0;
}

int RodModule::startSlave(const int mask) {

	/* start the slave */

	SlaveCtrlIn slaveCtrlInS;
	slaveCtrlInS.flag      = LOAD_DONE;
	slaveCtrlInS.slaveMask = mask;
	RodPrimitive *slvRWPrimS =
	new RodPrimitive(sizeof(SlaveCtrlIn)/sizeof(UINT32), 1,
		 SLAVE_CTRL, R_BOGUS, (long int *)&slaveCtrlInS);
	std::cout << "Next: slave start" << std::endl;
	executeMasterPrimitiveSync(*slvRWPrimS);
	delete slvRWPrimS;

	/* wait for boot */

	SlaveCtrlIn slaveCtrlInB;
	slaveCtrlInB.flag      = WAIT_BOOT;
	slaveCtrlInB.slaveMask = mask;
	RodPrimitive *slvRWPrimB =
	new RodPrimitive(sizeof(SlaveCtrlIn)/sizeof(UINT32), 1,
		 SLAVE_CTRL, R_BOGUS, (long int *)&slaveCtrlInB);
	std::cout << "Next: slave boot" << std::endl;
	executeMasterPrimitiveSync(*slvRWPrimB);
	delete slvRWPrimB;

	return 0;
}


//------------------------------loadSlaveImage-----------------------------                       
void RodModule::loadSlaveImage(const std::string & fileName, const unsigned long address,
     const long slaveNum, char opt) throw (NoImageFile&, RodException&, 
                                               VmeException &) {
  
  long numWords;
  unsigned long readBack;   
  int fileSize, i;
  const unsigned long DSP_BUSY_MASK=0x00000002;
  std::ifstream fin;
// Cannot load image if slave is running.  Check it and throw a rodException if it is
  readBack = slvHpiFetch(SLAVE_HPIC_BASE, slaveNum);
  if (readBack & DSP_BUSY_MASK) throw RodException(
     "loadSlaveImage tried to load to a running SDSP. slavenum = ", slaveNum);
     
  fin.open(fileName.c_str(),std::ios::binary);
  if (fin.is_open()) {

// Get size of file
    fin.seekg(0, std::ios::end);  // go to end of file
    fileSize = fin.tellg();  // file size is current location
    fin.seekg(0, std::ios::beg);  // go back to beginning of file

// Create buffer and fill it with file contents
    char * buffer; 
    try {
      buffer = new char[fileSize];
    }
    catch (std::bad_alloc & ba) {
      throw RodException(
                  "primLoadSlaveImage buffer failed to allocate. buffer, fileSize:",
                  (unsigned long)buffer, (unsigned long)fileSize);
    }
    fin.read(buffer, fileSize);

// Use RW_SLAVE_MEMORY to transfer buffer into slave DSP memory
    long* writeSlvBuff;
    numWords = (fileSize+3)/sizeof(UINT32);
    writeSlvBuff = new long[numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32)];
    RwSlaveMemoryIn* WSlaveMemoryIn = (RwSlaveMemoryIn*)writeSlvBuff; 
    WSlaveMemoryIn->slave         = slaveNum;
    WSlaveMemoryIn->flags         = RW_WRITE;
    WSlaveMemoryIn->slaveAddr     = (UINT32)address;
    WSlaveMemoryIn->masterAddr    = (UINT32)NULL; 
    WSlaveMemoryIn->nWords        = numWords;
    long tempWord;
    for (i = 0; i< numWords; i++) {
      tempWord = 0;
      tempWord = tempWord | (buffer[(i)*4+3]&0xff)<<24;
      tempWord = tempWord | (buffer[(i)*4+2]&0xff)<<16;
      tempWord = tempWord | (buffer[(i)*4+1]&0xff)<<8;
      tempWord = tempWord | (buffer[(i)*4]&0xff);
      writeSlvBuff[i + sizeof(RwSlaveMemoryIn)/sizeof(UINT32)] = tempWord;
    }
    RodPrimitive* slvRWPrim;
    slvRWPrim = new RodPrimitive(numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 
				 1, RW_SLAVE_MEMORY, R_BOGUS, writeSlvBuff);
    synchSendPrim(*slvRWPrim);
    delete [] writeSlvBuff;
    delete slvRWPrim;
    
// Retrieve output buffer, if there is one
    RodOutList* outList = getOutList();
    if (outList) {
/*      UINT32 outListLength = UINT32(outList->getLength());
      unsigned long* outBody = outList->getBody();
      UINT32 outLength       = UINT32(outBody[0]);
      UINT32 outIndex        = UINT32(outBody[1]);
      UINT32 outNumPrims     = outBody[2];
      UINT32 outPrimVersion  = outBody[3];
      UINT32 primLength      = outBody[4];
      UINT32 primIndex       = outBody[5];
      UINT32 primId          = outBody[6];
      UINT32 primVersion     = outBody[7];
*/
      deleteOutList();
    }

    
// Read back buffer for verification if 'v' option was given
    if (opt=='v') {
      char* buffer2;
      try {
        buffer2 = new char[fileSize];
      }
      catch (std::bad_alloc & ba) {
        throw RodException(
                  "loadSlaveImage buffer2 failed to allocate. buffer, fileSize:",
                  (unsigned long)buffer, (unsigned long)fileSize);
      }
      long * readSlvBuff;
      readSlvBuff = new long[numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32)];
      RwSlaveMemoryIn* RSlaveMemoryIn  = (RwSlaveMemoryIn*)readSlvBuff;
      RSlaveMemoryIn->slave      = slaveNum;
      RSlaveMemoryIn->flags      = RW_READ;
      RSlaveMemoryIn->slaveAddr  = (UINT32)address;
      RSlaveMemoryIn->masterAddr = (UINT32)NULL ;
      RSlaveMemoryIn->nWords     = numWords;
      slvRWPrim = new RodPrimitive(numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 
				   1, RW_SLAVE_MEMORY, R_BOGUS, readSlvBuff);
      synchSendPrim(*slvRWPrim);
      delete [] readSlvBuff;
      
// Retrieve output buffer - there should be only one primitive in the reply list
      RodOutList* outList = getOutList();
      if (outList) {
//        UINT32 outListLength = UINT32(outList->getLength());
        unsigned long* outBody = outList->getBody();
        UINT32 outLength = UINT32(outBody[0]);
//        UINT32 outIndex = UINT32(outBody[1]);
//        UINT32 outNumPrims = outBody[2];
//        UINT32 outPrimVersion = outBody[3];
//        UINT32 primLength = outBody[4];
//        UINT32 primIndex = outBody[5];
//        UINT32 primId = outBody[6];
//        UINT32 primVersion = outBody[7];
        int fileSize2 = (outLength-15)*4;
        if (fileSize2 != fileSize) throw RodException("primLoadSlaveImage read back wrong file size; expected, got =", 
          fileSize, fileSize2);
        char tempChar0, tempChar1, tempChar2, tempChar3;
        for (int i=0; i<((fileSize+3)/4); i++) {
          tempWord = outBody[i+8];
          tempChar0 = tempWord & 0x000000ff;
          tempChar1 = (tempWord & 0x0000ff00)>>8;
          tempChar2 = (tempWord & 0x00ff0000)>>16;
          tempChar3 = (tempWord & 0xff000000)>>24;
          buffer2[4*i] = tempChar0; 
          buffer2[4*i+1] = tempChar1;
          buffer2[4*i+2] = tempChar2;
          buffer2[4*i+3] = tempChar3;
        }
        for (int i=0; i<fileSize; i++) {
          if (buffer[i] != buffer2[i]) {
            deleteOutList();
            delete [] buffer;
            delete [] buffer2;
            delete slvRWPrim;
            fin.close();
            char message[] = "primLoadSlaveImage buffer readback invalid for file, slavenum, char index: ";
            throw RodException( message+fileName, slaveNum, i);
          }
        } 
        deleteOutList();
        delete [] buffer2;
        delete slvRWPrim;
        std::cout << "Image verification check complete\n";
      }
    }
// Close the file and delete the buffer    
    fin.close();
    delete [] buffer;
  }
  else {
    throw NoImageFile("NoImageFile thrown from primLoadSlaveImage", fileName);
  }
  return;
}

//--------------------------------hpiLoadSlaveImage-------------------------------                       
void RodModule::hpiLoadSlaveImage(const std::string & fileName, const unsigned long address,
     const long slaveNum, char opt) throw (NoImageFile&, RodException&, 
                                               VmeException &) {
  
  long numWords;
  unsigned long readBack;   
  int fileSize;
  const unsigned long DSP_BUSY_MASK=0x00000002;
  std::ifstream fin;
// Cannot load image if slave is running.  Check it and throw a rodException if it is
  readBack = slvHpiFetch(SLAVE_HPIC_BASE, slaveNum);
  if (readBack & DSP_BUSY_MASK) throw RodException(
     "loadSlaveImage tried to load to a running SDSP. slavenum = ", slaveNum);
     
  fin.open(fileName.c_str(),std::ios::binary);
  if (fin.is_open()) {

// Get size of file
    fin.seekg(0, std::ios::end);  // go to end of file
    fileSize = fin.tellg();  // file size is current location
    fin.seekg(0, std::ios::beg);  // go back to beginning of file

// Create buffer and fill it with file contents
    char * buffer; 
    try {
      buffer = new char[fileSize];
    }
    catch (std::bad_alloc & ba) {
      throw RodException(
                  "loadSlaveImage buffer failed to allocate. buffer, fileSize:",
                  (unsigned long)buffer, (unsigned long)fileSize);
    }
    fin.read(buffer, fileSize);

// Use slvHpiBlockWrite to transfer buffer into memory
    numWords = (fileSize+3)/4;
    slvHpiBlockWrite(address, (unsigned long*) buffer, numWords, slaveNum);
    
// Read back buffer for verification if 'v' option was given
    if (opt=='v') {
      char* buffer2;
      try {
        buffer2 = new char[fileSize];
      }
      catch (std::bad_alloc & ba) {
        throw RodException(
                  "HpiLoadSlaveImage buffer2 failed to allocate. buffer, fileSize:",
                  (unsigned long)buffer, (unsigned long)fileSize);
      }
      slvHpiBlockRead(address, (unsigned long*)buffer2, numWords, slaveNum);
   
      for (int i=0; i<fileSize-1; i++) {
        if (buffer[i] != buffer2[i]) {
          delete [] buffer;
          delete [] buffer2;
          fin.close();
          char message[] = "HpiLoadSlaveImage buffer readback invalid for file, slavenum, char index: ";
          throw RodException( message+fileName, slaveNum, i);
        }
      }
      delete [] buffer2;
      std::cout << "Image verification check complete\n";
    }
    
// Close the file and delete the buffer    
    fin.close();
    delete [] buffer;
  }
  else {
    throw NoImageFile("NoImageFile thrown from loadSlaveImage", fileName);
  }
  return;
}

/*---------------------------------synchSendPrim------------------------------                           
This version writes to cout. This needs to be changed.
*/
void RodModule::synchSendPrim(RodPrimitive & prim) throw (RodException&, 
                                                         VmeException &) {
  RodPrimList primList(1);  // Private primList (local to this routine)
  RodPrimList::PrimState returnPState;

// Create an error text buffer in case of error message
    primList.insert(primList.begin(), prim);
    try {
      primList.bufferBuild();
    }
    catch (PrimListException &p) {
//      throw (RodException(p.getDescriptor,p.getData1(), p.getData2());
      std::cout << p.getDescriptor() << " ";
      std::cout << p.getData1() << ", " << p.getData2() << "\n";
    };
    try {
      this->sendPrimList(&primList);
    }
    catch (BaseException &h) {
      std::cout << h;
    };

    do {
       try {
         returnPState = this->primHandler();
       }
       catch (RodException &r) {
       std::cout << r.getDescriptor() <<", " << r.getData1() << ", " << r.getData2()
               << '\n';
       }
    } while (returnPState != RodPrimList::WAITING && 
	     returnPState != RodPrimList::IDLE); 

  return;
}

//---------------------------------sendPrimList-------------------------------                           
void RodModule::sendPrimList(RodPrimList *plist) 
  throw(PrimListException &, VmeException &) {

  //  char filename[256]; // AK
  FILE *fp; // AK
  //static int jsv_index2 = 0;   //AK
  unsigned long * buffAddr;
  unsigned long statusword; 
  long buffCount;
  bool timeout = false;
  clock_t startTime;
  unsigned int count = 0;

  if (!plist->getBuffer()) {
    plist->bufferBuild();
  };

  buffCount = plist->getBufferLength();

    
  if (((unsigned int)abs(buffCount)) > m_masterXface.xface.requestChannelLen[0]) {
    throw PrimListException("PrimList is bigger than DSP buffer", buffCount, 
			    m_masterXface.xface.requestChannelLen[0]);
  }
  buffAddr = plist->getBuffer();

  startTime = clock();
  count = 0;
  // wait until previous primlist is finished
  startTime = time(0);
  do {
    statusword = mdspSingleRead(m_masterXface.xface.requestChannelStatus[0]);    
    if ((time(0)-startTime) > 100 && count>20) timeout = true;
    count++;
  }while(0!=statusword && timeout == false); 

  // write primitive into DSP buffer
  if(!timeout){
    //    std::cout << "Send primitive S" << m_slot << std::hex << m_masterXface.xface.requestChannelLoc[0]
    //              << buffAddr << std::dec << " " << buffCount << std::endl;
    mdspBlockWrite(m_masterXface.xface.requestChannelLoc[0], buffAddr, buffCount);
    m_myPrimState = RodPrimList::LOADED;
  }else{
    throw PrimListException("PrimList Timeout, previous list still executing", 0, 0);
  }

  if(getDebug()){
    //AK
    //    sprintf(filename, "prim%2.2d.bin", jsv_index2++);
    fp = fopen(m_debugFile.c_str(), "ab");
    if(fp!=NULL){
      fwrite(buffAddr, buffCount, 4, fp);
      fclose(fp);
    }
    //AK
  }

  return;  
}

//---------------------------------sendSlavePrimList---------------------                           
void RodModule::sendSlavePrimList(RodPrimList *slvPrimList, int slave) 
  throw(PrimListException &, VmeException &) {

  slvPrimList->bufferBuild();
  
  long buffLength = slvPrimList->getBufferLength();
  unsigned long* slavePrimList = new unsigned long[buffLength];
  unsigned long* bufferStart = slvPrimList->getBuffer();
  for (int i=0; i< buffLength; i++) {
    slavePrimList[i] = bufferStart[i];
  }
  
  // Create Send Slave List primitive
  RodPrimList primList(1);
  long int *slaveData = new long int[sizeof(SendSlaveListIn)/sizeof(UINT32) + 
				     buffLength]; 
  SendSlaveListIn* SendSlaveList_In = (SendSlaveListIn*)slaveData;
  SendSlaveList_In->slave         = slave;
  SendSlaveList_In->nWords        = buffLength;
  SendSlaveList_In->queueIndex    = 0;

  for (int i=0; i< buffLength; i++) {
    slaveData[i+sizeof(SendSlaveListIn)/4] = slavePrimList[i];
  }
  delete slavePrimList;
  RodPrimitive* send;
  send = new RodPrimitive(sizeof(SendSlaveListIn)/sizeof(UINT32) + buffLength, 
			  0, SEND_SLAVE_LIST, R_BOGUS, slaveData);
  primList.insert(primList.begin(), *send);

  primList.bufferBuild();
  sendPrimList(&primList);
  delete slaveData;
}

//---------------------------validateReplyBuffer ----------------
int RodModule::validateReplyBuffer() throw( VmeException &) {
  ReplyListHead replyListHead;
  ReplyListTail replyListTail;
  ReplyHead     replyHead;
  UINT32        curr_addr;

  curr_addr = m_masterXface.xface.requestChannelReplyBufferLoc[0];

  unsigned long int mbuffer2[100];

  //AKDEBUG fix probelm reading SDSP data, first word corrupted 
  mdspBlockRead(m_masterXface.xface.requestChannelReplyBufferLoc[0], &mbuffer2[0],18);

  //Debug
  // for(int i=0;i<3;i++)std::cout << std::hex << "Pre DATA: " << mbuffer2[i]<< std::dec << std::endl;


  mdspBlockRead(curr_addr, (unsigned long int*)&replyListHead, 
		sizeof(ReplyListHead)/sizeof(UINT32));

  curr_addr += sizeof(ReplyListHead);

  //read replies for each primitive
  for (unsigned int i = 0; i< replyListHead.nMsgs; i++){
    mdspBlockRead(curr_addr, (unsigned long int*)&replyHead, 
		  sizeof(ReplyHead)/sizeof(UINT32));
    //    curr_addr += replyHead.length*sizeof(UINT32) + sizeof(ReplyHead);
    curr_addr += replyHead.length*sizeof(UINT32) + sizeof(ReplyHead);
   if(curr_addr>(m_masterXface.xface.requestChannelReplyBufferLoc[0] +
		 m_masterXface.xface.requestChannelReplyBufferLen[0]*sizeof(UINT32)))return -1;
  }
  mdspBlockRead(curr_addr, (unsigned long int*)&replyListTail, 
		sizeof(ReplyListTail)/sizeof(UINT32));

  curr_addr += sizeof(ReplyListTail);
  //  UINT32 length = (curr_addr - m_masterXface.xface.requestChannelReplyBufferLoc[0] 
  //		   - sizeof(ReplyListHead) - sizeof(ReplyListTail))/sizeof(UINT32); 
  //
  //Debug
  // std::cout << " expected length from Tail " << replyListTail.length
  //          << " from Head " << replyListHead.length
  //	      << " counted:  " << length << std::endl;

//   unsigned long int mbuffer[1000];
//   mdspBlockRead(m_masterXface.xface.requestChannelReplyBufferLoc[0], &mbuffer[0], 
// 		(curr_addr - m_masterXface.xface.requestChannelReplyBufferLoc[0])/sizeof(UINT32));
//   for(int i=0;i<length;i++)std::cout << std::hex << "POST DATA" << mbuffer[i]<< std::dec << std::endl;


  //check consistency
  if(replyListHead.length==replyListTail.length){
    return replyListHead.length;
  }else{
    return -1;
  }

}

//---------------------------------primHandler-------------------------------                           

RodPrimList::PrimState RodModule::primHandler() throw(RodException &, VmeException &) {
unsigned long status;
unsigned long command;

  switch (m_myPrimState) {
    case RodPrimList::LOADED: {
      try {
	status = mdspSingleRead(m_masterXface.xface.requestChannelStatus[0]);
      }
      catch (VmeException ex) {
	std::cout << "RodModule::primHandler: Warning : BUS error reading status reg" 
		  << std::endl;
        break;
      }
      if (0==status) {
	command = 0;
	mdspSingleWrite(m_masterXface.xface.requestChannelControl[0], 1);
        command = mdspSingleRead(m_masterXface.xface.requestChannelControl[0]);
        m_myPrimState = RodPrimList::EXECUTING;
	// std::cout << "Control " << command << std::endl;
      };
    break;
    };
    case RodPrimList::EXECUTING: {
      try {
         status = mdspSingleRead(m_masterXface.xface.requestChannelStatus[0]);
      }
      catch (VmeException ex) {
	std::cout << "RodModule::primHandler: Warning : BUS error reading status reg" 
		  << std::endl;
        break;
      }		
      unsigned int nDone  = status & 0xFFFF;
      unsigned int nTotal = (status >> 16) & 0xFFFF;
      if(nTotal && (nDone == nTotal)){
	// we have processed all primitives
	m_myPrimState = RodPrimList::IDLE;

	// need to handle primitive replies
	int  rstatus = validateReplyBuffer();
	// std::cout << " validateReplyBuffer rstatus: " << rstatus << std::endl;
	int replylength =  rstatus + (sizeof(ReplyListHead) + sizeof(ReplyListTail))/sizeof(UINT32);
	// std::cout << "Have a ReplyBuffer of "<< replylength << " words" << std::endl;
	if(replylength > 0){
	  m_myPrimState = RodPrimList::WAITING;
	  if (m_myOutList) delete m_myOutList;
	  m_myOutList = new RodOutList(replylength);
	  mdspBlockRead(m_masterXface.xface.requestChannelReplyBufferLoc[0],
			m_myOutList->getBody(), replylength);
	  //AKDEBUG
	  // std::cout << "Have " << m_myOutList->nMsgs() << " replies" << std::endl;
	}
	// inform DSP that we are done
	mdspSingleWrite(m_masterXface.xface.requestChannelControl[0], 0);	
      } else {
// Cout to debug Primitive Lists not finishing....
//        std::cout << "RodModule::primHandler() not acting: " << nDone << "/" << nTotal << std::endl;
      }
    break;
    };
  case RodPrimList::WAITING: {
    m_myPrimState = RodPrimList::IDLE;
    break;
    };  
  default: {
    break;
    };
  };
  return m_myPrimState;
}

//--------------------------------deleteOutList--------------------------------

  void RodModule::deleteOutList() {
  delete m_myOutList;
  m_myOutList = 0;
  return;
  };

//---------------------------------getTextBuffer-------------------------------                           

bool RodModule::getTextBuffer(std::string &rettxtBuf, unsigned int buffnum, 
			      bool reset) throw(VmeException &){
  
  TextQueue textQueue;
  std::string tmptxtBuf = "";
  bool read = false;

  // bail out if non existing textbuffer requested
  if (buffnum >  n_textbuffers) return read;

  //  for (int buffnum=0; buffnum< m_masterXface.xface.nTextBuffers; buffnum++) {
    mdspBlockRead(m_textBuffer[buffnum], (long unsigned int *)&textQueue, 
		  sizeof(TextQueue)/sizeof(UINT32));
    // check if we found text
    UINT32 tail = textQueue.tail;
    long nWords, nChars;
    if(0 != (nWords = textQueue.head - textQueue.tail)){
        std::cout << "DEBUG: RodModule::getTextBuffer(): trying to read " << nWords<<" words into " 
                                                         << rettxtBuf<<std::endl;
        if (nWords > 4096){
            std::cerr <<"ERROR: RodModule::getTextBuffer(): nWords is larger than 4096!!! skipping download of text buffers"<<std::endl;
            return false;
     }
     read = true;
     // handle wrap
      if(nWords<0)nWords += textQueue.length;
      nChars = nWords*sizeof(UINT32);
      long nTop = textQueue.length - tail;
      long unsigned int *buffPtr = (long unsigned int*) m_textData[buffnum];
      if(nWords >  nTop){
	mdspBlockRead(textQueue.base + sizeof(UINT32)*tail, buffPtr, nTop);
	buffPtr += nTop;
	nWords  -= nTop;
	tail = 0;
      }
      if(nWords){
	mdspBlockRead(textQueue.base + sizeof(UINT32)*tail, buffPtr, nWords);	
	tail += nWords;
      }

      if(reset){
	// reset tail to read position on DSP
	TextQueue *DSPtextQueue = (TextQueue *)m_textBuffer[buffnum];
	//    tailoffset = &(queue.tail) - &queue;
	// AKDEBUG-> DANGEROUS, change interface
	mdspSingleWrite((long unsigned int)&(DSPtextQueue->tail), tail);
      }
      // copy into output string, no real need to keep local buffer anymore, 
      // but useful for debug

      for(int i = 0; i < nChars; i++){
	tmptxtBuf += m_textData[buffnum][i];
      }
    }
  //  }

  //copy constructor to output string
  rettxtBuf = tmptxtBuf;

  return read;
}

//----------------------------------hpiLoad------------------------------------                                

void RodModule::hpiLoad(unsigned long hpiReg, unsigned long hpiValue) 
     throw(VmeException &) {

  if (endianSwap) hpiValue = endianReverse32(hpiValue); 
  m_myVmePort->write32(hpiReg, hpiValue);
};

//--------------------------------hpiFetch-------------------------------------                                

unsigned long RodModule::hpiFetch(unsigned long hpiReg) 
              throw(VmeException &){

  unsigned long hpiValue;

  hpiValue=m_myVmePort->read32(hpiReg);
  if (endianSwap) hpiValue = endianReverse32(hpiValue);
  
  return hpiValue;
}

//-----------------------------mdspSingleRead---------------------------------                                

unsigned long RodModule::mdspSingleRead(const unsigned long dspAddr) 
              throw(VmeException &) {
  unsigned long value; 

//  Load the DSP address into the HPIA register 
  hpiLoad(HPIA, dspAddr);

//	Do the read 
  value = m_myVmePort->read32(HPID_NOAUTO);
  if (endianSwap) {
    value = endianReverse32(value);
  }
  return value;
};

//------------------------------mdspSingleWrite------------------------------------                                   

void RodModule::mdspSingleWrite(unsigned long dspAddr, unsigned long buffer) 
     throw(VmeException &) { 

//  Load the HPID address into the HPIA register 
  hpiLoad(HPIA, dspAddr);

//	Do the write 
  if (endianSwap) {
      buffer = endianReverse32(buffer);
  }

  m_myVmePort->write32(HPID_NOAUTO, buffer);

  return;
     }

//-----------------------------mdspBlockReadDMA---------------------------------                                


void RodModule::mdspBlockRead(const unsigned long dspAddr, unsigned long buffer[],
        const long wc, HpidMode mode) throw (HpiException &, 
            VmeException &) {
            long wordCount = wc; 
            bool forceSW = false;  
            if (wc < 0) {
              wordCount = -wc;
              forceSW = true;  
            }
            if (m_btFailure) forceSW = true;

            unsigned long hpidAddr; 
            long myCount, localCount, blockWordCount, wordIncr;
	    
            // Determine the HPI mode to use.
            switch (mode) {
                case AUTO:
                    hpidAddr= HPID_AUTO;
                    break;
                case NO_AUTO:
                    hpidAddr = HPID_NOAUTO; 
                    break;
                case DYNAMIC:
                default:
                    if (wordCount == 1){
                        hpidAddr = HPID_NOAUTO;
                    }	
                    else { 
                        hpidAddr = HPID_AUTO;
                    };
                    break;
            };

            //  Load the DSP address into the HPIA register 
            hpiLoad(HPIA, dspAddr);  

            // Check if wordcount is odd and, if so, first do a single word read.

            localCount = wordCount;
            wordIncr = 0;
            if (wordCount%2 !=0) {
                buffer[0] = m_myVmePort->read32(hpidAddr);
                wordIncr = 1;
                localCount -= 1;
            }

            // Check to see that we don't access too large a block. 
            // MAX_HPID_WORD_ELEMENTS is defined in RodVmeAddresses.h

            blockWordCount = std::min(localCount, MAX_HPID_WORD_ELEMENTS);

            //	Set up the transfer as a series of block transfers 
            for (myCount=localCount; myCount>0;
                    myCount-=blockWordCount, wordIncr+=blockWordCount) {

                //  Load the DSP address into the HPIA register 
                hpiLoad(HPIA, dspAddr+(wordIncr*sizeof(UINT32)));  


                //	Recheck each block 
                blockWordCount = std::min(myCount, MAX_HPID_WORD_ELEMENTS);

                //	Do the transfer for this block 
                // Block reads are being done in hardware as from 10 June 03.
                bool done = false;
                do {
                    if (blockWordCount < 140000 || forceSW) {
			int i;
                	for (i = 0; i<blockWordCount-1; i++) {
                		buffer[wordIncr+i] = m_myVmePort->read32(hpidAddr);
                	}
                	buffer[wordIncr+i] = m_myVmePort->read32(HPID_NOAUTO);
                        done = true;
		    } else {
			int rerun = 0;
                        int count = 0;
                	do{
                	    try {
                                count++;
	                        m_myVmePort->blockRead32(hpidAddr, buffer+wordIncr, blockWordCount*4);
	                        rerun =0;
                                done = true;
	                    }
	                    catch(VmeException &v){
	                        std::cout << "VmeException caught, retrying!" << std::endl;;
	                        usleep(10000);
	                        rerun =1;
	                    }
	                } while (rerun && count<10);
                        if (count >= 10) {
                          m_btFailure = true;
                          forceSW = true;
	                  std::cout << "Block transfer failure, reverting to single word transter" << std::endl;;
                        }
		    }
                } while (done==false);
		
                if (endianSwap) {
                    for (int i=0; i<blockWordCount; i++) {
                	buffer[wordIncr+i] = endianReverse32(buffer[wordIncr+i]);
                    }
                }
            }

            return;
        }


//------------------------------mdspBlockWrite-----------------------------------  

void RodModule::mdspBlockWrite(unsigned long dspAddr, unsigned long buffer[], 
        long wc, HpidMode mode) throw (HpiException &, 
            VmeException &) { 
            long wordCount = wc; 
            bool forceSW = false;  
            if (wc < 0) {
              wordCount = -wc;
              forceSW = true;  
            }
            if (m_btFailure) forceSW = true;

            unsigned long hpidAddr;
            long myCount, localCount, blockWordCount, wordIncr;

            //  Load the initial dsp address into the HPIA register 
            hpiLoad(HPIA, dspAddr);

            // Determine the HPI mode to use.
            switch (mode) {
                case AUTO:
                    hpidAddr= HPID_AUTO;
                    break;
                case NO_AUTO:
                    hpidAddr = HPID_NOAUTO; 
                    break;
                case DYNAMIC:
                default:
                    if (wordCount == 1){
                        hpidAddr = HPID_NOAUTO;
                    }	
                    else { 
                        hpidAddr = HPID_AUTO;
                    };
                    break;
            };

            // Check if wordcount is odd and, if so, first do a single word write.

            localCount = wordCount;
            wordIncr = 0;
            if (wordCount%2 !=0) {
                m_myVmePort->write32(hpidAddr, buffer[0]);
                wordIncr = 1;
                localCount -= 1;
            }

            // Check to see that we don't access too large a block.
            // MAX_HPID_WORD_ELEMENTS is defined in vmeAddressMap.h

            blockWordCount = std::min(wordCount, MAX_HPID_WORD_ELEMENTS);

            //	Set up the transfer as a series of block transfers 
            for (myCount=localCount; myCount>0;
                    myCount-=blockWordCount, wordIncr+=blockWordCount) {

                //	Recheck each block 
                blockWordCount = std::min(myCount, MAX_HPID_WORD_ELEMENTS);

                //	Do the transfer for this block 
                if (endianSwap) {
                    for (int i=0; i<blockWordCount; i++) {
                        buffer[wordIncr+i] = endianReverse32(buffer[wordIncr+i]);
                    }
                }

                // Block writes are being done in hardware as from 10 June 03.
                bool done = false;
                do {
                    if (blockWordCount<320000 || forceSW) {
			int i;
                	for (i = 0; i<blockWordCount-1; i++) {
	                        m_myVmePort->write32(hpidAddr, buffer[wordIncr+i]);
                	}
	                m_myVmePort->write32(HPID_NOAUTO, buffer[wordIncr+i]);
                        done = true;
		    } else {
			int rerun = 0;
                        int count = 0;
                	do{
                	    try {
                                count++;
	                        m_myVmePort->blockWrite32(hpidAddr, &buffer[wordIncr], blockWordCount*4);
	                        rerun =0;
                                done = true;
	                    }
	                    catch(VmeException &v){
	                        std::cout << "VmeException caught, retrying!" << std::endl;;
	                        usleep(10000);
	                        rerun =1;
	                    }
	                } while (rerun && count<10);
                        if (count >= 10) {
                          m_btFailure = true;
                          forceSW = true;
	                  std::cout << "Block transfer failure, reverting to single word transter" << std::endl;;
                        }
		    }
                } while (done==false);

            }
            return;
        }

//-----------------------------mdspBlockDump--------------------------------                                

void RodModule::mdspBlockDump(const unsigned long firstAddress, 
        const unsigned long lastAddress,
        const std::string & fileName) throw(RodException &, VmeException &) {

    long numBytes;
    std::ofstream fout;
    numBytes = lastAddress - firstAddress + 1;
    fout.open(fileName.c_str(), std::ios::binary);
    if (fout.is_open()) {
        unsigned long * buffer;
        try {
            buffer = new unsigned long[numBytes];
        }
        catch (std::bad_alloc & ba) {
            throw RodException(
                    "mdspBlockDump failed to allocate buffer; buffer size in bytes=", numBytes);
        }   
        mdspBlockRead(firstAddress, buffer, numBytes/4);
        fout.write((char*)buffer, numBytes);
        fout.close();
        delete [] buffer;
    }
    else {
        throw RodException("mdspBlockDump failed to open file");
    }
    return;                    
}

//--------------------------------slvHpiLoad-----------------------------------                                

void RodModule::slvHpiLoad(unsigned long hpiReg, unsigned long hpiValue,
        long slaveNum) throw(VmeException &) {

    unsigned long address;
    address = hpiReg + slaveNum*SLAVE_HPI_OFFSET;

    mdspSingleWrite(address, hpiValue);
}

//------------------------------slvHpiFetch------------------------------------

unsigned long RodModule::slvHpiFetch(unsigned long hpiReg, long slaveNum) 
    throw(VmeException &) {

        unsigned long hpiValue;
        unsigned long address;
        address = hpiReg + slaveNum*SLAVE_HPI_OFFSET;

        hpiValue=mdspSingleRead(address);

        return hpiValue;
    }

//-------------------------------slvHpiSingleRead---------------------------------
unsigned long RodModule::slvHpiSingleRead(unsigned long dspAddr, long slaveNum ) 
    throw(VmeException &, RodException &) {

        unsigned long slvHpia, slvHpid;
        unsigned long value;
        slvHpia = SLAVE_HPIA_BASE + slaveNum*SLAVE_HPI_OFFSET;
        slvHpid = SLAVE_HPID_NOAUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;

        // Check that address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
        if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
                "Slave address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr);  

        //  Load the DSP address into the HPIA register 
        mdspSingleWrite(slvHpia, dspAddr);

        //	Do the read 
        value = mdspSingleRead(slvHpid);

        return value;
    }

//------------------------------slvHpiSingleWrite------------------------------------                                   

void RodModule::slvHpiSingleWrite(unsigned long dspAddr, unsigned long buffer, 
        long slaveNum) throw(VmeException &, RodException &) { 

    unsigned long slvHpia, slvHpid;
    slvHpia = SLAVE_HPIA_BASE + slaveNum*SLAVE_HPI_OFFSET;
    slvHpid = SLAVE_HPID_NOAUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;

    // Check that address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr);  

    mdspSingleWrite(slvHpia, dspAddr);

    //	Do the write 
    mdspSingleWrite(slvHpid, buffer);

    return;
}

//-----------------------------slvHpiBlockRead---------------------------------                                

void RodModule::slvHpiBlockRead(const unsigned long dspAddr, 
        unsigned long buffer[], const long wordCount, 
        long slaveNum, HpidMode mode) 
throw (HpiException &, VmeException &, RodException &) {

    unsigned long hpidAddr, dspAddrLocal; 
    long myCount, blockWordCount, wordIncr;
    unsigned long slvHpia, slvHpidAuto, slvHpidNoAuto;

    // Check that starting address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave start address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr); 

    // Check that ending address is not in forbidden area as per TI errata doc SPRZ191F, p. 11

    unsigned long endAddr = dspAddr + 4*wordCount;

    if ((endAddr >= 0x60000000) && (endAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave stop address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", endAddr);  

    // Check that addresses do not straddle the forbidden area as per TI errata doc SPRZ191F, p. 11

    if ((dspAddr < 0x60000000) && (endAddr > 0x7FFFFFFF)) throw RodException(
            "Block of slave addresses straddles forbidden range of 0x60000000 to 0x7FFFFFFF; addresses =", dspAddr, endAddr);  

    slvHpia = SLAVE_HPIA_BASE + slaveNum*SLAVE_HPI_OFFSET;
    slvHpidAuto = SLAVE_HPID_AUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;
    slvHpidNoAuto = SLAVE_HPID_NOAUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;

    // Determine the HPI mode to use.
    switch (mode) {
        case AUTO:
            hpidAddr= slvHpidAuto;
            break;
        case NO_AUTO:
            hpidAddr = slvHpidNoAuto; 
            break;
        case DYNAMIC:
        default:
            if (wordCount == 1){
                hpidAddr = slvHpidNoAuto;
            }	
            else { 
                hpidAddr = slvHpidAuto;
            };
            break;
    };

    // TI Errata SPRZ191F, p. 17, requires that every autoincrement read/write be terminated with a fixed
    // mode read/write. This routine has been modified to comply with this.

    long modCount = wordCount -1;
    if (modCount == 0) modCount = 1;

    //  Load the DSP address into the HPIA register 
    mdspSingleWrite(slvHpia, dspAddr);

    // Check to see that we don't access too large a block. HPID has 20 bits of 
    // address space available. MAX_HPID_WORD_ELEMENTS is defined in RodVmeAddresses.h

    blockWordCount = std::min(modCount, MAX_HPID_WORD_ELEMENTS);

    //	Set up the transfer as a series of block transfers 
    for (myCount=modCount, wordIncr=0; myCount>0;
            myCount-=blockWordCount, wordIncr+=blockWordCount) {

        //	Recheck each block 
        blockWordCount = std::min(myCount, MAX_HPID_WORD_ELEMENTS);

        //	Do the transfer for this block 
        dspAddrLocal = dspAddr+ (4*wordIncr);
        mdspBlockRead(hpidAddr, &buffer[wordIncr], blockWordCount, NO_AUTO);
    }

    //     Do the final fixed-mode transfer
    if (hpidAddr == slvHpidAuto) {
        hpidAddr = slvHpidNoAuto;
        mdspSingleWrite(slvHpia, modCount*4);
        buffer[modCount] = mdspSingleRead(hpidAddr);
    }

    return;
}

//------------------------------slvHpiBlockWrite-----------------------------------  

void RodModule::slvHpiBlockWrite(unsigned long dspAddr, unsigned long buffer[], 
        long wordCount, long slaveNum, HpidMode mode) 
throw (HpiException &, VmeException &, RodException &) { 

    unsigned long hpidAddr, dspAddrLocal;
    long myCount, blockWordCount, wordIncr;
    unsigned long slvHpia, slvHpidAuto, slvHpidNoAuto;

    // Check that starting address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave start address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr); 

    unsigned long endAddr = dspAddr + 4*wordCount;

    // Check that ending address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((endAddr >= 0x60000000) && (endAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave stop address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", endAddr);  

    // Check that addresses do not straddle the forbidden area as per TI errata doc SPRZ191F, p. 11

    if ((dspAddr < 0x60000000) && (endAddr > 0x7FFFFFFF)) throw RodException(
            "Block of slave addresses straddles forbidden range of 0x60000000 to 0x7FFFFFFF; addresses =", dspAddr, endAddr);  

    slvHpia = SLAVE_HPIA_BASE + slaveNum*SLAVE_HPI_OFFSET;
    slvHpidAuto = SLAVE_HPID_AUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;
    slvHpidNoAuto = SLAVE_HPID_NOAUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;

    //  Load the initial dsp address into the HPIA register 
    mdspSingleWrite(slvHpia, dspAddr);

    // Determine the HPI mode to use.
    switch (mode) {
        case AUTO:
            hpidAddr= slvHpidAuto;
            break;
        case NO_AUTO:
            hpidAddr = slvHpidNoAuto; 
            break;
        case DYNAMIC:
        default:
            if (wordCount == 1){
                hpidAddr = slvHpidNoAuto;
            }	
            else { 
                hpidAddr = slvHpidAuto;
            };
            break;
    };

    // TI Errata SPRZ191F, p. 17, requires that every autoincrement read/write be terminated with a fixed
    // mode read/write. This routine has been modified to comply with this.

    long modCount = wordCount -1;
    if (modCount == 0) modCount = 1;

    // Check to see that we don't access too large a block. HPID has 20 bits of 
    // address space available. MAX_HPID_WORD_ELEMENTS is defined in vmeAddressMap.h

    blockWordCount = std::min(modCount, MAX_HPID_WORD_ELEMENTS);

    //	Set up the transfer as a series of block transfers 
    for (myCount=modCount, wordIncr=0; myCount>0;
            myCount-=blockWordCount, wordIncr+=blockWordCount) {

        //	Recheck each block 
        blockWordCount = std::min(myCount, MAX_HPID_WORD_ELEMENTS);

        //	Do the transfer for this block 
        dspAddrLocal = dspAddr+ (4*wordIncr);
        mdspBlockWrite(hpidAddr, &buffer[wordIncr], blockWordCount, NO_AUTO);
    }

    //    If autoincrement was used, do the final fixed-mode transfer
    if (hpidAddr == slvHpidAuto) {
        hpidAddr = slvHpidNoAuto;
        mdspSingleWrite(slvHpia, modCount*4);
        mdspSingleWrite(hpidAddr, buffer[modCount]);
    }

    return;
}

//-------------------------------slvSingleRead---------------------------------
unsigned long RodModule::slvSingleRead(unsigned long dspAddr, long slaveNum ) 
    throw(VmeException &, RodException &) {

        unsigned long slvHpia, slvHpid;
        unsigned long value;
        long numWords;
        slvHpia = SLAVE_HPIA_BASE + slaveNum*SLAVE_HPI_OFFSET;
        slvHpid = SLAVE_HPID_NOAUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;

        // Check that address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
        if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
                "Slave address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr);  

        // Use RW_SLAVE_MEMORY to read slave DSP memory
        long* writeSlvBuff;
        numWords = 1;

        writeSlvBuff = new long[numWords + sizeof(RwSlaveMemoryIn)/4];
        RwSlaveMemoryIn* RSlaveMemoryIn = (RwSlaveMemoryIn*)writeSlvBuff; 
        RSlaveMemoryIn->slave      = slaveNum;
        RSlaveMemoryIn->flags      = 1;
        RSlaveMemoryIn->slaveAddr  = (UINT32)dspAddr;
        RSlaveMemoryIn->masterAddr = (UINT32)NULL ;
        RSlaveMemoryIn->nWords     = numWords;
        writeSlvBuff[5] = 0;
        RodPrimitive* slvRWPrim;
        slvRWPrim = new RodPrimitive(numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 
                1, RW_SLAVE_MEMORY, R_BOGUS, writeSlvBuff);
        synchSendPrim(*slvRWPrim);
        delete [] writeSlvBuff;
        delete slvRWPrim;

        // Get reply buffer and extract the value    
        RodOutList* outList = getOutList();
        if (outList) {
            unsigned long* outBody = outList->getBody();
            value = outBody[sizeof(RwMemoryReplyHeader)/sizeof(UINT32)];
            deleteOutList();
        }
        else {
            throw RodException("No outList in slvPrimSingleRead");
        }
        return value;
    }

//------------------------------slvSingleWrite------------------------------------                                   

void RodModule::slvSingleWrite(unsigned long dspAddr, unsigned long buffer, 
        long slaveNum) throw(VmeException &, RodException &) { 

    unsigned long slvHpia, slvHpid;
    long numWords;
    slvHpia = SLAVE_HPIA_BASE + slaveNum*SLAVE_HPI_OFFSET;
    slvHpid = SLAVE_HPID_NOAUTO_BASE + slaveNum*SLAVE_HPI_OFFSET;

    // Check that address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr);  

    // Use RW_SLAVE_MEMORY to write to slave DSP memory
    long* writeSlvBuff;
    numWords = 1;
    writeSlvBuff = new long[numWords + sizeof(RwSlaveMemoryIn)/4];
    RwSlaveMemoryIn* WSlaveMemoryIn = (RwSlaveMemoryIn*)writeSlvBuff; 
    WSlaveMemoryIn->slave      = slaveNum;
    WSlaveMemoryIn->flags      = RW_WRITE;
    WSlaveMemoryIn->slaveAddr  = (UINT32)dspAddr;
    WSlaveMemoryIn->masterAddr = (UINT32)NULL ;
    WSlaveMemoryIn->nWords     = numWords;
    writeSlvBuff[5] = buffer;
    RodPrimitive* slvRWPrim;
    slvRWPrim = new RodPrimitive(numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 
            1, RW_SLAVE_MEMORY, R_BOGUS, writeSlvBuff);
    synchSendPrim(*slvRWPrim);
    delete [] writeSlvBuff;
    delete slvRWPrim;

    return;
}

//-----------------------------slvBlockRead---------------------------------                                

void RodModule::slvBlockRead(const unsigned long dspAddr, unsigned long buffer[],
        const long wordCount, long slaveNum, HpidMode mode) 
throw (HpiException &, VmeException &, RodException &) {

    long numWords;
    long readWords = 0;
    unsigned long slaveAddr = dspAddr;

    // Check that starting address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave start address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr); 

    // Check that ending address is not in forbidden area as per TI errata doc SPRZ191F, p. 11

    unsigned long endAddr = dspAddr + 4*wordCount;

    if ((endAddr >= 0x60000000) && (endAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave stop address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", endAddr);  

    // Check that addresses do not straddle the forbidden area as per TI errata doc SPRZ191F, p. 11

    if ((dspAddr < 0x60000000) && (endAddr > 0x7FFFFFFF)) throw RodException(
            "Block of slave addresses straddles forbidden range of 0x60000000 to 0x7FFFFFFF; addresses =", dspAddr, endAddr);  

    // Use RW_SLAVE_MEMORY to read slave DSP memory
    long* writeSlvBuff;
    numWords = wordCount;
    int k = 0;

    while(wordCount > readWords){
        writeSlvBuff = new long[sizeof(RwSlaveMemoryIn)/4];
        RwSlaveMemoryIn* RSlaveMemoryIn = (RwSlaveMemoryIn*)writeSlvBuff; 
        RSlaveMemoryIn->slave      = slaveNum;
        RSlaveMemoryIn->flags      = RW_READ;
        RSlaveMemoryIn->slaveAddr  = (UINT32)slaveAddr;
        RSlaveMemoryIn->masterAddr = (UINT32)NULL ;
        RSlaveMemoryIn->nWords     = numWords - readWords;
        RodPrimitive* slvRWPrim;
        slvRWPrim = new RodPrimitive(sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 
                1, RW_SLAVE_MEMORY, R_BOGUS, writeSlvBuff);
        synchSendPrim(*slvRWPrim);
        delete [] writeSlvBuff;
        delete slvRWPrim;

        // Get reply buffer and extract the value    
        RodOutList* outList = getOutList();
        if (outList) {
            unsigned long* outBody = (unsigned long*)outList->getMsgBody(1);
            RwMemoryReplyHeader * RwMemoryReplyHeader_Out = (RwMemoryReplyHeader *) outBody;
            readWords += RwMemoryReplyHeader_Out->nWords;
            slaveAddr += sizeof(UINT32)*RwMemoryReplyHeader_Out->nWords;
            /* break if we don't read anything */
            if(0==RwMemoryReplyHeader_Out->nWords)readWords += wordCount;
            for (int i = 0; i< readWords; i++) {
                buffer[k++] = outBody[i+(sizeof(RwMemoryReplyHeader)/sizeof(UINT32))];
            }
            deleteOutList();
        }
        else {
            throw RodException("No outList in slvPrimBlockRead");
        }
    }

    return;
}

//------------------------------slvBlockWrite-----------------------------------  

void RodModule::slvBlockWrite(unsigned long dspAddr, unsigned long buffer[], 
        long wordCount, long slaveNum, HpidMode mode) 
throw (HpiException &, VmeException &, RodException &) { 

    long numWords;

    // Check that starting address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((dspAddr >= 0x60000000) && (dspAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave start address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", dspAddr); 

    unsigned long endAddr = dspAddr + 4*wordCount;

    // Check that ending address is not in forbidden area as per TI errata doc SPRZ191F, p. 11
    if ((endAddr >= 0x60000000) && (endAddr <= 0x7FFFFFFF)) throw RodException(
            "Slave stop address is in forbidden range of 0x60000000 to 0x7FFFFFFF; address =", endAddr);  

    // Check that addresses do not straddle the forbidden area as per TI errata doc SPRZ191F, p. 11

    if ((dspAddr < 0x60000000) && (endAddr > 0x7FFFFFFF)) throw RodException(
            "Block of slave addresses straddles forbidden range of 0x60000000 to 0x7FFFFFFF; addresses =", dspAddr, endAddr);  

    // Use RW_SLAVE_MEMORY to write to slave DSP memory
    long* writeSlvBuff;
    numWords = wordCount;

    writeSlvBuff = new long[numWords + sizeof(RwSlaveMemoryIn)/4];
    RwSlaveMemoryIn* WSlaveMemoryIn = (RwSlaveMemoryIn*)writeSlvBuff; 
    WSlaveMemoryIn->slave      = slaveNum;
    WSlaveMemoryIn->flags      = RW_WRITE;
    WSlaveMemoryIn->slaveAddr  = (UINT32)dspAddr;
    WSlaveMemoryIn->masterAddr = (UINT32)NULL ;
    WSlaveMemoryIn->nWords     = numWords;
    for (int i=0; i<numWords; i++) {
        writeSlvBuff[i + sizeof(RwSlaveMemoryIn)/4] = buffer[i];
    }
    RodPrimitive* slvRWPrim;
    slvRWPrim = new RodPrimitive(numWords + sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 
            1, RW_SLAVE_MEMORY, R_BOGUS, writeSlvBuff);
    synchSendPrim(*slvRWPrim);
    delete [] writeSlvBuff;
    delete slvRWPrim;

    return;
};

//------------------------------resetMasterDsp--------------------------------                                

void RodModule::resetMasterDsp() throw(RodException&, VmeException &) {
    unsigned long value=0;
    unsigned long rodRegValue;
    clock_t startTime;

    setBit(&value, 1);
    if (endianSwap) {
        value = endianReverse32(value);
    }
    m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[2], value);
    sleep(4000);
    rodRegValue = 0;
    startTime = clock();
    do {
        rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[2]);
        if ((clock()-startTime)/CLOCKS_PER_SEC > DSP_RESET_TIMEOUT) {
            throw RodException("DSP reset timeout in resetMasterDsp(), DSP_RESET_TIMEOUT=",
                    (long)DSP_RESET_TIMEOUT);
        }
    }
    while (rodRegValue != ALLDSP_RESET_COMPLETE);

    return;
}

//------------------------------resetSlaveDsp--------------------------------                                

void RodModule::resetSlaveDsp(long slaveNumber) throw(RodException&, VmeException &) {
    unsigned long value=0;
    unsigned long rodRegValue;
    clock_t startTime;

    setBit(&value, 2 + slaveNumber);
    if (endianSwap) {
        value = endianReverse32(value);
    }
    m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[2], value);
    sleep(4000);
    rodRegValue = 0;
    startTime = clock();
    do {
        rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[2]);
        if ((clock()-startTime)/CLOCKS_PER_SEC > DSP_RESET_TIMEOUT) {
            throw RodException("DSP reset timeout in resetSlaveDsp(), DSP_RESET_TIMEOUT=",
                    (long)DSP_RESET_TIMEOUT);
        }
    }
    while (rodRegValue != ALLDSP_RESET_COMPLETE);

    return;
}


//------------------------------resetAllDsps--------------------------------                                

void RodModule::resetAllDsps() throw(RodException &, VmeException &) {
    unsigned long value=0;
    unsigned long rodRegValue;
    clock_t startTime;

    setBit(&value, 6);
    if (endianSwap) {
        value = endianReverse32(value);
    }
    m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[2], value);
    sleep(4000);
    rodRegValue = 0;
    startTime = clock();
    do {
        rodRegValue = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[2]);
        if ((clock()-startTime)/CLOCKS_PER_SEC > DSP_RESET_TIMEOUT) {
            throw RodException("DSP reset timeout in resetAllDsps(), DSP_RESET_TIMEOUT=",
                    (long)DSP_RESET_TIMEOUT);
        }
    }
    while (rodRegValue != ALLDSP_RESET_COMPLETE);

    return;
}

//-------------------------------chipEraseHpi---------------------------------

void RodModule::chipEraseHpi() throw(VmeException &) {
    unsigned long value = 0x10;
    unsigned long flashBase;

    //  flashBase = m_myMdspMap->flashBase();

    // Commands 1 to 5
    commonEraseCommandsHpi(flashBase);

    // Command 6
    mdspSingleWrite(flashBase+(0x5555<<2), value);

    // Wait for operation to complete
    sleep(CHIP_ERASE_TIME_MS);
    return;
}

//--------------------------------sectorErase---------------------------------

void RodModule::sectorErase(unsigned long sectorBaseAddress) 
    throw(RodException &, VmeException &) {
        unsigned long flashBaseAddress, valueD32;
        bool busyBit;

        // Get flash base address
        switch (m_revision) {
            case 0x0f: 
            case 0x0e: 
                flashBaseAddress = FPGA_FLASH_REL_ADDR_REVE;
                break;
            case 0x0b:
            case 0x0c:
            default: 
                if ((sectorBaseAddress<FPGA_FLASH_0_BOTTOM)||
                        (sectorBaseAddress>FPGA_FLASH_2_BOTTOM+FLASH_MEMORY_SIZE)) {
                    throw RodException("Flash sector base addr out of range, sectorBaseAddress=",
                            sectorBaseAddress);
                }
                if (sectorBaseAddress<FPGA_FLASH_1_BOTTOM) {
                    flashBaseAddress = FPGA_FLASH_0_BOTTOM;
                }
                else if (sectorBaseAddress<FPGA_FLASH_2_BOTTOM) {
                    flashBaseAddress = FPGA_FLASH_1_BOTTOM;
                }
                else flashBaseAddress = FPGA_FLASH_2_BOTTOM;
                break;
        }
        // Commands 1 to 5
        commonEraseCommands(flashBaseAddress);

        // Set write bit
        vmeWriteElementFlash(0x30, sectorBaseAddress, WRITE_COMMAND_HANDSHAKE_BIT);

        // Wait for operation to complete
        switch (m_revision ) {
            case 0x0f: 
            case 0x0e:
                do {                           /* Poll busy */ 
                    valueD32 = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[4]); 
                    busyBit = readBit(valueD32, 3);                
                } while (busyBit);
                break;
            case 0x0b:
            case 0x0c: 
            default:
                sleep(SECTOR_ERASE_TIME_MS);
                break;
        }

        return;
    }

//---------------_-------------writeByteToFlash-------------------------------

void RodModule::writeByteToFlash(unsigned long address, UINT8 data) 
    throw (RodException &, VmeException &) {
        UINT8 readData;
        clock_t startTime;   // start time for DQ7 data polling, in seconds
        long updateAddress;

        if (data!=0xFF) {   // if erased, don't rewrite. 0xFF is default after erasing
            // write data
            vmeWriteElementFlash(data, address, WRITE_DATA_HANDSHAKE_BIT);
            updateAddress = 0;
        }
        else {
            updateAddress = 1;
        }

        // Wait for operation to complete - data polling
        startTime = clock();
        while(1) {
            readData = readByteFromFlash(address, updateAddress);
            if (readData == data) return;
            else if((clock()-startTime)/CLOCKS_PER_SEC > FLASH_TIMEOUT) {
                throw RodException("Flash timeout in writeByteToFlash, FLASH_TIMEOUT=",
                        (long)FLASH_TIMEOUT);
            }
        }
        return;
    }

//----------------------------writeBlockToFlash-------------------------------

void RodModule::writeBlockToFlash(unsigned long address, UINT8 *data, 
        unsigned long numBytes) throw(RodException &, VmeException &) {
    unsigned long index, sectorSize;
    switch (m_revision) {
        case 0x0f: 
        case 0x0e:
            sectorSize = FLASH_SECTOR_SIZE_REVE;
            break;
        case 0x0b:
        case 0x0c:
        default:
            sectorSize = FLASH_SECTOR_SIZE;
            break;
    }
    for (index=0; index<numBytes; ++index) {
        if ((index%sectorSize)==0) {
            sectorErase(address+index);
        }
        writeByteToFlash(address+index, *(data+index));
    } 
    return;
}

//---------------------------writeBlockToFlashHpi-----------------------------

void RodModule::writeBlockToFlashHpi(unsigned long address, UINT8 *data, 
        unsigned long numBytes) throw (RodException &, VmeException &) {
    unsigned long index, sectorAddr;
    unsigned long flashBase = 0x01400000;
    const unsigned long eraseData=0x30;
    const unsigned long data1=0xAA;
    const unsigned long data2=0x55;
    const unsigned long data3=0xA0;
    const unsigned long addr1=flashBase+(0x5555<<2);
    const unsigned long addr2=flashBase+(0x2AAA<<2);
    unsigned long byteRelAddr;
    long words;
    UINT8 dataVal, verifyVal;
    unsigned long longVal, busyVal;

    for (index=0; index<numBytes; ++index) {
        byteRelAddr = address+index-flashBase;
        if ((index%FLASH_SECTOR_SIZE)==0) {
            // implement sectorEraseHpi(address+index) inline.
            //    commands 1 to 5
            commonEraseCommandsHpi(flashBase);
            // 6th command
            sectorAddr = flashBase+
                (byteRelAddr<<2);
            mdspSingleWrite(sectorAddr, eraseData);
            // Wait for operation to complete
            sleep(SECTOR_ERASE_TIME_MS);
        };

        // implement writeByteToFlashHpi(address+index, *(data+index)) inline.
        dataVal = *(data+index);
        longVal = dataVal&0x000000FF;
        if (dataVal != 0xFF) {
            mdspSingleWrite(addr1, data1);                      // 1st command
            mdspSingleWrite(addr2, data2);                      // 2nd command
            mdspSingleWrite(addr1, data3);                      // 3rd command
            mdspSingleWrite(flashBase+(byteRelAddr<<2), longVal);
            // Verify data by reading it back before continuing. Flash memory may need up to
            // 20 microseconds to complete the write sequence.
            for (int i=0; i<4000; i++) {
                busyVal = mdspSingleRead(flashBase+byteRelAddr);
                switch (byteRelAddr%4) {
                    case 1: busyVal = busyVal >> 8;
                            break;
                    case 2: busyVal = busyVal >> 16;
                            break;
                    case 3: busyVal = busyVal >> 24;
                            break;
                    default: break;
                }
                busyVal = busyVal & 0x000000FF;
                if (busyVal == longVal) break;
            }
        };
    };

    // Verification
    words = (numBytes+3)/4;       // round up
    UINT8 *buffer;
    try {
        buffer = new UINT8[words*4];
    }
    catch (std::bad_alloc & ba) {
        throw RodException("writeBlockToFlashHpi unable to get buffer.");
    }
    mdspBlockRead(address, (unsigned long*)buffer, -words);
    bool vfailed = false;
    for (index=0; index<numBytes; ++index) {
        dataVal = *(data+index);
        verifyVal = *(buffer+index);
        if (dataVal != verifyVal) {
            //      delete [] buffer;
            int dread =  verifyVal;
            int dwritten = dataVal;
            std::cout << "at " << index << " written "<< dwritten <<  " read " << dread 
                << " written "<< dataVal <<  " read " << verifyVal << std::endl;
            vfailed = true;
        };
    }
    if(vfailed) throw RodException("writeBlockToFlashHpi verify failed. index, data:", index, dataVal);
    delete [] buffer;
    return;
}

//-----------------------------readByteFromFlash------------------------------

UINT8 RodModule::readByteFromFlash(unsigned long address, long updateAddress) 
    throw (RodException &, VmeException &){
        UINT8 dataByte;
        unsigned long commandReg;
        clock_t startTime;
        unsigned long handshakeBitValue = 0;
        unsigned long valueD32;

        if (updateAddress) {
            m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[4], address);
        }
        // Set rd bit
        setBit(&handshakeBitValue, READ_HANDSHAKE_BIT);
        m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[3], handshakeBitValue);

        // Wait for valid data
        startTime = clock();
        while (1) {
            commandReg = m_myVmePort->read32(FPGA_CONTROL_REG_REL_ADDR[3]);
            if (0==readBit(commandReg, READ_HANDSHAKE_BIT)) break;
            if ((clock()-startTime)>FLASH_TIMEOUT) throw RodException(
                    "Timeout in readByteFromFlash. Address=", address);
        }

        // Read valid data
        valueD32 = m_myVmePort->read32(FPGA_STATUS_REG_REL_ADDR[7]); 
        dataByte = (UINT8)(valueD32&0xFF);
        return dataByte;
    }

//---------------------------vmeWriteElementFlash------------------------------

void RodModule::vmeWriteElementFlash(UINT8 value, unsigned long address, 
        long handshakeBit) throw (RodException &, VmeException &) {
    unsigned long ctrlReg4Val;  // address(23:0) + data(31:24)
    unsigned long commandReg;
    clock_t startTime;
    unsigned long handshakeBitValue=0;

    ctrlReg4Val = (value<<24)| address;
    m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[4], ctrlReg4Val);

    // Set wr bit
    setBit(&handshakeBitValue, handshakeBit);
    m_myVmePort->write32(FPGA_CONTROL_REG_REL_ADDR[3], handshakeBitValue);

    // Wait for ack
    startTime = clock();
    while(1) {
        commandReg = m_myVmePort->read32(FPGA_CONTROL_REG_REL_ADDR[3]);
        if (0==readBit(commandReg, handshakeBit)) break;
        if ((clock()-startTime)>FLASH_TIMEOUT) throw RodException(
                "Timeout in vmeWriteElementFlash. Address, value=", 
                address, (unsigned long)value);
    }
    return;   
}

//-----------------------------readBlockFromFlash------------------------------

void RodModule::readBlockFromFlash(unsigned long address, UINT8 *buffer, 
        unsigned long numBytes) throw(RodException &, VmeException &) {
    unsigned long index;
    for (index=0; index<numBytes; ++index) {
        buffer[index] = readByteFromFlash(address+index, 1);
    }
    return;
}

//-----------------------------commonEraseCommands----------------------------

void RodModule::commonEraseCommands(unsigned long flashBaseAddr) 
    throw(RodException &, VmeException &) {

        const unsigned long addr1 = flashBaseAddr+0x5555;
        const unsigned long addr2 = flashBaseAddr+0x2AAA;

        // 1st command
        vmeWriteElementFlash(0xAA, addr1, WRITE_COMMAND_HANDSHAKE_BIT);

        // 2nd command
        vmeWriteElementFlash(0x55, addr2, WRITE_COMMAND_HANDSHAKE_BIT);

        // 3rd command
        vmeWriteElementFlash(0x80, addr1, WRITE_COMMAND_HANDSHAKE_BIT);

        // 4th command
        vmeWriteElementFlash(0xAA, addr1, WRITE_COMMAND_HANDSHAKE_BIT);

        // 5th command
        vmeWriteElementFlash(0x55, addr2, WRITE_COMMAND_HANDSHAKE_BIT);

        return;
    }

//----------------------------commonEraseCommandsHpi--------------------------

void RodModule::commonEraseCommandsHpi(unsigned long flashBaseAddr) 
    throw(VmeException &) {

        unsigned long buffer;
        const unsigned long addr1 = flashBaseAddr+(0x5555<<2);
        const unsigned long addr2 = flashBaseAddr+(0x2AAA<<2);

        // 1st command
        buffer = 0xAA;
        mdspSingleWrite(addr1, buffer);

        // 2nd command
        buffer = 0x55;
        mdspSingleWrite(addr2, buffer);

        // 3rd command
        buffer = 0x80;
        mdspSingleWrite(addr1, buffer);

        // 4th command
        buffer = 0xAA;
        mdspSingleWrite(addr1, buffer);

        // 5th command
        buffer = 0x55;
        mdspSingleWrite(addr2, buffer);

        return;
    }

//---------------------------getFlashSectorSize---------------------------                                

unsigned long RodModule::getFlashSectorSize() {
    unsigned long sectorSize;
    switch (m_revision) {
        case 0x0f: 
        case 0x0e:
            sectorSize = FLASH_SECTOR_SIZE_REVE;
            break;
        case 0x0b:
        case 0x0c:
        default:
            sectorSize = FLASH_SECTOR_SIZE;
            break;
    }
    return sectorSize;
}

//----------------------------------sleep-------------------------------------                                
// ks: replaced buy an inline call to usleep(), which will suspend this thread and 
//     gives other threads the chances tpo talk to there rods 
/*void RodModule::sleep(const double milliSecs) {
    clock_t start, delay;
    delay = CLOCKS_PER_SEC;
    delay = clock_t(milliSecs * CLOCKS_PER_SEC / 1000 ); //need explicit type cast
    //to avoid warning.
    start = clock();
    while (clock()-start< delay)   // wait until time elapses
        ;
    return;
}*/
//--------------------------------checkSum------------------------------------                                

unsigned long RodModule::checkSum(const unsigned long *sourceArray, const long wordCount) {
    unsigned long result=0;
    long i;

    for (i=0; i<wordCount; ++i) {
        result ^= sourceArray[i];
    }
    return result;
}

//-----------------------------endianReverse32----------------------------------                                

unsigned long RodModule::endianReverse32(const unsigned long inVal) {
    unsigned long outVal;
    outVal  = (inVal & 0x000000ff) << 24;
    outVal |= (inVal & 0x0000ff00) << 8;
    outVal |= (inVal & 0x00ff0000) >> 8;
    outVal |= (inVal & 0xff000000) >> 24;
    return outVal;
}

//----------------------------------bocCheck------------------------------------                                
/*! Checks BOC status. If no BOC clock or BOC is busy, it returns 0; if BOC is OK it returns 1.
  It also sets the data member m_bocFlag as follows:
  0 = BOC OK
  1 = No clock from BOC
  2 = BOC is busy
 */
unsigned long RodModule::bocCheck() {
    unsigned long fpgaReg=0x404420;
    unsigned long clockBit=0x2, bocBusyBit=0x4;
    unsigned long regVal;
    regVal = mdspSingleRead(fpgaReg);
    //AKDEBUG careful this depends on the FPGA version due to a previous bug ....
    if ((regVal&clockBit)==1) {            // No clock
        m_bocFlag = 1;
        return 0;
    }
    if ((regVal&bocBusyBit)==1) {          // BOC busy
        m_bocFlag = 2;
        return 0;
    }
    m_bocFlag = 0;
    return 1;
}

void RodModule::readMasterMem(int startAddr, int nWords, std::vector<unsigned int> &out) {
    unsigned long *buf;
    buf = new unsigned long[nWords];
    mdspBlockRead(startAddr, buf, nWords); 
    for (int i=0; i<nWords; i++) out.push_back(buf[i]);
    delete[] buf;
}

void RodModule::writeMasterMem(int startAddr, int nWords, std::vector<unsigned int> &in) {
    unsigned long *buf;
    buf = new unsigned long[nWords];
    for (int i=0; i<nWords; i++) buf[i] = in[i];
  mdspBlockWrite(startAddr, buf, nWords);
  delete[] buf;
}

void RodModule::readSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out) {
  RwSlaveMemoryIn RSlaveMemoryIn;
  int slaveAddr = startAddr;
  int readWords = 0;

  out.clear();
  while((nWords>readWords)){
 
 // TBD JD: Find, whether we're good here.
 // Debug
 //    std::cout << "Just testing whether this procedure does anything at all..." << std::endl;
 //    std::cout << " RodModule::readSlaveMem nWords: " << nWords << " readWords: " << readWords << std::endl; 
    RSlaveMemoryIn.slave      = slaveId;
    RSlaveMemoryIn.flags      = RW_READ;
    RSlaveMemoryIn.slaveAddr  = (UINT32)slaveAddr;
    RSlaveMemoryIn.masterAddr = (UINT32)NULL ;
    RSlaveMemoryIn.nWords     = nWords - readWords;
    
    RodPrimitive* rwsmPrim = new RodPrimitive(sizeof(RwSlaveMemoryIn)/sizeof(UINT32), 0, 
					      RW_SLAVE_MEMORY, R_BOGUS, 
					      (long int*)&RSlaveMemoryIn);
    RodOutList* outList;
    executeMasterPrimitiveSync(*rwsmPrim, outList);
    delete rwsmPrim;
    if (outList == NULL) {
      return;
    } else {
      unsigned long *outBody = (unsigned long*)outList->getMsgBody(1);
      RwMemoryReplyHeader * RwMemoryReplyHeader_Out = (RwMemoryReplyHeader *) outBody;
      //Debug
      // std::cout << "Header says - Read nWords: " << RwMemoryReplyHeader_Out->nWords << " starting from 0x" << std::hex << RwMemoryReplyHeader_Out->addr << std::dec << " on Slave " << RwMemoryReplyHeader_Out->id << std::endl;
      readWords += RwMemoryReplyHeader_Out->nWords;      
      slaveAddr += sizeof(UINT32) * RwMemoryReplyHeader_Out->nWords;
      /* break if we don't read anything */
      if(0==RwMemoryReplyHeader_Out->nWords)readWords += nWords;
      for (unsigned int i=(sizeof(RwMemoryReplyHeader)/sizeof(UINT32)); i<((sizeof(RwMemoryReplyHeader)/sizeof(UINT32)) + RwMemoryReplyHeader_Out->nWords); i++) {
	out.push_back(outBody[i]);
      }
      deleteOutList();
    }
  }
}

void RodModule::writeSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in) {
  long int* rwsmData = new long int[nWords + sizeof(RwSlaveMemoryIn)/4];
  RwSlaveMemoryIn* WSlaveMemoryIn = (RwSlaveMemoryIn*)rwsmData;
  WSlaveMemoryIn->slave      = slaveId;
  WSlaveMemoryIn->flags      = RW_WRITE;
  WSlaveMemoryIn->slaveAddr  = (UINT32)startAddr;
  WSlaveMemoryIn->masterAddr = (UINT32)NULL ;
  WSlaveMemoryIn->nWords     = nWords;

  int i;
  for (i=0; i<nWords; i++) rwsmData[sizeof(RwSlaveMemoryIn)/sizeof(UINT32)+i] = in[i];
  RodPrimitive* rwsmPrim = new RodPrimitive(sizeof(RwSlaveMemoryIn)/sizeof(UINT32)+nWords
					    , 0, RW_SLAVE_MEMORY, R_BOGUS, rwsmData);
  executeMasterPrimitiveSync(*rwsmPrim);
  delete rwsmPrim;
  delete[] rwsmData;
}

void RodModule::readSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out) {
  std::cout << "RodModule::readSlaveMemDirect  NOT fully implemented" << std::endl;
//   unsigned int cr0 = mdspSingleRead(COMMAND_REG_0);
//   mdspSingleWrite(COMMAND_REG_0, cr0+(0x1<<CR_DMA_ACCESS_REQ));
//   unsigned int sr0, repCount = 10000;
//   do {
//     sr0 = mdspSingleRead(STATUS_REG_0);
//   } while ((sr0 & (0x1<<SR_DMA_ACCESS_ACK)) == 0 && repCount-- > 0);

//   if (repCount > 0) {
    unsigned long *buf;
    buf = new unsigned long[nWords];
    slvBlockRead(startAddr, buf, nWords, slaveId); 
    for (int i=0; i<nWords; i++) out.push_back(buf[i]);
    delete[] buf;
//     mdspSingleWrite(COMMAND_REG_0, cr0);
//   } else {
//   }
}

void RodModule::writeSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in) {
  std::cout << "RodModule::writeSlaveMemDirect NOT fully implemented" << std::endl;
//   unsigned int cr0 = mdspSingleRead(COMMAND_REG_0);
//   mdspSingleWrite(COMMAND_REG_0, cr0+CR_DMA_ACCESS_REQ);
//   unsigned int sr0, repCount = 10000;
//   do {
//     sr0 = mdspSingleRead(STATUS_REG_0);
//   } while ((sr0 & (0x1<<SR_DMA_ACCESS_ACK)) == 0 && repCount-- > 0);

//   if (repCount > 0) {
    unsigned long *buf;
    buf = new unsigned long[nWords];
    for (int i=0; i<nWords; i++) buf[i] = in[i];
    slvBlockWrite(startAddr, buf, nWords, slaveId);
    delete[] buf;
//   } else {
//   }
}

void RodModule::readFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &out) {
  RwFifoIn RwFifo_In;
  RwFifo_In.fifoId      = fifo;
  RwFifo_In.fRead       = RW_READ;
  RwFifo_In.length      = nWords;
  RwFifo_In.dataPtr     = (UINT32 )NULL;
  RodPrimitive* rwfPrim = new RodPrimitive(sizeof(RwFifoIn)/sizeof(UINT32), 0, 
					   RW_FIFO, R_BOGUS, (long int*)&RwFifo_In);
  RodOutList* outList;
  //  executeMasterPrimitiveSync(*rwfPrim, outList);
  //  executeMasterPrimitiveSync(*rwfPrim);
  synchSendPrim(*rwfPrim);
  delete rwfPrim;
  out.clear();
  outList = getOutList();
  unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);
  for (int i=0; i<outList->getMsgLength(1); i++) {
    out.push_back(outBody[i]);
  }
  deleteOutList();
}

void RodModule::writeFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &in) {
  long int* rwfData = new long int[sizeof(RwFifoIn)/sizeof(UINT32) + nWords];
  RwFifoIn *RwFifo_In = (RwFifoIn*) rwfData;
  RwFifo_In->fifoId   = fifo;
  RwFifo_In->fRead    = RW_WRITE;
  RwFifo_In->length    = nWords;
  RwFifo_In->dataPtr   = (UINT32)NULL;

  int i;
  for (i=0; i<nWords; i++) rwfData[sizeof(RwFifoIn)/sizeof(UINT32)+i] = in[i];
  RodPrimitive* rwfPrim = new RodPrimitive(sizeof(RwFifoIn)/sizeof(UINT32) + nWords, 
					   0, RW_FIFO, R_BOGUS, rwfData);
  executeMasterPrimitiveSync(*rwfPrim);
  delete rwfPrim;
  delete[] rwfData;
}

void RodModule::writeRegister(int regId, int offset, int size, unsigned int value) {
  RwRegFieldIn 	RwRegField_In;
  RwRegField_In.regAddr    = regId;
  RwRegField_In.offset     = offset;
  RwRegField_In.width      = size;
  RwRegField_In.flags      = RW_WRITE;
  RwRegField_In.value      = value;
  RodPrimitive* rwrPrim = new RodPrimitive(sizeof(RwRegFieldIn)/sizeof(UINT32), 
					   0, RW_REG_FIELD, R_BOGUS, 
					   (long int*)&RwRegField_In);
  //schsu: There was a 100ms sleep between RodPrimList::EXECUTING and RodPrimList::WAITING
  //It slows down the register write access.
  //executeMasterPrimitiveSync(*rwrPrim);

   //Since RW_WRITE primitive has relative smaller size.
   //No oberservation of failure by removing the 100ms sleep.
   executeMasterPrimitive(*rwrPrim);
 
   RodPrimList::PrimState returnPState;
   int count = 0;
   clock_t startTime;

   startTime = time(0);
   count = 0;
   do {
     if ((time(0)-startTime)>50 && count>20) {
       throw RodException("Cannot start primitive execution, TIMEOUT=50", m_slot);
     }
     returnPState = primHandler();
     count++;
   } while (returnPState != RodPrimList::EXECUTING); 

   startTime = time(0);
   count = 0;
   do {
     if ((time(0)-startTime)>100 && count>20) {
       throw RodException("Primitive execution did not complete, TIMEOUT=100", m_slot);
     }
     returnPState = primHandler();
     count++;
   } while ((returnPState != RodPrimList::WAITING)&&(returnPState != RodPrimList::IDLE)); 

   if (returnPState == RodPrimList::WAITING) {
     getOutList();
     deleteOutList();
   }

   delete rwrPrim;
}

unsigned int RodModule::readRegister(int regId, int offset, int size) {
  RwRegFieldIn 	RwRegField_In;
  RwRegField_In.regAddr    = regId;
  RwRegField_In.offset     = offset;
  RwRegField_In.width      = size;
  RwRegField_In.flags      = RW_READ;
  RwRegField_In.value      = (UINT32)NULL;
  RodPrimitive* rwrPrim = new RodPrimitive(sizeof(RwRegFieldIn)/sizeof(UINT32), 
					   0, RW_REG_FIELD, R_BOGUS, 
					   (long int*)&RwRegField_In);
  //std::vector<unsigned int> outs;
  RodOutList* outs;
  executeMasterPrimitiveSync(*rwrPrim, outs);
  delete rwrPrim;
  RwRegFieldOut *reply = (RwRegFieldOut *)outs->getMsgBody(1);
  return reply->value;
}

void RodModule::executeMasterPrimitive(RodPrimitive &prim) {
  RodPrimList primList(1);
  primList.insert(primList.begin(), prim);
  primList.bufferBuild();
  sendPrimList(&primList);
}

void RodModule::executeMasterPrimitiveSync(RodPrimitive &prim, std::vector<unsigned int> &out) {
  executeMasterPrimitive(prim);
  waitPrimitiveCompletion(out);
}

void RodModule::executeMasterPrimitiveSync(RodPrimitive &prim, RodOutList* &out) {
  executeMasterPrimitive(prim);
  waitPrimitiveCompletion(out);
}

void RodModule::executeMasterPrimitiveSync(RodPrimitive &prim) {
  executeMasterPrimitive(prim);
  waitPrimitiveCompletion();
}

void RodModule::executeSlavePrimitive(RodPrimitive &prim, int slave) {
  int i;

  RodPrimList slvPrimList(1);
  slvPrimList.insert(slvPrimList.begin(), prim);
  slvPrimList.bufferBuild();
  
  long buffLength = slvPrimList.getBufferLength();
  unsigned long* slavePrimList = new unsigned long[buffLength];
  unsigned long* bufferStart = slvPrimList.getBuffer();
  for (i=0; i< buffLength; i++) {
    slavePrimList[i] = bufferStart[i];
  }
  
  // Create Send Slave List primitive
  RodPrimList primList(1);
  long int *slaveData = new long int[sizeof(SendSlaveListIn)/sizeof(UINT32) + 
				     buffLength]; 
  SendSlaveListIn* SendSlaveList_In = (SendSlaveListIn*)slaveData;
  SendSlaveList_In->slave         = slave;
  SendSlaveList_In->nWords        = buffLength;
  SendSlaveList_In->queueIndex    = 0;

  for (i=0; i< buffLength; i++) {
    slaveData[i+sizeof(SendSlaveListIn)/4] = slavePrimList[i];
  }
  delete slavePrimList;
  RodPrimitive* send;
  send = new RodPrimitive(sizeof(SendSlaveListIn)/sizeof(UINT32) + buffLength, 
			  0, SEND_SLAVE_LIST, R_BOGUS, slaveData);
  primList.insert(primList.begin(), *send);

  primList.bufferBuild();
  sendPrimList(&primList);
  delete send;
  delete []slaveData;
  
}

void RodModule::executeSlavePrimitiveOnAll(RodPrimitive &prim) {

  RodPrimList slvPrimList(1);
  slvPrimList.insert(slvPrimList.begin(), prim);
  slvPrimList.bufferBuild();
  
  long buffLength = slvPrimList.getBufferLength();
  unsigned long* slavePrimList = new unsigned long[buffLength];
  unsigned long* bufferStart = slvPrimList.getBuffer();
  for (int i=0; i< buffLength; i++) {
    slavePrimList[i] = bufferStart[i];
  }
  
  // Create Send Slave List primitive
    RodPrimList primList(1);

    long int *slaveData = new long int[sizeof(SendSlaveListIn)/sizeof(UINT32) + buffLength]; 
    SendSlaveListIn* SendSlaveList_In = (SendSlaveListIn*)slaveData;
    SendSlaveList_In->slave       = SLAVE_MASK | 0xf;
    SendSlaveList_In->nWords      = buffLength;
    SendSlaveList_In->nPrimitives = 1;
    SendSlaveList_In->queueIndex  = 0;

    for (int i=0; i< buffLength; i++) {
      slaveData[i+sizeof(SendSlaveListIn)/sizeof(UINT32)] = slavePrimList[i];
    }
    RodPrimitive* send;
    send = new RodPrimitive(sizeof(SendSlaveListIn)/sizeof(UINT32)+buffLength, 
			    0, SEND_SLAVE_LIST, R_BOGUS, slaveData);
    primList.insert(primList.begin(), *send);

    primList.bufferBuild();

    delete slavePrimList;
    sendPrimList(&primList);
    delete send;
}

void RodModule::executeSlavePrimitiveOnAllSync(RodPrimitive &prim) {
  executeSlavePrimitiveOnAll(prim);
  waitPrimitiveCompletion();
}

void RodModule::executeSlavePrimitiveOnAllSync(RodPrimitive &prim, std::vector<unsigned int> &out) {
  executeSlavePrimitiveOnAll(prim);
  waitPrimitiveCompletion(out);
}

void RodModule::executeSlavePrimitiveOnAllSync(RodPrimitive &prim, RodOutList* &out) {
  executeSlavePrimitiveOnAll(prim);
  waitPrimitiveCompletion(out);
}

void RodModule::executeSlavePrimitiveSync(RodPrimitive &prim, std::vector<unsigned int> &out, int slave) {
  executeSlavePrimitive(prim, slave);
  waitPrimitiveCompletion(out);
}

void RodModule::executeSlavePrimitiveSync(RodPrimitive &prim, RodOutList* &out, int slave) {
  executeSlavePrimitive(prim, slave);
  waitPrimitiveCompletion(out);
}

void RodModule::executeSlavePrimitiveSync(RodPrimitive &prim, int slave) {
  executeSlavePrimitive(prim, slave);
  waitPrimitiveCompletion();
}

void RodModule::waitPrimitiveCompletion() {
  RodPrimList::PrimState returnPState;
  int count = 0;
  clock_t startTime;

  // wait until previous primlist is finished
  startTime = time(0);
  count = 0;
  do {
    if ((time(0)-startTime)>50 && count>20) {
      throw RodException("Cannot start primitive execution, TIMEOUT=50", m_slot);
    }
    returnPState = primHandler();
    count++;
  } while (returnPState != RodPrimList::EXECUTING); 

  //schsu: 100ms sleep is selected from several experimental trials
  //In the case of RodModule::loadAndStart or the RodPixController::writeModuleConfig 
  //both has large size of primitives. The primHandler() can sometimes returns EXECUTING
  //state without this 100ms delay
  sleep(100);

  startTime = time(0);
  count = 0;
  do {
    if ((time(0)-startTime)>100 && count>20) {
      throw RodException("Primitive execution did not complete, TIMEOUT=100", m_slot);
    }
    returnPState = primHandler();
    count++;
  } while ((returnPState != RodPrimList::WAITING)&&(returnPState != RodPrimList::IDLE)); 
  if (returnPState == RodPrimList::WAITING) {
    getOutList();
    deleteOutList();
  }
}

void RodModule::waitPrimitiveCompletion(std::vector<unsigned int> &out) {
  RodPrimList::PrimState returnPState;
  int count = 0;
  clock_t startTime;

  startTime = time(0);
  count = 0;
  do {
    if ((time(0)-startTime)>50 && count>20) {
      throw RodException("Cannot start primitive execution, TIMEOUT=50", m_slot);
    }
    returnPState = primHandler();
    count++;
  } while (returnPState != RodPrimList::EXECUTING); 

//  sleep(100);

  startTime = time(0);
  count = 0;
  do {
    if ((time(0)-startTime)>100 && count>20) {
      throw RodException("Primitive execution did not complete, TIMEOUT=100", m_slot);
    }
    returnPState = primHandler();
    count++;
  } while ((returnPState != RodPrimList::WAITING)&&(returnPState != RodPrimList::IDLE)); 
  RodOutList* outList = getOutList();
  out.clear();
  if (returnPState == RodPrimList::WAITING) {
    unsigned long *outBody = outList->getBody();
    for (int i=0; i<outList->getLength(); i++) {
      out.push_back(outBody[i]);
    }
    deleteOutList();
  }
}

void RodModule::waitPrimitiveCompletion(RodOutList* &out) {
  RodPrimList::PrimState returnPState;
  int count = 0;
  clock_t startTime;

  startTime = time(0);
  count = 0;
  do {
    if ((time(0)-startTime)>50 && count>20) {
      throw RodException("Cannot start primitive execution, TIMEOUT=50", m_slot);
    }
    returnPState = primHandler();
    count++;
  } while (returnPState != RodPrimList::EXECUTING); 

//  sleep(100);

  startTime = time(0);
  count = 0;
  do {
    if ((time(0)-startTime)>100 && count>20) {
      throw RodException("Primitive execution did not complete, TIMEOUT=100", m_slot);
    }
    returnPState = primHandler();
    count++;
  } while ((returnPState != RodPrimList::WAITING)&&(returnPState != RodPrimList::IDLE)); 
  out = getOutList();
  if (returnPState != RodPrimList::WAITING) {
    deleteOutList();
    out = NULL;
  }
}

void RodModule::initStatusRead() {

    StartTaskIn *startTaskIn = (StartTaskIn*) new UINT32[((sizeof(StartTaskIn))/sizeof(UINT32))];
	
    startTaskIn->id = STATUS_READ;
    startTaskIn->idMinor = 0;
    startTaskIn->where   = DSP_THIS;
    startTaskIn->dataPtr = 0;
    
    RodPrimitive* startStatusRead;
    try {
      startStatusRead = new RodPrimitive(((sizeof(StartTaskIn))/sizeof(UINT32)), 0, START_TASK, 0, (long*)startTaskIn);
    }
    catch (std::bad_alloc) {
      std::cout << "Unable to allocate startStatusRead primitive in main." << std::endl;
    }
    executeMasterPrimitiveSync(*startStatusRead);
    delete startStatusRead;
    delete []startTaskIn;

}

QuickStatusInfo* RodModule::talkStatusRead() {

    TalkTaskIn *talkTaskIn = (TalkTaskIn*) new UINT32[((sizeof(TalkTaskIn))/sizeof(UINT32))];
    
    talkTaskIn->task = STATUS_READ;
    talkTaskIn->topic = 0;
    talkTaskIn->item = 0;
    
    RodPrimitive* talkStatusRead;
    try {
      talkStatusRead = new RodPrimitive(((sizeof(talkTaskIn))/sizeof(UINT32)), 0, TALK_TASK, 0, (long*)talkTaskIn);
    }
    catch (std::bad_alloc) {
      std::cout << "Unable to allocate talkStatusRead primitive in main." << std::endl;
    }
    
    RodOutList* statusInfoAddress; 
    
    sleep(5);
    
    executeMasterPrimitiveSync(*talkStatusRead,statusInfoAddress);
    UINT32* messageBody = (UINT32*)statusInfoAddress->getMsgBody(1);
    TalkTaskOut *tout = (TalkTaskOut *)messageBody;
    UINT32 address = 0;
    if(0==tout->dataPtr){
      address = messageBody[sizeof(TalkTaskOut)/sizeof(UINT32)];
    }else{
      address = tout->dataPtr;
    }
    printf("******Address of statusInfo struct: %x \n", address);

    unsigned long numWords = 60;

    unsigned long * buffer = new unsigned long[numWords];
    mdspBlockRead(address, buffer, numWords);
    QuickStatusInfo* quickStatusInfo = (QuickStatusInfo*)buffer;

    delete talkStatusRead;
    delete []talkTaskIn;

    return quickStatusInfo;

}

} //  End namespace SctPixelRod

//------------------------- Overload insertion operators "<<" -------------------------
/* This overloaded operator lets us use cout to print the status of the ROD
*/
  std::ostream& operator<<(std::ostream& os, SctPixelRod::RodModule& rod) {
    os << "Slot: " << rod.getSlot() << std::endl;
    os << "Serial Number(hex):" << std::hex << rod.getSerialNumber() << std::endl;
    os << "Number of slave DSPs: " << rod.getNumSlaves() << std::endl;

    os << "Primitive state: " << rod.getPrimState() 
       << std::dec << std::endl;
    return os;
  }


