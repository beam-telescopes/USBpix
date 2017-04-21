/////////////////////////////////////////////////////////////////////
// TurboDaqDB.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 25/06/05  Version 2.0 (GG)
//           interface change
// 13/10/03  Version 1.0 (GG)
//           Initial release
//
//! Data Base implementation on TurboDaq existing files Abstract classes for the PixLib Database handling
#ifndef __TURBODAQ_DB
#define __TURBODAQ_DB

#ifdef WIN32
#pragma warning(disable: 4100)
#endif

#include "PixConfDBInterface.h"
#include <map>

using namespace PixLib;
using std::vector;
using std::string;

namespace PixLib{
  class TurboDaqDB;
  class TurboDaqDBInquire;
  
  class TurboDaqDBField : public DBField{ // Actual implementation of abstract interface PixLib::DBField
    friend class PixLib::TurboDaqDBInquire; // this class is friend to access to peculiar data and methods of TurboDaqDBField
    friend class PixLib::TurboDaqDB;  // this class is friend to access to peculiar data and methods of TurboDaqDBField
  protected:
    PixConfDBInterface* m_myDB; // pointer to database to which this field belongs
    string m_name; // field name
    string m_decName; // field full decorated name
    enum DBDataType m_dataType; // field data type (int, vector<int> etc. (see definition of enum DBDataType in PixConfDBInterface.h
    // content of the field
    bool m_boolContent;
    int m_integerContent;
    unsigned int m_ulintegerContent;
    string m_stringContent;
    float m_floatContent;
    double m_doubleContent;
    vector<int> m_vIntegerContent;
    vector<float> m_vFloatContent;
    vector<double> m_vDoubleContent;
    vector<bool> m_vBoolContent;
  public:
    // Constructor
    TurboDaqDBField(string n, enum DBDataType dt = EMPTY) : m_myDB(0), m_name(n), m_decName(""), m_dataType(dt){}
    // destructor
    virtual ~TurboDaqDBField(){std::cout << "Destructed field" << std::endl;}
    // Accessors
    virtual string& getName() {return m_name;} // read the field name
    virtual string& getDecName() {return m_decName;} // read the field decorated name
    virtual PixConfDBInterface* getDB() const {if(m_myDB) return m_myDB; else throw PixDBException("Error DBField::getDB(): null pointer requested");} // get the DataBase of this field;
    virtual enum DBDataType getDataType() const {return m_dataType;} // read the data type
    virtual void dump(std::ostream& os) const ; // Dump - debugging purpose
    // Modifiers
  };
  
  class TurboDaqDBInquire : public DBInquire{ // Actual implementation of DBInquire abstract class 
    friend class PixLib::TurboDaqDB;
  protected:
    vector<DBField*> m_recordFields; // the inquire fields
    vector<DBInquire*> m_recordSubRecords; // the inquire depending inquires
    string m_name; // the inquire name
    string m_decoratedName; // the inquire full decorated name
    string m_myDecName; // the proprer inquire decorated name 
    PixConfDBInterface* m_myDB; // the database the inquire belongs to
  public:
    // Constructor
    TurboDaqDBInquire(string n) : m_name(n), m_decoratedName(""), m_myDecName(n), m_myDB(0) {}  
    TurboDaqDBInquire(string n, PixConfDBInterface* db);  
    // Destructor
    virtual ~TurboDaqDBInquire(){std::cout << "destructed inquire" << std::endl;}
    // Accessors
    virtual PixConfDBInterface* getDB() const {if(m_myDB) return m_myDB; else throw PixDBException("Error DBInquire::getDB(): null pointer requested");} // get the DataBase of this inquire;
    virtual string& getName() {return m_name;} // get the record name - the base class name of the object the inquire refers to
    virtual string& getDecName() {return m_decoratedName;} // get the fully decorated name
    virtual string& getMyDecName(){ return m_myDecName;}  // get the decorated name portion of this inquire
    virtual recordIterator recordBegin() const ; // the iterator to the depending records	
    virtual fieldIterator fieldBegin()const ; // the iterator to the record data fields	
    virtual recordIterator recordEnd()const ; // the end iterator to the depending records	
    virtual fieldIterator fieldEnd()const; // the end iterator to the record data fields	
    virtual recordIterator findRecord(const string recordName)const ; // find a subrecord by its name
    virtual fieldIterator  findField(const string fieldName)const ; // find a field by its name 
    virtual recordIterator findRecord(const char* name)const; // find a subrecord by its name
    virtual fieldIterator  findField(const char* name)const; // find a field by its name 
    virtual int getDependingInquireSize(void) const; // Get the amount of depending inquires
    virtual int getDependingFieldSize(void) const; // Get the amount of depending field
    virtual fieldIterator getField(int fieldNumber)const; // Get the pointer to the field number fieldNumber; throw an exception if fails
    virtual recordIterator getInquire(int inquireNumber)const; // Get the pointer to the inquire number inquireNumber; throw an exception if fails
    virtual void dump(std::ostream& os) const; // Dump - debugging purpose
    // Modifiers
    virtual fieldIterator pushField(DBField* in); // add a field to the DBInquire fields and return the corresponding iterator
    virtual void eraseField(fieldIterator it); // erase a field - not implemented.
    virtual recordIterator pushRecord(DBInquire* in);// add a record to the record list and return the corresponding iterator
    virtual void eraseRecord(recordIterator it); // erase a record - not implemented
    virtual DBInquire* addInquire(string name, string dirName); //! add an inquire
  };

  class TurboDaqDB : public PixConfDBInterface { // actual implementation of abstract class PixConfDBInterface
  protected:
    void insertField(DBInquire* in, TurboDaqDBField* fi);
    void insertInquire(DBInquire* in, TurboDaqDBInquire* toin);
    void openTurboDaqFiles(string filename);
    void readModuleConfig(DBInquire *pixmod, int noc);
    void readSingleChipConfig(DBInquire *pixmod);
    int columnPairMaskDecode(int mask, int chipNumber);
    void boolMaskFileDecode(vector<bool>& vect, int valflag, std::ifstream* file);
    void intMaskFileDecode(vector<int>&vect, int valflag, std::ifstream* file);
    vector<string> appCfgFileLineDecode(string s);
    DBDataType decodeFieldType(string s);
    void openAppCfgFile(void);
    string getFullPath(string relPath);
    
    string m_version;
    string m_moduleCfgFilePath;
    std::ifstream m_appCfgFile;
    std::ifstream* m_moduleCfgFile;
    std::vector<std::ifstream*> m_readoutMaskFiles;
    std::vector<std::ifstream*> m_strobeMaskFiles;
    std::vector<std::ifstream*> m_preampMaskFiles;
    std::vector<std::ifstream*> m_hitbusMaskFiles;
    std::vector<std::ifstream*> m_tdacFiles;		
    std::vector<std::ifstream*> m_fdacFiles;		
    TurboDaqDBInquire* m_rootRecord;
    std::map<std::string,TurboDaqDBInquire*> m_mapDBInquireNames;
    std::multimap<std::string,TurboDaqDBField*> m_mapDBFieldNames;
    std::multimap<std::string,TurboDaqDBInquire*> m_mapDBInquireSimpleNames;
    
  public:
    ~TurboDaqDB(){}
    TurboDaqDB(); // empty constructor for inheritance
    TurboDaqDB(string nameFile); // TurboDaqDB actually read a text file the data are stored in
    TurboDaqDB(string nameFile, int option); // if option == 1, TurboDaqDB actually read only the turbodaq configuration file
    static void getLineDos(std::ifstream& is, string& str);
	// factory
	DBField* makeField(string name);
	DBInquire* makeInquire(string name, string decName);
    virtual DBInquire* readRootRecord(int ) const {return m_rootRecord;}  
    virtual DBInquire* readRootRecord(string ) const {return m_rootRecord;} // get the root record by tag
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, bool&) ;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>&);
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, int&) ;
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>&);
	virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int &);
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, float&);
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>&);
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, double&);
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>&);
    virtual bool DBProcess(fieldIterator , enum DBInquireType , Histo&){return false;}
    virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, string&);
    virtual bool DBProcess(recordIterator theRecord, enum DBInquireType mode);
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, bool&) ;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<bool>&);
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, int&) ;
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<int>&);
	virtual bool DBProcess(DBField* theField, enum DBInquireType mode, unsigned int &);
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, float&);
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<float>&);
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, double&);
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<double>&);
    virtual bool DBProcess(DBField* , enum DBInquireType , Histo&){return false;}
    virtual bool DBProcess(DBField* theField, enum DBInquireType mode, string&);
    virtual bool DBProcess(DBInquire* theRecord, enum DBInquireType mode);
    virtual void tag(DBInquire* therecord, enum DBInquireTagMode, string tagname); // tag the configuration of inquire pointed by iterator 
    virtual vector<string> getTags(DBInquire*, int ){vector<string> retval; return retval;} // get the tag list (defult value) or the tag name of one iteration of the pointed inquire
    virtual vector<string> getTags(DBInquire* theRecord){return getTags(theRecord, 0);}
    virtual std::vector<DBInquire*> DBFindRecordByName(enum DBFindMode mode, const string& name);
    virtual std::vector<DBField*> DBFindFieldByName(enum DBFindMode mode, const string& name);
  };
};

#endif
