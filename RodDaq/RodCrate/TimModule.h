//File: TimModule.h

#ifndef SCTPIXELROD_TIMMODULE_H
#define SCTPIXELROD_TIMMODULE_H

/*! \file
 * \brief TimModule: A derived class for VME TIM modules.
 *
 * This file declares a TIM class derived from the VmeModule base class.
 *
 * Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
 *
 * $Id: TimModule.h 8175 2009-12-18 21:25:25Z ibragimo $
 *
 * $Log$
 * Revision 1.12.4.1  2008/02/18 18:10:42  akorn
 * initial release of main differences for New DSP merged code
 *
 * Revision 1.12  2005/11/18 17:44:50  jchill
 * Revert to single enum
 *
 * Revision 1.11  2005/11/18 16:47:07  jchill
 * Extra primitives defined to allow for TimRegister and TimRegister3
 *
 * Revision 1.10  2005/05/19 13:16:20  jchill
 * Changes for TIM 3C
 *
 * Revision 1.9  2004/10/01 22:44:33  jbl
 * TimModule new methods
 *
 * Revision 1.8  2004/07/29 20:08:56  jbl
 * TimModule minor bug fixes
 *
 * Revision 1.7  2003/12/04 19:10:49  jbl
 * TimModule uses BaseException
 *
 * Revision 1.6  2003/06/04 15:04:47  tmeyer
 * Removed explicit directory structure from includes
 *
 * Revision 1.5  2003/05/20 19:26:25  jbl
 * TimModule UINT8 & UINT16
 *
 * Revision 1.4  2002/12/11 21:30:50  jbl
 * TimModule major update
 *
 *
 *
 * NB define eg I_AM_LINUX_HOST for processor.h (eg typedef UINT32)
 */

#include "TimDefine.h"
//#include "processor.h"
#include "VmeInterface.h"
#include "VmePort.h"
#include "VmeModule.h"
#include "BaseException.h"

namespace SctPixelRod {

// ------------------------- TimException ------------------------------------

//! TimException: a general exception class for TIM errors.
/*!
  This class is thrown if an error in a TIM operation is detected.

  Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
*/

class TimException : public BaseException {

public:

  TimException( std::string descriptor, int data1, int data2 );
  int         getData1()      { return m_data1; };
  int         getData2()      { return m_data2; };
  virtual void what( std::ostream & );

private:

  int         m_data1;       //!< First  data value returned
  int         m_data2;       //!< Second data value returned
};

// ------------------------- TimModule ---------------------------------------

//! TimModule: A derived class for VME TIM modules.
/*!
  This is the implementation of a TIM class derived from the VmeModule base
  class. It should be the sole interface for VME communication with TIM.

  Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
*/

class TimModule : public VmeModule {

public:

  // Constructor and destructor
  // There are no copy or assignment operators due to VME restrictions.

  TimModule( UINT32 baseAddr, UINT32 mapSize, VmeInterface & ourInterface );

  virtual ~TimModule();

  // Accessor functions

  UINT32   getFirmware()     { return m_firmware; };
  UINT32   getSerialNumber() { return m_serialNumber; };
  VmePort* getVmePort()      { return m_vmePort; };

  // Methods

  void initialize();
  void reset() throw (TimException &);
  void status();

  UINT32 fetchL1ID();                     //!< Read trigger number
  UINT16 fetchTimID();                    //!< Read TIM ID register

  void intTrigStart( const double frequency );           //!< Internal Trigger
  void intTrigStart( const TimMaskFrequency frequency ); //!< Internal Trigger
  void intTrigStop();                               //!< Stop Internal Trigger

  void issueCommand( const TimBitCommand mask );  //!<       Issue TIM command
  void issueVCAL( const UINT8 pipelineDelay );   //!< Issue VCAL + L1A command

  void loadBitClear( const TimRegister addr, const UINT16 mask );//!< Clear bit
  void loadBitSet( const TimRegister addr,   const UINT16 mask );//!<   Set bit
  void loadByteHi( const TimRegister addr, const UINT8 byte ); //!< Upper byte
  void loadByteLo( const TimRegister addr, const UINT8 byte ); //!< Lower byte
  void loadFrequencyFECR( const double frequency ); //!< Load FER/ECR frequency
  void loadFrequencyTrig( const double frequency ); //!< Load trigger frequency

  void msleep( const double milliSecs );

  void ttcRxWrite(const UINT8  address, const UINT8 data);
  UINT8 ttcRxRead(const UINT8 address);

  UINT16 regFetch( const TimRegister addr );
                                          //!< Read from a 16-bit VME register

  void   regLoad( const TimRegister addr, const UINT16 data );
                                          //!<  Write to a 16-bit VME register
  int  regTimeout( const TimRegister addr, const int mask1, const int mask2,
                   const int timeout );

  void seqFetch( const UINT16 size,      UINT16 buffer[] ); //!< Read sequencer
  void seqLoad( const UINT16 size, const UINT16 buffer[] ); //!< Load sequencer
  void seqRun(  const UINT16 size );                        //!< Run  sequencer

  void setupTTC();                                     //!< Setup TTC operation
  void setupVME();                                     //!< Setup VME operation

  void statusPrint( std::ostream& os );                //!< Print TIM status

  UINT16 vmeFetch( const UINT32 addr )
         throw (VmeException &);          //!< Read from a 16-bit VME register

  void   vmeLoad(  const UINT32 addr, const UINT16 data )
         throw (VmeException &);          //!<  Write to a 16-bit VME register

private:

  UINT32   m_firmware;     //!< Firmware version number
  UINT32   m_serialNumber; //!< Board serial number
  VmePort* m_vmePort;      //!< VME Port handle
  UINT8 ttcRxAccess(bool write, const UINT8 address, const UINT8 data);
}; // End class TimModule declaration

}  // End namespace SctPixelRod

// ------------------------- Overload operator<< -----------------------------

//! Overloaded operator to print TIM status

namespace std {

ostream& operator<<( ostream& os, SctPixelRod::TimModule& tim );

}  // End namespace std

#endif // SCTPIXELROD_TIMMODULE_H
