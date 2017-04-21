#ifndef MODWIN_H
#define MODWIN_H

#include <ui_ModWin.h>

class ModWin : public QDialog, public Ui::ModWin {

      Q_OBJECT

 public:
  
  ModWin( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~ModWin();

};

#endif // MODWIN_H
