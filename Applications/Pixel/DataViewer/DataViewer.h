#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include "ui_DataViewer.h"
#include <QThread>

class QApplication;
class QTreeWidgetItem;
class QAction;
class PlotFactory;
class ModuleAction;
class PixDBData;
namespace PixLib{
  class PixConfDBInterface;
}

class DataViewer : public QWidget, public Ui::DataViewer {

    Q_OBJECT

 public:
  class FitThread : public QThread {

  friend class DVFitting;

  public:
    FitThread(PixDBData &data, int fittype, int chip, float chicut, float xmin, float xmax, float fracErr, bool useNdof);
    void run();
  private:
    PixDBData &m_data;
    int m_fittype;
    int m_chip;
    float m_chicut;
    float m_xmin;
    float m_xmax;
    float m_fracErr;
    bool m_useNdof;
  };

  DataViewer(QWidget * parent = 0, Qt::WindowFlags flags = Qt::Window , bool waferView=false);
  ~DataViewer();

  int m_plotChip;

 public slots:
  void browseFile(const char *fname);
  void browseFile(){browseFile(0);};
  void updateView();
  void closeFile();
  void processDoubleClick(QTreeWidgetItem *item, int col);
  void processRightClick(const QPoint &point);
  void processHistoClick(int x, int y, float cont, bool dblClick);
  void getHistoPixel(float &x, float &y);
  void setChipToPlot();
  void setChipToPlot(bool);
  void setScanptBoxes(int loop_level);
  void saveOpt();
  void plotScan(bool mode);
  void plotScanPlain(){plotScan(false);};
  void plotScanFit(){plotScan(true);};
  void plotScanAvg();
  void plotOper(bool isScan);
  void plotOperScan(){plotOper(true);};
  void plotOperItems(){plotOper(false);};
  void plotCorrel();
  void fitScan();
  void generateMask(bool);
  void genMaskPlain(){generateMask(false);};
  void genMaskOper(){generateMask(true);};

 private:
  int m_lastQactID;
  bool m_waferView;
  QTreeWidgetItem *m_scans, *m_chipTests, *m_dcsReadings, *m_srvRecs, *m_gadcVals; 
  PixLib::PixConfDBInterface *m_file;
  PlotFactory *m_pfact;
  ModuleAction *m_lastAction;
  int m_lastPdbFID;
  std::vector<int> m_lastPdbFPars;

};

#endif //  DATAVIEWER_H
