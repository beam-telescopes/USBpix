#if !defined(LIBGPAC_AUX_ADC_H)
#define LIBGPAC_AUX_ADC_H

#include "phy.h"
#include <exception>
#include <vector>
#include <dllexport.h>

#ifdef WIN32
  #define STDE_NOEXCEPT
#else
  #define STDE_NOEXCEPT noexcept
#endif


namespace gpac
{
  namespace channels
  {
    class DllExport aux_adc
    {
      public:
        enum DllExport refdes {
          AUX_ADC0 = 0,
          AUX_ADC1,
          AUX_ADC2,
          AUX_ADC3,
        };
        
        class DllExport unknown_refdes: std::exception {
          public:
            const char* what() const STDE_NOEXCEPT {
              return "The supplied refdes is not supported.";
            }
        };

      private:
        phy &m_phy;
        refdes m_id;

        devices::adc_channel m_adc_channel;
      
      public:
        aux_adc(phy& aphy, refdes id);
        double capture();
        double capture_voltage() {return capture(); };
    };
  }

  namespace blocks
  {
    class DllExport aux_adcs
    {
      private:
        phy &m_phy;
        std::vector<channels::aux_adc> m_ch;

      public:
        aux_adcs(phy &aphy);
        channels::aux_adc &operator[] (int ch) { return m_ch.at(ch); };
    };
  }

}
#endif
