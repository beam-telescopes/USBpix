/***************************************************************************
                          LogPanel.cxx  -  description
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

#include <QTextBrowser>
#include <QTabWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include <QShowEvent>

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "LogPanel.h"

LogPanel::LogPanel( QWidget* parent )
  : QWidget( parent )
{
  setupUi(this);
  QObject::connect(SaveLogButton, SIGNAL(clicked()), this, SLOT(saveCurrentLog()));
  QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clearCurrentLog()));
  QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(browseLogFolder()));
  QObject::connect(getlogButton, SIGNAL(clicked()), this, SLOT(getRodBuffers()));

  logsavePath->setText(QString::null);
  QString dt = QDateTime::currentDateTime().toString("hh:mm:ss_MM-dd-yyyy");
  m_rodLogFile = "rodbuff_"+dt+".txt";
  m_stcLogFile = "stclog_"+dt+".txt";
#ifdef WIN32 // replace colons by dashes, win doesn't like them in file names...
  m_rodLogFile.replace(":","-");
  m_stcLogFile.replace(":","-");
#endif
  m_seenRodErrs = true;
}
LogPanel::~LogPanel(){
}
void LogPanel::saveCurrentLog(){
  QStringList filter;
  filter += "Text file (*.txt)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of log file");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);
  if(fdia.exec() == QDialog::Accepted) {
    QString myFile = fdia.selectedFiles().at(0);
    myFile.replace("\\", "/");
    std::ofstream out(myFile.toLatin1().data());
    if (out.is_open()){
      if(LogTabWidget->currentWidget()==LogTabWidget->widget(0)){       // STcontrol log
	out << "STcontrol log" << std::endl;
	out << STControlLogBrowser->toPlainText().toLatin1().data();
      }else if(LogTabWidget->currentWidget()==LogTabWidget->widget(1)){ // ROD buffer log
	out << "Controller buffer log" << std::endl;
	out << RODBuffersLogBrowser->toPlainText().toLatin1().data();
      }
      out.close();
    }
  }
}
void LogPanel::clearCurrentLog(){
  if(LogTabWidget->currentWidget()==LogTabWidget->widget(0)){       // STcontrol log
    STControlLogBrowser->clear();
  }else if(LogTabWidget->currentWidget()==LogTabWidget->widget(1)){ // ROD buffer log
    RODBuffersLogBrowser->clear();
  }

}
/** Append to the general Log */
void LogPanel::logToSTC( const QString& str_in ){
  STControlLogBrowser->moveCursor( QTextCursor::End);
  STControlLogBrowser->insertPlainText( str_in );
  if(!logsavePath->text().isEmpty()){
    FILE *sctFile = fopen((logsavePath->text()+"/"+m_stcLogFile).toLatin1().data(),"a");
    fprintf(sctFile,"%s",str_in.toLatin1().data());
    fclose(sctFile);
  }
}
/** Append to the general Log, and open window since this is an error msg. */
void LogPanel::errorToSTC( const QString& str_in ){
  // "normal" logging
  QString str = "ERROR:\n";
  str += str_in;
  logToSTC(str);
  // open waning box
  // default string should be class::routine : message -> split
  QString function="STcontrol", message=str_in;
  int pos = message.indexOf(" : ");
  if(pos>=0){
    function = message.left(pos);
    message = message.right(message.length()-pos-3);
  }
  QMessageBox::warning(this, function, message);
}
/** Append to the ROD buffers Log */
void LogPanel::logToROD( const QString& str_in ){
  RODBuffersLogBrowser->moveCursor( QTextCursor::End);
  RODBuffersLogBrowser->insertPlainText( str_in );
  if(!logsavePath->text().isEmpty()){
    FILE *rodFile = fopen((logsavePath->text()+"/"+m_rodLogFile).toLatin1().data(),"a");
    fprintf(rodFile,"%s",str_in.toLatin1().data());
    fclose(rodFile);
  }
}
/** Append to the ROD buffers Log, and open window since this is an error msg. */
void LogPanel::errorToROD( const QString& str_in ){
  // "normal" logging
  logToROD(str_in);
  // error window
  if(m_seenRodErrs){
    QMessageBox::warning(this, "ROD buffer", "There is a message in one of the controller error buffers.\n Please check log panel for details.");
    m_seenRodErrs = false;
  }
}
void LogPanel::getRodBuffers(){
  logToROD( "user-triggered buffer readout\n");
  emit wantRodBuffers();
}
void LogPanel::browseLogFolder(const char *in_path){
  
  QString path;
  if(in_path==0){
    path = QFileDialog::getExistingDirectory ( this, "Browse to log-saving directory", 
					       logsavePath->text());
  }else{
    path = in_path;
    if(path.right(1)!="/") path+="/";
  }

  if(!path.isEmpty() && logsavePath->text()!=path){
    logsavePath->setText(path);
    FILE *rodFile = fopen((logsavePath->text()+m_rodLogFile).toLatin1().data(),"w");
    FILE *stcFile = fopen((logsavePath->text()+m_stcLogFile).toLatin1().data(),"w");
    fprintf(stcFile,"%s",STControlLogBrowser->toPlainText().toLatin1().data());
    fprintf(rodFile,"%s",RODBuffersLogBrowser->toPlainText().toLatin1().data());
    fclose(rodFile);
    fclose(stcFile);
  }

  return;
}
void LogPanel::showEvent ( QShowEvent *e )
{
  QWidget::showEvent(e);
  m_seenRodErrs=true;
}
