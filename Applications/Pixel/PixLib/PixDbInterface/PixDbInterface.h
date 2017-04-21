/////////////////////////////////////////////////////////////////////
// PixDBInterface.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 21/06/06  Version 1.0 (GG)
//           Initial release
//
//! Abstract classes for the PixLib Database handling

#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <ctime>
#include "PixDBException.h"

#ifndef __PIX_DB_INTERFACE
#define __PIX_DB_INTERFACE

namespace PixLib{
  
  using namespace std;
  using std::string;
  using std::vector;

  namespace PixDb{
    enum DbAccessType {DBREAD=0, DBCOMMIT, DBCOMMITTREE, DBCOMMITREPLACE, DBCOMMITREPLACETREE};
    enum DbDataType{DBBOOL=0, DBVECTORBOOL, DBINT, DBVECTORINT, DBULINT, DBFLOAT, DBVECTORFLOAT, DBDOUBLE, DBVECTORDOUBLE, DBHISTO, DBSTRING, DBEMPTY};
    enum DbFindMode{DBBYNAME=0, DBBYDECNAME, DBBYSUBINNAME, DBBYSUBINDECNAME}; 
    enum DbPushMode {DBREPLACE=0, DBLINK, DBRECURSE};

#ifdef __CINT__
    const char DbDataTypeNames[11][20];
#else
    const char DbDataTypeNames[12][30]  = {"bool", "vector<bool>", "int", "vector<int>", "unsignedint","float", "vector<float>", "double", "vector<double>", "Histo", "string", "empty"};
#endif

  };

  using namespace PixLib::PixDb;
  
  class DbField;
  class DbRecord;

  class dbFieldIterator{    
  public:
    int m_number; // which field number is connected this pointer
    DbField* m_field; // which field this iterator points to
    DbRecord* m_parentRecord; // which record this iterator refers
  public:
    // Accessors
    DbField* operator*() const;
    DbField** operator->();
    int isNumber() const {return m_number;};
    // Modifiers
    dbFieldIterator& operator++();
    dbFieldIterator operator++(int);
    friend bool operator==(const dbFieldIterator& left,const dbFieldIterator& right);
    friend bool operator!=(const dbFieldIterator& left,const dbFieldIterator& right);
    void copyData(const dbFieldIterator fieldFromCopy);
    void pointsTo(DbField* newField); // make the iterator point to another field
    // constructors
    dbFieldIterator(int number, DbField* field, DbRecord* record) : m_number(number), m_field(field), m_parentRecord(record){}
    dbFieldIterator(): m_number(-1), m_field(0), m_parentRecord(0) {}
    dbFieldIterator(DbField* field) : m_number(-1), m_field(field), m_parentRecord(0) {}
    // destructor (delete the stored field!)
    ~dbFieldIterator();
  };


  class dbRecordIterator{
  public:
    int m_number; // which record number is connected this pointer
    DbRecord* m_record; // which record this iterator points to
    DbRecord* m_parentRecord; // which record this iterator has to ask for depending Records
  public:
    // Accessors
    DbRecord* operator*() const;
    DbRecord** operator->();
    int isNumber() const {return m_number;}
    // Modifiers
    dbRecordIterator& operator++();
    dbRecordIterator operator++(int);
    friend bool operator==(const dbRecordIterator& left,const dbRecordIterator& right);
    friend bool operator!=(const dbRecordIterator& left,const dbRecordIterator& right);
    void pointsTo(DbRecord* newRecord); // make the iterator point to another record
    // constructors
    dbRecordIterator(int number, DbRecord* record, DbRecord* parent) : m_number(number), m_record(record), m_parentRecord(parent){}
    dbRecordIterator(): m_number(-1), m_record(0), m_parentRecord(0) {}
    dbRecordIterator(DbRecord* record) : m_number(-1), m_record(record), m_parentRecord(0) {}
    //destructor (delete the stored record)
    ~dbRecordIterator();
  };

  class PixDbInterface;
  class DbRecord;
  class DbField{ // this class is an interface to the actual field of a record
  public:
    // Accessors
    virtual PixDbInterface* getDb() const = 0; // get the DataBase of this field;
    virtual enum DbDataType getDataType() const = 0; // read the data type
    virtual void dump(std::ostream& os) const = 0; // Dump - debugging purpose
    // Modifiers
    virtual string& getName() = 0 ; // read the field name - may also set the name 
    virtual string& getDecName() = 0 ; // read the field decorated name may also set the decorated name
    // Virtual destructor
    virtual ~DbField(){};
  };
  
  class DbRecord{ // this class is an interface to the actual record used by a actual database
  public:
    // Accessors
    virtual PixDbInterface* getDb() const = 0; // get the DataBase of this record;
    virtual dbRecordIterator recordBegin() = 0; // the iterator to the depending records	
    virtual dbFieldIterator fieldBegin() = 0; // the iterator to the record data fields	
    virtual dbRecordIterator recordEnd() = 0; // the end iterator to the depending records	
    virtual dbFieldIterator fieldEnd() = 0; // the end iterator to the record data fields	
    virtual dbRecordIterator findRecord(const string recordName) = 0; // find a subrecord by its name
    virtual dbFieldIterator findField(const string fieldName) = 0; // find a field by its name 
    virtual dbRecordIterator findRecord(const char* name) = 0; // find a subrecord by its name
    virtual dbFieldIterator findField(const char* name) = 0; // find a field by its name 
    virtual void dump(std::ostream& os) const = 0; // Dump - debugging purpose
    virtual int getDependingRecordSize(void) const = 0; // Get the amount of depending records
    virtual int getDependingFieldSize(void) const = 0; // Get the amount of depending field
    virtual dbFieldIterator getField(int fieldNumber) = 0; // Get the pointer to the field number fieldNumber; throw an exception if fails
    virtual dbRecordIterator getRecord(int recordNumber) = 0; // Get the pointer to the record number recordNumber; throw an exception if fails

    // Modifiers
    virtual string& getName() = 0; // get the record name - the base class name of the object the record refers to
    virtual string& getDecName() = 0; // get the fully decorated name
    virtual string& getMyDecName() = 0; // get the decorated name portion of this record

    virtual dbFieldIterator pushField(DbField* in) = 0; // add a field to the DbRecord fields and return the corresponding iterator
    virtual void eraseField(dbFieldIterator it) = 0; // erase a field.
    virtual dbRecordIterator linkRecord(DbRecord* in) = 0; // link an existing record and return the corresponding iterator
    virtual dbRecordIterator pushRecord(DbRecord* in, enum DbPushMode mode = DBREPLACE) = 0; // add a record to the record list and return the corresponding iterator
    virtual void eraseRecord(dbRecordIterator it) = 0; // erase a record
    virtual DbRecord* addRecord(string name, string dirName) = 0; // add a record
    // Virtual destructor
    virtual ~DbRecord() {};
  };
  
  class Histo;

  class PixDbInterface{ // this class is an interface to the actual Db used by the library
  public:
    // Accessors
    virtual DbRecord* readRootRecord() = 0; // get the root record by iteration number
    // Factory
    virtual DbField* makeField(string name) = 0;
    virtual DbRecord* makeRecord(string name, string decName) = 0;
    // Modifiers
    virtual void setTag(std::string tagName) = 0; // set the date of object to be read from Db to t1 
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, bool&) = 0; // read or commit the field pointed by the iterator
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<bool>&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, int&) = 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<int>&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, unsigned int &) = 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, float&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<float>&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, double&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<double>&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, Histo&)= 0;
    virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, string&)= 0;
    virtual dbRecordIterator DbProcess(DbRecord* theRecord, enum DbAccessType mode) = 0; // read or commit (possibly recursively) the record 
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, bool&) = 0; // read or commit the field pointed by the iterator
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<bool>&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, int&) = 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<int>&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, unsigned int &) = 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, float&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<float>&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, double&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<double>&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, Histo&)= 0;
    virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, string&)= 0;
    virtual dbRecordIterator DbProcess(dbRecordIterator theRecord, enum DbAccessType mode) = 0; // read or commit (possibly recursively) the record pointed by iterator
    virtual DbRecord* DbFindRecordByName(const string& name) = 0; // find a record by its name
    virtual DbField* DbFindFieldByName(const string& name) = 0; // find a data field by its name
    // Virtual destructor
    virtual ~PixDbInterface(){};
  };
  
  std::ostream& operator<<(std::ostream& os, const DbField& data); // output operator to dump the data
  std::ostream& operator<<(std::ostream& os, const DbRecord& data); // output operator to dump the record

  
};


#endif
