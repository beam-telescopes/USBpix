#ifndef __GPIBPIXDCS_H_
#define __GPIBPIXDCS_H_

#include <string>
#include <vector>
#include "PixDcs.h"

using namespace std;
class PixGPIBDevice;

namespace PixLib {

  class DBInquire;
  class Config;
  class GPIBPixDcs;

  class GPIBPixDcsChan : public PixDcsChan{
    
    friend class GPIBPixDcs;

  public:
    
    GPIBPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    GPIBPixDcsChan(GPIBPixDcsChan &chan_in);
    ~GPIBPixDcsChan();
    double ReadParam(std::string);
    void SetParam(std::string,double);
    void SetParam(std::string);
    std::string ReadState(std::string);
    void SetState(std::string);

  protected:
    int m_channelID;
    float m_nomVolts, m_nomVoltOffset, m_currLim, m_rampVoltSteps, m_frequency;
    int m_rampTime, m_nBurstPulses, m_waveFormType;
    float m_measRange;
    bool m_autoRange;

    float m_OCPCurrent;
    bool  m_OCPEnabled;
    float m_OVPVoltage;
    bool  m_OVPEnabled;
    bool  m_RemoteSensing;

  private:
    void configInit();   //! Init configuration structure
    void rampVoltage(double pVoltage);	//ramps the coltage of the channel

  };

  class GPIBPixDcs : public PixDcs {

    friend class GPIBPixDcsChan;

  public:
    
    // Constructor
    GPIBPixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor
    ~GPIBPixDcs();
    
    std::string ReadState(std::string);
    void SetState(std::string);
    void initHW();
    bool sendCommand(std::string &txt, bool readResp);
    void ReadError(std::string&);

  protected:
    PixGPIBDevice* m_gpibDev;

  private:
    void configInit();   //! Init configuration structure
    int m_boardID, m_gpibChan;

};
}


#endif
