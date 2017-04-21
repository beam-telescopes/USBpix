#include "PixConfDBInterface/TurboDaqDB.h"
#include "PixConfDBInterface/RootDB.h"
#include <iostream>

void printHelp(int argc, char* argv[]){
	std::cout << "ModuleGroup_Wizard -n <Cfg file>" << std::endl;
	std::cout << "ModuleGroup_Wizard -a <Root file> <Root Inquire Name> <ModuleGroup name> <TurboDaq .cfg file> <TurboDaq .cfg file> ..." << std::endl;
	std::cout << "supplied arguments were: " << std::endl;
	for(int i = 0; i < argc; i++){
	  std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
	}
}

int main(int argc, char* argv[]){

	RootDB* myDB;

	if(argc < 2){
		printHelp(argc,argv);
		exit(1);
  }
  string sw(argv[1]);
  if (sw == "-n" && argc == 3) {
    myDB = new RootDB(argv[2],"RECREATE");
	  std::cout << "Recreated RootDB file named " << argv[2] << ".root" << std::endl;
	  delete myDB;
		return 0;
  } else if (sw == "-a" && argc >= 6) {
    string argv1(argv[2]);
    string argv2(argv[3]);
    try{
      // first of all the application should instanziate a data base ...
      myDB = new RootDB(argv1,"UPDATE");
      // Suppose now you have a root file, you can instantiate a data base from the file
      DBInquire* root = myDB->DBFindRecordByName(BYDECNAME, argv2)[0]; 
      string name("PixModuleGroup");
      string decName(root->getDecName() + argv[4]);
      if(decName.at(decName.length()-1) != '/') decName += "/";
      string myDecName(decName); myDecName += "PixModuleGroup;1";
      RootDBInquire* pixmodgroup = new RootDBInquire(name, decName, myDecName,myDB);
      // deal with an existing pixmodule group of the same decorated name]
      string oldInquName = argv[4];
      if(oldInquName.at(oldInquName.length()-1) != '/') oldInquName += "/";
      oldInquName += "PixModuleGroup;1";
      recordIterator oldInqu;
      if((oldInqu = root->findRecord(oldInquName)) != root->recordEnd()){
        delete pixmodgroup;
        pixmodgroup = dynamic_cast<RootDBInquire*>(*oldInqu);
      }
      else{
        root->pushRecord(pixmodgroup);
      }
      std::cout << *root;

      fieldIterator oldFie;
      if((oldFie = pixmodgroup->findField("ModuleGroupName")) == pixmodgroup->fieldEnd()){
        RootDBField field("ModuleGroupName",myDB);
        fieldIterator f(&field);
        string value(argv[4]);
        myDB->DBProcess(f,COMMIT,value); // this COMMIT do not change the permanent storage - rootGroupModuleWizard later will do it
        pixmodgroup->pushField(*f); // add the field to the root record
      }
      else{
        string value(argv[4]);
        myDB->DBProcess(oldFie,COMMIT,value); // this COMMIT do not change the permanent storage - rootGroupModuleWizard later will do it
      }
      recordIterator ri(root);
      myDB->DBProcess(ri,COMMITREPLACE);
      //		recordIterator pi(0,pixmodgroup,0);
      //		myDB->DBProcess(pi,COMMIT);
      std::vector<std::string> vNames; 
      for(int i = 5; i < argc; i++) vNames.push_back(argv[i]); 
      rootGroupModuleWizard(pixmodgroup, 1.0f,5.3f,vNames);
      delete myDB;
    }
    catch(SctPixelRod::BaseException& exc){
      std::cout << exc << std::endl; 
      delete myDB;
      exit(1);
    }
    catch(...){
      std::cout << "unexpected exception" << std::endl;
      delete myDB;
      throw;
    }
    return 0;
  } else {
		printHelp(argc,argv);
		exit(1);   
  }
}
