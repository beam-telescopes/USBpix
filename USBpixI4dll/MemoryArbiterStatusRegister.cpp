#include "MemoryArbiterStatusRegister.h"

MemoryArbiterStatusRegister::MemoryArbiterStatusRegister(ConfigRegister *cr)
{
  SetConfigRegister(cr);
}
    
void MemoryArbiterStatusRegister::SetConfigRegister(ConfigRegister *cr)
{
  this->confReg = cr;
  update();
}

void MemoryArbiterStatusRegister::update()
{
  value = confReg->ReadRegister(CS_MA_FLAGS_REG);
}

void MemoryArbiterStatusRegister::reset()
{
  reset(0xff);
}

void MemoryArbiterStatusRegister::reset(int mask)
{
  confReg->WriteRegister(CS_MA_FLAGS_REG, mask);
  value &= ~mask;
}
    
int MemoryArbiterStatusRegister::GetValue()
{
  return value;
}
bool MemoryArbiterStatusRegister::get_fetched_cmd_fifo_overflow()
{
  return ((value & (1 << 0))!=0);
}

bool MemoryArbiterStatusRegister::get_merged_fifo_full()
{
  return ((value & (1 << 1))!=0);
}

bool MemoryArbiterStatusRegister::get_fetched_cmd_fifo_empty()
{
  return ((value & (1 << 2))!=0);
}

bool MemoryArbiterStatusRegister::get_merged_fifo_empty()
{
  return ((value & (1 << 3))!=0);
}
