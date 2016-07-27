/***************************************************************************
                          STCLogContainer.h  -  description
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

#ifndef STCLOGCONTAINER_H
#define STCLOGCONTAINER_H

#include <qobject.h>

class STCLog;

/**Simple Container for our logging channels
  *@author jschumac
  */

class STCLogContainer : QObject {

  Q_OBJECT

public:
  STCLogContainer( QObject* parent = 0 );
  virtual ~STCLogContainer();
  /** STControl Log */
  virtual STCLog& log();
  /** Log for ROD buffers */
  virtual STCLog& buffers();
  /** STControl Log (errors)*/
  virtual STCLog& errLog();
  /** Log for ROD buffers (errors)*/
  virtual STCLog& errBuffers();

private: // Private attributes
  /** General log */
  STCLog* m_log;
  /** Rod Buffers */
  STCLog* m_rODLog;
  /** General log (errors) */
  STCLog* m_errLog;
  /** Rod Buffers (errors) */
  STCLog* m_errRODLog;
};

#endif
