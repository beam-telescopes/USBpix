//File: BocAddresses.h

#ifndef SCTPIXELROD_BOCADDRESSES_H
#define SCTPIXELROD_BOCADDRESSES_H

/*!
 *  BocAddresses.h is a wrapper around the testBench C code header file
 * memoryPartitions.h. This wrapper adapts things to appear more like C++ and to
 * declare things const so the compiler can catch attempts to change them in the
 * code. Only BOC addresses appear in this file.
 * If a register has a width that differs from BOC_REGISTER_WIDTH (which is set
 * = 8 as the setup bus width, this is also defined.
 *
 * @author: J.C.Hill (hill@hep.phy.cam.ac.uk) 
 *
 */

#include "dsp_types.h"
// defines that used to be in memoryPartitions.h have been moved to comRegDfns.h
//#include "comRegDfns.h"

namespace SctPixelRod {

// Use only the names in this file in C++ code.

//Get the BOC base address from the testBench definitions.
//CE0_BASE is the start of the window for RRIF registers
//Unfortunately, memoryPartitions.h does not define the BOC
//offset from this base in an available form.

const unsigned long BOC_ADDRESS_WINDOW =       0x1000;

// Default width for BOC registers. Registers with smaller range
// are defined explicitly below.
const unsigned long BOC_REGISTER_WIDTH =       8;

//The following addresses are offsets from BOC_ADDRESS_BASE for BOC1

//Start with the BPM12 chips - base for channel 0
const unsigned long BOC_BPM_BASE =             0x000;

//Offsets within BPM12 for individual channel functions
enum bocBpmRegisters {
	BOC_BPM_INHIBIT =        0x0,
	BOC_BPM_MARK_SPACE =     0x4,
	BOC_BPM_COARSE =         0x8,
	BOC_BPM_FINE =           0xC
};
const unsigned long BOC_BPM_INHIBIT_WIDTH =    1;
const unsigned long BOC_BPM_MARK_SPACE_WIDTH = 5;
const unsigned long BOC_BPM_COARSE_WIDTH =     5;
const unsigned long BOC_BPM_FINE_WIDTH =       7;

//Start addresses for other "blocks" 
const unsigned long BOC_LASER_DAC =            0x600;
const unsigned long BOC_DATA_DELAY =           0x800;
const unsigned long BOC_DATA_DELAY_WIDTH =     5;
const unsigned long BOC_STROBE_DELAY =         0xA00;
const unsigned long BOC_STROBE_DELAY_WIDTH =   5;
const unsigned long BOC_THRESHOLD_DAC =        0xC00;

//Channel counts (useful for Pixels??)
const unsigned long BOC_TRANSMIT_CHANNELS =    48;
const unsigned long BOC_RECEIVE_CHANNELS =     96;
const unsigned long BOC_STROBE_CHANNELS =      26;

//The following addresses are for single registers.
const unsigned long BOC_BPM_CLK_PHASE =        0x980;
const unsigned long BOC_BREG_CLK_PHASE =       0x98C;
const unsigned long BOC_VERNIER_CLK0_PHASE =   0x990;
const unsigned long BOC_VERNIER_CLK0_WIDTH =   5;
const unsigned long BOC_VERNIER_CLK1_PHASE =   0x994;
const unsigned long BOC_VERNIER_CLK1_WIDTH =   5;

const unsigned long BOC_RESET =                0xF00;
const unsigned long BOC_BPM_RESET =            0xF04;
const unsigned long BOC_TXDAC_CLEAR =          0xF08;
const unsigned long BOC_RXDAC_CLEAR =          0xF0C;

// For series BOCs, all resets are on BOC_RESET as
// individual bits. Also more resets are defined.
enum boc_reset_bits {
	BOC_OK_RESET_BIT = 3,
	BOC_VPIN_RESET_BIT = 4,
	BOC_RXDAC_CLEAR_BIT = 5,
	BOC_TXDAC_CLEAR_BIT = 6,
	BOC_BPM_RESET_BIT = 7
};

const unsigned long BOC_STATUS =               0xF10;

// The meanings of the BOC_STATUS bits are different for
// pre-production and production BOCs. Some bits are
// not used.
enum boc_pre_production_status_bits {
	BOC_PRE_PRODUCTION_SW1 = 0,
	BOC_PRE_PRODUCTION_SW4 = 1,
	BOC_PRE_PRODUCTION_SW5 = 2,
	BOC_PRE_PRODUCTION_SW6 = 3,
	BOC_PRE_PRODUCTION_RODSENSE = 5,
	BOC_PRE_PRODUCTION_LOCLASEN = 6,
	BOC_PRE_PRODUCTION_REMLASEN = 7
};
//
enum boc_production_status_bits {
	BOC_PRODUCTION_BOCOK = 1,
        BOC_PRODUCTION_VBOK = 2,
        BOC_PRODUCTION_VAOK = 3,
        BOC_PRODUCTION_ERRFLAG = 4,
        BOC_PRODUCTION_RODSENSE = 5,
        BOC_PRODUCTION_LOCLASEN= 6,
        BOC_PRODUCTION_REMLASEN = 7
};

const unsigned long BOC_RX_DATA_MODE =         0xF14;
const unsigned long BOC_RX_DATA_MODE_WIDTH =   3;
// Define the Rx Data mode register values
// NB - not bits!!
enum boc_rx_data_mode_values {
	BOC_RX_DATA_MODE_NORMAL = 0,
	BOC_RX_DATA_MODE_TIMING = 1,
	BOC_RX_DATA_MODE_PIXEL2 = 2,
	BOC_RX_DATA_MODE_PIXEL1 = 3,
	BOC_RX_DATA_MODE_CLOCK = 6,
	BOC_RX_DATA_MODE_TRANSPARENT = 7
};
const unsigned long BOC_VERNIER_FINE_PHASE =   0xF20;
const unsigned long BOC_CLK_CONTROL =          0xF28;
//Width of series Clock Control register. Pre-production
//BOCs have the register 1 bit smaller.
const unsigned long BOC_CLK_CONTROL_WIDTH  =   5;
// Define the clock control bits
enum boc_clk_control_bits {
	BOC_CLOCK_INVERT = 0,
	BOC_CLOCK_HALF = 1,
	BOC_CLOCK_VERNIER_BYPASS = 2,
	BOC_CLOCK_BPMPH_BYPASS = 3,
	BOC_CLOCK_PHOS4_FIX = 4
};

const unsigned long BOC_FW_REV =               0xF40;
const unsigned long BOC_HW_REV =               0xF44;
const unsigned long BOC_MODULE_TYPE =          0xF48;
const unsigned long BOC_MANUFACTURER =         0xF4C;
const unsigned long BOC_SERIAL_NUMBER =        0xF60;

//Monitor ADC is new for the series BOCs. Details are different
//for the Rev A, B or C boards.
// Also there are potentially versions of the boards with 10
// or 12-bit ADCs.

const unsigned long BOC_ADC_SETUP =            0xE00;
const unsigned long BOC_ADC_CONFIG =           0xE04;
const unsigned long BOC_ADC_CONVERT =          0xE08;
const unsigned long BOC_ADC_LSB =              0xE10;
const unsigned long BOC_ADC_MSB =              0xE14;
const unsigned long BOC_ADC_MSB_WIDTH[2] =     {2 ,4};

const unsigned long BOC_MONITOR_CHANNELS =     12;

//BOC_BUSY_0 is bit 2 of RRIF_STATUS_1. I can't find the definition of
//this register anywhere, so put it here for now.
//This is now defined as an index elsewhere. Until I decide how to map
//the index to a real address, the name below has changed.

enum rrif_status_1_bits {
	BOC_BUSY_0 =		0x4
};

}; //  End namespace SctPixelRod

#endif //SCTPIXELROD_BOCADDRESSES_H
