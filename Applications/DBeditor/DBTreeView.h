#ifndef DBTREEVIEW_H
#define DBTREEVIEW_H

#include "ui_DBTreeView.h"
#include <QTreeWidget>

class DBTreeItem;
class DBEdtEngine;
class QMouseEvent;
class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;

class DBListView : public QTreeWidget
{
  Q_OBJECT

 public:
  DBListView(QWidget* parent = 0);
  ~DBListView(){};

 protected:
  void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
  void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
  void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

 protected slots:
  void prepareDrag();

 private:
  bool m_leftPressed;
  bool m_dcFlag;
  bool m_relFlag;
  bool m_inTimer;
};


class DBTreeView : public QWidget, public Ui::DBTreeView
{

  Q_OBJECT

 public:
  DBTreeView( DBEdtEngine &in_engine, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
  ~DBTreeView();

  DBListView *m_listView;

 public slots:
  void updateList();
  void fillView();

 private:
  DBEdtEngine &m_engine;

};


#endif // DBTREEVIEW_H
