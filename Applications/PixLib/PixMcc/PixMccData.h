/////////////////////////////////////////////////////////////////////
// PixMccData.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//

//! Image of internal MCC data structures

#ifndef _PIXLIB_PIXMCCDATA
#define _PIXLIB_PIXMCCDATA

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>

namespace PixLib {
	
  class Bits;
  class PixMccStructures;
  
  class PixMccData {
  public:
    PixMccData();  //! Constructor
    PixMccData(const PixMccData& d);  //! Copy constructor
    ~PixMccData(); //! Destructor
		
    //! Assignment operator
    PixMccData& operator = (const PixMccData& d);

    //! Strobe related parameters accessor methods
    float& getDelayRange(int n) {return m_delayRange[n];}
    void   setDelayRange(int n, float val) {m_delayRange[n] = val;}

    //! Register accessor methods
    void setReg(std::string name, int value);
    bool getReg(std::string name, int &value);
    int& getReg(std::string name);
		
    typedef std::map<std::string, int>::const_iterator MccRegConstIterator;
    MccRegConstIterator beginReg() {return m_registers.begin();}
    MccRegConstIterator endReg()   {return m_registers.end();}
    
    //! Reset all registers
    void resetReg();

    //! MCC structures setting
    void setMccStructures(PixMccStructures* structures) {m_structures = structures;}

  private:

    float m_delayRange[16]; // MCC CAL strobe delay range

    std::map<std::string, int>  m_registers; // MCC register values

    PixMccStructures* m_structures; // MCC structures
  };

}

#endif
