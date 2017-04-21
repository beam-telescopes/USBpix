#ifndef DBTOMADATA_H
#define DBTOMADATA_H
#include "PixDBData.h"

class DBtoMAData : public PixDBData {

 public:
  DBtoMAData(const char *name=0, const char *path=0, const char *modName=0);
  ~DBtoMAData(){};

  /*! Returns a TH2F-map of type "type" (see FileTypes.h)
   *  if chip>0 then the map for only this chip is returned
   */
  TH2F* GetMap(int chip, int type);
  /*! Returns a TH2F-map of type "type" from a 2D scan
   *  (e.g. TDAC) for the given scan point
   *  if chip>0 than the map for only this chip is returned
   */
  TH2F* Get2DMap(int chip, int type, int scanpt);
  /*! converts MA histogram type to PixLib::PixScan type
   */
  static PixLib::PixScan::HistogramType convertType(int MAtype);
  /*! returns true if a certain histo type was filled and
   *  kept during data taking
   */
  bool haveHistoType(int MAtype);
  /*! Returns a graph containing hits vs scan variable
   *  for a 1D scan, or threshold, noise or chi^2 vs outer var.
   *  for a 2D scan
   */
  TGraphErrors* GetScanHi(int chip, int col, int row, int type, int scanpt);

};
#endif // DBTOMADATA_H
