/************************************************************************************
 * bocStructure.h                                            
 *
 *  synopsis:  Collects together all the BOC settings for a given set of command &
 *             data links. This structure is intended to be incorporated into the
 *             ABCDModule & PixelModule structures.    
 *
 *  Douglas Ferguson, UW Madison                              dpferguson@lbl.gov
 *  Peter W Phillips, Oxford University                       P.W.Phillips@rl.ac.uk
 *  John Richardson, Lawrence Berkeley Laboratory             john.richardson@cern.ch
 *  Joseph Virzi, University of California                    jsvirzi@lbl.gov
 ************************************************************************************/
#ifndef BOC_STRUCTURE_H
#define BOC_STRUCTURE_H
/* register definitions for the BOC setting routines' case statements; note that some
   of these closely match the RRIF register definitions in registerIndices.h they are
   abbreviated here. (Some of the indices in rI.h are for register sets and so do not
   work well in the case statement). */
#define BOC_VAR_TXFINE       0x90
#define BOC_VAR_TXCOARSE     0x91
#define BOC_VAR_TXMARKSPACE  0x92
#define BOC_VAR_TXINHIBIT    0x93
#define BOC_VAR_TXCURRENT    0x94

#define BOC_VAR_RXTHRESHOLD  0xa0
#define BOC_VAR_RXDELAY      0xa1

#define BOC_VAR_BPM_CLOCK_PHASE            0xb0
#define BOC_VAR_BREG_CLOCK_PHASE           0xb1
#define BOC_VAR_VERNIER_CLOCK_STEPPHASE0   0xb2
#define BOC_VAR_VERNIER_CLOCK_STEPPHASE1   0xb3
#define BOC_VAR_VERNIER_CLOCK_FINEPHASE    0xb4

#define BOC_VAR_BOCRESET       0xc0
#define BOC_VAR_BPMRESET       0xc1
#define BOC_VAR_TXDAC_CLEAR    0xc2
#define BOC_VAR_RXDAC_CLEAR    0xc3

#define BOC_VAR_STAT           0xd0
#define BOC_VAR_RXDATAMODE     0xd1
#define BOC_VAR_CLOCKCTRLBITS  0xd2

#define BOC_VAR_FIRMWARE_REV    0xe0
#define BOC_VAR_HARDWARE_REV    0xe1
#define BOC_VAR_MODULETYPE      0xe2
#define BOC_VAR_MNFT            0xe3
#define BOC_VAR_SERIALNO        0xe4

typedef struct {
  UINT8 txFineDelay;      //transmitter fine delay. 
  UINT8 txCoarseDelay;    //transmitter coarse delay.
  UINT8 txMarkSpace;      //transmitter markSpace ratio.
  UINT8 txStreamInhibit;  //0 ==> inhibit channel.

  UINT8 txCurrent;        //Laser current.
  UINT8 unused[3];
} BOCTxParams;

typedef struct {
  UINT8 rxThreshold;      //receiver threshold.
  UINT8 rxDelay;          //receiver delay.
} BOCRxParams;

typedef struct {
  UINT8 bpmClockPhase;
  UINT8 bRegClockPhase;
  UINT8 vernierClockStepPhase[2];
  UINT8 vernierClockFinePhase;

  UINT8 bocReset;  //needed?
  UINT8 bpmReset;  //  "
  UINT8 txDacClear; // " 
  UINT8 rxDacClear; // " 

  UINT8 status;
  UINT8 rxDataMode;
  UINT8 clockCtrlBits;

  UINT8 firmwareRev;
  UINT8 hardwareRev;
  UINT8 moduleType;
  UINT8 manufacturer;
  UINT8 serialNumber;

  UINT8 unused[3];    //data alignment.
  UINT32 unused1[3];
} BOCGlobalParams;

typedef struct {
  BOCTxParams      bocTxParams[48];
  BOCRxParams      bocRxParams[96];
  BOCGlobalParams  bocGlobalParams;
} BOCConfig;

#endif

