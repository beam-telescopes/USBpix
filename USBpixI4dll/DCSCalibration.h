#ifndef CALIBRATIONWIZARD_H
#define CALIBRATIONWIZARD_H


#include <QtGui>
#include <QWizard>
#include "USBPixI4DCS.h"


class USBPixDCS;

 class CalibrationWizard : public QWizard
 {
     Q_OBJECT

 public:
	 CalibrationWizard(USBPixDCS *DCS, QWidget *parent = 0);
public slots:
	void readFile();
	void writeFile();
	void sampleZeroReadings();
	void CalibrateVoltage();
	void CalibrateAVDDCurrent();
	void CalibrateDVDDCurrent();
	void UpdateCalibrationConstantsFromTable();
	void UpdateTable();
	void Init();
	void updateConstants();



 private:
	 USBPixDCS *myDCS;
	 QStandardItemModel *model;
	 QTableView *table;

	 double x1;
	 double y1;
	 double x2;
	 double y2;
	 double zeroVoltageRaw;
	 double zeroAVDDCurrentRaw;
	 double zeroDVDDCurrentRaw;
	 QWizardPage *createIntroPage();
	 QWizardPage *createVoltageCalPage();
	 QWizardPage *createCurrentCalPage();
	 QWizardPage *createNTCCalPage();
	 QWizardPage *createFinalPage();
	 QLineEdit *ponLineEdit;
	 QLineEdit *ponALineEdit;
	 QLineEdit *ponDLineEdit;
	 QLineEdit *R12LineEdit;
	 QLineEdit *R13LineEdit;
	 QLineEdit *R14LineEdit;
	 QLineEdit *VCCALineEdit;

 protected:
 };
#endif