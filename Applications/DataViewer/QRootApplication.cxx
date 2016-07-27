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

// somewhat simplified to serve my needs only - JGK

#include "QRootApplication.h"
#include <TApplication.h>
#include <TSystem.h>
#include <TGraph.h>
#include <QTimer>

QRootApplication::QRootApplication(int& argc, char **argv, bool GUIenabled)
  : QApplication(argc,argv,GUIenabled), m_GUIenabled(GUIenabled){
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
}
QRootApplication::~QRootApplication(){
  if(rootapp!=0) rootapp->Terminate();
  delete rootapp;
  delete m_argv;
}

void QRootApplication::startTimer(){
  // connect ROOT via Timer call back
  QTimer *mainTimer = new QTimer( this );
  connect( mainTimer, SIGNAL(timeout()), this, SLOT(execute()) );
  mainTimer->start( 20 );
}   

void QRootApplication::execute(){
  // processes pending root stuff when QT is idle
  if(m_GUIenabled) {
    // create ROOT's main application if not done yet
    if(rootapp==0){
      rootapp = new TApplication("Pixel Data Viewer",&m_argc,m_argv);
      // JGK: taken out of PlotFactory to avoid creation of default Tapplication
      //      if there is any better way, put this back into PlotFactory's constructor
      // standard fit functions - make sure they are around
      float x[4]={1,2,3,4}, y[4]={2,3,4,7};
      TGraph g(4,x,y);
      g.Fit("pol0","0Q");
      g.Fit("pol1","0Q");
      g.Fit("pol2","0Q");
      g.Fit("pol3","0Q");
      g.Fit("expo","0Q");
      g.Fit("gaus","0Q");
    }
    gSystem->ProcessEvents();
  }
}
