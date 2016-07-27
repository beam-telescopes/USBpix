/* Slave/PIX Header */

#ifndef HISTOGRAMCTRL_H
#define HISTOGRAMCTRL_H

#include<histogramOptions.h>

enum {
	SAFE_TOT
};

enum {
	HIST_MODULE_PARAMS_VCAL_COEFFS
};

#define N_VCAL_COEFFS 4

typedef struct {
	int id;
	float coeffs[N_VCAL_COEFFS];
} ModuleHistogrammingParametersVcalCoeffs;

typedef struct {
	int type;
//	int (*init)(void *vHistogram);
//	int (*proc)(void *vHistogram);
//	int (*talk)(void *vHistogram, int topic, UINT32 item);
//	int (*exit)(void *vHistogram);
	int nOptions;
	int nModules;
	int nBins;
	int nTriggers;
	int link[NMODULES_PER_ROD];
	int fFree;
	int fRun;
	int bin;
	int maskStage;
	int kMaskStage;
	int maskStageBegin;
	int nMaskStages;
	int maskStageTotalSteps;
/* the following are for memory-mapping purposes. unfortunately, external memory must be reconfigured
	depending on the type of histogramming / scanning, etc. Also, since external memory is divided into
	two non-contiguous regions, special care must be taken */
	void *pixelHitsData; /* current address where data is stored */
	void *pixelFitsData; /* address where fitting results are stored */
	void *results[2]; /* where to store results for master's retrieval. ping pong scheme */
	int resultsLen; /* length of results area, in words */
	void *parent_task; /* who's your daddy? */
	void *child_task;
	void *status; /* points to a HistogramStatus structure */
	void *layout; /* points to a HistogramLayout structure */
/* ascii name of the module. the actual memory that holds the name must be allocated.
	this is not statically allocated here because this structure resides in precious internal memory
	and we don't need the name to be stored in fast memory */
	//char *name[NMODULES_PER_SLAVE]; 
	int id[NMODULES_PER_ROD]; /* this is the global id of the module */
	HistogramOptions *options; /* used to keep a copy of the input options structure around for later retrieval */
} HistogramCtrl;

void *makeHistogramCtrl(int type, void *vHistogramOptions);

#endif
