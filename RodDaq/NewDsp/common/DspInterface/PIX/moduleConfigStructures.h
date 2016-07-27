/**************************************************************************
 *
 * Title: Master/PIX moduleConfigStructures.h
 * Version: 11th November 2002
 *
 * Description:
 * ROD Master DSP Pixel module configuration structures
 * and constant definitions.
 *
 * Author: John Richardson; john.richardson@cern.ch
 *
 *   - Adjusted to make compatible with the SCT module structure &
 *     existing c code by adding members to the PixelModule structure.
 *     Structure PixelFETrims expanded to a full array
 *     (was a bit-field).                                    14.03.03 dpsf
 *   - Added FEIndex into PixelFEConfig struct               16 Apr 03
 *   - Added in the (virtual) MCC strobe register, so that the
 *     count register modifications done in the pixel serial stream
 *     building functions do not interfere with placing the module
 *     in data taking mode.                                  10 Feb 04 dpsf
 *   - Added the calCoeff structure, a separate structure which the
 *     MDSP transfers to the SDSPs during run-time (module loading &
 *     SDSP start-up), so they have enough information to perform
 *     the vcal to electrons conversion.                     28 Jan 05 dpsf
 **************************************************************************/
#ifndef _PIXEL_CONFIG_STRUCTURES_	 /* multiple inclusion protection */
#define _PIXEL_CONFIG_STRUCTURES_
#include "dsp_types.h"

/*  Up to 28 pixel modules, with gaps in module structure set where
    the corresponding data links are not connected */
// #define N_PIXEL_MODULES     28   
// #define N_PIXEL_EXTMODULES  0    /* no redundant command links */
// #define N_PIXEL_TOTMODULES  ((N_PIXEL_MODULES)+(N_PIXEL_EXTMODULES))

#define N_PIXEL_COLUMNS  18
#define N_PIXEL_ROWS     160
#define N_PIXEL_FE_CHIPS 16

#define FE_I1  1
#define FE_I2  2
#define FEI1   1
#define FEI2   2
#define MCC_I1 1
#define MCC_I2 2
#define MCCI1  1
#define MCCI2  2

/* MCC options */

#define MCC_SINGLE_40 0
#define MCC_DOUBLE_40 1
#define MCC_SINGLE_80 2
#define MCC_DOUBLE_80 3

/* Default & off-ROD positions of a module's TTC fibres. The default primary
   TTC fibre location for a module is simply the module's position inside the
   structure set. */
#define DEFAULT_TTC    0x80

/* Default Link & Data Links which are turned off: */
#define DEFAULT_DATA_LINK    0x80
#define DATA_LINK_OFF        0xff

#define PIXEL_STATIC_CONFIG 0
#define PIXEL_SCAN_CONFIG   1

/************* STRUCTURE DEFINITIONS ***********/
/* A small calibration coefficient structure containing just the vcal
   to electron conversion coefficients, for fitting on the SDSPs. */
typedef struct {
	FLOAT32 vcal[4];
	FLOAT32 cap[2];
	UINT8   capSel;
	UINT8   unused[3];
	UINT32  unused1;
} ChipCoeff;

typedef struct {
	ChipCoeff fe[N_PIXEL_FE_CHIPS];
} CalCoeff;


typedef struct { //FE Command register
	UINT32 address;		       /* 5 bits */
	UINT32 command;            /* 32 bits */
} PixelFECommand;

typedef struct { //FE global register
	//This is valid for both FE-I1 & 2, since the FE-I1 is a sub-set.
	UINT32  latency             : 8;                 
	UINT32  dacIVDD2            : 8;                    
	UINT32  dacIP2              : 8;                      
	UINT32  dacID               : 8;
	
	UINT32  dacIP               : 8;                       
	UINT32  dacITRIMTH          : 8;                  
	UINT32  dacIF               : 8;                       
	UINT32  dacITH1             : 8;
	
	UINT32  dacITH2             : 8;                     
	UINT32  dacIL               : 8;                       
	UINT32  dacIL2              : 8;                      
	UINT32  dacITRIMIF          : 8; 
	
	UINT32  dacSpare            : 8;                        
	UINT32  threshTOTMinimum    : 8;            
	UINT32  threshTOTDouble     : 8;             
	UINT32  hitbusScaler        : 8;
	
	UINT32  capMeasure          : 6; 
	UINT32  gdac                : 5;       
	UINT32  selfWidth           : 4;
	UINT32  selfLatency         : 4;
	UINT32  muxTestPixel        : 2;
	UINT32  spare               : 2;
	UINT32  aregTrim            : 2;
	UINT32  aregMeas            : 2;
	UINT32  dregTrim            : 2;
	UINT32  dregMeas            : 2;
	UINT32  parity              : 1;                
	
	UINT32  dacMonLeakADC       : 9;
	UINT32  dacVCAL             : 10; /* extended to 10 bit for FE2/3 */          
	UINT32  widthSelfTrigger    : 4;        
	UINT32  muxDO               : 4; /* note: dynamically defined */                      
	UINT32  muxMonHit           : 4;
	UINT32  muxEOC              : 2;                      
	
	UINT32  frequencyCEU        : 2;                
	UINT32  modeTOTThresh       : 2;               
	UINT32  enableTimestamp     : 1;             
	UINT32  enableSelfTrigger   : 1;        
	UINT32  enableHitParity     : 1;                    
	UINT32  monMonLeakADC       : 1;            
	UINT32  monADCRef           : 1;                
	UINT32  enableMonLeak       : 1;            
	UINT32  statusMonLeak       : 1;            
	UINT32  enableCapTest       : 1;                
	UINT32  enableBuffer        : 1;                 
	UINT32  enableVcalMeasure   : 1;            
	UINT32  enableLeakMeasure   : 1;            
	UINT32  enableBufferBoost   : 1;            
	UINT32  enableCP8           : 1;                    
	UINT32  monIVDD2            : 1;                     
	UINT32  monID               : 1;                        
	UINT32  enableCP7           : 1;                    
	UINT32  monIP2              : 1;                       
	UINT32  monIP               : 1;                        
	UINT32  enableCP6           : 1;                    
	UINT32  monITRIMTH          : 1;                   
	UINT32  monIF               : 1;                        
	UINT32  enableCP5           : 1;                    
	UINT32  monITRIMIF          : 1;                   
	UINT32  monVCAL             : 1;                      
	UINT32  enableCP4           : 1;                    
	UINT32  enableCinjHigh      : 1;               
	UINT32  enableExternal      : 1;               
	UINT32  enableTestAnalogRef : 1;
	
	UINT32  enableDigital       : 1;                
	UINT32  enableCP3           : 1;                    
	UINT32  monITH1             : 1;                      
	UINT32  monITH2             : 1;                      
	UINT32  enableCP2           : 1;                    
	UINT32  monIL               : 1;                        
	UINT32  monIL2              : 1;                       
	UINT32  enableCP1           : 1;                                 
	UINT32  enableCP0           : 1;   
	UINT32  enableHitbus        : 1;                 
	UINT32  monSpare            : 1;                     
	UINT32  enableAregMeas      : 1;
	UINT32  enableAreg          : 1;
	UINT32  enableLvdsRegMeas   : 1;
	UINT32  enableDregMeas      : 1;
	UINT32  enableTune          : 1;
	UINT32  enableBiasComp      : 1;
	UINT32  enableIpMonitor     : 1;
} PixelFEGlobal;				     
							    

typedef struct { //Pixel-level control bits
	UINT32 maskEnable[5][N_PIXEL_COLUMNS]; /* 32 bits, one bit per pixel thus 5 words per column */
	UINT32 maskSelect[5][N_PIXEL_COLUMNS];
	UINT32 maskPreamp[5][N_PIXEL_COLUMNS];
	UINT32 maskHitbus[5][N_PIXEL_COLUMNS];
} PixelFEMasks;  
   

typedef struct { //Trim DACs:
	UINT8 dacThresholdTrim[N_PIXEL_ROWS][N_PIXEL_COLUMNS];  /*: 5; Currently 5 bits per pixel, will change in next iteration of FE */
	UINT8  dacFeedbackTrim[N_PIXEL_ROWS][N_PIXEL_COLUMNS];  /*: 5;                 ""                                              */
} PixelFETrims; 


typedef struct { 
/* Sub-structure for calibration of injection-capacitors, VCAL-DAC and
   leakage current measurement */
	FLOAT32 cinjLo;
	FLOAT32 cinjHi;
	FLOAT32 vcalCoeff[4];
	FLOAT32 chargeCoeffClo;   //dpsf: ?
	FLOAT32 chargeCoeffChi;
	FLOAT32 chargeOffsetClo;
	FLOAT32 chargeOffsetChi;
	FLOAT32 monleakCoeff;
	/* need MCC time calibration also //dpsf: ? */
} PixelFECalib; 


typedef struct { //MCC registers
   UINT16 regCSR;                  
   UINT16 regLV1;                  
   UINT16 regFEEN;				 
   UINT16 regWFE;					 

   UINT16 regWMCC; 				
   UINT16 regCNT;					 
   UINT16 regCAL;					 
   UINT16 regPEF;		

   UINT16 regWBITD;
   UINT16 regWRECD;
   UINT16 regSBSR;

   /* Strobe duration is a virtual register (shared with CNT); when preparing the
      modules for data this value is substituted for the count. */
   UINT16 regSTR;
} PixelMCCRegisters;

typedef struct { //FE level parameters
	UINT32 FEIndex; 	
	PixelFECommand FECommand;
	PixelFEGlobal FEGlobal;
	PixelFEMasks FEMasks;
	PixelFETrims FETrims;
	PixelFECalib FECalib;
} PixelFEConfig;

typedef struct {
	UINT8  tdac, prevTdac;
	UINT8  fdac, prevFdac;
} PixelTrimScanData;

typedef struct {
	/* FE module-level options: */
	UINT16 maskEnableFEConfig; /* 16 bits, one per FE */
	UINT16 maskEnableFEScan;
	UINT16 maskEnableFEDacs;
	UINT8 feFlavour;
	UINT8 mccFlavour;
	
	/* FE configurations: */
	PixelFEConfig FEConfig[N_PIXEL_FE_CHIPS+1];
	//PixelFEConfig chip[N_PIXEL_FE_CHIPS]; //dpsf: ?
	
	/* MCC configuration */
	PixelMCCRegisters MCCRegisters;
	
	/* The current (uniform) value for the DACs of each pixel during a DAC scan */
	PixelTrimScanData  trimScanData;
	char  idStr[128]; /* Module identification string */
	char    tag[128]; /* Module configuration tag */
	UINT32  revision; /* Module configuration revision */

	UINT8 present;    /* Module is physically present. Does not need setting
	                     externally; handled by the Master DSP. */
	
	UINT8 active;     /* 1 -> participates in scans */
	                  /* 0 -> registers unchanged during scanning */
	UINT8 moduleIdx;  /* Copy of the module's index for access from a pointer */
	UINT8 groupId;    /* The ID of the module's group. This is used to indicate
	                     which slave DSP will receive the module's data (if group
	                     based distribution is set), and also to allow different
	                     module groups to be triggered independently (for
	                     cross-talk studies). valid range: [0,7] */
	UINT8 pTTC;       /* primary TX channel  */
	UINT8 rx;         /* data link used by module */
//	UINT8 unused1[2];
	
//	UINT32 unused2[0x653]; /* align module structures on convenient boundary */

} PixelModuleConfig; /* declare N_PIXEL_CONFIG_SETS structure for each of N_PIXEL_MODULES */

#endif   /* multiple inclusion protection */


