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

// @todo: Carefully replace the NBOARDS_MAX where appropriate with
// m_chipIds vector references.

#include <string.h>
#include <time.h>

//USB board classes
#include "SiLibUSB.h"
#include "defines.h"
#include "USBpix.h"
#include "ReadoutStatusRegister.h"
#include <USBPixI4DCS.h>
#include "processor.h"
//USB board classes
#include "defines.h"

#include "PixModuleGroup/PixModuleGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "Config/Config.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeI4A.h"
#include "PixFe/PixFeI4B.h"
#include "PixCcpd/PixCcpdv1.h"
#include "PixCcpd/PixCcpdv2.h"
#include "PixCcpd/PixCcpdLF.h"
#include "PixDcs/SleepWrapped.h"
#include "PixDcs/PixDcs.h"
#include "PixDcs/USB_PixDcs.h"
#include "USBPixController.h"
#include "logical.h"

#include <TMath.h>

#include <sstream>
#include <fstream>
#include <algorithm>

#include <cassert>

#define UPC_DEBUG_GEN false
#define UPC_DEBUG_FLAGS false

using namespace PixLib;
using namespace std;

// m_chipIds previously was an array, is now a vector and should therefore be 
// initialized with two entries for multi board operation.
const int initial_chip_ids = 2;

const int RCA_AUTO = -2;
const int RCA_AUTOSAVE = -1;

// UINT32 bitflip(UINT32 bits_in){
// 	UINT32 bits_out=0;
// 	for(int k=0;k<32;k++){
// 		if(k<16)
// 			bits_out |= (bits_in&(1<<k))<<(31-2*k);
// 		else
// 			bits_out |= (bits_in&(1<<k))>>(2*k-31);
// 	}
// 	return bits_out;
// }

// ***************** Constructors *********************
USBPixController::USBPixController(PixModuleGroup &modGrp, DBInquire *dbInquire) :
PixController(modGrp, dbInquire), m_readoutChannelReadsChip(4) { 
	initChipIds();
	configInit();
	m_conf->read(dbInquire);
	if((m_AdapterCardFlavor&0x1)==0){ // std. adapter card: no 4-FE-option, so force all related items to default
	  m_readoutChannelInput[0] = 1;
	  for (int i = 1; i < 4; i++) m_readoutChannelInput[i] = 0;
	}

	for(int ib=0;ib<NBOARDS_MAX;ib++)
		m_BoardHandle[ib] = NULL;
	m_USBpix = NULL;
	m_upcScanBusy = false;
	m_upcScanInit = false;
	m_upcScanCancelled = false;
	m_scanBusy = false;
	m_scanDone = false;
	m_sourceScanBusy = false;
	m_sourceScanDone = false;
	m_SourceScanEventQuantity = 0;
  m_createdRawDataFile = false;
  m_scanReady = false;
  m_upcStartScanHasFinished = false;
	m_infoBuff="";
	m_errBuff="";
	m_withCCPD=false;

	// to be updated later
	m_nColFe = 80;
	m_nRowFe = 336;
}

USBPixController::USBPixController(PixModuleGroup &modGrp, int adapterCardFlavor) : 
PixController(modGrp), m_readoutChannelReadsChip(4) { //! Constructor
	initChipIds();
	configInit();
	m_AdapterCardFlavor = adapterCardFlavor;
	if((m_AdapterCardFlavor&0x1)==0){ // std. adapter card: no 4-FE-option, so force all related items to default
	  m_readoutChannelInput[0] = 1;
	  for (int i = 1; i < 4; i++) m_readoutChannelInput[i] = 0;
	} else {
	  m_MultiChipWithSingleBoard = (m_AdapterCardFlavor==1); // multi-chip mode only for BIC if selected so (m_AdapterCardFlavor==3 is FE-by-fe)
	  m_AdapterCardFlavor = 1; // FE-by-FE mode was 3 which is otherwise not handles
	}
	m_USBpix = NULL;
	for(int ib=0;ib<NBOARDS_MAX;ib++)
		m_BoardHandle[ib] = NULL;
	m_upcScanBusy = false;
	m_upcScanInit = false;
	m_upcScanCancelled = false;
	m_scanBusy = false;
	m_scanDone = false;
	m_sourceScanBusy = false;
	m_sourceScanDone = false;
  m_createdRawDataFile = false;
  m_scanReady = false;
  m_upcStartScanHasFinished = false;
	m_infoBuff="";
	m_errBuff="";
	m_withCCPD=false;

	// to be updated later
	m_nColFe = 80;
	m_nRowFe = 336;
}

USBPixController::~USBPixController(){
  delete m_USBpix;
  for(int ib=0;ib<NBOARDS_MAX;ib++)
    delete (m_BoardHandle[ib]);
  delete m_conf;
}

void USBPixController::initChipIds()
{
  m_chipIds.resize(initial_chip_ids);
  std::fill(m_chipIds.begin(), m_chipIds.end(), 999);
}

void USBPixController::configInit(){

	// Create the Config object
	m_conf = new Config("USBPixController"); 
	Config &conf = *m_conf;

	// Group general
	conf.addGroup("general");

	conf["general"].addInt("BoardID", m_boardID[0], -1,
		"ID of (master) board to use - leave at -1 to take any", true);
	conf["general"].addInt("Board2ID", m_boardID[1], -1,
		"ID of slave board to use - leave at -1 to not use slave board", true);
	conf["general"].addString("FirmwareFile", m_FPGA_filename, "C:/USBPix/configs/usbpix.bit",
				  "FPGA firmware file name", true, 1);
	conf["general"].addString("uCFirmwareFile", m_uC_filename, "",
				  "micro controller firmware file name (if empty it is not overwritten)", true, 1);
	conf["general"].addInt("CableLength", m_cableLength, 2, 
		"Adjusts pixel register read-back phase for length of cable between USB board and FE board", true);
	std::map<std::string, int> roList;
	roList["40 Mb/s"]  = RO4OMBPS;
	roList["160 Mb/s"] = RO160MBPS;
	conf["general"].addList("ReadoutSpeed", m_readoutSpeed, RO160MBPS, roList,
		"Speed of data coming back from FE-I4", true);
	conf["general"].addBool("setSelCMD",m_selCmd, true,
		"set pixel register via command decoder", true);
	conf["general"].addBool("slowCtrlMode", m_slowCtrlMode, false, "Use slow control mode for registers", true);
	conf["general"].addBool("enableCmdLvl1", m_enableCmdLvl1, true,
		"use CMDs for trigger and injection", true);
	conf["general"].addBool("enableRJ45", m_enableRJ45, false, 
		"Enable RJ45 input during scans", true);
	std::map<std::string, int> triggRepList;
	triggRepList["Disabled"]  = REPLICATION_OFF;
	triggRepList["Master"] = REPLICATION_MASTER;
	triggRepList["Slave"] = REPLICATION_SLAVE;
	conf["general"].addList("TriggerReplication", m_triggerReplication, REPLICATION_OFF, triggRepList,
		"Role of USBpix Board for Trigger Replication", true);
	conf["general"].addInt("IoMuxSel", m_ioMuxSel, 0, "Set bit pattern on IoMxSel pads", true);
	conf["general"].addInt("TX2Signal", m_tx2signal, 0, "Signal written to the TX[2] output of the MultiIO board", true);
	conf["general"].addInt("IoMuxIn", m_ioMuxIn, 0, "Set bit pattern on IoMxIn pads", true);
	conf["general"].addInt("AuxClkDiv", m_auxClkDiv, 1, "Aux. clk.  40MHz div. by given amount", true);
	conf["general"].addInt("IrefPads", m_irefPads, 0, "Bit pattern on Iref pads", true);
	conf["general"].addBool("invIrefPads", m_invIrefPads, true, "Invert bit pattern on Iref pads", true);
	conf["general"].addBool("EnableGPACStrobePin", m_enableGPACStrobePin, false, "Enable GPAC Strobe", true);
	conf["general"].addBool("EnableSCAStrobePin", m_enableSCAStrobePin, false, "Enable SCA Strobe Pin", true);
	conf["general"].addInt("AdapterCardFlavor",m_AdapterCardFlavor, 0,
		"set adapter card flavor", true);
	conf["general"].addInt("FEToRead", m_feToRead, 4, "FE Number to read", true);
	conf["general"].addBool("enaManCode", m_enaManCode, false, "enable Machnester Coding", true);
	conf["general"].addInt("manCodePhase", m_manCodePhase, 0, "Clk phase when Manchester coding is used", true);
	conf["general"].addBool("MultiChipWithSingleBoard", 
			m_MultiChipWithSingleBoard, false, 
			"Use only a single USBpix unit to read out multiple "
			"chips as opposted to multiple parallel units.", true);
	conf["general"].addBool("OverrideEnableDemux", 
			m_OverrideEnableDemux, false, 
			"Always enable demux settings, even if other logic decides against it.",
			true);

  
  for (int i = 0; i < 4; i++)
  {
    {
      std::stringstream sv;
      sv << "readoutChannelsInput";
      sv << i;
      std::stringstream desc;
      desc << "Input to readout channel ";
      desc << i;
      conf["general"].addInt(sv.str(), 
          m_readoutChannelInput[i], 1, 
          desc.str(), true);
    }
    {
      std::stringstream sv;
      sv << "inputDivider";
      sv << i;
      std::stringstream desc;
      desc << "Input data divider for input channel ";
      desc << i;
      conf["general"].addInt(sv.str(), 
          m_inputDivider[i], 0, 
          desc.str(), true);
    }
    {
      std::stringstream sv;
      sv << "readoutChannelReadsChip";
      sv << i;
      std::stringstream desc;
      desc << "Chip that is read by read out channel ";
      desc << i;
      conf["general"].addInt(sv.str(), 
          m_readoutChannelReadsChip[i], i, 
          desc.str(), true);
    }
  }

	// Select default values
	conf.reset();

	m_globRegNames["TrigCnt"]           =  2;
	m_globRegNames["Conf_AddrEnable"]   =  2;
	m_globRegNames["Reg2Spare"]         =  2;
	m_globRegNames["ErrMask0"]          =  3;
	m_globRegNames["ErrMask1"]          =  4;
	m_globRegNames["PrmpVbpRight"]      =  5;
	m_globRegNames["Vthin"]             =  5;
	m_globRegNames["DisVbn_CPPM"]       =  6;
	m_globRegNames["PrmpVbp"]           =  6;
	m_globRegNames["TdacVbp"]           =  7;
	m_globRegNames["DisVbn"]            =  7;
	m_globRegNames["Amp2Vbn"]           =  8;
	m_globRegNames["Amp2VbpFol"]        =  8;
	m_globRegNames["PrmpVbpTop"]        =  9;
	m_globRegNames["Amp2Vbp"]           =  9;
	m_globRegNames["FdacVbn"]           = 10;
	m_globRegNames["Amp2Vbpf"]          = 10;
	m_globRegNames["PrmpVbnFol"]        = 11;
	m_globRegNames["PrmpVbpLeft"]       = 11;
	m_globRegNames["PrmpVbpf"]          = 12;
	m_globRegNames["PrmpVbnLcc"]        = 12;
	m_globRegNames["Reg13Spare"]        = 13;
	m_globRegNames["PxStrobes"]         = 13;
	m_globRegNames["S0"]                = 13;
	m_globRegNames["S1"]                = 13;
	m_globRegNames["LVDSDrvIref"]       = 14;
	m_globRegNames["BonnDac"]           = 14;
	m_globRegNames["PllIbias"]          = 15;
	m_globRegNames["LVDSDrvVos"]        = 15;
	m_globRegNames["TempSensBias"]      = 16;
	m_globRegNames["PllIcp"]            = 16;
	m_globRegNames["Reg17Spare"]        = 17;
	m_globRegNames["PlsrIdacRamp"]      = 17;
	m_globRegNames["Reg18Spare"]        = 18;
	m_globRegNames["PlsrVgOPamp"]       = 18;
	m_globRegNames["PlsrDacBias"]       = 19;
	m_globRegNames["Reg19Spare"]        = 19;
	m_globRegNames["Vthin_AltCoarse"]   = 20;
	m_globRegNames["Vthin_AltFine"]     = 20;
	m_globRegNames["PlsrDAC"]           = 21;
	m_globRegNames["DIGHITIN_Sel"]      = 21;
	m_globRegNames["DINJ_Override"]     = 21;
	m_globRegNames["HITLD_In"]          = 21;
	m_globRegNames["Reg21Spare"]        = 21;
	m_globRegNames["Reg22Spare2"]       = 22;
	m_globRegNames["Colpr_Addr"]        = 22;
	m_globRegNames["Colpr_Mode"]        = 22;
	m_globRegNames["Reg22Spare1"]       = 22;
	m_globRegNames["DisableColumnCnfg0"]= 23;
	m_globRegNames["DisableColumnCnfg1"]= 24;
	m_globRegNames["DisableColumnCnfg2"]= 25;
	m_globRegNames["TrigLat"]           = 25;
	m_globRegNames["CMDcnt"]            = 26;
	m_globRegNames["StopModeCnfg"]      = 26;
	m_globRegNames["HitDisableCnfg"]    = 26;
	m_globRegNames["EN_PLL"]            = 27;
	m_globRegNames["Efuse_sense"]       = 27;
	m_globRegNames["Stop_Clk"]          = 27;
	m_globRegNames["ReadErrorReq"]      = 27;
	m_globRegNames["ReadSkipped"]       = 27;
	m_globRegNames["Reg27Spare"]        = 27;
	m_globRegNames["GateHitOr"]         = 27;
	m_globRegNames["CalEn"]             = 27;
	m_globRegNames["SR_clr"]            = 27;
	m_globRegNames["Latch_en"]          = 27;
	m_globRegNames["SR_Clock"]          = 27;
	m_globRegNames["LVDSDrvSet06"]      = 28;
	m_globRegNames["Reg28Spare"]        = 28;
	m_globRegNames["EN40M"]             = 28;
	m_globRegNames["EN80M"]             = 28;
	m_globRegNames["CLK1"]              = 28;
	m_globRegNames["CLK0"]              = 28;
	m_globRegNames["EN160M"]            = 28;
	m_globRegNames["EN320M"]            = 28;
	m_globRegNames["Reg29Spare1"]       = 29;
	m_globRegNames["no8b10b"]           = 29;
	m_globRegNames["Clk2OutCnfg"]       = 29;
	m_globRegNames["EmptyRecord"]       = 29;
	m_globRegNames["Reg29Spare2"]       = 29;
	m_globRegNames["LVDSDrvEn"]         = 29;
	m_globRegNames["LVDSDrvSet30"]      = 29;
	m_globRegNames["LVDSDrvSet12"]      = 29;
	m_globRegNames["PlsrRiseUpTau"]     = 31;
	m_globRegNames["PlsrPwr"]           = 31;
	m_globRegNames["PlsrDelay"]         = 31;
	m_globRegNames["ExtDigCalSW"]       = 31;
	m_globRegNames["ExtAnaCalSW"]       = 31;
	m_globRegNames["Reg31Spare"]        = 31;
	m_globRegNames["SELB0"]             = 32;
	m_globRegNames["SELB1"]             = 33;
	m_globRegNames["SELB2"]             = 34;
	m_globRegNames["EfuseCref"]         = 34;
	m_globRegNames["EfuseVref"]         = 34;
	m_globRegNames["Chip_SN"]           = 35;

	// new for FE-I4B
	m_globRegNames["Reg1Spare"]         = 1;
	m_globRegNames["SmallHitErase"]     = 1;
	m_globRegNames["Eventlimit"]        = 1;
	m_globRegNames["BufVgOpAmp"]        = 5;
	m_globRegNames["Reg6Spare"]         = 6;
	m_globRegNames["Reg9Spare"]         = 9;
	m_globRegNames["GADCOpAmp"]         = 14;
	m_globRegNames["VrefDigTune"]       = 18;
	m_globRegNames["VrefAnTune"]        = 19;
	m_globRegNames["Reg27Spare1"]       = 27;
	m_globRegNames["GADC_Enable"]       = 27;
	m_globRegNames["ShiftReadBack"]     = 27;
	m_globRegNames["Reg27Spare2"]       = 27;
	m_globRegNames["TempSensDiodeSel"]  = 30;
	m_globRegNames["TempSensDisable"]   = 30;
	m_globRegNames["IleakRange"]        = 30;
	m_globRegNames["Reg30Spare"]        = 30;
	m_globRegNames["GADCSel"]           = 31;
	m_globRegNames["Reg34Spare1"]       = 34;
	m_globRegNames["PrmpVbpMsnEn"]      = 34;
	m_globRegNames["Reg34Spare2"]       = 34;

	m_latchNames["ENABLE"]  = ENABLE+1; // must add 1, since ENABLE=0 would spoil identification of wrong names
	m_latchNames["CAP0"]    = CAP0+1;
	m_latchNames["CAP1"]    = CAP1+1;
	m_latchNames["ILEAK"]   = HITBUS+1; // is both hitbus and Ileak control...
	m_latchNames["TDAC0"]   = TDAC0+1;
	m_latchNames["TDAC1"]   = TDAC1+1;
	m_latchNames["TDAC2"]   = TDAC2+1;
	m_latchNames["TDAC3"]   = TDAC3+1;
	m_latchNames["TDAC4"]   = TDAC4+1;
	m_latchNames["FDAC0"]   = FDAC0+1;
	m_latchNames["FDAC1"]   = FDAC1+1;
	m_latchNames["FDAC2"]   = FDAC2+1;
	m_latchNames["FDAC3"]   = FDAC3+1;
	m_latchNames["DIGINJ"]  = DIGINJ+1;

	//CCPD
	m_ccpd_globals["BLRes"]  = CCPD_BLRES;
	m_ccpd_globals["ThRes"]  = CCPD_THRES;
	m_ccpd_globals["VN"]     = CCPD_VN;
	m_ccpd_globals["VNFB"]   = CCPD_VNFB;
	m_ccpd_globals["VNFoll"] = CCPD_VNFOLL;
	m_ccpd_globals["VNLoad"] = CCPD_VNLOAD;
	m_ccpd_globals["VNDAC"]  = CCPD_VNDAC;
	m_ccpd_globals["NU1"]    = CCPD_NU1;
	m_ccpd_globals["NU2"]    = CCPD_NU2;
	m_ccpd_globals["NotUsed"]= CCPD_NOTUSED;
	m_ccpd_globals["VNComp"] = CCPD_VNCOMP;
	m_ccpd_globals["VNCompL"]= CCPD_VNCOMPL;
	m_ccpd_globals["VNOut0"] = CCPD_VNOUT0;
	m_ccpd_globals["VNOut1"] = CCPD_VNOUT1;
	m_ccpd_globals["VNOut2"] = CCPD_VNOUT2;
	
	//CCPD v2
	m_ccpd_globals2["BLRes"]   = V2_CCPD_BLRES;
	m_ccpd_globals2["ThRes"]   = V2_CCPD_THRES;
	m_ccpd_globals2["VN"]      = V2_CCPD_VN;
	m_ccpd_globals2["VN2"]     = V2_CCPD_VN2;
	m_ccpd_globals2["VNFB"]    = V2_CCPD_VNFB;
	m_ccpd_globals2["VNFoll"]  = V2_CCPD_VNFOLL;
	m_ccpd_globals2["VNLoad"]  = V2_CCPD_VNLOAD;
	m_ccpd_globals2["VNDAC"]   = V2_CCPD_VNDAC;
	m_ccpd_globals2["ThPRes"]  = V2_CCPD_THPRES;
	m_ccpd_globals2["ThP"]     = V2_CCPD_THP;
	m_ccpd_globals2["VNOut"]   = V2_CCPD_VNOUT;
	m_ccpd_globals2["VNComp"]  = V2_CCPD_VNCOMP;
	m_ccpd_globals2["VNCompLd"]= V2_CCPD_VNCOMPLD;
	m_ccpd_globals2["VNOut1"]  = V2_CCPD_VNOUT1;
	m_ccpd_globals2["VNOut2"]  = V2_CCPD_VNOUT2;
	m_ccpd_globals2["VNOut3"]  = V2_CCPD_VNOUT3;
	m_ccpd_globals2["VNBuffer"]= V2_CCPD_VNBUFFER;
	m_ccpd_globals2["VPFoll"]  = V2_CCPD_VPFOLL;
	m_ccpd_globals2["VNBias"]  = V2_CCPD_VNBIAS;
	m_ccpd_globals2["Q0"]      = V2_CCPD_Q0;
	m_ccpd_globals2["Q1"]      = V2_CCPD_Q0;
	m_ccpd_globals2["Q2"]      = V2_CCPD_Q0;
	m_ccpd_globals2["Q3"]      = V2_CCPD_Q0;
	m_ccpd_globals2["Q4"]      = V2_CCPD_Q0;
	m_ccpd_globals2["Q5"]      = V2_CCPD_Q0;
	
	//CCPD LF
	m_ccpd_globalsLF["BLRes"]   = LF_CCPD_BLRES;
	m_ccpd_globalsLF["VN"]   = LF_CCPD_VN;
	m_ccpd_globalsLF["VPFB"]   = LF_CCPD_VPFB;
	m_ccpd_globalsLF["VNFoll"]   = LF_CCPD_VNFOLL;
	m_ccpd_globalsLF["VPLoad"]   = LF_CCPD_VPLOAD;
	m_ccpd_globalsLF["LSBdacL"]   = LF_CCPD_LSBDACL;
	m_ccpd_globalsLF["IComp"]   = LF_CCPD_ICOMP;
	m_ccpd_globalsLF["VStrech"]   = LF_CCPD_VSTRETCH;
	m_ccpd_globalsLF["WGT0"]   = LF_CCPD_WGT0;
	m_ccpd_globalsLF["WGT1"]   = LF_CCPD_WGT1;
	m_ccpd_globalsLF["WGT2"]   = LF_CCPD_WGT2;
	m_ccpd_globalsLF["IDacTEST"]   = LF_CCPD_IDACTEST;
	m_ccpd_globalsLF["IDacLTEST"]   = LF_CCPD_IDACLTEST;
	m_ccpd_globalsLF["Trim_En"]   = LF_CCPD_TRIM_EN;
	m_ccpd_globalsLF["Inject_En"]   = LF_CCPD_INJECT_EN;
	m_ccpd_globalsLF["Monitor_En"]   = LF_CCPD_MONITOR_EN;
	m_ccpd_globalsLF["Preamp_En"]   = LF_CCPD_PREAMP_EN;

	m_configValid = false;
}

// ***************** Board functionality **************
void USBPixController::initHWMultipleBoards()
{
	for(unsigned int ib=0;ib<NBOARDS_MAX;ib++){
		if (ib < m_chipIds.size())
			m_chipIds.at(ib) = 999;	
		if(ib==0 ||  m_boardID[ib]>=0) {
			if(m_BoardHandle[ib]==0){
				if(UPC_DEBUG_GEN) cout<<"INFO: Creating SiUSBDevice" << endl;
				m_BoardHandle[ib] = new SiUSBDevice(0);
				if(UPC_DEBUG_GEN) cout<<"INFO: m_BoardHandle["<<ib<<"] = "<<m_BoardHandle[ib]<<endl;
			}

			if (m_BoardHandle[ib] != 0) {
				if (ib < m_chipIds.size())
					m_chipIds.at(ib) = ib; // default value to start with
				updateDeviceHandle();

				// check flavour of actually present adapter board and compare with config. setting - throw exception if inconsistent
				int adapType = detectAdapterCard(ib);
				if(UPC_DEBUG_GEN) cout<<"INFO: Detected adapter type="<< adapType<<endl;
				if(adapType<0) throw USBPixControllerExc(USBPixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getCtrlName(), m_boardID[ib]); 
				if(m_AdapterCardFlavor!=adapType){
				  m_AdapterCardFlavor=adapType;
				  int nmod = 0;
				  for(PixModuleGroup::moduleIterator MI=m_modGroup.modBegin(); MI!=m_modGroup.modEnd(); MI++) nmod++;
				  m_MultiChipWithSingleBoard = (m_AdapterCardFlavor==0x1 && nmod==1); // BIC with one module -> multi-FE-operation
				  throw USBPixControllerExc(USBPixControllerExc::WRONG_ADAPTER, PixControllerExc::ERROR, getCtrlName(), m_boardID[ib]); 
				}

				// write uC firmware
				if(m_uC_filename!=""){
					if(UPC_DEBUG_GEN) cout<<"Processing uC firmware" << endl;
					FILE *g=0;
#ifdef CF__LINUX
					g = fopen(m_uC_filename.c_str(),"r");
#else
					fopen_s(&g, m_uC_filename.c_str(),"r");
#endif
					if(g==0){
						if(UPC_DEBUG_GEN) cout<<"ERROR: uC bix file doesn't exist"<<endl;
						throw USBPixControllerExc(USBPixControllerExc::FILE_ERROR, PixControllerExc::ERROR, getCtrlName()); 
					} else
						fclose(g);
					if(!m_BoardHandle[ib]->LoadFirmwareFromFile(m_uC_filename.c_str())){
						if(UPC_DEBUG_GEN) cout<<"ERROR: uC didn't configure"<<endl;
						throw USBPixControllerExc(USBPixControllerExc::UC_ERROR, PixControllerExc::ERROR, getCtrlName()); 
					}
					// update device list until board has disappered
					int iw=0;
					const int iwmax=500;
					while(!OnDeviceChange() && iw<iwmax){
						if(UPC_DEBUG_GEN) cout <<"INFO: waiting for device to disappear from list, iteration " << iw << endl;
						iw++;
						sleep(50);
					}
					// get new handle as soon as uC is ready again
					iw=0;
					void *tempHandle=0;
					while(tempHandle==0 && iw<iwmax){
						if(OnDeviceChange()) tempHandle = GetUSBDevice(m_boardID[ib]);
						if(UPC_DEBUG_GEN) cout <<"INFO: waiting for device to be back, iteration " << iw << endl;
						iw++;
						sleep(50);
					}
					// set handle again
					if(tempHandle!=0){
						m_BoardHandle[ib]->SetDeviceHandle(tempHandle);
						if(UPC_DEBUG_GEN) cout <<"INFO: after uC load - found "<< m_BoardHandle[ib] << " - " << m_BoardHandle[ib]->GetName() << " with ID " << 
							m_BoardHandle[ib]->GetId() << " and FW " << m_BoardHandle[ib]->GetFWVersion() << endl;
					} else{
						if(UPC_DEBUG_GEN) cout<<"ERROR: no board handle found after uC load..."<<endl;
						throw USBPixControllerExc(USBPixControllerExc::NOBOARD, PixControllerExc::FATAL, getCtrlName(), m_boardID[ib]); 
					}
				}
				// write FPGA firmware
				if(UPC_DEBUG_GEN) cout<<"INFO: Processing FPGA firmware" << endl;
				FILE *f = 0;
#ifdef CF__LINUX
				f = fopen(m_FPGA_filename.c_str(),"r");
#else
				fopen_s(&f, m_FPGA_filename.c_str(),"r");
#endif
				if(f==0){
				  delete m_BoardHandle[ib]; m_BoardHandle[ib]=0; // this will make sure that ctrl. is fully initialised again
					if(UPC_DEBUG_GEN) cout<<"ERROR: FPGA bit file " << m_FPGA_filename <<" doesn't exist"<<endl;
					throw USBPixControllerExc(USBPixControllerExc::FILE_ERROR, PixControllerExc::ERROR, getCtrlName()); 
				} else
					fclose(f);
				if(!m_BoardHandle[ib]->DownloadXilinx(m_FPGA_filename.c_str())){
				  delete m_BoardHandle[ib]; m_BoardHandle[ib]=0; // this will make sure that ctrl. is fully initialised again
					if(UPC_DEBUG_GEN) cout<<"ERROR: FPGA didn't configure"<<endl;
					throw USBPixControllerExc(USBPixControllerExc::FPGA_ERROR, PixControllerExc::ERROR, getCtrlName()); 
				}
			}
		}
	}
	if(m_BoardHandle[0] != 0) {
	  // 	        if(UPC_DEBUG_GEN) cout<<"INFO: processing USBpix class matters: " << std::hex << ((int)m_USBpix) << std::dec << endl;
		// Create USBpix object - contains ConfigFEMemory and ConfigRegister classes    
		if (m_chipIds.size() < 2) 
			m_chipIds.resize(2);
		if (m_chipIds.size() == 0 || m_chipIds.at(1)==999) 
			m_chipIds.at(0)=8;
		if(m_USBpix==NULL) m_USBpix = new USBpix(m_chipIds.at(0),0, m_BoardHandle[0], true, m_BoardHandle[1], m_chipIds.at(1), m_MultiChipWithSingleBoard);
		else m_USBpix->SetUSBHandles(m_BoardHandle[0], m_BoardHandle[1]);

		// re-set phase shift - needed in case of initialisation for more than one time
		m_USBpix->SetCurrentPhaseshift(0, m_chipIds.at(0));
    m_USBpix->SetAdapterCardFlavor(m_AdapterCardFlavor&0x1);
		if(m_chipIds.at(1)==999) m_USBpix->SetCurrentPhaseshift(0,  m_chipIds.at(1));
		updateRegs();
	}else{
		if(UPC_DEBUG_GEN) cout<<"ERROR: no SiUSBDevice found..."<<endl;
		m_USBpix = NULL;
		throw USBPixControllerExc(USBPixControllerExc::NOHANDLE, PixControllerExc::FATAL, getCtrlName()); 
	}
        // refresh readback ID
        m_boardIDRB = m_BoardHandle[0]->GetId();
}

void USBPixController::initHWSingleBoard()
{
	if (UPC_DEBUG_GEN) 
		cout  << "INFO: Creating SiUSBDevice" << endl;
	m_BoardHandle[0] = new SiUSBDevice(0);
	if (UPC_DEBUG_GEN) 
		cout << "INFO: m_BoardHandle[0] = "
			<< m_BoardHandle[0] << endl;

	for(int ib=0;ib<NBOARDS_MAX;ib++){
		if (m_BoardHandle[ib] != 0) {
			updateDeviceHandle();
			// check flavour of actually present adapter board and compare with config. setting - throw exception if inconsistent
			int adapType = detectAdapterCard(ib);
			if(UPC_DEBUG_GEN) cout<<"INFO: Detected adapter type="<< adapType<<endl;
			if(adapType<0) throw USBPixControllerExc(USBPixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getCtrlName(), m_boardID[ib]); 
			if(m_AdapterCardFlavor!=adapType){
			  m_AdapterCardFlavor=adapType;
			  int nmod = 0;
			  for(PixModuleGroup::moduleIterator MI=m_modGroup.modBegin(); MI!=m_modGroup.modEnd(); MI++) nmod++;
			  m_MultiChipWithSingleBoard = (m_AdapterCardFlavor==0x1 && nmod==1); // BIC with one module -> multi-FE-operation
			  throw USBPixControllerExc(USBPixControllerExc::WRONG_ADAPTER, PixControllerExc::ERROR, getCtrlName(), m_boardID[ib]); 
			}
			// write uC firmware
			if(m_uC_filename!=""){
				if(UPC_DEBUG_GEN) cout<<"Processing uC firmware" << endl;
				FILE *g = 0;
#ifdef CF__LINUX
				g = fopen(m_uC_filename.c_str(),"r");
#else
				fopen_s(&g, m_uC_filename.c_str(),"r");
#endif
				if(g==0){
					if(UPC_DEBUG_GEN) cout<<"ERROR: uC bix file doesn't exist"<<endl;
					throw USBPixControllerExc(USBPixControllerExc::FILE_ERROR, PixControllerExc::ERROR, getCtrlName()); 
				} else
					fclose(g);
				if(!m_BoardHandle[ib]->LoadFirmwareFromFile(m_uC_filename.c_str())){
					if(UPC_DEBUG_GEN) cout<<"ERROR: uC didn't configure"<<endl;
					throw USBPixControllerExc(USBPixControllerExc::UC_ERROR, PixControllerExc::ERROR, getCtrlName()); 
				}
				// update device list until board has disappered
				int iw=0;
				const int iwmax=500;
				while(!OnDeviceChange() && iw<iwmax){
					if(UPC_DEBUG_GEN) cout <<"INFO: waiting for device to disappear from list, iteration " << iw << endl;
					iw++;
					sleep(50);
				}
				// get new handle as soon as uC is ready again
				iw=0;
				void *tempHandle=0;
				while(tempHandle==0 && iw<iwmax){
					if(OnDeviceChange()) tempHandle = GetUSBDevice(m_boardID[ib]);
					if(UPC_DEBUG_GEN) cout <<"INFO: waiting for device to be back, iteration " << iw << endl;
					iw++;
					sleep(50);
				}
				// set handle again
				if(tempHandle!=0){
					m_BoardHandle[ib]->SetDeviceHandle(tempHandle);
					if(UPC_DEBUG_GEN) cout <<"INFO: after uC load - found "<< m_BoardHandle[ib] << " - " << m_BoardHandle[ib]->GetName() << " with ID " << 
						m_BoardHandle[ib]->GetId() << " and FW " << m_BoardHandle[ib]->GetFWVersion() << endl;
				} else{
					if(UPC_DEBUG_GEN) cout<<"ERROR: no board handle found after uC load..."<<endl;
					throw USBPixControllerExc(USBPixControllerExc::NOBOARD, PixControllerExc::FATAL, getCtrlName(), m_boardID[ib]); 
				}
			}
			// write FPGA firmware
			if(UPC_DEBUG_GEN) cout<<"INFO: Processing FPGA firmware" << endl;
			FILE *f = 0;
#ifdef CF__LINUX
			f = fopen(m_FPGA_filename.c_str(),"r");
#else
			fopen_s(&f, m_FPGA_filename.c_str(),"r");
#endif
			if(f==0){
			  delete m_BoardHandle[ib]; m_BoardHandle[ib]=0; // this will make sure that ctrl. is fully initialised again
				if(UPC_DEBUG_GEN) cout<<"ERROR: FPGA bit file " << m_FPGA_filename <<" doesn't exist"<<endl;
				throw USBPixControllerExc(USBPixControllerExc::FILE_ERROR, PixControllerExc::ERROR, getCtrlName()); 
			} else
				fclose(f);
			if(!m_BoardHandle[ib]->DownloadXilinx(m_FPGA_filename.c_str())){
			  delete m_BoardHandle[ib]; m_BoardHandle[ib]=0; // this will make sure that ctrl. is fully initialised again
				if(UPC_DEBUG_GEN) cout<<"ERROR: FPGA didn't configure"<<endl;
				throw USBPixControllerExc(USBPixControllerExc::FPGA_ERROR, PixControllerExc::ERROR, getCtrlName()); 
			}
		}
	}
	if(m_BoardHandle[0] != 0) {
    m_chipIds.clear();
    const int dummyChipId = 999;
		if(m_USBpix==NULL) m_USBpix = new USBpix(dummyChipId, 0, 
				m_BoardHandle[0], true, m_BoardHandle[1], 
				dummyChipId, m_MultiChipWithSingleBoard);
    
		else m_USBpix->SetUSBHandles(m_BoardHandle[0], m_BoardHandle[1]);
  
    m_USBpix->SetAdapterCardFlavor(m_AdapterCardFlavor&0x1);

		// re-set phase shift - needed in case of initialisation for more than one time
		//m_USBpix->SetCurrentPhaseshift(0, m_chipIds.at(0));
		
    // Update regs is delayed until all chips are configured after first
    // writeModuleConfig
    //updateRegs();
	}else{
		if(UPC_DEBUG_GEN) cout<<"ERROR: no SiUSBDevice found..."<<endl;
		m_USBpix = NULL;
		throw USBPixControllerExc(USBPixControllerExc::NOHANDLE, PixControllerExc::FATAL, getCtrlName()); 
	}
        // refresh readback ID
        m_boardIDRB = m_BoardHandle[0]->GetId();
}

void USBPixController::initHW() {
	
	if(UPC_DEBUG_GEN){
		cout<<"INFO: FPGA_filename="<<m_FPGA_filename<<endl;
		cout<<"INFO: uC_filename="<<m_uC_filename<<endl;
		if (m_MultiChipWithSingleBoard)
		{
			cout << "INFO: Reading multiple chips with single "
				"board" << endl;
		}
		else
		{
			cout << "INFO: Reading multiple chips with two "
				"boards" << endl;
		}
	}
	// very simple check for consistency between bit FW file and adapter card
	if((m_AdapterCardFlavor==2 && m_FPGA_filename.find("gpac")==std::string::npos) || 
	   (m_AdapterCardFlavor<2 && m_FPGA_filename.find("usbpix")==std::string::npos) )
	  throw USBPixControllerExc(USBPixControllerExc::WRONGFW, PixControllerExc::FATAL, getCtrlName()); 
	  

	if (m_MultiChipWithSingleBoard)
	{
		initHWSingleBoard();
	}
	else
	{
		initHWMultipleBoards();
	}	
}

void USBPixController::testHW() {
  for(int ib=0;ib<NBOARDS_MAX;ib++){
    if(m_BoardHandle[ib]!=0){
      if(UPC_DEBUG_GEN) cout <<"INFO: found "<< m_BoardHandle[ib]->GetName() << " with ID " << m_BoardHandle[ib]->GetId() <<endl;
    }else if(ib==0){ // don't complain about missing slave board, not always present
      throw USBPixControllerExc(USBPixControllerExc::NOBOARD, PixControllerExc::FATAL, getModGroup().getRodName(), m_boardID[ib]); 
    }
  }
  return;
}
void USBPixController::updateRegs(){
	if(m_BoardHandle[0]!=0){
	        if(UPC_DEBUG_GEN) cout<<"INFO: starting USBPixController::updateRegs" << endl;
		// set cable length
		m_USBpix->SetCableLengthReg(m_cableLength);
		// set cmd register
		m_USBpix->setSelCMD(m_selCmd);
		if(m_slowCtrlMode){
			if(UPC_DEBUG_GEN) cout<<"setting USBPix to slow control mode"<<endl;
			m_USBpix->SetSlowControlMode();
		}else{
			if(UPC_DEBUG_GEN) cout<<"setting USBPix to CMD mode"<<endl;
			m_USBpix->SetCMDMode();
		}
		if(m_enableCmdLvl1)
			m_USBpix->enableCMDLV1();
		else
			m_USBpix->disableCMDLV1();

		// IoMuxSel
		int data, dataRB = m_USBpix->ReadRegister(CS_INMUX_CONTROL);
		data = (0x7 & m_ioMuxSel) + (dataRB & 0x7ffffff8);
		m_USBpix->WriteRegister(CS_INMUX_CONTROL, data);
		if(UPC_DEBUG_GEN) cout<<"setting USBPix to IoMuxSel "<<m_ioMuxSel << ": dataRB " << dataRB << " and data " << data << endl;
		// IoMuxIn
		dataRB = m_USBpix->ReadRegister(CS_INMUX_IN_CTRL); 
		data = (dataRB & 0x7ffffff0) + (0xf & m_ioMuxIn);
		m_USBpix->WriteRegister(CS_INMUX_IN_CTRL, data);
		if(UPC_DEBUG_GEN) cout<<"setting USBPix to IoMuxIo "<<m_ioMuxIn << ": dataRB " << dataRB << " and data " << data << endl;
		// aux clk settings
		int acReg = (int)pow(2., m_auxClkDiv-6); // config. value is in powers of 2, -6 because of offset in FPGA
		acReg = ((acReg)/2) >>1; // modify by factor of 2 and clip LSB to match Malte's definition
		m_USBpix->SetAuxClkFreq(acReg);
		if(UPC_DEBUG_GEN) cout<<"setting USBPix to aux clk. freq of "<< (40./pow(2., m_auxClkDiv-6)) << " (i.e. reg. to " << acReg << ")" << endl;

		// Iref pad settings
		int invIref = (~(m_irefPads&0xf))&0xf;
		m_USBpix->WriteRegister(CS_IREF_PAD_SELECT, m_invIrefPads?invIref:m_irefPads);
		if(UPC_DEBUG_GEN){
		  cout<<"setting USBPix to Iref pad of "<<  (m_invIrefPads?invIref:m_irefPads) ;
		  if(m_invIrefPads) cout << " (was " << (m_irefPads&0xf) << " before bit flip)";
		  cout << endl;
		}

		// Set Adapter Card Flavor and FE to read (same FPGA register)
		int domuxdata = (m_AdapterCardFlavor&0x01) << 2;
		domuxdata = domuxdata + (m_feToRead&0x03); // write 0 for channel 4, 1 2 3 for channel 1 2 3...
		m_USBpix->WriteRegister(CS_DOMUX_CONTROL, domuxdata);
		if(UPC_DEBUG_GEN) cout<<"setting USBPix reg 25 (DOMUX ctrl.) to " << domuxdata << endl;
		if(UPC_DEBUG_GEN) cout<<"m_AdapterCardFlavor was  " << m_AdapterCardFlavor << endl;
		if(UPC_DEBUG_GEN) cout<<"m_feToRead was  " << m_feToRead << endl;


		// Manchester encoding
		m_USBpix->EnableManEnc(m_enaManCode);
		m_USBpix->SetManEncPhase(m_manCodePhase);
	}
	return;  
}
void USBPixController::sendCommand(Bits , int ) {
  // not implemented
  throw USBPixControllerExc(USBPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR, getCtrlName()); 
}

void USBPixController::sendCommand(int command, int ){

  if(command==PixModuleGroup::PMG_CMD_HRESET_ON || command==PixModuleGroup::PMG_CMD_HRESET_OFF){
   int data = 0x0;
   if(command==PixModuleGroup::PMG_CMD_HRESET_ON)
     data = 0x7;
   m_USBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
   return;
 }
 if(command==PixModuleGroup::PMG_CMD_HRESET_PLS){
   int data = 0x0;
   m_USBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
   sleep(100); // check length
   data = 0x7;
   m_USBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
   return;
 }
 if(command==PixModuleGroup::PMG_CMD_SRESET){
   for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
     {
       if(*it==999) break;
       m_USBpix->WriteCommand(FE_GLOBAL_RESET, *it);
     }
 }
 if(command==PixModuleGroup::PMG_CMD_TRIGGER){
   for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
     {
       if(*it==999) break;
       m_USBpix->WriteCommand(FE_EN_DATA_TAKE, *it);
     }
   unsigned char reg_data[7]={0xff,0x01,0x03,0x05,1,0xff,0x04};
   m_USBpix->WriteStrbSave(reg_data);
   m_USBpix->WriteStrbStart();
 }
 if(command==PixModuleGroup::PMG_CMD_ECR){
   for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
     {
       if(*it==999) break;
       m_USBpix->WriteCommand(FE_ECR, *it);
     }
 }
 if(command==PixModuleGroup::PMG_CMD_BCR){
   for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
     {
       if(*it==999) break;
       m_USBpix->WriteCommand(FE_BCR, *it);
     }
 }
}

// ************ Pixel module functions ****************


void USBPixController::writeModuleConfig(PixModule& mod) {
  std::vector<int> newChipIds;

  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called writeModuleConfig()"<<endl;

  int moduleId = mod.moduleId();

  // only use module that matches adapter board channel if in burn-in mode
  if(!moduleActive(moduleId)){
    if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeModuleConfig() ignores module ID "<< moduleId << " since not selected on adapter board"<<endl;
    return;
  }
  
  // neded for 2-FE-config.
  int nFe=0;
  for(std::vector<PixFe*>::iterator fe = mod.feBegin(); fe != mod.feEnd(); fe++){
    if(dynamic_cast<PixFeI4A*>(*fe)!=0){
      if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl::writeModuleConfig() : FE"<<nFe<<" is of flavour I4A" << endl;
      if(nFe==0) m_USBpix->MakeMeFEI4A();
      nFe++;
      int address = ((ConfInt&)(*fe)->config()["Misc"]["Address"]).getValue(); 
      newChipIds.push_back(address);
    }
    else if(dynamic_cast<PixFeI4B*>(*fe)!=0){
      if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl::writeModuleConfig() : FE"<<nFe<<" is of flavour I4B" << endl;
      if(nFe==0) m_USBpix->MakeMeFEI4B();
      nFe++;
      int address = ((ConfInt&)(*fe)->config()["Misc"]["Address"]).getValue(); 
      newChipIds.push_back(address);
    }
    else
      if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl::writeModuleConfig() : FE"<<nFe<<" is of unknown flavour" << endl;
  }

  if (m_chipIds.empty())
  {
    m_chipIds = newChipIds;
    if ((m_MultiChipWithSingleBoard) && (m_boardID[0] != 0))
    {
      m_USBpix->initializeChips(m_chipIds);
    }

    updateRegs();
  }

  // check for consistency (has been canceled, the stuff that happens before
  // here is just not consistent at all)
  //if((nFe==1 && m_chipIds.at(1)!=999) || (nFe==2 && m_chipIds.at(1)==999) || nFe<1 || nFe>2)
  //  throw USBPixControllerExc(USBPixControllerExc::INCONS_FE_CFG, PixControllerExc::WARNING, getCtrlName()); 

  nFe=0;

  for(std::vector<PixFe*>::iterator fe = mod.feBegin(); fe != mod.feEnd(); fe++){

    // copy FE global register content
    int number = (*fe)->number(); // FE index in PixModule
    int address = ((ConfInt&)(*fe)->config()["Misc"]["Address"]).getValue(); // geographical address - this is the identifier for the TTC signal
    
    // fill global register structures of m_USBpix one by one
    if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl::writeModuleConfig() : new geogr. addr. of chip " << std::dec << nFe << ", index " << number
                          << ": "  << address << ", old ID: " << m_chipIds.at(nFe) << endl;
    
    if (m_MultiChipWithSingleBoard)
    {
      m_USBpix->SetChipAddByIndex(address, nFe);
    }
    else
    {
      m_USBpix->SetChipAdd(address, m_chipIds.at(nFe));
    }
    newChipIds.at(nFe) = address;

    if(dynamic_cast<PixFeI4A*>(*fe)!=0){
      m_USBpix->SetGlobalVal(TRIGCNT       , (*fe)->readGlobRegister("TrigCnt")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CONFADDRENABLE, (*fe)->readGlobRegister("Conf_AddrEnable")  , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CFGSPARE2     , (*fe)->readGlobRegister("Reg2Spare")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(ERRMASK0      , (*fe)->readGlobRegister("ErrMask0")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(ERRMASK1      , (*fe)->readGlobRegister("ErrMask1")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBP_R     , (*fe)->readGlobRegister("PrmpVbpRight")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(VTHIN         , (*fe)->readGlobRegister("Vthin")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DISVBN_CPPM   , (*fe)->readGlobRegister("DisVbn_CPPM")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBP       , (*fe)->readGlobRegister("PrmpVbp")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(TDACVBP       , (*fe)->readGlobRegister("TdacVbp")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DISVBN        , (*fe)->readGlobRegister("DisVbn")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(AMP2VBN       , (*fe)->readGlobRegister("Amp2Vbn")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(AMP2VBPFOL    , (*fe)->readGlobRegister("Amp2VbpFol")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBP_T     , (*fe)->readGlobRegister("PrmpVbpTop")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(AMP2VBP       , (*fe)->readGlobRegister("Amp2Vbp")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(FDACVBN       , (*fe)->readGlobRegister("FdacVbn")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(AMP2VBPFF     , (*fe)->readGlobRegister("Amp2Vbpf")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBNFOL    , (*fe)->readGlobRegister("PrmpVbnFol")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBP_L     , (*fe)->readGlobRegister("PrmpVbpLeft")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBPF      , (*fe)->readGlobRegister("PrmpVbpf")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PRMPVBNLCC    , (*fe)->readGlobRegister("PrmpVbnLcc")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG13SPARES   , (*fe)->readGlobRegister("Reg13Spare")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE0     , ((*fe)->readGlobRegister("PxStrobes")>>0) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE1     , ((*fe)->readGlobRegister("PxStrobes")>>1) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE2     , ((*fe)->readGlobRegister("PxStrobes")>>2) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE3     , ((*fe)->readGlobRegister("PxStrobes")>>3) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE4     , ((*fe)->readGlobRegister("PxStrobes")>>4) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE5     , ((*fe)->readGlobRegister("PxStrobes")>>5) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE6     , ((*fe)->readGlobRegister("PxStrobes")>>6) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE7     , ((*fe)->readGlobRegister("PxStrobes")>>7) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE8     , ((*fe)->readGlobRegister("PxStrobes")>>8) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE9     , ((*fe)->readGlobRegister("PxStrobes")>>9) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE10    , ((*fe)->readGlobRegister("PxStrobes")>>10) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE11    , ((*fe)->readGlobRegister("PxStrobes")>>11) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBE12    , ((*fe)->readGlobRegister("PxStrobes")>>12) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBES0    , (*fe)->readGlobRegister("S0")             , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PXSTROBES1    , (*fe)->readGlobRegister("S1")             , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LVDSDRVIREF   , (*fe)->readGlobRegister("LVDSDrvIref")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(BONNDAC       , (*fe)->readGlobRegister("BonnDac")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PLLIBIAS      , (*fe)->readGlobRegister("PllIbias")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LVDSDRVVOS    , (*fe)->readGlobRegister("LVDSDrvVos")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(TEMPDENSIBIAS , (*fe)->readGlobRegister("TempSensBias")   , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PLLICP        , (*fe)->readGlobRegister("PllIcp")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DAC8SPARE1    , (*fe)->readGlobRegister("Reg17Spare")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PLSRLDACRAMP  , (*fe)->readGlobRegister("PlsrIdacRamp")   , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DAC8SPARE2    , (*fe)->readGlobRegister("Reg18Spare")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PLSRVGOAMP    , (*fe)->readGlobRegister("PlsrVgOPamp")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PLSRDACBIAS   , (*fe)->readGlobRegister("PlsrDacBias")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DAC8SPARE5    , (*fe)->readGlobRegister("Reg19Spare")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(VTHIN_ALTCOARSE, (*fe)->readGlobRegister("Vthin_AltCoarse") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(VTHIN_ALTFINE , (*fe)->readGlobRegister("Vthin_AltFine")  , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PLSRDAC       , (*fe)->readGlobRegister("PlsrDAC")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DIGHITIN_SEL  , (*fe)->readGlobRegister("DIGHITIN_Sel")   , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DINJ_OVERRIDE , (*fe)->readGlobRegister("DINJ_Override")  , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(HITLD_IN      , (*fe)->readGlobRegister("HITLD_In")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG21SPARES   , (*fe)->readGlobRegister("Reg21Spare")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG22SPARES1  , (*fe)->readGlobRegister("Reg22Spare2")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(COLPR_ADDR    , (*fe)->readGlobRegister("Colpr_Addr")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(COLPR_MODE    , (*fe)->readGlobRegister("Colpr_Mode")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG22SPARES2  , (*fe)->readGlobRegister("Reg22Spare1")    , newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(KILLDC0-icm , ((*fe)->readGlobRegister("DisableColumnCnfg0")>>icm) & 0x1, newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(KILLDC16-icm, ((*fe)->readGlobRegister("DisableColumnCnfg1")>>icm) & 0x1, newChipIds.at(nFe));
      for(int icm=0;icm<8;icm++)
        m_USBpix->SetGlobalVal(KILLDC32-icm, ((*fe)->readGlobRegister("DisableColumnCnfg2")>>icm) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CHIP_LATENCY  , (*fe)->readGlobRegister("TrigLat")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CMDCNT0_12    , (*fe)->readGlobRegister("CMDcnt") & 0x1fff  , newChipIds.at(nFe));// skip last bit
      m_USBpix->SetGlobalVal(STOPMODECNFG  , (*fe)->readGlobRegister("StopModeCnfg")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(HITDISCCNFG   , (*fe)->readGlobRegister("HitDiscCnfg")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(ENPLL         , (*fe)->readGlobRegister("EN_PLL")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(FE_CLK_PULSE  , (*fe)->readGlobRegister("SR_Clock")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LATCH_EN      , (*fe)->readGlobRegister("Latch_en")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(SR_CLR        , (*fe)->readGlobRegister("SR_clr")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DIG_INJ       , (*fe)->readGlobRegister("CalEn")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG27SPARES   , (*fe)->readGlobRegister("Reg27Spare")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(GATEHITOR     , (*fe)->readGlobRegister("GateHitOr")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(RD_SKIPPED    , (*fe)->readGlobRegister("ReadSkipped")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(RD_ERRORS     , (*fe)->readGlobRegister("ReadErrorReq")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(STOP_CLK      , (*fe)->readGlobRegister("Stop_Clk")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EFUSE_SENSE   , (*fe)->readGlobRegister("Efuse_sense")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CMDCNT13      , ((*fe)->readGlobRegister("CMDcnt") & 0x2000)>>13, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LVDSDRVSET06  , (*fe)->readGlobRegister("LVDSDrvSet06")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG28SPARES   , (*fe)->readGlobRegister("Reg28Spare")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EN_40M        , (*fe)->readGlobRegister("EN40M")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EN_80M        , (*fe)->readGlobRegister("EN80M")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK1_S0       , ((*fe)->readGlobRegister("CLK1")>>0)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK1_S1       , ((*fe)->readGlobRegister("CLK1")>>1)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK1_S2       , ((*fe)->readGlobRegister("CLK1")>>2)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK0_S0       , ((*fe)->readGlobRegister("CLK0")>>0)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK0_S1       , ((*fe)->readGlobRegister("CLK0")>>1)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK0_S2       , ((*fe)->readGlobRegister("CLK0")>>2)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EN_160M       , (*fe)->readGlobRegister("EN160M")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EN_320M       , (*fe)->readGlobRegister("EN320M")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG29SPARES   , (*fe)->readGlobRegister("Reg29Spare1")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(DISABLE8B10B  , (*fe)->readGlobRegister("no8b10b")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(CLK2OUTCFG    , (*fe)->readGlobRegister("Clk2OutCnfg")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EMPTYRECORD   , (*fe)->readGlobRegister("EmptyRecord")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG29SPARE2   , (*fe)->readGlobRegister("Reg29Spare2")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LVDSDRVEN     , (*fe)->readGlobRegister("LVDSDrvEn")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LVDSDRVSET30  , (*fe)->readGlobRegister("LVDSDrvSet30")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(LVDSDRVSET12  , (*fe)->readGlobRegister("LVDSDrvSet12")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(RISEUPTAO     , (*fe)->readGlobRegister("PlsrRiseUpTau")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PULSERPWR     , (*fe)->readGlobRegister("PlsrPwr")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(PULSERDELAY   , (*fe)->readGlobRegister("PlsrDelay")        , newChipIds.at(nFe));
      if(UPC_DEBUG_GEN) cout<<"INFO: USBPix PlsrDelay set to " << ((*fe)->readGlobRegister("PlsrDelay")) << endl;
      m_USBpix->SetGlobalVal(EXTDIGCALSW   , (*fe)->readGlobRegister("ExtDigCalSW")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EXTANCALSW    , (*fe)->readGlobRegister("ExtAnaCalSW")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(REG31SPARES   , (*fe)->readGlobRegister("Reg31Spare")       , newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(EFUSEDC0+icm , ((*fe)->readGlobRegister("SELB0")>>icm) & 0x1 , newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(EFUSEDC16+icm, ((*fe)->readGlobRegister("SELB1")>>icm) & 0x1 , newChipIds.at(nFe));
      for(int icm=0;icm<8;icm++)
        m_USBpix->SetGlobalVal(EFUSEDC32+icm, ((*fe)->readGlobRegister("SELB2")>>icm) & 0x1 , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EFUSECREF     , (*fe)->readGlobRegister("EfuseCref")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EFUSEVREF     , (*fe)->readGlobRegister("EfuseVref")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(EFUSECHIPSERNUM, (*fe)->readGlobRegister("Chip_SN")         , newChipIds.at(nFe));

    } else if(dynamic_cast<PixFeI4B*>(*fe)!=0){
      m_USBpix->SetGlobalVal(B_REG1SPARE     , (*fe)->readGlobRegister("Reg1Spare")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_SMALLHITERASE , (*fe)->readGlobRegister("SmallHitErase")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EVENTLIMIT    , (*fe)->readGlobRegister("Eventlimit")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_TRIGCNT       , (*fe)->readGlobRegister("TrigCnt")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CONFADDRENABLE, (*fe)->readGlobRegister("Conf_AddrEnable")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CFGSPARE2     , (*fe)->readGlobRegister("Reg2Spare")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ERRMASK0      , (*fe)->readGlobRegister("ErrMask0")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ERRMASK1      , (*fe)->readGlobRegister("ErrMask1")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBP_R     , (*fe)->readGlobRegister("PrmpVbpRight")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_BUFVGOPAMP    , (*fe)->readGlobRegister("BufVgOpAmp")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG6SPARE     , (*fe)->readGlobRegister("Reg6Spare")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBP       , (*fe)->readGlobRegister("PrmpVbp")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_TDACVBP       , (*fe)->readGlobRegister("TdacVbp")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_DISVBN        , (*fe)->readGlobRegister("DisVbn")             , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_AMP2VBN       , (*fe)->readGlobRegister("Amp2Vbn")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_AMP2VBPFOL    , (*fe)->readGlobRegister("Amp2VbpFol")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG9SPARE     , (*fe)->readGlobRegister("Reg9Spare")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_AMP2VBP       , (*fe)->readGlobRegister("Amp2Vbp")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_FDACVBN       , (*fe)->readGlobRegister("FdacVbn")            , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_AMP2VBPFF     , (*fe)->readGlobRegister("Amp2Vbpf")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBNFOL    , (*fe)->readGlobRegister("PrmpVbnFol")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBP_L     , (*fe)->readGlobRegister("PrmpVbpLeft")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBPF      , (*fe)->readGlobRegister("PrmpVbpf")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBNLCC    , (*fe)->readGlobRegister("PrmpVbnLcc")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG13SPARES   , (*fe)->readGlobRegister("Reg13Spare")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE0     , ((*fe)->readGlobRegister("PxStrobes")>>0) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE1     , ((*fe)->readGlobRegister("PxStrobes")>>1) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE2     , ((*fe)->readGlobRegister("PxStrobes")>>2) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE3     , ((*fe)->readGlobRegister("PxStrobes")>>3) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE4     , ((*fe)->readGlobRegister("PxStrobes")>>4) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE5     , ((*fe)->readGlobRegister("PxStrobes")>>5) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE6     , ((*fe)->readGlobRegister("PxStrobes")>>6) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE7     , ((*fe)->readGlobRegister("PxStrobes")>>7) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE8     , ((*fe)->readGlobRegister("PxStrobes")>>8) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE9     , ((*fe)->readGlobRegister("PxStrobes")>>9) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE10    , ((*fe)->readGlobRegister("PxStrobes")>>10)& 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE11    , ((*fe)->readGlobRegister("PxStrobes")>>11)& 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBE12    , ((*fe)->readGlobRegister("PxStrobes")>>12)& 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBES1    , (*fe)->readGlobRegister("S0")                 , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PXSTROBES0    , (*fe)->readGlobRegister("S1")                 , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LVDSDRVIREF   , (*fe)->readGlobRegister("LVDSDrvIref")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ADCOPAMP      , (*fe)->readGlobRegister("GADCOpAmp")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PLLIBIAS      , (*fe)->readGlobRegister("PllIbias")           , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LVDSDRVVOS    , (*fe)->readGlobRegister("LVDSDrvVos")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_TEMPDENSIBIAS , (*fe)->readGlobRegister("TempSensBias")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PLLICP        , (*fe)->readGlobRegister("PllIcp")             , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_DAC8SPARE1    , (*fe)->readGlobRegister("Reg17Spare")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PLSRLDACRAMP  , (*fe)->readGlobRegister("PlsrIdacRamp")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_VREFDIGTUNE   , (*fe)->readGlobRegister("VrefDigTune")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PLSRVGOAMP    , (*fe)->readGlobRegister("PlsrVgOPamp")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PLSRDACBIAS   , (*fe)->readGlobRegister("PlsrDacBias")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_VREFANTUNE    , (*fe)->readGlobRegister("VrefAnTune")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_VTHIN_ALTCOARSE,(*fe)->readGlobRegister("Vthin_AltCoarse")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_VTHIN_ALTFINE , (*fe)->readGlobRegister("Vthin_AltFine")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PLSRDAC       , (*fe)->readGlobRegister("PlsrDAC")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_DIGHITIN_SEL  , (*fe)->readGlobRegister("DIGHITIN_Sel")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_DINJ_OVERRIDE , (*fe)->readGlobRegister("DINJ_Override")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_HITLD_IN      , (*fe)->readGlobRegister("HITLD_In")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG21SPARES   , (*fe)->readGlobRegister("Reg21Spare")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG22SPARES1  , (*fe)->readGlobRegister("Reg22Spare2")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_COLPR_ADDR    , (*fe)->readGlobRegister("Colpr_Addr")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_COLPR_MODE    , (*fe)->readGlobRegister("Colpr_Mode")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG22SPARES2  , (*fe)->readGlobRegister("Reg22Spare1")      , newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(B_KILLDC0-icm , ((*fe)->readGlobRegister("DisableColumnCnfg0")>>icm) & 0x1, newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(B_KILLDC16-icm, ((*fe)->readGlobRegister("DisableColumnCnfg1")>>icm) & 0x1, newChipIds.at(nFe));
      for(int icm=0;icm<8;icm++)
        m_USBpix->SetGlobalVal(B_KILLDC32-icm, ((*fe)->readGlobRegister("DisableColumnCnfg2")>>icm) & 0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CHIP_LATENCY  , (*fe)->readGlobRegister("TrigLat")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CMDCNT0_12    , (*fe)->readGlobRegister("CMDcnt") & 0x1fff  , newChipIds.at(nFe));// skip last bit
      m_USBpix->SetGlobalVal(B_STOPMODECNFG , (*fe)->readGlobRegister("StopModeCnfg"), newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_HITDISCCNFG  , (*fe)->readGlobRegister("HitDiscCnfg") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ENPLL        , (*fe)->readGlobRegister("EN_PLL")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_SR_CLR       , (*fe)->readGlobRegister("SR_clr")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LATCH_EN     , (*fe)->readGlobRegister("Latch_en")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_FE_CLK_PULSE , (*fe)->readGlobRegister("SR_Clock")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_DIG_INJ      , (*fe)->readGlobRegister("CalEn")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG27SPARES2 , (*fe)->readGlobRegister("Reg27Spare2") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_GATEHITOR    , (*fe)->readGlobRegister("GateHitOr")   , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG27SPARE1  , (*fe)->readGlobRegister("Reg27Spare1") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_RD_ERRORS    , (*fe)->readGlobRegister("ReadErrorReq"), newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_STOP_CLK     , (*fe)->readGlobRegister("Stop_Clk")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EFUSE_SENSE  , (*fe)->readGlobRegister("Efuse_sense") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ADC_EN_PULSE , (*fe)->readGlobRegister("GADC_Enable") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_SR_RD_EN     , (*fe)->readGlobRegister("ShiftReadBack"), newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CMDCNT13     , ((*fe)->readGlobRegister("CMDcnt") & 0x2000)>>13, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LVDSDRVSET06 , (*fe)->readGlobRegister("LVDSDrvSet06")  , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG28SPARES  , (*fe)->readGlobRegister("Reg28Spare")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EN_40M       , (*fe)->readGlobRegister("EN40M")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EN_80M       , (*fe)->readGlobRegister("EN80M")         , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK1_S0       , ((*fe)->readGlobRegister("CLK1")>>0)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK1_S1       , ((*fe)->readGlobRegister("CLK1")>>1)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK1_S2       , ((*fe)->readGlobRegister("CLK1")>>2)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK0_S0       , ((*fe)->readGlobRegister("CLK0")>>0)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK0_S1       , ((*fe)->readGlobRegister("CLK0")>>1)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK0_S2       , ((*fe)->readGlobRegister("CLK0")>>2)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EN_160M        , (*fe)->readGlobRegister("EN160M")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EN_320M        , (*fe)->readGlobRegister("EN320M")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG29SPARES    , (*fe)->readGlobRegister("Reg29Spare1") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_DISABLE8B10B   , (*fe)->readGlobRegister("no8b10b")     , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_CLK2OUTCFG     , (*fe)->readGlobRegister("Clk2OutCnfg") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EMPTYRECORD    , (*fe)->readGlobRegister("EmptyRecord") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG29SPARE2    , (*fe)->readGlobRegister("Reg29Spare2") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LVDSDRVEN      , (*fe)->readGlobRegister("LVDSDrvEn")   , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LVDSDRVSET30   , (*fe)->readGlobRegister("LVDSDrvSet30"), newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_LVDSDRVSET12   , (*fe)->readGlobRegister("LVDSDrvSet12"), newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_TMPSENSED0     , (*fe)->readGlobRegister("TempSensDiodeSel") &0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_TMPSENSED1     , ((*fe)->readGlobRegister("TempSensDiodeSel")>>1)&0x1, newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_TMPSENSEDISABLE, (*fe)->readGlobRegister("TempSensDisable")  , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ILEAKRANGE     , (*fe)->readGlobRegister("IleakRange")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG30SPARES    , (*fe)->readGlobRegister("Reg30Spare")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_RISEUPTAO      , (*fe)->readGlobRegister("PlsrRiseUpTau")    , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PULSERPWR      , (*fe)->readGlobRegister("PlsrPwr")          , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PULSERDELAY    , (*fe)->readGlobRegister("PlsrDelay")        , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EXTDIGCALSW    , (*fe)->readGlobRegister("ExtDigCalSW")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EXTANCALSW     , (*fe)->readGlobRegister("ExtAnaCalSW")      , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG31SPARES    , (*fe)->readGlobRegister("Reg31Spare")       , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_ADCSELECT      , (*fe)->readGlobRegister("GADCSel")          , newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(B_EFUSEDC0+icm , ((*fe)->readGlobRegister("SELB0")>>icm) & 0x1 , newChipIds.at(nFe));
      for(int icm=0;icm<16;icm++)
        m_USBpix->SetGlobalVal(B_EFUSEDC16+icm, ((*fe)->readGlobRegister("SELB1")>>icm) & 0x1 , newChipIds.at(nFe));
      for(int icm=0;icm<8;icm++)
        m_USBpix->SetGlobalVal(B_EFUSEDC32+icm, ((*fe)->readGlobRegister("SELB2")>>icm) & 0x1 , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG34SPARES1   , (*fe)->readGlobRegister("Reg34Spare1") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_PRMPVBPMSNEN   , (*fe)->readGlobRegister("PrmpVbpMsnEn"), newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_REG34SPARES2   , (*fe)->readGlobRegister("Reg34Spare2") , newChipIds.at(nFe));
      m_USBpix->SetGlobalVal(B_EFUSECHIPSERNUM, (*fe)->readGlobRegister("Chip_SN")     , newChipIds.at(nFe));
      
    } else // unhandled FE type, can't process
      throw USBPixControllerExc(USBPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::ERROR, 
				getModGroup().getRodName());    
    
    if(nFe==0){
      if((*fe)->readGlobRegister("no8b10b")){
        m_USBpix->disable_8b10_Decoding();
        if(UPC_DEBUG_GEN) cout<<"INFO: USBPix 8b10b disabled" << endl;
      }else{
        m_USBpix->enable_8b10_Decoding();
        if(UPC_DEBUG_GEN) cout<<"INFO: USBPix 8b10b enabled" << endl;
      }
    }
    
    // ********* GlobalRegister done! ***************

    // ********* Copy FE pixel register content ***************
    std::string pixName[4];
    pixName[0]="ENABLE";
    pixName[1]="CAP1";
    pixName[2]="CAP0";
    pixName[3]="ILEAK";
    
    // update, just to be on the safe side
    m_nColFe = (*fe)->nCol();
    m_nRowFe = (*fe)->nRow();

    // loop over individual pixel register bits and fill m_USBpix
    const int nlatches = 13, itdac=4, ifdac=9, nWords=(m_nColFe*m_nRowFe)/32;
    int *trimBits = new int[nWords];
    int latches[nlatches]={ENABLE, CAP1, CAP0, HITBUS, TDAC0, TDAC1, TDAC2, TDAC3, TDAC4, FDAC0, FDAC1, FDAC2, FDAC3};
    for(int i=0;i<nlatches;i++){
      
      int bit_offs = i;
      for(int j=0;j<nWords;j++) trimBits[j] = 0;
      
      for(unsigned int col=0; col < m_nColFe; col++){
        for(unsigned int row=0; row < m_nRowFe; row++){
          // determine pixel index as used in dll
          int pixnr = 0;
          int pixint = 0;
          if (0x01 & col)
            pixnr = (col-1) * m_nRowFe + m_nRowFe - row - 1;
          else
            pixnr = (col+1) * m_nRowFe + row;
          pixint = (pixnr / 32);
          int k = 31-(pixnr%32);
          // merge pixel register bits into 32-bit words
          if(i>=ifdac){ //FDAC starts here
            bit_offs = i-ifdac; // start again from 0
            trimBits[pixint] |= (int)((((*fe)->readTrim("FDAC")[col][row]>>bit_offs)&0x1)<<k);
          }else if(i>=itdac){ //TDAC starts here
            bit_offs = i-itdac; // start again from 0
            trimBits[pixint] |= (int)((((*fe)->readTrim("TDAC")[col][row]>>bit_offs)&0x1)<<k);
          }else{         //masks
            trimBits[pixint] |= ((int)(*fe)->readPixRegister(pixName[i])[col][row])<<k;
          }
        }
      }
      
      // store in register structures of m_USBpix
      for(int j=0;j<nWords;j++)
        m_USBpix->SetPixelVal(PIXEL26880+j, trimBits[j], latches[i], newChipIds.at(nFe));
    }
    delete[] trimBits;

    // ********* PixelRegister done! ***************
    nFe++;
  }
  m_chipIds = newChipIds;

  PixCcpd* ccpd = mod.pixCCPD();
  if(ccpd!=0){
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeModuleConfig() CCPD included" << endl;
	writeCcpdConfig(mod);
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeModuleConfig() CCPD included before m_withCCPD " << m_withCCPD << endl;
	m_withCCPD=true;
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeModuleConfig() CCPD included after m_withCCPD " << m_withCCPD << endl;
  }

  int demux_ctrl = 0;
  for (int i = 0; i < 4; i++)
  {
    demux_ctrl <<= 2;
    switch(m_inputDivider[3-i])
    {
      case 0:
        demux_ctrl |= 0x0;
        break;
      case 1:
        demux_ctrl |= 0x1;
        break;
      case 2:
        demux_ctrl |= 0x2;
        break;
      default:
        if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
          ": Unknown divider value, defaults to 1:1" << endl;
        demux_ctrl |= 0x0;
        break;
    }
  }
  if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
    ": CS_TIME_DEMULTIPLEXER_CTRL <= " << std::hex << demux_ctrl << std::dec
    << endl;
  m_USBpix->WriteRegister(CS_TIME_DEMULTIPLEXER_CTRL, demux_ctrl);

  int input_channel_map = 0;
  switch(m_readoutChannelInput[0])
  {
    case 0:
      input_channel_map |= 0x1;
      break;
    case 1:
      input_channel_map |= 0x0;
      break;
    default:
      if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
        ": Invalid readout channel map" << endl;
      break;
  }

  switch(m_readoutChannelInput[1])
  {
    case 0:
      input_channel_map |= 0x3 << 2;
      break;
    case 1:
      input_channel_map |= 0x0;
      break;
    case 2:
      input_channel_map |= 0x1 << 2;
      break;
    default:
      if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
        ": Invalid readout channel map" << endl;
      break;
  }

  switch(m_readoutChannelInput[2])
  {
    case 0:
      input_channel_map |= 0x3 << 4;
      break;
    case 1:
      input_channel_map |= 0x0;
      break;
    case 2:
      input_channel_map |= 0x1 << 4;
      break;
    default:
      if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
        ": Invalid readout channel map" << endl;
      break;
  }

  switch(m_readoutChannelInput[3])
  {
    case 0:
      input_channel_map |= 0x3 << 6 | 0 << 1;
      break;
    case 1:
      input_channel_map |= 0x0 << 6 | 0 << 1;
      break;
    case 2:
      input_channel_map |= 0x1 << 6 | 0 << 1;
      break;
    case 3:
      input_channel_map |= 0x2 << 6 | 0 << 1;
      break;
    case 4:
      input_channel_map |= 0x0 << 6 | 1 << 1;
      break;
    default:
      if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
        ": Invalid readout channel map" << endl;
      break;
  }

  if(UPC_DEBUG_GEN) cout<< __FILE__ << ":" << __LINE__ << 
    ": CS_RO_INPUT_SWITCH_MAP <= " << std::hex << input_channel_map << std::dec
    << endl;
  m_USBpix->WriteRegister(CS_RO_INPUT_SWITCH_MAP, input_channel_map);
  m_configValid = true;
  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: end of writeModuleConfig()"<<endl;
}

void USBPixController::readModuleConfig(PixModule& /*mod*/) {
	if(m_USBpix!=NULL) {
    for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
    {
      if (*it==999) break;
			m_USBpix->ReadGlobal(*it);
		}
	}
}

void USBPixController::sendGlobal(unsigned int /*moduleMask*/){
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called general sendGlobal "<<endl;

	if(m_USBpix!=NULL && m_configValid){
		//for(int ib=0;ib<NBOARDS_MAX;ib++){
    for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
    {
      if (*it==999) break;
			m_USBpix->WriteGlobal(*it);
		}
    configureReadoutChannelAssociation();
	} else {
		throw USBPixControllerExc(USBPixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
	}
  
}
void USBPixController::sendGlobal(unsigned int /*moduleMask*/, std::string regName){
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendGlobal for reg. "<< regName<<endl;

	int regNo = m_globRegNames[regName];
	if(m_USBpix!=NULL && m_configValid && regNo>0){
    for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
    {
      if (*it==999) break;
			m_USBpix->WriteGlobalSingleReg(regNo, *it);
		}
	} else {
		throw USBPixControllerExc(USBPixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
	}
}
int  USBPixController::readGlobal(int regNo, int feInd){
	int val = 0;
	if(m_USBpix!=NULL && m_configValid && regNo>0){
		val = m_USBpix->ReadGlobalSingleReg(regNo, m_chipIds.at(feInd));
	} else {
		throw USBPixControllerExc(USBPixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
	}
	return val;
}
void USBPixController::sendPixel(unsigned int moduleMask){
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendPixel"<<endl;
	for (std::map<std::string, int>::iterator it = m_latchNames.begin(); it!=m_latchNames.end(); it++)
	  sendPixel(moduleMask, it->first, -1, false);
	sendGlobal(0);  // M.B. Maybe this fixes a problem in DC module cfg.?
}
void USBPixController::sendPixel(unsigned int moduleMask, std::string regName, bool allDcsIdentical) {
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendPixel for reg. "<< regName<<endl;
	sendPixel(moduleMask, regName, allDcsIdentical?41:(-1), true);
}
void USBPixController::sendPixel(unsigned int moduleMask, std::string regName, int DC){
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendPixel for reg. "<< regName << " and DC " << DC <<endl;
	sendPixel(moduleMask, regName, DC, true);
}
void USBPixController::sendPixel(unsigned int /*moduleMask*/, std::string regName, int DC, bool sendGlob){

	int latch=m_latchNames[regName]-1;
	int DCmin=0, DCmax=40;
	if(DC>=0 && DC<40){
		DCmin = DC;
		DCmax = DC+1;
	}
	if(m_USBpix!=NULL && m_configValid && latch>=0){
    for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
    {
      if (*it==999) break;
			if(DC==41) // identical content in all DCs, use fast writing
			  m_USBpix->WritePixelSingleLatch(latch, *it);
			else{
			  for (int doublecolumn = DCmin; doublecolumn < DCmax; doublecolumn++){
			    m_USBpix->WritePixelSingleLatchDC(latch,doublecolumn, *it);
			  }
			}
		}
	} else {
		throw USBPixControllerExc(USBPixControllerExc::INIT_ERROR, PixControllerExc::ERROR, getModGroup().getRodName()); 
	}

	if(sendGlob) sendGlobal(0);  // M.B. Maybe this fixes a problem in DC module cfg.? - not needed for repeated calls
}

void USBPixController::sendModuleConfig(unsigned int moduleMask) {
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendModuleConfig"<<endl;
	// call sendPixel, has become identical by now
	sendPixel(moduleMask);
	if(m_withCCPD){ 
		if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: sendModuleConfig m_withCCPD true"<<endl;
		sendCCPD();
	}
}

void USBPixController::sendModuleConfig(PixModule& mod) {
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendModuleConfig"<<endl;
	writeModuleConfig(mod);
	sendModuleConfig(0);
}

bool USBPixController::testGlobalRegister(int module, std::vector<int> &data_in, std::vector<int> &data_out, 
					  std::vector<std::string> &label, bool sendCfg, int feIndex){

  if(UPC_DEBUG_GEN) cout << "Global register test" << std::endl;

  // only use module that matches adapter board channel if in burn-in mode
  if(!moduleActive(module)) return false;

  bool retVal = false;
  // check if requested FE exists in config.
  if (m_chipIds.at(feIndex) == 999) 
    return retVal;
  
  if(UPC_DEBUG_GEN) cout << "starting GR test for FE type " << (m_USBpix->FEisFEI4B()?"B":"A") << endl;
  if(m_USBpix!=0 && m_configValid){
    if(sendCfg){
      if(UPC_DEBUG_GEN) cout << "GR test - write global" << endl;
      m_USBpix->WriteGlobal(m_chipIds.at(feIndex));
    }
    if(UPC_DEBUG_GEN) cout << "GR test - read global" << endl;
    m_USBpix->ReadGlobal(m_chipIds.at(feIndex));
    if(UPC_DEBUG_GEN) cout << "GR test - comparison" << endl;
    retVal = true;
    int Address, Size, wValue, rValue;
    int imin=TRIGCNT, imax=READCMDERR, iign=EFUSECHIPSERNUM;
    if(m_USBpix->FEisFEI4B()){
      imin = B_REG1SPARE;
      imax = B_READCMDERR;
      iign = B_EFUSECHIPSERNUM;
    }
    for(int i=imin;i<=imax;i++){
      m_USBpix->GetGlobalVarAddVal(i, Address, Size, wValue, m_chipIds.at(feIndex));
      m_USBpix->GetGlobalRBVarAddVal(i, Address, Size, rValue, m_chipIds.at(feIndex));
      if(i<iign){ // ignore read-only registers
	data_in.push_back(wValue);
	data_out.push_back(rValue);
	label.push_back(m_USBpix->getGlobalVarName(i));
	if(wValue!=rValue) retVal = false;
      } else{ // fill in read value for information
	data_in.push_back(rValue); // shouldn't here also the write value be pushed for information?
	                           // JGK: no, would confuse later analysis
	data_out.push_back(rValue);
	label.push_back(m_USBpix->getGlobalVarName(i));
      }
    }
  }
  return retVal;
}

bool USBPixController::testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out, 
					 bool ignoreDCsOff, int DC, bool sendCfg, int feIndex, bool bypass) {

  if(UPC_DEBUG_GEN) cout << "Pixel register test in " << (bypass?"bypass":"normal") << " mode " << endl;

  // only use module that matches adapter board channel if in burn-in mode
  if(!moduleActive(module)) return false;

	bool retVal = false;
	if(feIndex<0 || feIndex>=((int) m_chipIds.size())) return retVal;
	if(m_chipIds.at(feIndex)==999) return retVal;

	bool allDcsIdentical = false;
	if(DC == 40) allDcsIdentical = true;

	int DCmin=0, DCmax=40;
	if(DC>=0 && DC<40){
		DCmin = DC;
		DCmax = DCmin + 1;
	}

	if(m_USBpix!=0 && m_configValid){
		int Address, Size, wValue, rValue;
		int latch=m_latchNames[regName]-1;
		// get settings of DC kill mask
		int DCmask[40];
		for(int icm=0;icm<40;icm++) DCmask[icm]=0;
		if(ignoreDCsOff){
			for(int icm=0;icm<16;icm++){
			  m_USBpix->GetGlobalVarAddVal((m_USBpix->FEisFEI4B()?B_KILLDC0:KILLDC0)-icm, Address, Size, wValue, m_chipIds.at(feIndex));
				DCmask[icm]=wValue;
			}
			for(int icm=0;icm<16;icm++){
			  m_USBpix->GetGlobalVarAddVal((m_USBpix->FEisFEI4B()?B_KILLDC16:KILLDC16)-icm, Address, Size, wValue, m_chipIds.at(feIndex));
				DCmask[16+icm]=wValue;
			}
			for(int icm=0;icm<8;icm++){
			  m_USBpix->GetGlobalVarAddVal((m_USBpix->FEisFEI4B()?B_KILLDC32:KILLDC32)-icm, Address, Size, wValue, m_chipIds.at(feIndex));
				DCmask[32+icm]=wValue;
			}
		}
		if(latch>=0){
			if(UPC_DEBUG_GEN) cout << regName << " : " << dec << latch << endl;
			if(sendCfg){
				if(allDcsIdentical){
					if(UPC_DEBUG_GEN) cout << "Pixel register test: using WritePixel(latch)" << endl;
					m_USBpix->WritePixelSingleLatch(latch, m_chipIds.at(feIndex));
				}else{
					if(UPC_DEBUG_GEN) cout << "Pixel register test: using WritePixel(latch, DC)" << endl;
					for (int doublecolumn = DCmin; doublecolumn < DCmax; doublecolumn++){
						if(DCmask[doublecolumn]==0) m_USBpix->WritePixelSingleLatchDC(latch,doublecolumn, m_chipIds.at(feIndex));
					}
				}
			}
			for (int doublecolumn = DCmin; doublecolumn < DCmax; doublecolumn++){
				if(DCmask[doublecolumn]==0) m_USBpix->ReadPixelSingleLatchDC(latch,doublecolumn, bypass, m_chipIds.at(feIndex));
			}
			retVal = true;
			int pixCount=0;
			for (int i=PIXEL26880; i<=PIXEL32; i++){
				int DCid = pixCount/21;
				if(DCmask[DCid]==0 && (DC<0 || DC>39 || DCid==DC) ){
					m_USBpix->GetPixelVarAddVal(i, Address, Size, wValue, latch, m_chipIds.at(feIndex));
					m_USBpix->GetPixelRBVarAddVal(i, Address, Size, rValue, latch, bypass, m_chipIds.at(feIndex));
					data_in.push_back(wValue);
					data_out.push_back(rValue);
					if(wValue!=rValue) retVal = false;
				} else{
					//if(UPC_DEBUG_GEN) cout << "excluding DC " << DCid << endl;
					data_in.push_back(0x0);
					data_out.push_back(0x0);
				}
				pixCount++;//--;
			}
			if(UPC_DEBUG_GEN) cout << "PR test return " << (retVal?"true":"false") << " for register " << latch << endl;
		}
	}
	return retVal;
}
// *******************************************************
// ******* Setup for runmodes: Calibration/Data Taking ***
// *******************************************************
void USBPixController::setCalibrationMode() {
  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting USBpix to calib. mode"<<endl;
  m_USBpix->StopMeasurement();
  m_USBpix->StopReadout();
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
    m_USBpix->ClearSRAM(*it);
  }
  m_USBpix->SetCalibrationMode();
}

void USBPixController::setConfigurationMode() {
  setCalibrationMode();
}

void USBPixController::setRunMode() {
  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting USBpix to run mode"<<endl;
  for (std::vector<int>::iterator it = m_chipIds.begin();
       it != m_chipIds.end(); it++)
    {
      if(*it==999) break;
      m_USBpix->ClearSRAM(*it);
    }
  m_USBpix->SetRunMode();
}

// *******************************************************
// ******* Setup FE operation modes **********************
// *******************************************************
void USBPixController::setFEConfigurationMode(){
  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting FE(s) to config. mode"<<endl;
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
    m_USBpix->WriteCommand(FE_CONF_MODE, *it);
  }
}

void USBPixController::setFERunMode() {
  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting FE(s) to run mode"<<endl;
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
    m_USBpix->WriteCommand(FE_EN_DATA_TAKE, *it);
  }
}

// *******************************************************
// ********** EPROM burning/reading **********************
// *******************************************************
void USBPixController::readEPROM()
{
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
		m_USBpix->ReadEPROMvalues(*it);
	}
	if(UPC_DEBUG_GEN) cout << "DEBUG M.B.: USBpixController::readEPROM executed. \n";
}

void USBPixController::burnEPROM()
{
	sleep(1000);
	m_USBpix->BurnEPROMvalues();
	sleep(1000);
	if(UPC_DEBUG_GEN) cout << "DEBUG M.B.: USBpixController::burnEPROM executed. \n";
}

// *******************************************************
// ********** GADC reading *******************************
// *******************************************************

void USBPixController::readGADC(int type, std::vector<int> &GADCvalues, int FEindex)
{
  if(UPC_DEBUG_GEN) cout << "DEBUG: USBpixController::readGADC with type " << type << " called for FE " << FEindex << ". \n";

  int nFe_req = getFECount();
  int ibmin=0, ibmax=nFe_req;
  // check if requested FE exists in config, otherwise assume all FEs need to be processed
  if (FEindex>=0 && m_chipIds.at(FEindex) != 999) {
    ibmin = FEindex;
    ibmax = ibmin + 1;
    nFe_req = 1;
  }

	GADCvalues.clear();
	GADCvalues.resize(3*nFe_req); // GADC results have 3 elements
	if(UPC_DEBUG_GEN) cout << "Result vector cleared and resized. \n";
	// read GADC for all requested FEs. Results in ConfigFEmemory now...
	for(int ib=ibmin;ib<ibmax && ib<(int)m_chipIds.size();ib++){
	  if(m_chipIds.at(ib)==999) break;
	  if(UPC_DEBUG_GEN) cout << "DEBUG: USBpixController::readGADC calls USBpix::ReadGADC for FE GA " << m_chipIds.at(ib) << ". \n";
	  m_USBpix->ReadGADC(type, m_chipIds.at(ib));
	}

	if(UPC_DEBUG_GEN) cout << "GADC digitized. \n";

	// Fill results for all requested FEs.
	for(int ib=0;ib<nFe_req;ib++){
		int add, size, val;
		int real_ib = ib+ibmin;
		m_USBpix->GetGlobalRBVarAddVal(B_GADCOUT, add, size, val, m_chipIds[real_ib]);
		GADCvalues[0+ib*3] = val;
		m_USBpix->GetGlobalRBVarAddVal(B_GADCSTATUS, add, size, val, m_chipIds[real_ib]);
		GADCvalues[1+ib*3] = val;
		m_USBpix->GetGlobalRBVarAddVal(B_GADCSELECTRB, add, size, val, m_chipIds[real_ib]);
		GADCvalues[2+ib*3] = val;
		if(UPC_DEBUG_GEN) cout << "Result vector item " << ib << " filled with: " << 
		  GADCvalues[0+ib*3] << ", " << GADCvalues[1+ib*3] << ", " << GADCvalues[2+ib*3] << 
		  " for chip ID " << m_chipIds[real_ib] << "\n";
	}
}

bool USBPixController::moduleActive(int nmod) {                                      //! True if module is active during scan or run
  // only module that matches adapter board channel can be active if not in multi-chip mode
  if((m_AdapterCardFlavor&0x1)!=0 && !m_MultiChipWithSingleBoard && (nmod+1)!=m_feToRead)
    return false;
  else
    return true;
}

// ***********************************************
// ******* Setup scanning on the board ***********
// ***********************************************

int USBPixController::translateScanParam(PixScan::ScanParam param) {

  int scanVar;
  
  switch(param) {
  default:
    scanVar = -1;
    break;
  case  PixScan::NO_PAR: // needed for fast T/GDAC tuning
    scanVar = m_USBpix->FEisFEI4B()?B_REG13SPARES:REG13SPARES;
    break;
  case PixScan::IF:
    scanVar = m_USBpix->FEisFEI4B()?B_PRMPVBPF:PRMPVBPF;
    break;
  case PixScan::GDAC:
    scanVar = m_USBpix->FEisFEI4B()?B_VTHIN_ALTFINE:VTHIN_ALTFINE;
    break;
  case PixScan::LATENCY:
    scanVar = m_USBpix->FEisFEI4B()?B_CHIP_LATENCY:CHIP_LATENCY;
    break;
  case PixScan::VCAL:
    scanVar = m_USBpix->FEisFEI4B()?B_PLSRDAC:PLSRDAC;
	  break;
  case PixScan::DISCBIAS:
    scanVar = m_USBpix->FEisFEI4B()?B_DISVBN:DISVBN;
    break;
  }
  return scanVar;
}

void USBPixController::writeScanConfig(PixScan &scn) { // write scan parameters
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: entering writeScanConfig()"<<endl;

	// set scan flag
	m_sourceScanFlag = scn.getSourceScanFlag();
        m_testBeamFlag = scn.getTestBeamFlag();

	// get loop swap parameter
	// TODO: use this paramter for fancy scans -> stuck pixel scan, merged bumps scan
	m_innerLoopSwap = scn.getInnerLoopSwap();

	int maskstages=0;
	int value = -1;
	int shiftmask=2;
	const int prWords = (m_nColFe*m_nRowFe/32);
	int *pixelmask = new int[prWords];
	int *pixelmask2 = new int[prWords];
	bool xtalk = false;

	for(int i=0; i<prWords; i++) {
		pixelmask[i]=0;
		pixelmask2[i]=0;
	}

	for(unsigned int i=0; i<NSCANCFG; i++)
		m_scanConfigArray[i] = 0;

	for(unsigned int i=0; i<8; i++)
		m_clusterPars[i] = 0;

	// set trigger count (# of consec. triggers) from scan config to be consistent
	// NB: FE-count of 0 is 16 consec. trigger, so need to consider special case
	int trgcnt = 0; // default, not overwritten for scan-trgcnt=16
	if(scn.getConsecutiveLvl1TrigA(0)<16) trgcnt = scn.getConsecutiveLvl1TrigA(0);
	if(scn.getConsecutiveLvl1TrigA(0)>16){
		throw USBPixControllerExc(USBPixControllerExc::NOT_IMPLEMENTED, PixControllerExc::INFO, getModGroup().getRodName()); 
	}
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
		if(!m_sourceScanFlag && scn.getAlterFeCfg()){
		  m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_HITDISCCNFG:HITDISCCNFG, 0, *it);
		  m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_HITDISCCNFG:HITDISCCNFG),
						 *it);
		  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting "<< (m_USBpix->FEisFEI4B()?"B_":"") << "HITDISCCNFG to 0"<<endl;
		}
		m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_TRIGCNT:TRIGCNT, trgcnt, *it);
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: "<< (m_USBpix->FEisFEI4B()?"B_":"") << "TRIGCNT="<<trgcnt<<endl;
		//JW: set digInject bit in global register
		m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_DIGHITIN_SEL:DIGHITIN_SEL, (int)scn.getDigitalInjection(), *it);
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: "<< (m_USBpix->FEisFEI4B()?"B_":"") << 
		  "DIGHITIN_SEL="<<scn.getDigitalInjection()<<endl;

		// set latency on FE from scan config if not scanned
		if((scn.getLoopParam(0)!=PixScan::LATENCY || !scn.getLoopActive(0)) &&
		   (scn.getLoopParam(1)!=PixScan::LATENCY || !scn.getLoopActive(1)) &&
		   (scn.getLoopParam(2)!=PixScan::LATENCY || !scn.getLoopActive(2))){
		  m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_CHIP_LATENCY:CHIP_LATENCY  , scn.getLVL1Latency(), *it);
		  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: Chip #"<<(*it)<< " Latency="<<scn.getLVL1Latency()<<endl;
		}

		//JW: update global register before scanning
		m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_DIGHITIN_SEL:DIGHITIN_SEL), 
					       *it);
		m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_CHIP_LATENCY:CHIP_LATENCY), 
					       *it);
		m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_TRIGCNT:TRIGCNT), 
					       *it);

		// set strobe delay on FE from scan config if delay isn't scanned
		if((scn.getLoopParam(0)!=PixScan::STROBE_DELAY || !scn.getLoopActive(0)) && 
			(scn.getLoopParam(1)!=PixScan::STROBE_DELAY || !scn.getLoopActive(1)) && 
			(scn.getLoopParam(2)!=PixScan::STROBE_DELAY || !scn.getLoopActive(2)) ){
		  if(scn.getStrobeMCCDelayRange()!=31){
		    m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_PULSERDELAY:PULSERDELAY  , scn.getStrobeMCCDelay(), *it);
		    if(UPC_DEBUG_GEN) cout << "DEBUG USBPixCtrl: Chip #"<<(*it)<< " pulser delay=" << scn.getStrobeMCCDelay() << endl;
		  }
		  m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_PULSERDELAY:PULSERDELAY), 
						 *it);
		}

		// set VCAL on FE from scan config if VCAL isn't scanned
		if((scn.getLoopParam(0)!=PixScan::VCAL || !scn.getLoopActive(0)) && 
			(scn.getLoopParam(1)!=PixScan::VCAL || !scn.getLoopActive(1)) && 
			(scn.getLoopParam(2)!=PixScan::VCAL || !scn.getLoopActive(2))){
		  if(scn.getFeVCal()!=8191){
		    m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_PLSRDAC:PLSRDAC , scn.getFeVCal(), *it);
		    if(UPC_DEBUG_GEN) cout << "DEBUG USBPixCtrl: Chip #"<<(*it)<< " pulser DAC=" << scn.getFeVCal() << endl;
		  }
		  // call this in any case to make sure FE setting is updated
		  m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_PLSRDAC:PLSRDAC), *it);
		}

		if(scn.getSrcTriggerType() == PixScan::FE_SELFTRIGGER)
		{
			// enable selftrigger mode in FE
		  m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_GATEHITOR:GATEHITOR, 1, *it);
		  m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_GATEHITOR:GATEHITOR), 
						 *it);
		  if(UPC_DEBUG_GEN) cout << "USBpixController: Enabled GateHitOR for FE " << (*it) << endl;
		}
	}
  if(scn.getSrcTriggerType() == PixScan::FE_SELFTRIGGER){
    // disable ext. trigger in FPGA - should only be called once, so taken out of loop
    m_USBpix->setTriggerMode(PixScan::USBPIX_SELF_TRG);//disableExtLV1();
    if(UPC_DEBUG_GEN) cout << "USBpixController: set rigger mode to USBPIX_SELF_TRG" << endl;
  }
	//Loop 0
	if(!scn.getLoopActive(0)) {    //JW: loop 0 is not active--> set scan parameter to VCal, scan at current value
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: Scan loop 0 is not active"<<endl;
		value = scn.getFeVCal();
		if(value == 8191){
			// scan something that isn't used and don't touch VCAL at all - otherwise won't work for >1 FE!
		  m_scanConfigArray[0] = m_USBpix->FEisFEI4B()?B_REG13SPARES:REG13SPARES;
			m_scanConfigArray[1] = 0;
			m_scanConfigArray[2] = 0;
		} else{
			m_scanConfigArray[0] = m_USBpix->FEisFEI4B()?B_PLSRDAC:PLSRDAC;
			m_scanConfigArray[1] = value;
			m_scanConfigArray[2] = value;
		}
		m_scanConfigArray[3]=1;
	} else if(scn.getDspProcessing(0)) {
		//JW: Get ScanParameter
		m_scanConfigArray[0] = translateScanParam(scn.getLoopParam(0));
		if(scn.getLoopVarUniform(0)) {
			m_scanConfigArray[1] = (int)scn.getLoopVarMin(0);
			m_scanConfigArray[2] = (int)scn.getLoopVarMax(0);
			if ((scn.getLoopVarNSteps(0) <= 1) || (scn.getLoopVarMax(0) == scn.getLoopVarMin(0))) { // calculate scan step size
				m_scanConfigArray[3] = 0;
			} else {
				m_scanConfigArray[3] = (int)((scn.getLoopVarMax(0) - scn.getLoopVarMin(0)) / (scn.getLoopVarNSteps(0) - 1));
			}
			if(m_scanConfigArray[3]<1) m_scanConfigArray[3]=1;
		} else {
			throw USBPixControllerExc(USBPixControllerExc::NON_UNIFORM_POINTS_NOT_IMPLEMENTED, PixControllerExc::FATAL, getModGroup().getRodName()); 
		}
	} else {        //JW: loop 0 is executed on the host
		// scan something that isn't used and don't touch VCAL at all - otherwise won't work for >1 FE!
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: Scan loop 0 is active but on host; scanning with  REG13SPARES" << endl;
		m_scanConfigArray[0] = m_USBpix->FEisFEI4B()?B_REG13SPARES:REG13SPARES;
		m_scanConfigArray[1] = 0;
		m_scanConfigArray[2] = 0;
		m_scanConfigArray[3] = 1;
	}

	m_scanConfigArray[4] = scn.getRepetitions();     //injectCount

	switch(scn.getMaskStageTotalSteps()) {
	case PixScan::STEPS_1:     maskstages = 1;    break;
	case PixScan::STEPS_2:     maskstages = 2;    break;
	case PixScan::STEPS_3:     maskstages = 3;    break;
	case PixScan::STEPS_4:     maskstages = 4;    break;
	case PixScan::STEPS_6:     maskstages = 6;    break;
	case PixScan::STEPS_8:     maskstages = 8;    break;
	case PixScan::STEPS_32:    maskstages = 32;   break;
	case PixScan::STEPS_336:   maskstages = 336;  break;
	case PixScan::STEPS_672:   maskstages = 672;  break;
	case PixScan::STEPS_26880: maskstages = 26880;break;
	default:                   maskstages = 1;    break;
	}
	
	m_scanConfigArray[5] = scn.getMaskStageStepWidth();  //maskStepSize;
	m_scanConfigArray[6] = scn.getMaskStageSteps();  //maskStepCount
	
	int capMask[3]={SHIFT_CAP1, SHIFT_CAP0, SHIFT_CAP1+SHIFT_CAP0};
	switch(scn.getMaskStageMode()) {
	  // SEL: select mask also contains Clow/Chigh info now, so must deal with this here, too
	case PixScan::SEL:          shiftmask = capMask[scn.getChargeInjCap()]; break;
	case PixScan::ENA:          shiftmask = SHIFT_ENABLE; break;
	case PixScan::SEL_ENA:      shiftmask = SHIFT_ENABLE+capMask[scn.getChargeInjCap()]; break;
	case PixScan::HITB:         shiftmask = SHIFT_HITBUS; break;
	case PixScan::HITB_INV:     shiftmask = SHIFT_INVHB; break;
	case PixScan::ENA_HITB:     shiftmask = SHIFT_ENABLE+SHIFT_HITBUS; break;
	case PixScan::SEL_ENA_HITB: shiftmask = SHIFT_ENABLE+SHIFT_HITBUS+capMask[scn.getChargeInjCap()]; break;
	case PixScan::XTALK:        shiftmask = SHIFT_ENABLE+capMask[scn.getChargeInjCap()]; xtalk = true; break;  
	case PixScan::DINJ:         shiftmask = SHIFT_DIGINJ; break;
	case PixScan::STATIC:       shiftmask = 0x00; break;
	default:                    shiftmask = 0x00; break; // static
	}
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: shift mask is " << shiftmask << endl;

	m_scanConfigArray[7] = shiftmask;  //use this to shift other masks

	if(!scn.getLoopActive(0) || !scn.getDspProcessing(0))                         //JW: loop 0 not active, expect one histogram
		m_scanConfigArray[8]=1; 
	else
		m_scanConfigArray[8]=scn.getLoopVarNSteps(0);   //keep number of steps, makes things easier

	m_scanConfigArray[9]  = (int)scn.getLoopOverDcs();
	m_scanConfigArray[10] = (int)scn.getAvoidSpecialsCols();
	m_scanConfigArray[11] = (int)scn.getSingleDCloop();

	// **** configuration for strobe and LVL1 signals *****
	unsigned char reg_data[7];
	int strb_dur=scn.getStrobeDuration();
	// set the following from scan panel, otherwise cross checks aren't possible
	//if(m_sourceScanFlag) strb_dur = 0;              //source scan: no injection, just LVL1; TODO: use trigger mode, new FPGA FSM
	int frequency = scn.getStrobeFrequency();
	int reduce_lvl1delay = 0;
	if (m_triggerReplication == REPLICATION_SLAVE)         // In trigger replication mode there is a delay between master and slave boards, to fix this, the lvl1 delay can be reduced
		reduce_lvl1delay = scn.getLowerLVL1DelayIfSlaveBy();
	reg_data[0] = (0x00ff & strb_dur);                     //set strobe duration
	reg_data[1] = (0xff00 & strb_dur) >> 8;                //set strobe duration
	reg_data[2] = 0xff & scn.getConsecutiveLvl1TrigA(0);   //set consecutive LVL1
	if(scn.getStrobeLVL1DelayOverride())
		reg_data[3] = (scn.getStrobeLVL1Delay() - reduce_lvl1delay);                //set trigger delay
	else
		reg_data[3] = m_modGroup.getTriggerDelay();
	reg_data[4] = scn.getRepetitions();                    //set number of injections per step
	reg_data[5] = (0x00ff & frequency);                    //set injection frequency
	reg_data[6] = (0xff00 & frequency) >> 8;               //set injection frequency

	if(UPC_DEBUG_GEN){
		cout<<"DEBUG USBPixCtrl: strobe settings are ";
		for(int kk=0;kk<7;kk++) cout << kk << ": " <<  ((int)reg_data[kk]) << ", ";
		cout << endl;
	}
	m_USBpix->WriteStrbSave(reg_data);

  m_USBpix->SetTX2Output(m_tx2signal);
  m_USBpix->SetDisableGpacStrb(!m_enableGPACStrobePin);
  m_USBpix->SetDisableScaStrb(!m_enableSCAStrobePin);
  m_USBpix->SetFineStrbDelay(scn.getStrobeFineDelay());

	if (scn.getSrcTriggerType()==PixScan::STROBE_SCAN || scn.getSrcTriggerType()==PixScan::STROBE_EXTTRG || 
	    scn.getSrcTriggerType()==PixScan::STROBE_USBPIX_SELF_TRG || scn.getSrcTriggerType()==PixScan::STROBE_FE_SELF_TRG ||
	    scn.getSrcTriggerType()==PixScan::CCPD_STROBE_TRG) {
	  m_USBpix->disableTriggerReplicationMaster();
	  if(scn.getSrcTriggerType()==PixScan::STROBE_SCAN){
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->disableExtLV1()"<<endl;
	    m_USBpix->setTriggerMode(PixScan::STROBE_SCAN);
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: STROBE_SCAN trigger mode set" << " - Board-ID: " << m_boardID[0] << endl;
	  } else if (scn.getSrcTriggerType()==PixScan::STROBE_USBPIX_SELF_TRG){
	    m_USBpix->setTriggerMode(PixScan::USBPIX_SELF_TRG); // USBpixSelftrigger should work without LEMO connection now.
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: USBPIX_SELF_TRG trigger mode set" << " - Board-ID: " << m_boardID[0] << endl;
	  } else if(scn.getSrcTriggerType()==PixScan::STROBE_FE_SELF_TRG){
	    // enable selftrigger mode in FE
	    for (std::vector<int>::iterator it = m_chipIds.begin();
		 it != m_chipIds.end(); it++)
	      {
		if(*it==999) break;
		m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_GATEHITOR:GATEHITOR, 1, *it);
		m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_GATEHITOR:GATEHITOR), 
					       *it);
	      }
	    // disable ext. trigger in FPGA
	    m_USBpix->setTriggerMode(PixScan::USBPIX_SELF_TRG);
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: FE_SELFTRIGGER trigger mode set" << " - Board-ID: " << m_boardID[0] << endl;
	  } else if (scn.getSrcTriggerType()==PixScan::CCPD_STROBE_TRG) {
	    m_USBpix->setTriggerMode(PixScan::STROBE_SCAN);
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: CCPD_EXT_TRG trigger mode set" << " - Board-ID: " << m_boardID[0] << endl;
	  } else {
	    m_USBpix->setTriggerMode(PixScan::EXT_TRG);
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: EXT_TRG trigger mode set" << " - Board-ID: " << m_boardID[0] << endl;
	  }
	} else {
	  if (scn.getSrcTriggerType()!=PixScan::USBPIX_SELF_TRG && scn.getSrcTriggerType()!=PixScan::FE_SELFTRIGGER){
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->enableExtLV1()"<<endl;
	    m_USBpix->enableExtLV1(); // JGK: really needed?
	  }
	  if (m_triggerReplication == REPLICATION_SLAVE) {
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: INIT setting Trigger-Mode to 5 - Board-ID: " << m_boardID[0] << endl;
	    m_USBpix->setTriggerMode(PixScan::USBPIX_REPLICATION_SLAVE);
	  } else {
	    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: INIT setting Trigger-Mode to " << scn.getSrcTriggerType() << " - Board-ID: " << m_boardID[0] << endl;
	    m_USBpix->setTriggerMode(scn.getSrcTriggerType());
	    
	    if (m_triggerReplication == REPLICATION_MASTER)
	      m_USBpix->enableTriggerReplicationMaster();
	  }
	}

	// ******** prepare pixel register masks ***************** 
	if(scn.getMaskStageTotalSteps()!=PixScan::STEPS_USER && shiftmask) { // if mask pattern set by user, don't touch
		if(!scn.getLoopOverDcs() && !scn.getAvoidSpecialsCols() && scn.getLoopFEI4GR(0)=="Colpr_Addr" && 
			scn.getLoopParam(0)==PixScan::FEI4_GR && scn.getLoopActive(0)){
				// make sure Colpair_Mode ist set to scan only selected DC
				if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting cpmode to 0" << endl;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
          if(*it==999) break;
					m_USBpix->SetGlobalVal(m_USBpix->FEisFEI4B()?B_COLPR_MODE:COLPR_MODE, 0, *it);
					m_USBpix->WriteGlobalSingleReg(m_USBpix->IndexToRegisterNumber(m_USBpix->FEisFEI4B()?B_COLPR_ADDR:COLPR_ADDR), *it);
					// first, set all pixel to off
					if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: resetting all masks to off"<<endl;
					for(int l = PIXEL26880; l <=PIXEL32; l++) {     //JW: this loops over the blocks of INTs, including Maltes offset
						if(shiftmask&SHIFT_ENABLE) m_USBpix->SetPixelVal(l, 0, ENABLE, *it);
						if(shiftmask&SHIFT_CAP1)   m_USBpix->SetPixelVal(l, 0, CAP1, *it);
						if(shiftmask&SHIFT_CAP0)   m_USBpix->SetPixelVal(l, 0, CAP0, *it);
						if(shiftmask&SHIFT_HITBUS) m_USBpix->SetPixelVal(l, 0xffffffff, HITBUS, *it); // NB: 0 turns connection to hitbus ON!
						if(shiftmask&SHIFT_INVHB)  m_USBpix->SetPixelVal(l, 0, HITBUS, *it);          // NB: 1 turns connection to Ileak ON!
						if(shiftmask&SHIFT_DIGINJ) m_USBpix->SetPixelVal(l, 0, DIGINJ, *it);
					}
					// writes mask to all DCs in the same way, faster
					if(shiftmask&SHIFT_CAP1)   m_USBpix->WritePixelSingleLatch(CAP1, *it);
					if(shiftmask&SHIFT_CAP0)   m_USBpix->WritePixelSingleLatch(CAP0, *it);
					if(shiftmask&SHIFT_HITBUS || shiftmask&SHIFT_INVHB) 
						m_USBpix->WritePixelSingleLatch(HITBUS, *it);
					if(shiftmask&SHIFT_ENABLE) m_USBpix->WritePixelSingleLatch(ENABLE, *it);
					if(shiftmask&SHIFT_DIGINJ) m_USBpix->WritePixelSingleLatch(DIGINJ, *it);
				}
		}
		if(maskstages==26880 && !scn.getLoopOverDcs() && !scn.getAvoidSpecialsCols() && scn.getLoopFEI4GR(0)=="Colpr_Addr" && 
			scn.getLoopParam(0)==PixScan::FEI4_GR && scn.getLoopActive(0)){
				// 26880 mask loop realised by outer DC loop
				// set 1st pixel ON in current DC
				int DC = (int)(scn.getLoopVarValues(0))[scn.scanIndex(0)];
				if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: writing mask for DC " << DC << endl;
				int index = 819-21*DC;
				if(index<prWords && index>=0){
					if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: setting pixel "<< index << " to 1 " << endl;
          for (std::vector<int>::iterator it = m_chipIds.begin();
              it != m_chipIds.end(); it++)
          {
            if(*it==999) break;
						// load actual content and write only to current DC
						//if((shiftmask&SHIFT_CAP1) || scn.getDigitalInjection()){
						if((shiftmask&SHIFT_CAP1) || (!(shiftmask&SHIFT_CAP1) && shiftmask&SHIFT_CAP0) || scn.getDigitalInjection()){
							m_USBpix->SetPixelVal(PIXEL32-index, 1, CAP1, *it);
							m_USBpix->WritePixelSingleLatchDC(CAP1,DC, *it);
						}
						if((shiftmask&SHIFT_CAP1) || (!(shiftmask&SHIFT_CAP1) && shiftmask&SHIFT_CAP0) || scn.getDigitalInjection()){
							m_USBpix->SetPixelVal(PIXEL32-index, 1, CAP0, *it);
							m_USBpix->WritePixelSingleLatchDC(CAP0,DC, *it);
						}
						if(shiftmask&SHIFT_HITBUS){
							m_USBpix->SetPixelVal(PIXEL32-index, 0xfffffffe, HITBUS, *it); // 0 turns hitbus ON!
							m_USBpix->WritePixelSingleLatchDC(HITBUS,DC, *it);
						}
						if(shiftmask&SHIFT_INVHB){ // inv. HITBUS
							m_USBpix->SetPixelVal(PIXEL32-index, 1, HITBUS, *it); // 1 turns Ileak ON!
							m_USBpix->WritePixelSingleLatchDC(HITBUS,DC, *it);
						}
						if(shiftmask&SHIFT_ENABLE){
							m_USBpix->SetPixelVal(PIXEL32-index, 1, ENABLE, *it);
							m_USBpix->WritePixelSingleLatchDC(ENABLE,DC, *it);
						}
						if(shiftmask&SHIFT_DIGINJ){
							m_USBpix->SetPixelVal(PIXEL32-index, 1, DIGINJ, *it);
							m_USBpix->WritePixelSingleLatchDC(DIGINJ,DC, *it);
						}
					}
				} else if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: pixel index "<< index << " out of range (0...839) " << endl;
		}else{
		  // prepare mask pattern
		  for (unsigned int j = 0; j < (m_nColFe*m_nRowFe); j++) {     //JW: this loops over all pixels in the FE
		    
		    if(j%maskstages==0) {
		      //if(!xtalk) 
		      pixelmask[j/32] = pixelmask[j/32] | (0x1<<(j%32));
		      //else pixelmask[j/32] = pixelmask[j/32] | (0x1<<((j%32)+1));
		      //***********
			  //only implemented for maskstage%4==0
			  if(j%32>0) {             //JW: only possible if not 32-step-mask
			    pixelmask2[j/32]= pixelmask2[j/32]| (0x1<<((j%32)+1));
			    pixelmask2[j/32]= pixelmask2[j/32]| (0x1<<((j%32)-1));
			  } else {                 //JW: first step for 32-step-mask
			pixelmask2[j/32]= pixelmask2[j/32]| 0x2; 
			if(j/32) pixelmask2[j/32-1]= pixelmask2[j/32-1]| 0x80000000;
		      }
		    } 
		  }
		  // then, load actual content and write to all DCs
      for (std::vector<int>::iterator it = m_chipIds.begin();
          it != m_chipIds.end(); it++)
      {
        if(*it==999) break;
        if(*it==999) break;
        for(int l = PIXEL26880; l <=PIXEL32; l++) { //JW: this loops over the blocks of INTs, including Maltes offset

          if(shiftmask&SHIFT_CAP1){
            if(UPC_DEBUG_GEN && l == PIXEL26880) cout<<"DEBUG USBPixCtrl: turning CAP1 ON" << endl;
            if(!xtalk) m_USBpix->SetPixelVal(l, pixelmask[PIXEL32-l], CAP1, *it);
            else       m_USBpix->SetPixelVal(l, pixelmask2[PIXEL32-l], CAP1, *it);
          } 
          if(scn.getDigitalInjection() || (!(shiftmask&SHIFT_CAP1) && shiftmask&SHIFT_CAP0)) {
            // turn cap's off for digital injectionand SEL with CAP1 off
            if(UPC_DEBUG_GEN && l == PIXEL26880) cout<<"DEBUG USBPixCtrl: turning CAP1 OFF" << endl;
            m_USBpix->SetPixelVal(l, 0, CAP1, *it);
          }
          if(shiftmask&SHIFT_CAP0) {
            if(UPC_DEBUG_GEN && l == PIXEL26880) cout<<"DEBUG USBPixCtrl: turning CAP0 ON" << endl;
            if(!xtalk) m_USBpix->SetPixelVal(l, pixelmask[PIXEL32-l], CAP0, *it);
            else       m_USBpix->SetPixelVal(l, pixelmask2[PIXEL32-l], CAP0, *it);
          }
          if(scn.getDigitalInjection() || (!(shiftmask&SHIFT_CAP0) && shiftmask&SHIFT_CAP1)) {
            if(UPC_DEBUG_GEN && l == PIXEL26880) cout<<"DEBUG USBPixCtrl: turning CAP0 OFF" << endl;
            // turn cap's off for digital injection and SEL with CAP0 off
            m_USBpix->SetPixelVal(l, 0, CAP0, *it);
          }

          if(shiftmask&SHIFT_HITBUS) // NB: 0 turns connection to hitbus ON!
            m_USBpix->SetPixelVal(l, ~(pixelmask[PIXEL32-l]), HITBUS, *it);

          if(shiftmask&SHIFT_INVHB) // NB: 1 turns connection to Ileak ON!
            m_USBpix->SetPixelVal(l, pixelmask[PIXEL32-l], HITBUS, *it);
          if(shiftmask&SHIFT_ENABLE)
            m_USBpix->SetPixelVal(l, pixelmask[PIXEL32-l], ENABLE, *it);
          if(shiftmask&SHIFT_DIGINJ)
            m_USBpix->SetPixelVal(l, pixelmask[PIXEL32-l], DIGINJ, *it);
        }

        int DC=40;
        if(!scn.getLoopOverDcs() && !scn.getAvoidSpecialsCols() && scn.getLoopFEI4GR(0)=="Colpr_Addr" && 
            scn.getLoopParam(0)==PixScan::FEI4_GR && scn.getLoopActive(0)) 
          DC = (int)(scn.getLoopVarValues(0))[scn.scanIndex(0)];
        // writes mask to all DCs in the same way, faster
        if((shiftmask&SHIFT_CAP1) || (!(shiftmask&SHIFT_CAP1) && shiftmask&SHIFT_CAP0) || scn.getDigitalInjection()) {
          if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: writing mask CAP1 for DC "<<DC<< endl;
          m_USBpix->WritePixelSingleLatchDC(CAP1, DC, *it);
        }
        if((shiftmask&SHIFT_CAP0) || (!(shiftmask&SHIFT_CAP0) && shiftmask&SHIFT_CAP1) || scn.getDigitalInjection()) {
          if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: writing mask CAP0 for DC "<<DC<< endl;
          m_USBpix->WritePixelSingleLatchDC(CAP0, DC, *it);
        }
        if(shiftmask&SHIFT_HITBUS || shiftmask&SHIFT_INVHB){
          if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: writing mask HITBUS for DC "<<DC<< endl;
          m_USBpix->WritePixelSingleLatchDC(HITBUS, DC, *it);
        }
        if(shiftmask&SHIFT_ENABLE){
          if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: writing mask ENABLE for DC "<<DC<< endl;
          m_USBpix->WritePixelSingleLatchDC(ENABLE, DC, *it);
        }
        if(shiftmask&SHIFT_DIGINJ){
          if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: writing mask DIGINJ for DC "<<DC<< endl;
          m_USBpix->WritePixelSingleLatchDC(DIGINJ, DC, *it);
	}
      }
      delete[] pixelmask;
      delete[] pixelmask2;
      for (std::vector<int>::iterator it = m_chipIds.begin();
          it != m_chipIds.end(); it++)
      {
        if(*it==999) break;
        m_USBpix->WriteGlobal(*it);
			}
		}
	}

	if(scn.getHistogramFilled(PixScan::DSP_ERRORS)){
	  // read error GR's and service records to clear them
	  for (std::vector<int>::iterator it = m_chipIds.begin();
	       it != m_chipIds.end(); it++)
	    {
	      if(*it==999) break;
	      if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: clearin GR r/o bits for FE "<<(*it)<< endl;
	      m_USBpix->ReadGlobalSingleReg(41, *it);
	      m_USBpix->ReadGlobalSingleReg(42, *it);
	    }
	  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: clearing SR counters"<< endl;
	  std::string txt;
	  std::vector<int> srvCounts;
	  getServiceRecords(txt, srvCounts);
	}
	
	// after config, switch back to data taking mode
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
		m_USBpix->WriteCommand(FE_EN_DATA_TAKE, *it);
	}
	// ******** end pixel register settings ************************

	// source scan
	if(m_sourceScanFlag) {
		// get histogram parameters
		scn.getClusterPars(m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], m_clusterPars[3], m_clusterPars[4], m_clusterPars[5], m_clusterPars[6], m_clusterPars[7]);
		m_rawDataFilename=scn.getSourceRawFile();
		m_skipSourceScanHistos=scn.getSkipSourceScanHistos();
		// fill histograms that do not need clustering
		m_fillSrcHistos = !m_skipSourceScanHistos && (scn.getHistogramFilled(PixScan::OCCUPANCY) || scn.getHistogramFilled(PixScan::TOT) ||
							      scn.getHistogramFilled(PixScan::LVL1) || scn.getHistogramFilled(PixScan::LV1ID) || scn.getHistogramFilled(PixScan::BCID));
		// fill histograms that need clustering
		m_fillClusterHistos = !m_skipSourceScanHistos && (scn.getHistogramFilled(PixScan::HITOCC) || scn.getHistogramFilled(PixScan::CLUSTER_TOT) || 
								  scn.getHistogramFilled(PixScan::CLUSTER_CHARGE) || scn.getHistogramFilled(PixScan::CLUSTER_POSITION) ||
								  scn.getHistogramFilled(PixScan::CLUSTER_SIZE) || scn.getHistogramFilled(PixScan::CLSIZE_TOT) || 
								  scn.getHistogramFilled(PixScan::CLSIZE_CHARGE) || scn.getHistogramFilled(PixScan::SEED_TOT) || 
								  scn.getHistogramFilled(PixScan::SEED_LVL1) || scn.getHistogramFilled(PixScan::NSEEDS));
		// set FPGA register
		m_USBpix->SetSramReadoutThreshold(scn.getSramReadoutAt()); // send the SRAM ro threshold to the fpga
		m_USBpix->WriteRegister(CS_TLU_TRIGGER_DATA_LENGTH, 15); // set TLU trigger data length (depends on TLU bit-file)
		m_USBpix->WriteRegister(CS_TLU_TRIGGER_DATA_DELAY, scn.getTLUTriggerDataDelay()); // set additional wait cycles (5)
		m_USBpix->SetNumberOfEvents(scn.getRepetitions()); // write #hits to FPGA register
		m_SourceScanEventQuantity = scn.getRepetitions();
		if (scn.getSrcTriggerType()==PixScan::STROBE_SCAN || scn.getSrcTriggerType()==PixScan::STROBE_EXTTRG || scn.getSrcTriggerType()==PixScan::STROBE_USBPIX_SELF_TRG
		    || scn.getSrcTriggerType()==PixScan::STROBE_FE_SELF_TRG) { // need internal strobes for noise occupancy
			m_USBpix->WriteStrbQuantity(0); // endless sending of strobes
			m_USBpix->WriteStrbStart();
		}
		clearSourceScanHistos();
		int intEnRJ45 = (m_enableRJ45 ? 1 : 0);
		// check if trigger mode needs TLU - RJ45 must be enabled then, otherwise this will fail unavoidably
		if(scn.getSrcTriggerType()==PixScan::TLU_SIMPLE || scn.getSrcTriggerType()==PixScan::TLU_DATA_HANDSHAKE) intEnRJ45 = 1;
		m_USBpix->WriteRegister(CS_ENABLE_RJ45, intEnRJ45);
		if(UPC_DEBUG_GEN) cout << "DEBUG USBPixCtrl: RJ45 " << ((intEnRJ45>0)?"ON":"OFF") << endl;

		// set counter mode
		if(UPC_DEBUG_GEN) cout<<"DEBUG: source measurement count mode: " << scn.getSrcCountType() <<endl;
		m_srcSecMax = 0; 
		int cntMode;
		switch(scn.getSrcCountType()){
	default:
	case PixScan::COUNT_TRIGGER:
		cntMode = FPGA_COUNT_LV1;
		break;
	case PixScan::COUNT_DH:
		cntMode = FPGA_COUNT_DH;
		break;
	case PixScan::COUNT_DR:
		cntMode = FPGA_COUNT_DR;
		break;
	case PixScan::COUNT_SECS:
		cntMode = FPGA_COUNT_LV1;
		m_USBpix->SetNumberOfEvents(0);// endless running, will abort from internal time control
		m_SourceScanEventQuantity = 0;
		m_srcSecMax = scn.getRepetitions();
		break;
		}
		m_USBpix->SetMeasurementMode(cntMode);
	}
}

void USBPixController::startScan(PixScan* scn) { // Start a scan
// 
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: entering startScan()"<<endl;

	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanInit="<<(m_upcScanInit?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanBusy="<<(m_upcScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcStartScanHasFinished="<<(m_upcStartScanHasFinished?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanCancelled="<<(m_upcScanCancelled?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanBusy="<<(m_scanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanDone="<<(m_scanDone?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanBusy="<<(m_sourceScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanDone="<<(m_sourceScanDone?"true":"false")<<endl;

	if(m_upcScanBusy || m_scanBusy || m_sourceScanBusy) throw USBPixControllerExc(USBPixControllerExc::SCAN_RUNNING, PixControllerExc::ERROR, getModGroup().getRodName());

	// set scan flags
	m_upcScanInit = true;
	m_upcScanBusy = true;
	m_upcScanCancelled = false;
	m_sourceScanFlag = scn->getSourceScanFlag();
        m_testBeamFlag = scn->getTestBeamFlag();
	if (!m_sourceScanFlag)
		m_scanBusy = true;
	else
		m_sourceScanBusy = true;

	// write scan configuration
	writeScanConfig(*scn);
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: finished writeScanConfig()"<<endl;

	if(!m_sourceScanFlag) { // normal scan
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: starting normal scan"<<endl;

		// set histogram mode in FPGA
		// clear histograms *here* and *before* calling USBpix::StartScan()
		if (scn->getHistogramFilled(PixScan::TOT_MEAN) || scn->getHistogramFilled(PixScan::TOT_SIGMA) || scn->getHistogramFilled(PixScan::TOT)|| scn->getHistogramFilled(PixScan::TOT0) || scn->getHistogramFilled(PixScan::TOT1) || scn->getHistogramFilled(PixScan::TOT2) || scn->getHistogramFilled(PixScan::TOT3) || scn->getHistogramFilled(PixScan::TOT4) || scn->getHistogramFilled(PixScan::TOT5) || scn->getHistogramFilled(PixScan::TOT6) || scn->getHistogramFilled(PixScan::TOT7) || scn->getHistogramFilled(PixScan::TOT8) || scn->getHistogramFilled(PixScan::TOT9) || scn->getHistogramFilled(PixScan::TOT10) || scn->getHistogramFilled(PixScan::TOT11) || scn->getHistogramFilled(PixScan::TOT12) || scn->getHistogramFilled(PixScan::TOT13) || scn->getHistogramFilled(PixScan::TOT14) || scn->getHistogramFilled(PixScan::TOT15)) {
			if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: Filling TOT histo"<<endl;
			m_USBpix->SetTOTMode();
      for (std::vector<int>::iterator it = m_chipIds.begin();
          it != m_chipIds.end(); it++)
      {
        if(*it==999) break;
				m_USBpix->ClearTOTHisto(*it);
      }
		} else if (scn->getHistogramFilled(PixScan::OCCUPANCY)) {
			m_USBpix->SetCalibrationMode();
			if(UPC_DEBUG_GEN) cout << "DEBUG Malte: Set Hit histogramming mode.\n";
      for (std::vector<int>::iterator it = m_chipIds.begin();
          it != m_chipIds.end(); it++)
      {
        if(*it==999) break;
				m_USBpix->ClearConfHisto(*it);
      }
		}

		// clear SRAM *here* and *before* calling USBpix::StartScan()
    for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
    {
      if(*it==999) break;
      m_USBpix->ClearSRAM(*it);
    }
    // to avoid race conditions:
		// reset scan status bits *before* calling the method USBpix::StartScan() and *before* setting m_upcScanInit to false
		m_USBpix->ResetScanStatus();

		if (scn->getMaskStageMode() == PixScan::XTALK) { //JW: run first step of scan here
		  if(UPC_DEBUG_GEN) cout << "DEBUG: scn->getMaskStageMode() == PixScan::XTALK" << endl;
		  //do the first scan step - i.e. 6th arg. is 1, not m_scanConfigArray[6]
		  if(UPC_DEBUG_GEN) cout << "DEBUG: Xtalk: m_USBPix->StartScan()" << endl;
		  m_USBpix->StartScan(m_scanConfigArray[0], m_scanConfigArray[1], m_scanConfigArray[2],
				      m_scanConfigArray[3],  m_scanConfigArray[4],  m_scanConfigArray[5],
				      1,  m_scanConfigArray[7],(bool)m_scanConfigArray[9], 
				      (bool)m_scanConfigArray[10], (bool)m_scanConfigArray[11]);
		  
		  if(UPC_DEBUG_GEN) cout << "DEBUG: Xtalk: finished m_USBPix->StartScan()" << endl;
		  //decrease number of masksteps to do by one
		  m_scanConfigArray[6]--;
		  //go ahead and run from the second maskstep on, if one is left
		  if(m_scanConfigArray[6]==0){
		    m_upcScanInit = false;
		    m_upcStartScanHasFinished = true;
		    return;
		  }
		  for (std::vector<int>::iterator it = m_chipIds.begin();
		       it != m_chipIds.end(); it++) {
		    if(*it==999) break;
		    m_USBpix->ClearSRAM(*it);
		  }
		  m_USBpix->ResetScanStatus();
		  // shift CAP masks by filling 1, ENABLE by filling 0
		  shiftPixMask(PixScan::XTALK, 0, 1);
		}

		if(UPC_DEBUG_GEN){
			cout<<"DEBUG USBPixCtrl: scan parameters: ";
			for(int kk=0;kk<NSCANCFG; kk++) cout << kk << ": " << m_scanConfigArray[kk] << ", ";
			cout << endl;
		}

		// TODO: there is a better way, need FPGA FSM that is capable to handle this mode
		if(scn->getSrcTriggerType()!=PixScan::STROBE_SCAN){
			// start measuerement, otherwise ext. trigger is ignored
			if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->StartMeasurement()"<<endl;
			m_USBpix->StartMeasurement();
		}

		// USBpix::StartScan() is not a thread, so setting m_upcScanInit to false enables readout of scan status
		// this has to happen *before* calling the method USBpix::StartScan()
		// now it is safe to call the method USBpix::GetScanStatus() because it will only readout member variables, *no* microcontroller access is needed
		m_upcScanInit = false;

		// start scan
		// this is *not* a thread so flow will stuck here until scan has finished
		if(UPC_DEBUG_GEN){ 
			cout<<"DEBUG USBPixCtrl: calling USBpix::StartScan"<<endl;
		}
		bool scanret = m_USBpix->StartScan(m_scanConfigArray[0], m_scanConfigArray[1], m_scanConfigArray[2],
			m_scanConfigArray[3],  m_scanConfigArray[4],  m_scanConfigArray[5],
			m_scanConfigArray[6],  m_scanConfigArray[7],
			(bool)m_scanConfigArray[9], // Malte: bool temporary added to choose scan mode all DCs or respect COLPR_MODE + ADDR...
			(bool)m_scanConfigArray[10], // JGK: bool temporary added to choose scan mode avoiding simulataneous injection into col's 0, 77, 78, 79
			(bool)m_scanConfigArray[11]); // JGK: bool temporary added to choose scan mode for ToT measurements


    int roid = 0;
    for (std::vector<ReadoutStatusRegister *>::iterator rostatus 
        = m_USBpix->readoutStatusRegisters.begin();
        rostatus != m_USBpix->readoutStatusRegisters.end();
        rostatus++)
    {
      if (m_readoutChannelInput[roid] == 0)
        continue;

      (*rostatus)->update();

      std::stringstream init;
      init << "Controller ";
      init <<  getCtrlName();
      init << ": Readout channel ";
      init << ((*rostatus)->GetChannelId());

      if ((*rostatus)->get_ddrd_resync())
      {
        std::stringstream s;
        s << init.str();
        s << " detected at least one 8b10b resynchronization event.";
        s << endl;
        m_errBuff += s.str();
      }
      
      if ((*rostatus)->get_ddrd_exc_8b10b_code())
      {
        std::stringstream s;
        s << init.str();
        s << " detected at least one 8b10b code error event.";
        s << endl;
        m_errBuff += s.str();
      }
      
      if ((*rostatus)->get_ddrd_exc_8b10b_code())
      {
        std::stringstream s;
        s << init.str();
        s << " detected at least one 8b10b disparity error event.";
        s << endl;
        m_errBuff += s.str();
      }
      
      if ((*rostatus)->get_ddrd_exc_comma_in_frame())
      {
        std::stringstream s;
        s << init.str();
        s << " detected a comma inside a frame.";
        s << endl;
        m_errBuff += s.str();
      }
      roid++;
    }

    {
      if (m_USBpix->memoryArbiterStatusRegister
          ->get_fetched_cmd_fifo_overflow())
      {
        m_errBuff += "Controller " + getCtrlName() + ": Memory Arbiter Error:"
         + " fetched_cmd_fifo overflow. This is a bug, please report this "
         + " error.\n";
      }
      
      if (m_USBpix->memoryArbiterStatusRegister
          ->get_merged_fifo_full())
      {
        m_errBuff += "Controller " + getCtrlName() + ": Memory Arbiter Error:"
         + " fetched_fifo full. This is a bug, please report this error.\n";
      }
    }

		if(scn->getSrcTriggerType()!=PixScan::STROBE_SCAN){
			// stop measuerement if it was started earlier on
			if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->StopMeasurement()"<<endl;
			m_USBpix->StopMeasurement();
		}

		if(!scanret){
		  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: USBpix::StartScan returns " << (scanret?"true":"false") << endl;
		  if(scanret) m_errBuff += "USBpix::StartScan finished with an error\n";
		}

	} else { // source scan

		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: starting source scan"<<endl;

		// store raw data in SRAM
		//m_USBpix->SetRunMode();
		// write new raw file
		m_newRawDataFile = true;

		// clear SRAM
		for (std::vector<int>::iterator it = m_chipIds.begin();
		     it != m_chipIds.end(); it++)
		  {
		    if(*it==999) break;
		    //m_USBpix->ClearSRAM(*it); // covered by setRunMode
		    m_USBpix->ResetClusterCounters(*it); //reset the clusterizer counters for the actual scan
		  }

		// start source scan here
		int intEnRJ45 = m_USBpix->ReadRegister(CS_ENABLE_RJ45);
		m_USBpix->WriteRegister(CS_ENABLE_RJ45, 0);
		setRunMode();
		setFERunMode();
 		m_USBpix->StartMeasurement(); // 
		m_USBpix->StartReadout();
		m_USBpix->WriteRegister(CS_ENABLE_RJ45, intEnRJ45);
		m_srcSecStart = clock()/CLOCKS_PER_SEC;  
		m_sramReadoutReady = false;
		m_sramFull = false;
		m_tluVeto = false;
		m_measurementPause = false;
		m_measurementRunning = true;
		m_sramFillLevel = 0;
		m_collectedTriggers = 0;
		m_collectedHits = 0;
		m_triggerRate = 0;
		m_eventRate = 0;

	}

	// setting m_upcScanInit to false to make readout of scan status information possible.
	// now all scan status bits are set properly, race conditions are avoided
	// normal scan needs a special treatment since not implemented as a thread
	m_upcScanInit = false;

	// m_scanDone has to be asserted here, not in nTrigger()
	// this is due to the bad implementation of strobe scan and because of USBpix::StartScan() is not a thread
	m_upcStartScanHasFinished = true;
}

void USBPixController::stopScan() {

	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: entering stopScan()"<<endl;

	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanInit="<<(m_upcScanInit?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanBusy="<<(m_upcScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcStartScanHasFinished="<<(m_upcStartScanHasFinished?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanCancelled="<<(m_upcScanCancelled?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanBusy="<<(m_scanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanDone="<<(m_scanDone?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanBusy="<<(m_sourceScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanDone="<<(m_sourceScanDone?"true":"false")<<endl;


	// repeatedly called in scan do-while loop in STPixModuleGroup
  m_USBpix->StopReadout();
	if(m_upcScanBusy && m_scanBusy) { // normal scan
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->SetScanCancelled()"<<endl;
		m_USBpix->SetScanCancelled();
		// set cancel flag, so we know that scan was cancelled/stopped
		m_upcScanCancelled = true;
	} else if(m_upcScanBusy && m_sourceScanBusy) { // source scan
		if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->StopMeasurement()"<<endl;
    m_USBpix->StopReadout();
		m_USBpix->StopMeasurement();
		m_USBpix->WriteStrbStop(); // in case strobe was started; shouldn't harm otherwise
		m_USBpix->StopCcpdInjections();	// in case strobe was started; shouldn't harm otherwise
		m_USBpix->SetNumberOfEvents(0); // Make sure number of events to count is 0 after scan. Might be troublesome in strobe scan with external/self triggering otherwise.
		m_SourceScanEventQuantity = 0;
		// set cancel flag, so we know that scan was cancelled/stopped
		m_upcScanCancelled = true;
	}
}

void USBPixController::pauseRun() {
  if(m_sourceScanFlag) {
    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: pausing by calling m_USBpix->StopMeasurement()"<<endl;
    m_USBpix->StopMeasurement();
  }
}

void USBPixController::resumeRun() {
  if(m_sourceScanFlag) {
    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: resuming by calling m_USBpix->StartMeasurement()"<<endl;
    m_USBpix->StartMeasurement();
  }
}

// ******* Histogram treatment on the board ******
bool USBPixController::fitHistos() {                                                 //! Fit collected histograms
	return false;
}

// TO DO: get rid of this function - controller should know themselves if fitting results exist
void USBPixController::getFitResults(HistoType /*type*/, unsigned int /*mod*/, unsigned int /*slv*/, 
				     std::vector< Histo * > &/*thr*/, std::vector< Histo * > &/*noise*/, std::vector< Histo * > &/*chi2*/) { //! Read a Fit from Dsp
}

bool USBPixController::getErrorHistos(unsigned int dsp, Histo* &his) {               //! Read collected error arrays 
  if(dsp!=0) return false;

	std::ostringstream nam, tit;
	nam << "FE_errors";
	tit << "FE Errors";

	// bin order: 0..31: service records; 32: CMDErrReg SEU, 33: CMDErrReg BitFlip, 
	// 34: CMDErrReg Trg BitFlip, 35: CMDErrReg hdr BitFlip, 36: EOCHLSkipped
	const int nSrvRec=32, nErrGR =5, errbins=nSrvRec+nErrGR;
	int histsize = 0; 
	for (std::vector<int>::iterator it = m_chipIds.begin();
	     it != m_chipIds.end(); it++){
	  if(*it==999) break;
	  histsize += errbins;
	}
	his = new Histo(nam.str(), tit.str(), histsize, -0.5, histsize-0.5);

	for (int ib=0; ib< (int) m_chipIds.size();ib++){
	  if(m_chipIds.at(ib)==999) break;
	  his->set(nSrvRec+0+ib*errbins,  readGlobal(42, ib)&0x3f);
	  his->set(nSrvRec+1+ib*errbins, (readGlobal(42, ib)&0x7c0)>>6);
	  his->set(nSrvRec+2+ib*errbins, (readGlobal(42, ib)&0x3800)>>11);
	  his->set(nSrvRec+3+ib*errbins, (readGlobal(42, ib)&0xc000)>>14);
	  his->set(nSrvRec+4+ib*errbins, (readGlobal(41, ib)&0xff00)>>8);
	}

	std::string txt;
	std::vector<int> srvCounts;
	getServiceRecords(txt, srvCounts);
	for(unsigned int i=0;i<srvCounts.size(); i++){
	  int ih=i+(i/nSrvRec)*nErrGR;// leave room for entries from GR error counters
	  if(ih<histsize) his->set(ih, srvCounts[i]);
	}
	return true;
}

void USBPixController::writeRawDataFile(bool close_file, int chipIndex) {
	// to do: currently called if any of the boards has a full SRAM
	bool retval = m_USBpix->WriteFileFromRawData(m_rawDataFilename, chipIndex, m_newRawDataFile, close_file);
	//create a new raw file only once per scan
	m_newRawDataFile = false; 
  m_createdRawDataFile = true;
	if (UPC_DEBUG_GEN) cout << "DEBUG USBPixCtrl: WriteFileFromRawData return " << (retval?"true":"false") << endl;
}

void USBPixController::ClusterRawData(int pChipIndex)	//DLP
{
	if (UPC_DEBUG_GEN)
		std::cout<<"DEBUG USBPixCtrl::ClusterRawData(): chip adress "<<pChipIndex<<", col range "<<m_clusterPars[0]<<", row range "<<m_clusterPars[1]
			 <<", BCID range "<<m_clusterPars[2]<<", min cluster hits "<<m_clusterPars[3]<<", max cluster hits "<<m_clusterPars[4]
			 <<", max cluster hit tot "<<m_clusterPars[5]<<", max event incomplete "<<m_clusterPars[6]<<", max event errors "<<m_clusterPars[7]<<"\n";
	if(!m_USBpix->ClusterRawData(pChipIndex, m_clusterPars[0], m_clusterPars[1], m_clusterPars[2], m_clusterPars[3], m_clusterPars[4], m_clusterPars[5], m_clusterPars[6], m_clusterPars[7])){
		if (UPC_DEBUG_GEN) std::cout<<"USBPixController::ClusterRawData: Bad data, clustering aborted"<<std::endl;
		m_errBuff += "USBPixController::ClusterRawData: Bad data, clustering aborted\n";
	}
}

void USBPixController::getHisto(HistoType type, unsigned int mod, unsigned int , std::vector< std::vector<Histo*> > &his) { // Read a histogram

  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: entered getHisto() for mod "<<mod<<endl;
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: HistoType="<< ((int)type) <<endl;

	bool TOTmode=false;
	bool MadeHitHisto=false;
	bool MadeTOTHisto=false;

	std::string scanname, scanname1;
	switch (type) {
  case OCCUPANCY :
	  TOTmode=false;
	  scanname   = "Occup_";
	  scanname1  = "Occupancy";
	  break;
  case TOT_MEAN :
	  TOTmode=true;
	  scanname   = "ToTMean_";
	  scanname1  = "ToTMean";
	  break;
  case TOT_SIGMA :
	  TOTmode=true;
	  scanname   = "ToTSigma_";
	  scanname1  = "ToTSigma";
	  break;
  case LVL1 :
	  scanname   = "LVL1_";
	  scanname1  = "LVL1";
	  break;
  case LV1ID :
	  scanname   = "LV1ID_";
	  scanname1  = "LV1ID";
	  break;
  case BCID :
	  scanname   = "BCID_";
	  scanname1  = "BCID";
	  break;
  case TOT:
	  TOTmode=true;
	  scanname   = "ToT_";
	  scanname1  = "ToT";
	  break;
  case TOT0:
	  TOTmode=true;
	  scanname   = "ToT0_";
	  scanname1  = "ToT0";
	  break;
  case TOT1:
	  TOTmode=true;
	  scanname   = "ToT1_";
	  scanname1  = "ToT1";
	  break;
  case TOT2:
	  TOTmode=true;
	  scanname   = "ToT2_";
	  scanname1  = "ToT2";
	  break;
  case TOT3:
	  TOTmode=true;
	  scanname   = "ToT3_";
	  scanname1  = "ToT3";
	  break;
  case TOT4:
	  TOTmode=true;
	  scanname   = "ToT4_";
	  scanname1  = "ToT4";
	  break;
  case TOT5:
	  TOTmode=true;
	  scanname   = "ToT5_";
	  scanname1  = "ToT5";
	  break;
  case TOT6:
	  TOTmode=true;
	  scanname   = "ToT6_";
	  scanname1  = "ToT6";
	  break;
  case TOT7:
	  TOTmode=true;
	  scanname   = "ToT7_";
	  scanname1  = "ToT7";
	  break;
  case TOT8:
	  TOTmode=true;
	  scanname   = "ToT8_";
	  scanname1  = "ToT8";
	  break;
  case TOT9:
	  TOTmode=true;
	  scanname   = "ToT9_";
	  scanname1  = "ToT9";
	  break;
  case TOT10:
	  TOTmode=true;
	  scanname   = "ToT10_";
	  scanname1  = "ToT10";
	  break;
  case TOT11:
	  TOTmode=true;
	  scanname   = "ToT11_";
	  scanname1  = "ToT11";
	  break;
  case TOT12:
	  TOTmode=true;
	  scanname   = "ToT12_";
	  scanname1  = "ToT12";
	  break;
  case TOT13:
	  TOTmode=true;
	  scanname   = "ToT13_";
	  scanname1  = "ToT13";
	  break;
  case TOT14:
	  TOTmode=true;
	  scanname   = "ToT14_";
	  scanname1  = "ToT14";
	  break;
  case TOT15:
	  TOTmode=true;
	  scanname   = "ToT15_";
	  scanname1  = "ToT15";
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
  case CLUSTER_CHARGE:
  	  scanname   = "ClCharge_";
  	  scanname1  = "Cluster Charge";
  	  break;
  case CLUSTER_POSITION:
	  scanname   = "ClPosition_";
	  scanname1  = "Cluster Position";
	  break;
  case CLUSTER_SIZE:
	  scanname   = "ClSize_";
	  scanname1  = "Cluster size";
	  break;
  case CLSIZE_TOT:
	  scanname   = "ClSizToT_";
	  scanname1  = "Cluster size vs ToT";
	  break;
  case CLSIZE_CHARGE:
  	  scanname   = "ClSizCharge_";
  	  scanname1  = "Cluster size vs Charge";
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
	//make sure, the vector we want to write to exists
	std::vector< Histo * > vh;
	for(int im=0;im<32;im++) his.push_back(vh); // will never have more than 32 modules

	//	if(mod!=0) mod = 0;             //need only treat one module at the moment
	his[mod].clear();               //clear vector for writing

	// read histo size from PixModule
	PixModule *pm = m_modGroup.module(mod);
	if(pm==0) return; // nothing to do
	unsigned int ncolm = pm->nColsMod();
	unsigned int nrowm = pm->nRowsMod();

	// source scan needs special types of histograms
	if(m_sourceScanFlag) {
		if (type == OCCUPANCY || type == HITOCC) MadeHitHisto = true;
		if (TOTmode) MadeTOTHisto = true;
	}

	//JW: the scan configuration should be the one from the last scan, 
	//JW: i.e. the scan, whose data we're downloading
	int bins = m_scanConfigArray[8];

	if(UPC_DEBUG_GEN) cout << "DEBUG USBPixCtrl: bins=" << bins << ", MadeHitHisto=" << (MadeHitHisto?"true":"false") << 
			    ", MadeTOTHisto=" << (MadeTOTHisto?"true":"false") << endl;

	// *** now loop over the innermost scanloop, e.g. VCal ***
	for(int i=0; i<bins; i++) {
		if(UPC_DEBUG_GEN) cout << "INFO USBPixCtrl: downloading scan histos for step " << i << endl;
		std::ostringstream nam, tit;
		nam << scanname  << mod    << "_" << i;
		tit << scanname1 <<" mod " << mod << " bin " << i;
		Histo *h;
		//JW: ConfigRegister only supports single-chip histograms
		//JW: stick to module format for the moment
		if (type == OCCUPANCY || type == HITOCC) {
			h = new Histo(nam.str(), tit.str(), (MadeHitHisto?8:1), ncolm, -0.5, -0.5+(double)ncolm, nrowm, -0.5, -0.5+(double)nrowm);
		}else if(type == TOTAVERAGE) {
			h = new Histo(nam.str(), tit.str(), nrowm*ncolm, -0.5, -0.5+(double)(nrowm*ncolm));
		}else if(type == LVL1 || type == SEED_LVL1 || type == NSEEDS) {
			h = new Histo(nam.str(), tit.str(), 16, -0.5, 15.5);
		}else if(type == LV1ID) {
			h = new Histo(nam.str(), tit.str(), 4096, -0.5, 4095.5);
		}else if(type == BCID) {
			h = new Histo(nam.str(), tit.str(), 8192, -0.5, 8191.5);
		}else if(type == TOT || type == SEED_TOT) {
			h = new Histo(nam.str(), tit.str(), 16, -0.5, 15.5);
		}else if(type == CLUSTER_SIZE) {
			h = new Histo(nam.str(), tit.str(), __MAXCLUSTERHITSBINS, -0.5, __MAXCLUSTERHITSBINS-0.5);
		}else if(type == CLUSTER_TOT) {
			h = new Histo(nam.str(), tit.str(), __MAXTOTBINS, -0.5, __MAXTOTBINS-0.5);
		}else if(type == CLUSTER_CHARGE) {
			h = new Histo(nam.str(), tit.str(), __MAXCHARGEBINS, -0.5, __MAXCHARGEBINS-0.5);
		}else if(type == CLUSTER_POSITION) {
			h = new Histo(nam.str(), tit.str(), 8, __MAXPOSXBINS, -0.5, __MAXPOSXBINS-0.5, __MAXPOSYBINS, -0.5, __MAXPOSYBINS-0.5);
		}else if(type == CLSIZE_TOT) {
			h = new Histo(nam.str(), tit.str(), 8, __MAXTOTBINS, -0.5, __MAXTOTBINS-0.5, __MAXCLUSTERHITSBINS, -0.5, __MAXCLUSTERHITSBINS-0.5);
		}else if(type == CLSIZE_CHARGE) {
			h = new Histo(nam.str(), tit.str(), 8, __MAXCHARGEBINS, -0.5, __MAXCHARGEBINS-0.5, __MAXCLUSTERHITSBINS, -0.5, __MAXCLUSTERHITSBINS-0.5);
		}else {
		  h = new Histo(nam.str(), tit.str(), 8, ncolm, -0.5, -0.5+(double)ncolm, nrowm, -0.5, -0.5+(double)nrowm);
		}

		int nr_hits = 0;
		int TOTsumsqr=0, TOTsum=0, hitsum=0, ToThis[16];
		double TOTsqravg=0, TOTavg=0, TOTsigma=0;
		for(int tot=0; tot<16; tot++) ToThis[tot]=0;

		// only use module that matches adapter board channel if in burn-in mode
		if(!moduleActive((int)mod)){
		  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: getHisto() ignores module ID "<< mod << " since not selected on adapter board"<<endl;
		}else{
		    
		  if(type == OCCUPANCY || type==HITOCC || TOTmode) {
			if(UPC_DEBUG_GEN) cout<<"Calling ConfigRegister functions with TOTmode="<<(TOTmode?"true":"false")<<endl;
			//for(int ib=0;ib<NBOARDS_MAX;ib++){
      
			int nFe = 0;
			for (std::vector<int>::iterator it = m_chipIds.begin();
			     it != m_chipIds.end(); it++, nFe++)
			  {
			    if(*it == 999) break;
			    for(unsigned int col=0; col<pm->nColsFe(); col++) {
			      for(unsigned int row=0; row<pm->nRowsFe(); row++) {
				unsigned int colm = pm->iColMod(nFe, col);
				unsigned int rowm = pm->iRowMod(nFe, row);
            
				if(!TOTmode) {
				  if(MadeHitHisto){
				    if(type == OCCUPANCY)
				      m_USBpix->GetConfHisto(col, row, 0, nr_hits, *it); // source scan
				  }else 
				    m_USBpix->GetConfHisto(col, row, i, nr_hits, *it); // normal scan, configuration parameter i starts from 0
				  h->set(colm, rowm, nr_hits);
				} else {
				  TOTsumsqr=0;
				  TOTsum=0;
				  hitsum=0;
				  TOTsqravg=0;
				  TOTavg=0;
				  for(int tot=0; tot<16; tot++) {
            m_USBpix->GetTOTHisto(col, row, tot, nr_hits, *it);
				    TOTsumsqr+=(nr_hits*tot*tot);
				    TOTsum+=(nr_hits*tot);
				    hitsum+=nr_hits;
				    ToThis[tot]+=nr_hits;
				    if((type - TOT0) == tot )
				      h->set(colm, rowm, nr_hits);
				  }
				  if(hitsum > 1) {
				    TOTsqravg = (double)TOTsumsqr/(double)hitsum;
				    TOTavg = (double)TOTsum/(double)hitsum;
				    TOTsigma = ((double)(TOTsqravg-TOTavg*TOTavg)/(double)(hitsum-1));
				    if(TOTsigma>=0.) TOTsigma = sqrt(TOTsigma);
				    else             TOTsigma = 0.;
				  } else {
				    TOTsqravg = 0;
				    TOTavg = 0;
				    TOTsigma = 0;
				  }
				  if(type == TOT_MEAN) {
				    h->set(colm, rowm, (double)TOTavg);
				  }
				  else if(type == TOT_SIGMA) {
				    h->set(colm, rowm, (double)TOTsigma);
				  }
				  else if(type == OCCUPANCY) {
				    h->set(colm, rowm, (double)hitsum);
				  }
				}
			      }
			    }
			  }
		  }
		if(type == TOT) {
			for(int i=0;i<16;i++) h->set(i, (double)ToThis[i]);
		}

		int value, cvalue;
		if(type==LVL1 || type==SEED_LVL1 || type==NSEEDS) {
			for(int i=0;i<16;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it==999) break;
					switch(type){
					  case LVL1:
						  m_USBpix->GetHitLV1HistoFromRawData(i, cvalue, *it);
						  break;
					  default:
						  value = 0;
					}
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		if(type==CLSIZE_TOT) {
			for(int i=0;i<__MAXTOTBINS;i++){
				for(int j=0;j<__MAXCLUSTERHITSBINS;j++){
					value = 0;
          for (std::vector<int>::iterator it = m_chipIds.begin();
              it != m_chipIds.end(); it++)
          {
						if(*it==999) break;
						m_USBpix->GetClusterTOTHistoFromRawData(i, j, cvalue, *it);
						value += cvalue;
					}
					h->set(i, j, (double)value);
				}
			}
		}

		if(type==CLSIZE_CHARGE) {
			for(int i=0;i<__MAXCHARGEBINS;i++){
				for(int j=0;j<__MAXCLUSTERHITSBINS;j++){
					value = 0;
          for (std::vector<int>::iterator it = m_chipIds.begin();
              it != m_chipIds.end(); it++)
          {
						if(*it==999) break;
						m_USBpix->GetClusterChargeHistoFromRawData(i, j, cvalue, *it);
						value += cvalue;
					}
					h->set(i, j, (double)value);
				}
			}
		}

		if(type==SEED_TOT) {
			for(int i=0;i<16;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it==999) break;
					if(type==CLUSTER_TOT)
						m_USBpix->GetClusterTOTHistoFromRawData(i, 0, cvalue, *it); // j = 0: every cluster size
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		if(type==CLUSTER_TOT){
			for(int i=0;i<__MAXTOTBINS;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it==999) break;
					m_USBpix->GetClusterTOTHistoFromRawData(i, 0, cvalue, *it); // j = 0: every cluster size
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		if(type==CLUSTER_SIZE){
			for(int i=0;i<__MAXCLUSTERHITSBINS;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it == 999) break;
					m_USBpix->GetClusterSizeHistoFromRawData(i, cvalue, *it);
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		if(type==CLUSTER_CHARGE){
			for(int i=0;i<__MAXCHARGEBINS;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it) break;
					m_USBpix->GetClusterChargeHistoFromRawData(i, 0, cvalue, *it); // j = 0: every cluster size
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		if(type==CLUSTER_POSITION){
			for(int i=0;i<__MAXPOSXBINS;++i){
				for(int j=0;j<__MAXPOSYBINS;++j){
					value = 0;
          for (std::vector<int>::iterator it = m_chipIds.begin();
              it != m_chipIds.end(); it++)
          {
						if(*it==999) break;
						m_USBpix->GetClusterPositionHistoFromRawData(i, j, cvalue, *it);
						value += cvalue;
					}
					h->set(i, j, (double)value);
				}
			}
		}

		if(type==LV1ID) {
			for(int i=0;i<4096;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it==999) break;
					m_USBpix->GetLV1IDHistoFromRawData(i, cvalue, *it);
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		if(type==BCID) {
			for(int i=0;i<8192;i++){
				value = 0;
        for (std::vector<int>::iterator it = m_chipIds.begin();
            it != m_chipIds.end(); it++)
        {
					if(*it==999) break;
					m_USBpix->GetBCIDHistoFromRawData(i, cvalue, *it); 
					value += cvalue;
				}
				h->set(i, (double)value);
			}
		}

		his[mod].push_back(h);
		//JW: DO NOT DELETE h HERE! It is passed to the vector his not as pointer but as itself...
		if(UPC_DEBUG_GEN) cout<<"DEBUG  USBPixCtrl: histogram downloaded"<<endl;
	   }
	}
}

// *********** Data Taking Control ***********
void USBPixController::writeRunConfig(PixRunConfig &/*cfg*/) {      //! Get the run configuration parameters from PixModuleGroup
}

void USBPixController::startRun(int /*ntrig*/) {           //! Start a run
}

void USBPixController::stopRun() {                     //! Terminates a run
}

int USBPixController::runStatus() {                   //! Check the status of the run

	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: entering runStatus()"<<endl;

	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanInit="<<(m_upcScanInit?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanBusy="<<(m_upcScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcStartScanHasFinished="<<(m_upcStartScanHasFinished?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanCancelled="<<(m_upcScanCancelled?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanBusy="<<(m_scanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanDone="<<(m_scanDone?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanBusy="<<(m_sourceScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanDone="<<(m_sourceScanDone?"true":"false")<<endl;

	if(m_upcScanBusy && m_scanBusy) { // normal scan
		if(m_scanDone) {
			m_upcScanBusy = false;
			m_scanBusy = false;
			m_scanDone = false;
			return 2;
		} else {
			return 1;
		}
	} else if(m_upcScanBusy && m_sourceScanBusy) { // source scan
		if(m_sourceScanDone) {
			m_upcScanBusy = false;
			m_sourceScanBusy = false;
			m_sourceScanDone = false;
			return 0;
		} else {
			return 1;
		}
	}

	// JGK: default return must ALWAYS be 0, otherwise scans that didn't call USBPix::StartScan get stuck
	return 0;
}

void USBPixController::clearSourceScanHistos() {

	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: entered clearSourceScanHistos()"<<endl;

  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
		if(*it==999) break;
    m_USBpix->ClearConfHisto(*it);
		m_USBpix->ClearTOTHisto(*it);
		m_USBpix->ClearHitLV1HistoFromRawData(*it);
		m_USBpix->ClearLV1IDHistoFromRawData(*it);
		m_USBpix->ClearBCIDHistoFromRawData(*it);
	}
}

int USBPixController::nTrigger() {                 //! Returns the number of trigger processed so far
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: entering nTrigger()"<<endl;

	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanInit="<<(m_upcScanInit?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanBusy="<<(m_upcScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcStartScanHasFinished="<<(m_upcStartScanHasFinished?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_upcScanCancelled="<<(m_upcScanCancelled?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanBusy="<<(m_scanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_scanDone="<<(m_scanDone?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanBusy="<<(m_sourceScanBusy?"true":"false")<<endl;
	if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_sourceScanDone="<<(m_sourceScanDone?"true":"false")<<endl;

	int nSteps=0;
	int nMasks=0;

	if(m_upcScanBusy) {

		if(m_scanBusy && !m_upcScanInit) { // normal scan

			m_USBpix->GetScanStatus(m_scanReady, m_scanCancelled, m_scanError, m_scanStep);
			if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: m_USBpix->GetScanStatus()"<<endl;
			if(UPC_DEBUG_FLAGS) cout<<"DEBUG USBPixCtrl: scanReady="<<(m_scanBusy?"true":"false")<<", scanCancelled="<<(m_scanCancelled?"true":"false")<<", scanError="<<(m_scanError?"true":"false")<<", scanStep="<<m_scanStep<<endl;

			// m_scanDone has to asserted here, not in startScan()
			// m_upcStartScanHasFinished is used to be sure that normal scan has finished properly
			if (m_scanReady && m_upcStartScanHasFinished)
				m_scanDone = true;

			nSteps = m_scanStep;
			if(nMasks>0) nMasks = (nMasks-1) << 12; // follow ROD conventions
			return (nSteps+nMasks);

		} else if(m_scanBusy && m_upcScanInit) { // normal scan during initialization
			m_scanReady = false;
			m_scanCancelled = false;
			m_scanError = false;
			m_scanStep = 0;
			return 0;
		} else if(m_sourceScanBusy && !m_upcScanInit) { // source scan
  
			m_measurementRunning = false;
			m_sramFillLevel=9999999; 
			m_collectedTriggers = 0;
			m_triggerRate = 0;
			// TODO: to be implemented
			m_sramReadoutReady = false;
			m_measurementPause = false;
			m_collectedHits = 0;
			m_eventRate = 0;
			if(m_srcSecMax > 0){ // time-based source measurement: check if time is up
			  int now = clock()/CLOCKS_PER_SEC;
			  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: sec's now:" << now << endl;
			  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: sec's at start:" << m_srcSecStart << endl;
			  if((now-m_srcSecStart)>m_srcSecMax) stopScan();
			}
			// to do : check if the displayed quantities can be revised for >1 FE
			int collectedTriggersTotal=0;
			for (std::vector<int>::iterator it = m_chipIds.begin();
			     it != m_chipIds.end(); it++){
			        // stop loop if no more meaningful chip IDs or after 1st chip if in multichip mode (always have one board then!)
			        if(*it==999 || (m_MultiChipWithSingleBoard && it != m_chipIds.begin())) break;
				if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: m_USBpix->GetSourceScanStatus() board with chip ID "<< (*it) << endl;
				bool measurementRunning=true; //, sramFull;
				int sramFillLevel=0, collectedTriggers=0, triggerRate=0, eventRate=0;
				m_USBpix->GetSourceScanStatus(m_sramFull, measurementRunning, sramFillLevel, collectedTriggers, triggerRate, eventRate, m_tluVeto, *it);		// added m_tluVeto

				m_measurementRunning |= measurementRunning;
				if(m_sramFillLevel > sramFillLevel) m_sramFillLevel = sramFillLevel;
				if(m_triggerRate < triggerRate) m_triggerRate = triggerRate;
				if(m_eventRate < eventRate) m_eventRate = eventRate;
				collectedTriggersTotal += collectedTriggers;
				if(!measurementRunning && !m_testBeamFlag /* && m_sramReadoutReady*/) { // TODO
				        if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: MeasRunning(" << (*it) << ") = false => reading SRAM..."<<endl;
					m_USBpix->WriteStrbStop(); // to be sure injection/trigger FSM is stopped. Additional call does no harm.
					m_USBpix->StopCcpdInjections();	// in case strobe was started; shouldn't harm otherwise
					m_USBpix->SetNumberOfEvents(0); // Make sure number of events to count is 0 after scan. Might be troublesome in strobe scan with external/self triggering otherwise.
					m_SourceScanEventQuantity = 0;
					m_USBpix->ReadSRAM(*it);
					if (m_MultiChipWithSingleBoard)
					{
						if(m_fillSrcHistos)
						  m_USBpix->FillHistosFromRawData(*it);
						for (std::vector<int>::iterator it2 = m_chipIds.begin(); it2 != m_chipIds.end(); it2++)
						{
							writeRawDataFile(false, *it2);
						}
						// USBpix somehow doesn't make use of chipID in MutliChip mode
						if(m_fillClusterHistos)
						  ClusterRawData(*it);
					}
					else
					{
						if(m_fillSrcHistos)
							m_USBpix->FillHistosFromRawData(*it);
						writeRawDataFile(false, *it);
						if(m_fillClusterHistos)//DLP
							ClusterRawData(*it);
					}
					m_USBpix->ClearSRAM(*it);
				} else if(measurementRunning  && !m_testBeamFlag && (m_sramFull || m_tluVeto) ){ // when using TLU, TLU veto seems to be raised before SRAM full flag, so must trigger on the former
				        if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: SRAMFull(" << (*it) << ") => reading SRAM..."<<endl;
					int tbefore = clock()/CLOCKS_PER_SEC;
					int intEnRJ45 = m_USBpix->ReadRegister(CS_ENABLE_RJ45);
					m_USBpix->WriteRegister(CS_ENABLE_RJ45, 0);
					m_USBpix->PauseMeasurement();
					m_USBpix->ReadSRAM(*it);
					int tafter = clock()/CLOCKS_PER_SEC;
					m_srcSecStart += tafter-tbefore;
					if (m_MultiChipWithSingleBoard)
					{
						if(m_fillSrcHistos)
						  m_USBpix->FillHistosFromRawData(*it);
						for (std::vector<int>::iterator it2 = m_chipIds.begin(); it2 != m_chipIds.end(); it2++)
						{
							writeRawDataFile(false, *it2);
						}
						// USBpix somehow doesn't make use of chipID in MutliChip mode
						if(m_fillClusterHistos)
						  ClusterRawData(*it);
					}
					else
					{
						if(m_fillSrcHistos)
							m_USBpix->FillHistosFromRawData(*it);
						writeRawDataFile(false, *it);
						if(m_fillClusterHistos)//DLP
							ClusterRawData(*it);
					}
					m_USBpix->ClearSRAM(*it);
					m_USBpix->ResumeMeasurement();
					m_USBpix->WriteRegister(CS_ENABLE_RJ45, intEnRJ45);
					//m_USBpix->StartReadout();
				}
			}
			if ((m_SourceScanEventQuantity != 0) && (collectedTriggersTotal >= m_SourceScanEventQuantity)){
			  if(UPC_DEBUG_GEN) cout<<"DEBUG: reached event limit, stopping scan" << std::endl;
				stopScan();
			}
			if(UPC_DEBUG_GEN) cout<<"DEBUG: SRAMReadoutReady: "<<(m_sramReadoutReady?"true":"false")<<" SRAMFull: "<<(m_sramFull?"true":"false")<<" measurementPause: "<<(m_measurementPause?"true":"false")<<" measurementRunning: "<<(m_measurementRunning?"true":"false")<<" collectedTriggers: "<<m_collectedTriggers<<" collectedHits: "<<m_collectedHits<<" triggerRate: "<<m_triggerRate<<" eventRate: "<<m_eventRate<<" tluVeto: "<<m_tluVeto<<endl;
			if(!m_measurementRunning && !m_testBeamFlag)
				// m_sourceScanDone has to be asserted here, not in startScan()
				// USBpix::StartMeasurement() is called once, but not active during source scan
			        // if in testbeam mode, taken care of by getSourceScanData
				m_sourceScanDone=true;
			// TODO
			// for now it is fine
			if(m_collectedTriggers < collectedTriggersTotal)
				m_collectedTriggers = collectedTriggersTotal;
			return m_collectedTriggers; // do not follow ROD convention in source scan mode
			// for later implementation
			//switch(m_srcCounterType){
			//  case PixScan::COUNT_TRIGGER:
			//    return m_collectedTriggers; // do not follow ROD convention in source scan mode
			//  case PixScan::COUNT_HITS:
			//    return m_collectedHits; // do not follow ROD convention in source scan mode
			//}
		} else if (m_sourceScanBusy && m_upcScanInit) { // source scan during initialization
			m_sramReadoutReady = false;
			m_sramFull = false;
			m_measurementPause = false;
			m_measurementRunning = true;
			m_sramFillLevel = 0;
            m_tluVeto = false;
			m_collectedTriggers = 0;
			m_collectedHits = 0;
			m_triggerRate = 0;
			m_eventRate = 0;
			return 0; // there is nothing to return, the scan/measurement has not yet started
		} else { // not covered??? d'oh...
			return -1;
		}
	} else {
		return 0; // set to zero for the case that nTrigger() is called before syncScan()
	}
}

bool USBPixController::getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram) 
{	
	const int data_size = SRAM_WORDSIZE/MAX_CHIP_COUNT;

	if(m_sourceScanBusy && !m_upcScanInit) 
	{
    		bool caseA = (m_measurementRunning && m_sramFull) || (!m_measurementRunning && forceReadSram)/* && m_sramReadoutReady*/;  // TODO
		bool caseB = !m_measurementRunning/* && m_sramReadoutReady*/ && !caseA; // TODO

		if(caseA || caseB)
		{
			if(UPC_DEBUG_GEN && caseA) std::cout << "DEBUG USBPixController::getSourceScanData : m_sramFull=" << (m_sramFull?"true":"false") <<
			" || "<<"forceReadSram=" << (forceReadSram?"true":"false") << " => reading SRAM..." << std::endl;
			if(UPC_DEBUG_GEN && caseB) std::cout << "DEBUG USBPixController::getSourceScanData : m_measurementRunning=false => reading SRAM..." << std::endl;

			std::vector<int> ch_assoc = m_USBpix->GetReverseReadoutChannelAssoc();

			for(std::vector<int>::iterator it = m_chipIds.begin(); it != m_chipIds.end(); it++)
			{
				// stop loop if no more meaningful chip IDs
				if(*it==999)
				{	
					if(UPC_DEBUG_GEN) std::cout << "DEBUG USBPixController::getSourceScanData: no meaningfull ID" << std::endl;
					break;
				}

				m_USBpix->ReadSRAM(*it);
				// copy sram-data to array in testbeam mode

				if(m_testBeamFlag)
				{
					if(UPC_DEBUG_GEN) std::cout << "DEBUG USBPixController::getSourceScanData : Copying data"<<std::endl;
					unsigned int* di = new unsigned int[data_size];
					m_USBpix->GetSRAMWordsRB(di, data_size, it - m_chipIds.begin());

        	  			data->push_back(di);
       				}
        			if(m_fillSrcHistos)
				{
        	  			m_USBpix->FillHistosFromRawData(*it);
				}

				writeRawDataFile(false, *it);

				if(m_fillClusterHistos)
				{
					ClusterRawData(*it);
				}
      			}

			//clear SRAM loop
			for(std::vector<int>::iterator it = m_chipIds.begin(); it != m_chipIds.end(); it++)
			{
        			// stop loop if no more meaningful chip IDs
				if(*it==999)
				{	
					if(UPC_DEBUG_GEN) std::cout << "DEBUG USBPixController::getSourceScanData: no meaningfull ID" << std::endl;
					break;
				}
				m_USBpix->ClearSRAM(*it);
			}
			if(caseB) m_sourceScanDone=true;
      			return true;
		} 
		else //not (caseA || caseB) 
		{
			return false;
    		}
  	}
	else //not (m_sourceScanBusy && !m_upcScanInit) 
	{
		return false;
  	}
}

void USBPixController::hwInfo(string &txt){
	txt = "";
	for(int ib=0;ib<NBOARDS_MAX;ib++){
		std::stringstream a;
		a << ib;
		if(m_BoardHandle[ib]!=0){
			txt  += "Board "+a.str()+" Name:  " + string(m_BoardHandle[ib]->GetName()) + "\n";
			stringstream a;
			a << m_BoardHandle[ib]->GetId();
			txt += "Board ID: " + a.str() + "\n";
			stringstream b;
			b << m_BoardHandle[ib]->GetFWVersion();
			txt += "uC Firmware: " + b.str() + "\n";
			// create temporary adapter card interace to read ID
			if (m_AdapterCardFlavor == 2) {
			  gpac::logical * gl = getGpac();
			  if (gl) {
			    stringstream c;
			    c << gl->getId();
			    txt += "Adapter card ID: " + c.str() + "\n";
			  }
			} else {
			  USBPixDCS *USBADC;
			  if(m_AdapterCardFlavor==1)
			    USBADC = new USBPixBIDCS(m_BoardHandle[ib]);
			  else
			    USBADC = new USBPixSTDDCS(m_BoardHandle[ib]);
			  USBADC->Init();
			  stringstream c;
			  c <<     USBADC->GetId();
			  txt += "Adapter card ID: " + c.str() + "\n";
			  delete USBADC;
			}
		}
	}
	return;
}

void USBPixController::shiftPixMask(int mask, int cap, int steps){
  // translate mask options
  int shiftmask;
  int capMask[3]={SHIFT_CAP1, SHIFT_CAP0, SHIFT_CAP1+SHIFT_CAP0};
  switch(mask) {
  case PixScan::SEL:          shiftmask = capMask[cap]; break;
  case PixScan::ENA:          shiftmask = SHIFT_ENABLE; break;
  case PixScan::SEL_ENA:      shiftmask = SHIFT_ENABLE+capMask[cap]; break;
  case PixScan::XTALK:        shiftmask = SHIFT_ENABLE+0x40+0x80; break;  
  case PixScan::HITB:         shiftmask = SHIFT_HITBUS; break;
  case PixScan::HITB_INV:     shiftmask = SHIFT_INVHB; break;
  case PixScan::DINJ:         shiftmask = SHIFT_DIGINJ; break;
  case PixScan::ENA_HITB:     shiftmask = SHIFT_ENABLE+SHIFT_HITBUS; break;
  case PixScan::SEL_ENA_HITB: shiftmask = SHIFT_ENABLE+SHIFT_HITBUS+capMask[cap]; break;
  case PixScan::STATIC:       shiftmask = 0x00; break;
  default:                    shiftmask = 0x00; break; // static
  }
  if(m_USBpix!=0){
    const int nmasks=8;
    int maskTags1[nmasks]={SHIFT_ENABLE, SHIFT_CAP0, SHIFT_CAP1, SHIFT_HITBUS, SHIFT_INVHB, SHIFT_DIGINJ, 0x40, 0x80};
    int maskTags2[nmasks]={ENABLE, CAP0, CAP1, HITBUS, HITBUS, DIGINJ, CAP0, CAP1};
    for(int imask=0;imask<nmasks;imask++){
      if(shiftmask&maskTags1[imask]){ // shift only if mask is selected
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: shifting mask " << maskTags1[imask] << " via register " << maskTags2[imask] << endl;
	//for(int ib=0;ib<NBOARDS_MAX;ib++){
        for (std::vector<int>::iterator it = m_chipIds.begin();
	     it != m_chipIds.end(); it++)
	  {
	    if(*it==999) break;
	    m_USBpix->ShiftPixMask(maskTags2[imask], steps, *it, (maskTags1[imask]==SHIFT_HITBUS || 
								  maskTags1[imask]==0x40 || 
								  maskTags1[imask]==0x80));
	  }
      }
    }
  }
  return;
}

int USBPixController::readHitBusScaler(int mod, int /*ife*/, PixScan* /*scn*/){
  // only use module that matches adapter board channel if in burn-in mode
  if(!moduleActive(mod)) return 0;
  else                   return (int)checkRxState(RX0);
}

void USBPixController::updateDeviceHandle(){
  for(int ib=0;ib<NBOARDS_MAX;ib++){
    if(m_BoardHandle[ib]!=0){
      if(UPC_DEBUG_GEN) cout <<"INFO: USBPixController::updateDeviceHandle is calling GetUSBDevice for board " << ib << endl;
      void * tempHandle = GetUSBDevice(m_boardID[ib]);
      //      if(UPC_DEBUG_GEN) cout <<"INFO: USBPixController::updateDeviceHandle got " << std::hex << ((int)tempHandle) << std::dec << " from GetUSBDevice" << endl;
      if(tempHandle!=0){
	m_BoardHandle[ib]->SetDeviceHandle(tempHandle);
	if(UPC_DEBUG_GEN) cout <<"INFO: USBPixController::updateDeviceHandle found "<< m_BoardHandle[ib]->GetName() << " with ID " << m_BoardHandle[ib]->GetId() <<endl;
	m_boardIDRB = m_BoardHandle[ib]->GetId();
      } else{
	if(UPC_DEBUG_GEN) cout<<"WARNING: no board handle found for board ID " << m_boardID[ib] << endl;
	delete m_BoardHandle[ib]; m_BoardHandle[ib]=0;
	throw USBPixControllerExc(USBPixControllerExc::NOBOARD, PixControllerExc::WARNING, getCtrlName(), m_boardID[ib]); 
      }  
    }
  }

  if (m_AdapterCardFlavor == 2)
  {
    gpac::logical * gl = getGpac();
    if (gl)
    {
      gl->updateDev(m_BoardHandle[0]);
    }
  }
}

SiUSBDevice* USBPixController::getUsbHandle(int id){
  if (m_MultiChipWithSingleBoard)
  {
    if (id)
      return 0;
    return m_BoardHandle[0];
  }
  else
  {
    if(id<0 || id>=NBOARDS_MAX) return 0;
    try{
      if(m_chipIds.at(id)==999) return 0;
    }catch(...){
      return 0;
    }
    return m_BoardHandle[id];
  }
}

bool USBPixController::checkRxState(rxTypes type){
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
}

void USBPixController::getServiceRecords(std::string &txt, std::vector<int> &srvCounts){
  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: called getServiceRecords" << endl;
  int nFe=0;
  const unsigned int nRec=32;
  const unsigned int asize = SRAM_WORDSIZE-1;
  unsigned int *SRAMwordsRB = (unsigned int*) malloc(asize * sizeof(unsigned int));
  if(SRAMwordsRB==0) throw USBPixControllerExc(USBPixControllerExc::ALLOC_PROBLEM, PixControllerExc::ERROR, getCtrlName()); 
  
  nFe = getFECount();
  srvCounts.clear();
  srvCounts.resize(nRec*nFe); // service records have 32 elements
  txt = "";
  m_USBpix->StartMeasurement();
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
    if(*it==999) break;
    m_USBpix->SendReadErrors(*it);
    if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: finished m_USBpix->SendReadErrors(m_chipIds.at(ib)) for chip " << *it << endl;
  }
  sleep(500);
  m_USBpix->StopMeasurement();

	// Not needed since interface function implemented
	std::stringstream srvrecStream;
	for(int ib=0;ib<nFe;ib++){
	  m_USBpix->ReadSRAM(m_chipIds.at(ib));
	  m_USBpix->GetSRAMWordsRB(SRAMwordsRB, asize, m_chipIds.at(ib));
	  if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: got SSRAMwordsRB for FE " << ib << endl;
	  for (int word = 0; word < (int)asize; word++){
	    if (!EMPTY_RECORD_MACRO(SRAMwordsRB[word]) && SERVICE_RECORD_MACRO(SRAMwordsRB[word])){
	      unsigned int recID  = SERVICE_RECORD_CODE_MACRO(SRAMwordsRB[word]);
	      unsigned int recCnt = SERVICE_RECORD_COUNTER_MACRO(SRAMwordsRB[word]);
	      if(recID<nRec) srvCounts[recID+ib*nRec] = recCnt;
	    }
	  }
	  m_USBpix->ClearSRAM(m_chipIds.at(ib));
	  // printf out - do it here such that record appear in order of their ID
	  srvrecStream << "FE " << ib << std::endl;
	  for(unsigned int recID=0;recID<nRec;recID++)
	    srvrecStream << "Code \t" << recID << "\t Counter \t" << srvCounts[recID+ib*nRec] << std::endl;
	}
	txt = srvrecStream.str();
	delete SRAMwordsRB;
	if(UPC_DEBUG_GEN) cout<<"DEBUG USBPixCtrl: service record request returns " << txt << endl;
}
bool USBPixController::testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp, 
				     bool shift_only, bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex){
  USBPixDcs *updcs = dynamic_cast<USBPixDcs*>(dcs);
  USBPixDCS *dll_upd=0;
  if(updcs!=0) dll_upd = updcs->getHwPtr();
  // to do: define specific exception for this type of error
  else throw USBPixControllerExc(USBPixControllerExc::NO_PIXDCS, PixControllerExc::ERROR, getCtrlName()); 

  int value, add, size;
  int scanChain = -1, indStart=-1, indEnd=-1;
  bool passFlag = true;

  if(chainName=="DOB"){
    scanChain = SC_DOB;
    indStart = SCDOB0;
    indEnd = SCDOB2;
  }
  else if(chainName=="CMD"){
    scanChain = SC_CMD;
    indStart = SCCMD0;
    indEnd = SCCMD8;
  }
  else if(chainName=="ECL"){
    scanChain = SC_ECL;
    if(m_USBpix->FEisFEI4B()){
      indStart = B_SCECL0;
      indEnd = B_SCECL108;
    } else{
      indStart = SCECL0;
      indEnd = SCECL99;
    }
  }
  // to do: define specific exception for this type of error
  else throw USBPixControllerExc(USBPixControllerExc::UNKNOWN_CHAIN, PixControllerExc::ERROR, getCtrlName()+" (scan chain "+
				 chainName+")"); 

  if(data_in.size()!=(unsigned int)(indEnd-indStart+1)) 
    throw USBPixControllerExc(USBPixControllerExc::WRONG_DATASIZE, PixControllerExc::ERROR, getCtrlName()); 

  for (int index = indStart; index <= indEnd; index++)
    m_USBpix->SetScanChainValue(scanChain, index, data_in[index-indStart], feIndex);

  m_USBpix->RunScanChain(scanChain, dll_upd, curr_bef, curr_after, shift_only, se_while_pulse, si_while_pulse, feIndex);

  data_out ="";
  for (int index = indStart; index <= indEnd; index++){
    m_USBpix->GetScanChainValueRB(scanChain, index, size, add, value, feIndex);
    std::string rval,eval;
    for(int is=0; is<size;is++){
      rval = ((1<<(size-is-1))&value)?"1":"0";
      eval = data_cmp.substr((index-indStart)*32+is,1);
      data_out += rval;
      // check if correct and ignore if undef. expected value (flagged as X)
      if(rval != eval && eval != "X" && eval != "x" ) passFlag = false;
    }
  }

  return passFlag;
}
void USBPixController::sendGlobalPulse(int length){
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
		if (*it==999) break;
    m_USBpix->WriteCommand(FE_GLOBAL_PULSE, *it, length);
  }
}

void USBPixController::sendPixelChargeCalib(int /*pModuleID*/, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge)
{
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
		if (*it==999) break;
		m_USBpix->SetChargeCalib(*it, pCol, pRow, pTot, pCharge);
	}
}
bool USBPixController::getGenericBuffer(const char *type, std::string &textBuf) {
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

int USBPixController::getFECount()
{
  int nFe = 0;
  for (std::vector<int>::iterator it = m_chipIds.begin();
      it != m_chipIds.end(); it++)
  {
		if (*it==999) continue;
		nFe++;
	}
  return nFe;
}
void USBPixController::measureEvtTrgRate(PixScan *scn, int /*mod*/, double &erval, double &trval){
  int nmeas = scn->getNptsRateAvg();
  if(UPC_DEBUG_GEN) cout << "USBPixController : starting src meas." << endl;
  startScan(scn);
  erval = 0.;
  trval = 0.;
  if(UPC_DEBUG_GEN) cout << "USBPixController : starting rate meas." << endl;
  for(int i=0;i<nmeas;){
    sleep(1200); // must wait > 1s, otherwise FPGA isn't updated
    nTrigger(); // update readings
    if(getSRAMFillLevel()<95) { // only read when SRAM not close to full, otherwise rates might be wrong
      erval += (double)getEventRate();
      trval += (double)getTriggerRate();
      i++;
      if(UPC_DEBUG_GEN) cout << "USBPixController : meas." << i << " has trg. rate of " << trval << " and evt. rate of " << erval << endl;
    }
  }
  if(UPC_DEBUG_GEN) cout << "USBPixController : stopping src meas." << endl;
  stopScan();
  if(UPC_DEBUG_GEN) cout << "USBPixController : waiting for meas. stop" << endl;
  int iwait=0;
  nTrigger();
  while(runStatus() && iwait<1000){
    sleep(100);
    iwait++;
    nTrigger();
  }
  if(UPC_DEBUG_GEN) cout << "USBPixController : storing rate meas., waited " << iwait << endl;
  erval /= (double)nmeas;
  trval /= (double)nmeas;
}

void USBPixController::finalizeScan()
{
  if (m_createdRawDataFile)
  {
    for (std::vector<int>::iterator it = m_chipIds.begin();
        it != m_chipIds.end(); it++)
    {
      writeRawDataFile(true, *it);
    }
    m_createdRawDataFile = false;
    m_USBpix->FinishFileFromRawData(m_rawDataFilename);
  }
}

void USBPixController::configureReadoutChannelAssociation()
{
  if(m_USBpix!=NULL)
  {
    bool run_auto_detect = false;

    for (std::vector<int>::iterator it = m_readoutChannelReadsChip.begin();
        it != m_readoutChannelReadsChip.end(); it++)
    {
      if (((*it) == RCA_AUTO) || ((*it) == RCA_AUTOSAVE))
      {
        run_auto_detect = true;
        break;
      }
    }

    bool erroneous_autodetection = false;
    if (run_auto_detect)
    {
      erroneous_autodetection = !m_USBpix->autoDetectReadoutChannels();

      if (erroneous_autodetection)
      {
        std::stringstream err;
        err << "Controller ";
        err <<  getCtrlName();
        err << ": Readout channel autodetection failed for at least one chip." << std::endl;
        m_errBuff += err.str();
      }

      for (std::vector<int>::iterator it = m_readoutChannelReadsChip.begin();
          it != m_readoutChannelReadsChip.end(); it++)
      {
        std::vector<int> autodetected = m_USBpix->GetReverseReadoutChannelAssoc();
        if (((*it) != RCA_AUTO) && ((*it) != RCA_AUTOSAVE))
        {
          int ch = it - m_readoutChannelReadsChip.begin();
          int dv = autodetected.at(ch);

          if (dv != *it)
          {
            if (UPC_DEBUG_GEN)
              std::cerr << __FILE__ << ":" << __LINE__ << ": "
                << "Readout channel autodetection contradicts settings."
                << "Readout channel " << ch << ", chip " << *it 
                << " configured, chip " << dv << " detected." << std::endl;
            std::stringstream err;
            err << "Controller ";
            err <<  getCtrlName();
            err << ": Readout channel autodetection contradicts settings."
              << "Readout channel " << ch << ", chip " << *it 
              << " configured, chip " << dv << " detected." << std::endl;
            m_errBuff += err.str();
            erroneous_autodetection = true;
          }
        }
      }
    }

    if (erroneous_autodetection)
    {
      std::stringstream err;
      err << "Controller ";
      err <<  getCtrlName();
      err << ": Readout channel autodetection failed. ";
      err << "The configuration will default to the following and the current scan will continue. ";
      err << "Either completely autodetect the readout channels or set all manually. ";
      for (std::vector<int>::iterator it = m_USBpix->ReadoutChannelAssoc.begin();
          it != m_USBpix->ReadoutChannelAssoc.end(); it++)
      {
        err << "CH" << (it -  m_USBpix->ReadoutChannelAssoc.begin()) << " reads chip " << (*it) << ". ";
      }
      err << std::endl;
      m_errBuff += err.str();
    }
    else
    {
      std::vector<int> autodetected = m_USBpix->GetReverseReadoutChannelAssoc();

      for (std::vector<int>::iterator it = m_readoutChannelReadsChip.begin();
          it != m_readoutChannelReadsChip.end(); it++)
      {
        int ch = it - m_readoutChannelReadsChip.begin();
        if (((*it) != RCA_AUTO) && ((*it) != RCA_AUTOSAVE))
        {
          autodetected.at(ch) = *it;
        }
        if ((*it) == RCA_AUTOSAVE)
        {
          (*it) = autodetected.at(ch);
        }
      }
      m_USBpix->SetReverseReadoutChannelAssoc(autodetected);
    }
  }		
}
    
gpac::logical* USBPixController::getGpac()
{
  if (!m_BoardHandle[0])
  {
    if (m_gpac.get() != nullptr)
    {
      m_gpac->updateDev(nullptr);
    }
    return nullptr;
  }
  
  if (!m_gpac.get())
  {
    m_gpac.reset(new gpac::logical(m_BoardHandle[0]));
  }
  return m_gpac.get();
}
int USBPixController::detectAdapterCard(unsigned int boardId){
  if(boardId>=NBOARDS_MAX) return -1;
  if(m_BoardHandle[boardId]==0) return -1;

  // read info about adapter board from EEPROM
  bool status;
  unsigned char addBuf[2]={0,0}, data[2]={0,0};
  
  status = m_BoardHandle[boardId]->WriteI2C(CAL_EEPROM_ADD, addBuf, 2);
  status &= m_BoardHandle[boardId]->ReadI2C(CAL_EEPROM_ADD | 0x01, &data[0], 2);

  if (!status) return -1;

  int header = (data[0] << 8) + data[1];
  switch (header){
  case CAL_DATA_HEADER_V2:  // it is a BIC
    return 1;
  case CAL_DATA_HEADER_V1:{ // it is a SCA or GPAC
    unsigned int nPages, nBytes;
    unsigned int dataPtr, size;
    size = sizeof(eepromDataStructV1);
    nPages = size / CAL_EEPROM_PAGE_SIZE;
    nBytes = size % CAL_EEPROM_PAGE_SIZE;
    unsigned char *dataBuf = new unsigned char[size];
    status = m_BoardHandle[boardId]->WriteI2C(CAL_EEPROM_ADD, addBuf, 2);
    dataPtr = 0;
    for (unsigned int i = 0; i < nPages; i++){ // 64 byte page write
      status &= m_BoardHandle[boardId]->ReadI2C(CAL_EEPROM_ADD | 0x01, &dataBuf[dataPtr], CAL_EEPROM_PAGE_SIZE);
      dataPtr += CAL_EEPROM_PAGE_SIZE;
    }
    if (nBytes > 0){
      status &= m_BoardHandle[boardId]->ReadI2C(CAL_EEPROM_ADD | 0x01, &dataBuf[dataPtr], nBytes);
    }
    char cal_name[MAX_PSU_NAME_SIZE];
    memcpy((unsigned char*)cal_name, &dataBuf[4], MAX_PSU_NAME_SIZE);
    delete[] dataBuf;
    if(std::string(cal_name)=="PWR0")
      return 2; // GPAC
    else
      return 0; // SCA
  }
  default:                  // no board
    return -1;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// M.B.: CCPD Support ////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void USBPixController::writeCcpdConfig(PixModule& mod) {
  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called writeCcpdConfig()"<<endl;

  int moduleId = mod.moduleId();
  
  PixCcpd* ccpd = mod.pixCCPD();
  PixCcpdv1* ccpdv1 = dynamic_cast<PixCcpdv1*>(ccpd);
  PixCcpdv2* ccpdv2 = dynamic_cast<PixCcpdv2*>(ccpd);
  PixCcpdLF* ccpdLF = dynamic_cast<PixCcpdLF*>(ccpd);

  if(ccpdv1!=0){
    if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() CCPD V1 found"<<endl;
    m_USBpix->SetCCPDVersion((int)ConfigCCPDMemory::HV2FEI4_V1);
    Config &cfg = ccpd->config();

    // copy CCPD global register content
	if(cfg["global"].name()!="__TrashConfGroup__"){
		for(std::map<std::string, int>::iterator it=m_ccpd_globals.begin(); it!=m_ccpd_globals.end(); it++){
			if(cfg["global"][it->first].name()!="__TrashConfObj__")
				m_USBpix->SetCcpdGlobalVal(it->second, ((ConfInt&)cfg["global"][it->first]).getValue());
		
		}
    }

    // and now the pixel register content
	if(cfg["pixel"].name()!="__TrashConfGroup__"){
    /*  if(cfg["pixel"]["PRtest"].name()!="__TrashConfObj__"){
	ConfMask<unsigned short int> &prVal = ((ConfMatrix&)cfg["pixel"]["PRtest"]).valueU16();
	for (unsigned int col=0; col<prVal.get().size(); col++) {
	  for (unsigned int row=0; row<prVal[0].size(); row++) {
	    unsigned int content = prVal[col][row];
	    // to do: write "content" to USBPix for all mapped CCPD pixels
	  }
	}
      }*/
		if(cfg["pixel"]["INDAC"].name()!="__TrashConfObj__"){
		ConfMask<unsigned short int> &InDAC = ((ConfMatrix&)cfg["pixel"]["INDAC"]).valueU16();
			if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() pixel register" <<endl;
			for (unsigned int col=0; col<InDAC.get().size(); col++) {
					for (unsigned int row=0; row<InDAC[0].size(); row++) {
						// if pixel is in	col 0,3,6...57 -> Wr_ID=0 ? col % 3 -> 0
						//					col 1,4,7...58 -> Wr_ID=1 ? col % 3 -> 1
						//					col 2,5,8...59 -> Wr_ID=2 ? col % 3 -> 2
						//					row 0,2,4...22 -> InDACR ?? Needs to be tested
						//					row 1,3,5...23 -> InDACL ?? Needs to be tested
						if ((row % 2)==0){
							m_USBpix->SetCcpdColCtrlVal(CCPD_WR, (col % 3) ,  (col % 3), col/3);
							m_USBpix->SetCcpdRowCtrlVal(CCPD_INDACR, InDAC[col][row] , (col % 3) , row/2, col/3);
						}
						else {
							m_USBpix->SetCcpdColCtrlVal(CCPD_WR, (col % 3) ,  (col % 3), col/3);
							m_USBpix->SetCcpdRowCtrlVal(CCPD_INDACL, InDAC[col][row] , (col % 3) , row/2, col/3);
						}
					}
			}
		}
		
		if(cfg["pixel"]["EnR"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_ENR, ((ConfBool&)cfg["pixel"]["EnR"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["EnL"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_ENL, ((ConfBool&)cfg["pixel"]["EnL"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["L0"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_L0, ((ConfBool&)cfg["pixel"]["L0"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["L1"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_L1, ((ConfBool&)cfg["pixel"]["L1"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["L2"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_L2, ((ConfBool&)cfg["pixel"]["L2"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["R0"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_R0, ((ConfBool&)cfg["pixel"]["R0"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["R1"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_R1, ((ConfBool&)cfg["pixel"]["R1"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["R2"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_R2, ((ConfBool&)cfg["pixel"]["R2"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["EnCurrent"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_ENCURRENT, ((ConfBool&)cfg["pixel"]["EnCurrent"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

		if(cfg["pixel"]["EnStrip"].name()!="__TrashConfObj__"){
			m_USBpix->SetCcpdColCtrlVal(CCPD_ENSTRIP, ((ConfBool&)cfg["pixel"]["EnStrip"]).value(), -1, -1); // Wr_ID=-1  || col=-1 means broadcast to all 			
		}

	}

	// copy CCPD on PCB register content
    if(cfg["OnPcbDACs"].name()!="__TrashConfGroup__"){
		m_USBpix->SetCcpdThr(((ConfInt&)cfg["OnPcbDACs"]["CCPD_Threshold"]).getValue());
		m_USBpix->SetCcpdVcal(((ConfInt&)cfg["OnPcbDACs"]["CCPD_Vcal"]).getValue());
	}

	
  } else if(ccpdv2!=0){ //------------------Version 2---------------------------------------
    if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() CCPD V2 found"<<endl;
    m_USBpix->SetCCPDVersion((int)ConfigCCPDMemory::HV2FEI4_V2);
    Config &cfg = ccpd->config();
	
	// copy CCPD global register content
	if(cfg["global"].name()!="__TrashConfGroup__"){
		for(std::map<std::string, int>::iterator it=m_ccpd_globals2.begin(); it!=m_ccpd_globals2.end(); it++){
			if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() V2 global it->first"<< it->first <<endl;
			if(cfg["global"][it->first].name()!="__TrashConfObj__")
				
				m_USBpix->SetCcpdGlobalVal(it->second, ((ConfInt&)cfg["global"][it->first]).getValue());
		
		}
    }
	bool StripReadout = false;
	bool DirectCurrentReadout = false;
	bool SimplePixel = false;
	
	if(cfg["pixel"].name()!="__TrashConfGroup__"){
	
		//get values from config
		
		// Simple Pixel
		if(cfg["pixel"]["simplepixel"].name()!="__TrashConfObj__"){
			SimplePixel = ((ConfBool&)cfg["pixel"]["simplepixel"]).value(); 	
			m_USBpix->SetCcpdsimplepixelValue(SimplePixel);
		}
		
		//row items
		if(cfg["pixel"]["Monitor"].name()!="__TrashConfObj__"){
			ConfMask<unsigned short int> &Monitor = ((ConfMatrix&)cfg["pixel"]["Monitor"]).valueU16();
			m_USBpix->ResetCcpdMonitor();
			if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() ResetCcpdMonitor called" << endl;
			for (unsigned int col=0; col<Monitor.get().size(); col++) {
				for (unsigned int row=0; row<Monitor[0].size(); row++) {
					m_USBpix->SetCcpdMonitorValue(col, row, Monitor[col][row]);	
				}
			}
		}
		if(cfg["pixel"]["INDAC"].name()!="__TrashConfObj__"){
			ConfMask<unsigned short int> &InDAC = ((ConfMatrix&)cfg["pixel"]["INDAC"]).valueU16();
			for (unsigned int col=0; col<InDAC.get().size(); col++) {
				for (unsigned int row=0; row<InDAC[0].size(); row++) {
					m_USBpix->SetCcpdInDacValue(col, row, InDAC[col][row]);
				}
			}
		}
		if(cfg["pixel"]["Enable"].name()!="__TrashConfObj__"){
			ConfMask<unsigned short int> &Enable = ((ConfMatrix&)cfg["pixel"]["Enable"]).valueU16();
			for (unsigned int row=0; row<Enable[0].size(); row++) {
				m_USBpix->SetCcpdEnableValue(row, Enable[0][row]);
			}
		}
		
		//col items
				
		if(cfg["pixel"]["Str"].name()!="__TrashConfObj__"){
			StripReadout = ((ConfBool&)cfg["pixel"]["Str"]).value(); 
			//FIXME at the moment all or no pixel enable, 	 
			m_USBpix->SetCcpdStripROValue(StripReadout); 				
		}
		if(cfg["pixel"]["dc"].name()!="__TrashConfObj__"){
			DirectCurrentReadout = ((ConfBool&)cfg["pixel"]["dc"]).value(); 
			//FIXME at the moment all or no pixel enable, 		
			m_USBpix->SetCcpddirectcurrentValue(DirectCurrentReadout);
		}
		if(cfg["pixel"]["Ampout"].name()!="__TrashConfObj__"){
			ConfMask<unsigned short int> &Ampout = ((ConfMatrix&)cfg["pixel"]["Ampout"]).valueU16();
			for (unsigned int col=0; col<Ampout.get().size(); col++) {
				m_USBpix->SetCcpdampoutValue(col, Ampout[col][0]);
			}
		}		
	}
  } else if(ccpdLF!=0){ //------------------Version LF---------------------------------------
  
	  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() CCPD LF found"<<endl;
		m_USBpix->SetCCPDVersion((int)ConfigCCPDMemory::HV2FEI4_LF);
		Config &cfg = ccpd->config();
		
	  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() after SetCCPDVersion"<<endl;
	  
		// copy CCPD global register content
		if(cfg["global"].name()!="__TrashConfGroup__"){
			for(std::map<std::string, int>::iterator it=m_ccpd_globalsLF.begin(); it!=m_ccpd_globalsLF.end(); it++){
				if(cfg["global"][it->first].name()!="__TrashConfObj__")
					m_USBpix->SetCcpdGlobalVal(it->second, ((ConfInt&)cfg["global"][it->first]).getValue());
			
			}
		}
		
		if(cfg["pixel"]["SW_Ana"].name()!="__TrashConfObj__"){
			ConfMask<unsigned short int> &SW_Ana = ((ConfMatrix&)cfg["pixel"]["SW_Ana"]).valueU16();
			for (unsigned int row=0; row<SW_Ana[0].size(); row++) {
				m_USBpix->SetCcpdLFSw_Ana(row, SW_Ana[0][row]);
			}
		}
		
		if(cfg["pixel"]["Pixels"].name()!="__TrashConfObj__"){
			ConfMask<unsigned short int> &Pixels = ((ConfMatrix&)cfg["pixel"]["Pixels"]).valueU16();
			for (unsigned int col=0; col<Pixels.get().size(); col++) {
				for (unsigned int row=0; row<Pixels[0].size(); row++) {
					m_USBpix->SetCcpdLFPixels(row, Pixels[col][row]);
				}
			}
		}
  } else{ 
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: writeCcpdConfig() CCPD version NOT found"<<endl;
  } 
  if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: end of writeCcpdConfig()"<<endl;
}

void USBPixController::StartCCPDInject() {
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called StartCCPDInject()"<<endl;
	m_USBpix->StartCcpdInjections(100, 10, 255);			
}

void USBPixController::StopCCPDInject() {
	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called StopCCPDInject()"<<endl;
	m_USBpix->StopCcpdInjections();			
}
void USBPixController::sendCCPDGlobal(){

	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendCCPDGlobal "<<endl;
	if(m_withCCPD){
		m_USBpix->WriteCcpdGlobal();
		m_USBpix->WriteCcpdOnPcbDacs();	
	}
	else if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendCCPDGlobal but no CCPD "<<endl;

}
void USBPixController::sendCCPDPixel(){

//CCPDTODO

	if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendCCPDPixel"<<endl;
	if(m_withCCPD){
		m_USBpix->WriteCcpdPixel(0);
	}
	else if(UPC_DEBUG_GEN) cout<<"INFO USBPixCtrl: called sendCCPDPixel	but no CCPD "<<endl;
}
void USBPixController::sendCCPD(){
	//sendCCPDGlobal();
	//sendCCPDPixel();
	m_USBpix->WriteCompleteCcpdShiftRegister();		
}
