#include "current_source.h"

using namespace gpac;

blocks::current_sources::current_sources(phy &aphy):
  m_phy(aphy)
{
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT0);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT1);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT2);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT3);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT4);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT5);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT6);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT7);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT8);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT9);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT10);
  m_ch.emplace_back(m_phy, channels::current_source::refdes::IOUT11);
}

channels::current_source::current_source(phy &aphy, 
    refdes id, 
    float initial_output_current):
  m_phy(aphy),
  m_id(id),
  m_vdac_id(-1),
  m_output_current(0.0),
  m_transfer_factor(0.00101),
  m_sense_transfer_factor(1.00101),
  m_Vref(1.024)
{
  
  m_voltage_adc.mux_id = devices::adc_channel::mux_refdes::U14;
  m_current_adc.mux_id = devices::adc_channel::mux_refdes::U15;

  switch (m_id)
  {
    case channels::current_source::refdes::IOUT0:
    case channels::current_source::refdes::IOUT1:
    case channels::current_source::refdes::IOUT2:
    case channels::current_source::refdes::IOUT3:
    case channels::current_source::refdes::IOUT4:
    case channels::current_source::refdes::IOUT5:
    case channels::current_source::refdes::IOUT6:
    case channels::current_source::refdes::IOUT7:
    case channels::current_source::refdes::IOUT8:
    case channels::current_source::refdes::IOUT9:
    case channels::current_source::refdes::IOUT10:
    case channels::current_source::refdes::IOUT11:
      m_voltage_adc.mux_port = m_current_adc.mux_port 
        = devices::adc_channel::mux_ports::S21 
        - channels::current_source::refdes::IOUT0 
        + m_id;
      m_vdac_id = 5 - channels::current_source::refdes::IOUT0 + m_id;
      break;
    default:
      throw unknown_refdes();
      break;
  }
  output_current(initial_output_current);
}
        
uint16_t channels::current_source::current_to_dac_value(double current)
{
  double Vset = current / m_transfer_factor + m_Vref;

  return m_phy.core().dac(false).voltage_to_dac_value(Vset);
}

void channels::current_source::output_current(double value)
{
  uint16_t dac_value = current_to_dac_value(value);
  m_phy.core().dac().update(m_vdac_id, dac_value);
  
  m_output_current = value;
}

double channels::current_source::capture_current()
{
  double cv = m_phy.core().adc().channel(m_current_adc).converter().capture();
  return (cv - m_Vref) * m_transfer_factor / m_sense_transfer_factor;
}

double channels::current_source::capture_voltage()
{
  double cv = m_phy.core().adc().channel(m_voltage_adc).converter().capture();
  return cv;
}
