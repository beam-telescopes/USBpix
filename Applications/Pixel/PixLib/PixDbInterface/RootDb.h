/////////////////////////////////////////////////////////////////////
// RootDb.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 23/06/06  Version 1.0 (GG)
//           Initial release
//! Implementation of abstract DB using root 

#ifndef _PIXLIB_ROOTDB
#define _PIXLIB_ROOTDB

#ifdef WIN32
// diable warning in dict file
#pragma warning(disable: 4100)
// get rid of ROOT warning - nothing we can do about them
#pragma warning(disable: 4189)
#pragma warning(disable: 4800)
#pragma warning(disable: 4996)
#endif

#include "PixDbInterface.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include <string>
#include <map>
#include "TNamed.h"
#include "TFile.h"
#include "TBtree.h"
#include "TArrayI.h"
#include "TArrayF.h"
#include "TArrayD.h"
#include "TCanvas.h"

using namespace PixLib;
using namespace PixLib::PixDb;

namespace PixLib{
  class RootDbRecord;
  class RootDb;
  
  class RootDbField : public TNamed, public DbField{
    static TCanvas* dumpCanvas; //! service canvas
    friend class RootDbRecord;
    friend class RootDb;
    friend void copyField(RootDbField* target, DBField* origin);

#ifndef __CINT__
    friend std::vector<DbRecord*> rootGroupModuleWizard(DbRecord* groupModuleRecord, float rootDbVersion, float turboDaqVerion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath, bool tdaqName);
#endif
    int m_id; //! the identifier in the tbtree (not permanent)
    string m_name; // the field name i.e. VCAL
    string m_decName; // the fully decorated name i.e. file:/dir/dir/dir/mydir(parentRecord)/VCAL;1
    TArrayI m_myIntCont;    // Integer content of the field
    TArrayF m_myFloatCont;  // float content of the field
    TArrayD m_myDoubleCont;  // float content of the field
    TString m_myStringCont; // string content of the field
    
    PixDbInterface* m_myDb; //! pointer to the database (not permanent)
    bool m_changedFromDisk; //! flag to inform if the field has been changed in memory (not permanent)
    enum DbDataType m_dataType; // field data type (int, vector<int> etc. (see definition of enum DbDataType in PixDbInterface.h
    
    RootDbField* copyField(dbFieldIterator f); // copy a field from a field iterator 
    
    RootDbField(string name, int m_id); // private constructor to be used by RootDb TBtree find method 		
  public:
    RootDbField(string name);
    RootDbField() : m_id(0), m_myDb(0), m_changedFromDisk(false), m_dataType(DBEMPTY){}
    // Accessors
    virtual string& getName() {return m_name;} // read the field name from TNamed object
    virtual string& getDecName() {return m_decName;} // read the field decorated name
    virtual PixDbInterface* getDb() const {return m_myDb;} // get the DataBase of this field;
    virtual enum DbDataType getDataType() const {return m_dataType;} // read the data type
    virtual void dump(std::ostream& os) const; // Dump - debugging purpose
    // Virtual destructor
    virtual ~RootDbField(){}
    // ROOT specific function overload
    void Dump(void); // *MENU* Dump to screen the content of this RootDbField
    void ModifyFieldValue(char* newValue); // *MENU* modify the content of the Field
    //    void DeleteFromFile(void); // *MENU* Delte a field from the file. ONLY MEANT TO CLEANING PURPOUSE: you never know if a field is still linked to some record
    ULong_t Hash() const;
    Bool_t  IsEqual(const TObject *obj) const;
    Bool_t  IsSortable() const;
    Int_t   Compare(const TObject *obj) const;
    // make ROOT know about RootDbField (note:this must be last line of the class definition
    ClassDef(RootDbField,1) 
};
  
class Histo;
  
class RootDbRecord : public TNamed, public DbRecord{
    
  friend class RootDb;
    
#ifndef __CINT__
  friend std::vector<DbRecord*> rootGroupModuleWizard(RootDbRecord* groupModuleRecord, float rootDbVersion, float turboDaqVerion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath, bool tdaqName);
#endif
  
  int m_id; //! the identifier in the tbtree (not permanent)
  string m_name; // the record name - the class object e.g. PixFE
  string m_decName; // the record decorated name - e.g. PixFE_1
  string m_myDecName; // the record fully decorated name e.g. file:/dir/dir/dir/mydir/M510902/PixFE_1/PixFE;1
  vector<string> m_myFieldNameList; // the list of names of the fields belonging to this record in format file:/dir/dir/dir/mydir/name
  vector<string> m_myRecordNameList; // the list of names of the record belonging to this record in format file:/dir/dir/dir/mydir/subdir/name
  RootDb* m_myDb; //! pointer to the database (not permanent)

  vector<DbField*> pushedFields; //! temporary memory cache
  vector<DbRecord*> pushedRecords; //! temporary memory cache
  
  RootDbRecord(string decname, int m_id);  // private constructor to be used by RootDb 
  
  void makeDecName(RootDbRecord* child, RootDbRecord* parent); // internal manipulator to link the child record to the parent
                               
  RootDbRecord* copyRecord(dbRecordIterator r); // copy a record from another record, including the fields


 public:
  std::string getFieldName(int number);
  std::string getRecordName(int number);
  // Constructors
  RootDbRecord() : m_id(0), m_name(""), m_myDb(0){};
  RootDbRecord(string name);
  RootDbRecord(string name, string decName);
  RootDbRecord(string name, string decName, string myDecName);
  // Accessors
  virtual PixDbInterface* getDb() const; // get the DataBase of this record;
  virtual dbRecordIterator recordBegin(); // the iterator to the depending records		
  virtual dbFieldIterator fieldBegin(); // the iterator to the record data fields	
  virtual dbRecordIterator recordEnd(); // the end iterator to the depending records	
  virtual dbFieldIterator fieldEnd(); // the end iterator to the record data fields	
  virtual dbRecordIterator findRecord(const string recordName); // find a subrecord by its name
  virtual dbFieldIterator findField(const string fieldName); // find a field by its name 
  virtual dbRecordIterator findRecord(const char* name); // find a subrecord by its name
  virtual dbFieldIterator findField(const char* name); // find a field by its name 
  virtual void dump(std::ostream& os) const; // Dump - debugging purpose
  virtual int getDependingRecordSize(void) const; // Get the amount of depending records
  virtual int getDependingFieldSize(void) const; // Get the amount of depending field
  virtual dbFieldIterator getField(int fieldNumber); // Get the pointer to the field number fieldNumber; throw an exception if fails
  virtual dbRecordIterator getRecord(int recordNumber); // Get the pointer to the record number recordNumber; throw an exception if fails
  // Modifiers
  virtual string& getName() {return m_name;} // get the record name - the base class name of the object the record refers to
  virtual string& getDecName() {return m_decName;} // get the fully decorated name
  virtual string& getMyDecName() {return m_myDecName;} // get the decorated name portion of this record
  virtual dbFieldIterator pushField(DbField* in); // add a field to the DbRecord fields and return the corresponding iterator
  virtual void eraseField(dbFieldIterator it); // erase a field.
  virtual dbRecordIterator linkRecord(DbRecord* in); // link an existing record and return the corresponding iterator
  virtual dbRecordIterator pushRecord(DbRecord* in, enum DbPushMode mode = DBREPLACE); // add a record to the record list and return the corresponding iterator
  virtual void eraseRecord(dbRecordIterator it); // erase a record
  virtual DbRecord* addRecord(string name, string dirName); // add an record
  // ROOT specific function overload
  void Dump(void); // *MENU* Dump to screen the content of this RootDbRecord
  void ChangeDependingRecord(int recordPosition, char* newDependingRecord); // *MENU* modify the depending record at position recordPosition
  void RemoveDependingRecord(int recordPosition); // *MENU* remove the depending record at position recordPosition
  void AddDependingRecord(char* newDependingRecord); // *MENU* add the depending record
  void DumpFields(void); // *MENU* dump on screen the record and its fields
  void AppendThisToAnotherRecord(char * newParentRecord, char* newDecName); // *MENU* copy this record and the depending record to the new parent record (found by its decName)
  
  ULong_t Hash() const;
  Bool_t  IsEqual(const TObject *obj) const;
  Bool_t  IsSortable() const;
  Int_t   Compare(const TObject *obj) const;
  // Virtual destructor
  virtual ~RootDbRecord() {}
  ClassDef(RootDbRecord,1) 
};
 
 
class RootDb : public PixDbInterface {
  friend class RootDbRecord;
#ifndef __CINT__
  friend std::vector<DbRecord*> rootGroupModuleWizard(DbRecord* groupModuleRecord, float rootDbVersion, float turboDaqVerion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath, bool tdaqName);
#endif
  
  TFile* m_theFile;
  string m_theFileName;
  TBtree m_theLoadedRecords;
  TBtree m_theLoadedFields;
  std::map<std::string, TFile*> m_theFileHandlers;

  
  DbRecord* processRecord(DbRecord* currRecord);
  bool commitRootRecordReplaceTree(RootDbRecord* currRecord); // recursively write to disk
  bool putRecord(RootDbRecord* recordToPut); // write a record into disk without taking it out from cache memory - record to put is valid after calling
  
  bool putField(RootDbField* fieldToPut, RootDbRecord* recordWherePut);
  RootDbRecord* getRecordByDecName(string name); // read from disk and add into cache memory
  RootDbField* getFieldByDecName(string name); // read from disk and add into cache memory
  bool getRecordByDecNameFromDisk(string name, RootDbRecord* toRead); // read from disk - do not change in memory
  bool getFieldByDecNameFromDisk(string name, RootDbField* toRead); // read from disk - do not change in memory
  void RootCDCreate(TDirectory* startingDir, RootDbRecord* record); // makes the directory tree pointed by record - propedeutic to any writing to the disk
  TFile* getFileHandler(std::string fileName);

 public:
  // constructors
  RootDb(string namefile, string mode = "READ", string tdaqnamefile = ""); // construct a rootdb database from the file name
  // Factory
  virtual DbField* makeField(string name);
  virtual DbRecord* makeRecord(string name, string decName);
  // Accessors
  virtual DbRecord* readRootRecord(); // get the root record 
  // Modifiers
  virtual void setTag(std::string /*tag*/){}; // set the date of object to be read from Db to t1 
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, bool&); // read or commit the field pointed by the iterator
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<bool>&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, int&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<int>&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, unsigned int &);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, float&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<float>&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, double&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, vector<double>&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, Histo&);
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode, string&);
  virtual dbRecordIterator DbProcess(DbRecord* therecord, enum DbAccessType mode); // read or commit (possibly recursively) the record pointed by iterator
  virtual dbFieldIterator DbProcess(DbRecord* theRecord, dbFieldIterator theFields, enum DbAccessType mode); // read or commit the record's fields in block transfer
  virtual dbFieldIterator DbProcess(DbField* theField, enum DbAccessType mode);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, bool&); // read or commit the field pointed by the iterator
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<bool>&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, int&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<int>&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, unsigned int &);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, float&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<float>&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, double&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<double>&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, Histo&);
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode, string&);
  virtual dbRecordIterator DbProcess(dbRecordIterator therecord, enum DbAccessType mode); // read or commit (possibly recursively) the record pointed by iterator
  virtual dbFieldIterator DbProcess(dbFieldIterator theField, enum DbAccessType mode);
  virtual DbRecord* DbFindRecordByName(const string& name); // find a record by its name
  virtual DbField* DbFindFieldByName(const string& name); // find a data field by its name
  // Virtual destructor
  virtual ~RootDb();
  ClassDef(RootDb,1) 
};
 
 void copyField(RootDbField* target, DBField* origin);
 string getMySubDir(string name);

};


#endif
