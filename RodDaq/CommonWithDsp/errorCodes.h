/************************************************************************************
 * errorCodes.h
 *
 *   synopsis: Defines return codes, error severity levels and prototypes for
 *             error handling routines.
 *
 *  Damon Fasching, UW Madison                            fasching@wisconsin.cern.ch
 *  Douglas Ferguson, UW Madison   (510) 486-5230         dpferguson@lbl.gov
 *  
 *  modifications    
 *   - added error code for an unavailable global address register.  13.03.02 dpsf
 *   - added error codes for setMem & memoryTest                     17.03.02 dpsf
 *   - added BOC busy errors                                         20.03.02  rcj
 *   - removed the two error macros from smSendTxtBuff.c  (ADD_SEVERITY and
 *     MERGE_ERROR) and put them inside errorCodes.h so that other routines
 *     can use them.                                                 22.03.02  dpsf
 *
 ************************************************************************************/
#ifndef ERROR_CODES
#define ERROR_CODES

#include "processor.h"

/************************
 *  return codes
 ************************/

/* Successful completion of routine */
#define SUCCESS                   0x0

/* Host errors: */
#ifdef I_AM_HOST
	//For the primitive list reading machinery:
	#define BAD_LIST_IO_VERSION    -1
	#define BAD_PRIMITIVE_ID       -2
	#define LIST_ELEMENT_ERR       -3
	#define LIST_CREATION_ERR      -4
	#define LIST_MODIFICATION_ERR  -5
	#define LIST_STATUS_ERR        -6
	#define LIST_ATTACH_ERR        -7
	#define LIST_MAX_ERR           -8
	#define PRIM_FILE_ERR          -9
	#define PRIM_VERSION_ERR       -10
#endif

/* Boot errors (MDSP only) */
#ifdef I_AM_MASTER_DSP
  #define CODE_EXCEEDS_ROM          0x100
  #define CODE_LENGTH_MISMATCH      0x101
  #define CODE_LOAD_ERROR           0x102
  #define FLASH_VERSION_MISMATCH    0x103
#endif

/* initialization errors */
#define EMIF_INIT_ERROR           0x200

/* primitive and reply message list handling errors */
#define PRIM_LIST_REV_MISMATCH    0x300
#define PRIM_LIST_TOO_LONG        0x301
#define PRIM_LIST_LENGTH_MISMATCH 0x302
#define CHECKSUM_ERROR            0x303
#define END_OF_LIST_ADDR_ERR      0x304
#define SLV_REPLY_ERR             0x305

/* primitive and reply message handling errors */
#define PRIM_REV_NOT_SET          0x400
#define PRIM_REV_MISMATCH         0x401
#define ILLEGAL_PRIM_ID           0x402
#define PRIM_FNCT_PTR_ERROR       0x403
#define PRIM_EXCEEDS_LIST_BOUND   0x404
#define DATA_EXCEEDS_BUFFER       0x405

/* primitive input parameter errors */
#define MEMORY_EXCEEDED           0x500
#define SLAVE_DSP_DNE             0x501
#define SLAVE_DSP_OFF             0x502
#define TRAP_PARAM_ERROR          0x503
#define SLAVE_TXT_BUFF_DNE        0x504
#define ROD_REG_DNE               0x505
#define FIFO_DNE                  0x506
#define FIFO_BANK_DNE             0x507
#define FRAME_SZ_LIMIT_ERR        0x508
#define TRAP_CONFIG_ERROR         0x509
#define BAD_EVENT_TRAP_PARAMS     0x50a
#define BACK_PRESSURE_ILLEGAL     0x50b
#define SERIAL_DATA_ILLEGAL       0x50c
#define HISTOGRAM_SETUP_ERROR     0x50d
#define PRIM_PARAMETER_ERROR      0x50e
#define TASK_INSERTION_ERROR      0x50f
#define TASK_REMOVAL_ERROR        0x510
#define INVALID_CONFIG_SET        0x511
#define INVALID_MODULE_NUM        0x512
#define INVALID_CMD_STREAM_BUF    0x513
#define LIMIT_EXCEEDED            0x514
#define BAD_MODULE_DATA           0x515
#define UNRECOGNIZED_DATA         0x516

/* task manager errors*/
#define TASK_PRIORITY_MAX_ERROR   0x580
#define TASK_DUPLICATE_ERROR      0x581
#define TASK_PRIORITY_ERROR       0x582
#define TASK_NONEXISTANT_ERROR    0x583
#define TASK_REVISION_ERROR       0x584
#define TASK_NOTRUNNING_ERROR     0x585
#define TASK_INCOMPLETE_ERROR     0x586
#define TASK_NOTRUN_ERROR         0x587
#define TASK_CODING_ERROR         0x588

/* task-related errors */
#define NO_HISTOGRAMS_ERROR       0x5c0
#define HISTOGRAMMING_ERROR_EVT   0x5c1
#define UNEXPECTED_EVENT          0x5c2
#define EVENT_DISCARDED           0x5c3
#define TIMING_ERROR              0x5c4
#define ACCUM_ERROR               0x5c5
#define HEADER_TIME_OUT_ERR       0x5c6
#define HEADER_ERR                0x5c7
#define TRANSMIT_ERR              0x5c8  /* non-fatal */
#define EVENT_ERR                 0x5c9  /* non-fatal */
#define EVENT_RETRANSMIT_ERR      0x5ca
#define TRIGGER_ERR               0x5cb

/* data analysis errors (slaves only) */
#ifdef I_AM_SLAVE_DSP
  #define MISPLACED_EVFRAG_HDR      0x5d0
  #define NOT_PKT_HEADER            0x5d1
#endif

/* misc errors */
#define TIMEOUT_ERR               0x600
#define NO_TIMER_CHAN_AVAIL       0x601
#define NO_CNT_RLD_REG_AVAIL      0x602
#define NO_GBL_ADDR_REG_AVAIL     0x603
#define ARRAY_INDEX_ERR           0x604
#define BAD_ADDRESS               0x605
#define SET_MEMORY_ERROR          0x606
#define COPY_MEMORY_ERROR         0x607
#define BOC_INITIALLY_BUSY        0x608
#define BOC_CONT_BUSY             0x609
#define SLAVE_NOT_READY           0x60a
#define NON_WORD_BOUNDARY         0x60b
#define WRONG_COMMAND_ID          0x60c
#define CMD_BUFFER_OVERFLOW       0x60d
#define CMD_STREAM_BUF_RSVD       0x60e
#define BUFFER_NOT_READY          0x60f
#define NO_SLAVES_ERROR           0x610
#define PARAM_ERROR               0x611
#define DSP_CMD_LINE_ERROR        0x612
#define ROD_BUSY_ERR              0x613
#define DATA_ERR                  0x614
#define LINKMASK_ERR              0x615
#define CONFIG_ERROR              0x616
#define ASM_FORMAT_ERROR          0x617
#define ASM_BINSIZE_ERROR         0x618
#define ASM_CHIPOCC_ERROR         0x619
#define TEXT_OVERFLOW_ERROR       0x61a
#define COMPILATION_MISMATCH      0x61b
#define PRIM_HEADER_ERR           0x61c
#define PRIMLIST_ERR              0x61d
#define LIST_ABORTED              0x61e
#define BANWIDTH_MISMATCH         0x61f
#define HEADER_NOT_FOUND          0x620
#define REGISTER_COMP_ERR         0x621
#define DMA_ALLOC_ERR             0X622

/* memory test errors */
#define MEMORY_TEST_ADDR_ERROR    0x701
#define MEMORY_TEST_CB1_ERROR     0x702
#define MEMORY_TEST_CB2_ERROR     0x704
#define MEMORY_TEST_FLT0_ERROR    0x708
#define MEMORY_TEST_FLT1_ERROR    0x710
#define MEMORY_TEST_RAND_ERROR    0x720

/* The following are not errors; a primitive can return REPEAT_PRIMITIVE to execPrim()
 * to indicate that it needs to be repeated, e.g. because a resource was busy.
 * Tasks use REPEAT_TASK to indicate that they are continuing to process data. If a
 * task is halted, it exits with TASK_HALTED. */
#define REPEAT_PRIMITIVE          0x1000
#define REPEAT_TASK               0x1001
#define TASK_HALTED               0x1002
#define TASK_QUERIED              0x1003
#define TASK_INITIALIZED          0x1004

/************************************
 * severity codes and related macros
 ************************************/

/* severity codes; sign bit is not an error bit */
#define ERROR_0     28
#define ERROR_1     29
#define FATAL_ERR   30

/* mask for severity bits */
#define SEVERITY_BITS  ((FATAL_BIT) | (1 << (ERROR_1)) | 1 << (ERROR_0))

/* mask for fatal error */
#define FATAL_BIT      (1 << (FATAL_ERR))

/* this macro returns true if the fatal error bit is set in word 'x' */
#define FATAL(x)       (abs(x) & FATAL_BIT)

/*  Given an error code, 0 < y < 2**28, and a severity level, ERR_0 <= z <= FATAL_ERR,
 * this macro adds the severity bit to the error code, negates the result and stores
 * it in x, keeping any severity bits which were already present in x.  y = ERROR_n
 * for nonfatal errors (execution continues), y = FATAL_ERR for fatal errors (routine
 * exits).  This macro should only be used via the function newError (below) which
 * should be called from the function in which the error occured.  */

#define ADD_SEVERITY(x,y,z)  (x = -((abs(x) & SEVERITY_BITS) | (abs(y) | (1 << (z)))))

/*  Given two error codes, x and y, this macro adds the SEVERITY bits of y to error
 * code x.  This allows all errors to have their severity reported up the calling
 * stack, with the final error having its full code reported.  This macro should only
 * be used via the function addError (below) which should be called from a calling
 * function which receives an error return from the function called. */

#define MERGE_ERROR(x,y)      (x = -((abs(x) & SEVERITY_BITS) | abs(y)))

#endif  /* Multiple inclusion protection. */
