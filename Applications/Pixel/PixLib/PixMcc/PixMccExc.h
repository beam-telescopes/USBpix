/////////////////////////////////////////////////////////////////////
// PixMccExc.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 16/03/04  Version 1.0 (CS)
//           Initial release
//

//! Pixel MCC excepltion class

#ifndef _PIXLIB_PIXMCCEXC
#define _PIXLIB_PIXMCCEXC

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif


namespace PixLib {
  
  class PixMccExc {
  public:
    enum ErrorType{CASTING_MISMATCH, INCOMPLETE_CONFIG,
		   REG_NOT_FOUND, FIFO_NOT_FOUND, CMD_NOT_FOUND, WRONG_MCC_OUTPUT, FIFO_OVERFLOW};
    enum ErrorLevel{INFO, WARNING, ERROR, FATAL};

    //! Constructor
    PixMccExc(ErrorType et, ErrorLevel el, std::string name) :
      m_errorType(et), m_errorLevel(el), m_name(name) {}; 
    //! Destructor
    ~PixMccExc() {};
		
    //! Dump the error
    std::string dumpLevel() {
      switch (m_errorLevel) {
      case INFO : 
	return "INFO";
      case WARNING :
	return "WARNING";
      case ERROR :
	return "ERROR";
      case FATAL :
	return "FATAL";
      default :
	return "UNKNOWN";
      }
    }

    std::string dumpType() {
      switch (m_errorType) {
      case CASTING_MISMATCH : 
        return "MISMATCH IN CASTING MCC TYPES";
      case INCOMPLETE_CONFIG : 
	return "INCOMPLETE MCC CONFIGURATION";
      case REG_NOT_FOUND : 
	return "REGISTER NOT FOUND";
      case FIFO_NOT_FOUND :
	return "FIFO NOT FOUND";
      case CMD_NOT_FOUND :
	return "COMMAND NOT FOUND";
      case WRONG_MCC_OUTPUT :
	return "WRONG MCC OUTPUT";
      default :
	return "UNKNOWN";
      }
    }

    void dump(std::ostream &out) {
      out << "Pixel MCC " << m_name << " -- Level : " << dumpLevel() << " -- Type : "
	<< dumpType();
    }

    //! m_errorType accessor
    ErrorType getErrorType() { return m_errorType; };
    //! m_errorLevel accessor
    ErrorLevel getErrorLevel() { return m_errorLevel; };
    //! m_name accessor
    std::string getCtrlName() { return m_name; };
  private:
    ErrorType m_errorType;
    ErrorLevel m_errorLevel;
    std::string m_name;
  };

}

#endif
