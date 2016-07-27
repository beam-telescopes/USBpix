/* Common Header */

#ifndef SYSPARAMS_H
#define SYSPARAMS_H

#include <sysParams_specific.h>
#include <sysParams_common.h>

/*storage space for occupancy histograms*/
#define HCACHE_LOCATION 0x18000
#define HCACHE_LENGTH 0x28000

#define N_FORMATTERS 8
#define N_TEXTBUFFERS 2
#define N_TEXTBUFFER_SLOTS 5
#define N_LEDS 3
#define N_SLAVES 4
#define N_GROUPS 8
#define N_LINKS 48

/* size of a slave data frame */
#define FRAME_SIZE 256

#define EVTMEM_FIFO_DEPTH 0x7fff
#define INMEM_FIFO_DEPTH 0x7fff

#define FRAMECOPY_LOCATION 0xb00000000
#define FRAMECOPY_MAGIC_ID 0xda7ac081
#define FRAMECOPY_HEADER 0xcc0ffeee
#define FRAMECOPY_TRAILER 0xeeeff0cc

/* should be used by all mechanisms requiring such level of control */
//moved to sysParams_common.h
/* enum { */
/* 	VerboseLevelQuiet = 0, */
/* 	VerboseLevelInfo, */
/* 	VerboseLevelDebug */
/* }; */

#endif
