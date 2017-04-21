/////////////////////////////////////////////////////////////////////
// config.h 
// version 4.0
/////////////////////////////////////////////////////////////////////
//
// 26/09/99  Version 0.1 (PM)
//         - Read/write on file of config data
//         - Import/export to tcl of config data
//         - Support for int, bool, char*, enum
//  7/10/99  Version 0.2 (PM)
//  4/11/99  Version 0.3 (PM)
// 25/07/01  Version 2.1.0 (PM)
// 26/03/04  Version 3.0 - Imported from SimPix (PM)
// 20/07/06  Version 4.0 - Interface with PixDbInterface (PM)
//

#ifndef PIXLIB_CONFIG
#define PIXLIB_CONFIG

#include <vector>
#include "Config/ConfGroup.h"


namespace PixLib {

class DBInquire;
class DbRecord;

class Config {
public:
  Config(std::string name) { 
    m_confName = name; 
    m_confType = name; 
  };                                                     // Constructor
  Config(std::string name, std::string type) { 
    m_confName = name; 
    m_confType = type; 
  };                                                     // Constructor
  Config(const Config &cfg);                             // Copy constructor
  ~Config();                                             // Destructor

  Config &operator=(const Config& cfg);                  // Copy operator
  void copy(const Config &cfg);                          // Copy values to another Config 

  void addGroup(std::string name);                       // Adds a group
  void addConfig(std::string name);                      // Adds a sub-config
  void addConfig(Config *conf);                          // Adds a sub-config
  void removeConfig(std::string name);                   // Removes a previoulsy added sub-config

  bool read(DBInquire *dbi);                             // Reads from DB
  bool write(DBInquire *dbi);                            // Writes to DB
  bool read(DbRecord *dbr);                              // Reads from DB2
  bool write(DbRecord *dbr);                             // Writes to DB2
  void dump(std::ostream &out, std::string incipit="");  // Dumps config content
  void reset();                                          // Set all parameters to default
  std::string name() { return m_confName; };             // Returns config name
  std::string type() { return m_confType; };             // Returns config type
  int size() { return m_group.size(); };                 // Returns the size
  ConfGroup &operator[](int i) { return *m_group[i]; };
  ConfGroup &operator[](std::string name);
  int subConfigSize() { return m_config.size(); };       // Returns the number of depending configs
  Config &subConfig(int i) { return *m_config[i]; };
  Config &subConfig(std::string name);

  //private: 
  std::vector<ConfGroup*> m_group;
  std::vector<Config*> m_config;
  std::string m_confName;
  std::string m_confType;
};

}

#endif

