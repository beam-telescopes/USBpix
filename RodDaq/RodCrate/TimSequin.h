//File: TimSequin.h

#ifndef SCTPIXELROD_TIMSEQUIN_H
#define SCTPIXELROD_TIMSEQUIN_H

/*! \file
 * \brief Declare a TIM Sequencer information class
 *
 * This file declares a TIM class for Sequencer information.
 * It has no VME communication with TIM.
 *
 * Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
 *
 *  $Id: TimSequin.h 8799 2011-01-11 17:13:01Z cvs2svn $
 * $Log$
 */

#include "TimDefine.h"

namespace SctPixelRod {

// ------------------------- TimScanControl ----------------------------------

//! Structure definition for control of scanning Sequencer buffer

struct TimScanControl {
  int size;  //!< Size to scan
  int byte;
  int print;
  int offset;
  int orbit;
  int pipeline;
  int nextL1ID;
  int nextBCID;
  int nextTTID;
};

// ------------------------- TimScanResults ----------------------------------

//! Structure definition for results of scanning Sequencer buffer

struct TimScanResults {
  int lastL1ID;
  int lastBCID;
  int lastTTID;
  int indexL1A;
  int countL1A;
  int countECR;
  int countBCR;
  int countCAL;
  int bitCount;
  int errors;
};

// ------------------------- TimSequin ---------------------------------------

//! TimSequin: A class for TIM Sequencer information.
/*!
  This is useful for building TIM Sequences.

  Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
*/

class TimSequin {

public:

  //! Constructor and destructor
  //! There are no copy or assignment operators yet

  TimSequin();
 ~TimSequin();

  // Accessor functions

  int  getBuffer( const UINT16 size,       UINT16 buffer[] );
  int  setBuffer( const UINT16 size, const UINT16 buffer[] );

  int  getLength()                   { return m_size; };
  void setOffset( const int offset ) { m_offset = offset; };

  // Methods

  void addByBunch( const TimBitBackplane mask, const int bcid );
  void addByIndex( const TimBitBackplane mask, const int index );
  void addTrigger( const int index, const UINT32 ids[3], const int delays[2] );

  void fill( const UINT16 size, const UINT16 value );

  int  getBCID(  const int iL1A, const int iBCR, const int offset );
  int  getIndex( const int bcid, const int iBCR, const int offset );

  void reset();
  int  scanRun();

  TimScanResults scan( const TimScanControl control );
  TimScanControl scanDefaults();

private:

  void m_scanError(    const char *s, const int x, const int y );
  void m_scanPrintCMD( const char *s, const int x, const int y,
                       const char *c );
  void m_scanPrintL1A( const int index, const int start[2], const int stop[2],
                       const int bcid,  const int l1id,     const int ttid );

  UINT16 m_buffer[ TIM_SEQ_SIZE ];

  INT32 m_l1id;
  int   m_lastBCR;
  int   m_size;
  int   m_offset;
  int   m_scanErrors;

}; // End class TimSequin declaration

}  // End namespace SctPixelRod

#endif // SCTPIXELROD_TIMSEQUIN_H
