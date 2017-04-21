#ifndef TOTRECAL
#define TOTRECAL

#include "ui_TotRecalBase.h"

class TopWin;
class ModItem;

/*! panel that allows relative calibration - manually or
 *  fitted - of Chi vs Clo ToT-vs-VCAL fits
 */

class TotRecal : public QDialog, public Ui::TotRecalBase{

  Q_OBJECT

 public:
  TotRecal(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 , ModItem *item = NULL);
  ~TotRecal(){return;};
  
 public slots:
  virtual void FitRelCal();
  virtual void LoadCal();
  
 private:
  ModItem *m_item, *m_plotit;
  TopWin *m_parent;
};
#endif // TOTRECAL
