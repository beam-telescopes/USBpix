/////////////////////////////////////////////////////////////////////
// USBI3PixController.h
/////////////////////////////////////////////////////////////////////
//
// 10/11/08  Version 0.0
// Author: Jens Weingarten



#ifndef _PIXLIB_USBI3PIXCONTROLLER
#define _PIXLIB_USBI3PIXCONTROLLER

#define PIXEL_REG_INT 90

#include <map>
#include <string>

#include "PixController.h"

#include <queue>

#include "PixScan.h"

class ConfigFEMemoryI3;
class ConfigRegisterI3;
class SiUSBDevice;

namespace PixLib {

  class Bits;
  class Config;

  //! Pix Controller Exception class; an object of this type is thrown in case of a USB error
  class USBI3PixControllerExc : public PixControllerExc {
  public:
    enum ErrorType{OK, INVALID_PID, INVALID_VID, FPGA_ERROR, UC_ERROR, NOBOARD, NOHANDLE, FILE_ERROR, INIT_ERROR, NOT_IMPLEMENTED, NON_UNIFORM_POINTS_NOT_IMPLEMENTED, SCAN_NOT_IMPLEMENTED, INCONS_FE_CFG}; 
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
    case INCONS_FE_CFG:
      message = "No. of chips handed over to USBPixController::writeModuleConfig is inconsistent with board set-up";
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
	enum triggerReplicationTypes{REPLICATION_OFF, REPLICATION_MASTER, REPLICATION_SLAVE};

    USBI3PixController(PixModuleGroup &modGrp, DBInquire *dbInquire);     //! Constructor
    USBI3PixController(PixModuleGroup &modGrp);                           //! Constructor
    ~USBI3PixController();                                        //! Destructor
    void initHW();      //! Hardware (re)init
    void testHW();      //! Hardware test --> JW: Abfrage ob FPGA konfiguriert oder so was ähnliches
    void updateRegs(){}; // update controller-specific registers - currently not needed

    void sendCommand(int commands, int moduleMask);   
    void sendCommand(Bits commands, int moduleMask);                                    //! Send command from bit pattern

    void sendCommandOutSync(Bits commands, int moduleMask, std::vector<Bits>& output);  //! Send command sync
    void sendCommandOutAsync(Bits commands, int moduleMask, int commandID);             //! Send command async

    bool checkOutReady(int commandID);                                                  //! Wait for command termination 
    void getOutput(int commandID, std::vector<Bits>& output);                           //! Get command output

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

    //DLP: sends the charge calibration for the clusterizer
    void sendPixelChargeCalib(int /*pModuleID*/, unsigned int /*pCol*/, unsigned int /*pRow*/, unsigned int /*pTot*/, float /*pCharge*/){};

    //JW: Setup data paths inside FPGA
    void setCalibrationMode();
    void setConfigurationMode();
    void setRunMode();

    void setFERunMode(){setRunMode();};
    void setFEConfigurationMode(){setConfigurationMode();};

    // dummy, don't exist for FE-I3
    void readEPROM(){};
    void burnEPROM(){};
    void readGADC(int , std::vector<int> &){};
    void readGADC(int , std::vector<int> &, int ){};
    void getServiceRecords(std::string &, std::vector<int> &){};
    void setAuxClkDiv(int ){};
    void setIrefPads(int ){};
    void setIOMUXin(int ){};
    void sendGlobalPulse(int){};

    bool checkRxState(rxTypes type);
    bool getGenericBuffer(const char *type, std::string &textBuf);

    void writeScanConfig(PixScan &scn);                               //! Write scan parameters

    void startScan(PixScan *scn);
    void measureEvtTrgRate(PixScan* /*scn*/, int /*mod*/, double& /*erval*/, double& /*trval*/){};


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
    void pauseRun();
    void resumeRun();
    int runStatus();                        //! Check the status of the run
    int nTrigger();                         //! Returns the number of trigger processed so far

    bool getSRAMReadoutReadySignal() {return m_SRAMReadoutReadySignal;}
    bool getSRAMFullSignal() {return m_SRAMFullSignal;}
    bool getMeasurementPauseSignal() {return m_MeasurementPauseSignal;}
    bool getMeasurementRunningSignal() {return m_MeasurementRunningSignal;}
    int getSRAMFillLevel() {return m_SRAMFillLevel;}
    int getNumberOfTriggers() {return m_NumberOfTriggers;}
    int getNumberOfHitWords() {return m_NumberOfHitWords;}
    int getTriggerRate() {return m_TriggerRate;}
    int getHitRate() {return m_HitRate;}
    bool getSourceScanData(unsigned int * & data, bool forceReadSram);
    //    bool getSourceScanData(std::list<unsigned int>* &data);
    int getBoardID() {return m_boardIDRB;};

    bool moduleActive(int nmod);            //! True if module is active during scan or run

    //JW: direct access to usb board FIFO
    void downloadFifo(int size);

    //JW: ROD spezifisch. Nicht nützlich?
    bool primitiveExists(std::string str);  //! Ask if a given primitive is implemented

    bool getSRAMReadoutReady(){return false;};
    bool getSRAMFull(){return false;};
    bool getMeasurementPause(){return false;};
    bool getMeasurementRunning(){return false;};
    int getCollectedTriggers(){return 0;};
    int getCollectedHits(){return 0;};
    int getEventRate(){return 0;};

    //JW: Funktionen von RodPixController. Einzeln nachsehen...
    void stopScan(); //JW: Abort Knopf, sehr nützlich!

    bool testGlobalRegister(int module, std::vector<int> &data_in,
      std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg=true, int feIndex=0);
    bool testPixelRegister(int module, std::string regName,
      std::vector<int> &data_in, std::vector<int> &data_out,
			   bool ignoreDCsOff, int DC=-1, bool sendCfg=true, int feIndex=0, bool bypass=true);
    // for compatibility with FE-I4; does not apply to FE-I3, will thus never be implemented
    bool testScanChain(std::string , std::vector<int> , std::string &, std::string ,
		       bool , bool , bool , PixDcs *, double &, double &, int){return false;};

    //JW: helper functions
    int translateScanParam(PixScan::ScanParam param);

    void hwInfo(std::string &txt);
    SiUSBDevice* getUsbHandle(){return m_BoardHandle;}
    void updateDeviceHandle();

    void setBocRegister(std::string , int , int ){};
    void setBocRegister(std::string , int ){}; 
    int getBocRegister(std::string , int ){return 0;};  
    int getBocRegister(std::string ){return 0;};              

    //Accessor functions
    ConfigFEMemoryI3* getFEMemory() {return m_FEconfig;}

    // new functions for MonLeak and HitBusScaler
    void shiftPixMask(int mask, int cap=1, int steps=1);
    int readMonLeakADC(int mod, int ife, int nbits=10);
    bool testMonLeakComp(int mod, int ife, int MLguess);
    int readHitBusScaler(int mod, int ife, PixScan* scn);

  private:
    void configInit();         //! Init configuration structure
    void shiftPixMask_Int(int mask, int steps=1);

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

    int m_clusterPars[8];              // parameters for USBI3PixDll clusering algorithm

    std::vector<unsigned char*> m_fifo;  //! JW: FIFO content
    std::string m_FPGA_filename, m_uC_filename;
    int m_boardID;
    int m_boardIDRB;
    int m_regSyncDel;
    unsigned char* m_ReadArray;
    bool m_configValid;
    int m_scanConfigArray[9];          //! Configuration for uController scans
	int m_triggerReplication;
    int m_globalHitcount;
    bool m_SourceScanFlag;
    bool m_TestBeamFlag;
	bool m_skipHistosFlag;
    bool m_SourceScanDone;
    std::string m_SourceFilename;
    int m_srcCounterType;
    bool m_enableRJ45;

    int m_MaskStep;
    bool m_ScanBusy;
    bool m_ScanReady;
    bool m_ScanCanceled;
    bool m_ScanError;
	bool m_FEI3Warning;
	bool m_FEI3Error1;
	bool m_FEI3Error2;
	bool m_FEI3Error3;
	bool m_FEI3Error4;

    bool m_SRAMReadoutReadySignal;
    bool m_SRAMFullSignal;
    bool m_MeasurementPauseSignal;
    bool m_MeasurementRunningSignal;
    int m_SRAMFillLevel;
    int m_NumberOfTriggers;
    int m_NumberOfHitWords;
    int m_TriggerRate;
    int m_HitRate;

    //USB board classes
    ConfigFEMemoryI3* m_FEconfig;
    ConfigRegisterI3* m_USBreg;

    // translations between USB classes and FE class
    std::map<std::string, int> m_globRegNames, m_latchNames, m_cmdNames;

    // info/problem/error message storage strings
    std::string m_infoBuff, m_errBuff;

  };

}

#endif
