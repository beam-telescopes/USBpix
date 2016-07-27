#ifndef _PIXLIB_USB3PIXCONTROLLER
#define _PIXLIB_USB3PIXCONTROLLER

#include <memory>
#include <vector>
#include <map>
#include <array>
#include <atomic>

#include "PixController.h"
#include "PixScan.h"

class Board;
class DataDecoder;
class SiUSBDevice;
class Frontend;
class Command;
class CommandBuffer;

namespace PixLib {

struct Scan;
struct MaskSettings;

class USB3PixController : public PixController {
	public:
		USB3PixController(PixModuleGroup &modGrp, DBInquire *dbInquire);     //! Constructor
		USB3PixController(PixModuleGroup &modGrp, int cardFlavour=0);        //! Constructor
		~USB3PixController(void);                                            //! Destructor

		void initHW(void);                                          //! Hardware (re)init
		void testHW(void);                                          //! Hardware test

		void sendCommand(int command, int moduleMask);                                    //! Send command from PMG list
		void sendCommand(Bits commands, int moduleMask);                                    //! Send command from pattern

		void writeModuleConfig(PixModule& mod);                                             //! Write module configuration 
		void readModuleConfig(PixModule& mod);                                              //! Read module configuration 
		void sendModuleConfig(unsigned int moduleMask);                                     //! Send module configuration 
		void sendPixel(unsigned int moduleMask);                                            //! send specif. pixel register cfg.
		void sendGlobal(unsigned int moduleMask);                                           //! send specif. gloabal register cfg.
		void sendPixel(unsigned int moduleMask, std::string regName, bool allDcsIdentical=false);  //! send pixel register cfg.
		void sendPixel(unsigned int moduleMask, std::string regName, int DC);  //! send pixel register cfg. for specific DC
		void sendGlobal(unsigned int moduleMask, std::string regName);                      //! send gloabal register cfg.

		//DLP: sends the charge calibration for the clusterizer
		void sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge);

		void setCalibrationMode(void);
		void setConfigurationMode(void);
		void setRunMode(void);
		void setFERunMode(void);
		void setFEConfigurationMode(void);

		void readEPROM(void); // sends a global pulse to read the EPROM values to the FE GR
		void burnEPROM(void); // burns the FE GR values to the EPROM. Caution, non reversibel!!!

		void readGADC(int type, std::vector<int> &GADCvalues, int FEindex);

		void writeScanConfig(PixScan &scn);                                           //! Write scan parameters
		void startScan(PixScan *scn);                                                 //! Start a scan
		void finalizeScan(void);                                                            //! finish undone issues after scan
		void measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval);  //! measure event and trigger rate - not a real occ./ToT-scan!

		bool fitHistos(void);                                                                                 //! fit histos
		bool getErrorHistos(unsigned int dsp, Histo* &his);                                               //! get error arrays 

		void getHisto(HistoType type, unsigned int xmod, unsigned int slv, std::vector< std::vector<Histo*> >& his); //! Read an histogram

		void getFitResults(HistoType type, unsigned int mod, unsigned int slv, std::vector< Histo * > &thr, std::vector< Histo * > &noise, std::vector< Histo * > &chi2);                                                  //! Read a Fit from Dsp
		bool moduleActive(int nmod);                    //! true if a module is active during scan or datataking

		void writeRunConfig(PixRunConfig &cfg);         //! Get the run configuration parameters from PixModuleGroup
		void startRun(int ntrig);                   //! Start a run
		void stopRun(void);                                 //! Terminates a run
		void pauseRun(void);                                 //! Terminates a run
		void resumeRun(void);
		int runStatus(void);                                //! Check the status of the run
		void stopScan(void);                                //! Aborts a running scan
		int nTrigger(void);              //! Returns the number of trigger processed so far
		int getBoardID() {return m_boardIDRB;};

		// new function for MonLeak and HitBusScaler
		void shiftPixMask(int mask, int cap=2, int steps=1);
		int  readHitBusScaler(int mod, int ife, PixScan* scn);
		bool checkRxState(rxTypes type);
		void getServiceRecords(std::string &txt, std::vector<int> &srvCounts);
		void setAuxClkDiv(int div);
		void setIrefPads(int bits);
		void setIOMUXin(int bits);
		void sendGlobalPulse(int length);

		bool testGlobalRegister(int module, std::vector<int> &data_in,
		  std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg=true, int feIndex=0);
		bool testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out, 
					 bool ignoreDCsOff, int DC=-1, bool sendCfg=true, int feIndex=0, bool bypass=true);
		bool testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp, bool shift_only, 
				     bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex=0);

		void hwInfo(std::string &txt);
		void updateDeviceHandle();
		SiUSBDevice* getUsbHandle(){return m_dev;};

		bool getGenericBuffer(const char *type, std::string &textBuf);

		// added from USBpix's source scan mode - leave empty if source scan mode is not supported
		bool getSRAMReadoutReady(void);
		bool getSRAMFull(void);
		bool getMeasurementPause(void);
		bool getMeasurementRunning(void);
		int  getSRAMFillLevel(void);
		bool getTluVeto(void);
		int  getTriggerRate(void);
		int  getEventRate(void);
		bool getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram);

	private:
		void configInit(void);   //! Init configuration structure

		void detectReadoutChannels(void);
		std::string translateScanParam(PixScan::ScanParam param, std::string gr);
		void getMaskSettings(MaskSettings &m, PixScan &scn);

		void initLoop(PixScan *scn, DataDecoder &dec);
		void strobeScan(PixScan *scn);
		void scanLoop(Command &loop_cmd, int repeat, MaskSettings &m);
		void sourceScan(int max_event, int trigger_type, int count_type, int lvl1_delay);

		SiUSBDevice* m_dev = nullptr;
		std::unique_ptr<Board> board;
		std::unique_ptr<Scan> current_scan;
		std::unique_ptr<CommandBuffer> module_config;

		std::map<int, Frontend> frontends;
		std::vector<int> fe_ids;

		static const int NUM_CHANNELS = 8;
		std::array<int, NUM_CHANNELS> ch_to_fe;// = {{-2,-2,-2,-2,-2,-2,-2,-2}};
		std::array<int, NUM_CHANNELS> fe_to_ch;// = {{-2,-2,-2,-2,-2,-2,-2,-2}};

		std::atomic<bool> running = {false};
		std::atomic<bool> finished = {false};
		std::atomic<bool> cancel_scan = {false};

		std::atomic<int> trigger_rate = {0};
		std::atomic<int> event_rate = {0};

		std::string m_infoBuff, m_errBuff;

		// configuration settings
		int m_outputMode;
		int m_readoutInterval;
		int m_boardID;
		int m_boardIDRB;

		// distinguish board type: 0: MMC3, 1: MIO3+SCA, 2: MIO3+BIC
		int m_cardFlavour;
		std::string m_FPGA_filename, m_uC_filename;
		
};

}

#endif
