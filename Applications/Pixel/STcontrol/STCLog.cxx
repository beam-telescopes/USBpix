/***************************************************************************
                          STCLog.cpp  -  description
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

#include <qstring.h>

#include "LogPanel.h"

#include "STCLog.h"

STCLog::STCLog(QObject* parent):
  QObject( parent ){
}
STCLog::~STCLog(){
}
/** Append to the log */
STCLog& STCLog::operator<< ( const QString& str_in ){

  // copy to make sure str_in is not destroyed when it is still needed
  const QString qstr( str_in );

  emit added( qstr );

  return *this;
}
STCLog& STCLog::operator<< ( int int_in ){
  return (*this) << QString::number( int_in );
}
STCLog& STCLog::operator<< ( unsigned int int_in ){
  return (*this) << QString::number( int_in );
}
