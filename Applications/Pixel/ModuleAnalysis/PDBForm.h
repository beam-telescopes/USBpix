#ifndef PDBFORM
#define PDBFORM

#include <qdir.h>

#include "ui_PDBFormBase.h"
#include "TopWin.h"
#include "ModTree.h"

class QWidget;
class QAReport;
class QApplication;
class QString;

class PDBForm : public QDialog, public Ui::PDBFormBase
{
 Q_OBJECT

 friend class TopWin;

 public:

  PDBForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 , 
	   ModItem *item = 0, QApplication *app=0, const char *defpath = 0 );
  ~PDBForm();
  int retValue;
  TopWin *m_parent;
  QString m_path;
  bool m_uploadJava;
  bool m_uploadPASS;
  bool m_cutVersioned;
  bool runJavaUpload(const char*, const char*, const char*,const char*, bool fileIsLast=false);
  // variables for PDB upload
  int m_allBadPixs, m_digBadPixs, m_srcBadPixs, m_srcMasked, m_anaBadPixs, m_discBadPixs, m_mlBadPixs;
  int m_xtBadPixs, m_intBadPixs, m_totBadPixs;
  float m_NTC, m_IV_T;
  void setMechForm(int isLoad);

 protected:
  QAReport *m_rep;

  QString m_User, m_Passwd;
  
  ModItem *IVitem;
  ModItem *DIGitem;
  ModItem *Curritem;
  ModItem *ANAitem;
  ModItem *SRCitem;
  ModItem *AMBitem;

 public slots:

  void CreatePDB();
  void LoadProfile();
  void SaveProfile();
  void Browse();
  void BrowseOutput();
  void checkType();
  int  IcheckType();
  void Index();
  void LoadRootPath();
  void enableUploadButt();
  void checkMech();
  void saveTable(const char *);
  const char* getModName();
  void setWWW();

 private:
  
  ModItem *m_modItem;
  QString m_javaPath;
  QDir dir;
  QString urlpath;
  QString modsn;
  QString modtype;
  int m_selcutID;
  int m_uploadWeb;
  bool m_uploadMech, m_pauseAfterWWW;
  QApplication *m_app;
  
  void CreateIV(QAReport *);
  void CreateDIG(QAReport *);
  void CreateANA(QAReport *);
  void CreateSRC(QAReport *);
  void CreateMECH();
  void UserReset();
  static QString Date(QString date = QString::null);

 protected slots:
  void launchJavaUpload(const char *in_User, const char *in_PWD);

 private slots:
  void launchJavaUpload(){launchJavaUpload(0,0);};
  void miscUpload();
  void getRootPath();

};

#endif // PDBFORM
