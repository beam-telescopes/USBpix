#include "aux_adc.h"

using namespace gpac;
        
blocks::aux_adcs::aux_adcs(phy &aphy):
  m_phy(aphy)
{
  m_ch.emplace_back(m_phy, channels::aux_adc::refdes::AUX_ADC0);
  m_ch.emplace_back(m_phy, channels::aux_adc::refdes::AUX_ADC1);
  m_ch.emplace_back(m_phy, channels::aux_adc::refdes::AUX_ADC2);
  m_ch.emplace_back(m_phy, channels::aux_adc::refdes::AUX_ADC3);
}

channels::aux_adc::aux_adc(phy& aphy, refdes id):
  m_phy(aphy),
  m_id(id)
{
  m_adc_channel.mux_id = devices::adc_channel::mux_refdes::U14;

  switch (m_id)
  {
    case channels::aux_adc::refdes::AUX_ADC0:
    case channels::aux_adc::refdes::AUX_ADC1:
    case channels::aux_adc::refdes::AUX_ADC2:
    case channels::aux_adc::refdes::AUX_ADC3:
      m_adc_channel.mux_port = devices::adc_channel::mux_ports::S12
        - (m_id - channels::aux_adc::refdes::AUX_ADC0);
      break;
    default:
      throw unknown_refdes();
      break;
  }
}

double channels::aux_adc::capture()
{
  return m_phy.core().adc().channel(m_adc_channel).converter().capture();
}
