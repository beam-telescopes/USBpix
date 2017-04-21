/////////////////////////////////////////////////////////////////////
// PixFeI4A.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 09/02/10  Version 0.1 (JGK)
// 04/11/11  Version 1.0 (JGK) - renamed from FeI4 to FeI4A
//

//! FE-I4A implementation

#include <math.h>
#include <iomanip>
#include <cmath>

#include "Bits/Bits.h"
#include "Config/ConfMask.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeStructures.h"
#include "PixFe/PixFeData.h"
#include "PixFe/PixFeI4AConfig.h"
#include "PixFe/PixFeI4A.h"

using namespace PixLib;


//! Constructor
PixFeI4A::PixFeI4A(DBInquire *dbInquire, PixModule *mod, std::string name, int number) {
  // Members initialization
  m_dbInquire = dbInquire;
  m_module = mod;
  m_name = name;

  // Create FE config
  m_data = new PixFeI4AConfig((m_dbInquire!=0)?m_dbInquire->getDB():0, m_dbInquire, m_name, number);
}

//! Destructor
PixFeI4A::~PixFeI4A() {
}

//! Store a configuration into map
void PixFeI4A::storeConfig(std::string configName) {
  PixFeI4AConfig *cfg = new PixFeI4AConfig(*m_data);
  if (m_configs.find(configName) != m_configs.end()) {
    delete m_configs[configName];
  }
  m_configs[configName] = cfg;
}
void  PixFeI4A::setDelayCalib(float cal){
  ((PixFeI4AConfig*)m_data)->setDelayCalib(cal);
}
float PixFeI4A::getDelayCalib(){
  return ((PixFeI4AConfig*)m_data)->getDelayCalib();
}
int PixFeI4A::getVcalFromCharge(int cap, float charge){
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
