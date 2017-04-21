#ifndef MODTREE
#define MODTREE

#include <vector>
#include <q3listview.h>
#include <qstring.h>
#include "FileTypes.h"
 
class PixelDataContainer;
class DatSet;

#define NREFS   5

/*!  Item in the tree-view of the TopWin main
 *   panel - contains either a module, a data set
 *   (DatSet ::GetDID()) or a plotting/analysis action
 *   (TopWin ::GetPID() calling a RMain class function).
 *
 *   Use from an application calling ModuleAnalysis:
 *   (1)  Create module items with ("MAF" is the pointer to your TopWin object):
 *        ModItem *item = new ModItem(MAF->ModuleList, [module name (const char*)]);
 *   (2)  load data by creating a corresponding DatSet class object (yields pointer "DATA")
 *   (3)  enter into MA tree view with:
 *        ModItem *dataitem = MAF->DataToTree(item,DATA);
 */

class ModItem:public Q3ListViewItem{

public:
  ModItem(Q3ListView *parent, QString label, PixelDataContainer* datset=0, int pmode=MODITEM);
  ModItem(ModItem   *parent, QString label, PixelDataContainer* datset=0, int pmode=NONE, PixelDataContainer* refset=0);
  ModItem(ModItem   *parent, ModItem *after, QString label, PixelDataContainer* datset=0, int pmode=NONE, 
	  PixelDataContainer* refset=0);
  ~ModItem();
  
  PixelDataContainer*  GetDataItem(){return DatID;};
  PixelDataContainer*  GetRefItem(int item=0){if(item<NREFS)return RefID[item];else return 0;};
  DatSet*  GetDID();
  DatSet*  GetRID(int item=0);
  DatSet** GetRIDArr();
  int      GetPID();
  int      GetModID();
  void     SetModPos(int pos);
  void     ChgPID(int newpid){PltID = newpid;};
  void     SetRefArr(PixelDataContainer **array, int nitems);
  void     SetRefArr(DatSet **array, int nitems);
  ModItem* Parent(){return (ModItem*)this->parent();};
  ModItem* ModParent();
  ModItem* DatParent();
  ModItem* findPlotItem(const char *label);
  ModItem* findPlotItem(int plotid);
  void     SetLastpath(QString path);
  std::vector<ModItem *> checkForRefs();
  std::vector<ModItem *> getAllRefItems();
  std::vector<ModItem *> getAllSelItems();
  
  QString m_lastpath;
  bool    m_lock;
  
 private:
  PixelDataContainer* DatID;
  PixelDataContainer* RefID[NREFS];
  int     PltID;
};

#endif
