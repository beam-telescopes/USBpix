#ifndef MODULECFGPAGE_H
#define MODULECFGPAGE_H

#include "ui_ModuleCfgPageBase.h"

class ModuleCfgPage : public QWidget, public Ui::ModuleCfgPageBase{ 
  Q_OBJECT

 public:
  ModuleCfgPage( QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0 , int setup_type=0, int pp0_type=0);
  ~ModuleCfgPage();
 public slots:
  void browseClicked();
  void defineSetup(int type, int pp0);
  void slotChanged(int slot);
  void setStavePosMax();
 
 signals:
  void haveFile();

};

#endif // MODULECFGPAGE_H
