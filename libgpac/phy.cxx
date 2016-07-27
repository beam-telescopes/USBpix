#include "phy.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <string.h>

using namespace gpac;

void convCont(unsigned char *newVar, std::vector<uint8_t> dataBuf, unsigned int offset, unsigned int length){
  unsigned char *dataArr = new unsigned char[length];
  for(unsigned int j=0;j<length;j++) dataArr[j] = dataBuf.at(offset+j);
  memcpy(newVar, dataArr, length);
  delete[] dataArr;
}

phy::phy(SiUSBDevice *dev):
  m_dev(dev),
  m_i2c(
    (dev == nullptr)
    ? (static_cast<i2chost *> (new dummy_i2chost())) 
    : (new usbpix_i2chost(m_dev))
  ),
  m_core(m_i2c.get())
{
}

void phy::updateDev(SiUSBDevice *dev)
{
  i2chost* h = m_i2c.get();
  usbpix_i2chost* uh = dynamic_cast<usbpix_i2chost *> (h);
  if (uh)
  {
    uh->updateDev(dev);
  }
}

devices::PCA9554::PCA9554(i2chost *i2c, uint8_t addr_pins):
  m_i2c(i2c),
  m_addr_pins(addr_pins & 0x7)
{
  // @todo: validate addr_pins value
}
        
uint8_t devices::PCA9554::addr(bool read)
{
  const uint8_t base_addr = 0x40;
  return base_addr + (m_addr_pins << 1) + int(read);
}

void devices::PCA9554::configure(uint8_t data)
{
  const uint8_t configure_register = 0x03;
  std::vector<uint8_t> buf;
  buf.push_back(configure_register);
  buf.push_back(data);
  m_i2c->write(addr(false), buf);
}

void devices::PCA9554::write(uint8_t data)
{
  const uint8_t out_register = 0x01;
  std::vector<uint8_t> buf;
  buf.push_back(out_register);
  buf.push_back(data);
  m_i2c->write(addr(false), buf);
}

uint8_t devices::PCA9554::read()
{
  m_i2c->write8(addr(false), 0x00);
  return m_i2c->read8(addr(true));
}
      
devices::PCA9540B::PCA9540B(i2chost *i2c):
  m_i2c(i2c),
  addr(0xe0),
  enabled(false),
  current_id(0)
{

}

void devices::PCA9540B::select(uint8_t id)
{
  const uint8_t cmd_enable = 0x04;

  if (id > 1)
  {
    disable();
    return;
  }

  if ((id == current_id) && enabled)
  {
    return;
  }

  current_id = id;
  enabled = true;

  m_i2c->write8(addr, cmd_enable | id);
}
        
void devices::PCA9540B::disable()
{
  const uint8_t cmd_disable = 0;
  enabled = false;
  m_i2c->write8(addr, cmd_disable);
}

gpac::devices::DAC7578::DAC7578(i2chost *i2c, gpac::devices::DAC7578::tristate addr0):
  m_i2c(i2c),
  m_addr0(addr0)
{

}
        
uint8_t gpac::devices::DAC7578::addr(bool read)
{
  uint8_t addr_base = 0x90;

  uint8_t addr = addr_base;

  switch(m_addr0)
  {
    case tristate::low:
      addr += 0;
      break;
    case tristate::high:
      addr += 4;
      break;
    case tristate::high_impedance:
      addr += 8;
      break;
  }

  addr += int(read);

  return addr;
}
        
uint8_t gpac::devices::DAC7578::command_and_access(uint8_t command, uint8_t access)
{
  return ((command << 4) & 0xf0) | (access & 0x0f);
}

void gpac::devices::DAC7578::update(uint8_t channel, uint16_t value)
{
  uint8_t command_write_and_update = 0x3;
  uint8_t val_high = (value >> 4);
  uint8_t val_low = (value << 4);

  std::vector<uint8_t> buf;
  buf.push_back(command_and_access(command_write_and_update, channel));
  buf.push_back(val_high);
  buf.push_back(val_low);

  m_i2c->write(addr(false), buf);
}

devices::MAX11644::MAX11644(i2chost *i2c):
  m_i2c(i2c),
  m_sel(internal_nooutput_on),
  m_clk_external(false),
  m_bipolar(true),
  m_cs0(0),
  m_single_ended(true),
  m_scan_mode(CS0_EIGHT_TIMES),
  m_addr(0x6C),
  m_ext_voltage(2.048),
  m_vdd(5.0)
{

}

uint8_t devices::MAX11644::setup_byte()
{
  uint8_t setup_byte = 1 << 7;
  uint8_t reset_bar = 1 << 1; 

  setup_byte |= (m_sel & 0x7) << 4;
  setup_byte |= uint8_t(m_clk_external) << 3;
  setup_byte |= uint8_t(m_bipolar) << 2;
  setup_byte |= reset_bar;

  return setup_byte;
}

uint8_t devices::MAX11644::configuration_byte()
{
  uint8_t conf_byte = 0x00;
  
  conf_byte |= uint8_t(m_scan_mode & 0x03) << 5;
  conf_byte |= uint8_t(m_cs0) << 1;
  conf_byte |= uint8_t(m_single_ended);

  return conf_byte;
}
        
void devices::MAX11644::update_registers()
{
  std::vector<uint8_t> data;

  data.push_back(configuration_byte());
  data.push_back(setup_byte());

  m_i2c->write(m_addr, data);
}

double devices::MAX11644::capture()
{
  double reference_voltage = 5.0;
  switch(m_sel)
  {
    case VDD:
    case VDD_:
      reference_voltage = m_vdd;
      break;
   case external:
    case external_:
      reference_voltage = m_ext_voltage;
      break;
    case internal_nooutput_off:
    case internal_nooutput_on:
    case internal_output_off:
    case internal_output_on:
      reference_voltage = 4.096;
      break;
  }

  std::unique_ptr<std::vector<uint8_t>> data;
  switch(m_scan_mode)
  {
    case AIN0_TO_CS0:
    case _RESERVED:
      throw std::exception();
      break;
    case CS0_EIGHT_TIMES:
      data = move(m_i2c->read(m_addr + 1, 16));
      break;
    case CS0_ONCE:
      data = move(m_i2c->read(m_addr + 1, 2));
      break;
  }

  uint16_t avg = 0;
  int count = 0;
  for (std::vector<uint8_t>::iterator it = data->begin();
      it != data->end(); it++)
  {
    uint16_t dw = uint16_t((*it) & 0x0f) << 8;
    it++;
    if (it == data->end())
      break;
    count++;
    avg  += dw | *it;
  }

  double averaged = double(avg) / count;

  double rescaled = averaged * reference_voltage / 4096.0;

  return rescaled;
}

devices::core::core(i2chost *i2c):
  m_power_enable_oc(i2c, 0x01),
  m_i2c_mux(i2c),
  m_dac(i2c),
  m_adc(i2c),
  m_i2c(i2c)
{
  init();
}

void devices::core::init()
{
  calib();

  power_enable_oc().write(0x00);
  power_enable_oc().configure(0xf0);
  adc().init();
  i2c_mux().disable();
}
void devices::core::calib()
{
  m_i2c_mux.select(0);

  const uint8_t calEepromAdd = 0xA8;
  const unsigned int calEepromPageSize = 32;
  // read calib. constants from EEPROM
// // #define MAX_PWR    4
// // #define MAX_VSRC   4  
// // #define MAX_VINJ   2  
// // #define MAX_ISRC  12
  const unsigned int nChan = 22, nNameLen=64, nDblLen=8, nDblVar=10;
  unsigned int size = nChan*(nDblLen*nDblVar+nNameLen)+4;
  unsigned int nPages, nBytes;
  nPages = size / calEepromPageSize;
  nBytes = size % calEepromPageSize;
  std::vector<uint8_t> addBuf(2,0);
  std::vector<uint8_t> dataBuf;
  
  m_i2c->write(calEepromAdd, addBuf);
  for (unsigned int i = 0; i < nPages; i++){ // 64 byte page write
    std::unique_ptr<std::vector<uint8_t>> data = m_i2c->read((calEepromAdd | 0x01), calEepromPageSize);
    dataBuf.insert(dataBuf.end(), data->begin(), data->end());
  }
  if (nBytes > 0){
    std::unique_ptr<std::vector<uint8_t>> data = m_i2c->read((calEepromAdd | 0x01), nBytes);
    dataBuf.insert(dataBuf.end(), data->begin(), data->end());
  }
  //unsigned int header = (((unsigned int)dataBuf.at(0)) << 8) + (unsigned int)dataBuf.at(1);
  m_ID = (((unsigned int)dataBuf.at(2)) << 8) + (unsigned int)dataBuf.at(3);
  //std::cout << "EEPROM header: " << header << ", ID = " << m_ID << std::endl;

  unsigned int vec_offs = 4;
  for (unsigned int i = 0; i < nChan; i++) {
    char cal_name[nNameLen];
    calData tmpCalData;
    convCont((unsigned char*)cal_name, dataBuf, vec_offs, nNameLen);
    tmpCalData.name = cal_name;
    vec_offs += nNameLen;
    // default
    vec_offs += nDblLen;
    // minimum
    vec_offs += nDblLen;
    // maximum
    vec_offs += nDblLen;
    convCont((unsigned char*)(&tmpCalData.IADCGain), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    convCont((unsigned char*)(&tmpCalData.IADCOffs), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    convCont((unsigned char*)(&tmpCalData.VADCGain), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    convCont((unsigned char*)(&tmpCalData.VADCOffs), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    convCont((unsigned char*)(&tmpCalData.DACGain), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    convCont((unsigned char*)(&tmpCalData.DACOffs), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    //convCont((unsigned char*)(&calVal), dataBuf, vec_offs, nDblLen);
    vec_offs += nDblLen;
    // limit

    // add constants to map for later retrieval
    m_calData.push_back(tmpCalData);
  }
}
        
devices::core::~core()
{
}

devices::adc_block::adc_block(i2chost *i2c):
  m_mux_ctrl(i2c, 0x00),
  m_converter(i2c),
  m_mux_id(0),
  m_mux_port(0)
{
}

void devices::adc_block::init()
{
  mux_ctrl().write(0x00);
  mux_ctrl().configure(0x00);
}
        
void devices::adc_block::update_mux()
{
  mux_ctrl().write(m_mux_port);
}

devices::adc_block& devices::adc_block::channel(int mux_id, uint8_t mux_port)
{
  if ((mux_id < 0) || (mux_id > 1))
  {
    throw unknown_mux_id();
  }

  if (mux_port > 31)
  {
    throw unknown_mux_port();
  }

  m_mux_id = mux_id;
  m_mux_port = mux_port;
  update_mux();
  m_converter.cs0(mux_id);

  return *this;
}
        
devices::dac_block::dac_block(i2chost *i2c):
  m_Vref(2.048)
{
  m_dacs.emplace_back(i2c, DAC7578::tristate::low);
  m_dacs.emplace_back(i2c, DAC7578::tristate::high);
  m_dacs.emplace_back(i2c, DAC7578::tristate::high_impedance);
}

void devices::dac_block::update(int vdac_id, uint16_t value)
{
  DAC7578 &rdac = dac_by_vdac(vdac_id);
  uint8_t channel = channel_by_vdac(vdac_id);
  rdac.update(channel, value);
}
        
uint16_t devices::dac_block::voltage_to_dac_value(double voltage)
{
  double dac_value = (voltage / m_Vref) * 4096.0;

  dac_value = std::max(dac_value, 0.0);
  dac_value = std::min(dac_value, 4095.0);

  return uint16_t(static_cast<int>(dac_value));
}
      
