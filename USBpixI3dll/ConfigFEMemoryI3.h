#ifndef CONFIGFEMEMORYI3_H
#define CONFIGFEMEMORYI3_H

#include "SiLibUSB.h"
#include "ConfDataArrayI3.h"
#include "ConfigRegisterI3.h"
#include "definesI3.h"
#include "dllexport.h"

class DllExport ConfigFEMemoryI3
{
public:
	ConfigFEMemoryI3(int chip_add, int mod_add, SiUSBDevice * Handle);
	~ConfigFEMemoryI3();

public:
	void CopyRBDataToWriteData();
	void CopyRBDataToWriteData(int latch);
	void SetUSBHandle(SiUSBDevice * hUSB);
	void SendRefReset();
	void SendSoftReset();
	void WriteGlobal();
	void WritePixel();
	void WritePixel(int command, int latch);
	void WritePixelByPixnum (int pixnum, int Value, int latch);
	void ShiftPixMask(int latch, int stepsize);
	void ReadGlobal();
	void ReadPixel();
	void ReadPixel(int command, int latch);
	void SetGlobalVal(int index, int value);
	void SetPixelVal(int index, int value, int latch);
	bool ReadGlobalFile(const char * globalfilename);
	void ReadPixelFile(const char * pixelfilename, int latch);
	void SaveGlobal(const char * newfilename);
	void SaveGlobalRB(const char * newfilename);
	void LoadGlobalDefault();
	void SavePixel(const char * newfilename, int latch);
	void SavePixelRB(const char * newfilename, int latch);
	void GetGlobalVarAddVal(int Variable, int & Address, int & Size, int & Value);
	void GetGlobalRBVarAddVal(int Variable, int & Address, int & Size, int & Value);
	void GetPixelVarAddVal(int Variable, int & Address, int & Size, int & Value, int latch);
	void GetPixelRBVarAddVal(int Variable, int & Address, int & Size, int & Value, int latch);
	void GetPixelRBValueByPixnum(int pixnum, int & Value, int latch);
	void GetPixelValueByPixnum(int pixnum, int & Value, int latch, int readback = 0);
	void SetChipAdd(int new_chip_add);
	const ConfDataArrayI3 & getGlobalReg(){return * globalReg;};
	int CalculateCommandParity(int Command);
	void StartuCScan(int ScanVarIndex, int ScanVarStart, int ScanVarStop, int ScanVarStepSize, int Injections, int ShiftMaskStepSize, int ShiftMaskSteps, int ShiftMaskType);
	void StopuCScan();
	void StartScan(int ScanVarIndex, int ScanVarStart, int ScanVarStop, int ScanVarStepSize, int Injections, int ShiftMaskStepSize, int ShiftMaskSteps, int ShiftMaskType);

private:
	void HandleFECommand(int major_command, int minor_command, int chip_add, unsigned char * data_array);
	void writeFEMemory(unsigned char * data_array, int size, bool isGlobal); 
	void readFEMemory(unsigned char * data_array, int size, bool isGlobal);
	void IntArrayToDataList();
	void setLatch(int latch);

private:
	ConfDataArrayI3 * pixelHITBUS;
	ConfDataArrayI3 * pixelSELECT;
	ConfDataArrayI3 * pixelMASK_B;
	ConfDataArrayI3 * pixelTDAC0;
	ConfDataArrayI3 * pixelTDAC1;
	ConfDataArrayI3 * pixelTDAC2;
	ConfDataArrayI3 * pixelTDAC3;
	ConfDataArrayI3 * pixelTDAC4;
	ConfDataArrayI3 * pixelTDAC5;
	ConfDataArrayI3 * pixelTDAC6;
	ConfDataArrayI3 * pixelFDAC0;
	ConfDataArrayI3 * pixelFDAC1;
	ConfDataArrayI3 * pixelFDAC2;
	ConfDataArrayI3 * pixelKILL;

	ConfDataArrayI3 * globalReg;
	ConfDataArrayI3 * theLatch;

	SiUSBDevice * myUSB;
	int chip_addr;
	int mod_addr;
	unsigned char DebugArray[33];
};

#endif
