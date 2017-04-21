#ifndef DAENGINE_H
#define DAENGINE_H

#include <iostream>
#include "mainwin.h"
#include "ModItem.h"
#include "Module.h"
#include "cuts.h"
#include <ModuleMap.h>

#include <PixDBData.h>



class DAEngine
{

  friend class MainWin;
  friend class cutPanel;

 public:
  //Methoden
  DAEngine(bool gui = true);
  ~DAEngine();
  void loadFile(const char *filename);//load MA RootDB files
  void loadCuts(const char *filename);
  void startAnalyse(const char *);
  void getCuts(std::map<int, std::vector<Cut *> > &cutsmap);
  void showMainWin();
  void showCutPanel(  );
  std::string getModName(PixLib::DBInquire *inq);
  Modul *findModule(const char *);
  void updateScanList(const char *);//add scan to m_scans
  bool findScan(const char *sc);
  void clearModule(bool all, QString);
  void saveCuts(const char *file);//save Cuts in file
  
  
 private:
  //Methoden
  QString testName(const char *testname);
  std::vector<Cut *> sort_m_cuts();
  Cut * findCut(const char *cutname,const char *testtype);

  //Member
  std::vector<Modul *> m_module;
  bool m_gui;
  //  MainWin *m_mainwin;
  //  CutPanel *m_cutpanel;
  std::vector<Cut *> m_cuts;
  std::vector<string> m_scans;
  std::map<int, std::vector<Modul *> > m_staves;
  std::map<std::string, std::pair< std::string, int > > m_structList;
  std::map<std::string, ModuleMap *> m_mmaps;
  std::map<std::string, int> m_scanTypes;
};


#endif
