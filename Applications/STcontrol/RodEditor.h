#ifndef RODEDITOR_H
#define RODEDITOR_H

#include "ui_RodEditor.h"
#include <vector>
#include <qapplication.h>
#include "PixBoc/PixBoc.h"

class STControlEngine;
class STRodCrate;
class USpinBox;
namespace PixLib{
  class Config;
}

class RodEditor : public QDialog, public Ui::RodEditor {
  
  Q_OBJECT

 public:
  RodEditor(STControlEngine &engine_in, STRodCrate *crate = 0, QWidget* parent = 0, Qt::WindowFlags fl = 0);
  RodEditor(PixLib::Config &cfg, STControlEngine &engine_in, STRodCrate *crate = 0, QWidget* parent = 0, Qt::WindowFlags fl = 0);
  RodEditor(PixLib::Config &cfg, PixLib::Config &bocCfg, STControlEngine &engine_in, STRodCrate *crate = 0, 
	    QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~RodEditor();
  std::vector<int> loaded_RODslots;
  USpinBox *fmtLinks[8][4];
  
  void setBocOK(bool flag){m_bocOK = flag; return;};

 public slots:
  void listRODs(bool only_present=true);
  void browseSlave();
  void setFMTdefault(int);
  void loadCfg();
  void saveCfg();

 private:
  void mapFmtBoxes();

  STControlEngine &m_engine;
  STRodCrate *m_crate;
  PixLib::Config &m_cfg;
  // needed to set BocModus when RodModus changed
  PixLib::Config &m_bocCfg;
  bool m_bocOK;

};

#endif // RODEDITOR_H
