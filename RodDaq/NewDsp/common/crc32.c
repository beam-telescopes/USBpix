#include <dsp_debug_conf.h>
#if debug_crc32_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(calc_crc32,"xprog");

#define CRC32POLY 0x04C11DB7 
unsigned int  calc_crc32(unsigned int *p,unsigned int size)
{   
 int hbit;
 unsigned int bit;
 unsigned int crc32=0;  
 int i,j;
 for(i=0;i<size;i++) {
   for(j=0;j<sizeof(int)*8;j++) {
     bit=(p[i]>>j)&1;
     hbit = (crc32 & 0x80000000) ? 1 : 0;
     if (hbit != bit)
       crc32 = (crc32 << 1) ^ CRC32POLY;
     else
       crc32 = crc32 << 1;
   }
 }
 return crc32;
}
