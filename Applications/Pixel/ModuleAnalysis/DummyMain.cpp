#include "DummyMain.h"
#include <qwidget.h>
#include "TopWin.h"

DummyMain::DummyMain( QWidget* parent, Qt::WFlags fl, QApplication *app)
  : QWidget(parent, fl){
  m_tw = new TopWin(this,"MAmain",0,false, app);
  QObject::connect(m_tw, SIGNAL(closed()), this, SLOT(close()));
}

