#include <eudaq/Timer.hh>
#include <eudaq/RawDataEvent.hh>
#include <eudaq/Logger.hh>

#include "STeudaq_producer.h"
#include "STeudaq.h"
//required for setting static tb flags
#include "STPixModuleGroup.h"

#include <defines.h>

#include <QFile>
#include <QDir>
#include <QThread>

#include <iterator>

using eudaq::to_string;

#define STEP_DEBUG true

EUDAQProducer::EUDAQProducer(STEUDAQ& base, /*QApplication * application,*/ std::string prdname, std::string runctrl):
  eudaq::Producer(prdname, runctrl), 
  m_STeudaq(base), 
  m_run(0), 
  m_ev(0)
{
	if(STEP_DEBUG) std::cout << "EUDAQ-Producer Start - Thread-ID: " << QThread::currentThreadId() << std::endl;
	//	m_app = application;
	scanning = false;
	readout_counter=0;
	boards_read_out=0;
	configuration=0;
	all_events_recieved=false;
	abort_run=false;
	EUDAQProducer::EVENT_TYPE = "USBPIXI4"; // default, to be changed after configuration
	boardChips.clear();
};

EUDAQProducer::~EUDAQProducer()
{
	if(STEP_DEBUG) std::cout << "Producer Thread Destructor" << std::endl;
	m_STeudaq.ProducerDisconnected();
};

//TODO: This should never happen, therefore should be removed
void EUDAQProducer::ScanStatus(int boardid, bool SRAMFullSignal, int /*SRAMFillingLevel*/, int /*TriggerRate*/)
{	
	if (SRAMFullSignal)
	{ 	
		EUDAQ_ERROR(QString("Board%1)'s SRAM reached a fill level of 100%, this should not happen!").arg(boardid).toLatin1().data());	
	}
}

//This gets called whenever the DAQ is configured
void EUDAQProducer::OnConfigure(const eudaq::Configuration & config) 
{
	if(STEP_DEBUG) std::cout << "EUDAQ-Producer OnConfigure thread-ID: " << QThread::currentThreadId() << std::endl;
     
	SetStatus(eudaq::Status::LVL_BUSY, "Configuring");

	m_config = config;
	configuration = 0;
	tot_mode = 0;

	//reset the module config
	boardChips.clear();	

	//Get Configuration Data
	QStringList config_files, config_modules, trigger_replication_modes;
	QString config_file, config_module, trigger_replication_mode;

	scan_options.boards = QString::fromStdString(config.Get("boards", "no")).split(",", QString::SkipEmptyParts);

	for(size_t i=0; i < (size_t)scan_options.boards.size() ; i++)
	{
		//Module configuration
		QStringList chipsOnBoard  = QString::fromStdString( config.Get((QString("modules[") + scan_options.boards[i] + QString("]")).toStdString(), "no") ).split(",", QString::SkipEmptyParts);
		int boardID = scan_options.boards[i].toInt();	

		boardOffset[boardID] = boardChips.size();
		boardChipCount[boardID] = chipsOnBoard.size();
     	
		//If not specified, inform user and terminate
		if(chipsOnBoard.size() == 0)
		{
			EUDAQ_ERROR((QString("Invalid module configuration for board  ") + scan_options.boards[i]).toStdString().c_str());
			SetStatus(eudaq::Status::LVL_ERROR, "Error during initialisation");
			return;
		}

		//Otherwise, get the module configuration
		else
		{
			int previous = -1;

			for(size_t i = 0; i < (size_t)chipsOnBoard.size(); i++)
			{
				int current = chipsOnBoard[i].toInt();

				if(boardChips.empty())
				{
					boardChips.push_back(1);
				}
				else
				{
					int lastEntry = boardChips.back();
     				
					if(current == previous)
					{
						boardChips.push_back(lastEntry);
					}
					else
					{	
						boardChips.push_back(lastEntry+1);	
					}						
				}
				previous = current;
			}	
		}
	}

	if(STEP_DEBUG)
	{
		std::cout << "data setup" << std::endl;

		for(std::vector<int>::iterator it = boardChips.begin(); it != boardChips.end(); it++)
		{
			std::cout << *it << std::endl;
		}

		std::cout << "offset list" << std::endl;

		for(std::map<int, int>::iterator it = boardOffset.begin(); it != boardOffset.end(); it++)
		{
			std::cout << (*it).first << "|" << (*it).second << std::endl;
		}

		std::cout << "chip count" << std::endl;

		for(std::map<int, int>::iterator it = boardChipCount.begin(); it != boardChipCount.end(); it++)
		{
			std::cout << (*it).first << "|" << (*it).second << std::endl;
		}
	}

	//Check if the skip configuration was set
	scan_options.SkipConfiguration = (QString::fromStdString(config.Get("SkipConfiguration", "no")).toLower()=="yes");	
	if(scan_options.SkipConfiguration) 
	{
		//No configuration requested
		SetStatus(eudaq::Status::LVL_OK, "Configured (" + m_config.Name() + ")");
		return;
	}

	scan_options.SRAM_READOUT_AT = config.Get("SRAM_READOUT_AT", 30);
	scan_options.UseSingleBoardConfig = (QString::fromStdString(config.Get("UseSingleBoardConfigs", "no")).toLower()=="yes");
	scan_options.config_file = QString::fromStdString( config.Get("config_file", ""));
     
	if (scan_options.UseSingleBoardConfig || scan_options.config_file=="")
	{
		scan_options.UseSingleBoardConfig = true;

		bool send_error=false;

		for(size_t i=0; i<(size_t)scan_options.boards.size(); i++)
		{
			//config file for current board
			config_file = QString::fromStdString(config.Get((QString("config_file[") + scan_options.boards[i] + QString("]")).toStdString().c_str(), "")).trimmed();
			if(config_file=="")
			{
				config_file=QString::fromStdString(config.Get("config_file[*]", "")).trimmed();
			}
			if(config_file=="" || !QFile::exists(config_file))
			{
				// No valid config
				EUDAQ_ERROR((QString("No valid config for board ") + scan_options.boards[i]).toStdString().c_str());
				send_error=true;
			}

			//Module from Config file to use
			config_module=QString::fromStdString(config.Get((QString("config_module[") + scan_options.boards[i] + QString("]")).toStdString().c_str(), "")).trimmed();
			if(config_module=="")
			{
				config_module=QString::fromStdString(config.Get("config_module[*]", "")).trimmed();
			}

			//Trigger replication mode
			trigger_replication_mode=QString::fromStdString(config.Get((QString("trigger_replication[") + scan_options.boards[i] + QString("]")).toStdString().c_str(), "")).toLower();
			if (trigger_replication_mode=="")
			{
				trigger_replication_mode=QString::fromStdString(config.Get("trigger_replication[*]", "off")).toLower();
			}
			if (trigger_replication_mode!="off" && trigger_replication_mode!="master" && trigger_replication_mode!="slave")
			{
				EUDAQ_ERROR((QString("Invalig trigger replication mode set for board ") + scan_options.boards[i]).toStdString().c_str());
				send_error=true;
			}

			config_files += config_file;
			config_modules += config_module;
			trigger_replication_modes += trigger_replication_mode;
			if(STEP_DEBUG) std::cout << "Configfile for Board " << scan_options.boards[i].toStdString() << ": " << config_file.toStdString() << std::endl;

     		
		}

		scan_options.config_files = config_files;
		scan_options.config_modules = config_modules;
		scan_options.trigger_replication_modes = trigger_replication_modes;

		if(send_error)
		{
			SetStatus(eudaq::Status::LVL_ERROR, "Error during initialisation");
			return;
		}
	}

	scan_options.fpga_firmware = QString::fromStdString(config.Get("fpga_firmware", ""));
	scan_options.uc_firmware = QString::fromStdString(config.Get("uc_firmware", ""));
	scan_options.adapterCardFlavour = config.Get("adapterCardFlavour", 0);
	scan_options.rawdata_path = QString::fromStdString(config.Get("rawdata_path", ""));
	scan_options.histogram_filename = QString::fromStdString(config.Get("histogram_filename", ""));
	scan_options.lvl1_delay  = QString::fromStdString(config.Get("lvl1_delay", "26")).toInt();
	scan_options.first_sensor_id  = QString::fromStdString(config.Get("first_sensor_id", "0")).toInt();
	scan_options.TLUTriggerDataDelay  = QString::fromStdString(config.Get("tlu_trigger_data_delay", "10")).toInt();
	scan_options.triggerRateThreshold = config.Get("trigger_rate_threshold", 0);
	scan_options.scan_type = "DEFAULT: TESTBEAM_EUDAQ";
     
	if(STEP_DEBUG) std::cout << "Configuring: " << m_config.Name() << std::endl;
	m_STeudaq.initControler(scan_options);

	configuration = 0;
	// wait for configuration to be finished
	// sending eudaq Event in this function ensures that
	// the status is displayed correctly
	while( configuration == 0 )
	{
		eudaq::mSleep(50);
		QApplication::processEvents();
	}

	if( configuration == 1 )
	{
		// set the status that will be displayed in the Run Control.
		SetStatus(eudaq::Status::LVL_OK, "Configured (" + m_config.Name() + ")");
	}
	else
	{
		SetStatus(eudaq::Status::LVL_ERROR, "Error while initializing PixControllers");
	}
} 

void EUDAQProducer::configured(bool success)
{
	QVector<int> dummy1, dummy2;
	configured(success, dummy1, dummy2);
}

void EUDAQProducer::configured(bool success, QVector<int> p_board_ids, QVector<int> feFlavours, int totMode)
{
	if(success)
	{
		//read orientations
	 	board_count = p_board_ids.size(); //p_board_count;
		
		board_ids.clear();
		board_orientation.clear ();
		recieved_good_status.clear();
		tot_mode = totMode;
		
		EUDAQ_DEBUG((QString("HitDiscConfig: ") + QString::number(tot_mode)).toStdString());

		// check if all requested boards are connected
		if(scan_options.UseSingleBoardConfig)
		{
			for (int i=0; i<scan_options.boards.count(); i++)
			{
				//if (id_list.indexOf(scan_options.boards[i])==-1)
				bool found=false;
				for(int j=0;j<p_board_ids.size();j++)
				{
					if(QString::number(p_board_ids[j])==scan_options.boards[i])
					{
						found=true;
						break;
					}
				}
				if(!found)
				{
					if(STEP_DEBUG) std::cout << "USBpix Board " <<  scan_options.boards[i].toStdString() << " not initialized!" << std::endl;
					configuration = 2;
					return;
				}
			}
		}

		if(STEP_DEBUG) std::cout << "Initialized Boards (have "<< p_board_ids.size() <<"): "<< std::endl;
		int myFeFlv = -1;

		for(int i=0;i<p_board_ids.size();i++)
		{
			board_ids.push_back(p_board_ids[i]);
			recieved_good_status.push_back(false);
			board_orientation.push_back(QString::fromStdString(m_config.Get((QString("orientation[") + QString::number(p_board_ids[i]) 
										   + QString("]")).toStdString(), "0")).toInt());
			if(STEP_DEBUG) std::cout << "board ID " << board_ids[i] << " has FE flavour " << feFlavours[i] << std::endl;
			
			if(myFeFlv == -1)
			{
				myFeFlv=feFlavours[i];
			}
			else if(feFlavours[i]!=myFeFlv)
			{
				configuration = 2;
				if(STEP_DEBUG) std::cout << "Error: mix of FE flavours is not supported" << std::endl;
				return;
		  	}
		}

		configuration = 1;

		//Determine the flavours of the FE's and set the EVENT_TYPE for EUDAQ
		if(myFeFlv == 2)
		{
			EUDAQProducer::EVENT_TYPE = "USBPIXI4";
		}
		else if(myFeFlv == 3)
		{
			EUDAQProducer::EVENT_TYPE = "USBPIXI4B";
		}		
		else if(myFeFlv == 1)
		{
			EUDAQProducer::EVENT_TYPE = "USBPIX";
		}
	}
	//called if (!success)
	else
	{
		configuration = 2;
		if(STEP_DEBUG) std::cout << "Error while initializing PixControllers" << std::endl;
	}
}
  
//This gets called whenever a new run is started, it receives the new run number as a parameter
void EUDAQProducer::OnStartRun(unsigned param)
{
	//OnPrepareRun(param);

	if(STEP_DEBUG)  std::cout << "EUDAQ-Producer OnStartRun - Thread-ID: " << QThread::currentThreadId() << std::endl;
	
	abort_run = false;
    	m_run = param;
	data_pending_running = false;
    	m_ev = 0;
    
	if(STEP_DEBUG) std::cout << "Start Run: " << m_run << std::endl;

	//wait - unsure if it is neccessary
	eudaq::mSleep(3000);

	//init arrays for temp data storage
	EventData.clear ();
	recieved_trigger.clear ();

	if(STEP_DEBUG) 
	{
		std::cout << "Boards used for this Run:";
		for (int i=0; i<board_ids.size(); i++) 
		{
			std::cout << " " << board_ids[i];
		}
		std::cout << std::endl;
	}

	//initialize data vector
	std::list<unsigned int> empty_list;
	for(size_t i=0; i < boardChips.size(); i++)
	{
		EventData.push_back(empty_list);
		recieved_trigger.push_back(-1);
	}

	//scan status variable
	scan_started_properly = false;

	lastTriggerNumber = -1;
	maxTriggerNumber = -1;

	if(STEP_DEBUG) std::cout << "Start Run!";

	scanning = true;
	QString raw_filename;

	if(scan_options.rawdata_path != "")
	{
		raw_filename = QDir::toNativeSeparators(QDir::cleanPath(scan_options.rawdata_path) + "/") + QString("testbeam_run_") + QString::number(m_run).rightJustified(6, '0') + ".raw";
		if(STEP_DEBUG) std::cout << "Writing RAW data to: " << raw_filename.toStdString() << std::endl;
	} 
	else 
	{
		raw_filename = "";
	}

	
	m_STeudaq.startCurrentScanSlot(QString(QString("Run ") + QString::number(m_run)), raw_filename);

	readout_counter = 0;
}

void EUDAQProducer::beganScanning() 
{
	if(!scanning) 
	{
		return;
	}

	//It must send a BORE to the Data Collector
	if(STEP_DEBUG) std::cout << "sending BORE for event type " << EUDAQProducer::EVENT_TYPE << std::endl;
	
	eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(EUDAQProducer::EVENT_TYPE, m_run));

	//send boards
	bore.SetTag("boards", eudaq::to_string(board_count)); //);

	//send board ids
	for(int i=0; i<board_count; i++) 
	{
		bore.SetTag("boardid_" + eudaq::to_string(i), eudaq::to_string(board_ids[i]));
	}

	//set module config	
	std::stringstream stream;
	const char* seperator = "";

	for(std::vector<int>::iterator it = boardChips.begin(); it != boardChips.end(); it++)
	{
		stream << seperator << (*it);
		seperator = ",";
	}
	
	bore.SetTag("modules", stream.str());

	std::cout << "TEST: " << stream.str();

	
	//send configuration data
	consecutive_lvl1 = m_STeudaq.getConsecutiveLvl1TrigA(0);
	bore.SetTag("consecutive_lvl1", consecutive_lvl1);
	bore.SetTag("first_sensor_id", scan_options.first_sensor_id);
	bore.SetTag("tot_mode", tot_mode);

	SendEvent(bore);
	
	if(STEP_DEBUG) std::cout << "bore sent" << std::endl;

	// At the end, set the status that will be displayed in the Run Control.
    	SetStatus(eudaq::Status::LVL_BUSY, "Running");

	if(STEP_DEBUG) std::cout << "Set Status to running" << std::endl;
}

//This gets called whenever a run is stopped
void EUDAQProducer::OnStopRun()
{
	if(STEP_DEBUG) std::cout << "Stopping Run" << std::endl;

	//If the Run is supposed to stop, make sure that the ctrlthread is not halted
	//by paused PixController, thus unlock them to make sure!

	if(STPixModuleGroup::getGlobalReadout())
	{
		STPixModuleGroup::disableGlobalReadout();
	 	if(STEP_DEBUG) std::cout << "Enabling PixControllers for run to end!" << std::endl;
	}

	scanning = false;

	SetStatus(eudaq::Status::LVL_BUSY, "Sending Data");
	m_STeudaq.stopScan();

	// wait for STcontrol to finish the scan
	// has to be done in a waiting loop, because after exiting this function
	// eudaq will not update the status any more
	run_finished = false;
	eudaq::Timer t;
	while ((!run_finished || data_pending_running || !all_events_recieved) && t.Seconds()<120) 
	{
	  // we're running this in a separate thread, so shouldn't be necessary
	  //		m_app->processEvents();
	}

	//send remaining events to run control
	if(!all_events_recieved)
	{
		EUDAQ_WARN("Trigger Number differs for connected boards");
		sendEvents(true);
	}

	if(STEP_DEBUG) std::cout << "in OnStopRun: events sent"<< std::endl;

	// Send an EORE after all the real events have been sent
	if(STEP_DEBUG) std::cout << "sending EORE for event type " << EUDAQProducer::EVENT_TYPE << std::endl;
	SendEvent(eudaq::RawDataEvent::EORE(EUDAQProducer::EVENT_TYPE, m_run, m_ev));

	SetStatus(eudaq::Status::LVL_OK, "Stopped");
	if(STEP_DEBUG) std::cout << "Status: stopped!" << std::endl;
}

void EUDAQProducer::AbortRun()
{
	if(STEP_DEBUG) std::cout << "Aborting Run" << std::endl;
	scanning = false;

	SetStatus(eudaq::Status::LVL_BUSY, "Sending Data");
	m_STeudaq.stopScan();

	// Send an EORE after all the real events have been sent
	if(STEP_DEBUG) std::cout << "sending EORE for event type " << EUDAQProducer::EVENT_TYPE << std::endl;
	SendEvent(eudaq::RawDataEvent::EORE(EUDAQProducer::EVENT_TYPE, m_run, m_ev));
	
	SetStatus(eudaq::Status::LVL_ERROR, "Aborted");
 }

void EUDAQProducer::scanFinished()
{
	if(STEP_DEBUG) std::cout << "scanFinished: called"<< std::endl;
	eudaq::mSleep(2000);
	run_finished = true;
}

// This gets called when the Run Control is terminating, we should also exit.
void EUDAQProducer::OnTerminate()
{
	scanning = false;
	
	if(STEP_DEBUG) std::cout << "Terminating..." << std::endl;
	
	m_STeudaq.ProducerDisconnected();
}

void EUDAQProducer::OnPrepareRun(unsigned runnumber) 
{
	if(STEP_DEBUG) std::cout << "Preparing Run " << runnumber << std::endl;

	// Set ScanType to TESTBEAM_EUDAQ
	m_STeudaq.prepareRun( runnumber );
	eudaq::mSleep(4000);
}

void EUDAQProducer::OnUnrecognised(const std::string & cmd, const std::string & param) 
{
	if(STEP_DEBUG)
	{
		 std::cout << "Unrecognised: (" << cmd.length() << ") " << cmd;

		if(param.length()) 
		{
			std::cout << " (" << param << ")" << std::endl;
		}
	}

	SetStatus(eudaq::Status::LVL_WARN, "Unrecognised command");
}

void EUDAQProducer::errorReceived( std::string msg )
{
	SetStatus(eudaq::Status::LVL_ERROR, "See Log");
	EUDAQ_ERROR(msg);
}

void EUDAQProducer::dataPending(std::vector<unsigned int *>* data_vec, int boardid)
{

	if(STEP_DEBUG)
	{	
		std::cout << "dataPending(...) in "<< __FILE__ << " at " << __LINE__ << " with data_vec->size(): " << data_vec->size() << std::endl;
		std::cout << "dataPending: - ThreadID: " << QThread::currentThread() << std::endl;
	}

	if(abort_run)
	{
		
		if(STEP_DEBUG) std::cout << "Run aborted!" << std::endl;
			
		for( std::vector<unsigned int*>::iterator it = data_vec->begin(); it != data_vec->end(); it++)
		{
			delete[] (*it);
		}
		delete data_vec;
		return;
	}

	//lock until data is processed
	QMutexLocker locker(&mutex);
	data_pending_running = true;

	for(size_t chip = 0; chip < data_vec->size(); chip++)
	{

		std::cout << "dataPending(...): processing chip: " << chip << std::endl;

		//the data for this chip
		unsigned int* data = data_vec->at(chip);

		if(STEP_DEBUG)
		{
			std::cout << "First 10 words of data[] from chip: " << chip << std::endl;
			for(int i = 0; i <= 10; i++)
			{
				std::cout << data[i] << std::endl;
			}
		}

		all_events_recieved = false;

		if(STEP_DEBUG) std::cout << "got data from board: " << boardid << " - Current Trigger ID: " << lastTriggerNumber << std::endl;

		int lastDataIndex = -1;

		//index of board and chip in EventData and recieved_trigger
		int index = boardOffset[boardid]+chip;

		if(STEP_DEBUG) std::cout << "Searching for last trigger" << std::endl;
		
		//incremented if data word is zero
		int zero_counter = 0;
		//break after ten zero words
		const int max_zero = 10;
		
		//current trigger no
		int t_trigger_number = 0;
		//TLU multiplier due to 15bit rollover
		int multiplier = 0;

		//this is the loop over all data words!
		//checking data[i+1], so max. index is one lower than array size
		for (int i = 0; i < (SRAM_WORDSIZE_PER_CHIP-1) ; i++)
		{	
			if (data[i] == 0) 
			{
				zero_counter++;
			} 
			else 
			{
				//reset zero counter
				zero_counter = 0;

				if( TRIGGER_WORD_MACRO(data[i]) ) 
				{
					//decode Triggernumber
					t_trigger_number = TRIGGER_NUMBER_MACRO2(data[i], data[i+1])%TLU_TRIGGER_AMOUNT;

					//INIT lastTriggerNumber
					if(lastTriggerNumber == -1)
					{
						 lastTriggerNumber = t_trigger_number-1;
					}

					// The TLU returns only 15 bit numbers, so we hfave to be aware of a restart
					if (recieved_trigger[index] == -1)
					{
						 multiplier = 0;
					}
					else 
					{
						multiplier = recieved_trigger[index]/TLU_TRIGGER_AMOUNT;
					}

					//increase multiplier only when close to the end of the trigger range
					if(
						(t_trigger_number < (recieved_trigger[index]%TLU_TRIGGER_AMOUNT)) && 
						((recieved_trigger[index]%TLU_TRIGGER_AMOUNT)>(TLU_TRIGGER_AMOUNT - 50))
					  ) 
					{
						multiplier++;
						recieved_trigger[index] = (multiplier*TLU_TRIGGER_AMOUNT)+t_trigger_number;
					} 
					else if (t_trigger_number > (recieved_trigger[index]%TLU_TRIGGER_AMOUNT))
					{
						recieved_trigger[index] = (multiplier*TLU_TRIGGER_AMOUNT)+t_trigger_number;
					}
						
					//increase i by 1, because trigger number is split into two words
					i++;
					lastDataIndex = i;
				}
			}

			// break loop if there are sufficient zeros
			if (zero_counter>max_zero)
			{
				//TODO: Give out error
				if(STEP_DEBUG) std::cout << "zero_counter>" << max_zero<<" -> exiting trigger finder loop, halting at: word index i=" << i << std::endl;
				break;
			}
		} //loop over data words

		if(lastDataIndex==-1)
		{
			if(STEP_DEBUG) std::cout << "LAST DATA INDEX is -1, nothing to do" << std::endl;
			delete[] data;
			data = NULL;
		
			//if we processed the last chip on the board, and this happens, there is no data pending anymore
			if(chip == data_vec->size()-1)
			{
				data_pending_running=false;
			}

			continue;
		}

		if(STEP_DEBUG) std::cout << "MAX TRIGGER NUMBER FOR BOARD : " << index << " : " << recieved_trigger[index] << 
		" : " << recieved_trigger[index] % TLU_TRIGGER_AMOUNT << "\nLAST DATA INDEX: " << lastDataIndex << std::endl;

		if(STEP_DEBUG) std::cout << "Inserting data into EventData[" << index << "]" << std::endl;

		//Instert data in EventData
		for (int i = 0; i <=lastDataIndex ; i++)
		{
			EventData[index].push_back(data[i]);
		}

		//Clean up data
		delete[] data;
		data = NULL;
	}//loop over all chips on board

	//check if all events have been received, i.e. if all chips have processed the same triggers:
	if(
	       *std::max_element(recieved_trigger.begin(), recieved_trigger.end()) 
	    == *std::min_element(recieved_trigger.begin(), recieved_trigger.end())
	  ) 
	{
		all_events_recieved = true;
	}

	//clean up vector carrying the data ptrs
	delete data_vec;
	data_vec=NULL;

	//finally send the data
	sendEvents();
	
	//unlock data processing function
	locker.unlock();
	
	//Increment the amount of read out boards!
	boards_read_out++;
		
	//If all boards sent their data and they have been processed, unlock the PixControllers
	if(boards_read_out == board_count)
	{
		STPixModuleGroup::disableGlobalReadout();
		std::cout << "Enabling PixControllers!" << std::endl;
		boards_read_out = 0;
	}
}

unsigned int EUDAQProducer::encodeTriggerNumber(unsigned int trigger_number, bool first) {
	// this trigger is good => save result!
	unsigned int word1, word2;
	word1=(TRIGGER_WORD_HEADER | (TRIGGER_NUMBER_31_24_MASK & ((trigger_number) >> 24)));
	word2=(TRIGGER_NUMBER_23_0_MASK & (trigger_number));
	if (TRIGGER_NUMBER_MACRO2(word1, word2) != trigger_number)
		EUDAQ_WARN((QString("Corrected Trigger missmatch: ") + QString::number(TRIGGER_NUMBER_MACRO2(word1, word2))).toStdString());

	if (first) return word1;
	else return word2;
}

void EUDAQProducer::sendEvents(bool endrun) 
{
	if(STEP_DEBUG) std::cout << "sendEvents: ThreadID: " << QThread::currentThread() << std::endl;

	// Lock the function until the data is processed
	QMutexLocker locker(&mutex_send);

	// Check if there are completed events
	int min_trigger_number;
	//int cur_BoardIndex;
	unsigned int Word, TriggerWord2;
	std::vector<unsigned int> one_event;
	bool isTriggerWord, proceedLoop;

	//if endrun then send all the data that is currently stored
	if(endrun) 
	{
		if(STEP_DEBUG) std::cout << "sendEvents: ENDRUN!" << std::endl;
		min_trigger_number = *std::max_element(recieved_trigger.begin(), recieved_trigger.end());
	} 
	else 
	{
		min_trigger_number = *std::min_element(recieved_trigger.begin(), recieved_trigger.end());
	}
	
	if(STEP_DEBUG) std::cout << "MIN TRIGGER NUMBER: " << min_trigger_number << std::endl;

	// Return if there are no events to be send to Run Control
	if(min_trigger_number == -1) 
	{
		data_pending_running=false;
		return;
	}

	if(STEP_DEBUG) std::cout << "LAST TRIGGER NUMBER: " << lastTriggerNumber << std::endl;
	
	int triggerCounter = lastTriggerNumber+1;
	int triggerNumber;

	if(STEP_DEBUG) std::cout << "creating RawDataEvent for event type " << EUDAQProducer::EVENT_TYPE << std::endl;

	while(triggerCounter <= min_trigger_number) 
	{
		triggerNumber = triggerCounter % TLU_TRIGGER_AMOUNT;

		if(STEP_DEBUG && triggerNumber%1000 == 0 ) std::cout << "Sending Event " << triggerNumber << std::endl;

		//Create a RawDataEvent to contain the event data to be sent
		eudaq::RawDataEvent ev (EUDAQProducer::EVENT_TYPE, m_run, m_ev);

		//Now search all chips for events with this trigger number
		for (size_t chip = 0; chip < boardChips.size(); chip++) 
		{

			//if(STEP_DEBUG) std::cout << "sendEvent() for chip: " << chip << std::endl;
			proceedLoop=true;

			while(proceedLoop)
			{

				//std::cout << "In proceedLoop!" << std::endl;
				isTriggerWord=false;
				one_event.clear();
				unsigned int dh_counter=0;
	
				while( !isTriggerWord && !EventData[chip].empty())
				 {
					//std::cout << "In !isTriggerWorld loop" << std::endl;

					//std::cout << "Front access!" << std::endl;
					Word = EventData[chip].front();
					//std::cout << "Front delete!" << std::endl;
					EventData[chip].pop_front();

		     			//std::cout << "trying TRIGGER_WORD_MACRO or DATA_HEADER_MACRO: "; 
					
					if(TRIGGER_WORD_MACRO(Word))
					{
						//std::cout << "success!" << std::endl;
						isTriggerWord = true;
					}
					else if(DATA_HEADER_MACRO(Word))
					{
						//std::cout << "success!" << std::endl;
						dh_counter++;
		      			}

					one_event.push_back(Word);
		    		}
		    
				// if we got here, this should be trigger word, but let's check again
		    		if(isTriggerWord)
				{

					//std::cout << "Found trigger word!" << std::endl;

					// get 2nd trigger word
					TriggerWord2 = EventData[chip].front();
					EventData[chip].pop_front();
					one_event.push_back(TriggerWord2);
		      
					//check if it is the desired trigger number
		      			if(( (int)(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))%TLU_TRIGGER_AMOUNT) < triggerNumber)
					{
						std::cout << "Found trigger whch should already have been processed!" << std::endl;
						//Events should have already been send in an earlier loop, process loop once more
						EUDAQ_WARN(QString("Trigger Number to small. recieved: " + QString::number(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2)) + 
						" for board " + /*String::number(board_ids[cur_BoardIndex]) + */" - expected: " + QString::number(triggerNumber)).toStdString().c_str());
			
						one_event.clear();
		      			}
					else
					{
						if( ((int)(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))%TLU_TRIGGER_AMOUNT) > triggerNumber)
						{
							std::cout << "Trigger missing!";
	
							// Trigger Number missing
							EUDAQ_INFO(QString("Trigger Number " + QString::number(triggerNumber) + " is missing for board ").toStdString()) ;// + 
							//QString::number(board_ids[cur_BoardIndex]) ).toStdString().c_str());
			  
							//Put Data back in list:
							EventData[chip].insert(EventData[chip].begin(), one_event.begin(), one_event.end());
							one_event.clear();
			  
							//Add two dummy trigger blocks (no data!)!
							one_event.push_back(encodeTriggerNumber(triggerNumber, true));
							one_event.push_back(encodeTriggerNumber(triggerNumber, false));
							ev.AddBlock(chip, one_event);
							proceedLoop = false;
						} 
						else
					 	{
							//Trigger Number fits, check for consistency
							if(dh_counter != consecutive_lvl1)
							{
								//SINCE WITH CURRENT 4CHIP MODULES IT OFTEN HAPPENS THAT A MODULE IS D 
								//std::cout << "On chip: " << chip<< ", DataHeader count is: " << dh_counter << " whereas " << 
								//consecutive_lvl1 << "consecutive lvl1 triggers were expected!" << std::endl; 
								
								//EUDAQ_WARN((QString("Wrong number of data header (") + QString::number(dh_counter) + 
								//QString(") for chip ") + chip +*/ QString(" at trigger ") + 
								//QString::number(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))).toLatin1().data());
			  				}
			  
							//add data (hit, EoE words and Trigger word to event)
			  				ev.AddBlock(chip, one_event);
			  				proceedLoop = false;
						}
		      			}
		    		} 
				else 
				{
					if(STEP_DEBUG) std::cout << "No Event: " << triggerNumber << " chip: " << chip << std::endl;
					one_event.clear();
	
					//Add two dummy trigger blocks (no data!)!
					one_event.push_back(encodeTriggerNumber(triggerNumber, true));
					one_event.push_back(encodeTriggerNumber(triggerNumber, false));
					ev.AddBlock(chip, one_event);
					proceedLoop=false;
				}
			}
		}
		
		try 
		{
			SendEvent(ev);
		} 
		catch (...)
		{
			if(STEP_DEBUG) std::cout << "ERROR: unable to send event " << m_ev << std::endl;
			break;
		}

		triggerCounter++;
		m_ev++;
	}

	lastTriggerNumber = triggerCounter-1; //triggerNumber;
	data_pending_running = false;

	if(STEP_DEBUG) std::cout << "Data send to Trigger: " << lastTriggerNumber << std::endl;
}
