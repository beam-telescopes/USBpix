
#ifdef CF__LINUX
// #include "win2linux.h"
#include <unistd.h>
#elif defined __VISUALC__
#include "stdafx.h"
#endif
#include <upsleep.h>

#include "MainForm.h"
//start additions by Stewart
QStringList xxFEarray;
int CURRENT_DC;
int xxpixelswitch=0;
int xxHitarray[80][366]; //this array stores the position [x][y] of a hit and its TOT (between 0-14). See xxUpdateFEDisplay().
int xxToTarray[80][366]; //this array stores the position [x][y] of a hit and its TOT (between 0-14). 
int dc_index = 0;  //double column index (to check whether output data comes from addressed pixel)
int row_index = 1; //row column index (to check whether output data comes from addressed pixel)
int error_cnt = 0;
int gr_index = 0; //global register index
int sr_index = 0; //shift register index
QString alt_word = "0000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010";
//end additions by Stewart
bool run_clk = false;

// This is for the GPIB operations
//#include "..\PixGPIB\PixGPIBDevice.h"
//#include "..\PixGPIB\PixGPIBError.h"
//#include "..\PixGPIB\ni488.h"
//unsigned int m_Device;

//----------Konstruktor-----------------------------------
MainForm::MainForm(QWidget *parent):QWidget(parent)
{
	setupUi(this);
	//setLayout(verticalLayout_2);

	// connections System-Configuration-Tab
	connect(BoardIDSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetBoardID()));
	connect(confFPGAButton, SIGNAL(clicked()), this, SLOT(confFPGA()));
	connect(BitfileDialogButton, SIGNAL(clicked()), this, SLOT(openBitFileDialog()));
	connect(confControllerButton, SIGNAL(clicked()), this, SLOT(confController()));
	connect(BixfileDialogButton, SIGNAL(clicked()), this, SLOT(openBixFileDialog()));
	connect(HexfileDialogButton, SIGNAL(clicked()), this, SLOT(openHexFileDialog()));
	connect(flashControllerEEPROMButton, SIGNAL(clicked()), this, SLOT(flashControllerEEPROM()));
	connect(CableLengthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(writeCableLength()));

	connect(writeResetAllButton, SIGNAL(clicked()), this, SLOT(resetAll()));
	connect(runModeButton, SIGNAL(clicked()), this, SLOT(setRunMode()));
	connect(tluModeButton, SIGNAL(clicked()), this, SLOT(setTLUMode()));
	connect(calModeButton, SIGNAL(clicked()), this, SLOT(setCalMode()));
	connect(totModeButton, SIGNAL(clicked()), this, SLOT(setToTMode()));
	connect(stopXCKButton, SIGNAL(clicked()), this, SLOT(stopXCK()));
	stopXCKButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	connect(InvertXckCheckBox, SIGNAL(stateChanged(int)), this, SLOT(writeInvertXck()));

	connect(writeGlobalResetButton, SIGNAL(clicked()), this, SLOT(write_GobalReset()));
	connect(sendStrobeButton, SIGNAL(clicked()), this, SLOT(sendStrbCommand()));
	connect(sendTriggerButton, SIGNAL(clicked()), this, SLOT(sendTriggerCommand()));
	connect(EnDataTakeButton, SIGNAL(clicked()), this, SLOT(sendEnDataTake()));
	connect(EnConfModeButton, SIGNAL(clicked()), this, SLOT(sendEnConfMode()));
	connect(sendGlobalPulseButton, SIGNAL(clicked()), this, SLOT(sendGlobalPulse()));
	connect(readEPROMButton, SIGNAL(clicked()), this, SLOT(readEPROM()));
	connect(burnEPROMButton, SIGNAL(clicked()), this, SLOT(burnEPROM()));


	connect(FEFlavorComboBox, SIGNAL(currentIndexChanged(const QString)), this, SLOT(setFEFlavor(const QString)));

	connect(HardRstAButton, SIGNAL(clicked()), this, SLOT(enableHardRstA()));
	connect(HardRstD1Button, SIGNAL(clicked()), this, SLOT(enableHardRstD1()));
	connect(HardRstD2Button, SIGNAL(clicked()), this, SLOT(enableHardRstD2()));
	HardRstAButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	HardRstD1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	HardRstD2Button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));

	connect(SelAltBusCheckBox, SIGNAL(stateChanged(int)), this, SLOT(writeSelAltBus()));
	connect(SelCmdCheckBox, SIGNAL(stateChanged(int)), this, SLOT(writeSelCMD()));

	connect(IoMUX0CheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetIoMUX0()));
	connect(IoMUX1CheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetIoMUX1()));
	connect(IoMUX2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetIoMUX2()));
	connect(IoMUX3CheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetIoMUX3()));

	writeBypassButton->hide();

	connect(RegABDacLdCheckBox, SIGNAL(stateChanged(int)), this, SLOT(LoadRegABDacLd()));
	connect(RegABStbLdCheckBox, SIGNAL(stateChanged(int)), this, SLOT(LoadRegABStbLd()));

	connect(writeSRAMButton, SIGNAL(clicked()), this, SLOT(write_SRAM()));
	connect(clearSRAMButton, SIGNAL(clicked()), this, SLOT(clear_SRAM()));
	connect(readSRAMButton, SIGNAL(clicked()), this, SLOT(read_SRAM()));
	connect(resetSRAMCounterButton, SIGNAL(clicked()), this, SLOT(reset_SRAMCounter()));
	connect(RetartDataTakeButton, SIGNAL(clicked()), this, SLOT(restartDataTake()));

	connect(TriggerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(write_TriggerInput()));
	connect(CMDLV1CheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_enableCMDLV1()));
	connect(SlowContrWrABCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegAB()));
	connect(SlowContrWrCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegC()));
	connect(SlowContrCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControl()));
	connect(SlowContrEfuseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlEfuse()));
	connect(DataRateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(writeDataRateMode()));
	connect(IncrPhaseButton, SIGNAL(clicked()), this, SLOT(incrSyncCLKPhase()));
	connect(DecrPhaseButton, SIGNAL(clicked()), this, SLOT(decrSyncCLKPhase()));
	connect(this, SIGNAL(PhaseShiftCountSignal(int)), PhaseShiftCountLCD, SLOT(display(int)));
	connect(StartSyncCheckButton, SIGNAL(clicked()), this, SLOT(StartSyncPhaseCheck()));
	connect(this, SIGNAL(SyncCheckResultSignal(double)), SyncCheckResultLCD, SLOT(display(double)));
	connect(StartSyncScanButton, SIGNAL(clicked()), this, SLOT(StartSyncScan()));
	connect(printSyncScanPlotButton, SIGNAL(clicked()), this, SLOT(printSyncScanPlot()));
	connect(UseStorePatternModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(StorePatternChanged()));
	connect(INMUXselectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetINMUXselectMode()));

	connect(AdapterFlavorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeAdapterFlavor()));
	connect(DOMUXSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetFEToRead()));


	// connections Strobe-Settings-Tab
	connect(L_StrobeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(writeSaveStrb()));
	connect(L_LV1SpinBox, SIGNAL(valueChanged(int)), this, SLOT(writeSaveStrb()));
	connect(D_LV1SpinBox, SIGNAL(valueChanged(int)), this, SLOT(writeSaveStrb()));
	connect(QuantitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(writeSaveStrb()));
	connect(FreqSpinBox, SIGNAL(valueChanged(int)), this, SLOT(writeSaveStrb()));

	connect(writeStartButton_2, SIGNAL(clicked()), this, SLOT(writeStartStrb()));
	connect(this, SIGNAL(register_data(int)), ReadRegisterlcdNumber, SLOT(display(int)));
	connect(readRegisterButton, SIGNAL(clicked()), this, SLOT(readRegister()));

	connect(StartWaferLoopButton, SIGNAL(clicked()), this, SLOT(startWaferProbingLoop()));

	connect(startHITORScanButton, SIGNAL(clicked()), this, SLOT(startHITORScan()));
	startHITORScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));

	connect(ScanChainEdit, SIGNAL(textChanged(const QString)), this, SLOT(showScanChainVectorLength()));
	connect(ScanChainPatternSpinBox, SIGNAL(valueChanged(int)), this, SLOT(loadScanChainPattern()));
	connect(ScanChainComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setupScanChainBlock()));
	

	// connections config Global-Tab
	connect(writeGlobalButton, SIGNAL(clicked()), this, SLOT(saveGloChanges()));
	connect(saveGlobalFileButton, SIGNAL(clicked()), this, SLOT(connect_save_GlobalFile()));
	connect(readGlobalFileDialogButton, SIGNAL(clicked()), this, SLOT(openReadGlobalFileDialog()));
	connect(saveGlobalFileDialogButton, SIGNAL(clicked()), this, SLOT(openSaveGlobalFileDialog()));

	//connect(readGlobalButton, SIGNAL(clicked()), this, SLOT(read_Global()));

	connect(LVDSOUTISpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg0()));
	connect(LVDSOUTVSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg0()));
	connect(WriteReg0Button, SIGNAL(clicked()), this, SLOT(write_Reg0()));
	connect(ReadReg0Button, SIGNAL(clicked()), this, SLOT(read_Reg0()));
	connect(this, SIGNAL(LVDSOUTISignal(int)), LVDSOUTILcd, SLOT(display(int)));
	connect(this, SIGNAL(LVDSOUTVSignal(int)), LVDSOUTVLcd, SLOT(display(int)));

	connect(VthinSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Vthin()));
	connect(VthinAltCoarseSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_VthinAltCoarse()));
	connect(VthinAltFineSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_VthinAltFine()));
	connect(PrmpVbp_RSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbp_R()));
	connect(PrmpVbpSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbp()));
	connect(DisVbn_CPPMSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_DisVbn_CPPM()));
	connect(DisVbnSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_DisVbn()));
	connect(TdacVbpSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_TdacVbp()));
	connect(Amp2VbnSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Amp2Vbn()));
	connect(Amp2VbpFolSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Amp2VbpFol()));
	connect(Amp2VbpSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Amp2Vbp()));
	connect(PrmpVbp_TSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbp_T()));
	connect(Amp2VbpffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Amp2Vbpff()));
	connect(FdacVbnSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_FdacVbn()));
	connect(PrmpVbp_LSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbp_L()));
	connect(PrmpVbnFolSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbnFol()));
	connect(PrmpVbpfSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbpf()));
	connect(PrmpVbnLccSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PrmpVbnLcc()));
	connect(PLSRDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_PlsrDac()));
	connect(CHIP_LATENCYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_ChipLatency()));
	connect(DISVBNASpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg1()));
	connect(DISVBNBSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg1()));
	connect(WriteReg1Button, SIGNAL(clicked()), this, SLOT(write_Reg1()));
	connect(ReadReg1Button, SIGNAL(clicked()), this, SLOT(read_Reg1()));
	connect(this, SIGNAL(DISVBNASignal(int)), DISVBNALcd, SLOT(display(int)));
	connect(this, SIGNAL(DISVBNBSignal(int)), DISVBNBLcd, SLOT(display(int)));

	connect(ReadRegNrButton, SIGNAL(clicked()), this, SLOT(read_Reg_Nr()));
	connect(this, SIGNAL(READREGNRSIGNAL(int)), ReadRegNrLcd, SLOT(display(int)));

	connect(TRIGCNTSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg2()));
	connect(EMPTYRECORDSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg29()));
	connect(CLK2OUTCFGCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_Reg29()));
	connect(DISABLE8B10BCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_Reg29()));
	connect(CHIP_LATENCYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg29()));
	connect(CONFADDRENABLECheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_Reg2()));
	connect(WriteReg2Button, SIGNAL(clicked()), this, SLOT(write_Reg2()));
	connect(ReadReg2Button, SIGNAL(clicked()), this, SLOT(read_Reg2()));
	connect(this, SIGNAL(TRIGCNTSignal(int)), TRIGCNTLcd, SLOT(display(int)));
	connect(this, SIGNAL(EMPTYRECORDSignal(int)), EMPTYRECORDLcd, SLOT(display(int)));
	connect(this, SIGNAL(CLK2OUTCFGSignal(int)), CLK2OUTCFGLcd, SLOT(display(int)));
	connect(this, SIGNAL(DISABLE8B10BSignal(int)), DISABLE8B10BLcd, SLOT(display(int)));
	connect(this, SIGNAL(CONFADDRENABLESignal(int)), CONFADDRENABLELcd, SLOT(display(int)));

	connect(WriteReg29Button, SIGNAL(clicked()), this, SLOT(write_Reg29()));
	connect(ReadReg29Button, SIGNAL(clicked()), this, SLOT(read_Reg29()));

	connect(WriteRegNrButton, SIGNAL(clicked()), this, SLOT(write_Reg_Nr()));

	connect(PowerFEButton, SIGNAL(clicked()), this, SLOT(PowerFEUp()));
	PowerFEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));

	connect(EnVdda1CheckBox, SIGNAL(stateChanged(int)), this, SLOT(setVDDA1()));
	connect(EnVdda2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(setVDDA2()));
	connect(EnVddd1CheckBox, SIGNAL(stateChanged(int)), this, SLOT(setVDDD1()));
	connect(EnVddd2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(setVDDD2()));

	connect(StartMeasurementButton, SIGNAL(clicked()), this, SLOT(startMeasurement()));
	connect(StartCalibrationTestButton, SIGNAL(clicked()), this, SLOT(startCalibrationTest()));
	StartMeasurementButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	connect(CounterSettingComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEventCounterMode()));
	connect(this, SIGNAL(SRAMFillingLevel(int)), SRAMFillingLcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(CountedEvents(int)), ReceivedEventsLcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(TriggerRateSignal(double)), TriggerRateLcdNumber, SLOT(display(double)));


	// more regs here
	//connect(CHIP_LATENCYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg18()));
	//connect(NRCONSLV1SpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Reg18()));

	/*connect(LatencySpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_Latency()));
	connect(GDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_GDAC()));
	connect(DOMUXComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(write_DOMUX()));
	connect(MONHITMUXComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(write_MONHITMUX()));
	connect(TESTPIXELMUXComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(write_TESTPIXELMUX()));
	connect(VCALDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_VCALDAC()));
	connect(IVDD2SpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_IVDD2DAC()));
	connect(ILDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_ILDAC()));
	connect(IL2DACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_IL2DAC()));
	connect(IFDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_IFDAC()));
	connect(IPDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_IPDAC()));
	connect(IP2DACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_IP2DAC()));
	connect(ITrimIfDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_ITrimIfDAC()));
	connect(ITh1DACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_ITh1DAC()));
	connect(ITh2DACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_ITh2DAC()));
	connect(ITrimThDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_ITrimThDAC()));
	connect(IDDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_IDDAC()));

	connect(writeStartButton, SIGNAL(clicked()), this, SLOT(writeStartStrb()));
	connect(writeStopButton, SIGNAL(clicked()), this, SLOT(writeStopStrb()));

	connect(ENABLEHITBUSCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_enableHitbus()));
	connect(HighCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_enableHighCap()));
	connect(ENABLEDIGITALINJCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_enableDigitInj()));
	connect(ENABLEBUFFERBOOSTCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_enableBufferBoost()));
	connect(ENABLEEXTINJCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write_enableExtInj()));*/

	// connections Read Global-Tab
	connect(readGlobalRegButton, SIGNAL(clicked()), this, SLOT(readGlobalReg()));
	connect(this, SIGNAL(RBglobal_line(const QString)), FEDataRBTextEdit, SLOT(setText(const QString)));
	connect(this, SIGNAL(Data_glo_line(const QString)), FEDataTextEdit, SLOT(setText(const QString)));

	// connections Pixel-Reg-Tab
	connect(writePixelRegButton, SIGNAL(clicked()), this, SLOT(write_Pixel()));
	connect(readPixelRegButton, SIGNAL(clicked()), this, SLOT(read_Pixel()));
	connect(writeTDACButton, SIGNAL(clicked()), this, SLOT(write_TDAC()));

	connect(readHITBUSButton, SIGNAL(clicked()), this, SLOT(read_HITBUS()));
	connect(readCAP0Button, SIGNAL(clicked()), this, SLOT(read_CAP0()));
	connect(readCAP1Button, SIGNAL(clicked()), this, SLOT(read_CAP1()));
	connect(readTDACButton, SIGNAL(clicked()), this, SLOT(read_TDAC()));
	connect(readFDACButton, SIGNAL(clicked()), this, SLOT(read_FDAC()));
	connect(readENABLEButton, SIGNAL(clicked()), this, SLOT(read_ENABLE()));

	connect(TDACSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write_TDAC()));
	connect(writeHitbusButton, SIGNAL(clicked()), this, SLOT(write_Hitbus()));
	connect(writeFDACButton, SIGNAL(clicked()), this, SLOT(write_FDAC()));
	connect(writeEnableButton, SIGNAL(clicked()), this, SLOT(write_Enable()));
	connect(writeCap0Button, SIGNAL(clicked()), this, SLOT(write_Cap0()));
	connect(writeCap1Button, SIGNAL(clicked()), this, SLOT(write_Cap1()));
	connect(writeDigInjButton, SIGNAL(clicked()), this, SLOT(write_DigInj()));

	connect(testPixelRegButton, SIGNAL(clicked()), this, SLOT(testPixelReg()));
	connect(testGlobalRegButton, SIGNAL(clicked()), this, SLOT(test_GlobalRegister()));

	connect(this, SIGNAL(HITBUSLCD(int)), HitbuslcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(CAP0LCD(int)), CAP0lcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(CAP1LCD(int)), CAP1lcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(TDACLCD(int)), TDAClcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(FDACLCD(int)), FDAClcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(ENABLELCD(int)), ENABLElcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(TESTNR(int)), RegTestNrlcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(ERRORNR(int)), RegErrorNrlcdNumber, SLOT(display(int)));

	connect(writeTDACFileButton, SIGNAL(clicked()), this, SLOT(write_TDACFile()));
	connect(readTDACFileDialogButton, SIGNAL(clicked()), this, SLOT(openTDACFileDialog()));

	connect(writeFDACFileButton, SIGNAL(clicked()), this, SLOT(write_FDACFile()));
	connect(readFDACFileDialogButton, SIGNAL(clicked()), this, SLOT(openFDACFileDialog()));

	connect(writeHITBUSFileButton, SIGNAL(clicked()), this, SLOT(write_HITBUSFile()));
	connect(readHITBUSFileDialogButton, SIGNAL(clicked()), this, SLOT(openHITBUSFileDialog()));

	connect(writeENABLEFileButton, SIGNAL(clicked()), this, SLOT(write_ENABLEFile()));
	connect(readENABLEFileDialogButton, SIGNAL(clicked()), this, SLOT(openENABLEFileDialog()));

	connect(writeCAP0FileButton, SIGNAL(clicked()), this, SLOT(write_CAP0File()));
	connect(readCAP0FileDialogButton, SIGNAL(clicked()), this, SLOT(openCAP0FileDialog()));

	connect(writeCAP1FileButton, SIGNAL(clicked()), this, SLOT(write_CAP1File()));
	connect(readCAP1FileDialogButton, SIGNAL(clicked()), this, SLOT(openCAP1FileDialog()));

	connect(writeDIGINJFileButton, SIGNAL(clicked()), this, SLOT(write_DIGINJFile()));
	connect(readDIGINJFileDialogButton, SIGNAL(clicked()), this, SLOT(openDIGINJFileDialog()));

	// Scans Tab
	connect(ScanPixNrSlider, SIGNAL(valueChanged(int)), ScanPixNrLcd, SLOT(display(int))); 
	connect(InjectionQuantitySlider, SIGNAL(valueChanged(int)), InjectionQuantityLcd, SLOT(display(int)));
	connect(StepQuantitySlider, SIGNAL(valueChanged(int)), StepQuantityLcd, SLOT(display(int)));
	connect(ScanPixQuantSlider, SIGNAL(valueChanged(int)), ScanPixQuantlcdNumber, SLOT(display(int)));
	connect(startValueSlider, SIGNAL(valueChanged(int)), startValuelcdNumber, SLOT(display(int))); 
	connect(stopValueSlider, SIGNAL(valueChanged(int)), stopValuelcdNumber, SLOT(display(int)));

	connect(ScanPixNrSlider, SIGNAL(sliderMoved(int)), ScanPixNrLcd, SLOT(display(int))); 
	connect(InjectionQuantitySlider, SIGNAL(sliderMoved(int)), InjectionQuantityLcd, SLOT(display(int)));
	connect(StepQuantitySlider, SIGNAL(sliderMoved(int)), StepQuantityLcd, SLOT(display(int)));
	connect(ScanPixQuantSlider, SIGNAL(sliderMoved(int)), ScanPixQuantlcdNumber, SLOT(display(int)));
	connect(startValueSlider, SIGNAL(sliderMoved(int)), startValuelcdNumber, SLOT(display(int))); 
	connect(stopValueSlider, SIGNAL(sliderMoved(int)), stopValuelcdNumber, SLOT(display(int)));



	connect(ScanComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setScanType()));

	connect(this, SIGNAL(THRESHOLD(int)), ThresholdlcdNumber, SLOT(display(int))); 
	connect(this, SIGNAL(SIGMA(int)), SigmalcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(THRESHOLDDAC(int)), ThresholdDAClcdNumber, SLOT(display(int))); 
	connect(this, SIGNAL(SIGMADAC(int)), SigmaDAClcdNumber, SLOT(display(int)));

	connect(startScanButton, SIGNAL(clicked()), this, SLOT(start_Scan()));
	//connect(startScanMyCButton, SIGNAL(clicked()), this, SLOT(start_ScanMuC()));
	connect(printPlotButton, SIGNAL(clicked()), this, SLOT(print_Plot()));

	connect(refreshDCSButton, SIGNAL(clicked()), this, SLOT(refresh_DCS()));

	connect(StartWordErrorRateTestButton, SIGNAL(clicked()), this, SLOT(start_WordErrorRateTest()));
	connect(StopWordErrorRateTestButton, SIGNAL(clicked()), this, SLOT(stop_WordErrorRateTest()));

	{
	//Start additions by Stewart
	connect(zzSendStream, SIGNAL(clicked()), this, SLOT( zzSendtoChip()));
	//connect(zzStreamLineEdit, SIGNAL(returnPressed()), this, SLOT(zzSendToChip()));
	connect(zzWrReg, SIGNAL(clicked()), this, SLOT( zzWriteRegister()));
	connect(zzLV1, SIGNAL(clicked()), this, SLOT( zzLevel1Trigger()));
	connect(zzBCR, SIGNAL(clicked()), this, SLOT( zzBunchCounterReset()));
	connect(zzECR, SIGNAL(clicked()), this, SLOT( zzEventCounterReset()));
	connect(zzCAL, SIGNAL(clicked()), this, SLOT( zzCalibrationPulse()));
	connect(zzRdReg, SIGNAL(clicked()), this, SLOT( zzReadRegister()));
	connect(zzWrFrontEnd, SIGNAL(clicked()), this, SLOT( zzWriteFrontEnd()));
	connect(zzGlobalReset, SIGNAL(clicked()), this, SLOT( zzGblReset()));
	connect(zzGlobalPulse, SIGNAL(clicked()), this, SLOT( zzGblPulse()));
	connect(zzChipIDLineEdit, SIGNAL(editingFinished()), this, SLOT( zzCheckChipIDLength()));
	connect(zzWrFrontEndLineEdit, SIGNAL(editingFinished()), this, SLOT( zzCheckWrFELength()));
	connect(zzRegWriteLineEdit, SIGNAL(editingFinished()), this, SLOT( zzCheckRegWriteLength()));
	connect(zzClearRegData, SIGNAL(clicked()), this, SLOT( zzClearData_Reg()));
	connect(zzClearFEData, SIGNAL(clicked()), this, SLOT( zzClearData_FE()));
	connect(zzClearPattern, SIGNAL(clicked()), this, SLOT( zzClearData_Pattern()));
	connect(zzClearBitStream, SIGNAL(clicked()), this, SLOT( zzClearData_BitStream()));
	connect(zzSubmitPattern, SIGNAL(clicked()), this, SLOT( zzCopyPattern()));
	connect(zzStreamLineEdit, SIGNAL(textChanged ( const QString &)), this, SLOT(zzchangeLCD()));
	connect(zzCommandSpacer, SIGNAL(clicked()), this, SLOT(zzSpacer()));
	connect(zzRunModeButton, SIGNAL(clicked()), this, SLOT(zzRunMode()));
	connect(zzConfigureModeButton, SIGNAL(clicked()), this, SLOT(zzConfigureMode()));
	connect(zzStreamLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(zzchangecolor()));
	connect(zzMultiCommandCheckBox, SIGNAL(stateChanged(int)), this, SLOT(zzMultiCommand(int)));
	connect(zzViewRegister, SIGNAL(clicked()), this, SLOT(zzviewRegister()));
	connect(zzWriteSRAM, SIGNAL(clicked()), this, SLOT(zzSRAMtoFILE()));
	connect(zzSaveLogButton, SIGNAL(clicked()), this, SLOT(zzSaveLog()));
	connect(zzClearSRAMButton, SIGNAL(clicked()), this, SLOT(zzClearSRAM()));
	zzCommandLog->setReadOnly(true);
	zzEditLabel->hide();
	zzLV1->setToolTip("11101 \nTriggers the acquisition of a new event from the chip.\nThis command simply generates a one clock wide pulse that is sent to the chip for processing");
	zzCAL->setToolTip("10110  0100 \nA calibration pulse is sent in response to a CAL command. \nThe pulse generation is user controllable by means of the CAL register");
	zzBCR->setToolTip("10110  0001 \nThe BCR counter inside the FE is set to zero in response to this command.\nIt has no effect on any other internal structure.");
	zzECR->setToolTip("10110  0010 \nThe ECR command is meant to completely clear the FE data path without touching the configuration of the chip. \nThis commandresets and clears all memory pointesr and data structures.");
	zzRdReg->setToolTip("10110  1000  0001  chipID(4b)  Address(6b)\nRead addressed FE register");
	zzWrReg->setToolTip("10110  1000  0010  chipID(4b)  Address(6b)  Data(16b)\n Write into addressed FE register");
	zzWrFrontEnd->setToolTip("10110  1000  0100  chipID(4b)  00000000  Data(672b)\nWrite configuration data to enable DC");
	zzGlobalReset->setToolTip("10110  1000  1000  chipID(4b)\nReset command; Puts chip in its idle state");
	zzGlobalPulse->setToolTip("10110  1000  1001  chipID(4b)  Width(6b)\nHas variable pulse width and functionality");
	zzRunModeButton->setToolTip("10110  1000  1010  chipID(4b)  111000\nSets chip to run mode");
	zzConfigureModeButton->setToolTip("10110  1000  1010  chipID(4b)  000111\nSets chip to configure mode");
	zzSubmitPattern->setToolTip("Fills the write front end data field with a user specified pattern");
	zzSendStream->setToolTip("Converts the binary string into a unsigned character array.\nThe elements are 8 bits long and are read LSB->MSB.\nIf the bit stream contains a RdReg command, the SRAM is cleared with ConfigReg->clearSRAM.\nThe bit stream is then sent through the USBPix system to the chip via ConfigReg->SendBitstream.");
	zzMultiCommandCheckBox->setToolTip("Allows multiple commands to be issued at once.\nIf checked, commands will append rather than replace current bit stream");
	zzWriteSRAM->setToolTip("Writes SRAM contents to file.");
	zzViewRegister->setToolTip("Displays VR character from SRAM corresponding to the LAST RdReg command issued.");
	zzClearSRAMButton->setToolTip("Clears the USBpix SRAM");
	pTaskFileName->setText("../USBpixI4Test/Parser_files/task_example.txt");
	pFileName->setText("../USBpixI4Test/Parser_files/config_standardcurrent.txt");



	qRegisterMetaType<QImage>("QImage");
	xxInitializeFEarray();
	xxUpdateAlert->hide();
	connect(xxLeftSlider, SIGNAL(valueChanged(int)), xxLeftLCDNumber, SLOT(display(int)));
	connect(xxRightSlider, SIGNAL(valueChanged(int)), xxRightLCDNumber, SLOT(display(int)));
	connect(this, SIGNAL(xxemitPad(QPushButton*)), this, SLOT(xxChangeColor(QPushButton*)));
	connect(xxFEonoff1, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff2, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff3, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff4, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff5, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff6, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff7, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff8, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff9, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff10, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff11, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff12, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff13, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff14, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff15, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff16, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff17, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff18, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff19, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff20, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff21, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff22, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff23, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff24, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff25, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff26, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff27, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff28, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff29, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff30, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff31, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEonoff32, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_1, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_2, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_3, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_4, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_5, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_6, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_7, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_8, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_9, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_10, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_11, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_12, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_13, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_14, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_15, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_16, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_17, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_18, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_19, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_20, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_21, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_22, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_23, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_24, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_25, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_26, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_27, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_28, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_29, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_30, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_31, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_32, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_33, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_34, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_35, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_36, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_37, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_38, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_39, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_40, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_41, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxPixel_42, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView1, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView2, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView3, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView4, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView5, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView6, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView7, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView8, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView9, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView10, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView11, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView12, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView13, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView14, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView15, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxFEView16, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxLeftTopOn, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxLeftTopOff, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxRightTopOn, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxRightTopOff, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxLeftBottomOn, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxLeftBottomOff, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxRightBottomOn, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxRightBottomOff, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxCurrentTabSetter,SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxEntireChipSetter, SIGNAL(clicked()), this, SLOT(xxbuttonClicked()));
	connect(xxSaveConfiguration, SIGNAL(clicked()), this, SLOT(xxSaveConfig()));
	connect(this, SIGNAL(xxupdateBitStreamPreview()), this, SLOT(xxBitStreamPreviewUpdate()));
	connect(this, SIGNAL(xxdisplay(const QImage&)), this, SLOT(xxShowCurrent(const QImage&)));
	connect(this, SIGNAL(xxSetRowLCDs(int,int)), this, SLOT(xxRowLCDs(int,int)));
	connect(this, SIGNAL( xxupdateFEDisplay()), this, SLOT(xxFEDisplayUpdate()));
	connect(this, SIGNAL( xxupdatePixels()), this, SLOT(xxPixelsUpdate()));
	connect(this, SIGNAL(xzViewRegData()), this, SLOT(zzviewRegister()));
	connect(this, SIGNAL(xxSetRows(int)),this, SLOT(xxChangeRows(int)));
	connect(this, SIGNAL(xxUpdate()), this, SLOT(xxUpdateEverything()));
	connect(this, SIGNAL(xzClear()),this,SLOT(zzClearData_BitStream()));
	connect(this, SIGNAL(xzSend()),this,SLOT( zzSendtoChip()));
	//connect(this, SIGNAL(xxUpdateNeeded()),this, SLOT( xxUpdateFlag()));
	connect(xxApplyPattern, SIGNAL(clicked()), this, SLOT(xxPattern()));
	connect(xxViewCurrent, SIGNAL(clicked()), this, SLOT(xxUpdateFEDisplay()));
	connect(xxViewHits, SIGNAL(clicked()), this, SLOT(xxShowHits()));
	connect(xxClearHits, SIGNAL(clicked()), this, SLOT(xxClearHitMap()));
	connect(xxViewToTs, SIGNAL(clicked()), this, SLOT(xxShowToTs()));
	connect(xxClearToTs, SIGNAL(clicked()), this, SLOT(xxClearToTMap()));
	connect(xxUpdateButton, SIGNAL(clicked()),this, SLOT(xxUpdateEverything()));
	connect(xxUpdateButton, SIGNAL(clicked()),this, SLOT(xxUpdateEverything()));
	connect(xxCopytoChip, SIGNAL(clicked()), this, SLOT(xxCopyDCtoChip()));
	connect(xxLoadDCConf, SIGNAL(clicked()), this, SLOT(xxChangeDC()));
	connect(xxSendtoChip, SIGNAL(clicked()), this, SLOT(xxSend()));
	connect(xxLoadConf, SIGNAL(clicked()), this, SLOT(xxLoadConfig()));
	connect(xxChipIDLineEdit, SIGNAL(editingFinished()), this, SLOT( xxCheckChipIDLength()));
	
	// Wafer Test tab
	connect(pushButton_pow, SIGNAL(clicked()), this, SLOT(pTest1Chip()));
	connect(pushButton_conf, SIGNAL(clicked()), this, SLOT(CallCalib()));
	connect(pushButton_gr, SIGNAL(clicked()), this, SLOT(CallwrRegister()));
	connect(pushButton_pr, SIGNAL(clicked()), this, SLOT(CalltestLatches()));
	connect(pushButton_dc0, SIGNAL(clicked()), this, SLOT(CallEnable0()));
	connect(pushButton_dc1, SIGNAL(clicked()), this, SLOT(CallEnable1()));
	connect(pushButton_dc2, SIGNAL(clicked()), this, SLOT(CallEnable2()));
	connect(pushButton_dc3, SIGNAL(clicked()), this, SLOT(CallEnable3()));
	connect(pushButton_pwoff, SIGNAL(clicked()), this, SLOT(pTurnPowerOff()));
	connect(pushButton_savelog, SIGNAL(clicked()), this, SLOT(pSaveProbeResults()));
	connect(pushButton_clearlog, SIGNAL(clicked()), this, SLOT(clearlog()));
	connect(pushButton_IREFconnect, SIGNAL(clicked()), this, SLOT(IREFconnect()));
	connect(pushButton_IREFdisconnect, SIGNAL(clicked()), this, SLOT(disconnectdevice()));
	connect(pushButton_IREFrun, SIGNAL(clicked()), this, SLOT(IREFrun()));
	connect(pushButton_VTHINCconnect, SIGNAL(clicked()), this, SLOT(VTHINCconnect()));
	connect(pushButton_VTHINCdisconnect, SIGNAL(clicked()), this, SLOT(disconnectdevice()));
	connect(pushButton_VTHINCrun, SIGNAL(clicked()), this, SLOT(VTHINCrun()));
	connect(pushButton_PULSERconnect, SIGNAL(clicked()), this, SLOT(PULSERconnect()));
	connect(pushButton_PULSERdisconnect, SIGNAL(clicked()), this, SLOT(disconnectdevice()));
	connect(pushButton_PULSERrun, SIGNAL(clicked()), this, SLOT(PULSERrun()));
	
	for (int i = 0; i < 40; i++) {
		QString qStr = QString::number(i);
		comboBox_pulseraddr->addItem(qStr);
	}

	for (int i = 0; i < 4; i++) {
		QString qStr = QString::number(i);
		comboBox_pulsermode->addItem(qStr);
	}

	for (int i = 0; i < 256; i++) {
		QString qStr = QString::number(i); 
		comboBox_constantvalue->addItem(qStr);
	}

	for (int i = 0; i < 256; i++) {
		QString qStr = QString::number(i);
		comboBox_pulserbias->addItem(qStr);
	}

}


	//End additions by Stewart  

	//Parser Tab
	connect(pBrowseFile, SIGNAL(clicked()), this, SLOT(pread_fileName()));
	connect(pReadButton, SIGNAL(clicked()), this, SLOT(pparse_file()));
	connect(pSaveDebug, SIGNAL(clicked()), this, SLOT(psave_debug()));
	connect(pTaskFileBrowse, SIGNAL(clicked()),this,SLOT(pread_taskfileName()));
	//end Parser Tab

	//David FIXME
	connect(pb_TestScanChain, SIGNAL(clicked()),this,SLOT(testScanChain_D()));
	connect(runScanChainButton, SIGNAL(clicked()),this,SLOT(runScanChain()));
	//	connect(pb_confFEmem_filesave, SIGNAL(clicked()),this,SLOT(test_configFEMemory()));
	pb_TestScanChain->hide();
	pb_confFEmem_filesave->hide();

	// Test Boards Tab
	connect(cmdInitChip, SIGNAL(clicked()), this, SLOT(initChip()));
	connect(cmdStartBoardTest, SIGNAL(clicked()), this, SLOT(runBoardTest()));
	connect(cmdTLUmode, SIGNAL(clicked()), this, SLOT(setTLUMode()));
	connect(cmdEnableRJ45, SIGNAL(clicked()), this, SLOT(enableRJ45()));
	connect(cmdDisableRJ45, SIGNAL(clicked()), this, SLOT(disableRJ45()));
	connect(cmdReadRJ45, SIGNAL(clicked()), this, SLOT(readRJ45()));

	connect(cmdSetTLUDelay, SIGNAL(clicked()), this, SLOT(setTLUdelay()));
	connect(cmdReadTLUDelay, SIGNAL(clicked()), this, SLOT(readTLUdelay()));
	connect(cmdSetTLULength, SIGNAL(clicked()), this, SLOT(setTLUlength()));
	connect(cmdReadTLULength, SIGNAL(clicked()), this, SLOT(readTLUlength()));

	InitUSB();

	QString newTitle = this->windowTitle() + " @ " + QApplication::applicationDirPath();
	this->setWindowTitle(newTitle);

	hUSB = new SiUSBDevice(NULL);

	//configFEMem = 0;
	//configFEMem = new ConfigFEMemory(8, 0, hUSB); // chip_add 8 means: all chips accept these commands...

	//configReg = 0;
	//configReg = new ConfigRegister(hUSB);

	chipID = 8;
	if(FEFlavorComboBox->currentText()=="FE-I4B")
		isFEI4B = true;
	else
		isFEI4B = false;

	myUSBpix = 0;
	myUSBpix = new USBpix(chipID, 0, hUSB, isFEI4B);
	UpdateGUItoGlobalVals();

	//regTest = 0;
	//regTest = new RegigsterTest(hUSB);

	PixHelp = 0;
	PixHelp = new Pixel();

	dcsInst = 0;
	dcsInst = new USBPixSTDDCS(hUSB);
	ChangeAdapterFlavor();
	VsetA1Box->setValue(isFEI4B?dcsInst->PSU[VDDD1]->CalData.DefaultVoltage:dcsInst->PSU[VDDA1]->CalData.DefaultVoltage);
	VsetA2Box->setValue(isFEI4B?dcsInst->PSU[VDDD2]->CalData.DefaultVoltage:dcsInst->PSU[VDDA2]->CalData.DefaultVoltage);
	VsetD1Box->setValue(isFEI4B?dcsInst->PSU[VDDA1]->CalData.DefaultVoltage:dcsInst->PSU[VDDD1]->CalData.DefaultVoltage);
	VsetD2Box->setValue(isFEI4B?dcsInst->PSU[VDDA2]->CalData.DefaultVoltage:dcsInst->PSU[VDDD2]->CalData.DefaultVoltage);
	
	UpdateSystem();
	setFEFlavor(FEFlavorComboBox->currentText());

	x_val = new double [256];
	y_val = new double [256];

	SRAMdataRB = new unsigned int [SRAM_WORDSIZE];

	Fit = 0;
	Fit = new FitClass();

	ScanPlot = new SimpleQWTPlot(SCurvePlot, 0, 1023, -1, 256, QString("Threshold Scan"));
	ScanPlot->setAxisTitle(QwtPlot::xBottom, "Vcal [DAC]");
	ScanPlot->setAxisTitle(QwtPlot::yLeft, "# Hits");

	ThresDispPlot = new SimpleQWTPlot(ThresPlot, -120, 20000, -5, 2000, QString("Threshold Distribution"));
	ThresDispPlot->setAxisTitle(QwtPlot::xBottom, "threshold [electrons]");
	ThresDispPlot->setAxisTitle(QwtPlot::yLeft, "# Pixel");

	NoiseDispPlot = new SimpleQWTPlot(NoisePlot, -4, 400, -10, 6000, QString("Noise"));
	NoiseDispPlot->setAxisTitle(QwtPlot::xBottom, "sigma [electrons]");
	NoiseDispPlot->setAxisTitle(QwtPlot::yLeft, "# Pixel");

	log10ScaleEngine = new QwtLogScaleEngine();
	QwtPlotColor = new QColor();
	SyncScanPlot = new SimpleQWTPlot(SyncScanqwtPlot, -4500, 4500, 0, 0.01, QString("Sync Scan"));
	SyncScanPlot->setAxisTitle(QwtPlot::xBottom, "~phase shift [ps]");
	SyncScanPlot->setAxisTitle(QwtPlot::yLeft, "bit error rate");
	SyncScanPlot->setAxisScaleEngine(QwtPlot::yLeft, log10ScaleEngine);
	SyncScanPlot->setAxisScale(QwtPlot::yLeft, 0.0000001, 0.05, 0);
	SyncScanPlot->setCanvasBackground(Qt::white);

	//SyncScanPlot->enableAxis(QwtPlot::xTop, "~phase shift [ps]");
	//SyncScanPlot->setAxisScale(QwtPlot::xTop, -4500, 4500, 0);
	//SyncScanPlot->setAxisTitle(QwtPlot::xTop, "~phase shift [ps]");

	//hitmapdata = new double [26880];

	//myHitmap = new HitmapPlot(HitmapWidget);
	//myHitmap->setTitle("Hitmap");
	//for (int i = 0; i < 26880; i++)
	//	hitmapdata[i] = i;
	//myHitmap->setRawSamples(HitmapData(hitmapdata));
	//myHitmap->reAdjustColorMap();
	//myHitmap->replot();

	ToTSimplePlot = new SimpleQWTPlot(ToTQwtPlot, 0, 16, -10, 300, QString("ToT Hist"));
	ToTSimplePlot->setAxisTitle(QwtPlot::xBottom, "ToT [bx]");
	ToTSimplePlot->setAxisTitle(QwtPlot::yLeft, "#");

	LV1SimplePlot = new SimpleQWTPlot(LV1QwtPlot, 0, 16, -10, 300, QString("LV1 Hist"));
	LV1SimplePlot->setAxisTitle(QwtPlot::xBottom, "LV1 number");
	LV1SimplePlot->setAxisTitle(QwtPlot::yLeft, "#");
	//ClusteredToTSimplePlot = new SimpleQWTPlot(ClusteredToTPlot, -4, 400, -10, 300, QString("Cl. ToT Hist"));

	thresvector.resize(26880);
	sigmavector.resize(26880);
	chi2vector.resize(26880);

	EEPROMStatusLineEdit->hide();
	ControllerStatusLineEdit->hide();
	FPGAStatusLineEdit->hide();
	WaferLoopProgressBar->hide();

	L_LV1SpinBox->setDisabled(true);

	SelCmdCheckBox->setChecked(true);

	ScanPixNrSlider->setEnabled(false);
	ScanPixNrLcd->setEnabled(false);
	ScanPixQuantSlider->setEnabled(false);
	ScanPixQuantlcdNumber->setEnabled(false);

	//setScanType();	// HK: does not work since FPGA is not configured yet at this time
	scanFileNameLineEdit->setText("./data/scan");


	// QThreads
	//IsmuCBusy = new IsmuCBusyQThread(hUSB);
	//QThread ScanStatus Connections
	//connect(IsmuCBusy, SIGNAL(CurrentMaskStep(int)), MaskSteplcdNumber, SLOT(display(int)));
	//connect(IsmuCBusy, SIGNAL(ScanStatus(QString)), ScanStatusLineEdit, SLOT(setText(QString)));
	//connect(this, SIGNAL(FittingStatus(QString)), ScanStatusLineEdit, SLOT(setText(QString)));
	//connect(IsmuCBusy, SIGNAL(ErrorStatus(QString)), ErrorStatusLineEdit, SLOT(setText(QString)));

	RateStatus = new WordErrorRateStatusQThread(hUSB, myUSBpix);
	connect(RateStatus, SIGNAL(RateTestReceivedWords(double)), RateTestReceivedWordsLCD, SLOT(display(double)));
	connect(RateStatus, SIGNAL(RateTestReceivedErrors(double)), RateTestReceivedErrorsLCD, SLOT(display(double)));
	connect(RateStatus, SIGNAL(RateTestErrorRate(double)), RateTestErrorRateLCD, SLOT(display(double)));

	/*
	if (getenv("USBPIXI4CONF") != NULL)

	{ 
	QString filename;
	filename = QString(getenv("USBPIXI4CONF"))+"/last_config.dat";
	read_GlobalFile(filename.toStdString());
	}
	else 
	read_GlobalFile("last_config.dat");  // HK: system hangs here ??!! FPGA not configured?

	//myUSBpix->WriteGlobal();
	//write_Pixel();
	*/
	PhaseShiftCount = 0;
	emit PhaseShiftCountSignal(PhaseShiftCount);

	INMUXselectComboBox->setDisabled(false);

}

MainForm::~MainForm(void)
{
	cancelMeasurement();

	UPGen::Sleep(1000);
	PowerFEDown();

	if (getenv("USBPIXI4CONF") != NULL)
	{
		QString filename;
		filename = QString(getenv("USBPIXI4CONF"))+"/last_config.dat";
		save_GlobalFile(filename.toStdString());
	}
	else
		save_GlobalFile("last_config.dat");

	//delete configFEMem;
	//delete configReg;
	delete myUSBpix;
	delete Fit;
	delete PixHelp;
	delete dcsInst;
	delete SRAMdataRB;
	//delete regTest;
	//delete IsmuCBusy;

	delete [] x_val;
	delete [] y_val;
}

void MainForm::UpdateSystem()
{
	std::stringstream ssbuf;
	std::string sbuf;
	void * tempHandle = GetUSBDevice(BoardIDSpinBox->value());

	if (tempHandle != NULL)
	{
		hUSB->SetDeviceHandle(tempHandle);
		dcsInst->SetUSBHandle(hUSB);
		ssbuf << hUSB->GetName() << " with ID " << hUSB->GetId() << ", µC FW: " << hUSB->GetFWVersion() << ", Adapter card ID: " << (int)dcsInst->GetId() ;
		sbuf = ssbuf.str();
		statusLabel->setText(sbuf.c_str());
		if (BitfileLineEdit->text() == "")// load default FPGA configuration from resource binary
		{
			//QFile file("../USBpixI4Test/Resources/usbpixi4_V10.bit");
			QFile file("../config/usbpixi4.bit");
			//QFile file(":/FPGA/Resources/usbpixi4_V07.bit");

		   if (file.open(QIODevice::ReadOnly))
		   {
				//file.rename("temp.bit");
				//file.close();
				FPGAFileName = "../config/usbpixi4.bit";
				BitfileLineEdit->setText(FPGAFileName);

				confFPGA();
				//file.remove("temp.bit");
			}
			//confFPGA();
		}
		else 
			confFPGA(); // load current FPGA configuration from file
	}
	else
	{
		hUSB->SetDeviceHandle(NULL);
		sbuf = "no board found";
		statusLabel->setText(sbuf.c_str());
	}

}

void MainForm::onDeviceChange()
{
	if (OnDeviceChange()) // call to SiUSBLib.dll
		UpdateSystem();
}


// ********************* System Configuration-Tab ****************************
void MainForm::SetBoardID()
{
	UpdateSystem();
}

void MainForm::openBitFileDialog()
{
	QString oldfilename = BitfileLineEdit->text();
	FPGAFileName = QFileDialog::getOpenFileName(this,
		tr("Select FPGA Configuration File"), oldfilename, tr("Bit Files (*.bit)"));

	if (!FPGAFileName.isEmpty())
		BitfileLineEdit->setText(FPGAFileName);
}

void MainForm::confFPGA()
{
	std::string sb = FPGAFileName.toStdString();
	FPGAStatusLineEdit->setText("Please wait...");
	FPGAStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);\n"));
	FPGAStatusLineEdit->show();
	QApplication::processEvents();

	if (sb != "")
	{
		PowerFEDown();

		if (hUSB->DownloadXilinx(sb.c_str()))
		{
			myUSBpix->WriteRegister(23, 0); //disable FE-Power
			EnVdda1CheckBox->setChecked(false);
			EnVdda2CheckBox->setChecked(false);
			EnVddd1CheckBox->setChecked(false);
			EnVddd2CheckBox->setChecked(false);
			refresh_DCS();
			writeCableLength();

			writeSaveStrb();
			FPGAStatusLineEdit->hide();
			FPGAStatusLineEdit->setText("done");
// 			myUSBpix->SetCurrentPhaseshift(0, chipID);
// 			PhaseShiftCount = 0;
// 			emit PhaseShiftCountSignal(PhaseShiftCount);
			CMDLV1CheckBox->setChecked(true);
			if (SelCmdCheckBox->isChecked())
				myUSBpix->setSelCMD(true);
			setScanType();
			DataRateComboBox->setCurrentIndex((int)1);
			writeDataRateMode();
			myUSBpix->WriteRegister(CS_DOMUX_CONTROL, 3); // Needed since DOMUX was implemented for BURN-IN card...
		}
		else
		{
			FPGAStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
			FPGAStatusLineEdit->setText("Error !!!");
		}
	}
	else 
	{
		FPGAStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		FPGAStatusLineEdit->setText("Unknown File");
	}
}

void MainForm::openBixFileDialog()
{
	QString oldfilename = BixfileLineEdit->text();
	ControllerFileName = QFileDialog::getOpenFileName(this,
		tr("Select Controller Configuration File"), oldfilename, tr("Bix Files (*.bix)"));

	if (!ControllerFileName.isEmpty())
		BixfileLineEdit->setText(ControllerFileName);
}

void MainForm::confController()
{
	std::string sb = ControllerFileName.toStdString();
	ControllerStatusLineEdit->setText("Please wait...");
	ControllerStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);\n"));
	ControllerStatusLineEdit->show();
	QApplication::processEvents();

	if (sb != "")
	{
		if(hUSB->LoadFirmwareFromFile(sb.c_str()))
			ControllerStatusLineEdit->hide();
		else
		{
			ControllerStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
			ControllerStatusLineEdit->setText("Error !!!");
		}
	}
	else 
	{
		ControllerStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		ControllerStatusLineEdit->setText("Unknown File");
	}
}

void MainForm::openHexFileDialog()
{
	QString oldfilename = HexfileLineEdit->text();
	EEPROMFileName = QFileDialog::getOpenFileName(this,
		tr("Select EEPROM Configuration File"), oldfilename, tr("Hex Files (*.hex)"));

	if (!EEPROMFileName.isEmpty())
		HexfileLineEdit->setText(EEPROMFileName);

}

void MainForm::flashControllerEEPROM()
{
	std::string sb = EEPROMFileName.toStdString();
	EEPROMStatusLineEdit->setText("Please wait...");
	EEPROMStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);\n"));
	EEPROMStatusLineEdit->show();
	QApplication::processEvents();
	if (sb != "")
	{
		if(hUSB->LoadHexFileToEeprom(sb.c_str()))
			EEPROMStatusLineEdit->hide();
		else
		{
			EEPROMStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
			EEPROMStatusLineEdit->setText("Error !!!");
		}
	}
	else 
	{
		EEPROMStatusLineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		EEPROMStatusLineEdit->setText("Unknown File");
	}
}

void MainForm::writeCableLength()
{
	myUSBpix->SetCableLengthReg(CableLengthSpinBox->value());
}

void MainForm::write_enableCMDLV1()
{
	if (CMDLV1CheckBox->isChecked())
	{
		L_LV1SpinBox->setDisabled(true);
		myUSBpix->enableCMDLV1();
	}
	else
	{
		L_LV1SpinBox->setDisabled(false);
		myUSBpix->disableCMDLV1();
	}
}

void MainForm::write_TriggerInput()
{
	switch(TriggerComboBox->currentIndex())
	{
	case 0:
		myUSBpix->setTriggerMode(0);
		break;
	case 1:
		myUSBpix->setTriggerMode(2);
		break;
	case 2:
		myUSBpix->setTriggerMode(1);
		break;
	}
}

void MainForm::setSlowControlWriteRegAB()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
	int data = 0x00;

	if (SlowContrWrABCheckBox->isChecked())
	{
		myUSBpix->SetSlowControlMode();

		SlowContrWrCCheckBox->disconnect();
		SlowContrWrCCheckBox->setChecked(false);
		connect(SlowContrWrCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegC()));

		SlowContrCheckBox->disconnect();
		SlowContrCheckBox->setChecked(false);
		connect(SlowContrCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControl()));

		SlowContrEfuseCheckBox->disconnect();
		SlowContrEfuseCheckBox->setChecked(false);
		connect(SlowContrEfuseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlEfuse()));
		IoMUX0CheckBox->hide();
		IoMUX2CheckBox->show();
		IoMUX3CheckBox->hide();

		if (dataRB & 0x08)
			data = dataRB;
		else
			data = dataRB + 0x08;

		if (dataRB & 0x10)
			data = data;
		else
			data = data + 0x10;

		if (dataRB & 0x20)
			data = data;
		else
			data = data + 0x20;

		writeBypassButton->setText("send bitstrem to AB");
		writeBypassButton->disconnect();
		connect(writeBypassButton, SIGNAL(clicked()), this, SLOT(sendBypassAB()));
		writeBypassButton->show();
	}
	else 
	{
		myUSBpix->SetCMDMode();

		IoMUX0CheckBox->show();
		IoMUX1CheckBox->show();
		IoMUX2CheckBox->show();
		IoMUX3CheckBox->show();

		if (dataRB & 0x08)
			data = dataRB - 0x08;
		else
			data = dataRB;

		if (dataRB & 0x10)
			data = data - 0x10;
		else
			data = data;

		if (dataRB & 0x20)
			data = data - 0x20;
		else
			data = data;

		writeBypassButton->hide();
	}
	myUSBpix->WriteRegister(CS_INMUX_CONTROL, data);
}

void MainForm::setSlowControlWriteRegC()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
	int data = 0x00;

	if (SlowContrWrCCheckBox->isChecked())
	{
		myUSBpix->SetSlowControlMode();

		SlowContrWrABCheckBox->disconnect();
		SlowContrWrABCheckBox->setChecked(false);
		connect(SlowContrWrABCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegAB()));

		SlowContrCheckBox->disconnect();
		SlowContrCheckBox->setChecked(false);
		connect(SlowContrCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControl()));

		SlowContrEfuseCheckBox->disconnect();
		SlowContrEfuseCheckBox->setChecked(false);
		connect(SlowContrEfuseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlEfuse()));
		IoMUX2CheckBox->hide();
		IoMUX3CheckBox->hide();
		IoMUX0CheckBox->show();

		if (dataRB & 0x08)
			data = dataRB - 0x08;
		else
			data = dataRB;

		if (dataRB & 0x10)
			data = data;
		else
			data = data + 0x10;

		if (dataRB & 0x20)
			data = data;
		else
			data = data + 0x20;

		writeBypassButton->setText("send bitstrem to C");
		writeBypassButton->disconnect();
		connect(writeBypassButton, SIGNAL(clicked()), this, SLOT(sendBypassC()));
		writeBypassButton->show();
	}
	else 
	{
		myUSBpix->SetCMDMode();

		IoMUX0CheckBox->show();
		IoMUX1CheckBox->show();
		IoMUX2CheckBox->show();
		IoMUX3CheckBox->show();

		if (dataRB & 0x08)
			data = dataRB - 0x08;
		else
			data = dataRB;

		if (dataRB & 0x10)
			data = data - 0x10;
		else
			data = data;

		if (dataRB & 0x20)
			data = data - 0x20;
		else
			data = data;
		writeBypassButton->hide();
	}
	myUSBpix->WriteRegister(CS_INMUX_CONTROL, data);
}

void MainForm::setSlowControl()
{
	
	if (SlowContrCheckBox->isChecked())
	{
		myUSBpix->SetSlowControlMode();

		SlowContrWrABCheckBox->disconnect();
		SlowContrWrABCheckBox->setChecked(false);
		connect(SlowContrWrABCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegAB()));

		SlowContrWrCCheckBox->disconnect();
		SlowContrWrCCheckBox->setChecked(false);
		connect(SlowContrWrCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegC()));

		SlowContrEfuseCheckBox->disconnect();
		SlowContrEfuseCheckBox->setChecked(false);
		connect(SlowContrEfuseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlEfuse()));

		writeBypassButton->hide();

	}
	else 
	{
		myUSBpix->SetCMDMode();

		IoMUX0CheckBox->show();
		IoMUX1CheckBox->show();
		IoMUX2CheckBox->show();
		IoMUX3CheckBox->show();

	}
}

void MainForm::setSlowControlEfuse()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
	int data = 0x00;

	if (SlowContrEfuseCheckBox->isChecked())
	{
		myUSBpix->SetSlowControlMode();

		SlowContrWrCCheckBox->disconnect();
		SlowContrWrCCheckBox->setChecked(false);
		connect(SlowContrWrCCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegC()));

		SlowContrCheckBox->disconnect();
		SlowContrCheckBox->setChecked(false);
		connect(SlowContrCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControl()));

		SlowContrWrABCheckBox->disconnect();
		SlowContrWrABCheckBox->setChecked(false);
		connect(SlowContrWrABCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setLowControlWriteRegAB()));


		IoMUX0CheckBox->show();
		IoMUX1CheckBox->show();
		IoMUX2CheckBox->hide();
		IoMUX3CheckBox->hide();

		if (dataRB & 0x08)
			data = dataRB - 0x08;
		else
			data = dataRB;

		if (dataRB & 0x10)
			data = data;
		else
			data = data + 0x10;

		if (dataRB & 0x20)
			data = data;
		else
			data = data + 0x20;

		writeBypassButton->setText("start clock");
		writeBypassButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		writeBypassButton->disconnect();
		connect(writeBypassButton, SIGNAL(clicked()), this, SLOT(startEfuseClock()));
		writeBypassButton->show();
	}
	else 
	{
		myUSBpix->SetCMDMode();

		IoMUX0CheckBox->show();
		IoMUX1CheckBox->show();
		IoMUX2CheckBox->show();
		IoMUX3CheckBox->show();

		if (dataRB & 0x08)
			data = dataRB - 0x08;
		else
			data = dataRB;

		if (dataRB & 0x10)
			data = data - 0x10;
		else
			data = data;

		if (dataRB & 0x20)
			data = data - 0x20;
		else
			data = data;

		writeBypassButton->hide();
	}
	myUSBpix->WriteRegister(CS_INMUX_CONTROL, data);
}

void MainForm::startEfuseClock() {

	disconnect(writeBypassButton, SIGNAL(clicked()), this, SLOT(startEfuseClock()));
	connect(writeBypassButton, SIGNAL(clicked()), this, SLOT(stopEfuseClock()));
	writeBypassButton->setText("stop clock");
	writeBypassButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	QApplication::processEvents();

	run_clk = true;
	
	int n = 0;
	int off = 0x00;
	int on = 0x02;
	while(run_clk)
	{
		UPGen::Sleep(1);

		if (n%2==0) {
			myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, off);	
		}
		else {
			myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, on);
		}

		QApplication::processEvents();
		n++;
	}

	myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, off);
	disconnect(writeBypassButton, SIGNAL(clicked()), this, SLOT(stopEfuseClock()));
	connect(writeBypassButton, SIGNAL(clicked()), this, SLOT(startEfuseClock()));
	writeBypassButton->setText("start clock");
	writeBypassButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));

}

void MainForm::stopEfuseClock()
{
	run_clk = false;
}
void MainForm::sendBypassAB()
{
	myUSBpix->sendBitstreamToAB(chipID);
}

void MainForm::sendBypassC()
{
	myUSBpix->sendBitstreamToC(chipID);
}

void MainForm::writeDataRateMode()
{
	//if (DataRateComboBox->currentIndex() == 0)
	//	myUSBpix->disable_160Mbps_data_rate();
	//else if (DataRateComboBox->currentIndex() == 1)
	//	myUSBpix->enable_160Mbps_data_rate();
}

void MainForm::incrSyncCLKPhase()
{
	myUSBpix->incr_phase_shift(chipID);
	PhaseShiftCount++;
	emit PhaseShiftCountSignal(PhaseShiftCount * 69);

	if (myUSBpix->check_phase_shift_overflow(chipID))
	{
		PSOverflowLineEdit->setText("PSOVERFLOW");
	}
	else
	{
		PSOverflowLineEdit->setText("DONE");
	}
}

void MainForm::decrSyncCLKPhase()
{
	myUSBpix->decr_phase_shift(chipID);
	PhaseShiftCount--;
	emit PhaseShiftCountSignal(PhaseShiftCount * 69);

	if (myUSBpix->check_phase_shift_overflow(chipID))
	{
		PSOverflowLineEdit->setText("PSOVERFLOW");
	}
	else
	{
		PSOverflowLineEdit->setText("DONE");
	}
}

void MainForm::StorePatternChanged()
{
	if(UseStorePatternModeCheckBox->isChecked())
		myUSBpix->StoreSyncCheckPattern(chipID);
	else
		myUSBpix->ResetSyncCheckPattern(chipID);
}

void MainForm::StartSyncPhaseCheck()
{
	double result = 0;
	double min_error_rate = 1;
	PSOverflowLineEdit->setText("Testing...");
	QApplication::processEvents();
	min_error_rate = MinErrorRateLineEdit->text().toDouble();
	result = myUSBpix->StartSyncCheck(min_error_rate, chipID);

	emit SyncCheckResultSignal(result);
	PSOverflowLineEdit->setText("Done");
}

void MainForm::StartSyncScan()
{
	myUSBpix->WriteGlobal(chipID);
	double phase_shift[201];
	double ErrorRate[201];
	double min_error_rate = 1;
	min_error_rate = MinErrorRateLineEdit->text().toDouble();
	PSOverflowLineEdit->setText("Scanning...");
	StartSyncScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	StartSyncScanButton->setText("wait");
	QApplication::processEvents();

	for (int i = 0; i < 201; i++)
	{
		phase_shift[i] = 0;
		ErrorRate[i] = 0;
	}

	if (myUSBpix->StartSyncScan(min_error_rate, chipID)) // returns true if succeeded
		StartSyncScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	else
		StartSyncScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	StartSyncScanButton->setText("start sync scan");

	PhaseShiftCount = myUSBpix->GetCurrentPhaseshift(chipID);
	emit PhaseShiftCountSignal(PhaseShiftCount * 69);
	StartSyncPhaseCheck();

	myUSBpix->GetSyncScanResult(phase_shift, ErrorRate, 201, chipID);

	for (int i = 0; i < 201; i++)
		phase_shift[i] = phase_shift[i] * 69;

	PSOverflowLineEdit->setText("Done");

	SyncScanPlot->detachItems();
	SyncScanCurve = new QwtPlotCurve("SyncScanCurve");
	QwtSymbol sym;
	sym.setStyle(QwtSymbol::XCross);
	sym.setPen(QColor(Qt::black));
	sym.setSize(10);

	SyncScanCurve->setSymbol(&sym);
	SyncScanCurve->setStyle(QwtPlotCurve::NoCurve);
	SyncScanCurve->setRawSamples(phase_shift, ErrorRate, 201);
	SyncScanCurve->attach(SyncScanPlot);



	/// try log yAxis !!!!!!!!!!!!!!!!!!
	SyncScanPlot->setAxisScale(QwtPlot::yLeft, min_error_rate, 0.5, 0);
	//SyncScanPlot->setAxisAutoScale(QwtPlot::yLeft);
	SyncScanPlot->setAxisAutoScale(QwtPlot::xBottom);
	//SyncScanPlot->setAxisScale(QwtPlot::xBottom, -4500, 4500, 0);
	SyncScanPlot->replot();		
}

void MainForm::printSyncScanPlot()
{
	SyncScanPlot->Print();
}

void MainForm::resetAll()
{
	myUSBpix->ResetAll();
}

void MainForm::setRunMode()
{
	myUSBpix->SetRunMode();
}

void MainForm::setTLUMode()
{
	myUSBpix->SetTLUMode();
}

void MainForm::setCalMode()
{
	myUSBpix->SetCalibrationMode();
}
void MainForm::setToTMode()
{
	myUSBpix->SetTOTMode();
}

void MainForm::stopXCK()
{
	myUSBpix->stopXCK(true);

	disconnect(stopXCKButton, SIGNAL(clicked()), this, SLOT(stopXCK()));
	connect(stopXCKButton, SIGNAL(clicked()), this, SLOT(runXCK()));
	stopXCKButton->setText("run XCK");
	stopXCKButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));

}

void MainForm::runXCK()
{
	myUSBpix->stopXCK(false);

	disconnect(stopXCKButton, SIGNAL(clicked()), this, SLOT(runXCK()));
	connect(stopXCKButton, SIGNAL(clicked()), this, SLOT(stopXCK()));
	stopXCKButton->setText("stop XCK");
	stopXCKButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
}

void MainForm::setFEFlavor(const QString flavor)
{
	if(hUSB!=0)
	{
		PowerFEDown();
	}
	if (myUSBpix != 0)
		delete myUSBpix;
	if(flavor == "FE-I4B")
	{
		isFEI4B = true;
		DisVbn_CPPMSpinBox->hide();
		label_133->hide();
		VthinSpinBox->hide();
		label_79->hide();
	}
	else
	{
		isFEI4B = false;
		DisVbn_CPPMSpinBox->show();
		label_133->show();
		VthinSpinBox->show();
		label_79->show();
	}
	myUSBpix = new USBpix(chipID, 0, hUSB, isFEI4B);
	
	VsetA1Box->setValue(isFEI4B?dcsInst->PSU[VDDD1]->CalData.DefaultVoltage:dcsInst->PSU[VDDA1]->CalData.DefaultVoltage);
	VsetA2Box->setValue(isFEI4B?dcsInst->PSU[VDDD2]->CalData.DefaultVoltage:dcsInst->PSU[VDDA2]->CalData.DefaultVoltage);
	VsetD1Box->setValue(isFEI4B?dcsInst->PSU[VDDA1]->CalData.DefaultVoltage:dcsInst->PSU[VDDD1]->CalData.DefaultVoltage);
	VsetD2Box->setValue(isFEI4B?dcsInst->PSU[VDDA2]->CalData.DefaultVoltage:dcsInst->PSU[VDDD2]->CalData.DefaultVoltage);
}

void MainForm::writeInvertXck()
{
	if (InvertXckCheckBox->isChecked())
		myUSBpix->WriteRegister(CS_XCK_PHASE_CTRL, 0x01);
	else 
		myUSBpix->WriteRegister(CS_XCK_PHASE_CTRL, 0x00);
}

void MainForm::SetINMUXselectMode()
{
	int dataRB;
	int data = 0;
	switch (INMUXselectComboBox->currentIndex())
	{
	case 0:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x00 + (dataRB << 3);
		break;
	case 1:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x01 + (dataRB << 3);
		break;
	case 2:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x02 + (dataRB << 3);
		break;
	case 3:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x03 + (dataRB << 3);
		break;
	case 4:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x04 + (dataRB << 3);
		break;
	case 5:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x05 + (dataRB << 3);
		break;
	case 6:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x06 + (dataRB << 3);
		break;
	case 7:
		dataRB = myUSBpix->ReadRegister(CS_INMUX_CONTROL);
		dataRB = dataRB >> 3;
		data = 0x07 + (dataRB << 3);
		break;
	}
	myUSBpix->WriteRegister(CS_INMUX_CONTROL, data);

}

void MainForm::ChangeAdapterFlavor()
{
	if (AdapterFlavorComboBox->currentIndex() == 0)
	{
		delete dcsInst;
		dcsInst = new USBPixSTDDCS(hUSB);
		myUSBpix->WriteRegister(CS_DOMUX_CONTROL, (int)3);
		DOMUXSpinBox->setValue(4);
		DOMUXSpinBox->setEnabled(false);
	}
	else
	{
		delete dcsInst;
		dcsInst = new USBPixBIDCS(hUSB);
		myUSBpix->WriteRegister(CS_DOMUX_CONTROL, (int)7);
		DOMUXSpinBox->setEnabled(true);
	}
}

void MainForm::SetFEToRead()
{
	int FEToRead = DOMUXSpinBox->value() - 1;
	FEToRead = 0x04 + (FEToRead&0x03);
	myUSBpix->WriteRegister(CS_DOMUX_CONTROL, FEToRead);
}

void MainForm::writeSelAltBus()
{
	if (SelAltBusCheckBox->isChecked())
		myUSBpix->setSelAltBus(true);
	else
		myUSBpix->setSelAltBus(false);
}

void MainForm::writeSelCMD()
{
	if (SelCmdCheckBox->isChecked())
		myUSBpix->setSelCMD(true);
	else
		myUSBpix->setSelCMD(false);
}

void MainForm::SetIoMUX0()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_IN_CTRL); 
	int data = 0x00;

	if (IoMUX0CheckBox->isChecked())
	{
		if (dataRB & 0x01)
			data = dataRB;
		else
			data = dataRB + 0x01;
	}
	else
	{
		if (dataRB & 0x01)
			data = dataRB - 0x01;
		else
			data = dataRB;
	}
	myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, data);
}

void MainForm::SetIoMUX1()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_IN_CTRL); 
	int data = 0x00;

	if (IoMUX1CheckBox->isChecked())
	{
		if (dataRB & 0x02)
			data = dataRB;
		else
			data = dataRB + 0x02;
	}
	else
	{
		if (dataRB & 0x02)
			data = dataRB - 0x02;
		else
			data = dataRB;
	}
	myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, data);
}

void MainForm::SetIoMUX2()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_IN_CTRL); 
	int data = 0x00;

	if (IoMUX2CheckBox->isChecked())
	{
		if (dataRB & 0x04)
			data = dataRB;
		else
			data = dataRB + 0x04;
	}
	else
	{
		if (dataRB & 0x04)
			data = dataRB - 0x04;
		else
			data = dataRB;
	}
	myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, data);
}

void MainForm::SetIoMUX3()
{
	int dataRB = myUSBpix->ReadRegister(CS_INMUX_IN_CTRL); 
	int data = 0x00;

	if (IoMUX3CheckBox->isChecked())
	{
		if (dataRB & 0x08)
			data = dataRB;
		else
			data = dataRB + 0x08;
	}
	else
	{
		if (dataRB & 0x08)
			data = dataRB - 0x08;
		else
			data = dataRB;
	}
	myUSBpix->WriteRegister(CS_INMUX_IN_CTRL, data);
}

void MainForm::write_GobalReset()
{
	myUSBpix->WriteCommand(FE_GLOBAL_RESET, chipID);
}

void MainForm::sendStrbCommand()
{
	myUSBpix->WriteCommand(FE_CAL, chipID);
}

void MainForm::sendTriggerCommand()
{
	myUSBpix->WriteCommand(FE_LV1_TRIGGER, chipID);
}

void MainForm::sendEnDataTake()
{
	myUSBpix->WriteCommand(FE_EN_DATA_TAKE, chipID);
}

void MainForm::sendEnConfMode()
{
	myUSBpix->WriteCommand(FE_CONF_MODE, chipID);
}

void MainForm::sendGlobalPulse()
{
	myUSBpix->WriteCommand(FE_GLOBAL_PULSE, chipID);
}

void MainForm::readEPROM()
{
	myUSBpix->ReadEPROMvalues(chipID);
}

void MainForm::burnEPROM()
{
	myUSBpix->BurnEPROMvalues();
}

void MainForm::enableHardRstA()
{
	HardRstAButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	disconnect(HardRstAButton, SIGNAL(clicked()), this, SLOT(enableHardRstA()));
	connect(HardRstAButton, SIGNAL(clicked()), this, SLOT(disableHardRstA()));

	int dataRB = myUSBpix->ReadRegister(CS_HARD_RST_CONTROL);
	int data = 0;
	if (dataRB & 0x01)
		data = dataRB;
	else
		data = dataRB + 0x01;
	myUSBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
}

void MainForm::disableHardRstA()
{
	HardRstAButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	disconnect(HardRstAButton, SIGNAL(clicked()), this, SLOT(disableHardRstA()));
	connect(HardRstAButton, SIGNAL(clicked()), this, SLOT(enableHardRstA()));

	int dataRB = myUSBpix->ReadRegister(CS_HARD_RST_CONTROL);
	int data = 0;
	if (dataRB & 0x01)
		data = dataRB - 0x01;
	else
		data = dataRB;
	myUSBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
}

void MainForm::enableHardRstD1()
{
	HardRstD1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	disconnect(HardRstD1Button, SIGNAL(clicked()), this, SLOT(enableHardRstD1()));
	connect(HardRstD1Button, SIGNAL(clicked()), this, SLOT(disableHardRstD1()));

	int dataRB = myUSBpix->ReadRegister(CS_HARD_RST_CONTROL);
	int data = 0;
	if (dataRB & 0x02)
		data = dataRB;
	else
		data = dataRB + 0x02;
	myUSBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
}

void MainForm::disableHardRstD1()
{
	HardRstD1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	disconnect(HardRstD1Button, SIGNAL(clicked()), this, SLOT(disableHardRstD1()));
	connect(HardRstD1Button, SIGNAL(clicked()), this, SLOT(enableHardRstD1()));

	int dataRB = myUSBpix->ReadRegister(CS_HARD_RST_CONTROL);
	int data = 0;
	if (dataRB & 0x02)
		data = dataRB - 0x02;
	else
		data = dataRB;
	myUSBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
}

void MainForm::enableHardRstD2()
{
	HardRstD2Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	disconnect(HardRstD2Button, SIGNAL(clicked()), this, SLOT(enableHardRstD2()));
	connect(HardRstD2Button, SIGNAL(clicked()), this, SLOT(disableHardRstD2()));

	int dataRB = myUSBpix->ReadRegister(CS_HARD_RST_CONTROL);
	int data = 0;
	if (dataRB & 0x04)
		data = dataRB;
	else
		data = dataRB + 0x04;
	myUSBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
}

void MainForm::disableHardRstD2()
{
	HardRstD2Button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	disconnect(HardRstD2Button, SIGNAL(clicked()), this, SLOT(disableHardRstD2()));
	connect(HardRstD2Button, SIGNAL(clicked()), this, SLOT(enableHardRstD2()));

	int dataRB = myUSBpix->ReadRegister(50);
	int data = 0;
	if (dataRB & 0x04)
		data = dataRB - 0x04;
	else
		data = dataRB;
	myUSBpix->WriteRegister(CS_HARD_RST_CONTROL, data);
}

void MainForm::LoadRegABDacLd()
{
	int dataRB = myUSBpix->ReadRegister(CS_CMOS_LINES);
	int data = 0;

	if (RegABDacLdCheckBox->isChecked())
	{
		if (dataRB & 0x01)
			data = dataRB;
		else
			data = dataRB + 0x01;
	}
	else
	{
		if (dataRB & 0x01)
			data = dataRB - 0x01;
		else
			data = dataRB;
	}
	myUSBpix->WriteRegister(CS_CMOS_LINES, data);
}

void MainForm::LoadRegABStbLd()
{
	int dataRB = myUSBpix->ReadRegister(CS_CMOS_LINES);
	int data = 0;

	if (RegABStbLdCheckBox->isChecked())
	{
		if (dataRB & 0x02)
			data = dataRB;
		else
			data = dataRB + 0x02;
	}
	else
	{
		if (dataRB & 0x02)
			data = dataRB - 0x02;
		else
			data = dataRB;
	}
	myUSBpix->WriteRegister(CS_CMOS_LINES, data);
}

void MainForm::clear_SRAM()
{
	myUSBpix->ClearSRAM(chipID);
}

void MainForm::write_SRAM()
{
	int startadd = 0;
	int bytenr = 0;
	startadd = writeSRAMstartaddSpinBox->value();
	bytenr = writeSRAMbytenrSpinBox->value();

	myUSBpix->WriteSRAM(startadd, bytenr, chipID);
}

void MainForm::read_SRAM()
{
	int startadd = 0;
	int bytenr = 0;
	//int nr_hits = 0;
	bool CalMode = 0;
	bool TOTMode = 0;

	myUSBpix->GetSystemMode(CalMode, TOTMode);

	startadd = readSRAMstartaddSpinBox->value();
	bytenr = readSRAMbytenrSpinBox->value();

	if (bytenr == 0)
	{
		if (TOTMode == true && CalMode == false) // ToT mode
			myUSBpix->ReadSRAM(chipID);
		else if (CalMode == true && TOTMode == false) // cal mode
			myUSBpix->ReadSRAM(0, chipID);
		else // run mode
			myUSBpix->ReadSRAM(chipID);
	}
	else
		myUSBpix->ReadSRAM(startadd, bytenr);

	
	if ((CalMode == true) && (TOTMode == false)) // calib mode
	{
		if (getenv("USBPIXI4DATA") != NULL)
		{
			QString filename;
			filename = QString(getenv("USBPIXI4DATA"))+"ConfHisto.bin";
			myUSBpix->WriteConfHisto(qPrintable(filename), chipID);
		}
		else
			myUSBpix->WriteConfHisto("ConfHisto.bin", chipID);
	}
	else if ((TOTMode == true) && (CalMode == false)) // ToT mode
	{
		if (getenv("USBPIXI4DATA") != NULL)
		{
			QString filename;
			filename = QString(getenv("USBPIXI4DATA"))+"/ToTHisto.bin";
			myUSBpix->WriteToTHisto(qPrintable(filename), chipID);
		}
		else
			myUSBpix->WriteToTHisto("ToTHisto.bin", chipID);
	}
	else // run mode
	{
		if (getenv("USBPIXI4DATA") != NULL)
		{
			QString filename;
			filename = QString(getenv("USBPIXI4DATA"))+"/dataRB.dat";
			myUSBpix->FileSaveRB(qPrintable(filename), bytenr, false, chipID);
		}
		else
			myUSBpix->FileSaveRB("dataRB.dat", SRAM_WORDSIZE, false, chipID);
	}
}

void MainForm::reset_SRAMCounter()
{
	myUSBpix->ResetSRAMCounter(chipID);
}

void MainForm::restartDataTake()
{
	myUSBpix->ClearSRAM(chipID);
	myUSBpix->resetRunModeAdd();
}


void MainForm::PowerFEUp()
{
	EnVddd1CheckBox->setChecked(true);
	UPGen::Sleep(10);
	EnVdda1CheckBox->setChecked(true);
	UPGen::Sleep(10);
	EnVddd2CheckBox->setChecked(true);
	UPGen::Sleep(10);
	EnVdda2CheckBox->setChecked(true);
	UPGen::Sleep(10);
	disconnect(PowerFEButton, SIGNAL(clicked()), this, SLOT(PowerFEUp()));
	connect(PowerFEButton, SIGNAL(clicked()), this, SLOT(PowerFEDown()));
	PowerFEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	PowerFEButton->setText("Power Down FE");
	refresh_DCS();
	QApplication::processEvents();
}

void MainForm::PowerFEDown()
{

	EnVdda2CheckBox->setChecked(false);
	UPGen::Sleep(10);
	EnVddd2CheckBox->setChecked(false);
	UPGen::Sleep(10);
	EnVdda1CheckBox->setChecked(false);
	UPGen::Sleep(10);
	EnVddd1CheckBox->setChecked(false);
	UPGen::Sleep(10);
	disconnect(PowerFEButton, SIGNAL(clicked()), this, SLOT(PowerFEDown()));
	connect(PowerFEButton, SIGNAL(clicked()), this, SLOT(PowerFEUp()));
	PowerFEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	PowerFEButton->setText("Power Up FE");
	refresh_DCS();
	QApplication::processEvents();
}



void MainForm::setVDDA1()
{
	if (EnVdda1CheckBox->isChecked())
		dcsInst->PSU[VDDA1]->Enable(true);
	else
		dcsInst->PSU[VDDA1]->Enable(false);
}

void MainForm::setVDDA2()
{
	if (EnVdda2CheckBox->isChecked())
		dcsInst->PSU[VDDA2]->Enable(true);
	else
		dcsInst->PSU[VDDA2]->Enable(false);
}
void MainForm::setVDDD1()
{	
	if (EnVddd1CheckBox->isChecked())
		dcsInst->PSU[VDDD1]->Enable(true);
	else
		dcsInst->PSU[VDDD1]->Enable(false);
}

void MainForm::setVDDD2()
{
	if (EnVddd2CheckBox->isChecked())
		dcsInst->PSU[VDDD2]->Enable(true);
	else
		dcsInst->PSU[VDDD2]->Enable(false);
}

void MainForm::setVDDPLL()
{
	/*	int dataRB = 0;
	dataRB = myUSBpix->ReadRegister(CS_POWER_CONTROL);
	if (dataRB & 0x04)
	{
	if (EnVddpllCheckBox->isChecked())
	dataRB = dataRB;
	else
	dataRB = dataRB - 0x04;
	}
	else
	{
	if (EnVddpllCheckBox->isChecked())
	dataRB = dataRB + 0x04;
	else
	dataRB = dataRB;
	}
	myUSBpix->WriteRegister(CS_POWER_CONTROL, dataRB);
	*/
}

///////////////////          Strobe and LV1         //////////////////////
void MainForm::writeSaveStrb()
{	
	unsigned char reg_data[7];

	//-------write length of Strobe----------
	RegisterValue   = L_StrobeSpinBox->text();
	reg_data[0] = (0xff & RegisterValue.toInt());
	reg_data[1] = (0xff & RegisterValue.toInt()) >> 8;

	//-------write length of LV1------------
	RegisterValue   = L_LV1SpinBox->text();
	reg_data[2]= 0xff & RegisterValue.toInt();

	//--------write delay of LV1------------
	RegisterValue   = D_LV1SpinBox->text();
	reg_data[3]= RegisterValue.toInt();

	//-----write quantity of LV1 and Strobe--
	RegisterValue   = QuantitySpinBox->text();
	reg_data[4]= 0xff & RegisterValue.toInt();

	//--------write frequency---------------
	RegisterValue   = FreqSpinBox->text();
	reg_data[5]= (0x00ff & RegisterValue.toInt());
	reg_data[6]= (0xff00 & RegisterValue.toInt()) >> 8;

	myUSBpix->WriteStrbSave(reg_data);
}	

void MainForm::writeStartStrb()
{
	writeSaveStrb();
	myUSBpix->WriteStrbStart();	
}

void MainForm::writeStopStrb()
{
	myUSBpix->WriteStrbStop();	
}

void MainForm::readRegister()
{
	int cs = CSSpinBox->value();
	int t = myUSBpix->ReadRegister(cs);
	emit register_data(t);
}
void MainForm::startMeasurement()
{
	CounterSettingComboBox->setDisabled(true);
	disconnect(StartMeasurementButton, SIGNAL(clicked()), this, SLOT(startMeasurement()));
	connect(StartMeasurementButton, SIGNAL(clicked()), this, SLOT(cancelMeasurement()));
	StartMeasurementButton->setText("cancel Measurement");
	StartMeasurementButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	QApplication::processEvents();

	myUSBpix->SetNumberOfEvents(NrEventsSpinBox->value());
	setEventCounterMode();
	myUSBpix->ClearSRAM(chipID);
	myUSBpix->ClearConfHisto(chipID);
	myUSBpix->ClearTOTHisto(chipID);
	//	myUSBpix->ClearHitHistoFromRawData(chipID);
	myUSBpix->ClearHitLV1HistoFromRawData(chipID);



//	myUSBpix->FileSaveRB("data.dat", 0, SRAM_WORDSIZE, false);
//	myUSBpix->CheckDataConsisty("error.log", false, false);
	myUSBpix->resetRunModeAdd();
	myUSBpix->StartMeasurement();

	ToTSimplePlot->detachItems();
	LV1SimplePlot->detachItems();

	bool MeasRunning = true;
	bool SRAMFull = false;
	bool newFile = true;
	int SRAMFillingLevelValue = 0;
	int CollectedEvents = 0;
	int TriggerRate = 0;
	int EventRate = 0;

	while((hUSB->HandlePresent()) & MeasRunning)
	{
		myUSBpix->GetSourceScanStatus(SRAMFull, MeasRunning, SRAMFillingLevelValue, CollectedEvents, TriggerRate, EventRate, chipID);
		emit SRAMFillingLevel(SRAMFillingLevelValue);
		emit CountedEvents(CollectedEvents);
		emit TriggerRateSignal(TriggerRate/1000);
		if (SRAMFull)
		{
			myUSBpix->ReadSRAM(chipID);
			myUSBpix->WriteFileFromRawData("data.raw", chipID, newFile, false);
			newFile = false;
//			myUSBpix->FileSaveRB("data.dat", 0, SRAM_WORDSIZE, true);
//			myUSBpix->CheckDataConsisty("error.log", true, false);
			restartDataTake();
			FillToTSimplePlot();
			FillLV1SimplePlot();
		}
		QApplication::processEvents();
	}

	myUSBpix->ReadSRAM(chipID);
	myUSBpix->WriteFileFromRawData("data.raw", chipID, newFile, true);
	FillToTSimplePlot();
	FillLV1SimplePlot();
//	myUSBpix->FileSaveRB("data.dat", 0, SRAM_WORDSIZE, true);
//	myUSBpix->CheckDataConsisty("error.log", true, true);
//	myUSBpix->FileSaveRBHitHisto("HitHisto.bin");
//	myUSBpix->FileSaveRBToTHisto("ToTHisto.bin");

	CounterSettingComboBox->setDisabled(false);
	disconnect(StartMeasurementButton, SIGNAL(clicked()), this, SLOT(cancelMeasurement()));
	connect(StartMeasurementButton, SIGNAL(clicked()), this, SLOT(startMeasurement()));
	StartMeasurementButton->setText("start Measurement");
	StartMeasurementButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
}

void MainForm::FillToTSimplePlot()
{
	ToTSimplePlot->detachItems();

	double y_val[16];
	double x_val[16];
	for (int i = 0; i < 16; i++)
	{
		y_val[i] = 0;
		x_val[i] = i;
	}

	for (int col = 0; col < 80; col++)
	{
		for (int row = 0; row < 336; row++)
		{
			for (int i = 0; i < 16; i++)
			{
				int newval = 0;
				myUSBpix->GetTOTHisto(col, row, i, newval, chipID);
				y_val[i] = y_val[i] + newval;
			}
		}
	}

	QwtPlotCurve * ToTCurve = new QwtPlotCurve("ToT");
	QwtSymbol sym;
	sym.setStyle(QwtSymbol::Cross);
	sym.setPen(QColor(Qt::black));
	sym.setSize(5);
	ToTCurve->setSymbol(&sym);
	ToTCurve->setStyle(QwtPlotCurve::NoCurve);
	ToTCurve->setRawSamples(x_val, y_val, 16);
	ToTCurve->attach(ToTSimplePlot);
	ToTSimplePlot->setAxisScale(QwtPlot::xBottom, 0, 16, 4);
	ToTSimplePlot->setAxisAutoScale(QwtPlot::yLeft);
	ToTSimplePlot->replot();
}

void MainForm::FillLV1SimplePlot()
{
	LV1SimplePlot->detachItems();

	double y_val[16];
	double x_val[16];
	for (int i = 0; i < 16; i++)
	{
		y_val[i] = 0;
		x_val[i] = i;
	}


	for (int i = 0; i < 16; i++)
	{
		int newval = 0;
		myUSBpix->GetHitLV1HistoFromRawData(i, newval, chipID);
		y_val[i] = newval;
	}

	QwtPlotCurve * LV1Curve = new QwtPlotCurve("LV1");
	QwtSymbol sym;
	sym.setStyle(QwtSymbol::Cross);
	sym.setPen(QColor(Qt::black));
	sym.setSize(5);
	LV1Curve->setSymbol(&sym);
	LV1Curve->setStyle(QwtPlotCurve::NoCurve);
	LV1Curve->setRawSamples(x_val, y_val, 16);
	LV1Curve->attach(LV1SimplePlot);
	LV1SimplePlot->setAxisScale(QwtPlot::xBottom, 0, 16, 4);
	LV1SimplePlot->setAxisAutoScale(QwtPlot::yLeft);
	LV1SimplePlot->replot();

}

void MainForm::setEventCounterMode()
{
	int mode = 0;
	mode = CounterSettingComboBox->currentIndex();
	if (mode == 0)
		myUSBpix->SetMeasurementMode(FPGA_COUNT_LV1);
	if (mode == 1)
		myUSBpix->SetMeasurementMode(FPGA_COUNT_DR);
	if (mode == 2)
		myUSBpix->SetMeasurementMode(FPGA_COUNT_DH);
}

void MainForm::cancelMeasurement()
{
	myUSBpix->StopMeasurement();
}

void MainForm::startCalibrationTest()
{
	StartCalibrationTestButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
	ofstream fout;
	fout.open("calibration_test.test");
	setCalMode();
	fout << "Vthin" << "\t" << "<Rate>[Hz]" << "\t" << "sigma[Hz]" << endl; 
	for (int vthin = VthinStartSpinBox->value(); vthin <= VthinStopSpinBox->value(); vthin++)
	{
		myUSBpix->SetGlobalVal(VTHIN, vthin, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(VTHIN), chipID);
		
		int repetitions = RateRepetitionsSpinBox->value();
		double meanrate = 0;
		double std_dev = 0;
		bool SRAMFull = false;
		bool MeasRunning = false;
		int SRAMFillingLevel = 0;
		int CollectedEvents = 0;
		int currentrate[255] = {0};
		int currenteventrate[255] = {0};

		for (int rep = 0; rep < repetitions; rep++)
		{
			myUSBpix->GetSourceScanStatus(SRAMFull, MeasRunning, SRAMFillingLevel, CollectedEvents, currentrate[rep], currenteventrate[rep], chipID);
			UPGen::Sleep(1200);
			emit TriggerRateSignal(currentrate[rep]/1000);
			QApplication::processEvents();
			meanrate = meanrate + currentrate[rep];
		}
		meanrate = meanrate / repetitions;
		
		// calc std dev.
		for (int i = 0; i < repetitions; i++)
			std_dev = (currentrate[i] - meanrate)*(currentrate[i] - meanrate);
		std_dev = std_dev / repetitions;
		std_dev = sqrt(std_dev);
		
		// do threshold scan
		//ScanComboBox->setCurrentIndex(2);	//set threshold scan
		//scanFileNameLineEdit->setText(".CalTestData/threshold_vthin" + QString::number(vthin));				// set filename
		//ExtLV1CheckBox->setChecked(false);
		//start_Scan();
		
		// calc mean threshold
		//int nr_good_fits = 0;
		//double mean_threshold = 0;
		//double std_dev_threshold = 0;
		//for (int i = 0; i < thresvector.size(); i++)
		//{
		//	if (thresvector[i] != 0)
		//	{
		//		mean_threshold = mean_threshold + thresvector[i];
		//		nr_good_fits++;
		//	}
		//}
		//mean_threshold = mean_threshold / nr_good_fits;

		//// calc std dev. of thresholda
		//for (int i = 0; i < thresvector.size(); i++)
		//{
		//	if (thresvector[i] != 0)
		//		std_dev_threshold = (thresvector[i] - mean_threshold)*(thresvector[i] - mean_threshold);
		//}
		//std_dev_threshold = std_dev_threshold / nr_good_fits;
		//std_dev_threshold = sqrt(std_dev_threshold);

		fout << vthin << "\t" /*<< mean_threshold << "\t" << std_dev_threshold << "\t" << "\t"*/ << meanrate << "\t" << std_dev << endl;

		// reset hitbus and enable masks
		//write_HITBUSFile();
		//write_ENABLEFile();
		//setCalMode();
		//ExtLV1CheckBox->setChecked(true);
	}
	fout.close();
	StartCalibrationTestButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
}

void MainForm::startWaferProbingLoop()
{
	WaferLoopProgressBar->show();
	WaferLoopProgressBar->setMaximum(100);
	WaferLoopProgressBar->setValue(0);
	QApplication::processEvents();
	// create new folder
#ifdef CF__LINUX
	mkdir((char*)(("./data/" + WaferNameLineEdit->text()).toStdString().c_str()),755);
#else
	_mkdir((char*)(("./data/" + WaferNameLineEdit->text()).toStdString().c_str()));
#endif
	QString path = "./data/" + WaferNameLineEdit->text() + "/chip_" + QString::number(WaferChipNrSpinBox->value());
#ifdef CF__LINUX
	mkdir((char*)path.toStdString().c_str(),755);
#else
	_mkdir((char*)path.toStdString().c_str());
#endif
	
	// read DCS unconfigured
	dcsInst->PSU[VDDD1]->Enable(true);
	QString filename = path + "/DCS_VDDD1.dcs";
	WriteDCStoFile(filename);
	dcsInst->PSU[VDDA1]->Enable(true);
	filename = path + "/DCS_VDDA1.dcs";
	WriteDCStoFile(filename);
	dcsInst->PSU[VDDD2]->Enable(true);
	filename = path + "/DCS_VDDD2.dcs";
	WriteDCStoFile(filename);
	dcsInst->PSU[VDDA2]->Enable(true);
	filename = path + "/DCS_VDDA2.dcs";
	WriteDCStoFile(filename);
	WaferLoopProgressBar->setValue(1);
	QApplication::processEvents();

	// configure ref_val
	read_GlobalFile("../config/ref_val.dat");
	filename = path + "/ref_val.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_ref_val.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/ref_val.dat");
		QString filenameRB = path + "/ref_valRB_" + QString::number(test) + ".dat";
		readGlobalReg();
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(2);
	QApplication::processEvents();

	// configure zeros
	read_GlobalFile("../config/zeros.dat");
	filename = path + "/zeros.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_zeros.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/zeros.dat");
		QString filenameRB = path + "/zerosRB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(3);
	QApplication::processEvents();

	// configure onesA
	read_GlobalFile("../config/onesA.dat");
	filename = path + "/onesA.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_onesA.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/onesA.dat");
		QString filenameRB = path + "/onesARB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(4);
	QApplication::processEvents();

	// configure onesB
	read_GlobalFile("../config/onesB.dat");
	filename = path + "/onesB.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_onesB.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/onesB.dat");
		QString filenameRB = path + "/onesBRB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(5);
	QApplication::processEvents();

	// configure onesC
	read_GlobalFile("../config/onesC.dat");
	filename = path + "/onesC.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_onesC.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/onesC.dat");
		QString filenameRB = path + "/onesCRB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(6);
	QApplication::processEvents();
	
	// configure onesD
	read_GlobalFile("../config/onesD.dat");
	filename = path + "/onesD.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_onesD.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/onesD.dat");
		QString filenameRB = path + "/onesDRB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(7);
	QApplication::processEvents();

	// configure onesE
	read_GlobalFile("../config/onesE.dat");
	filename = path + "/onesE.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_onesE.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/onesE.dat");
		QString filenameRB = path + "/onesERB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(8);
	QApplication::processEvents();

	// configure onesF
	read_GlobalFile("../config/onesF.dat");
	filename = path + "/onesF.dat";
	save_GlobalFile(filename.toStdString());
	filename = path + "/DCS_onesF.dcs";
	WriteDCStoFile(filename);
	StartSyncScan();
	QApplication::processEvents();
	for (int test = 0; test < 10; test++)
	{
		read_GlobalFile("../config/onesF.dat");
		QString filenameRB = path + "/onesFRB_" + QString::number(test) + ".dat";
		myUSBpix->SaveGlobalRB(qPrintable(filenameRB), chipID);
	}
	WaferLoopProgressBar->setValue(9);
	QApplication::processEvents();

	// reload config
	read_GlobalFile("../config/ref_val.dat");

	// pixelregister tests
	for (int index = 0; index < 4; index++)
	{
		TestPatternComboBox->setCurrentIndex(index);
		testPixelReg();
		CopyPixeltestFile(path, "HitbusRB_", index);
		CopyPixeltestFile(path, "EnableRB_", index);
		CopyPixeltestFile(path, "Cap0RB_", index);
		CopyPixeltestFile(path, "Cap1_", index);
		CopyPixeltestFile(path, "TDAC0RB_", index);
		CopyPixeltestFile(path, "TDAC1RB_", index);
		CopyPixeltestFile(path, "TDAC2RB_", index);
		CopyPixeltestFile(path, "TDAC3RB_", index);
		CopyPixeltestFile(path, "TDAC4RB_", index);
		CopyPixeltestFile(path, "FDAC0RB_", index);
		CopyPixeltestFile(path, "FDAC1RB_", index);
		CopyPixeltestFile(path, "FDAC2RB_", index);
		CopyPixeltestFile(path, "FDAC3RB_", index);

		WaferLoopProgressBar->setValue(10 + index);
		QApplication::processEvents();
	}

	writeSaveStrb();
	
	//digital scan
	int the_value = 0x00000000;
	for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
	myUSBpix->WritePixelSingleLatch(CAP0, chipID);
	for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
	myUSBpix->WritePixelSingleLatch(CAP1, chipID);

	read_GlobalFile("../config/ref_val_dig.dat");
	ScanComboBox->setCurrentIndex(0);
	scanFileNameLineEdit->setText(path + "/digital");
	start_Scan();

	//analog scan
	the_value = 0xffffffff;
	for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
	myUSBpix->WritePixelSingleLatch(CAP0, chipID);
	for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
	myUSBpix->WritePixelSingleLatch(CAP1, chipID);
	read_GlobalFile("../config/ref_val_analog.dat");
	ScanComboBox->setCurrentIndex(1);
	scanFileNameLineEdit->setText(path + "/analog");
	start_Scan();

	//threshold scan
	ScanComboBox->setCurrentIndex(2);
	scanFileNameLineEdit->setText(path + "/threshold");
	start_Scan();

	PowerFEDown();
	WaferLoopProgressBar->hide();
}
void MainForm::CopyPixeltestFile(QString path, QString filenamebasis, int index)
{
		QString sourcefilename = filenamebasis + QString::number(index) + "_map.out";
		QString drainfilename = path + "/" + filenamebasis + QString::number(index) + "_map.out";
		QDir filedir;
		filedir.remove(drainfilename);
		filedir.rename(sourcefilename, drainfilename);
}

bool MainForm::WriteDCStoFile(QString filename)
{
	refresh_DCS();

	std::ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif


	fout.open(qPrintable(filename));

	if (fout == NULL)
		return false;

	// write date and time...
		fout << "#" << endl;
		fout << "# DCS list format:" << endl;
		fout << "# <Pwr channel> \t <voltage [V]> \t <current [A]> " << endl;
		fout << "#" << endl;
		fout << "# Read at: \t" << std::string(timebuf) << endl;
		fout << endl;
		
	fout << "VDDD1" << "\t" << dcsInst->PSU[VDDD1]->GetVoltage() << "\t" << dcsInst->PSU[VDDD1]->GetCurrent() << endl;
	fout << "VDDA1" << "\t" << dcsInst->PSU[VDDA1]->GetVoltage() << "\t" << dcsInst->PSU[VDDA1]->GetCurrent() << endl;
	fout << "VDDD2" << "\t" << dcsInst->PSU[VDDD2]->GetVoltage() << "\t" << dcsInst->PSU[VDDD2]->GetCurrent() << endl;
	fout << "VDDA2" << "\t" << dcsInst->PSU[VDDA2]->GetVoltage() << "\t" << dcsInst->PSU[VDDA2]->GetCurrent() << endl;

	fout.close();
	return true;
}

void MainForm::startHITORScan()
{
	startHITORScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	startHITORScanButton->setText("wait...");
	myUSBpix->WriteCommand(FE_EN_DATA_TAKE, chipID);
	QApplication::processEvents();

	//myUSBpix->SetCalibrationMode();
	//myUSBpix->SetGlobalVal(DIGHITIN_SEL, 1);
	//myUSBpix->WriteGlobal(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL));

	//int pixelmask = 0xffffffff;
	//for (int i = PIXEL26880; i <= PIXEL32; i++)
	//{
	//	myUSBpix->SetPixelVal(i, pixelmask, HITBUS);
	//	myUSBpix->SetPixelVal(i, ~pixelmask, ENABLE);
	//}
	//myUSBpix->SetPixelVal(PIXEL26240, 0xfffffffe, HITBUS);
	//myUSBpix->SetPixelVal(PIXEL26240, ~0xfffffffe, ENABLE);
	//myUSBpix->WritePixel(HITBUS);
	//myUSBpix->WritePixel(ENABLE);

	//myUSBpix->SetGlobalVal(COLPR_MODE, 0);
	//myUSBpix->SetGlobalVal(COLPR_ADDR, 0);

	myUSBpix->setTriggerMode(1);

	
	//int ScanVarStart = 0;
	//int ScanVarStop = 0;
	//int stepquant = 1;
	//int stepwidth = 1;
	//int quantity = 1;
	//int startpixel = 0;
	//int MaskStepSize = 1;
	//int MaskStepCount = 672; // to be 26880 after debugging

	writeSaveStrb();	

	//int data = 0;
	//myUSBpix->WriteRegister(CS_CONFIGURATION_NR, data); // set step_nr register to 0

	myUSBpix->StartMeasurement(); // needed, because USBpix FPGA does only accept ext. triggers when measurement running...
	myUSBpix->StartHitORScan();
	//myUSBpix->ClearSRAM();
	//myUSBpix->ClearConfHisto();
	//myUSBpix->ClearTOTHisto();
	//myUSBpix->StartScan(PLSRDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, MaskStepSize, MaskStepCount, SHIFT_ENABLE + SHIFT_HITBUS, false, false);
	myUSBpix->StopMeasurement();

	QString resultfilename = "HITORscan.bin";
	QString configfilename = "HITORscan.dat";

	std::string cfgfilestdstring = configfilename.toStdString();
	ScanStatusLineEdit->setText("writing files");
	QApplication::processEvents();

	writeBinnedHitHisto(1, resultfilename);
	myUSBpix->SaveGlobal((char*)cfgfilestdstring.c_str(), chipID);

	myUSBpix->SetRunMode();

	write_TriggerInput();

	startHITORScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	startHITORScanButton->setText("start HITOR scan");
}

//////////////////   Global Register     /////////////////////////
void MainForm::write_Global()
{
	myUSBpix->WriteGlobal(chipID);
}

void MainForm::save_GlobalChanges()
{
	std::string sb = readGlobalFileName.toStdString();

	if (sb != "")
	{
		myUSBpix->SaveGlobal((char*)(sb.c_str()), chipID);
	}

	read_GlobalFile(sb);
}

bool MainForm::readGlobalReg()
{
	FEDataRBTextEdit->clear();
	QApplication::processEvents();

	myUSBpix->ReadGlobal(chipID);
	QString filename;
	if (getenv("$(USBPIXI4CONF)") != NULL)
		filename = QString(getenv("USBPIXI4CONF"))+"/readback_config.dat";
	else
		filename = "readback_config.dat";
	myUSBpix->SaveGlobalRB(qPrintable(filename), chipID);

	std::string sb = readGlobalFileName.toStdString();


	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream in(&file);
	while (!in.atEnd()) 
	{
		QString line = in.readAll();
		emit RBglobal_line(line);
	}	

	return true;
}

void MainForm::saveGloChanges()
{
	std::string sb = readGlobalFileName.toStdString();

	QFile file((char*)(sb.c_str()));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;


	QTextStream out(&file);
	out << FEDataTextEdit->toPlainText();
	file.close();

	read_GlobalFile("");
}

void MainForm::write_Vthin()
{
	if (!isFEI4B)
	{
		myUSBpix->SetGlobalVal(VTHIN, VthinSpinBox->value(), chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(VTHIN), chipID);
		updateFEDataTextEdit(VTHIN, VthinSpinBox->value());
	}
}

void MainForm::write_VthinAltCoarse()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_VTHIN_ALTCOARSE:VTHIN_ALTCOARSE, VthinAltCoarseSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_VTHIN_ALTCOARSE:VTHIN_ALTCOARSE), chipID);
	updateFEDataTextEdit(isFEI4B?B_VTHIN_ALTCOARSE:VTHIN_ALTCOARSE, VthinAltCoarseSpinBox->value());
}

void MainForm::write_VthinAltFine()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_VTHIN_ALTFINE:VTHIN_ALTFINE, VthinAltFineSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_VTHIN_ALTFINE:VTHIN_ALTFINE), chipID);
	updateFEDataTextEdit(isFEI4B?B_VTHIN_ALTFINE:VTHIN_ALTFINE, VthinAltFineSpinBox->value());
}

void MainForm::write_PrmpVbp_R()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PRMPVBP_R:PRMPVBP_R, PrmpVbp_RSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PRMPVBP_R:PRMPVBP_R), chipID);
	updateFEDataTextEdit(isFEI4B?B_PRMPVBP_R:PRMPVBP_R, PrmpVbp_RSpinBox->value());
}

void MainForm::write_PrmpVbp()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PRMPVBP:PRMPVBP, PrmpVbpSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PRMPVBP:PRMPVBP), chipID);
	updateFEDataTextEdit(isFEI4B?B_PRMPVBP:PRMPVBP, PrmpVbpSpinBox->value());
}

void MainForm::write_DisVbn_CPPM()
{
	if (!isFEI4B)
	{
		myUSBpix->SetGlobalVal(DISVBN_CPPM, DisVbn_CPPMSpinBox->value(), chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DISVBN_CPPM), chipID);
		updateFEDataTextEdit(DISVBN_CPPM, DisVbn_CPPMSpinBox->value());
	}
}

void MainForm::write_DisVbn()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_DISVBN:DISVBN, DisVbnSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_DISVBN:DISVBN), chipID);
	updateFEDataTextEdit(isFEI4B?B_DISVBN:DISVBN, DisVbnSpinBox->value());
}

void MainForm::write_TdacVbp()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_TDACVBP:TDACVBP, TdacVbpSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_TDACVBP:TDACVBP), chipID);
	updateFEDataTextEdit(isFEI4B?B_TDACVBP:TDACVBP, TdacVbpSpinBox->value());
}

void MainForm::write_Amp2Vbn()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_AMP2VBN:AMP2VBN, Amp2VbnSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_AMP2VBN:AMP2VBN), chipID);
	updateFEDataTextEdit(isFEI4B?B_AMP2VBN:AMP2VBN, Amp2VbnSpinBox->value());
}

void MainForm::write_Amp2VbpFol()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_AMP2VBPFOL:AMP2VBPFOL, Amp2VbpFolSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_AMP2VBPFOL:AMP2VBPFOL), chipID);
	updateFEDataTextEdit(isFEI4B?B_AMP2VBPFOL:AMP2VBPFOL, Amp2VbpFolSpinBox->value());
}

void MainForm::write_Amp2Vbp()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_AMP2VBP:AMP2VBP, Amp2VbpSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_AMP2VBP:AMP2VBP), chipID);
	updateFEDataTextEdit(isFEI4B?B_AMP2VBP:AMP2VBP, Amp2VbpSpinBox->value());
}

void MainForm::write_PrmpVbp_T()
{
	if (isFEI4B)
	{
		myUSBpix->SetGlobalVal(PRMPVBP_T, PrmpVbp_TSpinBox->value(), chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(PRMPVBP_T), chipID);
		updateFEDataTextEdit(PRMPVBP_T, PrmpVbp_TSpinBox->value());
	}
}

void MainForm::write_Amp2Vbpff()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_AMP2VBPFF:AMP2VBPFF, Amp2VbpffSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_AMP2VBPFF:AMP2VBPFF), chipID);
	updateFEDataTextEdit(isFEI4B?B_AMP2VBPFF:AMP2VBPFF, Amp2VbpffSpinBox->value());
}

void MainForm::write_FdacVbn()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_FDACVBN:FDACVBN, FdacVbnSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_FDACVBN:FDACVBN), chipID);
	updateFEDataTextEdit(isFEI4B?B_FDACVBN:FDACVBN, FdacVbnSpinBox->value());
}

void MainForm::write_PrmpVbp_L()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PRMPVBP_L:PRMPVBP_L, PrmpVbp_LSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PRMPVBP_L:PRMPVBP_L), chipID);
	updateFEDataTextEdit(isFEI4B?B_PRMPVBP_L:PRMPVBP_L, PrmpVbp_LSpinBox->value());
}

void MainForm::write_PrmpVbnFol()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PRMPVBNFOL:PRMPVBNFOL, PrmpVbnFolSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PRMPVBNFOL:PRMPVBNFOL), chipID);
	updateFEDataTextEdit(isFEI4B?B_PRMPVBNFOL:PRMPVBNFOL, PrmpVbnFolSpinBox->value());
}

void MainForm::write_PrmpVbpf()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PRMPVBPF:PRMPVBPF, PrmpVbpfSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PRMPVBPF:PRMPVBPF), chipID);
	updateFEDataTextEdit(isFEI4B?B_PRMPVBPF:PRMPVBPF, PrmpVbpfSpinBox->value());
}

void MainForm::write_PrmpVbnLcc()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PRMPVBNLCC:PRMPVBNLCC, PrmpVbnLccSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PRMPVBNLCC:PRMPVBNLCC), chipID);
	updateFEDataTextEdit(isFEI4B?B_PRMPVBNLCC:PRMPVBNLCC, PrmpVbnLccSpinBox->value());
}

void MainForm::write_PlsrDac()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_PLSRDAC:PLSRDAC, PLSRDACSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_PLSRDAC:PLSRDAC), chipID);
	updateFEDataTextEdit(isFEI4B?B_PLSRDAC:PLSRDAC, PLSRDACSpinBox->value());
}

void MainForm::write_ChipLatency()
{
	myUSBpix->SetGlobalVal(isFEI4B?B_CHIP_LATENCY:CHIP_LATENCY, CHIP_LATENCYSpinBox->value(), chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(isFEI4B?B_CHIP_LATENCY:CHIP_LATENCY), chipID);
	updateFEDataTextEdit(isFEI4B?B_CHIP_LATENCY:CHIP_LATENCY, CHIP_LATENCYSpinBox->value());
}

void MainForm::write_Reg_Nr()
{
	/*	unsigned char bitstream[5];
	bitstream[0] = 0x5A;
	bitstream[1] = 0x10;
	bitstream[2] = 0x40;
	bitstream[3] = 0xFF;
	bitstream[4] = 0xFF;

	myUSBpix->SendBitstream(bitstream, 40);

	//*/myUSBpix->WriteGlobalSingleReg(WriteRegNrSpinBox->value(), chipID);
}

void MainForm::read_Reg_Nr()
{
	int dataRB = myUSBpix->ReadGlobalSingleReg(WriteRegNrSpinBox->value(), chipID);
	emit READREGNRSIGNAL(dataRB);
}

void MainForm::write_Reg29()
{
	int data_EMPTYRECORD = 0;
	int data_CLK2OUTCFG = 0;
	int data_DISABLE8B10B = 0;

	data_EMPTYRECORD = EMPTYRECORDSpinBox->value();
	data_CLK2OUTCFG = (int)CLK2OUTCFGCheckBox->isChecked();
	data_DISABLE8B10B = (int)DISABLE8B10BCheckBox->isChecked();
	myUSBpix->SetGlobalVal(EMPTYRECORD, data_EMPTYRECORD, chipID);
	myUSBpix->SetGlobalVal(CLK2OUTCFG, data_CLK2OUTCFG, chipID);
	myUSBpix->SetGlobalVal(DISABLE8B10B, data_DISABLE8B10B, chipID);
	if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg((int)29, chipID);

	updateFEDataTextEdit(EMPTYRECORD, data_EMPTYRECORD);
	updateFEDataTextEdit(CLK2OUTCFG, data_CLK2OUTCFG);
	updateFEDataTextEdit(DISABLE8B10B, data_DISABLE8B10B);
}

void MainForm::read_Global()
{
	myUSBpix->ReadGlobal(chipID);
	myUSBpix->SaveGlobalRB("RBconfig.dat", chipID);
}


void MainForm::read_Reg2()
{
	int add = 0;
	int size = 0;
	int valueRB = 0;

	myUSBpix->ReadGlobalSingleReg(2, chipID);

	myUSBpix->GetGlobalRBVarAddVal(CONFADDRENABLE, add, size, valueRB, chipID);
	emit CONFADDRENABLESignal(valueRB);
	myUSBpix->GetGlobalRBVarAddVal(TRIGCNT, add, size, valueRB, chipID);
	emit TRIGCNTSignal(valueRB);
}

void MainForm::read_Reg29()
{
	int add = 0;
	int size = 0;
	int valueRB = 0;

	myUSBpix->ReadGlobalSingleReg(29, chipID);

	//myUSBpix->GetGlobalRBVarAddVal(CFGSPARE2, add, size, valueRB);
	//emit CFGSPARE2Signal(valueRB);
	myUSBpix->GetGlobalRBVarAddVal(EMPTYRECORD, add, size, valueRB, chipID);
	emit EMPTYRECORDSignal(valueRB);
	myUSBpix->GetGlobalRBVarAddVal(CLK2OUTCFG, add, size, valueRB, chipID);
	emit CLK2OUTCFGSignal(valueRB);
	myUSBpix->GetGlobalRBVarAddVal(DISABLE8B10B, add, size, valueRB, chipID);
	emit DISABLE8B10BSignal(valueRB);
	//myUSBpix->GetGlobalRBVarAddVal(CONFADDRENABLE, add, size, valueRB);
	//emit CONFADDRENABLESignal(valueRB);
	//myUSBpix->GetGlobalRBVarAddVal(TRIGCNT, add, size, valueRB);
	//emit TRIGCNTSignal(valueRB);
}

void MainForm::read_Reg18()
{

}


//////////////////// config Global //////////////////////////////////////////
void MainForm::openReadGlobalFileDialog()
{
	QString oldfilename = readGlobalFileLineEdit->text();
	readGlobalFileName = QFileDialog::getOpenFileName(this,
		tr("Select Global Register Configuration File"), oldfilename, tr("Dat Files (*.dat)"));

	if (!readGlobalFileName.isEmpty())
	{
		readGlobalFileLineEdit->setText(readGlobalFileName);
	}

	read_GlobalFile("");
}

void MainForm::read_GlobalFile(std::string filename)
{
	std::string sb = "";

	if (filename != "")
		sb = filename;
	else
		sb = readGlobalFileName.toStdString();

	if (sb != "")
		if (!myUSBpix->ReadGlobalFile((char*)(sb.c_str()), chipID))
			myUSBpix->LoadGlobalDefault(chipID);

	UpdateGUItoGlobalVals();


	QFile file1((char*)(sb.c_str()));
	if (!file1.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in1(&file1);
	while (!in1.atEnd()) 
	{
		QString line1 = in1.readAll();
		emit Data_glo_line(line1);
	}


	write_Global();
}

void MainForm::UpdateGUItoGlobalVals()
{
	int Address;
	int Size;
	int Value;
	//	// ********* set SpinBoxes **************
	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_TRIGCNT:TRIGCNT, Address, Size, Value, chipID);
	TRIGCNTSpinBox->setValue(Value);

	//enable add rec
	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_CONFADDRENABLE:CONFADDRENABLE, Address, Size, Value, chipID);
	CONFADDRENABLECheckBox->setChecked(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_EMPTYRECORD:EMPTYRECORD, Address, Size, Value, chipID);
	EMPTYRECORDSpinBox->setValue(Value);


	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_CLK2OUTCFG:CLK2OUTCFG, Address, Size, Value, chipID);
	CLK2OUTCFGCheckBox->setChecked(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_DISABLE8B10B:DISABLE8B10B, Address, Size, Value, chipID);
	DISABLE8B10BCheckBox->setChecked(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_CHIP_LATENCY:CHIP_LATENCY, Address, Size, Value, chipID);
	CHIP_LATENCYSpinBox->setValue(Value);

	if (!isFEI4B)
	{
		myUSBpix->GetGlobalVarAddVal(VTHIN, Address, Size, Value, chipID);
		VthinSpinBox->setValue(Value);
	}

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_VTHIN_ALTCOARSE:VTHIN_ALTCOARSE, Address, Size, Value, chipID);
	VthinAltCoarseSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_VTHIN_ALTFINE:VTHIN_ALTFINE, Address, Size, Value, chipID);
	VthinAltFineSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_AMP2VBN:AMP2VBN, Address, Size, Value, chipID);
	Amp2VbnSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_AMP2VBP:AMP2VBP, Address, Size, Value, chipID);
	Amp2VbpSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_AMP2VBPFF:AMP2VBPFF, Address, Size, Value, chipID);
	Amp2VbpffSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_AMP2VBPFOL:AMP2VBPFOL, Address, Size, Value, chipID);
	Amp2VbpFolSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PRMPVBP:PRMPVBP, Address, Size, Value, chipID);
	PrmpVbpSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PRMPVBP_R:PRMPVBP_R, Address, Size, Value, chipID);
	PrmpVbp_RSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PRMPVBP_L:PRMPVBP_L, Address, Size, Value, chipID);
	PrmpVbp_LSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_TDACVBP:TDACVBP, Address, Size, Value, chipID);
	TdacVbpSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_FDACVBN:FDACVBN, Address, Size, Value, chipID);
	FdacVbnSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_DISVBN:DISVBN, Address, Size, Value, chipID);
	DisVbnSpinBox->setValue(Value);

	if (!isFEI4B)
	{
		myUSBpix->GetGlobalVarAddVal(DISVBN_CPPM, Address, Size, Value, chipID);
		DisVbn_CPPMSpinBox->setValue(Value);
	}

	if (!isFEI4B)
	{
		myUSBpix->GetGlobalVarAddVal(PRMPVBP_T, Address, Size, Value, chipID);
		PrmpVbp_TSpinBox->setValue(Value);
	}

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PRMPVBNFOL:PRMPVBNFOL, Address, Size, Value, chipID);
	PrmpVbnFolSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PRMPVBNLCC:PRMPVBNLCC, Address, Size, Value, chipID);
	PrmpVbnLccSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PRMPVBPF:PRMPVBPF, Address, Size, Value, chipID);
	PrmpVbpfSpinBox->setValue(Value);

	myUSBpix->GetGlobalVarAddVal(isFEI4B?B_PLSRDAC:PLSRDAC, Address, Size, Value, chipID);
	PLSRDACSpinBox->setValue(Value);
}

void MainForm::openSaveGlobalFileDialog()
{
	QString oldfilename = saveGlobalFileLineEdit->text();
	saveGlobalFileName = QFileDialog::getSaveFileName(this,
		tr("Select Global Register Configuration File"), oldfilename, tr("Dat Files (*.dat)"));

	if (!saveGlobalFileName.isEmpty())
	{
		saveGlobalFileLineEdit->setText(saveGlobalFileName);
		connect_save_GlobalFile();
	}
}

void MainForm::connect_save_GlobalFile()
{
	save_GlobalFile("");
}

void MainForm::save_GlobalFile(std::string filename)
{
	std::string sb = "";

	if (filename != "")
		sb = filename;
	else		
		sb = saveGlobalFileName.toStdString();

	if (sb != "")
		myUSBpix->SaveGlobal((char*)(sb.c_str()), chipID);
}

void MainForm::updateFEDataTextEdit(int index, int value)
{
	std::string name = myUSBpix->getGlobalVarName(index) + "\t";
	
	QString old_text = FEDataTextEdit->toPlainText();
	QStringList list = old_text.split("\n");

	int size = list.size();
	for (int i = 0; i < size; i++)
	{
		if (list[i].contains(QString(name.c_str())))
			list[i] = QString(name.c_str()) + "\t" + QString::number(value);

	}

	QString new_text = list.join("\n");
	FEDataTextEdit->clear();
	FEDataTextEdit->setPlainText(new_text);
}

//////////////////// Pixel Register //////////////////////////////////
void MainForm::write_Pixel()					//write Pixel Register
{
	QString Hitbus = HitbusSpinBox->text();
	if (0x02 & Hitbus.toInt())
	{
		int the_value = 0x00000001;
		myUSBpix->SetPixelVal(PIXEL26880, the_value, HITBUS, chipID);
		the_value = 0x00000001;
		for (int pixint=PIXEL26880+1; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
	}
	else if (0x01 & Hitbus.toInt())
	{
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
	}	
	else
	{
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
	}

	QString TDAC = TDACSpinBox->text();
	if (0x01 & TDAC.toInt())
	{		
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC0, chipID);
	}
	else 
	{	
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC0, chipID);
	}

	if (0x02 & TDAC.toInt())
	{	
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC1, chipID);		
	}
	else 
	{		
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC1, chipID);
	}

	if (0x04 & TDAC.toInt())
	{
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC2, chipID);
	}
	else 
	{		
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC2, chipID);
	}

	if (0x08 & TDAC.toInt())
	{		
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC3, chipID);
	}
	else 
	{	
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC3, chipID);
	}

	if (0x10 & TDAC.toInt())
	{	
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC4, chipID);
	}
	else 
	{
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, TDAC4, chipID);
	}


	QString FDAC = FDACSpinBox->text();
	if (0x01 & FDAC.toInt())
	{	
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC0, chipID);
	}
	else 
	{	
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC0, chipID);
	}

	if (0x02 & FDAC.toInt())
	{		
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC1, chipID);
	}
	else 
	{		
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC1, chipID);
	}

	if (0x04 & FDAC.toInt())
	{	
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC2, chipID);
	}
	else 
	{		
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC2, chipID);
	}

	if (0x08 & FDAC.toInt())
	{	
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC3, chipID);
	}
	else 
	{		
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, FDAC3, chipID);
	}

	QString Enable = EnableSpinBox->text();
	if (0x02 & Enable.toInt())
	{
		int the_value = 0x00000001;
		myUSBpix->SetPixelVal(PIXEL26880, the_value, ENABLE, chipID);
		the_value = 0x00000001;
		for (int pixint=PIXEL26880+1; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
	}
	else if (0x01 & Enable.toInt())
	{
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
	}	
	else
	{
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
	}

	QString Cap0 = CAP0SpinBox->text();
	if (0x02 & Cap0.toInt())
	{
		int the_value = 0x00000001;
		myUSBpix->SetPixelVal(PIXEL26880, the_value, CAP0, chipID);
		the_value = 0x00000001;
		for (int pixint=PIXEL26880+1; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
	}
	else if (0x01 & Cap0.toInt())
	{
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
	}
	else
	{
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
	}

	QString Cap1 = CAP1SpinBox->text();
	if (0x02 & Cap1.toInt())
	{
		int the_value = 0x00000001;
		myUSBpix->SetPixelVal(PIXEL26880, the_value, CAP1, chipID);
		the_value = 0x00000001;
		for (int pixint=PIXEL26880+1; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
	}
	else if (0x01 & Cap1.toInt())
	{
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
	}	
	else
	{
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
	}

	QString DigInj = DigInjSpinBox->text();
	if (0x02 & DigInj.toInt())
	{
		int the_value = 0x00000001;
		myUSBpix->SetPixelVal(PIXEL26880, the_value, DIGINJ, chipID);
		the_value = 0x00000001;
		for (int pixint=PIXEL26880+1; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
	}	
	else if (0x01 & DigInj.toInt())
	{
		int the_value = 0xffffffff;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
	}	
	else
	{
		int the_value = 0x00000000;
		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
			myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
	}
	myUSBpix->WritePixel(chipID);
}

void MainForm::write_Hitbus()
{
	QString Hitbus = HitbusSpinBox->text();
	if ((HITBUSDCSpinBox->value()) == 40)
	{
		if (0x02 & Hitbus.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
		}
		else if (0x01 & Hitbus.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
		}
	}
	else
	{
		if (0x02 & Hitbus.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=(PIXEL26880 + (HITBUSDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (HITBUSDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
		}
		else if (0x01 & Hitbus.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (HITBUSDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (HITBUSDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (HITBUSDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (HITBUSDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, HITBUS, chipID);
		}
	}
	myUSBpix->WritePixelSingleLatchDC(HITBUS, HITBUSDCSpinBox->value(), chipID);
}

void MainForm::write_TDAC()
{
	QString TDAC = TDACSpinBox->text();

	if ((TDACDCSpinBox->value()) == 40)
	{
		if (0x01 & TDAC.toInt())
		{		
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC0, chipID);
		}
		else 
		{	
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC0, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC0, TDACDCSpinBox->value(), chipID);

		if (0x02 & TDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC1, chipID);		
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC1, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC1, TDACDCSpinBox->value(), chipID);
	
		if (0x04 & TDAC.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC2, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC2, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC2, TDACDCSpinBox->value(), chipID);

		if (0x08 & TDAC.toInt())
		{		
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC3, chipID);
		}
		else 
		{	
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC3, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC3, TDACDCSpinBox->value(), chipID);

		if (0x10 & TDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC4, chipID);
		}
		else 
		{
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC4, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC4, TDACDCSpinBox->value(), chipID);
	}
	else
	{
		if (0x01 & TDAC.toInt())
		{		
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC0, chipID);
		}
		else 
		{	
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC0, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC0, TDACDCSpinBox->value(), chipID);

		if (0x02 & TDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC1, chipID);		
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC1, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC1, TDACDCSpinBox->value(), chipID);
	
		if (0x04 & TDAC.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC2, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC2, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC2, TDACDCSpinBox->value(), chipID);

		if (0x08 & TDAC.toInt())
		{		
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC3, chipID);
		}
		else 
		{	
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC3, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC3, TDACDCSpinBox->value(), chipID);

		if (0x10 & TDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC4, chipID);
		}
		else 
		{
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (TDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, TDAC4, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(TDAC4, TDACDCSpinBox->value(), chipID);
	}
}

void MainForm::write_FDAC()
{
	QString FDAC = FDACSpinBox->text();


	if (FDACDCSpinBox->value() == 40)
	{
		if (0x01 & FDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC0, chipID);
		}
		else 
		{	
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC0, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC0, FDACDCSpinBox->value(), chipID);

		if (0x02 & FDAC.toInt())
		{		
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC1, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC1, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC1, FDACDCSpinBox->value(), chipID);

		if (0x04 & FDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC2, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC2, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC2, FDACDCSpinBox->value(), chipID);

		if (0x08 & FDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC3, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC3, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC3, FDACDCSpinBox->value(), chipID);
	}
	else
	{
		if (0x01 & FDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC0, chipID);
		}
		else 
		{	
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC0, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC0, FDACDCSpinBox->value(), chipID);

		if (0x02 & FDAC.toInt())
		{		
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC1, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC1, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC1, FDACDCSpinBox->value(), chipID);

		if (0x04 & FDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC2, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC2, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC2, FDACDCSpinBox->value(), chipID);

		if (0x08 & FDAC.toInt())
		{	
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC3, chipID);
		}
		else 
		{		
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (FDACDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, FDAC3, chipID);
		}
		myUSBpix->WritePixelSingleLatchDC(FDAC3, FDACDCSpinBox->value(), chipID);
	}
}

void MainForm::write_Enable()
{
	QString Enable = EnableSpinBox->text();
	if ((ENABLEDCSpinBox->value()) == 40)
	{
		if (0x02 & Enable.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
		}
		else if (0x01 & Enable.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
		}
	}
	else
	{
		if (0x02 & Enable.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=(PIXEL26880 + (ENABLEDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (ENABLEDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
		}
		else if (0x01 & Enable.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (ENABLEDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (ENABLEDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (ENABLEDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (ENABLEDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, ENABLE, chipID);
		}
	}
	myUSBpix->WritePixelSingleLatchDC(ENABLE, ENABLEDCSpinBox->value(), chipID);
}

void MainForm::write_Cap0()
{
	QString Cap0 = CAP0SpinBox->text();
	if ((CAP0DCSpinBox->value()) == 40)
	{
		if (0x02 & Cap0.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
		}
		else if (0x01 & Cap0.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
		}
	}
	else
	{
		if (0x02 & Cap0.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=(PIXEL26880 + (CAP0DCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP0DCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
		}
		else if (0x01 & Cap0.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (CAP0DCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP0DCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (CAP0DCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP0DCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP0, chipID);
		}
	}
	myUSBpix->WritePixelSingleLatchDC(CAP0, CAP0DCSpinBox->value(), chipID);
}

void MainForm::write_Cap1()
{
	QString Cap1 = CAP1SpinBox->text();
	if ((CAP1DCSpinBox->value()) == 40)
	{
		if (0x02 & Cap1.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
		}
		else if (0x01 & Cap1.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
		}
	}
	else
	{
		if (0x02 & Cap1.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=(PIXEL26880 + (CAP1DCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP1DCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
		}
		else if (0x01 & Cap1.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (CAP1DCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP1DCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (CAP1DCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP1DCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, CAP1, chipID);
		}
	}
	myUSBpix->WritePixelSingleLatchDC(CAP1, CAP1DCSpinBox->value(), chipID);
}

void MainForm::write_DigInj()
{
	QString DigInj = DigInjSpinBox->text();
	if ((DIGINJDCSpinBox->value()) == 40)
	{
		if (0x02 & DigInj.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
		}
		else if (0x01 & DigInj.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
		}
	}
	else
	{
		if (0x02 & DigInj.toInt())
		{
			int the_value = 0x00000001;
			for (int pixint=(PIXEL26880 + (DIGINJDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (DIGINJDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
		}
		else if (0x01 & DigInj.toInt())
		{
			int the_value = 0xffffffff;
			for (int pixint=(PIXEL26880 + (DIGINJDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (DIGINJDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
		}
		else
		{
			int the_value = 0x00000000;
			for (int pixint=(PIXEL26880 + (DIGINJDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (DIGINJDCSpinBox->value() * 21)); pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, DIGINJ, chipID);
		}
	}
	myUSBpix->WritePixelSingleLatchDC(DIGINJ, DIGINJDCSpinBox->value(), chipID);
}

void MainForm::read_HITBUS()
{
	readHITBUSButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readHITBUSButton->setText("wait...");
	QApplication::processEvents();

	int add;
	int siz;
	int val;
	int valRB;
	int errorcount = 0;

	if (HITBUSReadDCSpinBox->value() == 40)
	{
		myUSBpix->ReadPixelSingleLatch(HITBUS, BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, HITBUS, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, HITBUS, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}

		ofstream fout;
		fout.open((char*)QString("HitbusRB_" + QString::number(HitbusSpinBox->value()) + "_map.out").toStdString().c_str(), ios::out);

		bool data = false;
		for (int row = 0; row < 336; row++)
		{
			for (int col = 0; col < 80; col++)
			{
				int pixinthelp = PixHelp->PixColRowToPixInt(col, 335-row/*, pixint*/);
				myUSBpix->GetPixelRBVarAddVal(pixinthelp, add, siz, valRB, HITBUS, BypassPixelReadingCheckBox->isChecked(), chipID);
				if (0x01 & col)
					data = (valRB << (/*32 - */((335-row) % 32))) & 0x80000000;
				else
					data = (valRB >> ((335-row) % 32)) & 0x01;

				if (col == 79)
					fout << data << " " << endl;
				else
					fout << data << " ";
			}
		}
		fout.close();
	}
	else
	{
		myUSBpix->ReadPixelSingleLatchDC(HITBUS, HITBUSReadDCSpinBox->value(), BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=(PIXEL26880 + (HITBUSReadDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (HITBUSReadDCSpinBox->value() * 21)); pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, HITBUS, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, HITBUS, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}
	}

	if (errorcount)
	{
		readHITBUSButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
		readHITBUSButton->setText("read Hitbus");
	}
	else
	{
		readHITBUSButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		readHITBUSButton->setText("read Hitbus");
	}
	emit HITBUSLCD(errorcount);
}

void MainForm::read_CAP0()
{
	readCAP0Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readCAP0Button->setText("wait...");
	QApplication::processEvents();

	int add;
	int siz;
	int val;
	int valRB;
	int errorcount = 0;

	if (CAP0ReadDCSpinBox->value() == 40)
	{
		myUSBpix->ReadPixelSingleLatch(CAP0, BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, CAP0, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, CAP0, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}

		ofstream fout;
		fout.open((char*)QString("Cap0RB_" + QString::number(CAP0SpinBox->value()) + "_map.out").toStdString().c_str(), ios::out);

		bool data = false;
		for (int row = 0; row < 336; row++)
		{
			for (int col = 0; col < 80; col++)
			{
				int pixinthelp = PixHelp->PixColRowToPixInt(col, 335-row/*, pixint*/);
				myUSBpix->GetPixelRBVarAddVal(pixinthelp, add, siz, valRB, CAP0, BypassPixelReadingCheckBox->isChecked(), chipID);
				if (0x01 & col)
					data = (valRB << (/*32 - */((335-row) % 32))) & 0x80000000;
				else
					data = (valRB >> ((335-row) % 32)) & 0x01;

				if (col == 79)
					fout << data << " " << endl;
				else
					fout << data << " ";
			}
		}
		fout.close();
	}
	else
	{
		myUSBpix->ReadPixelSingleLatchDC(CAP0, CAP0ReadDCSpinBox->value(), BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=(PIXEL26880 + (CAP0ReadDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP0ReadDCSpinBox->value() * 21)); pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, CAP0, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, CAP0, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}
	}

	if (errorcount)
	{
		readCAP0Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
		readCAP0Button->setText("read Cap0");
	}
	else
	{
		readCAP0Button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		readCAP0Button->setText("read Cap0");
	}
	emit CAP0LCD(errorcount);
}

void MainForm::read_CAP1()
{
	readCAP1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readCAP1Button->setText("wait...");
	QApplication::processEvents();

	int add;
	int siz;
	int val;
	int valRB;
	int errorcount = 0;

	if (CAP1ReadDCSpinBox->value() == 40)
	{
		myUSBpix->ReadPixelSingleLatch(CAP1, BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, CAP1, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, CAP1, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}

		ofstream fout;
		fout.open((char*)QString("Cap1RB_" + QString::number(CAP1SpinBox->value()) + "_map.out").toStdString().c_str(), ios::out);

		bool data = false;
		for (int row = 0; row < 336; row++)
		{
			for (int col = 0; col < 80; col++)
			{
				int pixinthelp = PixHelp->PixColRowToPixInt(col, 335-row/*, pixint*/);
				myUSBpix->GetPixelRBVarAddVal(pixinthelp, add, siz, valRB, CAP1, BypassPixelReadingCheckBox->isChecked(), chipID);
				if (0x01 & col)
					data = (valRB << (/*32 - */((335-row) % 32))) & 0x80000000;
				else
					data = (valRB >> ((335-row) % 32)) & 0x01;

				if (col == 79)
					fout << data << " " << endl;
				else
					fout << data << " ";
			}
		}
		fout.close();
	}
	else
	{
		myUSBpix->ReadPixelSingleLatchDC(CAP1, CAP1ReadDCSpinBox->value(), BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=(PIXEL26880 + (CAP1ReadDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (CAP1ReadDCSpinBox->value() * 21)); pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, CAP1, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, CAP1, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}
	}

	if (errorcount)
	{
		readCAP1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
		readCAP1Button->setText("read Cap1");
	}
	else
	{
		readCAP1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		readCAP1Button->setText("read Cap1");
	}
	emit CAP1LCD(errorcount);
}

void MainForm::read_TDAC()
{	
	readTDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readTDACButton->setText("wait...");
	QApplication::processEvents();

	int add;
	int siz;
	int val;
	int valRB;
	int errorcount = 0;
	//int mask = 0;
	int tdac = 4;
		for (int latch = TDAC4; latch <= TDAC0; latch++)
		{
			if (TDACReadDCSpinBox->value() == 40)
			{
				myUSBpix->ReadPixelSingleLatch(latch, BypassPixelReadingCheckBox->isChecked(), chipID);

				for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				{
					myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, latch, BypassPixelReadingCheckBox->isChecked(), chipID);
					myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, latch, chipID);
					if (val != valRB)
					{
						for (int bit = 0; bit < 32; bit++)
						{
							val = val >> bit;
							valRB = valRB >> bit;
							if ((0x01 & valRB) ^ (0x01 & val))
								errorcount++;
						}
					}
				}

				ofstream fout;
				fout.open((char*)QString("TDAC" + QString::number(tdac) + "RB_" + QString::number(TDACSpinBox->value()) + "_map.out").toStdString().c_str(), ios::out);

				bool data = false;
				for (int row = 0; row < 336; row++)
				{
					for (int col = 0; col < 80; col++)
					{
						int pixinthelp = PixHelp->PixColRowToPixInt(col, 335-row/*, pixint*/);
						myUSBpix->GetPixelRBVarAddVal(pixinthelp, add, siz, valRB, latch, BypassPixelReadingCheckBox->isChecked(), chipID);
						if (0x01 & col)
							data = (valRB << (/*32 - */((335-row) % 32))) & 0x80000000;
						else
							data = (valRB >> ((335-row) % 32)) & 0x01;

						if (col == 79)
							fout << data << " " << endl;
						else
							fout << data << " ";
					}
				}
				fout.close();
			}
			else
			{
				myUSBpix->ReadPixelSingleLatchDC(latch, TDACReadDCSpinBox->value(), BypassPixelReadingCheckBox->isChecked(), chipID);

				for (int pixint=(PIXEL26880 + (TDACReadDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (TDACReadDCSpinBox->value() * 21)); pixint++)
				{
					myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, latch, BypassPixelReadingCheckBox->isChecked(), chipID);
					myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, latch, chipID);
					if (val != valRB)
					{
						for (int bit = 0; bit < 32; bit++)
						{
							val = val >> bit;
							valRB = valRB >> bit;
							if ((0x01 & valRB) ^ (0x01 & val))
								errorcount++;
						}
					}
				}
			}
			tdac--;
		}

	if (errorcount)
	{
		readTDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
		readTDACButton->setText("read Tdac");
	}
	else
	{
		readTDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		readTDACButton->setText("read Tdac");
	}
	emit TDACLCD(errorcount);

}

void MainForm::read_FDAC()
{	
	readFDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readFDACButton->setText("wait...");
	QApplication::processEvents();

	int add;
	int siz;
	int val;
	int valRB;
	int errorcount = 0;
	//int mask = 0;
	int fdac = 0;
		for (int latch = FDAC0; latch <= FDAC3; latch++)
		{
			if (FDACReadDCSpinBox->value() == 40)
			{
				myUSBpix->ReadPixelSingleLatch(latch, BypassPixelReadingCheckBox->isChecked(), chipID);

				for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				{
					myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, latch, BypassPixelReadingCheckBox->isChecked(), chipID);
					myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, latch, chipID);
					if (val != valRB)
					{
						for (int bit = 0; bit < 32; bit++)
						{
							val = val >> bit;
							valRB = valRB >> bit;
							if ((0x01 & valRB) ^ (0x01 & val))
								errorcount++;
						}
					}
				}

				ofstream fout;
				fout.open((char*)QString("FDAC" + QString::number(fdac) + "RB_" + QString::number(FDACSpinBox->value()) + "_map.out").toStdString().c_str(), ios::out);

				bool data = false;
				for (int row = 0; row < 336; row++)
				{
					for (int col = 0; col < 80; col++)
					{
						int pixinthelp = PixHelp->PixColRowToPixInt(col, 335-row/*, pixint*/);
						myUSBpix->GetPixelRBVarAddVal(pixinthelp, add, siz, valRB, latch, BypassPixelReadingCheckBox->isChecked(), chipID);
						if (0x01 & col)
							data = (valRB << (/*32 - */((335-row) % 32))) & 0x80000000;
						else
							data = (valRB >> ((335-row) % 32)) & 0x01;

						if (col == 79)
							fout << data << " " << endl;
						else
							fout << data << " ";
					}
				}
				fout.close();
			}
			else
			{
				myUSBpix->ReadPixelSingleLatchDC(latch, FDACReadDCSpinBox->value(), BypassPixelReadingCheckBox->isChecked(), chipID);

				for (int pixint=(PIXEL26880 + (FDACReadDCSpinBox->value() * 21)); pixint<=(PIXEL26240+ (FDACReadDCSpinBox->value() * 21)); pixint++)
				{
					myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, latch, BypassPixelReadingCheckBox->isChecked(), chipID);
					myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, latch, chipID);
					if (val != valRB)
					{
						for (int bit = 0; bit < 32; bit++)
						{
							val = val >> bit;
							valRB = valRB >> bit;
							if ((0x01 & valRB) ^ (0x01 & val))
								errorcount++;
						}
					}
				}
			}
			fdac++;
		}

	if (errorcount)
	{
		readFDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
		readFDACButton->setText("read Fdac");
	}
	else
	{
		readFDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		readFDACButton->setText("read Fdac");
	}
	emit FDACLCD(errorcount);

}

void MainForm::read_ENABLE()
{
	readENABLEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readENABLEButton->setText("wait...");
	QApplication::processEvents();

	int add;
	int siz;
	int val;
	int valRB;
	int errorcount = 0;

	if (ENABLEReadDCSpinBox->value() == 40)
	{
		myUSBpix->ReadPixelSingleLatch(ENABLE, BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, ENABLE, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, ENABLE, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}

		ofstream fout;
		fout.open((char*)QString("EnableRB_" + QString::number(EnableSpinBox->value()) + "_map.out").toStdString().c_str(), ios::out);

		bool data = false;
		for (int row = 0; row < 336; row++)
		{
			for (int col = 0; col < 80; col++)
			{
				int pixinthelp = PixHelp->PixColRowToPixInt(col, 335-row/*, pixint*/);
				myUSBpix->GetPixelRBVarAddVal(pixinthelp, add, siz, valRB, ENABLE, BypassPixelReadingCheckBox->isChecked(), chipID);
				if (0x01 & col)
					data = (valRB << (/*32 - */((335-row) % 32))) & 0x80000000;
				else
					data = (valRB >> ((335-row) % 32)) & 0x01;

				if (col == 79)
					fout << data << " " << endl;
				else
					fout << data << " ";
			}
		}
		fout.close();
	}
	else
	{
		myUSBpix->ReadPixelSingleLatchDC(ENABLE, ENABLEReadDCSpinBox->value(), BypassPixelReadingCheckBox->isChecked(), chipID);

		for (int pixint=(PIXEL26880 + (EnableSpinBox->value() * 21)); pixint<=(PIXEL26240+ (ENABLEReadDCSpinBox->value() * 21)); pixint++)
		{
			myUSBpix->GetPixelRBVarAddVal(pixint, add, siz, valRB, ENABLE, BypassPixelReadingCheckBox->isChecked(), chipID);
			myUSBpix->GetPixelVarAddVal(pixint, add, siz, val, ENABLE, chipID);
			if (val != valRB)
			{
				for (int bit = 0; bit < 32; bit++)
				{
					val = val >> bit;
					valRB = valRB >> bit;
					if ((0x01 & valRB) ^ (0x01 & val))
						errorcount++;
				}
			}
		}
	}

	if (errorcount)
	{
		readENABLEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
		readENABLEButton->setText("read Enable");
	}
	else
	{
		readENABLEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
		readENABLEButton->setText("read Enable");
	}
	emit ENABLELCD(errorcount);
}

void MainForm::read_Pixel()
{
		
	readHITBUSButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readHITBUSButton->setText("wait...");
	
	readCAP0Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readCAP0Button->setText("wait...");

	readCAP1Button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readCAP1Button->setText("wait...");

	readTDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readTDACButton->setText("wait...");

	readFDACButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readFDACButton->setText("wait...");

	readENABLEButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	readENABLEButton->setText("wait...");

	QApplication::processEvents();

	read_HITBUS();
	read_CAP0();
	read_CAP1();
	read_TDAC();
	read_FDAC();
	read_ENABLE();
}
void MainForm::test_GlobalRegister()
{
	int error_nr = 0;
	int quantity = RegTestQuantitySpinBox->value();

	for (int test_nr = 1; test_nr <= quantity; test_nr++)
	{
		error_nr = error_nr + DoGlobalTest();

		emit TESTNR(test_nr);
		emit ERRORNR(error_nr);
		QApplication::processEvents();
	}
}

int MainForm::DoGlobalTest()
{
	int errorcount = 0;
	myUSBpix->WriteGlobal(chipID);
	myUSBpix->ReadGlobal(chipID);

	for (int var = (isFEI4B?B_SMALLHITERASE:TRIGCNT); var <= (isFEI4B?B_LVDSDRVSET12:LVDSDRVSET12); var++)
	{
		int add;
		int size;
		int val_wr;
		int val_rb;
		myUSBpix->GetGlobalVarAddVal(var, add, size, val_wr, chipID);
		myUSBpix->GetGlobalRBVarAddVal(var, add, size, val_rb, chipID);

		//if ((var != PRMPVBP_R) & (var != VTHIN))// & (var != SELECTDO))
		{
			if (val_wr != val_rb)
				errorcount = errorcount + 1;
		}
	}//for(var)
	return errorcount;
}

void MainForm::testPixelReg()
{
	int old_hitbus = HitbusSpinBox->value();
	HitbusSpinBox->setValue(TestPatternComboBox->currentIndex());

	int old_cap0 = CAP0SpinBox->value();
	CAP0SpinBox->setValue(TestPatternComboBox->currentIndex());

	int old_cap1 = CAP1SpinBox->value();
	CAP1SpinBox->setValue(TestPatternComboBox->currentIndex());

	int old_tdac = TDACSpinBox->value();
	TDACSpinBox->setValue(TestPatternComboBox->currentIndex());

	int old_fdac = FDACSpinBox->value();
	FDACSpinBox->setValue(TestPatternComboBox->currentIndex());

	int old_enable = EnableSpinBox->value();
	EnableSpinBox->setValue(TestPatternComboBox->currentIndex());

	int the_value = 0;

	switch (TestPatternComboBox->currentIndex())
	{
	case 0:
		the_value = 0x00000000;
		for (int latch = ENABLE; latch <= FDAC3; latch++)
		{
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, latch, chipID);
		}
		myUSBpix->WritePixel(chipID);
		break;
	case 1:
		the_value = 0xffffffff;
		for (int latch = ENABLE; latch <= FDAC3; latch++)
		{
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, latch, chipID);
		}
		myUSBpix->WritePixel(chipID);
		break;
	case 2:
		the_value = 0xaaaaaaaa;
		for (int latch = ENABLE; latch <= FDAC3; latch++)
		{
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, latch, chipID);
		}
		myUSBpix->WritePixel(chipID);
		break;
	case 3:
		the_value = 0x55555555;
		for (int latch = ENABLE; latch <= FDAC3; latch++)
		{
			for (int pixint=PIXEL26880; pixint<=PIXEL32; pixint++)
				myUSBpix->SetPixelVal(pixint, the_value, latch, chipID);
		}
		myUSBpix->WritePixel(chipID);
		break;
	}

	read_Pixel();

	HitbusSpinBox->setValue(old_hitbus);
	CAP0SpinBox->setValue(old_cap0);
	CAP1SpinBox->setValue(old_cap1);
	TDACSpinBox->setValue(old_tdac);
	FDACSpinBox->setValue(old_fdac);
	EnableSpinBox->setValue(old_enable);
}

//
//void MainForm::test_PixelRegister()
//{
//	int error_nr = 0;
//	int quantity = RegTestQuantitySpinBox->value();
//
//	for (int test_nr = 1; test_nr <= quantity; test_nr++)
//	{
//		bool success = 0;
//		success = regTest->TestPixelReg(test_nr);
//
//		if(!success)
//			error_nr = error_nr + 1;
//
//		emit TESTNR(test_nr);
//		emit ERRORNR(error_nr);
//		QApplication::processEvents();
//	}//end for(test_nr)
//}
//


/*bool RegisterTest::TestPixelReg(int test_nr)
{
// Set Pixel Values alternating
int new_value = 0;
if (test_nr & 0x01)
new_value = 0xfffefffe;
else
new_value = 0xfffefffe;

for (int latch = TDAC0; latch <= CAP1; latch++)
{
for (int var = PIXEL672; var<=PIXEL32; var++)
{
myUSBpix->SetPixelVal(var, new_value, latch);
}
}//end set pixel values

return DoPixelTest();
}



bool RegisterTest::DoPixelTest()
{
myUSBpix->WritePixel();
myUSBpix->ReadPixel();

for (int latch = TDAC0; latch <= CAP1; latch++)
{
for (int var = PIXEL672; var<=PIXEL32; var++)
{
int add;
int size;
int val_wr;
int val_rb;
myUSBpix->GetPixelVarAddVal(var, add, size, val_wr, latch);
myUSBpix->GetPixelRBVarAddVal(var, add, size, val_rb, latch);

if (val_wr != val_rb)
return false;
}//for(pix)
}//for(latch)
return true;
}*/


void MainForm::openHITBUSFileDialog()
{
	QString oldfilename = HITBUSLineEdit->text();
	HITBUSFileName = QFileDialog::getOpenFileName(this,
		tr("Select HITBUS Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!HITBUSFileName.isEmpty())
		HITBUSLineEdit->setText(HITBUSFileName);
}

void MainForm::openCAP0FileDialog()
{
	QString oldfilename = CAP0LineEdit->text();
	CAP0FileName = QFileDialog::getOpenFileName(this,
		tr("Select CAP0 Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!CAP0FileName.isEmpty())
		CAP0LineEdit->setText(CAP0FileName);
}

void MainForm::openCAP1FileDialog()
{
	QString oldfilename = CAP1LineEdit->text();
	CAP1FileName = QFileDialog::getOpenFileName(this,
		tr("Select CAP1 Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!CAP1FileName.isEmpty())
		CAP1LineEdit->setText(CAP1FileName);
}

void MainForm::openENABLEFileDialog()
{
	QString oldfilename = ENABLELineEdit->text();
	ENABLEFileName = QFileDialog::getOpenFileName(this,
		tr("Select ENABLE Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!ENABLEFileName.isEmpty())
		ENABLELineEdit->setText(ENABLEFileName);
}

void MainForm::openTDACFileDialog()
{
	QString oldfilename = TDACLineEdit->text();
	TDACFileName = QFileDialog::getOpenFileName(this,
		tr("Select TDAC Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!TDACFileName.isEmpty())
		TDACLineEdit->setText(TDACFileName);
}

void MainForm::openFDACFileDialog()
{
	QString oldfilename = FDACLineEdit->text();
	FDACFileName = QFileDialog::getOpenFileName(this,
		tr("Select FDAC Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!FDACFileName.isEmpty())
		FDACLineEdit->setText(FDACFileName);
}

void MainForm::openDIGINJFileDialog()
{
	QString oldfilename = DIGINJLineEdit->text();
	DIGINJFileName = QFileDialog::getOpenFileName(this,
		tr("Select DIGINJ Configuration File"), oldfilename, tr("Out Files (*.out)"));

	if (!DIGINJFileName.isEmpty())
		DIGINJLineEdit->setText(DIGINJFileName);
}

bool MainForm::write_TDACFile()
{
	int value[5];
	int pixnr;
	int col;
	int row;
	int newval;

	//lese Wert aus Datei
	QFile file(TDACFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
		value[0] = 0;
		value[1] = 0;
		value[2] = 0;
		value[3] = 0;
		value[4] = 0;

		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();

			// build values for pixel

			if (0x01 &  newval)
				value[0] = value[0] + (0x80000000 >> inner);
			if (0x02 &  newval)
				value[1] = value[1] + (0x80000000 >> inner);
			if (0x04 &  newval)
				value[2] = value[2] + (0x80000000 >> inner);
			if (0x08 &  newval)
				value[3] = value[3] + (0x80000000 >> inner);
			if (0x10 &  newval)
				value[4] = value[4] + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value[0], TDAC0, chipID);
		myUSBpix->SetPixelVal(index, value[1], TDAC1, chipID);
		myUSBpix->SetPixelVal(index, value[2], TDAC2, chipID);
		myUSBpix->SetPixelVal(index, value[3], TDAC3, chipID);
		myUSBpix->SetPixelVal(index, value[4], TDAC4, chipID);
	}
	for (int dc = 0; dc < 40; dc++)
	{
		myUSBpix->WritePixelSingleLatchDC(TDAC0, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(TDAC1, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(TDAC2, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(TDAC3, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(TDAC4, dc, chipID);
	}

	file.close();
	return true;
}

//bool MainForm::write_TDACFile(const char * filename)
//{
//	int value[7];
//	int pixnr;
//	int col;
//	int row;
//	int newval;
//	for (int index = PIXEL672; index <= PIXEL32; index++)
//	{
//		value[0] = 0;
//		value[1] = 0;
//		value[2] = 0;
//		value[3] = 0;
//		value[4] = 0;
//		value[5] = 0;
//		value[6] = 0;
//		for (int inner = 0; inner < 32; inner ++)
//		{
//			pixnr = (index - 4)*32 + inner + 1;
//			PixHelp->PixNrToColRow(pixnr, row, col);
//
//			//lese Wert aus Datei
//			QFile file(filename);
//			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//				return false;
//
//			QTextStream in(&file);
//			QString line;
//			for (int i = 0; i <= row; i++)
//			{
//				line = in.readLine();		// jump to line to read
//			}
//			QStringList list = line.split(QRegExp(" "));
//			newval = list[17-col].toInt();
//
//			// bilde pixelwerte
//
//			if (0x01 &  newval)
//				value[0] = value[0] + (0x80000000 >> inner);
//			if (0x02 &  newval)
//				value[1] = value[1] + (0x80000000 >> inner);
//			if (0x04 &  newval)
//				value[2] = value[2] + (0x80000000 >> inner);
//			if (0x08 &  newval)
//				value[3] = value[3] + (0x80000000 >> inner);
//			if (0x10 &  newval)
//				value[4] = value[4] + (0x80000000 >> inner);
////			if (0x20 &  newval)
////				value[5] = value[5] + (0x80000000 >> inner);
////			if (0x40 &  newval)
////				value[6] = value[6] + (0x80000000 >> inner);
//		}
//		myUSBpix->SetPixelVal(index, value[0], TDAC0);
//		myUSBpix->SetPixelVal(index, value[1], TDAC1);
//		myUSBpix->SetPixelVal(index, value[2], TDAC2);
//		myUSBpix->SetPixelVal(index, value[3], TDAC3);
//		myUSBpix->SetPixelVal(index, value[4], TDAC4);
////		myUSBpix->SetPixelVal(index, value[5], TDAC5);
////		myUSBpix->SetPixelVal(index, value[6], TDAC6);
//	}
//	myUSBpix->WritePixel(FE_WRITE_TDAC0, TDAC0);
//	myUSBpix->WritePixel(FE_WRITE_TDAC1, TDAC1);
//	myUSBpix->WritePixel(FE_WRITE_TDAC2, TDAC2);
//	myUSBpix->WritePixel(FE_WRITE_TDAC3, TDAC3);
//	myUSBpix->WritePixel(FE_WRITE_TDAC4, TDAC4);
////	myUSBpix->WritePixel(FE_WRITE_TDAC5, TDAC5);
////	myUSBpix->WritePixel(FE_WRITE_TDAC6, TDAC6);
//	return true;
//}

bool MainForm::write_HITBUSFile()
{
	int value;
	int pixnr;
	int col;
	int row;
	int newval;

	//lese Wert aus Datei
	QFile file(HITBUSFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
		value = 0;
		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();

			if (0x01 &  newval)
				value = value + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value, HITBUS, chipID);
	}
	for (int dc = 0; dc < 40; dc++)
		myUSBpix->WritePixelSingleLatchDC(HITBUS, dc, chipID);

	file.close();
	return true;
}

bool MainForm::write_ENABLEFile()
{
	int value;
	int pixnr;
	int col;
	int row;
	int newval;


	//lese Wert aus Datei
	QFile file(ENABLEFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
		value = 0;
		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();

			if (0x01 &  newval)
				value = value + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value, ENABLE, chipID);
	}
	for (int dc = 0; dc < 40; dc++)
		myUSBpix->WritePixelSingleLatchDC(ENABLE, dc, chipID);

	file.close();
	return true;
}

bool MainForm::write_CAP0File()
{
	int value;
	int pixnr;
	int col;
	int row;
	int newval;

	//lese Wert aus Datei
	QFile file(CAP0FileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
	
		value = 0;
		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();
			if (0x01 &  newval)
				value = value + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value, CAP0, chipID);
	}
	for (int dc = 0; dc < 40; dc++)
		myUSBpix->WritePixelSingleLatchDC(CAP0, dc, chipID);

	file.close();
	return true;
}

bool MainForm::write_CAP1File()
{
	int value;
	int pixnr;
	int col;
	int row;
	int newval;

	//lese Wert aus Datei
	QFile file(CAP1FileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
		value = 0;
		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();

			if (0x01 &  newval)
				value = value + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value, CAP1, chipID);
	}
	for (int dc = 0; dc < 40; dc++)
		myUSBpix->WritePixelSingleLatchDC(CAP1, dc, chipID);

	file.close();
	return true;
}

bool MainForm::write_DIGINJFile()
{
	int value;
	int pixnr;
	int col;
	int row;
	int newval;

	//lese Wert aus Datei
	QFile file(DIGINJFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
		value = 0;
		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();

			if (0x01 &  newval)
				value = value + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value, DIGINJ, chipID);
	}
	for (int dc = 0; dc < 40; dc++)
		myUSBpix->WritePixelSingleLatchDC(DIGINJ, dc, chipID);

	file.close();
	return true;
}

bool MainForm::write_FDACFile()
{
	int value[4];
	int pixnr;
	int col;
	int row;
	int newval;

	//lese Wert aus Datei
	QFile file(FDACFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString line[336];

	for (int i = 335; i >= 0; i--)
	{
		line[i] = in.readLine();		// jump to line to read
	}

	for (int index = PIXEL26880; index <= PIXEL32; index++)
	{
		value[0] = 0;
		value[1] = 0;
		value[2] = 0;
		value[3] = 0;

		for (int inner = 0; inner < 32; inner ++)
		{
			pixnr = (index - 4)*32 + inner + 1;
			PixHelp->PixNrToColRow(pixnr, row, col);

			QStringList list = line[row].split(QRegExp(" "));
			if (0x01 & col)
				newval = list[col-1].toInt();
			else 
				newval = list[col+1].toInt();

			// bilde pixelwerte

			if (0x01 &  newval)
				value[0] = value[0] + (0x80000000 >> inner);
			if (0x02 &  newval)
				value[1] = value[1] + (0x80000000 >> inner);
			if (0x04 &  newval)
				value[2] = value[2] + (0x80000000 >> inner);
			if (0x08 &  newval)
				value[3] = value[3] + (0x80000000 >> inner);
		}
		myUSBpix->SetPixelVal(index, value[0], FDAC0, chipID);
		myUSBpix->SetPixelVal(index, value[1], FDAC1, chipID);
		myUSBpix->SetPixelVal(index, value[2], FDAC2, chipID);
		myUSBpix->SetPixelVal(index, value[3], FDAC3, chipID);

	}
	for (int dc = 0; dc < 40; dc++)
	{
		myUSBpix->WritePixelSingleLatchDC(FDAC0, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(FDAC1, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(FDAC2, dc, chipID);
		myUSBpix->WritePixelSingleLatchDC(FDAC2, dc, chipID);
	}

	file.close();
	return true;
}

// ********************** Scans Tab ********************************************
// high-level functionality: threshold scan - editor

//void MainForm::set_PixelSelect(int pixnr, int latch)
//{
//	int value = 0;
//	int index_nr = 0;
//	PixHelp->PixNrToArray(pixnr, index_nr, value);
//
//	for (int i = PIXEL672; i <= PIXEL32; i++)
//		if (i == index_nr)
//			myUSBpix->SetPixelVal(index_nr, value, latch);
//		else
//			myUSBpix->SetPixelVal(i, 0x00000000, latch);
//}


void MainForm::setScanType()
{
	switch (ScanComboBox->currentIndex())
	{
	case 0:	// Digital Scan
		disconnect(startValueSlider, SIGNAL(valueChanged(int)), stopValueSlider, SLOT(setValue(int)));
		StepQuantitySlider->setValue(1);
		StepQuantitySlider->setEnabled(false);
		startValueSlider->setValue(1);
		startValueSlider->setEnabled(false);
		stopValueSlider->setValue(1);
		stopValueSlider->setEnabled(false);
		scanFileNameLineEdit->setText("./data/digital");
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 1, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	case 1: // Analog Scan
		StepQuantitySlider->setValue(1);
		StepQuantitySlider->setEnabled(false);
		startValueSlider->setValue(400);
		connect(startValueSlider, SIGNAL(valueChanged(int)), stopValueSlider, SLOT(setValue(int)));
		startValueSlider->setEnabled(true);
		stopValueSlider->setValue(400);
		stopValueSlider->setEnabled(false);
		scanFileNameLineEdit->setText("./data/analog");
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 0, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	case 2: // Threshold Scan
		disconnect(startValueSlider, SIGNAL(valueChanged(int)), stopValueSlider, SLOT(setValue(int)));
		StepQuantitySlider->setValue(201);
		StepQuantitySlider->setEnabled(true);
		startValueSlider->setValue(0);
		startValueSlider->setEnabled(true);
		stopValueSlider->setValue(200);
		stopValueSlider->setEnabled(true);
		scanFileNameLineEdit->setText("./data/threshold");
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 0, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	case 3: // ToT Scan
		StepQuantitySlider->setValue(1);
		StepQuantitySlider->setEnabled(false);
		startValueSlider->setValue(400);
		connect(startValueSlider, SIGNAL(valueChanged(int)), stopValueSlider, SLOT(setValue(int)));
		startValueSlider->setEnabled(true);
		stopValueSlider->setValue(400);
		stopValueSlider->setEnabled(false);
		scanFileNameLineEdit->setText("./data/ToT");
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 0, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	}
}

void MainForm::start_Scan()
{
	startScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0);"));
	startScanButton->setText("wait...");
	ErrorStatusLineEdit->setText("");
	myUSBpix->WriteCommand(FE_EN_DATA_TAKE, chipID);
	myUSBpix->ResetScanStatus();

	QString scanfilename = scanFileNameLineEdit->text();
	//myUSBpix->SetCalibrationMode();
	//myUSBpix->ClearSRAM();	//lösche alte hits
	ScanStatusLineEdit->setText("preparing");
	QApplication::processEvents();
	int pixelmask = 0x00000000;
	switch (MaskStepsComboBox->currentIndex())
	{
	case 0: // all enabled
		pixelmask = 0xffffffff;
		break;
	case 1: // every 2nd enabled
		pixelmask = 0x55555555;
		break;
	case 2: // every 4th enabled
		pixelmask = 0x11111111;
		break;
	case 3: // every 8th enabled
		pixelmask = 0x01010101;
		break;
	case 4: // every 16th enabled
		pixelmask = 0x00010001;
		break;
	case 5: // every 32nd enabled
		pixelmask = 0x00000001;
		break;
	}

	switch (ScanComboBox->currentIndex())
	{
	case 0:	// Digital Scan
		myUSBpix->SetCalibrationMode();
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 1, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	case 1: // Analog Scan
		myUSBpix->SetCalibrationMode();
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 0, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	case 2: // Threshold Scan
		myUSBpix->SetCalibrationMode();
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 0, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	case 3: // ToT Scan
		myUSBpix->SetTOTMode();
		myUSBpix->SetGlobalVal(DIGHITIN_SEL, 0, chipID);
		if(hUSB->HandlePresent()) myUSBpix->WriteGlobalSingleReg(myUSBpix->IndexToRegisterNumber(DIGHITIN_SEL), chipID);
		break;
	}

	//if (ScanComboBox->currentIndex() == 0)
	//{
		for (int i = PIXEL26880; i <= PIXEL32; i++)
			myUSBpix->SetPixelVal(i, pixelmask, ENABLE, chipID);
		myUSBpix->WritePixelSingleLatch(ENABLE, chipID);
	//}
	//else
	//{
	//	for (int i = PIXEL26880; i <= PIXEL32; i++)
	//	{
	//		myUSBpix->SetPixelVal(i, pixelmask, CAP0);
	//		myUSBpix->SetPixelVal(i, pixelmask, CAP1);
	//	}
	//	myUSBpix->WritePixel(CAP0);
	//	myUSBpix->WritePixel(CAP1);
	//}

	int ScanVarStart = startValueSlider->value();
	int ScanVarStop = stopValueSlider->value();
	int stepquant = StepQuantitySlider->value();
	int stepwidth = (ScanVarStop - ScanVarStart) / stepquant;
	int quantity = InjectionQuantitySlider->value();
	int pixelquant = ScanPixQuantSlider->value();
	int startpixel = ScanPixNrSlider->value();

	writeSaveStrb();	
	myUSBpix->WriteRegister(CS_QUANTITY, quantity);

	if (stepwidth == 0)
		stepwidth = 1;
	while (((ScanVarStop - ScanVarStart)/stepwidth) > 255)
	{
		stepwidth++;
	}

	if ((startpixel-1) + pixelquant > 672)
		pixelquant = 672 - startpixel;

	int data = 0;
	myUSBpix->WriteRegister(CS_CONFIGURATION_NR, data); // set step_nr register to 0
	//int conf_nr = 1;

	ScanStatusLineEdit->setText("scanning");
	QApplication::processEvents();

	if (pixelquant == 672)
	{
		//myUSBpix->WriteRegister(CS_STATUS_REG, 0);
		//if (ScanComboBox->currentIndex() == 0)
		//{
			// start Scan

			myUSBpix->ClearSRAM(chipID);
			myUSBpix->ClearConfHisto(chipID);
			myUSBpix->ClearTOTHisto(chipID);

			if (!myUSBpix->StartScan(PLSRDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, MaskStepSizeSpinBox->value(), ((MaskStepsComboBox->currentText().toInt()) / (MaskStepSizeSpinBox->value())), SHIFT_ENABLE, allDCsCheckBox->isChecked(), specialDCloopCheckBox->isChecked(), false))
			{
				ErrorStatusLineEdit->setText("DH or COLPR_MODE");
				ScanStatusLineEdit->setText("ERROR !!!");
				startScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
				startScanButton->setText("start Scan");
				return;
			}
		//}
		//else
		//{
			// start Scan

		//	myUSBpix->ClearSRAM();
		//	myUSBpix->ClearConfHisto();
		//	myUSBpix->ClearTOTHisto();

		//	if (!myUSBpix->StartScan(PLSRDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, 1, 1/*32*/, 0/*SHIFT_CAP0 + SHIFT_CAP1*/, allDCsCheckBox->isChecked()))
		//	{
		//		ScanStatusLineEdit->setText("DH ERROR !!!");
		//		scanProgressBar->hide();
		//		return;
		//	}
		//}
	}

	//while (myUSBpix->ReadRegister(CS_TRIGGER_STRB_LV1)) // wait...
	//{}
	//conf_nr++;


	//myUSBpix->ReadSRAM();	// lese SRAM aus
	QString resultfilename = scanfilename + ".bin";
	QString configfilename = scanfilename + ".dat";
	QString scanconfigfilename = scanfilename + ".info";
	std::string cfgfilestdstring = configfilename.toStdString();
	ScanStatusLineEdit->setText("writing files");
	QApplication::processEvents();

	if (ScanComboBox->currentIndex() == 3)
	{
		std::string resultfilenamestring = resultfilename.toStdString();
		myUSBpix->WriteToTHisto((char*)resultfilenamestring.c_str(), chipID);
	}
	else
	{
		writeBinnedHitHisto(stepquant, resultfilename);
	}
	myUSBpix->SaveGlobal((char*)cfgfilestdstring.c_str(), chipID);
	writeScanConfigFile(PLSRDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, MaskStepSizeSpinBox->value(), (MaskStepsComboBox->currentText().toInt()) / (MaskStepSizeSpinBox->value()), SHIFT_ENABLE/*SHIFT_CAP0 + SHIFT_CAP1*/, allDCsCheckBox->isChecked(), scanconfigfilename);
	myUSBpix->SetRunMode();

	if (ScanComboBox->currentIndex() == 2)
	{
		ScanStatusLineEdit->setText("fitting");
		QApplication::processEvents();
		plot_SCurve(startpixel, pixelquant, stepquant-1, ScanVarStart, ScanVarStop, stepwidth);			// plot SCurve and fit data
		plot_dispersions();		// plot threshold / noise distribution
	}

	if (ScanComboBox->currentIndex() == 3)
		FillToTSimplePlot();

	ScanStatusLineEdit->setText("done :-)");
	startScanButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
	startScanButton->setText("start Scan");
}


//void MainForm::start_ScanMuC()
//{
//	scanProgressBar->show();
//	emit FittingStatus("preparing");
//	disconnect(startScanMyCButton, SIGNAL(clicked()), this, SLOT(start_ScanMuC()));
//	startScanMyCButton->setText("cancel Scan");
//	startScanMyCButton->setStyleSheet(QString::fromUtf8("background-color: rgb(170, 0, 0);\n"
//"background-color: rgb(255, 0, 0);"));
//	connect(startScanMyCButton, SIGNAL(clicked()), this, SLOT(cancel_Scan()));
//	QApplication::processEvents();
//	myUSBpix->SetCalibrationMode();
//	myUSBpix->ClearSRAM();	//lösche alte hits
//
//	int ScanVarStart = startValueSlider->value();
//	int ScanVarStop = stopValueSlider->value();
//	int stepquant = StepQuantitySlider->value();
//	int stepwidth = (ScanVarStop - ScanVarStart)/stepquant;
//	int quantity = InjectionQuantitySlider->value();
//	int pixelquant = ScanPixQuantSlider->value();
//	int startpixel = ScanPixNrSlider->value();
//
//	if (stepwidth == 0)
//		stepwidth = 1;
//	while (((ScanVarStop - ScanVarStart)/stepwidth) > 255)
//	{
//		stepwidth++;
//	}
//
//	int shiftMask = 0;
//
//	stepquant = (ScanVarStop - ScanVarStart)/stepwidth;
//
//	if (startpixel + pixelquant > 2881)
//		pixelquant = 2881 - startpixel;
//
//	writeSaveStrb();
//	myUSBpix->WriteGlobal();
//
//	if (pixelquant == 2880)
//	{
//		int value = 0x00000001;
//		for (int j = PIXEL672; j <= PIXEL32; j++)
//		{	
//			switch (ScanComboBox->currentIndex())
//			{
//			case 0:
//				myUSBpix->SetPixelVal(j, value, SELECT);
//				if (j == PIXEL32)
//				{
//					myUSBpix->WritePixel(FE_WRITE_SELECT, SELECT);
//					shiftMask = SHIFT_SELECT;
//				}
//				break;
//			case 1:
//				myUSBpix->SetPixelVal(j, value, SELECT);
//				myUSBpix->SetPixelVal(j, value, MASK_B);
//				if (j == PIXEL32)
//				{
//					myUSBpix->WritePixel(FE_WRITE_SELECT, SELECT);
//					myUSBpix->WritePixel(FE_WRITE_MASK_B, MASK_B);
//					shiftMask = SHIFT_SELECT + SHIFT_MASK_B;
//				}
//				break;
//			case 2:
//				myUSBpix->SetPixelVal(j, value, SELECT);
//				myUSBpix->SetPixelVal(j, value, HITBUS);
//				if (j == PIXEL32)
//				{
//					myUSBpix->WritePixel(FE_WRITE_SELECT, SELECT);
//					myUSBpix->WritePixel(FE_WRITE_HITBUS, HITBUS);
//					shiftMask = SHIFT_SELECT + SHIFT_HITBUS;
//				}
//				break;
//			case 3:
//				myUSBpix->SetPixelVal(j, value, SELECT);
//				myUSBpix->SetPixelVal(j, value, ENABLE);
//				if (j == PIXEL32)
//				{
//					myUSBpix->WritePixel(FE_WRITE_SELECT, SELECT);
//					myUSBpix->WritePixel(FE_WRITE_ENABLE, ENABLE);
//					shiftMask = SHIFT_SELECT + SHIFT_ENABLE;
//				}
//				break;
//			}
//		}
//
//
//		unsigned char dataRB = 1;
//		while (myUSBpix->ReadRegister(CS_START_CONFIG))
//		{}
//		// start the µC Scan
////		myUSBpix->StartmuCScan(VCALDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, 1, 32, shiftMask);
//		// start Software Scan
//		// myUSBpix->WriteStrbQuantity(quantity);
		// myUSBpix->ClearSRAM();
		// myUSBpix->ClearConfHisto();
		// myUSBpix->ClearTOTHisto();
//		// myUSBpix->StartScan(VCALDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, 1, 32, shiftMask);
//	}
//	else
//	{
//		switch (ScanComboBox->currentIndex())
//		{
//		case 0:
//			set_PixelSelect(startpixel, SELECT);
//			shiftMask = SHIFT_SELECT;
//			break;
//		case 1:
//			set_PixelSelect(startpixel, SELECT);
//			set_PixelSelect(startpixel, MASK_B);
//			shiftMask = SHIFT_SELECT + SHIFT_MASK_B;
//			break;
//		case 2:
//			set_PixelSelect(startpixel, SELECT);
//			set_PixelSelect(startpixel, HITBUS);
//			shiftMask = SHIFT_SELECT + SHIFT_HITBUS;
//			break;
//		case 3:
//			set_PixelSelect(startpixel, SELECT);
//			set_PixelSelect(startpixel, ENABLE);
//			shiftMask = SHIFT_SELECT + SHIFT_ENABLE;
//			break;
//		}
//
//		// start the µC Scan
////		myUSBpix->StartmuCScan(VCALDAC, ScanVarStart, ScanVarStop, stepwidth, quantity, 1, startpixel + pixelquant, shiftMask);
//	}
//
//// Erstelle hier Thread, der Abfragt ob µC busy ist. Ist uC !busy --> thread fertig. Solange thread läuft: proccess events! 
//	IsmuCBusy->start();
//
//
//	while (IsmuCBusy->isRunning())
//	{
//		QApplication::processEvents();
//	}
//
//	QApplication::processEvents();
//	emit FittingStatus("reading data");
//	myUSBpix->ReadSRAM();	// lese SRAM aus
//	emit FittingStatus("fitting data");
//	plot_SCurve(startpixel, pixelquant, stepquant-1, ScanVarStart, ScanVarStop, stepwidth);			// plot SCurve and fit data
//	plot_dispersions();		// plot threshold / noise distribution
//	emit FittingStatus("writing files");
//	if(getenv("USBPIXI4DATA") != NULL)
//	{
//		QString filename;
//		filename = QString(getenv("USBPIXI4DATA"))+"/scan_" + QString::number(TDACdecr) + ".bin";
//		writeBinnedHitHisto(stepquant, ScanVarStart, stepwidth, filename);
//	}
//	else
//	{
//		writeBinnedHitHisto(stepquant, ScanVarStart, stepwidth, "scan.bin");
//	}
//	myUSBpix->SetRunMode();
//	scanProgressBar->hide();
//	emit FittingStatus("done");
//	disconnect(startScanMyCButton, SIGNAL(clicked()), this, SLOT(cancel_Scan()));
//	startScanMyCButton->setText("start Scan");
//	startScanMyCButton->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 0);"));
//	connect(startScanMyCButton, SIGNAL(clicked()), this, SLOT(start_ScanMuC()));
//}
//
//void MainForm::cancel_Scan()
//{
//	myUSBpix->CancelScan();
//}
//
void MainForm::plot_SCurve(int startpixel, int pixelquant, int stepquant, int VcalStart, int VcalStop, int VcalStepwidth)
{
	bool freu = false;
	bool plotted = false; 
	ScanPlot->detachItems();

	int nr_hits = 0;
	for (int pixel = startpixel; pixel < (startpixel + (pixelquant*40)); pixel++)
	{
		int col = 1;
		int row = 1;
		PixHelp->PixNrToColRowDataRB(pixel, row, col);

		int index = 0;
		for (int vcal = VcalStart; vcal <= VcalStop; vcal = vcal + VcalStepwidth)
		{
			x_val[index] = vcal;
			index++;
		}

		for (int step = 1; step <= stepquant + 1; step++)	// fill Data Arrays...
		{
			myUSBpix->GetConfHisto(col-1, row-1, step, nr_hits, chipID);
			y_val[step-1] = nr_hits;
			if (nr_hits)
				freu = true;
		}

		if (freu & !plotted)
		{
			PixNrShowSpinBox->setValue(pixel);
			QApplication::processEvents();

			SCurve = new QwtPlotCurve("Scan");
			QwtSymbol sym;
			sym.setStyle(QwtSymbol::Cross);
			sym.setPen(QColor(Qt::black));
			sym.setSize(5);
			SCurve->setSymbol(&sym);
			SCurve->setStyle(QwtPlotCurve::NoCurve);
			SCurve->setRawSamples(x_val, y_val, stepquant-1);
			SCurve->attach(ScanPlot);
			ScanPlot->replot();	
			plotted = true;
		}
		fit_SCurve(stepquant,/* 100, 0, 0,*/ pixel);
	}
}

void MainForm::print_Plot()
{
	ScanPlot->Print();
	ThresDispPlot->Print();
	NoiseDispPlot->Print();
}

//void MainForm::write_Thresholds(int stepquant, int ScanStartVal, int ScanStepSize)
//{
//	std::ofstream fout;
//	time_t ltime;
//	char timebuf[26];
//
//	time(&ltime);
//#ifndef CF__LINUX
//	ctime_s(timebuf, 26, &ltime);
//#else
//	ctime_r(&ltime, timebuf);
//#endif
//
//	if(getenv("USBPIXI4DATA") != NULL)
//	{
//		QString filename;
//		filename = QString(getenv("USBPIXI4DATA"))+"/thresholds.dat";
//		fout.open(qPrintable(filename));
//	}
//	else
//	{
//		fout.open("thresholds.dat");
//	}
//	fout << "#" << std::endl;
//	fout << "# file list format:" << std::endl;
//	fout << "# <PixelNr> \t <threshold [e^-]> \t <sigma[e^-]>" << std::endl;
//	fout << "#" << std::endl;
//	fout << "# last change: " << std::string(timebuf) << std::endl;
//	fout << std::endl;
//	for (int pixnr = 0; pixnr < 2880; pixnr++)
//	{
//		fout << pixnr+1 << "\t" << thresvector[pixnr] << "\t" << sigmavector[pixnr] << std::endl;
//	}
//	fout.close();
//}
//


void MainForm::writeScanConfigFile(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSiz, int InjCount, int MaskStepSiz, int MaskStepCount, int ShiftMask, bool all_DCs, QString filename)
{
	std::ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif
	fout.open(qPrintable(filename));

	// write date and time...
		fout << "#" << endl;
		fout << "# configuration list format:" << endl;
		fout << "# <name> \t <value> " << endl;
		fout << "# ShiftMask is a code wich masks to shift..." << endl;
		fout << "# Scan all DCs is one, if every 8th DC is scanned at the same time and loop goes over all. Zero means COLPR_MODE and COLPR_ADDR from global cfg respected." << endl;
		fout << "#" << endl;
		fout << "# Scan done at: \t" << std::string(timebuf) << endl;
		fout << endl;
	
	// write scan config...
		if (ScanComboBox->currentIndex() == 0)
			fout << "Digital Scan" << endl;
		if (ScanComboBox->currentIndex() == 1)
			fout << "Analog Scan" << endl;
		if (ScanComboBox->currentIndex() == 2)
			fout << "Threshold Scan" << endl;
		fout << "-------------------------------------------------------" << endl;
		fout << endl;

		fout << "Scan configuration:" << endl;
		fout << "\t Latency                   \t " << D_LV1SpinBox->value() << endl;
		fout << "\t Scan variable             \t " << ScanVarIndex << endl;
		fout << "\t Scan variable start value \t " << ScanStartVal << endl;
		fout << "\t Scan variable stop value  \t " << ScanStopVal << endl;
		fout << "\t Scan variable step size   \t " << ScanStepSiz << endl;
		fout << "\t Injection quantity        \t " << InjCount << endl;
		fout << "\t Mask step size            \t " << MaskStepSiz << endl;
		fout << "\t Mask step quantity        \t " << MaskStepCount << endl;
		fout << "\t Shift Mask Code           \t " << "0x" << std::hex << ShiftMask  << std::dec << endl;
		fout << "\t Scan all DCs              \t " << all_DCs << endl;
		fout << endl;

		fout << "Pixel register configuration: " << endl;
		fout << "\t HITBUS                    \t " << HitbusSpinBox->value() << endl;
		fout << "\t CAP0                      \t " << CAP0SpinBox->value() << endl;
		fout << "\t CAP1                      \t " << CAP1SpinBox->value() << endl;
		fout << "\t TDAC                      \t " << TDACSpinBox->value() << endl;
		fout << "\t FDAC                      \t " << FDACSpinBox->value() << endl;
		fout << "\t ENABLE                    \t " << EnableSpinBox->value() << endl;
		fout << "\t DIGINJ                    \t " << DigInjSpinBox->value() << endl;
		fout << "(take care, this might not be correct. Some masks might be shifted or loaded from file!)" << endl;
		fout.close();
}

void MainForm::writeBinnedHitHisto(int stepquant, QString filename)
{
	std::ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif
	fout.open(qPrintable(filename));
	int hitquant = 0;

	for (int step = 0; step < stepquant; step++)
	{
		for (int col = 0; col < 80; col++)
		{
			for (int row = 0; row < 336; row++)
			{
				myUSBpix->GetConfHisto(col, row, step, hitquant, chipID);
				fout << "0" << "\t" << row+1 << "\t" << col+1 << "\t" << step << "\t" << "0" << "\t" << hitquant << endl;
			}
		}
	}
}
//
//void MainForm::writeBinnedTOTHisto(int stepquant, int ScanStartVal, int ScanStepSize, QString filename)
//{
//	std::ofstream fout;
//	time_t ltime;
//	char timebuf[26];
//
//	time(&ltime);
//#ifndef CF__LINUX
//	ctime_s(timebuf, 26, &ltime);
//#else
//	ctime_r(&ltime, timebuf);
//#endif
//	fout.open(qPrintable(filename));
//	
//	int col, row, hitquant, vcal;
//	for (int step = 1; step < stepquant; step++)
//	{
//		vcal = ScanStartVal + ((step-1) * ScanStepSize);
//		for (int pixnr = 1; pixnr <= 2880; pixnr++)
//		{
//			PixHelp->PixNrToColRow(pixnr, row, col);
//			myUSBpix->GetTOTHistoValue(col, row, step, hitquant);
//			fout << "0" << "\t" << row << "\t" << col << "\t" << vcal << "\t" << "0" << "\t" << hitquant << std::endl;
//		}
//	}
//	fout.close();
//}
//
//void MainForm::writeSFit(int stepquant, int ScanStartVal, int ScanStepSize)
//{
//	std::ofstream fout;
//	time_t ltime;
//	char timebuf[26];
//
//	time(&ltime);
//#ifndef CF__LINUX
//	ctime_s(timebuf, 26, &ltime);
//#else
//	ctime_r(&ltime, timebuf);
//#endif
//	if(getenv("USBPIXI4DATA") != NULL)
//	{
//		QString filename;
//		filename = QString(getenv("USBPIXI4DATA"))+"/sfit.out";
//		fout.open(qPrintable(filename));
//	}
//	else
//	{
//		fout.open("sfit.out");
//	}
//	int col, row;
//	for (int pixnr = 1; pixnr <= 2880; pixnr++)
//	{
//		PixHelp->PixNrToColRow(pixnr, row, col);
//		fout << "0" << "\t" << col << "\t" << row << "\t" << "0" << "\t" << thresvector[pixnr-1] << "\t" << sigmavector[pixnr-1] << "\t" << chi2vector[pixnr-1] << "\t" << "0" << std::endl;
//	}
//	fout.close();
//}

void MainForm::fit_SCurve(int quantity,/* float chicut, float xmin, float xmax,*/ int pixnr)
{
	int npar = Fit->getNPar((int)9)+4*(int)Fit->hasVCAL((int)10);

	double *x, *y, *xerr, *yerr, *pars;
	bool *pfix;
	x    = new double[quantity];
	y    = new double[quantity];
	xerr = new double[quantity];
	yerr = new double[quantity];
	pars = new double[npar];
	pfix = new bool[npar];

	for(int k = 0; k < quantity; k++)
	{
		x[k]    = (double)x_val[k];
		xerr[k] = 1e-5*(double)x_val[k];
	}

	double ymax = InjectionQuantitySlider->value();
	for(int k = 0; k < quantity; k++)
	{
		y[k]    = y_val[k];
		yerr[k] = y[k]/ymax*(ymax-y[k]); // binomial
	}
	pars[0] = 4000; // Schwelle
	pars[1] = 200; // Rauschen
	pars[2] = 100; // Plateau
	pars[3] = 0;
	pars[4] = 1.98 * InjCapDoubleSpinBox->value() / 0.16; // und par[4] ist VCAL-Steigung(mV)*Clow(fF)/0.16
	pars[5] = 0;
	pars[6] = 0;

	pfix[0] = false;
	pfix[1] = false;
	pfix[2] = false;
	pfix[3] = true;
	pfix[4] = true;
	pfix[5] = true;
	pfix[6] = true;


	// run fit
	float chi;
	chi = (float)Fit->runFit(quantity, x, y, xerr, yerr, pars, pfix, (int)10, 0, 0, true); 
	double thDAC = pars[0] / (1.98 * InjCapDoubleSpinBox->value() / 0.16);
	double siDAC = pars[1] / (1.98 * InjCapDoubleSpinBox->value() / 0.16);
	thresvector[pixnr-1] = pars[0];
	sigmavector[pixnr-1] = pars[1];
	chi2vector[pixnr-1] = chi;


	if (pixnr == PixNrShowSpinBox->value())
	{
		int thresh = pars[0];
		int sigma = pars[1];
		emit THRESHOLD(thresh);
		emit SIGMA(sigma);
		emit THRESHOLDDAC(thDAC);
		emit SIGMADAC(siDAC);

		// build Fit-Curve and plot
		double *x_fit, *y_fit;
		x_fit = new double [1024];
		y_fit = new double [1024];
		for (int i = 0; i < 1024; i++)
		{
			x_fit[i] = i;
			y_fit[i] = pars[2] * erfc(thDAC, siDAC, x_fit[i]);
		}


		FitCurve = new QwtPlotCurve("");
		FitCurve->setRawSamples(x_fit, x_fit, 1023);

		SCurve = new QwtPlotCurve("");
		SCurve->setRawSamples(x_fit, y_fit, 1024);
		SCurve->attach(ScanPlot);
		ScanPlot->replot();

		delete [] x_fit;
		delete [] y_fit;
	}

	delete [] x;
	delete [] y;
	delete [] xerr;
	delete [] yerr;
	delete [] pars;
	delete [] pfix;
}

void MainForm::plot_dispersions()
{
	ThresDispPlot->detachItems();
	NoiseDispPlot->detachItems();

	QVector<int> thresdiff(26880);
	thresdiff.fill(0);
	//QVector<double> thresh(600);
	QVector<double> thresh(2000);
	thresh.fill(0);
	//QVector<double> x_val_th(600);
	QVector<double> x_val_th(2000);
	x_val_th.fill(0);
	QVector<double> sigma(400);
	sigma.fill(0);
	QVector<double> x_val_si(400);
	x_val_si.fill(0);

	for (int i = 0; i < 26880; i++)
		thresdiff[i] = thresvector[i] / 20;

	for (int i = 0; i < 2000; i++)
	{
		thresh[i] = thresdiff.count(i);
		x_val_th[i] = (i * 20) + 10;
	}
	for (int i = 0; i < 400; i++)
	{
		sigma[i] = sigmavector.count(i);
		x_val_si[i] = i;
	}

	ThresDispCurve = new QwtPlotCurve("");
	ThresDispCurve->setRawSamples(x_val_th.data(), thresh.data(), 2000);
	ThresDispCurve->attach(ThresDispPlot);
	ThresDispPlot->replot();	

	NoiseDispCurve = new QwtPlotCurve("");
	NoiseDispCurve->setRawSamples(x_val_si.data(), sigma.data(), 400);
	NoiseDispCurve->attach(NoiseDispPlot);
	NoiseDispPlot->replot();

		// calc mean threshold
		int nr_good_fits = 0;
		double mean_threshold = 0;
		double std_dev_threshold = 0;
		for (int i = 0; i < thresvector.size(); i++)
		{
			if (thresvector[i] != 0)
			{
				mean_threshold = mean_threshold + thresvector[i];
				nr_good_fits++;
			}
		}
		mean_threshold = mean_threshold / nr_good_fits;

		// calc std dev. of thresholda
		for (int i = 0; i < thresvector.size(); i++)
		{
			if (thresvector[i] != 0)
				std_dev_threshold = (thresvector[i] - mean_threshold)*(thresvector[i] - mean_threshold);
		}
		std_dev_threshold = std_dev_threshold / nr_good_fits;
		std_dev_threshold = sqrt(std_dev_threshold);

		emit THRESHOLD((int)mean_threshold);
		emit SIGMA((int)std_dev_threshold);
		QApplication::processEvents();
}

void MainForm::refresh_DCS()
{
	if(AdapterFlavorComboBox->currentIndex() != 0)
	{
		USBPixBIDCS * testptr = NULL;
		testptr = dynamic_cast<USBPixBIDCS *>(dcsInst);
		if (testptr != NULL)
		{
			testptr->SetCurrentLimit(1.0);
		}
	}
	dcsInst->PSU[VDDA1]->SetVoltage(VsetA1Box->value());
	dcsInst->PSU[VDDA2]->SetVoltage(VsetA2Box->value());
	dcsInst->PSU[VDDD1]->SetVoltage(VsetD1Box->value());
	dcsInst->PSU[VDDD2]->SetVoltage(VsetD2Box->value());

	if(hUSB->HandlePresent()) dcsInst->UpdateMeasurements();  

	vdda1VLCD->display(QString::number(dcsInst->PSU[VDDA1]->GetVoltage(),'f',3));
	vdda2VLCD->display(QString::number(dcsInst->PSU[VDDA2]->GetVoltage(),'f',3));
	vddd1VLCD->display(QString::number(dcsInst->PSU[VDDD1]->GetVoltage(),'f',3));
	vddd2VLCD->display(QString::number(dcsInst->PSU[VDDD2]->GetVoltage(),'f',3));

	vdda1CLCD->display(QString::number(dcsInst->PSU[VDDA1]->GetCurrent(),'f',3));
	vdda2CLCD->display(QString::number(dcsInst->PSU[VDDA2]->GetCurrent(),'f',3));
	vddd1CLCD->display(QString::number(dcsInst->PSU[VDDD1]->GetCurrent(),'f',3));
	vddd2CLCD->display(QString::number(dcsInst->PSU[VDDD2]->GetCurrent(),'f',3));

	ntcLCD->display(QString::number(dcsInst->GetNTCTemperature(),'f',1));
}
//
//
//
//// Noise Occupancy Measurement Tab
//void MainForm::start_noiseOcMeasurement()
//{	
//	myUSBpix->WriteRegister(CS_CONFIGURATION_NR, 1);
//	if (writeHitHistoCheckBox->isChecked())
//		myUSBpix->SetCalibrationMode();
//	if (writeTOTHistoCheckBox->isChecked())
//		myUSBpix->SetTOTMode();
//
//	TDACdecr = 0;
//
//	// schalte Noisy Pixel ab...
//	int PixNr, PixAdd, PixSize;
//	int PixIndex;
//	int PixValue;
//
//	// kill ganged / interganged pixel, if selected
//	for(int column = 0; column<18; column++)
//	{
//		if(killGangedCheckBox->isChecked())
//		{
//			for (int rowganged = 153; rowganged<160; rowganged = rowganged+2)
//			{
//				int mask;
//				PixHelp->PixColRowToPixNr(column, rowganged, PixNr);
//				PixHelp->PixNrToArray(PixNr, PixIndex, mask);
//				myUSBpix->GetPixelVarAddVal(PixIndex, PixAdd, PixSize, PixValue, HITBUS);
//				PixValue = (~mask) & PixValue;
//				myUSBpix->SetPixelVal(PixIndex, PixValue, HITBUS);
//				myUSBpix->GetPixelVarAddVal(PixIndex, PixAdd, PixSize, PixValue, ENABLE);
//				PixValue = (~mask) & PixValue;
//				myUSBpix->SetPixelVal(PixIndex, PixValue, ENABLE);
//			}
//		}
//		if(killIntergangedCheckBox->isChecked())
//		{
//			for (int rowinterganged = 152; rowinterganged<159; rowinterganged = rowinterganged+2)
//			{
//				int mask;
//				PixHelp->PixColRowToPixNr(column, rowinterganged, PixNr);
//				PixHelp->PixNrToArray(PixNr, PixIndex, mask);
//				myUSBpix->GetPixelVarAddVal(PixIndex, PixAdd, PixSize, PixValue, HITBUS);
//				PixValue = (~mask) & PixValue;
//				myUSBpix->SetPixelVal(PixIndex, PixValue, HITBUS);
//				myUSBpix->GetPixelVarAddVal(PixIndex, PixAdd, PixSize, PixValue, ENABLE);
//				PixValue = (~mask) & PixValue;
//				myUSBpix->SetPixelVal(PixIndex, PixValue, ENABLE);
//			}
//		}
//	}
//
//	// HITBUS Long pixel if selected
//	if(killLongCheckBox->isChecked())
//	{
//		for (int longcol=0; longcol < 18; longcol = longcol + 17)
//		{
//			for(int longrow = 0; longrow<160; longrow++)
//			{
//				int mask;
//				PixHelp->PixColRowToPixNr(longcol, longrow, PixNr);
//				PixHelp->PixNrToArray(PixNr, PixIndex, mask);
//				myUSBpix->GetPixelVarAddVal(PixIndex, PixAdd, PixSize, PixValue, HITBUS);
//				PixValue = (~mask) & PixValue;
//				myUSBpix->SetPixelVal(PixIndex, PixValue, HITBUS);
//				myUSBpix->GetPixelVarAddVal(PixIndex, PixAdd, PixSize, PixValue, ENABLE);
//				PixValue = (~mask) & PixValue;
//				myUSBpix->SetPixelVal(PixIndex, PixValue, ENABLE);
//			}
//		}
//	}
//	myUSBpix->WritePixel(FE_WRITE_HITBUS, HITBUS);
//	myUSBpix->WritePixel(FE_WRITE_ENABLE, ENABLE);
//
//time_t ltime;
//char timebuf[26];
//
//time(&ltime);
//#ifndef CF__LINUX
//ctime_s(timebuf, 26, &ltime);
//#else
//ctime_r(&ltime, timebuf);
//#endif
//arrayout.open("./data/NoiseOccArray.dat");
//
//arrayout << "#" << std::endl;
//arrayout << "# time measured in million BX: " << NoOcTimeSpinBox->value() << std::endl;
//arrayout << "# Nr TDAC decrements: " << TDACdecrSpinBox->value() << std::endl;
//arrayout << "#" << std::endl;
//arrayout << "# last change: " << std::string(timebuf) << std::endl;
//arrayout << "# file format: " << "TDAC decr" << "\t" << "Nr. Hits" << std::endl;
//arrayout << std::endl;
//
//
//// beginne mit Messung
//NoiseOccRep = 0;
//if (NoOcNrMeasSpinBox->value() == 1)
//	NoiseOccProgressBar->setRange(0, 0);
//else
//	NoiseOccProgressBar->setRange(0, NoOcNrMeasSpinBox->value());
//NoiseOccProgressBar->setValue(0);
//TDACdecrProgressBar->setRange(0, TDACdecrSpinBox->value());
//TDACdecrProgressBar->setValue(TDACdecr);
//TDACdecrProgressBar->show();
//NoiseOccProgressBar->show();
//QApplication::processEvents();
//
//NoiseOccupancyMeasurementLoop();
//}
//
//void MainForm::NoiseOccupancyMeasurementLoop()
//{
//	if(NoiseOccRep < NoOcNrMeasSpinBox->value())
//	{
//		measureNoiseOcc();
//		return ;
//	}
//	else
//	{
//		QString filename = "./data/NoiseOcc_" + QString::number(TDACdecr) + ".dat";
//		writeNoiseOccFile(qPrintable(filename));
//		NoiseOccRep = 0;
//		TDACdecr++;
//
//		if(TDACdecr <= TDACdecrSpinBox->value())
//		{
//			NoiseOccRep = 0;
//			TDACdecrProgressBar->setValue(TDACdecr);
//			QApplication::processEvents();
//			decrementAllTDACs(TDACdecr);
//			if (writeDecrementedTDACFilesCheckBox->isChecked())
//			{
//				QString TDACfilename = "./data/TDAC_decrement_" + QString::number(TDACdecr) + ".out";		// disable, if write tdac-decr files
//				write_TDACFile(qPrintable(TDACfilename));													// disable, if write tdac-decr files
//			}
//			else
//				write_TDACFile("./data/TDAC_decrement.out");												// disable, if write tdac-decr files
//			NoiseOccupancyMeasurementLoop();
//		}
//		else
//		{
//			TDACdecrProgressBar->hide();
//			NoiseOccProgressBar->hide();
//			QApplication::processEvents();
//			arrayout.close();
//			myUSBpix->SetRunMode();
//		}
//		return ;
//	}
//}
//
//void MainForm::measureNoiseOcc()
//{
//	NoiseOccRep++;
//	NoiseOccProgressBar->setValue(NoiseOccRep);
//	QApplication::processEvents();
//
//	int time = NoOcTimeSpinBox->value() * 25;
//	myUSBpix->WriteRegister(30, 1);	//reset counter
//	QTimer *timer = new QTimer(this);
//	connect(timer, SIGNAL(timeout()), this, SLOT(stop_noiseOccMeasurement()));
//	timer->setSingleShot(true);
//	myUSBpix->ClearSRAM();
//	myUSBpix->WriteRegister(CS_ENABLE_NO_OC, 1); //start measurement
//	timer->start(time);	//start timeout
//}
//
//void MainForm::stop_noiseOccMeasurement()
//{
//	myUSBpix->WriteRegister(CS_ENABLE_NO_OC, 0); //stop measurement
//	myUSBpix->ReadSRAM();
//	if(writeHitHistoCheckBox->isChecked())
//	{
//		if(getenv("USBPIXI4DATA") != NULL)
//		{
//			QString filename;
//			filename = QString(getenv("USBPIXI4DATA"))+"/Hitmap" + QString::number(TDACdecr) + ".bin";
//			writeBinnedHitHisto(10, 0, 1, filename);
//		}
//		else
//		{
//			writeBinnedHitHisto(10, 0, 1, "data/Hitmap"+QString::number(TDACdecr)+".bin");
//		}
//	}
//	if(writeTOTHistoCheckBox->isChecked())
//	{
//		if(getenv("USBPIXI4DATA") != NULL)
//		{
//			QString filename;
//			filename = QString(getenv("USBPIXI4DATA"))+"/TOTmap" + QString::number(TDACdecr) + ".bin";
//			writeBinnedTOTHisto(255, 0, 1, filename);
//		}
//		else
//		{
//			writeBinnedTOTHisto(255, 0, 1,  "data/TOTmap"+QString::number(TDACdecr)+".bin");
//		}
//	}
//	int byteRB[4];
//	byteRB[0] = myUSBpix->ReadRegister(25);
//	byteRB[1] = myUSBpix->ReadRegister(26);
//	byteRB[2] = myUSBpix->ReadRegister(27);
//	byteRB[3] = myUSBpix->ReadRegister(28);
//	NoiseOccArray[NoiseOccRep-1] = (byteRB[3] << 24) + (byteRB[2] << 16) + (byteRB[1] << 8) + byteRB[0];
//
//	emit NOOCNRHITS(NoiseOccArray[NoiseOccRep-1]);
//	QApplication::processEvents();
//	arrayout << /*TDACdecr << "\t" << */NoiseOccArray[NoiseOccRep-1] << std::endl;
//	start_ScanMuC();
//	write_Select();
//	NoiseOccupancyMeasurementLoop();
//}
//
//
//void MainForm::writeNoiseOccFile(const char * filename)
//{
//	std::ofstream fout;
//	time_t ltime;
//	char timebuf[26];
//
//	time(&ltime);
//#ifndef CF__LINUX
//	ctime_s(timebuf, 26, &ltime);
//#else
//	ctime_r(&ltime, timebuf);
//#endif
//
//
//		fout.open(filename);
//
//	fout << "#" << std::endl;
//	fout << "# time measured in million BX: " << NoOcTimeSpinBox->value() << std::endl;
//	fout << "# Nr of repetitions: " << NoiseOccRep << std::endl;
//	fout << "#" << std::endl;
//	fout << "# last change: " << std::string(timebuf) << std::endl;
//	fout << std::endl;
//
//
//	for(int rep = 0; rep < NoiseOccRep; rep++)
//	{
//		fout << /*"repetition " << "\t" << rep + 1 << "\t \t \t" << "nr of noise hits " << "\t" <<  */NoiseOccArray[rep] << std::endl;
//	}	
//	fout.close();
//}
//
//void MainForm::decrementAllTDACs(int steps)
//{
//	//file to read
//	QFile fileread(TDACFileName);															// disable, if write tdac-decr files
//	//QFile fileread("./data/tuning_1900_tdacs.out");										// enable, if write tdac-decr files
//	if (!fileread.open(QIODevice::ReadOnly | QIODevice::Text))
//		return;
//
//	QTextStream in(&fileread);
//	QString line;
//
//
//	//file to write
//	QString TDACfilenameDecremented;
//	if (writeDecrementedTDACFilesCheckBox->isChecked())
//		TDACfilenameDecremented = "./data/TDAC_decrement_" + QString::number(TDACdecr) + ".out";	// enable, if write tdac-decr files
//	else
//		TDACfilenameDecremented = "./data/TDAC_decrement.out";		
//
//	QFile filewrite(qPrintable(TDACfilenameDecremented));
//
//	if (!filewrite.open(QIODevice::WriteOnly | QIODevice::Text))
//		return;
//
//	QTextStream out(&filewrite);
//
//	int newval;
//	QStringList list;
//	for (int row = 0; row < 160; row++)
//	{
//		// lese Zeile in Datei
//		line = in.readLine();
//		list = line.split(QRegExp("\\s+"));
//
//
//		for (int i = 0; i < 18; i++)
//		{
//			newval = list[i].toInt() - steps;
//			if(newval < 0 || newval > 127)
//				newval = 0;
//			list[i] = QString::number(newval, 10);
//		}
//			out << list.join(" ") << endl;
//	}
//	fileread.close();
//	filewrite.close();
//
//}
//
//void MainForm::set_writeTOTHistoCheckBox()
//{
//	if (writeHitHistoCheckBox->isChecked())
//		writeTOTHistoCheckBox->setChecked(false);
//}
//
//void MainForm::set_writeHitHistoCheckBox()
//{
//	if (writeTOTHistoCheckBox->isChecked())
//		writeHitHistoCheckBox->setChecked(false);
//	QApplication::processEvents();
//}
//
//
//






// ******************* Erfc-definition, used for plotting S-curves *********************
// This will still be needed in FE-I4 code 
//---------------------------------------------------------------------------
//
// Algorithm taken from the book __Numerical Recipes__   page 181, erfcc().
//
// We use here
// Erf(x) = 1 / sqrt(pi) \int_-\infinity^x exp(-t*t) dt
// and offset and scale fore Mean and sigma.
//
//---------------------------------------------------------------------------
void MainForm::start_WordErrorRateTest()
{
	myUSBpix->SetRunMode();
	myUSBpix->ClearSRAM(chipID);
	myUSBpix->resetRunModeAdd();
	RateStatus->start();
	while (RateStatus->isRunning())
	{
		QApplication::processEvents();
	}
}

void MainForm::stop_WordErrorRateTest()
{
	RateStatus->terminate();
}



const double ERFCC_1  = -1.26551223;
const double ERFCC_2  =  1.00002368;
const double ERFCC_3  =  0.37409196;
const double ERFCC_4  =  0.09678418;
const double ERFCC_5  = -0.18628806;
const double ERFCC_6  =  0.27886807;
const double ERFCC_7  = -1.13520398;
const double ERFCC_8  =  1.48851587;
const double ERFCC_9  = -0.82215223;
const double ERFCC_10 =  0.17087277;

double erfc(double Mean, double Sigma, double x)
{
	double t, xrel, z, ans;

	if (Sigma == 0) {
		xrel = x-Mean;
		ans = 0;
	} else {
		xrel = (x-Mean) / M_SQRT2 / Sigma;
		z    = fabs(xrel);
		t    = 1.0 / (1.0 + 0.5 * z );    // the division cannot fail!
		ans  = t*exp(-z*z+ERFCC_1+t*(ERFCC_2+t*(ERFCC_3+t*(ERFCC_4+t*(ERFCC_5+t*
			(ERFCC_6+t*(ERFCC_7+t* (ERFCC_8+t*(ERFCC_9+t*ERFCC_10))))))))) / 2.0;
	}

	if (xrel > 0)
		return 1.0-ans;
	else if (xrel < 0)
		return ans;
	else
		return 0.5;
} 

//Start additions by Stewart
//*************************************************************  NEW TAB - ABSG ******************************************************
int zzmulticommand=0; //turns on/off "overwrite" in the bit stream line
int edited=0; //prevents the bit stream line from raising an "edited" flag when it is set to 1
//bool k=false; 


int MainForm::BintoDec(QString string) // converts binary to decimal and reverses the number (i.e. 1100 (binary) -> 3)
{
	QString tempstring;
	bool ok;
	double saver=0;
	double number=0;
	for(int i=0;i<string.length();i++)
	{
		tempstring=string.at(i);
		saver=tempstring.toInt(&ok,10);
		if(i==0 && saver==0)
			number=number+0;
		else
			number=number+pow((2*saver),i);
	}
	return int (number);
}


void MainForm::zzSendtoChip()
{
	QString bitStreamString=zzStreamLineEdit->text();
	QString tempstring;
	while(bitStreamString.indexOf(" ")!=-1) //gets rid of any spaces
		bitStreamString.remove(bitStreamString.indexOf(" "),1);

	for(int i=1;i<bitStreamString.length();i++)//checks that everything's a 1 or 0.
	{
		if("1"!=bitStreamString.at(i) && "0"!=bitStreamString.at(i))
		{
			QMessageBox::warning(this, tr("Invalid character! Stream not sent!"),bitStreamString.at(i), QMessageBox::Ok);
			zzCommandLog->append("-- error: invalid character-- ");
			zzCommandLog->append(bitStreamString.at(i));
			return;
		}
	}
	// Will now check to see if its a trigger command being sent. If it is, then the answer will be appended to the log file.
	bool is_LV1_trigger = false;
	if(bitStreamString == "11101"){ is_LV1_trigger = true; zzClearSRAM(); zzCommandLog->append("Issued LV1, cleared SRAM.");}

	bitStreamString.prepend("0"); //all sent streams must have a 0 added in front of them
	QStringList bitstring;
	int divisor=bitStreamString.length()/8;
	if(bitStreamString.length()%8!=0)
		divisor++; //divisor now represents how many 8 bit rows are needed in an array to hold the bitStreamString
	tempstring="";
	for(int i=0;i<divisor;i++)//divides bitStreamString into strings of 8 characters stored in bitstring
	{
		for(int j=i*8;j<(i*8+8);j++)
		{
			if(bitStreamString.length()>j)
			{
				tempstring.append(bitStreamString.at(j));
			}
			else
				tempstring.append("0");
		}
		bitstring.append(tempstring);
		tempstring="";
	}
	bool ok;

	QStringList data;
	for(int i=0;i<divisor;i++)//converts the binary to decimal (simultaneously reversing the number) and decimal into hexadecimal
	{
		tempstring=QString("0x%1").arg(BintoDec(bitstring.value(i)), 0, 16);
		if(tempstring.length()!=4)
			tempstring.insert(2,"0");//prevents 0x05 from being written as 0x5
		data.append(tempstring);
	}
	unsigned char* bitstream=NULL;
	bitstream=new unsigned char[divisor];//this syntax allows the array to be dynamically declared
	for(int i=0;i<divisor;i++)//fills the dynamic array bitstream with the characters from the data array
		bitstream[i]=data[i].toUShort(&ok,16);
	myUSBpix->SendBitstream(bitstream, bitStreamString.length(), chipID);//sends out the bitstream!
	//tempstring=bitstring[0];

	if(ok==false)
		QMessageBox::warning(this, tr("unable to convert to hex"),"internal error", QMessageBox::Ok); //for debugging
	QTime t=QTime::currentTime();
	QString entry="-- ";
	entry.append(t.toString());
	entry.append(", sent bit stream to chip: --");

	zzCommandLog->append(entry);
	zzCommandLog->append(zzStreamLineEdit->text());

	// Now parse SRAM if it was a trigger command
	if(is_LV1_trigger){
		pget_LV1data(0);
	}
	return;
}

void MainForm::zzWriteRegister()
{
	if (zzmulticommand==1) //if the multicommand checkbox is checked, this adds the command to the bitstream rather than replacing the bitstream
		zzStreamLineEdit->insert("   10110   1000   0010   ");
	else
		zzStreamLineEdit->setText("   10110   1000   0010   ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());//chipID
	zzStreamLineEdit->insert(" ");
	QString RegNumber;
	bool ok;
	int tempint=(zzRegNumLineEdit->text()).toInt(&ok, 10);//register address
	if(ok==false)
	{
		if(""==zzRdRegNumLineEdit->text())
			QMessageBox::warning(this, tr("Warning - No reg number entered!"),zzRdRegNumLineEdit->text(), QMessageBox::Ok);
		else
			QMessageBox::warning(this, tr("Could not convert to binary"),zzRdRegNumLineEdit->text(), QMessageBox::Ok);
	}
	RegNumber.setNum(tempint, 2);
	int y=RegNumber.length();
	if(y<6)//makes sure the address is 6 bits long - if its too short, 0's are added. If it's too long, a warning pops up
	{
		for(int l=0;l<(6-y);l++)
			RegNumber.insert(0,"0");
	}
	else if (y>6)
	{
		QMessageBox::warning(this, tr("Address out of range"),RegNumber, QMessageBox::Ok);
	}
	zzStreamLineEdit->insert(RegNumber);
	zzStreamLineEdit->insert("     ");
	edited=1;
	zzStreamLineEdit->insert(zzRegWriteLineEdit->text());
	RegNumber=("-- Write Register ");
	RegNumber.append(zzRegNumLineEdit->text());
	RegNumber.append(" --");
	zzCommandLog->append(RegNumber);
	zzCommandLog->append(zzStreamLineEdit->text());

	return;
}

void MainForm::zzLevel1Trigger()
{
	edited=1;
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   11101");
	else
		zzStreamLineEdit->setText("   11101");
	zzCommandLog->append("-- Level 1 Trigger --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzBunchCounterReset()
{
	edited=1;
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110   0001");
	else
		zzStreamLineEdit->setText("   10110   0001");
	zzCommandLog->append("-- Bunch Counter Reset --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzEventCounterReset()
{
	edited=1;
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110   0010");
	else
		zzStreamLineEdit->setText("   10110   0010");
	zzCommandLog->append("-- Event Counter Reset --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzCalibrationPulse()
{
	edited=1;
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110   0100");
	else
		zzStreamLineEdit->setText("   10110   0100");
	zzCommandLog->append("-- Calibration Pulse --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzGblPulse()
{
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("10110 1000 1001 ");
	else
		zzStreamLineEdit->setText("10110 1000 1001 ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("   ");
	QString PulseWidth;
	bool ok;
	int tempint=(zzGlobalPulseLineEdit->text()).toInt(&ok, 10);
	if(ok==false)
	{
		if(""==zzRdRegNumLineEdit->text())
			QMessageBox::warning(this, tr("Warning - No width entered!"),zzRdRegNumLineEdit->text(), QMessageBox::Ok);
		else
			QMessageBox::warning(this, tr("Could not convert to binary"),zzRdRegNumLineEdit->text(), QMessageBox::Ok);
	}
	PulseWidth.setNum(tempint, 2);
	int y=PulseWidth.length();
	if(y<6)
	{
		for(int l=0;l<(6-y);l++)
			PulseWidth.insert(0,"0");
	}
	else if (y>6)
	{
		QMessageBox::warning(this, tr("Width out of range"),PulseWidth, QMessageBox::Ok);
	}
	edited=1;
	zzStreamLineEdit->insert(PulseWidth);
	PulseWidth="-- Global Pulse, width ";
	PulseWidth.append(zzGlobalPulseLineEdit->text());
	PulseWidth.append(" --");
	zzCommandLog->append(PulseWidth);
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzGblReset()
{
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("10110 1000 1000 ");
	else
		zzStreamLineEdit->setText("10110 1000 1000 ");
	zzStreamLineEdit->insert("   ");
	edited=1;
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzCommandLog->append("-- Global Reset --");
	return;
}

void MainForm::zzReadRegister()
{
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110   1000   0001   ");
	else
		zzStreamLineEdit->setText("   10110   1000   0001   ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("   ");
	QString RegNumber;
	bool ok;
	int tempint=(zzRdRegNumLineEdit->text()).toInt(&ok, 10);

	//myUSBpix->ClearSRAM();
	myUSBpix->SetRunMode();
	myUSBpix->resetRunModeAdd();

	if(ok==false)
	{
		if(""==zzRdRegNumLineEdit->text())
			QMessageBox::warning(this, tr("Warning - No reg number entered!"),zzRdRegNumLineEdit->text(), QMessageBox::Ok);
		else
			QMessageBox::warning(this, tr("Could not convert to binary"),zzRdRegNumLineEdit->text(), QMessageBox::Ok);
	}
	RegNumber.setNum(tempint, 2);
	int y=RegNumber.length();
	if(y<6)
	{
		for(int l=0;l<(6-y);l++)
			RegNumber.insert(0,"0");
	}
	else if (y>6)
	{
		QMessageBox::warning(this, tr("Address out of range"),RegNumber, QMessageBox::Ok);
	}
	zzStreamLineEdit->insert(RegNumber);
	edited=1;
	zzStreamLineEdit->insert("   ");
	RegNumber=("-- Read Register ");
	RegNumber.append(zzRdRegNumLineEdit->text());
	RegNumber.append(" --");
	zzCommandLog->append(RegNumber);
	zzCommandLog->append(zzStreamLineEdit->text());

	return;
}
void MainForm::zzWriteFrontEnd()
{
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110   1000   0100   ");
	else
		zzStreamLineEdit->setText("   10110   1000   0100   ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("   000000   ");
	edited=1;
	zzStreamLineEdit->insert(zzWrFrontEndLineEdit->text());
	zzCommandLog->append("-- Write Front End --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}
void MainForm::zzCheckChipIDLength()
{
	QString ChipID=zzChipIDLineEdit->text();
	int length=ChipID.length();
	if(length<4)
	{
		QMessageBox::warning(this, tr("ChipID must be 4 bits"),ChipID, QMessageBox::Ok);
		for(int l=0;l<(4-length);l++)
			ChipID.insert(0,"0");
	}
	else if (length>4)
	{
		QMessageBox::warning(this, tr("ChipID must be 4 bits"),ChipID, QMessageBox::Ok);
		for(int l=0;l<(length-4);l++)
			ChipID.remove(0,1);

	}
	zzChipIDLineEdit->setText(ChipID);
	return;
}

void MainForm::zzCheckRegWriteLength()
{
	QString WrReg=zzRegWriteLineEdit->text();
	if(WrReg.isEmpty())
		return;
	int length=WrReg.length();
	if(length<16)
	{
		QMessageBox::warning(this, tr("Register data must be 16 bits"),WrReg, QMessageBox::Ok);
		for(int l=0;l<(16-length);l++)
			WrReg.insert(0,"0");
	}
	else if (length>16)
	{
		QMessageBox::warning(this, tr("Register data must be 16 bits"),WrReg, QMessageBox::Ok);
		for(int l=0;l<(length-16);l++)
			WrReg.remove(1,0);

	}
	zzRegWriteLineEdit->setText(WrReg);
	return;
}
void MainForm::zzCheckWrFELength()
{
	QString WrReg=zzWrFrontEndLineEdit->text();
	if(WrReg.isEmpty())
		return;
	int length=WrReg.length();
	if(length<672)
	{
		QMessageBox::warning(this, tr("FE data must be 672 bits"),WrReg, QMessageBox::Ok);
		for(int l=0;l<(672-length);l++)
			WrReg.insert(0,"0");
	}
	else if (length>672)
	{
		QMessageBox::warning(this, tr("FE data must be 672 bits"),WrReg, QMessageBox::Ok);
		for(int l=0;l<(length-672);l++)
			WrReg.remove(1,0);

	}
	zzWrFrontEndLineEdit->setText(WrReg);
	return;
}

void MainForm::zzClearData_Reg()
{
	zzRegWriteLineEdit->setText("");
	return;
}

void MainForm::zzClearData_FE()
{
	zzWrFrontEndLineEdit->setText("");
	return;
}

void MainForm::zzClearData_Pattern()
{
	zzFEPatternLineEdit->setText("");
	return;
}

void MainForm::zzClearData_BitStream()
{
	edited=1;
	zzStreamLineEdit->setText(" ");
	zzCommandLog->append("-- Bit Stream Line Cleared --");
	return;
}

void MainForm::zzCopyPattern()
{
	QString Pattern=zzFEPatternLineEdit->text();
	if(Pattern.isEmpty())
		return;
	QString FEData;
	while(FEData.length()<672)
		FEData.append(Pattern);
	int remainder=(FEData.length()-672);
	for(int i=0;i<remainder;i++)
		FEData.remove((FEData.length()-1),1);
	zzWrFrontEndLineEdit->setText(FEData);
	return;
}

void MainForm::zzchangeLCD()
{
	QString bitStreamString=zzStreamLineEdit->text();
	while(bitStreamString.indexOf(" ")!=-1)
		bitStreamString.remove(bitStreamString.indexOf(" "),1);

	zzBitStreamLCD->display( bitStreamString.length());
	return;
}

void MainForm::zzSpacer()
{
	edited=1;
	zzStreamLineEdit->insert("   0000000000000000");
	return;
}

void MainForm::zzRunMode()
{
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110 1000 1010   ");
	else
		zzStreamLineEdit->setText("   10110 1000 1010   ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	edited=1;
	zzStreamLineEdit->insert("   111000");
	zzCommandLog->append("-- Run Mode --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzConfigureMode()
{
	if (zzmulticommand==1)
		zzStreamLineEdit->insert("   10110 1000 1010   ");
	else
		zzStreamLineEdit->setText("   10110 1000 1010   ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	edited=1;
	zzStreamLineEdit->insert("   000111");
	zzCommandLog->append("-- Configure Mode --");
	zzCommandLog->append(zzStreamLineEdit->text());
	return;
}

void MainForm::zzchangecolor()
{
	QPalette palette = zzStreamLineEdit->palette();
	if(edited==0)
	{
		palette.setColor(QPalette::Base, QColor(255,255,100));//yellow
		zzEditLabel->show();
	}
	else
	{
		zzEditLabel->hide();
		palette.setColor(QPalette::Base, QColor(255,255,255));//white
	}
	zzStreamLineEdit->setPalette(palette);
	edited=0;
	return;
}


void MainForm::zzviewRegister()
{
	//QString registernumber=zzRdRegNumLineEdit->text();
	//myUSBpix->ReadSRAM();
	//QString tempstring;
	//bool ok;
	//tempstring.setNum(myUSBpix->SRAMwords[0],10);
	//tempstring=QString("%1").arg(tempstring.toInt(&ok,10), 0, 2);
	//zzReadRegFieldsLineEdit->setText(tempstring.section("",0,8));
	//tempstring.remove(0,8);
	//zzCommandLog->append("-- View Data --");
	//zzCommandLog->append(zzReadRegLineEdit->text());
	//unsigned int SRAMdataRB[SRAM_WORDSIZE];
	for (int i = 0; i< SRAM_WORDSIZE; i++)
		SRAMdataRB[i] = 0;

	QString registernumber=zzRdRegNumLineEdit->text();
	myUSBpix->ReadSRAM(chipID); int totalwords = SRAM_WORDSIZE/*myUSBpix->total_SRAM_words*/;
	myUSBpix->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE, chipID);
	for(int cnt=0; cnt<totalwords; cnt++){
		QString tempstring; 
		tempstring.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt],10);
		unsigned theword = tempstring.toUInt();
		unsigned int headercheck = 0xFF0000 & theword;
		if(headercheck==0xec0000){ // this is a value record
			//check that the value record is not preceded by a shift register address record
			bool is_gr_value_record = false;
			if (cnt==0)
				is_gr_value_record = true;
			else {
				QString pre_tempstring; 
				pre_tempstring.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt-1],10);
				unsigned pre_theword = pre_tempstring.toUInt();
				unsigned int pre_headercheck = 0xFF8000 & pre_theword;
				if(pre_headercheck==0xea8000){ // this is a shift register address record header
					is_gr_value_record = false;}
				else {
					is_gr_value_record = true;}
			}
			if (is_gr_value_record) {
				bool ok;
				tempstring=QString("%1").arg(tempstring.toInt(&ok,10), 0, 2);
				zzReadRegFieldsLineEdit->setText(tempstring.section("",0,8));
				tempstring.remove(0,8);
				zzReadRegLineEdit->setText(tempstring);
				break;}
		}

	}
	zzCommandLog->append("-- View Data --");
	zzCommandLog->append(zzReadRegLineEdit->text());
	zzReadRegLineEdit->clear();
}

void MainForm::zzviewShiftRegister()
{
	//unsigned int SRAMdataRB[SRAM_WORDSIZE];
	for (int i = 0; i< SRAM_WORDSIZE; i++)
		SRAMdataRB[i] = 0;
	QString registernumber=zzRdRegNumLineEdit->text();
	myUSBpix->ReadSRAM(chipID); int totalwords = SRAM_WORDSIZE/*myUSBpix->total_SRAM_words*/;
	myUSBpix->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE, chipID);
	int n_vr = 0;
	QString output;
	for(int cnt=0; cnt<totalwords; cnt++){
		QString tempstring; 
		tempstring.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt],10);
		unsigned theword = tempstring.toUInt();
		unsigned int headercheck = 0xFF0000 & theword;
		if(headercheck==0xec0000){ // this is a value record
			//check that the value record is not preceded by a global register address record
			bool is_gr_value_record = false;
			if (cnt==0 || n_vr>0)
				is_gr_value_record = true;
			else {
				QString pre_tempstring; 
				pre_tempstring.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt-1],10);
				unsigned pre_theword = pre_tempstring.toUInt();
				unsigned int pre_headercheck = 0xFF8000 & pre_theword;
				if(pre_headercheck==0xea0000){ // this is a global register address record header
					is_gr_value_record = false;}
				else {
					is_gr_value_record = true;}
			}
			if (is_gr_value_record) {
				n_vr++;
				//bool ok;
				QString valuestring=QString("%1").arg(theword, 0, 2);
				valuestring.remove(0,8);
				output.append(valuestring);
				if (n_vr = 42) //last word of shift register output
					n_vr = 0;
			}
		}

	}
	zzReadRegLineEdit->setText(output);
	zzCommandLog->append("-- View Data --");
	zzCommandLog->append(zzReadRegLineEdit->text());
	zzReadRegLineEdit->clear();
	pDebugBox->append("Shift Register Data");
	pDebugBox->append(output);
	QString db1; db1.setNum(output.length());
	db1.prepend("Length: ");
	pDebugBox->append(db1);

}

void MainForm::zzMultiCommand(int checked)
{
	if(checked==0)
		zzmulticommand=0;
	else
		zzmulticommand=1;
	return;
}
void MainForm::zzSRAMtoFILE()
{
	/*time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif

	myUSBpix->ReadSRAM();
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","/home/SRAM.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite))
	{
		QMessageBox::warning(this, tr("Error"),"Failed to open/create file", QMessageBox::Ok);
	}
	QTextStream outstream(&file);
	outstream << "# Written from ABSG tab, only non zero words written" << endl;
	outstream << "#" << endl;
	//outstream << "# last change: " << std::string(timebuf) << endl;		
	outstream << "#" << endl;
	outstream << "#SRAM_WORDSIZE = " <<SRAM_WORDSIZE<< endl;
	outstream << endl;
	bool ok;
	for (int i=0;i<(SRAM_WORDSIZE);i++){
		if(myUSBpix->SRAMwordsRB[i]!=0)
			outstream<<(myUSBpix->SRAMwordsRB[i])<<endl;
	}
	file.close();*/
    myUSBpix->ReadSRAM(chipID);
    myUSBpix->FileSaveRB("SRAMdata.dat", SRAM_WORDSIZE, false, chipID);



}

void MainForm::zzClearSRAM()
{
	myUSBpix->ClearSRAM(chipID);
}

void MainForm::zzSaveLog()
{
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","/home/Log_1.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite))
	{
		QMessageBox::warning(this, tr("Error in View Register"),"Failed to open/create file", QMessageBox::Ok);
	}
	QTextStream outstream(&file);
	tempstring=zzCommandLog->toPlainText();
	QTime t=QTime::currentTime();
	QDate d=QDate::currentDate();
	tempstring.prepend(outstream.readAll());
	outstream<<tempstring<<endl<<"*** Command Log saved at "<<t.toString()<<" on "<<d.toString()<<" ***"<<endl<<endl;
	file.close();
	zzCommandLog->append("-- File saved to CommandLog.txt--");
}

//*************************************************************  NEW TAB - Pixel Manager ******************************************************
void MainForm::xxChangeColor(QPushButton * button) //Many buttons wind up activating this slot: view buttons, all on/off buttons on the sliders, pixels, and DC.
{												   //The function does a number of things depending on the name of the QPushbutton which is passed to it 
	QString tempstring;
	tempstring=button->text();
	if(tempstring.indexOf("View")!=-1){ //signals from View buttons are directed to the xxChangeTab function
		xxChangeTab(button);
		return;}
	else{
		QString tempstring=button->objectName();
		if(tempstring.indexOf("xxLeftTopOn")!=-1){ //These expressions deal with the on/off buttons for the sliders
			int setting=xxLeftLCDNumber->intValue();
			for(int i=0;i<setting;i++){
				xxUpdateArray(i,1);}} //This function changes the global xxFEarray at index i to the value of the second argument. Here, array elements from 0 to "setting" are changed to 1.
		else if(tempstring.indexOf("xxLeftTopOff")!=-1){
			int setting=xxLeftLCDNumber->intValue();
			for(int i=0;i<setting;i++){
				xxUpdateArray(i,0);}}
		else if(tempstring.indexOf("xxLeftBottomOn")!=-1){
			int setting=xxLeftLCDNumber->intValue();
			for(int i=setting;i<336;i++){
				xxUpdateArray(i,1);}}
		else if(tempstring.indexOf("xxLeftBottomOff")!=-1){
			int setting=xxLeftLCDNumber->intValue();
			for(int i=setting;i<336;i++){
				xxUpdateArray(i,0);}}
		else if(tempstring.indexOf("xxRightTopOn")!=-1){
			int setting=xxRightLCDNumber->intValue();
			for(int i=336;i<setting+336;i++){
				xxUpdateArray(i,1);}}
		else if(tempstring.indexOf("xxRightTopOff")!=-1){
			int setting=xxRightLCDNumber->intValue();
			for(int i=336;i<setting+336;i++){
				xxUpdateArray(i,0);}}
		else if(tempstring.indexOf("xxRightBottomOn")!=-1){
			int setting=xxRightLCDNumber->intValue();
			for(int i=336+setting;i<672;i++){
				xxUpdateArray(i,1);}}
		else if(tempstring.indexOf("xxRightBottomOff")!=-1){
			int setting=xxRightLCDNumber->intValue();
			for(int i=336+setting;i<672;i++){ //This slider covers the last section of the DC - from the position of the slider to pixel 672.
				xxUpdateArray(i,0);}}
		else if(tempstring.indexOf("xxEntireChipSetter")!=-1){ //This sets the bounds for the pattern-filler spinboxes to "0 to 26880"
			xxHighBoundbox->setValue(26880);
			return;}
		else if(tempstring.indexOf("xxCurrentTabSetter")!=-1){ //This sets the bounds for the pattern-filler spinboxes to the bounds of the currentDC
			xxLowBoundbox->setValue(672*CURRENT_DC); //I use 672*CURRENT_DC frequently to shift all of the buttons so they influence the correct section of the array
			xxHighBoundbox->setValue(672+672*CURRENT_DC);
			return;}//return is here so the update flag "xxUpdateAlert" at the end of this function isn't emitted
		else{
			QTextEdit *Pad=xxDirector(xxButtontoInt(button)); //I needed to specify a QTextEdit based on a button. the xxButtontoIntfunction associates each button with an integer (handy elsewhere in for loops). xxDirector then spits out a QTextEdit based on the integer. xxDirector is useful later in for loops
			tempstring=button->objectName();
			if(tempstring.indexOf("Pixel_")!=-1)
				xxpixelswitch=1;//Changes the behavior of xxUpdateArray to change an individual element based on the given index rather than the 21 elements starting with that index
			QPalette palette = Pad->palette();
			tempstring=Pad->toPlainText();
			QColor red(170,99,90,255);
			QColor green(117,170,117,225);
			if(tempstring.indexOf("1")!=-1) //These change switch the color/number in the textedit, effectively making the button a checkbox. The array is also updated.
			{
				palette.setColor(QPalette::Base, QColor(red));;
				Pad->setText("0");
				Pad->setAlignment(Qt::AlignCenter);
				xxUpdateArray(xxButtontoInt(button),0);
				Pad->setPalette(palette);
			}
			else
			{
				palette.setColor(QPalette::Base, green);
				Pad->setText("1");
				Pad->setAlignment(Qt::AlignCenter);
				xxUpdateArray(xxButtontoInt(button),1);
				Pad->setPalette(palette);
			}
		}}
	xxUpdateAlert->show();//highlights the UPDATE button
	//emit xxUpdate(); //any time this is called, all of the visual parts of the program look at the xxFEarray and update their display
}

void MainForm::xxbuttonClicked() //this allows the xxChangeColor function to distinguish which button signaled it.
{
	QPushButton *button = (QPushButton *)sender();
	emit xxemitPad(button);
}


QTextEdit* MainForm::xxDirector(int number) //associates a text edit with an integer. 
{
	if(number==-1)//the buttons on the DC are assigned negative intigers to differentiate them from the individual pixel buttons
		return xxFEPad1;
	if(number==-2)
		return xxFEPad2;
	if(number==-3)
		return xxFEPad3;
	if(number==-4)
		return xxFEPad4;
	if(number==-5)
		return xxFEPad5;
	if(number==-6)
		return xxFEPad6;
	if(number==-7)
		return xxFEPad7;
	if(number==-8)
		return xxFEPad8;
	if(number==-9)
		return xxFEPad9;
	if(number==-10)
		return xxFEPad10;
	if(number==-11)
		return xxFEPad11;
	if(number==-12)
		return xxFEPad12;
	if(number==-13)
		return xxFEPad13;
	if(number==-14)
		return xxFEPad14;
	if(number==-15)
		return xxFEPad15;
	if(number==-16)
		return xxFEPad16;
	if(number==-17)
		return xxFEPad17;
	if(number==-18)
		return xxFEPad18;
	if(number==-19)
		return xxFEPad19;
	if(number==-20)
		return xxFEPad20;
	if(number==-21)
		return xxFEPad21;
	if(number==-22)
		return xxFEPad22;
	if(number==-23)
		return xxFEPad23;
	if(number==-24)
		return xxFEPad24;
	if(number==-25)
		return xxFEPad25;
	if(number==-26)
		return xxFEPad26;
	if(number==-27)
		return xxFEPad27;
	if(number==-28)
		return xxFEPad28;
	if(number==-29)
		return xxFEPad29;
	if(number==-30)
		return xxFEPad30;
	if(number==-31)
		return xxFEPad31;
	if(number==-32)
		return xxFEPad32;
	if(number==0)
		return xxPixelpad_1;
	if(number==1)
		return xxPixelpad_2;
	if(number==2)
		return xxPixelpad_3;
	if(number==3)
		return xxPixelpad_4;
	if(number==4)
		return xxPixelpad_5;
	if(number==5)
		return xxPixelpad_6;
	if(number==6)
		return xxPixelpad_7;
	if(number==7)
		return xxPixelpad_8;
	if(number==8)
		return xxPixelpad_9;
	if(number==9)
		return xxPixelpad_10;
	if(number==10)
		return xxPixelpad_11;
	if(number==11)
		return xxPixelpad_12;
	if(number==12)
		return xxPixelpad_13;
	if(number==13)
		return xxPixelpad_14;
	if(number==14)
		return xxPixelpad_15;
	if(number==15)
		return xxPixelpad_16;
	if(number==16)
		return xxPixelpad_17;
	if(number==17)
		return xxPixelpad_18;
	if(number==18)
		return xxPixelpad_19;
	if(number==19)
		return xxPixelpad_20;
	if(number==20)
		return xxPixelpad_21;
	if(number==21)
		return xxPixelpad_22;
	if(number==22)
		return xxPixelpad_23;
	if(number==23)
		return xxPixelpad_24;
	if(number==24)
		return xxPixelpad_25;
	if(number==25)
		return xxPixelpad_26;
	if(number==26)
		return xxPixelpad_27;
	if(number==27)
		return xxPixelpad_28;
	if(number==28)
		return xxPixelpad_29;
	if(number==29)
		return xxPixelpad_30;
	if(number==30)
		return xxPixelpad_31;
	if(number==31)
		return xxPixelpad_32;
	if(number==32)
		return xxPixelpad_33;
	if(number==33)
		return xxPixelpad_34;
	if(number==34)
		return xxPixelpad_35;
	if(number==35)
		return xxPixelpad_36;
	if(number==36)
		return xxPixelpad_37;
	if(number==37)
		return xxPixelpad_38;
	if(number==38)
		return xxPixelpad_39;
	if(number==39)
		return xxPixelpad_40;
	if(number==40)
		return xxPixelpad_41;
	if(number==41)
		return xxPixelpad_42;

	return xxFEPad1;//should never get here
}

void MainForm::xxChangeTab(QPushButton* button) //changes which rows of pixels are shown (the pixels used to be held in a tab widget)
{
	QString tempstring;
	tempstring.append(button->objectName());
	if(tempstring.indexOf("xxFEView1")!=-1 && tempstring.length()==9)
		emit xxSetRows(1);
	if(tempstring.indexOf("xxFEView2")!=-1)
		emit xxSetRows(22);
	if(tempstring.indexOf("xxFEView3")!=-1)
		emit xxSetRows(43);
	if(tempstring.indexOf("xxFEView4")!=-1)
		emit xxSetRows(64);
	if(tempstring.indexOf("xxFEView5")!=-1)
		emit xxSetRows(85);
	if(tempstring.indexOf("xxFEView6")!=-1)
		emit xxSetRows(106);
	if(tempstring.indexOf("xxFEView7")!=-1)
		emit xxSetRows(127);
	if(tempstring.indexOf("xxFEView8")!=-1)
		emit xxSetRows(148);
	if(tempstring.indexOf("xxFEView9")!=-1)
		emit xxSetRows(169);
	if(tempstring.indexOf("xxFEView10")!=-1)
		emit xxSetRows(190);
	if(tempstring.indexOf("xxFEView11")!=-1)
		emit xxSetRows(211);
	if(tempstring.indexOf("xxFEView12")!=-1)
		emit xxSetRows(232);
	if(tempstring.indexOf("xxFEView13")!=-1)
		emit xxSetRows(253);
	if(tempstring.indexOf("xxFEView14")!=-1)
		emit xxSetRows(274);
	if(tempstring.indexOf("xxFEView15")!=-1)
		emit xxSetRows(295);
	if(tempstring.indexOf("xxFEView16")!=-1)
		emit xxSetRows(316);
	return;
}
void MainForm::xxShowCurrent(const QImage &i) //creates a new window with the image from xxUpdateFEDisplay()
{
	image *imageWindow=new image(this);
	imageWindow->plot = QPixmap::fromImage(i);
	imageWindow->show();
	imageWindow->update();
	imageWindow->raise();
	imageWindow->activateWindow();
}


void MainForm::xxUpdateArray(int index,int value)//Changes xxFEarray['index'] to 'value'
{
	QString tempstring;
	if(index<0){ //for the DC display in the center, the push buttons are associated by button to int with negative numbers (to differentiate them from the pixels)
		index=((index*-1)-1);
		for(int i=21*index+672*CURRENT_DC;i<21*index+21+672*CURRENT_DC;i++){//changes the 21 array elements starting with 'index' to 'value'
			if(value==1)
				xxFEarray[i]="1";
			else
				xxFEarray[i]="0";}
	}
	else if(xxpixelswitch==1){//this variable is switched on in the xxChangeColor funtion to specify that an individual pixel has been changed
		xxpixelswitch=0;
		int row=xxLeftRowLCD->value();
		if(index<21){//there is a discontinuity between the sides of the pixel display in the index of the shown pixels
			if(value==1)
				xxFEarray[index+row-1+672*CURRENT_DC]="1";//index depends on the pixel button (0-20), row on the current rows shown(1-316). the '-1' corrects the row number
			else
				xxFEarray[index+row-1+672*CURRENT_DC]="0";}
		else{
			if(value==1)
				xxFEarray[index+row+315-1+672*CURRENT_DC]="1";
			else
				xxFEarray[index+row+315-1+672*CURRENT_DC]="0";}
	}
	else{//the sliders use this one
		if(value==1)
			xxFEarray[index+672*CURRENT_DC]="1";
		else
			xxFEarray[index+672*CURRENT_DC]="0";}
	return;
}

void MainForm::xxUpdateFEDisplay() //Generates a colored map of the chip...
{

	QImage myQImage(880, 672, QImage::Format_RGB32);
	QPainter painter(&myQImage);
	for(int row=0;row<336;row++){
		for(int column =0; column<80; column++){
			QColor myColor;
			QColor myColor2(0,0,255);
			if(xxHitarray[column][row]!=0) myColor.setRgb(255-xxHitarray[column][row]*3,255,255-xxHitarray[column][row]*3);//hits take precedence in the coloring
			else if("0"==xxFEarray[row+column*336]) myColor.setRgb(130,130,130);//if the pixel is off, set color to dark grey
			else if("1"==xxFEarray[row+column*336]) myColor.setRgb(220,220,255);//if the pixel is on, set color to light blue
			painter.setPen(myColor);
			painter.drawPoint(column*11,row*2); //generates a 2*10 pixel rectangle followed by a 1 pixel wide space
			painter.drawPoint(column*11+1,row*2);
			painter.drawPoint(column*11+2,row*2);
			painter.drawPoint(column*11+3,row*2);
			painter.drawPoint(column*11+4,row*2);
			painter.drawPoint(column*11+5,row*2);
			painter.drawPoint(column*11+6,row*2);
			painter.drawPoint(column*11+7,row*2);
			painter.drawPoint(column*11+8,row*2);
			painter.drawPoint(column*11+9,row*2);
			painter.drawPoint(column*11,row*2+1);
			painter.drawPoint(column*11+1,row*2+1);
			painter.drawPoint(column*11+2,row*2+1);
			painter.drawPoint(column*11+3,row*2+1);
			painter.drawPoint(column*11+4,row*2+1);
			painter.drawPoint(column*11+5,row*2+1);
			painter.drawPoint(column*11+6,row*2+1);
			painter.drawPoint(column*11+7,row*2+1);
			painter.drawPoint(column*11+8,row*2+1);
			painter.drawPoint(column*11+9,row*2+1);
		}
	}
	emit xxdisplay(myQImage); //signals the xxShowCurrent(const QImage &i) slot
}


void MainForm::xxShowHits() //Generates a colored hit map of the chip...
{

	QImage myQImage(880, 672, QImage::Format_RGB32);
	QPainter painter(&myQImage);
	for(int row=0;row<336;row++){
		for(int column =0; column<80; column++){
			QColor myColor;
			double nhitmax = 3;
			double icolor = double(xxHitarray[column][row])/nhitmax;
			if (icolor>1) icolor = 1;
			int ir = int (255-255*icolor);
			int ig = int (255*icolor);
			myColor.setRgb(ir,ig,0);
			//if(xxHitarray[column][row]!=0) myColor.setRgb(255-xxHitarray[column][row]*8,255,255-xxHitarray[column][row]*8);//hits take precedence in the coloring
			//else myColor.setRgb(255,0,0);//if no hit, mark pixel red
			painter.setPen(myColor);
			painter.drawPoint(column*11,row*2); //generates a 2*10 pixel rectangle followed by a 1 pixel wide space
			painter.drawPoint(column*11+1,row*2);
			painter.drawPoint(column*11+2,row*2);
			painter.drawPoint(column*11+3,row*2);
			painter.drawPoint(column*11+4,row*2);
			painter.drawPoint(column*11+5,row*2);
			painter.drawPoint(column*11+6,row*2);
			painter.drawPoint(column*11+7,row*2);
			painter.drawPoint(column*11+8,row*2);
			painter.drawPoint(column*11+9,row*2);
			painter.drawPoint(column*11,row*2+1);
			painter.drawPoint(column*11+1,row*2+1);
			painter.drawPoint(column*11+2,row*2+1);
			painter.drawPoint(column*11+3,row*2+1);
			painter.drawPoint(column*11+4,row*2+1);
			painter.drawPoint(column*11+5,row*2+1);
			painter.drawPoint(column*11+6,row*2+1);
			painter.drawPoint(column*11+7,row*2+1);
			painter.drawPoint(column*11+8,row*2+1);
			painter.drawPoint(column*11+9,row*2+1);
		}
	}
	emit xxdisplay(myQImage); //signals the xxShowCurrent(const QImage &i) slot

}

void MainForm::xxClearHitMap() //Clears the hit map...
{
	for(int i=0;i<336;i++){
		for(int j=0;j<80;j++)
			xxHitarray[j][i]=0;}
	return;
}

void MainForm::xxShowToTs() //Generates a colored hit map of the chip...
{

	QImage myQImage(880, 672, QImage::Format_RGB32);
	QPainter painter(&myQImage);
	for(int row=0;row<336;row++){
		for(int column =0; column<80; column++){
			QColor myColor;
			double icolor = double(xxToTarray[column][row])/16.0;
			int ir = int (255*icolor);
			int ig = int (255-255*icolor);
			myColor.setRgb(ir,ig,255);
			painter.setPen(myColor);
			painter.drawPoint(column*11,row*2); //generates a 2*10 pixel rectangle followed by a 1 pixel wide space
			painter.drawPoint(column*11+1,row*2);
			painter.drawPoint(column*11+2,row*2);
			painter.drawPoint(column*11+3,row*2);
			painter.drawPoint(column*11+4,row*2);
			painter.drawPoint(column*11+5,row*2);
			painter.drawPoint(column*11+6,row*2);
			painter.drawPoint(column*11+7,row*2);
			painter.drawPoint(column*11+8,row*2);
			painter.drawPoint(column*11+9,row*2);
			painter.drawPoint(column*11,row*2+1);
			painter.drawPoint(column*11+1,row*2+1);
			painter.drawPoint(column*11+2,row*2+1);
			painter.drawPoint(column*11+3,row*2+1);
			painter.drawPoint(column*11+4,row*2+1);
			painter.drawPoint(column*11+5,row*2+1);
			painter.drawPoint(column*11+6,row*2+1);
			painter.drawPoint(column*11+7,row*2+1);
			painter.drawPoint(column*11+8,row*2+1);
			painter.drawPoint(column*11+9,row*2+1);
		}
	}
	emit xxdisplay(myQImage); //signals the xxShowCurrent(const QImage &i) slot
}

void MainForm::xxClearToTMap() //Clears ToT map...
{
	for(int i=0;i<336;i++){
		for(int j=0;j<80;j++)
			xxToTarray[j][i]=0;}
	return;
}


void MainForm::xxInitializeFEarray() //only called in the beginning to make sure there's a starting value for all the pixels
{
	for(int i=0;i<26880;i++)
		xxFEarray.append("0");
	for(int i=0;i<336;i++){
		for(int j=0;j<80;j++)
			xxHitarray[j][i]=0;}
	return;
}

int MainForm::xxButtontoInt(QPushButton* button) //associates most of the buttons on the page with an integer
{
	QString tempstring=button->objectName();
	if(tempstring.indexOf("xxFEonoff1")!=-1 && tempstring.length()==10)
		return -1;
	if(tempstring.indexOf("xxFEonoff2")!=-1 && tempstring.length()==10)
		return -2;
	if(tempstring.indexOf("xxFEonoff3")!=-1 && tempstring.length()==10)
		return -3;
	if(tempstring.indexOf("xxFEonoff4")!=-1 && tempstring.length()==10)
		return -4;
	if(tempstring.indexOf("xxFEonoff5")!=-1 && tempstring.length()==10)
		return -5;
	if(tempstring.indexOf("xxFEonoff6")!=-1 && tempstring.length()==10)
		return -6;
	if(tempstring.indexOf("xxFEonoff7")!=-1 && tempstring.length()==10)
		return -7;
	if(tempstring.indexOf("xxFEonoff8")!=-1 && tempstring.length()==10)
		return -8;
	if(tempstring.indexOf("xxFEonoff9")!=-1 && tempstring.length()==10)
		return -9;
	if(tempstring.indexOf("xxFEonoff10")!=-1 && tempstring.length()==11)
		return -10;
	if(tempstring.indexOf("xxFEonoff11")!=-1 && tempstring.length()==11)
		return -11;
	if(tempstring.indexOf("xxFEonoff12")!=-1 && tempstring.length()==11)
		return -12;
	if(tempstring.indexOf("xxFEonoff13")!=-1 && tempstring.length()==11)
		return -13;
	if(tempstring.indexOf("xxFEonoff14")!=-1 && tempstring.length()==11)
		return -14;
	if(tempstring.indexOf("xxFEonoff15")!=-1 && tempstring.length()==11)
		return -15;
	if(tempstring.indexOf("xxFEonoff16")!=-1 && tempstring.length()==11)
		return -16;
	if(tempstring.indexOf("xxFEonoff17")!=-1)
		return -17;
	if(tempstring.indexOf("xxFEonoff18")!=-1)
		return -18;
	if(tempstring.indexOf("xxFEonoff19")!=-1)
		return -19;
	if(tempstring.indexOf("xxFEonoff20")!=-1)
		return -20;
	if(tempstring.indexOf("xxFEonoff21")!=-1)
		return -21;
	if(tempstring.indexOf("xxFEonoff22")!=-1)
		return -22;
	if(tempstring.indexOf("xxFEonoff23")!=-1)
		return -23;
	if(tempstring.indexOf("xxFEonoff24")!=-1)
		return -24;
	if(tempstring.indexOf("xxFEonoff25")!=-1)
		return -25;
	if(tempstring.indexOf("xxFEonoff26")!=-1)
		return -26;
	if(tempstring.indexOf("xxFEonoff27")!=-1)
		return -27;
	if(tempstring.indexOf("xxFEonoff28")!=-1)
		return -28;
	if(tempstring.indexOf("xxFEonoff29")!=-1)
		return -29;
	if(tempstring.indexOf("xxFEonoff30")!=-1)
		return -30;
	if(tempstring.indexOf("xxFEonoff31")!=-1)
		return -31;
	if(tempstring.indexOf("xxFEonoff32")!=-1)
		return -32;
	if(tempstring.indexOf("xxPixel_42")!=-1)
		return 41;
	if(tempstring.indexOf("xxPixel_41")!=-1)
		return 40;
	if(tempstring.indexOf("xxPixel_40")!=-1)
		return 39;
	if(tempstring.indexOf("xxPixel_39")!=-1)
		return 38;
	if(tempstring.indexOf("xxPixel_38")!=-1)
		return 37;
	if(tempstring.indexOf("xxPixel_37")!=-1)
		return 36;
	if(tempstring.indexOf("xxPixel_36")!=-1)
		return 35;
	if(tempstring.indexOf("xxPixel_35")!=-1)
		return 34;
	if(tempstring.indexOf("xxPixel_34")!=-1)
		return 33;
	if(tempstring.indexOf("xxPixel_33")!=-1)
		return 32;
	if(tempstring.indexOf("xxPixel_32")!=-1)
		return 31;
	if(tempstring.indexOf("xxPixel_31")!=-1)
		return 30;
	if(tempstring.indexOf("xxPixel_30")!=-1)
		return 29;
	if(tempstring.indexOf("xxPixel_29")!=-1)
		return 28;
	if(tempstring.indexOf("xxPixel_28")!=-1)
		return 27;
	if(tempstring.indexOf("xxPixel_27")!=-1)
		return 26;
	if(tempstring.indexOf("xxPixel_26")!=-1)
		return 25;
	if(tempstring.indexOf("xxPixel_25")!=-1)
		return 24;
	if(tempstring.indexOf("xxPixel_24")!=-1)
		return 23;
	if(tempstring.indexOf("xxPixel_23")!=-1)
		return 22;
	if(tempstring.indexOf("xxPixel_22")!=-1)
		return 21;
	if(tempstring.indexOf("xxPixel_21")!=-1)
		return 20;
	if(tempstring.indexOf("xxPixel_20")!=-1)
		return 19;
	if(tempstring.indexOf("xxPixel_19")!=-1)
		return 18;
	if(tempstring.indexOf("xxPixel_18")!=-1)
		return 17;
	if(tempstring.indexOf("xxPixel_17")!=-1)
		return 16;
	if(tempstring.indexOf("xxPixel_16")!=-1)
		return 15;
	if(tempstring.indexOf("xxPixel_15")!=-1)
		return 14;
	if(tempstring.indexOf("xxPixel_14")!=-1)
		return 13;
	if(tempstring.indexOf("xxPixel_13")!=-1)
		return 12;
	if(tempstring.indexOf("xxPixel_12")!=-1)
		return 11;
	if(tempstring.indexOf("xxPixel_11")!=-1)
		return 10;
	if(tempstring.indexOf("xxPixel_10")!=-1)
		return 9;
	if(tempstring.indexOf("xxPixel_9")!=-1)
		return 8;
	if(tempstring.indexOf("xxPixel_8")!=-1)
		return 7;
	if(tempstring.indexOf("xxPixel_7")!=-1)
		return 6;
	if(tempstring.indexOf("xxPixel_6")!=-1)
		return 5;
	if(tempstring.indexOf("xxPixel_5")!=-1)
		return 4;
	if(tempstring.indexOf("xxPixel_4")!=-1)
		return 3;
	if(tempstring.indexOf("xxPixel_3")!=-1)
		return 2;
	if(tempstring.indexOf("xxPixel_2")!=-1)
		return 1;
	if(tempstring.indexOf("xxPixel_1")!=-1){
		return 0;}
	return -1;
}

void MainForm::xxBitStreamPreviewUpdate()//updates the 1s and 0s in the bit stream preview text edit
{
	QString tempstring;
	for(int i=CURRENT_DC*672;i<CURRENT_DC*672+672;i++)
		tempstring.append(xxFEarray.at(i));
	xxBitPreview->setText(tempstring);
	return;
}

void MainForm::xxRowLCDs(int left, int right) //changes the row LCDs when a view button is pressed
{
	xxRightRowLCD->display(right);
	xxLeftRowLCD->display(left);

}

void MainForm::xxChangeDC() //changes the currently selected DC
{
	QString tempstring;
	QString LCDString1="Col ";
	QString LCDString2="Col ";
	LCDString1.append(tempstring.setNum((xxChangeDCSpinBox->value())*2+1));
	LCDString2.append(tempstring.setNum((xxChangeDCSpinBox->value())*2+2));
	xxDCLCD->display(xxChangeDCSpinBox->value());
	CURRENT_DC=xxChangeDCSpinBox->value();
	xxLeftColumnLCD->display(LCDString1);
	xxRightColumnLCD->display(LCDString2);
	emit xxUpdate();
}

void MainForm::xxPattern() //coppies a pattern to a section of pixels
{
	QString tempstring;
	QString tempstring2;
	//bool ok=true;
	int low=xxLowBoundbox->value();
	int high=xxHighBoundbox->value();
	if(low>high)
		QMessageBox::warning(this, tr("Error: Invalid bounds"),"Make sure left bound is lower than right bound", QMessageBox::Ok);
	else{
		QString pattern=xxPatternLineEdit->text();
		if(pattern.isEmpty())
			return;
		for(int i=0;i<pattern.length();i++){
			if("0"!=pattern.at(i) && "1"!=pattern.at(i)){
				QMessageBox::warning(this, tr("Error: Invalid character"),pattern.at(i), QMessageBox::Ok);
				return;}}
		QString FEData;
		while(FEData.length()<(high-low))
			FEData.append(pattern);
		int remainder=(FEData.length()-(high-low));
		for(int i=0;i<remainder;i++)	
			FEData.remove((FEData.length()-1),1);
		for(int i=low;i<high;i++)
			xxFEarray[i]=FEData.at(i-low);}
	emit xxUpdate();
}

void MainForm::xxFEDisplayUpdate() //updates the colors of the DC. Sets the color to purple if the pixels aren't all on or off. signalled by xxUpdateEverything
{
	QString tempstring;
	for(int j=-1;j>-33;j--){
		tempstring="";
		for(int i=((-j)-1)*21;i<((-j)-1)*21+21;i++){
			tempstring.append(xxFEarray.value(i+672*CURRENT_DC));
			QTextEdit* Pad=xxDirector(j);
			QPalette palette = Pad->palette();
			QColor red(170,99,90,255);
			QColor green(117,170,117,225);
			QColor purple(200,100,200,255);
			int indexI=tempstring.indexOf("0");
			int indexO=tempstring.indexOf("1");
			if(indexO==-1) //all pixels are off: set color red
			{
				palette.setColor(QPalette::Base, QColor(red));;
				Pad->setText("0");
				Pad->setAlignment(Qt::AlignCenter);
				Pad->setPalette(palette);
			}
			else if(indexI==-1) //all pixels are on: set color green
			{
				palette.setColor(QPalette::Base, green);
				Pad->setText("1");
				Pad->setAlignment(Qt::AlignCenter);
				Pad->setPalette(palette);
			}
			else //some on, some off: set color purple
			{
				palette.setColor(QPalette::Base, purple);
				Pad->setText("Mixed");
				Pad->setAlignment(Qt::AlignCenter);
				Pad->setPalette(palette);}
		}
	}
}

void MainForm::xxPixelsUpdate()//same as pressing the view button for the rows that are already showing - used to update the pixel display
{
	emit xxSetRows(xxLeftRowLCD->value());
}

void MainForm::xxUpdateEverything()//updates the display to reflect changes made, ie a pixel switched should change the color of a text edit in the DC display
{
	emit xxupdateBitStreamPreview();//updates the bit stream text edit
	emit xxupdateFEDisplay();//updates the DC display
	emit xxupdatePixels();//updates the pixels rows display
	xxUpdateAlert->hide();//turns off UPDATE button flag
}

void MainForm::xxCopyDCtoChip() //copies config of currently selected DC to all other DCs
{
	QString tempstring;
	for(int i=0;i<672;i++)
		tempstring.append(xxFEarray[i+672*CURRENT_DC]);
	for(int i=0;i<40;i++){
		for(int j=0;j<672;j++)
			xxFEarray[672*i+j]=tempstring.at(j);}

}

void MainForm::xxChangeRows(int row)//updates the pixels to show the values of the xxFEarray starting at row 'row.' signaled by the "View" buttons and by xxUpdateEverything
{
	emit xxSetRowLCDs(row,row+20);
	int discontinuity=0;
	for(int i=0;i<42;i++){
		if(i==21)
			discontinuity=315;
		QTextEdit *Pad=xxDirector(i);
		QPalette palette = Pad->palette();
		QColor red(170,99,90,255);
		QColor green(117,170,117,225);
		if("0"==xxFEarray[i+row+discontinuity+672*CURRENT_DC-1]){
			palette.setColor(QPalette::Base, QColor(red));;
			Pad->setText("0");
			Pad->setAlignment(Qt::AlignCenter);
			Pad->setPalette(palette);
		}
		else{
			palette.setColor(QPalette::Base, green);
			Pad->setText("1");
			Pad->setAlignment(Qt::AlignCenter);
			Pad->setPalette(palette);
		}

	}
}


void MainForm::xxSaveConfig()//saves the entire configuration in a way that is easy to read
{
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","/home/Configuration_1.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite) || ""==tempstring)
	{
		QMessageBox::warning(this, tr("Error in SaveConfig"),"Failed to open/create file", QMessageBox::Ok);
	}
	QStringList tempstringlist;
	QTextStream outstream(&file);//to mark the time/date of the save
	QTime t=QTime::currentTime();
	QDate d=QDate::currentDate();
	tempstring=d.toString();
	tempstring.append("   ");
	tempstring.append(t.toString());
	tempstringlist.append(tempstring);
	tempstringlist.append("        DC 0             DC 1              DC 2              DC 3              DC 4              DC 5              DC 6              DC 7              DC 8              DC 9              DC 10            DC 11             DC 12             DC 13             DC 14             DC 15             DC 16              DC 17             DC 18            DC 19             DC 20             DC 21              DC 22            DC 23             DC 24             DC 25              DC 26             DC 27            DC 28             DC 29             DC 30             DC 31             DC 32              DC 33            DC 34              DC 35            DC 36             DC 37             DC 38             DC 39");

	for(int i=0;i<336;i++)
	{
		tempstring="";
		for(int j=0;j<40;j++){
			tempstring.append("     ");
			tempstring.append(xxFEarray.at(i+j*672));
			tempstring.append(xxFEarray.at(i+j*672));
			tempstring.append(xxFEarray.at(i+j*672));
			tempstring.append("  ");
			tempstring.append(xxFEarray.value(i+(2*j+1)*336));
			tempstring.append(xxFEarray.value(i+(2*j+1)*336));
			tempstring.append(xxFEarray.value(i+(2*j+1)*336));
			tempstring.append("     ");
			//the result is a string of xxFEarray values corresponding to the indices :    1  336   672 ...
			//the next row will look like this:    2  337   673...
		}
		tempstringlist.append(tempstring);
	}
	for(int i=0;i<1/*tempstringlist.length()*/;i++)
	{
		outstream<<tempstringlist.value(i)<<endl;
	}
	file.close();
}


void MainForm::xxLoadConfig()//reads a file in the format generated by xxSaveConfig
{
	QString loadstring=QFileDialog::getOpenFileName(this,"Select a file to open","/home");
	QFile file(loadstring);
	if(!file.open(QIODevice::ReadWrite))
	{
		QMessageBox::warning(this, tr("Error in accessing file"),"Failed to open", QMessageBox::Ok);
		return;
	}
	else{
		QTextStream instream(&file);
		QString tempstring;
		tempstring=instream.readLine();
		tempstring=instream.readLine();
		for(int i=0;i<336;i++){
			tempstring=instream.readLine();
			if(tempstring.isEmpty()){
				QMessageBox::warning(this, tr("Error while reading file"),"incorrect format", QMessageBox::Ok);
				return;}
			while(tempstring.indexOf(" ")!=-1)
				tempstring.remove(tempstring.indexOf(" "),1);
			if(tempstring.length()!=80*3){
				QMessageBox::warning(this, tr("Error while reading file"),"incorrect format", QMessageBox::Ok);
				return;}
			for(int j=0;j<80;j++){
				if("1"==tempstring.at(3*j))
					xxFEarray[336*j+i]="1";
				else
					xxFEarray[336*j+i]="0";
			}
		}}
	emit xxUpdate();
}
void MainForm::xxCheckChipIDLength()
{
	QString ChipID=xxChipIDLineEdit->text();
	int length=ChipID.length();
	if(length<4)
	{
		QMessageBox::warning(this, tr("ChipID must be 4 bits"),ChipID, QMessageBox::Ok);
		for(int l=0;l<(4-length);l++)
			ChipID.insert(0,"0");
	}
	else if (length>4)
	{
		QMessageBox::warning(this, tr("ChipID must be 4 bits"),ChipID, QMessageBox::Ok);
		for(int l=0;l<(length-4);l++)
			ChipID.remove(0,1);

	}
	xxChipIDLineEdit->setText(ChipID);
	return;
}



void MainForm::xxSend()//using FE-I4A doc, version 6.2
{
	QString pstreamholder=zzStreamLineEdit->text();
	QString ChipID=xxChipIDLineEdit->text(); //chip ID


	/*QString WrRegGlobalPulse="1011010000010";//enable shift register
	WrRegGlobalPulse.append(ChipID);
	WrRegGlobalPulse.append("011011");//register 27
	WrRegGlobalPulse.append("00010000000000000");// turn on SrClr - all others off*/
	QString WrRegPxSrSel="1011010000010"; //Write Register heading. Pxrssel register will be used to select DC(s)
	WrRegPxSrSel.append(ChipID); //Chip ID
	WrRegPxSrSel.append("010110"); //register address = pxsrsel?
	WrRegPxSrSel.append("000000");//first part of the register - never changed

	QString WrFrontEnd=("1011010000100"); //Write Front End command header.
	WrFrontEnd.append(ChipID);
	WrFrontEnd.append("000000");//suggested value for address according to FE-I4A doc

	QString tempstring1;
	QString tempstring2;
	if(xxCopyPMCheckBox->isChecked()==true){
		for(int i=0;i<40;i++){//copy entire Pixel Manager configuration to chip
			tempstring1.clear();
			tempstring1.append(WrRegPxSrSel);
			tempstring1.append("00");//column pair mode: <6,7> 00=just this one, 01=every fourth, 10=every 8th, 11=all
			tempstring1.append( QString("%1").arg(i, 0, 2)); //DC address <8,13>, use #s 0-39  
			while(tempstring1.length()<37)
				tempstring1.insert(31,"0");//makes sure the DC address is 6 bits'
			tempstring1.append("00");
			tempstring2.clear();
			tempstring2.append(WrFrontEnd);
			for(int j=0;j<672;j++)tempstring2.append(xxFEarray[j+i*672]);//FE data

			zzStreamLineEdit->clear();
			zzStreamLineEdit->insert(tempstring1); //this line edit is from the ABSG tab
			zzStreamLineEdit->insert(tempstring2);
			emit xzSend();

		}}
	else{//use only the current DC
		QString option="00";
		if(xxEveryFourthoption->isChecked()==true) option="01";
		else if(xxEveryEighthoption->isChecked()==true) option="10";
		else if(xxEveryoption->isChecked()==true) option="11";
		WrRegPxSrSel.append(option);//column pair mode:<6,7> 00=just this one, 01=every fourth, 10=every 8th, 11=all
		WrRegPxSrSel.append( QString("%1").arg(CURRENT_DC, 0, 2)); //DC address <8,13>, use #s 0-39   
		while(WrRegPxSrSel.length()<37) WrRegPxSrSel.insert(31,"0");//makes sure the DC address is 6 bits
		WrRegPxSrSel.append("00");

		for(int j=0;j<672;j++)WrFrontEnd.append(xxFEarray[672*CURRENT_DC+j]);//FE data

		zzStreamLineEdit->clear();
		zzStreamLineEdit->insert(WrRegPxSrSel); //this line edit is from the ABSG tab
		zzStreamLineEdit->insert(WrFrontEnd);
	}


	QString WrRegGlobalPulse="1011010000010"; // select LoadLatches line 27,3
	WrRegGlobalPulse.append(ChipID);
	WrRegGlobalPulse.append("011011");//register address=27
	WrRegGlobalPulse.append("00010000000000000"); //turn all off, except for LatchEn

	QString WrRegPxStrobes="1011010000010"; // PxStrobes register will be used to set S0=S1=0, select enable bit <1,12>;
	WrRegPxStrobes.append(ChipID);
	WrRegPxStrobes.append("001101"); //register address = 13
	WrRegPxStrobes.append("1000000000000000");//select enable bit <1,12>, <0> is on/off


	QString GlobalPulse="1011010001001";// pulse LoadLatches line
	GlobalPulse.append(ChipID);
	GlobalPulse.append("000101"); //width of pulse=5

	QString WrRegPxStrobes2="1011010000010";//set all bits to 0 - its bad to leave this high (SEUs)
	WrRegPxStrobes2.append(ChipID);
	WrRegPxStrobes2.append("001101"); //register address = 13
	WrRegPxStrobes2.append("0000000000000000");

	zzStreamLineEdit->insert(WrRegGlobalPulse);
	zzStreamLineEdit->insert(WrRegPxStrobes);
	zzStreamLineEdit->insert(GlobalPulse);
	zzStreamLineEdit->insert(WrRegPxStrobes2);
	emit xzSend(); //equivalent to pushing "sent to FE-I4" on ABSG tab
	edited=1;
	zzStreamLineEdit->setText(pstreamholder);//Puts back whatever was there before
}

//*************************************************************  NEW SUBTAB - Parser ******************************************************
// SLOTS defined by sourabh
void MainForm::pread_fileName()
{
	ppath = QFileDialog::getOpenFileName(this,"Choose a File to open","../USBpixI4Test/Parser_files/",QString::null);
	pFileName->setText(ppath);
	QString db1 = "Opening file "; db1+=ppath;
	pDebugBox->append(db1);
}

void MainForm::pparse_file()
{
	int option=0; int linecounter=0;
	//pDebugBox->append("Parsing the file");
	QFile pfile(ppath);
	if(!pfile.open(QIODevice::ReadOnly | QIODevice::Text) ){ pDebugBox->append("Could not open the file."); return; }

	QTextStream in(&pfile);
	while(!in.atEnd()){
		QString line_s = in.readLine(); linecounter++;
		QString line = line_s.simplified();
		QString name = line.section(" ",0,0);
		bool is_this_a_task = false;
		bool is_this_a_loop = false;
		bool is_this_a_sleep = false;
		if(linecounter==1 && name=="#command"){ option=2; pDebugBox->append("Parsing command file."); continue;}
		else if(linecounter==1 && name=="#config"){ option=1; pDebugBox->append("Parsing configuration file.");  pgrList.clear(); continue; }
		if(option==1){
			QString number = line.section(" ",1,1);
			QString address = line.section(" ",2,2);
			QString bitmask = line.section(" ",3,3);
			QString valu = line.section(" ",4,4);
			int regnum = number.toInt();
			bool ok;
			unsigned long regadd = address.toULong();
			unsigned regbitmask = bitmask.toInt(&ok,16);
			int regdefvalue = valu.toInt();
			GRegister temp; temp.addGRegister(name,regnum,regadd,regbitmask,regdefvalue);
			//list of GR
			pgrList.insert(pgrList.size(),temp);	
			//--
			//QString whichline; whichline.setNum(pgrList.size());
			//pDebugBox->append(whichline);
			//--
			//list of shadow registers 
			QString type = line.section(" ",5,5);      //shadow register type: 0=none, 1=ab, 2=c
			QString bitmask_s = line.section(" ",6,6); //shadow register bitmask
			unsigned long regtype = type.toULong();
			unsigned regbitmask_s = bitmask_s.toUInt(&ok,16);
			if (!name.contains("GR")) {
				GRegister temp_s; temp_s.addGRegister(name,regnum,regtype,regbitmask_s,0);
				psrList.insert(psrList.size(),temp_s);
			}
		}
		if(option==2){
			name = line.section(",",0,0);
			if(name=="sleep") {
				is_this_a_sleep = true;
				QString number = line.section(",",1,1);
				ulong sleeptime = number.toULong();
				UPGen::Sleep(sleeptime);
				//QString db1 = "Sleep for " + number + " us.";
				//pDebugBox->append(db1);
			}
			if(name=="task"){is_this_a_task = true; }
			else { is_this_a_task = false; }
			if(name=="loop"){ is_this_a_loop = true; }
			else { is_this_a_loop = false; }
			QString command_name;
			if(!is_this_a_task && !is_this_a_loop && !is_this_a_sleep){
				QString command_name = line.section(",",1,1);
				pprocess_command(command_name, line);
			}		
			if(is_this_a_task){
				pDebugBox->append(line);
				QString taskname = line.section(",",1,1);
				pprocess_task(taskname);
			}
			if(is_this_a_loop){
				pDebugBox->append(line);
				//is this a task loop or command loop
				int loop_number = (line.section(",",1,1)).toInt();
				if(line.section(",",2,2)=="task"){
					QString taskname = line.section(",",3,3);
					for(int i=0; i<loop_number; i++) pprocess_task(taskname);
				}
				if(line.section(",",2,2)=="comm"){
					QString command_name = line.section(",",3,3);
					for(int i=0;i<loop_number; i++) pprocess_command(command_name, line.section(",",2));
				}
			}
		}
	}

	if(option==1){
		QString counter; counter.setNum(pgrList.size());
		//QString db1 = "Read in " + counter + " configuration variables.";
		//pDebugBox->append(db1);
		//Loop over all registers
		bool checker = false;
		for(int j = 0; j < 32; j++){
			QString whichreg; whichreg.setNum(j);
			unsigned register_value = 0x0000;
			for(int i=0; i<pgrList.size(); i++){
				GRegister greg = pgrList.at(i);
				if(greg.getAddress()==j){
					checker = true;
					int nzeros_in_mask = 0;int nones_in_mask = 0;
					QString string_mask; string_mask.setNum(greg.getMask(),2);
					for(int k=string_mask.length()-1;k>=0;k--){
						if(string_mask.mid(k,1)=="0") nzeros_in_mask++;
						else break;
					}
					for(int k=0; k<string_mask.length();k++){ 
						if(string_mask.mid(k,1)=="1") nones_in_mask++; 
						else break;
					}
					QString value; value.setNum(greg.getValue(),2);
					value = value.rightJustified(nones_in_mask,'0');
					/*  *** NOTE FROM SOURABH Oct 26, 2010. ***
					Now some registers have bits are lower position as MSB. For example compare Register 2 and Register 5.
					In Register 2, TrigCnt<15:12> means that the bit that loads first (bit 15) is the MSB for TrigCnt.
					But in Register 5, Vthin<0:7> means that the bit that loads first (bit 7) is LSB, and bit 0 which loads last is MSB.

					To put the right value for TrigCnt, we just take the decimal value and turn into binary. But for Vthin, we need to
					convert the decimal to binary and then flip it, so that the MSB is rightmost.

					This is done below, for the registers which need this.
					Up-to-date as of document v8.4
					*/
					if(NeedsToBeReversed(j,string_mask)){//Flip bits
						//flip new_val;
						QString str=value;
						QByteArray ba = str.toLatin1();
						char *d = ba.data();
						std::reverse(d, d+str.length());
						str = QString(d);
						value = str;
					}
					bool ok;
					unsigned new_val = value.toUInt(&ok,2);			
					new_val = new_val << nzeros_in_mask;			
					unsigned mask_val = greg.getMask();
					new_val = mask_val & new_val;
					register_value = register_value | new_val;
				}
			}
			QString string_val; string_val.setNum(register_value,2);
			string_val = string_val.rightJustified(16,'0');
			QString answer = "Register" + whichreg + ": " + string_val;
			//Stewart -- WriteAllRegisters();
			if(checker){ 
				pDebugBox->append(answer);
				QString pstreamholder=zzStreamLineEdit->text();
				zzStreamLineEdit->setText("   10110   1000   0010   ");
				zzStreamLineEdit->insert(zzChipIDLineEdit->text());
				zzStreamLineEdit->insert(" ");
				QString RegNumber;
				bool ok;
				int tempint=whichreg.toInt(&ok,10);
				RegNumber.setNum(tempint, 2);
				int y=RegNumber.length();
				if(y<6)
				{
					for(int l=0;l<(6-y);l++)
						RegNumber.insert(0,"0");
				}
				else if (y>6)
				{
					QMessageBox::warning(this, tr("Address out of range"),RegNumber, QMessageBox::Ok);
				}
				zzStreamLineEdit->insert(RegNumber);
				zzStreamLineEdit->insert("     ");
				zzStreamLineEdit->insert(string_val);
				emit xzSend();
				edited=1;
				zzStreamLineEdit->setText(pstreamholder);}

			checker = false;
		}
	}
	if(option==0){
		QString db1 = "Unknown file. File must begin with '#config' or '#command' on a single line.";
		pDebugBox->append(db1);
	}
	pfile.close();

}
bool MainForm::NeedsToBeReversed(int reg, QString mask) {

	bool to_reverse = false;
	if((reg>=5 && reg<=20) && reg!=13) to_reverse = true; // All registers less than 20 except PxStrobes
	if(reg==13 && mask=="11110") to_reverse = true; // In PxStrobes only FDAC is reversed
	if(reg==22 && mask=="1100000000") to_reverse = true; // ColprMode
	if(reg==22 && mask=="11111100") to_reverse = true; // ColprAddress
	if(reg==29 && mask=="111111110000") to_reverse = true; // EmptyRecordConfig
	
	if(reg==21 && mask=="1111111111") to_reverse = true; // PlsrDac
	if(reg==17 && mask=="11111111") to_reverse = true; // PlsrIdacRamp
	if(reg==31 && mask=="111111000000") to_reverse = true; // Delay


	return to_reverse;
}

void MainForm::pprocess_task(QString taskname)
{
	QString taskpath = pTaskFileName->text(); QFile taskfile(taskpath);
	if(!taskfile.open(QIODevice::ReadOnly | QIODevice::Text) ){pDebugBox->append("didnt find taskfile = "); pDebugBox->append(taskpath);}
	bool process_this_line = false;
	QTextStream in(&taskfile);
	while(!in.atEnd()){
		QString taskline = in.readLine(); taskline = taskline.simplified();
		QString taskdefname = taskline.section(" ",0,0);
		if(taskdefname==taskname){process_this_line = true; continue; }
		if(taskdefname=="endtask"){process_this_line = false; }
		if(process_this_line){
			//--
			//comment out section which only allows for commands within a task
			//QString command_name = taskline.section(",",1,1);
			//pprocess_command(command_name,taskline);
			//add section which allows for commands and nested tasks
			QString name = taskline.section(",",0,0);
			if(name=="comm"){
				QString command_name = taskline.section(",",1,1);
				pprocess_command(command_name,taskline); }
			else if(name=="task"){
				QString task_name = taskline.section(",",1,1);
				pprocess_task(task_name); }
			else 
				pDebugBox->append("Unknown expression in task file.");

		}
	}
}
void MainForm::pprocess_command(QString command_name,QString line)
{
	pDebugBox->append(line);	
	if(command_name=="LV1"){ prun_command(1, line); }//pDebugBox->append("You issued a LV1 trigger.");}
	else if(command_name=="BCR"){ prun_command(2, line); }//pDebugBox->append("You issued a bunch counter reset.");}
	else if(command_name=="ECR"){ prun_command(3, line); }//pDebugBox->append("You issued a event counter reset.");}
	else if(command_name=="CAL"){ prun_command(4, line); }//pDebugBox->append("You issued a CAL.");}
	else if(command_name=="rdRegister"){ prun_command(5, line); }//pDebugBox->append("You issued a ReadRegister.");}
	else if(command_name=="wrRegister"){ prun_command(6, line); }//pDebugBox->append("You issued a WriteRegister.");}
	else if(command_name=="wrFE"){ prun_command(7, line); }//pDebugBox->append("You issued a wrFE.");}
	else if(command_name=="glbreset"){ prun_command(8, line); }//pDebugBox->append("You issued a global reset.");}
	else if(command_name=="glbpulse"){ prun_command(9, line); }//pDebugBox->append("You issued a global pulse.");}
	else if(command_name=="runmode"){ prun_command(10, line); }//pDebugBox->append("You issued a  FE runmode.");}
	else if(command_name=="confmode"){ prun_command(11, line); }//pDebugBox->append("You issued a FE confmode.");}
	else if(command_name=="rdData"){ prun_command(12, line); }//pDebugBox->append("You issued a read data from SRAM.");}
	else if(command_name=="clearHitMap"){ prun_command(13, line); }//pDebugBox->append("You issued a clear hit map.");}
	else if(command_name=="clearToTMap"){ prun_command(14, line); }//pDebugBox->append("You issued a clear ToT map.");}
	else if(command_name=="resetRowCnt"){ prun_command(15, line); }//pDebugBox->append("You issued a reset RowCounter.");}
	else if(command_name=="RowCnt++"){ prun_command(16, line); }//pDebugBox->append("You issued an increment RowCounter.");}
	else if(command_name=="setDCIdx"){ prun_command(17, line); }//pDebugBox->append("You issued a set DC index.");}
	else if(command_name=="wrShadowRegister"){ prun_command(18, line); }//pDebugBox->append("You issued a WriteShadowRegister.");}
	else if(command_name=="forcewrShadowRegister"){ prun_command(19, line); }//pDebugBox->append("You issued a WriteShadowRegister.");}
	else if(command_name=="rdSRData"){ prun_command(20, line); }//pDebugBox->append("You issued a read SR data from SRAM.");}
	else if(command_name=="clearSRAM"){ zzClearSRAM(); }//pDebugBox->append("You issued a clear SRAM.");}
	else if(command_name=="wrrdRegister"){ prun_command(21, line); }//pDebugBox->append("You issued a write/read command.");}
	else if(command_name=="resetErrorCnt"){ error_cnt = 0; }//pDebugBox->append("You issued a reset error counter.");}
	else if(command_name=="showErrorCnt"){ prun_command(22, line); }//pDebugBox->append("You issued a show error counter.");}
	else if(command_name=="resetGRIdx"){ prun_command(23, line); }//pDebugBox->append("You issued a reset GRIndex.");}
	else if(command_name=="GRIdx++"){ prun_command(24, line); }//pDebugBox->append("You issued an increment GRIndex.");}
	else if(command_name=="wrrdShiftRegister"){ prun_command(25, line); }//pDebugBox->append("You issued a write/read shift register command.");}
	else if(command_name=="resetSRIdx"){ prun_command(26, line); }//pDebugBox->append("You issued a reset SRIndex.");}
	else if(command_name=="SRIdx++"){ prun_command(27, line); }//pDebugBox->append("You issued an increment SRIndex.");}
	else if(command_name=="testlatches"){ prun_command(30, line); }//pDebugBox->append("You issued a test latches.");}
	else if(command_name=="enablepixels"){ prun_command(31, line); }//pDebugBox->append("You issued a test latches.");}
	else if(command_name=="sendzeros"){ prun_command(32, line); }//pDebugBox->append("You issued a test latches.");}
	else if(command_name=="dumpErrors"){ prun_command(33, line); }//pDebugBox->append("You issued a dump errors command.");}
	else if(command_name=="getBC"){ prun_command(34, line); }//pDebugBox->append("You issued a getBC.");}
	else if(command_name=="PulserScan"){ prun_command(35, line); }//pDebugBox->append("You issued a PlsrDelayScan.");}
	else if(command_name=="PulsrIDACScan"){ prun_command(36, line);}//pDebugBox->append("You issued a IDACScan.");}
	else if(command_name=="TimeWalkScan"){ prun_command(37, line);} //Calls TimeWalk();
	else if(command_name==""){bool emptyline=false; emptyline=true;}
	else pDebugBox->append("Unknown command.");
}

void MainForm::prun_command(int whichcommand, QString theline)
{
	/*     The commands are 1=LV1, 2=BCR, 3=ECR, 4=CAL, 5=RdReg, 6=WrReg, 7=WrFE, 8=GlbReset, 9=GlbPulse, 10=RunMode, 11=ConfMode                  */
	//pDebugBox->append("In the function now");
	// PARSE the line based on whichcommand
	QString command_name = theline.section(",",1,1);
	//--
	QString reg_name, reg_value; 
	if(whichcommand==5 || whichcommand==6 || whichcommand==18 || whichcommand==19 || whichcommand == 21 || whichcommand == 25){ 
		reg_name = theline.section(",",2,2);
		if(whichcommand==6  || whichcommand==18 || whichcommand==19 || whichcommand == 21 || whichcommand == 25) reg_value = theline.section(",",3,3);
	}
	QString fedata; if(whichcommand==7) fedata = theline.section(",",2,2);
	QString pulsewidth; if(whichcommand==9 || whichcommand==32) pulsewidth = theline.section(",",2,2);
	QString DCaddress; if(whichcommand==17|| whichcommand==31) DCaddress = theline.section(",",2,2);
	QString dac_init, dac_stop, dac_step, dac_delay_init, delay_step; 
	if(whichcommand==37){ 
		dac_init = theline.section(",",2,2);
		dac_stop = theline.section(",",3,3);
		dac_step = theline.section(",",4,4);
		dac_delay_init = theline.section(",",5,5);
		delay_step = theline.section(",",6,6);
		
	}
	QString num;
	if(whichcommand==35){
		num = theline.section(",",2,2);
	}
	QString numStr, IDACRAMP_init, IDACRAMP_last, IDACRAMP_step;
	if(whichcommand==36){
		numStr=theline.section(",",2,2);
		IDACRAMP_init = theline.section(",",3,3);
		IDACRAMP_last = theline.section(",",4,4);
		IDACRAMP_step = theline.section(",",5,5);

	}
	// EXECUTE according to whichcommand
	if(whichcommand==1){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText("11101");
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
		pget_LV1data(1);
	}
	if(whichcommand==2){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText("10110 0001");
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
	}
	if(whichcommand==3){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText("10110 0010 00000000000000000");
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
	}
	if(whichcommand==4){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText("10110 0100");
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
	}
	if(whichcommand==5){//Read register with reg_name
		int value = pread_Register(reg_name);
		QString redval; redval.setNum(value,10);
		QString redvalbin; redvalbin.setNum(value,2);
		while(redvalbin.length()<16)
				redvalbin.prepend("0");
		if (reg_name.contains("GR")) {
			QString db1; db1 = "Value of "; db1+=reg_name; db1+= " is "; db1+=redvalbin;
			pDebugBox->append(db1);
		}
		else {
			QString db1; db1 = "Value of "; db1+=reg_name; db1+= " is "; db1+=redval;
			pDebugBox->append(db1);
		}
	}
	if(whichcommand==6){//write register with reg_name, reg_value.
		int newvalue = reg_value.toInt();
		pwrite_Register(reg_name,newvalue);
	}
	if(whichcommand==7){
		pwrite_FE(fedata);
	}
	if(whichcommand==8){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText(" 10110 1000 1000");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
	}
	if(whichcommand==9){/*issue a Glbl pulse*/
		bool ok; unsigned int pulse_width = pulsewidth.toUInt(&ok,2);
		// Stewart --------------------
		// Issue Glbl pulse with width=pulse_width
		SendGlobalPulse(pulse_width);
	}
	if(whichcommand==10){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("111000");
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);}
	if(whichcommand==11){
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
	}
	if(whichcommand==12){
		pget_LV1data(2);
	}
	if(whichcommand==13){
		xxClearHitMap();
		pDebugBox->append("Cleared hit map.");
	}
	if(whichcommand==14){
		xxClearToTMap();
		pDebugBox->append("Cleared ToT map.");
	}
	if(whichcommand==15){
		row_index=1;
		//QString db1; db1 = "RowCnt reset. RowCnt = 1"; 
		//pDebugBox->append(db1);
	}
	if(whichcommand==16){
		row_index++;
		QString srow; srow.setNum(row_index);
		//QString db1; db1 = "RowCnt = "; db1+=srow; 
		//pDebugBox->append(db1);
	}
	if(whichcommand==17){
		dc_index = DCaddress.toInt();
		//QString dc; dc.setNum(dc_index);
		//QString db1; db1 = "DCidx = "; db1+=dc; 
		//pDebugBox->append(db1);
	}
	if(whichcommand==18){//write shadow register with reg_name, reg_value.
		int newvalue = reg_value.toInt();
		pwrite_ShadowRegister(reg_name, newvalue/*, false*/);//write register with name, value (do not force overwrite of fixed bits).
	}
	if(whichcommand==19){//write shadow register with reg_name, reg_value.
		int newvalue = reg_value.toInt();
		pwrite_ShadowRegister(reg_name, newvalue/*, true*/);//write register with name, value (force overwrite of fixed bits).
	}
	if(whichcommand==20){
		pget_SRdata(/*0*/);
	} 
	if(whichcommand==21){//Write register with reg_name and check that read back is correct
		//pDebugBox->setUpdatesEnabled(false);
		TestWRRDRegister(reg_name,reg_value);

	
	}
	if (whichcommand == 22) {
		QString db1; db1.setNum(error_cnt);
		db1.prepend("==> Number of errors: ");
		pDebugBox->setTextColor(Qt::blue);
		pDebugBox->append(db1);
		pDebugBox->setTextColor(Qt::black);
	}
	if(whichcommand==23){
		gr_index=0;
		//QString db1; db1 = "GRIndex reset. GRIndex = 2"; 
		//pDebugBox->append(db1);
	}
	if(whichcommand==24){
		gr_index++;
		//QString gr; gr.setNum(gr_index);
		//QString db1; db1 = "GRIndex = "; db1+=gr; 
		//pDebugBox->append(db1);
	}
	if(whichcommand==25){//Write shift register with reg_name and check that read back is correct

		//which shift register?
		int sr_address;
		if (reg_name == "i") {
			sr_address = sr_index;
		}
		else {
			sr_address = reg_name.toInt();
		}
		//select one double column at a time and shift register with address sr_address
		pwrite_Register("Colpr_Mode",0);
		pwrite_Register("Colpr_addr",sr_address);

		//what value?
		QString random = "random";
		QString value_wr;
		if (reg_value.compare(random,Qt::CaseInsensitive)==0) {//write random number
			QString app;
			while(value_wr.length()<672) {
				app.setNum(qrand()%2);
				value_wr.append(app);
			}
		}
		else if (reg_value == "1") 
			value_wr = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
		else 
			value_wr = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

		//write register
		pwrite_FE(value_wr); //write FE
		zzClearSRAM();
		UPGen::Sleep(200);
		pwrite_FE("0");//shift register value out
		UPGen::Sleep(600);
		QString value_rd = pget_SRdata(/*0*/);

		QString db1;
		if (value_wr.compare(value_rd,Qt::CaseInsensitive)==0) {
			pDebugBox->setTextColor(Qt::darkGreen);
			db1.setNum(sr_address);
			db1.prepend("==> Write/read ShiftRegister "); db1.append(": ok ");
			pDebugBox->append(db1);
			pDebugBox->setTextColor(Qt::black);
		}
		else {
			pDebugBox->setTextColor(Qt::red);
			db1.setNum(sr_address);
			db1.prepend("==> Write/read ShiftRegister "); db1.append(": ERROR ");
			pDebugBox->append(db1);
			db1 = "Write: "+value_wr;
			pDebugBox->append(db1);
			db1 = "Read:  "+value_rd;
			pDebugBox->append(db1);
			pDebugBox->setTextColor(Qt::black);
			error_cnt++;
		}


	}
	if(whichcommand==26){
		sr_index=1;
		QString db1; db1 = "SRIndex reset. SRIndex = 0"; 
		pDebugBox->append(db1);
	}
	if(whichcommand==27){
		sr_index++;
		QString sr; sr.setNum(sr_index);
		QString db1; db1 = "SRIndex = "; db1+=sr; 
		pDebugBox->append(db1);
	}	
	if(whichcommand==28){//Write all shift register and check that read back is correct

		//select all double columns 
		pwrite_Register("Colpr_Mode",3);

		//what value?
		QString random = "random";
		QString value_wr;
		if (reg_value.compare(random,Qt::CaseInsensitive)==0) {//write random number
			QString app;
			while(value_wr.length()<672) {
				app.setNum(qrand()%2);
				value_wr.append(app);
			}
		}
		else if (reg_value == "1") 
			value_wr = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
		else 
			value_wr = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

		//write register
		pwrite_FE(value_wr); //write FE
		zzClearSRAM();
		//select one double column at a time 
		pwrite_Register("Colpr_Mode",0);
		//loop over all shift registers
		for (unsigned int sr_address=0; sr_address<40; sr_address++) {
			pwrite_Register("Colpr_addr",sr_address);
			pwrite_FE("0");//shift register value out
			QString value_rd = pget_SRdata(/*0*/);

			QString db1;
			if (value_wr.compare(value_rd,Qt::CaseInsensitive)==0) {
				pDebugBox->setTextColor(Qt::darkGreen);
				db1.setNum(sr_address);
				db1.prepend("==> Write/read ShiftRegister "); db1.append(": ok ");
				pDebugBox->append(db1);
				pDebugBox->setTextColor(Qt::black);
			}
			else {
				pDebugBox->setTextColor(Qt::red);
				db1.setNum(sr_address);
				db1.prepend("==> Write/read ShiftRegister "); db1.append(": ERROR ");
				pDebugBox->append(db1);
				db1 = "Write: "+value_wr;
				pDebugBox->append(db1);
				db1 = "Read:  "+value_rd;
				pDebugBox->append(db1);
				pDebugBox->setTextColor(Qt::black);
				error_cnt++;
			}
		}


	}
	if(whichcommand==30){//Test pixel latches
		TestPixelLatches();
	}
	if(whichcommand==31){//enable pixels

		int dc_addr = DCaddress.toInt();
		EnablePixels4thDC(dc_addr);		
	}
	if(whichcommand==32){
		int nzeros = pulsewidth.toInt();
		QString zerostream;
		while (zerostream.length()<nzeros)
			zerostream.append("0");
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText(zerostream);
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
	}
	 if(whichcommand==33){
		pget_Errors(1);
	}
	 if(whichcommand==34){
		pget_BCIDwithData();
	}

	 if(whichcommand==35){
		PlsrDelayScan(num.toInt());
	 }
	 if(whichcommand==36){
		IDACScan(numStr.toInt(), IDACRAMP_init.toInt(), IDACRAMP_last.toInt(), IDACRAMP_step.toInt());
	 }
	 if(whichcommand==37){
		TimeWalk(dac_init.toInt(),dac_stop.toInt(), dac_step.toInt(), dac_delay_init.toInt(), delay_step.toFloat());
	 }



}
void MainForm::TestWRRDRegister(QString reg_name, QString reg_value){

		//which global register?
		if (reg_name == "i") {
			QString gr; gr.setNum(gr_index);
			gr.prepend("GR");
			reg_name = gr;
		}
		//what value?
		QString random = "random";
		unsigned int value_wr = 0;
		if (reg_value.compare(random,Qt::CaseInsensitive)==0) {//write random number
			value_wr = qrand() % 65535;
		}
		else {
			value_wr = reg_value.toInt();
		}
		//set forced bits
		QString gr27 = "GR27"; QString gr28 = "GR28"; QString gr29 = "GR29"; QString gr6 = "GR6"; 
		int pos_pllen  = 0; //<15>
		int pos_en40   = 6; //<9>
		int pos_clk1s0 = 8; //<7>
		int pos_clk1s1 = 9; //<6>
		int pos_clk1s2 = 10; //<5>
		int pos_clk0s0 = 11; //<4>
		int pos_clk0s1 = 12; //<3>
		int pos_clk0s2 = 13; //<2>
		int pos_en160  = 14; //<1>
		int pos_lvdsen = 13; //<2>
		int pos_lvdsset06 = 0; //<15>
		int pos_lvdsset12 = 15; //<0>
		int pos_lvdsset30 = 14; //<1>
		int pos_clkout = 3; //<12>
		int pos_8b10b = 2; //<13>

		if (reg_name.compare(gr6,Qt::CaseInsensitive)==0) {
			QString value_bit; value_bit.setNum(value_wr,2);
			while(value_bit.length()<16)
				value_bit.prepend("0");
			value_bit.replace(15,1,"0");
			value_bit.replace(14,1,"0");
			value_bit.replace(13,1,"0");
			value_bit.replace(12,1,"0");
			value_bit.replace(11,1,"0");
			value_bit.replace(10,1,"0");
			value_bit.replace(9,1,"0");
			value_bit.replace(8,1,"0");
			bool ok;
			value_wr = value_bit.toInt(&ok,2);
		}
		if (reg_name.compare(gr27,Qt::CaseInsensitive)==0) {
			QString value_bit; value_bit.setNum(value_wr,2);
			while(value_bit.length()<16)
				value_bit.prepend("0");
			value_bit.replace(pos_pllen,1,"1");
			bool ok;
			value_wr = value_bit.toInt(&ok,2);
		}
		if (reg_name.compare(gr28,Qt::CaseInsensitive)==0) {
			QString value_bit; value_bit.setNum(value_wr,2);
			while(value_bit.length()<16)
				value_bit.prepend("0");
			value_bit.replace(pos_clk1s0,1,"0");
			value_bit.replace(pos_clk1s1,1,"0");
			value_bit.replace(pos_clk1s2,1,"0");
			value_bit.replace(pos_clk0s0,1,"0");
			value_bit.replace(pos_clk0s1,1,"0");
			value_bit.replace(pos_clk0s2,1,"1");
			value_bit.replace(pos_en160,1,"1");
			value_bit.replace(pos_en40,1,"1");
			value_bit.replace(pos_lvdsset06,1,"1");
			bool ok;
			value_wr = value_bit.toInt(&ok,2);
		}
		if (reg_name.compare(gr29,Qt::CaseInsensitive)==0) {
			QString value_bit; value_bit.setNum(value_wr,2);
			while(value_bit.length()<16)
				value_bit.prepend("0");
			value_bit.replace(pos_lvdsen,1,"1");
			value_bit.replace(pos_lvdsset12,1,"1");
			value_bit.replace(pos_lvdsset30,1,"1");
			value_bit.replace(pos_clkout,1,"0");
			value_bit.replace(pos_8b10b,1,"0");
			
			bool ok;
			value_wr = value_bit.toInt(&ok,2);
		}
		//write register
		zzClearSRAM();
		pwrite_Register(reg_name,value_wr);
		unsigned int value_rd = pread_Register(reg_name);
		
		QString db1;
		if (value_wr != value_rd) {
			pDebugBox->setTextColor(Qt::red);
			textEdit_testresult->setTextColor(Qt::red);
			db1 = "==> Write/read GR "+reg_name+": ERROR ";
			pDebugBox->append(db1);
			textEdit_testresult->append(db1);
			db1.setNum(value_wr,2); db1.prepend("Write: ");
			pDebugBox->append(db1);
			textEdit_testresult->append(db1);
			db1.setNum(value_rd,2); db1.prepend("Read:  ");
			pDebugBox->append(db1);
			textEdit_testresult->append(db1);
			pDebugBox->setTextColor(Qt::black);
			textEdit_testresult->setTextColor(Qt::black);
			error_cnt++;
		
			//pDebugBox->setUpdatesEnabled(true);
            //pDebugBox->scrollToBottom();
           // pDebugBox->update();

		}
		else {
			pDebugBox->setTextColor(Qt::darkGreen);
			textEdit_testresult->setTextColor(Qt::darkGreen);
			db1 = "==> Write/read GR "+reg_name+": ok ";
			pDebugBox->append(db1);
			textEdit_testresult->append(db1);
			pDebugBox->setTextColor(Qt::black);
			textEdit_testresult->setTextColor(Qt::black);
			
			//pDebugBox->setUpdatesEnabled(true);
           // pDebugBox->scrollToBottom();
            //pDebugBox->update();

		}
}

	
void MainForm::TestPixelLatches(){

		QString all1 = Get672BitWord("1");
		QString all0 = Get672BitWord("0");
	    
		//pwrite_Register("ErrMask1",65535); 

		//select one double column at a time
		pwrite_Register("Colpr_Mode",1);
		pwrite_Register("Colpr_addr",0);
		pwrite_FE(all1); //write FE
		pwrite_Register("Colpr_addr",1);
		pwrite_FE(all1); //write FE
		pwrite_Register("Colpr_addr",2);
		pwrite_FE(all1); //write FE
		pwrite_Register("Colpr_addr",3);
		pwrite_FE(all1); //write FE


		pwrite_Register("Colpr_Mode",0);
		for (int sr_i=0; sr_i<40; sr_i++) {
		
			pwrite_Register("Colpr_addr",sr_i);
			zzClearSRAM();
			UPGen::Sleep(200);
			pwrite_FE(all1);//shift register value out
			UPGen::Sleep(600);
			QString value_rd = pget_SRdata(/*0*/);
			
			QString db1;
			if (all1.compare(value_rd,Qt::CaseInsensitive)==0) {
				pDebugBox->setTextColor(Qt::darkGreen);
				textEdit_testresult->setTextColor(Qt::darkGreen);
				db1.setNum(sr_i);
				db1.prepend("==> Write/read ShiftRegister "); db1.append(": ok ");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->setTextColor(Qt::black);
				textEdit_testresult->setTextColor(Qt::black);
			}
			else {
				pDebugBox->setTextColor(Qt::red);
				textEdit_testresult->setTextColor(Qt::red);
				db1.setNum(sr_i);
				db1.prepend("==> Write/read ShiftRegister "); db1.append(": ERROR ");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->append("Expect all 1");
				textEdit_testresult->append("Expect all 1");
				db1 = "Read:  "+value_rd;
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				db1.setNum(value_rd.length());
				db1.prepend("Length :");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->setTextColor(Qt::black);
				textEdit_testresult->setTextColor(Qt::black);
				error_cnt++;
			}
		}

		
		//select latches
		/*pwrite_Register("PxStr_OE",1);
		pwrite_Register("PxStr_CinjL",1);
		pwrite_Register("PxStr_Imon",0);
		pwrite_Register("PxStr_FDAC",0);
		pwrite_Register("PxStr_CinjS",0);
		pwrite_Register("PxStr_TDAC",0);

		//load latches with global pulse
		pwrite_Register("GR27",4);//LatchEn Bit
		pwrite_Register("PllEn",1);
		
		//enable latches
		pwrite_Register("Colpr_Mode",1); 
		pwrite_Register("Colpr_addr",0);
		SendGlobalPulse(10);
		UPGen::Sleep(600);

		pwrite_Register("Colpr_addr",1);
		SendGlobalPulse(10);
		UPGen::Sleep(600);

		pwrite_Register("Colpr_addr",2);
		SendGlobalPulse(10);
		UPGen::Sleep(600);
 
		pwrite_Register("Colpr_addr",3);
		SendGlobalPulse(10);
		UPGen::Sleep(600);

		//sel mode for copy
		pwrite_Register("S0",1);
		pwrite_Register("S1",1);	
		pwrite_Register("HITLD_IN",0);

		//pwrite_Register("GR27",2);//SRClk
		//pwrite_Register("PllEn",1);
		//SendGlobalPulse(5);
		//pwrite_FE(all1);
		pwrite_Register("Colpr_Mode",1);
		pwrite_Register("Colpr_addr",0);
		pwrite_FE(all1); //write FE
		pwrite_Register("Colpr_addr",1);
		pwrite_FE(all1); //write FE
		pwrite_Register("Colpr_addr",2);
		pwrite_FE(all1); //write FE
		pwrite_Register("Colpr_addr",3);
		pwrite_FE(all1); //write FE

		pwrite_Register("S0",0);
		pwrite_Register("S1",0);
		pwrite_Register("PxStr_OE",0);
		pwrite_Register("PxStr_CinjL",0);	
	

		//loop over SR (read OE)
		pwrite_Register("Colpr_Mode",0);
		for (int sr_i=0; sr_i<40; sr_i++) { //oe is lowest selected strobe

			pwrite_Register("Colpr_addr",sr_i);
			
			//read
			zzClearSRAM();
			UPGen::Sleep(200);
			pwrite_FE(all1);//shift register value out
			UPGen::Sleep(600);
			QString value_rd = pget_SRdata(0);
			QString db1;
			if (all0.compare(value_rd,Qt::CaseInsensitive)==0) {
				pDebugBox->setTextColor(Qt::darkGreen);
				textEdit_testresult->setTextColor(Qt::darkGreen);
				db1.setNum(sr_i);
				db1.prepend("==> Write/read Latch OE in SR "); db1.append(": ok ");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->setTextColor(Qt::black);
				textEdit_testresult->setTextColor(Qt::black);
			}
			else {
				pDebugBox->setTextColor(Qt::red);
				textEdit_testresult->setTextColor(Qt::red);
				db1.setNum(sr_i);
				db1.prepend("==> Write/read Latch OE in SR"); db1.append(": ERROR ");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
			
				pDebugBox->append("Expect all 0");
				textEdit_testresult->append("Expect all 0");
				db1 = "Read:  "+value_rd;
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				db1.setNum(value_rd.length());
				db1.prepend("Length :");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->setTextColor(Qt::black);
				textEdit_testresult->setTextColor(Qt::black);
				error_cnt++;
			}
		}*/

		//sel mode for copy
	/*	pwrite_Register("S0",1);
		pwrite_Register("S1",1);	
		pwrite_Register("HITLD_IN",0);
		pwrite_Register("PxStr_CinjL",1);

		pwrite_Register("Colpr_Mode",3); 
		pwrite_Register("Colpr_addr",0);

		//pwrite_Register("GR27",2);//SRClk
		//pwrite_Register("PllEn",1);
		//SendGlobalPulse(6);
		pwrite_FE(all1);

		pwrite_Register("S0",0);
		pwrite_Register("S1",0);
		pwrite_Register("PxStr_CinjL",0);	


		//loop over SR (read CapL)
		pwrite_Register("Colpr_Mode",0); 
		for (int sr_i=0; sr_i<40; sr_i++) {
 
			pwrite_Register("Colpr_addr",sr_i);
			
			//read
			zzClearSRAM();
			UPGen::Sleep(200);
			pwrite_FE(all1);//shift register value out
			UPGen::Sleep(600);
			QString value_rd = pget_SRdata(0);
			QString db1;
			if (all0.compare(value_rd,Qt::CaseInsensitive)==0) {
				pDebugBox->setTextColor(Qt::darkGreen);
				textEdit_testresult->setTextColor(Qt::darkGreen);
				db1.setNum(sr_i);
				db1.prepend("==> Write/read Latch CapL in SR "); db1.append(": ok ");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->setTextColor(Qt::black);
				textEdit_testresult->setTextColor(Qt::black);
			}
			else {
				pDebugBox->setTextColor(Qt::red);
				textEdit_testresult->setTextColor(Qt::red);
				db1.setNum(sr_i);
				db1.prepend("==> Write/read Latch CapL in SR"); db1.append(": ERROR ");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->append("Expect all 0");
				textEdit_testresult->append("Expect all 0");
				db1 = "Read:  "+value_rd;
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				db1.setNum(value_rd.length());
				db1.prepend("Length :");
				pDebugBox->append(db1);
				textEdit_testresult->append(db1);
				pDebugBox->setTextColor(Qt::black);
				textEdit_testresult->setTextColor(Qt::black);
				error_cnt++;
			}
		}*/
}


void MainForm::EnablePixels4thDC(int dc_addr){

		QString all1 = Get672BitWord("1");
		QString all0 = Get672BitWord("0");
	
		//select one double column at a time
		pwrite_Register("Colpr_Mode",1);
		pwrite_Register("Colpr_addr",dc_addr);
		pwrite_FE(all1); //write FE
		
		//select latches
		pwrite_Register("PxStr_OE",1);
		pwrite_Register("PxStr_CinjL",1);
		pwrite_Register("PxStr_Imon",0);
		pwrite_Register("PxStr_FDAC",0);
		pwrite_Register("PxStr_CinjS",0);
		pwrite_Register("PxStr_TDAC",0);

		//load latches with global pulse
		pwrite_Register("GR27",4);//LatchEn Bit
		pwrite_Register("PllEn",1);
		
		//enable latches
		pwrite_Register("Colpr_Mode",1); 
		pwrite_Register("Colpr_addr",dc_addr);
		SendGlobalPulse(10);
		UPGen::Sleep(600);

		//sel mode for copy
		pwrite_Register("S0",1);
		pwrite_Register("S1",1);	
		pwrite_Register("HITLD_IN",0);

		pwrite_Register("Colpr_Mode",1);
		pwrite_Register("Colpr_addr",dc_addr);
		pwrite_FE(all1); //write FE

		pwrite_Register("S0",0);
		pwrite_Register("S1",0);
		pwrite_Register("PxStr_OE",0);
		pwrite_Register("PxStr_CinjL",0);	
	
		//loop over SR (read OE)
		pwrite_Register("Colpr_Mode",0);
	
		//sel mode for copy
		pwrite_Register("S0",1);
		pwrite_Register("S1",1);	
		pwrite_Register("HITLD_IN",0);
		pwrite_Register("PxStr_CinjL",1);

		pwrite_Register("Colpr_Mode",1); 
		pwrite_Register("Colpr_addr",dc_addr);

		pwrite_FE(all1);

		pwrite_Register("S0",0);
		pwrite_Register("S1",0);
		pwrite_Register("PxStr_CinjL",0);	
}

QString MainForm::Get672BitWord(QString value){

QString value_wr;
  QString random = "random";	      
  if (value.compare(random,Qt::CaseInsensitive)==0) {//write random number
    QString app;
    while(value_wr.length()<672) {
      app.setNum(qrand()%2);
      value_wr.append(app);
    }
  }
  else if (value == "1") 
    value_wr = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
  else 
    value_wr = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

  return value_wr;

}

void MainForm::SendGlobalPulse(int pulsewidth) {
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText(" 10110 1000 1001 ");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("   ");
		QString PulseWidth;
		
		PulseWidth.setNum(pulsewidth, 2);
		int y=PulseWidth.length();
		if(y<6)
		{
			for(int l=0;l<(6-y);l++)
			PulseWidth.insert(0,"0");
		}
		else if (y>6)
		{
			QMessageBox::warning(this, tr("Width out of range"),PulseWidth, QMessageBox::Ok);
		}

		PulseWidth.append("000000000000000000000000000000000000000000000000000000000000000");
		
		zzStreamLineEdit->insert(PulseWidth);
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
}
unsigned MainForm::pread_Register(QString name){//Read register with name

	GRegister greg; int list_pos = -1;
	for(int i=0; i<pgrList.size(); i++){
		GRegister temp = pgrList.at(i);
		if(temp.getName().compare(name,Qt::CaseInsensitive)==0){greg = temp; list_pos=i;}
	}
	int read_address = greg.getAddress();
	// Stewart ------------------------
	//Read register with address read_address. Emit SendtoFEI4, Emit ReadRegisterData. zzReadRegLineEdit has now the 16bit value in binary.

	QString pstreamholder=zzStreamLineEdit->text();
	zzStreamLineEdit->setText("10110 1000 0001 ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("   ");
	QString RegNumber;
	int tempint=read_address;
	bool ok;
	RegNumber.setNum(tempint, 2);
	int y=RegNumber.length();
	if(y<6){
		for(int l=0;l<(6-y);l++)
			RegNumber.insert(0,"0");
	}
	else if (y>6){QMessageBox::warning(this, tr("Address out of range"),RegNumber, QMessageBox::Ok);}
	zzStreamLineEdit->insert(RegNumber);
	edited=1;
	zzStreamLineEdit->insert("   ");
	//myUSBpix->ClearSRAM();
	myUSBpix->SetRunMode();
	myUSBpix->resetRunModeAdd();
	emit xzSend();
	emit xzViewRegData();
	edited=1;
	zzStreamLineEdit->setText(pstreamholder);
	//-------------------------
	QString redval; redval = zzReadRegLineEdit->text(); zzReadRegLineEdit->clear();
	int nzeros_in_mask = 0;
	QString string_mask; string_mask.setNum(greg.getMask(),2);
	for(int j=string_mask.length()-1;j>=0;j--){ if(string_mask.mid(j,1)=="0"){ nzeros_in_mask++;} else {break;} }
	unsigned readbackval = redval.toUInt(&ok,2); unsigned maskval = greg.getMask();
	readbackval = readbackval & maskval;
	readbackval = readbackval >> nzeros_in_mask;

	return readbackval;
}

void MainForm::pwrite_Register(QString name, int newvalue){//write register with name, value.

	//loop over pgrList, find the reg with this name and change its value in pgrList.
	GRegister greg; int list_pos = -1;
	for(int i=0; i<pgrList.size(); i++){
		GRegister temp = pgrList.at(i);
		if(temp.getName().compare(name,Qt::CaseInsensitive)==0){ 
			greg = temp; list_pos=i; temp.setValue(newvalue); pgrList.replace(i,temp); break;
		}
	}
	if (list_pos!=-1) { //pgrList at list_pos has the right value now.
		// loop over pgrList and make the right 0x0000 register value.
		greg = pgrList.at(list_pos);
		QString string_mask; string_mask.setNum(greg.getMask(),2);
		unsigned write_reg_value = 0x0000;
		if (string_mask == "1111111111111111") {
			write_reg_value = greg.getValue();
		}
		else {
			for(int i=0; i<pgrList.size(); i++){
				GRegister greg2 = pgrList.at(i);
				if(greg2.getAddress()==greg.getAddress()){
					int nzeros_in_mask = 0; int nones_in_mask = 0;
					QString string_mask2; string_mask2.setNum(greg2.getMask(),2);
					for(int j=string_mask2.length()-1;j>=0;j--){ if(string_mask2.mid(j,1)=="0"){ nzeros_in_mask++;} else {break;} }
					for(int k=0; k<string_mask2.length();k++){ if(string_mask2.mid(k,1)=="1"){ nones_in_mask++; } else {break;} }
					QString value; value.setNum(greg2.getValue(),2);
					value = value.rightJustified(nones_in_mask,'0');
					if(NeedsToBeReversed(greg2.getAddress(),string_mask2)){//Flip bits
						//flip new_val;
						QString str=value;
						QByteArray ba = str.toLatin1();
						char *d = ba.data();
						std::reverse(d, d+str.length());
						str = QString(d);
						value = str;
					}
					bool ok;
					unsigned new_val = value.toUInt(&ok,2);
					new_val = new_val << nzeros_in_mask;
					unsigned mask_val = greg2.getMask();
					new_val = mask_val & new_val;
					write_reg_value = write_reg_value | new_val;
				}
			}
		}
		// Stewart ------------------------
		// Now write register with address=greg.getAddress() with value=write_reg_value;
		QString pstreamholder=zzStreamLineEdit->text();
		zzStreamLineEdit->setText("10110 1000 0010 ");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert(" ");
		QString RegNumber;
		int tempint=greg.getAddress();
		RegNumber.setNum(tempint, 2);
		int y=RegNumber.length();
		if(y<6)
		{
			for(int l=0;l<(6-y);l++)
				RegNumber.insert(0,"0");
		}
		else if (y>6)
		{
			QMessageBox::warning(this, tr("Address out of range"),RegNumber, QMessageBox::Ok);
		}
		zzStreamLineEdit->insert(RegNumber);
		zzStreamLineEdit->insert("     ");
		edited=1;
		RegNumber.setNum(write_reg_value,2);
		while(RegNumber.length()<16)
			RegNumber.prepend("0");
		zzStreamLineEdit->insert(RegNumber);
		emit xzSend();
		edited=1;
		zzStreamLineEdit->setText(pstreamholder);
		// -------------------------
		/*
		QString db1; db1 = "Will now write Register ";
		QString add; add.setNum(greg.getAddress(),10); db1+= add;
		db1+= " with value "; QString thev; thev.setNum(write_reg_value,2); thev=thev.rightJustified(16,'0'); db1+=thev;
		pDebugBox->append(db1); */
	}
}

void MainForm::pwrite_FE(QString data){//write frontend

	QString Pattern=data;
	if(Pattern.isEmpty())
		return;
	QString FEData;
	while(FEData.length()<672) 
		FEData.append(Pattern);
	int remainder=(FEData.length()-672);
	for(int i=0;i<remainder;i++)
		FEData.remove((FEData.length()-1),1);
	QString pstreamholder=zzStreamLineEdit->text();
	zzStreamLineEdit->setText("10110 1000 0100 ");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("000000 ");
	zzStreamLineEdit->insert(FEData);
	emit xzSend();
	edited=1;
	zzStreamLineEdit->setText(pstreamholder);

}

void MainForm::pwrite_ShadowRegister(QString name, int newvalue/*, bool forcewrite*/){//write shadow register with name, value.

	//loop over pgrList, find the reg with this name and change its value in psrList.
	GRegister greg; int list_pos = -1;
	for(int i=0; i<psrList.size(); i++){
		GRegister temp = psrList.at(i);
		if(temp.getName().compare(name,Qt::CaseInsensitive)==0){ 
			QString addr; addr.setNum(temp.getAddress());
			greg = temp; list_pos=i; temp.setValue(newvalue); psrList.replace(i,temp); break;}
	}
	if (list_pos!=-1) { //sgrList at list_pos has the right value now.
		//
		greg = psrList.at(list_pos);
		unsigned long address = greg.getAddress();
		if (address > 0 ) {

			unsigned write_reg_value = 0x00000000;
			for(int i=0; i<psrList.size(); i++){
				GRegister greg2 = psrList.at(i);
				if(greg2.getAddress()==greg.getAddress()){
					int nzeros_in_mask = 0;
					QString string_mask2; string_mask2.setNum(greg2.getMask(),2);
					for(int j=string_mask2.length()-1;j>=0;j--){ if(string_mask2.mid(j,1)=="0"){ nzeros_in_mask++;} else {break;} }
					QString value; value.setNum(greg2.getValue(),2);
					bool ok;
					unsigned new_val = value.toUInt(&ok,2);
					new_val = new_val << nzeros_in_mask;
					unsigned mask_val = greg2.getMask();
					new_val = mask_val & new_val;
					write_reg_value = write_reg_value | new_val;
				}
			}

			////
			// Now write shadow register with value=write_reg_value;
			QString pstreamholder=zzStreamLineEdit->text();
			QString bitstream;
			bitstream.setNum(write_reg_value,2);
			int nbits = 32;
			while(bitstream.length()<nbits)
				bitstream.prepend("0");

			//here we should determine whether bitstream goes to ab or c 
			bool goestoab = false;
			if (address > 5 ) {
				goestoab = true; }
			if (address == 5) {
				int position = -1; 
				for(int j=bitstream.length()-1;j>=0;j--){ if(bitstream.mid(j,1)=="1"){ position = j; break;}  }
				if (position < 16) 
					goestoab = true;
			}

			QString bitstream_append; bitstream_append = alt_word.right( (address-1)*32 );
			QString bitstream_prepend; bitstream_prepend = alt_word.left( (14-address)*32 );

			bitstream.append(bitstream_append);
			bitstream.prepend(bitstream_prepend);

			alt_word = bitstream;

			bitstream.remove(0,16);
			QString sbitstream;
			if (goestoab) {//SRAB
				sbitstream = bitstream.left(288);}
			else {//SRC
				sbitstream = bitstream.right(144);
				//force pll_en, en_40M, en_160M, clk0_s210 = 100,clk1_s210 = 110, lvdsdren...todo
				int pos_pllen  = 143-79; //src<79>
				int pos_en40   = 143-89; //src<89>
				int pos_clk1s0 = 143-87; //src<87>
				int pos_clk1s1 = 143-86; //src<86>
				int pos_clk1s2 = 143-85; //src<85>
				int pos_clk0s0 = 143-84; //src<84>
				int pos_clk0s1 = 143-83; //src<83>
				int pos_clk0s2 = 143-82; //src<82>
				int pos_en160  = 143-81; //src<81>
				int pos_lvdsen = 143-98; //src<98>
				/*		sbitstream.replace(pos_pllen,1,"1");
				sbitstream.replace(pos_lvdsen,1,"1");
				if (!forcewrite) {
				sbitstream.replace(pos_en40,1,"1");
				sbitstream.replace(pos_clk1s0,1,"0");
				sbitstream.replace(pos_clk1s1,1,"1");
				sbitstream.replace(pos_clk1s2,1,"1");
				sbitstream.replace(pos_clk0s0,1,"0");
				sbitstream.replace(pos_clk0s1,1,"0");
				sbitstream.replace(pos_clk0s2,1,"1");
				sbitstream.replace(pos_en160,1,"1");
				}*/
			}
			QString db1;
			db1.setNum(sbitstream.length());db1.prepend("Length ");
			pDebugBox->append(db1);
			pDebugBox->append(sbitstream);

			zzStreamLineEdit->insert(sbitstream);
			emit xzSend();
			edited=1;
			zzStreamLineEdit->setText(pstreamholder);
			////
		}	
	}
}

void MainForm::psave_debug()
{
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","/home/ParserLog.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite))
    {
		QMessageBox::warning(this, tr("Error"),"Failed to open/create file", QMessageBox::Ok);
    }
	QTextStream outstream(&file);
	tempstring=pDebugBox->toPlainText();
	QTime t=QTime::currentTime();
	QDate d=QDate::currentDate();
	tempstring.prepend(outstream.readAll());
	outstream<<tempstring<<endl<<"*** Parser Log saved at "<<t.toString()<<" on "<<d.toString()<<" ***"<<endl<<endl;
	file.close();
	/*
	for(int i=0; i<pgrList.size(); i++){
		GRegister greg = pgrList.at(i);
		QString mask; mask.setNum(greg.getMask(),2);
		QString value; value.setNum(greg.getValue(),10);
		QString add; add.setNum(greg.getAddress(),10);
		QString db1 = greg.getName() + " " + add + " " + value;
		pDebugBox->append(db1);
		//pDebugBox->append(greg.getName());
		//pDebugBox->append(mask);
		//pDebugBox->append(add);
		//pDebugBox->append(value);
	}	
*/
}
void MainForm::pread_taskfileName()
{
	QString tempstring=QFileDialog::getOpenFileName(this,"Enter Task File","../USBpixI4Test/Parser_files");
	pTaskFileName->setText(tempstring);
}
void MainForm::pget_LV1data(int opt)//opt=0: ouput to command log, opt=1: output to parser; opt=2: output to hit/ToT array
{
		//unsigned int SRAMdataRB[SRAM_WORDSIZE];
		for (int i = 0; i< SRAM_WORDSIZE; i++)
			SRAMdataRB[i] = 0;
		//IsmuCBusy->gotosleep(1000);
		unsigned int PDR[100][200]={0};
		//read SRAM, make words
		myUSBpix->ReadSRAM(chipID); int totalwords = SRAM_WORDSIZE/*myUSBpix->total_SRAM_words*/;
		myUSBpix->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE, chipID);
		int i = -1; int j = 0; int jmax[100] = {0};
		for(int cnt=0; cnt<totalwords; cnt++){
			QString the_word; the_word.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt],10);
			unsigned theword = the_word.toUInt();
			//QString db2; db2.setNum(theword,16); zzCommandLog->append(db2); 
			if(theword == 0){ if(i>-1){ break;} else {continue; } }
			unsigned int headercheck = 0xFF0000 & theword;
			if(headercheck==0xe90000){ // this is a dataheader
				i++; j=0; PDR[i][j]=theword; 
				//QString DH; DH.setNum(PDR[i][j],16);DH+=" =PDR."; zzCommandLog->append(DH);
				j++;
				jmax[i]++;
			}
			else if(theword!=0){
				// this is a hack for now. The col cannot be greater than 80. For some reason the SRAM has a word which is out of range.
				// This needs to be properly addressed at some point. -- Sourabh
				if(theword <= 10747903){
				 PDR[i][j]=theword; j++;jmax[i]++;
				}
			}
		}
		// Now lets output all the PDR data to the log
		zzCommandLog->append("Parsing LV1 data...");
		// Comment out the next part (nested for loops) as that is just a debug print out
		/*
		for(int icnt=0; icnt<i+1; icnt++){
			QString DH; DH.setNum(PDR[icnt][0],2); QString st1 = "Data Header = " + DH; zzCommandLog->append(st1);
			for(int jcnt=1; jcnt<j; jcnt++){
				QString DR; DR.setNum(PDR[icnt][jcnt],2); QString st2 = "Data Record = " + DR; zzCommandLog->append(st2);
			}
		}
		*/
		// Now this is the real output to the log.
		for(int icnt=0; icnt<i+1; icnt++){
			for(int jcnt=0; jcnt<jmax[icnt]; jcnt++){

				if(jcnt==0){//this is DH
					//DH = 11101 001 xxxx [3:0]LV1ID [7:0]bcID
					unsigned int LV1ID = PDR[icnt][jcnt] & 0x000F00;  LV1ID = LV1ID >> 8; QString LV1; LV1.setNum(LV1ID);
					unsigned int BCID = PDR[icnt][jcnt] & 0x0000FF; QString  BC; BC.setNum(BCID);
					QString ans = "LV1ID = " + LV1 + ". BCID = " + BC + ".";
					if(opt==0) zzCommandLog->append(ans);
					if(opt==1) pDebugBox->append(ans);
				}
				else{//this is DR
					//DR = [6:0]col [8:0]row [3:0]tottop [3:0]totbot
					unsigned int COL = PDR[icnt][jcnt] & 0xFE0000; COL = COL >> 17; QString colad; colad.setNum(COL);
					unsigned int ROW = PDR[icnt][jcnt] & 0x01FF00; ROW = ROW >> 8; QString rowad; rowad.setNum(ROW);
					unsigned int TotT = PDR[icnt][jcnt] & 0x0000F0; TotT = TotT >> 4; QString ttop; ttop.setNum(TotT);
					unsigned int TotB = PDR[icnt][jcnt] & 0x00000F; QString tbot; tbot.setNum(TotB);
					QString ans = "Col = " + colad + "   Row = " + rowad + "  Tot = [" + ttop + "," + tbot + "]";
					if(opt==0) zzCommandLog->append(ans);
					if(opt==1) pDebugBox->append(ans);
					///////////////////
					//readback (digital injection)
					if (opt==2) {
						pDebugBox->append(ans);
						if (COL>0 && COL < 81 && ROW>0 && ROW<337) {
							//hit counter
							xxHitarray[COL-1][ROW-1]++;
							if (row_index < 673 && dc_index < 40) {
								int check_row = 0;
								int	check_col = 0;
								//to be checked whether row injection really goes in that order...... !!!!!
								if (row_index>336) {
									check_col = 2*dc_index+1;
									check_row = row_index-336; }
								else {
									check_col = 2*dc_index;
									check_row = row_index; }
								if (COL==check_col && ROW == check_row) {//ToT only filled if returned address corresponds to sent address
									xxToTarray[COL-1][ROW-1]=TotT; } 
						        }
							else {
								if (row_index > 672) pDebugBox->append("Row index out of range.");
								else pDebugBox->append("DC index out of range."); } }
						else {
							if (ROW > 336 || ROW==0) pDebugBox->append("Row value out of range.");
							else pDebugBox->append("DC value out of range.");
						}
					}
					///////////////
				}
			}
		}

}

QString MainForm::pget_SRdata(/*int opt*/)
{
	zzviewShiftRegister();
	QString redval; redval = zzReadRegLineEdit->text(); zzReadRegLineEdit->clear();
	return redval;

}

void MainForm::pget_Errors(int opt)
{
	//unsigned int SRAMdataRB[SRAM_WORDSIZE];
	for (int i = 0; i< SRAM_WORDSIZE; i++)
		SRAMdataRB[i] = 0;
	QString registernumber=zzRdRegNumLineEdit->text();
	myUSBpix->ReadSRAM(chipID); int totalwords = SRAM_WORDSIZE/*myUSBpix->total_SRAM_words*/;
	myUSBpix->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE, chipID);

	if (opt==0) {
	  zzCommandLog->append("-- View Errors --");
	}
	if (opt==1) {
	  pDebugBox->append("-- View Errors --");
	}
	
	for(int cnt=0; cnt<totalwords; cnt++){
		QString tempstring; 
		tempstring.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt],10);
		unsigned theword = tempstring.toUInt();
		unsigned int headercheck = 0xFF0000 & theword;
		if(headercheck==0xef0000){ // this is a service record
			//read the error value
			QString valuestring=QString("%1").arg(theword, 0, 2);
			valuestring.remove(0,8);
			bool ok;
			QString errcode_bin = valuestring.left(6);
			QString errcnt_bin = valuestring.right(10);
			
			unsigned int errcode = errcode_bin.toUInt(&ok,2);
			unsigned int errcnt = errcnt_bin.toUInt(&ok,2);
			QString errcode_dec; errcode_dec.setNum(errcode);
			QString errcnt_dec; errcnt_dec.setNum(errcnt);

			QString outstr = "Error Code "+errcode_dec+" Error Cnt "+errcnt_dec; 
			if (opt==0) {
			  zzCommandLog->append(outstr);
			}
			if (opt==1) {
			  pDebugBox->append(outstr);
			}
			       
		
		}

	}
	
}

void MainForm::pTest1Chip()
{
	//The output is in box textEdit_testresult
	QTime t=QTime::currentTime();
	QDate d=QDate::currentDate();	
	QString datetime = d.toString() + " " + t.toString();
	textEdit_testresult->append(datetime);
	//in order VDDA1, VDDD1, VDDD2, VDDA2
	float voltages[4] = {0.}; float currents[4] = {0.};
	QString meashead = "Voltages/Currents are in order VDDA1, VDDD1, VDDD2, VDDA2";
	textEdit_testresult->append(meashead);
	// ALL OFF
	textEdit_testresult->append("All Off");
	refresh_DCS(); //got all the current voltages and currents.
	voltages[0] = vdda1VLCD->value();  currents[0] = vdda1CLCD->value();
	voltages[1] = vddd1VLCD->value();  currents[1] = vddd1CLCD->value();
	voltages[2] = vddd2VLCD->value();  currents[2] = vddd2CLCD->value();
	voltages[3] = vdda2VLCD->value();  currents[3] = vdda2CLCD->value();
	for(int i=0; i<4; i++){
		QString measure; QTextStream outs(&measure);
		outs<<voltages[i]<<" V    "<<currents[i]<<" A ";
		textEdit_testresult->append(measure);
	}
	// VDDA1 ON
	textEdit_testresult->append("VDDA1 On");
	EnVdda1CheckBox->click(); emit yzA1();
	refresh_DCS(); //got all the current voltages and currents.
	voltages[0] = vdda1VLCD->value();  currents[0] = vdda1CLCD->value();
	voltages[1] = vddd1VLCD->value();  currents[1] = vddd1CLCD->value();
	voltages[2] = vddd2VLCD->value();  currents[2] = vddd2CLCD->value();
	voltages[3] = vdda2VLCD->value();  currents[3] = vdda2CLCD->value();
	for(int i=0; i<4; i++){
		QString measure; QTextStream outs(&measure);
		outs<<voltages[i]<<" V    "<<currents[i]<<" A ";
		textEdit_testresult->append(measure);
	}
	// VDDA1, VDDD1 ON
	textEdit_testresult->append("VDDA1,VDDD1 On");
	EnVddd1CheckBox->click(); emit yzD1();
	refresh_DCS(); //got all the current voltages and currents.
	voltages[0] = vdda1VLCD->value();  currents[0] = vdda1CLCD->value();
	voltages[1] = vddd1VLCD->value();  currents[1] = vddd1CLCD->value();
	voltages[2] = vddd2VLCD->value();  currents[2] = vddd2CLCD->value();
	voltages[3] = vdda2VLCD->value();  currents[3] = vdda2CLCD->value();
	for(int i=0; i<4; i++){
		QString measure; QTextStream outs(&measure);
		outs<<voltages[i]<<" V    "<<currents[i]<<" A ";
		textEdit_testresult->append(measure);
	}
	// VDDA1, VDDD1, VDDD2 ON
	textEdit_testresult->append("VDDA1,VDDD1,VDDD2 On");
	EnVddd2CheckBox->click(); emit yzD2();
	refresh_DCS(); //got all the current voltages and currents.
	voltages[0] = vdda1VLCD->value();  currents[0] = vdda1CLCD->value();
	voltages[1] = vddd1VLCD->value();  currents[1] = vddd1CLCD->value();
	voltages[2] = vddd2VLCD->value();  currents[2] = vddd2CLCD->value();
	voltages[3] = vdda2VLCD->value();  currents[3] = vdda2CLCD->value();
	for(int i=0; i<4; i++){
		QString measure; QTextStream outs(&measure);
		outs<<voltages[i]<<" V    "<<currents[i]<<" A ";
		textEdit_testresult->append(measure);
	}
	// Now configure low current
	ppath = "../USBpixI4Test/Parser_files/config_lowest_current.txt";
	pparse_file();
	UPGen::Sleep(500);
	// Now all on
	textEdit_testresult->append("VDDA1,VDDD1,VDDD2,VDDA2 On : Low Current");
	EnVdda2CheckBox->click(); emit yzA2();
	UPGen::Sleep(1000);
	refresh_DCS(); //got all the current voltages and currents.
	refresh_DCS(); //got all the current voltages and currents.
	UPGen::Sleep(1000);
	voltages[0] = vdda1VLCD->value();  currents[0] = vdda1CLCD->value();
	voltages[1] = vddd1VLCD->value();  currents[1] = vddd1CLCD->value();
	voltages[2] = vddd2VLCD->value();  currents[2] = vddd2CLCD->value();
	voltages[3] = vdda2VLCD->value();  currents[3] = vdda2CLCD->value();
	for(int i=0; i<4; i++){
		QString measure; QTextStream outs(&measure);
		outs<<voltages[i]<<" V    "<<currents[i]<<" A ";
		textEdit_testresult->append(measure);
	}
	// Now configure standard current, turn off VDDA2 first
	EnVdda2CheckBox->click(); emit yzA2();
	refresh_DCS(); //got all the current voltages and currents.
	ppath = "../USBpixI4Test/Parser_files/config_standard_current.txt";
	pparse_file();
	UPGen::Sleep(500);
	// Now all on
	textEdit_testresult->append("VDDA1,VDDD1,VDDD2,VDDA2 On : Standard Current");
	EnVdda2CheckBox->click(); emit yzA2();
	UPGen::Sleep(1000);
	refresh_DCS(); //got all the current voltages and currents.
	UPGen::Sleep(1000);
	voltages[0] = vdda1VLCD->value();  currents[0] = vdda1CLCD->value();
	voltages[1] = vddd1VLCD->value();  currents[1] = vddd1CLCD->value();
	voltages[2] = vddd2VLCD->value();  currents[2] = vddd2CLCD->value();
	voltages[3] = vdda2VLCD->value();  currents[3] = vdda2CLCD->value();
	for(int i=0; i<4; i++){
		QString measure; QTextStream outs(&measure);
		outs<<voltages[i]<<" V    "<<currents[i]<<" A ";
		textEdit_testresult->append(measure);
	}

	
	//// Turn off power to chip.
	//EnVdda2CheckBox->click(); emit yzA2();
	//EnVddd2CheckBox->click(); emit yzD2();
	//EnVddd1CheckBox->click(); emit yzD1();
	//EnVdda1CheckBox->click(); emit yzA1();


}

void MainForm::pTurnPowerOff()
{
	// Turn off power to chip.
	EnVdda2CheckBox->click(); emit yzA2();
	EnVddd2CheckBox->click(); emit yzD2();
	EnVddd1CheckBox->click(); emit yzD1();
	EnVdda1CheckBox->click(); emit yzA1();
}

void MainForm::pSaveProbeResults()
{
	QTime t=QTime::currentTime();
	QDate d=QDate::currentDate();	
	QString tempfilename1 = QFileDialog::getSaveFileName(this,"Save File","./ProbeTestLog.txt");
	QFile file2(tempfilename1);
	if(!file2.open(QIODevice::ReadWrite))
    {
		QMessageBox::warning(this, tr("Error in File Open"),"Failed to open/create file", QMessageBox::Ok);
    }
	QTextStream outstream(&file2);
	QString tempstr = textEdit_testresult->toPlainText();
	tempstr.prepend(outstream.readAll());
	outstream<<tempstr<<endl<<"*** Log latest save at "<<t.toString()<<" on "<<d.toString()<<" ***"<<endl;
	file2.close();
}

void MainForm::clearlog()
{
	textEdit_testresult->clear();
}

void MainForm::setupScanChainBlock()
{
	switch (ScanChainComboBox->currentIndex())
	{
	case 0:
		ScanChainPatternSpinBox->setMaximum(28);
		break;
	case 1:
		ScanChainPatternSpinBox->setMaximum(265);
		break;
	case 2:
		ScanChainPatternSpinBox->setMaximum(isFEI4B?235:230);
		break;
	default:
		ScanChainPatternSpinBox->setMaximum(28);
		break;
	}
	ScanChainEdit->setText("(put bitstream here)");
	ScanChainEditRB->setText("Scan Chain Result");
	ScanChainEditTarget->setText("(target bitstream)");
	QPalette p = ScanChainEditRB->palette();
	p.setColor(QPalette::Normal, QPalette::Base, QColor(255,255,255));//white color
	ScanChainEditRB->setPalette(p);
}

QString MainForm::loadScanChainFromFile(int block, QString filename, int patternnr, bool& SEstate, bool& SIstate, bool& CLKstate)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
		return "File not found";
	QString patternheader = "pattern " + QString::number(patternnr);
	QTextStream in(&file);
	QString line;
	QString debug = "";
	QString pattern = "";
	QString pi_Definition = "";
	bool pattern_found = false;

	while (!in.atEnd())
	{
		line = in.readLine();
		if (line.indexOf(patternheader, 0, Qt::CaseInsensitive) != -1)
		{
			while (!in.atEnd())
			{
				line = in.readLine();
				if (line.indexOf("si\"=", 0, Qt::CaseInsensitive) != -1)
				{
					pattern_found = true;
					pattern = line;
					while (!in.atEnd())
					{
						if (!(pattern.indexOf(";", 0) != -1))
						{
							pattern += in.readLine();
						}
						else
							break;
					}
				}
				if (line.indexOf("_pi\"=", 0, Qt::CaseInsensitive) != -1)
				{
					pi_Definition = line;
					while (!in.atEnd())
					{
						if (!(pi_Definition.indexOf(";", 0) != -1))
						{
							pi_Definition += in.readLine();
						}
						else
							break;
					}
					//break;
				}
				if (line.indexOf("_po\"=", 0, Qt::CaseInsensitive) != -1) // needed to read second _pi  definition, if it is existing...
					break;

			}
			break;
		}
	}
	file.close();
	int se_select_decr = 0;
	int si_select_decr = 0;
	int clk_select_decr = 0;
	switch (block)
	{
	case SC_DOB: se_select_decr = 2; si_select_decr = 1; clk_select_decr = 4; break;
	case SC_CMD: se_select_decr = 3; si_select_decr = 2; clk_select_decr = 4; break; // ToDo: find correct value!
	case SC_ECL: se_select_decr = 4; si_select_decr = 1; clk_select_decr = 3; break;
	default: break;
	}
	pattern = pattern.mid(pattern.indexOf("=", 0)+1, pattern.indexOf(";", 0)-1 - pattern.indexOf("=", 0));
	QString SEstateString = pi_Definition.mid(pi_Definition.indexOf(";", 0)-se_select_decr, 1);
	QString SIstateString = pi_Definition.mid(pi_Definition.indexOf(";", 0)-si_select_decr, 1);
	QString CLKstateString = pi_Definition.mid(pi_Definition.indexOf(";", 0)-clk_select_decr, 1);
	
	SEstate = SEstateString.toInt();

	if (SIstateString != "0")
		SIstateString = "1";
	SIstate = SIstateString.toInt();

	if (CLKstateString != "0")
		CLKstateString = "1";
	CLKstate = CLKstateString.toInt();

	if (!pattern_found)
		return "Pattern not found";
	return pattern;
}

QString MainForm::loadScanChainTargetFromFile(QString filename, int patternnr)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
		return "File not found";
	QString patternheader = "pattern " + QString::number(patternnr+1);
	QTextStream in(&file);
	QString line;
	QString debug = "";
	QString pattern = "";
	bool pattern_found = false;

	while (!in.atEnd())
	{
		line = in.readLine();
		if (line.indexOf(patternheader, 0, Qt::CaseInsensitive) != -1)
		{
			while (!in.atEnd())
			{
				line = in.readLine();
				if (line.indexOf("so\"=", 0, Qt::CaseInsensitive) != -1)
				{
					pattern_found = true;
					pattern = line;
					while (!in.atEnd())
					{
						if (!(pattern.indexOf(";", 0) != -1))
						{
							pattern += in.readLine();
						}
						else
							break;
					}
					break;
				}
			}
			break;
		}
	}
	file.close();
	pattern = pattern.mid(pattern.indexOf("=", 0)+1, pattern.indexOf(";", 0)-1 - pattern.indexOf("=", 0));
	if (!pattern_found)
	{
		file.open(QIODevice::ReadOnly);
		while (!in.atEnd())
		{
			line = in.readLine();
			patternheader = QString::number(patternnr) + " unload\": Call \"load_unload\"";
			if (line.indexOf(patternheader, 0, Qt::CaseInsensitive) != -1)
			{
				while (!in.atEnd())
				{
					line = in.readLine();
					if (line.indexOf("so\"=", 0, Qt::CaseInsensitive) != -1)
					{
						pattern_found = true;
						pattern = line;
						while (!in.atEnd())
						{
							if (!(pattern.indexOf(";", 0) != -1))
							{
								pattern += in.readLine();
							}
							else
								break;
						}
						break;
					}
				}
				break;
			}
		}
		file.close();
	}
	pattern = pattern.mid(pattern.indexOf("=", 0)+1, pattern.indexOf(";", 0)-1 - pattern.indexOf("=", 0));
	if (!pattern_found)
		return "Pattern not found";
	pattern.replace(QString("H"), QString("1"));
	pattern.replace(QString("L"), QString("0"));
	return pattern;
}

void MainForm::showScanChainVectorLength()
{
	QString ss = ScanChainEdit->text();
	ScanChainSizeDisplay->display(ss.size());
}

void MainForm::loadScanChainPattern()
{
	bool SEstate = false;
	bool SIstate = false;
	bool CLKstate = false;
	switch(ScanChainComboBox->currentIndex())
	{
		case 0:
			ScanChainEdit->setText(loadScanChainFromFile(SC_DOB, "../config/dob.patterns.stil", ScanChainPatternSpinBox->value(), SEstate, SIstate, CLKstate));
			ScanChainEditTarget->setText(loadScanChainTargetFromFile("../config/dob.patterns.stil", ScanChainPatternSpinBox->value()));
			break;
		case 1:
			ScanChainEdit->setText(loadScanChainFromFile(SC_CMD, "../config/cmd.patterns.stil", ScanChainPatternSpinBox->value(), SEstate, SIstate, CLKstate));
			ScanChainEditTarget->setText(loadScanChainTargetFromFile("../config/cmd.patterns.stil", ScanChainPatternSpinBox->value()));
			break;
		case 2:
			ScanChainEdit->setText(loadScanChainFromFile(SC_ECL, isFEI4B?( "../config/eclB.patterns.stil"):("../config/eclA.patterns.stil"), ScanChainPatternSpinBox->value(), SEstate, SIstate, CLKstate));
			ScanChainEditTarget->setText(loadScanChainTargetFromFile(isFEI4B?("../config/eclB.patterns.stil"):("../config/eclA.patterns.stil"), ScanChainPatternSpinBox->value()));
			break;
		default:
			ScanChainEdit->setText("(put bitstream here)");
			ScanChainEditTarget->setText("(target bitstream)");
			break;
	}
	ScanChainSEStateCheckBox->setChecked(SEstate);
	ScanChainSIStateCheckBox->setChecked(SIstate);
	ScanChainClkPulseCheckBox->setChecked(CLKstate);
	QPalette p = ScanChainEditRB->palette();
	p.setColor(QPalette::Normal, QPalette::Base, QColor(255,255,255));//white color
	ScanChainEditRB->setPalette(p);
	ScanChainEditRB->setText("");
}

void MainForm::runScanChain()
{
	int size, value, valueRB, add;
	QString valuestringRB = "";
	QPalette p = ScanChainEditRB->palette();
	double current_before = 0;
	double current_after = 0;
	switch(ScanChainComboBox->currentIndex())
	{
	case 0:
		for (int index = SCDOB0; index <= SCDOB2; index++)
		{
			bool ok = true;
			myUSBpix->GetScanChainValue(SC_DOB, index, size, add, value, chipID);
			QString valuestring = ScanChainEdit->text();
			if(valuestring.size() != SC_DOB_BITSIZE)
			{
				ScanChainEditRB->setText("ERROR: bitstream length wrong !!!");
				p.setColor(QPalette::Normal, QPalette::Base, QColor(255,0,0));//red color
				ScanChainEditRB->setPalette(p);
				return;
			}
			valuestring = valuestring.mid(add, size);
			value = valuestring.toUInt(&ok, 2);
			if(!ok)
			{
				ScanChainEditRB->setText("FORMAT ERROR: base 2 numbers only !!!");
				p.setColor(QPalette::Normal, QPalette::Base, QColor(255,0,0));//red color
				ScanChainEditRB->setPalette(p);
				return;
			}
			myUSBpix->SetScanChainValue(SC_DOB, index, value, chipID);
		}
		myUSBpix->RunScanChain(SC_DOB, dcsInst, current_before, current_after, !ScanChainClkPulseCheckBox->isChecked(), ScanChainSEStateCheckBox->isChecked(), ScanChainSIStateCheckBox->isChecked(), chipID);
		for (int index = SCDOB0; index <= SCDOB2; index++)
		{
			myUSBpix->GetScanChainValueRB(SC_DOB, index, size, add, valueRB, chipID);
			QString stringRB = QString::number(valueRB, 2);
			if(stringRB.size() > size)
			{
				stringRB = stringRB.mid(stringRB.size() - size);
			}
			int ssize = stringRB.size();
			if (ssize != size)
			{
				for (int i = 0; i < (size - ssize); i++)
					stringRB = "0" + stringRB;
			}
			valuestringRB += stringRB;
		}
		ScanChainEditRB->setText(valuestringRB);
		myUSBpix->SaveFileFromScanChainRB(SC_DOB, chipID);
		break;
	case 1:
		for (int index = SCCMD0; index <= SCCMD8; index++)
		{
			bool ok = true;
			myUSBpix->GetScanChainValue(SC_CMD, index, size, add, value, chipID);
			QString valuestring = ScanChainEdit->text();
			if(valuestring.size() != SC_CMD_BITSIZE)
			{
				ScanChainEditRB->setText("ERROR: bitstream length wrong !!!");
				p.setColor(QPalette::Normal, QPalette::Base, QColor(255,0,0));//red color
				ScanChainEditRB->setPalette(p);
				return;
			}
			valuestring = valuestring.mid(add, size);
			value = valuestring.toUInt(&ok, 2);
			if(!ok)
			{
				ScanChainEditRB->setText("FORMAT ERROR: base 2 numbers only !!!");
				p.setColor(QPalette::Normal, QPalette::Base, QColor(255,0,0));//red color
				ScanChainEditRB->setPalette(p);
				return;
			}
			myUSBpix->SetScanChainValue(SC_CMD, index, value, chipID);
		}
		myUSBpix->RunScanChain(SC_CMD, dcsInst, current_before, current_after, !ScanChainClkPulseCheckBox->isChecked(), ScanChainSEStateCheckBox->isChecked(), ScanChainSIStateCheckBox->isChecked(), chipID);
		for (int index = SCCMD0; index <= SCCMD8; index++)
		{
			myUSBpix->GetScanChainValueRB(SC_CMD, index, size, add, valueRB, chipID);
			QString stringRB = QString::number(valueRB, 2);
			if(stringRB.size() > size)
			{
				stringRB = stringRB.mid(stringRB.size() - size);
			}
			int ssize = stringRB.size();
			if (ssize != size)
			{
				for (int i = 0; i < (size - ssize); i++)
					stringRB = "0" + stringRB;
			}
			valuestringRB += stringRB;
		}
		ScanChainEditRB->setText(valuestringRB);
		myUSBpix->SaveFileFromScanChainRB(SC_CMD, chipID);
		break;
	case 2:
		for (int index = (isFEI4B?B_SCECL0:SCECL0); index <= (isFEI4B?B_SCECL108:SCECL99); index++)
		{
			bool ok = true;
			myUSBpix->GetScanChainValue(SC_ECL, index, size, add, value, chipID);
			QString valuestring = ScanChainEdit->text();
			if(valuestring.size() != (isFEI4B?SC_ECL_B_BITSIZE:SC_ECL_A_BITSIZE))
			{
				ScanChainEditRB->setText("ERROR: bitstream length wrong !!!");
				p.setColor(QPalette::Normal, QPalette::Base, QColor(255,0,0));//red color
				ScanChainEditRB->setPalette(p);
				return;
			}
			valuestring = valuestring.mid(add, size);
			value = valuestring.toUInt(&ok, 2);
			if(!ok)
			{
				ScanChainEditRB->setText("FORMAT ERROR: base 2 numbers only !!!");
				p.setColor(QPalette::Normal, QPalette::Base, QColor(255,0,0));//red color
				ScanChainEditRB->setPalette(p);
				return;
			}
			myUSBpix->SetScanChainValue(SC_ECL, index, value, chipID);
		}
		myUSBpix->RunScanChain(SC_ECL, dcsInst, current_before, current_after, !ScanChainClkPulseCheckBox->isChecked(), ScanChainSEStateCheckBox->isChecked(), ScanChainSIStateCheckBox->isChecked(), chipID);
		for (int index = (isFEI4B?B_SCECL0:SCECL0); index <= (isFEI4B?B_SCECL108:SCECL99); index++)
		{
			myUSBpix->GetScanChainValueRB(SC_ECL, index, size, add, valueRB, chipID);
			QString stringRB = QString::number(valueRB, 2);
			if(stringRB.size() > size)
			{
				stringRB = stringRB.mid(stringRB.size() - size);
			}
			int ssize = stringRB.size();
			if (ssize != size)
			{
				for (int i = 0; i < (size - ssize); i++)
					stringRB = "0" + stringRB;
			}
			valuestringRB += stringRB;
		}
		ScanChainEditRB->setText(valuestringRB);
		myUSBpix->SaveFileFromScanChainRB(SC_ECL, chipID);
		break;
	}
	if (CompareScanChainResult())
		p.setColor(QPalette::Normal, QPalette::Base, QColor(0,255,0));//green color
	else
		p.setColor(QPalette::Normal, QPalette::Base, QColor(255,255,0));//orange color
	ScanChainEditRB->setPalette(p);
	CurrentBeforeLCD->display(QString::number(current_before,'f',3));
	CurrentAfterLCD->display(QString::number(current_after,'f',3));
}

bool MainForm::CompareScanChainResult()
{
	QString SCresult = ScanChainEditRB->text();
	QString target = ScanChainEditTarget->text();
	int result = 0;
	
	for (int i = 0; i < SCresult.size(); i++)
	{
		QString bitRB = SCresult.mid(i, 1);
		QString bitTG = target.mid(i, 1);
		if ((bitRB != bitTG) && (bitTG != "X"))
			return false;
	}
	return true;
}

void MainForm::CallCalib()
{

	ppath = "../USBpixI4Test/Parser_files/config_standard_current.txt";
	pparse_file();
	
}

void MainForm::CallwrRegister()
{

	ppath = "../USBpixI4Test/Parser_files/command_RW_GR.txt";
	pparse_file();
	CallCalib();
	
}

void MainForm::CalltestLatches()
{

	//pwrite_Register("ErrMask1",65535);
	TestPixelLatches();
	
}

void MainForm::CallEnable0()
{	
	ppath = "../USBpixI4Test/Parser_files/ClearAllLatches.txt";
    pparse_file();	
	EnablePixels4thDC(0);
	
}
void MainForm::CallEnable1()
{
	ppath = "../USBpixI4Test/Parser_files/ClearAllLatches.txt";
    pparse_file();	
	EnablePixels4thDC(1);
	
}

void MainForm::CallEnable2()
{
	
	ppath = "../USBpixI4Test/Parser_files/ClearAllLatches.txt";
    pparse_file();	
	EnablePixels4thDC(2);
	
}

void MainForm::CallEnable3()
{
	ppath = "../USBpixI4Test/Parser_files/ClearAllLatches.txt";
    pparse_file();	
	EnablePixels4thDC(3);
	
}

void MainForm::IREFconnect()
{
	// make busy cursor
	setCursor(Qt::WaitCursor);

	//m_Device = ibdev(0, 5, 0, 13, 1, 0);

	char command[32];

	sprintf(command, "*RST");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, "*CLS");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, ":CONF:CURR:DC");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, ":CURR:DC:RANG:UPP 0");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	// release mouse cursor
	setCursor(Qt::ArrowCursor);
}

void MainForm::VTHINCconnect()
{
	// make busy cursor
	setCursor(Qt::WaitCursor);

	//m_Device = ibdev(0, 5, 0, 13, 1, 0);

	char command[32];

	sprintf(command, "*RST");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, "*CLS");
	//ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, ":CONF:VOLT:DC");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, ":VOLT:DC:RANG:UPP 2");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	// release mouse cursor
	setCursor(Qt::ArrowCursor);
}

void MainForm::PULSERconnect()
{
	// make busy cursor
	setCursor(Qt::WaitCursor);

	//m_Device = ibdev(0, 5, 0, 13, 1, 0);

	char command[32];

	sprintf(command, "*RST");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, "*CLS");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, ":CONF:VOLT:DC");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	sprintf(command, ":VOLT:DC:RANG:UPP 2");
    //ibwrt(m_Device, command, strlen(command));
	UPGen::Sleep(500);

	// release mouse cursor
	setCursor(Qt::ArrowCursor);
}

void MainForm::disconnectdevice()
{
	//ibonl(m_Device, 0);
}

void MainForm::IREFrun()
{
	// make busy cursor
	setCursor(Qt::WaitCursor);

	char command[32];
	sprintf(command, "READ?");

	char answer[32];

	QString temp0;
	if(hUSB->HandlePresent()){
	  dcsInst->UpdateMeasurements();
	  temp0.setNum(dcsInst->GetNTCTemperature());
	}

	for (int i = 0; i < 16; i++) {
		pwrite_Register("CurrentReference", i);
		UPGen::Sleep(500);
		//ibwrt(m_Device, command, strlen(command));
		UPGen::Sleep(500);
		//ibrd(m_Device, answer, sizeof(answer));
		QString MyString;
		int j = 0;
		while (answer[j] != 10) {
			MyString.append(answer[j]);
			j++;
		}
		textEdit_testresult->append(MyString);
	}

	QString temp1;
	if(hUSB->HandlePresent()){
	  dcsInst->UpdateMeasurements();
	  temp1.setNum(dcsInst->GetNTCTemperature());
	}
		
	textEdit_testresult->append("Current Reference");

	QTime t1 = QTime::currentTime();
	textEdit_testresult->append(t1.toString());

	QDate d1 = QDate::currentDate();
	textEdit_testresult->append(d1.toString());

	temp0.prepend("T_i: ");
	textEdit_testresult->append(temp0);

	temp1.prepend("T_f: ");
	textEdit_testresult->append(temp1);

	// release mouse cursor
	setCursor(Qt::ArrowCursor);
}

void MainForm::VTHINCrun()
{
	// make busy cursor
	setCursor(Qt::WaitCursor);

	char command[32];
	sprintf(command, "READ?");

	char answer[32];

	QString temp0;
	if(hUSB->HandlePresent()) {
	  dcsInst->UpdateMeasurements();
	  temp0.setNum(dcsInst->GetNTCTemperature());
	}

	if (radioButton_Fine->isChecked()) {
		pwrite_Register("Vthin", 255);
		UPGen::Sleep(500);
		pwrite_Register("vthin_AltCoarse", comboBox_constantvalue->currentIndex());
		UPGen::Sleep(500);
		for (int i = 0; i < 256; i++) {
			pwrite_Register("vthin_AltFine", i);
			UPGen::Sleep(333);
			//ibwrt(m_Device, command, strlen(command));
			UPGen::Sleep(250);
			//ibrd(m_Device, answer, sizeof(answer));
			QString MyString;
			int j = 0;
			while (answer[j] != 10) {
				MyString.append(answer[j]);
				j++;
			}
			textEdit_testresult->append(MyString);
		}
	}

	if (radioButton_Coarse->isChecked()) {
		pwrite_Register("Vthin", 255);
		UPGen::Sleep(500);
		pwrite_Register("vthin_AltFine", comboBox_constantvalue->currentIndex());
		UPGen::Sleep(500);
		for (int i = 0; i < 256; i++) {
			pwrite_Register("vthin_AltCoarse", i);
			UPGen::Sleep(333);
			//ibwrt(m_Device, command, strlen(command));
			UPGen::Sleep(250);
			//ibrd(m_Device, answer, sizeof(answer));
			QString MyString;
			int j = 0;
			while (answer[j] != 10) {
				MyString.append(answer[j]);
				j++;
			}
			textEdit_testresult->append(MyString);
		}
	}

	QString temp1;
	if(hUSB->HandlePresent()) {
	  dcsInst->UpdateMeasurements();
	  temp1.setNum(dcsInst->GetNTCTemperature());
	}

	textEdit_testresult->append("VthinC_P");

	QTime t1 = QTime::currentTime();
	textEdit_testresult->append(t1.toString());

	QDate d1 = QDate::currentDate();
	textEdit_testresult->append(d1.toString());

	temp0.prepend("T_i: ");
	textEdit_testresult->append(temp0);

	temp1.prepend("T_f: ");
	textEdit_testresult->append(temp1);

	// release mouse cursor
	setCursor(Qt::ArrowCursor);
}

void MainForm::PULSERrun()
{
	// make busy cursor
	setCursor(Qt::WaitCursor);

	char command[32];
	sprintf(command, "READ?");

	char answer[32];

	QString temp0;
	if(hUSB->HandlePresent()) {
	  dcsInst->UpdateMeasurements();
	  temp0.setNum(dcsInst->GetNTCTemperature());
	}

	// Setup up the pulser
	pwrite_Register("PlsrIdacRamp", 128);
	UPGen::Sleep(500);
	pwrite_Register("Delay", 0);
	UPGen::Sleep(500);
	pwrite_Register("RiseUpTau", 7);
	UPGen::Sleep(500);
	pwrite_Register("pwr", 1);
	UPGen::Sleep(500);
	pwrite_Register("ExtDigCalSW", 0);
	UPGen::Sleep(500);
	pwrite_Register("ExtAnaCalSW", 1);
	UPGen::Sleep(500);
	pwrite_Register("PlsrVgOPamp", 255);
	UPGen::Sleep(500);
	pwrite_Register("PlsrDacBias", comboBox_pulserbias->currentIndex());
	UPGen::Sleep(500);
	pwrite_Register("Colpr_Mode", comboBox_pulsermode->currentIndex());
	UPGen::Sleep(500);
	pwrite_Register("Colpr_Addr", comboBox_pulseraddr->currentIndex());
	UPGen::Sleep(500);

	for (int i = 0; i < 1024; i++) {
		pwrite_Register("PlsrDac", i);
		UPGen::Sleep(333);
		//ibwrt(m_Device, command, strlen(command));
		UPGen::Sleep(250);
		//ibrd(m_Device, answer, sizeof(answer));
		QString MyString;
		int j = 0;
		while (answer[j] != 10) {
			MyString.append(answer[j]);
			j++;
		}
		textEdit_testresult->append(MyString);
	}

	QString temp1;
	if(hUSB->HandlePresent()) {
	  dcsInst->UpdateMeasurements();
	  temp1.setNum(dcsInst->GetNTCTemperature());
	}

	textEdit_testresult->append("Pulser Scan");

	QTime t1 = QTime::currentTime();
	textEdit_testresult->append(t1.toString());

	QDate d1 = QDate::currentDate();
	textEdit_testresult->append(d1.toString());
	
	temp0.prepend("T_i: ");
	textEdit_testresult->append(temp0);

	temp1.prepend("T_f: ");
	textEdit_testresult->append(temp1);

	// release mouse cursor
	setCursor(Qt::ArrowCursor);
}

void MainForm::runBoardTest()
{
	txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);\n"));
	cmdStartBoardTest->setEnabled(false);
	//txtBoardTest->clear();
	txtBoardTest->appendPlainText ("\n==========================================================================\n");
	txtBoardTest->appendPlainText ("Starting BoardTest for board " + QString::number(hUSB->GetId()) + " with Adapter Card " + QString::number(dcsInst->GetId()));
	txtBoardTest->appendPlainText ("Loading FPGA Firmware");
	if (BitfileLineEdit->text()=="") openBitFileDialog();
	if (BitfileLineEdit->text()=="")
	{
		txtBoardTest->appendPlainText ("canceled by user");
		cmdStartBoardTest->setEnabled(true);
		return;
	}
	connect(FPGAStatusLineEdit, SIGNAL(textChanged ( QString )),  this, SLOT(runBoardTest_configured( QString )));
	confFPGA();
}

void MainForm::runBoardTest_configured(QString text)
{
	txtBoardTest->appendPlainText (text);
	if (text=="done")
	{
		disconnect(FPGAStatusLineEdit, SIGNAL(textChanged ( QString )),  this, SLOT(runBoardTest_configured( QString )));
	} else return;

	txtBoardTest->appendPlainText ("Powering FE up");
	// Power UP
	PowerFEUp();
	UPGen::Sleep(1000);

	double dev1 = (fabs (VsetD1Box->value() - vddd1VLCD->value())) / VsetD1Box->value();
	double dev2 = (fabs (VsetA1Box->value() - vdda1VLCD->value())) / VsetA1Box->value();
	double dev3 = (fabs (VsetD2Box->value() - vddd2VLCD->value())) / VsetD2Box->value();
	double dev4 = (fabs (VsetA2Box->value() - vdda2VLCD->value())) / VsetA2Box->value();
	if (dev1 > 0.01 || dev2 > 0.01 || dev3 > 0.01 || dev4 > 0.01)
	{
		txtBoardTest->appendPlainText ("ERROR: Voltage could not be set!");
		txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		cmdStartBoardTest->setEnabled(true);
		PowerFEDown();
		return;
	}

	// load config
	txtBoardTest->appendPlainText ("Loading config");
	if (readGlobalFileLineEdit->text()=="") openReadGlobalFileDialog();
	if (readGlobalFileLineEdit->text()=="")
	{
		txtBoardTest->appendPlainText ("ERROR: no config selected!");
		PowerFEDown();
		txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		cmdStartBoardTest->setEnabled(true);
	}
	read_GlobalFile("");
	UPGen::Sleep(1000);
	saveGloChanges();
	UPGen::Sleep(1000);

	txtBoardTest->appendPlainText ("Enable 160 MHz");
	// enable160 MHz Clock
//	myUSBpix->enable_160Mbps_data_rate();

	txtBoardTest->appendPlainText ("Start SyncScan");
	// start sync scan
	connect(PSOverflowLineEdit, SIGNAL(textChanged ( QString )),  this, SLOT(runBoardTest_syncscan_done( QString )));
	StartSyncScan();
}

void MainForm::runBoardTest_syncscan_done(QString text)
{
	txtBoardTest->appendPlainText (text);
	if (text == "Done")
	{
		txtBoardTest->appendPlainText ("Sync done");
		disconnect(PSOverflowLineEdit, SIGNAL(textChanged ( QString )),  this, SLOT(runBoardTest_syncscan_done( QString )));
		UPGen::Sleep(100);
	} else return;

	if (chkIncludeGloballRegisterTest->isChecked())
	{
		txtBoardTest->appendPlainText ("Starting Global Register Test");
		connect(this, SIGNAL(TESTNR ( int )),  this, SLOT(runBoardTest_globalregtest_done( int )));
		RegTestQuantitySpinBox->setValue(10);
		test_GlobalRegister();
	}
	else
	{
		txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);\n"));
		cmdStartBoardTest->setEnabled(true);
		if (chkSwitchChipOff->isChecked()) PowerFEDown();
	}
}

void MainForm::runBoardTest_globalregtest_done(int testnr)
{
	if (testnr < RegTestQuantitySpinBox->value()) return;
	disconnect(this, SIGNAL(TESTNR ( int )),  this, SLOT(runBoardTest_globalregtest_done( int )));

	if (RegErrorNrlcdNumber->value()>0)
	{
		txtBoardTest->appendPlainText ("ERROR: " + QString::number(RegErrorNrlcdNumber->value()) + " errors during Global Register Test!");
		txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		cmdStartBoardTest->setEnabled(true);
		PowerFEDown();
		return;
	} else txtBoardTest->appendPlainText ("Global Register Test passed");

	if (chkIncludePixelRegisterTest->isChecked())
	{
		txtBoardTest->appendPlainText ("Pixelregister Test running");
		QTimer::singleShot(500,this, SLOT(runBoardTest_pixelregtest()));
	}
	else
	{
		txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);\n"));
		cmdStartBoardTest->setEnabled(true);
		if (chkSwitchChipOff->isChecked()) PowerFEDown();
	}
}

void MainForm::runBoardTest_pixelregtest()
{
	write_Pixel();
	QApplication::processEvents();
	UPGen::Sleep(100);
	connect (this, SIGNAL(HITBUSLCD(int)), this, SLOT(runBoardTest_pixelregtest_hitbus(int))); 
	connect (this, SIGNAL(CAP0LCD(int)), this, SLOT(runBoardTest_pixelregtest_cap0(int))); 
	connect (this, SIGNAL(CAP1LCD(int)), this, SLOT(runBoardTest_pixelregtest_cap1(int))); 
	connect (this, SIGNAL(TDACLCD(int)), this, SLOT(runBoardTest_pixelregtest_tdac(int))); 
	connect (this, SIGNAL(FDACLCD(int)), this, SLOT(runBoardTest_pixelregtest_fdac(int))); 
	connect (this, SIGNAL(ENABLELCD(int)), this, SLOT(runBoardTest_pixelregtest_enable(int))); 
	runBoardTest_pixelregtest_errors=0;
	runBoardTest_pixelregtest_tests=0;
	TestPatternComboBox->setCurrentIndex(2);

	QApplication::processEvents();
	testPixelReg();
}

void MainForm::runBoardTest_pixelregtest_done(int error, QString testreg)
{
	runBoardTest_pixelregtest_errors += error;
	runBoardTest_pixelregtest_tests++;
	if (error>0)
	{
		txtBoardTest->appendPlainText ("ERROR: " + testreg + ": " + QString::number(error) + " errors occured");
	}

	if (runBoardTest_pixelregtest_tests==6)
	{
		disconnect (this, SIGNAL(HITBUSLCD(int)), this, SLOT(runBoardTest_pixelregtest_hitbus(int))); 
		disconnect (this, SIGNAL(CAP0LCD(int)), this, SLOT(runBoardTest_pixelregtest_cap0(int))); 
		disconnect (this, SIGNAL(CAP1LCD(int)), this, SLOT(runBoardTest_pixelregtest_cap1(int))); 
		disconnect (this, SIGNAL(TDACLCD(int)), this, SLOT(runBoardTest_pixelregtest_tdac(int))); 
		disconnect (this, SIGNAL(FDACLCD(int)), this, SLOT(runBoardTest_pixelregtest_fdac(int))); 
		disconnect (this, SIGNAL(ENABLELCD(int)), this, SLOT(runBoardTest_pixelregtest_enable(int))); 

		if (runBoardTest_pixelregtest_errors > 0)
		{
			
			txtBoardTest->appendPlainText ("ERROR: Pixelregister Test was not successful!");
			txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);\n"));
		}
		else
		{
			txtBoardTest->appendPlainText ("Pixelregister passed successfully!");
			txtBoardTest->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);\n"));
		}

		runBoardTest_pixelregtest_errors=0;
		runBoardTest_pixelregtest_tests=0;
		cmdStartBoardTest->setEnabled(true);

		if (chkSwitchChipOff->isChecked()) PowerFEDown();
	}
}

void MainForm::enableRJ45()
{
	unsigned char data = 0xFF;
	myUSBpix->WriteRegister(CS_ENABLE_RJ45, data);
}

void MainForm::disableRJ45()
{
	unsigned char data = 0x00;
	myUSBpix->WriteRegister(CS_ENABLE_RJ45, data);
}

void MainForm::readRJ45()
{
	lcdRJ45->setBinMode();
	lcdRJ45->display(myUSBpix->ReadRegister(CS_ENABLE_RJ45));
}

void MainForm::initChip()
{
	chkIncludeGloballRegisterTest->setChecked(false);
	chkIncludePixelRegisterTest->setChecked(false);
	chkSwitchChipOff->setChecked(false);

	runBoardTest();
}

void MainForm::setTLUdelay()
{
	myUSBpix->WriteRegister(CS_TLU_TRIGGER_DATA_DELAY, spinTLUDelay->value());
}

void MainForm::readTLUdelay()
{
	spinTLUDelay->setValue(myUSBpix->ReadRegister(CS_TLU_TRIGGER_DATA_DELAY));
}

void MainForm::setTLUlength()
{
	myUSBpix->WriteRegister(CS_TLU_TRIGGER_DATA_LENGTH, spinTLULength->value());
}

void MainForm::readTLUlength()
{
	spinTLULength->setValue(myUSBpix->ReadRegister(CS_TLU_TRIGGER_DATA_LENGTH));
}



int MainForm::pget_BCIDwithData() {

        //unsigned int SRAMdataRB[SRAM_WORDSIZE];
	for (int i = 0; i< SRAM_WORDSIZE; i++)
		SRAMdataRB[i] = 0;
        //IsmuCBusy->gotosleep(1000);
		unsigned int PDR[100][200]={0};
        //read SRAM, make words
        myUSBpix->ReadSRAM(chipID); int totalwords = SRAM_WORDSIZE/*myUSBpix->total_SRAM_words*/;
		myUSBpix->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE, chipID);
        int i = -1; int j = 0;  int jmax[100] = {0};
        for(int cnt=0; cnt<totalwords; cnt++){
            QString the_word; the_word.setNum(/*myUSBpix->confReg0->SRAMwordsRB*/SRAMdataRB[cnt],10);
            unsigned theword = the_word.toUInt();
            //QString db2; db2.setNum(theword,16); zzCommandLog->append(db2);
            if(theword == 0){ if(i>-1){ break;} else {continue; } }
            unsigned int headercheck = 0xFF0000 & theword;
            //count number of data headers
            if(headercheck==0xe90000){ // this is a dataheader
                i++; j=0; PDR[i][j]=theword;
                //QString DH; DH.setNum(PDR[i][j],16);DH+=" =PDR."; zzCommandLog->append(DH);
                j++; jmax[i]++;
            }
            //count number of data records
            else if(theword!=0){
                // this is a hack for now. The col cannot be greater than 80. For some reason the SRAM has a word which is out of range.
                // This needs to be properly addressed at some point. -- Sourabh
                if(theword <= 10747903){
                 PDR[i][j]=theword; j++; jmax[i]++;
                }
				
            }
			
        }
		//what is the smallest BCID and in which BCID is pixel hit data
		unsigned int BCID_min = 300;
		unsigned int BCID_data = 300;
		for(int icnt=0; icnt<i+1; icnt++){
			 unsigned int BCID = PDR[icnt][0] & 0x0000FF; 
			 //smallest
			 if (BCID < BCID_min)
				 BCID_min = BCID;
			 //pixelhit data
			 if (jmax[icnt]>1)
				 BCID_data = BCID;
		}
		QString  qBC; qBC.setNum(BCID_min);
		qBC.prepend("BC min ");
        zzCommandLog->append(qBC);
		QString  qBCd; qBCd.setNum(BCID_data);
		qBCd.prepend("BC data ");
        zzCommandLog->append(qBCd);
		unsigned int trigger_number = BCID_data - BCID_min + 1;//start counting at 1
		QString  qBCt; qBCt.setNum(trigger_number);
		qBCt.prepend("Trigger Number ");
        zzCommandLog->append(qBCt);

        // Now lets output all the PDR data to the log
        zzCommandLog->append("Parsing LV1 data...");
        // Now this is the real output to the log.
        for(int icnt=0; icnt<i+1; icnt++){
            for(int jcnt=0; jcnt<jmax[icnt]; jcnt++){

                if(jcnt==0){//this is DH
                    //DH = 11101 001 xxxx [3:0]LV1ID [7:0]bcID
                    unsigned int LV1ID = PDR[icnt][jcnt] & 0x000F00;  LV1ID = LV1ID >> 8; QString LV1; LV1.setNum(LV1ID);
                    unsigned int BCID = PDR[icnt][jcnt] & 0x0000FF; QString  BC; BC.setNum(BCID);
                    QString ans = "LV1ID = " + LV1 + ". BCID = " + BC + ".";
                    zzCommandLog->append(ans);
                }
                else{//this is DR
                    //DR = [6:0]col [8:0]row [3:0]tottop [3:0]totbot
                    unsigned int COL = PDR[icnt][jcnt] & 0xFE0000; COL = COL >> 17; QString colad; colad.setNum(COL);
                    unsigned int ROW = PDR[icnt][jcnt] & 0x01FF00; ROW = ROW >> 8; QString rowad; rowad.setNum(ROW);
                    unsigned int TotT = PDR[icnt][jcnt] & 0x0000F0; TotT = TotT >> 4; QString ttop; ttop.setNum(TotT);
                    unsigned int TotB = PDR[icnt][jcnt] & 0x00000F; QString tbot; tbot.setNum(TotB);
                    QString ans = "Col = " + colad + "   Row = " + rowad + "  Tot = [" + ttop + "," + tbot + "]";
                    zzCommandLog->append(ans);
                   
                }
            }
        }

	if (trigger_number >0 && trigger_number < 300)
        return trigger_number;
	else
		return 0;

}

void MainForm::PlsrDelayScan(int num){ // setting num=15 seems to be a pretty good choice

	//Creates an output file
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","/home/SRAM.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite))
	{
		QMessageBox::warning(this, tr("Error"),"Failed to open/create file", QMessageBox::Ok);
	}
	QTextStream outstream(&file);

	int i;
	int temp;
	int avgSum;
	float temp1;
    //confmode
	zzStreamLineEdit->setText(" 10110 1000 1010");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("000111");
	QString DACStr; 
	DACStr.setNum(pread_Register("PlsrDAC"));

	// The following code runs through all the possible delay values, and recode all the trigger number
	for(i=1; i<63; i++){
		
		QString DelayStr;
		DelayStr.setNum(i);
		//confmode
		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		pwrite_Register("Delay", i);
		
		

		avgSum=0;
		for (int k=0; k<num; k++){

			myUSBpix->ClearSRAM(chipID);
			// Send the signal somehow: runmode, CAL, 39 0's, trigger
			zzStreamLineEdit->setText(" 10110 1000 1010");
			zzStreamLineEdit->insert(zzChipIDLineEdit->text());
			zzStreamLineEdit->insert("111000");
			emit xzSend();
			zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
			emit xzSend();

			avgSum+=pget_BCIDwithData();
		}


		temp1 = ((float) (avgSum/num));

		if(((float)(temp1 - avgSum/num)) > 0.5){
			temp = avgSum/num + 1;
		} else {
			temp = avgSum/num;
		}
		
		

		QString TNStr;
		TNStr.setNum(temp);
		//pget_LV1data(1);


		//writes the output to the file
		QString ans = "PlsrDAC " + DACStr + " " + "PlsrDelay " + DelayStr + " " +  "Trigger Number " + TNStr;
		pDebugBox->append(ans);
		//outstream << DACStr.toUInt() << "\t" << DelayStr.toUInt() << "\t" << TNStr.toUInt() << endl;
		outstream << DelayStr.toUInt() << "\t" << TNStr.toUInt() << endl;
	}
	
}



void MainForm::IDACScan(int num, int init, int last, int step_size){ //a better approach would be to use average values and reject irregular data (that are due to instability) like the
	// TimeWalk function does.

	//Creates a file	
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","../../RootScripts/Josephhome/IDACScan.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite))
	{
		QMessageBox::warning(this, tr("Error"),"Failed to open/create file", QMessageBox::Ok);
	}
	QTextStream outstream(&file);

	int i,j;

	for (j=init; j<=last; j+=step_size){

		/*

		QString index;
		index.setNum(j);


		QFile file2(index.append(".txt"));

		if(!file2.open(QIODevice::ReadWrite))
		{
		QMessageBox::warning(this, tr("Error"),"Failed to open/create file", QMessageBox::Ok);
		}
		QTextStream outstream2(&file2);
		*/

		//j = 180;

		int prev = 0; 
		int temp = 0;
		int beginning = 0;
		int end = 0;
		int avgSum = 0;
		bool first = true;
		bool sec = false;
		float temp1;
		QString prevStr;
		QString tempStr;
		QString begStr;
		QString endStr;

		myUSBpix->ClearSRAM(chipID);
		QString RampStr;
		//confmode
		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		pwrite_Register("PlsrIdacRamp", j);
		RampStr.setNum(j);



		QString DelayStr;
		DelayStr.setNum(1);

		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		pwrite_Register("Delay", 1);

		//myUSBpix->ClearSRAM();
		//// Send the signal somehow: runmode, CAL, 39 0's, trigger
		//zzStreamLineEdit->setText(" 10110 1000 1010");
		//zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		//zzStreamLineEdit->insert("111000");
		//emit xzSend();
		//zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
		//emit xzSend();

		//prev = pget_BCIDwithData();

		avgSum=0;
		for (int k=0; k<num; k++){

			myUSBpix->ClearSRAM(chipID);
			// Send the signal somehow: runmode, CAL, 39 0's, trigger
			zzStreamLineEdit->setText(" 10110 1000 1010");
			zzStreamLineEdit->insert(zzChipIDLineEdit->text());
			zzStreamLineEdit->insert("111000");
			emit xzSend();
			zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
			emit xzSend();
			avgSum+=pget_BCIDwithData();
		}

		temp1 = ((float) (avgSum/num));

		if(((float)(temp1 - avgSum/num)) > 0.5){
			prev = avgSum/num + 1;
		} else {
			prev = avgSum/num;
		}



		for(i=2; i<63; i++){

			myUSBpix->ClearSRAM(chipID);
			// QString DelayStr;
			// DelayStr.setNum(i);
			//confmode
			zzStreamLineEdit->setText(" 10110 1000 1010");
			zzStreamLineEdit->insert(zzChipIDLineEdit->text());
			zzStreamLineEdit->insert("000111");
			emit xzSend();
			pwrite_Register("Delay", i);
			DelayStr.setNum(i);

			

			avgSum=0;
			for (int k=0; k<num; k++){

				myUSBpix->ClearSRAM(chipID);
				// Send the signal somehow: runmode, CAL, 39 0's, trigger
				zzStreamLineEdit->setText(" 10110 1000 1010");
				zzStreamLineEdit->insert(zzChipIDLineEdit->text());
				zzStreamLineEdit->insert("111000");
				emit xzSend();
				zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
				emit xzSend();
				avgSum+=pget_BCIDwithData();
			}
			

			temp1 = ((float) (avgSum/num));

			if(((float)(temp1 - avgSum/num)) > 0.5){
				temp = avgSum/num + 1;
			} else {
				temp = avgSum/num;
			}

			/*
			zzCommandLog->append(DelayStr);
			prevStr.setNum(prev);
			tempStr.setNum(temp);
			zzCommandLog->append(prevStr);
			zzCommandLog->append(tempStr);
			*/

			/*
			if(prev<temp && first){
			prev = temp;
			beginning = i;
			continue;
			}
			*/


			if( prev == temp && first){
				prev = temp;
				beginning = i;
			} else if (first){
				prev = temp;
				beginning = i;
				first = false;
				sec = true;
				// continue;
			}

			if (prev == temp && sec){
				prev = temp;
				end = i;
			} else if (sec){
				prev = temp;
				end = i;
				sec = false;
				//break;
			}

			/*
			begStr.setNum(beginning);
			endStr.setNum(end);
			zzCommandLog->append(begStr);
			zzCommandLog->append(endStr);
			*/

			//QString TNStr;
			//TNStr.setNum(prev);
			//pget_LV1data(1);



			//QString ans = "PlsrDAC " + DACStr + " " + "PlsrDelay " + DelayStr + " " +  "Trigger Number " + TNStr;
			//zzCommandLog->append(ans);
			//pDebugBox->append(ans);
			//outstream2 << DelayStr.toUInt() << "\t" << tempStr.toUInt() << endl;
		}

		/*
		begStr.setNum(beginning);
		endStr.setNum(end);
		zzCommandLog->append(begStr);
		zzCommandLog->append(endStr);
		*/

		float DACUnit = ((float) end) - ((float) beginning);

		/*
		QString DACUStr;
		DACUStr.setNum(DACUnit);
		zzCommandLog->append(DACUStr);
		*/

		float step = 25.0/DACUnit;
		QString stepStr;
		stepStr.setNum(step);
		//zzCommandLog->append(stepStr);

		//RampStr.setNum(j);

		outstream << RampStr.toUInt() << "\t" << stepStr.toFloat() << endl;

	}

	file.close();
}


void MainForm::TimeWalk(int DACinitial, int DACFinal, int step_size, int delay_init, float delay_step){
	// DACinitial: the starting PlsrDAC value for the scan
	// DACfinal: the last PlsrDAC value included in the scanning range

	// Creates the output file
	QString tempstring=QFileDialog::getSaveFileName(this,"Save File","../../RootScripts/Josephhome/TIMEWALK.txt");
	QFile file(tempstring);
	if(!file.open(QIODevice::ReadWrite))
	{
		QMessageBox::warning(this, tr("Error"),"Failed to open/create file", QMessageBox::Ok);
	}
	QTextStream outstream(&file);

	/*
	// Sets the DAC current to 180, so we will have a delay step of 1ns
	myUSBpix->ClearSRAM();
	// confmode
	zzStreamLineEdit->setText(" 10110 1000 1010");
	zzStreamLineEdit->insert(zzChipIDLineEdit->text());
	zzStreamLineEdit->insert("000111");
	emit xzSend();
	pwrite_Register("PlsrIdacRamp", 20);
	*/
	
	// indices for the loops
	int i; // for PlsrDAC
	int j; // for plsrDelay
	int k; // for the multiple injections
	int delay_start = delay_init; // 10 before the modification
	int delay_stop;
	int trig_num_previous = 0;
	int trig_num_present;
	int trigNum;
	double time_walk = 0.0;
	//int trigNum1=0;
	//int trigNum2=0;
	int temp;
	int prevTemp;
	double avgSum = 0.0;
	int count1=0;
	//int count2=0;
	//int avgHolder = 0;
	//int temp1;
	//double delayArr[15] = {0.0};
	double trigTemp = 0;
	bool disregard = false;
	

	// The first loop begins here.
	 for(i=DACinitial;i>=DACFinal; i-=step_size){

		 myUSBpix->ClearSRAM(chipID);

		 disregard = false;
		 //int startArr[5] = {0};
		
		 //count2=0;
		 //avgHolder=0;
		 //int count1=0;
		
		/*
		QString magStr;
		magstr.setNum(i);
		*/

		// First injection

		// confmode
		
		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		pwrite_Register("PlsrDAC", i);
		
		//i=320;
		//pwrite_Register("PlsrDAC", 320);
		
		//when PlsrDAC = 190, the resulting time_walk should be about 252.897
		//i=190;
		//pwrite_Register("PlsrDAC", 190);
		
		// Calibrate the parameter.
		//delay_stop = 1; 

		QString DelayStr;
		DelayStr.setNum(delay_start);
		// confmode
		zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		pwrite_Register("Delay", delay_start);
		
		//// /*
		//myUSBpix->ClearSRAM();
		//// Send the signal somehow: runmode, CAL, 39 0's, trigger
		//zzStreamLineEdit->setText(" 10110 1000 1010");
		//zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		//zzStreamLineEdit->insert("111000");
		//emit xzSend();
		//zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
		//emit xzSend();

		//trigNum1 = pget_BCIDwithData();

		////Second Injection

		//
		//myUSBpix->ClearSRAM();
		//// Send the signal somehow: runmode, CAL, 39 0's, trigger
		//zzStreamLineEdit->setText(" 10110 1000 1010");
		//zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		//zzStreamLineEdit->insert("111000");
		//emit xzSend();
		//zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
		//emit xzSend();

		//trigNum2 = pget_BCIDwithData();

		// */
		
		// /*

		for(int l=0; l<5; l++){

			myUSBpix->ClearSRAM(chipID);
			// Send the signal somehow: runmode, CAL, 39 0's, trigger
			zzStreamLineEdit->setText(" 10110 1000 1010");
			zzStreamLineEdit->insert(zzChipIDLineEdit->text());
			zzStreamLineEdit->insert("111000");
			emit xzSend();
			zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
			emit xzSend();

			if(l == 0){
				prevTemp = pget_BCIDwithData();
				//startArr[l] = prevTemp;
			} else{
				
				prevTemp = temp;
				temp = pget_BCIDwithData();
				//startArr[l] = temp;

				if((prevTemp != temp) || (prevTemp == 0)){
					disregard = true;
					break;
				}

				
			}
			/*
			if(temp == 0){
				continue;
			} else {
				avgHolder+=temp;
				count2++;
			}

			*/
		}

		if(disregard) {
			continue;
		} else {
			trig_num_previous = temp;
			trigNum = trig_num_previous;
			trigTemp = trig_num_previous;
		}

		// */

		// /*

		//if(trigNum1 == 0 && trigNum2 == 0){
		//	
		//	
		//	continue;
		//} else if(trigNum1 == 0){
		//	trig_num_previous = trigNum2;
		//	trigNum = trigNum2;
		//	outstream << trigNum1 << "\t" <<  trigNum2 << endl;
		//} else if(trigNum2 == 0){
		//	trig_num_previous = trigNum1;
		//	trigNum = trigNum1;
		//	outstream << trigNum1 << "\t" <<  trigNum2 << endl;
		//} else{
		//	
		//	/*
		//	//for debugging
		//	QString trigStr1;
		//	QString trigStr2;
		//	trigStr1.setNum(trigNum1);
		//	trigStr2.setNum(trigNum2);
		//	zzCommandLog->append("the trigger Numbers are: \n");
		//	zzCommandLog->append(trigStr1);
		//	zzCommandLog->append(trigStr2);
		//	zzCommandLog->append("\n");
		//	*/

		//	trigTemp = (trigNum1 + trigNum2)/2;
		//	trig_num_previous = (trigNum1 + trigNum2)/2;
		//	trigNum = trig_num_previous;

		//	outstream << trigNum1 << "\t" <<  trigNum2 << "\t" << trigTemp << endl;
		//}

		// */
		
		/*
		if(count2==0){
			continue; //jump back to the first loop
		} else{
			trig_num_previous = avgHolder/count2;
			trigNum = trig_num_previous;
		}

		*/

		/*
		trig_num_previous = pget_BCIDwithData(); 
		trigNum = trig_num_previous; // The original trigger number.
		*/

		// /*
		// for debugging

				
		//QString stop;
		//QString start;
		//QString prevStr;
		//QString presStr;
		//// */
		//QString DACStr;
		//
		//// /*
		//QString trigStr;
		//QString trigStr1;
		//QString trigStr2;

		//QString avgStr;
		//QString count1Str;
		//// */

		// confmode
		/*zzStreamLineEdit->setText(" 10110 1000 1010");
		zzStreamLineEdit->insert(zzChipIDLineEdit->text());
		zzStreamLineEdit->insert("000111");
		emit xzSend();
		DACStr.setNum(pread_Register("PlsrDAC"));
		*/////DACStr.prepend("PlsrDAC : ");
		//// /*
		//prevStr.setNum(trig_num_previous);
		//trigStr.setNum(trigNum);
		///*trigStr1.setNum(trigNum1);
		//trigStr2.setNum(trigNum2);*/
		//prevStr.prepend("prev: ");
		//zzCommandLog->append(prevStr);
		//zzCommandLog->append(DACStr);
		//zzCommandLog->append(trigStr);
		///*zzCommandLog->append(trigStr1);
		//zzCommandLog->append(trigStr2);*/
		// // */

		
		//bool hasData = true;

		count1 = 0;
		avgSum = 0.0;
		
		for (k=0; k<15; k++){
			
			/*
			count1 = 0;
			avgSum = 0.0;
			*/

			delay_stop = delay_start;

			for(j=delay_start+1; j<63; j++){

				/*zzCommandLog->append("PulserDAC:");
				zzCommandLog->append(DACStr);
				zzCommandLog->append("\n");*/

				// confmode
				
				zzStreamLineEdit->setText(" 10110 1000 1010");
				zzStreamLineEdit->insert(zzChipIDLineEdit->text());
				zzStreamLineEdit->insert("000111");
				emit xzSend();
				pwrite_Register("Delay", j);

				//for debugging
				//DelayStr.setNum(j);

				/*
				count1 = 0;
				avgSum = 0;

				for (k=0; k<20; k++){

				myUSBpix->ClearSRAM();
				// Send the signal somehow: runmode, CAL, 39 0's, trigger
				zzStreamLineEdit->setText(" 10110 1000 1010");
				zzStreamLineEdit->insert(zzChipIDLineEdit->text());
				zzStreamLineEdit->insert("111000");
				emit xzSend();
				zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
				emit xzSend();

				temp = pget_BCIDwithData();

				if(temp == 0){
				continue;
				}else{
				avgSum += temp;
				count1++;
				}

				}
				*/


				myUSBpix->ClearSRAM(chipID);
				// Send the signal somehow: runmode, CAL, 39 0's, trigger
				zzStreamLineEdit->setText(" 10110 1000 1010");
				zzStreamLineEdit->insert(zzChipIDLineEdit->text());
				zzStreamLineEdit->insert("111000");
				emit xzSend();
				zzStreamLineEdit->setText("10110 0100 000000000000000000000000000000000000000 11101");
				emit xzSend();
				
								
				trig_num_present = pget_BCIDwithData();

				/*
				if (count1 ==0){
				break;
				} else{
				trig_num_present = avgSum/count1;
				}
				*/

				/*
				if(trig_num_present == 0) {
				hasData = false;
				break;
				}
				*/

				if(trig_num_present > trig_num_previous){
					delay_stop = j;
					break;
				} else {
					trig_num_previous = trig_num_present;
				}

				//// /*
				////for debugging
				//prevStr.setNum(trig_num_previous);
				////prevStr.prepend("prev in the loop ");
				//presStr.setNum(trig_num_present);
				////presStr.prepend("pres in the loop ");
				////zzCommandLog->append(prevStr);
				////zzCommandLog->append(presStr);
				/*zzCommandLog->append("Delay:");
				zzCommandLog->append(DelayStr);
				zzCommandLog->append("\n");*/
				// */



			}


			//// /*
			//for debugging
			//start.setNum(delay_start);
			//start.prepend("delay_start ");
			//stop.setNum(delay_stop);
			//stop.prepend("delay_stop ");
			//prevStr.setNum(trig_num_previous);
			//presStr.setNum(trig_num_present);
			//zzCommandLog->append("prevStr:");
			//zzCommandLog->append(prevStr);
			//zzCommandLog->append("presStr:");
			//zzCommandLog->append(presStr);
			//zzCommandLog->append("start:");
			//zzCommandLog->append(start);
			//zzCommandLog->append("stop:");
			//zzCommandLog->append(stop);
			//zzCommandLog->append("\n");
			// zzCommandLog->append(DelayStr);

			// */
			/*
			double delay_step1 = 0.961;

			if (delay_stop == delay_start){
				continue;
			} else{

				time_walk = 25*trigNum + 25 - (delay_stop - delay_start)*delay_step1; 
				//QString TWStr;
				//TWStr.setNum(time_walk);

				avgSum += time_walk;
				count1++;
			}
			*/			

			if (delay_stop == delay_start){
				continue;
			} else{

				avgSum += delay_stop;
				count1++;

			}

			//delayArr[k] = delay_stop;

			// temp1 = avgSum/count1;

			// /*
			//For debugging purposes
			///*avgStr.setNum(avgSum);
			//count1Str.setNum(count1);
			//zzCommandLog->append(avgStr);
			//zzCommandLog->append(count1Str);*/
			// */
		}

		
		if (count1 == 0) {

			
			/*for(int l=0; l<15; l++){
				outstream << delayArr[l] << "\t" << endl;
			}

			outstream << "\n" << endl;		*/	

			continue;
		} else{

			//double delay_step1 = 0.961;
			time_walk = 25*trigNum + 25 - (((double) (avgSum/count1))- delay_start)*delay_step; 
			
			//For now, we output the delay_start and delay_stop
			//if(hasData) {
			//outstream << trigTemp << "\t" <<  delay_start << "\t" << ((double) (avgSum/count1)) << endl;
			
			
			//outstream << DACStr.toInt() << "\t" << time_walk << endl;
			outstream << i << "\t" << time_walk << endl;
			//outstream << DACStr.toInt() << "\t" << avgSum/count1 << endl;

			/*for(int l=0; l<5; l++){
				outstream << startArr[l] << "\t" << endl;
			}

			outstream << "\n" << endl;

			
			for(int l=0; l<15; l++){
				outstream << delayArr[l] << "\t" << endl;
			}*/
			

			outstream << "\n" << endl;
			//} else break;

		} 
		// The first loop ends here

	
	}

	file.close();
}
