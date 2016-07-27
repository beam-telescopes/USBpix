#include <dsp_debug_conf.h>
#if debug_lowlevelc_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(copyBlock_c,"iprog");
#pragma CODE_SECTION(dsp_memcpy_c,"iprog");
#pragma CODE_SECTION(dsp_memset_c,"iprog");
#pragma CODE_SECTION(dsp_strlen_c,"iprog");
#pragma CODE_SECTION(clrRegisterBit_c,"iprog");
#pragma CODE_SECTION(readRegister_c,"iprog");
#pragma CODE_SECTION(readRegisterBit_c,"iprog");
#pragma CODE_SECTION(readRegisterField_c,"iprog");
#pragma CODE_SECTION(setRegisterBit_c,"iprog");
#pragma CODE_SECTION(writeRegister_c,"iprog");
#pragma CODE_SECTION(writeRegisterField_c,"iprog");
#pragma CODE_SECTION(zeroBytes_c,"iprog");
#pragma CODE_SECTION(zeroWords_c,"iprog");

#include <dsp_types.h>
#include <lowlevel.h>

#pragma FUNC_CANNOT_INLINE(readRegister_c);
#pragma FUNC_CANNOT_INLINE(readRegisterField_c);
#pragma FUNC_CANNOT_INLINE(writeRegister_c);
#pragma FUNC_CANNOT_INLINE(writeRegisterField_c);
#pragma FUNC_CANNOT_INLINE(setRegisterBit_c);
#pragma FUNC_CANNOT_INLINE(readRegisterBit_c);
#pragma FUNC_CANNOT_INLINE(clrRegisterBit_c);

#ifndef __GNUC__
void writeRegister_c(void volatile *regAddr, UINT32 value) {*((UINT32*) regAddr)=value; }
UINT32 readRegister_c(void volatile *regAddr) {return *((UINT32*) regAddr); }
void writeRegisterField_c(void volatile *regAddr, UINT32 value, int offset, int width) { *((UINT32*) regAddr)= _clr(*((UINT32*) regAddr),offset,offset+width-1)|(value<<offset); }
UINT32 readRegisterField_c(void volatile *regAddr, int offset, int width) { return _extu(*((UINT32*) regAddr),32-(offset+width),32-width); }
void setRegisterBit_c(void volatile *regAddr, int bit) {*((UINT32*) regAddr)=_set(*((UINT32*) regAddr),bit,bit);}
void clrRegisterBit_c(void volatile *regAddr, int bit) {*((UINT32*) regAddr)=_clr(*((UINT32*) regAddr),bit,bit)  ;}
int readRegisterBit_c(void volatile *regAddr, int bit) {return  _extu(*((UINT32*) regAddr),31-bit,31);}
#endif


void *dsp_memcpy_c(void *dst, const void *src, unsigned int nBytes)
{
  register int i;
  int nwords=nBytes/sizeof(int);
  int bytes=nBytes%sizeof(int);
  UINT32 *p; 
  UINT32 *ps; 
  if(nwords) {
#pragma MUST_ITERATE(1);
    for(i=0;i<nwords;i++) ((UINT32*)dst)[i]=((UINT32*)src)[i];
  }
  if(!bytes) return dst;
  p=((UINT32*)dst)+nwords; 
  ps=((UINT32*)src)+nwords; 
#pragma MUST_ITERATE(1,3);
  for(i=0;i<bytes;i++) ((unsigned char*) p)[i]=((unsigned char*)ps)[i];
  return dst;
}

void *dsp_memset_c(void *dst, int value, int nBytes) {
  register int i;
  int nwords=nBytes/sizeof(int);
  int bytes=nBytes%sizeof(int);
  unsigned int value1;
  unsigned int value2;  
  UINT32 *p; 
  value1=value&0xff;
  if(nwords) {
    value2=value1|(value1<<8)|(value1<<16)|(value1<<24);
    #pragma MUST_ITERATE(1);
    for(i=0;i<nwords;i++) ((UINT32*)dst)[i]=value2;
  }
  if(!bytes) return dst;
  p=((UINT32*)dst)+nwords;
#pragma MUST_ITERATE(1,3);
  for(i=0;i<bytes;i++) ((unsigned char*) p)[i]=(unsigned char) value1;
  return dst; 

}
void copyBlock_c(void *dst, void *src, int nWords) {
  register int i;
  if(!nWords) return;
#pragma MUST_ITERATE(1);
  for(i=0;i<nWords;i++) ((UINT32*)dst)[i]=((UINT32*)src)[i];   
}

void zeroWords_c(void *dst,int n) {
  register int i;
  if(!n) return;
#pragma MUST_ITERATE(1);
  for(i=0;i<n;i++) ((UINT32*)dst)[i]=0;  
}

void zeroBytes_c(void *dst,int n) {
  register int i;
  int nwords=n/sizeof(int);
  int bytes=n%sizeof(int);
  UINT32 *p;  
  if(nwords) 
#pragma MUST_ITERATE(1);
    for(i=0;i<nwords;i++) ((UINT32*)dst)[i]=0;
  if(!bytes) return;
  p=((UINT32*)dst)+nwords;
#pragma MUST_ITERATE(1,3);
  for(i=0;i<bytes;i++) ((unsigned char*) p)[i]=0;
  
}

int dsp_strlen_c(char *s)
{
  register int i=0;
  register char *p;
  if(!s) return -1;
  for(p=s;p[i]!=0;i++);
  return i;

}
