#include "DBTreeItem.h"
#include "DBFieldEdit.h"
#include "DBEdtEngine.h"
#include "GeneralDBfunctions.h"
#include "GRView.h"

#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/RootDB.h>
#include <BaseException.h>

#include <qdialog.h>
#include <qstring.h>
#include <qapplication.h>
#include <QStringList>
#include <qmessagebox.h>

#include <TCanvas.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TH2F.h>

#include <string>
#include <iostream>

#define Ncols 80
#define Nrows 336

DBTreeItem::DBTreeItem(DBTreeItem* item, PixLib::DBInquire *inquire, const char *fname, bool showTree, bool readOnly)
  : QTreeWidgetItem((QTreeWidgetItem*) item), m_fname(fname), m_file(0), m_CTtype(none), m_readOnly(readOnly)
{
  m_isInquire = true;
  m_isField = false;
  QString label = inquire->getName().c_str();
  std::string decName = inquire->getDecName();
  m_decName = decName+inquire->getName();
  PixLib::getDecNameCore(decName);
  if(decName!=""){
    label += " (";
    label += decName.c_str();
    label += ")";
  }
  setText(0,label);
  setText(1,"Inquire");

  // create childs for the depending inquires
  if(showTree){
    for(PixLib::recordIterator rit=inquire->recordBegin(); 
	rit!=inquire->recordEnd();rit++)
      new DBTreeItem(this,(*rit), m_fname.c_str(), showTree,  m_readOnly);
  }

  // create childs for the depending fields
  for(PixLib::fieldIterator fit=inquire->fieldBegin(); 
      fit!=inquire->fieldEnd();fit++)
    new DBTreeItem(this,(*fit), m_fname.c_str(), none, m_readOnly);
}
DBTreeItem::DBTreeItem(QTreeWidget *list, PixLib::DBInquire *inquire, const char *fname, bool showTree, bool readOnly)
  : QTreeWidgetItem(list), m_fname(fname), m_file(0), m_CTtype(none), m_readOnly(readOnly)
{
  m_isInquire = true;
  m_isField = false;
  QString label = inquire->getName().c_str();
  std::string decName = inquire->getDecName();
  m_decName = decName+inquire->getName();
  PixLib::getDecNameCore(decName);
  if(decName!=""){
    label += " (";
    label += decName.c_str();
    label += ")";
  }
  setText(0,label);
  setText(1,"Inquire");

  // create childs for the depending inquires
  if(showTree){
    for(PixLib::recordIterator rit=inquire->recordBegin(); 
	rit!=inquire->recordEnd();rit++)
      new DBTreeItem(this,(*rit), m_fname.c_str(), showTree,  m_readOnly);
  }

  // create childs for the depending fields
  for(PixLib::fieldIterator fit=inquire->fieldBegin(); 
      fit!=inquire->fieldEnd();fit++)
    new DBTreeItem(this,(*fit), m_fname.c_str(), none, m_readOnly);


}
DBTreeItem::DBTreeItem(QTreeWidgetItem *item, PixLib::DBInquire *inquire, const char *fname, bool showTree, bool readOnly)
  : QTreeWidgetItem(item), m_fname(fname), m_file(0), m_CTtype(none), m_readOnly(readOnly)
{
  m_isInquire = true;
  m_isField = false;
  QString label = inquire->getName().c_str();
  std::string decName = inquire->getDecName();
  m_decName = decName+inquire->getName();
  PixLib::getDecNameCore(decName);
  if(decName!=""){
    label += " (";
    label += decName.c_str();
    label += ")";
  }
  setText(0,label);
  setText(1,"Inquire");

  // create childs for the depending inquires
  if(showTree){
    for(PixLib::recordIterator rit=inquire->recordBegin(); 
	rit!=inquire->recordEnd();rit++)
      new DBTreeItem(this,(*rit), m_fname.c_str(), showTree,  m_readOnly);
  }

  // create childs for the depending fields
  for(PixLib::fieldIterator fit=inquire->fieldBegin(); 
      fit!=inquire->fieldEnd();fit++)
    new DBTreeItem(this,(*fit), m_fname.c_str(), none, m_readOnly);


}
DBTreeItem::DBTreeItem(QTreeWidgetItem* item, PixLib::DBField *field, const char *fname, ChipTestType type, bool readOnly)
  : QTreeWidgetItem(item), m_fname(fname), m_file(0), m_CTtype(type), m_readOnly(readOnly)
{
  m_isInquire = false;
  m_isField = true;
  m_decName = field->getDecName();
  setText(0,field->getName().c_str());
  setText(1,"Field");
}
DBTreeItem::DBTreeItem(QTreeWidget *list, PixLib::DBField *field, const char *fname, bool readOnly)
  : QTreeWidgetItem(list), m_fname(fname), m_file(0), m_CTtype(none), m_readOnly(readOnly)
{
  m_isInquire = false;
  m_isField = true;
  m_decName = field->getDecName();
  setText(0,field->getName().c_str());
  setText(1,"Field");
}
DBTreeItem::~DBTreeItem()
{
  delete m_file; m_file=0;
}
void DBTreeItem::edit()
{
  if(!m_isField) return; // can only edit fields

  // get parent's inquire name
  PixLib::DBInquire *inq = ((DBTreeItem*) parent())->getInquire();
  if(inq==0){
    ((DBTreeItem*) parent())->closeFile();
    return;
  }
  std::string inqName = inq->getDecName()+"/"+inq->getName();
  ((DBTreeItem*) parent())->closeFile();
  // temporarily, open DB file in rw mode
  delete m_file;
  m_file = DBEdtEngine::openFile(m_fname.c_str(), true);
  // find field and parent inquire in there
  inq = 0;
  if(inqName=="/rootRecord")
    inq = m_file->readRootRecord(1);
  else{
    std::vector<PixLib::DBInquire *> ti = m_file->DBFindRecordByName(PixLib::BYDECNAME, inqName);
    if(ti.size()==1) inq = ti[0];
  }
  std::vector<PixLib::DBField *> tf = m_file->DBFindFieldByName(PixLib::BYDECNAME, m_decName);
  if(tf.size()!=1 || inq==0){
    delete m_file; m_file = 0;
    throw SctPixelRod::BaseException("DBTreeItem::edit : can't find field "+m_decName+" or its parent inquire in file.");
  }
  //printf("CType %d\n", (int)m_CTtype);
  if(m_CTtype==none){
    DBFieldEdit editor(tf[0],treeWidget());
    if(m_readOnly){
      editor.okButton->hide();
      editor.cancelButton->setText("OK");
    }
    if(editor.exec()==QDialog::Accepted){
      QApplication::setOverrideCursor(Qt::WaitCursor);
      m_file->DBProcess(inq,PixLib::COMMITREPLACE);
      QApplication::restoreOverrideCursor();
    }
  } else if (m_CTtype&GR){
    std::string value;
    m_file->DBProcess(tf[0], READ, value);
    GRView gv(QString(value.c_str()), treeWidget());
    gv.exec();
  } else if (m_CTtype&SC){
    std::string value;
    m_file->DBProcess(tf[0], READ, value);
    GRView gv(QString(value.c_str()), treeWidget(),0,1);
    gv.exec();
  } else if (m_CTtype&PR){
    std::string value;
    m_file->DBProcess(tf[0], READ, value);
    QString text = value.c_str();
    QStringList list = text.split("\n", QString::SkipEmptyParts);
    bool measStart=false;
    QString lastLabel="";
    std::vector<unsigned long int> regSent, regRead;
    for(QStringList::iterator it=list.begin(); it!=list.end(); it++){
      if((*it).length()>3 && measStart){
	QStringList txitems = (*it).split("\t", QString::SkipEmptyParts);
	if(txitems.size()==2){
	  regSent.push_back(txitems.at(0).toULong(0,16));
	  regRead.push_back(txitems.at(1).toULong(0,16));
	}
	if((*it).left(2)=="DC" && txitems.size()==3){
	  regSent.push_back(txitems.at(1).toULong(0,16));
	  regRead.push_back(txitems.at(2).toULong(0,16));
	}
      }
      if((*it).left(14)=="Failed test of" || (*it).left(14)=="Passed test of"){
	if(lastLabel!=""){ // process previous data
	  measStart=false;
	  if(regSent.size()>0){
	    if(regSent.size()==(Ncols*Nrows/32))
	      processPRData(regSent, regRead, lastLabel);
	    else
	      QMessageBox::warning(0,"DBeditor DBTreeItem::edit",
			 "Found some data for register "+lastLabel+" but with too few entries for a PR: "
				   + QString::number(regSent.size()));

	  }
	  // clear vectors
	  regSent.clear();
	  regRead.clear();
	}
	lastLabel = (*it).mid(15,(*it).length()-16);
      }
      if((*it)=="Sent	Read") measStart = true;
    }
    if(lastLabel!=""){ // process last data
      measStart=false;
      if(regSent.size()==(Ncols*Nrows/32))
	processPRData(regSent, regRead, lastLabel);
      else
	QMessageBox::warning(0,"DBeditor DBTreeItem::edit",
			     "Found some data for register "+lastLabel+" but with too few entries for a PR.");
      // clear vectors
      regSent.clear();
      regRead.clear();
    }
  }

  // close temporarily opened file
  delete m_file; m_file = 0;
}
void DBTreeItem::addDBField(DBTreeItem *in_item)
{
  if(!m_isInquire)
    throw SctPixelRod::BaseException("DBTreeItem::addDBField : Item belongs to a field, can't add fields.");

  // temporarily, open DB file in rw mode
  delete m_file;
  m_file = DBEdtEngine::openFile(m_fname.c_str(), true);

  // find inquire to this item
  PixLib::DBInquire *inq = 0;
  if(m_decName=="/rootRecord")
    inq = m_file->readRootRecord(1);
  else{
    std::vector<PixLib::DBInquire *> ti = m_file->DBFindRecordByName(PixLib::BYDECNAME, m_decName);
    if(ti.size()!=1){
      delete m_file; m_file = 0;
      throw SctPixelRod::BaseException("DBTreeItem::edit : can't find inquire "+m_decName+" in file.");
    }
    inq = ti[0];
  }
  DBFieldEdit editor(inq, true, treeWidget());
  if(in_item==0){
    if(editor.exec()==QDialog::Accepted){
      QApplication::setOverrideCursor(Qt::WaitCursor);
      m_file->DBProcess(inq,PixLib::COMMITREPLACE);
      QApplication::restoreOverrideCursor();
    }
  } else if(in_item->isField()){
    PixLib::DBField *orgField = 0;
    if(in_item->getFname()!=m_fname){
      orgField = in_item->getField();
    } else{
      std::vector<PixLib::DBField *> tf2 = m_file->DBFindFieldByName(PixLib::BYDECNAME, in_item->getDecName());
      if(tf2.size()!=1){
	delete m_file; m_file = 0;
	throw SctPixelRod::BaseException("DBTreeItem::edit : can't find field "+in_item->getDecName()+" or its parent inquire in file.");
      }
      orgField = tf2[0];
    }
    PixLib::insertDBField(orgField,inq);
    if(in_item->getFname()!=m_fname)
      in_item->closeFile();
  }
  // close temporarily opened file
  delete m_file; m_file = 0;
}
void DBTreeItem::addDBInquire(DBTreeItem *in_item, bool tree, const char *newName)
{
  if(!m_isInquire)
    throw SctPixelRod::BaseException("DBTreeItem::addDBField : Item belongs to a field, can't add inquires.");

  // temporarily, open DB file in rw mode
  delete m_file;
  m_file = DBEdtEngine::openFile(m_fname.c_str(), true);

  // find inquire to this item
  PixLib::DBInquire *inq = 0;
  if(m_decName=="/rootRecord")
    inq = m_file->readRootRecord(1);
  else{
    std::vector<PixLib::DBInquire *> ti = m_file->DBFindRecordByName(PixLib::BYDECNAME, m_decName);
    if(ti.size()!=1){
      delete m_file; m_file = 0;
      throw SctPixelRod::BaseException("DBTreeItem::edit : can't find inquire "+m_decName+" in file.");
    }
    inq = ti[0];
  }

  DBFieldEdit editor(inq, false, treeWidget());
  if(in_item==0)
    editor.exec();
  else if(in_item->isInquire()){
    PixLib::DBInquire *orgInquire = 0;
    if(in_item->getFname()!=m_fname){
      orgInquire = in_item->getInquire();
    } else{
      std::vector<PixLib::DBInquire *> ti2 = m_file->DBFindRecordByName(PixLib::BYDECNAME, in_item->getDecName());
      if(ti2.size()!=1){
	delete m_file; m_file = 0;
	throw SctPixelRod::BaseException("DBTreeItem::edit : can't find field "+in_item->getDecName()+" or its parent inquire in file.");
      }
      orgInquire = ti2[0];
    }
    PixLib::insertDBInquire(orgInquire,inq, tree, newName);
    if(in_item->getFname()!=m_fname)
      in_item->closeFile();
  }
  // close temporarily opened file
  delete m_file; m_file = 0;
  return;
}
void DBTreeItem::deleteDBObject()
{
  if(parent()==0) return;

  if(m_isField){ // user wants to delete a field
    // get parent's inquire name
    PixLib::DBInquire *inq = ((DBTreeItem*) parent())->getInquire();
    if(inq==0){
      ((DBTreeItem*) parent())->closeFile();
      return;
    }
    std::string inqName = inq->getDecName()+"/"+inq->getName();
    ((DBTreeItem*) parent())->closeFile();
    // temporarily, open DB file in rw mode
    delete m_file;
    m_file = DBEdtEngine::openFile(m_fname.c_str(), true);
    // find field and parent inquire in there
    inq = 0;
    if(inqName=="/rootRecord")
      inq = m_file->readRootRecord(1);
    else{
      std::vector<PixLib::DBInquire *> ti = m_file->DBFindRecordByName(PixLib::BYDECNAME, inqName);
      if(ti.size()==1) inq = ti[0];
    }
    std::vector<PixLib::DBField *> tf = m_file->DBFindFieldByName(PixLib::BYDECNAME, m_decName);
    if(tf.size()!=1 || inq==0){
      delete m_file; m_file = 0;
      throw SctPixelRod::BaseException("DBTreeItem::deleteDBObject : can't find field "+m_decName+" or its parent inquire in file.");
    }
    inq->eraseField(tf[0]);
    m_file->DBProcess(inq,PixLib::COMMITREPLACE);
  }else{ // deleting an inquire - works only for RootDB!
    // get parent's inquire name
    PixLib::DBInquire *inq = ((DBTreeItem*) parent())->getInquire();
    if(inq==0){
      ((DBTreeItem*) parent())->closeFile();
      throw SctPixelRod::BaseException("DBTreeItem::deleteDBObject : can't find parent record of "+m_decName+" in file.");
      return;
    }
    std::string inqName = inq->getDecName()+"/"+inq->getName();
    ((DBTreeItem*) parent())->closeFile();
    // temporarily, open DB file in rw mode
    delete m_file;
    m_file = DBEdtEngine::openFile(m_fname.c_str(), true);
    // find field and parent inquire in there
    inq = 0;
    std::vector<PixLib::DBInquire *> mti;
    if(inqName=="/rootRecord")
      inq = m_file->readRootRecord(1);
    else{
      std::vector<PixLib::DBInquire *> ti = m_file->DBFindRecordByName(PixLib::BYDECNAME, inqName);
      if(ti.size()==1) inq = ti[0];
    }
    mti = m_file->DBFindRecordByName(PixLib::BYDECNAME, m_decName);
    if(inq==0){
      delete m_file; m_file = 0;
      throw SctPixelRod::BaseException("DBTreeItem::deleteDBObject : can't find parent record of "+m_decName+" in file.");
    }
    if(mti.size()!=1){
      delete m_file; m_file = 0;
      throw SctPixelRod::BaseException("DBTreeItem::deleteDBObject : can't find record "+m_decName+" in file.");
    }
    PixLib::RootDBInquire *rinq = dynamic_cast<PixLib::RootDBInquire*>(inq);
    if(rinq!=0){
      for(PixLib::recordIterator it = inq->recordBegin(); it!=inq->recordEnd(); it++){
	if(mti[0]==(*it)){
	  rinq->eraseRecord(it);
	  break;
	}
      }
      m_file->DBProcess(inq,PixLib::COMMITREPLACE);
    } else
      throw SctPixelRod::BaseException("DBTreeItem::deleteDBObject : Inquire is not a RootDBInquire, can't delete.");
  }
  // close temporarily opened file
  delete m_file; m_file = 0;
}
PixLib::DBInquire* DBTreeItem::getInquire()
{
  if(m_file==0)
    m_file = DBEdtEngine::openFile(m_fname.c_str());
  PixLib::DBInquire *inq = 0;
  if(m_decName=="/rootRecord")
    inq = m_file->readRootRecord(1);
  else{
    std::vector<PixLib::DBInquire *> ti = m_file->DBFindRecordByName(PixLib::BYDECNAME, m_decName);
    if(ti.size()==1) inq = ti[0];
  }
  return inq;
}
PixLib::DBField* DBTreeItem::getField()
{
  if(m_file==0)
   m_file = DBEdtEngine::openFile(m_fname.c_str());
  std::vector<PixLib::DBField *> tf = m_file->DBFindFieldByName(PixLib::BYDECNAME, m_decName);
  if(tf.size()==1) return tf[0];
  else             return 0;
}
void DBTreeItem::closeFile()
{
  delete m_file; 
  m_file = 0;
}
void DBTreeItem::processPRData(std::vector<unsigned long int> sentReg, std::vector<unsigned long int> readReg, QString label){
  //printf("Found %s with %d entries\n", label.latin1(), (int)sentReg.size());
  QString canName = "can"+label;
  TCanvas *can = (TCanvas*) gROOT->FindObject(canName.toLatin1().data());
  if(can==0) can = new TCanvas(canName.toLatin1().data(), canName.toLatin1().data(), 600, 600);
  can->Clear();
  QString hisName = "hi"+label;
  QString hisTitle = "Read back errors for pixel register "+label;
  gROOT->cd(); // make sure histo is created in memory
  TH2F *hi = (TH2F*) gROOT->FindObject(hisName.toLatin1().data());
  if(hi!=0) hi->Delete();
  hi = new TH2F(hisName.toLatin1().data(), hisTitle.toLatin1().data(), Ncols, -0.5, -0.5+(double)Ncols, Nrows, -0.5, -0.5+(double)Nrows);
  // fill histo from vectors
  for(int col=0; col < Ncols; col++){
    for(int row=0; row < Nrows; row++){
      // determine pixel index as used in dll
      int pixnr = 0;
      int pixint = 0;
      if (0x01 & col)
	pixnr = col * Nrows - row - 1;
      else
	pixnr = (col+1) * Nrows + row;
      pixint = (pixnr / 32);
      int k = 31-(pixnr%32);
      if(pixint<(int)sentReg.size() && pixint<(int)readReg.size()){
	bool agree = (sentReg[pixint]&(1<<k))==(readReg[pixint]&(1<<k));
	hi->SetBinContent(col+1,row+1,agree?0.:1.);
      }
    }
  }
  // draw histo to canvas
  can->cd();
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(.15f);
  gStyle->SetPadRightMargin(.05f);
  gStyle->SetPadBottomMargin(.15f);
  hi->SetMinimum(0.f);
  hi->SetMaximum(1.f);
  hi->Draw("COL"); 
  hi->GetXaxis()->SetTitle("column");
  hi->GetYaxis()->SetTitle("row");
  hi->GetYaxis()->SetTitleOffset(1.2f);
  can->Update();
}
