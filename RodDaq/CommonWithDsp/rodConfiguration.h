/************************************************************************************
 * rodConfiguration.h
 *
 *  synopsis: Describes ROD specific hardware details, e.g. is this an SCT or a pixel
 *           ROD, how many formatter FPGAs are there...
 *
 *  Damon Fasching, UW Madison                            fasching@wisconsin.cern.ch
 *  Douglas Ferguson, UW Madison   (510) 486-5230         dpferguson@lbl.gov
 *
 *  modifications/bugs    
 *   - Moved rodConfiguration #defines here (were in primParams.h)   11.04.02 dpsf
 ************************************************************************************/
#ifndef ROD_CONFIGURATION
#define ROD_CONFIGURATION

#include "dsp_types.h"

/* Some dataflow parameters */
#define EFBS_PER_ROD               2
#define FORMATTERS_PER_EFB         4
#define FORMATTERS_PER_ROD         (FORMATTERS_PER_EFB*EFBS_PER_ROD)
#define LINKS_PER_FORMATTER        12
#define DATA_LINKS_PER_EFB         (LINKS_PER_FORMATTER*FORMATTERS_PER_EFB)
#define DATA_LINKS_PER_ROD         (DATA_LINKS_PER_EFB * EFBS_PER_ROD)
#define CTRL_LINKS_PER_ROD         (DATA_LINKS_PER_ROD / 2)
#define STROBE_DELAYS_PER_BOC      24
#define INMEM_DEPTH                0x8000
#define INMEM_DEPTH_REV_B          0x1000
#define DEFAULT_CAPTURE_LINK       0x80
 
/* number of slave DSPs */
#if (defined(I_AM_MASTER_DSP) || defined(I_AM_HOST))
  #define N_SDSP  4
#endif
#ifdef I_AM_SLAVE_DSP
  #define N_SDSP  1
#endif

/* 6713 phase lock loop registers & device config register; these are used by the MDSP
   while setting a Rev. E ROD's SDSP clocks. */
#define PLL_TIMEOUT 5000000  // .5 sec.

#define SDSP6713_PLLCSR    0x01b7c100
#define SDSP6713_PLLM      0x01b7c110
#define SDSP6713_PLLDIV0   0x01b7c114
#define SDSP6713_DEVCFG    0x019c0200

/* configuration data structures */
struct SlvDspConfig {
	UINT8 present;
	UINT8 commOnOff;
	UINT8 type;
	UINT8 clkSpeed;
};

typedef struct RodConfig {
	UINT32 detector;
	UINT32 rodType;
	UINT32 boardClockInMHz;
	UINT32 DSPClockInMHz;
	UINT32 numSlaves;
	UINT32 numSlvsCommOn;

	/* The formatter modes reflect register settings on the formatter in question
	   (one nibble per formatter), with different meanings for SCT & Pixel. For
	   SCT 00= Off, 01= On in condensed mode, and 02= On in expanded format mode. 
	   For Pixels, 00= Off, 01= On in 40 MHz mode, 02= 80 MHz mode, and 03= 160
	   MHz mode: */

	UINT32  formatterMode;
	struct SlvDspConfig slvDspConfig[N_SDSP];
} RodConfig;

#if defined (SCT_ROD)
	#define RODCFG_FMT_OFF        0x0
	#define RODCFG_FMT_CONDENSED  0x1
	#define RODCFG_FMT_EXPANDED   0x2
#elif defined(PIXEL_ROD)
	#define RODCFG_FMT_OFF        0x0
	#define RODCFG_FMT_40_MHZ     0x1
	#define RODCFG_FMT_80_MHZ     0x2
	#define RODCFG_FMT_160_MHZ    0x3
#endif

/* Some modes are additive; e.g. CALIBRATION +SIMULATION. The ROD will
   return an error for those modes which are incompatible. */

/* NDT: Sets the data path according to the simulation type or link_data
        +capture,  sets the atlas running mode to true so that router is
        not allowed to apply back-pressure, and sets the link masks and
        assoc. variables according to the information stored in the 
        INIT module configuration set. Automatic inmem readout triggers an
        automatic readout of the inmems' link(s) into a MDSP buffer after
        completion of each serial stream output. */
//"dummy" mode modifiers (=> exit cfg readback, evt capture, or simulation):  
#define NOCAPTURE_MODE                    0
#define NOSIMULATION_MODE                 0
#define NORMAL_MODE                       0

//mode modifiers:
#define SIMULATION_MODE                   0x0001
#define CONFIG_READBACK_MODE              0x0002
#define INMEM_EVT_CAPTURE_MODE            0x0004
#define INMEM_AUTO_READOUT_MODE           0x0008
#define CALIBRATION_SLINK_OVERRIDE_MODE   0x0010
#define TIM_TRIGGER_MODE                  0x0020

//ROD modes:
#define ROD_INIT_MODE            0x00010000 
#define DATA_TAKING_MODE         0x00020000 
#define CALIBRATION_MODE         0x00040000

/* routine flags: */
#define SET_MODE        0
#define STORE_MODE      1
#define RESTORE_MODE    2
#define MODIFY_MODE     3

typedef struct {
	UINT8  sim, cfgReadback, inmemEvtCapture, autoInmemReadout;
	UINT32 rodMode, unused[2];
	#if  (  defined(I_AM_MASTER_DSP) || defined(I_AM_HOST))
		UINT32 rcfCmd[2], rtrCmdStat,  evtsPerL1A;
		UINT32 lutSelect, unused1[3], cmdMask[2][2], fmtCfg[8];
		#ifdef PIXEL_ROD
		UINT32 fmt_evtsPerL1A[8];
		#endif
	#endif
} RodModeCfg;

#if  (defined(I_AM_MASTER_DSP) || defined(I_AM_HOST))
	/* Link masks defined bitwise: set all masks on= CL on +DLP +LCFG on. Note that
	   settings within the same group are mutually exclusive; the values are set so
	   that the mask setting routine will ignore inappropriate mixtures. */

	#define DATA_LINK_MASKED  0x1  /* Dynamic modebits setting for disabled link */
	#define DATA_LINK_PLAY    0x3  /* Normal dyn. modebits setting for enabled link */
	#define DATA_LINK_SKIP    0x7  /* Sets link so that it skips over the next event */ 
	#define DATA_LINK_D1P1    0xe  /* Link will dump 1st event, play out 2nd. */ 

	#define COMMAND_LINK_OFF 0x40   
	#define COMMAND_LINK_ON  0x60   

	#define LINK_CFG_OFF     0x100   /* Link on;  for initialization of ROD. */
	#define LINK_CFG_ON      0x300   /* Link off; for initialization of ROD. */

	/* Internal masks: */
	#define DATA_LINK      0x00f
	#define COMMAND_LINK   0x0f0
	#define LINK_CFG       0xf00


	/* Storage flag settings for setLinkMasks: */
	#define INIT_MASK             0
	#define SET_MASK              1
	#define UPDATE_MASK           2
	#define SWITCH_MASK           3
	#define STORE_MASK            4
	#define COMPUTE_MASK_DELTAS   5

	typedef struct FmtMask {
		UINT16 fmtCfg[FORMATTERS_PER_ROD];
		UINT32 dataLinkMask[3], unused;
	} FmtMask;

	typedef struct DynMask {
		/*                                   MB0/1 */
		UINT16 modeBits[FORMATTERS_PER_ROD][2];
	} DynMask;

	typedef struct CmdMask {
		UINT32 highMask;
		UINT32 lowMask, unused[2];
	} CmdMask;
	
	typedef struct DeltaMask {
		UINT32 modeBits;
		UINT8  cmdLine, fmtCfg, dataLinkMask, unused;
	} DeltaMask;

	#define DELTA_SP0_LO  1
	#define DELTA_SP0_HI  2
	#define DELTA_SP1_LO  4
	#define DELTA_SP1_HI  8

	#define DELTA_DFLT_MB0(fmt)  (1<<(4*(fmt) +0))
	#define DELTA_DFLT_MB1(fmt)  (1<<(4*(fmt) +1))
	#define DELTA_CRTV_MB0(fmt)  (1<<(4*(fmt) +2))
	#define DELTA_CRTV_MB1(fmt)  (1<<(4*(fmt) +3))

	#define DELTA_FMT_CFG(fmt)        (1<<(fmt))
	#define DELTA_DATA_LINK_MASK(i)   (1<<(i))


	/* Link Modifications for use with the SCT setChipVariable routine: */
	#ifdef SCT_ROD
		#define LINK_MOD_FALSE  0
		#define LINK_MOD_LOWER  1
		#define LINK_MOD_UPPER  2
		#define LINK_MOD_BOTH   3
	#endif

	/* Te following definition is for global updates affecting all modules which have
	   been added to a mask set. Note that it cannot conflict with ALL_MODULES and
	   NO_MODULE which are defined in primParams.h */
	#define ALL_SET_MODULES  0xf0

	/* The MaskConfigData structure  is used to set the command & data link masks
	   when a new group of modules is being addressed. Up to 8 'mask sets' are
	   defined; the 1st five and last two should be reserved for internal program
	   use (esp. the last two). The INIT set should be set to represent the entire
	   module set. */
	#define N_MASK_SETS   8
		#define MASK_SET_0     0       //Note: Used by calibration routines.
		#define MASK_SET_1     1       //              ""
		#define MASK_SET_2     2       //              ""
		#define MASK_SET_3     3       //              ""
		#define MASK_SET_USER  4       //General purpose.
		#define MASK_SET_INIT  5       //Should be set using a primlist on startup.

		/* Work0 & 1 are general purpose; note however that they are used internally
		   as well and so their values should not be counted on to stay constant, esp.
		   during complex operations. */
		#define MASK_SET_WORK0 6
		#define MASK_SET_WORK1 7

		#define MASK_SET_ALL   8       

		#define MASK_SET_SEARCH  9  //Used internally.

	typedef struct MaskConfigData {
		CmdMask   cmdMask[2]; /* [0]= default / SP0  [1]= crtv. / SP1 */ 
		DynMask   dynMask[2];   
		FmtMask   fmtMask;

		/* Bitfield defining which modules are contained in the mask set; there are
		   separate sets for the command masks and mode bits. The link configuration
		   is not handled, since during mask operations normally just the command
		   links & mode bits are changed and link configuration is left alone. */
		UINT32    validModules[2][2];
		
		/* Used for diagnostics: set the output of the LEMO connector on the ROD
		   front panel to the lowermost link of last module updated in the set. */
		UINT32    lemoLink, lemoFmt, unused[2];

		DeltaMask deltaMask[N_MASK_SETS];
	} MaskConfigData;

#endif

/* An array of ModuleMaskData (defined in the c file) stores the mask configuration
   for each module. This data copies a small subset of the full module configuration
   data; the data is used by both the MDSP and SDSPs. */
typedef struct ModuleMaskData {
	UINT8 defined;
	UINT8 cmdLine;
	UINT8 fmtLink[2];
} ModuleMaskData;

typedef struct {
	UINT32 *buffPtr, nEvents, length;
} InmemTrap;

#endif
