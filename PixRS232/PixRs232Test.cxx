#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "PixRs232Device.h"
#ifdef WIN32
#include <windows.h>
#endif

using namespace std;


int main(int argc, char* argv[])
{
  std::string errtxt;

	if(argc!=2){
		printf("USAGE: PixRs232Test [COM PORT NUMBER]\n");
		return -1;
	}

	PixRs232Device::Portids tPortID = PixRs232Device::COM1;

	switch((int)atoi(argv[1])){
		case 1:
			tPortID = PixRs232Device::COM1;
			break;
		case 2:
			tPortID = PixRs232Device::COM2;
			break;
		case 3:
			tPortID = PixRs232Device::COM3;
			break;
		case 4:
			tPortID = PixRs232Device::COM4;
			break;
		default:
			std::cout<<"Only COM port 1..4 supported, abort."<<std::endl;
			return 1;
	}

	PixRs232Device device(tPortID);  // identify 

	if(device.getStatus()==PixRs232Device::COM_ERROR){
		device.getError(errtxt);
		printf("ERROR status: %s\n", errtxt.c_str());
	} else
	  device.printDevice();


//    device.getPosition(0);

//    device.setPosition(0, 110);

//		device.getPosition(0);

//	device.setPowerStatus(PixRs232Device::COM_ON);

//    device.setPosition(0, 1000);
//  device.findReference();

//    device.writeDevice("MN", 0);
//    device.writeDevice("SA100000", 0); // max speed
//		device.writeDevice("SV20000", 0);  

//				device.writeDevice("SV4000", 0);

//		device.writeDevice("LF", 0);
//		device.writeDevice("LH", 0);

//		std::cout<<"device.getSpeed(0);"<<device.getSpeed(0)<<"\n";

	//	std::cout<<"device.getPosition(0);"<<device.getPosition(0)<<"\n";
		
//		device.writeDevice("V1000", 0);
		
//		std::cout<<"device.getPosition(0);"<<device.getPosition(0)<<"\n";


//    device.writeDevice("MA1000", 0);

//  if(argc!=2){
//    printf("USAGE: PixRs232Test [nominal voltage]\n");
//    return -1;
//  }
//
//  PixRs232Device device(PixRs232Device::COM2);
//
//  device.setVoltage(0, (double)atoi(argv[1]));
//  if(device.getStatus()==PixRs232Device::COM_ERROR){
//    device.getError(errtxt);
//    printf("ERROR status: %s\n", errtxt.c_str());
//  }
//  float volts = device.getVoltage(0, false);
//  if(device.getStatus()==PixRs232Device::COM_ERROR){
//    device.getError(errtxt);
//    printf("ERROR status: %s\n", errtxt.c_str());
//  }
//  printf("Voltage on device is now %f\n", volts);
//  float current = device.getCurrent(0, false);
//  if(device.getStatus()==PixRs232Device::COM_ERROR){
//    device.getError(errtxt);
//    printf("ERROR status: %s\n", errtxt.c_str());
//  }
//  printf("Current on device is now %g\nPowering down\n", current);
//  device.setVoltage(0, 0.);
//  if(device.getStatus()==PixRs232Device::COM_ERROR){
//    device.getError(errtxt);
//    printf("ERROR status: %s\n", errtxt.c_str());
//  }
//  volts = device.getVoltage(0, false);
//  if(device.getStatus()==PixRs232Device::COM_ERROR){
//    device.getError(errtxt);
//    printf("ERROR status: %s\n", errtxt.c_str());
//  }
//  printf("Voltage on device is now %f\n", volts);

  return 0;
}
