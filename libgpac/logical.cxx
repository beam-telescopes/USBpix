#include "logical.h"
#include <sstream>
#include <iostream>

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
  // process calibration data read from EEPROM
  std::vector<calData> calGain = m_phy.core().getCalib();
  for(std::vector<calData>::iterator it = calGain.begin(); it!=calGain.end(); it++){
    // debug printout
//     std::cout <<"GPAC chan. " << it->name << ": VADCGain=" << it->VADCGain << ", VADCoffset=" << it->VADCOffs << std::endl;
//     std::cout <<"                  IADCGain=" << it->IADCGain << ", IADCoffset=" << it->IADCOffs << std::endl;
//     std::cout <<"                  DACGain=" << it->DACGain << ", DACoffset=" << it->DACOffs << std::endl;

    if(it->name=="VINJ0") injection.gain(2.*it->DACGain); // EEPROM gain differs by factor of 2 vs. Johannes' implementation

    if(it->name.substr(0,3)=="PWR"){
      std::stringstream a;
      a << it->name.substr(3,1);
      int id;
      a >> id;
      // consider factor 1000 (mV in calib. offset, V in our scheme)
      power_supplies[id].setDACCalib(1.e-3*it->DACOffs, 0.5/it->DACGain);
    }
    if(it->name.substr(0,4)=="VSRC"){
      std::stringstream a;
      a << it->name.substr(4,1);
      int id;
      a >> id;
      //voltage_sources[id].xxx
    }
    if(it->name.substr(0,4)=="ISRC"){
      std::stringstream a;
      a << it->name.substr(4,1);
      int id;
      a >> id;
      //current_sources[id].xxx
    }
  }

}
