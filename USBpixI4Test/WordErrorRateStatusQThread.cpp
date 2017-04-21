#ifdef CF__LINUX
// #include "win2linux.h"
#elif defined __VISUALC__
#include "stdafx.h"
#endif
#include "WordErrorRateStatusQThread.h"
//#include <QtGui>
#include <QApplication>



WordErrorRateStatusQThread::WordErrorRateStatusQThread(SiUSBDevice *Handle, USBpix *usbpix)
{
  	myUSB = Handle;
	first_header = 0;
	FEI4B = usbpix->FEisFEI4B();

	//configFEMem = 0;
	//configFEMem = new ConfigFEMemory(31, 0, myUSB);
	configReg = 0;
	configReg = usbpix;

	Words = new unsigned int [SRAM_WORDSIZE];
	SRAMdataRB = new unsigned int [SRAM_WORDSIZE];
	counter = 0;
}

WordErrorRateStatusQThread::~WordErrorRateStatusQThread()
{
	//delete configFEMem;
	delete configReg;
	delete Words;
	delete SRAMdataRB;
}

void WordErrorRateStatusQThread::run()
{
	int SRAMFillingLevel = 0;
	bool SRAMFull = false;
	bool MeasDone = false;
	int NrWords = 0;
	double ReceivedWords = 0;
	double NrErrors = 0;
	double ErrorRate = 0;
	int CollectedEvents = 0;
	int TriggerRate = 0;
	int EventRate = 0;
	

	emit RateTestReceivedWords(ReceivedWords);
	emit RateTestReceivedErrors(NrErrors);
	emit RateTestErrorRate(ErrorRate);

	msleep(500);

	configReg->ClearSRAM(8);
	configReg->resetRunModeAdd();

	while (myUSB->HandlePresent())
	{
	  configReg->GetSourceScanStatus(SRAMFull, MeasDone, SRAMFillingLevel, CollectedEvents, TriggerRate, EventRate, 8);

		if (SRAMFull)
		{
			configReg->ReadSRAM(8);
			configReg->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE, 8);
			NrWords = FillWords();
			ReceivedWords = ReceivedWords + NrWords;
			NrErrors = NrErrors + compare(NrWords);

			ErrorRate = (NrErrors + 1) / ReceivedWords;

			emit RateTestReceivedWords(ReceivedWords);
			emit RateTestReceivedErrors(NrErrors);
			emit RateTestErrorRate(ErrorRate);

			QApplication::processEvents();

			configReg->ClearSRAM(8);
			configReg->resetRunModeAdd();
			msleep(500);

		}
		msleep(500);
	}
	return;
}

unsigned int WordErrorRateStatusQThread::FillWords()
{
	bool comp = false;
	int NrWords = 0;

	for (int word = 0; word < 696320; word++)
	{
		if (!comp)
		{
			if (/*configReg->SRAMwordsRB*/SRAMdataRB[word] == 0x00e90000)
			{
				comp = true;
				first_header = word;
			}
			else 
				comp = false;
		}
		if (comp)
		{
			Words[NrWords] = /*configReg->SRAMwordsRB*/SRAMdataRB[word];
			NrWords++;
		}
	}
	return NrWords;
}

unsigned int WordErrorRateStatusQThread::RefillWords(int startvalue)
{
	bool comp = false;
	int ErrorNumber = 0;
	int NrWords = 0;
	int data = 0;

	for (int word = startvalue + first_header; word < 696320; word++)
	{
		data = /*configReg->SRAMwordsRB*/SRAMdataRB[word];

		if (!comp)
		{
			if (/*configReg->SRAMwordsRB*/SRAMdataRB[word] == 0x00e90000)
				comp = true;
			else 
			{
				comp = false;
				ErrorNumber++;
			}
		}
		if (comp)
		{
			Words[NrWords] = /*configReg->SRAMwordsRB*/SRAMdataRB[word];
			//if (Words[NrWords] == 0x00000000)
			//	return NrWords - 1;
			NrWords++;
		}
	}
	return ErrorNumber;
}

unsigned int WordErrorRateStatusQThread::compare(int NrWords)
{
	int NrErrors = 0;
	int word = 0;
	for (int i = 0; i < NrWords; i= i + 4)
	{
		word = Words[i];
		if (Words[i] != 0x00e90000)
		{
			NrErrors++;
			NrErrors = NrErrors + RefillWords(i+4);
		}
	}

	for (int i = 1; i < NrWords; i= i + 4)
	{
		if (Words[i] != 0x00123456)
		{
			NrErrors++;
			NrErrors = NrErrors + RefillWords(i+3);
		}
	}

	for (int i = 2; i < NrWords; i= i + 4)
	{
		word = Words[i];
		if (Words[i] != 0x00789abd)
		{
			NrErrors++;
			NrErrors = NrErrors + RefillWords(i+2);
		}
	}

	for (int i = 3; i < NrWords; i= i + 4)
	{
		word = Words[i];
		if (Words[i] != 0x00cef123)
		{
			NrErrors++;
			NrErrors = NrErrors + RefillWords(i+1);
		}
	}
	if (NrErrors != 0)
	{		
		counter++;
		QString filename;
		filename = QString("error_" + QString::number(counter) + ".dat");
		configReg->FileSaveRB(qPrintable(filename), SRAM_WORDSIZE, false, 8);
	}
	return NrErrors;
}




/*
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
}*/
//QTcpSocket socket;
// connect QTcpSocket's signals somewhere meaningful
//...
//socket.connectToHost(hostName, portNumber);
//exec();
// }
