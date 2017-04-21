#include "DBEdtEngine.h"

#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/RootDB.h>

#include <BaseException.h>

#include <TSystem.h>
#include <QString>

DBEdtEngine::DBEdtEngine(QApplication &app) : m_app(app)
{
  m_dbFiles.clear();
  // need $PIX_LIB to find base config files
  const char *plpath = gSystem->Getenv("PIX_LIB");
  if(plpath==0){
    m_plPath = gSystem->pwd();
#ifdef WIN32
    m_plPath+= "\\..\\PixLib";
#else
    m_plPath+= "/../PixLib";
#endif
  } else
    m_plPath = plpath;
}
DBEdtEngine::~DBEdtEngine()
{
}
void DBEdtEngine::addFile(const char *name)
{
  if(name==0)
    throw SctPixelRod::BaseException("DBEdtEngine::addFile : pointer to file name is NULL");
  
  std::string fname = name;
  
  m_dbFiles.push_back(fname);
}
PixConfDBInterface* DBEdtEngine::openFile(const char *name, bool write)
{
  PixConfDBInterface *file=0;

  if(name==0)
    throw SctPixelRod::BaseException("DBEdtEngine::openFile : pointer to file name is NULL");
  
  std::string fname = name;
  int fnamelen = fname.length();

  // simple way to identify file type by extension
  if ((fnamelen > 5) && (fname.substr(fnamelen - 5, 5) == ".root")){
    std::string opt="";
    if(write){
      // check if file exists - if not, create
      FILE *ftest = fopen(name,"r");
      if(ftest==0){
	file = new PixLib::RootDB(name, "NEW");
	delete file; file=0;
      } else
	fclose(ftest);
      // re-open with write option
      opt = "UPDATE";
    }
    file = (PixConfDBInterface*) new RootDB(name, opt.c_str()); 
  } else{
    throw SctPixelRod::BaseException("DBEdtEngine::openFile : file name with unknown extension provided: "+fname);
  }
  return file;
}
void DBEdtEngine::clearFiles()
{
  m_dbFiles.clear();
}
void DBEdtEngine::createFile(const char *name)
{
  if(name==0)
    throw SctPixelRod::BaseException("DBEdtEngine::createFile : pointer to file name is NULL");
  
  std::string fname = name;

  // check if file exists - we can't (don't want to?) overwrite existing files
  FILE *testFile = fopen(fname.c_str(),"r");
  if(testFile!=0){
    fclose(testFile);
    throw SctPixelRod::BaseException("DBEdtEngine::createFile : file exists already, can't create");
  }

  // simple way to identify file type by extension
  if(fname.substr(fname.length()-5,5)==".root"){ // RootDB file
    RootDB *rf = new RootDB(fname.c_str(),"NEW"); 
    delete rf;
  } else{
    throw SctPixelRod::BaseException("DBEdtEngine::createFile : file name with unknown extension provided: "+fname);
  }
}
void DBEdtEngine::createConfig(const char *fname, std::vector<PixLib::grpData> myGrpData)
{
  PixConfDBInterface *myDB = PixLib::addAppInq(fname, "TestApp");
  delete myDB;
  myDB = openFile(fname, true);
  DBInquire *startInq = PixLib::findAppInq(myDB, "TestApp");
  for(std::vector<grpData>::iterator IT=myGrpData.begin(); IT!=myGrpData.end(); IT++)
    PixLib::addGrouptoDB(startInq, *IT, m_plPath.c_str());
  delete myDB;
}
