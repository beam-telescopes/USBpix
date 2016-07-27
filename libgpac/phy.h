#if !defined(LIBGPAC_PHY_H)
#define LIBGPAC_PHY_H

#include "i2chost.h"

#include <dllexport.h>

#ifdef WIN32
  #if _MSC_VER >= 1700
    #include <stdint.h>
  #else
    #include "inttypes_win32.h"
  #endif
#else
  #include <inttypes.h>
#endif

#include <exception>
#include <map>

#ifdef WIN32
  #define STDE_NOEXCEPT
#else
  #define STDE_NOEXCEPT noexcept
#endif

class SiUSBDevice;

namespace gpac
{

  typedef struct _calData{
    std::string name;
    double IADCGain;
    double IADCOffs;
    double VADCGain;
    double VADCOffs;
    double DACGain;
    double DACOffs;
  } calData;

  namespace devices
  {
    class DllExport PCA9554 
    { 
      private:
        i2chost *m_i2c;
        uint8_t m_addr_pins;
        uint8_t addr(bool read);

      public:
        PCA9554(i2chost *i2c, uint8_t addr_pins);
        void configure(uint8_t data);
        void write(uint8_t data);
        uint8_t read();
    };

    class DllExport PCA9540B
    {
      private:
        i2chost *m_i2c;
        const uint8_t addr;
        bool enabled;
        uint8_t current_id;

      public:
        PCA9540B(i2chost *i2c);
        void select(uint8_t id);
        void disable();
    };
    
    class DllExport DAC7578
    {
      public:
        enum tristate {low = 0, high = 1, high_impedance = 2};

      private:
        i2chost *m_i2c;
        uint8_t addr(bool read);
        uint8_t command_and_access(uint8_t command, uint8_t access);
        tristate m_addr0;

      public:
        DAC7578(i2chost *i2c, tristate addr0);
        void update(uint8_t channel, uint16_t value);
    };
    
    class DllExport MAX11644
    {
      public:
        enum reference_voltage {
          VDD                   = 0, 
          VDD_                  = 1, 
          external              = 2,
          external_             = 3,
          internal_nooutput_off = 4,
          internal_nooutput_on  = 5,
          internal_output_off   = 6,
          internal_output_on    = 7,
        };

        enum scan_mode {
          AIN0_TO_CS0 = 0,
          CS0_EIGHT_TIMES = 1,
          _RESERVED = 2,
          CS0_ONCE = 3,
        };

      private:
        i2chost *m_i2c;

        reference_voltage m_sel;
        bool m_clk_external;
        bool m_bipolar;

        uint8_t m_cs0;
        bool m_single_ended;
        scan_mode m_scan_mode;

        uint8_t configuration_byte();
        uint8_t setup_byte();

        void update_registers();

        const uint8_t m_addr;

        double m_ext_voltage;
        double m_vdd;

      public:
        MAX11644(i2chost *i2c);
        void cs0(uint8_t value) {m_cs0 = value; update_registers();};
        double capture();
    };

    class DllExport dac_block
    {
      private:
        std::vector<DAC7578> m_dacs;
        
        double m_Vref;
      public:
        dac_block(i2chost *i2c);

        enum refdes {U4 = 0, U5 = 1, U6 = 2};
        std::vector<DAC7578> &dacs() {return m_dacs; };
        DAC7578 &operator[] (int id) {return m_dacs.at(id); };
        DAC7578 &dac_by_vdac (int vdac_id) {return m_dacs.at(vdac_id/8); };
        uint8_t channel_by_vdac (int vdac_id) {return vdac_id % 8; };
        void update(int vdac_id, uint16_t value);

        uint16_t voltage_to_dac_value(double voltage);
    };

    struct DllExport adc_channel
    {
      enum mux_refdes {U14 = 0, U15 = 1};
      enum mux_ports {S1 = 0, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, 
        S13, S14, S15, S16, S17, S18, S19, S20, S21, S22, S23, S24, S25, S26,
        S27, S28, S29, S30, S31, S32};
      int mux_id;
      uint8_t mux_port;

      adc_channel(int mux_id = 0, uint8_t mux_port = 0): 
        mux_id(mux_id), mux_port(mux_port) {};
    };

    class DllExport adc_block
    {
      public:
        class unknown_mux_id: std::exception {
          public:
            const char* what() const STDE_NOEXCEPT {
              return "The supplied mux id does not exist.";
            }
        };
        class unknown_mux_port: std::exception {
          public:
            const char* what() const STDE_NOEXCEPT {
              return "The supplied mux port does not exist.";
            }
        };
      private:
        PCA9554 m_mux_ctrl;
        MAX11644 m_converter;

        int m_mux_id;
        uint8_t m_mux_port;

        void update_mux();
      public:
        MAX11644 &converter(){return m_converter;};
        PCA9554 &mux_ctrl(){return m_mux_ctrl;};

        adc_block(i2chost *i2c);
        void init();
        adc_block &channel(int mux_id, uint8_t mux_port);
        adc_block &channel(adc_channel ch) {
          return channel(ch.mux_id, ch.mux_port);
        };
    };

    class DllExport core
    {
      protected:
        PCA9554 m_power_enable_oc;
        PCA9540B m_i2c_mux;
        dac_block m_dac;
        adc_block m_adc;
	i2chost *m_i2c;
	unsigned int m_ID;
	std::vector<calData> m_calData;
	void calib();

      public:
        core(i2chost *i2c);
        ~core();
        void init();
	unsigned int getId(){return m_ID;};
	std::vector<calData> getCalib(){return m_calData;};

        PCA9554 &power_enable_oc(bool select = true) 
        {
          if (select)
            i2c_mux().select(0);
          return m_power_enable_oc; 
        };

        PCA9540B &i2c_mux()
        {
          return m_i2c_mux;
        };

        adc_block &adc(bool select = true) 
        {
          if (select)
            i2c_mux().select(1);
          return m_adc; 
        };

        dac_block &dac(bool select = true) 
        {
          if (select)
            i2c_mux().select(0);
          return m_dac; 
        };
    };

  }

  class DllExport phy 
  {
    private:
      SiUSBDevice* m_dev;
      std::unique_ptr<i2chost> m_i2c;
      devices::core m_core;

    public:
      phy(SiUSBDevice *dev = nullptr);

      devices::core &core() { return m_core; };

      void dut() {m_core.i2c_mux().disable(); };
      void updateDev(SiUSBDevice *dev);
  };

}

#endif
