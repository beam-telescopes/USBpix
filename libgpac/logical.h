#if !defined(LIBGPAC_LOGICAL_H)
#define LIBGPAC_LOGICAL_H
  
class SiUSBDevice;

#include <exception>
#include <vector>

#include <dllexport.h>

#include "phy.h"
#include "power_supply.h"
#include "voltage_source.h"
#include "current_source.h"
#include "injection.h"
#include "aux_adc.h"

namespace gpac 
{
  class DllExport logical
  {
    private:
      SiUSBDevice *m_dev;
      phy m_phy;
      
    public:
      logical(SiUSBDevice *dev);

      blocks::power_supplies power_supplies;
      blocks::current_sources current_sources;
      blocks::voltage_sources voltage_sources;
      blocks::aux_adcs aux_adcs;
      channels::injection injection;

      void updateDev(SiUSBDevice *dev) {m_phy.updateDev(dev);};
  };
}

#endif
