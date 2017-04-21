#ifndef FILEWIN_H
#define FILEWIN_H

#include "ui_FileWinBase.h"
#define NTYP 20

class QString;
class QStringList;

/*! File selection panel
 *
 */

class FileWin : public QDialog, public Ui::FileWinBase
{
 Q_OBJECT

 public:
  FileWin( QWidget* parent = 0, Qt::WFlags fl = 0);
  ~FileWin();
  int m_seltype;
  QStringList m_pathlist;
  QString GetFilter(int);

  int  AddType(QString label, int type);
  int  AddType(QString label, QString filter, int type);
  void SetDefDir(const char *path){m_DefDir = path;};
  int  GuessType(QString fname);  

 private:
  int m_types[NTYP];
  QString m_filter_types[NTYP], m_DefDir;

 public slots:
  virtual void GetFile();
  virtual void SetType(int);

};

#endif // FILEWIN_H

