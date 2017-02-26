#include "STeudaq.h"
#include "STeudaq_producer.h"

#include <QDir>
#include <QFile>
#include <QApplication>
#include <QThread>
#include <QEvent>

#include <map>

#define STE_DEBUG true

STEUDAQ::STEUDAQ(STControlEngine& engine): QObject( &engine ), m_STControlEngine(&engine)
{
	status = disconnected;
	m_producer_id = 0;
	m_producer = 0;
	if(STE_DEBUG) std::cout << "STEUDAQ: started with thread with ID: " << QThread::currentThreadId() << std::endl;

	initializing = false;

	connect(m_STControlEngine,SIGNAL(finishedScanning()), this, SLOT(scanFinished()));
	connect(m_STControlEngine,SIGNAL(beganScanning()), this, SLOT(scanStarted()));
	connect(m_STControlEngine,SIGNAL(error( std::string )), this, SLOT(errorReceived( std::string )));
	connect(m_STControlEngine,SIGNAL(error( std::string )), this, SLOT(errorReceived( std::string )));
};

STEUDAQ::~STEUDAQ()
{
	stop();
	if (status != disconnected) setState(disconnected);
	delete m_producer;
	if(STE_DEBUG) std::cout << "STEUDAQ: finished" << std::endl;
};

std::string STEUDAQ::getProducerName()
{
	if (m_producer_id==0)
		return "USBpix";
	else
		return (QString("USBpix-") + QString::number(m_producer_id)).toStdString();
}

void STEUDAQ::errorReceived ( std::string msg )
{
	if(STE_DEBUG) std::cout << "EUDAQ ERROR: " << msg << std::endl;
	m_producer->errorReceived(msg);
}

void STEUDAQ::start()
{
	start(rc_address);
}

void STEUDAQ::scanStarted()
{
	if (configured==1)
	{
	  m_producer->beganScanning();
	  return;
	}
}

void STEUDAQ::scanFinished()
{
	if (configured==1)
	{
	  m_producer->scanFinished();
	  return;
	}
}

bool STEUDAQ::start(QString RunControlAddress)
{
  if(STE_DEBUG) std::cout << "STEUDAQ: start producer" << std::endl;
	
  	if (RunControlAddress == "")
	{
		setState (connectionerror);
		return false;
	}

	initializing = false;
	configured = 0;
	rc_address = RunControlAddress;
	setState(connecting);

	try
	{
		
	  if(STE_DEBUG) std::cout << "Starting producer... ";
	  m_producer = new EUDAQProducer(*this, getProducerName(), rc_address.toStdString());
	  if(STE_DEBUG) std::cout << "producer started!" << std::endl;
	  setState(connected);
	  
	  return true;
	}
	catch (...) 
	{
	  setState(connectionerror);
	  return false;
	}
};

void STEUDAQ::stop()
{
  ProducerDisconnected();
  if(m_producer!=0){
    delete m_producer;
    m_producer=0;
  }
}

void STEUDAQ::startCurrentScanSlot(QString run_label, QString raw_filename)
{
  // emit STControlEngine::startCurrentScan via main event loop to get out of producer's thread
  prodStartScanEvent* psse = new prodStartScanEvent(run_label, raw_filename);
  m_app->postEvent(this, psse);
}

void STEUDAQ::ProducerDisconnected()
{
	setState(STEUDAQ::disconnected);
	if(STE_DEBUG) std::cout << "EUDAQ: Producer Disconnected" << std::endl;
}

void STEUDAQ::initControler(extScanOptions ScanOptions){
  // calls initControlerTS via main event loop to get out of producer's thread
  prodInitEvent* pie = new prodInitEvent(ScanOptions);
  m_app->postEvent(this, pie);

}
void STEUDAQ::initControlerTS(extScanOptions ScanOptions)
{
	if(STE_DEBUG) std::cout << "EUDAQ CONFIG: " << ScanOptions.config_file.toLatin1().data() << std::endl;

	m_STControlEngine->setShowErrorPopups(false);

	// Load Config
	if(m_STControlEngine -> getPixConfDBFname()!=0) 
	{
		m_STControlEngine -> clear();
		m_STControlEngine -> setPixConfDBFname("");
	}

	ScanParameters = ScanOptions;
	
	if(ScanOptions.UseSingleBoardConfig || ScanOptions.config_file=="")
	{
	  CreateMultiBoardConfig();
	}
	else
	{
	  m_STControlEngine -> loadDB(ScanOptions.config_file.toLatin1().data());
	}
	
	if(STE_DEBUG) std::cout << "EUDAQ SCAN: " << ScanParameters.scan_type.toStdString() << std::endl;
	
	QApplication::processEvents();

	//if(STE_DEBUG) std::cout << "EUDAQ-Producer initControler - Thread-ID: " << QThread::currentThreadId() << std::endl;
	initializing = true;

	int nRods;
	if(STE_DEBUG) std::cout << "EUDAQ: Init Controller" << std::endl;
	configured = 0;

	m_STControlEngine -> initRods();
	m_STControlEngine -> initDcs();

	nRods = m_STControlEngine -> CtrlStatusSummary(); 
 
	if(nRods<=0)
	{
		//Error
		configured = 2;
		m_producer->configured(false);
		return;
       }

	m_STControlEngine -> processExecute();

	//power up fe
	emit m_STControlEngine -> powerOn();
	//Wait a second for the voltage to stabilize
	QTimer::singleShot(1000, this, SLOT(initControler2()));
}

void STEUDAQ::initControler2() 
{
	// continue with initialization
	if(STE_DEBUG) std::cout << "EUDAQ: Continue initialisation, configuring modules" << std::endl;

	// configure modules
	m_STControlEngine->configModules();

	if(STE_DEBUG) std::cout << "EUDAQ: Modules configured, retrieving board IDs" << std::endl;

	QVector<int> myBoards = QVector<int>::fromStdVector(m_STControlEngine -> GetBoardIDs());

	if(STE_DEBUG) std::cout << "EUDAQ: Boards checked, retrieving FE flavours" << std::endl;
	std::map<int, std::string> smff = m_STControlEngine->GetFeFlavours();
	QVector<int> feFlavours;
	
	for(int i=0;i<myBoards.size();i++)
	{
		int feint=-1;
		if	(smff[myBoards[i]]=="FE_I1")  feint = 1;
		else if	(smff[myBoards[i]]=="FE_I2")  feint = 1;
		else if	(smff[myBoards[i]]=="FE_I4A") feint = 2;
		else if	(smff[myBoards[i]]=="FE_I4B") feint = 3;
		feFlavours.push_back(feint);
	}
	ScanParameters.FEflavour = (myBoards.size()>0)?feFlavours[0]:3;

	if(STE_DEBUG) std::cout << "EUDAQ: got FE flavours, setting scan options now!" << std::endl;

	emit m_STControlEngine->setScanOptions(ScanParameters);
	QApplication::processEvents();

	configured = 1;

	if(STE_DEBUG) std::cout << "EUDAQ: Scan options set" << std::endl;
	
	int tot_mode = m_STControlEngine -> GetHitDiscCnfg();

	m_producer->configured(true, myBoards, feFlavours, tot_mode);
	initializing=false;
}

// Reads data from the USBpix boards
// void STEUDAQ::ReadData()
// {
// 	m_STControlEngine -> forceRead();
// }

int STEUDAQ::getBoardsConnected()
{
	return m_STControlEngine -> CtrlStatusSummary();
}

void STEUDAQ::stopScan()
{
  // calls STControlEngine::stopPixScan via main event loop to get out of producer's thread
  prodStopScanEvent* psse = new prodStopScanEvent();
  m_app->postEvent(this, psse);
}

void STEUDAQ::prepareRun( unsigned )
{
  // emit STControlEngine::setScanOptions via main event loop to get out of producer's thread
  prodPrepScanEvent* ppse = new prodPrepScanEvent();
  m_app->postEvent(this, ppse);
}

QString STEUDAQ::CreateMultiBoardConfig()
{
	// Determine filename for new Multiboardconfig
	QDir cur_dir=QDir(QApplication::applicationDirPath());
	if (cur_dir.dirName()==QString("bin"))
	{
		// move to config dir
		cur_dir.cdUp();
		cur_dir.cd("config");
	}

	int filecounter=0;
	QString my_fname=QDir::cleanPath(cur_dir.absolutePath() + "/eudaq_multiboard_00000.cfg.root");
	while (QFile::exists(my_fname))
	{ 
		filecounter++;
		my_fname = QDir::cleanPath(cur_dir.absolutePath() + QString("/eudaq_multiboard_") + QString::number(filecounter).rightJustified(5, '0') + ".cfg.root");
	}
	if(STE_DEBUG) std::cout << "Writing Multiboard Config to " << my_fname.toStdString() << std::endl;
	m_STControlEngine->createMultiBoardConfig(my_fname, ScanParameters);
	return my_fname;
}

int STEUDAQ::getConsecutiveLvl1TrigA(int superGroup)
{
  return m_STControlEngine->getConsecutiveLvl1TrigA(superGroup);
}
int STEUDAQ::getConsecutiveLvl1TrigB(int superGroup)
{
  return m_STControlEngine->getConsecutiveLvl1TrigB(superGroup);
}

void STEUDAQ::dataPending(std::vector<unsigned int *>* data, int boardid){
  m_producer->dataPending(data, boardid);
}
void STEUDAQ::ScanStatusSignal(int boardid, bool SRAMFullSignal, int SRAMFillingLevel, int TriggerRate){
  m_producer->ScanStatus(boardid, SRAMFullSignal, SRAMFillingLevel, TriggerRate);
}
void STEUDAQ::customEvent( QEvent * event ){
  switch((int)event->type()){
  case 3001:           // initialisation
    initControlerTS(dynamic_cast<prodInitEvent*>(event)->options());
    break;
  case 3002:           // start scan
    emit m_STControlEngine->startCurrentScan(dynamic_cast<prodStartScanEvent*>(event)->runLabel(),
					     dynamic_cast<prodStartScanEvent*>(event)->rawFilename());
    break;
  case 3003:           // stop scan
    m_STControlEngine->stopPixScan();
    break;
  case 3004:           // prepare scan
    emit m_STControlEngine->setScanOptions(ScanParameters);
    break;
  default:
    // do nothing
    break;
  }
}
