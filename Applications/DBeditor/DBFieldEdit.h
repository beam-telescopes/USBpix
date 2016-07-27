#ifndef DBFIELDEDIT_H
#define DBFIELDEDIT_H

#include "ui_DBFieldEdit.h"

namespace PixLib{
  class DBField;
  class DBInquire;
}

class DBFieldEdit : public QDialog, public Ui::DBFieldEdit
{

  Q_OBJECT

 public:
  // constructor for creating a new field/inquire
  DBFieldEdit( PixLib::DBInquire *in_inquire, bool newField, QWidget* parent = 0, Qt::WindowFlags f = 0 );
  // constructor for editing an existing field
  DBFieldEdit( PixLib::DBField *in_field, QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~DBFieldEdit();

 public slots:
  void save();
  void showHex(bool);
  void addField();
  void addInq();
  void setTableNrows(int);

 private:
  PixLib::DBField *m_field;
  PixLib::DBInquire *m_inquire;

};

#endif // DBFIELDEDIT_H
