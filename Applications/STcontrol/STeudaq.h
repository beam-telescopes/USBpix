/***************************************************************************
                      STeudaq_com.h  -  description
                             -------------------
    begin                : Thu 25 Nov 2010
    copyright            : (C) 2010 by Vladislav Libov and Sebastian Schultes
    email                : schultes@uni-bonn.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef STEUDAQ_H
#define STEUDAQ_H

#include <iostream>
#ifdef WIN32
#include <stdlib.h>
#pragma warning(push)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#else
#include <unistd.h>
#endif

#undef min
#undef max

#include "STControlEngine.h"

#include <QEvent>
#include <QTimer>
#include <QVector>
#include <string>

//#include <QThread>
#include <QObject>

class STControlEngine;
class EUDAQProducer;
class QEvent;

class STEUDAQ : public QObject {

	Q_OBJECT

public:
enum CON_STATE {
  unknown,
  connecting,
  connected,
  disconnecting,
  disconnected,
  connectionerror
};

struct cfgInfo{
  int boardID;
  QString newFeName;
  QString fileName;
  QString modDecName;
};

 public:
 class ProducerThread{// : public QThread {

  public:
   ProducerThread();
    ~ProducerThread();
    void startProducer(STEUDAQ& base, std::string prdname, std::string runctrl);
    //void run() Q_DECL_OVERRIDE ;
#ifdef WITHEUDAQ
    EUDAQProducer& producer(){return *m_producer;};
#endif    

  private:
#ifdef WITHEUDAQ
	EUDAQProducer *m_producer;
#endif
    
 };
	STEUDAQ(STControlEngine& engine); 
	~STEUDAQ();
	bool start(QString RunControlAddress); 
	void stop();
	//std::vector<int> GetBoardIDs();
	int configured;		// Current configuration state 0=unconfigured, 1=configured, 2=error
	void setState(CON_STATE m_state) {status = m_state; emit ProducerStateChanged(status);};
	int getBoardsConnected();
	void stopScan();
	CON_STATE getConnectionState() { return status; };
	QString rc_address;
	QString CreateMultiBoardConfig();
	extScanOptions ScanParameters;
	// Options of the current Scan, recieved from STControlEngine
	int getConsecutiveLvl1TrigA(int);
	int getConsecutiveLvl1TrigB(int);
	void setProducerId(unsigned int producer_id) {m_producer_id = producer_id;};
	unsigned int getProducerId() {return m_producer_id;};
	// needed for propagating actions from producer (separate thread) into main QT thread
	void customEvent( QEvent * e );

public slots:
        //void ReadData();
	void initControler(extScanOptions ScanOptions);
	void initControler2();
	void ProducerDisconnected();
	void start();
	void scanFinished();
	void scanStarted();
	void startCurrentScanSlot(QString run_label, QString raw_filename);
	void prepareRun( unsigned runnumber );
	void errorReceived ( std::string msg ); // Error recieved from STControlEngine
	void dataPending(std::vector<unsigned int *>* data, int boardid);
	void ScanStatusSignal(int boardid, bool SRAMFullSignal, int SRAMFillingLevel, int TriggerRate);

signals:
	void ProducerStateChanged(STEUDAQ::CON_STATE);
/* 	void setScanOptions(extScanOptions); */
/* 	void startCurrentScan(QString run_label, QString raw_filename); */
/* 	void powerOn(); */
/* 	void powerOff(); */
/* 	void startedScan(); */
/* 	void finishedScan(); */
/* 	void finishedConfiguration(bool success); */
/* 	void finishedConfiguration(bool success, QVector<int> p_board_ids, QVector<int> feFlavours, int tot_mode); */
/* 	void createMultiBoardConfig(QString filename, extScanOptions ScanParameters); */

 private:
	// methods encapsulated in QT main event sloop, should not be called from outside!
	void initControlerTS(extScanOptions ScanOptions);
	void startCurrentScanSlotTS(QString run_label, QString raw_filename);
#ifdef WITHEUDAQ
	EUDAQProducer *m_producer;
#endif
	STControlEngine *m_STControlEngine;
	QApplication* m_app;
	CON_STATE status;
	bool initializing;
	unsigned int m_producer_id;

	std::string getProducerName();

};

class prodInitEvent : public QEvent
{
 public:
  prodInitEvent(extScanOptions ScanOptions)
    : QEvent((QEvent::Type)3001){ m_opts = ScanOptions;};
  
  extScanOptions options(){return m_opts;};

 private:
  extScanOptions m_opts;
};

class prodStartScanEvent : public QEvent
{
 public:
  prodStartScanEvent(QString run_label, QString raw_filename)
    : QEvent((QEvent::Type)3002){ m_runLabel = run_label; m_rawFilename = raw_filename;};
  
  QString runLabel(){return m_runLabel;};
  QString rawFilename(){return m_rawFilename;};

 private:
  QString m_runLabel, m_rawFilename;
};

class prodStopScanEvent : public QEvent
{
 public:
  prodStopScanEvent()
    : QEvent((QEvent::Type)3003){};
};

class prodPrepScanEvent : public QEvent
{
 public:
  prodPrepScanEvent()
    : QEvent((QEvent::Type)3004){};
};

#endif
