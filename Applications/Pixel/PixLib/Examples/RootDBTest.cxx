#include "TApplication.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "Histo/Histo.h"
#include "BaseException.h"


#include <iostream>
#include <sstream>
// example of application using the PixConfDBInterface to get informations from the Data Base

using namespace PixLib;
using std::string;

// string purgeSlash(string in){
// 	string retval(in);
// 	if(in.at(in.size()-1) == '/'){
// 		retval =  in.substr(0,in.size()-1);
// 	}
// 	return retval;
// }

void exploreRootRecord(DBInquire* root){
  recordIterator it;
  for(it = root->recordBegin(); it != root->recordEnd(); it++){
    std::cout << "read record is "  << (*it)->getName() << std::endl;
  }
  for(it = root->recordBegin(); it != root->recordEnd(); it++)
    { 
      for(recordIterator iit = (*it)->recordBegin(); iit != (*it)->recordEnd(); iit++){
	for(recordIterator iiit = (*iit)->recordBegin(); iiit != (*iit)->recordEnd(); iiit++){						
	  std::cout << (*iiit)->getName().c_str() << std::endl;
	  for(recordIterator iiiit = (*iiit)->recordBegin(); iiiit != (*iiit)->recordEnd(); iiiit++){
	    if((*iiiit)->getName() == "PixFe") {
								for(recordIterator feit = (*iiiit)->recordBegin(); feit != (*iiiit)->recordEnd(); feit++){
									vector<int> fdac;
									fieldIterator f((*feit)->findField("Trim_FDAC"));
									if (f != (*feit)->fieldEnd()){
										root->getDB()->DBProcess(f,READ, fdac);
										std::cout << "fdac[0]" << fdac[0] << '\t' << "fdac[2879]" << fdac[2879] << std::endl;
									}
									vector<int> tdac;
									f = (*feit)->findField("Trim_TDAC");
									if (f != (*feit)->fieldEnd()){
										root->getDB()->DBProcess(f,READ, tdac);
										std::cout << "tdac[0]" << tdac[0] << '\t' << "tdac[2879]" << tdac[2879] << std::endl;
									}
								}
								fieldIterator fef((*iiiit)->findField("Misc_Address"));
								if (fef != (*iiiit)->fieldEnd()){
									int ga;
									root->getDB()->DBProcess(fef,READ, ga);
									std::cout << "Geographical address" << ga <<  std::endl;
								}
							}
							if((*iiiit)->getName() == "PixMcc") {
								std::cout << **iiiit;
							}
						}
					}
				}
			}
};

int main(int argc, char* argv[]){
	
	TApplication app("MyApp",0,0); // this is to allow pop up of graphical windows in root -> see root documentation
	
	if(argc != 3){
		std::cout << "Wrong program arguments. Usage:" << std::endl;
		std::cout << "RootDBTest -m <.cfg file> to create a new root file from a turbodaq file" << std::endl;
		std::cout << "RootDBTest -c <.cfg file> to create a new root file from .cfg" << std::endl;
		std::cout << "RootDBTest -n <.root file> create an empty RootDB file" << std::endl;
		std::cout << "RootDBTest -u <.root file> to operate on RootDB file" << std::endl;
		return 1;
	}
	try{
		string argv1(argv[1]);
		PixConfDBInterface* myDB;

		if(argv1 == "-c"){
			// first of all the application should instanziate a data base ...
			myDB = new RootDB(argv[2],"RECREATE");
			std::cout << "Recreated RootDB file named " << argv[2] << ".root" << std::endl;
			delete myDB;
			return 0;
		}
		if(argv1 == "-n"){
		  // first of all the application should instanziate a data base ...
		  try{
			myDB = new RootDB(argv[2],"NEW");
		  }catch(PixDBException &dbexc){
		    std::stringstream exmsg;
		    dbexc.what(exmsg);
		    std::cerr << "PixDB-exception: \"" <<exmsg.str() << "\" while creating file " << argv[2] << std::endl;
		    return -4;
		  }catch(SctPixelRod::BaseException & exc){
		    std::cerr <<"Base-exception: \""<<exc<<"\" while creating file " << argv[2] << std::endl;
		    return -3;
		  } catch(std::exception& s){
		    std::cerr << "Std-lib exception \""<< s.what()<<"\" while creating file " << argv[2] << std::endl;
		    return -2;
		  }catch(...){
		    std::cerr << "Unknown exception while creating file " << argv[2] << std::endl;
		    return -1;
		  }
		  std::cout << "Created RootDB file named " << argv[2] << std::endl;
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
			myDB = new RootDB(argv[2],"UPDATE");
			DBInquire* root = myDB->readRootRecord(1); std::cout << " read the version 1 of root record." << std::endl;
			recordIterator ri(root);
			
std::cout << " now testing root inquire capability." << std::endl;
			std::cout << " dump the root record." << std::endl;
			std::cout << *root;
			std::cout << " Explore the root record - find FE and MCC values only for 1.0 database schema." << std::endl;
			exploreRootRecord(root);
			std::cout << " adding a record to root record ." << std::endl;
			string name("PixModuleGroup");
			string decName(root->getDecName() + "Genova Group/");
			DBInquire* pixmodgroup = myDB->makeInquire(name, decName);
			root->pushRecord(pixmodgroup);
			myDB->DBProcess(ri,COMMITREPLACE);
			myDB->DBProcess(pixmodgroup,COMMIT);
			delete pixmodgroup;
			std::cout << " adding a second new record to the root record." << std::endl;
			decName = root->getDecName() + "Genova Group 2/";
			pixmodgroup = myDB->makeInquire(name, decName);
			root->pushRecord(pixmodgroup);
			myDB->DBProcess(ri,COMMITREPLACE);
			myDB->DBProcess(pixmodgroup,COMMIT);
			delete pixmodgroup;
			pixmodgroup = *(root->findRecord("Genova Group 2/PixModuleGroup"));
			std::cout << " dump again the root record." << std::endl;
			std::cout << *root;
			std::cout << " dump pixmodgroup version 2." << std::endl;
			std::cout << *pixmodgroup;
			std::cout << " adding a record to the new record." << std::endl;
			decName = pixmodgroup->getDecName() + "Genova Group/";
			DBInquire* pixmodgroup2 = myDB->makeInquire(name, decName);
			pixmodgroup->pushRecord(pixmodgroup2);
			myDB->DBProcess(pixmodgroup,COMMITREPLACE);
			myDB->DBProcess(pixmodgroup2,COMMIT);
			delete pixmodgroup2;
			pixmodgroup2 = *(pixmodgroup->findRecord("Genova Group/PixModuleGroup")); 
			std::cout << "at this point, there should be an new added inquire to the new inquire. Dumping it." << std::endl << *pixmodgroup;
			std::cout << "try to find in the DB the commitreplaced pixmodgroup " << std:: endl;
			recordIterator found = root->findRecord("Genova Group 2/PixModuleGroup");
			if (found != root->recordEnd()){
			  std::cout << "added pixmodgroup found: dumping it" << std::endl << **found;
			}
			else{
			  std::cout << "pixmodgroup not found. Problems. Exiting..." << std::endl;
			  exit(1);
			}
			std::cout << " dump the new record." << std::endl;
			std::cout << *pixmodgroup2;
			std::cout << " adding another record to the new record using addInquire." << std::endl;
			pixmodgroup->addInquire("S-curve","S-Curve");
			std::cout << " dump again the pixmodgroup record." << std::endl;
			std::cout << *pixmodgroup;
			std::cout << " dump the second new record." << std::endl;
			std::cout << *pixmodgroup2;

			std::cout << "now delete the record genova group/pixmodulegroup from the depending inquires" << endl;
			recordIterator genovagroup = pixmodgroup->findRecord("Genova Group/PixModuleGroup");
			pixmodgroup->eraseRecord(genovagroup);

			std::cout << " now do it again, but in a database which have already those entries." << std::endl;
			std::cout << "putting again pixmodgroup" << std::endl;
			delete myDB;

			int dummy;
//			std::cout << "enter a keystroke to continue" << std::endl;
//			std::cin >> dummy;			
			myDB = new RootDB(argv[2],"UPDATE");
			root = myDB->readRootRecord(1); std::cout << " read the version 1 of root record." << std::endl;
			ri.pointsTo(root);
			std::cout << " now testing root inquire capability." << std::endl;
			std::cout << " dump the root record." << std::endl;
			std::cout << *root;
			std::cout << " Explore the root record - find FE and MCC values only for 1.0 database schema." << std::endl;
			exploreRootRecord(root);
			std::cout << " adding a record to root record ." << std::endl;
			name = "PixModuleGroup";
			decName = root->getDecName() + "Genova Group 3/";
			pixmodgroup = myDB->makeInquire(name, decName);
			std::cout << *pixmodgroup;
			root->pushRecord(pixmodgroup);
			std::cout << *pixmodgroup;
			myDB->DBProcess(ri,COMMITREPLACE);
			myDB->DBProcess(pixmodgroup,COMMITREPLACE);
			std::cout << " dump again the root record." << std::endl;
			std::cout << *root;
			std::cout << " dump pixmodgroup version 2." << std::endl;
			std::cout << *pixmodgroup;
			std::cout << " adding a record to the new record." << std::endl;
			decName = pixmodgroup->getDecName() + "Genova Group/";
			pixmodgroup2 = myDB->makeInquire(name, decName);
			std::cout << *pixmodgroup2;
			pixmodgroup->pushRecord(pixmodgroup2);
			std::cout << *pixmodgroup2;
			myDB->DBProcess(pixmodgroup,COMMITREPLACE);
			std::cout << *pixmodgroup;
			myDB->DBProcess(pixmodgroup2,COMMITREPLACE);
			std::cout << " dump again the pixmodgroup record." << std::endl;
			std::cout << *pixmodgroup;
			std::cout << " dump the new record." << std::endl;
			std::cout << *pixmodgroup2;

			std::cout << " create a couple of modules." << std::endl;
			std::vector<std::string> vNames; vNames.push_back("ExampleConf/configs/example.cfg"); vNames.push_back("ExampleConf/configs/CTB module up B-Layer.cfg");
		        rootGroupModuleWizard(dynamic_cast<RootDBInquire*>(pixmodgroup), 1.0f,5.3f,vNames);
			std::cout << " dump the new PixModGroup." << std::endl;
			std::cout << *pixmodgroup;

			std::cout << " changing a root record field value." << std::endl;
			fieldIterator f;
			if((f = root->findField("TurboDaqDB Version")) != root->fieldEnd()){
				std::cout << **(root->findField("TurboDaqDB Version"));
				string dummy("dummy");
				myDB->DBProcess(f,COMMIT,dummy);
				std::cout << **(root->findField("TurboDaqDB Version"));
			}

			std::cout << " adding a field to the root record." << std::endl;
			std::cout << " instanziate and fill a field." << std::endl;
			DBField* field = (*f)->getDB()->makeField("RootDB Version");
			f.pointsTo(field);
			float value(0.1f);
			myDB->DBProcess(f,COMMIT,value); std::cout << " this COMMIT do not change the permanent storage." << std::endl;
			root->pushField(*f); std::cout << " add the field to the root record." << std::endl;
			value = 0.42f;
			myDB->DBProcess(f,COMMIT,value);
			root->pushField(*f); std::cout << " pushing same field twice do not have any effect." << std::endl;
			std::cout << *root;
			std::cout << **(root->findField("RootDB Version"));
			recordIterator iiter(root);
			myDB->DBProcess(iiter,COMMITREPLACE); std::cout << " this COMMIT change the permanent storage. COMMITREPLACE means that the old version is overwritten." << std::endl;

			delete myDB; std::cout << " deleting the database means also deleting all fields and record loaded in memory - and make unvalid ALL iterators around." << std::endl;
			std::cout << " explore the modified database." << std::endl;
			myDB = new RootDB(argv[2], "UPDATE");
			root = myDB->readRootRecord(1); std::cout << " read the version 1 of root record." << std::endl;	  
			if((f = root->findField("RootDB Version")) != root->fieldEnd()){
				std::cout << **(root->findField("RootDB Version"));
			}
			else{
				std::cout << "Warning! Unable to read 'RootDB Version' field from root record... check database integrity" << std::endl;
			}
			if((f = root->findField("TurboDaqDBVersion")) != root->fieldEnd()){
				std::cout << **(root->findField("TurboDaqDB Version"));
			}
			std::cout << " add an inquire non connectd to any other inquire." << std::endl;
			DBInquire* inquire = myDB->makeInquire("PixScanConfig","");
			string path = root->getDecName();
			string scanDecName = "Scan Burn-in/";
			
			inquire->getDecName() = path + scanDecName;
			inquire->getMyDecName() = inquire->getDecName();
			inquire->getMyDecName() +=inquire->getName();
			
			myDB->DBProcess(inquire,COMMIT);
			
			std::cout << " change memory content of the new inquire." << std::endl;
			inquire->pushField(field);

			std::cout << " save again root inquire and all connected inquires." << std::endl;
			recordIterator p(root);
			myDB->DBProcess(p,COMMITREPLACETREE);
			
			std::cout << " close DB, reopen it and check that the inquire not connected does not have the field on it." << std::endl;
			delete myDB;
			exit(0);
			myDB = new RootDB(argv[2], "UPDATE");
			std::cout << " get the unconnected inquire from DB by its decorated name." << std::endl;
			DBInquire* readInquire = myDB->DBFindRecordByName(BYDECNAME,"/Scan Burn-in/PixScanConfig;1")[0];
			std::cout << " dump the loaded inquire." << std::endl;
			std::cout << *readInquire;
			std::cout << " create histogram fields and data fields add them to the unconnected inquire." << std::endl;
			std::cout << " create the histograms." << std::endl;
			Histo dim1("Tmp 1 dim histogram","1 dim",10,0,10);
			dim1.set(0,42);
			dim1.set(9,42);			
			Histo dim2("Tmp 2 dim histogram","2 dim",10,0,10,5,0,5);
			dim2.set(0,0,42);
			dim2.set(5,0,42);
			dim2.set(0,10,42);
			dim2.set(5,10,42);
			Histo dim3("Tmp 2 dim histogram new","2 dim",10,0,10,5,0,5);
			dim3.set(0,0,84);
			dim3.set(5,0,84);
			dim3.set(0,10,84);
			dim3.set(5,10,84);
			
			std::cout << " create two local variable field." << std::endl;
			DBField* dbfield = myDB->makeField("1 dim histogram");
			DBField* dbfield2 = myDB->makeField("2 dim histogram");
			readInquire->pushField(dbfield); std::cout << " this add to the inquire a field which is the copy of the passed field." << std::endl;
			readInquire->pushField(dbfield2);  
			f.pointsTo(dbfield); std::cout << " dbfield is now pointed by f." << std::endl;
			myDB->DBProcess(f,COMMIT,dim1); std::cout << " fill the local field with histogram ." << std::endl;
			std::cout << **f;
			myDB->DBProcess(readInquire->getField(1),COMMIT,dim2); std::cout << " fill the database field with 2-dim histogram." << std::endl;
			std::cout << " create a local string field." << std::endl;
			DBField* stringField = myDB->makeField("Histogram description");
			string fieldValue("Dummy histograms");
			myDB->DBProcess(stringField,COMMIT,fieldValue);
			readInquire->pushField(stringField);
			myDB->DBProcess(readInquire,COMMITREPLACE); std::cout << " save the inquire with the three new fields - one still empty, the other two loaded with histogram and string." << std::endl;
			delete myDB;

			std::cout << " reopen the database and dump the histograms." << std::endl;
			myDB = new RootDB(argv[2], "UPDATE");			
			readInquire = myDB->DBFindRecordByName(BYDECNAME,"/Scan Burn-in/PixScanConfig;1")[0];
			Histo readHisto;
			f = readInquire->findField("1 dim histogram");
			std::cout << **f; std::cout << " this field has been saved as empty, so it is." << std::endl;
			myDB->DBProcess(f,COMMIT,dim1); std::cout << " fill the database field with histogram ." << std::endl;
			std::cout << **f; std::cout << " now the field is full." << std::endl;
			std::cout << "enter a keystroke to continue" << std::endl;
			std::cin >> dummy;
			f = readInquire->findField("2 dim histogram");
			myDB->DBProcess(f,COMMIT,dim3);
			std::cout << **f;
			std::cout << "enter a keystroke to continue" << std::endl;
			std::cin >> dummy;
			myDB->DBProcess(readInquire,COMMITREPLACE); std::cout << " save the inquire with the fields loaded with histograms." << std::endl;
			delete myDB;


			std::cout << " reopen the database and save a lot of histograms." << std::endl;
			myDB = new RootDB(argv[2], "UPDATE");			
			Histo writingHisto("big empty histo","bighisto", 320,0,320,144,0,144);
			
			for(int cicle = 0; cicle < 3000; cicle ++){
			  readInquire = myDB->DBFindRecordByName(BYDECNAME,"/Scan Burn-in/PixScanConfig;1")[0];
			  std::stringstream title;
			  title << "1 dim histogram" << cicle;
			  DBField* tmpdbfield = myDB->makeField(title.str());
			  std::cout << "saving " << cicle << " histogram" << std::endl;
			  readInquire->pushField(tmpdbfield);
			  f = readInquire->findField(title.str());
			  writingHisto.set(0,0,cicle);
			  myDB->DBProcess(f,COMMIT,writingHisto);
			  myDB->DBProcess(readInquire,COMMITREPLACE);
			  delete tmpdbfield;
			}
			delete myDB;

			return 0;
	  }	  
  } 
  catch(SctPixelRod::BaseException& exc){
	  std::cout << exc << std::endl; 
	  exit(1);
  }
  catch(...){
	  std::cout << "unexpected exception" << std::endl;
	  throw;
  }
  
  return 0;
}
