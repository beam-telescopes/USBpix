/////////////////////////////////////////////////////////////////////
// PixMccI2.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//           Initial release
//

//! Class for the MCC-I2

#ifndef _PIXLIB_PIXMCCI2
#define _PIXLIB_PIXMCCI2

#include <vector>
#include <string>
#include "PixMcc/PixMccI1.h"



namespace SctPixelRod {
  class VmeInterface;
}


namespace PixLib {
  
  class Bits;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixMccStructures;
  class PixMccData;
  class PixMccConfig;

  class PixMccI2 : public PixMccI1 {
	
    friend class PixModule;

  public:
		    
    PixMccI2(DBInquire *dbInquire, PixModule *mod, std::string name); // Constructor
    virtual ~PixMccI2(); // Destructor

    void storeConfig(std::string configName);               // Store a configuration into map

  };
  
}

#endif
