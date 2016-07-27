//File: RodRegisterBits.h

#ifndef SCTPIXELROD_RODREGISTERBITS_H 
#define SCTPIXELROD_RODREGISTERBITS_H

/*!
 *  RodRegisterBits.h is a wrapper around the testBench C code header file
 * comRegDfns.h. This wrapper adapts things to appear more like C++ and to
 * declare things const so the compiler can catch attempts to change them in the
 * code.
 *
 * When writing C++ code, use only the names in this file, not the names in the
 * C-code files it wraps.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

namespace SctPixelRod {

#include "processor.h"
#include "comRegDfns.h"

// Give aliases to the text buffer bits in the VmeCommand and Status registers

// StatusRegister[0] bits

const unsigned long OUTLIST_READY = SR_OUT_LIST_RDY;
const unsigned long DSPACK = SR_DSP_ACK;

const unsigned long TEXT_BUFF_NOT_EMPTY[4] = {
  SR_TXT_BUFF_NE(0),  // ERR Buffer
  SR_TXT_BUFF_NE(1),  // INFO Buffer
  SR_TXT_BUFF_NE(2),  // DIAG Buffer
  SR_TXT_BUFF_NE(3)   // XFER Buffer
};

const unsigned long SR_TEXT_BIT_MASK[4]  = {1<<SR_TXT_BUFF_NE(0),
            1<<SR_TXT_BUFF_NE(1), 1<<SR_TXT_BUFF_NE(2), 1<<SR_TXT_BUFF_NE(3)};
	    
const unsigned long SR_TEXT_MASK = SR_TEXT_BIT_MASK[0] | SR_TEXT_BIT_MASK[1] |
                                   SR_TEXT_BIT_MASK[2] | SR_TEXT_BIT_MASK[3];

// VmeCommandRegister[0] bits

const unsigned long INLISTRDY = CR_IN_LIST_RDY;

const unsigned long TEXT_BUFF_READ_REQ[4] = {
  CR_TXT_BUFF_RR(0),  // ERR Buffer
  CR_TXT_BUFF_RR(1),  // INFO Buffer
  CR_TXT_BUFF_RR(2),  // DIAG Buffer
  CR_TXT_BUFF_RR(3)   // XFER Buffer
};

} //  End namespace SctPixelRod

#endif // SCTPIXELROD_RODREGISTERBITS_H
