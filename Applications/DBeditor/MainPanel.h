#ifndef MAINPANEL_H
#define MAINPANEL_H

#include "ui_MainPanel.h"
#include "GeneralDBfunctions.h"
#include <vector>

class DBEdtEngine;
class DBTreeView;
class DBTreeItem;
class QTreeWidgetItem;
class QPoint;
class QSplitter;
class QMouseEvent;

class MainPanel : public QMainWindow, public Ui::MainPanel {

  Q_OBJECT

 public:
  MainPanel( DBEdtEngine &in_engine, QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~MainPanel();

 public slots:
  void fileOpen();
  void fileClose();
  void fileNew();
  void fileAction(int actionType);
  void showSingle();
  void showDouble();
  void selectView(bool);
  void rightSelection(QTreeWidgetItem*);
  void leftSelection(QTreeWidgetItem*);
  void clearSelection();
  void changeCurrent(DBTreeItem *item);
  void addDBField();
  void addDBInquire();
  void openEditMenu(const QPoint &);
  void deleteDBObject();
  void editCopy();
  void editCut();
  void editPaste();
  void editPasteTree(bool rename);
  void editPasteTreeAsis(){editPasteTree(false);};
  void editPasteTreeRename(){editPasteTree(true);};

 signals:
  void fileListChanged();
  void contentChanged();

 private:
  DBEdtEngine &m_engine;
  DBTreeView *m_leftTreeView,*m_rightTreeView;
  DBTreeItem *m_currentItem, *m_copyItem, *m_cutItem;
  QSplitter *m_split;
};


#endif // MAINPANEL_H
