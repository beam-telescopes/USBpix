/* Common Header */

#ifndef _STDIO_
#define _STDIO_

#ifdef SLAVE
#define DSPSTR "SLAVE"
#endif
#ifdef MASTER
#define  DSPSTR "MASTER"
#endif

#define LOCATIONSTR "[file %s, line %d]"
#define LOCATIONARG __FILE__,__LINE__
//For example, one can do:  dsp_printf(LOCATIONSTR ": WARNING! varname=%d\n", LOCATIONARG, varname);

/* system call */
void connectPrintf(int strdim, char *p, void *fxn);
//#define DISABLE_DSP_PRINTF
//#define DISABLE_PRINT

#ifdef DISABLE_DSP_PRINTF
inline dsp_printf(char *format, ... ) {}
void _dsp_printf_(char *format,...);
#else
void dsp_printf(char *format, ... );
#endif
#ifdef DISABLE_PRINT
inline void print(int level, char *format, ... ) {}
void _print_(int level,char *format, ...);
#else
void print(int level, char *format, ... );
#endif


#endif
