//File: TimDefine.h

#ifndef SCTPIXELROD_TIMDEFINE_H
#define SCTPIXELROD_TIMDEFINE_H

/*! \file
 * \brief TimDefine.h: An incomplete prototype definition of a TIM.
 *
 * NB define eg I_AM_LINUX_HOST for processor.h (eg typedef UINT32)
 *
 * Contributors: John Lane <jbl@hep.ucl.ac.uk> - originator
 *               John Hill <hill@hep.phy.cam.ac.uk> - extras for TIM 3C.
 *
 * 
 * Revision 2.1  2007/08/29 II
 *             Update according to the firmware 19:
 *             Add registers for trigger oscillator 2 periods
 *             ENABLE3 bits 
 *
 * Revision 2.0  2006/12/07 II
 *             Add Randomizer2 frequences
 *             ENABLE3 bits 
 *             add Sequencer control bits
 *             add TTC Run Enables bits
 *
 * $Id: TimDefine.h 5847 2008-08-22 07:29:49Z schsu $
 *
 * $Log$
 * Revision 1.9.2.3  2008/07/22 12:49:16  ibragimo
 * New definitions for v1b firmware
 *
 * Revision 1.9.2.2  2007/08/30 20:18:07  ibragimo
 * Registers updated to match firmware v19
 *
 * Revision 1.9.2.1  2006/12/16 01:34:33  pixeldaqdev
 * Support for LTP delay and sequencer
 *
 * Revision 1.9  2005/11/18 17:44:19  jchill
 * Merge TimRegister and TimRegister3
 *
 * Revision 1.8  2005/11/18 16:36:22  jchill
 * Do a complete update of all the register definitions for TIM-3.
 *
 * Revision 1.7  2005/11/18 16:00:22  jchill
 * Extra register definitions - needed by SlinkTest initially
 *
 *
 * Revision 1.6  2005/05/19 13:15:56  jchill
 * Changes for TIM 3C
 *
 * Revision 1.5  2004/10/01 22:44:33  jbl
 * TimModule new methods
 *
 * Revision 1.4  2003/06/04 15:04:32  tmeyer
 * Removed explicit directory structure from includes
 *
 * Revision 1.3  2002/12/11 21:30:49  jbl
 * TimModule major update
 *
 *
 *
 * Reference: http://www.hep.ucl.ac.uk/~jbl/SCT/TIM_registers.html
 */

namespace SctPixelRod {

#include "processor.h"

//! Define timing in clock cycles for SCT, Pixel is different

enum TimTimingPixel {
     TIM_L1A_DEADTIME =  3,
     TIM_ECR_DEADTIME =  7,
     TIM_BCR_DEADTIME =  7,
     TIM_CAL_DEADTIME = 27,
     TIM_BCID_OFFSET  =  8
};

const INT32 TIM_L1ID_FIRST = 0; //!< triggers

/*! Sequencer RAM is 16K bytes for both source and sink memory.
 *  They are accessed together as 16K 16-bit words.
 */
const INT32 TIM_SEQ_SIZE = 0x4000; //!< bytes
const INT32 TIM_SEQ_ADDR = 0x8000; //!< bytes

//! Define register offsets in bytes

//First definitions of a pair is the old one.
enum TimRegister {
      TIM_REG_ENABLES      = 0x00,
      TIM_REG_COMMAND      = 0x02,
      TIM_REG_BURST_COUNT  = 0x04,
      TIM_REG_BURST        = 0x04,
      TIM_REG_FREQUENCY    = 0x06,
      TIM_REG_WINDOW       = 0x08,
      TIM_REG_DELAY        = 0x0A,
      TIM_REG_DELAYS       = 0x0A,
      TIM_REG_STATUS       = 0x0C,
      TIM_REG_FIFO_STATUS  = 0x0E,
      TIM_REG_FIFO         = 0x0E,
      TIM_REG_TRIGGER_IDLO = 0x10,
      TIM_REG_L1IDL        = 0x10,
      TIM_REG_TRIGGER_IDHI = 0x12,
      TIM_REG_L1IDH        = 0x12,
      TIM_REG_TRIGGER_BCID = 0x14,
      TIM_REG_BCID         = 0x14,
      TIM_REG_TRIGGER_TYPE = 0x16,
      TIM_REG_TTID         = 0x16,
      TIM_REG_RUN_ENABLES  = 0x18,
      TIM_REG_RUN_ENABLE   = 0x18,
      TIM_REG_SEQ_CONTROL  = 0x1A,
      TIM_REG_SEQ_CTL      = 0x1A,
      TIM_REG_SEQ_END      = 0x1C,
      TIM_REG_ROD_MASK     = 0x1E,
      TIM_REG_RB_MASK      = 0x1E,
      TIM_REG_ROD_BUSY     = 0x20,
      TIM_REG_RB_STAT      = 0x20,
      TIM_REG_ROD_LATCH    = 0x22,
      TIM_REG_RB_LATCH     = 0x22,
      TIM_REG_ROD_MONITOR  = 0x24,
      TIM_REG_RB_MON       = 0x24,
      TIM_REG_TTC_DATA     = 0x26,
      TIM_REG_TTC_SELECT   = 0x28,
      TIM_REG_TTC_BCID     = 0x2A,
      TIM_REG_TTC_RX       = 0x2C,
      TIM_REG_TTCRX_CTL    = 0x2C,
      TIM_REG_TTC_STATUS   = 0x2E,
      TIM_REG_OUTPUT       = 0x30,
      TIM_REG_TIM_OUTPUT   = 0x30,
      TIM_REG_TIM_ID       = 0x32,


      TIM_REG_ENABLES3     = 0x42,

      TIM_REG_CONTROL      = 0x46,

      TIM_REG_STATUS3      = 0x4A,
      TIM_REG_STATUS_LCH   = 0x4C,
      TIM_REG_STAT3_LCH    = 0x4E,

      TIM_REG_QPLL_CTL     = 0x52,

      TIM_REG_BUSY_EN3     = 0x56,

      TIM_REG_BUSY_STAT3   = 0x5A,

      TIM_REG_BSTAT3_LCH   = 0x5E,
      TIM_REG_BCOUNTL      = 0x60,
      TIM_REG_BCOUNTH      = 0x62,
      TIM_REG_BCOUNTX      = 0x64,
      TIM_REG_BCID_OFFSET  = 0x66,
      TIM_REG_FV_VETOLEN   = 0x68,
      TIM_REG_FV_MATCH     = 0x6A,
      TIM_REG_FV_P_MIN     = 0x6C,
      TIM_REG_FV_P_MAX     = 0x6E,
      TIM_REG_FV_OFLOW     = 0x70,
      TIM_REG_FV_DELTAT    = 0x72,
      TIM_REG_FV_COUNTL    = 0x74,
      TIM_REG_FV_COUNTH    = 0x76,
      TIM_REG_FV_COUNTX    = 0x78,

      TIM_REG_FV_TCOUNTL   = 0x7C,
      TIM_REG_FV_TCOUNTH   = 0x7E,
      TIM_REG_TP_FIFOL     = 0x80,
      TIM_REG_TP_FIFOH     = 0x82,
      TIM_REG_FV_IDL       = 0x84,
      TIM_REG_FV_IDH       = 0x86,
      TIM_REG_TROSC2_PERL  = 0x88,
      TIM_REG_TROSC2_PERH  = 0x8A,

      TIM_REG_BURST_HI     = 0x8E,
      TIM_REG_TT_COUNT     = 0x90,
      TIM_REG_TTS_COUNT    = 0x92,
      TIM_REG_TSTAMPL      = 0x94,
      TIM_REG_TSTAMPH      = 0x96,
      TIM_REG_SRC_ADDR     = 0x98,
      TIM_REG_SINK_ADDR    = 0x9A,
      TIM_REG_DEBUG_CTL    = 0x9C,
      TIM_REG_DEBUG_STAT   = 0x9E
};

                       //! Applies to Signal Enables Register 0x00
enum TimBitEnables {
     TIM_BIT_EN_INT_TRIG = 0x0002,  //!< Enable internal repetitive Trigger
     TIM_BIT_EN_INT_ECR  = 0x0004,  //!< Enable internal repetitive ECReset
     TIM_BIT_EN_INT_BCR  = 0x0008,  //!< Enable internal repetitive BCReset
     TIM_BIT_EN_RANDOM   = 0x0010,  //!< Enable internal trigger randomizer
     TIM_BIT_EN_INT_FER  = 0x0020,  //!< Enable internal repetitive FEReset
     TIM_BIT_EN_WINDOW   = 0x0040,  //!< Enable trigger window
     TIM_BIT_EN_INT_BUSY = 0x0080,  //!< Enable internal Busy

     TIM_BIT_EN_EXT_CLK  = 0x0100,  //!< Enable external inputs: clock
     TIM_BIT_EN_EXT_TRIG = 0x0200,  //!< Enable external inputs: trigger
     TIM_BIT_EN_EXT_ECR  = 0x0400,  //!< Enable external inputs: ECReset
     TIM_BIT_EN_EXT_BCR  = 0x0800,  //!< Enable external inputs: BCReset
     TIM_BIT_EN_EXT_CAL  = 0x1000,  //!< Enable external inputs: Calibrate
     TIM_BIT_EN_EXT_FER  = 0x2000,  //!< Enable external inputs: FEReset
     TIM_BIT_EN_EXT_SEQ  = 0x4000,  //!< Enable external inputs: Sequencer Go
     TIM_BIT_EN_EXT_BUSY = 0x8000   //!< Enable external inputs: Busy
};

                       //!  Applies to Sequencer Control 0x1a and Output Latch 0x30
enum TimBitBackplane {
     TIM_L1A = 0x01,       //!< Level-1 Accept trigger
     TIM_ECR = 0x02,       //!< Event Counter Reset
     TIM_BCR = 0x04,       //!< Bunch Counter Reset
     TIM_CAL = 0x08,       //!< Calibrate strobe
     TIM_SID = 0x10,       //!< Serial event ID
     TIM_STT = 0x20,       //!< Serial Trigger Type
     TIM_FER = 0x40,       //!< Front-End Reset - reserved
     TIM_SPA = 0x80,       //!< Spare command   - reserved

     TIM_TRG = 0x31,       //!< Trigger and serial streams only
     TIM_CMD = 0xCF,       //!< Commands available
     TIM_RES = 0xC0        //!< Commands reserved
};
                        //!  Applies to Command register 0x02
enum TimBitCommand {
     TIM_VTRG = 0x02,               //!< Single VME Trigger
     TIM_VECR = 0x04,               //!< Single VME ECR
     TIM_VBCR = 0x08,               //!< Single VME BCR
     TIM_VCAL = 0x10,               //!< Single VME CAL
     TIM_VFER = 0x20,               //!< Single VME FER
     TIM_VSPA = 0x40,               //!< Single VME SPA
     TIM_VBUSY= 0x80,               //!< Set BUSY
     TIM_VROD_BUSY = 0x0100,        //!< Set ROD BUSY
     TIM_VBURST_MODE = 0x0200,      //!< Set BURST MODE == disable triggers
     TIM_VBURST_GO = 0x0400,        //!< Start BURST

     TIM_BIT_EN_TTC = 0x1000,           //!< Enable TTC Run Mode
     TIM_BIT_EN_TEST_BUSY = 0x2000,     //!< Enable TestBUSY
     TIM_BIT_EN_CLR_TEST_BUSY = 0x4000, //!< Clear TestBUSY
     TIM_BIT_VRESET = 0x8000            //!< Overall TIM Reset
};
                        //! Applies to Run Enables register 0x18,
                        //! which lowest 8 bits usually set together  
enum TimBitRunEnables {
     TIM_BIT_EN_TTC_CLK = 0x0001, //!< Enable TTC clk
     TIM_BIT_EN_TTC_L1A = 0x0002, //!< Enable TTC L1A
     TIM_BIT_EN_TTC_ECR = 0x0004, //!< Enable TTC ECR
     TIM_BIT_EN_TTC_BCR = 0x0008, //!< Enable TTC BCR
     TIM_BIT_EN_TTC_CAL = 0x0010, //!< Enable TTC CAL
     TIM_BIT_EN_TTC_FER = 0x0020, //!< Enable TTC FER
     TIM_BIT_EN_TTC_SPA = 0x0040, //!< Enable TTC SPA
     TIM_BIT_EN_ROD_BUSY = 0x0080, //!< Enable RodBusy stop internal TIM triggers
     TIM_BIT_EN_ID   = 0x0200,
     TIM_BIT_EN_TYPE = 0x0400,
     TIM_BIT_EN_SA_ECR = 0x4000, //!< Enable counter ECR
     TIM_BIT_EN_SA_BCR = 0x8000  //!< Enable counter ECR
};

                        //! Applies to  Enables3 Register 0x42
enum TimBitEnables3 {
     TIM_BIT_EN_L1ID2ECRID    = 0x0010,  //!< Enable L1ID rollover to increment ECR-ID
     TIM_BIT_EN_QPLLCONTROL   = 0x0020,  //!< Enable output QPLL lines to TTCrq
     TIM_BIT_EN_RANDOM2       = 0x0040,  //!< Enable randomizer 2 operation
     TIM_BIT_EN_OSC2          = 0x0080,  //!< Enable oscillator 2 operation 
     TIM_BIT_EN_TRIG_SEQ      = 0x0100,  //!< Stand-alone trigs (and SpareIN!!!) set to generate (repetitive!!!) SeqGo
     TIM_BIT_EN_SHORT_FPSIG   = 0x0200,
     TIM_BIT_EN_PREBUSY_BURST = 0x0400,
     TIM_BIT_EN_TTC_TRIG_DELAY= 0x0800,
     TIM_BIT_EN_TTC_TTYPE2SA  = 0x1000,
     TIM_BIT_EN_TTC_BCR2SA    = 0x2000, 
     TIM_BIT_EN_TTC_BCR_DELAY = 0x4000
};
                     //!  Applies to Sequencer Control 0x1a Register
enum TimBitSeqControl {
     TIM_BIT_SEQ_EN_ALL = 0x00FF,
     TIM_BIT_SEQ_TRIG   = 0x0001,
     TIM_BIT_SEQ_BCR    = 0x0004,
     TIM_BIT_SEQ_CAL    = 0x0008,
     TIM_BIT_SEQ_RESET  = 0x0200,
     TIM_BIT_SEQ_GO     = 0x0400,
     TIM_BIT_EN_CYCLIC  = 0x0800
};

enum TimBitTTCStatus {
     TIM_BIT_TTC_READY  = 0x4000
};
//! TIM 3C register bit definitions. JCH.

enum TimBitStatus3 {
     TIM_BIT_STATUS3_TTCCLKENOK = 0x0080,
     TIM_BIT_STATUS3_EXTCLKENOK = 0x0100,
     TIM_BIT_STATUS3_INTCLKENOK = 0x0200,
     TIM_BIT_STATUS3_PLLSTABLE  = 0x0400
};

                       //! Applies to Busy Enable 3 Register 0x56
enum TimBitBusyEnable3 {
  TIM_BIT_BUSY_EN3_ENRBBUSY            = 0x0001, //enable RodBusy stop triggers in SA mode, old
  TIM_BIT_BUSY_EN3_ROD_BUSY4SA_L1ASTOP = 0x0001, //enable RodBusy stop triggers in SA mode  
  TIM_BIT_BUSY_EN3_VME_BUSY4SA_L1ASTOP = 0x0010, //enable VME BUSY stop triggers in TIM
  TIM_BIT_BUSY_EN3_ROD_BUSY2TB_FP  = 0x0100, //enable RodBusy to be seen as TB on FP 
  TIM_BIT_BUSY_EN3_VME_BUSY2TB_FP  = 0x1000  //enable VME BUSY be seen as TB on FP
  
};

                       //! Applies to Debug Control Register 0x9c
enum TimBitDebugCtl { 
  TIM_BIT_DEBUGCTL_CSBDISABLE = 0x0100, // Disable Clk-Switch Busy
  TIM_BIT_DEBUGCTL_SARBDISABLE= 0x0200, // Disable SA-Mode setting RodBusy
  TIM_BIT_DEBUGCTL_FVDISABLE  = 0x1000 // FFTV Veto Disable
};

//! End TIM 3C extras

//! Define register values as masks

enum TimMaskFrequency {
     TIM_MASK_TRIG_600_KHZ = 0x0000,
     TIM_MASK_TRIG_300_KHZ = 0x0002,
     TIM_MASK_TRIG_200_KHZ = 0x0003,
     TIM_MASK_TRIG_150_KHZ = 0x0004,
     TIM_MASK_TRIG_120_KHZ = 0x0005,
     TIM_MASK_TRIG_100_KHZ = 0x0006,
     TIM_MASK_TRIG_60_0KHZ = 0x0001,
     TIM_MASK_TRIG_50_0KHZ = 0x0007,
     TIM_MASK_TRIG_30_0KHZ = 0x000A,
     TIM_MASK_TRIG_20_0KHZ = 0x000B,
     TIM_MASK_TRIG_15_0KHZ = 0x000C,
     TIM_MASK_TRIG_12_0KHZ = 0x000D,
     TIM_MASK_TRIG_10_0KHZ = 0x000E,
     TIM_MASK_TRIG_6_00KHZ = 0x0009,
     TIM_MASK_TRIG_5_00KHZ = 0x000F,
     TIM_MASK_TRIG_3_00KHZ = 0x0012,
     TIM_MASK_TRIG_2_00KHZ = 0x0013,
     TIM_MASK_TRIG_1_50KHZ = 0x0014,
     TIM_MASK_TRIG_1_20KHZ = 0x0015,
     TIM_MASK_TRIG_1_00KHZ = 0x0016,
     TIM_MASK_TRIG_0_60KHZ = 0x0011,
     TIM_MASK_TRIG_0_50KHZ = 0x0017,
     TIM_MASK_TRIG_0_30KHZ = 0x001A,
     TIM_MASK_TRIG_0_20KHZ = 0x001B,
     TIM_MASK_TRIG_0_15KHZ = 0x001C,
     TIM_MASK_TRIG_0_12KHZ = 0x001D,
     TIM_MASK_TRIG_0_10KHZ = 0x001E,
     TIM_MASK_TRIG_0_06KHZ = 0x0019,
     TIM_MASK_TRIG_0_05KHZ = 0x001F,

     TIM_MASK_FECR_60_00HZ = 0x0000,
     TIM_MASK_FECR_30_00HZ = 0x0200,
     TIM_MASK_FECR_20_00HZ = 0x0300,
     TIM_MASK_FECR_15_00HZ = 0x0400,
     TIM_MASK_FECR_12_00HZ = 0x0500,
     TIM_MASK_FECR_10_00HZ = 0x0600,
     TIM_MASK_FECR_6_000HZ = 0x0100,
     TIM_MASK_FECR_5_000HZ = 0x0700,
     TIM_MASK_FECR_3_000HZ = 0x0A00,
     TIM_MASK_FECR_2_000HZ = 0x0B00,
     TIM_MASK_FECR_1_500HZ = 0x0C00,
     TIM_MASK_FECR_1_200HZ = 0x0D00,
     TIM_MASK_FECR_1_000HZ = 0x0E00,
     TIM_MASK_FECR_0_600HZ = 0x0900,
     TIM_MASK_FECR_0_500HZ = 0x0F00,
     TIM_MASK_FECR_0_300HZ = 0x1200,
     TIM_MASK_FECR_0_200HZ = 0x1300,
     TIM_MASK_FECR_0_150HZ = 0x1400,
     TIM_MASK_FECR_0_120HZ = 0x1500,
     TIM_MASK_FECR_0_100HZ = 0x1600,
     TIM_MASK_FECR_0_060HZ = 0x1100,
     TIM_MASK_FECR_0_050HZ = 0x1700,
     TIM_MASK_FECR_0_030HZ = 0x1A00,
     TIM_MASK_FECR_0_020HZ = 0x1B00,
     TIM_MASK_FECR_0_015HZ = 0x1C00,
     TIM_MASK_FECR_0_012HZ = 0x1D00,
     TIM_MASK_FECR_0_010HZ = 0x1E00,
     TIM_MASK_FECR_0_006HZ = 0x1900,
     TIM_MASK_FECR_0_005HZ = 0x1F00
};

enum TimRand2Frequency {
  TIM_RAND2_FREQ_150KHZ   = 0xf,
  TIM_RAND2_FREQ_80KHZ    = 0xe,
  TIM_RAND2_FREQ_40KHZ    = 0xd,
  TIM_RAND2_FREQ_20KHZ    = 0xc,
  TIM_RAND2_FREQ_10KHZ    = 0xb,
  TIM_RAND2_FREQ_5KHZ     = 0xa,
  TIM_RAND2_FREQ_2_5KHZ   = 0x9,
  TIM_RAND2_FREQ_1_2KHZ   = 0x8,
  TIM_RAND2_FREQ_0_6KHZ   = 0x7,
  TIM_RAND2_FREQ_0_3KHZ   = 0x6,
  TIM_RAND2_FREQ_0_16KHZ  = 0x5,
  TIM_RAND2_FREQ_0_075KHZ = 0x4,
  TIM_RAND2_FREQ_0_04KHZ  = 0x3,
  TIM_RAND2_FREQ_0_01KHZ  = 0x2,
  TIM_RAND2_FREQ_0_005KHZ = 0x1,
  TIM_RAND2_FREQ_0_001KHZ = 0x0
};

const int TIM_FREQ_SIZE = 29;

const int TIM_TRIG_FREQUENCY[TIM_FREQ_SIZE][2] = {
   { TIM_MASK_TRIG_600_KHZ, 600000 },
   { TIM_MASK_TRIG_300_KHZ, 300000 },
   { TIM_MASK_TRIG_200_KHZ, 200000 },
   { TIM_MASK_TRIG_150_KHZ, 150000 },
   { TIM_MASK_TRIG_120_KHZ, 120000 },
   { TIM_MASK_TRIG_100_KHZ, 100000 },
   { TIM_MASK_TRIG_60_0KHZ,  60000 },
   { TIM_MASK_TRIG_50_0KHZ,  50000 },
   { TIM_MASK_TRIG_30_0KHZ,  30000 },
   { TIM_MASK_TRIG_20_0KHZ,  20000 },
   { TIM_MASK_TRIG_15_0KHZ,  15000 },
   { TIM_MASK_TRIG_12_0KHZ,  12000 },
   { TIM_MASK_TRIG_10_0KHZ,  10000 },
   { TIM_MASK_TRIG_6_00KHZ,   6000 },
   { TIM_MASK_TRIG_5_00KHZ,   5000 },
   { TIM_MASK_TRIG_3_00KHZ,   3000 },
   { TIM_MASK_TRIG_2_00KHZ,   2000 },
   { TIM_MASK_TRIG_1_50KHZ,   1500 },
   { TIM_MASK_TRIG_1_20KHZ,   1200 },
   { TIM_MASK_TRIG_1_00KHZ,   1000 },
   { TIM_MASK_TRIG_0_60KHZ,    600 },
   { TIM_MASK_TRIG_0_50KHZ,    500 },
   { TIM_MASK_TRIG_0_30KHZ,    300 },
   { TIM_MASK_TRIG_0_20KHZ,    200 },
   { TIM_MASK_TRIG_0_15KHZ,    150 },
   { TIM_MASK_TRIG_0_12KHZ,    120 },
   { TIM_MASK_TRIG_0_10KHZ,    100 },
   { TIM_MASK_TRIG_0_06KHZ,     60 },
   { TIM_MASK_TRIG_0_05KHZ,     50 }
};

const int TIM_FECR_FREQUENCY[TIM_FREQ_SIZE][2] = {
   { TIM_MASK_FECR_60_00HZ, 60000 },
   { TIM_MASK_FECR_30_00HZ, 30000 },
   { TIM_MASK_FECR_20_00HZ, 20000 },
   { TIM_MASK_FECR_15_00HZ, 15000 },
   { TIM_MASK_FECR_12_00HZ, 12000 },
   { TIM_MASK_FECR_10_00HZ, 10000 },
   { TIM_MASK_FECR_6_000HZ,  6000 },
   { TIM_MASK_FECR_5_000HZ,  5000 },
   { TIM_MASK_FECR_3_000HZ,  3000 },
   { TIM_MASK_FECR_2_000HZ,  2000 },
   { TIM_MASK_FECR_1_500HZ,  1500 },
   { TIM_MASK_FECR_1_200HZ,  1200 },
   { TIM_MASK_FECR_1_000HZ,  1000 },
   { TIM_MASK_FECR_0_600HZ,   600 },
   { TIM_MASK_FECR_0_500HZ,   500 },
   { TIM_MASK_FECR_0_300HZ,   300 },
   { TIM_MASK_FECR_0_200HZ,   200 },
   { TIM_MASK_FECR_0_150HZ,   150 },
   { TIM_MASK_FECR_0_120HZ,   120 },
   { TIM_MASK_FECR_0_100HZ,   100 },
   { TIM_MASK_FECR_0_060HZ,    60 },
   { TIM_MASK_FECR_0_050HZ,    50 },
   { TIM_MASK_FECR_0_030HZ,    30 },
   { TIM_MASK_FECR_0_020HZ,    20 },
   { TIM_MASK_FECR_0_015HZ,    15 },
   { TIM_MASK_FECR_0_012HZ,    12 },
   { TIM_MASK_FECR_0_010HZ,    10 },
   { TIM_MASK_FECR_0_006HZ,     6 },
   { TIM_MASK_FECR_0_005HZ,     5 }
};

const int TIM_RANDOM2_FREQ_SIZE = 16;

const int TIM_RANDOM2_FREQ[TIM_RANDOM2_FREQ_SIZE][2] = {
  { TIM_RAND2_FREQ_150KHZ, 150000 },
  { TIM_RAND2_FREQ_80KHZ,   80000 },
  { TIM_RAND2_FREQ_40KHZ,   40000 },
  { TIM_RAND2_FREQ_20KHZ,   20000 },
  { TIM_RAND2_FREQ_10KHZ,   10000 },
  { TIM_RAND2_FREQ_5KHZ,     5000 },
  { TIM_RAND2_FREQ_2_5KHZ,   2500 },
  { TIM_RAND2_FREQ_1_2KHZ,   1200 },
  { TIM_RAND2_FREQ_0_6KHZ,    600 },
  { TIM_RAND2_FREQ_0_3KHZ,    300 },
  { TIM_RAND2_FREQ_0_16KHZ,   160 },
  { TIM_RAND2_FREQ_0_075KHZ,   75 },
  { TIM_RAND2_FREQ_0_04KHZ,    40 },
  { TIM_RAND2_FREQ_0_01KHZ,    10 },
  { TIM_RAND2_FREQ_0_005KHZ,    5 },
  { TIM_RAND2_FREQ_0_001KHZ,    1 }
};

} // End namespace SctPixelRod

#endif // SCTPIXELROD_TIMDEFINE_H
