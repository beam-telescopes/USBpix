#include "stdafx.h"
#include "MainForm.h"
#include "defines.h"



MainForm::MainForm(void):QWidget()
{
	double x[100], y[100];
  setupUi(this);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateMeasurements()));
	
  InitUSB();
	myUSBdev = new SiUSBDevice(NULL);
	myGPIBIf = new TGPIB_Interface_USB();
  mySMU    = new TGPIB_Keithley24xx(myGPIBIf, 4);
	myAdapterCard = new USBPixSTDDCS(NULL);

	myPlot = new SimpleQWTPlot(qwtPlot1, 0, 100, -1, 1, QString("Some Title"));
	QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
	for (int i = 0; i < 100; i++)
	{  
		x[i] = i;
//		y[i] = sin(1/10.0*x[i]);
		y[i] = ErrorFunction(50, 10, x[i]);
	}

#if QWT_VERSION < 0x060000
	  curve1->setData(x, y, 100);     
#else
    curve1->setSamples(x, y, 100);
#endif
	
  curve1->attach(myPlot);



	UpdateSystem(); // check for USB devices
}

MainForm::~MainForm(void)
{
	Power(false);
	SelectChannel(-1);
	delete myPlot;
	delete mySMU;
	delete myGPIBIf;
  delete myAdapterCard;
	delete myUSBdev;

}

void MainForm::UpdateSystem()
{
	std::stringstream ssbuf;
	hUSB = GetUSBDevice(-1);

	if (hUSB != NULL)
	{
		myUSBdev->SetDeviceHandle(hUSB);
	  myUSBdev->DownloadXilinx("usbpixi4v07.bit");
		myAdapterCard->SetUSBHandle(myUSBdev);
		ssbuf << myUSBdev->GetName() << " ID: " << (int)myUSBdev->GetId() << ", Adapter card ID: " << (int)myAdapterCard->GetId();
		statusLabel->setText(ssbuf.str().c_str());
		IdNumBox->setValue(myAdapterCard->GetId());
		Power(false);
	  SelectChannel(-1);
	}
	else
	{
		myUSBdev->SetDeviceHandle(NULL);
		myAdapterCard->SetUSBHandle(NULL);
		statusLabel->setText("no board found");
	}
}


void MainForm::onDeviceChange()
{
	if (OnDeviceChange()) // call to USBBoardLib.dll
		UpdateSystem();
}


void MainForm::openFileDialog()
{
	FPGAFileName = QFileDialog::getOpenFileName(this,
		   tr("Select FPGA Configuration File"), "d:/icwiki_svn/USBpixI4/host/trunk/config", tr("Bit Files (*.bit)"));

	if (!FPGAFileName.isEmpty())
       fileLineEdit->setText(FPGAFileName);
  
}

void MainForm::confFPGA()
{
	std::string sb = FPGAFileName.toStdString();
	if (sb != "")
	  myUSBdev->DownloadXilinx((char*)(sb.c_str()));
}

void MainForm::printPlot1()
{
	myPlot->Print();
}

void MainForm::refreshGPIBList()
{
	std::string devList[16];
	myGPIBIf->SearchDevices(devList);
	
	textBrowser->clear();

	for (int i = 0; i < myGPIBIf->GetNumberOfDevices(); i++)
	  textBrowser->append(QString(devList[i].c_str()));
}

void MainForm::sendReceiveGPIB()
{
	std::stringstream ss;
	
	ss << "ID: " << deviceIdEdit->text().toStdString() << " send: "<< commandEdit->text().toStdString();

	ss << " received: " << myGPIBIf->SendAndReceive(deviceIdEdit->text().toInt(), commandEdit->text().toStdString());

	textBrowser->append(QString(ss.str().c_str()));
}
void MainForm::sendGPIB()
{
	std::stringstream ss;
	
	ss << "ID: " << deviceIdEdit->text().toStdString() << " send: "<< commandEdit->text().toStdString();

  myGPIBIf->Send(deviceIdEdit->text().toInt(), commandEdit->text().toStdString());

	textBrowser->append(QString(ss.str().c_str()));
}

void MainForm::clearGPIB()
{
	myGPIBIf->ClearDevice(deviceIdEdit->text().toInt());
}


void MainForm::setVDDA1()
{
	myAdapterCard->PSU[VDDA1]->SetVoltage(VsetA1Box->value());
	Power(EnVdda1CheckBox->isChecked(), VDDA1);
}

void MainForm::setVDDA2()
{
	myAdapterCard->PSU[VDDA2]->SetVoltage(VsetA2Box->value());
	Power(EnVdda2CheckBox->isChecked(), VDDA2);
}

void MainForm::setVDDD1()
{
	myAdapterCard->PSU[VDDD1]->SetVoltage(VsetD1Box->value());
	Power(EnVddd1CheckBox->isChecked(), VDDD1);
}

void MainForm::setVDDD2()
{
	myAdapterCard->PSU[VDDD2]->SetVoltage(VsetD2Box->value());
	Power(EnVddd2CheckBox->isChecked(), VDDD2);
}

void MainForm::updateMeasurements()
{
	double voltageVal, currentVal;

	myAdapterCard->UpdateMeasurements();  
	vdda1VLCD->display(QString::number(myAdapterCard->PSU[VDDA1]->GetVoltage(),'f',3));
	vdda2VLCD->display(QString::number(myAdapterCard->PSU[VDDA2]->GetVoltage(),'f',3));
	vddd1VLCD->display(QString::number(myAdapterCard->PSU[VDDD1]->GetVoltage(),'f',3));
	vddd2VLCD->display(QString::number(myAdapterCard->PSU[VDDD2]->GetVoltage(),'f',3));
	
	vdda1CLCD->display(QString::number(myAdapterCard->PSU[VDDA1]->GetCurrent(),'f',3));
	vdda2CLCD->display(QString::number(myAdapterCard->PSU[VDDA2]->GetCurrent(),'f',3));
	vddd1CLCD->display(QString::number(myAdapterCard->PSU[VDDD1]->GetCurrent(),'f',3));
	vddd2CLCD->display(QString::number(myAdapterCard->PSU[VDDD2]->GetCurrent(),'f',3));

	ntcLCD->display(QString::number(myAdapterCard->GetNTCTemperature(),'f',1));

	mySMU->SwitchOn();
	mySMU->Measure(currentVal, voltageVal);
	
  VmeasLCD->display(QString::number(voltageVal,'f',3));
	CmeasLCD->display(QString::number(currentVal,'f',3));

}

void MainForm::enableTimer(bool yes)
{
	if (yes)
		timer->start(100);
	else
		timer->stop();
}


void MainForm::selLoad()
{
	setLoadCurr();

	switch(loadSelBox->currentIndex())
	{
	  case 0: SelectChannel(-1); mySMU->SwitchOff(); break;
		case 1: SelectChannel(VDDA1); mySMU->SwitchOn(); break;
		case 2: SelectChannel(VDDA2); mySMU->SwitchOn();  break;
		case 3: SelectChannel(VDDD1); mySMU->SwitchOn();  break;
		case 4: SelectChannel(VDDD2); mySMU->SwitchOn();  break;
	}
}

void MainForm::setLoadCurr()
{
 	mySMU->SetSourceType(CURRENTMODE);

	double val = CsetBox->value();

	if ((val > 1) || (val < 0))
		return;

	mySMU->SetCurrent(-val, 2);
}


void MainForm::selectSMU()
{
	memo1->clear();
	switch(chSelBox->currentIndex())
	{
	  case 0: SelectChannel(-1); break;
		case 1: SelectChannel(VDDA1); break;
		case 2: SelectChannel(VDDA2); break;
		case 3: SelectChannel(VDDD1); break;
		case 4: SelectChannel(VDDD2); break;
	}
}


void MainForm::calibrate()
{
	memo1->clear();
	switch(chSelBox->currentIndex())
	{
	  case 0: CalibrateAll(); break;
		case 1: CalibrateChannel(VDDA1); break;
		case 2: CalibrateChannel(VDDA2); break;
		case 3: CalibrateChannel(VDDD1); break;
		case 4: CalibrateChannel(VDDD2); break;
	}
}

void MainForm::setSMUon()
{
	mySMU->SwitchOn();
	getSMUMeas();
}

void MainForm::setSMUoff()
{
	mySMU->SwitchOff();
  voltageLCD->display(QString::number(0,'f',4));
	currentLCD->display(QString::number(0,'f',4));
}

void MainForm::setSMUval()
{
	if (mySMU->DeviceStatus == VOLTAGEMODE)
		mySMU->SetVoltage(voltageVal->value(), 1);	
	
	if (mySMU->DeviceStatus == CURRENTMODE)
		mySMU->SetCurrent(currentVal->value(), 2);

	getSMUMeas();
}

void MainForm::setSMUCout()
{
	offBtn->setChecked(true);
	mySMU->SetSourceType(CURRENTMODE);
}

void MainForm::setSMUVout()
{
	offBtn->setChecked(true);
	mySMU->SetSourceType(VOLTAGEMODE);
}

void MainForm::getSMUMeas()
{
	double currentVal, voltageVal;
	
	mySMU->Measure(currentVal, voltageVal);
	
  voltageLCD->display(QString::number(voltageVal,'f',4));
	currentLCD->display(QString::number(currentVal,'f',4));
}

void MainForm::Power(bool on_off, int channel)
{

	unsigned char data;
  unsigned char mask;

	if (channel == -1)
	{
	  for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
		{
			mask = 0x01 << i;
			myUSBdev->ReadFPGA(CS_POWER_CONTROL, &data);
			if (on_off)
				data |= mask;
			else
				data &= ~mask;
			myUSBdev->WriteFPGA(CS_POWER_CONTROL, &data);
		}
	}
	else
	{
		mask = 0x01 << channel;
		myUSBdev->ReadFPGA(CS_POWER_CONTROL, &data);
		if (on_off)
			data |= mask;
		else
			data &= ~mask;
		myUSBdev->WriteFPGA(CS_POWER_CONTROL, &data);
	}
}

void MainForm::SelectChannel(unsigned char ch)
{
	unsigned char data;

  switch (ch)
	{
	  case VDDA1: data = ~0x04; break;
	  case VDDA2: data = ~0x06; break;
	  case VDDD1: data = ~0x00; break;
	  case VDDD2: data = ~0x02; break;
		default:    data =  0x00; break;
	}

	myUSBdev->WriteFPGA(CS_HARD_RST_CONTROL, &data);
}

void MainForm::CalibrateAll()
{
	int i;

	for (i = 0; i < MAX_SUPPLY_CHANNEL; i++)
	{
		CalibrateChannel(i);
	}
}

void MainForm::CalibrateChannel(int ch)
{
	double ADC_V1, ADC_V2, ADC_C1, ADC_C2, SMU_V1, SMU_V2, dummy;
	int DAC_low = 25;
	int DAC_high = 225;
	double current_low  = 0.1;
	double current_high = 0.6;
	std::stringstream ss;

	ss.str("");
  
  switch (ch)
	{
	  case VDDA1: 	ss << "Ch. " << (int) ch << " (VDDA1)" << std::endl; break;
	  case VDDA2: 	ss << "Ch. " << (int) ch << " (VDDA2)" << std::endl; break;
	  case VDDD1: 	ss << "Ch. " << (int) ch << " (VDDD1)" << std::endl; break;
	  case VDDD2: 	ss << "Ch. " << (int) ch << " (VDDD2)" << std::endl; break;
		default:      ss << "unknown channel: " << (int) ch << " exiting..." << std::endl; break;
	}


	//---- ADC calibration (voltage sense) ---
  //
	// Vmeas = VmeasGain * ADC + VmeasOffset

	mySMU->SwitchOff();     // switch off SMU
	mySMU->SetSourceType(CURRENTMODE); // set to current mode 
	mySMU->SetCurrent(0, 2);
	mySMU->SwitchOn();

	Power(false);
	Power(true, ch);
	SelectChannel(ch);

	myAdapterCard->PSU[ch]->SetVoltage(DAC_high, true);
	Sleep(100);
	mySMU->Measure(dummy, SMU_V1);
	myAdapterCard->UpdateMeasurements();
	ADC_V1 = myAdapterCard->PSU[ch]->GetVoltage(true);
	ADC_C1 = myAdapterCard->PSU[ch]->GetCurrent(RAW);


	myAdapterCard->PSU[ch]->SetVoltage(DAC_low, true);
	Sleep(100);
	mySMU->Measure(dummy, SMU_V2);
	myAdapterCard->UpdateMeasurements();
	ADC_V2 = myAdapterCard->PSU[ch]->GetVoltage(true);
	ADC_C2 = myAdapterCard->PSU[ch]->GetCurrent(RAW);


	myAdapterCard->CalculateGainAndOffset(SMU_V1, ADC_V1, SMU_V2, ADC_V2, myAdapterCard->PSU[ch]->CalData.VmeasGain, myAdapterCard->PSU[ch]->CalData.VmeasOffset);

	ss << "  ADC calibration" << std::endl;
	ss << "    1. {#ADC, Vout} = {" << ADC_V1 << ", " << SMU_V1 << "}" << std::endl;
	ss << "    2. {#ADC, Vout} = {" << ADC_V2 << ", " << SMU_V2 << "}" << std::endl;
	ss << "    VmeasGain   = " << myAdapterCard->PSU[ch]->CalData.VmeasGain << std::endl;
	ss << "    VmeasOffset = " << myAdapterCard->PSU[ch]->CalData.VmeasOffset << std::endl;

	memo1->append(QString(ss.str().c_str()));
	ss.str("");


  //--- DAC calibration (voltage setting) ---

	myAdapterCard->CalculateGainAndOffset(DAC_high, SMU_V1, DAC_low, SMU_V2, myAdapterCard->PSU[ch]->CalData.VsetGain, myAdapterCard->PSU[ch]->CalData.VsetOffset);

	ss << "  DAC calibration" << std::endl;
	ss << "    1. {#DAC, Vout} = {" << DAC_high << ", " << SMU_V1 << "}" << std::endl;
	ss << "    2. {#DAC, Vout} = {" << DAC_low << ", " << SMU_V2 << "}" << std::endl;
	ss << "    VsetGain   = " << myAdapterCard->PSU[ch]->CalData.VsetGain << std::endl;
	ss << "    VsetOffset = " << myAdapterCard->PSU[ch]->CalData.VsetOffset << std::endl;

	memo1->append(QString(ss.str().c_str()));
  ss.str("");


	// voltage dependent quiesent current
  //
	// Iq = IqGain * Vmeas + IqOffset

	myAdapterCard->CalculateGainAndOffset(SMU_V1, ADC_C1, SMU_V2, ADC_C2, myAdapterCard->PSU[ch]->CalData.IqVgain, myAdapterCard->PSU[ch]->CalData.IqOffset);

	ss << "  Iq calibration" << std::endl;
	ss << "    1. {#ADC_I, Vout} = {" << ADC_C1 << ", " << SMU_V1 << "}" << std::endl;
	ss << "    2. {#ADC_I, Vout} = {" << ADC_C2 << ", " << SMU_V2 << "}" << std::endl;
	ss << "    IqGain   = " << myAdapterCard->PSU[ch]->CalData.IqVgain << std::endl;
	ss << "    IqOffset = " << myAdapterCard->PSU[ch]->CalData.IqOffset << std::endl;

	memo1->append(QString(ss.str().c_str()));
  ss.str("");



	// current sense calibration
  //
	// Imeas = ImeasGain * ADC + ImeasOffset


	myAdapterCard->PSU[ch]->SetVoltage(DAC_low, true);
	mySMU->SetCurrent(-current_low, 2);
	Sleep(100);
	myAdapterCard->UpdateMeasurements();
//	ADC_V1 = myAdapterCard->PSU[ch]->GetVoltage(true);
	ADC_C1 = myAdapterCard->PSU[ch]->GetCurrent(RAW_IQ_COMPENSATED);

	mySMU->SetCurrent(-current_high, 2);
	Sleep(100);
	myAdapterCard->UpdateMeasurements();
//	ADC_V1 = myAdapterCard->PSU[ch]->GetVoltage(true);
	ADC_C2 = myAdapterCard->PSU[ch]->GetCurrent(RAW_IQ_COMPENSATED);

	mySMU->SwitchOff();     // switch off SMU


	myAdapterCard->CalculateGainAndOffset(current_low, ADC_C1, current_high, ADC_C2, myAdapterCard->PSU[ch]->CalData.ImeasGain, myAdapterCard->PSU[ch]->CalData.ImeasOffset);

	ss << "  Imeas calibration" << std::endl;
	ss << "    1. {#ADC_I, Iset} = {" << ADC_C1 << ", " << current_low << "}" << std::endl;
	ss << "    2. {#ADC_I, Iset} = {" << ADC_C2 << ", " << current_high << "}" << std::endl;
	ss << "    ImeasGain   = " << myAdapterCard->PSU[ch]->CalData.ImeasGain << std::endl;
	ss << "    ImeasOffset = " << myAdapterCard->PSU[ch]->CalData.ImeasOffset << std::endl;

	memo1->append(QString(ss.str().c_str()));
  ss.str("");


  // safe calibrations constants

	// clean up and exit

	Power(false);
	SelectChannel(-1);
}

void MainForm::updateEEPROM()
{
	myAdapterCard->WriteEEPROM();
}
	
	
void MainForm::dumpEEPROM()
{
	myAdapterCard->ReadEEPROM();
	myAdapterCard->WriteCalDataFile();
	std::stringstream ssbuf;
	ssbuf << myUSBdev->GetName() << " ID: " << (int)myUSBdev->GetId() << ", Adapter card ID: " << (int)myAdapterCard->GetId();
	statusLabel->setText(ssbuf.str().c_str());
}

void MainForm::setId()
{
	myAdapterCard->SetId(IdNumBox->value());
}

void MainForm::change4WireSense(bool enable)
{
	//mySMU->Set4WireSense(enable);
	;
}

void MainForm::autoTestCalibration()
{
	// enable 4 wire sense
	mySMU->SwitchOff();
	mySMU->SetSourceType(CURRENTMODE);

	// switch off everything
	//mySMU->Set4WireSense(true); ???
	Power(false, VDDA1);
	Power(false, VDDA2);
	Power(false, VDDD1);
	Power(false, VDDD2);

	// Test VDDA1
	if (checkChannel(VDDA1))
		vdda1VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	else
		vdda1VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	
	// Test VDDA2
	if (checkChannel(VDDA2))
		vdda2VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	else
		vdda2VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));

	// Test VDDD1
	if (checkChannel(VDDD1))
		vddd1VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	else
		vddd1VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));

	// Test VDDD2
	if (checkChannel(VDDD2))
		vddd2VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	else
		vddd2VLCD->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
}

bool MainForm::checkChannel(int channel)
{
	double currentVal, voltageVal, targetVoltage;

	if (channel==VDDA1 || channel==VDDA2)
		targetVoltage=1.5;
	else
		targetVoltage=1.2;

	SelectChannel(channel);

	myAdapterCard->PSU[channel]->SetVoltage(targetVoltage);
	Power(true, channel);
	QApplication::processEvents();
	Sleep(100);

	mySMU->SwitchOn();
	mySMU->SetCurrent(-0.2, 2);
	Sleep(100);
	mySMU->Measure(currentVal, voltageVal);

	if (channel==VDDA1)
		vdda1VLCD->display(voltageVal);
	else if (channel==VDDA2)
		vdda2VLCD->display(voltageVal);
	else if (channel==VDDD1)
		vddd1VLCD->display(voltageVal);
	else if (channel==VDDD2)
		vddd2VLCD->display(voltageVal);

	mySMU->SwitchOff();
	Power(false, channel);

	if ( fabs((voltageVal-targetVoltage)/targetVoltage) > 0.05 )	// derivation of 5 % allowed
		return false;
	else
		return true;
}