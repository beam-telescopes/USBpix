/////////////////////////////////////////////////////////////////////
// PixMccI1.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//

//! Class for the MCC-I1

#ifndef _PIXLIB_PIXMCCI1
#define _PIXLIB_PIXMCCI1

#include <vector>
#include <string>
#include "PixMcc/PixMcc.h"



namespace SctPixelRod {
  class VmeInterface;
}


namespace PixLib {
  
  class Bits;
  class Config;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixMccStructures;
  class PixMccData;
  class PixMccConfig;
  
  class PixMccI1 : public PixMcc {
	
    friend class PixModule;

  public:
		    
    PixMccI1() {} // Default constructor
    PixMccI1(DBInquire *dbInquire, PixModule *mod, std::string name); // Constructor
    virtual ~PixMccI1(); // Destructor
		
    Config &config(); // Configuration object accessor

    void writeRegister(std::string regName, int value); // Write into a register mem copy
    int  readRegister(std::string regName);             // Read a register from mem copy
    void setRegister(std::string regName, int value);   // Set register value into mem copy and device
    int  getRegister(std::string regName);              // Get register value from device to mem

    void setFifo(std::string fifoName, int value); // Write into a FIFO (directly into device)
    int  getFifo(std::string fifoName);            // Read from a FIFO (directly from device)
		
    void configure();                                       // Configure the MCC
    bool restoreConfig(std::string configName = "INITIAL"); // Restore a configuration from map
    void storeConfig(std::string configName);               // Store a configuration into map
    void deleteConfig(std::string configName);              // Remova a config from the map
    void loadConfig(std::string configName);                // Read the config from DB
    void saveConfig(std::string configName);                // Save the config to DB
		
    void writeEnableFE(int feNum);     // Enable a FE (mem copy)
    void setEnableFE(int feNum);       // Enable a FE (mem and device)
    void writeDisableFE(int feNum);    // Disable a FE (mem copy)
    void setDisableFE(int feNum);      // Disable a FE (mem and device)
    void writeEnableAllFE();           // Enable all FEs (mem copy)
    void setEnableAllFE();             // Enable all FEs (mem and device)
    void writeDisableAllFE();          // Disable all FEs (mem copy)
    void setDisableAllFE();            // Disable all FEs (mem and device)
    bool feEnabled(int feNum);         // Check if a FE is enabled (looking at mem copy)
	
    void writeOutSpeed(MccOutputSpeed speed = SINGLE_LINK_40); // Set MCC output speed (mem copy)
    void setOutSpeed(MccOutputSpeed speed = SINGLE_LINK_40);   // Set MCC output speed (mem and device)
		
    void writeStrobe(int width, int delay, int range, bool calEnable); // Write strobe parameters (mem copy)
    void setStrobe(int width, int delay, int range, bool calEnable);   // Write strobe parameters (mem and device)
		
    void testEventBuilder(); // Test the event builder
    void testFifo();         // Test the FIFOs
		
    void globalResetMCC();       // Issue a full MCC reset
    void globalResetFE(int nCK); // Issue a FE reset
    void ecr();                  // Send a ECR command
    void bcr();                  // Send a BCR command
    void syncFE();               // Send a SyncFE command
    void trigger();              // Issue a trigger command
		
    void enableDataTaking(); // Start MCC event builder
		
    void prepareTestPattern(std::vector<unsigned int> &val, Bits &prep, Bits &exec, Bits &ret);
    void prepareTestEvent(std::vector<unsigned int> &val, Bits &prep, Bits &exec, Bits &ret);

    void writeFE(const Bits &cmd, int dataLen);           // Send data to FE
    void readFE(const Bits &cmd, int dataLen, Bits &out); // Get data from FE
		
    void dump(std::ostream &out); // Dump MCC info to output stream
		
  protected:

    void setRegister(std::string regName); // Set register value from mem copy into device

  protected:
				
    // Pointer to corresponding db record
    DBInquire *m_dbInquire;
		
    // Pointer to parent PixModule
    PixModule *m_module;
		
    // MCC name
    std::string m_name;
		
    // Command and data structures
    PixMccConfig* m_data;                           // Current MCC configuration
    std::map<std::string, PixMccConfig*> m_configs; // Stored MCC configurations		
  };
  
}

#endif
