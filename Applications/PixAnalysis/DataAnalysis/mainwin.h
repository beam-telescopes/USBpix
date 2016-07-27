#ifndef MAINWIN_H
#define MAINWIN_H

#include <qvariant.h>
#include <qwidget.h>
#include <string.h>
//#include <TH2.h>
#include "mainwinbase.h"
#include "Module.h"
#include <PixConfDBInterface.h>
#include <qapplication.h> 
#include <iostream>


#define MGEO_NSTRUCTS 12


class QApplication;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QSpinBox;
class QFrame;
class QLineEdit;
class QListView;
class QListViewItem;
class QString;
class TH2F;
class TH1F;
class Moditem;
class QListBox;
class cutPanel;
class DAEngine;
class TopWin;

class MainWin : public MainWinBase  
{
    Q_OBJECT

public:
    MainWin(DAEngine *eng,  QApplication *a, QWidget *parent = 0, const char* name = 0, WFlags fl = 0);
    ~MainWin();
    
    double calcMean(TH2F *,TH1F *);
    Moditem * findModitem(const char *);
    std::string getModName(PixLib::DBInquire *inq);
    int findComboItem(const char * );

public slots: 
 
  virtual void fileOpen();  
  virtual void fileOpen(const char *filename);
  virtual void fileExit();
  virtual void doubleclickModule(QListViewItem *);
  virtual void startAna();
  // virtual void clearAna();
  void deleteModule(); 
  void deleteModuleAll();
  //  char *addTestCombo(const char *testname);
  // virtual void comboChange(int); //wenn neuer test in combo gewaehlt wird
  virtual void setCuts();//aufrufen des cut-Panels
  //void loadCuts(const char *file); // loads cuts into cut panel
  void loadModules();
  void loadScans();
  void showStatusMap();
  void doubleClickModMap(void *);
  void startMA(const char *,const char *, const char *);


signals:
  void s_updatecuts(QListBox *lb);
  void updateMainWin();


private:
  //Methoden
  void loadGeo();
  void buildStaves();

  //Members
  DAEngine *m_engine;
  QApplication *m_app;
  TopWin *m_MAFwin;

  //Menue fuer Analyse
  QPopupMenu *anaMenu, *cutMenu, *modMap; 
  QAction* anaStartAction;

  //CutPanel
  cutPanel *m_cutpanel;

  QString m_qparSN[MGEO_NSTRUCTS];
};

#endif
