/////////////////////////////////////////////////////////////////////
// PixController.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//

//! Abstract class for the generic pixel module controller

#ifndef _PIXLIB_PIXCONTROLLER
#define _PIXLIB_PIXCONTROLLER

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <string>
#include <memory>
#include <math.h>

#include "PixModuleGroup/PixModuleGroup.h"
#include "BaseException.h"

#include "Utility/CircularFifo.h"

namespace PixLib {

using UintCircBuff1MByte = CircularFifo<uint32_t, 1000000/sizeof(uint32_t)>;

class Bits;
class Histo;
class Config;
class PixModule;
class DBInquire;
class PixScan;
class PixRunConfig;
class PixDcs;

struct PixControllerInfo{
  std::string className;
  std::string decName;
  int extraOption;
};

//! Pix Controller Exception class; an object of this type is thrown in case of a controller error
class PixControllerExc : public SctPixelRod::BaseException{
public:
  enum ErrorLevel{INFO, WARNING, ERROR, FATAL};
  //! Constructor
  PixControllerExc(ErrorLevel el, std::string name) : BaseException(name), m_errorLevel(el), m_name(name) {}; 
  //! Destructor
  virtual ~PixControllerExc() {};

  //! Dump the error
  virtual void dump(std::ostream &out) {
    out << "Pixel Controller " << m_name << " -- Level : " << dumpLevel(); 
  }
  std::string dumpLevel() {
    switch (m_errorLevel) {
    case INFO : 
      return "INFO";
    case WARNING :
      return "WARNING";
    case ERROR :
      return "ERROR";
    case FATAL :
      return "FATAL";
    default :
      return "UNKNOWN";
    }
  }
  //! m_errorLevel accessor
  ErrorLevel getErrorLevel() { return m_errorLevel; };
  //! m_name accessor
  std::string getCtrlName() { return m_name; };
private:
  ErrorLevel m_errorLevel;
  std::string m_name;
};


class PixController {

public:
  enum HistoType{OCCUPANCY=0, LVL1, LV1ID, BCID, TOT, TOT_MEAN, TOT_SIGMA,
                 SCURVES, SCURVE_MEAN, SCURVE_SIGMA, SCURVE_CHI2, HITOCC, TOTAVERAGE,
                 TOTCAL_PARA, TOTCAL_PARB, TOTCAL_PARC, TOTCAL_CHI2, DSP_ERRORS,
                 CLUSTER_TOT, CLUSTER_SIZE, CLSIZE_TOT, SEED_TOT, SEED_LVL1, NSEEDS,
		 TOT0, TOT1, TOT2, TOT3, TOT4, TOT5, TOT6, TOT7,
		 TOT8, TOT9, TOT10, TOT11, TOT12, TOT13, TOT14, TOT15, CLUSTER_CHARGE, CLSIZE_CHARGE, CLUSTER_POSITION,
		 MON_LEAK, RAW_DATA_REF, RAW_DATA_DIFF_2 };

  enum roTypes{RO4OMBPS, RO160MBPS};
  enum rxTypes{RX0, RX1, RX2, EXT_TRG};
  enum triggerReplicationTypes{REPLICATION_OFF, REPLICATION_MASTER, REPLICATION_SLAVE};

  static PixController *make(PixModuleGroup &grp, DBInquire *dbInquire, std::string type);    //! Factory
  static PixController *make(PixModuleGroup &grp, std::string type, int extraOption=0);       //! Factory

  PixController(PixModuleGroup &modGrp, DBInquire *dbInquire) : 
                m_modGroup(modGrp), m_dbInquire(dbInquire) {          //! Constructor
    m_name = m_modGroup.getRodName();
  }
  PixController(PixModuleGroup &modGrp) : 
                m_modGroup(modGrp), m_dbInquire(NULL) {               //! Constructor
    m_name = m_modGroup.getRodName();
  }
  virtual ~PixController() {};                                        //! Destructor
  virtual void initHW() = 0;                                          //! Hardware (re)init
  virtual void testHW() = 0;                                          //! Hardware test

  virtual void sendCommand(int command, int moduleMask) = 0;                                    //! Send command from PMG list
  virtual void sendCommand(Bits commands, int moduleMask) = 0;                                    //! Send command from pattern

  virtual void writeModuleConfig(PixModule& mod) = 0;                                             //! Write module configuration 
  virtual void readModuleConfig(PixModule& mod) = 0;                                              //! Read module configuration 
  virtual void sendModuleConfig(unsigned int moduleMask) = 0;                                     //! Send module configuration 
  virtual void sendPixel(unsigned int moduleMask) = 0;                                            //! send specif. pixel register cfg.
  virtual void sendGlobal(unsigned int moduleMask) = 0;                                           //! send specif. gloabal register cfg.
  virtual void sendPixel(unsigned int moduleMask, std::string regName, bool allDcsIdentical=false) = 0;  //! send pixel register cfg.
  virtual void sendPixel(unsigned int moduleMask, std::string regName, int DC)=0 ;  //! send pixel register cfg. for specific DC
  virtual void sendGlobal(unsigned int moduleMask, std::string regName) = 0;                      //! send gloabal register cfg.

  //DLP: sends the charge calibration for the clusterizer
  virtual void sendPixelChargeCalib(int pModuleID, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge) = 0;

  virtual void setCalibrationMode() = 0;
  virtual void setConfigurationMode() = 0;
  virtual void setRunMode() = 0;
  virtual void setFERunMode() = 0;
  virtual void setFEConfigurationMode() = 0;
  
  virtual void readEPROM() = 0; // sends a global pulse to read the EPROM values to the FE GR
  virtual void burnEPROM() = 0; // burns the FE GR values to the EPROM. Caution, non reversibel!!!
  
  virtual void readGADC(int type, std::vector<int> &GADCvalues, int FEindex) = 0;

  virtual void writeScanConfig(PixScan &scn) = 0;    

  virtual void startScanDelegated(PixScan& scn) = 0;
  void startScan(PixScan* scn);                                                 //! Start a scan

  virtual void finalizeScan()=0;                                                            //! finish undone issues after scan
  virtual void measureEvtTrgRate(PixScan *scn, int mod, double &erval, double &trval) = 0;  //! measure event and trigger rate - not a real occ./ToT-scan!

  virtual bool fitHistos() = 0;                                                                                 //! fit histos
  virtual bool getErrorHistos(unsigned int dsp, Histo* &his) = 0;                                               //! get error arrays 

  virtual void getHisto(HistoType type, unsigned int xmod, unsigned int slv, std::vector< std::vector<Histo*> >& his) = 0; //! Read an histogram

  virtual void getFitResults(HistoType type, unsigned int mod, unsigned int slv, std::vector< Histo * > &thr, std::vector< Histo * > &noise, std::vector< Histo * > &chi2) = 0;                                                  //! Read a Fit from Dsp
  virtual bool moduleActive(int nmod) = 0;                    //! true if a module is active during scan or datataking

  virtual void writeRunConfig(PixRunConfig &cfg) = 0;         //! Get the run configuration parameters from PixModuleGroup
  virtual void startRun(int ntrig = 0) = 0;                   //! Start a run
  virtual void stopRun() = 0;                                 //! Terminates a run
  virtual void pauseRun() = 0;                                 //! Terminates a run
  virtual void resumeRun()= 0;
  virtual int runStatus() = 0;                                //! Check the status of the run
  virtual void stopScan() = 0;                                //! Aborts a running scan
  virtual int nTrigger() = 0;              //! Returns the number of trigger processed so far
  virtual int getBoardID() = 0;

  // new function for MonLeak and HitBusScaler
  virtual void shiftPixMask(int mask, int cap=2, int steps=1) = 0;
  virtual int  readHitBusScaler(int mod, int ife, PixScan* scn) = 0;
  virtual bool checkRxState(rxTypes type) = 0;
  virtual void getServiceRecords(std::string &txt, std::vector<int> &srvCounts) = 0;
  virtual void setAuxClkDiv(int div) = 0;
  virtual void setIrefPads(int bits) = 0;
  virtual void setIOMUXin(int bits) = 0;
  virtual void sendGlobalPulse(int length) = 0;

  virtual bool testGlobalRegister(int module, std::vector<int> &data_in,
      std::vector<int> &data_out, std::vector<std::string> &label, bool sendCfg=true, int feIndex=0) = 0;
  virtual bool testPixelRegister(int module, std::string regName, std::vector<int> &data_in, std::vector<int> &data_out, 
				 bool ignoreDCsOff, int DC=-1, bool sendCfg=true, int feIndex=0, bool bypass=true) = 0;
  virtual bool testScanChain(std::string chainName, std::vector<int> data_in, std::string &data_out, std::string data_cmp, bool shift_only, 
			     bool se_while_pulse, bool si_while_pulse, PixDcs *dcs, double &curr_bef, double &curr_after, int feIndex=0) = 0;

  virtual void hwInfo(std::string &txt) = 0;

  virtual bool getGenericBuffer(const char *type, std::string &textBuf)=0;

  // added from USBpix's source scan mode - leave empty if source scan mode is not supported
  virtual bool getSRAMReadoutReady() = 0;
  virtual bool getSRAMFull() = 0;
  virtual bool getMeasurementPause() = 0;
  virtual bool getMeasurementRunning() = 0;
  virtual int  getSRAMFillLevel() = 0;
  virtual bool getTluVeto() = 0;
  virtual int  getTriggerRate() = 0;
  virtual int  getEventRate() = 0;
  virtual bool getSourceScanData(std::vector<unsigned int *>* data, bool forceReadSram) = 0;


  // Accessors
  PixModuleGroup &getModGroup() { return m_modGroup; };       //! Parent module group accessor
  std::string getCtrlName() { return m_name; };               //! Group name accessor
  Config &config() { return *m_conf; };                       //! Configuration object accessor
  static void listTypes(std::vector<PixControllerInfo> &list);             //! available controller types

  auto getCircularBufferPtr() const -> std::shared_ptr<UintCircBuff1MByte> {
	return m_circularBuffer;
  }

protected:
  Config *m_conf;                  //! Configuration object
  PixModuleGroup &m_modGroup;      //! Pointer to the module group using this controller
  std::shared_ptr<UintCircBuff1MByte> m_circularBuffer = nullptr;

private:
  virtual void configInit() = 0;   //! Init configuration structure
  std::string m_name;              //! Name of the controller
  DBInquire *m_dbInquire;          //! DBInquire
  bool m_inTBMode = false;
};

}

#endif

