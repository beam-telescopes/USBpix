/*
* ConfigRegister is a interface class between the USBpix-board and the GUIs
* It contains all functionality to handle thee test system configuration and handle the read data
*/

#ifndef CONFIGREGISTER_H
#define CONFIGREGISTER_H

#include "defines.h"
#include "SiLibUSB.h"
#include "Clusterizer.h"
#include "Records.h"

#include "dllexport.h"
#include "HistogramTypes.h"

#include "RawDataHistogrammer.h"

struct DllExport StrbOutputGate {
  bool m_disable_sca_strb;
  bool m_disable_gpac_strb;
  short m_fine_strb_select;
  short m_tx2_output;

  StrbOutputGate():
    m_disable_sca_strb(false), 
    m_disable_gpac_strb(false),
    m_fine_strb_select(0),
    m_tx2_output(0)
    { };

  unsigned char encode()
  {
    unsigned char ret = 0
      | (((unsigned char)m_disable_sca_strb) << 0)
      | (((unsigned char)m_disable_gpac_strb) << 1)
      | ((((unsigned char)m_tx2_output) & 1) << 2)
      | ((((unsigned char)m_fine_strb_select) & 0xf) << 4)
    ;
    return ret;
  }
};

class DllExport ConfigRegister {

public:
	ConfigRegister(SiUSBDevice * Handle, bool isFEI4B, bool MultiChipWithSingleBoard);
	~ConfigRegister();

public:
	void SetUSBHandle(SiUSBDevice * Handle); // sets USB-handle tp current boards. This function needs to be called by the applications, everytime the USB-handle changed (Plug'n'Play)
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
	void WriteFEMemBlockRAMReset(); // resets BlockRAM. FE-configurations are stored in the BlockRAM
	void resetRunModeAdd();
	void SetCalibrationMode(); // sets system to calibration mode (HIT-HISTOGRAMMING)
	void SetRunMode(); // sets system to run mode (FULL DATA STORAGE)
	void SetTOTMode(); // sets system to tot mode (TOT-HISTOGRAMMING)
	void SetTLUMode(); // sets system to TLU Mode (FULL DATA Storage + Trigger Number recieved from TLU)
  void AutoSRAMErase();
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
	void stopXCK(bool status); // stops XCK output
	void SetAuxClkFreq(int freq); // sets auxiliary clk frequency to 40 MHz / n. n is 1 for freq = 0, 2 for freq[0] = 1, 4 for freq[0] = 0 && freq[1] = 1, 8 for freq[0] = 0 && freq[1] = 0 && freq[2] = 1, ...
	void incr_phase_shift(); // increments incoming data synchronization by 1/256 clock duration
	void decr_phase_shift(); // decrements incoming data synchronization by 1/256 clock duration
	bool check_phase_shift_overflow(); // checks for overflow of phase shift
	double StartSyncCheck(double min_BitErrorRate); // starts synchro checker until min_BitErrorRate was achieved
	bool StartSyncScan(double min_BitErrorRate);  // Scans for opimal Sync clk-phase 
	void StoreSyncCheckPattern(); // stores new SyncCheckPattern
	void ResetSyncCheckPattern(); // resets SyncCheckPattern
	void ResetSRAMCounter(); // set SRAM address to 0
	void SetSRAMCounter(int startadd); // set RAM address to any value
	void ReadSRAM(); // reads complete SRAM, further data handling dependent on system mode
	void ReadSRAM(int scan_nr); // reads complete SRAM, further data handling dependent on system mode and fills correct scansteps of ConfData
	void ReadSRAM(int StartAdd, int NumberOfWords); // reads SRAM partially
	void ClearSRAM(); // clears SRAM
	void WriteSRAM(int StartAdd, int NumberOfWords); // writes SRAM, only for debugging purposes needed
  void GetConfHisto(int col, int row, int confStep, int readout_channel, int &Value);
  void GetTOTHisto(int col, int row, int tot, int &Value, int readout_channel);
  void GetTOTHisto(int col, int row, int tot, int &Value);
	void ClearTOTHisto();
	void ClearConfHisto();
	void GetHitLV1HistoFromRawData(int LV1ID, int& Value, int roch);
	void GetLV1IDHistoFromRawData(int LV1ID, int& Value, int roch);
	void GetBCIDHistoFromRawData(int BCID, int& Value, int roch);
	void ClearHitLV1HistoFromRawData();
	void ClearLV1IDHistoFromRawData();
	void ClearBCIDHistoFromRawData();

	void FillHistosFromRawData(bool suppress_tot_14);

	// cluster histograms
	void GetClusterSizeHistoFromRawData(int Size, int& Value);
	void GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value);
	void GetClusterChargeHistoFromRawData(int pCharge, int pSize, int& rValue);
	void GetClusterPositionHistoFromRawData(int pX, int pY, int& rValue);

	bool ClusterRawData(int pColumnRange, int pRowRange, int pTimeRange, int pMinClusterSize, int pMaxClusterSize, int pMaxHitTot, int pMaxEventsIncomplete, int pMaxEventsErrors);		//takes the raw data, sends and sends it to the clusterizer DLP
	void SetChargeCalib(unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge);
	void ResetClusterCounters();

	bool WriteFileFromRawData(std::string filename, bool new_file, bool close_file, int readout_channel); // new raw data format, human & machine readable file format
	bool FinishFileFromRawData(std::string filename);
	bool FileSaveRB(const char *filename, int event_quant, bool attach_data); // old raw data format
	bool CheckDataConsisty(const char * filename, bool attach_data, bool write_summary);
	bool WriteToTHisto(const char *filename);
	bool WriteConfHisto(const char *filename);
	void GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate);
	void BuildWords(); // in run mode: makes array of words out of character array
	bool WriteSRAMWords(char* filename);
	bool WriteSRAMBitsFromWords(char *filename);
	bool WriteSRAMBytes(char* filename);
	bool WriteSRAMBitsFromBytes(char *filename);
	void GetSystemMode(bool &CalMode, bool &TOTMode);
	void SetMeasurementMode(int mode); // selects which events to count (LV1, DH, DR...)
	void StartMeasurement();
	void StopMeasurement();
	void PauseMeasurement();
	void ResumeMeasurement();
	void SetNumberOfEvents(int data);
	int GetCountedEvents();
	void GetScanStatus(bool & scanReady, bool & scanCancelled, bool & scanError, int & scanStep);
	void SetScanReady();
	void SetScanCancelled();
	void SetScanError();
	void ResetScanStatus();
	int GetCurrentPhaseshift();
	void SetCurrentPhaseshift(int value);
	bool CheckRX0State();
	bool CheckRX1State();
	bool CheckRX2State();
	bool CheckExtTriggerState();
	void GetSyncScanResult(double* dataX, double* dataY, int size);
	void GetSRAMWordsRB(unsigned int* data, int size, int chip_index = -1);
	void EnableManEnc(bool on_off);
	void SetManEncPhase(int phase);
  void StopReadout();
  void StartReadout();
  bool ReadoutStopped();
  void SetAdapterCardFlavor(int flavor);
  void FinalizeScan();



	// member variables for scan status
	bool m_scanReady;
	bool m_scanCancelled;
	bool m_scanError;
	int m_scanStep;

private:
	SiUSBDevice * myUSB;

	Clusterizer _clusterizer;	//DLP

	ConfHisto_t ConfHisto;
  TOTHisto_t TOTHisto;

	int current_phaseshift;

	unsigned char SRAMdata[SRAM_BYTESIZE]; // write buffer
	unsigned char SRAMdataRB[SRAM_BYTESIZE]; // read buffer

	void MakeConfHisto(int scan_nr); // makes hit histogram in scan mode / calib mode
	void MakeTOTHisto(); // makes TOT histogram in TOT mode

	bool isCalMode;
	bool isTOTMode;

	unsigned int SRAMwordsRB[SRAM_WORDSIZE]; // read buffer, 32-bit words

	double SyncScanResultsX[201];
	double SyncScanResultsY[201];

	int m_lengthLVL1;

  HitLV1Histo_t HitLV1Histo;
  LV1IDHisto_t LV1IDHisto;
  BCIDHisto_t BCIDHisto;


	time_t start_time;
	time_t stop_time;

	int nr_SR_detected;
	int nr_DH_inconsistent;
	int nr_DR_wo_DH;
	int nr_lv1_jumps;
	int nr_bcid_jumps;

	int full_bcid;
	int full_lv1id;

	bool FEI4B; // Is set in constructor, if FE flavor is FEI4B.

	void write_debug_file();

  bool MultiChipWithSingleBoard;
  bool sram_cleared;
  int GetSRAMByteDataOffset(int readout_channel_id);
  int GetSRAMWordDataOffset(int readout_channel_id);

  std::vector<RecordStream *> record_streams;
  std::vector<RawDataHistogrammer> rawDataHistogrammers;

private:
  StrbOutputGate m_strbOutputGate;
public:
  void UpdateStrbOutputGate();
  void SetDisableScaStrb(bool value) 
    {m_strbOutputGate.m_disable_sca_strb = value; UpdateStrbOutputGate();};
  void SetDisableGpacStrb(bool value) 
    {m_strbOutputGate.m_disable_gpac_strb = value; UpdateStrbOutputGate(); };
  void SetFineStrbDelay(int value);
  void SetTX2Output(short value)
    {m_strbOutputGate.m_tx2_output = value; UpdateStrbOutputGate(); };
};

#endif // CONFIGREGISTER_H
