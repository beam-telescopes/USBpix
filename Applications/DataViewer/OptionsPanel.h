#ifndef OPTIONSPANEL_H
#define OPTIONSPANEL_H

#include <QDialog>
#include <string>
#include <map>

class QTabWidget;
class QWidget;
class QObject;
class QPushButton;
namespace PixLib{
 class Config;
 class ConfGroup;
}

class fileBrwsManager : public QObject{
  Q_OBJECT

 public:
  fileBrwsManager(QWidget *parent, int type, QWidget *textField);
  ~fileBrwsManager();

 public slots:
  void browse();

 private:
  int m_type;
  QWidget *m_textField, *m_wparent;
};

class optionsPanel : public QDialog{

  Q_OBJECT

 public:
  optionsPanel( PixLib::Config &in_cfg, QWidget* parent = 0, Qt::WindowFlags fl = 0 , 
		bool subGrps = false, bool embed=false, bool useScrollbar=false,
		bool noEdit = false);
  ~optionsPanel();

  int  fillTab(QWidget *, PixLib::ConfGroup &, const char *subcfg_name=0);
  void loadSubConf(PixLib::Config &inConf);

  QTabWidget *m_tabwidget;
  std::map<std::string, QWidget*> m_handles;
  PixLib::Config &m_config;
  QPushButton *cancB;
  QPushButton *saveB;
  std::map<std::string, QPushButton*> getMatrixEdit(){return m_matrixEdit;};

 public slots:
  void save();
  void cancel(){close();};
  void setTableNrows(int nrow);

 signals:
  void saveDone();

 private:
  bool m_embed, m_useScrollbar, m_noEdit;
  std::map<std::string, QPushButton*> m_matrixEdit;

};

#endif //  OPTIONSPANEL_H
