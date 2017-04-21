#ifndef MODULE
#define MODULE

#define NPARS          10
#define NRAW            7
#define NPROF           5
#define NGRAPH         15
#define NFITRES         5
#define AVTOT_HISTO     0
#define SIGTOT_HISTO    5
#define HITS_HISTO      1
#define MASK_HISTO      2     
#define DEADPIXS_HISTO  3
#define CALTOT_HISTO    4
#define LEAK_HISTO      0
#define MLORG_HISTO     1
#define DAC_HISTO       3
#define TOTPEAK_HISTO   0
#define TOTPEAK_FAST    1
#define CALPEAK_HISTO   2
#define CALPEAK_FAST    3
#define TOTMAX          4
#define CTOTMAX         5

#define TWLK_TIME20NS_HISTO   0
#define TWLK_Q20NS_HISTO      1
#define TWLK_OVERDRV_HISTO    2

//burn-in graph indexes
#define BI_CHTEMP       0
#define BI_NTC          1
#define BI_DIG_VOLT     2
#define BI_DIG_CURR     3
#define BI_ANALOG_VOLT  4
#define BI_ANALOG_CURR  5
#define BI_BIAS_VOLT    6
#define BI_BIAS_CURR    7
#define BI_THRESHOLD    8
#define BI_NOISE        9
#define BI_GOODFITS    10
#define BI_BADPIXLS    11
#define BI_LOGERR_MCC    12
#define BI_LOGERR_MCC01  13
#define BI_LOGERR_FE     14

#include <string>
#include <vector>
#include <stdio.h>

#ifdef PIXEL_ROD // can link to PixScan
#include <TKey.h>
#include <TSystem.h>
#include <qobject.h>
#include <PixConfDBInterface.h>
#include <RootDB.h>
#include <PixScan.h>
#endif

#include "LogClasses.h"
#include "FileTypes.h"
#include "PixelDataContainer.h"

class TH1F;
class TH2F;
class TGraph;
class TGraphErrors;
class QAReport;
class QString;
#ifdef HAVE_PLDB
namespace PixLib{
 class Histo;
 // class DBInquire;
}
#endif

#ifdef PIXEL_ROD // can link to PixScan

class wrapPs : public PixLib::PixScan{

 public:
  wrapPs();
/*   wrapPs(ScanType presetName); */
/*   wrapPs(PixLib::DBInquire *dbi); */
/*   wrapPs(const wrapPs &scn); */

  //! Destructor
  ~wrapPs();

  int m_nUsed;
};
#endif // PIXEL_ROD

/*! This class provides the data storage in ROOT objects
 *
 * TurboDAQ ascii-files are read upon creation of a DatSet
 * object and stored in ROOT TH2F or TH1F histograms.
 * Data storage is organised dynamically, depending on the
 * type (see FileTypes.h for types) only a certain set of
 * histograms is created and filled
 *
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 * Aldo Saveedra <afsaavedra@lbl.gov>
 * @version 3.7
 */

class DatSet : public PixelDataContainer {

  friend class QAReport;

 public:
  /*! constructor when reading from TurboDAQ ascii files
   *
   */
  DatSet(const char *name, const char *path, int type, int* rderr, const char* cfgpath=NULL, int *extra_pars=NULL);
  /*! constructor when doing analysis on several DatSet items,
   *  producing a new DatSet object; allows to use a set of analysis parameters
   */
  DatSet(const char *name, DatSet **rawdat, int nfiles, int type, int* rderr, 
	 float *anapar, int npar=1);
  /*! constructor when reading from ROOT file
   *
   */
  DatSet(const char *orgpath, int *rderr);
  /*! constructor for purposes of MA controlled by an external application: 
   *  creating empty data, but assuming hit and/or scan info follows.
   *  log is the config data used for the module,
   *  and the scan info: must be filled with scan steps
   */
  DatSet(const char *name, int type, bool isscan, TLogFile *log, int *rderr, const char *path=0);
  /*! constructor for purposes of external data upload:
   *  creating empty data, hit and/or scan info taken from 
   *  histograms (void* vector)
   *  log is the config data used for the module,
   *  and the scan info: must be filled with scan steps
   */
  void genericHistoConstr(int type, bool isscan, TLogFile *log, 
			  std::vector<void*> dataHistos, int histoType, int *rderr);
  /*! similar constructor for user convenience
   */
  DatSet(const char *name, int type, bool isscan, TLogFile *log, 
	 std::vector<TH2F*> dataHistos, int *rderr, const char *path=0);
#ifdef HAVE_PLDB
  /*! similar to above, just allows to load
   *  histograms right away - only works with PixLib
   */
  DatSet(const char *name, int type, bool isscan, TLogFile *log, 
	 std::vector<PixLib::Histo*> dataHistos, int *rderr, const char *path=0);
#endif
  /*! destructor
   */
  ~DatSet();
  /*! ext appl. contructor ctd: then load vector corresponding to a histogram, one per pix
   *  bin centre must be supplied in above contructor (scan info)
   */
  int LoadVector(int chip, int col, int row, std::vector<int> hitvec);
  /*! ext. appl. contructor ctd: then load thresh/noise/chi^2 maps
   *  in case of 2D scan creation
   */
  int Load2DHisto(int scanpoint, TH2F *thresh, TH2F *noise, TH2F *chi);
  /*! ext. appl. contructor ctd: then load hitmap, one entry per pixel
   */
  int LoadHits(int chip, int col, int row, int nhits);
  /*! ext. appl. contructor ctd: load graphs
   */
  int LoadGraph(int gid, std::vector<float> xvec, std::vector<float> yvec);
#ifdef HAVE_PLDB
  /*! ext. appl. contructor ctd: then load thresh/noise/chi^2 maps
   */
  int LoadThrHisto(std::vector<PixLib::Histo *> inHisto);
  /*! ext. appl. contructor ctd: load ToT fit data if appropriate 
   */
  int LoadToTFit(std::vector<PixLib::Histo *> inPars);
  /*! ext. appl. contructor ctd: load ToT data if appropriate 
   */
  int LoadToTHisto(std::vector<PixLib::Histo *> inMean, std::vector<PixLib::Histo *> inSigma);
  /*! for my personal convenience: copy Histo to TH2F
   */
  void CopyHisto(PixLib::Histo &hiIn, TH2F *hiOut);
#endif
  /*! check if "testver" MAF-version is newer than 
   *  "thisver" label
   */
  static bool OlderVersion(const char *testver, const char *thisver);
  /*! check if "testver" MAF-version is newer than 
   *  current version
   */
  static bool OlderVersion(const char *testver){return OlderVersion(testver,VERSION);};
  // misc static functions
  /*! sets the pointer of a FILE to the last line in the file
   */
  static void SetToLastLine(FILE *f);
  /*! checks for exisiting directories and creates them if necessary
   */
  static int  CheckFolder(const char* path);
  /*! processes DAC names
   */
  static void DACNames(const char *org_name, const char *path, std::string &supp_name, std::string &cfg_name);
  /*! turns a stream of 1's and 0's into a
   *  bit-decoded integer value (i.e. 1001 -> 9)
   */
  static unsigned int StringToBit(const char *bitString);
  /*! returns a pointer to the logfile info
   *
   */
  TLogFile*   GetLog() {return m_LogFile;}
  /*! (re-)load log ascii-file
   *
   */ 
  int LoadLog(const char*);
  /*! (re-)load load file object (ie ascii logfile has to be
   *  loaded with TLogFile::ReadoutLog first)
   */
  void LoadLog(TLogFile *in_log){m_LogFile = in_log;};
#ifdef COMPL
  /*! updates logfile V,I,T-type information
   *  from an ambush data set
   */
  void updateLogFromAmbush(DatSet *ambushData, int aSP, int aCH, int dSP, int dCH, int hSP, int hCH);
#endif
  /*! converts DatSet error integers to a message string
   */
  static std::string ErrorToMsg(int error_code);

  /*! reads mask files into TH2F
   * put here temporarily, was private shouldn't be public though
   */
  int  ReadMaskSet(const char *path);
  /*! Loads pixels and CP that are off
   *  into a TH2F object as mask
   */
  int   LoadMask();
  /*! finds channels w/o response or
   *  too little ToT response in source data
   */
  void  FindDeadChannels(int minToT=0);
  /*! generates a TH2F-mask object from
   *  from hot pixel data (ie either too many hits
   *  or too low ToT as given as arguments)
   */
  void  GetHotPix(float hitcut=0, float TOTcut=3000, TH2F* ormask=NULL);
  /*! writes a hot-pixel mask to a
   *  TurboDAQ mask file
   */
  void  WriteHotPix(const char *path, int doinv=0, TH2F *hi=NULL, int chip=-1);
  /*! Sets all ganged and long pixels masked
   */
  void  MaskGLSet(int, int);
  /*! Writes pixels marked in "pixarr"
   *  as masked in a TH2F-mask object in this
   *  data set. "pixarr" should contain chip,col,row
   *  as returned by DatSet::PixXYtoBins
   */
  void  CreateMask(int *pixarr, int nentries);
  /*! Creates a new mask from a map stored as TH2F (as returned
   *  e.g. by DatSet::GetMap); NB: the map can be anything,
   *  anything non-zero is counted as masked pixel
   *  mode==0: straight copy from original
   *  mode==1: non-zero entries copied as entry 1
   *  mode==2: "empty-flagged" entries of -1000 are not ignored
   */
  void  CreateMask(TH2F *inmask, int mode=0);
  /*! Writes the TDACs or FDACs from a TH2F object
   *  to a DatSet owned TH2F
   */
  void  SetDAC(TH2F *hi);
  /*! Writes the TDACs or FDACs from a TH2F object
   *  to a given file in TurboDAQ format 
   */
  void  WriteDAC(const char *path, TH2F *hi=NULL, int chip=-1);
  /*! Writes the contents of the logfile entry
   *  into a TurboDAQ ascii-config file
   */
  int   WriteConfig(const char *fname, const char **auxnames=0, int phase1=-1, int phase2=-1);
  /*! returns the scan level,
   * i.e. 0 for no scan, 1 for 1D, 2 for 2D scan etc
   */
  int getScanLevel(){return m_isscan;};
  /*! returns number of events, i.e. number of injections 
   *  followed by LVL1 trigger(s)
   */
  int getNevents();
  /*! returns number of scan steps on requested loop level
   */
  int getScanSteps(int loopLevel);
  /*! returns scan start value on requested loop level
   */
  float getScanStart(int loopLevel);
  /*! returns scan start value on requested loop level
   */
  float getScanStop (int loopLevel);

  /*! Returns a TH2F-map of type "type" (see FileTypes.h)
   *  if chip>0 than the map for only this chip is returned
   */
  TH2F* GetMap(int chip, int type);
  /*! Returns a TH2F-map of type "type" from a 2D scan
   *  (e.g. TDAC) for the given scan point
   *  if chip>0 than the map for only this chip is returned
   */
  TH2F* Get2DMap(int chip, int type, int scanpt);
  TH2F* GetScanMap(int chip, int col, int row, int type=RAW);
  /*! Returns a graph containing hits vs scan variable
   *  for a 1D scan, or threshold, noise or chi^2 vs outer var.
   *  for a 2D scan
   */
  TGraphErrors* GetScanHi(int chip, int col, int row, int type=TWODMEAN, int scanpt=-1);
  /*! Returns a graph containing ToT vs scan variable (1D only)
   */
  TGraphErrors* GetToTHi(int chip, int col, int row);
  /*! Returns the ToT spectrum e.g. for a source scan
   *  as a summary for a chip (chip==-1 is the module summary)
   */
  TH1F* GetSpecHi(int chip);
  /*! Returns the ToT spectrum e.g. for a source scan
   *  for a given pixel; if calibration is provided
   *  the ToT is calibrated into electrons
   */
  TH1F* GetToTDist(int chip, int col, int row, bool nofit=false, DatSet *ToTcal=NULL);
  /*! Returns the ToT spectrum e.g. for a source scan
   *  for a given pixel; a fit to the spectrum is made
   */
  TH1F* GetToTDist(int chip, int col, int row, bool nofit, DatSet *ToTcal, float *peakpos, int hit_min=-1, float tot_max_diff=-1);
  /*! Returns the LVL1 histogram for a pixel
   */
  TH1F* GetLvl1(int chip, int col, int row, int scanpt=0);
  /*! Returns a map of LVL1 histogram vs scan par. for a pixel
   */
  TH2F* GetLvl1Scan(int chip, int col, int row);
  /*! returns a map with TDAC or FDAC as in the logfile
   */
  TH2F* GetDACfromLog(int type);
  /*! returns a map with mask data (any of the four) as in the logfile
   */
  TH2F* GetMaskfromLog(int type);
  /*! returns a map with a ratio of hits to the top and
   *  bottom neighboring pixel (i.e. on the long edge)
   *  as obtained from the stored hit map (e.g. source data)
   */
  TH2F* GetHitRatio();
  /*! returns a graph as stored in miscrgr array (like IV scan or LV Currents)
   */
  TGraph* GetGraph(int ind=0){return GetGraph(ind,0);};
  TGraph* GetGraph(int ind, int){if(ind<NGRAPH)return m_miscgr[ind];else return NULL;};
  /*! returns a graph with IV scan data, the current having been
   *  temperature corrected using the "standard" formula
   *  BEWARE: only valid for bulk current!
   */
  TGraph* GetCorrIV();
  /*! runs one of the fits defined in the MA structures (variable fittype)
   *  conversion VCAL->charge can be provided as simple factor (to be changed)
   */
  int   FitStd(float **convfac, int fittype=-1, int chip=-1, float chicut=20, float xmin=0, float xmax=0, bool selfit=false, bool useNdof=true);
  int   GetFitPar(const char * funcname, float chicut);
  /*! clears all raw (i.e. hit-vs-variable) scan data
   */
  void  ClearRaw();
  /*! clears all data but summary stuff (that in m_FitResArray)
   */
  int  ClearFit();
  /*! sets the ID referring to one of the standard tests in MA
   */
  void  SetStdtest(int setval){m_stdtest = setval;};
  /*! gets the ID referring to one of the standard tests in MA
   */
  int   GetStdtest(){return m_stdtest;};
  /*! obsolete
   */
  int   ITHAna(float);
  /*!  searches for the optimum TDAC value from TDAC tuning data
   */
  int   TDACAna(int do_per_chip=0, float fixed_target=-1);
  static double CombCal(double ToT, double *parl, double *parh, double smooth=2, double totsw=-1);
  /*! stores DatSet pointer to ToT calibration fit data
   */
  void  CalibrateToT(DatSet *ToTcal);
  /*! runs a combined ToT fit on the DatSet objects
   *  as given in "rawdat". result stored in this DatSet object
   *  (this is of an according data type!)
   */
  int   TotCombFit(int dochip=-1, DatSet **rawdat=NULL, float *anapar=NULL, int npar=0);
  /*! runs a relative Chigh-Clow ToT fit such that both ToT-calibration
   *  data agree best; result returned in array per chip ("relcal")
   */
  int   TotRelCalFit(DatSet **rawdat, float *relcal, float totmin=-1, float totmax=-1);
  /*! get the relative Clow/Chigh ToT calibration
   *  from a run of TotRelCalFit
   */
  void  GetRelCal(float*);
  /*! get the ToT (raw counts or calibrated in electrons)
   *  of the peak in a spectrum
   */
  void  PeakAna(bool dofit=true, DatSet *ToTCal=NULL, int hit_min=-1, float tot_max_diff=-1);
  /*! fills the maximum ToT of a spectrum
   *  into a TH2F map
   */
  void  GetmaxToT(DatSet *ToTCal=NULL);
  /*! (re-)defines temperature of IV scan
   *  as used for correction
   */
  void  CorrectIVTemp(float);
  int   ModifyToTCal(float *calib);
  static void PathBase(std::string& path);
  static int LogPath(std::string &path);
  void  Write();  
  /*! return parameter iPar from analysis
   *  (like timewalk parameters)
   */
  float getAnaPar(int iPar);

#ifdef PIXEL_ROD // can link to PixScan
  /*! get pointer to PixScan config object if there
   */
  wrapPs* getPixScan();
  /*! set pointer to PixScan config object if there
   */
  void setPixScan(wrapPs *in_ps);
#endif

  /*! burn-in scan summary (added 7/5/04 by LT)
   */
  int BI_updateScanSummary(TGraph *thrGr, TGraph *noiseGr, TGraph *nfitsGr, TGraph *digErrGr, 
			   TGraph *mccErrGr, TGraph *mcc01ErrGr, TGraph *feErrGr);
  /*! returns an average value for the time interval specified
   *  from quantities in ambush(-type-)data, like T_NTC
   */
  double BI_getAvgValueOnInterval(int dataType, double time1, double time2);
  int BI_getMeanValue(int dataType, double *mean, double *nfits);


  // public members which may be changed directly by the user
  int   m_istype, m_isscan, m_iscal, m_fittype;
  // relative calibration of Chi and Clow ToT (only used then)
  float *m_ChiCloCal;
  
//********* by JW *****************************************
  void Fill_FitResArray(int type, int index, float value);
  void Increment_FitResArrayCell(int type, int chip);
  float** Get_FitResArray();
  float Get_FitResArrayCell(int type, int chip);
  void Get_FitResArrayMinMax(int type, float &Min, float &Max);
  void Init_FitResArray(int init_value=-1);
//*********************************************************

 private:
  // the ascii-file reading routines
  int  ReadDataSet(const char *path);
  int  ReadBinData(const char *path);
  int  ReadSfitData(const char *path);
  int  ReadSfitData2D(const char *path);
  int  ReadTfitData(const char *path);
  int  ReadTDACSet(const char *path);
  int  ReadTDAC(const char *path, int chip);
  int  ReadTOTData(const char *path);
  int  ReadTOTDataSet(const char *path);
  int  ReadLeakFile(const char* path);
  int  ReadIV(const char *path, bool takeAbsVal);
  int  ReadSensorIV(const char *path);
  int  ReadMask(int chip_number,int col_mask, std::string mask_file);
  int  ReadCurrData(const char *path, int LV_D, int CH_D, int LV_A, int CH_A);
  int  ReadBurnInData(const char *path);
  int  ReadRegtestSet(const char *path);
  int  ReadRegtestData(const char *path, int chip);

  // aux. routine for TDAC tuning
  int   FindTarget(float *avgthr, int &dacmax);
  // TOT calibration
  double calTOT(double tot, int chip, int col, int row, DatSet *cal, DatSet* CHiCal=NULL);
  // used in the timewalk to obtain the charge at a certain point or the time a certain point
  int  ExtrapolateToGetX(float x1, float y1, float x2, float y2, float &xref, float &yref);
  int  ExtrapolateToGetY(float x1, float y1, float x2, float y2, float &xref, float &yref); 
  void GuessScanPars(const char *path);
  //  2D histograms to store results from fits
  TH2F *m_parmap[NPARS], *m_chimap;
  //  2D histograms to store raw data (hits, TOT etc); see above definitions
  TH2F *m_maps_2D[NRAW];
  //  1D scan histos: for scan (eg hit vs VCAL DAC) and for cal. + raw TOT spectrum
  std::vector<float> *m_scanToT, *m_scanerr, *m_scanhi;
  unsigned int **m_ToTspec;
  TH1F **m_spectrum;
  // S-curve fit data from 2D scans
  TH2F **m_2DRawData; 
  //  Graph for misc. use - so far for IV-curve and timewalk only
  TGraph *m_miscgr[NGRAPH];
  // pointer to log file
  TLogFile *m_LogFile;
  // data and file name
  //std::string m_dtname, m_pname, m_fullpath;
  // index if data was loaded from standard-test menu (-1 otherwise)
  int m_stdtest;    
  // ugly, but can't think of anything better for now
  DatSet **m_refdata;
  // ROOT can't identify TGraphs by name, so have to remember last
  // one created to be able to clean up
  TGraphErrors *m_lastgr;
  // store analysis parameters (like Q_t0 and del-t0 in time walk)
  // to check later what was used
  std::vector<float> m_anaPars;
  // temporary object (i.e. not savet to MA root files) for PixScan config
  void *m_pixScan;

//********* by JW ***********************************************************
  // holds Thresh/noise summary results, 0=threshold, 1=threshold dispersion,
  // 2=noise
  float **m_FitResArray;
//***************************************************************************
};

#endif //MODULE
