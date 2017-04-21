#ifndef QAREPORT
#define QAREPORT

#include "ui_QAReportBase.h"
#include <vector>

class ModItem;
class CutPanel;
class RMain;
class TopWin;
class TH2F;
class BMPWin;
class Q3ListViewItem;
class QApplication;
class QString;

class QAReport : public QDialog, public Ui::QAReportBase{

 friend class BMPWin;

  Q_OBJECT

 public:
  QAReport(ModItem *modit, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, 
	   Qt::WFlags fl = 0 , const char *outpath=NULL, QApplication *app=0);
  ~QAReport();
  // information need externally
  std::vector<ModItem*> m_badpixit;
  bool m_IV_OK, m_DIG_OK, m_ANA_OK, m_SRC_OK, m_all_OK;
  double m_IV_bdV, m_IV_oper, m_IV_100, m_IV_max, m_VDD, m_VDDA;
  float m_avgThr, m_sdThr, m_avgNse, m_avgXtalk, m_avgITThresh, m_sdITThresh, m_avgTOT, m_sdTOT;
  float m_minVDD, m_maxVDD, m_minVDDA, m_maxVDDA, m_minTHR, m_maxTHR, m_maxXCK;
  int m_GDACfail, m_nBadPix;
  // static functions
  static float calcNeffBad(float val, float mean, float sigma, float weight);

 public slots:
  void  LoadPath(const char *path);
  void  LoadPath(){LoadPath(NULL);};
  void  StartReport();
  float CountChi(TH2F * chi_histo, TH2F* mask);
  void  copyToCB();
  void  fillTable();
  void  saveTable(const char *path);
  void  recalcMech();
  void  processLink(const QString &link_text);

 private:
  QApplication *m_app;
  CutPanel *m_cutpnl;
  ModItem *modit;
  RMain *rmain;
  TopWin *m_parent;
  std::vector<TH2F*> m_badpix_hi;
  int m_selcutID, m_neff;
  bool m_usedef;

  void BadPixelTest(int cutid, const char *cutname, TH2F *hitocheck, float convert, 
		    float offset=0, TH2F *ignore=NULL, bool zerosupp=false);
  bool ModuleClass(int cutid, const char *cutname, std::vector<float> &valarr, std::vector<bool> &passed, long int plotid=0);
  bool ModuleClass(int cutid, const char *cutname, std::vector<float> &valarr, std::vector<bool> &passed, std::vector<long int> vpid);
};

#endif // QAREPORT
