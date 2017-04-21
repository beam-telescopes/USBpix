/////////////////////////////////////////////////////////////////////
// PixModule.h
// version 1.0.1
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//
// 13/04/03  Version 1.0.1 (CS)
//           First concrete implementation
//

//! Class for the Pixel Module

#ifndef _PIXLIB_MODULE
#define _PIXLIB_MODULE

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <string>
#include <map>

#include "BaseException.h"

namespace SctPixelRod {
  class RodPrimitive;
}

namespace PixLib {
  
  class PixController;
  class RodPixController;
  class PixConfDBInterface;
  class DBInquire;
  class PixModuleGroup;
  class PixMcc;
  class PixFe;
  class Config;

  //! Pix Module Exception class; an object of this type is thrown in case of a module error
  class PixModuleExc : public SctPixelRod::BaseException {
  public:
    enum ErrorLevel{INFO, WARNING, ERROR, FATAL};
    //! Constructor
    PixModuleExc(ErrorLevel el, std::string name) : BaseException(name), m_errorLevel(el), m_name(name) {}; 
    //! Destructor
    virtual ~PixModuleExc() {}; 
 
    //! Dump the error
    virtual void dump(std::ostream &out) {
      out << "Pixel Module " << m_name << " -- Level : " << dumpLevel(); 
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
  
  class PixModule {
    friend class PixModuleGroup;
  public:
    enum AssemblyType { NONE, STAVE, SECTOR };
    enum PP0Type { OPTICAL, OPTICAL_TEST, ELECTRICAL, USB };
    enum FEFlavour{PM_FE_I1, PM_FE_I2, PM_FE_I4A, PM_FE_I4B};
    enum MCCFlavour {PM_MCC_I1, PM_MCC_I2, PM_NO_MCC};

    typedef std::vector<PixFe*>::iterator feIterator;
    
    PixModule(DBInquire *dbInquire, PixModuleGroup *modGrp, std::string name); // Constructor
    PixModule(PixModuleGroup *modGrp, unsigned int id, std::string name, std::string feType, 
	      int nFe, int nFeRows, std::string mccType);                      // custom-made constructor
    ~PixModule();                                                              // Destructor
    
    // Accessors
    std::string moduleName() {return m_name;}          // Name of the module 
    PixModuleGroup *getPixModGroup() {return m_group;}    // Pointer to Module Group
    PixMcc *pixMCC() {return m_mcc;}                      // Pointer to MCC
    PixFe  *pixFE(int nFE);                               // Pointers to FEs
    feIterator feBegin() { return m_fe.begin(); };
    feIterator feEnd()   { return m_fe.end(); };
    Config &config() { return *m_conf; };
   
    // Configuration
    //void loadConfig(std::string configDir, std::string configName); // Load a configuration from DB
    //void saveConfig(std::string configName);                        // Save current configuration to DB

    void loadConfig(std::string name);             // Load a configuration from DB
    void saveConfig(std::string name);             // Save current configuration to DB
    void storeConfig(std::string name);            // Save a configuration in the map
    bool restoreConfig(std::string name);          // Restore a configuration from the map
    void deleteConfig(std::string name);           // Remove a configuration from the map
    void setConfig(int structId, int moduleNum);   // Send configuration data to the enabled modules
    void reset(int type);                          // reset entire module; 0: soft, 1: hard pulse, 2: hard on, 3: hard off

    // Run control
    //void startHistoRun(int nEv, std::string histoName, int histoBin2) {} // Start a run with histogram output
    void startHistoRun(int , std::string , int ) {}             // Start a run with histogram output
    void startVmeRun(int ) {}                                            // Start a run with VME output
    void startSlinkRun(int ) {}                                          // Start a run with S-Link output
    void stopRun() {}                                                    // Stop a run
    int  nEvTaken() {                                                    // Return the number of events taken so far
      return 0;
    }
    // Histogram handling
    //histo getHisto(std::string histoName) {}   // Read an histogram
    
    // Event handling
    bool hitReady() {  // Test for pending hits
      return false;
    }
    int getHit() {     // Read an hit via VME
      return 0;
    }
    
    // Scans
    //    void thresholdScan(int nEv, int nStep, int vcalMin, int vcalMax, std::string histoName) {} // Perform a threshold scan
    void thresholdScan(int , int , int , int , std::string ) {}                                // Perform a threshold scan
    void tuneTDACScan(int , std::string ) {}                                                   // Start a TDAC tuning procedure
    void tuneFDACScan(int , std::string ) {}                                                   // Start a FDAC tuning procedure
    void monleakScan(int ) {}                                                                  // Start a leakage current measurement

    //Accessors
    int moduleId() { return m_moduleId; }
    int groupId() { return m_groupId; }

    // FE geometry
    unsigned int iColMod(unsigned int iFe, unsigned int col);
    unsigned int iRowMod(unsigned int iFe, unsigned int row);
    unsigned int nColsFe();
    unsigned int nRowsFe();
    unsigned int nColsMod();
    unsigned int nRowsMod();

  private:

    void createConfig(DBInquire *dbInquire);

    // Module name
    std::string m_name;
    
    // Pointer to parent Pixel Module Group
    PixModuleGroup* m_group;
    
    // Pointer to child  MCC
    PixMcc* m_mcc;
    
    // Pointers to child FEs
    std::vector<PixFe*> m_fe;
    // to manage FE geometry
    unsigned int m_feRows;
    
    // Configuration structure    
    Config *m_conf;
    
    // Link Parameters
    int m_inputLink;
    int m_outputLink1;
    int m_outputLink2;
    int m_outputLink3;
    int m_outputLink4;
    int m_bocInputLink;
    int m_bocOutputLink1;
    int m_bocOutputLink2;
    int m_delay;
    int m_latency;
    int m_outputBandwidth;
    int m_moduleId;
    int m_groupId;
    int m_active;
    int m_present;
    int m_mccFlavour;
    int m_feFlavour;
    AssemblyType m_geomType;
    int m_geomPosition;
    int m_geomAssemblyId;
    PP0Type m_pp0Type;
    int m_pp0Position;

    // Modules properties during scan
    bool m_configActive;
    bool m_triggerActive;
    bool m_strobeActive;
    bool m_readoutActive;

    // flavour lists
    std::map<std::string, int> m_mccfl;
    std::map<std::string, int> m_fefl;
    
  };
  
}

#endif
