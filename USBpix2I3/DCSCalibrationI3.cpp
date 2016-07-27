
#include <QtGui>


#include "DCSCalibrationI3.h"
#include "USBPixDCSI3.h"

CalibrationWizardI3::CalibrationWizardI3(USBPixDCSI3 *DCS, QWidget *parent)
     : QWizard(parent)
 {
	 myDCS = DCS;
   model = new QStandardItemModel(0, 3);
	 table = new QTableView();
   table->setModel(model);

	 ponLineEdit = new QLineEdit;
	 ponALineEdit = new QLineEdit;
	 ponDLineEdit = new QLineEdit;
	 R12LineEdit = new QLineEdit;
	 R13LineEdit = new QLineEdit;
	 R14LineEdit = new QLineEdit;
	 VCCALineEdit = new QLineEdit;
	 R12LineEdit = new QLineEdit;
	 R13LineEdit = new QLineEdit;
	 R14LineEdit = new QLineEdit;
	 VCCALineEdit = new QLineEdit;


	 setOptions(WizardOptions(QWizard::CancelButtonOnLeft | QWizard::ModernStyle |  QWizard::NoBackButtonOnStartPage));
	 setOption(WizardOption(QWizard::IndependentPages), false);
	 addPage(createIntroPage());
   addPage(createVoltageCalPage());
   addPage(createCurrentCalPage());
	 addPage(createNTCCalPage());
	 addPage(createFinalPage());

 }

void CalibrationWizardI3::updateConstants()
{
  UpdateCalibrationConstantsFromTable();
	myDCS->currentVal = myDCS->newVal;
	myDCS->UpdateEEPROM(&myDCS->newVal);
	myDCS->WriteCalDataFile(myDCS->newVal);
}

void CalibrationWizardI3::Init()
{
	myDCS->newVal = myDCS->currentVal;
	UpdateTable();
	restart();
}

void CalibrationWizardI3::readFile()
{
	QString FileName = QFileDialog::getOpenFileName(this,
		tr("Open ADC calibration file"), "../config", tr("Ini Files (*.ini)"));

	if (!FileName.isEmpty())
	{
		myDCS->ReadCalDataFile(myDCS->newVal, FileName.toStdString().c_str());
		UpdateTable();
	}
}

void CalibrationWizardI3::writeFile()
{
	{
	QString FileName = QFileDialog::getSaveFileName(this,
		tr("Save ADC calibration file"), "../config", tr("Ini Files (*.ini)"));

	if (!FileName.isEmpty())
	{
		UpdateCalibrationConstantsFromTable();
 	  myDCS->WriteCalDataFile(myDCS->newVal, FileName.toStdString().c_str());
	}
  }
}


void CalibrationWizardI3::sampleZeroReadings()
{
	zeroVoltageRaw     = myDCS->GetAVDDVoltage(true);
	zeroAVDDCurrentRaw = myDCS->GetAVDDCurrent(true);
	zeroDVDDCurrentRaw = myDCS->GetDVDDCurrent(true);
}


void CalibrationWizardI3::CalibrateVoltage()
{
	myDCS->CalculateGainAndOffset(0, zeroVoltageRaw, 
		                            ponLineEdit->text().toDouble(), myDCS->GetAVDDVoltage(true), 
																myDCS->newVal.ADCgain, myDCS->newVal.ADCoffset);

	model->setData(model->index(0, 0), myDCS->newVal.ADCoffset);
	model->setData(model->index(1, 0), myDCS->newVal.ADCgain);
}

void CalibrationWizardI3::CalibrateAVDDCurrent()
{
	myDCS->CalculateGainAndOffset(0, zeroAVDDCurrentRaw, 
	                            ponALineEdit->text().toDouble(), myDCS->GetAVDDCurrent(true), 
															myDCS->newVal.AVDDIgain, myDCS->newVal.AVDDIoffset);

	model->setData(model->index(2, 0), myDCS->newVal.AVDDIoffset);
	model->setData(model->index(3, 0), myDCS->newVal.AVDDIgain);
}

void CalibrationWizardI3::CalibrateDVDDCurrent()
{
	myDCS->CalculateGainAndOffset(0, zeroDVDDCurrentRaw, 
	                            ponDLineEdit->text().toDouble(), myDCS->GetDVDDCurrent(true), 
															myDCS->newVal.DVDDIgain, myDCS->newVal.DVDDIoffset);	

	model->setData(model->index(4, 0), myDCS->newVal.DVDDIoffset);
	model->setData(model->index(5, 0), myDCS->newVal.DVDDIgain);
}


void CalibrationWizardI3::UpdateCalibrationConstantsFromTable()
{
	myDCS->newVal.ADCoffset   = model->data(model->index(0, 0)).toDouble();
	myDCS->newVal.ADCgain     = model->data(model->index(1, 0)).toDouble();
	myDCS->newVal.AVDDIoffset = model->data(model->index(2, 0)).toDouble();
	myDCS->newVal.AVDDIgain   = model->data(model->index(3, 0)).toDouble();
	myDCS->newVal.DVDDIoffset = model->data(model->index(4, 0)).toDouble();
	myDCS->newVal.DVDDIgain   = model->data(model->index(5, 0)).toDouble();
 	myDCS->newVal.R_NTC_25    = model->data(model->index(6, 0)).toDouble();
 	myDCS->newVal.B_NTC       = model->data(model->index(7, 0)).toDouble();
 	myDCS->newVal.R12         = model->data(model->index(8, 0)).toDouble();
 	myDCS->newVal.R13         = model->data(model->index(9, 0)).toDouble();
 	myDCS->newVal.R14         = model->data(model->index(10, 0)).toDouble();
 	myDCS->newVal.VCCA        = model->data(model->index(11, 0)).toDouble();
	myDCS->newVal.nMeasDummy  = model->data(model->index(12, 0)).toInt();
	myDCS->newVal.nMeasMean   = model->data(model->index(13, 0)).toInt();
}

void CalibrationWizardI3::UpdateTable()
{
	model->setData(model->index(0, 1), QString::number(myDCS->currentVal.ADCoffset, 'f', 6));
	model->setData(model->index(1, 1), QString::number(myDCS->currentVal.ADCgain, 'f', 6));
	model->setData(model->index(2, 1), QString::number(myDCS->currentVal.AVDDIoffset, 'f', 6));
	model->setData(model->index(3, 1), QString::number(myDCS->currentVal.AVDDIgain, 'f', 6));
	model->setData(model->index(4, 1), QString::number(myDCS->currentVal.DVDDIoffset, 'f', 6));
	model->setData(model->index(5, 1), QString::number(myDCS->currentVal.DVDDIgain, 'f', 6));
	model->setData(model->index(6, 1), QString::number(myDCS->currentVal.R_NTC_25, 'f', 6));
	model->setData(model->index(7, 1), QString::number(myDCS->currentVal.B_NTC, 'f', 6));
	model->setData(model->index(8, 1), QString::number(myDCS->currentVal.R12, 'f', 6));
	model->setData(model->index(9, 1), QString::number(myDCS->currentVal.R13, 'f', 6));
	model->setData(model->index(10, 1), QString::number(myDCS->currentVal.R14, 'f', 6));
	model->setData(model->index(11, 1), QString::number(myDCS->currentVal.VCCA, 'f', 6));
	model->setData(model->index(12, 1), QString::number(myDCS->currentVal.nMeasDummy, 'f', 6));
	model->setData(model->index(13, 1), QString::number(myDCS->currentVal.nMeasMean, 'f', 6));	
	
	model->setData(model->index(0, 0), QString::number(myDCS->newVal.ADCoffset, 'f', 6));
	model->setData(model->index(1, 0), QString::number(myDCS->newVal.ADCgain, 'f', 6));
	model->setData(model->index(2, 0), QString::number(myDCS->newVal.AVDDIoffset, 'f', 6));
	model->setData(model->index(3, 0), QString::number(myDCS->newVal.AVDDIgain, 'f', 6));
	model->setData(model->index(4, 0), QString::number(myDCS->newVal.DVDDIoffset, 'f', 6));
	model->setData(model->index(5, 0), QString::number(myDCS->newVal.DVDDIgain, 'f', 6));
	model->setData(model->index(6, 0), QString::number(myDCS->newVal.R_NTC_25, 'f', 6));
	model->setData(model->index(7, 0), QString::number(myDCS->newVal.B_NTC, 'f', 6));
	model->setData(model->index(8, 0), QString::number(myDCS->newVal.R12, 'f', 6));
	model->setData(model->index(9, 0), QString::number(myDCS->newVal.R13, 'f', 6));
	model->setData(model->index(10, 0), QString::number(myDCS->newVal.R14, 'f', 6));
	model->setData(model->index(11, 0), QString::number(myDCS->newVal.VCCA, 'f', 6));
	model->setData(model->index(12, 0), QString::number(myDCS->newVal.nMeasDummy));
	model->setData(model->index(13, 0), QString::number(myDCS->newVal.nMeasMean));
}



QWizardPage * CalibrationWizardI3::createIntroPage()
 {
     QWizardPage *page = new QWizardPage;
     page->setTitle("Introduction to the DCS calibration procedure");

     QLabel *label = new QLabel("This wizard will guide you through the steps of the DCS calibration procedure "
                                "for voltage, current and temperature measurements. "
																"During the calibration procedure you will need a external multimeter " 
																"for precise voltage and current measurements."
																"\n\n"
																"ATTENTION: \n"
																"1. Connect the single chip card and configure the chip.\n"
																"2. If you skip any of the calibration procedure steps "
																" default values (or last calibration) will be used. \n"
																"3. You can change the FE configuration settings during the "
																"calibration procedure via the main panel. For example restore the "
																"global register settings after the supply has been toggled during the ADC "
																"calibration.\n"
																"4. To calibrate the temperature sensing the values of R12, R13 and R14 "
																"and the voltage across C9 on the adapter card have to be measured. "
																"The resistors have to be measured with the adapter card disconnected from the FPGA board "
																"and the single chip card."
																);
     label->setWordWrap(true);

     QVBoxLayout *layout = new QVBoxLayout;
     layout->addWidget(label);
     page->setLayout(layout);

		 // initialize new data values
		 myDCS->newVal = myDCS->currentVal;

     return page;
 }

 QWizardPage * CalibrationWizardI3::createVoltageCalPage()
 {
     QWizardPage *page = new QWizardPage;
		 page->setTitle("Step 1: ADC voltage calibration");
     page->setSubTitle("Calibrate ADC gain and offset with external voltmeter");

		 QLabel *a_l = new QLabel("a)");
     QLabel *a_label = new QLabel("Switch off the AVDD and DVDD supplies and push the 'power off reading' button. "
			                            "This step is mandatory also for the current measurement calibration on the next page.");
     a_label->setWordWrap(true);

		 QLabel *b_l = new QLabel("b)");
		 QLabel *b_label = new QLabel("Switch AVDD and DVDD back on and measure the voltage across C5 (AVDD) on the adapter card with an external voltmeter. "
			                            "Enter the measured value and push the 'power on reading' button."
																);
     b_label->setWordWrap(true);

		 QLabel *poffLabel = new QLabel("zero AVDD voltage measurement [volt]:");
		 QLabel *zeroLabel = new QLabel("0 (fixed)");

		 QLabel *ponLabel = new QLabel("external AVDD voltage measured [volt]:");

		 QPushButton *offBtn = new QPushButton("power off reading");
		 QPushButton *onBtn = new QPushButton("power on reading");
		 connect(offBtn, SIGNAL(clicked()), this, SLOT(sampleZeroReadings()));
		 connect(onBtn, SIGNAL(clicked()), this, SLOT(CalibrateVoltage()));

     QGridLayout *layout = new QGridLayout;
     layout->addWidget(a_l, 0, 0, Qt::AlignTop);
     layout->addWidget(a_label, 0, 1, 1, -1);
     layout->addWidget(poffLabel, 1, 1);
     layout->addWidget(zeroLabel, 1, 2);
     layout->addWidget(offBtn, 1, 3);
     layout->addWidget(b_l, 2, 0, Qt::AlignTop);
     layout->addWidget(b_label, 2, 1, 1, -1);
     layout->addWidget(ponLabel, 3, 1);
     layout->addWidget(ponLineEdit, 3, 2);
     layout->addWidget(onBtn, 3, 3);
     page->setLayout(layout);

     return page;
 }

 QWizardPage * CalibrationWizardI3::createCurrentCalPage()
 {
     QWizardPage *page = new QWizardPage;
		 page->setTitle("Step 2: Current measurement calibration");
     page->setSubTitle("Calibrate current sense gain and offset with external amperemeter");

		 QLabel *a_l = new QLabel("a)");
		 QLabel *a_label = new QLabel("Switch off the power and insert an amperemeter in the AVDD line and switch the power back on. "
			                            "Restore the global register configuration "
			                            "(main panel) and measure the currrent consumption with the amperemeter. "
			                            "Enter the measured current and push the 'Update' button."
																);
     a_label->setWordWrap(true);
	
		 QLabel *b_l = new QLabel("b)");
		 QLabel *b_label = new QLabel("Repeat step a) with DVDD instead of AVDD."
																);
     b_label->setWordWrap(true);
		 
		 QLabel *ponALabel = new QLabel("external AVDD current measurement [ampere]:");
		 QLabel *ponDLabel = new QLabel("external DVDD current measurement [ampere]:");

		 QPushButton *onABtn = new QPushButton("Update");
		 QPushButton *onDBtn = new QPushButton("Update");

		 connect(onABtn, SIGNAL(clicked()), this, SLOT(CalibrateAVDDCurrent()));
		 connect(onDBtn, SIGNAL(clicked()), this, SLOT(CalibrateDVDDCurrent()));

     QGridLayout *layout = new QGridLayout;
     layout->addWidget(a_l, 0, 0, Qt::AlignTop);
     layout->addWidget(a_label, 0, 1, 1, -1);
     layout->addWidget(ponALabel, 1, 1);
     layout->addWidget(ponALineEdit, 1, 2);
     layout->addWidget(onABtn, 1, 3);
     layout->addWidget(b_l, 2, 0, Qt::AlignTop);
     layout->addWidget(b_label, 2, 1, 1, -1);
     layout->addWidget(ponDLabel, 3, 1);
     layout->addWidget(ponDLineEdit, 3, 2);
     layout->addWidget(onDBtn, 3, 3);

     page->setLayout(layout);

     return page;
 } 
 
 
 QWizardPage * CalibrationWizardI3::createNTCCalPage()
 {
     QWizardPage *page = new QWizardPage;
		 page->setTitle("Step 3: Temperature measurement calibration");
     page->setSubTitle("Calibrate the resitive voltage divider for the NTC");

		 QLabel *a_l = new QLabel("a)");
     QLabel *a_label = new QLabel("Measure the voltage across C9 on the adapter card (VCCA).");		 
     a_label->setWordWrap(true);
		 
		 QLabel *b_l = new QLabel("b)");
     QLabel *b_label = new QLabel("Switch off the power, remove the adapter card from the FPGA board "
			                          "and disconnect the single chip card. Measure the values of R12, R13 and R14 "
																"on the adapter card.");
     b_label->setWordWrap(true);

		 QLabel *c_l = new QLabel("c)");
     QLabel *c_label = new QLabel("Reconnect the card and switch the power back on. "
																  "Enter the measured values for R12, R13, R14 and VCCA in the table on the next page.");
     c_label->setWordWrap(true);

     QGridLayout *layout = new QGridLayout;
     layout->addWidget(a_l, 0, 0, Qt::AlignTop);
   	 layout->addWidget(a_label, 0, 1);
		 layout->addWidget(b_l, 1, 0, Qt::AlignTop);
     layout->addWidget(b_label, 1, 1);
     layout->addWidget(c_l, 2, 0, Qt::AlignTop);
     layout->addWidget(c_label, 2, 1);
		 layout->setColumnStretch(1, 10);
 
		 page->setLayout(layout);

     return page;
 }

QWizardPage * CalibrationWizardI3::createFinalPage()
{
	QWizardPage *page = new QWizardPage;
	page->setTitle("Finish the DCS calibration procedure");

	QLabel *label = new QLabel("Press:\n\n"
														"'Finish' to update the EEPROM with the new values or\n"
														"'Cancel' to exit without updating the data or\n"
														"'Back' to repeat any of the calibration steps.\n\n"
														"The values in the 'new' column can also be adjusted "
														"manually or saved to or loaded from file."
														);
	label->setWordWrap(true);

	addCalItem(model, myDCS->newVal.ADCoffset, myDCS->currentVal.ADCoffset, myDCS->defaultVal.ADCoffset);
	addCalItem(model, myDCS->newVal.ADCgain, myDCS->currentVal.ADCgain, myDCS->defaultVal.ADCgain);
	addCalItem(model, myDCS->newVal.AVDDIoffset, myDCS->currentVal.AVDDIoffset, myDCS->defaultVal.AVDDIoffset);
	addCalItem(model, myDCS->newVal.AVDDIgain, myDCS->currentVal.AVDDIgain, myDCS->defaultVal.AVDDIgain);
	addCalItem(model, myDCS->newVal.DVDDIoffset, myDCS->currentVal.DVDDIoffset, myDCS->defaultVal.DVDDIoffset);
	addCalItem(model, myDCS->newVal.DVDDIgain, myDCS->currentVal.DVDDIgain, myDCS->defaultVal.DVDDIgain);
	addCalItem(model, myDCS->newVal.R_NTC_25, myDCS->currentVal.R_NTC_25, myDCS->defaultVal.R_NTC_25);
	addCalItem(model, myDCS->newVal.B_NTC, myDCS->currentVal.B_NTC, myDCS->defaultVal.B_NTC);
	addCalItem(model, myDCS->newVal.R12, myDCS->currentVal.R12, myDCS->defaultVal.R12);
	addCalItem(model, myDCS->newVal.R13, myDCS->currentVal.R13, myDCS->defaultVal.R13);
	addCalItem(model, myDCS->newVal.R14, myDCS->currentVal.R14, myDCS->defaultVal.R14);
	addCalItem(model, myDCS->newVal.VCCA, myDCS->currentVal.VCCA, myDCS->defaultVal.VCCA);
	addCalItem(model, myDCS->newVal.nMeasDummy, myDCS->currentVal.nMeasDummy, myDCS->defaultVal.nMeasDummy);
	addCalItem(model, myDCS->newVal.nMeasMean, myDCS->currentVal.nMeasMean, myDCS->defaultVal.nMeasMean);

	QStringList columnLabel;
  columnLabel << "new" << "active" << "default";
	model->setHorizontalHeaderLabels(columnLabel);

	QStringList rowLabel;
  rowLabel << "ADC offset" << "ADC gain" 
		          << "AVDD current offset" << "AVDD current gain"  
		          << "DVDD current offset" << "DVDD current gain" 
							<< "NTC 25° resistance [ohm]" << "NTC b coefficient" 
							<< "R12 [ohm]" << "R13 [ohm]" << "R14 [ohm]" << "VCCA [volt]"
							<< "# dummy measurements" << "# measurement averages";
	model->setVerticalHeaderLabels(rowLabel);
	
	table->resizeRowsToContents();


	QPushButton *writeFileBtn = new QPushButton("Save to file");
	QPushButton *readFileBtn = new QPushButton("Load from file");
	connect(writeFileBtn, SIGNAL(clicked()), this, SLOT(writeFile()));
	connect(readFileBtn, SIGNAL(clicked()), this, SLOT(readFile()));

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(label, 0, 0, 1, -1);
	layout->addWidget(table, 1, 0, 1, -1);
	layout->addWidget(writeFileBtn, 2, 0);
	layout->addWidget(readFileBtn, 2, 1);
	page->setLayout(layout);

  connect(this, SIGNAL(accepted()), this, SLOT(updateConstants()));


	return page;
}
void CalibrationWizardI3::addCalItem(QStandardItemModel *model, const double &currVal,
             const double &newVal, double  &defVal)
{
	  int currentIndex = model->rowCount();
    model->insertRow(currentIndex);
    model->setData(model->index(currentIndex, 0), QString::number(newVal,  'f' ,6));
    model->setData(model->index(currentIndex, 1), QString::number(currVal, 'f' ,6));
    model->setData(model->index(currentIndex, 2), QString::number(defVal,  'f' ,6));
		model->item(currentIndex,1)->setEditable(false);
		model->item(currentIndex,1)->setEnabled(false);
		model->item(currentIndex,2)->setEditable(false);
		model->item(currentIndex,2)->setEnabled(false);
}

void CalibrationWizardI3::addCalItem(QStandardItemModel *model, const int &currVal,
             const int &newVal, int  &defVal)
{
	  int currentIndex = model->rowCount();
    model->insertRow(currentIndex);
    model->setData(model->index(currentIndex, 0), QString::number(newVal));
    model->setData(model->index(currentIndex, 1), QString::number(currVal));
    model->setData(model->index(currentIndex, 2), QString::number(defVal));
		model->item(currentIndex,1)->setEditable(false);
		model->item(currentIndex,1)->setEnabled(false);
		model->item(currentIndex,2)->setEditable(false);
		model->item(currentIndex,2)->setEnabled(false);
}
