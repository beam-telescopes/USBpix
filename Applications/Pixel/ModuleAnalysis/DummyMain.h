#ifndef DUMMY_MAIN_H
#define DUMMY_MAIN_H

#include "ui_DummyMain.h"

class TopWin;
class QApplication;

class DummyMain : public QWidget, public Ui::DummyMain{
  Q_OBJECT
 public:
  DummyMain( QWidget* parent = 0, Qt::WFlags fl = 0, QApplication *app=0);
  ~DummyMain(){};

  TopWin *m_tw;
};

#endif // DUMMY_MAIN_H
