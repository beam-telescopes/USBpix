// File: BocCard.h

#ifndef SCTPIXELROD_BOCCARD_H
#define SCTPIXELROD_BOCCARD_H

#include <iostream>
#include <cmath>

#include "BocAddresses.h"
#include "RodModule.h"


namespace SctPixelRod {

//! Parameters needed by BocCard.
//!
//! Identifiers for types of BOC, as defined in m_bocType.
//! These values are the upper 3 bits of the Hardware Revision register
//! which are now (from 3 March 2004) used to identify the PCB version.
//!
	const unsigned int PRE_PRODUCTION_BOC  = 0x0;
	const unsigned int PRODUCTION_REVA_BOC = 0x1;
        const unsigned int PRODUCTION_REVB_BOC = 0x2;
        const unsigned int PRODUCTION_REVC_BOC = 0x3;

//! BOC Monitor ADC parameters. There are three types of channel:
//! PIN Current, PIN Voltage and Thermistor temperature for each
//! of Rev A, Rev B and Rev C production boards.
//! Define the channel types and identify them - the number of channels
//! is defined in BocAddresses.h along with the register addresses.
//!

	const unsigned int MONITOR_CURRENT  = 1;
	const unsigned int MONITOR_VOLTAGE  = 2;
	const unsigned int MONITOR_TEMP     = 3;
#ifdef PIXEL_ROD
	const unsigned int MONITOR_CHANNEL_TYPE[BOC_MONITOR_CHANNELS] = {
		MONITOR_CURRENT,
		MONITOR_VOLTAGE,
		MONITOR_VOLTAGE,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_VOLTAGE,
		MONITOR_VOLTAGE,
		MONITOR_CURRENT,
		MONITOR_VOLTAGE,
		MONITOR_VOLTAGE,
		MONITOR_TEMP,
		MONITOR_TEMP};
#else 
	const unsigned int MONITOR_CHANNEL_TYPE[BOC_MONITOR_CHANNELS] = {
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_CURRENT,
		MONITOR_VOLTAGE,
		MONITOR_VOLTAGE,
		MONITOR_TEMP,
		MONITOR_TEMP};
#endif

//! Now the factors used in the calculations.
//! First the Rev A parameters
//! Currents
        const double iArevA = -0.04051;
        const double iBrevA = 41.52;
//! Voltages
        const double vArevA = 0.011106;
        const double vBrevA = 0.0;
//! Temperatures
        const double tArevA = 10000.0;
        const double tBrevA = 1023.0;
        const double tC0revA = 7.4717e-4;
        const double tC1revA = 2.7726e-4;
        const double tC2revA = 6.8388e-8;

//! Now Rev B parameters
//! Currents
        const double iArevB = 0.02383;
        const double iBrevB = 0.0;
//! Voltages
        const double vArevB = 0.014492;
        const double vBrevB = 0.0;
//! Temperatures
        const double tArevB = 10000.0;
        const double tBrevB = 1023.0;
        const double tC0revB = 7.4717e-4;
        const double tC1revB = 2.7726e-4;
        const double tC2revB = 6.8388e-8;

//! and Rev C parameters
//! Currents
        const double iArevC = 0.02383;
        const double iBrevC = 0.0;
//! Voltages
        const double vArevC = 0.014492;
        const double vBrevC = 0.0;
//! Temperatures
        const double tArevC = 10000.0;
        const double tBrevC = 1023.0;
        const double tC0revC = 7.4717e-4;
        const double tC1revC = 2.7726e-4;
        const double tC2revC = 6.8388e-8;


//------------------------------------------------------------------------

//! BocException: general exception class for BOC-specific errors.

/*! @class BocException
 *
 * @brief This class is thrown if an error specific to the BocCard
 * class is encountered. Typically this is if a value in an argument
 * is outside the valid range.
 * An exception is now also thrown if the BOC_BUSY bit does not clear
 * in one second - this is to avoid a hangup on the setup bus.
 * The class inherits from BaseException
 *
 * @author John Hill (hill@hep.phy.cam.ac.uk) - originator
 */

class BocException : public BaseException {

public:

  BocException( std::string descriptor);
  BocException( std::string descriptor, INT32 value1, INT32 value2);
  INT32 getNumData() {return m_numValue; };
  INT32 getData1() {return m_value1; };
  INT32 getData2() {return m_value2; };
  virtual void what(std::ostream&);

private:

// Number of data items
  INT32 m_numValue;
// 1st data value returned by exception
  INT32 m_value1;

// 2nd data value returned by exception
  INT32 m_value2;

// End of BocException declarations

};


//------------------------------------------------------------------------

//! BocCard: A class for setting-up etc. the BOC.

/*! 
 * @class BocCard
 *
 * @brief This class contains the methods to set up a BOC. The name
 * BocCard was chosen because the possibly more obvious "BocModule"
 * implies that this class is derived from VmeModule. In fact it relies on
 * RodModule for the interface to the BOC.  
 *
 * @author John Hill (hill@hep.phy.cam.ac.uk) - originator
 */

class BocCard
{
public:

// Constructor and destructor
	BocCard(RodModule & rod );                     // Constructor
	~BocCard();                                    // Destructor


// Accessor methods

	UINT32 getSerialNumber() {return m_serialNumber;};
	UINT32 getManufacturer() {return m_manufacturer;};
	UINT32 getModuleType() {return m_moduleType;};
	UINT32 getHardwareRevision() {return m_hardwareRevision;};
	UINT32 getFirmwareRevision() {return m_firmwareRevision;};
	RodModule* getRod() {return m_myrod;};

/*! read register info from BOC
 */

	void readInfo();

/*! initialize() sets the BOC into a well-defined state. Power-on should
 *  set the BOC into much the same state, but we may wish to do this to
 *  recover that state without power cycling.

 *  Activities:
 *     Reset the BOC
 *     Read the read-only registers
 *     Set clock registers
 *     Initialize the I2C busses by writing to each in turn
 *     Set 0xFF into the Rx thresholds
 *     Set 0x00 into the data delays, strobe delays and laser current DACs
 *     Reset the BPM12 chips
 *     Set the defaults values for the BPM12 chips (including the test
 *     structures).
 */
        void initialize();


/*! reset() issues a reset to all parts of the BOC.
 */
        void reset();


/* status() reports the status of the BOC.
 * This initial version just prints the information to standard output.
 */
        void status();


/*! getLaserCurrent() reads the contents of one or more Tx laser current
 * DACs. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value.
 */
        void getLaserCurrent(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getLaserCurrent(const UINT32 channel) throw (BocException&);


/*! setLaserCurrent() writes the contents of one or more Tx laser current
 * DACs. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 */
        void setLaserCurrent(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	void setLaserCurrent(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getRxThreshold() reads the contents of one or more Rx threshold
 * DACs. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value.
 */
	void getRxThreshold(const UINT32 channel, UINT32 buffer[], 
                const UINT32 numChannels) throw (BocException&);
	UINT32 getRxThreshold(const UINT32 channel) throw (BocException&);


/*! setRxThreshold() writes the contents of one or more Rx threshold
 * DACs. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 */
	void setRxThreshold(const UINT32 channel, const UINT32 buffer[], 
                const UINT32 numChannels) throw (BocException&);
        void setRxThreshold(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getRxDataDelay() reads the contents of one or more Rx data delay
 * registers. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value.
 */
        void getRxDataDelay(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getRxDataDelay(const UINT32 channel) throw (BocException&);


/*! setRxDataDelay() writes the contents of one or more Rx data delay
 * registers. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 */
        void setRxDataDelay(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
        void setRxDataDelay(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getStrobeDelay() reads the contents of one or more strobe delay
 * registers. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value.
 * Note that these registers no longer do anything that is of practical
 * use - the methods are retained for completeness and might be made
 * private at a later date.
 */
        void getStrobeDelay(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getStrobeDelay(const UINT32 channel) throw (BocException&);


/*! setStrobeDelay() writes the contents of one or more strobe delay
 * registers. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 * Note that these registers no longer do anything that is of practical
 * use - the methods are retained for completeness and might be made
 * private at a later date.
 */
        void setStrobeDelay(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
        void setStrobeDelay(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getBpmStreamInhibit() reads the contents of one or more BPM stream inhibit
 * registers. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value. "Channel" refers to the BOC channel number -
 * the BPM stream number mapping is dealt with internally.
 */
        void getBpmStreamInhibit(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getBpmStreamInhibit(const UINT32 channel) throw (BocException&);


/*! setBpmStreamInhibit() writes the contents of one or more BPM stream inhibit
 * registers. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 * "Channel" refers to the BOC channel number - the BPM stream number mapping
 * is dealt with internally.
 */
        void setBpmStreamInhibit(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&); 
        void setBpmStreamInhibit(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getBpmMarkSpace() reads the contents of one or more BPM mark/space
 * registers. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value. "Channel" refers to the BOC channel number -
 * the BPM stream number mapping is dealt with internally.
 */
        void getBpmMarkSpace(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getBpmMarkSpace(const UINT32 channel) throw (BocException&);


/*! setBpmMarkSpace() writes the contents of one or more BPM mark/space
 * registers. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 * "Channel" refers to the BOC channel number - the BPM stream number mapping
 * is dealt with internally.
 */
        void setBpmMarkSpace(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
        void setBpmMarkSpace(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getBpmCoarseDelay() reads the contents of one or more BPM coarse delay
 * registers. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value. "Channel" refers to the BOC channel number -
 * the BPM stream number mapping is dealt with internally.
 */
        void getBpmCoarseDelay(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getBpmCoarseDelay(const UINT32 channel) throw (BocException&);


/*! setBpmCoarseDelay() writes the contents of one or more BPM coarse delay
 * registers. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 * "Channel" refers to the BOC channel number - the BPM stream number mapping
 * is dealt with internally.
 */
        void setBpmCoarseDelay(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
        void setBpmCoarseDelay(const UINT32 channel, const UINT32 value)
                throw (BocException&);


/*! getBpmFineDelay() reads the contents of one or more BPM fine delay
 * registers. The method is overloaded to return either a number of consecutive
 * channels from a starting channel number in a buffer, or a single channel
 * via the method return value. "Channel" refers to the BOC channel number -
 * the BPM stream number mapping is dealt with internally.
 */
        void getBpmFineDelay(const UINT32 channel, UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
	UINT32 getBpmFineDelay(const UINT32 channel) throw (BocException&);


/*! setBpmFineDelay() writes the contents of one or more BPM fine delay
 * registers. The method is overloaded to write either a number of consecutive
 * channels from a starting channel number, or a single channel.
 * "Channel" refers to the BOC channel number - the BPM stream number mapping
 * is dealt with internally.
 */
        void setBpmFineDelay(const UINT32 channel, const UINT32 buffer[],
                const UINT32 numChannels) throw (BocException&);
        void setBpmFineDelay(const UINT32 channel, const UINT32 value) 
                throw (BocException&);


/*! getClockControl() reads the contents of the Clock Control register.
 */
        UINT32 getClockControl();


/*! setClockControl() writes the contents of the Clock Control register.
 */
        void setClockControl(const UINT32 value) throw (BocException&);


/*! getRxDataMode() reads the contents of the Data Mode register.
 */
        UINT32 getRxDataMode();


/*! setRxDataMode() writes the contents of the Data Mode register.
 */
        void setRxDataMode(const UINT32 value) throw (BocException&);


/*! getRxDacClear() reads the contents of the Rx Dac Clear register.
 */
        UINT32 getRxDacClear();


/*! clearRxDac() clears the Rx Dac register by writing 1 followed by
 *  0 to the register. A method to write a specific value to the register
 *  is not provided, as this is not a meaningful operation for the user.
 */
        void clearRxDac();


/*! getTxDacClear() reads the contents of the Tx Dac Clear register.
 */
        UINT32 getTxDacClear();


/*! clearTxDac() clears the Tx Dac register by writing 1 followed by
 *  0 to the register. A method to write a specific value to the register
 *  is not provided, as this is not a meaningful operation for the user.
 */
        void clearTxDac();


/*! getVernierFinePhase() reads the contents of the Vernier Fine Phase
 * register.
 */
	UINT32 getVernierFinePhase();


/*! setVernierFinePhase() writes the contents of the Vernier Fine Phase
 * register.
 */
	void setVernierFinePhase(const UINT32 value) throw (BocException&);


/*! getVernierClockPhase0() reads the contents of the Vernier Clock Phase
 * register 0. This is one of two clock phase registers allowing the user
 * to set a delay of up to 50 nsec.
 */
        UINT32 getVernierClockPhase0();


/*! setVernierClockPhase0() writes the contents of the Vernier Clock Phase
 * register 0. This is one of two clock phase registers allowing the user
 * to set a delay of up to 50 nsec.
 */
        void setVernierClockPhase0(const UINT32 value) throw (BocException&);


/*! getVernierClockPhase1() reads the contents of the Vernier Clock Phase
 * register 1. This is one of two clock phase registers allowing the user
 * to set a delay of up to 50 nsec.
 */
        UINT32 getVernierClockPhase1();


/*! setVernierClockPhase1() writes the contents of the Vernier Clock Phase
 * register 1. This is one of two clock phase registers allowing the user
 * to set a delay of up to 50 nsec.
 */
        void setVernierClockPhase1(const UINT32 value) throw (BocException&);


/*! getBpmClockPhase() reads the contents of the BPM Clock Phase register.
 */
	UINT32 getBpmClockPhase();


/*! setBpmClockPhase() writes the contents of the BPM Clock Phase register.
 */
	void setBpmClockPhase(const UINT32 value) throw (BocException&);


/*! getBregClockPhase() reads the contents of the BREG Clock Phase register.
 */
        UINT32 getBregClockPhase();


/*! setBregClockPhase() writes the contents of the BREG Clock Phase register.
 */
        void setBregClockPhase(const UINT32 value) throw (BocException&);


/*! getBocReset() reads the contents of the BOC Reset register.
 */
	UINT32 getBocReset();


/*! resetBoc() resets the BOC by writing 1 followed by 0 to the Reset register.
 *  A method to write a specific value to the register is not provided as
 *  this would not be a useful operation.
 */
	void resetBoc();


/*! getBpmReset() reads the contents of the BPM Reset register.
 */
        UINT32 getBpmReset();


/*! resetBpm() resets the BPM registers on the BOC by writing 1 followed by 0 
 *  to the BPM Reset register. A method to write a specific value to the
 *  register is not provided as this would not be a useful operation.
 */
        void resetBpm();


/*! getVpinReset() reads the contents of the Vpin Reset register.
 */
        UINT32 getVpinReset();


/*! resetVpin() resets the Vpin register on the BOC by writing 1 followed by 0
 *  to the Vpin Reset register. A method to write a specific value to the
 *  register is not provided as this would not be a useful operation.
 */
        void resetVpin();


/*! getBocOkReset() reads the contents of the BOC_OK Reset register.
 */
        UINT32 getBocOkReset();


/*! resetBocOk() resets the BOC_OK register on the BOC by writing 1 followed by
 *  0 to the BOC_OK Reset register. A method to write a specific value to the
 *  register is not provided as this would not be a useful operation.
 */
        void resetBocOk();


/*! getBocStatusRegister() reads the contents of the BOC Status register.
 *  This is a readonly register, hence there is no "set" method provided.
 */

	UINT32 getBocStatusRegister();		//Read-only register


/*! getMonitorAdc() gets the reading for a single channel of the
 *  monitoring ADC on the BOC. The returned value is in "natural"
 *  units: mA for current, V for voltage and degrees C for temperature.
 *  The method is now (11 March 2004) overloaded to provide the units and
 *  type of quantity monitored if desired.
 */

	double getMonitorAdc(const UINT32 channel, std::string &units, 
                             std::string &type) throw (BocException&);
	double getMonitorAdc(const UINT32 channel) throw (BocException&);


/*! getMonitorAdcUnits() returns the units in which the channel monitoring
 *  is reported by getMonitorAdc().
 */

	std::string getMonitorAdcUnits(const UINT32 channel) 
                                       throw (BocException&);


/*! getMonitorAdcType() returns the type of information monitored (PIN current,
 *  PIN voltage or temperature).
 */

        std::string getMonitorAdcType(const UINT32 channel) 
                                      throw (BocException&);


/*! resetMonitorAdc() initialises the monitoring ADC chip. This is done
 *  in the initialisation method, but can be redone any time.
 */

	void resetMonitorAdc();

/*! getInterlockStatus() returns the status of the laser interlocks.
 *  The returned value is 0 if there is an interlock, 1 if not. The
 *  relevant bits on the status register are used. The state of the
 *  local enable and remote enable can be returned in the arguments, to
 *  give extra information if required - the method is overloaded so
 *  that the user can avoid this complication if necessary.
 */

	UINT32 getInterlockStatus(UINT32 *localEnable, UINT32 *remoteEnable);
	UINT32 getInterlockStatus();

private: 


//! Read and write to single BOC registers
        UINT32 singleRead(const UINT32 bocAddress) throw (BocException&);
        void singleWrite(const UINT32 bocAddress, const UINT32 value) throw (BocException&);


//! Read and write to a block of BOC registers
        void blockRead(const UINT32 bocAddress,  UINT32 buffer[],
                const INT32 length);
        void blockWrite(const UINT32 bocAddress,  const UINT32 buffer[],
                const INT32 length);


//! Read and write to BPM streams
	UINT32 bpmRead(const UINT32 bpm, const UINT32 stream, 
                const UINT32 offset);
	void bpmWrite(const UINT32 bpm, const UINT32 stream, 
                const UINT32 offset, const UINT32 value);


//! Handle for the ROD through which this BOC is accessed.
	RodModule* m_myrod;


//! BOC serial number (read in initialize method). Set on a pair of hex switches.
	UINT32 m_serialNumber;


//! BOC manufacturer (read in initialize method). Should be 0xCB.
	UINT32 m_manufacturer;


//! BOC module type (read in initialize method).
	UINT32 m_moduleType;


//! BOC hardware revision (read in initialize method). This variable holds
//! the lower 5 bits of the register only now.
	UINT32 m_hardwareRevision;


//! BOC firmware revision (read in initialize method).
	UINT32 m_firmwareRevision;

//! BOC type. This identifies the different BOC revisions, deduced
//! from the upper 3 bits of the Hardware Revision register (modified
//! 3 March 2004).
	UINT32 m_bocType;

//! BOC setup bus mask. This is the maximum width of any data to be
//! read or written via the setup bus.
	UINT32 m_bocMask;

  }; // End of class BocCard declaration


}; // End namespace SctPixelRod

#endif  // SCTPIXELROD_BOCCARD_H
