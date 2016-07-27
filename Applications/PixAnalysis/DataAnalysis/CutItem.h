#ifndef CUTITEM_H
#define CUTITEM_H

#include <vector>
#include <qlistview.h>
#include <qstring.h>
#include "cutpanel.h"


class CutItem:public QListViewItem{

public:

  CutItem(QListView *parent, QString l1, QString l2, QString l3,QString l4,QString l5,QString l6);
  ~CutItem();
  
  std::string getCutName();
  std::string getMin();
  std::string getMax();
  std::string getTestType();
  std::string getHistoType();
  std::string getActType();
  
};

#endif
