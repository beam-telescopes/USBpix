#include <qapplication.h>
#include <BaseException.h>
#include <RCCVmeInterface.h>
#include <QRootApplication.h>
#include "mainwin.h"
#include "TopWin.h"
#include "DAEngine.h"

#include <iostream>
#include <sstream>
#include <exception>

int main( int argc, char ** argv )
{
  char *ho = argv[1];
  if(argc<=2)//GUI mode
    {
      QRootApplication a( argc, argv );
      //TopWin *top = new TopWin(NULL,"MAmain",0,true, &a);
      // MainWin w; 
      DAEngine eng(true);
      MainWin w(&eng, &a);
      a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
      if( a.argc()>1)// load cut file - must be arg. #1
      	eng.loadCuts(ho);//a.argv()[1]);
     
      w.show();
      int retval = -1;
      std::stringstream msg;
      try{
	retval = a.exec();
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
      return retval;
    }
  else if(argc>2)//batch mode
    {
      QApplication a( argc, argv, false);
    
      // QListViewItem *lv = new QListViewItem();
      // MainWin w;//= new MainWin();
      //w.show();
      DAEngine eng(false);
      a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

      
      for ( int i = 2; i < a.argc(); i++ )  // a.argc() == argc
      	{
      	  eng.loadFile(a.argv()[i]);
      	}
      eng.loadCuts(a.argv()[1]);
      eng.startAnalyse("All Tests");
    }
}
