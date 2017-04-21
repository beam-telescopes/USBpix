#include <qapplication.h>
#include "LogDisp.h"
#include "LogMain.h"
#include "FileTypes.h"
#include <qfiledialog.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qlabel.h>

LogMain::LogMain( QWidget* parent, const char* name, bool modal, WFlags fl):
  LogMainBase(parent,name,modal,fl){
  FileList->clear();
  path = "";
  QString verlabel = "based on ModuleAnalysis version ";
  verlabel += VERSION;
  VersionLabel->setText(verlabel);
}

LogMain::~LogMain(){
}

void LogMain::LoadDir(){

  path = QFileDialog::getExistingDirectory(0,this,"Logfile Display",
					      "select logfile dir",FALSE);
  QString fltr="*.logfile";
  QDir fdir(path,fltr,QDir::Name|QDir::IgnoreCase, QDir::Files);
  QStringList flist;
  flist = fdir.entryList();
  FileList->clear();
  for(QStringList::Iterator it = flist.begin(); it != flist.end(); ++it){
    FileList->insertItem((*it).latin1());
  }
  return;
}

void LogMain::ShowLog(QListBoxItem* item){
  TLogFile *tmpptr = 0;
  LogDisp disp(this,"Logfile Display",TRUE,0,tmpptr);
  QString fname;
  fname = path + item->text().latin1();
  disp.LoadFile(fname);
  disp.FileLabel->setText(item->text());
  disp.exec();
  return;
}

int main( int argc, char** argv )
{
  // start root and QT application
  QApplication app( argc, argv );

  // launch main GUI window
  LogMain* Win = new LogMain(NULL,"TurboDAQ logfile display");
  app.setMainWidget(Win);    
  Win->show();
  float xpos = QApplication::desktop()->width()-Win->width()-10;
  Win->move((int)xpos,10);
  
  int ret = app.exec();  
  delete Win;
  return ret;
}
