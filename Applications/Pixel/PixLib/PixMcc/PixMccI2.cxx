//////////////////////////////////////////////////////////////////////
// PixMccI2.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 30/04/04  Version 1.0 (CS)
//           Initial release
//

//! MCC-I2 implementation

#include <math.h>
#include <algorithm>

#include "Bits/Bits.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/PixController.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMccExc.h"
#include "PixMcc/PixMccStructures.h"
#include "PixMcc/PixMccData.h"
#include "PixMcc/PixMccI2Config.h"
#include "PixMcc/PixMccI2.h"


using namespace PixLib;

//! Constructor
PixMccI2::PixMccI2(DBInquire *dbInquire, PixModule *mod, std::string name) {
  // Members initialization
  m_dbInquire = dbInquire;
  m_module = mod;
  m_name = name;

  // Create MCC config
  m_data = new PixMccI2Config((m_dbInquire!=0)?m_dbInquire->getDB():0, m_dbInquire, m_name);
}


//! Destructor
PixMccI2::~PixMccI2() {
}


//! Store a configuration into map
void PixMccI2::storeConfig(std::string configName) {  
  // Look for the requested configuration inside config map
  PixMccI2Config *cfg = new PixMccI2Config(*m_data);
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
  }
  m_configs[configName] = cfg;
}

