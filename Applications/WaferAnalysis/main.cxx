#include <QRootApplication.h>
#include "WaferAnalysisPanel.h"

#include <iostream>
#include <sstream>
#include <exception>

int main( int argc, char** argv )
{
  // start root and QT application
  QRootApplication app( argc, argv );

  // launch main GUI window
  WaferAnalysisPanel *Win = new WaferAnalysisPanel( 0 );
  Win->show();
  //  app.setMainWidget(Win);

  // executing our application
  int ret = 0;
  std::stringstream msg;
  try{
	  if (argc == 2){
		  std::string tFile(argv[1]);
		  if(tFile.substr(tFile.find_last_of(".")).compare(".waprj") == 0)
			  Win->openProject(tFile);	//auto open project file if its in the argument list
	  }
    app.startTimer(); // needed to start TApplication event loop
    ret  = app.exec();
//   } catch(SctPixelRod::BaseException& b){
//     msg << "Base exception ";
//     msg << b;
  } catch(std::exception& s){
    msg << "Std-lib exception ";
    msg << s.what();
  } catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!="")
    std::cerr << msg.str() << " not caught during execution of main window." << std::endl;
  // cleaning up
  delete Win;

  return ret;
}
