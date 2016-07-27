#ifndef CALIBRATIONWIZARDI3_H
#define CALIBRATIONWIZARDI3_H


#include <QtGui>
#include <QWizard>


class USBPixDCSI3;

 class CalibrationWizardI3 : public QWizard
 {
     Q_OBJECT

 public:
	 CalibrationWizardI3(USBPixDCSI3 *DCS, QWidget *parent = 0);
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

	static void addCalItem(QStandardItemModel *model, const double &currVal,
			       const double &newVal, double  &defVal);
	static void addCalItem(QStandardItemModel *model, const int &currVal,
			       const int &newVal, int  &defVal);


 private:
	 USBPixDCSI3 *myDCS;
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
