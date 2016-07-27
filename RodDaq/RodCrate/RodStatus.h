// File: RodStatus.h

#ifndef SCTPIXELROD_RODSTATUS_H
#define SCTPIXELROD_RODSTATUS_H

#include "RodModule.h"
#include "RodPrimList.h"

namespace SctPixelRod {

/*! 
 * @class RodStatus  
 *
 * @brief This is a class for ROD status reports.
 *
 * This class contains the status information for a ROD. For now it does very
 * little.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator, Fredrik Tegenfeldt (fredrik.tegenfeldt@cern.ch)
 */

class RodStatus
{
public:
  RodStatus();                                     // Constructor
  RodStatus( RodModule& );                         // Constructor
  RodStatus( const RodStatus& );                   // Copy constructor
  ~RodStatus();                                    // Destructor
  //! Overload = operator
  RodStatus& operator=( const RodStatus&);
  //! Overload << operator
  friend std::ostream& operator<<(std::ostream& os, RodStatus& rod);
  //! Clears all members
  void clear();
  
  //! Accessor function to get the RodModule
  RodModule *getRod() const { return m_rod; }
  //! Accessor function to get the byte order correctness
  bool isByteOrderOK() const { return m_correctByteOrder; }

  //! Accessor function to get the sanity of the ROD
  bool isSane() const { return m_sane; }

  //! Accessor function to get slot number
  long getSlotNumber() const { return m_slotNumber; };
  
  //! Accessor function to get serial number
  long getSerialNumber() const { return m_serialNumber; };

  //! Accessor function to get revision number
  unsigned long getRevision() const { return m_revision; };

  //! Accessor function to get MDSP revision number
  unsigned long getMdspMapRev() const { return m_mdspMapRev; };

  //! Accessor function to get MDSP program revision
  unsigned long getMdspProgRev() const { return m_mdspProgRev; };

  //! Accessor function to get FPGA formatter revision
  unsigned long getFpgaFmtRev() const { return m_fpgaFmtRev; };

  //! Accessor function to get FPGA Event Fragment Builder revision
  unsigned long getFpgaEfbRev() const { return m_fpgaEfbRev; };

  //! Accessor function to get FPGA router code revision
  unsigned long getFpgaRtrRev() const { return m_fpgaRtrRev; };

  //! Accessor function to get ROD controller FPGA code revision
  unsigned long getFpgaRcfRev() const { return m_fpgaRcfRev; };
  
  //! Accessor function to get number of slaves installed
  long getNumSlaves() const { return m_numSlaves; };
	
  //! Accessor function to get a status register.
  unsigned long getStatusReg(long regNumber) const {return
						      m_statusReg[regNumber]; };

  //! Accessor function to get a command register.
  unsigned long getCommandReg(long regNumber) const {return
						       m_commandReg[regNumber]; };

  //! Accessor function to get the Primitive state
  RodPrimList::PrimState getPrimState() const {return m_primState;};
  
  // Methods
  //! Get text string corresponding to the current primitive state
  const char *getPrimStateTxt() const;

  //! Takes a snapshot of a given ROD.
  void snapShot(RodModule& rod); 

  //! Output status to an xml file
  void writeToXml(std::ofstream& os) const;
  void writeToXml(std::string& fileName) const;

private:
  //! Pointer to the RodModule class
  RodModule *m_rod;
  //! True if byte order is OK
  bool m_correctByteOrder;
  //! True if the ROD seems OK
  bool m_sane;
  //! The ROD serial number. 
  unsigned long m_serialNumber;
  //! The ROD revision
  unsigned long m_revision;
  //! The ROD slot number
  unsigned long m_slotNumber;
  //! The MDSP revision
  unsigned long m_mdspMapRev;
  //! The MDSP program version
  unsigned long m_mdspProgRev;
  //! The FPGA formatter revision
  unsigned long m_fpgaFmtRev;
  //! The FPGA event fragment builder revision
  unsigned long m_fpgaEfbRev;
  //! The FPGA router revision
  unsigned long m_fpgaRtrRev;
  //! The ROD controller FPGA revision
  unsigned long m_fpgaRcfRev;
  //! The number of slave DSPs installed
  unsigned long m_numSlaves;
  //! The status registers.
  unsigned long m_statusReg[3];
  //! The command registers.
  unsigned long m_commandReg[2];
  //! The state of the Primitive transfer operation
  RodPrimList::PrimState m_primState;
  //! Copies members from another RodStatus
  void copy(const RodStatus & other);
  //! Get text string corresponding to the primlist state
  const char *getPrimStateTxt(RodPrimList::PrimState st) const;

};                                   // End of RodStatus declaration
}; // End namespace SctPixelRod

#endif  // SCTPIXELROD_RODSTATUS_H
