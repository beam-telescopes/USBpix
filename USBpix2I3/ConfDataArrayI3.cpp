#ifdef CF__LINUX
//#include "win2linux.h"
//#include "SURConstants.h"
#include "SiLibUSB.h"
#include <stdio.h>
#include <string.h>
#define __stdcall
#elif defined __VISUALC__
#include "stdafx.h"
#include <tchar.h>
#endif 

#include "ConfDataArrayI3.h"
#include "definesI3.h"

using namespace std;

void __stdcall debug_out1(std::string text)
{
#ifdef CF__LINUX
	std::cerr << text << std::endl;
#else
	OutputDebugStringA(text.c_str());
#endif
}

ConfDataArrayI3::ConfDataArrayI3(ConfDataStructI3 *ds, int i_size, int b_size, std::string /*filename*/, bool global)
{
	default_ds = ds;
	isGlobal = global;
	size = i_size;
	byte_size = b_size;
	byteArray = new unsigned char[byte_size];
	byteArrayRB = new unsigned char[byte_size];
	nullArray = new unsigned char[byte_size];
	oneArray = new unsigned char[byte_size];
	dataList = new ConfDataStructI3[i_size];
	dataListRB = new ConfDataStructI3[i_size];

#ifdef CF__LINUX
	memset(nullArray, 0,byte_size);
	memset(oneArray, 0xff, byte_size);
#else
	ZeroMemory(nullArray, byte_size);
	FillMemory(oneArray, byte_size, 0xff);
#endif

	LoadDefault();
}

ConfDataArrayI3::~ConfDataArrayI3()
{
	//FileSave();
	delete[] byteArray;
	delete[] byteArrayRB;
	delete[] nullArray;
	delete[] oneArray;
	delete[] dataList;
	delete[] dataListRB;
}

//---begin functions------------------
void ConfDataArrayI3::DataStructCopyA2B(ConfDataStructI3 * A, ConfDataStructI3 * B, int i)
{
#ifdef CF__LINUX
	strcpy(B[i].name, A[i].name);
#else
	strcpy_s(B[i].name, A[i].name);
#endif
	B[i].index = A[i].index;
	B[i].add = A[i].add;
	B[i].size = A[i].size;
	B[i].value = A[i].value;
}

int ConfDataArrayI3::GetValue(int index) // get value of index no.
{
	if ((index >= 0) && (index < size))
		return dataList[index].value;
	else
		return -1;
}

bool ConfDataArrayI3::SetValue(int index, int value) // set value of index no.
{
	if ((index >= 0) && (index < size))
	{
		dataList[index].value = value;
		return true;
	}
	return false;
}

void ConfDataArrayI3::UpdateByteArrayItem(int add, int size, int val, unsigned char *dataArray)
{
	int i;
	int i_byte;
	int i_add;
	int i_bit;
	int mask;

	if (dataArray == NULL)
		dataArray = byteArray;

	i_add = add;

	for (i = 0; i < size; i++)
	{
		i_byte = i_add / 8; // byte number
		i_bit  = i_add % 8; // bit address

		mask = 1 << i_bit; // normal seqence,

		if (isGlobal)
		{
			//    if ((0x01 & (val >> i)) == 0) // normal order
			if ((0x01 & (val >> ((size - 1) - i))) == 0) // inverted order
				dataArray[i_byte] &= ~mask;
			else
				dataArray[i_byte] |=  mask;
		}
		else
		{
			//    if ((0x01 & (val >> i)) == 0) // normal order
			if ((0x01 & (val >> ((size - 1) - i))) == 0) // inverted order
				dataArray[i_byte] &= ~mask;
			else
				dataArray[i_byte] |=  mask;
		}

		i_add++;
	}
}

void ConfDataArrayI3::UpdateStructListRBItem(int index)
{
	int i;
	int add;
	int size;
	int val;
	int i_byte;
	int i_add;
	int i_bit;
	int mask;

	add  = dataListRB[index].add;
	size = dataListRB[index].size;
	val = 0;

	i_add = add;

	for (i = 0; i < size; i++)
	{
		i_byte = i_add / 8; // byte number
		i_bit  = (i_add % 8); // bit address

		mask = 1 << i_bit;   

		if ((byteArrayRB[i_byte] & mask) == mask)
		{
			val |= 1 << (size - 1 - i); //reverse order
			//val |= 1 << i; // normal order
		}
		i_add++;
	}
	dataListRB[index].value = val;
}

void ConfDataArrayI3::UpdateStructListItem(int index)
{
	int i;
	int add;
	int size;
	int val;
	int i_byte;
	int i_add;
	int i_bit;
	int mask;

	add  = dataList[index].add;
	size = dataList[index].size;
	val = 0;

	i_add = add;

	for (i = 0; i < size; i++)
	{
		i_byte = i_add / 8; // byte number
		i_bit  = (i_add % 8); // bit address

		mask = 1 << i_bit;   

		if ((byteArray[i_byte] & mask) == mask)
		{
			//			val |= 1 << i; // normal order
			val |= 1 << (size - 1 - i); // reverse order
		}
		i_add++;
	}
	dataList[index].value = val;
}

void ConfDataArrayI3::MakeByteArray()
{
	int i;
	for (i = 0; i < size; i++)
	{
		UpdateByteArrayItem(dataList[i].add, dataList[i].size, dataList[i].value, byteArray);
	} 
}

void ConfDataArrayI3::MakeStructListRB()
{
	int i;
	for (i = 0; i < size; i++)
		UpdateStructListRBItem(i);
}

void ConfDataArrayI3::MakeStructList()
{
	int i;
	for (i = 0; i < size; i++)
		UpdateStructListItem(i);
}

bool ConfDataArrayI3::FileSave(const char *filename)
{
	ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif

	fout.open(filename);

	if (!fout.good())
		return false;

	fout << "#" << endl;
	fout << "# configuration list format:" << endl;
	fout << "# <name> \t <value> " << endl;
	fout << "#" << endl;
	fout << "# last change: " << std::string(timebuf) << endl;
	fout << endl;

	for (int i = 0; i < size; i++)
		fout<< DataStructToString(dataList[i])<< endl;

	fout.close();

	return true;
}

bool ConfDataArrayI3::FileSaveRB(const char *filename)
{
	ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif

	fout.open(filename);

	if (!fout.good())
		return false;

	fout << "#" << endl;
	fout << "# configuration list format:" << endl;
	fout << "# <name> \t <value> " << endl;
	fout << "#" << endl;
	fout << "# last change: " << std::string(timebuf) << endl;
	fout << endl;

	for (int i = 0; i < size; i++)
		fout<< DataStructToString(dataListRB[i])<< endl;

	//fout<< DataStructToString(dataListRB[size - 1]);

	fout.close();
	return true;
}

bool ConfDataArrayI3::FileOpen(const char *filename)
{
	ifstream fin;
	char line[MAX_LINE_SIZE];
	int pos;

	string sbuf;

    if (fin.good())
		return false;

	lastFilename = filename;
	fin.open(filename);
	if (!fin)
		return false;

	while (fin.getline(line, MAX_LINE_SIZE))
	{
		sbuf = line;
		pos = sbuf.find('#',0); // comment delimiter
		sbuf = sbuf.substr(0, pos);
		if (sbuf.size() > 3) // TODO: add sanity check
			StringToDataStruct(sbuf);
	}
	MakeByteArray();

	fin.close();
	return true;
}

bool ConfDataArrayI3::FileReload(void)
{
	return FileOpen(lastFilename.c_str());
}

void ConfDataArrayI3::LoadDefault()
{
	for (int i = 0; i < size; i++)
	{
		DataStructCopyA2B(default_ds, dataList, i);
		DataStructCopyA2B(default_ds, dataListRB, i);
	}
}

bool ConfDataArrayI3::StringToDataStruct(string buffer)
{
	string name;
	string value;
	int i_value;

	stringstream sstr;

	sstr << buffer;

	sstr >> name >> value ;

	i_value = strtol(value.c_str(), NULL, 0);

	int index = GetIndexFromName(name);

	if ((index == -1) || (index >= size))
		return false;

	dataList[index].value = i_value;
	return true;
}

string ConfDataArrayI3::DataStructToString(ConfDataStructI3 ds)
{
	string buffer;

	stringstream sstr;

	if (isGlobal)
		sstr << ds.name << '\t' << '\t' << ds.value;

	else 
		sstr << ds.name << '\t' << '\t' << "0x" << std::hex<<ds.value;

	buffer = sstr.str();
	return buffer;
}

int ConfDataArrayI3::GetIndexFromName(string name)
{
	for (int i = 0; i < size; i++)
	{
		if (string (dataList[i].name) == name)
			return i;
	}
#ifdef CF__LINUX
	//cout<<"ERROR: Index not found"<<endl;
#else
	//MessageBox(NULL, _T("Error"), _T("Index not found"), MB_OK|MB_ICONERROR);
#endif
	return -1;
}

bool ConfDataArrayI3::UpdateDataList(string name, int value)
{
	int index = GetIndexFromName(name);

	return UpdateDataList(index, value);
}

bool ConfDataArrayI3::UpdateDataList(int index, int value)
{
	if ((index == -1) || (index >= size))
		return false;

	dataList[index].value = value;
	return true;
}

int ConfDataArrayI3::CalculateGlobalParity()
{
	int Value = 0;
	int Parity = 0;

	for (int i = CHIP_LATENCY; i < GLOBAL_REG_ITEMS; i++)
	{
		Value = dataList[i].value;

		switch (i)
		{
		case HITBUSSCALER: // HITBUSSCALER is a 8-bit read only register, not included in global parity
			break;
		case MONLEAKADC: // MONLEAKADC[13] is a read only register, not included in global parity
			for (int j = 0; j < (dataList[i].size - 1); j++)
				Parity = (Parity ^ (0x01 & (Value >> j)));
			break;
		default:
			for (int j = 0; j < dataList[i].size; j++)
				Parity = (Parity ^ (0x01 & (Value >> j)));
			break;
		}
	}
	return Parity;
}
