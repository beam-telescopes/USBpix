#ifndef CFGFORM_H
#define CFGFORM_H

#include "ui_CfgFormBase.h"
#include <vector>
//Added by qt3to4:
#include <QLabel>
#include <Q3VBoxLayout>

class QWidget;
class QString;
class QLineEdit;
class QPushButton;
class QLabel;
class Q3VBoxLayout;

class CfgFormItem : public QWidget{
  Q_OBJECT

 public:
  CfgFormItem(QWidget* parent = 0, const char* name = 0, const char *label="new item", 
	      const char *filter="*.cfg");
  ~CfgFormItem();

  QString *m_Filter;
  QLabel *m_Label;
  QLineEdit *m_FileName;
  QPushButton *m_Button;
  Q3VBoxLayout *m_Layout;

 public slots:
  void browseFiles();
};

class CfgForm : public QDialog, public Ui::CfgFormBase{
  Q_OBJECT

 public:
  CfgForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~CfgForm();

  std::vector<CfgFormItem*> m_items;
};

#endif // CFGFORM_H
