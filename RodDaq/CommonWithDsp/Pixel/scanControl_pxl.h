/************************************************************************************
 * Title:   scanControl_pxl.h
 * Version: 5th December 2003
 *
 * Description:   ROD Master DSP scan configuration structure and constant definitions.
 *                See sctScanStructures.h, & pixelScanStructures.h for scan parameter
 *                definitions and scan structures. 
 ************************************************************************************/

/* External setup flags from ScanRODSetup: */
#define EXT_SETUP_NONE    0
#define EXT_SETUP_RCF     1 
#define EXT_SETUP_ROUTER  2
#define EXT_SETUP_SET     4
#define EXT_SETUP_HISTO   8 
#define EXT_SETUP_HTASK   16 

/* Scan control structures.  For Pixels, the dual parameters in ScanGeneral describe
   the inner and outer loops ([0] & [1] respectively) for a two dimensional scan. The
   groupRangeMap parameter is currently unused; all module groups use the same
   mapping. */


//For the 1.1 release, use the original scan structure:

#ifndef COMMENTED

typedef struct  {
	UINT8  configSet;         /* The configuration set to use (INIT/WORK/SPARE) */
	UINT8  reportErrors;      /* Toggle to enable data integrity checks on SDSPs */         
	UINT8  globalCtrl;        /* Determines whether MDSP or TIM sends triggers */         
	UINT8  moduleScanMode;    /* Concurrent or FE by FE */

	UINT8  scanParameter[2];  /* Scan parameter specifier */
	UINT8  uniformPoints[2];     /* If TRUE scanStart, scanEnd & nBins determine points */

	MDAT32 scanStart[2];      /* Start value for uniform scan */
	MDAT32 scanEnd[2];        /* End value for uniform scan */

		UINT16 nBins[2];          /* # of bins in inner & outer loop */
	UINT32 repetitions;       /* # of events per bin */
	MDAT32 *dataPtr[2];       /* pointers to data for custom scans; 0xffffffff
	                             (DEFAULT) indicates that data is in the primitive list
	                             right after the scan structure. */

	UINT8  maskMode;          /* Staged or static */
	UINT8  stageAdvanceFirst; /* Indicates whether the innermost loop is the mask stage
	                             or bin[0] parameter */
	UINT16 maskStages;        /* Number of mask stages to actually execute in scan */

	UINT8  maskStageMode;     /* Mask staging option i.e. which ctrl bits are staged */
	UINT8  maskStagePattern;  /* Mask stage pattern option e.g. 32-stage, 160-stage etc. */
	UINT16 unused2;

	UINT8  unused3;
	UINT8  currentChip;       /* Current chip being scanned if FE-by-FE mode */
	UINT16 currentMaskStage;  /* Current mask stage */    
	UINT16 currentBin[2];     /* Current scanning point */
	UINT32 unused4;           

} ScanGeneral; /* General scan parameters */

typedef struct {
	UINT32 *base;    /* Base of histograms in SDSP memory; 0xffffffff=> default */
	UINT8  opt[4];   /* histogramming options: [0] TRUE => do occupancy histo.
	                                           [1] TRUE => do time-slice spectrum
	                                           [2] bit 0 set: do TOT spectrum,
	                                               bit 3 set: do TOT mean calculation
	                                           [3] will be filled in by MDSP (indicates
	                                               #evt./L1A for SDSP). */
	UINT8 extSetup;  /* Indicates whether MDSP will set up the router & SDSPs or not */
	UINT8 errors;    /* Toggles histogramming of errors  (error task) */
		UINT8 errorsMCC; /* Toggles histogramming of MCC errors (error task) */
		UINT8 unused;

} ScanRODSetup; /* Histogramming options */


typedef struct {
	UINT8 definedGroups;    /* bitfield indicating which groups are included in scan */
	UINT8 nDspPairs;        /* The # of SDSP pairs. A DSP pair use complementary SP
	                            and receive their trigger streams simultaneously; used
	                            for interference/ cross-rtalk checks. */
	UINT8 slvBits;          /* Indicates which SDSPs participate in scan. */
	UINT8 unused0;

	UINT8 dspPair[2][2];   /* The DSP pairs. */
		UINT8 unused[2];
	UINT8 groupSPMap[2];    /* Bitfield indicating which serial port groups use. */
	UINT8 groupDSPMap[4];   /* Bitfields indicating to which SDSP groups send events.*/

} ScanDspDistribution; /* Histogramming options */

typedef struct {
	struct  CmdList triggerSequence[2];
		UINT8 calcFromStruct; /* Pixel: indicates that the serial stream (cal. pulse/
		                         delay/L1A) should be calculated from the scan control
		                         structure. */
		UINT8 port;           /* Serial ports to use: 0, 1 or both (2) */
		UINT8 unused[2];
} ScanSerialData;



typedef struct {
	/* registers on the MCC which need setting: */
	UINT16 duration;             /* Length of strobe in BCO units */
	UINT8 delay;                 /* Strobe delay setting on MCC */
	UINT8 delayRange;            /* Strobe delay range on MCC */          

} PixelScanStrobe; /* Strobe conditioning */				     
							    
typedef struct {
	/* registers on the MCC which need setting: */
	UINT8 accepts;               /* Number of contiguous triggers in event (1-16) */
	UINT8 self;                  /* Selects module self-triggering mode */       
	UINT8 latency;               /* 8-bit trigger latency as programmed on FEs */
	UINT8 calL1ADelay;           /* sets the delay (in BCO units) in the serial stream
	                                between the calibration command and the L1A */
} PixelScanTrigger;	/* Trigger conditioning */

typedef struct {
	/* options for resetting modules (beginning of scan & between bins) */
	UINT8 binReset;
	UINT8 softFE;
	UINT8 shortSync;
	UINT8 mediumSync;
	UINT8 longSync;
	UINT8 ECR;
	UINT8 BCR;
	UINT8 moduleInit;

} PixelScanReset; /* Start of scan reset options */	

typedef struct {
	/* global register parameters which need setting: */
	UINT16 vCal;
	UINT8  cInject;             /* Injection capacitor toggle (c-lo/c-high) */
	UINT8  phi;                 /* Column readout frequency */
	UINT8  totThresholdMode;    /* Sets TOT threshold mode */
	UINT8  totMinimum;          /* TOT minimum */
	UINT8  totTwalk;            /* TOT double-hit for digital timewalk correction */
	UINT8  totLeMode;           /* TOT or timestamp leading edge option */
	UINT8  hitbus;          
	UINT8  digitalInject;
	UINT8  unused[2];
      
} PixelScanFE; /* FE specific options during scans */	

typedef struct {
	UINT8 checkRegisters;     /* Enable periodic checking of MCC warning registers */
    UINT8 bandwidth;
	UINT8 unused[2];
	/* registers on MCC which need setting: */
	UINT16 enableFEContinuityCheck;        /* Bit field */
	UINT16 enableFECrosscheck;             /* Bit field */       

} PixelScanMCC; /* MCC options for scans */

typedef struct {
	UINT8   sCurves;            /* Fit s-curves to the occupancy histos */
	UINT8   totCalibration;     /* Fit TOT calibration functions to TOT data */
	UINT8   tdacTune;           /* Perform TDAC determination based on fit results */
	UINT8   tdacAlgorithm;      /* Method for determining TDAC tune */
	UINT8   fdacTune;           /* Perform FDAC tune based on data from scan */
	UINT8   calibrationOption;  /* Standard time, standard charge or custom */
	UINT8   unused[2];
	
	FLOAT32 chi2Cut;
	FLOAT32 customScale;
	FLOAT32 customOffset;

} PixelScanFitting; /* Fitting options */	



	typedef struct {
		ScanGeneral          general;
	PixelScanStrobe      strobe;
	PixelScanTrigger     trigger;
	PixelScanReset       reset;
	PixelScanFE          fe;
	PixelScanMCC         mcc;
	PixelScanFitting     fitting;
		ScanRODSetup         rodSetup;
		ScanDspDistribution  dspDist;
		ScanSerialData       serial;
	
	} ScanControl;


typedef struct {
	UINT8  unused;
	UINT8  currentChip;       /* Current chip being scanned if FE-by-FE mode */
	UINT16 currentMaskStage;  /* Current mask stage */    
	UINT16 currentBin[2];     /* Current scanning point */
	UINT32 currentDelay;           
} ScanStatus;

//For 1.2 release:
#else

typedef struct  {
	UINT8  configSet;         /* The configuration set to use (INIT/WORK/SPARE) */
	UINT8  globalCtrl;        /* Determines whether MDSP or TIM sends triggers */         
	UINT8  moduleScanMode;    /* Concurrent or FE by FE */
	UINT8  unused;          

	UINT8  scanParameter[2];  /* Scan parameter specifier */
	UINT8  uniformPoints[2];     /* If TRUE scanStart, scanEnd & nBins determine points */

	MDAT32 scanStart[2];      /* Start value for uniform scan */
	MDAT32 scanEnd[2];        /* End value for uniform scan */

	UINT16 nBins[2];          /* # of bins in inner & outer loop */
	UINT32 repetitions;       /* # of events per bin */
	MDAT32 *dataPtr[2];       /* pointers to data for custom scans; 0xffffffff
	                             (DEFAULT) indicates that data is in the primitive list
	                             right after the scan structure. */

	UINT8  maskMode;          /* Staged or static */
	UINT8  stageAdvanceFirst; /* Indicates whether the innermost loop is the mask stage
	                             or bin[0] parameter */
	UINT16 maskStages;        /* Number of mask stages to actually execute in scan */

	UINT16 stage0;            /* 1st mask stage in scan */
	UINT8  maskStageMode;     /* Mask staging option i.e. which ctrl bits are staged */
	UINT8  maskStagePattern;  /* Mask stage pattern option e.g. 32-stage, 160-stage etc. */
} ScanGeneral; /* General scan parameters */

typedef struct {
	UINT32 *base;    /* Base of histograms in SDSP memory; 0xffffffff=> default */
	UINT8  opt[4];   /* options: [0] TRUE => do occupancy histo.
	                             [1] TRUE => do time-slice spectrum
	                             [2] bit 0 set: do TOT spectrum,
	                                 bit 4 set: do TOT mean calculation
	                             [3] will be filled in by MDSP (indicates
	                                 #evt./L1A for SDSP). */
	UINT8 binSize;   /* Occupancy histogram bin size; other sizes are based on this. */
	UINT8 extSetup;  /* Indicates whether MDSP will set up the router & SDSPs or not */
	UINT8 errors;    /* Toggles histogramming of errors  (error task) */
	UINT8 errorsMCC; /* Toggles histogramming of MCC errors (error task) */
} ScanRODSetup; /* Histogramming options */

typedef struct {
	UINT8 definedGroups;    /* bitfield indicating which groups are included in scan */
	UINT8 nDspPairs;        /* The # of SDSP pairs. A DSP pair use complementary SP
	                            and receive their trigger streams simultaneously; used
	                            for interference/ cross-rtalk checks. */
	UINT8 slvBits;          /* Indicates which SDSPs participate in scan. */
	UINT8 mirroredRanges;   /* If set, range map[1] will be a mirror image of map[0]
	                           (given by the bin[0] range); for 2-dimensional histograms
	   this is the only way to have dual ranges. If not set, then for 1-D histograms,
	   if nBins[1] == nBins[0] & both are read in as lists (not calculated), the lists
	   give the customized rangeMaps. */

	UINT8 dspPair[2][2];   /* The DSP pairs. */
	UINT8 groupRangeMap[2]; /* Bitfield indicating which variable mapping groups use. */
	UINT8 groupSPMap[2];    /* Bitfield indicating which serial port groups use. */
	UINT8 groupDSPMap[4];   /* Bitfields indicating to which SDSP groups send events.*/
} ScanDspDistribution; /* Histogramming options */

typedef struct {
	struct  CmdList triggerSequence[2];
	UINT8 calcFromStruct; /* Pixel: indicates that the serial stream (cal. pulse/
	                         delay/L1A) should be calculated from the scan control
	                         structure. */
	UINT8 calL1ADelay;    /* sets the delay (in BCO units) in the serial stream
	                         between the calibration command and the L1A */
	UINT8 port;           /* Serial ports to use: 0, 1 or both (2) */
	UINT8 unused;
} ScanSerialData;

typedef struct {
	/* registers on the MCC which need setting: */
	UINT16 duration;             /* Length of strobe in BCO units */
	UINT8 delay;                 /* Strobe delay setting on MCC */
	UINT8 delayRange;            /* Strobe delay range on MCC */          

} PixelScanStrobe; /* Strobe conditioning */				     
							    
typedef struct {
	/* registers on the MCC which need setting: */
	UINT8 accepts;               /* Number of contiguous triggers in event (1-16) */
	UINT8 self;                  /* Selects module self-triggering mode */       
	UINT8 latency;               /* 8-bit trigger latency as programmed on FEs */
	UINT8 unused;
} PixelScanTrigger;	/* Trigger conditioning */

typedef struct {
	/* options for resetting modules (beginning of scan & between bins) */
	UINT8 moduleInit;
	UINT8 binReset;
	UINT8 ECR;
	UINT8 BCR;
	UINT8 softFE;
	UINT8 shortSync;
	UINT8 mediumSync;
	UINT8 longSync;
} PixelScanReset; /* Start of scan reset options */	

typedef struct {
	/* global register parameters which need setting: */
	UINT16 vCal;
	UINT8  cInject;             /* Injection capacitor toggle (c-lo/c-high) */
	UINT8  phi;                 /* Column readout frequency */
	UINT8  totThresholdMode;    /* Sets TOT threshold mode */
	UINT8  totMinimum;          /* TOT minimum */
	UINT8  totTwalk;            /* TOT double-hit for digital timewalk correction */
	UINT8  totLeMode;           /* TOT or timestamp leading edge option */
	UINT8  hitbus;          
	UINT8  digitalInject;
	UINT8  unused[2];
} PixelScanFE; /* FE specific options during scans */	

typedef struct {
	UINT8 checkRegisters;     /* Enable periodic checking of MCC warning registers */
    UINT8 bandwidth;
	UINT8 unused[2];
	/* registers on MCC which need setting: */
	UINT16 enableFEContinuityCheck;        /* Bit field */
	UINT16 enableFECrosscheck;             /* Bit field */       

} PixelScanMCC; /* MCC options for scans */

typedef struct {
	UINT8   sCurves;            /* Fit s-curves to the occupancy histos */
	UINT8   totCalibration;     /* Fit TOT calibration functions to TOT data */
	UINT8   tdacTune;           /* Perform TDAC determination based on fit results */
	UINT8   tdacAlgorithm;      /* Method for determining TDAC tune */
	UINT8   fdacTune;           /* Perform FDAC tune based on data from scan */
	UINT8   calibrationOption;  /* Standard time, standard charge or custom */
	UINT8   unused[2];
	
	FLOAT32 chi2Cut;
	FLOAT32 customScale;
	FLOAT32 customOffset;
} PixelScanFitting; /* Fitting options */	

typedef struct {
	UINT8  unused;
	UINT8  currentChip;       /* Current chip being scanned if FE-by-FE mode */
	UINT16 currentMaskStage;  /* Current mask stage */    
	UINT16 currentBin[2];     /* Current scanning point */
	UINT32 currentDelay;           
} ScanStatus;


	typedef struct {
		ScanGeneral          general;
		PixelScanStrobe      strobe;
		PixelScanTrigger     trigger;
		PixelScanReset       reset;
		PixelScanFE          fe;
		PixelScanMCC         mcc;
		PixelScanFitting     fitting;
		ScanRODSetup         rodSetup;
		ScanDspDistribution  dspDist;
		ScanSerialData       serial;
} ScanControl;
	
#endif
