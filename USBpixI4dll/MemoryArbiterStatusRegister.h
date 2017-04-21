#if !defined(MEMORYARBITER_STATUS_REGISTER_H)
#define MEMORYARBITER_STATUS_REGISTER_H

#include "ConfigRegister.h"

#include "dllexport.h"

class DllExport MemoryArbiterStatusRegister

{
  private:
    ConfigRegister *confReg;
    int value;
  public:
    MemoryArbiterStatusRegister(ConfigRegister *cr);
    void update();
    void reset();
    void reset(int mask);
    void SetConfigRegister(ConfigRegister *cr);

    int GetValue();

    bool get_fetched_cmd_fifo_overflow();
    bool get_merged_fifo_full();
    bool get_fetched_cmd_fifo_empty();
    bool get_merged_fifo_empty();
    
};

#endif /* !defined(READOUMEMORYARBITER_STATUS_REGISTER_H) */
