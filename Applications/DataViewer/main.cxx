#include <BaseException.h>

#include "QRootApplication.h"
#include "DataViewer.h"

#include <iostream>
#include <sstream>
#include <exception>
#include <string>

int main( int argc, char** argv )
{

  bool showWaferV = false;
  for(int i=1;i<argc; i++){
    if(strcmp(argv[i],"-wafer")==0)
      showWaferV = true;
  }

  // start root and QT application
  QRootApplication app( argc, argv);

  // create main data viewer window
  DataViewer *Win = new DataViewer(0, Qt::Window, showWaferV);
  Win->show();

  // executing our application
  int ret = 0;
  std::stringstream msg;
  try{
    app.startTimer(); // needed to start TApplication event loop
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

  return ret;
}
