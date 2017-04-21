#ifndef REGISTERTEST_H
#define REGISTERTEST_H

#include "ConfigFEMemory.h"
#include "SiLibUSB.h"

class RegisterTest
{

public:
	RegisterTest(SiUSBDevice * handle);
	~RegisterTest();

public:
	bool TestGlobalReg(int test_nr);
	bool TestPixelReg(int test_nr);

private:
	bool DoGlobalTest();
	bool DoPixelTest();

	ConfigFEMemory * configFEMem;
};



#endif
