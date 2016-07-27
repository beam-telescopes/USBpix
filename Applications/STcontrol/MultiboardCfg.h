#ifndef MULTIBOARDCFG
#define MULTIBOARDCFG

#include "ui_MultiboardCfg.h"
#include <vector>

class STControlEngine;
class QString;

class MultiboardCfg : public QDialog, public Ui::MultiboardCfg
{

 Q_OBJECT


 public:

struct cfgInfo{
  int boardID[2];
  std::vector<QString> newFeName;
  std::vector<QString> fileName;
  std::vector<QString> modDecName;
  int FEflavour;
  int FEnum;
  int adapterType;
};

  MultiboardCfg(STControlEngine &engine_in, QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~MultiboardCfg();

  void browseFeConfig(const char *path_in, int modId);
  std::vector<cfgInfo>& getCfgList(){return m_cfgList;};

 public slots:
  void updateDevList();
  void acceptCfg();
  void nextBoard();
  void browseFeConfig0(){browseFeConfig(0,0);}
  void browseFeConfig1(){browseFeConfig(0,1);}
  void browseFeConfig2(){browseFeConfig(0,2);}
  void browseFeConfig3(){browseFeConfig(0,3);}
  void browseFeConfig4(){browseFeConfig(0,4);}
  void browseFeConfig5(){browseFeConfig(0,5);}
  void browseFeConfig6(){browseFeConfig(0,6);}
  void browseDcsCfg();
  void browseuCFW();
  void browsefpgaFW();
  void setCfgBrws();
  void enaButtons();
  void set2FeMode();
  void selMod(int);
  void setSelMod(int);

 private:
  std::vector<cfgInfo> m_cfgList;
  STControlEngine &m_engine;
  QLineEdit* m_feName[4];
  QLineEdit* m_rootCfgFile[7];
  QComboBox* m_rootFeCfgName[7];
  QGroupBox* m_modGrp[4];
  QLabel*    m_modFileLabel[4];
  QLabel*    m_modCfgLabel[4];
  QPushButton* m_cfgBrowseButton[4];
};

#endif //MULTIBOARDCFG
