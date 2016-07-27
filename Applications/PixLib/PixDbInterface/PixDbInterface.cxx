/////////////////////////////////////////////////////////////////////
// PixDbInterface.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 23/06/06  Version 1.0 (GG)
//           Initial release
//
//! Implementatin of methods for the abstract classes for the PixLib Database handling

#include "Histo/Histo.h"
#include "PixDbInterface/PixDbInterface.h"
#include <sstream>

using namespace PixLib;
//using namespace PixLib::PixDb;

std::ostream& PixLib::operator<<(std::ostream& os, const DbField& data) {
  data.dump(os);
  return os;
}

std::ostream& PixLib::operator<<(std::ostream& os, const DbRecord& data) {
  data.dump(os);
  return os;
}

DbField* dbFieldIterator::operator*() const {
    return m_field;
}

DbField** dbFieldIterator::operator->() {
  return &m_field;
}

dbFieldIterator dbFieldIterator::operator++(int /*i*/){
//  DbRecord* record = m_field->getParentRecord();
	DbRecord* record = m_parentRecord;
  if((m_number + 1) > record->getDependingFieldSize()){
    std::stringstream a;
    a << "dbFieldIterator::operator++() : field number " << (m_number+1) << " not existing in record: ";
    a << m_parentRecord;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number + 1) == record->getDependingFieldSize()){
    *this = record->fieldEnd();
    return *this;
  }
  else{
    m_number = record->getField(m_number+1).m_number;
    m_field = record->getField(m_number).m_field;
    return *this;
  }
}

dbFieldIterator& dbFieldIterator::operator++(){
//  DbRecord* record = m_field->getParentRecord();
	DbRecord* record = m_parentRecord;
	if((m_number + 1) > record->getDependingFieldSize()){
    std::stringstream a;
    a << "dbFieldIterator::operator++() : field number " << (m_number+1) << " not existing in record: ";
    a << m_parentRecord;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number +1) == record->getDependingFieldSize()){
    *this = record->fieldEnd();
    return *this;
  } 
  else{
    m_number = record->getField(m_number+1).m_number;
    m_field = record->getField(m_number).m_field;
    return *this;
  }
}

bool PixLib::operator==(const dbFieldIterator& left,const dbFieldIterator& right){
  if((left.m_field == right.m_field) && (left.m_parentRecord == right.m_parentRecord) &&
     (left.m_number == right.m_number))
    return true;
  return false;
}

bool PixLib::operator!=(const dbFieldIterator& left,const dbFieldIterator& right){
  if(left == right) return false;
  else return true;
}

void dbFieldIterator::pointsTo(DbField* newField){ // make the iterator point to another field
	this->m_field = newField;
}

dbFieldIterator::~dbFieldIterator(){ /* if(m_field) delete m_field; */}

DbRecord* dbRecordIterator::operator*() const {
  return m_record;
}

DbRecord** dbRecordIterator::operator->() {
  return &m_record;
}

dbRecordIterator dbRecordIterator::operator++(int /*i*/){
  if((m_number + 1) > m_parentRecord->getDependingRecordSize()){
    std::stringstream a;
    a << "dbRecordIterator::operator++() : record number " << (m_number+1) << " not existing in record: ";
    a << m_parentRecord;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number +1) == m_parentRecord->getDependingRecordSize()){
    *this = m_parentRecord->recordEnd();
    return *this;
  }
  else{
    m_number = m_parentRecord->getRecord(m_number+1).m_number;
    m_record = m_parentRecord->getRecord(m_number).m_record;
    return *this;
  }
}

dbRecordIterator& dbRecordIterator::operator++(){
  if((m_number + 1) > m_parentRecord->getDependingRecordSize()){
    std::stringstream a;
    a << "dbRecordIterator::operator++() : record number " << (m_number+1) << " not existing in record: ";
    a << m_parentRecord;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number+1) == m_parentRecord->getDependingRecordSize()){
    *this = m_parentRecord->recordEnd();
    return *this;
  }
  else{
    m_number = m_parentRecord->getRecord(m_number+1).m_number;
    m_record = m_parentRecord->getRecord(m_number).m_record;
    return *this;
  }
}

bool PixLib::operator==(const dbRecordIterator& left,const dbRecordIterator& right){
  if((left.m_record == right.m_record) && (left.m_parentRecord == right.m_parentRecord) &&
     (left.m_number == right.m_number))
    return true;
  return false;
}

bool PixLib::operator!=(const dbRecordIterator& left,const dbRecordIterator& right){
  if(left == right) return false;
  else return true;
}

void dbRecordIterator::pointsTo(DbRecord* newRecord){ // make the iterator point to another record
	this->m_record = newRecord;
}


void dbFieldIterator::copyData(const dbFieldIterator fieldFromCopy){
	int i_copyvalue = 0;
	float f_copyvalue = 0;
	double d_copyvalue = 0;
	bool b_copyvalue = false;
	std::vector<int> vi_copyvalue;
	std::vector<float> vf_copyvalue;
	std::vector<double> vd_copyvalue;
	std::vector<bool> vb_copyvalue;
	std::string s_copyvalue;
	Histo h_copyvalue;
	stringstream a;
	switch(this->m_field->getDataType()){
	case DBINT:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,i_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,i_copyvalue);
		break;
	case DBFLOAT:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,f_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,f_copyvalue);
		break;
	case DBDOUBLE:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,d_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,d_copyvalue);
		break;
	case DBSTRING:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,s_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,s_copyvalue);
		break;
	case DBBOOL:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,b_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,b_copyvalue);
		break;
	case DBVECTORINT:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,vi_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,vi_copyvalue);
		break;
	case DBVECTORFLOAT:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,vf_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,vf_copyvalue);
		break;
	case DBVECTORBOOL:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,vb_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,vb_copyvalue);
		break;
	case DBVECTORDOUBLE:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,vd_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,vd_copyvalue);
		break;
	case DBHISTO:
		fieldFromCopy.m_field->getDb()->DbProcess(fieldFromCopy,DBREAD,h_copyvalue);
		this->m_field->getDb()->DbProcess(*this,DBCOMMIT,h_copyvalue);
		break;
	case DBEMPTY: default:
		a << "dbFieldIterator::copyData() : copying empty or not supported data type ";
		a << DbDataTypeNames[this->m_field->getDataType()];
		throw PixDBException(a.str().c_str());

	}	
}

 dbRecordIterator::~dbRecordIterator(){/* if(m_record) delete m_record; */}
