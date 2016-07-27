/* Slave/PIX Header */

#ifndef HISTOGRAMOPTIONS_H
#define HISTOGRAMOPTIONS_H

#include"scanOptions_2.h"
#include"moduleConfig.h"
#include"histogramOptions_common.h"

#define MAX_BINS 256

typedef struct {
	UINT32  type;
        UINT32 optionMask;
	int nBins[SCAN_MAXLOOPS];
	int bin[SCAN_MAXLOOPS];
        UINT32 nTriggers;
	UINT32 param[MAX_BINS];
	UINT32 nModules;
	UINT32 maskStageBegin;
	UINT32 nMaskStages;
	UINT32 maskStageTotalSteps;
	UINT32 link[NMODULES_PER_ROD];
	UINT32 id[NMODULES_PER_ROD]; /* this is the global id of the module, not local */
	ModuleCalib calCoeff[NMODULES_PER_SLAVE]; /* conversion to electrons */
	UINT32 fOptimize; /* C or Assembly, if available */
	UINT32 innerParameterType;
	//UINT32 name[NMODULES_PER_SLAVE][SIZEOF_NAME]; /* this provides for ascii name + null-termination */
	UINT32 exotic_options[30]; /* to reserve additional space for some options */
} HistogramOptions, HistogramIn;

typedef struct {
	UINT32 bin;
	UINT32 maskStage;
	UINT32 isrCounter;
	UINT32 options; /* points to a copy of the input structure */
	UINT32 done;
} HistogramStatus;

//AKDEBUG use same notation as for  
/* enum { */
/* 	BIN_OCC_HISTOGRAM, */
/* 	TOT_HISTOGRAM, */
/* 	BOC_SCAN_HI, */
/* 	BIN_BCID_HISTOGRAM */
/* }; */

typedef struct{
	int sto,normalmode,contin_mode,ngroups,nsteps,nhits,pattern,modoffs,nevnts;
	HISTO_PARAM hist_param;
} BS4Slaves;/* parameters transferred from master to slave in BOC_SCAN */


#endif

