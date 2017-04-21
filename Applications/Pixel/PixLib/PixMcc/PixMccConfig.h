/////////////////////////////////////////////////////////////////////
// PixMccConfig.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 16/03/04  Version 1.0 (CS)
//           Initial release
//

//! Image of internal MCC configuration

#ifndef _PIXLIB_PIXMCCCONFIG
#define _PIXLIB_PIXMCCCONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>


namespace PixLib {
	
  class Bits;
  class Config;
  class PixMcc;
  class PixMccI1;
  class PixMccData;
  class PixMccStructures;
  
  class PixMccConfig {
    
    friend class PixMcc;
    friend class PixMccI1;
    
  public:
    //! Destructor
    virtual ~PixMccConfig() {};
		
    virtual PixMccConfig& operator = (const PixMccConfig&) {return *this;} //! Assignment operator

    virtual Config &config()=0; //! Config object accessor

    virtual void writeRegister(std::string regName, int value)=0;    // Write into a register mem copy
    virtual int  readRegister(std::string regName)=0;                // Read a register from mem copy

    virtual void loadConfig(std::string configName)=0;                // Read the config from DB
    virtual void saveConfig(std::string configName)=0;                // Save the config to DB
		
    virtual void dump(std::ostream &out)=0; // Dump MCC info to output stream
		
  private:
    //! Config data and structures accessor methods
    virtual PixMccData& data()=0;
    virtual PixMccStructures& structures()=0;
  };

}

#endif
