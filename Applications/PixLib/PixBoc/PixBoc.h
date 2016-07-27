/*--------------------------------------------------------------*
 *  PixBoc.h                                                    *
 *  Version:  1.0                                               *
 *  Created:  03 March 2004                                     *
 *  Author:   Tobias Flick                                      *
 *  Modifications: Iris Rottlaender, June 2005                  * 
 *                 Tobias Flick, July 2005                      *
 *                                                              *
 *  Interface to the PixelBoc                                   *
 *--------------------------------------------------------------*/

#ifndef _PIXLIB_PIXBOC
#define _PIXLIB_PIXBOC

#include <processor.h>
#include "Config/Config.h"
#include "Config/ConfObj.h"

//#include "PixConfDBInterface/RootDB.h"
#include <string>

namespace PixLib { 
  
class DBInquire;
class PixModuleGroup;
class Config;
//class PixTx;
//class PixRx;

//------------------------- Class PixBox--------------------------
// The class contains the inteface between the BocCard owned by 
// PixModuleGroup and accessed through RodPixController. 
// The class has two subclasses: PixTx and PixRx. All 
// accessfunctions are part of the class which should be used 
// from the user outside PixLib.

class PixBoc {
public:

  class PixTx {
  public: 
    //Constructor:
    PixTx(PixBoc &pixboc, DBInquire *dbInquire);
   
    //Destructor
    ~PixTx(){};

    //Function to get config object
    Config* getConfigTx(){return m_conf;};


    //Functions to set values to specific TX registers
    void setTxFineDelay(const int channel, const UINT32 value);
    void setTxCoarseDelay(int channel, UINT32 value);
    void setTxStreamInhibit(int channel, UINT32 value);
    void setTxMarkSpace(int channel, UINT32 value);
    void setTxLaserCurrent(int channel, UINT32 value);

    //Accessors
    void getTxLaserCurrent(int channel, UINT32 buffer[], int numChannels);
    void getTxFineDelay(int channel, UINT32 buffer[], int numChannels);
    void getTxCoarseDelay(int channel, UINT32 buffer[], int numChannels);
    void getTxStreamInhibit(int channel, UINT32 buffer[], int numChannels);
    void getTxMarkSpace(int channel, UINT32 buffer[], int numChannels);
    UINT32 getTxMarkSpace(int channel){return m_bpmMarkSpace[channel];}
    UINT32 getTxCoarseDelay(int channel){return m_bpmCoarseDelay[channel];}
    UINT32 getTxFineDelay(int channel){return m_bpmFineDelay[channel];}
    UINT32 getTxStreamInhibit(int channel){return m_bpmStreamInhibit[channel];}
    UINT32 getTxLaserCurrent(int channel){return m_LaserCurrent[channel];}
    PixBoc &getPixBoc(){return m_pixboc;};
 
  protected:
    Config *m_conf;

  private:
    PixBoc &m_pixboc;
    DBInquire *m_dbInquireTx;
    void configInit();

  public:
    
    int m_plugin;
    int m_bpmFineDelay[8];
    int m_bpmCoarseDelay[8];
    int m_bpmStreamInhibit[8];
    int m_bpmMarkSpace[8];
    int m_LaserCurrent[8];
  };

  class PixRx {
  public: 
    //Constructor
    PixRx(PixBoc &pixboc, DBInquire *dbInquire);

    //Destructor
    ~PixRx(){};

    //Funktion to get config object
    
    Config* getConfigRx(){return m_conf;};

    //Functions to set values to specific RX registers
    void setRxDataDelay(int channel, UINT32 value);
    void setRxThreshold(int channel, UINT32 value);

    //Accessors
    UINT32 getRxDataDelay(int channel){return m_dataDelay[channel];};
    void getRxDataDelay(int channel, UINT32 buffer[], int numChannels);
    UINT32 getRxThreshold(int channel){return m_threshold[channel];};
    void getRxThreshold(int channel, UINT32 buffer[], int numChannels);
    PixBoc &getPixBoc(){return m_pixboc;};

  protected:
    Config *m_conf;
    
  public:
    int m_plugin;
    int m_dataDelay[8];
    int m_threshold[8];

  private:
    PixBoc &m_pixboc;
    DBInquire *m_dbInquireRx;
    void configInit();
  };


  //Constructor
  PixBoc(PixModuleGroup &modGrp, DBInquire *dbInquire);

  //Destructor
  ~PixBoc();

  //Functions to set values to specific BOC registers
  void BocReset();                  //Reset the BOC
  void BocInit();                   //Initialize the BOC
  void BocConfigure();              //Load configdata to the BOC
  void BocSetValue();               //Set individual values to the BOC
  void BocMonitor(double buffer[]); //Get monitored data from the BOC
  void resetBocMonitor();           //Reset Monitor ADCs
  void getBocStatus();              //get the BOC status
  void scanDataDelay();             //Scan the RX data delay
  void showBocConfig();             //prints out the config of the BOC

  //Accessors
  PixModuleGroup &getModGroup() { return m_modGroup; };
  std::string getCtrlName() { return m_name; };
  PixTx* getTx(int nr){return m_TxSection[nr];};
  PixRx* getRx(int nr){return m_RxSection[nr];};
  Config* getConfig(){return m_conf;};
  // DBInquire* getDBInquire(){return m_dbInquire;};
  
 protected:
  Config *m_conf;

private:
  void configInit();
  bool m_BocCfgEdited();

  PixTx* m_TxSection[4];          //! Pointer to the TX section
  PixRx* m_RxSection[4];          //! Pointer to the RX section
  PixModuleGroup &m_modGroup;  //! Pointer to the module group using this PixBoc
  std::string m_name;          //! Name of the controller
  DBInquire *m_dbInquire;      //! DBInquire
  
  int m_mode;
  int m_breg;
  int m_clockControl;
  int m_bpmClock;
  int m_v1Clock;
  int m_v2Clock;
  int m_vFineClock;
  int m_bocMode;
};

} //end of namespace PixLib

#endif      //_PIXLIB_PIXBOC
 
