#include "ReadoutStatusRegister.h"

ReadoutStatusRegister::ReadoutStatusRegister(ConfigRegister *cr, int readout_channel)
{
  this->readout_channel = readout_channel;
  SetConfigRegister(cr);
}
    
void ReadoutStatusRegister::SetConfigRegister(ConfigRegister *cr)
{
  this->confReg = cr;
  update();
}

void ReadoutStatusRegister::update()
{
  value = confReg->ReadRegister(CS_RO_FLAGS_REG(readout_channel));
}

void ReadoutStatusRegister::reset()
{
  reset(0xff);
}

void ReadoutStatusRegister::reset(int mask)
{
  confReg->WriteRegister(CS_RO_FLAGS_REG(readout_channel), mask);
  value &= ~mask;
}
    
int ReadoutStatusRegister::GetChannelId()
{
  return readout_channel;
}
    
int ReadoutStatusRegister::GetValue()
{
  return value;
}
    
bool ReadoutStatusRegister::get_ddrd_resync()
{
  return !!(value & (1 << 0));
}

bool ReadoutStatusRegister::get_ddrd_exc_8b10b_code()
{
  return !!(value & (1 << 1));
}

bool ReadoutStatusRegister::get_ddrd_exc_8b10b_disp()
{
  return !!(value & (1 << 2));
}

bool ReadoutStatusRegister::get_type_dh()
{
  return !!(value & (1 << 3));
}

bool ReadoutStatusRegister::get_ddrd_data_out_valid()
{
  return !!(value & (1 << 4));
}

bool ReadoutStatusRegister::get_synchronized()
{
  return !!(value & (1 << 5));
}

bool ReadoutStatusRegister::get_type_vr()
{
  return !!(value & (1 << 6));
}

bool ReadoutStatusRegister::get_ddrd_exc_comma_in_frame()
{
  return !!(value & (1 << 7));
}

