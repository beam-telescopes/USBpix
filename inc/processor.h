/************************************************************************************
 * processor.h
 *
 *  synopsis: Sets some parameters and typedefs which may be processor dependant.  The
 *           processor type should be defined globally, e.g. via a -d or -D compiler
 *           option.
 *
 *  Damon Fasching, UW Madison (510)486-5230               fasching@wisconsin.cern.ch
 ************************************************************************************/
#ifndef PROCESSOR_SET
#define PROCESSOR_SET

#if (defined(I_AM_NT_HOST) || defined(I_AM_LINUX_HOST))
   #define I_AM_HOST
#endif

/* macros */
#ifndef SIZEOF_MACRO_SET
#define SIZEOF_MACRO_SET
#define SIZEOF(x) (sizeof(x) >> 2) /* sizeof is in bytes, SIZEOF is in 32 bit words */
#endif

/* typedef the fundamental data types.
 * some of these are already defined in TI's stdinc.h, new with CCS v1.20 */
#ifndef _STDINC_H_

/* VME host data types */
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
#endif

#ifdef I_AM_SLAVE_DSP                       /* slave DSP (TMS320C6701) data types */
	typedef char           INT8;
	typedef short          INT16;
	typedef int            INT32;
	typedef long           INT40;
	typedef unsigned char  UINT8;
	typedef unsigned short UINT16;
	typedef unsigned int   UINT32;
	typedef unsigned long  UINT40;
	typedef float          FLOAT32;
	typedef double         FLOAT64;

	typedef  int            int32;
	typedef  unsigned int   uint32;
	typedef  unsigned short uint16;
	typedef  unsigned char  uint8;
#endif

#ifdef I_AM_MASTER_DSP                      /* master DSP (TMS320C6201) data types */
	typedef char           INT8;
	typedef short          INT16;
	typedef int            INT32;
	typedef long           INT40;
	typedef unsigned char  UINT8;
	typedef unsigned short UINT16;
	typedef unsigned int   UINT32;
	typedef unsigned long  UINT40;
	typedef float          FLOAT32;
	typedef double         FLOAT64;

	typedef  int            int32;
	typedef  unsigned int   uint32;
	typedef  unsigned short uint16;
	typedef  unsigned char  uint8;
#endif

/* other variables seemed to have escaped stdinc.h... */
#else
	#if (!defined(I_AM_HOST))
		typedef float          FLOAT32;
		typedef double         FLOAT64;
	#endif
#endif  /* stdinc.h*/

#if   defined(SCT_ROD)
	typedef FLOAT32        MDAT32;
#elif defined(PIXEL_ROD)
	typedef UINT32         MDAT32;
#endif



#endif  /* PROCESSOR_SET */
