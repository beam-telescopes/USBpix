#ifndef REFWIN_H
#define REFWIN_H

#include <map>
#include <vector>

#include "ui_RefWinBase.h"
#include "FileTypes.h"
#include "StdTestWin.h"

#define maxnit 50

class ModItem;

/*! panel for selection of reference data needed for analysis,
 *  e.g. cross talk needs threshold reference.
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 */

class RefWin : public QDialog, public Ui::RefWinBase
{
 Q_OBJECT

 public:
  RefWin( ModItem* thisis, QWidget* parent = 0, const char* name = 0, 
	  bool modal = FALSE, Qt::WFlags fl = 0, int RefForWhat=0 , const char *DBfname=0);
  ~RefWin();
  
  StdTestWin *stdtestwin;
  ModItem *selit, *callit, *inlist[50];
  int func;
  std::vector<int> list_of_stdscans;
  std::map<int,int> map_list_stdindex;
  int FuncToItem(int func);
  void AddToList(const char *, int); 
  void GetListAndMap(std::vector<int>&, std::map<int,int>&);
  bool nextClicked(){return m_nextClicked;};

 public slots:
  void RDAccept();
  void Naccept(){m_nextClicked=true; accept();};
  void GetTestList();
  void GetHistoList();
  void GetModuleList();
  
 private:
  int nit;
  int items[maxnit];
  QString m_DBfname, m_grpName;
  bool m_nextClicked;
  
};

#endif // REFWIN_H
