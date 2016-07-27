/////////////////////////////////////////////////////////////////////
// PixConfDBInterface.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 13/10/03  Version 1.0 (GG)
//           Initial release
//
//! Implementatin of methods for the abstract classes for the PixLib Database handling
#include "Histo/Histo.h"
#include "PixConfDBInterface.h"
#include <sstream>

using namespace PixLib;

std::ostream& PixLib::operator<<(std::ostream& os, const DBField& data) {
  data.dump(os);
  return os;
}

std::ostream& PixLib::operator<<(std::ostream& os, const DBInquire& data) {
  data.dump(os);
  return os;
}

DBField* fieldIterator::operator*() const {
  return m_field;
}

DBField** fieldIterator::operator->() {
  return &m_field;
}

fieldIterator fieldIterator::operator++(int){
//  DBInquire* inquire = m_field->getParentInquire();
	const DBInquire* inquire = m_parentInquire;
  if((m_number + 1) > inquire->getDependingFieldSize()){
    std::stringstream a;
    a << "fieldIterator::operator++() : field number " << (m_number+1) << " not existing in inquire: ";
    a << m_parentInquire;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number + 1) == inquire->getDependingFieldSize()){
    *this = inquire->fieldEnd();
    return *this;
  }
  else{
    m_number = inquire->getField(m_number+1).m_number;
    m_field = inquire->getField(m_number).m_field;
    return *this;
  }
}

fieldIterator& fieldIterator::operator++(){
//  DBInquire* inquire = m_field->getParentInquire();
	const DBInquire* inquire = m_parentInquire;
	if((m_number + 1) > inquire->getDependingFieldSize()){
    std::stringstream a;
    a << "fieldIterator::operator++() : field number " << (m_number+1) << " not existing in inquire: ";
    a << m_parentInquire;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number +1) == inquire->getDependingFieldSize()){
    *this = inquire->fieldEnd();
    return *this;
  } 
  else{
    m_number = inquire->getField(m_number+1).m_number;
    m_field = inquire->getField(m_number).m_field;
    return *this;
  }
}

bool PixLib::operator==(const fieldIterator& left,const fieldIterator& right){
  if((left.m_field == right.m_field) && (left.m_parentInquire == right.m_parentInquire) &&
     (left.m_number == right.m_number))
    return true;
  return false;
}

bool PixLib::operator!=(const fieldIterator& left,const fieldIterator& right){
  if(left == right) return false;
  else return true;
}

void fieldIterator::pointsTo(DBField* newField){ // make the iterator point to another field - lost ANY relationship with possible parent inquire and number
	this->m_field = newField;
	this->m_number = -1;
	this->m_parentInquire = 0;
}

DBInquire* recordIterator::operator*() const {
  return m_record;
}

DBInquire** recordIterator::operator->() {
  return &m_record;
}

recordIterator recordIterator::operator++(int){
  if((m_number + 1) > m_parentInquire->getDependingInquireSize()){
    std::stringstream a;
    a << "recordIterator::operator++() : record number " << (m_number+1) << " not existing in inquire: ";
    a << m_parentInquire;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number +1) == m_parentInquire->getDependingInquireSize()){
    *this = m_parentInquire->recordEnd();
    return *this;
  }
  else{
    m_number = m_parentInquire->getInquire(m_number+1).m_number;
    m_record = m_parentInquire->getInquire(m_number).m_record;
    return *this;
  }
}

recordIterator& recordIterator::operator++(){
  if((m_number + 1) > m_parentInquire->getDependingInquireSize()){
    std::stringstream a;
    a << "recordIterator::operator++() : record number " << (m_number+1) << " not existing in inquire: ";
    a << m_parentInquire;
    //return 0;
    throw PixDBException(a.str().c_str());    
  }
  else if((m_number+1) == m_parentInquire->getDependingInquireSize()){
    *this = m_parentInquire->recordEnd();
    return *this;
  }
  else{
    m_number = m_parentInquire->getInquire(m_number+1).m_number;
    m_record = m_parentInquire->getInquire(m_number).m_record;
    return *this;
  }
}

bool PixLib::operator==(const recordIterator& left,const recordIterator& right){
  if((left.m_record == right.m_record) && (left.m_parentInquire == right.m_parentInquire) &&
     (left.m_number == right.m_number))
    return true;
  return false;
}

bool PixLib::operator!=(const recordIterator& left,const recordIterator& right){
  if(left == right) return false;
  else return true;
}

void recordIterator::pointsTo(DBInquire* newInquire){ // make the iterator point to another inquire - lost ANY relationship with possible parent inquire
	this->m_record = newInquire;
	this->m_number = -1;
	this->m_parentInquire = 0;
}


void fieldIterator::copyData(const fieldIterator fieldFromCopy){
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
	case INT:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,i_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,i_copyvalue);
		break;
	case FLOAT:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,f_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,f_copyvalue);
		break;
	case DOUBLE:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,d_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,d_copyvalue);
		break;
	case STRING:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,s_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,s_copyvalue);
		break;
	case BOOL:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,b_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,b_copyvalue);
		break;
	case VECTORINT:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,vi_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,vi_copyvalue);
		break;
	case VECTORFLOAT:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,vf_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,vf_copyvalue);
		break;
	case VECTORBOOL:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,vb_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,vb_copyvalue);
		break;
	case VECTORDOUBLE:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,vd_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,vd_copyvalue);
		break;
	case HISTO:
		fieldFromCopy.m_field->getDB()->DBProcess(fieldFromCopy,READ,h_copyvalue);
		this->m_field->getDB()->DBProcess(*this,COMMIT,h_copyvalue);
		break;
	case EMPTY: default:
		a << "fieldIterator::copyData() : copying empty or not supported data type ";
		a << DBDataTypeNames[this->m_field->getDataType()];
		throw PixDBException(a.str().c_str());

	}	
}
