#ifndef REFWIN_H
#define REFWIN_H

#include "ui_RefWin.h"

/*! panel for misc selection purposes
 * @authors Joern Grosse-Knetter <jgrosse1@physik.uni-goettingen.de>
 */

class RefWin : public QDialog, public Ui::RefWin
{
 Q_OBJECT

 public:
 RefWin( QWidget* parent = 0, Qt::WindowFlags = 0, const char *fname=0, const char *htype=0);
  ~RefWin();
  
  void addToList(const char *, int); 
  int  getSelVal();
  bool nextClicked(){return m_nextClicked;};

 public slots:
  void nextAccept(){m_nextClicked=true; accept();};
  void getModuleList();

 private:
  QString m_DBfname, m_grpName;
  bool m_nextClicked;
};

#endif // REFWIN_H
