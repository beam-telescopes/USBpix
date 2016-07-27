#pragma once

#include <QWidget>
#include <QObject>
#include <QFileDialog>
#include <QStatusBar>
#include <QTreeWidget>
#include <QMainWindow>
#include <QMenu>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QTextStream>
#include <QProgressBar>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>



#include "ui_MainForm.h"
#include "SiUSBLib.h"

#include <math.h>

namespace Ui {
    class MainForm;
}

class MainForm : public QMainWindow
{

Q_OBJECT
public:
	MainForm(QWidget *parent = 0);
	~MainForm(void);

public:
  void doflashEEPROM();


public slots:
  void onDeviceChange();
	void openFPGADialog();
	void openEEPROMDialog();
	void openControllerDialog();
	void loadFPGA();
	void loadController();
	void flashEEPROM();
	void updateDeviceId();
	void listItemClicked(QTreeWidgetItem *item, int column);
	void showAbout();
	void writeClicked();
	void readClicked();
  void selectMode(QString mode);
	void runBenchmark();
	void expertModeChanged();

private:
  Ui::MainForm *ui;
	SiUSBDevice *myUSBdev;
	QString EEPROMFileName;
	QString FPGAFileName;
	QString ControllerFileName;
	void UpdateDevList();
	void SetCurrentUSBDevice(int id);
	QIcon *silab_usb_ico;
  QList<QTreeWidgetItem *> devListItems;
  QTreeWidgetItem *currentItem;
	QAction *showAboutAct;
	QAction *expertModeAct;
  
	QwtPlotCurve *curve1;
	QwtPlotCurve *curve2;

	static unsigned long MyThreadFunc(void* ptr);



protected:
	void createActions();
	void DisplayDeviceInfo();
};

