#ifndef RENAMEWIN_H
#define RENAMEWIN_H

#include <ui_RenameWin.h>

class RenameWin : public QDialog, public Ui::RenameWin {
 Q_OBJECT

 public:
  RenameWin( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
  ~RenameWin();

};

#endif // RENAMEWIN_H
