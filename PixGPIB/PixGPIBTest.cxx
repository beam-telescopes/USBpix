#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

#include "PixGPIB.h"
#include "PixGPIBDevice.h"

int main(void)
{
	cout << "************************" << endl;
	cout << "* PixGPIBTest      1.0 *" << endl;
	cout << "* daniel.dobos@cern.ch *" << endl;
	cout << "************************" << endl;

	PixGPIB boardController(0);

	//string test = boardController.getDevice(0)->getDeviceType();

// 	PixGPIBDevice* myDevice = boardController.getDevice(0);
// 	myDevice->setState(PGD_ON);
// 	myDevice->measureVoltages();
// 	myDevice->measureCurrents();
	boardController.printDevices();
// 	myDevice->setState(PGD_OFF);

	//  to use PixGPIB::getDevice you have to uncomment this debug function in PixGPIB
//	boardController.getDevice(0)->setFrequency(0, 1000);
//	boardController.getDevice(0)->setVoltage(0, 1);
//	boardController.getDevice(0)->setNburstPulses(0, 3);
//	boardController.getDevice(0)->sendTrigger();
//	boardController.getDevice(0)->setState(PGD_OFF);
}
