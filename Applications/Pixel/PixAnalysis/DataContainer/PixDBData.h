#ifndef PIXDBDATA_H
#define PIXDBDATA_H

#include "PixelDataContainer.h"
#include <PixController/PixScan.h>
#include <string>
#include <vector>
#include <map>

class FitClass;
class TH2F;
class TH1F;
class TObject;
class TGrahpErrors;
namespace PixLib{
  class PixConfDBInterface;
  class DBInquire;
  class Config;
  class Histo;
  class PixModule;
}

/*! This class provides easy access to scan data
 *  such as configuration information or histograms
 * @authors Joern Grosse-Knetter <joern.grosse-knetter@uni-bonn.de>
 */

class PixDBData : public PixelDataContainer {

 public:
  /*! The arguments refer to the file, the module group inquire and the module:<br>
   *  <b>name</b>: arbritrary label, use whatever you like to identify this object<br>
   *  <b>path</b>: combination of file name, scan label, and the name of the module group 
   *        in ROOT-fashion; e.g. if you data is in "/data/myfile.root", 
   *        your scan was labelled "threshold scan", and the module belongs to group "SLOT15"
   *        set name to "/data/myfile.root:/threshold scan/SLOT15"<br>
   *  <b>modName</b>: is the name of the module as used by PixLib::PixModule and as displayed
   *           by STcontrol<br><br>
   */
  enum OperType {RATIO, DIFFERENCE, QUADDIFF, XTRATIO, MAX_OPERS};

  PixDBData(const char *name, const char *path, const char *modName);
  ~PixDBData();

  /*! returns module name for identification purposes
   */
  const char* GetModName(){return m_modName.c_str();};

  /*! returns the scan level,
   * i.e. 0 for no scan, 1 for 1D, 2 for 2D scan etc
   */
  int getScanLevel();
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
  /*! returns vector with individual scan point values on requested loop level
   */
  std::vector<float>& getScanPoints (int loopLevel);
  /*! returns the name of the scanned variable
   */
  std::string getScanPar(int loopLevel);

  /*! Returns a TH2F-map of PixScan::HistogramType "type"
   *  if chip>0 than the map for only this chip is returned;
   *  in case of several histograms for the various scan points,
   *  this is "guesssed"     <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TH2F* GetMap(int chip, int type){return GetMap(chip,type,-1);};
  /*! Returns a TH2F-map of PixScan::HistogramType "type"
   *  if chip>0 than the map for only this chip is returned;
   *  in case of several histograms for the various scan points,
   *  this is specified by the scanot argument   <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TH2F* GetMap(int chip, int type, int scanpt);
  /*! reduce module-wide current map to chip-map
   *  NB: map stored in memory is replaced!
   */
  TH2F* getChipMap(int chip);
  /*! Returns a TH2F-map of type "type" from a 2D scan
   *  (e.g. TDAC) for the given scan point
   *  if chip>0 than the map for only this chip is returned    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TH2F* Get2DMap(int chip, int type, int scanpt){return GetMap(chip,type,scanpt);};
  /*! Returns a TH2F-map of TDAC or FDAC values as in the
   *  module config
   *  if chip>0 than the map for only this chip is returned    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TH2F* getDACMap(int chip, const char *type);
  /*! Returns a TH2F-map of any of the four masks as in the
   *  module config
   *  if chip>0 than the map for only this chip is returned    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TH2F* getMaskMap(int chip, const char *type);
  /*! Returns a graph containing hits vs scan variable
   *  for a 1D scan, or threshold, noise or chi^2 vs outer var.
   *  for a 2D scan    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TGraphErrors* GetScanHi(int chip, int col, int row, int type, int scanpt=-1);
  /*! Returns a PixLib::Histo object of the requested type    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          Histo copy constructor to retain a permanent Histo object
   */
  PixLib::Histo* getGenericPixLibHisto(PixLib::PixScan::HistogramType type, int scanpt=-1,
				       int scan_level=-1);
  /*! Returns a PixLib::Histo object of the requested type - all scan levels must be specified <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          Histo copy constructor to retain a permanent Histo object
   */
  PixLib::Histo* getGenericPixLibHisto(PixLib::PixScan::HistogramType type, const int idx[4]);
  /*! Returns a ROOT TObject corresponding to a PixLib::Histo object of the requested type    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TObject* getGenericHisto(PixLib::PixScan::HistogramType type, int scanpt=-1, int scan_level=-1);
  /*! Returns a ROOT TObject corresponding to a PixLib::Histo object of the requested type
   *  - all scan levels must be specified   <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          resp. copy constructor to retain a permanent object
   */
  TObject* getGenericHisto(PixLib::PixScan::HistogramType type, const int idx[4]);
  /*! generate a PixLib histogram from an pixel-by-pixel operation oin two histos
   */
  PixLib::Histo* getOperPixLibHisto(PixLib::PixScan::HistogramType type1, PixDBData *data2, PixLib::PixScan::HistogramType type2, 
				    OperType operation, int scanpt=-1, int scanpt2=-1, int scan_level=-1);
  /*! generate a root histogram from an pixel-by-pixel operation oin two histos
   */
  TObject* getOperHisto(PixLib::PixScan::HistogramType type1, PixDBData *data2, PixLib::PixScan::HistogramType type2, 
				    OperType operation, int scanpt=-1, int scanpt2=-1, int scan_level=-1);
  /*! generate a mask from provided histogram and return it
   */
  PixLib::Histo* GenMask(PixLib::Histo *inHisto, double minVal, double maxVal);
  /*! generate a mask from given histogram and return it
   */
  PixLib::Histo* GenMask(PixLib::PixScan::HistogramType type, int scanpt, int scan_level, double minVal, double maxVal);
  /*! generate a mask from the result of a given operation on two histograms and return it
   */
  PixLib::Histo* GenMask(PixLib::PixScan::HistogramType type1, PixDBData *data2, PixLib::PixScan::HistogramType type2, 
			 OperType operation, int scanpt, int scan_level, double minVal, double maxVal);
  /*! returns true if a certain histo type was filled and
   *  kept during data taking
   */
  bool haveHistoType(int type){return haveHistoType((PixLib::PixScan::HistogramType) type);};
  bool haveHistoType(PixLib::PixScan::HistogramType type);
  /*! returns the name of a PixLib histogram type
   */
  std::string getHistoName(PixLib::PixScan::HistogramType type);

  /*! Returns a reference to the PixScan config    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          Config copy constructor to retain a permanent object
   */
  PixLib::Config& getScanConfig();
  /*! Returns a reference to the PixModule config    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          Config copy constructor to retain a permanent object
   */
  PixLib::Config& getModConfig();
  /*! Returns a reference to the PixBoc config    <br>
   *  BEWARE: the object returned is temporary and will be deleted
   *          by most calls to other PixDBData functions; use the
   *          Config copy constructor to retain a permanent object
   */
  PixLib::Config& getBocConfig();
  /*! returns the info string from PixController 
   */
  const char* getCtrlInfo();
  /*! returns the DCS value read for a specific channel; type: 0=current, 1=voltage
   */
  double getDCSValue(std::string chanName);

  /*! either returns the results inquire or makes it if it is not there
   */
  PixLib::DBInquire* getResultsInquire();
  /*! either returns an inquire of specified name or makes it if it is not there
   */
  PixLib::DBInquire* getInquire(PixLib::DBInquire *parent, std::string dname, std::string name);
  /*! initialise fitting
   */
  void initFit(PixLib::PixScan::HistogramType type, int loop_level, float *pars, bool *pars_fixed, int *par_his, int fittype=1, 
	       PixLib::PixScan::HistogramType errType=PixLib::PixScan::MAX_HISTO_TYPES,
	       PixLib::PixScan::HistogramType chiType=PixLib::PixScan::MAX_HISTO_TYPES, bool doCalib=true);
  /*! clear fit remnants; bool argument: true -> clear also histos, otherwise only the rest
   */
  void clearFit(bool);
  /*! Fit specified histogram versus variable scanned in specified loop
   */
  void fitHisto(int fittype=1, int chip=-1, float chicut=20,float xmin=0, float xmax=0, 
		float fracErr=0, bool useNdof=true);

  /*! fit graph as obtained with GetGraph function with specified fit function
   *  parameter fit result is written to vector and to file
   */
  void fitGraph(std::vector<float> &fitPars, float &chi2, int fittype=1, float xmin=0, float xmax=0, bool useNdof=true);
  /*! fit graph assuming it contains the result of a CAPMEASURE scan with FE-I3
   */
  void fitFEI3CapMeasure(std::vector<float> &capval, std::vector<float> &intermedFitPars, double Uload=2.0);

  /*! returns a graph like from IV scan
   */
  TGraph* GetGraph(int ind=0){return GetGraph(ind,0);};;
  TGraph* GetGraph(int ind, int level);

  // save histo - not finalised yet
  void writePixLibHisto(PixLib::Histo*, PixLib::PixScan::HistogramType type, int scanpt, int scan_level);
  // save fit ID
  void writeFitType(int fittype, int scan_level);

  static int getPixScanID(int stdScanID);

  // temporary: retrieve one of the current fit parameter histos
  PixLib::Histo* getParHisto(int ipar);
  TH2F* getParHistoR(int ipar);

  // access to operation (ratio, dif.. etc.) types
  std::map<std::string, int> &getOperTypes(){return m_operTypes;};
  // find the name to a given operation type
  std::string& findOperName(int);

  // handling of opening files
  PixLib::DBInquire* openDBFile(int getType=1, bool write=false);
  bool m_keepOpen;

  // thread var's
  void abortFit(){m_abort = true;};
  void getPix(int &col, int &row){row = m_currRow, col = m_currCol;};

  bool validPath(){return (m_modID>=0);};

 private:
  // temporary variables
  TH2F *m_2dHisto;
  TH1F *m_1dHisto;
  TGraph *m_graphSimp;
  TGraphErrors *m_graph;
  PixLib::PixScan *m_ps;
  PixLib::PixModule *m_pm;
#ifndef NOTDAQ
  PixLib::PixBoc *m_pb;
#endif
  // derived info about this module/scan
  std::string m_modName;
  int m_modID;
  PixLib::PixConfDBInterface *m_DBfile;
  // temporary storage for fitting routine
  std::vector<PixLib::Histo*> m_fitHistos, m_errHistos, m_parHistos;
  PixLib::Histo *m_oldChiHisto;
  std::vector<int> m_parHis;
  std::vector<float> m_parInit, m_varValues;
  std::vector<bool> m_parFixed;
  FitClass *m_fitClass;
  std::map<std::string, int> m_operTypes;
  // private helper functions
  PixLib::Histo* readHisto(PixLib::DBInquire* dbi, PixLib::PixScan::HistogramType type, 
			   const int id[4] );

  // for communication in case of threaded operation
  bool m_abort;
  int m_currRow, m_currCol;
};

#endif // PIXDBDATA_H
