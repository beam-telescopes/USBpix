#ifndef NEW_DCS_DIALOG_H
#define NEW_DCS_DIALOG_H

#include "ui_NewDcsDialog.h"
#include <vector>
#include <string>

class NewDcsDialog : public QDialog, public Ui::NewDcsDialog{

  Q_OBJECT

 public:
  NewDcsDialog(std::vector<std::string> grpNames, QWidget* parent = 0, Qt::WindowFlags fl=0);
  ~NewDcsDialog();

  public slots:
  void showCtrlBox(int);

};

#endif // NEW_DCS_DIALOG_H
