//#ifndef REGISTERTEST
//#define REGISTERTEST

#ifdef __VISUALC__
#include "stdafx.h"
#endif
#include "RegisterTest.h"
#include "defines.h"



RegisterTest::RegisterTest(SiUSBDevice * handle)
{
	configFEMem = 0;
	configFEMem = new ConfigFEMemory(31, 0, handle);
}

RegisterTest::~RegisterTest()
{
	delete configFEMem;
}



bool RegisterTest::TestPixelReg(int test_nr)
{
	// Set Pixel Values alternating
	int new_value = 0;
	if (test_nr & 0x01)
		new_value = 0xfffefffe;
	else
		new_value = 0xfffefffe;

	for (int latch = TDAC0; latch <= CAP1; latch++)
	{
		for (int var = PIXEL672; var<=PIXEL32; var++)
		{
			configFEMem->SetPixelVal(var, new_value, latch);
		}
	}//end set pixel values

	return DoPixelTest();
}



bool RegisterTest::DoPixelTest()
{
	configFEMem->WritePixel();
	configFEMem->ReadPixel();

	for (int latch = TDAC0; latch <= CAP1; latch++)
	{
		for (int var = PIXEL672; var<=PIXEL32; var++)
		{
			int add;
			int size;
			int val_wr;
			int val_rb;
			configFEMem->GetPixelVarAddVal(var, add, size, val_wr, latch);
			configFEMem->GetPixelRBVarAddVal(var, add, size, val_rb, latch);

			if (val_wr != val_rb)
				return false;
		}//for(pix)
	}//for(latch)
	return true;
}

//#endif
