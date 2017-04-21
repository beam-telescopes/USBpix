#ifndef STDLISTCHOICE
#define STDLISTCHOICE
#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>
#include "StdTestWin.h"
#include <vector>

class QCheckBox;
class QLabel;
class TopWin;
class Q3ScrollView;

class StdListChoice : public QDialog{
  
  Q_OBJECT

 public:
  StdListChoice(QWidget *parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0, TopWin *tw = 0);
  ~StdListChoice();

  std::vector<QCheckBox*> m_checkBoxes;

 public slots:
    // type: 0 = user, 1 = BARE, 2 = ASSY etc
    void setType(int type=2);
    void checkAll(){setAll(true);};
    void uncheckAll(){setAll(false);};
    void setAll(bool);

 private:
  StdTestWin *m_stdwin;
  QLabel *m_Heading;
  Q3ScrollView *m_scrlv;
};
#endif // STDLISTCHOICE
