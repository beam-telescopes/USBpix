/* Slave/PIX - histogram.h */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "histogramCtrl.h"

/* really we need only 7 modules maximum. however, I use an
	additional to park bad data */
#define BADMODULE NMODULES_PER_SLAVE
#define NCHIPS 16
#define NCOLS 18
#define XCOLS 32
#define NROWS 160
#define XROWS 256
#define NTOTS 256
#define PIXEL_PER_MODULE 46080
#define FE_PER_MODULE 16

#define INNERPARMS 25 /* SLAVE DSPs only have enough memory for around 25 inner parameters for TOT */
#define MASKSTAGES 32

#define HFES (NCHIPS)
#define HROWS ((NROWS / MASKSTAGES) + 2)
#define HCOLS (NCOLS + 1)
#define HMODS (NMODULES_PER_SLAVE + 1)
#define TOT_IBIN_DELTA NCHIPS * HCOLS * HROWS /* bin delta for TOT */
#define TOT_DELTA MAX_TOT_BINS * HMODS * MASKSTAGES * NCHIPS * HCOLS * HROWS

#define FROWS (NROWS / MASKSTAGES)
#define BAD_MODULE NMODULES_PER_SLAVE
#define LUT_ROW_OUT_OF_MASKSTAGE (NROWS / MASKSTAGES)
#define LUT_INVALID_ROW ((NROWS / MASKSTAGES) + 1)

/* PIXEL_HITS_BASE0 is 0xcfd000 long
This is bins * (modules + 1) * maskstages * chip * (col + 1) * (row/maskstages + 2)
= 25 * 8 * 32 * 16 * 19 * 7 = 13619200 = 0xcfd000 */
#define PIXEL_HITS_BASE0 0xA0400000
/* PIXEL_HITS_BASE1 is 0x3b100 long: it stores the results from the occupancy fits */
#define PIXEL_HITS_BASE1 0xB0000000

/* for now TOT histograms will OVERWRITE occupancy histograms */
#define PIXEL_TOT_BASE0 PIXEL_HITS_BASE0 /* hits location; len : 0x6 000 000 */
#define PIXEL_TOT_BASE1 PIXEL_HITS_BASE1 /* fits location; len " 0x5 fa0 000 */

/* maximum number of bins allowed. this goes hand-in-hand with
	the sizeof(BinType) */
#define MAX_BINS 256
#define MAX_TOT_BINS 25
typedef unsigned char BinType; /*binning for occupancy histogram*/
typedef unsigned char OccBinType;
typedef float TotMeanBinType; /*binning for TOT mean histograms */
typedef float TotSigmaBinType; /*binning for TOT sigma histograms */
typedef float BcidMeanBinType;
typedef float BcidSigmaBinType;
typedef float FitBinType;

/* Binning for TOT histograms.  We assume that the maximum number of events is 256,
   the following need to be redefined. */
typedef unsigned char NTot; /* count of TOT */
typedef unsigned short int Tot; /* sum of TOT */
typedef int Tot2; /* sum of TOT^2 */

#define TOT_BIN_SIZE 6 /* in bytes */
#define BCID_BIN_SIZE 6
#define OCC_BIN_SIZE 1
//#define KROWS NROWS/MASKSTAGES
/* used to index within the the PixelHitsData structure */
//#define ICACHE_OFFSET(CHIP, COL, ROW) ((NROWS/histogramCtrl->maskStageTotalSteps) * ((NCOLS * CHIP) + COL) + ROW)
//#define ICACHE_OFFSET(CHIP, COL, ROW) (KROWS * ((NCOLS * CHIP) + COL) + ROW)
//#define ICACHE_OFFSET_I(CHIP, INDEX) ((PIXEL_PER_FE/histogramCtrl->maskStageTotalSteps)*(CHIP)+INDEX)

int connectHistogram(Task *task, HistogramCtrl *histogramCtrl);

#endif 
