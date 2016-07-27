#ifndef PixelFEI4BConfig_HH
#define PixelFEI4BConfig_HH

#include "PixelFEI4GenConfig.hh"

namespace ipc{

struct PixelFEI4BGlobal{ //FE global register
  unsigned short TrigCnt;
  unsigned short Conf_AddrEnable;
  unsigned short Reg2Spare;
  unsigned short ErrMask0;
  unsigned short ErrMask1;
  unsigned short PrmpVbpRight;
  unsigned short BufVgOpAmp;
  unsigned short Reg6Spare;
  unsigned short PrmpVbp;
  unsigned short TdacVbp;
  unsigned short DisVbn;
  unsigned short Amp2Vbn;
  unsigned short Amp2VbpFol;
  unsigned short Reg9Spare;
  unsigned short Amp2Vbp;
  unsigned short FdacVbn;
  unsigned short Amp2Vbpf;
  unsigned short PrmpVbnFol;
  unsigned short PrmpVbpLeft;
  unsigned short PrmpVbpf;
  unsigned short PrmpVbnLcc;
  unsigned short Reg13Spare;
  unsigned short PxStrobes;
  unsigned short S0;
  unsigned short S1;
  unsigned short LVDSDrvIref;
  unsigned short GADCOpAmp;
  unsigned short PllIbias;
  unsigned short LVDSDrvVos;
  unsigned short TempSensBias;
  unsigned short PllIcp;
  unsigned short Reg17Spare;
  unsigned short PlsrIdacRamp;
  unsigned short VrefDigTune;
  unsigned short PlsrVgOPamp;
  unsigned short PlsrDacBias;
  unsigned short VrefAnTune;
  unsigned short Vthin_AltCoarse;
  unsigned short Vthin_AltFine;
  unsigned short PlsrDAC;
  unsigned short DIGHITIN_Sel;
  unsigned short DINJ_Override;
  unsigned short HITLD_In;
  unsigned short Reg21Spare;
  unsigned short Reg22Spare2;
  unsigned short Colpr_Addr;
  unsigned short Colpr_Mode;
  unsigned short Reg22Spare1;
  unsigned short DisableColumnCnfg0;
  unsigned short DisableColumnCnfg1;
  unsigned short DisableColumnCnfg2;
  unsigned short TrigLat;
  unsigned short CMDcnt;
  unsigned short StopModeCnfg;
  unsigned short HitDiscCnfg;
  unsigned short EN_PLL;
  unsigned short Efuse_sense;
  unsigned short Stop_Clk;
  unsigned short ReadErrorReq;
  unsigned short Reg27Spare1;
  unsigned short GADC_Enable;
  unsigned short ShiftReadBack;
  unsigned short Reg27Spare2;
  unsigned short GateHitOr;
  unsigned short CalEn;
  unsigned short SR_clr;
  unsigned short Latch_en;
  unsigned short SR_Clock;
  unsigned short LVDSDrvSet06;
  unsigned short Reg28Spare;
  unsigned short EN40M;
  unsigned short EN80M;
  unsigned short CLK1;
  unsigned short CLK0;
  unsigned short EN160M;
  unsigned short EN320M;
  unsigned short Reg29Spare1;
  unsigned short no8b10b;
  unsigned short Clk2OutCnfg;
  unsigned short EmptyRecord;
  unsigned short Reg29Spare2;
  unsigned short LVDSDrvEn;
  unsigned short LVDSDrvSet30;
  unsigned short LVDSDrvSet12;
  unsigned short TempSensDiodeSel;
  unsigned short TempSensDisable;
  unsigned short IleakRange;
  unsigned short Reg30Spare;
  unsigned short PlsrRiseUpTau;
  unsigned short PlsrPwr;
  unsigned short PlsrDelay;
  unsigned short ExtDigCalSW;
  unsigned short ExtAnaCalSW;
  unsigned short Reg31Spare;
  unsigned short GADCSel;
  unsigned short SELB0;
  unsigned short SELB1;
  unsigned short SELB2;
  unsigned short Reg34Spare1;
  unsigned short PrmpVbpMsnEn;
  unsigned short Reg34Spare2;
  unsigned short Chip_SN;
  unsigned short Reg1Spare;
  unsigned short SmallHitErase;
  unsigned short Eventlimit;
} ;				     

struct PixelFEI4BConfig{ //FE level parameters
        char             idStr[16]; //Module ID string
	unsigned long    FEIndex; 	
	PixelFECommand   FECommand;
	PixelFEI4BGlobal FEGlobal;
        unsigned int     FEMasks[IPC_N_I4_PIXEL_COLUMNS][IPC_N_I4_PIXEL_ROWS]; // all masks combined, i.e. 4 bits per pixel
	PixelFEI4Trims   FETrims;
	PixelFECalibFEI4 FECalib;
} ;

};

#endif
