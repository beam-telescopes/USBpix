#include "FileWin.h"
#include "FileTypes.h"
#include "MAEngine.h"

#include <qstring.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <q3filedialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qstringlist.h>

#include <iostream>
#include <fstream>
#include <map>

FileWin::FileWin( QWidget* parent, Qt::WFlags fl) : QDialog( parent, fl )
{  
  setupUi(this);
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(FileType, SIGNAL(activated(int)), this, SLOT(SetType(int)));
  QObject::connect(BrwsButt, SIGNAL(clicked()), this, SLOT(GetFile()));
  QObject::connect(Comments, SIGNAL(returnPressed()), this, SLOT(accept()));

  FileType->insertItem( tr( "guess type from extension" ),0);
  m_types[0] = NONE;
  m_seltype = NONE;
  SetType(0);
  m_DefDir = QString::null;
  m_pathlist = QStringList();

  std::map<int,QString> labels, extensions;
  MAEngine::fileExtensions(labels, extensions);
  for(std::map<int,QString>::iterator it=labels.begin(); it!=labels.end(); it++)
    AddType(it->second, extensions[it->first], it->first);
}

FileWin::~FileWin()
{ /* NOOP */ }

void FileWin::GetFile(){
  QString path=QString::null,text;
  QStringList filter;

  if(m_seltype==NONE || m_seltype==MULTIF){
    //QMessageBox::warning(this,"Module Analysis","You must define a file type before browsing");
    //return;
    filter += "Any file (*.*)";
    for(int i=1;i<FileType->count();i++)
      filter += m_filter_types[i];
  } else{  
    filter += GetFilter(m_seltype);
    filter += "Any file (*.*)";
  }

  // open file dialog box
  
  Q3FileDialog fdia(m_DefDir, QString::null,this,"select data file",TRUE);
  path = QString::null;
  fdia.setFilters(filter);
  if(m_seltype==MULTIF) // multiple file mode
    fdia.setMode(Q3FileDialog::ExistingFiles);
  if(fdia.exec() == QDialog::Accepted){
    if(fdia.mode()!=Q3FileDialog::ExistingFiles)
      path = fdia.selectedFile();
    else{
      OKButt->setEnabled(fdia.selectedFiles().count()>0);
      m_pathlist.clear();
      m_pathlist = fdia.selectedFiles();
      path = "";
      for(int i=0;i<(int)fdia.selectedFiles().count();i++){
	path += fdia.selectedFiles()[i];
	path += ";";
      }
    }
  }else
    return;
  
  if(path.isEmpty()){
    FileName->setText("No file has been selected");
    return;
  } else{
    FileName->setText(path);
    if(m_seltype==NONE){ // user wants us to guess, let's see what we can do
      int type = GuessType(path);
      FileType->setCurrentItem(type);
      SetType(type);
    }
    if(m_seltype!=ROOTF && m_seltype!=MULTIF){
      Comments->validateAndSet("new data",8,0,8);
      Comments->setFocus();
    }
  }
  
  return;
}
void FileWin::SetType(int item){
  if(item<0 || item >=NTYP)
    m_seltype = NONE;
  else
    m_seltype = m_types[item];

  OKButt->setEnabled(m_seltype!=NONE);
  // hide comments box if root-file (comments are already in file)
  if(m_seltype==ROOTF || m_seltype==MULTIF){
    Comments->hide();
    ComLabel->hide();
  } else{
    Comments->show();
    ComLabel->show();
  }

  return;
}


int FileWin::AddType(QString label, int type){
  int newindex = FileType->count();
  if(newindex>=NTYP) return -1;
  FileType->insertItem(label,newindex);
  m_types[newindex] = type;
  m_filter_types[newindex] = QString();
  return 0;
}
int FileWin::AddType(QString label, QString filter, int type){
  int newindex = FileType->count();
  if(newindex>=NTYP) return -1;
  FileType->insertItem(label,newindex);
  m_types[newindex] = type;
  m_filter_types[newindex] = filter;
  return 0;
}

QString FileWin::GetFilter(int type){
  int i;
  for(i=0;i<NTYP;i++)
    if(m_types[i]==type) break;
  if(i==NTYP) return QString("");
  return m_filter_types[i];
}

int FileWin::GuessType(QString fname){
  QString ext;
  int i,pos, tdind, odind;
  for(i=0;i<NTYP;i++)
    if(m_types[i]==ONEDSCAN) break;
  odind = i;
  for(i=0;i<NTYP;i++)
    if(m_types[i]==TWODSCAN) break;
  tdind = i;
  for(i=1;i<FileType->count();i++){
    ext = m_filter_types[i];
    pos = ext.find("(*dacs*.out)"); //T/FDAC files: special case, since *.out is not unique...
    if(pos>=0){
      pos = fname.find(".out");
      if(pos>=0){
	pos = fname.find("_tdacs_");
	if(pos<0){
	  pos = fname.find("_fdacs_");
	  if(pos>=0) return i;
	}else
	  return i;
      }else
	pos = -1;
    } else{
      pos = ext.find("(*");
      if(pos>=0)
	ext.remove(0,pos+2);
      pos = ext.find(")");
      if(pos>=0)
	ext.remove(pos,ext.length());
      pos = fname.find(ext);
      if(pos>=0){
	if(i!=odind && i!=tdind)
	  return i;
	else{ // find out if it's 1D or 2D scan
	  int chip, col, row;
	  int scpt1, scpt2;
	  char c;
	  std::ifstream input(fname.latin1());
	  if(!input.is_open()) return odind;
	  input >> chip >> col >> row >> scpt1;
	  // go to end and step back to last line
	  for(i=3;i<500;i++){
	    input.seekg(-i,std::ios::end);
	    input.get(c);
	    if(c=='\n') break;
	  }
	  input >> chip >> col >> row >> scpt2;
	  input.close();
	  if(scpt1==scpt2) return odind;
	  else             return tdind;
	}
      }
    }
  }
  return 0;
}
