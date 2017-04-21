#ifndef __USBPIXDCSBI_H_
#define __USBPIXDCSBI_H_

#include <string>
#include <vector>
#include <map>
#include "USB_PixDcs.h"

class USBPixDCS;
using namespace std;

namespace PixLib {

  class DBInquire;
  class Config;
  class USBPixController;

  class USBBIPixDcsChan : public USBPixDcsChan{

    friend class USBBIPixDcs;

  public:
    
    USBBIPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    USBBIPixDcsChan(USBBIPixDcsChan &chan_in);
    // Destructor - use that from parent class
    //    ~USBBIPixDcsChan();
    std::string ReadState(std::string);

  private:
    void configInit();   //! Init configuration structure

  };

  class USBBIPixDcs : public USBPixDcs {
    friend class USBBIPixDcsChan;
  public:
    
    // Constructor
    USBBIPixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor - can't use that from parent class
    ~USBBIPixDcs();
    
    std::string ReadState(std::string);
    void SetState(std::string);
    void initHW();
    float getTempLim(){return m_tempLim;};

  private:
    void configInit();   //! Init configuration structure
    float  m_currLim;
    float  m_tempLim;
    
};
}


#endif
