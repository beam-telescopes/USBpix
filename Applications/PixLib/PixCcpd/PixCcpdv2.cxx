#include "Config/ConfMask.h"
#include "PixCcpdv2.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Config/Config.h"

#include <string.h>

using namespace PixLib;

PixCcpdv2::PixCcpdv2(DBInquire *dbInquire){
  // create configuration holder object
  m_conf = new Config("PixCcpd_0/PixCcpd");
  // add some structure to m_conf

  // init. config and read from DB file
  
  m_dbInquire = dbInquire;

 // Create the Config object
  //stringstream configName;
  //configName<<name<<"_0"<<"/PixModule";
  //m_conf = new Config("CCPD_Quatsch");
  Config &conf = *m_conf;

  // Insert the configuration parameters in the Config object
  std::map<std::string, int> tf;
  tf["FALSE"] = 0;
  tf["TRUE"] = 1;

  // Group class info - needed for RootDB file writing/reading
  conf.addGroup("ClassInfo");
  conf["ClassInfo"].addString("ClassName", m_className, "PixCcpdv2", "Derived class name", false);

  // Group general
  conf.addGroup("global");

  conf["global"].addInt("BLRes", m_BLRes, 1, "BLRes value", true);
  conf["global"].addInt("ThRes", m_ThRes, 20, "ThRes value", true);
  conf["global"].addInt("VN", m_VN, 0, "VN value", true);
  conf["global"].addInt("VN2", m_VN2, 60, "VN2 value", true);
  conf["global"].addInt("VNFB", m_VNFB, 1, "VNFB value", true);
  conf["global"].addInt("VNFoll", m_VNFoll, 30, "VNFoll value", true);
  conf["global"].addInt("VNLoad", m_VNLoad, 10, "VNLoad value", true);
  conf["global"].addInt("VNDAC", m_VNDAC, 6, "VNDAC value", true);
  conf["global"].addInt("ThPRes", m_ThPRes, 8, "ThPRes value", true);
  conf["global"].addInt("ThP", m_ThP, 30, "ThP value", true);
  conf["global"].addInt("VNOut", m_VNOut, 50, "VNOut value", true);
  conf["global"].addInt("VNComp", m_VNComp, 10, "VNComp value", true);
  conf["global"].addInt("VNCompLd", m_VNCompLd, 5,"VNCompLd value", true);
  conf["global"].addInt("VNOut1", m_VNOut1, 60, "VNOut1 value", true);
  conf["global"].addInt("VNOut2", m_VNOut2, 60, "VNOut2 value", true);
  conf["global"].addInt("VNOut3", m_VNOut3, 60, "VNOut3 value", true);
  conf["global"].addInt("VNBuffer", m_VNBuffer, 30, "VNBuffer value", true);
  conf["global"].addInt("VPFoll", m_VPFoll, 30, "VPFoll value", true);
  conf["global"].addInt("VNBias", m_VNBias, 0, "VNBias value", true);
  conf["global"].addInt("Q0", m_Q0, 0, "Q0 value", true);
  conf["global"].addInt("Q1", m_Q1, 0, "Q1 value", true);
  conf["global"].addInt("Q2", m_Q2, 0, "Q2 value", true);
  conf["global"].addInt("Q3", m_Q3, 0, "Q3 value", true);
  conf["global"].addInt("Q4", m_Q4, 0, "Q4 value", true);
  conf["global"].addInt("Q5", m_Q5, 0, "Q5 value", true);
  
  

  conf.addGroup("pixel");

//_________________________one value for all pixels__________________
  conf["pixel"].addBool("Str", m_Str, false, "Strip Readout value", true);
  conf["pixel"].addBool("dc", m_dc, false, "Direct Current value", true);
  conf["pixel"].addBool("simplepixel", m_SimplePixel, false, "Enable Simple Pixel", true);

//__________________________________________________________________________________

  // temporary variable to init. Matrix-conf.-object
  //                                      nCol nRow maxVal defVal
  //ConfMask<unsigned short int> prDefault(10, 20,    31,    15);
  //conf["pixel"].addMatrix("PRtest", m_prtest, prDefault, "Pixel register matrix for testing", true);
  ConfMask<unsigned short int> EnableDefault(1, 12,   63,    63);
  ConfMask<unsigned short int> InDACDefault(60, 24,    15,    7);
  ConfMask<unsigned short int> AmpoutDefault(20, 1,   1,    0);
  ConfMask<unsigned short int> MonitorDefault(60, 24,   1,    0);

  conf["pixel"].addMatrix("INDAC", m_InDAC_mask, InDACDefault, "INDAC value", true);
  conf["pixel"].addMatrix("Ampout", m_Ampout_mask, AmpoutDefault, "Ampout value", true);
  conf["pixel"].addMatrix("Enable", m_Enable_mask, EnableDefault, "Enable value", true);
  conf["pixel"].addMatrix("Monitor", m_Monitor_mask, MonitorDefault, "Monitor output", true);
  
  conf.reset();
  if(dbInquire!=0){
	conf.read(dbInquire);
}

}
PixCcpdv2::~PixCcpdv2(){
  delete m_conf;
}


//! Read the config from DB
void PixCcpdv2::loadConfig(std::string configName) {
  m_conf->read(m_dbInquire);
  //std::cout << "PixCcpdv2::loadConfig(" << configName << ")" << endl;
}


//! Save the config to DB
void PixCcpdv2::saveConfig(std::string configName) {
  m_conf->write(m_dbInquire);
  //std::cout << "PixCcpdv2::saveConfig(" << configName << ")" << endl;
}
