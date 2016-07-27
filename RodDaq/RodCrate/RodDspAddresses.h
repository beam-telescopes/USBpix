//File: RodDspAddresses.h

#ifndef SCTPIXELROD_RODDSPADDRESSES_H 
#define SCTPIXELROD_RODDSPADDRESSES_H

/*!
 * RodDspAddresses.h is a wrapper around the testBench C code header file
 * memoryPartitions.h. This wrapper adapts things to appear more like C++ and to
 * declare things const so the compiler can catch attempts to change them in the
 * code.
 *
 * It also has hard-coded slave addresses which are needed by initSlaveDsp().
 *
 * Use only the names declared in this file when writing C++ code.
 *
 * @author T. Meyer (meyer@iastate.edu)
 */

namespace SctPixelRod {

#include "dsp_types.h"
// defines that used to be in memoryPartitions.h have been moved to comRegDfns.h
//#include "comRegDfns.h"
#include "rodConfiguration.h"

// Use only the names in this file in C++ code.

//AKDEBUG
/* // Give aliases to VmeCommand and RodStatus registers */
/* const unsigned long STATUS_REG[2] = {STATUS_REG_0, STATUS_REG_2}; */
/* const unsigned long COMMAND_REGISTER = COMMAND_REG_0; */

// Define slave DSP addresses for initializing and direct HPI communication
// with slave
const unsigned long SLAVE_HPIC_BASE = 0x00780000;
const unsigned long SLAVE_HPIA_BASE = SLAVE_HPIC_BASE+0x4;
const unsigned long SLAVE_HPID_AUTO_BASE = SLAVE_HPIC_BASE+0x8;
const unsigned long SLAVE_HPID_NOAUTO_BASE = SLAVE_HPIC_BASE+0xC;
const unsigned long SLAVE_HPI_OFFSET = 0x20000;

const unsigned long SLAVE_EMIF_ADDR =0x01800010;

// Define DSP reset timeout
const long DSP_RESET_TIMEOUT = 500;

} //  End namespace SctPixelRod

#endif // SCTPIXELROD_RODDSPADDRESSES_H
