#include <QRootApplication.h>
#include <BaseException.h>
#include <VmeInterface.h>

#include "MainPanel.h"
#include "STCLogContainer.h"
#include "STControlEngine.h"
#include "STRodCrate.h"

#include <QString>

#include <iostream>
#include <sstream>
#include <exception>

int main( int argc, char** argv )
{

  // process command line arguments
  QString pathToRootDB="";
  bool autoInit = false;
  bool smallScreen = false;
  QString prlPath="";
  for(int i=0;i<argc;i++){
    // -l <filename.cfg.root> syntax: load RootDB cfg file
    if(strcmp(argv[i],"-l")==0 && i<(argc-1))
      pathToRootDB=argv[i+1];
    // -i plus above option -> auto-init of all ROD/BOCs
    if(strcmp(argv[i],"-i")==0)
      autoInit = true;
    // -s -> start with smaller initial window size
    if(strcmp(argv[i],"-s")==0)
      smallScreen = true;
  }

  // start root and QT application
  QRootApplication app( argc, argv );
  
  // create logging facility
  STCLogContainer *log = new STCLogContainer( &app );

  // create Engine
  STControlEngine *engine = new STControlEngine( &app, *log , 0);

  // launch main GUI window
  MainPanel *Win = new MainPanel(*engine, *log, 0, 0, smallScreen);
  Win->show();
   
  // create a crate on local host
  // always have exactly one crate object, never changed, never deleted
#ifdef WIN32
  QString SBCname  = getenv("COMPUTERNAME");
#else
  QString SBCname  = getenv("HOSTNAME");
#endif
  engine->addCrate(SBCname.toLatin1().data());

  // RootDB file given on command line: load
  if(pathToRootDB!="")
    engine->init(pathToRootDB.toLatin1().data(), autoInit, 0);
  else
    engine->init(0,false,0);

  // executing our application
  int ret = 0;
  std::stringstream msg;
  try{
    app.startTimer(); // needed to start TApplication event loop
    ret  = app.exec();
  } catch (SctPixelRod::VmeException& v) {
    msg << "VME-exception ";
    msg << v;
  } catch(SctPixelRod::BaseException& b){
    msg << "Base exception ";
    msg << b;
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
  // globalData is deleted by STControlEngine, which takes ownership
  delete engine;

  return ret;
}
