/////////////////////////////////////////////////////////////////////
// PixFeI2.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//	     Initial release
//

//! FE-I2 implementation

#include <math.h>
#include <iomanip>

#include "Bits/Bits.h"
#include "Config/ConfMask.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeData.h"
#include "PixFe/PixFeI2Config.h"
#include "PixFe/PixFeI2.h"

using namespace PixLib;


//! Constructor
PixFeI2::PixFeI2(DBInquire *dbInquire, PixModule *mod, std::string name, int number) {
  // Members initialization
  m_dbInquire = dbInquire;
  m_module = mod;
  m_name = name;

  // Create FE config
  m_data = new PixFeI2Config((m_dbInquire!=0)?m_dbInquire->getDB():0, m_dbInquire, m_name, number);
}

//! Destructor
PixFeI2::~PixFeI2() {
}

//! Store a configuration into map
void PixFeI2::storeConfig(std::string configName) {
  PixFeI2Config *cfg = new PixFeI2Config(*m_data);
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
  }
  m_configs[configName] = cfg;
}

