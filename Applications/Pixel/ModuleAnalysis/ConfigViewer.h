#ifndef CONFIGVIEWER_H

#include <qdialog.h>
#include <qpushbutton.h>

// special table view for any PixLib config
class QTabWidget;
namespace PixLib{
 class Config;
 class ConfGroup;
 class ConfMatrix;
}

class QTablePushButton : public QPushButton {

  Q_OBJECT
 public:
  QTablePushButton( PixLib::ConfMatrix &confObj, QWidget * parent, const char * name = 0 );
  ~QTablePushButton();

 public slots:
  void showContent();

 private:
  PixLib::ConfMatrix &m_obj;
};

class matrixViewer : public QDialog {

  Q_OBJECT
 public:
  matrixViewer(PixLib::ConfMatrix &in_cfg, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, 
	       Qt::WFlags fl = 0);
  ~matrixViewer();

 public slots:

 private:
  PixLib::ConfMatrix &m_config;

};

class configViewer : public QDialog {

  Q_OBJECT
 public:
  configViewer(PixLib::Config &in_cfg, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, 
	       Qt::WFlags fl = 0);
  ~configViewer();

  QTabWidget *m_tabwidget;
  QPushButton *m_saveB;

 public slots:
  void fillTab(QWidget *tab, PixLib::ConfGroup &cgrp);
  void save(){save(0,0);};
  void save(const char *file, const char *label);

 private:
  PixLib::Config &m_config;
  void loadSubConf(PixLib::Config &inConf);

};

#endif // CONFIGVIEWER_H 
