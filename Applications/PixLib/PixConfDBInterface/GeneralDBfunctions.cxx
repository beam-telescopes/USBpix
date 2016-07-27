#include "GeneralDBfunctions.h"

#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/RootDB.h>
#include <Bits/Bits.h>
#include <Histo/Histo.h>
#include <PixModule/PixModule.h>
#include <Config/ConfMask.h>
#include <Config/ConfObj.h>

#include <sstream>

void PixLib::insertDBInquire(DBInquire *org_inquire, DBInquire *insert_inquire, bool tree, const char *newName)
{
  PixConfDBInterface *myDB = insert_inquire->getDB();
  std::string decName = org_inquire->getDecName();
  if(newName==0)
    getDecNameCore(decName);
  else
    decName = newName;
  decName = insert_inquire->getDecName() + decName;
  DBInquire *new_inq = myDB->makeInquire(org_inquire->getName(),decName);
  insert_inquire->pushRecord(new_inq);
  myDB->DBProcess(insert_inquire,COMMITREPLACE);
  myDB->DBProcess(new_inq,COMMIT);
  if(tree){
    // call resp. function for all depending fields
    for(fieldIterator fit=org_inquire->fieldBegin(); fit!=org_inquire->fieldEnd(); fit++)
      insertDBField(*fit, new_inq);
    // call this function for all depending inquires
    for(recordIterator rit=org_inquire->recordBegin();rit!=org_inquire->recordEnd();rit++)
      insertDBInquire(*rit, new_inq);
  }
  return;
}
void PixLib::insertDBField(DBField *org_field, DBInquire *insert_inquire)
{
  PixConfDBInterface *myDB = insert_inquire->getDB();
  DBField *new_field = myDB->makeField(org_field->getName());

  int i_copyvalue = 0;
  unsigned int ui_copyvalue = 0;
  float f_copyvalue = 0;
  double d_copyvalue = 0;
  bool b_copyvalue = false;
  std::vector<int> vi_copyvalue;
  std::vector<float> vf_copyvalue;
  std::vector<double> vd_copyvalue;
  std::vector<bool> vb_copyvalue;
  std::string s_copyvalue;
  Histo h_copyvalue;
  stringstream a;
  switch(org_field->getDataType()){
  case INT:
    org_field->getDB()->DBProcess(org_field,READ,i_copyvalue);
    myDB->DBProcess(new_field,COMMIT,i_copyvalue);
    break;
  case ULINT:
    org_field->getDB()->DBProcess(org_field,READ,ui_copyvalue);
    myDB->DBProcess(new_field,COMMIT,ui_copyvalue);
    break;
  case FLOAT:
    org_field->getDB()->DBProcess(org_field,READ,f_copyvalue);
    myDB->DBProcess(new_field,COMMIT,f_copyvalue);
    break;
  case DOUBLE:
    org_field->getDB()->DBProcess(org_field,READ,d_copyvalue);
    myDB->DBProcess(new_field,COMMIT,d_copyvalue);
    break;
  case STRING:
    org_field->getDB()->DBProcess(org_field,READ,s_copyvalue);
    myDB->DBProcess(new_field,COMMIT,s_copyvalue);
    break;
  case BOOL:
    org_field->getDB()->DBProcess(org_field,READ,b_copyvalue);
    myDB->DBProcess(new_field,COMMIT,b_copyvalue);
    break;
  case VECTORINT:
    org_field->getDB()->DBProcess(org_field,READ,vi_copyvalue);
    myDB->DBProcess(new_field,COMMIT,vi_copyvalue);
    break;
  case VECTORFLOAT:
    org_field->getDB()->DBProcess(org_field,READ,vf_copyvalue);
    myDB->DBProcess(new_field,COMMIT,vf_copyvalue);
    break;
  case VECTORBOOL:
    org_field->getDB()->DBProcess(org_field,READ,vb_copyvalue);
    myDB->DBProcess(new_field,COMMIT,vb_copyvalue);
    break;
  case VECTORDOUBLE:
    org_field->getDB()->DBProcess(org_field,READ,vd_copyvalue);
    myDB->DBProcess(new_field,COMMIT,vd_copyvalue);
    break;
  case HISTO:
    org_field->getDB()->DBProcess(org_field,READ,h_copyvalue);
    myDB->DBProcess(new_field,COMMIT,h_copyvalue);
    break;
  case EMPTY: default:
    a << "insertDBField : copying empty or not supported data type ";
    a << DBDataTypeNames[org_field->getDataType()];
    throw PixDBException(a.str().c_str());
  }	
  insert_inquire->pushField(new_field);
  myDB->DBProcess(insert_inquire,COMMITREPLACE);
  return;
}
void PixLib::getDecNameCore(std::string &name)
{
  if(name.length()<=0) return; // Nothing to do
  if(name.substr(name.length()-1,1)=="/")
    name.erase(name.length()-1,1);
  int pos = name.find_last_of("/");
  if(pos!=(int)std::string::npos)
    name.erase(0,pos+1);
  return;
}

// original STcontrol/ConfigClasses code

void PixLib::WriteIntConf(ConfInt& myco,int myvalue){
  switch( myco.subtype() ){
  case ConfInt::S32:
    *((int *)myco.m_value) = (int)myvalue;
    break;
  case ConfInt::U32:
    *((unsigned int *)myco.m_value) = (unsigned int)myvalue;
    break;
  case ConfInt::S16:
    *((short int *)myco.m_value) = (short int)myvalue;
    break;
  case ConfInt::U16:
    *((unsigned short int *)myco.m_value) = (unsigned short int)myvalue;
    break;
  case ConfInt::S8:
    *((char *)myco.m_value) = (char)myvalue;
    break;
  case ConfInt::U8:
    *((unsigned char *)myco.m_value) = (unsigned char)myvalue;
    break;
  default: ;
  }
}


// --------------- static RootDB functions start here ---------------------------------
//
// ideally, this should go into PixLib::RootDB at some point; leave here to mature a bit

PixConfDBInterface* PixLib::createEmptyDB(const char *fname){
  // this routine is actually obsolete, keep it for a while for compatibility

  RootDB *myDB = new RootDB(fname,"NEW");
  // re-open file
  delete myDB;
  myDB = new RootDB(fname,"UPDATE");
  return myDB;
}

PixConfDBInterface* PixLib::createDefaultDB(const char *fname, const char *){
  RootDB *myDB = new RootDB(fname,"NEW");
  if(myDB==0){
    throw PixDBException("PixLib::createEmptyDB: Can't open new cfg file "+std::string(fname)+".");
  }
  PixLib::DBInquire *startInq=0, *root;
  //PixLib::recordIterator it;
  
  root = myDB->readRootRecord(1);
  
  if(root==0){
    throw PixDBException("Can't find root record in DB file");
  }
  // create application folder
  std::string name, decName, myDecName;
  // new module group
  name="application";
  decName = root->getDecName()+"TestApp/";
  startInq = myDB->makeInquire(name, decName);
  root->pushRecord(startInq);
  myDB->DBProcess(root,COMMITREPLACE);
  myDB->DBProcess(startInq,COMMIT);
  
  // re-open file
  delete myDB;
  myDB = new RootDB(fname,"UPDATE");
  return myDB;
}

PixConfDBInterface* PixLib::addAppInq(const char *fname, const char *inAppName){
  // check if file exists; create if not
  FILE *f = fopen(fname,"r");
  RootDB *myDB = 0;
  if(f==0){
    myDB = new RootDB(fname, "NEW");
    delete myDB;
  }else
    fclose(f);

  myDB = new RootDB(fname,"UPDATE");
  if(myDB==0){
    throw PixDBException("PixLib::addAppInq: Can't open new cfg file "+std::string(fname)+".");
  }
  PixLib::DBInquire *startInq=0, *root;
  //PixLib::recordIterator it;
  
  root = myDB->readRootRecord(1);
  
  if(root==0){
    throw PixDBException("Can't find root record in DB file");
  }
  // create application folder
  std::string name, decName, myDecName, appName;
  // new module group
  name="application";
  if(inAppName!=0) appName = inAppName;
  else             appName = "TestApp";
  decName = root->getDecName()+appName+"/";
  startInq = myDB->makeInquire(name, decName);
  root->pushRecord(startInq);
  myDB->DBProcess(root,COMMITREPLACE);
  myDB->DBProcess(startInq,COMMIT);
  
  // re-open file
  delete myDB;
  myDB = new RootDB(fname,"UPDATE");
  return myDB;
}
DBInquire* PixLib::findAppInq(PixConfDBInterface* myDB, const char *inAppName){
  if(myDB==0) return 0;
  DBInquire *root = myDB->readRootRecord(1);  
  if(root==0)
    throw PixDBException("Can't find root record.");
  // get application folder - we will write in here
  DBInquire *startInq=0;
  std::string appName, decName;
  if(inAppName!=0) appName = inAppName;
  else             appName = "TestApp";
  decName = myDB->readRootRecord(1)->getDecName();
  decName += appName;
  std::vector<DBInquire*> ti = myDB->DBFindRecordByName(PixLib::BYDECNAME, decName+"/application");
  if(ti.size()==1) startInq=ti[0];
  return startInq;
}
DBInquire* PixLib::newGroupWiz(const char *class_name, const char *grp_name, DBInquire *startInq){
  if(startInq==0){
    throw PixDBException("Supplied application-record is NULL.");
  }
  // get DB pointer
  PixConfDBInterface *myDB = startInq->getDB();

  DBInquire *grpInq, *pcInq;
  DBField *newField;
  std::string name, decName, myDecName;
  // new module group
  name="PixModuleGroup";
  decName = startInq->getDecName() + std::string(grp_name);
  grpInq = myDB->makeInquire(name, decName);
  startInq->pushRecord(grpInq);
  myDB->DBProcess(startInq,COMMITREPLACE);
  myDB->DBProcess(grpInq,COMMIT);
  newField = myDB->makeField("ModuleGroupName");
  name=grp_name;
  myDB->DBProcess(newField,COMMIT,name);
  grpInq->pushField(newField);
  myDB->DBProcess(grpInq,COMMITREPLACE);
  // add PixController
  name="PixController";
  decName = grpInq->getDecName() + std::string(class_name);
  pcInq = myDB->makeInquire(name, decName);
  grpInq->pushRecord(pcInq);
  myDB->DBProcess(grpInq,COMMITREPLACE);
  myDB->DBProcess(pcInq,COMMIT);    
  newField = myDB->makeField("ActualClassName");
  name=class_name;
  myDB->DBProcess(newField,COMMIT,name);
  pcInq->pushField(newField);
  myDB->DBProcess(pcInq,COMMITREPLACE);
  
  return grpInq;
}
DBInquire* PixLib::newGroupWiz(rodData grpDt_in, DBInquire *startInq){
  if(startInq==0){
    throw PixDBException("Supplied application-record is NULL.");
  }
  // get DB pointer
  PixConfDBInterface *myDB = startInq->getDB();

  DBInquire *grpInq, *rpcInq;
  DBField *newField;
  std::string name, decName, myDecName;
  // new module group
  name="PixModuleGroup";
  std::stringstream rs;
  rs << grpDt_in.slot;
  decName = startInq->getDecName() + "RODslot"+rs.str()+"/";
  grpInq = myDB->makeInquire(name, decName);
  startInq->pushRecord(grpInq);
  myDB->DBProcess(startInq,COMMITREPLACE);
  myDB->DBProcess(grpInq,COMMIT);
  newField = myDB->makeField("ModuleGroupName");
  name="SLOT"+rs.str();
  myDB->DBProcess(newField,COMMIT,name);
  grpInq->pushField(newField);
  myDB->DBProcess(grpInq,COMMITREPLACE);
  // add PixController
  name="PixController";
  decName = grpInq->getDecName() + "RodPixController/";
  rpcInq = myDB->makeInquire(name, decName);
  grpInq->pushRecord(rpcInq);
  myDB->DBProcess(grpInq,COMMITREPLACE);
  myDB->DBProcess(rpcInq,COMMIT);    
  // add ROD slot and SRAM files
  newField = myDB->makeField("ActualClassName");
  name="RodPixController";
  myDB->DBProcess(newField,COMMIT,name);
  rpcInq->pushField(newField);
  myDB->DBProcess(rpcInq,COMMITREPLACE);
  newField = myDB->makeField("general_Slot");
  int slot = grpDt_in.slot;
  myDB->DBProcess(newField,COMMIT,slot);
  rpcInq->pushField(newField);
  myDB->DBProcess(rpcInq,COMMITREPLACE);
  newField = myDB->makeField("general_IPRAMFile");
  name=grpDt_in.IPfile;
  myDB->DBProcess(newField,COMMIT,name);
  rpcInq->pushField(newField);
  myDB->DBProcess(rpcInq,COMMITREPLACE);
  newField = myDB->makeField("general_IDRAMFile");
  name=grpDt_in.IDfile;
  myDB->DBProcess(newField,COMMIT,name);
  rpcInq->pushField(newField);
  myDB->DBProcess(rpcInq,COMMITREPLACE);
  newField = myDB->makeField("general_EXTFile");
  name=grpDt_in.Xfile;
  myDB->DBProcess(newField,COMMIT,name);
  rpcInq->pushField(newField);
  myDB->DBProcess(rpcInq,COMMITREPLACE);
  std::string fmtName="fmt_linkMap_";
  for(int fID=0;fID<8;fID++){
    std::stringstream fidStr;
    fidStr << fID;
    newField = myDB->makeField(fmtName+fidStr.str());
    unsigned int fmtVal = grpDt_in.mode;//0x54320000;
    for(int linkID=0;linkID<4;linkID++)
      fmtVal += grpDt_in.fmtLinkMap[fID][linkID]*(16<<(12+4*linkID));
    myDB->DBProcess(newField,COMMIT,fmtVal);
    rpcInq->pushField(newField);
    myDB->DBProcess(rpcInq,COMMITREPLACE);
  }
  
  return grpInq;
}

void PixLib::addBocToGroup(bocData bocDt_in, DBInquire *grpInq){	
  if(grpInq==0) return;
  if(!bocDt_in.haveBoc) return; // this shouldn't have been called
  // get DB pointer
  PixConfDBInterface *myDB = grpInq->getDB();

  DBInquire *bocInq;
  DBField *newField;
  DBInquire *txInq[4];
  DBInquire *rxInq[4];
  std::string name, decName, myDecName;
  int zero = 0;
  char number[3];
  std::string nr;
  std::string txnr, rxnr;
  char txnumber[3], rxnumber[3];
  
  // new module group
  name="PixBoc";
  decName = grpInq->getDecName() + "OpticalBoc/";
  bocInq = myDB->makeInquire(name, decName);
  grpInq->pushRecord(bocInq);
  myDB->DBProcess(grpInq,COMMITREPLACE);
  myDB->DBProcess(bocInq,COMMIT);

  newField = myDB->makeField("General_Mode");
  myDB->DBProcess(newField,COMMIT,bocDt_in.mode);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE); 
  
  newField = myDB->makeField("Clocks_ClockControl");
  myDB->DBProcess(newField,COMMIT,zero);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE);
 
  int bclockinit = 0xc;
  newField = myDB->makeField("Clocks_BRegClockPhase");
  myDB->DBProcess(newField,COMMIT,bclockinit);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE); 

  newField = myDB->makeField("Clocks_BpmClock");
  myDB->DBProcess(newField,COMMIT,zero);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE); 

  newField = myDB->makeField("Clocks_V1Clock");
  myDB->DBProcess(newField,COMMIT,zero);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE); 
  
  newField = myDB->makeField("Clocks_V2Clock");
  myDB->DBProcess(newField,COMMIT,zero);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE); 

  newField = myDB->makeField("Clocks_VfineClock");
  myDB->DBProcess(newField,COMMIT,zero);
  bocInq->pushField(newField);
  myDB->DBProcess(bocInq,COMMITREPLACE); 

  //inquires for 4 tx-boards

  for(int i =0 ;i<4;i++)
    {
      sprintf(txnumber,"%d",i);
      txnr=txnumber;

      decName = bocInq->getDecName() + "TxNr" + txnr + "/";
  
      txInq[i] = myDB->makeInquire("PixTx", decName);
      bocInq->pushRecord(txInq[i]);
      myDB->DBProcess(bocInq,COMMITREPLACE);
      myDB->DBProcess(txInq[i],COMMIT);

      newField = new RootDBField("General_Plugin", myDB);
      if (i==0)
	{
	  myDB->DBProcess(newField,COMMIT,zero);
	}

      if(i==1)
	{
	  int one = 1;
	  myDB->DBProcess(newField,COMMIT,one);
	}
      if(i==2)
	{
	  int two = 2;
	  myDB->DBProcess(newField,COMMIT,two);
	}   
      if(i==3)
	{
	  int three = 3;
	  myDB->DBProcess(newField,COMMIT,three);
	}  

      txInq[i]->pushField(newField);
      myDB->DBProcess(txInq[i],COMMITREPLACE);
      

      for (int j=0; j<8; j++){
	sprintf(number,"%d",j+2);
	nr = number;

	newField = myDB->makeField("Bpm_FineDelay"+nr);
	myDB->DBProcess(newField,COMMIT,zero);
	txInq[i]->pushField(newField);
	myDB->DBProcess(txInq[i],COMMITREPLACE);

	newField= myDB->makeField("Bpm_CoarseDelay"+nr);
	myDB->DBProcess(newField,COMMIT,zero);
	txInq[i]->pushField(newField);
	myDB->DBProcess(txInq[i],COMMITREPLACE);

	newField = myDB->makeField("Bpm_StreamInhibit"+nr);
	myDB->DBProcess(newField,COMMIT,zero);
	txInq[i]->pushField(newField);
	myDB->DBProcess(txInq[i],COMMITREPLACE);  
    
	int MSRinit = 0x13;
	newField = myDB->makeField("Bpm_MarkSpace"+nr);
	myDB->DBProcess(newField,COMMIT,MSRinit);
	txInq[i]->pushField(newField);
	myDB->DBProcess(txInq[i],COMMITREPLACE);  
    
	newField = myDB->makeField("Opt_LaserCurrent"+nr);
	int current=0;
	// check if TX link is used, then turn on by default
	// NB: we count from 0 here, but effectively start at 2 
	for(unsigned int nil=0;nil<bocDt_in.used_in_links[i].size();nil++)
	  if((int)bocDt_in.used_in_links[i][nil]==(j+2)) current=0xa0;
	myDB->DBProcess(newField,COMMIT,current);
	txInq[i]->pushField(newField);
	myDB->DBProcess(txInq[i],COMMITREPLACE);
      }  
    }

  // inquires for 4 rx-boards

  for(int i=0; i<4; i++)
    {
      sprintf(rxnumber,"%d",i);
      rxnr=rxnumber;
      
      decName = bocInq->getDecName() + "RxNr" + rxnr + "/";
      
      rxInq[i] = myDB->makeInquire("PixRx", decName);
      bocInq->pushRecord(rxInq[i]);
      myDB->DBProcess(bocInq,COMMITREPLACE);
      myDB->DBProcess(rxInq[i],COMMIT);

      newField = myDB->makeField("General_Plugin");
    
      if (i==0)
	{
	  myDB->DBProcess(newField,COMMIT,zero);
	}
      if(i==1)
	{
	  int three = 3;
	  myDB->DBProcess(newField,COMMIT,three);
	}
      if(i==2)
	{
	  int four = 4;
	  myDB->DBProcess(newField,COMMIT,four);
	}   
      if(i==3)
	{
	  int seven = 7;
	  myDB->DBProcess(newField,COMMIT,seven);
	}  
      
      rxInq[i]->pushField(newField);
      myDB->DBProcess(rxInq[i],COMMITREPLACE); 

      for (int j=0; j<8; j++){
	sprintf(number,"%d",j+2);
	nr = number;
	
	newField = myDB->makeField("General_DataDelay"+nr);
	myDB->DBProcess(newField,COMMIT,zero);
	rxInq[i]->pushField(newField);
	myDB->DBProcess(rxInq[i],COMMITREPLACE);
	
	int thrinit = 0xff;
	newField= myDB->makeField("Opt_RxThreshold"+nr);
	myDB->DBProcess(newField,COMMIT,thrinit);
	rxInq[i]->pushField(newField);
	myDB->DBProcess(rxInq[i],COMMITREPLACE); 
      }
    }
  return;
}

std::vector<DBInquire*> PixLib::newModWiz(std::vector<modData> modDt_in, DBInquire *grpInq, const char *plPath){

  // get some basic info from DB
  //PixConfDBInterface *myDB = grpInq->getDB();

  std::string my_plPath = plPath;
  std::vector<DBInquire*> retModInq;

  // use Guido's wizard first
  std::vector<std::string> cfgnames;
  for(unsigned int i=0;i<modDt_in.size();i++){
    cfgnames.push_back(modDt_in[i].fname);
  }
  RootDBInquire *rinq = dynamic_cast<RootDBInquire*>(grpInq);
  retModInq = rootGroupModuleWizard(rinq, 1.0f,5.3f,cfgnames,(my_plPath+"/Examples").c_str(),true);
  // then edit module according to information provided by user
  for(unsigned int i=0;i<modDt_in.size();i++){
    // first get file name since module DB entry is identified by this
    std::vector<DBInquire*> modInq = retModInq;
    if(modInq.size()>i){ // found correct DB object, edit that now
      //retModInq.push_back(modInq[i]);
      fixModule(modInq[i], modDt_in[i]);
    }
    
  }
  return retModInq;
}
void PixLib::fixModule(DBInquire *modInq, modData modDt_in){
  // get some basic info from DB
  PixConfDBInterface *myDB = modInq->getDB();
  int new_value;
  std::string new_name;
  fieldIterator f;
  f = modInq->findField("general_GroupId");
  new_value = modDt_in.grpID;
  if(f!=modInq->fieldEnd())
    myDB->DBProcess(*f,COMMIT,new_value);
  f = modInq->findField("general_ModuleId");
  new_value = modDt_in.modID;
  if(f!=modInq->fieldEnd())
    myDB->DBProcess(*f,COMMIT,new_value);
  DBField * newField = myDB->makeField("pp0_position");
  new_value = modDt_in.slot;
  myDB->DBProcess(newField,COMMIT,new_value);
  modInq->pushField(newField);
  newField = myDB->makeField("pp0_Type");
  new_value = modDt_in.pp0;
  if(new_value==PixLib::PixModule::OPTICAL_TEST)
    new_name = "Optical_Test";
  else if(new_value==PixLib::PixModule::OPTICAL)
    new_name = "Optical";
  else if(PixLib::PixModule::ELECTRICAL)
    new_name = "Electrical";
  else
    new_name = "unknown";
  myDB->DBProcess(newField,COMMIT,new_name);
  modInq->pushField(newField);
  f = modInq->findField("general_InputLink");
  new_value = modDt_in.inLink;
  if(f!=modInq->fieldEnd())
    myDB->DBProcess(*f,COMMIT,new_value);
  for(int olID=0;olID<4;olID++){
    std::string oname = "general_OutputLink";
    std::stringstream oss;
    oss << (olID+1);
    oname += oss.str();
    f = modInq->findField(oname);
    new_value = modDt_in.outLink[olID];
    if(f!=modInq->fieldEnd())
      myDB->DBProcess(*f,COMMIT,new_value);
  }
  f = modInq->findField("general_Active");
  new_name = modDt_in.active?"TRUE":"FALSE";
  if(f!=modInq->fieldEnd())
    myDB->DBProcess(*f,COMMIT,new_name);
  if(modDt_in.roType<3){
    new_value = modDt_in.roType;
    // roType:  0=40MHz,                1=80MHz,    2=160MHz    -  but
    // MCC CSR: 0=1x40Mb/s, 1=2x40Mb/s, 2=1x80Mb/s, 3=2x80Mb/s
    if(new_value>0) new_value++;
    recordIterator mccit = modInq->findRecord("PixMcc_0/PixMcc");
    //    DBInquire* myMcc = *(modInq->findRecord("PixMcc_0/PixMcc"));
    if(mccit!=modInq->recordEnd()){
      DBInquire* myMcc = *mccit;
      f = myMcc->findField("Registers_CSR_OutputMode");
      myDB->DBProcess(f,COMMIT, new_value);
      myDB->DBProcess(myMcc,COMMITREPLACE);
    }
  }
  // add dummy entries for geometry
  std::string geomType[3]={"unknown","Stave","Sector"};
  newField = myDB->makeField("geometry_Type");
  myDB->DBProcess(newField,COMMIT,geomType[modDt_in.assyType]);
  modInq->pushField(newField);
  newField = myDB->makeField("geometry_staveID");
  new_value = modDt_in.assyID;
  myDB->DBProcess(newField,COMMIT,new_value);
  modInq->pushField(newField);
  newField = myDB->makeField("geometry_position");
  new_value = modDt_in.pos_on_assy;
  myDB->DBProcess(newField,COMMIT,new_value);
  modInq->pushField(newField);
  newField = myDB->makeField("geometry_connName");
  myDB->DBProcess(newField,COMMIT,modDt_in.connName);
  modInq->pushField(newField);
  // final commit
  myDB->DBProcess(modInq,COMMITREPLACE);
}

DBInquire* PixLib::createEmptyModule(DBInquire *baseInq, std::string modName, int nFe, int nMcc){
  if(baseInq==0) return 0;
  // get some basic info from DB
  RootDB *myDB = dynamic_cast<RootDB*>(baseInq->getDB());
  // build basic module record
  std::string name, decName;
  name="PixModule";
  decName = baseInq->getDecName() + modName;
  DBInquire *modInq = myDB->makeInquire(name, decName);
  baseInq->pushRecord(modInq);
  myDB->DBProcess(baseInq,COMMITREPLACE);
  myDB->DBProcess(modInq,COMMIT);
  // module sub-structure (MCC, FEs)
  DBInquire *subInq = 0;
  for(int i=0;i<nMcc;i++){
    std::stringstream b;
    b << i;
    name="PixMcc";
    decName = modInq->getDecName() + "PixMcc_"+b.str();
    subInq = myDB->makeInquire(name, decName);
    modInq->pushRecord(subInq);
    myDB->DBProcess(modInq,COMMITREPLACE);
    myDB->DBProcess(subInq,COMMIT);
  }
  for(int i=0;i<nFe;i++){
    std::stringstream b;
    b << i;
    name="PixFe";
    decName = modInq->getDecName() + "PixFe_" + b.str();
    subInq = myDB->makeInquire(name, decName);
    modInq->pushRecord(subInq);
    myDB->DBProcess(modInq,COMMITREPLACE);
    myDB->DBProcess(subInq,COMMIT);
    name="GlobalRegister";
    decName = subInq->getDecName() + "GlobalRegister_0";
    DBInquire *dacInq = myDB->makeInquire(name,decName);
    subInq->pushRecord(dacInq);
    myDB->DBProcess(subInq,COMMITREPLACE);
    myDB->DBProcess(dacInq,COMMIT);
    name="PixelRegister";
    decName = subInq->getDecName() + "PixelRegister_0";
    dacInq = myDB->makeInquire(name,decName);
    subInq->pushRecord(dacInq);
    myDB->DBProcess(subInq,COMMITREPLACE);
    myDB->DBProcess(dacInq,COMMIT);
    name="Trim";
    decName = subInq->getDecName() + "Trim_0";
    dacInq = myDB->makeInquire(name,decName);
    subInq->pushRecord(dacInq);
    myDB->DBProcess(subInq,COMMITREPLACE);
    myDB->DBProcess(dacInq,COMMIT);
    myDB->DBProcess(modInq,COMMITREPLACE);
  }

  myDB->DBProcess(modInq,COMMITREPLACE);

  return modInq;
}

DBInquire* PixLib::createEmptyBoc(DBInquire *baseInq, std::string bocName){	
  if(baseInq==0) return 0;
  RootDB *myDB = dynamic_cast<RootDB*>(baseInq->getDB());
  std::string name, decName;
  std::string txnr, rxnr;
  char txnumber[3], rxnumber[3];
  name="PixBoc";
  decName = baseInq->getDecName() + bocName;
  DBInquire *bocInq = myDB->makeInquire(name, decName);
  baseInq->pushRecord(bocInq);
  myDB->DBProcess(baseInq,COMMITREPLACE);
  myDB->DBProcess(bocInq,COMMIT);
  // boc sub-structure (Rx, Tx)
  DBInquire *subInq = 0;
  //inquires for 4 tx-boards
  for(int i =0 ;i<4;i++)
    {
      sprintf(txnumber,"%d",i);
      txnr=txnumber;

      decName = bocInq->getDecName() + "TxNr" + txnr + "/";
      subInq = myDB->makeInquire("PixTx", decName);
      bocInq->pushRecord(subInq);
      myDB->DBProcess(bocInq,COMMITREPLACE);
      myDB->DBProcess(subInq,COMMIT);  
    }  
  

  // inquires for 4 rx-boards

  for(int i=0; i<4; i++)
    {
      sprintf(rxnumber,"%d",i);
      rxnr=rxnumber;
      
      decName = bocInq->getDecName() + "RxNr" + rxnr + "/";
      
      subInq = myDB->makeInquire("PixRx", decName);
      bocInq->pushRecord(subInq);
      myDB->DBProcess(bocInq,COMMITREPLACE);
      myDB->DBProcess(subInq,COMMIT);
    
    }
  myDB->DBProcess(bocInq,COMMITREPLACE);

  return bocInq;
}
void PixLib::listLinks(std::vector<int> &in, std::vector<int> &out1, std::vector<int> &out2,
		       std::vector<int> &out3, std::vector<int> &out4, int setup, int pp0){
  const int n40=28, n80=14, n160=7;

  // for PP0-Type 0 : Opto Test PP0
  int in_40_test[n40]     ={2,6,3,7,4,8,5,
		       14,18,15,19,16,20,17,
		       26,30,27,31,28,32,29,
		       38,42,39,43,40,44,41};
  // use hexadecimal numbers for easier writing of formatter links (eg. 0x3 = 0:3)
  // WARNING: since later algorithm is based on basis of 16, cannot use "normal" data line numbers
  // here (eg. do not use 5 instead of 0x3, Formatter has 12 links, not 16!) 
  int out_40_test[n40]    ={0x0, 0x10, 0x1, 0x11, 0x2, 0x12, 0x3, 
                       0x20, 0x30, 0x21, 0x31, 0x22, 0x32, 0x23,
                       0x40, 0x50, 0x41, 0x51, 0x42, 0x52, 0x43,
                       0x60, 0x70, 0x61, 0x71, 0x62, 0x72, 0x63};  
  int in_80_test[n80]     ={2,6,3,7,4,8,5,38,42,39,43,40,44,41};
  int out1_80_test[n80]   ={0x0,0x20,0x2,0x22,0x10,0x30,0x12, 0x40,0x60,0x42,0x62,0x50,0x70,0x52};
  int out2_80_test[n80]   ={0x1,0x21,0x3,0x23,0x11,0x31,0x13, 0x41,0x61,0x43,0x63,0x51,0x71,0x53};
  int in_160_test[n160]   ={2,6,3,7,4,8,5};
  int out1_160_test[n160] ={0x0,0x40,0x10,0x50,0x20,0x60,0x30};
  int out2_160_test[n160] ={0x1,0x41,0x11,0x51,0x21,0x61,0x31};
  int out3_160_test[n160] ={0x2,0x42,0x12,0x52,0x22,0x62,0x32};
  int out4_160_test[n160] ={0x3,0x43,0x13,0x53,0x23,0x63,0x33};
  
  // for PP0-Type 1 : Opto Final PP0
  int in_40_final[n40]     ={ 5,8,4,7,3,6,2,
                              17,20,16,19,15,18,14,
			      29,32,28,31,27,30,26,
			      41,44,40,43,39,42,38};

  int out_40_final[n40]    ={ 0x3, 0x12, 0x2, 0x11, 0x1, 0x10, 0x0,
			      0x23, 0x32, 0x22, 0x31, 0x21, 0x30, 0x20,
			      0x43, 0x52, 0x42, 0x51, 0x41, 0x50, 0x40,
			      0x63, 0x72, 0x62, 0x71, 0x61, 0x70, 0x60};
  
  int in_80_final[n80]     ={5,8,4,7,3,6,2,41,44,40,43,39,42,38};
  int out1_80_final[n80]   ={0x12,0x30,0x10,0x22,0x2,0x20,0x0, 0x52,0x70,0x50,0x62,0x42,0x60,0x40};
  int out2_80_final[n80]   ={0x13,0x31,0x11,0x23,0x3,0x21,0x1, 0x53,0x71,0x51,0x63,0x43,0x61,0x41};
  int in_160_final[n160]   ={5,8,4,7,3,6,2};
  int out1_160_final[n160] ={0x30,0x60,0x20,0x50,0x10,0x40,0x0};
  int out2_160_final[n160] ={0x31,0x61,0x21,0x51,0x11,0x41,0x1};
  int out3_160_final[n160] ={0x32,0x62,0x22,0x52,0x12,0x42,0x2};
  int out4_160_final[n160] ={0x33,0x63,0x23,0x53,0x13,0x43,0x3};
  
  
  // for PP0-Type 2: eBoc
  int in_40_eboc[n40]     ={5,2,6,3,7,4,8,17,14,18,15,19,16,20,29,
                       26,30,27,31,28,32,41,38,42,39,43,40,44};
  int out_40_eboc[n40]    ={0x3, 0x0, 0x10, 0x1, 0x11, 0x2, 0x12, 
                            0x23, 0x20, 0x30, 0x21, 0x31, 0x22, 0x32,
                            0x43, 0x40, 0x50, 0x41, 0x51, 0x42, 0x52,
                            0x63, 0x60, 0x70, 0x61, 0x71, 0x62, 0x72};
  int in_80_eboc[n80]     ={5,2,6,3,7,4,8,17,14,18,15,19,16,20};
  int out1_80_eboc[n80]   ={0x30,0x0,0x40,0x10,0x50,0x20,0x60,0,0,0,0,0,0,0};
  int out2_80_eboc[n80]   ={0x32,0x2,0x42,0x12,0x52,0x22,0x62,0,0,0,0,0,0,0};
  int in_160_eboc[n160]   ={5,2,6,3,7,4,8};
  int out1_160_eboc[n160] ={0x30,0x0,0x40,0x10,0x50,0x20,0x60};
  int out2_160_eboc[n160] ={0x31,0x1,0x41,0x11,0x51,0x21,0x61};
  int out3_160_eboc[n160] ={0x32,0x2,0x42,0x12,0x52,0x22,0x62};
  int out4_160_eboc[n160] ={0x33,0x3,0x43,0x13,0x53,0x23,0x63};


  int i, n, *ina, *outa1, *outa2, *outa3, *outa4;

  switch(setup){
  case 0:  // 40MHz
  default:
    n    = n40;
    switch(pp0)
      {
      case PixLib::PixModule::OPTICAL_TEST: // opto test
      default:
	ina  = in_40_test;
	outa1= out_40_test;
	outa2= out_40_test;
	outa3= out_40_test;
	outa4= out_40_test;
	break;
      case PixLib::PixModule::OPTICAL: // opto final
	ina  = in_40_final;
	outa1= out_40_final;
	outa2= out_40_final;
	outa3= out_40_final;
	outa4= out_40_final;
	break;
      case PixLib::PixModule::ELECTRICAL: // eboc
	ina  = in_40_eboc;
	outa1= out_40_eboc;
	outa2= out_40_eboc;
	outa3= out_40_eboc;
	outa4= out_40_eboc;
	break;
      }
    break;
  case 1:  // 80MHz
    n    = n80;
    switch(pp0)
      {
      case PixLib::PixModule::OPTICAL_TEST: // opto test
      default:
	ina  = in_80_test;
	outa1= out1_80_test;
	outa2= out2_80_test;
	outa3= out1_80_test;
	outa4= out2_80_test;
	break;
      case PixLib::PixModule::OPTICAL: // opto final
	ina  = in_80_final;
	outa1= out1_80_final;
	outa2= out2_80_final;
	outa3= out1_80_final;
	outa4= out2_80_final;
	break;
      case PixLib::PixModule::ELECTRICAL: // eboc
	ina  = in_80_eboc;
	outa1= out1_80_eboc;
	outa2= out2_80_eboc;
	outa3= out1_80_eboc;
	outa4= out2_80_eboc;
	break;
      }
    break;
  case 2:  // 160MHz
    n    = n160;
    switch(pp0)
      {
      case PixLib::PixModule::OPTICAL_TEST: // opto test
      default:
	ina  = in_160_test;
	outa1= out1_160_test;
	outa2= out2_160_test;
	outa3= out3_160_test;
	outa4= out4_160_test;
	break;
      case PixLib::PixModule::OPTICAL: // opto final
	ina  = in_160_final;
	outa1= out1_160_final;
	outa2= out2_160_final;
	outa3= out3_160_final;
	outa4= out4_160_final;
	break;
      case PixLib::PixModule::ELECTRICAL: // eboc
	ina  = in_160_eboc;
	outa1= out1_160_eboc;
	outa2= out2_160_eboc;
	outa3= out3_160_eboc;
	outa4= out4_160_eboc;
	break;
      }
    break;
  }

  in.clear();
  out1.clear();
  out2.clear();
  out3.clear();
  out4.clear();
  for(i=0;i<n;i++){
    in.push_back(ina[i]);
    out1.push_back(outa1[i]);
    out2.push_back(outa2[i]);
    out3.push_back(outa3[i]);
    out4.push_back(outa4[i]);
  }

  return;
}
/** adds a PixModuleGroup to a loaded DB file */
void PixLib::addGrouptoDB(DBInquire *startInq, grpData inGrpData, const char *pathPL){
  // create group inquire with ROD entries
  PixLib::DBInquire *grpInq = newGroupWiz(inGrpData.myROD,startInq);
  if(inGrpData.myBOC.haveBoc) addBocToGroup(inGrpData.myBOC, grpInq);
  // add modules
//   if(inGrpData.cfgType){ // RootDB type input - temporary
//     for(unsigned int i=0;i<inGrpData.myMods.size();i++){
//       PixConfDBInterface *modDB = DBEdtEngine::openFile(inGrpData.myMods[i].fname.c_str());
//       PixConfDBInterface *myDB = grpInq->getDB();
//       DBInquire *root    = modDB->readRootRecord(1);
//       DBInquire *appInq  = *(root->recordBegin());
//       DBInquire *grpInq2 = *(appInq->recordBegin());
//       DBInquire *modInq  = *(grpInq2->recordBegin());
//       std::string decName;
//       decName = grpInq->getDecName() + inGrpData.myMods[i].modname;
//       DBInquire *modInq2 = myDB->makeInquire("PixModule",decName);
//       grpInq->pushRecord(modInq2);
//       myDB->DBProcess(grpInq,COMMITREPLACE);
//       myDB->DBProcess(modInq2,COMMIT);
//       // call resp. function for all depending fields
//       for(fieldIterator fit=modInq->fieldBegin(); fit!=modInq->fieldEnd(); fit++)
// 	insertDBField(*fit, modInq2);
//       // call this function for all depending inquires
//       for(recordIterator rit=modInq->recordBegin();rit!=modInq->recordEnd();rit++)
// 	insertDBInquire(*rit, modInq2);
//       fixModule(modInq2, inGrpData.myMods[i]);
//       delete modDB;
//     }
//   } else{ // TurboDAQ input
      newModWiz(inGrpData.myMods, grpInq, pathPL);
      //  }
}
