//Added by qt3to4:
#include <QShowEvent>
/***************************************************************************
                          LogPanel.h  -  description
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

#ifndef LOGPANEL_H
#define LOGPANEL_H

class QString;

#include <ui_LogPanel.h>

/**Interface Class to display text output
  *@author jschumac
  */

class LogPanel : public QWidget, public Ui::LogPanel  {

  Q_OBJECT

public:
  LogPanel( QWidget* parent = 0 );
   ~LogPanel();
public slots: // Public slots
  virtual void saveCurrentLog();
  void clearCurrentLog();
  /** Append to the general Log and open warning box */
  void errorToSTC( const QString& str_in );
  /** Append to the general Log */
  void logToSTC( const QString& str_in );
  /** Append to the ROD buffers Log */
  void logToROD( const QString& str_in );
  /** Append to the ROD buffers Log and open warning box */
  void errorToROD( const QString& str_in );
  /** Let the user get ROD buffers on demand */
  void getRodBuffers();
  /** browse to a folder for log saving so it's there after a crash */
  void browseLogFolder(const char *in_path);
  void browseLogFolder(){browseLogFolder(0);};
  void showEvent ( QShowEvent * );

 signals:
  /** emitted when the user asks for the current ROD buffers to be read */
  void wantRodBuffers();

 private:
  QString m_rodLogFile, m_stcLogFile;
  bool m_seenRodErrs;

};

#endif
