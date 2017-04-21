#ifndef WORDERRORRATESTATUSQTHREAD_H
#define WORDERRORRATESTATUSQTHREAD_H
#include <QThread>
//#include "ConfigFEMemory.h"
//#include "ConfigRegister.h"
#include <USBpix.h>
#include <SiLibUSB.h>
#include <qwidget.h>
#include <qobject.h>
#include <QVector>
#include <QFileDialog>
#include <QStatusBar> 

class WordErrorRateStatusQThread : public QThread
{
	 Q_OBJECT;
	public:
	 WordErrorRateStatusQThread(SiUSBDevice *, USBpix *);//SiUSBDevice * Handle, bool isFEI4B);
		~WordErrorRateStatusQThread();

 public:
     void run();

signals:
	void RateTestReceivedWords(double value);
	void RateTestReceivedErrors(double value);
	void RateTestErrorRate(double value);

 public:
	//ConfigFEMemory * configFEMem;
	//ConfigRegister * configReg;
	SiUSBDevice *myUSB;
	USBpix *configReg;

private:
	int first_header;
	unsigned int * Words;
	unsigned int FillWords();
	unsigned int RefillWords(int startvalue);
	unsigned int compare(int NrWords);
	int counter;
	bool FEI4B;
	unsigned int * SRAMdataRB/*[SRAM_WORDSIZE]*/;
 };

#endif
