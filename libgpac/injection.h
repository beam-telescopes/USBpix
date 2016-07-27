#if !defined(LIBGPAC_INJECTION_H)
#define LIBGPAC_INJECTION_H

#include "phy.h"
#include <exception>
#include <vector>
#include <dllexport.h>

namespace gpac 
{
  namespace channels
  {
    class DllExport injection 
    {
      private:
        phy &m_phy;

        int m_vdac_high;
        int m_vdac_low;

        double m_gain;

        double m_high_voltage;
        double m_low_voltage;

        void commit();

      public:
        injection(phy &aphy);

        void high_voltage(double value);
        double high_voltage() {return m_high_voltage; };

        void low_voltage(double value);
        double low_voltage() {return m_low_voltage; };

        void gain(double value);
        double gain() {return m_gain; };
    };
  }
}

#endif

