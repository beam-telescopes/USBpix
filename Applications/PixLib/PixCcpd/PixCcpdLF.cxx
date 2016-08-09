#include "Config/ConfMask.h"
#include "PixCcpdLF.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Config/Config.h"

#include <string.h>

using namespace PixLib;

PixCcpdLF::PixCcpdLF(DBInquire *dbInquire){
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
  conf["ClassInfo"].addString("ClassName", m_className, "PixCcpdLF", "Derived class name", false);

  // Group general
  conf.addGroup("global");

  conf["global"].addInt("BLRes", m_BLRes, 17, "BLRes value", true);
  conf["global"].addInt("VN", m_VN, 32, "VN value", true);
  conf["global"].addInt("VPFB", m_VPFB, 28, "VPFB value", true);
  conf["global"].addInt("VNFoll", m_VNFoll, 17, "VNFoll value", true);
  conf["global"].addInt("VPLoad", m_VPLoad, 14, "VPLoad value", true);
  conf["global"].addInt("LSBdacL", m_LSBdacL, 12, "LSBdacL value", true);
  conf["global"].addInt("IComp", m_IComp, 17, "IComp value", true);
  conf["global"].addInt("VStrech", m_VStretch, 15, "VStretch value", true);
  conf["global"].addInt("WGT0", m_WGT0, 10, "WGT0 value", true);
  conf["global"].addInt("WGT1", m_WGT1, 35, "WGT1 value", true);
  conf["global"].addInt("WGT2", m_WGT2, 63, "WGT2 value", true);
  conf["global"].addInt("IDacTEST", m_IDacTEST, 0,"IDacTEST value", true);
  conf["global"].addInt("IDacLTEST", m_IDacLTEST, 0, "m_IDacLTEST value", true);
  conf["global"].addInt("Trim_En", m_Trim_En, 0, "Trim_En value", true);
  
  conf["global"].addBool("Inject_En", m_Inject_En, false, "Inject_En value", true);
  conf["global"].addBool("Monitor_En", m_Monitor_En, false, "Monitor_En value", true);
  conf["global"].addBool("Preamp_En", m_Preamp_En, false, "Preamp_En value", true);

  conf.addGroup("pixel");

  // temporary variable to init. Matrix-conf.-object
  //                                      nCol nRow maxVal defVal
  //ConfMask<unsigned short int> prDefault(10, 20,    31,    15);
  //conf["pixel"].addMatrix("PRtest", m_prtest, prDefault, "Pixel register matrix for testing", true);
  ConfMask<unsigned short int> SW_Ana_Default(1, 24,   1,    0);
  ConfMask<unsigned short int> PixelsDefault(114, 24,    1,    0);
 

  conf["pixel"].addMatrix("SW_Ana", m_SW_Ana_mask, SW_Ana_Default, "SW_Ana value", true);
  conf["pixel"].addMatrix("Pixels", m_Pixels_mask, PixelsDefault, "Pixels value", true);
  

  conf.reset();
  if(dbInquire!=0){
	conf.read(dbInquire);
}

}
PixCcpdLF::~PixCcpdLF(){
  delete m_conf;
}


//! Read the config from DB
void PixCcpdLF::loadConfig(std::string configName) {
  m_conf->read(m_dbInquire);
  //std::cout << "PixCcpdLF::loadConfig(" << configName << ")" << endl;
}


//! Save the config to DB
void PixCcpdLF::saveConfig(std::string configName) {
  m_conf->write(m_dbInquire);
  //std::cout << "PixCcpdLF::saveConfig(" << configName << ")" << endl;
}
