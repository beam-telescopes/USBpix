#ifndef ISUCBMUSYQTHREAD_H
#define ISUCBMUSYQTHREAD_H
#include <QThread>
#include "ConfigFEMemory.h"
#include "ConfigRegister.h"
#include "SiUSBLib.h"
#include <Qt/qwidget.h>
#include <Qt/qobject.h>
#include <QVector>
#include <QFileDialog>
#include <QStatusBar> 

class IsmuCBusyQThread : public QThread
{
	 Q_OBJECT;
	public:
		IsmuCBusyQThread(SiUSBDevice * Handle);
		~IsmuCBusyQThread();

 public:
     void run();
	 void gotosleep(unsigned long msecs);

signals:
	 void CurrentMaskStep(int step);
	 void ScanStatus(QString status);
	 void ErrorStatus(QString status);

 public:
	ConfigFEMemory * configFEMem;
	//ConfigRegister * configReg;
	SiUSBDevice * myUSB;

 };
#endif
