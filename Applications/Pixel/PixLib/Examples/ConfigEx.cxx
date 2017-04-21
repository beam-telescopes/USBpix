#include "PixDbInterface/PixDbInterface.h"
#include "PixDbInterface/RootDb.h"
#include "Config/Config.h"
#include "Examples/ConfigEx.h"

#include <iostream>
#include <sstream>

GenericConnectivityObject::~GenericConnectivityObject() {
  if (m_config != NULL) delete m_config;
  if (m_record != NULL) delete m_record;
}
void GenericConnectivityObject::readConfig() { 
  // Read from DB
  m_config->read(m_record);
}
void GenericConnectivityObject::readConfig(DbRecord *r) { 
  // Read from DB
  m_config->read(r);
}
void GenericConnectivityObject::writeConfig() { 
  // Write to DB
  m_config->write(m_record);
}
void GenericConnectivityObject::writeConfig(DbRecord *r) { 
  // Write to DB
  m_config->write(r);
}



BocRodConn::BocRodConn() : GenericConnectivityObject(NULL) {
  // Create the config object
  initConfig(); 
}
BocRodConn::BocRodConn(DbRecord *r) : GenericConnectivityObject(r) {
  // Create the config object
  initConfig();
  readConfig(m_record); 
}
void BocRodConn::initConfig() { 
  m_config = new Config("PixModuleGroup");
  Config &conf = *m_config;
  conf.addGroup("Identifier");
  conf["Identifier"].addString("Id", id, "Crate0Slot0", "Object identifier", true);
  conf.addGroup("Config");
  //conf["Config"].addLinkString("ModuleGroupConfigLink", moduleGroupConfigLink, "---", "Link to configuration record", true);
  conf.addGroup("Down");
  conf.addGroup("Up");
  conf.addGroup("Maps");
  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      std::ostringstream o;
      o << "_" << i << "_" << j;
      conf["Maps"].addInt("RodTx"+o.str(), rodTx[i][j], -1, "BOC/ROD tx map", true);
      conf["Maps"].addInt("RodFmt"+o.str(), rodFmt[i][j], -1, "BOC/ROD fmt map", true);
      conf["Maps"].addInt("RodCh"+o.str(), rodCh[i][j], -1, "BOC/ROD ch map", true);
    }
  }
  conf.reset();
}

void BocRodConn::write(DbRecord *r) {
  //PixDbInterface *db = r->getDb();
  //dbRecordIterator sub = r->findRecord(id);
  //if(sub == r->recordEnd()) {
  //    DbRecord* rec = db->makeRecord("BocRodConn", id);
  //  r->pushRecord(rec);
  //  sub = r->findRecord(rec->getDecName()+"/"+rec->getName());
  //  sub = db->DbProcess(sub, PixDb::DBREAD);
  //  std::cout << **sub;
  //}
  writeConfig(r);
}

int main() {
  BocRodConn rod;
 
  PixDbInterface* rDb;
  rDb = new RootDb("connectivity.root", "NEW");
  DbRecord* tRec = rDb->readRootRecord(); 
  rod.write(tRec);
  delete tRec;
  delete rDb;
}
