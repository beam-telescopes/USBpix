
#include "SiLibUSB.h"
//#include "PixModuleGroup/PixModuleGroup.h"
//#include "PixConfDBInterface/PixConfDBInterface.h"
#include "USB3PixController.h"

#include "Config/Config.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeI4A.h"
#include "PixFe/PixFeI4B.h"

#include "board.h"
#include "histogrammer.h"
#include "datadecoder.h"
#include "readout.h"
#include "frontend.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <list>

#include <fstream>

#define U3PC_DEBUG false

using namespace std;

namespace PixLib {

USB3PixController::USB3PixController(PixModuleGroup &modGrp, DBInquire *dbInquire) : PixController(modGrp, dbInquire) {
        ch_to_fe = {{-2,-2,-2,-2,-2,-2,-2,-2}};
        fe_to_ch = {{-2,-2,-2,-2,-2,-2,-2,-2}};
	configInit();
	m_conf->read(dbInquire);
}

USB3PixController::USB3PixController(PixModuleGroup &modGrp, int cardFlavour) : PixController(modGrp) {
        ch_to_fe = {{-2,-2,-2,-2,-2,-2,-2,-2}};
        fe_to_ch = {{-2,-2,-2,-2,-2,-2,-2,-2}};
	configInit();
	m_cardFlavour = cardFlavour;
	if(m_cardFlavour>9) m_outputMode = CmdSeq::MANCHESTER_THOMAS; // works for MCC3, but apparently not (yet?) for MIO3
}

USB3PixController::~USB3PixController(void) {
  delete m_dev;
}

void USB3PixController::initHW(void) {
	if(U3PC_DEBUG) cout << "USB3PixController::initHW" << endl;

	m_dev = new SiUSBDevice(0);
	void *tempHandle = GetUSBDevice(m_boardID, 300);
	if(tempHandle==0) throw std::runtime_error("Pointer to SiUSBDevice is NULL!");
	m_dev->SetDeviceHandle(tempHandle);
	m_boardIDRB = m_dev->GetId();

	if(m_dev->GetBcdUSB() != 0x300)
		throw std::runtime_error("Board not reported as USB 3 device. Verify that a USB 3 cable and port are used.");

	if(m_FPGA_filename!=""){
	  if(U3PC_DEBUG) cout << "checking file " << m_FPGA_filename << endl;
	  FILE *f = 0;
#ifdef CF__LINUX
	  f = fopen(m_FPGA_filename.c_str(),"r");
#else
	  fopen_s(&f, m_FPGA_filename.c_str(),"r");
#endif
	  if(f==0){
	    if(U3PC_DEBUG) cout<<"ERROR: FPGA bit file " << m_FPGA_filename <<" doesn't exist"<<endl;
	    throw PixControllerExc(PixControllerExc::ERROR, getCtrlName()+" (FPGA bit file " + m_FPGA_filename + " doesn't exist)"); 
	  } else
	    fclose(f);
	}

	// temp. work-around for FW upload via JTAG
#ifdef CF__LINUX
	if(system("which xc3sprog >&/dev/null")==0){
	  if(U3PC_DEBUG) cout << "checking availability of JTAG programmer" << endl;
	  bool succ = (system("xc3sprog -c xpc >& /tmp/xctest.txt")==0);
	  system("cat /tmp/xctest.txt | grep XC7K > /tmp/xctest2.txt");
	  system("rm -f /tmp/xctest.txt");
	  if(U3PC_DEBUG) cout << "preparing upload" << endl;
	  int havexc7k = 0;
	  FILE *f = fopen("/tmp/xctest2.txt","r");
	  if(f!=0){
	    char line[2001];
	    while (fgets(line,2000, f)!=0) havexc7k++;
	    fclose(f);
	  }
	  system("rm -f /tmp/xctest2.txt");
	  if(succ && havexc7k==1){
	      if(U3PC_DEBUG) cout << "uploading FW via command: xc3sprog -c xpc -p 0 "+m_FPGA_filename << endl;
	      succ = (system(("xc3sprog -c xpc -v -p 0 "+m_FPGA_filename).c_str())==0);
	  }
	  else succ = false;
	  
	  if(!succ){
	    if(U3PC_DEBUG) cout<<"ERROR: FPGA didn't configure"<<endl;
	    throw PixControllerExc(PixControllerExc::ERROR, getCtrlName()+" (FPGA didn't configure)");
	  }
	}
#else // WINDOWS
	if(system("where \"%PATH%:xc3sprog\"")==0) {
	  std::string stPath = getenv ("TEMP");
	  if(U3PC_DEBUG) cout << "checking availability of JTAG programmer"<< endl;
	  bool succ = (system(("xc3sprog -c xpc > nul 2> \""+stPath+"\\xctest.txt\"").c_str())==0);
	  int havexc7k = 0;
	  FILE *f = 0;
	  size_t pos=0;
	  while((pos=stPath.find("\\"))!=std::string::npos)
			stPath = stPath.replace(pos, 1, "/");
	  stPath += "/xctest.txt";
  	  fopen_s(&f, stPath.c_str(),"r");
	  if(f!=0){
	    char line[2001];
	    while (fgets(line,2000, f)!=0){
			std::string sline(line);
			if(sline.find("JTAG loc")!=std::string::npos)
				havexc7k++;
		}
	    fclose(f);
	  }
	  system("del /F \"%TEMP%\\xctest.txt\"");
	  if(succ && havexc7k==1){
	      if(U3PC_DEBUG) cout << "uploading FW via command: xc3sprog -c xpc -p 0 "+m_FPGA_filename << endl;
	      succ = (system(("xc3sprog -c xpc -v -p 0 "+m_FPGA_filename).c_str())==0);
	  }
	  else succ = false;
	  
	  if(!succ){
	    if(U3PC_DEBUG) cout<<"ERROR: FPGA didn't configure"<<endl;
	    throw PixControllerExc(PixControllerExc::ERROR, getCtrlName()+" (FPGA didn't configure)");
	  }
	}
#endif

	if(!board) {
	  board = std::unique_ptr<Board>(new Board(m_dev, m_cardFlavour));
	}

	board->init(m_outputMode);
}

void USB3PixController::testHW(void) {                                          //! Hardware test
  if(U3PC_DEBUG) cout << "testHW" << endl;
  if(m_dev!=0 && m_dev->HandlePresent()){
    if(U3PC_DEBUG) cout <<"INFO: found "<< m_dev->GetName() << " with ID " << m_dev->GetId() <<endl;
  }else{
    throw std::runtime_error("HW not initialised");
  }
  return;
}

void USB3PixController::sendCommand(int command, int moduleMask){                                    //! Send command from PMG list
	if(U3PC_DEBUG) cout << "sendCommand" << endl;
	if(U3PC_DEBUG) cout << "module mask " << moduleMask << endl;

	switch(command) {
		case PixModuleGroup::PMG_CMD_HRESET_ON:
			if(U3PC_DEBUG) cout << "PMG_CMD_HRESET_ON" << endl;
			break;

		case PixModuleGroup::PMG_CMD_HRESET_OFF:
			if(U3PC_DEBUG) cout << "PMG_CMD_HRESET_OFF" << endl;
			break;

		case PixModuleGroup::PMG_CMD_HRESET_PLS:
			if(U3PC_DEBUG) cout << "PMG_CMD_HRESET_PLS" << endl;
			break;

		case PixModuleGroup::PMG_CMD_SRESET: {
			if(U3PC_DEBUG) cout << "PMG_CMD_SRESET" << endl;

			CommandBuffer c;

			for(auto &fe : frontends) {
				c += fe.second.confMode();
				c += fe.second.globalReset();
			}

			board->sendCommands(c);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			for(auto &fe : frontends) {
				c += fe.second.confMode();
				c += fe.second.runMode();
			}

			board->sendCommands(c);
			break;
		}

		case PixModuleGroup::PMG_CMD_ECR: {
			if(U3PC_DEBUG) cout << "PMG_CMD_ECR" << endl;

			CommandBuffer c;

			for(auto &fe : frontends) {
				c += fe.second.confMode();
			}

			c += CommandGenerator::ecr();

			board->sendCommands(c);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			for(auto &fe : frontends) {
				c += fe.second.confMode();
				c += fe.second.runMode();
			}

			board->sendCommands(c);
			break;
		}

		case PixModuleGroup::PMG_CMD_BCR: {
			if(U3PC_DEBUG) cout << "PMG_CMD_BCR" << endl;

			CommandBuffer c;

			for(auto &fe : frontends) {
				c += fe.second.confMode();
			}

			c += CommandGenerator::bcr();

			board->sendCommands(c);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			for(auto &fe : frontends) {
				c += fe.second.confMode();
				c += fe.second.runMode();
			}

			board->sendCommands(c);
			break;
		}
	}
}

void USB3PixController::sendCommand(Bits commands, int moduleMask){                                    //! Send command from pattern
	if(U3PC_DEBUG) cout << "sendCommand" << endl;
}

std::string &str(const char *c) {
	return *new std::string(c);
}

void USB3PixController::writeModuleConfig(PixModule& mod){                                             //! Write module configuration 
	if(U3PC_DEBUG) cout << "writeModuleConfig" << endl;

	if(U3PC_DEBUG) cout << "moduleID " << mod.moduleId() << endl;

	frontends.clear();
	fe_ids.clear();

	//module_config = std::unique_ptr<CommandBuffer>(new CommandBuffer);

	//CommandBuffer &c = *module_config;

	for(auto fe = mod.feBegin(); fe != mod.feEnd(); fe++) {
		int address = static_cast<ConfInt&>((*fe)->config()["Misc"]["Address"]).getValue();
		if(U3PC_DEBUG) cout << "address " << address << endl;

		if(Frontend::ROWS != (*fe)->nRow() || Frontend::COLUMNS != (*fe)->nCol()) {
			throw(std::runtime_error("Inconsistent FE geometry."));
		}

		if(dynamic_cast<PixFeI4A*>(*fe)) {
			frontends.emplace(std::piecewise_construct, std::forward_as_tuple(address), std::forward_as_tuple(address, Frontend::FEI4A));
		} else {
			frontends.emplace(std::piecewise_construct, std::forward_as_tuple(address), std::forward_as_tuple(address, Frontend::FEI4B));
		}

		fe_ids.push_back(address);

		Frontend &f = frontends.at(address);
		//c += f.confMode();

		f["Vthin_AltFine"] = 255;
		f["Vthin_AltCoarse"] = 255;

		//c += f.flushWrites();

		for(auto &reg : f) {
			f[reg] = (*fe)->readGlobRegister(reg);
		}

		//c += f.flushWrites();

		for(auto &reg : {"FDAC", "TDAC"}) {
			for(int column = 0; column < Frontend::COLUMNS; column++) {
				for(int row = 0; row < Frontend::ROWS; row++) {
					f.setPixelRegister(reg, column, row, (*fe)->readTrim(reg)[column][row]);
				}
			}
			//c += f.writePixelRegister(reg);
		}

		for(auto &reg : {"CAP1", "CAP0", "ILEAK", "ENABLE"}) {
			for(int column = 0; column < Frontend::COLUMNS; column++) {
				for(int row = 0; row < Frontend::ROWS; row++) {
					f.setPixelRegister(reg, column, row, (*fe)->readPixRegister(reg)[column][row]);
				}
			}
			//c += f.writePixelRegister(reg);
		}

// 		c += f.runMode();


// 		c += f.confMode(); //FIXME
// 		f["ReadErrorReq"] = 1;
// 		c += f.flushWrites();
// 		c += f.globalPulse(0);
// 		f["ReadErrorReq"] = 0;
// 		c += f.flushWrites();
// 		c += f.runMode();
	}
}
void USB3PixController::readModuleConfig(PixModule& mod){                                              //! Read module configuration 
	if(U3PC_DEBUG) cout << "readModuleConfig" << endl;
}
void USB3PixController::sendModuleConfig(unsigned int moduleMask){                                     //! Send module configuration 
	if(U3PC_DEBUG) cout << "sendModuleConfig" << endl;
	sendPixel(moduleMask);
	sendGlobal(moduleMask);
	board->getData();
	detectReadoutChannels();
}
void USB3PixController::sendPixel(unsigned int moduleMask){                                            //! send specif. pixel register cfg.
	if(U3PC_DEBUG) cout << "sendPixel" << endl;

	CommandBuffer c;

	for(auto &i : frontends) {
		Frontend &f = i.second;
		c += f.confMode();

		int org_vc = f["Vthin_AltCoarse"];
		int org_vf = f["Vthin_AltFine"];
		f["Vthin_AltFine"] = 255;
		f["Vthin_AltCoarse"] = 255;
		c += f.flushWrites();

		for(auto &reg : {"FDAC", "TDAC","CAP1", "CAP0", "ILEAK", "ENABLE"}) {
		  if(U3PC_DEBUG) cout << "sendPixel calls writePixelRegister for " << reg << endl;
			c += f.writePixelRegister(reg);
		}

		f["Vthin_AltFine"] = org_vf;
		f["Vthin_AltCoarse"] = org_vc;
		c += f.flushWrites();

		c += f.runMode();

		c += f.confMode(); //FIXME
		f["ReadErrorReq"] = 1;
		c += f.flushWrites();
		c += f.globalPulse(0);
		f["ReadErrorReq"] = 0;
		c += f.flushWrites();
		c += f.runMode();
	}
	board->sendCommands(c);
}
void USB3PixController::sendPixel(unsigned int moduleMask, std::string regName, bool /*allDcsIdentical*/){  //! send pixel register cfg.
  if(U3PC_DEBUG) cout << "sendPixel for reg " << regName << endl;
	CommandBuffer c;

	// USBpix3I4 code deals with FDAC and TDAC in one chunk -> act only on item 0, ignore rest
	std::string reg = regName;
	if(regName.substr(0,4)=="TDAC"){
	  if(regName!="TDAC0") return;
	  else reg = "TDAC";
	}
	else if(regName.substr(0,4)=="FDAC"){
	  if(regName!="FDAC0") return;
	  else reg = "FDAC";
	}

	for(auto &i : frontends) {
		Frontend &f = i.second;
		c += f.confMode();

		int org_vc = f["Vthin_AltCoarse"];
		int org_vf = f["Vthin_AltFine"];
		f["Vthin_AltFine"] = 255;
		f["Vthin_AltCoarse"] = 255;
		c += f.flushWrites();

		c += f.writePixelRegister(reg);

		f["Vthin_AltFine"] = org_vf;
		f["Vthin_AltCoarse"] = org_vc;
		c += f.flushWrites();

		c += f.runMode();

		c += f.confMode(); //FIXME
		f["ReadErrorReq"] = 1;
		c += f.flushWrites();
		c += f.globalPulse(0);
		f["ReadErrorReq"] = 0;
		c += f.flushWrites();
		c += f.runMode();
	}
	board->sendCommands(c);
}
void USB3PixController::sendPixel(unsigned int moduleMask, std::string regName, int DC){  //! send pixel register cfg. for specific DC
  if(U3PC_DEBUG) cout << "sendPixel for reg " << regName << " and DC " << DC << endl;
	CommandBuffer c;

	// USBpix3I4 code deals with FDAC and TDAC in one chunk -> act only on item 0, ignore rest
	std::string reg = regName;
	if(regName.substr(0,4)=="TDAC"){
	  if(regName!="TDAC0") return;
	  else reg = "TDAC";
	}
	else if(regName.substr(0,4)=="FDAC"){
	  if(regName!="FDAC0") return;
	  else reg = "FDAC";
	}

	for(auto &i : frontends) {
		Frontend &f = i.second;
		c += f.confMode();

		int org_vc = f["Vthin_AltCoarse"];
		int org_vf = f["Vthin_AltFine"];
		f["Vthin_AltFine"] = 255;
		f["Vthin_AltCoarse"] = 255;
		c += f.flushWrites();

		c += f.writePixelRegister(reg, {DC});

		f["Vthin_AltFine"] = org_vf;
		f["Vthin_AltCoarse"] = org_vc;
		c += f.flushWrites();

		c += f.runMode();

		c += f.confMode(); //FIXME
		f["ReadErrorReq"] = 1;
		c += f.flushWrites();
		c += f.globalPulse(0);
		f["ReadErrorReq"] = 0;
		c += f.flushWrites();
		c += f.runMode();
	}
	board->sendCommands(c);
}
void USB3PixController::sendGlobal(unsigned int moduleMask){                                           //! send specif. gloabal register cfg.
	if(U3PC_DEBUG) cout << "sendGlobal" << endl;
	if(U3PC_DEBUG) cout << "moduleMask " << moduleMask << endl;

	CommandBuffer c;

	for(auto &i : frontends) {
		c += i.second.confMode();
		c += i.second.writeGlobalRegisters();
		c += i.second.runMode();
	}

	board->sendCommands(c);
}

void USB3PixController::sendGlobal(unsigned int moduleMask, std::string regName){                      //! send gloabal register cfg.
	if(U3PC_DEBUG) cout << "sendGlobal" << endl;
	if(U3PC_DEBUG) cout << "regName " << regName << endl; 

	CommandBuffer c;

	for(auto &i : frontends) {
		c += i.second.confMode();
		c += i.second.writeGlobalRegisters({regName});
		c += i.second.runMode();
	}

	board->sendCommands(c);
}

//DLP: sends the charge calibration for the clusterizer
void USB3PixController::sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge){
	//if(U3PC_DEBUG) cout << "sendPixelChargeCalib" << endl;
}

void USB3PixController::setCalibrationMode(void) {
	if(U3PC_DEBUG) cout << "setCalibrationMode" << endl;
}
void USB3PixController::setConfigurationMode(void) {
	if(U3PC_DEBUG) cout << "setConfigurationMode" << endl;
}
void USB3PixController::setRunMode(void) {
	if(U3PC_DEBUG) cout << "setRunMode" << endl;
}
void USB3PixController::setFERunMode(void) {
	if(U3PC_DEBUG) cout << "setFERunMode" << endl;

	CommandBuffer c;

	for(auto &fe : frontends) {
		c += fe.second.runMode();
	}

	board->sendCommands(c);
}

void USB3PixController::setFEConfigurationMode(void) {
	if(U3PC_DEBUG) cout << "setFEConfigurationMode" << endl;

	CommandBuffer c;

	for(auto &fe : frontends) {
		c += fe.second.confMode();
	}

	board->sendCommands(c);
}

void USB3PixController::readEPROM(void) { // sends a global pulse to read the EPROM values to the FE GR
	if(U3PC_DEBUG) cout << "readEPROM" << endl;
}
void USB3PixController::burnEPROM(void) { // burns the FE GR values to the EPROM. Caution, non reversibel!!!
	if(U3PC_DEBUG) cout << "burnEPROM" << endl;
}

void USB3PixController::readGADC(int type, std::vector<int> &GADCvalues, int FEindex) {
	if(U3PC_DEBUG) cout << "readGADC" << endl;
}

std::string USB3PixController::translateScanParam(PixScan::ScanParam param, std::string gr) {
	switch(param) {
		default:
		case PixScan::NO_PAR:
			return "";
		case PixScan::IF:
			return "PrmpVbpf";
		case PixScan::GDAC:
			return "Vthin_AltFine";
		case PixScan::LATENCY:
			return "TrigLat";
		case PixScan::VCAL:
			return "PlsrDAC";
		case PixScan::DISCBIAS:
			return "DisVbn";
		case PixScan::FEI4_GR:
			return gr;
	}
}

void USB3PixController::writeScanConfig(PixScan &scn) {                                           //! Write scan parameters
	if(U3PC_DEBUG) cout << "writeScanConfig" << endl;

	CommandBuffer c;

	for(auto &i : frontends) {
		Frontend &fe = i.second;

		c += fe.confMode();

		int trigcnt = 0;
		if(scn.getConsecutiveLvl1TrigA(0)<16) {
			trigcnt = scn.getConsecutiveLvl1TrigA(0);
		} else if(scn.getConsecutiveLvl1TrigA(0)>16) {
			throw "up"; //fixme
		}

		fe["TrigCnt"] = trigcnt;
		fe["DIGHITIN_Sel"] = scn.getDigitalInjection();

		fe["TrigLat"] = scn.getLVL1Latency();
		fe["PlsrDelay"] = scn.getStrobeMCCDelay();

		if(scn.getFeVCal()!=8191){
			fe["PlsrDAC"] = scn.getFeVCal();
		}

		if(scn.getSrcTriggerType() == PixScan::FE_SELFTRIGGER) {
			fe["GateHitOr"] = 1;
		}

		if(!scn.getSourceScanFlag() && scn.getAlterFeCfg()) {
			fe["HitDiscCnfg"] = 0;
		}

		c += fe.flushWrites();
		c += fe.runMode();
	}

	board->sendCommands(c);
}

Matrix<uint8_t, 336, 80> pixelMask(int mask_steps, int offset, uint8_t disabled = 0, uint8_t enabled = 1) {
	Matrix<uint8_t, 336, 80> mask{{{{disabled}}}};

	for(int i=offset; i<336*80; i+=mask_steps) {
		int x = i/672;
		int y = i%672;

		int column = x*2;

		if(y>=336) {
			column++;
			y=672-y-1;
		}

		mask[336-y-1][column] = enabled;
	}

	return mask;
}

Matrix<uint8_t, 336, 80> crosstalkMask(const Matrix<uint8_t, 336, 80> &m, uint8_t disabled = 0, uint8_t enabled = 1) {
	Matrix<uint8_t, 336, 80> mask{{{{disabled}}}};

	for(int column=0; column < 80; column++) {
		for(int row=0; row<336; row++) {
			if(row>0 && m[row-1][column] == enabled) {
				mask[row][column] = enabled;
			}

			if(row<335 && m[row+1][column] == enabled) {
				mask[row][column] = enabled;
			}
		}
	}

	return mask;
}

struct ScanLoop {
	int loop_index;
	int loop_value;

	std::list<HitHistogrammer> hit_hist;
	std::list<TotHistogrammer> tot_hist;
	std::list<TotHistogrammer2D> tot_hist2d;
	std::list<BCIDHistogrammer> bcid_hist;
	std::list<LV1IDHistogrammer> lv1id_hist;
	std::list<LVL1Histogrammer> lvl1_hist;
};

struct Scan {
	int num_loops;
	int current_loop = 0;

	DataDecoder dec;

	int event_count = 0; //source scan
	std::atomic<bool> source_scan = {false};
	std::thread scan_thread;
	std::future<void> thread_future;

	std::list<ScanLoop> loops;
	std::unique_ptr<RawFileWriter> raw_writer;
};

struct MaskSettings {
	int mask_stages;
	int mask_steps;
	int mask_step_width;
	bool xtalk;

	std::vector<std::string> shift_mask;
};

void USB3PixController::getMaskSettings(MaskSettings &m, PixScan &scn) {
	m.mask_steps = scn.getMaskStageSteps();
	m.mask_step_width = scn.getMaskStageStepWidth();

	switch(scn.getMaskStageTotalSteps()) {
		case PixScan::STEPS_1:		m.mask_stages = 1;		break;
		case PixScan::STEPS_2:		m.mask_stages = 2;		break;
		case PixScan::STEPS_3:		m.mask_stages = 3;		break;
		case PixScan::STEPS_4:		m.mask_stages = 4;		break;
		case PixScan::STEPS_6:		m.mask_stages = 6;		break;
		case PixScan::STEPS_8:		m.mask_stages = 8;		break;
		case PixScan::STEPS_32:		m.mask_stages = 32;		break;
		case PixScan::STEPS_336:	m.mask_stages = 336;	break;
		case PixScan::STEPS_672:	m.mask_stages = 672;	break;
		case PixScan::STEPS_26880:	m.mask_stages = 26880;	break;
		default:					m.mask_stages = 1;		break;
	}

	m.xtalk = false;

	if(scn.getMaskStageTotalSteps() != PixScan::STEPS_USER) {
		static const std::vector<std::vector<std::string>> cap_mask {
			{"CAP1"}, {"CAP0"}, {"CAP1", "CAP0"}
		};

		auto mode = scn.getMaskStageMode();

		switch(mode) {
			case PixScan::SEL:			break;
			case PixScan::ENA:			m.shift_mask = {"ENABLE"}; break;
			case PixScan::SEL_ENA:		m.shift_mask = {"ENABLE"}; break;
			case PixScan::HITB:			m.shift_mask = {"HITBUS"}; break;
			case PixScan::HITB_INV:		m.shift_mask = {"INVHB"}; break;
			case PixScan::ENA_HITB:		m.shift_mask = {"ENABLE", "HITBUS"}; break;
			case PixScan::SEL_ENA_HITB:	m.shift_mask = {"ENABLE", "HITBUS"}; break;
			case PixScan::XTALK:		m.shift_mask = {"ENABLE"}; m.xtalk = true; break;  
			case PixScan::DINJ:			m.shift_mask = {"DIGINJ"}; break;
			case PixScan::STATIC:		break;
			default:					break;
		}

		if(mode == PixScan::SEL || mode == PixScan::SEL_ENA || mode == PixScan::SEL_ENA_HITB) {
			auto &i = cap_mask[scn.getChargeInjCap()];
			m.shift_mask.insert(m.shift_mask.end(), i.begin(), i.end());

			CommandBuffer c;

			for(auto & i : frontends) {
				i.second.setPixelRegister("CAP0", 0);
				i.second.setPixelRegister("CAP1", 0);

				c += i.second.confMode();
				c += i.second.writePixelRegister("CAP0");
				c += i.second.writePixelRegister("CAP1");
				c += i.second.runMode();
			}

			board->sendCommands(c);
		}

		if(m.xtalk) {
			CommandBuffer c;

			for(auto & i : frontends) {
				i.second.setPixelRegister("CAP0", 0);
				i.second.setPixelRegister("CAP1", 0);

				c += i.second.confMode();
				c += i.second.writePixelRegister("CAP0");
				c += i.second.writePixelRegister("CAP1");
				c += i.second.runMode();
			}

			board->sendCommands(c);
		}

	} else {
		m.mask_steps = 1;
	}
}

void USB3PixController::startScan(PixScan *scn) {                                                 //! Start a scan
	if(U3PC_DEBUG) cout << "startScan" << endl;

	writeScanConfig(*scn);

	current_scan = std::unique_ptr<Scan>(new Scan);

	if(scn->getSrcTriggerType() == PixScan::STROBE_SCAN) {
		current_scan->source_scan = false;
		strobeScan(scn);
	} else {
		current_scan->source_scan = true;
		initLoop(scn, current_scan->dec);
		sourceScan(scn->getRepetitions(), scn->getSrcTriggerType(), scn->getSrcCountType(), scn->getStrobeLVL1Delay());

		//std::packaged_task<void()> tsk(std::bind(&USB3PixController::sourceScan, this, scn->getRepetitions(), scn->getSrcTriggerType(), scn->getSrcCountType(), scn->getStrobeLVL1Delay()));

		//current_scan->thread_future = tsk.get_future();
		//current_scan->scan_thread = std::thread(std::move(tsk));
	}
}

void USB3PixController::initLoop(PixScan *scn, DataDecoder &dec) {
	current_scan->loops.emplace_back();
	ScanLoop &loop = current_scan->loops.back();

	dec.clearHistogrammers();

	if(!scn->getSourceRawFile().empty()) {
		if(!current_scan->raw_writer) {
			std::vector<int> channels;

			for(auto &i : frontends) {
				if(fe_to_ch[i.first]>=0) { // ignore undetected frontends
					channels.push_back(fe_to_ch[i.first]);
				}
			}

			current_scan->raw_writer = std::unique_ptr<RawFileWriter>(new RawFileWriter(channels, scn->getSourceRawFile()));
		}
		dec.addHistogrammer(current_scan->raw_writer.get());
	}

	for(auto id : fe_ids) {
		Frontend &fe = frontends.at(id);

		if(scn->getHistogramFilled(PixScan::OCCUPANCY)) {
			loop.hit_hist.emplace_back(fe_to_ch[id]);
			dec.addHistogrammer(&loop.hit_hist.back());
		}

		if(scn->getHistogramFilled(PixScan::TOT0) || scn->getHistogramFilled(PixScan::TOT_MEAN) ||
			scn->getHistogramFilled(PixScan::TOT_SIGMA) || scn->getHistogramFilled(PixScan::TOT)) {

			loop.tot_hist2d.emplace_back(fe_to_ch[id], fe["HitDiscCnfg"] == 0 ? 14 : 15);
			dec.addHistogrammer(&loop.tot_hist2d.back());
		}

		if(scn->getHistogramFilled(PixScan::BCID)) {
			loop.bcid_hist.emplace_back(fe_to_ch[id]);
			dec.addHistogrammer(&loop.bcid_hist.back());
		}

		if(scn->getHistogramFilled(PixScan::LV1ID)) {
			loop.lv1id_hist.emplace_back(fe_to_ch[id]);
			dec.addHistogrammer(&loop.lv1id_hist.back());
		}

		if(scn->getHistogramFilled(PixScan::LVL1)) {
			loop.lvl1_hist.emplace_back(fe_to_ch[id]);
			dec.addHistogrammer(&loop.lvl1_hist.back());
		}
	}
}

void USB3PixController::strobeScan(PixScan *scn) {
	int lvl1_delay = scn->getStrobeLVL1Delay();

	MaskSettings m;
	getMaskSettings(m, *scn);

	std::vector<int> loop_values{0};

	if(scn->getLoopActive(0) && scn->getDspProcessing(0)) {
		const std::vector<float> &val = scn->getLoopVarValues(0);
		loop_values = std::vector<int>(val.begin(), val.end());
	}

	int repeat = scn->getRepetitions();
	Command loop_cmd = CommandGenerator::cal() + CommandGenerator::zeros(lvl1_delay - CommandGenerator::cal().size ) + CommandGenerator::lv1() + CommandGenerator::zeros(336.0 / m.mask_steps * 25 + 700);

	current_scan->num_loops = loop_values.size();

	Readout r([&](void) {
		current_scan->dec.decode(board->getData());
	}, m_readoutInterval);

	CommandBuffer c;

	finished = false;
	running = true;

	board->getData();

	try {
		for(int loop_val : loop_values) {
			if(U3PC_DEBUG) cout << "loop " << loop_val << endl;

			initLoop(scn, current_scan->dec);

			for(auto id : fe_ids) {
				Frontend &fe = frontends.at(id);

				c += fe.confMode();
				fe["Colpr_Mode"] = 0;

				if(scn->getLoopActive(0) && scn->getDspProcessing(0)) {
					std::string gr = translateScanParam(scn->getLoopParam(0), scn->getLoopFEI4GR(0));
					if(!gr.empty()) {
						if(U3PC_DEBUG) cout << "setting GR " << gr << " to value " << loop_val << endl; 
						fe[gr] = loop_val;
					}

				}

				c += fe.flushWrites();
				c += fe.runMode();
			}

			board->sendCommands(c);

			r.startReadout();

			scanLoop(loop_cmd, repeat, m);

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			r.pauseReadout();

			current_scan->current_loop++;

			if(cancel_scan) {
				cancel_scan = false;
				break;
			}
		}

		r.stopReadout();

		board->printRxStatus();
		if(U3PC_DEBUG) cout << current_scan->dec.size*4.0/(1024*1024) << " MB" << endl;

	} catch (const std::exception &e) {
		m_errBuff += std::string("Exception: ") + e.what() + '\n';
	}

	current_scan->raw_writer.reset(); // flush raw data

	if(U3PC_DEBUG) cout << "invalid DR: " << current_scan->dec.invalid_dr << endl;
	
	finished = true;
}

void USB3PixController::sourceScan(int max_event, int trigger_type, int count_type, int lvl1_delay) {
	Readout r([&](void) {
		current_scan->dec.decode(board->getData());
	}, m_readoutInterval);

	CommandBuffer c;

	finished = false;
	running = true;

	board->getData();

	r.startReadout();

	if(trigger_type == PixScan::FE_SELFTRIGGER) {
		for(auto &i : frontends) {
			c += i.second.confMode();
			i.second["GateHitOr"] = 1;
			c += i.second.flushWrites();
			c += i.second.runMode();
		}
		board->sendCommands(c);
	} else {
		Command cmd = CommandGenerator::zeros(lvl1_delay+3) + CommandGenerator::lv1() + CommandGenerator::zeros(500);
		board->setCommand(cmd);

		int trigger_mode;

		switch(trigger_type) {
			case PixScan::TLU_SIMPLE:
				trigger_mode = TLU::SIMPLE_HANDSHAKE;
				break;

			case PixScan::TLU_DATA_HANDSHAKE:
				trigger_mode = TLU::DATA_HANDSHAKE;
				break;

			case PixScan::EXT_TRG:
				trigger_mode = TLU::EXTERNAL;
				break;

			default: //fixme
				trigger_mode = TLU::NO_HANDSHAKE;
				break;
		}

		board->enableTrigger(trigger_mode);
	}

	auto start = std::chrono::steady_clock::now();
	auto t0 = start;
	uint32_t trigger_count = board->getTriggerCounter(); //fixme
	if(U3PC_DEBUG) cout << "trigger count " << trigger_count << endl;
	uint32_t prev_trigger_count = trigger_count;
	uint32_t prev_event_count = 0;

	while(current_scan->event_count < max_event) {
		switch(count_type) {
			default:
			case PixScan::COUNT_TRIGGER:
				current_scan->event_count = board->getTriggerCounter() - trigger_count;
				break;
			case PixScan::COUNT_DH:
				current_scan->event_count = current_scan->dec.data_headers;
				break;
			case PixScan::COUNT_DR:
				current_scan->event_count = current_scan->dec.data_records;
				break;
			case PixScan::COUNT_SECS:
				current_scan->event_count = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
				break;
		}

		uint32_t tc = board->getTriggerCounter();
		auto t1 = std::chrono::steady_clock::now();
		std::chrono::duration<double> d = t1 - t0;
		trigger_rate = (tc - prev_trigger_count)/d.count();
		event_rate = (current_scan->event_count - prev_event_count)/d.count();
		t0 = t1;
		prev_trigger_count = tc;
		prev_event_count = current_scan->event_count;

		if(cancel_scan) {
			cancel_scan = false;
			break;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if(trigger_type == PixScan::FE_SELFTRIGGER) {
		for(auto &i : frontends) {
			c += i.second.confMode();
			i.second["GateHitOr"] = 0;
			c += i.second.flushWrites();
			c += i.second.runMode();
		}
		board->sendCommands(c);
	} else {
		board->disableTrigger();
	}

	r.stopReadout();

	current_scan->raw_writer.reset(); // flush raw data

	cout << "invalid DR: " << current_scan->dec.invalid_dr << endl;
	
	finished = true;
}

void USB3PixController::scanLoop(Command &loop_cmd, int repeat, MaskSettings &m) {
	CommandBuffer c;

	for(int mask_step = 0; mask_step < m.mask_steps; mask_step++) {
		auto mask = pixelMask(m.mask_stages, mask_step*m.mask_step_width);

		for(auto &i : frontends) {
			c += i.second.confMode();

			for(auto &reg : m.shift_mask) {
				i.second.setPixelRegister(reg, mask);
				c += i.second.writePixelRegister(reg);
			}

			if(m.xtalk) {
				auto xmask = crosstalkMask(mask);
				i.second.setPixelRegister("CAP0", xmask); //fixme
				i.second.setPixelRegister("CAP1", xmask);

				c += i.second.writePixelRegister("CAP0");
				c += i.second.writePixelRegister("CAP1");
			}

			c += i.second.runMode();
		}

		board->sendCommands(c);

		std::this_thread::sleep_for(std::chrono::milliseconds(50)); // small delay between mask writing and scan

		for(int dc = 0; dc < 40; dc++) {
			for(auto &i : frontends) {
				Frontend &fe = i.second;

				c += fe.confMode();
				fe["Colpr_Addr"] = dc;
				c += fe.flushWrites();
				c += fe.runMode();
			}
			board->sendCommands(c);

			board->sendCommands(loop_cmd, repeat);
		}
	}
}

void USB3PixController::finalizeScan(void) {                                                            //! finish undone issues after scan
	if(U3PC_DEBUG) cout << "finalizeScan" << endl;

	if(current_scan && current_scan->source_scan && current_scan->scan_thread.joinable()) {
		current_scan->scan_thread.join();
		current_scan->thread_future.get();
	}
}

void USB3PixController::measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval) {  //! measure event and trigger rate - not a real occ./ToT-scan!
	if(U3PC_DEBUG) cout << "measureEvtTrgRate" << endl;
}

bool USB3PixController::fitHistos(void) {                                                                                 //! fit histos
	if(U3PC_DEBUG) cout << "fitHistos" << endl;
	return false;
}

bool USB3PixController::getErrorHistos(unsigned int dsp, Histo* &his) {                                               //! get error arrays 
	if(U3PC_DEBUG) cout << "getErrorHistos" << endl;
	return false;
}

void USB3PixController::getHisto(HistoType type, unsigned int mod, unsigned int slv, std::vector< std::vector<Histo*> >& his) { //! Read an histogram
	if(U3PC_DEBUG) cout << "getHisto" << endl;

	for(int i=0;i<32;i++) {
		his.emplace_back();
	}

	//his[mod].clear();
	PixModule *pm = m_modGroup.module(mod);
	if(!pm) {
		return;
	}

	static const std::map<HistoType, std::pair<std::string, std::string>> hist_types {
		{OCCUPANCY, {"Occup_", "Occupancy"}},
		{TOT_MEAN, {"ToTMean_", "ToTMean"}},
		{TOT_SIGMA, {"ToTSigma_", "ToTSigma"}},
		{LVL1, {"LVL1_", "LVL1"}},
		{LV1ID, {"LV1ID_", "LV1ID"}},
		{BCID, {"BCID_", "BCID"}},

		{TOT, {"ToT_", "ToT"}},
		{TOT0, {"ToT0_", "ToT0"}},
		{TOT1, {"ToT1_", "ToT1"}},
		{TOT2, {"ToT2_", "ToT2"}},
		{TOT3, {"ToT3_", "ToT3"}},
		{TOT4, {"ToT4_", "ToT4"}},
		{TOT5, {"ToT5_", "ToT5"}},
		{TOT6, {"ToT6_", "ToT6"}},
		{TOT7, {"ToT7_", "ToT7"}},
		{TOT8, {"ToT8_", "ToT8"}},
		{TOT9, {"ToT9_", "ToT9"}},
		{TOT10, {"ToT10_", "ToT10"}},
		{TOT11, {"ToT11_", "ToT11"}},
		{TOT12, {"ToT12_", "ToT12"}},
		{TOT13, {"ToT13_", "ToT13"}},
		{TOT14, {"ToT14_", "ToT14"}},
		{TOT15, {"ToT15_", "ToT15"}},

		{TOTAVERAGE, {"ToTAvg_", "ToTAverage"}},
		{CLUSTER_TOT, {"ClToT_", "Cluster ToT"}},
		{CLUSTER_CHARGE, {"ClCharge_", "Cluster Charge"}},
		{CLUSTER_POSITION, {"ClPosition_", "Cluster Position"}},
		{CLUSTER_SIZE, {"ClSize_", "Cluster size"}},
		{CLSIZE_TOT, {"ClSizToT_", "Cluster size vs ToT"}},
		{CLSIZE_CHARGE, {"ClSizCharge_", "Cluster size vs Charge"}},
		{HITOCC, {"HitOcc_", "Hit Occupancy (of cluster seeds)"}},
		{SEED_TOT, {"SdToT_", "Cluster-seed ToT"}},
		{SEED_LVL1, {"SdLVL1_", "Cluster-seed LVL1"}},
		{NSEEDS, {"Nseeds", "Number of cluster seeds per trigger"}}
	};

	unsigned int ncolm = pm->nColsMod();
	unsigned int nrowm = pm->nRowsMod();

	for(auto &loop : current_scan->loops) {
		std::string name = hist_types.at(type).first + std::to_string(mod)+"_"+std::to_string(loop.loop_index);
		std::string title = hist_types.at(type).second + " mod "+std::to_string(mod)+" bin "+std::to_string(loop.loop_index);

		Histo *h;

		if (type == OCCUPANCY || type == HITOCC) {
			h = new Histo(name, title, (current_scan->source_scan?8:1), ncolm, -0.5, -0.5+(double)ncolm, nrowm, -0.5, -0.5+(double)nrowm);
		}else if(type == TOTAVERAGE) {
			h = new Histo(name, title, nrowm*ncolm, -0.5, -0.5+(double)(nrowm*ncolm));
		}else if(type == LVL1 || type == SEED_LVL1 || type == NSEEDS) {
			h = new Histo(name, title, 16, -0.5, 15.5);
		}else if(type == LV1ID) {
			h = new Histo(name, title, 4096, -0.5, 4095.5);
		}else if(type == BCID) {
			h = new Histo(name, title, 8192, -0.5, 8191.5);
		}else if(type == TOT || type == SEED_TOT) {
			h = new Histo(name, title, 16, -0.5, 15.5);
		} /*else if(type == CLUSTER_SIZE) {
			h = new Histo(name, title, __MAXCLUSTERHITSBINS, -0.5, __MAXCLUSTERHITSBINS-0.5);
		}else if(type == CLUSTER_TOT) {
			h = new Histo(name, title, __MAXTOTBINS, -0.5, __MAXTOTBINS-0.5);
		}else if(type == CLUSTER_CHARGE) {
			h = new Histo(name, title, __MAXCHARGEBINS, -0.5, __MAXCHARGEBINS-0.5);
		}else if(type == CLUSTER_POSITION) {
			h = new Histo(name, title, 8, __MAXPOSXBINS, -0.5, __MAXPOSXBINS-0.5, __MAXPOSYBINS, -0.5, __MAXPOSYBINS-0.5);
		}else if(type == CLSIZE_TOT) {
			h = new Histo(name, title, 8, __MAXTOTBINS, -0.5, __MAXTOTBINS-0.5, __MAXCLUSTERHITSBINS, -0.5, __MAXCLUSTERHITSBINS-0.5);
		}else if(type == CLSIZE_CHARGE) {
			h = new Histo(name, title, 8, __MAXCHARGEBINS, -0.5, __MAXCHARGEBINS-0.5, __MAXCLUSTERHITSBINS, -0.5, __MAXCLUSTERHITSBINS-0.5);
		} */else {
		  h = new Histo(name, title, 8, ncolm, -0.5, -0.5+(double)ncolm, nrowm, -0.5, -0.5+(double)nrowm);
		}

		switch(type) {
			case OCCUPANCY: {
				int fe = 0;
				for(auto &hist : loop.hit_hist) {

					for(int x = 0; x < hist.columns; x++) {
						for(int y = 0; y < hist.rows; y++) {
						        int colm = pm->iColMod(fe, x);
							int rowm = pm->iRowMod(fe, y);
							if(hist[y][x]>255 && !current_scan->source_scan) { // cap strobe scan histos to 255 to get same behaviour as usbpix 2
								h->set(colm, rowm, 255);
							} else {
								h->set(colm, rowm, hist[y][x]);
							}
						}
					}
					fe++;
				}
				break;
			}

			case LV1ID: {
				for(auto & hist : loop.lv1id_hist) {
					for(int i=0; i<hist.size; i++) {
						h->set(i, hist[i]);
					}
				}
				break;
			}

			case BCID: {
				for(auto & hist : loop.bcid_hist) {
					for(int i=0; i<hist.size; i++) {
						h->set(i, hist[i]);
					}
				}
				break;
			}

			case LVL1: {
				size_t lvl1His[16] = {0};

				for(auto & hist : loop.lvl1_hist) {
					for(int i=0; i<hist.size; i++) {
						lvl1His[i] += hist[i];
					}
				}

				for(int i=0; i<16; i++) {
					h->set(i, lvl1His[i]);
				}
			}
		}

		if((type>=TOT0 && type<=TOT15) || type == TOT_MEAN || type == TOT_SIGMA || type == TOT) {
			size_t totHis[16] = {0};

			int fe = 0;
			for(auto & hist : loop.tot_hist2d) {

				for(int column=0; column<hist.columns; column++) {
					for(int row=0; row<hist.rows; row++) {
					        int colm = pm->iColMod(fe, column);
						int rowm = pm->iRowMod(fe, row);
						int TOTsumsqr = 0;
						int TOTsum = 0;
						int hitsum = 0;
						double TOTsqravg = 0;
						double TOTavg = 0;
						double TOTsigma = 0;

						for(int tot=0;tot<16;tot++) {
							int nr_hits = hist[row][column][tot];
							totHis[tot] += nr_hits;

							TOTsumsqr += (nr_hits*tot*tot);
							TOTsum += nr_hits*tot;
							hitsum += nr_hits;

							if(type - TOT0 == tot) {
								h->set(colm, rowm, hist[row][column][type-TOT0]);
							}
						}
						if(hitsum > 1) {
							TOTsqravg = (double)TOTsumsqr/(double)hitsum;
							TOTavg = (double)TOTsum/(double)hitsum;
							TOTsigma = ((double)(TOTsqravg-TOTavg*TOTavg)/(double)(hitsum-1));
							if(TOTsigma>=0.) {
								TOTsigma = sqrt(TOTsigma);
							 } else {
							 	TOTsigma = 0.;
							 }
						} else {
							TOTsqravg = 0;
							TOTavg = 0;
							TOTsigma = 0;
						}

						if(type == TOT_MEAN) {
							h->set(colm, rowm, TOTavg);
						}

						if(type == TOT_SIGMA) {
							h->set(colm, rowm, TOTsigma);
						}
					}
				}
				fe++;
			}

			if(type == TOT) {
				for(int tot = 0; tot < 16; tot++) {
					h->set(tot, totHis[tot]);
				}
			}
		}

		his[mod].push_back(h);
	}
}

void USB3PixController::getFitResults(HistoType type, unsigned int mod, unsigned int slv, std::vector< Histo * > &thr, std::vector< Histo * > &noise, std::vector< Histo * > &chi2){}                                                  //! Read a Fit from Dsp

bool USB3PixController::moduleActive(int nmod) {                    //! true if a module is active during scan or datataking
	if(U3PC_DEBUG) cout << "moduleActive " << nmod << endl;
	return true;
}

void USB3PixController::writeRunConfig(PixRunConfig &cfg) {         //! Get the run configuration parameters from PixModuleGroup
	if(U3PC_DEBUG) cout << "writeRunConfig" << endl;
}

void USB3PixController::startRun(int ntrig) {                   //! Start a run
	if(U3PC_DEBUG) cout << "startRun" << endl;
}

void USB3PixController::stopRun(void) {                                 //! Terminates a run
	if(U3PC_DEBUG) cout << "stopRun" << endl;
}

void USB3PixController::pauseRun(void) {                                 //! Terminates a run
	if(U3PC_DEBUG) cout << "pauseRun" << endl;
}

void USB3PixController::resumeRun(void) {
	if(U3PC_DEBUG) cout << "resumeRun" << endl;
}

int USB3PixController::runStatus(void) {                                //! Check the status of the run
	if(U3PC_DEBUG) cout << "runStatus" << endl;

	if(running && !finished) {
		if(U3PC_DEBUG) cout << 1 << endl;
		return 1;
	} else if(finished) {
		running = false;
		finished = false;

		if(U3PC_DEBUG) cout << 2 << endl;
		return 2;
	}

	if(U3PC_DEBUG) cout << 0 << endl;
	return 0;
}

void USB3PixController::stopScan(void) {                                //! Aborts a running scan
	if(U3PC_DEBUG) cout << "stopScan" << endl;

	if(!finished) {
		cancel_scan = true;
	}
}

int USB3PixController::nTrigger(void) {              //! Returns the number of trigger processed so far
	if(U3PC_DEBUG) cout << "nTrigger" << endl;

	if(current_scan && current_scan->source_scan) {
		return current_scan->event_count ;
	} else {
		if(current_scan) {
			return current_scan->current_loop;
		} else {
			return 0;
		}
	}
}

// new function for MonLeak and HitBusScaler
void USB3PixController::shiftPixMask(int mask, int cap, int steps){
	if(U3PC_DEBUG) cout << "shiftPixMask" << endl;
}

int  USB3PixController::readHitBusScaler(int mod, int ife, PixScan* scn){
	if(U3PC_DEBUG) cout << "readHitBusScaler" << endl;
	return 0;
}

bool USB3PixController::checkRxState(rxTypes type){
	if(U3PC_DEBUG) cout << "checkRxState" << endl;
	return false;
}

void USB3PixController::getServiceRecords(std::string &txt, std::vector<int> &srvCounts){
	if(U3PC_DEBUG) cout << "getServiceRecords" << endl;
}

void USB3PixController::setAuxClkDiv(int div){
	if(U3PC_DEBUG) cout << "setAuxClkDiv" << endl;
}

void USB3PixController::setIrefPads(int bits){
	if(U3PC_DEBUG) cout << "setIrefPads" << endl;
}

void USB3PixController::setIOMUXin(int bits){
	if(U3PC_DEBUG) cout << "setIOMUXin" << endl;
}

void USB3PixController::sendGlobalPulse(int length){
	if(U3PC_DEBUG) cout << "sendGlobalPulse" << endl;
}

bool USB3PixController::testGlobalRegister(int module, std::vector<int> &data_in,
  std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg, int feIndex){
	if(U3PC_DEBUG) cout << "testGlobalRegister" << endl;
	if(U3PC_DEBUG) cout << feIndex << endl;

	CommandBuffer c;

	board->getData();

	Frontend &fe = frontends.at(fe_ids.at(feIndex));

	c += fe.confMode();

	if(sendCfg) {
		if(U3PC_DEBUG) cout << "sendCfg" << endl;
		c += fe.writeGlobalRegisters();
	}

	c += fe.readGlobalRegisters();

	c += fe.runMode();

	board->sendCommands(c);

	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	RegisterDecoder reg;
	DataDecoder dec{&reg};

	dec.decode(board->getData());

	Frontend testfe(fe.address, fe.flavour);

	for(auto i : reg.getRegisters()) {
		testfe[i.first] = i.second;
	}

	bool retval = true;

	for(auto const &i : testfe) {
		data_in.push_back(fe[i]);
		data_out.push_back(testfe[i]);
		label.push_back(i);

		if(fe[i]!=testfe[i]) {
			retval = false;
		}
	}

	return retval;
}

bool USB3PixController::testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out, 
			 bool ignoreDCsOff, int DC, bool sendCfg, int feIndex, bool bypass){
	if(U3PC_DEBUG) cout << "testPixelRegister " << regName << endl;
	if(U3PC_DEBUG) cout << bypass << endl;

	int dcmin = 0;
	int dcmax = 39;

	if(DC>=0 && DC<=39) {
		dcmin = dcmax = DC;
	}

	static const std::map<std::string, std::pair<std::string, int>> pixreg = {
		{"ENABLE", {"ENABLE", 0}},
		{"TDAC0", {"TDAC", 0}},
		{"TDAC1", {"TDAC", 1}},
		{"TDAC2", {"TDAC", 2}},
		{"TDAC3", {"TDAC", 3}},
		{"TDAC4", {"TDAC", 4}},
		{"CAP0", {"CAP0", 0}},
		{"CAP1", {"CAP1", 0}},
		{"ILEAK", {"ILEAK", 0}},
		{"FDAC0", {"FDAC", 0}},
		{"FDAC1", {"FDAC", 1}},
		{"FDAC2", {"FDAC", 2}},
		{"FDAC3", {"FDAC", 3}}
	};

	CommandBuffer c;

	board->getData();

	Frontend &fe = frontends.at(fe_ids.at(feIndex));
	std::string name = pixreg.at(regName).first;
	int bitnr = pixreg.at(regName).second;

	c += fe.confMode();

	if(sendCfg) {
		c += fe.writePixelRegister(name);
	}

	board->sendCommands(c);

	bool dcmask[40] = {false};

	if(ignoreDCsOff) {
		int disable_column = fe["DisableColumnCnfg0"];
		for(int i=0; i<16; i++) {
			dcmask[i] = disable_column & 1;
			disable_column >>= 1;
		}

		disable_column = fe["DisableColumnCnfg1"];
		for(int i=16; i<32; i++) {
			dcmask[i] = disable_column & 1;
			disable_column >>= 1;
		}

		disable_column = fe["DisableColumnCnfg2"];
		for(int i=32; i<40; i++) {
			dcmask[i] = disable_column & 1;
			disable_column >>= 1;
		}
	}

	bool retval = true;

	for(int dc = dcmin; dc <= dcmax; dc++) {
		if(dcmask[dc]) {
			for(int i = 0; i<21; i++) {
				data_in.push_back(0);
				data_out.push_back(0);
			}
		} else {
			c += fe.readPixelRegister(name, bitnr, {dc});
			board->sendCommands(c);

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			PixelRegisterDecoder reg;
			DataDecoder dec{&reg};

			dec.decode(board->getData());

			auto fereg = fe.getPixelRegisterDC(name, bitnr, dc);
			auto registers = reg.getRegisters();

			auto it = registers.cbegin();
			for(int i=0; i<42; i+=2) {
				uint32_t wValue = (fereg[i] << 16) | fereg[i+1];
				uint32_t rValue = 0;

				auto reg1 = find_if(it, registers.cend(), [=](const pair<uint16_t, uint16_t> &e){ return e.first == i*16+15; });

				if(reg1 != registers.cend()) {
					rValue |= reg1->second << 16;
					it = reg1;
				}

				auto reg2 = find_if(it, registers.cend(), [=](const pair<uint16_t, uint16_t> &e){ return e.first == (i+1)*16+15; });

				if(reg2 != registers.cend()) {
					rValue |= reg2->second;
					it = reg2;
				}

				data_in.push_back(wValue);
				data_out.push_back(rValue);

				if(rValue != wValue || reg1 == registers.cend() || reg2 == registers.cend()) {
					retval = false;
				}
			}
		}
	}

	c += fe.runMode();
	board->sendCommands(c);

	return retval;
}

bool USB3PixController::testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp, bool shift_only, 
		     bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex){
	if(U3PC_DEBUG) cout << "testScanChain" << endl;

	return false;
}

void USB3PixController::hwInfo(std::string &txt) {
  txt = "";
  if(m_dev!=0){
    txt  += "Board name:  " + string(m_dev->GetName()) + "\n";
    stringstream a;
    a << m_dev->GetId();
    txt += "Board ID: " + a.str() + "\n";
    stringstream b;
    b << m_dev->GetFWVersion();
    txt += "uC Firmware: " + b.str() + "\n";
  }
  return;
}

bool USB3PixController::getGenericBuffer(const char *type, std::string &textBuf){
	if(U3PC_DEBUG) cout << "getGenericBuffer: " << type << endl;
	textBuf = "";

	if(std::string(type) == "err") {
		if(m_errBuff.empty()) return false;
		textBuf = m_errBuff;
		m_errBuff = "";
		return true;
	} else if(std::string(type) == "info") {
		if(m_infoBuff.empty()) return false;
		textBuf = m_infoBuff;
		m_infoBuff = "";
		return true;
	} else {
		return false;
	}
}

// added from USBpix's source scan mode - leave empty if source scan mode is not supported
bool USB3PixController::getSRAMReadoutReady(void) {
	if(U3PC_DEBUG) cout << "getSRAMReadoutReady" << endl;
	return false;
}

bool USB3PixController::getSRAMFull(void) {
	if(U3PC_DEBUG) cout << "getSRAMFull" << endl;
	return false;
}

bool USB3PixController::getMeasurementPause(void) {
	if(U3PC_DEBUG) cout << "getMeasurementPause" << endl;
	return false;
}

bool USB3PixController::getMeasurementRunning(void) {
	if(U3PC_DEBUG) cout << "getMeasurementRunning" << endl;
	//return false;
	return running && !finished;
}

int  USB3PixController::getSRAMFillLevel(void) {
	if(U3PC_DEBUG) cout << "getSRAMFillLevel" << endl;
	return 0;
}

bool USB3PixController::getTluVeto(void) {
	if(U3PC_DEBUG) cout << "getTluVeto" << endl;
	return false;
}

int  USB3PixController::getTriggerRate(void) {
	if(U3PC_DEBUG) cout << "getTriggerRate" << endl;
	return trigger_rate;
}

int  USB3PixController::getEventRate(void) {
	if(U3PC_DEBUG) cout << "getEventRate" << endl;
	return event_rate;
}

bool USB3PixController::getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram){
	if(U3PC_DEBUG) cout << "getSourceScanData" << endl;
	return false;
}

void USB3PixController::configInit(void) {   //! Init configuration structure
	m_conf = new Config("USB3PixController");
	Config &conf = *m_conf;

	conf.addGroup("general");

	std::map<std::string, int> cmd_seq = {
		{"Positive Edge", CmdSeq::POSITIVE_EDGE},
		{"Negative Edge", CmdSeq::NEGATIVE_EDGE},
		{"Manchester (IEEE)", CmdSeq::MANCHESTER_IEEE},
		{"Manchester (Thomas)", CmdSeq::MANCHESTER_THOMAS}
	};

	conf["general"].addInt("BoardID", m_boardID, -1, "ID of board to use - leave at -1 to take any", true);
	conf["general"].addList("Output Mode", m_outputMode, CmdSeq::POSITIVE_EDGE, cmd_seq, "Output Mode", true);
	conf["general"].addInt("ReadoutInterval", m_readoutInterval, 5, "Polling interval (ms)", true);
	std::map<std::string, int> card_flavours = {
		{"MIO3+SCA", 0},
		{"MIO3+BIC", 1},
		{"MIO3+GPAC", 2},
		{"MMC3", 10}
	};
	conf["general"].addList("AdapterCardFlavor", m_cardFlavour, 10, card_flavours, "Type of USBpix3 board used", false);
	conf["general"].addString("FirmwareFile", m_FPGA_filename, "", "FPGA firmware file name (not used if empty)", true, 1, "bit");
	conf.reset();
}

void USB3PixController::detectReadoutChannels(void) {
	CommandBuffer c;

	std::fill(ch_to_fe.begin(), ch_to_fe.end(), -2);
	std::fill(fe_to_ch.begin(), fe_to_ch.end(), -2);

	for(auto &i : frontends) {
		int id = i.first;
		Frontend &fe = i.second;
		int channel = -1;

		DataDecoder dec;
		list<RegisterDecoder> reg; // a vector<> would invalidate all references after each reallocation

		for(int i=0; i<NUM_CHANNELS; i++) {
			reg.emplace_back(i);
			dec.addHistogrammer(&reg.back());
		}

		c += fe.confMode();
		c += fe.readGlobalRegisters({"Chip_SN"});
		c += fe.runMode();
		board->sendCommands(c);

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		dec.decode(board->getData());

		for(auto &r : reg) {
			auto registers = r.getRegisters();

			if(registers.size() == 1 && channel == -1) {
				channel = r.channel;
				continue;
			}

			if(registers.size() > 1 || (registers.size() > 0 && channel != -1)) {
				channel = -2;
				break;
			}

		}

		if(channel==-1) {
			if(U3PC_DEBUG) cout << "could not detect FE" << id << endl;
			m_errBuff += "could not detect FE "+std::to_string(id)+'\n';
		} else if(channel == -2) {
			if(U3PC_DEBUG) cout << "error detecting FE" << id << endl;
			m_errBuff += "error detecting FE "+std::to_string(id)+'\n';
		} else {
			ch_to_fe[channel] = id;
			fe_to_ch[id] = channel;

			if(U3PC_DEBUG) cout << "found FE" << id << " with address " << fe.address << " at channel " << channel << endl;
		}
	}
}

}
void USB3PixController::updateDeviceHandle(){
  if(m_dev!=0){
    if(U3PC_DEBUG) cout <<"INFO: USB3PixController::updateDeviceHandle is calling GetUSBDevice" << endl;
    void * tempHandle = GetUSBDevice(m_boardID);
    if(tempHandle!=0){
      m_dev->SetDeviceHandle(tempHandle);
      if(U3PC_DEBUG) cout <<"INFO: USB3PixController::updateDeviceHandle found "<< m_dev->GetName() << " with ID " << m_dev->GetId() <<endl;
      m_boardIDRB = m_dev->GetId();
    } else{
      if(U3PC_DEBUG) cout<<"WARNING: no board handle found for board ID " << m_boardID << endl;
      delete m_dev; m_dev=0;
      std::stringstream a;
      a << m_boardID;
      throw std::runtime_error("WARNING: no board handle found for board ID "+a.str());
    }  
  }
}
