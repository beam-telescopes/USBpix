/////////////////////////////////////////////////////////////////////
// PixFeI2Config.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//           Initial release
//

//! Image of internal FE-I2 configuration

#ifndef _PIXLIB_PIXFEI2CONFIG
#define _PIXLIB_PIXFEI2CONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>
#include "PixFe/PixFeI1Config.h"


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

  class PixFeI2Config : public PixFeI1Config {

    friend class PixFeI2;

  public:

    PixFeI2Config() {} //! Default constructor
    PixFeI2Config(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number); //! Constructor
    PixFeI2Config(const PixFeConfig& c); //! Copy constructor
    virtual ~PixFeI2Config(); //! Destructor
		
    PixFeConfig& operator = (const PixFeConfig& c); //! Assignment operator

    //! Partial copy methods
    void copyGlobReg(const PixFeConfig& source);
    void copyPixReg(const PixFeConfig& source, std::string name="ALL");
    void copyTrim(const PixFeConfig& source, std::string name="ALL");

  protected:

    void setupStructures(); // Setup FE configuration structures elements
    void setupData(); // Setup FE configuration default data
  };

}

#endif
