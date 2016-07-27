#include "TurboDaqDB.h"

namespace PixLib{
  class CfgWizardDBInquire;
  class CfgWizardDB; 

  class CfgWizardDBField : public TurboDaqDBField{
    friend class PixLib::CfgWizardDBInquire;
    friend class PixLib::CfgWizardDB; 
  public:
    CfgWizardDBField(string n, enum DBDataType dt = EMPTY) : TurboDaqDBField(n,dt) {};
    virtual ~CfgWizardDBField(){};
  };
  
  class CfgWizardDBInquire : public TurboDaqDBInquire{
    friend class PixLib::CfgWizardDB; 
  public:
    CfgWizardDBInquire(std::string name) : TurboDaqDBInquire(name){};
    ~CfgWizardDBInquire(){};
  };
  
  class CfgWizardDB : public TurboDaqDB{ //public PixConfDBInterface{
    friend class CfgWizardDBInquire;
  public:
    CfgWizardDB();
    ~CfgWizardDB(){};
    void newModuleGroup(const char *name, int slot, const char *ipram,
			const char *idram, const char *extfile);
    void newModule(const char *fname, int in_link, int *out_link, int latency, int delay, int bwidth, 
		   int modID,int grpID, bool active);
  private:
    CfgWizardDBInquire* m_currGroup;
    CfgWizardDBInquire *m_app;
  };
}
