#include "RootStuff.h"
#include "OptClass.h"
#include "StdTestWin.h"
#include "CDatabase.h"
#include "DataStuff.h"
#include "verauth.h"

#include <q3cstring.h>
#include <qstring.h>
#include <string>

#ifdef PIXEL_ROD // can link to PixScan
#include <PixScan.h>
#endif

optClass::optClass(bool haveGUI){
  // define password encryption key
  int i;
  // plotting style options - default
  m_SCmode = false;
  m_splitnsp = true;
  m_splitthr = false;
  m_showwarn = true;
  m_plotfitopt = 0;  
  m_showchipid = 0;
  m_showAxisLabels = false;
  m_projlogy = 0;
  m_hitcut = 1; 
  m_totcut = 200000;
  m_masklong = 0;
  m_maskgang = 0;
  m_diffcut = 110;
  m_noisecut = 360;
  m_showcutpix = false;
  m_xtfrcut = 2.5;
  m_xtnoisecut = 1.5;
  m_noisetab = 0;
  m_zerotab = 1;
  m_MCCdelay = 0;
  m_QhighTW  = 100000;
  m_ChipTarget = 3000;
  m_doChipTarget = 0;
  m_doFixedTarget = 0;
  m_rec2DFit = true;
  m_keep1d = true;
  m_DACdoFit = true;
  m_FitGauss = 1;
  m_ToggleMask = 0;
  m_ToTPlotPnt = 20;
  m_fixedToTSwitch = true;
  m_ToTSwitch = 30;
  m_QPlotPnt = 8;
  m_UsrMin = 0;
  m_UsrMax = 10000;
  m_doUsrRg = false;
  m_doTotRelcal = true;
  m_minToT = 0;
  m_peakMinHit = 1;
  m_showChipCuts = false;
  m_pdbPlots = false;
  m_pdbRemove = false;
  m_pdbClear = false;
  for(i=0;i<DSET;i++){
    m_UsrIMin[i] = -9999;
    m_UsrIMax[i] = -9999;
    m_UsrIUse[i] = false;
  }
  for(i=0;i<NPLHIMAX;i++){
    if(i==(int)PixLib::PixScan::OCCUPANCY || i==(int)PixLib::PixScan::HITOCC)
      m_PLplotType[i] = 1;
    else if((i>=(int)PixLib::PixScan::RAW_DATA_REF && i<=(int)PixLib::PixScan::RAW_DATA_DIFF_2) ||
	    i==(int)PixLib::PixScan::TOT || i==(int)PixLib::PixScan::CLUSTER_SIZE || i==(int)PixLib::PixScan::CLSIZE_TOT || i==(int)PixLib::PixScan::CLSIZE_CHARGE || i==(int)PixLib::PixScan::CLUSTER_POSITION ||
	    i==(int)PixLib::PixScan::DCS_DATA || i==(int)PixLib::PixScan::CLUSTER_TOT || i==(int)PixLib::PixScan::CLUSTER_CHARGE || i==(int)PixLib::PixScan::SEED_TOT ||
	    i==(int)PixLib::PixScan::SEED_LVL1|| i==(int)PixLib::PixScan::NSEEDS || i==(int)PixLib::PixScan::LVL1 ||
	    i==(int)PixLib::PixScan::DSP_ERRORS)
      m_PLplotType[i] = 0;
    else
      m_PLplotType[i] = 2;
    if(i<(int)PixLib::PixScan::MAX_HISTO_TYPES)
      m_PLhistoExists[i] = true;
    else
      m_PLhistoExists[i] = false;
    m_PLUsrIMin[i] = -9999;
    m_PLUsrIMax[i] = -9999;
    m_PLUsrIUse[i] = false;
    if(i==(int)PixLib::PixScan::SCURVE_MEAN || i==(int)PixLib::PixScan::SCURVE_SIGMA ||
       i==(int)PixLib::PixScan::TDAC_THR || i==(int)PixLib::PixScan::GDAC_THR)
      m_PLPltZero[i] = false;
    else
      m_PLPltZero[i] = true;
  }
  m_PLplotTypeLabels.push_back("Simple map/graph");
  m_PLplotTypeLabels.push_back("Module-like colour map");    
  m_PLplotTypeLabels.push_back("Module-Map/histo/scatter (as sel.)");
  m_PLplotTypeLabels.push_back("Module-Map/2xhisto/scatter (as sel.)"); // CRGG
  m_PLplotTypeLabels.push_back("Simple map/histo/scatter (as sel.)");
  m_PLplotTypeLabels.push_back("Simple map/2xhisto/scatter (as sel.)");	// CRGG
  m_PLplotTypeLabels.push_back("Module-Map/2xhisto(VNCAP)/scatter (as sel.)");	// CRGG
  m_PLplotTypeLabels.push_back("Simple map/2xhisto(VNCAP)/scatter (as sel.)");	// CRGG

  m_colourScale = 0;
  m_doBlackBg = false;
  m_srcMin = -1;
  m_srcRestrict = false;
  m_srcRecalcPeak = false;
  m_cfgUsrdefNames = false;
  m_VbiasMin = 145;
  m_VbiasMax = 155;
  m_IbiasCorr = true;
  m_chiCutmT = 50;
  m_nseCutmT = -1;
  m_nseCutmT_long = -1;
  m_nseCutmT_ganged = -1;
  m_nseCutmT_inter = -1;
  m_runCombToTfit = false;
  m_runBadFitsIntime = false;
  m_runBadFitsTzero = false;
  m_runBadFitsThresh = false;
  m_runBadFitsXtalk = false;
  RMain::ColourScale(m_colourScale);
  // supply channel default
  m_DDchan = 1;
  m_DDAchan = 0;
  m_BIASchan = 0;
  m_DDsupp = 0;
  m_DDAsupp = 0;
  m_BIASsupp = 0;
  m_absValIV = false;
  m_ambushStart = 30;
  m_ambushStop = 120;
  // std paths
  m_DefDataPath  = ".";
  m_UseMname = false;
  m_DefMacroPath = ".";
  m_DefCutPaths  = ".";
  m_pdbpath = ".";
  m_userPDB = "";
  m_passwdPDB = "";
  m_webUploadOpt = 0;
  m_delT0_twalk = 20;
  m_totcheckVar = 7;
  m_totcheckValue = 45;
  m_totcheckKeep = true;
  m_corPlotOpt = "LEGO";
  m_occPlotHisto = false;

  // std file names
  // get NSLOTS from StdTestWin
  int NSLOTS=23;
  if(haveGUI){
    StdTestWin tmpwin(NULL,"tmptestwin");
    NSLOTS = tmpwin.NSLOTS;
  }
  m_StdFname.resize(NSLOTS,"");
  m_StdFname[ 0] = "DigitalInj";
  m_StdFname[ 1] = "HV_off";
  m_StdFname[ 2] = "HV_on";
  m_StdFname[ 3] = "IVcurve";
  m_StdFname[ 4] = "Tuning_roomT";
  m_StdFname[ 5] = "Tuned_thres";
  m_StdFname[ 6] = "";
  m_StdFname[ 7] = "xtalk";
  m_StdFname[ 8] = "ToT_CLow";
  m_StdFname[ 9] = "ToT_CHigh";
  m_StdFname[10] = "leakage_150V";
  m_StdFname[11] = "twalk_CLow";
  m_StdFname[12] = "twalk_CHigh";
  m_StdFname[13] = "HotPixel";
  m_StdFname[14] = "Source";
  m_StdFname[15] = "";
  m_StdFname[16] = "TuningF_roomT";
  m_StdFname[17] = "leakage_600V";
  m_StdFname[18] = "";
  m_StdFname[19] = "";
  m_StdFname[20] = "ChipCurr";
  m_StdFname[21] = "in_time_thres";
  m_StdFname[22] = "StuckPixel";

  // cut panel genral options
  m_CutCreateRoot= false;
  m_CutIgnoreBad = true;
  m_CutWebCopy   = false;
  m_CutWebRemove = false; // not used anymore
  m_CutWebPath   = "";
  m_CutWebUser   = "";
  m_CutWebCmd    = "";

  // histogram names/titles for automated plotting
  for(i=0;i<DSET;i++){
    hinam[i] = "";
    hitit[i] = "";
    grtit[i] = "";
    axtit[i] = "";
  }
  hinam[RAW] = "hitproj";
  hinam[MEAN]= "thrproj";
  hinam[SIG] = "nseproj";
  hinam[CHI] = "chiproj";
  hinam[T20K]= "totproj";
  hinam[PARA]= "paraproj";
  hinam[PARB]= "parbproj";
  hinam[PARC]= "parcproj";
  hinam[PARD]= "pardproj";
  hinam[PARE]= "pareproj";
  hinam[XTFR]= "xtproj";
  hinam[XTFRNN]= "xtfnproj";
  hinam[XTNOISE]= "xtnproj";
  hinam[NRAT]= "nrproj";
  hinam[NDIF]= "ndproj";
  hinam[NSQDIF]= "nsdproj";
  hinam[TDIF]= "tdproj";
  hinam[TRAT]= "thrproj";
  hinam[LOOSE]="lbmpproj";
  hinam[SHORT]="mbmpproj";
  hinam[CLTOT]="ctotproj";
  hinam[TDACP]="tdacproj";
  hinam[TDACLOG]="tdacproj";
  hinam[LEAK_PLOT]="leakproj";
  hinam[MLPLOT]="mlproj";
  hinam[TIME]= "timproj";
  hinam[DELCAL]= "delcalproj";
  hinam[TIMEDIF]= "timdproj";
  hinam[TCALDIF]= "tcdproj";
  hinam[TWLK_TIME20NS_2D]="t20proj";
  hinam[TWLK_Q20NS_2D]="qt20proj";
  hinam[TWLK_OVERDRV_2D]="overdrproj";
  hinam[TWODMEAN]="twodmeanproj";
  hinam[TWODSIG]="twodsigproj";
  hinam[TWODCHI]="twodschiproj";
  hinam[AVTOT]="avtotproj";
  hinam[SIGTOT]="sigtotproj";
  hinam[TOT_SUMM]="totpeakproj";
  hinam[TOT_FAST]="totpeakproj";
  hinam[CAL_SUMM]="calpeakproj";
  hinam[CAL_FAST]="calpeakproj";
  hinam[TOT_MAX]="totmaxproj";
  hinam[DEAD_CHAN]="deadchanproj";
  hinam[TOT_RATE]="hitratio";
  hinam[TWODMINT]="minthresh";
  hinam[TWODMINTN]="minthrnoise";
  hinam[TWODMINTC]="minthrchi2";
  hinam[MINTDIF]= "tmdproj";
  hinam[MINTRAT]= "tmrproj";
  hinam[GDIF]= "gdproj";
  hinam[GRAT]= "grproj";
  hinam[GSQDIF]= "gsqdproj";

  hitit[RAW]= "Nhits distribution";
  hitit[MEAN]="Threshold distribution";
  hitit[SIG]= "Noise distribution";
  hitit[CHI]= "#chi^{2} distribution";
  hitit[PARA]= "Distribution of fit-P_{0}";
  hitit[PARB]= "Distribution of fit-P_{1}";
  hitit[PARC]= "Distribution of fit-P_{2}";
  hitit[PARD]= "Distribution of fit-P_{3}";
  hitit[PARE]= "Distribution of fit-P_{4}";
  hitit[XTFR]="Cross-talk distribution";
  hitit[XTFRNN]="Cross-talk distribution";
  hitit[XTNOISE]="Cross-talk noise distribution";
  hitit[NRAT]="Noise ratio distribution";
  hitit[NDIF]="Noise difference distribution";
  hitit[NSQDIF]="Noise quadratic difference distribution";
  hitit[TDIF]="Threshold difference distribution";
  hitit[TRAT]="Threshold ratio distribution";
  hitit[LOOSE]="Noise diff. (unconnected bumps) distribution";
  hitit[SHORT]="Cross-talk (merged bumps) distribution";
  hitit[T20K]="ToT (Q_{target}) distribution";
  hitit[CLTOT]="calib. ToT distribution";
  hitit[TDACP]="DAC distribution";
  hitit[TDACLOG]="DAC distribution";
  hitit[LEAK_PLOT]="Leakage current distribution";
  hitit[MLPLOT]="MonLeak distribution";
  hitit[TIME]="Delay distribution";
  hitit[DELCAL]="Delay scale factor distribution";
  hitit[TIMEDIF]="Delay difference distribution";
  hitit[TCALDIF]="Delay difference distribution";
  hitit[TWLK_TIME20NS_2D]="t_{0}-#Delta t distribution";
  hitit[TWLK_Q20NS_2D]="Q(t_{0}-#Delta t) distribution";
  hitit[TWLK_OVERDRV_2D]="Overdrive distribution";
  hitit[IVPLOT]="(-1)*Bias Current (#muA)";
  hitit[IVCORR]="(-1)*Bias Current (#muA) - T-corr.";
  hitit[TWLKGR]="Time (ns)";
  hitit[TWODMEAN]="Mean distribution";
  hitit[TWODSIG]= "Sigma distribution";
  hitit[TWODCHI]= "#chi^{2} distribution";
  hitit[AVTOT]="Average ToT distribution";
  hitit[SIGTOT]="#sigma(ToT) distribution";
  hitit[TOT_SUMM]="Position of source peak (ToT)";
  hitit[TOT_FAST]="Position of source peak (ToT)";
  hitit[CAL_SUMM]="Position of source peak (e from ToT)";
  hitit[CAL_FAST]="Position of source peak (e from ToT)";
  hitit[TOT_MAX]="Maximum ToT from spectrum";
  hitit[DEAD_CHAN]="Dead channels (source)";
  hitit[TOT_RATE]="Ratio of hits to neighbours";
  hitit[TWODMINT]="Minimum threshold (e)";
  hitit[TWODMINTN]="Noise at min. threshold (e)";
  hitit[TWODMINTC]="Chi^2 at min. threshold";
  hitit[MINTDIF]= "Diff. min. threshold (e)";
  hitit[MINTRAT]= "Ratio min. threshold (e)";
  hitit[GDIF]  = "Difference";
  hitit[GRAT]  = "Ratio";
  hitit[GSQDIF]= "Quadratic difference";
  
  grtit[RAW]= "Nhits scatter plot";
  grtit[MEAN]="Threshold scatter plot";
  grtit[SIG]= "Noise scatter plot";
  grtit[CHI]= "#chi^{2} scatter plot";
  grtit[PARA]= "Fit-P_{0}";
  grtit[PARB]= "Fit-P_{1}";
  grtit[PARC]= "Fit-P_{2}";
  grtit[PARD]= "Fit-P_{3}";
  grtit[PARE]= "Fit-P_{4}";
  grtit[XTFR]="Cross-talk scatter plot";
  grtit[XTFRNN]="Cross-talk scatter plot";
  grtit[XTNOISE]="Cross-talk noise scatter plot";
  grtit[NRAT]="Noise ratio scatter plot";
  grtit[NDIF]="Noise difference scatter plot";
  grtit[NSQDIF]="Noise quadratic difference scatter plot";
  grtit[TDIF]="Threshold difference scatter plot";
  grtit[TRAT]="Threshold ratio scatter plot";
  grtit[LOOSE]="Noise diff. (unconnected bumps) scatter plot";
  grtit[SHORT]="Cross-talk (merged bumps) scatter plot";
  grtit[T20K]="ToT (Q_{target}) scatter plot";
  grtit[CLTOT]="calib. ToT scatter plot";
  grtit[TDACP]="DAC scatter plot";
  grtit[TDACLOG]="DAC scatter plot";
  grtit[LEAK_PLOT]="Leakage current scatter plot";
  grtit[MLPLOT]="MonLeak scatter plot";
  grtit[TIME]="Delay scatter plot";
  grtit[DELCAL]="Delay scale factor scatter plot";
  grtit[TIMEDIF]="Delay difference scatter plot";
  grtit[TCALDIF]="Delay difference scatter plot";
  grtit[TWLK_TIME20NS_2D]="t_{0}-#Delta t scatter plot";
  grtit[TWLK_Q20NS_2D]="Q(t_{0}-#Delta t) scatter plot";
  grtit[TWLK_OVERDRV_2D]="Overdrive scatter plot";
  grtit[IVPLOT]="IV-curve";
  grtit[IVCORR]="T-corrected IV-curve";
  grtit[TWLKGR]="Timewalk";
  grtit[TWODMEAN]="Mean scatter plot";
  grtit[TWODSIG]= "Sigma scatter plot";
  grtit[TWODCHI]= "#chi^{2} scatter plot";
  grtit[AVTOT]="Average ToT scatter plot";
  grtit[SIGTOT]="#sigma(ToT) scatter plot";
  grtit[TOT_SUMM]="Source peak scatter plot";
  grtit[TOT_FAST]="Source peak scatter plot";
  grtit[CAL_SUMM]="Cal. ource peak scatter plot";
  grtit[CAL_FAST]="Cal. source peak scatter plot";
  grtit[TOT_MAX]="Maximum ToT scatter plot";
  grtit[DEAD_CHAN]="Dead channels (source) scatter plot";
  grtit[TOT_RATE]="Ratio of hits scatter plot";
  grtit[TWODMINT]="Minimum thresh. scatter plot";
  grtit[TWODMINTN]="Noise at min. thresh. scatter plot";
  grtit[TWODMINTC]="Chi^2 at min. thresh. scatter plot";
  grtit[MINTDIF]= "Diff. min. threshold scatter polot";
  grtit[MINTRAT]= "Ratio min. threshold scatter plot";
  grtit[GDIF]  = "Difference scatter plot";
  grtit[GRAT]  = "Ratio scatter plot";
  grtit[GSQDIF]= "Quadratic difference scatter plot";
  
  axtit[RAW]= "Nhits";
  axtit[MEAN]="Threshold (e)";
  axtit[SIG]= "Noise (e)";
  axtit[CHI]= "S-curve fit #chi^{2}";
  axtit[PARA]= "Fit-P_{0}";
  axtit[PARB]= "Fit-P_{1}";
  axtit[PARC]= "Fit-P_{2}";
  axtit[PARD]= "Fit-P_{3}";
  axtit[PARE]= "Fit-P_{4}";
  axtit[XTFR]="Cross-talk (per cent)";
  axtit[XTFRNN]="Cross-talk (per cent)";
  axtit[XTNOISE]="Cross-talk noise (per cent)";
  axtit[NRAT]="Noise ratio";
  axtit[NDIF]="Noise difference";
  axtit[NSQDIF]="Noise quadratic difference";
  axtit[TDIF]="Threshold difference";
  axtit[TRAT]="Threshold ratio";
  axtit[LOOSE]="Noise difference (disc. bumps)";
  axtit[SHORT]="Cross-talk (merged bumps)";
  axtit[T20K]="ToT (Q_{target})";
  axtit[CLTOT]="Charge from ToT (e)";
  axtit[TDACP]="DAC";
  axtit[TDACLOG]="DAC";
  axtit[LEAK_PLOT]="Leakage current (nA)";
  axtit[MLPLOT]="MonLeak current (nA)";
  axtit[TIME]="Delay (MCC cnts)";
  axtit[DELCAL]="Delay scale factor (ns/cnts)";
  axtit[TIMEDIF]="Delay difference (MCC cnts)";
  axtit[TCALDIF]="Delay difference (ns)";
  axtit[TWLK_TIME20NS_2D]="t_{0}-#Delta t";
  axtit[TWLK_Q20NS_2D]="Q(t_{0}-#Delta t) (e)";
  axtit[TWLK_OVERDRV_2D]="Overdrive (e)";
  axtit[IVPLOT]="(-1)*Bias Voltage (V)";
  axtit[IVCORR]="(-1)*Bias Voltage (V)";
  axtit[TWLKGR]="Charge(e)";
  axtit[TWODMEAN]="S-curve mean";
  axtit[TWODSIG]= "S-curve sigma";
  axtit[TWODCHI]= "S-curve fit #chi^{2}";
  axtit[AVTOT]="Average ToT";
  axtit[SIGTOT]="#sigma(ToT)";
  axtit[TOT_SUMM]="Source peak (ToT)";
  axtit[TOT_FAST]="Source peak (ToT)";
  axtit[CAL_SUMM]="Source peak (e from ToT)";
  axtit[CAL_FAST]="Source peak (e from ToT)";
  axtit[TOT_MAX]="Maximum ToT from spectrum";
  axtit[TDTDIF]="Threshold difference";
  axtit[TDTRAT]="Threshold ratio";
  axtit[TDNDIF]="Noise difference";
  axtit[TDNRAT]="Noise ratio";
  axtit[DEAD_CHAN]="Dead channels";
  axtit[TOT_RATE]="Ratio of hits to neighbours";
  axtit[TWODMINT]="Minimum threshold (e)";
  axtit[TWODMINTN]="Noise at min. threshold (e)";
  axtit[TWODMINTC]="Chi^2 at min. threshold";
  axtit[MINTDIF]= "Diff. min. threshold (e)";
  axtit[MINTRAT]= "Ratio min. threshold (e)";
  axtit[GDIF]  = "Difference";
  axtit[GRAT]  = "Ratio";
  axtit[GSQDIF]= "Quadratic difference";
  
//  m_summode = 5; //threshold scan internal!
}

optClass::~optClass(){
}

void optClass::Write(FILE *outfile){
  fprintf(outfile,"SCmode: %d\n",(int)m_SCmode);
  fprintf(outfile,"MAFVERSION %s\n",PLGUI_VERSION);
  fprintf(outfile,"splitnsp: %d\n",(int)m_splitnsp);
  fprintf(outfile,"splitthr: %d\n",(int)m_splitthr);
  fprintf(outfile,"showwarn: %d\n",(int)m_showwarn);
  fprintf(outfile,"plotfitopt: %d\n",m_plotfitopt);
  fprintf(outfile,"showchipid: %d\n",m_showchipid);
  fprintf(outfile,"showAxisLabels: %d\n",(int)m_showAxisLabels);
  fprintf(outfile,"projlogy: %d\n",m_projlogy);
  fprintf(outfile,"hitcut: %f\n",m_hitcut);
  fprintf(outfile,"totcut: %f\n",m_totcut);
  fprintf(outfile,"masklong: %d\n",m_masklong);
  fprintf(outfile,"maskgang: %d\n",m_maskgang);
  fprintf(outfile,"diffcut: %f\n",m_diffcut);
  fprintf(outfile,"noisecut: %f\n",m_noisecut);
  fprintf(outfile,"showcutpix: %d\n",(int)m_showcutpix);
  fprintf(outfile,"xtfrcut: %f\n",m_xtfrcut);
  fprintf(outfile,"xtnoisecut: %f\n",m_xtnoisecut);
  fprintf(outfile,"noisetab: %d\n",m_noisetab);
  fprintf(outfile,"zerotab: %d\n",m_zerotab);
  fprintf(outfile,"MCCdelay: %d\n",m_MCCdelay);
  fprintf(outfile,"QhighTW : %d\n",m_QhighTW );
  fprintf(outfile,"ChipTarget: %f\n",m_ChipTarget);
  fprintf(outfile,"doChipTarget: %d\n",m_doChipTarget);
  fprintf(outfile,"doFixedTarget: %d\n",m_doFixedTarget);
  fprintf(outfile,"rec2DFit: %d\n",(int)m_rec2DFit);
  fprintf(outfile,"keep1d: %d\n",(int)m_keep1d);
  fprintf(outfile,"DACdoFit: %d\n",(int)m_DACdoFit);
  fprintf(outfile,"FitGauss: %d\n",m_FitGauss);
  fprintf(outfile,"ToggleMask: %d\n",m_ToggleMask);
  fprintf(outfile,"ToTPlotPnt: %d\n",m_ToTPlotPnt);
  fprintf(outfile,"fixedToTSwitch: %d\n",(int)m_fixedToTSwitch);
  fprintf(outfile,"ToTSwitch: %d\n",m_ToTSwitch);
  fprintf(outfile,"QPlotPnt: %d\n",m_QPlotPnt);
  fprintf(outfile,"UsrMin: %f\n",m_UsrMin);
  fprintf(outfile,"UsrMax: %f\n",m_UsrMax);
  fprintf(outfile,"doUsrRg: %d\n",(int)m_doUsrRg);
  fprintf(outfile,"doTotRelcal: %d\n",(int)m_doTotRelcal);
  fprintf(outfile,"minToT: %d\n",m_minToT);
  fprintf(outfile,"peakMinHit: %d\n",m_peakMinHit);
  fprintf(outfile,"colourScale: %d\n",m_colourScale);
  fprintf(outfile,"doBlackBg: %d\n",(int)m_doBlackBg);
  fprintf(outfile,"DDchan: %d\n",m_DDchan);
  fprintf(outfile,"DDAchan: %d\n",m_DDAchan);
  fprintf(outfile,"BIASchan: %d\n",m_BIASchan);
  fprintf(outfile,"DDsupp: %d\n",m_DDsupp);
  fprintf(outfile,"DDAsupp: %d\n",m_DDAsupp);
  fprintf(outfile,"BIASsupp: %d\n",m_BIASsupp);
  fprintf(outfile,"DefDataPath: %s\n",m_DefDataPath.c_str());
  fprintf(outfile,"UseMname: %d\n",(int)m_UseMname);
  fprintf(outfile,"DefMacroPath: %s\n",m_DefMacroPath.c_str());
  fprintf(outfile,"DefCutPaths: %s\n",m_DefCutPaths.c_str());
  fprintf(outfile,"showChipCuts: %d\n",(int)m_showChipCuts);
  fprintf(outfile,"pdbpath: %s\n",m_pdbpath.c_str());
  fprintf(outfile,"pdbplot: %d\n",(int)m_pdbPlots);
  fprintf(outfile,"pdbrem: %d\n",(int)m_pdbRemove);
  fprintf(outfile,"pdbclr: %d\n",(int)m_pdbClear);
  fprintf(outfile,"SrcMin: %f\n", m_srcMin);
  fprintf(outfile,"SrcRestrict: %d\n", (int)m_srcRestrict);
  fprintf(outfile,"SrcRcPeak: %d\n", (int)m_srcRecalcPeak);
  fprintf(outfile,"VbiasMin: %d\n",m_VbiasMin);
  fprintf(outfile,"VbiasMax: %d\n",m_VbiasMax);
  fprintf(outfile,"IbiasCorr: %d\n",(int)m_IbiasCorr);
  fprintf(outfile,"RunCombToTfit: %d\n",(int)m_runCombToTfit);
  fprintf(outfile,"RunBadFitsIntime: %d\n",(int)m_runBadFitsIntime);
  fprintf(outfile,"RunBadFitsTzero: %d\n",(int)m_runBadFitsTzero);
  fprintf(outfile,"RunBadFitsThresh: %d\n",(int)m_runBadFitsThresh);
  fprintf(outfile,"RunBadFitsXtalk: %d\n",(int)m_runBadFitsXtalk);
  fprintf(outfile,"chiCutmT: %f\n",m_chiCutmT);
  fprintf(outfile,"nseCutmT: %d\n",m_nseCutmT);
  fprintf(outfile,"nseCutmT_long: %d\n",m_nseCutmT_long);
  fprintf(outfile,"nseCutmT_ganged: %d\n",m_nseCutmT_ganged);
  fprintf(outfile,"nseCutmT_inter: %d\n",m_nseCutmT_inter);
  fprintf(outfile,"absValIV: %d\n",(int)m_absValIV);
  fprintf(outfile,"ambushStart: %d\n",m_ambushStart);
  fprintf(outfile,"ambushStop: %d\n",m_ambushStop );

  int i;
  for(i=0;i<DSET;i++){
    if(m_UsrIMin[i]!=-9999 || m_UsrIMax[i]!=-9999){
      fprintf(outfile,"UsrIMin: %d %f\n",i,m_UsrIMin[i]);
      fprintf(outfile,"UsrIMax: %d %f\n",i,m_UsrIMax[i]);
      fprintf(outfile,"UsrIUse: %d %d\n",i,(int)m_UsrIUse[i]);
    }
  }
  for(i=0;i<NPLHIMAX;i++){
    if(m_PLhistoExists[i]){
      fprintf(outfile,"PLUsrIMin: %d %f\n",i,m_PLUsrIMin[i]);
      fprintf(outfile,"PLUsrIMax: %d %f\n",i,m_PLUsrIMax[i]);
      fprintf(outfile,"PLUsrIUse: %d %d\n",i,(int)m_PLUsrIUse[i]);
      fprintf(outfile,"PLPltZero: %d %d\n",i,(int)m_PLPltZero[i]);
      fprintf(outfile,"PLplotType: %d %d\n",i,m_PLplotType[i]);
    }
  }
  for(i=0;i<(int)m_StdFname.size();i++){
    if(m_StdFname[i]!="")
      fprintf(outfile,"StdFname: %d %s\n",i,m_StdFname[i].c_str());
    else
      fprintf(outfile,"StdFname: %d NONE\n",i);
  }
  fprintf(outfile,"CutCreateRoot: %d\n",(int)m_CutCreateRoot);
  fprintf(outfile,"CutIgnoreBad: %d\n",(int)m_CutIgnoreBad);
  fprintf(outfile,"CutWebCopy: %d\n",(int)m_CutWebCopy);
  //fprintf(outfile,"CutWebRemove: %d\n",(int)m_CutWebRemove);  -- obsolete
  fprintf(outfile,"CutWebPath: %s\n",m_CutWebPath.c_str());
  fprintf(outfile,"CutWebUser: %s\n",m_CutWebUser.c_str());
  fprintf(outfile,"CutWebCmd: %s\n",m_CutWebCmd.c_str());
  fprintf(outfile,"userPDB: %s\n",m_userPDB.c_str());
  // encode PDB password
  Q3CString cpasswd = KCodecs::base64Encode(Q3CString(m_passwdPDB.c_str()));
  fprintf(outfile,"passwdPDB: %s\n",(const char *)cpasswd);
  fprintf(outfile,"webUploadOpt: %d\n", m_webUploadOpt);

  fprintf(outfile,"delT0_twalk: %d\n", m_delT0_twalk);

  fprintf(outfile,"totcheckVar: %d\n", m_totcheckVar);
  fprintf(outfile,"totcheckValue: %f\n", m_totcheckValue);
  fprintf(outfile,"totcheckKeep: %d\n", (int)m_totcheckKeep);
  fprintf(outfile,"corPlotOpt: %s\n", m_corPlotOpt.c_str());
  fprintf(outfile,"occPlotHisto: %d\n",(int)m_occPlotHisto);

  return;
}

void optClass::Read(FILE *infile){
  char line[2001], version[100], *tmpc;
  int tmpint,i;
  std::string sline;
  sprintf(version,"1.0.0");
  for (i=0;i<10000000 && (tmpc=fgets(line,2000,infile))!=NULL;i++){
    sline = line;
    if(sline.find("MAFVERSION")!= std::string::npos){
      //      sscanf(line,"MAFVERSION %s",version);
      QString verqs = line;
      //remove trailing "\n" and leading MAFVERSION
      verqs.remove(verqs.length()-1,1);
      verqs=verqs.mid(11,-1);
      if(verqs.left(8)=="USBPixI4"){ // add 10 to main version no. to distiguish from non-PixLib versions
	int vmain, vmid,vend=0;
	char isTrunk[10];
	sscanf(verqs.mid(9,-1).latin1(), "%d.%d.%s", &vmain, &vmid, isTrunk);
	vmain+=10;
	if(strcmp(isTrunk,"trunk")==0) vend=1;
	sprintf(version,"%d.%d.%d", vmain, vmid, vend);
      } else
	sprintf(version,"%s",verqs.latin1());
    }

    if(sline.find("SCmode")!= std::string::npos){
      sscanf(line,"SCmode: %d",&tmpint);
      m_SCmode = (bool) tmpint;
    }
    if(sline.find("splitnsp")!= std::string::npos){
      sscanf(line,"splitnsp: %d",&tmpint);
      m_splitnsp = (bool) tmpint;
    }
    if(sline.find("splitthr")!= std::string::npos){
      sscanf(line,"splitthr: %d",&tmpint);
      m_splitthr = (bool) tmpint;
    }
    if(sline.find("showwarn")!=std::string::npos){
      sscanf(line,"showwarn: %d",&tmpint);
      m_showwarn = (bool) tmpint;
    }
    if(sline.find("plotfitopt")!=std::string::npos)
      sscanf(line,"plotfitopt: %d",&m_plotfitopt);
    if(sline.find("showchipid")!=std::string::npos)
      sscanf(line,"showchipid: %d",&m_showchipid);
    if(sline.find("showAxisLabels")!=std::string::npos){
      sscanf(line,"showAxisLabels: %d",&tmpint);
      m_showAxisLabels = (bool) tmpint;
    }
    if(sline.find("projlogy")!=std::string::npos)
      sscanf(line,"projlogy: %d",&m_projlogy);
    if(sline.find("hitcut")!=std::string::npos)
      sscanf(line,"hitcut: %f",&m_hitcut);
    if(sline.find("totcut")!=std::string::npos)
      sscanf(line,"totcut: %f",&m_totcut);
    if(sline.find("masklong")!=std::string::npos)
      sscanf(line,"masklong: %d",&m_masklong);
    if(sline.find("maskgang")!=std::string::npos)
      sscanf(line,"maskgang: %d",&m_maskgang);
    if(sline.find("diffcut")!=std::string::npos)
      sscanf(line,"diffcut: %f",&m_diffcut);
    if(sline.find("noisecut")!=std::string::npos)
      sscanf(line,"noisecut: %f",&m_noisecut);
    if(sline.find("showcutpix")!=std::string::npos){
      sscanf(line,"showcutpix: %d",&tmpint);
      m_showcutpix = (bool) tmpint;
    }
    if(sline.find("xtfrcut")!=std::string::npos)
      sscanf(line,"xtfrcut: %f",&m_xtfrcut);
    if(sline.find("xtnoisecut")!=std::string::npos)
      sscanf(line,"xtnoisecut: %f",&m_xtnoisecut);
    if(sline.find("noisetab")!=std::string::npos)
      sscanf(line,"noisetab: %d",&m_noisetab);
    if(sline.find("zerotab")!=std::string::npos)
      sscanf(line,"zerotab: %d",&m_zerotab);
    if(sline.find("MCCdelay")!=std::string::npos)
      sscanf(line,"MCCdelay: %d",&m_MCCdelay);
    if(sline.find("QhighTW ")!=std::string::npos)
      sscanf(line,"QhighTW : %d",&m_QhighTW );
    if(sline.find("ChipTarget")!=std::string::npos)
      sscanf(line,"ChipTarget: %f",&m_ChipTarget);
    if(sline.find("doChipTarget")!=std::string::npos)
      sscanf(line,"doChipTarget: %d",&m_doChipTarget);
    if(sline.find("doFixedTarget")!=std::string::npos)
      sscanf(line,"doFixedTarget: %d",&m_doFixedTarget);
    if(sline.find("rec2DFit")!=std::string::npos){
      sscanf(line,"rec2DFit: %d",&tmpint);
      m_rec2DFit = (bool) tmpint;
    }
    if(sline.find("keep1d")!=std::string::npos){
      sscanf(line,"keep1d: %d",&tmpint);
      m_keep1d = (bool) tmpint;
    }
    if(sline.find("DACdoFit")!=std::string::npos){
      sscanf(line,"DACdoFit: %d",&tmpint);
      m_DACdoFit = (bool) tmpint;
    }
    if(sline.find("FitGauss")!=std::string::npos)
      sscanf(line,"FitGauss: %d",&m_FitGauss);
    if(sline.find("ToggleMask")!=std::string::npos)
      sscanf(line,"ToggleMask: %d",&m_ToggleMask);
    if(sline.find("ToTPlotPnt")!=std::string::npos)
      sscanf(line,"ToTPlotPnt: %d",&m_ToTPlotPnt);
    if(sline.find("fixedToTSwitch")!=std::string::npos){
      sscanf(line,"fixedToTSwitch: %d",&tmpint);
      m_fixedToTSwitch = (bool) tmpint;
    }
    if(sline.find("ToTSwitch")!=std::string::npos)
      sscanf(line,"ToTSwitch: %d",&m_ToTSwitch);
    if(sline.find("QPlotPnt")!=std::string::npos)
      sscanf(line,"QPlotPnt: %d",&m_QPlotPnt);
    if(sline.find("UsrMin")!=std::string::npos)
      sscanf(line,"UsrMin: %f",&m_UsrMin);
    if(sline.find("UsrMax")!=std::string::npos)
      sscanf(line,"UsrMax: %f",&m_UsrMax);
    if(sline.find("doUsrRg")!=std::string::npos){
      sscanf(line,"doUsrRg: %d",&tmpint);
      m_doUsrRg = (bool) tmpint;
    }
    if(sline.find("doTotRelcal")!=std::string::npos){
      sscanf(line,"doTotRelcal: %d",&tmpint);
      m_doTotRelcal = (bool) tmpint;
    }
    if(sline.find("RunCombToTfit")!=std::string::npos){
      sscanf(line,"RunCombToTfit: %d",&tmpint);
      m_runCombToTfit = (bool) tmpint;
    }
    if(sline.find("RunBadFitsIntime")!=std::string::npos){
      sscanf(line,"RunBadFitsIntime: %d",&tmpint);
      m_runBadFitsIntime = (bool) tmpint;
    }
    if(sline.find("RunBadFitsTzero")!=std::string::npos){
      sscanf(line,"RunBadFitsTzero: %d",&tmpint);
      m_runBadFitsTzero = (bool) tmpint;
    }
    if(sline.find("RunBadFitsThresh")!=std::string::npos){
      sscanf(line,"RunBadFitsThresh: %d",&tmpint);
      m_runBadFitsThresh = (bool) tmpint;
    }
    if(sline.find("RunBadFitsXtalk")!=std::string::npos){
      sscanf(line,"RunBadFitsXtalk: %d",&tmpint);
      m_runBadFitsXtalk = (bool) tmpint;
    }
    if(sline.find("minToT")!=std::string::npos)
      sscanf(line,"minToT: %d",&m_minToT);
    if(sline.find("peakMinHit")!=std::string::npos)
      sscanf(line,"peakMinHit: %d",&m_peakMinHit);
    if(sline.find("colourScale")!=std::string::npos)
      sscanf(line,"colourScale: %d",&m_colourScale);
    if(sline.find("doBlackBg")!=std::string::npos){
      sscanf(line,"doBlackBg: %d",&tmpint);
      m_doBlackBg = (bool) tmpint;
    }
    if(sline.find("VbiasMin")!=std::string::npos)
      sscanf(line,"VbiasMin: %d",&m_VbiasMin);
    if(sline.find("VbiasMax")!=std::string::npos)
      sscanf(line,"VbiasMax: %d",&m_VbiasMax);
    if(sline.find("IbiasCorr")!=std::string::npos){
      sscanf(line,"IbiasCorr: %d",&tmpint);
      m_IbiasCorr = (bool) tmpint;
    }
    if(sline.find("DDchan")!=std::string::npos)
      sscanf(line,"DDchan: %d",&m_DDchan);
    if(sline.find("DDAchan")!=std::string::npos)
      sscanf(line,"DDAchan: %d",&m_DDAchan);
    if(sline.find("BIASchan")!=std::string::npos)
      sscanf(line,"BIASchan: %d",&m_BIASchan);
    if(sline.find("DDsupp")!=std::string::npos)
      sscanf(line,"DDsupp: %d",&m_DDsupp);
    if(sline.find("DDAsupp")!=std::string::npos)
      sscanf(line,"DDAsupp: %d",&m_DDAsupp);
    if(sline.find("BIASsupp")!=std::string::npos)
      sscanf(line,"BIASsupp: %d",&m_BIASsupp);

    if(sline.find("absValIV")!=std::string::npos){
      sscanf(line,"absValIV: %d",&tmpint);
      m_absValIV = (bool) tmpint;
    }
    if(sline.find("ambushStart")!=std::string::npos)
      sscanf(line,"ambushStart: %d",&m_ambushStart);
    if(sline.find("ambushStop")!=std::string::npos)
      sscanf(line,"ambushStop: %d",&m_ambushStop);

    if(sline.find("chiCutmT")!=std::string::npos)
      sscanf(line,"chiCutmT: %f",&m_chiCutmT);

    if(sline.find("nseCutmT")!=std::string::npos){
      sscanf(line,"nseCutmT: %d",&m_nseCutmT);
      if(DatSet::OlderVersion(version,"5.2.8")){
	m_nseCutmT_long   = m_nseCutmT;
	m_nseCutmT_ganged = m_nseCutmT;
	m_nseCutmT_inter  = m_nseCutmT;
      }
    }
    if(sline.find("nseCutmT_long")!=std::string::npos)
      sscanf(line,"nseCutmT_long: %d",&m_nseCutmT_long);
    if(sline.find("nseCutmT_ganged")!=std::string::npos)
      sscanf(line,"nseCutmT_ganged: %d",&m_nseCutmT_ganged);
    if(sline.find("nseCutmT_inter")!=std::string::npos)
      sscanf(line,"nseCutmT_inter: %d",&m_nseCutmT_inter);

    if(sline.find("DefDataPath")!=std::string::npos){
      sline.erase(0,13);
      sline.erase(sline.length()-1,1);
      m_DefDataPath = sline;
    }
    if(sline.find("DefMacroPath")!=std::string::npos){
      sline.erase(0,14);
      sline.erase(sline.length()-1,1);
      m_DefMacroPath = sline;
    }
    if(sline.find("DefCutPaths")!=std::string::npos){
      sline.erase(0,13);
      sline.erase(sline.length()-1,1);
      m_DefCutPaths = sline;
    }
    if(sline.find("showChipCuts")!=std::string::npos){
      sscanf(line,"showChipCuts: %d",&tmpint);
      m_showChipCuts = (bool) tmpint;
    }
    if(sline.find("pdbpath")!=std::string::npos){
      sline.erase(0,9);
      sline.erase(sline.length()-1,1);
      m_pdbpath = sline;
    }
    if(sline.find("pdbplot")!=std::string::npos){
      sscanf(line,"pdbplot: %d",&tmpint);
      m_pdbPlots = (bool) tmpint;
    }
    if(sline.find("pdbrem")!=std::string::npos){
      sscanf(line,"pdbrem: %d",&tmpint);
      m_pdbRemove = (bool) tmpint;
    }
    if(sline.find("pdbclr")!=std::string::npos){
      sscanf(line,"pdbclr: %d",&tmpint);
      m_pdbClear = (bool) tmpint;
    }
    if(sline.find("SrcMin")!=std::string::npos)
      sscanf(line,"SrcMin: %f",&m_srcMin);
    if(sline.find("SrcRestrict")!=std::string::npos){
      sscanf(line,"SrcRestrict: %d",&tmpint);
      m_srcRestrict = (bool) tmpint;
    }
    if(sline.find("SrcRcPeak")!=std::string::npos){
      sscanf(line,"SrcRcPeak: %d",&tmpint);
      m_srcRecalcPeak = (bool) tmpint;
    }
    if(sline.find("CfgUsrdefNames")!=std::string::npos){
      sscanf(line,"CfgUsrdefNames: %d",&tmpint);
      m_cfgUsrdefNames = (bool) tmpint;
    }

    if(sline.find("UseMname")!=std::string::npos){
      sscanf(line,"UseMname: %d",&tmpint);
      m_UseMname = (bool) tmpint;
    }


    int id;
    float tmpfl;
    if(sline.find("UsrIMin")!=std::string::npos && sline.find("PLUsrIMin")==std::string::npos){
      sscanf(line,"UsrIMin: %d %f",&id,&tmpfl);
      if(id>LOAD && id<DSET)
	m_UsrIMin[id] = tmpfl;
    }
    if(sline.find("UsrIMax")!=std::string::npos && sline.find("PLUsrIMax")==std::string::npos){
      sscanf(line,"UsrIMax: %d %f",&id,&tmpfl);
      if(id>LOAD && id<DSET)
	m_UsrIMax[id] = tmpfl;
    }
    if(sline.find("UsrIUse")!=std::string::npos && sline.find("PLUsrIUse")==std::string::npos){
      sscanf(line,"UsrIUse: %d %d\n",&id,&tmpint);
      if(id>LOAD && id<DSET)
	m_UsrIUse[id] = (bool)tmpint;
    }

    if(sline.find("PLUsrIMin")!=std::string::npos){
      sscanf(line,"PLUsrIMin: %d %f",&id,&tmpfl);
      if(id>=0 && id<NPLHIMAX && m_PLhistoExists[id])
	m_PLUsrIMin[id] = tmpfl;
    }
    if(sline.find("PLUsrIMax")!=std::string::npos){
      sscanf(line,"PLUsrIMax: %d %f",&id,&tmpfl);
      if(id>=0 && id<NPLHIMAX && m_PLhistoExists[id])
	m_PLUsrIMax[id] = tmpfl;
    }
    if(sline.find("PLplotType")!=std::string::npos){
      sscanf(line,"PLplotType: %d %d\n",&id,&tmpint);
      if(id>=0 && id<NPLHIMAX && m_PLhistoExists[id]  && !DatSet::OlderVersion(version,"11.2.1"))
	m_PLplotType[id] = tmpint;
    }
    if(sline.find("PLUsrIUse")!=std::string::npos){
      sscanf(line,"PLUsrIUse: %d %d\n",&id,&tmpint);
      if(id>=0 && id<NPLHIMAX && m_PLhistoExists[id])
	m_PLUsrIUse[id] = (bool)tmpint;
    }
    if(sline.find("PLPltZero")!=std::string::npos){
      sscanf(line,"PLPltZero: %d %d\n",&id,&tmpint);
      if(id>=0 && id<NPLHIMAX && m_PLhistoExists[id])
	m_PLPltZero[id] = (bool)tmpint;
    }

    if(sline.find("StdFname")!=std::string::npos){
      sscanf(line,"StdFname: %d %s\n",&id,line);
      if(id>=0 && id<(int)m_StdFname.size())
	m_StdFname[id] = line;
      if(m_StdFname[id]=="NONE") m_StdFname[id] = "";
    }

    if(sline.find("CutCreateRoot")!=std::string::npos){
      sscanf(line,"CutCreateRoot: %d",&tmpint);
      m_CutCreateRoot = (bool) tmpint;
    }
    if(sline.find("CutIgnoreBad")!=std::string::npos){
      sscanf(line,"CutIgnoreBad: %d",&tmpint);
      m_CutIgnoreBad = (bool) tmpint;
    }
    if(sline.find("CutWebCopy")!=std::string::npos){
      sscanf(line,"CutWebCopy: %d",&tmpint);
      m_CutWebCopy = (bool) tmpint;
    }
    //    if(sline.find("CutWebRemove")!=std::string::npos){
    //      sscanf(line,"CutWebRemove: %d",&tmpint);
    //      m_CutWebRemove = (bool) tmpint;
    //    }  -- obsolete
    if(sline.find("CutWebPath")!=std::string::npos){
      sline.erase(0,12);
      sline.erase(sline.length()-1,1);
      m_CutWebPath = sline;
    }
    if(sline.find("CutWebUser")!=std::string::npos){
      sline.erase(0,12);
      sline.erase(sline.length()-1,1);
      m_CutWebUser = sline;
    }
    if(sline.find("CutWebCmd")!=std::string::npos){
      sline.erase(0,11);
      sline.erase(sline.length()-1,1);
      m_CutWebCmd = sline;
    }
    if(sline.find("userPDB")!=std::string::npos){
      sline.erase(0,9);
      sline.erase(sline.length()-1,1);
      m_userPDB = sline;
    }
    // decode PDB password
    if(sline.find("passwdPDB")!=std::string::npos){
      sline.erase(0,11);
      sline.erase(sline.length()-1,1);
      Q3CString cpasswd = KCodecs::base64Decode(Q3CString(sline.c_str()));
      m_passwdPDB = (const char*) cpasswd;
    }
    if(sline.find("webUploadOpt")!=std::string::npos)
      sscanf(line,"webUploadOpt: %d",&m_webUploadOpt);

    if(sline.find("delT0_twalk")!=std::string::npos)
      sscanf(line,"delT0_twalk: %d",&m_delT0_twalk);

    if(sline.find("totcheckVar")!=std::string::npos)
      sscanf(line,"totcheckVar: %d",&m_totcheckVar);
    if(sline.find("totcheckValue")!=std::string::npos)
      sscanf(line,"totcheckValue: %f",&m_totcheckValue);
    if(sline.find("totcheckKeep")!=std::string::npos){
      sscanf(line,"totcheckKeep: %d",&tmpint);
      m_totcheckKeep = (bool) tmpint;
    }

    if(sline.find("corPlotOpt")!=std::string::npos){
      sscanf(line,"corPlotOpt: %s",line);
      m_corPlotOpt = line;
    }
    if(sline.find("occPlotHisto")!=std::string::npos){
      sscanf(line,"occPlotHisto: %d",&tmpint);
      m_occPlotHisto = (bool) tmpint;
    }

  } // end file scanning loop

  return;
}
