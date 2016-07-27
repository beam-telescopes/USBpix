#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixConfDBInterface/GeneralDBfunctions.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>

// application using the PixConfDBInterface to merge two scan files

using namespace PixLib;

void printUsage(){
  std::cout << "Wrong program arguments. Usage:" << std::endl;
  std::cout << "MergeScans <new file> <old file 1> (<old file 2> ...)" << std::endl;
}

int main(int argc, char* argv[]){
  std::vector<std::string> inputNames;
  std::string outputName="";

  if(argc <3){
    printUsage();
    return 1;
  }

  for(int i = 1; i < argc; i++){    
    if (strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-h") == 0){
      printUsage();
      return 2;
    }else{
      if(outputName.empty())
	outputName = argv[i];
      else
	inputNames.push_back(argv[i]);
    }
  }

  PixConfDBInterface *outDB, *inDB;

  std::map<std::string, int > scanNames;
  int scanId=0;
  // first, store scan names to check for overlap later - will not be possible!
  for(std::vector<std::string>::iterator it=inputNames.begin(); it!=inputNames.end();it++){
    inDB = new RootDB(it->c_str(),"READ");
    DBInquire* root = inDB->readRootRecord(1);
    for(recordIterator scIter = root->recordBegin();scIter!=root->recordEnd();scIter++){
      std::string decName = (*scIter)->getDecName();
      getDecNameCore(decName);
      // keep ID only of first appearance, others will have to be re-named
      if(scanNames.find(decName)==scanNames.end()) scanNames[decName] = scanId;
      scanId++;
    }
    delete inDB;
  }
  
  std::cout << "Writing into " << outputName << std::endl;

  // then, open new file and copy-paste DBinquires from all input files
  FILE *testF = fopen(outputName.c_str(),"r");
  if(testF!=0){
    fclose(testF);
    std::cout << "Output file " << outputName << " already exists, replace? (y|n)";
    std::string answer;
    std::cin >> answer;
    std::cin.clear();
    std::cin.ignore(10000,'\n');
    if(answer=="y") remove(outputName.c_str());
    else return -1;
  }
  outDB = new RootDB(outputName.c_str(), "NEW");
  DBInquire* outRoot = outDB->readRootRecord(1);

  scanId=0;
  for(std::vector<std::string>::iterator it=inputNames.begin(); it!=inputNames.end();it++){
    inDB = new RootDB(it->c_str(),"READ");
    DBInquire* root = inDB->readRootRecord(1);
    for(recordIterator scIter = root->recordBegin();scIter!=root->recordEnd();scIter++){
      std::string decName = (*scIter)->getDecName();
      getDecNameCore(decName);
      if(scanNames[decName]!=scanId){ // duplicated scan name, fix
	std::string newName=decName;
	while(scanNames[newName]!=0 && newName.length()>0){
	  char str[2001];
	  std::cout << "Item name " << newName << " already exists, please enter a new name: ";
	  std::cin.get(str,2000);
	  std::cin.clear();
	  std::cin.ignore(10000,'\n');
	  newName = str;
	}
	std::cout << "Copying file " << it->c_str() << " item " << decName << " re-named to " << newName << std::endl;
	insertDBInquire((*scIter), outRoot, true, newName.c_str());
      } else{
	std::cout << "Copying file " << it->c_str() << " item " << decName << std::endl;
	insertDBInquire((*scIter), outRoot, true);
      }
      scanId++;
    }
    delete inDB;
  }
    
  delete outDB;

  return 0;
}
