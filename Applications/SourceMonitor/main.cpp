
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
	TApplication t ("app", &argc, argv);
	QApplication q (        argc, argv);
	
	std::set <std::string> arguments;
	for (int i = 0; i < argc; ++i) if ( arguments.count( argv[i] ) == 0 ) arguments.insert( argv[i] );
	
	online::monitor m (arguments);
	
	m.resize(m.sizeHint());
	m.setWindowTitle("Online Monitor");
	m.setGeometry( 100, 100, 700, 500 );
	m.show();
	
	QObject::connect( &q, SIGNAL(lastWindowClosed()), &q, SLOT(quit()) );
	
	return q.exec();
}

