#include "Config/ConfMask.h"
#include "PixCcpdv1.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Config/Config.h"

#include <string.h>

using namespace PixLib;

PixCcpdv1::PixCcpdv1(DBInquire *dbInquire){
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
  conf["ClassInfo"].addString("ClassName", m_className, "PixCcpdv1", "Derived class name", false);

  // Group general
  conf.addGroup("global");

  conf["global"].addInt("BLRes", m_BLRes, 60, "BLRes value", true);
  conf["global"].addInt("ThRes", m_ThRes, 60, "ThRes value", true);
  conf["global"].addInt("VN", m_VN, 60, "VN value", true);
  conf["global"].addInt("VNFB", m_VNFB, 2, "VNFB value", true);
  conf["global"].addInt("VNFoll", m_VNFoll, 30, "VNFoll value", true);
  conf["global"].addInt("VNLoad", m_VNLoad, 5, "VNLoad value", true);
  conf["global"].addInt("VNDAC", m_VNDAC, 0, "VNDAC value", true);
  conf["global"].addInt("NU1", m_NU1, 0, "NU1 value", true);
  conf["global"].addInt("NU2", m_NU2, 0, "NU2 value", true);
  conf["global"].addInt("NotUsed", m_NotUsed, 0, "NotUsed value", true);
  conf["global"].addInt("VNComp", m_VNComp, 5, "VNComp value", true);
  conf["global"].addInt("VNCompL", m_VNCompL, 5,"VNCompL value", true);
  conf["global"].addInt("VNOut0", m_VNOut0, 60, "VNOut0 value", true);
  conf["global"].addInt("VNOut1", m_VNOut1, 0, "VNOut1 value", true);
  conf["global"].addInt("VNOut2", m_VNOut2, 0, "VNOut2 value", true);

  conf.addGroup("pixel");

//_________________________one value for all pixels__________________
//  conf["pixel"].addInt("InDACR", m_InDACR, 7, "InDACR value", true);
//  conf["pixel"].addInt("InDACL", m_InDACL, 7, "InDACL value", true);
  conf["pixel"].addBool("EnR", m_EnR, false, "EnR value", true);
  conf["pixel"].addBool("EnL", m_EnL, false, "EnL value", true);
  conf["pixel"].addBool("L0", m_L0, false, "L0 value", true);
  conf["pixel"].addBool("L1", m_L1, false, "L1 value", true);
  conf["pixel"].addBool("L2", m_L2, false, "L2 value", true);
  conf["pixel"].addBool("R0", m_R0, false, "R0 value", true);
  conf["pixel"].addBool("R1", m_R1, false, "R1 value", true);
  conf["pixel"].addBool("R2", m_R2, false, "R2 value", true);
  conf["pixel"].addBool("EnCurrent", m_EnCurrent, false, "EnCurrent value", true);
  conf["pixel"].addBool("EnStrip", m_EnStrip, false, "EnStrip value", true);
  
//__________________________________________________________________________________

  // temporary variable to init. Matrix-conf.-object
  //                                      nCol nRow maxVal defVal
  //ConfMask<unsigned short int> prDefault(10, 20,    31,    15);
  //conf["pixel"].addMatrix("PRtest", m_prtest, prDefault, "Pixel register matrix for testing", true);

  ConfMask<unsigned short int> InDACDefault(60, 24,    15,    7);
  conf["pixel"].addMatrix("INDAC", m_InDAC_mask, InDACDefault, "INDAC value", true);

  conf.addGroup("OnPcbDACs");
  conf["OnPcbDACs"].addInt("CCPD_Threshold", m_CcpdThreshold, 7800, "CCPD Threshold value", true);
  conf["OnPcbDACs"].addInt("CCPD_Vcal", m_CcpdVcal, 7800, "CCPD Vcal value", true);

  conf.reset();
  if(dbInquire!=0){
    //std::cout << "PixCcpdv1:: if(dbInquire!=0)" << endl;
	conf.read(dbInquire);
}

}
PixCcpdv1::~PixCcpdv1(){
  delete m_conf;
}


//! Read the config from DB
void PixCcpdv1::loadConfig(std::string configName) {
  m_conf->read(m_dbInquire);
  //  std::cout << "PixCcpdv1::loadConfig(" << configName << ")" << endl;
}


//! Save the config to DB
void PixCcpdv1::saveConfig(std::string configName) {
  m_conf->write(m_dbInquire);
  //std::cout << "PixCcpdv1::saveConfig(" << configName << ")" << endl;
}
