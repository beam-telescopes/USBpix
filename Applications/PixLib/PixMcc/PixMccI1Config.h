/////////////////////////////////////////////////////////////////////
// PixMccI1Config.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 11/03/04  Version 1.0 (CS)
//           Initial release
//

//! Image of internal MCC-I1 configuration

#ifndef _PIXLIB_PIXMCCI1CONFIG
#define _PIXLIB_PIXMCCI1CONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>
#include "PixMcc/PixMccConfig.h"


namespace PixLib {
	
  class Bits;
  class Config;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixMccStructures;
  class PixMccData;
  class PixMcc;

  class PixMccI1Config : public PixMccConfig {

    friend class PixMccI1;

  public:

    PixMccI1Config() {} // Default constructor
    PixMccI1Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name); //! Constructor
    PixMccI1Config(const PixMccConfig& c); //! Copy constructor
    virtual ~PixMccI1Config(); //! Destructor
		
    Config &config() {return *m_conf;} //! Config object accessor

    PixMccConfig& operator = (const PixMccConfig& c); //! Assignment operator

    void writeRegister(std::string regName, int value); // Write into a register mem copy
    int  readRegister(std::string regName);             // Read a register from mem copy

    void loadConfig(std::string configName); // Read the config from DB
    void saveConfig(std::string configName); // Save the config to DB
		
    void dump(std::ostream &out); // Dump MCC config info to output stream

  protected:

    PixConfDBInterface *m_db; // Pointer to db interface
    DBInquire *m_dbInquire;   // Pointer to corresponding db record
	
    std::string m_name; // MCC name
    std::string m_className; // MCC class name
		
    PixMccData m_data;             // MCC config data
    PixMccStructures m_structures; // MCC structures (should become static)

    Config *m_conf;
  
  protected:    

    void setupStructures(); // Setup MCC configuration structures elements
    void setupData(); // Setup MCC configuration default data
    void setupConfigObject(); // Setup MCC configuration block

    PixMccData& data() {return m_data;} //! Config data accessor method
    PixMccStructures& structures() {return m_structures;} //! Config structures accessor method

  };

}

#endif
