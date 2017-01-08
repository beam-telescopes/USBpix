#include "PixScanPanel.h"
#include "STRodCrate.h"

#include <PixConfDBInterface/PixConfDBInterface.h>
#include <GeneralDBfunctions.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixFe/PixFeStructures.h>
#include <PixFe/PixFeData.h>
#include <PixFe/PixFeI1Config.h>
#include <PixFe/PixFeI4AConfig.h>
#include <PixFe/PixFeI4BConfig.h>
#include <PixModule/PixModule.h>

#include <RenameWin.h>
#include <DBEdtEngine.h>

#include <QObject>
#include <QPushButton>
#include <QString>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox> 
#include <QMessageBox>
#include <QListWidget>
#include <QComboBox>
#include <QVariant>
#include <QTableWidget>
#include <QFileDialog>
#include <QApplication>
#include <QLCDNumber>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QLabel>
#include <QToolTip>
#include <QDateTime>

#include <sstream>

PixScanPanel::PixScanPanel( STControlEngine &engine_in, QWidget* parent, Qt::WindowFlags fl, bool editor )
  : QWidget(parent,fl), m_engine( engine_in ), m_lastPercentage(0)
{

  setupUi(this);
  QObject::connect(scanFileBrowse, SIGNAL(clicked()), this, SLOT(browseButton_clicked()));
  QObject::connect(rawFileBrowse, SIGNAL(clicked()), this, SLOT(rawBrowseButton_clicked()));
  QObject::connect(clearCfgButton, SIGNAL(clicked()), this, SLOT(clearConfig()));
  QObject::connect(clearLabelMem, SIGNAL(clicked()), this, SLOT(clearScanLabelMemory()));
  QObject::connect(saveFile, SIGNAL(toggled(bool)), this, SLOT(fileEnabled(bool)));
  QObject::connect(fixedSdelBox, SIGNAL(toggled(bool)), this, SLOT(fixSdelay(bool)));
  QObject::connect(fixedVCALBox, SIGNAL(toggled(bool)), this, SLOT(fixVCAL(bool)));
  QObject::connect(loadCfgButton, SIGNAL(clicked()), this, SLOT(loadConfig()));
  QObject::connect(loop0LoadPoints, SIGNAL(clicked()), this, SLOT(loadLoop0Pts()));
  QObject::connect(loop1LoadPoints, SIGNAL(clicked()), this, SLOT(loadLoop1Pts()));
  QObject::connect(loop2LoadPoints, SIGNAL(clicked()), this, SLOT(loadLoop2Pts()));
  QObject::connect(newCfgButton, SIGNAL(clicked()), this, SLOT(newConfig()));
  QObject::connect(configParameterTable, SIGNAL(itemClicked (QTableWidgetItem*)), this, SLOT(openTableMenu(QTableWidgetItem*)));
  QObject::connect(configParameterTable, SIGNAL(cellChanged(int,int)), this, SLOT(readFromHandlersFromTable(int,int)));
  QObject::connect(runButton, SIGNAL(clicked()), this, SLOT(runButton_clicked()));
  QObject::connect(saveCfgButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
  QObject::connect(strDelRange, SIGNAL(valueChanged(int)), this, SLOT(SdelayChanged(int)));
  QObject::connect(showLoopIndex, SIGNAL(valueChanged(int)), this, SLOT(showLoop(int)));
  QObject::connect(scanConfigSel, SIGNAL(activated(QString)), this, SLOT(updateConfig()));
  QObject::connect(i4Button, SIGNAL(toggled(bool)), this, SLOT(updateConfig()));
  QObject::connect(setVcal, SIGNAL(valueChanged(int)), this, SLOT(VcalChanged(int)));
  QObject::connect(saveFile, SIGNAL(toggled(bool)), this, SLOT(enableRawFileStuff(bool)));
  QObject::connect(appIndRawfile, SIGNAL(toggled(bool)), this, SLOT(enableRawFileStuff(bool)));
  QObject::connect(toggleClusButton,SIGNAL(clicked()), this, SLOT(toggleClusHis()));
  QObject::connect(srcEventCount, SIGNAL(activated(QString)), this, SLOT(setEvtLabel(QString)));
  QObject::connect(addModNameBox, SIGNAL(toggled(bool)), this, SLOT(showAddModLabels(bool)));
  QObject::connect(tbModeBox  , SIGNAL(toggled(bool)), this, SLOT(goToTbMode(bool)));
  QObject::connect(appIndRawfile, SIGNAL( toggled(bool) ), this, SLOT( setSrcMonBox() ) );
  QObject::connect(rawFileName, SIGNAL( textChanged(const QString&) ), this, SLOT( setSrcMonBox() ) );

  // not needed anymore, but for the moment just hide this button
  clearLabelMem->hide();
  
  // create one PixScan object so the user can at least use default scans
  m_stdScan = new PixScan(PixScan::DIGITAL_TEST, PixModule::PM_FE_I4B);
  m_scanCfg = m_stdScan;
  m_currStdScanID = -1;

  // initialise variables for rem. time estimation
  m_StatusUpdatesRecieved=0;
  m_MeasurementStartTime=-1;

  connect( &m_engine, SIGNAL(configChanged()), this, SLOT(updateFegrSel()) );
  connect( &m_engine, SIGNAL(crateListChanged()), this, SLOT(updateCrateGrpSel()) );
  connect( &m_engine, SIGNAL(sendPixScanStatus(int,int,int,int,int,int,int,int,int)), this, SLOT(updateStatus(int,int,int,int,int,int,int,int,int)));
  connect( rodBox, SIGNAL(activated(int)), this , SLOT(observedRodChanged(int)));
  connect( crateBox, SIGNAL(activated(int)), this , SLOT(observedCrateChanged(int)));
  connect( &m_engine, SIGNAL(startCurrentScan(QString, QString)), this , SLOT(extStartScan(QString, QString)));
  connect( &m_engine, SIGNAL(setScanOptions(extScanOptions)), this , SLOT(extScanConfig(extScanOptions)));

  // set default data path/name
  ConfGroup &grp = m_engine.getOptions()["paths"];
  std::string dpath, dname;
  dpath = ((ConfString&)grp["defDataPath"]).value();
  dname = ((ConfString&)grp["defDataName"]).value();
  bool appDate = ((ConfBool&)grp["addDatetoDataFile"]).value();
  if(dname!=""){
    QString fullPath = (dpath+"/"+dname).c_str();
    if(appDate)
      fullPath+="_"+QDateTime::currentDateTime().toString("ddMMMyy");
    fullPath+=".root";
    browseButton_clicked(fullPath.toLatin1().data());
  }

  // translates individual loop_i variable names to arrays
  for(int sci=0;sci<MAX_LOOPS;sci++){
    loopType[sci] = 0;
    loopEndAction[sci] = 0;
    loopActive[sci] = 0;
    loopDspProc[sci] = 0;
    loopRegular[sci] = 0;
    loopFree[sci] = 0;
    loopOnDsp[sci] = 0;
    loopStart[sci] = 0;
    loopStop[sci] = 0;
    loopStep[sci] = 0;
    loopPtsBox[sci] = 0;
    loopLoadPoints[sci] = 0;
    loopFromLabel[sci] = 0;
    loopToLabel[sci] = 0;
    loopStepsLabel[sci] = 0;
    loopPtlLabel[sci] = 0;
    loopPostActLabel[sci] = 0;
    LoopBox[sci] = 0;
    loopFERegName[sci] = 0;
  }

  loopType[0] = loop0Type;
  loopEndAction[0] = loop0EndAction;
  loopActive[0] = loop0Active;
  loopDspProc[0] = loop0DspProc;
  loopRegular[0] = loop0Regular;
  loopFree[0] = loop0Free;
  loopOnDsp[0] = loop0OnDsp;
  loopStart[0] = loop0Start;
  loopStop[0] = loop0Stop;
  loopStep[0] = loop0Step;
  loopPtsBox[0] = loop0PtsBox;
  loopLoadPoints[0] = loop0LoadPoints;
  loopFromLabel[0] = loop0FromLabel;
  loopToLabel[0] = loop0ToLabel;
  loopStepsLabel[0] = loop0StepsLabel;
  loopPtlLabel[0] = loop0PtlLabel;
  loopPostActLabel[0] = loop0PostActLabel;
  LoopBox[0] = Loop0Box;
  loopFERegName[0] = feGlobRegName0;
  loopType[1] = loop1Type;
  loopEndAction[1] = loop1EndAction;
  loopActive[1] = loop1Active;
  loopDspProc[1] = loop1DspProc;
  loopRegular[1] = loop1Regular;
  loopFree[1] = loop1Free;
  loopOnDsp[1] = loop1OnDsp;
  loopStart[1] = loop1Start;
  loopStop[1] = loop1Stop;
  loopStep[1] = loop1Step;
  loopPtsBox[1] = loop1PtsBox;
  loopLoadPoints[1] = loop1LoadPoints;
  loopFromLabel[1] = loop1FromLabel;
  loopToLabel[1] = loop1ToLabel;
  loopStepsLabel[1] = loop1StepsLabel;
  loopPtlLabel[1] = loop1PtlLabel;
  loopPostActLabel[1] = loop1PostActLabel;
  LoopBox[1] = Loop1Box;
  loopFERegName[1] = feGlobRegName1;
  loopType[2] = loop2Type;
  loopEndAction[2] = loop2EndAction;
  loopActive[2] = loop2Active;
  loopDspProc[2] = loop2DspProc;
  loopRegular[2] = loop2Regular;
  loopFree[2] = loop2Free;
  loopOnDsp[2] = loop2OnDsp;
  loopStart[2] = loop2Start;
  loopStop[2] = loop2Stop;
  loopStep[2] = loop2Step;
  loopPtsBox[2] = loop2PtsBox;
  loopLoadPoints[2] = loop2LoadPoints;
  loopFromLabel[2] = loop2FromLabel;
  loopToLabel[2] = loop2ToLabel;
  loopStepsLabel[2] = loop2StepsLabel;
  loopPtlLabel[2] = loop2PtlLabel;
  loopPostActLabel[2] = loop2PostActLabel;
  LoopBox[2] = Loop2Box;
  loopFERegName[2] = feGlobRegName2;

  // enabling of these
  connect( loop0Type,   SIGNAL( activated(int) ), this, SLOT( setFeReg0() ) );
  connect( loop0Active, SIGNAL( toggled(bool) ),  this, SLOT( setFeReg0() ) );
  connect( loop1Type,   SIGNAL( activated(int) ), this, SLOT( setFeReg1() ) );
  connect( loop1Active, SIGNAL( toggled(bool) ),  this, SLOT( setFeReg1() ) );
  connect( loop2Type,   SIGNAL( activated(int) ), this, SLOT( setFeReg2() ) );
  connect( loop2Active, SIGNAL( toggled(bool) ),  this, SLOT( setFeReg2() ) );

  // fill map of known scan option handles
  if(!editor) m_knownHandles.insert(std::make_pair("general_sourceRawFile",(QObject*)rawFileName));
  m_knownHandles.insert(std::make_pair("general_sourceScanFlag",(QObject*)sourceScan));
  m_knownHandles.insert(std::make_pair("general_repetitions",(QObject*)nEvents));
  m_knownHandles.insert(std::make_pair("general_srcCountType", (QObject*)srcEventCount));
  m_knownHandles.insert(std::make_pair("general_srcTriggerType", (QObject*)trgTypeBox));
  m_knownHandles.insert(std::make_pair("general_restoreModuleConfig",(QObject*)restorConfig));
  m_knownHandles.insert(std::make_pair("general_maskStageSteps",(QObject*)nMaskSteps));
  m_knownHandles.insert(std::make_pair("general_maskStageStepWidth",(QObject*)maskStepWidth));
  m_knownHandles.insert(std::make_pair("general_modScanConcurrent",(QObject*)concurrMode));
  m_knownHandles.insert(std::make_pair("general_maskStageTotalSteps",(QObject*)maskStage));
  m_knownHandles.insert(std::make_pair("general_maskStageMode",(QObject*)maskMode));
  m_knownHandles.insert(std::make_pair("trigger_consecutiveLevl1TrigA_0",(QObject*)nAccepts_A0));
  m_knownHandles.insert(std::make_pair("trigger_consecutiveLevl1TrigB_0",(QObject*)nAccepts_B0));
  m_knownHandles.insert(std::make_pair("trigger_consecutiveLevl1TrigA_1",(QObject*)nAccepts_A1));
  m_knownHandles.insert(std::make_pair("trigger_consecutiveLevl1TrigB_1",(QObject*)nAccepts_B1));
  m_knownHandles.insert(std::make_pair("trigger_strobeLVL1Delay",(QObject*)l1Delay));
  m_knownHandles.insert(std::make_pair("trigger_LVL1Latency",(QObject*)trgLatency));
  m_knownHandles.insert(std::make_pair("trigger_lvl1HistoBinned",(QObject*)lvl1Binned));
  m_knownHandles.insert(std::make_pair("trigger_strobeLVL1DelayOverride",(QObject*)overwL1delBox));
  m_knownHandles.insert(std::make_pair("trigger_superGroupTrigDelay",(QObject*)superGrpDelay));
  m_knownHandles.insert(std::make_pair("trigger_trigABDelay_0",(QObject*)delayAB0));
  m_knownHandles.insert(std::make_pair("trigger_trigABDelay_1",(QObject*)delayAB1));
  m_knownHandles.insert(std::make_pair("trigger_self",(QObject*)selfTrigger));
  m_knownHandles.insert(std::make_pair("trigger_strobeDuration",(QObject*)strDuration));
  m_knownHandles.insert(std::make_pair("trigger_strobeFrequency",(QObject*)strFrequency));
  m_knownHandles.insert(std::make_pair("trigger_strobeMCCDelay",(QObject*)strDelay));
  m_knownHandles.insert(std::make_pair("trigger_strobeMCCDelayRange",(QObject*)strDelRange));
  m_knownHandles.insert(std::make_pair("fe_digitalInjection",(QObject*)injectionMode));
  m_knownHandles.insert(std::make_pair("fe_chargeInjCap",(QObject*)capSelBox));
  m_knownHandles.insert(std::make_pair("fe_vCal",(QObject*)setVcal));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledLVL1",(QObject*)lvl1Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptLVL1",(QObject*)lvl1Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledOCCUPANCY",(QObject*)occFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptSUM_OCC",(QObject*)sumOccKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledSUM_OCC",(QObject*)sumOccFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptOCCUPANCY",(QObject*)occKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT",(QObject*)totFill));
  // treat TOT0...15 equally, should never be set differently!
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT0",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT1",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT2",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT3",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT4",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT5",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT6",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT7",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT8",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT9",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT10",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT11",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT12",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT13",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT14",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT15",(QObject*)tot015Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT0",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT1",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT2",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT3",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT4",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT5",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT6",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT7",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT8",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT9",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT10",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT11",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT12",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT13",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT14",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT15",(QObject*)tot015Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT",(QObject*)totKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT_MEAN",(QObject*)totmeanFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT_MEAN",(QObject*)totmeanKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOT_SIGMA",(QObject*)totsigFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOT_SIGMA",(QObject*)totsigKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCHARGE_MEAN",(QObject*)chargeMeanFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCHARGE_MEAN",(QObject*)chargeMeanKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCHARGE_SIGMA",(QObject*)chargeSigmaFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCHARGE_SIGMA",(QObject*)chargeSigmaKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCHARGE_RECO_ERROR",(QObject*)chargeRecoErrorFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCHARGE_RECO_ERROR",(QObject*)chargeRecoErrorKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledBCID",(QObject*)bcidFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptBCID",(QObject*)bcidKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledLV1ID",(QObject*)lvl1idFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptLV1ID",(QObject*)lvl1idKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledNOCC",(QObject*)noccFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptNOCC",(QObject*)noccKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledMEAN_NOCC",(QObject*)noccMeanFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptMEAN_NOCC",(QObject*)noccMeanKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledNUM_NOISY_PIXELS",(QObject*)noNsePixFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptNUM_NOISY_PIXELS",(QObject*)noNsePixKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledFDAC_T",(QObject*)fdactfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptFDAC_T",(QObject*)fdactkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledFDAC_TOT",(QObject*)fdactotfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptFDAC_TOT",(QObject*)fdactotkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledGDAC_T",(QObject*)gdactfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptGDAC_T",(QObject*)gdactkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledGDAC_THR",(QObject*)gdacthrfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptGDAC_THR",(QObject*)gdacthrkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledGDAC_OCC",(QObject*)gdacOccFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptGDAC_OCC",(QObject*)gdacOccKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledIF_T",(QObject*)iftfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptIF_T",(QObject*)iftkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledIF_TOT",(QObject*)iftotfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptIF_TOT",(QObject*)iftotkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledSCURVE_CHI2",(QObject*)scurvechi2fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptSCURVE_CHI2",(QObject*)scurvechi2keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledSCURVE_MEAN",(QObject*)scurvemeanfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptSCURVE_MEAN",(QObject*)scurvemeankeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledSCURVE_SIGMA",(QObject*)scurvesigmafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptSCURVE_SIGMA",(QObject*)scurvesigmakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTDAC_T",(QObject*)tdactfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTDAC_T",(QObject*)tdactkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTDAC_THR",(QObject*)tdacthrfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTDAC_THR",(QObject*)tdacthrkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTDAC_OCC",(QObject*)tdacOccFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTDAC_OCC",(QObject*)tdacOccKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTIMEWALK",(QObject*)twfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTIMEWALK",(QObject*)twkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOTAVERAGE",(QObject*)totavfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOTAVERAGE",(QObject*)totavkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledRAW_DATA_0",(QObject*)rd0fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptRAW_DATA_0",(QObject*)rd0keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledRAW_DATA_1",(QObject*)rd1fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptRAW_DATA_1",(QObject*)rd1keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledRAW_DATA_DIFF_1",(QObject*)rdd1fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptRAW_DATA_DIFF_1",(QObject*)rdd1keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledRAW_DATA_DIFF_2",(QObject*)rdd2fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptRAW_DATA_DIFF_2",(QObject*)rdd2keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledRAW_DATA_REF",(QObject*)rdrfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptRAW_DATA_REF",(QObject*)rdrkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledFE_ERRORS",(QObject*)dspefill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptFE_ERRORS",(QObject*)dspekeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOTCAL_PARA",(QObject*)totcalafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOTCAL_PARA",(QObject*)totcalakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOTCAL_PARB",(QObject*)totcalbfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOTCAL_PARB",(QObject*)totcalbkeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOTCAL_PARC",(QObject*)totcalcfill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOTCAL_PARC",(QObject*)totcalckeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTOTCAL_CHI2",(QObject*)totchifill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTOTCAL_CHI2",(QObject*)totchikeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledDCS_DATA",(QObject*)dcsdatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptDCS_DATA",(QObject*)dcsdatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledMON_LEAK",(QObject*)MLdatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptMON_LEAK",(QObject*)MLdatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledHB_SCALER_CNT",(QObject*)HBSdatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptHB_SCALER_CNT",(QObject*)HBSdatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCLUSTER_SIZE",(QObject*)CSdatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCLUSTER_SIZE",(QObject*)CSdatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCLUSTER_TOT",(QObject*)CTOTdatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCLUSTER_TOT",(QObject*)CTOTdatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCLUSTER_CHARGE",(QObject*)CChargedatafill));	//DLP
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCLUSTER_CHARGE",(QObject*)CChargedatakeep));	//DLP
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCLUSTER_POSITION",(QObject*)CPositiondatafill));	//DLP
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCLUSTER_POSITION",(QObject*)CPositiondatakeep));	//DLP
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCLSIZE_TOT",(QObject*)CSTOTdatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCLSIZE_TOT",(QObject*)CSTOTdatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledCLSIZE_CHARGE",(QObject*)CSChargedatafill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptCLSIZE_CHARGE",(QObject*)CSChargedatakeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledNSEEDS",(QObject*)CnSeedsFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptNSEEDS",(QObject*)CnSeedsKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledSEED_TOT",(QObject*)CSeedToTFill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptSEED_TOT",(QObject*)CSeedToTKeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledSEED_LVL1",(QObject*)CSeedLVL1Fill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptSEED_LVL1",(QObject*)CSeedLVL1Keep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledHIT_RATE",(QObject*)Eventratefill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptHIT_RATE",(QObject*)Eventratekeep));
  m_knownHandles.insert(std::make_pair("histograms_histogramFilledTRG_RATE",(QObject*)Trgratefill));
  m_knownHandles.insert(std::make_pair("histograms_histogramKeptTRG_RATE",(QObject*)Trgratekeep));

  m_knownHandles.insert(std::make_pair("histograms_maxColClusterDist",(QObject*)maxColClusterDist));
  m_knownHandles.insert(std::make_pair("histograms_maxRowClusterDist",(QObject*)maxRowClusterDist));
  m_knownHandles.insert(std::make_pair("histograms_maxClusterBcidDepth",(QObject*)maxClusterBcidDepth));
  m_knownHandles.insert(std::make_pair("histograms_maxClusterHitTot",(QObject*)maxClusterHitTot));
  m_knownHandles.insert(std::make_pair("histograms_minClusterHits",(QObject*)minClusterHits));
  m_knownHandles.insert(std::make_pair("histograms_maxClusterHits",(QObject*)maxClusterHits));
  m_knownHandles.insert(std::make_pair("histograms_maxEventIncomplete",(QObject*)maxEventIncomplete));
  m_knownHandles.insert(std::make_pair("histograms_maxEventErrors",(QObject*)maxEventErrors));
  m_knownHandles.insert(std::make_pair("histograms_getNptsRateAvg", (QObject*)nptsRateAvgBox));

  m_knownHandles.insert(std::make_pair("dcsControl_readDcsChan",(QObject*)readDcsChan));
  m_knownHandles.insert(std::make_pair("dcsControl_readDcsMode",(QObject*)readDcsMode));
  m_knownHandles.insert(std::make_pair("dcsControl_scanDcsChan",(QObject*)scanDcsChan));
  m_knownHandles.insert(std::make_pair("dcsControl_addModNameToDcs",(QObject*)addModNameBox));
  m_knownHandles.insert(std::make_pair("dcsControl_scanPar1DcsChan",(QObject*)scanPar1DcsChan));
  m_knownHandles.insert(std::make_pair("dcsControl_scanPar1DcsPar",(QObject*)scanPar1DcsPar));
  m_knownHandles.insert(std::make_pair("dcsControl_scanPar2DcsChan",(QObject*)scanPar2DcsChan));
  m_knownHandles.insert(std::make_pair("dcsControl_scanPar2DcsPar",(QObject*)scanPar2DcsPar));
  m_knownHandles.insert(std::make_pair("dcsControl_scanPar3DcsChan",(QObject*)scanPar3DcsChan));
  m_knownHandles.insert(std::make_pair("dcsControl_scanPar3DcsPar",(QObject*)scanPar3DcsPar));

  for(int sci=0;sci<MAX_LOOPS;sci++){
    m_knownHandles.insert(std::make_pair(("loops_activeLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopActive[sci]));
    m_knownHandles.insert(std::make_pair(("loops_dspProcessingLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopDspProc[sci]));
    m_knownHandles.insert(std::make_pair(("loops_paramLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopType[sci]));
    m_knownHandles.insert(std::make_pair(("loops_loopVarNStepsLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopStep[sci]));
    m_knownHandles.insert(std::make_pair(("loops_loopVarMinLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopStart[sci]));
    m_knownHandles.insert(std::make_pair(("loops_loopVarMaxLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopStop[sci]));
    m_knownHandles.insert(std::make_pair(("loops_loopVarUniformLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopRegular[sci]));
    m_knownHandles.insert(std::make_pair(("loops_endActionLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopEndAction[sci]));
    m_knownHandles.insert(std::make_pair(("loops_dspActionLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopOnDsp[sci]));
    m_knownHandles.insert(std::make_pair(("loops_loopVarValuesFreeLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopFree[sci]));
    m_knownHandles.insert(std::make_pair(("loops_loopVarValuesLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopPtsBox[sci]));
    m_knownHandles.insert(std::make_pair(("loops_feGlobRegNameLoop_"+QString::number(sci)).toLatin1().data(),
					 (QObject*)loopFERegName[sci]));
  }
  m_knownHandles.insert(std::make_pair("loops_innerLoopSwap",(QObject*)stageAdvBox));
  m_knownHandles.insert(std::make_pair("loops_dspMaskStaging",(QObject*)maskOnDsp));

  m_knownHandles.insert(std::make_pair("scans_thresholdTargedValue",(QObject*)thresvalue));
  m_knownHandles.insert(std::make_pair("scans_totTargedCharge",(QObject*)totcharge));
  m_knownHandles.insert(std::make_pair("scans_totTargedValue",(QObject*)totvalue));
  m_knownHandles.insert(std::make_pair("scans_useMaskForGlobal",(QObject*)maskGlobTune));
  m_knownHandles.insert(std::make_pair("scans_minThrNoiseCutNormal", (QObject*)minThrsNoiseCutNormal));
  m_knownHandles.insert(std::make_pair("scans_minThrNoiseCutLong",  (QObject*)minThrsNoiseCutLong));
  m_knownHandles.insert(std::make_pair("scans_minThrNoiseCutGanged",  (QObject*)minThrsNoiseCutGanged));
  m_knownHandles.insert(std::make_pair("scans_minThrNoiseCutInterGanged",  (QObject*)minThrsNoiseCutInterGanged));
  m_knownHandles.insert(std::make_pair("scans_minThrChi2Cut", (QObject*)minThrsChi2Cut));
  m_knownHandles.insert(std::make_pair("scans_chiCut", (QObject*)scurveChi2Cut));
  m_knownHandles.insert(std::make_pair("scans_nbadChiCut", (QObject*)scurveNbadChi2));
  m_knownHandles.insert(std::make_pair("scans_noiseOccCut", (QObject*)noccCut));
  m_knownHandles.insert(std::make_pair("scans_useGrpThrRange", (QObject*)redThrRangeBox));
  m_knownHandles.insert(std::make_pair("scans_fastThrUsePseudoPix", (QObject*) pseudoPixFastThrBox));
  m_knownHandles.insert(std::make_pair("reset_sendHardReset", (QObject*)hardReset));
  m_knownHandles.insert(std::make_pair("reset_sendSoftReset", (QObject*)softReset));
  m_knownHandles.insert(std::make_pair("reset_sendBCR", (QObject*)sendBCR));
  m_knownHandles.insert(std::make_pair("reset_sendECR", (QObject*)sendECR));
  m_knownHandles.insert(std::make_pair("reset_sendGlobalPulse", (QObject*)globalPulse));
  m_knownHandles.insert(std::make_pair("reset_GlobalPulseLen", (QObject*)globalPulseLength));

  m_knownHandles.insert(std::make_pair("general_testbeamFlag", (QObject*)tbModeBox));
  m_knownHandles.insert(std::make_pair("trigger_TLUTriggerDataDelay", (QObject*)tluDelay));
  m_knownHandles.insert(std::make_pair("trigger_ReduceLVL1DelayIfReplcationSlaveBy", (QObject*)reduceDelay));
  m_knownHandles.insert(std::make_pair("general_tbSRAMReadoutAt", (QObject*)sramFillLevel));
  m_knownHandles.insert(std::make_pair("general_skipSourceScanHistos", (QObject*)noSrcHistos));
#ifndef WITHEUDAQ
  tbBox->hide();
#endif

  m_knownHandles.insert(std::make_pair("loops_configAtStart", (QObject*)cfgAtStart));
  m_knownHandles.insert(std::make_pair("fe_alterFeCfg", (QObject*)alterCfgBox));
  m_knownHandles.insert(std::make_pair("loops_loopOverDCs", (QObject*)dcloopBox));
  m_knownHandles.insert(std::make_pair("loops_avoidSpecialCols", (QObject*)noSpecialColBox));
  m_knownHandles.insert(std::make_pair("loops_singleDCloop", (QObject*)singleDcLoopBox));

  // unused handles
  staticMode->hide();
  strDelRange->hide();
  delRangeLabel->hide();
  superGrpDelay->hide();
  delayAB0->hide();
  delayAB1->hide();
  nAccepts_B0->hide();
  nAccepts_A1->hide();
  nAccepts_B1->hide();
  sgrpLabel->hide();
  delg0Label->hide();
  delg1Label->hide();
  selfTrigger->hide();
  // hide until min. thresh. scan is needed again (if ever)
  minThrGroupBox->hide();

  showLoop(0);
  updateConfigSel();

  rodBox->setEnabled(true);
  crateBox->setEnabled(true);
}
PixScanPanel::~PixScanPanel(){
  delete m_stdScan;
  //  delete fillingLabel;
}

void PixScanPanel::setStdScanCfg(int scan_type, int fe_type){
  m_scanCfg = m_stdScan;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  PixScan::ScanType st = (PixScan::ScanType) scan_type;
  PixModule::FEFlavour flv = (PixModule::FEFlavour) fe_type;
  try{
    m_scanCfg->preset(st, flv);
  }catch(SctPixelRod::BaseException& exc){
    QApplication::restoreOverrideCursor();
    std::stringstream msg;
    msg << exc;
    QMessageBox::warning(this,"PixScanPanel::setStdScanCfg","Error changing scan cfg: "+
                         QString(msg.str().c_str()));
    scanConfigSel->setCurrentIndex(0);
    return;
  }catch(...){
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,"PixScanPanel::setStdScanCfg","Error changing scan cfg.");
    scanConfigSel->setCurrentIndex(0);
    return;
  }
  QApplication::restoreOverrideCursor();
}
void PixScanPanel::newConfig(){
  RenameWin rnw(this);
  rnw.EnterLabel->setText("Enter label for new config");
  rnw.exec();
  if(!rnw.Name->text().isEmpty()){
    m_engine.newPixScanCfg(rnw.Name->text().toLatin1().data(), m_scanCfg);
    updateConfigSel();
    scanConfigSel->setCurrentIndex(scanConfigSel->count()-1);
    updateConfig();
  }
  return;
}
void PixScanPanel::loadConfig()
{
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT config file (*.cfg.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this,"scanrootfile","Specify name of DB file with PixScan entries", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
    m_engine.loadScanCfg(fname.toLatin1().data());
    updateConfigSel();
  }
  return;
}
void PixScanPanel::clearConfig()
{
  for(std::vector<PixScan*>::const_iterator it = m_engine.getPixScanCfg().begin(); it != m_engine.getPixScanCfg().end(); it++)
    delete (*it);
  m_engine.getPixScanCfg().clear();
  updateConfigSel();
}
void PixScanPanel::saveConfig()
{
  Config &opts = m_engine.getOptions();
  std::string defPath = ((ConfString&)opts["paths"]["defCfgPath"]).value();
  QString qpath = QString::null;
  if(defPath!=".") qpath = defPath.c_str();
  QStringList filter;
  filter += "DB ROOT config file (*.cfg.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of DB file to save PixScan entries", qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::AnyFile);
  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    if(fname.right(9)!=".cfg.root")
      fname += ".cfg.root";
    fname.replace("\\", "/");
    m_engine.savePixScan(fname.toLatin1().data());
  }
  return;
}
void PixScanPanel::updateConfigSel(){
  scanConfigSel->clear();
  // load the names of pre-set configs
  // standard config labels:
  std::map<std::string, int> defScans = m_stdScan->getScanTypes();
  for(std::map<std::string, int>::iterator it = defScans.begin(); it!=defScans.end(); it++){
    scanConfigSel->addItem(("DEFAULT: "+it->first).c_str());
  }

  // load the scan config items into combo box
  for(std::vector<PixScan*>::const_iterator it = m_engine.getPixScanCfg().begin(); it != m_engine.getPixScanCfg().end(); it++) {
    std::string inq_name = m_engine.getPixScanCfgDBName(*it);
    getDecNameCore(inq_name);
    scanConfigSel->addItem(("USERDEF: "+inq_name).c_str());
  }

  updateConfig();

  return;
}
void PixScanPanel::updateConfig(){
  snglfeMode->setChecked(false);
  int type = scanConfigSel->currentIndex();
  QString label = scanConfigSel->currentText();
  std::map<std::string, int> defScans = m_stdScan->getScanTypes();
  if(label.left(8)=="DEFAULT:"){
    label.remove(0,9);
    setStdScanCfg(defScans[label.toLatin1().data()], (int)i4Button->isChecked()?PixModule::PM_FE_I4B:PixModule::PM_FE_I2);
    m_currStdScanID = defScans[label.toLatin1().data()];
  }else{
    int utype = type-defScans.size();
    m_scanCfg = m_engine.getPixScanCfg()[utype];
    if(m_scanCfg==0) // selected cfg. doesn't exist somehow, go to default (=digital scan)
      scanConfigSel->setCurrentIndex(0);
    label.remove(0,9);
    m_currStdScanID = -1;
  }
  if(((ConfBool&)m_engine.getOptions()["autoload"]["doPixScanAutoLabel"]).value())
    scanLabel->setText(label);

  updateFegrSel();
  updateConfigHandlers();

  QString fname = scanFileName->text();
  if(m_currStdScanID==PixScan::SOURCE_SCAN || m_currStdScanID==PixScan::FE_ST_SOURCE_SCAN){
    appIndRawfile->setChecked(true);
    rawBrowseButton_clicked(fname.left(fname.length()-5).toLatin1().data());
  } else if(m_currStdScanID>=0){
    appIndRawfile->setChecked(false);
    rawBrowseButton_clicked("");
  }
}
void PixScanPanel::updateFegrSel(){
  // test version - not sure this is the best way of doing this
  // fill names on FE global register combo box
  PixFeI4AConfig fei4a(0,0, "tmpfe", 0);
  PixFeI4BConfig fei4b(0,0, "tmpfe", 0);
  Config &cfga = fei4a.config();
  Config &cfgb = fei4b.config();
  ConfGroup &grgrpa = cfga.subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"];
  ConfGroup &grgrpb = cfgb.subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"];
  std::vector<std::string> grNames;
  if(m_engine.currFEFlavour()==2){
    for(int j=0;j<grgrpb.size();j++){
      std::string grName = grgrpb[j].name();
      grName.erase(0,std::string("GlobalRegister_").length());
      grNames.push_back(grName);
    }
  }
  else if(m_engine.currFEFlavour()==1){
    for(int j=0;j<grgrpa.size();j++){
      std::string grName = grgrpa[j].name();
      grName.erase(0,std::string("GlobalRegister_").length());
      grNames.push_back(grName);
    }
  }
  else if(m_engine.currFEFlavour()==10){// mix, allow only GR names that exist in both
    for(int j=0;j<grgrpa.size();j++){
      std::string grName = grgrpa[j].name();
      grName.erase(0,std::string("GlobalRegister_").length());
      if(grgrpb[grName].name()!="__TrashConfObj__") grNames.push_back(grName);
    }
  }
  for(int i=0;i<MAX_LOOPS;i++){
    loopFERegName[i]->clear();
    loopFERegName[i]->addItem("unknown");
    for(int j=0;j<(int)grNames.size();j++){
      loopFERegName[i]->addItem(grNames[j].c_str());
    }
  }

}
void PixScanPanel::updateConfigHandlers(){

  if(m_scanCfg==0) return;

  Config &myconf = m_scanCfg->config();

  configParameterTable->clear();
  configParameterTable->setRowCount(0);

  // make sure none of the handles disturbs loading
  std::map<std::string, QObject*>::const_iterator hndlIT;
  for(hndlIT = m_knownHandles.begin(); hndlIT!=m_knownHandles.end();hndlIT++){
    if(dynamic_cast<QSpinBox*>(hndlIT->second) != 0)
      disconnect( hndlIT->second, SIGNAL( valueChanged(int) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QDoubleSpinBox*>(hndlIT->second) != 0)
      disconnect( hndlIT->second, SIGNAL( valueChanged(double) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QComboBox*>(hndlIT->second) != 0)
      disconnect( hndlIT->second, SIGNAL( activated(int) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QCheckBox*>(hndlIT->second) != 0)
      disconnect( hndlIT->second, SIGNAL( toggled(bool) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QLineEdit*>(hndlIT->second) != 0)
      disconnect( hndlIT->second, SIGNAL( textChanged(const QString&) ), this, SLOT( readFromHandles() ) );
  }
  disconnect(configParameterTable, SIGNAL(cellChanged(int,int)), this, SLOT(readFromHandlersFromTable(int,int)));

  // loop over config items and see if it's in our "known" list
  // Loop over the ConfGroups
  for (int i=0; i<myconf.size(); i++) {
    ConfGroup &grp = myconf[i];
    // Loop over the ConfObj
    for (int j=0; j<grp.size(); j++) {
      QObject *handle = m_knownHandles[grp[j].name()];
      if(handle!=0){ // yeah, found something!
	((QWidget*)handle)->setToolTip(grp[j].comment().c_str());
        switch( grp[j].type() ) {
        case ConfObj::INT :{
          int ival=0;
          switch( ((ConfInt&)grp[j]).subtype() ){
          case ConfInt::S32:
            ival = (int)((ConfInt&)grp[j]).valueS32();
            break;
          case ConfInt::U32:
            ival = (int)((ConfInt&)grp[j]).valueU32();
            break;
          case ConfInt::S16:
            ival = (int)((ConfInt&)grp[j]).valueS16();
            break;
          case ConfInt::U16:
            ival = (int)((ConfInt&)grp[j]).valueU16();
            break;
          case ConfInt::S8:
            ival = (int)((ConfInt&)grp[j]).valueS8();
            break;
          case ConfInt::U8:
            ival = (int)((ConfInt&)grp[j]).valueU8();
            break;
          default: ;
          }
	  QSpinBox *sbhandle = dynamic_cast<QSpinBox*>(handle);
	  QCheckBox *tbhandle = dynamic_cast<QCheckBox*>(handle);
          QComboBox *cbhandle = dynamic_cast<QComboBox*>(handle);
	  if(sbhandle!=0)
	    sbhandle->setValue(ival);
	  else if(tbhandle!=0)
	    tbhandle->setChecked((bool)ival);
	  else if(cbhandle!=0)
	    cbhandle->setCurrentIndex(ival);
          break;}
        case ConfObj::LIST : {
          QComboBox *cbhandle = dynamic_cast<QComboBox*>(handle);
          QCheckBox *tbhandle = dynamic_cast<QCheckBox*>(handle);
          if(cbhandle!=0){
            cbhandle->clear();
            int currID=0;
            std::map<std::string, int>::const_iterator mapIT;
            for(mapIT = ((ConfList&)grp[j]).m_symbols.begin(); mapIT != ((ConfList&)grp[j]).m_symbols.end();mapIT++){
              cbhandle->addItem(mapIT->first.c_str());
              if(mapIT->first==((ConfList&)grp[j]).sValue())
                currID = cbhandle->count()-1;
            }
            cbhandle->setCurrentIndex(currID);
          } else if(tbhandle!=0){
            tbhandle->setChecked((bool)((ConfList&)grp[j]).getValue());
          }
          break;}
        case ConfObj::BOOL : {
          QCheckBox *cbhandle = dynamic_cast<QCheckBox*>(handle);
          if(cbhandle!=0)
            cbhandle->setChecked((bool)((ConfBool&)grp[j]).value());
	  break;}
        case ConfObj::VECTOR : {
          QListWidget *lbhandle = dynamic_cast<QListWidget*>(handle);
          if(lbhandle!=0){
	    lbhandle->clear();
	    switch( ((ConfVector&)grp[j]).subtype() ){
	    case ConfVector::INT:{
	      std::vector<int> &tmpVec = ((ConfVector&)grp[j]).valueVInt();
	      for(std::vector<int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
		lbhandle->addItem(QString::number(*IT));
	      break;}
	    case ConfVector::UINT:{
	      std::vector<unsigned int> &tmpVec = ((ConfVector&)grp[j]).valueVUint();
	      for(std::vector<unsigned int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
		lbhandle->addItem(QString::number(*IT));
	      break;}
	    case ConfVector::FLOAT:{
	      std::vector<float> &tmpVec = ((ConfVector&)grp[j]).valueVFloat();
	      for(std::vector<float>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
		lbhandle->addItem(QString::number(*IT, 'f',2));
	      break;}
	    default:
	      m_engine.toErrLog("PixScanPanel::readFromHandles:\n  found unhandled "
				"config type for item "+grp[j].name());
	    }
	  }
	  break;}
        case ConfObj::FLOAT :{
	  QSpinBox *sbhandle = dynamic_cast<QSpinBox*>(handle);
	  if(sbhandle!=0)
	    sbhandle->setValue((int)((ConfFloat&)grp[j]).value());
	  QDoubleSpinBox *dsbhandle = dynamic_cast<QDoubleSpinBox*>(handle);
	  if(dsbhandle!=0)
	    dsbhandle->setValue((float)((ConfFloat&)grp[j]).value());
	  break;}
        case ConfObj::STRING :{
          QComboBox *cbhandle = dynamic_cast<QComboBox*>(handle);
	  QLineEdit *strhandle = dynamic_cast<QLineEdit*>(handle);
	  if(strhandle!=0)
	    strhandle->setText((((ConfString&)grp[j]).value()).c_str());
	  if(cbhandle!=0){
	    for(int l=0;l<cbhandle->count();l++){
	      if(cbhandle->itemText(l)==QString((((ConfString&)grp[j]).value()).c_str())){
		cbhandle->setCurrentIndex(l);
		break;
	      }
	    }
	  }
	  break;}
        case ConfObj::VOID :
        default:
	  m_engine.toErrLog("PixScanPanel::updateConfigHandlers:\n  found unknown/unhandled "
			     "config type for item "+grp[j].name());
        }
      } else{ // dump info to table
	if(grp[j].visible() && ( // do not list items that are auto-generated by us:
				(grp[j].name().find("modGroups_mod")==std::string::npos && 
				 grp[j].name().find("modGroups_config")==std::string::npos && 
				 grp[j].name().find("modGroups_trigger")==std::string::npos && 
				 grp[j].name().find("modGroups_readout")==std::string::npos && 
				 grp[j].name().find("modGroups_strobe")==std::string::npos
				 ))){
	  int k = configParameterTable->rowCount();
	  configParameterTable->insertRow(k);
	  configParameterTable->setItem(k,0, new QTableWidgetItem(grp[j].name().c_str() ));
	  QString typeStr, valStr;
	  switch( grp[j].type() ) {
	  case ConfObj::INT : {
	    typeStr = "INT"; 
	    int ival=0;
	    switch( ((ConfInt&)grp[j]).subtype() ){
	    case ConfInt::S32:
	      ival = (int)((ConfInt&)grp[j]).valueS32();
	      break;
	    case ConfInt::U32:
	      ival = (int)((ConfInt&)grp[j]).valueU32();
	      break;
	    case ConfInt::S16:
	      ival = (int)((ConfInt&)grp[j]).valueS16();
	      break;
	    case ConfInt::U16:
	      ival = (int)((ConfInt&)grp[j]).valueU16();
	      break;
	    case ConfInt::S8:
	      ival = (int)((ConfInt&)grp[j]).valueS8();
	      break;
	    case ConfInt::U8:
	      ival = (int)((ConfInt&)grp[j]).valueU8();
	      break;
	    default: ;
	    }
	    valStr  = QString::number(ival);
	    break;}
	  case ConfObj::FLOAT : 
	    typeStr = "FLOAT";
	    valStr  = QString::number(((ConfFloat&)grp[j]).value(),'f',3);
	    break;
	  case ConfObj::LIST : 
	    typeStr = "LIST"; 
	    valStr  = ((ConfList&)grp[j]).sValue().c_str();
	    break;
	  case ConfObj::BOOL : 
	    typeStr = "BOOL";
	    valStr  = ((ConfBool&)grp[j]).value()?"true":"false";
	    break;
	  case ConfObj::STRING : 
	    typeStr = "STRING";
	    valStr  = ((ConfString&)grp[j]).value().c_str();
	    break;
	  case ConfObj::VOID : 
	    typeStr = "VOID"; 
	    valStr  = "void";
	    break;
	  case ConfObj::VECTOR :
	    typeStr = "VECTOR"; 
	    valStr  = "vector";
	    break;
	  default: 
	    typeStr = "Unrecognized";
	  }
	  configParameterTable->setItem(k,1, new QTableWidgetItem( typeStr ));
	  configParameterTable->setItem(k,2, new QTableWidgetItem( grp[j].comment().c_str() ));
	  
	  if ( grp[j].type() != ConfObj::LIST && grp[j].type() !=ConfObj::VECTOR ) {
	    configParameterTable->setItem(k,3, new QTableWidgetItem( valStr ));
          } else if(grp[j].type() == ConfObj::LIST){ //deal with LIST
	    std::map<std::string, int> stdmap = ((ConfList&)grp[j]).symbols();
	    std::map<std::string, int>::iterator pos;
	    QComboBox *comboItem = new QComboBox();
	    int cbIndex=0, currInd=-1;
	    for(pos = stdmap.begin(); pos != stdmap.end(); pos ++) {
	      comboItem->addItem(QString(pos->first.c_str()), QVariant(pos->second));
	      if(((ConfList&)grp[j]).sValue()==pos->first) currInd = cbIndex;
	      cbIndex++;
	    }
	    if(currInd>=0) comboItem->setCurrentIndex(currInd);
	    configParameterTable->setCellWidget(k,3, comboItem );
          } else { // deal with vectors
	    QComboBox *comboItem = new QComboBox();
	    switch( ((ConfVector&)grp[j]).subtype() ){
	    case ConfVector::INT:{
	      std::vector<int> &tmpVec = ((ConfVector&)grp[j]).valueVInt();
	      for(std::vector<int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
		comboItem->addItem(QString::number(*IT), QVariant(*IT));
	      configParameterTable->item( k, 1)->setText("INT VECTOR" );
	      break;}
	    case ConfVector::UINT:{
	      std::vector<unsigned int> &tmpVec = ((ConfVector&)grp[j]).valueVUint();
	      for(std::vector<unsigned int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
		comboItem->addItem(QString::number(*IT), QVariant(*IT));
	      configParameterTable->item( k, 1)->setText("UINT VECTOR" );
	      break;}
	    case ConfVector::FLOAT:{
	      std::vector<float> &tmpVec = ((ConfVector&)grp[j]).valueVFloat();
	      for(std::vector<float>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
		comboItem->addItem(QString::number(*IT, 'f',2), QVariant(*IT));
	      configParameterTable->item( k, 1)->setText("FLOAT VECTOR" );
	      break;}
	    default:
	      configParameterTable->item( k, 1)->setText("unknown VECTOR" );
	      comboItem->addItem("unknown values");
	      break;
	    }
	    comboItem->setCurrentIndex( 0 );
	    configParameterTable->setCellWidget(k,3, comboItem );
	  }
	}
      }
    }
  }

  // set columns for Name, Type, Comment and Visibility read-only; only Value stays editable
//   configParameterTable->setColumnReadOnly( 0, true );
//   configParameterTable->setColumnReadOnly( 1, true );
//   configParameterTable->setColumnReadOnly( 2, true );

  // make sure all related settings on loop panels are correct
  for(int sci=0;sci<MAX_LOOPS;sci++)
    updateLoopSettings(sci);

  // restore all connections
  for(hndlIT = m_knownHandles.begin(); hndlIT!=m_knownHandles.end();hndlIT++){
    if(dynamic_cast<QSpinBox*>(hndlIT->second) != 0)
      connect( hndlIT->second, SIGNAL( valueChanged(int) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QDoubleSpinBox*>(hndlIT->second) != 0)
      connect( hndlIT->second, SIGNAL( valueChanged(double) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QComboBox*>(hndlIT->second) != 0)
      connect( hndlIT->second, SIGNAL( activated(int) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QCheckBox*>(hndlIT->second) != 0)
      connect( hndlIT->second, SIGNAL( toggled(bool) ), this, SLOT( readFromHandles() ) );
    if(dynamic_cast<QLineEdit*>(hndlIT->second) != 0)
      connect( hndlIT->second, SIGNAL( textChanged(const QString&) ), this, SLOT( readFromHandles() ) );
  }
  connect(configParameterTable, SIGNAL(cellChanged(int,int)), this, SLOT(readFromHandlersFromTable(int,int)));

  return;
}
void PixScanPanel::readFromHandles(){

  if(m_scanCfg==0) return;

  for(int sci=0;sci<MAX_LOOPS;sci++){
    updateLoopSettings(sci);
    updateLoopPts(sci);
  }

  Config &myconf = m_scanCfg->config();
  
  // config was edited -> no longer a std. scan definition
  m_currStdScanID = -1;

  // loop over config items and see if it's in our "known" list
  // Loop over the ConfGroups
  for (int i=0; i<myconf.size(); i++) {
    ConfGroup &grp = myconf[i];
    // Loop over the ConfObj
    for (int j=0; j<grp.size(); j++) {
      QObject *handle = m_knownHandles[grp[j].name()];
      if(handle!=0){ // yeah, found something!
        switch( grp[j].type() ) {
        case ConfObj::INT :{
	  int read_val=-1;
	  QSpinBox *sbhandle = dynamic_cast<QSpinBox*>(handle);
	  QCheckBox *tbhandle = dynamic_cast<QCheckBox*>(handle);
          QComboBox *cbhandle = dynamic_cast<QComboBox*>(handle);
	  if(sbhandle!=0)
	    read_val = sbhandle->value();
	  else if(tbhandle!=0)
	    read_val = (int) tbhandle->isChecked();
	  else if(cbhandle!=0)
	    read_val = (int) cbhandle->currentIndex();
	  else{
	    m_engine.toErrLog("PixScanPanel::readFromHandles:\n  found unhandled "
			      "config type for item "+grp[j].name());
	    break;
	  }
          switch( ((ConfInt&)grp[j]).subtype() ){
          case ConfInt::S32:
            *((int *)((ConfInt&)grp[j]).m_value) = (int)read_val;
            break;
          case ConfInt::U32:
            *((unsigned int *)((ConfInt&)grp[j]).m_value) = (unsigned int)read_val;
            break;
          case ConfInt::S16:
            *((short int *)((ConfInt&)grp[j]).m_value) = (short int)read_val;
            break;
          case ConfInt::U16:
            *((unsigned short int *)((ConfInt&)grp[j]).m_value) = (unsigned short int)read_val;
            break;
          case ConfInt::S8:
            *((char *)((ConfInt&)grp[j]).m_value) = (char)read_val;
            break;
          case ConfInt::U8:
            *((unsigned char *)((ConfInt&)grp[j]).m_value) = (unsigned char)read_val;
            break;
          default: ;
          }
          break;}
        case ConfObj::LIST :{
          int read_val = 0;
          QComboBox *cbhandle = dynamic_cast<QComboBox*>(handle);
          QCheckBox *tbhandle = dynamic_cast<QCheckBox*>(handle);
          if(cbhandle!=0){
            read_val = (int) ((ConfList&)grp[j]).m_symbols[cbhandle->currentText().toLatin1().data()];
          }else if(tbhandle!=0){
            read_val = (int) tbhandle->isChecked();
          } else{
	    m_engine.toErrLog("PixScanPanel::readFromHandles:\n  found unhandled "
			      "config type for item "+grp[j].name());
	    break;
	  }
          switch( ((ConfList&)grp[j]).subtype() ){
          case ConfList::S32:
            *((int *)((ConfList&)grp[j]).m_value) = (int) read_val;
            break;
          case ConfList::U32:
            *((unsigned int *)((ConfList&)grp[j]).m_value) = (unsigned int) read_val;
            break;
          case ConfList::S16:
            *((short int *)((ConfList&)grp[j]).m_value) = (short int) read_val;
            break;
          case ConfList::U16:
            *((unsigned short int *)((ConfList&)grp[j]).m_value) = (unsigned short int) read_val;
            break;
          case ConfList::S8:
            *((char *)((ConfList&)grp[j]).m_value) = (char) read_val;
            break;
          case ConfList::U8:
            *((unsigned char *)((ConfList&)grp[j]).m_value) = (unsigned char) read_val;
            break;
          default:
	    m_engine.toErrLog("PixScanPanel::readFromHandles:\n  found unhandled "
			      "config type for item "+grp[j].name());
          }
          break;}
        case ConfObj::BOOL :{
          QCheckBox *tbhandle = dynamic_cast<QCheckBox*>(handle);
          if(tbhandle!=0)
	    ((ConfBool&)grp[j]).m_value = tbhandle->isChecked();
	  break;}
        case ConfObj::VECTOR : {
          QListWidget *lbhandle = dynamic_cast<QListWidget*>(handle);
          if(lbhandle!=0){
	    bool isOK;
	    switch( ((ConfVector&)grp[j]).subtype() ){
	    case ConfVector::INT:{
	      int value;
	      std::vector<int> &tmpVec = ((ConfVector&)grp[j]).valueVInt();
	      tmpVec.clear();
	      for(unsigned int sci=0;sci<(unsigned int)lbhandle->count();sci++){
		value = lbhandle->item(sci)->text().toInt(&isOK);
		if(isOK)
		  tmpVec.push_back(value);
	      }
	      break;}
	    case ConfVector::UINT:{
	      unsigned int value;
	      std::vector<unsigned int> &tmpVec = ((ConfVector&)grp[j]).valueVUint();
	      tmpVec.clear();
	      for(unsigned int sci=0;sci<(unsigned int)lbhandle->count();sci++){
		value = (unsigned int) lbhandle->item(sci)->text().toInt(&isOK);
		if(isOK)
		  tmpVec.push_back(value);
	      }
	      break;}
	    case ConfVector::FLOAT:{
	      float value;
	      std::vector<float> &tmpVec = ((ConfVector&)grp[j]).valueVFloat();
	      tmpVec.clear();
	      for(unsigned int sci=0;sci<(unsigned int)lbhandle->count();sci++){
		value = lbhandle->item(sci)->text().toFloat(&isOK);
		if(isOK)
		  tmpVec.push_back(value);
	      }
	      break;}
	    default:
	      m_engine.toErrLog("PixScanPanel::readFromHandles:\n  found unhandled "
				"config type for item "+grp[j].name());
	    }
	  }
	  break;}
        case ConfObj::FLOAT :{
	  QSpinBox *sbhandle = dynamic_cast<QSpinBox*>(handle);
	  if(sbhandle!=0)
	    ((ConfFloat&)grp[j]).m_value = (float)sbhandle->value();
	  QDoubleSpinBox *dsbhandle = dynamic_cast<QDoubleSpinBox*>(handle);
	  if(dsbhandle!=0)
	    ((ConfFloat&)grp[j]).m_value = (double)dsbhandle->value();
	  break;}
        case ConfObj::STRING :{
          QComboBox *cbhandle = dynamic_cast<QComboBox*>(handle);
	  QLineEdit *strhandle = dynamic_cast<QLineEdit*>(handle);
	  if(strhandle!=0){
	    ((ConfString&)grp[j]).m_value = std::string(strhandle->text().toLatin1().data());
	  }
	  if(cbhandle!=0){
	    ((ConfString&)grp[j]).m_value = std::string(cbhandle->currentText().toLatin1().data());
	  }
	  break;}
        case ConfObj::VOID :
        default:
	  m_engine.toErrLog("PixScanPanel::readFromHandles:\n  found unknown "
			     "config type for item "+grp[j].name());
        }
      }
    }
  }
  return;
}
void PixScanPanel::readFromHandlersFromTable(int row, int){

  if(m_scanCfg==0) return;

  Config &myconf = m_scanCfg->config();

  // config was edited -> no longer a std. scan definition
  m_currStdScanID = -1;

  // Loop over the ConfGroups
  for (int i=0; i<myconf.size(); i++) {
    ConfGroup &grp = myconf[i];

    //    ConfObj &obj = grp[configParameterTable->text( row, 0 )]; // find ConfObj according to its name
    std::string obj_name = configParameterTable->item( row, 0 )->text().toLatin1().data();
    unsigned int pos = (unsigned int)obj_name.find("_");
    if(pos!=(unsigned int)std::string::npos) // remove group name from table
      obj_name.erase(0,pos+1);
    ConfObj &obj = grp[obj_name]; // find ConfObj according to its name
    if (obj.name() == "__TrashConfObj__") continue; //not found, try next ConfGroup

    switch( obj.type() ) {
      case ConfObj::INT :
        PixLib::WriteIntConf((ConfInt&) obj, (configParameterTable->item(row,3))->text().toInt());
        break;
      case ConfObj::FLOAT :
        ((ConfFloat&)obj).m_value = (configParameterTable->item(row,3))->text().toFloat();
        break;
      case ConfObj::BOOL :
        ((ConfBool&)obj).m_value =  (configParameterTable->item(row,3)->text().toLower() == "true") ? true : false;
        break;
      case ConfObj::STRING :
        ((ConfString&)obj).m_value = configParameterTable->item(row,3)->text().toLatin1().data();
        break;
      case ConfObj::LIST :
        {
          QString str_val = ((QComboBox *)(configParameterTable->cellWidget(row,3)))->currentText();
          int read_val = (int) ((ConfList&)obj).m_symbols[str_val.toLatin1().data()];
          switch( ((ConfList&)obj).subtype() ){
            case ConfList::S32:
              *((int *)((ConfList&)obj).m_value) = (int) read_val;
              break;
            case ConfList::U32:
              *((unsigned int *)((ConfList&)obj).m_value) = (unsigned int) read_val;
              break;
            case ConfList::S16:
              *((short int *)((ConfList&)obj).m_value) = (short int) read_val;
              break;
            case ConfList::U16:
              *((unsigned short int *)((ConfList&)obj).m_value) = (unsigned short int) read_val;
              break;
            case ConfList::S8:
              *((char *)((ConfList&)obj).m_value) = (char) read_val;
              break;
            case ConfList::U8:
              *((unsigned char *)((ConfList&)obj).m_value) = (unsigned char) read_val;
              break;
            default: ;
          }
        } 
        break;
      case ConfObj::VECTOR :
        {
          QComboBox *cb = dynamic_cast<QComboBox *>(configParameterTable->cellWidget(row,3));
          bool isOK;
          if(cb!=0){
            int nent = cb->count();
            switch( ((ConfVector&)obj).subtype() ){
              case ConfVector::UINT:
                {
                  std::vector<unsigned int> &tmpVec = ((ConfVector&)obj).valueVUint();
                  tmpVec.clear();
                  for(int i=0;i<nent;i++){
                    unsigned int value = (unsigned int)cb->itemData(i).toInt(&isOK);
                    if(isOK){
                      tmpVec.push_back(value);
                    }
                  }
                }
                break;
              case ConfVector::INT:
                {
                  std::vector<int> &tmpVec = ((ConfVector&)obj).valueVInt();
                  tmpVec.clear();
                  for(int i=0;i<nent;i++){
                    int value = cb->itemData(i).toInt(&isOK);
                    if(isOK){
                      tmpVec.push_back(value);
                    }
                  }
                }
                break;
              case ConfVector::FLOAT:
                {
                  std::vector<float> &tmpVec = ((ConfVector&)obj).valueVFloat();
                  tmpVec.clear();
                  for(int i=0;i<nent;i++){
                    float value = cb->itemData(i).toFloat(&isOK);
                    if(isOK){
                      tmpVec.push_back(value);
                    }
                  }
                }
                break;
              default:
                break;
            }
          }
          else
          {
            m_engine.toErrLog("PixScanPanel::readFromHandlersFromTable:\n "+obj.name()+
                "was supposed to have a combo box but doesn't");
          }
        }
        break;
      case ConfObj::VOID :
      default:
        std::stringstream ss;
        ss << "PixScanPanel::readFromHandlersFromTable:\n  found unknown "
           << "config type " << obj.type();
        m_engine.toErrLog(ss.str());
        break;
    }
  } //end of loop over ConfGroups
  return;
}
void PixScanPanel::updateLoopSettings(int ID){
  if(ID<0 || ID>MAX_LOOPS) return;
  if(LoopBox[ID]==0) return;
  if(!loopActive[ID]->isChecked()){
    loopStep[ID]->setValue(1);
    loopStart[ID]->setValue(0);
    loopStop[ID]->setValue(0);
    loopRegular[ID]->setChecked(true);
    loopStart[ID]->setEnabled(false);
    loopStop[ID]->setEnabled(false);
    loopStep[ID]->setEnabled(false);
    loopRegular[ID]->setEnabled(false);
    loopPtsBox[ID]->setEnabled(false);
    loopDspProc[ID]->setEnabled(false);
    loopEndAction[ID]->setEnabled(false);
    loopOnDsp[ID]->setEnabled(false);
    loopType[ID]->setEnabled(false);
  } else{
    loopStart[ID]->setEnabled(loopRegular[ID]->isChecked());
    loopStop[ID]->setEnabled(loopRegular[ID]->isChecked());
    loopStep[ID]->setEnabled(loopRegular[ID]->isChecked());
    loopRegular[ID]->setEnabled(true);
    loopPtsBox[ID]->setEnabled(true);
    loopDspProc[ID]->setEnabled(true);
    loopEndAction[ID]->setEnabled(true);
    loopOnDsp[ID]->setEnabled(true);
    loopType[ID]->setEnabled(true);
  }
  setFeReg(ID);
}
void PixScanPanel::updateLoopPts(int ID){
  if(ID<0 || ID>MAX_LOOPS) return;
  if(LoopBox[ID]==0) return;

  if(loopRegular[ID]->isChecked()){
    loopPtsBox[ID]->clear();
    if(loopStep[ID]->value()>1){
      for(int i=0;i<loopStep[ID]->value();i++){
        double scanpt = loopStart[ID]->value() + (loopStop[ID]->value()-loopStart[ID]->value())/
          (double)(loopStep[ID]->value()-1)*((double)i);
        loopPtsBox[ID]->addItem(QString::number(scanpt));
      }
    }else if(loopStep[ID]->value()==1)
      loopPtsBox[ID]->addItem(loopStart[ID]->text());
  } else{
    //    loopPtsBox[ID]->clear();
  }
  return;
}
void PixScanPanel::showLoop(int ID){
  if(ID<0 || ID>MAX_LOOPS) return;
  if(LoopBox[ID]==0) return;

  for(int sci=0;sci<MAX_LOOPS;sci++)
    LoopBox[sci]->hide();

  LoopBox[ID]->show();

  return;
}

void PixScanPanel::runButton_clicked(bool tbmode){
// Just start the scan and return
  if(runButton->text()=="Start Scan"){  
    QString new_label;
    if(STControlEngine::checkScanLabel(m_scanLabels, scanLabel->text(), new_label)){
      if (tbmode){ // test beam mode
	// remote scan
	scanLabel->setText(new_label);
      } else {
	QMessageBox::StandardButton user_answer = QMessageBox::information(this, "PixScanPanel",
									   "Current scan label has been used before.\n"
									   "RootDB does not support this.\n"
									   "Do you want to use \""+new_label+"\" instead?",
									   QMessageBox::Yes, QMessageBox::No);
	if(user_answer == QMessageBox::Yes) {
	  scanLabel->setText(new_label);
	}else{
	  QMessageBox::information(this, "PixScanPanel", 
				   "Please change scan label manually to something different before "
				   "trying to scan again.");
	  return;
	}
      }
    }

    // change panel view
    int type = tbmode?4:(2*(int)m_scanCfg->getSourceScanFlag());
    if(type==2) type += (int)m_scanCfg->getLoopActive(0);
    //    if(type==0 && !m_scanCfg->getDspMaskStaging()) type = 1;
    if(type==0 && m_scanCfg->getInnerLoopSwap()) type = 1;
    scanRunning(type);
    emit enableBocAnaScan(false);

    // gather information and actually launch scan
    pixScanRunOptions myscopt;
    myscopt.scanConfig = m_scanCfg; // ok now, engine will make a copy
    myscopt.loadToAna = 0;
    myscopt.anaLabel = scanLabel->text().toLatin1().data();
    myscopt.writeToFile = saveFile->isChecked();
    myscopt.fileName = scanFileName->text().toLatin1().data();
    myscopt.determineSDSPcfg = true;
    myscopt.stdTestID = m_currStdScanID;
    myscopt.runFEbyFE = snglfeMode->isChecked();
    myscopt.indexRawFile = appIndRawfile->isChecked();
    myscopt.readDcs = dcsReadBox->currentIndex();
    myscopt.timeToAbort = -1; // no abort
    myscopt.openSrcMon = sourceMonBox->isChecked() && sourceMonBox->isEnabled();

    int nRods = m_engine.CtrlStatusSummary();	
    // check if none of the controllers has been initialised, unless
    // scan uses DCS only
    bool onlyDcsScanned=true;
    for(int il=0;il<3;il++){
      if(m_scanCfg->getLoopActive(il) && m_scanCfg->getLoopParam(il)!=PixLib::PixScan::DCS_VOLTAGE) onlyDcsScanned = false;
    }
    onlyDcsScanned &= (m_scanCfg->getDcsChan()!="");

    if (nRods == 0 && (m_scanCfg->getDcsChan()=="" || !onlyDcsScanned))
      {
	QMessageBox initRods(QMessageBox::Question, "PixScanPanel", 
			     "Can't find any initialised PixController\nDo you want to initialise all PixControllers?",
			     QMessageBox::Yes | QMessageBox::No, this);
	if(initRods.exec()==QMessageBox::Yes)
	  {
	    // set wait cursor
	    QApplication::setOverrideCursor(Qt::WaitCursor);
	    m_engine.initRods();
	    // restore normal cursor
	    QApplication::restoreOverrideCursor();
	    //  status = m_engine.pixScan(myscopt);
	    nRods = m_engine.CtrlStatusSummary();  
	    if(nRods<=0)
	       {
		 updateStatus(0,0,0,0,-1,0,0,0,1);
		 QMessageBox::warning(this,"PixScanPanel","Error in initialising PixControllers\n->not scanning.");	
	       }
	  }
	else
	  {
	    updateStatus(0,0,0,0,-1,0,0,0,1);
	    QMessageBox::warning(this,"PixScanPanel","Error in initialising PixControllers\n->not scanning.");	
	  }
      }
    else if (nRods==-1)
      {
	QMessageBox::warning(this,"PixScanPanel","No PixController found.\n->not scanning.");
	updateStatus(0,0,0,0,0,0,0,0,1); // set state back to idle
      }
    else if (nRods<0)
      {
	QMessageBox::warning(this,"PixScanPanel","Strange Output from STControlEngine::CtrlStatusSummary().\n->not scanning.");
	updateStatus(0,0,0,0,0,0,0,0,1); // set state back to idle
      }
    
    if (nRods>0 || (m_scanCfg->getDcsChan()!="" && onlyDcsScanned))
      {
	// only add label if it is really used (if there is a db file to be opened), 
	// and after initialisation of Rods (otherwise label could be added even if scan is not made)
	if(myscopt.writeToFile && (!myscopt.fileName.empty()))
	  m_scanLabels.push_back(scanLabel->text());
	
	int status = m_engine.pixScan(myscopt); 
	
	QString errMsg;
	switch(status)
	  {
	  case -1:
	    errMsg = "Error: no config submitted to scan initialisation\n->not scanning.";
	    break;
	  case -2:
	    errMsg = "Error in processing PixScan configuration\n->not scanning.";
	    break;
	  case -3:
	    errMsg = "Error while sending PixScan configuration to PixControllers\n->not scanning.";
	    break;
	  case -4:
	    errMsg = "Timeout while waiting for PixController to get into scan mode\n->not scanning.";
	    break;
	  default:
	    errMsg = "Unknown error during scan initialisation\n->not scanning.";
	  }
	
	return;
      }
  }
  else if(runButton->text()=="Abort" || runButton->text()=="Stop")
    {
      runButton->setText("Wait");
      runButton->setEnabled(false);
      m_engine.stopPixScan();
      return;
    }
  return;
}

void PixScanPanel::extScanConfig(extScanOptions options){
	rawFileName->setText("");
	QString fname = scanFileName->text();
	appIndRawfile->setCheckState(Qt::Unchecked);
	saveFile->setCheckState(Qt::Unchecked);
	scanFileName->setText("");

	// Set Scan Type:
	if(options.FEflavour==1) i3Button->setChecked(true);
	else                     i4Button->setChecked(true);
	int index = scanConfigSel->findText(options.scan_type, Qt::MatchEndsWith);
	if (index>=0)
	{
	  // std::cout << "PixScanPanel::extScanConfig: Scan Type found: " << index << std::endl;
		scanConfigSel->setCurrentIndex (index);
		updateConfig();
	}

	if (options.histogram_filename!="")
	{
		saveFile->setCheckState(Qt::Checked);
		scanFileName->setText(options.histogram_filename);
		setScanValue("general_skipSourceScanHistos", false);

		bool do_cluster = false;
		setScanValue("histograms_histogramFilledOCCUPANCY", true);
		setScanValue("histograms_histogramKeptOCCUPANCY", true);
		setScanValue("histograms_histogramFilledTOT", true);
		setScanValue("histograms_histogramKeptTOT", true);
		setScanValue("histograms_histogramFilledLVL1", true);
		setScanValue("histograms_histogramKeptLVL1", true);
		setScanValue("histograms_histogramFilledLV1ID", true);
		setScanValue("histograms_histogramKeptLV1ID", true);
		setScanValue("histograms_histogramFilledBCID", true);
		setScanValue("histograms_histogramKeptBCID", true);
		setScanValue("histograms_histogramFilledCLUSTER_TOT", do_cluster);
		setScanValue("histograms_histogramKeptCLUSTER_TOT", do_cluster);
		setScanValue("histograms_histogramFilledCLUSTER_SIZE", do_cluster);
		setScanValue("histograms_histogramKeptCLUSTER_SIZE", do_cluster);
		setScanValue("histograms_histogramFilledCLUSTER_CHARGE", do_cluster);
		setScanValue("histograms_histogramKeptCLUSTER_CHARGE", do_cluster);
                setScanValue("histograms_histogramFilledCLUSTER_POSITION", do_cluster);
		setScanValue("histograms_histogramKeptCLUSTER_POSITION", do_cluster);
		setScanValue("histograms_histogramFilledCLSIZE_CHARGE", do_cluster);
		setScanValue("histograms_histogramKeptCLSIZE_CHARGE",do_cluster);
		setScanValue("histograms_histogramFilledCLSIZE_TOT", do_cluster);
		setScanValue("histograms_histogramKeptCLSIZE_TOT", do_cluster);
		setScanValue("histograms_histogramFilledHITOCC", do_cluster);
		setScanValue("histograms_histogramKeptHITOCC", do_cluster);
		setScanValue("histograms_histogramFilledNSEEDS", do_cluster);
		setScanValue("histograms_histogramKeptNSEEDS", do_cluster);
		setScanValue("histograms_histogramFilledSEED_TOT", do_cluster);
		setScanValue("histograms_histogramKeptSEED_TOT", do_cluster);
		setScanValue("histograms_histogramFilledSEED_LVL1", do_cluster);
		setScanValue("histograms_histogramKeptSEED_LVL1", do_cluster);
	} else {
		setScanValue("histograms_histogramFilledOCCUPANCY", true);	// one histogram needed, because scan is canceled otherwise
		setScanValue("histograms_histogramKeptOCCUPANCY", false);
		setScanValue("histograms_histogramFilledTOT", false);
		setScanValue("histograms_histogramKeptTOT", false);
		setScanValue("histograms_histogramFilledLVL1", false);
		setScanValue("histograms_histogramKeptLVL1", false);
		setScanValue("histograms_histogramFilledLV1ID", false);
		setScanValue("histograms_histogramKeptLV1ID", false);
		setScanValue("histograms_histogramFilledBCID", false);
		setScanValue("histograms_histogramKeptBCID", false);
		setScanValue("histograms_histogramFilledCLUSTER_TOT", false);
		setScanValue("histograms_histogramKeptCLUSTER_TOT", false);
		setScanValue("histograms_histogramFilledCLUSTER_SIZE", false);
		setScanValue("histograms_histogramKeptCLUSTER_SIZE", false);
		setScanValue("histograms_histogramFilledCLUSTER_CHARGE", false);
		setScanValue("histograms_histogramKeptCLUSTER_CHARGE", false);
                setScanValue("histograms_histogramFilledCLUSTER_POSITION", false);
		setScanValue("histograms_histogramKeptCLUSTER_POSITION", false);
		setScanValue("histograms_histogramFilledCLSIZE_CHARGE", false);
		setScanValue("histograms_histogramKeptCLSIZE_CHARGE",false);
		setScanValue("histograms_histogramFilledCLSIZE_TOT", false);
		setScanValue("histograms_histogramKeptCLSIZE_TOT", false);
		setScanValue("histograms_histogramFilledHITOCC", false);
		setScanValue("histograms_histogramKeptHITOCC", false);
		setScanValue("histograms_histogramFilledNSEEDS", false);
		setScanValue("histograms_histogramKeptNSEEDS", false);
		setScanValue("histograms_histogramFilledSEED_TOT", false);
		setScanValue("histograms_histogramKeptSEED_TOT", false);
		setScanValue("histograms_histogramFilledSEED_LVL1", false);
		setScanValue("histograms_histogramKeptSEED_LVL1", false);
	}

	if (options.rawdata_path!="") setScanValue("general_skipSourceScanHistos", false);

	setScanValue("trigger_strobeLVL1Delay", options.lvl1_delay);
	setScanValue("trigger_TLUTriggerDataDelay", options.TLUTriggerDataDelay);
	setScanValue("general_tbSRAMReadoutAt", options.SRAM_READOUT_AT);
	updateConfigHandlers();
}

void PixScanPanel::setScanValue(std::string parameter, bool value) {
	if(m_scanCfg==0) return;
	Config &myconf = m_scanCfg->config();
	 // Loop over the ConfGroups
	  for (int i=0; i<myconf.size(); i++)
	  {
		ConfGroup &grp = myconf[i];

		for (int j=0; j<grp.size(); j++)
		{
			//std::cout << grp[j].name() << std::endl;
			if (grp[j].name()==parameter) 
			{
			  //std::cout << "Found: " << parameter << std::endl;
				((ConfBool&)grp[j]).m_value = value;
				return;
			}
		}
	  }
}

void PixScanPanel::setScanValue(std::string parameter, int value) {
	if(m_scanCfg==0) return;
	Config &myconf = m_scanCfg->config();
	 // Loop over the ConfGroups
	  for (int i=0; i<myconf.size(); i++)
	  {
		ConfGroup &grp = myconf[i];

		for (int j=0; j<grp.size(); j++)
		{
			//std::cout << grp[j].name() << std::endl;
			if (grp[j].name()==parameter) 
			{
				std::cout << "Found: " << parameter << std::endl;
				*((int *)((ConfInt&)grp[j]).m_value) = value;
				return;
			}
		}
	  }
}

void PixScanPanel::updateStatus(int nSteps0, int nSteps1, int nSteps2, int nMasks, int ,//in_currFe, 
				int sramFillLevel, int triggerRate, int event_Rate, int status){

  scanLabel->setText(m_engine.getLastPxScanConfig().anaLabel.c_str());

  if(nMasks<2147483647){// 2147483647 is max. pos. 32-bit int number, not used anywhere else, so should be safe identifier
    if (eventDisp->isEnabled()) {
      if(eventLabel->text()=="Processed Events:"){
	eventDisp->display(nMasks);
      } else{
	PixLib::PixScan *scanOptions=m_engine.getLastPxScanConfig().scanConfig;
	// Estimate remaining scantime
	double percentage=0;
	double percentage_error=0;
	if (scanOptions!=NULL) {
	  int ntotstp[3];
	  double max = 1.;
	  bool loopSwap=false;
	  for(int is=0;is<3;is++) {
	    if(scanOptions->getLoopActive(is))
	      ntotstp[is] = scanOptions->getLoopVarNSteps(is);
	    else
	      ntotstp[is] = 1;
	    loopSwap = scanOptions->getInnerLoopSwap();
	    if(is==0 && loopSwap) // inner lop is not scan but mask -> get that counter
	      ntotstp[is] = scanOptions->getMaskStageSteps();
	    if(ntotstp[is]==0) ntotstp[is]=1;
	    max *= (double)ntotstp[is];
	    //std::cout << "PixScanPanel::updateStatus: Npts(loop" << is << ")=" << ntotstp[is] << ", max=" << max << std::endl;
	  }
	  if (max>0){
	    percentage=(double)(nSteps2*ntotstp[1]*ntotstp[0]+nSteps1*ntotstp[0]+(loopSwap?nMasks:nSteps0))/max*100.;
	    percentage_error=100/max;
	  } else
	    percentage=100;
	  
	  if (percentage<0)
	    percentage=0;
	  else if (percentage+percentage_error>100) {
	    percentage=100;
	    percentage_error=0;
	  }
	  //std::cout << "PixScanPanel::updateStatus: percentage="<<percentage<<", error=" << percentage_error << ", max=" << max << std::endl;
	  
	  if (percentage!=m_lastPercentage) m_StatusUpdatesRecieved++;
	  //std::cout << "PixScanPanel::updateStatus: corrected percentage="<<percentage<< std::endl;
	  
	  if (m_StatusUpdatesRecieved==2 && percentage!=m_lastPercentage) {
	    m_MeasurementStartTime=QDateTime::currentDateTime().toTime_t();
	    m_MeasurementStartPercentage=percentage;
	    //std::cout << "PixScanPanel::updateStatus: storing m_MeasurementStartTime=" << m_MeasurementStartTime<<", m_MeasurementStartPercentage=" << m_MeasurementStartPercentage<<std::endl;
	  }
	  // First estimation after 1 minute
	  if (percentage>m_lastPercentage && (QDateTime::currentDateTime().toTime_t()-m_MeasurementStartTime)>30 && percentage>0 && m_MeasurementStartTime>0) {
	    int remaining_minutes=(int)(((100/(percentage-m_MeasurementStartPercentage-percentage_error))-1)*(QDateTime::currentDateTime().toTime_t()-m_MeasurementStartTime)/60)+1;
	    eventLabel->setText("Time remaining (min.):");
	    eventDisp->display(remaining_minutes);
	    //std::cout << "PixScanPanel::updateStatus:  m_StatusUpdatesRecieved="<<m_StatusUpdatesRecieved << " at " << std::string(QDateTime::currentDateTime().toString("hh:mm:ss").toLatin1().data()) 
	    //      << " = " <<QDateTime::currentDateTime().toTime_t() <<std::endl;
	  }
	  
	  m_lastPercentage=percentage;
	}
      }
    }
    if(usedMem->isEnabled())     usedMem->display(sramFillLevel);
    if(procEvents0->isEnabled()){
      if (stepsLabel0->text()=="Processed scan steps loop 0:") 
	procEvents0->display(nSteps0);
      else
	procEvents0->display(nMasks);
    }
    if(procEvents1->isEnabled()) procEvents1->display(nSteps1);
    if(procEvents2->isEnabled()) procEvents2->display(nSteps2);
    if(eventRate->isEnabled())   eventRate->display(event_Rate);
    if(trgRate->isEnabled())     trgRate->display(triggerRate);

//     currFe->setEnabled(in_currFe>=0);
//     currFeLabel->setEnabled(in_currFe>=0);
//     if(in_currFe>=0)
//       currFe->display(in_currFe);
//     else
//       currFe->display(0);
  }
  switch(status){
  case 1: //finished scan
    runButton->setEnabled(true);
    runButton->setText("Start Scan");
    runButton->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    scanLabel->setEnabled(true);
    saveFile->setEnabled(true);
    rawFileName->setEnabled(saveFile->isChecked() && !appIndRawfile->isChecked());
    rawFileBrowse->setEnabled(saveFile->isChecked() && !appIndRawfile->isChecked());    
    scanFileName->setEnabled(saveFile->isChecked());
    scanFileBrowse->setEnabled(saveFile->isChecked());    
    appIndRawfile->setEnabled(saveFile->isChecked());
    statusText->setText("idle");
    statusText->repaint();
    fileEnabled(saveFile->isChecked());
    eventDisp->display(0);
    repaint();
    emit enableBocAnaScan(true);
    break;
  case 2: //downloading histos from ROD
    statusText->setText("downloading data");
    statusText->repaint();
    repaint();
    break;
  case 3: //writing histos from ROD
    statusText->setText("writing data");
    statusText->repaint();
    repaint();
    break;
  case 4: //MAControl is at work
    statusText->setText("post-scan actions");
    statusText->repaint();
    repaint();
    break;
  case 5: // fitting/downloading loop 0
    statusText->setText("end loop 0");
    statusText->repaint();
    repaint();
    break;
  case 6: // fitting/downloading loop 1
    statusText->setText("end loop 1");
    statusText->repaint();
    repaint();
    break;
  case 7: // fitting/downloading loop 2
    statusText->setText("end loop 2");
    statusText->repaint();
    repaint();
    break;
  case 8:// for multi-threaded scan mode
    statusText->setText("wait f. other grps");
    statusText->repaint();
    repaint();
    break;
  case 9:// needed if observed Rod is not determinded (probl. due to error)
    statusText->setText("no Rod observed");
    statusText->repaint();
    repaint();
    break;
  case 10:// rod initialisation
    statusText->setText("initialising scan");
    statusText->repaint();
    repaint();
    break;
  case 11:// kept disabled from someone else
    statusText->setText("waiting...");
    statusText->repaint();
    repaint();
    break;
  default: // scan on at least one ROD is still running
    statusText->setText("scanning");
    statusText->repaint();
  }

  return;
}

void PixScanPanel::rawBrowseButton_clicked(const char *in_path){
  bool go_for_it = (in_path!=0);
  QString fname;
  if(!go_for_it){
    QString old_path = QString::null;
    if(!rawFileName->text().isEmpty()){
      old_path = rawFileName->text();
      int pos = old_path.lastIndexOf("/");
      if(pos>=0)
	old_path.remove(pos,old_path.length()-pos);
    } else{
      // use default data path/name
      old_path = (((ConfString&)m_engine.getOptions()["paths"]["defDataPath"]).value()).c_str();
    }
    QStringList filter;
    filter += "Raw data file (*.raw)";
    filter += "Any file (*.*)";
    QFileDialog fdia(this, "Specify name of new raw data file", old_path);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::AnyFile);
    go_for_it = (fdia.exec() == QDialog::Accepted);
    if(go_for_it)
      fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
  } else
    fname = in_path;
  if(go_for_it){
    if(fname.right(4)!=".raw" && fname!="")
      fname+=".raw";
    disconnect( rawFileName, SIGNAL( textChanged(const QString&) ), this, SLOT( readFromHandles() ) );
    rawFileName->setText(fname);
    ((ConfString&)m_scanCfg->config()["general"]["sourceRawFile"]).m_value = std::string(fname.toLatin1().data());
    connect( rawFileName, SIGNAL( textChanged(const QString&) ), this, SLOT( readFromHandles() ) );
  }
  return;
}
void PixScanPanel::browseButton_clicked(const char *in_path){
  bool go_for_it = (in_path!=0);
  QString fname;
  if(!go_for_it){
    QString old_path = QString::null;
    if(!scanFileName->text().isEmpty()){
      old_path = scanFileName->text();
      int pos = old_path.lastIndexOf("/");
      if(pos>=0)
	old_path.remove(pos,old_path.length()-pos);
    } else{
      // use default data path/name
      old_path = (((ConfString&)m_engine.getOptions()["paths"]["defDataPath"]).value()).c_str();
    }
    QStringList filter;
    filter += "DB ROOT file (*.root)";
    filter += "Any file (*.*)";
    QFileDialog fdia(this, "Specify name of new RootDB data-file", old_path);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::AnyFile);
    go_for_it = (fdia.exec() == QDialog::Accepted);
    if(go_for_it)
      fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
  } else
    fname = in_path;
  if(go_for_it){
    if(fname.right(5)!=".root" && fname!="")
      fname+=".root";
    scanFileName->setText(fname);
    if(m_currStdScanID==PixScan::SOURCE_SCAN || m_currStdScanID==PixScan::FE_ST_SOURCE_SCAN){
      appIndRawfile->setChecked(true);
      rawBrowseButton_clicked(fname.left(fname.length()-5).toLatin1().data());
    } else if(m_currStdScanID>=0){
      appIndRawfile->setChecked(false);
      rawBrowseButton_clicked("");
    }
    // get list of scan labels and fill local memory of it
    m_scanLabels.clear();
    try{
      PixConfDBInterface *myDB = DBEdtEngine::openFile(fname.toLatin1().data(), false);
      DBInquire *root = myDB->readRootRecord(1);
      for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
	QString scname = (*it)->getDecName().c_str();
	scname.remove(0,1);
	scname.remove(scname.length()-1,1);
	m_scanLabels.push_back(scname);
      }
      delete myDB;
    } catch(...){
      m_scanLabels.clear();
    }
    emit scanFileChanged(fname.toLatin1().data());
  }
  return;
}
void PixScanPanel::fileEnabled(bool ena)
{
  QString fname;
  if(ena){
    fname = scanFileName->text();
    browseButton_clicked(fname.toLatin1().data());
  }else{
    m_scanLabels.clear();
    fname = "";
  }
  emit scanFileChanged(fname.toLatin1().data());
}
void PixScanPanel::loadLoopPts(int ID){
  if(ID<0 || ID>MAX_LOOPS) return;
  if(LoopBox[ID]==0) return;

  int nPts=0;
  char line[2000];
  double testDbl;
  bool isOK;
  QStringList filter;
  filter += "Text file (*.txt)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Load ascii-list of scan points");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
    FILE *in = fopen(fname.toLatin1().data(),"r");
    if(in!=0){
      loopPtsBox[ID]->clear();
      while(fgets(line,2000,in)!=0){
	QString tmpstr = line;
	tmpstr.remove(tmpstr.length()-1,1);//remove trailing '\n'
	testDbl = tmpstr.toDouble(&isOK);
	if(isOK){
	  if(nPts==0)
	    loopStart[ID]->setValue(testDbl);
	  else
	    loopStop[ID]->setValue(testDbl);
	  nPts++;
	  loopPtsBox[ID]->addItem(tmpstr);
	}
      }
      loopStep[ID]->setValue(nPts);
      fclose(in);
    } else
      QMessageBox::warning(this,"loadInnerPts()","Error opening file "+fname);
  }
  // update from ListBox seems to fail occaisonally, do so explicitly
  readFromHandles();
  return;
}

void PixScanPanel::updateCrateGrpSel(){
  crateBox->clear();
  rodBox->clear();

  for(std::vector<STRodCrate*>::iterator crIT = m_engine.getSTRodCrates().begin(); crIT != m_engine.getSTRodCrates().end(); crIT++)  
    crateBox->addItem((*crIT)->getName().c_str());
  
  if(crateBox->count()==0) return;
  STRodCrate *crate = m_engine.getSTRodCrates()[crateBox->currentIndex()];
  for(int i=0; i<crate->nGroups(); i++)
    rodBox->addItem(crate->getGrpName(i).c_str());

}

void PixScanPanel::observedRodChanged(int index)
{
  m_engine.setObservedGroup(crateBox->currentIndex(), index);
}

void PixScanPanel::observedCrateChanged(int index)
{
  // show Rods belonging to observed Crate
  rodBox->clear();
  
  if(crateBox->count()==0) return;
  STRodCrate *crate = m_engine.getSTRodCrates()[index];
  for(int i=0; i<crate->nGroups(); i++)
    rodBox->addItem(crate->getGrpName(i).c_str());

  m_engine.setObservedGroup(crateBox->currentIndex(), rodBox->currentIndex());

}
void PixScanPanel::fixVCAL(bool doFix){
  if(doFix)
    setVcal->setValue(0x1fff);
  else
    setVcal->setValue(0);
}
void PixScanPanel::VcalChanged(int value)
{
  bool isFix = (value==0x1fff);
  disconnect( fixedVCALBox, SIGNAL( toggled(bool) ), this, SLOT( fixVCAL(bool) ) );
  fixedVCALBox->setChecked(isFix);
  connect( fixedVCALBox, SIGNAL( toggled(bool) ), this, SLOT( fixVCAL(bool) ) );
}
void PixScanPanel::fixSdelay(bool doFix)
{
  if(doFix)
    strDelRange->setValue(0x1f);
  else
    strDelRange->setValue(5);
}
void PixScanPanel::SdelayChanged(int value)
{
  bool isFix = (value==0x1f);
  disconnect( fixedSdelBox, SIGNAL( toggled(bool) ), this, SLOT( fixSdelay(bool) ) );
  fixedSdelBox->setChecked(isFix);
  connect( fixedSdelBox, SIGNAL( toggled(bool) ), this, SLOT( fixSdelay(bool) ) );
}
void PixScanPanel::openTableMenu(QTableWidgetItem *item){
  if(QApplication::mouseButtons()==Qt::RightButton && item!=0 && item->column()==3 && item->text().indexOf("VECTOR")){ 
    // right mouse click and last column for VECTORS only
    int row = item->row();
    int col = item->column();
    QComboBox *cb = dynamic_cast<QComboBox *>(configParameterTable->cellWidget(row,col));
    if(cb!=0){
      char line[2000];
      QStringList filter;
      filter += "Text file (*.txt)";
      filter += "Any file (*.*)";
      QFileDialog fdia(this, "Load ascii-list of vector entries");
#if defined(QT5_FIX_QDIALOG)
      fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
      fdia.setNameFilters(filter);
      fdia.setFileMode(QFileDialog::ExistingFile);
      if(fdia.exec() == QDialog::Accepted){
	QString fname = fdia.selectedFiles().at(0);
	fname.replace("\\", "/");
	FILE *in = fopen(fname.toLatin1().data(),"r");
	if(in!=0){
	  while(fgets(line,2000,in)!=0){
	    QString tmpstr = line;
	    tmpstr.remove(tmpstr.length()-1,1);//remove trailing '\n'
	    if(configParameterTable->item(row,1)->text()=="FLOAT VECTOR")
	      cb->addItem(tmpstr, QVariant(tmpstr.toFloat()));
	    else
	      cb->addItem(tmpstr, QVariant(tmpstr.toInt()));
	  }
	  cb->setCurrentIndex( 0 );
	}
	readFromHandlersFromTable (row,col);
      }
    }
  }
}

void PixScanPanel::scanRunning(int type)
{
  scanLabel->setText(m_engine.getLastPxScanConfig().anaLabel.c_str());

  // initialise variables for rem. time estimation
  m_StatusUpdatesRecieved=0;
  m_MeasurementStartTime=-1;

  // adjust layout

  switch(type){
  case 4: // test beam mode
    runButton->setEnabled(false);
    runButton->setText("TB Mode");
    stepsLabel2->setEnabled(false);
    stepsLabel1->setEnabled(false);
    stepsLabel0->setEnabled(false);
    stepsLabel0->setText("Processed scan steps loop 0:");
    procEvents2->setEnabled(false);
    procEvents1->setEnabled(false);
    procEvents0->setEnabled(false);
    eventLabel->setEnabled(true);
    eventLabel->setText("Processed Events:");
    usedMem->setEnabled(true);
    usedMemLabel->setEnabled(true);
    eventRate->setEnabled(true);
    eventRateLabel->setEnabled(true);
    trgRate->setEnabled(true);
    trgRateLabel->setEnabled(true);
    break;
  case 3: // source scan mode with scan
    runButton->setEnabled(true);
    runButton->setText("Abort");
    stepsLabel2->setEnabled(true);
    stepsLabel1->setEnabled(true);
    stepsLabel0->setEnabled(true);
    stepsLabel0->setText("Processed scan steps loop 0:");
    procEvents2->setEnabled(true);
    procEvents1->setEnabled(true);
    procEvents0->setEnabled(true);
    eventLabel->setEnabled(true);
    eventLabel->setText("Processed Events:");
    usedMem->setEnabled(true);
    usedMemLabel->setEnabled(true);
    eventRate->setEnabled(true);
    eventRateLabel->setEnabled(true);
    trgRate->setEnabled(true);
    trgRateLabel->setEnabled(true);
    break;
  case 2: // source scan mode w/o scan
    runButton->setEnabled(true);
    runButton->setText("Stop");
    stepsLabel2->setEnabled(false);
    stepsLabel1->setEnabled(false);
    stepsLabel0->setEnabled(false);
    stepsLabel0->setText("Processed scan steps loop 0:");
    procEvents2->setEnabled(false);
    procEvents1->setEnabled(false);
    procEvents0->setEnabled(false);
    eventLabel->setEnabled(true);
    eventLabel->setText("Processed Events:");
    usedMem->setEnabled(true);
    usedMemLabel->setEnabled(true);
    eventRate->setEnabled(true);
    eventRateLabel->setEnabled(true);
    trgRate->setEnabled(true);
    trgRateLabel->setEnabled(true);
    break;
  case 1: // strobe-induced scanning, mask display
    runButton->setEnabled(true);
    runButton->setText("Abort");
    stepsLabel2->setEnabled(true);
    stepsLabel1->setEnabled(true);
    stepsLabel0->setEnabled(true);
    stepsLabel0->setText("Processed mask steps:");
    procEvents2->setEnabled(true);
    procEvents1->setEnabled(true);
    procEvents0->setEnabled(true);
    eventLabel->setEnabled(true);
    eventLabel->setText("Time remaining: initialising...");
    usedMem->setEnabled(false);
    usedMemLabel->setEnabled(false);
    eventRate->setEnabled(false);
    eventRateLabel->setEnabled(false);
    trgRate->setEnabled(false);
    trgRateLabel->setEnabled(false);
   break;
  case 0: // strobe-induced scanning, no mask display
  default:
    runButton->setEnabled(true);
    runButton->setText("Abort");
    stepsLabel2->setEnabled(true);
    stepsLabel1->setEnabled(true);
    stepsLabel0->setEnabled(true);
    stepsLabel0->setText("Processed scan steps loop 0:");
    procEvents2->setEnabled(true);
    procEvents1->setEnabled(true);
    procEvents0->setEnabled(true);
    eventLabel->setEnabled(true);
    eventLabel->setText("Time remaining: initialising...");
    usedMem->setEnabled(false);
    usedMemLabel->setEnabled(false);
    eventRate->setEnabled(false);
    eventRateLabel->setEnabled(false);
    trgRate->setEnabled(false);
    trgRateLabel->setEnabled(false);
    break;
  }

  runButton->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
  
  scanLabel->setEnabled(false);
  saveFile->setEnabled(false);
  scanFileName->setEnabled(false);
  scanFileBrowse->setEnabled(false);
  rawFileName->setEnabled(false);
  rawFileBrowse->setEnabled(false);
  appIndRawfile->setEnabled(false);
  statusText->setText("intialising scan");
  statusText->repaint();
  
  m_engine.setObservedGroup(crateBox->currentIndex(), rodBox->currentIndex());

}

void PixScanPanel::setFeReg(int regId){
  if(regId>=0 && regId<MAX_LOOPS)
    loopFERegName[regId]->setEnabled(loopActive[regId]->isChecked() && loopType[regId]->currentText()=="FEI4_GR");
}
void PixScanPanel::enableRawFileStuff(bool){
  bool setEn = saveFile->isChecked() && !(appIndRawfile->isChecked());
  rawFileName->setEnabled(setEn);
  rawFileBrowse->setEnabled(setEn);
}
void PixScanPanel::toggleClusHis(){
  CSdatafill->toggle();
  CSdatakeep->toggle();
  CTOTdatafill->toggle();
  CTOTdatakeep->toggle();
  CChargedatafill->toggle();	//DLP
  CChargedatakeep->toggle();	//DLP
  CPositiondatafill->toggle();	//DLP
  CPositiondatakeep->toggle();	//DLP
  CSChargedatafill->toggle();
  CSChargedatakeep->toggle();
  CSTOTdatafill->toggle();
  CSTOTdatakeep->toggle();
  CnSeedsFill->toggle();
  CnSeedsKeep->toggle();
  CSeedToTFill->toggle();
  CSeedToTKeep->toggle();
  CSeedLVL1Fill->toggle();
  CSeedLVL1Keep->toggle();
} 
void PixScanPanel::setEvtLabel(QString txt){
  if(txt=="seconds")
    eventsLabel->setText("Seconds per scan point:");
  else
    eventsLabel->setText("Events per scan point:");
}
void PixScanPanel::showAddModLabels(bool show){
  if(show){
    addModLabel1->show();
    addModLabel2->show();
    addModLabel3->show();
    addModLabel4->show();
    addModLabel5->show();
  }else{
    addModLabel1->hide();
    addModLabel2->hide();
    addModLabel3->hide();
    addModLabel4->hide();
    addModLabel5->hide();
  }
}
void PixScanPanel::goToTbMode(bool tbMode){
  //nEvents->setDisabled(tbMode);
  nEvents->setMinimum(tbMode?0:1);
  if(tbMode)nEvents->setValue(0);
  eventsLabel->setText(tbMode?"Events per scan point (0=run until stopped):":"Events per scan point:");
}
void PixScanPanel::setSrcMonBox(){
  if(rawFileName->text().isEmpty() && !appIndRawfile->isChecked()){
    sourceMonBox->setChecked(false);
    sourceMonBox->setEnabled(false);
  } else
    sourceMonBox->setEnabled(true);
}
