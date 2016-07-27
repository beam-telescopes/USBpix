/************************************************************************************
 * comRegDfns.h:  Defines bits in the communications registers of the the MDSP and
 *                SDSPs.
 *
 *  modifications:    
 *   - Removed the CommRegs structure from accessCommRegs.h, placed it here and
 *     deleted the file. Removed the comm. regs #defines from memoryPartitions.h
 *     and placed them here. This gives DAQ one file (this one) to include in their
 *     programs which, in conjunction with the ROD memory map structure (accessible
 *     via the MEMORY_MAP_REG), gives information on all the important areas of
 *     MDSP & SDSP memory.                                             26.02.04 dpsf
 *
 *  -  Created a new compatibility structure for transparent migration of the
 *     SCT code.                                                       21.10.04 dpsf
 ************************************************************************************/
#ifndef COM_REG_DFNS
#define COM_REG_DFNS

#include "txtBuffer.h"             /* pick up N_TXT_BUFFS */
#include "rodConfiguration.h"      /* pick up N_SDSP      */

/* DSP Register Map Summary:
STATUS_REG_0     STATUS_REG_1     LIST_STATUS_REG  LIST_CNT_REG  
COMMAND_REG      TASK_STATE_REG   MEMORY_MAP_REG   TIME_REG      
DIAGNOSTIC_REG   HCMD_REG         HSTATUS_REG_0    HSTATUS_REG_1 
HSSTAT_REG_0     HSSTAT_REG_1     HSSTAT_REG_2     HSSTAT_REG_3  

//dpsf: move trapping regs closer together.
TRAP_CMD_REG     TRAP_REQ_REG     TRAPSTAT_REG_0   TRAPSTAT_REG_1
RESERVED_REG_0   RESERVED_REG_1   RESERVED_REG_2   RESERVED_REG_3
RESERVED_REG_4   RESERVED_REG_5   RESERVED_REG_6   RESERVED_REG_7
RESERVED_REG_8   LOOP_REG         IDSP_HSHK        IDSP_HSHK     
*/

/* The host must know the starting address of the communication registers for
   the MDSP, so that it can find & use the memory map register: */
#if defined(I_AM_HOST)
	#define IDREGS_BASE  0x80000000
#endif

/* The CommRegs structure contains all the communications registers on the ROD.
   The DSP prepares a copy with the location of all the communication registers'
   addresses for inter-DSP & host use. Other instances can contain the values of
   the registers (after downloading from the ROD, for example). */

#if defined(SCT_ROD)
	#include "comRegDfns_11x.h"

#elif defined(PIXEL_ROD)
typedef struct {
	UINT32 status[2];
	UINT32 listStatus;
	UINT32 listCnt;
	
	UINT32 command;
	UINT32 taskState;
	UINT32 memoryMap;
	UINT32 time;
	
	UINT32 diagnostic;
	UINT32 hCmd;
	UINT32 hStat[2];
	
	UINT32 hSdspStat[4];
	
	UINT32 trapCmd;
	UINT32 trapReq;
	UINT32 trapStat[2];
	
	UINT32 reservedReg[9];
	UINT32 loop;

/* Inter-dsp communication registers: used for the handshake between master and
 * slave DSPs when one sends a list for the other to process.  These registers
 * reside in the IDRAM of each DSP.  The addresses are known to both the master
 * and the slaves.  Note that the write register for the SDSP is the read
 * register for the MDSP and vice versa. */
	#if (defined(I_AM_HOST))
		UINT32 privateDsp[2];
	#elif (defined(I_AM_MASTER_DSP))
		UINT32 interDspHshkWr, interDspHshkRd;
	#elif (defined(I_AM_SLAVE_DSP))
		UINT32 interDspHshkRd, interDspHshkWr;
	#endif

} CommRegs;

//communication register addresses:
/* +0x00 */
#define STATUS_REG_0     (IDREGS_BASE)
#define STATUS_REG_1     ((STATUS_REG_0)    + 4)
#define LIST_STATUS_REG  ((STATUS_REG_1)    + 4)
#define LIST_CNT_REG     ((LIST_STATUS_REG) + 4)

/* +0x10 */
#define COMMAND_REG      ((LIST_CNT_REG)    + 4)
#define TASK_STATE_REG   ((COMMAND_REG)     + 4)
#define MEMORY_MAP_REG   ((TASK_STATE_REG)  + 4)
#define TIME_REG         ((MEMORY_MAP_REG)  + 4)

#define STATUS_REG_2  STATUS_REG_1
#define COMMAND_REG_0 COMMAND_REG

/* +0x20 */
#define DIAGNOSTIC_REG   ((TIME_REG)        + 4)
#define HCMD_REG         ((DIAGNOSTIC_REG)  + 4)
#define HSTATUS_REG_0    ((HCMD_REG)        + 4)
#define HSTATUS_REG_1    ((HSTATUS_REG_0)   + 4)

#define HCMD_STAT_REG_0   (HCMD_REG)
#define HSTAT_REG_0       (HSTATUS_REG_0)
#define HSTAT_REG_1       (HSTATUS_REG_1)
#define HCMD_STAT_REG_1   (TIME_REG)

/* +0x30 */
#define HSSTAT_REG_0     ((HSTATUS_REG_1)   + 4)
#define HSSTAT_REG_1     ((HSSTAT_REG_0)    + 4)
#define HSSTAT_REG_2     ((HSSTAT_REG_1)    + 4)
#define HSSTAT_REG_3     ((HSSTAT_REG_2)    + 4)

//The SDSPs use these registers as general status registers.
#if ((defined(I_AM_MASTER_DSP)) || (defined(I_AM_HOST)))
	#define SDSP_HSTATUS_REG_2   (HSSTAT_REG_0)
	#define SDSP_HSTATUS_REG_3   (HSSTAT_REG_1)
	#define SDSP_HSTATUS_REG_4   (HSSTAT_REG_2)
	#define SDSP_HSTATUS_REG_5   (HSSTAT_REG_3)

#elif (defined(I_AM_SLAVE_DSP))
	#define HSTATUS_REG_2   (HSSTAT_REG_0)
	#define HSTATUS_REG_3   (HSSTAT_REG_1)
	#define HSTATUS_REG_4   (HSSTAT_REG_2)
	#define HSTATUS_REG_5   (HSSTAT_REG_3)
#endif

/* +0x40 */
#define TRAP_CMD_REG     ((HSSTAT_REG_3)    + 4)
#define TRAP_REQ_REG     ((TRAP_CMD_REG)    + 4)
#define TRAPSTAT_REG_0   ((TRAP_REQ_REG)    + 4)
#define TRAPSTAT_REG_1   ((TRAPSTAT_REG_0)  + 4)
/* dpsf temporary for backwards compatibility */
#define TRAP_CMD_STAT    (TRAP_CMD_REG)

/* +0x50 */
#define RESERVED_REG_0   ((TRAPSTAT_REG_1)  + 4)
#define RESERVED_REG_1   ((RESERVED_REG_0)  + 4)
#define RESERVED_REG_2   ((RESERVED_REG_1)  + 4)
#define RESERVED_REG_3   ((RESERVED_REG_2)  + 4)

/* +0x60 */
#define RESERVED_REG_4   ((RESERVED_REG_3)  + 4)
#define RESERVED_REG_5   ((RESERVED_REG_4)  + 4)
#define RESERVED_REG_6   ((RESERVED_REG_5)  + 4)
#define RESERVED_REG_7   ((RESERVED_REG_6)  + 4)

/* +0x70 */
#define RESERVED_REG_8   ((RESERVED_REG_7)  + 4)
#define LOOP_REG         ((RESERVED_REG_8)  + 4)

/* IDSP handshake registers: */
#if defined(I_AM_HOST)
	#define PRIVATE_DSP_0 ((LOOP_REG)         + 4)
	#define PRIVATE_DSP_1 ((PRIVATE_DSP_0)    + 4)
#elif defined(I_AM_MASTER_DSP)
	#define IDSP_HSHK_WR  ((LOOP_REG)         + 4)
	#define IDSP_HSHK_RD  ((IDSP_HSHK_WR)     + 4)
#elif defined(I_AM_SLAVE_DSP)
	#define IDSP_HSHK_RD  ((LOOP_REG)         + 4)
	#define IDSP_HSHK_WR  ((IDSP_HSHK_RD)     + 4)
#endif

#endif //SCT or Pixel ROD

/*********************** Register Bitfield Definitions ******************************/
/* Main Status Register (0): */
#define SR_RUNNING              0
#define SR_BUSY                 1
#define SR_EXECUTING            2
#define SR_PAUSED               3

#define SR_OUT_LIST_RDY         4
#define SR_DSP_ACK              5
#define SR_DMA_ACCESS_ACK       6
#define SR_DMA_ACCESS_ERR       7

#define SR_TXT_BUFF_NE(x)      (8  + (x))
#define SR_TXT_BUFF_PROC(x)    (12 + (x))

#define SR_NTASKS               16
#define SR_NTASKS_W             4

#define SR_IDLS_ACTIVE          20
#define SR_EVM                  21  /* SDSP only */
#define SR_IDLP_ACTIVE          21  /* MDSP only: for SDSP in CR */
#define SR_LISTERR              22
#define SR_LISTFATAL            23

//dpsf: add when new text buff free up bits #define SR_TXT_BUFF_OVF(x)      (24 +(x))
#define SR_CACHE_ACTIVE         27

#define SR_DSP_WAITING          28
#define SR_INMEM_BUFF_ACK       29
#define SR_INIT_ERROR           30
#define SR_EMIF_ERROR           31

/* MDSP/SDSP Comm. Status Register (1): */
#define SR_SLVCOMM(x)             (x)
#define SR_HLTS(x)                (4 + (x))
#define SR_SLVATTEND               8

/* List Status Register: one INT32 field */

#if defined(SCT_ROD)
/* STATUS_REG_1 */
#define SR_PRIM_LIST_INDEX      0
#define SR_PRIM_LIST_INDEX_W    4
#define SR_PRIM_INDEX           4
#define SR_PRIM_INDEX_W         20

/* List Count Register (Status Register 1): */
#define LCR_PRIM_INDEX      (SR_PRIM_INDEX)
#define LCR_PRIM_INDEX_W    (SR_PRIM_INDEX_W)
#define LCR_ERR_INDEX       24           
#define LCR_ERR_INDEX_W     8
#define LCR_LIST_INDEX      (SR_PRIM_LIST_INDEX)
#define LCR_LIST_INDEX_W    (SR_PRIM_LIST_INDEX_W)

#elif defined(PIXEL_ROD)
/* List Count Register: */
#define LCR_PRIM_INDEX      0
#define LCR_PRIM_INDEX_W    12
#define LCR_ERR_INDEX       12           
#define LCR_ERR_INDEX_W     12
#define LCR_LIST_INDEX      24
#define LCR_LIST_INDEX_W    8

#endif

/* Command Register: */
#define CR_IN_LIST_RDY            0
#define CR_PAUSE                  1
#define CR_RESUME                 2
#define CR_ABORT                  3
#define CR_TXT_BUFF_RR(x)        (4 + (x))
#define CR_RESET_INMEM_BUFF       8    /* MDSP: reset & clear the inmem buffer */
#define CR_DMA_ACCESS_REQ         10
#define CR_FLUSH_EVENTS           11   /* SDSPs */
#define CR_IDLP_ACTIVE            12   /* SDSPs */
#define CR_ID                     28
#define CR_ID_W                   2    /* IDs for SDSPs, assigned by MDSP */

#define CR_HISTO_OVERRIDE         13   /* SDSPs */

/* Task State Register */
#define TASK_STATE(x)            (4*x)
#define TASK_STATE_W              4

/* Memory Map Register: UINT32 * pointer */
/* Time & Loop Registers: one field */

/* Diagnostic Register: */
#define DR_DISP_ROD_REG_ID       0   //MDSP
#define DR_LOOPEVENT             0   //SDSP
#define DR_DISP_SERIAL_LOOPS     1
#define DR_USE_ROD_MASK_LUT      2   //MDSP
#define DR_HISTO_1               2   //SDSP
#define DR_HISTO_3               3   //SDSP
#define DR_SCAN_LEMO             3   //MDSP
#define DR_DISP_MIRROR_CTIME     4
#define DR_SCAN_DELAY            5   //MDSP

#define DR_STEP_CTRL             6
#define DR_STEP_TRIGGER          7
#define DR_PASSON                8
#define DR_SOFT_BC_RESET         9
#define DR_HTYPE_REGSET          10
#define DR_HISTO_CHIPOCC         11
#define DR_MAINLOOP_TOGGLE       12

#define DR_PROCWAIT              13
#define DR_PULSE_PAUSE           14
#define DR_PULSEWAIT             15  //MDSP
#define DR_HISTO_ASM             15
#define DR_INFO_DISPLAY          16
#define DR_MODULE_CFG_LOOP       17
#define DR_SLAVE_WRITE_TEST      18
#define DR_CACHE_FLUSH           17 //SDSP
#define DR_CACHE_RANGE_FLUSH     18 //SDSP
#define DR_CACHE_TOGGLE          19 //SDSP

#define DR_ISR_DELAY             19
#define DR_DISP_LINK_DATA        20
#define DR_AUTO_STALL            21
#define DR_STAGE_PAUSE           22
#define DR_CAL_EVT_CAPTURE       23
#define DR_BIN_PAUSE             24
#define DR_NOMASKSET             25
#define DR_TRIGGER_SERIES        26
#define DR_REGULATE_TRIGGER      27

#define DR_SPEED_220             30
#define DR_RESET_FIFOS           31

/* Histogram Command & Status Registers: */
#if (defined(I_AM_MASTER_DSP) || defined(I_AM_HOST))
	#define HCMD_EXE             0
	#define HCMD_BINADVANCE      1
	#define HCMD_STAGEADVANCE    2
	#define HCMD_SWITCHMASK      3
	#define HCMD_MASK            4
	#define HCMD_MASK_W          4
	#define HCMD_CHIPADVANCE     8
	#define HCMD_CFG_NORM        9
	#define HCMD_CFG_OFFROD      10

	#define HSTAT0_BIN           0
	#define HSTAT0_BIN_W         8
	#define HSTAT0_CHIP          8
	#define HSTAT0_CHIP_W        8
	#define HSTAT0_STAGE         16
	#define HSTAT0_STAGE_W       12
	#define HSTAT0_ACK           28

	#define HSTAT1_NERR         0
	#define HSTAT1_NERR_W       8
	#define HSTAT1_AVGTRAN      8
	#define HSTAT1_AVGTRAN_W    8
	#define HSTAT1_AVGPROC      16
	#define HSTAT1_AVGPROC_W    16

	//For compatibility:
	#define HCSR0_BIN          0
	#define HCSR0_BIN_W        8
	#define HCSR0_CHIP         8
	#define HCSR0_CHIP_W       8
	#define HCSR0_STAGE        16
	#define HCSR0_STAGE_W      12

	#define HSR_SLV_W        16
	#define HSR1_SLV1        16
	#define HSR1_SLV0        0
#endif

/* Definitions of the SDSPs' histogramming command and status registers
   are different; MDSP must know both. */
#define HCMD_SLV_BIN          0
#define HCMD_SLV_BIN_W        8
#define HCMD_SLV_NEWBIN       8

#define HCMD_SLV_STAGE        16
#define HCMD_SLV_STAGE_W      12

#define HSR0_SLV_RDY          0
#define HSR0_SLV_EXP          1
#define HSR0_SLV_PROC         2
#define HSR0_SLV_DONE         3
#define HSR0_SLV_PBIN         4
#define HSR0_SLV_HFIT         5
#define HSR0_SLV_WARN         6
#define HSR0_SLV_WARN_W       2
#define HSR0_SLV_BIN          8
#define HSR0_SLV_BIN_W        8
#define HSR0_SLV_BINERR       16
#define HSR0_SLV_BINERR_W     8

#define HSR1_SLV_NEVTBIN      0
#define HSR1_SLV_NEVTBIN_W    32

#define HSR2_SLV_NERRBIN      0
#define HSR2_SLV_NERRBIN_W    32

#define HSR3_SLV_PROCTIME     0
#define HSR3_SLV_PROCTIME_W   32

//For compatibility:
#define HCSR0_SLV_BIN          0
#define HCSR0_SLV_BIN_W        8
#define HCSR0_SLV_NEWBIN       8

#define HCSR0_SLV_STAGE        16
#define HCSR0_SLV_STAGE_W      12

#define HCSR1_SLV_BINEVT       0
#define HCSR1_SLV_BINEVT_W     32


/* Trap command-status register */
#define TCSR_TRAILER           0
#define TCSR_TRANSMIT          1
#define TCSR_HEADER            2
#define TCSR_ISR_ACTIVE        3

#define TCSR_DATA_ERROR        4
#define TCSR_HEADER_ERROR      5
#define TCSR_TRAILER_ERROR     6
#define TCSR_LINK_ERROR        7

#define TCSR_ERROR             8
#define TCSR_ISR_PENDING      11

#define TCSR_OVERFLOW_ERROR   14
#define TCSR_OVERFLOW         15

#define TCSR_ERR_COUNT        16
#define TCSR_ERR_COUNT_W       8

#define TCSR_EVT_COUNT        24
#define TCSR_EVT_COUNT_W       8

/* Trap status register 0 */
#define TSR0_EVT_WORD_CNT        0
#define TSR0_EVT_WORD_CNT_W     16

#define TSR0_IFRAME_TAIL        16
#define TSR0_IFRAME_TAIL_W      8
#define TSR0_XFRAME_TAIL        24
#define TSR0_XFRAME_TAIL_W      8

/* Trap status register 1 */
#define TSR1_IFRAME_HEAD        16
#define TSR1_IFRAME_HEAD_W      8
#define TSR1_XFRAME_HEAD        24
#define TSR1_XFRAME_HEAD_W      8

/* IDSP_HSHK_WR and IDSP_HSHK_RD */
/* The following bits are defined in each of these registers.  They are used for the
 * handshake between the master and a slave when either sends a list to the other.
 * The state machine is simpler than for host-originated lists, e.g. there is no
 * option to pause, it is processed all at once, and a simple status is given. */
#define IDSP_IN_LIST_RDY     0
#define IDSP_ABORT           1
#define IDSP_ACK             2
#define IDSP_OUT_LIST_RDY    3
#define IDSP_LIST_ERR        4
#define IDSP_LIST_FATAL      5

#endif /* Multiple inclusion protection */
