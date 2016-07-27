/////////////////////////////////////////////////////////////////////
// RodPixController.cxx
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//

#ifdef WIN32
#pragma warning(disable: 4290)
#endif

typedef struct { 
	float mu, sigma, a0, sigmaInv;
} GaussianCurve;

typedef struct {
	unsigned int pixelId, tPixel, iterations;
	GaussianCurve s;
	float chi2;
} PixelFit;

#include "processor.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixController/RodPixController.h"
#include "RodCrate/RodModule.h"
#include "RodCrate/BocCard.h"
#include "PixController/PixScan.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "Config/Config.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixMcc/PixMccI1.h"
#include "PixMcc/PixMccI2.h"
#include "PixFe/PixFe.h"
#include "PixFe/PixFeI1.h"
#include "PixFe/PixFeI2.h"
#include "RCCVmeInterface.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#ifndef WIN32
#include <unistd.h>
#endif

using namespace PixLib;
using namespace SctPixelRod;

RodPixController::RodPixController(PixModuleGroup &modGrp, DBInquire *dbInquire) : 
                                   PixController(modGrp, dbInquire) { //! Constructor
  configInit();
  createVmeInterface();
  if(dbInquire!=0) m_conf->read(dbInquire);
  m_preAmpOff = false;
  for (unsigned int i=0; i<32; i++) m_autoDisabledLinks[i] = -1;
  m_scanParInt = NULL;
}

RodPixController::RodPixController(PixModuleGroup &modGrp) : 
                                   PixController(modGrp) { //! Constructor
  configInit();
  createVmeInterface();
  m_preAmpOff = false;
  for (unsigned int i=0; i<32; i++) m_autoDisabledLinks[i] = -1;
  m_scanParInt = NULL;
}

RodPixController::~RodPixController() { //! Destructor
  if (m_conf != NULL) delete m_conf;
  if (m_rod != NULL) delete m_rod;
  if (m_scanParInt) delete [] m_scanParInt;
}

void RodPixController::createVmeInterface() {
  // Create VME interface
  m_vme = NULL;
  try {
    m_vme = SctPixelRod::RCCVmeInterface::makeRCCVmeInterface();
  }
  catch(SctPixelRod::VmeException &v) {
    std::cerr << " ErrorClass = " << v.getErrorClass() << " ErrorCode = " << v.getErrorCode() << std::endl;
  }
  catch(SctPixelRod::BaseException & exc) {
    std::cerr << "Unknown failure during VME Interface setup" << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown failure during VME Interface setup"  << std::endl;
  }
}

void RodPixController::updateRegs(){
  // taken from PixelDAQ's fpgaReconfig()
  printf("%s\n",__PRETTY_FUNCTION__);
  // Get VME port
  VmePort* port = m_rod->getVmePort();
  // Check if the ROD is busy
  unsigned int rodBusy;
  rodBusy = ((port->read32(0xC00014)) & 0x8);
  //if (rodBusy != 0x0) {
    // Configure all FPGA on the ROD
    //port->write32(0xc00000, 0x20);
    // Wait for ROD Busy to clear
    //int count = 0;
    //while (rodBusy != 0x0 && count<20){
      //sleep(1);
      //rodBusy = ((port->read32(0xC00014)) & 0x8);
      //count++;
    //}
  //}
  m_rod->fpgaReset(ALL);
}

void RodPixController::initHW() {
  if (m_vme->ok()) {
    if (m_ctrlID > 4 && m_ctrlID < 22) {
      try {
        int i;
        if (m_rod != NULL) delete m_rod;
        m_rod = new RodModule(m_ctrlID<<24, 0x00c03000, *m_vme, 4);
        m_rod->initialize(true);
        // Enable BUSY histogramming
        //m_rod->getVmePort()->write32(0xc0001c, 0x2);
        m_rod->getVmePort()->write32(0xc0001c, 0x1);
        // Load the memory images and remember the file names
        m_rod->loadAndStartSlaves(m_extFile, 0xf);
        // Set the formatter link maps !!! CHECK
        for (i=0; i<8; i++) {
          //    writeRegister(0x48+i, 0, 32, m_fmtLinkMap[i]);     
          writeRegister(FMT_PXL_BANDWIDTH(i), 0, 32, m_fmtLinkMap[i]);     
        }    
        //AKDEBUG  assume 2*(4 + 1) Textbuffers
        for(int k=0;k<10;k++)setVerboseLevel(VerboseLevelDebug, k);
        m_mode = "RESET";
      }
      catch (RodException &r) {
        throw RodPixControllerExc(RodPixControllerExc::BAD_SLOT_NUM, PixControllerExc::FATAL,  
            getModGroup().getRodName(), m_ctrlID); 
      }
    } else {
      throw RodPixControllerExc(RodPixControllerExc::BAD_SLOT_NUM, PixControllerExc::ERROR,  
          getModGroup().getRodName(), m_ctrlID); 
    }
    if (m_scanParInt) delete [] m_scanParInt;
    m_scanParInt = NULL;
    int i;
    for (i=0; i<4; i++) {
      m_trapRunning[i] = false;
      m_histoRunning[i] = false;
      m_dspModMask[i] = 0x0;
    }
    for (i=0; i<32; i++) {
      m_modPosition[i] = -1;
    }
    m_nMod = 0;
    m_lsNbin = 0;
    m_scanActive = false;
    m_runActive = false;
    m_fitDone = false;
  } else {
      throw RodPixControllerExc(RodPixControllerExc::NO_VME_INTERFACE, PixControllerExc::ERROR,  
				getModGroup().getRodName(), m_ctrlID); 
  }
}

void RodPixController::testHW() {
  if(m_rod==0)
    throw RodPixControllerExc(RodPixControllerExc::NO_VME_INTERFACE, PixControllerExc::ERROR,  
			      getModGroup().getRodName(), m_ctrlID); 
}
void RodPixController::hwInfo(std::string &txt){
  if(m_rod!=0){
    std::stringstream infoROD;
    infoROD << "ROD rev." << std::hex <<(int)m_rod->getRevision() << " SN " << 
      std::dec <<(int)m_rod->getSerialNumber();
    txt = infoROD.str();
  } else
    txt = "ROD not initialised";
}

void RodPixController::setVerboseLevel(int level, int buffernr){
  int nArgs = 2;

  RodPrimitive *psetverbose;
  ConfigureEnvironmentIn *ConfigureEnvironment_In;  
  SendSlavePrimitiveIn *SendSlavePrimitive_In;

  std::cout << "RodPixController::setVerboseLevel: " << buffernr << " to: " << level << std::endl; 

  if(buffernr>1){
    SendSlavePrimitive_In = (SendSlavePrimitiveIn *)new UINT32[(sizeof(ConfigureEnvironmentIn)+sizeof(SendSlavePrimitiveIn))/4 + nArgs];
    /*AKDEBUG assumes 2 buffers per 1 Master and 4 Slaves*/
    SendSlavePrimitive_In->slave = buffernr/2 - 1;
    SendSlavePrimitive_In->primitiveId = SLAVE_CONFIGURE_ENVIRONMENT;
    SendSlavePrimitive_In->nWords = sizeof(ConfigureEnvironmentIn)/4 + nArgs;
    SendSlavePrimitive_In->fetchReply = 0;

    ConfigureEnvironment_In = (ConfigureEnvironmentIn *)&SendSlavePrimitive_In[1];
  }else{
    ConfigureEnvironment_In =  (ConfigureEnvironmentIn *)new UINT32[sizeof(ConfigureEnvironmentIn)/4 + nArgs];
  }

  ConfigureEnvironment_In->item    = SET_VERBOSE_LEVEL;
  ConfigureEnvironment_In->nArgs   = nArgs;
  ConfigureEnvironment_In->dataPtr = (UINT32)NULL;
  UINT32 * arglist = (UINT32 *)&ConfigureEnvironment_In[1];
  arglist[0] = buffernr%2;
  arglist[1] = level;

  if(buffernr>1){
    psetverbose = new RodPrimitive((sizeof(ConfigureEnvironmentIn)+sizeof(SendSlavePrimitiveIn))/4 + nArgs, 0, 
        SEND_SLAVE_PRIMITIVE, 0, 
        (long int*)SendSlavePrimitive_In);
  }else{
    psetverbose = new RodPrimitive((sizeof(ConfigureEnvironmentIn))/4 + nArgs, 0, 
        CONFIGURE_ENVIRONMENT, 0, 
        (long int*)ConfigureEnvironment_In);
  }
  std::cout << "executeMasterPrimitiveSync setVerboseLevel" << std::endl;  
  executeMasterPrimitiveSync(*psetverbose);

}

void RodPixController::configInit() {

  m_hscanData = NULL;
  int i;
  for (i=0; i<4; i++) {
    m_trapRunning[i] = false;
    m_histoRunning[i] = false;
    m_dspModMask[i] = 0x0;
  }
  for (i=0; i<32; i++) {
    m_modPosition[i] = -1;
  }
  m_nMod = 0;
  m_lsNbin = 0;
  m_scanActive = false;
  m_runActive = false;

  m_rod = NULL;
  m_mode = "UNCONF";

  // Create the Config object
  m_conf = new Config("RodPixController"); 
  Config &conf = *m_conf;

  // Group general
  conf.addGroup("general");

  conf["general"].addInt("Slot", m_ctrlID, 10,
		  "Rod slot", true);
  conf["general"].addString("EXTFile", m_extFile, "sdsp_xcode.bin",
			    "XCODE file name", true, 1, "bin");

  // Group FMT
  conf.addGroup("fmt");
  for (unsigned int i=0; i<8; i++) {
    std::ostringstream fnum;
    fnum << i;
    std::string tit = "linkMap_"+fnum.str();
    conf["fmt"].addInt("linkMap_"+fnum.str(), m_fmtLinkMap[i], 0x54320000,
		      "Formatter "+fnum.str()+" link map", true);
  }

  // Select default values
  conf.reset();
}

void RodPixController::sendCommand(Bits commands, int moduleMask) { //! Send command no out
  // Copy the bits stream to the primitive
  int packSize = commands.size()/32;
  if (packSize*32 != commands.size()) packSize++;
  int pLen = sizeof(SendSerialStreamIn)/sizeof(UINT32);
  long int *tsdData = new long int[pLen+packSize];
  for (int i=0; i<packSize; i++) {
    tsdData[i+pLen] = commands.pack(i);
  }

  // Set the output mask
  unsigned int mask1 = 0x0, mask2 = 0x0;
  for (unsigned int ib=0; ib<32; ib++) {
    if (moduleMask&(0x1<<ib)) {
      int pos = m_modPosition[ib];
      if (pos>=0) {
	if (m_modDCI[pos]<32) {
	  mask1 |= (0x1<<m_modDCI[pos]);
	} else {
	  mask2 |= (0x1<<(m_modDCI[pos]-32));
	}
      }
    }
  }

  SendSerialStreamIn *sendSerialStreamIn = (SendSerialStreamIn*)tsdData;
  sendSerialStreamIn->sportId  = 0;
  sendSerialStreamIn->nBits    = packSize*32;
  sendSerialStreamIn->fCapture = 1;
  sendSerialStreamIn->nCount   = 1;
  sendSerialStreamIn->dataPtr  = 0;
  sendSerialStreamIn->maskLo   = mask1;
  sendSerialStreamIn->maskHi   = mask2;

  RodPrimitive* sendStream;
  sendStream = new RodPrimitive(packSize+pLen, 0, SEND_SERIAL_STREAM, 0, (long*)sendSerialStreamIn);
  std::cout << "executeMasterPrimitiveSync sendCommand" << std::endl; 
  m_rod->executeMasterPrimitiveSync(*sendStream);
  delete sendStream;
  delete[] tsdData;
}

void RodPixController::writeModuleConfig(PixModule& mod) {     //! Write module configuration
  int moduleId = mod.moduleId();

  // Update the module info
  if (moduleId < 0 || moduleId > 31) {
    throw RodPixControllerExc(RodPixControllerExc::INVALID_MODID, PixControllerExc::ERROR,  
        getModGroup().getRodName(), m_ctrlID); 
  }
  unsigned int pos;
  if (m_modPosition[moduleId] < 0) {
    // New module
    pos = m_nMod++;
    if (m_nMod >= 32) {
      throw RodPixControllerExc(RodPixControllerExc::TOO_MANY_MODULES, PixControllerExc::FATAL,  
          getModGroup().getRodName(), m_ctrlID); 
    }
    m_modPosition[moduleId] = pos; 
  } else {
    // Redefinition of a module
    pos = m_modPosition[moduleId];
  }

  PixelModuleConfig config;

  strncpy(config.idStr, mod.moduleName().c_str(), 128);
  strncpy(config.tag  , "none", 128);//mod.config().tag().c_str(),128);
  config.revision = 0;//mod.config().rev();

  config.active = true;
  config.moduleIdx = moduleId;
  config.groupId = mod.groupId();
  try{
    config.pTTC = ((ConfInt&)mod.config()["general"]["InputLink"]).getValue();
    config.rx = ((ConfInt&)mod.config()["general"]["OutputLink1"]).getValue(); 
  } catch(...){
    throw RodPixControllerExc(RodPixControllerExc::BAD_CONFIG, PixControllerExc::ERROR,  
			      getModGroup().getRodName(), m_ctrlID); 
  }

  PixMcc *mcc = mod.pixMCC();
  config.mccFlavour = 0;
  if(mcc!=0){
    try{
      config.MCCRegisters.regCSR  = mcc->readRegister("CSR");
      config.MCCRegisters.regLV1  = mcc->readRegister("LV1");
      config.MCCRegisters.regFEEN = mcc->readRegister("FEEN");
      config.MCCRegisters.regWFE  = mcc->readRegister("WFE");
      config.MCCRegisters.regWMCC = mcc->readRegister("WMCC");
      config.MCCRegisters.regCNT  = mcc->readRegister("CNT");
      config.MCCRegisters.regCAL  = mcc->readRegister("CAL");
      config.MCCRegisters.regPEF  = mcc->readRegister("PEF");
      config.MCCRegisters.regWBITD = 0;
      config.MCCRegisters.regWRECD = 0;
      config.MCCRegisters.regSBSR  = 0;
      config.MCCRegisters.regSTR   = 0;
      if(dynamic_cast<PixMccI2*>(mcc)!=0){
	config.mccFlavour = MCC_I2;
	config.MCCRegisters.regWBITD = mcc->readRegister("WBITD");
	config.MCCRegisters.regWRECD = mcc->readRegister("WRECD");
	config.MCCRegisters.regSBSR  = mcc->readRegister("SBSR");
	config.MCCRegisters.regSTR   = mcc->readRegister("CNT");
      }
      else if(dynamic_cast<PixMccI1*>(mcc)!=0){
	config.mccFlavour = MCC_I1;
      }
    } catch(...){
      throw RodPixControllerExc(RodPixControllerExc::BAD_CONFIG, PixControllerExc::ERROR,  
				getModGroup().getRodName(), m_ctrlID); 
    }
    if(config.mccFlavour == 0)
      throw RodPixControllerExc(RodPixControllerExc::BAD_CONFIG, PixControllerExc::ERROR,  
				getModGroup().getRodName(), m_ctrlID); 
  }


  config.maskEnableFEConfig =0;
  config.maskEnableFEScan   =0;
  config.maskEnableFEDacs   =0;
  config.feFlavour = 0;

  for(std::vector<PixFe*>::iterator fe = mod.feBegin(); fe != mod.feEnd(); fe++){
    
    try{
      if(dynamic_cast<PixFeI2*>(*fe)!=0)
	config.feFlavour = FE_I2;
      else if(dynamic_cast<PixFeI1*>(*fe)!=0)
	config.feFlavour = FE_I1;
      if(config.feFlavour == 0)
	throw RodPixControllerExc(RodPixControllerExc::BAD_CONFIG, PixControllerExc::ERROR,  
				  getModGroup().getRodName(), m_ctrlID); 

      // copy FE global register content
      int number = (*fe)->number();

      config.FEConfig[number].FEGlobal.latency		 = (*fe)->readGlobRegister("LATENCY");
      config.FEConfig[number].FEGlobal.dacIVDD2 	 = (*fe)->readGlobRegister("DAC_IVDD2");
      config.FEConfig[number].FEGlobal.dacIP2 		 = (*fe)->readGlobRegister("DAC_IP2");
      config.FEConfig[number].FEGlobal.dacID		 = (*fe)->readGlobRegister("DAC_ID");
      config.FEConfig[number].FEGlobal.dacIP		 = (*fe)->readGlobRegister("DAC_IP");
      config.FEConfig[number].FEGlobal.dacITRIMTH 	 = (*fe)->readGlobRegister("DAC_ITRIMTH");
      config.FEConfig[number].FEGlobal.dacIF		 = (*fe)->readGlobRegister("DAC_IF");
      config.FEConfig[number].FEGlobal.dacITH1		 = (*fe)->readGlobRegister("DAC_ITH1");
      config.FEConfig[number].FEGlobal.dacITH2		 = (*fe)->readGlobRegister("DAC_ITH2");
      config.FEConfig[number].FEGlobal.dacIL		 = (*fe)->readGlobRegister("DAC_IL");
      config.FEConfig[number].FEGlobal.dacIL2 		 = (*fe)->readGlobRegister("DAC_IL2");
      config.FEConfig[number].FEGlobal.dacITRIMIF 	 = (*fe)->readGlobRegister("DAC_ITRIMIF");
      config.FEConfig[number].FEGlobal.dacSpare 	 = (*fe)->readGlobRegister("DAC_SPARE");
      config.FEConfig[number].FEGlobal.threshTOTMinimum  = (*fe)->readGlobRegister("THRESH_TOT_MINIMUM");
      config.FEConfig[number].FEGlobal.threshTOTDouble	 = (*fe)->readGlobRegister("THRESH_TOT_DOUBLE");
      config.FEConfig[number].FEGlobal.capMeasure 	 = (*fe)->readGlobRegister("CAP_MEASURE");
      config.FEConfig[number].FEGlobal.gdac      	 = 0;
      config.FEConfig[number].FEGlobal.muxTestPixel 	 = (*fe)->readGlobRegister("MUX_TEST_PIXEL");
      config.FEConfig[number].FEGlobal.dacMonLeakADC	 = (*fe)->readGlobRegister("DAC_MON_LEAK_ADC");
      config.FEConfig[number].FEGlobal.dacVCAL		 = (*fe)->readGlobRegister("DAC_VCAL");
      config.FEConfig[number].FEGlobal.widthSelfTrigger  = (*fe)->readGlobRegister("WIDTH_SELF_TRIGGER");
      config.FEConfig[number].FEGlobal.muxDO		 = (*fe)->readGlobRegister("MUX_DO");
      config.FEConfig[number].FEGlobal.muxMonHit	 = (*fe)->readGlobRegister("MUX_MON_HIT");
      config.FEConfig[number].FEGlobal.muxEOC 		 = (*fe)->readGlobRegister("MUX_EOC");
      config.FEConfig[number].FEGlobal.frequencyCEU 	 = (*fe)->readGlobRegister("FREQUENCY_CEU");
      config.FEConfig[number].FEGlobal.modeTOTThresh	 = (*fe)->readGlobRegister("MODE_TOT_THRESH");
      config.FEConfig[number].FEGlobal.enableTimestamp	 = (*fe)->readGlobRegister("ENABLE_TIMESTAMP");
      config.FEConfig[number].FEGlobal.enableSelfTrigger = (*fe)->readGlobRegister("ENABLE_SELF_TRIGGER");
      config.FEConfig[number].FEGlobal.spare		 = (*fe)->readGlobRegister("SPARE");
      config.FEConfig[number].FEGlobal.monMonLeakADC	 = (*fe)->readGlobRegister("MON_MON_LEAK_ADC");
      config.FEConfig[number].FEGlobal.monADCRef	 = (*fe)->readGlobRegister("MON_ADC_REF");
      config.FEConfig[number].FEGlobal.enableMonLeak	 = (*fe)->readGlobRegister("ENABLE_MON_LEAK");
      config.FEConfig[number].FEGlobal.statusMonLeak	 = (*fe)->readGlobRegister("STATUS_MON_LEAK");
      config.FEConfig[number].FEGlobal.enableCapTest	 = (*fe)->readGlobRegister("ENABLE_CAP_TEST");
      config.FEConfig[number].FEGlobal.enableBuffer 	 = (*fe)->readGlobRegister("ENABLE_BUFFER");
      config.FEConfig[number].FEGlobal.enableVcalMeasure = (*fe)->readGlobRegister("ENABLE_VCAL_MEASURE");
      config.FEConfig[number].FEGlobal.enableLeakMeasure = (*fe)->readGlobRegister("ENABLE_LEAK_MEASURE");
      config.FEConfig[number].FEGlobal.enableBufferBoost = (*fe)->readGlobRegister("ENABLE_BUFFER_BOOST");
      config.FEConfig[number].FEGlobal.enableCP8	 = (*fe)->readGlobRegister("ENABLE_CP8");
      config.FEConfig[number].FEGlobal.monIVDD2 	 = (*fe)->readGlobRegister("MON_IVDD2");
      config.FEConfig[number].FEGlobal.monID		 = (*fe)->readGlobRegister("MON_ID");
      config.FEConfig[number].FEGlobal.enableCP7	 = (*fe)->readGlobRegister("ENABLE_CP7");
      config.FEConfig[number].FEGlobal.monIP2 		 = (*fe)->readGlobRegister("MON_IP2");
      config.FEConfig[number].FEGlobal.monIP		 = (*fe)->readGlobRegister("MON_IP");
      config.FEConfig[number].FEGlobal.enableCP6	 = (*fe)->readGlobRegister("ENABLE_CP6");
      config.FEConfig[number].FEGlobal.monITRIMTH 	 = (*fe)->readGlobRegister("MON_ITRIMTH");
      config.FEConfig[number].FEGlobal.monIF		 = (*fe)->readGlobRegister("MON_IF");
      config.FEConfig[number].FEGlobal.enableCP5	 = (*fe)->readGlobRegister("ENABLE_CP5");
      config.FEConfig[number].FEGlobal.monITRIMIF 	 = (*fe)->readGlobRegister("MON_ITRIMIF");
      config.FEConfig[number].FEGlobal.monVCAL		 = (*fe)->readGlobRegister("MON_VCAL");
      config.FEConfig[number].FEGlobal.enableCP4	 = (*fe)->readGlobRegister("ENABLE_CP4");
      config.FEConfig[number].FEGlobal.enableCinjHigh 	 = (*fe)->readGlobRegister("ENABLE_CINJ_HIGH");
      config.FEConfig[number].FEGlobal.enableExternal 	 = (*fe)->readGlobRegister("ENABLE_EXTERNAL");
      config.FEConfig[number].FEGlobal.enableTestAnalogRef = (*fe)->readGlobRegister("ENABLE_TEST_ANALOG_REF");
      config.FEConfig[number].FEGlobal.enableDigital	 = (*fe)->readGlobRegister("ENABLE_DIGITAL");
      config.FEConfig[number].FEGlobal.enableCP3	 = (*fe)->readGlobRegister("ENABLE_CP3");
      config.FEConfig[number].FEGlobal.monITH1		 = (*fe)->readGlobRegister("MON_ITH1");
      config.FEConfig[number].FEGlobal.monITH2		 = (*fe)->readGlobRegister("MON_ITH2");
      config.FEConfig[number].FEGlobal.enableCP2	 = (*fe)->readGlobRegister("ENABLE_CP2");
      config.FEConfig[number].FEGlobal.monIL		 = (*fe)->readGlobRegister("MON_IL");
      config.FEConfig[number].FEGlobal.monIL2 		 = (*fe)->readGlobRegister("MON_IL2");
      config.FEConfig[number].FEGlobal.enableCP1	 = (*fe)->readGlobRegister("ENABLE_CP1");
      config.FEConfig[number].FEGlobal.enableCP0	 = (*fe)->readGlobRegister("ENABLE_CP0");
      config.FEConfig[number].FEGlobal.monSpare 	 = (*fe)->readGlobRegister("MON_SPARE");
      
      config.FEConfig[number].FEGlobal.hitbusScaler          = 0; // : 8; ???
      config.FEConfig[number].FEGlobal.selfWidth             = 0; // : 4; ???
      config.FEConfig[number].FEGlobal.selfLatency           = 0; // : 4; ???
      config.FEConfig[number].FEGlobal.aregTrim              = 1; // : 2;
      config.FEConfig[number].FEGlobal.aregMeas              = 0; // : 2;
      config.FEConfig[number].FEGlobal.dregTrim              = 1; // : 2;
      config.FEConfig[number].FEGlobal.dregMeas              = 0; // : 2;
      config.FEConfig[number].FEGlobal.parity                = 0; // : 1;                
      config.FEConfig[number].FEGlobal.enableHitParity       = 0; // : 1;                    
      config.FEConfig[number].FEGlobal.enableMonLeak         = 0; // : 1;            
      config.FEConfig[number].FEGlobal.enableHitbus          = 1; // : 1;                 
      config.FEConfig[number].FEGlobal.monSpare              = 0; // : 1; ???                     
      config.FEConfig[number].FEGlobal.enableAregMeas        = 0; // : 1;
      config.FEConfig[number].FEGlobal.enableAreg            = 0; // : 1;
      config.FEConfig[number].FEGlobal.enableLvdsRegMeas     = 0; // : 1; ???
      config.FEConfig[number].FEGlobal.enableDregMeas        = 0; // : 1;
      config.FEConfig[number].FEGlobal.enableTune            = 0; // : 1;
      config.FEConfig[number].FEGlobal.enableBiasComp        = 1; // : 1;
      config.FEConfig[number].FEGlobal.enableIpMonitor       = 0; // : 1; ???
      
      // Copy FE pixel register content
      int i, row, col;
      
      UINT32* pixValue[4];
      std::string pixName[4];
      pixName[0]="ENABLE"; pixValue[0]=&(config.FEConfig[number].FEMasks.maskEnable[0][0]);
      pixName[1]="SELECT"; pixValue[1]=&(config.FEConfig[number].FEMasks.maskSelect[0][0]);
      pixName[2]="PREAMP"; pixValue[2]=&(config.FEConfig[number].FEMasks.maskPreamp[0][0]);
      pixName[3]="HITBUS"; pixValue[3]=&(config.FEConfig[number].FEMasks.maskHitbus[0][0]);
      
      for(int pixNum=0; pixNum<4; pixNum++) {
	ConfMask<bool> &pixReg = (*fe)->readPixRegister(pixName[pixNum]);
	for(col=0; col<pixReg.ncol(); col++)
	  for(i=0; i<5; i++)
	    *(pixValue[pixNum]+(i*pixReg.ncol() + col)) = 0;
	
	for(row=0; row<pixReg.nrow(); row++)
	  for(col=0; col<pixReg.ncol(); col++)
	    *(pixValue[pixNum]+((row/32)*pixReg.ncol() + col)) |= ((UINT32)pixReg[col][row])<<(row%32);
      }
      
      // Copy FE trim content
      
      UINT8* trimValue[2];
      std::string trimName[2];
      trimName[0]="TDAC"; trimValue[0]=&(config.FEConfig[number].FETrims.dacThresholdTrim[0][0]);
      trimName[1]="FDAC"; trimValue[1]=&(config.FEConfig[number].FETrims.dacFeedbackTrim[0][0]);
      
      for(int trimNum=0; trimNum<2; trimNum++) {
	ConfMask<unsigned short int> &trim = (*fe)->readTrim(trimName[trimNum]);
	for(row=0; row<trim.nrow(); row++)
	  for(col=0; col<trim.ncol(); col++)
	    *(trimValue[trimNum]+(row*trim.ncol() + col)) = (UINT8)trim.get(col,row);
      }
      
      // Various stuff in the configuration structure
      config.FEConfig[number].FEIndex = number; // fe index
      config.FEConfig[number].FECommand.address = ((ConfInt&)(*fe)->config()["Misc"]["Address"]).getValue(); // geographical address
      config.FEConfig[number].FECalib.cinjLo = ((ConfFloat&)(*fe)->config()["Misc"]["CInjLo"]).value();
      config.FEConfig[number].FECalib.cinjHi = ((ConfFloat&)(*fe)->config()["Misc"]["CInjHi"]).value();
      config.FEConfig[number].FECalib.vcalCoeff[0] = ((ConfFloat&)(*fe)->config()["Misc"]["VcalGradient0"]).value();
      config.FEConfig[number].FECalib.vcalCoeff[1] = ((ConfFloat&)(*fe)->config()["Misc"]["VcalGradient1"]).value();
      config.FEConfig[number].FECalib.vcalCoeff[2] = ((ConfFloat&)(*fe)->config()["Misc"]["VcalGradient2"]).value();
      config.FEConfig[number].FECalib.vcalCoeff[3] = ((ConfFloat&)(*fe)->config()["Misc"]["VcalGradient3"]).value();
      
      // Copy mask parameters
      int enDac, enCfg, enScan;
      enDac  = (int)((ConfBool&)(*fe)->config()["Misc"]["ConfigEnable"]).value();
      enScan = (int)((ConfBool&)(*fe)->config()["Misc"]["ScanEnable"]).value();
      enCfg  = (int)((ConfBool&)(*fe)->config()["Misc"]["DacsEnable"]).value();
      config.maskEnableFEConfig &= (~(0x1<<number));  config.maskEnableFEConfig |= enCfg<<number;
      config.maskEnableFEScan   &= (~(0x1<<number));  config.maskEnableFEScan   |= enScan<<number;
      config.maskEnableFEDacs   &= (~(0x1<<number));  config.maskEnableFEDacs   |= enDac<<number;
      
      if(config.feFlavour == FE_I2){
	// special registers for FE-I2
	config.FEConfig[number].FEGlobal.gdac      	         = (*fe)->readGlobRegister("GLOBAL_DAC");
      }
    } catch(...){
      throw RodPixControllerExc(RodPixControllerExc::BAD_CONFIG, PixControllerExc::ERROR,  
				getModGroup().getRodName(), m_ctrlID); 
    }
    
  }

  m_modNum[pos] = moduleId;
  m_modGrp[pos] = config.groupId;
  m_modGrpDef[pos] = config.groupId;
  m_modDCI[pos] = config.pTTC;
  m_modDTO[pos] = config.rx;
  m_modActive[pos] = config.active;
  m_modTrigger[pos] = config.active;

  long int *configInt = (long int *)(&config);
  int configSize = sizeof(PixelModuleConfig)/sizeof(UINT32);
  int headSize = sizeof(LoadModuleConfigIn)/sizeof(UINT32);
  int pDataLen = headSize + configSize;
  UINT32 *pData = new UINT32[pDataLen];
  LoadModuleConfigIn *loadModuleConfigIn = (LoadModuleConfigIn *)pData; 
  loadModuleConfigIn->cfgSize = configSize;
  loadModuleConfigIn->module  = moduleId;
  for (int i=0; i<configSize; i++) pData[i+headSize] = configInt[i];

  loadModuleConfigIn->dataPtr =0;
  loadModuleConfigIn->format  =0;
  loadModuleConfigIn->cfgId   = PHYSICS_CFG;
  RodPrimitive* loadConfP;
  loadConfP = new RodPrimitive(pDataLen, 0, LOAD_MODULE_CONFIG, 0, (long*)loadModuleConfigIn);
  m_rod->executeMasterPrimitiveSync(*loadConfP);
  delete loadConfP;

  delete[] pData;

  // Build module masks form config structure
  int maskStrSize = sizeof(ModuleMaskIn)/sizeof(UINT32);
  int maskStrAuxSize = sizeof(ModuleMaskInAux)/sizeof(UINT32);
  UINT32 *mmData = new UINT32[maskStrSize + maskStrAuxSize];

  ModuleMaskIn *moduleMaskIn;
  ModuleMaskInAux *moduleMaskAux;
  moduleMaskIn  = (ModuleMaskIn*)&(mmData[0]);
  moduleMaskAux = (ModuleMaskInAux*)&(mmData[maskStrSize]);

  moduleMaskIn->dataPtr   = 0;
  moduleMaskIn->nModules  = 1;
  moduleMaskAux->moduleId = m_modNum[pos];

  moduleMaskAux->moduleMask.formatterId   = m_modDTO[pos]/16;
  moduleMaskAux->moduleMask.formatterLink = m_modDTO[pos]%16;
  UINT32 link = (((m_fmtLinkMap[moduleMaskAux->moduleMask.formatterId]) >>
        (16 + 4*moduleMaskAux->moduleMask.formatterLink)) & 0x0f);
  moduleMaskAux->moduleMask.detLine = moduleMaskAux->moduleMask.formatterId*12 + link;
  moduleMaskAux->moduleMask.cmdLine = m_modDCI[pos];

  RodPrimitive* moduleMaskP;
  moduleMaskP = new RodPrimitive((sizeof(ModuleMaskIn)+sizeof(ModuleMaskInAux))/sizeof(UINT32), 
      0, MODULE_MASK, 0, (long*)moduleMaskIn);
  m_rod->executeMasterPrimitiveSync(*moduleMaskP);

  delete moduleMaskP;
  delete[] mmData;

  //readModuleConfigIdentifier test 
  char tag[128], idStr[128];
  UINT32 revision, crc32chksm;
  readModuleConfigIdentifier(moduleId, idStr, tag, &revision,&crc32chksm);
}

void RodPixController::readModuleConfigIdentifier(int moduleId, char* idStr, char* tag, UINT32* revision, UINT32* crc32chksm){
  ReadModuleConfigIn readModuleConfigIn;
  readModuleConfigIn.moduleId  = moduleId;
  readModuleConfigIn.cfgId= PHYSICS_CFG;
  readModuleConfigIn.source  = CONFIG_IDENTIFIER;

  RodPrimitive* readConfP;
  readConfP = new RodPrimitive(sizeof(ReadModuleConfigIn)/sizeof(UINT32), 0, READ_MODULE_CONFIG, 0, (long*)&readModuleConfigIn);

  RodOutList * outList;
  std::cout << "executeMasterPrimitiveSync readModuleConfigIdentifier" << std::endl;  
  m_rod->executeMasterPrimitiveSync(*readConfP, outList);

  if(outList->getMsgLength(1)==66){

    char *outBody  = (char*)   outList->getMsgBody(1);
    for(int i = 0; i < 128; ++i) {
      idStr[i]=outBody[i];
      tag [i] =outBody[i+128];
    }

    UINT32 *numArr = (UINT32*) outList->getMsgBody(1);
    *revision   = numArr[64];
    *crc32chksm = numArr[65];
  }else{
    std::cout <<"ERROR "<< getCtrlName()<<" RodPixController::readModuleConfigIdentifier getMsgLength="<< outList->getMsgLength(1) <<std::endl;
    sprintf(tag  , "NULL");
    sprintf(idStr, "NULL");
    *revision  =0;
    *crc32chksm=0;
  }
  m_rod->deleteOutList();
  delete readConfP;
}

void RodPixController::readModuleConfig(PixModule& mod) {     //! Read module configuration 
  /*
  int configSize = sizeof(Module)/4;
  std::vector<unsigned int> out;
  long int *configInt = (long int *)(&config);
  long int rwData[4] = {1, PHYSICS_MODULE_CONFIG, moduleId, 0};
  RodPrimitive* rwPrim = new RodPrimitive(8, 0, RW_MODULE_DATA, R_RW_MODULE_DATA, rwData);
  executeMasterPrimitiveSync(*rwPrim,out);
  delete rwPrim;
  for (int i=0; i<configSize; i++) {
    configInt[i] = out[i];
  }
  */
}

void RodPixController::sendModuleConfig(unsigned int moduleMask) {            //! Send module configuration
  int mod;
  if (moduleMask == 0) moduleMask = 0xffffffff;
 
  for (mod=0; mod<32; mod++) {
    if ((m_modPosition[mod]>=0) && (moduleMask&(0x1<<mod))) {
      int pos = m_modPosition[mod];
      SendModuleConfigIn sendModuleConfigIn;

      sendModuleConfigIn.moduleId  = m_modNum[pos];
      sendModuleConfigIn.cfgId     = PHYSICS_CFG;
      sendModuleConfigIn.chipMask  = 0; /* Reserved for future */
      sendModuleConfigIn.bitMask   = 0x3fff;

      RodPrimitive* sendConfP;
      sendConfP = new RodPrimitive(sizeof(SendModuleConfigIn)/sizeof(UINT32), 
          0, SEND_MODULE_CONFIG, 0, (long*)&sendModuleConfigIn);
      std::cout << "executeMasterPrimitiveSync sendModuleConfig" << std::endl;
      m_rod->executeMasterPrimitiveSync(*sendConfP);
      delete sendConfP;
    }
  }
  m_preAmpOff = false;
}

void RodPixController::setCalibrationMode() { 
  //===TO RE-IMPLEMENT
  if (m_mode != "CALIBRATION") {
    // Configure links and masks

    /*
    // Turn S-Link off
    writeRegister(RTR_CMND_STAT, 7, 1, 0x1);
    // Create and Send a SET_ROD_MODE primitive to a master DSP
    SetRodModeIn SetRodMode_In;
    SetRodMode_In.mode    = CALIBRATION_MODE+CALIBRATION_SLINK_OVERRIDE_MODE;
    SetRodMode_In.flag    = SET_MODE;
    SetRodMode_In.dataLen = 0;
    SetRodMode_In.data    = 0;

    RodPrimitive* rodmPrim = new RodPrimitive(8, 0, SET_ROD_MODE, R_SET_ROD_MODE, (long int*)&SetRodMode_In);
    executeMasterPrimitiveSync(*rodmPrim);
    delete rodmPrim;
     */
    m_mode = "CALIBRATION";
  }
}

void RodPixController::setConfigurationMode() {
  if (m_mode != "CONFIGURATION") {
    for (int i=0; i<32; i++) {
      disableLink(moduleOutLink(i));
    }
    writeRegister(RRIF_CMND_1,0,32,0x8100001);
    writeRegister(RTR_CMND_STAT, 3, 1, 0x1);
    writeRegister(RTR_CMND_STAT, 7, 1, 0x1);
    m_rod->fpgaReset(EFR);

    m_mode = "CONFIGURATION";
  }
}

int RodPixController::moduleOutLink(int nmod) {  
  if (nmod >= 0 && nmod < 32) {
    // Check if module is defined
    if (m_modPosition[nmod] >=0) {
      // Check if module is active
      if (m_modActive[m_modPosition[nmod]]) {
	int modOutLink = ((ConfInt&)m_modGroup.module(nmod)->config()["general"]["OutputLink1"]).getValue();
	unsigned int modFmtId = modOutLink / 16;
	unsigned int modFmtLink = modOutLink % 16;
	int roSpeed = ((ConfInt&)m_modGroup.module(nmod)->pixMCC()->config()["Registers"]["CSR_OutputMode"]).getValue();
	if (roSpeed == 0x2 || // single 80
	    roSpeed == 0x1) { // double 40
	  if (modFmtLink == 2) {
	    modFmtLink = 1;
	  }
	}
	return modFmtId*16 + modFmtLink;
      }
    }
  }
  return -1;
}

void RodPixController::disableLink(int link) {
  if (link >= 0) {
    unsigned int modFmtId = link / 16;
    unsigned int modFmtLink = link % 16; 
    unsigned int linkMask = readRegister(FMT_LINK_EN(modFmtId),0,4);
    unsigned int linkMaskAnd = 0x0;
    unsigned int linkn = modFmtId*4+modFmtLink;
    if (linkn<32) {
      if ((linkMask&(0x1<<modFmtLink)) == 0) {
	m_autoDisabledLinks[linkn] = 0;
      } else {
	m_autoDisabledLinks[linkn] = 1;
      }
    }
    for (unsigned int i=0; i<4; i++) {
      if (modFmtLink != i) linkMaskAnd |= (0x1<<i);
    }
    linkMask &= linkMaskAnd;
    writeRegister(FMT_LINK_EN(modFmtId),0,4,linkMask);
  }
}

void RodPixController::enableLink(int link) {
  if (link >= 0) {
    unsigned int modFmtId = link / 16;
    unsigned int modFmtLink = link % 16; 
    unsigned int linkMask = readRegister(FMT_LINK_EN(modFmtId),0,4);
    unsigned int linkMaskOr = 0x0;
    unsigned int linkn = modFmtId*4+modFmtLink;
    if (linkn<32) {
      if (m_autoDisabledLinks[linkn] == 1) {
	m_autoDisabledLinks[linkn] = -1;
	for (unsigned int i=0; i<4; i++) {
	  if (modFmtLink == i) linkMaskOr |= (0x1<<i);
	}
	linkMask |= linkMaskOr;
	writeRegister(FMT_LINK_EN(modFmtId),0,4,linkMask);
      }
    }
  }
}

void RodPixController::setRunMode() {
  //===RE-IMPLEMENTATION IN DEVELOPMENT
  std::cout << "!!==================   setRunMode -  first attempt at implementation in NewDSP" << std::endl;

  if (m_mode == "RUN") {
    std::cout<<"RodPixController::setRunMode() sees that m_mode == RUN already, so it will do nothing -- exiting."<<std::endl;
    return;
  }

  // Enable TIM control and set the normal data-taking routing path
  writeRegister(RRIF_CMND_1,0,32,0x8009523);

  // Enable links, set mode bits, and compute command masks

  int commandMaskLo(0), commandMaskHi(0);
  unsigned int linkMask[8];
  int modFmtId, modFmtLink, modInLink,modOutLink;

  for(modFmtId = 0; modFmtId<8; modFmtId++) {
    linkMask[modFmtId] =  0;
  }

  for (int mod=0; mod<32; mod++) {
    bool active = false;
    // Check if module is defined
    if (m_modPosition[mod] >=0) {
      // Check if module is active
      if (m_modActive[m_modPosition[mod]])
        active = true;
    }

    if (active) {
      modOutLink = ((ConfInt&)m_modGroup.module(mod)->config()["general"]["OutputLink1"]).getValue();
      modFmtId = modOutLink / 16;
      modFmtLink = modOutLink % 16; 
      
      int roSpeed = ((ConfInt&)m_modGroup.module(mod)->pixMCC()->config()["Registers"]["CSR_OutputMode"]).getValue();
      if (roSpeed == 0x2 || // single 80
	  roSpeed == 0x1) { // double 40
        if(modFmtLink == 2)
          modFmtLink = 1;
      }
      modInLink = ((ConfInt&)m_modGroup.module(mod)->config()["general"]["InputLink"]).getValue();
      linkMask[modFmtId] |= 0x1<<modFmtLink;

      if(modInLink < 32)
        commandMaskLo |= (0x1 << modInLink);
      else
        commandMaskHi |= (0x1 << (modInLink-32));
    }
  }

  //EFB Dynamic Mask Registers 00404700:
  //ROD event type turns into group id. Only group0 is used for data-taking
  writeRegister(DFLT_ROD_EVT_TYPE ,0,7,0x0);
  writeRegister(FE_MASK_LUT_SELECT,0,3,0x0);

  for(int modFmtId = 0; modFmtId<8; modFmtId++) {

    //enable formatter link
    writeRegister(FMT_LINK_EN(modFmtId),0,4,linkMask[modFmtId]);

    //modebit0=0  modebit1=0 : set to play mode (normal data-taking mode)
    writeRegister(RMB_DFLT_LUT(0,modFmtId,0),0,12,0x0);
    writeRegister(RMB_DFLT_LUT(0,modFmtId,1),0,12,0x0);
  }

  //clear bit6 to disable back pressure to EFB  
  writeRegister(RTR_CMND_STAT,6,1,0);

  //enable S-Link
  writeRegister(RTR_CMND_STAT, 3, 1, 0x0);
  writeRegister(RTR_CMND_STAT, 7, 1, 0x0);

  // Set command masks
  writeRegister(FE_CMND_MASK_0_LO,0,32,commandMaskLo);
  writeRegister(FE_CMND_MASK_0_HI,0,16,commandMaskHi);

  // force command mask load, since BCR won't do it like a trigger would
  writeRegister(RRIF_CMND_1,20,1,0x1); 

  // New FE Command Mask Ready -- self-clearing bit
  writeRegister(RRIF_CMND_1,2,1,0x1);

  // clear bit 20 after command mask is loaded
  writeRegister(RRIF_CMND_1,20,1,0x0); 

  // Enable 3564 rollover in EFB
  writeRegister(EFB_CMND_0, 13, 1, 0x1);


  //Event Capture in InMEM
  //If it is called, the input serial stream will be stored in the input FIFO
  //until next inMEM capture is issues. Since we only call it once here.
  //we can only catch the first triggered event.
  //Reset INMEM 
  //writeRegister(RRIF_CMND_0, INP_MEM_RST_O, 1, 1);
  //writeRegister(RRIF_CMND_0, INP_MEM_RST_O, 1, 0);
  //writeRegister(RRIF_CMND_1, CONFIGURATION_READBACK_O, 1, 0);
  //writeRegister(RRIF_CMND_1, FIFO_CTRL_MUX_O, 3, 4);
  //int delay=0;
  //int nBits=0x8000;
  //int mux = 0x7; //rout to INMEM and FORMATTER
  //Configure INMEM
  //writeRegister(CFG_READBACK_CNT, FIFO_WRITE_CNT_O  , FIFO_WRITE_CNT_W, nBits); 
  //writeRegister(CFG_READBACK_CNT, DELAY_FIFO_WRITE_O, 1, delay);
  //writeRegister(RRIF_CMND_1, FIFO_CTRL_MUX_O, 3, mux);


  // ROD Register Dumps to std::cout

  // Dump RTR_CMND_STAT 
  //std::cout << "RTR_CMND_STAT = " << std::hex << readRegister(RTR_CMND_STAT, 0, 32) << std::dec << std::endl;
  // Dump master reg 0 and 1
  //std::cout << "RRIF_CMND_1 = "   << std::hex << readRegister(RRIF_CMND_1, 0, 32)   << std::dec << std::endl;      
  //std::cout << "RRIF_CMND_0 = "   << std::hex << readRegister(RRIF_CMND_0, 0, 32)   << std::dec << std::endl;      
  // Dump EFB diagnostic regster
  //std::cout << "EVT_MEM_CMND_STAT = " << std::hex << readRegister(EVT_MEM_CMND_STAT, 0, 32) << std::dec << std::endl;      
  //std::cout << "EVT_MEM_MODE = "  << std::hex << readRegister(EVT_MEM_MODE, 0, 32)  << std::dec << std::endl;      
  //std::cout << "EVT_MEM_FLAGS = " << std::hex << readRegister(EVT_MEM_FLAGS, 0, 32) << std::dec << std::endl;      
  //std::cout << "EFB_CMND_0 = "    << std::hex << readRegister(EFB_CMND_0, 0, 32)    << std::dec << std::endl;      
  //std::cout << "EFB_FORMATTER_STAT = "   << std::hex << readRegister(EFB_FORMATTER_STAT, 0, 32) 
  //       << std::dec << std::endl;      
  //std::cout << "EFB_RUNTIME_STAT_REG = " << std::hex << readRegister(EFB_RUNTIME_STAT_REG, 0, 32) 
  //       << std::dec << std::endl;       
  // Dump FMT status
  //for (int i=0; i<8; i++) {
  //  std::cout << "FMT_HEADER_TRAILER_LIMIT" << i << " = " << std::hex 
  //           << readRegister(FMT_HEADER_TRAILER_LIMIT(i), 0, 32) << std::dec << std::endl;      
  //  std::cout << "FMT_ROD_BUSY_LIMIT"       << i << " = " << std::hex 
  //           << readRegister(FMT_ROD_BUSY_LIMIT(i), 0, 32) << std::dec << std::endl;      
  //  std::cout << "FMT_PXL_LINK_L1A_CNT"     << i << " = " << std::hex 
  //           << readRegister(FMT_PXL_LINK_L1A_CNT(i), 0, 32) << std::dec << std::endl;      
  //}

  for (unsigned int i=0; i<32; i++) m_autoDisabledLinks[i] = -1;

  m_mode = "RUN";

}


void RodPixController::writeScanConfig(PixScan &scn) {  //! Write scan parameters
  // Initialize ScanPar structure
  int nLoopTot = 0;
  unsigned int i,j;
  m_nBinTot = 0; 
  for (i=0; i<SCAN_MAXLOOPS; i++) {
    if (scn.getLoopActive(i) && scn.getDspProcessing(i)) {
      nLoopTot++;
      m_nBinTot += scn.getLoopVarNSteps(i);
    } else {
      break;
    }
  }
  int nTaskIn = sizeof(StartTaskIn)/sizeof(unsigned int);
  int nScanOptions = sizeof(ScanOptions)/sizeof(unsigned int);
  // Zero all fields
  if (m_scanParInt != NULL) delete[] m_scanParInt;
  m_scanParInt = new unsigned int[nTaskIn + nScanOptions + m_nBinTot];
  ScanOptions &m_scanPar = (ScanOptions&)(m_scanParInt[nTaskIn]);
  StartTaskIn &startTaskIn = (StartTaskIn&)(m_scanParInt[0]);
  for (unsigned int i=0; i< (nTaskIn + nScanOptions + m_nBinTot); i++) m_scanParInt[i] = 0;
  // Load scan config
  if (scn.getMaskStageMode() == PixScan::STATIC) {
    m_scanPar.stagingMode = SCAN_STATIC;
    m_scanPar.maskStageTotalSteps = SCAN_STEPS_32;
    m_scanPar.nMaskStages = 32;
  } else {
    m_scanPar.stagingMode = (MaskStageMode)scn.getMaskStageMode();
    m_scanPar.maskStageTotalSteps = (MaskStageSteps)scn.getMaskStageTotalSteps();
    m_scanPar.nMaskStages = scn.getMaskStageSteps();
  }

  switch(m_scanPar.maskStageTotalSteps){
    case SCAN_STEPS_5:
      m_MaskStageTotalSteps=5;
      break;
    case SCAN_STEPS_32:
      m_MaskStageTotalSteps=32;
      break;
    case SCAN_STEPS_40:
      m_MaskStageTotalSteps=40;
      break;
    case SCAN_STEPS_64:
      m_MaskStageTotalSteps=64;
      break;
    case SCAN_STEPS_80:
      m_MaskStageTotalSteps=80;
      break;
    case SCAN_STEPS_160:
      m_MaskStageTotalSteps=160;
      break;
    case SCAN_STEPS_320:
      m_MaskStageTotalSteps=320;
      break;
    case SCAN_STEPS_2880:
      m_MaskStageTotalSteps=2880;
      break;
    default:
      m_MaskStageTotalSteps=32;
  }
  //scn.setMaskLoop(1);
  m_scanPar.maskLoop = 1;//scn.getMaskLoop();

  m_scanPar.FEbyFEMask = 65535;//(unsigned int) scn.getFEbyFEMask();

  m_scanPar.nLoops = 0;
  int paramCount = 0;
  for (int i=0; i<nLoopTot; i++) {
    if (scn.getLoopActive(i) && scn.getDspProcessing(i)) {
      m_scanPar.nLoops++;
      m_scanPar.scanLoop[i].scanParameter = (ScanParam)scn.getLoopParam(i);
      m_scanPar.scanLoop[i].nPoints = scn.getLoopVarNSteps(i);
      m_scanPar.scanLoop[i].dataPointsPtr = 0x0;
      m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_NO_ACTION;
      if (scn.getDspLoopAction(i)) {
        if (scn.getLoopAction(i) == PixScan::SCURVE_FIT) {
          //    m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_CALC_THRESH;
          m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_FIT;
          m_scanPar.scanLoop[i].endofLoopAction.fitFunction = SCAN_SCURVE;
        } else if (scn.getLoopAction(i) == PixScan::T0_SET) {
          m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_FIT;
          m_scanPar.scanLoop[i].endofLoopAction.fitFunction = SCAN_T0;
        } else if (scn.getLoopAction(i) == PixScan::TDAC_TUNING) {
          m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_TUNE_THRESH;
          m_scanPar.scanLoop[i].endofLoopAction.fitFunction = SCAN_SCURVE;
          m_scanPar.scanLoop[i].endofLoopAction.targetThreshold =  scn.getThresholdTargetValue();
        } else if (scn.getLoopAction(i) == PixScan::TOTCAL_FIT){
          m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_FIT;
          m_scanPar.scanLoop[i].endofLoopAction.fitFunction = SCAN_TOTCAL;
        }
      }
      if (m_scanPar.scanLoop[i].nPoints > 0) {
        //Add loop values at the very end
        std::vector<float> loopvalues = scn.getLoopVarValues(i);
        for(unsigned int k=0;k<m_scanPar.scanLoop[i].nPoints;k++,paramCount++){
          m_scanParInt[nTaskIn + nScanOptions + paramCount] = (int)loopvalues[k];
        }
      }

    } else {
      m_scanPar.scanLoop[i].scanParameter = SCAN_NONE; 
      m_scanPar.scanLoop[i].endofLoopAction.Action = SCAN_NO_ACTION;
    }
  }
  //m_scanPar.general.configSet = scn.getModConfig(); //?  
  m_scanPar.trigOpt.nEvents = scn.getRepetitions();
  m_scanPar.trigOpt.nL1AperEvent = scn.getConsecutiveLvl1TrigA(0);
  m_scanPar.trigOpt.Lvl1_Latency = scn.getLVL1Latency();
  m_scanPar.trigOpt.strobeDuration = scn.getStrobeDuration();
  m_scanPar.trigOpt.strobeMCCDelay = scn.getStrobeMCCDelay();    
  m_scanPar.trigOpt.strobeMCCDelayRange = scn.getStrobeMCCDelayRange();    
  if (scn.getStrobeLVL1DelayOverride()) {
    m_scanPar.trigOpt.CalL1ADelay = scn.getStrobeLVL1Delay();
  } else {
    m_scanPar.trigOpt.CalL1ADelay = ((ConfInt&)m_modGroup.config()["general"]["TriggerDelay"]).getValue();
  }
  m_scanPar.trigOpt.eventInterval = 4000; //??
  m_scanPar.trigOpt.superGroupDelay = scn.getSuperGroupTrigDelay();
  m_scanPar.trigOpt.superGroupAnL1A[0] = scn.getConsecutiveLvl1TrigA(0);
  m_scanPar.trigOpt.superGroupAnL1A[1] = scn.getConsecutiveLvl1TrigA(1);
  m_scanPar.trigOpt.superGroupBnL1A[0] = scn.getConsecutiveLvl1TrigB(0);
  m_scanPar.trigOpt.superGroupBnL1A[1] = scn.getConsecutiveLvl1TrigB(1);
  m_scanPar.trigOpt.trigABDelay[0] = scn.getTrigABDelay(0);
  m_scanPar.trigOpt.trigABDelay[1] = scn.getTrigABDelay(1);
  m_scanPar.trigOpt.vcal_charge = scn.getFeVCal();
  //scn.getModScanConcurrent() //?
  //scn.getFeHitbus() //?
  if (scn.getDigitalInjection()) m_scanPar.trigOpt.optionsMask += (1 << SCAN_DIGITAL_INJECT);
  else if (scn.getChargeInjCap()!=0) {
    m_scanPar.trigOpt.optionsMask += (1 << SCAN_USE_CHIGH);             
  } else {
    m_scanPar.trigOpt.optionsMask += (1 << SCAN_USE_CLOW);             
  }

  if (scn.getSelfTrigger()) {
    m_scanPar.trigOpt.triggerMode = SCAN_INTERNAL_SELF;
  } else {
    // More options needed here in PixScan
    m_scanPar.trigOpt.triggerMode = SCAN_DSP;
  }
  for (i=0; i<MAX_GROUPS; i++) {
    if (scn.getConfigEnabled(i)) m_scanPar.groupOpt.groupActionMask[i] |= (1 << SCAN_CONFIGURE_ALL) ;
    if (scn.getTriggerEnabled(i)) m_scanPar.groupOpt.groupActionMask[i] |= (1 << SCAN_TRIGGER) ;
    if (scn.getStrobeEnabled(i)) m_scanPar.groupOpt.groupActionMask[i] |= (1 << SCAN_STROBE) ;
    if (scn.getReadoutEnabled(i)) m_scanPar.groupOpt.groupActionMask[i] |= (1 << SCAN_READOUT) ;
  }  
  //Front-end level option
  m_scanPar.feOpt.phi = 3;//scn.getColumnROFreq()-1;
  if(m_scanPar.feOpt.phi<0 || m_scanPar.feOpt.phi >3){ 
    std::cout << getCtrlName() << " invalid m_scanPar.feOpt.phi (ColumnROFreq) =" << m_scanPar.feOpt.phi << std::endl;
    m_scanPar.feOpt.phi = 0;
  }
  m_scanPar.feOpt.totThresholdMode = 0; //scn.getTotThrMode();
  m_scanPar.feOpt.totMinimum = 0;//scn.getTotMin();
  m_scanPar.feOpt.totTwalk = 32;//scn.getTotDHThr();
  m_scanPar.feOpt.totLeMode = 0;//scn.getTotThrMode();
  m_scanPar.feOpt.hitbus = 0;//scn.getFeHitbus();

  //AKDEBUG hack to be fixed proper
  if(scn.getHistogramFilled(PixScan::OCCUPANCY))m_scanPar.histOpt[SCAN_OCCUPANCY].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOT_MEAN)) m_scanPar.histOpt[SCAN_TOT_MEAN].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOT_SIGMA)) m_scanPar.histOpt[SCAN_TOT_SIGMA].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::LVL1))m_scanPar.histOpt[SCAN_L1ID].optionMask = 1;
  /*
  if(scn.getHistogramFilled(PixScan::BCID_MEAN)) m_scanPar.histOpt[SCAN_BCID_MEAN].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::BCID_SIGMA)) m_scanPar.histOpt[SCAN_BCID_SIGMA].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::DSP_ERRORS))m_scanPar.histOpt[SCAN_ERROR].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::SCURVE_MEAN)) m_scanPar.histOpt[SCAN_SCURVE_MEAN].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::SCURVE_SIGMA)) m_scanPar.histOpt[SCAN_SCURVE_SIGMA].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::SCURVE_CHI2)) m_scanPar.histOpt[SCAN_SCURVE_CHI2].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_TOT_P0)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_TOT_P0].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_TOT_P1)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_TOT_P1].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_TOT_P2)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_TOT_P2].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_TOT_CHI2)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_TOT_CHI2].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_DISP_P0)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_DISP_P0].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_DISP_P1)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_DISP_P1].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_PIX_DISP_CHI2)) m_scanPar.histOpt[SCAN_TOTCAL_PIX_DISP_CHI2].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_TOT_P0)) m_scanPar.histOpt[SCAN_TOTCAL_FE_TOT_P0].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_TOT_P1)) m_scanPar.histOpt[SCAN_TOTCAL_FE_TOT_P1].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_TOT_P2)) m_scanPar.histOpt[SCAN_TOTCAL_FE_TOT_P2].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_TOT_CHI2)) m_scanPar.histOpt[SCAN_TOTCAL_FE_TOT_CHI2].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_DISP_P0)) m_scanPar.histOpt[SCAN_TOTCAL_FE_DISP_P0].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_DISP_P1)) m_scanPar.histOpt[SCAN_TOTCAL_FE_DISP_P1].optionMask = 1;
  if(scn.getHistogramFilled(PixScan::TOTCAL_FE_DISP_CHI2)) m_scanPar.histOpt[SCAN_TOTCAL_FE_DISP_CHI2].optionMask = 1;
  */  
  //enum BinSize m_scanPar.histOpt[h].binSize;

  unsigned int sum = 0;
  for (i=0; i<MAX_GROUPS; i++) sum += scn.getModuleMask(i);

  if (sum != 0) {
    unsigned int activeModMask = 0;
    for (int mod=0; mod<32; mod++) {
      // Check if module is defined
      if (m_modPosition[mod] >=0) {
        // Find the group containing the module
        bool notInGrp = true;
        for (int grp=0; grp<MAX_GROUPS; grp++) {
          if (scn.getModuleMask(grp) & (0x1<<mod)) {
            notInGrp = false;
            setGroupId(mod, grp);
            if (scn.getReadoutEnabled(grp)) {
              enableModuleReadout(mod);
              activeModMask |= (0x1 << mod);
            } else {
              disableModuleReadout(mod, false);
            }
          }
        }
        if (notInGrp) {
          setGroupId(mod, 4);
          disableModuleReadout(mod, false);
        }
      }
    }
    for (int grp=0; grp<MAX_GROUPS; grp++) {
      if (scn.getReadoutEnabled(grp)) {
        m_dspModMask[grp] = scn.getModuleMask(grp) & activeModMask;
      } else {
        m_dspModMask[grp] = 0x0;
      }    
    }
  } else {
    // Compute the SDSP module masks
    for (i=0; i<MAX_GROUPS; i++) m_dspModMask[i] = 0x0;
    for (i=0; i<m_nMod; i++) {
      int group = m_modGrpDef[i];
      std::cout << "m_modGrpDef[" << i << "] " << m_modGrpDef[i] << std::endl; 
      setGroupId(i, group);      
      int modNum = m_modNum[i];
      if (m_modActive[i] && group<MAX_GROUPS) {
        m_dspModMask[group] |= (0x1<<modNum);
      }
    }
  }
  for (i=0; i<MAX_GROUPS; i++) {
    int gnModules = 0;
    if (m_dspModMask[i] != 0) {
      for (j=0; j<32; j++) {
        if (m_dspModMask[i] & (0x1<<j)) gnModules++;
      }
      SetupGroupIn *setupGroupIn = (SetupGroupIn *)new UINT32[(sizeof(SetupGroupIn))/sizeof(UINT32) + gnModules + 1];
      setupGroupIn->groupId  = i;
      setupGroupIn->nModules = gnModules;
      setupGroupIn->nSlaves  = 1;
      setupGroupIn->dataPtr  = 0;
      SetupGroupInAux *setupGroupInAux = (SetupGroupInAux *)&setupGroupIn[1];
      // add modules
      int count = 0;
      for (j=0; j<32; j++) {
        if (m_dspModMask[i] & (0x1<<j)) setupGroupInAux[count++].id = j;
      }
      // add slaves 0
      setupGroupInAux[count++].id = i;

      RodPrimitive* confGroup;
      confGroup = new RodPrimitive(sizeof(SetupGroupIn)/sizeof(UINT32) + gnModules + 1, 
          0, SETUP_GROUP, 0, (long*)setupGroupIn);
      std::cout << "executeMasterPrimitiveSync writeScanConfig" << std::endl;
      executeMasterPrimitiveSync(*confGroup);
      delete confGroup;
      delete[] setupGroupIn;
    }
    else{
      m_scanPar.groupOpt.groupActionMask[i]=0x0;
    }
  }
  // Prepare the config primitive body
  startTaskIn.id = SCAN_TASK_2;
  //AKDEBUG  startTaskIn.idMinor = DIGITAL_INJECT_THRESHOLD_SCAN;
  startTaskIn.where   = DSP_THIS;
  startTaskIn.dataPtr = 0;

  // Store the total number of bins
  if (m_scanPar.scanLoop[2].nPoints >0) {
    m_lsNbin = m_scanPar.scanLoop[0].nPoints*m_scanPar.scanLoop[1].nPoints*m_scanPar.scanLoop[2].nPoints;
  } else if (m_scanPar.scanLoop[1].nPoints > 0) {
    m_lsNbin = m_scanPar.scanLoop[0].nPoints*m_scanPar.scanLoop[1].nPoints;
  } else {
    m_lsNbin = m_scanPar.scanLoop[0].nPoints;
  }
}

void RodPixController::startScan() {                       //! Write scan parameters
  //SLink off
  writeRegister(RTR_CMND_STAT, 3, 1, 0x1);
  writeRegister(RTR_CMND_STAT, 7, 1, 0x1);
  //SLink on
  //writeRegister(0x402500, 3, 1, 0x0);
  //writeRegister(0x402500, 7, 1, 0x0);

  if (m_scanParInt != NULL) {
    RodPrimitive *startScan = new RodPrimitive((sizeof(StartTaskIn)+sizeof(ScanOptions))/sizeof(unsigned int) +  m_nBinTot, 0, START_TASK,  0, (long*)m_scanParInt);
    executeMasterPrimitiveSync(*startScan);

    StartTaskIn *startTaskIn = (StartTaskIn*) m_scanParInt;
    // get locations for scan status structures
    TalkTaskIn TalkTask_In;
    TalkTask_In.task  = startTaskIn->id;
    TalkTask_In.topic = SCAN_QUERY_STATUS;
    TalkTask_In.item  = 0;

    RodPrimitive *htalktaskprim = new RodPrimitive((sizeof(TalkTaskIn))/sizeof(unsigned int), 0, TALK_TASK, 
        0, (long*)&TalkTask_In);

    RodOutList* outList;
    executeMasterPrimitiveSync(*htalktaskprim, outList);

    unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);
    UINT32 location = 0;

    if((sizeof(TalkTaskOut)/4) < (unsigned long)outList->getMsgLength(1)) {
      location = outBody[sizeof(TalkTaskOut)/4];      
    }
    m_rod->deleteOutList();

    if(location!=0)m_rod->mdspBlockRead(location, 
        (long unsigned int *)&m_dspScanStatus, 
        sizeof(m_dspScanStatus)/4);

    delete startScan;
    m_scanActive = true;
    m_runActive = false;
    m_fitDone = false;

  }
}


bool RodPixController::getErrorHistos(unsigned int dsp, Histo* &his) {
  unsigned int xmod = 0;

  if(dsp<0 || dsp>3) return false;

  SendSlavePrimitiveIn *SendSlavePrimitive_In = (SendSlavePrimitiveIn *) new UINT32[(sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4];
  SendSlavePrimitive_In->slave = dsp;
  SendSlavePrimitive_In->primitiveId = SLAVE_TALK_TASK;
  SendSlavePrimitive_In->nWords = sizeof(TalkTaskIn)/4;
  SendSlavePrimitive_In->fetchReply = 1;

  TalkTaskIn *SlaveTalkTask_In = (TalkTaskIn *)&SendSlavePrimitive_In[1];
  SlaveTalkTask_In->task = HISTOGRAM_TASK;
  SlaveTalkTask_In->topic = HIST_QUERY_LAYOUT;
  SlaveTalkTask_In->item = 0;

  RodPrimitive *hquery_hist_layout = new RodPrimitive((sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4,0, SEND_SLAVE_PRIMITIVE, 0, (long int*)SendSlavePrimitive_In);
  RodOutList * outList;
  std::cout << "executeMasterPrimitiveSync getErrorHistos" << std::endl;
  executeMasterPrimitiveSync(*hquery_hist_layout, outList);

  unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);

  std::vector<unsigned int> outs;
  for(int i = 0; i < outList->getMsgLength(1); ++i) {
    outs.push_back(outBody[i]);
  }
  m_rod->deleteOutList();
  delete hquery_hist_layout;

  HistogramLayout *hinfo;
  //AKDEBUG FIX BELOW
  int infooffset = (sizeof(TalkTaskOut)/4);

  if(outs.size() >= (infooffset+sizeof(HistogramLayout)/4)) {
    hinfo = (HistogramLayout *)&(outs[infooffset]);
  } else {
    std::cout << "Corrupted reply from HistogramLayout, expected "
      << (infooffset+sizeof(HistogramLayout)/4)
      << "got " << outs.size() << " Words ... " << std::endl;
    std::cerr << "RodPixController::getErrorHisto"
      << " No histograms for slot " << m_ctrlID << std::endl;
    return false;
  }

  int base, modoffset, size;
  base = (int)hinfo->histMemInfo[SCAN_ERROR].base;
  modoffset = 4*(int)hinfo->histMemInfo[SCAN_ERROR].moduleOffset;
  size = hinfo->histMemInfo[SCAN_ERROR].size;

  //int histsize = 40;
  std::vector<unsigned int> out;
  std::vector<unsigned int> out_bad;
  std::vector<unsigned int> out_rod;
  std::ostringstream nam, tit;
  nam << "dsp_errors_" << dsp;
  tit << "DSP # " << dsp << " Errors";

  // map PixLib module into DSP module
  /*
  if (mod >= 32) return;
  for (im=0; im<32; im++ ) {
    if ((m_dspModMask[dsp]&(0x1<<im))) {
      if (im == mod) xmod = nmod;
      nmod++;
    }
  }
  if (nmod == 0) return;
  */
  // Bad module = 7
  readSlaveMem(dsp, (int)base+xmod*modoffset, size, out);
  readSlaveMem(dsp, (int)base+26*modoffset, size, out_bad);
  readSlaveMem(dsp, (int)base+(26+1)*modoffset, size, out_rod);

  ErrorInfo *errorInfo, *errorInfo_bad, *errorInfo_rod;
  errorInfo = (ErrorInfo *)&out[0];
  errorInfo_bad = (ErrorInfo *)&out_bad[0];
  errorInfo_rod = (ErrorInfo *)&out_rod[0];

  /*
  Histo *h = new Histo(nam.str(), tit.str(), histsize, -0.5, histsize-0.5, 2, -0.5, 1.5);

  // fill module level histograms
  for(int i = 0; i < ERRORTYPE_M_LAST; ++i) {
    h->set(i,0,errorInfo->type[i]);
  }
  for(unsigned int i = 0; i < 16; i++) {
    h->set(i,1, ( (errorInfo->fe>>i)&0x1 ) );
  }
  moduleHist = h;
  */

  // fill dsp level histograms
  for(int i = 0; i < ERRORTYPE_D_LAST; ++i) {
    his->set(i,dsp,errorInfo_rod->type[i]);
  }
  for(int i = 0; i < ERRORTYPE_M_LAST; ++i) {
    his->set(i,dsp+4,errorInfo_bad->type[i]);
  }
  return true;
}

void RodPixController::getFitResults(HistoType type, unsigned int mod, unsigned int slv, 
std::vector< Histo * > &thr, std::vector< Histo * > &noise, std::vector< Histo * > &chi2){
  unsigned int im, nmod = 0, xmod = 0, nmoddsp = 0;

  std::cout << " getFitResult mod " << mod << " slv " << slv << std::endl;

  SendSlavePrimitiveIn *SendSlavePrimitive_In = (SendSlavePrimitiveIn *)new UINT32[(sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4];
  SendSlavePrimitive_In->slave = slv;
  SendSlavePrimitive_In->primitiveId = SLAVE_TALK_TASK;
  SendSlavePrimitive_In->nWords = sizeof(TalkTaskIn)/4;
  SendSlavePrimitive_In->fetchReply = 1;

  TalkTaskIn *SlaveTalkTask_In = (TalkTaskIn *)&SendSlavePrimitive_In[1];
  SlaveTalkTask_In->task  = HISTOGRAM_TASK;
  SlaveTalkTask_In->topic = HIST_QUERY_LAYOUT;
  SlaveTalkTask_In->item  = 0;

  RodPrimitive *hquery_hist_layout = new RodPrimitive((sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4, 0, 
      SEND_SLAVE_PRIMITIVE, 0, 
      (long int*)SendSlavePrimitive_In);

  RodOutList* outList;
  std::cout << "executeMasterPrimitiveSync getFitResults" << std::endl;  
  executeMasterPrimitiveSync(*hquery_hist_layout, outList);

  unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);

  std::vector<unsigned int> outs;  
  for (int i=0; i<outList->getMsgLength(1); i++) {
    outs.push_back(outBody[i]);
  }
  m_rod->deleteOutList();
  delete hquery_hist_layout;

  HistogramLayout *hinfo;
  //AKDEBUG FIX BELOW
  int infooffset = (sizeof(TalkTaskOut)/4);
  if(outs.size()>=(infooffset + sizeof(HistogramLayout)/4)){
    hinfo = (HistogramLayout *)&(outs[infooffset]);
  }
  else{
    std::cout << " corrupted reply from HistogramLayout, expected " << (infooffset + sizeof(HistogramLayout)/4)
      << " gotten " << outs.size() << " words ..." << std::endl;
  }
  if (mod >= 32) return;
  for (im=0; im<32; im++ ) {
    if((hinfo->validModules)&(0x1<<im))nmoddsp++;
    if ((m_dspModMask[slv]&(0x1<<im))) {
      if (im == mod) xmod = nmod;
      nmod++;
    }
  }
  if (nmod == 0) return;

  std::ostringstream mnum;
  std::string nam, tit;
  switch (type) {
    case SCURVES:
      while (thr.size() < 32) thr.push_back((Histo *)NULL);
      while (noise.size() < 32) noise.push_back((Histo *)NULL);
      while (chi2.size() < 32) chi2.push_back((Histo *)NULL);
      mnum << mod;
      if (thr[mod] == NULL) {
        nam = "Thr_" + mnum.str();
        tit = "Threshold mod " + mnum.str();
        thr[mod] = new Histo(nam, tit, 144, -0.5, 143.5, 320, -0.5, 319.5);
      }
      if (noise[mod] == NULL) {
        nam = "Noise_" + mnum.str();
        tit = "Noise mod " + mnum.str();
        noise[mod] = new Histo(nam, tit, 144, -0.5, 143.5, 320, -0.5, 319.5);
      }
      if (chi2[mod] == NULL) {
        nam = "Chi2_" + mnum.str();
        tit = "Chi2 mod " + mnum.str();
        chi2[mod] = new Histo(nam, tit, 144, -0.5, 143.5, 320, -0.5, 319.5);
      }

      int bins, base, modoffset, binoffset, size;
      bins       = hinfo->histMemInfo[SCAN_FITRESULTS].nBins;
      base       = (int)hinfo->histMemInfo[SCAN_FITRESULTS].base;
      modoffset  = 4*hinfo->histMemInfo[SCAN_FITRESULTS].moduleOffset;
      binoffset  = 4*hinfo->histMemInfo[SCAN_FITRESULTS].binOffset;
      size       = hinfo->histMemInfo[SCAN_FITRESULTS].size;

      std::cout << "Fits DECODED: " 
        << " valid modules on SDSP: " << std::hex << hinfo->validModules
        << " # of bins: " << std::hex << bins
        << " base: "      << std::hex << base
        << " boffset: "   << std::hex << binoffset
        << " moffset: "   << std::hex << modoffset
        << " size: "   << std::hex << size
        <<  std::dec<< std::endl; 


      if ((m_dspModMask[slv]&(0x1<<mod)) != 0) {
        std::cout << "Reading Fit for mod " << mod << " on slave " 
          << slv << " debug version 2" << std::endl;

        if(m_MaskStageTotalSteps<=0) {
          std::cout <<"Error "<< getCtrlName()<<" invalid m_MaskStageTotalSteps=" << m_MaskStageTotalSteps <<std::endl;
          return;
        }

        int PIX_PER_CHIP_PER_MASKSTAGE = 2880/m_MaskStageTotalSteps;
        int HFES=16;
        FitOut *out;
        std::vector<unsigned int> out1;
        FLOAT32 *chisq, *mu, *sigma;
        for (unsigned int i=0; i<hinfo->histMemInfo[SCAN_FITRESULTS].nBins; i++) {
          out1.clear();
          //readSlaveMemDirect(slv, (int)hinfo->histMemInfo[HIST_FIT].base+
          //i*4*hinfo->histMemInfo[HIST_FIT].binOffset+
          //xmod*4*hinfo->histMemInfo[HIST_FIT].moduleOffset,
          //                   0xB400*sizeof(PixelFit)/4, out1);
          readSlaveMem(slv,(int)base + i*binoffset + xmod*modoffset, size, out1);
          std::cout << "Done reading slave mem" << std::endl;
          out = (FitOut *)&(out1[0]);

          for (int is=0; is<m_MaskStageTotalSteps; is++) {//mask stage
            for (int fe=0; fe<HFES; fe++) {//fe
              for (int ir=0; ir<PIX_PER_CHIP_PER_MASKSTAGE; ir++) {//pix per mask stage
                int ipix=ir*m_MaskStageTotalSteps+is;
                int col=ipix/160;
                int row=ipix%160;
                if(col%2)row=159-row;
                int xc,xr;
                if (fe < 8) {
                  xc = 18*fe + col;
                  xr = row;
                } else {
                  xc = 143 - 18*(fe-8) - col;
                  xr = 319-row;
                }
                int idx=(is*HFES+fe)*PIX_PER_CHIP_PER_MASKSTAGE+ir;
                //AKDEBUG get right parameters, BEWARE of conversions UNIT32<->FLOAT
                mu    = (FLOAT32*) &(out[idx].coeffs[1]);
                sigma = (FLOAT32*) &(out[idx].coeffs[2]);
                chisq = (FLOAT32*) &(out[idx].chi2);

                thr[mod]  ->set(xc,xr,(double) (*mu));
                noise[mod]->set(xc,xr,(double) (*sigma));
                chi2[mod] ->set(xc,xr,(double) (*chisq));
              }//pix per mask stage
            }//fe
          }//mask stage
        }//fit results nBins
      }
      break;
    default:
      break;
  };
}

void RodPixController::getHisto(HistoType type, unsigned int mod, unsigned int slv, std::vector< std::vector<Histo*> >& his) {  //! Read a histogram

  unsigned int im, nmod = 0, xmod = 0, nmoddsp = 0;

  while (his.size() < 32) {
    std::vector< Histo * > vh;
    vh.clear();
    his.push_back(vh);
  }

  std::cout << " getHistoResult mod " << mod << " slv " << slv << std::endl;
  if( type ==MON_LEAK ){//In this case, the histo is on the Master
    his[mod].clear();
    int modoffset=46080*2;//2 chars per pixel
    unsigned long *buf = new unsigned long[modoffset/4];
    long unsigned int strAddr=MAST_FREEMEMORY_BASE+ modoffset*mod;
    std::cout<<"strAddr: " << strAddr <<std::endl;
    m_rod->mdspBlockRead(strAddr, buf, modoffset/4); 
    std::ostringstream nam, tit;    
    nam << "MonLeak_Scan" <<  mod    << "_" << 0;//bin==0
    tit << "I(*125 pA), mod " << mod;
    Histo *h = new Histo(nam.str(), tit.str(),  2, 144, -0.5, 143.5, 320, -0.5, 319.5);
    UINT16 *out16  = (UINT16 *)buf;
    int Icol,Irow,ipix,ic,icol,irow;
    for(int i=0;i<46080;i++){
      ipix=i;
      icol=(ipix/160)%18;
      irow=ipix%160;
      ic=(ipix/(160*18))%16;
      if(ic<8){
        Icol=ic*18+icol;
        Irow=irow;
      }
      else {
        Icol=(16-ic)*18-icol-1;
        Irow=319-irow;
      }
      //     float v =log(out16[i]+1);
      float v = out16[i];
      if(v>0x4ff)v=0x4ff;//0x4ff==error
      if(v==0x402)v=0x5ff;//expert kind of error (temporary)
      h->set(Icol, Irow,v);
      // if(i<100||i%100==0)std::cout<<"i: " << i <<" " << ic <<" " << Irow <<" " << Icol <<" " << v <<" " <<std::endl;
    }
    his[mod].push_back(h); 
    delete buf;
    return;
  }

  SendSlavePrimitiveIn *SendSlavePrimitive_In = (SendSlavePrimitiveIn *)new UINT32[(sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4];
  SendSlavePrimitive_In->slave = slv;
  SendSlavePrimitive_In->primitiveId = SLAVE_TALK_TASK;
  SendSlavePrimitive_In->nWords = sizeof(TalkTaskIn)/4;
  SendSlavePrimitive_In->fetchReply = 1;

  TalkTaskIn *SlaveTalkTask_In = (TalkTaskIn *)&SendSlavePrimitive_In[1];
  SlaveTalkTask_In->task  = HISTOGRAM_TASK;
  SlaveTalkTask_In->topic = HIST_QUERY_LAYOUT;
  SlaveTalkTask_In->item  = 0;

  RodPrimitive *hquery_hist_layout = new RodPrimitive((sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4, 0, 
      SEND_SLAVE_PRIMITIVE, 0, 
      (long int*)SendSlavePrimitive_In);

  RodOutList* outList;
  std::cout << "executeMasterPrimitiveSync getHisto" << std::endl;
  executeMasterPrimitiveSync(*hquery_hist_layout, outList);

  unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);

  std::vector<unsigned int> outs;  
  for (int i=0; i<outList->getMsgLength(1); i++) {
    outs.push_back(outBody[i]);
  }
  m_rod->deleteOutList();
  delete hquery_hist_layout;

  HistogramLayout *hinfo;
  //AKDEBUG FIX BELOW
  int infooffset = (sizeof(TalkTaskOut)/4);

  if(outs.size()>= (infooffset + sizeof(HistogramLayout)/4)){
    hinfo = (HistogramLayout *)&(outs[infooffset]);
  }else{
    std::cout << " corrupted reply from HistogramLayout, expected " << (infooffset + sizeof(HistogramLayout)/4)
      << " gotten " << outs.size() << " Words ... " << std::endl;
    std::cerr << "RodPixController::getHisto"
      << " No histograms for module" << mod 
      << " on slot " << m_ctrlID 
      << std::endl;
    his.resize(32);
    return;
  }

  if (mod >= 32)  {
    std::cout << "ATTENTION: RodPixController::getHisto: received module "<<mod<<std::endl;
    return;
  }
  for (im=0; im<32; im++ ) {
    if((hinfo->validModules)&(0x1<<im))nmoddsp++;
    if ((m_dspModMask[slv]&(0x1<<im))) {
      if (im == mod) xmod = nmod;
      nmod++;
    }
  }
  if (nmod == 0) {
    std::cout << "ATTENTION: RodPixController::getHisto: # of validated modules 0 "<<std::endl;
    return;
  }

  std::string scanname, scanname1;
  int bins, base, modoffset, binoffset, size;
  int htype;
  switch (type) {
    case OCCUPANCY :
      scanname   = "Occup_";
      scanname1  = "Occupancy";
      htype      = SCAN_OCCUPANCY;
      break;
    case HITOCC:
      scanname   = "HitOcc_";
      scanname1  = "HitOccupancy";
      htype      = SCAN_HITOCC;
      break;
    case TOTAVERAGE:
      scanname   = "ToTAvg_";
      scanname1  = "ToTAverage";
      htype      = SCAN_TOTAVERAGE;
      break;
    case TOT_MEAN :
      scanname   = "ToTMean_";
      scanname1  = "ToTMean";
      htype      = SCAN_TOT_MEAN;
      break;
    case TOT_SIGMA :
      scanname   = "ToTSigma_";
      scanname1  = "ToTSigma";
      htype      = SCAN_TOT_SIGMA;
      break;
    case SCURVE_MEAN :
      scanname   = "Thr_";
      scanname1  = "Thr";
      htype      = SCAN_SCURVE_MEAN;
      break;
    case SCURVE_SIGMA :
      scanname   = "Noise_";
      scanname1  = "Noise";
      htype      = SCAN_SCURVE_SIGMA;
      break;
    case SCURVE_CHI2 :
      scanname   = "Chi2_";
      scanname1  = "Chi2";
      htype      = SCAN_SCURVE_CHI2;
      break;
    case LVL1 :
      scanname   = "LVL1_";
      scanname1  = "LVL1";
      htype      = SCAN_L1ID;
      break; 
    case RAW_DATA_DIFF_2 :
      htype      = 0;
      break;
    case RAW_DATA_REF :
      scanname = "RAW_DATA_REF_";
      scanname1  = "time_dependent error count vs Threshold";
      htype      = 0;
      break;
    default: 
      break;
  };

  bins       = hinfo->histMemInfo[htype].nBins;
  base       = (int)hinfo->histMemInfo[htype].base;
  modoffset  = 4*hinfo->histMemInfo[htype].moduleOffset;
  binoffset  = 4*hinfo->histMemInfo[htype].binOffset;
  size       = hinfo->histMemInfo[htype].size;

  std::cout << "Histograms DECODED: " 
    << scanname1
    << " valid modules on SDSP: " << nmoddsp
    << " module mask on SDSP: "  << std::hex << hinfo->validModules
    << " # of bins: " << std::hex << bins
    << " base: "      << std::hex << base
    << " boffset: "   << std::hex << binoffset
    << " moffset: "   << std::hex << modoffset
    << " size in words:    "  << std::hex << size
    <<  std::dec<< std::endl; 

  if ((m_dspModMask[slv]&(0x1<<mod)) != 0) {

    std::cout << "Reading histo for mod " << mod << " on slave " << slv << std::endl;
    his[mod].clear();
    //    unsigned short int *out;
    //AKDEBUG fix propper, needs to be configurable, 8bit default sensible
    char *out;
    FLOAT32 *outf;
    std::vector<unsigned int> out1;
    for (int i=0; i<bins; i++) {
      out1.clear();
      readSlaveMem(slv,(int)base + i*binoffset + xmod*modoffset, size, out1);
      //      out  = (unsigned short *)&(out1[0]);
      out  = (char *)&(out1[0]);
      outf = (FLOAT32 *)&(out1[0]);
      std::ostringstream nam, tit;
      HISTO_PARAM *hist_param=(HISTO_PARAM *)&hinfo->histMemInfo[1];//used for Boc_scan
      if(type==RAW_DATA_DIFF_2){
        int ilnk=hist_param->itmp;
        nam << "RAW_2_" << mod << "-Link" << ilnk << "_" << hist_param->loop2_step_id;
        tit << "Raw data 2 mod " << mod << "-Link" << ilnk << " L2=" << hist_param->loop2_step_id;
      }else{
        nam << scanname  <<  mod    << "_" << i;
        tit << scanname1 << " mod " << mod << " bin " << i;
      }
      Histo *h;
      if (type == OCCUPANCY) {
        h = new Histo(nam.str(), tit.str(), 1, 144, -0.5, 143.5, 
            320, -0.5, 319.5);
      }else if(type == HITOCC || type == TOTAVERAGE) {
        h = new Histo(nam.str(), tit.str(), 46080, 0, 46079);
      }else if(type == RAW_DATA_DIFF_2){
        h = new Histo(nam.str(), tit.str(), hist_param->loop0_nsteps, -0.5, hist_param->loop0_nsteps-0.5, 
            hist_param->loop1_nsteps, -0.5, hist_param->loop1_nsteps-0.5);
      }else if(type == RAW_DATA_REF){
        h = new Histo(nam.str(), tit.str(), 2,hist_param->loop0_nsteps, -0.5, hist_param->loop0_nsteps-0.5, 
            hist_param->loop1_nsteps, -0.5, hist_param->loop1_nsteps-0.5);
      } else {
        h = new Histo(nam.str(), tit.str(), 4, 144, -0.5, 143.5, 
            320, -0.5, 319.5);
      }
      if(type==RAW_DATA_DIFF_2||type==RAW_DATA_REF){
        UINT16 *out16  = (UINT16 *)&(out1[0]);
        if(hist_param->loop0_nsteps<=0){
          std::cout << "Wrong histogram parameter hist_param->loop0_nsteps: "<<hist_param->loop0_nsteps<<" => 5"<<std::endl;
          hist_param->loop0_nsteps=5;
        }    
        for(int i=0;i<size*2;i++){
          int xc=hist_param->loop1_nsteps-1-i/hist_param->loop0_nsteps;//threshold is decremented
          int xr=i%hist_param->loop0_nsteps;
          if(xc>=0){
            h->set(xr,xc, out16[i]);
          }
        }
      }else{
        if(m_MaskStageTotalSteps<=0) {
          std::cout <<"Error "<< getCtrlName()<<" invalid m_MaskStageTotalSteps=" << m_MaskStageTotalSteps <<std::endl;
          return;
        }
        int PIX_PER_CHIP_PER_MASKSTAGE = 2880/m_MaskStageTotalSteps;
        int HFES=16;
        for (int is=0; is<m_MaskStageTotalSteps; is++) {//mask stage
          for (int fe=0; fe<HFES; fe++) {//fe
            for (int ir=0; ir<PIX_PER_CHIP_PER_MASKSTAGE; ir++) {//pix per mask stage
              int ipix=ir*m_MaskStageTotalSteps+is;
              int col=ipix/160;
              int row=ipix%160;
              if(col%2)row=159-row;
              int xc,xr;
              if (fe < 8) {
                xc = 18*fe + col;
                xr = row;
              } else {
                xc = 143 - 18*(fe-8) - col;
                xr = 319-row;
              }
              int idx=(is*HFES+fe)*PIX_PER_CHIP_PER_MASKSTAGE+ir;
              switch (type) {
                case OCCUPANCY :
                  // case LVL1 : 
                  //AK take into account outofmaskstage and bad histo areas
                  h->set(xc,xr, out[idx]);
                  break;
                case HITOCC :
                case TOTAVERAGE :
                  //h->set(is*HFES*HCOLS*HROWS+fe*HCOLS*HROWS+ic*HROWS+ir, out[is*HFES*HCOLS*HROWS+fe*HCOLS*HROWS+ic*HROWS+ir]);
                  break;
                case TOT_MEAN :
                case TOT_SIGMA :
                  h->set(xc,xr, outf[idx]);
                  break;
                case SCURVE_MEAN :
                case SCURVE_SIGMA :
                case SCURVE_CHI2 :
                  h->set(xc,xr, outf[idx]);
                  break;
                default:
                  break;
              };
            }
          }
        }
      }
      his[mod].push_back(h);
    }
  }
}

void RodPixController::writeRunConfig(PixRunConfig &run) { //! Get the run configuration parameters
}

void RodPixController::startRun(int ntrig) {     //! Start a run
}

void RodPixController::stopRun() {               //! Terminates a run
}

int RodPixController::runStatus() {              //! Check the status of the run
  if (m_scanActive) {
    unsigned int sr = m_rod->mdspSingleRead(m_dspScanStatus.progress);
    return sr;

  } else if (m_runActive) {
    return 0;
  }
  return 0;
}

int RodPixController::nTrigger() {               //! Returns the number of trigger processed so far
  if (m_scanActive) {
    int maskStage;
    int loop0Parameter;
    int loop1Parameter;
    int loop2Parameter;
    int nmaskStage;
    int nLoop0Parameters;
    int nLoop1Parameters;
    int nLoop2Parameters;
    loop0Parameter = m_rod->mdspSingleRead(m_dspScanStatus.loop0Parameter)-1;
    loop1Parameter = m_rod->mdspSingleRead(m_dspScanStatus.loop1Parameter)-1;
    loop2Parameter = m_rod->mdspSingleRead(m_dspScanStatus.loop2Parameter)-1;
    maskStage      = m_rod->mdspSingleRead(m_dspScanStatus.maskStage)-1;
    nLoop0Parameters = m_rod->mdspSingleRead(m_dspScanStatus.nLoop0Parameters);
    nLoop1Parameters = m_rod->mdspSingleRead(m_dspScanStatus.nLoop1Parameters);
    nLoop2Parameters = m_rod->mdspSingleRead(m_dspScanStatus.nLoop2Parameters);
    nmaskStage      = m_rod->mdspSingleRead(m_dspScanStatus.nMaskStages);
    unsigned int progress       = m_rod->mdspSingleRead(m_dspScanStatus.progress);
    //unsigned int state          = m_rod->mdspSingleRead(m_dspScanStatus.state);

    for(int i=SCAN_NUM_STATES; i>=0 ;i--){
      if( progress & (0x1<<i)) std::cout << 1;
      else                     std::cout << 0;
    }

    if(maskStage     <0) maskStage=0;
    if(loop0Parameter<0) loop0Parameter=0;
    if(loop1Parameter<0) loop1Parameter=0;
    if(loop2Parameter<0) loop2Parameter=0;
    return (((loop0Parameter) & 0xff) + (((maskStage) & 0xff) << 8) + (((loop1Parameter) & 0xff) << 16) + (((loop2Parameter) & 0xff) << 24));
    //return 0;

  } else if (m_runActive) {
    return 0;
  }
  return 0;
}

void RodPixController::downloadFifo(int size) {
  m_fifoA.clear();
  m_fifoB.clear();
  readFifo(FIFO_INMEMA, 0, size, m_fifoA);
  readFifo(FIFO_INMEMB, 0, size, m_fifoB);
}

bool RodPixController::getFifoBit(unsigned int ic, unsigned int il) {
  bool bit = false;
  if (il<48) {
    unsigned int ip = (ic*48+il)/32;
    unsigned int ib = (ic*48+il)%32;
    if (ip<m_fifoA.size()) bit = ((m_fifoA[ip]&(0x1<<ib)) != 0);
  } else {
    unsigned int ip = (ic*48+il-48)/32;
    unsigned int ib = (ic*48+il-48)%32;
    if (ip<m_fifoB.size()) bit = ((m_fifoB[ip]&(0x1<<ib)) != 0);
  } 
  return bit;
}

void RodPixController::prepareTestPattern(PixScan *scn) {
  unsigned int i,sum = 0;
  for (i=0; i<96; i++) {
    m_inmLinkTable[i] = 0;
    m_inmModuleTable[i] = 0;
  }
  for (i=0; i<4; i++) sum += scn->getModuleMask(i);
  if (sum != 0) {
    m_modMask = 0;
    bool first = true;
    for (int mod=0; mod<32; mod++) {
      // Check if module is defined
      if (m_modPosition[mod] >=0) {
	bool active = false;
	// Find the group containing the module
	bool notInGrp = true;
	for (int grp=0; grp<4; grp++) {
	  if (scn->getModuleMask(grp) & (0x1<<mod)) {
	    notInGrp = false;
	    setGroupId(mod, grp);
	    if (scn->getReadoutEnabled(grp)) {
	      enableModuleReadout(mod);
              m_modMask |= (0x1 << mod);
	      active = true;
	    } else {
	      disableModuleReadout(mod, false);
	    }
	  }
	}
	if (notInGrp) {
	  setGroupId(mod, 4);
	  disableModuleReadout(mod, false);
	}
	if (active) {
	  PixModule* pixmod = m_modGroup.module(mod);
	  int il1 = ((ConfInt&)pixmod->config()["general"]["OutputLink1"]).getValue();
	  il1 = (il1/16)*12+(((m_fmtLinkMap[il1/16]>>(16+(il1%16)*4)))&0xf);
	  int il2 = ((ConfInt&)pixmod->config()["general"]["OutputLink2"]).getValue();
	  il2 = (il2/16)*12+(((m_fmtLinkMap[il2/16]>>(16+(il2%16)*4)))&0xf);
	  int il3 = ((ConfInt&)pixmod->config()["general"]["OutputLink3"]).getValue();
	  il3 = (il3/16)*12+(((m_fmtLinkMap[il3/16]>>(16+(il3%16)*4)))&0xf);
	  int il4 = ((ConfInt&)pixmod->config()["general"]["OutputLink4"]).getValue();
	  il4 = (il4/16)*12+(((m_fmtLinkMap[il4/16]>>(16+(il4%16)*4)))&0xf);
	  // needs fix!
// 	  if (scn->getMccBandwidth() == PixScan::SINGLE_40 || scn->getRunType() == PixScan::RAW_PATTERN) {
// 	    m_inmLinkTable[il1] = 100;
// 	    m_inmModuleTable[il1] = mod;
// 	    pixmod->pixMCC()->writeRegister("CSR_OutputMode", 0);
// 	  } else if (scn->getMccBandwidth() == PixScan::DOUBLE_80) {
// 	    m_inmLinkTable[il1] = il2;
// 	    m_inmLinkTable[il2] = -il3;
// 	    m_inmLinkTable[il3] = -il4;
// 	    m_inmLinkTable[il4] = -100;
// 	    m_inmModuleTable[il1] = mod;
// 	    m_inmModuleTable[il2] = mod;
// 	    m_inmModuleTable[il3] = mod;
// 	    m_inmModuleTable[il4] = mod;
// 	    pixmod->pixMCC()->writeRegister("CSR_OutputMode", 3);
// 	  } else {
	    m_inmLinkTable[il1] = il2;
	    m_inmLinkTable[il2] = -100;
	    m_inmModuleTable[il1] = mod;
	    m_inmModuleTable[il2] = mod;
// 	    if (scn->getMccBandwidth() == PixScan::DOUBLE_40) {
// 	      pixmod->pixMCC()->writeRegister("CSR_OutputMode", 1);
// 	    } else {
	      pixmod->pixMCC()->writeRegister("CSR_OutputMode", 2);
// 	    }
// 	  }
	  if (first) {
	    if (scn->getRunType() == PixScan::RAW_EVENT) {
	      pixmod->pixMCC()->prepareTestEvent(scn->getPatternSeeds(), m_prepareSeq, m_executeSeq, m_expectedOut);
	    } else {
	      pixmod->pixMCC()->prepareTestPattern(scn->getPatternSeeds(), m_prepareSeq, m_executeSeq, m_expectedOut);
	    }
	    first = false;
	  }
	  if (scn->getHistogramFilled(PixScan::RAW_DATA_REF) && scn->getHistogramKept(PixScan::RAW_DATA_REF)) {
	    Histo *hRef;
	    hRef = new Histo("RAW_DATA_REF", "Reference raw pattern", 
			     (int)(m_expectedOut.size()*1.1), -0.5, (m_expectedOut.size()*1.1)-0.5, 1, -0.5, 0.5);
	    scn->addHisto(*hRef, PixScan::RAW_DATA_REF, mod, -1, -1, -1);
	    for (int ib=0; ib<m_expectedOut.size(); ib++) {
	      if (m_expectedOut[ib]) {
		hRef->set(ib, 0, 1);
	      }
	    }
	  }
	}
      }
    }
  }
}

void RodPixController::runTestPattern(PixScan *scn, std::vector< Histo * > &vh) {
  unsigned int outSize, fifoSize;
  bool patch2ndLink = false;
  outSize = (unsigned int)(m_expectedOut.size()*1.1);
  // needs fix
//   if (scn->getMccBandwidth() == PixScan::SINGLE_40 || scn->getRunType() == PixScan::RAW_PATTERN) {
//     fifoSize = (unsigned int)(outSize*2);
//   } else if (scn->getMccBandwidth() == PixScan::DOUBLE_40) {
//     fifoSize = (unsigned int)(outSize);
//   } else if (scn->getMccBandwidth() == PixScan::SINGLE_80) {
//     fifoSize = (unsigned int)(outSize);
//     patch2ndLink = true;
//   } else {
    fifoSize = (unsigned int)(outSize*0.5);
    patch2ndLink = true;
//   }

  while (vh.size() < 32) vh.push_back((Histo *)NULL);
  for (int mod=0; mod<32; mod++) {
    vh[mod] = (Histo*)NULL;
    if ((m_modMask & (0x1<<mod)) != 0) {
      std::ostringstream nam, tit;
      nam << "RAW_" << mod << "_" << scn->scanIndex(0) << "_" << scn->scanIndex(1) << 
                              "_" << scn->scanIndex(2);
      tit << "Raw data mod " << mod << " L0=" << scn->scanIndex(0) << " L1=" << scn->scanIndex(1) << 
                                       " L2=" << scn->scanIndex(2);
      vh[mod] = new Histo(nam.str(), tit.str(), 4, outSize, -0.5, outSize-0.5, 2, -0.5, 1.5);
    }
  }
  for (int ir = 0; ir < scn->getRepetitions(); ir++) {
    sendCommand(m_prepareSeq, m_modMask);
    setCalibrationDebugMode();
    sendCommand(m_executeSeq, m_modMask);
    downloadFifo(fifoSize);
    unsigned int ic, il;
    bool oneFound[96];
    int idx[96];
    for (il=0; il<96; il++) {
      oneFound[il] = false;
      idx[il] = 0;
    }
    bool bit;
    for (ic=0; ic<m_fifoA.size(); ic++) {
      for (il=0 ; il<96; il++) {
	    if (m_inmLinkTable[il] > 0) {
	      if (getFifoBit(ic,il)) oneFound[il] = true;
	      if (oneFound[il]) {
	        unsigned int cl = il;
	        int mod = m_inmModuleTable[il];
	        while (1) {
	          // Fill histogram
	          bit = getFifoBit(ic,cl);
              int pos = idx[il];
              //if (pos%2 == 1 && pos > 2 && patch2ndLink) pos = pos-2;
	          if (bit) vh[mod]->set(pos+1, 0, (*vh[mod])(pos+1, 0)+1.0);
	          if (pos < m_expectedOut.size()) {
		        if (bit != m_expectedOut[pos]) {
		          vh[mod]->set(pos+1, 1, (*vh[mod])(pos+1, 1)+1.0);
		          vh[mod]->set(0, 1, (*vh[mod])(0, 1)+1.0);
				}
			  }
	          idx[il]++;
	          if (abs(m_inmLinkTable[cl]) == 100) break;
	          cl = abs(m_inmLinkTable[cl]);
			}
		  }
		}
      }
    }
    for (il=0 ; il<96; il++) {
	  if (m_inmLinkTable[il] > 0) {
	    int mod = m_inmModuleTable[il];
		int ip;
        for (ip = idx[il]; ip < m_expectedOut.size(); ip++) {
          if (m_expectedOut[ip]) {
	        vh[mod]->set(ip+1, 1, (*vh[mod])(ip+1, 1)+1.0);
	        vh[mod]->set(0, 1, (*vh[mod])(0, 1)+1.0);
		  }
		}
      }
    }
  }
}

void RodPixController::setConfigurationDebugMode() {
}

void RodPixController::setCalibrationDebugMode() { 
}

void RodPixController::setupTrapping(int slave, PixScan & scn) {
}

void RodPixController::setupTrapping(int slave) {
}


void RodPixController::startTrapping(int slave) {
}

void RodPixController::stopTrapping(int slave) {
}

void RodPixController::startHistogramming(int slave) {
  int modFmtId, modFmtLink, modOutLink;

  //send slave primitive via MDSP
  SendSlavePrimitiveIn *SendSlavePrimitive_In = (SendSlavePrimitiveIn *)new UINT32[(sizeof(StartTaskIn)+sizeof(HistogramOptions)+sizeof(SendSlavePrimitiveIn))/4];
  SendSlavePrimitive_In->slave = slave;
  SendSlavePrimitive_In->primitiveId = SLAVE_START_TASK;
  SendSlavePrimitive_In->nWords = (sizeof(StartTaskIn)+sizeof(HistogramOptions))/4;
  SendSlavePrimitive_In->fetchReply = 2;

  //start slave histogram task
  StartTaskIn *SlaveStartTask_In = (StartTaskIn *)&SendSlavePrimitive_In[1];
  SlaveStartTask_In->id  = HISTOGRAM_TASK;
  SlaveStartTask_In->idMinor = 0;
  SlaveStartTask_In->dataPtr = 0;

  //Initialize histogram control 
  HistogramOptions* histogramOptions = (HistogramOptions *)&SlaveStartTask_In[1];
  histogramOptions->type = SCAN_DATAMON;

  // configuration for different phases
  histogramOptions->exotic_options[4] = 1.e-5;//occRate;

  for(int il=0;il<SCAN_MAXLOOPS;il++){
    histogramOptions->nBins[il] = 1;
    histogramOptions->bin[il]=1;
  }
  histogramOptions->nTriggers = 1;
  histogramOptions->maskStageBegin =0;
  histogramOptions->nMaskStages=32;
  histogramOptions->maskStageTotalSteps = 32;
  histogramOptions->param[0] = 0;

  //add all modules into one SDSP
  int k=0;
  for (int mod=0; mod<32; mod++) {
    // Check if module is defined and active d
    if (m_modPosition[mod] < 0) continue;
    // Check if module is active
    if (!m_modActive[m_modPosition[mod]]) continue;

    modOutLink = ((ConfInt&)m_modGroup.module(mod)->config()["general"]["OutputLink1"]).getValue();
    //modFmtId = modOutLink / 16;
    //modFmtLink = modOutLink % 16;
    int pos = m_modPosition[mod];
    modFmtId   = m_modDTO[pos]/16;
    modFmtLink = m_modDTO[pos]%16;

    int roSpeed = ((ConfInt&)m_modGroup.module(mod)->pixMCC()->config()["Registers"]["CSR_OutputMode"]).getValue();
    if (roSpeed == 0x2 || // single 80
	roSpeed == 0x1) { // double 40
      if(modFmtLink == 2)
        modFmtLink = 1;
    }
    histogramOptions->link[k] =  ((modFmtId << 0x4) + modFmtLink);
    k++;
  }
  histogramOptions->nModules = k;

  std::cout << getCtrlName ()<<" send configuration nModules="<< histogramOptions->nModules  << std::endl;
  RodPrimitive *slaveStartHistogram = new RodPrimitive((sizeof(StartTaskIn)+sizeof(HistogramOptions)+sizeof(SendSlavePrimitiveIn))/4, 0,
      SEND_SLAVE_PRIMITIVE, 0,
      (long int*)SendSlavePrimitive_In);

  RodOutList* outList;
  std::cout << "executeMasterPrimitiveSync startHistogramming" << std::endl;  
  executeMasterPrimitiveSync(*slaveStartHistogram, outList);

  unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);

  if(outList->getMsgLength(1)>0){
    std::cout << getCtrlName()<<" Start HistogramTask slave "<< slave << " nModule " << histogramOptions->nModules<<" length "<< outList->getMsgLength(1) << " outBody= ";
    for (int i=0; i<outList->getMsgLength(1); i++) {
      std::cout <<"0x"<<std::hex << outBody[i]<<" ";
    }
    std::cout << std::endl;
  }
  else{
    std::cout << getCtrlName()<< " Error: Could not start Histogramming on slave "<< slave <<" nModule "<< histogramOptions->nModules<<" "<< outList->getMsgLength(1) << std::dec << std::endl;
  }
  m_rod->deleteOutList();
  delete slaveStartHistogram;
  delete[] SendSlavePrimitive_In;


  //start task after parameters set and histogram task has been activated
  SendSlavePrimitive_In = (SendSlavePrimitiveIn *) new UINT32[(sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4];
  SendSlavePrimitive_In->slave = slave;
  SendSlavePrimitive_In->primitiveId = SLAVE_TALK_TASK;
  SendSlavePrimitive_In->nWords = sizeof(TalkTaskIn)/4;
  SendSlavePrimitive_In->fetchReply = 0;

  TalkTaskIn *SlaveTalkTask_In = (TalkTaskIn *)&SendSlavePrimitive_In[1];
  SlaveTalkTask_In->task  = HISTOGRAM_TASK;
  SlaveTalkTask_In->topic = HIST_GO;
  SlaveTalkTask_In->item  = 0;

  RodPrimitive *hist_go = new RodPrimitive((sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4,0, SEND_SLAVE_PRIMITIVE, 0, (long int*)SendSlavePrimitive_In);
  std::cout << "executeMasterPrimitiveSync hist_go" << std::endl;  
  executeMasterPrimitiveSync(*hist_go);

  m_histoRunning[slave] = true;

  m_rod->deleteOutList();
  delete hist_go;
  delete[] SendSlavePrimitive_In;
}

void RodPixController::stopHistogramming(int slave) {
  SendSlavePrimitiveIn *SendSlavePrimitive_In = (SendSlavePrimitiveIn *) new UINT32[(sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4];
  SendSlavePrimitive_In->slave = slave;
  SendSlavePrimitive_In->primitiveId = SLAVE_TALK_TASK;
  SendSlavePrimitive_In->nWords = sizeof(TalkTaskIn)/4;
  SendSlavePrimitive_In->fetchReply = 0;

  TalkTaskIn *SlaveTalkTask_In = (TalkTaskIn *)&SendSlavePrimitive_In[1];
  SlaveTalkTask_In->task  = HISTOGRAM_TASK;
  SlaveTalkTask_In->topic = HIST_CLOSE;
  SlaveTalkTask_In->item  = 0;

  RodPrimitive *hist_close = new RodPrimitive((sizeof(TalkTaskIn)+sizeof(SendSlavePrimitiveIn))/4,0, SEND_SLAVE_PRIMITIVE, 0, (long int*)SendSlavePrimitive_In);
  std::cout << "executeMasterPrimitiveSync hist_close" << std::endl;
  executeMasterPrimitiveSync(*hist_close);

  m_histoRunning[slave] = false;

  m_rod->deleteOutList();
  delete hist_close;
  delete[] SendSlavePrimitive_In;
}

void RodPixController::stopScan() {
  if(!m_scanParInt) return;

  StartTaskIn *startTaskIn = (StartTaskIn*) m_scanParInt;
  int taskid  = startTaskIn->id;
  if(!(taskid==SCAN_TASK_2||taskid==BOC_SCANM||taskid==LEAK_SCAN)) taskid=SCAN_TASK_2;//if task id is unknown let it be SCAN_TASK_2

  TalkTaskIn TalkTask_In;
  TalkTask_In.task  = taskid;
  TalkTask_In.topic = SCAN_CLOSE;
  TalkTask_In.item  = 0;

  RodPrimitive *stopscantaskprim = new RodPrimitive((sizeof(TalkTaskIn))/sizeof(unsigned int), 0, TALK_TASK, 
      0, (long*)&TalkTask_In);

  RodOutList* outList;
  executeMasterPrimitiveSync(*stopscantaskprim, outList);

  unsigned long *outBody = (unsigned long *) outList->getMsgBody(1);
  UINT32 location = 0;
  for(int i=0;i<outList->getLength();i++)std::cout << "i " << i << std::hex<< " " << outList->getBody()[i]<<std::dec<<std::endl; 

  if((long)(sizeof(TalkTaskOut)/4) < (long)outList->getMsgLength(1)) {
    location = outBody[sizeof(TalkTaskOut)/4];      
  }
  m_scanActive = false;
  m_rod->deleteOutList();
}

void RodPixController::setGroupId(int module, int group) {
  unsigned int mod;
  if (module<0 || module>31) return;
  if (group<0 || group>7) return;
  if (m_modPosition[module] >= 0) {
    mod = m_modPosition[module];
    m_modGrp[mod] = group;
  }
}
 
void RodPixController::enableModuleReadout(int module) {
  std::cout << "!!==================   Not yet implemented " << std::endl;
  unsigned int mod;
  if (module<0 || module>31) return;
  if (m_modPosition[module] >= 0) {
    mod = m_modPosition[module];
    m_modActive[mod]  = true;
    m_modTrigger[mod] = true;
  }  
}

void RodPixController::disableModuleReadout(int module, bool trigger) {
  std::cout << "!!==================   Not yet implemented " << std::endl;
  unsigned int mod;
  if (module<0 || module>31) return;
  if (m_modPosition[module] >= 0) {
    mod = m_modPosition[module];
    m_modActive[mod]  = false;
    m_modTrigger[mod] = trigger;
  }
}


void RodPixController::testGlobalRegister(int module, int frontend) {
}

void RodPixController::ledFlash(int period, int repet, int led) {
}

void RodPixController::ledFlashSlave(int period, int repet, int slave) {
}

void RodPixController::echo(Bits in, Bits out, unsigned int ndat) {
}

void RodPixController::readSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out) {
}

void RodPixController::writeSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in) {
}

bool RodPixController::getGenericBuffer(const char *type, std::string &textBuf) {
  std::string err="";
  std::string info="";

  int i;
  bool ret = false;
  if(strcmp(type,"err") == 0){
    for (i=0; i<5; i++) {
      if (m_rod->getTextBuffer(err, i*2+1, true)) {;
	textBuf += err;
	if (err != "") ret = true;
	err.clear();
      }
    }
    return ret;
  }
  else if(strcmp(type,"info") == 0){
    for (i=0; i<5; i++) {
      if (m_rod->getTextBuffer(info, i*2, true)) {
	textBuf += info;
	if (info != "") ret = true;
	info.clear();
      }
    }
    return ret;
  }
  else{
    textBuf += "Buffer type not available";
  }
  return ret;
}

void RodPixController::initBoc(){               
  BocCard* m_boc = m_rod->getBocCard();
  m_boc->initialize();
  return;
}

void RodPixController::getBocStatus(){
  if(m_rod==0) return;
  BocCard* m_boc = m_rod->getBocCard();
  m_boc->status();
  return;
}

void RodPixController::resetBoc(){
  BocCard* m_boc = m_rod->getBocCard();
  m_boc->reset();
  return;
}

void RodPixController::setBocRegister(std::string regtype, int channel, int value){
  BocCard* m_boc = m_rod->getBocCard();
  if (regtype == "DataDelay"){ m_boc->setRxDataDelay(channel, (UINT32) value);}
  if (regtype == "RxThreshold"){ m_boc->setRxThreshold(channel, (UINT32) value);}
  if (regtype == "BpmFineDelay"){ m_boc->setBpmFineDelay(channel, (UINT32) value);}
  if (regtype == "BpmCoarseDelay"){ m_boc->setBpmCoarseDelay(channel, (UINT32) value);}
  if (regtype == "BpmStreamInhibit"){ m_boc->setBpmStreamInhibit(channel, (UINT32) value);}
  if (regtype == "BpmMarkSpace"){ m_boc->setBpmMarkSpace(channel, (UINT32) value);}
  if (regtype == "LaserCurrent"){ m_boc->setLaserCurrent(channel, (UINT32) value);}
  return;
};

void RodPixController::setBocRegister(std::string regtype, int value){
  BocCard* m_boc = m_rod->getBocCard();
  if (regtype == "ClockControl"){ m_boc->setClockControl((UINT32)value);}
  if (regtype == "BRegClockPhase"){ m_boc->setBregClockPhase((UINT32) value);}
  if (regtype == "BpmClockPhase"){ m_boc->setBpmClockPhase((UINT32) value);}
  if (regtype == "VernierClockPhase0"){ m_boc->setVernierClockPhase0((UINT32) value);}
  if (regtype == "VernierClockPhase1"){ m_boc->setVernierClockPhase1((UINT32) value);}
  if (regtype == "VernierFinePhase"){ m_boc->setVernierFinePhase((UINT32) value);}
  if (regtype == "RxDataMode"){m_boc->setRxDataMode((UINT32) value);}
  return;
};

int RodPixController::getBocRegister(std::string regtype, int channel){
  UINT32 value=0;
  BocCard* m_boc = m_rod->getBocCard();
  if (regtype == "DataDelay"){value = m_boc->getRxDataDelay(channel);}
  if (regtype == "RxThreshold"){
    try {value = m_boc->getRxThreshold(channel);}
    catch (BocException &b) {
      std::cout << "BocException when setting RX threshold" << std::endl;
      std::cout << b.getDescriptor() << " ";
      std::cout << b.getData1() << " : " << b.getData2() << std::endl;
    }
  }
  if (regtype == "BpmFineDelay"){value = m_boc->getBpmFineDelay(channel);}
  if (regtype == "BpmCoarseDelay"){value = m_boc->getBpmCoarseDelay(channel);}
  if (regtype == "BpmStreamInhibit"){value = m_boc->getBpmStreamInhibit(channel);}
  if (regtype == "BpmMarkSpace"){value = m_boc->getBpmMarkSpace(channel);}
  if (regtype == "LaserCurrent"){value = m_boc->getLaserCurrent(channel);}
  return (int)value;
};

int RodPixController::getBocRegister(std::string regtype){
  UINT32 value=0;
  BocCard* m_boc = m_rod->getBocCard();
  if (regtype == "ClockControl"){value = m_boc->getClockControl();}
  if (regtype == "BRegClockPhase"){value = m_boc->getBregClockPhase();}
  if (regtype == "BpmClockPhase"){value = m_boc->getBpmClockPhase();}
  if (regtype == "VernierClockPhase0"){value = m_boc->getVernierClockPhase0();}
  if (regtype == "VernierClockPhase1"){value = m_boc->getVernierClockPhase1();}
  if (regtype == "VernierFinePhase"){value = m_boc->getVernierFinePhase();}
  if (regtype == "RxDataMode"){value = m_boc->getRxDataMode();}
  return (int)value;
};

double RodPixController::getBocMonitorAdc(int channel){
  if(m_rod==0) return 0.0;
  BocCard* m_boc = m_rod->getBocCard();
  return m_boc->getMonitorAdc(channel); 
};

void RodPixController::resetBocMonitorAdc(){
  if(m_rod==0) return;
  BocCard* m_boc = m_rod->getBocCard();
  m_boc->resetMonitorAdc();
  return;
};
void RodPixController::shiftPixMask(int mask, int cap, int steps){
  throw RodPixControllerExc(RodPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR,  
			    getModGroup().getRodName()); 
}
int  RodPixController::readHitBusScaler(int , int , PixScan* ){return 0;}
bool RodPixController::checkRxState(rxTypes ){return false;}
void RodPixController::getServiceRecords(std::string &, std::vector<int> &){}
void RodPixController::setAuxClkDiv(int ){}
void RodPixController::setIrefPads(int ){}
void RodPixController::setIOMUXin(int ){}
void RodPixController::sendGlobalPulse(int ){}

void RodPixController::readMasterMem(int startAddr, int nWords, std::vector<unsigned int> &out) {
  m_rod->readMasterMem(startAddr, nWords, out);
}
void RodPixController::writeMasterMem(int startAddr, int nWords, std::vector<unsigned int> &in) {
  m_rod->writeMasterMem(startAddr, nWords, in);
}
void RodPixController::readSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out) {
  m_rod->readSlaveMem(slaveId, startAddr, nWords, out);
}
void RodPixController::writeSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in) {
  m_rod->writeSlaveMem(slaveId, startAddr, nWords, in);
}
void RodPixController::readFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &out) {
  m_rod->readFifo(fifo, bank, nWords, out);
}
void RodPixController::writeFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &in) {
  m_rod->writeFifo(fifo, bank, nWords, in);
}
void RodPixController::writeRegister(int regId, int offset, int size, unsigned int value) {
  m_rod->writeRegister(regId, offset, size, value);
}
unsigned int RodPixController::readRegister(int regId, int offset, int size) {
  return m_rod->readRegister(regId, offset, size);
}

void RodPixController::executeMasterPrimitive(SctPixelRod::RodPrimitive &prim) {
  m_rod->executeMasterPrimitive(prim);
}
void RodPixController::executeMasterPrimitiveSync(SctPixelRod::RodPrimitive &prim, std::vector<unsigned int> &out) {
  m_rod->executeMasterPrimitiveSync(prim, out);
}
void RodPixController::executeMasterPrimitiveSync(SctPixelRod::RodPrimitive &prim, SctPixelRod::RodOutList* &out) {
  m_rod->executeMasterPrimitiveSync(prim, out);
}
void RodPixController::executeMasterPrimitiveSync(SctPixelRod::RodPrimitive &prim) {
  m_rod->executeMasterPrimitiveSync(prim);
}
void RodPixController::executeSlavePrimitive(SctPixelRod::RodPrimitive &prim, int slave) {
  m_rod->executeSlavePrimitive(prim, slave);
}
void RodPixController::executeSlavePrimitiveSync(SctPixelRod::RodPrimitive &prim, std::vector<unsigned int> &out, int slave) {
  m_rod->executeSlavePrimitiveSync(prim, out, slave);
}
void RodPixController::executeSlavePrimitiveSync(SctPixelRod::RodPrimitive &prim, SctPixelRod::RodOutList* &out, int slave) {
  m_rod->executeSlavePrimitiveSync(prim, out, slave);
}
void RodPixController::executeSlavePrimitiveSync(SctPixelRod::RodPrimitive &prim, int slave) {
  m_rod->executeSlavePrimitiveSync(prim, slave);
}
void RodPixController::executeSlavePrimitiveOnAll(SctPixelRod::RodPrimitive &prim) {
  m_rod->executeSlavePrimitiveOnAll(prim);
}
void RodPixController::executeSlavePrimitiveOnAllSync(SctPixelRod::RodPrimitive &prim) {
  m_rod->executeSlavePrimitiveOnAllSync(prim);
}
void RodPixController::executeSlavePrimitiveOnAllSync(SctPixelRod::RodPrimitive &prim, std::vector<unsigned int> &out) {
  m_rod->executeSlavePrimitiveOnAllSync(prim, out);
}
void RodPixController::executeSlavePrimitiveOnAllSync(SctPixelRod::RodPrimitive &prim, SctPixelRod::RodOutList* &out) {
  m_rod->executeSlavePrimitiveOnAllSync(prim, out);
}

void RodPixController::waitPrimitiveCompletion(std::vector<unsigned int> &out) {
  m_rod->waitPrimitiveCompletion(out);
}
void RodPixController::waitPrimitiveCompletion(SctPixelRod::RodOutList* &out) {
  m_rod->waitPrimitiveCompletion(out);
}
void RodPixController::waitPrimitiveCompletion() {
  m_rod->waitPrimitiveCompletion();
}
bool RodPixController::getSourceScanData(std::vector<unsigned int *>*, bool){
  throw RodPixControllerExc(RodPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR,  
			    getModGroup().getRodName()); 
  return false;
}
bool RodPixController::testGlobalRegister(int module, std::vector<int> &data_in, std::vector<int> &data_out, 
					  std::vector<std::string> &label, bool sendCfg, int feid){
  return false; // to be implemented
}
bool RodPixController::testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out,
			   bool ignoreDCsOff, int DC, bool senCfg, int feIndex, bool bypass) {
  return false; // to be implemented
}
bool RodPixController::testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp, bool shift_only, 
				       bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex){
  return false;
}
void RodPixController::sendCommand(int command, int moduleMask){
  throw RodPixControllerExc(RodPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR,  
			    getModGroup().getRodName()); 
}
void RodPixController::measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval){
  throw RodPixControllerExc(RodPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR,  
			    getModGroup().getRodName()); 
}

void RodPixController::sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge){
  throw RodPixControllerExc(RodPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR,  
			    getModGroup().getRodName()); 
}

bool RodPixController::fitHistos() {    
  // should be done within DSP code now, but flag to PixScan that this is the case
  return true;
}
