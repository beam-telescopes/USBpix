#ifndef FITWIN_H
#define FITWIN_H

#include "ui_FitWin.h"

#define NPMAX 5

class QLabel;
class QDoubleSpinBox;
class QCheckBox;
class ModItem;
class FitClass;
class PixDBData;

class FitWin : public QDialog, public Ui::FitWin
{
 Q_OBJECT
  
 public:
 FitWin(PixDBData &data, FitClass &fc, QWidget* parent=0, Qt::WindowFlags fl = 0, int htype=-1, int loopLev=-1);
  ~FitWin();

  QDoubleSpinBox *m_parValues[NPMAX];
  QCheckBox *m_parFixed[NPMAX];

 public slots:
  void fitfunSel(int);
  void binomClicked();
  void anotherClicked();
  void fractClicked();

 private:
  FitClass &m_fitClass;
  QLabel *m_parLabels[NPMAX];
  
};
#endif // FITWIN_H
