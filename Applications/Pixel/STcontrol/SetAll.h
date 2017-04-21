#ifndef SETALL_H
#define SETALL_H

#include "ui_SetAll.h"

class SetAll : public QDialog, public Ui::SetAll
{

 Q_OBJECT

 public:
  SetAll(QWidget* parent = 0, Qt::WindowFlags fl = 0):QDialog(parent,fl){setupUi(this);};
  ~SetAll(){};

};
#endif // SETALL_H
