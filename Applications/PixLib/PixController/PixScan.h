/////////////////////////////////////////////////////////////////////
// PixScan.h
/////////////////////////////////////////////////////////////////////
//
// 17/06/05  Version 1.2 (PM)
//           Explicit separation of scan loop start and
//           scan step preparation
//           Some modification to the proposal to allow
//           - execution of mask staging a tthe host level
//           - execution of loops with values dynamically
//             choosen by PixModuleGroup guring the scan
//

/*

PixScan is a structure describing a scan and containing
scan results. It's not responsible for the execution of
the scan; this task is performed by PixModuleGroup, which
in turns uses PixController methods to activate the DSPs.
PixScan however can contain helper funcions used during the
execution of a scan (e.g. histogram manipulation and fit).
PixScan can be used as a carrier to move scan reults from a
processor to another, having the ability to save and retreive
data from a permanent storage; it can also implement intelligent
caching mechanism to allow a processor to access a large
set of histograms minimizing memory occupation.

The structure is designed for a three-loop scan. The innermost
loop (which implicitly includes the mask stagin as an extra
loop) is usually performed by the ROD DSPs. The second loop can be
performed by the ROD or by the host while the third loop is
usually performed by the host.

PixScan defines four groups of modules. The histograms
of every group are collected and processed by the corresponding
SDSP. Modules not assigned to any group are ignored during
the scan. The activity of the modules of a group during a scan
is controlled by four parameters:

configure : if true the modules are configured at the beginning of the scan
trigger   : if true the modules receive LVL1 commands
strobe    : if true the modules receive STROBE commands
readout   : if true the modules are read-out

It's possible to specify a fixed delay between strobe/trigger
sequences sent to groups 1,2 and 3,4. Finally, it's possible
to split a single event (which is usually made of 16 consecutive
LVL1) in two parts of N amd M (N+M<=16) consecutive LVL1
separated by a fixed time gap. Two different settings are possible,
one for groups 1,2 and the other for groups 3,4.

There is a set of scan attributes and a set of loop attributes.
Scan attributes are parameters which are set at the beginning of
a scan and remains constant during the entire scan.
Loop attributes indicates specific properties of every loop,
like the variable being scanned during the loop or the operation
to e performed at the end of the loop. PixScan includes a set of
hidden attributes; these are not intended to be modified by the
user, so no interface is provided; hidden attributes are computed
automatically when needed, based on scan and loop attributes,
and passed to the PixController.

Scan attributes
---------------
- Configuration set to use
- Concurrent scan or FE by FE
- Mask stage mode
- Mask stage # of steps
- Mask stage # of steps to execute
- Inversion between mask stage and innermost loop
- Number of events in the innermost loop
- Enable self trigger
- Trigger/Strobe delay
- LVL1 latency
- Strobe duration
- Module mask (gr 1-4)
- Configuration enabled (gr 1-4)
- Trigger enabled (gr 1-4)
- Strobe enabled (gr 1-4)
- Readout enabled (gr 1-4)
- Trigger delay gr 1,2-3,4
- Number of consecutive LVL1 in Trigger group A (gr 1,2 3,4)
- Number of consecutive LVL1 in Trigger group B (gr 1,2 3,4)
- Delay between trigger group 1 and 2 (gr 1,2 3,4)
- Type of histogramming routine to be used in the DSPs
- Histograms to be filled during the scan
- TOT histogams min, max and # of bins
- Raw histograms to be kept till the end of the scan
- Analog or digital injection
- Charge injection capacitor

Loop attributes
---------------
- Parameter being scanned during the loop
- Loop execution in the DSP or in the Host
- Array of values for the scan parameter or
  Min, Max and number of steps or sequence not predefined
- Operation to be performed at the end of the loop (e.g.
  s-curve fit or tdac adjust).
- Use of DSP for the end loop action

This examples illustrates how PixScan used to perform a
scan:

PixScan *scn;
PixModuleGroup *grp;

scn->resetScan();
grp->initScan(scn);

grp->scanLoopStart(2, scn);
while (scn->loop(2)) {
  grp->prepareStep(2,scn);
  grp->scanLoopStart(1, scn);
  while (scn->loop(1)) {
    grp->prepareStep(1,scn);
    grp->scanLoopStart(0, scn);
    while (scn->loop(0)) {
      grp->prepareStep(0,scn);
      grp->scanExecute(scn);
      std::cout << "Completed loop: " << scn->scanIndex(2) << "/";
      std::cout << scn->scanIndex(1) << "/" << scn->scanIndex(0) << std::endl;
      grp->scanTerminate(scn);
      scn->next(0);
    }
    grp->scanLoopEnd(0, scn);
    scn->next(1);
  }
  grp->scanLoopEnd(1, scn);
  scn->next(2);
}
grp->scanLoopEnd(2, scn);

grp->terminateScan(scn);

grp->initScan(scn) has to prepare the module group for the scn.
This includes, for egxample, the regulation of the voltages via DCS

grp->scanLoopStart(index, scn) prepares the module group for the
beginning of a particular loop. This includes any hardware or
software setting to be performed once at the beginning of the loop,

grp->prepareStep(index, scn) the typical operation to perform
in this phase is the setting of the loop scan variable to the
appropriate value. For the innermost loop (index = 0) this
operation has to be performed only if scn->newScanStep() returns
TRUE; in case the mask staging is under the responsability
of the host, this method will also setup the appropriate mask
when scn->newMaskStep() returns TRUE.

grp->scanExecute(scn) executes a step of the scan. If the loop is
executed in the DSP this method will setup the ROD and complete an
entire loop.

grp->scanLoopEnd(index, scn) will perform the end-of-loop actions.
Typically this method will upload histograms or fit results from
the ROD and store them in the appropriate PixScan structures; it
will also compute the results needed for the execution of the
subsequent step.

grp->terminateScan(scn) executes end od scan actions, like resotring
the initial module configuration if needed.

*/

#ifndef _PIXLIB_PIXSCAN
#define _PIXLIB_PIXSCAN

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <vector>
#include <list>
#include <map>
#include <string>

#include "PixController/PixController.h"
#include "Histo/Histo.h"

namespace PixLib {

class Config;
class PixControllerExc;
class PixConfDBInterface;
class DBInquire;

//! Pix Controller Exception class; an object of this type is thrown in case of a controller error
class PixScanExc : public PixControllerExc {
 public:
  //! Constructor
  PixScanExc(ErrorLevel el, std::string name) : PixControllerExc(el, name) {};
  //! Destructor
  virtual ~PixScanExc() {};
 private:
};

#define MAX_GROUPS 4
#define MAX_SGROUPS 2
#define MAX_LOOPS 3

class PixScanHisto {
public:
  PixScanHisto();
  PixScanHisto(Histo &h);
  PixScanHisto(unsigned int ih, Histo &h);
  PixScanHisto(unsigned int ix, PixScanHisto &sh);
  ~PixScanHisto();

  void add(unsigned int ix, PixScanHisto &sh);
  void add(unsigned int ih, Histo &h);
  void add(unsigned int ih, std::vector< Histo * >&h);
  bool exists(unsigned int ix);
  bool histoExists(unsigned int ih);
  unsigned int size();
  unsigned int histoSize();
  bool histoMode();
  PixScanHisto &operator[](unsigned int ix);
  Histo &histo();
  Histo &histo(unsigned int ih);
  void clear();

private:
  std::map<unsigned int, Histo*> *m_histo;
  std::map<unsigned int, PixScanHisto*> *m_map;
  bool m_histoMode;
  std::string m_id;
  bool m_cached;
};

class PixScan {

  friend class PixController;

public:
  enum ScanType       { DIGITAL_TEST, ANALOG_TEST, THRESHOLD_SCAN, TOT_CALIB, TDAC_TUNE, GDAC_TUNE,
                        FDAC_TUNE, IF_TUNE, TIMEWALK_MEASURE, INCREMENTAL_TDAC_SCAN, BOC_RX_DELAY_SCAN,
                        BOC_THR_RX_DELAY_SCAN, BOC_V0_RX_DELAY_SCAN, INTIME_THRESH_SCAN, T0_SCAN, CROSSTALK_SCAN,
                        IV_SCAN, DAC_SCAN, MONLEAK, HITBUS_SCALER, SOURCE_SCAN, EUDAQ, TOT_VERIF, NOISE_OCC, INJ_CALIB,
						TDAC_FAST_TUNE, GDAC_FAST_TUNE, GDAC_FAST_TUNE2, HITOR_SCAN, FE_ST_SOURCE_SCAN, TOT_CALIB_LUT, CHARGE_CALIB_VERIF, 
						DISCBIAS_TUNE, LASER_SCAN};
  enum ModConfigType  { PHYSICS, CALIB,
                        SPARE };
  enum MaskStageMode  { SEL_ENA, SEL,
                        ENA, XTALK,
                        SEL_ENA_PRE,
                        SEL_PRE,
						HITB, ENA_HITB, SEL_ENA_HITB, STATIC, HITB_INV, DINJ };
  enum MaskStageSteps { STEPS_1, STEPS_2, STEPS_3, STEPS_4, STEPS_6, STEPS_8, STEPS_32, STEPS_336, STEPS_672, STEPS_26880, STEPS_USER, STEPS_40, STEPS_64, STEPS_80, STEPS_160, STEPS_320, STEPS_2880};
  enum DspHistoCode   { ASM, C };
  enum DspHistoDepth  { BITS_8, BITS_16,
                        BITS_32};
  enum HistogramType  { OCCUPANCY = PixController::OCCUPANCY,
                        LVL1 = PixController::LVL1,
			LV1ID = PixController::LV1ID,
			BCID = PixController::BCID,
                        TOT = PixController::TOT,
                        TOT_MEAN = PixController::TOT_MEAN,
                        TOT_SIGMA = PixController::TOT_SIGMA,
                        SCURVE_MEAN = PixController::SCURVE_MEAN,
                        SCURVE_SIGMA = PixController::SCURVE_SIGMA,
                        SCURVE_CHI2 = PixController::SCURVE_CHI2,
                        HITOCC = PixController::HITOCC,
                        TOTAVERAGE = PixController::TOTAVERAGE,
			TOTCAL_PARA = PixController::TOTCAL_PARA,
			TOTCAL_PARB = PixController::TOTCAL_PARB,
			TOTCAL_PARC = PixController::TOTCAL_PARC,
			TOTCAL_CHI2 = PixController::TOTCAL_CHI2,
			DSP_ERRORS = PixController::DSP_ERRORS,
			CLUSTER_TOT = PixController::CLUSTER_TOT,
			CLUSTER_SIZE = PixController::CLUSTER_SIZE,
			CLSIZE_TOT = PixController::CLSIZE_TOT,
			SEED_TOT = PixController::SEED_TOT,
			SEED_LVL1 = PixController::SEED_LVL1,
			NSEEDS = PixController::NSEEDS,
			TOT0 = PixController::TOT0, TOT1 = PixController::TOT1, TOT2 = PixController::TOT2,
			TOT3 = PixController::TOT3, TOT4 = PixController::TOT4, TOT5 = PixController::TOT5,
			TOT6 = PixController::TOT6, TOT7 = PixController::TOT7, TOT8 = PixController::TOT8,
			TOT9 = PixController::TOT9, TOT10 = PixController::TOT10, TOT11 = PixController::TOT11,
			TOT12 = PixController::TOT12, TOT13 = PixController::TOT13, TOT14 = PixController::TOT14,
			TOT15 = PixController::TOT15,
			CLUSTER_CHARGE = PixController::CLUSTER_CHARGE, CLSIZE_CHARGE = PixController::CLSIZE_CHARGE, 
			CLUSTER_POSITION = PixController::CLUSTER_POSITION, //DLP
                        MON_LEAK = PixController::MON_LEAK, 
                        RAW_DATA_REF = PixController::RAW_DATA_REF, RAW_DATA_DIFF_2 = PixController::RAW_DATA_DIFF_2, 
                        RAW_DATA_DIFF_1, RAW_DATA_0, RAW_DATA_1,
			TDAC_T, TDAC_THR, GDAC_T, GDAC_THR, FDAC_T,
                        FDAC_TOT, IF_T, IF_TOT, TIMEWALK,
			DCS_DATA, HB_SCALER_CNT,
			FIT_PAR0, FIT_PAR1, FIT_PAR2, FIT_PAR3, FIT_PAR4, FIT_CHI2, TDAC_OCC, SUM_OCC,
			HIT_RATE, TRG_RATE, GDAC_OCC, CHARGE_MEAN, CHARGE_SIGMA, CHARGE_RECO_ERROR, MEAN_NOCC, NOCC, NUM_NOISY_PIXELS, DISCBIAS_T, DISCBIAS_TIMEWALK, MAX_HISTO_TYPES};
  enum DcsReadMode    { VOLTAGE, CURRENT, FE_ADC };
  enum ScanParam      { NO_PAR,  VCAL, GDAC, IF, LATENCY, TDACS, FDACS, FEI4_GR, FEI3_GR, 
                        TRIGGER_DELAY, STROBE_DURATION, STROBE_DELAY, TDACS_VARIATION,
			DCS_VOLTAGE, DCS_PAR1, DCS_PAR2, DCS_PAR3, CAPMEAS, CAPSEL, 
			AUXFREQ, IREF_PAD, INCR_LAT_TRGDEL, IOMUX_IN, STROBE_FINE_DELAY, DISCBIAS, STRBFREQ};
// remember just in case they are needed again
/*                         BOC_BPH, BOC_VPH0, BOC_VPH1, BOC_VFINE, BOC_BPMPH, BOC_TX_CURR, BOC_TX_MS, BOC_RX_THR, BOC_RX_DELAY, */
/*                         BOC_TX_BPM, BOC_TX_BPMF, BOC_BVPH, */
  enum EndLoopAction  { NO_ACTION, SCURVE_FIT, TDAC_TUNING, GDAC_TUNING, T0_SET, FDAC_TUNING,
			IF_TUNING, MIN_THRESHOLD, MCCDEL_FIT, TOTCAL_FIT, FDAC_TUNING_ALT, OFFSET_CALIB,
			TDAC_FAST_TUNING, OCC_SUM, GDAC_FAST_TUNING, TOTCAL_FEI4, CLEAR_IOMUX_BITS, TOT_CHARGE_LUT, 
			TOT_TO_CHARGE, CALC_MEAN_NOCC, SCURVE_FAST, DISCBIAS_TUNING};
  enum RunType        { NORMAL_SCAN, RAW_PATTERN, RAW_EVENT };
  enum CountType      {COUNT_TRIGGER, COUNT_DH, COUNT_DR, COUNT_SECS};
  enum TriggerType    {STROBE_SCAN=0, USBPIX_SELF_TRG=1, EXT_TRG=2, TLU_SIMPLE=3, TLU_DATA_HANDSHAKE=4, USBPIX_REPLICATION_SLAVE=5, 
		       STROBE_EXTTRG=6, FE_SELFTRIGGER=7, STROBE_USBPIX_SELF_TRG=8, STROBE_FE_SELF_TRG=9};



private:
  // Global scan config
  ModConfigType m_modConfig;
  bool m_modScanConcurrent;
  MaskStageMode m_maskStageMode;
  MaskStageSteps m_maskStageTotalSteps;
  int m_maskStageSteps;
  int m_maskStageStepWidth;
  int m_repetitions;
  int m_srcCountType;
  int m_srcTriggerType;
  bool m_selfTrigger;
  bool m_strobeLVL1DelayOverride;
  int m_strobeLVL1Delay;
  int m_LVL1Latency;
  int m_strobeDuration;
  int m_strobeFineDelay;
  int m_strobeFrequency;
  int m_strobeMCCDelay;
  int m_strobeMCCDelayRange;
  unsigned int m_moduleMask[MAX_GROUPS];
  bool m_configEnabled[MAX_GROUPS];
  bool m_triggerEnabled[MAX_GROUPS];
  bool m_strobeEnabled[MAX_GROUPS];
  bool m_readoutEnabled[MAX_GROUPS];
  int m_superGroupTrigDelay;
  int m_consecutiveLvl1TrigA[MAX_SGROUPS];
  int m_consecutiveLvl1TrigB[MAX_SGROUPS];
  int m_trigABDelay[MAX_SGROUPS];
  bool m_lvl1HistoBinned;
  bool m_histogramFilled[MAX_HISTO_TYPES];
  bool m_histogramKept[MAX_HISTO_TYPES];
  int m_getNptsRateAvg;
  int m_maxColClusterDist;
  int m_maxRowClusterDist;
  int m_maxClusterBcidDepth;
  int m_minClusterHits;
  int m_maxClusterHits;
  int m_maxClusterHitTot;
  int m_maxEventIncomplete;
  int m_maxEventErrors;
  bool m_digitalInjection;
  int m_chargeInjCap;
  int m_feVCal;
  bool m_restoreModuleConfig;
  RunType m_runType;
  std::vector<unsigned int> m_patternSeeds;
  // DLP: added for advanced DCS scanning:
  std::string m_scanPar1DcsChan;	//channel to send a parameter 1 to
  std::string m_scanPar1DcsPar;	//parameter 1 parameter (voltage,...)
  std::string m_scanPar2DcsChan;	//channel to send a parameter 2 to
  std::string m_scanPar2DcsPar;	//parameter 2 parameter (voltage,...)
  std::string m_scanPar3DcsChan;	//channel to send a parameter 3 to
  std::string m_scanPar3DcsPar;	//parameter 3 parameter (voltage,...)
  // JGK: added for DCS scanning:
  std::string m_readDcsChan;
  DcsReadMode m_readDcsMode;
  std::string m_scanDcsChan;
  bool m_addModNameToDcs;
  //JW: source scan flag and raw data file
  bool m_isSourceScan;
  std::string m_sourceRawFile;
  //SS: testbeam flags
  bool m_isTestBeam;
  bool m_skipSourceScanHistos;
  int m_lowerLVL1DelayIfSlaveBy;
  int m_TLUTriggerDataDelay;
  int m_globalPulseLen;
  bool m_sendGlobalPulse;
  bool m_sendHardReset;
  bool m_sendSoftReset;
  bool m_sendBCR;
  bool m_sendECR;

  //TB: testbeam sram readout at
  int m_tb_sram_readout_at;

  // Loop configuration
  bool m_loopActive[MAX_LOOPS];
  ScanParam m_loopParam[MAX_LOOPS];
  bool m_dspProcessing[MAX_LOOPS];             //JW: kann für Processing auf Mikrocontroller benutzt werden...
  bool m_dspMaskStaging;                       //JW: ROD specific stuff
  bool m_innerLoopSwap;
  std::vector<float> m_loopVarValues[MAX_LOOPS];
  std::string m_loopFEI4GR[MAX_LOOPS];
  int m_loopVarNSteps[MAX_LOOPS];
  float m_loopVarMin[MAX_LOOPS];
  float m_loopVarMax[MAX_LOOPS];
  bool m_loopVarUniform[MAX_LOOPS];
  bool m_loopVarValuesFree[MAX_LOOPS];
  EndLoopAction m_loopAction[MAX_LOOPS];
  bool m_dspLoopAction[MAX_LOOPS];            //JW: ROD specific stuff
  bool m_loopOverDcs;
  bool m_avoidSpecialsCols;
  bool m_singleDCloop;
  bool m_configAtStart;
  bool m_alterFeCfg;

  // Scan execution
  int m_loopIndex[MAX_LOOPS];
  int m_maskStageIndex;
  bool m_newMaskStage;
  bool m_newScanStep;
  bool m_loopTerminating[MAX_LOOPS];
  bool m_loopEnded[MAX_LOOPS];

  // Scan specific parameters
  int m_thresholdTargetValue;
  int m_totTargetValue;
  int m_totTargetCharge;
  bool m_useMaskForGlobal;
  float m_minThrNoiseCutNormal;
  float m_minThrNoiseCutLong;
  float m_minThrNoiseCutGanged;
  float m_minThrNoiseCutInterGanged;
  float m_minThrChi2Cut;
  float m_noiseOccCut;

  // DB related
  DBInquire* m_dbInquire;
  PixConfDBInterface *m_db;
  Config *m_conf;

  // Histograms
  int m_nHisto;
  int m_nHistoSize;
  std::map<HistogramType, PixScanHisto> m_histo;
  std::map<std::string, int> m_histogramTypes;
  std::map<std::string, int> m_dspHistogramTypes;
  std::map<std::string, int> m_scanTypes;
  std::vector<Histo *> m_tmpSCmean;
  std::vector<Histo *> m_tmpSCsigma;
  std::vector<Histo *> m_tmpSCchi2;
  unsigned int m_modScurveFit;
  // S-curve fit quality
  float m_chicut;
  int m_nbadchicut;


  // Private methods
  void writeHistoLevel(DBInquire *dbi, PixScanHisto &sc, std::string lName, int lvl);
  void readHistoLevel(DBInquire *dbi, PixScanHisto &sc, std::string lName, int lvl, int idx);
  void prepareIndexes(HistogramType type, unsigned int mod, int ix2, int ix1, int ix0);

public:
  //! Constructors
  PixScan();
  PixScan(ScanType presetName, int FE_flavour);
  PixScan(DBInquire *dbi);
  PixScan(const PixScan &scn);

  //! Destructor
  ~PixScan();

  //! Init configuration
  void initConfig();

  //! Load predefined configurations
  void presetI3(ScanType presetName);
  void presetI4A(ScanType presetName);
  void presetI4B(ScanType presetName);
  void preset(ScanType presetName, int FE_flavour);

  //! Scan attributes
  ModConfigType getModConfig() {
    return m_modConfig;
  };
  void setModConfig(ModConfigType cfgTyp) {
    m_modConfig = cfgTyp;
  };
  bool getModScanConcurrent() {
    return m_modScanConcurrent;
  };
  void setModScanConcurrent(bool concurr) {
    m_modScanConcurrent = concurr;
  };
  MaskStageMode getMaskStageMode() {
    return m_maskStageMode;
  };
  void setMaskStageMode(MaskStageMode stageMode) {
    m_maskStageMode = stageMode;
  };
  MaskStageSteps getMaskStageTotalSteps() {
    return m_maskStageTotalSteps;
  };
  void setMaskStageTotalSteps(MaskStageSteps nSteps) {
    m_maskStageTotalSteps = nSteps;
  };
  int getMaskStageSteps() {
    return m_maskStageSteps;
  };
  void setMaskStageSteps(int nSteps) {
    m_maskStageSteps = nSteps;
  };
  int getMaskStageStepWidth() {
    return m_maskStageStepWidth;
  };
  int getMaskStageIndex(){
    return m_maskStageIndex;
  }
  int getRepetitions() {
    return m_repetitions;
  };
  void setRepetitions(int nRep) {
    m_repetitions = nRep;
  };
  bool getSelfTrigger() {
    return m_selfTrigger;
  };
  void setSelfTrigger(bool self) {
    m_selfTrigger = self;
  };
  bool getStrobeLVL1DelayOverride() {
    return m_strobeLVL1DelayOverride;
  };
  void setStrobeLVL1DelayOveride(bool lvl1StrDelayOverride) {
    m_strobeLVL1DelayOverride = lvl1StrDelayOverride;
  };
  int getStrobeLVL1Delay() {
    return m_strobeLVL1Delay;
  };
  void setStrobeLVL1Delay(int lvl1StrDelay) {
    m_strobeLVL1Delay = lvl1StrDelay;
  };
  int getLVL1Latency() {
    return m_LVL1Latency;
  };
  void setLVL1Latency(int lvl1Latency) {
    m_LVL1Latency = lvl1Latency;
  };
  int getStrobeMCCDelay() {
    return m_strobeMCCDelay;
  };
  void setStrobeMCCDelay(int delay) {
    m_strobeMCCDelay = delay;
  };
  int getStrobeMCCDelayRange() {
    return m_strobeMCCDelayRange;
  };
  void setStrobeMCCDelayRange(int delayRange) {
    m_strobeMCCDelayRange = delayRange;
  };
  int getStrobeDuration() {
    return m_strobeDuration;
  };
  void setStrobeDuration(int strobeDuration) {
    m_strobeDuration = strobeDuration;
  };
  int getStrobeFineDelay() {
    return m_strobeFineDelay;
  };
  void setStrobeFineDelay(int strobeFineDelay) {
    m_strobeFineDelay = strobeFineDelay;
  };
  int getStrobeFrequency() {
    return m_strobeFrequency;
  };
  void setStrobeFrequency(int strobeFrequency) {
    m_strobeFrequency = strobeFrequency;
  };
  unsigned int getModuleMask(int group) {
    if (group >=0 && group < MAX_GROUPS) {
      return m_moduleMask[group];
    }
    return 0;
  };
  void setModuleMask(int group, unsigned int mask) {
    if (group >=0 && group < MAX_GROUPS) {
      m_moduleMask[group] = mask;
    }
  };
  bool getConfigEnabled(int group) {
    if (group >=0 && group < MAX_GROUPS) {
      return m_configEnabled[group];
    }
    return false;
  };
  void setConfigEnabled(int group, bool ena) {
    if (group >=0 && group < MAX_GROUPS) {
      m_configEnabled[group] = ena;
    }
  };
  bool getTriggerEnabled(int group) {
    if (group >=0 && group < MAX_GROUPS) {
      return m_triggerEnabled[group];
    }
    return false;
  };
  void setTriggerEnabled(int group, bool ena) {
    if (group >=0 && group < MAX_GROUPS) {
      m_triggerEnabled[group] = ena;
    }
  };
  bool getStrobeEnabled(int group) {
    if (group >=0 && group < MAX_GROUPS) {
      return m_strobeEnabled[group];
    }
    return false;
  };
  void setStrobeEnabled(int group, bool ena) {
    if (group >=0 && group < MAX_GROUPS) {
      m_strobeEnabled[group] = ena;
    }
  };
  bool getReadoutEnabled(int group) {
    if (group >=0 && group < MAX_GROUPS) {
      return m_readoutEnabled[group];
    }
    return false;
  };
  void setReadoutEnabled(int group, bool ena) {
    if (group >=0 && group < MAX_GROUPS) {
      m_readoutEnabled[group] = ena;
    }
  };
  int getSuperGroupTrigDelay() {
    return m_superGroupTrigDelay;
  };
  void setSuperGroupTrigDelay(int delay) {
    m_superGroupTrigDelay = delay;
  };
  int getConsecutiveLvl1TrigA(int superGroup) {
    if (superGroup >=0 && superGroup < MAX_SGROUPS) {
      return m_consecutiveLvl1TrigA[superGroup];
    }
    return 0;
  };
  void setConsecutiveLvl1TrigA(int superGroup, int nLvl1) {
    if (superGroup >=0 && superGroup < MAX_SGROUPS) {
      m_consecutiveLvl1TrigA[superGroup] = nLvl1;
    }
  };
  int getConsecutiveLvl1TrigB(int superGroup) {
    if (superGroup >=0 && superGroup < MAX_SGROUPS) {
      return m_consecutiveLvl1TrigB[superGroup];
    }
    return 0;
  };
  void setConsecutiveLvl1TrigB(int superGroup, int nLvl1) {
    if (superGroup >=0 && superGroup < MAX_SGROUPS) {
      m_consecutiveLvl1TrigB[superGroup] = nLvl1;
    }
  };
  bool getLvl1HistoBinned() {
    return m_lvl1HistoBinned;
  };
  void setLvl1HistoBinned(bool bin) {
    m_lvl1HistoBinned = bin;
  };
  int getTrigABDelay(int superGroup) {
    if (superGroup >=0 && superGroup < MAX_SGROUPS) {
      return m_trigABDelay[superGroup];
    }
    return 0;
  };
  void setTrigABDelay(int superGroup, int delay) {
    if (superGroup >=0 && superGroup < MAX_SGROUPS) {
      m_trigABDelay[superGroup] = delay;
    }
  };
  bool getHistogramFilled(HistogramType type) {
    return m_histogramFilled[type];
  };
  void setHistogramFilled(HistogramType type, bool fill) {
    m_histogramFilled[type] = fill;
  };
  bool getHistogramKept(HistogramType type) {
    return m_histogramKept[type];
  };
  void setHistogramKept(HistogramType type, bool keep) {
    m_histogramKept[type] = keep;
  };
  int getNptsRateAvg() {
    return m_getNptsRateAvg;
  }
  void setNptsRateAvg(int npts) {
    m_getNptsRateAvg = npts;
  }
  void getClusterPars(int &maxColClusterDist, int &maxRowClusterDist, int &maxClusterBcidDepth, int &minClusterHits, int &maxClusterHits, int &maxClusterHitTot, int &maxEventIncomplete, int &maxEventErrors){
    maxColClusterDist   = m_maxColClusterDist;
    maxRowClusterDist   = m_maxRowClusterDist;
    maxClusterBcidDepth = m_maxClusterBcidDepth;
    minClusterHits = m_minClusterHits;
    maxClusterHits = m_maxClusterHits;
    maxClusterHitTot = m_maxClusterHitTot;
    maxEventIncomplete = m_maxEventIncomplete;
    maxEventErrors = m_maxEventErrors;
  }
//  void setClusterPars(int maxColClusterDist, int maxRowClusterDist, int maxClusterBcidDepth, int &maxClusterHits, int &maxEventErrors, int &maxClusterHitTot){
//    m_maxColClusterDist   = maxColClusterDist;
//    m_maxRowClusterDist   = maxRowClusterDist;
//    m_maxClusterBcidDepth = maxClusterBcidDepth;
//    m_maxClusterHitTot = maxClusterHitTot;
//    m_maxClusterHits = maxClusterHits;
//    m_maxEventErrors = maxEventErrors;
//  }
  bool getDigitalInjection() {
    return m_digitalInjection;
  };
  void setDigitalInjection(bool digiInj) {
    m_digitalInjection = digiInj;
  };
  int getChargeInjCap() {
    return m_chargeInjCap;
  };
  void setChargeInjCap(int cap) {
    m_chargeInjCap = cap;
  };
  int getFeVCal() {
    return m_feVCal;
  };
  void setFeVCal(int vcal) {
    m_feVCal = vcal;
  };
  // new new DCS scan functionality
  std::string getDcsScanPar1Name(){return m_scanPar1DcsPar;};
  std::string getDcsScanPar2Name(){return m_scanPar2DcsPar;};
  std::string getDcsScanPar3Name(){return m_scanPar3DcsPar;};
  std::string getDcsScanPar1ChannelName(){return m_scanPar1DcsChan;};
  std::string getDcsScanPar2ChannelName(){return m_scanPar2DcsChan;};
  std::string getDcsScanPar3ChannelName(){return m_scanPar3DcsChan;};

  // new DCS scan functionality
  std::string getDcsChan(){return m_readDcsChan;};
  DcsReadMode getDcsMode(){return m_readDcsMode;};
  std::string getScanDcsChan(){return m_scanDcsChan;};
  bool getAddModNameToDcs(){return m_addModNameToDcs;};

  bool getRestoreModuleConfig() {
    return m_restoreModuleConfig;
  }
  void setRestoreModuleConfig(bool restore) {
    m_restoreModuleConfig = restore;
  }
  RunType getRunType() {
    return m_runType;
  }
  void setRunType(RunType run) {
    m_runType = run;
  }
  std::vector<unsigned int>& getPatternSeeds() {
    return m_patternSeeds;
  };
  void setPatternSeeds(std::vector<unsigned int> values) {
    m_patternSeeds = values;
  };

  //! Loop attributes
  bool getLoopActive(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopActive[index];
    }
    return false;
  };
  void setLoopActive(int index, bool active) {
    if (index >=0 && index < MAX_LOOPS) {
      m_loopActive[index] = active;
    }
  };
  ScanParam getLoopParam(int index) {
     if (index >=0 && index < MAX_LOOPS) {
       return m_loopParam[index];
     }
     return NO_PAR;
  };
  void setLoopParam(int index, ScanParam par) {
     if (index >=0 && index < MAX_LOOPS) {
       m_loopParam[index] = par;
     }
  };
  bool getDspProcessing(int index) {
     if (index >=0 && index < MAX_LOOPS) {
       return m_dspProcessing[index];
     }
     return false;
  };
  void setDspProcessing(int index, bool dsp) {
     if (index >=0 && index < MAX_LOOPS) {
       m_dspProcessing[index] = dsp;
     }
  };
  bool getDspMaskStaging() {
    return m_dspMaskStaging;
  };
  void setDspMaskStaging(bool dsp) {
    m_dspMaskStaging = dsp;
  };
  bool getInnerLoopSwap() {
    return m_innerLoopSwap;
  };
  void setInnerLoopSwap(bool swp) {
    m_innerLoopSwap = swp;
  };
  void setLoopFEI4GR(int index, std::string nameGr){
    if (index >=0 && index < MAX_LOOPS) {
      m_loopFEI4GR[index] = nameGr;
    }
  }
  std::string& getLoopFEI4GR(int index){
    static std::string tmp="unknown";
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopFEI4GR[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return tmp;
  }
  std::vector<float>& getLoopVarValues(int index) {
    static std::vector<float> tmp;
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopVarValues[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return tmp;
  };
  void setLoopVarValues(int index, double startVal, double endVal, int nSteps) {
    if (index >=0 && index < MAX_LOOPS) {
      m_loopVarNSteps[index] = nSteps;
      m_loopVarMin[index] = (float)startVal;
      m_loopVarMax[index] = (float)endVal;
      m_loopVarValues[index].clear();
      double v = startVal;
      double incr = (endVal - startVal)/(nSteps -1);
      for (int iv=0; iv<nSteps; iv++) {
        m_loopVarValues[index].push_back((float)v);
        v += incr;
      }
      m_loopVarUniform[index] = true;
      m_loopVarValuesFree[index] = false;
    } else {
      throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    }
  };
  void setLoopVarValues(int index, std::vector<float> values) {
    if (index >=0 && index < MAX_LOOPS) {
      m_loopVarValues[index] = values;
      m_loopVarNSteps[index] = values.size();
      m_loopVarMin[index] = m_loopVarMax[index] = values[0];
      for (unsigned int i=0; i<values.size(); i++) {
        if (values[i] < m_loopVarMin[index]) m_loopVarMin[index] = values[i];
        if (values[i] > m_loopVarMax[index]) m_loopVarMax[index] = values[i];
      }
      m_loopVarUniform[index] = false;
      m_loopVarValuesFree[index] = false;
    } else
      throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
  };
  int getLoopVarNSteps(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopVarNSteps[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return 0;
  }
  float getLoopVarMin(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopVarMin[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return 0;
  }
  float getLoopVarMax(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopVarMax[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return 0;
  }
  bool getLoopVarValuesFree(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopVarValuesFree[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return false;
  };
  void setLoopVarValuesFree(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      m_loopVarValuesFree[index] = true;
    } else
      throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
  };
  bool getLoopVarUniform(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopVarUniform[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return false;
  };
  EndLoopAction getLoopAction(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_loopAction[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return NO_ACTION;
  };
  void setLoopAction(int index, EndLoopAction action) {
    if (index >=0 && index < MAX_LOOPS) {
      m_loopAction[index] = action;
    } else
      throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
  };
  bool getDspLoopAction(int index) {
    if (index >=0 && index < MAX_LOOPS) {
      return m_dspLoopAction[index];
    }
    throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
    return false;
  };
  void setDspLoopAction(int index, bool dsp) {
    if (index >=0 && index < MAX_LOOPS) {
      m_dspLoopAction[index] = dsp;
    } else
      throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
  };
  void setLoopOverDcs(bool val){
    m_loopOverDcs = val;
  };
  bool getLoopOverDcs(){
    return m_loopOverDcs;
  };
  void setAvoidSpecialsCols(bool val){
    m_avoidSpecialsCols = val;
  }
  bool getAvoidSpecialsCols(){
    return m_avoidSpecialsCols;
  }
  void setSingleDCloop(bool val){
    m_singleDCloop = val;
  }
  bool getSingleDCloop(){
    return m_singleDCloop;
  }

  bool getConfigAtStart(){
    return m_configAtStart;
  }
  bool getAlterFeCfg(){
    return m_alterFeCfg;
  }

  //! Scan control
  void resetScan();
  bool loop(int index);
  int scanIndex(int index);
  void next(int index);
  void terminate(int index);
  bool newMaskStep();
  bool newScanStep();

  // Scan specific parameters
  void setThresholdTargetValue(int val) {
    m_thresholdTargetValue = val;
  };
  int getThresholdTargetValue() {
    return m_thresholdTargetValue;
  };
  void setTotTargetValue(int val) {
    m_totTargetValue = val;
  };
  int getTotTargetValue() {
    return m_totTargetValue;
  };
  void setTotTargetCharge(int val) {
    m_totTargetCharge = val;
  };
  int getTotTargetCharge() {
    return m_totTargetCharge;
  };
  bool getUseMaskForGlobal(){
    return m_useMaskForGlobal;
  }
  float getMinThrNoiseCutNormal(){
    return m_minThrNoiseCutNormal;
  }
  float getMinThrNoiseCutLong(){
    return m_minThrNoiseCutLong;
  }
  float getMinThrNoiseCutGanged(){
    return m_minThrNoiseCutGanged;
  }
  float getMinThrNoiseCutInterGanged(){
    return m_minThrNoiseCutInterGanged;
  }
  float getMinThrNoiseCut(int type);
  float getMinThrChi2Cut(){
    return m_minThrChi2Cut;
  }
  void setNoiseOccCut(float val) {
    m_noiseOccCut = val;
  };
  float getNoiseOccCut(){
    return m_noiseOccCut;
  }
  bool getSourceScanFlag() {
    return m_isSourceScan;
  };
  bool getTestBeamFlag() {
    return m_isTestBeam;
  };
  int getSramReadoutAt(){
    return m_tb_sram_readout_at;
  }
  int getLowerLVL1DelayIfSlaveBy() {
	  return m_lowerLVL1DelayIfSlaveBy;
  };
  int getTLUTriggerDataDelay() {
	  return m_TLUTriggerDataDelay;
  };
  bool getSendGlobalPulse(){
    return m_sendGlobalPulse;;
  }
  int getGlobalPulseLen(){
    return m_globalPulseLen;
  }
  bool getSendHardReset(){
    return m_sendHardReset;;
  }
  bool getSendSoftReset(){
    return m_sendSoftReset;;
  }
  bool getSendBCR(){
    return m_sendBCR;;
  }
  bool getSendECR(){
    return m_sendECR;;
  }
  bool getSkipSourceScanHistos() {
	  return m_skipSourceScanHistos;
  };
  std::string getSourceRawFile(){
    return m_sourceRawFile;
  }
  void setSourceRawFile(std::string newFileName){
    m_sourceRawFile=newFileName;
  }
  int getSrcCountType(){
    return m_srcCountType;
  }
  int getSrcTriggerType(){
    return m_srcTriggerType;
  }
  float getChiCut(){
    return m_chicut;
  }
  int getNbadchiCut(){
    return m_nbadchicut;
  }

  //! DataBase interaction
  void writeConfig(DBInquire *dbi);
  void writeHisto(DBInquire *dbi);
  void write(DBInquire* dbi);
  void readConfig(DBInquire *dbi);
  void readHisto(DBInquire *dbi);
  void read(DBInquire *dbi);
  Config &config() { return *m_conf; };

  //! Histogram handling
  void addHisto(Histo &his, HistogramType type, unsigned int mod, int ix2, int ix1, int ix0);
  void addHisto(std::vector< Histo * >&his, HistogramType type, unsigned int mod, int ix2, int ix1, int ix0);
  void addHisto(std::vector< Histo * >&his, HistogramType type, unsigned int mod, int ix2, int ix1);
  void addHisto(std::vector< Histo * >&his, unsigned int nh, HistogramType type, unsigned int mod, int ix2, int ix1);
  void addHisto(std::vector< Histo * >&his, unsigned int d2, unsigned int d1, HistogramType type, unsigned int mod, int ix2);
  void downloadHisto(PixController *ctrl, unsigned int mod, HistogramType type);
  bool downloadErrorHisto(PixController *ctrl, unsigned int dsp);
  void clearHisto(unsigned int mod, HistogramType type);
  Histo& getHisto(HistogramType type, int module, int idx2, int idx1, int idx0);
  Histo& getHisto(HistogramType type, int module, int idx2, int idx1, int idx0, int ih);
  PixScanHisto& getHisto(HistogramType type);
  std::map<std::string, int> &getHistoTypes() {
    return m_histogramTypes;
  }
  std::map<std::string, int> &getDspHistoTypes() {
    return m_dspHistogramTypes;
  };
  std::map<std::string, int> &getScanTypes() {
    return m_scanTypes;
  };

  //! Helper functions
  void fitSCurve(PixScanHisto &sc, Histo &thr, Histo &noise, Histo& chi2, int ih, int rep);
};

}

#endif
