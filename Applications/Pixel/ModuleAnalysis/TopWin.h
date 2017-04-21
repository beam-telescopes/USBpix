#ifndef TOPWIN_H
#define TOPWIN_H

#include "ui_TopWinBase.h"
#include "FileTypes.h"
#include <vector>
#include <map>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class FitWin;
class FileWin;
class RMain;
class ModItem;
class MATH2F;
class DatSet;
class OptWin;
class CutPanel;
class TotRecal;
class QMenu;
class QApplication;
class Q3Process;
class TDirectory;
class TLogFile;
class MAFitting;
class PixelDataContainer;
#ifdef PIXEL_ROD // can link to PixScan
namespace PixLib{
 class DBInquire;
 class PixScan;
}
class wrapPs;
#endif

/*! main GUI panel that controls everything else
 *  in ModuleAnalysis
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de> et al.
 */

class TopWin : public QWidget, public Ui::TopWinBase
{
  Q_OBJECT
    
    friend class MATH2F;
    friend class TotRecal;

 public:
  /*! constructor - first four arguments are for QT use,
   *  last tells MAF that it's not a stand-alone routine
   * and thus must not shut down QT upon closing
   */
  TopWin( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 , bool depends=FALSE, QApplication *app=0);
  /*! destructor - does nothing for now (TO DO: check for
   *  memory leaks!)
   */
  ~TopWin();
  
  /*! load ascii-TurboDAQ data file into
   *  tree-view
   */
  ModItem* LoadData(ModItem*, const char *, const char *, int, const char *cfgname=NULL);
  /*! loads a DatSet/PixelDataContainer item into
   *  the main-panel tree-view
   */
  ModItem* DataToTree(ModItem*, PixelDataContainer*);
  /*! get DAC value of 1D scan that's supposed to end up in
   *  2D scan (i.e. the scan point of the 1D scan)
   */
  static int getDacVal(TLogFile *log1D, TLogFile *log2D, int offset=0, int mapPt=10);
  /*! check if DAC value of 1D scan is inside scan range of 2D scan
   */
  static int CheckScanpt(TLogFile *log1D, TLogFile *log2D, int offset=0, int mapPt=10);
  static int getMinDAC(std::vector<ModItem*>, int mapPt=-1, bool reverse=false);
  /*! time/date finder (get resp. line from logfile comment)
   *
   */
  static void DateFromLog(const char *log_comments, QString &dateLine, QString &theTime);
  /*! turn time/date into double
   *  (added 7/5/04 by LT)
   */
  static double BI_getTimeFromStrings(QString &theDate, QString &theTime); 
  // misc functions mostly for internal use (make private??)
  void     CalibrateToT(ModItem *item, ModItem *ToTcal=NULL);
  void     PlotMasked(ModItem*);
  /*! get a reference to the QApllication
   *  (needed for GUI event processing and updates)
   */
  QApplication* getApp(){return m_app;};

  /*! get list of all module item
   *  i.e. items independet of their
   *  association with staves/sectors or not
   */
  std::vector<ModItem*> getAllModuleItems();

  RMain *rootmain;
  OptWin *options;
  QString m_verstrg;
  FileWin *m_fwin;
  FitWin *m_FitWin;
  QMenu *m_editpop, *m_mainpop, *m_anapop, *m_optpop, *m_helppop, *m_submenu1, *m_submenu2;
  CutPanel *m_cutpanel;
  bool m_PlotIsActive;
//******************** added by JW *******************************************
  std::map<int, std::vector<ModItem*> > m_itemmap;
  std::map<QString, std::vector<ModItem*> > m_labelmap;
  bool SystestMode;
//*****************************************************************************
 private:
  ModItem *m_lastPitem;
  int m_lastPdbID;
  std::vector<int> m_lastPdbFPars;
  int m_lastPdbFID;
  int m_lastchip;
  bool m_depends;
  QString m_browser;
  Q3Process *m_brwsproc;
  ModItem* FitToTComb(ModItem *item);
  ModItem * GetCurrMod();
  QApplication *m_app;
  MAFitting *m_fitting;

#ifdef PIXEL_ROD // can link to PixScan
  //  std::vector<wrapPs *> m_pixScans;
#endif
  std::vector<QString> m_reports;  // stores QA report text for all modules
  int m_endRow;

 protected:
  TDirectory *m_memdir;

 signals:
  void closed();

 public slots:

  void setText(QString txt){textwin->setText(txt);textwin->repaint();};
  void ExitButton_clicked();
  void ModuleButton_clicked(){ModuleButton_clicked(0,0,false);};
  void newStave(){ModuleButton_clicked(0,0,true);};
  ModItem* ModuleButton_clicked(const char*, ModItem *parent=0, bool isStave=false);
  void assignToStave(ModItem *module);
  void assignToStave(ModItem *module, ModItem *stave);
  void Cuts_open();
  void Replot();
  void Options_clicked();
  void Options_save();
  void Options_saveAs(){Options_saveAs(NULL);};
  void Options_saveAs(const char *path);
  void Options_load();
  void Options_loadFrom(){Options_loadFrom(NULL);};
  void Options_loadFrom(const char *path);
  void PlotCorrel();
  void RootCommand();
  void RootMacro();
  void ModuleList_return(Q3ListViewItem* item);
  void ModuleList_itemselected(ModItem*);
  void ModuleList_itemselected(Q3ListViewItem* item)
    {ModuleList_itemselected((ModItem*)item);};
  void ModuleList_rpr(ModItem*, int lastPdbID=-1);
  void ModuleList_rpr(Q3ListViewItem* item,const QPoint&, int i)
    {if(i==0) ModuleList_rpr((ModItem*)item);};		
  void LoadData(ModItem*);
  ModItem* LoadData(ModItem* item, int type, const char *path, const char *label);
  void LoadList(ModItem *item);
  void LoadList(ModItem *item, std::vector<ModItem*> &loadedList);
  void LoadStd(ModItem*);
  void LoadMultiModule();
  void LoadMultiModuleRDB();
  void SaveData(ModItem *item, int doall){SaveData(item,doall,NULL,NULL);};
  void SaveData(ModItem *item, int doall, const char *in_path, const char *in_comment = NULL);
  void SaveAllData(const char *in_path=0, const char *in_comment=0);
  int  saveDataCore(ModItem *item, bool doall, TDirectory *outdir);
  void LoadRoot(ModItem*,const char*, bool showlist=false, const char *subpath=0, bool *loadData=0);
  void LoadRootPixLib(bool link);
  void LoadRootPixLib(){LoadRootPixLib(false);};
  void ListRootPixLib(){LoadRootPixLib(true);};
  void loadPixScanData(void *DBscan, int summary=0,bool link=false, const char *fname=0);
  void loadPixScanData(std::vector<ModItem*> &data, void *DBscan, int summary=0,bool link=false, 
		       const char *fname=0, const char *mname=0);
  void LoadDataCurrMod();
  void LoadListCurrMod();
  void LoadStdCurrMod();
  void SaveDataCurrMod();
  void SaveSelDataCurrMod();
  void LoadRoot_Menu(){LoadRoot_Menu(true,NULL);};
  void LoadRootAll(){LoadRoot_Menu(false,NULL);};
  ModItem* LoadRoot_Menu(bool interactive, const char *in_path, ModItem *parent=0, bool *loadData=0);
  ModItem* RetrieveOrgData(ModItem *);
  void ModChipSel();
  void ModChipSel2();
  void RemData(ModItem*, bool do_confirm=true);
  void RemRaw(ModItem*, bool do_prompt=true);
  void RemFit(ModItem *, bool do_prompt=true);
  void RemRawCurrMod();
  void ClearFitsCurrMod();
  void RenameMod(ModItem*);
  void setStavePos(ModItem*);
  void RenameCurrMod();
  void RemoveMod(ModItem *item, bool ask=true);
  void RemoveCurrMod();
  void RemoveAllMod(){RemoveAllMod(true);};
  void RemoveAllMod(bool warn);
  void CreatePDB(ModItem *item);
  void CreatePDBCurrMod();
  void PDBProf();
  void PlotCorrelCurrMod();
  void PlotMaskedCurrMod();
  void PlotMultiGraphCurrMod();
  void CompareBadChans(ModItem *);
  void CompareBadChansCurrMod();
  void BiasStab(ModItem*);
  void BiasStabCurrMod();
  int  CheckChilds(ModItem *);
  void ProcStd(std::vector<ModItem*> itemarr, std::vector<int> exists);
  void MapAction(int, int, int, float, int);
  void LogToTree(DatSet *, ModItem *);
  void MakeStd(ModItem *item);
  void SetCurrMod(Q3ListViewItem *);
  void PlotMultiGraphs(ModItem *item);
  void RunQAReport(ModItem *item);
  void RunQAReportCurrModule();
  void UpdatePixVal();
  void helpAbout();
  void helpUserRef();
  void helpCoding();
  void openUrl(const char *url);
  void SaveCanvas(const char *path=0);
  void PrintCanvas();
  void FitScurves(bool do_all);
  void FitScurves(){FitScurves(false);};
  void FitAllScurves(){FitScurves(true);};
  void FitAll(int fit_type=0, int fit_min=0, int fit_max=0, int chi_max=-1, bool do_all=false);
  void RemoveAllRaw(){RemoveAllRaw(0,true);};
  void RemoveAllRaw(ModItem *in_item, bool do_prompt);
  void ClearAllFits(){ClearAllFits(0,true);};
  void ClearAllFits(ModItem *in_item, bool do_prompt);
  void CombHitMaps();
  void CombHitMaps(int scanType, std::vector< std::vector<ModItem*> >dataVec, std::vector< std::vector<int> > scanPars, 
		   bool reverse, bool clear, const char *label);
  void CombScurves();
  void CombScurves(int scanType, std::vector< std::vector<ModItem*> >dataVec, std::vector< std::vector<int> > scanPars, 
		   bool reverse, bool clear, const char *label);
  void CheckToTsrc();

  void getModuleRootFromRDB();
  void getStaveRootFromRDB();
  void getListRootFromRDB();
  void getStaveConfig();
  void staveMechPDB();
  void plotDBscan(ModItem *item, bool, bool pltAvg=false);
  void showReport(int);
  void qTableClicked( int row, int col, int button);
  // debugging
  void memoryTest();
  void rootdbFitAll();
  void setScaptBoxes(int);

//******************** added by JW *******************************************
  void DateFromSTLog(const char *log_comments, QString &theDate, QString &theTime);
  void GetItemmapByTimestamp();               //gets similar scanitems by time in the logfile
  void GetLabelmap();                         //gets similar scanitems by their label
  void PlotSystestSummary(int sumtype, QString Plot_label, int ComboIndex);
  void CreateDifferencePlots();
//******************************************************************************
// added by JW on 02/02/2005
  //SLOTS
  void PlotMultiMean();
  void PlotMultiSig();
  void OverlayMultiMean();
  void OverlayMultiSig();
  void PlotGraphThrSummary();
  void PlotGraphThrDispSummary();
  void PlotGraphNoiseSummary();
  void PlotGraphDigSummary();
  void PlotGraphAnaGoodSummary();
  void PlotGraphGoodBumpSummary();
  void SelectOverview();
  //functions
  void PlotMultiProjection(int type, bool overlay);	
  void PlotModuleGraphSummary(int scantype, int sumtype, QString label, int meastype=-1, bool chipwise=true);
  void UpdateChipVal();
  ModItem* getSimilarData(ModItem *module, int stdType);
  bool checkDBLinkExists(const char *name, const char *path, const char *modName);

};
#endif // TOPWIN_H

