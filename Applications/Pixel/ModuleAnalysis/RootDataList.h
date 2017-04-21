#ifndef ROOTDATALIST
#define ROOTDATALIST

#include "ui_RootDataListBase.h"
#include <map>

class Q3ListViewItem;

class RootDataList : public QDialog, public Ui::RootDataListBase{

  Q_OBJECT

 public:
  RootDataList( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~RootDataList();

  Q3ListViewItem* LoadName(const char *, long int pointer_address=0,const char *timestamp=0);

  std::map<int, int> loadedItems; // supposed to keep the addresses of our QListViewItem and the loaded object
  std::map<int, int> loadedItemsInv; // as above, but the other way round

 public slots:
   void SelectAll(){SelectAll(true);};
   void SelectAll(bool select);
   void DeselectAll(){SelectAll(false);};
};


#endif
