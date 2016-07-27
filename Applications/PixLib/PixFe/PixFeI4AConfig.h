/////////////////////////////////////////////////////////////////////
// PixFeI4AConfig.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 09/02/10  Version 0.1 (JGK)
// 04/11/11  Version 1.0 (JGK) - renamed from FeI4 to FeI4A
//

//! Image of internal FE-I4A configuration

#ifndef _PIXLIB_PIXFEI4ACONFIG
#define _PIXLIB_PIXFEI4ACONFIG

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>
#include "PixFe/PixFeI2Config.h"


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

  class PixFeI4AConfig : public PixFeI2Config {

    friend class PixFeI4A;

  public:

    PixFeI4AConfig() {} //! Default constructor
    PixFeI4AConfig(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number); //! Constructor
    PixFeI4AConfig(const PixFeConfig& c); //! Copy constructor
    virtual ~PixFeI4AConfig(); //! Destructor
		
    PixFeConfig& operator = (const PixFeConfig& c); //! Assignment operator

    //! Partial copy methods
    void copyGlobReg(const PixFeConfig& source);
    void copyPixReg(const PixFeConfig& source, std::string name="ALL");
    void copyTrim(const PixFeConfig& source, std::string name="ALL");

    void  setDelayCalib(float cal){m_delayCalib = cal;};
    float getDelayCalib(){return m_delayCalib;};

  protected:

    void setupStructures(); // Setup FE configuration structures elements
    void setupData(); // Setup FE configuration default data
    float m_delayCalib;
  };

}

#endif
