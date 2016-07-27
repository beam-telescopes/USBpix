/* Common Header */

#ifndef PRIMXFACE_COMMON_H
#define PRIMXFACE_COMMON_H

#define PRIM_ECHO_STRING "echo"
#define PRIM_RW_MEMORY_STRING "rw memory"
#define PRIM_SET_LED_STRING "set led"
#define PRIM_RW_REG_FIELD_STRING "rw reg field"
#define PRIM_MODULE_MASK_STRING "module mask"
#define PRIM_SLAVE_CONTROL_STRING "slave control"
#define PRIM_TEST_GLOBAL_REGISTER_STRING "test global register"
#define PRIM_TEST_ALL_REGISTERS_STRING "test all module registers"
#define PRIM_LOAD_MODULE_STRING "load module configuration"
#define PRIM_READ_MODULE_STRING "read module configuration"
#define PRIM_SEND_MODULE_STRING "send module configuration"
#define PRIM_SEND_MODULE_WITH_PREAMP_OFF_STRING "send modCfg Preamp Off"
#define PRIM_RW_SLAVE_MEMORY_STRING "rw slave memory"
#define PRIM_SEND_SERIAL_STRING "send serial stream"
#define PRIM_RW_FIFO_STRING "rw fifo"
#define PRIM_SEND_SLAVE_PRIMITIVE_STRING "send slave primitive"
#define PRIM_WRITE_FLASH_STRING "write flash"
#define PRIM_QUICK_XFER_STRING "quick xfer"
#define PRIM_EVENT_TRAP_SETUP_STRING "event trap setup"
#define PRIM_START_TASK_STRING "start task"
#define PRIM_TALK_TASK_STRING "talk task"
#define PRIM_SEND_TRIGGER_STRING "send trigger"
#define PRIM_SETUP_GROUP_STRING "setup group"
#define PRIM_TEST_PIXEL_REGISTER_STRING "test pixel register"
#define PRIM_LOAD_DATA_FRAME_STRING "load data frame"
#define PRIM_EXPERT_STRING "expert"
#define PRIM_FIND_LINK_STRING "find link"
#define PRIM_MCC_EVENT_BUILD_STRING "mcc event build"
#define PRIM_SET_ROD_MODE_STRING "set rod mode"
#define PRIM_RW_MCC_STRING "rw mcc"
#define PRIM_CONFIGURE_ENVIRONMENT_STRING "configure environment"
#define PRIM_SEND_SLAVE_LIST_STRING "send slave list"
#define PRIM_FIT_STRING "fit"

#define CHIP_MASK 0x10000
#define MODULE_FLAG	0x80000000
#define SLAVE_FLAG	0x40000000
#define MAST_FREEMEMORY_BASE  0x02d00000

enum PrimitiveIds {
	ECHO = 1,
	RW_MEMORY,
	SET_LED,
	RW_REG_FIELD,
	MODULE_MASK,
	SLAVE_CTRL,
	TEST_GLOBAL_REGISTER,
	LOAD_MODULE_CONFIG,
	READ_MODULE_CONFIG,
	SEND_MODULE_CONFIG,
	RW_SLAVE_MEMORY,
	SEND_SERIAL_STREAM,
	RW_FIFO,
	SEND_SLAVE_PRIMITIVE,
	WRITE_FLASH,
	START_TASK,
	TALK_TASK,
	SEND_TRIGGER,
	SETUP_GROUP,
	TEST_PIXEL_REGISTER,
	LOAD_DATA_FRAME,
	EXPERT,
	FIND_LINK,
	MCC_EVENT_BUILD,
	SET_ROD_MODE,
	RW_MCC,
	CONFIGURE_ENVIRONMENT,
	SEND_SLAVE_LIST,
	QUICKXFER, /* generic quick transfer */
	TEST_ALL_REGISTERS,
	SEND_MODULE_CONFIG_WITH_PREAMP_OFF,
//	LOAD_FRAME = SLAVE_PRIMITIVE_BASE,
//	FIT_SCURVE,
//	INVOKE_INTERRUPT,
//	QUICKSLAVEXFER, /* transfer from slave */
/*
	SETUP_HISTOGRAM,
	START_HISTOGRAM,
	DOWNLOAD_HISTOGRAM,
	SETUP_FIT,
	START_FIT,
	DOWNLOAD_FIT,
 */
	LAST_PRIMITIVE /* dummy place holder */
};

/*** TASK DEFINITIONS ***/

enum {
	DIAGNOSTIC_TASK = 0,
	SCAN_TASK,
	SCAN_TASK_2,
	HISTOGRAM_TASK,
	BOC_SCANM,
	LEAK_SCAN,
	STATUS_READ,
	DUMMY_TASK,
	N_PREDEFINED_TASKS
};

/*** primitive structures ***/
enum {
	OPEN_FILE,
	POSITION_FILE,
	WRITE_FILE_DATA,
	READ_FILE_DATA,
	CLOSE_FILE,
	LAST_HOST_PRIMITIVE
};

/*** INVOKE INTERRUPT ( diagnostic ) ***/
typedef struct {
	UINT32 irq;
} InvokeInterruptIn;

/* used for RW flags across various primitives */
#define RW_WRITE 1
#define RW_READ 2

/* section header */
typedef struct {
	UINT32 id; /* slave = 0, 1, 2, 3; master = 4 */
	UINT32 nWords;
	UINT32 addr;
} RwMemoryReplyHeader;

/*** RW SLAVE MEMORY ***/
/* if SLAVE_MASK is set, RwSlaveMemory::slave is interpreted as a mask */
#define SLAVE_MASK 0x10

typedef struct {
	UINT32 slave, flags, slaveAddr, nWords, masterAddr;
} RwSlaveMemoryIn;

/*** TEST GLOBAL REGISTER ***/
typedef struct {
	UINT32 moduleIndex, chipIndex;
} TestGlobalRegisterIn;

/*** TEST ALL REGISTERS ***/
typedef struct {
  UINT32 moduleMask, chipMask, doSEU;
} TestAllRegistersIn;

/*** TEST PIXEL REGISTER ***/
typedef struct {
	UINT32 moduleIndex, chipIndex;
} TestPixelRegisterIn;

/*** LEAK CURRENT SCAN ***/
typedef struct {
  UINT32 moduleMask, chipMask, control, nmaskStages;
} LeakCurrentScanIn;

/*** BOC SCAN ***/
typedef struct {
	UINT32 moduleMask, nThrs, nDelays, ThrMin,ThrMax,DelMin,DelMax,pattern_in, nHits, control,nevnts,loop2ID,iLink;
} BocScanIn;

/*** SEND SLAVE LIST ***/
/* back by popular demand */
typedef struct {
	UINT32 slave, queueIndex, nPrimitives, nWords;
} SendSlaveListIn;

/*** SEND SLAVE PRIMITIVE ***/
typedef struct {
	UINT32 slave, primitiveId, nWords, fetchReply;
} SendSlavePrimitiveIn;

/*** RW MEMORY ***/

#define RW_SPECIFY_LENGTH 1

typedef struct {
	UINT32 src, dst, count, flag;
} RwMemoryIn;

/*** LOAD SLAVE CODE ***/

typedef struct {
	UINT32 addr, length, fInternal;
} DldHeader;

#define DSP_RESET		0x01
#define SLAVE_INIT	0x02
#define LOAD_COFF		0x04
#define LOAD_DONE		0x08
#define WAIT_BOOT		0x10

typedef struct {
	UINT32 slaveMask, flag, nSections;
} SlaveCtrlIn;

/*** SET LED ***/

typedef struct {
	UINT32  num, state;
} SetLedIn;

#define YELLOW_LED 0
#define GREEN_LED  1
#define RED_LED    2

#define OFF     0
#define ON      1
#define TOGGLE  2

/*** RW REG FIELD ***/

typedef struct {
	UINT32 regAddr;
	UINT32 value;
	UINT32 offset;
	UINT32 width;
	UINT32 flags;
} RwRegFieldIn;

typedef struct {
	UINT32 value;
} RwRegFieldOut;

/*** MODULE MASK ***/

typedef struct {
	UINT32 nModules;
	UINT32 dataPtr;
} ModuleMaskIn;

typedef struct {
	UINT32 cmdLine;
	UINT32 formatterLink;
	UINT32 formatterId;
	UINT32 detLine;
//AKDEBUG	UINT32 name[SIZEOF_NAME]; /* interpreted as a character string including null termination */
} ModuleMask;

typedef struct {
	UINT32 moduleId;
	ModuleMask moduleMask;
} ModuleMaskInAux;

/*** LOAD MODULE CONFIG ***/

enum {
	MODULE_CONFIG_FORMAT_LEGACY = 0, /* want to keep as zero so it is the default if noone specifies */
	MODULE_CONFIG_FORMAT_NATIVE
};

typedef struct {
	UINT32 cfgSize;
	UINT32 cfgId;
	UINT32 module;
	UINT32 format;
	UINT32 dataPtr;
} LoadModuleConfigIn;

typedef struct {
	UINT32 dataPtr; /* returns address of configuration data */
	UINT32 cfgSize; /* size of configuration structure */
} LoadModuleConfigOut;

/*** READ MODULE CONFIG ***/

enum {
	CONFIG_SOURCE_MODULE,
	CONFIG_SOURCE_MEMORY,
	CONFIG_IDENTIFIER
};


/*** SEND SERIAL STREAM ***/

typedef struct {
	UINT32 sportId, nBits, fCapture, nCount;
	UINT32 dataPtr, maskLo, maskHi;
} SendSerialStreamIn;

/*** SEND MODULE CONFIGURATION ***/

typedef struct {
	UINT32 moduleId;
	UINT32 cfgId;
	UINT32 chipMask;
	UINT32 bitMask;
} SendModuleConfigIn;

typedef struct {
	UINT32 length;
	UINT32 dataPtr;
} SendModuleConfigOut;

/*** SET LEMO ***/

typedef struct {
	UINT32 fmtLink;
} SetLemoIn;

/*** WRITE FLASH ***/

typedef struct {
	UINT32 dataPtr;
} WriteFlashIn;

/*** ECHO MEMORY ***/

typedef struct {
	UINT32 nWords, dataPtr;
} EchoIn;

/*** INMEMS ***/

typedef struct {
	UINT32 fRead, length, fifoId, dataPtr;
} RwFifoIn;

enum {
	FIFO_INMEMS, /* both inmems */
	FIFO_INMEMA,
	FIFO_INMEMB,
	FIFO_EVTMEMS, /* evtmems A & B */
	FIFO_EVTMEMA,
	FIFO_EVTMEMB,
	FIFO_EVTMEMC
};

/*** LOAD SLAVE CODE ***/

typedef struct {
	UINT32 slaveMask;
	UINT32 dataPtr;
} LoadSlaveCodeIn;

/*** QUICK TRANSFER ***/
typedef struct {
	UINT32 src, nWords;
} QuickXferIn;

typedef struct {
	UINT32 control, dst0, dst1;
} QuickXferOut;


/*** START TASK ***/

enum {
	DSP_THIS = 0, /* used to identify current processor */
	DSP_MASTER,
	DSP_SLAVE0,
	DSP_SLAVE1,
	DSP_SLAVE2,
	DSP_SLAVE3
};

typedef struct {
	UINT32 id;
	UINT32 idMinor;
	UINT32 where;
	UINT32 dataPtr;
} StartTaskIn;

typedef struct {
	UINT32 task;
	UINT32 status;
} StartTaskOut;

/*** SEND TRIGGER ***/
typedef struct {
	UINT32 groupId;
	UINT32 calDelay; /* if !0 this is delay between CAL and TRIGGER */
} SendTriggerIn;

/*** SETUP GROUP ***/
typedef struct {
	UINT32 id; /* id for either module or slave */
} SetupGroupInAux;

typedef struct {
	UINT32 groupId;
	UINT32 nModules;
	UINT32 nSlaves;
	UINT32 dataPtr;
} SetupGroupIn;

/*** EVENT TRAP SETUP ***/

typedef struct {
	UINT32 group;
	UINT32 dataPtr;
} EventTrapSetupIn;

/*** LOAD DATA FRAME ***/

typedef struct {
	UINT32 slaveMask;
	UINT32 dataPtr;
} LoadDataFrameIn;

/*** EXPERT MODE ***/

enum {
	SPORT0_MODE,
	SPORT1_MODE,
	INTERRUPT_SLAVE,
	ROUTER_ADDR,
	UART_SRC    // set the source for the ROD USB UART. 0 = PPC, 1 = slave0, 2 = slave1
};

enum {
	SPORT_MODE_NORMAL,
	SPORT_MODE_DEBUG
};

typedef struct {
	UINT32 item, arg[4];
} ExpertIn;

/*** FIND LINK ***/

/* link < N_LINKS searches the specified link. link = N_LINKS searches all links */
typedef struct {
	UINT32 link;
} FindLinkIn;

typedef struct {
	UINT32 detLine[96];
} FindLinkOut;

/*** MCC EVENT BUILD ***/

typedef struct {
	UINT32 module;
	UINT32 chip;
	UINT32 nData;
	UINT32 dataPtr;
} MccEventBuildIn;

/*** SET ROD MODE ***/

enum {
	ROD_MODE_DATA_TAKING,
	ROD_MODE_DATA_TAKING_SPORT0_CONFIGURATION,
	ROD_MODE_CALIBRATION,
	ROD_MODE_SPORT0_SPORT1_CONFIGURATION,
	ROD_MODE_SETUP,
	ROD_MODE_SPORT0_SPORT1_ACTIVE_FOR_TRIGGER,
	ROD_MODE_TEST
};

typedef struct {
	UINT32 mode;
} SetRodModeIn;

/*** Read/Write MCC ***/

typedef struct {
	UINT32 module;
	UINT32 fRead;
	UINT32 reg;
	UINT32 data;
} RwMCCIn;

typedef struct {
	UINT32 nRegs;
	UINT32 dataPtr;
} RwMCCOut;

/*** Talk To Task ***/

typedef struct {
	UINT32 task;
	UINT32 topic;
	UINT32 item;
} TalkTaskIn;

typedef struct {
	UINT32 length;
	UINT32 dataPtr;
} TalkTaskOut;

/*** Host Primitives ***/

typedef struct {
	UINT32 mode;
	UINT32 dataPtr; /* filename */
} OpenFileIn;

typedef struct {
	UINT32 item;
	UINT32 nArgs;
	UINT32 dataPtr;
} ConfigureEnvironmentIn;

typedef struct {
	UINT32 status;
} ConfigureEnvironmentOut;

enum {
	SET_VERBOSE_LEVEL = 0
};

//basic common primitive structres for Ibldsp Code and Newdsp code (originally been in primXface.h)
/* header for primitive list */
typedef struct {
	UINT32 length;/* total number of words in the list payload */
	UINT32 nMsgs; /* number of primitives */
	UINT32 index;
} MsgListHead;

/* header for each primitive in list */
typedef struct {
	UINT32 length;
	UINT32 id; /* identifies primitive */
	UINT32 index; /* used for reply */
	UINT32 revision;
} MsgHead;

/* trailer for primitive list */
typedef struct {
	UINT32 length; /* should = length in the header, a check */
	UINT32 crc; /* checksum for entire message */
} MsgListTail;

/* header for reply buffer */
typedef struct {
	UINT32 length; /* total length of this reply, through trailer */
	UINT32 nMsgs; /* number of messages */
} ReplyListHead;

/* header for each reply in the buffer */
typedef struct {
	UINT32 length;
	UINT32 id; /* identifies primitive to which this is the reply */
} ReplyHead;

typedef struct {
	UINT32 length; /* an echo of the length in the header */
	UINT32 crc;
} ReplyListTail;

/* the host needs to see the available primitives */
typedef struct {
	int id, structSize;
} PrimitiveXface;

typedef struct {
	PrimitiveXface xface;
	int (*handler)(void *primData);
	char *name;
} PrimitiveEntry;

/*** READ MODULE CONFIG ***/

typedef struct {
	UINT32 moduleId;
	UINT32 source;
	UINT32 cfgId;
} ReadModuleConfigIn;


#endif
