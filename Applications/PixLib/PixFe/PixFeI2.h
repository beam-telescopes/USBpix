/////////////////////////////////////////////////////////////////////
// PixFeI2.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//           Initial release
//

//! Class for the FE-I2

#ifndef _PIXLIB_PIXFEI2
#define _PIXLIB_PIXFEI2

#include <vector>
#include <string>
#include "PixFe/PixFeI1.h"


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
  
  class PixFeI2 : public PixFeI1 {

    friend class PixModule;
		
  public:
		
    PixFeI2() {} // Default constructor
    PixFeI2(DBInquire *dbInquire, PixModule *mod, std::string name, int number); // Constructor
    virtual ~PixFeI2(); // Destructor

    void storeConfig(std::string configName);               // Store a configuration into map

  };
  
}

#endif
