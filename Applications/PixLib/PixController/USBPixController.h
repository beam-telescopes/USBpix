/////////////////////////////////////////////////////////////////////
// USBPixController.h
/////////////////////////////////////////////////////////////////////
//
// 10/11/08  Version 0.0
// Author: Jens Weingarten



#ifndef _PIXLIB_USBPIXCONTROLLER
#define _PIXLIB_USBPIXCONTROLLER

#define PIXEL_REG_INT 90
#define NSCANCFG 12
#define NBOARDS_MAX 2

#include <map>
#include <string>
#include <queue>
#include <memory>
#include <vector>

#include "PixController.h"
#include "PixScan.h"

// forward declarations
class USBpix;
class SiUSBDevice;

namespace gpac {
  class logical;
}
namespace PixLib {

  class Bits;
  class Config;
  class PixDcs;
  class PixScan;


//! Pix Controller Exception class; an object of this type is thrown in case of a USB error
  class USBPixControllerExc : public PixControllerExc {
  public:
    enum ErrorType{OK, INVALID_PID, INVALID_VID, FPGA_ERROR, UC_ERROR, NOBOARD, NOHANDLE, FILE_ERROR, INIT_ERROR, NOT_IMPLEMENTED, NON_UNIFORM_POINTS_NOT_IMPLEMENTED, SCAN_NOT_IMPLEMENTED, SYNC_PROBLEM, SCAN_RUNNING, NO_PIXDCS, UNKNOWN_CHAIN, WRONG_DATASIZE, ALLOC_PROBLEM, INCONS_FE_CFG, CLUSTER_ERROR, NO_ADAPTER, WRONG_ADAPTER, WRONGFW};


    //! Constructor
    USBPixControllerExc(ErrorType type, ErrorLevel el, std::string name, int info1, int info2) :
    PixControllerExc(el, name), m_errorType(type), m_info1(info1), m_info2(info2) {};
    USBPixControllerExc(ErrorType type, ErrorLevel el, std::string name, int info1) :
    PixControllerExc(el, name), m_errorType(type), m_info1(info1), m_info2(0) {};
    USBPixControllerExc(ErrorType type, ErrorLevel el, std::string name) :
    PixControllerExc(el, name), m_errorType(type), m_info1(0), m_info2(0) {};

    //! Dump the error
    virtual void dump(std::ostream &out) {
      out << "Pixel Controller " << getCtrlName();
      out << " -- Type: " << dumpType();
      out << " -- Level: " << dumpLevel() << std::endl;
    }
    //! m_errorType accessor
    ErrorType getErrorType() { return m_errorType; };
  private:
    std::string dumpType() {
      std::string message;
      std::stringstream a,b;
      a << m_info1;
      b << m_info2;
      switch (m_errorType) {
      case OK :
	return "No errors";
      case INVALID_PID:
	message = "Product ID invalid (" + a.str() + ")" ;
	return message;
      case INVALID_VID:
	message = "Vendor ID invalid (" + a.str() + ")" ;
	return message;
      case FPGA_ERROR:
	message = "Something wrong with the FPGA" ;
	return message;
      case UC_ERROR:
	message = "Something wrong with the microcontroller" ;
	return message;
      case NOBOARD:
	message = "No USB board with ID " + a.str() + " found. Board connected?" ;
	return message;
      case NOHANDLE:
	message = "No SiUSBDevice found." ;
	return message;
      case FILE_ERROR:
	message = "Specified FPGA bit file not found." ;
	return message;
      case INIT_ERROR:
	message = "Initialisation not successful (" + a.str() + ")" ;
	return message;
      case NOT_IMPLEMENTED:
	message = "The function you tried to call is not yet implemented." ;
	return message;
      case NON_UNIFORM_POINTS_NOT_IMPLEMENTED:
	message = "Non-uniform scanpoints are not supported yet." ;
	return message;
      case SCAN_NOT_IMPLEMENTED:
	message = "Scan type not implemented.";
	return message;
      case SYNC_PROBLEM:
	message = "Delay scan could not find a reasonable delay value for board ID "+a.str()+".";
	return message;
      case SCAN_RUNNING:
	message = "A scan is already running.";
	return message;
      case NO_PIXDCS:
	message = "Pointer to PixDcs is NULL or of wrong inherited type.";
	return message;
      case UNKNOWN_CHAIN:
	message = "Requested scan chain name is not known.";
	return message;
      case WRONG_DATASIZE:
	message = "Provided data size does not match size of scan chain.";
	return message;
      case ALLOC_PROBLEM:
	message = "Couldn't allocate required memory for processing.";
	return message;
      case INCONS_FE_CFG:
	message = "No. of chips handed over to USBPixController::writeModuleConfig is inconsistent with board set-up";
	return message;
      case CLUSTER_ERROR:
	message = "Clustering aborted. Raw data structure is invalid or incomplete. Noisy Chip?";
	return message;
      case NO_ADAPTER:
	message = "No adapter board detected, cannot check sanity of settings and thus will not proceed with initialisation.";
	return message;
      case WRONG_ADAPTER:
	message = "Different adapter board detected than found in configuration. Configuration was adjusted - check and re-start initialisation.";
	return message;
      case WRONGFW:
	message = "FW bit file is not compatible with adapter card type as set in configuration: this can potentially damage your hardware!";
	return message;
      default :
	return "Unknown error.";
      }
    }
    ErrorType m_errorType;
    int m_info1, m_info2;
  };

  class USBPixController : public PixController {
  public:

    USBPixController(PixModuleGroup &modGrp, DBInquire *dbInquire);     //! Constructor
    USBPixController(PixModuleGroup &modGrp, int adapterCardFlavor=0);  //! Constructor
    ~USBPixController();                                                //! Destructor
    void initHW();      //! Hardware (re)init
    void testHW();      //! Hardware test --> JW: Abfrage ob FPGA konfiguriert oder so was ähnliches
    void updateRegs(); // update USBPix-specific registers (not in PixController!)

    void sendCommand(int command, int moduleMask);                                      //! Send command from PMG list
    void sendCommand(Bits commands, int moduleMask);                                    //! Send command from bit pattern

    void writeModuleConfig(PixModule& mod);                         //! Write module configuration 
    void readModuleConfig(PixModule& mod);                          //! Read module configuration 
    void sendModuleConfig(unsigned int moduleMask);                 //! Send module configuration
    void sendModuleConfig(PixModule& mod);                          //! Send module configuration
    void sendPixel(unsigned int moduleMask);                        //! send specif. pixel register cfg.
    void sendGlobal(unsigned int moduleMask);                       //! send specif. gloabal register cfg.
    void sendPixel(unsigned int moduleMask, std::string regName, bool allDcsIdentical=false) ;  //! send pixel register cfg.
    void sendPixel(unsigned int moduleMask, std::string regName, int DC) ;  //! send pixel register cfg. for specific DC
    void sendGlobal(unsigned int moduleMask, std::string regName);  //! send gloabal register cfg.
    int  readGlobal(int regNo, int feIndex);
    void writeCcpdConfig(PixModule& mod);                                             //! write CCPD-part of config - called by writeModuleConfig
    void sendCCPDGlobal();                                                            //! send CCPD global register
    void sendCCPDPixel();                                                             //! send CCPD pixel register
    void sendCCPD();                                                                  //! send CCPD cfg

    //DLP: sends the charge calibration for the clusterizer
    void sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge);

    //JW: Setup data paths inside FPGA
    void setCalibrationMode();
    void setConfigurationMode();
    void setRunMode();

    void setFERunMode();
    void setFEConfigurationMode();

    // M.B.: FEs EPROM control
    void readEPROM(); // sends a global pulse to read the EPROM values to the FE GR
    void burnEPROM(); // burns the FE GR values to the EPROM. Caution, non reversibel!!!
    
    // M.B.: Read FEs GADC
    void readGADC(int type, std::vector<int> &GADCvalues, int FEindex=-1);

    void StartCCPDInject(); //! start CCPD injection 
    void StopCCPDInject();  //! stop CCPD injection

    void writeScanConfig(PixScan &scn);                               //! Write scan parameters
    void startScan(PixScan *scn);
    void finalizeScan();
    void measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval);

    bool fitHistos();                                                 //! Fit collected histograms
    bool getErrorHistos(unsigned int dsp, Histo* &his);               //! Read collected error arrays

    //JW: read histograms from SRAM
    void getHisto(HistoType type, unsigned int xmod, unsigned
      int slv, std::vector< std::vector<Histo*> > &his);  //! Read an histogram
    void getFitResults(HistoType type, unsigned int mod,
      unsigned int slv, std::vector< Histo * > &thr,
      std::vector< Histo * > &noise,
      std::vector< Histo * > &chi2);                 //! Read a Fit from Dsp
    void clearSourceScanHistos();
    void writeRawDataFile(bool close_file, int chipIndex);


    //JW: Setup, start und stop Datennahme Run
    void writeRunConfig(PixRunConfig &cfg); //! Get the run configuration parameters from PixModuleGroup
    void startRun(int ntrig = 0);           //! Start a run
    void stopRun();                         //! Terminates a run
    void pauseRun();
    void resumeRun();
    int getBoardID() {return m_boardIDRB;};
    int runStatus();                        //! Check the status of the run
    int nTrigger();                         //! Returns the number of trigger processed so far

    bool getSRAMReadoutReady() {return m_sramReadoutReady;}
    bool getSRAMFull() {return m_sramFull;}
    bool getTluVeto() {return m_tluVeto;}       // get readback state of TLU veto from FPGA
    bool getMeasurementPause() {return m_measurementPause;}
    bool getMeasurementRunning() {return m_measurementRunning;}
    int getSRAMFillLevel() {return m_sramFillLevel;}
	int getCollectedTriggers() {return m_collectedTriggers;}
    int getCollectedHits() {return m_collectedHits;}
    int getTriggerRate() {return m_triggerRate;}
    int getEventRate() {return m_eventRate;}
    bool getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram);

    bool moduleActive(int nmod);            //! True if module is active during scan or run

    void stopScan(); // called from scan abort button

    bool testGlobalRegister(int module, std::vector<int> &data_in,
      std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg=true, int feIndex=0);
    bool testPixelRegister(int module, std::string regName,
      std::vector<int> &data_in, std::vector<int> &data_out,
			   bool ignoreDCsOff, int DC=-1, bool sendCfg=true, int feIndex=0, bool bypass=true);
    bool testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp,
		       bool shift_only, bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex=0);

    void hwInfo(std::string &txt);
    void updateDeviceHandle();

    //JW: helper functions
    int translateScanParam(PixScan::ScanParam param);

    //Accessor functions
    SiUSBDevice*    getUsbHandle(int id=0);
    USBpix* getUSBpix() {return m_USBpix;};
    int getAdapterCardFlavor(){return m_AdapterCardFlavor;};

    // new functions specific to USBPix
    void shiftPixMask(int mask, int cap=2, int steps=1);
    int  readHitBusScaler(int mod, int ife, PixScan* scn);
    bool checkRxState(rxTypes type);
    void getServiceRecords(std::string &txt, std::vector<int> &srvCounts);
    void setAuxClkDiv(int div){m_auxClkDiv = div&0xff; updateRegs();};
    void setIrefPads(int bits){m_irefPads = bits&0xf; updateRegs();};
    void setIOMUXin(int bits){m_ioMuxIn = bits&0xf; updateRegs();};
    void sendGlobalPulse(int length);
    void configureReadoutChannelAssociation();

    bool getGenericBuffer(const char *type, std::string &textBuf);

  private:
    void configInit();         //! Init configuration structure
    void ClusterRawData(int pChipIndex);
    void sendPixel(unsigned int moduleMask, std::string regName, int DC, bool sendGlob) ;  //! mother of all send pixel register functions
    void initHWMultipleBoards();
    void initHWSingleBoard();
    int getFECount();
    void initChipIds();
    int detectAdapterCard(unsigned int boadId);

    SiUSBDevice* m_BoardHandle[NBOARDS_MAX];        //! handle of one or two USB boards

    long int m_hscanDataSize;          //! Size of the scan data structure
    long int* m_hscanData;             //! Scan data structure
    unsigned int m_nMod;               //! Number of active modules
    unsigned int m_modNum[32];         //! Module id
    unsigned int m_modGrp[32];         //! Module group
    bool m_modActive[32];              //! Active flag for a module
    bool m_modTrigger[32];             //! Trigger flag for a module
    unsigned int m_modMask;            //! Modules being read out
    int m_modPosition[32];             //! Position of a moduleId in the internal vectors

    //JW: change m_scanPar to a format usable with USB system!
    bool m_slowCtrlMode;
    int m_clusterPars[8];              // parameters for USBPixDll clustering algorithm: cluster depth in col, row, BCID, min cluster size, max cluster size, max cluster hit TOT, max event incomplete, max event errors

    std::vector<unsigned char*> m_fifo;  //! JW: FIFO content
    std::string m_FPGA_filename, m_uC_filename;
    int m_boardID[NBOARDS_MAX];
    int m_boardIDRB;
    int m_readoutSpeed;
    int m_triggerReplication;
    int m_cableLength;
    bool m_selCmd;
    bool m_enableCmdLvl1;
    bool m_enableGPACStrobePin;
    bool m_enableSCAStrobePin;
    int m_ioMuxSel;
    int m_tx2signal;
    int m_ioMuxIn;
    int m_auxClkDiv;
    int m_irefPads;
    bool m_invIrefPads;
    int m_AdapterCardFlavor; // selects between std. and BURN-IN adapter card. false == std. adapter card.
    int m_feToRead; // only with BURN-IN card, otherwise FE3 needs to be read!
    unsigned char* m_ReadArray;
    bool m_configValid;
    int m_scanConfigArray[NSCANCFG];          //! Configuration for uController scans
    // Manchester coding enable and clk phase
    bool m_enaManCode;
    int m_manCodePhase;
    bool m_MultiChipWithSingleBoard;
    bool m_OverrideEnableDemux;

    // source scan
    bool m_newRawDataFile;
    bool m_createdRawDataFile;
    bool m_sourceScanFlag;
    bool m_testBeamFlag;
    bool m_skipSourceScanHistos;
    bool m_fillSrcHistos;
    bool m_fillClusterHistos;
    std::string m_rawDataFilename;
    bool m_enableRJ45;
    bool m_sramReadoutReady;
    bool m_measurementPause;
    bool m_sramFull;
    bool m_tluVeto;         // readback state of TLU veto
    bool m_measurementRunning;
    int m_sramFillLevel;
    int m_collectedTriggers;
    int m_collectedHits;
    int m_triggerRate;
    int m_eventRate;
    bool m_sourceScanBusy; // is set during source scan
    bool m_sourceScanDone; // is set when source scan has finished
	int m_SourceScanEventQuantity;

    // normal scan
    bool m_innerLoopSwap;
    bool m_scanReady; // is set after USBpix::StartScan() method has finished
    bool m_scanCancelled; // is set when normal scan was aborted externally
    bool m_scanError; // is set by USBpix::StartScan() method, when an internal error occured, e.g. wrong parameters, timeouts, ...
    bool m_scanBusy; // is set during normal scan
    bool m_scanDone; // is set when the normal scan has finished, *only* asserted in nTrigger() method, used in runStatus()
    int m_scanStep;

    // any scan
    bool m_upcScanInit; // signal is used to avoid status readout in nTrigger() method during scan initialization
    bool m_upcScanBusy; // signal that is asserted during any kind of scan
    bool m_upcStartScanHasFinished; // signal that is asserted after startScan() has finished, only used for normal scan, used in nTrigger() method to assert m_scanDone
    bool m_upcScanCancelled; // signal that is asserted when a scan was successfully cancelled/stopped

    //USB board class
    USBpix* m_USBpix;

    // book-keeping of chip IDs to use them in case of >1 FE per board
    std::vector<int> m_chipIds;

    // counter and limit for time-controller source scan
    int m_srcSecStart, m_srcSecMax;

    // translations between USB classes and FE class
    std::map<std::string, int> m_globRegNames, m_latchNames;//, m_cmdNames;

    // translations between USB classes and CCPD class
    std::map<std::string, int> m_ccpd_globals;
    std::map<std::string, int> m_ccpd_globals2;
    std::map<std::string, int> m_ccpd_globalsLF;
    //CCPD 
    bool m_withCCPD;

    // info/problem/error message storage strings
    std::string m_infoBuff, m_errBuff;

    int m_readoutChannelInput[4];
    int m_inputDivider[4];
    std::vector<int> m_readoutChannelReadsChip;

    // for internal use: FE geometry - should be updated when cfg, is written
    unsigned int m_nColFe;
    unsigned int m_nRowFe;

  private:
    std::unique_ptr<gpac::logical> m_gpac;
  public:
    gpac::logical *getGpac();
  };

}

#endif
