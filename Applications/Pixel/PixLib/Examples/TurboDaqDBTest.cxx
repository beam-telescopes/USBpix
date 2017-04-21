#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/TurboDaqDB.h"

#include <iostream>


// example of application using the PixConfDBInterface to get informations from the Data Base

using namespace PixLib;


int main(int argc, char* argv[]){

  if(argc != 2){
    cout << "Usage: TurboDaqDBTest <configuration file>" << endl;
    return 1;
  }
  try{
    // first of all the application should instanziate a data base ...
    PixConfDBInterface* myDB = new TurboDaqDB(argv[1]);	
    // second: get the root node of the configuration tree
    DBInquire* root = myDB->readRootRecord(1);
    cout << (*root);
    // third: access to the datafield of root record
    {for(fieldIterator ft = root->fieldBegin(); ft != root->fieldEnd(); ft++)
      {
	string name;
	myDB->DBProcess(ft, READ, name);
	std::cout << "read name is " << name << std::endl;
      }
    }
    // 4th: access to the subrecord of root record and their datafield
    {for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++)
      { 
	//cout << **it;
	for(recordIterator iit = (*it)->recordBegin(); iit != (*it)->recordEnd(); iit++){
	  //cout << **iit;	  
	  for(recordIterator iiit = (*iit)->recordBegin(); iiit != (*iit)->recordEnd(); iiit++){
	    //cout << **iiit;
	    cout << (*iiit)->getName().c_str() << endl;
	    for(recordIterator iiiit = (*iiit)->recordBegin(); iiiit != (*iiit)->recordEnd(); iiiit++){
	      //cout << (*iiiit)->getName() << endl;
	      if((*iiiit)->getName() == "PixFe") {
		vector<int> fdac;
		myDB->DBProcess((*iiiit)->findField("FDAC"),READ, fdac);
		cout << "fdac[0]" << fdac[0] << '\t' << "fdac[2879]" << fdac[2879] << endl;
		vector<int> tdac;
		myDB->DBProcess((*iiiit)->findField("TDAC"),READ, tdac);
		cout << "tdac[0]" << tdac[0] << '\t' << "tdac[2879]" << tdac[2879] << endl;
		int ga;
		myDB->DBProcess((*iiiit)->findField("geographical address"),READ, ga);
		cout << "Geographical address" << ga << '\t' << "tdac[2879]" << fdac[2879] << endl;
	      }
	      if((*iiiit)->getName() == "PixMcc") {
		cout << **iiiit;
	      }
	    }
		  if((*iiit)->findRecord("PixMcc") != (*iiit)->recordEnd()) cout << **((*iiit)->findRecord("PixMcc"));
		  if((*iiit)->findRecord("PixMcc_0/PixMcc") != (*iiit)->recordEnd()) cout << **((*iiit)->findRecord("PixMcc"));
	  }
	}
      }
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
