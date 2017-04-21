/////////////////////////////////////////////////////////////////////
// PixFeI1.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 10/06/03  Version 1.0 (CS)
//           Initial release
//
// 15/03/04  Version 1.1 (CS)
//           Revised version using PixFeI1Config
//

//! Class for the FE-I1

#ifndef _PIXLIB_PIXFEI1
#define _PIXLIB_PIXFEI1

#include <vector>
#include <string>
#include "PixFe/PixFe.h"


namespace SctPixelRod {
  class VmeInterface;
}

namespace PixLib {
  
  class Bits;
  class Config;
  template<class T> class ConfMask;
  class PixConfDBInterface;
  class DBInquire;
  class PixModule;
  class PixFeConfig;
  class PixFeStructures;
  class PixFeData;
	  
  class PixFeI1 : public PixFe {

    friend class PixModule;
		
  public:
		
    PixFeI1() {} // Default constructor
    PixFeI1(DBInquire *dbInquire, PixModule *mod, std::string name, int number); // Constructor
    virtual ~PixFeI1(); // Destructor
		
    Config &config();                                      // Configuration object accessor
    Config &config(std::string configName);                // Alternate configuration objects accessor
    PixFeConfig &feConfig();                               // Configuration object accessor
    PixFeConfig &feConfig(std::string configName);         // Alternate configuration objects accessor

    int  number();                                          // Return FE number
    void writeGlobRegister(std::string regName, int value); // Write a value in the Global Register mem copy
    int  readGlobRegister(std::string regName);             // Read a value from the Global Register mem copy
    void setGlobRegister();                                 // Copy Global Registers from mem to real device
    void getGlobRegister();                                 // Copy Global Registers from real device to mem
    bool getGlobRegisterLength(std::string regName, int &length); // Get length of register in no. of bits

    void writePixRegister(std::string regName, ConfMask<bool> &value); // Write a value in the Pixel Register mem copy
    ConfMask<bool>& readPixRegister(std::string regName);              // Read a value from the Pixel Register mem copy
    void setPixRegister(std::string regName);                    // Copy Pixel Register from mem to real device
    void getPixRegister(std::string regName);                    // Copy Pixel Register from real device to mem
    void stepPixRegister(std::string regName, int nStep);        // Step a mask pattern

    void writeTrim(std::string trimName, ConfMask<unsigned short int> &value); // Write a value in the Trim mem copy
    ConfMask<unsigned short int>& readTrim(std::string trimName);              // Read a value from the Trim mem copy
    void setTrim(std::string trimName);                    // Copy Trim from mem to real device
    void getTrim(std::string trimName);                    // Copy Trim from real device to mem
    bool getTrimMax(std::string name, int &maximum);       // Get max. value of this trim register

    void configure();                                       // Configure the FE
    bool restoreConfig(std::string configName = "INITIAL"); // Restore a configuration from map (calls configure)
    void storeConfig(std::string configName);               // Store a configuration into map
    void deleteConfig(std::string configName);              // Remove a configuration from the map
    void loadConfig(std::string configName);                // Read the config from DB
    void saveConfig(std::string configName);                // Save the config to DB

    void dump(std::ostream &out); // Dump FE info to output stream
		
    virtual void  setDelayCalib(float){};
    virtual float getDelayCalib(){return 1.;};
    virtual int   getVcalFromCharge(int cap, float charge);

    unsigned int nCol(); // returns no. of columns in chip
    unsigned int nRow(); // returns no. of rows in chip

  protected:

  protected:
				
    // Pointer to corresponding db record
    DBInquire *m_dbInquire;
		
    // Pointer to parent PixModule
    PixModule *m_module;
		
    // FE info
    std::string m_name;
		
    // Command and data structures
    PixFeConfig* m_data;                           // Current FE configuration
    std::map<std::string, PixFeConfig*> m_configs; // Stored FE configurations		
  };
  
}

#endif
