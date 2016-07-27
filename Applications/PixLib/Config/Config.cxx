/////////////////////////////////////////////////////////////////////
// config.cxx 
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

#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixDbInterface/PixDbInterface.h"
#include "Config/Config.h"

using namespace PixLib;

ConfGroup trashConfGroup("__TrashConfGroup__");
Config    trashConfig("__TrashConfig__");

Config::~Config() {
  for (unsigned int i = 0; i < m_group.size(); i++) {
    delete m_group[i];
  }
  for (unsigned i = 0; i < m_config.size(); i++) {
    delete m_config[i];
  }
}

Config &Config::operator=(const Config &cfg) {
  if (&cfg != this) {
    copy(cfg);
  }
  return *this;
}

void Config::copy(const Config &cfg) {
  for (unsigned int i = 0; i < m_group.size(); i++) {
    for (unsigned int j = 0; j< cfg.m_group.size(); j++) {
      if (m_group[i]->name() == cfg.m_group[j]->name()) {
	m_group[i]->copy(*cfg.m_group[j]);
      }
    }
  }
  for (unsigned i = 0; i < m_config.size(); i++) {
    for (unsigned int j = 0; j< cfg.m_config.size(); j++) {
      if (m_config[i]->name() == cfg.m_config[j]->name()) {
	m_config[i]->copy(*cfg.m_config[j]);
      }
    }
  }
}

void Config::addGroup(std::string name) {
  m_group.push_back(new ConfGroup(name));
}

void Config::addConfig(std::string name) {
  m_config.push_back(new Config(name));
}

void Config::addConfig(Config *conf) {
  m_config.push_back(conf);
}

bool Config::read(DBInquire *dbi) {
  unsigned int i;
  bool ret = true;
  for(i = 0; i < m_group.size(); i++) {
    if(!m_group[i]->read(dbi)) ret = false;  
  }
  for(i = 0; i < m_config.size(); i++) {
    recordIterator subdbi = dbi->findRecord(m_config[i]->name());
    if (subdbi != dbi->recordEnd()) {
      if (!m_config[i]->read(*subdbi)) ret = false;
    } else {
      ret = false;
      std::cout << "PixLib::Config " << m_confName << " - Error reading sub-config " << m_config[i]->name() << endl;
    }
  }
  return ret;
}

bool Config::write(DBInquire *dbi) {
  for (unsigned int i = 0; i < m_group.size(); i++)
    if(!m_group[i]->write(dbi)) return false;
#ifdef WIN32
  {for(unsigned int i = 0; i < m_config.size(); i++) {
#else
  for(unsigned int i = 0; i < m_config.size(); i++) {
#endif
    recordIterator subdbi = dbi->findRecord(m_config[i]->name());
    DBInquire *subInq=0;
    if(subdbi == dbi->recordEnd()){
      std::string name1=m_config[i]->name();
      std::string name2=m_config[i]->name();
      size_t pos = name1.find("/");
      if(pos!=std::string::npos){
	name1.erase(pos,name1.length()-pos);
	name2.erase(0, pos+1);
      } else{
	pos = name1.find("Pix");
	if(pos!=std::string::npos) name2.erase(0, pos);
      }
      subInq = dbi->getDB()->makeInquire(name2, dbi->getDecName() + name1 + "/");
      dbi->pushRecord(subInq);
      dbi->getDB()->DBProcess(dbi,COMMITREPLACE);
      dbi->getDB()->DBProcess(subInq,COMMIT);
    } else
      subInq = *subdbi;
    if(subInq==0) return false;
    if(!m_config[i]->write(subInq)) return false;
  }
#ifdef WIN32
  }
#endif
  
  recordIterator record(dbi);
  return dbi->getDB()->DBProcess(record,COMMITREPLACE);
}
  
bool Config::read(DbRecord *dbr) {
  unsigned int i;
  bool ret = true;
  for(i = 0; i < m_group.size(); i++) {
    if(!m_group[i]->read(dbr)) ret = false;  
  }
  for(i = 0; i < m_config.size(); i++) {
    dbRecordIterator subdbi = dbr->findRecord(m_config[i]->name());
    if (subdbi != dbr->recordEnd()) {
      if (!m_config[i]->read(*subdbi)) ret = false;
    } else {
      ret = false;
      std::cout << "PixLib::Config " << m_confName << " - Error reading sub-config " << m_config[i]->name() << endl;
    }
  }
  return ret;
}

bool Config::write(DbRecord *dbr) {
  for (unsigned int i = 0; i < m_group.size(); i++)
    if(!m_group[i]->write(dbr)) return false;
#ifdef WIN32
  {for(unsigned int i = 0; i < m_config.size(); i++) {
#else
  for(unsigned int i = 0; i < m_config.size(); i++) {
#endif
    dbRecordIterator subdbi = dbr->findRecord(m_config[i]->name());
    if(subdbi == dbr->recordEnd()) {
      DbRecord* subrec = dbr->getDb()->makeRecord(m_config[i]->name(), m_config[i]->name());
      dbr->pushRecord(subrec);
      subdbi = dbr->findRecord(m_config[i]->name());
    }
    if(!m_config[i]->write(*subdbi)) return false;
  }
#ifdef WIN32
  }
#endif
  
  dbRecordIterator record(dbr);
  dbr->getDb()->DbProcess(record,PixDb::DBCOMMITREPLACE);
  return true;
}
  
void Config::dump(std::ostream &out, std::string incipit) {
  
  std::string newIncipit;
  std::string fullName = name();
  size_t position = fullName.find("/");
  std::string name;
  if(position != std::string::npos) {
    name = std::string(fullName, position+1, fullName.length()); 
    newIncipit = std::string(fullName, 0, position);
  }
  else {
    name = fullName;
    newIncipit = "";
  }
  
  out << "//" << std::endl
      << "BEGININQUIRE " << incipit << newIncipit << std::endl;
  
  for(unsigned int i = 0; i < m_group.size(); i++)
    m_group[i]->dump(out);
  
  
#ifdef WIN32
  {for(unsigned int i = 0; i < m_config.size(); i++) {
#else
  for(unsigned int i = 0; i < m_config.size(); i++) {
#endif
    std::string fullSubName = m_config[i]->name();
    size_t subPosition = fullSubName.find("/");
    std::string subName;
    if(subPosition != std::string::npos)
      subName = std::string(fullSubName, subPosition+1, fullSubName.length()); 
    else
      subName = fullSubName;
    out << "INQUIRE " << subName << " " << subName << std::endl;
  }
#ifdef WIN32
  }
#endif
  
  out << "ENDINQUIRE " << std::endl;
  
#ifdef WIN32
  {for(unsigned int i = 0; i < m_config.size(); i++) 
#else
  for(unsigned int i = 0; i < m_config.size(); i++) 
#endif
      m_config[i]->dump(out, incipit+newIncipit+"/");
#ifdef WIN32
  }
#endif
}

void Config::reset() {
  for (unsigned int i = 0; i < m_group.size(); i++) {
    m_group[i]->reset();
  }
}

ConfGroup &Config::operator[](std::string name) {
  for (unsigned int i=0; i<m_group.size(); i++) {
    if (m_group[i]->name() == name) {
      return *m_group[i];
    }
  }
    return trashConfGroup;
}
  
Config &Config::subConfig(std::string name) {
  for (unsigned int i=0; i<m_config.size(); i++) {
    if (m_config[i]->name() == name) {
      return *m_config[i];
    }
  }
  return trashConfig;
}
void Config::removeConfig(std::string name) {
  for (unsigned int i=0; i<m_config.size(); i++) {
    if (m_config[i]->name() == name) {
      m_config.erase(m_config.begin()+i);
      break;
    }
  }
}
