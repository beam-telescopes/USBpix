/* Common Header */

#ifndef _LOWLEVEL_
#define _LOWLEVEL_

/* choice of c or asm implementation */
/* change suffix from _asm to _c or vice versa */
/* Makefile.wine needs lowlevelc.c added to list of object files when c is chosen */
#define dsp_memcpy dsp_memcpy_c
#define dsp_memset dsp_memset_c
#define copyBlock copyBlock_c
#define writeRegister writeRegister_c
#define readRegister readRegister_c
#define readRegisterField readRegisterField_c
#define writeRegisterField writeRegisterField_c 
#define setRegisterBit setRegisterBit_c
#define clrRegisterBit clrRegisterBit_c
#define readRegisterBit readRegisterBit_c
#define dsp_strlen dsp_strlen_c
#define zeroBytes zeroBytes_c
#define zeroWords zeroWords_c

void *dsp_memcpy_asm(void *dst, const void *src, unsigned int nBytes);
void dsp_memset_asm(void *dst, int value, int nBytes);
void copyBlock_asm(void *dst, void *src, int nWords);
void writeRegister_asm(void *regAddr, UINT32 value);
UINT32 readRegister_asm(void *regAddr);
void writeRegisterField_asm(void *regAddr, UINT32 value, int offset, int width);
UINT32 readRegisterField_asm(void *regAddr, int offset, int width);
void setRegisterBit_asm(void *regAddr, int bit);
void clrRegisterBit_asm(void *regAddr, int bit);
int readRegisterBit_asm(void *regAddr, int bit);
int dsp_strlen_asm(char *);
void zeroBytes_asm(void *,int);
void zeroWords_asm(void *,int);

void *dsp_memcpy_c(void *dst, const void *src, unsigned int nBytes);
void *dsp_memset_c(void *dst, int value, int nBytes);
void copyBlock_c(void *dst, void *src, int nWords);
void zeroWords_c(void*,int);

void writeRegister_c(void volatile *regAddr,UINT32 value);
UINT32 readRegister_c(void volatile *regAddr);
void writeRegisterField_c(void volatile *regAddr, UINT32 value, int offset, int width);
UINT32 readRegisterField_c(void volatile *regAddr, int offset, int width);
void setRegisterBit_c(void volatile *regAddr, int bit);
void clrRegisterBit_c(void volatile *regAddr, int bit);
int readRegisterBit_c(void volatile *regAddr, int bit);

int dsp_strlen_c(char *);
void zeroBytes_asm(void *,int);
void zeroWords_asm(void *,int);
void ierAddMask(int bit);
void csrAddMask(int bit);
UINT32 ierGetMask();
void enableInterrupts(void);
void disableInterrupts(void);
void invokeInterrupt(int irq);
void readInmems(void *buffer, int length);
void readInmemAAux(void *buffer, int length);
void readInmemBAux(void *buffer, int length);
void ierClr(void);
void ierClrMask(int bit);
void clearInterrupts();
#define NMIE 1

#define DMA_INT0 8
#define DMA_INT1 9

#endif
