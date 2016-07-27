// File: BocCard.cxx


#include "BocCard.h"

//! Namespace for the common routines for SCT and PIXEL ROD software.
namespace SctPixelRod {



//**************************class BocException***********************
//
// Description:
//  This class provides the exception handling for the BocCard routines.
//
// Author(s):
//  John Hill (hill@hep.phy.cam.ac.uk) - originator
//
//-------------------------------Constructor-------------------------

/*
Define two constructors - one for no data, one for two data items.
Default destructor is used.
*/


BocException::BocException(std::string descriptor) :
                           BaseException(descriptor) {
  m_numValue = 0;
  m_value1 = 0;
  m_value2 = 0;
  setType(BOC);
}

BocException::BocException(std::string descriptor,INT32 value1,INT32 value2) :
                           BaseException(descriptor) {
  m_numValue = 2;
  m_value1 = value1;
  m_value2 = value2;
  setType(BOC);
}

void BocException::what(std::ostream& os) {
  unsigned long numData;
  numData = getNumData();
  os << "BocException: " << getDescriptor() << std::endl;
  if (numData == 0) return;
  os << "Data1:" << getData1() << std::endl;
  os << "Data2:" << getData2() << std::endl;
}


//****************************class BocCard**************************
//
// Description:
//  This class provides the software interface to the BOC module.
//
// Author(s):
//  John Hill (hill@hep.phy.cam.ac.uk) - originator
//
//-------------------------------Constructor-------------------------

/*
This is the only constructor to use.
*/


BocCard::BocCard(RodModule & rod) {

  std::cout << "New BOC\n" << std::endl;

  m_myrod = &rod;
  //  m_serialNumber = 0xFFFFFFFF;  //initialize overwrites this

//   m_serialNumber = singleRead(BOC_SERIAL_NUMBER);
//   m_manufacturer = singleRead(BOC_MANUFACTURER);
//   m_moduleType = singleRead(BOC_MODULE_TYPE);
//   m_hardwareRevision = singleRead(BOC_HW_REV);
  
  // The type of BOC is now (from 3 March 2004) determined from the
  // top 3 bits of the Hardware Revision register. The bottom 5 bits
  // indicate the hardware revision for that BOC type.
  
  m_bocType = ((m_hardwareRevision)>>5) & 0x7;
  m_hardwareRevision &= 0x1F;
  //
//  m_firmwareRevision = singleRead(BOC_FW_REV);
  
  // Set the setup bus mask.
  m_bocMask = (1<<BOC_REGISTER_WIDTH) - 1;
  
}


//-------------------------------Destructor--------------------------

/*
Just remove reference to ROD. The RodModule class is not deleted, as
BocCard did not create it.
*/


BocCard::~BocCard() {
  m_myrod = 0;
}


//  Member methods

void BocCard::readInfo() {

// Get all the readonly identification information into the private
// variables

  m_serialNumber = singleRead(BOC_SERIAL_NUMBER);
  m_manufacturer = singleRead(BOC_MANUFACTURER);
  m_moduleType = singleRead(BOC_MODULE_TYPE);
  m_hardwareRevision = singleRead(BOC_HW_REV);

// The type of BOC is now (from 3 March 2004) determined from the
// top 3 bits of the Hardware Revision register. The bottom 5 bits
// indicate the hardware revision for that BOC type.

  m_bocType = ((m_hardwareRevision)>>5) & 0x7;
  m_hardwareRevision &= 0x1F;
//
  m_firmwareRevision = singleRead(BOC_FW_REV);

// Set the setup bus mask.
  m_bocMask = (1<<BOC_REGISTER_WIDTH) - 1;

}

//------------------------initialize--------------------------------

/*! Initialize (sic!) the BOC - set to a well-defined state. A lot
 * of the settings in this method will be power-on defaults, but it
 * is helpful to do this explicitly.
 */


void BocCard::initialize() {

  std::cout << "Init BOC\n" << std::endl;

// Reset the BOC

  resetBoc();

// Get all the readonly identification information into the private
// variables

  m_serialNumber = singleRead(BOC_SERIAL_NUMBER);
  m_manufacturer = singleRead(BOC_MANUFACTURER);
  m_moduleType = singleRead(BOC_MODULE_TYPE);
  m_hardwareRevision = singleRead(BOC_HW_REV);

// The type of BOC is now (from 3 March 2004) determined from the
// top 3 bits of the Hardware Revision register. The bottom 5 bits
// indicate the hardware revision for that BOC type.

  m_bocType = ((m_hardwareRevision)>>5) & 0x7;
  m_hardwareRevision &= 0x1F;
//
  m_firmwareRevision = singleRead(BOC_FW_REV);

// Set the setup bus mask.
  m_bocMask = (1<<BOC_REGISTER_WIDTH) - 1;

// Initialise the two I2C streams (Rx data delays), by writing to the first
// channel in each stream (data written is not relevant)

  setRxDataDelay(0,0);		//I2C0
  setRxDataDelay(0x40,0);	//I2C1

// Initialise the Monitor ADC

  resetMonitorAdc();

//Set the clocks

  setClockControl(0);		//Clock Control register
  setVernierClockPhase0(0);
  setVernierClockPhase1(0);	//Two vernier clocks...
  setVernierFinePhase(0);	//...and the vernier fine phase
  setBpmClockPhase(0);		//BPM Clock Phase
  setBregClockPhase(0xC);	//Breg Clock Phase (note non-zero).

//Clear the RX and TX DACs

  clearRxDac();
  clearTxDac();

//Reset the Vpins
  resetVpin();

//Reset the BOC_OK bit
  resetBocOk();

// Set all the receiver thresholds to 0xFF (ie. off!) and the data delays
// to zero.

  for(unsigned int i=0;i<BOC_RECEIVE_CHANNELS;i++) {
    setRxThreshold(i,0xFF);
    setRxDataDelay(i,0);

  }

// Set all the laser currents to 0

  for(unsigned int i=0;i<BOC_TRANSMIT_CHANNELS;i++) {
    setLaserCurrent(i,0);
  }

// Set all the strobe delays to 0. These are no longer used in normal
// operation, but the spare channels ARE connected, so for completeness...

  for(unsigned int i=0;i<BOC_STROBE_CHANNELS;i++) {
    setStrobeDelay(i,0);
  }
  
// Make sure the BPMs are reset before trying to set them.
 
  resetBpm();

// Now set the BPMs for all channels. The first 12 channels
// of a BPM are used by the "real" control fibres. Channels
// 12 and 13 of a BPM are used for test structures, and these
// also have to be set correctly.

  for(unsigned int i=0;i<BOC_TRANSMIT_CHANNELS;i++) {
    setBpmStreamInhibit(i,0);	//Enable all channels
    setBpmMarkSpace(i,0x13);		//Approved starting value
    setBpmCoarseDelay(i,0);
    setBpmFineDelay(i,0);
  }

//Use the private bpmWrite method for writing to BPM channels 12 and 13
//The code assumes that BPMs are fully utilised, with the Tx channels
//in groups of 12 in as many BPMs as are needed. This may not be true
//for Pixels?

  for(unsigned int i=0;i<(BOC_TRANSMIT_CHANNELS+11/12);i++) {
    bpmWrite(i,12,0,0x20);		//0x20 into channel 12
    bpmWrite(i,13,0,0x40);		//0x40 into channel 13
  }

//Now set the Rx Data Mode, and we should be done.

  setRxDataMode(0);

}

  
//------------------------reset-------------------------------------

/*! Reset the BOC. For now, this just issues full BOC reset, followed
 * by individual component resets, until it is clear what is needed to
 * achieve a true reset.
 */


void BocCard::reset() {

  resetBoc();
  resetBpm();
  clearRxDac();
  clearTxDac();
  resetVpin();
  resetBocOk();

}


//------------------------status------------------------------------

/*! This method reports the status of the BOC.
 *  For now, it simply prints to standard output.
 */


void BocCard::status() {

  if(m_bocType == PRE_PRODUCTION_BOC) {
    std::cout << "Pre-production BOC: ";
  }
  else if(m_bocType == PRODUCTION_REVA_BOC) {
    std::cout << "Production BOC - Revision A: ";
  }
  else if(m_bocType == PRODUCTION_REVB_BOC) {
    std::cout << "Production BOC - Revision B: ";
  }
  else if(m_bocType == PRODUCTION_REVC_BOC) {
    std::cout << "Production BOC - Revision C: ";
  }
  else {
    std::cout << "Unknown BOC type: ";
  }
  std::cout << "status" << std::endl;
//
  std::cout << std::dec << " Module Type: " << m_moduleType;
  std::cout << " Serial Number: " << m_serialNumber;
  std::cout << std::endl;
  std::cout << " Hardware Version: " << m_hardwareRevision;
  std::cout << " Firmware Version: " << m_firmwareRevision;
  std::cout << std::endl;
  std::cout << std::hex << " Manufacturer: "   << m_manufacturer;
//
// If a series BOC, least significant 2 bits are undefined.
//
  std::cout << " Status Register: " << getBocStatusRegister();
  
  std::cout << std::dec << std::endl;
}

//------------------------getLaserCurrent---------------------------

/*! Read one or more TX laser current settings from the BOC
 */


void BocCard::getLaserCurrent(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Transmit, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Transmit, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC Transmit, maximum channel >=",
       	BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_LASER_DAC + (channel<<2);

  if(numChannels <= 1) {
// Use the single read method
    buffer[0] = singleRead(address);
  }
  else {
// Use the block read method
    blockRead(address, buffer, numChannels);
  }
// Now mask off bits that are not well-defined.
//
  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] &= m_bocMask;
  }

}


UINT32 BocCard::getLaserCurrent(const UINT32 channel) throw (BocException&) {
//
  UINT32 address;

//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Transmit, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC Transmit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_LASER_DAC + (channel<<2);

// Use the single read method
  return (singleRead(address) & m_bocMask);

}


//------------------------setLaserCurrent---------------------------

/*! Write one or more TX laser current settings to the BOC
 */


void BocCard::setLaserCurrent(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Transmit, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Transmit, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC Transmit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>m_bocMask) {
      throw BocException("BOC Transmit, maximum value >",
	m_bocMask,buffer[i]);
    }
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_LASER_DAC + (channel<<2);

  if(numChannels <= 1) {
// Use the single write method
    singleWrite(address,buffer[0]);
  }
  else {
// Use the block write method
    blockWrite(address, buffer, numChannels);
  }

}


void BocCard::setLaserCurrent(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
  UINT32 address;

//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Transmit, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC Transmit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

  if(value>m_bocMask) {
    throw BocException("BOC Transmit, maximum value >",
      m_bocMask,value);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_LASER_DAC + (channel<<2);

// Use the single write method
  singleWrite(address,value);

}


//------------------------getRxThreshold---------------------------

/*! Read one or more RX threshold settings from the BOC
 */


void BocCard::getRxThreshold(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Receive, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Receive, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Receive, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel+numChannels-1);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_THRESHOLD_DAC + (channel<<2);

  if(numChannels <= 1) {
// Use the single read method
    buffer[0] = singleRead(address);
  }
  else {
// Use the block read method
    blockRead(address, buffer, numChannels);
  }

// Now mask off bits that are not well-defined.
//
  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] &= m_bocMask;
  }

}


UINT32 BocCard::getRxThreshold(const UINT32 channel) throw (BocException&) {
//
  UINT32 address;

//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Receive, start channel <",0,channel);
  }

  if(channel>=BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Receive, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_THRESHOLD_DAC + (channel<<2);

// Use the single read method

  return (singleRead(address) & m_bocMask);

}


//------------------------setRxThreshold---------------------------

/*! Write one or more RX threshold settings to the BOC
 */


void BocCard::setRxThreshold(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Receive, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Receive, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Receive, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>m_bocMask) {
      throw BocException("BOC Transmit, maximum value >",
        m_bocMask,buffer[i]);
    }
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_THRESHOLD_DAC + (channel<<2);

  if(numChannels <= 1) {
// Use the single write method
    singleWrite(address,buffer[0]);
  }
  else {
// Use the block write method
    blockWrite(address, buffer, numChannels);
  }

}


void BocCard::setRxThreshold(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Receive, start channel <",0,channel);
  }

  if(channel>=BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Receive, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel);
  }

  if(value>m_bocMask) {
    throw BocException("BOC Transmit, maximum value >",
      m_bocMask,value);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_THRESHOLD_DAC + (channel<<2);

// Use the single write method
  singleWrite(address,value);

}


//------------------------getRxDataDelay---------------------------

/*! Read one or more RX data delay settings from the BOC
 */


void BocCard::getRxDataDelay(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&){
//
  UINT32 address;
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Data Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Data Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Data Delay, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel+numChannels-1);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_DATA_DELAY + (channel<<2);

  if(numChannels <= 1) {
// Use the single read method
    buffer[0] = singleRead(address);
  }
  else {
// Use the block read method
    blockRead(address, buffer, numChannels);
  }

// Now mask off bits that are not well-defined.
//
  mask = (1<<BOC_DATA_DELAY_WIDTH)-1;
  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] &= mask;
  }

}


UINT32 BocCard::getRxDataDelay(const UINT32 channel) throw (BocException&){
//
  UINT32 address;
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Data Delay, start channel <",0,channel);
  }

  if(channel>=BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Data Delay, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_DATA_DELAY + (channel<<2);
  mask = (1<<BOC_DATA_DELAY_WIDTH)-1;

// Use the singleRead method
  return (singleRead(address) & mask);

}


//------------------------setRxDataDelay---------------------------

/*! Write one or more RX data delay settings to the BOC
 */


void BocCard::setRxDataDelay(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Data Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Data Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Data Delay, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>=(1<<BOC_DATA_DELAY_WIDTH)) {
      throw BocException("BOC Transmit, maximum value >=",
        (1<<BOC_DATA_DELAY_WIDTH),buffer[i]);
    }
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_DATA_DELAY + (channel<<2);

  if(numChannels <= 1) {
// Use the single write method
    singleWrite(address,buffer[0]);
  }
  else {
// Use the block write method
    blockWrite(address, buffer, numChannels);
  }

}


void BocCard::setRxDataDelay(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Data Delay, start channel <",0,channel);
  }

  if(channel>=BOC_RECEIVE_CHANNELS) {
    throw BocException("BOC Data Delay, maximum channel >=",
        BOC_RECEIVE_CHANNELS,channel);
  }

  if(value>=(1<<BOC_DATA_DELAY_WIDTH)) {
    throw BocException("BOC Transmit, maximum value >=",
      (1<<BOC_DATA_DELAY_WIDTH),value);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_DATA_DELAY + (channel<<2);

// Use the single write method
  singleWrite(address,value);

}


//------------------------getStrobeDelay---------------------------

/*! Read one or more strobe delay settings from the BOC. These delays
 * are probably not used under normal circumstances in BOC1, but the 
 * registers exist...
 */


void BocCard::getStrobeDelay(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Strobe Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Strobe Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_STROBE_CHANNELS) {
    throw BocException("BOC Strobe Delay, maximum channel >=",
        BOC_STROBE_CHANNELS,channel+numChannels-1);
  }

//
// Get the start address (relative to start of BOC window)
// Note that step is in units of 0x10.
//
  address = BOC_STROBE_DELAY + (channel<<4);

  if(numChannels <= 1) {
// Use the single read method
    buffer[0] = singleRead(address);
  }
  else {
// Use the block read method
    blockRead(address, buffer, numChannels);
  }

// Now mask off bits that are not well-defined.
//
  mask = (1<<BOC_STROBE_DELAY_WIDTH)-1;
  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] &= mask;
  }

}


UINT32 BocCard::getStrobeDelay(const UINT32 channel) throw (BocException&) {
//
  UINT32 address;
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Strobe Delay, start channel <",0,channel);
  }

  if(channel>=BOC_STROBE_CHANNELS) {
    throw BocException("BOC Strobe Delay, maximum channel >=",
        BOC_STROBE_CHANNELS,channel);
  }

//
// Get the start address (relative to start of BOC window)
//
  address = BOC_STROBE_DELAY + (channel<<4);
  mask = (1<<BOC_STROBE_DELAY_WIDTH)-1;

// Use the single read method
  return (singleRead(address) & mask);

}


//------------------------setStrobeDelay---------------------------

/*! Write one or more strobe delay settings to the BOC. These delays
 * are probably not used in BOC1 under normal circumstances, but the 
 * registers exist...
 */


void BocCard::setStrobeDelay(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Strobe Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC Strobe Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_STROBE_CHANNELS) {
    throw BocException("BOC Strobe Delay, maximum channel >=",
        BOC_STROBE_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>=(1<<BOC_STROBE_DELAY_WIDTH)) {
      throw BocException("BOC Transmit, maximum value >=",
        (1<<BOC_STROBE_DELAY_WIDTH),buffer[i]);
    }
  }

//
// Get the start address (relative to start of BOC window)
// Note that the step is 0x10.
//
  address = BOC_STROBE_DELAY + (channel<<4);

  if(numChannels <= 1) {
// Use the single write method
    singleWrite(address,buffer[0]);
  }
  else {
// Use the block write method
    blockWrite(address, buffer, numChannels);
  }

}


void BocCard::setStrobeDelay(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
  UINT32 address;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC Strobe Delay, start channel <",0,channel);
  }

  if(channel>=BOC_STROBE_CHANNELS) {
    throw BocException("BOC Strobe Delay, maximum channel >=",
        BOC_STROBE_CHANNELS,channel);
  }

  if(value>=(1<<BOC_STROBE_DELAY_WIDTH)) {
    throw BocException("BOC Transmit, maximum value >=",
      (1<<BOC_STROBE_DELAY_WIDTH),value);
  }
//
// Get the start address (relative to start of BOC window)
// Note that the step is 0x10.
//
  address = BOC_STROBE_DELAY + (channel<<4);

// Use the single write method
    singleWrite(address,value);

}


//BPM public methods. Private methods are provided to access channel n on
//BPM m. The following methods hide the details of BPM channel number -
//the BPM functionality is accessed via Tx stream number. 


//------------------------getBpmStreamInhibit----------------------

/*! Read one or more BPM stream inhibits from the BOC. 
 */


void BocCard::getBpmStreamInhibit(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Stream Inhibit, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Stream Inhibit, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Stream Inhibit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//

  mask = (1<<BOC_BPM_INHIBIT_WIDTH) - 1;

  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] = (bpmRead((channel+i)/12,(channel+i)%12,BOC_BPM_INHIBIT) & mask);
  }

}


UINT32 BocCard::getBpmStreamInhibit(const UINT32 channel) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Stream Inhibit, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Stream Inhibit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_INHIBIT_WIDTH) - 1;

  return (bpmRead(channel/12,channel%12,BOC_BPM_INHIBIT) & mask);

}


//------------------------setBpmStreamInhibit----------------------

/*! Write one or more BPM stream inhibits to the BOC.
 */


void BocCard::setBpmStreamInhibit(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Stream Inhibit, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Stream Inhibit, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Stream Inhibit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>=(1<<BOC_BPM_INHIBIT_WIDTH)) {
      throw BocException("BOC BPM Stream Inhibit, maximum value >=",
        (1<<BOC_BPM_INHIBIT_WIDTH),buffer[i]);
    }
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  for(unsigned int i=0;i<numChannels;i++) {
    bpmWrite((channel+i)/12,(channel+i)%12,BOC_BPM_INHIBIT,buffer[i]);
  }

}


void BocCard::setBpmStreamInhibit(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Stream Inhibit, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Stream Inhibit, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

  if(value>=(1<<BOC_BPM_INHIBIT_WIDTH)) {
    throw BocException("BOC BPM Stream Inhibit, maximum value >=",
      (1<<BOC_BPM_INHIBIT_WIDTH),value);
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  bpmWrite(channel/12,channel%12,BOC_BPM_INHIBIT,value);

}


//------------------------getBpmMarkSpace--------------------------

/*! Read one or more BPM mark/space settings from the BOC.
 */


void BocCard::getBpmMarkSpace(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Mark/Space, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Mark/Space, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Mark/Space, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_MARK_SPACE_WIDTH) - 1;

  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] = (bpmRead((channel+i)/12,(channel+i)%12,BOC_BPM_MARK_SPACE) & mask);
  }

}


UINT32 BocCard::getBpmMarkSpace(const UINT32 channel) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Mark/Space, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Mark/Space, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_MARK_SPACE_WIDTH) - 1;

  return (bpmRead(channel/12,channel%12,BOC_BPM_MARK_SPACE) & mask);

}


//------------------------setBpmMarkSpace--------------------------

/*! Write one or more BPM mark/space settings to the BOC.
 */


void BocCard::setBpmMarkSpace(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {

//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Mark/Space, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Mark/Space, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Mark/Space, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>=(1<<BOC_BPM_MARK_SPACE_WIDTH)) {
      throw BocException("BOC BPM Mark/Space, maximum value >=",
        (1<<BOC_BPM_MARK_SPACE_WIDTH),buffer[i]);
    }
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  for(unsigned int i=0;i<numChannels;i++) {
    bpmWrite((channel+i)/12,(channel+i)%12,BOC_BPM_MARK_SPACE,buffer[i]);
  }

}


void BocCard::setBpmMarkSpace(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Mark/Space, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Mark/Space, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

  if(value>=(1<<BOC_BPM_MARK_SPACE_WIDTH)) {
    throw BocException("BOC BPM Mark/Space, maximum value >=",
      (1<<BOC_BPM_MARK_SPACE_WIDTH),value);
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  bpmWrite(channel/12,channel%12,BOC_BPM_MARK_SPACE,value);

}


//------------------------getBpmCoarseDelay------------------------

/*! Read one or more BPM coarse delay settings from the BOC.
 */


void BocCard::getBpmCoarseDelay(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 mask;

// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Coarse Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Coarse Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Coarse Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_COARSE_WIDTH) - 1;

  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] = (bpmRead((channel+i)/12,(channel+i)%12,BOC_BPM_COARSE) & mask);
  }

}


UINT32 BocCard::getBpmCoarseDelay(const UINT32 channel) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Coarse Delay, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Coarse Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_COARSE_WIDTH) - 1;

  return (bpmRead(channel/12,channel%12,BOC_BPM_COARSE) & mask);

}


//------------------------setBpmCoarseDelay----------------------

/*! Write one or more BPM coarse delay settings to the BOC.
 */


void BocCard::setBpmCoarseDelay(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Coarse Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Coarse Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Coarse Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>=(1<<BOC_BPM_COARSE_WIDTH)) {
      throw BocException("BOC BPM Coarse Delay, maximum value >=",
        (1<<BOC_BPM_COARSE_WIDTH),buffer[i]);
    }
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  for(unsigned int i=0;i<numChannels;i++) {
    bpmWrite((channel+i)/12,(channel+i)%12,BOC_BPM_COARSE,buffer[i]);
  }

}


void BocCard::setBpmCoarseDelay(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Coarse Delay, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Coarse Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

  if(value>=(1<<BOC_BPM_COARSE_WIDTH)) {
    throw BocException("BOC BPM Coarse Delay, maximum value >=",
      (1<<BOC_BPM_COARSE_WIDTH),value);
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  bpmWrite(channel/12,channel%12,BOC_BPM_COARSE,value);

}


//------------------------getBpmFineDelay--------------------------

/*! Read one or more BPM fine delay settings from the BOC.
 */


void BocCard::getBpmFineDelay(const UINT32 channel, UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Fine Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Fine Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Fine Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_FINE_WIDTH) - 1;
 
  for(unsigned int i=0;i<numChannels;i++) {
    buffer[i] = (bpmRead((channel+i)/12,(channel+i)%12,BOC_BPM_FINE) & mask);
  }

}


UINT32 BocCard::getBpmFineDelay(const UINT32 channel) throw (BocException&) {
//
  UINT32 mask;
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Fine Delay, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Fine Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

//
// Use bpmRead to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  mask = (1<<BOC_BPM_FINE_WIDTH) - 1;

  return (bpmRead(channel/12,channel%12,BOC_BPM_FINE) & mask);

}


//------------------------setBpmFineDelay--------------------------

/*! Write one or more BPM fine delay settings to the BOC.
 */


void BocCard::setBpmFineDelay(const UINT32 channel, const UINT32 buffer[],
        const UINT32 numChannels) throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Fine Delay, start channel <",0,channel);
  }

  if(numChannels<0) {
    throw BocException("BOC BPM Fine Delay, number of channels <",0,numChannels);
  }

  if((channel+numChannels)>BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Fine Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel+numChannels-1);
  }

  for(unsigned int i=0;i<numChannels;i++) {
    if(buffer[i]>=(1<<BOC_BPM_FINE_WIDTH)) {
      throw BocException("BOC BPM Fine Delay, maximum value >=",
        (1<<BOC_BPM_FINE_WIDTH),buffer[i]);
    }
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  for(unsigned int i=0;i<numChannels;i++) {
    bpmWrite((channel+i)/12,(channel+i)%12,BOC_BPM_FINE,buffer[i]);
  }

}


void BocCard::setBpmFineDelay(const UINT32 channel, const UINT32 value) 
        throw (BocException&) {
//
// Check for invalid values in arguments
//
  if(channel<0) {
    throw BocException("BOC BPM Fine Delay, start channel <",0,channel);
  }

  if(channel>=BOC_TRANSMIT_CHANNELS) {
    throw BocException("BOC BPM Fine Delay, maximum channel >=",
        BOC_TRANSMIT_CHANNELS,channel);
  }

  if(value>=(1<<BOC_BPM_FINE_WIDTH)) {
    throw BocException("BOC BPM Fine Delay, maximum value >=",
      (1<<BOC_BPM_FINE_WIDTH),value);
  }

//
// Use bpmWrite to access the registers. Streams 0-11 on the BPM are used
// for "real" channels.
//
  bpmWrite(channel/12,channel%12,BOC_BPM_FINE,value);

}


// Methods to access individual registers. Whether it is the best solution
// to have a series of trivial methods is open to debate...


//------------------------getClockControl------------------------

/*! Read the Clock Control register
 */


UINT32 BocCard::getClockControl() {
//
  UINT32 mask;
  UINT32 width;

// Mask off undefined bits. The width of this register is different for
// pre-production or series BOCs.
//
  if(m_bocType == PRE_PRODUCTION_BOC) {
        width = BOC_CLK_CONTROL_WIDTH - 1;
  }
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
        width = BOC_CLK_CONTROL_WIDTH;
  }
// Unknown BOC, set the mask to return maximum number of bits.
  else {
        width = BOC_REGISTER_WIDTH;
  }
  mask = (1<<width) - 1;
  
  return (singleRead(BOC_CLK_CONTROL) & mask);

}


//------------------------setClockControl------------------------

/*! Write to the Clock Control register
 */


void BocCard::setClockControl(const UINT32 value) throw (BocException&) {
//
UINT32 limit;
UINT32 width;
//
// Check for invalid values in the argument - the PHOS4_FIX bit is only
// available for series BOCs, so the maximum allowed value of the argument
// varies with BOC type.
//
  if(m_bocType == PRE_PRODUCTION_BOC) { 
        width = BOC_CLK_CONTROL_WIDTH - 1;
  }
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
        width = BOC_CLK_CONTROL_WIDTH;
  }
// Unknown BOC, set the limit to the maximum.
  else {
	width = BOC_REGISTER_WIDTH;
  }
  limit = 1<<width;

//
  if(value>=limit) {
        throw BocException("BOC Clock Control >=",limit,value);
  }
//
  singleWrite(BOC_CLK_CONTROL,value);

}


//------------------------getRxDataMode--------------------------

/*! Read the Rx Data Mode register
 */


UINT32 BocCard::getRxDataMode() {
//
  UINT32 mask;
// Mask off undefined bits
  mask = (1<<BOC_RX_DATA_MODE_WIDTH) - 1;

  return (singleRead(BOC_RX_DATA_MODE) & mask);

}


//------------------------setRxDataMode--------------------------

/*! Write to the Rx Data Mode register
 */


void BocCard::setRxDataMode(const UINT32 value) throw (BocException&) {
//
//
//
// Check for invalid values in the argument - Rx Data Mode can have values
// up to BOC_RX_DATA_MODE_WIDTH bits wide.
//
  if(value>=(1<<BOC_RX_DATA_MODE_WIDTH)) {
        throw BocException("BOC Rx Data Mode >=",
          1<<BOC_RX_DATA_MODE_WIDTH,value);
  }
//
  singleWrite(BOC_RX_DATA_MODE,value);

}


//------------------------getRxDacClear--------------------------

/*! Read the Rx DAC Clear register. For series BOCs, return the
 * value of the BOC_RXDAC_CLEAR_BIT of BOC_RESET.
 */


UINT32 BocCard::getRxDacClear() {
//

// Pre-production BOC
  if(m_bocType == PRE_PRODUCTION_BOC) {
    return (singleRead(BOC_RXDAC_CLEAR) & 0x1);
  }
// Series BOCs
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    return ((singleRead(BOC_RESET)>>BOC_RXDAC_CLEAR_BIT)&0x1);
  }
  else {
    return 0;
  }

}


//------------------------clearRxDac-----------------------------

/*! Clear the Rx DAC. This involves writing 1 followed by 0 to
 *  the RXDAC register. A method to do a single write does not seem
 *  to be necessary, and so is not provided.
 *  For pre-production BOCs, bit 0 of the BOC_RXDAC_CLEAR register
 *  is used.
 *  For series BOCs, bit BOC_RXDAC_CLEAR_BIT of BOC_RESET is used. 
 */


void BocCard::clearRxDac() {
//

// Pre-production BOC
  if(m_bocType == PRE_PRODUCTION_BOC) {
    singleWrite(BOC_RXDAC_CLEAR,1);
    singleWrite(BOC_RXDAC_CLEAR,0);
  }
// Series BOCs
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    singleWrite(BOC_RESET,1<<BOC_RXDAC_CLEAR_BIT);
    singleWrite(BOC_RESET,0);
  }
// Any other value - don't do anything.
}


//------------------------getTxDacClear--------------------------

/*! Read the Tx DAC Clear register. For series BOCs, return the
 * value of the BOC_TXDAC_CLEAR_BIT of BOC_RESET.
 */


UINT32 BocCard::getTxDacClear() {
//

// Pre-production BOC
  if(m_bocType == PRE_PRODUCTION_BOC) {
    return (singleRead(BOC_TXDAC_CLEAR) & 0x1);
  }
// Series BOCs
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    return ((singleRead(BOC_RESET)>>BOC_TXDAC_CLEAR_BIT)&0x1);
  }
  else {
    return 0;
  }

}


//------------------------clearTxDac-----------------------------

/*! Clear the Tx DAC. This involves writing 1 followed by 0 to
 *  the TXDAC register. A method to do a single write does not seem
 *  to be necessary, and so is not provided.
 *  For pre-production BOCs, bit 0 of the BOC_TXDAC_CLEAR register
 *  is used.
 *  For series BOCs, bit BOC_TXDAC_CLEAR_BIT of BOC_RESET is used.
 */


void BocCard::clearTxDac() {
//

// Pre-production BOC
  if(m_bocType == PRE_PRODUCTION_BOC) {
    singleWrite(BOC_TXDAC_CLEAR,1);
    singleWrite(BOC_TXDAC_CLEAR,0);
  }
// Series BOCs
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    singleWrite(BOC_RESET,1<<BOC_TXDAC_CLEAR_BIT);
    singleWrite(BOC_RESET,0);
  }
// Any other value - don't do anything.
}


//------------------------getVernierFinePhase--------------------

/*! Read the Vernier Fine Phase register
 */


UINT32 BocCard::getVernierFinePhase() {
//

  return (singleRead(BOC_VERNIER_FINE_PHASE) & m_bocMask);

}


//------------------------setVernierFinePhase--------------------

/*! Write to the VernierFinePhase register
 */


void BocCard::setVernierFinePhase(const UINT32 value) throw (BocException&){
//
// Check that the argument is not too large.
//
  if(value>m_bocMask) {
    throw BocException("BOC Vernier Fine Phase >",
          m_bocMask,value);
  }

  singleWrite(BOC_VERNIER_FINE_PHASE,value);

}


//------------------------getVernierClockPhase0-------------------

/*! Read the Vernier Clock Phase register 0. There are two registers
 * (0 and 1) which act in series to give a clock range of 50nsec in
 * 1 nsec steps.  
 */


UINT32 BocCard::getVernierClockPhase0() {
//
  UINT32 mask;
//
  mask = (1<<BOC_VERNIER_CLK0_WIDTH) - 1;

  return (singleRead(BOC_VERNIER_CLK0_PHASE) & mask);

}


//------------------------setVernierClockPhase0-------------------

/*! Write to the Vernier Clock Phase register 0. There are two registers
 * (0 and 1) which act in series to give a clock range of 50nsec in
 * 1 nsec steps.
 */


void BocCard::setVernierClockPhase0(const UINT32 value) throw (BocException&){
//
// Check that the argument is not too large.
//
  if(value>=(1<<BOC_VERNIER_CLK0_WIDTH)) {
    throw BocException("BOC Vernier Clock Phase0 >=",
          (1<<BOC_VERNIER_CLK0_WIDTH),value);
  }

  singleWrite(BOC_VERNIER_CLK0_PHASE,value);

}


//------------------------getVernierClockPhase1-------------------

/*! Read the Vernier Clock Phase register 1. There are two registers
 * (0 and 1) which act in series to give a clock range of 50nsec in
 * 1 nsec steps.
 */


UINT32 BocCard::getVernierClockPhase1() {
//
  UINT32 mask;
//
  mask = (1<<BOC_VERNIER_CLK1_WIDTH) - 1;

  return singleRead(BOC_VERNIER_CLK1_PHASE);

}


//------------------------setVernierClockPhase1-------------------

/*! Write to the Vernier Clock Phase register 1. There are two registers
 * (0 and 1) which act in series to give a clock range of 50nsec in
 * 1 nsec steps.
 */


void BocCard::setVernierClockPhase1(const UINT32 value) throw (BocException&) {
//
// Check that the argument is not too large.
//
  if(value>=(1<<BOC_VERNIER_CLK1_WIDTH)) {
    throw BocException("BOC Vernier Clock Phase1 >=",
          (1<<BOC_VERNIER_CLK1_WIDTH),value);
  }

  singleWrite(BOC_VERNIER_CLK1_PHASE,value);

}


//------------------------getBpmClockPhase-------------------

/*! Read the BPM Clock Phase register.
 */


UINT32 BocCard::getBpmClockPhase() {
//

  return (singleRead(BOC_BPM_CLK_PHASE) & m_bocMask);

}


//------------------------setBpmClockPhase-------------------

/*! Write to the BPM Clock Phase register.
 */


void BocCard::setBpmClockPhase(const UINT32 value) throw (BocException&) {
//
// Check that the argument is not too large.
//
  if(value>m_bocMask) {
    throw BocException("BOC BPM Clock Phase >",
          m_bocMask,value);
  }

  singleWrite(BOC_BPM_CLK_PHASE,value);

}


//------------------------getBregClockPhase-------------------

/*! Read the Breg Clock Phase register.
 */


UINT32 BocCard::getBregClockPhase() {
//

  return (singleRead(BOC_BREG_CLK_PHASE) & m_bocMask);

}


//------------------------setBregClockPhase-------------------

/*! Write to the Breg Clock Phase register.
 */


void BocCard::setBregClockPhase(const UINT32 value) throw (BocException&) {
//
// Check that the argument is not too large.
//
  if(value>m_bocMask) {
    throw BocException("BOC BPM Clock Phase >",
          m_bocMask,value);
  }

  singleWrite(BOC_BREG_CLK_PHASE,value);

}


//------------------------getBocReset-------------------------

/*! Read the BOC Reset register. For series BOCs, this register
 * merges the various registers that were separate on pre-production
 * BOCs.
 */


UINT32 BocCard::getBocReset() {
//
// Determine the type of BOC.
//
  if(m_bocType == PRE_PRODUCTION_BOC) {
// Pre-production BOCs
    return (singleRead(BOC_RESET) & 0x1);
  }
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs - resets are top 4 bits.
    return (singleRead(BOC_RESET) & 0xF0);
  }
  else {
    return 0;
  }

}


//------------------------resetBoc-----------------------------

/*! Reset the BOC by writing 0xFF followed by 0 to the BOC Reset 
 * register. A method to write a specific value to this register
 * is not provided, as it doesn't seem to be necessary. However,
 * for series BOCs, the resetting of BPM, Rx, Tx are bits on this
 * register, and so there are methods to reset these individually.
 */


void BocCard::resetBoc() {
//

  singleWrite(BOC_RESET,0xFF);
  singleWrite(BOC_RESET,0);

}


//------------------------getBpmReset-------------------------

/*! Read the BPM Reset register. For series BOCs, this is the
 *  BOC_BPM_RESET_BIT on the BOC_RESET register.
 */


UINT32 BocCard::getBpmReset() {
//

  if(m_bocType == PRE_PRODUCTION_BOC) {
// Pre-production BOCs
    return (singleRead(BOC_BPM_RESET) & 0x1);
  }
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs
    return ((singleRead(BOC_RESET)>>BOC_BPM_RESET_BIT)&0x1);
  }
  else {
    return 0;
  }

}


//------------------------resetBpm-----------------------------

/*! Reset the BPMs by writing 1 followed by 0 to the BPM Reset
 * register. A method to write a specific value to this register
 * is not provided, as it doesn't seem to be necessary.
 * For pre-production BOCs, this is a separate register. For
 * series BOCs, this is bit BOC_BPM_RESET_BIT on BOC_RESET.
 */


void BocCard::resetBpm() {
//

  if(m_bocType == PRE_PRODUCTION_BOC) {
// Pre-production BOCs
    singleWrite(BOC_BPM_RESET,1);
    singleWrite(BOC_BPM_RESET,0);
  }
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs
    singleWrite(BOC_RESET,1<<BOC_BPM_RESET_BIT);
    singleWrite(BOC_RESET,0);
  }
// For other types, do nothing.
}


//------------------------getVpinReset-------------------------

/*! Read the Vpin overcurrent trip Reset register. For series BOCs, this is
 *  the BOC_VPIN_RESET_BIT on the BOC_RESET register. For pre-production
 *  BOCs, it doesn't exist!
 */


UINT32 BocCard::getVpinReset() {
//

  if((m_bocType == PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs
    return ((singleRead(BOC_RESET)>>BOC_VPIN_RESET_BIT)&0x1);
  }
  else {
    return 0;
  }

}


//------------------------resetVpin----------------------------

/*! Reset the Vpin overcurrent trip by writing 1 followed by 0 to the Vpin 
 * reset bit on BOC_RESET register. This function is only available
 * on series BOCs. It is necessary to have the "1" asserted for a length
 * of time, so sleep between the writes.
 */


void BocCard::resetVpin() {
//

  if((m_bocType == PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs
    singleWrite(BOC_RESET,1<<BOC_VPIN_RESET_BIT);
// Sleep long enough for the reset to occur. Use the RodModule sleep
// as it is convenient.
    m_myrod->sleep(1);
//
    singleWrite(BOC_RESET,0);
  }
// For other types, do nothing.
}


//------------------------getBocOkReset-------------------------

/*! Read the BOC_OK Reset register. For series BOCs, this is
 *  the BOC_OK_RESET_BIT on the BOC_RESET register. For pre-production
 *  BOCs, it doesn't exist!
 */


UINT32 BocCard::getBocOkReset() {
//

  if((m_bocType == PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs
    return ((singleRead(BOC_RESET)>>BOC_OK_RESET_BIT)&0x1);
  }
  else {
    return 0;
  }

}


//------------------------resetBocOk----------------------------

/*! Reset the BOC_OK bit by writing 1 followed by 0 to the BOC_OK
 * reset bit on BOC_RESET register. This function is only available
 * on series BOCs. 
 */


void BocCard::resetBocOk() {
//

  if((m_bocType == PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
//Series BOCs
    singleWrite(BOC_RESET,1<<BOC_OK_RESET_BIT);
//
    singleWrite(BOC_RESET,0);
  }
// For other types, do nothing.
}


//------------------------getBocStatusRegister----------------

/*! Read the BOC Status Register (readonly).
 */


UINT32 BocCard::getBocStatusRegister() {
//
  UINT32 mask;
//
// If this is a series BOC, the first bit is ill-defined
// and hence is masked off.
//
  if((m_bocType == PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    mask = m_bocMask & ~0x1;
  }
  else {
    mask = m_bocMask;
  }
  return (singleRead(BOC_STATUS) & mask);
}


//------------------------getMonitorAdc-------------------------

/*! Return the value from a single monitoring ADC channel in natural units.
 * Only relevant to series BOCs.
 * For hardware revision = 1 in all series BOCs, have a 12-bit rather than
 * 10-bit ADC. Scale is the same, so the sensitivity is x4.
 */

double BocCard::getMonitorAdc(const UINT32 channel, std::string &units,
                              std::string &type) throw (BocException&) {
//
//
// Check if channel number valid.
//
  if (channel > BOC_MONITOR_CHANNELS) {
     throw BocException("BOC Monitor Adc, channel >",
                        BOC_MONITOR_CHANNELS, channel);
  }
  units = getMonitorAdcUnits(channel);
  type = getMonitorAdcType(channel);
//
// Use the alternative method to do the work.
//
  return getMonitorAdc(channel);
}

double BocCard::getMonitorAdc(const UINT32 channel) throw (BocException&) {
//
  UINT32 lsbMask;
  UINT32 msbMask;
  double adcValue;
  double logRTherm;
//
// Check if channel number valid.
//
  if (channel > BOC_MONITOR_CHANNELS) {
     throw BocException("BOC Monitor Adc, channel >",
                        BOC_MONITOR_CHANNELS, channel);
  }
//
//Only do this with production BOC.
//
  if((m_bocType == PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {

    singleWrite(BOC_ADC_CONFIG, channel);	//set channel to convert
    singleWrite(BOC_ADC_CONVERT, 0);		//convert - data irrelevant
//
//10-bit or 12-bit ADC is in two registers
//
    lsbMask = m_bocMask;
    msbMask = (1<<BOC_ADC_MSB_WIDTH[m_hardwareRevision]) - 1;

    adcValue = (double) ((singleRead(BOC_ADC_LSB)&lsbMask) 
               + ((singleRead(BOC_ADC_MSB)&msbMask)<<BOC_REGISTER_WIDTH));
//Rescale for 12-bit ADC.
    if(m_hardwareRevision == 1) adcValue /= 4.0;
//
// Channel types are defined in the array MONITOR_CHANNEL_TYPE[].
//
//Rev A, Rev B and Rev C have different parameters.
//
// Currents in mA
//
    if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_CURRENT) {
      if(m_bocType == PRODUCTION_REVA_BOC) {
        return(iArevA*adcValue+iBrevA);
      }
      else if(m_bocType == PRODUCTION_REVB_BOC) {
	return(iArevB*adcValue+iBrevB);
      }
      else if(m_bocType == PRODUCTION_REVC_BOC) {
        return(iArevC*adcValue+iBrevC);
      }
      else {
        return -1.0;
      }
    }
//Voltages in Volts
//
    else if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_VOLTAGE) {
      if(m_bocType == PRODUCTION_REVA_BOC) {
        return(vArevA*adcValue+vBrevA);
      }
      else if(m_bocType == PRODUCTION_REVB_BOC) {
        return(vArevB*adcValue+vBrevB);
      }
      else if(m_bocType == PRODUCTION_REVC_BOC) {
        return(vArevC*adcValue+vBrevC);
      }
      else {
        return -1.0;
      }
    }
//Temperatures in degrees C
//
    else if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_TEMP) {
//
// Temperature calculation is more complicated.
//
// Get the log(resistance) first, then plug into the formula.
//
      if(m_bocType == PRODUCTION_REVA_BOC) {
	logRTherm = log((tArevA*(tBrevA-adcValue))/adcValue);
        return((1.0/(tC0revA+tC1revA*logRTherm + tC2revA*pow(logRTherm,3)))-273.15);
      }
      else if(m_bocType == PRODUCTION_REVB_BOC) {
        logRTherm = log((tArevB*(tBrevB-adcValue))/adcValue);
        return((1.0/(tC0revB+tC1revB*logRTherm + tC2revB*pow(logRTherm,3)))-273.15);
      }
      else if(m_bocType == PRODUCTION_REVC_BOC) {
        logRTherm = log((tArevC*(tBrevC-adcValue))/adcValue);
        return((1.0/(tC0revC+tC1revC*logRTherm + tC2revC*pow(logRTherm,3)))-273.15);
      }
      else {
        return -1.0;
      }
    }
    else {
      return -1.0;
    }
  }
  return -1.0;
}


//------------------------getMonitorAdcType-------------------

/*! Get the type of channel we are monitoring
 */

std::string BocCard::getMonitorAdcType(const UINT32 channel) throw (BocException&) {
//
// Check if channel number valid.
//
  if (channel > BOC_MONITOR_CHANNELS) {
     throw BocException("BOC Monitor Adc Type, channel >",
                        BOC_MONITOR_CHANNELS, channel);
  }
//
// Return the type in a string
//
  if (MONITOR_CHANNEL_TYPE[channel] == MONITOR_CURRENT) {
     return "PIN Current";
  }
  else if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_VOLTAGE) {
     return "PIN Voltage";
  }
  else if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_TEMP) {
     return "Temperature";
  }
  else {
     return "";
  }
//
}


//------------------------getMonitorAdcUnits------------------

/*! Get the units for the channel we are monitoring
 */

std::string BocCard::getMonitorAdcUnits(const UINT32 channel) throw (BocException&) {
//
// Check if channel number valid.
//
  if (channel > BOC_MONITOR_CHANNELS) {
     throw BocException("BOC Monitor Adc Units, channel >",
                        BOC_MONITOR_CHANNELS, channel);
  }
//
// Return the units in a string
//
  if (MONITOR_CHANNEL_TYPE[channel] == MONITOR_CURRENT) {
     return "mA";
  }
  else if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_VOLTAGE) {
     return "Volts";
  }
  else if(MONITOR_CHANNEL_TYPE[channel] == MONITOR_TEMP) {
     return "Celcius";
  }
  else {
     return "";
  }
}


//------------------------resetMonitorAdc---------------------

/*! Reset the monitoring ADC chip.
 */


void BocCard::resetMonitorAdc() {
//
// Only do this if production BOC
//
  if((m_bocType==PRODUCTION_REVA_BOC)||
    (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    singleWrite(BOC_ADC_SETUP,0);		//data is irrelevant
  }
}


//------------------------getInterlockStatus----------------

/*! Read the BOC Status Register and extract the status of the laser
 *  interlocks. Return value = 1 for all interlocks OK, 0 if there
 *  is an interlock of some sort. The two arguments return the local
 *  and remote interlock enable statuses.
 *  Note that for pre-production boards, SW4 is the physical switch,
 *  which may be removed and replaced by a short for sites that don't
 *  need its protection. The switch is checked, as it is part of
 *  the interlock mechanism on these boards, but its status is not
 *  returned by this method for reasons of forward compatibility.
 */


UINT32 BocCard::getInterlockStatus(UINT32 *localEnable, UINT32 *remoteEnable) {
//
//
// For pre-production BOC, check bits 2, 6 and 7
//
  if(m_bocType==PRE_PRODUCTION_BOC) {
    *localEnable = (getBocStatusRegister()&(1<<BOC_PRE_PRODUCTION_LOCLASEN))!=0;
    *remoteEnable = (getBocStatusRegister()&(1<<BOC_PRE_PRODUCTION_REMLASEN))!=0;

    return(((getBocStatusRegister()&(1<<BOC_PRE_PRODUCTION_SW4))!=0)&&
          (*localEnable)&&(*remoteEnable));
  }
//
// If this is a series BOC, the interlock status bits are 6 and 7.
//
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType==PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    *localEnable = (getBocStatusRegister()&(1<<BOC_PRODUCTION_LOCLASEN))!=0;
    *remoteEnable = (getBocStatusRegister()&(1<<BOC_PRODUCTION_REMLASEN))!=0;

    return ((*localEnable)&&(*remoteEnable));
  }
//
// Unknown BOC, return 0
//
  else {
    *localEnable = 0;
    *remoteEnable = 0;
    return 0;
  }
}

UINT32 BocCard::getInterlockStatus() {
//
  UINT32 localEnable;
  UINT32 remoteEnable;
//
// For pre-production BOC, check bits 2, 6 and 7
//
  if(m_bocType == PRE_PRODUCTION_BOC) {
    localEnable = getBocStatusRegister()&(1<<BOC_PRE_PRODUCTION_LOCLASEN);
    remoteEnable = getBocStatusRegister()&(1<<BOC_PRE_PRODUCTION_REMLASEN);

    return(((getBocStatusRegister()&(1<<BOC_PRE_PRODUCTION_SW4))!=0)&&
          (localEnable!=0)&&(remoteEnable!=0));
  }
//
// If this is a series BOC, the interlock status bits are 6 and 7.
//
  else if((m_bocType == PRODUCTION_REVA_BOC)||
       (m_bocType == PRODUCTION_REVB_BOC)||(m_bocType == PRODUCTION_REVC_BOC)) {
    localEnable = getBocStatusRegister()&(1<<BOC_PRODUCTION_LOCLASEN);
    remoteEnable = getBocStatusRegister()&(1<<BOC_PRODUCTION_REMLASEN);

    return ((localEnable!=0)&&(remoteEnable!=0));
  }
//
// Unknown BOC, return 0
//
  else {
    return 0;
  }
}



//Private methods follow

//Read/write access methods. Block read and write methods are provided,
// though in fact they use the single read and write methods. I have made
// these methods private, as the user shouldn't need to see them.
//


//------------------------singleRead-----------------------------

/*! This method reads a single 32-bit word from a BOC address. An exception
 *  will be thrown if the BOC_BUSY_0 bit does not clear within a second.
 */


UINT32 BocCard::singleRead(const UINT32 address) throw (BocException&) {
//
  UINT32 bocAddress, bocAddressBase, ce0Base, rrifStatus1Address;
  UINT32 value;
  //clock_t start;
  ce0Base = 0x00400000;
  bocAddressBase = ce0Base + 0x8000;
  bocAddress = address + bocAddressBase;
  value = m_myrod->mdspSingleRead(bocAddress);
//
//Check if the BOC is busy and wait until it isn't (with a 1 second
//timeout).
//
  //start = clock();
  int count = 0;
  rrifStatus1Address = ce0Base + 0x4420;
  while(m_myrod->mdspSingleRead(rrifStatus1Address)&BOC_BUSY_0) {
    if (count++ > 1000) {
      //if(clock() - start > (clock_t)CLOCKS_PER_SEC) {
      throw BocException("single read - BOC_BUSY_0 not cleared after 1000 attempts");
    }
  }

//
  return value;
}


//------------------------singleWrite-----------------------------

/*! This method writes a single 32-bit word to a BOC address. An exception
 *  will be thrown if the BOC_BUSY_0 bit does not clear within 1 second.
 */


void BocCard::singleWrite(const UINT32 address, const UINT32 value) throw (BocException&) {
//
  UINT32 bocAddress, bocAddressBase, ce0Base, rrifStatus1Address;
  //clock_t start;
  ce0Base = 0x00400000;
  bocAddressBase = ce0Base + 0x8000;
  bocAddress = address + bocAddressBase;
  m_myrod->mdspSingleWrite(bocAddress, value);
//
//Check if the BOC is busy and wait until it isn't - there is a timeout of
//1 second for this happening.
//
  //start = clock();
  int count = 0;
  rrifStatus1Address = ce0Base + 0x4420;
  while(m_myrod->mdspSingleRead(rrifStatus1Address)&BOC_BUSY_0) {
    if (count++ > 1000) {
    //if((clock() - start) > 5*((clock_t)CLOCKS_PER_SEC)) {
      throw BocException("single write, BOC_BUSY_0 not cleared after 1000 attempts");
    }
  }

//

}


//------------------------blockRead-----------------------------

/*! This method reads a block of 32-bit words from a BOC. Because
 *  the setup bus is slow, we have to wait after each read until
 *  the BOC_BUSY_0 bit is cleared. Hence we use the singleRead
 *  method in a loop. 
 */


void BocCard::blockRead(const UINT32 address, UINT32 buffer[],
         const INT32 wordCount) {
//
//
  for(int i=0;i<wordCount;i++) {
    buffer[i] = singleRead(address+4*i);
  }
//

}


//------------------------blockWrite-----------------------------

/*! This method writes a block of 32-bit words to a BOC. Because
 *  the setup bus is slow, we have to wait after each read until
 *  the BOC_BUSY_0 bit is cleared. Hence we use the singleWrite
 *  method in a loop. 
 */

void BocCard::blockWrite(const UINT32 address, const UINT32 buffer[],
         const INT32 wordCount) {
//
//
  for(int i=0;i<wordCount;i++) {
    singleWrite(address+4*i,buffer[i]);
  }
//

}


//Read and write routines for BPM. These methods access a BPM via its
//internal channel numbers, and shouldn't be needed by users. They are
//useful mainly for access to the channels not used for "normal" BPM
//operations (channels 12-15 have special uses and are not mapped to
//by the Tx streams).


//------------------------bpmRead---------------------------------

/*! This method reads from a BPM location, identified by BPM #, stream #
 *  and offset.
 */


UINT32 BocCard::bpmRead(const UINT32 bpm, const UINT32 stream, 
        const UINT32 offset) {
//
  UINT32 address;
  UINT32 value;
//
// Get the address (relative to start of BOC window)
//
  address = BOC_BPM_BASE + (bpm<<8) + (stream<<4) + offset;

  value = singleRead(address);

  return value;

}


//------------------------bpmWrite--------------------------------

/*! This method writes to a BPM location, identified by BPM #, stream #
 *  and offset.
 */


void BocCard::bpmWrite(const UINT32 bpm, const UINT32 stream,
        const UINT32 offset, const UINT32 value) {
//
  UINT32 address;
//
// Get the address (relative to start of BOC window)
//
  address = BOC_BPM_BASE + (bpm<<8) + (stream<<4) + offset;

  singleWrite(address, value);

}


} // End namespace SctPixelRod
