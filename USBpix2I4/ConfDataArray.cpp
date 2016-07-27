#ifdef CF__LINUX
//#include "win2linux.h"
//#include "SURConstants.h"
#include "SiUSBLib.h"
#include <stdio.h>
#include <string.h>
#define __stdcall
#elif defined __VISUALC__
#include "stdafx.h"
#include <tchar.h>
#endif 

#include "ConfDataArray.h"
#include "defines.h"

using namespace std;

ConfDataArray::ConfDataArray(ConfDataStruct *ds, int i_size, int b_size, std::string /*filename*/, bool global, bool decimal_display) 
{
	default_ds = ds;
	isGlobal = global;
	dec_dis = decimal_display;

	size = i_size;
	byte_size = b_size;
	byteArray = 0;
	byteArray = new unsigned char[byte_size]();
	byteArrayRB = 0;
	byteArrayRB = new unsigned char[byte_size]();
	nullArray = 0;
	nullArray = new unsigned char[byte_size]();
	oneArray = 0;
	oneArray = new unsigned char[byte_size]();

	dataList = 0;
	dataList = new ConfDataStruct[i_size];
	dataListRB = 0;
	dataListRB = new ConfDataStruct[i_size];

#ifdef CF__LINUX
	memset(nullArray, 0,byte_size);
	memset(oneArray, 0xff, byte_size);
#else
	ZeroMemory(nullArray, byte_size);
	FillMemory(oneArray, byte_size, 0xff);
#endif

	LoadDefault();
}

ConfDataArray::~ConfDataArray()
{
	delete[] byteArray;
	delete[] byteArrayRB;
	delete[] nullArray;
	delete[] oneArray;


	delete[] dataList;
	delete[] dataListRB;
}


//---begin functions------------------
void ConfDataArray::DataStructCopyA2B(ConfDataStruct * A, ConfDataStruct * B, int i)
{
#ifdef CF__LINUX
	strcpy(B[i].name, A[i].name);

#else
	strcpy_s(B[i].name, A[i].name);
#endif
	B[i].regindex = A[i].regindex;
	B[i].index = A[i].index;
	B[i].add = A[i].add;
	B[i].size = A[i].size;
	B[i].value = A[i].value;
	B[i].inv_bitorder = A[i].inv_bitorder;
}

 int ConfDataArray::GetValue(int index)	//get Value of Index Nr. index
{
	if (index < size)
		return dataList[index].value;
	else
		return 0;
}

 int ConfDataArray::GetValueRB(int index)	//get Value of Index Nr. index
{
	if (index < size)
		return dataListRB[index].value;
	else
		return 0;
}

bool ConfDataArray::SetValue(int index, int val) //sets Value of Index Nr. index
{
	if (GetNameFromIndex(index) != "INDEX NOT FOUND")
	{
		dataList[GetRegindexFromIndex(index)].value = val;
		return true;
	}
	return false;
}

bool ConfDataArray::SetValueRB(int index, int val) //sets Value of Index Nr. index
{
	if (GetNameFromIndex(index) != "INDEX NOT FOUND")
	{
		dataListRB[index].value = val;
		return true;
	}
	return false;
}


bool ConfDataArray::UpdateByteArrayItem(int index, int val, unsigned char *dataArray, int regidx)
{
  if (regidx <= 0)
  {
    regidx = GetRegindexFromIndex(index);
  }

  if (regidx <= 0)
  {
    return false;
  }

  int add = dataList[regidx].add;
  int size = dataList[regidx].size;
  bool inv_bitorder = dataList[regidx].inv_bitorder;
  
  UpdateByteArrayItem(add, size, val, inv_bitorder, dataArray);
  return true;
}

void ConfDataArray::UpdateByteArrayItem(int add, int size, int val, bool inv_bitorder, unsigned char *dataArray)
{
	int i;
	int i_byte;
	int i_add;
	unsigned char i_bit;
	unsigned char mask;




	if (dataArray == NULL)
		dataArray = byteArray;

	i_add = add;

	for (i = 0; i < size; i++)
	{
		i_byte = i_add / 8;   // byte number
		i_bit  = i_add % 8;   // bit address

		mask = 1 << i_bit;   // normal seqence,

		if (inv_bitorder)
		{
			if ((0x01 & (val >> i)) == 0)  // inverted order
			//	if ((0x01 & (val >> ((size - 1) - i))) == 0)  // normal order
				dataArray[i_byte] &= ~mask;
			else
				dataArray[i_byte] |=  mask;
		}
		else
		{
			//    if ((0x01 & (val >> i)) == 0)  // inverted order
			if ((0x01 & (val >> ((size - 1) - i))) == 0)  // normal order
				dataArray[i_byte] &= ~mask;
			else
				dataArray[i_byte] |=  mask;
		}

		i_add++;
	}
}

void ConfDataArray::UpdateStructListRBItem(int index)
{
	int i;
	int add;
	int size;
	int val;
	int i_byte;
	int i_add;
	unsigned char i_bit;
	unsigned char mask;
	//bool inv_bitorder;

	add  = dataListRB[index].add;
	size = dataListRB[index].size;
	val = 0;
	//inv_bitorder = dataListRB[index].inv_bitorder;

	i_add = add;

	for (i = 0; i < size; i++)
	{
		i_byte = i_add / 8;   // byte number
		i_bit  = (i_add % 8);   // bit address

		mask = 1 << i_bit;   

		if ((byteArrayRB[i_byte] & mask) == mask)
		{
		//	if (inv_bitorder) // in RB path inversion needs to be done in case of pixel bypass mode. Covered in GetPixelRBVal function.
		//		val |= 1 << i;  //reverse order
		//	else
				val |= 1 << (size - 1 - i); // normal order
		}
		i_add++;
	}
	dataListRB[index].value = val;
}

void ConfDataArray::UpdateStructListItem(int index)
{
	int i;
	int add;
	int size;
	int val;
	int i_byte;
	int i_add;
	unsigned char i_bit;
	unsigned char mask;
	//bool inv_bitorder;

	add  = dataList[index].add;
	size = dataList[index].size;
	val = 0;
	//inv_bitorder = dataList[index].inv_bitorder;

	i_add = add;

	for (i = 0; i < size; i++)
	{
		i_byte = i_add / 8;   // byte number
		i_bit  = (i_add % 8);   // bit address

		mask = 1 << i_bit;   

		if ((byteArray[i_byte] & mask) == mask)
		{
	//		if (inv_bitorder)
	//			val |= 1 << i;  //reverse order
	//		else
				val |= 1 << (size - 1 - i); // normal order
		}
		i_add++;
	}
	dataList[index].value = val;
}

void ConfDataArray::MakeByteArray()
{
	for (int i = 0; i < size/*CONF_REG_NUMBER*/; i++)
	{
		//if (GetNameFromIndex(i) != "INDEX NOT FOUND")
		//{
			UpdateByteArrayItem(dataList[i].add, dataList[i].size, dataList[i].value, dataList[i].inv_bitorder, byteArray);
		//}
	}
}


void ConfDataArray::MakeStructListRB()
{
	for (int i = 0; i < size/*CONF_REG_NUMBER*/; i++)
	{
		//if (GetNameFromIndex(i) != "INDEX NOT FOUND")
			UpdateStructListRBItem(i);
	}
}

void ConfDataArray::MakeStructList()
{
	for (int i = 0; i < size/*CONF_REG_NUMBER*/; i++)
	{
		//if (GetNameFromIndex(i) != "INDEX NOT FOUND")
			UpdateStructListItem(i);
	}
}

bool ConfDataArray::FileSave(const char *filename, int reg_nr)
{
	ofstream fout;

	if (reg_nr == 2)
	{
		time_t ltime;
		char timebuf[26];

		time(&ltime);
#ifndef CF__LINUX
		ctime_s(timebuf, 26, &ltime);
#else
		ctime_r(&ltime, timebuf);
#endif


		fout.open(filename, ios::out);


		if (!fout.is_open())
			return false;


		fout << "#" << endl;
		fout << "# configuration list format:" << endl;
		fout << "# <name> \t <value> " << endl;
		fout << "#" << endl;
		fout << "# last change: " << std::string(timebuf) << endl;
		fout << endl;

	}
	else
	{
		fout.open(filename, ios::app);
		if (!fout.is_open())
			return false;
	}
	if (isGlobal)
	{

	for (int i = 4; i < size; i++)
		fout<< DataStructToString(dataList[i])<< endl;
	}
	else
	{
		for (int i = 0; i < size; i++)
			fout<< DataStructToString(dataList[i])<< endl;
	}



	fout.close();

	return true;
}

bool ConfDataArray::FileSaveRB(const char *filename, int reg_nr)
{
	ofstream fout;

	if (reg_nr == 2)
	{

		time_t ltime;
		char timebuf[26];

		time(&ltime);
#ifndef CF__LINUX
		ctime_s(timebuf, 26, &ltime);
#else
		ctime_r(&ltime, timebuf);
#endif


		fout.open(filename);

		if (!fout.is_open())
			return false;


		fout << "#" << endl;
		fout << "# configuration list format:" << endl;
		fout << "# <name> \t <value> " << endl;
		fout << "#" << endl;
		fout << "# last change: " << std::string(timebuf) << endl;
		fout << endl;

	}
	else
	{
		fout.open(filename, ios::app);
		if (!fout.is_open())
			return false;
	}
	if (isGlobal)
	{
		for (int i = 4; i < size; i++)
			fout<< DataStructToString(dataListRB[i])<< endl;
	}
	else
	{
		for (int i = 0; i < size; i++)
			fout<< DataStructToString(dataListRB[i])<< endl;
	}
	//fout<< DataStructToString(the_listRB[size - 1]);

	fout.close();
	return true;
}

bool ConfDataArray::FileOpen(const char *filename)
{
	ifstream fin;
	char line[MAX_LINE_SIZE];
	int pos;

	string sbuf;

	lastFilename = filename;
	fin.open(filename);
	if (!fin.is_open())
		return false;

	while (fin.getline(line, MAX_LINE_SIZE))
	{
		sbuf = line;
		pos = sbuf.find('#',0);  // comment delimiter
		sbuf = sbuf.substr(0, pos);
		if (sbuf.size() > 3)   // to do :: add sanity check
			StringToDataStruct(sbuf);
	}
	MakeByteArray();

	fin.close();
	return true;
}

bool ConfDataArray::FileReload(void)
{
	return FileOpen(lastFilename.c_str());
}

void ConfDataArray::LoadDefault()
{
	for (int i = 0; i < size/*< CONF_REG_NUMBER*/; i++)
	{
		//if (GetNameFromIndex(i) != "INDEX NOT FOUND")
		//{
			DataStructCopyA2B(default_ds, dataList, i);
			DataStructCopyA2B(default_ds, dataListRB, i);
		//}
	} // for i_size
	//}

	//else if (isADC)
	//{
	//	for (int i = 0; i < size; i++)
	//	{
	//		DataStructCopyA2B(default_ds, dataList, i);
	//	} // for i_size
	//}

	//else
	//{
	//	for (int i = 0; i < size; i++)
	//	{
	//		DataStructCopyA2B(default_ds, dataListHitbus, i);
	//		DataStructCopyA2B(default_ds, dataListHitbusRB, i);
	//		DataStructCopyA2B(default_ds, dataListSelect, i);
	//		DataStructCopyA2B(default_ds, dataListSelectRB, i);
	//		DataStructCopyA2B(default_ds, dataListMask_B, i);
	//		DataStructCopyA2B(default_ds, dataListMask_BRB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC0, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC0RB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC1, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC1RB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC2, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC2RB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC3, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC3RB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC4, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC4RB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC5, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC5RB, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC6, i);
	//		DataStructCopyA2B(default_ds, dataListTDAC6RB, i);
	//		DataStructCopyA2B(default_ds, dataListFDAC0, i);
	//		DataStructCopyA2B(default_ds, dataListFDAC0RB, i);
	//		DataStructCopyA2B(default_ds, dataListFDAC1, i);
	//		DataStructCopyA2B(default_ds, dataListFDAC1RB, i);
	//		DataStructCopyA2B(default_ds, dataListFDAC2, i);
	//		DataStructCopyA2B(default_ds, dataListFDAC2RB, i);
	//		DataStructCopyA2B(default_ds, dataListKill, i);
	//		DataStructCopyA2B(default_ds, dataListKillRB, i);		
	//	}
	//}

}

bool ConfDataArray::StringToDataStruct(string buffer)
{
	string name;
	string value;
	unsigned int i_value;

	stringstream sstr;

	sstr << buffer;

	sstr >> name >> value ;

	i_value = strtoul(value.c_str(), NULL, 0); // atoi

	int index = GetIndexFromName(name);

	if ((index == -1) || (index >= size))
		return false;

	dataList[index].value = i_value;
	return true;
}

string ConfDataArray::DataStructToString(ConfDataStruct ds)
{
	string buffer;

	stringstream sstr;

	if (dec_dis)
		sstr << ds.name << '\t' << '\t' << ds.value;

	else 
		sstr << ds.name << '\t' << '\t' << "0x" << std::hex<<ds.value;

	buffer = sstr.str();
	return buffer;
}

int ConfDataArray::GetIndexFromName(string name)
{
	for (int i = 0; i < size; i++)
	{
		if (string (dataList[i].name) == name)
			return i;
	}
#ifdef CF__LINUX
	cout<<"ERROR: Index not found"<<endl;
#else
	//  MessageBox(NULL, _T("Error"), _T("Index not found"), MB_OK|MB_ICONERROR);
#endif
	return -1;
}

string ConfDataArray::GetNameFromIndex(int index)
{
	for (int i = 0; i < size; i++)
	{
		if (dataList[i].index == index)
			return dataList[i].name;
	}
	
	return "INDEX NOT FOUND";
}

int ConfDataArray::GetRegindexFromIndex(int index)
{
	for (int i = 0; i < size; i++)
	{
		if (dataList[i].index == index)
			return dataList[i].regindex;
	} 

	return -1;
}
//bool ConfDataArray::IndexIsElementOfReg(int index)
//{
//	for (int i = 0; < size; i++)
//	{
//		data
//	}
//}

bool ConfDataArray::UpdateDataList(string name, int value)
{
	int index = GetIndexFromName(name);

	return UpdateDataList(index, value);
}

bool ConfDataArray::UpdateDataList(int index, int value)
{
	if ((index == -1) || (index >= size))
		return false;

	dataList[index].value = value;
	return true;
}

// ************************ solve parity *********************
//int ConfDataArray::calculateParity()
//{
//	int parity = 0;
//	// calkulate parity for all Items of global Register, except HITBUSSCALER
//	for (int i = CHIP_LATENCY; i<CONFIG_REG_ITEMS; i++)
//	{
//		if (i == HITBUSSCALER)
//			;
//		else
//		{
//			int Value = dataList[i].value;
//			if ( i == MONLEAKADC)
//			{
//				for (int j=0; j < dataList[i].size - 1; j++)
//					parity += ((0x1<<j) & Value) >> j;
//			}
//			else
//			{ 
//				for (int j=0; j<dataList[i].size; j++)
//					parity += ((0x1<<j) & Value) >> j;
//			}
//
//			parity = (0x1 & parity);
//		}
//	}
//	return parity;
//}
