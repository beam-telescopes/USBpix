/***************************************************************************
                          STCLogContainer.cxx  -  description
                             -------------------
    begin                : Tue Jul 13 2004
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

#include "STCLog.h"

#include "STCLogContainer.h"

STCLogContainer::STCLogContainer( QObject* parent ):
  QObject( parent ){
  m_log       = new STCLog( this );
  m_rODLog    = new STCLog( this );
  m_errLog    = new STCLog( this );
  m_errRODLog = new STCLog( this );
}
STCLogContainer::~STCLogContainer(){
}
/** Read property of STCLog* m_log. */
STCLog& STCLogContainer::log(){
  return *m_log;
}
/** Log for ROD buffers */
STCLog& STCLogContainer::buffers(){
  return *m_rODLog;
}
/** Read property of STCLog* m_log. */
STCLog& STCLogContainer::errLog(){
  return *m_errLog;
}
/** Log for ROD buffers */
STCLog& STCLogContainer::errBuffers(){
  return *m_errRODLog;
}
