///////////////////////////////////////////////////////////////////
// PixGPIB.h
//   Header file for class PixGPIB
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////
//  Version 00-00-01 21/011/2008 Daniel Dobos
///////////////////////////////////////////////////////////////////

#ifndef PixGPIB_H
#define PixGPIB_H

#include "PixGPIBDevice.h"

//#include "ni488.h"

class PixGPIB
{

public:
    
  // constructors
  PixGPIB();
  PixGPIB(unsigned int board);

  // destructor 
  ~PixGPIB();
  
  int scanBoard(unsigned int board);
  int printDevices();
  void printGPIBError(char *msg);

  PixGPIBDevice* getDevice(unsigned int pDeviceIndex);//for debugging purpose only
    
private:

  int openBoard(unsigned int board);
  int closeBoard(unsigned int board);

  PixGPIBDevice *m_Devices[281];
  unsigned int m_NumbDevices;

};

#endif //PixGPIB_H
