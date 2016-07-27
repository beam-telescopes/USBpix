/* Slave/PIX Header */

#ifndef HISTOGRAMOPTIONS_COMMON_H
#define HISTOGRAMOPTIONS_COMMON_H

#define MAX_BINS 256

//AKDEBUG use same notation as for  
/* enum { */
/* 	BIN_OCC_HISTOGRAM, */
/* 	TOT_HISTOGRAM, */
/* 	BOC_SCAN_HI, */
/* 	BIN_BCID_HISTOGRAM */
/* }; */

enum {
	HIST_SET_BIN,
	HIST_FLUSH_BIN,
	HIST_SET_MASK_STAGE,
	HIST_PAUSE,
	HIST_GO,
	HIST_MODULE_PARAMS,
	HIST_START_FIT,
	HIST_FIT,
	HIST_QUERY_LAYOUT,
	HIST_CLOSE,
	HIST_LAST_COMMAND
}; /* options for HISTOGRAM_TASK */

typedef struct{
	int loop0_min,loop0_max,loop0_nsteps,loop1_min,loop1_max,loop1_nsteps,loop2_step_id, itmp;
} HISTO_PARAM;/* histogram parameters for BOC_SCAN and other 2D scans with variable parameters */


#endif

