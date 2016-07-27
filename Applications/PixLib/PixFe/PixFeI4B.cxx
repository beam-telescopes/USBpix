/////////////////////////////////////////////////////////////////////
// PixFeI4B.cxx
// version 0.1
/////////////////////////////////////////////////////////////////////
//
// 02/11/11  Version 0.1 (JGK)
//

//! FE-I4B implementation

#include <math.h>
#include <iomanip>
#include <cmath>

#include "Bits/Bits.h"
#include "Config/ConfMask.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeData.h"
#include "PixFe/PixFeI4BConfig.h"
#include "PixFe/PixFeI4B.h"

using namespace PixLib;


//! Constructor
PixFeI4B::PixFeI4B(DBInquire *dbInquire, PixModule *mod, std::string name, int number) {
  // Members initialization
  m_dbInquire = dbInquire;
  m_module = mod;
  m_name = name;

  // Create FE config
  m_data = new PixFeI4BConfig((m_dbInquire!=0)?m_dbInquire->getDB():0, m_dbInquire, m_name, number);
}

//! Destructor
PixFeI4B::~PixFeI4B() {
}

//! Store a configuration into map
void PixFeI4B::storeConfig(std::string configName) {
  PixFeI4BConfig *cfg = new PixFeI4BConfig(*m_data);
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
  }
  m_configs[configName] = cfg;
}
void  PixFeI4B::setDelayCalib(float cal){
  ((PixFeI4BConfig*)m_data)->setDelayCalib(cal);
}
float PixFeI4B::getDelayCalib(){
  return ((PixFeI4BConfig*)m_data)->getDelayCalib();
}
int PixFeI4B::getVcalFromCharge(int cap, float charge){
  float vcal_a = m_data->data().getVcalGradient(3);
  float vcal_b = m_data->data().getVcalGradient(2);
  float vcal_c = m_data->data().getVcalGradient(1);
  float vcal_d = m_data->data().getVcalGradient(0);
  float cInj;
  switch(cap){
  default:
  case 2:
    cInj = m_data->data().getCInjHi();
    break;
  case 1:
    cInj = m_data->data().getCInjMed();
    break;
  case 0:
    cInj = m_data->data().getCInjLo();
  }
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
