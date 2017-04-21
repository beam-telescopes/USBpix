#ifndef GRVIEW_H
#define GRVIEW_H

#include "ui_GRView.h"


class GRView : public QDialog, public Ui::GRView
{
    Q_OBJECT

public:

  GRView(QString text, QWidget* parent = 0, Qt::WindowFlags fl = 0, int type=0);
  ~GRView();

};
#endif // GRVIEW_H
