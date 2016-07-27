#ifndef SCANITEM_H
#define SCANITEM_H

#include <QTreeWidgetItem>
#include <string>

namespace PixLib{
  class DBInquire;
}
class PixDBData;
class PlotFactory;

class ModuleAction : public QTreeWidgetItem {

 public:

  ModuleAction(QTreeWidgetItem* item, PixDBData &dbData, int type);
  ~ModuleAction();

  PixDBData& getData(){return m_dbData;};
  bool isHistoPlot();
  int getType(){return m_type;};

 private:
  PixDBData &m_dbData;
  int m_type;
};

class ModuleItem : public QTreeWidgetItem {

 public:

  ModuleItem(QTreeWidgetItem* item, PixLib::DBInquire *inquire, const char *fname, const char *modname);
  ~ModuleItem();

  PixDBData& getData(){return *m_dbData;};
  void loadContent();
  void reLoadContent();

 private:
  PixDBData *m_dbData;
};

class ScanItem : public QTreeWidgetItem {

 public:

  ScanItem(QTreeWidgetItem* item, PixLib::DBInquire *inquire, const char *fname);
  ScanItem(QTreeWidget* list, PixLib::DBInquire *inquire, const char *fname);

  ~ScanItem();

 private:
  void setupContent(PixLib::DBInquire *inquire);
  std::string m_decName, m_fname;

};

#endif // SCANITEM_H
