#ifndef DBTREEITEM_H
#define DBTREEITEM_H

#include <QTreeWidgetItem>
#include <QString>
#include <string>
#include <vector>

namespace PixLib{
  class DBInquire;
  class DBField;
  class PixConfDBInterface;
}

class DBTreeItem : public QTreeWidgetItem
{


 public:
  enum ChipTestType {none=0, GR=1, PR=2, SC=4, DCdata=8};

  DBTreeItem(DBTreeItem* item, PixLib::DBInquire *inquire, const char *fname, bool showTree=true, bool readOnly=false);
  DBTreeItem(QTreeWidgetItem* item, PixLib::DBField *field, const char *fname, ChipTestType type=none, bool readOnly=false);
  DBTreeItem(QTreeWidget *list, PixLib::DBInquire *inquire, const char *fname, bool showTree=true, bool readOnly=false);
  DBTreeItem(QTreeWidgetItem *item, PixLib::DBInquire *inquire, const char *fname, bool showTree=true, bool readOnly=false);
  DBTreeItem(QTreeWidget *list, PixLib::DBField *field, const char *fname, bool readOnly=false);
  ~DBTreeItem();

  void edit();

  bool isField(){return m_isField;};
  bool isInquire(){return m_isInquire;};

  void addDBField(DBTreeItem *in_item);
  void addDBField(){addDBField(0);};
  void addDBInquire(DBTreeItem *in_item, bool tree=true, const char *newName=0);
  void addDBInquire(){addDBInquire(0);};
  void addDBInquireOnly(DBTreeItem *in_item){addDBInquire(in_item,false);};
  void deleteDBObject();
  void closeFile();
  PixLib::DBInquire* getInquire();
  PixLib::DBField* getField();
  std::string getFname(){return m_fname;};
  std::string getDecName(){return m_decName;};
  void processPRData(std::vector<unsigned long int> sentReg, std::vector<unsigned long int> readReg, QString label);
  void setWritable(){m_readOnly=false;};

 private:
  bool m_isInquire;
  bool m_isField;
  std::string m_decName, m_fname;
  PixLib::PixConfDBInterface *m_file;
  ChipTestType m_CTtype;
  bool m_readOnly;
};

#endif // DBTREEITEM_H
