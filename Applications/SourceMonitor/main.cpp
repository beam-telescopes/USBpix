
#include "monitor.h"

#include <TApplication.h>

#include <QStatusBar>
#include <QMessageBox>
#include <QMenuBar>
#include <QApplication>
#include <QImage>
#include <QTimer>

#include <set>
#include <string>
#include <iostream>

int main ( int argc, char* argv[] )
{
	std::set <std::string> arguments;
	for (int i = 0; i < argc; ++i){
	  if ( std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help"){
	    std::cout << "Usage: SourceMonitor [--\"option name\":\"option argument\"]" << std::endl;
	    std::cout << "                     (\"option name\" can be any of those listed in the parameters tab of the GUI)" << std::endl;
	    std::cout << " e.g.: SourceMonitor --input:<file_name.raw>" << std::endl;
	    std::cout << "                     to specify the STcontrol .raw file to be read" << std::endl;
	    return 0;
	  } else if ( arguments.count( argv[i] ) == 0 ) arguments.insert( argv[i] );
	}
	
	TApplication t ("app", &argc, argv);
	QApplication q (        argc, argv);
	
	online::monitor m (arguments);
	
	m.resize(m.sizeHint());
	m.setWindowTitle("Online Monitor");
	m.setGeometry( 100, 100, 700, 500 );
	m.show();
	
	QObject::connect( &q, SIGNAL(lastWindowClosed()), &q, SLOT(quit()) );
	
	return q.exec();
}

