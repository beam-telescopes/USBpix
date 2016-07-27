#include <BaseException.h>

#include "MainPanel.h"
#include "DBEdtEngine.h"

#include <QApplication>

#include <iostream>
#include <sstream>
#include <exception>
#include <string>

int main( int argc, char** argv )
{

  std::vector<std::string> fname;
  for(int i=1;i<argc; i++) fname.push_back(std::string(argv[i]));

  // start root and QT application
  QApplication app( argc, argv );

  // create engine
  DBEdtEngine *eng = new DBEdtEngine(app);
  for(std::vector<std::string>::iterator it=fname.begin(); it!=fname.end(); it++) 
    eng->addFile(it->c_str());
  // create main DBeditor GUI window
  MainPanel *Win = new MainPanel(*eng);
  Win->show();
  
  // executing our application
  int ret = 0;
  std::stringstream msg;
  try{
    ret  = app.exec();
  } catch(SctPixelRod::BaseException& b){
    msg << "Base exception \"";
    msg << b;
  } catch(std::exception& s){
    msg << "Std-lib exception \"";
    msg << s.what();
  } catch(...){
    msg << "Unknown exception \"";
  }
  if(msg.str()!="")
    std::cerr << msg.str() << "\" not caught during execution of main window." << std::endl;
  // cleaning up
  delete Win;
  delete eng;

  return ret;
}
