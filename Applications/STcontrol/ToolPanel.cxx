/***************************************************************************
                          ToolPanel.cpp  -  description
                             -------------------
    begin                : Wed May 12 2004
    copyright            : (C) 2004 by Jan Schumacher
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

#include "ToolPanel.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#ifdef WIN32
#include <stdlib.h>
#else
#include <unistd.h>
#endif

#include <QRadioButton>
#include <QApplication>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLCDNumber>
#include <QLabel>
#include <QComboBox>

#include <PixDcs/SleepWrapped.h>
#include <PixModuleGroup/PixModuleGroup.h>
#include <PixModule/PixModule.h>
#include <PixMcc/PixMcc.h>

#include <PixController/PixScan.h>
#include "STRodCrate.h"
#include "STControlEngine.h"

using namespace PixLib;
using namespace SctPixelRod;

ToolPanel::ToolPanel( STControlEngine &engine_in, QWidget* parent ):
  QWidget( parent ), m_engine( engine_in ){
  setupUi(this);
  QObject::connect(sendCommandButton, SIGNAL(clicked()), this, SLOT(sendCommand()));
  QObject::connect(stopButton, SIGNAL(clicked()), this, SLOT(setBreak()));
#ifdef WITHEUDAQ
//  QObject::connect(eudaq_cmdStartStopProducer, SIGNAL(clicked()), this, SLOT(eudaq_cmdStartStopProducer_clicked()));
//  QObject::connect(engine_in.m_STeudaq, SIGNAL(ProducerStateChanged(STEUDAQ::CON_STATE)), this, SLOT(eudaq_ProducerStatusChangedSlot(STEUDAQ::CON_STATE)));
#endif
  m_break = false;
  statusLabel->setText("idle");
  setIdle();

  resetOpt->insertItem(9999,"hard reset (ON)",     QVariant(PixModuleGroup::PMG_CMD_HRESET_ON));
  resetOpt->insertItem(9999,"hard reset (OFF)",    QVariant(PixModuleGroup::PMG_CMD_HRESET_OFF));
  resetOpt->insertItem(9999,"hard reset (pulse)",  QVariant(PixModuleGroup::PMG_CMD_HRESET_PLS));
  resetOpt->insertItem(9999,"soft global reset",   QVariant(PixModuleGroup::PMG_CMD_SRESET));
  resetOpt->insertItem(9999,"event counter reset", QVariant(PixModuleGroup::PMG_CMD_ECR));
  resetOpt->insertItem(9999,"bunch counter reset", QVariant(PixModuleGroup::PMG_CMD_BCR));

//#ifdef WITHEUDAQ
//  eudaq_rcAddress->setText(engine_in.m_STeudaq->rc_address.section(":", 0, 0));
//  if (engine_in.m_STeudaq->rc_address.section(":", 1, 1)=="") eudaq_rcPort->setText("44000");
//  else eudaq_rcPort->setText(engine_in.m_STeudaq->rc_address.section(":", 1, 1));
//#else
  eudaq_rcAddress->setText("NOT SUPPORTED");
  eudaq_groupBox->hide();
//#endif
}
ToolPanel::~ToolPanel(){
}
/** Send the chosen command(s) to the module. */
void ToolPanel::sendCommand(){

  int repetitions = repetitionSpinBox->value();
  
  if(repetitions>1)
    stopButton->setEnabled(true);

  setBusy();

  for( int repc=0 ; repc < repetitions ; repc++ ) {
      
    m_engine.updateGUI();
    if(m_break){
      setIdle();
      return;
    }

    statusLabel->setText("executing command");
    iterDisplay->display(repc);
    repaint();
    m_engine.updateGUI();

    if( configurationChoiceRadioButton->isChecked() ) {
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell engine to configure depending on user selection
      m_engine.configModules(configOpt->currentIndex());
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }
    
    if( resetChoiceRadioButton->isChecked() ) {
      int type = resetOpt->itemData(resetOpt->currentIndex()).toInt();
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell engine to configure them all
      m_engine.resetModules(type);
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }
    
    if( reinitRODs->isChecked() ) {
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell the engine to initialise RODs
      m_engine.initRods();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }
    
    if( resetBOCs->isChecked() ) {
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell the engine to initialise RODs
      m_engine.initBocs();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }
    
    if( initDCS->isChecked() ) {
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell the engine to initialise DCS objects
      m_engine.initDcs();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }
    
    if( initAll->isChecked() ) {
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell the engine to initialise all
      m_engine.initRods();
      m_engine.initDcs();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }
    
    if( triggerChoiceRadioButton->isChecked() ) {
      // set wait cursor
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell engine to trigger them all
      m_engine.triggerModules();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }

    if(getSrvRec->isChecked() ) {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell engine to trigger them all
      m_engine.getSrvRec();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }

    if(readEPROM->isChecked() ) {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      // tell engine to trigger them all
      m_engine.readEPROM();
      // restore normal cursor
      QApplication::restoreOverrideCursor();
    }

    // wait for given time if this is not the last iteration
    if(delaySpinBox->value()>0 && repc<(repetitions-1)){
      QApplication::setOverrideCursor(Qt::WaitCursor);
      for(int k=0;k<delaySpinBox->value();k++){
	statusLabel->setText("waiting");
	repaint();
	m_engine.updateGUI();
	if(m_break){
	  setIdle();
	  QApplication::restoreOverrideCursor();
	  return;
	}
	PixLib::sleep(1000);
      }
      QApplication::restoreOverrideCursor();
    }

  }

  setIdle();

  return;
}
void ToolPanel::setIdle(){
  stopButton->setEnabled(false);
  sendCommandButton->setEnabled(true);
  triggerChoiceRadioButton->setEnabled(true);
  getSrvRec->setEnabled(true);
  readEPROM->setEnabled(true);
  configurationChoiceRadioButton->setEnabled(true);
  resetChoiceRadioButton->setEnabled(true);
  reinitRODs->setEnabled(true);
  resetBOCs->setEnabled(m_engine.whichIFtype()!=tUSBSys);
  repetitionSpinBox->setEnabled(true);
  delaySpinBox->setEnabled(true);
  initDCS->setEnabled(true);
  initAll->setEnabled(true);
  m_break = false;
  statusLabel->setText("idle");
  repaint();
  m_engine.updateGUI();
}
void ToolPanel::setBusy(){
  sendCommandButton->setEnabled(false);  
  triggerChoiceRadioButton->setEnabled(false);
  getSrvRec->setEnabled(false);
  readEPROM->setEnabled(false);
  configurationChoiceRadioButton->setEnabled(false);
  resetChoiceRadioButton->setEnabled(false);
  reinitRODs->setEnabled(false);
  resetBOCs->setEnabled(false);
  repetitionSpinBox->setEnabled(false);
  delaySpinBox->setEnabled(false);
  initDCS->setEnabled(false);
  initAll->setEnabled(false);
  m_break = false;
  repaint();
  m_engine.updateGUI();
}
void ToolPanel::setBreak(){
  m_break = true;
}

#ifdef WITHEUDAQ /*
void ToolPanel::eudaq_cmdStartStopProducer_clicked(){
	if (m_engine.m_STeudaq->getConnectionState() == STEUDAQ::disconnected || m_engine.m_STeudaq->getConnectionState() == STEUDAQ::connectionerror) {
	  m_engine.m_STeudaq->setProducerId(pidBox->isChecked()?PIDVal->value():0);
		m_engine.m_STeudaq->start(eudaq_rcAddress->text () + ":" + eudaq_rcPort->text());
	} else if (m_engine.m_STeudaq->getConnectionState() == STEUDAQ::connecting) {
		// not supported
	} else if (m_engine.m_STeudaq->getConnectionState() == STEUDAQ::connected) {
		m_engine.m_STeudaq->stop();
	}
}

void ToolPanel::eudaq_ProducerStatusChangedSlot(STEUDAQ::CON_STATE state){
  // if called first time, fix producer ID (STcontrol crashes if changed after first connection...)
  if(pidBox->isEnabled()){
    pidBox->setEnabled(false);
    PIDVal->setEnabled(false);
    PIDVal->setValue((int)m_engine.m_STeudaq->getProducerId());
    pidBox->setChecked(m_engine.m_STeudaq->getProducerId()>0);
    PIDVal->setEnabled(false);
  }
  // then, display current producer connection status
	if (state == STEUDAQ::disconnected) {
		eudaq_cmdStartStopProducer->setText("Connect to Run Control");
		eudaq_cmdStartStopProducer->setEnabled(true);
		eudaq_status->setText("Disconnected");
	} else if (state == STEUDAQ::connecting) {
		//eudaq_cmdStartStopProducer->setText("Abort");
		eudaq_cmdStartStopProducer->setEnabled(false);
		eudaq_status->setText("Connecting");
	} else if (state == STEUDAQ::disconnecting) {
		eudaq_cmdStartStopProducer->setEnabled(false);
		eudaq_status->setText("Disconnecting");
	} else if (state == STEUDAQ::connected) {
		eudaq_cmdStartStopProducer->setText("Disconnect");
		eudaq_cmdStartStopProducer->setEnabled(true);
		eudaq_status->setText("Connected to Run Control at " + m_engine.m_STeudaq->rc_address);
	} else if (state == STEUDAQ::connectionerror) {
		eudaq_cmdStartStopProducer->setText("Disconnecting");
		eudaq_cmdStartStopProducer->setEnabled(false);
		eudaq_status->setText("Connection Error");
	}
} */
#endif
