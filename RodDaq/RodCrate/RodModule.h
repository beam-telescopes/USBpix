//File: RodModule.h

#ifndef SCTPIXELROD_RODMODULE_H 
#define SCTPIXELROD_RODMODULE_H

#include <ctime>     // Pick up clock() function and clock_t typedef
#include <new>
#include <unistd.h>
#include <fstream> 
#include <iostream>
#include <string>
#include <vector>

#include "BaseException.h"
#include "RodPrimitive.h" 
#include "RodPrimList.h"
#include "RodDspAddresses.h"
#include "RodVmeAddresses.h"
#include "RodOutList.h"
#include "rodXface.h"
#include "VmeInterface.h" 
#include "VmePort.h"
#include "VmeModule.h"

namespace SctPixelRod {

// Forward definition
class BocCard;

enum HpidMode {DYNAMIC, AUTO, NO_AUTO};

//------------------------------NoImageFile--------------------------------- 

/*! @class NoImageFile
 *
 *  @brief This is an exception class for when a binary image file is not found.
 *  It inherits from BaseException, adding the name of the missing file as a data member.
 *
 *  If we try to load a binary image for a master or slave DSP and the file is 
 *  not found, an object of this class is thrown.
 *
 *  @author Tom Meyer (meyer@iastate.edu) - originator
 */

class NoImageFile : public BaseException {
public:
  NoImageFile( std::string descriptor, std::string fileName);
  std::string getFileName() {return m_fileName;};
  virtual void what(std::ostream&);

private:
  std::string m_fileName;
};                                                                             

//------------------------------HpiException--------------------------------- 

/*! @class HpiException
 *
 *  @brief This is an exception class for Host Port Interface errors.
 *  It inherits from BaseException, adding the expected and actual addresses.

 *  This class is thrown if an error in an HPI read/write operation is detected.
 *
 *  @author Tom Meyer (meyer@iastate.edu) - originator
 */

class HpiException : public BaseException {
public:
  HpiException( std::string descriptor, unsigned long calcAddr, unsigned long readAddr);
  unsigned long getCalcAddr() {return m_calcAddr;};
  unsigned long getReadAddr() {return m_readAddr;};
  virtual void what(std::ostream&);

private:
  unsigned long m_calcAddr;    // The calculated (expected) address
  unsigned long m_readAddr;    // The address actually read
};                                                                            

//------------------------------RodException------------------------------ 

/*! @class rodException
 *
 *  @brief This is a general exception class for ROD errors.
 *
 *  This class is thrown if an error in a ROD operation is detected.
 *  It inherits from BaseException, adding zero, one, or two parameters.
 *
 *  @author Tom Meyer (meyer@iastate.edu) - originator
 */

class RodException : public BaseException {
public:
  RodException( std::string descriptor);
  RodException( std::string descriptor, unsigned long data1);
  RodException( std::string descriptor, unsigned long data1, unsigned long data2);
  unsigned long getData1() {return m_data1;};
  unsigned long getData2() {return m_data2;};
  unsigned long getNumData() {return m_numData;};
  virtual void what(std::ostream&);
  
private:
  unsigned long m_numData;  // Number of data values returned
  unsigned long m_data1;    // First data value returned
  unsigned long m_data2;    // Second data value returned
};                                                                            


enum FpgaType {CONTROLLER, FMTA, FMTB, EFR, ROUTER, ALL};
enum DspType {MODE, MASTER, SLV0, SLV1, SLV2, SLV3};

//------------------------------------------------------------------------------              

 class RodModule : public VmeModule {
  RodModule(const RodModule& rhs);
  RodModule &operator=(const RodModule & rhs);

public:

  // Constructors, destructor, and assignment
  RodModule( unsigned long baseAddr, unsigned long mapSize, VmeInterface & ourInterface, 
             long numSlaves) throw (RodException&, VmeException &);
  virtual ~RodModule();                                  // Destructor
  
  // Accessor functions

  bool isSane(); 
  unsigned long fmtVersion();
  unsigned long efbVersion();
  unsigned long rtrVersion();
  unsigned long rcfVersion();

  unsigned long getVmeCommandRegVal() {return m_vmeCommandReg;};
  VmePort* getVmePort() {return m_myVmePort; };
    
  void setDebug(unsigned int debug) {m_debug = debug; }
  unsigned int getDebug() {return m_debug; }
  void setDebugFile(const std::string &debugFile) {m_debugFile = debugFile; }
  long getSlot() const { return m_slot; }
  unsigned long getSerialNumber() {return m_serialNumber;};
  unsigned long getRevision() {return m_revision;};
  void setNumSlaves( long numSlaves) { m_numSlaves = numSlaves; }
  long getNumSlaves() const { return m_numSlaves; }
  void setFinBufferSize( long finBufferSize) { m_finBufferSize = finBufferSize;}
  long getFinBufferSize() const { return m_finBufferSize; }
  std::string getMasterImageName() {return m_masterImageName; }
  void setOutList(RodOutList* outList) {m_myOutList = outList;};
  RodOutList* getOutList() {return m_myOutList;};
  RodPrimList::PrimState getPrimState() {return m_myPrimState;};
  void setBoc(BocCard* myBoc){m_myBoc=myBoc; return;};;
  BocCard* getBocCard(){return m_myBoc; };
  unsigned long getBocFlag() {return m_bocFlag; };
  unsigned long getBaseAddress() {return m_baseAddress; }

/*! initialize() performs all actions necessary to place a properly loaded ROD 
in its initialized state, except for initializing the slave DSPs. This must be done
separately using initSlaveDsp().

Activities:
    Reset the ROD
    Read ROD serial number
    Load 0x000l000l into Master DSP HPIC register. (Says low order 16-bit word
                                                    comes first)
    Read back HPIC to verify
    Retrieve pointers to text buffer structs
    Initialize PrimList and TextBuff state variables
*/
  void initialize();
  void initialize(bool resetFlag) throw (RodException &, VmeException &);
  
/*! reset() issues a reset signal to the ROD, clears IDRAM, resets all DSPs, and
 *  re-initializes all state variables.
*/
  void reset() throw (RodException&, VmeException &);

/*! fpgaReset() resets individual FPGAs on the ROD. The ROD can reset only one or all at a time
 */
  void fpgaReset(FpgaType targetFpga) throw (RodException&, VmeException &);

/*! dspReset() resets individual DSPs on the ROD. The ROD can reset only one or all at a time
 */
  void dspReset(DspType targetDsp) throw (RodException&, VmeException &);

/*! status() reports the status of the ROD.
 *  For now, it simply prints to standard output. Later we will add a structured
 *  report of name=value pairs.
*/
  void status();

/*! verify checks the h.o. byte of the serial number register to see that it contains
 *  the hex value 'ad', which is the signature of a ROD. It returns true if this is\
 *  the case, false otherwise.
*/
  bool verify();
  
/*! loadAndStartSlaves() reads a binary file from disk and loads it into the slave
 *  memory space and boots up the specified Slaves
 *  !!!THIS IS THE PREFERRED WAY TO LOAD A SLAVE IMAGE!!! - do not use
 *  hpiLoadSlaveImage without having a good reason to do so.
 */
  int loadAndStartSlaves(const std::string & filename, 
			 const int slaveMask) 
        throw (NoImageFile &, RodException &, VmeException &);

  int loadAndStartFpgaSlaves(const std::string & filename, 
			 const int slaveMask) 
        throw (NoImageFile &, RodException &, VmeException &);

/*! startSlave() just starts the slave and checks that it has  booted
 */

  int startSlave(const int slaveId);
     
/*! loadSlaveImage() reads a binary file from disk and loads it into the slave
 *  memory space at the specified location using the RW_SLAVE_MEMORY primitive.
 *  !!!THIS IS THE PREFERRED WAY TO LOAD A SLAVE IMAGE!!! - do not use
 *  hpiLoadSlaveImage without having a good reason to do so.
 *  Usually, the optional last argument should be omitted.
 */
  void loadSlaveImage(const std::string & filename, const unsigned long address,
        const long slaveNumber, char opt='n') 
        throw (NoImageFile &, RodException &, VmeException &);
                      
/*! hpiLoadSlaveImage() reads a binary file from disk and loads it into the slave
 *  memory space at the specified location.
 *  !!!THIS IS NO LONGER THE PREFERRED WAY. YOU'D BETTER HAVE A GOOD READSON TO USE
 *  IT !!!
 *  Usually, the optional last argument should be omitted.
 */
  void hpiLoadSlaveImage(const std::string & filename, const unsigned long address,
        const long slaveNumber, char opt='n') 
        throw (NoImageFile &, RodException &, VmeException &);
                      
/*! startSlave() sends a START_SLAVE_EXECUTING primitive to start a given slave
 *  DSP. Usually, the optional last argument should be omitted.
 */
  void startSlave(const long slaveNumber, char mode='s') throw(RodException &, VmeException &);
  
/*! synchSendPrim() synchronously sends a single primitive to the MDSP and 
 *  handles the calls to primHandler() to finish the transfer before returning 
 *  control to its calling routine.  It is not the normal way to send 
 *  primitives, but is useful for initialization where time criticality is not 
 *  so much of a factor.
 *
 *  The present version writes to cout. This needs to be changed.
 */
  void synchSendPrim(RodPrimitive & prim) throw (RodException &, VmeException &) ;

/*! sendPrimList and sendSlavePrimList send a PrimList to the respective buffers
 */
  void sendPrimList(RodPrimList *l) throw(PrimListException &, VmeException &) ;
  void sendSlavePrimList(RodPrimList *slvPrimList, int slave) throw(PrimListException &, VmeException &) ;

/*! validateReplyBuffer() checks the consistency of the reply buffer */
int validateReplyBuffer() throw ( VmeException &); 

/*! primHandler() handles steps 2-4 in sending a primitive list to
 *  the master DSP. Its action depends on the value of the m_myPrimState state
 *  variable.
 */
  RodPrimList::PrimState primHandler() throw (RodException &, VmeException &);
  
//! Delete reply buffer object
  void deleteOutList();

/*! getTextBuffer() reads a complete text buffer into the return 
 *  string rettxtBuff. buffnum selects  0 for ERR buffer, 1 for INFO buffer. 
 *  If reset is true, the textBuffers on the DSP are emptied after 
 *  beeing read.
 */
  bool getTextBuffer(std::string &rettxtBuf, unsigned int buffnum, 
		     bool reset) throw(VmeException &) ;
  
//! Inline method to set VmeCommand Register Bit
  inline void setVmeCommandRegBit(const long bitNumber) {
    //AKDEBUG
/*     m_vmeCommandReg = mdspSingleRead(COMMAND_REGISTER); */
/*     setBit(&m_vmeCommandReg, bitNumber); */
/*     mdspSingleWrite(COMMAND_REGISTER, m_vmeCommandReg); */
    return;
  };
  
//! Inline method to clear VmeCommand Register Bit
  inline void clearVmeCommandRegBit(const long bitNumber) {
    //AKDEBUG
/*     m_vmeCommandReg = mdspSingleRead(COMMAND_REGISTER); */
/*     clearBit(&m_vmeCommandReg, bitNumber); */
/*     mdspSingleWrite(COMMAND_REGISTER, m_vmeCommandReg); */
    return;
  };
  
/*! hpiLoad() loads a 32-bit value into the HPIA or HPID register. Although 
 *  the HPI has a 16-bit data path, we use a 32-bit VME access and the word is 
 *  broken into two 16-bit values in the ROD firmware.
*/
  void hpiLoad(const unsigned long hpiReg, const unsigned long hpiValue) 
       throw(VmeException &);

/*! hpiFetch() fetches a 32-bit value from one of the HPI registers. Although 
 *  the HPI has a 16-bit data path, we use a 32-bit VME access, with the ROD 
 *  firmware doing the necessary conversion.
*/
  unsigned long hpiFetch(const unsigned long hpiReg) throw(VmeException &);

/*! mdspSingleRead() reads a single 32-bit word from the Master DSP SDRAM via 
 *  its Host Port Interface (HPI).
*/
  unsigned long mdspSingleRead(const unsigned long dspAddr) throw(VmeException &);

/*! mdspSingleWrite() writes a single 32-bit word to the Master DSP SDRAM via 
 *  its Host Port Interface (HPI).
*/
  void mdspSingleWrite(unsigned long dspAddr, unsigned long buffer) 
    throw(VmeException &);

/*! mdspBlockRead() reads a block of size wordCount 32-bit words from the Master 
 *  DSP SDRAM via its Host Port Interface (HPI). 
 *  The present version uses a loop over single word reads instead of a block
 *  read.
*/
  void mdspBlockRead(const unsigned long dspAddr, unsigned long *buffer, 
       long wordCount, HpidMode mode=DYNAMIC) 
       throw (HpiException &, VmeException &);

/*! mdspBlockWrite() writes a buffer of wordCount 32-bit words to the Master DSP 
 *  SDRAM via its Host Port Interface (HPI).
 *  The present version uses a loop over single word writes instead of a block
 *  write.
*/
  void mdspBlockWrite(const unsigned long dspAddr, unsigned long *buffer, 
       long wordCount, HpidMode mode=DYNAMIC) 
       throw (HpiException &, VmeException &);

/*! mdspBlockDump() reads a block of MDSP memory and write it to a binary file 
 *  (debug use only)
 */
  void mdspBlockDump(const unsigned long firstAddress, 
       const unsigned long lastAddress, const std::string & fileName) 
       throw(RodException &, VmeException &);
                     
/*! slvHpiLoad() loads a 32-bit value into a slave's HPIA or HPID register. 
 *  Endian swapping, if needed, is done in mdspSingleWrite.
 */
  void slvHpiLoad(unsigned long hpiReg, unsigned long hpiValue,
                           long slaveNum) throw(VmeException &);
                           
/*! slvHpiFetch() fetches a 32-bit value from a slave's HPI registers. 
 *  Endian swapping, if needed, is done in mdspSingleRead.
 */
  unsigned long slvHpiFetch(unsigned long hpiReg, long slaveNum) 
                throw(VmeException &);

/*! slvHpiSingleRead() reads a single 32-bit word from the the memory space of a 
 *  slave DSP. Endian swapping, if needed, is done in mdspSingleRead.
 *  Note: this is not the method you should normally use; it risks
 *  conflicts with MDSP access to the slave memory. Use slvSingleRead() instead.
 */
  unsigned long slvHpiSingleRead(unsigned long dspAddr, long slaveNum ) 
                throw(VmeException &, RodException &);
                                          
/*! slvHpiSingleWrite() writes a single 32-bit word to a slave DSP SDRAM via its
 *  Host Port Interface (HPI). Endian swapping, if needed, is done in 
 *  mdspSingleWrite.  Note: this is not the method you should normally use; it risks
 *  conflicts with MDSP access to the slave memory. Use slvSingleWrite() instead.
 */
  void slvHpiSingleWrite(unsigned long dspAddr, unsigned long buffer, 
       long slaveNum) throw(VmeException &, RodException &);

/*! slvHpiBlockRead() reads a block of size wordCount 32-bit words from a slave 
 *  DSP's SDRAM via its Host Port Interface (HPI). Endian swapping, if needed, 
 *  is done in mdspBlockRead. Usually, the optional last argument should be
 *  omitted. Note: this is not the method you should normally use; it risks
 *  conflicts with MDSP access to the slave memory. Use slvBlockRead() instead.
 */
  void slvHpiBlockRead(const unsigned long dspAddr, unsigned long buffer[],
       const long wordCount, long slaveNum, HpidMode mode=DYNAMIC) 
       throw (HpiException &, VmeException &, RodException &);
                
/*! slvHpiBlockWrite() writes a buffer of wordCount 32-bit words to a slave DSP's 
 *  SDRAM via its Host Port Interface (HPI). Endian swapping, if needed, is 
 *  done in mdspBlockWrite. Usually, the optional last argument should be
 *  omitted. Note: this is not the method you should normally use; it risks
 *  conflicts with MDSP access to the slave memory. Use slvBlockWrite() instead.
 */
  void slvHpiBlockWrite(unsigned long dspAddr, unsigned long buffer[], 
       const long wordCount, long slaveNum, HpidMode mode=DYNAMIC) 
        throw (HpiException &, VmeException &, RodException &);
         
/*! slvSingleRead() reads a single 32-bit word from the the memory space of a 
 *  slave DSP via the RW_SLAVE_MEMORY primitive. This is method is STRONGLY preferred
 *  over using slvHpiSingleRead since it does not risk conflicting with MDSP access of 
 *  the slave's memory at the same time.
 */
  unsigned long slvSingleRead(unsigned long dspAddr, long slaveNum ) 
                throw(VmeException &, RodException &);
                                          
/*! slvSingleWrite() writes a single 32-bit word to a slave DSP SDRAM via the 
 *  RW_SLAVE_MEMORY primitive. This is method is STRONGLY preferred over using 
 *  slvHpiSingleWrite since it does not risk conflicting with MDSP access of the slave's 
 *  memory at the same time.
 */
  void slvSingleWrite(unsigned long dspAddr, unsigned long buffer, 
       long slaveNum) throw(VmeException &, RodException &);

/*! slvBlockRead() reads a block of size wordCount 32-bit words from a slave 
 *  DSP's SDRAM via the RW_SLAVE_MEMORY primitive. This is method is STRONGLY
 *  preferred over using slvHpiBlockRead since it does not risk conflicting with MDSP
 *  access of the slave's memory at the same time. Usually, the optional last 
 *  argument should be omitted.
 */
  void slvBlockRead(const unsigned long dspAddr, unsigned long buffer[],
       const long wordCount, long slaveNum, HpidMode mode=DYNAMIC) 
       throw (HpiException &, VmeException &, RodException &);
                
/*! slvPrimBlockWrite() writes a buffer of wordCount 32-bit words to a slave DSP's 
 *  SDRAM via the RW_SLAVE_MEMORY primitive. This is method is STRONGLY
 *  preferred over using slvHpiBlockWrite since it does not risk conflicting with MDSP
 *  access of the slave's memory at the same time.
 *  Usually, the optional last argument should be omitted.
 */
  void slvBlockWrite(unsigned long dspAddr, unsigned long buffer[], 
       const long wordCount, long slaveNum, HpidMode mode=DYNAMIC) 
        throw (HpiException &, VmeException &, RodException &);
         
/*! resetMasterDsp() resets the Master DSP via FPGA Control Register 2
 */
  void resetMasterDsp() throw(RodException &, VmeException &);
  
/*! resetSlaveDsp() resets a slave DSP via FPGA Control Register 2
 */
  void resetSlaveDsp(long slaveNumber) throw(RodException &, VmeException &);
  
/*! resetAllDsps() resets all five DSPs via FPGA Control Register 2
 */
  void resetAllDsps() throw(RodException &, VmeException &);
  
/*! chipEraseHpi(VmeException &) erases the master program flash memory chip.
 */
  void chipEraseHpi() throw(VmeException &);
  
/*! sectorErase() erases a sector of FPGA flash memory.
 */
  void sectorErase(unsigned long sectorBaseAddress) 
       throw(RodException &, VmeException &) ;
  
/*! writeByteToFlash() writes a byte to FPGA flash memory.
 */
  void writeByteToFlash(unsigned long address, UINT8 data) 
       throw (RodException &, VmeException &);
  
/*! writeBlockToFlash() writes a block to FPGA flash memory.
 */
  void writeBlockToFlash(unsigned long address, UINT8 *data, 
                      unsigned long numBytes) throw(RodException &, VmeException &);
                      
/*! writeBlockToFlashHpi() writes a block to the master program flash memory.
 */
  void writeBlockToFlashHpi(unsigned long address, UINT8 *data, unsigned long numBytes)
                            throw (RodException &, VmeException &);
                            
/*! readByteFromFlash( reads a byte from FPGA flash memory.
 */  
  UINT8 readByteFromFlash(unsigned long address, long updateAddress) 
                            throw (RodException &, VmeException &);
                            
/*! vmeWriteElementFlash() does a VME write of an element to an FPGA flash.
 */  
  void vmeWriteElementFlash(UINT8 value, unsigned long address, 
                 long handshakeBit) throw (RodException &, VmeException &);
                 
/*! readBlockFromFlash() reads a block from FPGA flash memory.
 */  
  void readBlockFromFlash(unsigned long address, UINT8 *buffer, unsigned long
                          numBytes) throw(RodException &, VmeException &) ;
                          
/*! commonEraseCommands() implements common erase commands for the FPGA flash.
 */  
  void commonEraseCommands(unsigned long flashBaseAddr) throw(RodException &, VmeException &);
  
/*! commonEraseCommandsHpi() implements common erase commands for the master 
 *  program flash.
 */  
  void commonEraseCommandsHpi(unsigned long flashBaseAddr) throw(VmeException &);
   
/*! getFlashSectorSize() test the board revision and returns the correct value
 *  for the flash sector size.
 */
  unsigned long getFlashSectorSize();
  
/*! sleep() sleeps for a given number of milliseconds, VmeException &
*/
  //void sleep(const double milliSecs);
  inline void sleep(const unsigned long milliSecs){usleep(milliSecs*1000);}
/*! checkSum() calculates a bit-wise XOR checksum on an array
*/
  unsigned long checkSum(const unsigned long *sourceArray, const long wordCount);
  
/*! endianReverse32( reverses the endian-ness of a 32 bit value
*/
  unsigned long endianReverse32(const unsigned long inVal);
  
/*! Checks BOC status. If no BOC clock or BOC is busy, it returns 0; if BOC is OK it returns 1.
*/
  unsigned long bocCheck();
  
void readMasterMem(int startAddr, int nWords, std::vector<unsigned int> &out) ;
void writeMasterMem(int startAddr, int nWords, std::vector<unsigned int> &in) ;
void readSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out) ;
void writeSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in) ;
void readSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out) ;
void writeSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in) ;
void readFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &out) ;
void writeFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &in) ;
void writeRegister(int regId, int offset, int size, unsigned int value) ;
unsigned int readRegister(int regId, int offset, int size) ;
void executeMasterPrimitive(RodPrimitive &prim) ;
void executeMasterPrimitiveSync(RodPrimitive &prim, std::vector<unsigned int> &out) ;
void executeMasterPrimitiveSync(RodPrimitive &prim, RodOutList* &out) ;
void executeMasterPrimitiveSync(RodPrimitive &prim) ;
void executeSlavePrimitive(RodPrimitive &prim, int slave) ;
void executeSlavePrimitiveOnAll(RodPrimitive &prim) ;
void executeSlavePrimitiveOnAllSync(RodPrimitive &prim) ;
void executeSlavePrimitiveOnAllSync(RodPrimitive &prim, std::vector<unsigned int> &out) ;
void executeSlavePrimitiveOnAllSync(RodPrimitive &prim, RodOutList* &out) ;
void executeSlavePrimitiveSync(RodPrimitive &prim, std::vector<unsigned int> &out, int slave) ;
void executeSlavePrimitiveSync(RodPrimitive &prim, RodOutList* &out, int slave) ;
void executeSlavePrimitiveSync(RodPrimitive &prim, int slave) ;
void waitPrimitiveCompletion() ;
void waitPrimitiveCompletion(std::vector<unsigned int> &out) ;
void waitPrimitiveCompletion(RodOutList* &out) ;

void initStatusRead();
QuickStatusInfo* talkStatusRead();

typedef struct {
    UINT32 tail, head, base, length;
} TextQueue;

protected:
  unsigned int m_debug;              //!< Debug flag (bits of levels)
  std::string m_debugFile;           //!< Debug output filename
  long m_slot;                       //!< Slot number in VME crate
  unsigned long m_serialNumber;      //!< Board serial number (read during init)
  unsigned long m_revision;          //!< Board version (read during init)
  long m_numSlaves;                  //!< Number of slave DSPs
  unsigned int n_textbuffers;        //!< number of available textbuffers
  bool m_btFailure;                  //!< TRUE in case of  block transfer failure
  MasterXface m_masterXface;         //!< Master DSP interface
  VmePort* m_myVmePort;              //!< VME Port handle

  // pointer to buffer array for Text data
  char   **m_textData;
  UINT32  *m_textBuffer;
  UINT32  *m_textBufferSize;
     
  //! Size of file input buffer, in bytes. Default=4096 bytes, probably good enough 
  long m_finBufferSize;

  //! Cached copy of VME command registers and ROD status registers
  unsigned long m_vmeCommandReg;
  unsigned long m_rodStatusReg[2];

  //! string holding name of master image file
  std::string m_masterImageName;

  //! A pointer to the array holding a reply buffer from a PrimList
  RodOutList* m_myOutList;
  
  //! State variable for sending primitive lists
  RodPrimList::PrimState m_myPrimState;
      
  //! BOC status flag (0=OK, 1=no clock, 2=busy)
  unsigned long m_bocFlag;
    
  //! BocCard handle
  BocCard* m_myBoc;

  //! inline bit manipulation functions
  inline void setBit(unsigned long *var, long bitNumber) {
    *var = *var | (1<<bitNumber);
    return;
  }
  
  inline void clearBit(unsigned long *var, long bitNumber) {
    *var&=(~(1<<bitNumber));
    return;
  }
  
  inline bool readBit(unsigned long var, long bitNumber) {
    return ((var>>bitNumber)&1);
  }

}; // End of class RodModule declaration

}; //  End namespace SctPixelRod

//---------------------------- Overload operator<< -----------------------------
/* This overloaded operator lets us use cout to print the status of the ROD
 * and exception information.
*/
  std::ostream& operator<<(std::ostream& os, SctPixelRod::RodModule& rod);

#endif // SCTPIXELROD_RODMODULE_H

