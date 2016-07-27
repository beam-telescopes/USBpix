////////////////////////////////////////////////////////////////////
// RootDb.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 23/06/03  Version 1.0 (GG)
//           Initial release
//! Data Base implementation of RootDb.h

#include "PixDbInterface/RootDb.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "PixConfDBInterface/RootDB.h"
#include "Histo/Histo.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
using std::stringstream;
using namespace PixLib;
typedef unsigned int uint;

ClassImp(RootDbField)
ClassImp(RootDbRecord)
ClassImp(RootDb)

namespace{

  std::vector<std::string> pathItemize(std::string& inputString){
    // utility function to itemize a root path - used to create the directory structure in the disk if it is not there
    // a path like file:/dir/dir/dir/dir/dir is itemized in a vector containing the "dir"
    
    std::vector<std::string> retval;
    int runnpos = 0;
    runnpos = (int)inputString.find_first_of(':'); // skip the long file name 
    if(runnpos == (int)std::string::npos) runnpos = 0; // in the case there are not ':', this returns to the previous situation
    else inputString.erase(0,runnpos+1);
    runnpos = (int)inputString.find_first_of('/');
    if(runnpos == (int)std::string::npos) return retval; 
    runnpos += 1;
    while(inputString.find_first_of('/',runnpos) != std::string::npos){
      std::string item = inputString.substr(runnpos,inputString.find_first_of('/',runnpos+1)-runnpos);
      retval.push_back(item);
      runnpos = inputString.find_first_of('/',runnpos) + 1;
    }
    if(runnpos != (int)inputString.size()){
      retval.push_back(inputString.substr(runnpos, inputString.size()));
    }
    return retval;
  }

//   string getAbsolutePath(TDirectory* currDir){ // from a root directory file:/dir/dir/dir to a string /dir/dir/dir
//     string path = currDir->GetPath();
//     int colonPos = path.find(":");
//     path.erase(0,colonPos+1);
//     return path;
//   }

  string getFileName(string in){
    string retval = in;
    int colonPos = retval.find(":");
    retval.erase(colonPos, retval.size());
    return retval;
  }

  static string purgeSlash(string in){
    // from file:/dir/dir/dir/dir/dir/ to file:/dir/dir/dir/dir/dir
    // from / to nothing
    string retval(in);
    if(retval.size() != 0){
      if(in.at(in.size()-1) == '/'){
	retval =  in.substr(0,in.size()-1);
      }
    }
    return retval;
  }


//   string getMyVersionName(string name){
//     // from file:/dir/dir/dir/dir/name;1 to name;1
//     int pos = (int)name.find_last_of("/");
//     return name.substr(pos+1);
//   }

  string getMyDirectoryName(string name){
    // from file:/dir/dir/dir/dir/dir/name;1 to file:/dir/dir/dir/dir/dir
    int pos = (int)name.find_last_of("/");
    return name.substr(0,pos);
  }

  string getSubRecordNameFromList(string listItem){
    // from file:/dir/dir/dir/mydir/subdir/name to subdir/name
    // from file:/subdir/name to subdir/name
    string retval;
    int pos = listItem.find_last_of("/");
    pos--;
    pos = listItem.find_last_of("/",pos);
    retval = listItem.substr(pos+1, listItem.size());
    return retval;
  }

  string getFieldNameFromList(string listItem){
    // from file:/dir/dir/dir/mydir/subdir/name to name
    // from file:/subdir/name to name
    string retval;
    int pos = listItem.find_last_of("/");
    if(pos != (int)std::string::npos){
      retval = listItem.substr(pos+1, listItem.size());
    }
    return retval;
  }

  string stripIteration(string name){
    // from file:/dir/dir/dir/dir/name;1 to file:/dir/dir/dir/dir/name
    string retval = name;
    int resize = (int)retval.find_last_of(";");
    if(resize != (int)std::string::npos)
      retval.resize(resize);
    return retval;
  }
    
//   string addFileToName(std::string toBeAdded, std::string fileName){
//     // from dir/dir/dir to filename:/dir/dir/dir
//     // from file:/dir/dir/dir to fileName:/dir/dir/dir
//     int retval = (int)toBeAdded.find(":");
//     if(retval == (int)std::string::npos){
//       string tmp = fileName;
//       tmp += ":";
//       tmp += toBeAdded;
//       return tmp;
//     }
//     else{
//       toBeAdded.erase(0,retval+1);
//       string tmp = fileName;
//       tmp += ":";
//       tmp += toBeAdded;
//       return tmp;    
//     }
//   }

//   std::vector<double> histoToVector(Histo& his){
//     std::vector<double> retval;
//     uint i = 0;
//     int ii = 0;
//     retval.push_back(0.987654321); // code for identify an histogram

//     retval.push_back(his.name().size());
//     for(i = 0; i < his.name().size(); i++){
//       retval.push_back(his.name().at(i));
//     }
//     retval.push_back(his.title().size());
//     for(i = 0; i < his.title().size(); i++){
//       retval.push_back(his.title().at(i));
//     }
//     retval.push_back(his.nDim());
//     if(his.nDim() == 1){
//       retval.push_back(his.nBin(0));
//       retval.push_back(his.max(0));
//       retval.push_back(his.min(0));
//       for(ii = 0; ii < his.nBin(0); ii++){
// 	retval.push_back(his(ii));
//       }
//     }
//     else if(his.nDim() == 2){
//       retval.push_back(his.nBin(0));
//       retval.push_back(his.max(0));
//       retval.push_back(his.min(0));
//       retval.push_back(his.nBin(1));
//       retval.push_back(his.max(1));
//       retval.push_back(his.min(1));
//       int j = 0;
//       for(ii = 0; ii < his.nBin(0); ii++){
// 	for(j = 0; j < his.nBin(1); j++){
// 	  retval.push_back(his(ii,j));
// 	}
//       }
//     }
//     else{
//       stringstream a;
//       a << "histoToVector error: nDim " << his.nDim() << "not allowed";
//       throw PixDBException(a.str());
//     }

//     return retval;
//   }

//   Histo vectorToHisto(std::vector<double>& vec){
//     if(vec.size() == 0){
//       stringstream a;
//       a << "histoToVector error: vector size is zero" ; 
//       throw PixDBException(a.str());
//     }
//     if(vec[0] != 0.987654321){
//       stringstream a;
//       a << "vectorToHisto error: vec[0] is " << vec[0] << "not 0.987654321, vector is not an histo ";
//       throw PixDBException(a.str());
//     }

//     std::string hisName;
//     hisName.resize(static_cast<int>(vec[1]));
//     uint i;
//     for(i = 0; i < hisName.size(); i++){
//       hisName.at(i) = static_cast<int>(vec[2+i]);
//     }

//     std::string hisTitle;
//     hisTitle.resize(static_cast<int>(vec[2+hisName.size()]));

//     for(i = 0; i < hisTitle.size(); i++){
//       hisTitle.at(i) = static_cast<int>(vec[2+hisName.size()+1+i]);
//     }
//     int hisnDim = static_cast<int>(vec[2+hisName.size()+hisTitle.size()+1]);
//     double hisnBin[2];
//     double hisMax[2];
//     double hisMin[2];

//     std::vector<std::vector<double> > his;

//     if(hisnDim == 1){
//       hisnBin[0] = vec[2+hisName.size()+hisTitle.size()+1+1];
//       hisMax[0] = vec[2+hisName.size()+hisTitle.size()+1+2];
//       hisMin[0] = vec[2+hisName.size()+hisTitle.size()+1+3];
//       std::vector<double> hisRow;
//       for(i = 0; i < hisnBin[0]; i++){
// 	hisRow.push_back(vec[2+hisName.size()+hisTitle.size()+5+i]);
//       }
//       his.push_back(hisRow);
//     }
//     else if(hisnDim == 2){
//       hisnBin[0] = vec[2+hisName.size()+hisTitle.size()+1+1];
//       hisMax[0] = vec[2+hisName.size()+hisTitle.size()+1+2];
//       hisMin[0] = vec[2+hisName.size()+hisTitle.size()+1+3];
//       hisnBin[1] = vec[2+hisName.size()+hisTitle.size()+1+4];
//       hisMax[1] = vec[2+hisName.size()+hisTitle.size()+1+5];
//       hisMin[1] = vec[2+hisName.size()+hisTitle.size()+1+6];
//       uint j = 0;
//       for(i = 0; i < hisnBin[0]; i++){
// 	std::vector<double> hisRow;
// 	for(j = 0; j < hisnBin[1]; j++){
// 	  hisRow.push_back(vec[2+hisName.size()+hisTitle.size()+1+6+1+j+i*(int)hisnBin[1]]);
// 	}
// 	his.push_back(hisRow);
//       }
//     }
//     else{
//       stringstream a;
//       a << "vectorToHisto error: nDim is " << hisnDim << " value not allowed";
//       throw PixDBException(a.str());
//     }

//     if(hisnDim == 1){
//       Histo retval(hisName,hisTitle,(int)hisnBin[0],hisMin[0],hisMax[0]);
//       for(i = 0; i < hisnBin[0]; i++){
// 	retval.set(i,his[0][i]);
//       }
//       return retval;
//     }
//     else{
//       Histo retval(hisName,hisTitle,(int)hisnBin[0],hisMin[0],hisMax[0],(int)hisnBin[1],hisMin[1],hisMax[1]);
//       for(i = 0; i < hisnBin[0]; i++){
// 	for(uint j = 0; j < hisnBin[1]; j++){
// 	  retval.set(i,j,his[i][j]);
// 	}
//       }
//       return retval;
//     }
//   }

//   int vectorHistoDim(vector<double>& vec){ // returns 0 if vec is not an Histo, 1 or 2 as Histo dimension
//     if(vec.size() == 0 || vec[0] != 0.987654321){
//       return 0;
//     }	
//     return static_cast<int>(vec[(int)(2+vec[1]+ vec[2+(int)vec[1]]+1)]);
//   }


}


TCanvas* RootDbField::dumpCanvas = 0;

string PixLib::getMySubDir(string name){
  // from file:/dir/dir/dir/mysubdir to mysubdir
  // from file:/ to nothing
  // from /dir/dir/dir/mysubdir/ to mysubdir
  // from /mysubdir/ to mysubdir

  name = purgeSlash(name);
  int pos = (int)name.find_last_of("/",name.length());
  if(pos != (int)std::string::npos){
    name.erase(0,pos+1);
  }
  return name;
}

void PixLib::copyField(RootDbField* target, DBField* origin){
  // copy the content of a field pointed by f to the current field. If the field already does have a content, erase it.
  // the name (e.g. VCAL) and the decname (e.g. file:/dir/mydir(parentRecord)/VCAL;1) of the field is copied too. 
  
  target->getName() = origin->getName();
  target->getDecName() = origin->getDecName();
  
  if(target->m_dataType != DBEMPTY){
    target->m_myIntCont.Set(1);
    target->m_myFloatCont.Set(1);
    target->m_myDoubleCont.Set(1);
    target->m_myStringCont = "";
    target->m_dataType = DBEMPTY;
  }
  DBDataType originType = origin->getDataType();
  switch(originType){
  case INT:
    {int valInt;
    target->m_dataType = DBINT;
    origin->getDB()->DBProcess(origin,READ,valInt);
    target->m_myIntCont.Set(1);
    target->m_myIntCont.AddAt(valInt,0);
    break;}
  case VECTORINT:
    {vector<int> valVInt;
    target->m_dataType = DBVECTORINT;
    origin->getDB()->DBProcess(origin,READ,valVInt);
    target->m_myIntCont.Set(valVInt.size());
    for(size_t i = 0; i < valVInt.size(); i++){
      target->m_myIntCont.AddAt(valVInt[i],i);      
    }}
    break;
  case ULINT:
    {unsigned int valUInt;
    target->m_dataType = DBULINT;
    origin->getDB()->DBProcess(origin,READ,valUInt);
    stringstream s;
    s << valUInt;
    target->m_myStringCont = s.str().c_str();
    break;}
  case FLOAT:
    {float valFloat;
    target->m_dataType = DBFLOAT;
    origin->getDB()->DBProcess(origin,READ,valFloat);
    target->m_myFloatCont.Set(1);
    target->m_myFloatCont.AddAt(valFloat,0);
    break;}
  case VECTORFLOAT:
    {vector<float> valVFloat;
    target->m_dataType = DBVECTORFLOAT;
    origin->getDB()->DBProcess(origin,READ,valVFloat);
    target->m_myFloatCont.Set(valVFloat.size());
    for(size_t i = 0; i < valVFloat.size(); i++){
      target->m_myFloatCont.AddAt(valVFloat[i],i);      
    }}
    break;
  case DOUBLE:
    {double valDouble;
    target->m_dataType = DBDOUBLE;
    origin->getDB()->DBProcess(origin,READ,valDouble);
    target->m_myDoubleCont.Set(1);
    target->m_myDoubleCont.AddAt(valDouble,0);
    break;}
  case VECTORDOUBLE:
    {vector<double> valVDouble;
    target->m_dataType = DBVECTORDOUBLE;
    origin->getDB()->DBProcess(origin,READ,valVDouble);
    target->m_myDoubleCont.Set(valVDouble.size());
    for(size_t i = 0; i < valVDouble.size(); i++){
      target->m_myDoubleCont.AddAt(valVDouble[i],i);      
    }}
    break;
  case BOOL:
    {bool valBool;
    target->m_dataType = DBBOOL;
    origin->getDB()->DBProcess(origin,READ,valBool);
    target->m_myIntCont.Set(1);
    target->m_myIntCont.AddAt(valBool,0);
    break;}
  case VECTORBOOL:
    {vector<bool> valVBool;
    target->m_dataType = DBVECTORBOOL;
    origin->getDB()->DBProcess(origin,READ,valVBool);
    target->m_myIntCont.Set(valVBool.size());
    for(size_t i = 0; i < valVBool.size(); i++){
      target->m_myIntCont.AddAt(valVBool[i],i);      
    }}
    break;
  case HISTO:
    {Histo valHisto;
    target->m_dataType = DBHISTO;
    origin->getDB()->DBProcess(origin,READ,valHisto);
    target->getDb()->DbProcess(target,PixDb::DBCOMMIT,valHisto);
    break;}
  case STRING:
    {string valString;
    target->m_dataType = DBSTRING;
    origin->getDB()->DBProcess(origin,READ,valString);
    target->m_myStringCont = valString.c_str();
    }
    break;
  default:
    {string error;
    error += "PixLib::copyField error: type not processed. Type is ";
    error += DbDataTypeNames[target->m_dataType];
    throw PixDBException(error);}
  }
  return;
} 


RootDbField* RootDbField::copyField(dbFieldIterator f){
  // copy the content of a field pointed by f to the current field. If the field already does have a content, erase it.
  // the name (e.g. VCAL) and the decname (e.g. file:/dir/mydir(parentRecord)/VCAL;1) of the field is copied too. 

  DbField* ft = *f;
  this->getName() = ft->getName();
  this->getDecName() = ft->getDecName();

  if(m_dataType != DBEMPTY){
    m_myIntCont.Set(1);
    m_myFloatCont.Set(1);
    m_myDoubleCont.Set(1);
    m_myStringCont = "";
    m_dataType = DBEMPTY;
  }
  m_dataType = ft->getDataType();
  dbFieldIterator fiter(0,this,0);
  switch(m_dataType){
  case DBINT:
    {int valInt;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valInt);
    m_myIntCont.Set(1);
    m_myIntCont.AddAt(valInt,0);
    break;}
  case DBVECTORINT:
    {vector<int> valVInt;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valVInt);
    m_myIntCont.Set(valVInt.size());
    for(size_t i = 0; i < valVInt.size(); i++){
      m_myIntCont.AddAt(valVInt[i],i);      
    }}
    break;
  case DBULINT:
    {unsigned int valUInt;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valUInt);
	stringstream s;
	s << valUInt;
    m_myStringCont = s.str().c_str();
    break;}
  case DBFLOAT:
    {float valFloat;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valFloat);
    m_myFloatCont.Set(1);
    m_myFloatCont.AddAt(valFloat,0);
    break;}
  case DBVECTORFLOAT:
    {vector<float> valVFloat;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valVFloat);
    m_myFloatCont.Set(valVFloat.size());
    for(size_t i = 0; i < valVFloat.size(); i++){
      m_myFloatCont.AddAt(valVFloat[i],i);      
    }}
    break;
  case DBDOUBLE:
    {double valDouble;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valDouble);
    m_myDoubleCont.Set(1);
    m_myDoubleCont.AddAt(valDouble,0);
    break;}
  case DBVECTORDOUBLE:
    {vector<double> valVDouble;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valVDouble);
    m_myDoubleCont.Set(valVDouble.size());
    for(size_t i = 0; i < valVDouble.size(); i++){
      m_myDoubleCont.AddAt(valVDouble[i],i);      
    }}
    break;
  case DBBOOL:
    {bool valBool;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valBool);
    m_myIntCont.Set(1);
    m_myIntCont.AddAt(valBool,0);
    break;}
  case DBVECTORBOOL:
    {vector<bool> valVBool;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valVBool);
    m_myIntCont.Set(valVBool.size());
    for(size_t i = 0; i < valVBool.size(); i++){
      m_myIntCont.AddAt(valVBool[i],i);      
    }}
    break;
  case DBHISTO:
    {Histo valHisto;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valHisto);
    this->getDb()->DbProcess(fiter,DBCOMMIT,valHisto);
    break;}
  case DBSTRING:
    {string valString;
    ft->getDb()->DbProcess(f,PixDb::DBREAD,valString);
    m_myStringCont = valString.c_str();
    }
    break;
  default:
    {string error;
    error += "RootDbField::copyField error: type not processed. Type is ";
    error += DbDataTypeNames[m_dataType];
    throw PixDBException(error);}
  }
  return this;
}

RootDbField::RootDbField(string decname, int id) : m_id(id), m_decName(decname), m_changedFromDisk(false), m_dataType(DBEMPTY){
  this->m_myFloatCont.Set(1);
  this->m_myDoubleCont.Set(1);
  this->m_myIntCont.Set(1);
}

RootDbField::RootDbField(string name) : m_id(0), m_name(name), m_changedFromDisk(false), m_dataType(DBEMPTY){
  this->m_myDoubleCont.Set(1);
  this->m_myFloatCont.Set(1);
  this->m_myIntCont.Set(1);
} 

void RootDbField::dump(std::ostream& os) const{
  
  os << "Dumping RootDbField named: " << m_name << std::endl;
  os << "RootDbField decName: " << m_decName << std::endl;
  os << "Data type: " <<  DbDataTypeNames[m_dataType] << std::endl;
  os << "Data content: ";
  switch (m_dataType){
  case DBBOOL:
    if(m_myIntCont.At(0) == 0) os << "false" << std::endl; else os << "true" << std::endl;
    break;
  case DBVECTORBOOL:
    int count;
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myIntCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      if(m_myIntCont.At(count) == 0) os << "false" << '\t'; else os << "true" << '\t';
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case DBINT:
    os << m_myIntCont.At(0) << std::endl;
    break;
  case DBULINT:
    os << m_myStringCont.Data() << std::endl;
    break;
  case DBFLOAT:
    os << m_myFloatCont.At(0) << std::endl;
    break;
  case DBDOUBLE:
    os << m_myDoubleCont.At(0) << std::endl;
    break;
  case DBVECTORINT:
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myIntCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      os << m_myIntCont.At(count) << '\t'; 
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case DBVECTORFLOAT:
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myFloatCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      os << m_myFloatCont.At(count) << '\t'; 
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case DBVECTORDOUBLE:
    for(count = 0; count < 5; count++){
      os << "" << count << '\t';
    }
    os << std::endl;
    for(count = 0; count < m_myDoubleCont.GetSize(); count++){
      if(!(count%5) || count == 0) os << count << '\t'; else os << '\t';
      os << m_myDoubleCont.At(count) << '\t'; 
      if(!((count+1)%5)) os << std::endl;
    }
    os << std::endl;
    break;
  case DBHISTO:{
    os << "dumping histogram to Dump Window" << std::endl;
    
    if(!gROOT->GetListOfCanvases()->FindObject("Dump Window")){
      dumpCanvas = new TCanvas("Dump Window","Dump Window",2);
    }
    dumpCanvas->cd();
    std::vector<double> v;
    for(count = 0; count < m_myDoubleCont.GetSize(); count++){
      v.push_back(m_myDoubleCont[count]);
    }
    TH1D* histo1;
    TH2D* histo2;
    Histo his(vectorToHisto(v));
    if(vectorHistoDim(v) == 1){
      histo1 = new TH1D(his.name().c_str(),his.title().c_str(), his.nBin(0), his.min(0),his.max(0));
      for(count = 0; count < his.nBin(0); count++){
	histo1->SetBinContent(count+1,his(count));
      }
      histo1->Draw();
    }
    else if(vectorHistoDim(v) == 2){
      histo2 = new TH2D(his.name().c_str(),his.title().c_str(), his.nBin(0), his.min(0),his.max(0),his.nBin(1), his.min(1),his.max(1));
      int count2;
      for(count = 0; count < his.nBin(0); count++){
	for(count2 = 0; count2 < his.nBin(1); count2++){
	  histo2->SetBinContent(count+1, count2+1, his(count,count2));
	}
      }
      histo2->Draw();
    }
    else{
      stringstream a;
      a << "RootDbField::Dump : Histogram is not 1 or 2 dimensional " << vectorHistoDim(v);
      throw PixDBException(a.str());
    }
    dumpCanvas->Modified();
    dumpCanvas->Update();}
    break;
  case DBSTRING:
    os << m_myStringCont.Data() << std::endl;
    break;
  case DBEMPTY:
    os << "field does not have data content" << std::endl;
  default:
    break;
  }
}

void RootDbField::Dump(void){
  dump(std::cout);
}

void RootDbField::ModifyFieldValue(char* newValue){ // *MENU* modify the content of the Field
  int i = 0; 
  int hi = 0; 
  float f = 0;
  double d = 0;
  bool b = false;
  
  stringstream ss;
  ss.str(newValue);
  ss >> i;
  ss.clear();
  ss.str(newValue);
  ss >> std::hex >> hi >> std::dec;
  ss.clear();
  ss.str(newValue);
  ss >> f;
  ss.clear();
  ss.str(newValue);
  ss >> d;
  ss.clear();
  ss.str(newValue);
  ss >> std::boolalpha >> b;
  
  switch(this->getDataType()){
  case DBINT:
    if(hi!=0 && i==0)
      this->m_myIntCont[0] = hi;
    else 
      this->m_myIntCont[0] = i;
		break;
  case DBULINT:
    this->m_myStringCont = ss.str().c_str();
    break;
  case DBFLOAT:
    this->m_myFloatCont[0] = f;
    break;
  case DBBOOL:
    this->m_myIntCont[0] = b;
    break;
  case DBSTRING:
    this->m_myStringCont = ss.str().c_str();
    break;
  case DBDOUBLE:
    this->m_myDoubleCont[0] = b;
    break;
  default:
    std::cout << "RootDbField::ModifyFieldValue : not supported type" << std::endl;
  }  
  this->Write(this->getName().c_str(),TObject::kOverwrite);
}


ULong_t RootDbField::Hash() const { return TMath::Hash(m_decName.c_str()); }

Bool_t  RootDbField::IsEqual(const TObject *obj) const {
		if (dynamic_cast<RootDbField*>(const_cast<TObject*>(obj)) == NULL) return kFALSE;
		if(m_decName == dynamic_cast<RootDbField*>(const_cast<TObject*>(obj))->m_decName) return kTRUE; else return kFALSE;
	}

Bool_t  RootDbField::IsSortable() const { return kTRUE; }

Int_t   RootDbField::Compare(const TObject *obj) const {
  if (dynamic_cast<RootDbField*>(const_cast<TObject*>(obj)) == NULL) {
    std::stringstream a;
    a << "RootDbField::Compare(): Bad comparison";
    throw PixDBException(a.str());
  }
  ULong_t h1,h2;
  h1 = TMath::Hash(m_decName.c_str());
  h2 = TMath::Hash(dynamic_cast<RootDbField*>(const_cast<TObject*>(obj))->m_decName.c_str());
  if (h1 > h2) return 1;
  else if (h1 < h2) return -1;
  else return 0; }

void RootDbRecord::makeDecName(RootDbRecord* child, RootDbRecord* parent){
  // create the child fully decorated name from its names and the dec name of the parent
  // e.g. from child myDecName file1:/dir/dir/M510234/PixModule;1 and parent mydecName file2:/dir/pixmodgroup1/PixModuleGroup;1 
  // to child myDecName file2:/dir/pixmodgroup1/M510234/PixModule;1
  string tmpDecName = getMyDirectoryName(parent->getMyDecName());
  child->getMyDecName() = purgeSlash(tmpDecName);
  child->getMyDecName() += "/";
  child->getMyDecName() += child->getDecName();
  child->getMyDecName() += "/";
  child->getMyDecName() += child->getName();
}

RootDbRecord* RootDbRecord::copyRecord(dbRecordIterator r){ // copy a record from another record, including the list of fields and the list of depending records
  // copy the names
  this->getName() = (*r)->getName();
  this->getDecName() = (*r)->getDecName();
  this->getMyDecName() = (*r)->getMyDecName();
  // possibly delete old fields connected to this record. Do not remove them from database file
  this->m_myFieldNameList.clear();
  // copy the fields and push them into this record
  dbFieldIterator f = (*r)->fieldBegin();  
  for(;f != (*r)->fieldEnd();f++){
    //    this->pushField(dynamic_cast<RootDbField*>((this->getDb()->makeField((*f)->getName())))->copyField(f));
    this->m_myFieldNameList.push_back((*f)->getDecName());
  }
  dbRecordIterator rd = (*r)->recordBegin();
  for(;rd != (*r)->recordEnd();rd++){
    this->m_myRecordNameList.push_back((*rd)->getMyDecName());    
  }
  //
  return this;
}

std::string RootDbRecord::getFieldName(int number){
  if((uint)number >= m_myFieldNameList.size() || number < 0) return "";
  else return m_myFieldNameList[number];
}

std::string RootDbRecord::getRecordName(int number){
  if((uint)number >= m_myRecordNameList.size() || number < 0) return "";
  else return m_myRecordNameList[number];
}


PixDbInterface* RootDbRecord::getDb() const {return m_myDb;}

dbRecordIterator RootDbRecord::recordBegin() {
  if (m_myRecordNameList.size() == 0) return recordEnd();
  return getRecord(0);
} // the iterator to the depending records		

dbFieldIterator RootDbRecord::fieldBegin() {	
  if (m_myFieldNameList.size() == 0) return fieldEnd();
  return getField(0);
} // the iterator to the record data fields	

dbRecordIterator  RootDbRecord::recordEnd() {
  dbRecordIterator r(m_myRecordNameList.size(),NULL,this);
  return r;
} // the end iterator to the depending records	

dbFieldIterator  RootDbRecord::fieldEnd() {
  dbFieldIterator r(m_myFieldNameList.size(),NULL,this);
  return r;
} // the end iterator to the record data fields	

dbRecordIterator RootDbRecord::findRecord(const string recordName){
  uint it;
  // try first with namedir/name 
  //if not found, try with file:/dir/dir/dir/namedir/name;1
  for(it = 0; it < m_myRecordNameList.size(); it++)
    {
      if(stripIteration(getSubRecordNameFromList(m_myRecordNameList[it])) == stripIteration(recordName)) break;
    }
  if(it == m_myRecordNameList.size()){
    for(it = 0; it < m_myRecordNameList.size(); it++)
      {
	if(m_myRecordNameList[it] == recordName) break;
      }
  }
  if(it == m_myRecordNameList.size()){
    return recordEnd();
  }
  else{
    dbRecordIterator returnvalue(it, NULL, this);
    return returnvalue;
  }
  //  else return getRecord(it);
}; // find a subrecord by its name

dbFieldIterator RootDbRecord::findField(const string fieldName){
  uint it;
  for(it = 0; it < m_myFieldNameList.size(); it++)
    {
      if(stripIteration(getFieldNameFromList(m_myFieldNameList[it])) == stripIteration(fieldName)) break;
    }
  if(it == m_myFieldNameList.size()){
    for(it = 0; it < m_myFieldNameList.size(); it++)
      {
	if(m_myFieldNameList[it] == fieldName) break;
      }
  }
  if(it == m_myFieldNameList.size()){
    return fieldEnd();
  }
  else{
    dbFieldIterator retvalue(it, NULL, this);
    return retvalue;
  }
  //  return getField(it); // commented since findField should only return the iterator, not actually access to the database
}; // find a field by its name 

dbRecordIterator RootDbRecord::findRecord(const char* name){
  string sname(name);
  return findRecord(sname);
}; // find a subrecord by its name

dbFieldIterator RootDbRecord::findField(const char* name){
  string sname(name);
  return findField(sname);
}; // find a field by its name 

void RootDbRecord::dump(std::ostream& os) const{
  os << "Dumping RootDbRecord" << std::endl;
  os << "record name: " << m_name << std::endl;
  os << "record decName: " << m_decName << std::endl;
  os << "record myDecName: " << m_myDecName << std::endl;
  
  os << "List of depending field: " << m_myFieldNameList.size() << " depending fields found" << std::endl;
  unsigned int i;
  for(i = 0; i < m_myFieldNameList.size(); i++){
    os << m_myFieldNameList[i] << std::endl;
    //   dbFieldIterator f = (const_cast<RootDbRecord*>(this))->getField(i);
    //(*f)->dump(os);
  }
  os << "List of depending record: " << m_myRecordNameList.size() << " depending records found" << std::endl;
  for(i = 0; i < m_myRecordNameList.size(); i++){
    os << m_myRecordNameList[i] << std::endl;
    //dbRecordIterator f = (const_cast<RootDbRecord*>(this))->getRecord(i);
    //(*f)->dump(os);
  }
  
} // Dump - debugging purpose

int RootDbRecord::getDependingRecordSize(void) const {return m_myRecordNameList.size();} // RootDbRecord::Get the amount of depending records

int RootDbRecord::getDependingFieldSize(void) const{return m_myFieldNameList.size();} // Get the amount of depending field

dbFieldIterator RootDbRecord::getField(int fieldNumber){
  uint in = fieldNumber;
  if(in > m_myFieldNameList.size()){
    std::stringstream a;
    a << "RootDbRecord::getField(int fieldNumber): record number " << in << " not found in record " << getDecName().c_str() << getMyDecName().c_str();
    throw PixDBException(a.str().c_str());    
  }
  else if(in == m_myFieldNameList.size()){
    return fieldEnd();
  }
  else{
    //    dbFieldIterator r(fieldNumber,m_myDb->DbFindFieldByName(this->m_myFieldNameList[in]),this); 
    dbFieldIterator r(fieldNumber,0,this); 
    return r;
  }
} // Get the pointer to the field number fieldNumber; throw an exception if fails

dbRecordIterator RootDbRecord::getRecord(int recordNumber){
  uint in = recordNumber;
  if(in > m_myRecordNameList.size()){
    std::stringstream a;
    a << "RootDbRecord::getRecord(int recordNumber): record number " << in << " not found in record " << getDecName().c_str() << getMyDecName().c_str();
    throw PixDBException(a.str().c_str());    
  }
  else if(in == m_myRecordNameList.size()){
    return recordEnd();
  }
  else{
    //    dbRecordIterator r(recordNumber,m_myDb->DbFindRecordByName(m_myRecordNameList[in]),this); 
    dbRecordIterator r(recordNumber,0,this); 
    return r;
  }
} // Get the pointer to the record number recordNumber; throw an exception if fails

dbFieldIterator RootDbRecord::pushField(DbField* in){
  // push the field in into the record; temporary copy it into the list of pushed fields
  RootDbField* dbf = dynamic_cast<RootDbField*>(this->getDb()->makeField(in->getName()));
  dbFieldIterator f;
  f.pointsTo(in);
  dbf->copyField(f);

  int i = 0; 
  for(; i < (int)pushedFields.size(); i++){
    if(pushedFields.at(i)->getName() == (*f)->getName()){
      delete pushedFields.at(i);
      pushedFields.at(i) = *f;
      break;
    }
  } 
  if(i == (int)pushedFields.size()){
    pushedFields.push_back(*f);
  }

  dbf->getDecName() = getMyDirectoryName(this->m_myDecName);
  dbf->getDecName() += "/";
  dbf->getDecName() += dbf->getName();
  int pos = (int)(dbf->getDecName().find(";1"));
  if(pos == (int)std::string::npos) // found no ;1 in the name: adding it
    dbf->getDecName() += ";1";  

  //overwrite if same name
  dbFieldIterator fieldToFind = findField(dbf->getName());
  if(fieldToFind == fieldEnd()){
    this->m_myFieldNameList.push_back(dbf->getDecName());         
  }
  else{
    //    dbFieldIterator fi = findField(in->getName());
    eraseField(fieldToFind);
    this->m_myFieldNameList.push_back(dbf->getDecName());             
  }
  // access to the file 
  m_myDb->putField(dbf, this);
  m_myDb->putRecord(this);
  // get back the field
  dbFieldIterator retval = this->findField(dbf->getDecName());
  // delete the temporary field
  delete dbf;

  return retval;
} // add a field to the DbRecord fields and return the corresponding iterator. the field is actually copied into the database

void RootDbRecord::eraseField(dbFieldIterator it){
  dbFieldIterator i = this->findField((*it)->getName());
  if(i != this->fieldEnd()){
    this->m_myFieldNameList.erase(i.isNumber() + m_myFieldNameList.begin());
    // access to file
    m_myDb->putRecord(this);
  }
  else{
    stringstream a;
    a << "RootDbRecord::eraseField: field " << (*it)->getName() << " not found in record " << *this;
    std::cout << a.str();
    return;
    throw PixDBException(a.str());
  }
  return;
  
}; // erase a field. This means taking it out of the list of depending fields of this record. The disk content and memory content is not changed, since other records could refer to the same field

dbRecordIterator RootDbRecord::linkRecord(DbRecord* in){
  string tmpDecName = in->getDecName(); string tmpName = in->getName();
  if(findRecord(tmpDecName + "/" + tmpName) == recordEnd()){
    this->m_myRecordNameList.push_back(in->getMyDecName());         
  }
  else{
    dbRecordIterator ri = findRecord((tmpDecName + "/" + tmpName));
    eraseRecord(ri);
    this->m_myRecordNameList.push_back(in->getMyDecName());             
  }
  // write the pushing record - access to file
  m_myDb->putRecord(this);

  dbRecordIterator retval = this->findRecord(tmpDecName + "/" + tmpName);
  return retval;
}

dbRecordIterator RootDbRecord::pushRecord(DbRecord* in, enum DbPushMode pushMode){
  if(pushMode == DBLINK) linkRecord(in);
  // create a copy of the pushed record
  RootDbRecord* dbi = dynamic_cast<RootDbRecord*>(this->getDb()->makeRecord(in->getName(), getMySubDir(in->getDecName())));
  // change the fully decorated name
  makeDecName(dbi,this);
  // overwrite if find a record ->of the same name
  string tmpDecName = dbi->getDecName(); string tmpName = dbi->getName();
  if(findRecord(tmpDecName + "/" + tmpName) == recordEnd()){
    this->m_myRecordNameList.push_back(dbi->getMyDecName());         
  }
  else{
    dbRecordIterator ri = findRecord((tmpDecName + "/" + tmpName));
    eraseRecord(ri);
    this->m_myRecordNameList.push_back(in->getMyDecName());             
  }

  // push fields of the pushed record
  dbFieldIterator fi = in->fieldBegin();
  for(; fi != in->fieldEnd(); fi++){
    dbi->pushField(*fi);
  }
  // push records of the pushed record - if mode == DBRECURSE
  if(pushMode == DBRECURSE){
    dbRecordIterator rri = in->recordBegin();
    for(; rri != in->recordEnd(); rri++){
      dbi->pushRecord(*rri);
    }
    m_myDb->DbProcess(dbi,PixDb::DBCOMMIT);
  }
  // write the pushing record and the pushed record - both have changed - access to file
  m_myDb->putRecord(dbi);
  m_myDb->putRecord(this);
  // get back the field
  dbRecordIterator retval = this->findRecord(tmpDecName + "/" + tmpName);
  // delete the temporary field
  delete dbi;

  return retval;
} // add a record to the record list and return the corresponding iterator

void RootDbRecord::eraseRecord(dbRecordIterator it) {
  
  if(it != this->recordEnd()){
    this->m_myRecordNameList.erase(it.isNumber() + m_myRecordNameList.begin());
    // access to file
    m_myDb->putRecord(this);
  }
  return;
}; // erase a record

DbRecord* RootDbRecord::addRecord(string name, string dirName) {
  dbRecordIterator ri = findRecord(dirName+"/"+name);
  if (ri == recordEnd()) {
    std::cout << "record not found" << std::endl;
    DbRecord *newInq = m_myDb->makeRecord(name, dirName);
    dbRecordIterator r = pushRecord(newInq);
    getDb()->DbProcess(r,PixDb::DBREAD);
    delete newInq;
    return *r;
  }
  return *ri;
}

void RootDbRecord::Dump(void){
  dump(std::cout);
}

void RootDbRecord::RemoveDependingRecord(int recordPosition){
  std::vector<std::string>::iterator i;
  int ii = 0;
  for(i =  m_myRecordNameList.begin(); i !=  m_myRecordNameList.end(); i++,ii++){
    if(ii == recordPosition){
      m_myRecordNameList.erase(i);
      break;
    }
  }
  this->Write(this->getName().c_str(),TObject::kOverwrite);
} // *MENU* remove the depending record at position recordPosition

void RootDbRecord::AddDependingRecord(char* newDependingRecord){
  m_myRecordNameList.push_back(newDependingRecord);
  this->Write(this->getName().c_str(),TObject::kOverwrite);
} // *MENU* add the depending record

void RootDbRecord::ChangeDependingRecord(int recordPosition, char* newDependingRecord){
  m_myRecordNameList[recordPosition] = newDependingRecord;
  this->Write(this->getName().c_str(),TObject::kOverwrite);
}// *MENU* modyfy the depending record

void RootDbRecord::DumpFields(void){
  Dump();
  for(uint i = 0; i < this->m_myFieldNameList.size(); i ++){
    std::cout << i << m_myFieldNameList[i] << std::endl;
    dynamic_cast<RootDbField*>(*(this->getField(i)))->Dump();
  }
} // *MENU* dump on screen the record and its fields

void RootDbRecord::AppendThisToAnotherRecord(char * /*newParentRecord*/, char* /*newDecName*/){
} // *MENU* copy this record and the depending record to the new parent record (found by its decName)

ULong_t RootDbRecord::Hash() const {
  return TMath::Hash(m_myDecName.c_str());
}

Bool_t  RootDbRecord::IsEqual(const TObject *obj) const {
  if (dynamic_cast<RootDbRecord*>(const_cast<TObject*>(obj)) == NULL) return kFALSE;
  if((m_myDecName == dynamic_cast<RootDbRecord*>(const_cast<TObject*>(obj))->m_myDecName) &&
     (m_id == dynamic_cast<RootDbRecord*>(const_cast<TObject*>(obj))->m_id))
    return kTRUE; else return kFALSE;
}

Bool_t  RootDbRecord::IsSortable() const { 
  return kTRUE; 
}

Int_t   RootDbRecord::Compare(const TObject *obj) const {
  if (dynamic_cast<RootDbRecord*>(const_cast<TObject*>(obj)) == NULL) {
    std::stringstream a;
    a << "RootDbRecord::Compare(): Bad comparison";
    throw PixDBException(a.str());
  }
  ULong_t h1,h2;
  stringstream s1,s2;
  s1 << m_id;
  s2 << dynamic_cast<RootDbRecord*>(const_cast<TObject*>(obj))->m_id;
  std::string nameNumber = m_myDecName + s1.str();
  h1 = TMath::Hash(nameNumber.c_str());
  RootDbRecord* tr =  dynamic_cast<RootDbRecord*>(const_cast<TObject*>(obj));
  h2 = TMath::Hash((tr->m_myDecName+s2.str()).c_str());
  if (h1 > h2) return 1;
  else if (h1 < h2) return -1;
  else return 0; 
}

RootDbRecord::RootDbRecord(string name, string decName, string myDecName) : 
  m_id(0),
  m_name(name),
  m_decName(decName),
  m_myDecName(myDecName)
{}

RootDbRecord::RootDbRecord(string name, string decName):
  m_id(0),
  m_name(name),
  m_decName(decName)
{}

RootDbRecord::RootDbRecord(string name) : 
m_id(0), 
m_name(name)
{}

RootDbRecord::RootDbRecord(string mydecname, int id) : 
  m_id(id), 
  m_myDecName(mydecname)
{} // constructor used in RootDb::getRecordbydecname. In the TBTree records are ordered by thei unique mydecname

bool  RootDb::commitRootRecordReplaceTree(RootDbRecord* currRecord){
  bool retval;
  int i;
  retval = putRecord(currRecord);
  for(i = 0; i < (int)currRecord->m_myRecordNameList.size(); i++){
    retval = commitRootRecordReplaceTree(dynamic_cast<RootDbRecord*>(*(currRecord->getRecord(i))));
  }
  return retval;
}

bool RootDb::putRecord(RootDbRecord* recordToPut){
  bool retval;
  // write the record to the disk
  // first: move to the correct directory, possibly creating it
  TDirectory* currDir = gDirectory;
  TFile* theFile = getFileHandler(recordToPut->getMyDecName());
  //  m_theFile->cd();
  theFile->cd();
    //    gDirectory = m_theFile;
    //gDirectory = theFile;
  //  std::cout << gDirectory->GetPath() << std::endl;
  RootCDCreate(theFile,recordToPut);
  // second: write the record to the disk
  retval = recordToPut->Write(stripIteration(recordToPut->getName().c_str()).c_str(),TObject::kOverwrite);
  // third: delete the record from memory
  //  std::string recordName = recordToPut->getMyDecName();
  //retval = (RootDbRecord*)m_theLoadedRecords.FindObject(recordToPut);
  //if(retval != 0)m_theLoadedRecords.Remove(recordToPut);
  // forth: reload the record from disk and put it into the database, in order to achieve a "clean" situation  
  //recordToPut = getRecordByDecName(recordName);
  // sixth: return to the original directory
  gDirectory = currDir;
  return retval;
}

bool RootDb::putField(RootDbField* fieldToPut, RootDbRecord* recordWherePut){
  // first: move to the correct directory, possibly creating it
  bool retval;
  TDirectory* currDir = gDirectory; 
  TFile* theFile = getFileHandler(recordWherePut->getMyDecName());
  theFile->cd();
  //  gDirectory = m_theFile;
  RootCDCreate(gDirectory,recordWherePut);
  // write the field to the disk
  retval = fieldToPut->Write(stripIteration(fieldToPut->getName().c_str()).c_str(),TObject::kOverwrite);
  // remove the field from memory
  currDir->cd();
  return retval;
}

RootDbRecord* RootDb::getRecordByDecName(string name) {
  TDirectory* currDir = gDirectory; 
  TFile* theFile = getFileHandler(name);
  RootDbRecord * rootval;
  RootDbRecord * retval = 0;
  // add the ;1 required by root in order to get the object from file rather than from memory, if in the name there is not that ;1
  int pos = (int)name.find(";1");
  if(pos == (int)std::string::npos) // found no ;1 in the name: adding it
    name += ";1";

  rootval = (RootDbRecord*)theFile->Get(name.c_str());
  if(rootval != 0){ // found and loaded into memory a record named name.c_str()
    rootval->m_myDb = this;
    retval = dynamic_cast<RootDbRecord*>(this->makeRecord(" "," "));
    dbRecordIterator ri(rootval);
    retval->copyRecord(ri);
  }
  gDirectory = currDir;
  return retval;
}

RootDbField* RootDb::getFieldByDecName(string name) {
  TDirectory* currDir = gDirectory; 
  TFile* theFile = getFileHandler(name);
  RootDbField* rootval = 0;
  RootDbField* retval = 0;
  // add the ;1 required by root in order to get the object from file rather than from memory, if in the name there is not that ;1
  int pos = (int)name.find(";1");
  if(pos == (int)std::string::npos) // found no ;1 in the name: adding it
    name += ";1";
  
  rootval = (RootDbField*)theFile->Get(name.c_str());
  if(rootval != 0) {
    rootval->m_myDb = this;
    retval = dynamic_cast<RootDbField*>(this->makeField(" "));
    dbFieldIterator fi(rootval);
    retval->copyField(fi);
  }
  gDirectory = currDir;
  return retval;
}

bool RootDb::getRecordByDecNameFromDisk(string name, RootDbRecord* toRead){
  // add the ;1 required by root in order to get the object from file rather than from memory, if in the name there is not that ;1
  int pos = (int)name.find(";1");
  if(pos == (int)std::string::npos) // found no ;1 in the name: adding it
    name += ";1";
  RootDbRecord* fromDisk = (RootDbRecord*)m_theFile->Get(name.c_str());
  if(fromDisk != 0){
    (*toRead) = (*fromDisk);
    delete fromDisk;
    return true;
  }
  else return false;
}

bool RootDb::getFieldByDecNameFromDisk(string name, RootDbField* toRead){
  // add the ;1 required by root in order to get the object from file rather than from memory, if in the name there is not that ;1
  int pos = (int)name.find(";1");
  if(pos == (int)std::string::npos) // found no ;1 in the name: adding it
    name += ";1";
  RootDbField* fromDisk = (RootDbField*)m_theFile->Get(name.c_str());
  if(fromDisk != 0){
    (*toRead) = (*fromDisk);
    delete fromDisk;
    return true;
  }
  else return false;
}

void RootDb::RootCDCreate(TDirectory* /*startingDir*/, RootDbRecord* record){
  std::vector<string> items;
  std::string tmpstring = getMyDirectoryName(record->getMyDecName());
  items = pathItemize(tmpstring);
  if(items.size() == 0) return;
  uint count = 0;
  // create the directory if needed
  for(;count < items.size(); count++){
    if(gDirectory->GetKey(purgeSlash(items[count]).c_str()) == 0){
      gDirectory->mkdir(purgeSlash(items[count]).c_str());
    }
    gDirectory->cd(items[count].c_str());
  }
}

TFile* RootDb::getFileHandler(std::string name){
  std::string fileName = getFileName(name);
  std::map<std::string, TFile*>::iterator itFind = m_theFileHandlers.find(fileName);
  if(itFind != m_theFileHandlers.end()){
    return (*itFind).second; 
  }
  else{
    TFile * addedFile = new TFile(fileName.c_str(), "READ");
    if (addedFile->IsZombie()){
      stringstream a;
      a << "File " << addedFile->GetName() << "not opened";
      throw PixDBException(a.str());
    }
    std::pair<std::string, TFile*> insPair(fileName, addedFile);
    m_theFileHandlers.insert(insPair);
    return addedFile;
  }
}

DbRecord* RootDb::makeRecord(string name, string decName){ // name: the object this Record points to, e.g. PixFE; decName: the decorated Name, that is the mydir of the record 
  if(decName.size() == 0){
    throw PixDBException("RootDb::MakeRecord: Directory name not provided, decName.size() == 0");    
  }
  //  decName = addFileToName(decName, this->m_theFileName);
  RootDbRecord * dbrecord = new RootDbRecord(name, decName);
  dbrecord->m_myDb = this;
  return dbrecord;
}

DbField* RootDb::makeField(string name){
  RootDbField * dbfield = new RootDbField(name);
  dbfield->m_myDb = this;
  return dbfield;
}

DbRecord* RootDb::readRootRecord() { // get the root record 
  TDirectory* k = gDirectory;
  // build the ROOT name of the required rootRecord
  m_theFile->cd();
  string name(m_theFile->GetPath());
  name += "rootRecord;1";
  // get the rootRecord from file or from the TBtree of the records
  RootDbRecord* retval = getRecordByDecName(name);
  if(retval == 0){
    string error("RootDb::readRootRecord(): not found root record");
    throw PixDBException(error.c_str());    
  }
  gDirectory = k;
  return retval;
}

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, Histo& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBHISTO || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    
  
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    for(int i = 0; i < field->m_myDoubleCont.GetSize(); i++){
      tmpvec.push_back(field->m_myDoubleCont.At(i));
    }
    retval = vectorToHisto(tmpvec);		
    return theField;
  case DBCOMMIT:{
    field->m_dataType = DBHISTO;
    tmpvec = histoToVector(retval);
    field->m_myDoubleCont.Set(tmpvec.size());
    for(uint ii = 0; ii < tmpvec.size(); ii++){
      field->m_myDoubleCont[ii] = tmpvec[ii];
    }
    field->m_changedFromDisk = true;
    return theField;
    break;}
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented histo" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, string& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBSTRING || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    retval = field->m_myStringCont;
    return theField;
  case DBCOMMIT:
    field->m_dataType = DBSTRING;
    field->m_myStringCont = retval.c_str();
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented string" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, int& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBINT || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    

  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    retval = field->m_myIntCont.At(0);
    return theField;
  case DBCOMMIT:
    field->m_dataType = DBINT;
    field->m_myIntCont[0] = retval;
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented int" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, unsigned int & retval){
  std::vector<double> tmpvec;
  stringstream s;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBULINT || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    

  switch (mode){
  case DBREAD:
    {    
      if(field->getDataType() == DBEMPTY){
	stringstream a;
	a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
	throw PixDBException(a.str());
      }    
      unsigned int i, hi;
      s.str(field->m_myStringCont.Data());
      s >> i;
      s.clear();
      s.str(field->m_myStringCont.Data());
      s >> std::hex >> hi >> std::dec;
      if(hi!=0 && i==0) 
	retval = hi;
      else
	retval = i;
      return theField;}
    break;
  case DBCOMMIT:
    field->m_dataType = DBULINT;
    s << retval;
    s >> field->m_myStringCont;
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented uint" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, float& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBFLOAT || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    
  
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    retval = field->m_myFloatCont.At(0);
    return theField;
  case DBCOMMIT:
    field->m_dataType = DBFLOAT;
    field->m_myFloatCont[0] = retval;
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented float " << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, double& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBDOUBLE || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    

  
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    retval = field->m_myDoubleCont.At(0);
    return theField;
    break;
  case DBCOMMIT:
    field->m_dataType = DBDOUBLE;
    field->m_myDoubleCont[0] = retval;
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented double" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, bool& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBBOOL || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    

  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    retval = field->m_myIntCont.At(0);
    return theField;
    break;
  case DBCOMMIT:
    field->m_dataType = DBBOOL;
    field->m_myIntCont[0] = retval;
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented bool" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator


dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<int>& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBVECTORINT || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    


  
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    for(int i = 0; i < field->m_myIntCont.GetSize(); i++){
      retval.push_back(field->m_myIntCont.At(i));
    }
    return theField;
    break;
  case DBCOMMIT:
    uint ii;
    field->m_dataType = DBVECTORINT;
    field->m_myIntCont.Set(retval.size());
    for(ii = 0; ii < retval.size(); ii++){
      field->m_myIntCont[ii] = retval[ii];
    }
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented vectorint" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator


dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<bool>& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBVECTORBOOL || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    

  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    for(int i = 0; i < field->m_myIntCont.GetSize(); i++){
      retval.push_back(field->m_myIntCont.At(i));
    }
    return theField;
    break;
  case DBCOMMIT:
    uint ii;
    field->m_dataType = DBVECTORBOOL;
    field->m_myIntCont.Set(retval.size());
    for(ii = 0; ii < retval.size(); ii++){
      field->m_myIntCont[ii] = retval[ii];
    }
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented vectorbool" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator


dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<float>& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBVECTORFLOAT || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    
  
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    for(int i = 0; i < field->m_myFloatCont.GetSize(); i++){
      retval.push_back(field->m_myFloatCont.At(i));
    }
    return theField;
    break;
  case DBCOMMIT:
    uint ii;
    field->m_dataType = DBVECTORFLOAT;
    field->m_myFloatCont.Set(retval.size());
    for(ii = 0; ii < retval.size(); ii++){
      field->m_myFloatCont[ii] = retval[ii];
    }
    field->m_changedFromDisk = true;
    
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented vectorfloat " << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode, vector<double>& retval){
  std::vector<double> tmpvec;
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(*theField != 0){
    field = dynamic_cast<RootDbField*>(*theField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  else{
    if(theField.m_parentRecord != 0) {
      dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
      tmpField = rf;
    }
    else{
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
      throw PixDBException(a.str());
    }
    field = dynamic_cast<RootDbField*>(*tmpField);
    if(field == 0){
      stringstream a;
      a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
    }
  }
  if(!(field->getDataType() == DBVECTORDOUBLE || field->getDataType() == DBEMPTY)){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is a " << DbDataTypeNames[field->getDataType()] << " on a string process";
    throw PixDBException(a.str());
  }    




  
  switch (mode){
  case DBREAD:
    if(field->getDataType() == DBEMPTY){
      stringstream a;
      a << "RootDb::DbProcess: try to read an empty dbFieldIterator";
      throw PixDBException(a.str());
    }    
    for(int i = 0; i < field->m_myDoubleCont.GetSize(); i++){
      retval.push_back(field->m_myDoubleCont.At(i));
    }
    return theField;
    break;
  case DBCOMMIT:
    uint ii;
    field->m_dataType = DBVECTORDOUBLE;
    field->m_myDoubleCont.Set(retval.size());
    for(ii = 0; ii < retval.size(); ii++){
      field->m_myDoubleCont[ii] = retval[ii];
    }
    field->m_changedFromDisk = true;
    return theField;
    break;
  default:
    stringstream a;
    a << "RootDb::DbProcess: mode not implemented vector double" << mode;
    throw PixDBException(a.str());
    break;
  };
  return 0;
} // read or commit the field pointed by the iterator

dbRecordIterator RootDb::DbProcess(dbRecordIterator therecord, enum DbAccessType mode){ // read or commit (possibly recursively) the record pointed by iterator
  RootDbRecord* dbi = dynamic_cast<RootDbRecord*>(*therecord);
  //if(dbi == 0) {
  //    stringstream a;
  //  a << "RootDb::DbProcess: trying to process an non RootDbRecord";
  //  throw PixDBException(a.str());
  //}
  if(mode == PixDb::DBREAD){
    dbRecordIterator rf(therecord.m_number, getRecordByDecName(dynamic_cast<RootDbRecord*>(therecord.m_parentRecord)->getRecordName(therecord.m_number)), therecord.m_parentRecord);
    return rf;
  }
  else if (mode == DBCOMMITREPLACETREE){
    commitRootRecordReplaceTree(dbi);
    return therecord;
  }
  else if (mode == DBCOMMIT){
    putRecord(dbi);
    therecord.pointsTo(dbi);
    return therecord;
  }
  else{
    dbRecordIterator rdef = (*therecord)->recordEnd();
    return rdef;		
  }
}

dbFieldIterator RootDb::DbProcess(DbRecord* theRecord, dbFieldIterator /*theFields*/, enum DbAccessType /*mode*/){return theRecord->fieldEnd();} // read or commit the record's fields in block transfer

dbFieldIterator RootDb::DbProcess(dbFieldIterator theField, enum DbAccessType mode){
  if(mode != PixDb::DBREAD){
    stringstream a;
    a << "RootDb::DbProcess: this call can only DBREAD a field!";
    throw PixDBException(a.str());
  }
  
  dbFieldIterator tmpField;
  RootDbField* field = 0;
  if(theField.m_parentRecord != 0) {
    dbFieldIterator rf(theField.m_number, getFieldByDecName(dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number)), theField.m_parentRecord);
    tmpField = rf;
  }
  else{
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is invalid, parent is zero ";
    throw PixDBException(a.str());
  }
  field = dynamic_cast<RootDbField*>(*tmpField);
  if(field == 0){
    stringstream a;
    a << "RootDb::DbProcess: dbFieldIterator is invalid, field " << dynamic_cast<RootDbRecord*>(theField.m_parentRecord)->getFieldName(theField.m_number) << " is not present on this Db";
      throw PixDBException(a.str());
  }
  
  return tmpField;
};

dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, bool& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);} // read or commit the field pointed by the iterator
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, vector<bool>& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, int& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, vector<int>& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, unsigned int & v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, float& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, vector<float>& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, double& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, vector<double>& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, Histo& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode, string& v){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode, v);}
dbRecordIterator RootDb::DbProcess(DbRecord* therecord, enum DbAccessType mode){dbRecordIterator r(therecord); return RootDb::DbProcess(r,mode);} // read or commit (possibly recursively) the record pointed by iterator
dbFieldIterator RootDb::DbProcess(DbField* theField, enum DbAccessType mode){dbFieldIterator f(theField); return RootDb::DbProcess(f,mode);}

DbRecord* RootDb::DbFindRecordByName(const string& name) {
  DbRecord* retvalue = 0;
  TDirectory* dir = gDirectory; // copy locally the current directory
  m_theFile->cd(); // go to the root directory
  retvalue = getRecordByDecName(name);
  if(retvalue == 0){
    stringstream a;
    a << "RootDb::DbFindRecordByName(): record" << name.c_str() << " not found";
    throw PixDBException(a.str());
  }
  dir->cd();
  return retvalue;
} // find a record by its name, returning a vector of records which fulfill the find requests

DbField* RootDb::DbFindFieldByName(const string& name) {
  DbField* retvalue = 0; 
  TDirectory* dir = gDirectory; // copy locally the current directory
  m_theFile->cd(); // go to the root directory
  retvalue = getFieldByDecName(name);
  if(retvalue == 0){
    stringstream a;
    a << "RootDb::DbFindFieldByName(): field" << name.c_str() << " not found";
    throw PixDBException(a.str());
  }
  dir->cd();
  return retvalue;
} // find a data field by its name, returning a vector of fields which fulfill the find requests

RootDb::RootDb(string namefile, string mode, string tdaqNameFile){	
  if(tdaqNameFile == "") tdaqNameFile = namefile;
  //create a RootDb from
  // a) a .cfg file used as template
  // b) a 
  m_theLoadedFields.SetOwner();
  m_theLoadedRecords.SetOwner();
  if (mode == "NEW" || mode == "RECREATE"){
    // part 1: opening the root file
    m_theFile = new TFile(namefile.c_str(), mode.c_str());
    m_theFileName = namefile;
    if(m_theFile->IsZombie()) 
      throw PixDBException("RootDb constructor: Application description file not open as good");
    m_theFile->cd();
    std::pair<std::string, TFile*> insPair(namefile, m_theFile);
    m_theFileHandlers.insert(insPair);
    
    // part 2: creating the root record
    string rootDecName = m_theFileName + ":/rootRecord;1";
    RootDbRecord* rootrec = dynamic_cast<RootDbRecord*>(makeRecord("rootRecord","/")); 
    rootrec->m_decName = "";
    rootrec->m_myDecName = rootDecName;
    // part 3: committing the newly created rootrecord into the database
    this->DbProcess(rootrec,DBCOMMIT);
    // part 4: closing and reopening in "UPDATE" mode the newly created database
    m_theFile->Close();
    m_theFileHandlers.clear();
    m_theFile = new TFile(namefile.c_str(),"UPDATE");
    if(m_theFile->IsZombie()) 
      throw PixDBException("RootDb constructor: Application description file not open as good");
    std::pair<std::string, TFile*> insPair2(namefile, m_theFile);
    m_theFileHandlers.insert(insPair2);
  }
  else if (mode == "UPDATE" || mode == "READ"){
    // part 1: opening the root file in the mode specified by the user
    m_theFile = new TFile(namefile.c_str(), mode.c_str());
    m_theFileName = namefile;
    if(m_theFile->IsZombie()) 
      throw PixDBException("RootDb constructor: Application description file not open as good");
    std::pair<std::string, TFile*> insPair2(namefile, m_theFile);
    m_theFileHandlers.insert(insPair2);
  }
  m_theFile->cd();
  gDirectory = m_theFile;
}

RootDb::~RootDb(){
  if(!(m_theFile->IsZombie()))
    m_theFile->Close();
  std::map<std::string, TFile*>::iterator i;
  for(i = m_theFileHandlers.begin(); i != m_theFileHandlers.end(); i++){
    if(!((*i).second->IsZombie()))
      (*i).second->Close();
  }
}
