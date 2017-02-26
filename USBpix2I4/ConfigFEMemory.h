/*
 * ConfigFEMemory is a interface class between the USBpix-board and the GUIs
 * It contains all functionality to store/write/read FE - configuration
 * FE - configuration is stored in instances of ConfDataArray
 */

#ifndef CONFIGFEMEMORY_H
#define CONFIGFEMEMORY_H

#include "SiLibUSB.h"
#include "ConfDataArray.h"
#include "ConfigRegister.h"
#include "USBPixI4DCS.h"
#include "defines.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "dllexport.h"

class DllExport ConfigFEMemory {

public:
  ConfigFEMemory(int c_add, SiUSBDevice * Handle, ConfigRegister * cR, bool isFEI4B, int index = 0);
	~ConfigFEMemory();

public:
	//void test_set_chain_value();
	void SetSlowControlMode(); // sets system to bypass-mode
	void setSelAltBus(bool on_off); // toggels SelAltBus_P
	void setSelCMD(bool on_off);
	void SetCMDMode(); // sets system to default mode (use CMD...)
	void SetUSBHandle(SiUSBDevice * hUSB); // sets pointer hUSB to correct instance SiUSBDevice. Needed for Plug'n'Play
	void WriteCommand(int the_command, int GlobalPulseLength=10); // sends command to FE, command definition given in defines.h
	void WriteGlobal(); // READY !!! // writes complete global configuration
	void WriteGlobal(int RegisterNumber); //READY!!! // writes global register number RegisterNumber 
	void WritePixel(); // writes complete pixel configuration
	void WritePixel(int latch); // writes one latch in all DCs. Will be overloaded later to write variable DCs
	void WritePixel(int latch, int DC); // writes one latch in given DCs.
	void ShiftPixMask(int latch, int steps, bool fillOnes=false); // shifts pixel masks - last parameter allows to fill with ones, not zeros
	void ReadGlobal(); // reads complete global configuration, will be overloaded later to read just one global register
	int ReadGlobal(int RegisterNumber); // NOT IMPLEMENTED YET // reads global register number RegisterNumber
	void ReadPixel(); // reads complete pixel configuration
	void ReadPixel(int latch); // reads one latch of all DCs. Will be overloaded later to read variable DCs
	void ReadPixel(int latch, bool bypass);
	void ReadPixel(int latch, int DC); // reads one latch of the given DC.
	void ReadPixel(int latch, int DC, bool bypass);
	void SetGlobalVal(int the_index, int the_value); // sets one item in global configuration
	void SetPixelVal(int the_index, int the_value, int latch); // sets one item in pixel configuration
	void SetPixelVal(int latch, int theDC, int the_DCindex, int the_value);
	bool ReadGlobalFile(const char * globalfilename); // reads global configuration from file
	void ReadPixelFile(const char * pixelfilename, int latch); //reads pixel configuration for one latch from file
	void SaveGlobal(const char * newfilename); // saves global configuration to file
	void SaveGlobalRB(const char * newfilename); // saves read-back global configuration to file
	void LoadGlobalDefault(); // loads default configuration
	void SavePixel(const char * newfilename, int latch, int doublecolumn); //saves pixel configuration for one latch/DC to file
	void SavePixelRB(const char * newfilename, int latch, int doublecolumn); // saves read-back pixel configuration for one latch/DC to file
	bool GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value); // writes value, bitsize and address of one item of global configuration to given addresses
	bool GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value); // writes value, bitsize and address of one item of read-back global configuration to given addresses
	void GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch); // writes value, bitsize and address of one item of pixel configuration to given addresses
	void GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch); // writes value, bitsize and address of one item of read-back pixel configuration to given addresses
	void GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, bool bypass);
	void SetChipAdd(int new_chip_add); // sets chip address
	int GetChipAdd(); // sets chip address
	void SendReadErrors(); // sends a global pulse to read error counters
	void ReadEPROMvalues(); // Sends global pulse to read the values from EPROM to GR.
	void BurnEPROMvalues(); // Burns to the EPROM whatever is stored in GR. Note that burning a 1 is non reversibel!
	bool ReadGADC(int GADCselect); // Configures chip to read value of "voltage_select" and sends global pulse to load GADC to GR40. True if ok, false if fail, esp. if called on FE-I4A!
	void SetAndWriteCOLPRReg(int colpr_mode, int colpr_addr);

	//void GetConfValue(int col, int row, int step, int &Value);	//writes histogram-value for col, row, step to &Value (needs calibration mode)
	//void GetTOTHistoValue(int col, int row, int tot, int &Value);
	void SendBitstream(unsigned char * bitstream, int bitsize); // sends bitsream to FE
	std::string getGlobalVarName(int Variable);

	void sendBitstreamToAB();
	void sendBitstreamToC();
	//ScanChain
	void SetScanChainValue	(int BlockSelect,int index, int val);
	void GetScanChainValue	(int BlockSelect,int index, int& size, int& add, int& Value);
	void GetScanChainValueRB(int BlockSelect,int index, int& size, int& add, int& Value);
	void RunScanChain(int ScanChainSelect, USBPixSTDDCS* dcs, double& current_before, double& current_after, bool shift_only, bool se_while_pulse, bool si_while_pulse);
	void LoadFileToScanChain(int ScanChainSelect);
	void SaveFileFromScanChain(int ScanChainSelect);
	void SaveFileFromScanChainRB(int ScanChainSelect);
	//void ReadScanChainFromFE(int ScanChainSelect);

// *************** scan functionality, currently only software scans supported ***********

	//const ConfDataArray& getGlobalReg(){return *globalReg;};
	//void StartmuCScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSiz, int InjCount, int MaskStepSiz, int MaskStepCount, int ShiftMask);
	//bool StartScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSiz, int InjCount, int MaskStepSiz, int MaskStepCount, int ShiftMask, bool all_DCs);
	//void GetScanStatus(int& MaskStep, bool& ScanBusy, bool& ScanReady, bool& ScanCanceled, bool& EOEerror);
	//void ResetStatusFlags();
	//void CancelScan();


	int IndexToRegisterNumber(int the_index);   // needed to hide FE-I4 / FE-I3 structure differences to outside world
  bool dummyRegisterSelected();


private:
	void HandleFECommand(int major_command, int minor_command, int chip_add, unsigned char *data_array);
	void writeFEMemory(unsigned char *data_array, short int size, bool isGlobal); 
	void readFEMemory(unsigned char *data_array, short int size, bool isGlobal);
	int GetFEDataRB(bool global);
	int GetFEDataRB(ConfDataArray * theReg);
	void IntArrayToDataList();
	void setLatch(int latch, int doublecolumn);
	bool setRegisterNumber(int RegisterNumber);	// needed to hide FE-I4 / FE-I3 structure differences to outside world


	//int IndexToConfRegIndex(int the_index);		// needed to hide FE-I4 / FE-I3 structure differences to outside world
	int IndexToShiftRegIndex(int the_index);	// needed to hide FE-I4 / FE-I3 structure differences to outside world
	int PixIndexToDCindex(int pixindex);		// needed to hide FE-I4 / FE-I3 structure differences to outside world
	int PixIndexToDCNumber(int pixindex);		// needed to hide FE-I4 / FE-I3 structure differences to outside world
	int LatchToPXSTROBE(int latch);				// needed to hide FE-I4 / FE-I3 structure differences to outside world
	bool useSlowControl;
	void LoadPixelLatch(int latch, int DC);
	void LatchBackToSR(int latch, int DC);

private:

	ConfigRegister * configReg;

	ConfDataArray * pixelHITBUS[40];
	ConfDataArray * pixelTDAC0[40];
	ConfDataArray * pixelTDAC1[40];
	ConfDataArray * pixelTDAC2[40];
	ConfDataArray * pixelTDAC3[40];
	ConfDataArray * pixelTDAC4[40];
	ConfDataArray * pixelFDAC0[40];
	ConfDataArray * pixelFDAC1[40];
	ConfDataArray * pixelFDAC2[40];
	ConfDataArray * pixelFDAC3[40];
	ConfDataArray * pixelENABLE[40];
	ConfDataArray * pixelCAP0[40];
	ConfDataArray * pixelCAP1[40];
	ConfDataArray * pixelDIGINJ[40];

	ConfDataArray * pixelHITBUSRB[40];
	ConfDataArray * pixelTDAC0RB[40];
	ConfDataArray * pixelTDAC1RB[40];
	ConfDataArray * pixelTDAC2RB[40];
	ConfDataArray * pixelTDAC3RB[40];
	ConfDataArray * pixelTDAC4RB[40];
	ConfDataArray * pixelFDAC0RB[40];
	ConfDataArray * pixelFDAC1RB[40];
	ConfDataArray * pixelFDAC2RB[40];
	ConfDataArray * pixelFDAC3RB[40];
	ConfDataArray * pixelENABLERB[40];
	ConfDataArray * pixelCAP0RB[40];
	ConfDataArray * pixelCAP1RB[40];

	ConfDataArray * theLatch;		// pointer to Latch of pixel register
	ConfDataArray * theLatchRB;		// pointer to Latch of pixel register RB
	ConfDataArray * theGlobalReg;   // pointer to global register number
	ConfDataArray * the_globalShiftReg;  // pointer to global shift reg

	ConfDataArray * trigger;
	ConfDataArray * fast_command;
	ConfDataArray * slow_command;
	
	//ConfDataArray * globalReg[32];
	ConfDataArray * globalReg_0;
	ConfDataArray * globalReg_1;
	ConfDataArray * globalReg_2;
	ConfDataArray * globalReg_3;
	ConfDataArray * globalReg_4;
	ConfDataArray * globalReg_5;
	ConfDataArray * globalReg_6;
	ConfDataArray * globalReg_7;
	ConfDataArray * globalReg_8;
	ConfDataArray * globalReg_9;
	ConfDataArray * globalReg_10;
	ConfDataArray * globalReg_11;
	ConfDataArray * globalReg_12;
	ConfDataArray * globalReg_13;
	ConfDataArray * globalReg_14;
	ConfDataArray * globalReg_15;
	ConfDataArray * globalReg_16;
	ConfDataArray * globalReg_17;
	ConfDataArray * globalReg_18;
	ConfDataArray * globalReg_19;
	ConfDataArray * globalReg_20;
	ConfDataArray * globalReg_21;
	ConfDataArray * globalReg_22;
	ConfDataArray * globalReg_23;
	ConfDataArray * globalReg_24;
	ConfDataArray * globalReg_25;
	ConfDataArray * globalReg_26;
	ConfDataArray * globalReg_27;
	ConfDataArray * globalReg_28;
	ConfDataArray * globalReg_29;
	ConfDataArray * globalReg_30;
	ConfDataArray * globalReg_31;
	ConfDataArray * globalReg_32;
	ConfDataArray * globalReg_33;
	ConfDataArray * globalReg_34;
	ConfDataArray * globalReg_35;
	ConfDataArray * globalReg_40;
	ConfDataArray * globalReg_41;
	ConfDataArray * globalReg_42;

	// bypass structures for FE-I4A
	ConfDataArray * shift_globalConf_AB;
	ConfDataArray * shift_globalConf_C;

	SiUSBDevice * myUSB;
	int chip_addr;
	int mod_addr;
  int index;
	bool ChipInRunMode;
	bool FEI4B;
  int registerNumber;

	// ScanChain
	ConfDataArray * scanChainCMD;
	ConfDataArray * scanChainECL;
	ConfDataArray * scanChainDOB;

	unsigned char DebugArray[33];
	unsigned int * SRAMdataRB/*[SRAM_WORDSIZE]*/;

};



#endif
