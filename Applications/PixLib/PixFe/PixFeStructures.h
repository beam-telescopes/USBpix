/////////////////////////////////////////////////////////////////////
// PixFeStructures.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 27/06/03  Version 1.0.0 (CS)
//           Initial release
//

//! FE structures: commands and data structures

#ifndef _PIXLIB_PIXFESTRUCTURES
#define _PIXLIB_PIXFESTRUCTURES

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>

namespace PixLib {
	
  class Bits;
	
  class PixFeStructures {
  public:
		
    typedef std::vector<std::string> nameVector;
    typedef nameVector::iterator nameIterator;

    PixFeStructures();  //! Constructor
    ~PixFeStructures(); //! Destructor
		
    //! Misc info accessor methods
    void nCol(int col) {m_nCol=col;}
    void nRow(int row) {m_nRow=row;}
    int  nCol() {return m_nCol;}
    int  nRow() {return m_nRow;}

    //! Global register accessor methods
    void setGlobReg(std::string name, int length, int offset);		
    bool getGlobReg(std::string name, int &length, int &offset);
    bool getGlobRegLength(std::string name, int &length);
    bool getGlobRegOffset(std::string name, int &offset);
    int  getGlobRegTotLength();
    int  getGlobRegSize() {return m_globRegisters.size();}

    nameIterator globRegBegin();
    nameIterator globRegEnd();
		
    //! Pixel register accessor methods
    void setPixReg(std::string name, int address);		
    bool getPixReg(std::string name, int &address);
		
    nameIterator pixRegBegin();
    nameIterator pixRegEnd();

    //! Trims accessor methods
    void setTrim(std::string name, int address, int number, int maximum);
    bool getTrim(std::string name, int &address, int &number);
    bool getTrimMax(std::string name, int &maximum);
		
    nameIterator trimBegin();
    nameIterator trimEnd();

  private:
    
    struct FeGlobReg {
      FeGlobReg(int len, int off) {
	length = len;
	offset = off;
      }
      int  length;
      int  offset;
    };
    
    struct FePixReg {
      FePixReg(int addr) {
	address = addr;
      }
      int  address;
    };
    
    struct FeTrim {
      FeTrim(int addr, int num, int max) {
	address = addr;
	number  = num;
	maximum = max;
      }
      int  address;
      int  number;
      int  maximum;
    };
    
    int m_nCol, m_nRow; // Number of columns and rows

    std::map<std::string, FeGlobReg>  m_globRegisters; // FE Global registers
    int m_globRegisterLen; // FE Global register total length

    std::map<std::string, FePixReg>  m_pixRegisters;   // FE Pixel registers
    std::map<std::string, FeTrim>    m_trims;          // FE Trims

    bool m_updateNames;        // Names update flag
    nameVector m_globRegNames; // FE Global registers names
    nameVector m_pixRegNames;  // FE Pixel registers names
    nameVector m_trimNames;    // FE Trim names

  private:
    void updateNames(); // Name vectors updating
  };
}


#endif
