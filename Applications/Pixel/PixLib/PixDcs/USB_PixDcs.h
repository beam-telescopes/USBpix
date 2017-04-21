#ifndef __USBPIXDCS_H_
#define __USBPIXDCS_H_

#include <string>
#include <vector>
#include <map>
#include "PixDcs.h"

class USBPixDCS;
using namespace std;

namespace PixLib {

  class DBInquire;
  class Config;
  class USBPixController;

  class USBPixDcsChan : public PixDcsChan{

    friend class USBPixDcs;

  public:
    
    USBPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    USBPixDcsChan(USBPixDcsChan &chan_in);
    virtual ~USBPixDcsChan();
    virtual double ReadParam(std::string);
    virtual void SetParam(std::string,double);
    virtual void SetParam(string varType);
    virtual std::string ReadState(std::string);
    virtual void SetState(std::string);

  protected:
    int m_channelDescr;
    float m_nomVolts;
    bool m_chanIsOn;
    int m_feAdcType;
    int m_feIndex;

  private:
    virtual void configInit();   //! Init configuration structure

  };

  class USBPixDcs : public PixDcs {
    friend class USBPixDcsChan;
    friend class USBBIPixDcsChan;
  public:
    
    // Constructor
    USBPixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor
    virtual ~USBPixDcs();
    
    virtual std::string ReadState(std::string);
    virtual std::string ReadState2(std::string);
    virtual void SetState(std::string);
    virtual void initHW();
    bool sendCommand(std::string &, bool){return false;}; // not implemented, so always return false
    USBPixDCS *getHwPtr(){return m_USBADC;};
    USBPixController *getCtrl(){return m_USBPC;};
    virtual void ReadError(std::string&);

  protected:
    USBPixDCS *m_USBADC;
    USBPixController *m_USBPC;
    bool m_problemInit;

  private:
    virtual void configInit();   //! Init configuration structure
    
};
}


#endif
