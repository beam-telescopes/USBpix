#ifndef __USBI3PIXDCS_H_
#define __USBI3PIXDCS_H_

#include <string>
#include <vector>
#include <map>
#include "PixDcs.h"

class USBPixDCSI3;
using namespace std;

namespace PixLib {

  class DBInquire;
  class Config;
  class USBI3PixController;

  class USBI3PixDcsChan : public PixDcsChan{

    friend class USBI3PixDcs;

    enum ChanType {DVDD, AVDD, LVDD};

  public:
    
    USBI3PixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    USBI3PixDcsChan(USBI3PixDcsChan &chan_in);
    ~USBI3PixDcsChan();
    double ReadParam(std::string);
    void SetParam(std::string,double);
    std::string ReadState(std::string);
    void SetState(std::string);
    
  private:
    void configInit();   //! Init configuration structure

    ChanType m_channelDescr;
  };

  class USBI3PixDcs : public PixDcs {
    friend class USBI3PixDcsChan;
  public:
    
    // Constructor
    USBI3PixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor
    ~USBI3PixDcs();
    
    std::string ReadState(std::string);
    void SetState(std::string);
    bool sendCommand(std::string &/*txt*/, bool /*readResp*/){return false;}; // not implemented, so always return false
    void initHW();
    void ReadError(std::string&);

  protected:
    USBPixDCSI3 *m_USBADC;

  private:
    void configInit();   //! Init configuration structure
    USBI3PixController *m_USBPC;
    bool m_problemInit;
    
};
}


#endif
