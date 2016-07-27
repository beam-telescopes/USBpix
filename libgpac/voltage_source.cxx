#include "voltage_source.h"

using namespace gpac;

blocks::voltage_sources::voltage_sources(phy &aphy):
  m_phy(aphy)
{
  m_ch.emplace_back(m_phy, channels::voltage_source::refdes::VOUT0);
  m_ch.emplace_back(m_phy, channels::voltage_source::refdes::VOUT1);
  m_ch.emplace_back(m_phy, channels::voltage_source::refdes::VOUT2);
  m_ch.emplace_back(m_phy, channels::voltage_source::refdes::VOUT3);
}

channels::voltage_source::voltage_source(phy &aphy, 
    refdes id, 
    float initial_output_voltage):
  m_phy(aphy),
  m_id(id),
  m_output_voltage(0.0),
  m_current_sense_transimpedance_gain(1000.0)
{
  m_vdac_id = -1;
  
  m_voltage_adc.mux_id = devices::adc_channel::mux_refdes::U14;
  m_current_adc.mux_id = devices::adc_channel::mux_refdes::U15;

  switch (m_id)
  {
    case channels::voltage_source::refdes::VOUT0:
    case channels::voltage_source::refdes::VOUT1:
    case channels::voltage_source::refdes::VOUT2:
    case channels::voltage_source::refdes::VOUT3:
      m_voltage_adc.mux_port = m_current_adc.mux_port 
        = devices::adc_channel::mux_ports::S16
        - (m_id - channels::voltage_source::refdes::VOUT0);
      m_vdac_id = 17 - channels::voltage_source::refdes::VOUT0 + m_id;
      break;
    default:
      throw unknown_refdes();
      break;
  }

  output_voltage(initial_output_voltage);
}
        
uint16_t channels::voltage_source::voltage_to_dac_value(double voltage)
{
  return m_phy.core().dac(false).voltage_to_dac_value(voltage);
}

void channels::voltage_source::output_voltage(double value)
{
  uint16_t dac_value = voltage_to_dac_value(value);
  m_phy.core().dac().update(m_vdac_id, dac_value);
  
  m_output_voltage = value;
}

double channels::voltage_source::capture_current()
{
  double cv = m_phy.core().adc().channel(m_current_adc).converter().capture();
  cv -= capture_voltage();
  return cv / m_current_sense_transimpedance_gain;
}

double channels::voltage_source::capture_voltage()
{
  double cv = m_phy.core().adc().channel(m_voltage_adc).converter().capture();
  return cv;
}

