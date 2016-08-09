/////////////////////////////////////////////////////////////////////
// PixCcpd.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 11/02/13  Version 1.0 (JGK)

//! Abstract class for the CCPD chips

#ifndef _PIXLIB_CCPD
#define _PIXLIB_CCPD

#include <map>
#include <string>

namespace PixLib {

  class Config;
  class PixModule;
  class DBInquire;

  class PixCcpd {

    friend class PixModule;

  public:

    virtual ~PixCcpd(){};
                  
    Config &config(){ return *m_conf; };       // Configuration object accessor

    virtual void loadConfig(std::string configName)=0; // Read the config from DB
    virtual void saveConfig(std::string configName)=0; // Save the config to DB

    // generate different types of CPIX
    static PixCcpd* make(DBInquire *dbInquire, std::string type);    //! Factory
    static void listTypes(std::vector<std::string> &list);           //! List available types

  protected:

    Config *m_conf;
    DBInquire *m_dbInquire;
    std::string m_className;
  };
  
}
#endif // _PIXLIB_CCPD
