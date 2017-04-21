//Added by qt3to4:
#include <Q3CString>
#ifndef CDATABASE_H
#define CDATABASE_H

class Q3Http;
class Q3HttpHeader;
class Q3UrlOperator;
class QApplication;
class QFile;

#include <qobject.h>
#include <vector>
#include <string>
#include <fstream>

class CDatabaseCom : public QObject{
  Q_OBJECT

public:

  CDatabaseCom(std::string user, std::string password, std::string SN);
  ~CDatabaseCom();

  Q3UrlOperator *m_database;

  std::string m_encoded_password;

  std::vector <std::string> m_htmlfile;
  Q3Http *m_qhttp;
  Q3HttpHeader *m_requestheader;
  int m_id;
  bool m_complete;
  std::string m_SN;

public slots:
  void Read_Incoming(int,bool);
  void Request_Started(int id);
  void GetHtml();
  void GetHtmlAss(bool parent=true);
  std::vector<std::string> getStaveSNs(QApplication &app);
  void getParent(QApplication &app, std::string &parentSN, int &assPos);
  void Write_Socket(std::string command);

signals:
  void DownloadComplete(const char*);
};

class RDBManager  : public QObject{
  Q_OBJECT

 public:
   RDBManager(QApplication *app);
   ~RDBManager();

   int getRootFile(QString modSN, QString measType, QString outPath, bool retainName=true);
   std::vector<std::string> getRootUrls(QString modSN, QString measType);
   void getRootFromUrl(QString input_url, QString outPath);

 private:
    Q3Http *m_qhttp;
    QString m_html_line;
    QApplication *m_app;
    QFile *m_fout;

 private slots:
    void readRawData();
    void procRawData();
    void getRawData(QString url, QString host, QString path=QString::null);

};


typedef char HASH[16];
typedef char HASHHEX[33];

#define KBase64 KCodecs

class QString;
class Q3CString;

class KCodecs
{
public:

  static Q3CString base64Encode( const QByteArray& in, bool insertLFs = false);
  static Q3CString base64Encode( const Q3CString& str, bool insertLFs = false );
  static void base64Encode( const QByteArray& in, QByteArray& out,
			    bool insertLFs = false );
  static Q3CString base64Decode( const QByteArray& in );
  static Q3CString base64Decode( const Q3CString& str );
  static void base64Decode( const QByteArray& in, QByteArray& out );

private:
  KCodecs();
  static const char Base64EncMap[64];
  static const char Base64DecMap[128];

};

#endif  //CDATABASE_H
