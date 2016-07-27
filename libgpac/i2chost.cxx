#include "i2chost.h"
#include <iostream>
#include <SiLibUSB.h>

using namespace gpac;

const bool GPAC_DEBUG_I2C = false;

uint8_t dummy_i2chost::read8(uint8_t addr)
{
  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[dummy_i2chost::read8] (" 
      << std::hex << "0x" << int(addr) << std::dec << ")" << std::endl;
  }
  return 0;
}

std::unique_ptr<std::vector<uint8_t>> 
  dummy_i2chost::read(uint8_t addr, size_t len)
{
  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[dummy_i2chost::read] (" 
      << std::hex << "0x" << int(addr) << std::dec << ", " << len << ")" 
      << std::endl;
  }
  std::unique_ptr<std::vector<uint8_t>> r (new std::vector<uint8_t> (0));
  return std::move(r);
}

bool dummy_i2chost::write8(uint8_t addr, uint8_t data)
{
  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[dummy_i2chost::write8] (" 
      << std::hex << "0x" << int(addr) << std::dec << ", " 
      << std::hex << "0x" << int(data) << std::dec << ")" << std::endl;
  }
  return true;
}

bool dummy_i2chost::write(uint8_t addr, std::vector<uint8_t> data)
{
  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[dummy_i2chost::write] (" 
      << std::hex << "0x" << int(addr) << std::dec << ", {";
    for (auto d: data)
    {
      std::cout << "0x" << std::hex << int(d) << std::dec << ", ";
    }
    std::cout << "})" << std::endl;
  }
  return true;
}

      
usbpix_i2chost::usbpix_i2chost(SiUSBDevice *dev):
  m_dev(dev)
{
  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[usbpix_i2chost::usbpix_i2chost] (" 
      << std::hex << "0x" << dev << std::dec << ")" << std::endl;
  }
  if (dev == nullptr)
  {
    throw i2chost_exception();
  }
}

uint8_t usbpix_i2chost::read8(uint8_t addr)
{
  if (!m_dev)
    return 0;

  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[usbpix_i2chost::read8] (" 
      << std::hex << "0x" << int(addr) << std::dec << ")";
  }

  uint8_t result = 0;
  bool ret = m_dev->ReadI2C(addr, &result, 1);
  
  if (GPAC_DEBUG_I2C)
  {
    std::cout << " = " << int(result) << std::endl;
  }

  if (!ret)
  {
    throw i2chost_exception();
  }

  return result;
}

std::unique_ptr<std::vector<uint8_t>> 
  usbpix_i2chost::read(uint8_t addr, size_t len)
{
  if (!m_dev)
    return std::unique_ptr<std::vector<uint8_t>> (new std::vector<uint8_t> (0));

  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[usbpix_i2chost::read] (" 
      << std::hex << "0x" << int(addr) << std::dec << ", " << len << ")";
  }
  std::unique_ptr<std::vector<uint8_t>> r (new std::vector<uint8_t> (len));
  bool ret = m_dev->ReadI2C(addr, r->data(), len);

  if (GPAC_DEBUG_I2C)
  {
    std::cout << " = ";
    for (auto x: *r)
    {
      std::cout << std::hex << "0x" << int(x) << std::dec << ", ";
    }
    std::cout << std::endl;
  }
  if (!ret)
  {
    throw i2chost_exception();
  }

  return std::move(r);
}

bool usbpix_i2chost::write8(uint8_t addr, uint8_t data)
{
  if (!m_dev)
    return false;

  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[usbpix_i2chost::write8] (" 
      << std::hex << "0x" << int(addr) << std::dec << ", " 
      << std::hex << "0x" << int(data) << std::dec << ")" << std::endl;
  }
  return m_dev->WriteI2C(addr, &data, 1);
}

bool usbpix_i2chost::write(uint8_t addr, std::vector<uint8_t> data)
{
  if (!m_dev)
    return false;

  if (GPAC_DEBUG_I2C)
  {
    std::cout << "[usbpix_i2chost::write] (" 
      << std::hex << "0x" << int(addr) << std::dec << ", {";
    for (auto d: data)
    {
      std::cout << "0x" << std::hex << int(d) << std::dec << ", ";
    }
    std::cout << "})" << std::endl;
  }

  return m_dev->WriteI2C(addr, data.data(), data.size());
}
