/////////////////////////////////////////////////////////////////////
// PixFeI4A.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 09/02/10  Version 0.1 (JGK)
// 04/11/11  Version 1.0 (JGK) - renamed from FeI4 to FeI4A
//

//! Class for FE-I4A

#ifndef _PIXLIB_PIXFEI4A
#define _PIXLIB_PIXFEI4A

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
  
  class PixFeI4A : public PixFeI2 {

    friend class PixModule;
		
  public:
		
    PixFeI4A() {} // Default constructor
    PixFeI4A(DBInquire *dbInquire, PixModule *mod, std::string name, int number); // Constructor
    virtual ~PixFeI4A(); // Destructor

    void storeConfig(std::string configName);               // Store a configuration into map
    void  setDelayCalib(float cal);
    float getDelayCalib();
    int   getVcalFromCharge(int cap, float charge);

  };
  
}

#endif
