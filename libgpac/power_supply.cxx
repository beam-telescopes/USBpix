#include "power_supply.h"
#include <iostream>

using namespace gpac;
        
blocks::power_supplies::power_supplies(phy &aphy):
  m_phy(aphy),
  m_current_vdac_id(0),
  m_Rsns(0.5),
  m_current_sense_gain(20)
{
  m_ch.emplace_back(m_phy, channels::power_supply::refdes::PWR0, *this, 0.0);
  m_ch.emplace_back(m_phy, channels::power_supply::refdes::PWR1, *this, 0.0);
  m_ch.emplace_back(m_phy, channels::power_supply::refdes::PWR2, *this, 0.0);
  m_ch.emplace_back(m_phy, channels::power_supply::refdes::PWR3, *this, 0.0);
  current_limit(0.0);
}

channels::power_supply::power_supply(phy &aphy, 
    channels::power_supply::refdes id, 
    blocks::power_supplies& parent, 
    float initial_output_voltage):
  m_phy(aphy),
  m_id(id),
  m_parent(parent),
  m_enabled(false),
  m_overcurrent(false),
  m_Rs1(10.0e3),
  m_Rs2(649e1),
  m_Rs3(10e3)
{
  m_vdac_id = 1;
  m_voltage_adc.mux_id = devices::adc_channel::mux_refdes::U14;
  m_current_adc.mux_id = devices::adc_channel::mux_refdes::U15;

  switch(m_id)
  {
    case channels::power_supply::refdes::PWR0:
      m_vdac_id = 1;
      m_voltage_adc.mux_port 
        = m_current_adc.mux_port
        = devices::adc_channel::mux_ports::S17;
      break;
    case channels::power_supply::refdes::PWR1:
      m_vdac_id = 2;
      m_voltage_adc.mux_port 
        = m_current_adc.mux_port
        = devices::adc_channel::mux_ports::S18;
      break;
    case channels::power_supply::refdes::PWR2:
      m_vdac_id = 3;
      m_voltage_adc.mux_port 
        = m_current_adc.mux_port
        = devices::adc_channel::mux_ports::S19;
      break;
    case channels::power_supply::refdes::PWR3:
      m_vdac_id = 4;
      m_voltage_adc.mux_port 
        = m_current_adc.mux_port
        = devices::adc_channel::mux_ports::S20;
      break;
    default:
      throw unknown_refdes();
      break;
  }

  output_voltage(initial_output_voltage);
}

uint16_t channels::power_supply::voltage_to_dac_value(double voltage)
{
  // From schematics:
  //    Vout = 0.8 V * (1 + Rs1/Rs2) + (0.8 V - Vset) * Rs1/Rs3
  // -> 0.8 V - (Vout - 0.8 V * (1 + Rs1/Rs2)) * Rs3 / Rs1 = Vset

  double Vset = 0.8 - (voltage - 0.8 * (1 + m_Rs1 / m_Rs2)) * m_Rs3 / m_Rs1;

  return m_phy.core().dac(false).voltage_to_dac_value(Vset);
}
        
void channels::power_supply::output_voltage(double value)
{
  // @todo: update output voltage
  uint16_t dac_value = voltage_to_dac_value(value);
  m_phy.core().dac().update(m_vdac_id, dac_value);
  m_output_voltage = value;
}
        
void channels::power_supply::enabled(bool en, bool update) 
{ 
  m_enabled = en; 

  if (update)
  {
    m_parent.update_enabled();
  }
}

bool channels::power_supply::overcurrent(bool update) 
{ 
  if (update)
  {
    m_parent.update_overcurrent();
  }
  return m_overcurrent;
}

double channels::power_supply::capture_voltage()
{
  return m_phy.core().adc().channel(m_voltage_adc).converter().capture();
}

double channels::power_supply::capture_current()
{
  double cv = m_phy.core().adc().channel(m_current_adc).converter().capture();
  return cv / m_parent.current_sense_transimpedance_gain();
}
        
void blocks::power_supplies::update_enabled()
{
  uint8_t mask = 0;
  for (auto &ch: m_ch)
  {
    mask |= ch.enabled() << 4;
    mask >>= 1;
  }
  m_phy.core().power_enable_oc().write(mask);
}

void blocks::power_supplies::update_overcurrent()
{
  uint8_t mask = m_phy.core().power_enable_oc().read();
  mask = ~mask;
  mask >>= 4;

  for (auto &ch: m_ch)
  {
    ch.force_oc_state(mask & 1);
    mask >>= 1;
  }
}

uint16_t blocks::power_supplies::current_to_dac_value(double current)
{
  double vset = current_sense_transimpedance_gain() * current;
  
  return m_phy.core().dac(false).voltage_to_dac_value(vset);
}

void blocks::power_supplies::current_limit(double value) 
{
  uint16_t dac_value = current_to_dac_value(value);
  
  m_phy.core().dac().update(m_current_vdac_id, dac_value);
}

