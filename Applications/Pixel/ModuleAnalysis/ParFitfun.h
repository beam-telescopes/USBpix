#ifndef PARFITFUN_H
#define PARFITFUN_H

#define NP_BOX_MAX 6

#include "ui_ParFitfunBase.h"
#include <vector>
//Added by qt3to4:
#include <QLabel>

class PixDBData;
class QComboBox;
class QLabel;

class ParFitfun : public QDialog, public Ui::ParFitfunBase{

  Q_OBJECT

 public:
  ParFitfun(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0, PixDBData *data=0);
  ~ParFitfun();

  int getPars(std::vector<int> &pars){return getPars(pars,false);};
  int getPars(std::vector<int> &pars, bool);

 public slots:
  void genHistoBoxes(int fid){genHistoBoxes(fid, true, false);};
  void genHistoBoxes(int, bool, bool);

 private:
  PixDBData *m_data;
  QComboBox *m_boxes[NP_BOX_MAX];
  QLabel *m_labels[NP_BOX_MAX];
};

#endif // PARFITFUN_H
