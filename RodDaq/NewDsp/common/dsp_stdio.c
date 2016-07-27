#include <dsp_debug_conf.h>
#if debug_dsp_stdio_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */

#pragma DATA_SECTION(formattedString,"xdata");
#pragma DATA_SECTION(fxnStdout,"xdata");
#pragma DATA_SECTION(printfIndexMax,"xdata");
#pragma DATA_SECTION(printfString,"xdata");
/* Functions declared: */
#pragma CODE_SECTION(connectPrintf,"xprog");
#pragma CODE_SECTION(dsp_printf,"xprog");
#pragma CODE_SECTION(print,"xprog");


/*! \file Common - stdio.c */
#ifdef SLAVE_SIM
#include <stdio.h>
#endif
#include <dspGlobal.h>
#include <dsp_stdio.h>
#include <dsp_types.h>
#include <stdio.h>
#include <stdarg.h>


static char formattedString[4096];
static char *printfString;
static int printfIndexMax;
static void (*fxnStdout)(int level, char *str);

/*!
\brief Connect to stdout \n

*/
void connectPrintf(int strdim, char *str, void *fxn) {
	unsigned int *p;
	p = (unsigned int *)&fxnStdout;
	*p = (unsigned int)fxn;
	printfString = str;
	printfIndexMax = strdim;
}
/*!
\brief Formatted print to stdout \n

*/


#ifdef DISABLE_DSP_PRINTF
void _dsp_printf_(char *format, ...) 
#else
void dsp_printf(char *format, ...)
#endif 
{
  va_list args;
  va_start (args,format);
  vsnprintf(formattedString,sizeof(formattedString),format,args);
  va_end(args);
#ifdef SLAVE_SIM
        printf("dsp_printf:  ");printf(formattedString);
#endif
	fxnStdout(0, formattedString); // always sends out as level 0 
}


/*!
\brief Unformatted print to stdout \n

*/
#ifdef DISABLE_PRINT
void _print_(int level, char *format, ... )
#else
void print(int level, char *format, ... ) 
#endif
{
  va_list args;
  va_start (args,format);
  vsnprintf(formattedString,sizeof(formattedString),format,args);
  va_end(args);
  
#ifdef SLAVE_SIM
	printf("print:  ");printf(formattedString);
#endif	
	fxnStdout(level, formattedString); // always sends out as level 0 
}
	

