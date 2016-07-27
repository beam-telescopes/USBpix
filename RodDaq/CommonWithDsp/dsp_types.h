/* Common Header */

#ifndef _DSP_TYPES_
#define _DSP_TYPES_

#if (defined(I_AM_NT_HOST) || defined(I_AM_LINUX_HOST))
   #define I_AM_HOST
#endif

#if (defined(I_AM_HOST))
typedef short          INT16;
typedef int            INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef float          FLOAT32;
typedef double         FLOAT64;

typedef  int            int32;
typedef  unsigned int   uint32;
typedef  unsigned short uint16;
typedef  unsigned char  uint8;
#else
typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned char UINT8;
typedef char INT8;
typedef unsigned char BYTE;
typedef unsigned short int USHORT;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned long int ULONG;
typedef long int LONG;
typedef float FLOAT32;
typedef float MDAT32;
#endif

#define DSP_SUCCESS 0

#ifndef SIZEOF_MACRO_SET
#define SIZEOF_MACRO_SET
#define SIZEOF(x) (sizeof(x) >> 2) /* sizeof is in bytes, SIZEOF is in 32 bit words */
#endif
#define DIMENSION(X) sizeof(X)/sizeof(X[0])

#endif
