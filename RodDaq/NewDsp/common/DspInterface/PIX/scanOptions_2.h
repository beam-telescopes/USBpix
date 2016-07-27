/* Common Header */

#ifndef SCAN_OPTIONS_2_H
#define SCAN_OPTIONS_2_H

//#include "sysParams.h"
#include "dsp_types.h"
#include "scanEnums.h"
#include "sysParams.h"

typedef struct {
  UINT32 optionMask;
  enum BinSize binSize;
} HistoOptions;


#define MAXGROUPS 8
typedef struct {
  UINT16 groupActionMask[MAXGROUPS];
  UINT8  superGroupAMask;   
  UINT8  superGroupBMask;
  UINT16 spare;   
} GroupOptions;



typedef struct {
  enum TriggMode triggerMode;
  UINT32 nEvents;
  UINT8  nL1AperEvent;
  UINT8  Lvl1_Latency;
  UINT16 strobeDuration;
  UINT32 strobeMCCDelay;    
  UINT32 strobeMCCDelayRange;    
  UINT32 CalL1ADelay;
  UINT32 eventInterval;
  UINT32 superGroupDelay;
  UINT8  superGroupAnL1A[2];
  UINT8  superGroupBnL1A[2];
  UINT16 trigABDelay[2];
  UINT32 vcal_charge;
  UINT32 optionsMask; /* this is TriggOptions */
  UINT32 customTriggerSequence[10];
} TriggerOptions;

typedef struct {
  /* global register parameters which need setting: */
  UINT8  phi;                 /* Column readout frequency */
  UINT8  totThresholdMode;    /* Sets TOT threshold mode */
  UINT8  totMinimum;          /* TOT minimum */
  UINT8  totTwalk;            /* TOT double-hit for digital timewalk correction */
  UINT8  totLeMode;           /* TOT or timestamp leading edge option */
  UINT8  hitbus;
  
} PixelScanFE; /* FE specific options during scans */



typedef struct {
  enum ScanAct Action;
  enum FitFunc fitFunction;
  UINT32 targetThreshold;
} ActionOptions;



typedef struct {
  enum ScanParam     scanParameter;
  ActionOptions endofLoopAction; 
  UINT32        nPoints;
  UINT32        dataPointsPtr;
} ScanLoop;




typedef struct {
	UINT32 maskStage;
	UINT32 kMaskStage;
	UINT32 nMaskStages;
	UINT32 loop0Parameter;
	UINT32 nLoop0Parameters;	
	UINT32 loop1Parameter;
	UINT32 nLoop1Parameters;	
	UINT32 loop2Parameter;
	UINT32 nLoop2Parameters;	
	UINT32 scanType;
	UINT32 options; /* points to a copy of the input structure */
	UINT32 activeModules;
	UINT32 progress;
	UINT32 state;	
	UINT32 task[N_SLAVES]; /* contains the (histogramming) task for each of the slaves */
} ScanStatus;

/* Main Scan interface */
#define SCAN_MAXLOOPS 3

typedef struct {
  enum MaskStageMode  stagingMode;
  enum MaskStageSteps maskStageTotalSteps;
  UINT16              nMaskStages;
  int		      maskLoop; 
  UINT16              FEbyFEMask;
  UINT8               spare;
  UINT8               nLoops;
  GroupOptions        groupOpt;
  TriggerOptions      trigOpt;
  PixelScanFE         feOpt;
  ScanLoop            scanLoop[SCAN_MAXLOOPS];
  HistoOptions        histOpt[SCAN_MAXHIST];
} ScanOptions;
 

#endif
