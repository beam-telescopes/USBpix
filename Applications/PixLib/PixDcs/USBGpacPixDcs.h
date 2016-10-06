#if !defined(_PIXLIB_PIXDCS_USBGPACPIXDCS_H)
#define _PIXLIB_PIXDCS_USBGPACPIXDCS_H

#include "PixDcs.h"

namespace PixLib {
  class DBInquire;
  class Config;
  class USBPixController;
  class USBGpacPixDcs;

  class USBGpacSupplyPixDcsChan: public PixDcsChan
  {
    friend class USBGpacPixDcs;

    public:
      USBGpacSupplyPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
      USBGpacSupplyPixDcsChan(USBGpacSupplyPixDcsChan &chan_in);
      virtual ~USBGpacSupplyPixDcsChan();
      virtual double ReadParam(std::string);
      virtual void SetParam(std::string,double);
      virtual void SetParam(string varType);
      virtual std::string ReadState(std::string);
      virtual void SetState(std::string);
    private:
      virtual void configInit();   //! Init configuration structure
      int m_gpac_channel;
      float m_nominalVoltage;
  };

  class USBGpacVoltageSourcePixDcsChan: public PixDcsChan
  {
    friend class USBGpacPixDcs;

    public:
      USBGpacVoltageSourcePixDcsChan(PixDcs *parent, DBInquire *dbInquire);
      USBGpacVoltageSourcePixDcsChan(USBGpacVoltageSourcePixDcsChan &chan_in);
      virtual ~USBGpacVoltageSourcePixDcsChan();
      virtual double ReadParam(std::string);
      virtual void SetParam(std::string,double);
      virtual void SetParam(string varType);
      virtual std::string ReadState(std::string);
      virtual void SetState(std::string);
    private:
      virtual void configInit();   //! Init configuration structure
      int m_gpac_channel;
      float m_nominalVoltage;
      float m_setVoltage;
      bool m_enabled;
  };

  class USBGpacCurrentSourcePixDcsChan: public PixDcsChan
  {
    friend class USBGpacPixDcs;

    public:
      USBGpacCurrentSourcePixDcsChan(PixDcs *parent, DBInquire *dbInquire);
      USBGpacCurrentSourcePixDcsChan(USBGpacCurrentSourcePixDcsChan &chan_in);
      virtual ~USBGpacCurrentSourcePixDcsChan();
      virtual double ReadParam(std::string);
      virtual void SetParam(std::string,double);
      virtual void SetParam(string varType);
      virtual std::string ReadState(std::string);
      virtual void SetState(std::string);
    private:
      virtual void configInit();   //! Init configuration structure
      int m_gpac_channel;
      float m_nominalCurrent;
      float m_setCurrent;
      bool m_enabled;
  };

  class USBGpacInjectPixDcsChan: public PixDcsChan
  {
    friend class USBGpacPixDcs;

    public:
      USBGpacInjectPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
      USBGpacInjectPixDcsChan(USBGpacInjectPixDcsChan &chan_in);
      virtual ~USBGpacInjectPixDcsChan();
      virtual double ReadParam(std::string);
      virtual void SetParam(std::string,double);
      virtual void SetParam(string varType);
      virtual std::string ReadState(std::string);
      virtual void SetState(std::string);
    private:
      virtual void configInit();   //! Init configuration structure
      int m_gpac_channel;
      bool m_enabled;
      float m_nominalHigh;
      float m_nominalLow;
      float m_setHigh;
      float m_setLow;
  };

  class USBGpacAuxAdcPixDcsChan: public PixDcsChan
  {
    friend class USBGpacPixDcs;

    public:
      USBGpacAuxAdcPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
      USBGpacAuxAdcPixDcsChan(USBGpacAuxAdcPixDcsChan &chan_in);
      virtual ~USBGpacAuxAdcPixDcsChan();
      virtual double ReadParam(std::string);
      virtual void SetParam(std::string,double);
      virtual void SetParam(string varType);
      virtual std::string ReadState(std::string);
      virtual void SetState(std::string);
    private:
      virtual void configInit();   //! Init configuration structure
      int m_gpac_channel;
  };

  class USBGpacPixDcs : public PixDcs 
  {
    public:
      USBGpacPixDcs(DBInquire *dbInquire, void *interface);
      
      virtual ~USBGpacPixDcs();
      
      virtual std::string ReadState(std::string);
      virtual std::string ReadState2(std::string);
      virtual void SetState(std::string);
      virtual void initHW();
      bool sendCommand(std::string &, bool){return false;}; // not implemented, so always return false
      USBPixController *getCtrl() {return m_upc; };

      virtual void ReadError(std::string&);

    protected:
      bool m_problemInit;
      float m_currentLimit;
      USBPixController *m_upc;
      std::string m_ctrlName;

    private:
      virtual void configInit();   //! Init configuration structure
      
  };
}

#endif
