/***************************************************************************
                          ToolPanel.h  -  description
                             -------------------
    begin                : Wed May 12 2004
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

#ifndef TOOLPANEL_H
#define TOOLPANEL_H

#include <ui_ToolPanel.h>
#ifdef WITHEUDAQ
#include "STeudaq.h"
#endif

class STControlEngine;

/**
  *@author Jan Schumacher
  */

class ToolPanel : public QWidget, public Ui::ToolPanel  {

 Q_OBJECT

public: 
  ToolPanel( STControlEngine &engine_in, QWidget* parent = 0 );
  virtual ~ToolPanel();
public slots: // Public slots
  /** Send the chosen command(s) to the module. */
  void sendCommand();
  void setBreak(); 
  void setBusy();
  void setIdle();
#ifdef WITHEUDAQ
//  void eudaq_cmdStartStopProducer_clicked();
//  void eudaq_ProducerStatusChangedSlot(STEUDAQ::CON_STATE state);
#endif

private:
  STControlEngine & m_engine;
  bool m_break;
};

#endif
