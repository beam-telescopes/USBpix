/////////////////////////////////////////////////////////////////////
// PixFeI1Config.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 11/03/04  Version 1.0 (CS)
//           Initial release
//

//! Image of internal FE-I1 configuration

#ifndef _PIXLIB_PIXFEI1CONFIG
#define _PIXLIB_PIXFEI1CONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>
#include "PixFe/PixFeConfig.h"


namespace PixLib {
	
  class Bits;
  class Config;
  template<class T> class ConfMask;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixFeStructures;
  class PixFeData;
  class PixFe;

  class PixFeI1Config : public PixFeConfig {

    friend class PixFeI1;

  public:

    PixFeI1Config() {} //! Default constructor
    PixFeI1Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number); //! Constructor
    PixFeI1Config(const PixFeConfig& c); //! Copy constructor
    virtual ~PixFeI1Config(); //! Destructor
		
    PixFeConfig& operator = (const PixFeConfig& c); //! Assignment operator

    //! Config object accessor
    Config &config() {return *m_conf;}

    //! Partial copy methods
    void copyGlobReg(const PixFeConfig& source);
    void copyPixReg(const PixFeConfig& source, std::string name="ALL");
    void copyTrim(const PixFeConfig& source, std::string name="ALL");

    int  number() {return m_data.getIndex();} // Return FE number

    void writeGlobRegister(std::string regName, int value); // Write a value in the Global Register mem copy
    int& readGlobRegister(std::string regName);             // Read a value from the Global Register mem copy

    void writePixRegister(std::string regName, ConfMask<bool> &value); // Write a value in the Pixel Register mem copy
    ConfMask<bool>& readPixRegister(std::string regName);              // Read a value from the Pixel Register mem copy

    void writeTrim(std::string trimName, ConfMask<unsigned short int> &value); // Write a value in the Trim mem copy
    ConfMask<unsigned short int>& readTrim(std::string trimName);              // Read a value from the Trim mem copy

    void loadConfig(std::string configName); // Read the config from DB
    void saveConfig(std::string configName); // Save the config to DB
    
    void dump(std::ostream &out); // Dump FE config info to output stream
    
    PixFeData& data() {return m_data;} //! Config data accessor method
    PixFeStructures& structures() {return m_structures;} //! Config structures accessor method

  protected:

    PixConfDBInterface *m_db; // Pointer to db interface
    DBInquire *m_dbInquire;   // Pointer to corresponding db record
	
    std::string m_name; // FE name
    std::string m_className; // FE derived class name
		
    PixFeData m_data;             // FE config data
    PixFeStructures m_structures; // FE structures (should become static)

    Config *m_conf;
  
  protected:    

    void setupStructures(); // Setup FE configuration structures elements
    void setupData(); // Setup FE configuration default data
    void setupConfigObject(); // Setup FE configuration block

  };

}

#endif
