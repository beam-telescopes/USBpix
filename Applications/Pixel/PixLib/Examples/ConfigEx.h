namespace PixLib {
  class DbRecord;
  class Config;
}

using namespace PixLib;

class ModuleConn;
class Pp0Conn;
class BocRodConn;

class GenericConnectivityObject {
public:
  // Identifier
  std::string id;
  // Config object
  Config *m_config;
  // DB record
  DbRecord *m_record;
  
  // Constructors
  GenericConnectivityObject(DbRecord *r) : m_record(r) {};
  // Destructor
  virtual ~GenericConnectivityObject();
  // Config handling
  virtual void readConfig();
  virtual void readConfig(DbRecord *r);
  virtual void writeConfig();
  virtual void writeConfig(DbRecord *r);
};

class ModuleConn : public GenericConnectivityObject {
public:
  // Down link description
  // Up Link description
  std::string t0CableId;
  Pp0Conn* pp0Lnk;
  int pp0Slot;
  // Soft Link description
  // Internal connectivity tables

  // Constructors
  ModuleConn();
  ModuleConn(DbRecord *r);
  // Config handling
  void initConfig();
  // Link to configuration
  std::string getModuleConfigLnk();
  // Soft Links
  // Down link handling
  // read & write
  void write(DbRecord *r);
  void read(DbRecord *r);
};

class Pp0Conn : public GenericConnectivityObject {
public:
  // Down link description
  std::string t0CableId[7];
  ModuleConn* moduleLnk[7];
  // Up Link description
  // Soft Link description
  std::string txRibbonId;
  std::string rxRibbonId[2];
  int txSlot;
  int rxSlot[2];
  // Internal connectivity tables
  int dciFiber[7];
  int dtoFiber[7];
  int dtoRibbon[7];
  int dto2Fiber[7];
  int dto2Ribbon[7];

  // Constructors
  Pp0Conn();
  Pp0Conn(DbRecord *r);
  // Config handling
  void initConfig();
  // Link to configuration
  // Soft Links
  std::string getBocLink();
  // Down link handling
  void createDownLinks(bool recursive);
  void readDownLinks(bool recirsive);
  void createModule(int moduleId);
  void linkModule(ModuleConn* mod, int moduleId);
  void readModule(int moduleId);
  // read & write
  void write(DbRecord *r);
  void read(DbRecord *r);
};

class BocRodConn : public GenericConnectivityObject {
public:
  // Down link description
  // Up Link description
  Config *rodCrate;
  int vmeSlot;
  // Soft Link description
  std::string txRibonId[4];
  std::string rxRibbonId[4];
  int rxOptoBoardCh;
  // Internal connectivity tables
  int rodTx[4][8];
  int rodFmt[4][8];
  int rodCh[4][8];  

  // Constructors
  BocRodConn();
  BocRodConn(DbRecord *r);
  // Config handling
  void initConfig();
  // Link to configuration
  std::string moduleGroupConfigLnk;
  // Soft Links
  std::string getPp0Link(int pp0Id);
  // Down link handling
  // read & write
  void write(DbRecord *r);
  void read(DbRecord *r);
};
