/***************************************************************************
                          STCLog.h  -  description
                             -------------------
    begin                : Mon Jul 12 2004
    copyright            : (C) 2004 by jschumac
    email                : jschumac@physik.uni-bonn.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STCLOG_H
#define STCLOG_H

#include <qobject.h>

class LogPanel;


/**A logger for STControl
  *@author jschumac
  */

class STCLog : public QObject {

  Q_OBJECT

public:
  STCLog( QObject* parent = 0 );
  virtual ~STCLog();
  /** Append to the log */
  virtual STCLog& operator<< ( const QString& );
  virtual STCLog& operator<< ( int );
  virtual STCLog& operator<< ( unsigned int );

signals: // Signals
  /** Emitted when an item is inserted */
  void added( const QString& );
};

#endif
