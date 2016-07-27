/////////////////////////////////////////////////////////////////////
// TurboDaqDB.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 13/10/03  Version 1.0 (GG)
//           Initial release
//
//! Data Base implementation of TurboDaqDB.h
#include "TurboDaqDB.h"
#include <sstream>
#include <iostream>
#include "Bits/Bits.h"
#include <typeinfo>
typedef unsigned int uint;

const int numberofcolumns = 18; // number of columns read in the readoutenable, pream, strobe and hitbus config file
const int numberofrows = 160; // number of rows read in the tdacs and fdacs config file

void TurboDaqDBField::dump(std::ostream& os) const 
{ 
  os << "Begin Field Dump" << std::endl << "Field name: " << m_name << std::endl;
  os << "Data Type: " << DBDataTypeNames[m_dataType] << " value:  ";
  switch(m_dataType){
  case INT:
    os << m_integerContent << std::endl; 
    break;
  case ULINT:
    os << m_ulintegerContent << std::endl; 
    break;
  case FLOAT:
    os << m_floatContent << std::endl; 
    break;
  case STRING:
    os << m_stringContent << std::endl;
    break;
  case VECTORINT:
    size_t i;
    os << "Vector size: " << m_vIntegerContent.size() << std::endl;
    for(i = 0; i < m_vIntegerContent.size(); i++){
      os << m_vIntegerContent[i] << '\t';
      if(!(i % 10)) os << std::endl;
    }
    break;
  case VECTORFLOAT:
    os << "Vector size: " << m_vFloatContent.size() << std::endl;
    for(i = 0; i < m_vFloatContent.size(); i++){
      os << m_vFloatContent[i] << '\t';
      if(!(i % 10)) os << std::endl;
    }
    break;
  case BOOL:
    os << m_boolContent << std::endl;
    break;
  case VECTORBOOL:
    os << "Vector size: " << m_vBoolContent.size() << std::endl;
    for(i = 0; i < m_vBoolContent.size(); i++){
      os << m_vBoolContent[i] << '\t';
      if(!(i % 10)) os << std::endl;
    }
    break;    
  case EMPTY:
    os << "Field is empty - no stored informations" << std::endl;
    break;
  default:
    ostringstream errorstring;
    errorstring << "Error TurboDaqDBField::Dump(): type not correct!!!" << std::endl;
    throw PixDBException(errorstring.str().c_str());
    break;
  }
}
      
void TurboDaqDBInquire::dump(std::ostream& os) const { // Dumps the inquire
  os << "Begin Inquire Dump" << std::endl << "Inquire name: " << m_name << '\n' << "Inquire full decorated name: " << m_decoratedName << "\nInquire decorated name: " << m_myDecName << std::endl;
  os << "Number of data field for this Inquire: " << m_recordFields.size() << std::endl;
  os << "Number of depending inquires for this Inquire: " << m_recordSubRecords.size() << std::endl;
  for(size_t it = 0; it < m_recordFields.size(); it++)
    os << *m_recordFields[it];
}

void TurboDaqDB::getLineDos(std::ifstream& is, string& str){
  bool readline = false;
  while(readline != true){
    if(!is.good()){
      ostringstream errorstring;
      errorstring << "Error TurboDaqDB::getLineDos: not good file status" << std::endl;
      throw PixDBException(errorstring.str().c_str());
    }
    std::getline(is,str);
    if(str.c_str()[str.size()-1] == '\r')
      str.erase(str.size()-1, 1);
    std::stringstream s(str);
    std::string field;
    s >> field;
    if(field != "//"){
      readline = true;
    }
  }
  //  cout << str.c_str() << endl;
}

int TurboDaqDB::columnPairMaskDecode(int mask, int columnPairNumber){

  switch(columnPairNumber){
  case 0:
    return ((mask & 0x001)>>0);
    break;
  case 1:
    return ((mask & 0x002)>>1);
    break;
  case 2:
    return ((mask & 0x004)>>2);
    break;
  case 3:
    return ((mask & 0x008)>>3);
    break;
  case 4:
    return ((mask & 0x010)>>4);
    break;
  case 5:
    return ((mask & 0x020)>>5);
    break;
  case 6:
    return ((mask & 0x040)>>6);
    break;
  case 7:
    return ((mask & 0x080)>>7);
    break;
  case 8:
    return ((mask & 0x100)>>8);
    break;
  default:
    std::stringstream a;
    a << "Request for decoding column pair number illegal in TurboDaqDB::columnPairMaskDecode. Mask value: " << mask;
    a << "Column Pair value: " << columnPairNumber << std::endl;
    throw PixDBException(a.str().c_str());
  }    
  return 1;
}

void TurboDaqDB::intMaskFileDecode(std::vector<int>&vect, int valflag, ifstream* file){
  if(valflag == 128){ // Turbo Daq alternative file found
    int j;
    for(j = 0; j < numberofrows; j++){
      std::string readstring;
      getLineDos(*file,readstring);
      std::stringstream a(readstring);
      int ncols;
      for(ncols = 0; ncols < numberofcolumns; ncols++){
	int val;
	a>> val;
	vect.push_back(val);
      }      
    }
  }
  else{
    for(int h = 0; h < numberofcolumns * numberofrows; h++){
      vect.push_back(valflag);
    }
  }   
}

void TurboDaqDB::boolMaskFileDecode(std::vector<bool>& vect, int valflag, ifstream* file){
  if(valflag == 0) { // Turbo Daq alternative file found
    int j;
    for(int h = 0; h < numberofcolumns * numberofrows; h++)
      vect.push_back(1);
    for(j = 0; j < numberofcolumns; j++){
      std::string readstring;
      getLineDos(*file,readstring);
      std::stringstream a(readstring);
      std::string column;
      a>>column;
      int nrows;
      for(nrows = 0; nrows < 5; nrows++){
	std::string columnmask;
	a >> columnmask;
	Bits b(columnmask.c_str());
	int bcount, irow;
	//for(bcount = b.size()-1; bcount >= 0; bcount--){
	for(bcount=0;bcount<b.size();bcount++){
	  irow = bcount + nrows*b.size();
	  irow = numberofrows-1-irow;
	  vect[j+irow*numberofcolumns] = b[bcount];
	}	
      }
    }
  }
  else{
    if(valflag == 1){ // TurboDaq all off found
      for(int h = 0; h < numberofcolumns * numberofrows; h++)
	vect.push_back(0);
    }   
    else if(valflag == 2){ // TurboDaq all on found
      for(int h = 0; h < numberofcolumns * numberofrows; h++)
	vect.push_back(1);
    }
    else{
      std::stringstream a;
      a << "TurboDaqDB::intMaskFileDecode: Turbo daq code not correct (0, 1 or 2 allowed) " << valflag; 
      throw PixDBException(a.str().c_str());
    }
  }  
}

void TurboDaqDB::insertField(DBInquire* in, TurboDaqDBField* fi)
{
  in->pushField(fi);
  fi->m_myDB = in->getDB();
  m_mapDBFieldNames.insert(std::map<string,TurboDaqDBField*>::value_type(fi->getName(),fi));
}

void TurboDaqDB::insertInquire(DBInquire* in, TurboDaqDBInquire* toin)
{  
    toin->m_decoratedName = in->getDecName();
    toin->m_decoratedName += "/";
    toin->m_decoratedName += toin->getMyDecName();
    ostringstream number;
    string finalnumber = "0";
    number << "_0";
    toin->m_decoratedName += number.str().c_str();
    int count = 0;
    while(m_mapDBInquireNames.find(toin->m_decoratedName) != m_mapDBInquireNames.end()){
      int resize = toin->getDecName().find_last_of("_");
      toin->getDecName().resize(resize+1);
      ostringstream rnumber;
      rnumber << count;
      toin->getDecName() += rnumber.str().c_str();
      count ++;
      finalnumber = rnumber.str().c_str(); 
    }
    toin->getMyDecName() += "_";
    toin->getMyDecName() += finalnumber;
    toin->m_myDB = this;
    in->pushRecord(toin);    
    m_mapDBInquireNames.insert(std::map<string,TurboDaqDBInquire*>::value_type(toin->m_decoratedName,toin));
}

std::vector<std::string> TurboDaqDB::appCfgFileLineDecode(std::string s){
  std::vector<std::string> retVector;
  std::istringstream os(s);
  std::string runnString;
  while(os.good()){
    os >> runnString;
    retVector.push_back(runnString);
  }
  return retVector;
}

DBDataType TurboDaqDB::decodeFieldType(std::string s){
  static std::map<std::string,DBDataType> mymap;
  static bool toinit = true;
  if(toinit){
    for(int it = BOOL; it < EMPTY; it++){
      std::string tmpstring = DBDataTypeNames[it];
      mymap.insert(std::map<std::string,DBDataType>::value_type(tmpstring,DBDataType(it)));
    }
    toinit = false;
  }
  if(mymap.find(s) != mymap.end()){
    return mymap.find(s)->second;
  }
  else{
    std::stringstream a;
    a << "TurboDaqDB::decodedFieldType error: " << s.c_str() << " not a valid type";
    throw PixDBException(a.str().c_str());
  } 
}

void TurboDaqDB::openAppCfgFile(void){
  std::string readString;
  DBInquire* currInquire = 0;
  std::vector<std::string> lineDecoded;
  enum readstatus {BEGIN, END, READINGINQUIRE, READINGFIELD, IDLE} status = IDLE;
  // read the application file name
  while(getLineDos(m_appCfgFile,readString), m_appCfgFile.good()){
    lineDecoded = appCfgFileLineDecode(readString);
    if(lineDecoded.begin() == lineDecoded.end()){
      std::stringstream a;
      a << "TurboDaqDB::openAppCfgFile: error decoding file, found empty line decoding " << readString.c_str(); 
      throw PixDBException(a.str().c_str());
    } 

    if(*lineDecoded.begin() == "BEGININQUIRE"){
      status = BEGIN;
    }
    else if(*lineDecoded.begin() == "ENDINQUIRE"){
      status = END;
    }
    else if(*lineDecoded.begin() == "INQUIRE"){
      status = READINGINQUIRE;
    }
    else if(*lineDecoded.begin() == "FIELD"){
	status = READINGFIELD;
    }

    switch(status){
    case BEGIN:
      if(lineDecoded[1] == "rootRecord"){
	m_rootRecord = new TurboDaqDBInquire("rootRecord", this);
	m_rootRecord->getDecName() = "/rootRecord";
	TurboDaqDBField * db = new TurboDaqDBField("TurboDaqDB Version", STRING);
	db->m_stringContent = m_version;
	insertField(m_rootRecord,db);
	m_rootRecord->m_myDB = this;
	currInquire = m_rootRecord;
      }
      else{
	currInquire = *DBFindRecordByName(BYDECNAME, lineDecoded[1]).begin();
      }
      // cout << "found begin inquire " << currInquire->getName().c_str() << " " << currInquire->getMyDecName().c_str() << " " << currInquire->getDecName().c_str() << std::endl;       
      break; 
    case READINGINQUIRE:
      {TurboDaqDBInquire* in = new TurboDaqDBInquire(lineDecoded[1],this);
      in->getMyDecName() = lineDecoded[2];
      insertInquire(currInquire,in);
      break;}
    case READINGFIELD:
      {TurboDaqDBField* db = new TurboDaqDBField(lineDecoded[1], decodeFieldType(lineDecoded[2]));
      db->m_myDB = this;
      std::istringstream istr(lineDecoded[3]);
      std::vector<std::string>::iterator i;
      switch(decodeFieldType(lineDecoded[2])){
      case INT:
	istr >> db->m_integerContent;
	break;
      case ULINT:
	{std::string s;
	istr >> s;
	stringstream ss;
        if (s.substr(0,2) == "0x") {
	  ss.str(s.substr(2));
	  ss >> std::hex >> db->m_ulintegerContent;
	} else {
	  ss.str(s);
	  ss >> db->m_ulintegerContent;
	}}
	break;
      case STRING:
	istr >> db->m_stringContent;
	break;
      case FLOAT:
	istr >> db->m_floatContent;
	break;
      case BOOL:
	istr >> std::boolalpha >> db->m_boolContent;
	break;
      case VECTORINT:
	i = lineDecoded.begin(); 
	i++; i++; i++;
	for(;i != lineDecoded.end(); i++){
	  int runnInt;
	  std::istringstream runstr(*i);
	  runstr >> runnInt;
	  db->m_vIntegerContent.push_back(runnInt);
	}
	break;
	
      case VECTORFLOAT:
	i = lineDecoded.begin(); 
	i++; i++; i++;
	for(;i != lineDecoded.end(); i++){
	  float runnFloat;
	  std::istringstream runstr(*i);
	  runstr >> runnFloat;
	  db->m_vFloatContent.push_back(runnFloat);
	}
	break;
	
      case VECTORBOOL:
	i = lineDecoded.begin(); 
	i++; i++; i++;
	for(;i != lineDecoded.end(); i++){
	  bool runnBool;
	  std::istringstream runstr(*i);
	  runstr >> runnBool;
	  db->m_vBoolContent.push_back(runnBool);
	}
	break;
	
      default:
	break;
      }
      insertField(currInquire,db);
      // Special case: the field is named TDAQConfigFileName, that means that a TDAQ file for a module has been found and need to be processed, in order to have the 
      // inquires referring to the MCC and the FE's in the module itself
      if(db->getName() == "TDAQConfigFileName"){ 
	string nameConfigFile;
	this->DBProcess(currInquire->findField("TDAQConfigFileName"),READ,nameConfigFile);
	openTurboDaqFiles(nameConfigFile);
	getLineDos(*m_moduleCfgFile,readString);
	getLineDos(*m_moduleCfgFile,readString);
	getLineDos(*m_moduleCfgFile,readString);
	getLineDos(*m_moduleCfgFile,readString);
	if(readString == "1"){ // module
	  readModuleConfig(currInquire, 16);
	}
	else if(readString == "0"){ // single chip
	  readSingleChipConfig(currInquire);
	}
	else throw PixDBException("Module type decoding info not found");	
	delete m_moduleCfgFile;
	//cout << *currInquire;
      } 
      break;}
    case END:
      break;
    default:
      break;
    }    
  }
}

void TurboDaqDB::openTurboDaqFiles(string filename){
  m_moduleCfgFile = new std::ifstream(filename.c_str());
  m_moduleCfgFilePath = filename;
  if(!m_moduleCfgFile->good()){
    throw PixDBException("Module config file not open as good");
  }
}

void TurboDaqDB::readModuleConfig(DBInquire *pixmod, int nofchip){
  string readstring;
  TurboDaqDBField *modfield;
  std::stringstream modstream;
  int modrunnint;
  getLineDos(*m_moduleCfgFile, readstring);
  getLineDos(*m_moduleCfgFile, readstring); // here we read the MCC type
  modstream.str(readstring); 
  modstream >> modrunnint; modstream.clear();
  modfield= new TurboDaqDBField("MCC flavour", INT);
  modfield->m_integerContent = modrunnint;
  int mccflavour = modrunnint;
  insertField(pixmod, modfield);

  getLineDos(*m_moduleCfgFile, readstring);
  getLineDos(*m_moduleCfgFile, readstring); // here we read the FE type
  modstream.str(readstring); 
  modstream >> modrunnint; modstream.clear();
  modfield= new TurboDaqDBField("FE flavour", INT);
  modfield->m_integerContent = modrunnint;
  int feflavour = modrunnint;
  insertField(pixmod, modfield);

  if(nofchip==16){ // otherwise we can't be dealing with a MCC module
    TurboDaqDBInquire *mcc = new TurboDaqDBInquire("PixMcc", this); // here to put the decoding for MCC-I2
    modfield = new TurboDaqDBField("ClassInfo_ClassName",STRING);
    if(mccflavour == 1) {
      modfield->m_stringContent = "PixMccI1";
    }
    else if(mccflavour == 2){
      modfield->m_stringContent = "PixMccI2";
    }
    else{
      stringstream a;
      a << "TurboDaqDB::readModuleConfig: mcc flavour not decoded. Value = " << mccflavour;
      throw PixDBException(a.str().c_str());
    }
    insertField(mcc,modfield);
    insertInquire(pixmod, mcc);
  }

  // variables used to temporary store the information from config file
  vector<string> rmname; rmname.resize(nofchip); 
  vector<string> smname; smname.resize(nofchip); 
  vector<string> pkname; pkname.resize(nofchip); 
  vector<string> hmname; hmname.resize(nofchip); 
  vector<string> tdname; tdname.resize(nofchip); 
  vector<string> fdname; fdname.resize(nofchip); 


  int runnInt;
  string runString;
  TurboDaqDBField *fefi;
  std::stringstream a(readstring); // Chip 0 geographical address (0-15)

  // scan the config file
  for(int i = 0; i < nofchip; i++){
    TurboDaqDBInquire *fei = new TurboDaqDBInquire("PixFe", this);

    for(int j = 0; j < 4; j++) getLineDos(*m_moduleCfgFile,readstring);

  modfield = new TurboDaqDBField("ClassInfo_ClassName",STRING);
  if(feflavour == 0) {
	  modfield->m_stringContent = "PixFeI1";
  }
  else if(feflavour == 1){
	  modfield->m_stringContent = "PixFeI2";
  }
  else{
	  stringstream a;
	  a << "TurboDaqDB::readModuleConfig: fe flavour not decoded. Value = " << feflavour;
	  	throw PixDBException(a.str().c_str());
  }

	insertField(fei,modfield);

    getLineDos(*m_moduleCfgFile,readstring);
    a.str(readstring); 
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("geographical address", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 global configuration enable (0 = off, 1 = on)
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("global configuration enable", BOOL);
    fefi->m_boolContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 global scan/readout enable (0 = off, 1 = on)
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("global scan_readout enable", BOOL);
    fefi->m_boolContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 DACs enable (0 = off, 1 = on)
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DACs enable", BOOL);
    fefi->m_boolContent = runnInt;
    insertField(fei, fefi);

	if(feflavour == 1){
		getLineDos(*m_moduleCfgFile,readstring); 
		getLineDos(*m_moduleCfgFile,readstring); // Chip 0 Global Threshold DAC
		a.str(readstring);
		a>>runnInt; a.clear();
		fefi = new TurboDaqDBField("GLOBAL_DAC", INT);
		fefi->m_integerContent = runnInt;
		insertField(fei, fefi);
	}

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 IVDD2 DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_IVDD2", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 ID DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_ID", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 IP2 DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_IP2", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 IP DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_IP", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 TRIMT DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_ITRIMTH", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 IF DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_IF", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 TRIMF DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_ITRIMIF", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 ITH1 DAC 
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_ITH1", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 ITH2 DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_ITH2", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 IL DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_IL", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 IL2 DAC
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("DAC_IL2", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 column-pair mask parameter
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("ENABLE_CP0", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,0);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP1", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,1);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP2", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,2);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP3", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,3);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP4", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,4);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP5", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,5);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP6", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,6);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP7", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,7);
    insertField(fei, fefi);
    fefi = new TurboDaqDBField("ENABLE_CP8", INT);
    fefi->m_integerContent = columnPairMaskDecode(runnInt,8);
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 Timestamp enable (0 = OFF, 1 = ON)
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("ENABLE_TIMESTAMP", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 Decoupling capacitor enable (0 = OFF, 1 = ON)
    a.str(readstring);
    a>>runnInt; a.clear();
    fefi = new TurboDaqDBField("ENABLE_CAP_TEST", INT);
    fefi->m_integerContent = runnInt;
    insertField(fei, fefi);

    ifstream * ifp = 0;

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 readout mask mode (see end for details)
    a.str(readstring);
    a>>runnInt; a.clear();
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 readout mask file (if appl.)
    a.str(readstring);
    a>>rmname[i]; a.clear(); //cout << rmname[i].c_str() << endl;
    if(runnInt == 0){
      string full_path = getFullPath(rmname[i]);
      ifp = new ifstream();
      ifp->open(full_path.c_str());
      if(!ifp->good()){
	std::stringstream a;
	a << "Chip " << i << " ";
	a << "readout mask file not open as good: " << full_path.c_str();
	throw PixDBException(a.str().c_str());
      }
    }
    fefi = new TurboDaqDBField("ENABLE", VECTORBOOL);
    std::vector<bool> readout;
    boolMaskFileDecode(readout, runnInt, ifp);
    if(runnInt == 0) delete ifp;
    fefi->m_vBoolContent = readout;
    insertField(fei, fefi);

    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 strobe mask mode (see end for details)
    a.str(readstring);
    a>>runnInt; a.clear();
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 strobe mask file (if appl.)
    a.str(readstring);
    a>>smname[i]; a.clear();
    if(runnInt == 0){
      string full_path = getFullPath(smname[i]);
      ifp = new ifstream();
      ifp->open(full_path.c_str());
      if(!ifp->good()){
	std::stringstream a;
	a << "Chip " << i << " ";
	a << "strobe mask file not open as good: " << full_path.c_str();
	throw PixDBException(a.str().c_str());
      }
    }    
    fefi = new TurboDaqDBField("SELECT", VECTORBOOL);
    std::vector<bool> strobe;
    boolMaskFileDecode(strobe, runnInt, ifp);
    if (runnInt == 0) delete ifp;
    fefi->m_vBoolContent = strobe;
    insertField(fei, fefi);


    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 preamp kill mask mode (FE-I only)
    a.str(readstring);
    a>>runnInt; a.clear();
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 preamp mask file (if appl.)
    a.str(readstring);
    a>>pkname[i]; a.clear();
    if(runnInt == 0){
      string full_path = getFullPath(pkname[i]);
      ifp = new ifstream();
      ifp->open(full_path.c_str());
      if(!ifp->good()){
	std::stringstream a;
	a << "Chip " << i << " ";
	a << "preamp mask file not open as good: " << full_path.c_str();
	throw PixDBException(a.str().c_str());
      }
    }
    fefi = new TurboDaqDBField("PREAMP", VECTORBOOL);
    std::vector<bool> preamp;
    boolMaskFileDecode(preamp, runnInt, ifp);
    if (runnInt == 0) delete ifp;
    fefi->m_vBoolContent = preamp;
    insertField(fei, fefi);


    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 hitbus mask mode (FE-I only)
    a.str(readstring);
    a>>runnInt; a.clear();
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 hitbus mask file (if appl.)
    a.str(readstring);
    a>>hmname[i]; a.clear();
    if(runnInt == 0){
      string full_path = getFullPath(hmname[i]);
      ifp = new ifstream();
      ifp->open(full_path.c_str());
      if(!ifp->good()){
	std::stringstream a;
	a << "Chip " << i << " ";
	a << "hitbus mask file not open as good: " << full_path.c_str();
	throw PixDBException(a.str().c_str());
      }
    }
    fefi = new TurboDaqDBField("HITBUS", VECTORBOOL);
    std::vector<bool> hitbus;
    boolMaskFileDecode(hitbus, runnInt, ifp);
    if (runnInt == 0) delete ifp;
    fefi->m_vBoolContent = hitbus;
    insertField(fei, fefi);


    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 TDAC mode (n = all n (for n = 0-31), 32 = alternative file)
    a.str(readstring);
    a>>runnInt; a.clear();
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 TDAC file
    a.str(readstring);
    a>>tdname[i]; a.clear();
    if(runnInt == 128){
      string full_path = getFullPath(tdname[i]);
      ifp = new ifstream();
      ifp->open(full_path.c_str());
      if(!ifp->good()){
	std::stringstream a;
	a << "Chip " << i << " ";
	a << "TDAC file not open as good: " << full_path.c_str();
	throw PixDBException(a.str().c_str());
      }
    }
    fefi = new TurboDaqDBField("TDAC", VECTORINT);
    std::vector<int> tdac;
    intMaskFileDecode(tdac, runnInt, ifp);
    if (runnInt == 128) delete ifp;
    fefi->m_vIntegerContent = tdac;
    insertField(fei, fefi);


    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 FDAC mode (n = all n (for n = 0-31), 32 = alternative file)
    a.str(readstring);
    a>>runnInt; a.clear();
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 FDAC file
    a.str(readstring);
    a>>fdname[i]; a.clear();
    if(runnInt == 128){
      string full_path = getFullPath(fdname[i]);
      ifp = new ifstream();
      ifp->open(full_path.c_str());
      if(!ifp->good()){
	std::stringstream a;
	a << "Chip " << i << " ";
	a << "FDAC file not open as good: " << full_path.c_str();
	throw PixDBException(a.str().c_str());
      }
    }
    fefi = new TurboDaqDBField("FDAC", VECTORINT);
    std::vector<int> fdac;
    intMaskFileDecode(fdac, runnInt, ifp);
    if (runnInt == 128) delete ifp;
    fefi->m_vIntegerContent = fdac;
    insertField(fei, fefi);
 
	
    insertInquire(pixmod, fei);
  }

  getLineDos(*m_moduleCfgFile, readstring);
  getLineDos(*m_moduleCfgFile, readstring);
  getLineDos(*m_moduleCfgFile, readstring); 

  float runnFloat;
  recordIterator it = pixmod->recordBegin();if(nofchip==16) it++;  
  {for(int k = 0; k < nofchip; k++, it++){
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip k Cinj-LO (fF)
    a.str(readstring);
    a>>runnFloat; a.clear();
    fefi = new TurboDaqDBField("Cinj-LO", FLOAT);
    fefi->m_floatContent = runnFloat;
    insertField(*it, fefi);
  }}    

  it = pixmod->recordBegin();if(nofchip==16) it++;  
  {for(int k = 0; k < nofchip; k++, it++){
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip k Cinj-HI (fF)
    a.str(readstring);
    a>>runnFloat; a.clear();
    fefi = new TurboDaqDBField("Cinj-HI", FLOAT);
    fefi->m_floatContent = runnFloat;
    insertField(*it, fefi);
  }}    

  it = pixmod->recordBegin();if(nofchip==16) it++;  
  {for(int k = 0; k < nofchip; k++, it++){
    bool isCubicFit=false;
    getLineDos(*m_moduleCfgFile,readstring); 
    if((int)readstring.find("VCAL-FE coefficients")!=(int)std::string::npos)
      isCubicFit = true;
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 VCAL-FE gradient (mV/count)
    a.str(readstring);
    float c0, c1, c2, c3;
    if(isCubicFit){ // new TurboDAQ format has slope, cubic and quadratic fit pars in one row
      a.str(readstring);
      a>>c3>>c2>>c1>>c0; a.clear();
    } else { 
      a>>c1; a.clear();
      c0 = c2 = c3 = 0.0;
    }
    fefi = new TurboDaqDBField("VCAL-FE gradient 0", FLOAT);
    fefi->m_floatContent = c0;
    insertField(*it, fefi);
    fefi = new TurboDaqDBField("VCAL-FE gradient 1", FLOAT);
    fefi->m_floatContent = c1;
    insertField(*it, fefi);
    fefi = new TurboDaqDBField("VCAL-FE gradient 2", FLOAT);
    fefi->m_floatContent = c2;
    insertField(*it, fefi);
    fefi = new TurboDaqDBField("VCAL-FE gradient 3", FLOAT);
    fefi->m_floatContent = c3;
    insertField(*it, fefi);
  }}    

  it = pixmod->recordBegin();if(nofchip==16) it++;  
  for(int k = 0; k < nofchip; k++, it++){
    getLineDos(*m_moduleCfgFile,readstring); 
    getLineDos(*m_moduleCfgFile,readstring); // Chip 0 Internal-injection offset correction (VCAL-FE counts)
    a.str(readstring);
    a>>runnFloat; a.clear();
    fefi = new TurboDaqDBField("Internal-injection offset correction", FLOAT);
    fefi->m_floatContent = runnFloat;
    insertField(*it, fefi);
  }   

  // two empty lines and TPLL XCKr phase
  getLineDos(*m_moduleCfgFile,readstring); 
  getLineDos(*m_moduleCfgFile,readstring); 
  getLineDos(*m_moduleCfgFile,readstring); 
  getLineDos(*m_moduleCfgFile,readstring); 

  if(nofchip==16){ // otherwise we can't be dealing with a MCC module
    it = pixmod->recordBegin();  
    for(int k = 0; k < 16; k++){
      getLineDos(*m_moduleCfgFile,readstring); 
      getLineDos(*m_moduleCfgFile,readstring); // MCC CAL strobe-delay range 0: calibration factor (ns/count)
      a.str(readstring);
      a>>runnFloat; a.clear();
      std::string name = "MCC CAL strobe-delay range ";
      stringstream b;
      b << k;
      name += b.str(); 
      fefi = new TurboDaqDBField(name.c_str(), FLOAT);
      fefi->m_floatContent = runnFloat;
      insertField(*it, fefi);
    }
  }    
}
void TurboDaqDB::readSingleChipConfig(DBInquire *pixmod){
  readModuleConfig(pixmod, 1);
}


TurboDaqDB::TurboDaqDB() :  m_version("1.0"), m_moduleCfgFilePath(".") , m_appCfgFile(""), m_rootRecord(0) {
}

TurboDaqDB::TurboDaqDB(string namefile) :  m_version("1.0"), m_moduleCfgFilePath(".") , m_appCfgFile(namefile.c_str()), m_rootRecord(0)  {
	
  // part 1: opening the turboDAQ configuration files
  if(!m_appCfgFile.good()) 
  {
	  stringstream a;
	  a << "TurboDaqDB constructor: Application description file not open as good: ";
	  a << namefile;
	  throw PixDBException(a.str().c_str());
  }	
  //part 2: creating the tree and map structures
  openAppCfgFile();
}

TurboDaqDB::TurboDaqDB(string namefile, int option) : m_version("1.0"), m_moduleCfgFilePath(".") , m_appCfgFile(namefile.c_str()), m_rootRecord(0){ // if option == 1, TurboDaqDB actually read only the turbodaq configuration file
  if(!m_appCfgFile.good()) 
  {
	  stringstream a;
	  a << "TurboDaqDB constructor: Application description file not open as good: ";
	  a << namefile;
    throw PixDBException(a.str().c_str());
  }
  if(option != 1)
    throw PixDBException("TurboDaqDB constructor: only option = 1 supported");

  m_rootRecord = new TurboDaqDBInquire("rootRecord", this);
  m_rootRecord->getDecName() = "/rootRecord";
  TurboDaqDBField * db = new TurboDaqDBField("TurboDaqDB Version", STRING);
  db->m_stringContent = m_version;
  insertField(m_rootRecord,db);
  m_rootRecord->m_myDB = this;

  openTurboDaqFiles(namefile);
  std::string readString;
  getLineDos(*m_moduleCfgFile,readString);
  getLineDos(*m_moduleCfgFile,readString);
  getLineDos(*m_moduleCfgFile,readString);
  getLineDos(*m_moduleCfgFile,readString);
  if(readString == "1"){ // module
	  readModuleConfig(m_rootRecord, 16);
  }
  else if(readString == "0"){ // single chip
	  readSingleChipConfig(m_rootRecord);
  }
  else throw PixDBException("Module type decoding info not found");	
  delete m_moduleCfgFile;

}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, string& s){  
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, string& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, string& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == STRING || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, string& s) : mode COMMIT for a non-string and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_stringContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = STRING;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != STRING){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, string& s) : mode READ for a non-string or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_stringContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, string& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int & s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == ULINT || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int& s) : mode COMMIT for a non-long int and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_ulintegerContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = ULINT;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != ULINT){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int& s) : mode READ for a non-int or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_ulintegerContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, unsigned int& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, int& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, int& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, int& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == INT || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, int& s) : mode COMMIT for a non-int and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_integerContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = INT;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != INT){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, int& s) : mode READ for a non-int or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_integerContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, int& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == VECTORINT || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& s) : mode COMMIT for a non-vector<int> and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_vIntegerContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = VECTORINT;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != VECTORINT){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& s) : mode READ for a non-vector<int> or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_vIntegerContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<int>& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, float& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, float& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, float& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == FLOAT || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, float& s) : mode COMMIT for a non-float and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_floatContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = FLOAT;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != FLOAT){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, float& s) : mode READ for a non-float or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_floatContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, float& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, double& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, double& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, double& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == DOUBLE || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, double& s) : mode COMMIT for a non-double and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_doubleContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = DOUBLE;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != DOUBLE){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, double& s) : mode READ for a non-double or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_doubleContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, double& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, bool& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, bool& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, bool& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == BOOL || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, bool& s) : mode COMMIT for a non-bool and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_boolContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = BOOL;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != BOOL){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, bool& s) : mode READ for a non-bool or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_boolContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, bool& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == VECTORFLOAT || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& s) : mode COMMIT for a non-vector<float> and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_vFloatContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = VECTORFLOAT;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != VECTORFLOAT){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& s) : mode READ for a non-vector<float> or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_vFloatContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<float>& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == VECTORDOUBLE || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& s) : mode COMMIT for a non-vector<double> and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_vDoubleContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = VECTORDOUBLE;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != VECTORDOUBLE){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& s) : mode READ for a non-vector<double> or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_vDoubleContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<double>& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& s){
  TurboDaqDBField* fi = 0;
  fi = dynamic_cast<TurboDaqDBField*>(*theField);
  if(!fi){
    throw PixDBException("TurboDaqDB::DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& s) : trying to process with a TurboDaqDB database a non-TurboDaqDB field");
    return false;
  }
  switch(mode){
  case COMMITTREE:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& s) : mode COMMITTREE selected not available for this method");
    return false;
    break;
  case COMMIT:
    if(!(fi->m_dataType == VECTORBOOL || fi->m_dataType == EMPTY)){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& s) : mode COMMIT for a non-vector<bool> and non-empty field");
      return false;
      break;
    }
    else{
      fi->m_vBoolContent = s;
      if(fi->m_dataType == EMPTY){
	fi->m_dataType = VECTORBOOL;
      }
    }
    break;
  case READ:
    if(fi->m_dataType == EMPTY || fi->m_dataType != VECTORBOOL){
      throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& s) : mode READ for a non-vector<bool> or empty field");
      return false;
      break;
    }
    else{
      s = fi->m_vBoolContent;
      break;
    }		
  default:
    throw PixDBException("DBProcess(fieldIterator theField, enum DBInquireType mode, vector<bool>& s) : mode unknown");
    return false;
    break;
  };
  return true;
}

bool TurboDaqDB::DBProcess(recordIterator /*therecord*/, enum DBInquireType /*mode*/){
  return true;
}

void TurboDaqDB::tag(DBInquire* /*therecord*/, enum DBInquireTagMode, string /*tagname*/){// tag the configuration of inquire pointed by iterator 
}

recordIterator TurboDaqDBInquire::findRecord(const char* recordName) const {
  std::string s(recordName);
  return findRecord(s);
}

std::vector<std::string> decomposeRecordName(string recordName){
        int pos = (int)recordName.find_first_of("/");
	std::vector<std::string> retval;

	if(pos != (int)std::string::npos){
		retval.push_back(recordName.substr(recordName.find_first_of("/")+1,recordName.length()));
		retval.push_back(recordName.substr(0,recordName.find_first_of("/")));
	}
	else retval.push_back(recordName);

	return retval;
}

recordIterator TurboDaqDBInquire::findRecord(const string recordName) const {
	if(decomposeRecordName(recordName).size() == 1){
		uint it;
		for(it = 0; it < m_recordSubRecords.size(); it++)
		{
			if(m_recordSubRecords[it]->getName() == recordName) break;
		}
		if(it == m_recordSubRecords.size()){
			//    std::stringstream a;
			//    a << recordName.c_str() << " not found in method FindRecord";
			//    throw PixDBException(a.str().c_str());    
			return recordEnd();
		}
	        const DBInquire* dummy = this;
		recordIterator r(it, m_recordSubRecords[it], dummy);
		return r;
	}
	else{
		uint it;
		for(it = 0; it < m_recordSubRecords.size(); it++)
		{
			if(m_recordSubRecords[it]->getName() == decomposeRecordName(recordName)[0] && 
				m_recordSubRecords[it]->getMyDecName() == decomposeRecordName(recordName)[1]) break;
		}
		if(it == m_recordSubRecords.size()){
			//    std::stringstream a;
			//    a << recordName.c_str() << " not found in method FindRecord";
			//    throw PixDBException(a.str().c_str());    
			return recordEnd();
		}
		recordIterator r(it, m_recordSubRecords[it], this);
		return r;
	}
} // find a subrecord by its name

fieldIterator TurboDaqDBInquire::findField(const char* fieldName) const {
  std::string s(fieldName);
  return findField(s);
}

fieldIterator TurboDaqDBInquire::findField(const string fieldName) const {
  uint it;
  for(it = 0; it < m_recordFields.size(); it++)
    {
      if(m_recordFields[it]->getName() == fieldName) break;
    }
  if(it == m_recordFields.size()){
    // std::stringstream a;
    // a << fieldName.c_str() << " not found in method findField";
    //return 0;
    // throw PixDBException(a.str().c_str());    
	  return fieldEnd();
  }
  fieldIterator f(it, m_recordFields[it], this);
  return f;
} // find a field by its name

DBInquire* TurboDaqDBInquire::addInquire(string name, string dirName) {
	TurboDaqDBInquire *newInq = new TurboDaqDBInquire(name, m_myDB);
	newInq->m_decoratedName = getDecName()+dirName;
	newInq->m_myDecName = getDecName()+dirName + "/" + name;
	return newInq;
}


std::vector<DBField*> TurboDaqDB::DBFindFieldByName(enum DBFindMode /*mode*/, const std::string& /*name*/){
  std::vector<DBField*> r;
  return r;
}


std::vector<DBInquire*> TurboDaqDB::DBFindRecordByName(enum DBFindMode mode, const std::string& name){
  std::map<std::string,TurboDaqDBInquire*>::iterator it;
  std::multimap<std::string,TurboDaqDBInquire*>::iterator mit;
  std::vector<DBInquire*> retVector;

  switch (mode){
  case BYDECNAME:
    it = m_mapDBInquireNames.find(name);
    if(it != m_mapDBInquireNames.end()){
      DBInquire* rit = it->second;
      retVector.push_back(rit);
    }
    break;
  case BYNAME:
    if((mit = m_mapDBInquireSimpleNames.find(name)) != m_mapDBInquireSimpleNames.end()){
      std::multimap<std::string, TurboDaqDBInquire*>::iterator eit = m_mapDBInquireSimpleNames.upper_bound(name);
      for(;mit != eit; mit++){
	DBInquire* rit = mit->second;
	retVector.push_back(rit);
      }
    }
    break;
  default:
    return retVector;
  }

  return retVector;
}

string TurboDaqDB::getFullPath(string relPath){
  string newPath = relPath, basePath=m_moduleCfgFilePath, testName;
  unsigned long int pos;
  // skip config file-name part of base path
  pos = basePath.find_last_of('/');
  if(pos!=(unsigned long int)string::npos) basePath.erase(pos,basePath.length()-pos);
  // skip "config" part of base path
  pos = basePath.find_last_of('/');
  if(pos!=(unsigned long int)string::npos) basePath.erase(pos,basePath.length()-pos);
  // now skip module part of base path, but keep last "/"
  pos = basePath.find_last_of('/');
  if(pos!=(unsigned long int)string::npos) basePath.erase(pos+1,basePath.length()-pos);
  else basePath="";
  // then add relative path of DAC or mask file
  newPath = basePath + newPath;
  return newPath;
}

DBField* TurboDaqDB::makeField(string name){
	TurboDaqDBField* retVal =  new TurboDaqDBField(name);
	retVal->m_myDB = this;
	return retVal;
}
DBInquire* TurboDaqDB::makeInquire(string name, string decName){
	TurboDaqDBInquire* retVal = new TurboDaqDBInquire(name, this);
	retVal->m_decoratedName = decName;
	retVal->m_myDecName = decName + "/";
	retVal->m_myDecName += name;
	return retVal;
}

TurboDaqDBInquire::TurboDaqDBInquire(string n, PixConfDBInterface* db) : m_name(n), m_decoratedName(""), m_myDecName(n) {m_myDB = db;}  

int TurboDaqDBInquire::getDependingInquireSize(void) const{ // Get the amount of depending inquires
  return m_recordSubRecords.size();
}

int TurboDaqDBInquire::getDependingFieldSize(void) const{ // Get the amount of depending field
  return m_recordFields.size();
}

fieldIterator TurboDaqDBInquire::getField(int fieldNumber) const{ // Get the pointer to the field number fieldNumber; throw an exception if fails
  uint fn = fieldNumber;
  if(fn > m_recordFields.size()){
    std::stringstream a;
    a << "TurboDaqDBInquire::getField(int fieldNumber): field number " << fn << " not found in inquire " << m_decoratedName.c_str();
    throw PixDBException(a.str().c_str());    
  }
  else if(fn == m_recordFields.size()){
    fieldIterator r = fieldEnd();
    return r;
  }
  else{
    fieldIterator r(fieldNumber, m_recordFields[fieldNumber], this);
    return r;
  }
}

recordIterator TurboDaqDBInquire::getInquire(int inquireNumber) const { // Get the pointer to the inquire number inquireNumber; throw an exception if fails
  uint in = inquireNumber;
  if(in > m_recordSubRecords.size()){
    std::stringstream a;
    a << "TurboDaqDBInquire::getInquire(int inquireNumber): inquire number " << in << " not found in inquire " << m_decoratedName.c_str();
    throw PixDBException(a.str().c_str());    
  }
  else if(in == m_recordSubRecords.size()){
    return recordEnd();
  }
  else{
    recordIterator r(inquireNumber, m_recordSubRecords[in], this);
    return r;
  }

}


recordIterator TurboDaqDBInquire::recordBegin() const {
  if(m_recordSubRecords.size() == 0) return recordEnd();

  recordIterator r(0,*m_recordSubRecords.begin(),this);
  return r;
}

recordIterator TurboDaqDBInquire::recordEnd() const {
  recordIterator r(m_recordSubRecords.size(), NULL, this);
  return r;
}

fieldIterator TurboDaqDBInquire::fieldBegin() const {
  if(m_recordFields.size() == 0) return fieldEnd();

  fieldIterator r(0,*m_recordFields.begin(), this);
  return r;
}


fieldIterator TurboDaqDBInquire::fieldEnd() const {
  fieldIterator r(m_recordFields.size(),NULL,this);
  return r;
}


fieldIterator TurboDaqDBInquire::pushField(DBField* in) {m_recordFields.push_back(in); return getField(getDependingFieldSize()-1);} // add a field to the DBInquire fields and return the corresponding iterator

void TurboDaqDBInquire::eraseField(fieldIterator /*it*/) {} // erase a field - not implemented.

recordIterator TurboDaqDBInquire::pushRecord(DBInquire* in) {m_recordSubRecords.push_back(in); return getInquire(getDependingInquireSize()-1);} // add a record to the record list and return the corresponding iterator

void TurboDaqDBInquire::eraseRecord(recordIterator /*it*/) {}; // erase a record - not implemented

bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, bool& v) {fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<bool>& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, int& v) {fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<int>& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, unsigned int & v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, float& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<float>& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, double& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, vector<double>& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBField* theField, enum DBInquireType mode, string& v){fieldIterator f(theField); return DBProcess(f,mode, v);}
bool TurboDaqDB::DBProcess(DBInquire* theRecord, enum DBInquireType mode){recordIterator r(theRecord); return DBProcess(r,mode);}
