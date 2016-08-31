/////////////////////////////////////////////////////////////////////
// PixController.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//

//! Abstract class for the generic pixel module controller

#ifndef _PIXLIB_DUMMYPIXCONTROLLER
#define _PIXLIB_DUMMYPIXCONTROLLER

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <string>

#include "PixController.h"

namespace PixLib {

  class DummyPixController : public PixController {

public:

  DummyPixController(PixModuleGroup &modGrp, DBInquire *dbInquire);    //! Constructor
  DummyPixController(PixModuleGroup &modGrp);                          //! Constructor
  ~DummyPixController() {};                                        //! Destructor
  void initHW(){};                                            //! Hardware (re)init
  void testHW(){};                                            //! Hardware test

  void sendCommand(int command, int moduleMask);                                    //! Send command from PMG list
  void sendCommand(Bits commands, int moduleMask);                                    //! Send command from pattern

  void writeModuleConfig(PixModule& mod);                                             //! Write module configuration
  void readModuleConfig(PixModule& mod);                                              //! Read module configuration
  void sendModuleConfig(unsigned int moduleMask);                                     //! Send module configuration
  void sendPixel(unsigned int moduleMask);                                            //! send specif. pixel register cfg.
  void sendGlobal(unsigned int moduleMask);                                           //! send specif. gloabal register cfg.
  void sendPixel(unsigned int moduleMask, std::string regName, bool allDcsIdentical=false);  //! send pixel register cfg.
  void sendPixel(unsigned int moduleMask, std::string regName, int DC) ;  //! send pixel register cfg. for specific DC
  void sendGlobal(unsigned int moduleMask, std::string regName);                      //! send gloabal register cfg.

  void sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge);

  void setCalibrationMode();
  void setConfigurationMode();
  void setRunMode();
  void setFERunMode();
  void setFEConfigurationMode();

  void readEPROM(); // sends a global pulse to read the EPROM values to the FE GR
  void burnEPROM(); // burns the FE GR values to the EPROM. Caution, non reversibel!!!

  void readGADC(int type, std::vector<int> &GADCvalues, int FEindex);

  void writeScanConfig(PixScan &scn);                                           //! Write scan parameters
  void startScanDelegated(PixScan& scn);                                                 //! Start a scan
  void finalizeScan();                                                          //! finish undone issues after scan
  void measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval);  //! measure event and trigger rate - not a real occ./ToT-scan!

  bool fitHistos();                                                                                 //! fit histos
  bool getErrorHistos(unsigned int dsp, Histo* &his);                                               //! get error arrays

  void getHisto(HistoType type, unsigned int xmod, unsigned int slv, std::vector< std::vector<Histo*> >& his); //! Read an histogram

  void getFitResults(HistoType type, unsigned int mod, unsigned int slv, std::vector< Histo * > &thr, std::vector< Histo * > &noise, std::vector< Histo * > &chi2);                                                  //! Read a Fit from Dsp
  bool moduleActive(int nmod);                    //! true if a module is active during scan or datataking

  void writeRunConfig(PixRunConfig &cfg);         //! Get the run configuration parameters from PixModuleGroup
  void startRun(int ntrig=0);                   //! Start a run
  void stopRun();                                 //! Terminates a run
  void pauseRun();                                 //! Terminates a run
  void resumeRun();
  int runStatus();                                //! Check the status of the run
  void stopScan();                                //! Aborts a running scan
  int nTrigger();              //! Returns the number of trigger processed so far
  int getBoardID();

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

  bool getGenericBuffer(const char *type, std::string &textBuf);

  // added from USBpix's source scan mode - leave empty if source scan mode is not supported
  bool getSRAMReadoutReady(){return false;};
  bool getSRAMFull(){return false;};
  bool getMeasurementPause(){return false;};
  bool getMeasurementRunning(){return false;};
  int  getSRAMFillLevel(){return 0;};
  bool getTluVeto(){return false;};
  int  getTriggerRate(){return 0;};
  int  getEventRate(){return 0;};
  bool getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram){return false;};

protected:

private:
  void configInit();   //! Init configuration structure
  int m_boardId;
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
  //  ScanControl m_scanPar;             //! Scan configuration structure
    bool m_slowCtrlMode;
    int m_clusterPars[8];              // parameters for USBPixDll clustering algorithm: cluster depth in col, row, BCID, min cluster size, max cluster size, max cluster hit TOT, max event incomplete, max event errors

    std::vector<unsigned char*> m_fifo;  //! JW: FIFO content
    std::string m_FPGA_filename, m_uC_filename;
 //   int m_boardID[NBOARDS_MAX];
    int m_boardIDRB;
    int m_readoutSpeed;
    int m_triggerReplication;
    int m_cableLength;
    bool m_selCmd;
    bool m_enableCmdLvl1;
    int m_ioMuxSel;
    int m_ioMuxIn;
    int m_auxClkDiv;
    int m_irefPads;
    bool m_invIrefPads;
    int m_AdapterCardFlavor; // selects between std. and BURN-IN adapter card. false == std. adapter card.
    int m_feToRead; // only with BURN-IN card, otherwise FE3 needs to be read!
    unsigned char* m_ReadArray;
    bool m_configValid;
   // int m_scanConfigArray[NSCANCFG];          //! Configuration for uController scans
    // Manchester coding enable and clk phaseint m_boardId;
    bool m_enaManCode;
    int m_manCodePhase;
    int m_ctrlID;
    std::string m_ipramFile;           //! SDSP ipram file
    std::string m_idramFile;           //! SDSP idram file
    std::string m_extFile;
    int m_grId[4][4];
    int loop0Parameter;
    int loop1Parameter;
    int loop2Parameter;
    int nmaskStage;
    int nLoop0Parameters;
    int nLoop1Parameters;
    int nLoop2Parameters;
    int m_nTrigger;
    int m_status;
    int maskStage;


    unsigned int m_fmtLinkMap[8];
    //PixModule::FEflavour m_flavour;

    // source scan
    bool m_newRawDataFile;
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
    bool m_upcStartScanHasFinished; // signal that is asserted after startScanDelegated() has finished, only used for normal scan, used in nTrigger() method to assert m_scanDone
    bool m_upcScanCancelled; // signal that is asserted when a scan was successfully cancelled/stopped

    //USB board class
//    USBpix* m_USBpix;

    // book-keeping of chip IDs to use them in case of >1 FE per board
    //int m_chipIds[NBOARDS_MAX];

    // counter and limit for time-controller source scan
    int m_srcSecStart, m_srcSecMax;

    // translations between USB classes and FE class
    std::map<std::string, int> m_globRegNames, m_latchNames;//, m_cmdNames;

    // info/problem/error message storage strings
    std::string m_infoBuff, m_errBuff, m_diagBuff, m_xferBuff;

};

}

#endif

