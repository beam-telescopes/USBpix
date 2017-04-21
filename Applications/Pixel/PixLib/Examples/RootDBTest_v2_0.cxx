#include "TApplication.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixDbInterface/PixDbInterface.h"
#include "PixDbInterface/RootDb.h"
#include "Histo/Histo.h"

#include <iostream>
#include <exception>
#include <stdlib.h>
// example of application using the PixConfDBInterface to get informations from the Data Base

using namespace PixLib;
using std::string;


int main(int argc, char* argv[]){
  
  TApplication app("MyApp",0,0); // this is to allow pop up of graphical windows in root -> see root documentation
  
  if(argc != 3){
    std::cout << "Wrong program arguments. Usage:" << std::endl;
    std::cout << "RootDBTest -m <.cfg file> to create a new root file from a turbodaq file" << std::endl;
    std::cout << "RootDBTest -n <.cfg file> to create a new root file from .cfg" << std::endl;
    std::cout << "RootDBTest -u <.root file> to operate on RootDb file" << std::endl;
    return 1;
  }
  try{
    string argv1(argv[1]);
    PixConfDBInterface* myDB;
    PixDbInterface* myNewDb;

    if(argv1 == "-n"){
      // first of all the application should instanziate a data base ...
      myDB = new RootDB(argv[2],"RECREATE");
      std::cout << "Recreated RootDB file named " << argv[2] << ".root" << std::endl;
      delete myDB;
      return 0;
    }
    else if(argv1 == "-m"){
      myDB = new RootDB(argv[2],"MODULE");
      std::cout << "Recreated RootDB file named " << argv[2] << ".root from turbodaq" << std::endl;
      delete myDB;
      return 0;
    }
    else if(argv1 == "-u"){ 
      std::cout << " first of all the application should instanziate a data base ..." << std::endl;
      myDB = new RootDB(argv[2],"READ");
      DBInquire* root = myDB->readRootRecord(1); std::cout << " read the root record." << std::endl;
      recordIterator ri(root);
      std::string dbLinkString = argv[2];
      std::string link("link");
      if(dbLinkString.find_last_of("/") != std::string::npos){
	dbLinkString.insert(dbLinkString.find_last_of("/")+1, link); 
      }
      else dbLinkString = link + dbLinkString;
      std::cout << " the application now instanziate a data base of new type..." << std::endl;
      myNewDb = new RootDb(dbLinkString, "RECREATE");
      DbRecord* newRoot = myNewDb->readRootRecord(); 
      dbRecordIterator nri(newRoot);
      std::cout << " dump the new root record." << std::endl;
      std::cout << *newRoot;
      std::cout << "now dumping the iterator (pointer to) the root record; it does dump, since the pointer is loaded by readRootRecord" << std::endl;
      std::cout << **nri;
      dbFieldIterator fff(myNewDb->makeField("test field"));
      string dummyString("dummy");
      myNewDb->DbProcess(fff,PixDb::DBCOMMIT,dummyString);
      std::cout << **fff;
      dummyString = "";
      myNewDb->DbProcess(fff,PixDb::DBREAD,dummyString);
      std::cout << dummyString << std::endl;
      std::cout << "adding the field to the record" << std::endl;
      fff = myNewDb->DbProcess(newRoot->pushField(*fff),PixDb::DBREAD);
      std::cout << *fff;
      dbFieldIterator findField = newRoot->findField((*fff)->getName());
      std::cout << **(myNewDb->DbProcess(findField, PixDb::DBREAD));
      DbRecord* testRec = myNewDb->makeRecord("test", "testSubDir");
      newRoot->pushRecord(testRec);
      dbRecordIterator findRecord = newRoot->findRecord(testRec->getDecName() + "/" + testRec->getName());
      std::cout << *newRoot;
      std::cout << **(myNewDb->DbProcess(findRecord, PixDb::DBREAD));
      std::cout << "closing the new Db file" << std::endl;
      delete myNewDb;



      std::string news("New");
      std::string myNewDbName = argv[2];
      if(myNewDbName.find_last_of("/") != std::string::npos){
	myNewDbName.insert(myNewDbName.find_last_of("/")+1, news);
      }
      else myNewDbName = news + myNewDbName;

      std::cout << " the application now instanziate a data base of new type..." << std::endl;
      myNewDb = new RootDb(myNewDbName, "RECREATE");
      newRoot = myNewDb->readRootRecord(); 
      nri.pointsTo(newRoot);
      std::cout << " dump the new root record." << std::endl;
      std::cout << *newRoot;
      std::cout << "now dumping the iterator (pointer to) the root record; it does dump, since the pointer is loaded by readRootRecord" << std::endl;
      std::cout << **nri;
      fff = myNewDb->makeField("test field");
      dummyString = "dummy";
      myNewDb->DbProcess(fff,PixDb::DBCOMMIT,dummyString);
      std::cout << **fff;
      dummyString = "";
      myNewDb->DbProcess(fff,PixDb::DBREAD,dummyString);
      std::cout << dummyString << std::endl;
      std::cout << "adding the field to the record" << std::endl;
      newRoot->pushField(*fff);
      findField = newRoot->findField((*fff)->getName());
      std::cout << **(myNewDb->DbProcess(findField, PixDb::DBREAD));
      testRec = myNewDb->makeRecord("test", "testSubDir");
      newRoot->pushRecord(testRec);
      findRecord = newRoot->findRecord(testRec->getDecName() + "/" + testRec->getName());
      std::cout << *newRoot;
      std::cout << **(myNewDb->DbProcess(findRecord, PixDb::DBREAD));
      


      std::cout << "now operating on the new record based on the old root file" << std::endl;
      int time=system("ps -aux|grep -i HistTry");

	//      int i = system("ps -aux|grep -i RootDBTest"); std::cout << i << std::endl;
      recordIterator DBfLRecIt = root->recordBegin();
      for(;DBfLRecIt != root->recordEnd(); DBfLRecIt++){
	DbRecord* fLRec = myNewDb->makeRecord((*DBfLRecIt)->getName(), PixLib::getMySubDir((*DBfLRecIt)->getDecName()));
	newRoot->pushRecord(fLRec);
	//DbRecord* fLRec = newRoot->addRecord((*DBfLRecIt)->getName(), PixLib::getMySubDir((*DBfLRecIt)->getDecName()));
	dbRecordIterator fLFindRec = newRoot->findRecord(fLRec->getDecName() + "/" + fLRec->getName());
	fLFindRec = myNewDb->DbProcess(fLFindRec, PixDb::DBREAD);
	std::cout << **fLFindRec;
	std::string mDecName = (*fLFindRec)->getMyDecName();
	DbRecord* findRec = myNewDb->DbFindRecordByName(mDecName);
	std::cout << "found record" << std::endl << *findRec;
	
	std::cout << **DBfLRecIt; 
	for(fieldIterator f = (*DBfLRecIt)->fieldBegin(); f != (*DBfLRecIt)->fieldEnd(); f++){
	  std::string empty;
	  DbField* newField = myNewDb->makeField(empty);
	  copyField(dynamic_cast<RootDbField*>(newField),*f); 
	  std::cout << "adding the field to the record" << std::endl;
      	  (*fLFindRec)->pushField(newField);
	  //fLRec->pushField(newField);
	  dbFieldIterator findField = (*fLFindRec)->findField(newField->getName());
	  //std::cout << **f;
	  std::cout << **(myNewDb->DbProcess(findField, PixDb::DBREAD));
	}
	std::cout << **fLFindRec;
	for(recordIterator DBsLRecIt = (*DBfLRecIt)->recordBegin(); DBsLRecIt != (*DBfLRecIt)->recordEnd(); DBsLRecIt++){
	  DbRecord* sLRec = myNewDb->makeRecord((*DBsLRecIt)->getName(), PixLib::getMySubDir((*DBsLRecIt)->getDecName()));
	  dbRecordIterator sLFindRec = (*fLFindRec)->pushRecord(sLRec);
	  sLFindRec = myNewDb->DbProcess(sLFindRec,PixDb::DBREAD);
	  std::cout << **DBsLRecIt; 
	  for(fieldIterator f = (*DBsLRecIt)->fieldBegin(); f != (*DBsLRecIt)->fieldEnd(); f++){
	    std::string empty;
	    DbField* newField = myNewDb->makeField(empty);
	    copyField(dynamic_cast<RootDbField*>(newField),*f); 
	    (*sLFindRec)->pushField(newField);
	    dbFieldIterator findField = (*sLFindRec)->findField(newField->getName());
	    //	    std::cout << **f;
	    std::cout << **(myNewDb->DbProcess(findField, PixDb::DBREAD));
	  }
	}	
      }
    }

    std::cout << "copied two levels of cfg files in the new file New<oldfile>" << std::endl;
    system("ps -aux | grep -i RootDBTest");
    
    return 0;
  }
//       std::cout << " now testing root inquire capability." << std::endl;
//       std::cout << " dump the root record." << std::endl;
//       std::cout << *root;
//       std::cout << "trying to add, read, delete a field to/from  the root record" << std::endl;
//       std::cout << "making a field" << std::endl;
//       fieldIterator f(myDB->makeField("test field"));
//       string dummyString("dummy");
//       myDB->DBProcess(f,COMMIT,dummyString);
//       std::cout << "adding the field to the record" << std::endl;
//       root->pushField(*f);
//       std::cout << "dump again the root record with the newly added field" << std::endl;
//       std::cout << *root;
//       std::cout << "dump the original field" << std::endl;
//       std::cout << (**f);
//       std::cout << "find the added field and dump it" << std::endl;
//       fieldIterator fieldToFind = root->findField("test field");
//       if(fieldToFind != root->fieldEnd()) std::cout << (**fieldToFind); 
//       std::cout << "erasing the added field from root record and dumping it again" << std::endl;
//       root->eraseField(fieldToFind);
//       std::cout << (*root);
//       std::cout << "add a integer field to root record and changhing it 100 times" << std::endl; 
//       fieldIterator fi(myDB->makeField("test integer field"));
//       int dummyint = 0;
//       myDB->DBProcess(fi,COMMIT, dummyint);
//       root->pushField(*fi);
//       fieldToFind = root->findField("test integer field");
//       int iref = 10;
//       if(fieldToFind != root->fieldEnd()) myDB->DBProcess(fieldToFind,COMMIT, iref);
//       root->pushField(*fieldToFind);	
//       std::cout << "trying to add, read, delete an inquire" << std::endl;
//       recordIterator r(myDB->makeRecord("PixTestClass","test_add_0"));
//       std::cout << "add the new made record to root inquire" << std::endl;
//       root->pushRecord(*r);
//       std::cout << "dump again root record after having added a record" << std::endl;
//       std::cout << (*root);
//       std::cout << "get the iterator pointing to the newly added record" << std::endl;
//       recordIterator recordToFind = root->findRecord("test_add_0/PixTestClass");
//       if(recordToFind != root->recordEnd()) {std::cout << "Found linked to root inquire the new record" << std::endl << (**recordToFind);}
//       else 
// 	{std::cout << "problems: new record not found. exiting..." << std::endl; exit(1);}
//       std::cout << "push the previous field into the record. NOTE: you cannot push any field in a record if the record is not already in the database (you do not know where the field is going to be written..." << std::endl;
//       (*recordToFind)->pushField(*f);
//       recordToFind = root->findRecord("test_add_0/PixTestClass");
//       std::cout << "dump new record with a field" << std::endl;
//       std::cout << (**recordToFind);
//       std::cout << "erasing the new added record and dump again the root record" << std::endl;
//       root->eraseRecord(recordToFind);
//       std::cout << (*root);
//       delete myDB;
//       exit(0);
//       myDB = new RootDB(argv[2], "UPDATE");
//       std::cout << " get the unconnected inquire from DB by its decorated name." << std::endl;
//       std::string recordNameString();
//       DBInquire* readInquire = myDB->DBFindRecordByName("/Test_0/application;1");
//       std::cout << " dump the loaded inquire." << std::endl;
//       std::cout << *readInquire;
//       std::cout << " create histogram fields and data fields add them to the unconnected inquire." << std::endl;
//       std::cout << " create the histograms." << std::endl;
//       Histo dim1("Tmp 1 dim histogram","1 dim",10,0,10);
//       dim1.set(0,42);
//       dim1.set(9,42);			
//       Histo dim2("Tmp 2 dim histogram","2 dim",10,0,10,5,0,5);
//       dim2.set(0,0,42);
//       dim2.set(5,0,42);
//       dim2.set(0,10,42);
//       dim2.set(5,10,42);
//       Histo dim3("Tmp 2 dim histogram new","2 dim",10,0,10,5,0,5);
//       dim3.set(0,0,84);
//       dim3.set(5,0,84);
//       dim3.set(0,10,84);
//       dim3.set(5,10,84);
      
//       std::cout << " create two local variable field." << std::endl;
//       DBField* dbfield = myDB->makeField("1 dim histogram");
//       DBField* dbfield2 = myDB->makeField("2 dim histogram");
//       readInquire->pushField(dbfield); std::cout << " this add to the inquire a field which is the copy of the passed field." << std::endl;
//       readInquire->pushField(dbfield2);  
//       f.pointsTo(dbfield); std::cout << " dbfield is now pointed by f." << std::endl;
//       myDB->DBProcess(f,COMMIT,dim1); std::cout << " fill the local field with histogram ." << std::endl;
//       std::cout << **f;
//       myDB->DBProcess(readInquire->getField(1),COMMIT,dim2); std::cout << " fill the database field with 2-dim histogram." << std::endl;
//       std::cout << " create a local string field." << std::endl;
//       DBField* stringField = myDB->makeField("Histogram description");
//       string fieldValue("Dummy histograms");
//       myDB->DBProcess(stringField,COMMIT,fieldValue);
//       readInquire->pushField(stringField);
      
    
  catch(PixLib::PixDBException& exc){
    std::cout << exc << std::endl; 
    exc.what(std::cout);
    throw;
  }
  catch(SctPixelRod::BaseException& exc){
    std::cout << exc << std::endl; 
    exc.what(std::cout);
    throw;
  }
  catch(std::exception& exc){
    std::cout << "std exception caught" << std::endl;
    std::cout << exc.what() << std::endl;
    throw;
  }
  catch(...){
    std::cout << "unexpected exception" << std::endl;
    throw;
  }
  return 0;
}
//       root->pushRecord(pixmodgroup);
//       myDB->DBProcess(ri,COMMITREPLACE);
//       myDB->DBProcess(pixmodgroup,COMMIT);
//       delete pixmodgroup;
//       std::cout << " adding a second new record to the root record." << std::endl;
//       decName = root->getDecName() + "Genova Group 2/";
//       pixmodgroup = myDB->makeInquire(name, decName);
//       root->pushRecord(pixmodgroup);
//       myDB->DBProcess(ri,COMMITREPLACE);
//       myDB->DBProcess(pixmodgroup,COMMIT);
//       delete pixmodgroup;
//       pixmodgroup = *(root->findRecord("Genova Group 2/PixModuleGroup"));
//       std::cout << " dump again the root record." << std::endl;
//       std::cout << *root;
//       std::cout << " dump pixmodgroup version 2." << std::endl;
//       std::cout << *pixmodgroup;
//       std::cout << " adding a record to the new record." << std::endl;
//       decName = pixmodgroup->getDecName() + "Genova Group/";
//       DBInquire* pixmodgroup2 = myDB->makeInquire(name, decName);
//       pixmodgroup->pushRecord(pixmodgroup2);
//       myDB->DBProcess(pixmodgroup,COMMITREPLACE);
//       myDB->DBProcess(pixmodgroup2,COMMIT);
//       delete pixmodgroup2;
//       pixmodgroup2 = *(pixmodgroup->findRecord("Genova Group/PixModuleGroup")); 
//       std::cout << "at this point, there should be an new added inquire to the new inquire. Dumping it." << std::endl << *pixmodgroup;
//       std::cout << "try to find in the DB the commitreplaced pixmodgroup " << std:: endl;
//       recordIterator found = root->findRecord("Genova Group 2/PixModuleGroup");
//       if (found != root->recordEnd()){
// 	std::cout << "added pixmodgroup found: dumping it" << std::endl << **found;
//       }
//       else{
// 	std::cout << "pixmodgroup not found. Problems. Exiting..." << std::endl;
// 	exit(1);
//       }
//       std::cout << " dump the new record." << std::endl;
//       std::cout << *pixmodgroup2;
//       std::cout << " adding another record to the new record using addInquire." << std::endl;
//       pixmodgroup->addInquire("S-curve","S-Curve");
//       std::cout << " dump again the pixmodgroup record." << std::endl;
//       std::cout << *pixmodgroup;
//       std::cout << " dump the second new record." << std::endl;
//       std::cout << *pixmodgroup2;
      
//       std::cout << "now delete the record genova group/pixmodulegroup from the depending inquires" << endl;
//       recordIterator genovagroup = pixmodgroup->findRecord("Genova Group/PixModuleGroup");
//       pixmodgroup->eraseRecord(genovagroup);
      
//       std::cout << " now do it again, but in a database which have already those entries." << std::endl;
//       std::cout << "putting again pixmodgroup" << std::endl;
//       delete myDB;
      
//       int dummy;
//       std::cout << "enter a keystroke to continue" << std::endl;
//       std::cin >> dummy;			
//       myDB = new RootDB(argv[2],"UPDATE");
//       root = myDB->readRootRecord(); std::cout << " read the version 1 of root record." << std::endl;
//       ri.pointsTo(root);
//       std::cout << " now testing root inquire capability." << std::endl;
//       std::cout << " dump the root record." << std::endl;
// 			std::cout << *root;
// 			std::cout << " Explore the root record - find FE and MCC values only for 1.0 database schema." << std::endl;
// 			exploreRootRecord(root);
// 			std::cout << " adding a record to root record ." << std::endl;
// 			name = "PixModuleGroup";
// 			decName = root->getDecName() + "Genova Group 3/";
// 			pixmodgroup = myDB->makeInquire(name, decName);
// 			std::cout << *pixmodgroup;
// 			root->pushRecord(pixmodgroup);
// 			std::cout << *pixmodgroup;
// 			myDB->DBProcess(ri,COMMITREPLACE);
// 			myDB->DBProcess(pixmodgroup,COMMITREPLACE);
// 			std::cout << " dump again the root record." << std::endl;
// 			std::cout << *root;
// 			std::cout << " dump pixmodgroup version 2." << std::endl;
// 			std::cout << *pixmodgroup;
// 			std::cout << " adding a record to the new record." << std::endl;
// 			decName = pixmodgroup->getDecName() + "Genova Group/";
// 			pixmodgroup2 = myDB->makeInquire(name, decName);
// 			std::cout << *pixmodgroup2;
// 			pixmodgroup->pushRecord(pixmodgroup2);
// 			std::cout << *pixmodgroup2;
// 			myDB->DBProcess(pixmodgroup,COMMITREPLACE);
// 			std::cout << *pixmodgroup;
// 			myDB->DBProcess(pixmodgroup2,COMMITREPLACE);
// 			std::cout << " dump again the pixmodgroup record." << std::endl;
// 			std::cout << *pixmodgroup;
// 			std::cout << " dump the new record." << std::endl;
// 			std::cout << *pixmodgroup2;

// 			std::cout << " create a couple of modules." << std::endl;
// 			std::vector<std::string> vNames; vNames.push_back("ExampleConf/configs/example.cfg"); vNames.push_back("ExampleConf/configs/CTB module up B-Layer.cfg");
// 		        rootGroupModuleWizard(dynamic_cast<RootDBInquire*>(pixmodgroup), 1.0f,5.3f,vNames);
// 			std::cout << " dump the new PixModGroup." << std::endl;
// 			std::cout << *pixmodgroup;

// 			std::cout << " changing a root record field value." << std::endl;
// 			fieldIterator f;
// 			if((f = root->findField("TurboDaqDB Version")) != root->fieldEnd()){
// 				std::cout << **(root->findField("TurboDaqDB Version"));
// 				string dummy("dummy");
// 				myDB->DBProcess(f,COMMIT,dummy);
// 				std::cout << **(root->findField("TurboDaqDB Version"));
// 			}

// 			std::cout << " adding a field to the root record." << std::endl;
// 			std::cout << " instanziate and fill a field." << std::endl;
// 			DBField* field = (*f)->getDB()->makeField("RootDB Version");
// 			f.pointsTo(field);
// 			float value(0.1f);
// 			myDB->DBProcess(f,COMMIT,value); std::cout << " this COMMIT do not change the permanent storage." << std::endl;
// 			root->pushField(*f); std::cout << " add the field to the root record." << std::endl;
// 			value = 0.42f;
// 			myDB->DBProcess(f,COMMIT,value);
// 			root->pushField(*f); std::cout << " pushing same field twice do not have any effect." << std::endl;
// 			std::cout << *root;
// 			std::cout << **(root->findField("RootDB Version"));
// 			recordIterator iiter(root);
// 			myDB->DBProcess(iiter,COMMITREPLACE); std::cout << " this COMMIT change the permanent storage. COMMITREPLACE means that the old version is overwritten." << std::endl;

// 			delete myDB; std::cout << " deleting the database means also deleting all fields and record loaded in memory - and make unvalid ALL iterators around." << std::endl;
// 			std::cout << " explore the modified database." << std::endl;
// 			myDB = new RootDB(argv[2], "UPDATE");
// 			root = myDB->readRootRecord(); std::cout << " read the version 1 of root record." << std::endl;	  
// 			if((f = root->findField("RootDB Version")) != root->fieldEnd()){
// 				std::cout << **(root->findField("RootDB Version"));
// 			}
// 			else{
// 				std::cout << "Warning! Unable to read 'RootDB Version' field from root record... check database integrity" << std::endl;
// 			}
// 			if((f = root->findField("TurboDaqDBVersion")) != root->fieldEnd()){
// 				std::cout << **(root->findField("TurboDaqDB Version"));
// 			}
// 			std::cout << " add an inquire non connectd to any other inquire." << std::endl;
// 			DBInquire* inquire = myDB->makeInquire("PixScanConfig","");
// 			string path = root->getDecName();
// 			string scanDecName = "Scan Burn-in/";
			
// 			inquire->getDecName() = path + scanDecName;
// 			inquire->getMyDecName() = inquire->getDecName();
// 			inquire->getMyDecName() +=inquire->getName();
			
// 			myDB->DBProcess(inquire,COMMIT);
			
// 			std::cout << " change memory content of the new inquire." << std::endl;
// 			inquire->pushField(field);

// 			std::cout << " save again root inquire and all connected inquires." << std::endl;
// 			recordIterator p(root);
// 			myDB->DBProcess(p,COMMITREPLACETREE);
			
// 			std::cout << " close DB, reopen it and check that the inquire not connected does not have the field on it." << std::endl;
// 			delete myDB;
// 			exit(0);
// 			myDB = new RootDB(argv[2], "UPDATE");
// 			std::cout << " get the unconnected inquire from DB by its decorated name." << std::endl;
// 			DBInquire* readInquire = myDB->DBFindRecordByName(BYDECNAME,"/Scan Burn-in/PixScanConfig;1")[0];
// 			std::cout << " dump the loaded inquire." << std::endl;
// 			std::cout << *readInquire;
// 			std::cout << " create histogram fields and data fields add them to the unconnected inquire." << std::endl;
// 			std::cout << " create the histograms." << std::endl;
// 			Histo dim1("Tmp 1 dim histogram","1 dim",10,0,10);
// 			dim1.set(0,42);
// 			dim1.set(9,42);			
// 			Histo dim2("Tmp 2 dim histogram","2 dim",10,0,10,5,0,5);
// 			dim2.set(0,0,42);
// 			dim2.set(5,0,42);
// 			dim2.set(0,10,42);
// 			dim2.set(5,10,42);
// 			Histo dim3("Tmp 2 dim histogram new","2 dim",10,0,10,5,0,5);
// 			dim3.set(0,0,84);
// 			dim3.set(5,0,84);
// 			dim3.set(0,10,84);
// 			dim3.set(5,10,84);
			
// 			std::cout << " create two local variable field." << std::endl;
// 			DBField* dbfield = myDB->makeField("1 dim histogram");
// 			DBField* dbfield2 = myDB->makeField("2 dim histogram");
// 			readInquire->pushField(dbfield); std::cout << " this add to the inquire a field which is the copy of the passed field." << std::endl;
// 			readInquire->pushField(dbfield2);  
// 			f.pointsTo(dbfield); std::cout << " dbfield is now pointed by f." << std::endl;
// 			myDB->DBProcess(f,COMMIT,dim1); std::cout << " fill the local field with histogram ." << std::endl;
// 			std::cout << **f;
// 			myDB->DBProcess(readInquire->getField(1),COMMIT,dim2); std::cout << " fill the database field with 2-dim histogram." << std::endl;
// 			std::cout << " create a local string field." << std::endl;
// 			DBField* stringField = myDB->makeField("Histogram description");
// 			string fieldValue("Dummy histograms");
// 			myDB->DBProcess(stringField,COMMIT,fieldValue);
// 			readInquire->pushField(stringField);
// 			myDB->DBProcess(readInquire,COMMITREPLACE); std::cout << " save the inquire with the three new fields - one still empty, the other two loaded with histogram and string." << std::endl;
// 			delete myDB;

// 			std::cout << " reopen the database and dump the histograms." << std::endl;
// 			myDB = new RootDB(argv[2], "UPDATE");			
// 			readInquire = myDB->DBFindRecordByName(BYDECNAME,"/Scan Burn-in/PixScanConfig;1")[0];
// 			Histo readHisto;
// 			f = readInquire->findField("1 dim histogram");
// 			std::cout << **f; std::cout << " this field has been saved as empty, so it is." << std::endl;
// 			myDB->DBProcess(f,COMMIT,dim1); std::cout << " fill the database field with histogram ." << std::endl;
// 			std::cout << **f; std::cout << " now the field is full." << std::endl;
// 			std::cout << "enter a keystroke to continue" << std::endl;
// 			std::cin >> dummy;
// 			f = readInquire->findField("2 dim histogram");
// 			myDB->DBProcess(f,COMMIT,dim3);
// 			std::cout << **f;
// 			std::cout << "enter a keystroke to continue" << std::endl;
// 			std::cin >> dummy;
// 			myDB->DBProcess(readInquire,COMMITREPLACE); std::cout << " save the inquire with the fields loaded with histograms." << std::endl;
// 			delete myDB;
// 			return 0;
// 	  }	  
//   } 
//   catch(SctPixelRod::BaseException& exc){
// 	  std::cout << exc << std::endl; 
// 	  exit(1);
//   }
//   catch(...){
// 	  std::cout << "unexpected exception" << std::endl;
// 	  throw;
//   }
  
//   return 0;
// }
