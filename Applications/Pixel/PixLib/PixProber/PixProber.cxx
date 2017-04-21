/////////////////////////////////////////////////////////////////////
// PixProber.cxx
/////////////////////////////////////////////////////////////////////

#include "PixProber/PixProber.h"
#include "PixProber/OpenSuessPixProber.h"
#include "PixProber/SuessPixProber.h"
#include "PixProber/LBNLPixProber.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixProber/AlessiLBNLPixProber.h"
#include "PixProber/CascadePixProber.h"

using namespace PixLib;

PixProber::PixProber(DBInquire *dbInquire) :
  m_dbInquire(dbInquire), m_name("unknown"), m_decName("")
{
  if(dbInquire!=0) m_decName = dbInquire->getDecName();
}

PixProber* PixProber::make(DBInquire *dbInquire, std::string type)
{
  PixProber *prober = 0;
  if (type == "SuessPixProber") {
    prober = new SuessPixProber(dbInquire);
  }
  if (type == "OpenSuessPixProber") {
    prober = new OpenSuessPixProber(dbInquire);
  }
  if (type == "LBNLPixProber") {
    prober = new LBNLPixProber(dbInquire);
  }
  if (type == "DummyPixProber") {
    prober = new DummyPixProber(dbInquire);
  }
  if (type == "AlessiLBNLPixProber") {
    prober = new AlessiLBNLPixProber(dbInquire);
  }
  if (type == "CascadePixProber"){
    prober = new CascadePixProber(dbInquire);
  }
  return prober;  
}

void PixProber::MoveChuck(float x, float y){
  m_posX=x;
  m_posY=y;
} 

void PixProber::MoveChuckZ(float z){
 m_posZ=z;
} 
void PixProber::ReadChuckPosition(float&, float&, float&){//(float &x, float &y, float &z)
}

void PixProber::MoveScope(float x, float y){
  m_posX=x;
  m_posY=y;
} 

void PixProber::MoveScopeZ(float z){
 m_posZ=z;
} 
void PixProber::ReadScopePosition(float&, float&, float&){//(float &x, float &y, float &z)
}


void PixProber::listTypes(std::vector<std::string> &list)
{
  list.push_back("OpenSuessPixProber");
  list.push_back("SuessPixProber");
  list.push_back("LBNLPixProber");
  list.push_back("DummyPixProber");
  list.push_back("AlessiLBNLPixProber");
  list.push_back("CascadePixProber");
  return;
}

// only for testing, no real functionality
DummyPixProber::DummyPixProber(DBInquire *dbInquire) : PixProber(dbInquire)
{
  m_name = "DummyPixProber";
  m_wasInit = false;
  configInit();
  if(dbInquire!=0) m_conf->read(dbInquire);
}

DummyPixProber::~DummyPixProber()
{
  delete m_conf;
}
    
void DummyPixProber::initHW()
{
  m_wasInit = true;
}

void DummyPixProber::configInit()
{
  // Create the Config object
  m_conf = new Config("DummyPixProber"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addInt("dummy", m_dummyCfgVar, 1234,
		  "no meaning", true);
  conf.reset();
}


void DummyPixProber::chuckGotoFirst(){
  m_col = 0;
  m_row = 0;
  m_index = 1;
}

void DummyPixProber::MoveChuck(float x, float y){
  m_x=x;
  m_y=y;
}

void DummyPixProber::ReadChuckPosition(float &x, float &y, float &z){
  x = m_x;
  y = m_y;
  z = m_z;
}

void DummyPixProber::MoveScopeZ(float z){
  m_z=z;
}

void DummyPixProber::MoveScope(float x, float y){
  m_x=x;
  m_y=y;
}

void DummyPixProber::ReadScopePosition(float &x, float &y, float &z){
  x = m_x;
  y = m_y;
  z = m_z;
}

void DummyPixProber::MoveChuckZ(float z){
  m_z=z;
}

void DummyPixProber::chuckContact(){}
void DummyPixProber::chuckSeparate(){}
bool DummyPixProber::chuckNextDie(){
  if (((m_col==7)&&(m_row==4)) || m_index>=61) return true;				//Reached end of wafer.

  m_index++;

  if ((m_col==0)&&(m_row==4))						
    {
      m_col = 1;
      m_row = -1;
      return false;
    }
  if ((m_col==1)&&(m_row==5))
    {
      m_col = 2;
      m_row = -2;
      return false;
    }
  if ((m_col==2)&&(m_row==6))
    {
      m_col = 3;
      m_row = -2;
      return false;
    }
  if ((m_col==3)&&(m_row==6))
    {
      m_col = 4;
      m_row = -2;
      return false;
    }
  if ((m_col==4)&&(m_row==6))
    {
      m_col = 5;
      m_row = -2;
      return false;
    }
  if ((m_col==5)&&(m_row==6))
    {
      m_col = 6;
      m_row = -1;
      return false;
    }
  if ((m_col==6)&&(m_row==5))
    {
      m_col = 7;
      m_row = 0;
      return false;
    }
  m_row++;
  return false;
}
void DummyPixProber::chuckGotoPosition(int col, int row, int ){
  m_col = col;
  m_row = row;  
}
void DummyPixProber::chuckGetPosition(int &col, int &row, int &type){
  col = m_col;
  row = m_row;
  type = 0;
}
void DummyPixProber::chuckGotoIndex(int index){
  m_index = index;
}
void DummyPixProber::chuckGetIndex(int &index){
  index = m_index;
}
