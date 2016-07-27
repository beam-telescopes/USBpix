#ifndef CONFIGUREDATAARRAYI3_H
#define CONFIGUREDATAARRAYI3_H

#include "dllexport.h"
#include "DataStructI3.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_LINE_SIZE 512

class DllExport ConfDataArrayI3
{
public:
	ConfDataArrayI3(ConfDataStructI3 *ds, int i_size, int b_size, std::string filename, bool global);
	virtual ~ConfDataArrayI3();

public:
	ConfDataStructI3 *dataList;
	ConfDataStructI3 *dataListRB;
	unsigned char *byteArray;
	unsigned char *byteArrayRB;
	unsigned char *nullArray;
	unsigned char *oneArray;
	int size; // # items
	int bit_size;
	int byte_size;
	bool isGlobal;

public:
	bool FileSave(const char *filename);
	bool FileSaveRB(const char *filename);
	bool FileOpen(const char *filename);
	bool FileReload(void);
	void LoadDefault();
	void MakeByteArray();
	int CalculateGlobalParity();
	void MakeStructListRB();
	void MakeStructList(); 
	void UpdateByteArrayItem(int add, int size, int val, unsigned char *dataArray = NULL);
	int GetValue(int index);
	bool SetValue(int index, int val);

private:
	void DataStructCopyA2B(ConfDataStructI3 * A, ConfDataStructI3 * B, int i);
	bool StringToDataStruct(std::string buffer);
	std::string DataStructToString(ConfDataStructI3 ds);
	int GetIndexFromName(std::string name);
	void UpdateStructListItem(int index);
	void UpdateStructListRBItem(int index);
	bool UpdateDataList(std::string name, int val);
	bool UpdateDataList(int index, int val);
	std::string lastFilename;
	ConfDataStructI3 * default_ds;
};

#endif
