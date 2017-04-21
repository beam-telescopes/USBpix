#include "gpac_example.h"


GPACMainForm::GPACMainForm(QWidget *parent, Qt::WindowFlags flags): QMainWindow(parent, flags),ui(new Ui::GPACMainForm)
{
  ui->setupUi(this);
  connect(ui->PWR0CheckBox, SIGNAL(clicked(bool)), this, SLOT(enablePWR0(bool)));
  connect(ui->PWR1CheckBox, SIGNAL(clicked(bool)), this, SLOT(enablePWR1(bool)));
  connect(ui->PWR2CheckBox, SIGNAL(clicked(bool)), this, SLOT(enablePWR2(bool)));
  connect(ui->PWR3CheckBox, SIGNAL(clicked(bool)), this, SLOT(enablePWR3(bool)));
  connect(ui->PWR0spin, SIGNAL(valueChanged(double)), this, SLOT(setPWR0(double)));
  connect(ui->PWR1spin, SIGNAL(valueChanged(double)), this, SLOT(setPWR1(double)));
  connect(ui->PWR2spin, SIGNAL(valueChanged(double)), this, SLOT(setPWR2(double)));
  connect(ui->PWR3spin, SIGNAL(valueChanged(double)), this, SLOT(setPWR3(double)));
  connect(ui->VSRC0spin, SIGNAL(valueChanged(double)), this, SLOT(setVSRC0(double)));
  connect(ui->VSRC1spin, SIGNAL(valueChanged(double)), this, SLOT(setVSRC1(double)));
  connect(ui->VSRC2spin, SIGNAL(valueChanged(double)), this, SLOT(setVSRC2(double)));
  connect(ui->VSRC3spin, SIGNAL(valueChanged(double)), this, SLOT(setVSRC3(double)));
  connect(ui->ISRC0spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC0(double)));
  connect(ui->ISRC1spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC1(double)));
  connect(ui->ISRC2spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC2(double)));
  connect(ui->ISRC3spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC3(double)));
  connect(ui->ISRC4spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC4(double)));
  connect(ui->ISRC5spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC5(double)));
  connect(ui->ISRC6spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC6(double)));
  connect(ui->ISRC7spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC7(double)));
  connect(ui->ISRC8spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC8(double)));
  connect(ui->ISRC9spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC9(double)));
  connect(ui->ISRC10spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC10(double)));
  connect(ui->ISRC11spin, SIGNAL(valueChanged(double)), this, SLOT(setISRC11(double)));
  connect(ui->CurrLimBox, SIGNAL(valueChanged(double)), this, SLOT(setCurrLim()));

  InitUSB();             // init USB device manager
	myUSBdev = new SiUSBDevice(NULL); // USB device handle (take the one USBpix is using)
	myTLUSB  = new TL_USB(myUSBdev);  // USB transfer layer object based on USB device (for GPAC only)
	myTLUSB->Open(-1);                // get next available USB device instance
	myAnalogCard = new HL_GPAC(*myTLUSB);

	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(updateMeasurements()));

                          //         , name , min,  max,  def, I lim
 	PWR[0] = myAnalogCard->PWR[0]->Init("DVDD",  1000, 1800, 1500, 100);
 	PWR[1] = myAnalogCard->PWR[1]->Init("AVDD",  1000, 1800, 1500, 100);
 	PWR[2] = myAnalogCard->PWR[2]->Init("VDD",   1000, 1800, 1500, 100);
 	PWR[3] = myAnalogCard->PWR[3]->Init("VDDIO", 1000, 1800, 1500, 100);

 	 
                           //            name,  min,  max, def, 
 	ISRC[0]  = myAnalogCard->ISRC[0] ->Init("Iaa", -500,  500,  30);
 	ISRC[1]  = myAnalogCard->ISRC[1] ->Init("Ibb", -500,  500,  31);
 	ISRC[2]  = myAnalogCard->ISRC[2] ->Init("Icc", -500,  500,  32);
 	ISRC[3]  = myAnalogCard->ISRC[3] ->Init("Idd", -500,  500,  33);
 	ISRC[4]  = myAnalogCard->ISRC[4] ->Init("Iee", -500,  500,  34);
 	ISRC[5]  = myAnalogCard->ISRC[5] ->Init("Iff", -500,  500,  35);
 	ISRC[6]  = myAnalogCard->ISRC[6] ->Init("Igg", -500,  500, -30);
 	ISRC[7]  = myAnalogCard->ISRC[7] ->Init("Ihh", -500,  500, -31);
 	ISRC[8]  = myAnalogCard->ISRC[8] ->Init("Iii", -500,  500, -32);
 	ISRC[9]  = myAnalogCard->ISRC[9] ->Init("Ijj", -500,  500, -33);
 	ISRC[10] = myAnalogCard->ISRC[10]->Init("Ikk", -500,  500, -34);
 	ISRC[11] = myAnalogCard->ISRC[11]->Init("Ill", -500,  500, -35);

                           //            name,  min,  max, def, 
 	VSRC[0]  = myAnalogCard->VSRC[0] ->Init("Vaa", 0,  500,  300);
 	VSRC[1]  = myAnalogCard->VSRC[1] ->Init("Vbb", 0,  500,  300);
 	VSRC[2]  = myAnalogCard->VSRC[2] ->Init("Vcc", 0,  500,  300);
 	VSRC[3]  = myAnalogCard->VSRC[3] ->Init("Vdd", 0,  500,  300);

 	VINJ[0]  = myAnalogCard->VINJ[0] ->Init("InjLo", 200, 2000,  200);
 	VINJ[1]  = myAnalogCard->VINJ[1] ->Init("InjHi",   0, 2000, 1300);


	ui->PWR0spin-> setValue( PWR[0]->CalData.DefaultValue);
	ui->PWR1spin-> setValue( PWR[1]->CalData.DefaultValue);
	ui->PWR2spin-> setValue( PWR[2]->CalData.DefaultValue);
	ui->PWR3spin-> setValue( PWR[3]->CalData.DefaultValue);
	ui->VSRC0spin->setValue(VSRC[0]->CalData.DefaultValue);
	ui->VSRC1spin->setValue(VSRC[1]->CalData.DefaultValue);
	ui->VSRC2spin->setValue(VSRC[2]->CalData.DefaultValue);
	ui->VSRC3spin->setValue(VSRC[3]->CalData.DefaultValue);
	ui->ISRC0spin->setValue(ISRC[0]->CalData.DefaultValue);
	ui->ISRC1spin->setValue(ISRC[1]->CalData.DefaultValue);
	ui->ISRC2spin->setValue(ISRC[2]->CalData.DefaultValue);
	ui->ISRC3spin->setValue(ISRC[3]->CalData.DefaultValue);
	ui->ISRC4spin->setValue(ISRC[4]->CalData.DefaultValue);
	ui->ISRC5spin->setValue(ISRC[5]->CalData.DefaultValue);
	ui->ISRC6spin->setValue(ISRC[6]->CalData.DefaultValue);
	ui->ISRC7spin->setValue(ISRC[7]->CalData.DefaultValue);
	ui->ISRC8spin->setValue(ISRC[8]->CalData.DefaultValue);
	ui->ISRC9spin->setValue(ISRC[9]->CalData.DefaultValue);
	ui->ISRC10spin->setValue(ISRC[10]->CalData.DefaultValue);
	ui->ISRC11spin->setValue(ISRC[11]->CalData.DefaultValue);

	ui->PWRname_0-> setText(QString( PWR[0]->GetName()));
	ui->PWRname_1-> setText(QString( PWR[1]->GetName()));
	ui->PWRname_2-> setText(QString( PWR[2]->GetName()));
	ui->PWRname_3-> setText(QString( PWR[3]->GetName()));
	ui->VSRCname_0->setText(QString(VSRC[0]->GetName()));
	ui->VSRCname_1->setText(QString(VSRC[1]->GetName()));
	ui->VSRCname_2->setText(QString(VSRC[2]->GetName()));
	ui->VSRCname_3->setText(QString(VSRC[3]->GetName()));
	ui->ISRCname_0->setText(QString(ISRC[0]->GetName()));
	ui->ISRCname_1->setText(QString(ISRC[1]->GetName()));
	ui->ISRCname_2->setText(QString(ISRC[2]->GetName()));
	ui->ISRCname_3->setText(QString(ISRC[3]->GetName()));
	ui->ISRCname_4->setText(QString(ISRC[4]->GetName()));
	ui->ISRCname_5->setText(QString(ISRC[5]->GetName()));
	ui->ISRCname_6->setText(QString(ISRC[6]->GetName()));
	ui->ISRCname_7->setText(QString(ISRC[7]->GetName()));
	ui->ISRCname_8->setText(QString(ISRC[8]->GetName()));
	ui->ISRCname_9->setText(QString(ISRC[9]->GetName()));
	ui->ISRCname_10->setText(QString(ISRC[10]->GetName()));
	ui->ISRCname_11->setText(QString(ISRC[11]->GetName()));

	UpdateSystem(); // update system information
}

GPACMainForm::~GPACMainForm()
{

}

void GPACMainForm::onDeviceChange()
{
	if (OnDeviceChange()) // call to SiLibUSB
		UpdateSystem();
}

void GPACMainForm::UpdateSystem()
{
	std::stringstream ssbuf;

	if (myTLUSB->Open(-1))
	{
		myAnalogCard->Init(*myTLUSB);  
		ssbuf << myTLUSB->GetName() << " ID: " << (int)myTLUSB->GetId() << ", Analog card ID: " << (int)myAnalogCard->GetId();
		ui->statusLabel->setText(ssbuf.str().c_str());
	}
	else
	{
		ui->statusLabel->setText("no board found");
	}
}

void  GPACMainForm::enablePWR0(bool on_off){	PWR[0]->Switch(on_off);}
void  GPACMainForm::enablePWR1(bool on_off){	PWR[1]->Switch(on_off);}
void  GPACMainForm::enablePWR2(bool on_off){	PWR[2]->Switch(on_off);}
void  GPACMainForm::enablePWR3(bool on_off){	PWR[3]->Switch(on_off);}

void  GPACMainForm::setPWR0(double val){	PWR[0]->SetVoltage(val);}
void  GPACMainForm::setPWR1(double val){	PWR[1]->SetVoltage(val);}
void  GPACMainForm::setPWR2(double val){	PWR[2]->SetVoltage(val);}
void  GPACMainForm::setPWR3(double val){	PWR[3]->SetVoltage(val);}

void  GPACMainForm::setVSRC0(double val){	VSRC[0]->SetVoltage(val);}
void  GPACMainForm::setVSRC1(double val){	VSRC[1]->SetVoltage(val);}
void  GPACMainForm::setVSRC2(double val){	VSRC[2]->SetVoltage(val);}
void  GPACMainForm::setVSRC3(double val){	VSRC[3]->SetVoltage(val);}

void  GPACMainForm::setISRC0(double val){	ISRC[0]->SetCurrent(val);}
void  GPACMainForm::setISRC1(double val){	ISRC[1]->SetCurrent(val);}
void  GPACMainForm::setISRC2(double val){	ISRC[2]->SetCurrent(val);}
void  GPACMainForm::setISRC3(double val){	ISRC[3]->SetCurrent(val);}
void  GPACMainForm::setISRC4(double val){	ISRC[4]->SetCurrent(val);}
void  GPACMainForm::setISRC5(double val){	ISRC[5]->SetCurrent(val);}
void  GPACMainForm::setISRC6(double val){	ISRC[6]->SetCurrent(val);}
void  GPACMainForm::setISRC7(double val){	ISRC[7]->SetCurrent(val);}
void  GPACMainForm::setISRC8(double val){	ISRC[8]->SetCurrent(val);}
void  GPACMainForm::setISRC9(double val){	ISRC[9]->SetCurrent(val);}
void  GPACMainForm::setISRC10(double val){	ISRC[10]->SetCurrent(val);}
void  GPACMainForm::setISRC11(double val){	ISRC[11]->SetCurrent(val);}


void GPACMainForm::setCurrLim()
{
  PWR[0]->SetCurrentLimit(ui->CurrLimBox->value());
}

void GPACMainForm::enableTimer(bool yes)
{
	if (yes)
		timer->start(100);
	else
		timer->stop();
}

void GPACMainForm::updateMeasurements()
{
	PWR[0]->UpdateMeasurements();
	PWR[1]->UpdateMeasurements();
	PWR[2]->UpdateMeasurements();
	PWR[3]->UpdateMeasurements();

	ui->PWR0VLCD->display(QString::number(PWR[0]->GetVoltage(),'f',0));
	ui->PWR1VLCD->display(QString::number(PWR[1]->GetVoltage(),'f',0));
	ui->PWR2VLCD->display(QString::number(PWR[2]->GetVoltage(),'f',0));
	ui->PWR3VLCD->display(QString::number(PWR[3]->GetVoltage(),'f',0));

	ui->PWR0CLCD->display(QString::number(PWR[0]->GetCurrent(),'f',0));
	ui->PWR1CLCD->display(QString::number(PWR[1]->GetCurrent(),'f',0));
	ui->PWR2CLCD->display(QString::number(PWR[2]->GetCurrent(),'f',0));
	ui->PWR3CLCD->display(QString::number(PWR[3]->GetCurrent(),'f',0));

	VSRC[0]->UpdateMeasurements();
	VSRC[1]->UpdateMeasurements();
	VSRC[2]->UpdateMeasurements();
	VSRC[3]->UpdateMeasurements();

	ui->VSRC0VLCD->display(QString::number(VSRC[0]->GetVoltage(),'f',0));
	ui->VSRC1VLCD->display(QString::number(VSRC[1]->GetVoltage(),'f',0));
	ui->VSRC2VLCD->display(QString::number(VSRC[2]->GetVoltage(),'f',0));
	ui->VSRC3VLCD->display(QString::number(VSRC[3]->GetVoltage(),'f',0));

	ui->VSRC0CLCD->display(QString::number(VSRC[0]->GetCurrent(),'f',0));
	ui->VSRC1CLCD->display(QString::number(VSRC[1]->GetCurrent(),'f',0));
	ui->VSRC2CLCD->display(QString::number(VSRC[2]->GetCurrent(),'f',0));
	ui->VSRC3CLCD->display(QString::number(VSRC[3]->GetCurrent(),'f',0));

  for (int i = 0; i < MAX_ISRC; i ++)
		ISRC[i]->UpdateMeasurements();

	ui->ISRC0VLCD->display(QString::number(ISRC[0]->GetVoltage(),'f',0));
	ui->ISRC1VLCD->display(QString::number(ISRC[1]->GetVoltage(),'f',0));
	ui->ISRC2VLCD->display(QString::number(ISRC[2]->GetVoltage(),'f',0));
	ui->ISRC3VLCD->display(QString::number(ISRC[3]->GetVoltage(),'f',0));
	ui->ISRC4VLCD->display(QString::number(ISRC[4]->GetVoltage(),'f',0));
	ui->ISRC5VLCD->display(QString::number(ISRC[5]->GetVoltage(),'f',0));
	ui->ISRC6VLCD->display(QString::number(ISRC[6]->GetVoltage(),'f',0));
	ui->ISRC7VLCD->display(QString::number(ISRC[7]->GetVoltage(),'f',0));
	ui->ISRC8VLCD->display(QString::number(ISRC[8]->GetVoltage(),'f',0));
	ui->ISRC9VLCD->display(QString::number(ISRC[9]->GetVoltage(),'f',0));
	ui->ISRC10VLCD->display(QString::number(ISRC[10]->GetVoltage(),'f',0));
	ui->ISRC11VLCD->display(QString::number(ISRC[11]->GetVoltage(),'f',0));

	ui->ISRC0CLCD->display(QString::number(ISRC[0]->GetCurrent(),'f',0));
	ui->ISRC1CLCD->display(QString::number(ISRC[1]->GetCurrent(),'f',0));
	ui->ISRC2CLCD->display(QString::number(ISRC[2]->GetCurrent(),'f',0));
	ui->ISRC3CLCD->display(QString::number(ISRC[3]->GetCurrent(),'f',0));
	ui->ISRC4CLCD->display(QString::number(ISRC[4]->GetCurrent(),'f',0));
	ui->ISRC5CLCD->display(QString::number(ISRC[5]->GetCurrent(),'f',0));
	ui->ISRC6CLCD->display(QString::number(ISRC[6]->GetCurrent(),'f',0));
	ui->ISRC7CLCD->display(QString::number(ISRC[7]->GetCurrent(),'f',0));
	ui->ISRC8CLCD->display(QString::number(ISRC[8]->GetCurrent(),'f',0));
	ui->ISRC9CLCD->display(QString::number(ISRC[9]->GetCurrent(),'f',0));
	ui->ISRC10CLCD->display(QString::number(ISRC[10]->GetCurrent(),'f',0));
	ui->ISRC11CLCD->display(QString::number(ISRC[11]->GetCurrent(),'f',0));
}

