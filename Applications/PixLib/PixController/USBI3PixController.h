/////////////////////////////////////////////////////////////////////
// USBPixControllerI3.h
/////////////////////////////////////////////////////////////////////
//
// 10/11/08  Version 0.0
// Author: Jens Weingarten



#ifndef _PIXLIB_USBPIXCONTROLLERI3
#define _PIXLIB_USBPIXCONTROLLERI3

#define PIXEL_REG_INT 90

#include "PixController.h"

#include "SiUSBLib.h"

#include <queue>

#include "PixScan.h"



//class ConfigFPGA;
//class ConfigFEMemoryI3;


//JW: forward declarations
//class ConfigFPGA;
class ConfigFEMemoryI3;
class ConfigRegisterI3;
class SiUSBDevice;

namespace PixLib {

class Bits;
class Config;
class PixDcs;	//CG:EDIT : added for virtual testScanChain()

//! Pix Controller Exception class; an object of this type is thrown in case of a USB error
class USBI3PixControllerExc : public PixControllerExc {
public:
  //CG: added new enum "WRONG_FEI3_COUNT"
  enum ErrorType{OK, INVALID_PID, INVALID_VID, FPGA_ERROR, UC_ERROR, NOBOARD, NOHANDLE, FILE_ERROR, INIT_ERROR, NOT_IMPLEMENTED, NON_UNIFORM_POINTS_NOT_IMPLEMENTED, SCAN_NOT_IMPLEMENTED, WRONG_FEI3_COUNT};
  //! Constructor
  USBI3PixControllerExc(ErrorType type, ErrorLevel el, std::string name, int info1, int info2) :
    PixControllerExc(el, name), m_errorType(type), m_info1(info1), m_info2(info2) {};
  USBI3PixControllerExc(ErrorType type, ErrorLevel el, std::string name, int info1) :
    PixControllerExc(el, name), m_errorType(type), m_info1(info1), m_info2(0) {};
  USBI3PixControllerExc(ErrorType type, ErrorLevel el, std::string name) :
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
    case INVALID_PID :
      message = "Product ID invalid ("+a.str()+ ")" ;
      return message;
    case INVALID_VID :
      message = "Vendor ID invalid ("+a.str()+ ")" ;
      return message;
    case FPGA_ERROR :
      message = "Something wrong with the FPGA" ;
      return message;
    case UC_ERROR :
      message = "Something wrong with the micro controller" ;
      return message;
    case NOBOARD :
      message = "No USB board with ID " + a.str() + " found. Board connected?" ;
      return message;
    case NOHANDLE :
      message = "No SiUSBDevice found." ;
      return message;
    case FILE_ERROR :
      message = "Specified FPGA bit file not found." ;
      return message;
    case INIT_ERROR :
      message = "Initialisation not successful (" + a.str() + ")" ;
      return message;
    case NOT_IMPLEMENTED :
      message = "The function you tried to call is not yet implemented. Sorry..." ;
      return message;
    case NON_UNIFORM_POINTS_NOT_IMPLEMENTED:
       message = "Non-uniform scanpoints are not supported yet. Sorry..." ;
      return message;
    case SCAN_NOT_IMPLEMENTED:
      message = "Scan type not yet implemented. Sorry...";
      return message;
    case WRONG_FEI3_COUNT:
      message = "FE_Count != 1. only 1 FE supported.";
      return message;
    default :
      return "Uknown error";
    }
  }
  ErrorType m_errorType;
  int m_info1, m_info2;
};

class USBI3PixController : public PixController {
public:
  USBI3PixController(PixModuleGroup &modGrp, DBInquire *dbInquire);     //! Constructor
  USBI3PixController(PixModuleGroup &modGrp);                           //! Constructor
  ~USBI3PixController();                                        //! Destructor
  void initHW();      //! Hardware (re)init
  void testHW();      //! Hardware test --> JW: Abfrage ob FPGA konfiguriert oder so was ähnliches

  void sendCommand(int command, int moduleMask);                                    //! Send command from PMG list
  void sendCommand(Bits commands, int moduleMask);

  void sendModuleConfig(unsigned int moduleMask);               //! Send module configuration 
  void sendModuleConfig(PixModule& mod);
  void writeModuleConfig(PixModule& mod);                         //! Write module configuration
  void readModuleConfig(PixModule& mod);                          //! Read module configuration
  void sendPixel(unsigned int moduleMask);
  void sendPixel(unsigned int moduleMask, std::string regName, bool allDcsIdentical=false);
  void sendPixel(unsigned int moduleMask, std::string regName, int DC);
  void sendPixel(unsigned int moduleMask, std::string regName, int DC, bool sendGlob);
  void sendGlobal(unsigned int moduleMask);
  void sendGlobal(unsigned int moduleMask, std::string regName);

  //JW: Setup data paths inside FPGA
  void setCalibrationMode();
  void setConfigurationMode();
  void setRunMode();

  void setUSBRunMode();

  void writeScanConfig(PixScan &scn);                               //! Write scan parameters

  void startScanDelegated(PixScan& scn);
  void finalizeScan();                                              //! finish undone issues after scan

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
  void makeSourceScanHistos();


  //JW: Setup, start und stop Datennahme Run
  void writeRunConfig(PixRunConfig &cfg); //! Get the run configuration parameters from PixModuleGroup
  void startRun(int ntrig = 0);           //! Start a run
  void stopRun();                         //! Terminates a run
  int runStatus();                        //! Check the status of the run
  int nTrigger();                         //! Returns the number of trigger processed so far

  bool getSRAMReadoutReady() {return m_sramReadoutReady;}
  bool getSRAMFull() {return m_sramFull;}
  bool getMeasurementPause() {return m_measurementPause;}
  bool getMeasurementRunning() {return m_measurementRunning;}
  int getSRAMFillLevel() {return m_sramFillLevel;}
  bool getTluVeto() {return m_tluVeto;}       // get readback state of TLU veto from FPGA
  int getTriggerRate() {return m_triggerRate;}
  int getEventRate() {return m_eventRate;}
  bool getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram);

  bool moduleActive(int nmod);            //! True if module is active during scan or run

  //JW: Funktionen von RodPixController. Einzeln nachsehen...
  void stopScan(); //JW: Abort Knopf, sehr nützlich!

//  void ledFlash(int period, int repet, int led); //JW: Spielerei für Board Test
  bool testGlobalRegister(int module, std::vector<int> &data_in, std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg, int frontend = 0);
  bool testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out, bool ignoreDCsOff, int DC = -1, bool sendCfg = true, int frontend = 0, bool bypass = true);

//JW: helper functions
  int translateScanParam(PixScan::ScanParam param);

  void hwInfo(std::string &txt);
  SiUSBDevice* getUsbHandle(){return m_BoardHandle;};
  void updateDeviceHandle();

//Accessor functions
  ConfigFEMemoryI3* getFEMemory() {return m_FEconfig;};

  // new functions for MonLeak and HitBusScaler
  void shiftPixMask(int mask, int cap=2, int steps=1);
  int  readMonLeakADC(int mod, int ife, int nbits=10);
  bool testMonLeakComp(int mod, int ife, int MLguess);
  int  readHitBusScaler(int mod, int ife, PixScan* scn);

  bool getGenericBuffer(const char *type, std::string &textBuf);

  //Additional empty public functions for abstract PixController functions

  void setFERunMode();
  void setFEConfigurationMode();
  void readEPROM(){};
  void burnEPROM(){};
  void readGADC(int /*type*/, std::vector<int> &/*GADCvalues*/, int /*FEindex=-1*/){};
  void pauseRun();
  void resumeRun();
  int getBoardID(){return m_boardIDRB;};
  bool checkRxState(rxTypes type);
  void getServiceRecords(std::string &/*txt*/, std::vector<int> &/*srvCounts*/){};
  void setAuxClkDiv(int /*div*/){};
  void setIrefPads(int /*bits*/){};
  void setIOMUXin(int /*bits*/){};
  void sendGlobalPulse(int /*length*/){};
  void sendPixelChargeCalib(int /*pModuleID*/, unsigned int /*pCol*/, unsigned int /*pRow*/, unsigned int /*pTot*/, float /*pCharge*/){};
  void measureEvtTrgRate(PixScan */*scn*/, int /*mod*/, double &/*erval*/, double &/*trval*/){};
  bool testScanChain(std::string /*chainName*/, std::vector<int> /*data_in*/, std::string &/*data_out*/, std::string /*data_cmp*/, bool /*shift_only*/,
		     bool /*se_while_pulse*/, bool /*si_while_pulse*/, PixDcs */*dcs*/, double &/*curr_bef*/, double &/*curr_after*/, int /*feIndex*/)
  {return false;};

private:
  void configInit();         //! Init configuration structure
  void procSourceScan();

  SiUSBDevice* m_BoardHandle;        //JW: Handle des USB Boards

  long int m_hscanDataSize;          //! Size of the scan data structure
  long int* m_hscanData;             //! Scan data structure
  unsigned int m_nMod;               //! Number of active modules
  unsigned int m_modNum[32];         //! Module id
  unsigned int m_modGrp[32];         //! Module group
  bool m_modActive[32];              //! Active flag for a module
  bool m_modTrigger[32];             //! Trigger flag for a module
  unsigned int m_modMask;            //! Modules being read out
  int m_modPosition[32];             //! Position of a moduleId in the internal vectors

  bool m_NewRawDataFile;

  std::map<std::string, int> m_latchNames;

  int m_clusterPars[5];              // parameters for USBPixDll clusering algorithm

  int m_boardIDRB;
  std::vector<unsigned char*> m_fifo;  //! JW: FIFO content
  std::string m_FPGA_filename, m_uC_filename;
  int m_boardID;
  int m_regSyncDel;
  unsigned char* m_ReadArray;
  bool m_configValid;
  int m_scanConfigArray[9];          //! Configuration for uController scans
  int m_globalHitcount;
  bool m_SourceScanFlag;
  bool m_testBeamFlag;
  int m_sramReadoutAt;
  bool m_SourceScanDone;
  std::string m_SourceFilename;
  int m_srcCounterType;
  bool m_enableRJ45;

  bool m_sramReadoutReady;
  bool m_sramFull;
  bool m_measurementPause;
  bool m_measurementRunning;
  int  m_sramFillLevel;
  bool m_tluVeto;
  int  m_triggerRate;
  int  m_eventRate;

  //USB board classes
  //  ConfigFPGA*     m_FPGAconfig;
  ConfigFEMemoryI3* m_FEconfig;
  ConfigRegisterI3* m_USBreg;

  // info/problem/error message storage strings
  std::string m_infoBuff, m_errBuff;


};

}

#endif
