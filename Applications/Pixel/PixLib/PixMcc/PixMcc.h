/////////////////////////////////////////////////////////////////////
// PixMcc.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//

//! Abstract class for the Pixel MCC

#ifndef _PIXLIB_PIXMCC
#define _PIXLIB_PIXMCC

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <string>

namespace PixLib {
  
  class Bits;
  class Config;
  class PixModule;
  
  class PixMcc {
		
    friend class PixModule;
		
  public:
    enum MccOutputSpeed  {SINGLE_LINK_40, DOUBLE_LINK_40, SINGLE_LINK_80, DOUBLE_LINK_80};
	
    virtual ~PixMcc() {}; // Destructor
		
    virtual Config &config()=0; //! Config object accessor

    virtual void writeRegister(std::string regName, int value)=0;    // Write into a register mem copy
    virtual int  readRegister(std::string regName)=0;                // Read a register from mem copy
    virtual void setRegister(std::string regName, int value)=0;      // Set register value into mem copy and device
    virtual int  getRegister(std::string regName)=0;                 // Get register value from device to mem

    virtual void setFifo(std::string fifoName, int value)=0; // Write into a FIFO (directly into device)
    virtual int  getFifo(std::string fifoName)=0;            // Read from a FIFO (directly from device)
		
    virtual void configure()=0;                                       // Configure the MCC
    virtual bool restoreConfig(std::string configName = "INITIAL")=0; // Restore a configuration from map
    virtual void storeConfig(std::string configName)=0;               // Store a configuration into map
    virtual void deleteConfig(std::string configName)=0;              // Remova a config from the map
    virtual void loadConfig(std::string configName)=0;                // Read the config from DB
    virtual void saveConfig(std::string configName)=0;                // Save the config to DB
		
    virtual void writeEnableFE(int feNum)=0;     // Enable a FE (mem copy)
    virtual void setEnableFE(int feNum)=0;       // Enable a FE (mem and device)
    virtual void writeDisableFE(int feNum)=0;    // Disable a FE (mem copy)
    virtual void setDisableFE(int feNum)=0;      // Disable a FE (mem and device)
    virtual void writeEnableAllFE()=0;           // Enable all FEs (mem copy)
    virtual void setEnableAllFE()=0;             // Enable all FEs (mem and device)
    virtual void writeDisableAllFE()=0;          // Disable all FEs (mem copy)
    virtual void setDisableAllFE()=0;            // Disable all FEs (mem and device)
    virtual bool feEnabled(int feNum)=0;         // Check if a FE is enabled (looking at mem copy)

    virtual void writeOutSpeed(MccOutputSpeed speed = SINGLE_LINK_40)=0; // Set MCC output speed (mem copy)
    virtual void setOutSpeed(MccOutputSpeed speed = SINGLE_LINK_40)=0;   // Set MCC output speed (mem and device)
		
    virtual void writeStrobe(int width, int delay, int range, bool calEnable)=0; // Write strobe parameters (mem copy)
    virtual void setStrobe(int width, int delay, int range, bool calEnable)=0;   // Write strobe parameters (mem and device)
		
    virtual void testEventBuilder()=0; // Test the event builder
    virtual void testFifo()=0;         // Test the FIFOs
		
    virtual void globalResetMCC()=0;       // Issue a full MCC reset
    virtual void globalResetFE(int nCK)=0; // Issue a FE reset
    virtual void ecr()=0;                  // Send a ECR command
    virtual void bcr()=0;                  // Send a BCR command
    virtual void syncFE()=0;               // Send a SyncFE command
    virtual void trigger()=0;              // Issue a trigger command
		
    virtual void prepareTestPattern(std::vector<unsigned int> &val, Bits &prep, Bits &exec, Bits &ret)=0;
    virtual void prepareTestEvent(std::vector<unsigned int> &val, Bits &prep, Bits &exec, Bits &ret)=0;

    virtual void enableDataTaking()=0; // Start MCC event builder
		
    virtual void writeFE(const Bits &cmd, int dataLen)=0;           // Send FE config
    virtual void readFE(const Bits &cmd, int dataLen, Bits &out)=0; // Get FE configi

    virtual void dump(std::ostream &out)=0; // Dump MCC info to output stream
		
  private:
    virtual void setRegister(std::string regName)=0; // Set register value from mem copy into device
  };
 
}

#endif
