/////////////////////////////////////////////////////////////////////
// PixFeConfig.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 11/03/04  Version 1.0 (CS)
//           Initial release
//

//! Image of internal FE configuration

#ifndef _PIXLIB_PIXFECONFIG
#define _PIXLIB_PIXFECONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>


namespace PixLib {
	
  class Bits;
  class Config;
  template<class T> class ConfMask;
  class PixFe;
  class PixFeData;
  class PixFeStructures;
  
  class PixFeConfig {
    
    friend class PixFe;
    friend class PixFeI1;
    
  public:

    //! Destructor
    virtual ~PixFeConfig() {};
		
    virtual PixFeConfig& operator = (const PixFeConfig& ) {return *this;} //! Assignment operator

    //! Config object accessor
    virtual Config &config()=0;

    //! Partial copy methods
    virtual void copyGlobReg(const PixFeConfig& source)=0;
    virtual void copyPixReg(const PixFeConfig& source, std::string name="ALL")=0;
    virtual void copyTrim(const PixFeConfig& source, std::string name="ALL")=0;

    virtual int  number()=0; // Return FE number

    virtual void writeGlobRegister(std::string regName, int value)=0; // Write a value in the Global Register mem copy
    virtual int& readGlobRegister(std::string regName)=0;             // Read a value from the Global Register mem copy

    virtual void writePixRegister(std::string regName, ConfMask<bool> &value)=0; // Write a value in the Pixel Register mem copy
    virtual ConfMask<bool>& readPixRegister(std::string regName)=0;              // Read a value from the Pixel Register mem copy

    virtual void writeTrim(std::string trimName, ConfMask<unsigned short int> &value)=0; // Write a value in the Trim mem copy
    virtual ConfMask<unsigned short int>& readTrim(std::string trimName)=0;              // Read a value from the Trim mem copy

    virtual void loadConfig(std::string configName)=0; // Read the config from DB
    virtual void saveConfig(std::string configName)=0; // Save the config to DB

    virtual void dump(std::ostream &out)=0; // Dump FE config info to output stream
		
    //! Config data and structures accessor methods
    virtual PixFeData& data()=0;
    virtual PixFeStructures& structures()=0;
  };

}

#endif
