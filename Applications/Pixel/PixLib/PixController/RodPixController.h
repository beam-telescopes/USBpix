/////////////////////////////////////////////////////////////////////
// RodPixController.h
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//

//! Class for the Pixel ROD

#ifndef _PIXLIB_RODPIXCONTROLLER
#define _PIXLIB_RODPIXCONTROLLER

#include "processor.h"
#include "PixController.h"
#include "scanOptions_2.h"

/* cannot really include registeraddress.h clashes with BocAddrresses.h */
#include "RodCrate/registeraddress.h"
/* ugly hack */
#undef BOC_RESET 
#undef BOC_STATUS
#undef BOC_MODULE_TYPE 
#undef BOC_MANUFACTURER
#undef BOC_SERIAL_NUMBER
#include <queue>

namespace SctPixelRod {
  class VmeInterface;
  class RodPrimitive; 
  class RodModule;
  class RodOutList;
  class BocCard;
}

namespace PixLib {

class Bits;
class Config;

//! Pix Controller Exception class; an object of this type is thrown in case of a ROD error
class RodPixControllerExc : public PixControllerExc {
public:
  enum ErrorType{OK, BAD_SLOT_NUM, DMA_NOT_PERMITTED, TOO_MANY_MODULES, INVALID_MODID, NO_VME_INTERFACE,
                 ILLEGAL_HASHING_SCHEME, NOT_IMPLEMENTED, BAD_CONFIG};
  //! Constructor
  RodPixControllerExc(ErrorType type, ErrorLevel el, std::string name, int info1, int info2) : 
    PixControllerExc(el, name), m_errorType(type), m_info1(info1), m_info2(info2) {}; 
  RodPixControllerExc(ErrorType type, ErrorLevel el, std::string name, int info1) : 
    PixControllerExc(el, name), m_errorType(type), m_info1(info1), m_info2(0) {}; 
  RodPixControllerExc(ErrorType type, ErrorLevel el, std::string name) : 
    PixControllerExc(el, name), m_errorType(type), m_info1(0), m_info2(0) {}; 

  //! Dump the error
  virtual void dump(std::ostream &out) {
    out << "Pixel Controller " << getCtrlName();
    out << " -- Type : " << dumpType();
    out << " -- Level : " << dumpLevel() << std::endl; 
  }
  //! m_errorType accessor
  ErrorType getErrorType() { return m_errorType; };
private:
  std::string dumpType() {
    std::string message;
    switch (m_errorType) {
    case OK : 
      return "No errors";
    case BAD_SLOT_NUM :
      std::ostringstream(message) <<  "Invalid ROD slot number (" << m_info1 << ")" ;
      return message;
    case NOT_IMPLEMENTED :
      std::ostringstream(message) <<  "Function is not implemented in RodPixController" ;
      return message;
    default :
      return "Uknown error";
    }
  }
  ErrorType m_errorType;
  int m_info1, m_info2;
};

class RodPixController : public PixController {
public:
  RodPixController(PixModuleGroup &modGrp, DBInquire *dbInquire);     //! Constructor
  RodPixController(PixModuleGroup &modGrp);                           //! Constructor
  virtual ~RodPixController();                                        //! Destructor
  virtual void initHW();                                              //! Hardware (re)init
  virtual void testHW();                                              //! Hardware test
  virtual void updateRegs();                                          //! update controller-specific stuff internally
  virtual void hwInfo(std::string &txt);                              //! simple text string info
  virtual void createVmeInterface();                                  //! Create a (shared) VME interface

  virtual void sendCommand(int command, int moduleMask);                                      //! Send command from PMG list
  virtual void sendCommand(Bits commands, int moduleMask);                                    //! Send command no out
  virtual void sendCommandOutSync(Bits commands, int moduleMask, std::vector<Bits>& output);  //! Send command sync
  virtual void sendCommandOutAsync(Bits commands, int moduleMask, int commandID);             //! Send command async
  virtual bool checkOutReady(int commandID);                                                  //! Wait for command termination 
  virtual void getOutput(int commandID, std::vector<Bits>& output);                           //! Get command output
  virtual void writeModuleConfig(PixModule& mod);                                             //! Write module configuration 
  virtual void readModuleConfigIdentifier(int moduleId, char* idStr, char* tag, UINT32* revision, UINT32* crc32chksm);  //! Read module configuration identifier
  virtual void readModuleConfig(PixModule& mod);                                              //! Read module configuration 
  virtual void sendModuleConfig(unsigned int moduleMask);                                     //! Send module configuration
  virtual void sendPixel(unsigned int){};                                                     //! send specif. pixel register cfg.
  virtual void sendGlobal(unsigned int){};                                                    //! send specif. gloabal register cfg.
  virtual void sendPixel(unsigned int, std::string, bool) {};                                 //! send pixel register cfg.
  virtual void sendPixel(unsigned int, std::string, int){} ;                                  //! send pixel register cfg. for specific DC
  virtual void sendGlobal(unsigned int, std::string){};                                       //! send gloabal register cfg.

  virtual void sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge);

  virtual void setCalibrationMode();
  virtual void setConfigurationMode();
  virtual void disableLink(int link);
  virtual void enableLink(int link);
  virtual void setRunMode();
  virtual void setFERunMode(){};
  virtual void setFEConfigurationMode(){};
  
  virtual void readEPROM(){}; // sends a global pulse to read the EPROM values to the FE GR
  virtual void burnEPROM(){}; // burns the FE GR values to the EPROM. Caution, non reversibel!!!
  
  virtual void readGADC(int , std::vector<int> &, int){};

  virtual void writeScanConfig(PixScan &scn);                               //! Write scan parameters
  virtual void startScan(PixScan *scn){writeScanConfig(*scn); startScan();};
  virtual void measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval);  //! measure event and trigger rate - not a real occ./ToT-scan!

  virtual bool fitHistos();                                                 //! fit histos
  virtual bool getErrorHistos(unsigned int dsp, Histo* &his);               //! Read collected error arrays  
  virtual void getHisto(HistoType type, unsigned int xmod, unsigned 
                        int slv, std::vector< std::vector<Histo*> > &his);  //! Read an histogram
  virtual void getFitResults(HistoType type, unsigned int mod, 
                             unsigned int slv, std::vector< Histo * > &thr, 
                             std::vector< Histo * > &noise, 
                             std::vector< Histo * > &chi2);                 //! Read a Fit from Dsp
  virtual void writeRunConfig(PixRunConfig &cfg); //! Get the run configuration parameters from PixModuleGroup
  virtual void startRun(int ntrig = 0);           //! Start a run
  virtual void stopRun();                         //! Terminates a run
  virtual void pauseRun(){};                          
  virtual void resumeRun(){};   
  virtual int runStatus();                        //! Check the status of the run
  virtual int nTrigger();              //! Returns the number of trigger processed so far
  virtual int getBoardID() {return m_ctrlID;};
  virtual bool moduleActive(int nmod) {           //! True if module is active during scan or run
    if (nmod>=0 && nmod<32) {
      for (unsigned int dsp=0; dsp<4; dsp++) {
	if (m_dspModMask[dsp] & (0x1<<nmod)) return true;
      } 
    }
    return false;
  }
  virtual int moduleOutLink(int nmod);            //! Returns the module out link or -1 if the module is not active or undefined
  virtual void downloadFifo(int size);
  virtual bool getFifoBit(unsigned int ic, unsigned int il);
  virtual void prepareTestPattern(PixScan *scn);
  virtual void runTestPattern(PixScan *scn, std::vector< Histo* >&vh);

  virtual bool getSRAMReadoutReady(){return false;};
  virtual bool getSRAMFull(){return false;};
  virtual bool getMeasurementPause(){return false;};
  virtual bool getMeasurementRunning(){return false;};
  virtual int getSRAMFillLevel(){return 0;};
  virtual int getCollectedTriggers(){return 0;};
  virtual int getCollectedHits(){return 0;};
  virtual int getTriggerRate(){return 0;};
  virtual int getEventRate(){return 0;};
  virtual bool getSourceScanData(unsigned int * & data, bool forceReadSram);

  virtual bool testGlobalRegister(int module, std::vector<int> &data_in, std::vector<int> &data_out, 
				  std::vector<std::string> &label, bool sendCfg, int feid);
  virtual bool testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out,
			   bool ignoreDCsOff, int DC=-1, bool senCfg=true, int feIndex=0, bool bypass=true);
  virtual bool testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp, bool shift_only, 
			     bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex=0);

  virtual void setCalibrationDebugMode();
  virtual void setConfigurationDebugMode(); 
  virtual void setupTrapping(int slave, PixScan &scn);
  virtual void setupTrapping(int slave);
  virtual void startTrapping(int slave);
  virtual void stopTrapping(int slave);
  virtual void startHistogramming(int slave);
  virtual void stopHistogramming(int slave);
  virtual void stopScan();

  virtual void setGroupId(int module, int group);
  virtual void enableModuleReadout(int module);
  virtual void disableModuleReadout(int module, bool trigger);

  virtual void ledFlash(int period, int repet, int led);
  virtual void ledFlashSlave(int period, int repet, int slave);
  virtual void setVerboseLevel(int level, int buffernr);
  virtual void echo(Bits in, Bits out, unsigned int ndat);
  virtual void testGlobalRegister(int module, int frontend);
  // new function for MonLeak and HitBusScaler
  void shiftPixMask(int mask, int cap=2, int steps=1);
  int  readHitBusScaler(int mod, int ife, PixScan* scn);
  bool checkRxState(rxTypes type);
  void getServiceRecords(std::string &txt, std::vector<int> &srvCounts);
  void setAuxClkDiv(int div);
  void setIrefPads(int bits);
  void setIOMUXin(int bits);
  void sendGlobalPulse(int length);
  
  void readMasterMem(int startAddr, int nWords, std::vector<unsigned int> &out);
  void writeMasterMem(int startAddr, int nWords, std::vector<unsigned int> &in);
  void readSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out);
  void writeSlaveMem(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in);
  void readSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &out);
  void writeSlaveMemDirect(int slaveId, int startAddr, int nWords, std::vector<unsigned int> &in);
  void readFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &out);
  void writeFifo(int fifo, int bank, int nWords, std::vector<unsigned int> &in);
  void writeRegister(int regId, int offset, int size, unsigned int value);
  unsigned int readRegister(int regId, int offset, int size);
  void executeMasterPrimitive(SctPixelRod::RodPrimitive &prim);
  void executeMasterPrimitiveSync(SctPixelRod::RodPrimitive &prim, std::vector<unsigned int> &out);
  void executeMasterPrimitiveSync(SctPixelRod::RodPrimitive &prim, SctPixelRod::RodOutList* &out);
  void executeMasterPrimitiveSync(SctPixelRod::RodPrimitive &prim);
  void executeSlavePrimitive(SctPixelRod::RodPrimitive &prim, int slave);
  void executeSlavePrimitiveSync(SctPixelRod::RodPrimitive &prim, std::vector<unsigned int> &out, int slave);
  void executeSlavePrimitiveSync(SctPixelRod::RodPrimitive &prim, SctPixelRod::RodOutList* &out, int slave);
  void executeSlavePrimitiveSync(SctPixelRod::RodPrimitive &prim, int slave);
  void executeSlavePrimitiveOnAll(SctPixelRod::RodPrimitive &prim);
  void executeSlavePrimitiveOnAllSync(SctPixelRod::RodPrimitive &prim);
  void executeSlavePrimitiveOnAllSync(SctPixelRod::RodPrimitive &prim, std::vector<unsigned int> &out);
  void executeSlavePrimitiveOnAllSync(SctPixelRod::RodPrimitive &prim, SctPixelRod::RodOutList* &out);
  void waitPrimitiveCompletion(std::vector<unsigned int> &out);
  void waitPrimitiveCompletion(SctPixelRod::RodOutList* &out);
  void waitPrimitiveCompletion();

  bool getGenericBuffer(const char *type, std::string &textBuf);

  void initBoc();              
  void getBocStatus();              
  void resetBoc();                
  void setBocRegister(std::string regtype, int channel, int value);
  void setBocRegister(std::string regtype, int value); 
  int getBocRegister(std::string type, int channel);  
  int getBocRegister(std::string type);              
  double getBocMonitorAdc(int channel);     
  void resetBocMonitorAdc();

private:
  virtual void configInit();         //! Init configuration structure
  virtual void startScan();          //! Start a scan
  
  SctPixelRod::RodModule *m_rod;     //! Pointer to RodModule
  SctPixelRod::VmeInterface *m_vme;  //! VME interface
  std::string m_ipramFile;           //! SDSP ipram file
  std::string m_idramFile;           //! SDSP idram file
  std::string m_extFile;             //! SDSP ext file
  int m_ctrlID;                      //! ROD Slot
  std::string m_mode;                //! ROD Operating Mode
  long int m_hscanDataSize;          //! Size of the scan data structure
  long int* m_hscanData;             //! Scan data structure
  bool m_trapRunning[4];             //! Trapping active for SDSPs
  bool m_histoRunning[4];            //! Histogram task running on SDSPs
  unsigned int m_nMod;               //! Number of active modules
  unsigned int m_modNum[32];         //! Module id
  unsigned int m_modGrp[32];         //! Module group
  unsigned int m_modGrpDef[32];      //! Module group
  unsigned int m_modDCI[32];         //! DCI line for a module
  unsigned int m_modDTO[32];         //! DTO line for a module
  bool m_modActive[32];              //! Active flag for a module
  bool m_modTrigger[32];             //! Trigger flag for a module
  int m_autoDisabledLinks[32];       //! Original status of autodisabled links
  unsigned int m_dspModMask[4];      //! Modules being histogrammed by a SDSP
  unsigned int m_modMask;            //! Modules being read out
  int m_modPosition[32];             //! Position of a moduleId in the internal vectors
  unsigned int m_lsNbin;             //! Total number of bins during las scan
  unsigned int m_nBinTot;            //! Total number of Points to store
  bool m_scanActive;                 //! true if a scan is active
  bool m_runActive;                  //! true if a run is active
  bool m_fitDone;                    //! true if fit has been performed after a scan
  unsigned int m_fmtLinkMap[8];      //! Formatters link map
  unsigned int  *m_scanParInt;       //! Scan configuration structure
  int m_MaskStageTotalSteps;         //! Mask Stage Total Steps

  Bits m_prepareSeq;                 //! Preparation sequence
  Bits m_executeSeq;                 //! Execution sequence
  Bits m_expectedOut;                //! Expected output pattern
  int m_inmLinkTable[96];            //! InLink usage table
  int m_inmModuleTable[96];          //| Module using InLink i
  std::vector<unsigned int> m_fifoA; //! InMem fifo A contetnt
  std::vector<unsigned int> m_fifoB; //! InMem fifo B conetent
  bool m_preAmpOff;         //! Pre-amp status
  ScanStatus m_dspScanStatus;        //! location of Dsp scan status structure
};

}

#endif

