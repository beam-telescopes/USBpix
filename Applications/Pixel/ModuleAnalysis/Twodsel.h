#ifndef TWODSEL_H
#define TWODSEL_H

#include "ui_TwodselBase.h"

#include <string>
#include <map>
#include <vector>

class ModItem;
class TopWin;

/*! panel for initialising 1D->2D scan combination
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 */

class TwodSel : public QDialog, public Ui::TwodSelBase
{
    Q_OBJECT

      friend class TopWin;

 public:
  
  TwodSel( std::vector<ModItem *> allMods, int type=0, QWidget* parent = 0, 
	   const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~TwodSel();
  int getSelScan();

 protected:
  std::map<std::string,int> scan_list;
  std::map<int,int> scan_ind;
  std::vector< std::vector<ModItem*> > m_dataList;
  std::vector< std::vector<int> > m_scanPts;
  bool m_threshIn;

 public slots:
  void checkScantype();
  void scanEvenSpacing(bool);
  void updateScanPts();
  void tableScanPts();

};

#endif // TWODSEL_H
