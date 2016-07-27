#ifndef PRLITEMINIT_H
#define PRLITEMINIT_H

#include "ui_PrlItemInit.h"

class STControlEngine;


class PrlItemInit : public QDialog, public Ui::PrlItemInit
{
    Q_OBJECT

 public:
  PrlItemInit( STControlEngine& , QWidget* parent = 0, Qt::WindowFlags fl = 0 );
  ~PrlItemInit(){};

 public slots:
  void setType(int);

 private:
  STControlEngine &m_engine;

};

#endif // PRLITEMINIT_H
