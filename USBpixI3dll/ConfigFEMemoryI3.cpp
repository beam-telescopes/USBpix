#ifdef CF__LINUX
//#include "win2linux.h"
#include <math.h>
#define __stdcall
#elif defined __VISUALC__
#include "stdafx.h"
#endif 

#include "ConfigFEMemoryI3.h"
#include "DataStructDefinesI3.h"

void __stdcall debug_out(std::string text)
{
#ifdef CF__LINUX
	std::cerr << text << std::endl;
#else
	OutputDebugStringA(text.c_str());
#endif
}

ConfigFEMemoryI3::ConfigFEMemoryI3(int c_add, int m_add, SiUSBDevice * Handle)
{
	chip_addr = c_add;
	mod_addr = m_add;
	myUSB = Handle;

	globalReg = 0;
	globalReg = new ConfDataArrayI3(gloCnfRegItem, GLOBAL_REG_ITEMS, GLOBAL_REG_BYTESIZE, "glo_reg.dat", ISGLOBAL);

	pixelHITBUS = 0;
	pixelHITBUS = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "hitbus.dat", false);
	pixelSELECT = 0;
	pixelSELECT = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "select.dat", false);
	pixelMASK_B = 0;
	pixelMASK_B = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "mask_b.dat", false);
	pixelTDAC0 = 0;
	pixelTDAC0 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac0.dat", false);
	pixelTDAC1 = 0;
	pixelTDAC1 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac1.dat", false);
	pixelTDAC2 = 0;
	pixelTDAC2 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac2.dat", false);
	pixelTDAC3 = 0;
	pixelTDAC3 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac3.dat", false);
	pixelTDAC4 = 0;
	pixelTDAC4 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac4.dat", false);
	pixelTDAC5 = 0;
	pixelTDAC5 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac5.dat", false);
	pixelTDAC6 = 0;
	pixelTDAC6 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac6.dat", false);
	pixelFDAC0 = 0;
	pixelFDAC0 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac0.dat", false);
	pixelFDAC1 = 0;
	pixelFDAC1 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac1.dat", false);
	pixelFDAC2 = 0;
	pixelFDAC2 = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac2.dat", false);
	pixelKILL = 0;
	pixelKILL = new ConfDataArrayI3(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "kill.dat", false);

	theLatch = pixelHITBUS;
}

ConfigFEMemoryI3::~ConfigFEMemoryI3()
{
	delete globalReg;
	delete pixelHITBUS;
	delete pixelSELECT;
	delete pixelMASK_B;
	delete pixelTDAC0;
	delete pixelTDAC1;
	delete pixelTDAC2;
	delete pixelTDAC3;
	delete pixelTDAC4;
	delete pixelTDAC5;
	delete pixelTDAC6;
	delete pixelFDAC0;
	delete pixelFDAC1;
	delete pixelFDAC2;
	delete pixelKILL;
}

void ConfigFEMemoryI3::CopyRBDataToWriteData (int latch)
{
	int Address, Size, loc_value;

	for (int i=0; i<93; i++)
	{
		GetPixelRBVarAddVal (i, Address, Size, loc_value, latch);
		SetPixelVal(i, loc_value, latch);
	}
}

void ConfigFEMemoryI3::CopyRBDataToWriteData()
{
	CopyRBDataToWriteData (HITBUS);
	CopyRBDataToWriteData (SELECT);
	CopyRBDataToWriteData (MASK_B);
	CopyRBDataToWriteData (KILL);
	CopyRBDataToWriteData (FDAC0);
	CopyRBDataToWriteData (FDAC1);
	CopyRBDataToWriteData (FDAC2);
	CopyRBDataToWriteData (TDAC0);
	CopyRBDataToWriteData (TDAC1);
	CopyRBDataToWriteData (TDAC2);
	CopyRBDataToWriteData (TDAC3);
	CopyRBDataToWriteData (TDAC4);
	CopyRBDataToWriteData (TDAC5);
	CopyRBDataToWriteData (TDAC6);
}

void ConfigFEMemoryI3::SetChipAdd(int new_chip_add)
{
	chip_addr = new_chip_add;
}

void ConfigFEMemoryI3::writeFEMemory(unsigned char * data_array, int size, bool isGlobal)
{
	unsigned char read_data;
	unsigned char cmd_dcnt[3];
	int array_size;

	array_size = (COMMAND_REG_SIZE + size)/8;

	if (((COMMAND_REG_SIZE + size) % 8) != 0)
		array_size++;

	cmd_dcnt[0] = (unsigned char)(0xff & size);
	cmd_dcnt[1] = (unsigned char)(0xff & (size >> 8));

	if (isGlobal)
	{
		cmd_dcnt[2] = 0x01; //start config_sm with global data

		myUSB->WriteExternal(CS_CONFIG_GLOBAL_WRITEMEM, data_array, array_size); // write configuration-data to BlockRAM
		myUSB->WriteXilinx(CS_CONFIG_FSM_DCNT_0, &cmd_dcnt[0], 1);
		myUSB->WriteXilinx(CS_CONFIG_FSM_DCNT_1, &cmd_dcnt[1], 1);
		myUSB->WriteXilinx(CS_START_CONFIG_FSM, &cmd_dcnt[2], 1);

		//myUSB->WriteExternal(CS_CONFIG_FSM_DCNT_0, cmd_dcnt, 3); // configure and enable + start config_sm
	}
	else
	{
		// debugging
		//for (int i = 0; i < PIXEL_REG_BYTESIZE; i++)
		//{
		//	std::stringstream ss;
		//	ss << "WriteFEMemory -> index: " << i << " value: " << std::hex << (int)data_array[i] << "\n";
		//	debug_out(ss.str());
		//}

		cmd_dcnt[2] = 0x02; //start config_sm with pixel data

		myUSB->WriteExternal(CS_CONFIG_PIXEL_WRITEMEM, data_array, array_size); // write configuration-data to BlockRAM
		myUSB->WriteXilinx(CS_CONFIG_FSM_DCNT_0, &cmd_dcnt[0], 1);
		myUSB->WriteXilinx(CS_CONFIG_FSM_DCNT_1, &cmd_dcnt[1], 1);
		myUSB->WriteXilinx(CS_START_CONFIG_FSM, &cmd_dcnt[2], 1);

		//myUSB->WriteExternal(CS_CONFIG_FSM_DCNT_0, cmd_dcnt, 3); // configure and enable + start config_sm
	}

	// waiting for finishing of configuration-FSM
	do
	{
		myUSB->ReadXilinx(CS_START_CONFIG_FSM, &read_data, 1);
	}
	while ((read_data != 0) && (myUSB->HandlePresent()));
}

void ConfigFEMemoryI3::readFEMemory(unsigned char * data_array, int size, bool isGlobal)
{
	int array_size;
	double temp;

	temp = (double)((COMMAND_REG_SIZE + size)/8.0);

	array_size = (int)ceil(temp);
	if (isGlobal) // read global data
	{
		myUSB->ReadExternal(CS_CONFIG_GLOBAL_READMEM, data_array, array_size); // read read memory
		//myUSB->ReadExternal(CS_CONFIG_GLOBAL_WRITEMEM, data_array, array_size); // debugging/testing write memory
	}
	else // read pixel data
	{
		myUSB->ReadExternal(CS_CONFIG_PIXEL_READMEM, data_array, array_size); // read read memory
		//myUSB->ReadExternal(CS_CONFIG_PIXEL_WRITEMEM, data_array, array_size); // debugging/testing write memory
	}
}

void ConfigFEMemoryI3::HandleFECommand(int major_command, int minor_command, int chip_add, unsigned char * data_array)
{
	switch(major_command)
	{
	case 0: // FE_CMD_NULL
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_REF_RESET: // FE_REF_RESET
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_SOFT_RESET: // FE_SOFT_RESET
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_CLOCK_GLOBAL: // FE_CLOCK_GLOBAL
		globalReg->UpdateByteArrayItem(globalReg->dataList[COMMAND].add, globalReg->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		globalReg->UpdateByteArrayItem(globalReg->dataList[CHIPADDRESS].add, globalReg->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, minor_command, true);
		break;
	case FE_WRITE_GLOBAL: // FE_WRITE_GLOBAL
		globalReg->UpdateByteArrayItem(globalReg->dataList[COMMAND].add, globalReg->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		globalReg->UpdateByteArrayItem(globalReg->dataList[CHIPADDRESS].add, globalReg->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, true);
		break;
	case FE_READ_GLOBAL: // FE_READ_GLOBAL
		globalReg->UpdateByteArrayItem(globalReg->dataList[COMMAND].add, globalReg->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		globalReg->UpdateByteArrayItem(globalReg->dataList[CHIPADDRESS].add, globalReg->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, true);
		break;
	case FE_CLOCK_PIXEL: // also used for shifting the mask
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, minor_command, false);
		break;
	case FE_WRITE_HITBUS:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_SELECT:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_MASK_B:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC0:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC1:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array);  // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC2:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC3:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC4:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC5:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_TDAC6:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_FDAC0:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_FDAC1:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_FDAC2:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_WRITE_KILL:
		theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
		theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
		writeFEMemory(data_array, DCNT_COMMAND, false);
		break;
	case FE_READ_PIXEL: // shift/readback
		switch (minor_command)
		{
		case 0: // FE_CMD_NULL, only ReadPixel
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(major_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_COMMAND, false);
			break;
		case FE_RB_HITBUS: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_SELECT: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_MASK_B: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC0: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC1: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC2: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC3: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC4: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC5: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_TDAC6: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_FDAC0: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_FDAC1: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_FDAC2: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		case FE_RB_KILL: // latches SEU-latch back into pixel shift register
			theLatch->UpdateByteArrayItem(theLatch->dataList[COMMAND].add, theLatch->dataList[COMMAND].size, CalculateCommandParity(minor_command), data_array); // set command
			theLatch->UpdateByteArrayItem(theLatch->dataList[CHIPADDRESS].add, theLatch->dataList[CHIPADDRESS].size, chip_add, data_array); // set chip address
			writeFEMemory(data_array, DCNT_READPIXEL, false);
			break;
		}
	}
}

void ConfigFEMemoryI3::SetUSBHandle(SiUSBDevice * Handle)
{
	myUSB = Handle;
}

void ConfigFEMemoryI3::SendRefReset()
{
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_REF_RESET, DCNT_COMMAND, chip_addr, theLatch->nullArray);
}

void ConfigFEMemoryI3::SendSoftReset()
{
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_SOFT_RESET, DCNT_COMMAND, chip_addr, theLatch->nullArray);
}

void ConfigFEMemoryI3::WriteGlobal()
{
	globalReg->MakeByteArray(); // build array from Struct
	globalReg->UpdateByteArrayItem(globalReg->dataList[GLOBALPARITY].add, globalReg->dataList[GLOBALPARITY].size, globalReg->CalculateGlobalParity(), globalReg->byteArray); // update array with correct parity

	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->byteArray); // clock global and shift in global data
	HandleFECommand(FE_WRITE_GLOBAL, DCNT_COMMAND, chip_addr, globalReg->byteArray); // write to global register
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->nullArray); // fill shift register with zeros
}

void ConfigFEMemoryI3::ReadGlobal()
{
	globalReg->FileSave("write_array.dat");
	globalReg->MakeByteArray();	// build array from Struct
	globalReg->UpdateByteArrayItem(gloCnfRegItem[SELECTDO].add, gloCnfRegItem[SELECTDO].size, GLOBAL_OUT, globalReg->byteArray); // set Select DO to GlobalOut
	globalReg->UpdateByteArrayItem(globalReg->dataList[GLOBALPARITY].add, globalReg->dataList[GLOBALPARITY].size, globalReg->CalculateGlobalParity(), globalReg->byteArray); // update array with correct parity

	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->byteArray); // clock global and shift in global data
	HandleFECommand(FE_WRITE_GLOBAL, DCNT_COMMAND, chip_addr, globalReg->byteArray); // write to global register
	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->nullArray); // fill shift register with zeros
	HandleFECommand(FE_READ_GLOBAL, DCNT_COMMAND, chip_addr, globalReg->nullArray); // latch back global registers
	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->nullArray); // enable global clock and shift-out data

	readFEMemory(globalReg->byteArrayRB, DCNT_GLOBAL, true);
	globalReg->MakeStructListRB();
	globalReg->FileSaveRB("read_array.dat");

	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray); // disable command
}

void ConfigFEMemoryI3::SetGlobalVal(int index, int value)
{
	globalReg->SetValue(index, value);
}

bool ConfigFEMemoryI3::ReadGlobalFile(const char * globalfilename)
{
	if (globalReg->FileOpen(globalfilename))
		return true;
	else 
		return false;
}

void ConfigFEMemoryI3::SaveGlobal(const char * newfilename)
{
	globalReg->FileSave(newfilename);
}

void ConfigFEMemoryI3::SaveGlobalRB(const char * newfilename)
{
	globalReg->FileSaveRB(newfilename);
}

void ConfigFEMemoryI3::GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value)
{
	if ((Variable >= 0) && (Variable < GLOBAL_REG_ITEMS))
	{
		Address = globalReg->dataList[Variable].add;
		Size = globalReg->dataList[Variable].size;
		Value = globalReg->dataList[Variable].value;
	}
	else
	{
		Address = -1;
		Size = -1;
		Value = -1;
	}
}

void ConfigFEMemoryI3::GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value)
{
	if ((Variable >= 0) && (Variable < GLOBAL_REG_ITEMS))
	{
		Address = globalReg->dataListRB[Variable].add;
		Size = globalReg->dataListRB[Variable].size;
		Value = globalReg->dataListRB[Variable].value;
	}
	else
	{
		Address = -1;
		Size = -1;
		Value = -1;
	}
}

void ConfigFEMemoryI3::LoadGlobalDefault()
{
	globalReg->LoadDefault();
}

void ConfigFEMemoryI3::setLatch(int latch)
{
	switch (latch)
	{
	case HITBUS:
		theLatch = pixelHITBUS;
		break;
	case SELECT:
		theLatch = pixelSELECT;
		break;
	case MASK_B:
		theLatch = pixelMASK_B;
		break;
	case TDAC0:
		theLatch = pixelTDAC0;
		break;
	case TDAC1:
		theLatch = pixelTDAC1;
		break;
	case TDAC2:
		theLatch = pixelTDAC2;
		break;
	case TDAC3:
		theLatch = pixelTDAC3;
		break;
	case TDAC4:
		theLatch = pixelTDAC4;
		break;
	case TDAC5:
		theLatch = pixelTDAC5;
		break;
	case TDAC6:
		theLatch = pixelTDAC6;
		break;
	case FDAC0:
		theLatch = pixelFDAC0;
		break;
	case FDAC1:
		theLatch = pixelFDAC1;
		break;
	case FDAC2:
		theLatch = pixelFDAC2;
		break;
	case KILL:
		theLatch = pixelKILL;
		break;
	}// end switch (latch)
}

/*
* Set the register value for the pixel with number pixnum and the latch latch in the configuration array
* Supports setting TDAC and FDAC with integer values
* The settings are not written to the chip!
*/
void ConfigFEMemoryI3::WritePixelByPixnum(int pixnum, int Value, int latch) // int readback
{
	// higer level functionality for setting TDAC and FDAC with integer values
	if (latch == PIX_TDAC)
	{
		int tdac;
		tdac = ((Value & 1)>0);
		WritePixelByPixnum(pixnum, tdac, TDAC0);

		tdac = ((Value & (1 << 1))>0);
		WritePixelByPixnum(pixnum, tdac, TDAC1);

		tdac = ((Value & (1 << 2))>0);
		WritePixelByPixnum(pixnum, tdac, TDAC2);

		tdac = ((Value & (1 << 3))>0);
		WritePixelByPixnum(pixnum, tdac, TDAC3);

		tdac = ((Value & (1 << 4))>0);
		WritePixelByPixnum(pixnum, tdac, TDAC4);

		tdac = ((Value & (1 << 5))>0);
		WritePixelByPixnum(pixnum, tdac, TDAC5);

		tdac = ((Value & (1 << 6))>0);
		WritePixelByPixnum(pixnum, tdac, TDAC6);
		return;
	}
	else if (latch == PIX_FDAC)
	{
		int fdac;
		fdac = ((Value & 1)>0);
		WritePixelByPixnum(pixnum, fdac, FDAC0);

		fdac = ((Value & (1 << 1))>0);
		WritePixelByPixnum(pixnum, fdac, FDAC1);

		fdac = ((Value & (1 << 2))>0);
		WritePixelByPixnum(pixnum, fdac, FDAC2);
		return;
	}

	// just for safety
	if (Value!=0 && Value!=1) return;

	int loc_value; // local storage of returned 32bit value
	int Address, Size;

	// calculate array index in ConfDataStruct_ and bit position in integer for pixel pixnum
	int array_index = ((2879 - pixnum) / 32) + PIXEL2880; // index of element in ConfDataStruct_ containing the requested data
	int int_position = pixnum % 32; // bit-position in the selected int containing the requested bit (pixnum - 1)

	GetPixelVarAddVal (array_index, Address, Size, loc_value, latch);

	// create bit mask
	int bit_mask = 1 << int_position;

	// put the configuration integer together
	int int_value = (Value << int_position) | (loc_value & ~ bit_mask );

	// store data
	SetPixelVal(array_index, int_value, latch);
}

void ConfigFEMemoryI3::WritePixel(int command, int latch)
{
	setLatch(latch);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(command, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);	

	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray); // fill shift register with zeros
}

void ConfigFEMemoryI3::WritePixel()
{
	setLatch(HITBUS);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_HITBUS, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(SELECT);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_SELECT, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(MASK_B);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_MASK_B, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC0);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC0, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC1);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC1, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC2);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC2, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC3);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC3, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC4);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC4, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC5);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC5, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(TDAC6);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_TDAC6, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(FDAC0);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_FDAC0, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(FDAC1);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_FDAC1, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(FDAC2);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_FDAC2, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	setLatch(KILL);
	theLatch->MakeByteArray();
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_WRITE_KILL, DCNT_COMMAND, chip_addr, theLatch->byteArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray); // fill shift register with zeros
}

void ConfigFEMemoryI3::SetPixelVal(int index, int value, int latch)
{
	setLatch(latch);
	theLatch->SetValue(index, value);
}

void ConfigFEMemoryI3::ReadPixelFile(const char * pixelfilename, int latch)
{
	setLatch(latch);
	theLatch->FileOpen(pixelfilename);
}

void ConfigFEMemoryI3::SavePixel(const char * newfilename, int latch)
{
	setLatch(latch);
	theLatch->FileSave(newfilename);
}

void ConfigFEMemoryI3::SavePixelRB(const char * newfilename, int latch)
{
	setLatch(latch);
	theLatch->FileSaveRB(newfilename);
}

void ConfigFEMemoryI3::ReadPixel(int command, int latch)
{
	setLatch(latch);

	theLatch->MakeByteArray();
	globalReg->MakeByteArray();

	globalReg->UpdateByteArrayItem(gloCnfRegItem[SELECTDO].add, gloCnfRegItem[SELECTDO].size, PIXEL_OUT, globalReg->byteArray); // set command
	globalReg->UpdateByteArrayItem(globalReg->dataList[GLOBALPARITY].add, globalReg->dataList[GLOBALPARITY].size, globalReg->CalculateGlobalParity(), globalReg->byteArray); //update array with correct parity

	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->byteArray); // clock global and shift in global data
	HandleFECommand(FE_WRITE_GLOBAL, DCNT_COMMAND, chip_addr, globalReg->byteArray); // write to global register

	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, command, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);

	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);

	setLatch(latch);
	theLatch->MakeStructListRB();
}

void ConfigFEMemoryI3::ReadPixel()
{
	theLatch->MakeByteArray();
	globalReg->MakeByteArray();

	globalReg->UpdateByteArrayItem(gloCnfRegItem[SELECTDO].add, gloCnfRegItem[SELECTDO].size, PIXEL_OUT, globalReg->byteArray); // set command

	HandleFECommand(FE_CLOCK_GLOBAL, DCNT_GLOBAL, chip_addr, globalReg->byteArray); // clock global and shift in global data
	HandleFECommand(FE_WRITE_GLOBAL, DCNT_COMMAND, chip_addr, globalReg->byteArray); // write to global register

	//** Read HITBUS
	setLatch(HITBUS);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_HITBUS, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(HITBUS);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/HitbusRB.dat");

	//** Read SELECT
	setLatch(SELECT);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_SELECT, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(SELECT);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/SelectRB.dat");

	//** Read MASK_B
	setLatch(MASK_B);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_MASK_B, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(MASK_B);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/HitbusRB.dat");

	//** Read TDAC0
	setLatch(TDAC0);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC0, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC0);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac0RB.dat");

	//** Read TDAC1
	setLatch(TDAC1);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC1, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC1);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac1RB.dat");

	//** Read TDAC2
	setLatch(TDAC2);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC2, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC2);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac2RB.dat");

	//** Read TDAC3
	setLatch(TDAC3);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC3, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC3);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac3RB.dat");

	//** Read TDAC4
	setLatch(TDAC4);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC4, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC4);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac4RB.dat");

	//** Read TDAC5
	setLatch(TDAC5);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC5, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC5);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac5RB.dat");

	//** Read TDAC6
	setLatch(TDAC6);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_TDAC6, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(TDAC6);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Tdac6RB.dat");

	//** Read FDAC0
	setLatch(FDAC0);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_FDAC0, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(FDAC0);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Fdac0RB.dat");

	//** Read FDAC1
	setLatch(FDAC1);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_FDAC1, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(FDAC1);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Fdac1RB.dat");

	//** Read FDAC2
	setLatch(FDAC2);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_FDAC2, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(FDAC2);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/Fdac2RB.dat");

	//** Read KILL
	setLatch(KILL);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_RB_KILL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, FE_CMD_NULL, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_CLOCK_PIXEL, DCNT_PIXEL, chip_addr, theLatch->nullArray);
	readFEMemory(theLatch->byteArrayRB, DCNT_PIXEL, false);
	setLatch(KILL);
	theLatch->MakeStructListRB();
	theLatch->FileSaveRB("./pixelreg/KillRB.dat");
}

void ConfigFEMemoryI3::GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch)
{
	if ((Variable >= 0) && (Variable < PIXEL_REG_ITEMS) && (latch >= 0) && (latch < PIXEL_REG_LATCHES))
	{
		setLatch(latch);
		Address = theLatch->dataList[Variable].add;
		Size = theLatch->dataList[Variable].size;
		Value = theLatch->dataList[Variable].value;
	}
	else
	{
		Address = -1;
		Size = -1;
		Value = -1;
	}
}

void ConfigFEMemoryI3::GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch)
{
	if ((Variable >= 0) && (Variable < PIXEL_REG_ITEMS) && (latch >= 0) && (latch < PIXEL_REG_LATCHES))
	{
		setLatch(latch);
		Address = theLatch->dataListRB[Variable].add;
		Size = theLatch->dataListRB[Variable].size;
		Value = theLatch->dataListRB[Variable].value;
	}
	else
	{
		Address = -1;
		Size = -1;
		Value = -1;
	}
}

/*
* Returns the read back register value as integer for the pixel with number pixnum and the latch latch
* Supports readout of TDAC and FDAC values as integer
*/
void ConfigFEMemoryI3::GetPixelRBValueByPixnum (int pixnum, int& Value, int latch)
{
	GetPixelValueByPixnum (pixnum, Value, latch, 1);
}

/*
* Returns the register value as integer for the pixel with number pixnum and the latch latch
* Supports readout of TDAC and FDAC values as integer
* If the optional parameter readback is set to 1, the read back values are returned
*/
void ConfigFEMemoryI3::GetPixelValueByPixnum(int pixnum, int & Value, int latch, int readback)
{
	int loc_value; // local storage of returned 32bit value
	int Address, Size;

	// higer level functionality for returning TDAC and FDAC values as integer
	if (latch == PIX_TDAC)
	{
		int tdac;

		GetPixelValueByPixnum(pixnum, loc_value, TDAC0, readback);
		tdac = loc_value;
		GetPixelValueByPixnum(pixnum, loc_value, TDAC1, readback);
		tdac = tdac + (loc_value<<1);
		GetPixelValueByPixnum(pixnum, loc_value, TDAC2, readback);
		tdac = tdac + (loc_value<<2);
		GetPixelValueByPixnum(pixnum, loc_value, TDAC3, readback);
		tdac = tdac + (loc_value<<3);
		GetPixelValueByPixnum(pixnum, loc_value, TDAC4, readback);
		tdac = tdac + (loc_value<<4);
		GetPixelValueByPixnum(pixnum, loc_value, TDAC5, readback);
		tdac = tdac + (loc_value<<5);
		GetPixelValueByPixnum(pixnum, loc_value, TDAC6, readback);
		tdac = tdac + (loc_value<<6);

		Value = tdac;
		return;
	}
	else if (latch == PIX_FDAC)
	{
		int fdac;

		GetPixelValueByPixnum(pixnum, loc_value, FDAC0, readback);
		fdac = loc_value;
		GetPixelValueByPixnum(pixnum, loc_value, FDAC1, readback);
		fdac = fdac + (loc_value<<1);
		GetPixelValueByPixnum(pixnum, loc_value, FDAC2, readback);
		fdac = fdac + (loc_value<<2);

		Value = fdac;
		return;
	}

	// calculate array index in ConfDataStruct_ and bit position in integer for pixel pixnum
	int array_index = ((2879 - pixnum) / 32) + PIXEL2880; // index of element in ConfDataStruct_ containing the requested data
	int int_position = pixnum % 32; // bit-position in the selected int containing the requested bit // (pixnum - 1) % 32

	if (readback==1)
	{
		// get the requested values from the read back data
		GetPixelRBVarAddVal (array_index, Address, Size, loc_value, latch);
	}
	else
	{
		// get the requested data from the local array
		GetPixelVarAddVal (array_index, Address, Size, loc_value, latch);
	}

	// create bit mask for readout
	int bit_mask = 1 << int_position;

	// get the requested bit from the integer value
	if ((loc_value & bit_mask) != 0)
	{
		Value = 1;
	}
	else
	{
		Value = 0;
	}
}

void ConfigFEMemoryI3::ShiftPixMask(int latch, int stepsize)
{
	int write_command;
	int read_command;

	if (stepsize == 0)
		return;

	if ((latch != HITBUS) && (latch != SELECT) && (latch != MASK_B) && (latch != KILL))
		return;

	switch (latch)
	{
	case HITBUS:
		write_command = FE_WRITE_HITBUS;
		read_command = FE_RB_HITBUS;
		break;
	case SELECT:
		write_command = FE_WRITE_SELECT;
		read_command = FE_RB_SELECT;
		break;
	case MASK_B:
		write_command = FE_WRITE_MASK_B;
		read_command = FE_RB_MASK_B;
		break;
	case KILL:
		write_command = FE_WRITE_KILL;
		read_command = FE_RB_KILL;
		break;
	}

	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	// latching SEU-latch back into pixel shift register
	HandleFECommand(FE_READ_PIXEL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
	HandleFECommand(FE_READ_PIXEL, read_command, chip_addr, theLatch->nullArray);
	//HandleFECommand(FE_READ_PIXEL, read_command, chip_addr, theLatch->nullArray); // issue two ReadPixel + ClockPixel + load strobe; see Kevin Einsweiler's FE-I3 documentation

	// shift mask for N=stepsize steps
	HandleFECommand(FE_CLOCK_PIXEL, stepsize, chip_addr, theLatch->nullArray); 

	// latch pixel shift register into SEU-latch
	HandleFECommand(write_command, DCNT_COMMAND, chip_addr, theLatch->nullArray);

	HandleFECommand(FE_CMD_NULL, DCNT_COMMAND, chip_addr, theLatch->nullArray);
}

int ConfigFEMemoryI3::CalculateCommandParity(int Command)
{
	int Parity=0;

	for (int i = 1; i < COMMAND_REG_SIZE_WO_ADD; i++) // do not include parity bit
		Parity = (Parity ^ (0x01 & (Command >> i)));

	Command = Command & (~0x01); // clear old parity bit

	return Command = Command | Parity; // set new parity bit
}

void ConfigFEMemoryI3::StartuCScan(int ScanVarIndex, int ScanVarStart, int ScanVarStop, int ScanVarStepSize, int Injections, int ShiftMaskStepSize, int ShiftMaskSteps, int ShiftMaskType)
{
	if ((ScanVarIndex >= 0) && (ScanVarIndex < GLOBAL_REG_ITEMS) && (ScanVarStart >= 0) && (ScanVarStop >= 0) && (ScanVarStepSize >= 0) && (ScanVarStart <= ScanVarStop) && ((ScanVarStop - ScanVarStart) >= ScanVarStepSize) && (((ScanVarStop - ScanVarStart)%ScanVarStepSize) == 0) && (Injections >= 0) && (Injections < 256) && (ShiftMaskStepSize >= 0) && (ShiftMaskSteps >= 0) && (ShiftMaskType >= 0) && (ShiftMaskType < 16))
	{
		unsigned char ScanConf[15];
		int ScanVarAdd = 0;
		int ScanVarSize = 0;
		int DummyValue = 0;
		GetGlobalVarAddVal(ScanVarIndex, ScanVarAdd, ScanVarSize, DummyValue);

		ScanConf[0] = CMD_START_SCAN; // starts scan if == 1
		ScanConf[1] = (unsigned char)ScanVarAdd;
		ScanConf[2] = (unsigned char)ScanVarSize;
		ScanConf[3] = (unsigned char)(ScanVarStart);
		ScanConf[4] = (unsigned char)(ScanVarStart >> 8);
		ScanConf[5] = (unsigned char)(ScanVarStop);
		ScanConf[6] = (unsigned char)(ScanVarStop >> 8);
		ScanConf[7] = (unsigned char)(ScanVarStepSize);
		ScanConf[8] = (unsigned char)(ScanVarStepSize >> 8);
		ScanConf[9] = (unsigned char)Injections;
		ScanConf[10] = (unsigned char)ShiftMaskStepSize;
		ScanConf[11] = (unsigned char)(ShiftMaskStepSize >> 8);
		ScanConf[12] = (unsigned char)ShiftMaskSteps;
		ScanConf[13] = (unsigned char)(ShiftMaskSteps >> 8);
		ScanConf[14] = (unsigned char)ShiftMaskType;

		// solve predefined parity
		SetGlobalVal(ScanVarIndex, 0);
		globalReg->MakeByteArray();
		globalReg->UpdateByteArrayItem(globalReg->dataList[GLOBALPARITY].add, globalReg->dataList[GLOBALPARITY].size, globalReg->CalculateGlobalParity(), globalReg->byteArray); //update array with correct parity
		myUSB->WriteExternal(CS_CONFIG_GLOBAL_WRITEMEM, globalReg->byteArray, GLOBAL_REG_BYTESIZE); // write configuration-data to BlockRAM

		// update uCScan parameters and start uCScan
		myUSB->WriteCommand(ScanConf, 15);
	}
}

void ConfigFEMemoryI3::StopuCScan()
{
  unsigned char Data[4] = {0,0,0,0};
	Data[0] = SCAN_CANCELED;
	myUSB->WriteRegister(Data);
}

// legacy scan
void ConfigFEMemoryI3::StartScan(int ScanVarIndex, int ScanVarStart, int ScanVarStop, int ScanVarStepSize, int Injections, int ShiftMaskStepSize, int ShiftMaskSteps, int ShiftMaskType)
{
	unsigned char regdata;

	regdata = (unsigned char)Injections;
	myUSB->WriteXilinx(CS_QUANTITY_STRB, & regdata, 1);

	for (int maskstep = 0; maskstep < ShiftMaskSteps; maskstep++)
	{
		unsigned char confstep = 0; // start from 0
		for (int ScanVal = ScanVarStart; ScanVal <= ScanVarStop; ScanVal = ScanVal + ScanVarStepSize)
		{
			myUSB->WriteXilinx(CS_CONFIGURATION_STEP, & confstep, 1);
			SetGlobalVal(ScanVarIndex, ScanVal);
			WriteGlobal();

			regdata = 0x01;
			myUSB->WriteXilinx(CS_MEASUREMENT_START_STOP, & regdata, 1); // start injections

			do
			{
				myUSB->ReadXilinx(CS_MEASUREMENT_START_STOP, & regdata, 1);
			}
			while (regdata != 0);

			confstep++;
		}
		if((ShiftMaskType & 0x01) == 0x01)
			ShiftPixMask(HITBUS, ShiftMaskStepSize);
		if((ShiftMaskType & 0x02) == 0x02)
			ShiftPixMask(SELECT, ShiftMaskStepSize);
		if((ShiftMaskType & 0x04) == 0x04)
			ShiftPixMask(MASK_B, ShiftMaskStepSize);
		if((ShiftMaskType & 0x08) == 0x08)
			ShiftPixMask(KILL, ShiftMaskStepSize);
	}
}
