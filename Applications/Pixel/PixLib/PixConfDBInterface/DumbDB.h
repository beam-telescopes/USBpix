
#ifndef __DUMBDB__
#define __DUMBDB__

#include "DBInquire.h"
#include <xercesc/dom/DOM.hpp>

 
 

using namespace pixlib;
using namespace std;
using namespace XERCES_CPP_NAMESPACE;

class DumbDBField : public DBField{
public:
	string name;	
	int iteration;
	int integerContent;
	string stringContent;
	float floatContent;
};

class DumbDBInquire : public DBInquire{
	vector<DumbDBField*> recordFields;
	vector<DumbDBInquire*> recordSubRecords;
	string name;
	int iteration;	
public:
	typedef vector<DumbDBInquire*>::iterator recordIterator;
	typedef vector<DumbDBField*>::iterator fieldIterator;
	string getName();
	void setIteration(int i);
	int getIteration();
	DBInquire::recordIterator recordBegin() {return *recordSubRecords.begin();} // the iterator to the depending records	
	DBInquire::fieldIterator fieldBegin() {return *recordFields.begin();} // the iterator to the record data fields	
	DBInquire::recordIterator recordEnd() {return *recordSubRecords.end();}  // the end iterator to the depending records	
	DBInquire::fieldIterator fieldEnd() {return *recordFields.end();} // the end iterator to the record data fields	
};

class DumbDB : public PixConfDBInterface {
		std::fstream datafile;
        DOMDocument *doc;
		DBInquire *currentInquire;
	public:
		DumbDB(string nameFile); // DumbDB actually read a text file the data are stored in
		DBInquire* readRootRecord(int iteration);  
	};

bool DBProcess(DBInquire::recordIterator theRecord, DBInquire::fieldIterator theField, int&);
bool DBProcess(DBInquire::recordIterator theRecord, DBInquire::fieldIterator theField, vector<int>&);
bool DBProcess(DBInquire::recordIterator theRecord, DBInquire::fieldIterator theField, float&);
bool DBProcess(DBInquire::recordIterator theRecord, DBInquire::fieldIterator theField, string&);


#endif