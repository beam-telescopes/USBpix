//Author : Denis Bertini 01.11.2000

/**************************************************************************
* Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI           *
*                     Planckstr. 1, 64291 Darmstadt, Germany              *
*                     All rights reserved.                                * 
* Contact:            http://go4.gsi.de                                   * 
*                                                                         * 
* This software can be used under the license agreements as stated in     * 
* Go4License.txt file which is part of the distribution.                  *  
***************************************************************************/

// somewhat simplified to serve my needs only
// canvas inside QT doesn't work for win32 anyway - JGK

#include "QRootApplication.h"
#include <TSystem.h>

QRootApplication::QRootApplication(int& argc, char **argv, int poll, bool GUIenabled)
  : QApplication(argc,argv,GUIenabled), m_poll(poll), m_GUIenabled(GUIenabled){
  rootapp = 0;
  m_argv  = 0;
  if(GUIenabled){
    m_argc=argc;
    m_argv = new char*[argc];
    for(int i=0;i<argc;i++){
      m_argv[i] = new char[100];
      sprintf(m_argv[i], "%s",argv[i]);
    }
  }
  // connect ROOT via Timer call back
  if (poll >= 0){
    m_mainTimer = new QTimer( this );
    connect( m_mainTimer, SIGNAL(timeout()), this, SLOT(execute()) );
    m_mainTimer->start( 20, FALSE );
  }
  if(poll>0){
    m_appTimer = new QTimer( this );
    m_appTimer->setSingleShot(true);
    connect( m_appTimer, SIGNAL(timeout()), this, SLOT(makeRootApp()) );
    m_appTimer->start( poll, FALSE );
  }
}   

QRootApplication::~QRootApplication(){
  if(rootapp!=0) rootapp->Terminate();
  delete rootapp;
  delete m_argv;
}

void QRootApplication::execute(){
  // processes pending root stuff when QT is idle
  if(m_GUIenabled){
    if(m_poll==0) makeRootApp();
    if(rootapp!=0) gSystem->ProcessEvents();
  }
}
void QRootApplication::makeRootApp(){
  if(rootapp==0) rootapp = new TApplication("Pixel Module Analysis",&m_argc,m_argv);
}
