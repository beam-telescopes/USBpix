#include "ChipTest.h"
#include <Config/Config.h>
#include <PixFe/PixFeI2.h>
#include <PixFe/PixFeI4A.h>
#include <iostream>
#include <sstream>
#include <TMath.h>

using namespace PixLib;
using namespace std;

ChipTest::ChipTest(chipTestType type){
  m_type = type;
  m_typeList["GLOBALREG"] = GLOBALREG;
  m_typeList["PIXELREG"] = PIXELREG;
  m_typeList["SCANCHAIN"] = SCANCHAIN;
  configInit();
}
ChipTest::~ChipTest(){
  delete m_conf;
}
void ChipTest::configInit(){
  m_conf = new Config(getTypeName()); 
}
ChipTest* ChipTest::make(chipTestType type){
  ChipTest *ct;
  switch(type){
  case GLOBALREG:
    ct = new GlobalRegTest(type);
    break;
  case PIXELREG:
    ct = new PixelRegTest(type);
    break;
  case SCANCHAIN:
    ct = new ScanChainTest(type);
    break;
  default:
    ct = 0;
  }

  return ct;
}
std::string ChipTest::getTypeName(){
  static std::string defName = "unknown";
  for(std::map<std::string, int>::iterator it=m_typeList.begin(); it!=m_typeList.end(); it++)
    if(it->second==m_type)  return it->first;
  return defName;
}
GlobalRegTest::GlobalRegTest(chipTestType type) : ChipTest(type){
  configInit();
}
GlobalRegTest::~GlobalRegTest(){
}
void GlobalRegTest::configInit(){
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("options");
  std::map<std::string,int> typeMap;
  typeMap["current configuration"] = CURRENTCFG;
  typeMap["read back only"]        = READONLY;
  typeMap["all 0's"]               = ALLZERO;
  typeMap["all 1's part A"]        = ALLONE_A;
  typeMap["all 1's part B"]        = ALLONE_B;
  typeMap["all 1's part C"]        = ALLONE_C;
  typeMap["all 1's part D"]        = ALLONE_D;
  typeMap["all 1's part E"]        = ALLONE_E;
  typeMap["all 1's part F"]        = ALLONE_F;
  typeMap["random"]                = RANDOM;
  conf["options"].addList("Bitpattern", (int &)m_patternType, 0, typeMap, "Type of bit pattern to write", true); 

  conf.reset();
}
PixelRegTest::PixelRegTest(chipTestType type) : ChipTest(type){
  configInit();
}
PixelRegTest::~PixelRegTest(){
}
void PixelRegTest::configInit(){
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("options");
  std::map<std::string,int> typeMap;
  typeMap["current configuration"] = CURRENTCFG;
  typeMap["read back only"]        = READONLY;
  typeMap["all 0's"]               = ALLZERO;
  typeMap["all 1's"]               = ALLONE;
  typeMap["odd rows 1"]            = ALLODD;
  typeMap["even rows 1"]           = ALLEVEN;
  typeMap["random"]                = RANDOM;
  conf["options"].addList("Bitpattern", (int &)m_patternType, 0, typeMap, "Type of bit pattern to write", true); 
  std::map<std::string,int> pixNames;
  pixNames["ALL"] = 0;
  PixFeI4A fe(0,0,"DummyFE",0); // pixel registers are identical in I4A and I4B, so doesn't matter which is used
  Config &feconf = fe.config();
  ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];
  ConfGroup &maskgrp = feconf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
  for(int i=0;i<(int)maskgrp.size();i++){
    std::string prname = maskgrp[i].name();
    prname.erase(0,std::string("PixelRegister_").length());
    // don't include DIGINJ, which is just the shift register used to enable digital injection, can't be tested
    if(prname!="DIGINJ") pixNames[prname] = i+1;
  }
  // trim registers: >1 bit, must add bit index to name
  int maxval, nmask=pixNames.size()+1;
  for(int i=0;i<(int)trimgrp.size();i++){
    std::string prname = trimgrp[i].name();
    prname.erase(0,std::string("Trim_").length());
    if(!fe.getTrimMax(prname,maxval)) maxval=0;
    maxval = (int)TMath::Log2((double)(maxval+1));
    for(int j=0;j<maxval;j++){
      std::stringstream a;
      a << j;
      pixNames[prname+a.str()] = nmask;
      nmask++;
    }
  }
  conf["options"].addList("Latch", (int &)m_latchType, 0, pixNames, "Latch (FE-I4) to be tested", true);
  std::map<std::string,int> pixNamesI3;
  pixNamesI3["ALL"] = 0;
  PixFeI2 fei3(0,0,"DummyFEI3",0); // pixel registers are identical in I4A and I4B, so doesn't matter which is used
  Config &feconfi3 = fei3.config();
  ConfGroup &trimgrpi3 = feconfi3.subConfig("Trim_0/Trim")["Trim"];
  ConfGroup &maskgrpi3 = feconfi3.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
  for(int i=0;i<(int)maskgrpi3.size();i++){
    std::string prname = maskgrpi3[i].name();
    prname.erase(0,std::string("PixelRegister_").length());
    pixNamesI3[prname] = i+1;
  }
  // trim registers: >1 bit, must add bit index to name
  nmask=pixNamesI3.size()+1;
  for(int i=0;i<(int)trimgrpi3.size();i++){
    std::string prname = trimgrpi3[i].name();
    prname.erase(0,std::string("Trim_").length());
    if(!fei3.getTrimMax(prname,maxval)) maxval=0;
    maxval = (int)TMath::Log2((double)(maxval+1));
    for(int j=0;j<maxval;j++){
      std::stringstream a;
      a << j;
      pixNamesI3[prname+a.str()] = nmask;
      nmask++;
    }
  }
  conf["options"].addList("LatchI3", (int &)m_latchTypeI3, 0, pixNamesI3, "Latch (FE-I3) to be tested", true);
  conf["options"].addBool("IgnoreDCsOff", m_ignoreDCsOff, true, "Ignore DCs that are turned off in the total pass/fail result", true);
  conf["options"].addInt("DCtoTest", m_DCtoTest, -1, "DC to be tested (-1: test all DCs)", true);
  conf["options"].addBool("BypassMode", m_bypass, false, "Bypass mode: use IoMux for read-back", true);
 
  conf.reset();
}
ScanChainTest::ScanChainTest(chipTestType type) : ChipTest(type){
  configInit();
}
ScanChainTest::~ScanChainTest(){
}
void ScanChainTest::configInit(){
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("options");
  conf["options"].addString("SteeringFile", m_steeringFile, "C:/Users/jgrosse/configs/dob.patterns.stil", "stil-file", true, 1); 
  std::map<std::string,int> typeMap;
  typeMap["DOB"] = 0;
  typeMap["CMD"] = 1;
  typeMap["ECL"] = 2;
  conf["options"].addList("ChainType", m_chainType, 0, typeMap, "Chain to be tested", true);
  conf["options"].addString("DscName", m_dcsDeviceName, "USB-regulators", "Name of the USB-regulator device in DCS", true); 

  conf.reset();
}
