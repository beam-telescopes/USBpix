/***************************************************************************
                          PrimlistPanel.h  -  description
                             -------------------
    begin                : Wed 25 Jan 2006
    copyright            : (C) 2006 by jgrosse
    email                : joern.grosse-knetter@uni-bonn.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRIMLISTPANEL_H
#define PRIMLISTPANEL_H

#include "ui_PrimlistPanel.h"
#include <QTreeWidgetItem>
#include "PrimListItem.h"

class STControlEngine;
namespace PixLib{
  class PixScan;
  class DBInquire;
}
class PrimListItem;
class QString;

/**
  *@author Joern Grosse-Knetter
  */

class PrimListViewItem : public QWidget, public QTreeWidgetItem, public PrimListItem {

  Q_OBJECT

 public:
  PrimListViewItem( QTreeWidget* parent, PrimListTypes, int, const char* label, PixLib::PixScan *in_ps=0 );
  PrimListViewItem( QTreeWidget* parent, PrimListViewItem&, int);
  PrimListViewItem( QTreeWidget* parent, PixLib::DBInquire*);
  ~PrimListViewItem();

 public slots:

  void edit(){edit(true);};
  void edit(bool isNew);
  void up();
  void down();
  void editPS();
  void saveEdit();
  void cleanEdit();
  void toolTypeChanged();

 signals:
  void editThisPS(PixLib::PixScan*, QWidget*);

 private:
  QWidget *m_tmpWidget;
  PrimListItem *m_copyItem;
};

class PrimlistPanel : public QWidget, public Ui::PrimlistPanel  {

  Q_OBJECT

 public: 
  PrimlistPanel( STControlEngine &engine_in, QWidget* parent = 0 , bool extCtrl=false);
  virtual ~PrimlistPanel();

 public slots: // Public slots
  void mergePrl();
  void loadPrl(const char *extFile, int index);
  void loadPrl(const char *extFile){loadPrl(extFile, -1);};
  void loadPrl(){loadPrl(0,-1);};
  void freezePanel();
  void disableStart();
  void startPrl();
  void startPrlIter();
  void finishedScan();
  void finishedPrl();
  void currentProcItem(){currentProcItem(m_lastProcInd);};
  void currentProcItem(int);
  void savePrl();
  void abortPrl();
  void clearPrl();
  void browseOutput();
  void addItem();
  void editItem(QTreeWidgetItem*, int);
  void deleteCurrItem();
  void up100CurrItem();
  void up10CurrItem();
  void upCurrItem();
  void downCurrItem();
  void down10CurrItem();
  void down100CurrItem();
  void editPS(PixLib::PixScan*, QWidget*);
  void copyItem();
  void pasteItem();
  void modTestMode(bool);
  std::vector<PrimListItem*>& getPrlItems();
  void setBirep(bool isBi);

 signals:
  //  void scanFileChanged(const char*);
  void finishedList();
  void changeStartState(bool);

 private:

  STControlEngine &m_engine;
  PrimListViewItem *m_currItem;
  PrimListViewItem *m_copyItem;
  bool m_extCtrl;
  bool m_abortPressed;
  int m_iiter;
  int m_iterMask;
  int m_lastProcInd;
  std::vector<PrimListItem*> m_items;

};

#endif //PRIMLISTPANEL_H
