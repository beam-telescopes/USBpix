#ifndef MULTILOADRDB_H
#define MULTILOADRDB_H

#include "ui_MultiLoadRDBBase.h"
#include <vector>
#include <map>
#include <string>

class QString;

class MultiLoadRDB : public QDialog, public Ui::MultiLoadRDBBase{

    Q_OBJECT

 public:
  MultiLoadRDB(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0, const char *path=0);
  ~MultiLoadRDB();

 public slots:
  void browseFile();
  void browsePlots();
  void loadFile();
  void fillCombos(bool);
  void fillCombos(bool, std::vector<int>);
  void sel(bool);
  void selAll(){sel(true);};
  void selNone(){sel(false);};
  void getMods(std::vector<std::string> &mods);
  void getScans(std::map<std::string, int> &scans);

 private:
  QString m_path;
};

#endif // MULTILOADRDB_H
