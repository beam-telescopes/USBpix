/////////////////////////////////////////////////////////////////////
// USBPixController.cxx
/////////////////////////////////////////////////////////////////////
//
// 10/11/08  Version 0.0
// 03/09/09  Version 1.0 for USBPix tutorial
// Author: Jens Weingarten
//          
// This is the PixController class for the Bonn USB system.
//


#include <string.h>
#include <string>

using namespace std;

#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "Config/Config.h"

#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFeI2.h"
#include "PixDcs/SleepWrapped.h"

#include "USBI3PixController.h"
#include "ConfigRegisterI3.h"
#include "ConfigFEMemoryI3.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "SiLibUSB.h"

#include <sstream>
#include <fstream>

#define UPC_DEBUG false

using namespace PixLib;

// UINT32 bitflip(UINT32 bits_in){
//   UINT32 bits_out=0;
//   for(int k=0;k<32;k++){
//     if(k<16)
//       bits_out |= (bits_in&(1<<k))<<(31-2*k);
//     else
//       bits_out |= (bits_in&(1<<k))>>(2*k-31);
//   }
//   return bits_out;
// }

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

  m_MaskStep = 0;
  m_ScanBusy = false;
  m_ScanReady = true;
  m_ScanCanceled = false;
  m_ScanError = false;

  m_SRAMReadoutReadySignal = false;
  m_SRAMFullSignal = false;
  m_MeasurementPauseSignal = false;
  m_MeasurementRunningSignal = false;
  m_SRAMFillLevel = 0;
  m_NumberOfTriggers = 0;
  m_NumberOfHitWords = 0;
  m_TriggerRate = 0;
  m_HitRate = 0;
}

USBI3PixController::USBI3PixController(PixModuleGroup &modGrp) : 
PixController(modGrp) { //! Constructor
  configInit();

  //  m_FPGAconfig = NULL;
  m_FEconfig = NULL;
  m_BoardHandle = NULL;
  m_USBreg = 0;
  m_SRAMReadoutReadySignal = false;
  m_SRAMFullSignal = false;
  m_MeasurementPauseSignal = false;
  m_MeasurementRunningSignal = false;
  m_SRAMFillLevel = 0;
  m_NumberOfTriggers = 0;
  m_NumberOfHitWords = 0;
  m_TriggerRate = 0;
  m_HitRate = 0;
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
			    "FPGA firmware file name", true, 1);
  conf["general"].addString("uCFirmwareFile", m_uC_filename, "",
			    "micro controller firmware file name (if empty it is not overwritten)", true, 1);
  conf["general"].addInt("RegSyncDel", m_regSyncDel, 0,
    "Synchronisation phase adjust (0: 0deg. , 1: 180deg.) for register test read-back", true);
  conf["general"].addBool("enableRJ45", m_enableRJ45, false,
    "Enable RJ45 input during scans", true);
  std::map<std::string, int> triggRepList;
  triggRepList["Disabled"]  = REPLICATION_OFF;
  triggRepList["Master"] = REPLICATION_MASTER;
  triggRepList["Slave"] = REPLICATION_SLAVE;
  conf["general"].addList("TriggerReplication", m_triggerReplication, REPLICATION_OFF, triggRepList,
    "Role of USBpix Board for Trigger Replication", true);

  // Select default values
  conf.reset();

  // translation PixFe -> ConfigFEMemory
  m_latchNames["HITBUS"]  = HITBUS+1; // must add 1, since HITBUS=0 would spoil identification of wrong names
  m_latchNames["SELECT"]  = SELECT+1;
  m_latchNames["ENABLE"]  = MASK_B+1;
  m_latchNames["PREAMP"]  = KILL+1;
  m_latchNames["TDAC0"]   = TDAC0+1;
  m_latchNames["TDAC1"]   = TDAC1+1;
  m_latchNames["TDAC2"]   = TDAC2+1;
  m_latchNames["TDAC3"]   = TDAC3+1;
  m_latchNames["TDAC4"]   = TDAC4+1;
  m_latchNames["TDAC5"]   = TDAC5+1;
  m_latchNames["TDAC6"]   = TDAC6+1;
  m_latchNames["FDAC0"]   = FDAC0+1;
  m_latchNames["FDAC1"]   = FDAC1+1;
  m_latchNames["FDAC2"]   = FDAC2+1;

  m_cmdNames["HITBUS"]  = FE_WRITE_HITBUS;
  m_cmdNames["SELECT"]  = FE_WRITE_SELECT;
  m_cmdNames["ENABLE"]  = FE_WRITE_MASK_B;
  m_cmdNames["PREAMP"]  = FE_WRITE_KILL;
  m_cmdNames["TDAC0"]   = FE_WRITE_TDAC0;
  m_cmdNames["TDAC1"]   = FE_WRITE_TDAC1;
  m_cmdNames["TDAC2"]   = FE_WRITE_TDAC2;
  m_cmdNames["TDAC3"]   = FE_WRITE_TDAC3;
  m_cmdNames["TDAC4"]   = FE_WRITE_TDAC4;
  m_cmdNames["TDAC5"]   = FE_WRITE_TDAC5;
  m_cmdNames["TDAC5"]   = FE_WRITE_TDAC6;
  m_cmdNames["FDAC0"]   = FE_WRITE_FDAC0;
  m_cmdNames["FDAC1"]   = FE_WRITE_FDAC1;
  m_cmdNames["FDAC2"]   = FE_WRITE_FDAC2;

  m_configValid = false;
}

// ***************** Board functionality **************
void USBI3PixController::initHW() {

  int chip_address = 0;

  if(UPC_DEBUG){
    cout<<"myINFO: FPGA_filename="<<m_FPGA_filename<<endl;
    cout<<"myINFO: uC_filename="<<m_uC_filename<<endl;
  }

  if(m_BoardHandle==0){
    // dealt with outside of controller now
    //    if(UPC_DEBUG) cout<<"myINFO: calling InitUSB " << endl;
    //    InitUSB();
    m_BoardHandle = new SiUSBDevice(0); // JJ: every "new" needs a "delete"! eh?
    //    cout<<"myINFO: m_BoardHandle = "<<m_BoardHandle<<endl;
  }

  if (m_BoardHandle != 0) {

    // Create FE config object
    if(m_FEconfig==NULL){
      m_FEconfig = new ConfigFEMemoryI3(chip_address,0, m_BoardHandle); // JJ: every "new" needs a "delete"! eh?
      m_USBreg = new ConfigRegisterI3(m_BoardHandle); // JJ: every "new" needs a "delete"! eh?
    }

    updateDeviceHandle();
    // write uC firmware
    if(m_uC_filename!=""){
      if(UPC_DEBUG) cout<<"Processing uC firmware" << endl;
      FILE *g=0;
#ifdef CF__LINUX
      g = fopen(m_uC_filename.c_str(),"r");
#else
      fopen_s(&g, m_uC_filename.c_str(),"r");
#endif
      if(g==0){
        if(UPC_DEBUG) cout<<"ERROR: uC bix file doesn't exist"<<endl;
	throw USBI3PixControllerExc(USBI3PixControllerExc::FILE_ERROR, PixControllerExc::FATAL, getCtrlName()); 
      } else
        fclose(g);
      if(!m_BoardHandle->LoadFirmwareFromFile(m_uC_filename.c_str())){
        if(UPC_DEBUG) cout<<"ERROR: uC didn't configure"<<endl;
        throw USBI3PixControllerExc(USBI3PixControllerExc::UC_ERROR, PixControllerExc::FATAL, getCtrlName()); 
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
    FILE *f = 0;
#ifdef CF__LINUX
    f = fopen(m_FPGA_filename.c_str(),"r");
#else
    fopen_s(&f, m_FPGA_filename.c_str(),"r");
#endif
    if(f==0){
      if(UPC_DEBUG) cout<<"ERROR: FPGA bit file " << m_FPGA_filename <<" doesn't exist"<<endl;
      throw USBI3PixControllerExc(USBI3PixControllerExc::FILE_ERROR, PixControllerExc::FATAL, getCtrlName()); 
    } else
      fclose(f);
    if(!m_BoardHandle->DownloadXilinx(m_FPGA_filename.c_str())){
      if(UPC_DEBUG) cout<<"ERROR: FPGA didn't configure"<<endl;
      throw USBI3PixControllerExc(USBI3PixControllerExc::FPGA_ERROR, PixControllerExc::FATAL, getCtrlName()); 
    }
    // set phase register
    m_USBreg->WriteRegister(CS_XCK_PHASE, m_regSyncDel);
  }else{
    if(UPC_DEBUG) cout<<"ERROR: no SiUSBDevice found..."<<endl;
    throw USBI3PixControllerExc(USBI3PixControllerExc::NOHANDLE, PixControllerExc::FATAL, getCtrlName()); 
    m_FEconfig = NULL;
    m_USBreg = 0;
  }
  // refresh readback ID
  m_boardIDRB = m_BoardHandle->GetId();
}

void USBI3PixController::testHW() {
  if(m_BoardHandle!=0){
    if(UPC_DEBUG) cout <<"INFO: found "<< m_BoardHandle->GetName() << " with ID " << m_BoardHandle->GetId() <<endl;
  }else
    throw USBI3PixControllerExc(USBI3PixControllerExc::NOBOARD, PixControllerExc::FATAL, getModGroup().getRodName()); 
}

void USBI3PixController::sendCommand(Bits commands, int moduleMask){
}
void USBI3PixController::sendCommand(int command, int moduleMask) {

  if(command==PixModuleGroup::PMG_CMD_ECR){
    m_FEconfig->SendRefReset();
  }
  if(command==PixModuleGroup::PMG_CMD_BCR){
    m_FEconfig->SendSoftReset();
  }
  if(command==PixModuleGroup::PMG_CMD_SRESET){
    int sync_length = 16;
    m_USBreg->WriteSync(sync_length);
  }
  if(command==PixModuleGroup::PMG_CMD_TRIGGER){
    m_USBreg->WriteRegister(CS_LENGTH_STRB_0, 0xff);
    m_USBreg->WriteRegister(CS_LENGTH_STRB_1, 0x01);
    m_USBreg->WriteRegister(CS_LENGTH_LV1, 0x03);
    m_USBreg->WriteRegister(CS_DELAY_LV1, 0x05);
    m_USBreg->WriteRegister(CS_QUANTITY_STRB, 1);
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_0, 0xff);
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_1, 0x04);
    m_USBreg->StartMeasurement();
  }
}

void USBI3PixController::sendCommandOutSync(Bits commands, int moduleMask, std::vector<Bits>& output) {
  //JW: not used
}

void USBI3PixController::sendCommandOutAsync(Bits commands, int moduleMask, int commandID) {
  //JW: not used
}

bool USBI3PixController::checkOutReady(int commandID) {
  //JW: not used
  return false;
}

void USBI3PixController::getOutput(int commandID, std::vector<Bits>& output) {
  //JW: not used
}

void USBI3PixController::downloadFifo(int size) {
}

bool USBI3PixController::primitiveExists(std::string str) { 
  return false;
}

// ************ Pixel module functions ****************

void USBI3PixController::writeModuleConfig(PixModule& mod) {

  //int moduleId = mod.moduleId(); // do we need this anywhere?

  // check consistency of config.
  int nFe=0;
  for(std::vector<PixFe*>::iterator fe = mod.feBegin(); fe != mod.feEnd(); fe++) nFe++;
  
  PixFe *fe = *mod.feBegin(); // should have only one FE; check first in list
  if(dynamic_cast<PixFeI2*>(fe)==0 && nFe!=1)
    throw USBI3PixControllerExc(USBI3PixControllerExc::INCONS_FE_CFG, PixControllerExc::FATAL, getCtrlName()); 
  
  try{
    
    // copy FE global register content
    //int number = fe->number();
    m_FEconfig->SetChipAdd(((ConfInt&)fe->config()["Misc"]["Address"]).getValue());
    
    // fill global register structures of m_FEconfig one by one
    m_FEconfig->SetGlobalVal(CHIP_LATENCY, fe->readGlobRegister("LATENCY"));
    m_FEconfig->SetGlobalVal(GLOBALPARITY, 1);
    m_FEconfig->SetGlobalVal(IVDD2DAC, fe->readGlobRegister("DAC_IVDD2"));
    m_FEconfig->SetGlobalVal(IP2DAC, fe->readGlobRegister("DAC_IP2"));
    m_FEconfig->SetGlobalVal(IDDAC, fe->readGlobRegister("DAC_ID"));
    m_FEconfig->SetGlobalVal(IPDAC, fe->readGlobRegister("DAC_IP"));
    m_FEconfig->SetGlobalVal(ITRIMTHDAC, fe->readGlobRegister("DAC_ITRIMTH"));
    m_FEconfig->SetGlobalVal(IFDAC, fe->readGlobRegister("DAC_IF"));
    m_FEconfig->SetGlobalVal(ITH1DAC, fe->readGlobRegister("DAC_ITH1"));
    m_FEconfig->SetGlobalVal(ITH2DAC, fe->readGlobRegister("DAC_ITH2"));
    m_FEconfig->SetGlobalVal(ILDAC, fe->readGlobRegister("DAC_IL"));
    m_FEconfig->SetGlobalVal(IL2DAC, fe->readGlobRegister("DAC_IL2"));
    m_FEconfig->SetGlobalVal(ITRIMIFDAC, fe->readGlobRegister("DAC_ITRIMIF"));
    m_FEconfig->SetGlobalVal(THRMIN, fe->readGlobRegister("THRESH_TOT_MINIMUM"));
    m_FEconfig->SetGlobalVal(THRDUB, fe->readGlobRegister("THRESH_TOT_DOUBLE"));
    m_FEconfig->SetGlobalVal(CAPMEASCIRCUIT, fe->readGlobRegister("CAP_MEASURE"));
    m_FEconfig->SetGlobalVal(GLOBALTDAC, fe->readGlobRegister("GLOBAL_DAC"));
    m_FEconfig->SetGlobalVal(TESTPIXELMUX, fe->readGlobRegister("MUX_TEST_PIXEL"));
    m_FEconfig->SetGlobalVal(VCALDAC, fe->readGlobRegister("DAC_VCAL"));
    m_FEconfig->SetGlobalVal(SELFTRGWIDTH, fe->readGlobRegister("SELF_WIDTH"));
    m_FEconfig->SetGlobalVal(SELFTRGDELAY, fe->readGlobRegister("SELF_LATENCY"));
    m_FEconfig->SetGlobalVal(ENABLESELFTRG, fe->readGlobRegister("ENABLE_SELF_TRIGGER"));
    m_FEconfig->SetGlobalVal(SELECTDO, fe->readGlobRegister("MUX_DO"));
    m_FEconfig->SetGlobalVal(SELECTMONHIT, fe->readGlobRegister("MUX_MON_HIT"));
    m_FEconfig->SetGlobalVal(EOCMUXCONTROL, fe->readGlobRegister("MUX_EOC"));
    m_FEconfig->SetGlobalVal(CEUCLOCKCONTROL, fe->readGlobRegister("FREQUENCY_CEU"));
    m_FEconfig->SetGlobalVal(READMODE,  fe->readGlobRegister("MODE_TOT_THRESH"));
    m_FEconfig->SetGlobalVal(TSITSCENABLE, fe->readGlobRegister("ENABLE_TIMESTAMP"));
    fe->readGlobRegister("SPARE");
    int ml1 = fe->readGlobRegister("DAC_MON_LEAK_ADC");
    int ml2 = fe->readGlobRegister("MON_ADC_REF");
    int ml3 = fe->readGlobRegister("ENABLE_MON_LEAK");
    int ml4 = fe->readGlobRegister("MON_MON_LEAK_ADC");
    m_FEconfig->SetGlobalVal(MONLEAKADC, ml2 + (ml1<<1) + (ml4<<11) + (ml3<<12));
    m_FEconfig->SetGlobalVal(ENABLEEOEPARITY, 0);
    m_FEconfig->SetGlobalVal(SELECTDATAPHASE, fe->readGlobRegister("STATUS_MON_LEAK"));
    m_FEconfig->SetGlobalVal(ENABLECAPTEST, fe->readGlobRegister("ENABLE_CAP_TEST"));
    m_FEconfig->SetGlobalVal(ENABLEANALOGOUT, fe->readGlobRegister("ENABLE_BUFFER"));
    m_FEconfig->SetGlobalVal(ENABLEVCALMEAS, fe->readGlobRegister("ENABLE_VCAL_MEASURE"));
    m_FEconfig->SetGlobalVal(ENABLELEAKMEAS, fe->readGlobRegister("ENABLE_LEAK_MEASURE"));
    m_FEconfig->SetGlobalVal(ENABLEBUFFERBOOST, fe->readGlobRegister("ENABLE_BUFFER_BOOST"));
    m_FEconfig->SetGlobalVal(ENABLECOL8, fe->readGlobRegister("ENABLE_CP8"));
    m_FEconfig->SetGlobalVal(TESTDACIVDD2, fe->readGlobRegister("MON_IVDD2"));
    m_FEconfig->SetGlobalVal(TESTDACID, fe->readGlobRegister("MON_ID"));
    m_FEconfig->SetGlobalVal(ENABLECOL7, fe->readGlobRegister("ENABLE_CP7"));
    m_FEconfig->SetGlobalVal(TESTDACIP2, fe->readGlobRegister("MON_IP2"));
    m_FEconfig->SetGlobalVal(TESTDACIP, fe->readGlobRegister("MON_IP"));
    m_FEconfig->SetGlobalVal(ENABLECOL6, fe->readGlobRegister("ENABLE_CP6"));
    m_FEconfig->SetGlobalVal(TESTDACITRIMTH, fe->readGlobRegister("MON_ITRIMTH"));
    m_FEconfig->SetGlobalVal(TESTDACIF, fe->readGlobRegister("MON_IF"));
    m_FEconfig->SetGlobalVal(ENABLECOL5, fe->readGlobRegister("ENABLE_CP5"));
    m_FEconfig->SetGlobalVal(TESTDACITRIMIF, fe->readGlobRegister("MON_ITRIMIF"));
    m_FEconfig->SetGlobalVal(TESTDACVCAL, fe->readGlobRegister("MON_VCAL"));
    m_FEconfig->SetGlobalVal(ENABLECOL4, fe->readGlobRegister("ENABLE_CP4"));
    m_FEconfig->SetGlobalVal(HITINJECTCAPSEL, fe->readGlobRegister("ENABLE_CINJ_HIGH"));
    m_FEconfig->SetGlobalVal(ENABLEEXTINJ, fe->readGlobRegister("ENABLE_EXTERNAL"));
    m_FEconfig->SetGlobalVal(TESTANALOGREF, fe->readGlobRegister("ENABLE_TEST_ANALOG_REF"));
    m_FEconfig->SetGlobalVal(ENABLEDIGITALINJ, fe->readGlobRegister("ENABLE_DIGITAL"));
    m_FEconfig->SetGlobalVal(ENABLECOL3, fe->readGlobRegister("ENABLE_CP3"));
    m_FEconfig->SetGlobalVal(TESTDACITH1, fe->readGlobRegister("MON_ITH1"));
    m_FEconfig->SetGlobalVal(TESTDACITH2, fe->readGlobRegister("MON_ITH2"));
    m_FEconfig->SetGlobalVal(ENABLECOL2, fe->readGlobRegister("ENABLE_CP2"));
    m_FEconfig->SetGlobalVal(TESTDACIL, fe->readGlobRegister("MON_IL"));
    m_FEconfig->SetGlobalVal(TESTDACIL2, fe->readGlobRegister("MON_IL2"));
    m_FEconfig->SetGlobalVal(ENABLECOL1, fe->readGlobRegister("ENABLE_CP1"));
    m_FEconfig->SetGlobalVal(ENABLECOL0, fe->readGlobRegister("ENABLE_CP0"));
    m_FEconfig->SetGlobalVal(HITBUSENABLE, fe->readGlobRegister("ENABLE_HITBUS"));
    m_FEconfig->SetGlobalVal(ENABLETUNE, fe->readGlobRegister("ENABLE_AUTOTUNE"));
    m_FEconfig->SetGlobalVal(ENABLEBIASCOMP, fe->readGlobRegister("ENABLE_BIASCOMP"));
    m_FEconfig->SetGlobalVal(ENABLEHITPARITY, 0);      
    m_FEconfig->SetGlobalVal(HITBUSSCALER, 0);// is read-only
    m_FEconfig->SetGlobalVal(AREGTRIM, 1);
    m_FEconfig->SetGlobalVal(ENABLEAREGMEAS, 0);
    m_FEconfig->SetGlobalVal(AREGMEAS, 0);
    m_FEconfig->SetGlobalVal(ENABLEAREG, 0);
    m_FEconfig->SetGlobalVal(ENABLELVDSREFMEAS, 0);
    m_FEconfig->SetGlobalVal(DREGTRIM, 1);
    m_FEconfig->SetGlobalVal(ENABLEDREGMEAS, 0);
    m_FEconfig->SetGlobalVal(DREGMEAS, 0);
    m_FEconfig->SetGlobalVal(ENABLEIPMONITOR, 0);
    
    // ********* GlobalRegister done! ***************
    
    // ********* Copy FE pixel register content ***************
    std::string pixName[4];
    pixName[0]="ENABLE"; 
    pixName[1]="SELECT"; 
    pixName[2]="PREAMP"; 
    pixName[3]="HITBUS"; 
    
    // loop over individual pixel register bits and fill m_USBpix
    ConfMask<unsigned short int> &tdreg = fe->readTrim("TDAC");
    int nCol = tdreg.get().size();
    int nRow = tdreg[0].size();
    const int nlatches = 14, itdac=4, ifdac=11, nWords=nCol*nRow/32;
    int *trimBits;
    trimBits = new int[nWords];
    std::string lName;

    for(int i=0;i<nlatches;i++){
      int bit_offs = i;
      std::stringstream a;
      if(i>=ifdac){ //FDAC starts here
	bit_offs = i-ifdac; // start again from 0
	a << bit_offs;
	lName = "FDAC"+a.str();
      }else if(i>=itdac){ //TDAC starts here
	bit_offs = i-itdac; // start again from 0
	a << bit_offs;
	lName = "TDAC"+a.str();
      } else{
	lName = pixName[i];
      }


      for(int j=0;j<nWords;j++) trimBits[j] = 0;
      
      for(int col=0; col < nCol; col++){
        for(int row=0; row < nRow; row++){
          // determine pixel index as used in dll
          int pixnr = 0;
          int pixint = 0;
          if (0x01 & col)
            pixnr = (col-1) * nRow + nRow - row - 1;
          else
            pixnr = (col+1) * nRow + row;
          pixint = (pixnr / 32);
          int k = 31-(pixnr%32);
          // merge pixel register bits into 32-bit words
          if(i>=ifdac){ //FDAC starts here
            trimBits[pixint] |= (int)(((fe->readTrim("FDAC")[col][row]>>bit_offs)&0x1)<<k);
          }else if(i>=itdac){ //TDAC starts here
            trimBits[pixint] |= (int)(((fe->readTrim("TDAC")[col][row]>>bit_offs)&0x1)<<k);
          }else{         //masks
            trimBits[pixint] |= ((int)fe->readPixRegister(pixName[i])[col][row])<<k;
          }
        }
      }
      
      // store in register structures of m_USBpix
      for(int j=0;j<nWords;j++)
	m_FEconfig->SetPixelVal(92-j, trimBits[j], m_latchNames[lName]);

    }
    delete[] trimBits;

  } catch(...){
    throw USBI3PixControllerExc(USBI3PixControllerExc::NOT_IMPLEMENTED, PixControllerExc::FATAL, getCtrlName()); 
  }
    
  // pixel register done
  m_configValid = true;
}

void USBI3PixController::readModuleConfig(PixModule& ){//mod) {
  if(m_FEconfig!=NULL) {
    m_FEconfig->ReadGlobal();
  }
}

void USBI3PixController::sendModuleConfig(unsigned int moduleMask) {

  if(m_FEconfig!=NULL && m_configValid){
    m_FEconfig->WriteGlobal();
    m_FEconfig->WritePixel();
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::FATAL, getCtrlName()); 
  }
}

void USBI3PixController::sendModuleConfig(PixModule& mod){

  writeModuleConfig(mod);
  if(m_FEconfig!=NULL && m_configValid) {
    m_FEconfig->WriteGlobal();
    m_FEconfig->WritePixel();
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::FATAL, getCtrlName()); 
  }

}

void USBI3PixController::sendPixel(unsigned int moduleMask){
  if(m_FEconfig!=NULL && m_configValid) {
    m_FEconfig->WritePixel();
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::FATAL, getCtrlName());
  }
}
void USBI3PixController::sendGlobal(unsigned int moduleMask){
  if(m_FEconfig!=NULL && m_configValid) {
    m_FEconfig->WriteGlobal();
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::FATAL, getCtrlName());
  }
}
void USBI3PixController::sendPixel(unsigned int moduleMask, std::string regName, bool ){
  int latch = m_latchNames[regName]-1;
  int command = m_cmdNames[regName];
  if(m_FEconfig!=NULL && m_configValid && latch>=0 && command>=0) {
    m_FEconfig->WritePixel(command, latch);
  } else {
    throw USBI3PixControllerExc(USBI3PixControllerExc::INIT_ERROR, PixControllerExc::FATAL, getCtrlName()); 
  }
}
void USBI3PixController::sendPixel(unsigned int moduleMask, std::string regName, int ){
  sendPixel(moduleMask, regName); // FE-I3 can't configure individual DCs
}
void USBI3PixController::sendGlobal(unsigned int moduleMask, std::string ){
  sendGlobal(moduleMask); // FE-I3 can't send individual GR values
}
int  USBI3PixController::readGlobal(int regNo, int ){
  m_FEconfig->ReadGlobal();
  // to do: look for register
  return 0;
}

bool USBI3PixController::testGlobalRegister(int module, std::vector<int> &data_in, std::vector<int> &data_out, 
					    std::vector<std::string> &label, bool sendCfg, int feIndex){

  if(module!=0 || feIndex!=0) return false; // can deal with just one FE!

  bool retVal = false;
  if(sendCfg) sendGlobal(0);

  if(m_FEconfig!=0 && m_configValid){
    m_FEconfig->ReadGlobal();
    retVal = true;
    int Address, Size, wValue, rValue;
    for(int i=4;i<78;i++){
      m_FEconfig->GetGlobalVarAddVal(i, Address, Size, wValue);
      m_FEconfig->GetGlobalRBVarAddVal(i, Address, Size, rValue);
      stringstream a;//,b;
      if(i!=MONLEAKADC && i!=SELECTDO && i!=HITBUSSCALER){
        if(wValue!=rValue) retVal = false;
	data_in.push_back(wValue);
	data_out.push_back(rValue);
	a << (i-4);
	label.push_back("Reg "+a.str());
      }
    }
  }

  return retVal;
}
bool USBI3PixController::testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out,
					   bool, int , bool sendCfg, int feIndex, bool ){

  if(module!=0 || feIndex!=0) return false; // can deal with just one FE!

  bool retVal = false;

  if(sendCfg){
    sendPixel(0, regName);
    sendGlobal(0);
  }

  map<string, int> latchNames, cmdNames;
  latchNames.insert(make_pair("HITBUS",HITBUS));
  latchNames.insert(make_pair("PREAMP",KILL));
  latchNames.insert(make_pair("SELECT",SELECT));
  latchNames.insert(make_pair("ENABLE",MASK_B));
  latchNames.insert(make_pair("TDAC0",TDAC0));
  latchNames.insert(make_pair("TDAC1",TDAC1));
  latchNames.insert(make_pair("TDAC2",TDAC2));
  latchNames.insert(make_pair("TDAC3",TDAC3));
  latchNames.insert(make_pair("TDAC4",TDAC4));
  latchNames.insert(make_pair("TDAC5",TDAC5));
  latchNames.insert(make_pair("TDAC6",TDAC6));
  latchNames.insert(make_pair("FDAC0",FDAC0));
  latchNames.insert(make_pair("FDAC1",FDAC1));
  latchNames.insert(make_pair("FDAC2",FDAC2));
  cmdNames.insert(make_pair("HITBUS",FE_RB_HITBUS));
  cmdNames.insert(make_pair("PREAMP",FE_RB_KILL));
  cmdNames.insert(make_pair("SELECT",FE_RB_SELECT));
  cmdNames.insert(make_pair("ENABLE",FE_RB_MASK_B));
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
    int latch=latchNames[regName];
    int cmd = cmdNames[regName];
    if(cmd>0){
      int Address, Size, wValue, rValue;
      m_FEconfig->ReadPixel(cmd, latch);
      retVal = true;
      for(int i=0;i<90;i++){
        m_FEconfig->GetPixelVarAddVal(92-i, Address, Size, wValue, latch);
        m_FEconfig->GetPixelRBVarAddVal(92-i, Address, Size, rValue, latch);
        if(wValue!=rValue) retVal = false;
	data_in.push_back(wValue);
	data_out.push_back(rValue);
      }
    }
  }
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

bool USBI3PixController::moduleActive(int nmod) {                                      //! True if module is active during scan or run
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
  case PixScan::TDACS_VARIATION:
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

  m_SourceScanFlag = false;

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
  if(scn.getChargeInjCap()) m_FEconfig->SetGlobalVal(HITINJECTCAPSEL, 1);
  else m_FEconfig->SetGlobalVal(HITINJECTCAPSEL, 0);

  //JW: set selftrigger
  if(scn.getSelfTrigger()) m_FEconfig->SetGlobalVal(ENABLESELFTRG, 1);
  else m_FEconfig->SetGlobalVal(ENABLESELFTRG, 0);

  //Loop 0
  if(!scn.getLoopActive(0)) {    //JW: loop 0 is not active--> set scan parameter to VCal, scan at current value
    //cout<<"myINFO USBI3PixCtrl: Scan loop 0 is not active"<<endl;
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
      throw USBI3PixControllerExc(USBI3PixControllerExc::NON_UNIFORM_POINTS_NOT_IMPLEMENTED, PixControllerExc::FATAL, getCtrlName()); 
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
    // must be re-implemented
//   case MASK_SCAN_40_STEP:   maskstages = 40;   break;
//   case MASK_SCAN_64_STEP:   maskstages = 64;   break;
//   case MASK_SCAN_80_STEP:   maskstages = 80;   break;
//   case MASK_SCAN_160_STEP:  maskstages = 160;  break;
//   case MASK_SCAN_320_STEP:  maskstages = 320;  break;
//   case MASK_SCAN_2880_STEP: maskstages = 2880; break;
  default:                  maskstages = 32;   break;
  }
  maskshift = 1; //(int)((maskstages/scn.getMaskStageSteps()) + 0.5);

  m_scanConfigArray[5]= maskshift; // ShiftMaskStepSize
  if(UPC_DEBUG) cout << "N total mask steps: " << maskstages << endl;
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

  m_scanConfigArray[7] = shiftmask;  // set masks types that are shifted during uCScan

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
    if(scn.getSourceScanFlag()) {
      // store clustering parameters
      //bool enAdvClus;
      scn.getClusterPars(m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], m_clusterPars[3], m_clusterPars[4], m_clusterPars[5], m_clusterPars[6], m_clusterPars[7]);
//       scn.getClusterPars(m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], m_clusterPars[3], enAdvClus);
//       m_clusterPars[4] = (int)enAdvClus;

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

      m_SourceScanFlag = true;
      m_globalHitcount = 0;
      m_USBreg->SetNumberOfEvents(scn.getRepetitions()); // JJ: write #events to FPGA register, see also CS_COUNTER_MODE

      m_USBreg->ClearSRAM();
    }
    m_TestBeamFlag=scn.getTestBeamFlag();
    m_skipHistosFlag=scn.getSkipSourceScanHistos();
    // ******** end source scan settings ***************************************
}

void USBI3PixController::startScan(PixScan* scn) {                                                 //! Start a scan
  int maskstages;
  int pixelmask[90], pixelmask2[90];
  m_srcCounterType = 0;
  m_SourceScanDone=false;

  for(int i=0; i<90; i++) {
    pixelmask[i]  = 0;
    pixelmask2[i] = 0;
  }

  if(UPC_DEBUG) cout << "N total mask steps: " << scn->getMaskStageTotalSteps() << endl;

  switch(scn->getMaskStageTotalSteps()) {
  case PixScan::STEPS_32:   maskstages = 32;   break;
    // must be re-implemented
//   case MASK_SCAN_40_STEP:   maskstages = 40;   break;
//   case MASK_SCAN_64_STEP:   maskstages = 64;   break;
//   case MASK_SCAN_80_STEP:   maskstages = 80;   break;
//   case MASK_SCAN_160_STEP:  maskstages = 160;  break;
//   case MASK_SCAN_320_STEP:  maskstages = 320;  break;
//   case MASK_SCAN_2880_STEP: maskstages = 2880; break;
  default:                  maskstages = 32;   break;
  }

  //m_USBreg->SetCalibrationMode();
  writeScanConfig(*scn);
  if(!scn->getSourceScanFlag()) {

    m_USBreg->ClearSRAM();

    if (scn->getHistogramFilled(PixScan::TOT_MEAN) || scn->getHistogramFilled(PixScan::TOT_SIGMA)) {
      m_USBreg->SetTOTMode();
      m_USBreg->ClearTOTHisto(); // JJ: need to clear tot histogram 
    }
    else if (scn->getHistogramFilled(PixScan::OCCUPANCY)) {
      m_USBreg->SetCalibrationMode();
      m_USBreg->ClearConfHisto(); // JJ: need to clear conf data array
    }

    // JW: DEBUGING source scan --> do it in raw data mode!
    //if(scn->getSourceScanFlag()) m_USBreg->SetRunMode();

    if (scn->getMaskStageMode() == PixScan::XTALK) { // JW: XTALK: run first step of scan here
      if(UPC_DEBUG) cout << "DEBUG: scn->getMaskStageMode() == PixScan::XTALK" << endl;
      // do the scan step
      if(UPC_DEBUG) cout << "DEBUG: m_FEconfig->StartuCScan()" << endl;
      m_FEconfig->StartuCScan(m_scanConfigArray[0], m_scanConfigArray[1], m_scanConfigArray[2],
        m_scanConfigArray[3], m_scanConfigArray[4], m_scanConfigArray[5],
        1, m_scanConfigArray[7]);
      m_ScanBusy = true;
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
    //    m_FEconfig->StartuCScan
    m_ScanBusy = true;
    if(UPC_DEBUG) cout << "DEBUG: " << "ScanVarIndex: " << m_scanConfigArray[0] << " ScanVarStart: " << m_scanConfigArray[1] << " ScanVarStop: " << m_scanConfigArray[2]
    << " ScanVarStepSize: " << m_scanConfigArray[3] << " Injections: " << m_scanConfigArray[4] << " ShiftMaskStepSize: " << m_scanConfigArray[5]
    << " ShiftMaskSteps: " << m_scanConfigArray[6] << " ShiftMaskType: " << m_scanConfigArray[7] << endl;
    m_FEconfig->StartScan(m_scanConfigArray[0],  m_scanConfigArray[1],  m_scanConfigArray[2], 
      m_scanConfigArray[3],  m_scanConfigArray[4],  m_scanConfigArray[5],
      m_scanConfigArray[6],  m_scanConfigArray[7]);
    if(UPC_DEBUG) cout << "DEBUG: m_FEconfig->StartuCScan()" << endl;
    m_ScanBusy = false;
  } else { // source scan
    clearSourceScanHistos();

    if(UPC_DEBUG) cout<<"DEBUG: starting source measurement"<<endl;
    m_USBreg->SetRunMode();
    m_USBreg->ClearSRAM();
    m_globalHitcount = 0;
    if(UPC_DEBUG) cout<<"DEBUG: source measurement trigger and count modes: " << scn->getSrcTriggerType() << " - " 
      << scn->getSrcCountType() <<endl;
    m_USBreg->WriteRegister(CS_TRIGGER_MODE, scn->getSrcTriggerType());
    m_USBreg->WriteRegister(CS_COUNTER_MODE, scn->getSrcCountType()); // select event counter mode: 0:triggers, 1:hits
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_CLOCK_CYCLES, 15);//scn->getTLUTriggerDataLength() /*32*/); // set TLU trigger data length (depends on TLU bit-file) - re-implement scan option?
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_DATA_DELAY, scn->getTLUTriggerDataDelay() /*0*/); // set additional wait cycles (depends on TLU trigger data length: 32 - CS_TLU_TRIGGER_CLOCK_CYCLES + additional delay = CS_TLU_TRIGGER_DATA_DELAY)
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_LOW_TIME_OUT, 20); // set time out for TLU trigger for not going in low state
    m_USBreg->WriteRegister(CS_TLU_TRIGGER_DATA_MSB_FIRST, 0);
    m_USBreg->WriteRegister(CS_SAVE_UNKNOWN_WORD, 1);
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_0, 0);
    m_USBreg->WriteRegister(CS_LENGTH_TOTAL_1, 0);
    m_USBreg->WriteRegister(CS_MINIMUM_TRIGGER_LENGTH, 0);
    if (scn->getTestBeamFlag()) {
      if(UPC_DEBUG) cout<<"DEBUG: Testbeam mode => activate RJ45" << endl;
      m_USBreg->WriteRegister(CS_ENABLE_RJ45, 1);
    } else {
      if(UPC_DEBUG) cout<<"DEBUG: setting RJ45 to " << (m_enableRJ45?"ON":"OFF") << endl;
      m_USBreg->WriteRegister(CS_ENABLE_RJ45, (m_enableRJ45 ? 1 : 0));
    }
    m_srcCounterType = scn->getSrcCountType();
    m_USBreg->StartMeasurement();
  }
}

void USBI3PixController::stopScan() {
  if(!m_SourceScanFlag) // normal scan
    m_FEconfig->StopuCScan(); 
  else { // source scan
    m_USBreg->StopMeasurement();
  }
}

void USBI3PixController::pauseRun() {
  if(m_SourceScanFlag) {
    m_USBreg->PauseMeasurement();
  }
}

void USBI3PixController::resumeRun() {
  if(m_SourceScanFlag) {
    m_USBreg->ResumeMeasurement();
  }
}

// ******* Histogram treatment on the board ******
bool USBI3PixController::fitHistos() {                                                 //! Fit collected histograms
  return false;
}

void USBI3PixController::getFitResults(HistoType type, unsigned int mod, 
                                     unsigned int slv, std::vector< Histo * > &thr, 
                                     std::vector< Histo * > &noise, 
                                     std::vector< Histo * > &chi2) {                                   //! Read a Fit from Dsp
}


bool USBI3PixController::getErrorHistos(unsigned int dsp, Histo* &his) {               //! Read collected error arrays 
  // don't have error histograms
  his = 0;
  return false;
}


void USBI3PixController::clearSourceScanHistos() {

  if(UPC_DEBUG) cout<<"myINFO: Entered clearSourceScanHistos()"<<endl;
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

  if (UPC_DEBUG) cout << "myINFO USBI3PixCTRL: Entered makeSourceScanHistos()"<<endl;
  if (UPC_DEBUG) cout << "myINFO USBI3PixCTRL: MakeHitHistoFromRawData()"<<endl;
  m_USBreg->MakeHitHistoFromRawData();
  if (UPC_DEBUG) cout << "myINFO USBI3PixCTRL: MakeHitTOTHistoFromRawData()"<<endl;
  m_USBreg->MakeHitTOTHistoFromRawData();
  if (UPC_DEBUG) cout << "myINFO USBI3PixCTRL: MakeHitLV1HistoFromRawData()"<<endl;
  m_USBreg->MakeHitLV1HistoFromRawData();
  if (UPC_DEBUG) cout << "myINFO USBI3PixCTRL: MakeClusterHistoFromRawData("<<m_clusterPars[0]<<", "<<
    m_clusterPars[1]<<", "<<m_clusterPars[2]<<", "<<m_clusterPars[3]<<", "<<((m_clusterPars[4]!=0)?"ON":"OFF")<<")"<<endl;
  // max. column cluster distance, max. row cluster distance, max. cluster BCID depth, 
  // max. threshold for timewalk, enable/disable advanced clustering
  m_USBreg->MakeClusterHistoFromRawData(m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], 
    m_clusterPars[3], (bool)m_clusterPars[4]); 
}

void USBI3PixController::getHisto(HistoType type, unsigned int mod, unsigned int slv, std::vector< std::vector<Histo*> > &his) {  //! Read an histogram

  if(UPC_DEBUG) cout << "DEBUG: USBI3PixController::getHisto()"<< endl;

  bool TOTmode=false;
  bool MadeHitHisto=false;
  bool MadeTOTHisto=false;
  bool MadeLVL1Histo=false;

  std::string scanname, scanname1;
  switch (type) {
  case OCCUPANCY :
    m_USBreg->SetCalibrationMode();
    // JW DEBUG: temporarily enabled raw data mode for debugging of source scan
    //m_USBreg->SetRunMode();
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

  // *** read histogram from SRAM ***
  m_USBreg->ReadSRAM();            //now the data is in ConfHisto[col][row][step]

  if(!m_SourceScanFlag && m_SourceScanDone) {
    if (type == OCCUPANCY || type == HITOCC) MadeHitHisto = true;
    if (TOTmode) MadeTOTHisto = true;
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
    //JW: NOTE ConfigRegister only supports single-chip histograms
    //JW:      stick to module format for the moment
    if (type == OCCUPANCY) {
      h = new Histo(nam.str(), tit.str(), (MadeHitHisto?8:1), 144, -0.5, 143.5,320, -0.5, 319.5);
    }else if(type == TOTAVERAGE) {
      h = new Histo(nam.str(), tit.str(), 46080, 0, 46079);
    }else if(type == LVL1 || type == CLUSTER_SIZE || type == SEED_LVL1 || type == NSEEDS) {
      h = new Histo(nam.str(), tit.str(), 16, -0.5, 15.5);
    }else if(type == TOT || type == CLUSTER_TOT || type == SEED_TOT) {
      h = new Histo(nam.str(), tit.str(), 256, -0.5, 255.5);
    }else if(type == CLSIZE_TOT) {
      h = new Histo(nam.str(), tit.str(), 8, 256, -0.5, 255.5, 16, -0.5, 15.5);
    }else {
      h = new Histo(nam.str(), tit.str(), 8, 144, -0.5, 143.5, 320, -0.5, 319.5);
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
            //	    cout<<dec<<"DEBUG USBI3PixCtrl: col="<<col<<"  \trow="<<row<<"  \tscanindex="<<i<<"  \tentries="<<nr_hits<<endl;
            //	  }
          } else { // ToT mode
            TOTsumsqr=0;
            TOTsum=0;
            hitsum=0;
            TOTsqravg=0;
            TOTavg=0;
            //	  cout<<"DEBUG USBI3PixCtrl: col "<<col<<"\t  row "<<row;
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
void USBI3PixController::writeRunConfig(PixRunConfig &cfg) {                           //! Get the run configuration parameters from PixModuleGroup
}

void USBI3PixController::startRun(int ntrig) {                                     //! Start a run
}

void USBI3PixController::stopRun() {                                                   //! Terminates a run
}

int USBI3PixController::runStatus() {                                                  //! Check the status of the run
  if( UPC_DEBUG) cout << "USBI3PixController::runStatus - m_ScanBusy: " << (m_ScanBusy?"true":"false")<< std::endl;
  if(!m_SourceScanFlag) { // normal scan
    if (m_ScanBusy) {
      return 1;
    } else {
      return 0;
    }
  } else { // source scan
    if(m_SourceScanDone) {
      m_SourceScanDone = false; // reset flag
      return 0;
    } else {
      return 1;
    }
  }
}

int USBI3PixController::nTrigger() {                                                  //! Returns the number of trigger processed so far
  if(!m_SourceScanFlag) { // JW: Normal scan
    //m_USBreg->GetuCScanStatus(m_MaskStep, m_ScanBusy, m_ScanReady, m_ScanCanceled, m_ScanError, m_FEI3Warning, m_FEI3Error1, m_FEI3Error2, m_FEI3Error3, m_FEI3Error4);
    if( UPC_DEBUG) cout << "Read m_MaskStep = " << m_MaskStep << " and m_ScanBusy: " << (m_ScanBusy?"true":"false")<< " ready: "<<(m_ScanReady?"true":"false")<< " canceled: "<<(m_ScanCanceled?"true":"false")<< endl;
    if(m_MaskStep>0) m_MaskStep = (m_MaskStep << 12); // follow ROD conventions
    return m_MaskStep;
  } else { // JW: Source scan
    m_USBreg->GetSourceScanStatus(m_SRAMReadoutReadySignal, m_SRAMFullSignal, m_MeasurementPauseSignal, m_MeasurementRunningSignal, m_SRAMFillLevel, m_NumberOfTriggers, m_NumberOfHitWords, m_TriggerRate, m_HitRate);
    if(UPC_DEBUG) cout<<"DEBUG: m_SRAMReadoutReadySignal: "<<(m_SRAMReadoutReadySignal?"true":"false")<<" m_SRAMFullSignal: "<<(m_SRAMFullSignal?"true":"false")<<" m_MeasurementPauseSignal: "<<(m_MeasurementPauseSignal?"true":"false")<<" m_MeasurementRunningSignal: "<<(m_MeasurementRunningSignal?"true":"false")<<" m_NumberOfTriggers: "<<m_NumberOfTriggers<<" m_NumberOfHitWords: "<<m_NumberOfHitWords<<" m_TriggerRate: "<<m_TriggerRate<<" m_HitRate: "<<m_HitRate<<endl;
    switch(m_srcCounterType){
    case PixScan::COUNT_TRIGGER:
      return m_NumberOfTriggers;
      break;
    case PixScan::COUNT_DH:
    case PixScan::COUNT_DR:
      return m_NumberOfHitWords;
      break;
    default:
      break;
    }
  }
  return -1;
}

bool USBI3PixController::getSourceScanData(unsigned int * & data, bool forceReadSram){
//bool USBI3PixController::getSourceScanData(list<unsigned int>* &data) {
  if(m_SourceScanFlag) {
    if(!m_MeasurementRunningSignal && m_SRAMReadoutReadySignal) { // JJ: readout at the end of scan when desired number of hits is reached
      if(UPC_DEBUG) cout<<"DEBUG USBI3PixCtrl: (!MeasurementRunningSignal && SRAMReadoutReadySignal) => reading SRAM..."<<endl;
      m_USBreg->ReadSRAM();
      if(UPC_DEBUG) std::cout << "MEASUREMENT END: read SRAM" << std::endl;
      if (!m_skipHistosFlag) {
        makeSourceScanHistos();
      }
	  if (m_TestBeamFlag) {
        if(UPC_DEBUG) std::cout << "TB: Create SRAM data list" << std::endl;
        list<unsigned int> data_int;
        m_USBreg->GetRawData(data_int);
	data = new unsigned int [SRAM_WORDSIZE]; // deleted in STeudaq.cxx
	// to do: data_int -> data
      }
      m_USBreg->WriteTurboDAQFromRawData(m_SourceFilename.c_str(), m_NewRawDataFile, true); // JJ: close file; if m_TestBeamFlag is set, raw file is only written when m_SourceFilename.c_str() is not empty
      m_USBreg->ClearSRAM();
      m_USBreg->WriteRegister(CS_TRIGGER_MODE, 0); // JJ: disable external trigger
      m_SourceScanDone = true;
      m_SourceScanFlag = false;
      return true;
    } else if (m_MeasurementRunningSignal && (m_SRAMFullSignal || m_MeasurementPauseSignal) && m_SRAMReadoutReadySignal) { // JJ: readout during scan when SRAM is full
      if(UPC_DEBUG) cout<<"DEBUG USBI3PixCtrl: (m_MeasurementRunningSignal && (m_SRAMFullSignal || m_MeasurementPauseSignal) && m_SRAMReadoutReadySignal) => reading SRAM..."<<endl;
      m_USBreg->ReadSRAM();
      if(UPC_DEBUG) std::cout << "MEASUREMENT RUNNING: read SRAM" << std::endl;
      if (!m_skipHistosFlag) {
        makeSourceScanHistos();
      }
	  if (m_TestBeamFlag) {
        if(UPC_DEBUG) std::cout << "TB: Create SRAM data list" << std::endl;
        list<unsigned int> data_int;
        m_USBreg->GetRawData(data_int);
	data = new unsigned int [SRAM_WORDSIZE]; // deleted in STeudaq.cxx
	// to do: data_int -> data
      }
      m_USBreg->WriteTurboDAQFromRawData(m_SourceFilename.c_str(), m_NewRawDataFile, false); // JJ: do not close file, if m_TestBeamFlag is set no raw file is written because m_SourceFilename.c_str() is empty
      m_USBreg->ClearSRAM();
      m_NewRawDataFile = false; // JJ: from now on append data to existing file, do not overwrite
      m_SourceScanDone = false;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
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

void USBI3PixController::shiftPixMask(int mask, int , int steps){
  // translate mask options
  int shiftmask;
  switch(mask&0xff) {
  case PixScan::SEL:          shiftmask = SELECT; break;
  case PixScan::ENA:          shiftmask = MASK_B; break;
  case PixScan::SEL_ENA:      shiftmask = MASK_B+SELECT; break;
  case PixScan::STATIC:       shiftmask = 0x00; break;
  default:                    shiftmask = 0x00; break; // static
  }
}
void USBI3PixController::shiftPixMask_Int(int mask, int steps){
  if(m_FEconfig!=0){
    int maskTags[4]={HITBUS, SELECT, MASK_B, KILL};
    for(int imask=0;imask<4;imask++){
      if(mask&(0x01<<imask)){ // shift only if mask is selected
        // call shift command
        m_FEconfig->ShiftPixMask(maskTags[imask], steps);
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
bool USBI3PixController::testMonLeakComp(int mod, int ife, int MLguess){
  // ignoring mod and fe input, always using 1 mod, 1 FE
  int Address, Size, rValue;
  const int compBit = 0x1000, mlStatus = 0x2000;
  int mlVal = (MLguess<<1)+compBit; // set to current test value plus comp. bit on
  if(m_FEconfig!=0){
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
int USBI3PixController::readHitBusScaler(int mod, int ife, PixScan* scn){
  // ignoring mod and fe input, always using 1 mod, 1 FE
  int Address, Size, rValue;
  if(m_FEconfig!=0){
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
bool USBI3PixController::checkRxState(rxTypes ){//type){
  bool retval = false;
//   switch(type){
//   case RX0:
//     retval = m_USBpix->CheckRX0State();
//     break;
//   case RX1:
//     retval = m_USBpix->CheckRX1State();
//     break;
//   case RX2:
//     retval = m_USBpix->CheckRX2State();
//     break;
//   case EXT_TRG:
//     retval = m_USBpix->CheckExtTriggerState();
//     break;
//   default:
//     break;
//   }
  return retval;
}
bool USBI3PixController::getGenericBuffer(const char *type, std::string &textBuf) {
  std::string err;
  std::string info;

  bool ret = false;
  if(strcmp(type,"err") == 0){
    textBuf += m_errBuff;
    if (m_errBuff != "") ret = true;
    m_errBuff.clear();
    return ret;
  }
  else if(strcmp(type,"info") == 0){
    textBuf += m_infoBuff;
    if (m_infoBuff != "") ret = true;
    m_infoBuff.clear();
    return ret;
  }
  else{
    textBuf += "Buffer type not available";
  }
  return ret;
}
