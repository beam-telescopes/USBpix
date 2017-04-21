#ifndef PSFITWIN_H
#define PSFITWIN_H

#include "ui_PSFitWinBase.h"
//Added by qt3to4:
#include <QLabel>

#define NPMAX 5

class QLabel;
class QDoubleSpinBox;
class QCheckBox;
class ModItem;
class FitClass;
class PixDBData;

class PSFitWin : public QDialog, public Ui::PSFitWinBase
{
 Q_OBJECT
  
 public:
  PSFitWin( QWidget* parent, const char* name, bool modal, Qt::WFlags fl, ModItem *item );
  PSFitWin( QWidget* parent, const char* name, bool modal, Qt::WFlags fl, PixDBData *data );
  ~PSFitWin();

  FitClass *m_fitClass;
  QDoubleSpinBox *m_parValues[NPMAX];
  QCheckBox *m_parFixed[NPMAX];

 public slots:
  void fitfunSel(int);
  void binomClicked();
  void anotherClicked();
  void fractClicked();

 private:
  QLabel *m_parLabels[NPMAX];
  
};
#endif // PSFITWIN_H
