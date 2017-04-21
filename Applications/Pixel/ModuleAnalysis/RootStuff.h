#ifndef TESTROOT_H
#define TESTROOT_H

#include <string>
#include <stdio.h>

#include "PixelDataContainer.h"
#include "DataStuff.h"
#include "FileTypes.h"
#include "TopWin.h"
#include "ModTree.h"
#include "OptClass.h"
#include "OptWin.h"

#include <TROOT.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TPaveText.h>
#include <TGraph.h>
#include <TProfile.h>
#include <TPostScript.h>
#include <THStack.h>

class BadPixel;
class MATPaletteAxis;
class FitClass;
class TGaxis;

/*! This class inherits from root's TH2F and allows user
 *  interaction via mouse click with 2D colour maps.
 *
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 * @version 1.5
 */

class MATH2F : public TH2F
{
 public:
   MATH2F();
   MATH2F(MATH2F&, bool do_dclk=true);
   ~MATH2F();
   TopWin *m_parnt;
   bool m_dodclk;
   void SetWidget(TopWin *parnt){m_parnt = parnt;};
   virtual void ExecuteEvent(Int_t event, Int_t px, Int_t py);
};

/*	Here are the interesting numbers obtained when projecting a
	2d histogram to 1d
*/
struct MapInfo{
	int ntot, nfitted, leaktot, dacmax;
	int ntdacl, ntdach;
};

/*! This class provides the plotting interface to ROOT
 *
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 * Giorgio Cardarelli <giorgio@pcteor1.mi.infn.it>
 * Aldo Saveedra <afsaavedra@lbl.gov>
 * @version 2.4
 */
class RMain{

public:
  RMain(optClass *optptr);
  ~RMain();

  static void MA_popup_test(int in1, int in2, TObject *obj);

  /*! Plots plain 2D colour map
   *
   */
  int  Plot2DColour(ModItem *item, int chip=-1);
  /*! Plots plain 2D colour map
   *  but special bit-mask for bad-pixel plots
   */
  int  PlotBadSel(ModItem *item, int chip=-1, int bit_mask=9999999, bool doAND=false);
  /*! Plots raw-data colour map, projection histogram and scatter (used for S-curve fit results)
   *
   */
  int  PlotMapProjScat(ModItem *item, int chip=-1, int scanpt=0, TH2F *user_hi=0, int ps_level=-1);
  int  PlotMapProjScat(ModItem *item, int chip, int scanpt[3], TH2F *user_hi, int ps_level);
  /*! Produces comparison-type plot (noise ratio, cross-talk, etc)
   *
   */
  int  PlotComp(ModItem *item, int chip=-1, BadPixel *summtab=NULL, int act_type=NONE, int ps_level=-1, int scanpt1=-1, int scanpt2=-1);
  /*! Produces comparison-type plot for 2D scans
   *  (threshold or noise ratio vs outer par.)
   */
  int  Plot2DComp(ModItem *item, int chip=-1);
  /*! Plots raw data with optional scan histograms
   *
   */
  int  PlotMapScanhi (ModItem *item, int chip=-1, int scanpt=-1, TH2F *user_hi=0, 
		      TObject *user_gr=0, TF1 *fun=0);
  int  PlotMapScanhi (ModItem *item, int chip, int scanpt[3], TH2F *user_hi, 
		      TObject *user_gr, int ps_level, TF1 *fun);
  /*! Plots result from ToT cal. combination (Clow+Chigh)
   *  as functions vs VCAL
   */
  int  PlotCombCal (ModItem *item, int chip=-1);
  /*! Plots result from ToT cal. combination (Clow+Chigh)
   *  as hisdtogrammed ToT for Q=10,20,50,100ke
   */
  int  PlotCombCalProj(ModItem *item, int chip=-1);
  /*! Plots time over threshold - average in colour-2D and spectrum histogram (if calib'ed)
   *  
   */
  int  PlotTOT (ModItem *item, int chip=-1);
  /*! Plots IV curve
   *  
   */
  int  PlotGraph (ModItem *item, int index=0, int scan_level=0);
  int  PlotGraph (ModItem **item, int ngraphs);
  int  PlotScan (ModItem **item, int nitems);
  /*! Plot CapMeasure for FE-I3
   */
  int  PlotCM3Graph (ModItem *item, double Uload=2.0, double *Cvals=0);
  /*! Fit graph with specific function
   */
  int FitGraph(int iFitFunc, ModItem *item, int plotType, double xmin=0., double xmax=-1.);
  /*! Plots timewalk vs charge graph
   *
   */
  int  TwalkGraphs(ModItem *item, int chip=-1);  //DatSet **rawdat);
  /*! missing/excess hits in digital scan vs Vdd
   *
   */
  int DigExcMiss(ModItem *item, int chip=-1, int plt_type=0, float vmin=-999, TH2F *mask=NULL, bool merge=false);
  /*! gets a TH2F-map of the minimum "good" threshold for a G/TDAC threshold scan.
   *  "good" = chi^2 and/or noise of fit is below provided cut value
   */
  TH2F* MapMinThresh(PixelDataContainer *did, int chip=-1);
  /*! gets TH2F-maps of the minimum "good" threshold and the noise and chi^2
   *  at this threshold for a G/TDAC threshold scan.
   *  "good" = chi^2 and/or noise of fit is below provided cut value
   */
  TH2F** AllMapsMinThresh(PixelDataContainer *did, int chip=-1);
  /*! plots the minimum "good" threshold for a G/TDAC threshold scan
   *  "good" = chi^2 and/or noise of fit is below provided cut value
   */
  int PlotMinThresh(ModItem *item, int chip=-1){return PlotMinThresh(item,chip,MEAN);};
  /*! plots the minimum "good" threshold or nise or chi^2 at this point 
   * for a G/TDAC threshold scan.
   *  "good" = chi^2 and/or noise of fit is below provided cut value
   */
  int PlotMinThresh(ModItem *item, int chip, int type);
  /*! plot correlation scatter/lego
   *
   */
  int  PlotCorrel(const char*, ModItem *itema, ModItem *itemb, int pida, int pidb, int scatplt=1, int scanpt=0, int chipID=-1, 
		  bool doPlot=true, int ps_level=-1);
  /*! plot correlation scatter/lego for 2D sfit data
   *
   */
  int  Plot2DCorrel(const char*, DatSet *did, int pid, Bool_t scatplt=kTRUE);
  /*! Plots masks (test phase of this)
   *
   */
  int  PlotMasked(ModItem *itema, ModItem *itemb, int pida, int pidb, Bool_t scatplt=kTRUE, 
		  int chip=-1, int scanpt=0, int bit_masp = -1);
  /*! to plot the profiles from the timewalk analysis
   *
   */
  int PlotProfile(ModItem *item, int chip);
  /*! reads time and bias current from logfiles of
   *  a vector of data items and plots that
   */
  int PlotBiasStab(std::vector<ModItem*> data_items);
  /*! Plots summary of burn-in scans (added 27/4/04 by LT)
   * 
   */
  int PlotBI_scans(ModItem *item);
  int BI_getScanSummary(ModItem *item);
  /*! Plots burn-in V,I, T vs Time (added 27/4/04 by LT)
   * 
   */
  int PlotBI_vitVsTime(ModItem *item);
  /*! // Plots burn-in currents vs NTC (added 27/4/04 by LT)
   * 
   */
  int PlotBI_currentVsTemp(ModItem *item);
  /*! plots lvl1 histograms (per scan point if neccessary)
   */
  int PlotLvl1(ModItem *item, int chip=-1, int scanpt=0);
  /*! plots any TH1F provided by the user
   */
  int PlotHisto(ModItem *item, int scanpt=0, int ps_lvel=-1, TH1F *user_hi=0);
  /*! returns the number of pixels with hits different from 100%
   *  (supposed to be used for digital scan)
   *  If show==true than a summary table with #bad pixels will be shown
   */
  static int nBadPixs(ModItem *item, bool show=false);
  /*! shows table with threshold, dispersion, noise summary
   *  (table is only shown if show==1, otherwise only a pointer
   *  to the table panel is returned)
   */
  BadPixel* ThreshSumm(ModItem *item, int show=1);
  /*! shows table with outer fit from 2D-sfit scan (ITH only for now)
   *  (table is only shown if show==1, otherwise only a pointer
   *  to the table panel is returned)
   */
  BadPixel* ITHSumm(ModItem *item, int show=1);
  /*! shows table with avg. peak position from source
   *  (table is only shown if show==1, otherwise only a pointer
   *  to the table panel is returned)
   */
  BadPixel* PeakSumm(ModItem *item, int show=1, float nom_peak=16600);
  /*! table with masked or dead channels
   *
   */
  BadPixel* MaskTable(ModItem *item, int chip);
  /*! creates the map for comparison style plots
   */
  TH2F* GetComp(ModItem *item, int chip, int ps_level, int scanpt1, int scanpt2);
  /*! creates the map for comparison style plots
   *  with userd-chosen plot type
   */
  TH2F* GetComp(ModItem *item, int chip, int type, int ps_level, int scanpt1, int scanpt2);
  /*! creates the map for comparison style plots
   *  with userd-provided histograms
   */
  TH2F* GetComp(ModItem *item, int type, TH2F *map1, TH2F *map2);
  //
  TH2F* GetBadSelMap(TH2F *orgmap, int bit_mask, bool doAND=false);
  /*! allows to combine fit parameter maps to something new
   *
   */
  void ParComb(const char *formula, DatSet *did);
  /*! plot PixScan type data as value versus scan par
   */
  int plotPS_ValVSPar(std::vector<int> pars, ModItem *item, int scan_level, int scanpts[3], int chip=-1, int fid=-1, bool pltAvg=false);

  /*! turns the error code in (maybe) something meaningful
   *
   */
  static std::string ErrorToMsg(int error);
  /*! removes histos and graphs stored in memory
   *
   */
  void ClearMemory();

  void SetCal(float cal) {for(int i=0;i<NCHIP;i++){ m_convfac[i][0] = cal;for(int j=1;j<4;j++)m_convfac[i][j] = 0;}};
  void SetCal(float *cal){for(int i=0;i<NCHIP;i++){ m_convfac[i][0] = cal[i];for(int j=1;j<4;j++)m_convfac[i][j] = 0;}};
  void SetCal(float **cal){for(int i=0;i<NCHIP;i++){ for(int j=0;j<4;j++)m_convfac[i][j] = cal[i][j];}};
  /*! gets calibration constants for fitting
   *
   */
  float** GetCal(){return m_convfac;};
  /*! Fit functions
   *
   */
  static double ErrorFunc(double *x, double *par);
  static double ComErFunc(double *x, double *par);
  static double TwoErFunc(double *x, double *par);
  static double TOTFunc(double *x, double *par);
  static TF1* GetFitFunc(int type);
  static int GetFitType(const char *name);
  static std::string GetFitName(int type);

  /*! defines the various colour schemes for 2D maps
   *
   */
  static void ColourScale(int type=0);
  /*! defines size of axes' title and label; type=general size
   *
   */
  static void SetStyle(int type=0);
  static void SetStyle(int type, TH2F *histo);
  static void SetStyle(int type, TH1F *histo);

  /*! last plotted map, and the one before which we have to remember for deletion
   *
   */
  MATH2F *m_currmap, *m_lastmap;
  /*! container for misc. objects that cannot be identified by name
   *
   */
  std::vector<TObject*> m_misc_obj;

  /*! those define the pixel to be shown in raw plot
   *
   */
  int m_chip, m_col, m_row;
  // (plot) options class
  optClass *m_plotOpts;

  // post script file if one is open
  TPostScript *m_psfile;

  // info from reading 2d histogram
  MapInfo *m_MapInfo;
  
  static double CombCalRoot(double *x, double *par);
  void CheckCan();
  void SaveCan(const char *path){CheckCan();m_canvas->SaveAs(path);};

//************************** by JW **********************************************************************************
  /*!  takes 2D-map and gets projections (for all, long, ganged, interganged pixels)
   *   returns array of 1D-histogram pointers  (added 25/01/05 by JW)
   */
  int GetProjection(ModItem *item, TH2F *user_hi, TH1F **projection, BadPixel *badpixtab);
  int GetProjection(int type, DatSet *data, TH2F *user_hi, TH1F **projection, BadPixel *badpixtab);
  int PlotSingleProjection(ModItem *item, TPad *pplt, int Pixel_type, int chip=-1, int scanpt=0);
  int PlotMultipleProjections(ModItem **itarr,int nrProj,bool overlay, int Pixel_type=1, int chip=-1, int scanpt=0);
  int OverlayHistos(ModItem **itarr, int nrProj, int Pixel_type=1, int chip=-1, int scanpt=0);
  int PlotMultiHist();

//  void PlotModuleSummary(ModItem* item, int summary_type);
  //void PlotModuleSummary(std::vector<ModItem*> item, int summary_type, TopWin* tw, QString label);
  void PlotModuleSummary(std::map<int,std::vector<ModItem*> > itemmap, int summary_type, TopWin* tw, QString label);
  TCanvas* GetCanvas(){return m_canvas;};
  TPaveText* GetText(){return m_text;};
  void RescaleOverview(int Min, int Max);
//********************************************************************************************************************

private:
  TopWin *m_widget;
  // default canvas and text
  TCanvas   *m_canvas;
  TPaveText *m_text;
  std::vector<TPaveText*> m_stnames;
  // standard fit functions
  TF1 *m_erfunc, *m_totfunc, *m_expo, *m_twoexp, *m_RDBfunc;
  // cal factor for fitting
  float  **m_convfac;
  // colour scale for multi-module overview plots
  MATPaletteAxis *m_palette;
  //  void setupMenu(MATPaletteAxis &palette);

  // some cosmetics for the graphics
  void TitleResize(float);
  void FitBoxResize(TObject*, float factorx, float factory=-1, float shift=0);
  // frequently used stuff
  void PlotStat(TH1F* hi);
  void PlotFit(TF1* fu);
  std::vector<TPad*> PadSetup();
  void ModuleGrid(TH2F *hi, int nChips);
  void flipHistoCont(TH2F *hi);
  // internal function for TOT calculation
  TH2F* TOT20K(int chip, DatSet *did);
  // for function plotting
  FitClass *m_fc;
};

#endif
