#ifndef CONFIG_CREATOR_H
#define CONFIG_CREATOR_H

#include "ui_ConfigCreator.h"
#include <vector>

namespace PixLib{
  class PixModuleGroup;
}

class ConfigCreator : public QDialog, public Ui::ConfigCreator {

  Q_OBJECT

 public:
  ConfigCreator(QWidget *parent = 0, Qt::WindowFlags f = 0 );
  ~ConfigCreator();

  std::vector<PixLib::PixModuleGroup*> getCfg(){return m_pmg;};

  public slots:
    void createCfg(bool);
    void createCfgDone(){createCfg(true);};
    void createCfgNext(){createCfg(false);};
    void mnameListSetup(int nMod);
    void preset();

 private:
    std::vector<PixLib::PixModuleGroup*> m_pmg;

};


#endif // CONFIG_CREATOR_H
