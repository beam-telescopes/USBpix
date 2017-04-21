#ifndef MASKMAKER_H
#define MASKMAKER_H

#include "ui_MaskMaker.h"

class MaskMaker : public QDialog, public Ui::MaskMaker
{
    Q_OBJECT

 public:
    MaskMaker( QWidget* parent = 0, Qt::WFlags fl = 0);
    ~MaskMaker();
    
    void   setHistoLabel(int);
    double getMin();
    double getMax();

};

#endif // MASKMAKER_H
