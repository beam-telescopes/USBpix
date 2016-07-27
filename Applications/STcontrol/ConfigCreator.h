#ifndef CONFIG_CREATOR_H
#define CONFIG_CREATOR_H

#include "ui_ConfigCreator.h"
#include <vector>
#include <map>

namespace PixLib{
  class PixModuleGroup;
}
class QString;

class ConfigCreator : public QDialog, public Ui::ConfigCreator {

  Q_OBJECT

 public:
  ConfigCreator(QWidget *parent = 0, Qt::WindowFlags f = 0 );
  ~ConfigCreator();

  std::vector<PixLib::PixModuleGroup*> getCfg(){return m_pmg;};
  std::vector<int> getMakeRegs(){return m_makeRegs;};

  public slots:
    void createCfg(bool);
    void createCfgDone(){createCfg(true);};
    void createCfgNext(){createCfg(false);};
    void mnameListSetup(int nMod);
    void preset();
    void cfgLoadSelect(int type);
    void browseConfigFile();
    void setFromFile(int modIt);
    void setDbInfo(int modListRow);
	void selectFileDisp(int);
	void setFecombMax(int val);
	void ctrlTypeSelected(const QString & text);

 private:
    std::vector<PixLib::PixModuleGroup*> m_pmg;
    std::vector<int> m_makeRegs;
    std::map<int, std::vector<QString> > m_dbFnames;
    std::map<int, std::vector<QString> > m_dbMnames;
};


#endif // CONFIG_CREATOR_H
