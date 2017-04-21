/////////////////////////////////////////////////////////////////////
// PixFeI2.h
// version 0.1
/////////////////////////////////////////////////////////////////////
//
// 02/11/11  Version 0.1 (JGK)
//

//! Class for FE-I4B

#ifndef _PIXLIB_PIXFEI4B
#define _PIXLIB_PIXFEI4B

#include <vector>
#include <string>
#include "PixFe/PixFeI2.h"


namespace SctPixelRod {
  class VmeInterface;
}

namespace PixLib {
  
  class Bits;
  class Config;
  template<class T> class ConfMask;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixFeI1Config;
  class PixFeStructures;
  class PixFeData;
  
  class PixFeI4B : public PixFeI2 {

    friend class PixModule;
		
  public:
		
    PixFeI4B() {} // Default constructor
    PixFeI4B(DBInquire *dbInquire, PixModule *mod, std::string name, int number); // Constructor
    virtual ~PixFeI4B(); // Destructor

    void storeConfig(std::string configName);               // Store a configuration into map
    void  setDelayCalib(float cal);
    float getDelayCalib();
    int   getVcalFromCharge(int cap, float charge);

  };
  
}

#endif
