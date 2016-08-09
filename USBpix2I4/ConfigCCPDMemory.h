/*
 * ConfigCCPDMemory is a interface class between the USBpix-board and the GUIs
 * It contains all functionality to store/write CCPD - configuration
 * CCPD - configuration is stored in instances of ConfDataArray
 */

#ifndef CONFIGCCPDMEMORY_H
#define CONFIGCCPDMEMORY_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dllexport.h>

class ConfDataArray;
class ConfigRegister;

class DllExport ConfigCCPDMemory
{

//Variables for values from config
int InDac[60][24]; //V2: 60 col, 24 row
int EnR[12];  //V2: 12 row
int EnL[12];  //V2: 12 row
int Enable[12]; //V2: 12 row

int L0[20];//V2: 20 col
int L1[20];//V2: 20 col
int L2[20];//V2: 20 col
int R0[20];//V2: 20 col
int R1[20];//V2: 20 col
int R2[20];//V2: 20 col
int StripRO;
int Wr_Ld_ID[20];//V2: 20 col
int directcurrent;
int simplepixel;
int ampout[20];//V2: 20 col
bool isone;

//LF variables
int aPixel[2736];
int sw_ana[24];




public:
	ConfigCCPDMemory(ConfigRegister * cR);
	~ConfigCCPDMemory();

public:

	void SetCcpdGlobalVal(int the_index, int the_value); // sets one item in global CCPD configuration
	void SetCcpdColCtrlVal(int the_index, int the_value, int Wr_ID, int col); // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
	void SetCcpdRowCtrlVal(int the_index, int the_value, int Wr_ID, int row, int col); // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
	void SetCcpdv2ColCtrlVal(int the_index, int the_value, int Wr_ID, int col); // sets one item in pixel CCPD V2 configuration. row or col == -1 means broadcast to all
	void SetCcpdv2RowCtrlVal(int the_index, int the_value, int Wr_ID, int row, int col); // sets one item in pixel CCPD V2 configuration. row or col == -1 means broadcast to all
	bool GetCcpdGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value); // writes value, bitsize and address of one item of global configuration to given addresses
	bool GetCcpdGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value); // writes value, bitsize and address of one item of read-back global configuration to given addresses
	void WriteCcpdGlobal();
	void WriteCcpdPixel(int col);
	void WriteCompleteCcpdShiftRegister();
	void SetCcpdThr(int the_value); // sets the Thr value that is generated using the on pcb DACs
	void SetCcpdVcal(int the_value); // sets the Vcal value that is generated using the on pcb DACs
	void WriteCcpdOnPcbDacs(); // writes the Thr and Vcal value in the DACs and enables the DACs.
	void SetCCPDVersion(int version); // Get the CCPD version and sets if for USBpix
	
	void SetCcpdInDacValue(int col, int row, int value); //V2: sets array with InDac values
	void ResetCcpdMonitor(); //V2: resets Monitor
	void SetCcpdMonitorValue(int col, int row, int value); //V2: sets array with Monitor values
	void SetCcpdEnableValue(int row, int value); //V2: sets array with Enable values
	
	void SetCcpdStripROValue(int value); //V2: sets StripRO values
	void SetCcpdWrLdIDValue(int col, int value); //V2: sets array with WrLdID values
	void SetCcpddirectcurrentValue(int value); //V2: sets directcurrent values
	void SetCcpdampoutValue(int col, int value); //V2: sets array with ampout values
	void SetCcpdsimplepixelValue(int value); //V2: enables simple pixel
	
	void SetCcpdLFPixels(int pixel, int value);
	void SetCcpdLFSw_Ana(int row, int value);
	

private:
	
	int ReshuffleCcpdGlobalVal(int the_value); // reshuffles the bitorder to match Ccpd DAC bitorder
	int ReshuffleCcpdGlobalValRB(int the_value); // reshuffles the bitorder to match Ccpd DAC bitorder
	void SendCcpdBitstream(int bitsize, unsigned char* byteArray, bool isGlobal);
	bool GetBitStatusFromDataArray(int BitNumber, unsigned char* dataArray);
	void WriteCcpdCol(int Wr_ID, int col);
	void PrepareSendCcpdBitstream(bool isGlobal);
	void LoadCcpdBitstream(bool isGlobal);
	
	void SetCcpdV2pixelRow(int row, int InDacL, int InDacR); //Sets CCPD_pixelRow
	void SetCcpdV2pixelCol(int col, int iWrLdID); //Sets CCPD_pixelRow
	void SetCcpdLFpixel(); //Sets LF pixel register

private:

	ConfigRegister * configReg;

	ConfDataArray * CCPD_globalReg;
	ConfDataArray * CCPD_pixelReg;
	ConfDataArray * CCPD_pixelCol;
	ConfDataArray * CCPD_pixelRow;
	ConfDataArray * CCPD_rowItem[3][12][20];
	ConfDataArray * CCPD_colItem[3][20];

	ConfDataArray * CCPD_OnPcbDacReg;

 public:
	enum CCPDVersion {HV2FEI4_V1, HV2FEI4_V2, HV2FEI4_LF} ccpdversion;

};



#endif
