/***************************************************************************
                          WaferTestPanel.h  -  description
                             -------------------
    begin                : Wed 22 Dec 2010
    copyright            : (C) 2010 by jgrosse
    email                : jgrosse1@uni-goettingen.de
    modifications        : 
 ***************************************************************************/
#ifndef WAFERTESTPANEL_H
#define WAFERTESTPANEL_H

#include "ui_WaferTestPanel.h"

class  STControlEngine;
class PrimlistPanel;
namespace PixLib{
  class PixProber;
}

class WaferTestPanel : public QWidget, public Ui::WaferTestPanel {
  Q_OBJECT

 public: 
  WaferTestPanel( STControlEngine &engine_in, QWidget* parent = 0 );
  virtual ~WaferTestPanel();

 public slots:
  void newProber();
  void editPrbCfg();
  void deleteProber();
  void loadProber();
  void saveProber();
  void browseOutput();
  void startProbing();
  void abortProbing();
  void finishProbing();
  void probeStepInit(bool);
  void probeStepInit(){probeStepInit(false);};
  void enableSDbox(int labelType);

 private slots:
   void checkReadyForProbing(){checkReadyForProbing(true);};
  void checkReadyForProbing(bool);

 private:
  STControlEngine &m_engine;
  PrimlistPanel *m_prlPan;
  PixLib::PixProber *m_prober;

  int m_nChips;
  bool m_abortFlag;

};


#endif // WAFERTESTPANEL_H
