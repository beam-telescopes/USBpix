#include "injection.h"
#include "phy.h"

using namespace gpac;



channels::injection::injection(phy &aphy):
  m_phy(aphy),
  m_vdac_high(22),
  m_vdac_low(21),
  m_gain(1.0),
  m_high_voltage(0.0),
  m_low_voltage(0.0)
{

}

void channels::injection::commit()
{
  gpac::devices::dac_block & da = m_phy.core().dac();

  uint16_t dac_value_high = da.voltage_to_dac_value(m_high_voltage / m_gain);
  uint16_t dac_value_low  = da.voltage_to_dac_value(m_low_voltage  / m_gain);

  da.update(m_vdac_high, dac_value_high);
  da.update(m_vdac_low,  dac_value_low);
}

void channels::injection::high_voltage(double value)
{
  m_high_voltage = value;
  commit();
}

void channels::injection::low_voltage(double value)
{
  m_low_voltage = value;
  commit();
}
        
void channels::injection::gain(double value)
{
  m_gain = value;
  commit();
}
