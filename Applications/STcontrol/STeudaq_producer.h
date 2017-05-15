/***************************************************************************
                      STeudaq_producer.h  -  description
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
#ifndef STEUDAQPRODUCER_H
#define STEUDAQPRODUCER_H

#define STEP_DEBUG true

#include <iostream>
#ifdef WIN32
#include <stdlib.h>
#pragma warning(push)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#else
#include <unistd.h>
#endif

class STEUDAQ;

#include <QVector>
#include <QObject>
#include <QMutex>

#include <string>
#include <map>
#include <queue>

#include <eudaq/Producer.hh>
#include <eudaq/Configuration.hh>
#include "STCdefines.h"

//amount of trigers TLU can handle (15bit), i.e. one larger than max trigger no from TLU
#define TLU_TRIGGER_AMOUNT	0x8000

//----------------------------------------------------------------------
class EUDAQProducer : public eudaq::Producer {

  public:
 EUDAQProducer(STEUDAQ& base, std::string prdname, std::string runctrl); 
	~EUDAQProducer();
	// EUDAQ methods, must be re-implemented here
	void DoInitialise() override;
	void DoConfigure() override;
	void DoStartRun() override;
	void DoStopRun() override;
	void DoTerminate() override;
	void DoReset() override;
	// misc. helper functions for STcontrol communication
	void dataPending(std::vector<unsigned int *>* data, int boardid);
	void ScanStatus(int boardid, bool SRAMFullSignal, int SRAMFillingLevel, int TriggerRate);
	void scanFinished();
	void beganScanning();
	void configured(bool success);
	void configured(bool success, QVector<int> p_board_ids, QVector<int> feFlavours, int tot_mode=0);
	void errorReceived(std::string msg); // Error recieved from STControlEngine

  private:
	std::string EVENT_TYPE;
	STEUDAQ &m_STeudaq;

 	unsigned m_run, m_ev;
	bool data_pending_running, run_finished;
	int board_count, boards_read_out;
	std::vector<std::list <unsigned int> > EventData;
	std::vector<int> board_ids;
	std::vector<int> recieved_trigger;

	//A vector storing to which sensor the data belongs, e.g. "1 1 2 3 4"
	//tells you that there will be 5 chips in your setup, the first two
	//belonging to a double chip and three singles, NOTE: this is for all
	//boards together! Order is defined by the order from the "boards"
	//entry in EUDAQ config file
	std::vector<int> boardChips;

	//Offset (value) where to write your data for any board (key) in the EventData
	std::map<int, int> boardOffset;

	//Amount of data (i.e. chips read out by board) you will receive for given board (key)
	//e.g. a board w/ 4-chip module will return 4, one with one double chip and a single chip
	//together 3 and so on
	std::map<int, int> boardChipCount;

	int lastTriggerNumber;
	int maxTriggerNumber;
	void sendEvents(bool endrun=false);

	// make partially use of specific data formats -> keep separate
	void dataPendingI4(std::vector<unsigned int *>* data, int boardid);
	void dataPendingI3(std::vector<unsigned int *>* data, int boardid);
	void sendEventsI3(bool endrun);
	void sendEventsI4(bool endrun);
	
	//Confiuration status: 0: busy/unconfigured, 1: success, 2: error
	int configuration; 

	QMutex mutex, mutex_send;
	unsigned int readout_counter;
	bool scanning;
	bool all_events_recieved;
	int trigger_zero_counter;
	eudaq::Configuration m_config;
	extScanOptions scan_options;
	unsigned int consecutive_lvl1;
	unsigned int tot_mode;
	bool abort_run;
	bool scan_started_properly;
	std::map<int, std::queue<int> > triggerRate;
	//	unsigned int encodeTriggerNumber(unsigned int trigger_number, bool first);
	void AbortRun();

};

#ifdef WIN32
#pragma warning(pop)
#endif
#endif // STEUDAQ_H
