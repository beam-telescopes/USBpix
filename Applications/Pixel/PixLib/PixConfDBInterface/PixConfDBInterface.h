/////////////////////////////////////////////////////////////////////
// PixConfDBInterface.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 25/06/05  Version 2.0 (GG)
//           interface change
// 13/10/03  Version 1.0 (GG)
//           Initial release
//
// 
//! Abstract classes for the PixLib Database handling

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include "PixDbInterface/PixDBException.h"

#ifndef __PIX_CONFDB_INTERFACE
#define __PIX_CONFDB_INTERFACE

namespace PixLib{
  
  using namespace std;
  using std::string;
  using std::vector;

  enum DBInquireType {READ=0, COMMIT, COMMITTREE, COMMITREPLACE, COMMITREPLACETREE};
  enum DBDataType{BOOL=0, VECTORBOOL, INT, VECTORINT, ULINT, FLOAT, VECTORFLOAT, DOUBLE, VECTORDOUBLE, HISTO, STRING, EMPTY};
  enum DBFindMode{BYNAME=0, BYDECNAME, BYSUBINNAME, BYSUBINDECNAME}; 

#ifdef __CINT__
  const char DBDataTypeNames[11][20];
#else
  const char DBDataTypeNames[12][30]  = {"bool", "vector<bool>", "int", "vector<int>", "unsignedint","float", "vector<float>", "double", "vector<double>", "Histo", "string", "empty"};
#endif

  enum DBInquireTagMode {TAG, RECURSIVETAG};
  
  class DBField;
  class DBInquire;

  class fieldIterator{    
    int m_number; // which field number is connected this pointer
    DBField* m_field; // which field this iterator points to
    const DBInquire* m_parentInquire; // which inquire this iterator refers
  public:
    // Accessors
    DBField* operator*() const;
    DBField** operator->();
    // Modifiers
    fieldIterator& operator++();
    fieldIterator operator++(int);
    friend bool operator==(const fieldIterator& left,const fieldIterator& right);
    friend bool operator!=(const fieldIterator& left,const fieldIterator& right);
	void copyData(const fieldIterator fieldFromCopy);
	void pointsTo(DBField* newField); // make the iterator point to another field - lost ANY relationship with possible parent inquire and number
    // constructors
    fieldIterator(int number, DBField* field, const DBInquire* inquire) : m_number(number), m_field(field), m_parentInquire(inquire){}
    fieldIterator(): m_number(-1), m_field(NULL), m_parentInquire(NULL) {}
	fieldIterator(DBField* field) : m_number(-1), m_field(field), m_parentInquire(NULL) {}
  };


  class recordIterator{
    int m_number; // which record number is connected this pointer
    DBInquire* m_record; // which record this iterator points to
    const DBInquire* m_parentInquire; // which record this iterator has to ask for depending inquires
  public:
    // Accessors
    DBInquire* operator*() const;
    DBInquire** operator->();
    // Modifiers
    recordIterator& operator++();
    recordIterator operator++(int);
    friend bool operator==(const recordIterator& left,const recordIterator& right);
    friend bool operator!=(const recordIterator& left,const recordIterator& right);
	void pointsTo(DBInquire* newInquire); // make the iterator point to another inquire - lost ANY relationship with possible parent inquire
    // constructors
    recordIterator(int number, DBInquire* record, const DBInquire* parent) : m_number(number), m_record(record), m_parentInquire(parent){}
    recordIterator(): m_number(-1), m_record(NULL), m_parentInquire(NULL) {}
	recordIterator(DBInquire* record) : m_number(-1), m_record(record), m_parentInquire(NULL) {}
  };

  class PixConfDBInterface;
  class DBInquire;
  class DBField{ // this class is an interface to the actual field of an inquire
  public:
    // Accessors
    virtual string& getName() = 0 ; // read the field name - may also set the name 
    virtual string& getDecName() = 0 ; // read the field decorated name may also set the decorated name
    virtual PixConfDBInterface* getDB() const = 0; // get the DataBase of this field;
    virtual enum DBDataType getDataType() const = 0; // read the data type
    virtual void dump(std::ostream& os) const = 0; // Dump - debugging purpose
    // Modifiers
    // Virtual destructor
    virtual ~DBField(){};
  };
  
  class DBInquire{ // this class is an interface to the actual inquire used by a actual database
  public:
    // Accessors
    virtual PixConfDBInterface* getDB() const = 0; // get the DataBase of this inquire;
    virtual recordIterator recordBegin() const  = 0; // the iterator to the depending records	
    virtual fieldIterator fieldBegin() const = 0; // the iterator to the record data fields	
    virtual recordIterator recordEnd() const = 0; // the end iterator to the depending records	
    virtual fieldIterator fieldEnd() const = 0; // the end iterator to the record data fields	
    virtual recordIterator findRecord(const string recordName) const = 0; // find a subrecord by its name
    virtual fieldIterator findField(const string fieldName) const = 0; // find a field by its name 
    virtual recordIterator findRecord(const char* name) const  = 0; // find a subrecord by its name
    virtual fieldIterator findField(const char* name) const = 0; // find a field by its name 
    virtual void dump(std::ostream& os) const = 0; // Dump - debugging purpose
    virtual int getDependingInquireSize(void) const = 0; // Get the amount of depending inquires
    virtual int getDependingFieldSize(void) const = 0; // Get the amount of depending field
    virtual fieldIterator getField(int fieldNumber) const = 0; // Get the pointer to the field number fieldNumber; throw an exception if fails
    virtual recordIterator getInquire(int inquireNumber) const = 0; // Get the pointer to the inquire number inquireNumber; throw an exception if fails

    // Modifiers
    virtual string& getName() = 0; // get the record name - the base class name of the object the inquire refers to
    virtual string& getDecName() = 0; // get the fully decorated name
    virtual string& getMyDecName() = 0; // get the decorated name portion of this inquire
    virtual fieldIterator pushField(DBField* in) = 0; // add a field to the DBInquire fields and return the corresponding iterator
    virtual void eraseField(fieldIterator it) = 0; // erase a field.
    virtual recordIterator pushRecord(DBInquire* in) = 0; // add a record to the record list and return the corresponding iterator
    virtual void eraseRecord(recordIterator it) = 0; // erase a record
    virtual DBInquire* addInquire(string name, string dirName) = 0; //! Add an inquire

    // Virtual destructor
    virtual ~DBInquire() {};
  };
  
  class Histo;

  class PixConfDBInterface{ // this class is an interface to the actual DB used by the library
  public:
    // Accessors
    virtual DBInquire* readRootRecord(int iteration) const = 0; // get the root record by iteration number
    virtual DBInquire* readRootRecord(string tag) const = 0; // get the root record by tag
	// Factory
	virtual DBField* makeField(string name) = 0;
	virtual DBInquire* makeInquire(string name, string decName) = 0;
    // Modifiers
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, bool&) = 0; // read or commit the field pointed by the iterator
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<bool>&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, int&) = 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<int>&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, unsigned int &) = 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, float&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<float>&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, double&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<double>&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, Histo&)= 0;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, string&)= 0;
    virtual bool DBProcess(DBInquire* theRecord, enum DBInquireType mode) = 0; // read or commit (possibly recursively) the inquire pointed by iterator
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, bool&) = 0; // read or commit the field pointed by the iterator
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, int&) = 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int &) = 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, float&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, double&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, Histo&)= 0;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, string&)= 0;
    virtual bool DBProcess(recordIterator theRecord, enum DBInquireType mode) = 0; // read or commit (possibly recursively) the inquire pointed by iterator
    virtual std::vector<DBInquire*> DBFindRecordByName(enum DBFindMode mode, const string& name) = 0; // find a record by its name, returning a vector of iterator which fulfill the find requests
    virtual std::vector<DBField*> DBFindFieldByName(enum DBFindMode mode, const string& name) = 0; // find a data field by its name, returning a vector of iterator which fulfill the find requests
    // Virtual destructor
    virtual ~PixConfDBInterface(){};
  };
  
  std::ostream& operator<<(std::ostream& os, const DBField& data); // output operator to dump the data
  std::ostream& operator<<(std::ostream& os, const DBInquire& data); // output operator to dump the inquire

  
};


#endif
