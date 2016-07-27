#ifndef CONFIGUREDATAARRAY_H
#define CONFIGUREDATAARRAY_H

#include "DataStruct.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_LINE_SIZE 512

#include "dllexport.h"

class DllExport ConfDataArray {

public:
	ConfDataArray(ConfDataStruct *ds, int i_size, int b_size, std::string filename, bool global, bool decimal_display);	//Konstruktor
	virtual ~ConfDataArray();	

public:
	//ConfDataStruct *the_list;
	//ConfDataStruct *the_listRB;

	ConfDataStruct *dataList;
	ConfDataStruct *dataListRB;
	
	/*ConfDataStruct *dataListHitbus;
	ConfDataStruct *dataListHitbusRB;
	ConfDataStruct *dataListSelect;
	ConfDataStruct *dataListSelectRB;
	ConfDataStruct *dataListMask_B;
	ConfDataStruct *dataListMask_BRB;
	ConfDataStruct *dataListTDAC0;
	ConfDataStruct *dataListTDAC0RB;
	ConfDataStruct *dataListTDAC1;
	ConfDataStruct *dataListTDAC1RB;
	ConfDataStruct *dataListTDAC2;
	ConfDataStruct *dataListTDAC2RB;
	ConfDataStruct *dataListTDAC3;
	ConfDataStruct *dataListTDAC3RB;
	ConfDataStruct *dataListTDAC4;
	ConfDataStruct *dataListTDAC4RB;
	ConfDataStruct *dataListTDAC5;
	ConfDataStruct *dataListTDAC5RB;
	ConfDataStruct *dataListTDAC6;
	ConfDataStruct *dataListTDAC6RB;
	ConfDataStruct *dataListFDAC0;
	ConfDataStruct *dataListFDAC0RB;
	ConfDataStruct *dataListFDAC1;
	ConfDataStruct *dataListFDAC1RB;
	ConfDataStruct *dataListFDAC2;
	ConfDataStruct *dataListFDAC2RB;
	ConfDataStruct *dataListKill;
	ConfDataStruct *dataListKillRB;*/

	unsigned char  *byteArray;
	unsigned char  *byteArrayRB;
	unsigned char  *nullArray;
	unsigned char  *oneArray;
	int size;  // # items
	int bit_size;
	int byte_size;
	bool isGlobal;
	bool dec_dis;
	//bool isADC;

public:

	bool FileSave(const char *filename, int reg_nr);
	bool FileSaveRB(const char *filename, int reg_nr);
	bool FileOpen(const char *filename);
	bool FileReload(void);
	void LoadDefault();
	void MakeByteArray();
	//int calculateParity();
	void MakeStructListRB();
	void MakeStructList(); 
  bool UpdateByteArrayItem(int index, int val, unsigned char *dataArray, int regidx = -1);
	void UpdateByteArrayItem(int add, int size, int val, bool inv_bitorder, unsigned char *dataArray/* = NULL*/);
	int GetValue(int index);
	int GetValueRB(int index);
	bool SetValue(int index, int val);
	bool SetValueRB(int index, int val);
	//void SetList(int latch);
	//void SetListRB(int latch);
	std::string GetNameFromIndex(int index);
	int GetIndexFromName(std::string name);
	int GetRegindexFromIndex(int index);
	//bool IndexIsElementOfReg(int index);

private:
	void DataStructCopyA2B(ConfDataStruct * A, ConfDataStruct * B, int i);
	bool StringToDataStruct(std::string buffer);
	std::string DataStructToString(ConfDataStruct ds);
	//int GetIndexFromName(std::string name);
	void UpdateStructListItem(int index);
	void UpdateStructListRBItem(int index);
	bool UpdateDataList(std::string name, int val);
	bool UpdateDataList(int index, int val);
	std::string lastFilename;
	ConfDataStruct * default_ds;


};

#endif
