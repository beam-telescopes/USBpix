#if !defined(LIBGPAC_POWER_SUPPLY_H)
#define LIBGPAC_POWER_SUPPLY_H

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
    class power_supplies;
  }

  namespace channels
  {
    class DllExport power_supply
    {
      public:
        enum refdes {PWR0, PWR1, PWR2, PWR3};
        class unknown_refdes: std::exception {
          public:
            const char* what() const STDE_NOEXCEPT {
              return "The supplied refdes is not supported.";
            }
        };

      private:
        phy &m_phy;
        refdes m_id;
        blocks::power_supplies &m_parent;

        int m_vdac_id;

        devices::adc_channel m_voltage_adc;
        devices::adc_channel m_current_adc;

        double m_output_voltage;
        bool m_enabled;
        bool m_overcurrent;

      private:
        double m_Rs1;
        double m_Rs2;
        double m_Rs3;

        uint16_t voltage_to_dac_value(double voltage);

      public:
        power_supply(phy &aphy, refdes id, blocks::power_supplies& parent, 
            float initial_output_voltage = 0.0);

        void output_voltage(double value);
        double output_voltage() {return m_output_voltage; };
        void enabled(bool en, bool update = true);
        bool enabled() {return m_enabled; };
        void force_oc_state(bool state) {m_overcurrent = state; };
        bool overcurrent(bool update = true);
        double capture_voltage();
        double capture_current();
    };
  }

  namespace blocks
  {
    class DllExport power_supplies
    {
      private:
        phy &m_phy;
        std::vector<channels::power_supply> m_ch;

        double m_current_limit;
        uint16_t current_to_dac_value(double current);

        int m_current_vdac_id;
        double m_Rsns;
        double m_current_sense_gain;

      public:
        power_supplies(phy &aphy);
        channels::power_supply &operator[] (int ch) { return m_ch.at(ch); };
        void update_enabled();
        void update_overcurrent();
        void current_limit(double value);
        double current_limit() {return m_current_limit; };
        double current_sense_transimpedance_gain()
        {
          return m_Rsns * m_current_sense_gain;
        }
    };
  }
}

#endif
