/////////////////////////////////////////////////////////////////////
// PixMccStructures.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0.0 (CS)
//           Initial release
//

//! MCC structures: commands and data structures

#ifndef _PIXLIB_PIXMCCSTRUCTURES
#define _PIXLIB_PIXMCCSTRUCTURES

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <set>
#include <string>

namespace PixLib {
	
  class Bits;
	
  class PixMccStructures {
  public:

    typedef std::vector<std::string> nameVector;
    typedef nameVector::iterator nameIterator;

    PixMccStructures();  //! Constructor
    ~PixMccStructures(); //! Destructor
		
    //! Command accessor methods
    void setCmd(std::string name, std::string pattern, int npar, int datalen, int outlen);
		
    bool getCmd(std::string name, Bits &pattern, int &nPar, int &dataLen, int &outLen);
    bool getCmd(std::string name, Bits &pattern);
    bool getCmdPar(std::string name, int &nPar);
    bool getCmdData(std::string name, int &dataLen);
    bool getCmdOut(std::string name, int &outLen);
    int  getCmdSize() {return m_commands.size();}
		
    nameIterator cmdBegin();
    nameIterator cmdEnd();

    //! Register accessor methods
    void setReg(std::string name, std::string regId, int writeMask, int readMask);
		
    bool getReg(std::string name, Bits &regId, int &writeMask, int &readMask);
    bool getReg(std::string name, Bits &regId);
    bool getRegWMask(std::string name, int &writeMask);
    bool getRegRMask(std::string name, int &readMask);
    int  getRegSize() {return m_registers.size();}

    nameIterator regBegin();
    nameIterator regEnd();
    
    //! FIFO accessor methods
    void setFifo(std::string name, std::string fifoId);
    void setFifoMask(int mask) {m_fifoMask = mask;}
		
    bool getFifo(std::string name, Bits &fifoId);
    int  getFifoMask() {return m_fifoMask;}
    int  getFifoSize() {return m_fifos.size();}

    nameIterator fifoBegin();
    nameIterator fifoEnd();
		
    //! Header accessor methods
    void setHeader(std::string header) {m_header = Bits(header);}
    Bits& getHeader() {return m_header;}
		
  private:
		
    struct MccCmd {
      MccCmd(std::string patt, int np, int dl, int ol) {
	pattern = Bits(patt);
	nPar    = np;
	dataLen = dl;
	outLen  = ol;	
      }
      Bits pattern;
      int  nPar;
      int  dataLen;
      int  outLen;
    };
		
    struct MccReg {
      MccReg(std::string ri, int wm, int rm) {	
	id        = Bits(ri);
	writeMask = wm;
	readMask  = rm;
      }
      Bits id;
      int  value;
      int  writeMask;
      int  readMask;
    };
    
    struct MccFifo {
      MccFifo(std::string fi) {
	id = Bits(fi);
      }
      Bits id;
    };
    
    std::map<std::string, MccReg>  m_registers; // MCC registers
    std::map<std::string, MccFifo> m_fifos;     // MCC FIFO		
    std::map<std::string, MccCmd>  m_commands;  // MCC commands

    bool m_updateNames;     // Names update flag
    nameVector m_regNames;  // MCC registers names
    nameVector m_fifoNames; // MCC FIFO names		
    nameVector m_cmdNames;  // MCC commands names

    int m_fifoMask; // FIFO r/w mask

    Bits m_header; // Data header bit stream

  private:
    void updateNames(); // Name vectors updating
  };
	
}

#endif
