#ifndef PLOTFACTORY_H
#define PLOTFACTORY_H

#include <TH2F.h>
#include <QObject>
#include <vector>
class TCanvas;
class TPaveText;
class TF1;
class TPad;
class PixDBData;
class DataViewer;
class FitClass;
namespace PixLib{
  class Config;
}

/*! This class inherits from root's TH2F and allows user
 *  interaction via mouse click with 2D colour maps.
 *
 * @authors Joern Grosse-Knetter <jgrosse1@uni-goettingen.de>
 * @version 2.0
 */

class MATH2F : public TH2F
{

 public:
  MATH2F();
  MATH2F(MATH2F&, bool do_dclk=true, DataViewer *dv=0);
  ~MATH2F();
  
  void ExecuteEvent(Int_t event, Int_t px, Int_t py);
  void Draw(Option_t* option = "");
  void flipHistoCont();
  
 private:
  bool m_dodclk;
  bool m_flipped;
  DataViewer *m_dv;
};

/*! Main plotting class and ROOT interface of DataViewer
 *
 * @authors Joern Grosse-Knetter <jgrosse1@uni-goettingen.de>
 * @version 1.0
 */

class PlotFactory {

  enum PlotStyleIds{MAPHISCAT, MAPSPLHISCAT, MAPVSPLHISCAT, MAPONLY, HISTOASIS, GRAPH1D};

 public:
  PlotFactory(DataViewer *dv);
  ~PlotFactory();

  /*! Fit functions
   *
   */
  static double ErrorFunc(double *x, double *par);
  static double ComErFunc(double *x, double *par);
  static double TwoErFunc(double *x, double *par);
  static double TOTFunc(double *x, double *par);

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
  void CheckCan();
  void SaveCan(const char *path);

  /*! last plotted map, and the one before which we have to remember for deletion
   *
   */
  MATH2F* getCurrMap(){return m_currmap;};
  MATH2F* getLastMap(){return m_lastmap;};
  PixLib::Config& config(){return *m_config;};
  std::string getDataPath(){return m_dataPath;};
  bool getChipMode(){return m_startChipMode;};


  void ClearMemory();

  int GenericPlotting(PixDBData &pdbd, int chip, int scanpt[3], int ps_level, int ps_type);
  int PlotMapProjScat(PixDBData &pdbd, PixDBData &pdbd2, int chip, int scanpt[3], int ps_level, int ps_type, 
		      int scanpt2[3], int ps_type2, int oper_type);
  int PlotMapProjScat(PixDBData &pdbd, int chip, int scanpt[3], int ps_level, int ps_type);
  int PlotHisto(PixDBData &pdbd, int scanpt, int ps_level, int ps_type);
  int PlotGraph(std::vector<PixDBData*> pdbds);
  int plotValVsPar(PixDBData &pdbd, int ps_type, int scan_level, int scanidx[3], int in_chip, 
		   int sel_chip, int sel_col, int sel_row, std::vector<int> pars, int fit);
  int plotQVsToT(PixDBData &pdbd, int ps_type, int scan_level, int scanidx[3], int in_chip, 
		   int sel_chip, int sel_col, int sel_row, std::vector<int> pars, int fit);
  int plotAvgVsPar(PixDBData &pdbd, int ps_type, int scan_level, int scanidx[3], int in_chip);
  int PlotCorrel(PixDBData &pdbd, PixDBData &pdbd2, int chip, int scanpt[3], int ps_level, int ps_type, int ps_type2);
  FitClass &getFitClass(){return *m_fc;};

private:
  DataViewer *m_dv;
  FitClass *m_fc;
  // default canvas and text
  TCanvas   *m_canvas;
  TPaveText *m_text;
  std::vector<TPaveText*> m_stnames;
  // standard fit functions
  TF1 *m_erfunc, *m_totfunc, *m_expo, *m_twoexp, *m_RDBfunc;
  MATH2F *m_currmap, *m_lastmap;
  PixLib::Config *m_config;

  /*! container for misc. objects that cannot be identified by name
   *
   */
  std::vector<TObject*> m_misc_obj;

  /*! those define the pixel to be shown in raw plot
   *
   */
  int m_chip, m_col, m_row;

  // Helper functions
  void setupConfig();
  std::vector<TPad*> PadSetup();
  void ModuleGrid(TH2F *hi, int nChips);
  void TitleResize(float factor);
  void FitBoxResize(TObject*, float factorx, float factory=-1, float shift=0);
  void PlotFit(TF1* fu);

  // configuration items
  std::string m_dataPath;
  bool m_startChipMode;

  int m_plotLayout;
  int m_colourScheme;
  bool m_logScale;
  bool m_runFit;
  bool m_chipLabel;
  bool m_axisLabel;
  bool m_useHistoRange;
  float m_histoRgMin, m_histoRgMax;
  bool m_blackBkg;
  int m_scatplt;
  int *m_1dplotStyle;
  float *m_1dplotRangeMin;
  float *m_1dplotRangeMax;
  bool *m_1dplotRangeAct;
};

#endif // PLOTFACTORY_H
