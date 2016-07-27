#ifndef FITSTATUS_H
#define FITSTATUS_H

#include "ui_FitStatus.h"
#include <QDialog>

class FitStatus : public QDialog, public Ui::FitStatus
{
 Q_OBJECT
  
 public:
 FitStatus(QWidget* parent=0, Qt::WindowFlags fl = 0) : QDialog(parent, fl){setupUi(this);};
 ~FitStatus(){};

};


#endif // FITSTATUS_H
