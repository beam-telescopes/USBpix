#if !defined(LIBGPAC_VOLTAGE_SOURCE_H)
#define LIBGPAC_VOLTAGE_SOURCE_H

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
  namespace blocks
  {
    class voltage_sources;
  }

  namespace channels
  {
    class DllExport voltage_source
    {
      public:
        enum refdes {
          VOUT0 = 0,
          VOUT1,
          VOUT2,
          VOUT3
        };
        
        class unknown_refdes: std::exception {
          public:
            const char* what() const STDE_NOEXCEPT {
              return "The supplied refdes is not supported.";
            }
        };

      private:
        phy &m_phy;
        refdes m_id;

        int m_vdac_id;

        devices::adc_channel m_current_adc;
        devices::adc_channel m_voltage_adc;
        double m_output_voltage;

        double m_current_sense_transimpedance_gain;

        uint16_t voltage_to_dac_value(double voltage);

      public:
        voltage_source(phy &aphy, refdes id, 
            float initial_output_voltage = 0.0);

        void output_voltage(double value);
        double output_voltage() {return m_output_voltage; };
        double capture_current();
        double capture_voltage();
    };
  }

  namespace blocks
  {
    class DllExport voltage_sources
    {
      private:
        phy &m_phy;
        std::vector<channels::voltage_source> m_ch;

      public:
        voltage_sources(phy &aphy);
        channels::voltage_source &operator[] (int ch) { return m_ch.at(ch); };
    };
  }
}

#endif
