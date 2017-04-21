#ifndef PIXELDATACONTAINER_H
#define PIXELDATACONTAINER_H

/* #define NCHIP           2 */
/* #define NPIX        26880 */
/* #define NCOL           80 */
/* #define NROW          336 */

#include <string>

class TH1F;
class TH2F;
class TGraph;
class TGraphErrors;

/*! This is a abstract class combining needs from both the ModuleAnalysis 
 *  <a href="http://sctpixel.home.cern.ch/sctpixel/RodDaq/PixelDoxy/ModuleAnalysis/html/classDatSet.html">DatSet</a>
 *  and the PixLib::PixScan-oriented PixDBData classes.
 * 
 * @authors Joern Grosse-Knetter <joern.grosse-knetter@uni-bonn.de>
 */

class PixelDataContainer {

 public:
  PixelDataContainer(const char *name=0, const char *path=0);
  virtual ~PixelDataContainer(){};

  /*! get the name of the data item
   */
  const char* GetName(){return m_dtname.c_str();};
  /*! get the original file name (as from TurboDAQ, ROOT etc)
   *
   */
  const char* GetPath(){return m_pname.c_str();};
  /*! get the original file path
   *
   */
  const char* GetFullPath(){return m_fullpath.c_str();};
  /*! change the name (ie that referring to the tree-view
   *  label) of the data item; path (ie the org. file name
   *  remains unchaned
   */
  void ChangeName(const char* newname){m_dtname = newname;};

  /*! map coordinates: convert chip, column, row into x and y coordinate
   */
  void PixXY(int chip, int col, int row, float *xval, float *yval);
  /*! scan histogram index: convert chip, column, row into index
   */
  int  PixIndex(int chip, int col, int row){return chip*m_nRows*m_nCols+row+m_nRows*col;}
  /*! scan histogram index: convert x and y coordinate into index
   */
  int  PixXYtoInd(int xval, int yval);
  /*! map coordinates: convert x and y coordinates into chip, column, row
   */
  void PixXYtoCCR(int xval, int yval, int *chip, int *col, int *row);
  /*! map coordinates: convert x and y coordinates into x and y bin index
   */
  void PixXYtoBins(float xval, float yval, int &binx, int &biny);
  /*! map coordinates: convert chip, column, row into x and y bin index
   */
  void PixCCRtoBins(int chip, int col, int row,int &binx, int &biny);
  /*! distinguish normal 50x400um, long and ganged pixels (index)
   * 0 normal
   * 1 long
   * 2 ganged
   * 3 long ganged
   * 5 inter-ganged
   */
  int  Pixel_Type(int col, int row);
  /*! distinguish normal 50x400um, long and ganged pixels (name)
   */
  static const char * Pixel_TypeName(int type);

  /*! returns the scan level,
   * i.e. 0 for no scan, 1 for 1D, 2 for 2D scan etc
   */
  virtual int getScanLevel(){return -1;};
  /*! returns true if a certain histo type was filled and
   *  kept during data taking
   */
  virtual bool haveHistoType(int){return false;};
  /*! returns number of events, i.e. number of injections 
   *  followed by LVL1 trigger(s)
   */
  virtual int getNevents(){return -1;};
  /*! returns number of scan steps on requested loop level
   */
  virtual int getScanSteps(int){return -1;};
  /*! returns scan start value on requested loop level
   */
  virtual float getScanStart(int){return -1.;};
  /*! returns scan start value on requested loop level
   */
  virtual float getScanStop (int){return -1.;};

  /*! Returns a TH2F-map of type "type" (see FileTypes.h)
   *  if chip>0 than the map for only this chip is returned
   */
  virtual TH2F* GetMap(int, int){return 0;};
  /*! Returns a TH2F-map of type "type" from a 2D scan
   *  (e.g. TDAC) for the given scan point
   *  if chip>0 than the map for only this chip is returned
   */
  virtual TH2F* Get2DMap(int, int, int){return 0;};
  /*! returns a TH2F map of "type"
   * versus inner+outer scan parameter (e.g. #hits vs
   * TDAC and VCAL) or "type"-distribution vs
   * inner scan parameter (e.g. LVL1 vs MCC strobe delay)
   */
  virtual TH2F* GetScanMap(int, int, int, int){return 0;};
  /*! Returns a graph containing hits vs scan variable
   *  for a 1D scan, or threshold, noise or chi^2 vs outer var.
   *  for a 2D scan
   */
  virtual TGraphErrors* GetScanHi(int, int, int, int, int){return 0;};
  TGraphErrors* GetScanHi(int i, int j, int k, int a){return GetScanHi(i,j,k,a,-1);};
  TGraphErrors* GetScanHi(int i, int j, int k){return GetScanHi(i,j,k,38);};
  /*! Returns a graph containing ToT vs scan variable (1D only)
   */
  virtual TGraphErrors* GetToTHi(int, int, int){return 0;};
  /*! Returns the ToT spectrum e.g. for a source scan
   *  as a summary for a chip (chip==-1 is the module summary)
   */
  virtual TH1F* GetSpecHi(int){return 0;};
  /*! Returns the ToT spectrum e.g. for a source scan
   *  for a given pixel; if calibration is provided
   *  the ToT is calibrated into electrons
   */
  virtual TH1F* GetToTDist(int, int, int, bool, PixelDataContainer*){return 0;};
  TH1F* GetToTDist(int i, int j, int k, bool nofit){return GetToTDist(i,j,k,nofit,0);};
  TH1F* GetToTDist(int i, int j, int k){return GetToTDist(i,j,k,false);};
  /*! returns a graph like from IV scan
   */
  virtual  TGraph* GetGraph(int, int){return 0;};
  TGraph* GetGraph(){return GetGraph(0,0);};

  // geometry info
  void getGeom(int &nrow, int &ncol, int &nfe){nrow=m_nRows; ncol=m_nCols; nfe=m_nFe;};
  void getGeom(int &nrow, int &ncol, int &nfe, int &nferows){nrow=m_nRows; ncol=m_nCols; nfe=m_nFe; nferows=m_nFeRows;};

 protected:
  std::string m_dtname, m_pname, m_fullpath;
  // for flexible geometry
  int m_nRows, m_nCols, m_nFe, m_nFeRows;

};


#endif // PIXELDATACONTAINER_H
