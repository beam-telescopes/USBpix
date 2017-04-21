/////////////////////////////////////////////////////////////////////
// PixFeI1.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//	     Initial release
//
// 15/03/04  Version 1.1 (CS)
//           Revised version using PixFeI1Config
//

//! FE-I1 implementation

#include <math.h>
#include <iomanip>
#include <cmath>

#include "Bits/Bits.h"
#include "Config/ConfMask.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeData.h"
#include "PixFe/PixFeConfig.h"
#include "PixFe/PixFeI1Config.h"
#include "PixFe/PixFe.h"
#include "PixFe/PixFeI1.h"
#include "PixFe/PixFeExc.h"

using namespace PixLib;


//! Constructor
PixFeI1::PixFeI1(DBInquire *dbInquire, PixModule *mod, std::string name, int number) :
m_dbInquire(dbInquire), m_module(mod), m_name(name), m_data(0) {
  // Create FE config
  m_data = new PixFeI1Config((m_dbInquire!=0)?m_dbInquire->getDB():0, m_dbInquire, m_name, number);
}


//! Destructor
PixFeI1::~PixFeI1() {
  delete m_data;
}

//! Config object accessor
Config &PixFeI1::config() {
  return m_data->config();
} 

//! Config object accessor
Config &PixFeI1::config(std::string configName) {
  if (m_configs.find(configName) != m_configs.end()) {
    return m_configs[configName]->config();
  }
  throw PixFeExc(PixFeExc::CONF_NOT_IN_MAP, PixFeExc::ERROR, m_name);
  return m_data->config();
} 

//! FE Config object accessor
PixFeConfig &PixFeI1::feConfig() {
  return *m_data;
} 

//! FE Config object accessor
PixFeConfig &PixFeI1::feConfig(std::string configName) {
  if (m_configs.find(configName) != m_configs.end()) {
    return *m_configs[configName];
  }
  throw PixFeExc(PixFeExc::CONF_NOT_IN_MAP, PixFeExc::ERROR, m_name);
  return *m_data;
} 

//! Return FE number
int PixFeI1::number() {
  return m_data->number();
}

//! Write a value in the Global Register mem copy
void PixFeI1::writeGlobRegister(std::string regName, int value) {
  m_data->writeGlobRegister(regName, value);
}

//! Read a value from the Global Register mem copy 
int PixFeI1::readGlobRegister(std::string regName) {
  return m_data->readGlobRegister(regName);
}

//! Copy Global Register from mem to real device
void PixFeI1::setGlobRegister() {
}

//! Copy Global Register from real device to mem
void PixFeI1::getGlobRegister() {
}


//! Write a value in the Pixel Register mem copy
void PixFeI1::writePixRegister(std::string regName, ConfMask<bool> &value) {
  m_data->writePixRegister(regName, value);
}

//! Read a value from the Pixel Register mem copy
ConfMask<bool>& PixFeI1::readPixRegister(std::string regName) {
  return m_data->readPixRegister(regName);
}

//! Copy Pixel Register from mem to real device
void PixFeI1::setPixRegister(std::string /*regName*/) {
}

//! Copy Pixel Register from real device to mem
void PixFeI1::getPixRegister(std::string /*regName*/) {
}

//! Step a mask pattern
void PixFeI1::stepPixRegister(std::string /*regName*/, int /*nStep*/) {
}


//! Write a value in the Trim mem copy
void PixFeI1::writeTrim(std::string trimName, ConfMask<unsigned short int> &value) {
  m_data->writeTrim(trimName, value);
}

//! Read a value from the Trim mem copy
ConfMask<unsigned short int>& PixFeI1::readTrim(std::string trimName) {
  return m_data->readTrim(trimName);
}

//! Copy Trim from mem to real device
void PixFeI1::setTrim(std::string /*trimName*/) {
}

//! Copy Trim from real device to mem
void PixFeI1::getTrim(std::string /*trimName*/) {
}


//! Configure the FE
void PixFeI1::configure() {
}

//! Restore a configuration from map
bool PixFeI1::restoreConfig(std::string configName /*= "INITIAL"*/) {
  if (m_configs.find(configName) != m_configs.end()) {
    *m_data = *(m_configs[configName]);
    return true;
  }
  return false;
}

//! Store a configuration into map
void PixFeI1::storeConfig(std::string configName) {
  PixFeI1Config *cfg = new PixFeI1Config(*m_data);
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
  }
  m_configs[configName] = cfg;
}

//! Remove a configuration from the map
void PixFeI1::deleteConfig(std::string configName) {
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
    m_configs.erase(configName);
  }
}

//! Read the config from DB
void PixFeI1::loadConfig(std::string configName) {
  m_data->loadConfig(configName);
}


//! Save the config to DB
void PixFeI1::saveConfig(std::string /*configName*/) {
}


void PixFeI1::dump(std::ostream &out) {
  m_data->dump(out);
}


// Get length of register in no. of bits
bool PixFeI1::getGlobRegisterLength(std::string name, int &length){
  length=0;
  return m_data->structures().getGlobRegLength(name, length);
}
bool PixFeI1::getTrimMax(std::string name, int &maximum){
  maximum = 0;
  return m_data->structures().getTrimMax(name, maximum);
}
unsigned int PixFeI1::nCol(){
  return (unsigned int) m_data->structures().nCol();
}
unsigned int PixFeI1::nRow(){
  return (unsigned int) m_data->structures().nRow();
}
int PixFeI1::getVcalFromCharge(int cap, float charge){
  float vcal_a = m_data->data().getVcalGradient(3);
  float vcal_b = m_data->data().getVcalGradient(2);
  float vcal_c = m_data->data().getVcalGradient(1);
  float vcal_d = m_data->data().getVcalGradient(0);
  float cInj;
  if(cap) cInj = m_data->data().getCInjHi();
  else    cInj = m_data->data().getCInjLo();
  float qBest=0;
  int vBest=0;
  for (float v=0.f; v<1024.f; v+=1.f) {
    float q = (vcal_a*v*v*v + vcal_b*v*v + vcal_c*v + vcal_d)*cInj/0.160218f;
    if (abs(q-charge) < abs(qBest-charge)) {
      qBest = q;
      vBest = (int)v;
    }
  }
  return vBest;
}
