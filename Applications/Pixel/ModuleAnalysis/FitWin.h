#ifndef FITWIN_H
#define FITWIN_H

#include "ui_FitWinBase.h"
//Added by qt3to4:
#include <QLabel>
//#include "RootStuff.h"
//#include "TopWin.h"

#define NPARMAX 10

class QLabel;
class QCheckBox;
class QLineEdit;
class RMain;

/*! panel for initialising fits to scan data like ToT calibration,
 *  S-curve fit, polynomials etc.
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 */

class FitWin : public QDialog, public Ui::FitWinBase
{
    Q_OBJECT

 public:
  
  FitWin( QWidget* parent = 0, Qt::WFlags fl = 0 );
  ~FitWin();
  RMain *rootmain;
  
  QLineEdit* SetPar[NPARMAX];
  QCheckBox* FixPar[NPARMAX];
  QLabel* TextPar[NPARMAX];

 public slots:
  void CPCButt_pressed();
  void safe_accept();
  void Load_Clicked();
  void FitSelected(int);
  void setCalTable(float **calfac);
};

#endif // FITWIN_H
