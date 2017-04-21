#pragma once

#include "stdafx.h"
#include <qwidget.h>
#include <qobject.h>
#include <QFileDialog>
#include <QStatusBar>
#include <QTimer>

#include "SimpleQWTPlot.h"
#include "SILIB_GPIB_Interfaces.h"
#include "SILIB_GPIB_Device.h"
#include "SILIB_GPIB_Keithley24xx.h"
#include "SilibDistributions.h"

#include "USBPixI4DCS.h"

#include "ui_mainform.h"


class MainForm :
	 public QWidget, protected Ui_MainForm
{

Q_OBJECT
public:
	MainForm(void);
	virtual ~MainForm(void);

public:
  void onDeviceChange();

public slots:
	void openFileDialog();
	void confFPGA();
	void printPlot1();
	void refreshGPIBList();
	void sendReceiveGPIB();
	void sendGPIB();
	void clearGPIB();
	void setSMUon();
	void setSMUoff();
	void setSMUval();
	void setSMUCout();
	void setSMUVout();
	void getSMUMeas();
	void calibrate();

	void setVDDA1();
	void setVDDA2();
	void setVDDD1();
	void setVDDD2();
	void selectSMU();
	void updateMeasurements();
	void updateEEPROM();
	void dumpEEPROM();
	void selLoad();
	void setLoadCurr();
	void setId();
	void enableTimer(bool yes_no);
	void change4WireSense(bool enable);
	void autoTestCalibration();

;
private:
	SiUSBDevice *myUSBdev;
	void * hUSB;
  SimpleQWTPlot *myPlot;
  TGPIB_Interface_USB *myGPIBIf;
  USBPixSTDDCS	*myAdapterCard;
	TGPIB_Keithley24xx *mySMU;
	QString FPGAFileName;
	QTimer *timer;
	void UpdateSystem();
	void CalibrateAll();
	void CalibrateChannel(int ch);
	void Power(bool on_off, int channel = -1);
	void SelectChannel(unsigned char ch);
	bool checkChannel(int channel);

};

