#include <sstream>

#include "MainForm.h"
//#include "AboutForm.h"
#include "SiUSBLib.h"

#ifdef __VISUALC__
  #include <Windows.h>
  #pragma warning(disable: 4100)
#else
  #define __stdcall
  #include <iostream>
#endif


void __stdcall debug_out(std::string text)
{
#ifdef CF__LINUX
	std::cerr << text << std::endl;
#else
	OutputDebugStringA(text.c_str());
#endif
}



MainForm::MainForm(QWidget *):QMainWindow(),ui(new Ui::MainForm)
{
  ui->setupUi(this);
	silab_usb_ico = new QIcon(":/Icons/resources/usb.ico");
	QStringList header;
	header << "Id" << "Name" << "Class" << "FW";
	ui->devListTree->setHeaderItem(new QTreeWidgetItem((QTreeWidget *)0, header));
  ui->devListTree->setColumnCount(4);
	ui->devListTree->setColumnWidth(0,60);
	ui->devListTree->setColumnWidth(1,120);
	ui->devListTree->setColumnWidth(2,50);
	ui->devListTree->setColumnWidth(3,50);
	ui->devListTree->setIconSize(QSize(16, 16));
	ui->devListTree->show();

	
	curve1 = new QwtPlotCurve("write");
	curve2 = new QwtPlotCurve("read");
// 	curve1->attach(ui->benchPlot);
// 	curve2->attach(ui->benchPlot);
	curve1->setPen(QPen(Qt::blue));
	curve2->setPen(QPen(Qt::green));

// 	ui->benchPlot->setAxisScale(QwtPlot::xBottom, 0, 64, 8);
// 	ui->benchPlot->setAxisScale(QwtPlot::yLeft, 0, 30, 0);
// 	ui->benchPlot->setAxisTitle(QwtPlot::xBottom, QwtText("block size [kbyte]"));
//   ui->benchPlot->setAxisTitle(QwtPlot::yLeft,  QwtText("transfer rate [Mbyte/sec]"));
// 	ui->benchPlot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);


//   double x[100];
//   double y[100];

// 	for (int i = 0; i < 100; i++)
// 	{  
// 		x[i] = i;
//     y[i] = 50 + 25*sin(1/10.0*x[i]);
// 	}
//   ui->benchPlot->axisAutoScale(QwtPlot::xBottom);
//   ui->benchPlot->axisAutoScale(QwtPlot::yLeft);
//   curve1->setSamples(x, y, 100);
//   ui->benchPlot->replot();

  createActions();

  InitUSB();
  myUSBdev = new SiUSBDevice(NULL);
  UpdateDevList();
  SetCurrentUSBDevice(-1);
  expertModeChanged();
  ui->flashProgressBar->hide();

  QTimer *timer = new QTimer( this );
  connect(timer, SIGNAL(timeout()), this, SLOT(onDeviceChange()) );
  timer->start(50);

}

MainForm::~MainForm(void)
{
  delete myUSBdev;
  TermUSB();
  delete ui;
}

void MainForm::createActions()
{
		showAboutAct = new QAction(tr("&About..."), this);
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

		expertModeAct = new QAction(tr("&Expert mode"), this);
    connect(ui->actionExpertMode, SIGNAL(triggered()), this, SLOT(expertModeChanged()));
}

void MainForm::showAbout()
{
	//myAboutForm->show();
	QMessageBox ad;
	ad.setWindowTitle("USB Device Manager");

	ad.setText( "This is the SILAB USB device manager GUI\n"
		"For updates an more information see:\n"
		"http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/WebHome");
    
 ad.setInformativeText("Hans Krueger (krueger@physik.uni-bonn.de)");
 ad.setStandardButtons(QMessageBox::Ok);
 ad.setIconPixmap(QPixmap(":/Icons/resources/silab_usb.ico"));
 ad.exec();
 ad.close();
}

void MainForm::expertModeChanged()
{
	int indx;
	if (ui->actionExpertMode->isChecked())
	{
		ui->firmwareTab->setEnabled(true);
		ui->fpgaTab->setEnabled(true);
		if ((indx = ui->modeComboBox->findText("EEPROM", Qt::MatchFixedString) ) != -1)
 	    ui->modeComboBox->removeItem(indx);  
		ui->modeComboBox->addItem("EEPROM");
	}
	else
	{
		ui->firmwareTab->setEnabled(false);
		ui->fpgaTab->setEnabled(false);
		ui->modeComboBox->removeItem(ui->modeComboBox->findText("EEPROM", Qt::MatchFixedString));
	}
}



void MainForm::SetCurrentUSBDevice(int id)
{
	std::stringstream ssbuf;
	std::string sbuf;
	void *hUSB = GetUSBDevice(id);

	if (hUSB != NULL)
	{
		myUSBdev->SetDeviceHandle(hUSB);
		ssbuf << "Current device: " << myUSBdev->GetName() << " with ID " << (int)myUSBdev->GetId();
		statusBar()->showMessage(ssbuf.str().c_str());
	  ui->infoWindow->setText(QString(myUSBdev->GetEndpointInfo()));
		ui->classLineEdit->setText(QString::number(myUSBdev->GetClass()));
		ui->IdLineEdit->setText(QString::number(myUSBdev->GetId()));
		ui->nameLineEdit->setText(QString(myUSBdev->GetName()));
	}
	else
	{
		myUSBdev->SetDeviceHandle(NULL);
		statusBar()->showMessage("No USB device found or selected.");
		ui->infoWindow->setText("");
	}
}

void MainForm::DisplayDeviceInfo()
{
	ui->infoWindow->setText(QString(myUSBdev->GetEndpointInfo()));
}

void MainForm::onDeviceChange()
{
	if (OnDeviceChange()) // call to USBBoardLib.dll
	{
		debug_out("Called onDeviceChange()\n");
		UpdateDevList();
		SetCurrentUSBDevice(-1);
	}

}

void MainForm::openFPGADialog()
{
	QFileDialog fd;
	static QByteArray dialogState;
	QStringList sl;

	fd.setNameFilter("Bit Files (*.bit)");
	fd.setWindowTitle("Select FPGA configuration file");
  fd.restoreState(dialogState);
	fd.setModal(true);
	fd.show();
	
	if (fd.exec() == QDialog::Accepted)
	{
 	  sl = fd.selectedFiles();
    ui->fpgaFileLineEdit->setText(sl[0]);
	}
	dialogState = fd.saveState();
}

void MainForm::openControllerDialog()
{
	QFileDialog fd(this); 
	static QByteArray dialogState;
	QStringList sl;

	fd.setNameFilter("BIX Files (*.bix)");
  fd.setViewMode(QFileDialog::Detail);
	fd.setWindowTitle("Select USB controller firmware");
  fd.restoreState(dialogState);
	fd.setModal(true);
	

  fd.show();
	
	if (fd.exec() == QDialog::Accepted)
	{
 	  sl = fd.selectedFiles();
    ui->controllerFileLineEdit->setText(sl[0]);
	}
	dialogState = fd.saveState();
}
  
  

void MainForm::openEEPROMDialog()
{
	QFileDialog fd;
	static QByteArray dialogState;
	QStringList sl;

	fd.setNameFilter("HEX Files (*.hex)");
	fd.setWindowTitle("Select USB controller firmware");
  fd.restoreState(dialogState);
	fd.setModal(true);
	fd.show();
	
	if (fd.exec() == QDialog::Accepted)
	{
 	  sl = fd.selectedFiles();
    ui->eepromFileLineEdit->setText(sl[0]);
		EEPROMFileName = ui->eepromFileLineEdit->text();
	}
	dialogState = fd.saveState();

}

void MainForm::loadFPGA()
{
	if (!ui->fpgaFileLineEdit->text().isEmpty())
	  myUSBdev->DownloadXilinx(ui->fpgaFileLineEdit->text().toLatin1().data());
}

void MainForm::loadController()
{
	if (!ui->controllerFileLineEdit->text().isEmpty())
  {
//		myUSBdev->LoadFirmwareFromFile(controllerFileLineEdit->text().toStdString().c_str());
		myUSBdev->LoadFirmwareFromFile(ui->controllerFileLineEdit->text().toLatin1().data());
	}
}

void MainForm::doflashEEPROM()
{
	if (!ui->eepromFileLineEdit->text().isEmpty())
		myUSBdev->LoadHexFileToEeprom(ui->eepromFileLineEdit->text().toLatin1().data());
}


unsigned long MainForm::MyThreadFunc(void* ptr)
{
	MainForm *hdl = (MainForm *)ptr;
	hdl->doflashEEPROM();
  return 1;
}

void MainForm::flashEEPROM()
{
	if (!ui->eepromFileLineEdit->text().isEmpty())
	{
		ui->flashProgressBar->show();
#ifdef __VISUALC__
	  	unsigned long tId;
		HANDLE tFlash = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) MyThreadFunc, this, 0, &tId); 
		while(WaitForSingleObject(tFlash, 0))
#endif
    	 QCoreApplication::processEvents();
		ui->flashProgressBar->hide();
	}
}


void MainForm::updateDeviceId()
{
	myUSBdev->WriteIDToEEPROM(ui->IdLineEdit->text().toInt());
	myUSBdev->WriteNameToEEPROM(ui->nameLineEdit->text().toLatin1().data());
	int myclass = ui->classLineEdit->text().toInt();
	if(myclass>255) myclass=255;
	myUSBdev->WriteDeviceClassToEEPROM(myclass);

	UpdateDevList();
}


void MainForm::listItemClicked(QTreeWidgetItem *currentItem, int )//column)
{
	SetCurrentUSBDevice(devListItems[ui->devListTree->indexOfTopLevelItem(currentItem)]->text(0).toInt());
}

void MainForm::UpdateDevList()
{
 // int numDev = GetNumberOfUSBBoards();
	QStringList devInfoString;
	std::string devId, devName, devClass, devFirmware;
	std::stringstream tmp;
	QTreeWidgetItem *tmpItem;
	SiUSBDevice tmpDev(NULL);

	devListItems.clear();
	ui->devListTree->clear();

	for (int i = 0; i < GetMaxNumberOfUSBBoards(); i++)
	{
		if (GetUSBDeviceIndexed(i) != NULL)
		{
			tmpDev.SetDeviceHandle(GetUSBDeviceIndexed(i));
			devInfoString.clear();
			devInfoString << QString::number((int)tmpDev.GetId()) << QString(tmpDev.GetName()) << QString::number(tmpDev.GetClass()) << QString::number(tmpDev.GetFWVersion());
			tmpItem = new QTreeWidgetItem(ui->devListTree, devInfoString);
			tmpItem->setIcon(0, *silab_usb_ico);
			devListItems.append(tmpItem);
  	}
	}
  ui->devListTree->insertTopLevelItems(0, devListItems);
}

void MainForm::writeClicked()
{
	std::stringstream ins;
	std::stringstream outs;
	int sBuffer[16];
	unsigned char buffer[16];
	int count = 0;

	ins << ui->writeDataLine->text().toStdString();

	for (int i = 0; (i < 16) && !ins.eof(); i++)
	{
		if (ui->hexCheckBox->isChecked())
			ins >> std::hex >> sBuffer[i] ;
		else
			ins >>  sBuffer[i] ;
		buffer[i] = (unsigned char)(0xff & sBuffer[i]);
		count ++;
	}

	ui->sizeLine->setText(QString::number(count));

	/*  debug
	for (int i = 0; i < count; i++)
	{
  	outs << (int)buffer[i] << " ";
	}
	readDataLine->setText(QString(outs.str().c_str()));  
  */

	if (ui->modeComboBox->currentText() == QString("External"))
		myUSBdev->WriteExternal(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("FPGA"))
		myUSBdev->WriteXilinx(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("Command"))
		myUSBdev->WriteCommand(buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("I2C"))
		myUSBdev->WriteI2C(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("SPI"))
		myUSBdev->WriteSPI(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("UART"))
		myUSBdev->WriteSerial(buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("Controller"))
		myUSBdev->Write8051(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("Interrupt"))
		myUSBdev->WriteRegister(&buffer[0]);
	else
	if (ui->modeComboBox->currentText() == QString("Block"))
		myUSBdev->WriteBlock(buffer, count);
}

void MainForm::readClicked()
{
  unsigned char buffer[16];
	std::stringstream outs;
#ifdef CF__LINUX 
	int count = std::min(16, ui->sizeLine->text().toInt());
#else
	int count = min(16, ui->sizeLine->text().toInt());
#endif

	if (ui->modeComboBox->currentText() == QString("External"))
		myUSBdev->ReadExternal(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("FPGA"))
		myUSBdev->ReadXilinx(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("I2C"))
		myUSBdev->ReadI2C(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("SPI"))
		myUSBdev->ReadSPI(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("UART"))
		myUSBdev->ReadSerial(buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("Controller"))
		myUSBdev->Read8051(ui->addLine->text().toInt(), buffer, count);
	else
	if (ui->modeComboBox->currentText() == QString("Interrupt"))
		myUSBdev->ReadRegister(buffer);
	else
	if (ui->modeComboBox->currentText() == QString("Block"))
		myUSBdev->ReadBlock(buffer, count);

	for (int i = 0; i < count; i++)
	{
		if (ui->hexCheckBox->isChecked())
			outs << std::hex << (int)buffer[i] << " ";
		else
  	  outs << (int)buffer[i] << " ";
	}

	ui->readDataLine->setText(QString(outs.str().c_str()));  

}

void MainForm::runBenchmark()
{
	QTime myTimer;
	int chunkSize = 0x1000; // 4k
	int bufferSize = 0x10000; // 64k
	int numBlocks = bufferSize/chunkSize;
	int blockSize;
	int cycles = 20;
	unsigned char *buffer = new unsigned char[bufferSize];
  double *x  = new double[numBlocks];
  double *yw = new double[numBlocks];
  double *yr = new double[numBlocks];
	int ptr = 0;
	int stopTime;

	myTimer.start();


	for (int i=1; i <= numBlocks; i++)
	{ 
		blockSize = i * chunkSize;
		x[ptr] = (double) blockSize / 1024.0; // block size in kbytes
		cycles = (numBlocks + 2 - i) * 4;
    
		// write loop
	  myTimer.restart();
		for(int j=0; j < cycles; j++)
			myUSBdev->WriteBlock(buffer, blockSize);
		stopTime = myTimer.elapsed();
		yw[ptr] = x[ptr] / ((double) stopTime / cycles); // kbyte / ms
#if QWT_VERSION < 0x060000
	  curve1->setData(x, yw, i);     
#else
    curve1->setSamples(x, yw, i);

#endif
	
		// read loop
	  myTimer.restart();
		for(int j=0; j < cycles; j++)
			myUSBdev->ReadBlock(buffer, blockSize);
		stopTime = myTimer.elapsed();
		yr[ptr] = x[ptr] / ((double) stopTime / cycles); // kbyte / ms
#if QWT_VERSION < 0x060000
		curve2->setData(x, yr, i); 
#else
    curve2->setSamples(x, yr, i);

#endif
//   ui->benchPlot->replot();
		ptr++;
	}

	myTimer.restart();
	for (int i = 0; i < 10000; i++)
    myUSBdev->ReadRegister(buffer);
	stopTime = myTimer.elapsed();
	ui->rateLabel->setText(QString("%1 kHz").arg(10000.0/(double)stopTime));

	delete[] buffer;
	delete[] x;
	delete[] yw;
	delete[] yr;
}

	
void MainForm::selectMode(QString )//mode)
{
  ;
}

 
