/////////////////////////////////////////////////////////////////////
// USBI3PixController.cxx
/////////////////////////////////////////////////////////////////////
//
// 10/11/08  Version 0.0
// 03/09/09  Version 1.0 for USBPix tutorial
// 16/06/15  Version 2.0 copied from release-4-based developement of CG
// Authors: Jens Weingarten, Christian Gobert
//          
// This is the PixController class for the Bonn USB system.
//


#include <string.h>

#include "PixController/PixScan.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "Config/Config.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "PixFe/PixFeI2.h"

#include "PixDcs/SleepWrapped.h"
#include <Config/ConfMask.h>	//CG: needed for 32 package building in writeModuleConfig()


//JW: from forward declarations
//#include "ConfigFPGA.h"
#include "ConfigFEMemoryI3.h"
#include "ConfigRegisterI3.h"

//USB board classes
#include "definesI3.h"
//#include "DataStructDefinesI3.h"

#include "USBI3PixController.h"

#include <processor.h>
#include <sstream>
#include <fstream>
#include <thread>

#define UPC_DEBUG false

using namespace PixLib;
using namespace std;

UINT32 bitflip(UINT32 bits_in){
  UINT32 bits_out=0;
  for(int k=0;k<32;k++){
    if(k<16)
      bits_out |= (bits_in&(1<<k))<<(31-2*k);
    else
      bits_out |= (bits_in&(1<<k))>>(2*k-31);
  }
  return bits_out;
}



// ***************** Constructors *********************
USBI3PixController::USBI3PixController(PixModuleGroup &modGrp, DBInquire *dbInquire) :
                                   PixController(modGrp, dbInquire) { 
  //initHW();
  configInit();
  m_conf->read(dbInquire);

  //  m_FPGAconfig = NULL;
  m_FEconfig = NULL;
  m_BoardHandle = NULL;
  m_USBreg = 0;
}

USBI3PixController::USBI3PixController(PixModuleGroup &modGrp) : 
                                   PixController(modGrp) { //! Constructor
  configInit();

  //  m_FPGAconfig = NULL;
  m_FEconfig = NULL;
  m_BoardHandle = NULL;
  m_USBreg = 0;
}

USBI3PixController::~USBI3PixController(){
}

void USBI3PixController::configInit(){

  // Create the Config object
  m_conf = new Config("USBI3PixController"); 
  Config &conf = *m_conf;

  // Group general

  conf.addGroup("general");

  conf["general"].addInt("BoardID", m_boardID, -1,
  		  "ID of board to use - leave at -1 to take any", true);
  conf["general"].addString("FirmwareFile", m_FPGA_filename, "C:/USBPix/configs/usbpix.bit",
			    "FPGA firmware file name", true, 1, "bit");
  conf["general"].addString("uCFirmwareFile", m_uC_filename, "",
			    "micro controller firmware file name (if empty it is not overwritten)", true, 1, "bix");
  conf["general"].addInt("RegSyncDel", m_regSyncDel, 0,
  		  "Synchronisation phase adjust (0: 0deg. , 1: 180deg.) for register test read-back", true);
  conf["general"].addBool("enableRJ45", m_enableRJ45, false, 
			  "Enable RJ45 input during scans", true);
  // Select default values
  conf.reset();

  m_latchNames.insert(make_pair("HITBUS",HITBUS));
  m_latchNames.insert(make_pair("PREAMP",KILL)); // NB: label is from PixFeI2 class, differes in USBpixI3dll
  m_latchNames.insert(make_pair("SELECT",SELECT));
  m_latchNames.insert(make_pair("ENABLE",MASK_B)); // NB: label is from PixFeI2 class, differes in USBpixI3dll
  m_latchNames.insert(make_pair("TDAC0",TDAC0));
  m_latchNames.insert(make_pair("TDAC1",TDAC1));
  m_latchNames.insert(make_pair("TDAC2",TDAC2));
  m_latchNames.insert(make_pair("TDAC3",TDAC3));
  m_latchNames.insert(make_pair("TDAC4",TDAC4));
  m_latchNames.insert(make_pair("TDAC5",TDAC5));
  m_latchNames.insert(make_pair("TDAC6",TDAC6));
  m_latchNames.insert(make_pair("FDAC0",FDAC0));
  m_latchNames.insert(make_pair("FDAC1",FDAC1));
  m_latchNames.insert(make_pair("FDAC2",FDAC2));

  m_configValid = false;
}

// ***************** Board functionality **************
void USBI3PixController::initHW() {

  int chip_address = 0;

  if(UPC_DEBUG){
    cout<<"INFO: FPGA_filename="<<m_FPGA_filename<<endl;
    cout<<"INFO: uC_filename="<<m_uC_filename<<endl;
  }
  
  if(m_BoardHandle==0){
    // dealt with outside of controller now
    //    if(UPC_DEBUG) cout<<"INFO: calling InitUSB " << endl;
    //    InitUSB();
    m_BoardHandle = new SiUSBDevice(0);
//    cout<<"INFO: m_BoardHandle = "<<m_BoardHandle<<endl;
  }

  if (m_BoardHandle != 0) {

    // Create FE config object
    if(m_FEconfig==NULL){
      m_FEconfig = new ConfigFEMemoryI3(chip_address,0, m_BoardHandle);
      m_USBreg = new ConfigRegisterI3(m_BoardHandle);
    }

    updateDeviceHandle();
    // write uC firmware
    if(m_uC_filename!=""){
      if(UPC_DEBUG) cout<<"Processing uC firmware" << endl;
      FILE *g = fopen(m_uC_filename.c_str(),"r");
      if(g==0){
	if(UPC_DEBUG) cout<<"ERROR: uC bix file doesn't exist"<<endl;
	throw USBI3PixControllerExc(USBI3PixControllerExc::FILE_ERROR, PixControllerExc::ERROR, getCtrlName()); 
      } else
	fclose(g);
      if(!m_BoardHandle->LoadFirmwareFromFile(m_uC_filename.c_str())){
	if(UPC_DEBUG) cout<<"ERROR: uC didn't configure"<<endl;
	throw USBI3PixControllerExc(USBI3PixControllerExc::UC_ERROR, PixControllerExc::ERROR, getCtrlName()); 
      }
      // update device list until board has disappered
      int iw=0;
      const int iwmax=500;
      while(!OnDeviceChange() && iw<iwmax){
	if(UPC_DEBUG) cout <<"INFO: waiting for device to disappear from list, iteration " << iw << endl;
	iw++;
	sleep(50);
      }
      // get new handle as soon as uC is ready again
      iw=0;
      void *tempHandle=0;
      while(tempHandle==0 && iw<iwmax){
	if(OnDeviceChange()) tempHandle = GetUSBDevice(m_boardID);
	if(UPC_DEBUG) cout <<"INFO: waiting for device to be back, iteration " << iw << endl;
	iw++;
	sleep(50);
      }
      // set handle again
      if(tempHandle!=0){
	m_BoardHandle->SetDeviceHandle(tempHandle);
	if(UPC_DEBUG) cout <<"INFO: after uC load - found "<< m_BoardHandle->GetName() << " with ID " << 
	  m_BoardHandle->GetId() << " and FW " << m_BoardHandle->GetFWVersion() << endl;
      } else{
	if(UPC_DEBUG) cout<<"ERROR: no board handle found after uC load..."<<endl;
	throw USBI3PixControllerExc(USBI3PixControllerExc::NOBOARD, PixControllerExc::FATAL, getCtrlName(), m_boardID); 
      }
    }
    // write FPGA firmware
    if(UPC_DEBUG) cout<<"Processing FPGA firmware" << endl;
    FILE *f = fopen(m_FPGA_filename.c_str(),"r");
    if(f==0){
      if(UPC_DEBUG) cout<<"ERROR: FPGA bit file " << m_FPGA_filename <<" doesn't exist"<<endl;
      throw USBI3PixControllerExc(USBI3PixControllerExc::FILE_ERROR, PixControllerExc::ERROR, getCtrlName()); 
    } else
      fclose(f);
    if(!m_BoardHandle->DownloadXilinx(m_FPGA_filename.c_str())){
      if(UPC_DEBUG) cout<<"ERROR: FPGA didn't configure"<<endl;
      throw USBI3PixControllerExc(USBI3PixControllerExc::FPGA_ERROR, PixControllerExc::ERROR, getCtrlName()); 
    }
    // set phase register
    m_USBreg->WriteRegister(CS_XCK_PHASE, m_regSyncDel);
  }else{
    if(UPC_DEBUG) cout<<"ERROR: no SiUSBDevice found..."<<endl;
    throw USBI3PixControllerExc(USBI3PixControllerExc::NOHANDLE, PixControllerExc::FATAL, getCtrlName()); 
    m_FEconfig = NULL;
    m_USBreg = 0;
  }
     if(UPC_DEBUG) cout<<"reading ID..." << endl;
 m_boardIDRB = m_BoardHandle->GetId();
}

void USBI3PixController::testHW() {
  if(m_BoardHandle!=0){
    if(UPC_DEBUG) cout <<"INFO: found "<< m_BoardHandle->GetName() << " with ID " << m_BoardHandle->GetId() <<endl;
  }else
    throw USBI3PixControllerExc(USBI3PixControllerExc::NOBOARD, PixControllerExc::FATAL, getModGroup().getRodName()); 
}

void USBI3PixController::sendCommand(Bits /*command*/, int /*moduleMask*/) {
  throw USBI3PixControllerExc(USBI3PixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR, getModGroup().getRodName()); 
}
void USBI3PixController::sendCommand(int command, int ){
  
  if(command==PixModuleGroup::PMG_CMD_HRESET_ON || command==PixModuleGroup::PMG_CMD_HRESET_PLS){
    int sync_length = 16;
    m_USBreg->WriteSync(sync_length);
    return;
  }
  if(command==PixModuleGroup::PMG_CMD_SRESET){
    m_FEconfig->SendSoftReset();
    return;
  }
  if(command==PixModuleGroup::PMG_CMD_TRIGGER){
    m_USBreg->WriteRegister(CS_LENGTH_STRB_0, 0xff); // ???
    m_USBreg->WriteRegister(CS_LENGTH_STRB_1, 0x01); // ???
    m_USBreg->WriteRegister(CS_LENGTH_LV1, 0x03); // ???
    m_USBreg->WriteRegister(CS_DELAY_LV1, 0x05); // ???
    m_USBreg->WriteRegister(CS_QUANTITY_STRB, 1); // ???
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_0, 0xff); // ???
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_1, 0x04); // ???
    m_USBreg->StartMeasurement();
    return;
  }
  if(command==PixModuleGroup::PMG_CMD_ECR){
    m_FEconfig->SendRefReset();
    return;
  }
  if(command==PixModuleGroup::PMG_CMD_BCR){
    m_FEconfig->SendRefReset();
    return;
  }
}

// ************ Pixel module functions ****************


void USBI3PixController::writeModuleConfig(PixModule& mod) {

  if(UPC_DEBUG) cout<<"INFO USBPixCtrl: called writeModuleConfig()"<<endl;
  if(UPC_DEBUG) cout<<"mod.getFECount() returns\t"<<mod.getFECount()<<endl;
  if(mod.getFECount() != 1)
    throw USBI3PixControllerExc(USBI3PixControllerExc::WRONG_FEI3_COUNT, PixControllerExc::ERROR, getCtrlName()); //Error if FEI3-count differs from 1
  PixFe* firstFE = dynamic_cast<PixFeI2*>(*(mod.feBegin())); //CG: pointer on first and only FEI3
  
  m_FEconfig->SetChipAdd(((ConfInt&)firstFE->config()["Misc"]["Address"]).getValue());	//CG:EDIT : now works fine for FE-I3.
  
  firstFE = dynamic_cast<PixFeI2*>(*(mod.feBegin()));
  
  m_FEconfig->SetGlobalVal(CHIP_LATENCY, 255);// globReg.latency);
  if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: LATENCY \t"<<255<<endl;
  m_FEconfig->SetGlobalVal(GLOBALPARITY, 1);

  //alt:  m_FEconfig->SetGlobalVal(SELFTRGDELAY, globReg.selfLatency);  
  m_FEconfig->SetGlobalVal(SELFTRGDELAY, firstFE->readGlobRegister("SELF_LATENCY"));
  
  
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: SELFTRGDELAY \t"<<firstFE->readGlobRegister("SELF_LATENCY")<<endl;
  m_FEconfig->SetGlobalVal(SELFTRGWIDTH, firstFE->readGlobRegister("SELF_WIDTH"));
if(UPC_DEBUG)cout<<"DEBUG USBPixCtrl: SELFTRGWIDTH \t"<<firstFE->readGlobRegister("SELF_WIDTH")<<endl;
  m_FEconfig->SetGlobalVal(ENABLESELFTRG, firstFE->readGlobRegister("ENABLE_SELF_TRIGGER"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLESELFTRIGGER \t"<<firstFE->readGlobRegister("ENABLE_SELF_TRIGGER")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEHITPARITY, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLE HITPARITY \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(SELECTDO, 8/*firstFE->readGlobRegister("MUX_DO")*/);//globReg.muxDO);	// !!! once was directly initialized with the number 8. //CG:INFO : need to be initialized with 8 in order to allow FE to answer with data on trigger.
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: DO mux \t"<<firstFE->readGlobRegister("MUX_DO")<<endl;
  m_FEconfig->SetGlobalVal(SELECTMONHIT, firstFE->readGlobRegister("MUX_MON_HIT"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: SELECTMONHIT \t"<<firstFE->readGlobRegister("MUX_MON_HIT")<<endl;
  m_FEconfig->SetGlobalVal(TSITSCENABLE, firstFE->readGlobRegister("ENABLE_TIMESTAMP"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TSITSCENABLE \t"<<firstFE->readGlobRegister("ENABLE_TIMESTAMP")<<endl;
  m_FEconfig->SetGlobalVal(SELECTDATAPHASE, 0);//globReg.//statusMonLeak);	//once was directly initialized with the number 0
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: SELECTDATAPHASE \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(ENABLEEOEPARITY, 0); //globReg.enableMonLeak);		//once was directly initialized with the number 0
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TSITSCENABLE \t"<<"0"<<endl;
//  m_FEconfig->SetGlobalVal(HITBUSSCALER, firstFE->readGlobRegister("MON_MON_LEAK_ADC"));// is read-only					//once was directly initialized with the number 0	//maybe might even be deleted.
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: HITBUSSCALER \t"<<firstFE->readGlobRegister("MON_MON_LEAK_ADC")<<endl;
  m_FEconfig->SetGlobalVal(MONLEAKADC, firstFE->readGlobRegister("MON_ADC_REF") + (firstFE->readGlobRegister("DAC_MON_LEAK_ADC")<<1) + 
			   (firstFE->readGlobRegister("MON_MON_LEAK_ADC")<<11) + (firstFE->readGlobRegister("ENABLE_MON_LEAK")<<12));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: MONLEAKADC \t"<<firstFE->readGlobRegister("DAC_MON_LEAK_ADC")<<endl;
  m_FEconfig->SetGlobalVal(AREGTRIM, 1);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: AREGTRIM \t"<<"1"<<endl;
  m_FEconfig->SetGlobalVal(ENABLEAREGMEAS, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEAREGMEAS \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(AREGMEAS, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: AREGMEAS \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(ENABLEAREG, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEAREG \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(ENABLELVDSREFMEAS, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLELVDSREFMEAS \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(DREGTRIM, 1);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: DREGTRIM \t"<<"1"<<endl;
  m_FEconfig->SetGlobalVal(ENABLEDREGMEAS, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEDREGMEAS\t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(DREGMEAS, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: DREGMEAS \t"<<"0"<<endl;
  m_FEconfig->SetGlobalVal(CAPMEASCIRCUIT, firstFE->readGlobRegister("CAP_MEASURE"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: CAPMEASCIRCUIT \t"<<firstFE->readGlobRegister("CAP_MEASURE")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECAPTEST, firstFE->readGlobRegister("ENABLE_CAP_TEST"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECAPTEST \t"<<firstFE->readGlobRegister("ENABLE_CAP_TEST")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEANALOGOUT, firstFE->readGlobRegister("ENABLE_BUFFER"));//monADCRef);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEANALOGOUT \t"<<firstFE->readGlobRegister("ENABLE_BUFFER")<<endl;
  m_FEconfig->SetGlobalVal(TESTPIXELMUX, firstFE->readGlobRegister("MUX_TEST_PIXEL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTPIXELMUX \t"<<firstFE->readGlobRegister("MUX_TEST_PIXEL")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEVCALMEAS, firstFE->readGlobRegister("ENABLE_VCAL_MEASURE"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEVCALMEAS \t"<<firstFE->readGlobRegister("ENABLE_VCAL_MEASURE")<<endl;
  m_FEconfig->SetGlobalVal(ENABLELEAKMEAS, firstFE->readGlobRegister("ENABLE_LEAK_MEASURE"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLELEAKMEAS \t"<<firstFE->readGlobRegister("ENABLE_LEAK_MEASURE")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEBUFFERBOOST, firstFE->readGlobRegister("ENABLE_BUFFER_BOOST"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEBUFFERBOOST \t"<<firstFE->readGlobRegister("ENABLE_BUFFER_BOOST")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL8, firstFE->readGlobRegister("ENABLE_CP8"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL8 \t"<<firstFE->readGlobRegister("ENABLE_CP8")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACIVDD2, firstFE->readGlobRegister("MON_IVDD2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACIVDD2 \t"<<firstFE->readGlobRegister("MON_IVDD2")<<endl;
  m_FEconfig->SetGlobalVal(IVDD2DAC, firstFE->readGlobRegister("DAC_IVDD2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: IVDD2DAC \t"<<firstFE->readGlobRegister("DAC_IVDD2")<<endl;
  m_FEconfig->SetGlobalVal(IDDAC, firstFE->readGlobRegister("DAC_ID"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: IDDAC \t"<<firstFE->readGlobRegister("DAC_ID")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACID, firstFE->readGlobRegister("MON_ID"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACID \t"<<firstFE->readGlobRegister("MON_ID")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL7, firstFE->readGlobRegister("ENABLE_CP7"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL7 \t"<<firstFE->readGlobRegister("ENABLE_CP7")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACIP2, firstFE->readGlobRegister("MON_IP2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACIP2 \t"<<firstFE->readGlobRegister("MON_IP2")<<endl;
  m_FEconfig->SetGlobalVal(IP2DAC, firstFE->readGlobRegister("DAC_IP2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: IP2DAC \t"<<firstFE->readGlobRegister("DAC_IP2")<<endl;
  m_FEconfig->SetGlobalVal(IPDAC, firstFE->readGlobRegister("DAC_IP"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: IPDAC \t"<<firstFE->readGlobRegister("DAC_IP")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACIP, firstFE->readGlobRegister("MON_IP"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACIP \t"<<firstFE->readGlobRegister("MON_IP")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL6, firstFE->readGlobRegister("ENABLE_CP6"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL6 \t"<<firstFE->readGlobRegister("ENABLE_CP6")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACITRIMTH, firstFE->readGlobRegister("MON_ITRIMTH"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACITRIMTH \t"<<firstFE->readGlobRegister("MON_ITRIMTH")<<endl;
  m_FEconfig->SetGlobalVal(ITRIMTHDAC, firstFE->readGlobRegister("DAC_ITRIMTH"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ITRIMTHDAC \t"<<firstFE->readGlobRegister("DAC_ITRIMTH")<<endl;
  m_FEconfig->SetGlobalVal(IFDAC, firstFE->readGlobRegister("DAC_IF"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: IFDAC \t"<<firstFE->readGlobRegister("DAC_IF")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACIF, firstFE->readGlobRegister("MON_IF"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACIF \t"<<firstFE->readGlobRegister("MON_IF")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL5, firstFE->readGlobRegister("ENABLE_CP5"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL5 \t"<<firstFE->readGlobRegister("ENABLE_CP5")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACITRIMIF, firstFE->readGlobRegister("MON_ITRIMIF"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACITRIMIF \t"<<firstFE->readGlobRegister("MON_ITRIMIF")<<endl;
  m_FEconfig->SetGlobalVal(ITRIMIFDAC, firstFE->readGlobRegister("DAC_ITRIMIF"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ITRIMIFDAC \t"<<firstFE->readGlobRegister("DAC_ITRIMIF")<<endl;
  m_FEconfig->SetGlobalVal(VCALDAC, firstFE->readGlobRegister("DAC_VCAL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: VCALDAC \t"<<firstFE->readGlobRegister("DAC_VCAL")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACVCAL, firstFE->readGlobRegister("MON_VCAL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACVCAL \t"<<firstFE->readGlobRegister("MON_VCAL")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL4, firstFE->readGlobRegister("ENABLE_CP4"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL4 \t"<<firstFE->readGlobRegister("ENABLE_CP4")<<endl;
  m_FEconfig->SetGlobalVal(HITINJECTCAPSEL, firstFE->readGlobRegister("ENABLE_CINJ_HIGH"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: HITINJECTCAPSEL, i.e. enableCinjHigh \t"<<firstFE->readGlobRegister("ENABLE_CINJ_HIGH")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEEXTINJ, firstFE->readGlobRegister("ENABLE_EXTERNAL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEEXTINJ \t"<<firstFE->readGlobRegister("ENABLE_EXTERNAL")<<endl;
  m_FEconfig->SetGlobalVal(TESTANALOGREF, firstFE->readGlobRegister("ENABLE_TEST_ANALOG_REF"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTANALOGREF \t"<<firstFE->readGlobRegister("ENABLE_TEST_ANALOG_REF")<<endl;
  m_FEconfig->SetGlobalVal(EOCMUXCONTROL, firstFE->readGlobRegister("MUX_EOC"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: EOCMUXCONTROL \t"<<firstFE->readGlobRegister("MUX_EOC")<<endl;
  m_FEconfig->SetGlobalVal(CEUCLOCKCONTROL, firstFE->readGlobRegister("FREQUENCY_CEU"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: CEUCLOCKCONTROL \t"<<firstFE->readGlobRegister("FREQUENCY_CEU")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEDIGITALINJ, firstFE->readGlobRegister("ENABLE_DIGITAL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEDIGITALINJ \t"<<firstFE->readGlobRegister("ENABLE_DIGITAL")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL3, firstFE->readGlobRegister("ENABLE_CP3"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL3  \t"<<firstFE->readGlobRegister("ENABLE_CP3")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACITH1, firstFE->readGlobRegister("MON_ITH1"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACITH1 \t"<<firstFE->readGlobRegister("MON_ITH1")<<endl;
  m_FEconfig->SetGlobalVal(ITH1DAC, firstFE->readGlobRegister("DAC_ITH1"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ITH1DAC \t"<<firstFE->readGlobRegister("DAC_ITH1")<<endl;
  m_FEconfig->SetGlobalVal(ITH2DAC, firstFE->readGlobRegister("DAC_ITH2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ITH2DAC \t"<<firstFE->readGlobRegister("DAC_ITH2")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACITH2, firstFE->readGlobRegister("MON_ITH2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACITH2 \t"<<firstFE->readGlobRegister("MON_ITH2")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL2, firstFE->readGlobRegister("ENABLE_CP2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL2 \t"<<firstFE->readGlobRegister("ENABLE_CP2")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACIL, firstFE->readGlobRegister("MON_IL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACIL \t"<<firstFE->readGlobRegister("MON_IL")<<endl;
  m_FEconfig->SetGlobalVal(ILDAC, firstFE->readGlobRegister("DAC_IL"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ILDAC \t"<<firstFE->readGlobRegister("DAC_IL")<<endl;
  m_FEconfig->SetGlobalVal(IL2DAC, firstFE->readGlobRegister("DAC_IL2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: IL2DAC \t"<<firstFE->readGlobRegister("DAC_IL2")<<endl;
  m_FEconfig->SetGlobalVal(TESTDACIL2, firstFE->readGlobRegister("MON_IL2"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: TESTDACIL2 \t"<<firstFE->readGlobRegister("MON_IL2")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL1, firstFE->readGlobRegister("ENABLE_CP1"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL1 \t"<<firstFE->readGlobRegister("ENABLE_CP1")<<endl;
  m_FEconfig->SetGlobalVal(THRMIN, firstFE->readGlobRegister("THRESH_TOT_MINIMUM"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: THRMIN \t"<<firstFE->readGlobRegister("THRESH_TOT_MINIMUM")<<endl;
  m_FEconfig->SetGlobalVal(THRDUB, firstFE->readGlobRegister("THRESH_TOT_DOUBLE"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: THRDUB \t"<<firstFE->readGlobRegister("THRESH_TOT_DOUBLE")<<endl;
  m_FEconfig->SetGlobalVal(READMODE, firstFE->readGlobRegister("MODE_TOT_THRESH"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: READMODE \t"<<firstFE->readGlobRegister("MODE_TOT_THRESH")<<endl;
  m_FEconfig->SetGlobalVal(ENABLECOL0, firstFE->readGlobRegister("ENABLE_CP0"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLECOL0 \t"<<firstFE->readGlobRegister("ENABLE_CP0")<<endl;
  m_FEconfig->SetGlobalVal(HITBUSENABLE, firstFE->readGlobRegister("ENABLE_HITBUS"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: HITBUSENABLE \t"<<firstFE->readGlobRegister("ENABLE_HITBUS")<<endl;
  m_FEconfig->SetGlobalVal(GLOBALTDAC, firstFE->readGlobRegister("GLOBAL_DAC"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: GLOBALTDAC \t"<<firstFE->readGlobRegister("GLOBAL_DAC")<<endl;
  m_FEconfig->SetGlobalVal(ENABLETUNE, firstFE->readGlobRegister("ENABLE_AUTOTUNE"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLETUNE \t"<<firstFE->readGlobRegister("ENABLE_AUTOTUNE")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEBIASCOMP, firstFE->readGlobRegister("ENABLE_BIASCOMP"));
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEBIASCOMP \t"<<firstFE->readGlobRegister("ENABLE_BIASCOMP")<<endl;
  m_FEconfig->SetGlobalVal(ENABLEIPMONITOR, 0);
if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: ENABLEIPMONITOR \t"<<"0"<<endl;

  // ********* GlobalRegister done! ***************
if(UPC_DEBUG) cout<<"GlobalRegister done!"<<endl;

  // merge pixel register stuff into 90 32-bit chunks (already done for masks) and 
  // store in register structures of m_FEconfig
  std::string pixName[4];
  pixName[0]="ENABLE";
  pixName[1]="SELECT";
  pixName[2]="PREAMP";
  pixName[3]="HITBUS";
  int col, row;
  if(UPC_DEBUG) cout<<"writeModuleConfig(): entering TDAC/FDAC loops..."<<endl;
  for(int i=TDAC0;i<=FDAC2;i++){
    for(int j=0;j<PIXEL_REG_INT;j++){
      int trimBits = 0;
      for(int k=0;k<32;k++){
	int bit_offs = i-TDAC0;
	col = j/5;
	if(col%2==0)
	  row = k+32*(j%5);
	else
	  row = 159-k-32*(j%5);
	if(i>=FDAC0){ // FDAC starts here
	  bit_offs = i-FDAC0; // start again from 0
	  trimBits |= (int)(((firstFE->readTrim("FDAC")[col][row] & (1<<bit_offs))>>bit_offs)<<k);	//CG:changed to PixFE-access
	}else // still in TDAC
	  trimBits |= (int)(((firstFE->readTrim("TDAC")[col][row] & (1<<bit_offs))>>bit_offs)<<k);	//CG:changed to PixFE-access
      }
      m_FEconfig->SetPixelVal(92-j, trimBits, i);
    }
  }
  //CG:EDIT : added and debugged this 32 bit encoding for FE-I3.
  ConfMask<bool> pixReg(18, 160, 1);
  UINT32 blocks32[4][5][18];
  
  UINT32* vanisher_ptr = &blocks32[0][0][0];
  for(int vanisher = 0; vanisher < 18*5*4; vanisher++)	//CG: making sure that every 32bit-block is = 0
  {
	*vanisher_ptr = 0;
	vanisher_ptr +=1;
  }
  
  UINT32* pixValue[4];
  pixValue[0] = &(blocks32[0][0][0]);
  pixValue[1] = &(blocks32[1][0][0]);
  pixValue[2] = &(blocks32[2][0][0]);
  pixValue[3] = &(blocks32[3][0][0]);
  
  int pixNum;
  for(pixNum=0; pixNum<4; pixNum++)
  {
	//if(UPC_DEBUG) cout<<"pixNum =\t"<<pixNum<<endl;
    //try
	//{
		pixReg = firstFE->readPixRegister(pixName[pixNum]);
    //}
	//catch(PixFeExc &FeExc)
	//{
	//  FeExc.dump(std::cout);
	//  throw PixFeExc(PixFeExc::INCOMPLETE_CONFIG, PixFeExc::ERROR, m_name);
	//}
	
	for(int col_enc=0; col_enc<18; col_enc++)
      for(int i=0; i<5; i++)
		*(pixValue[pixNum]+(i*18 + col_enc)) = 0;
      
	for(int row_enc=0; row_enc<160; row_enc++)
	{
		//if(UPC_DEBUG) cout<<"row_enc =\t"<<row_enc<<endl;
		for(int col_enc=0; col_enc<18; col_enc++)
		{
			//if(UPC_DEBUG) cout<<"col_enc =\t"<<col_enc<<endl;
			*(pixValue[pixNum]+((row_enc/32)*18 + col_enc)) |= ((UINT32)pixReg[col_enc][row_enc])<<(row_enc%32);
		}
    }
  }
  
  
/*  
  for (int maskType = 0; maskType < 4; maskType++)
  {
	if(UPC_DEBUG) cout<<"maskType =\t"<<maskType<<endl;
	for(int row_enc = 0; row_enc< 160; row_enc++)
	{
		if(UPC_DEBUG) cout<<"\trow_enc =\t"<<row_enc<<endl;
		for(int col_enc = 0; col_enc < 18; col_enc++)
		{
			if(UPC_DEBUG) cout<<"\t\tcol_enc =\t"<<col_enc<<endl;
			**(blocks32[maskType]+((row_enc/32)*18 + col_enc)) |= ((int)(firstFE->readPixRegister(pixName[maskType])[col_enc][row_enc]))<<(row_enc%32); 
		}
	}
  }
*/ 
  if(UPC_DEBUG) cout<<"writeModuleConfig(): entering mask loops..."<<endl;
  for(row=0; row<5; row++){
    for(col=0; col<18; col++){
      int index= row+5*col;
      int pm_row = row;
      if(col%2!=0) pm_row = 4-row;
      UINT32 pm_mask = blocks32[3][pm_row][col];	//masks.maskHitbus[pm_row][col]
      if(col%2!=0) pm_mask = bitflip(pm_mask);
      m_FEconfig->SetPixelVal(92-index, (int)pm_mask, HITBUS); //JW: 1 = pixel connected to hitbus
      pm_mask = blocks32[1][pm_row][col];	//masks.maskSelect[pm_row][col]
      if(col%2!=0) pm_mask = bitflip(pm_mask);
      m_FEconfig->SetPixelVal(92-index, (int)pm_mask, SELECT); //JW: 1 = inject into pixel
      pm_mask = blocks32[0][pm_row][col];	//masks.maskEnable[pm_row][col]
      if(col%2!=0) pm_mask = bitflip(pm_mask);
      m_FEconfig->SetPixelVal(92-index, (int)pm_mask, MASK_B); //JW: 1 = pixel enabled
      pm_mask = blocks32[2][pm_row][col];	//masks.maskPreamp[pm_row][col]
      if(col%2!=0) pm_mask = bitflip(pm_mask);
      m_FEconfig->SetPixelVal(92-index, (int)pm_mask, KILL); //JW: 1 = pixel operable
    }
  }
  // pixel register done
  if(UPC_DEBUG) cout<<"writeModuleConfig(): all loops done"<<endl;
  m_configValid = true;
  if(UPC_DEBUG) cout<<"writeModuleConfig(): done and now leaving..."<<endl;
}

void USBI3PixController::readModuleConfig(PixModule& /*mod*/){
  if(m_FEconfig!=NULL) {
    m_FEconfig->ReadGlobal();
    m_FEconfig->ReadPixel();
  }
}

void USBI3PixController::sendModuleConfig(unsigned int /*moduleMask*/) {

  if(m_FEconfig!=NULL && m_configValid){
    m_FEconfig->WriteGlobal();
    m_FEconfig->WritePixel();
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
  }
}
 
//CG:EDIT
void USBI3PixController::sendModuleConfig(PixModule& mod) {
  writeModuleConfig(mod);
  if(m_FEconfig!=NULL && m_configValid) {
    m_FEconfig->WriteGlobal();
    m_FEconfig->WritePixel();
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
  }

}

bool USBI3PixController::testGlobalRegister(int /*module*/, std::vector<int> &data_in, std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg, int frontend)
{
  if(UPC_DEBUG) cout<<"INFO USBPixCtrlI3: called testGlobalRegister()"<<endl;
  bool retVal = false;
  if(frontend!=0) return retVal; // board can deal with just one FE

  if(m_FEconfig!=0 && m_configValid){
  
    if(sendCfg) m_FEconfig->WriteGlobal();
    m_FEconfig->ReadGlobal();
    retVal = true;
    //infoTxt = "Sent\tRead\n";
    int Address, Size, wValue, rValue;
    for(int i=CHIP_LATENCY;i<=ENABLEIPMONITOR;i++){
      m_FEconfig->GetGlobalVarAddVal(i, Address, Size, wValue);
      m_FEconfig->GetGlobalRBVarAddVal(i, Address, Size, rValue);
      stringstream a,b,st;
      //st << std::string(gloCnfRegItem[i].name);
      st << m_FEconfig->getGlobalVarName(i);
      data_in.push_back(wValue);
      data_out.push_back(rValue);
      label.push_back(st.str());	//only plain loop index as string as temporary solution
      // skip bits that are changed during write/read process
      if(i!=MONLEAKADC && i!=SELECTDO && i!=HITBUSSCALER){
	if(wValue!=rValue) retVal = false;
      }
    }
  }
  if(UPC_DEBUG) cout<<"INFO USBPixCtrlI3: testGlobalRegister returns: "<<retVal<<endl;
  return retVal;
}
bool USBI3PixController::testPixelRegister(int /*module*/, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out, 
					   bool /*ignoreDCsOff*/, int /*DC*/, bool sendCfg, int frontend, bool /*bypass*/)
{
  if(UPC_DEBUG) cout<<"INFO USBPixCtrlI3: called testPixelRegister()"<<endl;
  bool retVal = false;
  if(frontend!=0) return retVal; // board can deal with just one FE

  map<string, int> cmdNames;
  cmdNames.insert(make_pair("HITBUS",FE_RB_HITBUS));
  cmdNames.insert(make_pair("SELECT",FE_RB_SELECT));
  cmdNames.insert(make_pair("ENABLE",FE_RB_MASK_B)); // NB: label is from PixFeI2 class, differes in USBpixI3dll
  cmdNames.insert(make_pair("PREAMP",FE_RB_KILL)); // NB: label is from PixFeI2 class, differes in USBpixI3dll
  cmdNames.insert(make_pair("TDAC0",FE_RB_TDAC0));
  cmdNames.insert(make_pair("TDAC1",FE_RB_TDAC1));
  cmdNames.insert(make_pair("TDAC2",FE_RB_TDAC2));
  cmdNames.insert(make_pair("TDAC3",FE_RB_TDAC3));
  cmdNames.insert(make_pair("TDAC4",FE_RB_TDAC4));
  cmdNames.insert(make_pair("TDAC5",FE_RB_TDAC5));
  cmdNames.insert(make_pair("TDAC6",FE_RB_TDAC6));
  cmdNames.insert(make_pair("FDAC0",FE_RB_FDAC0));
  cmdNames.insert(make_pair("FDAC1",FE_RB_FDAC1));
  cmdNames.insert(make_pair("FDAC2",FE_RB_FDAC2));
  if(m_FEconfig!=0 && m_configValid){
    int latch=m_latchNames[regName];
    int cmd = cmdNames[regName];
    if(cmdNames.find(regName)!=cmdNames.end()){
      int Address, Size, wValue, rValue;
      if(sendCfg) m_FEconfig->WritePixel(); //CG:EDIT : writeback-only support added for I3.
      m_FEconfig->ReadPixel(cmd, latch);
      retVal = true;
      for(int i=0;i<90;i++){
	m_FEconfig->GetPixelVarAddVal(92-i, Address, Size, wValue, latch);
	m_FEconfig->GetPixelRBVarAddVal(92-i, Address, Size, rValue, latch);
	stringstream st;
	st << i;
	data_in.push_back(wValue);
	data_out.push_back(rValue);
	if(wValue!=rValue) retVal = false;
      }
    }
  }
  if(UPC_DEBUG) cout<<"INFO USBPixCtrlI3: testPixelRegister returns: "<<retVal<<endl;
  return retVal;
}
// *******************************************************
// ******* Setup for runmodes: Calibration/Data Taking ***
// *******************************************************
void USBI3PixController::setCalibrationMode() {
}

void USBI3PixController::setConfigurationMode() {
}

void USBI3PixController::setRunMode() {
  m_USBreg->SetRunMode();
  m_USBreg->ClearSRAM();
}

void USBI3PixController::setUSBRunMode() {
  m_USBreg->SetRunMode();
  m_USBreg->ClearSRAM();
}


bool USBI3PixController::moduleActive(int /*nmod*/) {                                      //! True if module is active during scan or run
  //JW: Do something like this:
  //return config["general"]["Active"];

  //JW: Then again, we only have one 'module', that should always be active...

  return true;
}

// ***********************************************
// ******* Setup scanning on the board ***********
// ***********************************************

int USBI3PixController::translateScanParam(PixScan::ScanParam param) {

  int scanVar = -2;

  switch(param) {
  default:
    scanVar = -2;
    break;
  case  PixScan::NO_PAR:
    scanVar = 0;
    break;
//   case PixScan::IVDD2:
//     scanVar = IVDD2DAC;
//     break;
//   case PixScan::ID:
//     scanVar = IDDAC;
//     break;
//   case PixScan::IP2:
//     scanVar = IP2DAC;
//     break;
//   case PixScan::IP:
//     scanVar = IPDAC;
//     break;
//   case PixScan::TRIMT:
//     scanVar = ITRIMTHDAC;
//     break;
//   case PixScan::IF:
//     scanVar = IFDAC;
//     break;
//   case PixScan::TRIMF:
//     scanVar = ITRIMIFDAC;
//     break;
//   case PixScan::ITH1:
//     scanVar = ITH1DAC;
//     break;
//   case PixScan::ITH2:
//     scanVar = ITH2DAC;
//     break;
//   case PixScan::IL:
//     scanVar = ILDAC;
//     break;
//   case PixScan::IL2:
//     scanVar = IL2DAC;
//     break;
  case PixScan::LATENCY:
    scanVar = CHIP_LATENCY;
    break;
  case PixScan::GDAC:
    scanVar = GLOBALTDAC;
    break;
  case PixScan::VCAL:
    scanVar = VCALDAC;
    break;
  case PixScan::TDACS:
  case PixScan::FDACS:
  case PixScan::TRIGGER_DELAY:
  case PixScan::STROBE_DURATION:
  case PixScan::STROBE_DELAY:
//   case PixScan::TDACS_VARIATION:
    scanVar = -1;
    break;
//   case PixScan::BOC_BPH:
//   case PixScan::BOC_VPH0: 
//   case PixScan::BOC_VPH1: 
//   case PixScan::BOC_VFINE:
//   case PixScan::BOC_BPMPH:
//   case PixScan::BOC_TX_CURR:
//   case PixScan::BOC_TX_MS:
//   case PixScan::BOC_RX_THR:
//   case PixScan::BOC_RX_DELAY:
//   case PixScan::BOC_TX_BPM:
//   case PixScan::BOC_TX_BPMF:
//   case PixScan::BOC_BVPH:
//     scanVar = -5;
//     break;
  }
  return scanVar;
}

void USBI3PixController::writeScanConfig(PixScan &scn) {                               //! Write scan parameters

  int maskshift=0, maskstages=0;
  int address=-1, size=-1, value=-1;
  //  int index = -1;
  int shiftmask=2;
  int pixelmask[90], pixelmask2[90];
  bool xtalk = false;

  m_SourceScanFlag = scn.getSourceScanFlag();
  m_testBeamFlag = scn.getTestBeamFlag();
  m_sramReadoutAt = scn.getSramReadoutAt();

  for(int i=0; i<90; i++) {
    pixelmask[i]=0;
    pixelmask2[i]=0;
  }

  for(unsigned int i=0; i<8; i++)
    m_scanConfigArray[i] = 0;

  for(unsigned int i=0; i<5; i++)
    m_clusterPars[i] = 0;

  //JW: set digInject bit in global register
  if(scn.getDigitalInjection()) m_FEconfig->SetGlobalVal(ENABLEDIGITALINJ, 1);
  else m_FEconfig->SetGlobalVal(ENABLEDIGITALINJ, 0);

  //JW: set injection capacitor
  if(scn.getChargeInjCap()!= 0) m_FEconfig->SetGlobalVal(HITINJECTCAPSEL, 1);
  else m_FEconfig->SetGlobalVal(HITINJECTCAPSEL, 0);

  //JW: set selftrigger
  if(scn.getSelfTrigger()) m_FEconfig->SetGlobalVal(ENABLESELFTRG, 1);
  else m_FEconfig->SetGlobalVal(ENABLESELFTRG, 0);

  //Loop 0
  if(!scn.getLoopActive(0)) {    //JW: loop 0 is not active--> set scan parameter to VCal, scan at current value
    //cout<<"INFO USBPixCtrl: Scan loop 0 is not active"<<endl;
    m_scanConfigArray[0] = VCALDAC;
    value = scn.getFeVCal();
    if(value == 8191) m_FEconfig->GetGlobalVarAddVal(VCALDAC, address, size, value);
    m_FEconfig->SetGlobalVal(VCALDAC, value);
    if(value!=-1){
      m_scanConfigArray[1] = value;
      m_scanConfigArray[2] = value+1;
    }
    m_scanConfigArray[3]=1;
  } else if(scn.getDspProcessing(0)) {  //JW: loop 0 is active, executed on controller
    m_scanConfigArray[0] = translateScanParam(scn.getLoopParam(0));  //JW: Get ScanParameter
    if(scn.getLoopVarUniform(0)) {
      m_scanConfigArray[1] = (int)scn.getLoopVarMin(0);
      m_scanConfigArray[2] = (int)scn.getLoopVarMax(0);
      if ((scn.getLoopVarNSteps(0) == 1) || (scn.getLoopVarMax(0) == scn.getLoopVarMin(0))) { // ScanVarStepSize
        m_scanConfigArray[3] = 0;
      } else {
        m_scanConfigArray[3] = (int)((scn.getLoopVarMax(0) - scn.getLoopVarMin(0)) / (scn.getLoopVarNSteps(0) - 1));
      }
    } else 
      throw USBI3PixControllerExc(USBI3PixControllerExc::NON_UNIFORM_POINTS_NOT_IMPLEMENTED, PixControllerExc::FATAL, getModGroup().getRodName()); 
  }
  else { // JW: loop 0 is executed on the host
    m_scanConfigArray[0] = VCALDAC;
    m_FEconfig->GetGlobalVarAddVal(m_scanConfigArray[0], address, size, value); //JW: read current DAC setting
    m_FEconfig->GetGlobalVarAddVal(VCALDAC, address, size, value);
    m_scanConfigArray[1] = value;
    m_scanConfigArray[2] = value+1;
    m_scanConfigArray[3] = 1;
  }

  m_scanConfigArray[4] = scn.getRepetitions(); // Number of Injections

  //  int val = 0; 
  switch(scn.getMaskStageTotalSteps()) {
  case PixScan::STEPS_32:   maskstages = 32;   break;
  case PixScan::STEPS_40:   maskstages = 40;   break;
  case PixScan::STEPS_64:   maskstages = 64;   break;
  case PixScan::STEPS_80:   maskstages = 80;   break;
  case PixScan::STEPS_160:  maskstages = 160;  break;
  case PixScan::STEPS_320:  maskstages = 320;  break;
  case PixScan::STEPS_2880: maskstages = 2880; break;
  default:                  maskstages = 32;   break;
  }
  maskshift = 1; //(int)((maskstages/scn.getMaskStageSteps()) + 0.5);

  m_scanConfigArray[5]= maskshift; // ShiftMaskStepSize
  if(UPC_DEBUG) cout << "N total mask steps: " << scn.getMaskStageTotalSteps() << endl;
  if(UPC_DEBUG) cout << "N mask stages: " << scn.getMaskStageSteps() << endl;
  m_scanConfigArray[6]= scn.getMaskStageSteps(); // ShiftMaskSteps

  switch(scn.getMaskStageMode()) {
  case PixScan::SEL:         shiftmask = 0x02; break;
  case PixScan::ENA:         shiftmask = 0x04; break;
  case PixScan::SEL_ENA:     shiftmask = 0x06; break;
  case PixScan::SEL_PRE:     shiftmask = 0x0a; break;
  case PixScan::SEL_ENA_PRE: shiftmask = 0x0e; break;
  case PixScan::HITB:        shiftmask = 0x01; break;
  case PixScan::XTALK:       shiftmask = 0x06; 
                             xtalk = true; 
			     break; //JW: shift SELECT and ENABLE, set shiftmask to 0x06 after testing!
  case PixScan::STATIC:      shiftmask = 0x00; break;
  default: shiftmask = 0x00; break; // static
  }

  m_scanConfigArray[7] = shiftmask;  //use this to shift other masks

  if(!scn.getLoopActive(0) || !scn.getDspProcessing(0)) //JW: loop 0 not active, expect one histogram
    m_scanConfigArray[8]=1; 
  else
    m_scanConfigArray[8]=scn.getLoopVarNSteps(0); //keep number of steps, makes things easier

// **** configuration for strobe and LVL1 signals *****
  int scan_array[7];
  int maximum_length = 4000; // JJ: set maximum length of period in numbers of bunch crossings, should give enough time to receive all EoE/hit words

  scan_array[0] = (0xff & scn.getStrobeDuration());
  scan_array[1] = (0xff & (scn.getStrobeDuration() >> 8));
  scan_array[2] = (0xff & scn.getConsecutiveLvl1TrigA(0));
  scan_array[3] = (0xff & scn.getStrobeLVL1Delay());
  scan_array[4] = (0xff & scn.getRepetitions());
  scan_array[5] = (0xff & maximum_length);
  scan_array[6] = (0xff & (maximum_length >> 8));

  m_USBreg->WriteRegister(CS_LENGTH_STRB_0, scan_array[0]);
  m_USBreg->WriteRegister(CS_LENGTH_STRB_1, scan_array[1]);
  m_USBreg->WriteRegister(CS_LENGTH_LV1, scan_array[2]);
  m_USBreg->WriteRegister(CS_DELAY_LV1, scan_array[3]);
  m_USBreg->WriteRegister(CS_QUANTITY_STRB, scan_array[4]);
  m_USBreg->WriteRegister(CS_LENGTH_TOTAL_0, scan_array[5]);
  m_USBreg->WriteRegister(CS_LENGTH_TOTAL_1, scan_array[6]);

// ******** prepare pixel register masks ***************** 
  for (int j = 0; j < 2880; j++)      //JW: this loops over all pixels in the FE
    if(j%maskstages==0) {
      pixelmask[j/32] = pixelmask[j/32] | (0x00000001<<(j%32));
      if(j%32>0) {             //JW: only possible if not 32-step-mask
	pixelmask2[j/32]= pixelmask2[j/32]| (0x00000001<<((j%32)+1));
	pixelmask2[j/32]= pixelmask2[j/32]| (0x00000001<<((j%32)-1));
      } else {                 //JW: first step for 32-step-mask
	pixelmask2[j/32]= pixelmask2[j/32]| 0x00000002; 
	if(j/32) pixelmask2[j/32-1]= pixelmask2[j/32-1]| 0x80000000;
      }
    }
  
  m_FEconfig->WriteGlobal();                      //JW: update global register before scanning
  if(shiftmask) { // JGK: does nothing for now, but perhaps useful when using static masks?
    for(int l = PIXEL2880; l <=PIXEL32; l++) {     //JW: this loops over the 90 blocks of INTs, including Maltes offset
      if(shiftmask&0x01)
	m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask[l-3], HITBUS);
      if(shiftmask&0x02){
	if(!xtalk) m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask[l-3],  SELECT);
	else       m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask2[l-3], SELECT);
      }
      if(shiftmask&0x04)
	m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask[l-3], MASK_B);
      if(shiftmask&0x08)
	m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask[l-3], KILL);
    }
  }
  m_FEconfig->WritePixel();

// ******** end pixel register settings ************************

// ******** special settings for externally triggered source scan *********
  if(m_SourceScanFlag) {
    // store clustering parameters
    int minClusterHits, maxClusterHits, maxClusterHitTot, maxEventIncomplete, maxEventErrors; // needed for compatibility with FE-I4 parameters
    scn.getClusterPars(m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], minClusterHits, maxClusterHits, maxClusterHitTot, maxEventIncomplete, maxEventErrors);
    m_clusterPars[3] = 8; // timewalk thresh. = 8
    m_clusterPars[4] = 0; // advanced clustering off

    if(UPC_DEBUG) cout << "DEBUG: found SourceScanFlag active" << endl;
    m_SourceFilename=scn.getSourceRawFile();
    if(UPC_DEBUG) cout << "DEBUG: using source raw file name " << m_SourceFilename << endl;
    m_NewRawDataFile = true; // JJ: new scan, new file/overwrite old file with same filename

    m_USBreg->WriteRegister(CS_LENGTH_STRB_0, scan_array[0]);
    m_USBreg->WriteRegister(CS_LENGTH_STRB_1, scan_array[1]);
    m_USBreg->WriteRegister(CS_LENGTH_LV1, scan_array[2]);
    m_USBreg->WriteRegister(CS_DELAY_LV1, scan_array[3]);
    m_USBreg->WriteRegister(CS_QUANTITY_STRB, scan_array[4]);
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_0, scan_array[5]);
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_1, scan_array[6]);

    m_globalHitcount = 0;
    m_USBreg->SetNumberOfEvents(scn.getRepetitions()); // JJ: write #events to FPGA register, see also CS_COUNTER_MODE
    
    m_USBreg->ClearSRAM();
  }
// ******** end source scan settings ***************************************
}

void USBI3PixController::finalizeScan(){
  if(UPC_DEBUG) cout << "USBPixController::finalizeScan" << endl;
  
  if (m_scanExcept)
    std::rethrow_exception(m_scanExcept);
  
  if(m_scanThread.joinable()) 
    m_scanThread.join();

//   if (m_createdRawDataFile)
//   {
//     for (std::vector<int>::iterator it = m_chipIds.begin();
//         it != m_chipIds.end(); it++)
//     {
//       writeRawDataFile(true, *it);
//     }
//     m_createdRawDataFile = false;
//     m_USBpix->FinishFileFromRawData(m_rawDataFilename);
//   }
}
void USBI3PixController::startScanDelegated(PixScan& scn) {                                                 //! Start a scan
  int maskstages;
  int pixelmask[90], pixelmask2[90];
  m_srcCounterType = 0;
  m_SourceScanDone=false;
  
  for(int i=0; i<90; i++) {
    pixelmask[i]  = 0;
    pixelmask2[i] = 0;
  }
  
  if(UPC_DEBUG) cout << "N total mask steps: " << scn.getMaskStageTotalSteps() << endl;

  switch(scn.getMaskStageTotalSteps()) {
  case PixScan::STEPS_32:   maskstages = 32;   break;
  case PixScan::STEPS_40:   maskstages = 40;   break;
  case PixScan::STEPS_64:   maskstages = 64;   break;
  case PixScan::STEPS_80:   maskstages = 80;   break;
  case PixScan::STEPS_160:  maskstages = 160;  break;
  case PixScan::STEPS_320:  maskstages = 320;  break;
  case PixScan::STEPS_2880: maskstages = 2880; break;
  default:                  maskstages = 32;   break;
  }

  //m_USBreg->SetCalibrationMode();
  writeScanConfig(scn);
  if(!scn.getSourceScanFlag()) {
    
    m_USBreg->ClearSRAM();
    
    if (scn.getHistogramFilled(PixScan::TOT_MEAN) || scn.getHistogramFilled(PixScan::TOT_SIGMA)) {
      m_USBreg->SetTOTMode();
      m_USBreg->ClearTOTHisto(); // JJ: need to clear tot histogram 
    }
    else if (scn.getHistogramFilled(PixScan::OCCUPANCY)) {
      m_USBreg->SetCalibrationMode();
      m_USBreg->ClearConfHisto(); // JJ: need to clear conf data array
    }
    
    // JW: DEBUGING source scan --> do it in raw data mode!
    //if(scn->getSourceScanFlag()) m_USBreg->SetRunMode();
    
    if (scn.getMaskStageMode() == PixScan::XTALK) { // JW: XTALK: run first step of scan here
      if(UPC_DEBUG) cout << "DEBUG: scn->getMaskStageMode() == PixScan::XTALK" << endl;
      // do the scan step
      if(UPC_DEBUG) cout << "DEBUG: m_FEconfig->StartuCScan()" << endl;
      m_FEconfig->StartuCScan(m_scanConfigArray[0], m_scanConfigArray[1], m_scanConfigArray[2],
                  m_scanConfigArray[3], m_scanConfigArray[4], m_scanConfigArray[5],
                  1, m_scanConfigArray[7]);
      if(UPC_DEBUG) cout << "DEBUG: " << "ScanVarIndex: " << m_scanConfigArray[0] << " ScanVarStart: " << m_scanConfigArray[1] << " ScanVarStop: " << m_scanConfigArray[2]
                  << " ScanVarStepSize: " << m_scanConfigArray[3] << " Injections: " << m_scanConfigArray[4] << " ShiftMaskStepSize: " << m_scanConfigArray[5]
                  << " ShiftMaskSteps: " << "1" << " ShiftMaskType: " << m_scanConfigArray[7] << endl;
      // create masks for second step
      for (int j = 0; j < 2880; j++) { // JW: this loops over all pixels in the FE
	if(j%maskstages==0) {
	  pixelmask[j/32] = pixelmask[j/32] | (0x00000001<<((j%32)+1));
	  if(j%32>0) { // JW: only possible if not 32-step-mask
	    pixelmask2[j/32]= pixelmask2[j/32]| (0x00000001<<((j%32)+2));
	    pixelmask2[j/32]= pixelmask2[j/32]| (0x00000001<<((j%32)));
	  } else { // JW: first step for 32-step-mask
	    pixelmask2[j/32]= pixelmask2[j/32]| 0x00000005; 
	  }
	}
      }
      // write masks to pixel registers
      for(int l = PIXEL2880; l <=PIXEL32; l++) {
	m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask2[l-3], SELECT);
	m_FEconfig->SetPixelVal((PIXEL32-l+3), pixelmask[l-3], MASK_B);
      }
      m_FEconfig->WritePixel();
      // decrease number of masksteps to do by one
      m_scanConfigArray[6]--;
      // go ahead and run from the second maskstep on
    }
    m_FEconfig->StartuCScan(m_scanConfigArray[0],  m_scanConfigArray[1],  m_scanConfigArray[2], 
			    m_scanConfigArray[3],  m_scanConfigArray[4],  m_scanConfigArray[5],
			    m_scanConfigArray[6],  m_scanConfigArray[7]);
    m_SourceScanDone=true;// (ab)using this flag to singal runStatus() that scan was actually started
    if(UPC_DEBUG) cout << "DEBUG: m_FEconfig->StartuCScan()" << endl;
    if(UPC_DEBUG) cout << "DEBUG: " << "ScanVarIndex: " << m_scanConfigArray[0] << " ScanVarStart: " << m_scanConfigArray[1] << " ScanVarStop: " << m_scanConfigArray[2]
                << " ScanVarStepSize: " << m_scanConfigArray[3] << " Injections: " << m_scanConfigArray[4] << " ShiftMaskStepSize: " << m_scanConfigArray[5]
                << " ShiftMaskSteps: " << m_scanConfigArray[6] << " ShiftMaskType: " << m_scanConfigArray[7] << endl;
  } else { // source scan, external trigger
    
    clearSourceScanHistos();
    
    if(UPC_DEBUG) cout<<"DEBUG: starting source measurement"<<endl;
    m_USBreg->SetRunMode();
    m_USBreg->ClearSRAM();
    m_globalHitcount = 0;
    if(UPC_DEBUG) cout<<"DEBUG: source measurement trigger and count modes: " << scn.getSrcTriggerType() << " - " 
		      << scn.getSrcCountType() <<endl;
    int trgmod;
    switch((PixScan::TriggerType)scn.getSrcTriggerType()){
    default:
    case PixScan::EXT_TRG:
      trgmod = 1;
      break;
    case PixScan::TLU_SIMPLE:
      trgmod = 2;
      break;
    case PixScan::TLU_DATA_HANDSHAKE:
      trgmod = 3;
      break;
    }
    m_USBreg->WriteRegister(CS_TRIGGER_MODE, trgmod); // select external trigger mode: 1:no handshake, 2:simple handshake, 3:trigger data handshake
    m_USBreg->WriteRegister(CS_COUNTER_MODE, scn.getSrcCountType()); // select event counter mode: 0:triggers, 1:hits, 2:EOE
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_DATA_LENGTH, 32); // set TLU trigger data length (depends on TLU bit-file)
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_DATA_DELAY, 0); // set additional wait cycles (depends on TLU trigger data length: 32 - CS_TLU_TRIGGER_DATA_LENGTH + additional delay = CS_TLU_TRIGGER_DATA_DELAY)
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_LOW_TIME_OUT, 20); // set time out for TLU trigger for not going in low state
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_DATA_MSB_FIRST, 0);
    m_USBreg->WriteRegister(CS_MINIMUM_TRIGGER_LENGTH, 0);
    if(UPC_DEBUG) cout<<"DEBUG: setting RJ45 to " << ((m_enableRJ45||m_testBeamFlag)?"ON":"OFF") << endl;
    m_USBreg->WriteRegister(CS_ENABLE_RJ45, ((m_enableRJ45||m_testBeamFlag) ? 1 : 0)); // if in testbeam mode, *always* turn RJ45 on; anything else is pointless
    m_srcCounterType = scn.getSrcCountType();
    m_sramReadoutReady = false;
    m_sramFull = false;
    m_tluVeto = false;
    m_measurementPause = false;
    m_measurementRunning = true;
    m_sramFillLevel = 0;
    m_triggerRate = 0;
    m_eventRate = 0;
    m_USBreg->StartMeasurement();
    m_scanThread = std::thread(&USBI3PixController::procSourceScan, this);// start SRAM processing in a thread
  }
}

void USBI3PixController::procSourceScan() {
  bool sramReadoutReady, sramFull, measurementPause, measurementRunning;
  int sramFillLevel=0;
  int NumberOfTriggers=0;
  int NumberOfHitWords=0;
  int NumberOfEOEWords=0;

  try{
    while(!m_SourceScanDone){
      m_USBreg->GetSourceScanStatus(sramReadoutReady, sramFull, measurementPause, measurementRunning, sramFillLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
      if(measurementRunning && sramFull && sramReadoutReady) { // JJ: readout during scan when SRAM is full
	if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: MeasurementRunning && SRAMFullSignal && SRAMReadoutReadySignal => reading SRAM..."<<endl;
	m_USBreg->ReadSRAM();
	makeSourceScanHistos();
	m_USBreg->WriteTurboDAQFromRawData(m_SourceFilename.c_str(), m_NewRawDataFile, false); // JJ: do not close file
	m_NewRawDataFile = false; // JJ: from now on append data to existing file, do not overwrite
	m_USBreg->ClearSRAM();
	m_SourceScanDone=false;
      } else if(!measurementRunning && sramReadoutReady) { // JJ: readout at the end of scan when desired number of hits is reached
	if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: !MeasurementRunning && SRAMReadoutReadySignal => reading SRAM..."<<endl;
	m_USBreg->ReadSRAM();
	makeSourceScanHistos();
	m_USBreg->WriteTurboDAQFromRawData(m_SourceFilename.c_str(), m_NewRawDataFile, true); // JJ: close file
	m_USBreg->ClearSRAM();
	m_SourceScanDone=true;
      }
      sleep(500);
    }
  }catch(...){
    m_scanExcept = std::current_exception();
  }
}

void USBI3PixController::stopScan() {

  //needed for reading SRAM:
  int NumberOfTriggers=0;
  int NumberOfHitWords=0;
  int NumberOfEOEWords=0;

  if(!m_SourceScanFlag) // normal scan
    m_FEconfig->StopuCScan(); 
  else { // source scan
    m_USBreg->StopMeasurement();
    if(UPC_DEBUG) cout<<"INFO USBPixCtrl: Source measurement stopped. Reading SRAM..."<<endl;
    do { // JJ: make sure all data is processed and written to SRAM, now ready for readout via fast USB interface
      m_USBreg->GetSourceScanStatus(m_sramReadoutReady, m_sramFull, m_measurementPause, m_measurementRunning, m_sramFillLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
    } while (m_measurementRunning || !m_sramReadoutReady);
    if (UPC_DEBUG) cout << "INFO USBPixCTRL: ReadSRAM()"<<endl;
    m_USBreg->ReadSRAM();
    if (UPC_DEBUG) cout << "INFO USBPixCTRL: makeSourceScanHistos()"<<endl;
    makeSourceScanHistos();
    if (UPC_DEBUG) cout << "INFO USBPixCTRL: WriteTurboDAQFromRawData()"<<endl;
    m_USBreg->WriteTurboDAQFromRawData(m_SourceFilename.c_str(), m_NewRawDataFile, true); // JJ: close file
    if (UPC_DEBUG) cout << "INFO USBPixCTRL: ClearSRAM()"<<endl;
    m_USBreg->ClearSRAM();
    m_USBreg->GetSourceScanStatus(m_sramReadoutReady, m_sramFull, m_measurementPause, m_measurementRunning, m_sramFillLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
    if(UPC_DEBUG) cout<<"INFO USBPixCtrl: read SRAMReadoutReadySignal: "<<(m_sramReadoutReady?"true":"false")<<" MeasurementRunning: "<<(m_measurementRunning?"true":"false")<<" NumberOfTriggers: "<<NumberOfTriggers<<endl;
    m_SourceScanDone=true;
    m_USBreg->WriteRegister(CS_TRIGGER_MODE, 0); // JJ: disable external trigger
    if(UPC_DEBUG) cout<<"DEBUG USBPixCtrl: set CS_TRIGGER_MODE to 0"<<endl;
  }
}

// ******* Histogram treatment on the board ******
bool USBI3PixController::fitHistos() {                                                 //! Fit collected histograms
  return false;
}

void USBI3PixController::getFitResults(HistoType /*type*/, unsigned int /*mod*/, unsigned int /*slv*/, 
				     std::vector< Histo * > &/*thr*/, std::vector< Histo * > &/*noise*/, std::vector< Histo * > &/*chi2*/) { //! Read a Fit from Dsp
}


bool USBI3PixController::getErrorHistos(unsigned int /*dsp*/, Histo* &/*his*/) {               //! Read collected error arrays 
  return false;
}


void USBI3PixController::clearSourceScanHistos() {

  if(UPC_DEBUG) cout<<"INFO: Entered clearSourceScanHistos()"<<endl;
  m_USBreg->ClearHitHistoFromRawData();
  m_USBreg->ClearHitTOTHistoFromRawData();
  m_USBreg->ClearHitLV1HistoFromRawData();
  m_USBreg->ClearClusterSizeHistoFromRawData();
  m_USBreg->ClearClusterTOTHistoFromRawData();
  m_USBreg->ClearClusterSeedTOTHistoFromRawData();
  m_USBreg->ClearClusterSeedLV1HistoFromRawData();
  m_USBreg->ClearClusterSeedHistoFromRawData();
  m_USBreg->ClearClusterSeedPerTriggerHistoFromRawData();
}

void USBI3PixController::makeSourceScanHistos() {

  if (UPC_DEBUG) cout << "INFO USBPixCTRL: Entered makeSourceScanHistos()"<<endl;
  if (UPC_DEBUG) cout << "INFO USBPixCTRL: MakeHitHistoFromRawData()"<<endl;
  m_USBreg->MakeHitHistoFromRawData();
  if (UPC_DEBUG) cout << "INFO USBPixCTRL: MakeHitTOTHistoFromRawData()"<<endl;
  m_USBreg->MakeHitTOTHistoFromRawData();
  if (UPC_DEBUG) cout << "INFO USBPixCTRL: MakeHitLV1HistoFromRawData()"<<endl;
  m_USBreg->MakeHitLV1HistoFromRawData();
  if (UPC_DEBUG) cout << "INFO USBPixCTRL: MakeClusterHistoFromRawData("<<m_clusterPars[0]<<", "<<
    m_clusterPars[1]<<", "<<m_clusterPars[2]<<", "<<m_clusterPars[3]<<", "<<((m_clusterPars[4]!=0)?"ON":"OFF")<<")"<<endl;
  // max. column cluster distance, max. row cluster distance, max. cluster BCID depth, 
  // max. threshold for timewalk, enable/disable advanced clustering
  m_USBreg->MakeClusterHistoFromRawData(m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], 
					m_clusterPars[3], (bool)m_clusterPars[4]); 
}

void USBI3PixController::getHisto(HistoType type, unsigned int mod, unsigned int /*slv*/, std::vector< std::vector<Histo*> > &his) {  //! Read an histogram

  if(UPC_DEBUG) cout << "DEBUG: USBI3PixController::getHisto()"<< endl;

  bool TOTmode=false;
  bool MadeHitHisto=false;
  bool MadeTOTHisto=false;
  bool MadeLVL1Histo=false;

  std::string scanname, scanname1;
  switch (type) {
  case OCCUPANCY :
    m_USBreg->SetCalibrationMode();
    //JW DEBUG: temporarily enabled raw data mode for debugging of source scan
//    m_USBreg->SetRunMode();
    TOTmode=false;
    scanname   = "Occup_";
    scanname1  = "Occupancy";
    break;
  case TOT_MEAN :
    m_USBreg->SetTOTMode();
    TOTmode=true;
    scanname   = "ToTMean_";
    scanname1  = "ToTMean";
    break;
  case TOT_SIGMA :
    m_USBreg->SetTOTMode();
    TOTmode=true;
    scanname   = "ToTSigma_";
    scanname1  = "ToTSigma";
    break;
  case LVL1 :
    scanname   = "LVL1_";
    scanname1  = "LVL1";
    break;
  case TOT:
    TOTmode=true;
    scanname   = "ToT_";
    scanname1  = "ToT";
    break;
  case TOTAVERAGE:
    TOTmode=true;
    scanname   = "ToTAvg_";
    scanname1  = "ToTAverage";
    break;
  case CLUSTER_TOT:
    scanname   = "ClToT_";
    scanname1  = "Cluster ToT";
    break;
  case CLUSTER_SIZE:
    scanname   = "ClSize_";
    scanname1  = "Cluster size";
    break;
  case CLSIZE_TOT:
    scanname   = "ClSizToT_";
    scanname1  = "Cluster size vs ToT";
    break;
  case HITOCC:
    scanname   = "HitOcc_";
    scanname1  = "Hit Occupancy (of cluster seeds)";
    break;
  case SEED_TOT:
    scanname   = "SdToT_";
    scanname1  = "Cluster-seed ToT";
    break;
  case SEED_LVL1:
    scanname   = "SdLVL1_";
    scanname1  = "Cluster-seed LVL1";
    break;
  case NSEEDS:
    scanname   = "Nseeds";
    scanname1  = "Number of cluster seeds per trigger";
    break;
 default: 
    break;
  };
  std::vector< Histo * > vh;    //make sure, the vector we  
  his.push_back(vh);            //want to write to exists

  if(mod!=0) mod = 0;             //need only treat one module at the moment
  his[mod].clear();               //clear vector for writing

  // read histo size from PixModule
  PixModule *pm = m_modGroup.module(mod);
  if(pm==0) return; // nothing to do
  unsigned int ncolm = pm->nColsMod();
  unsigned int nrowm = pm->nRowsMod();

// *** read histogram from SRAM ***
  m_USBreg->ReadSRAM();            //now the data is in ConfHisto[col][row][step]

  if(!m_SourceScanFlag && m_SourceScanDone) {
    if(type == OCCUPANCY||type==HITOCC) MadeHitHisto = true;
    if(TOTmode)                          MadeTOTHisto = true;
  }

  //JW: the scan configuration should be the one from the last scan, 
  //JW: i.e. the scan, whose data we're downloading
  int bins = m_scanConfigArray[8];
  
  if(UPC_DEBUG) cout << "USBI3PixController::getHisto: bins = " << bins << " MadeHitHisto = " << 
    (MadeHitHisto?"true":"false") << " - MadeLVL1Histo = " << (MadeLVL1Histo?"true":"false")<< endl;

  // *** now loop over the innermost scanloop, e.g. VCal ***
  for(int i=0; i<bins; i++) {
    std::ostringstream nam, tit;
    nam << scanname  << mod    << "_" << i;
    tit << scanname1 <<" mod " << mod << " bin " << i;
    Histo *h;
    //JW: NOTE ConfigRegisterI3 only supports single-chip histograms
    //JW:      stick to module format for the moment
    if (type == OCCUPANCY) {
      h = new Histo(nam.str(), tit.str(), (MadeHitHisto?8:1), ncolm, -0.5, -0.5+(double)ncolm, nrowm, -0.5, -0.5+(double)nrowm);
    }else if(type == TOTAVERAGE) {
      h = new Histo(nam.str(), tit.str(), nrowm*ncolm, -0.5, -0.5+(double)(nrowm*ncolm));
    }else if(type == LVL1 || type == CLUSTER_SIZE || type == SEED_LVL1 || type == NSEEDS) {
      h = new Histo(nam.str(), tit.str(), 16, -0.5, 15.5);
    }else if(type == TOT || type == CLUSTER_TOT || type == SEED_TOT) {
      h = new Histo(nam.str(), tit.str(), 256, -0.5, 255.5);
    }else if(type == CLSIZE_TOT) {
      h = new Histo(nam.str(), tit.str(), 8, 256, -0.5, 255.5, 16, -0.5, 15.5);
    }else {
      h = new Histo(nam.str(), tit.str(), 8, ncolm, -0.5, -0.5+(double)ncolm, nrowm, -0.5, -0.5+(double)nrowm);
    }

    int nr_hits = 0;
    int TOTsumsqr=0, TOTsum=0, hitsum=0, ToThis[256];
    double TOTsqravg=0, TOTavg=0, TOTsigma=0;
    for(int tot=0; tot<256; tot++) ToThis[tot]=0;
    if(type == OCCUPANCY || type==HITOCC || TOTmode) {
      // JW: histogram filling only for OCCUPANCY at the moment
      for(int col=0; col<18; col++){
	for(int row=0; row<160; row++) {
	  if(!TOTmode) { // conf mode
	    if(MadeHitHisto){
	      if(type == OCCUPANCY)
		m_USBreg->GetHitHistoFromRawData(col, row, nr_hits); // get entries from source scan hit histo, i.e. hitmap
	      else if(type==HITOCC)
		m_USBreg->GetClusterSeedHistoFromRawData(col, row, 0, nr_hits);
	    }else 
	      m_USBreg->GetConfHisto(col, row, i, nr_hits); // get entries from normal scan histos, JJ: Configuration parameter i starts from 0
	    
	    h->set(col, row, (double)nr_hits);
	    //	  if(nr_hits!=0) {
	    //	    cout<<dec<<"DEBUG USBPixCtrl: col="<<col<<"  \trow="<<row<<"  \tscanindex="<<i<<"  \tentries="<<nr_hits<<endl;
	    //	  }
	  } else { // ToT mode
	    TOTsumsqr=0;
	    TOTsum=0;
	    hitsum=0;
	    TOTsqravg=0;
	    TOTavg=0;
	    //	  cout<<"DEBUG USBPixCtrl: col "<<col<<"\t  row "<<row;
	    for(int tot=0; tot<256; tot++) {
	      // JJ: tot = 0 means ToT in hit word ist really 0...
	      if(MadeTOTHisto)
		m_USBreg->GetHitTOTHistoFromRawData(col, row, tot, nr_hits); // get entries from source scan ToT
	      else
		m_USBreg->GetTOTHisto(col, row, tot, nr_hits); // normal scan ToT
	      TOTsumsqr+=(nr_hits*tot*tot);
	      TOTsum+=(nr_hits*tot);
	      hitsum+=nr_hits;
	      ToThis[tot]+=nr_hits;
	    }
	    if(hitsum > 1) {
	      TOTsqravg = (double)TOTsumsqr/(double)hitsum;
	      TOTavg = (double)TOTsum/(double)hitsum;
	      TOTsigma = ((double)(TOTsqravg-TOTavg*TOTavg)/(double)(hitsum-1));
	    } else {
	      TOTsqravg = 0;
	      TOTavg = 0;
	      TOTsigma = 0;
	    }
	    //	  cout<<"\t  TOTMean="<<TOTavg;
	    if(scanname1=="ToTMean") {
	      h->set(col, row, (double)TOTavg);
	      //	    cout<<" written"<<endl;
	    }
	    else if(scanname1=="ToTSigma") {
	      h->set(col, row, (double)TOTsigma);
	      //	    cout<<"\t  TOTSigma="<<((TOTsqravg-TOTavg*TOTavg)/(hitsum+1))<<endl;
	    }
	    else if(scanname1=="Occupancy") {
	      h->set(col, row, (double)hitsum);
	    }
	  }
	}
      }
      if(type == TOT){
	for(int i=0;i<256;i++)h->set(i, (double) ToThis[i]);
      }
    }

    if(type==LVL1 || type==CLUSTER_SIZE || type==SEED_LVL1 || type==NSEEDS){
      int value;
      for(int i=0;i<16;i++){
	switch(type){
	case LVL1:
	  m_USBreg->GetHitLV1HistoFromRawData(i, value);
	  break;
	case CLUSTER_SIZE:
	  m_USBreg->GetClusterSizeHistoFromRawData(i, value);
	  break;
	case SEED_LVL1:
	  m_USBreg->GetClusterSeedLV1HistoFromRawData(i, 0, value);
	  break;
	case NSEEDS:
	  m_USBreg->GetClusterSeedPerTriggerHistoFromRawData(i, value);
	  break;
	default:
	  value = 0;
	}
	h->set(i, (double) value);
      }
    }

    if(type==CLSIZE_TOT){
      int value;
      for(int i=0;i<256;i++){
	for(int j=0;j<16;j++){
	  m_USBreg->GetClusterTOTHistoFromRawData(i, j, value); // j = 0: every cluster size
	  h->set(i, j, (double) value);
	}
      }
    }

    if(type==CLUSTER_TOT || type==SEED_TOT){
      for(int i=0;i<256;i++){
	int value;
	if(type==CLUSTER_TOT)
	  m_USBreg->GetClusterTOTHistoFromRawData(i, 0, value); // j = 0: every cluster size
	else
	  m_USBreg->GetClusterSeedTOTHistoFromRawData(i, 0, value);
	h->set(i, (double) value);
      }
    }

    his[mod].push_back(h);
    //JW: DO NOT DELETE h HERE! It is passed to the vector his not as pointer but as itself...
  }
}

//void USBI3PixController::setupHistogramming(int slave, PixScan &scn) {                 //JW: Setup für Histogramming auf dem Mikrocontroller?
//}

//void USBI3PixController::startHistogramming(int slave) {
//}

//void USBI3PixController::stopHistogramming(int slave) {
//}

// *********** Data Taking Control ***********
void USBI3PixController::writeRunConfig(PixRunConfig &/*cfg*/) {                           //! Get the run configuration parameters from PixModuleGroup
}

void USBI3PixController::startRun(int /*ntrig*/) {                                     //! Start a run
}

void USBI3PixController::stopRun() {                                                   //! Terminates a run
}

int  USBI3PixController::runStatus() {                                                 //! Check the status of the run
  int MaskStep;
  bool ScanBusy, ScanReady, ScanCanceled, EOEerror;
  int NumberOfTriggers=0;
  int NumberOfHitWords=0;
  int NumberOfEOEWords=0;
  if(!m_SourceScanFlag) { // normal scan
    m_USBreg->GetuCScanStatus(MaskStep, ScanBusy, ScanReady, ScanCanceled, EOEerror);
    if(m_SourceScanDone){
      m_SourceScanDone=false;
      if(ScanReady) return 1;
      else          return (int)ScanBusy;
    } else
      return (int)ScanBusy;
  } else {  // source scan, external trigger
    //m_USBreg->GetSourceScanStatus(SRAMReadoutReadySignal, SRAMFullSignal, MeasurementPauseSignal, MeasurementRunning, SRAMFillingLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
    m_USBreg->GetSourceScanStatus(m_sramReadoutReady, m_sramFull, m_measurementPause, m_measurementRunning, m_sramFillLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
    if(m_sramReadoutReady && m_SourceScanDone) {
      m_SourceScanFlag=false;
      m_USBreg->WriteRegister(CS_TRIGGER_MODE, 0); // disable external trigger
      return false;
    }
    else return true;
  }
}

int  USBI3PixController::nTrigger() {                                                  //! Returns the number of trigger processed so far
  int nMasks;
  bool ScanBusy, ScanReady, ScanCanceled, EOEerror;
  int NumberOfTriggers=0;
  int NumberOfHitWords=0;
  int NumberOfEOEWords=0;
  int retNTrigger=0;

  if(!m_SourceScanFlag) { // JW: Normal scan
    m_USBreg->GetuCScanStatus(nMasks, ScanBusy, ScanReady, ScanCanceled, EOEerror);
    //  cout << "Read nMasks = " << nMasks << " and ScanBusy: " << (ScanBusy?"true":"false")<< " ready: "<<(ScanReady?"true":"false")<< " canceled: "<<(ScanCanceled?"true":"false")<< endl;
    if(nMasks>0)  nMasks = nMasks << 12; // follow ROD conventions
    return nMasks;
  } else { // JW: Source scan, external trigger
    m_USBreg->GetSourceScanStatus(m_sramReadoutReady, m_sramFull, m_measurementPause, m_measurementRunning, m_sramFillLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
    if(UPC_DEBUG) cout<<"DEBUG: SRAMReadoutReadySignal: "<<(m_sramReadoutReady?"true":"false")<<" MeasurementRunning: "<<(m_measurementRunning?"true":"false")<<" NumberOfTriggers: "<<  NumberOfTriggers
		      <<", NumberOfHitWords: " << NumberOfHitWords << ", NumberOfEOEWords: " << NumberOfEOEWords <<endl;
    // not available on FW, so set here manually
    m_tluVeto = (m_sramFillLevel>m_sramReadoutAt) && m_testBeamFlag; 
    m_triggerRate= 0; // not available
    m_eventRate  = 0; // not available
    switch((PixScan::CountType)m_srcCounterType){
    default:
    case PixScan::COUNT_TRIGGER:
      retNTrigger = NumberOfTriggers;
      break;
    case PixScan::COUNT_DH:
      retNTrigger = NumberOfHitWords;
      break;
    case PixScan::COUNT_DR:
      retNTrigger = NumberOfEOEWords;
      break;
    }
    
    if(UPC_DEBUG) cout<<"DEBUG: sending No. triggers to application: " << retNTrigger << endl;
    return retNTrigger;
  }
  return -1;
}

bool USBI3PixController::getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram){
  const int data_size = SRAM_BYTESIZE/4;
  if(m_SourceScanFlag) {
    bool caseA = (m_measurementRunning && m_sramFull) || ((!m_measurementRunning || m_measurementPause) && forceReadSram)/* && m_sramReadoutReady*/;  // TODO
    bool caseB = !m_measurementRunning/* && m_sramReadoutReady*/ && !caseA; // TODO
    
    if(caseA || caseB) {
      if(UPC_DEBUG && caseA) std::cout << "DEBUG USBPixController::getSourceScanData : m_sramFull=" << (m_sramFull?"true":"false") <<
				   " || "<<"forceReadSram=" << (forceReadSram?"true":"false") << " => reading SRAM..." << std::endl;
      if(UPC_DEBUG && caseB) std::cout << "DEBUG USBPixController::getSourceScanData : m_measurementRunning=false => reading SRAM..." << std::endl;
    
      m_USBreg->ReadSRAM();
      // copy sram-data to array in testbeam mode
      if(m_testBeamFlag){
	if(UPC_DEBUG) std::cout << "DEBUG USBPixController::getSourceScanData : Copying data"<<std::endl;
	unsigned int *di = new unsigned int [data_size];
	for (int k = 0; k < data_size; k++) {
	  di[k] = m_USBreg->SRAMwordsRB[k];
	}
	data->push_back(di);
      }
      makeSourceScanHistos();
      m_USBreg->WriteTurboDAQFromRawData(m_SourceFilename.c_str(), m_NewRawDataFile, caseB);
      m_NewRawDataFile = false;
      m_USBreg->ClearSRAM();
      if(caseB) m_SourceScanDone=true;
      return true;
    } else //not (caseA || caseB) 
	return false;

  } else // no source scan
    return false;
}

void USBI3PixController::hwInfo(string &txt){
  txt = "";
  if(m_BoardHandle!=0){
    txt  = "Board Name:  " + string(m_BoardHandle->GetName()) + "\n";
    stringstream a;
    a << m_BoardHandle->GetId();
    txt += "Board ID: " + a.str() + "\n";
    stringstream b;
    b << m_BoardHandle->GetFWVersion();
    txt += "uC Firmware: " + b.str();
  }
  return;
}
void USBI3PixController::shiftPixMask(int mask, int /*cap*/, int steps){
  if(m_FEconfig!=0){
    int maskTags[4]={HITBUS, SELECT, MASK_B, KILL};
    for(int imask=0;imask<4;imask++){
      if(mask&(0x01<<imask)){ // shift only if mask is selected
	// call shift command
	m_FEconfig->ShiftPixMask(maskTags[imask], steps);
	// wait for command to finish
	//	PixLib::sleep(10);
      }
    }
  }
}
int USBI3PixController::readMonLeakADC(int mod, int ife, int nbits){
  // ignoring mod and fe input, always using 1 mod, 1 FE
  if(m_FEconfig!=0){
    // turn MonLeak measurement on
    m_FEconfig->SetGlobalVal(ENABLELEAKMEAS, 1);
    m_FEconfig->WriteGlobal();
    int mlcore=0, mlmin=0, mlmax;

    // measurement a la TurboDAQ
//   int Address, Size, rValue, mlVal;
//   const int compBit = 0x1000, mlStatus = 0x2000;
//     for(int ibit=nbits-1;ibit>=0;ibit--){
//       mlcore += (1<<ibit);
//       mlVal = (mlcore<<1)+compBit; // set to current test value plus comp. bit on
//       m_FEconfig->SetGlobalVal(MONLEAKADC, mlVal);
//       m_FEconfig->WriteGlobal();
//       m_FEconfig->ReadGlobal();
//       // read back and stop if status bit is off
//       m_FEconfig->GetGlobalRBVarAddVal(MONLEAKADC, Address, Size, rValue);
//       //cout << "MLtest: " << mlcore << " yields " << (rValue&mlStatus) << endl;
//       //if((rValue&mlStatus)==0) break;
//       if((rValue&mlStatus)==0) mlcore -= (1<<ibit);
//     }

    // iterative alternative
    // check min. val.
    if(!testMonLeakComp(mod,ife,mlmin)) return mlmin; // even 0 is above comparator, done
    // check max. val - try low value first for un-irradiated devices
    mlmax=((1<<nbits)-1)/4;
    if(testMonLeakComp(mod,ife,mlmax)){
      mlmax=(1<<nbits)-1;
      // check max. val
      if(testMonLeakComp(mod,ife,mlmax)) return mlmax; // even max. MonLeak is below comparator, done
    }
    mlcore = (mlmax+mlmin)/2;
    int nsteps=0;
    while((mlmax-mlmin)>1){
      if(testMonLeakComp(mod,ife,mlcore))
	mlmin = mlcore;
      else
	mlmax = mlcore;
      mlcore = (mlmax+mlmin)/2;
      nsteps++;
    }
    // turn MonLeak measurement off
    m_FEconfig->SetGlobalVal(ENABLELEAKMEAS, 0);
    m_FEconfig->SetGlobalVal(MONLEAKADC, 0);
    m_FEconfig->WriteGlobal();
    return mlmin;
  } else
    return 0;
}
bool USBI3PixController::testMonLeakComp(int /*mod*/, int ife, int MLguess){
  // ignoring mod and fe input, always using 1 mod, 1 FE
  int Address, Size, rValue;
  const int compBit = 0x1000, mlStatus = 0x2000;
  int mlVal = (MLguess<<1)+compBit; // set to current test value plus comp. bit on
  if(m_FEconfig!=0 || ife!=0){
    m_FEconfig->SetGlobalVal(MONLEAKADC, mlVal);
    m_FEconfig->SetGlobalVal(SELECTDO, 15);
    m_FEconfig->WriteGlobal();
    m_FEconfig->ReadGlobal();//Nowrite();
    // read back and stop if status bit is off
    m_FEconfig->GetGlobalRBVarAddVal(MONLEAKADC, Address, Size, rValue);
    return ((rValue&mlStatus)!=0);
  } else
    return false;
}
int USBI3PixController::readHitBusScaler(int /*mod*/, int ife, PixScan* scn){
  // ignoring mod and fe input, always using 1 mod, 1 FE
  int Address, Size, rValue;
  if(m_FEconfig!=0 || ife!=0){
    // call WriteGlobal to reset hitbus scaler and to set DO MUX to GR
    m_FEconfig->SetGlobalVal(SELECTDO, 15);
    //m_FEconfig->SetGlobalVal(HITBUSENABLE, 1);
    m_FEconfig->WriteGlobal();
    // send strobes with requested length, number and frequency
    // telling scaler to increment
    m_USBreg->WriteRegister(CS_LENGTH_STRB_0, (0xff & scn->getStrobeDuration()));
    m_USBreg->WriteRegister(CS_LENGTH_STRB_1, (0xff & (scn->getStrobeDuration() >> 8)));
    m_USBreg->WriteRegister(CS_LENGTH_LV1, 0);
    m_USBreg->WriteRegister(CS_DELAY_LV1, 0xff);
    m_USBreg->WriteRegister(CS_QUANTITY_STRB, (0xff & scn->getRepetitions()));
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_0, (0xff & (scn->getStrobeDuration()+100)));
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_1, (0xff & ((scn->getStrobeDuration()+100) >> 8)));
    m_USBreg->StartMeasurement();

    // waiting for FSM
    while (0 != m_USBreg->ReadRegister(CS_MEASUREMENT_START_STOP));

    // read back GR
    m_FEconfig->ReadGlobal();//Nowrite();
    // get HitBusScaler bits
    m_FEconfig->GetGlobalRBVarAddVal(HITBUSSCALER, Address, Size, rValue);
    if(UPC_DEBUG) cout << "Read from HitBus scaler: " << rValue << endl;
    return rValue;
  }

  return 0;
}
void USBI3PixController::updateDeviceHandle(){
  if (m_BoardHandle != 0) {
    void * tempHandle = GetUSBDevice(m_boardID);
    if(tempHandle!=0){
      m_BoardHandle->SetDeviceHandle(tempHandle);
      if(UPC_DEBUG) cout <<"INFO: found "<< m_BoardHandle->GetName() << " with ID " << m_BoardHandle->GetId() <<endl;
    } else{
      if(UPC_DEBUG) cout<<"WARNING: no board handle found..."<<endl;
      throw USBI3PixControllerExc(USBI3PixControllerExc::NOBOARD, PixControllerExc::WARNING, getCtrlName(), m_boardID); 
    }  
  }
}

void USBI3PixController::sendPixel(unsigned int moduleMask)
{
	if(UPC_DEBUG) cout<<"INFO USBPixCtrl: called sendPixel"<<endl;
	for (std::map<std::string, int>::iterator it = m_latchNames.begin(); it!=m_latchNames.end(); it++)
	  sendPixel(moduleMask, it->first, -1, false);
	sendGlobal(0);
}
void USBI3PixController::sendPixel(unsigned int moduleMask, std::string regName, bool /*allDcsIdentical*/) 
{
	if(UPC_DEBUG) cout<<"INFO USBPixCtrl: called sendPixel for reg. "<< regName<<endl;
	sendPixel(moduleMask, regName, -1, true);
}
void USBI3PixController::sendPixel(unsigned int moduleMask, std::string regName, int /*DC*/)
{
	if(UPC_DEBUG) cout<<"INFO USBPixCtrl: called sendPixel for reg. "<< regName << endl;
	sendPixel(moduleMask, regName, -1, true);
}
void USBI3PixController::sendPixel(unsigned int /*moduleMask*/, std::string regName, int /*DC*/, bool sendGlob)
{
  if(UPC_DEBUG) cout<<"INFO USBPixCtrl: called sendPixel for reg. "<< regName  << " with sendGlobal=" << (sendGlob?"true":"false")<<endl;

  if(m_FEconfig!=NULL && m_configValid && m_latchNames.find(regName)!=m_latchNames.end()){
    // offset needed to determine FE_WRITE_xxx enum for 1st argument
    int offset = m_latchNames[regName] - HITBUS;
    int shift_by_nbits = (int)(log((float)FE_WRITE_HITBUS)/log(2.f));
      m_FEconfig->WritePixel((1<<(shift_by_nbits+offset)), m_latchNames[regName]);
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
  }

  if(sendGlob) sendGlobal(0);
}
void USBI3PixController::sendGlobal(unsigned int /*moduleMask*/)
{
  if(UPC_DEBUG) cout<<"INFO USBPixCtrl: called general sendGlobal "<<endl;

	if(m_FEconfig!=NULL && m_configValid){
	  m_FEconfig->WriteGlobal();
	} else {
		throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
	}
}
void USBI3PixController::sendGlobal(unsigned int moduleMask, std::string /*regName*/)
{
  // not possible to address individual global registers for FE-I3, so always call other sendGLoabl
  if(m_FEconfig!=NULL && m_configValid)
    sendGlobal(moduleMask);
  else 
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
}
void USBI3PixController::setFERunMode() 
{
  // doesn't exist for FE-I3
}
void USBI3PixController::setFEConfigurationMode()
{
  // doesn't exist for FE-I3
}
void USBI3PixController::pauseRun() 
{
  if(m_SourceScanFlag) {
    if(UPC_DEBUG) cout<<"DEBUG USBI3PixController: pausing by calling m_USBreg->PauseMeasurement()"<<endl;
    m_USBreg->PauseMeasurement();
    do { // make sure we are actually in pause state
      int NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords;
      m_USBreg->GetSourceScanStatus(m_sramReadoutReady, m_sramFull, m_measurementPause, m_measurementRunning, m_sramFillLevel, NumberOfTriggers, NumberOfHitWords, NumberOfEOEWords);
    } while (!m_measurementPause);
  }
}
void USBI3PixController::resumeRun() 
{
  if(m_SourceScanFlag) {
    if(UPC_DEBUG) cout<<"DEBUG USBI3PixController: resuming by calling m_USBreg->StartMeasurement()"<<endl;
    m_USBreg->ResumeMeasurement();
  }
}
 bool USBI3PixController::checkRxState(rxTypes /*type*/)
{/*
	bool retval = false;
	switch(type){
  case RX0:
	  retval = m_USBpix->CheckRX0State();
	  break;
  case RX1:
	  retval = m_USBpix->CheckRX1State();
	  break;
  case RX2:
	  retval = m_USBpix->CheckRX2State();
	  break;
  case EXT_TRG:
	  retval = m_USBpix->CheckExtTriggerState();
	  break;
  default:
	  break;
	}
	//if(UPC_DEBUG_GEN) cout<<"USBPixController::checkRxState returns " << (retval?"TRUE":"FALSE") << endl;
	return retval;
	*/
return false;
}
bool USBI3PixController::getGenericBuffer(const char *type, std::string &textBuf) {
  textBuf="";
  if(strcmp(type,"err") == 0){
    if(m_errBuff.empty()) return false;
    textBuf = m_errBuff;
    m_errBuff = "";
    return true;
  } else if(strcmp(type,"info") == 0){
    if(m_infoBuff.empty()) return false;
    textBuf = m_infoBuff;
    m_infoBuff = "";
    return true;
  } else
    return false;
}
