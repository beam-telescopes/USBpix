/////////////////////////////////////////////////////////////////////
// PixScan.cxx
/////////////////////////////////////////////////////////////////////
//
// 17/06/05  Version 1.0 (PM)
//


#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "PixFe/PixFeI4A.h"
#include "PixFe/PixFeI4B.h"
#include "PixController/PixScan.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Config/Config.h"
#include "Histo/Histo.h"
#include <FitClass.h>

#include <sstream>
#include <math.h>

#define PS_DEBUG false

using namespace PixLib;

PixScanHisto::PixScanHisto() {
	m_histoMode = false;
	m_histo = NULL;
	m_map = NULL;
	m_cached = false;
}

PixScanHisto::PixScanHisto(Histo &h) {
	m_histoMode = true;
	m_map = NULL;
	m_histo = new std::map<unsigned int, Histo*>;
	m_cached = false;
	(*m_histo)[0] = &h;
}

PixScanHisto::PixScanHisto(unsigned int ih, Histo &h) {
	m_histoMode = true;
	m_map = NULL;
	m_histo = new std::map<unsigned int, Histo*>;
	m_cached = false;
	(*m_histo)[ih] = &h;
}

PixScanHisto::PixScanHisto(unsigned int ix, PixScanHisto &sh) {
	m_histoMode = false;
	m_histo = NULL;
	m_map = new std::map<unsigned int,PixScanHisto*>;
	m_cached = false;
	(*m_map)[ix] = &sh;
}

PixScanHisto::~PixScanHisto() {
	clear();
}

void PixScanHisto::add(unsigned int ix, PixScanHisto &sh) {
	if (m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in map mode");
	} else {
		if (m_map == NULL) m_map = new std::map<unsigned int, PixScanHisto*>;
		if (m_map->find(ix) != m_map->end()) {
			delete (*m_map)[ix];
		}
		(*m_map)[ix] = &sh;
	}
}

void PixScanHisto::add(unsigned int ih, Histo &h) {
	if (!m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in histo mode");
	} else {
		if (m_histo == NULL) m_histo = new std::map<unsigned int, Histo*>;
		if (m_histo->find(ih) != m_histo->end()) {
			delete (*m_histo)[ih];
		}
		(*m_histo)[ih] = &h;
	}
}

void PixScanHisto::add(unsigned int ih, std::vector< Histo * >&h) {
	if (m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in map mode");
	} else {
		PixScanHisto *sh = new PixScanHisto(*(h[0]));
		if (m_map == NULL) m_map = new std::map<unsigned int, PixScanHisto*>;
		if (m_map->find(ih) != m_map->end()) {
			delete (*m_map)[ih];
		}
		(*m_map)[ih] = sh;
		for (unsigned int iih = 1; iih < h.size(); iih++) {
			sh->add(iih, *(h[iih]));
		}
	}
}

bool PixScanHisto::exists(unsigned int ix) {
	if (m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in map mode");
	} else {
		if (m_map != NULL) {
			if (m_map->find(ix) != m_map->end()) return true;
		}
	}
	return false;
}

bool PixScanHisto::histoExists(unsigned int ih) {
	if (!m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in histo mode");
	} else {
		if (m_histo != NULL) {
			if (m_histo->find(ih) != m_histo->end()) return true;
		}
	}
	return false;
}

unsigned int PixScanHisto::size() {
	if (m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in map mode");
	} else {
		if (m_map != NULL) {
			return m_map->size();
		}
	}
	return 0;
}

unsigned int PixScanHisto::histoSize() {
	if (!m_histoMode) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in histo mode");
	} else {
		if (m_histo != NULL) {
			return m_histo->size();
		}
	}
	return 0;
}

bool PixScanHisto::histoMode() {
	return m_histoMode;
}

PixScanHisto& PixScanHisto::operator[](unsigned int ix) {
	static PixScanHisto tmp;
	if (m_histoMode) {
		return *this;
	} else {
		if (m_map != NULL) {
			if (m_map->find(ix) != m_map->end()) return *((*m_map)[ix]);
		}
		throw PixScanExc(PixControllerExc::ERROR, "Index not found in map");
	}
	return tmp;
}

Histo& PixScanHisto::histo() {
	static Histo tmp;
	if (m_histoMode) {
		if (m_histo != NULL) {
			if (m_histo->find(0) != m_histo->end()) return *((*m_histo)[0]);
		} else {
			//throw PixScanExc(PixControllerExc::ERROR, "Histogram not present");
			cerr << "Histogram not present" << endl;
		}
	}
	//throw PixScanExc(PixControllerExc::ERROR, "Not in histogram mode");
	cerr << "Not in histogram mode" << endl;
	return tmp;
}

Histo& PixScanHisto::histo(unsigned int ih) {
	static Histo tmp;
	if (m_histoMode) {
		if (m_histo != NULL) {
			if (m_histo->find(ih) != m_histo->end()) {
				return *((*m_histo)[ih]);
			}
		} else {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram not present");
		}
	}
	throw PixScanExc(PixControllerExc::ERROR, "Not in histogram mode");
	return tmp;
}

void PixScanHisto::clear() {
	if (m_histoMode) {
		if (m_histo != NULL) {
			std::map<unsigned int, Histo*>::iterator it;
			for (it = m_histo->begin(); it != m_histo->end(); ++it) {
				delete (*it).second;
			}
			m_histo->clear();
			delete m_histo;
			m_histo = NULL;
		}
	} else {
		if (m_map != NULL) {
			std::map<unsigned int, PixScanHisto*>::iterator it;
			for (it = m_map->begin(); it != m_map->end(); ++it) {
				delete (*it).second;
			}
			m_map->clear();
			delete m_map;
			m_map = NULL;
		}
	}
}

PixScan::PixScan() {
	initConfig();
	resetScan();
}

PixScan::PixScan(ScanType presetName, int FE_flavour) {
	m_db = NULL;
	m_dbInquire = NULL;
	preset(presetName, FE_flavour);
}

PixScan::PixScan(DBInquire *dbi) : m_dbInquire(dbi) {
	m_db = m_dbInquire->getDB();
	initConfig();
	readConfig(dbi);
	resetScan();
}

PixScan::PixScan(const PixScan &scn) {
	m_db = NULL;
	m_dbInquire = NULL;
	initConfig();
	*m_conf = *(scn.m_conf);
	resetScan();
}

PixScan::~PixScan() {
	m_histo.clear();
  delete m_conf;
}

void PixScan::preset(ScanType presetName, int FE_flavour) {
  initConfig();
  PixModule::FEFlavour flv = (PixModule::FEFlavour) FE_flavour;
  switch(flv){
  case PixModule::PM_FE_I1:
  case PixModule::PM_FE_I2:
    presetI3(presetName);
    break;
  case PixModule::PM_FE_I4A:
    presetI4A(presetName);
    break;
  default:
  case PixModule::PM_FE_I4B:
    presetI4B(presetName);
    break;
  }
  resetScan();
}
void PixScan::presetI3(ScanType presetName) {
  // config default is I4, so fix some general things first
  m_maskStageTotalSteps = STEPS_32;
  m_maskStageSteps = 3;
  m_totTargetValue = 30;
  m_totTargetCharge = 9000;
  m_minThrNoiseCutNormal = 220.;
  m_minThrNoiseCutLong = 250.;
  m_minThrNoiseCutGanged = 500.;
  m_minThrNoiseCutInterGanged = 250.;
  m_minThrChi2Cut = 200.;
  m_nbadchicut = 10;
  m_LVL1Latency = 255;
  m_strobeLVL1Delay = 250;
  m_strobeMCCDelay = 0;
  m_strobeMCCDelayRange = 5;
  m_chargeInjCap = 0;
  m_chicut = 200.;

  if (presetName == DIGITAL_TEST) {
    m_maskStageSteps = 32;
    m_strobeDuration = 50;
    m_digitalInjection = true;    
    m_innerLoopSwap = true;  
  } else if (presetName == ANALOG_TEST) {
    m_maskStageSteps = 32;
    m_strobeDuration = 500;
    m_feVCal = 400;
    m_digitalInjection = false;    
    m_innerLoopSwap = true;  
  } else if (presetName == THRESHOLD_SCAN) {
    m_repetitions = 100;
    m_strobeDuration = 500;
    m_maskStageSteps = 32;
    m_innerLoopSwap = true;  
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = true;
    setLoopVarValues(0, 0, 200, 201);
    m_loopAction[0] = SCURVE_FIT;
    m_dspLoopAction[0] = false; // changed JGK
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SCURVE_MEAN] = true;
    m_histogramKept[SCURVE_MEAN] = true;
    m_histogramFilled[SCURVE_SIGMA] = true;
    m_histogramKept[SCURVE_SIGMA] = true;
    m_histogramFilled[SCURVE_CHI2] = true;
    m_histogramKept[SCURVE_CHI2] = true;
  } else if (presetName == TDAC_TUNE || presetName == GDAC_TUNE) {
    m_repetitions = 25;
    m_maskStageSteps = 32;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_innerLoopSwap = true;  
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = true;
    setLoopVarValues(0, 0, 200, 201);
    m_loopAction[0] = SCURVE_FIT;
    m_dspLoopAction[0] = false; // changed JGK
    m_loopActive[1] = true;
    m_dspProcessing[1] = false;
    m_dspLoopAction[1] = false;
    if (presetName == TDAC_TUNE) {
      m_loopParam[1] = TDACS;
      std::vector<float> st;
      st.push_back(70);
      st.push_back(16);
      st.push_back(8);
      st.push_back(4);
      st.push_back(2);
      st.push_back(1);
      st.push_back(1);
      setLoopVarValues(1, st);
      m_loopAction[1] = TDAC_TUNING;
      m_histogramFilled[TDAC_T] = true;
      m_histogramKept[TDAC_T] = true;
      m_histogramFilled[TDAC_THR] = true;
      m_histogramKept[TDAC_THR] = true;
    } else {
      m_loopParam[1] = GDAC;
      setLoopVarValues(1, 10, 30, 3);
      m_loopAction[1] = GDAC_TUNING;
      m_histogramFilled[GDAC_T] = true;
      m_histogramKept[GDAC_T] = true;
      m_histogramFilled[GDAC_THR] = true;
      m_histogramKept[GDAC_THR] = true;
    }
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SCURVE_MEAN] = true;
    m_histogramKept[SCURVE_MEAN] = true;
    m_histogramFilled[SCURVE_SIGMA] = true;
    m_histogramKept[SCURVE_SIGMA] = true;
    m_histogramFilled[SCURVE_CHI2] = true;
    m_histogramKept[SCURVE_CHI2] = true;
    m_thresholdTargetValue = 4000;
    m_restoreModuleConfig = false;
  } else if (presetName == GDAC_FAST_TUNE) {
    m_repetitions = 200;
    m_maskStageSteps = 32;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    setLoopVarValues(0, 0, 2, 1);
    m_loopActive[0] = true;
    m_dspProcessing[0] = false;
    m_dspLoopAction[0] = false;
    m_loopParam[0] = NO_PAR;
    m_loopAction[0] = OCC_SUM;
    m_loopActive[1] = true;
    m_dspProcessing[1] = false;
    m_dspLoopAction[1] = false;
    m_loopParam[1] = GDAC;
    std::vector<float> st;
    st.push_back(16);
    st.push_back(4);
    st.push_back(2);
    st.push_back(1);
    st.push_back(1);
    setLoopVarValues(1, st);
    m_loopAction[1] = GDAC_FAST_TUNING;
    m_loopVarValuesFree[1] = false;
    m_histogramFilled[GDAC_T] = true;
    m_histogramKept[GDAC_T] = true;
    m_histogramFilled[GDAC_OCC] = true;
    m_histogramKept[GDAC_OCC] = true;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SUM_OCC] = true;
    m_histogramKept[SUM_OCC] = true;
    m_thresholdTargetValue = 4000;
    m_restoreModuleConfig = false;
    m_feVCal = 0x1fff;
  } else if (presetName == TDAC_FAST_TUNE) {
    m_repetitions = 200;
    m_maskStageSteps = 32;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_innerLoopSwap = true;  
    setLoopVarValues(0, 0, 2, 2);
    m_loopActive[0] = true;
    m_dspProcessing[0] = true;
    m_dspLoopAction[0] = false;
    m_loopParam[0] = NO_PAR;
    m_loopAction[0] = OCC_SUM;
    m_loopActive[1] = true;
    m_dspProcessing[1] = false;
    m_dspLoopAction[1] = false;
    m_loopParam[1] = TDACS;
    std::vector<float> st;
    st.push_back(64);
    st.push_back(32);
    st.push_back(16);
    st.push_back(8);
    st.push_back(4);
    st.push_back(2);
    st.push_back(1);
    st.push_back(1);
    setLoopVarValues(1, st);
    m_loopAction[1] = TDAC_FAST_TUNING;
    m_loopVarValuesFree[1] = false;
    m_histogramFilled[TDAC_T] = true;
    m_histogramKept[TDAC_T] = true;
    m_histogramFilled[TDAC_OCC] = true;
    m_histogramKept[TDAC_OCC] = true;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SUM_OCC] = true;
    m_histogramKept[SUM_OCC] = true;
    m_thresholdTargetValue = 4000;
    m_restoreModuleConfig = false;
    m_feVCal = 0x1fff;
  } else if (presetName == FDAC_TUNE || presetName == IF_TUNE) {
    m_repetitions = 25;
    m_maskStageSteps = 32;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_dspProcessing[0] = false;
    m_dspLoopAction[0] = false;
    if (presetName == FDAC_TUNE) {
      m_loopParam[0] = FDACS;
      setLoopVarValues(0, 0, 7, 8);
      m_loopAction[0] = FDAC_TUNING;
      m_histogramFilled[FDAC_T] = true;
      m_histogramKept[FDAC_T] = true;
      m_histogramFilled[FDAC_TOT] = true;
      m_histogramKept[FDAC_TOT] = true;
    } else {
      m_loopParam[0] = IF;
      setLoopVarValues(0, 25, 50, 26);
      m_loopAction[0] = IF_TUNING;
      m_histogramFilled[IF_T] = true;
      m_histogramKept[IF_T] = true;
      m_histogramFilled[IF_TOT] = true;
      m_histogramKept[IF_TOT] = true;
    }
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[TOT_MEAN] = true;
    m_histogramKept[TOT_MEAN] = true;
    m_histogramFilled[TOT_SIGMA] = true;
    m_histogramKept[TOT_SIGMA] = false;
    m_totTargetValue = 30;
    m_totTargetCharge = 20000;
    m_restoreModuleConfig = false;
    m_feVCal = 0x1fff;
  } else if (presetName == TOT_CALIB) {
    m_repetitions = 50;
    m_maskStageSteps = 32;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = false; // JGK changed
    setLoopVarValues(0, 70, 950, 23);
    m_dspLoopAction[0] = false;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[TOT_MEAN] = true;
    m_histogramKept[TOT_MEAN] = true;
    m_histogramFilled[TOT_SIGMA] = true;
    m_histogramKept[TOT_SIGMA] = true;
    m_histogramFilled[TOTCAL_PARA] = true;
    m_histogramKept[TOTCAL_PARA] = true;
    m_histogramFilled[TOTCAL_PARB] = true;
    m_histogramKept[TOTCAL_PARB] = true;
    m_histogramFilled[TOTCAL_PARC] = true;
    m_histogramKept[TOTCAL_PARC] = true;
    m_histogramFilled[TOTCAL_CHI2] = true;
    m_histogramKept[TOTCAL_CHI2] = true;
    m_loopAction[0] = TOTCAL_FIT;  // changed JGK
  } else if (presetName == T0_SCAN) {
    m_repetitions = 25;
    m_maskStageSteps = 32;
    m_feVCal = 0x1fff;
    m_totTargetCharge = 100000;
    m_chargeInjCap = 1;
    m_consecutiveLvl1TrigA[0] = 1;
    m_consecutiveLvl1TrigB[0] = 1;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_loopParam[0] = STROBE_DELAY;
    m_innerLoopSwap = true;  
    m_dspProcessing[0] = true;
    setLoopVarValues(0, 0, 63, 64);
    m_dspLoopAction[0] = false;
    m_loopActive[1] = true;
    m_dspProcessing[1] = false;
    m_dspLoopAction[1] = false;
    m_loopParam[1] = TRIGGER_DELAY;
    setLoopVarValues(1, 248, 251, 4);
    m_loopAction[1] = T0_SET;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = true;
    m_histogramFilled[TIMEWALK] = true;
    m_histogramKept[TIMEWALK] = true;
    m_restoreModuleConfig = false;
  } else if (presetName == TIMEWALK_MEASURE) {
    m_repetitions = 25;
    m_maskStageSteps = 32;
    m_chargeInjCap = 0;
    m_consecutiveLvl1TrigA[0] = 1;
    m_consecutiveLvl1TrigB[0] = 1;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_strobeLVL1DelayOverride = false;
    m_loopActive[0] = true;
    m_loopParam[0] = STROBE_DELAY;
    m_dspProcessing[0] = true;
    m_innerLoopSwap = true;  
    setLoopVarValues(0, 0, 63, 64);
    m_loopAction[0] = MCCDEL_FIT;
    m_dspLoopAction[0] = false;//true;
    m_loopActive[1] = true;
    m_dspProcessing[1] = false;
    m_dspLoopAction[1] = false;
    m_loopParam[1] = VCAL;
    std::vector<float> vcalVals;
    vcalVals.push_back(70);
    vcalVals.push_back(75);
    vcalVals.push_back(80);
    vcalVals.push_back(85);
    vcalVals.push_back(90);
    vcalVals.push_back(95);
    vcalVals.push_back(100);
    vcalVals.push_back(110);
    vcalVals.push_back(120);
    vcalVals.push_back(130);
    vcalVals.push_back(140);
    vcalVals.push_back(160);
    vcalVals.push_back(180);
    vcalVals.push_back(200);
    vcalVals.push_back(220);
    vcalVals.push_back(240);
    vcalVals.push_back(280);
    vcalVals.push_back(320);
    vcalVals.push_back(360);
    vcalVals.push_back(400);
    vcalVals.push_back(500);
    vcalVals.push_back(600);
    vcalVals.push_back(700);
    vcalVals.push_back(800);
    vcalVals.push_back(1000);
    setLoopVarValues(1, vcalVals);
    m_loopAction[1] = NO_ACTION;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[TIMEWALK] = true;
    m_histogramKept[TIMEWALK] = true;
    m_restoreModuleConfig = true;
  } else if (presetName == INTIME_THRESH_SCAN) {
    m_repetitions = 100;
    m_strobeDuration = 500;
    m_maskStageSteps = 32;
    m_consecutiveLvl1TrigA[0] = 1;
    m_consecutiveLvl1TrigB[0] = 1;
    m_strobeLVL1DelayOverride = false;
    m_strobeMCCDelayRange = 31;
    m_digitalInjection = false;
    m_innerLoopSwap = true;  
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = true;
    setLoopVarValues(0, 0, 200, 201);
    m_loopAction[0] = SCURVE_FIT;
    m_dspLoopAction[0] = false;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SCURVE_MEAN] = true;
    m_histogramKept[SCURVE_MEAN] = true;
    m_histogramFilled[SCURVE_SIGMA] = true;
    m_histogramKept[SCURVE_SIGMA] = true;
    m_histogramFilled[SCURVE_CHI2] = true;
    m_histogramKept[SCURVE_CHI2] = true;
  } else if (presetName == CROSSTALK_SCAN) {
    m_nbadchicut = 47000;
    m_repetitions = 100;
    m_strobeDuration = 500;
    m_maskStageSteps = 32;
    m_maskStageMode = XTALK;
    m_digitalInjection = false;
    m_chargeInjCap = 1;
    m_innerLoopSwap = true;  
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = true;
    setLoopVarValues(0, 0, 1000, 26);
    m_loopAction[0] = SCURVE_FIT;
    m_dspLoopAction[0] = false;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SCURVE_MEAN] = true;
    m_histogramKept[SCURVE_MEAN] = true;
    m_histogramFilled[SCURVE_SIGMA] = true;
    m_histogramKept[SCURVE_SIGMA] = true;
    m_histogramFilled[SCURVE_CHI2] = true;
    m_histogramKept[SCURVE_CHI2] = true;
  } else if (presetName == INCREMENTAL_TDAC_SCAN) {
    m_nbadchicut = 47000;
    m_repetitions = 25;
    m_maskStageSteps = 32;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_innerLoopSwap = true;  
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = true;
    setLoopVarValues(0, 0, 200, 201);
    m_loopAction[0] = SCURVE_FIT;
    m_dspLoopAction[0] = false;
    m_loopActive[1] = true;
    m_dspProcessing[1] = false;
    m_dspLoopAction[1] = false;
    m_loopParam[1] = TDACS_VARIATION;
    std::vector<float> st;
    st.push_back(-5);
    setLoopVarValues(1, st);
    setLoopVarValuesFree(1);
    m_loopAction[1] = MIN_THRESHOLD;
    m_histogramFilled[TDAC_T] = true;
    m_histogramKept[TDAC_T] = true;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[SCURVE_MEAN] = true;
    m_histogramKept[SCURVE_MEAN] = true;
    m_histogramFilled[SCURVE_SIGMA] = true;
    m_histogramKept[SCURVE_SIGMA] = true;
    m_histogramFilled[SCURVE_CHI2] = true;
    m_histogramKept[SCURVE_CHI2] = true;
  } else if (presetName == BOC_RX_DELAY_SCAN) {
    m_repetitions = 10;
    m_maskStageSteps = 0;
    m_maskStageMode = STATIC;
    m_loopActive[0] = true;
    m_loopParam[0] = NO_PAR;
    m_dspProcessing[0] = true;
    m_loopAction[0] = NO_ACTION;
    m_dspLoopAction[0] = false;
    m_runType = RAW_PATTERN;
    m_histogramFilled[OCCUPANCY] = false;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[RAW_DATA_0] = true;
    m_histogramKept[RAW_DATA_0] = false;
//     m_histogramFilled[RAW_DATA_1] = true;
//     m_histogramKept[RAW_DATA_1] = true;
//     m_histogramFilled[RAW_DATA_DIFF_1] = true;
//     m_histogramKept[RAW_DATA_DIFF_1] = true;
//     m_histogramFilled[RAW_DATA_REF] = true;
//     m_histogramKept[RAW_DATA_REF] = true;
//   } else if (presetName == BOC_V0_RX_DELAY_SCAN) {
//     m_repetitions = 10;
//     m_maskStageSteps = 0;
//     m_maskStageMode = STATIC;
//     m_loopActive[0] = true;
//     m_loopParam[0] = BOC_RX_DELAY;
//     m_dspProcessing[0] = false;
//     setLoopVarValues(0, 0, 24, 13);
//     m_loopAction[0] = NO_ACTION;
//     m_dspLoopAction[0] = false;
//     m_loopActive[1] = true;
//     m_loopParam[1] = BOC_VPH0;
//     m_dspProcessing[1] = false;
//     setLoopVarValues(1, 0, 24, 13);
//     m_loopAction[1] = NO_ACTION;
//     m_dspLoopAction[1] = false;
//     m_runType = RAW_EVENT;
//     m_histogramFilled[OCCUPANCY] = false;
//     m_histogramKept[OCCUPANCY] = false;
//     m_histogramFilled[RAW_DATA_0] = true;
//     m_histogramKept[RAW_DATA_0] = false;
//     m_histogramFilled[RAW_DATA_1] = true;
//     m_histogramKept[RAW_DATA_1] = true;
//     m_histogramFilled[RAW_DATA_DIFF_1] = true;
//     m_histogramKept[RAW_DATA_DIFF_1] = true;
//     m_histogramFilled[RAW_DATA_DIFF_2] = true;
//     m_histogramKept[RAW_DATA_DIFF_2] = true;
//     m_histogramFilled[RAW_DATA_REF] = true;
//     m_histogramKept[RAW_DATA_REF] = true;
//     m_mccBandwidth = SINGLE_80;
//   } else if (presetName == BOC_THR_RX_DELAY_SCAN) {
//     m_repetitions = 10;
//     m_maskStageSteps = 0;
//     m_maskStageMode = STATIC;
//     m_loopActive[0] = true;
//     m_loopParam[0] = BOC_RX_DELAY;
//     m_dspProcessing[0] = false;
//     setLoopVarValues(0, 0, 24, 25);
//     m_loopAction[0] = NO_ACTION;
//     m_dspLoopAction[0] = false;
//     m_loopActive[1] = true;
//     m_loopParam[1] = BOC_RX_THR;
//     m_dspProcessing[1] = false;
//     setLoopVarValues(1, 50, 250, 6);
//     m_loopAction[1] = NO_ACTION;
//     m_dspLoopAction[1] = false;
//     m_runType = RAW_PATTERN;
//     m_histogramFilled[OCCUPANCY] = false;
//     m_histogramKept[OCCUPANCY] = false;
//     m_histogramFilled[RAW_DATA_0] = true;
//     m_histogramKept[RAW_DATA_0] = false;
//     m_histogramFilled[RAW_DATA_1] = true;
//     m_histogramKept[RAW_DATA_1] = true;
//     m_histogramFilled[RAW_DATA_DIFF_1] = true;
//     m_histogramKept[RAW_DATA_DIFF_1] = true;
//     m_histogramFilled[RAW_DATA_DIFF_2] = true;
//     m_histogramKept[RAW_DATA_DIFF_2] = true;
//     m_histogramFilled[RAW_DATA_REF] = true;
//     m_histogramKept[RAW_DATA_REF] = true;
  } else if (presetName == IV_SCAN){
    m_repetitions = 1;
    m_maskStageSteps = 1;
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_loopParam[0] = DCS_VOLTAGE;
    m_dspProcessing[0] = false;
    std::vector<float> st;
    st.push_back(-3.);
    for(float vbias=-10;vbias>=-150;vbias-=10.)
      st.push_back(vbias);
    setLoopVarValues(0, st);
    m_loopVarMin[0] = -3.;
    m_loopVarMax[0] = -150.;
    m_dspLoopAction[0] = false;
    m_readDcsChan = "Vbias";
    m_readDcsMode = CURRENT;
    m_scanDcsChan = "Vbias";
    m_histogramFilled[OCCUPANCY] = false;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[DCS_DATA] = true;
    m_histogramKept[DCS_DATA] = true;
  } else if (presetName == DAC_SCAN){
    m_repetitions = 1;
    m_maskStageSteps = 1;
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    m_dspProcessing[0] = false;
    setLoopVarValues(0, 0, 1023, 1024);
    m_dspLoopAction[0] = false;
    m_readDcsChan = "VCAL_MEAS";
    m_readDcsMode = VOLTAGE;
    m_histogramFilled[OCCUPANCY] = false;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[DCS_DATA] = true;
    m_histogramKept[DCS_DATA] = true;
  } else if (presetName == MONLEAK) {
    m_repetitions = 1;
    m_maskStageSteps = 1;
    m_digitalInjection = false;
    m_loopActive[0] = false;
    m_maskStageSteps = 2880;
    m_maskStageTotalSteps = STEPS_2880;
    m_maskStageMode = HITB;
    m_dspMaskStaging = false;
    m_strobeDuration = 0;
    m_histogramFilled[OCCUPANCY] = false;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[MON_LEAK] = true;
    m_histogramKept[MON_LEAK] = true;
  } else if (presetName== HITBUS_SCALER){
    m_repetitions = 1000;
    m_maskStageSteps = 1;
    m_digitalInjection = false;
    m_loopActive[0] = false;
    m_maskStageSteps = 2880;
    m_maskStageTotalSteps = STEPS_2880;
    m_maskStageMode = HITB;
    m_dspMaskStaging = false;
    m_strobeDuration = 1000;
    m_histogramFilled[OCCUPANCY] = false;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[HB_SCALER_CNT] = true;
    m_histogramKept[HB_SCALER_CNT] = true;
  } else if (presetName== EUDAQ){
    m_repetitions = 9999999;
    m_maskStageSteps = 1;
    m_strobeLVL1Delay = 247;
    m_digitalInjection = false;
    m_loopActive[0] = false;
    m_isSourceScan = true;
    m_isTestBeam = true;
    m_srcTriggerType = TLU_DATA_HANDSHAKE;
    m_configAtStart=false;
    m_maskStageTotalSteps = STEPS_USER;
    m_maskStageMode = STATIC;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = true;
    m_histogramFilled[TOT] = true;
    m_histogramKept[TOT] = true;
    m_histogramFilled[LVL1] = true;
    m_histogramKept[LVL1] = true;
  } else if (presetName== SOURCE_SCAN || presetName== FE_ST_SOURCE_SCAN){
    // self-trigger not implemented, thus set this as ext. trg. source scan
    m_repetitions = 1000;
    m_maskStageSteps = 1;
    m_strobeLVL1Delay = 247;
    m_digitalInjection = false;
    m_loopActive[0] = false;
    m_isSourceScan = true;       
    m_maskStageMode = STATIC;
    m_maskStageTotalSteps = STEPS_USER;
    m_srcTriggerType = EXT_TRG;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = true;
    m_histogramFilled[TOT] = true;
    m_histogramKept[TOT] = true;
    m_histogramFilled[LVL1] = true;
    m_histogramKept[LVL1] = true;
    /*
    m_histogramFilled[CLUSTER_TOT] = true;
    m_histogramKept[CLUSTER_TOT] = true;
    m_histogramFilled[CLUSTER_SIZE] = true;
    m_histogramKept[CLUSTER_SIZE] = true;
    m_histogramFilled[CLSIZE_TOT] = true;
    m_histogramKept[CLSIZE_TOT] = true;
    m_histogramFilled[HITOCC] = true;
    m_histogramKept[HITOCC] = true;
    m_histogramFilled[NSEEDS] = true;
    m_histogramKept[NSEEDS] = true;
    m_histogramFilled[SEED_TOT] = true;
    m_histogramKept[SEED_TOT] = true;
    m_histogramFilled[SEED_LVL1] = true;
    m_histogramKept[SEED_LVL1] = true;
    */
  } else if (presetName == TOT_VERIF) {
    m_repetitions = 200;
    m_maskStageSteps = 32;
    m_maskStageMode = SEL_ENA;
    m_strobeDuration = 500;
    m_digitalInjection = false;
    m_loopActive[0] = true;
    m_dspProcessing[0] = false;
    m_dspLoopAction[0] = false;
    setLoopVarValues(0, 0, 0, 1);
    m_loopParam[0] = NO_PAR;
    m_loopAction[0] = FDAC_TUNING;
    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = false;
    m_histogramFilled[TOT_MEAN] = true;
    m_histogramKept[TOT_MEAN] = true;
    m_histogramFilled[TOT_SIGMA] = true;
    m_histogramKept[TOT_SIGMA] = true;
    m_totTargetValue = 30;
    m_totTargetCharge = 20000;
    m_restoreModuleConfig = true;
    m_feVCal = 0x1fff;
  } else if (presetName == THR_FAST_SCAN) {
    m_maskStageSteps = 32;
    m_strobeDuration = 500;
    m_feVCal = 0x1fff;              
    m_repetitions = 100;
    m_innerLoopSwap = true;  
    m_digitalInjection = false;

    m_loopActive[0] = true;
    m_loopParam[0] = VCAL;
    std::vector<float> vcalSteps;
    vcalSteps.push_back(512);
    vcalSteps.push_back(256);
    vcalSteps.push_back(128);
    vcalSteps.push_back(64);
    vcalSteps.push_back(32);
    vcalSteps.push_back(16);
    vcalSteps.push_back(8);
    vcalSteps.push_back(4);
    vcalSteps.push_back(2);
    vcalSteps.push_back(1);
    vcalSteps.push_back(1);
    setLoopVarValues(0,vcalSteps);
    m_loopAction[0] = THR_FAST_SCANNING;
    m_dspProcessing[0] = false;
    m_dspLoopAction[0] = false;

    m_histogramFilled[OCCUPANCY] = true;
    m_histogramKept[OCCUPANCY] = true;
    m_histogramFilled[SCURVE_MEAN] = true;
    m_histogramKept[SCURVE_MEAN] = true;
    m_histogramFilled[SCURVE_SIGMA] = true;

    m_restoreModuleConfig = true;

  } else {
    throw PixScanExc(PixControllerExc::ERROR, "Undefined scan preset");
  }
}
void PixScan::presetI4A(ScanType presetName) {
  presetI4B(presetName);
}
void PixScan::presetI4B(ScanType presetName) {
	if (presetName == DIGITAL_TEST) {
		m_strobeDuration = 50;
		m_maskStageMode = ENA;
		m_digitalInjection = true;
		m_avoidSpecialsCols = false;
	} else if (presetName == ANALOG_TEST) {
		m_feVCal = 400;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_digitalInjection = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
	} else if (presetName == THRESHOLD_SCAN) {
		m_repetitions = 100;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = true;
		setLoopVarValues(0, 0, 200, 201);
		m_loopAction[0] = SCURVE_FIT;
		m_dspLoopAction[0] = false;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		m_histogramKept[SCURVE_SIGMA] = true;
		m_histogramFilled[SCURVE_CHI2] = true;
		m_histogramKept[SCURVE_CHI2] = true;
	} else if (presetName == TDAC_TUNE || presetName == GDAC_TUNE) {
		m_repetitions = 50;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = true;
		setLoopVarValues(0, 0, 200, 201);
		m_loopAction[0] = SCURVE_FIT;
		m_dspLoopAction[0] = false;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		if (presetName == TDAC_TUNE) {
			m_loopParam[1] = TDACS;
			std::vector<float> st;
			st.push_back(16);
			st.push_back(8);
			st.push_back(4);
			st.push_back(2);
			st.push_back(1);
			st.push_back(1);
			setLoopVarValues(1, st);
			m_loopAction[1] = TDAC_TUNING;
			m_histogramFilled[TDAC_T] = true;
			m_histogramKept[TDAC_T] = true;
			m_histogramFilled[TDAC_THR] = true;
			m_histogramKept[TDAC_THR] = true;
		} else {
			m_maskStageSteps = 1;
			m_loopParam[1] = GDAC;
			std::vector<float> st;
			st.push_back(140);
			st.push_back(160);
			st.push_back(185);
			st.push_back(218);
			st.push_back(260);
			setLoopVarValues(1, st);
			m_loopAction[1] = GDAC_TUNING;
			m_histogramFilled[GDAC_T] = true;
			m_histogramKept[GDAC_T] = true;
			m_histogramFilled[GDAC_THR] = true;
			m_histogramKept[GDAC_THR] = true;
			m_chicut = 50.; // Must be strict, otherwise average gets spoiled
		}
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		m_histogramKept[SCURVE_SIGMA] = true;
		m_histogramFilled[SCURVE_CHI2] = true;
		m_histogramKept[SCURVE_CHI2] = true;
		m_thresholdTargetValue = 3000;
		m_restoreModuleConfig = false;
	} else if (presetName == GDAC_FAST_TUNE || presetName == GDAC_FAST_TUNE2) {
		m_repetitions = 200;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_digitalInjection = false;
		setLoopVarValues(0, 0, 2, 1);
		m_loopActive[0] = true;
		m_dspProcessing[0] = true;
		m_dspLoopAction[0] = false;
		m_loopParam[0] = NO_PAR;
		m_loopAction[0] = OCC_SUM;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_loopParam[1] = (presetName == GDAC_FAST_TUNE)?GDAC:FEI4_GR;
		if(presetName == GDAC_FAST_TUNE2) setLoopFEI4GR(1, "Vthin_AltFine");
		std::vector<float> st;
		if(presetName == GDAC_FAST_TUNE) st.push_back(256);
		st.push_back(128);
		st.push_back(64);
		st.push_back(32);
		st.push_back(16);
		st.push_back(8);
		st.push_back(4);
		st.push_back(2);
		st.push_back(1);
		st.push_back(1);
		setLoopVarValues(1, st);
		m_loopAction[1] = GDAC_FAST_TUNING;
		m_loopVarValuesFree[1] = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_histogramFilled[GDAC_T] = true;
		m_histogramKept[GDAC_T] = true;
		m_histogramFilled[GDAC_OCC] = true;
		m_histogramKept[GDAC_OCC] = true;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SUM_OCC] = true;
		m_histogramKept[SUM_OCC] = true;
		m_thresholdTargetValue = 3000;
		m_restoreModuleConfig = false;
		m_feVCal = 0x1fff;
	} else if (presetName == TDAC_FAST_TUNE) {
		m_repetitions = 200;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_digitalInjection = false;
		setLoopVarValues(0, 0, 2, 2);
		m_loopActive[0] = true;
		m_dspProcessing[0] = true;
		m_dspLoopAction[0] = false;
		m_loopParam[0] = NO_PAR;
		m_loopAction[0] = OCC_SUM;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_loopParam[1] = TDACS;
		std::vector<float> st;
		st.push_back(16);
		st.push_back(8);
		st.push_back(4);
		st.push_back(2);
		st.push_back(1);
		st.push_back(1);
		setLoopVarValues(1, st);
		m_loopAction[1] = TDAC_FAST_TUNING;
		m_loopVarValuesFree[1] = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_histogramFilled[TDAC_T] = true;
		m_histogramKept[TDAC_T] = true;
		m_histogramFilled[TDAC_OCC] = true;
		m_histogramKept[TDAC_OCC] = true;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SUM_OCC] = true;
		m_histogramKept[SUM_OCC] = true;
		m_thresholdTargetValue = 3000;
		m_restoreModuleConfig = false;
		m_feVCal = 0x1fff;
	} else if (presetName == FDAC_TUNE || presetName == IF_TUNE) {
		m_repetitions = 25;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_digitalInjection = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageTotalSteps = STEPS_3;
		m_maskStageSteps = 3;
		m_loopActive[0] = true;
		m_dspProcessing[0] = false;
		m_dspLoopAction[0] = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		if (presetName == FDAC_TUNE) {
			m_loopParam[0] = FDACS;
			setLoopVarValues(0, 0, 15, 16);
			m_loopAction[0] = FDAC_TUNING;
			m_histogramFilled[FDAC_T] = true;
			m_histogramKept[FDAC_T] = true;
			m_histogramFilled[FDAC_TOT] = true;
			m_histogramKept[FDAC_TOT] = true;
		} else {
			m_loopParam[0] = IF;
			setLoopVarValues(0, 0, 250, 26);
			m_loopAction[0] = IF_TUNING;
			m_histogramFilled[IF_T] = true;
			m_histogramKept[IF_T] = true;
			m_histogramFilled[IF_TOT] = true;
			m_histogramKept[IF_TOT] = true;
		}
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[TOT_MEAN] = true;
		m_histogramKept[TOT_MEAN] = true;
		m_histogramFilled[TOT_SIGMA] = true;
		m_histogramKept[TOT_SIGMA] = false;
		m_totTargetValue = 6;
		m_totTargetCharge = 20000;
		m_restoreModuleConfig = false;
		m_feVCal = 0x1fff;
	} else if (presetName == INJ_CALIB){
		m_repetitions = 50;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 1;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = true;
		setLoopVarValues(0, 0, 400, 401);
		m_loopAction[0] = SCURVE_FIT;
		m_dspLoopAction[0] = false;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_maskStageSteps = 1;
		m_loopParam[1] = CAPSEL;
		setLoopVarValues(1, 0, 2, 3);
		m_loopAction[1] = OFFSET_CALIB;
		//m_histogramFilled[GDAC_T] = true;
		//m_histogramKept[GDAC_T] = true;
		m_chicut = 50.; // Must be strict, otherwise average gets spoiled
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		m_histogramKept[SCURVE_SIGMA] = true;
		m_histogramFilled[SCURVE_CHI2] = true;
		m_histogramKept[SCURVE_CHI2] = true;
		m_thresholdTargetValue = 3000;
		m_restoreModuleConfig = false;
	} else if (presetName == TOT_CALIB) {
		m_repetitions = 50;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = false; // JGK changed
		setLoopVarValues(0, 50, 700, 27);
		m_dspLoopAction[0] = false;
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[TOT0] = true;
		m_histogramKept  [TOT0] = true;
		m_histogramFilled[TOT1] = true;
		m_histogramKept  [TOT1] = true;
		m_histogramFilled[TOT2] = true;
		m_histogramKept  [TOT2] = true;
		m_histogramFilled[TOT3] = true;
		m_histogramKept  [TOT3] = true;
		m_histogramFilled[TOT4] = true;
		m_histogramKept  [TOT4] = true;
		m_histogramFilled[TOT5] = true;
		m_histogramKept  [TOT5] = true;
		m_histogramFilled[TOT6] = true;
		m_histogramKept  [TOT6] = true;
		m_histogramFilled[TOT7] = true;
		m_histogramKept  [TOT7] = true;
		m_histogramFilled[TOT8] = true;
		m_histogramKept  [TOT8] = true;
		m_histogramFilled[TOT9] = true;
		m_histogramKept  [TOT9] = true;
		m_histogramFilled[TOT10] = true;
		m_histogramKept  [TOT10] = true;
		m_histogramFilled[TOT11] = true;
		m_histogramKept  [TOT11] = true;
		m_histogramFilled[TOT12] = true;
		m_histogramKept  [TOT12] = true;
		m_histogramFilled[TOT13] = true;
		m_histogramKept  [TOT13] = true;
		m_histogramFilled[TOT14] = true;
		m_histogramKept  [TOT14] = true;
		m_histogramFilled[TOT15] = true;
		m_histogramKept  [TOT15] = true;
		m_histogramFilled[TOTCAL_PARA] = false;
		m_histogramKept[TOTCAL_PARA] = true;
		m_histogramFilled[TOTCAL_PARB] = false;
		m_histogramKept[TOTCAL_PARB] = true;
		m_histogramFilled[TOTCAL_PARC] = false;
		m_histogramKept[TOTCAL_PARC] = true;
		m_histogramFilled[TOTCAL_CHI2] = false;
		m_histogramKept[TOTCAL_CHI2] = true;
		//m_loopAction[0] =  NO_ACTION;//TOTCAL_FIT;
		m_loopAction[0] = TOTCAL_FEI4;
	} else if (presetName == TOT_CALIB_LUT) {
		m_repetitions = 200;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = false;
		setLoopVarValues(0, 0, 700, 141);
		m_dspLoopAction[0] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[TOT0] = true;
		m_histogramKept  [TOT0] = false;
		m_histogramFilled[TOT1] = true;
		m_histogramKept  [TOT1] = false;
		m_histogramFilled[TOT2] = true;
		m_histogramKept  [TOT2] = false;
		m_histogramFilled[TOT3] = true;
		m_histogramKept  [TOT3] = false;
		m_histogramFilled[TOT4] = true;
		m_histogramKept  [TOT4] = false;
		m_histogramFilled[TOT5] = true;
		m_histogramKept  [TOT5] = false;
		m_histogramFilled[TOT6] = true;
		m_histogramKept  [TOT6] = false;
		m_histogramFilled[TOT7] = true;
		m_histogramKept  [TOT7] = false;
		m_histogramFilled[TOT8] = true;
		m_histogramKept  [TOT8] = false;
		m_histogramFilled[TOT9] = true;
		m_histogramKept  [TOT9] = false;
		m_histogramFilled[TOT10] = true;
		m_histogramKept  [TOT10] = false;
		m_histogramFilled[TOT11] = true;
		m_histogramKept  [TOT11] = false;
		m_histogramFilled[TOT12] = true;
		m_histogramKept  [TOT12] = false;
		m_histogramFilled[TOT13] = true;
		m_histogramKept  [TOT13] = false;
		m_histogramFilled[TOT14] = true;
		m_histogramKept  [TOT14] = false;
		m_histogramFilled[TOT15] = true;
		m_histogramKept  [TOT15] = false;
		m_restoreModuleConfig = true;
		m_loopAction[0] = TOT_CHARGE_LUT;
		m_restoreModuleConfig = false;
	} else if (presetName == CHARGE_CALIB_VERIF) {
		m_repetitions = 200;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = false;
		setLoopVarValues(0, 100, 700, 7);
		m_dspLoopAction[0] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramFilled[TOT0] = true;
		m_histogramKept  [TOT0] = false;
		m_histogramFilled[TOT1] = true;
		m_histogramKept  [TOT1] = false;
		m_histogramFilled[TOT2] = true;
		m_histogramKept  [TOT2] = false;
		m_histogramFilled[TOT3] = true;
		m_histogramKept  [TOT3] = false;
		m_histogramFilled[TOT4] = true;
		m_histogramKept  [TOT4] = false;
		m_histogramFilled[TOT5] = true;
		m_histogramKept  [TOT5] = false;
		m_histogramFilled[TOT6] = true;
		m_histogramKept  [TOT6] = false;
		m_histogramFilled[TOT7] = true;
		m_histogramKept  [TOT7] = false;
		m_histogramFilled[TOT8] = true;
		m_histogramKept  [TOT8] = false;
		m_histogramFilled[TOT9] = true;
		m_histogramKept  [TOT9] = false;
		m_histogramFilled[TOT10] = true;
		m_histogramKept  [TOT10] = false;
		m_histogramFilled[TOT11] = true;
		m_histogramKept  [TOT11] = false;
		m_histogramFilled[TOT12] = true;
		m_histogramKept  [TOT12] = false;
		m_histogramFilled[TOT13] = true;
		m_histogramKept  [TOT13] = false;
		m_histogramFilled[TOT14] = true;
		m_histogramKept  [TOT14] = false;
		m_histogramFilled[TOT15] = true;
		m_histogramKept  [TOT15] = false;
		m_histogramFilled[CHARGE_MEAN] = true;
		m_histogramKept  [CHARGE_MEAN] = true;
		m_histogramFilled[CHARGE_SIGMA] = true;
		m_histogramKept  [CHARGE_SIGMA] = true;
		m_histogramFilled[CHARGE_RECO_ERROR] = true;
		m_histogramKept  [CHARGE_RECO_ERROR] = true;
		m_restoreModuleConfig = true;
		m_loopAction[0] = TOT_TO_CHARGE;
	} else if (presetName == T0_SCAN) {
		m_repetitions = 25;
		m_feVCal = 1000;
		m_totTargetCharge = 100000;
		m_chargeInjCap = 2;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageTotalSteps = STEPS_3;
		m_maskStageSteps = 1;
		m_consecutiveLvl1TrigA[0] = 1;
		m_consecutiveLvl1TrigB[0] = 1;
		m_strobeDuration = 500;
		m_digitalInjection = false;
		m_loopActive[0] = true;
		m_loopParam[0] = STROBE_DELAY;
		m_dspProcessing[0] = false;
		setLoopVarValues(0, 1, 62, 62);
		m_chicut = 5000.; // Must be strict, otherwise average gets spoiled
		m_dspLoopAction[0] = false;
		m_loopAction[0] = MCCDEL_FIT;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_loopParam[1] = TRIGGER_DELAY;
		setLoopVarValues(1, 55, 58, 4);
		m_loopAction[1] = T0_SET;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;//true;
		m_histogramFilled[TIMEWALK] = true;
		m_histogramKept[TIMEWALK] = true;
		m_restoreModuleConfig = false;
	} else if (presetName == DISCBIAS_TUNE) {
		m_repetitions = 25;
		m_feVCal = 1000;
		m_totTargetCharge = 100000;
		m_chargeInjCap = 2;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageTotalSteps = STEPS_3;
		m_maskStageSteps = 1;
		m_consecutiveLvl1TrigA[0] = 1;
		m_consecutiveLvl1TrigB[0] = 1;
		m_strobeDuration = 500;
		m_strobeLVL1DelayOverride = false;
		m_digitalInjection = false;
		m_loopActive[0] = true;
		m_loopParam[0] = STROBE_DELAY;
		m_dspProcessing[0] = false;
		setLoopVarValues(0, 1, 62, 62);
		m_chicut = 5000.; // Must be strict, otherwise average gets spoiled
		m_dspLoopAction[0] = false;
		m_loopAction[0] = MCCDEL_FIT;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_loopParam[1] = DISCBIAS;
		std::vector<float> st;
		st.push_back(-1);
		st.push_back(8);
		st.push_back(4);
		st.push_back(2);
		st.push_back(1);
		st.push_back(1);
		setLoopVarValues(1, st);
		m_loopAction[1] = DISCBIAS_TUNING;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[TIMEWALK] = true;
		m_histogramKept[TIMEWALK] = true;
		m_histogramFilled[DISCBIAS_T] = true;
		m_histogramKept[DISCBIAS_T] = true;
		m_histogramFilled[DISCBIAS_TIMEWALK] = true;
		m_histogramKept[DISCBIAS_TIMEWALK] = true;
		m_restoreModuleConfig = false;
	} else if (presetName == TIMEWALK_MEASURE) {
		m_repetitions = 25;
		m_chargeInjCap = 2;
		m_consecutiveLvl1TrigA[0] = 1;
		m_consecutiveLvl1TrigB[0] = 1;
		m_strobeDuration = 500;
		m_digitalInjection = false;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageTotalSteps = STEPS_3;
		m_maskStageSteps = 3;
		m_strobeLVL1DelayOverride = false;
		m_loopActive[0] = true;
		m_loopParam[0] = STROBE_DELAY;
		m_dspProcessing[0] = false;
		setLoopVarValues(0, 0, 63, 64);
		m_loopAction[0] = MCCDEL_FIT;
		m_dspLoopAction[0] = false;//true;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_loopParam[1] = VCAL;
		std::vector<float> vcalVals;
		vcalVals.push_back(70);
		vcalVals.push_back(75);
		vcalVals.push_back(80);
		vcalVals.push_back(85);
		vcalVals.push_back(90);
		vcalVals.push_back(95);
		vcalVals.push_back(100);
		vcalVals.push_back(110);
		vcalVals.push_back(120);
		vcalVals.push_back(130);
		vcalVals.push_back(140);
		vcalVals.push_back(160);
		vcalVals.push_back(180);
		vcalVals.push_back(200);
		vcalVals.push_back(220);
		vcalVals.push_back(240);
		vcalVals.push_back(280);
		vcalVals.push_back(320);
		vcalVals.push_back(360);
		vcalVals.push_back(400);
		vcalVals.push_back(500);
		vcalVals.push_back(600);
		vcalVals.push_back(700);
		vcalVals.push_back(800);
		vcalVals.push_back(1000);
		setLoopVarValues(1, vcalVals);
		m_loopAction[1] = NO_ACTION;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[TIMEWALK] = true;
		m_histogramKept[TIMEWALK] = true;
		m_restoreModuleConfig = true;
	} else if (presetName == INTIME_THRESH_SCAN) {
		m_repetitions = 100;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_consecutiveLvl1TrigA[0] = 1;
		m_consecutiveLvl1TrigB[0] = 1;
		m_strobeLVL1DelayOverride = false;
		m_strobeMCCDelayRange = 31; // not used, but forbids overwrite
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = true;
		setLoopVarValues(0, 0, 200, 201);
		m_loopAction[0] = SCURVE_FIT;
		m_dspLoopAction[0] = false;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		m_histogramKept[SCURVE_SIGMA] = true;
		m_histogramFilled[SCURVE_CHI2] = true;
		m_histogramKept[SCURVE_CHI2] = true;
	} else if (presetName == CROSSTALK_SCAN) {
		m_repetitions = 100;
		m_strobeDuration = 500;
		m_maskStageMode = XTALK;
		m_maskStageSteps = 8;
		m_maskStageTotalSteps = STEPS_8;
		m_digitalInjection = false;
		m_chargeInjCap = 2;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		setLoopVarValues(0, 0, 1000, 26);
		m_loopAction[0] = SCURVE_FIT;
		m_dspLoopAction[0] = false;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		m_histogramKept[SCURVE_SIGMA] = true;
		m_histogramFilled[SCURVE_CHI2] = true;
		m_histogramKept[SCURVE_CHI2] = true;
	} else if (presetName == INCREMENTAL_TDAC_SCAN) {
		m_repetitions = 25;
		m_strobeDuration = 500;
		m_digitalInjection = false;
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		m_dspProcessing[0] = true;
		setLoopVarValues(0, 0, 200, 201);
		m_loopAction[0] = SCURVE_FIT;
		m_dspLoopAction[0] = false;
		m_loopActive[1] = true;
		m_dspProcessing[1] = false;
		m_dspLoopAction[1] = false;
		m_loopParam[1] = TDACS_VARIATION;
		std::vector<float> st;
		st.push_back(-5);
		setLoopVarValues(1, st);
		setLoopVarValuesFree(1);
		m_loopAction[1] = MIN_THRESHOLD;
		m_histogramFilled[TDAC_T] = true;
		m_histogramKept[TDAC_T] = true;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		m_histogramKept[SCURVE_SIGMA] = true;
		m_histogramFilled[SCURVE_CHI2] = true;
		m_histogramKept[SCURVE_CHI2] = true;
// 	} else if (presetName == BOC_RX_DELAY_SCAN) {
// 		m_repetitions = 1;
// 		m_maskStageSteps = 0;
// 		m_maskStageMode = STATIC;
// 		m_loopActive[0] = false;
// 		m_loopParam[0] = NO_PAR;
// 		m_dspProcessing[0] = true;
// 		m_loopAction[0] = NO_ACTION;
// 		m_dspLoopAction[0] = false;
// 		m_runType = RAW_PATTERN;
// 		//m_restoreModuleConfig = false;
// 		m_histogramFilled[OCCUPANCY] = false;
// 		m_histogramKept[OCCUPANCY] = false;
// 		m_histogramFilled[RAW_DATA_0] = true;
// 		m_histogramKept[RAW_DATA_0] = true;
// 		//     m_histogramFilled[RAW_DATA_1] = true;
// 		//     m_histogramKept[RAW_DATA_1] = true;
// 		//     m_histogramFilled[RAW_DATA_DIFF_1] = true;
// 		//     m_histogramKept[RAW_DATA_DIFF_1] = true;
// 		//     m_histogramFilled[RAW_DATA_REF] = true;
// 		//     m_histogramKept[RAW_DATA_REF] = true;
// 	} else if (presetName == BOC_V0_RX_DELAY_SCAN) {
// 		m_repetitions = 10;
// 		m_maskStageSteps = 0;
// 		m_maskStageMode = STATIC;
// 		m_loopActive[0] = true;
// 		m_loopParam[0] = BOC_RX_DELAY;
// 		m_dspProcessing[0] = false;
// 		setLoopVarValues(0, 0, 24, 13);
// 		m_loopAction[0] = NO_ACTION;
// 		m_dspLoopAction[0] = false;
// 		m_loopActive[1] = true;
// 		m_loopParam[1] = BOC_VPH0;
// 		m_dspProcessing[1] = false;
// 		setLoopVarValues(1, 0, 24, 13);
// 		m_loopAction[1] = NO_ACTION;
// 		m_dspLoopAction[1] = false;
// 		m_runType = RAW_EVENT;
// 		m_histogramFilled[OCCUPANCY] = false;
// 		m_histogramKept[OCCUPANCY] = false;
// 		m_histogramFilled[RAW_DATA_0] = true;
// 		m_histogramKept[RAW_DATA_0] = false;
// 		m_histogramFilled[RAW_DATA_1] = true;
// 		m_histogramKept[RAW_DATA_1] = true;
// 		m_histogramFilled[RAW_DATA_DIFF_1] = true;
// 		m_histogramKept[RAW_DATA_DIFF_1] = true;
// 		m_histogramFilled[RAW_DATA_DIFF_2] = true;
// 		m_histogramKept[RAW_DATA_DIFF_2] = true;
// 		m_histogramFilled[RAW_DATA_REF] = true;
// 		m_histogramKept[RAW_DATA_REF] = true;
// 	} else if (presetName == BOC_THR_RX_DELAY_SCAN) {
// 		m_repetitions = 10;
// 		m_maskStageSteps = 0;
// 		m_maskStageMode = STATIC;
// 		m_loopActive[0] = true;
// 		m_loopParam[0] = BOC_RX_DELAY;
// 		m_dspProcessing[0] = false;
// 		setLoopVarValues(0, 0, 24, 25);
// 		m_loopAction[0] = NO_ACTION;
// 		m_dspLoopAction[0] = false;
// 		m_loopActive[1] = true;
// 		m_loopParam[1] = BOC_RX_THR;
// 		m_dspProcessing[1] = false;
// 		setLoopVarValues(1, 50, 250, 6);
// 		m_loopAction[1] = NO_ACTION;
// 		m_dspLoopAction[1] = false;
// 		m_runType = RAW_PATTERN;
// 		m_histogramFilled[OCCUPANCY] = false;
// 		m_histogramKept[OCCUPANCY] = false;
// 		m_histogramFilled[RAW_DATA_0] = true;
// 		m_histogramKept[RAW_DATA_0] = false;
// 		m_histogramFilled[RAW_DATA_1] = true;
// 		m_histogramKept[RAW_DATA_1] = true;
// 		m_histogramFilled[RAW_DATA_DIFF_1] = true;
// 		m_histogramKept[RAW_DATA_DIFF_1] = true;
// 		m_histogramFilled[RAW_DATA_DIFF_2] = true;
// 		m_histogramKept[RAW_DATA_DIFF_2] = true;
// 		m_histogramFilled[RAW_DATA_REF] = true;
// 		m_histogramKept[RAW_DATA_REF] = true;
	} else if (presetName == IV_SCAN){
		m_repetitions = 1;
		m_maskStageSteps = 1;
		m_digitalInjection = false;
		m_loopActive[0] = true;
		m_loopParam[0] = DCS_VOLTAGE;
		m_dspProcessing[0] = false;
		std::vector<float> st;
		st.push_back(-3.);
		for(float vbias=-10;vbias>=-150;vbias-=10.)
			st.push_back(vbias);
		setLoopVarValues(0, st);
		m_loopVarMin[0] = -3.;
		m_loopVarMax[0] = -150.;
		m_dspLoopAction[0] = false;
		m_readDcsChan = "Vbias";
		m_readDcsMode = CURRENT;
		m_scanDcsChan = "Vbias";
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[DCS_DATA] = true;
		m_histogramKept[DCS_DATA] = true;
	} else if (presetName == DAC_SCAN){
		m_repetitions = 1;
		m_maskStageSteps = 1;
		m_digitalInjection = false;
		m_loopActive[0] = true;
		m_loopParam[0] = FEI4_GR;
		m_loopFEI4GR[0] = "PlsrDAC";
		m_dspProcessing[0] = false;
		setLoopVarValues(0, 0, 1023, 1024);
		m_dspLoopAction[0] = false;
		m_readDcsChan = "VCAL_MEAS";
		m_readDcsMode = VOLTAGE;
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[DCS_DATA] = true;
		m_histogramKept[DCS_DATA] = true;
	} else if (presetName == MONLEAK) {
		m_repetitions = 1;
		m_loopActive[0] = true;
		m_dspProcessing[0] = false;
		m_dspLoopAction[0] = false;
		m_loopParam[0] = FEI4_GR;
		m_loopFEI4GR[0] = "Colpr_Addr";
		setLoopVarValues(0, 0, 39, 40);
		m_loopAction[0] = NO_ACTION;
		m_maskStageSteps = 672;
		m_maskStageTotalSteps = STEPS_26880;
		m_maskStageMode = HITB_INV;
		m_dspMaskStaging = false;
		m_loopOverDcs = false;
		m_avoidSpecialsCols = false;
		m_strobeDuration = 0;
		m_readDcsChan = "IleakOut";
		m_readDcsMode = VOLTAGE;
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[MON_LEAK] = true;
		m_histogramKept[MON_LEAK] = true;
	} else if (presetName== HITBUS_SCALER){
		m_repetitions = 1;
		m_loopActive[0] = true;
		m_dspProcessing[0] = false;
		m_dspLoopAction[0] = false;
		m_loopParam[0] = FEI4_GR;
		m_loopFEI4GR[0] = "Colpr_Addr";
		setLoopVarValues(0, 0, 39, 40);
		m_loopAction[0] = NO_ACTION;
		m_maskStageSteps = 672;
		m_maskStageTotalSteps = STEPS_26880;
		m_maskStageMode = HITB;
		m_dspMaskStaging = false;
		m_loopOverDcs = false;
		m_avoidSpecialsCols = false;
		m_strobeDuration = 0;
		m_readDcsChan = "";
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[HB_SCALER_CNT] = true;
		m_histogramKept[HB_SCALER_CNT] = true;
		// old version with data taking
		//     m_strobeLVL1Delay = 30;
		//     m_feVCal = 0;
		//     m_strobeDuration = 50;
		//     m_digitalInjection = true;
		//     m_repetitions = 10;
		//     m_loopActive[0] = true;
		//     m_dspProcessing[0] = false;
		//     m_dspLoopAction[0] = false;
		//     m_loopParam[0] = FEI4_GR;
		//     m_loopFEI4GR[0] = "Colpr_Addr";
		//     setLoopVarValues(0, 0, 39, 40);
		//     m_loopAction[0] = NO_ACTION;
		//     m_maskStageSteps = 672;
		//     m_maskStageTotalSteps = STEPS_26880;
		//     m_maskStageMode = ENA_HITB;
		//     m_dspMaskStaging = true;
		//     m_loopOverDcs = false;
		//     m_avoidSpecialsCols = false;
		//     m_srcTriggerType = EXT_TRG;
		//     m_histogramFilled[OCCUPANCY] = true;
		//     m_histogramKept[OCCUPANCY] = true;
	} else if (presetName== EUDAQ){
		m_repetitions = 9999999;
		m_maskStageSteps = 1;
		m_strobeLVL1Delay = 26;
		m_digitalInjection = false;
		m_loopActive[0] = false;
		m_isSourceScan = true;
		m_isTestBeam = true;
		m_skipSourceScanHistos=true;
		m_lowerLVL1DelayIfSlaveBy = 2;
		m_TLUTriggerDataDelay = 10;
		m_maskStageTotalSteps = STEPS_USER;
		m_maskStageMode = STATIC;
		m_srcTriggerType=TLU_DATA_HANDSHAKE;
		m_configAtStart=false;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[TOT] = true;
		m_histogramKept[TOT] = true;
		m_histogramFilled[LVL1] = true;
		m_histogramKept[LVL1] = true;
		m_histogramFilled[LV1ID] = true;
		m_histogramKept[LV1ID] = true;
		m_histogramFilled[BCID] = true;
		m_histogramKept[BCID] = true;
		// clustering is too time-consuming
// 		m_histogramFilled[CLUSTER_TOT] = true;
// 		m_histogramKept[CLUSTER_TOT] = true;
// 		m_histogramFilled[CLUSTER_SIZE] = true;
// 		m_histogramKept[CLUSTER_SIZE] = true;
// 		m_histogramFilled[CLSIZE_TOT] = true;
// 		m_histogramKept[CLSIZE_TOT] = true;
// 		m_histogramFilled[HITOCC] = true;
// 		m_histogramKept[HITOCC] = true;
// 		m_histogramFilled[NSEEDS] = true;
// 		m_histogramKept[NSEEDS] = true;
// 		m_histogramFilled[SEED_TOT] = true;
// 		m_histogramKept[SEED_TOT] = true;
// 		m_histogramFilled[SEED_LVL1] = true;
// 		m_histogramKept[SEED_LVL1] = true;
	} else if (presetName== SOURCE_SCAN){
		m_repetitions = 1000;
		m_maskStageSteps = 1;
		m_strobeLVL1Delay = 26;
		m_lowerLVL1DelayIfSlaveBy = 2;
		m_TLUTriggerDataDelay = 10;
		m_digitalInjection = false;
		m_loopActive[0] = false;
		m_isSourceScan = true;
		m_maskStageTotalSteps = STEPS_USER;
		m_maskStageMode = STATIC;
		m_srcTriggerType = EXT_TRG;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[TOT] = true;
		m_histogramKept[TOT] = true;
		m_histogramFilled[LVL1] = true;
		m_histogramKept[LVL1] = true;
// 		m_histogramFilled[CLUSTER_TOT] = true;
// 		m_histogramKept[CLUSTER_TOT] = true;
// 		m_histogramFilled[CLUSTER_CHARGE] = false;	//DLP
// 		m_histogramKept[CLUSTER_CHARGE] = false;	//DLP
// 		m_histogramFilled[CLSIZE_CHARGE] = false;	//DLP
// 		m_histogramKept[CLSIZE_CHARGE] = false;		//DLP
// 		m_histogramFilled[CLUSTER_POSITION] = false;//DLP
// 		m_histogramKept[CLUSTER_POSITION] = false;	//DLP
// 		m_histogramFilled[CLUSTER_SIZE] = true;
// 		m_histogramKept[CLUSTER_SIZE] = true;
// 		m_histogramFilled[CLSIZE_TOT] = true;
// 		m_histogramKept[CLSIZE_TOT] = true;
	} else if (presetName== FE_ST_SOURCE_SCAN){
		m_repetitions = 1000;
		m_maskStageSteps = 1;
		m_strobeLVL1Delay = 26;
		m_LVL1Latency = 236;
		m_lowerLVL1DelayIfSlaveBy = 2;
		m_TLUTriggerDataDelay = 10;
		m_digitalInjection = false;
		m_loopActive[0] = false;
		m_isSourceScan = true;
		m_maskStageMode = STATIC;
		m_maskStageTotalSteps = STEPS_USER;
		m_srcTriggerType = FE_SELFTRIGGER;
		m_srcCountType = COUNT_DR;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[TOT] = true;
		m_histogramKept[TOT] = true;
		m_histogramFilled[LVL1] = true;
		m_histogramKept[LVL1] = true;
// 		m_histogramFilled[CLUSTER_TOT] = true;
// 		m_histogramKept[CLUSTER_TOT] = true;
// 		m_histogramFilled[CLUSTER_CHARGE] = false;
// 		m_histogramKept[CLUSTER_CHARGE] = false;
// 		m_histogramFilled[CLSIZE_CHARGE] = false;
// 		m_histogramKept[CLSIZE_CHARGE] = false;
// 		m_histogramFilled[CLUSTER_POSITION] = false;
// 		m_histogramKept[CLUSTER_POSITION] = false;
// 		m_histogramFilled[CLUSTER_SIZE] = true;
// 		m_histogramKept[CLUSTER_SIZE] = true;
// 		m_histogramFilled[CLSIZE_TOT] = true;
// 		m_histogramKept[CLSIZE_TOT] = true;
	} else if(presetName== NOISE_OCC){
		m_repetitions = 10000000;
		m_maskStageSteps = 1;
		m_strobeLVL1Delay = 60;
		m_strobeDuration = 0;
		m_strobeFrequency = 13333;// yields 3kHz trigger rate
		m_lowerLVL1DelayIfSlaveBy = 2;
		m_TLUTriggerDataDelay = 10;
		m_digitalInjection = false;
		m_loopActive[0] = false;
		m_isSourceScan = true;
		m_maskStageTotalSteps = STEPS_USER;
		m_maskStageMode = STATIC;
		m_srcTriggerType = STROBE_SCAN;
		m_srcCountType = COUNT_DH;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[MEAN_NOCC] = true;
		m_histogramKept[MEAN_NOCC] = true;
		m_histogramFilled[NOCC] = true;
		m_histogramKept[NOCC] = true;
		m_histogramFilled[NUM_NOISY_PIXELS] = true;
		m_histogramKept[NUM_NOISY_PIXELS] = true;
		m_loopAction[0] = CALC_MEAN_NOCC;
		m_dspLoopAction[0] = false;
		m_dspLoopAction[1] = false;
		m_dspLoopAction[2] = false;
	} else if (presetName == TOT_VERIF) {
		m_repetitions = 200;
		m_strobeDuration = 500;
		m_strobeFrequency = 4000;
		m_digitalInjection = false;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_maskStageTotalSteps = STEPS_3;
		m_maskStageSteps = 3;
		m_loopActive[0] = true;
		m_dspProcessing[0] = false;
		m_dspLoopAction[0] = false;
		setLoopVarValues(0, 0, 0, 1);
		m_loopParam[0] = NO_PAR;
		m_loopAction[0] = FDAC_TUNING;
		m_histogramFilled[OCCUPANCY] = false;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[TOT_MEAN] = true;
		m_histogramKept[TOT_MEAN] = true;
		m_histogramFilled[TOT_SIGMA] = true;
		m_histogramKept[TOT_SIGMA] = true;
		m_totTargetValue = 6;
		m_totTargetCharge = 20000;
		m_restoreModuleConfig = true;
		m_feVCal = 0x1fff;
	} else if(presetName == HITOR_SCAN){
		m_repetitions = 10;
		m_strobeDuration = 50;
		m_strobeLVL1Delay = 26;
		m_srcTriggerType = STROBE_USBPIX_SELF_TRG;
		m_maskStageMode = ENA_HITB;
		m_digitalInjection = true;
		m_loopActive[0] = true;
		m_dspProcessing[0] = false;
		m_dspLoopAction[0] = false;
		m_loopParam[0] = FEI4_GR;
		m_loopFEI4GR[0] = "Colpr_Addr";
		setLoopVarValues(0, 0, 39, 40);
		m_loopAction[0] = OCC_SUM;
		m_maskStageSteps = 672;
		m_maskStageTotalSteps = STEPS_26880;
		m_dspMaskStaging = true;
		m_loopOverDcs = false;
		m_avoidSpecialsCols = false;
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = false;
		m_histogramFilled[SUM_OCC] = true;
		m_histogramKept[SUM_OCC] = true;
	} else if (presetName == THR_FAST_SCAN) {
		m_strobeFrequency = 4000;
		m_singleDCloop = true;
		m_avoidSpecialsCols = false;
		m_digitalInjection = false;
		m_maskStageSteps = 3;
		m_maskStageTotalSteps = STEPS_3;
		m_strobeDuration = 500;
		m_feVCal = 0x1fff;              
		m_repetitions = 100;
		m_innerLoopSwap = true;  
		m_digitalInjection = false;
		
		m_loopActive[0] = true;
		m_loopParam[0] = VCAL;
		std::vector<float> vcalSteps;
		vcalSteps.push_back(512);
		vcalSteps.push_back(256);
		vcalSteps.push_back(128);
		vcalSteps.push_back(64);
		vcalSteps.push_back(32);
		vcalSteps.push_back(16);
		vcalSteps.push_back(8);
		vcalSteps.push_back(4);
		vcalSteps.push_back(2);
		vcalSteps.push_back(1);
		vcalSteps.push_back(1);
		setLoopVarValues(0,vcalSteps);
		m_loopAction[0] = THR_FAST_SCANNING;
		m_dspProcessing[0] = false;
		m_dspLoopAction[0] = false;
		
		m_histogramFilled[OCCUPANCY] = true;
		m_histogramKept[OCCUPANCY] = true;
		m_histogramFilled[SCURVE_MEAN] = true;
		m_histogramKept[SCURVE_MEAN] = true;
		m_histogramFilled[SCURVE_SIGMA] = true;
		
		m_restoreModuleConfig = true;
		
	} else {
		throw PixScanExc(PixControllerExc::ERROR, "Undefined scan preset");
	}
}

void PixScan::initConfig() {

	// ********* JW: This is OK for the USB system **************
	//  m_isSourceScan = false;
	m_isTestBeam = false;
	m_lowerLVL1DelayIfSlaveBy = 0;

	// Initialize histogramTypes and dspHistogramTypes
	m_dspHistogramTypes["OCCUPANCY"] = OCCUPANCY;
	m_dspHistogramTypes["TOT_MEAN"] = TOT_MEAN;
	m_dspHistogramTypes["TOT_SIGMA"] = TOT_SIGMA;
	m_dspHistogramTypes["LVL1"] = LVL1;
	m_dspHistogramTypes["LV1ID"] = LV1ID;
	m_dspHistogramTypes["BCID"] = BCID;
	m_dspHistogramTypes["HITOCC"] = HITOCC;
	m_dspHistogramTypes["TOTAVERAGE"] = TOTAVERAGE;
	m_dspHistogramTypes["CLUSTER_TOT"] = CLUSTER_TOT;
	m_dspHistogramTypes["CLUSTER_CHARGE"] = CLUSTER_CHARGE;	//DLP
	m_dspHistogramTypes["CLUSTER_POSITION"] = CLUSTER_POSITION;	//DLP
	m_dspHistogramTypes["CLUSTER_SIZE"] = CLUSTER_SIZE;
	m_dspHistogramTypes["CLSIZE_TOT"] = CLSIZE_TOT;
	m_dspHistogramTypes["CLSIZE_CHARGE"] = CLSIZE_CHARGE;
	m_dspHistogramTypes["SEED_TOT"] = SEED_TOT;
	m_dspHistogramTypes["SEED_LVL1"] = SEED_LVL1;
	m_dspHistogramTypes["NSEEDS"] = NSEEDS;
	m_dspHistogramTypes["TOT"] = TOT;
	m_dspHistogramTypes["TOT0"] = TOT0;
	m_dspHistogramTypes["TOT1"] = TOT1;
	m_dspHistogramTypes["TOT2"] = TOT2;
	m_dspHistogramTypes["TOT3"] = TOT3;
	m_dspHistogramTypes["TOT4"] = TOT4;
	m_dspHistogramTypes["TOT5"] = TOT5;
	m_dspHistogramTypes["TOT6"] = TOT6;
	m_dspHistogramTypes["TOT7"] = TOT7;
	m_dspHistogramTypes["TOT8"] = TOT8;
	m_dspHistogramTypes["TOT9"] = TOT9;
	m_dspHistogramTypes["TOT10"] = TOT10;
	m_dspHistogramTypes["TOT11"] = TOT11;
	m_dspHistogramTypes["TOT12"] = TOT12;
	m_dspHistogramTypes["TOT13"] = TOT13;
	m_dspHistogramTypes["TOT14"] = TOT14;
	m_dspHistogramTypes["TOT15"] = TOT15;
	m_histogramTypes["OCCUPANCY"] = OCCUPANCY;
	m_histogramTypes["TOT"] = TOT;
	m_histogramTypes["TOT0"] = TOT0;
	m_histogramTypes["TOT1"] = TOT1;
	m_histogramTypes["TOT2"] = TOT2;
	m_histogramTypes["TOT3"] = TOT3;
	m_histogramTypes["TOT4"] = TOT4;
	m_histogramTypes["TOT5"] = TOT5;
	m_histogramTypes["TOT6"] = TOT6;
	m_histogramTypes["TOT7"] = TOT7;
	m_histogramTypes["TOT8"] = TOT8;
	m_histogramTypes["TOT9"] = TOT9;
	m_histogramTypes["TOT10"] = TOT10;
	m_histogramTypes["TOT11"] = TOT11;
	m_histogramTypes["TOT12"] = TOT12;
	m_histogramTypes["TOT13"] = TOT13;
	m_histogramTypes["TOT14"] = TOT14;
	m_histogramTypes["TOT15"] = TOT15;
	m_histogramTypes["TOT_MEAN"] = TOT_MEAN;
	m_histogramTypes["TOT_SIGMA"] = TOT_SIGMA;
	m_histogramTypes["CHARGE_MEAN"] = CHARGE_MEAN;
	m_histogramTypes["CHARGE_SIGMA"] = CHARGE_SIGMA;
	m_histogramTypes["CHARGE_RECO_ERROR"] = CHARGE_RECO_ERROR;
	m_histogramTypes["LVL1"] = LVL1;
	m_histogramTypes["LV1ID"] = LV1ID;
	m_histogramTypes["BCID"] = BCID;
	m_histogramTypes["SCURVE_MEAN"] = SCURVE_MEAN;
	m_histogramTypes["SCURVE_SIGMA"] = SCURVE_SIGMA;
	m_histogramTypes["SCURVE_CHI2"] = SCURVE_CHI2;
	m_histogramTypes["TDAC_T"] = TDAC_T;
	m_histogramTypes["TDAC_THR"] = TDAC_THR;
	m_histogramTypes["TDAC_OCC"] = TDAC_OCC;
	m_histogramTypes["GDAC_OCC"] = GDAC_OCC;
	m_histogramTypes["SUM_OCC"] = SUM_OCC;
	m_histogramTypes["GDAC_T"] = GDAC_T;
	m_histogramTypes["GDAC_THR"] = GDAC_THR;
	m_histogramTypes["FDAC_T"] = FDAC_T;
	m_histogramTypes["FDAC_TOT"] = FDAC_TOT;
	m_histogramTypes["IF_T"] = IF_T;
	m_histogramTypes["IF_TOT"] = IF_TOT;
	m_histogramTypes["TIMEWALK"] = TIMEWALK;
	m_histogramTypes["RAW_DATA_REF"] = RAW_DATA_REF;
	m_histogramTypes["RAW_DATA_0"] = RAW_DATA_0;
	m_histogramTypes["RAW_DATA_1"] = RAW_DATA_1;
	m_histogramTypes["RAW_DATA_DIFF_1"] = RAW_DATA_DIFF_1;
	m_histogramTypes["RAW_DATA_DIFF_2"] = RAW_DATA_DIFF_2;
	m_histogramTypes["HITOCC"] = HITOCC;
	m_histogramTypes["TOTAVERAGE"] = TOTAVERAGE;
	m_histogramTypes["TOTCAL_PARA"] = TOTCAL_PARA;
	m_histogramTypes["TOTCAL_PARB"] = TOTCAL_PARB;
	m_histogramTypes["TOTCAL_PARC"] = TOTCAL_PARC;
	m_histogramTypes["TOTCAL_CHI2"] = TOTCAL_CHI2;
	m_histogramTypes["FE_ERRORS"] = DSP_ERRORS;
	m_histogramTypes["DCS_DATA"] = DCS_DATA;
	m_histogramTypes["MON_LEAK"] = MON_LEAK;
	m_histogramTypes["HB_SCALER_CNT"] = HB_SCALER_CNT;
	m_histogramTypes["CLUSTER_TOT"] = CLUSTER_TOT;
	m_histogramTypes["CLUSTER_CHARGE"] = CLUSTER_CHARGE;	//DLP
	m_histogramTypes["CLUSTER_POSITION"] = CLUSTER_POSITION;	//DLP
	m_histogramTypes["CLUSTER_SIZE"] = CLUSTER_SIZE;
	m_histogramTypes["CLSIZE_TOT"] = CLSIZE_TOT;
	m_histogramTypes["CLSIZE_CHARGE"] = CLSIZE_CHARGE;//DLP
	m_histogramTypes["SEED_TOT"] = SEED_TOT;
	m_histogramTypes["SEED_LVL1"] = SEED_LVL1;
	m_histogramTypes["NSEEDS"] = NSEEDS;
	m_histogramTypes["FIT_PAR0"] = FIT_PAR0;
	m_histogramTypes["FIT_PAR1"] = FIT_PAR1;
	m_histogramTypes["FIT_PAR2"] = FIT_PAR2;
	m_histogramTypes["FIT_PAR3"] = FIT_PAR3;
	m_histogramTypes["FIT_PAR4"] = FIT_PAR4;
	m_histogramTypes["FIT_CHI2"] = FIT_CHI2;
	m_histogramTypes["HIT_RATE"] = HIT_RATE;
	m_histogramTypes["TRG_RATE"] = TRG_RATE;
	m_histogramTypes["MEAN_NOCC"] = MEAN_NOCC;
	m_histogramTypes["NOCC"] = NOCC;
	m_histogramTypes["NUM_NOISY_PIXELS"] = NUM_NOISY_PIXELS;
	m_histogramTypes["DISCBIAS_T"] = DISCBIAS_T;
	m_histogramTypes["DISCBIAS_T"] = DISCBIAS_T;
	m_histogramTypes["DISCBIAS_TIMEWALK"] = DISCBIAS_TIMEWALK;
	m_histogramTypes["DISCBIAS_TIMEWALK"] = DISCBIAS_TIMEWALK;
	// Initialize scanTypes
	m_scanTypes["DIGITAL_TEST"] = DIGITAL_TEST;
	m_scanTypes["ANALOG_TEST"] = ANALOG_TEST;
	m_scanTypes["THRESHOLD_SCAN"] = THRESHOLD_SCAN;
	m_scanTypes["TOT_CALIB"] = TOT_CALIB;// NB: for FE-I4, this is what used to be TOT_CALIB_ALT
	m_scanTypes["TOT_CALIB_LUT"] = TOT_CALIB_LUT;
	m_scanTypes["TDAC_TUNE"] = TDAC_TUNE;
	m_scanTypes["TDAC_FAST_TUNE"] = TDAC_FAST_TUNE;
	m_scanTypes["GDAC_TUNE"] = GDAC_TUNE;
	m_scanTypes["GDAC_FAST_TUNE"] = GDAC_FAST_TUNE;
	m_scanTypes["VTHIN_FAST_TUNE"] = GDAC_FAST_TUNE2;
	m_scanTypes["FDAC_TUNE"] = FDAC_TUNE;
	m_scanTypes["IF_TUNE"] = IF_TUNE;
	m_scanTypes["TIMEWALK_MEASURE"] = TIMEWALK_MEASURE;
	// no longer needed for FE-I4
	//	m_scanTypes["RX_DELAY_SCAN"] = BOC_RX_DELAY_SCAN;
	m_scanTypes["INTIME_THRESH_SCAN"] = INTIME_THRESH_SCAN;
	m_scanTypes["T0_SCAN"] = T0_SCAN;
	m_scanTypes["DISCBIAS_TUNE"] = DISCBIAS_TUNE;
	m_scanTypes["CROSSTALK_SCAN"] = CROSSTALK_SCAN;
	m_scanTypes["IV_SCAN"] = IV_SCAN;
	m_scanTypes["DAC_SCAN"] = DAC_SCAN;
	m_scanTypes["MONLEAK"] = MONLEAK;
	m_scanTypes["SOURCE_SCAN"] = SOURCE_SCAN;
	m_scanTypes["FE_ST_SOURCE_SCAN"] = FE_ST_SOURCE_SCAN;
	m_scanTypes["NOISE_OCC"] = NOISE_OCC;
	m_scanTypes["INJ_CALIB"] = INJ_CALIB;
#ifdef WITH_EUDAQ
	m_scanTypes["TESTBEAM_EUDAQ"] = EUDAQ;
#endif
	m_scanTypes["TOT_VERIF"] = TOT_VERIF;
	m_scanTypes["CHARGE_CALIB_VERIF"] = CHARGE_CALIB_VERIF;
	m_scanTypes["STUCK_PIXEL"] = HITBUS_SCALER;
	m_scanTypes["HITOR_SCAN"] = HITOR_SCAN;
	m_scanTypes["THR_FAST_SCAN"] = THR_FAST_SCAN;

	// **********************************************************

	// Create the Config object
	m_conf = new Config("ScanConfig");
	Config &conf = *m_conf;

	// Insert the configuration parameters in the Config object
	std::map<std::string, int> tf;
	tf["FALSE"] = 0;
	tf["TRUE"] = 1;

	// Group general
	conf.addGroup("general");

	std::map<std::string, int> configMap;
	configMap["PHYSICS"] = PHYSICS;
	configMap["CALIB"] = CALIB;
	configMap["SPARE"] = SPARE;
	conf["general"].addList("modConfig", (int &)m_modConfig, PHYSICS, configMap,
		"Configuration set to use", true);
	conf["general"].addBool("modScanConcurrent", m_modScanConcurrent, true,
		"Scan concurrent or FE by FE", true);
	conf["general"].addInt("repetitions", m_repetitions, 200,
		"# of events per bin", true);
	std::map<std::string, int> countMap;
	countMap["trigger"]     = COUNT_TRIGGER;
	countMap["data header"] = COUNT_DH;
	countMap["data record"] = COUNT_DR;
	countMap["seconds"] = COUNT_SECS;
	conf["general"].addList("srcCountType", m_srcCountType, COUNT_TRIGGER, countMap,
		"type of events to count for source scans", true);
	std::map<std::string, int> triggerMap;
	triggerMap["strobe scan"]                = STROBE_SCAN;
	triggerMap["USBpix Self Trigger"]        = USBPIX_SELF_TRG;
	triggerMap["ext. trigger"]               = EXT_TRG;
	triggerMap["TLU Simple Handshake"]       = TLU_SIMPLE;
	triggerMap["TLU Trigger Data Handshake"] = TLU_DATA_HANDSHAKE;
	triggerMap["strobe + ext.trg."]          = STROBE_EXTTRG;
	triggerMap["FE Self Trigger"]            = FE_SELFTRIGGER;
	triggerMap["strobe + USBpix selftrg."]   = STROBE_USBPIX_SELF_TRG;
	triggerMap["strobe + FE selftrg."]       = STROBE_FE_SELF_TRG;
	conf["general"].addList("srcTriggerType", m_srcTriggerType, STROBE_SCAN, triggerMap,
		"type of trigger for source scans", true);

	std::map<std::string, int> stageStepsMap;
	stageStepsMap["USER"] = STEPS_USER;
	// FE-I4 masks
	stageStepsMap["STEPS_1"] = STEPS_1;
	stageStepsMap["STEPS_2"] = STEPS_2;
	stageStepsMap["STEPS_3"] = STEPS_3;
	stageStepsMap["STEPS_4"] = STEPS_4;
	stageStepsMap["STEPS_6"] = STEPS_6;
	stageStepsMap["STEPS_8"] = STEPS_8;
	stageStepsMap["STEPS_336"] = STEPS_336;
	stageStepsMap["STEPS_672"] = STEPS_672;
	stageStepsMap["STEPS_26880"] = STEPS_26880;
	// FE-I3 masks
	stageStepsMap["STEPS_32"] = STEPS_32;
	stageStepsMap["STEPS_40"] = STEPS_40;
	stageStepsMap["STEPS_64"] = STEPS_64;
	stageStepsMap["STEPS_80"] = STEPS_80;
	stageStepsMap["STEPS_160"] = STEPS_160;
	stageStepsMap["STEPS_320"] = STEPS_320;
	stageStepsMap["STEPS_2880"] = STEPS_2880;
	conf["general"].addList("maskStageTotalSteps", (int &)m_maskStageTotalSteps, STEPS_6, stageStepsMap,
		"Total number of mask stages", true);
	conf["general"].addInt("maskStageSteps", m_maskStageSteps, 6,
		"Number of mask stages to actually execute in scan", true);
	conf["general"].addInt("maskStageStepWidth", m_maskStageStepWidth, 1,
		"Width of mask stage steps", true);

	std::map<std::string, int> maskstgmodeMap;
	maskstgmodeMap["STATIC"] = STATIC;
	maskstgmodeMap["SEL_ENA"] = SEL_ENA;
	maskstgmodeMap["SEL"] = SEL;
	maskstgmodeMap["XTALK"] = XTALK;
	maskstgmodeMap["ENA"] = ENA;
	maskstgmodeMap["HITBUS"] = HITB;
	maskstgmodeMap["ILEAK"] = HITB_INV;
	maskstgmodeMap["ENA_HITB"] = ENA_HITB;
	maskstgmodeMap["SEL_ENA_HITB"] = SEL_ENA_HITB;
	maskstgmodeMap["DIGINJ"] = DINJ;
	conf["general"].addList("maskStageMode", (int &)m_maskStageMode, SEL_ENA, maskstgmodeMap,
		"Mask staging option i.e. which ctrl bits are staged", true);
	conf["general"].addBool("restoreModuleConfig", m_restoreModuleConfig, true,
		"Original module configuration is restored at the end of the scan", true);
	std::map<std::string, int> runType;
	runType["NORMAL_SCAN"] = NORMAL_SCAN;
	runType["RAW_PATTERN"] = RAW_PATTERN;
	runType["RAW_EVENT"] = RAW_EVENT;
	conf["general"].addList("runType", (int &)m_runType, NORMAL_SCAN, runType,
		"Type of run to be executed", true);
	std::vector<unsigned int> seedsDef;
	for (unsigned int ix=0; ix<32; ix++) {
		seedsDef.push_back(ix+(ix<<5)+(ix<<10)+(ix<<15)+(ix<<20));
	}
	conf["general"].addVector("patternSeeds", m_patternSeeds, seedsDef,
		"seeds for the MCC pattern generation", true);

	//JW: source scan
	conf["general"].addBool("sourceScanFlag", m_isSourceScan, false, "Source scan flag", true);
	conf["general"].addString("sourceRawFile",m_sourceRawFile, "", "Raw data output file name", true);

	//SS: Testbeam
	conf["general"].addBool("testbeamFlag", m_isTestBeam, false, "TestBeam flag", true);
	conf["general"].addBool("skipSourceScanHistos", m_skipSourceScanHistos, false, "Skip Source Scan Histogramming", true);
	//TB: Testbeam
	conf["general"].addInt("tbSRAMReadoutAt", m_tb_sram_readout_at, 100, "SRAM readout value for testbeam operation", true);

	// Group histograms
	conf.addGroup("loops");

	std::map<std::string, int> scanvarMap;
	scanvarMap["NO_PAR"] = NO_PAR;
	scanvarMap["TDACS"] = TDACS;
	scanvarMap["FDACS"] = FDACS;
	scanvarMap["GDAC"] = GDAC;
	scanvarMap["DISCBIAS"] = DISCBIAS;
	scanvarMap["STRB_TRIG_FREQ"] = STRBFREQ;
	scanvarMap["IF"] = IF;
	scanvarMap["VCAL"] = VCAL;
	scanvarMap["CAPSEL"] = CAPSEL;
	//scanvarMap["FEI3_GR"] = FEI3_GR;
	//scanvarMap["CAPMEAS"] = CAPMEAS;
	scanvarMap["FEI4_GR"] = FEI4_GR;
	scanvarMap["TRIGGER_DELAY"] = TRIGGER_DELAY;
	scanvarMap["LATENCY"] = LATENCY;
	scanvarMap["INCR_LAT_TRGDEL"] = INCR_LAT_TRGDEL;
	scanvarMap["STROBE_DURATION"] = STROBE_DURATION;
	scanvarMap["STROBE_DELAY"] = STROBE_DELAY;
	scanvarMap["STROBE_FINE_DELAY"] = STROBE_FINE_DELAY;
	scanvarMap["AUXFREQ"] = AUXFREQ;
	scanvarMap["IREF_PAD"] = IREF_PAD;
	scanvarMap["IOMUX_IN"] = IOMUX_IN;
	scanvarMap["DCS_VOLTAGE"] = DCS_VOLTAGE;
	scanvarMap["DCS_PAR1"] = DCS_PAR1;
	scanvarMap["DCS_PAR2"] = DCS_PAR2;
	scanvarMap["DCS_PAR3"] = DCS_PAR3;
	//scanvarMap["RX_DELAY"] = BOC_RX_DELAY;
	std::map<std::string, int> endactMap;
	endactMap["NONE"] = NO_ACTION;
	endactMap["SCURVE_FIT"] = SCURVE_FIT;
	endactMap["SCURVE_FAST"] = SCURVE_FAST;
	endactMap["TDAC_TUNING"] = TDAC_TUNING;
	endactMap["TDAC_FAST_TUNING"] = TDAC_FAST_TUNING;
	endactMap["GDAC_FAST_TUNING"] = GDAC_FAST_TUNING;
	endactMap["OCC_SUM"] = OCC_SUM;
	endactMap["GDAC_TUNING"] = GDAC_TUNING;
	endactMap["FDAC_TUNING"] = FDAC_TUNING;
	endactMap["FDAC_TUNING_ALT"] = FDAC_TUNING_ALT;
	endactMap["IF_TUNING"] = IF_TUNING;
	//endactMap["IF_FAST_TUNING"] = IF_FAST_TUNING;
	endactMap["T0_SET"] = T0_SET;
	endactMap["DISCBIAS_TUNING"] = DISCBIAS_TUNING;
	endactMap["DELAY_FIT"] = MCCDEL_FIT;
	endactMap["TOTCAL_FIT"] = TOTCAL_FIT;
	endactMap["OFFSET_CALIB"] = OFFSET_CALIB;
	endactMap["TOTCAL_FEI4"] = TOTCAL_FEI4;
	endactMap["TOT_CHARGE_LUT"] = TOT_CHARGE_LUT;
	endactMap["TOT_TO_CHARGE"] = TOT_TO_CHARGE;
	endactMap["CALC_MEAN_NOCC"] = CALC_MEAN_NOCC;
	endactMap["CLEAR_IOMUX_BITS"] = CLEAR_IOMUX_BITS;
	endactMap["THR_FAST_SCANNING"] = THR_FAST_SCANNING;
	conf["loops"].addBool("dspMaskStaging", m_dspMaskStaging, true,
		"Mask staging executed by the DSP", true);
	conf["loops"].addBool("innerLoopSwap", m_innerLoopSwap, false,
		"Mask staging is executed before Loop 0", true);
	conf["loops"].addBool("loopOverDCs", m_loopOverDcs, true,
		"Automatically loop over DC's while scanning", true);
	conf["loops"].addBool("avoidSpecialCols", m_avoidSpecialsCols, true,
		"Special loop mode avoiding simultaneous injection into problematic col's 0, 77, 78, 79", true);
	conf["loops"].addBool("singleDCloop", m_singleDCloop, false,
		"Special loop mode over single DCs for injection", true);
	conf["loops"].addBool("configAtStart", m_configAtStart, true,
		"Configure all modules before starting the scan", true);

	for (unsigned int ll=0; ll<MAX_LOOPS; ll++) {
		std::ostringstream lnum;
		lnum << "Loop_" << ll;
		conf["loops"].addBool("active"+lnum.str(), m_loopActive[ll], false,
			"Loop activation", true);
		conf["loops"].addBool("dspProcessing"+lnum.str(), m_dspProcessing[ll], false,
			"Loop executed by the DSP", true);
		conf["loops"].addList("param"+lnum.str(), (int &)m_loopParam[ll], NO_PAR, scanvarMap,
			"Loop varaible", true);
		std::vector<float> defVal;
		conf["loops"].addVector("loopVarValues"+lnum.str(), m_loopVarValues[ll], defVal,
			"Loop variable values", true);
		conf["loops"].addInt("loopVarNSteps"+lnum.str(), m_loopVarNSteps[ll], 0,
			"Loop varaible step", true);
		conf["loops"].addFloat("loopVarMin"+lnum.str(), m_loopVarMin[ll], 0,
			"Loop varaible min value", true);
		conf["loops"].addFloat("loopVarMax"+lnum.str(), m_loopVarMax[ll], 0,
			"Loop varaible max value", true);
		conf["loops"].addBool("loopVarUniform"+lnum.str(), m_loopVarUniform[ll], true,
			"Equally spaced loop variable points", true);
		conf["loops"].addBool("loopVarValuesFree"+lnum.str(), m_loopVarValuesFree[ll], false,
			"Loop var valued dynamically calculated", true);
		conf["loops"].addList("endAction"+lnum.str(), (int &)m_loopAction[ll], NO_ACTION, endactMap,
			"End loop action", true);
		conf["loops"].addBool("dspAction"+lnum.str(), m_dspLoopAction[ll], true,
			"End loop action executed by the DSP", true);
		conf["loops"].addString("feGlobRegName"+lnum.str(), m_loopFEI4GR[ll], "PlsrDAC",
			"Name of FE-I4 register to be scanned", true);
	}

	// Scan specific parameters
	conf.addGroup("scans");

	conf["scans"].addInt("thresholdTargedValue", m_thresholdTargetValue, 3000,
		"Threshold target value for TDAC tuning", true);
	conf["scans"].addInt("totTargedValue", m_totTargetValue, 5,
		"TOT target value for FDAC tuning", true);
	conf["scans"].addInt("totTargedCharge", m_totTargetCharge, 20000,
		"Reference charge for FDAC tuning", true);
	conf["scans"].addBool("useMaskForGlobal", m_useMaskForGlobal, false,
		"Mask analysed quantity with FE enable mask for global tunings (avoid e.g. noisy pixels for avg.)", true);
	conf["scans"].addFloat("minThrNoiseCutNormal", m_minThrNoiseCutNormal, 999999.,
		"Noise cut value for normal pixels to stop minimal threshold scan", true);
	conf["scans"].addFloat("minThrNoiseCutLong", m_minThrNoiseCutLong, 999999.,
		"Noise cut value for long pixels to stop minimal threshold scan", true);
	conf["scans"].addFloat("minThrNoiseCutGanged", m_minThrNoiseCutGanged, 999999.,
		"Noise cut value for ganged pixels to stop minimal threshold scan", true);
	conf["scans"].addFloat("minThrNoiseCutInterGanged", m_minThrNoiseCutInterGanged, 999999.,
		"Noise cut value for inter-ganged pixels to stop minimal threshold scan", true);
	conf["scans"].addFloat("minThrChi2Cut", m_minThrChi2Cut, 50.,
		"Noise cut value for inter-ganged pixels to stop minimal threshold scan", true);
	conf["scans"].addFloat("chiCut", m_chicut, 100000.,
		"Chi^2 cut for considering a S-curve fit as bad", true);
	conf["scans"].addFloat("noiseOccCut", m_noiseOccCut, 0.00001f,
		"Cut on NOcc above which pixels are defined noisy", true);
	conf["scans"].addInt("nbadChiCut", m_nbadchicut, 100000,
		"Number of bad chi^2 values in S-curve scans after which a re-fit with MINUIT is started", true);

	// Group histograms
	conf.addGroup("histograms");

	for (std::map<std::string, int>::iterator itt = m_histogramTypes.begin();
		itt != m_histogramTypes.end(); ++itt)                                 {
			std::string name = (*itt).first;
			int id = (*itt).second;
			if (name == "OCCUPANCY") {
				conf["histograms"].addBool("histogramFilled"+name, m_histogramFilled[id], true,
					"Fill histogram "+name, true);
				conf["histograms"].addBool("histogramKept"+name, m_histogramKept[id], true,
					"Keep histogram "+name, true);
			} else {
				conf["histograms"].addBool("histogramFilled"+name, m_histogramFilled[id], false,
					"Fill histogram "+name, true);
				conf["histograms"].addBool("histogramKept"+name, m_histogramKept[id], false,
					"Keep histogram "+name, true);
			}
	}
	conf["histograms"].addInt("maxColClusterDist", m_maxColClusterDist, 1,
		"max. column cluster distance for source scan clustering", true);
	conf["histograms"].addInt("maxRowClusterDist", m_maxRowClusterDist, 4,
		"max. row cluster distance for source scan clustering", true);
	conf["histograms"].addInt("maxClusterBcidDepth", m_maxClusterBcidDepth, 2,
		"max. cluster BCID depth for source scan clustering", true);
	conf["histograms"].addInt("maxClusterHitTot", m_maxClusterHitTot, 13,
		"max TOT of hit, otherwise cluster omitted", true);
	conf["histograms"].addInt("minClusterHits", m_minClusterHits, 1,
		"min cluster hits, otherwise cluster omitted", true);
	conf["histograms"].addInt("maxClusterHits", m_maxClusterHits, 50,
		"max cluster hits, otherwise cluster omitted", true);
	conf["histograms"].addInt("maxEventIncomplete", m_maxEventIncomplete, 50,
		"max number of incomplete event data per SRAM readout, otherwise clustering aborted", true);
	conf["histograms"].addInt("maxEventErrors", m_maxEventErrors, 2,
		"max number of invalid event data per SRAM readout, otherwise clustering aborted", true);
	conf["histograms"].addInt("getNptsRateAvg", m_getNptsRateAvg, 10, 
			     "No. values taken for average of rate measurement", true);

	// Group trigger
	conf.addGroup("trigger");

	conf["trigger"].addBool("self", m_selfTrigger, 0,
		"Selects module self-triggering mode", true);
	conf["trigger"].addInt("LVL1Latency", m_LVL1Latency, 210,
		"8-bit trigger latency as programmed on FEs", true);
	conf["trigger"].addBool("strobeLVL1DelayOverride", m_strobeLVL1DelayOverride, true,
		"Override ModuleGroup trigger delay", true);
	conf["trigger"].addInt("strobeLVL1Delay", m_strobeLVL1Delay, 50,
		"Sets the delay (in BCO units) between strover and LVL1", true);
	conf["trigger"].addInt("strobeDuration", m_strobeDuration, 50,
		"Length of strobe in BCO units", true);
	conf["trigger"].addInt("strobeFineDelay", m_strobeFineDelay, 0,
		"Strobe fine delay in finest available steps.", true);
	conf["trigger"].addInt("strobeFrequency", m_strobeFrequency, 8000,
		"Frequency of strobes", true);
	conf["trigger"].addInt("strobeMCCDelay", m_strobeMCCDelay, 2,
		"Strobe delay setting", true);
	conf["trigger"].addInt("strobeMCCDelayRange", m_strobeMCCDelayRange, 0,
		"Strobe delay range", true);
	conf["trigger"].addInt("superGroupTrigDelay", m_superGroupTrigDelay, -1,
		"Trigger delay between supergroups 0 and 1 (-1 means independent)", true);
	conf["trigger"].addInt("consecutiveLevl1TrigA_0", m_consecutiveLvl1TrigA[0], 16,
		"Number of LVL1 in block A for supergroup 0", true);
	conf["trigger"].addInt("consecutiveLevl1TrigB_0", m_consecutiveLvl1TrigB[0], 16,
		"Number of LVL1 in block B for supergroup 0", true);
	conf["trigger"].addInt("consecutiveLevl1TrigA_1", m_consecutiveLvl1TrigA[1], 0,
		"Number of LVL1 in block A for supergroup 1", true);
	conf["trigger"].addInt("consecutiveLevl1TrigB_1", m_consecutiveLvl1TrigB[1], 0,
		"Number of LVL1 in block B for supergroup 1", true);
	conf["trigger"].addInt("trigABDelay_0", m_trigABDelay[0], 0,
		"Delay between trigger blocks A and B for supergroup 0", true);
	conf["trigger"].addInt("trigABDelay_1", m_trigABDelay[1], 0,
		"Delay between trigger blocks A and B for supergroup 1", true);
	conf["trigger"].addBool("lvl1HistoBinned", m_lvl1HistoBinned, false,
		"LVL1 histograms are binned", true);
	conf["trigger"].addInt("ReduceLVL1DelayIfReplcationSlaveBy", m_lowerLVL1DelayIfSlaveBy, 0, "Reduce LVL1 Delay by the value if Trigger Mode is Replication Slave", true);
	conf["trigger"].addInt("TLUTriggerDataDelay", m_TLUTriggerDataDelay, 10,
		"Delay before trigger number is expected back from TLU", true);
	// Group reset
	conf.addGroup("reset");
	conf["reset"].addBool("sendHardReset", m_sendHardReset, false, "Send a hardware reset pulse to FEs after finishing innermost loop", true);
	conf["reset"].addBool("sendSoftReset", m_sendSoftReset, false, "Send a software reset command to FEs after finishing innermost loop", true);
	conf["reset"].addBool("sendBCR", m_sendBCR, true, "Send a BCR after finishing innermost loop", true);
	conf["reset"].addBool("sendECR", m_sendECR, true, "Send a ECR after finishing innermost loop", true);
	// special for stop mode scan
	conf["reset"].addBool("sendGlobalPulse", m_sendGlobalPulse, false, "Send global pulse after finishing innermost loop", true);
	conf["reset"].addInt("GlobalPulseLen", m_globalPulseLen, 1, "Length of global pulse to be sent", true);

	// Group fe
	conf.addGroup("fe");

	std::map<std::string, int> capMap;
	capMap["CAP1"] = 0;
	capMap["CAP0"] = 1;
	capMap["CAP0+CAP1"] = 2;
	conf["fe"].addList("chargeInjCap", m_chargeInjCap, 2, capMap,
		"Selects injection capacitor", true);
	conf["fe"].addBool("digitalInjection", m_digitalInjection, true,
		"Select digital or analog injection", true);
	conf["fe"].addInt("vCal", m_feVCal, 0,
		"Value for the VCAL DAC", true);
	conf["fe"].addBool("alterFeCfg", m_alterFeCfg, true,
		"Alter certain parts of FE cfg. when running non-src-scans", true);

	// Group modGroups
	conf.addGroup("modGroups");

	conf["modGroups"].addInt("moduleMask_0", m_moduleMask[0], 0xffffffff,
		"Modules in group 0", true);
	conf["modGroups"].addBool("configEnabled_0", m_configEnabled[0], true,
		"Config enable group 0", true);
	conf["modGroups"].addBool("strobeEnabled_0", m_strobeEnabled[0], true,
		"Strobe enable group 0", true);
	conf["modGroups"].addBool("triggerEnabled_0", m_triggerEnabled[0], true,
		"Trigger enable group 0", true);
	conf["modGroups"].addBool("readoutEnabled_0", m_readoutEnabled[0], true,
		"Readout enable group 0", true);
	for (unsigned int i=1; i<MAX_GROUPS; i++) {
		std::ostringstream gnum;
		gnum << i;
		std::string tit = "Threshold mod # "+gnum.str();
		conf["modGroups"].addInt("moduleMask_"+gnum.str(), m_moduleMask[i], 0x0,
			"Modules in group "+gnum.str(), true);
		conf["modGroups"].addBool("configEnabled_"+gnum.str(), m_configEnabled[i], false,
			"Config enable group "+gnum.str(), true);
		conf["modGroups"].addBool("strobeEnabled_"+gnum.str(), m_strobeEnabled[i], false,
			"Strobe enable group "+gnum.str(), true);
		conf["modGroups"].addBool("triggerEnabled_"+gnum.str(), m_triggerEnabled[i], false,
			"Trigger enable group "+gnum.str(), true);
		conf["modGroups"].addBool("readoutEnabled_"+gnum.str(), m_readoutEnabled[i], false,
			"Readout enable group "+gnum.str(), true);
	}

	// Group DCS Control
	conf.addGroup("dcsControl");

	std::map<std::string, int> vmodeMap;
	// added for DCS scan functionality
	std::map<std::string, int> dmodeMap;
	dmodeMap["VOLTAGE"] = VOLTAGE;
	dmodeMap["CURRENT"] = CURRENT;
	dmodeMap["FE_ADC"] = FE_ADC;
	conf["dcsControl"].addString("readDcsChan", m_readDcsChan, "",
				     "name of DCS channel read while scanning", true);
	conf["dcsControl"].addList("readDcsMode", (int&)m_readDcsMode, VOLTAGE, dmodeMap,
				   "voltage or current to be read from DCS channel", true);
	conf["dcsControl"].addString("scanDcsChan", m_scanDcsChan, "Vbias",
				     "name of DCS channel which controls scanned voltage", true);
	conf["dcsControl"].addBool("addModNameToDcs", m_addModNameToDcs, true, 
				   "Add _[module name] to DCS channel name for scanned/read voltage", true);

	conf["dcsControl"].addString("scanPar1DcsChan", m_scanPar1DcsChan, "DCS_channel name for par 1","name of DCS channel to set a parameter 1", true);
	conf["dcsControl"].addString("scanPar1DcsPar", m_scanPar1DcsPar, "parameter name (e.g. voltage, current, position)","name of DCS parameter 1 to change", true);
	conf["dcsControl"].addString("scanPar2DcsChan", m_scanPar2DcsChan, "DCS_channel name for par 2","name of DCS channel to set a parameter 2", true);
	conf["dcsControl"].addString("scanPar2DcsPar", m_scanPar2DcsPar, "voltage","name of DCS parameter 2 to change", true);
	conf["dcsControl"].addString("scanPar3DcsChan", m_scanPar3DcsChan, "DCS_channel name for par 3","name of DCS channel to set a parameter 3", true);
	conf["dcsControl"].addString("scanPar3DcsPar", m_scanPar3DcsPar, "voltage","name of DCS parameter 3 to change", true);
	// Select default values
	conf.reset();

}

//! Scan control

void PixScan::resetScan() {
	// Reset indexes
	for (int i=0; i<MAX_LOOPS; i++) {
		m_loopIndex[i] = 0;
		m_loopTerminating[i] = false;
		m_loopEnded[i] = false;
	}
	m_maskStageIndex = 0;
	m_newMaskStage = true;
	m_newScanStep = true;

	//JW: ROD specific stuff--> no need to change this if no ROD is used!
	if (m_dspProcessing[0]) m_newScanStep = false;
	if (m_dspMaskStaging || m_maskStageMode == STATIC || m_runType != NORMAL_SCAN) m_newMaskStage = false;

	m_modScurveFit = 0;
	// Reset histograms
	m_histo.clear();
}

bool PixScan::loop(int index) {
	if (index < MAX_LOOPS) {
		if (m_loopTerminating[index]) {
			m_loopTerminating[index] = false;
			m_loopEnded[index] = true;
			if (index == 0) {
				m_maskStageIndex = 0;
				m_newMaskStage = true;
				m_newScanStep = true;

				//JW: ROD specific stuff--> no need to change this if no ROD is used!
				if (m_dspProcessing[0]) m_newScanStep = false;
				if (m_dspMaskStaging || m_maskStageMode == STATIC || m_runType != NORMAL_SCAN) m_newMaskStage = false;

			}
			m_loopIndex[index] = 0;
			return false;
		} else {
			m_loopEnded[index] = false;
			return true;
		}
	}
	throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
	return false;
}

int PixScan::scanIndex(int index) {
	if (index < MAX_LOOPS) {
		return m_loopIndex[index];
	}
	throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
	return 0;
}

void PixScan::next(int index) {
	if (index == 0) {

		//JW: ROD specific stuff
		if ((m_dspProcessing[0] || !m_loopActive[0]) &&
			(m_dspMaskStaging || m_maskStageMode == STATIC || m_runType != NORMAL_SCAN)) {

				m_loopIndex[index] = m_loopVarNSteps[index] - 1;
				m_maskStageIndex = m_maskStageSteps - 1;
				m_loopTerminating[0] = true;
				m_newScanStep = false;
				m_newMaskStage = false;
		} else if (m_dspProcessing[0] || !m_loopActive[0]) {
			if (m_maskStageIndex < m_maskStageSteps - 1) {
				m_maskStageIndex++;
				m_newScanStep = false;
				m_newMaskStage = true;
			} else {
				m_loopTerminating[0] = true;
			}
		} else if (m_dspMaskStaging || m_maskStageMode == STATIC || m_runType != NORMAL_SCAN) {
			if (m_loopIndex[0] < m_loopVarNSteps[0] - 1) {
				m_loopIndex[0]++;
				m_newScanStep = true;
				m_newMaskStage = false;
			} else {
				m_loopTerminating[0] = true;
			}
		} else {
			if ((m_loopIndex[0] == m_loopVarNSteps[0] - 1) && (m_maskStageIndex == m_maskStageSteps - 1)) {
				m_loopTerminating[0] = true;
				m_newScanStep = false;
				m_newMaskStage = false;
			} else {
				if (m_innerLoopSwap) {
					if (m_loopIndex[0] < m_loopVarNSteps[0] - 1) {
						m_loopIndex[0]++;
						m_newScanStep = true;
						m_newMaskStage = false;
					} else {
						m_loopIndex[0] = 0;
						m_maskStageIndex++;
						m_newScanStep = false;
						m_newMaskStage = true;
					}
				} else {
					if (m_maskStageIndex < m_maskStageSteps - 1) {
						m_maskStageIndex++;
						m_newScanStep = false;
						m_newMaskStage = true;
					} else {
						m_maskStageIndex = 0;
						m_loopIndex[0]++;
						m_newScanStep = true;
						m_newMaskStage = true;
					}
				}
			}
		}
	} else if (index < MAX_LOOPS) {
		if (m_dspProcessing[index] || !m_loopActive[index]) {
			m_loopIndex[index] = m_loopVarNSteps[index] - 1;
			m_loopTerminating[index] = true;
		} else {
			if ((m_loopIndex[index] < m_loopVarNSteps[index] - 1) || m_loopVarValuesFree[index]) {
				m_loopIndex[index]++;
			} else {
				m_loopTerminating[index] = true;
			}
		}
	} else {
		throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
	}
}

void PixScan::terminate(int index) {
	if (index > 0 && index < MAX_LOOPS) {
		if (m_loopVarValuesFree[index]) {
			m_loopTerminating[index] = true;
			m_loopVarNSteps[index] = m_loopIndex[index]+1;
		}
	} else {
		throw PixScanExc(PixControllerExc::ERROR, "Invalid loop index");
	}
}

bool PixScan::newMaskStep() {
	return m_newMaskStage;
}

bool PixScan::newScanStep() {
	return m_newScanStep;
}

void PixScan::clearHisto(unsigned int mod, HistogramType type) {
	// Clear histograms
	if (m_histo.find(type) != m_histo.end()) {
		if (m_histo[type].exists(mod)) {
			m_histo[type][mod].clear();
		}
	}
}

void PixScan::prepareIndexes(HistogramType type, unsigned int mod, int ix2, int ix1, int ix0) {
	bool typeOk = false;
	for (std::map<std::string, int>::iterator itt = m_histogramTypes.begin();
		itt != m_histogramTypes.end(); ++itt)                                 {
			if ((*itt).second == type) typeOk = true;
	}
	if (!typeOk) {
		throw PixScanExc(PixControllerExc::ERROR, "Invalid histogram type");
	}
	if (mod > 31) {
		throw PixScanExc(PixControllerExc::ERROR, "Invalid module number");
	}
	// Check if m_histo[type] is defined
	if (m_histo.find(type) == m_histo.end()) m_histo[type] = *(new PixScanHisto());
	// If ix2 is positive check if m_histo[type][mod] is defined as a list
	if (ix2 >= 0) {
		if (m_histo[type].exists(mod)) {
			if (m_histo[type][mod].histoMode()) {
				throw PixScanExc(PixControllerExc::ERROR, "Invalid loop 2 index");
			}
		} else {
			PixScanHisto *sh = new PixScanHisto();
			m_histo[type].add(mod, *sh);
		}
		// If ix1 is positive check if m_histo[type][mod][ix2] is defined as a list
		if (ix1 >= 0) {
			if (m_histo[type][mod].exists(ix2)) {
				if (m_histo[type][mod][ix2].histoMode()) {
					throw PixScanExc(PixControllerExc::ERROR, "Invalid loop 1 index");
				}
			} else {
				PixScanHisto *sh = new PixScanHisto();
				m_histo[type][mod].add(ix2, *sh);
			}
			// If ix0 is positive check if m_histo[type][mod][ix2] is defined as a list
			if (ix0 >= 0) {
				if (m_histo[type][mod][ix2].exists(ix1)) {
					if (m_histo[type][mod][ix2][ix1].histoMode()) {
						throw PixScanExc(PixControllerExc::ERROR, "Invalid loop 0 index");
					}
				} else {
					PixScanHisto *sh = new PixScanHisto();
					m_histo[type][mod][ix2].add(ix1, *sh);
				}
			}
		}
	}
}

void PixScan::addHisto(Histo &his, HistogramType type, unsigned int mod, int ix2, int ix1, int ix0) {
	// Check if the list structure is in place and create the missing parts
	prepareIndexes(type, mod, ix2, ix1, ix0);
	// Now add the histogram
	if (ix2 < 0) {
		if (m_histo[type].exists(mod)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type].add(mod, *(new PixScanHisto(his)));
	} else if (ix1 < 0) {
		if (m_histo[type][mod].exists(ix2)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type][mod].add(ix2, *(new PixScanHisto(his)));
	} else if (ix0 < 0) {
		if (m_histo[type][mod][ix2].exists(ix1)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type][mod][ix2].add(ix1, *(new PixScanHisto(his)));
	} else {
		if (m_histo[type][mod][ix2][ix1].exists(ix0)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type][mod][ix2][ix1].add(ix0, *(new PixScanHisto(his)));
	}
}

void PixScan::addHisto(std::vector< Histo * >&his, HistogramType type, unsigned int mod, int ix2, int ix1, int ix0) {
	// Check if the list structure is in place and create the missing parts
	prepareIndexes(type, mod, ix2, ix1, ix0);
	// Now add the histogram
	if (ix2 < 0) {
		if (m_histo[type].exists(mod)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type].add(mod, his);
	} else if (ix1 < 0) {
		if (m_histo[type][mod].exists(ix2)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type][mod].add(ix2, his);
	} else if (ix0 < 0) {
		if (m_histo[type][mod][ix2].exists(ix1)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type][mod][ix2].add(ix1, his);
	} else {
		if (m_histo[type][mod][ix2][ix1].exists(ix0)) {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
		}
		m_histo[type][mod][ix2][ix1].add(ix0, his);
	}
}

void PixScan::addHisto(std::vector< Histo * >&his, HistogramType type, unsigned int mod, int ix2, int ix1) {
	for (unsigned int ih = 0; ih < his.size(); ih++) {
		// Check if the list structure is in place and create the missing parts
		if (ih == 0) {
			if (ix2 < 0) {
				prepareIndexes(type, mod, ih, -1, -1);
			} else if (ix1 < 0) {
				prepareIndexes(type, mod, ix2, ih, -1);
			} else {
				prepareIndexes(type, mod, ix2, ix1, ih);
			}
		}
		// Now add the histograms
		if (ix2 < 0) {
			if (m_histo[type][mod].exists(ih)) {
				throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
			}
			m_histo[type][mod].add(ih, *(new PixScanHisto(*his[ih])));
		} else if (ix1 < 0) {
			if (m_histo[type][mod][ix2].exists(ih)) {
				throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
			}
			m_histo[type][mod][ix2].add(ih, *(new PixScanHisto(*his[ih])));
		} else {
			if (m_histo[type][mod][ix2][ix1].exists(ih)) {
				throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
			}
			m_histo[type][mod][ix2][ix1].add(ih, *(new PixScanHisto(*his[ih])));
		}
	}
}

void PixScan::addHisto(std::vector< Histo * >&his, unsigned int nh, HistogramType type, unsigned int mod, int ix2, int ix1) {
	unsigned int nbin = his.size()/nh;
	for (unsigned int ib = 0; ib < nbin; ib++) {
		for (unsigned int ih = 0; ih < nh; ih++) {
			// Check if the list structure is in place and create the missing parts
			if (ih == 0) {
				if (ix2 < 0) {
					prepareIndexes(type, mod, ib, -1, -1);
				} else if (ix1 < 0) {
					prepareIndexes(type, mod, ix2, ib, -1);
				} else {
					prepareIndexes(type, mod, ix2, ix1, ib);
				}
				// Now add the histograms
				if (ix2 < 0) {
					if (m_histo[type][mod].exists(ib)) {
						throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
					}
					m_histo[type][mod].add(ib, *(new PixScanHisto(0, *his[ib*nh])));
				} else if (ix1 < 0) {
					if (m_histo[type][mod][ix2].exists(ib)) {
						throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
					}
					m_histo[type][mod][ix2].add(ib, *(new PixScanHisto(0, *his[ib*nh])));
				} else {
					if (m_histo[type][mod][ix2][ix1].exists(ib)) {
						throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
					}
					m_histo[type][mod][ix2][ix1].add(ib, *(new PixScanHisto(0, *his[ib*nh])));
				}
			} else {
				if (ix2 < 0) {
					m_histo[type][mod][ib].add(ih, *his[ib*nh+ih]);
				} else if (ix1 < 0) {
					m_histo[type][mod][ix2][ib].add(ih, *his[ib*nh+ih]);
				} else {
					m_histo[type][mod][ix2][ix1][ib].add(ih, *his[ib*nh+ih]);
				}
			}
		}
	}
}

void PixScan::addHisto(std::vector< Histo * >&his, unsigned int d2, unsigned int d1, HistogramType type, unsigned int mod, int ix2) {
	if (d1*d2 != his.size()) {
		throw PixScanExc(PixControllerExc::ERROR, "Number of histograms invalid");
	}
	unsigned int cnt = 0;
	for (unsigned int ih2 = 0; ih2 < d2; ih2++) {
		for (unsigned int ih1 = 0; ih1 < d1; ih1++) {
			// Check if the list structure is in place and create the missing parts
			if (ih1 == 0) {
				if (ix2 < 0) {
					prepareIndexes(type, mod, ih2, ih1, -1);
				} else {
					prepareIndexes(type, mod, ix2, ih2, ih1);
				}
			}
			// Now add the histograms
			if (ix2 < 0) {
				if (m_histo[type][mod][ih2].exists(ih1)) {
					throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
				}
				m_histo[type][mod][ih2].add(ih1, *his[cnt]);
			} else {
				if (m_histo[type][mod][ix2][ih2].exists(ih1)) {
					throw PixScanExc(PixControllerExc::ERROR, "Histogram already exists");
				}
				m_histo[type][mod][ix2][ih2].add(ih1, *his[cnt]);
			}
			cnt++;
		}
	}
}

void PixScan::downloadHisto(PixController *ctrl, unsigned int mod, HistogramType type) {
	// Check if it's time to download
	int ix0 = m_loopIndex[0];
	int ix1 = m_loopIndex[1];
	int ix2 = m_loopIndex[2];
	int level = -1, depth = -1;

	//  USBPixController *usb = dynamic_cast<USBPixController*>(ctrl);

	if(PS_DEBUG) cout<<"DEBUG PixScan::downloadHisto: HistogramType="<<type<<", module " << mod<<endl;
	if (type == OCCUPANCY || type == LVL1 || type == TOT || type == TOT_MEAN || type == LV1ID || type == BCID ||
		type == TOT_SIGMA || type == SCURVE_MEAN || type == SCURVE_SIGMA || type == SCURVE_CHI2 ||
		type == HITOCC || type==TOTAVERAGE || type == CLUSTER_SIZE || type == CLSIZE_TOT || type == CLUSTER_TOT || type == CLUSTER_CHARGE || type == CLUSTER_POSITION || type == CLSIZE_CHARGE ||//DLP
		type == SEED_TOT || type == SEED_LVL1 || type == NSEEDS || type == TOT0 || type == TOT1 || type == TOT2 || type == TOT3 || type == TOT4 || type == TOT5 || type == TOT6 || type == TOT7 || type == TOT8 || type == TOT9 || type == TOT10 || type == TOT11 || type == TOT12 || type == TOT13 || type == TOT14 || type == TOT15) {
			if (m_loopActive[1] && m_dspProcessing[1]) {
				if (m_loopEnded[1]) {
					level = 1;
					depth = 2;
				}
			} else if (m_loopActive[0] && m_dspProcessing[0]) {
				if (m_loopEnded[0]) {
					level = 2;
					depth = 1;
				}
			} else if (type == OCCUPANCY || type == LVL1 || type == TOT || type == TOT_MEAN || type == LV1ID || type == BCID ||
				type == TOT_SIGMA || type == HITOCC || type==TOTAVERAGE || type == CLUSTER_SIZE || type == CLSIZE_TOT || type == CLUSTER_TOT || type == CLUSTER_CHARGE || type == CLUSTER_POSITION || type == CLSIZE_CHARGE ||//DLP
				type == SEED_TOT || type == SEED_LVL1 || type == NSEEDS || type == TOT0 || type == TOT1|| type == TOT2 || type == TOT3 || type == TOT4 || type == TOT5 || type == TOT6 || type == TOT7 || type == TOT8 || type == TOT9 || type == TOT10 || type == TOT11 || type == TOT12 || type == TOT13 || type == TOT14 || type == TOT15) {
					level = 3;
					depth = 0;
			}
			if (type == LVL1 || type == SCURVE_MEAN || type == SCURVE_SIGMA || type == SCURVE_CHI2) {
				depth = 0;
			}
	}
	if (level >= 0) {
		// Check if the module is active
		int dsp = -1;
		bool active = false;
		for (unsigned int ndsp=0; ndsp<MAX_GROUPS; ndsp++) {
			if (((m_moduleMask[ndsp] & (0x1<<mod)) && ctrl->moduleActive(mod))) {
				active = true;
				dsp = ndsp;
			}
		}

		bool numok = false;
		if (active && m_histogramFilled[type]) {
			if(PS_DEBUG) cout<<"DEBUG PixScan::downloadHisto: level="<<level<<endl;
			if(PS_DEBUG) cout<<"DEBUG PixScan::downloadHisto: depth="<<depth<<endl;

			if (type == OCCUPANCY || type == LVL1 || type == TOT || type == TOT_MEAN || type == LV1ID || type == BCID ||
				type == TOT_SIGMA || type == HITOCC || type==TOTAVERAGE || type == CLUSTER_SIZE || type == CLSIZE_TOT || type == CLUSTER_TOT || type == CLUSTER_CHARGE || type == CLUSTER_POSITION || type == CLSIZE_CHARGE || //DLP
				type == SEED_TOT || type == SEED_LVL1 || type == NSEEDS || type == TOT0 || type == TOT1 || type == TOT2 || type == TOT3 || type == TOT4 || type == TOT5 || type == TOT6 || type == TOT7 || type == TOT8 || type == TOT9 || type == TOT10 || type == TOT11 || type == TOT12 || type == TOT13 || type == TOT14 || type == TOT15) {
					std::vector< std::vector< Histo * > >tmpHisto;
					ctrl->getHisto((PixController::HistoType)type, mod, dsp, tmpHisto);
					if(PS_DEBUG) cout<<"DEBUG PixScan::downloadHisto: Histograms downloaded. tmpHisto.size()="<<tmpHisto.size()<<endl;

					if (level == 1) {
						if (type == LVL1) {
							if (m_lvl1HistoBinned) {
								throw PixScanExc(PixControllerExc::ERROR, "Binned LVL1 histos not supported in 2-dim loops");
							} else {
								if ((int)tmpHisto[mod].size() == m_consecutiveLvl1TrigA[0]) {
									addHisto(tmpHisto[mod], type, mod, ix2, -1, -1);
									numok = true;
								}
							}
						} else if (type == HITOCC || type==TOTAVERAGE ) {
							if ((int)tmpHisto[mod].size() == 1) {
								addHisto(tmpHisto[mod], type, mod, ix2, -1, -1);
								numok = true;
							}
						} else {
							if ((int)tmpHisto[mod].size() == m_loopVarNSteps[0]*m_loopVarNSteps[1] || m_loopVarValuesFree[1]) {
								addHisto(tmpHisto[mod], m_loopVarNSteps[1], m_loopVarNSteps[0], type, mod, ix2);
								numok = true;
							}
						}
					} else if (level == 2) {
						if (type == LVL1) {
							if (m_lvl1HistoBinned) {
								if ((int)tmpHisto[mod].size() == m_consecutiveLvl1TrigA[0]*m_loopVarNSteps[0] || m_loopVarValuesFree[0]) {
									addHisto(tmpHisto[mod], m_consecutiveLvl1TrigA[0], type, mod, ix2, ix1);
									numok = true;
								}
							} else {
								if ((int)tmpHisto[mod].size() == m_consecutiveLvl1TrigA[0] || m_loopVarValuesFree[0]) {
									addHisto(tmpHisto[mod], type, mod, ix2, ix1, -1);
									numok = true;
								}
							}
						} else if (type == HITOCC || type==TOTAVERAGE) {
							if ((int)tmpHisto[mod].size() == 1) {
								addHisto(tmpHisto[mod], type, mod, ix2, ix1);
								numok = true;
							}
						} else {
							//	    cout<<"DEBUG PixScan: level=2 - tmpHisto[mod].size()="<<tmpHisto[mod].size()<<"; m_loopVarNSteps[0]="<<m_loopVarNSteps[0]<<endl;
							if ((int)tmpHisto[mod].size() == m_loopVarNSteps[0]) {
								addHisto(tmpHisto[mod], type, mod, ix2, ix1);
								numok = true;
							}
						}
					} else if (level == 3) {
						if (type == LVL1) {
							if(PS_DEBUG) cout<<"DEBUG PixScan: downloadHisto found level=3 and type=LVL1"<<endl;
							if(PS_DEBUG) cout<<"DEBUG PixScan: histo size = "<<(int)tmpHisto[mod].size()<<endl;
							if ((int)tmpHisto[mod].size() == 1) {   //m_consecutiveLvl1TrigA[0]) {
								addHisto(tmpHisto[mod], type, mod, ix2, ix1, ix0);
								numok = true;
							}
						} else {
							if(PS_DEBUG) cout<<"DEBUG PixScan: level=3 - tmpHisto[mod].size()="<<tmpHisto[mod].size()<<" with mod="<<mod<<endl;
							if (tmpHisto[mod].size() == 1) {
								addHisto(*tmpHisto[mod][0], type, mod, ix2, ix1, ix0);
								numok = true;
							}
						}
					}
					if (!numok) throw PixScanExc(PixControllerExc::ERROR, "Wrong number of histograms downloaded");
			} else if (type == SCURVE_MEAN || type == SCURVE_SIGMA || type == SCURVE_CHI2) {
				// Perform fits
				if (ctrl->fitHistos()) m_modScurveFit = 0;
				if ((m_modScurveFit & (0x1<<mod)) == 0) {
					if (m_tmpSCmean.size() > mod) m_tmpSCmean[mod] = NULL;
					if (m_tmpSCmean.size() > mod) m_tmpSCsigma[mod] = NULL;
					if (m_tmpSCmean.size() > mod) m_tmpSCchi2[mod] = NULL;
					// Get fit results from the controller
					ctrl->getFitResults(PixController::SCURVES, mod, dsp, m_tmpSCmean, m_tmpSCsigma, m_tmpSCchi2);
					m_modScurveFit |= (0x1<<mod);
				}
				Histo *h;;
				if (type == SCURVE_MEAN) {
					h = m_tmpSCmean[mod];
				} else if (type == SCURVE_SIGMA) {
					h = m_tmpSCsigma[mod];
				} else if (type == SCURVE_CHI2) {
					h = m_tmpSCchi2[mod];
				}
				if (level == 1) {
					addHisto(*h, type, mod, ix2, -1, -1);
				} else if (level == 2) {
					addHisto(*h, type, mod, ix2, ix1, -1);
				}
			}
		}
	}
}

bool PixScan::downloadErrorHisto(PixController *ctrl, unsigned int dsp) {

  HistogramType type = DSP_ERRORS;
  
  int ix0 = m_loopIndex[0];
  int ix1 = m_loopIndex[1];
  int ix2 = m_loopIndex[2];
  int level = -1;
  
  if (m_loopActive[1] && m_dspProcessing[1]) {
    level = 1;
  } else if (m_loopActive[0] && m_dspProcessing[0]) {
    level = 2;
  } else {
    level = 3;
  }
  
  
  if(PS_DEBUG) std::cout << "level " << level
			 << "m_loopActive " << m_loopActive[1] << " " <<   m_loopActive[0]
			 << " m_loopEnded " << m_loopEnded[1] << " " << m_loopEnded[0]
			 << " m_dspProcessing " <<  m_dspProcessing[1] << " " << m_dspProcessing[0]
			 << std::endl;
  
  if (level >= 0) {
    if (m_histogramFilled[type]) {
      // Get error histograms from the controller
      Histo *h;;
      if(!ctrl->getErrorHistos(dsp, h)) return false;
      for (unsigned int mod=0; mod<32; mod++) {
	if ((m_moduleMask[dsp] & (0x1<<mod)) && ctrl->moduleActive(mod)) {
	  if (level == 1) {
	    addHisto(*h, type, mod, ix2, -1, -1);
	  } else if (level == 2) {
	    addHisto(*h, type, mod, ix2, ix1, -1);
	  } else {
	    addHisto(*h, type, mod, ix2, ix1, ix0);
	  }
	}
      }
    } else
      return false;
  } else
    return false;
  return true;
}

Histo& PixScan::getHisto(HistogramType type, int module, int idx2, int idx1, int idx0) {
	static Histo tmp("Not Found","Not Found",10, -0.5, 9.5, 10, -0.5, 9.5);
	try {
		Histo &h = m_histo[type][module][idx2][idx1][idx0].histo();
		return h;
	}
	catch (PixScanExc &) {
		tmp.clear();
		return tmp;
	}
	return tmp;
}

Histo& PixScan::getHisto(HistogramType type, int module, int idx2, int idx1, int idx0, int ih) {
	static Histo tmp("Not Found","Not Found",10, -0.5, 9.5, 10, -0.5, 9.5);
	try {
		Histo &h = m_histo[type][module][idx2][idx1][idx0].histo(ih);
		return h;
	}
	catch (PixScanExc &) {
		tmp.clear();
		return tmp;
	}
	return tmp;
}

PixScanHisto& PixScan::getHisto(HistogramType type) {
	bool typeOk = false;
	for (std::map<std::string, int>::iterator itt = m_histogramTypes.begin();
		itt != m_histogramTypes.end(); ++itt)                                 {
			if ((*itt).second == type) typeOk = true;
	}
	if (!typeOk) {
		throw PixScanExc(PixControllerExc::ERROR, "Invalid histogram type");
	}
	// Check if m_histo[type] is defined
	if (m_histo.find(type) == m_histo.end()) {
		std::stringstream a;
		a << (int) type;
		throw PixScanExc(PixControllerExc::ERROR, "Histogram type "+a.str()+" not found");
	} else {
		return m_histo[type];
	}
}

void PixScan::fitSCurve(PixScanHisto &sc, Histo &thr, Histo &noise, Histo& chi2, int /*ih*/, int rep) {
	if (sc.histoMode()) {
		throw PixScanExc(PixControllerExc::ERROR, "Not in map mode");
	}
	if (!sc[0].histoMode()) {
		throw PixScanExc(PixControllerExc::ERROR, "Histogram not found");
	}
	if (sc.size() < 4) {
		throw PixScanExc(PixControllerExc::ERROR, "Not enough bin to fit");
	}
	Histo **hsc = new Histo*[sc.size()];
	for (unsigned int i=0; i<sc.size(); i++) {
		hsc[i] = &sc[i].histo();
	}
	for (int col = 0; col<hsc[0]->nBin(0); col++) {
	  for (int row = 0; row<hsc[0]->nBin(1); row++) {
			bool low = true;
			bool top = false;
			bool fl = false;
			bool fh = false;
			unsigned int i;
			double ymax = rep;
			double xmax = sc.size();
			double xl=0,yl=0,xh=xmax,yh=ymax;
			for (i=1; i<sc.size()-1; i++) {
				double his = (*hsc[i])(col,row);
				double hisp= (*hsc[i-1])(col,row);
				double hisn= (*hsc[i+1])(col,row);
				if (low) {
					if (his > ymax/2) low = false;
					if (!fl && his>0.18*ymax && hisn>0.18*ymax) {
						fl = true;
						xl = i;
						yl = (hisp+his+hisn)/3.0;
					}
				} else {
					if (his>0.9*ymax && hisn>0.9*ymax) top =true;
					if (!fh && his>0.82*ymax && hisn>0.82*ymax) {
						fh = true;
						xh = i;
						yh = (hisp+his+hisn)/3.0;
					}
				}
				if (top) {
					double a = (yh-yl)/(xh-xl);
					double b = yl - a*xl;
					if (a > 0) {
						double threshold = (0.5*ymax/a - b/a);
						double sigma = (0.3413*ymax/a);
						double c2 = 0;
						for (i=(unsigned int)xl; i<(unsigned int)xh+1; i++) {
							if(ymax!=0){
								double ycurr = his;
								double yerr  = sqrt(ycurr/ymax*(ymax-ycurr));
								if(yerr!=0) {
									c2 += pow((ycurr-b-a*(double)i)/yerr,2);
								}
							}
						}
						thr.set(col,row,threshold);
						noise.set(col,row,sigma);
						chi2.set(col,row,c2);
						//std::cout << col << " " << row << " " << threshold << " " << sigma << std::endl;
					} else {
						thr.set(col,row,0.0);
						noise.set(col,row,0.0);
						chi2.set(col,row,-2.0);
					}
					break;
				}
			}
		}
	}
}

void PixScan::readConfig(DBInquire *dbi){  // load configuration from the database
	m_dbInquire = dbi;
	m_conf->read(m_dbInquire);
}

void PixScan::writeConfig(DBInquire *dbi){  // save current configuration into the database
	m_dbInquire = dbi;
	m_conf->write(m_dbInquire);
}

void PixScan::writeHistoLevel(DBInquire *dbi, PixScanHisto &sc, std::string lName, int lvl) {
	if (sc.histoMode()) {
		int nh = sc.histoSize();
		for (int ih=0; ih<nh; ih++) {
			std::stringstream fieldname;
			fieldname << lName << "_" << ih;
			DBField* dbfield = dbi->getDB()->makeField(fieldname.str());
			Histo& bla=sc.histo(ih);
			dbi->getDB()->DBProcess(dbfield,COMMIT, bla);     // sc.histo(ih));
			dbi->pushField(dbfield);
			delete dbfield;
		}
	} else {
		int nl = sc.size();
		if (lvl == 0) nl = 32;
		for (int il=0; il<nl; il++) {
			if (sc.exists(il)) {
				std::stringstream fieldname;
				std::string nlName = "_";
				if (lvl == 0) nlName = "Mod";
				if (lvl == 1) nlName = "_A";
				if (lvl == 2) nlName = "_B";
				if (lvl == 3) nlName = "_C";
				fieldname << lName << nlName << il;
				DBInquire* nli = dbi->addInquire(fieldname.str(), fieldname.str());
				writeHistoLevel(nli, sc[il], fieldname.str(), lvl+1);
			}
		}
	}
	dbi->getDB()->DBProcess(dbi,COMMITREPLACE);
}

void PixScan::writeHisto(DBInquire *dbi){
	for (std::map<std::string, int>::iterator itt = m_histogramTypes.begin();
		itt != m_histogramTypes.end(); ++itt)                                 {
			HistogramType type = (HistogramType)((*itt).second);
			std::string typeName = (*itt).first;
			if (m_histogramKept[type] && m_histo.find(type) != m_histo.end()) {
				DBInquire* typeI = dbi->addInquire(typeName, typeName);
				writeHistoLevel(typeI, m_histo[type], "", 0);
			}
	}
}

void PixScan::readHisto(DBInquire* dbi) {
	m_histo.clear();
	for (recordIterator it = dbi->recordBegin(); it!=dbi->recordEnd(); it++) {
		std::map<std::string, int>::iterator typeit=m_histogramTypes.find((*it)->getName());
		if (typeit!=m_histogramTypes.end()) {
			//std::cout << "histo type " <<(*it)->getName() << std::endl;
			HistogramType type = (HistogramType)typeit->second;
			if (m_histo.find(type) == m_histo.end()) m_histo[type] = *(new PixScanHisto());
			std::string fieldname = "Mod";
			for (recordIterator vt = (*it)->recordBegin(); vt!=(*it)->recordEnd(); vt++){
				std::string recname = (*vt)->getName();
				unsigned long int pos = recname.find(fieldname);
				if (pos != (unsigned long int) std::string::npos){
					unsigned int il;
					std::istringstream is(recname.substr(fieldname.size(), recname.size()-fieldname.size()));
					is >> il;
					readHistoLevel(*vt, m_histo[type], recname, 1, il);
				}
			}
		}
	}
}

void PixScan::readHistoLevel(DBInquire* dbi, PixScanHisto &sc, std::string lName, int lvl, int idx) {
	std::stringstream fieldname;
	unsigned int ih = 0;
	fieldname << lName << "_" << ih;
	fieldIterator fit = dbi->findField(fieldname.str());
	if (fit != dbi->fieldEnd()) {  // histos on this level
		do {
			Histo *h = new Histo();
			dbi->getDB()->DBProcess(fit, READ, *h);
			if (ih == 0) {
				PixScanHisto *sh = new PixScanHisto(*h);
				sc.add(idx, *sh);
			} else {
				sc[idx].add(ih, *h);
			}
			fieldname.str("");
			fieldname << lName << "_" << ++ih;
			fit = dbi->findField(fieldname.str());
		} while (fit != dbi->fieldEnd());
	} else {
		fieldname.str("");
		std::string nlName = "_", recname;
		if (lvl == 0) nlName = "Mod";
		if (lvl == 1) nlName = "_A";
		if (lvl == 2) nlName = "_B";
		if (lvl == 3) nlName = "_C";
		fieldname << lName << nlName;
		for(recordIterator vt = dbi->recordBegin(); vt!=dbi->recordEnd(); vt++){
			recname = (*vt)->getName();
			unsigned long int pos = recname.find(fieldname.str());
			if (pos != (unsigned long int) std::string::npos){
				unsigned int il;
				std::istringstream is(recname.substr(fieldname.str().size(), recname.size()-fieldname.str().size()));
				is >> il;
				if (!sc.exists(idx)) sc.add(idx, *(new PixScanHisto()));
				readHistoLevel(*vt, sc[idx], recname, lvl+1, il);
			}
		}
	}
}

void PixScan::write(DBInquire* dbi){
	// write config
	writeConfig(dbi);
	// write histograms
	writeHisto(dbi);
}

void PixScan::read(DBInquire* dbi){
	// read config
	readConfig(dbi);
	// read histograms
	readHisto(dbi);
}

float PixScan::getMinThrNoiseCut(int type){
	switch(type){
  case 0: // normal
  default:
	  return m_minThrNoiseCutNormal;
  case 1: // long
	  return m_minThrNoiseCutLong;
  case 2: // ganged
	  return m_minThrNoiseCutGanged;
  case 3: // inter-ganged
	  return m_minThrNoiseCutInterGanged;
	}
}
