/************************************************************************************
 * primParams_pxl.h
 *
 *  synopsis: Defines IDs and structures for Pixel-specific primitives and tasks.
 * 
 *  Douglas Ferguson, UW Madison   (510) 486-5230         dpferguson@lbl.gov
 ************************************************************************************/
#ifndef PRIM_PARAMS_PXL
#define PRIM_PARAMS_PXL

/************************************************************************************
 *                        Primitives common to all DSPs go here
 * Note: some of these primitives are used to coordinate the DSPs with each other or
 * other electronics on the ROD (such as the router). These primitives should be sent
 * to the master DSP from the host; the master DSP primitive will then set the
 * appropriate board registers, etc. and pass on the primitive to the slave DSP.
 * The primitive passing software will not break if the host sends one of these
 * primitives directly to a slave, but slave applications might break as a result of
 * inconsistent parameter settings. See the code in eventTrapSetup and moduleMask for
 * examples of how to do this.
 ************************************************************************************/
#define COMMON_PIXEL_PRIMITIVES_BASE 0x800

#ifdef TSDEF
	enum {
		LAST_COMMON_PIXEL_PRIMITIVE= COMMON_PIXEL_PRIMITIVES_BASE
	};

#else
	enum {
		TEST= COMMON_PIXEL_PRIMITIVES_BASE,
		LAST_COMMON_PIXEL_PRIMITIVE
	};

#endif

#define NUM_COMMON_PIXEL_PRIMITIVES \
       ((LAST_COMMON_PIXEL_PRIMITIVE) -(COMMON_PIXEL_PRIMITIVES_BASE))

#define R_TEST              100
	typedef struct {
		UINT32 dataLen, *dataPtr;
	} TestIn;

/************************************************************************************
 *                             Slave DSP primitives go here.
 ************************************************************************************/
#define SLAVE_PIXEL_PRIMITIVES_BASE 0x1800

#ifdef TSDEF
	enum {
		LAST_SLAVE_PIXEL_PRIMITIVE= SLAVE_PIXEL_PRIMITIVES_BASE
	};

#else
	enum {
		HISTOGRAM_SETUP= SLAVE_PIXEL_PRIMITIVES_BASE,
		FIT_HISTOGRAMS,
		LAST_SLAVE_PIXEL_PRIMITIVE
	};

#endif

#define NUM_SLAVE_PIXEL_PRIMITIVES \
       ((LAST_SLAVE_PIXEL_PRIMITIVE) -(SLAVE_PIXEL_PRIMITIVES_BASE))

/* Set up & define the histograms. */
#define R_HISTOGRAM_SETUP            109
	typedef struct {
		UINT32  *base;
		UINT32  nBins;
		UINT32  binSize;
		UINT32  routineType;
		UINT32  dataType[2];
		UINT32  opt;
        UINT32  eventsPerL1A; 

		UINT32  hashScheme;
		UINT32  totResolution;
		UINT32  totMin;
		UINT32  totMax;

		UINT32  validModules;
		MDAT32 *xPtr[2];
	} HistogramSetupIn;

	typedef struct {
		UINT32 nBins, binOffset, moduleOffset, size;
		UINT32 *base;
	} HistoMemInfo;

	typedef struct {
		UINT32 nValidModules, validModules;
 		UINT32 *binCtrPtr, *varRangePtr[2];
		HistoMemInfo hmeminfo[8];
	} HistogramSetupOut;

	typedef HistogramSetupOut HistoInfo;

	/* Histogramming options (bitfield):
	 *     b0:    Occupancy histograms are desired 
	 *     b1:    Time-slice histograms are desired.
     *
	 *     b2-4:  Bits set if time-over-threshold histograms are desired:
	 *     b2:    Full TOT spectrum,    
	 *     b3:    TOT mean calculation,
	 *     b4:    TOT sigma calculation.
	 *     b6:    Use TOT range restriction,
         *     b7:    Time-slice are binned
	 *
	 *     b5:    Indicates that fitting is desired.
     *
	 *     b16:   Automatic caching flag: if set, caching will be turned on during
	 *            scanning and fitting, and turned off again upon completion (with
	 *            cache flush).
	 *     
	 *
	 * Output:
	 *     nValidModules  : number of modules in this scan
	 *     base[6]        : occupancy, time-slice, TOT spectrum, TOT mean and TOT sigma, fit
	 *     nBins[6]       : number of scan bins
	 *     binOffset[6]   : the location of a particular bin is given by:
	 *     moduleOffset[6]: base + bin*binOffset + module*moduleOffset
	 *     size[6]        : the corresponding sizes (in words).
	 *     binCtrPtr      : Pointer to the start of the counters which contain the # of
	 *                      events which arrived for each bin.
	 *     varRangePtr    : Pointers to the two variable range lists which define the
	 *                      histograms' x axis (floating point copy of the input).
	 */

	/* The default histogram base is defined inside memoryPartitions.h along with
	   the other memory-related constants. */

	#define HISTOGRAM_8BIT      8
	#define HISTOGRAM_16BIT     16
	#define HISTOGRAM_32BIT     32

	#define HISTO_ROUTINE_C     0
	#define HISTO_ROUTINE_ASM   1

	enum { HIST_OCC= 0, 
	 	   HIST_TIMESLICE, 
	 	   HIST_TOTSPEC, 
	 	   HIST_TOTMEAN, 
	 	   HIST_TOTSIGMA, 
	 	   HIST_FIT,
		   HIST_HITOCC,	
		   HIST_TOTAVERAGE,
	 	   HIST_TOTUSERRANGE,
	       HIST_TSBINNED,

	 	   HIST_AUTOCACHE= 16
	 	 };

#define R_FIT_HISTOGRAMS           100
	typedef struct {
		UINT32 *fitBase;
		UINT32 fitFlags;
		UINT32 routineType;
	} FitHistogramsIn;

	typedef struct {
		UINT32 *fitBase;
		UINT32 dataLen;
	} FitHistogramsOut;

	/* flags: */
	#define USER_WEIGHTS   1
	#define USER_GUESS     2
	#define MAX_LIKELIHOOD 4

	/* routine: */
	#define FIT_ROUTINE_C     0
	#define FIT_ROUTINE_ASM   1

/************************************************************************************
 *                          Master DSP primitives go here.
 ************************************************************************************/
#define MASTER_PIXEL_PRIMITIVES_BASE 0x2800

enum {
#ifdef TSDEF
	TEST_GLOBAL_REG= MASTER_PIXEL_PRIMITIVES_BASE,

#else
	RW_MODULE_DATA= MASTER_PIXEL_PRIMITIVES_BASE,
	RW_MODULE_VARIABLE,
	SEND_CONFIG,
	TEST_GLOBAL_REG,

#endif
	TEST_PIXEL_REG,
	SET_MODULE_SCAN_MODE,
	SET_MASK_STAGE,
	LAST_MASTER_PIXEL_PRIMITIVE
};

#define NUM_MASTER_PIXEL_PRIMITIVES \
       ((LAST_MASTER_PIXEL_PRIMITIVE) -(MASTER_PIXEL_PRIMITIVES_BASE))


/* configure a module, or send it's data (resident in memory) to control links.
   For input data, the pointer to configData is the start. */
#define R_RW_MODULE_DATA   102
	typedef struct {
		UINT32 fRead, cfgSet, module;
		Module *configData;
	} RwModuleDataIn;

	typedef struct {
		Module configData;
	} RwModuleDataOut;

	#define PHYSICS_MODULE_CONFIG  0
	#define SCAN_MODULE_CONFIG     1
	#define SPARE_MODULE_CONFIG    2
	#define N_MODULE_CONFIG_SETS ((SPARE_MODULE_CONFIG) -(PHYSICS_MODULE_CONFIG) +1)

#define R_RW_MODULE_VARIABLE    103
	typedef struct {
		UINT32  fRead, cfgSet, groupId, module, chip, 
		        varType;

		UINT32 info;      /* for writing: output message? */
		UINT32 dataLen;   /* for writing: length of data (e.g. mask= 4) */
		MDAT32 *data;     /* for writing: *data is written to all selected chips/modules */
	} RwModuleVariableIn;

	typedef struct {
		UINT32  nModData, dataLen;
		MDAT32 *data;
	} RwModuleVariableOut;

	#define MVAR_GROUP_ID    100
	#define MVAR_ACTIVE      101

#define R_SEND_CONFIG    105
	typedef struct {
		UINT32   port,  captureSerOn, module[2], chipNum,
		         setLinks, restore, cfgSet, groupId, dataType,
		         activeOnly, enableDataTaking;
	} SendConfigIn;

	#define NO_CONFIG_LOOP             0
	#define NORMAL_CONFIG_LOOP         1

	//Pixel additive bitwise definitions; the 1st 16 correspond to the bit
	//definitions in the pixel control register.
	#define CONFIG_MODULE_ENABLE       0
	#define CONFIG_MODULE_SELECT       1
	#define CONFIG_MODULE_PREAMP       2
	#define CONFIG_MODULE_HITBUS       3

	#define CONFIG_MODULE_TDAC_0       4
	#define CONFIG_MODULE_TDAC_1       5
	#define CONFIG_MODULE_TDAC_2       6
	#define CONFIG_MODULE_TDAC_3       7
	#define CONFIG_MODULE_TDAC_4       8
	#define CONFIG_MODULE_TDAC_5       9
	#define CONFIG_MODULE_TDAC_6       10

	#define CONFIG_MODULE_FDAC_0       11
	#define CONFIG_MODULE_FDAC_1       12
	#define CONFIG_MODULE_FDAC_2       13
	#define CONFIG_MODULE_FDAC_3       14
	#define CONFIG_MODULE_FDAC_4       15

	#define CONFIG_MODULE_GLOBAL       16
	#define CONFIG_MODULE_CONTROL      17
	#define CONFIG_MODULE_TDAC         18
	#define CONFIG_MODULE_FDAC         19

	#define CONFIG_MODULE_SCAN_TDAC    20
	#define CONFIG_MODULE_SCAN_FDAC    21
	#define CONFIG_MODULE_MCC          22
	#define CONFIG_MODULE_ALL          23

	//More definitions are in serialStreams.h

#define R_TEST_GLOBAL_REG   112
	typedef struct {
		UINT32 structId;
		UINT32 moduleId;
		UINT32 FEIndex; 
	} TestGlobalRegIn;

#if 0
	typedef struct {
		UINT32 testResult;
		UINT32 dataLen;
		MDAT32 *data;
	} TestGlobalRegOut;
#endif

#define R_TEST_PIXEL_REG   100
	typedef struct TestPixelRegIn {
		UINT32 structId;
		UINT32 moduleId;
		UINT32 FEIndex; 
		UINT32 regId;
	} TestPixelRegIn;
	typedef struct {
		UINT32 testResult;
		UINT32 dataLen;
		MDAT32 *data;
	} TestPixelRegOut;

#define R_SET_MODULE_SCAN_MODE   100
	typedef struct {
		UINT32 cfgSet;
		UINT32 moduleId;

		PixelScanStrobe   scanStrobe;
		PixelScanTrigger  scanTrigger;
		PixelScanFE       scanFE;
		PixelScanMCC      scanMCC;
	} SetModuleScanModeIn;

#define R_SET_MASK_STAGE   100
	typedef struct {
		UINT32 cfgSet;
		UINT32 moduleId;
		UINT32 chip;

		UINT32 stageMode;
		UINT32 stagePattern;
		UINT32 stage;
	} SetMaskStageIn;

/************************************************************************************
 *  Primitive function prototypes (not needed by the host processor).
 ************************************************************************************/
#if (defined(I_AM_MASTER_DSP) || defined(I_AM_SLAVE_DSP))

INT32 test(PrimData *);

INT32 histogramSetup(PrimData *);
INT32 fitHistograms(PrimData *);

INT32 rwModuleData(PrimData *);
INT32 rwModuleVariable(PrimData *);
INT32 sendConfig(PrimData *);
INT32 testGlobalReg(PrimData *);
INT32 testPixelReg(PrimData *);
INT32 setModuleScanMode(PrimData *);
INT32 setMaskStage(PrimData *);

#endif  /* primitive parameters definition block */
#endif  /* Multiple inclusion protection */
