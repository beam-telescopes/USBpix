#ifndef USBPIX_H
#define USBPIX_H

#include "SiLibUSB.h"
#include "ConfigRegister.h"
#include "ConfigFEMemory.h"
#include "ConfigCCPDMemory.h"
#include "ReadoutStatusRegister.h"
#include "MemoryArbiterStatusRegister.h"
#include "defines.h"

#include <stdexcept>
#include <dllexport.h>

class DllExport USBpix { //: public ConfigRegister

	// public functions in this class
public:
	USBpix(int chip_add_0, int mod_add, SiUSBDevice * Handle0, bool isFEI4B, SiUSBDevice * Handle1 = NULL, int chip_add_1 = 999, bool MultiChipWithSingleBoard = false);
	~USBpix();

	bool StartScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSiz, int InjCount, int MaskStepSiz, int MaskStepCount, int ShiftMask, bool all_DCs, bool special_dc_loop, bool singleDCloop);
	void StartHitORScan();

	void SetUSBHandles(SiUSBDevice * hUSB0, SiUSBDevice * hUSB1); // sets pointers hUSB to correct instances of SiUSBDevice. Needed for Plug'n'Play

	bool MakeMeFEI4A(); // creates a NEW instance of FE-I4A classes and deletes existing FE-I4B classes. Returns false if FE-I4A class are already existing.
	bool MakeMeFEI4B(); // creates a NEW instance of FE-I4B classes and deletes existing FE-I4A classes. Returns false if FE-I4B class are already existing.
	bool FEisFEI4B();   // returns true if FE is FE-I4B.
  void initializeChips(std::vector<int> chipAdds);
  void initializeChips();

private:
  void ReinitializeStatusRegisters();

	// accessor functions for ConfigFEMemory
public:
	//void test_set_chain_value(int chip_addr);
	void SetSlowControlMode(); // sets system to bypass-mode
	void setSelAltBus(bool on_off); // toggels SelAltBus_P
	void setSelCMD(bool on_off);
	void SetCMDMode(); // sets system to default mode (use CMD...)
	void WriteCommand(int the_command, int chip_addr, int GlobalPulseLength=10); // sends command to FE, command definition given in defines.h
  void SetAndWriteCOLPRReg(int colpr_mode, int colpr_addr);
	void WriteGlobal(int chip_addr); // writes complete global configuration
	void WriteGlobalSingleReg(int RegisterNumber, int chip_addr); // writes global register number RegisterNumber 
	void WritePixel(int chip_addr); // writes complete pixel configuration
	void WritePixelSingleLatch(int latch, int chip_addr); // writes one latch in all DCs. Will be overloaded later to write variable DCs
	void WritePixelSingleLatchDC(int latch, int DC, int chip_addr); // writes one latch in given DCs.
	void ShiftPixMask(int latch, int steps, int chip_addr, bool fillOnes=false); // shifts pixel masks - last parameter allows to fill with ones, not zeros
	void ReadGlobal(int chip_addr); // reads complete global configuration, will be overloaded later to read just one global register
	 int ReadGlobalSingleReg(int RegisterNumber, int chip_addr); // reads global register number RegisterNumber
	void ReadPixel(int chip_addr); // reads complete pixel configuration
	//void ReadPixelSingleLatch(int latch, int chip_addr); // reads one latch of all DCs. Will be overloaded later to read variable DCs
	void ReadPixelSingleLatch(int latch, bool bypass, int chip_addr);
	void ReadPixelSingleLatchDC(int latch, int DC, int chip_addr); // reads one latch of the given DC.
	void ReadPixelSingleLatchDC(int latch, int DC, bool bypass, int chip_addr);
	void SetGlobalVal(int the_index, int the_value, int chip_addr); // sets one item in global configuration
	void SetPixelVal(int the_index, int the_value, int latch, int chip_addr); // sets one item in pixel configuration
	void SetPixelVal(int latch, int theDC, int the_DCindex, int the_value, int chip_addr);
	bool ReadGlobalFile(const char * globalfilename, int chip_addr); // reads global configuration from file
	void ReadPixelFile(const char * pixelfilename, int latch, int chip_addr); //reads pixel configuration for one latch from file
	void SaveGlobal(const char * newfilename, int chip_addr); // saves global configuration to file
	void SaveGlobalRB(const char * newfilename, int chip_addr); // saves read-back global configuration to file
	void LoadGlobalDefault(int chip_addr); // loads default configuration
	void SavePixel(const char * newfilename, int latch, int doublecolumn, int chip_addr); //saves pixel configuration for one latch/DC to file
	void SavePixelRB(const char * newfilename, int latch, int doublecolumn, int chip_addr); // saves read-back pixel configuration for one latch/DC to file
	void GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr); // writes value, bitsize and address of one item of global configuration to given addresses
	void GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr); // writes value, bitsize and address of one item of read-back global configuration to given addresses
	void GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, int chip_addr); // writes value, bitsize and address of one item of pixel configuration to given addresses
	//void GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, int chip_addr); // writes value, bitsize and address of one item of read-back pixel configuration to given addresses
	void GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, bool bypass, int chip_addr);
	void SetChipAdd(int new_chip_add, int chip_addr); // sets chip address
	void SetChipAddByIndex(int new_chip_add, int chip_index); // sets chip address, given the index of the chip (only for single board readout)
	void SendReadErrors(int chip_addr); // sends a global pulse to read error counters
	void ReadEPROMvalues(int chip_addr); // Sends global pulse to read the values from EPROM to GR.
	void BurnEPROMvalues(); // Burns to the EPROM whatever is stored in GR. Note that burning a 1 is non reversibel! Does not respect chip ID.
	bool ReadGADC(int GADCselect, int chip_addr); // Configures chip to read value of "voltage_select" and sends global pulse to load GADC to GR40. True if ok, false if fail, esp. if called on FE-I4A!

	void SendBitstream(unsigned char * bitstream, int bitsize, int chip_addr); // sends bitsream to FE
	std::string getGlobalVarName(int Variable);

	void sendBitstreamToAB(int chip_addr);
	void sendBitstreamToC(int chip_addr);
	//ScanChain
	void SetScanChainValue	(int BlockSelect,int index, int val, int chip_addr);
	void GetScanChainValue	(int BlockSelect,int index, int& Size, int&add, int& Value, int chip_addr);
	void GetScanChainValueRB(int BlockSelect,int index, int& Size, int& add, int& Value,int chip_addr);
	void RunScanChain(int ScanChainSelect, USBPixDCS* dcs, double& current_before, double& current_after, bool shift_only, bool se_while_pulse, bool si_while_pulse, int chip_addr);
	void LoadFileToScanChain(int ScanChainSelect, int chip_addr);
	void SaveFileFromScanChain(int ScanChainSelect, int chip_addr);
	void SaveFileFromScanChainRB(int ScanChainSelect, int chip_addr);
	//void ReadScanChainFromFE(int ScanChainSelect, int chip_addr);
	 int IndexToRegisterNumber(int the_index);   // needed to hide FE-I4 / FE-I3 structure differences to outside world

	 // Some CCPD Stuff
	void SetCcpdGlobalVal(int the_index, int the_value); // sets one item in global CCPD configuration
	void SetCcpdColCtrlVal(int the_index, int the_value, int Wr_ID, int col); // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
	void SetCcpdRowCtrlVal(int the_index, int the_value, int Wr_ID, int row, int col); // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
	void SetCcpdv2ColCtrlVal(int the_index, int the_value, int Wr_ID, int col); // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
	void SetCcpdv2RowCtrlVal(int the_index, int the_value, int Wr_ID, int row, int col); // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
	void GetCcpdGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value); // writes value, bitsize and address of one item of global configuration to given addresses
	void GetCcpdGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value); // writes value, bitsize and address of one item of read-back global configuration to given addresses
	void WriteCcpdGlobal(); // writes the global register of CCPD
	void WriteCcpdPixel(int col); // writes the pixel register of CCPD
	void WriteCompleteCcpdShiftRegister(); //writes the complete CCPD shift register with global and pixel register

	void SetCcpdThr(int the_value); // sets the Thr value that is generated using the on pcb DACs
	void SetCcpdVcal(int the_value); // sets the Vcal value that is generated using the on pcb DACs
	void WriteCcpdOnPcbDacs(); // writes the Thr and Vcal value in the DACs and enables the DACs.
	
	void SetCcpdInDacValue(int col, int row, int value); //V2: sets array with InDac values
	void SetCcpdMonitorValue(int col, int row, int value); //V2: sets array with Monitor values
	void ResetCcpdMonitor(); // V2: reset Monitor
	void SetCcpdEnableValue(int row, int value); //V2: sets array with Enable values

	void SetCcpdStripROValue(int value); //V2: sets StripRO values
	void SetCcpdWrLdIDValue(int col, int value); //V2: sets array with WrLdID values
	void SetCcpddirectcurrentValue(int value); //V2: sets directcurrent values
	void SetCcpdampoutValue(int col, int value); //V2: sets array with ampout values
	void SetCcpdsimplepixelValue(int value); //V2: enables simple pixel
	
	void SetCcpdLFPixels(int pixel, int value);
	void SetCcpdLFSw_Ana(int row, int value);
	
	// accessor function for ConfigRegister
public:
	void ResetAll(); // sets all registers in the fpga to zero

	// access to FPGA configuration registers
	int ReadRegister(int CS); // read 8bit register, wrapper for ReadXilinx() to protect against accidental misuse
	void WriteRegister(int CS, int data); // write 8bit register, wrapper for WriteXilinx() to protect against accidental misuse

	//---write strb & LV1 registers---------------
	void WriteStrbSave(unsigned char *data); // saves all parameters for injection / LV1 generating state machine
	void WriteStrbStart(); // start measurements / LV1 FSM
	void WriteStrbStop(); // stop measurements / LV1 FSM

	//---write ConfigSM registers-----------------
	void SetCableLengthReg(int value);
	//void WriteFEMemBlockRAMReset(); // resets BlockRAM. FE-configurations are stored in the BlockRAM
	void resetRunModeAdd();
	void SetCalibrationMode(); // sets system to calibration mode (HIT-HISTOGRAMMING)
	void SetRunMode(); // sets system to run mode (FULL DATA STORAGE)
	void SetTOTMode(); // sets system to tot mode (TOT-HISTOGRAMMING)
	void SetTLUMode(); // sets system to TLU Mode (FULL DATA Storage + Trigger Number recieved from TLU)
	void WriteStrbQuantity(int value); // writes quantity of generated strobe and LV1 signals
	void enableExtLV1(); // sets TriggerMode to 2 => simple external trigger via LEMO or RJ45
	void disableExtLV1(); // disables external triggers via LEMO (TX0) or ethernet connector
	void setTriggerMode(int TriggerMode); // sets mode for external triggers via LEMO, RJ45 or MULTI_IO pins
	void enableTriggerReplicationMaster(); // enables forwarding of triggers via MULTI_IO pins
	void disableTriggerReplicationMaster(); // disables forwarding of triggers via MULTI_IO pins
	void enable_8b10_Decoding(); // enables decoding
	void disable_8b10_Decoding(); // disables decoding
	void enableCMDLV1(); // sets LV1 generating state machine to CMD mode
	void disableCMDLV1(); // sets LV1 generating state machine to external pad mode
	//void EnablePowerChannel(bool on_off, int channel); // disables/enables power channel number "channel", channel defined in defines.h
	void stopXCK(bool status);
	void SetAuxClkFreq(int freq); // sets auxiliary clk frequency to 40 MHz / n. n is 1 for freq = 0, 2 for freq[0] = 1, 4 for freq[0] = 0 && freq[1] = 1, 8 for freq[0] = 0 && freq[1] = 0 && freq[2] = 1, ...
	void incr_phase_shift(int chip_addr); // increments incoming data synchronization by 1/256 clock duration
	void decr_phase_shift(int chip_addr); // decrements incoming data synchronization by 1/256 clock duration
	bool check_phase_shift_overflow(int chip_addr); // checks for overflow of phase shift
	double StartSyncCheck(double min_BitErrorRate, int chip_addr); // starts synchro checker until min_BitErrorRate was achieved
	bool StartSyncScan(double min_BitErrorRate, int chip_addr);  // Scans for opimal Sync clk-phase 
	void StoreSyncCheckPattern(int chip_addr); // stores new SyncCheckPattern
	void ResetSyncCheckPattern(int chip_addr); // resets SyncCheckPattern
	void ResetSRAMCounter(int chip_addr); // set SRAM address to 0
	void SetSRAMCounter(int startadd, int chip_addr); // set RAM address to any value
	void ReadSRAM(int chip_addr); // reads complete SRAM, further data handling dependent on system mode
	void ReadSRAM(int scan_nr, int chip_addr); // reads complete SRAM, further data handling dependent on system mode and fills correct scansteps of ConfData
	void ReadSRAM(int StartAdd, int NumberOfWords, int chip_addr); // reads SRAM partially
	void ClearSRAM(int chip_addr); // clears SRAM
	void WriteSRAM(int StartAdd, int NumberOfWords, int chip_addr); // writes SRAM, only for debugging purposes needed
	void GetConfHisto(int col, int row, int confstep, int &Value, int chip_addr); // writes histogram-value for col, row, step to &Value (needs calibration mode)
	void GetTOTHisto(int col, int row, int tot, int& Value, int chip_addr);
	void ClearTOTHisto(int chip_addr);
	void ClearConfHisto(int chip_addr);
	void GetHitLV1HistoFromRawData(int LV1ID, int& Value, int chip_addr);
	void GetLV1IDHistoFromRawData(int LV1ID, int& Value, int chip_addr); // Gets the LV1 COMMAND ID. LV1 COMMAND ID is constant within one LV1 window in FE-I4
	void GetBCIDHistoFromRawData(int BCID, int& Value, int chip_addr);	 // Gets the BCID histogram.
	void ClearHitLV1HistoFromRawData(int chip_addr);
	void ClearLV1IDHistoFromRawData(int chip_addr);
	void ClearBCIDHistoFromRawData(int chip_addr);

	void FillHistosFromRawData(int pChipAddress);

	// cluster histograms
	void GetClusterSizeHistoFromRawData(int Size, int& Value, int chip_addr);
	void GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value, int chip_addr);
	void GetClusterChargeHistoFromRawData(int pCharge, int pSize, int& rValue, int pChipAddr);
	void GetClusterPositionHistoFromRawData(int pX, int pY, int& rValue, int pChipAddr);
	void ClearClusterSizeHistoFromRawData(int chip_addr);
	void ClearClusterTOTHistoFromRawData(int chip_addr);

	bool ClusterRawData(int pChipAddress, int pColumnRange, int pRowRange, int pTimeRange, int pMinClusterSize, int pMaxClusterSize, int pMaxHitTot, int pMaxEventsIncomplete, int pMaxEventsErrors);	//takes the raw data, converts data to hits and clusters the hits DLP
	void ResetClusterCounters(int pChipAddress);	//resets the trigger counter, set at the beginning of a source scan DLP
	void SetChargeCalib(int pChipAddress, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge);	//sets the charge calibration for the clusterizer DLP

	bool WriteFileFromRawData(std::string filename, int chip_addr, bool new_file, bool close_file); // new raw data format, human & machine readable file format
  void FinishFileFromRawData(std::string filename);
	bool FileSaveRB(const char *filename, int event_quant, bool attach_data, int chip_addr); // old raw data format
	bool CheckDataConsisty(const char * filename, bool attach_data, bool write_summary, int chip_addr);
	bool WriteToTHisto(const char *filename, int chip_addr);
	bool WriteConfHisto(const char *filename, int chip_addr);
	void GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate, int chip_addr);
	void GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate, bool &TluVeto, int chip_addr);	// Overloaded to add TLU veto flag
	void BuildWords(int chip_addr); // in run mode: makes array of words out of character array
	bool WriteSRAMWords(char* filename, int chip_addr);
	bool WriteSRAMBitsFromWords(char *filename, int chip_addr);
	bool WriteSRAMBytes(char* filename, int chip_addr);
	bool WriteSRAMBitsFromBytes(char *filename, int chip_addr);
	void GetSystemMode(bool &CalMode, bool &TOTMode);
	void SetMeasurementMode(int mode); // selects which events to count (LV1, DH, DR...)
	void StartMeasurement();
	void StopMeasurement();
	void PauseMeasurement();
	void ResumeMeasurement();
	void SetNumberOfEvents(int data);
    void SetSramReadoutThreshold(int value);    // threshold in percent
    bool GetTluVetoFlag();                      // readback TLU veto state
	int GetCountedEvents();
	void GetScanStatus(bool & scanReady, bool & scanCancelled, bool & scanError, int & scanStep);
	void SetScanReady();
	void SetScanCancelled();
	void SetScanError();
	void ResetScanStatus();
	int GetCurrentPhaseshift(int chip_addr);
	void SetCurrentPhaseshift(int value, int chip_addr);
	bool CheckRX0State();
	bool CheckRX1State();
	bool CheckRX2State();
	bool CheckExtTriggerState();
	void GetSyncScanResult(double* dataX, double* dataY, int size, int chip_addr);
	void GetSRAMWordsRB(unsigned int* data, int size, int chip_addr);
	void EnableManEnc(bool on_off);
	void SetManEncPhase(int phase);
	void StartCcpdInjections(int duration, int strobeLength, int numberInjections);
	bool StopCcpdInjections();
	void CcpdSingleInject(int strobeLength);
  size_t ConvertChipAddrToIndex(int chip_addr);
  void StartReadout();
  void StopReadout();
  void SetAdapterCardFlavor(int flavor);
  void ResetReadoutStatusRegisters();
  void SetCCPDVersion(int version);

public:
  std::vector<int> ReadoutChannelAssoc;
  void resetReadoutChannelAssoc();

private:
	SiUSBDevice * myUSB0;
	ConfigCCPDMemory * confCCPDMem;
  std::vector<ConfigFEMemory *> confFEMem;
  ConfigFEMemory * confFEBroadcast;

  std::vector<int> myChipAdd;

	int myModAdd;

	bool FEI4B;	// is set true in constructor if the chip os flavour FE-I4B...

	bool FEI4Aexisting;
	bool FEI4Bexisting;

	bool MultiChipWithSingleBoard;
  bool isTot14SuppressionRequired();

public:
  std::vector<ReadoutStatusRegister *> readoutStatusRegisters;
  MemoryArbiterStatusRegister * memoryArbiterStatusRegister;

	ConfigRegister * confReg1;

  int detectReadoutChannel(int chip_addr);
  bool autoDetectReadoutChannels();
  void SetReverseReadoutChannelAssoc(const std::vector<int>& roch_assoc_rev);
  std::vector<int> GetReverseReadoutChannelAssoc();

public:
  void SetDisableScaStrb(bool value);
  void SetDisableGpacStrb(bool value);
  void SetFineStrbDelay(int delay);
  void SetTX2Output(short value);

};

class WrongMultiBoardMode: public std::runtime_error{
  public:
    WrongMultiBoardMode(std::string const& msg): std::runtime_error(msg) {};
};
#endif
