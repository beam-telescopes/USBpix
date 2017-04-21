#ifndef GPAC_EXAMPLE_H
#define GPAC_EXAMPLE_H

#include <QMainWindow>
#include <QTimer>
#include <QFileDialog>

#include "ui_gpac_example.h"

#include "HL_GPAC.h"
#include "TL_USB.h"
#include "SiLibUSB.h"

namespace Ui {
    class GPACMainForm;
}

class GPACMainForm : public QMainWindow
{
	Q_OBJECT

public:
	GPACMainForm(QWidget *parent = 0,  Qt::WindowFlags flags=0);
	~GPACMainForm();

  void onDeviceChange();

public slots:
	void enablePWR0(bool on_off);
	void enablePWR1(bool on_off);
	void enablePWR2(bool on_off);
	void enablePWR3(bool on_off);
	void setPWR0(double val);
	void setPWR1(double val);
	void setPWR2(double val);
	void setPWR3(double val);
	void setVSRC0(double val);
	void setVSRC1(double val);
	void setVSRC2(double val);
	void setVSRC3(double val);
	void setISRC0(double val);
	void setISRC1(double val);
	void setISRC2(double val);
	void setISRC3(double val);
	void setISRC4(double val);
	void setISRC5(double val);
	void setISRC6(double val);
	void setISRC7(double val);
	void setISRC8(double val);
	void setISRC9(double val);
	void setISRC10(double val);
	void setISRC11(double val);
	void updateMeasurements();
	void setCurrLim();
	void enableTimer(bool yes_no);

private:
	// UI
	Ui::GPACMainForm *ui;
	// USB device
	SiUSBDevice	*myUSBdev;
	// USB transfer layer object (new framework approach introduced with GPAC)
	TL_USB  *myTLUSB;
	// general purpose analog card
  HL_GPAC	*myAnalogCard;
	// pointer to analog channels for convenience
  PowerSupply      *PWR[MAX_PWR];
	VoltageSource    *VSRC[MAX_VSRC];
	InjVoltageSource *VINJ[MAX_VINJ];
	CurrentSource    *ISRC[MAX_ISRC];

	QTimer  *timer;
	void UpdateSystem();
};

#endif // GPAC_EXAMPLE_H
