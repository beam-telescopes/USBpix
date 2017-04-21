#ifndef __GOEUSBPIXDCS_H_
#define __GOEUSBPIXDCS_H_

#ifdef HAVE_GOEUSB
#include <string>
#include <vector>
#include "PixDcs.h"

struct usb_dev_handle;

/* classes GoeUSBPixDcs and GoeUSBPixDcsChan 
 * for handling of Goettingen's USB probe card
 * author Joern Grosse-Knetter, 23.11.2009
 *
 * needs libusb drivers and libraries to be downloaded from 
 * http://libusb-win32.sourceforge.net (Windows) or
 * http://libusb.sourceforge.net (Linux) and set
 * GOEUSBDIR to the base directory of lib and include
 * directories and GOEUSB_FLAG to /DHAVE_GOEUSB to have
 * this class included in PixLib
 */

namespace PixLib {

  class GoeUSBPixDcsChan : public PixDcsChan{
    
    friend class GoeUSBPixDcs;

  public:
    
    GoeUSBPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    //GoeUSBPixDcsChan(GoeUSBPixDcsChan &chan_in);
    ~GoeUSBPixDcsChan();
    double ReadParam(std::string);
    void SetParam(std::string,double);
    void SetParam(std::string);

  protected:
    int m_channelID;

  private:
    void configInit();   //! Init configuration structure

    int m_channelDescr, m_samples;
    float m_grad, m_offset;

  };

  class GoeUSBPixDcs : public PixDcs {

    friend class GoeUSBPixDcsChan;

  public:
    
    // Constructor
    GoeUSBPixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor
    ~GoeUSBPixDcs();
    
    std::string ReadState(std::string);
    void SetState(std::string);
    void initHW();
    bool sendCommand(std::string &, bool){return false;}; // not implemented, so always return false

  private:
    void configInit();   //! Init configuration structure
    struct usb_dev_handle *m_handle;
    bool m_problemInit;
};
}

#endif // HAVE_GOEUSB

#endif GOEUSBPIXDCS_H
