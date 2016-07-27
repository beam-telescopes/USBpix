#ifndef CHIPTEST
#define CHIPTEST
#include <string>
#include <map>

namespace PixLib{
  class Config;
}

class ChipTest {

 public:
  enum chipTestType{GLOBALREG=0, PIXELREG, SCANCHAIN, CTDUMMY};
  enum regTestType{CURRENTCFG, ALLZERO, ALLONE, ALLONE_A, ALLONE_B, ALLONE_C, ALLONE_D, ALLONE_E, ALLONE_F, ALLODD, ALLEVEN, RANDOM, READONLY};

  ChipTest(chipTestType type);
  virtual ~ChipTest();

  static ChipTest *make(chipTestType type);                    //! Factory

  PixLib::Config &config() { return *m_conf; };                //! Configuration object accessor
  std::map<std::string, int> getTypeList(){return m_typeList;};
  chipTestType getType(){return m_type;};
  std::string getTypeName();

 private:
  virtual void configInit();   //! Init configuration structure

 protected:
  PixLib::Config *m_conf;                  //! Configuration object
  std::map<std::string, int> m_typeList;
  chipTestType m_type;
};

class GlobalRegTest : public ChipTest {

 public:
  GlobalRegTest(chipTestType type);
  ~GlobalRegTest();

 private:
  void configInit();

  int m_patternType;
};

class PixelRegTest : public ChipTest {

 public:
  PixelRegTest(chipTestType type);
  ~PixelRegTest();

 private:
  void configInit();

  int  m_patternType;
  int  m_latchType;
  int  m_latchTypeI3;
  bool m_ignoreDCsOff;
  int  m_DCtoTest;
  bool m_bypass;
};

class ScanChainTest : public ChipTest {

 public:
  ScanChainTest(chipTestType type);
  ~ScanChainTest();

 private:
  void configInit();

  std::string m_steeringFile;
  int m_chainType;
  std::string m_dcsDeviceName;

};

#endif // CHIPTEST
