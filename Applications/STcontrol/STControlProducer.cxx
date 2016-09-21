#include <iostream>
#include <chrono>
#include <thread>

#include <eudaq/Logger.hh>

#include "PixController/PixController.h"

#include "STControlProducer.h"
#include "STControlEngine.h"
#include "STRodCrate.h"
#include "STPixModuleGroup.h"

#include <QFile>
#include <QDir>

#define STE_DEBUG 1
#define STEP_DEBUG 1

STControlProducer::STControlProducer(STControlEngine& engine, std::string const& prdname, std::string const& runctrl): 
eudaq::Producer(prdname, runctrl),
m_STControlEngine(engine),
m_rcAddress(runctrl) {
	std::cout << "Started STControlProducer" << std::endl;
}

std::vector<PixLib::PixController*> STControlProducer::getPixControllers(){
	auto result = std::vector<PixLib::PixController*>();
	auto rodCrates = m_STControlEngine.getSTRodCrates();
	for(auto& crate: rodCrates) {
		auto pixModuleGroups = crate->getSTPixModuleGroups();
		for(auto& pixModGrp: pixModuleGroups) {
			result.push_back(pixModGrp->getPixController());
		}
	}
	return result; //NRVO will kick in
}
 
void STControlProducer::OnConfigure(const eudaq::Configuration& config){

	SetStatus(eudaq::Status::LVL_BUSY, "Configuring");

	//Get Configuration Data
	QStringList config_files, config_modules, trigger_replication_modes;
	QString config_file, config_module, trigger_replication_mode;

	scan_options.boards = QString::fromStdString(config.Get("boards", "no")).split(",", QString::SkipEmptyParts);

	for(size_t i=0; i < (size_t)scan_options.boards.size() ; i++) {
		//Module configuration
		QStringList chipsOnBoard = QString::fromStdString( config.Get((QString("modules[") + scan_options.boards[i] + QString("]")).toStdString(), "no") ).split(",", QString::SkipEmptyParts);
		int boardID = scan_options.boards[i].toInt();	
	}

	//Check if the skip configuration was set
	scan_options.SkipConfiguration = (QString::fromStdString(config.Get("SkipConfiguration", "no")).toLower()=="yes");	
	if(scan_options.SkipConfiguration) {
		//No configuration requested
		SetStatus(eudaq::Status::LVL_OK, "Configured (" + config.Name() + ")");
		return;
	}

	scan_options.SRAM_READOUT_AT = config.Get("SRAM_READOUT_AT", 30);
	scan_options.UseSingleBoardConfig = (QString::fromStdString(config.Get("UseSingleBoardConfigs", "no")).toLower()=="yes");
	scan_options.config_file = QString::fromStdString( config.Get("config_file", ""));
     
	if (scan_options.UseSingleBoardConfig || scan_options.config_file=="") {
		scan_options.UseSingleBoardConfig = true;

		bool send_error=false;

		for(size_t i=0; i<(size_t)scan_options.boards.size(); i++) {
			//config file for current board
			config_file = QString::fromStdString(config.Get((QString("config_file[") + scan_options.boards[i] + QString("]")).toStdString().c_str(), "")).trimmed();
			if(config_file=="") {
				config_file=QString::fromStdString(config.Get("config_file[*]", "")).trimmed();
			}
			if(config_file=="" || !QFile::exists(config_file)) {
				// No valid config
				EUDAQ_ERROR((QString("No valid config for board ") + scan_options.boards[i]).toStdString().c_str());
				send_error=true;
			}

			//Module from Config file to use
			config_module=QString::fromStdString(config.Get((QString("config_module[") + scan_options.boards[i] + QString("]")).toStdString().c_str(), "")).trimmed();
			if(config_module=="") {
				config_module=QString::fromStdString(config.Get("config_module[*]", "")).trimmed();
			}

			//Trigger replication mode
			trigger_replication_mode=QString::fromStdString(config.Get((QString("trigger_replication[") + scan_options.boards[i] + QString("]")).toStdString().c_str(), "")).toLower();
			if (trigger_replication_mode=="") {
				trigger_replication_mode=QString::fromStdString(config.Get("trigger_replication[*]", "off")).toLower();
			}
			if (trigger_replication_mode!="off" && trigger_replication_mode!="master" && trigger_replication_mode!="slave") {
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

		if(send_error) {
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
     
	if(STEP_DEBUG) std::cout << "Configuring: " << config.Name() << std::endl;
	auto initSuccess = InitControllers(scan_options);

	if(initSuccess){
		// set the status that will be displayed in the Run Control.
		SetStatus(eudaq::Status::LVL_OK, "Configured (" + config.Name() + ")");
	} else {
		SetStatus(eudaq::Status::LVL_ERROR, "Error while initializing PixControllers");
	}     
}

bool STControlProducer::InitControllers(extScanOptions ScanOptions) {

	m_STControlEngine.setShowErrorPopups(false);

	// Load Config
	if(m_STControlEngine.getPixConfDBFname()!=0) {
		m_STControlEngine.clear();
		m_STControlEngine.setPixConfDBFname("");
	}
	
	if(ScanOptions.UseSingleBoardConfig || ScanOptions.config_file=="") {
		CreateMultiBoardConfig(ScanOptions);
	} else {
		m_STControlEngine.loadDB(ScanOptions.config_file.toLatin1().data());
	}
	
	//QApplication::processEvents();

	m_STControlEngine.initRods();
	m_STControlEngine.initDcs();

	auto nRods = m_STControlEngine.CtrlStatusSummary(); 
 
	if(nRods<=0) {
		//Error
	//	configured = 2;
	//	m_producer->configured(false);
		return false;
    }

	//m_STControlEngine.processExecute();

	//power up fe
	emit m_STControlEngine.powerOn();
	//Wait a second for the voltage to stabilize
	std::this_thread::sleep_for(std::chrono::milliseconds(750));    

	// configure modules
	m_STControlEngine.configModules();

	if(STE_DEBUG) std::cout << "EUDAQ: Modules configured, retrieving board IDs" << std::endl;

	auto myBoards = m_STControlEngine.GetBoardIDs();

	if(STE_DEBUG) std::cout << "EUDAQ: Boards checked, retrieving FE flavours" << std::endl;
	std::map<int, std::string> smff = m_STControlEngine.GetFeFlavours();
	std::vector<int> feFlavours;
	
	for(size_t i=0; i<myBoards.size();i++)
	{
		int feint=-1;
		if	(smff[myBoards[i]]=="FE_I1")  feint = 1;
		else if	(smff[myBoards[i]]=="FE_I2")  feint = 1;
		else if	(smff[myBoards[i]]=="FE_I4A") feint = 2;
		else if	(smff[myBoards[i]]=="FE_I4B") feint = 3;
		feFlavours.push_back(feint);
	}
	ScanOptions.FEflavour = (myBoards.size()>0)?feFlavours[0]:3;

	if(STE_DEBUG) std::cout << "EUDAQ: got FE flavours, setting scan options now!" << std::endl;

	emit m_STControlEngine.setScanOptions(ScanOptions);
//	QApplication::processEvents();

//	configured = 1;

	if(STE_DEBUG) std::cout << "EUDAQ: Scan options set" << std::endl;
	
	auto tot_mode = m_STControlEngine.GetHitDiscCnfg();

//	m_producer->configured(true, myBoards, feFlavours, tot_mode);
//	initializing=false;
	return true;
}

QString STControlProducer::CreateMultiBoardConfig(extScanOptions& ScanOptions) {
	// Determine filename for new Multiboardconfig
	QDir cur_dir=QDir(QApplication::applicationDirPath());
	if (cur_dir.dirName()==QString("bin")) {
		// move to config dir
		cur_dir.cdUp();
		cur_dir.cd("config");
	}

	int filecounter=0;
	QString my_fname=QDir::cleanPath(cur_dir.absolutePath() + "/eudaq_multiboard_00000.cfg.root");
	while (QFile::exists(my_fname)) { 
		filecounter++;
		my_fname = QDir::cleanPath(cur_dir.absolutePath() + QString("/eudaq_multiboard_") + QString::number(filecounter).rightJustified(5, '0') + ".cfg.root");
	}
	if(STE_DEBUG) std::cout << "Writing Multiboard Config to " << my_fname.toStdString() << std::endl;
	m_STControlEngine.createMultiBoardConfig(my_fname, ScanOptions);
	return my_fname;
}

void STControlProducer::OnStartRun (unsigned param){}
void STControlProducer::OnStopRun (){}
void STControlProducer::OnTerminate (){}
void STControlProducer::OnUnrecognised(const std::string & cmd, const std::string & param){}
void STControlProducer::OnPrepareRun(unsigned runnumber){}
void STControlProducer::OnStatus(){}