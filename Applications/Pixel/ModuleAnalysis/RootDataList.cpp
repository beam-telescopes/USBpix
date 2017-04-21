#include "RootDataList.h"
#include <q3listview.h>

RootDataList::RootDataList( QWidget* parent, const char* , bool , Qt::WFlags fl)
  : QDialog(parent,fl){
  //  : RootDataListBase(parent,name,modal,fl){
  setupUi(this);
  QObject::connect(OKButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(AbortButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(AllButton, SIGNAL(clicked()), this, SLOT(SelectAll()));
  QObject::connect(DeAllButton, SIGNAL(clicked()), this, SLOT(DeselectAll()));

  FileList->setColumnWidth(0,350);
  FileList->setColumnWidth(1,350);
  FileList->setColumnWidth(2,1);
  loadedItems.clear();
  loadedItemsInv.clear();
}

RootDataList::~RootDataList(){
}

void RootDataList::SelectAll(bool select){
  Q3ListViewItem *item;
  item = FileList->firstChild();
  while(item!=NULL){
    FileList->setSelected(item, select);
    item = item->nextSibling();
  }
}

Q3ListViewItem* RootDataList::LoadName(const char *label, long int pointer_address,
				      const char *timestamp){
  int n = FileList->childCount();
  QString qts="";
  if(timestamp!=0)
    qts = timestamp;
  Q3ListViewItem *item = new Q3ListViewItem(FileList,label,qts,QString::number(n));
  loadedItems.insert(std::make_pair((long int)item,pointer_address));
  loadedItemsInv.insert(std::make_pair(pointer_address,(long int)item));
  return item;
}
