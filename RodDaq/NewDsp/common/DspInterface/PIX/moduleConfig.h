#ifndef MODULE_CONFIG_H
#define MODULE_CONFIG_H

/* pixel order
	HITBUS, SELECT, MASK,
	TDAC0, TDAC1, TDAC2, TDAC3, TDAC4, TDAC5, TDAC6,
	FDAC0, FDAC1, FDAC2,
	KILL 
 */

enum {
	pixel_hitbus,
	pixel_select,
	pixel_enable,
	pixel_tdac0,
	pixel_tdac1,
	pixel_tdac2,
	pixel_tdac3,
	pixel_tdac4,
	pixel_tdac5,
	pixel_tdac6,
	pixel_fdac0,
	pixel_fdac1,
	pixel_fdac2,
	pixel_kill
};

#define FE_I3_N_GLOBAL_BITS 231

/* MCC registers */
#define MCC_CSR	0x0
#define MCC_LV1	0x1
#define MCC_FEEN	0x2
#define MCC_WFE	0x3
#define MCC_WMCC	0x4
#define MCC_CNT	0x5
#define MCC_CAL	0x6
#define MCC_PEF	0x7

/* MCC commands */
#define MCC_CMD_LV1	0x0
#define MCC_CMD_BCR	0x1
#define MCC_CMD_ECR	0x2
#define MCC_CMD_CAL	0x4
#define MCC_CMD_SYNC	0x8

/* Front End Command Register Bits */
#define FE_CMD_NULL				0x000000
#define FE_CMD_REF_RESET		0x000002
// Changed PDJ Feb 10, was previously 0x00000C
#define FE_CMD_SOFT_RESET		0x000004 
#define FE_CMD_CLOCK_GLOBAL	0x000010
#define FE_CMD_WRITE_GLOBAL	0x000060
#define FE_CMD_READ_GLOBAL		0x000080
#define FE_CMD_CLOCK_PIXEL		0x000100
#define FE_CMD_WRITE_HITBUS	0x000200
#define FE_CMD_WRITE_SELECT	0x000400
#define FE_CMD_WRITE_MASK		0x000800
#define FE_CMD_WRITE_TDAC0		0x001000
#define FE_CMD_WRITE_TDAC1		0x002000
#define FE_CMD_WRITE_TDAC2		0x004000
#define FE_CMD_WRITE_TDAC3		0x008000
#define FE_CMD_WRITE_TDAC4		0x010000
#define FE_CMD_WRITE_TDAC5		0x020000
#define FE_CMD_WRITE_TDAC6		0x040000
#define FE_CMD_WRITE_FDAC0		0x080000
#define FE_CMD_WRITE_FDAC1		0x100000
#define FE_CMD_WRITE_FDAC2		0x200000
#define FE_CMD_WRITE_KILL		0x400000
#define FE_CMD_READ_PIXEL		0x800000

typedef struct {
//size = 82 bytes 
	unsigned char parity;
	unsigned char latency;
	unsigned char selfTriggerDelay;
	unsigned char selfTriggerWidth;
	unsigned char enableSelfTrigger;
	unsigned char enableHitParity;
	unsigned char doMux;
	unsigned char selectMonHit;
	unsigned char tsiTscEnable;
	unsigned char selectDataPhase;
	unsigned char enableEOEParity;
	unsigned char hitBusScaler;
	unsigned char monLeakADCRefTest;
	UINT16 monLeakADCDAC;
	unsigned char monLeakDACTest;
	unsigned char monLeakADCEnableComparator;
	unsigned char monLeakADCMonComp;
	unsigned char aRegTrim;
	unsigned char enableARegMeas;
	unsigned char aRegMeas;
	unsigned char enableAReg;
	unsigned char enableLVDSRefMeas;
	unsigned char dRegTrim;
	unsigned char enableDRegMeas;
	unsigned char dRegMeas;
	unsigned char capMeasCircuitry;
	
	unsigned char enableCapTest;
	unsigned char enableAnalogOut;
	unsigned char enableTestPixelMux;
	unsigned char enableVCalMeas;
	unsigned char enableLeakMeas;
	unsigned char enableBufferBoost;
	
	unsigned char enableCP8;
	unsigned char testDacForIVDD2Dac;
	unsigned char dacIVDD2;
	unsigned char dacID;
	unsigned char testDacForIDDac;
	
	unsigned char enableCP7;
	unsigned char testDacForIP2Dac;
	unsigned char dacIP2;
	unsigned char dacIP;
	unsigned char testDacForIPDac;
	
	unsigned char enableCP6;
	unsigned char testDacForITrimThDac;
	unsigned char dacITRIMTH;
	unsigned char dacIF;
	unsigned char testDacForIFDac;
	
	unsigned char enableCP5;
	unsigned char testDacForITrimIfDac;
	unsigned char dacITRIMIF;
	UINT16 dacVCAL;
	unsigned char testDacForVCalDac;
	
	unsigned char enableCP4;
	unsigned char enableCinjHigh;
	unsigned char enableExternalInj;
	unsigned char enableTestAnalogRef;
	unsigned char eocMux;
	unsigned char CEUClockControl;
	
	unsigned char enableDigitalInject;
	unsigned char enableCP3;
	unsigned char testDacForITH1Dac;
	unsigned char dacITH1;
	unsigned char dacITH2;
	unsigned char testDacForITH2Dac;
	
	unsigned char enableCP2;
	unsigned char testDacILDac;
	unsigned char dacIL;
	unsigned char dacIL2;
	unsigned char testDacIL2Dac;
	
	unsigned char enableCP1;
	unsigned char threshTOTMinimum;
	unsigned char threshTOTDouble;
	unsigned char modeTOTThresh;
	
	unsigned char enableCP0;
	unsigned char enableHitbus;
	unsigned char gTDac;
	unsigned char enableTune;
	unsigned char enableBiasComp;
	unsigned char enableIpMonitor;

} GlobalRegister;

typedef struct {

	unsigned char enableIpMonitor		;
	unsigned char enableBiasComp		;
	unsigned char enableTune			;
	unsigned char gTDac					[5];
	
	unsigned char enableHitbus			;	
	unsigned char enableCP0				;
//
	unsigned char modeTOTThresh		[2];
	
	unsigned char threshTOTDouble		[8];
	
	unsigned char threshTOTMinimum	[8];
	
	unsigned char enableCP1				;
	unsigned char testDacIL2Dac		;
	unsigned char dacIL2					[8];
	unsigned char dacIL					[8];
	unsigned char testDacILDac			;
	unsigned char enableCP2				;
	
	unsigned char testDacForITH2Dac	;
	unsigned char dacITH2				[8];
	unsigned char dacITH1				[8];
	unsigned char testDacForITH1Dac	;
	unsigned char enableCP3				;
	unsigned char enableDigitalInject;

	unsigned char CEUClockControl		[2];
	unsigned char eocMux					[2];
	unsigned char enableTestAnalogRef;
	unsigned char enableExternalInj	;
	unsigned char enableCinjHigh		;
	unsigned char enableCP4				;
	
	unsigned char testDacForVCalDac		;
	unsigned char dacVCAL					[10];
	unsigned char dacITRIMIF				[8];
	unsigned char testDacForITrimIfDac	;
	unsigned char enableCP5					;
	
	unsigned char testDacForIFDac			;
	unsigned char dacIF						[8];	
	unsigned char dacITRIMTH				[8];
	unsigned char testDacForITrimThDac	;
	unsigned char enableCP6					;
	
	unsigned char testDacForIPDac;
	unsigned char dacIP						[8];
	unsigned char dacIP2						[8];
	unsigned char testDacForIP2Dac;
	unsigned char enableCP7;
	
	unsigned char testDacForIDDac;
	unsigned char dacID						[8];
	unsigned char dacIVDD2					[8];
	unsigned char testDacForIVDD2Dac;
	unsigned char enableCP8;
	
	unsigned char enableBufferBoost;
	unsigned char enableLeakMeas;
	unsigned char enableVCalMeas;
	unsigned char enableTestPixelMux		[2];
	unsigned char enableAnalogOut;	
	unsigned char enableCapTest;

	unsigned char capMeasCircuitry		[6];
	unsigned char dRegMeas					[2];
	unsigned char enableDRegMeas			;
	unsigned char dRegTrim					[2];
	unsigned char enableLVDSRefMeas		;
	unsigned char enableAReg				;
	unsigned char aRegMeas					[2];
	unsigned char enableARegMeas			;
	unsigned char aRegTrim					[2];

	unsigned char monLeakADCMonComp		;
	unsigned char monLeakADCEnableComparator	;
	unsigned char monLeakDACTest			;
	unsigned char monLeakADCDAC			[10];
	unsigned char monLeakADCRefTest		;
	unsigned char hitBusScaler				[8];
	unsigned char enableEOEParity			;
	unsigned char selectDataPhase			;
	unsigned char tsiTscEnable				;

	unsigned char selectMonHit				[4];
	unsigned char doMux						[4];
	unsigned char enableHitParity			;
	unsigned char enableSelfTrigger		;
	unsigned char selfTriggerWidth		[4];
	unsigned char selfTriggerDelay		[4];	
	unsigned char latency					[8];

	unsigned char parity;
	unsigned char rsvd; /* to ensure word alignment */

} GlobalRegisterRaw;

#if 0

typedef struct {
	unsigned short int hitBus : 1;
	unsigned short int select : 1;
	unsigned short int mask : 1; /* enable readout */
	unsigned short int tdac : 7;
	unsigned short int fdac : 3;
	unsigned short int kill : 1; /* preamp. 1 = enable preamp */
} PixelRegister;

#else

typedef UINT16 PixelRegister;

#endif

typedef struct {
	int csr; /* control and status register */
	int lv1; /* number of contiguous L1 to be issued to fe's */
	int feen; /* front end enable */
	int wfe, wmcc; /* warnings from fe and mcc. read only */
	int cnt;
	int cal; /* calibration pulse delay settings */
	int pef; /* pending events fifo */
} ModuleMCC;

typedef struct {
/* this needs to be first so that word alignment occurs properly */
	PixelRegister pixelRegister[N_COLS][N_ROWS];
	int id;
	GlobalRegister globalRegister;
} ModuleFE;

typedef struct {
	ModuleFE fe[N_CHIPS];
	ModuleMCC mcc;
} ModuleConfig;

typedef struct {
  float vcalCoeff[4];
  float capVal[2];
} CalibFE;

typedef struct {
	CalibFE fe[N_CHIPS];
} ModuleCalib;

#endif
