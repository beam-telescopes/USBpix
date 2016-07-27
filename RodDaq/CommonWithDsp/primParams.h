/************************************************************************************
 * primParams.h
 *
 *  synopsis: Defines primitive ids which are used to index the array of pointers to
 *           primitive functions, structure tags for primitive data and reply data,
 *           and prototypes of primitive functions.
 *
 *  related files:
 *   primitiveBuilding.txt:  describes how to build primitives
 *   primParams_pxl.h, primParams_sct.h: detector-specific versions of this file.
 *   listManager.c: Routines which manage the execution of a primitive list and
 *                 writing the reply data.
 *   primFuncts.c: Contains primitive execution functions common to the master and the
 *                slaves and the function which initializes the primParameters array
 *                of structures.
 *   masterPrimFuncts.c/slavePrimFuncts.c: Contain primitive execution functions
 *                specific to the master/slave DSP.
 *   primFuncts.h: Declares the structure tag which communicates necessary information
 *                to primitive functions.
 *
 *  Damon Fasching, UW Madison                            fasching@wisconsin.cern.ch
 *  Douglas Ferguson, UW Madison   (510) 486-5230         dpferguson@lbl.gov
 *
 *  modifications/bugs
 *   - Split the primitive IDs into 3 sets, so that future primitives can
 *     be placed in the appropriate section (common/slave/master) without
 *     affecting the already existing primitive lists on the host machines.
 *     Currently primitives must be added to the end of the entire list to
 *     avoid ID changes of existing primitives & thus re-writes of the
 *     primitive lists. Updated the example.                         10.04.02 dpsf
 *   - Moved rodConfiguration #defines to rodConfiguration.h, where
 *     they belong.                                                  11.04.02 dpsf
 *
 *   - Split off detector-specific primitives into their own files.  
 ************************************************************************************/
#ifndef PRIM_PARAMS
#define PRIM_PARAMS

#include "processor.h"
#include "serialStreams.h"
#include "scanControl.h"
#include "bocStructure.h"
#include "rodConfiguration.h"
#include "router.h"
//#include "primList.h"

#if   defined(SCT_ROD)
	#include "primParams_sct.h"
#elif defined(PIXEL_ROD)
	#include "primParams_pxl.h"
#endif

#define PRIM_LIST_REVISION     112

/* default value for primitive parameters, function arguments, etc */
#define DEFAULT (0xFFFFFFFF)

/************************************************************************************
 *                        Primitives common to all DSPs go here
 * Note: some of these primitives (eventTrapSetup, triggerMask, setTrigger) are used
 * to coordinate the DSPs with each other or other electronics on the ROD (such as
 * the router). These primitives should be sent to the master DSP from the host; the
 * master DSP primitive will then set the appropriate board registers, etc. and pass
 * on the primitive to the slave DSP. The primitive passing software will not break
 * if the host sends one of these primitives directly to a slave, but slave
 * applications might break as a result of inconsistent parameter settings.
 ************************************************************************************/
#define COMMON_PRIMITIVES_BASE 0x0

enum {
	ECHO= COMMON_PRIMITIVES_BASE,
	SET_MESSAGE_MASK,
	PAUSE_LIST,
	EVENT_TRAP_SETUP,
	SET_MEMORY,
	COPY_MEMORY,
	MEMORY_TEST,
	SET_LED,
	FLASH_LED,
	SEND_DATA,
	MODULE_MASK,
	SET_TRIGGER,
	START_TASK,
	TASK_OPERATION,

#ifdef TSDEF
	TEST,

#else
	ZEN_PRIM,

#endif

	WRITE_BUFFER,
	SET_BIN,
	XFER_DSP_DATA,
	LAST_COMMON_PRIMITIVE
};

#define NUM_COMMON_PRIMITIVES   ((LAST_COMMON_PRIMITIVE)-(COMMON_PRIMITIVES_BASE))

/* Echo the primitive message body back in the reply message body. Echo's input
   and output data is unstructured. */
#define R_ECHO                  100

/* Set message level mask - levels which are masked will not write a message
   to the text buffer. */
#define R_SET_MESSAGE_MASK          100
	typedef struct {
		UINT32 messageMask;
	} SetMessageMaskIn;

/* Pause the execution of the primitive list.  List execution will resume when the
   RESUME bit is set in the command register. */
#define R_PAUSE_LIST            100

/* eventTrapSetup:
   Sets the event trapping parameters for a particular slave.  They need to be set
   in the slave DSP and in the router FPGA. The primitive is common to MDSP & SDSPs-
   however eventTrapSetup is intended to be a HOST to MASTER DSP primitive only; the
   MDSP then sets up the router registers and passes a primitive list to call
   eventTrapSetup on the SDSP(s).  Trap configuation is not independent, if only
   one trap is used, it must be trap 0. The primary purpose for trap 1 is to have
   a way of running short immediate tasks with a different function (like trapping
   events) on the DSP, while it continues a task such as occupancy counting.
 */
#define R_EVENT_TRAP_SETUP      103
	typedef struct {
		UINT32 slvBits, numberOfEvents, timeoutInUsec;
		UINT32 extRouterSetup, distribute;
		UINT32 releaseFrames, permitBackPressure, dataMode, sLink,
		       format, trapStray,  iterLimit;
		UINT32 trapConfig[2], trapExclusionFlag[2], trapFunction[2],
		       trapMatch[2],  trapModulus[2],       trapRemainder[2];
	} EventTrapSetupIn;

	typedef struct {
		UINT32 errorCode;
	} EventTrapSetupOut;

	#define  COLLECT_FOREVER  0
	#define  KEEP_EVENTS      0

	#define  TRAP_DISTRIB_PRIMARY    0
	#define  TRAP_DISTRIB_SECONDARY  1
	#define  TRAP_DISTRIB_BOTH       2
	#define  TRAP_DISTRIB_NEITHER    3

	#define  TRAP_FMT_NORMAL  0
	#define  TRAP_FMT_ERROR   1

/* Quickly set a section of the DSP's memory */
#define R_SET_MEMORY         100
	typedef struct {
		UINT32 *start, size, val;
	} SetMemoryIn;

/* Quickly copy one section of the DSP's memory into another section */
#define R_COPY_MEMORY        100
	typedef struct {
		UINT32 *source, *destination, size;
	} CopyMemoryIn;

/* New memory test, designed to work with full code (ie not the memory-test mode) */
#define R_MEMORY_TEST        102
	typedef struct {
		UINT32 *start, size, repetitions[6], errorsBeforeFail, continueOnError;
		UINT32 nReads, dmaFlag;
	} MemoryTestIn;

	typedef struct {
		UINT32 returnCode;
	} MemoryTestOut;

/* Turn an LED on or off. */
#define R_SET_LED            104
	typedef struct {
		UINT32  ledNum, ledState;
	} SetLedIn;

	#define YELLOW_LED 0
	#define GREEN_LED  1
	#define RED_LED    2

	#define OFF     0
	#define ON      1
	#define TOGGLE  2

/* Flash an LED.  Period and number of flashes are settable parameters. */
#define R_FLASH_LED          103
	typedef struct {
		UINT32 ledNum, period, numTimes;
	} FlashLedIn;

/* Retrieve data, or a pointer to it, from the DSP */
#define R_SEND_DATA          103
	typedef struct {
		UINT32 dataType, auxVal, repBufferFlag, timeout;
	} SendDataIn;
	typedef struct {
		void   *dataPtr;
		UINT32 dataLength;
	} SendDataOut;

	#define REPLY_POINTER      0
	#define REPLY_DIRECT_DATA  1

	#define MAP_DATA       0x00
	#define PRIMITIVE_DATA 0x01

	#define MIRROR_DATA    0x10
	#define ROUTER_DATA    0x11
	#define STREAM_DATA    0x12

	#define EVENT_DATA     0x20
	#define HISTOGRAM_DATA 0x21
	#define OCCUPANCY_DATA 0x22
	#define FIT_DATA       0x23
	#define BIN_DATA       0x24

	/* The following data types are defined for use primarily by the xferDspData
	   (inter-DSP) primitive, but may be of interest to the host as well: */
	#define MODULE_MASK_DATA        0x30
	#define CALIBRATION_COEFF_DATA  0x31

/* moduleMask defines the link setup (command & data) for the modules. Like
   eventTrapSetup, moduleMask is meant to be sent from the host to the master DSP,
   which will transfer its data to the slave DSP(s) if desired. */
	#define R_MODULE_MASK       101

	#if defined(I_AM_SLAVE_DSP)
		typedef struct {
			ModuleMaskData moduleMaskData[N_TOTMODULES];
		} ModuleMaskIn;
	#else
		typedef struct {
			UINT32 module, port, useStructSet, passOn, slvBits;
			UINT32 cmdLine, fmtLink[2], unused[2];
			UINT32 cfg, modMask[2], maskType, storage, maskSet; 
		} ModuleMaskIn;
	#endif

/* Send a trigger to the detector electronics. If setTrigger is issued to a
   slave DSP, the trigger source is assumed as external (possibly not even issued
   by the master DSP); in this case only the current bin & histogram set are set. */
#define R_SET_TRIGGER        102
	typedef struct {
		CmdList cmdList[2];
		UINT32  slvBits;
		UINT16  cmdBuff, preBuilt;
		UINT32  chipAddress, bin, set;
		UINT32  repetitions, interval;
		UINT32  preBuiltDataOffset[2];
		UINT16  incCmd[2];
		UINT32  incData[2];
	} SetTriggerIn;

/* Serial port definitions; used by several primitives & tasks: */
#define SP0      0
#define SP1      1
#define SP_BOTH  2

#define R_START_TASK         101
	/* Task revision numbers must be updated whenever a task's input parameters
	   are modified. Revision numbers of the tasks should not exceed 0xff. If
	   a task has a default position for its memory base (eg. trapTask), that is
	   defined inside memoryPartitions.h along with the other memory-related
	   constants. */

	/* master */
/* Task which controls an entire histogram scan */
	#define R_HISTOGRAM_CTRL_TASK   107

	typedef struct {
		ScanControl scanControl;
	} HistoCtrlTaskIn;

	typedef struct {
		UINT32  totalTime;
		UINT32  slvProcTime[4];
		UINT32  *dataPtr, dataLength;
	} HistoCtrlTaskOut;

	#define R_MIRROR_TASK     101
	typedef struct {
		UINT32 slvBits, mirrorFreq, *mirrorBase,
		       *mirrorSlvBase, mirrorLen;
	} MirrorTaskIn;

	#define MIRROR_DEFAULT_BASE 0x80008000

	#define R_TRAP_REQ_TASK   101
	typedef struct {
		UINT32 slvBits, watchFreq;
	} TrapReqTaskIn;

	/* slave */
	#define R_HISTOGRAM_TASK  100
	typedef struct {
		UINT32 nEvents, controlFlag;  /* nEvents can equal COLLECT_FOREVER too. */
	} HistogramTaskIn;

	/* control flag settings: */
	#define MASTER_HREG     0
	#define LOCAL_INC       1
	#define LOCAL_SET_TRIG  2

	typedef struct {
		UINT32  nEvents, binsDone;
		FLOAT32 avgProcTime;
		UINT32  *dataPtr, dataLength;
	} HistogramTaskOut;

	#define R_TRAP_TASK       100
	typedef struct {
		UINT32 nEvents, reloadInterval, trapType, eventType,
		       *trapBufferBase, trapBufferLength;
	} TrapTaskIn;

	typedef struct {
		UINT32  nEvents, dataLen;
		UINT32  *bufferBase, bufferLen;
	} TrapTaskOut;

	#define R_OCCUPANCY_TASK  100
	typedef struct {
		UINT32 nEvents, nFilters, splitFlag;
        char   filter[16];
	} OccupancyTaskIn;

	#define R_ERROR_TASK      100
	typedef struct {
		UINT32 errorType;
	} ErrorTaskIn;
	
/* old Rev E EFB firmware defs
	#define N_EFB_ERROR_BITS 16
	enum {
		ERRORTYPE_HEADER=0,
		ERRORTYPE_TRAILER,
		ERRORTYPE_FEFLAG,
		ERRORTYPE_SYNCHBIT,
		ERRORTYPE_MCCFLAG,
		ERRORTYPE_L1ID,
		ERRORTYPE_BCID,
		ERRORTYPE_TIMEOUT,
		ERRORTYPE_ALMOSTFULL,
		ERRORTYPE_OVERFLOW,
		ERRORTYPE_NONSEQCHIP,
		ERRORTYPE_INVROW,
		ERRORTYPE_INVCOL,
		ERRORTYPE_MCCSKIP,
		ERRORTYPE_SPARE1,
		ERRORTYPE_SPARE2,
		ERRORTYPE_DATALOSS,
		ERRORTYPE_LAST
	};
*/
	#define N_EFB_ERROR_BITS 32
	enum {
		ERRORTYPE_BCID=0,
 		ERRORTYPE_L1ID,
		ERRORTYPE_TIMEOUT,
		ERRORTYPE_DATAINCORRECT,
 		ERRORTYPE_BUFFEROVERFLOW,
		ERRORTYPE_ATLAS0,
		ERRORTYPE_ATLAS1,
		ERRORTYPE_ATLAS2,
		ERRORTYPE_ATLAS3,
		ERRORTYPE_ATLAS4,
		ERRORTYPE_ATLAS5,
		ERRORTYPE_ATLAS6,
		ERRORTYPE_ATLAS7,
		ERRORTYPE_ATLAS8,
		ERRORTYPE_ATLAS9,
		ERRORTYPE_ATLAS10,
		ERRORTYPE_ALMOSTFULL,
 		ERRORTYPE_OVERFLOW,
 		ERRORTYPE_HEADER,
		ERRORTYPE_SYNCHBIT,
		ERRORTYPE_INVROWCOL,
 		ERRORTYPE_MCCSKIP,
		ERRORTYPE_FE_ERROR_EOC,
		ERRORTYPE_FE_ERROR_HAMMING,
		ERRORTYPE_FE_ERROR_REGPARITY,
		ERRORTYPE_FE_ERROR_HITPARITY,
		ERRORTYPE_FE_ERROR_BITFLIP,
 		ERRORTYPE_MCC_ERROR_HITOVERFLOW,
		ERRORTYPE_MCC_ERROR_EOEOVERFLOW,
		ERRORTYPE_MCC_ERROR_L1FEFAIL,
		ERRORTYPE_MCC_ERROR_BCIDFAIL,
		ERRORTYPE_MCC_ERROR_L1GLOABLFAIL,
  		ERRORTYPE_DATALOSS,
		ERRORTYPE_LAST
	};
	
	enum {
		ERRORTYPE_D_=0,
		ERRORTYPE_D_TIMEOUTDATA,
		ERRORTYPE_D_MISSINGTRAILER,
		ERRORTYPE_D_BCID,
		ERRORTYPE_D_BCID_L1,
		ERRORTYPE_D_BCID_TIMEOUT,
		ERRORTYPE_D_BCID_PREAMBLE,
		ERRORTYPE_D_OVERFLOW,
		ERRORTYPE_D_OVERFLOW_HEADERTRAILER,
		ERRORTYPE_D_OVERFLOW_ZTRAILER,
		ERRORTYPE_D_INVROWCOL,
		ERRORTYPE_D_RAWDATA,
		ERRORTYPE_D_FE_ERROR_EOC,
		ERRORTYPE_D_FE_ERROR_HAMMING,
		ERRORTYPE_D_FE_ERROR_REGPARITY,
		ERRORTYPE_D_FE_ERROR_HITPARITY,
		ERRORTYPE_D_FE_ERROR_U4,
		ERRORTYPE_D_FE_ERROR_U5,
		ERRORTYPE_D_FE_ERROR_U6,
		ERRORTYPE_D_FE_ERROR_U7,
		ERRORTYPE_D_MCC_ERROR_HITOVERFLOW,
		ERRORTYPE_D_MCC_ERROR_EOEOVERFLOW,
		ERRORTYPE_D_MCC_ERROR_L1FEFAIL,
		ERRORTYPE_D_MCC_ERROR_BCIDFAIL,
		ERRORTYPE_D_MCC_ERROR_L1GLOABLFAIL,
		ERRORTYPE_D_MCC_ERROR_U5,
		ERRORTYPE_D_MCC_ERROR_U6,
		ERRORTYPE_D_MCC_ERROR_U7,
		ERRORTYPE_D_LAST
	};
//  start of 16 bit FE and MCC Error word
	#define ERRORTYPE_D_FE_ERROR ERRORTYPE_D_FE_ERROR_EOC

//	#define MAX_LINKS 96
	#define MAX_LINKS 128
	typedef struct {
		UINT32	errorCount;	                  // # total of errors (e.g if more than 1 per event)
		UINT32	eventCount;	                  // # total events processed by error task
		UINT32	errorEventCount;	          // # events with errors
		UINT16  type[ERRORTYPE_LAST];         // from FPGA
		UINT16  detailedtype[ERRORTYPE_D_LAST]; // from decoded data
		UINT16  linkErrorCnt[MAX_LINKS];	
	} ErrorInfo;
	typedef struct {
		UINT32 nErrors;
		ErrorInfo errorInfo;	
	} ErrorTaskOut;

	#define R_RESYNCH_TASK    100
	typedef struct {
		UINT32 errorType;
	} ResynchTaskIn;

	/* tasks can have their own specialized output structures, this is a
	   general one. It is stored as an array of external structures in
	   the task manager, so that if a task completes & later the data is
	   requested, it can be supplied. */
	typedef struct {
		UINT32 *dataPtr, dataLength;
	} GenTaskOut;

	typedef union {
		HistoCtrlTaskOut  histoCtrlTaskOut;
		HistogramTaskOut  histogramTaskOut;
		TrapTaskOut       trapTaskOut;
		ErrorTaskOut      errorTaskOut;
		GenTaskOut        genTaskOut;
	} TaskOutput;

	/* MAX_NUM_TASKS is the maximum of the two task numbers for master &
	   slave DSPs. Bad Things can happen if it is not kept up to date */
	#define MAX_NUM_TASKS   5

	/* The IDs of the two task sets cannot not exceed 0xff, and must increment by 1. */
	/* master DSP tasks: */
	#define MASTER_TASK_BASE (0x10)
	#define SLAVE_TASK_BASE  (0x20)

	enum {
		HISTOGRAM_CTRL_TASK= MASTER_TASK_BASE,
		MIRROR_TASK,
		TRAP_REQ_TASK,
		
		HISTOGRAM_TASK= SLAVE_TASK_BASE,
		TRAP_TASK,
		OCCUPANCY_TASK,
		ERROR_TASK,
		RESYNCH_TASK
	};


	#define ALL_SLAVESX      5

	#define LAST_MASTER_TASK  (TRAP_REQ_TASK)
	#define NUM_MASTER_TASKS  ((LAST_MASTER_TASK)-(MASTER_TASK_BASE)+1)

	#define LAST_SLAVE_TASK  (RESYNCH_TASK)
	#define NUM_SLAVE_TASKS  ((LAST_SLAVE_TASK)-(SLAVE_TASK_BASE)+1)

	#if   defined(I_AM_MASTER_DSP)
		#define LAST_TASK  (LAST_MASTER_TASK)
		#define NUM_TASKS  (NUM_MASTER_TASKS)
		#define TASK_BASE  (MASTER_TASK_BASE)
	#elif defined(I_AM_SLAVE_DSP)
		#define LAST_TASK  (LAST_SLAVE_TASK)
		#define NUM_TASKS  (NUM_SLAVE_TASKS)
		#define TASK_BASE  (SLAVE_TASK_BASE)
	#endif

	typedef union {
		HistoCtrlTaskIn  histoCtrlTaskIn;
		MirrorTaskIn     mirrorMemoryTaskIn;
		TrapReqTaskIn    trapRequestTaskIn;

		HistogramTaskIn  histogramTaskIn;
		TrapTaskIn       trapTaskIn;
		OccupancyTaskIn  occupancyTaskIn;
		ErrorTaskIn      errorTaskIn;
		ResynchTaskIn    resynchTaskIn;
	} TaskInput;

	typedef struct {
		UINT32 taskType, taskRevision, priority, completionFlag;
		TaskInput taskStruct;
	} StartTaskIn;

/* Perform operations such as stop, pause etc. on a running task: */
#define R_TASK_OPERATION         100
	typedef struct {
		UINT32 taskType, taskOperation, data;
	} TaskOperationIn;

#define TASK_STOP         0
#define TASK_PAUSE        1
#define TASK_RESUME       2
#define TASK_QUERY        3
#define TASK_RESET        4
#define TASK_SETPRIORITY  5

#define R_WRITE_BUFFER         102
	typedef struct {
		UINT32 buffer;
		char string[128];
	} WriteBufferIn;

	#define WRITE_BUFFER_ERR  0
	#define WRITE_BUFFER_INFO 1
	#define WRITE_BUFFER_DIAG 2
	#define WRITE_BUFFER_XFER 3

#define R_SET_BIN   100
	typedef struct {
		#if defined(I_AM_MASTER_DSP)
			UINT32 slvBits;
		#endif

		UINT32 bin;
	} SetBinIn;

#define R_XFER_DSP_DATA 100
	typedef struct {
		UINT32 dataType;
		UINT32 dataLen;
	} XferDspDataIn;

/************************************************************************************
 *                             Slave DSP primitives go here.
 ************************************************************************************/
#define SLAVE_PRIMITIVES_BASE  0x1000

#ifdef TSDEF
	enum {
		START_EVENT_TRAPPING= SLAVE_PRIMITIVES_BASE,
		STOP_EVENT_TRAPPING,
		HISTOGRAM_SETUP,
		FIT_HISTOGRAMS,
		LAST_SLAVE_PRIMITIVE
	};

#else
	enum {
		START_EVENT_TRAPPING= SLAVE_PRIMITIVES_BASE,
		STOP_EVENT_TRAPPING,
		LAST_SLAVE_PRIMITIVE
	};
	
#endif

#define NUM_SLAVE_PRIMITIVES   ((LAST_SLAVE_PRIMITIVE)-(SLAVE_PRIMITIVES_BASE))

#define R_START_EVENT_TRAPPING       101

#define R_STOP_EVENT_TRAPPING        100

/************************************************************************************
 *                          Master DSP primitives go here.
 ************************************************************************************/
#define MASTER_PRIMITIVES_BASE 0x2000

enum {
	RW_SLAVE_MEMORY= MASTER_PRIMITIVES_BASE,
	TRANS_SERIAL_DATA,
	START_SLAVE_EXECUTING,
	CONFIG_SLAVE,
	RW_REG_FIELD,
	POLL_REG_FIELD,
	RW_FIFO,
	SEND_SLAVE_LIST,
	START_SLAVE_LIST,
	SLAVE_LIST_OP,
	BUILD_STREAM,
	SEND_STREAM,

#ifdef TSDEF
	RW_MODULE_DATA,
	SEND_CONFIG,
#endif

	DSP_RESET,
	SET_ROD_MODE,

#ifdef TSDEF
	RW_MODULE_VARIABLE,
#endif

	RW_BOC_DATA,
	BOC_HISTOGRAM,
	WRITE_FLASH,
	LAST_MASTER_PRIMITIVE
};

#define NUM_MASTER_PRIMITIVES   ((LAST_MASTER_PRIMITIVE)-(MASTER_PRIMITIVES_BASE))

/* Read or write a block of SDSP memory. The structure is the same for input & output */
#define R_RW_SLAVE_MEMORY       100
	typedef struct {
		UINT32 slaveNumber;
		UINT32 fRead;
		UINT32 *slaveAddress;   /* DEFAULT address defined above */
		UINT32 *masterAddress;  /* DEFAULT address defined above */
		UINT32 numWords;
	} RwSlaveMemory;
	typedef RwSlaveMemory RwSlaveMemoryIn;
	typedef RwSlaveMemory RwSlaveMemoryOut;

/* Transmit serial data to controller FPGA */
#define R_TRANS_SERIAL_DATA 100
	typedef struct {
		UINT32 port0Data; /* 32b serial data port 0 */
		UINT32 port1Data; /* 32b serial data port 1 */
	} TransData;
 	typedef struct {
		UINT32   captureSerOn;      /* Capture module return data in input FIFOs */
		UINT32   streamLen[2], *streams;   /* data sets to transmit */
	} TransSerialDataIn;

/* Set DSPINT to start SDSP. */
#define R_START_SLAVE_EXECUTING   104
	typedef struct {
		UINT32 slaveNumber;
		UINT32 commOnOff;
		UINT32 slaveType;
		UINT32 timeoutInUsec;
	} StartSlaveExecutingIn;
	typedef struct {
		UINT32 slaveNumber;
	} StartSlaveExecutingOut;

	/* commOnOff options */
	#define SLV_DSP_COMM_OFF      0
	#define SLV_DSP_COMM_ON       1

	/* slaveType options */
	#define SLV_DSP_UNCONFIGURED  0
	#define SLAVE_CALIBRATE       1
	#define SLAVE_MONITOR         2
	#define SLAVE_ERROR_CHECK     3
	#define SLAVE_MEMORY_TEST     4

/* Sets the parameters of the structure rodConfig.slvDspConfig[slvId]. */
#define R_CONFIG_SLAVE          102
	typedef struct {
		UINT32 slaveNumber;
		UINT32 commOnOff;
		UINT32 slaveType;
	} ConfigSlaveIn;  /* parameters values are same as for START_SLAVE_EXECUTING */

/* Reads or writes a field of a register in the master DSP memory map.
   registerID: ID of the register being polled
   offset: number of lsb in the field, starting with 0
   width: width of the field mnemonic symbols for registerID, offset and width
          are in registerIndices.h
   fRead: TRUE for read, FALSE for write
   dataIn: data to write for fRead= FALSE; no significance for fRead= TRUE
   dataOut: data read for fRead= TRUE; no significance for fRead= FALSE  */
#define R_RW_REG_FIELD               105
	typedef struct {
		UINT32 registerID;
		UINT32 offset;
		UINT32 width;
		UINT32 fRead;
		UINT32 dataIn;
	} RwRegFieldIn;
	typedef struct {
		UINT32 dataOut;
	} RwRegFieldOut;

/* Polls a field of a register for a value.  A timeout is available.
   registerID: ID of the register being polled
   offset: number of lsb in the field, starting with 0
   width: width of the field mnemonic symbols for registerID, offset and width
          are in registerIndices.h
   desiredValue: value of the defined field to poll for.
   timeoutInUsec: timeout period in microseconds.
   found: indicates whether 'desiredValue' was found within the timed period. */
#define R_POLL_REG_FIELD             105
	typedef struct {
		UINT32 registerID;
		UINT32 offset;
		UINT32 width;
		UINT32 desiredValue;
		UINT32 timeoutInUsec;
	} PollRegFieldIn;
	typedef struct {
		UINT32 found;
	} PollRegFieldOut;

/* Reads/writes some number of words from/to one of the FIFOs on the ROD.
  - fifoID: INPUT_MEM, DEBUG_MEM or EVENT_MEM
  - bank: BANK_A, BANK_B or (EVENT_MEM only) BANK_C
  - fRead: TRUE for read, FALSE for write
  - numElements: number of fifo entries to read or write; NOTE: no packing, eg
               to write 20 elements to INPUT_MEM BANK_A requires 40 32b data words
               to write 20 elements to EVENT_MEM BANK_C requires 20 32b data words
               20 elements read from EVENT_MEM BANK_C will occupy 20 32b data words
  - *dataBaseAdr: source/destination  address of data for/from the FIFO
    if *dataBaseAdr = DEFAULT
      fRead = FALSE: data is supplied with the RW_FIFO primitive after *data
      fRead = TRUE: data is written to the reply message body after bytesXfrd
    There is also a scratch space in SDRAM at SCRATCH_BASE (memoryPartitions.h)
  - bytesXfrd: Number of bytes read or written */
#define R_RW_FIFO                 104
	typedef struct {
		UINT32 fifoId;
		UINT32 bank;
		UINT32 fRead;
		UINT32 numElements;
		UINT32 *dataBaseAdr;    /* DEFAULT value defined above */
	} RwFifoIn;
	typedef struct {
		UINT32 bytesXfrd;
	} RwFifoOut;

	#define INPUT_MEM 0x0             /* values of fifoID */
	#define DEBUG_MEM 0x1
	#define EVENT_MEM 0x2
	#define TIM_MEM   0x3
	#define BANK_A    0x0             /* values of bank; ignored for TIM_MEM */
	#define BANK_B    0x1
	#define BANK_C    0x2             /* EVENT_MEM only */

/* Send a primitive list to a slave DSP.
   slaveNumber: ID of slave DSP to recieve the list.
   listLength: number of 32 bit words in the list.
   *slavePrimList: Address of start of source data for the list.
     - *slavePrimList = DEFAULT indicates that the slave primitive list is stored in
       the body of this SEND_SLAVE_LIST primtive starting just after the last
       SEND_SLAVE_LIST_IN structure element.
   *slaveRepData: Address where master should put slave reply data if the
               START_SLAVE_LIST primitive is executed with getSlaveReply != 0.
     - *slaveRepData = DEFAULT indicates that the slave reply list should be stored
       in the body of a reply message to the START_SLAVE_LIST primtive. */
#define R_SEND_SLAVE_LIST    103
	typedef struct {
		UINT32 slaveNumber;
		UINT32 listLength;
		UINT32 *slavePrimList;    /* DEFAULT value defined above */
		UINT32 *slaveRepData;     /* DEFAULT value defined above */
	} SendSlaveListIn;

/* Start a slave DSP executing its primitive list by setting its inListRdy bit.
   slaveNumber: ID of slave DSP to execute list
   pauseMasterList= TRUE if master DSP list execution pauses while slave executes list
   getSlaveReply= TRUE if the master should read the slave reply data. */
#define R_START_SLAVE_LIST    103
	typedef struct {
		UINT32 slaveNumber;
		UINT32 pauseMasterList;
		UINT32 getSlaveReply;
	} StartSlaveListIn;

/* pause, resume or abort a slave list */
#define R_SLAVE_LIST_OP    101
	typedef struct {
		UINT32 slaveNumber;
		UINT32 listOp;
	} SlaveListOpIn;

	#define LIST_PAUSE  0
	#define LIST_RESUME 1
	#define LIST_ABORT  2

/* Build & send a command stream to the detector electronics. sendData will ship
   a command stream back to the VME host. */
#define R_BUILD_STREAM        102
	typedef struct {
		CmdList cmdList;
		UINT32  port, reset, chip, fibre;
		UINT32  dataLen;    /* Length of the extra data, in bits. */
		UINT32  *data;      /* Start of the data for a command with large amounts of
		                       data (masks & some pixel commands). */
	} BuildStreamIn;

#define R_SEND_STREAM        100
	typedef struct {
		UINT32 port, captureSerOn;
	} SendStreamIn;

#define R_DSP_RESET    100
	typedef struct {
		UINT32  slvBits, forceSync, nAttempts, timeOut;
	} DspResetIn;

#define R_SET_ROD_MODE    100
	typedef struct {
		UINT32  mode, flag, dataLen, *data;
	} SetRodModeIn;

#define R_RW_BOC_DATA    100
	typedef struct {
		UINT32  fRead, sendToBoc, dataLen, *data;
	} RwBocDataIn;
	typedef struct {
		BOCConfig bocCfgData;
	} RwBocDataOut;

#define R_BOC_HISTOGRAM    100
	typedef struct {
		UINT32  numSamples, numLoops;
	} BocHistogramIn;
	typedef struct {
		UINT32 histo[96][2];
	} BocHistogramOut;

#define R_WRITE_FLASH    100
	typedef struct {
		void *buffPtr;
		UINT32 offset, length; /* effective address = &flash[offset] */
	} WriteFlashIn;

#if defined(I_AM_HOST)
	#if   defined(SCT_ROD)
		#define NUM_PRIMITIVES ( (NUM_COMMON_PRIMITIVES)     \
		                        +(NUM_SLAVE_PRIMITIVES)      \
		                        +(NUM_MASTER_PRIMITIVES)     \
		                        +(NUM_COMMON_SCT_PRIMITIVES) \
		                        +(NUM_SLAVE_SCT_PRIMITIVES)  \
		                        +(NUM_MASTER_SCT_PRIMITIVES)   )

	#elif defined(PIXEL_ROD)
		#define NUM_PRIMITIVES ( (NUM_COMMON_PRIMITIVES)       \
		                        +(NUM_SLAVE_PRIMITIVES)        \
		                        +(NUM_MASTER_PRIMITIVES)       \
		                        +(NUM_COMMON_PIXEL_PRIMITIVES) \
		                        +(NUM_SLAVE_PIXEL_PRIMITIVES)  \
		                        +(NUM_MASTER_PIXEL_PRIMITIVES)   )
	#endif

#elif defined(I_AM_MASTER_DSP)
	#if   defined(SCT_ROD)
		#define NUM_PRIMITIVES ( (NUM_COMMON_PRIMITIVES)     \
		                        +(NUM_MASTER_PRIMITIVES)     \
		                        +(NUM_COMMON_SCT_PRIMITIVES) \
		                        +(NUM_MASTER_SCT_PRIMITIVES)   )

	#elif defined(PIXEL_ROD)
		#define NUM_PRIMITIVES ( (NUM_COMMON_PRIMITIVES)       \
		                        +(NUM_MASTER_PRIMITIVES)       \
		                        +(NUM_COMMON_PIXEL_PRIMITIVES) \
		                        +(NUM_MASTER_PIXEL_PRIMITIVES)   )
	#endif

#elif defined(I_AM_SLAVE_DSP)
	#if   defined(SCT_ROD)
		#define NUM_PRIMITIVES ( (NUM_COMMON_PRIMITIVES)     \
		                        +(NUM_SLAVE_PRIMITIVES)      \
		                        +(NUM_COMMON_SCT_PRIMITIVES) \
		                        +(NUM_SLAVE_SCT_PRIMITIVES)    )

	#elif defined(PIXEL_ROD)
		#define NUM_PRIMITIVES ( (NUM_COMMON_PRIMITIVES)       \
		                        +(NUM_SLAVE_PRIMITIVES)        \
		                        +(NUM_COMMON_PIXEL_PRIMITIVES) \
		                        +(NUM_SLAVE_PIXEL_PRIMITIVES)    )
	#endif

#endif

/************************************************************************************
 *  Primitive function prototypes (not needed by the host processor).
 ************************************************************************************/
#if (defined(I_AM_MASTER_DSP) || defined(I_AM_SLAVE_DSP))
INT32 noPrimitive(PrimData *);

INT32 echo(PrimData *);
INT32 setMessageMask(PrimData *);
INT32 pauseList(PrimData *);
INT32 eventTrapSetup(PrimData *);
INT32 setMemory(PrimData *);
INT32 copyMemory(PrimData *);
INT32 memoryTest(PrimData *);
INT32 setLed(PrimData *);
INT32 flashLed(PrimData *);
INT32 sendData(PrimData *);
INT32 moduleMask(PrimData *);
INT32 setTrigger(PrimData *);
INT32 startTask(PrimData *);
INT32 taskOperation(PrimData *);
INT32 writeBuffer(PrimData *);
INT32 setBin(PrimData *);
INT32 xferDspData(PrimData *);

INT32 startEventTrapping(PrimData *);
INT32 stopEventTrapping(PrimData *);

INT32 rwSlaveMemory(PrimData *);
INT32 transSerialData (PrimData *);
INT32 startSlaveExecuting(PrimData *);
INT32 configSlave(PrimData *);
INT32 rwRegField(PrimData *);
INT32 pollRegField(PrimData *);
INT32 rwFifo(PrimData *);
INT32 sendSlaveList(PrimData *);
INT32 startSlaveList(PrimData *);
INT32 slaveListOp(PrimData *);
INT32 buildStream(PrimData *);
INT32 sendStream(PrimData *);
INT32 dspReset(PrimData *);
INT32 setRodMode(PrimData *);
INT32 rwBocData(PrimData *);
INT32 bocHistogram(PrimData *);
/* INT32 configBoc(PrimData *); */
INT32 writeFlash(PrimData *);

#endif  /* primitive parameters definition block */
#endif  /* Multiple inclusion protection */
