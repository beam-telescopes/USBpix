/////////////////////////////////////////////////////////////////////
// ConfObj.cxx
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
#include "Config/ConfMask.h"
#include "Config/ConfObj.h"

#include <iostream>
#include <fstream>
#include <typeinfo>

using namespace PixLib;

/////////////////////
// ConfObj methods
/////////////////////

ConfObj::ConfObj(std::string nam, std::string comm, bool vis, types typ) {
  m_name = nam;
  m_comment = comm;
  m_visible = vis;
  m_type = typ;
}
 
ConfObj::~ConfObj() {
}

/////////////////////
// ConfInt methods
/////////////////////

ConfInt::ConfInt(std::string nam, int &val, int def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,INT) {
    m_value = (void *)&val;
    m_subtype = S32;
    m_defS32 = def;
}
ConfInt::ConfInt(std::string nam, int &val, int def, std::string comm, bool vis, types typ) :
  ConfObj(nam,comm,vis,typ) {
    m_value = (void *)&val;
    m_subtype = S32;
    m_defS32 = def;
}
ConfInt::ConfInt(std::string nam, unsigned int &val, unsigned int def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,INT) {
    m_value = (void *)&val;
    m_subtype = U32;
    m_defU32 = def;
}
ConfInt::ConfInt(std::string nam, unsigned int &val, unsigned int def, std::string comm, bool vis, types typ) :
  ConfObj(nam,comm,vis,typ) {
    m_value = (void *)&val;
    m_subtype = U32;
    m_defU32 = def;
}
ConfInt::ConfInt(std::string nam, short int &val, short int def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,INT) {
    m_value = (void *)&val;
    m_subtype = S16;
    m_defS16 = def;
}
ConfInt::ConfInt(std::string nam, short int &val, short int def, std::string comm, bool vis, types typ) :
  ConfObj(nam,comm,vis,typ) {
    m_value = (void *)&val;
    m_subtype = S16;
    m_defS16 = def;
}
ConfInt::ConfInt(std::string nam, unsigned short int &val, unsigned short int def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,INT) {
    m_value = (void *)&val;
    m_subtype = U16;
    m_defU16 = def;
}
ConfInt::ConfInt(std::string nam, unsigned short int &val, unsigned short int def, std::string comm, bool vis, types typ) :
  ConfObj(nam,comm,vis,typ) {
    m_value = (void *)&val;
    m_subtype = U16;
    m_defU16 = def;
}
ConfInt::ConfInt(std::string nam, char &val, char def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,INT) {
    m_value = (void *)&val;
    m_subtype = S8;
    m_defS8 = def;
}
ConfInt::ConfInt(std::string nam, char &val, char def, std::string comm, bool vis, types typ) :
  ConfObj(nam,comm,vis,typ) {
    m_value = (void *)&val;
    m_subtype = S8;
    m_defS8 = def;
}
ConfInt::ConfInt(std::string nam, unsigned char &val, unsigned char def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,INT) {
    m_value = (void *)&val;
    m_subtype = U8;
    m_defU8 = def;
}
ConfInt::ConfInt(std::string nam, unsigned char &val, unsigned char def, std::string comm, bool vis, types typ) :
  ConfObj(nam,comm,vis,typ) {
    m_value = (void *)&val;
    m_subtype = U8;
    m_defU8 = def;
}
  
ConfInt::~ConfInt() {
}

void ConfInt::copy(const ConfObj &obj) {
  try { 
    const ConfInt &cObj = dynamic_cast<const ConfInt &>(obj);
    if (m_subtype == cObj.m_subtype) {
      switch(m_subtype){
      case S32:
	*((int *)m_value) = *((int *)(cObj.m_value));
	break;
      case U32:
	*((unsigned int *)m_value) = *((unsigned int *)(cObj.m_value));
	break;
      case S16:
	*((short int *)m_value) = *((short int *)(cObj.m_value));
	break;
      case U16:
	*((unsigned short int *)m_value) = *((unsigned short int *)(cObj.m_value));
	break;
      case S8:
	*((char *)m_value) = *((char *)(cObj.m_value));
	break;
      case U8:
	*((unsigned char *)m_value) = *((unsigned char *)(cObj.m_value));
	break;
      default: ;
      }
    }
  }
  catch (std::bad_cast err) {
    return;
  } 
}

int ConfInt::getValue() {
  int ivalue;
  switch (m_subtype) {
  case S32:
    ivalue = (int)*((int *)m_value);
    break;
  case U32:  
    ivalue = (int)*((unsigned int *)m_value);
    break;
  case S16:
    ivalue = (int)*((short int *)m_value);
    break;
  case U16:
    ivalue = (int)*((unsigned short int *)m_value);  
    break;
  case S8:
    ivalue = (int)*((char *)m_value);
    break;
  case U8:
    ivalue = (int)*((unsigned char *)m_value);
    break;
  }
  return ivalue;
}

void ConfInt::setValue(int val) {
  switch(m_subtype){
  case S32:
    *((int *)m_value) = (int)val;
    break;
  case U32:
    *((unsigned int *)m_value) = (unsigned int)val;
    break;
  case S16:
    *((short int *)m_value) = (short int)val;
    break;
  case U16:
    *((unsigned short int *)m_value) = (unsigned short int)val;
    break;
  case S8:
    *((char *)m_value) = (char)val;
    break;
  case U8:
    *((unsigned char *)m_value) = (unsigned char)val;
    break;
  default: ;
  }
}

void ConfInt::reset() {
  switch (m_subtype) {
  case S32:
    *((int *)m_value) = m_defS32;
    break;
  case U32:
    *((unsigned int *)m_value) = m_defU32;
    break;
  case S16:
    *((short int *)m_value) = m_defS16;
    break;
  case U16:
    *((unsigned short int *)m_value) = m_defU16;
    break;
  case S8:
    *((char *)m_value) = m_defS8;
    break;
  case U8:
    *((unsigned char *)m_value) = m_defU8;
    break;
  }
}

bool ConfInt::read(DBInquire *dbi) {
  fieldIterator f;
  int value;
  unsigned int uvalue;
  bool stat;

  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  switch (m_subtype) {
  case S32:
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if (stat) *((int *)m_value) = value;
    break;
  case U32:
    stat = dbi->getDB()->DBProcess(f,READ,uvalue);
    if (stat) *((unsigned int *)m_value) = uvalue;
    break;
  case S16:
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if (stat) *((short int *)m_value) = value;
    break;
  case U16:
    stat = dbi->getDB()->DBProcess(f,READ,uvalue);
    if (stat) *((unsigned short int *)m_value) = uvalue;
    break;
  case S8:
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if (stat) *((char *)m_value) = value;
    break;
  case U8:
    stat = dbi->getDB()->DBProcess(f,READ,uvalue);
    if (stat) *((unsigned char *)m_value) = uvalue;
    break;
  }
  return stat;
}
  
bool ConfInt::write(DBInquire *dbi) {
  fieldIterator f;
  int value;
  unsigned int uvalue;
  bool unsig;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }
  switch (m_subtype) {
  case S32:
    value = (int) *((int *)m_value);
    unsig = false;
    break;
  case U32:
    uvalue = (unsigned int) *((unsigned int *)m_value);
    unsig = true;
    break;
  case S16:
    value = (int) *((short int *)m_value);
    unsig = false;
    break;
  case U16:
    uvalue = (unsigned int) *((unsigned short int *)m_value);
    unsig = true;
    break;
  case S8:
    value = (int) *((char *)m_value);
    unsig = false;
    break;
  case U8:
    uvalue = (unsigned int) *((unsigned char *)m_value);
    unsig = true;
    break;
  }
  bool ret;
  if (unsig) {
    ret = dbi->getDB()->DBProcess(f,COMMIT,uvalue);
  } else {
    ret = dbi->getDB()->DBProcess(f,COMMIT,value);
  }
  return ret;
}

bool ConfInt::read(DbRecord *dbr) {
  dbFieldIterator f;
  int value;
  unsigned int uvalue;

  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;

  try {
    switch (m_subtype) {
    case S32:
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      *((int *)m_value) = value;
      break;
    case U32:
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,uvalue);
      *((unsigned int *)m_value) = uvalue;
      break;
    case S16:
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      *((short int *)m_value) = value;
      break;
    case U16:
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      *((unsigned short int *)m_value) = value;
      break;
    case S8:
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      *((char *)m_value) = value;
      break;
    case U8:
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      *((unsigned char *)m_value) = value;
      break;
    }
  }
  catch (PixDBException ex) {
    return false;
  }
  return true;
}
  
bool ConfInt::write(DbRecord *dbr) {
  dbFieldIterator f;
  DbField *fi = NULL;
  int value;
  unsigned int uvalue;
  bool unsig;
  bool ret = true;
  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }

  switch (m_subtype) {
  case S32:
    value = (int) *((int *)m_value);
    unsig = false;
    break;
  case U32:
    uvalue = (unsigned int) *((unsigned int *)m_value);
    unsig = true;
    break;
  case S16:
    value = (int) *((short int *)m_value);
    unsig = false;
    break;
  case U16:
    value = (unsigned int) *((unsigned short int *)m_value);
    unsig = false;
    break;
  case S8:
    value = (int) *((char *)m_value);
    unsig = false;
    break;
  case U8:
    value = (unsigned int) *((unsigned char *)m_value);
    unsig = false;
    break;
  }
  try {
    if (unsig) {
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,uvalue);
    } else {
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,value);
    }
    f = dbr->pushField(*f); 
  }
  catch (PixDBException ex) {
    ret = false;
  }
  if (fi != NULL) delete fi;

  return ret;
}

void ConfInt::dump(std::ostream &out) {
  out << "FIELD " << name() << " int " << getValue() << std::endl;
}


/////////////////////
// ConfFloat methods
/////////////////////

ConfFloat::ConfFloat(std::string nam, float &val, float def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,FLOAT), m_value(val) {
    m_defval = def;
}
  
ConfFloat::~ConfFloat() {
}

void ConfFloat::copy(const ConfObj &obj) {
  try { 
    const ConfFloat &cObj = dynamic_cast<const ConfFloat &>(obj);
    m_value = cObj.m_value;
  }
  catch (std::bad_cast err) {
    return;
  } 
}

bool ConfFloat::read(DBInquire *dbi) {
  fieldIterator f;
  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  return dbi->getDB()->DBProcess(f,READ,m_value);
}
  
bool ConfFloat::write(DBInquire *dbi) {
  fieldIterator f;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }

  if(f==dbi->fieldEnd()) return false;

  return dbi->getDB()->DBProcess(f,COMMIT,m_value);
}

bool ConfFloat::read(DbRecord *dbr) {
  dbFieldIterator f;
  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;

  try {
    dbr->getDb()->DbProcess(f,PixDb::DBREAD,m_value);
  }
  catch (PixDBException ex) {
    return false;
  }
  return true;
}
  
bool ConfFloat::write(DbRecord *dbr) {
  dbFieldIterator f;
  DbField *fi = NULL;
  bool ret = true;
  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    DbField *fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }
  try {
    dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,m_value);
    f = dbr->pushField(*f); 
    if (fi != NULL) delete fi;
  }
  catch (PixDBException ex) {
    ret = false;
  }
  return ret;
}

void ConfFloat::dump(std::ostream &out) {
  out << "FIELD " << name() << " float " << m_value << std::endl;
}

/////////////////////
// ConfVector methods
/////////////////////

ConfVector::ConfVector(std::string nam, std::vector<int> &val, std::vector<int> def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,VECTOR) {
  m_value = (void *)&val;
  m_subtype = INT;
  m_defInt = def;
}
  
ConfVector::ConfVector(std::string nam, std::vector<unsigned int> &val, std::vector<unsigned int> def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,VECTOR) {
  m_value = (void *)&val;
  m_subtype = UINT;
  m_defUint = def;
}
  
ConfVector::ConfVector(std::string nam, std::vector<float> &val, std::vector<float> def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,VECTOR) {
  m_value = (void *)&val;
  m_subtype = FLOAT;
  m_defFloat = def;
}
  
ConfVector::~ConfVector() {
}

void ConfVector::copy(const ConfObj &obj) {
  try { 
    const ConfVector &cObj = dynamic_cast<const ConfVector &>(obj);
    if (m_subtype == cObj.m_subtype) {
      switch (m_subtype) {
      case INT:
	*((std::vector<int> *)m_value) = *((std::vector<int> *)(cObj.m_value));
	break;
      case UINT:
	*((std::vector<unsigned int> *)m_value) = *((std::vector<unsigned int> *)(cObj.m_value));
	break;
      case FLOAT:
	*((std::vector<float> *)m_value) = *((std::vector<float> *)(cObj.m_value));
	break;
      }
    }
  }
  catch (std::bad_cast err) {
    return;
  } 
}

void ConfVector::reset() {
  switch (m_subtype) {
  case INT:
    *((std::vector<int> *)m_value) = m_defInt;
    break;
  case UINT:
    *((std::vector<unsigned int> *)m_value) = m_defUint;
    break;
  case FLOAT:
    *((std::vector<float> *)m_value) = m_defFloat;
    break;
  }
}

bool ConfVector::read(DBInquire *dbi) {
  bool stat=false;
  fieldIterator f;
  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  switch (m_subtype) {
  case INT: {
    std::vector<int> value;
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if(stat) {
      *(std::vector<int>*)m_value = value;
    }
    break;
  }
  case UINT: {
    std::vector<int> value;
    std::vector<unsigned int> uvalue;
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if(stat) {
      for (unsigned int i=0; i<value.size(); i++) uvalue.push_back((unsigned int)value[i]);
      *(std::vector<unsigned int> *)m_value = uvalue;
    }
    break;
  }
  case FLOAT:
    std::vector<float> value;    
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if(stat) {
      *(std::vector<float>*)m_value = value;
    }
    break;
  }
  
  return stat;
}
  
bool ConfVector::write(DBInquire *dbi) {
  fieldIterator f;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }

  switch (m_subtype) {
  case INT: {
    std::vector<int> value;
    value = *(std::vector<int> *)m_value;
    return dbi->getDB()->DBProcess(f,COMMIT,value);
    break;
  }
  case UINT: {
    std::vector<unsigned int> &v = *(std::vector<unsigned int> *)m_value;
    std::vector<int> value;
    std::vector<unsigned int>::iterator it;
    for(it=v.begin(); it!=v.end(); it++) {
      value.push_back((int)*it);
    }
    return dbi->getDB()->DBProcess(f,COMMIT,value);
    break;
  }
  case FLOAT: {
    std::vector<float> value;
    value = *(std::vector<float> *)m_value;
    return dbi->getDB()->DBProcess(f,COMMIT,value);
    break;
  }
  }
  return false;
}

bool ConfVector::read(DbRecord *dbr) {
  bool stat = true;
  dbFieldIterator f;
  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;

  try {
    switch (m_subtype) {
    case INT: {
      std::vector<int> value;
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      if(stat) {
	*(std::vector<int>*)m_value = value;
      }
      break;
    }
    case UINT: {
      std::vector<int> value;
      std::vector<unsigned int> uvalue;
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      if(stat) {
	for (unsigned int i=0; i<value.size(); i++) uvalue.push_back((unsigned int)value[i]);
	*(std::vector<unsigned int> *)m_value = uvalue;
      }
      break;
    }
    case FLOAT:
      std::vector<float> value;
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      if(stat) {
	*(std::vector<float>*)m_value = value;
      }
      break;
    }
  }
  catch (PixDBException ex) {
    stat = false;
  }
  
  return stat;
}
  
bool ConfVector::write(DbRecord *dbr) {
  dbFieldIterator f;
  bool ret=true;
  DbField *fi = NULL;
  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }

  try {
    switch (m_subtype) {
    case INT: {
      std::vector<int> value;
      value = *(std::vector<int> *)m_value;
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,value);
      break;
    }
    case UINT: {
      std::vector<unsigned int> &v = *(std::vector<unsigned int> *)m_value;
      std::vector<int> value;
      std::vector<unsigned int>::iterator it;
      for(it=v.begin(); it!=v.end(); it++) {
	value.push_back((int)*it);
      }
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,value);
      break;
    }
    case FLOAT: {
      std::vector<float> value;
      value = *(std::vector<float> *)m_value;
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,value);
      break;
    }
    }
  }
  catch (PixDBException) {
    ret = false;
  }

  f = dbr->pushField(*f); 
  if (fi != NULL) delete fi;

  return ret;
}

void ConfVector::dump(std::ostream &out) {
  out << "FIELD " << name();
  std::vector<int> value;
  switch (m_subtype) {
  case INT: {
    out << " vector<int>";
    std::vector<int> &temp = *(std::vector<int> *)m_value;
    std::vector<int>::iterator it;
    for(it=temp.begin(); it != temp.end(); it++) {
      out << " " << (*it);
    }
    out << std::endl;
    break;
  }
  case UINT: {
    out << " vector<int>";
    std::vector<unsigned int> &temp = *(std::vector<unsigned int> *)m_value;
    std::vector<unsigned int>::iterator it;
    for(it=temp.begin(); it != temp.end(); it++) {
      out << " " << (*it);
    }
    out << std::endl;
    break;
  }
  case FLOAT: {
    out << " vector<float>";
    std::vector<float> &temp = *(std::vector<float> *)m_value;
    std::vector<float>::iterator it;
    for(it=temp.begin(); it != temp.end(); it++) {
      out << " " << (*it);
    }
    out << std::endl;
    break;
  }
  }
}

/////////////////////
// ConfMatrix methods
/////////////////////

ConfMatrix::ConfMatrix(std::string nam, ConfMask<unsigned short int> &val, ConfMask<unsigned short int> def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,MATRIX) {
  m_value = (void *)&val;
  m_subtype = U16;
  m_defU16 = def;
}
ConfMatrix::ConfMatrix(std::string nam, ConfMask<bool> &val, ConfMask<bool> def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,MATRIX) {
  m_value = (void *)&val;
  m_subtype = U1;
  m_defU1 = def;
}
ConfMatrix::ConfMatrix(std::string nam, ConfMask<float> &val, ConfMask<float> def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,MATRIX) {
  m_value = (void *)&val;
  m_subtype = FLOAT;
  m_defFloat = def;
}
  
ConfMatrix::~ConfMatrix() {
}

void ConfMatrix::copy(const ConfObj &obj) {
  try { 
    const ConfMatrix &cObj = dynamic_cast<const ConfMatrix &>(obj);
    if (m_subtype == cObj.m_subtype) {
      switch(m_subtype){
		  case U16:
			  *((ConfMask<unsigned short int> *)m_value) = *((ConfMask<unsigned short int> *)(cObj.m_value));
			  break;
		  case U1:
			  *((ConfMask<bool> *)m_value) = *((ConfMask<bool> *)(cObj.m_value));
			  break;
		  case FLOAT:
		  	  *((ConfMask<float> *)m_value) = *((ConfMask<float> *)(cObj.m_value));
		  	  break;
		  default: ;
      }
    }
  }
  catch (std::bad_cast err) {
    return;
  } 
}

void ConfMatrix::reset() {
  switch (m_subtype) {
	  case U16:
		*((ConfMask<unsigned short int> *)m_value) = m_defU16;
		break;
	  case U1:
		*((ConfMask<bool> *)m_value) = m_defU1;
		break;
	  case FLOAT:
	  	*((ConfMask<float> *)m_value) = m_defFloat;
	  	break;
  }
}

bool ConfMatrix::read(DBInquire *dbi) {
  bool stat=false;
  fieldIterator f;
  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  switch (m_subtype) {
  case U16: {
    std::vector<int> value;
    std::vector<unsigned short int> valueU16;
    stat = dbi->getDB()->DBProcess(f,READ,value);
    if(stat) {
      for(unsigned int i=0; i<value.size(); i++) valueU16.push_back((unsigned short int)value[i]);
      ((ConfMask<unsigned short int>*)m_value)->set(valueU16);
    }
    break;
  }
  case U1: {
    std::vector<bool> valueU1;
    stat = dbi->getDB()->DBProcess(f,READ,valueU1);
    if(stat)
      ((ConfMask<bool>*)m_value)->set(valueU1);
    break;
  }
  case FLOAT: {
    std::vector<float> valueFLOAT;
    stat = dbi->getDB()->DBProcess(f,READ,valueFLOAT);
    if(stat)
      ((ConfMask<float>*)m_value)->set(valueFLOAT);
    break;
    }
  }
  
  return stat;
}
  
bool ConfMatrix::write(DBInquire *dbi) {
  std::vector<int> value;
  fieldIterator f;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }

  switch (m_subtype) {
  case U16: {
    std::vector<unsigned short int> temp;
    ((ConfMask<unsigned short int> *)m_value)->get(temp);
    std::vector<unsigned short int>::iterator it, itEnd=temp.end();
    for(it=temp.begin(); it!=itEnd; it++)
      value.push_back((int)*it);
    break;
  }
  case U1: {
    std::vector<bool> temp;
    ((ConfMask<bool> *)m_value)->get(temp);
    return dbi->getDB()->DBProcess(f,COMMIT,temp);
    break;
  }
  case FLOAT: {
    std::vector<float> temp;
    ((ConfMask<float> *)m_value)->get(temp);
    return dbi->getDB()->DBProcess(f,COMMIT,temp);
    break;
  }
  }
  return dbi->getDB()->DBProcess(f,COMMIT,value);
}

bool ConfMatrix::read(DbRecord *dbr) {
  bool stat=true;
  dbFieldIterator f;
  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;

  try {
    switch (m_subtype) {
    case U16: {
      std::vector<int> value;
      std::vector<unsigned short int> valueU16;
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
      for(unsigned int i=0; i<value.size(); i++) valueU16.push_back((unsigned short int)value[i]);
      ((ConfMask<unsigned short int>*)m_value)->set(valueU16);
      break;
    }
    case U1:{
      std::vector<bool> valueU1;
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,valueU1);
      ((ConfMask<bool>*)m_value)->set(valueU1);
      break;
    }
    case FLOAT: {
      std::vector<float> valueFLOAT;
      dbr->getDb()->DbProcess(f,PixDb::DBREAD,valueFLOAT);
      ((ConfMask<float>*)m_value)->set(valueFLOAT);
      break;
    }
    }
  }
  catch (PixDBException ex) {
    stat = false;
  }    
  
  return stat;
 }
  
bool ConfMatrix::write(DbRecord *dbr) {
  std::vector<int> value;
  dbFieldIterator f;
  DbField *fi = NULL;
  bool ret = true;
  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }

  try {
    switch (m_subtype) {
    case U16: {
      std::vector<unsigned short int> temp;
      ((ConfMask<unsigned short int> *)m_value)->get(temp);
      std::vector<unsigned short int>::iterator it, itEnd=temp.end();
      for(it=temp.begin(); it!=itEnd; it++)
    	  value.push_back((int)*it);
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,value);
      break;
    }
    case U1: {
      std::vector<bool> temp;
      ((ConfMask<bool> *)m_value)->get(temp);
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,temp);
      break;
    }
    case FLOAT: {
      std::vector<float> temp;
      ((ConfMask<float> *)m_value)->get(temp);
      dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,temp);
      break;
    }
    }
    f = dbr->pushField(*f); 
    if (fi != NULL) delete fi;
  }
  catch (PixDBException) {
    ret = false;
  }
  return ret;

}

void ConfMatrix::dump(std::ostream &out) {
  out << "FIELD " << name();

  std::vector<int> value;
  switch (m_subtype) {
  case U16: {
    out << " vector<int>";
    std::vector<unsigned short int> temp;
    ((ConfMask<unsigned short int> *)m_value)->get(temp);
    std::vector<unsigned short int>::iterator it, itEnd=temp.end();
    for(it=temp.begin(); it!=itEnd; it++)
      out << " " << ((int)*it);
    out << std::endl;
    break;
  }
  case U1: {
    out << " vector<bool>";
    std::vector<bool> temp;
    ((ConfMask<bool> *)m_value)->get(temp);
    std::vector<bool>::iterator it, itEnd=temp.end();
    for(it=temp.begin(); it!=itEnd; it++)
      out << " " << ((int)*it);
    out << std::endl;
    break;
  }
  case FLOAT: {
    out << " vector<float>";
    std::vector<float> temp;
    ((ConfMask<float> *)m_value)->get(temp);
    std::vector<float>::iterator it, itEnd=temp.end();
    for(it=temp.begin(); it!=itEnd; it++)
      out << " " << ((float)*it);
    out << std::endl;
    break;
  }
  }
}

/////////////////////
// ConfList methods
/////////////////////

ConfList::ConfList(std::string nam, int &val, int def, std::map<std::string, int>symb, std::string comm, bool vis) :
  ConfInt(nam, val, def, comm, vis, LIST) {
    m_symbols = symb;
}
ConfList::ConfList(std::string nam, unsigned int &val, unsigned int def, std::map<std::string, int>symb, std::string comm, bool vis) :
  ConfInt(nam, val, def, comm, vis, LIST) {
    m_symbols = symb;
}
ConfList::ConfList(std::string nam, short int &val, short int def, std::map<std::string, int>symb, std::string comm, bool vis) :
  ConfInt(nam, val, def, comm, vis, LIST) {
    m_symbols = symb;
}
ConfList::ConfList(std::string nam, unsigned short int &val, unsigned short int def, std::map<std::string, int>symb, std::string comm, bool vis) :
  ConfInt(nam, val, def, comm, vis, LIST) {
    m_symbols = symb;
}
ConfList::ConfList(std::string nam, char &val, char def, std::map<std::string, int>symb, std::string comm, bool vis) :
  ConfInt(nam, val, def, comm, vis, LIST) {
    m_symbols = symb;
}
ConfList::ConfList(std::string nam, unsigned char &val, unsigned char def, std::map<std::string, int>symb, std::string comm, bool vis) :
  ConfInt(nam, val, def, comm, vis, LIST) {
    m_symbols = symb;
}

ConfList::~ConfList() {
}

void ConfList::copy(const ConfObj &obj) {
  try { 
    const ConfList &cObj = dynamic_cast<const ConfList &>(obj);
    if (m_subtype == cObj.m_subtype) {
      switch(m_subtype){
      case S32:
	*((int *)m_value) = *((int *)(cObj.m_value));
	break;
      case U32:
	*((unsigned int *)m_value) = *((unsigned int *)(cObj.m_value));
	break;
      case S16:
	*((short int *)m_value) = *((short int *)(cObj.m_value));
	break;
      case U16:
	*((unsigned short int *)m_value) = *((unsigned short int *)(cObj.m_value));
	break;
      case S8:
	*((char *)m_value) = *((char *)(cObj.m_value));
	break;
      case U8:
	*((unsigned char *)m_value) = *((unsigned char *)(cObj.m_value));
	break;
      default: ;
      }
    }
  }
  catch (std::bad_cast err) {
    return;
  } 
}

bool ConfList::read(DBInquire *dbi) {
  fieldIterator f;
  std::string value;
  int ivalue;
  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  bool stat = dbi->getDB()->DBProcess(f,READ,value);
  if (stat) {
    if (m_symbols.find(value) != m_symbols.end()) {
      ivalue = m_symbols[value];
    } else {
      stat = false;
    }
  }
  if (stat) {
    switch (m_subtype) {
    case S32:
      *((int *)m_value) = ivalue;
      break;
    case U32:  
      *((unsigned int *)m_value) = ivalue;
      break;
    case S16:
      *((short int *)m_value) = ivalue;
      break;
    case U16:
      *((unsigned short int *)m_value) = ivalue;  
      break;
    case S8:
      *((char *)m_value) = ivalue;
      break;
    case U8:
      *((unsigned char *)m_value) = ivalue;
      break;
    }
  }
  return stat;
}
  
bool ConfList::write(DBInquire *dbi) {
  bool stat=false;
  fieldIterator f;
  std::string value;
  int ivalue;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }

  switch (m_subtype) {
  case S32:
    ivalue = (int) *((int *)m_value);
    break;
  case U32:  
    ivalue = (int) *((unsigned int *)m_value);
    break;
  case S16:
    ivalue = (int) *((short int *)m_value);
    break;
  case U16:
    ivalue = (int) *((unsigned short int *)m_value);  
    break;
  case S8:
    ivalue = (int) *((char *)m_value);
    break;
  case U8:
    ivalue = (int) *((unsigned char *)m_value);
    break;
  }

  std::map<std::string, int>::iterator s, sEnd=m_symbols.end();
  for(s=m_symbols.begin(); s!=sEnd; s++) {
    if (s->second == ivalue) {
      value = s->first;
      stat = true;
    }
  }

  if(stat) stat = dbi->getDB()->DBProcess(f,COMMIT,value);

  return stat;
}

bool ConfList::read(DbRecord *dbr) {
  dbFieldIterator f;
  std::string value;
  int ivalue;
  bool stat = true;
  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;

  try {
    dbr->getDb()->DbProcess(f,PixDb::DBREAD,value);
  }
  catch (PixDBException) {
    stat = false;
  }
  if (stat) {
    if (m_symbols.find(value) != m_symbols.end()) {
      ivalue = m_symbols[value];
    } else {
      stat = false;
    }
  }
  if (stat) {
    switch (m_subtype) {
    case S32:
      *((int *)m_value) = ivalue;
      break;
    case U32:  
      *((unsigned int *)m_value) = ivalue;
      break;
    case S16:
      *((short int *)m_value) = ivalue;
      break;
    case U16:
      *((unsigned short int *)m_value) = ivalue;  
      break;
    case S8:
      *((char *)m_value) = ivalue;
      break;
    case U8:
      *((unsigned char *)m_value) = ivalue;
      break;
    }
  }
  return stat;
}
  
bool ConfList::write(DbRecord *dbr) {
  bool stat=false;
  dbFieldIterator f;
  DbField *fi = NULL;
  std::string value;
  int ivalue;

  switch (m_subtype) {
  case S32:
    ivalue = (int) *((int *)m_value);
    break;
  case U32:  
    ivalue = (int) *((unsigned int *)m_value);
    break;
  case S16:
    ivalue = (int) *((short int *)m_value);
    break;
  case U16:
    ivalue = (int) *((unsigned short int *)m_value);  
    break;
  case S8:
    ivalue = (int) *((char *)m_value);
    break;
  case U8:
    ivalue = (int) *((unsigned char *)m_value);
    break;
  }

  std::map<std::string, int>::iterator s, sEnd=m_symbols.end();
  for(s=m_symbols.begin(); s!=sEnd; s++) {
    if (s->second == ivalue) {
      value = s->first;
      stat = true;
    }
  }

  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }
  try {
    dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,ivalue);
    f = dbr->pushField(*f); 
  }
  catch (PixDBException) {
    stat = false;
  }
  if (fi != NULL) delete fi;
  return stat;
}

void ConfList::dump(std::ostream &out) {
  out << "FIELD " << name() << " string " << sValue() << std::endl;
}

std::string ConfList::sValue() {
  int ivalue;
  switch (m_subtype) {
  case S32:
    ivalue = *((int *)m_value);
    break;
  case U32:  
    ivalue = *((unsigned int *)m_value);
    break;
  case S16:
    ivalue = *((short int *)m_value);
    break;
  case U16:
    ivalue = *((unsigned short int *)m_value);  
    break;
  case S8:
    ivalue = *((char *)m_value);
    break;
  case U8:
    ivalue = *((unsigned char *)m_value);
    break;
  }
  std::map<std::string, int>::iterator it;
  for (it = m_symbols.begin(); it != m_symbols.end(); ++it) {
    if ((*it).second == ivalue) {
      return (*it).first;
    } 
  }
  return "????";
}

/////////////////////
// CF_bool methods
/////////////////////

ConfBool::ConfBool(std::string nam, bool &val, bool def, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,BOOL), m_value(val) {
    m_defval = def;
    m_yes = "TRUE";
    m_no = "FALSE";
}
 
ConfBool::ConfBool(std::string nam, bool &val, bool def, std::string y, std::string n, std::string comm, bool vis) :
  ConfObj(nam,comm,vis,BOOL), m_value(val) {
    m_defval = def;
    m_yes = y;
    m_no = n;
}

ConfBool::~ConfBool() {
}

void ConfBool::copy(const ConfObj &obj) {
  try { 
    const ConfBool &cObj = dynamic_cast<const ConfBool &>(obj);
    m_value = cObj.m_value;
  }
  catch (std::bad_cast err) {
    return;
  } 
}

bool ConfBool::read(DBInquire *dbi) {
  fieldIterator f;
  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  return dbi->getDB()->DBProcess(f,READ, m_value);
}
  
bool ConfBool::write(DBInquire *dbi) {

  fieldIterator f;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }

  return dbi->getDB()->DBProcess(f,COMMIT,m_value);
}

bool ConfBool::read(DbRecord *dbr) {
  dbFieldIterator f;
  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;
  try {
    dbr->getDb()->DbProcess(f,PixDb::DBREAD,m_value);
  }
  catch (PixDBException) {
    return false;
  }
  return true;
}
  
bool ConfBool::write(DbRecord *dbr) {
  dbFieldIterator f;
  DbField *fi = NULL;
  bool ret = true;
  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }
  try {
    dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,m_value);
    f = dbr->pushField(*f); 
  }
  catch (PixDBException) {
    ret= false;
  }
  if (fi != NULL) delete fi;
  return ret;
}

void ConfBool::dump(std::ostream &out) {
  out << "FIELD " << name() << " bool " << (int)m_value << std::endl;
}

/////////////////////
// CF_string methods
/////////////////////

ConfString::ConfString(std::string nam, std::string &val, std::string def, std::string comm, bool vis, int type) :
  ConfObj(nam,comm,vis,STRING), m_value(val) {
    m_defval = def;
    m_type = type;
}
  
ConfString::~ConfString() {
}

void ConfString::copy(const ConfObj &obj) {
  try { 
    const ConfString &cObj = dynamic_cast<const ConfString &>(obj);
    m_value = cObj.m_value;
  }
  catch (std::bad_cast err) {
    return;
  } 
}

bool ConfString::read(DBInquire *dbi) {
  fieldIterator f;
  f = dbi->findField(m_name);
  if(f==dbi->fieldEnd()) return false;

  return dbi->getDB()->DBProcess(f,READ,m_value);
}
  
bool ConfString::write(DBInquire *dbi) {
  fieldIterator f;
  f = dbi->findField(m_name);
  if (f==dbi->fieldEnd()) {
    DBField *fi = dbi->getDB()->makeField(m_name);
    f.pointsTo(fi);
    f = dbi->pushField(*f); 
    delete fi;
  }

  return dbi->getDB()->DBProcess(f,COMMIT,m_value);
}

bool ConfString::read(DbRecord *dbr) {
  dbFieldIterator f;
  f = dbr->findField(m_name);
  if(f==dbr->fieldEnd()) return false;
  try {
    dbr->getDb()->DbProcess(f,PixDb::DBREAD,m_value);
  }
  catch (PixDBException) {
    return false;
  }
  return true;
}
  
bool ConfString::write(DbRecord *dbr) {
  dbFieldIterator f;
  DbField *fi = NULL;
  bool ret = true;
  f = dbr->findField(m_name);
  if (f==dbr->fieldEnd()) {
    fi = dbr->getDb()->makeField(m_name);
    f.pointsTo(fi);
  }
  try {
    dbr->getDb()->DbProcess(f,PixDb::DBCOMMIT,m_value);
    f = dbr->pushField(*f); 
  }
  catch (PixDBException) {
    ret = false;
  } 
  if (fi != NULL) delete fi;
  return ret;
}

void ConfString::dump(std::ostream &out) {
  out << "FIELD " << name() << " string " << m_value << std::endl;
}
