#ifndef PixelFEI4Config_IDL
#define PixelFEI4Config_IDL

namespace ipc{

  typedef std::vector<unsigned long> uvec;

  const long IPC_N_I4_PIXEL_COLUMNS = 80;
  const long IPC_N_I4_PIXEL_ROWS = 336;
  const long enable = 0;
  const long largeCap = 1;
  const long smallCap = 2;
  const long hitbus = 3;

struct PixelFECommand{ //FE Command register
	unsigned int address;		       /* 5 bits */
	unsigned long command;            /* 32 bits */
} ;

struct PixelFECalibFEI4{ 
/* Sub-structure for calibration of injection-capacitors, VCAL-DAC and
   leakage current measurement */
	float cinjLo;
	float cinjHi;
	float vcalCoeff[4];
	float chargeCoeffClo;  
	float chargeCoeffChi;
	float chargeOffsetClo;
	float chargeOffsetChi;
	float monleakCoeff;
} ; 

struct PixelFEI4Trims{ //Trim DACs:
  unsigned int dacThresholdTrim[IPC_N_I4_PIXEL_COLUMNS][IPC_N_I4_PIXEL_ROWS];  /* 5 bits per pixel */
  unsigned int dacFeedbackTrim [IPC_N_I4_PIXEL_COLUMNS][IPC_N_I4_PIXEL_ROWS];  /* 4 bits per pixel */
} ; 

};

#endif
