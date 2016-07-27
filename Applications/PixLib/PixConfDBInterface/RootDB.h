/////////////////////////////////////////////////////////////////////
// RootDB.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 25/06/05  Version 2.0 (GG)
//           interface change
// 13/10/03  Version 1.0 (GG)
//           Initial release
//
//! Implementation of abstract DB using root
#ifndef __ROOTDB
#define __ROOTDB

#ifdef WIN32
// diable warning in dict file
#pragma warning(disable: 4100)
// get rid of ROOT warning - nothing we can do about them
#pragma warning(disable: 4189)
#pragma warning(disable: 4800)
#pragma warning(disable: 4996)
#endif

#include "PixConfDBInterface.h"
#include "TurboDaqDB.h"
#include <string>
#include <map>
#include "TNamed.h"
#include "TFile.h"
#include "TBtree.h"
#include "TArrayI.h"
#include "TArrayF.h"
#include "TArrayD.h"
#include "Histo/Histo.h"

#include "TCanvas.h"

using namespace PixLib;

namespace PixLib{
	class RootDBInquire;
	class RootDB;
	class Histo;

	class RootDBField : public TNamed, public DBField{
		static TCanvas* dumpCanvas; //! service canvas
		friend class RootDBInquire;
		friend class RootDB;
#ifndef __CINT__
		friend std::vector<DBInquire*> rootGroupModuleWizard(RootDBInquire* groupModuleInquire, float rootDBVersion, float turboDaqVersion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath, bool tdaqName);
#endif
		string m_name; // the field name
		string m_decName; // the fully decorated name
		TArrayI m_myIntCont;    // Integer content of the field
		TArrayF m_myFloatCont;  // float content of the field
		TArrayD m_myDoubleCont;  // float content of the field
		TString m_myStringCont; // string content of the field
		Histo m_myHistoCont; //! reference to histogram stored into field - saved as vector
		PixConfDBInterface* m_myDB; //! pointer to the database (not permanent)
		enum DBDataType m_dataType; // field data type (int, vector<int> etc. (see definition of enum DBDataType in PixConfDBInterface.h
		bool m_modified; //! flag true if a commit has been done on the field
		
		void copyField(fieldIterator f); // copy a field from iterator 
		
		RootDBField(string name); // private constructor to be used by RootDB TBtree find method 		
	public:
		RootDBField(string name, PixConfDBInterface* myDB);
		RootDBField() : m_myDB(NULL), m_dataType(EMPTY), m_modified(true){}
		// Accessors
		virtual string& getName() {return m_name;} // read the field name from TNamed object
		virtual string& getDecName() {return m_decName;} // read the field decorated name
		virtual PixConfDBInterface* getDB() const {return m_myDB;} // get the DataBase of this field;
		virtual enum DBDataType getDataType() const {return m_dataType;} // read the data type
		virtual void dump(std::ostream& os) const; // Dump - debugging purpose
		// Modifiers
		// Virtual destructor
		virtual ~RootDBField(){}
		// ROOT specific function overload
		void Dump(void); // *MENU* Dump to screen the content of this RootDBField
		void ModifyFieldValue(char* newValue); // *MENU* modify the content of the Field
		ULong_t Hash() const;
		Bool_t  IsEqual(const TObject *obj) const;
		Bool_t  IsSortable() const;
		Int_t   Compare(const TObject *obj) const;
		// make ROOT know about RootDBField (note:this must be last line of the class definition
		ClassDef(RootDBField,1) 
	};

	class Histo;
	
	class RootDBInquire : public TNamed, public DBInquire{
		
		friend class RootDB;
		
#ifndef __CINT__
		friend std::vector<DBInquire*> rootGroupModuleWizard(RootDBInquire* groupModuleInquire, float rootDBVersion, float turboDaqVerion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath, bool tdaqName);
#endif
		
		string m_name; // the inquire name
		string m_decName; // the inquire decorated name
		string m_myDecName; // the inquire fully decorated name
		vector<string> m_myFieldNameList; // the list of names of the fields belonging to this inquire
		vector<string> m_myRecordNameList; // the list of names of the record belonging to this inquire
		RootDB* m_myDB; //! pointer to the database (not permanent)
		RootDBInquire(string decname);  // private constructor to be used by RootDB 
		
		void makeDecName(RootDBInquire* child, RootDBInquire* parent);
		int getVersionFromFile(RootDBInquire* dbi);
		
	public:
		// Constructors
		RootDBInquire() : m_name(""), m_myDB(0) {};
		RootDBInquire(string name, PixConfDBInterface* mydb );
		RootDBInquire(string name, string decName, string myDecName, PixConfDBInterface* mydb );
		// Accessors
		virtual PixConfDBInterface* getDB() const; // get the DataBase of this inquire;
		virtual recordIterator recordBegin() const ; // the iterator to the depending records		
		virtual fieldIterator fieldBegin() const ; // the iterator to the record data fields	
		virtual recordIterator recordEnd() const; // the end iterator to the depending records	
		virtual fieldIterator fieldEnd() const; // the end iterator to the record data fields	
		virtual recordIterator findRecord(const string recordName)const ; // find a subrecord by its name
		virtual fieldIterator findField(const string fieldName)const; // find a field by its name 
		virtual recordIterator findRecord(const char* name)const; // find a subrecord by its name
		virtual fieldIterator findField(const char* name)const; // find a field by its name 
		virtual void dump(std::ostream& os) const; // Dump - debugging purpose
		virtual int getDependingInquireSize(void) const; // Get the amount of depending inquires
		virtual int getDependingFieldSize(void) const; // Get the amount of depending field
		virtual fieldIterator getField(int fieldNumber) const; // Get the pointer to the field number fieldNumber; throw an exception if fails
		virtual recordIterator getInquire(int inquireNumber) const; // Get the pointer to the inquire number inquireNumber; throw an exception if fails
		// Modifiers
		virtual string& getName() {return m_name;} // get the record name - the base class name of the object the inquire refers to
		virtual string getName() const {return m_name;} // get the record name - the base class name of the object the inquire refers to
		virtual string& getDecName() {return m_decName;} // get the fully decorated name
		virtual string getDecName() const {return m_decName;} // get the fully decorated name
		virtual string& getMyDecName() {return m_myDecName;} // get the decorated name portion of this inquire
		virtual string getMyDecName() const {return m_myDecName;} // get the decorated name portion of this inquire
		virtual fieldIterator pushField(DBField* in); // add a field to the DBInquire fields and return the corresponding iterator
		virtual void eraseField(fieldIterator it); // erase a field.
		virtual recordIterator pushRecord(DBInquire* in); // add a record to the record list and return the corresponding iterator
		virtual void eraseRecord(recordIterator it); // erase a record
		virtual DBInquire* addInquire(string name, string dirName); //! add an inquire
		// ROOT specific function overload
		void Dump(void); // *MENU* Dump to screen the content of this RootDBInquire
		void ChangeDependingInquire(int inquirePosition, char* newDependingInquire); // *MENU* modify the depending inquire at position inquirePosition
		void RemoveDependingInquire(int inquirePosition); // *MENU* remove the depending inquire at position inquirePosition
		void AddDependingInquire(char* newDependingInquire); // *MENU* add the depending inquire
		void DumpFields(void); // *MENU* dump on screen the inquire and its fields
		void AppendThisToAnotherInquire(char * newParentInquire, char* newDecName); // *MENU* copy this inquire and the depending inquire to the new parent inquire (found by its decName)

		ULong_t Hash() const;
		Bool_t  IsEqual(const TObject *obj) const;
		Bool_t  IsSortable() const;
		Int_t   Compare(const TObject *obj) const;
		// Virtual destructor
		virtual ~RootDBInquire() {}
		ClassDef(RootDBInquire,1) 
	};
	
	
	class RootDB : public PixConfDBInterface {
		friend class RootDBInquire;
#ifndef __CINT__
		friend std::vector<DBInquire*> rootGroupModuleWizard(RootDBInquire* groupModuleInquire, float rootDBVersion, float turboDaqVerion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath, bool tdaqName);
#endif
		
		TFile* m_theFile;
		string m_theFileName;
		TBtree m_theLoadedInquires;
		TBtree m_theLoadedFields;
		
		void makeRootFileFromTurboDaqDB(string namefile, string mode);
		DBInquire* processInquire(DBInquire* currInquire);
		void processRootInquire(DBInquire* currInquire);
		void processRootInquireTree(DBInquire* currInquire);
		void processRootInquireReplace(DBInquire* currInquire);
		void processRootInquireReplaceTree(DBInquire* currInquire);
		RootDBInquire* getInquireByDecName(string name) const;
		RootDBField* getFieldByDecName(string name) const;
		RootDBInquire* memoryGetInquireByDecName(string name);
		RootDBField* memoryGetFieldByDecName(string name);
		bool getInquireByDecNameFromDisk(string name, RootDBInquire* toRead);
		void RootCDCreate(TDirectory* startingDir, RootDBInquire* inquire);
		string getAbsolutePath(TDirectory* currDir);
		
	public:
		// constructors
		RootDB(string namefile, string mode = "READ"); // construct a rootdb database from the file name
		// Factory
		virtual DBField* makeField(string name);
		virtual DBInquire* makeInquire(string name, string decName);
		// Accessors
		virtual DBInquire* readRootRecord(int iteration) const; // get the root record 
		virtual DBInquire* readRootRecord(string tag) const; // get the root record by tag
		// Modifiers
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, bool&); // read or commit the field pointed by the iterator
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<bool>&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, int&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<int>&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, unsigned int &);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, float&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<float>&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, double&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, vector<double>&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, Histo&);
		virtual bool DBProcess(DBField* theField, enum DBInquireType mode, string&);
		virtual bool DBProcess(DBInquire* therecord, enum DBInquireType mode); // read or commit (possibly recursively) the inquire pointed by iterator
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, bool&); // read or commit the field pointed by the iterator
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, int&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int &);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, float&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, double&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, Histo&);
		virtual bool DBProcess(fieldIterator theField, enum DBInquireType mode, string&);
		virtual bool DBProcess(recordIterator therecord, enum DBInquireType mode); // read or commit (possibly recursively) the inquire pointed by iterator
		virtual void tag(DBInquire* , enum DBInquireTagMode, string ){} // tag the configuration of inquire pointed by iterator 
		virtual vector<string> getTags(DBInquire*, int ){vector<string> a; return a;} // get the tag list (defult value) or the tag name of one iteration of the pointed inquire
		virtual vector<string> getTags(DBInquire* theRecord){return getTags(theRecord, 0);}
		virtual std::vector<DBInquire*> DBFindRecordByName(enum DBFindMode mode, const string& name); // find a record by its name, returning a vector of records which fulfill the find requests
		virtual std::vector<DBField*> DBFindFieldByName(enum DBFindMode mode, const string& name); // find a data field by its name, returning a vector of fields which fulfill the find requests
		// Virtual destructor
		virtual ~RootDB();
		ClassDef(RootDB,1) 
	};
	
	
	std::vector<double> histoToVector(Histo& his);
	Histo vectorToHisto(std::vector<double>& vec);
	int vectorHistoDim(vector<double>& vec); // returns 0 if vec is not an Histo, 1 or 2 as Histo dimension
	std::vector<DBInquire*> rootGroupModuleWizard(RootDBInquire* groupModuleInquire, float rootDBVersion, float turboDaqVerion, std::vector<std::string> turboDaqConfigFileNames, std::string plPath=".", bool tdaqName=false);
	std::string getTdaqModuleName(std::string file_name);
	
};


#endif
