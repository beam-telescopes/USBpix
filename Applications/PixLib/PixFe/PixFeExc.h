/////////////////////////////////////////////////////////////////////
// PixFeExc.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 15/03/04  Version 1.0 (CS)
//           Initial release
//

//! Exception class for FE classes

#ifndef _PIXLIB_PIXFEEXC
#define _PIXLIB_PIXFEEXC

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif


namespace PixLib {
  
  class PixFeExc {
  public:
    enum ErrorType{CASTING_MISMATCH, INCOMPLETE_CONFIG, GLOB_REG_NOT_FOUND, 
                   PIX_REG_NOT_FOUND, TRIM_NOT_FOUND, CONF_NOT_IN_MAP};
    enum ErrorLevel{INFO, WARNING, ERROR, FATAL};

    //! Constructor
    PixFeExc(ErrorType et, ErrorLevel el, std::string name) :
      m_errorType(et), m_errorLevel(el), m_name(name) {}; 
    //! Destructor
    ~PixFeExc() {};
		
    //! Dump the error
    void dump(std::ostream &out) {
      out << "Pixel FE " << m_name << " -- Level : " << dumpLevel() << " -- Type : "
	<< dumpType();
    }

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
	return "MISMATCH IN CASTING FE TYPES ";
      case INCOMPLETE_CONFIG : 
	return "INCOMPLETE FE CONFIGURATION ";
      case GLOB_REG_NOT_FOUND : 
	return "GLOBAL REGISTER NOT FOUND ";
      case PIX_REG_NOT_FOUND : 
	return "PIXEL REGISTER NOT FOUND ";
      case TRIM_NOT_FOUND : 
	return "TRIM NOT FOUND ";
      case CONF_NOT_IN_MAP : 
	return "CONFIGURATION NOT FOUND IN MAP ";
      default :
	return "UNKNOWN";
      }
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
