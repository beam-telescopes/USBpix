#include <dsp_debug_conf.h>
#if debug_lowlevel_sim_c == 1
#define DEBUG_THIS_FILE
#endif
#include <dsp_types.h>
#include <string.h>
#include <stdio.h>
#include <lowlevel.h>
void copyBlock_c(void *dst, void *src, int nWords){
  int i;
  for(i=0;i<nWords;i++)
  ((int*)dst)[i]=((int*)src)[i];
}

void *dsp_memcpy_c(void *dst, const void *src, unsigned int nBytes)
{
  return memcpy(dst,src,nBytes);

}

int dsp_strlen_c(char *s) {return strlen(s);}

void *dsp_memset_c(void *dst, int value, int bytes)
{
  return memset(dst,value,bytes);
}



void writeRegister_c(void volatile *regAddr, UINT32 value){}
UINT32 readRegister_c(void volatile *regAddr){return 0;}

void writeRegisterField_c(void volatile *regAddr, UINT32 value, int offset, int width){}
UINT32 readRegisterField_c(void volatile *regAddr, int offset, int width){return 0;}
void ierAddMask(int bit){return;}
void csrAddMask(int bit){return;}
UINT32 ierGetMask(){return 0;}
void enableInterrupts(void){}
void disableInterrupts(void){}
void clearInterrupts(void){}
void setRegisterBit_c(void volatile *regAddr, int bit){}
void clrRegisterBit_c(void volatile *regAddr, int bit){}
int readRegisterBit_c(void volatile *regAddr, int bit){return 0;}
void invokeInterrupt(int irq){}
void readInmems(void *buffer, int length){}
void readInmemAAux(void *buffer, int length){}
void ierClr(void){}
void readInmemBAux(void *buffer, int length){}
void zeroWords_c(void *buff, int nWords){
  int i;
  for(i=0;i<nWords;i++) ((int*)buff)[i]=0;
}
void zeroBytes_c(void *buff, int nBytes){
  int i;
  for(i=0;i<nBytes;i++) ((char*)buff)[i]=(char) 0;

}
void echoRegister(char *title, void *regAddr){}
void readBirthday(void){}

