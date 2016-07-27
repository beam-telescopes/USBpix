#ifndef SCANENUMS_H
#define SCANENUMS_H
/* enums for DSP interface */

#define MCC_SINGLE_40 0
#define MCC_DOUBLE_40 1
#define MCC_SINGLE_80 2
#define MCC_DOUBLE_80 3

/* several histogram options can be enabled at once, using a bit mask */
/* the bining can be set for each histogram according to need, a
   default setting provides a sensible choice   */

enum BinSize {SCAN_BIN_DEFAULT, SCAN_UNBINNED, SCAN_BITS_8, SCAN_BITS_16, SCAN_BITS_32};
/* the configuration actions can be set separately for each group using a bit mask,
   e.g. some groups can choose not to receive a strobe.
   Groups can choose to configure one FE at a time (CONFIGURE_CONSEC).
   Two trigger super groups can be selected for cross talk studies with defined
   delays (defined in triggerOptions) 
*/
enum ModActions {SCAN_ACT_DEFAULT, SCAN_RESET, SCAN_CONFIGURE_ALL, SCAN_CONFIGURE_CONSEC, SCAN_TRIGGER, SCAN_STROBE, SCAN_READOUT};
/* Trigger setup                                                */
/* bit mask optionsMask selects TriggerOptions                  */
/* Override of global VCAL and delays with individual values    */
/* from ModuleConfig via special USE_MODULECONFIG_DEFAULT value */
/* custom bitstream for cal trigger command sequence            */
/* define Triggermodes compatible with RODregisters             */
enum TriggMode {SCAN_DSP=0, SCAN_INTERNAL_SELF=0x1, SCAN_INTERNAL_INTERVAL=0x3, SCAN_INTERNAL_SYNCHRONOUS=0x4, 
				SCAN_EXTERNAL, SCAN_TRIGGERMODES};
enum TriggOptions {SCAN_USE_CLOW, SCAN_USE_CHIGH, SCAN_DIGITAL_INJECT, SCAN_HITBUS_ON, SCAN_SPECIFY_CHARGE_NOT_VCAL};
#define SCAN_USE_MODULECONFIG_DEFAULT 0xFFFF
/* Action to be performed after the scan, provision for different fit functions */
enum ScanAct {SCAN_NO_ACTION, SCAN_FIT, SCAN_TUNE_THRESH, SCAN_CALC_THRESH};
enum FitFunc {SCAN_SCURVE, SCAN_TOTCAL, SCAN_GAUSS,SCAN_T0};
/* parameters of one loop, custom spaced data points   */
/* possible at the end of the structure                */
/* currently excerpt of most important scan variables  */
/* TRYING to reproduce the old scan variable numbering */
/* such that old and new code work with the same defs  */
enum ScanParam {SCAN_NONE=0, SCAN_L1ALATENCY=13, SCAN_TDAC=14, 
		SCAN_FDAC=15, SCAN_GDAC=16, SCAN_MONLEAK_DAC,
		SCAN_TRIGGER_DELAY=21, SCAN_VCAL=24, SCAN_STROBE_DELAY=25,

/* Starting of from here, this File is to reproduce the same order as given in
PixEnumBase.h, to be able to commuicate Scanvariables
to the DSP code correctly*/
		
		CHARGE, TDACS_VARIATION, GDAC_VARIATION,
		BOC_BPH, SCAN_BOC_V_CLOCK, BOC_VPH1, BOC_VFINE, BOC_VPH, BOC_BPMPH, BOC_TX_CURR, 
		SCAN_BOC_TX_MSR, SCAN_BOC_RX_THR, SCAN_BOC_RX_DELAY, BOC_RX_THR_DIFF, BOC_RX_DELAY_DIFF,
		BOC_TX_BPM, BOC_TX_BPMF, BOC_BVPH, BPM_INVERT, OB_VISET, OPTO_VISET, BOC_THR_TUNE};

//mw clashes with identical definition in scanOptions.h
//AKDEBUG scanOptions.h is superceded by scanOptions_2.h
enum {
	SCAN_QUERY_STATUS = 0,
	SCAN_GO,
	SCAN_PAUSE,
	SCAN_CLOSE,
	SCAN_KILL
};
/*** SCAN State ***/
enum {
	SCAN_IDLE,
	SCAN_INIT,
	SCAN_ENTER_TRIGGER_LOOP,
	SCAN_TRIGGER_LOOP,
	TRIGGER_LOOP_COMPLETE,
	SCAN_IN_PROGRESS,
	SCAN_END_OF_LOOP_ACTION,
	SCAN_COMPLETE,
	SCAN_TERMINATED,
	SCAN_NUM_STATES
};
enum HistogramType {SCAN_OCCUPANCY, SCAN_TOT, SCAN_TOT_MEAN, SCAN_TOT_SIGMA, SCAN_TOTAVERAGE, SCAN_L1ID, SCAN_HITOCC,
                    SCAN_FITRESULTS, SCAN_MONLEAK_HIST, SCAN_RAW_INMEM, SCAN_BOC_HI, SCAN_ERROR, SCAN_DATAMON, 
                    SCAN_BCID_MEAN, SCAN_BCID_SIGMA, SCAN_SCURVE_MEAN, SCAN_SCURVE_SIGMA, SCAN_SCURVE_CHI2,
                    SCAN_TOTCAL_PIX_TOT_P0, SCAN_TOTCAL_PIX_TOT_P1, SCAN_TOTCAL_PIX_TOT_P2, 
                    SCAN_TOTCAL_PIX_DISP_P0, SCAN_TOTCAL_PIX_DISP_P1, SCAN_TOTCAL_FE_TOT_P0, SCAN_TOTCAL_FE_TOT_P1,
                    SCAN_TOTCAL_FE_TOT_P2, SCAN_TOTCAL_FE_DISP_P0, SCAN_TOTCAL_FE_DISP_P1, SCAN_TOTCAL_FE_SCALE, 
                    SCAN_TOTCAL_PIX_TOT_CHI2, SCAN_TOTCAL_PIX_DISP_CHI2,
                    SCAN_TOTCAL_FE_TOT_CHI2, SCAN_TOTCAL_FE_DISP_CHI2,
                    SCAN_MAXHIST};
enum MaskStageMode  {SCAN_SEL_ENA, SCAN_SEL, SCAN_ENA, SCAN_SEL_ENA_PRE, SCAN_SEL_PRE, SCAN_XTALK, SCAN_SENS_XTALK, 
		     SCAN_MONLEAK, SCAN_STATIC, SCAN_FEI4_ENA_NOCAP, SCAN_FEI4_ENA_SCAP, SCAN_FEI4_ENA_LCAP, 
		     SCAN_FEI4_ENA_BCAP, SCAN_FEI4_ENA_HITBUS, SCAN_FEI4_XTALK };
enum MaskStageSteps {SCAN_STEPS_5, SCAN_STEPS_32, SCAN_STEPS_40, SCAN_STEPS_64, SCAN_STEPS_80, SCAN_STEPS_160, SCAN_STEPS_320,
		     SCAN_STEPS_2880, SCAN_STEPS_1, SCAN_STEPS_2, SCAN_STEPS_3, SCAN_STEPS_4, SCAN_STEPS_6, SCAN_STEPS_8, 
		     SCAN_STEPS_1_DC, SCAN_STEPS_2_DC, SCAN_STEPS_3_DC, SCAN_STEPS_4_DC, SCAN_STEPS_6_DC, SCAN_STEPS_8_DC, 
		     SCAN_STEPS_26880_DC };
enum {
	PHYSICS_CFG,
	SCAN_CFG,
	N_CFG /* number of configuration structures */
};

typedef enum 
{ 
   PIX_FE_NULL          =1,
   PIX_FE_SOFT_RESET    =2,
   PIX_FE_WRITE_GLOBAL  =3,
   PIX_FE_READ_GLOBAL   =4,
   PIX_FE_WRITE_HITBUS  =5,
   PIX_FE_WRITE_SELECT  =6,
   PIX_FE_WRITE_PREAMP  =7,
   PIX_FE_WRITE_ENABLE  =8,
   PIX_FE_WRITE_TDAC0   =9,
   PIX_FE_WRITE_TDAC1   =10,
   PIX_FE_WRITE_TDAC2   =11,
   PIX_FE_WRITE_TDAC3   =12,
   PIX_FE_WRITE_TDAC4   =13,
   PIX_FE_WRITE_TDAC5   =14,
   PIX_FE_WRITE_TDAC6   =15,   
   PIX_FE_WRITE_FDAC0   =16,
   PIX_FE_WRITE_FDAC1   =17,
   PIX_FE_WRITE_FDAC2   =18,
   PIX_FE_WRITE_FDAC3   =19,
   PIX_FE_WRITE_FDAC4   =20,
   PIX_FE_READ_PIXEL    =21,
   PIX_FE_REF_RESET     =22,
   PIX_FE_EC_RESET      =23,
   PIX_FE_BC_RESET      =24
   
} FE_CMD_ID; 

typedef enum {
   PIX_MC_TRIGGER       =101,
   PIX_MC_ECR                   =102,
   PIX_MC_BCR                   =103,
   PIX_MC_SYNC                  =104,
   PIX_EDUMMY           =105,
   PIX_MC_CAL                   =106
   
} MCFAST_CMD_ID;

#endif
