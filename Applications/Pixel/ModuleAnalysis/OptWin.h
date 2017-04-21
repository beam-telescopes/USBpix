#ifndef OPTWIN_H
#define OPTWIN_H

#include "ui_OptWinBase.h"
//Added by qt3to4:
#include <QLabel>

class TopWin;
class optClass;

class PathItem : public QWidget{
  
  Q_OBJECT

 public:
  PathItem(QLineEdit*, QLabel*, QPushButton*);
  ~PathItem();
  QLineEdit   *m_fname;
  QLabel      *m_label;
  QPushButton *m_button;
 public slots:
  virtual void GetFile();
};

/*! Displays the plotting/analysis options, ie is the
 *  visual interface to the optClass class.
 */

class OptWin : public QDialog, public Ui::OptWinBase
{
  Q_OBJECT
    
 public:
  OptWin( QWidget* parent = 0, const char* name = 0, bool modal=FALSE, Qt::WFlags fl = 0 );
  ~OptWin();

  optClass *m_Opts;

 public slots:
   void Reset_clicked();
   void Save_clicked();
   void Split_clicked(int);
   void FT_clicked(int);
   void SC_clicked();
   void FixSwitch_clicked();

 signals:
   void showCCclicked(bool);
};

#endif  // end OPTWIN_H
