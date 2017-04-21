#ifndef __RS232PIXDCS_H_
#define __RS232PIXDCS_H_

#include <string>
#include <vector>
#include "PixDcs.h"

using namespace std;
class PixRs232Device;

namespace PixLib {

  class DBInquire;
  class Config;
  class RS232PixDcs;

  class RS232PixDcsChan : public PixDcsChan{
    
    friend class RS232PixDcs;

  public:
    
    RS232PixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    RS232PixDcsChan(RS232PixDcsChan &chan_in);
    ~RS232PixDcsChan();
    double ReadParam(std::string);
    void SetParam(std::string,double);
    void SetParam(std::string);
    std::string ReadState(std::string);
    void SetState(std::string);

  protected:
    int m_channelID;
    float m_nomVolts, m_currLim, m_rampVoltSteps;
    int m_rampTime;
    int m_maxSpeed, m_position;
    float m_measRange;
    bool m_autoRange;
    bool m_externalTempSensor;
    float m_nomTemp;

  private:
    void configInit();   //! Init configuration structure
    void rampVoltage(double pVoltage);	//ramps the voltage of the channel

  };

  class RS232PixDcs : public PixDcs {

    friend class RS232PixDcsChan;

  public:
    
    // Constructor
    RS232PixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor
    ~RS232PixDcs();
    
    std::string ReadState(std::string);
    void SetState(std::string);
    void initHW();
    bool sendCommand(std::string &txt, bool readResp);
    void ReadError(std::string&);

  protected:
    PixRs232Device* m_rs232Dev;

  private:
    void configInit();   //! Init configuration structure
    int m_port, m_rs232Chan;

};
}


#endif
