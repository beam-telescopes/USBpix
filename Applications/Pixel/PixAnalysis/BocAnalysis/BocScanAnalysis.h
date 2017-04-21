/***************************************************************************
                          BocScanAnalysis -  description
                             -------------------
    begin                : Sun, 7 Dec 2005
    author               : Iris Rottlaender
    email                : rottlaen@physik.uni-bonn.de
*************************************************************************/

#ifndef PIXANA_BOCANA
#define PIXANA_BOCANA

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>

#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include <Config/Config.h>
#include <Config/ConfObj.h>
#include "PixController/PixScan.h"
//#include "../PixAnalysis/DataContainer/PixDBData.h"
#include "DataContainer/PixDBData.h"
#include "Histo/Histo.h"

using namespace PixLib;
using namespace std;

// contains results of Bocscan-Analysis
struct BocScanAnalysisResults{
  
  int dim;
  // maximal distance to errors from distance matrix alone
  int maxdist;
  bool seenError;
  vector<int> bestX, bestY, bestZ;
  // modified distance to errors as used by shiftResults()
  vector<int> modmaxdistX, modmaxdistY, modmaxdistZ;
  string param[3];
  int inlink;
  int outlink[4];
  // used for exeption in shiftResults().
  bool seeUpperThreshold;

  BocScanAnalysisResults();
  ~BocScanAnalysisResults();
  
  void DumpResults();      
};


class BocScanAnalysis{
  
 public:
  struct ScanSpecs{
    
    int Bins;
    float min, max, minpos, maxpos; 
    float BinWidth;
    bool pacman, LoopUsed;
    string ParamLoop;
  };
  
  class DistanceMatrix{
    
  public:
    DistanceMatrix(int X, int Y, int Z);
    ~DistanceMatrix();
    void DumpMatrix();
    void FillError(int x, int y, int z);
    BocScanAnalysisResults ComputeDistances(ScanSpecs X, ScanSpecs Y, ScanSpecs Z);
    void Initialize();

  private:
    int XBins;
    int YBins;
    int ZBins;
    short ***matrix;

  };
  
  int InitializeAnalysis(PixDBData DataObj);
  BocScanAnalysisResults FindBestParameter();
    // if dim = 2 the distance to parameter which is closer to errors  might dominate results, to avoid this:
  void RefineResults(vector<int>* longParam, vector<int>* longMaxDist, vector<int> *shortParam, vector<int>* shortMaxDist, ScanSpecs longSpecs);
  void shiftResults(BocScanAnalysisResults* results);

 private:
 
 ScanSpecs Dim[3];
 vector<Histo> histovec;
 int nevents;
 int ndim;
 int inlink;
 int outlink[4];
 
};

#endif
