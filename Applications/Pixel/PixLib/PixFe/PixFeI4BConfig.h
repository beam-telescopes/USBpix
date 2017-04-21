/////////////////////////////////////////////////////////////////////
// PixFeI4BConfig.h
// version 0.1
/////////////////////////////////////////////////////////////////////
//
// 02/11/11  Version 0.1 (JGK)
//

//! Image of internal FE-I4B configuration

#ifndef _PIXLIB_PIXFEI4BCONFIG
#define _PIXLIB_PIXFEI4BCONFIG

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

  class PixFeI4BConfig : public PixFeI2Config {

    friend class PixFeI4B;

  public:

    PixFeI4BConfig() {} //! Default constructor
    PixFeI4BConfig(PixConfDBInterface *db, DBInquire *dbInquire, std::string name, int number); //! Constructor
    PixFeI4BConfig(const PixFeConfig& c); //! Copy constructor
    virtual ~PixFeI4BConfig(); //! Destructor
		
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
