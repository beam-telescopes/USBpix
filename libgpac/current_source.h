#if !defined(LIBGPAC_CURRENT_SOURCE_H)
#define LIBGPAC_CURRENT_SOURCE_H

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
    class current_sources;
  }

  namespace channels
  {
    class DllExport current_source
    {
      public:
        enum refdes {
          IOUT0 = 0,
          IOUT1,
          IOUT2,
          IOUT3,
          IOUT4,
          IOUT5,
          IOUT6,
          IOUT7,
          IOUT8,
          IOUT9,
          IOUT10,
          IOUT11
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
        double m_output_current;

        double m_transfer_factor;
        double m_sense_transfer_factor;
        double m_Vref;

        uint16_t current_to_dac_value(double current);

      public:
        current_source(phy &aphy, refdes id, 
            float initial_output_current = 0.0);

        void output_current(double value);
        double output_current() {return m_output_current; };
        double capture_current();
        double capture_voltage();
    };
  }
  
  namespace blocks
  {
    class DllExport current_sources
    {
      private:
        phy &m_phy;
        std::vector<channels::current_source> m_ch;

      public:
        current_sources(phy &aphy);
        channels::current_source &operator[] (int ch) { return m_ch.at(ch); };
    };
  }
}
#endif
