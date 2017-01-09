#ifndef PIXSCANPANEL
#define PIXSCANPANEL

#include "ui_PixScanPanel.h"
#include "STControlEngine.h"
#include <PixController/PixScan.h>

#include <map>
#include <string>

class QObject;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QListWidget;
class QPushButton;
class QLabel;
class QGroupBox;
class STControlEngine;
class STRodCrate;
class QString;
class QTableWidgetItem;

class PixScanPanel : public QWidget, public Ui::PixScanPanel{

  Q_OBJECT

 public:
  PixScanPanel( STControlEngine &engine_in, QWidget* parent = 0, Qt::WindowFlags fl = 0, bool editor=false);
  ~PixScanPanel();

  void setStdScanCfg(int scan_type, int fe_type);

 public slots:
  void updateConfig();
  void updateConfigSel();
  void updateConfigHandlers();
  /* creates a new PixScan object for user-config */
  void newConfig();
  void loadConfig();
  void clearConfig();
  void saveConfig();
  void readFromHandles();
  void readFromHandlersFromTable(int row, int);
  void updateLoopPts(int);
  void updateLoopSettings(int);
  void loadLoopPts(int);
  void loadLoop0Pts(){loadLoopPts(0);};
  void loadLoop1Pts(){loadLoopPts(1);};
  void loadLoop2Pts(){loadLoopPts(2);};
  void fileEnabled(bool);
  void setEvtLabel(QString txt);

  void showLoop(int);
  void updateStatus(int nSteps0, int nSteps1, int nSteps2, int nMasks, int in_currFe, int sramFillLevel, int triggerRate, int eventRate, int status);
  void runButton_clicked(bool tbmode);
  void runButton_clicked(){runButton_clicked(false);};
  void extStartScan(){runButton_clicked(true);};
  void extStartScan(QString scan_label, QString raw_filename){scanLabel->setText(scan_label); rawFileName->setText(raw_filename); runButton_clicked(true);};
  void extScanConfig(extScanOptions options);
  void browseButton_clicked(const char *in_path);
  void browseButton_clicked(){browseButton_clicked(0);};
  void rawBrowseButton_clicked(const char *in_path);
  void rawBrowseButton_clicked(){rawBrowseButton_clicked(0);};
  void updateCrateGrpSel();
  void clearScanLabelMemory(){m_scanLabels.clear();};
  // updates the observedRod-Object of m_engine
  void observedRodChanged(int index);
  // updates the observedCrate-Object of m_engine
  void observedCrateChanged(int index); 
  // sets VCAL to the "magic" value to leave it unchanged during a scan
  void fixVCAL(bool);
  // if VCAL is set to this value manually, translate accordingly
  void VcalChanged(int);
  // ditto for MCC strobe delay
  void fixSdelay(bool);
  void SdelayChanged(int);
  // right-click on a config table item: only works for combo-boxes in last column
  void openTableMenu(QTableWidgetItem*);
  void scanRunning(int type);
  void scanRunning(){scanRunning(0);};
  void enableRawFileStuff(bool);

  void toggleClusHis();
  void goToTbMode(bool tbMode);

  void setSrcMonBox();

  void setFeReg0(){setFeReg(0);};
  void setFeReg1(){setFeReg(1);};
  void setFeReg2(){setFeReg(2);};
  void setFeReg(int);
  void updateFegrSel();
  void setCCPDReg0(){setCCPDReg(0);};
  void setCCPDReg1(){setCCPDReg(1);};
  void setCCPDReg2(){setCCPDReg(2);};
  void setCCPDReg(int);
  void updateCCPDgrSel();
  void showAddModLabels(bool);

  // Slots to set config parameters
  void setScanValue(std::string parameter, bool value);
  void setScanValue(std::string parameter, int value);

 signals:
  void enableBocAnaScan(bool flag);
  void scanFileChanged(const char*);

 private:
  STControlEngine & m_engine;
  PixLib::PixScan *m_stdScan, *m_scanCfg;
  std::map<std::string,QObject*> m_knownHandles;
  std::vector<QString> m_scanLabels;
  // translates individual loop_i variable names to arrays
  QComboBox* loopType[MAX_LOOPS];
  QComboBox* loopEndAction[MAX_LOOPS];
  QCheckBox* loopActive[MAX_LOOPS];
  QCheckBox* loopDspProc[MAX_LOOPS];
  QCheckBox* loopRegular[MAX_LOOPS];
  QCheckBox* loopFree[MAX_LOOPS];
  QCheckBox* loopOnDsp[MAX_LOOPS];
  QDoubleSpinBox*  loopStart[MAX_LOOPS];
  QDoubleSpinBox*  loopStop[MAX_LOOPS];
  QSpinBox*  loopStep[MAX_LOOPS];
  QListWidget*  loopPtsBox[MAX_LOOPS];
  QPushButton* loopLoadPoints[MAX_LOOPS];
  QLabel*    loopFromLabel[MAX_LOOPS];
  QLabel*    loopToLabel[MAX_LOOPS];
  QLabel*    loopStepsLabel[MAX_LOOPS];
  QLabel*    loopPtlLabel[MAX_LOOPS];
  QLabel*    loopPostActLabel[MAX_LOOPS];
  QGroupBox* LoopBox[MAX_LOOPS];
  QComboBox* loopFERegName[MAX_LOOPS];
  QComboBox* loopCCPDRegName[MAX_LOOPS];
  QLabel*    fillingLabel;

  // estimate remaining time:
  int	m_MeasurementStartTime;
  int	m_StatusUpdatesRecieved;
  double	m_MeasurementStartPercentage;
  double	m_lastPercentage;

  // integer holding the current std. scan ID (-1 of not standard)
  int m_currStdScanID;
  // map of PixScan pre-sets and std. scan IDs
  std::map<PixLib::PixScan::ScanType, int> m_stdScanIDs;
};

#endif // PIXSCANPANEL

