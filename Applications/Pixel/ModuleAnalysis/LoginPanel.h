#ifndef LOGINPANEL_H
#define LOGINPANEL_H

#include "ui_LoginPanelBase.h"

class LoginPanel : public QDialog, public Ui::LoginPanelBase {
    Q_OBJECT

 public:
  
  LoginPanel( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~LoginPanel();

};

#endif // LOGINPANEL_H
