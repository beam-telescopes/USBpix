#if !defined(PIXLIB_PIXDCS_PROBERPIXDCS_H)
#define PIXLIB_PIXDCS_PROBERPIXDCS_H

#include <string>
#include "PixDcs.h"

namespace PixLib {

  enum ProberType {Chuck, Scope};
  enum Position {x,y,z};
  class DBInquire;
  class Config;
  class PixProber;

  class ProberPixDcsChan : public PixDcsChan{

    friend class ProberPixDcs;
    friend class ProbestationChan;

  public:

    ProberPixDcsChan(PixDcs *parent, DBInquire *dbInquire);
    ProberPixDcsChan(ProberPixDcsChan &chan_in);
    virtual ~ProberPixDcsChan();
    virtual double ReadParam(std::string);
    virtual void SetParam(std::string varType, double);
    virtual void SetParam(std::string varType);
    virtual std::string ReadState(std::string);
    virtual void SetState(std::string);	
  protected:
    
    int m_type;
    int m_direction;
    float m_xPos, m_yPos, m_zPos;
    bool m_useDefPos;
    float m_defPos;

  private:
    virtual void configInit();   //! Init configuration structure

  };

    class ProberPixDcs : public PixDcs {
    friend class ProberPixDcsChan;
    friend class ProbestationChan;
    
  public:
    
   
    // Constructor
    ProberPixDcs(DBInquire *dbInquire, void *interface);
    
    // Destructor
    virtual ~ProberPixDcs();
    
    std::string ReadState(std::string);
    void SetState(std::string);
    void initHW();
    bool sendCommand(std::string &, bool){return false;}; // not implemented, so always return false
    void ReadError(std::string&);

  protected:
    int	m_proberType;
    PixProber *m_Prober;
    
    bool m_problemInit;
    bool m_wasInit;

    std::string m_errorMsg;

  private:
    virtual void configInit();   //! Init configuration structure
    
};
}

#endif
