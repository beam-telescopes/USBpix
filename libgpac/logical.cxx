#include "logical.h"

using namespace gpac;

logical::logical(SiUSBDevice *dev):
  m_dev(dev),
  m_phy(m_dev),
  power_supplies(m_phy),
  current_sources(m_phy),
  voltage_sources(m_phy),
  aux_adcs(m_phy),
  injection(m_phy)
{

}
