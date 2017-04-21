#include "DataStuff.h"
#include "PixelDataContainer.h"
#include "ModTree.h"
#include <string>

ModItem::ModItem(Q3ListView *parent, QString label, PixelDataContainer* datset, 
		 int plotmode):Q3ListViewItem(parent,label){
  m_lock = false;
  DatID = datset;
  PltID = plotmode;
  m_lastpath = "";
  for(int i=0;i<NREFS;i++)
    RefID[i] = NULL;
  return;
}
ModItem::ModItem(ModItem *parent, QString label, PixelDataContainer* datset, 
		 int plotmode, PixelDataContainer* refset):Q3ListViewItem(parent,label){
  m_lock = false;
  DatID = datset;
  PltID = plotmode;
  m_lastpath = "";
  for(int i=1;i<NREFS;i++)
    RefID[i] = NULL;
  RefID[0] = refset;
  return;
}
ModItem::ModItem(ModItem *parent, ModItem *, QString label, PixelDataContainer* datset, 
		 int plotmode, PixelDataContainer* refset):Q3ListViewItem(parent,label){
  m_lock = false;
  DatID = datset;
  PltID = plotmode;
  m_lastpath = "";
  for(int i=1;i<NREFS;i++)
    RefID[i] = NULL;
  RefID[0] = refset;
  return;
}
ModItem::~ModItem(){
  return;
}
void ModItem::SetRefArr(PixelDataContainer **array, int nitems){
  int i,nit=nitems;
  if(nit>NREFS) nit=NREFS;
  for(i=0;i<nit;i++)
    RefID[i] = array[i];
  return;
}
void ModItem::SetRefArr(DatSet **array, int nitems){
  int i,nit=nitems;
  if(nit>NREFS) nit=NREFS;
  for(i=0;i<nit;i++)
    RefID[i] = (PixelDataContainer*)array[i];
  return;
}
ModItem* ModItem::ModParent(){
  ModItem *parnt;
  parnt = this->Parent();
  if(parnt==0) return 0;
  while(parnt->Parent()!=0 && parnt->GetPID()!=MODITEM){    // find PID=MODITEM item (=module)
    parnt = parnt->Parent();    // and the module item
  }
  if(parnt!=0 && parnt->GetPID()==MODITEM)
    return parnt;
  else
    return 0;
}
ModItem* ModItem::DatParent(){
  ModItem *datit, *parnt;
  datit = this;
  parnt = this->Parent();
  if(parnt==0) return 0;
  while(parnt->Parent()!=0 && parnt->GetPID()!=MODITEM){    // find PID=MODITEM item (=module)
    datit = parnt;              // the actual data set item
    parnt = parnt->Parent();    // and the module item
  }
  if(parnt!=0 && parnt->GetPID()==MODITEM)
    return datit;
  else
    return 0;
}

void ModItem::SetLastpath(QString path){
  std::string mypath = path.latin1();
  //m_lastpath = path;
  int i = mypath.find_last_of("/");
  mypath.erase(i,mypath.length()-i);
  m_lastpath = mypath.c_str();
  return;
}

ModItem* ModItem::findPlotItem(const char *label){
  QString qlabel = label;
  if(childCount()<=0) return 0;
  ModItem *test=(ModItem*)firstChild();
  ModItem *ch_test = 0;
  while(test!=0){
    if(test->text(0)==qlabel) return test;
    if((ch_test = test->findPlotItem(label))!=0) return ch_test;
    test = (ModItem*) test->nextSibling();
  }
  return 0;
}
ModItem* ModItem::findPlotItem(int plotid){
  if(childCount()<=0) return 0;
  ModItem *test=(ModItem*)firstChild();
  ModItem *ch_test = 0;
  while(test!=0){
    if(test->GetPID()==plotid) return test;
    if((ch_test = test->findPlotItem(plotid))!=0) return ch_test;
    test = (ModItem*) test->nextSibling();
  }
  return 0;
}
int ModItem::GetPID(){
  if(PltID<=STVITEM)
    return PltID;
  else if(PltID>MODITEM)
    return PltID;
  else
    return MODITEM;
}
int ModItem::GetModID(){
  if(PltID<=STVITEM || PltID>MODITEM)
    return 0;
  else
    return MODITEM-PltID;
}
void ModItem::SetModPos(int pos){
  if(GetPID()!=MODITEM) return; // only allowed for modules!
  PltID = MODITEM-pos;
}
std::vector<ModItem *> ModItem::checkForRefs(){
  std::vector<ModItem *> resultVec;
  ModItem *parent = ModParent();
  if(parent==0) return resultVec;

  ModItem *test1, *test2, *test3;
  test1 = (ModItem*) parent->firstChild();
  while(test1!=0){
    if(test1!=this){
      if(test1->GetRID() == GetDID() && GetDID()!=0)
        resultVec.push_back(test1);
      test2 = (ModItem*) test1->firstChild();
      while(test2!=0){
        if(test2->GetRID() == GetDID() && GetDID()!=0)
	  resultVec.push_back(test2);
        test3 = (ModItem*) test2->firstChild();
        while(test3!=0){
          if(test3->GetRID() == GetDID() && GetDID()!=0)
	    resultVec.push_back(test3);
          test3 = (ModItem*) test3->nextSibling();
        }
        test2 = (ModItem*) test2->nextSibling();
      }
    }
    test1 = (ModItem*) test1->nextSibling();
  }

  return resultVec;
}
std::vector<ModItem *> ModItem::getAllRefItems(){
  std::vector<ModItem *> resultVec;
  ModItem *parent = DatParent();
  if(parent==0) return resultVec;

  ModItem *test1, *test2;
  if(parent->GetRID()!=0)
    resultVec.push_back(parent);
  test1 = (ModItem*) parent->firstChild();
  // check for referencing items the new two levels below
  while(test1!=0){
    if(test1->GetRID()!=0)
      resultVec.push_back(test1);
    test2 = (ModItem*) test1->firstChild();
    while(test2!=0){
      if(test2->GetRID()!=0)
	resultVec.push_back(test2);
      test2 = (ModItem*) test2->nextSibling();
    }
    test1 = (ModItem*) test1->nextSibling();
  }

  return resultVec;
}
DatSet** ModItem::GetRIDArr()
{
  DatSet** tmpID;
  tmpID = new DatSet*[NREFS];
  for(int i=0;i<NREFS;i++)
    tmpID[i] = dynamic_cast<DatSet*>(RefID[i]);
  return tmpID;
}
DatSet*  ModItem::GetDID()
{
  return dynamic_cast<DatSet*>(DatID);
}
DatSet*  ModItem::GetRID(int item)
{
  if(item<NREFS) 
    return dynamic_cast<DatSet*>(RefID[item]); 
  else 
    return 0;
}
std::vector<ModItem *> ModItem::getAllSelItems()
{
  std::vector<ModItem *> resultVec, tmpVec;

  ModItem *test;
  test = (ModItem*) firstChild();
  while(test!=0){
    if(test->isSelected())
      resultVec.push_back(test);
    tmpVec = test->getAllSelItems();
    for(std::vector<ModItem *>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
      resultVec.push_back(*IT);
    test = (ModItem*) test->nextSibling();
  }
  
  return resultVec;
}
