#ifndef CUTPANEL_H
#define CUTPANEL_H

#define NCUTS  19
#define NMCUTS 29
#define NCCUTS 7
#define START_RANGE 38
#define UNDEFCUT -999999
#define DEFCUTVER 7

#include "ui_CutPanelBase.h"
#include <q3listview.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <string>

class TopWin;
class optClass;
class Q3HBoxLayout; 
class QCheckBox;
class QLabel;
class QSpinBox;
class QLineEdit;
class QString;
class CutPanel;

/*! Structure containing min/max values and labels for one QA cut
 */

typedef struct {
  std::string label; // the label of the cut
  bool enabled;   // whether the cut is enabled
  double min;   // minimum
  double max;   // maximum
  std::string unit;
} CutParam;

/*! Visual interface to the CutParam cut-structure
 */

class CutWidget : public QWidget{

  friend class CutPanel;

  Q_OBJECT

 public:
  CutWidget(QWidget *, const char*);
  ~CutWidget();
  CutParam *m_cp;

 public slots:
  virtual void Reset();
  virtual void Save();
  virtual void LoadCuts(CutParam *cp){m_cp = cp; Reset();};

 protected:
  Q3HBoxLayout *Layout;

 private:
  QLabel *CutLabel;
  QCheckBox *Enabled;
  QLineEdit *UpperCut, *LowerCut;
  QLabel *MinLabel, *MaxLabel, *UnitLabel;
};

/*! Stores the actual set of cut values displayed in CutPanel
 */

class CutClassItem : public Q3ListViewItem{

  friend class CutPanel; 

 public:
  CutClassItem(Q3ListView*,QString);
  ~CutClassItem();
  void SetDefaultCuts();
  void SetPath(const char *path){m_path = path;};  
  int  GetVersion(){return m_version;};
  int  itemID();
 protected:
  int m_version;
  CutParam cp[NCUTS+NMCUTS+NCCUTS];
 private:
  std::string m_path;
};

/*! Displays the classification cuts, ie is the
 *  visual interface to set of CutClassItem.
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 */

class CutPanel : public QDialog, public Ui::CutPanelBase{

  Q_OBJECT
    
 public:
  CutPanel( QWidget* parent = 0, Qt::WFlags fl = 0 , int npixcuts=-1, int nmcuts=-1, QString cutfile="/.cutpaths");
  ~CutPanel();

  int npcuts, nmcuts, ncuts;
  CutParam* GetCutPars(int id);
  QWidget *tab_2, *tab_3, *tab_4, *tab_5;

 public slots:
  void LoadClass();
  int  LoadClass(const char *path);
  void SaveClass(){SaveClass("");};
  void SaveClass(const char *inpath);
  void RenameClass();
  void NewClass();
  void DeleteClass();
  void selectedClass(Q3ListViewItem*);
  void Reset();
  void Save();
  void SaveClose();
  void hideCCTab(bool);

 private:
  CutWidget **m_cw;
  QString m_cutfile;
};

#endif  // end CUTPANEL_H
