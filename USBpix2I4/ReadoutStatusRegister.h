#if !defined(READOUT_STATUS_REGISTER_H)
#define READOUT_STATUS_REGISTER_H

#include "ConfigRegister.h"

#include "dllexport.h"

class DllExport ReadoutStatusRegister {

  private:
    ConfigRegister *confReg;
    int readout_channel;
    int value;
  public:
    ReadoutStatusRegister(ConfigRegister *cr, int readout_channel);
    void update();
    void reset();
    void reset(int mask);
    void SetConfigRegister(ConfigRegister *cr);

    int GetValue();
    int GetChannelId();
    
    bool get_ddrd_resync();
    bool get_ddrd_exc_8b10b_code();
    bool get_ddrd_exc_8b10b_disp();
    bool get_type_dh();
    bool get_ddrd_data_out_valid();
    bool get_synchronized();
    bool get_type_vr();
    bool get_ddrd_exc_comma_in_frame();
};

#endif /* !defined(READOUT_STATUS_REGISTER_H) */
