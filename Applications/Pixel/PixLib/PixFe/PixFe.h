/////////////////////////////////////////////////////////////////////
// PixFe.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 02/07/03  Version 1.0 (CS)
//           Initial release
//

//! Abstract class for the Pixel FE

#ifndef _PIXLIB_PIXFE
#define _PIXLIB_PIXFE

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <string>

#include "PixFe/PixFeConfig.h"

namespace PixLib {
  
  class Bits;
  class Config;
  template<class T> class ConfMask;
  class PixModule;

  class PixFe {

    friend class PixModule;

  public:

    virtual ~PixFe() {}; // Destructor
                  
    virtual Config &config()=0;                                // Configuration object accessor
    virtual Config &config(std::string configName) = 0;        // Alternate configuration objects accessor
    virtual PixFeConfig &feConfig()=0;                         // Configuration object accessor
    virtual PixFeConfig &feConfig(std::string configName) = 0; // Alternate configuration objects accessor

    virtual int  number()=0; // Return FE number

    virtual void writeGlobRegister(std::string regName, int value)=0; // Write a value in the Global Register mem copy
    virtual int  readGlobRegister(std::string regName)=0;             // Read a value from the Global Register mem copy
    virtual void setGlobRegister()=0;                                 // Copy Global Registers from mem to real device
    virtual void getGlobRegister()=0;                                 // Copy Global Registers from real device to mem
    virtual bool getGlobRegisterLength(std::string regName, int &length)=0; // Get length of register in no. of bits

    virtual void writePixRegister(std::string regName, ConfMask<bool> &value)=0; // Write a value in the Pixel Register mem copy
    virtual ConfMask<bool>& readPixRegister(std::string regName)=0;              // Read a value from the Pixel Register mem copy
    virtual void setPixRegister(std::string regName)=0;                    // Copy Pixel Register from mem to real device
    virtual void getPixRegister(std::string regName)=0;                    // Copy Pixel Register from real device to mem
    virtual void stepPixRegister(std::string regName, int nStep)=0;        // Step a mask pattern

    virtual void writeTrim(std::string trimName, ConfMask<unsigned short int> &value)=0; // Write a value in the Trim mem copy
    virtual ConfMask<unsigned short int>& readTrim(std::string trimName)=0;              // Read a value from the Trim mem copy
    virtual void setTrim(std::string trimName)=0;                    // Copy Trim from mem to real device
    virtual void getTrim(std::string trimName)=0;                    // Copy Trim from real device to mem
    virtual bool getTrimMax(std::string name, int &maximum)=0;         // Get max. value of this trim register


    virtual void configure()=0;                                       // Configure the FE
    virtual bool restoreConfig(std::string configName = "INITIAL")=0; // Restore a configuration from map
    virtual void storeConfig(std::string configName)=0;               // Store a configuration into map
    virtual void deleteConfig(std::string configName)=0;              // Remova a config from the map
    virtual void loadConfig(std::string configName)=0;                // Read the config from DB
    virtual void saveConfig(std::string configName)=0;                // Save the config to DB

    virtual void dump(std::ostream &out)=0; // Dump FE info to output stream
		
    virtual void  setDelayCalib(float cal)=0;
    virtual float getDelayCalib()=0;
    virtual int   getVcalFromCharge(int cap, float charge) = 0;

    virtual unsigned int nCol()=0; // returns no. of columns in chip
    virtual unsigned int nRow()=0; // returns no. of rows in chip

  private:

  };
  
}

#endif
