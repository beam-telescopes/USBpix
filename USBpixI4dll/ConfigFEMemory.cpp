
#ifdef CF__LINUX
//#include "win2linux.h"
#include "SiUSBLib.h"
#include <math.h>
#include <unistd.h>
#define __stdcall
#elif defined __VISUALC__
#include "stdafx.h"
#endif 

#include <upsleep.h>

#include "ConfigFEMemory.h"
#include "defines.h"
#include "ConfDataArray.h"
#include "DataStructDefines.h"

// configuration register structure:  {index, name, bit address, size, default value}


const bool fifo_config = false;

void __stdcall debug_out(std::string text)
{
#ifdef CF__LINUX
	std::cerr << text << std::endl;
#else
	OutputDebugStringA(text.c_str());
#endif
}


//---Konstruktor/Destruktor----------

ConfigFEMemory::ConfigFEMemory(int c_add, int m_add, SiUSBDevice * Handle, 
    ConfigRegister * cR, bool isFEI4B, int index)
{
	chip_addr = c_add;
	mod_addr  = m_add;
	myUSB = Handle;
	FEI4B = isFEI4B;
  this->index = index;
	useSlowControl = false;
	ChipInRunMode = false;

	SRAMdataRB = new unsigned int [SRAM_WORDSIZE];

	trigger = new ConfDataArray(triggerCnfRegItem, TRIGGER_REG_ITEMS, TRIGGER_REG_BYTESIZE, "trigger.dat", false, false);
	fast_command = new ConfDataArray(fastCnfRegItem, FAST_REG_ITEMS, FAST_REG_BYTESIZE, "fast_command.dat", false, false);
	slow_command = new ConfDataArray(slowCnfRegItem, SLOW_REG_ITEMS, SLOW_REG_BYTESIZE, "slow_command.dat", false, false);

	if(!FEI4B)
	{
		shift_globalConf_AB = new ConfDataArray(shiftCnfRegItemAB_A, SHIFT_REG_AB_A_ITEMS, SHIFT_REG_AB_BYTESIZE, "shift_glbalConf_AB.dat", false, true);
		shift_globalConf_C = new ConfDataArray(shiftCnfRegItemC_A, SHIFT_REG_C_A_ITEMS, SHIFT_REG_C_BYTESIZE, "shift_glbalConf_C.dat", false, true);

		the_globalShiftReg = shift_globalConf_AB;

		globalReg_0 = 0;
		globalReg_0 = new ConfDataArray(gloCnfRegItem_DUMMY, CONFIG_REG_DUMMY_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_0.dat", true, true);
		globalReg_1 = 0;
		globalReg_1 = new ConfDataArray(gloCnfRegItem_DUMMY, CONFIG_REG_DUMMY_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_1.dat", true, true);
		globalReg_2 = 0;
		globalReg_2 = new ConfDataArray(gloCnfRegItem_FEI4A_2, CONFIG_REG_FEI4A_2_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_2.dat", true, true);
		globalReg_3 = 0;
		globalReg_3 = new ConfDataArray(gloCnfRegItem_FEI4A_3, CONFIG_REG_FEI4A_3_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_3.dat", true, true);
		globalReg_4 = 0;
		globalReg_4 = new ConfDataArray(gloCnfRegItem_FEI4A_4, CONFIG_REG_FEI4A_4_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_4.dat", true, true);
		globalReg_5 = 0;
		globalReg_5 = new ConfDataArray(gloCnfRegItem_FEI4A_5, CONFIG_REG_FEI4A_5_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_5.dat", true, true);
		globalReg_6 = 0;
		globalReg_6 = new ConfDataArray(gloCnfRegItem_FEI4A_6, CONFIG_REG_FEI4A_6_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_6.dat", true, true);
		globalReg_7 = 0;
		globalReg_7 = new ConfDataArray(gloCnfRegItem_FEI4A_7, CONFIG_REG_FEI4A_7_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_7.dat", true, true);
		globalReg_8 = 0;
		globalReg_8 = new ConfDataArray(gloCnfRegItem_FEI4A_8, CONFIG_REG_FEI4A_8_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_8.dat", true, true);
		globalReg_9 = 0;
		globalReg_9 = new ConfDataArray(gloCnfRegItem_FEI4A_9, CONFIG_REG_FEI4A_9_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_9.dat", true, true);
		globalReg_10 = 0;
		globalReg_10 = new ConfDataArray(gloCnfRegItem_FEI4A_10, CONFIG_REG_FEI4A_10_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_10.dat", true, true);
		globalReg_11 = 0;
		globalReg_11 = new ConfDataArray(gloCnfRegItem_FEI4A_11, CONFIG_REG_FEI4A_11_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_11.dat", true, true);
		globalReg_12 = 0;
		globalReg_12 = new ConfDataArray(gloCnfRegItem_FEI4A_12, CONFIG_REG_FEI4A_12_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_12.dat", true, true);
		globalReg_13 = 0;
		globalReg_13 = new ConfDataArray(gloCnfRegItem_FEI4A_13, CONFIG_REG_FEI4A_13_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_13.dat", true, true);
		globalReg_14 = 0;
		globalReg_14 = new ConfDataArray(gloCnfRegItem_FEI4A_14, CONFIG_REG_FEI4A_14_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_14.dat", true, true);
		globalReg_15 = 0;
		globalReg_15 = new ConfDataArray(gloCnfRegItem_FEI4A_15, CONFIG_REG_FEI4A_15_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_15.dat", true, true);
		globalReg_16 = 0;
		globalReg_16 = new ConfDataArray(gloCnfRegItem_FEI4A_16, CONFIG_REG_FEI4A_16_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_16.dat", true, true);
		globalReg_17 = 0;
		globalReg_17 = new ConfDataArray(gloCnfRegItem_FEI4A_17, CONFIG_REG_FEI4A_17_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_17.dat", true, true);
		globalReg_18 = 0;
		globalReg_18 = new ConfDataArray(gloCnfRegItem_FEI4A_18, CONFIG_REG_FEI4A_18_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_18.dat", true, true);
		globalReg_19 = 0;
		globalReg_19 = new ConfDataArray(gloCnfRegItem_FEI4A_19, CONFIG_REG_FEI4A_19_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_19.dat", true, true);
		globalReg_20 = 0;
		globalReg_20 = new ConfDataArray(gloCnfRegItem_FEI4A_20, CONFIG_REG_FEI4A_20_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_20.dat", true, true);
		globalReg_21 = 0;
		globalReg_21 = new ConfDataArray(gloCnfRegItem_FEI4A_21, CONFIG_REG_FEI4A_21_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_21.dat", true, true);
		globalReg_22 = 0;
		globalReg_22 = new ConfDataArray(gloCnfRegItem_FEI4A_22, CONFIG_REG_FEI4A_22_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_22.dat", true, true);
		globalReg_23 = 0;
		globalReg_23 = new ConfDataArray(gloCnfRegItem_FEI4A_23, CONFIG_REG_FEI4A_23_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_23.dat", true, true);
		globalReg_24 = 0;
		globalReg_24 = new ConfDataArray(gloCnfRegItem_FEI4A_24, CONFIG_REG_FEI4A_24_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_24.dat", true, true);
		globalReg_25 = 0;
		globalReg_25 = new ConfDataArray(gloCnfRegItem_FEI4A_25, CONFIG_REG_FEI4A_25_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_25.dat", true, true);
		globalReg_26 = 0;
		globalReg_26 = new ConfDataArray(gloCnfRegItem_FEI4A_26, CONFIG_REG_FEI4A_26_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_26.dat", true, true);
		globalReg_27 = 0;
		globalReg_27 = new ConfDataArray(gloCnfRegItem_FEI4A_27, CONFIG_REG_FEI4A_27_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_27.dat", true, true);
		globalReg_28 = 0;
		globalReg_28 = new ConfDataArray(gloCnfRegItem_FEI4A_28, CONFIG_REG_FEI4A_28_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_28.dat", true, true);
		globalReg_29 = 0;
		globalReg_29 = new ConfDataArray(gloCnfRegItem_FEI4A_29, CONFIG_REG_FEI4A_29_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_29.dat", true, true);
		globalReg_30 = 0;
		globalReg_30 = new ConfDataArray(gloCnfRegItem_DUMMY, CONFIG_REG_DUMMY_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_29.dat", true, true);
		globalReg_31 = 0;
		globalReg_31 = new ConfDataArray(gloCnfRegItem_FEI4A_31, CONFIG_REG_FEI4A_31_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_31.dat", true, true);

		//efuse register
		globalReg_32 = 0;
		globalReg_32 = new ConfDataArray(gloCnfRegItem_FEI4A_32, CONFIG_REG_FEI4A_32_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_32.dat", true, true);
		globalReg_33 = 0;
		globalReg_33 = new ConfDataArray(gloCnfRegItem_FEI4A_33, CONFIG_REG_FEI4A_33_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_33.dat", true, true);
		globalReg_34 = 0;
		globalReg_34 = new ConfDataArray(gloCnfRegItem_FEI4A_34, CONFIG_REG_FEI4A_34_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_34.dat", true, true);
		globalReg_35 = 0;
		globalReg_35 = new ConfDataArray(gloCnfRegItem_FEI4A_35, CONFIG_REG_FEI4A_35_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_35.dat", true, true);

		//read-only register
		globalReg_40 = 0;
		globalReg_40 = new ConfDataArray(gloCnfRegItem_FEI4A_40, CONFIG_REG_FEI4A_40_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_40.dat", true, true);
		globalReg_41 = 0;
		globalReg_41 = new ConfDataArray(gloCnfRegItem_FEI4A_41, CONFIG_REG_FEI4A_41_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_41.dat", true, true);
		globalReg_42 = 0;
		globalReg_42 = new ConfDataArray(gloCnfRegItem_FEI4A_42, CONFIG_REG_FEI4A_42_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_42.dat", true, true);
	}
	else
	{
		shift_globalConf_AB = new ConfDataArray(shiftCnfRegItemAB_B, SHIFT_REG_AB_B_ITEMS, SHIFT_REG_AB_BYTESIZE, "shift_glbalConf_AB.dat", false, true);
		shift_globalConf_C = new ConfDataArray(shiftCnfRegItemC_A, SHIFT_REG_C_B_ITEMS, SHIFT_REG_C_BYTESIZE, "shift_glbalConf_C.dat", false, true);

		the_globalShiftReg = shift_globalConf_AB;

		globalReg_0 = 0;
		globalReg_0 = new ConfDataArray(gloCnfRegItem_DUMMY, CONFIG_REG_DUMMY_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_0.dat", true, true);
		globalReg_1 = 0;
		globalReg_1 = new ConfDataArray(gloCnfRegItem_FEI4B_1, CONFIG_REG_FEI4B_1_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_1.dat", true, true);
		globalReg_2 = 0;
		globalReg_2 = new ConfDataArray(gloCnfRegItem_FEI4B_2, CONFIG_REG_FEI4B_2_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_2.dat", true, true);
		globalReg_3 = 0;
		globalReg_3 = new ConfDataArray(gloCnfRegItem_FEI4B_3, CONFIG_REG_FEI4B_3_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_3.dat", true, true);
		globalReg_4 = 0;
		globalReg_4 = new ConfDataArray(gloCnfRegItem_FEI4B_4, CONFIG_REG_FEI4B_4_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_4.dat", true, true);
		globalReg_5 = 0;
		globalReg_5 = new ConfDataArray(gloCnfRegItem_FEI4B_5, CONFIG_REG_FEI4B_5_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_5.dat", true, true);
		globalReg_6 = 0;
		globalReg_6 = new ConfDataArray(gloCnfRegItem_FEI4B_6, CONFIG_REG_FEI4B_6_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_6.dat", true, true);
		globalReg_7 = 0;
		globalReg_7 = new ConfDataArray(gloCnfRegItem_FEI4B_7, CONFIG_REG_FEI4B_7_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_7.dat", true, true);
		globalReg_8 = 0;
		globalReg_8 = new ConfDataArray(gloCnfRegItem_FEI4B_8, CONFIG_REG_FEI4B_8_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_8.dat", true, true);
		globalReg_9 = 0;
		globalReg_9 = new ConfDataArray(gloCnfRegItem_FEI4B_9, CONFIG_REG_FEI4B_9_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_9.dat", true, true);
		globalReg_10 = 0;
		globalReg_10 = new ConfDataArray(gloCnfRegItem_FEI4B_10, CONFIG_REG_FEI4B_10_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_10.dat", true, true);
		globalReg_11 = 0;
		globalReg_11 = new ConfDataArray(gloCnfRegItem_FEI4B_11, CONFIG_REG_FEI4B_11_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_11.dat", true, true);
		globalReg_12 = 0;
		globalReg_12 = new ConfDataArray(gloCnfRegItem_FEI4B_12, CONFIG_REG_FEI4B_12_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_12.dat", true, true);
		globalReg_13 = 0;
		globalReg_13 = new ConfDataArray(gloCnfRegItem_FEI4B_13, CONFIG_REG_FEI4B_13_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_13.dat", true, true);
		globalReg_14 = 0;
		globalReg_14 = new ConfDataArray(gloCnfRegItem_FEI4B_14, CONFIG_REG_FEI4B_14_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_14.dat", true, true);
		globalReg_15 = 0;
		globalReg_15 = new ConfDataArray(gloCnfRegItem_FEI4B_15, CONFIG_REG_FEI4B_15_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_15.dat", true, true);
		globalReg_16 = 0;
		globalReg_16 = new ConfDataArray(gloCnfRegItem_FEI4B_16, CONFIG_REG_FEI4B_16_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_16.dat", true, true);
		globalReg_17 = 0;
		globalReg_17 = new ConfDataArray(gloCnfRegItem_FEI4B_17, CONFIG_REG_FEI4B_17_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_17.dat", true, true);
		globalReg_18 = 0;
		globalReg_18 = new ConfDataArray(gloCnfRegItem_FEI4B_18, CONFIG_REG_FEI4B_18_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_18.dat", true, true);
		globalReg_19 = 0;
		globalReg_19 = new ConfDataArray(gloCnfRegItem_FEI4B_19, CONFIG_REG_FEI4B_19_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_19.dat", true, true);
		globalReg_20 = 0;
		globalReg_20 = new ConfDataArray(gloCnfRegItem_FEI4B_20, CONFIG_REG_FEI4B_20_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_20.dat", true, true);
		globalReg_21 = 0;
		globalReg_21 = new ConfDataArray(gloCnfRegItem_FEI4B_21, CONFIG_REG_FEI4B_21_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_21.dat", true, true);
		globalReg_22 = 0;
		globalReg_22 = new ConfDataArray(gloCnfRegItem_FEI4B_22, CONFIG_REG_FEI4B_22_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_22.dat", true, true);
		globalReg_23 = 0;
		globalReg_23 = new ConfDataArray(gloCnfRegItem_FEI4B_23, CONFIG_REG_FEI4B_23_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_23.dat", true, true);
		globalReg_24 = 0;
		globalReg_24 = new ConfDataArray(gloCnfRegItem_FEI4B_24, CONFIG_REG_FEI4B_24_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_24.dat", true, true);
		globalReg_25 = 0;
		globalReg_25 = new ConfDataArray(gloCnfRegItem_FEI4B_25, CONFIG_REG_FEI4B_25_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_25.dat", true, true);
		globalReg_26 = 0;
		globalReg_26 = new ConfDataArray(gloCnfRegItem_FEI4B_26, CONFIG_REG_FEI4B_26_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_26.dat", true, true);
		globalReg_27 = 0;
		globalReg_27 = new ConfDataArray(gloCnfRegItem_FEI4B_27, CONFIG_REG_FEI4B_27_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_27.dat", true, true);
		globalReg_28 = 0;
		globalReg_28 = new ConfDataArray(gloCnfRegItem_FEI4B_28, CONFIG_REG_FEI4B_28_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_28.dat", true, true);
		globalReg_29 = 0;
		globalReg_29 = new ConfDataArray(gloCnfRegItem_FEI4B_29, CONFIG_REG_FEI4B_29_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_29.dat", true, true);
		globalReg_30 = 0;
		globalReg_30 = new ConfDataArray(gloCnfRegItem_FEI4B_30, CONFIG_REG_FEI4B_30_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_30.dat", true, true);
		globalReg_31 = 0;
		globalReg_31 = new ConfDataArray(gloCnfRegItem_FEI4B_31, CONFIG_REG_FEI4B_31_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_31.dat", true, true);

		//efuse register
		globalReg_32 = 0;
		globalReg_32 = new ConfDataArray(gloCnfRegItem_FEI4B_32, CONFIG_REG_FEI4B_32_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_32.dat", true, true);
		globalReg_33 = 0;
		globalReg_33 = new ConfDataArray(gloCnfRegItem_FEI4B_33, CONFIG_REG_FEI4B_33_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_33.dat", true, true);
		globalReg_34 = 0;
		globalReg_34 = new ConfDataArray(gloCnfRegItem_FEI4B_34, CONFIG_REG_FEI4B_34_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_34.dat", true, true);
		globalReg_35 = 0;
		globalReg_35 = new ConfDataArray(gloCnfRegItem_FEI4B_35, CONFIG_REG_FEI4B_35_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_35.dat", true, true);

		//read-only register
		globalReg_40 = 0;
		globalReg_40 = new ConfDataArray(gloCnfRegItem_FEI4B_40, CONFIG_REG_FEI4B_40_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_40.dat", true, true);
		globalReg_41 = 0;
		globalReg_41 = new ConfDataArray(gloCnfRegItem_FEI4B_41, CONFIG_REG_FEI4B_41_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_41.dat", true, true);
		globalReg_42 = 0;
		globalReg_42 = new ConfDataArray(gloCnfRegItem_FEI4B_42, CONFIG_REG_FEI4B_42_ITEMS, CONFIG_REG_BYTESIZE, "glo_reg_42.dat", true, true);
	}

  setRegisterNumber(2);

	for (int i = 0; i < 40; i++)
	{
		pixelHITBUS[i] = 0;
		pixelHITBUS[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "hitbus.dat", false, false);
		pixelTDAC0[i] = 0;
		pixelTDAC0[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac0.dat", false, false);
		pixelTDAC1[i] = 0;
		pixelTDAC1[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac1.dat", false, false);
		pixelTDAC2[i] = 0;
		pixelTDAC2[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac2.dat", false, false);
		pixelTDAC3[i] = 0;
		pixelTDAC3[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac3.dat", false, false);
		pixelTDAC4[i] = 0;
		pixelTDAC4[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "tdac4.dat", false, false);
		pixelFDAC0[i] = 0;
		pixelFDAC0[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac0.dat", false, false);
		pixelFDAC1[i] = 0;
		pixelFDAC1[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac1.dat", false, false);		
		pixelFDAC2[i] = 0;
		pixelFDAC2[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac2.dat", false, false);
		pixelFDAC3[i] = 0;
		pixelFDAC3[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "fdac3.dat", false, false);
		pixelENABLE[i] = 0;
		pixelENABLE[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "enable.dat", false, false);
		pixelCAP0[i] = 0;
		pixelCAP0[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "cap0.dat", false, false);
		pixelCAP1[i] = 0;
		pixelCAP1[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "cap1.dat", false, false);
		pixelDIGINJ[i] = 0;
		pixelDIGINJ[i] = new ConfDataArray(pixCnfRegItem, PIXEL_REG_ITEMS, PIXEL_REG_BYTESIZE, "diginj.dat", false, false);
	}

		for (int i = 0; i < 40; i++)
	{
		pixelHITBUSRB[i] = 0;
		pixelHITBUSRB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "hitbusRB.dat", false, false);
		pixelTDAC0RB[i] = 0;
		pixelTDAC0RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "tdac0RB.dat", false, false);
		pixelTDAC1RB[i] = 0;
		pixelTDAC1RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "tdac1RB.dat", false, false);
		pixelTDAC2RB[i] = 0;
		pixelTDAC2RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "tdac2RB.dat", false, false);
		pixelTDAC3RB[i] = 0;
		pixelTDAC3RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "tdac3RB.dat", false, false);
		pixelTDAC4RB[i] = 0;
		pixelTDAC4RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "tdac4RB.dat", false, false);
		pixelFDAC0RB[i] = 0;
		pixelFDAC0RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "fdac0RB.dat", false, false);
		pixelFDAC1RB[i] = 0;
		pixelFDAC1RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "fdac1RB.dat", false, false);		
		pixelFDAC2RB[i] = 0;
		pixelFDAC2RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "fdac2RB.dat", false, false);
		pixelFDAC3RB[i] = 0;
		pixelFDAC3RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "fdac3RB.dat", false, false);
		pixelENABLERB[i] = 0;
		pixelENABLERB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "enableRB.dat", false, false);
		pixelCAP0RB[i] = 0;
		pixelCAP0RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "cap0RB.dat", false, false);
		pixelCAP1RB[i] = 0;
		pixelCAP1RB[i] = new ConfDataArray(pixCnfRegRBItem, PIXEL_REG_RB_ITEMS, PIXEL_REG_RB_BYTESIZE, "cap1RB.dat", false, false);
	}

	theLatch = pixelHITBUS[0];
	theLatchRB = pixelHITBUSRB[0];
  
	configReg = 0;
	configReg = cR;//new ConfigRegister(myUSB);

	// ScaChain
	scanChainCMD = 0;				//FIXME
	scanChainCMD = new ConfDataArray(scanChainCMDds, SC_CMD_ITEMS, SC_CMD_BYTESIZE, "ScanChainCMD.dat", false, true);

	scanChainECL = 0;				//FIXME
	if(!isFEI4B)
		scanChainECL = new ConfDataArray(scanChainECL_FEI4A_ds, SC_ECL_A_ITEMS, SC_ECL_A_BYTESIZE, "ScanChainDCB.dat", false, true);
	else
		scanChainECL = new ConfDataArray(scanChainECL_FEI4B_ds, SC_ECL_B_ITEMS, SC_ECL_B_BYTESIZE, "ScanChainDCB.dat", false, true);

	
	scanChainDOB = 0;				//FIXME
	scanChainDOB = new ConfDataArray(scanChainDOBds, SC_DOB_ITEMS, SC_DOB_BYTESIZE, "ScanChainDOB.dat", false, true);
}

ConfigFEMemory::~ConfigFEMemory()
{
	//for (int i = 0; i < 32; i++)
	//{
	//	delete globalReg[i];
	//}
	delete[] SRAMdataRB;
	delete scanChainDOB;
	delete scanChainECL;
	delete scanChainCMD;


	delete trigger;
	delete fast_command;
	delete slow_command;

  delete shift_globalConf_C;
  delete shift_globalConf_AB;

	delete globalReg_0;
	delete globalReg_1;
	delete globalReg_2;
	delete globalReg_3;
	delete globalReg_4;
	delete globalReg_5;
	delete globalReg_6;
	delete globalReg_7;
	delete globalReg_8;
	delete globalReg_9;
	delete globalReg_10;
	delete globalReg_11;
	delete globalReg_12;
	delete globalReg_13;
	delete globalReg_14;
	delete globalReg_15;
	delete globalReg_16;
	delete globalReg_17;
	delete globalReg_18;
	delete globalReg_19;
	delete globalReg_20;
	delete globalReg_21;
	delete globalReg_22;
	delete globalReg_23;
	delete globalReg_24;
	delete globalReg_25;
	delete globalReg_26;
	delete globalReg_27;
	delete globalReg_28;
	delete globalReg_29;
	delete globalReg_30;
	delete globalReg_31;
	delete globalReg_32;
	delete globalReg_33;
	delete globalReg_34;
	delete globalReg_35;
	delete globalReg_40;
	delete globalReg_41;
	delete globalReg_42;

	for (int i = 0; i < 40; i++)
	{
    delete pixelHITBUS[i];
    delete pixelHITBUSRB[i];
		delete pixelTDAC0[i];
		delete pixelTDAC0RB[i];
		delete pixelTDAC1[i];
		delete pixelTDAC1RB[i];
		delete pixelTDAC2[i];
		delete pixelTDAC2RB[i];
		delete pixelTDAC3[i];
		delete pixelTDAC3RB[i];
		delete pixelTDAC4[i];
		delete pixelTDAC4RB[i];
		delete pixelFDAC0[i];
		delete pixelFDAC0RB[i];
		delete pixelFDAC1[i];
		delete pixelFDAC1RB[i];
		delete pixelFDAC2[i];
		delete pixelFDAC2RB[i];
		delete pixelFDAC3[i];
		delete pixelFDAC3RB[i];
		delete pixelENABLE[i];
		delete pixelENABLERB[i];
		delete pixelCAP0[i];
		delete pixelCAP0RB[i];
		delete pixelCAP1[i];
		delete pixelCAP1RB[i];
		delete pixelDIGINJ[i];
	}
}


// ************************ begin functions *****************************************************
void ConfigFEMemory::SetSlowControlMode()
{
	useSlowControl = true;

	unsigned char reg_data = 0x00;
	unsigned char reg_data_RB;

	//QMutexLocker locker(myUSB->getMutex());

	myUSB->ReadXilinx(CS_SYSTEM_CONF, &reg_data_RB, 1);

	
	if (0x08 & reg_data_RB)
		reg_data = reg_data_RB;
	else 
		reg_data = reg_data_RB + 0x08;

	myUSB->WriteXilinx(CS_SYSTEM_CONF, &reg_data, 1);
}

void ConfigFEMemory::SetCMDMode()
{
	useSlowControl = false;

	unsigned char reg_data = 0x00;
	unsigned char reg_data_RB;

	//QMutexLocker locker(myUSB->getMutex());

	myUSB->ReadXilinx(CS_SYSTEM_CONF, &reg_data_RB, 1);
	
	if (0x08 & reg_data_RB)
		reg_data = reg_data_RB - 0x08;
	else 
		reg_data = reg_data_RB;

	myUSB->WriteXilinx(CS_SYSTEM_CONF, &reg_data, 1);
}

void ConfigFEMemory::setSelAltBus(bool on_off)
{
	unsigned char reg_data = 0x00;
	unsigned char reg_data_RB;

	//QMutexLocker locker(myUSB->getMutex());

	myUSB->ReadXilinx(CS_CMOS_LINES, &reg_data_RB, 1);

	if (on_off)
	{
		if (0x08 & reg_data_RB)
			reg_data = reg_data_RB;
		else 
			reg_data = reg_data_RB + 0x08;
	}
	else 
	{
		if (0x08 & reg_data_RB)
			reg_data = reg_data_RB - 0x08;
		else 
			reg_data = reg_data_RB;
	}

	myUSB->WriteXilinx(CS_CMOS_LINES, &reg_data, 1);
}

void ConfigFEMemory::setSelCMD(bool on_off)
{
	unsigned char reg_data = 0x00;
	unsigned char reg_data_RB;

	//QMutexLocker locker(myUSB->getMutex());

	myUSB->ReadXilinx(CS_CMOS_LINES, &reg_data_RB, 1);

	if (on_off)
	{
		if (0x04 & reg_data_RB)
			reg_data = reg_data_RB;
		else 
			reg_data = reg_data_RB + 0x04;
	}
	else 
	{
		if (0x04 & reg_data_RB)
			reg_data = reg_data_RB - 0x04;
		else 
			reg_data = reg_data_RB;
	}

	myUSB->WriteXilinx(CS_CMOS_LINES, &reg_data, 1);
}

void ConfigFEMemory::SetChipAdd(int new_chip_add)
{
	chip_addr = new_chip_add;
}

int ConfigFEMemory::GetChipAdd()
{
	return chip_addr;
}

void ConfigFEMemory::SendBitstream(unsigned char * bitstream, int bitsize)
{
	//QMutexLocker locker(myUSB->getMutex());
	writeFEMemory(bitstream, bitsize, true);
}

void ConfigFEMemory::writeFEMemory(unsigned char *data_array, short int size, bool isGlobal)
{
	unsigned char reg_data;					
	reg_data = 0x01;
  // private function, no need to use mutex locker - but must lock *all* functions calling this!
  if (!fifo_config)
  {
    while ((reg_data == 0x01 || reg_data == 0x02))
    {
      myUSB->ReadXilinx(CS_START_CONFIG, &reg_data, 1);
    }
	}

  if (!myUSB->HandlePresent())
    return;

	unsigned char cmd_dcnt[3];
	int array_size;

	array_size = ((size + 7)/8);
	if (fifo_config)
	{
		size = array_size * 8;
	}

	cmd_dcnt[0] = 0xff & size;
	cmd_dcnt[1] = 0xff & (size >> 8);

	if (isGlobal)
	{
		cmd_dcnt[2] = 0x01;		//start config_sm with global data

		myUSB->WriteExternal(CS_CONFIG_GLOBAL_WRITEMEM, data_array, array_size);	// write configuration-data to BlockRAM
	}
	else
	{
		cmd_dcnt[2] = 0x02;		//start config_sm with pixel data

		myUSB->WriteExternal(CS_CONFIG_PIXEL_WRITEMEM, data_array, array_size);	// write configuration-data to BlockRAM
	}
  myUSB->WriteExternal(CS_WRITE_CONFIG_SM, cmd_dcnt, 3);
}



void ConfigFEMemory::readFEMemory(unsigned char *data_array, short int size, bool isGlobal)
{
  // private function, no need to use mutex locker - but must lock *all* functions calling this!

	int array_size;
	array_size = (size/8);

	if ((size % 8) != 0)
		array_size++;


	if (isGlobal)
		myUSB->ReadExternal(CS_CONFIG_GLOBAL_READMEM, data_array, array_size);	// read configuration data
	else 
		myUSB->ReadExternal(CS_CONFIG_PIXEL_READMEM, data_array, array_size);	// read configuration data

	unsigned char reg_data;					
	reg_data = 0x01;
	while ((reg_data == 0x01) && (myUSB->HandlePresent() ==  true))
	{
		myUSB->ReadXilinx(CS_START_CONFIG, &reg_data, 1);
	}
}

int ConfigFEMemory::GetFEDataRB(bool global)
{
	//int dataRB[5000/*SRAM_WORDSIZE*/];
//	int dataRB[SRAM_WORDSIZE];
	int RegisterDataRB = 0;
	int ItemDataRB = 0;
	int RegisterAddressRB = 999;
	bool RecIsGlobal = global;
	//	bool VR_found = false;
	//bool LowData = false;
	int wordnr = 0;
	//unsigned int SRAMdataRB[SRAM_WORDSIZE];
	for (int i = 0; i< SRAM_WORDSIZE; i++)
		SRAMdataRB[i] = 0;

	// reset old values
	if (global)
	{
		for (int reg = 2; reg < 35; reg++)
		{
			if (!setRegisterNumber(reg)) 
        continue;
      if (dummyRegisterSelected())
        continue;
			RegisterDataRB = 0x00000000;
			theGlobalReg->UpdateByteArrayItem(24, 16, RegisterDataRB, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, theGlobalReg->byteArrayRB);
			theGlobalReg->MakeStructListRB();
		}
	}
	else
	{
		for (int pix = PIXEL26880; pix <= PIXEL32; pix++)
			theLatch->SetValueRB(pix, 0x00000000);
	}

	configReg->ReadSRAM();

	configReg->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE_PER_CHIP, this->index);

	//for (int i = 0; i < 5000/*SRAM_WORDSIZE*/; i++)
//	for (int i = 0; i < SRAM_WORDSIZE; i++)
//	{
//		dataRB[i] = configReg->SRAMwordsRB[i];
//	}	

	

	//for (int i = 0; i < 5000/*SRAM_WORDSIZE*/; i++)
	for (int i = 0; i < SRAM_WORDSIZE; i++)
	{
		//if (((dataRB[i] & 0x00ff0000) >> 16) == 0xEA) // Adress record
		if (((/*configReg->SRAMwordsRB*/SRAMdataRB[i] & 0x00ff0000) >> 16) == 0xEA) // Adress record
		{
			//RegisterAddressRB = 0x00007FFF & dataRB[i];
			RegisterAddressRB = 0x00007FFF & /*configReg->SRAMwordsRB*/SRAMdataRB[i];
			//RecIsGlobal = !(0x00008000 && dataRB[i]);
			if (RecIsGlobal)
			{
				setRegisterNumber(RegisterAddressRB);
        //std::cerr << "AR: 0x" << std::hex << RegisterAddressRB << std::dec << std::endl; // @todo remove
			}
			/*else
			{
				if ((RegisterAddressRB % 32) != 0) // needs prooving...
				{
					LowData = true;
				}
				else
				{
					LowData = false;
				}
			}*/// for pixel reg the latch needs to be set before calling this function...
		} // if (Address Record)
		//if (((dataRB[i] & 0x00ff0000) >> 16) == 0xEC) // Value record
		if (((/*configReg->SRAMwordsRB*/SRAMdataRB[i] & 0x00ff0000) >> 16) == 0xEC) // Value record
		{
			if (RecIsGlobal)
			{
				//RegisterDataRB = 0x0000ffff & dataRB[i];
				RegisterDataRB = 0x0000ffff & /*configReg->SRAMwordsRB*/SRAMdataRB[i];
				int regindex = 4;
				int add = 0;
				int size = 0;


				int index = theGlobalReg->dataList[regindex].index;
        //std::cerr << "VR: 0x" << std::hex << RegisterDataRB << std::dec << std::endl; // @todo remove

				while ((theGlobalReg->GetNameFromIndex(index)) != "INDEX NOT FOUND")
				{
					int bitchoose = 0;
					add = theGlobalReg->dataListRB[regindex].add;
					size = theGlobalReg->dataListRB[regindex].size;

					for (int c = 0; c < size; c++)
						bitchoose = (bitchoose << 1) + 0x01;

					bitchoose = bitchoose << (40 - (add + size));			//(add - (40 - (add - size)));

					//ItemDataRB = (bitchoose & dataRB[i]) >> (40 - (add + size));
					ItemDataRB = (bitchoose & /*configReg->SRAMwordsRB*/SRAMdataRB[i]) >> (40 - (add + size));
          //std::cerr << "VR bit: 0x" << std::hex << ItemDataRB << std::dec << std::endl; // @todo remove
					theGlobalReg->UpdateByteArrayItem(add, size, ItemDataRB, theGlobalReg->dataList[regindex].inv_bitorder, theGlobalReg->byteArrayRB);
					regindex++;
					if (regindex == theGlobalReg->size)
						break;
					index = theGlobalReg->dataList[regindex].index;
				}
				theGlobalReg->MakeStructListRB();
			}
			else
			{
				wordnr++;
				if (0x01 & wordnr)
				{
					//RegisterDataRB = (0x0000ffff & dataRB[i]) << 16;
					RegisterDataRB = (0x0000ffff & /*configReg->SRAMwordsRB*/SRAMdataRB[i]) << 16;
				}
				else
				{
					//RegisterDataRB = RegisterDataRB + (0x0000ffff & dataRB[i]);
					RegisterDataRB = RegisterDataRB + (0x0000ffff & /*configReg->SRAMwordsRB*/SRAMdataRB[i]);
					// FE latches back inverted data...
					RegisterDataRB = ~RegisterDataRB;
					theLatch->UpdateByteArrayItem(24 + (((wordnr/2)-1) * 32), 32, RegisterDataRB, false, theLatch->byteArrayRB);
					theLatch->MakeStructListRB();
				}
			}
			// VR_found = true;

		} // if (Value Record)
	} // loop over SRAMwords

	return RegisterDataRB;
}

int ConfigFEMemory::GetFEDataRB(ConfDataArray *)// theReg)
{
        //# define RB_SIZE 2
	//int dataRB[RB_SIZE];
	//int dataRB[SRAM_WORDSIZE];
	int RegisterDataRB = 0;
	int ItemDataRB = 0;
	//int RegisterAddressRB = 999;
	//bool RecIsGlobal = 0;
	bool VR_found = false;
	//unsigned int SRAMdataRB[SRAM_WORDSIZE];
	//bool LowData = false;

	//configReg->ReadSRAM(0, 1);
	for (int i = 0; i< SRAM_WORDSIZE; i++)
		SRAMdataRB[i] = 0;
	configReg->ReadSRAM();
	configReg->GetSRAMWordsRB(SRAMdataRB, SRAM_WORDSIZE);
	//configReg->BuildWords();

	//for (int i = 0; i < RB_SIZE; i++)
// 	for (int i = 0; i < SRAM_WORDSIZE; i++)
// 	{
// 		dataRB[i] = configReg->SRAMwordsRB[i];
// 	}	

	//test only
	//dataRB[3] = 0x00EC0801;

	for (int i = 0; i < SRAM_WORDSIZE/*SRAM_WORDSIZE*/; i++)
	{
		/*if (((dataRB[i] & 0x00ff0000) >> 16) == 0xEA) // Adress record
		{
			RegisterAddressRB = 0x00007FFF & dataRB[i];
			RecIsGlobal = 0x00008000 && dataRB[i];
			if (RecIsGlobal)
			{
				setRegisterNumber(RegisterAddressRB);
			}
			else
			{
				if ((RegisterAddressRB % 32) != 0) // needs prooving...
				{
					LowData = true;
				}
				else
				{
					LowData = false;
				}
			}// for pixel reg the latch needs to be set before calling this function...
		}*/
		//if (((dataRB[i] & 0x00ff0000) >> 16) == 0xEC) // Value record
		if (((/*configReg->SRAMwordsRB*/SRAMdataRB[i] & 0x00ff0000) >> 16) == 0xEC) // Value record
		{
			//if (RecIsGlobal)
		                RegisterDataRB = 0x0000ffff & /*configReg->SRAMwordsRB*/SRAMdataRB[i];//dataRB[i];
				int regindex = 4;
				int add = 0;
				int size = 0;

				
				int index = theGlobalReg->dataList[regindex].index;

				while ((theGlobalReg->GetNameFromIndex(index)) != "INDEX NOT FOUND")
				{
					int bitchoose = 0;
					add = theGlobalReg->dataListRB[regindex].add;
					size = theGlobalReg->dataListRB[regindex].size;

					for (int c = 0; c < size; c++)
						bitchoose = (bitchoose << 1) + 0x01;

					bitchoose = bitchoose << (40 - (add + size));			//(add - (40 - (add - size)));

					//ItemDataRB = (bitchoose & dataRB[i]) >> (40 - (add + size));
					ItemDataRB = (bitchoose & /*configReg->SRAMwordsRB*/SRAMdataRB[i]) >> (40 - (add + size));
					theGlobalReg->UpdateByteArrayItem(add, size, ItemDataRB, theGlobalReg->dataList[regindex].inv_bitorder, theGlobalReg->byteArrayRB);
					regindex++;
					index = theGlobalReg->dataList[regindex].index;
				}
				theGlobalReg->MakeStructListRB();
			//else
			//{
				//if (LowData)
			//	{
			//		RegisterDataRB = 0x0000ffff & dataRB[i];
			//	}
			//	else
			//	{
			//		RegisterDataRB = RegisterDataRB + ((RegisterDataRB = 0x0000ffff & dataRB[i]) << 16);
			//		theLatch->UpdateByteArrayItem(24, 16, RegisterDataRB, theGlobalReg->byteArrayRB);
			//		theGlobalReg->MakeStructListRB();
			//	}
			//}
				VR_found = true;
		}
	}
	if (VR_found == false)
	{
		RegisterDataRB = 0x00000000;
		theGlobalReg->UpdateByteArrayItem(24, 16, RegisterDataRB, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, theGlobalReg->byteArrayRB);
		theGlobalReg->MakeStructListRB();
		//		int regnr = theReg->GetValue(theReg->GetIndexFromName("REGADDRESS"));
	}
	return RegisterDataRB;
}

void ConfigFEMemory::HandleFECommand(int major_command, int /*minor_command*/, int chip_add, unsigned char *data_array)
{
  int c_chipaddress = FEI4B ? B_CHIPADDRESS : CHIPADDRESS;
  int c_command     = FEI4B ? B_COMMAND     : COMMAND;

  bool successful = true;
	switch(major_command)
	{
	case FE_WRITE_GLOBAL:
    if (dummyRegisterSelected())
      return;
    successful &= theGlobalReg->UpdateByteArrayItem(c_command,     major_command, data_array);
    successful &= theGlobalReg->UpdateByteArrayItem(c_chipaddress, chip_add,      data_array);
    if (successful)
      writeFEMemory(data_array, CONFIG_REG_BITSIZE, true);
		break;

	case FE_WRITE_PIXEL:
    successful &= theLatch->UpdateByteArrayItem(-1, major_command, data_array, COMMAND);
    successful &= theLatch->UpdateByteArrayItem(-1, 8,             data_array, CHIPADDRESS);
    if (successful)
      writeFEMemory(data_array, PIXEL_REG_BITSIZE, false);
		break;

	case FE_LV1_TRIGGER:
    trigger->UpdateByteArrayItem(c_command, major_command, data_array, COMMAND);
    if (successful)
      writeFEMemory(data_array, TRIGGER_REG_BITSIZE, false);
		break;

  case FE_BCR:
  case FE_CAL:
  case FE_ECR:
    successful &= fast_command->UpdateByteArrayItem(c_command, major_command, data_array, 1);
    if (successful)
    {
      writeFEMemory(data_array, FAST_REG_BITSIZE, false);
    }
    break;
//	case FE_BCR:
//		fast_command->UpdateByteArrayItem(fast_command->dataList[COMMAND].add,      fast_command->dataList[COMMAND].size, major_command/* | parity */, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
//		writeFEMemory(data_array, FAST_REG_BITSIZE, false);
//		break;
//	case FE_CAL:
//		fast_command->UpdateByteArrayItem(fast_command->dataList[COMMAND].add,      fast_command->dataList[COMMAND].size, major_command/* | parity */, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
//		writeFEMemory(data_array, FAST_REG_BITSIZE, false);
//		break;
//	case FE_ECR:
//		fast_command->UpdateByteArrayItem(fast_command->dataList[COMMAND].add,      fast_command->dataList[COMMAND].size, major_command/* | parity */, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
//		writeFEMemory(data_array, FAST_REG_BITSIZE, false);
//		break;
	case FE_GLOBAL_RESET:
	case FE_GLOBAL_PULSE:
	case FE_EN_DATA_TAKE:
	case FE_READ_GLOBAL:
		//slow_command->UpdateByteArrayItem(slow_command->dataList[COMMAND].add,      slow_command->dataList[COMMAND].size, major_command, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
		//slow_command->UpdateByteArrayItem(slow_command->dataList[CHIPADDRESS].add,      slow_command->dataList[CHIPADDRESS].size, chip_add, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set address
    successful &= slow_command->UpdateByteArrayItem(c_command,     major_command, data_array, COMMAND);
    successful &= slow_command->UpdateByteArrayItem(c_chipaddress, chip_add,      data_array, CHIPADDRESS);
    if (successful)
    {
      writeFEMemory(data_array, SLOW_REG_BITSIZE, false);
    }
		break;
//	case FE_GLOBAL_PULSE:
//		slow_command->UpdateByteArrayItem(slow_command->dataList[COMMAND].add,      slow_command->dataList[COMMAND].size, major_command/* | parity */, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
//		slow_command->UpdateByteArrayItem(slow_command->dataList[CHIPADDRESS].add,      slow_command->dataList[CHIPADDRESS].size, chip_add, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set address
//		writeFEMemory(data_array, SLOW_REG_BITSIZE, false);
//		break;
//	case FE_EN_DATA_TAKE:
//		slow_command->UpdateByteArrayItem(slow_command->dataList[COMMAND].add,      slow_command->dataList[COMMAND].size, major_command/* | parity */, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
//		slow_command->UpdateByteArrayItem(slow_command->dataList[CHIPADDRESS].add,      slow_command->dataList[CHIPADDRESS].size, chip_add, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set address
//		writeFEMemory(data_array, SLOW_REG_BITSIZE, false);
//		break;
//	case FE_READ_GLOBAL:
//		slow_command->UpdateByteArrayItem(slow_command->dataList[COMMAND].add,      slow_command->dataList[COMMAND].size, major_command/* | parity */, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set command
//		slow_command->UpdateByteArrayItem(slow_command->dataList[CHIPADDRESS].add,      slow_command->dataList[CHIPADDRESS].size, chip_add, theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(FEI4B?B_COMMAND:COMMAND)].inv_bitorder, data_array); // set address
//		writeFEMemory(data_array, SLOW_REG_BITSIZE, false);
//		break;
		// *********** slow control **********************
	case FE_WRITE_GLOBAL_AB:
		writeFEMemory(data_array, SHIFT_REG_AB_BITSIZE, true);
		break;
	case FE_WRITE_GLOBAL_C:
		writeFEMemory(data_array, SHIFT_REG_C_BITSIZE, true);
		break;
	case FE_READ_GLOBAL_AB:
		writeFEMemory(shift_globalConf_AB->nullArray, SHIFT_REG_AB_BITSIZE, true);
		readFEMemory(data_array, SHIFT_REG_AB_BITSIZE, true);
		break;
	case FE_READ_GLOBAL_C:
		writeFEMemory(shift_globalConf_C->nullArray, SHIFT_REG_C_BITSIZE, true);
		readFEMemory(data_array, SHIFT_REG_C_BITSIZE, true);
		break;
	case FE_WRITE_SC_DOB:
		writeFEMemory(data_array, SC_DOB_BITSIZE, true);
		break;
	case FE_WRITE_SC_CMD:
		writeFEMemory(data_array, SC_CMD_BITSIZE, true);
		break;
	case FE_WRITE_SC_ECL:
		writeFEMemory(data_array, FEI4B?SC_ECL_B_BITSIZE:SC_ECL_A_BITSIZE, true);
		break;
	case FE_READ_SC_DOB:
		writeFEMemory(scanChainDOB->nullArray, SC_DOB_BITSIZE, true); 
		readFEMemory(data_array, SC_DOB_BITSIZE, true);
		break;
	case FE_READ_SC_CMD:
		writeFEMemory(scanChainCMD->nullArray, SC_CMD_BITSIZE+1, true); 
		readFEMemory(data_array, SC_CMD_BITSIZE+1, true);
		break;
	case FE_READ_SC_ECL:
		writeFEMemory(scanChainECL->nullArray, FEI4B?SC_ECL_B_BITSIZE:SC_ECL_A_BITSIZE, true); 
		readFEMemory(data_array, FEI4B?SC_ECL_B_BITSIZE:SC_ECL_A_BITSIZE, true);
		break;
	default:// FE_REF_RESET, FE_SOFT_RESET, FE_WRITE_GLOBAL, FE_READ_GLOBAL,
		//writeFEMemory(data_array, DCNT_LOAD, true);
		break;

	}
//	unsigned char FPGAbusy = 1;
//	while (((FPGAbusy == 0x01) || (FPGAbusy == 0x02)) && myUSB->HandlePresent())
//		myUSB->ReadXilinx(CS_START_CONFIG, &FPGAbusy, 1);

}

void ConfigFEMemory::SetUSBHandle(SiUSBDevice * Handle)
{
	myUSB = Handle;
}

void ConfigFEMemory::WriteCommand(int the_command, int GlobalPulseLength)
{
	switch (the_command)
	{
	case FE_LV1_TRIGGER:
		trigger->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, trigger->byteArray);
		break;
	case FE_BCR:
		fast_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, fast_command->byteArray);
		break;
	case FE_ECR:
		fast_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, fast_command->byteArray);
		break;
	case FE_CAL:
		fast_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, fast_command->byteArray);
		break;
	case FE_GLOBAL_RESET:
		slow_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, slow_command->byteArray);
		break;
	case FE_GLOBAL_PULSE:
		slow_command->SetValue(FIELD5, GlobalPulseLength);
		slow_command->SetValue(CHIPADDRESS, 8);  // TODO: set Pulse Width (set in REGADRESS FIELD) to correct length (here set to 1!!!)
		slow_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, slow_command->byteArray);
		break;
	case FE_READ_GLOBAL:
		slow_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, slow_command->byteArray);
		break;
	case FE_EN_DATA_TAKE:
		slow_command->SetValue(FIELD5, 0x38);
		slow_command->MakeByteArray();
		HandleFECommand(the_command, 0, chip_addr, slow_command->byteArray);
		ChipInRunMode = true;
		break;
	case FE_CONF_MODE:
		slow_command->SetValue(FIELD5, 0x07);
		slow_command->MakeByteArray();
		HandleFECommand(FE_EN_DATA_TAKE, 0, chip_addr, slow_command->byteArray);  //set FE_EN_DATA_TAKE with different FIELD5!
		ChipInRunMode = false;
		break;
	}
	//HandleFECommand(FE_CMD_NULL,     0, chip_addr, globalReg[0]->nullArray);
}

int ConfigFEMemory::IndexToRegisterNumber(int the_index)
{
	for (int i = 0; i <= 35; i++)
	{
		setRegisterNumber(i);
			if (theGlobalReg->GetNameFromIndex(the_index) != "INDEX NOT FOUND")
				return i;
	}

// 	for (int i = 31; i <= 35; i++)
// 	{
// 		setRegisterNumber(i);
// 			if (theGlobalReg->GetNameFromIndex(the_index) != "INDEX NOT FOUND")
// 				return i;
// 	}

	for (int i = 40; i <= 42; i++)
	{
		setRegisterNumber(i);
			if (theGlobalReg->GetNameFromIndex(the_index) != "INDEX NOT FOUND")
				return i;
	}

	return -1;
	
}	

int ConfigFEMemory::IndexToShiftRegIndex(int the_index)
{
	int regindex = 0;
	the_globalShiftReg = shift_globalConf_AB;
	regindex = the_globalShiftReg->GetRegindexFromIndex(the_index);
	
	if (regindex != -1)
		return regindex;
	else
	{
		the_globalShiftReg = shift_globalConf_C;
		regindex = the_globalShiftReg->GetRegindexFromIndex(the_index);
	}
	
	return regindex;
}

bool ConfigFEMemory::setRegisterNumber(int RegisterNumber)
{
	switch (RegisterNumber)
	{
	case 0:
		theGlobalReg = globalReg_0;
		break;
	case 1:
		theGlobalReg = globalReg_1;
		break;
	case 2:
		theGlobalReg = globalReg_2;
		break;
	case 3:
		theGlobalReg = globalReg_3;
		break;
	case 4:
		theGlobalReg = globalReg_4;
		break;
	case 5:
		theGlobalReg = globalReg_5;
		break;
	case 6:
		theGlobalReg = globalReg_6;
		break;
	case 7:
		theGlobalReg = globalReg_7;
		break;
	case 8:
		theGlobalReg = globalReg_8;
		break;
	case 9:
		theGlobalReg = globalReg_9;
		break;
	case 10:
		theGlobalReg = globalReg_10;
		break;
	case 11:
		theGlobalReg = globalReg_11;
		break;
	case 12:
		theGlobalReg = globalReg_12;
		break;
	case 13:
		theGlobalReg = globalReg_13;
		break;
	case 14:
		theGlobalReg = globalReg_14;
		break;
	case 15:
		theGlobalReg = globalReg_15;
		break;
	case 16:
		theGlobalReg = globalReg_16;
		break;
	case 17:
		theGlobalReg = globalReg_17;
		break;
	case 18:
		theGlobalReg = globalReg_18;
		break;
	case 19:
		theGlobalReg = globalReg_19;
		break;
	case 20:
		theGlobalReg = globalReg_20;
		break;
	case 21:
		theGlobalReg = globalReg_21;
		break;
	case 22:
		theGlobalReg = globalReg_22;
		break;
	case 23:
		theGlobalReg = globalReg_23;
		break;
	case 24:
		theGlobalReg = globalReg_24;
		break;
	case 25:
		theGlobalReg = globalReg_25;
		break;
	case 26:
		theGlobalReg = globalReg_26;
		break;
	case 27:
		theGlobalReg = globalReg_27;
		break;
	case 28:
		theGlobalReg = globalReg_28;
		break;
	case 29:
		theGlobalReg = globalReg_29;
		break;
	case 30:
		theGlobalReg = globalReg_30;
		break;
	case 31:
		theGlobalReg = globalReg_31;
		break;

		// EFUSE register
	case 32:
		theGlobalReg = globalReg_32;
		break;
	case 33:
		theGlobalReg = globalReg_33;
		break;
	case 34:
		theGlobalReg = globalReg_34;
		break;
	case 35:
		theGlobalReg = globalReg_35;
		break;

		// read-only registers
	case 40:
		theGlobalReg = globalReg_40;
		break;
	case 41:
		theGlobalReg = globalReg_41;
		break;
	case 42:
		theGlobalReg = globalReg_42;
		break;
	default:
		return false;
	}
  this->registerNumber = RegisterNumber;
	return true;
}

void ConfigFEMemory::WriteGlobal() // work needed: command flow / write all registers... 
{
	if (useSlowControl)
	{
		{	// write SR C
			//bool ld_line_state = false;
			//// set ld line low
			//int dataRB = configReg->ReadRegister(CS_INMUX_IN_CTRL); 
			//int data = 0x00;
			//if (dataRB & 0x02)
			//{
			//	ld_line_state = true;
			//	data = dataRB - 0x02;
			//}
			//else
			//	data = dataRB;
			//configReg->WriteRegister(CS_INMUX_IN_CTRL, data);

			// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to Creg state
			//int data = 0x36;
			//configReg->WriteRegister(CS_INMUX_CONTROL, data);

			//shift_globalConf_C->MakeByteArray();
			//HandleFECommand(FE_WRITE_GLOBAL_C, 0, chip_addr, shift_globalConf_C->byteArray); // write to global register C

			// toggle ld line
			//dataRB = configReg->ReadRegister(CS_INMUX_IN_CTRL); 
			//data = 0x00;
			//if (dataRB & 0x02)
			//	data = dataRB;
			//else
			//	data = dataRB + 0x02;
			//configReg->WriteRegister(CS_INMUX_IN_CTRL, data);

			//// reset ld line
			//dataRB = configReg->ReadRegister(CS_INMUX_IN_CTRL); 
			//data = 0x00;

			//if (ld_line_state)
			//{
			//	if (dataRB & 0x02)
			//		data = dataRB;
			//	else
			//		data = dataRB + 0x02;
			//}
			//else
			//{
			//	if (dataRB & 0x02)
			//		data = dataRB - 0x02;
			//	else
			//		data = dataRB;
			//}
			//configReg->WriteRegister(CS_INMUX_IN_CTRL, data);
		} // end write SR C



		{	// write SR AB
			//int ld_line_state = false;
			//// set ld line low
			//int dataRB = configReg->ReadRegister(CS_INMUX_IN_CTRL); 
			//int data = 0x00;
			//if (dataRB & 0x02)
			//{
			//	ld_line_state = true;
			//	data = dataRB - 0x02;
			//}
			//else
			//	data = dataRB;
			//configReg->WriteRegister(CS_INMUX_IN_CTRL, data);

			// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to Creg state
			int data = 0x3f;
			configReg->WriteRegister(CS_INMUX_CONTROL, data);

			shift_globalConf_AB->MakeByteArray();
			HandleFECommand(FE_WRITE_GLOBAL_AB, 0, chip_addr, shift_globalConf_AB->byteArray); // write to global register C
			shift_globalConf_AB->MakeStructListRB();
			shift_globalConf_AB->FileSaveRB("debug_SR_AB_shiftthrough.dat", 2);

			//// toggle ld line
			//dataRB = configReg->ReadRegister(CS_INMUX_IN_CTRL); 
			//data = 0x00;
			//if (dataRB & 0x02)
			//	data = dataRB;
			//else
			//	data = dataRB + 0x02;
			//configReg->WriteRegister(CS_INMUX_IN_CTRL, data);

			//// reset ld line
			//dataRB = configReg->ReadRegister(CS_INMUX_IN_CTRL); 
			//data = 0x00;

			//if (ld_line_state)
			//{
			//	if (dataRB & 0x02)
			//		data = dataRB;
			//	else
			//		data = dataRB + 0x02;
			//}
			//else
			//{
			//	if (dataRB & 0x02)
			//		data = dataRB - 0x02;
			//	else
			//		data = dataRB;
			//}
			//configReg->WriteRegister(CS_INMUX_IN_CTRL, data);
		} // end write SR C





		//shift_globalConf_AB->MakeByteArray();
		//HandleFECommand(FE_WRITE_GLOBAL_AB, 0, chip_addr, shift_globalConf_AB->byteArray); // write to global register AB


		//HandleFECommand(FE_READ_GLOBAL_AB, 0, chip_addr, shift_globalConf_AB->byteArrayRB); // read to global register AB
		//shift_globalConf_AB->MakeStructListRB();
		//shift_globalConf_AB->FileSaveRB("shift_reg_testread.dat", 0);

		//shift_globalConf_C->MakeByteArray();
		//HandleFECommand(FE_WRITE_GLOBAL_C, 0, chip_addr, shift_globalConf_C->byteArray); // write to global register C
		//HandleFECommand(FE_READ_GLOBAL_C, 0, chip_addr, shift_globalConf_C->byteArrayRB); // read to global register C
		//shift_globalConf_C->MakeStructListRB();
		//shift_globalConf_C->FileSaveRB("shift_reg_testread.dat", 1);

		//HandleFECommand(FE_CLOCK_GLOBAL, 0, chip_addr, globalReg[0]->nullArray); // fill shift register with zeros
		//HandleFECommand(FE_CMD_NULL,     0, chip_addr, globalReg[0]->nullArray); // disable command (should be done in shift null_array already, because on rising edge of ld the command_reg is latched)
	}
	else
	{
		int OldVthinAltFine, OldVthinAltCoarse, add, size;
		// set first high threshold
		{
			GetGlobalVarAddVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, add, size, OldVthinAltFine);
			GetGlobalVarAddVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, add, size, OldVthinAltCoarse);
			SetGlobalVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, 255);
			SetGlobalVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, 255);
			WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
		}
		for (int reg = 1; reg <= 35; reg++)
		{
			WriteGlobal(reg);
		}
		// set target threshold last threshold
		{
			SetGlobalVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, OldVthinAltCoarse);
			SetGlobalVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, OldVthinAltFine);
			WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
		}
	}
}

void ConfigFEMemory::WriteGlobal(int RegisterNumber)
{
	bool runmode = ChipInRunMode;
	// enable conf mode
	WriteCommand(FE_CONF_MODE);
	setRegisterNumber(RegisterNumber);
	theGlobalReg->MakeByteArray();	// build array from Struct
	HandleFECommand(FE_WRITE_GLOBAL, 0, chip_addr, theGlobalReg->byteArray); // send write global command
	//theGlobalReg->MakeStructList();
	if (runmode)
		WriteCommand(FE_EN_DATA_TAKE);
}

void ConfigFEMemory::ReadGlobal()  // much work needed: bypass mode...
{
	if (useSlowControl)
	{
		//todo: shift zeros + cmd-flow

		// get read data
		//call --> readFEMemory(unsigned char *data_array, short int size, bool isGlobal)
		
	}
	else
	{
		int add = 0, size = 0,val = 0;
		GetGlobalVarAddVal(FEI4B?B_CONFADDRENABLE:CONFADDRENABLE, add, size, val);
		if (val != 0)
		{
			bool runmode = ChipInRunMode;

			configReg->ClearSRAM();
			WriteCommand(FE_CONF_MODE);
			configReg->SetRunMode();
			configReg->resetRunModeAdd();
			for (int reg = 1; reg <= 35; reg++)
			{
				// set Register Number to theGloablReg and slow command...
				setRegisterNumber(reg);
				slow_command->SetValue(FIELD5, reg);
				WriteCommand(FE_READ_GLOBAL);
			}
			for (int reg = 40; reg <= 42; reg++)
			{
				// set Register Number to theGloablReg and slow command...
				setRegisterNumber(reg);
				slow_command->SetValue(FIELD5, reg);
				WriteCommand(FE_READ_GLOBAL);
			}
			
			GetFEDataRB(true);

			if (runmode)
				WriteCommand(FE_EN_DATA_TAKE);
		}
		else
		{
			for (int reg = 1; reg <= 35; reg++)
			{
				ReadGlobal(reg);
			}
			for (int reg = 40; reg <= 42; reg++)
			{
				ReadGlobal(reg);
			}
		}
	}
}

int ConfigFEMemory::ReadGlobal(int RegisterNumber)
{
	bool runmode = ChipInRunMode;
	int size = 0, add = 0, val = 0;
	GetGlobalVarAddVal(FEI4B?B_CONFADDRENABLE:CONFADDRENABLE, add, size, val);
	bool confaddrenable = 0x01 & val;

	// set chip to conf mode
	WriteCommand(FE_CONF_MODE);
	// set Register Number to theGloablReg and slow command...
	setRegisterNumber(RegisterNumber);
	slow_command->SetValue(FIELD5, RegisterNumber);

	// clear SRAM and send read global command
	configReg->ClearSRAM();
	configReg->SetRunMode();
	configReg->resetRunModeAdd();
	WriteCommand(FE_READ_GLOBAL);

	if (runmode)
		WriteCommand(FE_EN_DATA_TAKE);

	if (confaddrenable)
		return GetFEDataRB(true);
	else
		return GetFEDataRB(theGlobalReg);
}

void ConfigFEMemory::SetGlobalVal(int the_index, int the_value)
{
	// set Index value in CMD mode configuration
	if (IndexToRegisterNumber(the_index) != -1)
			theGlobalReg->SetValue(the_index, the_value);
	
	// set Index value in bypass mode configuration
	if(IndexToShiftRegIndex(the_index) != -1)	// already sets pointer "the_globalShiftReg"...
		the_globalShiftReg->SetValue(the_index, the_value);

	// set length of LV1 in FPGA address on change automatically
	if (FEI4B)
	{
		if (the_index == B_TRIGCNT)
		{
			unsigned char data;

			if (the_value != 0)
				data = 0xff & the_value;
			else
				data = 16;

			//QMutexLocker locker(myUSB->getMutex());
			myUSB->WriteFPGA(CS_L_LV1, &data, 1);
		}

		// set 8b10b on change automatically
		if (the_index == B_DISABLE8B10B)
		{
			if (the_value)
				configReg->disable_8b10_Decoding();
			else
				configReg->enable_8b10_Decoding();
		}
	}
	else
	{
		if (the_index == TRIGCNT)
		{
			unsigned char data;

			if (the_value != 0)
				data = 0xff & the_value;
			else
				data = 16;

			//QMutexLocker locker(myUSB->getMutex());
			myUSB->WriteFPGA(CS_L_LV1, &data, 1);
		}

		// set 8b10b on change automatically
		if (the_index == DISABLE8B10B)
		{
			if (the_value)
				configReg->disable_8b10_Decoding();
			else
				configReg->enable_8b10_Decoding();
		}
	}
}


std::string ConfigFEMemory::getGlobalVarName(int Variable)
{
	std::string name = "INDEX NOT FOUND";
	setRegisterNumber(IndexToRegisterNumber(Variable));
		
	name = theGlobalReg->GetNameFromIndex(Variable);
	return name;	
}

bool ConfigFEMemory::ReadGlobalFile(const char * globalfilename)
{
	if (useSlowControl)
	{
		shift_globalConf_AB->FileOpen(globalfilename);
		shift_globalConf_C->FileOpen(globalfilename);
	}
	else
	{
		for (int reg_nr = 2; reg_nr <= 35; reg_nr++)
		{
			setRegisterNumber(reg_nr);
			if (!theGlobalReg->FileOpen(globalfilename))
				return false;
		}
		for (int reg_nr = 40; reg_nr <= 42; reg_nr++)
		{
			setRegisterNumber(reg_nr);
			if (!theGlobalReg->FileOpen(globalfilename))
				return false;
		}
		/*for (int reg_nr = 50; reg_nr <= 52; reg_nr++)
		{
		setRegisterNumber(reg_nr);
		if (!theGlobalReg->FileOpen(globalfilename))
		return false;
		}*/ //Reserved- do not use
	}
	return true;

}

void ConfigFEMemory::SaveGlobal(const char * newfilename)
{
	for (int reg_nr = 2; reg_nr <= 29; reg_nr++)
	{
		setRegisterNumber(reg_nr);
		theGlobalReg->FileSave(newfilename, reg_nr);
	}
	for (int reg_nr = 31; reg_nr <= 35; reg_nr++)
	{
		setRegisterNumber(reg_nr);
		theGlobalReg->FileSave(newfilename, reg_nr);
	}
	for (int reg_nr = 40; reg_nr <= 42; reg_nr++)
	{
		setRegisterNumber(reg_nr);
		theGlobalReg->FileSave(newfilename, reg_nr);
	}
	/*for (int reg_nr = 50; reg_nr <= 52; reg_nr++)
	{
	setRegisterNumber(reg_nr);
	theGlobalReg->FileSave(newfilename, reg_nr);
	}*/ //Reserved- do not use
}

void ConfigFEMemory::SaveGlobalRB(const char * newfilename)
{
	if (useSlowControl)
	{
		shift_globalConf_AB->FileSaveRB(newfilename, 0);
		shift_globalConf_C->FileSaveRB(newfilename, 1);
	}
	else
	{
		for (int reg_nr = 2; reg_nr <= 29; reg_nr++)
		{
			setRegisterNumber(reg_nr);
			theGlobalReg->FileSaveRB(newfilename, reg_nr);
		}
		for (int reg_nr = 31; reg_nr <= 35; reg_nr++)
		{
			setRegisterNumber(reg_nr);
			theGlobalReg->FileSaveRB(newfilename, reg_nr);
		}
		for (int reg_nr = 40; reg_nr <= 42; reg_nr++)
		{
			setRegisterNumber(reg_nr);
			theGlobalReg->FileSaveRB(newfilename, reg_nr);
		}
	}
}

bool ConfigFEMemory::GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value)
{
	if (IndexToRegisterNumber(Variable) != -1)
	{
		Address = theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(Variable)].add;
		Size = theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(Variable)].size;
		Value = theGlobalReg->dataList[theGlobalReg->GetRegindexFromIndex(Variable)].value;
    return true;
	}
  return false;
}

bool ConfigFEMemory::GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value)
{
	if (IndexToRegisterNumber(Variable) != -1)
	{
		Address = theGlobalReg->dataListRB[theGlobalReg->GetRegindexFromIndex(Variable)].add;
		Size = theGlobalReg->dataListRB[theGlobalReg->GetRegindexFromIndex(Variable)].size;
		Value = theGlobalReg->dataListRB[theGlobalReg->GetRegindexFromIndex(Variable)].value;
    return true;
	}
  return false;
}

void ConfigFEMemory::LoadGlobalDefault()
{
	for (int reg = 2; reg < 35; reg++)
	{
		setRegisterNumber(reg);
		theGlobalReg->LoadDefault();
	}
}

// ******************** PIXEL *****************************************************
void ConfigFEMemory::setLatch(int latch, int doublecolumn)
{
	switch (latch)
	{
	case HITBUS:
		theLatch = pixelHITBUS[doublecolumn];
		theLatchRB = pixelHITBUSRB[doublecolumn];
		break;
	case TDAC0:
		theLatch = pixelTDAC0[doublecolumn];
		theLatchRB = pixelTDAC0RB[doublecolumn];
		break;
	case TDAC1:
		theLatch = pixelTDAC1[doublecolumn];
		theLatchRB = pixelTDAC1RB[doublecolumn];
		break;
	case TDAC2:
		theLatch = pixelTDAC2[doublecolumn];
		theLatchRB = pixelTDAC2RB[doublecolumn];
		break;
	case TDAC3:
		theLatch = pixelTDAC3[doublecolumn];
		theLatchRB = pixelTDAC3RB[doublecolumn];
		break;
	case TDAC4:
		theLatch = pixelTDAC4[doublecolumn];
		theLatchRB = pixelTDAC4RB[doublecolumn];
		break;
	case FDAC0:
		theLatch = pixelFDAC0[doublecolumn];
		theLatchRB = pixelFDAC0RB[doublecolumn];
		break;
	case FDAC1:
		theLatch = pixelFDAC1[doublecolumn];
		theLatchRB = pixelFDAC1RB[doublecolumn];
		break;
	case FDAC2:
		theLatch = pixelFDAC2[doublecolumn];
		theLatchRB = pixelFDAC2RB[doublecolumn];
		break;
	case FDAC3:
		theLatch = pixelFDAC3[doublecolumn];
		theLatchRB = pixelFDAC3RB[doublecolumn];
		break;
	case ENABLE:
		theLatch = pixelENABLE[doublecolumn];
		theLatchRB = pixelENABLERB[doublecolumn];
		break;
	case CAP0:
		theLatch = pixelCAP0[doublecolumn];
		theLatchRB = pixelCAP0RB[doublecolumn];
		break;
	case CAP1:
		theLatch = pixelCAP1[doublecolumn];
		theLatchRB = pixelCAP1RB[doublecolumn];
		break;
	case DIGINJ:
		theLatch = pixelDIGINJ[doublecolumn];
		theLatchRB = pixelENABLERB[doublecolumn]; // not correct, but needs to be filled?
		break;
	}// end switch (latch)
}

int ConfigFEMemory::LatchToPXSTROBE(int latch)
{/*
	int pxstrobe = 999;

	switch (latch)
	{
	case ENABLE:
		pxstrobe = 0;
		break;
	case TDAC4:					//TDAC0 is MSB in FE !!!
		pxstrobe = 1;
		break;
	case TDAC3:
		pxstrobe = 2;
		break;
	case TDAC2:
		pxstrobe = 3;
		break;
	case TDAC1:
		pxstrobe = 4;
		break;
	case TDAC0:
		pxstrobe = 5;
		break;
	case CAP0:
		pxstrobe = 6;
		break;
	case CAP1:
		pxstrobe = 7;
		break;
	case HITBUS:
		pxstrobe = 8;
		break;
	case FDAC0:
		pxstrobe = 9;
		break;
	case FDAC1:
		pxstrobe = 10;
		break;
	case FDAC2:
		pxstrobe = 11;
		break;
	case FDAC3:
		pxstrobe = 12;
		break;
	}
	return pxstrobe;
*/
	
	if (latch != DIGINJ)
	{
		int pxstrobe = 0;
		pxstrobe = latch + (FEI4B?B_PXSTROBE0:PXSTROBE0);
		return pxstrobe;
	}
	else
		return 999;
		
}

void ConfigFEMemory::WritePixel(int latch, int DC)
{
	bool runmode = ChipInRunMode;
	int add = 0;
  int size = 0;
  int colpr_mode = 0;
  int colpr_addr = 0;//, dig_select;
	//GetGlobalVarAddVal(DIGHITIN_SEL, add, size, dig_select);

	WriteCommand(FE_CONF_MODE);
	
	int OldVthinAltFine = 0, OldVthinAltCoarse = 0;
	// set first high threshold
	{
	  GetGlobalVarAddVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, add, size, OldVthinAltFine);
	  GetGlobalVarAddVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, add, size, OldVthinAltCoarse);
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, 255);
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, 255);
	  WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
	}

	//update global register
	{
		// set latch
		if (latch != DIGINJ)
		{
		  if(FEI4B){
			for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
				SetGlobalVal(i, 0);
			SetGlobalVal(LatchToPXSTROBE(latch), 1);
			SetGlobalVal(B_PXSTROBES0, 0);
			SetGlobalVal(B_PXSTROBES1, 0);
			WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
		  } else{
			for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
				SetGlobalVal(i, 0);
			SetGlobalVal(LatchToPXSTROBE(latch), 1);
			SetGlobalVal(PXSTROBES0, 0);
			SetGlobalVal(PXSTROBES1, 0);
			WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
		  }
		}
		// set double column and correct mode
		if (DC == 40)
			SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 3);
		else 
			SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 0);

		if (DC == 40)
			SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, 0);
		else 
			SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, DC);

		WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));

		// set correct shift register mode
		SetGlobalVal(FEI4B?B_HITLD_IN:HITLD_IN, 0);
		WriteGlobal(IndexToRegisterNumber(FEI4B?B_HITLD_IN:HITLD_IN));
	}

	//write pixel shift register
	{
		if (DC == 40)
			setLatch(latch, 0);
		else
			setLatch(latch, DC);
		theLatch->MakeByteArray();
		HandleFECommand(FE_WRITE_PIXEL, 0, chip_addr, theLatch->byteArray);
	}

	//send global pulse to latch SR-content to memory cells
	if (latch != DIGINJ)
	{
		int addr, siz;
		int value[12];
		if(FEI4B){
		  for (int i = 0; i < 12; i++)
		    GetGlobalVarAddVal(i + B_EFUSE_SENSE, addr, siz, value[i]);
		} else{
		  for (int i = 0; i < 9; i++)
		    GetGlobalVarAddVal(i + EFUSE_SENSE, addr, siz, value[i]);
		}

		{
		  SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
		  SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 1);
		  SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
		  SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
		  SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
		  SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
		  if(FEI4B){
		    SetGlobalVal(B_ADC_EN_PULSE, 0);
		    SetGlobalVal(B_SR_RD_EN    , 0);
		    SetGlobalVal(B_REG27SPARES2, 0);
		  }
		  SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
		  SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
		  SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
		  WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

			WriteCommand(FE_GLOBAL_PULSE);
		}

		if(FEI4B){
		  for (int i = 0; i < 12; i++)
		    SetGlobalVal(i + B_EFUSE_SENSE, value[i]);
		}else{
		  for (int i = 0; i < 9; i++)
		    SetGlobalVal(i + EFUSE_SENSE, value[i]);
		}
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
	
		// set all strobes 0 for SEU hardness...
		{
		  if(FEI4B){
		    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
		      SetGlobalVal(i, 0);
		    WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
		  } else{
		    for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
		      SetGlobalVal(i, 0);
		    WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
		  }
		}
	}

	//SetGlobalVal(DIGHITIN_SEL, dig_select);
	//WriteGlobal(IndexToRegisterNumber(DIGHITIN_SEL));
	SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, colpr_mode);
	SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, colpr_addr);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_MODE:COLPR_MODE));

	// set back to target threshold
	{
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, OldVthinAltCoarse);
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, OldVthinAltFine);
	  WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
	}
	
	if (runmode)
		WriteCommand(FE_EN_DATA_TAKE);
}
void ConfigFEMemory::WritePixel(int latch)
{
	/*for (int doublecolumn = 0; doublecolumn < 40; doublecolumn++)
	{
		WritePixel(latch, doublecolumn);
	}*/

	// This should be faster...
	WritePixel(latch, 40);
}

void ConfigFEMemory::WritePixel()
{
	if (useSlowControl)
		;
	// TODO: command flow in slow control mode...!!!
	else
	{
		for (int latch = ENABLE; latch <= FDAC3; latch++)
		{
			WritePixel(latch);
		}
	}
}

void ConfigFEMemory::SetPixelVal(int the_index, int the_value, int latch)
{
	setLatch(latch, PixIndexToDCNumber(the_index));
	theLatch->SetValue(PixIndexToDCindex(the_index), the_value);
}

void ConfigFEMemory::SetPixelVal(int latch, int theDC, int the_DCindex, int the_value)
{
	setLatch(latch, theDC);
	theLatch->SetValue(the_DCindex, the_value);
}


int ConfigFEMemory::PixIndexToDCNumber(int pixindex)
{
	int dc_number = 0;
	dc_number = (pixindex - 4) / DC_ITEM_COUNT;

	return dc_number;
}

int ConfigFEMemory::PixIndexToDCindex(int pixindex)
{
	int dc_index = 0;
	dc_index = ((pixindex-4) % 21) + 4;

	return dc_index;
}

void ConfigFEMemory::ReadPixelFile(const char * pixelfilename, int latch)
{
	for (int doublecolumn = 0; doublecolumn < 40; doublecolumn++)
	{
		setLatch(latch, doublecolumn);
		theLatch->FileOpen(pixelfilename);
	}
}

void ConfigFEMemory::SavePixel(const char * /*newfilename*/, int latch, int doublecolumn)
{
	setLatch(latch, doublecolumn);
	//	theLatch->FileSave(newfilename);
}

void ConfigFEMemory::SavePixelRB(const char * newfilename, int latch, int doublecolumn)
{
	setLatch(latch, doublecolumn);
	theLatch->FileSaveRB(newfilename, doublecolumn);
}

void ConfigFEMemory::LoadPixelLatch(int latch, int DC)
{
	if (latch != DIGINJ)
	{
		//update global register
		{
			// set latch
		  if(FEI4B){
		    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
				SetGlobalVal(i, 0);
			SetGlobalVal(LatchToPXSTROBE(latch), 1);
			SetGlobalVal(B_PXSTROBES0, 0);
			SetGlobalVal(B_PXSTROBES1, 0);
			WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
		  } else{
			for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
				SetGlobalVal(i, 0);
			SetGlobalVal(LatchToPXSTROBE(latch), 1);
			SetGlobalVal(PXSTROBES0, 0);
			SetGlobalVal(PXSTROBES1, 0);
			WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
		  }
			// set double column and correct mode
			if (DC < 40)
			{
				SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 0);
				SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, DC);
			}
			else if (DC == 40)
			{
				SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 3);
				SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, 0);
			}	
			else
				return;


			WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));

			// set correct shift register mode
			SetGlobalVal(FEI4B?B_HITLD_IN:HITLD_IN, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_HITLD_IN:HITLD_IN));
		}

		//send global pulse to latch SR-content to memory cells
		{
		  SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
		  SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 1);
		  SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
		  SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
		  SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
		  SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
		  if(FEI4B){
		    SetGlobalVal(B_ADC_EN_PULSE, 0);
		    SetGlobalVal(B_SR_RD_EN    , 0);
		    SetGlobalVal(B_REG27SPARES2, 0);
		  }
		  SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
		  SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
		  SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
		  WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

			WriteCommand(FE_GLOBAL_PULSE);
		}

		// set all strobes 0 for SEU hardness...
		{
		  if(FEI4B){
		    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
		      SetGlobalVal(i, 0);
		    WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
		  } else{
		    for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
		      SetGlobalVal(i, 0);
		    WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
		  }
		}
	}
}

void ConfigFEMemory::LatchBackToSR(int latch, int DC)
{
	if (latch != DIGINJ)
	{
		int add, size, colpr_mode, colpr_addr;
		GetGlobalVarAddVal(FEI4B?B_COLPR_MODE:COLPR_MODE, add, size, colpr_mode);
		GetGlobalVarAddVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, add, size, colpr_addr);

		//update global register
		{
			// set latch and shift register mode
		  if(FEI4B){
		    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
		      SetGlobalVal(i, 0);
		    SetGlobalVal(LatchToPXSTROBE(latch), 1);
		    SetGlobalVal(B_PXSTROBES0, 1);
		    SetGlobalVal(B_PXSTROBES1, 1);
		    WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
		  } else{
			for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
				SetGlobalVal(i, 0);
			SetGlobalVal(LatchToPXSTROBE(latch), 1);
			SetGlobalVal(PXSTROBES0, 1);
			SetGlobalVal(PXSTROBES1, 1);
			WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
		  }


			// set double column and correct mode
			if (DC < 40)
			{
				SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 0);
				SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, DC);
			}
			else if (DC == 40)
			{
				SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 3);
				SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, 0);
			}
			else
				return;

			WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));
		}

		//send global pulse to latch SR-content to memory cells
		{
			//WriteCommand(FE_GLOBAL_PULSE);
			SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 1);
			SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
			SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
			SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
			SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
			SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
			if(FEI4B){
			  SetGlobalVal(B_ADC_EN_PULSE, 0);
			  SetGlobalVal(B_SR_RD_EN    , 0);
			  SetGlobalVal(B_REG27SPARES2, 0);
			}
			SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
			SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
			SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

			WriteCommand(FE_GLOBAL_PULSE);
		}

		// return to normal mode
		SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, colpr_mode);
		SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, colpr_addr);
		WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));

		{
		  SetGlobalVal(FEI4B?B_PXSTROBES0:PXSTROBES0, 0);
		  SetGlobalVal(FEI4B?B_PXSTROBES1:PXSTROBES1, 0);
		  SetGlobalVal(LatchToPXSTROBE(latch), 0);
		  WriteGlobal(IndexToRegisterNumber(FEI4B?B_PXSTROBE12:PXSTROBE12));
		}
	}
}

void ConfigFEMemory::ReadPixel(int latch, int DC)
{
	bool runmode = ChipInRunMode;
	WriteCommand(FE_CONF_MODE);

	int add, size, colpr_mode = 0, colpr_addr = 0;
	GetGlobalVarAddVal(FEI4B?B_COLPR_MODE:COLPR_MODE, add, size, colpr_mode);
	GetGlobalVarAddVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, add, size, colpr_addr);

	// if latch not DIG_INJ, clear SR and latch back
	if (latch != DIGINJ)
	{
		// Clear SR
		{
			SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 0);
			SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, DC);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));

			SetGlobalVal(FEI4B?B_PXSTROBES0:PXSTROBES0, 0);
			SetGlobalVal(FEI4B?B_PXSTROBES1:PXSTROBES1, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_PXSTROBES1:PXSTROBES1));

			SetGlobalVal(FEI4B?B_HITLD_IN:HITLD_IN, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_HITLD_IN:HITLD_IN));


			SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
			SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
			SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 1);
			SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
			SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
			SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
			if(FEI4B){
			  SetGlobalVal(B_ADC_EN_PULSE, 0);
			  SetGlobalVal(B_SR_RD_EN    , 0);
			  SetGlobalVal(B_REG27SPARES2, 0);
			}
			SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
			SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
			SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

			WriteCommand(FE_GLOBAL_PULSE);
		}
	

		// copy inverse of memory cells to SR
		LatchBackToSR(latch, DC);
	}
	
	//write "0" pixel shift register to get back data
	{
		configReg->ClearSRAM();
		configReg->SetRunMode();
		configReg->resetRunModeAdd();
		setLatch(latch, DC);

		// if FE-I4B, need to enable readback of SR in EOCL
		int oldval = 1;
		if(FEI4B)
		{
			int add, size;
			GetGlobalVarAddVal(B_SR_RD_EN, add, size, oldval);
			if(oldval != 0)
			{
				SetGlobalVal(B_SR_RD_EN, 0);
				WriteGlobal(IndexToRegisterNumber(B_SR_RD_EN));
			}
			SetGlobalVal(B_SR_RD_EN, 1);
			WriteGlobal(IndexToRegisterNumber(B_SR_RD_EN));

		}
		
		HandleFECommand(FE_WRITE_PIXEL, 0, chip_addr, theLatch->nullArray);

		UPGen::Sleep(200);

		if(FEI4B && (oldval == 0))
		{
			SetGlobalVal(B_SR_RD_EN, 0);
			WriteGlobal(IndexToRegisterNumber(B_SR_RD_EN));
		}
	}
	GetFEDataRB(false);

	SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, colpr_mode);
	SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, colpr_addr);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_MODE:COLPR_MODE));

	if (runmode)
		WriteCommand(FE_EN_DATA_TAKE);
}

void ConfigFEMemory::ReadPixel(int latch, int DC, bool bypass)
{
	if (bypass)
	{
		bool runmode = ChipInRunMode;
		WriteCommand(FE_CONF_MODE);

// 		int add = 0;
//     int size = 0;
    int colpr_mode = 0;
    int colpr_addr = 0;

		//// clear SR DCs
		//{
		//	SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 0);
		//	SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, DC);
		//	WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));

		//	if (latch != DIGINJ)
		//	{
		//		if(FEI4B){
		//	    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
		//	      SetGlobalVal(i, 0);
		//	  } else{
		//		for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
		//			SetGlobalVal(i, 0);
		//	  }
		//		SetGlobalVal(LatchToPXSTROBE(latch), 1);
		//		WriteGlobal(IndexToRegisterNumber(FEI4B?B_PXSTROBE0:PXSTROBE0));

		//		LatchBackToSR(latch, DC);
		//	}
		//}
		// if latch not DIG_INJ, clear SR and latch back
	if (latch != DIGINJ)
	{
		// Clear SR
		{
			SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 0);
			SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, DC);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_ADDR:COLPR_ADDR));

			SetGlobalVal(FEI4B?B_PXSTROBES0:PXSTROBES0, 0);
			SetGlobalVal(FEI4B?B_PXSTROBES1:PXSTROBES1, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_PXSTROBES1:PXSTROBES1));

			SetGlobalVal(FEI4B?B_HITLD_IN:HITLD_IN, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_HITLD_IN:HITLD_IN));


			SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
			SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
			SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 1);
			SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
			SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
			SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
			if(FEI4B){
			  SetGlobalVal(B_ADC_EN_PULSE, 0);
			  SetGlobalVal(B_SR_RD_EN    , 0);
			  SetGlobalVal(B_REG27SPARES2, 0);
			}
			SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
			SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
			SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
			WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

			WriteCommand(FE_GLOBAL_PULSE);
		}
	

		// copy inverse of memory cells to SR
		LatchBackToSR(latch, DC);
	}

		//write "0" pixel shift register to get back data
		{
			configReg->ClearSRAM();
			configReg->SetRunMode();
			configReg->resetRunModeAdd();
			setLatch(latch, DC);

			// set slow control and correct MUXES and send bitstream to SR
			int oldmode = configReg->ReadRegister(CS_INMUX_CONTROL);
			int newmode = 0x24;
			configReg->WriteRegister(CS_INMUX_CONTROL, newmode);
			SetSlowControlMode();
			setSelCMD(false);

			//QMutexLocker locker(myUSB->getMutex());
			writeFEMemory(theLatchRB->nullArray, PIXEL_REG_RB_BITSIZE, true);	// has to be global, because only global conf fsm usable in bypass mode...
			readFEMemory(theLatchRB->byteArrayRB, PIXEL_REG_RB_BITSIZE, true);	// has to be global, because only global conf fsm usable in bypass mode...
			//locker.unlock();

			theLatchRB->MakeStructListRB();

			configReg->WriteRegister(CS_INMUX_CONTROL, oldmode);
			SetCMDMode();
			setSelCMD(true);
		}
		SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, colpr_mode);
		SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, colpr_addr);
		WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_MODE:COLPR_MODE));

		if (runmode)
			WriteCommand(FE_EN_DATA_TAKE);
	}
	else
		ReadPixel(latch, DC);
}

void ConfigFEMemory::ReadPixel(int latch)
{
	for (int doublecolumn = 0; doublecolumn < 40; doublecolumn++)
	{
		ReadPixel(latch, doublecolumn);
	}
}

void ConfigFEMemory::ReadPixel(int latch, bool bypass)
{
	if (bypass)
	{
		for (int doublecolumn = 0; doublecolumn < 40; doublecolumn++)
		{
			ReadPixel(latch, doublecolumn, true);
		}
	}
	else
		ReadPixel(latch);
}

void ConfigFEMemory::ReadPixel()
{
	for (int latch = ENABLE; latch <= FDAC3; latch++)
	{
		ReadPixel(latch);
	}
}

void ConfigFEMemory::GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch)
{
	int doublecolumn = PixIndexToDCNumber(Variable);
	setLatch(latch, doublecolumn);
	Address = theLatch->dataList[theLatch->GetRegindexFromIndex(PixIndexToDCindex(Variable))].add;
	Size = theLatch->dataList[theLatch->GetRegindexFromIndex(PixIndexToDCindex(Variable))].size;
	Value = theLatch->dataList[theLatch->GetRegindexFromIndex(PixIndexToDCindex(Variable))].value;
}

void ConfigFEMemory::GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch)
{
	int doublecolumn = PixIndexToDCNumber(Variable);
	setLatch(latch, doublecolumn);
	Address = theLatch/*RB*/->dataListRB[theLatch->GetRegindexFromIndex(PixIndexToDCindex(Variable))].add;
	Size = theLatch/*RB*/->dataListRB[theLatch->GetRegindexFromIndex(PixIndexToDCindex(Variable))].size;
	Value = theLatch/*RB*/->dataListRB[theLatch->GetRegindexFromIndex(PixIndexToDCindex(Variable))].value;
}

void ConfigFEMemory::GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, bool bypass)
{
	if (bypass)
	{
		int doublecolumn = PixIndexToDCNumber(Variable);
		setLatch(latch, doublecolumn);
		Address = theLatchRB->dataListRB[theLatchRB->GetRegindexFromIndex(PixIndexToDCindex(Variable))].add;
		Size = theLatchRB->dataListRB[theLatchRB->GetRegindexFromIndex(PixIndexToDCindex(Variable))].size;
		Value = ~theLatchRB->dataListRB[theLatchRB->GetRegindexFromIndex(PixIndexToDCindex(Variable))].value;
	}
	else
		GetPixelRBVarAddVal(Variable, Address, Size, Value, latch);
}

void ConfigFEMemory::ShiftPixMask(int latch, int steps, bool fillOnes)
{
	int add, size, colpr_mode, colpr_addr;
	GetGlobalVarAddVal(FEI4B?B_COLPR_MODE:COLPR_MODE, add, size, colpr_mode);
	GetGlobalVarAddVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, add, size, colpr_addr);

	bool runmode = ChipInRunMode;
	WriteCommand(FE_CONF_MODE);

	int OldVthinAltFine, OldVthinAltCoarse;
	// set first high threshold
	{
	  GetGlobalVarAddVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, add, size, OldVthinAltFine);
	  GetGlobalVarAddVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, add, size, OldVthinAltCoarse);
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, 255);
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, 255);
	  WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
	}

	if(runmode)
	  WriteCommand(FE_EN_DATA_TAKE);

	if (latch != DIGINJ)
	{
	  LatchBackToSR(latch, 40); // inverted Mask in SR now...
	  if(!fillOnes){ // if ones instead of zeros are supposed to be filled, do the inverting after the shift
	    LoadPixelLatch(latch, 40); // write inverted to memory cells
	    LatchBackToSR(latch, 40); // latch back double inverted --> correct data in SR now
	  }
	}

	// send step amount of single clock pulses to SR
	{
	  // set latch
	  if(FEI4B){
	    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
	      SetGlobalVal(i, 0);
	    SetGlobalVal(LatchToPXSTROBE(latch), 1);
	    SetGlobalVal(B_PXSTROBES0, 0);
	    SetGlobalVal(B_PXSTROBES1, 0);
	    WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
	  } else{
		for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
			SetGlobalVal(i, 0);
		SetGlobalVal(LatchToPXSTROBE(latch), 1);
		SetGlobalVal(PXSTROBES0, 0);
		SetGlobalVal(PXSTROBES1, 0);
		WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
	  }

		// set global pulse to FE_CLK_PULSE mode 
		  SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 1);
		  SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
		  SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
		  SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
		  SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
		  SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
		  if(FEI4B){
		    SetGlobalVal(B_ADC_EN_PULSE, 0);
		    SetGlobalVal(B_SR_RD_EN    , 0);
		    SetGlobalVal(B_REG27SPARES2, 0);
		  }
		  SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
		  SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
		  SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
		  WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

		SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, 3);
		SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, 0);
		WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_MODE:COLPR_MODE));	
	
		for (int i = 0; i < steps; i++)
			WriteCommand(FE_GLOBAL_PULSE);
	}

	// write new mask
	if (latch != DIGINJ)
	  {
	    if(fillOnes){ // do the inverting now
	      LoadPixelLatch(latch, 40); // write inverted to memory cells
	      LatchBackToSR(latch, 40); // latch back double inverted --> correct data in SR now
	    }
	  }
		LoadPixelLatch(latch, 40);

		// reset global pulse mode 
	{
		  SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
		  SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
		  SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
		  SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
		  SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
		  SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
		  if(FEI4B){
		    SetGlobalVal(B_ADC_EN_PULSE, 0);
		    SetGlobalVal(B_SR_RD_EN    , 0);
		    SetGlobalVal(B_REG27SPARES2, 0);
		  }
		  SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
		  SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
		  SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
		  WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
	}
	
		// set all strobes 0 for SEU hardness...
		{
		  if(FEI4B){
		    for (int i = B_PXSTROBE0; i <= B_PXSTROBE12; i++)
		      SetGlobalVal(i, 0);
		    WriteGlobal(IndexToRegisterNumber(B_PXSTROBE12));
		  } else{
			for (int i = PXSTROBE0; i <= PXSTROBE12; i++)
				SetGlobalVal(i, 0);
			WriteGlobal(IndexToRegisterNumber(PXSTROBE12));
		  }
		}

		// fill SR with "0", because SR is overloaded as DIG_INJ !!!
		if (latch != DIGINJ)
			HandleFECommand(FE_WRITE_PIXEL, 0, chip_addr, theLatch->nullArray);

	SetGlobalVal(FEI4B?B_COLPR_MODE:COLPR_MODE, colpr_mode);
	SetGlobalVal(FEI4B?B_COLPR_ADDR:COLPR_ADDR, colpr_addr);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_COLPR_MODE:COLPR_MODE));

	WriteCommand(FE_CONF_MODE);
	// set back to target threshold
	{
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE, OldVthinAltCoarse);
	  SetGlobalVal(FEI4B ? B_VTHIN_ALTFINE : VTHIN_ALTFINE, OldVthinAltFine);
	  WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
	}

	if(runmode)
	  WriteCommand(FE_EN_DATA_TAKE);
}

// ********************************* DO THE SCAN **********************************************
//void ConfigFEMemory::StartmuCScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSize, int InjCount, int MaskStepSize, int MaskStepCount, int ShiftMask)
//{
//	//unsigned char ScanConf[13];
//	//int ScanVarAdd = 0;
//	//int ScanVarSize = 0;
//	//int ScanVarValue = 0;
//	//GetGlobalVarAddVal(ScanVarIndex, ScanVarAdd, ScanVarSize, ScanVarValue);
//
//
//	//ScanConf[0] = CMD_START_SCAN; // starts scan
//	//ScanConf[1] = ScanVarAdd;
//	//ScanConf[2] = ScanVarSize;
//	//ScanConf[3] = (0xff & ScanStartVal);
//	//ScanConf[4] = (0xff00 & ScanStartVal) >> 8;
//	//ScanConf[5] = (0xff & ScanStopVal);
//	//ScanConf[6] = (0xff00 & ScanStopVal) >> 8;
//	//ScanConf[7] = (0xff & ScanStepSize);
//	//ScanConf[8] = (0xff00 & ScanStepSize) >> 8;
//	//ScanConf[9] = InjCount;
//	//ScanConf[10] = MaskStepSize;
//	//ScanConf[11] = MaskStepCount;
//	//ScanConf[12] = ShiftMask;
//
//	//// solve predefined parity...
//	//SetGlobalVal(ScanVarIndex, 0);
//	//globalReg[0]->MakeByteArray();
//	//globalReg[0]->UpdateByteArrayItem(globalReg[0]->dataList[GLOBALPARITY].add, globalReg[0]->dataList[GLOBALPARITY].size, globalReg[0]->calculateParity(), globalReg[0]->byteArray); //update array with correct parity
//	//myUSB->WriteExternal(CS_CONFIG_GLOBAL_WRITEMEM, globalReg[0]->byteArray, (COMMAND_REG_SIZE + GLOBAL_REG_SIZE)/8);	// write configuration-data to BlockRAM
//
//	//// start µC...
//	//ResetStatusFlags();
//	//myUSB->WriteCommand(ScanConf, 13);
//	//
//	//myUSB->ReadInterrupt(); // read must be done, because ReadInterrupt reads "last" status... 
//}
//
//bool ConfigFEMemory::StartScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanStepSize, int InjCount, int MaskStepSize, int MaskStepCount, int ShiftMask, bool all_DCs)
//{
//	{	// needed to set this instance of ConfigRegister to correct mode, mode MUST be set before calling this function...
//		unsigned char data = 0x00;
//		unsigned char dataRB;
//		myUSB->ReadXilinx(CS_SYSTEM_CONF, &dataRB, 1);
//		if ((0x01 & dataRB) & !(0x02 & dataRB))	// system is in hit histogramming mode...
//			configReg->SetCalibrationMode();
//		else if (!(0x01 & dataRB) & (0x02 & dataRB))	// system is in ToT histogramming mode...
//			configReg->SetTOTMode();
//		//else if ((0x01 & dataRB) & (0x02 & dataRB))	// system is in TLU histogramming mode...
//			//configReg->SetTLUMode();
//		else
//			configReg->SetRunMode();
//	}
//
//	int add, size, colpr_mode, colpr_addr;
//	GetGlobalVarAddVal(COLPR_MODE, add, size, colpr_mode);
//	GetGlobalVarAddVal(COLPR_ADDR, add, size, colpr_addr);
//
//	configReg->ClearSRAM();
//	//configReg->SetCalibrationMode();
//	configReg->ClearConfData();
//	configReg->ClearTOTHisto();
//
//	int LV1_length = 0;
//	LV1_length = configReg->ReadRegister(CS_L_LV1);
//
//	configReg->WriteRegister(CS_QUANTITY, 1);
//	//configReg->WriteRegister(CS_QUANTITY, InjCount);
//
//	//for (int mask_step = 0; mask_step < MaskStepCount; mask_step++)
//	{
//		unsigned char conf_nr = 0;
//		int ScanVal = ScanStartVal;
//		int read_SRAM_nr = 0;
//		int nr_steps = (ScanStopVal - ScanStartVal + 1) / ScanStepSize;
//		for (int ScanVal = ScanStartVal; ScanVal <= ScanStopVal; ScanVal = ScanVal + ScanStepSize)
//		{
//			myUSB->WriteXilinx(CS_CONFIGURATION_NR, &conf_nr, 1);	// set scan step in FPGA to conf_nr
//
//			// set scan variable to start value
//			SetGlobalVal(ScanVarIndex, ScanVal);					
//			WriteGlobal(IndexToRegisterNumber(ScanVarIndex));
//
//			for (int mask_step = 0; mask_step < MaskStepCount; mask_step++)
//			{
//				if (all_DCs) // ((colpr_mode == 1) | (colpr_mode == 2))) // for debugging one might want to use other settings also???
//				{
//					//scan loop for COLPR_ADDR. Scans every 8th DC at once! Comment for faster scanning and tests!!!
//					for (int col_add = 1; col_add <= 4/*8*//*colpr_mode * 4*/; col_add++)
//					{														
//						SetGlobalVal(COLPR_ADDR, col_add);
//						SetGlobalVal(COLPR_MODE, 1/*2*//*colpr_mode*/);
//						WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));
//
//						// enable data take mode
//						// WriteCommand(FE_EN_DATA_TAKE);
//
//						for (int injection = 0; injection < InjCount; injection++)
//						{   
//							// start injections
//							int received_DH = 0;
//							//reset event counter
//							configReg->WriteRegister(CS_STATUS_REG, received_DH);
//
//							unsigned char reg_data;
//							reg_data = 0x01;
//							myUSB->WriteXilinx(CS_TRIGGER_STRB_LV1, &reg_data, 1);
//							//while (reg_data & myUSB->HandlePresent())
//							//{
//							//	myUSB->ReadXilinx(CS_TRIGGER_STRB_LV1, &reg_data, 1);
//							//}
//							// wait for chip to send data
//							int timeout = 0;
//							while ((received_DH < LV1_length - 2))//!= LV1_length) && (received_DH != (LV1_length - 1)) && (received_DH != (LV1_length + 1)))
//							{
//								received_DH = configReg->ReadRegister(CS_STATUS_REG);
//								timeout++;
//								if (timeout == 100000) // to be adjusted...
//								{
//									//SetGlobalVal(COLPR_ADDR, 0);				// comment this line												// comment this line
//									//SetGlobalVal(COLPR_MODE, 3);						// comment this line
//									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));		// comment this line
//
//									//configReg->SetRunMode();
//
//									//SetGlobalVal(COLPR_MODE, colpr_mode);
//									//SetGlobalVal(COLPR_ADDR, colpr_addr);
//									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));
//
//									break;
//									//return false;
//								}
//							}
//						}
//					}
//
//					SetGlobalVal(COLPR_ADDR, 0);				// comment this line												// comment this line
//					SetGlobalVal(COLPR_MODE, 3);						// comment this line
//					WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));		// comment this line
//				}
//				//}
//				else if (!all_DCs) // all DCs at once
//				{
//					for (int injection = 0; injection < InjCount; injection++)
//					{   
//						// start injections
//						int received_DH = 0;
//						// reset event counter
//						configReg->WriteRegister(CS_STATUS_REG, received_DH);
//
//						unsigned char reg_data;
//						reg_data = 0x01;
//						myUSB->WriteXilinx(CS_TRIGGER_STRB_LV1, &reg_data, 1);
//						//while (reg_data & myUSB->HandlePresent())
//						//{
//						//	myUSB->ReadXilinx(CS_TRIGGER_STRB_LV1, &reg_data, 1);
//						//}
//						// wait for chip to send data
//						int timeout = 0;
//							while ((received_DH < LV1_length - 2))//!= LV1_length) && (received_DH != (LV1_length - 1)) && (received_DH != (LV1_length + 1)))
//							{
//								received_DH = configReg->ReadRegister(CS_STATUS_REG);
//								timeout++;
//								if (timeout == 100000) // to be adjusted...
//								{
//									//SetGlobalVal(COLPR_ADDR, 0);				// comment this line												// comment this line
//									//SetGlobalVal(COLPR_MODE, 3);						// comment this line
//									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));		// comment this line
//
//									//configReg->SetRunMode();
//
//									//SetGlobalVal(COLPR_MODE, colpr_mode);
//									//SetGlobalVal(COLPR_ADDR, colpr_addr);
//									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));
//
//									break;
//									//return false;
//								}
//							}
//					}
//				}
//				else
//					return false;
//
//				//WriteCommand(FE_CONF_MODE);
//
//				if(ShiftMask & 0x01)
//					ShiftPixMask(HITBUS, MaskStepSize);
//				if(ShiftMask & 0x02)
//					ShiftPixMask(CAP0, MaskStepSize);
//				if(ShiftMask & 0x04)
//					ShiftPixMask(CAP1, MaskStepSize);
//				if(ShiftMask & 0x08)
//					ShiftPixMask(ENABLE, MaskStepSize);
//			} // end for masksteps
//
//			// reload Mask after 32 Masksteps...
//			if(ShiftMask & 0x01)
//				WritePixel(HITBUS, 40);
//			if(ShiftMask & 0x02)
//				WritePixel(CAP0, 40);
//			if(ShiftMask & 0x04)
//				WritePixel(CAP1, 40);
//			if(ShiftMask & 0x08)
//				WritePixel(ENABLE, 40);
//
//			// read SRAM if conf_nr [4:0] = 5'b11111
//			if (((0x1f & conf_nr) == 31) | conf_nr == (nr_steps - 1))
//			{
//				configReg->ReadSRAM(read_SRAM_nr);
//				configReg->ClearSRAM();
//				read_SRAM_nr++;
//			}
//			// increment scan step
//			conf_nr++;
//		} // end for scanstep
//	}
//
//	configReg->SetRunMode();
//
//	SetGlobalVal(COLPR_MODE, colpr_mode);
//	SetGlobalVal(COLPR_ADDR, colpr_addr);
//	WriteGlobal(IndexToRegisterNumber(COLPR_MODE));
//
//	return true;
//}
//
//void ConfigFEMemory::GetScanStatus(int& MaskStep, bool& ScanBusy, bool& ScanReady, bool& ScanCanceled, bool& EOEerror)
//{
//	int data = 0x00;
//
//	data = myUSB->ReadInterrupt();
//
//	MaskStep = 0x000000ff & data;
//	ScanBusy = 0x00020000 & data;
//	ScanReady = 0x00040000 & data;
//	ScanCanceled = 0x00080000 & data;
//	EOEerror = 0x00010000 & data;
//}
//
//
//void ConfigFEMemory::ResetStatusFlags()
//{
//	int data = 0x00;
//	myUSB->WriteInterrupt(data);
//}
//
//void ConfigFEMemory::CancelScan()
//{
//	int data = SCAN_CANCELED;
//	myUSB->WriteInterrupt(SCAN_CANCELED);
//}
//
//void ConfigFEMemory::GetConfValue(int col, int row, int step, int &Value)
//{
//	configReg->GetConfValue(col, row, step, Value);
//}

//void ConfigFEMemory::GetTOTHistoValue(int col, int row, int tot, int &Value)
//{
//	configReg->GetTOTHistoValue(col, row, tot, Value);
//}


// Weiterleitung zu configReg für "MakeHisto from words Funktionen und GetHistos...



void ConfigFEMemory::sendBitstreamToAB()
{
	shift_globalConf_AB->FileOpen("debug_SR_AB.dat");
	shift_globalConf_AB->MakeByteArray();
	HandleFECommand(FE_WRITE_GLOBAL_AB, 0, chip_addr, shift_globalConf_AB->byteArray); // write to global register AB
	

	// debugging...
	HandleFECommand(FE_READ_GLOBAL_AB, 0, chip_addr, shift_globalConf_AB->byteArrayRB); // write to global register AB
	shift_globalConf_AB->MakeStructListRB();
	shift_globalConf_AB->FileSaveRB("debug_SR_AB_shiftthrough.dat", 2);

	// to be able to check latching afterwards...
	HandleFECommand(FE_WRITE_GLOBAL_AB, 0, chip_addr, shift_globalConf_AB->byteArray); // write to global register AB
}

void ConfigFEMemory::sendBitstreamToC()
{
	shift_globalConf_C->FileOpen("debug_SR_C.dat");
	shift_globalConf_C->MakeByteArray();
	HandleFECommand(FE_WRITE_GLOBAL_C, 0, chip_addr, shift_globalConf_C->byteArray); // write to global register AB
	

	// debugging...
	HandleFECommand(FE_READ_GLOBAL_C, 0, chip_addr, shift_globalConf_C->byteArrayRB); // write to global register AB
	shift_globalConf_C->MakeStructListRB();
	shift_globalConf_C->FileSaveRB("debug_SR_C_shiftthrough.dat", 2);

	// to be able to check latching afterwards...
	HandleFECommand(FE_WRITE_GLOBAL_C, 0, chip_addr, shift_globalConf_C->byteArray); // write to global register AB
}

void ConfigFEMemory::RunScanChain(int ScanChainSelect, USBPixSTDDCS* dcs, double& current_before, double& current_after, bool shift_only, bool se_while_pulse, bool si_while_pulse)
{
	bool OldCtrlMode = useSlowControl;
	if (!OldCtrlMode)
		SetSlowControlMode();

	// xck should not be running...
	configReg->stopXCK(true);

	int inmux_select_state = configReg->ReadRegister(CS_INMUX_CONTROL);
	int inmux_in_ctrl_state = configReg->ReadRegister(CS_INMUX_IN_CTRL);
	int data = 0x00;

	switch(ScanChainSelect)
	{
	case SC_DOB:
		scanChainDOB->MakeByteArray();	

		// set IoMUX[0] and IoMUX[1] to correct state...
		configReg->WriteRegister(CS_INMUX_IN_CTRL, 0x06);

		// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to SC_DOB state
		{
			data = 0x09;
			configReg->WriteRegister(CS_INMUX_CONTROL, data);
		}

		// send bitstream
		HandleFECommand(FE_WRITE_SC_DOB, 0, chip_addr, scanChainDOB->byteArray);

		//			  Send Clk Pulse and go back
		{
			// First set INMUX to correct state
			int datadebug = se_while_pulse?0x06:0x02;
			if (si_while_pulse)
				datadebug = datadebug + 0x08;
			configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

			datadebug = 0x01;
			configReg->WriteRegister(CS_INMUX_CONTROL, datadebug);

			// Should be clk pulse...
			UPGen::Sleep(100);

			dcs->UpdateMeasurements();
			current_before = fabs(dcs->PSU[FEI4B?VDDA1:VDDD1]->GetCurrent());
			current_before += fabs(dcs->PSU[FEI4B?VDDA2:VDDD2]->GetCurrent());

			UPGen::Sleep(100);

			if (!shift_only)
			{
				datadebug = se_while_pulse?0x07:0x03;
				if (si_while_pulse)
					datadebug = datadebug + 0x08;
				configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);


				UPGen::Sleep(100);

				datadebug = se_while_pulse?0x06:0x02;
				if (si_while_pulse)
					datadebug = datadebug + 0x08;
				configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);
			}

			UPGen::Sleep(100);

			dcs->UpdateMeasurements();
			current_after = fabs(dcs->PSU[FEI4B?VDDA1:VDDD1]->GetCurrent());
			current_after += fabs(dcs->PSU[FEI4B?VDDA2:VDDD2]->GetCurrent());
			datadebug = 0x06;
			configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

			// Set back INMUX control
			// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to SC_DOB state
			datadebug = 0x09;
			configReg->WriteRegister(CS_INMUX_CONTROL, datadebug);
		}


		// read scan chain
		HandleFECommand(FE_READ_SC_DOB, 0, chip_addr, scanChainDOB->byteArrayRB);
		scanChainDOB->MakeStructListRB();
		break;
	case SC_CMD:
		scanChainCMD->MakeByteArray();	

		// set IoMUX[0] and IoMUX[1] to correct state...
		configReg->WriteRegister(CS_INMUX_IN_CTRL, 0x0C);

		// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to SC_CMD state
		{
			data = 0x2D;
			configReg->WriteRegister(CS_INMUX_CONTROL, data);
		}

		// send bitstream
		HandleFECommand(FE_WRITE_SC_CMD, 0, chip_addr, scanChainCMD->byteArray);

		//			  Send Clk Pulse and go back
		{
			// First set INMUX to correct state
			int datadebug = se_while_pulse?0x0C:0x08;
			if (si_while_pulse)
				datadebug = datadebug + 0x01;
			configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

			datadebug = 0x05;
			configReg->WriteRegister(CS_INMUX_CONTROL, datadebug);

			// Should be clk pulse...

			UPGen::Sleep(100);

			dcs->UpdateMeasurements();
			current_before = fabs(dcs->PSU[FEI4B?VDDA1:VDDD1]->GetCurrent());
			current_before += fabs(dcs->PSU[FEI4B?VDDA2:VDDD2]->GetCurrent());
			if (!shift_only)
			{
				datadebug = se_while_pulse?0x0E:0x0A;
				if (si_while_pulse)
					datadebug = datadebug + 0x01;
				configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

				UPGen::Sleep(100);

				datadebug = se_while_pulse?0x0C:0x08;
				if (si_while_pulse)
					datadebug = datadebug + 0x01;
				configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);
			}

			UPGen::Sleep(100);

			dcs->UpdateMeasurements();
			current_after = fabs(dcs->PSU[FEI4B?VDDA1:VDDD1]->GetCurrent());
			current_after += fabs(dcs->PSU[FEI4B?VDDA2:VDDD2]->GetCurrent());
			datadebug = 0x0C;
			configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

			// Set back INMUX control
			// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to SC_CMD state
			datadebug = 0x2D;
			configReg->WriteRegister(CS_INMUX_CONTROL, datadebug);
		}


		// read scan chain
		HandleFECommand(FE_READ_SC_CMD, 0, chip_addr, scanChainCMD->byteArrayRB);
		scanChainCMD->MakeStructListRB();
		break;
	case SC_ECL:
		scanChainECL->MakeByteArray();	

		// set IoMUX[0] and IoMUX[1] to correct state...
		configReg->WriteRegister(CS_INMUX_IN_CTRL, 0x05);

		// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to SC_ECL state
		{
			data = 0x12;
			configReg->WriteRegister(CS_INMUX_CONTROL, data);
		}

		// send bitstream
		HandleFECommand(FE_WRITE_SC_ECL, 0, chip_addr, scanChainECL->byteArray);


		//			  Send Clk Pulse and go back
		{
			// First set INMUX to correct state
			int datadebug = se_while_pulse?0x05:0x01;
			if (si_while_pulse)
				datadebug = datadebug + 0x02;
			configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

			datadebug = 0x02;
			configReg->WriteRegister(CS_INMUX_CONTROL, datadebug);

			// Should be clk pulse...

			UPGen::Sleep(100);

			dcs->UpdateMeasurements();
			current_before = fabs(dcs->PSU[FEI4B?VDDA1:VDDD1]->GetCurrent());
			current_before += fabs(dcs->PSU[FEI4B?VDDA2:VDDD1]->GetCurrent());
			if (!shift_only)
			{
				datadebug = se_while_pulse?0x0D:0x09;
				if (si_while_pulse)
					datadebug = datadebug + 0x02;
				configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

				UPGen::Sleep(100);

				datadebug = se_while_pulse?0x05:0x01;
				if (si_while_pulse)
					datadebug = datadebug + 0x02;
				configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);
			}

			UPGen::Sleep(100);

			dcs->UpdateMeasurements();
			current_after = fabs(dcs->PSU[FEI4B?VDDA1:VDDD1]->GetCurrent());
			current_after += fabs(dcs->PSU[FEI4B?VDDA2:VDDD2]->GetCurrent());
			datadebug = 0x05;
			configReg->WriteRegister(CS_INMUX_IN_CTRL, datadebug);

			// Set back INMUX control
			// set IOMUXselect AND IOMUXcontrol (FPGA internal MUX ctrl) to SC_ECL state
			datadebug = 0x12;
			configReg->WriteRegister(CS_INMUX_CONTROL, datadebug);
		}	

		// read scan chain
		HandleFECommand(FE_READ_SC_ECL, 0, chip_addr, scanChainECL->byteArrayRB);
		scanChainECL->MakeStructListRB();
		break;
	default:
		break;
	}
	// reset inmux select
	configReg->WriteRegister(CS_INMUX_CONTROL, inmux_select_state);
	configReg->WriteRegister(CS_INMUX_IN_CTRL, inmux_in_ctrl_state);

	configReg->stopXCK(false);

	if (!OldCtrlMode)
		SetCMDMode();
}

void ConfigFEMemory::SetScanChainValue(int BlockSelect, int index, int val)
{
	switch(BlockSelect)
	{
		case SC_DOB:
			scanChainDOB->SetValue(index, val);
			break;
		case SC_CMD:
			scanChainCMD->SetValue(index, val);
			break;
		case SC_ECL:
			scanChainECL->SetValue(index, val);
			break;
		default:
			break;
	}
}

void ConfigFEMemory::LoadFileToScanChain(int ScanChainSelect)
{
	switch(ScanChainSelect)
	{
	case SC_DOB:
		scanChainDOB->FileOpen("ScanChainDOB.dat");
		
		break;
	case SC_CMD:
		scanChainCMD->FileOpen("ScanChainCMD.dat");
		
		break;
	case SC_ECL:
		scanChainECL->FileOpen("ScanChainECL.dat");
		break;
	default:
		break;
	}
}

void ConfigFEMemory::SaveFileFromScanChain(int ScanChainSelect)
{
	switch(ScanChainSelect)
	{
	case SC_DOB:
		scanChainDOB->FileSave("ScanChainDOB.dat", 2);
		break;
	case SC_CMD:
		scanChainCMD->FileSave("ScanChainCMD.dat", 2);
		break;
	case SC_ECL:
		scanChainECL->FileSave("ScanChainECL.dat", 2);
		break;
	default:
		break;
	}
}

void ConfigFEMemory::SaveFileFromScanChainRB(int ScanChainSelect)
{
	switch(ScanChainSelect)
	{
	case SC_DOB:
		scanChainDOB->FileSaveRB("ScanChainDOB_RB.dat", 2);
		break;
	case SC_CMD:
		scanChainCMD->FileSaveRB("ScanChainCMD_RB.dat", 2);
		break;
	case SC_ECL:
		scanChainECL->FileSaveRB("ScanChainECL_RB.dat", 2);
		break;
	default:
		break;
	}
}

void ConfigFEMemory::GetScanChainValue(int BlockSelect, int index, int& size, int& add, int& Value)
{
	switch(BlockSelect)
	{
		case SC_DOB:
			Value = scanChainDOB->GetValue(scanChainDOB->GetRegindexFromIndex(index));
			size = scanChainDOB->dataList[scanChainDOB->GetRegindexFromIndex(index)].size;
			add = scanChainDOB->dataList[scanChainDOB->GetRegindexFromIndex(index)].add;
			break;
		case SC_CMD:
			Value = scanChainCMD->GetValue(scanChainCMD->GetRegindexFromIndex(index));
			size = scanChainCMD->dataList[scanChainCMD->GetRegindexFromIndex(index)].size;
			add = scanChainCMD->dataList[scanChainCMD->GetRegindexFromIndex(index)].add;
			break;
		case SC_ECL:
			Value = scanChainECL->GetValue(scanChainECL->GetRegindexFromIndex(index));
			size = scanChainECL->dataList[scanChainECL->GetRegindexFromIndex(index)].size;
			add = scanChainECL->dataList[scanChainECL->GetRegindexFromIndex(index)].add;
			break;
		default:
			Value = 0;
			size = 0;
			add = 0;
	}
}

void ConfigFEMemory::GetScanChainValueRB(int BlockSelect, int index, int& size, int& add, int& Value)
{
	switch(BlockSelect)
	{
		case SC_DOB:
			Value = scanChainDOB->GetValueRB(scanChainDOB->GetRegindexFromIndex(index));
			size = scanChainDOB->dataList[scanChainDOB->GetRegindexFromIndex(index)].size;
			add = scanChainDOB->dataList[scanChainDOB->GetRegindexFromIndex(index)].add;
			break;
		case SC_CMD:
			Value = scanChainCMD->GetValueRB(scanChainCMD->GetRegindexFromIndex(index));
			size = scanChainCMD->dataList[scanChainCMD->GetRegindexFromIndex(index)].size;
			add = scanChainCMD->dataList[scanChainCMD->GetRegindexFromIndex(index)].add;
			break;
		case SC_ECL:
			Value = scanChainECL->GetValueRB(scanChainECL->GetRegindexFromIndex(index));
			size = scanChainECL->dataList[scanChainECL->GetRegindexFromIndex(index)].size;
			add = scanChainECL->dataList[scanChainECL->GetRegindexFromIndex(index)].add;
			break;
		default:
			Value = 0;
			size = 0;
			add = 0;
	}
}

void ConfigFEMemory::SendReadErrors()
{
	//send global pulse to latch SR-content to memory cells
	SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
	SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
	SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
	SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
	SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
	SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
	if(FEI4B){
	  SetGlobalVal(B_ADC_EN_PULSE, 0);
	  SetGlobalVal(B_SR_RD_EN    , 0);
	  SetGlobalVal(B_REG27SPARES2, 0);
	}
	SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 1);
	SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
	SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
	WriteCommand(FE_GLOBAL_PULSE);

	SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 1);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
}

void ConfigFEMemory::ReadEPROMvalues()
{	
	bool runmode = ChipInRunMode;
	// enable conf mode
	WriteCommand(FE_CONF_MODE);

	// Get old data
	int old_data_RB = configReg->ReadRegister(CS_INMUX_CONTROL);
	int old_INMUX_data = configReg->ReadRegister(CS_INMUX_IN_CTRL);

	// in case of FE-I4A: Need to set INMUXselect to EFUSE mode and pull reset high (active low)
	if (!FEI4B)
	{
		configReg->WriteRegister(CS_INMUX_IN_CTRL, 1);
		int new_data = 0x03;
		configReg->WriteRegister(CS_INMUX_CONTROL, new_data);
	}

	//send global pulse to latch SR-content to memory cells
	SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
	SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
	SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
	SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
	SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
	SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
	if(FEI4B){
		SetGlobalVal(B_ADC_EN_PULSE, 0);
		SetGlobalVal(B_SR_RD_EN    , 0);
		SetGlobalVal(B_REG27SPARES2, 0);
	}
	SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
	SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
	SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 1);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

	// send global pulse
	//slow_command->SetValue(FIELD5, 0);  // TODO: set Pulse Width (set in REGADRESS FIELD) to correct length (here set to 1!!!)
	//slow_command->SetValue(CHIPADDRESS, 8);  // TODO: set Pulse Width (set in REGADRESS FIELD) to correct length (here set to 1!!!)
	//slow_command->MakeByteArray();
	//HandleFECommand(FE_GLOBAL_PULSE, 0, chip_addr, slow_command->byteArray);
	WriteCommand(FE_GLOBAL_PULSE);



	SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
	WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));

	// in case of FE-I4A: reset INMUXselect and INMUX
	if (!FEI4B)
	{
		configReg->WriteRegister(CS_INMUX_IN_CTRL, old_INMUX_data);
		configReg->WriteRegister(CS_INMUX_CONTROL, old_data_RB);
	}

	if (runmode)
		WriteCommand(FE_EN_DATA_TAKE);
}

void ConfigFEMemory::BurnEPROMvalues() // Burns to the EPROM whatever is stored in GR. Note that burning a 1 is non reversibel!
{
	// Get old data
	int old_data_RB = configReg->ReadRegister(CS_INMUX_CONTROL);
	int old_INMUX_data = configReg->ReadRegister(CS_INMUX_IN_CTRL);

	// Set IOMUXselect to EFUSE mode and make sure reset (IOMUX[0]) is inactive (active low in FE-I4A, active high in FE-I4B)
	configReg->WriteRegister(CS_INMUX_IN_CTRL, FEI4B?0:1);
	int new_data = 0x03;
	configReg->WriteRegister(CS_INMUX_CONTROL, new_data);

//	// Toggle EFUSE FSM reset 
//	configReg->WriteRegister(CS_INMUX_IN_CTRL, FEI4B?1:0);

//	UPGen::Sleep(10);

//	configReg->WriteRegister(CS_INMUX_IN_CTRL, FEI4B?0:1);

//	UPGen::Sleep(10);



	// Provide 16 positive edges on INMUX[1], keep high quite long
	for (int i = 0; i < 64; i++)
	{

		UPGen::Sleep(1);

		configReg->WriteRegister(CS_INMUX_IN_CTRL, 2);

		UPGen::Sleep(1);

		configReg->WriteRegister(CS_INMUX_IN_CTRL, 0);
	}


		UPGen::Sleep(10);

	// Toggle EFUSE FSM reset 
	configReg->WriteRegister(CS_INMUX_IN_CTRL, FEI4B?1:0);

		UPGen::Sleep(10);

	configReg->WriteRegister(CS_INMUX_IN_CTRL, FEI4B?0:1);

		UPGen::Sleep(10);


	// Set back old settings
	configReg->WriteRegister(CS_INMUX_CONTROL, old_data_RB);
	configReg->WriteRegister(CS_INMUX_IN_CTRL, old_INMUX_data);
}

bool ConfigFEMemory::ReadGADC(int GADCselect)
{
	// Check if FE-I4B
	if(!FEI4B)
		return false;

	int oldADCselect, add, size;
	// Select what to read
	{
		GetGlobalVarAddVal(B_ADCSELECT, add, size, oldADCselect);
		SetGlobalVal(B_ADCSELECT, GADCselect);
		WriteGlobal(IndexToRegisterNumber(B_ADCSELECT));
	}

//		// Reset GR 27
//	{
//		SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
//		SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
//		SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
//		SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
//		SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
//		SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
//		if(FEI4B){
//			SetGlobalVal(B_ADC_EN_PULSE, 0);
//			SetGlobalVal(B_SR_RD_EN    , 0);
//			SetGlobalVal(B_REG27SPARES2, 0);
//		}
//		SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
//		SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
//		SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
//		WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
//	}
//

//	    UPGen::Sleep(10);


	// Configure GR27 for global pulse definition
	{
		SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
		SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
		SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
		SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
		SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
		SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
		if(FEI4B){
			SetGlobalVal(B_ADC_EN_PULSE, 1);
			SetGlobalVal(B_SR_RD_EN    , 0);
			SetGlobalVal(B_REG27SPARES2, 0);
		}
		SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
		SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
		SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
		WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
	}

	// Send Global Pulse
	WriteCommand(FE_GLOBAL_PULSE, 63);

		// some sleep time needed...

		UPGen::Sleep(100);


	// need to read GR40 here!
		ReadGlobal(IndexToRegisterNumber(B_GADCOUT));

	// Reset GR 27
	{
		SetGlobalVal(FEI4B?B_FE_CLK_PULSE:FE_CLK_PULSE, 0);
		SetGlobalVal(FEI4B?B_LATCH_EN:LATCH_EN, 0);
		SetGlobalVal(FEI4B?B_SR_CLR:SR_CLR, 0);
		SetGlobalVal(FEI4B?B_DIG_INJ:DIG_INJ, 0);
		SetGlobalVal(FEI4B?B_GATEHITOR:GATEHITOR, 0);
		SetGlobalVal(FEI4B?B_REG27SPARE1:RD_SKIPPED, 0);
		if(FEI4B){
			SetGlobalVal(B_ADC_EN_PULSE, 0);
			SetGlobalVal(B_SR_RD_EN    , 0);
			SetGlobalVal(B_REG27SPARES2, 0);
		}
		SetGlobalVal(FEI4B?B_RD_ERRORS:RD_ERRORS, 0);
		SetGlobalVal(FEI4B?B_STOP_CLK:STOP_CLK, 0);
		SetGlobalVal(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE, 0);
		WriteGlobal(IndexToRegisterNumber(FEI4B?B_EFUSE_SENSE:EFUSE_SENSE));
	}

	// Reset ADC select
	{
		SetGlobalVal(B_ADCSELECT, oldADCselect);
		WriteGlobal(IndexToRegisterNumber(B_ADCSELECT));
	}
	return true;
}

bool ConfigFEMemory::dummyRegisterSelected()
{
  if (FEI4B)
  {
    if ((registerNumber >= 1) && (registerNumber <= 42))
      return false;
  }
  else
  {
    if ((registerNumber >= 2) && (registerNumber <= 29))
      return false;
    if ((registerNumber >= 31) && (registerNumber <= 42))
      return false;
  }
  return true;
}
