#ifndef MAINFORM_H
#define MAINFORM_H
#include <QtCore>
#include <qwidget.h>
#include <qobject.h>
#include <QTextEdit>
#include <QVector>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QThread>

#include <string>
#include <algorithm>
#ifdef CF__LINUX
#include <sys/stat.h>
#else
#include <direct.h>
#endif

#include "ui_MainForm.h"

#include "SiUSBLib.h"
#include "USBpix.h"
#include "WordErrorRateStatusQThread.h"
#include "USBPixI4DCS.h"
#include "defines.h"
//#include "RegisterTest.h"
#include "SimpleQWTPlot.h"
#include "Pixel.h"
#include "FitClass.h"
#include "qwt_symbol.h"
#include "qwt_scale_engine.h"
#include "math.h"
#include "image.h"
#include "GRegister.h"
//#include "HitmapPlot.h"

double erfc(double Mean, double Sigma, double x);

class MainForm :
	public QWidget, protected Ui_MainForm
{

	Q_OBJECT;

public:
	MainForm(QWidget *parent = 0);
	virtual ~MainForm(void);

public:
	void onDeviceChange();
	//Start additions by Stewart
	int BintoDec(QString string);
	QTextEdit* xxDirector(int number);
	void xxChangeTab(QPushButton* button);
	void xxUpdateArray(int index,int value);
	int xxButtontoInt(QPushButton* button);
	void xxInitializeFEarray();
	//End additions by Stewart

	int runBoardTest_pixelregtest_errors;
	int runBoardTest_pixelregtest_tests;

	public slots:
		void SetBoardID();
		void openBitFileDialog();
		void openReadGlobalFileDialog();
		void openBixFileDialog();
		void openSaveGlobalFileDialog();
		void confFPGA();
		void confController();
		void openHexFileDialog();
		void flashControllerEEPROM();
		void setVDDA1();
		void setVDDA2();
		void setVDDD1();
		void setVDDD2();
		void setVDDPLL();
		void resetAll();
		void setRunMode();
		void setTLUMode();
		void setCalMode();
		void setToTMode();
		void stopXCK();
		void runXCK();
		void setFEFlavor(const QString flavor);
		void writeCableLength();
		void writeInvertXck();
		void enableHardRstA();
		void disableHardRstA();
		void enableHardRstD1();
		void disableHardRstD1();
		void enableHardRstD2();
		void disableHardRstD2();
		void writeSelAltBus();
		void LoadRegABDacLd(); 
		void LoadRegABStbLd();
		void writeSelCMD();

		void SetIoMUX0();
		void SetIoMUX1();
		void SetIoMUX2();
		void SetIoMUX3();

		void sendBypassAB();
		void sendBypassC();

		void PowerFEUp();
		void PowerFEDown();

		void startWaferProbingLoop();
		void CopyPixeltestFile(QString path, QString filenamebasis, int index);
		bool WriteDCStoFile(QString filename);

		void startHITORScan();

		void SetINMUXselectMode();
		void ChangeAdapterFlavor();
		void SetFEToRead();
		void write_GobalReset();
		void sendEnDataTake();
		void sendEnConfMode();
		void sendGlobalPulse();
		void readEPROM();
		void burnEPROM();
		void setEventCounterMode();
		void startMeasurement();
		void cancelMeasurement();
		void startCalibrationTest();
		//void setEventCounterMode();
		//void startMeasurement();
		//void cancelMeasurement();
		//void writeCableLength();
		void write_TriggerInput();
		void write_enableCMDLV1();
		void setSlowControlWriteRegAB();
		void setSlowControlWriteRegC();
		void setSlowControlEfuse();
		void setSlowControl();
		void startEfuseClock();
		void stopEfuseClock();
		void writeDataRateMode();
		void incrSyncCLKPhase();
		void decrSyncCLKPhase();
		void StartSyncPhaseCheck();
		void StartSyncScan();
		void printSyncScanPlot();
		void StorePatternChanged();
		void writeSaveStrb();
		void writeStartStrb();
		void writeStopStrb();
		void sendTriggerCommand();
		void sendStrbCommand();
		bool readGlobalReg();
		void save_GlobalChanges();
		void write_Global();
		void write_Pixel();
		//void read_Pixel();
		//void start_Sync_1();
		//void start_Sync_2();
		//void start_Sync_3();
		void readRegister();
		void saveGloChanges();
		void read_GlobalFile(std::string filename);
		void UpdateGUItoGlobalVals();
		void connect_save_GlobalFile();
		void save_GlobalFile(std::string filename);
		void updateFEDataTextEdit(int index, int value);
		void read_Global();
		void write_Vthin();
		void write_VthinAltCoarse();
		void write_VthinAltFine();
		void write_PrmpVbp_R();
		void write_PrmpVbp();
		void write_DisVbn_CPPM();
		void write_DisVbn();
		void write_TdacVbp();
		void write_Amp2Vbn();
		void write_Amp2VbpFol();
		void write_Amp2Vbp();
		void write_PrmpVbp_T();
		void write_Amp2Vbpff();
		void write_FdacVbn();
		void write_PrmpVbp_L();
		void write_PrmpVbnFol();
		void write_PrmpVbpf();
		void write_PrmpVbnLcc();
		void write_PlsrDac();
		void write_ChipLatency();
		void write_Reg29();
		//void write_Reg17();
		//void write_Reg18();
// more regs here
		void write_Reg_Nr();
		void read_Reg_Nr();
		void read_Reg2();
		void read_Reg18();
		void read_Reg29();

		void write_TDAC();
		//void write_TDAC(int TDAC);
		void write_FDAC();
		void write_Enable();
		void write_Cap0();
		void write_Cap1();
		void write_DigInj();
		void write_Hitbus();
		//void write_Latency();
		//void write_GDAC();
		//void write_DOMUX();
		//void write_MONHITMUX();
		//void write_TESTPIXELMUX();
		//void write_VCALDAC();
		//void write_IVDD2DAC();
		//void write_ILDAC();
		//void write_IL2DAC();
		//void write_IFDAC();
		//void write_IPDAC();
		//void write_IP2DAC();
		//void write_ITrimIfDAC();
		//void write_ITh1DAC();
		//void write_ITrimThDAC();
		//void write_ITh2DAC();
		//void write_IDDAC();
		//void write_enableHitbus();
		//void write_enableHighCap();
		//void write_enableDigitInj();
		//void write_SoftReset();
		//void write_RefReset();
		//void write_enableBufferBoost();
		//void write_enableExtInj();
		void read_HITBUS();
		void read_CAP0();
		void read_CAP1();
		void read_TDAC();
		void read_FDAC();
		void read_ENABLE();
		void read_Pixel();
		//void test_PixelRegister();
		void test_GlobalRegister();
		int DoGlobalTest();
		void testPixelReg();
		//void set_PixelSelect(int pixnr, int latch);
		void read_SRAM();
		void write_SRAM();
		void clear_SRAM();
		void reset_SRAMCounter();
		void restartDataTake();
		void setScanType();
		void start_Scan();
		//void start_ScanMuC();
		//void cancel_Scan();
		void print_Plot();
		void refresh_DCS();
		void openHITBUSFileDialog();
		void openENABLEFileDialog();
		void openCAP0FileDialog();
		void openCAP1FileDialog();
		void openTDACFileDialog();
		void openFDACFileDialog();
		void openDIGINJFileDialog();

		bool write_TDACFile();
		//bool write_TDACFile(const char * filename);
		bool write_FDACFile();
		bool write_HITBUSFile();
		bool write_DIGINJFile();
		bool write_CAP0File();
		bool write_CAP1File();
		bool write_ENABLEFile();


		void start_WordErrorRateTest();
		void stop_WordErrorRateTest();
		//Start additions by Stewart
		void zzSendtoChip();
		void zzWriteRegister();
		void zzReadRegister();
		void zzWriteFrontEnd();
		void zzGblReset();
		void zzGblPulse();
		void zzLevel1Trigger();
		void zzCalibrationPulse();
		void zzBunchCounterReset();
		void zzEventCounterReset();
		void zzCheckChipIDLength();
		void zzCheckWrFELength();
		void zzCheckRegWriteLength();
		void zzClearData_Reg();
		void zzClearData_FE();
		void zzClearData_Pattern();
		void zzClearData_BitStream();
		void zzCopyPattern();
		void zzchangeLCD();
		void zzSpacer();
		void zzRunMode();
		void zzConfigureMode();
		void zzchangecolor();
		void zzMultiCommand(int checked);
		void zzviewRegister();
		void zzviewShiftRegister();
		void zzSRAMtoFILE();
		void zzSaveLog();
		void zzClearSRAM();

		void xxChangeColor(QPushButton * button);
		void xxbuttonClicked();
		void xxShowCurrent(const QImage &i);
		void xxBitStreamPreviewUpdate();
		void xxRowLCDs(int left, int right);
		void xxChangeDC();
		void xxPattern();
		void xxFEDisplayUpdate();
		void xxPixelsUpdate();
		void xxUpdateEverything();
		void xxCopyDCtoChip();
		void xxChangeRows(int row);
		void xxSaveConfig();
		void xxLoadConfig();
		void xxSend();
		void xxUpdateFEDisplay();
		void xxShowHits();
		void xxClearHitMap();
		void xxShowToTs();
		void xxClearToTMap();
		void xxCheckChipIDLength();
		QString Get672BitWord(QString value);
		void SendGlobalPulse(int pulse_width);

		// Wafer Test tab
		void pTest1Chip();
		void CallCalib();
		void CallwrRegister();
		void CalltestLatches();
		void CallEnable0();
		void CallEnable1();
		void CallEnable2();
		void CallEnable3();
		void pTurnPowerOff();
		void pSaveProbeResults();
		void disconnectdevice();
		void IREFconnect();		
		void IREFrun();
		void VTHINCconnect();
		void VTHINCrun();
		void PULSERconnect();
		void PULSERrun();
		void clearlog();
		void TestWRRDRegister(QString reg_name, QString reg_value);
		void TestPixelLatches();
		void EnablePixels4thDC(int dc_addr);

		//Parser Code
		void pread_fileName();
		void pparse_file();
		void psave_debug();
		void prun_command(int, QString);
		void pprocess_command(QString,QString);
		void pprocess_task(QString);
		void pread_taskfileName();
		void pget_LV1data(int opt);
		int pget_BCIDwithData();
		void PlsrDelayScan(int num);
		void IDACScan(int num, int init, int last, int step_size);
		void TimeWalk(int DACinitial, int DACFinal, int step_size, int delay_init,float delay_step);
		QString pget_SRdata(/*int opt*/);
		void pget_Errors(int opt);
		unsigned pread_Register(QString name);
		void pwrite_Register(QString name, int value);
		void pwrite_FE(QString data);
		void pwrite_ShadowRegister(QString name, int newvalue/*, bool forcewrite*/);
		
		bool NeedsToBeReversed(int reg, QString mask);
		//end Parser Code

				//David FIXME
		void setupScanChainBlock();
		QString loadScanChainFromFile(int block, QString filename, int pattern, bool& SEstate, bool& SIstate, bool& CLKstate);
		QString loadScanChainTargetFromFile(QString filename, int patternnr);
		void runScanChain();
		void showScanChainVectorLength();
		void loadScanChainPattern();
		void runBoardTest();
		bool CompareScanChainResult();
		void runBoardTest_configured(QString text);
		void runBoardTest_syncscan_done(QString text);
		void runBoardTest_globalregtest_done(int testnr);
		void runBoardTest_pixelregtest();
		void runBoardTest_pixelregtest_done(int error, QString testreg);
		void runBoardTest_pixelregtest_hitbus(int error) { runBoardTest_pixelregtest_done(error, QString("Hitbus")); };
		void runBoardTest_pixelregtest_cap0(int error) { runBoardTest_pixelregtest_done(error, QString("Cap0")); };
		void runBoardTest_pixelregtest_cap1(int error) { runBoardTest_pixelregtest_done(error, QString("Cap1")); };
		void runBoardTest_pixelregtest_tdac(int error) { runBoardTest_pixelregtest_done(error, QString("TDAC")); };
		void runBoardTest_pixelregtest_fdac(int error) { runBoardTest_pixelregtest_done(error, QString("FDAC")); };
		void runBoardTest_pixelregtest_enable(int error) { runBoardTest_pixelregtest_done(error, QString("Enable")); };
		
		void enableRJ45();
		void disableRJ45();
		void readRJ45();
		void initChip();

		void setTLUdelay();
		void readTLUdelay();
		void setTLUlength();
		void readTLUlength();

signals:
		//Start additions by Stewart 
		void SendtoFE();
		void xxemitPad(QPushButton*);
		void xxupdateBitStreamPreview(); 
		void xxSetRowLCDs(int,int);
		void xxUpdate();
		void xxupdateFEDisplay();
		void xxupdatePixels();
		void xxSetRows(int row);
		void xxdisplay(const QImage&);
		void xzSend();
		void xzClear();
		void xzViewRegData();
		//End additions by Stewart
		void yzA1();
		void yzA2();
		void yzD1();
		void yzD2();
		void dataratesignal();
		void selcmdenab();

		void register_data(int i);
		void FEMemWriteChanged(int j);
		void RBglobal_line(QString line);
		void Data_glo_line(QString line);

		void TESTNR(int test_nr);
		void ERRORNR(int error_nr);
		void THRESHOLD(int th);
		void SIGMA(int si);
		void THRESHOLDDAC(int th);
		void SIGMADAC(int si);
		void NOOCNRHITS(int hits);
		void FittingStatus(QString status);
		void LVDSOUTISignal(int value);
		void LVDSOUTVSignal(int value);
		void DISVBNASignal(int value);
		void DISVBNBSignal(int value);
		void TRIGCNTSignal(int value);
		void EMPTYRECORDSignal(int value);
		void CLK2OUTCFGSignal(int value);
		void DISABLE8B10BSignal(int value); 
		void CONFADDRENABLESignal(int value);
		void HITBUSLCD(int value);
		void CAP0LCD(int value);
		void CAP1LCD(int value);
		void TDACLCD(int value);
		void FDACLCD(int value);
		void ENABLELCD(int value);

		void PhaseShiftCountSignal(int value);
		void SyncCheckResultSignal(double value);


		void RateTestReceivedWords(int value);
		void RateTestReceivedErrors(int value);
		void RateTestErrorRate(int value);

		void SRAMFillingLevel(int value);
		void CountedEvents(int value);
		void TriggerRateSignal(double TriggerRate);

		void READREGNRSIGNAL(int value);


private:
	void build2DArray();
	void fit_SCurve(int quantity,/* float chicut, float xmin, float xmax,*/ int pixnr);
	void write_Thresholds(int stepquant, int ScanStartVal, int ScanStepSize);
	void writeScanConfigFile(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSiz, int InjCount, int MaskStepSiz, int MaskStepCount, int ShiftMask, bool all_DCs, QString filename);
	void writeBinnedHitHisto(int stepquant, QString filename);
//	void writeBinnedTOTHisto(int stepquant, int ScanStartVal, int ScanStepSize, QString filename);
	void writeSFit(int stepquant, int ScanStartVal, int ScanStepSize);

	void plot_dispersions();
	void plot_SCurve(int startpixel, int pixelquant, int stepquant, int VcalStart, int VcalStop, int VcalStepwidth);
	void writeNoiseOccFile(const char * filename);
	void NoiseOccupancyMeasurementLoop();
	void decrementAllTDACs(int steps);

	void FillToTSimplePlot();
	void FillLV1SimplePlot();


	double * x_val, * y_val;
	QVector<int> thresvector;
	QVector<int> sigmavector;
	QVector<double> chi2vector;

	QString FPGAFileName;
	QString ControllerFileName;
	QString EEPROMFileName;
	QString readGlobalFileName;
	QString readPixelFileName;
	QString saveGlobalFileName;
	QString ResultFileName;
	QString TDACFileName;
	QString FDACFileName;
	QString HITBUSFileName;
	QString CAP0FileName;
	QString CAP1FileName;
	QString ENABLEFileName;
	QString DIGINJFileName;


	void UpdateSystem();
	QString RegisterValue;	
	QString FreqError;
	QString FreqCorrect;

	std::ofstream arrayout;

	//ConfigFEMemory * configFEMem;
//	ConfigFPGA * configFPGA;
	//ConfigRegister * configReg;
	USBpix * myUSBpix;
	USBPixDCS  * dcsInst;
	//IsmuCBusyQThread * IsmuCBusy;
	WordErrorRateStatusQThread * RateStatus;

	//RegisterTest * regTest;
	SimpleQWTPlot * ScanPlot;
	SimpleQWTPlot * ThresDispPlot;
	SimpleQWTPlot * NoiseDispPlot;
	QwtPlotCurve * SCurve;
	QwtPlotCurve * FitCurve;
	QwtPlotCurve * ThresDispCurve;
	QwtPlotCurve * NoiseDispCurve;
	QwtPlotCurve * SyncScanCurve;
	SimpleQWTPlot * SyncScanPlot;
	SimpleQWTPlot * ToTSimplePlot;
	SimpleQWTPlot * LV1SimplePlot;
	Pixel * PixHelp;
	FitClass * Fit;
	SiUSBDevice * hUSB;
	QwtScaleEngine * log10ScaleEngine;
	QColor * QwtPlotColor;
	//HitmapPlot * myHitmap;

	double * hitmapdata;

	int NoiseOccArray[10000];
	int NoiseOccRep;
	int TDACdecr;
	int PhaseShiftCount;
	int chipID;
	bool isFEI4B;

	unsigned int * SRAMdataRB/*[SRAM_WORDSIZE]*/;
	//Parser Code
	QString ppath;
	QList<GRegister> pgrList;
	QList<GRegister> psrList;
	//end Parser Code
};  

#endif
