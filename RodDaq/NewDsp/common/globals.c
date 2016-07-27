#include <dsp_debug_conf.h>
#if debug_globals_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
#pragma DATA_SECTION(dataAcqEventCntr,"idata");
/* Functions declared: */

/*! \file Common - globals.c */

#include <globals.h>

int dataAcqEventCntr;
