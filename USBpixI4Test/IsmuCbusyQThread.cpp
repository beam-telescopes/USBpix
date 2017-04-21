#ifdef __VISUALC__
#include "stdafx.h"
#endif
#include "IsmuCbusyQThread.h"
//#include <QtGui>




IsmuCBusyQThread::IsmuCBusyQThread(SiUSBDevice * Handle)
{
	myUSB = Handle;
	
	configFEMem = 0;
	configFEMem = new ConfigFEMemory(31, 0, myUSB);
	//configReg = new ConfigRegister(myUSB);
}

IsmuCBusyQThread::~IsmuCBusyQThread()
{
	delete configFEMem;
	//delete configReg;
}

void IsmuCBusyQThread::gotosleep(unsigned long msecs)
{
	QThread::msleep(msecs);
}

void IsmuCBusyQThread::run()
 {
	 int MaskStep = 0;
	 bool ScanBusy = 1;
	 bool ScanReady = 0;
	 bool ScanCanceled = 0;
	 bool EOEerror = 0;

	 //msleep(2000);

	 while(myUSB->HandlePresent() && ScanBusy)
	 {
		//configFEMem->GetScanStatus(MaskStep, ScanBusy, ScanReady, ScanCanceled, EOEerror);

		if (ScanCanceled)
		{
			//configFEMem->ResetStatusFlags();
			emit ScanStatus("canceled");
			emit ErrorStatus("scan canceled");
			return;
		}
		if (EOEerror)
		{
			//configFEMem->ResetStatusFlags();
			emit ScanStatus("canceled");
			emit ErrorStatus("EoE-error, canceled");
			return;
		}
		if (ScanReady)
		{
			//configFEMem->ResetStatusFlags();
			//configFEMem->GetScanStatus(MaskStep, ScanBusy, ScanReady, ScanCanceled, EOEerror);
			emit CurrentMaskStep(MaskStep);
			emit ScanStatus("scan ready");
			emit ErrorStatus("no error");
			return;
		}
		emit CurrentMaskStep(MaskStep);
		emit ScanStatus("scanning");
		emit ErrorStatus("no error");

		msleep(1000);
	 }
     //QTcpSocket socket;
     // connect QTcpSocket's signals somewhere meaningful
     //...
     //socket.connectToHost(hostName, portNumber);
    //exec();
 }
