#include <stdio.h>
#include "PixDcs/SleepWrapped.h"
#include "SiUSBLib.h"

int main(void){
  InitUSB();
  SiUSBDevice *BoardHandle = new SiUSBDevice(0);
  int i=0;
  while (i<1000000){
    if(OnDeviceChange()){
      printf("Device change detected\n");
    }
    i++;
  }
  delete BoardHandle;
  return 0;
}
