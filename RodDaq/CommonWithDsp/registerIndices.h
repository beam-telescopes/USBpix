/************************************************************************************
 * registerIndices.h
 *
 *  synopsis: Constructs ID numbers, sequential integers starting at 0, for each
 *           register in the DSPs memory map.  LAST_ROD_REG, defined below, is the
 *           ID of the last register; it is one less than the number of registers. The
 *           IDs run from 0 to LAST_ROD_REG.  An array of structures, rodRegister[],
 *           which contains information about the registers is indexed by these IDs.
 *
 *  related files:
 *   accessRegister.c: Routines for reading and writing to the registers in the DSP
 *                     memory map and for initializing the rodRegister array with
 *                     information about each register, e.g. its register's address.
 *   rodConfiguration.h: Has constants which are specific to each ROD, e.g. is it SCT
 *                      or pixel, how many formatter FPGAs are there...
 *
 *  Damon Fasching, UW Madison                            fasching@wisconsin.cern.ch
 *  Douglas Ferguson, UW Madison   (510) 486-5230         dpferguson@lbl.gov
 *
 *  modifications/bugs    
 *   - Modified the router registers to reflect new map (changes to trapping
 *     registers, and event-based trapping).                           18.04.02 dpsf
 *   - changed the BOC section to reflect the revised map. Extra space has been
 *     inserted in the MDSP register map between and within sections to allow
 *     for future changes without having to completely re-write the test stand
 *     rwRegField text (and any existing primitives & lists on the test stand which
5~ *     use rwRegField. Thus, the first register of each section is defined as (for ex.):
 *     #define RTR_TRAP_CMND_0(slv)       (17 + EVT_MEM_STATUS + slv)  -- 16 spare
 *     indices have been inserted between the EFB registers and the router registers. 
 *     If extra registers are needed in any section, they should be put in the
 *     appropriate spare slot; the definition of the first register of the next
 *     section should be left AS IS. Just check to make sure that the spare space
 *     has not been exceeded (highly unlikely, since changes from now on should be
 *     minor).                                                         24.04.02 dpsf
 *   - Updated the formatter registers here and in accessRegister.c to
 *     match the new formatter VHDL code.                             02.04.03 dpsf
 *   - Updated the registers here and in accessRegister.c to
 *     reflect a few new ROD registers & bit re-definitions.          04.04.03 dpsf
 ************************************************************************************/
#ifndef REGISTER_INDICES
#define REGISTER_INDICES

#include "rodConfiguration.h"

#define NUM_FE_OCC_REGS      3
#define NUM_DATA_LINK_MASKS  3
#define NUM_DM_LUTS          0xC 
#define NUM_FE_OCC_STAT_REGS 8
#define N_MODE_BITS          2
#define N_ROD_MASK_LUTS      8
#define N_ROD_SP             2
#define N_ROD_SP_REG         2
/* Formatter FPGA registers */

#define FMT_LINK_EN(fmt)              (0x400000 + fmt*0x400)
#define FMT_EXP_MODE_EN(fmt)          (4 +(FMT_LINK_EN(fmt)))
#define FMT_CONFIG_MODE_EN(fmt)       (4 +(FMT_EXP_MODE_EN(fmt)))
#define FMT_EDGE_MODE_EN(fmt)         (4 +(FMT_CONFIG_MODE_EN(fmt)))
#define FMT_READOUT_TIMEOUT(fmt)      (4 +(FMT_EDGE_MODE_EN(fmt)))
#define FMT_DATA_OVERFLOW_LIMIT(fmt)  (4 +(FMT_READOUT_TIMEOUT(fmt)))
#define FMT_HEADER_TRAILER_LIMIT(fmt) (4 +(FMT_DATA_OVERFLOW_LIMIT(fmt)))
#define FMT_ROD_BUSY_LIMIT(fmt)       (4 +(FMT_HEADER_TRAILER_LIMIT(fmt)))
#define FMT_PXL_LINK_L1A_CNT(fmt)     (4 +(FMT_ROD_BUSY_LIMIT(fmt)))
#define FMT_PXL_BANDWIDTH(fmt)        (4 +(FMT_PXL_LINK_L1A_CNT(fmt)))
#define FMT_LINK_DATA_TEST_MUX(fmt)   (20 +(FMT_PXL_BANDWIDTH(fmt)))
/* +4*8 reserved IDs */
#define FMT_MB_DIAG_REN(fmt)          (4 +(FMT_LINK_DATA_TEST_MUX(fmt)))
#define FMT_LINK_OCC_CNT(fmt,lnk)     (4 +(FMT_MB_DIAG_REN(fmt) +lnk*4))

#define FMT_TIMEOUT_ERR(fmt)          (4 +(FMT_LINK_OCC_CNT(fmt,(LINKS_PER_FORMATTER-1))))
#define FMT_DATA_OVERFLOW_ERR(fmt)    (4 +(FMT_TIMEOUT_ERR(fmt)))
#define FMT_HEADER_TRAILER_ERR(fmt)   (4 +(FMT_DATA_OVERFLOW_ERR(fmt)))
#define FMT_ROD_BUSY_ERR(fmt)         (4 +(FMT_HEADER_TRAILER_ERR(fmt)))

#define FMT_DATA_FMT_STATUS(fmt)      (4 +(FMT_ROD_BUSY_ERR(fmt)))
#define FMT_STATUS(fmt)               (4 +(FMT_DATA_FMT_STATUS(fmt)))
#define FMT_VERSION(fmt)              (4 +(FMT_STATUS(fmt)))
#define FMT_MODEBIT_STAT(fmt)         (4 +(FMT_VERSION(fmt)))
#define FMT_MODEBIT_STAT_05(fmt)      (4 +(FMT_VERSION(fmt)))
#define FMT_MODEBIT_STAT_6B(fmt)      (4 +(FMT_MODEBIT_STAT_05(fmt)))


/* Event Fragment Builder FPGA registers */
#define EFB_ERROR_MASK(efb, lnk) (0x402000 + efb*0x100 + (lnk/12)*0x40 + (lnk%12)*4)
#define FORMAT_VRSN          (20 + EFB_ERROR_MASK((EFBS_PER_ROD-1),(DATA_LINKS_PER_EFB-1)))
#define SOURCE_ID            (4 + FORMAT_VRSN)
#define RUN_NUMBER           (4 + SOURCE_ID)

#define EFB_CMND_0           (8 + RUN_NUMBER)
#define EFB_FORMATTER_STAT   (4 + EFB_CMND_0)
#define EFB_RUNTIME_STAT_REG (4 + EFB_FORMATTER_STAT)
#define EFB_CODE_VERSION     (4 + EFB_RUNTIME_STAT_REG)
#define EVENT_HEADER_DATA    (4 + EFB_CODE_VERSION)
#define EV_FIFO_DATA1        (4 + EVENT_HEADER_DATA)
#define EV_FIFO_DATA2        (4 + EV_FIFO_DATA1)

#define EVT_MEM_MODE         (24 + EV_FIFO_DATA2)
#define EVT_MEM_CMND_STAT    (4 + EVT_MEM_MODE)
#define EVT_MEM_RESET        (4 + EVT_MEM_CMND_STAT)
#define EVT_MEM_FLAGS        (4 + EVT_MEM_RESET)
#define EVT_MEM_A_WRD_CNT    (4 + EVT_MEM_FLAGS)
#define EVT_MEM_B_WRD_CNT    (4 + EVT_MEM_A_WRD_CNT)
#define EVT_MEM_PLAY_EVENT   (4 + EVT_MEM_B_WRD_CNT)
#define EVT_MEM_STATUS       (4 + EVT_MEM_PLAY_EVENT)
#define EFB_EVT_CNT          (4 + EVT_MEM_STATUS)
#define EFB_BANDWIDTH_CNT    (4 + EFB_EVT_CNT)
/*#define EFB_CODE_VERSION     (4 + EFB_BANDWIDTH_CNT)*/

/* Router FPGA registers- note that addresses in router are done in 4 blocks
 * (1 block per slv) of CMD0(slv), CMD1(slv), .... INT_DELAY _CNT, ..., while here
 * they are indexed (used in the rodRegister structure in accessRegister.c) as
 * CMD0(0), ... CMD0(3), CMD1(0)...  The addressing for rodRegister is done in
 * accessRegister.c  dpsf. */
#define RTR_TRAP_CMND_0(slv)       (0x402400 + slv*0x40)
#define RTR_TRAP_CMND_1(slv)       (4 + RTR_TRAP_CMND_0(slv))
#define RTR_TRAP_RESET(slv)        (4 + RTR_TRAP_CMND_1(slv)) 
#define RTR_TRAP_STATUS(slv)       (4 + RTR_TRAP_RESET(slv))
#define RTR_TRAP_MATCH_0(slv)      (4 + RTR_TRAP_STATUS(slv))
#define RTR_TRAP_MOD_0(slv)        (4 + RTR_TRAP_MATCH_0(slv))
#define RTR_TRAP_MATCH_1(slv)      (4 + RTR_TRAP_MOD_0(slv))
#define RTR_TRAP_MOD_1(slv)        (4 + RTR_TRAP_MATCH_1(slv))
#define RTR_TRAP_XFR_FRM_SIZE(slv) (4 + RTR_TRAP_MOD_1(slv))
#define RTR_TRAP_FIFO_WRD_CNT(slv) (4 + RTR_TRAP_XFR_FRM_SIZE(slv))
                   /* 1 unused half-word/slv in router, reserved here (1 word/hw) */
#define RTR_TRAP_EVT_CNT(slv)      (4 + RTR_TRAP_FIFO_WRD_CNT(slv))
#define RTR_TRAP_INT_DELAY_CNT(slv)(8 + RTR_TRAP_EVT_CNT(slv))
                      /* 3 unused hw before beginning of next slave address block */
#define RTR_CMND_STAT              (16 + RTR_TRAP_INT_DELAY_CNT((N_SDSP-1)))
#define RTR_SLNK_ATLAS_DUMP_MATCH  (4 + RTR_CMND_STAT)
#define RTR_SLNK_ROD_DUMP_MATCH    (4 + RTR_SLNK_ATLAS_DUMP_MATCH)

#define RTR_CODE_VERSION           (4 + RTR_SLNK_ROD_DUMP_MATCH)
#define RTR_OUTPUT_SIGNAL_MUX      (4 + RTR_CODE_VERSION)

/* ROD resources interface FPGA (RRIF) registers */
#define RRIF_CODE_VERSION         (0x404400)
#define RRIF_CMND_1               (16 + RRIF_CODE_VERSION)
#define RRIF_CMND_0               (4 + RRIF_CMND_1)
#define ROD_MODE_REG              (4 + RRIF_CMND_0)
#define FE_MASK_LUT_SELECT        (4 + ROD_MODE_REG)
#define RRIF_STATUS_1             (4 + FE_MASK_LUT_SELECT)
#define RRIF_STATUS_0             (4 + RRIF_STATUS_1)
#define FE_CMND_MASK_0_LO         (12 + RRIF_STATUS_0)
#define FE_CMND_MASK_0_HI         (4 + FE_CMND_MASK_0_LO)
#define FE_CMND_MASK_1_LO         (4 + FE_CMND_MASK_0_HI)
#define FE_CMND_MASK_1_HI         (4 + FE_CMND_MASK_1_LO)
#define CALSTROBE_DELAY           (4 + FE_CMND_MASK_1_HI)
#define CAL_CMND                  (4 + CALSTROBE_DELAY) 
#define ECR_COUNTER_VAL           (4 + CAL_CMND)
#define FRMT_RMB_STATUS           (8 + ECR_COUNTER_VAL)
#define EFB_DM_FIFO_FLAG_STA      (8 + FRMT_RMB_STATUS)
#define EFB_DM_WC_STA_REG         (4 + EFB_DM_FIFO_FLAG_STA)
#define INP_MEM_CTRL              (4 + EFB_DM_WC_STA_REG)
#define DBG_MEM_CTRL              (4 + INP_MEM_CTRL)
#define CFG_READBACK_CNT          (4 + DBG_MEM_CTRL)
#define IDE_MEM_CTRL              (8 + CFG_READBACK_CNT)
#define IDE_MEM_STAT              (4 + IDE_MEM_CTRL)
#define INTRPT_TO_SLV             (28 + IDE_MEM_STAT)
#define INTRPT_FROM_SLV           (4 + INTRPT_TO_SLV)

#define FE_OCC_CNTR_RESET(msk)    (12 + INTRPT_FROM_SLV + msk*4) 
#define FE_OCC_CNTR_LOAD(msk)     (4 + FE_OCC_CNTR_RESET((NUM_FE_OCC_REGS-1)) + msk*4)
#define FE_OCC_CNTR_NUM_ACC(msk)  (8 + FE_OCC_CNTR_RESET((NUM_FE_OCC_REGS-1)) + msk*4)
//#define FE_OCC_LOAD_VALUE         (1 + FE_OCC_CNTR_LOAD(NUM_FE_OCC_REGS-1))
#define DATA_LINK_MASK(msk)       (4 + FE_OCC_CNTR_NUM_ACC(3) +  msk*4)

#define CAL_L1_TRIG_TYPE_0        (8 + DATA_LINK_MASK((NUM_DATA_LINK_MASKS-1)))
#define CAL_L1_TRIG_TYPE_1        (4 + CAL_L1_TRIG_TYPE_0)
#define CAL_L1_ID_0               (12 + CAL_L1_TRIG_TYPE_1)
#define CAL_L1_ID_1               (4 + CAL_L1_ID_0)
#define CAL_BCID                  (4 + CAL_L1_ID_1)

#define FE_OCC_CNTR(occ)          (24 + CAL_BCID + occ*4)

#define CMND_MASK_LUT(lutset, sp, lohi) (256 + FE_OCC_CNTR(0) + (((N_ROD_SP*N_ROD_SP_REG)*lutset + (N_ROD_SP_REG)*sp +lohi))*4)
#define DFLT_ROD_EVT_TYPE         (132 + CMND_MASK_LUT(7,1,1))
#define DM_DFLT_LUT(mbLut)        (4 + DFLT_ROD_EVT_TYPE + mbLut*4)
#define CRTV_ROD_EVT_TYPE         (16 + DM_DFLT_LUT((NUM_DM_LUTS-1)))
#define DM_CRTV_LUT(mbLut)        (4 + CRTV_ROD_EVT_TYPE + mbLut*4)
#define CORRECTED_EVENTS_FIFO     (16 + DM_CRTV_LUT((NUM_DM_LUTS-1)))

#define RMB_DFLT_LUT(lutset, fmt, mb)  (128 + CORRECTED_EVENTS_FIFO + (0x80)*lutset  +((N_MODE_BITS)*fmt +mb)*4)

#define RMB_CRTV_LUT(lutset, fmt, mb)  (0x40 + RMB_DFLT_LUT(lutset,fmt,mb))

/* Old definitions for backward compatability; default set is #0 */
#define RMB0_DFLT_LUT(fmt)    RMB_DFLT_LUT(0, fmt, 0)
#define RMB1_DFLT_LUT(fmt)    RMB_DFLT_LUT(0, fmt, 1)
#define RMB0_CRTV_LUT(fmt)    RMB_CRTV_LUT(0, fmt, 0)
#define RMB1_CRTV_LUT(fmt)    RMB_CRTV_LUT(0, fmt, 1)


/* BOC (optocard) registers */
#define FIRST_BOC_REG             (0x408000)

#define STREAM_INHIBIT_MASK(clnk) (FIRST_BOC_REG + clnk*4)
#define MARK_SPACE(clnk)          (4 + STREAM_INHIBIT_MASK((CTRL_LINKS_PER_ROD-1)) + clnk*4)
#define COARSE_DELAY(clnk)        (4 + MARK_SPACE((CTRL_LINKS_PER_ROD - 1)) + clnk*4)
#define FINE_DELAY(clnk)          (4 + COARSE_DELAY((CTRL_LINKS_PER_ROD - 1)) + clnk*4)

#define LASER_CURR_DAC(clnk)      (4 + FINE_DELAY((CTRL_LINKS_PER_ROD - 1)) + clnk*4)
#define IN_LINK_DATA_DELAY(lnk)   (4 + LASER_CURR_DAC((CTRL_LINKS_PER_ROD - 1)) + lnk*4)

#define BPM_CLOCK_PHASE           (4 + IN_LINK_DATA_DELAY((DATA_LINKS_PER_ROD-1)))
#define BREG_CLOCK_PHASE          (12 + BPM_CLOCK_PHASE)
#define VERNIER_CLOCK_STEP_PHASE0 (4 + BREG_CLOCK_PHASE)
#define VERNIER_CLOCK_STEP_PHASE1 (4 + VERNIER_CLOCK_STEP_PHASE0)

#define STROBE_DELAY(sdlnk)       (12 + VERNIER_CLOCK_STEP_PHASE1 + (sdlnk*16 ))
#define IN_DATA_RX_THRESH_DAC(lnk) (FIRST_BOC_REG + 0xc00 + lnk*4)

#define BOC_RESET                 (FIRST_BOC_REG +0xf00)
#define BPM_RESET                 (4 + BOC_RESET)
#define TX_DAC_CLEAR              (4 + BPM_RESET)
#define RX_DAC_CLEAR              (4 + TX_DAC_CLEAR)
#define BOC_STATUS                (4 + RX_DAC_CLEAR)
#define RX_DATA_MODE              (4 + BOC_STATUS)
#define VERNIER_CLOCK_FINE_PHASE  (12 + RX_DATA_MODE)
#define CLOCK_CONTROL_BITS        (8 + VERNIER_CLOCK_FINE_PHASE)

#define BOC_FIRMWARE_VERSION      (24 + CLOCK_CONTROL_BITS)
#define BOC_HARDWARE_VERSION      (4 + BOC_FIRMWARE_VERSION)
#define BOC_MODULE_TYPE           (4 + BOC_HARDWARE_VERSION)
#define BOC_MANUFACTURER          (4 + BOC_MODULE_TYPE)
#define BOC_SERIAL_NUMBER         (20 + BOC_MANUFACTURER)

#define LAST_BOC_REG              (BOC_SERIAL_NUMBER)
#define LAST_ROD_REG              (LAST_BOC_REG)

/* Define the fields (widths and locations) in each register and their legal values. */

/* FMT_READOUT_TIMEOUT */
#define FMT_READOUT_TIMEOUT_O          0
#define FMT_READOUT_TIMEOUT_W          8

/* FMT_DATA_OVERFLOW_LIMIT */
#define FMT_DATA_OVERFLOW_LIMIT_O      0
#define FMT_DATA_OVERFLOW_LIMIT_W      9

/* FMT_HEADER_TRAILER_LIMIT */
#define FMT_HEADER_TRAILER_LIMIT_O     0
#define FMT_HEADER_TRAILER_LIMIT_W     5

/* FMT_ROD_BUSY_LIMIT */
#define FMT_ROD_BUSY_LIMIT_O           0
#define FMT_ROD_BUSY_LIMIT_W           8

/* FMT_PXL_LINK_L1A_CNT */
#define FMT_PXL_LINK_W                 4
#define FMT_PXL_LINK0_O                0
#define FMT_PXL_LINK1_O                4
#define FMT_PXL_LINK2_O                8
#define FMT_PXL_LINK3_O                12

/* FMT_PXL_BANDWIDTH */
#define FMT_PXL_BANDWIDTH_W            2
#define FMT_PXL_BANDWIDTH_40MHZ        0
#define FMT_PXL_BANDWIDTH_80MHZ        1
#define FMT_PXL_BANDWIDTH_160MHZ       2

/* FMT_LINK_DATA_TEST_MUX */
#define FMT_LINK_DATA_TEST_MUX_O       0
#define FMT_LINK_DATA_TEST_MUX_W       4

/* FMT_LINK_OCC_CNT */
#define FMT_LINK_OCC_CNT_O             0
#define FMT_LINK_OCC_CNT_W             9

/* FMT_STATUS */
#define FMT_STAT_TRIG_CNT_O            0
#define FMT_STAT_TRIG_CNT_W            5
/* bit 5 not used */
#define FMT_STAT_LINK_MB_FIFO_NE_O     6
#define FMT_STAT_LINK_MB_FIFO_FULL_O   7
#define FMT_STAT_ACTIVE_LINK_O         8
#define FMT_STAT_ACTIVE_LINK_W         4
#define FMT_STAT_CHIP_HAS_TOKEN_O     12
#define FMT_STAT_HOLD_OUTPUT_O        13

#define FMT_STAT_MASTER_SLAVE_O       14
#define FMT_STAT_DLL_LOCKED_O         15

/* FMT_VERSION */
#define FMT_CODE_VERSION_O            0
#define FMT_CODE_VERSION_W            8
#define FMT_BOARD_VERSION_O           8
#define FMT_BOARD_VERSION_W           8


/* EFB_ERROR_MASK: */
#define EFB_HEADER_BIT_ERR_O               0
#define EFB_TRAILER_BIT_ERR_O              1
#define EFB_FLAGGED_ERR_O                  2
#define EFB_SYNC_ERR_O                     3
#if defined(SCT_ROD)
	#define EFB_CONDENSED_MODE_PATTERN_ERR_O   4
#elif defined(PIXEL_ROD)
	#define EFB_MCC_ERR_O                      4
#endif
#define EFB_L1ID_ERR_O                     5
#define EFB_BCID_ERR_O                     6
#define EFB_TIMEOUT_ERR_O                  7
#define EFB_ALMOST_FULL_ERR_O              8
#define EFB_DATA_OVERFLOW_ERR_O            9
#define EFB_NONSEQUENTIAL_CHIP_ERR_O       10
#if defined(SCT_ROD)
	#define EFB_INVALID_CHIP_ERR_O             11
#elif defined(PIXEL_ROD)
	#define EFB_INVALID_ROW_ERR_O              11
#endif
#if defined(SCT_ROD)
#elif defined(PIXEL_ROD)
	#define EFB_INVALID_COLUMN_ERR_O           12
	#define EFB_MCC_EMPTY_EVT_ERR_O            13
#endif

#define EFB_DYNAMIC_MASK_STATUS_O          14
#define EFB_DYNAMIC_MASK_STATUS_W          2

/* EFB_CMND_0 */
#define EFB_SEND_EVENTS_O      0
#define EFB_MASK_BCID_O        1
#define EFB_GROUP_COUNTER_EN_O 2
#define EFB_MASK_L1ID_0        3
#define EFB_DATA_LINK_SEL_O    4
#define EFB_DATA_LINK_SEL_W    4
#define EFB_BCID_OFFSET_O      8
#define EFB_BCID_OFFSET_W      8

/* EFB_FORMATTER_STAT  */ 
#define EFB_FORMATTER_STAT_O 0
#define EFB_FORMATTER_STAT_W 8    /* one bit for each formatter FPGA */

/*EFB_RUNTIME_STAT_REG*/
#define  FIFO_1_ALMOST_FULL_O          0x0
#define  ERR_SUM_FIFO_1_ALMOST_FULL_O  0x1
#define  EV_ID_FIFO_EMP_ERR1_O         0x2
#define  FIFO1_PAUSE_TO_FORMATTER_O    0x3
#define  FIFO_2_ALMOST_FULL_O          0x4
#define  ERR_SUM_FIFO_2_ALMOST_FULL_O  0x5
#define  EV_ID_FIFO_EMP_ERR2_O         0x6
#define  FIFO2_PAUSE_TO_FORMATTER_O    0x7
#define  HALT_OUTPUT_FROM_ROUTER_O     0x8

/*  EVENT_HEADER_DATA  */
#define  EVENT_HEADER_DATA_O   0
#define  EVENT_HEADER_DATA_W   16

/* EV_FIFO_DATA1 */ 
#define EV_FIFO_DATA1_O    0
#define EV_FIFO_DATA1_W    12

/* EV_FIFO_DATA2 */ 
#define EV_FIFO_DATA2_O    0
#define EV_FIFO_DATA2_W    12

/* EVT_MEM_MODE (values) */
#define EVT_MEM_MODE_O    0
#define EVT_MEM_MODE_W    3
#define DATA_TAKING     0x0     /* normal data path */
#define PLAY_TO_ROUTER  0x1     /* load via rod bus, play EVENT_MEM event to router */
#define RODBUS_ACCESS   0x2     /* load via rod bus, read via rod bus */
#define TRAP_REAL_DATA  0x4     /* play through normal data path, trap in EVENT_MEM */

/* EVT_MEM_CMND_STAT */
#define EVT_MEM_SEL_O       0
#define EVT_MEM_SEL_W       3
#define A_SELECT          0x1
#define B_SELECT          0x2
#define C_SELECT          0x4
#define OUTPUT_FRAGMENT_O   3
#define OUTPUT_FRAGMENT_W   1
#define OUTPUT_FRAGMENT   0x1
#define BUS_ENABLED_O       4
#define BUS_ENABLED_W       3
#define A_BUS_ENABLED     0x1
#define B_BUS_ENABLED     0x2
#define C_BUS_ENABLED     0x4

/* EVT_MEM_RESET */
#define RESET_EVT_MEM_O   0
 
/* EVT_MEM_FLAGS */
#define EFB_EVT_MEM_A_EMPTY_O      0
#define EFB_EVT_MEM_A_AE_O         1
#define EFB_EVT_MEM_A_FULL_O       2
#define EFB_EVT_MEM_A_AF_O         3
#define EFB_EVT_MEM_B_EMPTY_O      4
#define EFB_EVT_MEM_B_AE_O         5
#define EFB_EVT_MEM_B_FULL_O       6
#define EFB_EVT_MEM_B_AF_O         7
#define EFB_EVT_MEM_C_EMPTY_O      8
#define EFB_EVT_MEM_C_FULL_O       9

/* EFB_EVT_CNT */
#define EFB_EVT_CNT_ID(x)   (4*x)
#define EFB_EVT_CNT_ID_W     4

/* EFB_BANDWIDTH_CNT */
#define EFB_BANDWIDTH_CNT_DATA_0  0
#define EFB_BANDWIDTH_CNT_TIME_0  16
#define EFB_BANDWIDTH_CNT_W       16

/* EVT_MEM_PLAY_EVENT */
#define PLAY_EVENT_O  0





/* RTR_TRAP_CMND_0/1(slv) (only bit 0-4 in 1) */
#define RTR_TRAP_ATLAS_EVT_TYPE_O   0
#define RTR_TRAP_TIM_EVT_TYPE_O     1
#define RTR_TRAP_ROD_EVT_TYPE_O     2
#define RTR_TRAP_ERROR_FMT_O        3
#define RTR_TRAP_EXCLUSION_FLAG_O   4
#define RTR_TRAP_ALL_EVT_SLINK_O    5
#define RTR_TRAP_DATA_MODE_O        6

/* RTR_TRAP_RESET(slv) */
#define RTR_TRAP_RESET_O      0
#define RTR_TRAP0_LOAD_NEW_O  1
#define RTR_TRAP1_LOAD_NEW_O  2

/* RTR_TRAP_STATUS(slv) */
#define RTR_TRAP_ENABLE_FROM_DSP_O   0  /* READ ONLY */
#define RTR_INTERRUPT_OK_FROM_DSP_O  1  /* READ ONLY */
#define RTR_TRAP_FIFO_EMPTY_O        2  /* READ ONLY */
#define RTR_TRAP_FIFO_FULL_O         3  /* READ ONLY */
#define RTR_TRAP0_IDLE_O             4  /* READ ONLY */
#define RTR_TRAP1_IDLE_O             5  /* READ ONLY */
#define RTR_TRAP0_READY_O            6  /* READ ONLY */
#define RTR_TRAP1_READY_O            7  /* READ ONLY */
#define RTR_SLAVE_CLK_SYNC_O         8  /* READ ONLY */

/* RTR_TRAP_MATCH_X(slv) (X= 0 or 1) */
#define RTR_TRAP_MATCH_O      0 
#define RTR_TRAP_MATCH_W      8

/* RTR_TRAP_MOD_X(slv) */
#define RTR_TRAP_MODULUS_O    0 
#define RTR_TRAP_MODULUS_W    8
#define RTR_TRAP_REMAINDER_O  8 
#define RTR_TRAP_REMAINDER_W  8

/* RTR_TRAP_XFR_FRM_SIZE(slv) */ 
#define RTR_TRAP_XFR_FRM_SIZE_O  0
#define RTR_TRAP_XFR_FRM_SIZE_W  10

/* RTR_TRAP_FIFO_WRD_CNT(slv) */  
#define RTR_TRAP_FIFO_WRD_CNT_O  0
#define RTR_TRAP_FIFO_WRD_CNT_W  10

/* RTR_TRAP_EVT_CNT(slv)   */ 
#define  RTR_TRAP_EVT_CNT_O      0
#define  RTR_TRAP_EVT_CNT_W      16

/* RTR_TRAP_INT_DELAY_CNT(slv) */ 
#define  RTR_TRAP_INT_DELAY_CNT_O       0
#define  RTR_TRAP_INT_DELAY_CNT_W       6   /* delay= 25 ns per count */

/* RTR_CMND_STAT */
#define RTR_DUMP_ATLAS_EVT_TYPE_O       0
#define RTR_DUMP_TIM_EVT_TYPE_O         1
#define RTR_DUMP_ROD_EVT_TYPE_O         2
#define RTR_INHIBIT_SLNK_WE_O           3
#define RTR_RESET_SLNK_O                4
#define RTR_SET_SLNK_TEST_O             5
#define RTR_CALIB_BACK_PRES_EFB_O       6
#define RTR_SLINK_DOWN_OVERRIDE_O       7
/* bit 7 not used */
#define RTR_SLNK_XOFF_STAT_O            8
#define RTR_SLKN_BAD_STAT_O             9
#define RTR_STOP_OUTPUT_O              10
#define RTR_CLK_DLL_LOCKED_O           11
 
/* RTR_SLNK_ATLAS_DUMP_MATCH */
#define RTR_ATLAS_EVT_TYPE_O  0
#define RTR_ATLAS_EVT_TYPE_W  8     /* ATLAS SPECIFIC EVENT TYPE FOR MATCH */
#define RTR_TIM_EVT_TYPE_O    8
#define RTR_TIM_EVT_TYPE_W    2     /* TIM SPECIFIC EVENT TYPE FOR MATCH*/
 
/* RTR_SLNK_ROD_DUMP_MATCH  */  
#define  RTR_SLNK_ROD_DUMP_MATCH_O   0
#define  RTR_SLNK_ROD_DUMP_MATCH_W  16
 

/* BOC Registers  */
/*BOC  STREAM_INHIBIT_MASK(clnk) */  
#define  BOC_STREAM_INHIBIT_O  0
#define  BOC_STREAM_INHIBIT_W  1
/*BOC  MARK_SPACE_(clnk) */ 
#define  MARK_SPACE_O  0
#define  MARK_SPACE_W  6
/*BOC  COARSE_DELAY(clnk) */ 
#define  COARSE_DELAY_O         0
#define  COARSE_DELAY_W         5
/*BOC  FINE_DELAY(clnk) */ 
#define  FINE_DELAY_O           0
#define  FINE_DELAY_W           7

/* LASER_CURR_DAC(clnk)  */
#define  LASER_CURR_DAC_O 0
#define  LASER_CURR_DAC_W 8
/* IN_LINK_DATA_DELAY(lnk)*/
#define  IN_LINK_DATA_DELAY_O  0
#define  IN_LINK_DATA_DELAY_W  8

/* BPM_CLOCK_PHASE */
#define  BPM_CLOCK_PHASE_O     0
#define  BPM_CLOCK_PHASE_W     8
/* BREG_CLOCK_PHASE */
#define  BREG_CLOCK_PHASE_O     0
#define  BREG_CLOCK_PHASE_W     8
/* VERNIER_CLOCK_STEP_PHASE0 */
#define  VERNIER_CLOCK_STEP_PHASE0_O     0
#define  VERNIER_CLOCK_STEP_PHASE0_W     8
/* VERNIER_CLOCK_STEP_PHASE1 */
#define  VERNIER_CLOCK_STEP_PHASE1_O     0
#define  VERNIER_CLOCK_STEP_PHASE1_W     8

/*   STROBE_DELAY(sdlnk) */           
#define   STROBE_DELAY_O        0
#define   STROBE_DELAY_W        8
/* IN_DATA_RX_THRESH_DAC(lnk)  */ 
#define   IN_DATA_RX_THRESH_O    0
#define   IN_DATA_RX_THRESH_W    8

/* BOC_RESET  */
#define    BOC_RESET_O       0             
#define    BOC_RESET_W       1
/* BOC_STATUS */
#define    BOC_STATUS_O      0              
#define    BOC_STATUS_W      8     /* READ ONLY */
/* BPM_RESET  */
#define    BPM_RESET_O       0             
#define    BPM_RESET_W       1
/* TX_DAC_CLEAR */
#define    TX_DAC_CLEAR_O    0                
#define    TX_DAC_CLEAR_W    1
/* RX_DAC_CLEAR */
#define    RX_DAC_CLEAR_O    0                
#define    RX_DAC_CLEAR_W    1
/* RX_DATA_MODE */
#define    RX_DATA_MODE_O    0                
#define    RX_DATA_MODE_W    3
/* VERNIER_CLOCK_FINE_PHASE */
#define    VERNIER_CLOCK_FINE_PHASE_O   0                 
#define    VERNIER_CLOCK_FINE_PHASE_W   8
/* CLOCK_CONTROL_BITS */
#define    CLOCK_CONTROL_BITS_O   0                 
#define    CLOCK_CONTROL_BITS_W   4

/* BOC_FIRMWARE_VERSION  */
#define    BOC_FIRMWARE_VERSION_O   0                  
#define    BOC_FIRMWARE_VERSION_W   8    /* READ ONLY */
/* BOC_HARDWARE_VERSION  */
#define    BOC_HARDWARE_VERSION_O   0                  
#define    BOC_HARDWARE_VERSION_W   8    /* READ ONLY */
/* BOC_MODULE_TYPE  */
#define    BOC_MODULE_TYPE_O   0                  
#define    BOC_MODULE_TYPE_W   8         /* READ ONLY */
/* BOC_MANUFACTURER  */
#define    BOC_MANUFACTURER_O   0                  
#define    BOC_MANUFACTURER_W   8        /* READ ONLY */
/* BOC_SERIAL_NUMBER  */         
#define    BOC_SERIAL_NUMBER_O  0                 
#define    BOC_SERIAL_NUMBER_W  8        /* READ ONLY */



/* RRIF_CMND_0 */
#define FMT_MB_RST_OUT_O                 0 /*  formatter mode bits reset */
#define FMT_MB_RST_OUT_W                 2 
#define EFB_EDM_RST_OUT_O                2 /*  efb dynamic mask bits reset */
#define INP_MEM_RST_O                    3
#define DBG_MEM_A_RST_O                  4
#define DBG_MEM_B_RST_O                  5 
#define TIM_FIFO_RST_O                   6
#define TRIG_FIFO_RST_O                  7 
#define TRIG_FIFO_RETRANSMIT_O           8
#define INMEM_FIFO_RETRANSMIT_O          9
#define DBGMEMA_FIFO_RETRANSMIT_O        10
#define DBGMEMB_FIFO_RETRANSMIT_O        11
#define ECR_ID_COUNTER_RESET_O           12
#define SP1_FRAME_SYNC_OFFSET_O          13
#define SP1_FRAME_SYNC_OFFSET_W          5
#define FSX_CLKX_OUTPUT_ENABLE_O         18
//bits 19-23 not used
#define SLOW_SDSP_CLOCK_ENABLE           24

#define MDSP_TOUT_ENABLE                 25
#define MDSP_INT4_ENABLE                 26
#define MDSP_INT5_ENABLE                 27
#define MDSP_INT6_ENABLE                 28
#define MDSP_INT7_ENABLE                 29
#define SDSP_INT_ENABLE                  30
#define VME_INT_ENABLE                   31

/* RRIF_CMND_1 */  
#define FE_CMND_OUTPUT_ENABLE_O          0
#define FE_SP0_FR_DSP_TIM_O              1 
#define FE_SP0_FR_DSP                    1
#define FE_SP0_FR_TIM                    0
#define NEW_MASK_READY_O                 2  /* self clearing */
#define FE_OCC_CNTR_O                    3  /* FE occupancy counter */
#define CMD_PULSE_CTR_RESET_O            4  
#define CMD_PULSE_CTR_ENABLE_O           5
#define CMD_PULSE_CTR_LOAD_O             6
#define TRG_DECODER_RESET_O              7
#define TRG_DECODER_ENABLE_O             8
#define FORM_RMB_FLUSH_O                 9
#define FORM_RMB_ENABLE_XFR_O           10
#define EFB_DYN_MSK_FLUSH_O             11
#define EFB_DYN_MSK_ENABLE_XFR_O        12
#define EFB_DYN_MSK_EVT_HDR_CNT_LD_O    13
#define EFB_DYN_MSK_EVT_MSK_CNT_LD_O    14  
#define TEST_BENCH_RESET_O              15
#define TEST_BENCH_ENABLE_O             16
#define TEST_BENCH_RUN_O                17
#define SP_TRIGGER_SIGNAL_DECODER_EN_O  18  /* 0=> Off, 1=> Enabled */
#define CONFIGURATION_READBACK_O        19  /* 0=> Off, 1=> Enabled */
#define FE_MASK_LOAD_ENABLE_O           20  /* 0=> Off, 1=> Enabled */
#define STATIC_BCID_ENABLE_O            21  /* 0=> Off, 1=> Enabled */
#define STATIC_L1ID_ENABLE_O            22  /* 0=> Off, 1=> Enabled */
#define CMB_DYN_MASK_RDY_O              23  /* self clearing */

#define INPUT_FIFO_PLAY_INHIBIT_O       24

#define FIFO_CTRL_MUX_O                 25 /* U FIFO (in, debug, event) MUX control */
#define FIFO_CTRL_MUX_W                 1 /* U */
#define FIFO_RESET                   0    /* U FIFO rst:debug FIFO block control */
#define ROD_BUS_FIFO_ACCESS          1    /* U allows FIFO access over the ROD bus */

#define DATA_PATH_SELECT_O              26 /* U data path mode selector */
#define DATA_PATH_SELECT_W               2 /* U */
#define TEST_BENCH_DATA_PATH          1    /* enable test bench data path */
#define STANDARD_DATA_PATH            2    /* enable run time data path */

#define ROD_TYPE_O                      31
#define ROD_TYPE_SCT                0
#define ROD_TYPE_PIXEL              1 

/* RRIF_STATUS_1 */
#define  TIM_CLK_OK_O         0  /* TRUE IS OK*/
#define  BOC_CLK_OK_O         1  /* TRUE IS OK*/
#define  BOC_BUSY_O           2  /*  TRUE IS BUSY */
#define  CFG_READBACK_DONE_O  3
#define  CAL_TEST_RDY_O       4
#define  TRIG_FIFO_EF_O       5 
#define  TRIG_FIFO_FF_O       6
#define  RMB_FIFOA_EF_O       7  /* FORM MODEBIT FIFOA EMPTY FLAG */
#define  RMB_FIFOA_FF_O       8  /* FORM MODEBIT FIFOA FULL FLAG */
#define  RMB_FIFOB_EF_O       9  /* FORM MODE FULL FLAG,BIT 0 BANK A,BIT 1 BANK B */
#define  RMB_FIFOB_FF_O       10 /* FULL IS TRUE */
#define  HEADER_TRAIL_LMT_O   11 /* FORM TRAILER LIMIT,BIT 0 BANK A,BIT 1 BANK B*/
#define  HEADER_TRAIL_LMT_W 2    /* LIMIT IS TRUE */
#define  ROD_BUSY_O           13 /* ROD BUSY,BIT 0 BANK A, BIT 1 BANK B */
#define  ROD_BUSY_W         2    /* BUSY IS TRUE */
#define  DM_FIFO_EF_O         15 /* DYN MASK FIFO EMPTY FLAG, EMPTY IS TRUE */
#define  DM_FIFO_FF_O         16 /* DYN MASK FIFO FULL FLAG, FULL IS TRUE */
#define  EFB_EV_ID_EMP_ERR_O  17 /* EFB EVENT EMPTY FLAG, ERROR IS TRUE 
                                    (FORMATTER PLAYS DATA BEFORE ID IN EFB) */
#define  EVT_MEM_A_EMP_O      18 /* EVENT MEMORY A EMPTY, EMPTY IS TRUE */
#define  EVT_MEM_A_FULL_O     19 /* EVENT MEMORY A FULL, FULL IS TRUE */
#define  EVT_MEM_B_EMP_O      20 /* EVENT MEMORY B EMPTY, EMPTY IS TRUE */
#define  EVT_MEM_B_FULL_O     21 /* EVENT MEMORY B FULL, FULL IS TRUE */
#define  FE_CMD_PULSE_CNT_O   22 /* CNT # TRIG DATA,IT HAS NOT SENT TO FORMATTERS */
#define  FE_CMD_PULSE_CNT_W    8
#define  FE_OCC_CNTRS_EF_O    30 /* TRUE IF ALL FRONT END COUNTER ARE ZERO */
#define  MODE_BITS_ERR        31 

/* RRIF_STATUS_O */
#define  RS0_ROD_TYPE_O           31 
#define  RS0_DSP_PRESENT_O(slv)   (27 +slv) 

/* FE_CMND_MASK_LO     */ 
#define  FE_CMD_MASK_LO_O      0 /* COMMAND MASK 0 FOR LINKS 0(BIT 0) TO 31 */
#define  FE_CMD_MASK_LO_W     32

/* FE_CMND_MASK_HI     */
#define  FE_CMD_MASK_HI_O      0 /* COMMAND MASK 0 FOR LINKS 32(BIT 0) TO 47(BIT 15)*/
#define  FE_CMD_MASK_HI_W     16

/* CALSTROBE_DELAY       */
#define  CALSTROBE_DELAY_O     0 /* DELAY IN CLK40 TICK FROM TEST PULSE TO TRIG GEN*/
#define  CALSTROBE_DELAY_W     6

/* CAL_CMND              */
#define  CAL_CMD_O             0 /* REG THAT IS SHIFTED OUT SERIAL ON COMD ???*/
#define  CAL_CMD_W            26

/* FRMT_RMB_STATUS    */
#define  MB_FIFO_A_EMP_O       0
#define  MB_FIFO_A_FULL_O      1
#define  MB_FIFO_A_WC_O        2
#define  MB_FIFO_A_WC_W        8
                           /* 10 TO 15 NOT USED  */
#define  MB_FIFO_B_EMP_O      16
#define  MB_FIFO_B_FULL_O     17
#define  MB_FIFO_B_WC_O       18
#define  MB_FIFO_B_WC_W        8 
                           /* 26 TO 31 NOT USED */ 

/* EFB_DM_FIFO_FLAG_STA  */
#define  L1ID_BCID_FIFO_EMP_O            0  /*L1 & BC ID FIFO EMPTY, EMPTY IS TRUE*/
#define  TRIG_TYP_FIFO_EMP_O             1  /*TRIG TYP FROM TIM FIFO EMP. EMP IS TRUE*/
#define  L1ID_BCID_TT_FIFO_EMP_O         2  /*FIFO OF ABOVE 21 FIFO EMP. EMP IS TRUE*/
#define  DEFAULT_DYN_MASK_FIFO_EMP_O     3  /*DYN MASK FIFO EMP, EMP IS TRUE*/
#define  CORR_TRIG_DYN_MASK_FIFO_EMP_O   4  /*CORR DNT MASK FIFO EMP, EMP IS TRUE */
//bits 5-7 not used.
#define  L1ID_BCID_FIFO_FULL_O           8  /*L1 & BC ID FIFO FULL, FULL IS TRUE*/
#define  TRIG_TYP_FIFO_FULL_O            9  /*TRIG TYP FROM TIM FIFO FULL. FULL IS TRUE*/
#define  L1ID_BCID_TT_FIFO_FULL_O        10 /*FIFO OF ABOVE 21 FIFO FULL. FULL IS TRUE*/
#define  DFLT_MASK_FIFO_FULL_O           11 /*DEFAULT DYN MASK FIFO FULL, FULL IS TRUE*/
#define  CORR_TRIG_DYN_MASK_FIFO_FULL_O  12 /*CORR DYN MASK FIFO FULL, FULL IS TRUE */

/* EFB_DM_WC_STA_REG     */                         
#define  EVENT_ID_COUNT_O       0  /* WORD COUNT FOR EVENT ID FIFO  */
#define  EVENT_ID_COUNT_W       6
//bits 6-7 not used
#define  EVENT_TRIG_COUNT_O     8  /* WORD COUNT FOR EVENT TRIG FIFO  */
#define  EVENT_TRIG_COUNT_W     6
//bits 14-15 not used
#define  EVENT_MASK_COUNT_O    16  /* WORD COUNT FOR EVENT DYN MASK FIFO  */
#define  EVENT_MASK_COUNT_W     6
//bits 22-23 not used
#define  EVENT_HEADER_COUNT_O  24  /* WORD COUNT FOR EVENT HEADER FIFO  */
#define  EVENT_HEADER_COUNT_W   6
 
/* INP_MEM_CTRL          */
#define  INMEM_A_WC_O           0  /*INMEM A WC OR GEN PURPOSE DEBUG COUNTER */
#define  INMEM_A_WC_W          16
#define  INMEM_B_WC_O          16  /*INMEM B WC OR GEN PURPOSE DEBUG COUNTER */
#define  INMEM_B_WC_W          16

/* DBG_MEM_CTRL          */
#define  DBGMEM_A_WC_O          0  /*DBGMEM A WC OR GEN PURPOSE DEBUG COUNTER */
#define  DBGMEM_A_WC_W         16
#define  DBGMEM_B_WC_O         16  /*DBGMEM B WC OR GEN PURPOSE DEBUG COUNTER */
#define  DBGMEM_B_WC_W         16

/* CFG_READBACK_CNT          */ 
#define  FIFO_WRITE_CNT_O        0  /* FIFO writing time, in clock ticks. */
#define  FIFO_WRITE_CNT_W        16
#define  DELAY_FIFO_WRITE_O      16  /* Delay in ticks before the FIFO begins trapping data, */
#define  DELAY_FIFO_WRITE_W      16  /* when the cfg readback bit is set in RRIF CMD 1 */

/* IDE_MEM_CTRL          */
#define  EN_INMEM_A_COUNT_O     0 /* ENABLE INPUT MEMORY A COUNTER */
#define  LOAD_INMEM_A_COUNT_O   1 /* LOAD INPUT MEMORY A COUNTER */
#define  EN_INMEM_B_COUNT_O     2 /* ENABLE INPUT MEMORY B COUNTER */
#define  LOAD_INMEM_B_COUNT_O   3 /* LOAD INPUT MEMORY B COUNTER */
#define  EN_DBGMEM_A_COUNT_O    4 /* ENABLE DEBUG MEMORY A COUNTER */
#define  LOAD_DBGMEM_A_COUNT_O  5 /* LOAD DEBUG MEMORY A COUNTER */
#define  EN_DBGMEM_B_COUNT_O    6 /* ENABLE DEBUG MEMORY B COUNTER */
#define  LOAD_DBGMEM_B_COUNT_O  7 /* LOAD DEBUG MEMORY B COUNTER */
#define  EN_EVTMEM_A_COUNT_O    8 /* ENABLE EVENT MEMORY A COUNTER */
#define  LOAD_EVTMEM_A_COUNT_O  9 /* LOAD EVENT MEMORY A COUNTER */
#define  EN_EVTMEM_B_COUNT_O   10 /* ENABLE EVENT MEMORY B COUNTER */
#define  LOAD_EVTMEM_B_COUNT_O 11 /* LOAD EVENT MEMORY B COUNTER */
#define TEST_FIXTURE_MODE_O    12 /* SELECTION OF TEST BENC MODES */
#define TEST_FIXTURE_MODE_W     6 /* LIST OF MODES ????????      */

/* IDE_MEM_STAT */
#define INP_MEM_A_DONE_O                  0  /* IN MEM COUNTER, ZERO IS TRUE */
#define INP_MEM_B_DONE_O                  1  /* IN MEM COUNTER, ZERO IS TRUE */
#define DBG_MEM_A_DONE_O                  2  /* IN MEM COUNTER, ZERO IS TRUE */
#define DBG_MEM_B_DONE_O                  3  /* IN MEM COUNTER, ZERO IS TRUE */
#define CFG_READBACK_WRT_CNT_DONE_O       4
#define CFG_READBACK_DELAY_CNT_DONE_O     5
#define OPERATION_DONE_O                  6  /* TEST BENCH OPER COMPLETE, COMP IS TRUE*/
#define MEM_OP_DONE_W                     2  /*  00 AND 11 IS ONLY VALUE USED */  
#define INMEM_A_EMP_O                     8
#define INMEM_A_FULL_O                    9
#define INMEM_B_EMP_O                     10
#define INMEM_B_FULL_O                    11
#define DBGMEM_A_EMP_O                    12
#define DBGMEM_A_FULL_O                   13
#define DBGMEM_B_EMP_O                    14
#define DBGMEM_B_FULL_O                   15
#define TIM_FIFO_EF_O                     16  /*  INTERN TIM FIFO EMPTY, EMPTY IS TRUE*/
#define TIM_FIFO_FF_O                     17  /*  INTERN TIM FIFO FULL, FULL IS TRUE*/
#define TIM_FIFO_WC_O                     18  /*  INTERN TIM FIFO CURR WORD COUNT */ 
#define TIM_FIFO_WC_W                     13  

/* INTRPT_TO_SLV         */ 
#define MASTER_TO_SLV_INT_O    0  /* MASTER TO SLAVE HPI INT,BIT 0 IS SLAVE 1,*/ 
#define MASTER_TO_SLV_INT_W    4  /* BIT 3 IS SLAVE 4 ??????? */

/* INTRPT_FROM_SLV       */
#define SLAVE_TO_MAST_INT_O    0
#define SLAVE_TO_MAST_INT_W    4

/* FE_OCC_CNTR_RESET(msk) */
#define FE_OCC_CNTR_RST_O      0  /* RESET FOR 32 LINKS */
#define FE_OCC_CNTR_RST_W     32

/* FE_OCC_CNTR_LOAD(msk) */
#define FE_OCC_CNTR_LOAD_O     0  /* LOAD FOR 32 LINKS */
#define FE_OCC_CNTR_LOAD_W    32

/* FE_OCC_LOAD_VALUE     */
#define FE_OCC_LOAD_VAL_O      0  /* VALUE LOADED TO FE OCC COUNTERS */
#define FE_OCC_LOAD_VAL_W      4

/* DATA_LINK_MASK(msk)   */
#define DATA_LINK_MASK_O       0
#define DATA_LINK_MASK_W      32

/* FE_OCC_CNTR(occ)     */
#define  FE_OCC_CNTR_VAL_0_O    0
#define  FE_OCC_CNTR_VAL_0_W    4
#define  FE_OCC_CNTR_VAL_1_O    4
#define  FE_OCC_CNTR_VAL_1_W    4
#define  FE_OCC_CNTR_VAL_2_O    8
#define  FE_OCC_CNTR_VAL_2_W    4
#define  FE_OCC_CNTR_VAL_3_O   12
#define  FE_OCC_CNTR_VAL_3_W    4
#define  FE_OCC_CNTR_VAL_4_O   16
#define  FE_OCC_CNTR_VAL_4_W    4
#define  FE_OCC_CNTR_VAL_5_O   20
#define  FE_OCC_CNTR_VAL_5_W    4
#define  FE_OCC_CNTR_VAL_6_O   24
#define  FE_OCC_CNTR_VAL_6_W    4
#define  FE_OCC_CNTR_VAL_7_O   28
#define  FE_OCC_CNTR_VAL_7_W    4

/* RMB0_DFLT_LUT(fmt)    */
#define  FMAT_MODE_BIT0_LINK_0_O    0
#define  FMAT_MODE_BIT0_LINK_1_O    1
#define  FMAT_MODE_BIT0_LINK_2_O    2
#define  FMAT_MODE_BIT0_LINK_3_O    3
#define  FMAT_MODE_BIT0_LINK_4_O    4
#define  FMAT_MODE_BIT0_LINK_5_O    5
#define  FMAT_MODE_BIT0_LINK_6_O    6
#define  FMAT_MODE_BIT0_LINK_7_O    7
#define  FMAT_MODE_BIT0_LINK_8_O    8
#define  FMAT_MODE_BIT0_LINK_9_O    9
#define  FMAT_MODE_BIT0_LINK_10_O  10
#define  FMAT_MODE_BIT0_LINK_11_O  11
#define  FMAT_MODE_BIT1_LINK_0_O    0
#define  FMAT_MODE_BIT1_LINK_1_O    1
#define  FMAT_MODE_BIT1_LINK_2_O    2
#define  FMAT_MODE_BIT1_LINK_3_O    3
#define  FMAT_MODE_BIT1_LINK_4_O    4
#define  FMAT_MODE_BIT1_LINK_5_O    5
#define  FMAT_MODE_BIT1_LINK_6_O    6
#define  FMAT_MODE_BIT1_LINK_7_O    7
#define  FMAT_MODE_BIT1_LINK_8_O    8
#define  FMAT_MODE_BIT1_LINK_9_O    9
#define  FMAT_MODE_BIT1_LINK_10_O  10
#define  FMAT_MODE_BIT1_LINK_11_O  11

/* RMB0&1_CRTV_LUT(fmt)    */
#define  FMAT_CRTV_BIT0_LINK_0_O    0
#define  FMAT_CRTV_BIT0_LINK_1_O    1
#define  FMAT_CRTV_BIT0_LINK_2_O    2
#define  FMAT_CRTV_BIT0_LINK_3_O    3
#define  FMAT_CRTV_BIT0_LINK_4_O    4
#define  FMAT_CRTV_BIT0_LINK_5_O    5
#define  FMAT_CRTV_BIT0_LINK_6_O    6
#define  FMAT_CRTV_BIT0_LINK_7_O    7
#define  FMAT_CRTV_BIT0_LINK_8_O    8
#define  FMAT_CRTV_BIT0_LINK_9_O    9
#define  FMAT_CRTV_BIT0_LINK_10_O  10
#define  FMAT_CRTV_BIT0_LINK_11_O  11
#define  FMAT_CRTV_BIT1_LINK_0_O    0
#define  FMAT_CRTV_BIT1_LINK_1_O    1
#define  FMAT_CRTV_BIT1_LINK_2_O    2
#define  FMAT_CRTV_BIT1_LINK_3_O    3
#define  FMAT_CRTV_BIT1_LINK_4_O    4
#define  FMAT_CRTV_BIT1_LINK_5_O    5
#define  FMAT_CRTV_BIT1_LINK_6_O    6
#define  FMAT_CRTV_BIT1_LINK_7_O    7
#define  FMAT_CRTV_BIT1_LINK_8_O    8
#define  FMAT_CRTV_BIT1_LINK_9_O    9
#define  FMAT_CRTV_BIT1_LINK_10_O  10
#define  FMAT_CRTV_BIT1_LINK_11_O  11

/* DFLT_ROD_EVT_TYPE     */
#define  DFLT_ROD_EVT_TYPE_O   0
#define  DFLT_ROD_EVT_TYPE_W   16

/* DM_DFLT_LUT(mbLut)    */
#define  EFB_DF_DYN_MASK_BIT0_LNK0_O  0  /* WORD HAS SAME PAT FOR EACH LINK */
#define  EFB_DF_DYN_MASK_BIT1_LNK0_O  1   
#define  EFB_DF_DYN_MASK_BIT0_LNK1_O  2  
#define  EFB_DF_DYN_MASK_BIT1_LNK1_O  3   
#define  EFB_DF_DYN_MASK_BIT0_LNK2_O  4  
#define  EFB_DF_DYN_MASK_BIT1_LNK2_O  5   
#define  EFB_DF_DYN_MASK_BIT0_LNK3_O  6  
#define  EFB_DF_DYN_MASK_BIT1_LNK3_O  7   
#define  EFB_DF_DYN_MASK_BIT0_LNK4_O  8  
#define  EFB_DF_DYN_MASK_BIT1_LNK4_O  9   
#define  EFB_DF_DYN_MASK_BIT0_LNK5_O 10  
#define  EFB_DF_DYN_MASK_BIT1_LNK5_O 11   
#define  EFB_DF_DYN_MASK_BIT0_LNK6_O 12  
#define  EFB_DF_DYN_MASK_BIT1_LNK6_O 13   
#define  EFB_DF_DYN_MASK_BIT0_LNK7_O 14  
#define  EFB_DF_DYN_MASK_BIT1_LNK7_O 15   

/* CRTV_ROD_EVT_TYPE     */
#define  CRTV_ROD_EVT_TYPE_O   0
#define  CRTV_ROD_EVT_TYPE_W   16

/* DM_CRTV_LUT(mbLut)    */
#define  EFB_CR_DYN_MASK_BIT0_LNK0_O  0  /* WORD HAS SAME PAT FOR EACH LINK */
#define  EFB_CR_DYN_MASK_BIT1_LNK0_O  1   
#define  EFB_CR_DYN_MASK_BIT0_LNK1_O  2  
#define  EFB_CR_DYN_MASK_BIT1_LNK1_O  3   
#define  EFB_CR_DYN_MASK_BIT0_LNK2_O  4  
#define  EFB_CR_DYN_MASK_BIT1_LNK2_O  5   
#define  EFB_CR_DYN_MASK_BIT0_LNK3_O  6  
#define  EFB_CR_DYN_MASK_BIT1_LNK3_O  7   
#define  EFB_CR_DYN_MASK_BIT0_LNK4_O  8  
#define  EFB_CR_DYN_MASK_BIT1_LNK4_O  9   
#define  EFB_CR_DYN_MASK_BIT0_LNK5_O 10  
#define  EFB_CR_DYN_MASK_BIT1_LNK5_O 11   
#define  EFB_CR_DYN_MASK_BIT0_LNK6_O 12  
#define  EFB_CR_DYN_MASK_BIT1_LNK6_O 13   
#define  EFB_CR_DYN_MASK_BIT0_LNK7_O 14  
#define  EFB_CR_DYN_MASK_BIT1_LNK7_O 15  

/* CORRECTED_EVENTS_FIFO */
#define  CRTV_EVNT_FIFO_O  0
#define  CRTV_EVNT_FIFO_W 16

/* CAL_L1_TRIG_TYPE */
#define  CAL_L1_TRIG_TYPE_O     0
#define  CAL_L1_TRIG_TYPE_W    10
/* CAL_L1_ID */
#define  CAL_L1_ID_O            0
#define  CAL_L1_ID_W           24

#endif
