#include <eudaq/Timer.hh>
#include <eudaq/RawDataEvent.hh>
#include <eudaq/Logger.hh>

#include "STeudaq_producer.h"
#include "STeudaq.h"
//required for setting static tb flags
#include "STPixModuleGroup.h"

#include <QFile>
#include <QDir>
#include <QThread>

#include <iterator>

using eudaq::to_string;

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
		for (unsigned int i=0; i<board_ids.size(); i++) 
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

	if(EUDAQProducer::EVENT_TYPE == "USBPIX") dataPendingI3(data_vec, boardid); // for FE-I2/3
	else                                      dataPendingI4(data_vec, boardid); // for FE-I4A/B

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

void EUDAQProducer::sendEvents(bool endrun) 
{
  if(STEP_DEBUG) std::cout << "sendEvents: ThreadID: " << QThread::currentThread() << std::endl;
  
  // Lock the function until the data is processed
  QMutexLocker locker(&mutex_send);
  
  if(EUDAQProducer::EVENT_TYPE == "USBPIX") sendEventsI3(endrun); // for FE-I2/3
  else                                      sendEventsI4(endrun); // for FE-I4A/B

}
