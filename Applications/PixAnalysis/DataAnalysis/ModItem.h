#ifndef MODITEM_hh
#define MODITEM_hh

#include <vector>
#include <qlistview.h>
#include <qstring.h>
#include "Module.h"


class Moditem:public QListViewItem{

public:
  Moditem(QListView *parent, QString l1, QString l2, Modul *mod);
  // ModItem(ModItem   *parent, QString label, PixelDataContainer* datset=0, int pmode=NONE);
  //ModItem(ModItem   *parent, ModItem *after, QString label, PixelDataContainer* datset=0, int pmode=NONE, 
  //	  PixelDataContainer* refset=0);
  ~Moditem();
  Modul * GetModul(){ return m_modul;};
  void updateStatus();
  void setzen(){};
  //PixelDataContainer*  GetDataItem(){return DatID;};
  //PixelDataContainer*  GetRefItem(int item=0){if(item<NREFS)return RefID[item];else return 0;};
 
  
 private:
  Modul *m_modul;
  
};

#endif
