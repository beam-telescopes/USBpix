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

#ifndef QROOTAPPLICATION_H
#define QROOTAPPLICATION_H

/**
*  QRootApplication 
*
*  This class creates Qt environement that will
*  interface with the ROOT windowing system eventloop and eventhandlers. 
*  via a polling mechanism. 
*
*  @short Root application interfaced with Qt  
*
* @see QApplication 
* @authors Denis Bertini <d.bertini@gsi.de> 
* @version 2.0
*/

// somewhat simplified to serve my needs only
// canvas inside QT doesn't work for win32 anyway - JGK

#include <QApplication>

class TApplication;

class QRootApplication : public QApplication {
  Q_OBJECT  

 protected:
   TApplication *rootapp;
   bool m_GUIenabled;
   int m_argc;
   char **m_argv;
 public:
   QRootApplication(int& argc, char **argv,bool GUIenabled=true);
   ~QRootApplication(); 
   void startTimer();  

 public slots:
   void execute();

};

#endif

