//File: TimModule.cxx

/*! \file
 * \brief TimModule: A derived class for VME TIM modules.
 *
 * This is the implementation of a TIM class derived from the VmeModule base
 * class. It should be the sole interface for VME communication with TIM.
 *
 * Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
 *
 * $Id: TimModule.cxx 8175 2009-12-18 21:25:25Z ibragimo $
 *
 * $Log$
 * Revision 1.11  2005/11/26 13:23:40  jchill
 * Don't disable FFTV in initialize method. Also change names of defines used
 * in this file to match new names supplied by Matt Warren.
 *
 * Revision 1.10  2005/11/23 20:41:22  sctroddq
 * Bug fix to reset freqency setting.
 * 2005-11-23 AJB
 *
 * Revision 1.9  2005/05/19 13:16:40  jchill
 * Changes for TIM 3C
 *
 * Revision 1.8  2004/10/01 22:44:33  jbl
 * TimModule new methods
 *
 * Revision 1.7  2004/07/29 20:08:56  jbl
 * TimModule minor bug fixes
 *
 * Revision 1.6  2004/04/01 13:26:04  gallop
 * BJG 1/4/04
 *  Restore decimal mode after status
 *
 * Revision 1.5  2003/12/04 19:10:49  jbl
 * TimModule uses BaseException
 *
 * Revision 1.4  2003/05/20 19:26:25  jbl
 * TimModule UINT8 & UINT16
 *
 * Revision 1.3  2002/12/11 21:30:50  jbl
 * TimModule major update
 *
 */

#include <ctime>     // for clock only
#include <iostream>  // for debug only

using namespace std;

#include "TimModule.h"

#ifdef    RCDAQ
#include "TIMModule/TIMException.h"
#endif // RCDAQ

// Namespace for the common routines for SCT and PIXEL ROD software.

namespace SctPixelRod {

// ************************* Class TimException ******************************

//! Constructors. Use defaults for destructor, copy, and assignment.

TimException::TimException( std::string descriptor, int data1, int data2 ) :
             BaseException( descriptor ) {

  m_data1 = data1;
  m_data2 = data2;
  setType( TIM );

#ifdef    RCDAQ
  throw   TIMException (TIMException::TIMBusError);
#endif // RCDAQ
}

void TimException::what( std::ostream& os ) {

  os << "TimException: " << getDescriptor() << endl;
  os << "Data1: " << getData1() << endl;
  os << "Data2: " << getData2() << endl;
}

// ************************* Class TimModule *********************************

// Constructors, destructor, and assignment

// ------------------------- Constructor -------------------------------------

//! This is the only constructor to use.
/*! Objects of this class cannot be copied or assigned.
 *  This could lead to VME conflicts.
 */

TimModule::TimModule( UINT32 baseAddr, UINT32 mapSize, VmeInterface & vme ) :
	   VmeModule( baseAddr, mapSize, vme ) {

  m_serialNumber = 0xFFFFFFFF;  // Set in initialize

  if (&vme == 0) throw TimException( "TimModule has no VmeInterface", 0, 0 );

  try {
    m_vmePort = new VmePort( baseAddr, mapSize, VmeInterface::A32, vme );

    m_vmePort->setExceptionTrapping( true );
  }
  catch (bad_alloc) {
    throw TimException( "TimModule caught bad_alloc", 0, 0 );
  }
  catch (VmeException x) {
    throw TimException( "TimModule caught VmeException",
                        x.getErrorClass(), x.getErrorCode() );
  }
}

// ------------------------- Destructor --------------------------------------

/*! This deletes all its VME Ports.
 */

TimModule::~TimModule() {

  delete m_vmePort;
  m_vmePort = 0;
}

// ---------------------------------------------------------------------------

// Member methods

// ------------------------- initialize --------------------------------------

/*! This method configures the TIM into its initialized state.
 */

void TimModule::initialize() {


  regLoad( TIM_REG_BCID, TIM_BCID_OFFSET << 12 );  // set BCID offset
  regLoad( TIM_REG_RB_MASK, 0xFFFF );                    // enable all slots
  fetchTimID();                                          // read serial number

// Don't disable fixed frequency veto on TIM 3C - remove code which
// was here (JCH)

  loadFrequencyTrig( 10. ); // kHz
  loadFrequencyFECR( 0.1 ); //  Hz

  setupVME();
}

// ------------------------- TTCrx r/w -- ----------------------------------

void TimModule::ttcRxWrite(const UINT8 address, const UINT8 data){
  ttcRxAccess(true, address, data);
  UINT8 test = ttcRxAccess(false, address, data);

  int count = 0;
  while(test != data) {
    ttcRxAccess(true, address, data);
    test = ttcRxAccess(false, address, data);
    count ++;
    if(count > 100) {
      std::cout << "ttcRxWrite failed after " << count << " tries\n";
      throw TimException("ttcRxWrite failed", count, 0);
    }
  }

  if(count > 0) {
    std::cout << "ttcRxWrite took " << count << " tries to succeed\n";
  }
}

UINT8 TimModule::ttcRxRead(const UINT8 address){
  return ttcRxAccess(false, address, (UINT8)0);
}
 

UINT8 TimModule::ttcRxAccess(bool write, const UINT8 address, const UINT8 data){
  regLoad( TIM_REG_TTCRX_CTL, 0x8000);  // enable I2C interface
  int value = 1<<15;                   // Enable + Go
  if (!write) value |= 1<<13;          // enable write bit 
  value |= ( (address & 0x1F ) << 8);   // write the address (which is 5 bits)
  value |= (data & 0xff ) ;             // write the 8 bit data
  regLoad( TIM_REG_TTCRX_CTL, value);   // write to the register
  
  int timeout = 0xffff;
  int i=0;
  int busy=1;
  while (i<timeout && busy==1){
    ++i;
    value = regFetch( TIM_REG_TTCRX_CTL );
    busy  =  ( ( value & 0x8000) >> 15 ) & 1;
  }
  if (i==timeout) {
    std::cerr << "ERROR: Timeout in TimModule ttcRxAccess with arguments write:" 
	      << write << " address = 0x" << std::hex << address 
	      << " data = 0x" << data << std::dec << std::endl;
  }
  value = regFetch( TIM_REG_TTCRX_CTL ) & 0xff;
  return value;
}
// ------------------------- reset -------------------------------------------

/*! This method issues a reset to the TIM
 *  and waits for the reset bit to clear.
 *  For TIM 3C, also check the PLLSTABLE bit.
 */

void TimModule::reset() throw (TimException &) {

  regLoad( TIM_REG_COMMAND, TIM_BIT_VRESET );
  int timeLeft = regTimeout( TIM_REG_COMMAND, TIM_BIT_VRESET, 0, 100 );
  if (timeLeft < 1) throw TimException( "TimModule reset bit timeout", 0, 0 );

//For TIM 3C, should check that the PLL is stable also. Before doing so,
//set the CSBDISABLE bit.

  fetchTimID();
  if(m_serialNumber>=0x26) {    //Select TIM 3C
    loadBitSet(TIM_REG_DEBUG_CTL,TIM_BIT_DEBUGCTL_CSBDISABLE);
//
    int timeLeft = regTimeout(TIM_REG_STATUS3,TIM_BIT_STATUS3_PLLSTABLE,TIM_BIT_STATUS3_PLLSTABLE,1000);
    if (timeLeft < 1) throw TimException( "TimModule PLL stable bit timeout", 0, 0 );
  }

}

// ------------------------- status ------------------------------------------

/*! This method reports the status of the TIM.
 *  For now, it simply prints to standard output.
 */

void TimModule::status() {

  statusPrint( cout );
}

// ------------------------- fetchL1ID ---------------------------------------
/*! This method reads the last TIM L1ID value.
 */

UINT32 TimModule::fetchL1ID() {

  const UINT32 lo = regFetch( TIM_REG_L1IDL );
  const UINT32 hi = regFetch( TIM_REG_L1IDH );
  UINT32 l1id;

  if (m_firmware < 9) l1id = (lo & 0x0FFF) + ((hi & 0x0FFF) << 12);
  else                l1id = (lo & 0xFFFF) + ((hi & 0x00FF) << 16);
  return l1id;
}

// ------------------------- fetchTimID --------------------------------------

/*! This method reads the TIM ID register.
 */

UINT16 TimModule::fetchTimID() {

  const UINT16 timID = regFetch( TIM_REG_TIM_ID );

  m_serialNumber = timID & 0xFF;
  m_firmware     = timID >> 8;

  if (m_serialNumber == 0    || m_firmware == 0 ||
      m_serialNumber == 0xFF || m_firmware == 0xFF)
      throw TimException( "TimModule finds illegal TIM ID register",
                          m_serialNumber, m_firmware );

  if (m_serialNumber >= 0x20) m_firmware += 0xF00; // FPGA

  return timID;
}

// ------------------------- intTrigStart ------------------------------------

/*! This method Enables Internal Triggers with the given repeat frequency.
 */

void TimModule::intTrigStart( const double frequency ) {

  loadFrequencyTrig( frequency );
  loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_TRIG );
}

// ------------------------- intTrigStart ------------------------------------

/*! This method Enables Internal Triggers with the given repeat frequency.
 */

void TimModule::intTrigStart( const TimMaskFrequency frequency ) {

  loadByteLo( TIM_REG_FREQUENCY, frequency );
  loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_TRIG );
}

// ------------------------- intTrigStop -------------------------------------

/*! This method stops Internal Triggers.
 */

void TimModule::intTrigStop(void) { //*** void

  loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_TRIG );
}

// ------------------------- issueCommand ------------------------------------

/*! This method issues a TIM command edge-mode bit - experimental!
 */

void TimModule::issueCommand( const TimBitCommand mask ) {

  if (mask <= TIM_VSPA) loadBitClear( TIM_REG_COMMAND, mask );
  loadBitSet( TIM_REG_COMMAND, mask );
  if (mask <= TIM_VSPA) loadBitClear( TIM_REG_COMMAND, mask );
}

// ------------------------- issueVCAL ---------------------------------------

/*! This method issues a VME CAL command followed by an L1A,
 *  after the given pipeline delay.
 */

void TimModule::issueVCAL( const UINT8 pipelineDelay ) {

  loadByteLo(   TIM_REG_DELAYS, pipelineDelay );
  issueCommand( TIM_VCAL );
}

// ------------------------- loadBitSet --------------------------------------

/*! This method writes the bits set in a 16-bit bit mask into a VME register,
 *  leaving the other bits unchanged.
 */

void TimModule::loadBitSet( const TimRegister addr, const UINT16 mask ) {

  UINT16 value = vmeFetch( addr );
  vmeLoad( addr, value | mask );
}

// ------------------------- loadBitClear ------------------------------------

/*! This method clears the bits set in a 16-bit bit mask into a VME register,
 *  leaving the other bits unchanged.
 */

void TimModule::loadBitClear( const TimRegister addr, const UINT16 mask ) {

  UINT16 value = vmeFetch( addr );
  vmeLoad( addr, value & ~mask );
}

// ------------------------- loadByteHi --------------------------------------

/*! This method writes a byte into the upper half of a 16-bit VME register,
 *  leaving the other byte unchanged.
 */

void TimModule::loadByteHi( const TimRegister addr, const UINT8 byte ) {

  UINT16 value = vmeFetch( addr );
  vmeLoad( addr, (value & 0x00FF) | (byte << 8) );
}

// ------------------------- loadByteLo --------------------------------------

/*! This method writes a byte into the lower half of a 16-bit VME register,
 *  leaving the other byte unchanged.
 */

void TimModule::loadByteLo( const TimRegister addr, const UINT8 byte ) {

  UINT16 value = vmeFetch( addr );
  vmeLoad( addr, (value & 0xFF00) | byte );
}

// ------------------------- loadFrequencyFECR -------------------------------

/*! This method loads the FER/ECR repeat frequency given in Hz.
 */

void TimModule::loadFrequencyFECR( const double frequency ) {

  const int freq = int (frequency * 1000.); // mHz

  int pattern = TIM_MASK_FECR_0_100HZ; // default
  int chooser = 0;

  for (int i = 0; i < TIM_FREQ_SIZE; i++) {
    if (TIM_FECR_FREQUENCY[i][1] <= freq &&
        TIM_FECR_FREQUENCY[i][1] > chooser) {
      chooser = TIM_FECR_FREQUENCY[i][1];
      pattern = TIM_FECR_FREQUENCY[i][0];
    }
  }

  loadByteHi( TIM_REG_FREQUENCY, pattern >> 8 );
}

// ------------------------- loadFrequencyTrig -------------------------------

/*! This method loads the trigger repeat frequency given in kHz.
 */

void TimModule::loadFrequencyTrig( const double frequency ) {

  const int freq = int (frequency * 1000.); // Hz

  int pattern = TIM_MASK_TRIG_10_0KHZ; // default
  int chooser = 0;

  for (int i = 0; i < TIM_FREQ_SIZE; i++) {
    if (TIM_TRIG_FREQUENCY[i][1] <= freq &&
        TIM_TRIG_FREQUENCY[i][1] > chooser) {
      chooser = TIM_TRIG_FREQUENCY[i][1];
      pattern = TIM_TRIG_FREQUENCY[i][0];
    }
  }

  loadByteLo( TIM_REG_FREQUENCY, pattern );
}

// ------------------------- msleep ------------------------------------------

/*! This method waits until milliseconds of time elapses.
 */

void TimModule::msleep( const double milliSecs ) {

  clock_t delay = clock_t( milliSecs * CLOCKS_PER_SEC / 1000. );
  clock_t start = clock();

  while (clock() - start < delay)
      ;
}

// ------------------------- regFetch ----------------------------------------

/*! This method reads a 16-bit value from a VME register.
 */

UINT16 TimModule::regFetch( const TimRegister addr ) {

  UINT16 data = vmeFetch( addr );
  return data;
}

// ------------------------- regLoad -----------------------------------------

/*! This method writes a 16-bit value into a VME register.
 */

void TimModule::regLoad( const TimRegister addr, const UINT16 data ) {

  vmeLoad( addr, data );
}

// ------------------------- regTimeout --------------------------------------

/*! This method waits for a given register value.
 */

int TimModule::regTimeout( const TimRegister addr, const int mask1,
                           const int mask2, const int timeout ) {

  int timeLeft = timeout;

  while (timeLeft > 0 && (regFetch( addr ) & mask1) != mask2) {
    timeLeft --;
    msleep( 1.);
  }
  return timeLeft;
}

// ------------------------- seqRun ------------------------------------------

/*! This method starts the Sequencer executing.
 */

void TimModule::seqRun( const UINT16 size ) {

  regLoad( TIM_REG_SEQ_END,     size - 1 );
  regLoad( TIM_REG_SEQ_CTL, TIM_BIT_SEQ_RESET );
  regLoad( TIM_REG_SEQ_CTL, 0 );
  regLoad( TIM_REG_SEQ_CTL, TIM_BIT_SEQ_GO | TIM_BIT_SEQ_EN_ALL );
  // all outputs enabled
}

// ------------------------- seqFetch ----------------------------------------

/*! This method reads the Sequencer memory into a buffer.
 */

void TimModule::seqFetch( const UINT16 size, UINT16 buffer[] ) {

  for (int i = 0; i < size; i++) {
    buffer[i] = vmeFetch( TIM_SEQ_ADDR + i*2 );
  }
}

// ------------------------- seqLoad -----------------------------------------

/*! This method writes a buffer into the Sequencer memory.
 */

void TimModule::seqLoad( const UINT16 size, const UINT16 buffer[] ) {

  UINT16 data;
  for (int i = 0; i < size; i++) {
    data = buffer[i];
    vmeLoad( TIM_SEQ_ADDR + i*2, data );
  }
}

// ------------------------- setupTTC ----------------------------------------

/*! This method configures the TIM into its TTC-mode state.
 */

void TimModule::setupTTC() {

  // enable serial output streams and TTC signals
  regLoad( TIM_REG_RUN_ENABLE, TIM_BIT_EN_ID | TIM_BIT_EN_TYPE | 0xFF );
  regLoad( TIM_REG_COMMAND, TIM_BIT_EN_TTC );       // set Run mode
  regLoad( TIM_REG_ENABLES, 0 );                    // ensure no signals

  int timeLeft = regTimeout( TIM_REG_TTC_STATUS, TIM_BIT_TTC_READY,
                                                 TIM_BIT_TTC_READY, 1000 );
  if (timeLeft < 1) throw TimException( "TimModule TTC Ready timeout", 0, 0 );
}

// ------------------------- setupVME ----------------------------------------

/*! This method configures the TIM into its VME-mode state.
 */

void TimModule::setupVME() {

  // enable serial output streams
  regLoad( TIM_REG_RUN_ENABLE, TIM_BIT_EN_ID | TIM_BIT_EN_TYPE );
  regLoad( TIM_REG_COMMAND, 0 );                    // ensure stand-alone mode
  regLoad( TIM_REG_ENABLES, 0 );                    // ensure no signals
}

// ------------------------- statusPrint -------------------------------------

/*! This method reports the status of the TIM.
 */

void TimModule::statusPrint( ostream& os ) {

  os << "TIM status" << endl;

  os << " Serial Number: " << m_serialNumber;
  os << " Version: "       << m_firmware;
  os << endl;
  hex(os);
  os << " L1ID: "   << fetchL1ID();
  os << " BCID: "   << regFetch( TIM_REG_BCID );
  os << " status: " << regFetch( TIM_REG_STATUS );
  os << endl;
  dec(os);
}

// ------------------------- vmeFetch ----------------------------------------

/*! This method reads a 16-bit value from a VME register.
 */

UINT16 TimModule::vmeFetch( const UINT32 addr )
		  throw (VmeException &) {

  UINT16 data = 0;
  bool trying = true;
  bool error = false;

  while (trying) {
    try {
      data = m_vmePort->read16( addr );
      trying = false;
    }
    catch (VmeException & vmeFailed) {
      if (error) throw vmeFailed;
      error = true;
    }
  }
  if (error) cerr << "TimModule::vmeFetch recovers from VmeException" << endl;
  return data;
}

// ------------------------- vmeLoad -----------------------------------------

/*! This method writes a 16-bit value into a VME register.
 */

void TimModule::vmeLoad( const UINT32 addr, const UINT16 data )
		throw (VmeException & ) {

  bool trying = true;
  bool error = false;

  while (trying) {
    try {
      m_vmePort->write16( addr, data );
//debug  throw VmeException(VmeException::BUS_ERROR, 1, m_vmePort);
      trying = false;
    }
    catch (VmeException & vmeFailed) {
      if (error) throw vmeFailed;
      error = true;
    }
  }
  if (error) cerr << "TimModule::vmeLoad recovers from VmeException" << endl;
}

} // End namespace SctPixelRod

// ------------------------- Overload operator<< -----------------------------

/*! This overloaded operator lets us use cout to print the status of the TIM.
 *  It overloads std::operator<< when used with an object of class TimModule.
 */

using namespace SctPixelRod;

namespace std {

ostream& operator<<( ostream& os, TimModule& tim ) {

  tim.statusPrint( os );

  return os;
}

} // End namespace std
