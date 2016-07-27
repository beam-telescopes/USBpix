////////////////////////////////////////////////////////////////////
// RootDB.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 13/10/03  Version 1.0 (GG)
//           Initial release
//
//! Data Base implementation of RootDB.h
#include "PixConfDBInterface/RootDB.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
//#include "Histo/Histo.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
using std::stringstream;
typedef unsigned int uint;


ClassImp(RootDBField)
ClassImp(RootDBInquire)
ClassImp(RootDB)



static string purgeSlash(string in){
	string retval(in);
	if(in.at(in.size()-1) == '/'){
		retval =  in.substr(0,in.size()-1);
	}
	return retval;
}

string getMySubDir(DBInquire* in){
//	if(in == in->getDB()->readRootRecord(in->getIteration())) return "/";
	string retval (in->getDecName()); 

	if(retval == "/") { // handles the root record decorated name
	  std::cout << "retval == /" << std::endl;
	  return retval;
	}

	if(retval.size() != 0){
	  int pos = retval.find_last_of("/",in->getDecName().length()-2);
	  retval.erase(0,pos+1);
	  return retval;
	}
	else 
	  {
	    stringstream a;
	    a << "In RootDB.cxx getMySubDir: input inquire has decorated name lenght 0";
	    throw PixDBException(a.str());
	  }

}

string getMyVersionName(DBInquire* in){
	int pos = in->getMyDecName().find_last_of("/");
	return in->getMyDecName().substr(pos+1);
}


ULong_t RootDBField::Hash() const { return TMath::Hash(m_decName.c_str()); }

Bool_t  RootDBField::IsEqual(const TObject *obj) const {
		if (dynamic_cast<RootDBField*>(const_cast<TObject*>(obj)) == NULL) return kFALSE;
		if(m_decName == dynamic_cast<RootDBField*>(const_cast<TObject*>(obj))->m_decName) return kTRUE; else return kFALSE;
	}

Bool_t  RootDBField::IsSortable() const { return kTRUE; }

Int_t   RootDBField::Compare(const TObject *obj) const {
		if (dynamic_cast<RootDBField*>(const_cast<TObject*>(obj)) == NULL) {
			std::stringstream a;
		  a << "RootDBField::Compare(): Bad comparison";
	      throw PixDBException(a.str());
		}
		ULong_t h1,h2;
		h1 = TMath::Hash(m_decName.c_str());
		h2 = TMath::Hash(dynamic_cast<RootDBField*>(const_cast<TObject*>(obj))->m_decName.c_str());
		if (h1 > h2) return 1;
        else if (h1 < h2) return -1;
        else return 0; }

ULong_t RootDBInquire::Hash() const {
	return TMath::Hash(m_myDecName.c_str());
}

Bool_t  RootDBInquire::IsEqual(const TObject *obj) const {
		if (dynamic_cast<RootDBInquire*>(const_cast<TObject*>(obj)) == NULL) return kFALSE;
		if(m_myDecName == dynamic_cast<RootDBInquire*>(const_cast<TObject*>(obj))->m_myDecName) return kTRUE; else return kFALSE;
	}

Bool_t  RootDBInquire::IsSortable() const { return kTRUE; }

Int_t   RootDBInquire::Compare(const TObject *obj) const {
		if (dynamic_cast<RootDBInquire*>(const_cast<TObject*>(obj)) == NULL) {
			std::stringstream a;
		  a << "RootDBInquire::Compare(): Bad comparison";
	      throw PixDBException(a.str());
		}
		ULong_t h1,h2;
		h1 = TMath::Hash(m_myDecName.c_str());
		RootDBInquire* tr =  dynamic_cast<RootDBInquire*>(const_cast<TObject*>(obj));
		h2 = TMath::Hash(tr->m_myDecName.c_str());
		if (h1 > h2) return 1;
        else if (h1 < h2) return -1;
        else return 0; }

PixConfDBInterface* RootDBInquire::getDB() const {return m_myDB;}


std::vector<DBField*> RootDB::DBFindFieldByName(enum DBFindMode /*mode*/, const string& name) {
	vector<DBField*> retvalue; 
	TDirectory* dir = gDirectory; // copy locally the current directory
	m_theFile->cd(); // go to the root directory
	RootDBField* in = NULL;
	in = getFieldByDecName(name);
	if(in == 0){
	  stringstream a;
	  a << "RootDB::DBFindFieldByName(): field" << name.c_str() << " not found";
      throw PixDBException(a.str());
	}
	retvalue.push_back(in);
	dir->cd();
	return retvalue;
} // find a data field by its name, returning a vector of fields which fulfill the find requests

std::vector<DBInquire*> RootDB::DBFindRecordByName(enum DBFindMode /*mode*/, const string& name) {
	std::vector<DBInquire*> retvalue;
	TDirectory* dir = gDirectory; // copy locally the current directory
	m_theFile->cd(); // go to the root directory
	RootDBInquire* in = NULL;
	in = getInquireByDecName(name);
	if(in == 0){
	  stringstream a;
	  a << "RootDB::DBFindRecordByName(): inquire" << name.c_str() << " not found";
      throw PixDBException(a.str());
	}
	retvalue.push_back(in);
	dir->cd();
	return retvalue;
} // find a record by its name, returning a vector of records which fulfill the find requests

recordIterator RootDBInquire::recordBegin() const {
	if(m_myDB->m_theFile->IsZombie()){
      throw PixDBException("RootDBInquire::recordBegin(): DataBase File not good");
	}
	if (m_myRecordNameList.size() == 0) return recordEnd();
	return getInquire(0);
} // the iterator to the depending records		

fieldIterator RootDBInquire::fieldBegin() const {	
	if(m_myDB->m_theFile->IsZombie()){
      throw PixDBException("RootDBInquire::fieldBegin(): DataBase File not good");
	}
	if (m_myFieldNameList.size() == 0) return fieldEnd();
	return getField(0);
} // the iterator to the record data fields	

recordIterator  RootDBInquire::recordEnd() const {
	if(m_myDB->m_theFile->IsZombie()){
      throw PixDBException("RootDBInquire::recordEnd(): DataBase File not good");
	}
	recordIterator r(m_myRecordNameList.size(),NULL,this);
	return r;
} // the end iterator to the depending records	

fieldIterator  RootDBInquire::fieldEnd() const {
	if(m_myDB->m_theFile->IsZombie()){
      throw PixDBException("RootDBInquire::fieldEnd(): DataBase File not good");
	}
	fieldIterator r(m_myFieldNameList.size(),NULL,this);
	return r;
} // the end iterator to the record data fields	

int RootDBInquire::getDependingInquireSize(void) const {return m_myRecordNameList.size();} // RootDBInquire::Get the amount of depending inquires

int RootDBInquire::getDependingFieldSize(void) const{return m_myFieldNameList.size();} // Get the amount of depending field


fieldIterator RootDBInquire::getField(int fieldNumber) const {
  uint in = fieldNumber;
  if(in > m_myFieldNameList.size()){
    std::stringstream a;
    a << "TurboDaqDBInquire::getField(int fieldNumber): inquire number " << in << " not found in inquire " << getDecName().c_str() << getMyDecName().c_str();
    throw PixDBException(a.str().c_str());    
  }
  else if(in == m_myFieldNameList.size()){
    return fieldEnd();
  }
  else{
	string fieldName = this->getDecName();
	// JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
	if(fieldName.length()==0) fieldName = "/";	
	else if(fieldName[fieldName.length()-1] != '/') fieldName += "/";	
	fieldName += this->m_myFieldNameList[in];
	fieldIterator r(fieldNumber,m_myDB->DBFindFieldByName(BYDECNAME, fieldName)[0],this); 
	return r;
  }
} // Get the pointer to the field number fieldNumber; throw an exception if fails

recordIterator RootDBInquire::getInquire(int inquireNumber) const {
  uint in = inquireNumber;
  if(in > m_myRecordNameList.size()){
    std::stringstream a;
    a << "RootDBInquire::getInquire(int inquireNumber): inquire number " << in << " not found in inquire " << getDecName().c_str() << getMyDecName().c_str();
    throw PixDBException(a.str().c_str());    
  }
  else if(in == m_myRecordNameList.size()){
    return recordEnd();
  }
  else{
	string inquireName = this->getDecName();
	// JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
	if(inquireName.length()==0) inquireName = "/";	
	else if(inquireName[inquireName.length()-1] != '/') inquireName += "/";	
	inquireName += this->m_myRecordNameList[in];
	recordIterator r(inquireNumber,m_myDB->DBFindRecordByName(BYDECNAME, inquireName)[0],this); 
	return r;
  }
} // Get the pointer to the inquire number inquireNumber; throw an exception if fails

string stripIteration(string name){
	string retval = name;
  int resize = retval.find_last_of(";");
  if(resize != (int)std::string::npos)
	  retval.resize(resize);
  return retval;
}

int readIteration(string name){
	int resize = name.find_last_of(";");
	string number = name.substr(resize+1, name.size());
	return atoi(number.c_str());
}


recordIterator RootDBInquire::findRecord(const string recordName) const {
  uint it;

  // try first with versioned names; if not found, try with unversioned names
  for(it = 0; it < m_myRecordNameList.size(); it++)
    {
      if(m_myRecordNameList[it] == recordName) break;
    }
  if(it == m_myRecordNameList.size()){
	  for(it = 0; it < m_myRecordNameList.size(); it++)
	  {
		  if(stripIteration(m_myRecordNameList[it]) == stripIteration(recordName)) break;
	  }
	  if(it == m_myRecordNameList.size()){
//    std::stringstream a;
//    a << recordName.c_str() << " not found in method findRecord";
//    throw PixDBException(a.str().c_str());    
		  return recordEnd();
	  }
	  else return getInquire(it);
  }
  return getInquire(it);

}; // find a subrecord by its name

fieldIterator RootDBInquire::findField(const string fieldName) const {
  uint it;
  for(it = 0; it < m_myFieldNameList.size(); it++)
    {
      if(stripIteration(m_myFieldNameList[it]) == fieldName) break;
    }
  if(it == m_myFieldNameList.size()){
//    std::stringstream a;
//   a << fieldName.c_str() << " not found in method findField";
//    throw PixDBException(a.str().c_str());    
	  return fieldEnd();
  }
  return getField(it);
}; // find a field by its name 

recordIterator RootDBInquire::findRecord(const char* name) const{
	string sname(name);
	return findRecord(sname);
}; // find a subrecord by its name

fieldIterator RootDBInquire::findField(const char* name) const{
	string sname(name);
	return findField(sname);
}; // find a field by its name 

DBInquire* RootDBInquire::addInquire(string name, string dirName) {
  //std::cout << "looking for record " << dirName.c_str() << "/" << name.c_str() << std::endl << "in inquire " << *this;
        recordIterator ri = findRecord(dirName+"/"+name);
        if (ri == recordEnd()) {
	  //std::cout << "record not found" << std::endl;
	  //	  DBInquire *newInq = new RootDBInquire(name, getDecName()+dirName, getDecName()+dirName+"/"+name, m_myDB);
	  //std::cout << "creating new inquire with name " << name.c_str() << "and dec name " << (getDecName()+dirName).c_str() << std::endl; 
	  DBInquire *newInq = m_myDB->makeInquire(name, getDecName()+dirName);
	  pushRecord(newInq);
	  m_myDB->DBProcess(this,COMMITREPLACE);
	  m_myDB->DBProcess(newInq,COMMIT);
	  //std::cout << "dump the parent record with the added inquire" << std::endl << *this;
	  return newInq;
	}
	//std::cout << "record found: dumping it" << std::endl << **ri;
	return *ri;
}

void RootDBField::copyField(fieldIterator f){
  DBField* ft = *f;
  // std::cout << "copying field named " << ft->getName() << std::endl; 
  SetName(ft->getName().c_str());
  m_dataType = ft->getDataType();
  fieldIterator fiter(0,this,0);
  switch(m_dataType){
  case INT:
    {int valInt;
    ft->getDB()->DBProcess(f,READ,valInt);
    m_myIntCont.Set(1);
    m_myIntCont.AddAt(valInt,0);
    break;}
  case VECTORINT:
    {vector<int> valVInt;
    ft->getDB()->DBProcess(f,READ,valVInt);
    m_myIntCont.Set(valVInt.size());
    for(size_t i = 0; i < valVInt.size(); i++){
      m_myIntCont.AddAt(valVInt[i],i);      
    }}
    break;
  case ULINT:
    {unsigned int valUInt;
    ft->getDB()->DBProcess(f,READ,valUInt);
	stringstream s;
	s << valUInt;
    m_myStringCont = s.str().c_str();
    break;}
  case FLOAT:
    {float valFloat;
    ft->getDB()->DBProcess(f,READ,valFloat);
    m_myFloatCont.Set(1);
    m_myFloatCont.AddAt(valFloat,0);
    break;}
  case VECTORFLOAT:
    {vector<float> valVFloat;
    ft->getDB()->DBProcess(f,READ,valVFloat);
    m_myFloatCont.Set(valVFloat.size());
    for(size_t i = 0; i < valVFloat.size(); i++){
      m_myFloatCont.AddAt(valVFloat[i],i);      
    }}
    break;
  case DOUBLE:
    {double valDouble;
    ft->getDB()->DBProcess(f,READ,valDouble);
    m_myDoubleCont.Set(1);
    m_myDoubleCont.AddAt(valDouble,0);
    break;}
  case VECTORDOUBLE:
    {vector<double> valVDouble;
    ft->getDB()->DBProcess(f,READ,valVDouble);
    m_myDoubleCont.Set(valVDouble.size());
    for(size_t i = 0; i < valVDouble.size(); i++){
      m_myDoubleCont.AddAt(valVDouble[i],i);      
    }}
    break;
  case BOOL:
    {bool valBool;
    ft->getDB()->DBProcess(f,READ,valBool);
    m_myIntCont.Set(1);
    m_myIntCont.AddAt(valBool,0);
    break;}
  case VECTORBOOL:
    {vector<bool> valVBool;
    ft->getDB()->DBProcess(f,READ,valVBool);
    m_myIntCont.Set(valVBool.size());
    for(size_t i = 0; i < valVBool.size(); i++){
      m_myIntCont.AddAt(valVBool[i],i);      
    }}
    break;
  case HISTO:
    {Histo valHisto;
    ft->getDB()->DBProcess(f,READ,valHisto);
    this->getDB()->DBProcess(fiter,COMMIT,valHisto);
    break;}
  case STRING:
    {string valString;
    ft->getDB()->DBProcess(f,READ,valString);
    m_myStringCont = valString.c_str();
    }
    break;
  default:
    {string error;
    error += "RootDBField::copyField error: type not processed. Type is ";
    error += DBDataTypeNames[m_dataType];
    throw PixDBException(error);}
  }
}

DBField* RootDB::makeField(string name){
	return new RootDBField(name,this);
}

DBInquire* RootDB::makeInquire(string name, string decName){
  string myDecName = decName;
  if(decName.size() != 0){
    if(decName.at(decName.size()-1) != '/'){
      myDecName = decName + "/";
      decName += "/";
    }
  }
  myDecName += name;
  return new RootDBInquire(name, decName, myDecName, this);
}

RootDBInquire::RootDBInquire(string name, string decName, string myDecName, PixConfDBInterface* mydb ) : 
m_name(name),
m_decName(decName),
m_myDecName(myDecName)
{
	if((m_myDB = dynamic_cast<RootDB*>(mydb)) == 0) std::cout << "Warning: initialize RootDBInquire with a non RootDB database" << std::endl;
}

RootDBInquire::RootDBInquire(string name, PixConfDBInterface* mydb ) : m_name(name){
	if((m_myDB = dynamic_cast<RootDB*>(mydb)) == 0) std::cout << "Warning: initialize RootDBInquire with a non RootDB database" << std::endl;
}

RootDBInquire::RootDBInquire(string decname) : m_myDecName(decname){}

RootDBField::RootDBField(string decname) : m_decName(decname){
	this->m_myFloatCont.Set(1);
	this->m_myDoubleCont.Set(1);
	this->m_myIntCont.Set(1);
}

string RootDB::getAbsolutePath(TDirectory* currDir){
	// get the current path, and take out the file name in order to make inquires file independents
	string path = currDir->GetPath();
	int colonPos = path.find(":");
#ifdef WIN32 // in windows, fist ":" is most likely indicating the drive, not the begin of root's internal name...
	if(colonPos==1){ 
	  path.erase(0,colonPos+1);
	  colonPos = path.find(":");
	}
#endif
	path.erase(0,colonPos+1);
	return path;
}

DBInquire* RootDB::processInquire(DBInquire* currInquire){
  RootDBInquire* rootInquire = new RootDBInquire(currInquire->getName(), this);
  rootInquire->m_decName = getAbsolutePath(gDirectory);
  // JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
  if(rootInquire->m_decName.length()==0) rootInquire->m_decName = "/";
  else if(rootInquire->m_decName[rootInquire->m_decName.length()-1] != '/') rootInquire->m_decName += "/";
  rootInquire->m_myDecName = rootInquire->m_decName + rootInquire->m_name;
  TKey* k = gDirectory->GetKey(rootInquire->getName().c_str());
  if(k){
    int version = k->GetCycle();
	rootInquire->getMyDecName() += ";"; 
    stringstream s;
    s << version+1;   
	rootInquire->getMyDecName() += s.str();
  }
  else{
   int version = 1;
   rootInquire->getMyDecName() += ";"; 
   stringstream s;
   s << version;   
   rootInquire->getMyDecName() += s.str();
  } 
  fieldIterator f = currInquire->fieldBegin();
  for(;f!=currInquire->fieldEnd();f++){
    RootDBField* dbf = new RootDBField((*f)->getName(), this);
    dbf->copyField(f);
	dbf->getDecName() = rootInquire->m_decName;
	dbf->getDecName() += dbf->getName();
    TKey* k = gDirectory->GetKey(dbf->getName().c_str());
    if(k){
      int version = k->GetCycle();
      stringstream s;
      s << ";";
      s << version+1;    
      dbf->getDecName() += s.str();
      rootInquire->m_myFieldNameList.push_back(dbf->getName() + s.str());
    }
    else{
      int version = 1;
      stringstream s;
      s << ";";
      s << version;    
      dbf->getDecName() += s.str();
      rootInquire->m_myFieldNameList.push_back(dbf->getName() + s.str());
    }
    dbf->Write(dbf->getName().c_str());
    delete dbf;
  }

  recordIterator r = currInquire->recordBegin();
  for(;r != currInquire->recordEnd();r++){
	  if(!gDirectory->GetKey((*r)->getMyDecName().c_str())){
		  gDirectory->mkdir((*r)->getMyDecName().c_str());
	  }
      gDirectory->cd((*r)->getMyDecName().c_str());
    TKey* k = gDirectory->GetKey((*r)->getName().c_str());
    if(k){
      int version = k->GetCycle();
      stringstream s;
      s << (*r)->getMyDecName();
      s << "/" ;
      s << (*r)->getName();
      s << ";";
      s << version+1;    
      rootInquire->m_myRecordNameList.push_back(s.str());
    }
    else{
      int version = 1;
      stringstream s;
      s << (*r)->getMyDecName();
      s << "/" ;
      s << (*r)->getName();
      s << ";";
      s << version;    
      rootInquire->m_myRecordNameList.push_back(s.str());
    }
    processInquire(*r);
  }
  
  rootInquire->Write(rootInquire->getName().c_str());
  gDirectory->cd("..");
  return rootInquire;
}

void RootDB::processRootInquireReplace(DBInquire* currInquire){
  RootDBInquire* rootInquire = dynamic_cast<RootDBInquire*>(currInquire);
  RootDBInquire * memInquire;
  uint i = 0;
  for(i = 0; i < rootInquire->m_myFieldNameList.size(); i++){
    fieldIterator f = rootInquire->getField(i);
    RootDBField* tmpField = dynamic_cast<RootDBField*>(*f);
    if(tmpField->m_modified == true){
      if(tmpField->getDataType() == HISTO){
	std::vector<double> tmpvec;
	if(tmpField->m_myHistoCont.nDim() != 0){
	  tmpvec = histoToVector(tmpField->m_myHistoCont);
	  tmpField->m_myDoubleCont.Set(tmpvec.size());
	  for(uint ii = 0; ii < tmpvec.size(); ii++){
	    tmpField->m_myDoubleCont[ii] = tmpvec[ii];
	  }
	}
	tmpField->Write(stripIteration(rootInquire->m_myFieldNameList[i]).c_str(),TObject::kOverwrite);
	m_theFile->SaveSelf(kTRUE); // to actually write to disk also the keys
	tmpField->m_myDoubleCont.Set(0);
	Histo tmpHisto;
      tmpField->m_myHistoCont = tmpHisto;
      }
      else{
	dynamic_cast<RootDBField*>(*f)->Write(stripIteration(rootInquire->m_myFieldNameList[i]).c_str(),TObject::kOverwrite);
      }
      tmpField->m_modified = false;
    }
  }
  rootInquire->Write(rootInquire->getName().c_str(),TObject::kOverwrite);
  memInquire = (RootDBInquire*)m_theLoadedInquires.FindObject(rootInquire);
  (*memInquire) = *(dynamic_cast<RootDBInquire*>(currInquire));
}

void RootDB::processRootInquireReplaceTree(DBInquire* currInquire){
  RootDBInquire* rootInquire = dynamic_cast<RootDBInquire*>(currInquire);
  uint i = 0;
  for(i = 0; i < rootInquire->m_myFieldNameList.size(); i++){
    fieldIterator f = rootInquire->getField(i);
    RootDBField* tmpField = dynamic_cast<RootDBField*>(*f);
    if(tmpField->m_modified == true){
      if(tmpField->getDataType() == HISTO){
	std::vector<double> tmpvec;
	if(tmpField->m_myHistoCont.nDim() != 0){
	  tmpvec = histoToVector(tmpField->m_myHistoCont);
	  tmpField->m_myDoubleCont.Set(tmpvec.size());
	  for(uint ii = 0; ii < tmpvec.size(); ii++){
	    tmpField->m_myDoubleCont[ii] = tmpvec[ii];
	  }
	}
	tmpField->Write(stripIteration(rootInquire->m_myFieldNameList[i]).c_str(),TObject::kOverwrite);
	m_theFile->SaveSelf(kTRUE); // to actually write to disk also the keys
	tmpField->m_myDoubleCont.Set(0);
	Histo tmpHisto;
	tmpField->m_myHistoCont = tmpHisto;
      }
      else{
	dynamic_cast<RootDBField*>(*f)->Write(stripIteration(rootInquire->m_myFieldNameList[i]).c_str(),TObject::kOverwrite);
      }
      tmpField->m_modified = false;
    }    
    dynamic_cast<RootDBField*>(*f)->Write(stripIteration(rootInquire->m_myFieldNameList[i]).c_str(),TObject::kOverwrite);
  }
  rootInquire->Write(rootInquire->getName().c_str(),TObject::kOverwrite);
  for(i = 0; i < rootInquire->m_myRecordNameList.size(); i++){
    recordIterator r = rootInquire->getInquire(i);
    if(!gDirectory->GetKey(purgeSlash(getMySubDir(*r)).c_str())) gDirectory->mkdir(purgeSlash(getMySubDir(*r)).c_str());
    gDirectory->cd(getMySubDir(*r).c_str());
    processRootInquireReplaceTree(*r);
    gDirectory->cd("..");
  }
}

void RootDB::processRootInquire(DBInquire* currInquire){
  RootDBInquire* rootInquire = new RootDBInquire(currInquire->getName(), this);
  rootInquire->m_decName = getAbsolutePath(gDirectory);
  // JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
  if(rootInquire->m_decName.length()==0) rootInquire->m_decName = "/";
  else if(rootInquire->m_decName[rootInquire->m_decName.length()-1] != '/') rootInquire->m_decName += "/";
  rootInquire->m_myDecName = rootInquire->m_decName + rootInquire->m_name;
  TKey* k = gDirectory->GetKey(rootInquire->getName().c_str());
  if(k){
    int version = k->GetCycle();
	rootInquire->getMyDecName() += ";"; 
    stringstream s;
    s << version+1;   
	rootInquire->getMyDecName() += s.str();
  }
  else{
   int version = 1;
   rootInquire->getMyDecName() += ";"; 
   stringstream s;
   s << version;   
   rootInquire->getMyDecName() += s.str();
  } 
  fieldIterator f = currInquire->fieldBegin();
  for(;f!=currInquire->fieldEnd();f++){
    RootDBField* dbf = new RootDBField((*f)->getName(), this);
    //	std::cout << **f << std::endl;
    dbf->copyField(f);
	dbf->getDecName() = rootInquire->m_decName;
	dbf->getDecName() += dbf->getName();
    TKey* k = gDirectory->GetKey(dbf->getName().c_str());
    if(k){
      int version = k->GetCycle();
      stringstream s;
      s << ";";
      s << version+1;    
      dbf->getDecName() += s.str();
      rootInquire->m_myFieldNameList.push_back(dbf->getName() + s.str());
    }
    else{
      int version = 1;
      stringstream s;
      s << ";";
      s << version;    
      dbf->getDecName() += s.str();
      rootInquire->m_myFieldNameList.push_back(dbf->getName() + s.str());
    }
    dbf->Write(dbf->getName().c_str());
    delete dbf;
  }  
  rootInquire->m_myRecordNameList = dynamic_cast<RootDBInquire*>(currInquire)->m_myRecordNameList;
  
  rootInquire->Write(rootInquire->getName().c_str());
  
}


void RootDB::processRootInquireTree(DBInquire* currInquire){
	
  RootDBInquire* rootInquire = new RootDBInquire(currInquire->getName(), this);
  rootInquire->m_decName = getAbsolutePath(gDirectory);
  // JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
  if(rootInquire->m_decName.length()==0) rootInquire->m_decName = "/";
  else if(rootInquire->m_decName[rootInquire->m_decName.length()-1] != '/') rootInquire->m_decName += "/";
  rootInquire->m_myDecName = rootInquire->m_decName + rootInquire->m_name;
  TKey* k = gDirectory->GetKey(rootInquire->getName().c_str());
  if(k){
    int version = k->GetCycle();
	rootInquire->getMyDecName() += ";"; 
    stringstream s;
    s << version+1;   
	rootInquire->getMyDecName() += s.str();
  }
  else{
   int version = 1;
   rootInquire->getMyDecName() += ";"; 
   stringstream s;
   s << version;   
   rootInquire->getMyDecName() += s.str();
  } 
  fieldIterator f = currInquire->fieldBegin();
  for(;f!=currInquire->fieldEnd();f++){
    RootDBField* dbf = new RootDBField((*f)->getName(), this);
    dbf->copyField(f);
	dbf->getDecName() = rootInquire->m_decName;
	dbf->getDecName() += dbf->getName();
    TKey* k = gDirectory->GetKey(dbf->getName().c_str());
    if(k){
      int version = k->GetCycle();
      stringstream s;
      s << ";";
      s << version+1;    
      dbf->getDecName() += s.str();
      rootInquire->m_myFieldNameList.push_back(dbf->getName() + s.str());
    }
    else{
      int version = 1;
      stringstream s;
      s << ";";
      s << version;    
      dbf->getDecName() += s.str();
      rootInquire->m_myFieldNameList.push_back(dbf->getName() + s.str());
    }
    dbf->Write(dbf->getName().c_str());
    delete dbf;
  }

  recordIterator r = currInquire->recordBegin();
  for(;r != currInquire->recordEnd();r++){
	  if(!gDirectory->GetKey(purgeSlash(getMySubDir(*r)).c_str())) gDirectory->mkdir(purgeSlash(getMySubDir(*r)).c_str());
	  gDirectory->cd(getMySubDir(*r).c_str());
    TKey* k = gDirectory->GetKey((*r)->getName().c_str());
    if(k){
      int version = k->GetCycle();
      stringstream s;
      s << getMySubDir(*r);
      s << (*r)->getName();
      s << ";";
      s << version+1;    
      rootInquire->m_myRecordNameList.push_back(s.str());
    }
    else{
      int version = 1;
      stringstream s;
      s << getMySubDir(*r);
      s << (*r)->getName();
      s << ";";
      s << version;    
      rootInquire->m_myRecordNameList.push_back(s.str());
    }
    processRootInquireTree(*r);
  }
  
  rootInquire->Write(rootInquire->getName().c_str());
  gDirectory->cd("../");
}

void RootDB::makeRootFileFromTurboDaqDB(string namefile, string mode){
	PixConfDBInterface* myDB;
	if(mode == "MODULE"){
		myDB = new TurboDaqDB(namefile.c_str(),1);	
		mode = "UPDATE";
	}
	else{
		// read the turbo daq db
		myDB = new TurboDaqDB(namefile.c_str());	
	}
	DBInquire* root = myDB->readRootRecord(1);
	// create a new root file name
	string rootFileName(namefile);
	rootFileName += ".root";
	this->m_theFileName = rootFileName;
	// create a Root db with the file and fill it;
	m_theFile = new TFile(rootFileName.c_str(), mode.c_str(), rootFileName.c_str());
	m_theFile->cd();
	processInquire(root);
	m_theFile->Close();
}

void insertModuleConfIntoRootDb(RootDB& db, string namefile);

RootDB::RootDB(string namefile, string mode){	
	// first, some root stuff for the TBtree
	m_theLoadedFields.SetOwner();
	m_theLoadedInquires.SetOwner();
  // check if mode is for turbodaq files
  if(mode == "CREATE" || mode == "RECREATE" || mode == "MODULE"){
    makeRootFileFromTurboDaqDB(namefile,mode);
  }
  else if (mode == "NEW"){
    // part 1: opening the root file
    m_theFile = new TFile(namefile.c_str(), mode.c_str());
	m_theFileName = namefile;
    if(m_theFile->IsZombie()) 
      throw PixDBException("RootDB constructor: Application description file not opened as good");
    m_theFile->cd();
    DBInquire* rootrec = 0;
    try{
      rootrec = this->makeInquire("rootRecord","");
    } catch(exception& s){
      throw PixDBException("Std-lib exception \""+string(s.what())+"\" while creating root record");
    }catch(...){
      throw PixDBException("Unknown exception while creating root record");
    }
    RootDBInquire* ri = dynamic_cast<RootDBInquire*>(rootrec);
    ri->getMyDecName() += ";1";
    try{
      ri->Write("rootRecord");
    } catch(exception& s){
      throw PixDBException("Std-lib exception \""+string(s.what())+"\" while writing to root record");
    }catch(...){
      throw PixDBException("Unknown exception while writing to root record");
    }
    m_theFile->Close();
    m_theFile = new TFile(namefile.c_str(),"UPDATE");
    if(m_theFile->IsZombie()) 
      throw PixDBException("RootDB constructor: Application description file not re-opened as good");
  }
  else{
    // part 1: opening the root file
    m_theFile = new TFile(namefile.c_str(), mode.c_str());
	m_theFileName = namefile;
    if(m_theFile->IsZombie()) 
      throw PixDBException("RootDB constructor: Application description file not open as good");
  }
  m_theFile->cd();
  gDirectory = m_theFile;
}

RootDBInquire* RootDB::getInquireByDecName(string name) const {
  //  bool rootInquires = false;
  RootDBInquire* objToFound = new RootDBInquire(name);
  RootDBInquire* retval = (RootDBInquire*)m_theLoadedInquires.FindObject(objToFound);
  std::string myname=name, file_path=m_theFile->GetName();
  int pos;
  pos = file_path.find_last_of("/");
  if(pos!=(int)std::string::npos)
    file_path.erase(0,pos+1);
  pos = myname.find(":");
#ifdef WIN32 // in windows, first ":" is most likely indicating the drive, not the begin of root's internal name...
  if(pos==1){ 
    myname.erase(0,pos+1);
    pos = myname.find(":");
  }
#endif
  if(pos!=(int)std::string::npos)
    myname.erase(0,pos+2);
  if (retval == 0) {
    //    if((int)myname.find_last_of("/") == (int)std::string::npos) {rootInquires = true;}// handle special case of root inquires
    retval = (RootDBInquire*)m_theFile->Get(myname.c_str());
    if(retval != 0){
      retval->m_myDB = const_cast<RootDB*>(this);
      const_cast<RootDB*>(this)->m_theLoadedInquires.Add(retval);
    }
  }
  delete objToFound;
  return retval;
}

bool RootDB::getInquireByDecNameFromDisk(string name, RootDBInquire* toRead){
	  RootDBInquire* fromDisk = (RootDBInquire*)m_theFile->Get(name.c_str());
	  if(fromDisk != 0){
		  (*toRead) = (*fromDisk);
		  toRead->m_myDB = this;
		  delete fromDisk;
		  return true;
	  }
	  else return false;
}


RootDBInquire* RootDB::memoryGetInquireByDecName(string name) {
  RootDBInquire* objToFound = new RootDBInquire(name);
  RootDBInquire* retval = (RootDBInquire*)m_theLoadedInquires.FindObject(objToFound);
  delete objToFound;
  return retval;
}

RootDBField* RootDB::getFieldByDecName(string name) const {
  RootDBField* objToFound = new RootDBField(name);
  RootDBField* retval = (RootDBField*)m_theLoadedFields.FindObject(objToFound);
  std::string myname=name, file_path=m_theFile->GetName();
  int pos;
  pos = file_path.find_last_of("/");
  if(pos!=(int)std::string::npos)
    file_path.erase(0,pos+1);
  pos = myname.find(file_path);
  if(pos!=(int)std::string::npos)
    myname.erase(0,pos+2+file_path.length());
  // debug
  //  printf("search string (3) - %d,%d: %s\n",pos,file_path.length(),myname.c_str());
  //end debug
  if(myname.find_last_of("/") == 0) myname.erase(0,1); // handle special case of field of root record
  if (retval == 0) {
	  retval = (RootDBField*)m_theFile->Get(myname.c_str());
	  if(retval != 0) {
		  retval->m_myDB = const_cast<RootDB*>(this);
		  const_cast<RootDB*>(this)->m_theLoadedFields.Add(retval);
	  }
  }
  delete objToFound;
  return retval;
}

RootDBField* RootDB::memoryGetFieldByDecName(string name){
  RootDBField* objToFound = new RootDBField(name);
  RootDBField* retval = (RootDBField*)m_theLoadedFields.FindObject(objToFound);
  delete objToFound;
  return retval;
}

DBInquire* RootDB::readRootRecord(int iteration) const{ // get the root record 
	TDirectory* k = gDirectory;
  // build the ROOT name of the required rootRecord
	m_theFile->cd();
	string name(m_theFile->GetPath());
	name += "rootRecord";
	stringstream i;
	// if the iteration is -1, this means the user wants to read the latest version of rootRecord
	if(iteration > 0){
		i << iteration;
	}
	else{
		// get the oldest version from file
		RootDBInquire* dummyInquire = new RootDBInquire("rootRecord","/","",const_cast<RootDB*>(this));
		int version = dummyInquire->getVersionFromFile(dummyInquire);
		// check if in memory there is another version with highest number
		stringstream v;
		string tmpName(name);
		tmpName += ";";
		v << version + 1;
		tmpName += v.str();
		dummyInquire = getInquireByDecName(tmpName);
		if (dummyInquire != 0) version++;
		i << version;
		delete dummyInquire;
	}
	name += ";";
	name += i.str();
  // get the rootRecord from file or from the TBtree of the inquires
	RootDBInquire* retval = getInquireByDecName(name);
  if(retval == 0){
    string error("RootDB::readRootRecord(int): not found root record of iteration ");
    error += i.str();
    throw PixDBException(error.c_str());    
  }
  gDirectory = k;
  return retval;
}

RootDB::~RootDB(){
  if(!(m_theFile->IsZombie()))
    m_theFile->Close();

  m_theLoadedFields.Delete();
  m_theLoadedInquires.Delete();
}

DBInquire* RootDB::readRootRecord(string /*tag*/) const{ // get the root record by tag
	return readRootRecord(0);
}

RootDBField::RootDBField(string name, PixConfDBInterface * db) : m_name(name), m_myDB(db), m_dataType(EMPTY), m_modified(true){
	this->m_myDoubleCont.Set(1);
	this->m_myFloatCont.Set(1);
	this->m_myIntCont.Set(1);
} 

void RootDBInquire::dump(std::ostream& os) const{
  os << "Dumping RootDBInquire" << std::endl;
  os << "inquire name: " << m_name << std::endl;
  os << "inquire decName: " << m_decName << std::endl;
  os << "inquire myDecName: " << m_myDecName << std::endl;

  os << "List of depending field: " << m_myFieldNameList.size() << " depending fields found" << std::endl;
  unsigned int i;
  for(i = 0; i < m_myFieldNameList.size(); i++){
    os << m_myFieldNameList[i] << std::endl;
    //   fieldIterator f = (const_cast<RootDBInquire*>(this))->getField(i);
    //(*f)->dump(os);
  }
  os << "List of depending inquire: " << m_myRecordNameList.size() << " depending inquires found" << std::endl;
  for(i = 0; i < m_myRecordNameList.size(); i++){
    os << m_myRecordNameList[i] << std::endl;
    //recordIterator f = (const_cast<RootDBInquire*>(this))->getInquire(i);
    //(*f)->dump(os);
  }
  
} // Dump - debugging purpose

void RootDBInquire::Dump(void){
	dump(std::cout);
}

void RootDBInquire::RemoveDependingInquire(int inquirePosition){
  std::vector<std::string>::iterator i;
  int ii = 0;
  for(i =  m_myRecordNameList.begin(); i !=  m_myRecordNameList.end(); i++,ii++){
    if(ii == inquirePosition){
      m_myRecordNameList.erase(i);
      break;
    }
  }
  this->Write(this->getName().c_str(),TObject::kOverwrite);
} // *MENU* remove the depending inquire at position inquirePosition

void RootDBInquire::AddDependingInquire(char* newDependingInquire){
      m_myRecordNameList.push_back(newDependingInquire);
	this->Write(this->getName().c_str(),TObject::kOverwrite);
} // *MENU* add the depending inquire

void RootDBInquire::ChangeDependingInquire(int inquirePosition, char* newDependingInquire){
	m_myRecordNameList[inquirePosition] = newDependingInquire;
	this->Write(this->getName().c_str(),TObject::kOverwrite);
}// *MENU* modyfy the depending inquire

void RootDBInquire::DumpFields(void){
  Dump();
  for(uint i = 0; i < this->m_myFieldNameList.size(); i ++){
    std::cout << i << m_myFieldNameList[i] << std::endl;
    dynamic_cast<RootDBField*>(*(this->getField(i)))->Dump();
  }
} // *MENU* dump on screen the inquire and its fields

void RootDBInquire::AppendThisToAnotherInquire(char * /*newParentInquire*/, char* /*newDecName*/){
} // *MENU* copy this inquire and the depending inquire to the new parent inquire (found by its decName)

TCanvas* RootDBField::dumpCanvas = 0;

void RootDBField::dump(std::ostream& os) const{

  os << "Dumping RootDBField named: " << m_name << std::endl;
  os << "RootDBField decName: " << m_decName << std::endl;
  os << "Data type: " <<  DBDataTypeNames[m_dataType] << std::endl;
  os << "Data content: ";
  switch (m_dataType){
  case BOOL:
    if(m_myIntCont.At(0) == 0) os << "false" << std::endl; else os << "true" << std::endl;
    break;
  case VECTORBOOL:
    int count;
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myIntCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      if(m_myIntCont.At(count) == 0) os << "false" << '\t'; else os << "true" << '\t';
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case INT:
    os << m_myIntCont.At(0) << std::endl;
    break;
  case ULINT:
    os << m_myStringCont.Data() << std::endl;
    break;
  case FLOAT:
    os << m_myFloatCont.At(0) << std::endl;
    break;
  case DOUBLE:
    os << m_myDoubleCont.At(0) << std::endl;
    break;
  case VECTORINT:
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myIntCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      os << m_myIntCont.At(count) << '\t'; 
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case VECTORFLOAT:
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myFloatCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      os << m_myFloatCont.At(count) << '\t'; 
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case VECTORDOUBLE:
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myDoubleCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      os << m_myDoubleCont.At(count) << '\t'; 
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case HISTO:{
	  os << "dumping histogram to Dump Window" << std::endl;

	  if(!gROOT->GetListOfCanvases()->FindObject("Dump Window")){
		  dumpCanvas = new TCanvas("Dump Window","Dump Window",2);
	  }
	  dumpCanvas->cd();

	 
	  TH1D* histo1;
	  TH2D* histo2;
//	  const Histo& his;
	  Histo his;

	  if(m_myHistoCont.nDim() == 0){
	    std::vector<double> v;
	    for(count = 0; count < m_myDoubleCont.GetSize(); count++){
	      v.push_back(m_myDoubleCont[count]);
	    }
	   his = vectorToHisto(v);
	  }
	  else {
	    his = m_myHistoCont;
	  }
//	  if(vectorHistoDim(v) == 1){	  
	  if(his.nDim()  == 1){
	    histo1 = new TH1D(his.name().c_str(),his.title().c_str(), his.nBin(0), his.min(0),his.max(0));
	    for(count = 0; count < his.nBin(0); count++){
	      histo1->SetBinContent(count+1,his(count));
	    }
	    histo1->Draw();
	  }
	  else if(his.nDim() == 2){
	    histo2 = new TH2D(his.name().c_str(),his.title().c_str(), his.nBin(0), his.min(0),his.max(0),his.nBin(1), his.min(1),his.max(1));
	    int count2;
	    for(count = 0; count < his.nBin(0); count++){
	      for(count2 = 0; count2 < his.nBin(1); count2++){
		histo2->SetBinContent(count+1, count2+1, his(count,count2));
	      }
	    }
	    histo2->Draw();
	  }
	  else{
	    stringstream a;
	    a << "RootDBField::Dump : Histogram is not 1 or 2 dimensional ";
	    throw PixDBException(a.str());
	  }
	  dumpCanvas->Modified();
	  dumpCanvas->Update();}
    break;
    


  case STRING:
    os << m_myStringCont.Data() << std::endl;
    break;
  case EMPTY:
    os << "field does not have data content" << std::endl;
  default:
    break;
  }
}

void RootDBField::Dump(void){
	dump(std::cout);
}

void RootDBField::ModifyFieldValue(char* newValue){ // *MENU* modify the content of the Field
	int i = 0; 
	int hi = 0; 
	float f = 0;
	double d = 0;
	bool b = false;

	stringstream ss;
	ss.str(newValue);
	ss >> i;
       	ss.clear();
	ss.str(newValue);
	ss >> std::hex >> hi >> std::dec;
       	ss.clear();
       	ss.str(newValue);
	ss >> f;
       	ss.clear();
       	ss.str(newValue);
	ss >> d;
       	ss.clear();
       	ss.str(newValue);
	ss >> std::boolalpha >> b;

	switch(this->getDataType()){
	case INT:
	        if(hi!=0 && i==0)
		  this->m_myIntCont[0] = hi;
		else 
		  this->m_myIntCont[0] = i;
		break;
	case ULINT:
		this->m_myStringCont = ss.str().c_str();
		break;
	case FLOAT:
		this->m_myFloatCont[0] = f;
		break;
	case BOOL:
		this->m_myIntCont[0] = b;
		break;
	case STRING:
		this->m_myStringCont = ss.str().c_str();
		break;
	case DOUBLE:
		this->m_myDoubleCont[0] = d;
		break;
	default:
		std::cout << "RootDBField::ModifyFieldValue : not supported type" << std::endl;
	}

	this->Write(this->getName().c_str(),TObject::kOverwrite);
	m_modified = false;
}

/*void RootDB::updateFieldMemoryContent(RootDBField* theField){
	RootDBField* objToFound = new RootDBField(theField->getDecName());
	RootDBField* retval = (RootDBField*)m_theLoadedFields.FindObject(objToFound);
	if(retval != 0){
		fieldIterator to(retval);
		fieldIterator from(theField);
		to.copyData(from);
	}
}*/

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, Histo& retval){
	RootDBField* field;
	std::vector<double> tmpvec;
	int i;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == HISTO || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a Histo process";
      throw PixDBException(a.str());
	}

	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		if(field->m_myDoubleCont.GetSize() != 0){
		  for(i = 0; i < field->m_myDoubleCont.GetSize(); i++){
		    tmpvec.push_back(field->m_myDoubleCont.At(i));
		  }
		  field->m_myHistoCont = vectorToHisto(tmpvec);
		  field->m_myDoubleCont.Reset();
		}
//		retval = vectorToHisto(tmpvec);		
		retval = field->m_myHistoCont;
		return true; 
		break;
	case COMMIT:
	  //	  uint ii;
			field->m_dataType = HISTO;
			field->m_myHistoCont = retval;
//			tmpvec = histoToVector(retval);
//			field->m_myDoubleCont.Set(tmpvec.size());
//			for(ii = 0; ii < tmpvec.size(); ii++){
//				field->m_myDoubleCont[ii] = tmpvec[ii];
//			}
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
	return false;
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, string& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == STRING || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a string process";
      throw PixDBException(a.str());
	}

	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		retval = field->m_myStringCont;
		return true; 
		break;
	case COMMIT:
			field->m_dataType = STRING;
			field->m_myStringCont = retval.c_str();
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, int& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == INT || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a int process";
      throw PixDBException(a.str());
	}

	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		retval = field->m_myIntCont.At(0);
		return true; 
		break;
	case COMMIT:
			field->m_dataType = INT;
			field->m_myIntCont[0] = retval;
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int & retval){
  RootDBField* field;
  stringstream s;
  field = dynamic_cast<RootDBField*>(*theField);
  if(field == 0) {
    stringstream a;
    a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
    throw PixDBException(a.str());
  }
  if(!(field->getDataType() == ULINT || field->getDataType() == EMPTY)){
    stringstream a;
    a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a uint process";
    throw PixDBException(a.str());
  }
  
  unsigned int i, hi;
  switch (mode){
  case READ:
    if(field->getDataType() == EMPTY){
      stringstream a;
      a << "RootDB::DBProcess: try to read an empty fieldIterator";
      throw PixDBException(a.str());
    }
    s.str(field->m_myStringCont.Data());
    s >> i;
    s.clear();
    s.str(field->m_myStringCont.Data());
    s >> std::hex >> hi >> std::dec;
    if(hi!=0 && i==0) 
      retval = hi;
    else
      retval = i;
    return true; 
    break;
  case COMMIT:
    field->m_dataType = ULINT;
    s << retval;
    s >> field->m_myStringCont;
    field->m_modified = true;
    return true;
    break;
  default:
    stringstream a;
    a << "RootDB::DBProcess: mode not implemented " << mode;
    throw PixDBException(a.str());
    break;
  };
  return false;
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, float& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == FLOAT || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a float process";
      throw PixDBException(a.str());
	}

	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		retval = field->m_myFloatCont.At(0);
		return true; 
		break;
	case COMMIT:
			field->m_dataType = FLOAT;
			field->m_myFloatCont[0] = retval;
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, double& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == DOUBLE || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a double process";
      throw PixDBException(a.str());
	}

	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		retval = field->m_myDoubleCont.At(0);
		return true; 
		break;
	case COMMIT:
			field->m_dataType = DOUBLE;
			field->m_myDoubleCont[0] = retval;
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, bool& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == BOOL || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a bool process";
      throw PixDBException(a.str());
	}

	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		retval = field->m_myIntCont.At(0);
		return true; 
		break;
	case COMMIT:
			field->m_dataType = BOOL;
			field->m_myIntCont[0] = retval;
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator


bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == VECTORINT || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a vector<int> process";
      throw PixDBException(a.str());
	}

	int i;
	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		for(i = 0; i < field->m_myIntCont.GetSize(); i++){
			retval.push_back(field->m_myIntCont.At(i));
		}
		return true; 
		break;
	case COMMIT:
	  uint ii;
			field->m_dataType = VECTORINT;
			field->m_myIntCont.Set(retval.size());
		for(ii = 0; ii < retval.size(); ii++){
			field->m_myIntCont[ii] = retval[ii];
		}
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator


bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == VECTORBOOL || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a vector<bool> process";
      throw PixDBException(a.str());
	}

	int i;
	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		for(i = 0; i < field->m_myIntCont.GetSize(); i++){
			retval.push_back(field->m_myIntCont.At(i));
		}
		return true; 
		break;
	case COMMIT:
	  uint ii;
			field->m_dataType = VECTORBOOL;
			field->m_myIntCont.Set(retval.size());
		for(ii = 0; ii < retval.size(); ii++){
			field->m_myIntCont[ii] = retval[ii];
		}
			field->m_modified = true;
		return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator


bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == VECTORFLOAT || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a vector<float> process";
      throw PixDBException(a.str());
	}

	int i;
	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		for(i = 0; i < field->m_myFloatCont.GetSize(); i++){
			retval.push_back(field->m_myFloatCont.At(i));
		}
		return true; 
		break;
	case COMMIT:
	  uint ii;
			field->m_dataType = VECTORFLOAT;
			field->m_myFloatCont.Set(retval.size());
		for(ii = 0; ii < retval.size(); ii++){
			field->m_myFloatCont[ii] = retval[ii];
		}
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator

bool RootDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& retval){
	RootDBField* field;
	field = dynamic_cast<RootDBField*>(*theField);
	if(field == 0) {
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator do not refer to RootDBField";
      throw PixDBException(a.str());
	}
	if(!(field->getDataType() == VECTORDOUBLE || field->getDataType() == EMPTY)){
		stringstream a;
		a << "RootDB::DBProcess: fieldIterator is a " << DBDataTypeNames[field->getDataType()] << " on a vector<double> process";
      throw PixDBException(a.str());
	}

	int i;
	switch (mode){
	case READ:
		if(field->getDataType() == EMPTY){
			stringstream a;
			a << "RootDB::DBProcess: try to read an empty fieldIterator";
			throw PixDBException(a.str());
		}
		for(i = 0; i < field->m_myDoubleCont.GetSize(); i++){
			retval.push_back(field->m_myDoubleCont.At(i));
		}
		return true; 
		break;
	case COMMIT:
	  uint ii;
			field->m_dataType = VECTORDOUBLE;
			field->m_myDoubleCont.Set(retval.size());
		for(ii = 0; ii < retval.size(); ii++){
			field->m_myDoubleCont[ii] = retval[ii];
		}
			field->m_modified = true;
			return true;
		break;
	default:
		stringstream a;
		a << "RootDB::DBProcess: mode not implemented " << mode;
	    throw PixDBException(a.str());
		break;
	};
} // read or commit the field pointed by the iterator

fieldIterator RootDBInquire::pushField(DBField* in){
	RootDBField* dbf;
	RootDBField* retvalFromFind = 0;
	dbf = dynamic_cast<RootDBField*>(in);
	if(dbf == 0) {
		stringstream a;
		a << "RootDBInquire::pushField: field is not a RootDBField";
      throw PixDBException(a.str());
	}
	dbf = new RootDBField(*(dynamic_cast<RootDBField*>(in))); // make the local copy (evenctually deleting it if necessary after
	// check if the pushed field is already loaded in memory - that means that we have to do a copy of this field to 
	// act on, in order not to modify also other inquires that could refer to this field
	dbf->getDecName() = this->m_decName;
	// JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
	if(dbf->getDecName().length()==0) dbf->getDecName() = "/";
	else if(dbf->getDecName()[dbf->getDecName().length()-1] != '/') dbf->getDecName() += "/";
	dbf->getDecName() += dbf->getName();
//	dbf->m_myParentInquire = this->getMyDecName();
	if(findField(dbf->getName()) != fieldEnd()){
		TDirectory* dir = gDirectory;
		m_myDB->m_theFile->cd();
		m_myDB->m_theFile->cd(this->getDecName().c_str());
        TKey* k = gDirectory->GetKey(dbf->getName().c_str());
		int version;
		if(k){
			version = k->GetCycle();
		}
		else{
			version = 0;
		}
		stringstream s;
		s << ";";
		s << version+1;    
		dbf->getDecName() += s.str();
		retvalFromFind = this->m_myDB->getFieldByDecName(dbf->getDecName());
		if(retvalFromFind != 0) { // found already in memory. This means that the action of pushing this field do not have any effect;
			uint i;
			for(i = 0; i < this->m_myFieldNameList.size(); i++){
				if(stripIteration(this->m_myFieldNameList[i]) == dbf->getName()) break;
			}
			fieldIterator f(i,this->m_myDB->DBFindFieldByName(BYDECNAME, dbf->getDecName())[0],this);
			if(dbf != *f){
				*dynamic_cast<RootDBField*>(*f) = *dbf; 
				delete dbf; // avoid memory leakage: the check is required in the case that the same field (not only having the same name and data) is required to add twice
			}
			dir->cd();
			return f;
		}
		else{ // this means that the field has to be added to the memory and the field list in this inquire has to be updated
			uint i;
			for(i = 0; i < this->m_myFieldNameList.size(); i++){
				if(stripIteration(this->m_myFieldNameList[i]) == dbf->getName()) break;
			}
			this->m_myFieldNameList[i] = stripIteration(this->m_myFieldNameList[i]);
			this->m_myFieldNameList[i] += s.str();
			this->m_myDB->m_theLoadedFields.Add(dbf); // this does not make a copy... the user must not delete the DBField* in he supplied
			dir->cd();
			fieldIterator f(i,this->m_myDB->DBFindFieldByName(BYDECNAME, dbf->getDecName())[0],this);
			return f;
		}
	} // field already exists: get the higher iteration number from file; check if next iteration is in memory; add to memory or return the one in memory (this means that only one iteration can be added to the inquire without committing it)
	else {
		int version;
		TDirectory* dir = gDirectory;
		m_myDB->m_theFile->cd();
		// check if the field exists already in the permanent storage. If that, give it the uppper version number; otherwise, give it 0
		if(m_myDB->m_theFile->GetKey(this->getDecName().c_str()) != 0){
			m_myDB->m_theFile->cd(this->getDecName().c_str());
			TKey* k = gDirectory->GetKey(dbf->getName().c_str());
			if(k){
				version = k->GetCycle();
			}
			else{
				version = 0;
			}
		}
		else{
			version = 0;
		}

		stringstream s;
		s << ";";
		s << version+1;    
		dbf->getDecName() += s.str();
		this->m_myFieldNameList.push_back(dbf->getName()+s.str()); 
		retvalFromFind = this->m_myDB->getFieldByDecName(dbf->getDecName());
		if(retvalFromFind != 0) { // found already in memory. This means that the action of pushing this field do not have any effect;
			fieldIterator f(this->m_myFieldNameList.size()-1,retvalFromFind,this);
			if(dbf != *f){
				*dynamic_cast<RootDBField*>(*f) = *dbf; 
				delete dbf;
			}
			dir->cd();
			return f;
		}
		else{ // this means that the field has to be added to the memory and the field list in this inquire has to be updated
			this->m_myDB->m_theLoadedFields.Add(dbf); // this does not make a copy... the user must not delete the DBField* in he supplied
			dir->cd();
			fieldIterator f(this->m_myFieldNameList.size()-1,dbf,this);
			return f;
		}		
	} // field do not exists: add it to memory and to this inquire
} // add a field to the DBInquire fields and return the corresponding iterator

void RootDBInquire::eraseField(fieldIterator it){
	std::vector<string>::iterator i = this->m_myFieldNameList.begin();
	for(; i != this->m_myFieldNameList.end(); i++){
		if(stripIteration(*i) == (*it)->getName()) break;
	}
	if(i != this->m_myFieldNameList.end()){
		this->m_myFieldNameList.erase(i);
	}
	else{
		stringstream a;
		a << "RootDBInquire::eraseField: field " << (*it)->getName() << " not found in inquire " << *this;
      throw PixDBException(a.str());
	}
	
}; // erase a field. This means taking it out of the list of depending fields of this inquire. The disk content and memory content is not changed, since other inquires could refer to the same field


void RootDBInquire::makeDecName(RootDBInquire* child, RootDBInquire* parent){
	string tmpDecName = getMySubDir(child);
	child->getDecName() = parent->m_decName;
	// JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
	if(child->getDecName().length()==0) child->getDecName() = "/";
	else if(child->getDecName()[child->getDecName().length()-1] != '/') child->getDecName() += "/";
	child->getDecName() += tmpDecName;
	string recordName = getMyVersionName(child);
	child->getMyDecName() = child->getDecName();
	// JGK: see above
	if(child->getMyDecName().length()==0) child->getMyDecName() = "/";
	else if(child->getMyDecName()[child->getMyDecName().length()-1] != '/') child->getMyDecName() += "/";
	child->getMyDecName() += recordName;
}

int RootDBInquire::getVersionFromFile(RootDBInquire* dbi){
		TDirectory* dir = gDirectory;
		gDirectory = this->m_myDB->m_theFile;
		gDirectory->cd();
		gDirectory->cd(getDecName().c_str());
		if(gDirectory->GetKey(purgeSlash(getMySubDir(dbi)).c_str()) == 0) return 0; else gDirectory->cd(getMySubDir(dbi).c_str());
        TKey* k = gDirectory->GetKey(dbi->getName().c_str());
		int version;
		if(k){
			version = k->GetCycle();
		}
		else{
			version = 0;
		}
//		dir->cd();
		gDirectory = dir;
		return version;
}

recordIterator RootDBInquire::pushRecord(DBInquire* in){
	RootDBInquire* dbi;
	RootDBInquire* retvalFromFind = 0;
	bool dbiToDelete = false;
	// check if the record to be added is a RootDBRecord
	dbi = dynamic_cast<RootDBInquire*>(in);
	if(dbi == 0) {
	  stringstream a;
	  a << "RootDBInquire::pushRecord: record is not a RootDBInquire";
	  throw PixDBException(a.str());
	}
	dbi = new RootDBInquire(*(dynamic_cast<RootDBInquire*>(in))); // make the local copy (evenctually deleting it if necessary after
	// check if the pushed record is already loaded in memory 
	
	// change the decorated name of the record to be pushed according to the one of the parent record 
	makeDecName(dbi,this);
	if(findRecord(stripIteration(getMySubDir(dbi) + getMyVersionName(dbi))) != recordEnd()){ // found in the subrecords of this record.
		stringstream s;
		s << ";";
		//		s << getVersionFromFile(dbi)+1;    
		s << getVersionFromFile(dbi);  // patch - no versioning   
		dbi->getMyDecName() = stripIteration(dbi->getMyDecName()) + s.str();
		retvalFromFind = this->m_myDB->getInquireByDecName(dbi->getMyDecName());
		if(retvalFromFind != 0) { // found already in memory. This means that the action of pushing this inquire copy the data of new inquire into the old inquire;
			uint i;
			for(i = 0; i < this->m_myRecordNameList.size(); i++){
				if(stripIteration(this->m_myRecordNameList[i]) == stripIteration(getMySubDir(dbi) + getMyVersionName(dbi))) break;
			}
			*retvalFromFind = *dbi; 
			// patch - no versioning
			// since the version number is not increased, the incoming inquire may not have any number, and dbi have ;1
			// but if the incoming inquire has been already loaded, it is found in the memory
			// so it is necessary to copy also the dbi into in to have correct decorated name of in
			*(dynamic_cast<RootDBInquire*>(in)) = *dbi;
			// end patch - no versioning
			dbiToDelete = true; 
		}
		else{ // this means that the inquire has to be added to the memory and the inquire list in this inquire has to be updated
			uint i;
			for(i = 0; i < this->m_myRecordNameList.size(); i++){
				if(stripIteration(this->m_myRecordNameList[i]) == stripIteration(getMySubDir(dbi) + getMyVersionName(dbi))) break;
			}
			this->m_myRecordNameList[i] = stripIteration(this->m_myRecordNameList[i]);
			this->m_myRecordNameList[i] += s.str();
			this->m_myDB->m_theLoadedInquires.Add(dbi); 
		}
	} // inquire already exists: get the higher iteration number from file; check if next iteration is in memory; add to memory or return the one in memory (this means that only one iteration can be added to the inquire without committing it)
	else {
		stringstream s;
		string corrMySubDir = getMySubDir(dbi);
		// JGK: this didn't seem to cause trouble under SLC, but does under windows - attempt to fix
		if(corrMySubDir.length()==0) corrMySubDir = "/";
	        else if(corrMySubDir[corrMySubDir.length()-1] != '/') corrMySubDir += "/";
		s << stripIteration(corrMySubDir + getMyVersionName(dbi));
		s << ";";
		if(getVersionFromFile(dbi) == 0){
		  s << 1;
		}
		else{
		  s << getVersionFromFile(dbi);
		}
		
		//		s << getVersionFromFile(dbi)+1;    
		this->m_myRecordNameList.push_back(s.str()); 
		stringstream ss;
		ss << ";";
		if(getVersionFromFile(dbi) == 0){
		  ss << 1;
		}
		else{
		  ss << getVersionFromFile(dbi);
		}
		//		ss << getVersionFromFile(dbi)+1;
		dbi->getMyDecName() = stripIteration(dbi->getMyDecName()) + ss.str();
		this->m_myDB->m_theLoadedInquires.Add(dbi); 
		// patch - no versioning
		// since the version number is not increased, the incoming inquire may not have any number, and dbi have ;1
		// but if the incoming inquire has been already loaded, it is found in the memory
		// so it is necessary to copy also the dbi into in to have correct decorated name of in
		*(dynamic_cast<RootDBInquire*>(in)) = *dbi;
		// end patch - no versioning
	} // inquire do not exists: add it to memory and to this inquire

	uint i;
	dbi->m_myFieldNameList.clear();
	for(i = 0; i < dynamic_cast<RootDBInquire*>(in)->m_myFieldNameList.size(); i++){
		dbi->pushField(*(in->getField(i)));
	}
	dbi->m_myRecordNameList.clear();
	for(i= 0; i < dynamic_cast<RootDBInquire*>(in)->m_myRecordNameList.size(); i++){
		dbi->pushRecord(*(in->getInquire(i)));
	}
	
	for(i = 0; i < this->m_myRecordNameList.size(); i++){
		if(stripIteration(this->m_myRecordNameList[i]) == stripIteration(getMySubDir(dbi) + getMyVersionName(dbi))) break;
	}
	recordIterator f(i,this->m_myDB->DBFindRecordByName(BYDECNAME, dbi->getMyDecName())[0],this);
	if(dbiToDelete) delete dbi;
	return f;

} // add a record to the record list and return the corresponding iterator

void RootDBInquire::eraseRecord(recordIterator it) {
  TDirectory *orgDir = gDirectory;
  std::string rname = (*it)->getDecName();
  gDirectory->cd(rname.c_str());
  int k=0;
  for(recordIterator i = recordBegin(); i!=recordEnd();i++){
    if((*i)->getDecName()==(*it)->getDecName()) break;
    k++;
  }
  this->RemoveDependingInquire(k);
  gDirectory->cd("..");
  rname = purgeSlash(getMySubDir(*it));
  gDirectory->rmdir(rname.c_str());
  orgDir->cd();
}; // erase a record


std::vector<double> PixLib::histoToVector(Histo& his){
	std::vector<double> retval;
	uint i = 0;
	int ii = 0;
	retval.push_back(0.987654321); // code for identify an histogram

	retval.push_back(his.name().size());
	for(i = 0; i < his.name().size(); i++){
		retval.push_back(his.name().at(i));
	}
	retval.push_back(his.title().size());
	for(i = 0; i < his.title().size(); i++){
		retval.push_back(his.title().at(i));
	}
	retval.push_back(his.nDim());
	if(his.nDim() == 1){
		retval.push_back(his.nBin(0));
		retval.push_back(his.max(0));
		retval.push_back(his.min(0));
		for(ii = 0; ii < his.nBin(0); ii++){
			retval.push_back(his(ii));
		}
	}
	else if(his.nDim() == 2){
		retval.push_back(his.nBin(0));
		retval.push_back(his.max(0));
		retval.push_back(his.min(0));
		retval.push_back(his.nBin(1));
		retval.push_back(his.max(1));
		retval.push_back(his.min(1));
		int j = 0;
		for(ii = 0; ii < his.nBin(0); ii++){
			for(j = 0; j < his.nBin(1); j++){
				retval.push_back(his(ii,j));
			}
		}
	}
	else{
		stringstream a;
		a << "histoToVector error: nDim " << his.nDim() << "not allowed";
      throw PixDBException(a.str());
	}

	return retval;
}

Histo PixLib::vectorToHisto(std::vector<double>& vec){
  if(vec.size() == 0){
    stringstream a;
    a << "histoToVector error: vector size is zero" ; 
    throw PixDBException(a.str());
  }
  if(vec[0] != 0.987654321){
    stringstream a;
    a << "vectorToHisto error: vec[0] is " << vec[0] << "not 0.987654321, vector is not an histo ";
    throw PixDBException(a.str());
  }
  
  std::string hisName;
  hisName.resize(static_cast<int>(vec[1]));
  uint i;
  for(i = 0; i < hisName.size(); i++){
    hisName.at(i) = static_cast<int>(vec[2+i]);
  }
  
  std::string hisTitle;
  hisTitle.resize(static_cast<int>(vec[2+hisName.size()]));

  for(i = 0; i < hisTitle.size(); i++){
    hisTitle.at(i) = static_cast<int>(vec[2+hisName.size()+1+i]);
  }
  int hisnDim = static_cast<int>(vec[2+hisName.size()+hisTitle.size()+1]);
  double hisnBin[2];
  double hisMax[2];
  double hisMin[2];
  
  std::vector<std::vector<double> > his;
  
	if(hisnDim == 1){
	  hisnBin[0] = vec[2+hisName.size()+hisTitle.size()+1+1];
	  hisMax[0] = vec[2+hisName.size()+hisTitle.size()+1+2];
	  hisMin[0] = vec[2+hisName.size()+hisTitle.size()+1+3];
	  std::vector<double> hisRow;
	  for(i = 0; i < hisnBin[0]; i++){
	    hisRow.push_back(vec[2+hisName.size()+hisTitle.size()+5+i]);
	  }
	  his.push_back(hisRow);
	}
	else if(hisnDim == 2){
	  hisnBin[0] = vec[2+hisName.size()+hisTitle.size()+1+1];
	  hisMax[0] = vec[2+hisName.size()+hisTitle.size()+1+2];
	  hisMin[0] = vec[2+hisName.size()+hisTitle.size()+1+3];
	  hisnBin[1] = vec[2+hisName.size()+hisTitle.size()+1+4];
	  hisMax[1] = vec[2+hisName.size()+hisTitle.size()+1+5];
	  hisMin[1] = vec[2+hisName.size()+hisTitle.size()+1+6];
	  uint j = 0;
	  for(i = 0; i < hisnBin[0]; i++){
	    std::vector<double> hisRow;
	    for(j = 0; j < hisnBin[1]; j++){
	      hisRow.push_back(vec[2+hisName.size()+hisTitle.size()+1+6+1+j+i*(int)hisnBin[1]]);
	    }
	    his.push_back(hisRow);
	  }
	}
	else{
	  stringstream a;
	  a << "vectorToHisto error: nDim is " << hisnDim << " value not allowed";
	  throw PixDBException(a.str());
	}
	
	if(hisnDim == 1){
	  Histo retval(hisName,hisTitle,(int)hisnBin[0],hisMin[0],hisMax[0]);
	  for(i = 0; i < hisnBin[0]; i++){
	    retval.set(i,his[0][i]);
	  }
	  return retval;
	}
	else{
	  Histo retval(hisName,hisTitle,(int)hisnBin[0],hisMin[0],hisMax[0],(int)hisnBin[1],hisMin[1],hisMax[1]);
	  for(i = 0; i < hisnBin[0]; i++){
	    for(uint j = 0; j < hisnBin[1]; j++){
	      retval.set(i,j,his[i][j]);
	    }
	  }
	  return retval;
	}
}

int PixLib::vectorHistoDim(vector<double>& vec){ // returns 0 if vec is not an Histo, 1 or 2 as Histo dimension
	if(vec.size() == 0 || vec[0] != 0.987654321){
		return 0;
	}	
	return static_cast<int>(vec[(int)(2+vec[1]+ vec[2+(int)vec[1]]+1)]);
}

std::vector<std::string> pathItemize(std::string& inputString){
	std::vector<std::string> retval;
	uint runnpos = 0;
	if(inputString.find_first_of('/') == 0) { // handle the beginning / in the absolut path
		std::string item("/");
		retval.push_back(item);
		runnpos = 1;
	}
	while((int)inputString.find_first_of('/',runnpos) != (int)std::string::npos){
		std::string item = inputString.substr(runnpos,inputString.find_first_of('/',runnpos+1)-runnpos);
		retval.push_back(item);
		runnpos = inputString.find_first_of('/',runnpos) + 1;
	}
	if(runnpos != inputString.size()){
		retval.push_back(inputString.substr(runnpos, inputString.size()));
	}
	return retval;
}

void RootDB::RootCDCreate(TDirectory* /*startingDir*/, RootDBInquire* inquire){
	std::vector<string> items;
	items = pathItemize(inquire->getDecName());
	if(items.size() == 0) return;
	uint count = 0;
	// handle the file name
	if(items[0] == "/"){
		if(items.size() == 1) return;
		count++;
	}
	// create the directory if needed
	for(;count < items.size(); count++){
		if(gDirectory->GetKey(purgeSlash(items[count]).c_str()) == 0){
			gDirectory->mkdir(purgeSlash(items[count]).c_str());
		}
		gDirectory->cd(items[count].c_str());
	}
}

bool RootDB::DBProcess(recordIterator therecord, enum DBInquireType mode){ // read or commit (possibly recursively) the inquire pointed by iterator
	bool retval;
	recordIterator f;
	RootDBInquire* dbi = dynamic_cast<RootDBInquire*>(*therecord);
	TDirectory* dir = gDirectory;
	if(dbi == 0) {
	  stringstream a;
	  a << "RootDB::DBProcess: trying to process an non RootDBInquire";
	  throw PixDBException(a.str());
	}
	switch (mode){
	case READ:
		// find if the record is already loaded in memory
		if(memoryGetInquireByDecName(dbi->getMyDecName()) == 0) {
			(*dbi) = *getInquireByDecName(dbi->getMyDecName());
			//std::cout << "in READ: find the record in memory. Dumping it: " << *dbi;
		}
		else{
		  // std::cout << "in READ: not found the record in memory. getting from disk" << std::endl;
		  return getInquireByDecNameFromDisk(dbi->getMyDecName(),dbi) ;
		}
		return true;
		break;
	case COMMITREPLACETREE:
		m_theFile->cd();
		gDirectory = m_theFile;
		RootCDCreate(gDirectory,dbi);
		processRootInquireReplaceTree(dbi);
		retval = DBProcess(therecord,READ);
		gDirectory = dir;
		return retval;
		break;		
	case COMMITREPLACE:
		m_theFile->cd();
		gDirectory = m_theFile;
		RootCDCreate(gDirectory,dbi);
		makeInquire(dbi->getName(), dbi->getDecName());
		processRootInquireReplace(dbi);
		retval = DBProcess(therecord,READ);
		//		std::cout << "in COMMITREPLACE: inquire committed and reread" << std::endl << "now try to find it in the database";
		f = DBFindRecordByName(BYDECNAME, dbi->getMyDecName())[0];
		//std::cout << "vector size of found records is: " << DBFindRecordByName(BYDECNAME, dbi->getMyDecName()).size() << std::endl;
		//std::cout << "found in memory is " << **f;
		gDirectory = dir;
		return retval;
		break;
	case COMMIT:
		m_theFile->cd();
		gDirectory = m_theFile;
		RootCDCreate(gDirectory,dbi);
		processRootInquire(dbi);
		retval = DBProcess(therecord,READ);
		gDirectory = dir;
		return retval;
		break;
	case COMMITTREE:
		m_theFile->cd();
		gDirectory = m_theFile;
		RootCDCreate(gDirectory,dbi);
		processRootInquireTree(dbi);
		// then modify also upper record for new version of depending inquire
		gDirectory = dir;
		return true;
		break;
	default:
	  return false;		
	};
}

		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, bool& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);} // read or commit the field pointed by the iterator
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<bool>& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, int& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<int>& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, unsigned int & v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, float& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<float>& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, double& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<double>& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, Histo& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBField* theField, enum DBInquireType mode, string& v){fieldIterator f(theField); return RootDB::DBProcess(f,mode, v);}
		 bool RootDB::DBProcess(DBInquire* therecord, enum DBInquireType mode){recordIterator r(therecord); return RootDB::DBProcess(r,mode);} // read or commit (possibly recursively) the inquire pointed by iterator




std::vector<DBInquire*> PixLib::rootGroupModuleWizard(RootDBInquire* groupModuleInquire, float rootDBVersion, 
						      float /*turboDaqVersion*/, std::vector<std::string> turboDaqConfigFileNames, 
				  std::string plPath, bool tdaqName){
  std::vector<DBInquire*> retInq;
  TDirectory* dir = gDirectory;	
  RootDB* DB = dynamic_cast<RootDB*>(groupModuleInquire->getDB());
  
  if(rootDBVersion == 1.0){
    // creating the modules inquires from names of .cfg files and default value
    {
      for(uint i = 0; i < turboDaqConfigFileNames.size(); i++){
	std::string name("PixModule");
	std::string decName; decName += groupModuleInquire->getDecName();
	std::string properTdaqName(turboDaqConfigFileNames[i]);
	if(tdaqName){
	  properTdaqName = getTdaqModuleName(turboDaqConfigFileNames[i]);
	} else{
	  if(properTdaqName.substr(properTdaqName.length()-4,4) == ".cfg") properTdaqName = properTdaqName.substr(0,properTdaqName.length()-4);
	  if((int)properTdaqName.find_last_of('/') != (int)std::string::npos) properTdaqName = properTdaqName.substr(properTdaqName.find_last_of('/')+1, properTdaqName.length() - (properTdaqName.find_last_of('/')+1));
	}
	decName += properTdaqName;; 
	stringstream a;
	a << decName; a << "/"; a << name; a << ";1";
	std::string myDecName(a.str());
	RootDBInquire* tmpInquire = new RootDBInquire(name, decName, myDecName, groupModuleInquire->getDB());   
	groupModuleInquire->pushRecord(tmpInquire);
	recordIterator r(groupModuleInquire);
	DB->DBProcess(r,COMMITREPLACE);
	recordIterator rr(tmpInquire);
	DB->DBProcess(rr,COMMITREPLACE);

	// now read the turbodaq files, and copy the content of files into the rootDB module structure
	PixConfDBInterface *myDB = new TurboDaqDB(turboDaqConfigFileNames[i],1);				
	DBInquire* root = 0;
	root = myDB->readRootRecord(1);
	DBInquire* pixModule = *(root->getInquire(0));
	// find out which structure to create 
	fieldIterator thisModuleMcc = root->findField("MCC flavour");	
	fieldIterator thisModuleFe = root->findField("FE flavour");
	int modMCC = -1;
	int modFE = -1;
	myDB->DBProcess(thisModuleMcc,READ,modMCC);
	myDB->DBProcess(thisModuleFe,READ,modFE);
	
	// going to the root directory of the inquire
	gDirectory = DB->m_theFile;
	gDirectory->cd();
	//				gDirectory->cd(groupModuleInquire->getDecName().c_str());
	gDirectory->cd(tmpInquire->getDecName().c_str());
	
	PixConfDBInterface* myDBTemplate = 0;
	
	if(modMCC == 1 && modFE == 0){
	  myDBTemplate = new TurboDaqDB(plPath+"/rootDB1.0_1_1.cfg");	
	}
	else if(modMCC == 1 && modFE == 1){
	  myDBTemplate = new TurboDaqDB(plPath+"/rootDB1.0_1_2.cfg");	
	}
	else if(modMCC == 2 && modFE == 0){
	  myDBTemplate = new TurboDaqDB(plPath+"/rootDB1.0_2_1.cfg");	
	}
	else if(modMCC == 2 && modFE == 1){
	  myDBTemplate = new TurboDaqDB(plPath+"/rootDB1.0_2_2.cfg");	
	}
	else if(modMCC == 0 && modFE == 0){
	  myDBTemplate = new TurboDaqDB(plPath+"/rootDB1.0_0_1.cfg");	
	}
	else if(modMCC == 0 && modFE == 1){
	  myDBTemplate = new TurboDaqDB(plPath+"/rootDB1.0_0_2.cfg");	
	}
	else{
	  stringstream a;
	  a << "PixLib::rootGroupModuleWizard: not valid FE and/or MCC types. MCC " << modMCC << " FE " << modFE;
	  throw PixDBException(a.str());
	}
	
	root = myDBTemplate->readRootRecord(1);
	pixModule = *(root->getInquire(0));
	// tmp before correcting versioning in root
	string keyToDelete("PixModule;*");
	gDirectory->Delete(keyToDelete.c_str());
	//
	recordIterator tiiter(DB->processInquire(pixModule));
	DB->DBProcess(tiiter,COMMITREPLACE);
	DB->m_theLoadedInquires.Remove(tmpInquire);
	
	
	decName = turboDaqConfigFileNames[i]; 
	if(tdaqName){
	  decName = getTdaqModuleName(turboDaqConfigFileNames[i]);
	} else{
	  if(decName.substr(decName.length()-4,4) == ".cfg") decName = decName.substr(0,decName.length()-4);
	  if((int)decName.find_last_of('/') != (int)std::string::npos) decName = decName.substr(decName.find_last_of('/')+1, decName.length() - (decName.find_last_of('/')+1));
	}
	stringstream b;
	b << decName; b << "/"; b << name; 				
	/*std::cout << * */(tmpInquire = dynamic_cast<RootDBInquire*>(*(groupModuleInquire->findRecord(b.str()))));
	tiiter = groupModuleInquire->findRecord(b.str());
	delete myDBTemplate;
	
	// now read the turbodaq files, and copy the content of files into the rootDB module structure
	root = myDB->readRootRecord(1);
	if(modMCC!=0){
	  // dealing with mcc tdaq fields
	  DBInquire* tDaqMcc = *(root->findRecord("PixMcc"));
	  DBInquire* rMcc = *(tmpInquire->findRecord("PixMcc_0/PixMcc"));
	  
	  for(int j = 0; j < 16; j++){
	    stringstream tdmccfname;
	    tdmccfname << "MCC CAL strobe-delay range "; 
	    tdmccfname << j;
	    stringstream rmccfname;
	    rmccfname << "Strobe_DELAY_";
	    rmccfname << j;
	    float tmpfloat;
	    myDB->DBProcess(tDaqMcc->findField(tdmccfname.str()),READ,tmpfloat);
	    DB->DBProcess(rMcc->findField(rmccfname.str()),COMMIT, tmpfloat);
	    
	  }
	  
	  string tmpstring;
	  myDB->DBProcess(tDaqMcc->findField("ClassInfo_ClassName"),READ,tmpstring);
	  DB->DBProcess(rMcc->findField("ClassInfo_ClassName"),COMMIT, tmpstring);
	  string mccFlavour = "No valid value: ";
	  mccFlavour += tmpstring;
	  
	  
	  fieldIterator modField = tmpInquire->findField("general_MCC_Flavour");
	  DB->DBProcess(rMcc->findField("ClassInfo_ClassName"),READ, tmpstring);
	  if(tmpstring == "PixMccI1") mccFlavour = "MCC_I1"; else if (tmpstring == "PixMccI2") mccFlavour = "MCC_I2"; 
	  DB->DBProcess(modField,COMMIT,mccFlavour);
	  
	  r = tmpInquire->findRecord("PixMcc_0/PixMcc");
	  DB->DBProcess(r,COMMITREPLACE);
	}

	// dealing with fe fields
	int nfe=16;
	if(modMCC==0) nfe=1;
	for(int z = 0; z < nfe; z++){
	  stringstream fename;
	  fename << "PixFe_";
	  fename << z;
	  fename << "/PixFe";
	  DBInquire* tDaqFe = *(root->getInquire(z+(int)(modMCC!=0)));
	  DBInquire* rFe = *(tmpInquire->findRecord(fename.str()));
	  r = tmpInquire->findRecord(fename.str());
	  
	  if(z == 0){
	    string tmpstring;
	    myDB->DBProcess(tDaqFe->findField("ClassInfo_ClassName"),READ,tmpstring);
	    string feFlavour = "No valid value: ";
	    feFlavour += tmpstring;
	    
	    fieldIterator modField = tmpInquire->findField("general_FE_Flavour");
	    if(tmpstring == "PixFeI1") feFlavour = "FE_I1"; else if (tmpstring == "PixFeI2") feFlavour = "FE_I2"; 
	    DB->DBProcess(modField,COMMIT,feFlavour);
	  }
	  
	  string tmpstring;
	  
	  // dealing with misc
	  string tdaqmiscname = "ClassInfo_ClassName";
	  string rmiscname = "ClassInfo_ClassName";
	  fieldIterator tfi;
	  fieldIterator rfi;
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "Cinj-HI";
	  rmiscname = "Misc_CInjHi";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  tdaqmiscname = "Cinj-LO";
	  rmiscname = "Misc_CInjLo";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "global configuration enable";
	  rmiscname = "Misc_ConfigEnable";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "global scan_readout enable";
	  rmiscname = "Misc_ScanEnable";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "VCAL-FE gradient 0";
	  rmiscname = "Misc_VcalGradient0";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "VCAL-FE gradient 1";
	  rmiscname = "Misc_VcalGradient1";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "VCAL-FE gradient 2";
	  rmiscname = "Misc_VcalGradient2";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "VCAL-FE gradient 3";
	  rmiscname = "Misc_VcalGradient3";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "geographical address";
	  rmiscname = "Misc_Address";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "Internal-injection offset correction";
	  rmiscname = "Misc_OffsetCorrection";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  tdaqmiscname = "DACs enable";
	  rmiscname = "Misc_DacsEnable";
	  if(( (tfi = tDaqFe->findField(tdaqmiscname)) != tDaqFe->fieldEnd() ) && 
	     ( (rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()) ){
	    rfi.copyData(tfi);
	  }
	  
	  rmiscname = "Misc_Index";
	  int index = z;
	  if((rfi = rFe->findField(rmiscname)) != rFe->fieldEnd()){
	    DB->DBProcess(rfi,COMMIT, index);
	  }
	  
	  
	  // dealing with global register
	  DBInquire* rFeGR = *(rFe->findRecord("GlobalRegister_0/GlobalRegister"));
	  {for(int y = 0; y < rFeGR->getDependingFieldSize(); y++){
	    string grfname = (*(rFeGR->getField(y)))->getName();
	    string tdafname = grfname.substr(grfname.find_first_of("_",0)+1,grfname.length());
	    fieldIterator fi;
	    if((fi = tDaqFe->findField(tdafname)) != tDaqFe->fieldEnd()){
	      rFeGR->getField(y).copyData(fi);
	    }
	  }}
	  // dealing with trims
	  DBInquire* rFeTR = *(rFe->findRecord("Trim_0/Trim"));
	  {for(int y = 0; y < rFeTR->getDependingFieldSize(); y++){
	    string trfname = (*(rFeTR->getField(y)))->getName();
	    string tdafname = trfname.substr(trfname.find_first_of("_",0)+1,trfname.length());
	    fieldIterator fi;
	    if((fi = tDaqFe->findField(tdafname)) != tDaqFe->fieldEnd()){
	      rFeTR->getField(y).copyData(fi);
	    }
	  }}
	  // dealing with PixelRegister
	  DBInquire* rFePR = *(rFe->findRecord("PixelRegister_0/PixelRegister"));
	  {for(int y = 0; y < rFePR->getDependingFieldSize(); y++){
	    string prfname = (*(rFePR->getField(y)))->getName();
	    string tdafname = prfname.substr(prfname.find_first_of("_",0)+1,prfname.length());
	    fieldIterator fi;
	    if((fi = tDaqFe->findField(tdafname)) != tDaqFe->fieldEnd()){
	      rFePR->getField(y).copyData(fi);
	    }
	  }}
	  DB->DBProcess(r,COMMITREPLACETREE);
	}
	
	DB->DBProcess(tiiter,COMMITREPLACE);
	retInq.push_back(*tiiter);

      }
    }
    
    
  }
  
  gDirectory = dir;

  return retInq;
}
std::string PixLib::getTdaqModuleName(std::string file_name){
  std::ifstream *TDFile = new std::ifstream(file_name.c_str());
  std::string line;
  while(!TDFile->eof()){
    std::getline(*TDFile,line);
    if((int)line.find("Module string identifier")!=(int)std::string::npos){
      *TDFile >> line;
      TDFile->close();
      return line;
    }
  }
  TDFile->close();
  line = "new module";
  return line;
}

