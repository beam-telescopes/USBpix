#if !defined(LIBGPAC_I2CHOST_H)
#define LIBGPAC_I2CHOST_H

#include <vector>
#include <memory>
#include <cstdlib>
#include <exception>
#include <dllexport.h>

#ifdef WIN32
  #if _MSC_VER >= 1700
    #include <stdint.h>
  #else
    #include "inttypes_win32.h"
  #endif
#else
  #include <inttypes.h>
#endif

class SiUSBDevice;

namespace gpac {
  class DllExport i2chost {
    public:
      virtual uint8_t read8(uint8_t addr) = 0;
      virtual std::unique_ptr<std::vector<uint8_t>> 
        read(uint8_t addr, size_t len) = 0;
      virtual bool write8(uint8_t addr, uint8_t data) = 0;
      virtual bool write(uint8_t addr, const std::vector<uint8_t> data) = 0;
  };
  
  class DllExport dummy_i2chost: public i2chost {
    public:
      uint8_t read8(uint8_t addr);
      std::unique_ptr<std::vector<uint8_t>> read(uint8_t addr, size_t len);
      bool write8(uint8_t addr, uint8_t data);
      bool write(uint8_t addr, const std::vector<uint8_t> data);
  };
  
  class DllExport usbpix_i2chost: public i2chost {
    private:
      SiUSBDevice *m_dev;

    public:
      usbpix_i2chost(SiUSBDevice *dev);
      uint8_t read8(uint8_t addr);
      std::unique_ptr<std::vector<uint8_t>> read(uint8_t addr, size_t len);
      bool write8(uint8_t addr, uint8_t data);
      bool write(uint8_t addr, const std::vector<uint8_t> data);
      void updateDev(SiUSBDevice *dev) {m_dev = dev;};
  };

  class DllExport i2chost_exception: public std::exception {
  };
}
#endif
