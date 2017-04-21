#ifndef MAINPANEL
#define MAINPANEL

#include "ui_MainPanel.h"
//#include <GeneralDBfunctions.h>

class DataViewer;
class PixScanPanel;
class RegisterPanel;
class ModList;
class ModListItem;
class QTreeWidgetItem;
class QSplitter;
class STControlEngine;
class ToolPanel;
class LogPanel;
class STCLogContainer;
class QString;
#ifndef NOTDAQ
class BocAnalysisPanel;
#endif
class PrimlistPanel;
class WaferTestPanel;
class STRodCrate;

class MainPanel : public QMainWindow, public Ui::MainPanel {
  
  Q_OBJECT

 public:
  MainPanel( STControlEngine &engine_in, STCLogContainer &log_in, QWidget* parent = 0, Qt::WindowFlags fl = 0, bool smallScreen=false);
  ~MainPanel();

 public slots:
  void checkClose();
  void showModList();
  void showDcsView();
  void showData();
  void showPixScan();
  void showRegister();
  void showBocAnalysis();
  void showPrimList();
  void showWaferTest();
  void setShowMenues();
  void newConfig();
  void newConfigGeneric();
  void addDcs(ModListItem*);
  void saveConfig(){saveConfig(0);};
  void saveConfig(const char *path);
  void saveAs();
  void reopenConfig();
  void loadConfig(){loadConfig(0);};
  void loadConfig(const char *path);
  void showInfo();
  void editCurrentModule();
  void setCurrentModule(QTreeWidgetItem *item, int col);
  void setCurrentModule(QTreeWidgetItem *item){setCurrentModule(item, 0);};
  /** Close the current configuration file and clear STCEngine. */
  bool b_closeConfig();
  void closeConfig(){b_closeConfig();};
  void showTools();
  void showLogs();
  void decrGDAC(){decrGDAC(-1);};
  void decrTDAC(){decrTDAC(-1);};
  void decrGDAC(int step);
  void decrTDAC(int step);
  void decrGDACBy();
  void decrTDACBy();
  void setVcal();
  void incrMccDelay();
  void setTDACs(){setTFDACs(true);};
  void setFDACs(){setTFDACs(false);};
  void setTFDACs(bool isTDAC);
  void setMasks();
  void showAbout();
  void showReadme();
  void showOptions();
  void saveOptions();
  void loadOptions();
  void setSingleFe();
  void AnalyseBocData(const char* filename, const char* scanlabel );
  void showPopupWin(const char*);
  void setScanBusy();
  void setScanDone();
  void setMyTitle();

 private:
  QSplitter     *m_split;
  QSplitter     *m_split_left_panel;
  QWidget       *m_cmnwid;
  ModList       *m_modList, *m_dcsList;
  PixScanPanel  *m_pixScanPanel;
#ifndef NOTDAQ
  BocAnalysisPanel *m_bocPanel;
#endif
  DataViewer    *m_dataPanel;
  RegisterPanel *m_regPanel;
  PrimlistPanel *m_prlPanel;
  WaferTestPanel *m_prbPanel;
  /** Main engine. Also a reference*/
  STControlEngine &m_engine;
  ToolPanel     *m_toolPanel;
  /** Panel for displaying (log-) text */
  LogPanel      *m_logPanel;
  /** Logging facility */
  STCLogContainer &m_sTCLogContainer;
  ModListItem *m_currentListItem;
  bool m_scanActive;
};

#endif // MAINPANEL
