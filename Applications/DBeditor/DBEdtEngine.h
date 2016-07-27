#ifndef DBEDTENGINE_H
#define DBEDTENGINE_H

#include <vector>
#include <string>
#include "GeneralDBfunctions.h"

namespace PixLib{
  class PixConfDBInterface;
}
class QApplication;

class DBEdtEngine {

 public:
  DBEdtEngine(QApplication &app);
  ~DBEdtEngine();

  static PixLib::PixConfDBInterface* openFile(const char *name, bool write=false);
  static void createFile(const char *name);
  void createConfig(const char *fname, std::vector<PixLib::grpData> myGrpData);
  void addFile(const char *name);
  void clearFiles();
  std::vector<std::string>::iterator beginDBFiles(){return m_dbFiles.begin();};
  std::vector<std::string>::iterator endDBFiles(){return m_dbFiles.end();};
  QApplication& getApp(){return m_app;};

 private:
  std::vector<std::string> m_dbFiles;
  QApplication &m_app;
  std::string m_plPath;

};

#endif // DBEDTENGINE_H
