/***************************************************************************
                          BocAnalysisPanel.h  -  description
                             -------------------
    begin                : Dez 9 2005
    author               : by Iris Rottlaender
  ***************************************************************************/


#ifndef BOCANALYSISPANEL
#define BOCANALYSISPANEL

#include "ui_BocAnalysisPanel.h"
#include <string>
#include <vector>
#include <stdio.h>

class STControlEngine;
class STPixModuleGroup;

class BocAnalysisPanel : public QWidget, public Ui::BocAnalysisPanel{

  Q_OBJECT
    
    public:

  BocAnalysisPanel(STControlEngine &engine_in, QWidget* parent, const char* name, Qt::WindowFlags fl);
  ~BocAnalysisPanel();

  public slots:
  void Analyse(std::string filename, std::string scanlabel );  
  void Analyse(){Analyse("","");};
  void Browse1(){Browse(0, anaFileName);};
  void Browse2(){Browse(0, scanFileName);};
  void Browse(const char *in_path, QLineEdit* field);
  void OpenFile();
  void AnaScan();
  void FinishAnaScan();
  void WriteData();
  void updateCrateGrpSel();
  void enableAnaScan(bool flag);

 private:
  STControlEngine & m_engine;
  std::vector<std::string> m_scanLabels;
  std::string m_dataPath;
  std::vector<std::string> currentScanPars;
  std::vector<std::vector<int> > writeResults;
  std::vector<std::vector<int> > writeLinks;
  std::vector<std::string> writeSlots;
  QTimer *m_scanTimer;
  std::string scanfilename;
  std::string scanlabelname;
  bool newfileflag;

   //vector containing a vector of ModuleGroups for each crate in cratelist
  // used for GUI-updating
  std::vector<std::vector<STPixModuleGroup*> > CrateRodList;

 signals:
  /** emitted when an error message should be forwarded to somewhere */
  void errorMessage(std::string);
  void bocConfigChanged();
  void bocScanRunning();
  // emitted when scan panel should be updated
  void sendPixScanStatus(int, int, int, int, int, int, int, int, int);
};




#endif

