#ifndef SLAVEPRIMXFACE_H
#define SLAVEPRIMXFACE_H

//#include <scanOptions.h>
#include <scanOptions_2.h>
#include <slavePrimXface_common.h>

typedef struct {
  long long hitCount;
  long long eventCount;
  UINT32 data[N_PIXELS];
  UINT32 tot[8*256];
} PixelData;

//** HISTOGRAM SETUP **

typedef struct {
	UINT32 nBins;
	UINT32 binOffset;
	UINT32 moduleOffset;
	UINT32 size;
	UINT32 base;
} HistMemInfo;

typedef struct {
	UINT32 validModules;
	HistMemInfo histMemInfo[SCAN_MAXHIST];
} HistogramLayout;

#endif
