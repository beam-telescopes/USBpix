#ifndef OPTCLASS_H
#define OPTCLASS_H

#include <stdio.h>
#include <string>
#include <vector>
#include "FileTypes.h"

#define NPLHIMAX 100

/*! Contains the plotting/analysis options; inheritance
 *  from TObject allows saving as ROOT object.
 */

class optClass{

 public:
  optClass(bool haveGUI=true);
  ~optClass();
  void Write(FILE *);
  void Read(FILE *);

  bool   m_SCmode;     // start TopWin panel in SC mode, not in MCM mode
  bool   m_splitnsp;   // split noise/xtalk plot into normal, long,...
  bool   m_splitthr;   // split threshold plot into normal, long,...
  bool   m_showwarn;
  int    m_plotfitopt; // 0: plot all three; 1=map, 2=proj., 3=scatter only
  int    m_showchipid; // show the chip numbers in 2D maps
  bool   m_showAxisLabels;
  int    m_projlogy;   // plot Y-axis in log for projections
  float  m_hitcut, m_totcut; // cuts for mask generation
  float  m_diffcut, m_noisecut; // cuts for HV=0(AMS) test
  bool   m_showcutpix; // HV=0 noise cut: show pixels failing cut in the plot
  float  m_xtfrcut, m_xtnoisecut; // cut for merged pixels
  int    m_masklong, m_maskgang; // auto-mask of ganged/long pixels
  int    m_MCCdelay;  // obsolete 
  int    m_QhighTW;  // Q_t0 for timewalk analysis
  float  m_ChipTarget;   // fixed target for TDAC calculation
  int    m_doChipTarget; // DAC calculation per chip or module
  int    m_doFixedTarget; // DAC calculation with set target
  bool   m_rec2DFit;  // use existing fit-results for TDAC calculation
  bool   m_keep1d; // keep DAC fitting histos after calculation
  bool   m_DACdoFit;  // do a fit to get best DAC; otherwise look at each scan pt.
  int	 m_noisetab;  // do plot of pixels passing noise(AMS)-cut
  int    m_zerotab;  // to list also channels=0 in BadPixel tables
  int    m_FitGauss;  // fit a Gauss to (almost) all 1D projections
  int    m_ToggleMask; // allow user to toggle mask at indiv. pixels via mouse click
  int    m_colourScale; // colour scheme for 2D maps
  bool   m_doBlackBg;   // make 2D maps background black
  int    m_ToTPlotPnt;  // charge at which ToT from cal. fit is plotted
  int    m_QPlotPnt;    // historic name; now is point at which we start looking at the data
  bool   m_fixedToTSwitch; // ToT cal combination: fixed changeover-point
  int    m_ToTSwitch;    // ToT cal combination: changeover point (if fixed)
  float  m_UsrMin, m_UsrMax; // user-defined range for projection histograms
  bool   m_doUsrRg;       // switch user-defined range on
  bool   m_doTotRelcal;   // run ToT Chi/Clo calibration fit before combined fit (std-tests only)
  int    m_minToT;        // source dead pixel ana: if spectrum doesn't exceed this it's dead too
  int    m_peakMinHit;    // ToT peak finder: ignore pixels with hits less than this
  bool   m_showChipCuts;  // CutPanel lists and QAreport uses chip-level cuts (intended for baremodule analysis)
  float  m_UsrIMin[DSET], m_UsrIMax[DSET];// user-defined range indiv. def. for projection histograms
  bool   m_UsrIUse[DSET];// enable user-defined range indiv. def. for projection histograms
  bool   m_PLhistoExists[NPLHIMAX]; // does this histo exist in PixLib?
  int    m_PLplotType[NPLHIMAX]; // plot type - see array below
  std::vector<std::string> m_PLplotTypeLabels;
  float  m_PLUsrIMin[NPLHIMAX], m_PLUsrIMax[NPLHIMAX];// user-defined range indiv. def. for projection histograms
  bool   m_PLUsrIUse[NPLHIMAX];// enable user-defined range indiv. def. for projection histograms
  bool   m_PLPltZero[NPLHIMAX];// ignose values below zero
  std::vector<std::string> m_StdFname; // user-defined std. file names
  std::string m_DefDataPath, m_DefMacroPath, m_DefCutPaths; // std. paths (for eg. module data)
  bool   m_UseMname;
  bool   m_CutCreateRoot, m_CutIgnoreBad, m_CutWebCopy, m_CutWebRemove;
  std::string m_CutWebPath, m_CutWebUser, m_CutWebCmd;
  std::string m_pdbpath;
  bool m_pdbPlots, m_pdbRemove, m_pdbClear;
  float m_srcMin;  // min ToT from ToT_max for source peak finder
  bool m_srcRestrict; // restrict with the above
  bool m_srcRecalcPeak; // re-determine peak position each time user wants the plot
  bool m_cfgUsrdefNames;
  int m_VbiasMin, m_VbiasMax; // Vbias range for Ibias stability plot
  bool m_IbiasCorr;  // same plot, apply T-correction to current
  // supply & channels for LV, HV supplies in the _pictmon.out file
  int    m_DDchan, m_DDAchan, m_BIASchan,m_DDsupp, m_DDAsupp, m_BIASsupp;
  // noise cuts (normal, long ganged int.g. pixels) for min. threshold analysis
  int m_nseCutmT, m_nseCutmT_long, m_nseCutmT_ganged, m_nseCutmT_inter; 
  float m_chiCutmT; // chi^2 cut for min. threshold determination
  bool m_runCombToTfit; // running combined ToT fit is default in std. load window
  // load and fit bad-fits data for resp. std. threshold scans
  bool m_runBadFitsIntime, m_runBadFitsTzero, m_runBadFitsThresh, m_runBadFitsXtalk; 
  // histogram titles and names
  std::string hinam[DSET], hitit[DSET], axtit[DSET], grtit[DSET];
  int m_summode;  // which scan is to be summarized?
  bool m_absValIV;
  int m_ambushStart, m_ambushStop; // start and stop time relative to scan start
                                   // for averaging in stave/sector production testing
  int m_webUploadOpt;
  std::string m_userPDB, m_passwdPDB;
  int m_delT0_twalk;
  int m_totcheckVar;
  float m_totcheckValue;
  bool m_totcheckKeep;
  std::string m_corPlotOpt;
  bool m_occPlotHisto; // plot hit histograms as map only or as map+histo+scatter?
};

#endif //OPTCLASS_H
