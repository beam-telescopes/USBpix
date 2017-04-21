/////////////////////////////////////////////////////////////////////
// PixMccI2Config.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//           Initial release
//

//! Image of internal MCC-I2 configuration

#ifndef _PIXLIB_PIXMCCI2CONFIG
#define _PIXLIB_PIXMCCI2CONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>
#include "PixMcc/PixMccI1Config.h"


namespace PixLib {
	
  class Bits;
  class Config;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixMccStructures;
  class PixMccData;
  
  class PixMccI2Config : public PixMccI1Config {
    
    friend class PixMccI2;
    
  public:
    PixMccI2Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name); //! Constructor
    PixMccI2Config(const PixMccConfig& c); //! Copy constructor
    virtual ~PixMccI2Config(); //! Destructor
		
    PixMccConfig& operator = (const PixMccConfig& c); //! Assignment operator

  protected:
    void setupStructures(); // Setup MCC configuration structures elements
    void setupData(); // Setup MCC configuration default data
  };

}

#endif
