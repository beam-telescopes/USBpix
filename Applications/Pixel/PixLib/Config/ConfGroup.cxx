/////////////////////////////////////////////////////////////////////
// CF_group.cxx 
// version 4.0
/////////////////////////////////////////////////////////////////////
//
// 26/09/99  Version 0.1 (PM)
//  7/10/99  Version 0.2 (PM)
//  4/11/99  Version 0.3 (PM)
// 25/07/01  Version 2.1.0 (PM)
// 26/03/04  Version 3.0 - Imported from SimPix (PM)
// 20/07/06  Version 4.0 - Interface with PixDbInterface (PM)
//

#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixDbInterface/PixDbInterface.h"
#include "Config/ConfGroup.h"

using namespace PixLib;

ConfObj trashConfObj("__TrashConfObj__", "Object not found", false, ConfObj::VOID);

ConfGroup::~ConfGroup() {
  for (unsigned int i = 0; i < m_param.size(); i++) {
    delete m_param[i];
  }
}

void ConfGroup::copy(const ConfGroup &grp) {
  for (unsigned int i = 0; i < m_param.size(); i++) {
    for (unsigned int j = 0; j< grp.m_param.size(); j++) {
      if (m_param[i]->name() == grp.m_param[j]->name()) {
	m_param[i]->copy(*grp.m_param[j]);
      }
    }
  }
}

void ConfGroup::copyRef(const ConfGroup &grp) {
  for (unsigned int i = 0; i < grp.m_param.size(); i++) {
    switch(grp.m_param[i]->type()){
      // to do: VECTOR, MATRIX, LIST};
    case ConfObj::INT:
      switch(((ConfInt*)grp.m_param[i])->subtype()){
      case  ConfInt::S32:
      default:
	addInt(grp.m_param[i]->name(), ((ConfInt*)grp.m_param[i])->valueS32(), 0, grp.m_param[i]->comment(), grp.m_param[i]->visible());
	break;
      case  ConfInt::U32:
	addInt(grp.m_param[i]->name(), ((ConfInt*)grp.m_param[i])->valueU32(), 0, grp.m_param[i]->comment(), grp.m_param[i]->visible());
	break;
      case ConfInt::S16:
	addInt(grp.m_param[i]->name(), ((ConfInt*)grp.m_param[i])->valueS16(), 0, grp.m_param[i]->comment(), grp.m_param[i]->visible());
	break;
      case ConfInt::U16:
	addInt(grp.m_param[i]->name(), ((ConfInt*)grp.m_param[i])->valueU16(), 0, grp.m_param[i]->comment(), grp.m_param[i]->visible());
	break;
      case ConfInt::S8:
	addInt(grp.m_param[i]->name(), ((ConfInt*)grp.m_param[i])->valueS8(), 0, grp.m_param[i]->comment(), grp.m_param[i]->visible());
	break;
      case ConfInt::U8:
	addInt(grp.m_param[i]->name(), ((ConfInt*)grp.m_param[i])->valueU8(), 0, grp.m_param[i]->comment(), grp.m_param[i]->visible());
	break;
      }
      break;
    case ConfObj::FLOAT:
      addFloat(grp.m_param[i]->name(), grp.m_param[i]->valueFloat(), 0., grp.m_param[i]->comment(), grp.m_param[i]->visible());
      break;
    case ConfObj::BOOL:
      addBool(grp.m_param[i]->name(), grp.m_param[i]->valueBool(), false, grp.m_param[i]->comment(), grp.m_param[i]->visible());
      break;
    case ConfObj::STRING:
      addString(grp.m_param[i]->name(), grp.m_param[i]->valueString(), "", grp.m_param[i]->comment(), grp.m_param[i]->visible());
      break;
    case ConfObj::VOID:
    default:
      break;
    }
  }
}
void ConfGroup::addInt(std::string name, int &val, int def, std::string comm, bool vis) {
  m_param.push_back(new ConfInt(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addInt(std::string name, unsigned int &val, unsigned int def, std::string comm, bool vis) {
  m_param.push_back(new ConfInt(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addInt(std::string name, short int &val, short int def, std::string comm, bool vis) {
  m_param.push_back(new ConfInt(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addInt(std::string name, unsigned short int &val, unsigned short int def, std::string comm, bool vis) {
  m_param.push_back(new ConfInt(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addInt(std::string name, char &val, char def, std::string comm, bool vis) {
  m_param.push_back(new ConfInt(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addInt(std::string name, unsigned char &val, unsigned char def, std::string comm, bool vis) {
  m_param.push_back(new ConfInt(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addFloat(std::string name, float &val, float def, std::string comm, bool vis) {
  m_param.push_back(new ConfFloat(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addVector(std::string name, std::vector<int> &val, std::vector<int> def, std::string comm, bool vis) {
  m_param.push_back(new ConfVector(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addVector(std::string name, std::vector<unsigned int> &val, std::vector<unsigned int> def, std::string comm, bool vis) {
  m_param.push_back(new ConfVector(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addVector(std::string name, std::vector<float> &val, std::vector<float> def, std::string comm, bool vis) {
  m_param.push_back(new ConfVector(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addMatrix(std::string name, ConfMask<unsigned short int> &val, ConfMask<unsigned short int> def, std::string comm, bool vis) {
  m_param.push_back(new ConfMatrix(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addMatrix(std::string name, ConfMask<bool> &val, ConfMask<bool> def, std::string comm, bool vis) {
  m_param.push_back(new ConfMatrix(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addMatrix(std::string name, ConfMask<float> &val, ConfMask<float> def, std::string comm, bool vis) {
  m_param.push_back(new ConfMatrix(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addList(std::string name, int &val, int def, std::map<std::string, int> symb, std::string comm, bool vis) {
  m_param.push_back(new ConfList(m_groupName+"_"+name, val, def, symb, comm, vis));
}

void ConfGroup::addList(std::string name, unsigned int &val, unsigned int def, std::map<std::string, int> symb, std::string comm, bool vis) {
  m_param.push_back(new ConfList(m_groupName+"_"+name, val, def, symb, comm, vis));
}

void ConfGroup::addList(std::string name, short int &val, short int def, std::map<std::string, int> symb, std::string comm, bool vis) {
  m_param.push_back(new ConfList(m_groupName+"_"+name, val, def, symb, comm, vis));
}

void ConfGroup::addList(std::string name, unsigned short int &val, unsigned short int def, std::map<std::string, int> symb, std::string comm, bool vis) {
  m_param.push_back(new ConfList(m_groupName+"_"+name, val, def, symb, comm, vis));
}

void ConfGroup::addList(std::string name, char &val, char def, std::map<std::string, int> symb, std::string comm, bool vis) {
  m_param.push_back(new ConfList(m_groupName+"_"+name, val, def, symb, comm, vis));
}

void ConfGroup::addList(std::string name, unsigned char &val, unsigned char def, std::map<std::string, int> symb, std::string comm, bool vis) {
  m_param.push_back(new ConfList(m_groupName+"_"+name, val, def, symb, comm, vis));
}

void ConfGroup::addBool(std::string name, bool &val, bool def, std::string comm, bool vis) {
  m_param.push_back(new ConfBool(m_groupName+"_"+name, val, def, comm, vis));
}

void ConfGroup::addBool(std::string name, bool &val, bool def, std::string y, std::string n, std::string comm, bool vis) {
  m_param.push_back(new ConfBool(m_groupName+"_"+name, val, def, y, n, comm, vis));
}

void ConfGroup::addString(std::string name, std::string &val, std::string def, std::string comm, bool vis, int type) {
  m_param.push_back(new ConfString(m_groupName+"_"+name, val, def, comm, vis, type));
}

bool ConfGroup::read(DBInquire *dbi) {
  bool ret = true;
  for (unsigned int i = 0; i < m_param.size(); i++) {
    if (!m_param[i]->read(dbi)) {
      ret = false;
      //      std::cout << "PixLib::ConfGrp - Group " << m_groupName << " - Error reading parameter " << m_param[i]->name() << endl;
    }
  }
  return ret;
}

bool ConfGroup::write(DBInquire *dbi) {
  for (unsigned int i = 0; i < m_param.size(); i++)
    if (!m_param[i]->write(dbi)) return false;

  return true;
}

bool ConfGroup::read(DbRecord *dbr) {
  bool ret = true;
  for (unsigned int i = 0; i < m_param.size(); i++) {
    if (!m_param[i]->read(dbr)) {
      ret = false;
      //      std::cout << "PixLib::ConfGrp - Group " << m_groupName << " - Error reading parameter " << m_param[i]->name() << endl;
    }
  }
  return ret;
}

bool ConfGroup::write(DbRecord *dbr) {
  for (unsigned int i = 0; i < m_param.size(); i++)
    if (!m_param[i]->write(dbr)) return false;

  return true;
}

void ConfGroup::dump(std::ostream &out) {
  for (unsigned int i = 0; i < m_param.size(); i++)
    m_param[i]->dump(out);
}
  
void ConfGroup::reset() {
  for (unsigned int i = 0; i < m_param.size(); i++) {
    m_param[i]->reset();
  }
}

ConfObj &ConfGroup::operator[](std::string name) {
  std::string xnam = m_groupName+"_"+name;
  for (unsigned int i=0; i<m_param.size(); i++) {
    if (m_param[i]->name() == xnam) {
      return *m_param[i];
    }
  }
  return trashConfObj;
}
