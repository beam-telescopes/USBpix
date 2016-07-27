//File: TimSequin.cxx

/*! \file
 * \brief A TIM Sequencer information class
 *
 * This is the implementation of a TIM Sequencer information class.
 * It has no VME communication with TIM.
 *
 * Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
 *
 *  $Id: TimSequin.cxx 8799 2011-01-11 17:13:01Z cvs2svn $
 * $Log$
 */

#include <iostream>
#include <iomanip>

using namespace std;

#include "TimSequin.h"

namespace SctPixelRod {

enum Name { NONE = -999 };

static const int s_masks[2]   =  { TIM_SID, TIM_STT };
static const int s_words[2]   =  {  2,  1 };
static const int s_bits[2][2] = {{ 24, 12 },{ 10, 0 }};

// ************************* Class TimSequin *********************************

//! TimSequin: A class for TIM Sequencer.
/*!
  This is useful for loading the TIM Sequencer.

  Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
*/

// Constructors, destructor, and assignment

// ------------------------- Constructor -------------------------------------

TimSequin::TimSequin() {

  reset();
}

// ------------------------- Destructor --------------------------------------

TimSequin::~TimSequin() {
}

// ---------------------------------------------------------------------------

// Accessor functions

// ------------------------- getBuffer ---------------------------------------

int TimSequin::getBuffer( const UINT16 size, UINT16 buffer[] ) {

  const int n = (m_size > size) ? size : m_size;

  for (int i = 0; i < n; i++) {
    buffer[i] = m_buffer[i];
  }
  return n;
}

// ------------------------- setBuffer ---------------------------------------

int TimSequin::setBuffer( const UINT16 size, const UINT16 buffer[] ) {

  m_size = (size > TIM_SEQ_SIZE) ? TIM_SEQ_SIZE : size;

  for (int i = 0; i < m_size; i++) {
    m_buffer[i] = buffer[i];
  }
  return m_size;
}

// ---------------------------------------------------------------------------

// Member methods

// ------------------------- addByBunch --------------------------------------

void TimSequin::addByBunch( const TimBitBackplane mask, const int bcid ) {

  if (m_lastBCR == NONE) m_lastBCR = - (bcid + m_offset);

  int index = getIndex( bcid, m_lastBCR, m_offset );

  addByIndex( mask, index );
}

// ------------------------- addByIndex --------------------------------------

void TimSequin::addByIndex( const TimBitBackplane mask, const int index ) {

  if (index >= 0) m_buffer[index] |= mask; // serial bits may already be set
  if (index >= m_size) m_size = index + 1;

  UINT32 ids[3] = { 0, 0, 0x3FF };
  int delays[2] = { 1, 1 };

  switch (mask) {
    case TIM_ECR:
      m_l1id = TIM_L1ID_FIRST;
      break;
    case TIM_BCR:
      m_lastBCR = index;
      break;
    case TIM_CAL:
      break;
    case TIM_L1A:
      if (m_lastBCR == NONE) {
          m_lastBCR = getBCID( index, 0, m_offset ); // set BCID = 0
      }
      ids[0] = m_l1id ++;
      ids[1] = getBCID( index, m_lastBCR, m_offset );
      addTrigger( index, ids, delays );
      break;
    default :
      break;
  }
}

// ------------------------- addTrigger --------------------------------------

void TimSequin::addTrigger(
  const int    index, //!< Index of L1A in data buffer
  const UINT32 ids[3],
  const int    delays[2]
) {

  const  int size    = TIM_SEQ_SIZE;
  static int lastL1A = NONE;
  static int next[2] = { 0, 0 };

  if (m_lastBCR == NONE) {
      m_lastBCR = getBCID( index, 0, m_offset ); // set BCID = 0
  }
  if (index >= 0 && index < size) {
    if (lastL1A != NONE && m_buffer[lastL1A] == 0) {
        lastL1A =  NONE;
        next[0] = 0;
        next[1] = 0;
    }
    else {
        lastL1A = index;
    }
    m_buffer[index] |= TIM_L1A; // serial bits may already be set

    for (int i = 0; i < 2; i++) { // loop over serial stream
      if (next[i] < index + delays[i]) {
          next[i] = index + delays[i]; // start bit index
      }
      if (next[i] >= 0 && next[i] < size) {
          m_buffer[ next[i] ++ ] |= s_masks[i]; // set start bit:
      }                             // may = last stop bit for tests only
      for (int j = 0; j < s_words[i] && j < 2; j++) { // loop over word
        INT32 id = ids[ j + i * s_words[0] ];
        int  k;
        for (k = next[i];
             k < next[i] + s_bits[i][j] && k < size;
             k++) {                              // loop over bit
          if (id & 01) m_buffer[k] |= s_masks[i];
          id >>= 1;
        }
        next[i] = k;             // next bit index
      }
      if (delays[i]) next[i] ++; // stop bit index
      if (next[i] > m_size) m_size = next[i];
    }
    if (!delays[0]) m_size ++; // account for delay of 0
  }
}

// ------------------------- fill --------------------------------------------

void TimSequin::fill( const UINT16 size, const UINT16 value ) {

  const int n = (size > TIM_SEQ_SIZE) ? TIM_SEQ_SIZE : size;

  for (int i = 0; i < n; i++) {
    m_buffer[i] = value;
  }
}

// ------------------------- getBCID -----------------------------------------

int TimSequin::getBCID( const int iL1A, const int iBCR, const int offset ) {

  return (iL1A - iBCR - offset);
}

// ------------------------- getIndex ----------------------------------------

int TimSequin::getIndex( const int bcid, const int iBCR, const int offset ) {

  return (bcid + iBCR + offset);
}

// ------------------------- reset -------------------------------------------

// This method is a reset!

void TimSequin::reset() {

  m_size    = 0;
  m_l1id    = TIM_L1ID_FIRST;
  m_lastBCR = NONE;
  m_offset  = TIM_BCID_OFFSET;

  for (int i = 0; i < TIM_SEQ_SIZE; i++) m_buffer[i] = 0;
}

// ------------------------- scan --------------------------------------------

TimScanResults TimSequin::scan( const TimScanControl control ) {

// NB data buffer is not declared volatile, ie it should not point to TIM RAM

  TimScanResults results;
  unsigned char *data;
  int byte;
  const int report   = control.print;
  int       nextL1ID = control.nextL1ID;
  int       nextBCID = control.nextBCID;
  int       nextTTID = control.nextTTID;
  const int incr = sizeof( m_buffer[0] );
  const int size = control.size * incr;

  int next[2] = { -1, -1 }; // next index
  int countData = 0;
  int countL1A = 0;
  int countECR = 0;
  int countBCR = 0;
  int countCAL = 0;
  int nextCMD  = 0;
  int lastL1A  = NONE;
  int lastBCR  = NONE;
  int lastCAL  = NONE;

  int l1id = NONE, bcid = NONE, ttid = NONE;
  int j;

  if (control.byte == 0) byte = 0; // source
  else                   byte = 1; // sink
  m_scanErrors = 0;

  data = (unsigned char *) m_buffer;

  if (report) {
    cout << endl << "Scan report:" << endl;
    cout <<  "byte "  << byte
         << " size "  << size
         << " incr "  << incr
         << " orbit " << control.orbit
         << endl;
  }

  for (int i = byte; i < size; i += incr) {
    if (data[i]) { // != 0
      countData ++;
      if (data[i] & TIM_SID && i > next[0]) {
        m_scanError( "SerialID: index want", i / incr, next[0] / incr );
      }
      if (data[i] & TIM_STT && i > next[1]) {
        m_scanError( "SerialTT: index want", i / incr, next[1] / incr );
      }
      if (data[i] & TIM_CMD) {
        const int index = i / incr;
        int countCMDs = 0;
        //  NB last nextCMD value taken if multiCMD
        if (index < nextCMD) {
          m_scanError( "deadtime: index want", index, nextCMD );
        }
        if (data[i] & TIM_RES) {
          countCMDs ++;
          m_scanError( "reserved: index data", index, data[i] );
        }
        if (data[i] & TIM_ECR) {
          countECR ++;
          countCMDs ++;
          nextCMD = index + TIM_ECR_DEADTIME;
          nextL1ID = TIM_L1ID_FIRST;
          if (report) m_scanPrintCMD( "ECR index", index, index, "\n" );
        }
        if (data[i] & TIM_BCR) {
          countBCR ++;
          countCMDs ++;
          nextCMD = index + TIM_BCR_DEADTIME;
          if (lastBCR != NONE && index != lastBCR + control.orbit) {
            m_scanError( "BCRorbit: index want", index,
                          lastBCR + control.orbit );
          }
          lastBCR = index;
          if (report) m_scanPrintCMD( "BCR index", index, index, "\n" );
        }
        if (data[i] & TIM_CAL) {
          countCAL ++;
          countCMDs ++;
          nextCMD = index + TIM_CAL_DEADTIME;
          lastCAL = index;
          if (report) m_scanPrintCMD( "CAL index", index, index, "\n" );
        }
        if (data[i] & TIM_L1A) {
          countL1A ++;
          countCMDs ++;
          nextCMD = index + TIM_L1A_DEADTIME;
          lastL1A = index;
          if (lastBCR != NONE) {
            nextBCID = getBCID( index, lastBCR, control.offset );
          }
          if (lastCAL != NONE && index != lastCAL + control.pipeline) {
            m_scanError( "pipeline: index want", index,
                          lastCAL + control.pipeline );
          }
          lastCAL = NONE;

          INT32 ids[2][2] = {{ NONE,NONE },{ NONE,NONE }};
          int  stop[2]    =  { NONE,NONE };
          int start[2];

          for (int is = 0; is < 2; is++) { // loop over serial stream

            // Scan for serial start bit (allow starting on last stop bit)

            if (next[is] < i) next[is] = i;
            for (j = next[is];
                 j < size && (data[j] & s_masks[is]) == 0;
                 j += incr)
                ;
            if (j < size) start[is] = j / incr; // start bit index
            else          start[is] = 0;
            next[is] = j + incr; // next bit index may be > size

            // Scan serial stream until stop bit reached

            for (int iw = 0;
                     iw < s_words[is] && iw < 2;
                     iw++) { // loop over word
              INT32 id = 0;
              const int bitSetMask = 01 << (s_bits[is][iw] - 1);

              for (j = next[is];
                   j < next[is] + s_bits[is][iw] * incr;
                   j += incr) { // loop over bit
                id >>= 1;
                if (j < size && (data[j] & s_masks[is])) id += bitSetMask;
              }
              ids [is][iw] = id;
              next[is] = j;      // next bit index may be >> size
            }
            if (j >= size) {
              if (stop[is] == NONE) {
                m_scanError( "overflow: index loop", j/incr, is );
              }
            }
            else {
              if (data[j] & s_masks[is]) {
                m_scanError( "stop bit: index data", j/incr, data[j] );
              }
            }
            stop[is] = j / incr; // stop bit index (j = next[is])
          }

          l1id = ids[0][0];
          bcid = ids[0][1];
          ttid = ids[1][0];
          if (report) m_scanPrintL1A( index, start, stop, bcid, l1id, ttid );
          if (nextL1ID != NONE && l1id != nextL1ID) {
            m_scanError( "nextL1ID: L1id  want", l1id, nextL1ID );
          }
          nextL1ID = l1id + 1;

          if (lastBCR == NONE) {
              lastBCR = getBCID( lastL1A, bcid, control.offset );
              if (lastBCR + control.orbit < 0) lastBCR += control.orbit;
          } //    lastBCR may be negative
          else {
            if (bcid != ((nextBCID + control.orbit) % control.orbit)) {
              m_scanError( "nextBCID: BCid  want", bcid, nextBCID );
            }
          }
          if (nextTTID != NONE && ttid != nextTTID) {
            m_scanError( "nextTYPE: TTid  want", ttid, nextTTID );
          }
        }
        if (countCMDs != 1) {
          m_scanError( "multiCMD: index cmds", index, countCMDs );
        }
      }
    }
  }
  if (report) cout << "Scan end: non-zero bytes " << countData
                   << " errors " << m_scanErrors << endl;

  results.lastL1ID =  l1id;
  results.lastBCID =  bcid;
  results.lastTTID =  ttid;
  results.indexL1A =  lastL1A;
  results.countL1A = countL1A;
  results.countECR = countECR;
  results.countBCR = countBCR;
  results.countCAL = countCAL;
  results.bitCount = countData;
  results.errors   = m_scanErrors;

  return results;
}

// ------------------------- scanDefaults ------------------------------------

TimScanControl TimSequin::scanDefaults() {

  TimScanControl control;

  control.size     = TIM_SEQ_SIZE;
  control.byte     = 1;
  control.print    = 0;
  control.offset   = m_offset;
  control.orbit    = 0x1000;
  control.pipeline = TIM_CAL_DEADTIME;
  control.nextL1ID = NONE;
  control.nextBCID = NONE; // not implemented ***
  control.nextTTID = NONE;

  return control;
}

// ------------------------- scanRun -----------------------------------------

int TimSequin::scanRun() {

  static TimScanControl control = scanDefaults();

  control.offset   = m_offset;
  control.pipeline = TIM_CAL_DEADTIME;
  control.orbit    = m_lastBCR + 1;

  control.size  = m_size;
  control.byte  = 0;
  control.print = 1;

  TimScanResults results = scan( control );

  return results.errors;
}

// ---------------------------------------------------------------------------

// Private methods

// ------------------------- m_scanError -------------------------------------

void TimSequin::m_scanError( const char *s, const int x, const int y ) {

  m_scanErrors ++;

  cout << "SCAN ERROR: " << s
       << hex << " " << setw(4) << x
              << " " << setw(4) << y
       << dec << " diff " << x - y << endl;
}

// ------------------------- m_scanPrintCMD ----------------------------------

void TimSequin::m_scanPrintCMD( const char *s, const int x, const int y,
                                const char *c ) {
  cout << s << " "
       << dec << setw(5) << x << " " << setfill('0')
       << hex << setw(4) << y <<  c  << setfill(' ')
       << dec;
}

// ------------------------- m_scanPrintL1A ----------------------------------

void TimSequin::m_scanPrintL1A( const int index, const int start[2],
     const int stop[2], const int bcid, const int l1id, const int ttid ) {

  cout.setf(ios::uppercase);

  m_scanPrintCMD( "L1A index", index, index, " " );

  cout << hex << setiosflags(ios::uppercase) << setfill('0');

  cout << " start " << setw(4) << start[0]
       << " stop "  << setw(4) <<  stop[0]
       << " BCID "  << setw(3) <<  bcid
       << " L1ID "  << setw(6) <<  l1id
       << endl;

  cout << "                     "
       << " start " << setw(4) << start[1]
       << " stop "  << setw(4) <<  stop[1]
       << " TTID "  << setw(3) <<  ttid
       << endl;

  cout << dec << setfill(' ');
}

} // End namespace SctPixelRod
