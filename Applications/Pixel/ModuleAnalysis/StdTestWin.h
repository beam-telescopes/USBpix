#ifndef STDTEST
#define STDTEST
#include "ui_StdTestWinBase.h"
#include <qevent.h>
#include <qstring.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QLabel>
#include <vector>

#define NTYPES 12 // old def., bare, pre-burnin, post-burnin (c+w), reception, pre-load, post-load, loaded receive
                  // special disk test, special bi-stave test, generic for system tests
class QLineEdit;
class QPushButton;
class QString;
class QWidget;
class FileWin;
class TopWin;
class ModItem;

/*! Item in the StdTestWin panel
 *
 */

class StdTestItem : public QWidget{
  
  Q_OBJECT

 public:
  StdTestItem(QWidget *parent, TopWin*, int, QLineEdit*, QLabel*, QPushButton*, 
	      QString defname="", bool *testtype=0);
  ~StdTestItem();
  QLineEdit   *m_fname;
  QLabel      *m_label;
  int          m_type;
  QPushButton *m_button;
  FileWin     *m_fwin;
  QString      m_defname;
  bool         m_testtp[NTYPES];
 public slots:
  void GetFile();
  void disable(const char *,const char *);
  void enable();
  void hide();
  void show();
};

/*!  File-selection window for a set of standard tests 
 *   (prelim.).
 */

class StdTestWin : public QDialog, public Ui::StdTestWinBase
{
  Q_OBJECT

 public:
  StdTestWin( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0, ModItem *item=0 );
  ~StdTestWin();
  std::vector<StdTestItem*> m_items;
  std::vector<ModItem*> Mit_arr;
  int NSLOTS;
  QString prefix[NTYPES];
 public slots:
  void GetDir();
  void ClearNames(){GenerateNames(true);};
  void GenerateNames(){GenerateNames(false);};
  void GenerateNames(bool empty);
  void TypeSet();
  void TabChanged(int);
 private:
  virtual void showEvent(QShowEvent*){TypeSet();};
  TopWin *m_parent;
  ModItem *m_modItem;
  bool didbrws;
};

#endif //STDTEST
