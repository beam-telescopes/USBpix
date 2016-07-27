#include "MainPanel.h"
#include "DBEdtEngine.h"
#include "DBTreeView.h"
#include "DBTreeItem.h"
#include "GeneralDBfunctions.h"
#include "RenameWin.h"

#include <BaseException.h>

#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QLayout>
#include <QApplication>
#include <QAction>
#include <QPoint>
#include <QSplitter> 
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTreeWidget>
#include <QTableWidget>
#include <QMouseEvent>

#include <string>
#include <sstream>
#ifndef WIN32
#include <unistd.h>
#endif

MainPanel::MainPanel(DBEdtEngine &in_engine, QWidget* parent, Qt::WindowFlags fl)
  : QMainWindow(parent,fl), m_engine(in_engine)
{
  setupUi(this);
  QObject::connect(fileNewAction, SIGNAL(triggered()), this, SLOT(fileNew()));
  QObject::connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
  QObject::connect(fileCloseAction, SIGNAL(triggered()), this, SLOT(fileClose()));
  QObject::connect(fileExitAction, SIGNAL(triggered()), this, SLOT(close()));
  QObject::connect(editCutAction, SIGNAL(triggered()), this, SLOT(editCut()));
  QObject::connect(editCopyAction, SIGNAL(triggered()), this, SLOT(editCopy()));
  QObject::connect(editPasteAction, SIGNAL(triggered()), this, SLOT(editPaste()));
  QObject::connect(editPasteTreeAction, SIGNAL(triggered()), this, SLOT(editPasteTreeAsis()));
  QObject::connect(editPasteRenameTreeAction, SIGNAL(triggered()), this, SLOT(editPasteTreeRename()));
  QObject::connect(viewButton, SIGNAL(toggled(bool)), this, SLOT(selectView(bool)));
  QObject::connect(editAdd_DBFieldAction, SIGNAL(triggered()), this, SLOT(addDBField()));
  QObject::connect(editAdd_DBInquireAction, SIGNAL(triggered()), this, SLOT(addDBInquire()));
  QObject::connect(editDelete_DB_objectAction, SIGNAL(triggered()), this, SLOT(deleteDBObject()));

  m_currentItem = 0;
  m_copyItem = 0;
  m_cutItem = 0;
  m_split = new QSplitter( this );
  verticalLayout->addWidget(m_split);
  m_leftTreeView = new DBTreeView(m_engine, m_split);
  m_rightTreeView = new DBTreeView(m_engine, m_split);
  connect(this,SIGNAL(fileListChanged()),m_leftTreeView,SLOT(updateList()));
  connect(this,SIGNAL(fileListChanged()),m_rightTreeView,SLOT(updateList()));
  connect(this,SIGNAL(contentChanged()),m_leftTreeView,SLOT(fillView()));
  connect(this,SIGNAL(contentChanged()),m_rightTreeView,SLOT(fillView()));
  connect(m_leftTreeView->m_listView,SIGNAL(itemClicked(QTreeWidgetItem*, int)),
	  this,SLOT(leftSelection(QTreeWidgetItem *)));
  connect(m_rightTreeView->m_listView,SIGNAL(itemClicked(QTreeWidgetItem*, int)),
	  this,SLOT(rightSelection(QTreeWidgetItem *)));
  connect(m_leftTreeView->m_listView,SIGNAL(customContextMenuRequested (const QPoint &)),
	  this,SLOT(openEditMenu(const QPoint &)));
  connect(m_rightTreeView->m_listView,SIGNAL(customContextMenuRequested (const QPoint &)),
	  this,SLOT(openEditMenu(const QPoint &)));

  showSingle();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  emit fileListChanged();
  clearSelection();
  QApplication::restoreOverrideCursor();
}

MainPanel::~MainPanel()
{
  fileClose();
}
void MainPanel::fileOpen()
{
  fileAction(0);
}
void MainPanel::fileNew()
{
  fileAction(1);
}
void MainPanel::fileAction(int actionType)
{
  if(actionType<0 || actionType>2){
    QMessageBox::warning(this,"DBeditor MainPanel::fileAction",
			 "Wrong action type: "+QString::number(actionType)+
			 ". Can only handle these types:\n0: open\n1:create new\n1:create new config");
    return;
  }

  QString qpath = QString::null;
  QStringList filter;
  filter += "RootDB file (*.root)";
  filter += "DB ascii-config file (*.cfg)";
  filter += "Any file (*.*)";
  QFileDialog fdia(0,"Specify name of DB file",qpath);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  switch(actionType){
  default:
  case 0:
    fdia.setFileMode(QFileDialog::ExistingFile);
    break;
  case 1:
    fdia.setFileMode(QFileDialog::AnyFile);
    break;
  }
  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().at(0);
    qpath.replace("\\", "/");
    std::stringstream msg;
    try{
      switch(actionType){
      case 1:
	QApplication::setOverrideCursor(Qt::WaitCursor);
	m_engine.createFile(qpath.toLatin1().data());
	QApplication::restoreOverrideCursor();
	// do not break, must call addFile from case 0, too
      default:
      case 0:
	QApplication::setOverrideCursor(Qt::WaitCursor);
	m_engine.addFile(qpath.toLatin1().data());
	QApplication::restoreOverrideCursor();
	break;
      }
    } catch(SctPixelRod::BaseException& b){
      msg << "\"";
      msg << b;
      msg << "\"";
    }catch(...){
      msg << "Unknown exception ";
    }
    if(msg.str()!=""){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"DBeditor MainPanel::fileAction",
			   QString((msg.str()+ "caught on file ").c_str())+
			   qpath);
      return;
    }
  }
  QApplication::setOverrideCursor(Qt::WaitCursor);
  emit fileListChanged();
  clearSelection();
  QApplication::restoreOverrideCursor();
  return;
}
void MainPanel::fileClose()
{
  std::stringstream msg;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  try{
    m_engine.clearFiles();
  } catch(SctPixelRod::BaseException& b){
    msg << "\"";
    msg << b;
    msg << "\"";
  }catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,"DBeditor MainPanel::fileClose",
			 QString((msg.str()+ " caught while closing all files.").c_str()));
    return;
  }
  emit fileListChanged();
  clearSelection();
  QApplication::restoreOverrideCursor();
  return;
}
void MainPanel::showSingle()
{
   m_leftTreeView->show();
   m_rightTreeView->hide();
}
void MainPanel::showDouble()
{
   m_leftTreeView->show();
   m_rightTreeView->show();
}
void MainPanel::selectView(bool both)
{
  if(both)
    showDouble();
  else
    showSingle();
}
void MainPanel::rightSelection(QTreeWidgetItem *item)
{
  changeCurrent(dynamic_cast<DBTreeItem*>(item));
  m_leftTreeView->m_listView->clearSelection();
}
void MainPanel::leftSelection(QTreeWidgetItem *item)
{
  changeCurrent(dynamic_cast<DBTreeItem*>(item));
  m_rightTreeView->m_listView->clearSelection();
}
void MainPanel::clearSelection()
{
  changeCurrent(0);
  m_rightTreeView->m_listView->clearSelection();
  m_leftTreeView->m_listView->clearSelection();
}
void MainPanel::changeCurrent(DBTreeItem *item)
{
  m_currentItem = item;
  // enable copy-paste functionality if appropriate
  editCutAction->setEnabled(m_currentItem!=0);
  editCopyAction->setEnabled(m_currentItem!=0);
  editPasteAction->setEnabled(((m_cutItem!=0 && m_cutItem!=m_currentItem && m_cutItem->isField()) || 
 			       (m_copyItem!=0 && m_copyItem!=m_currentItem)) && 
 			      m_currentItem!=0 && m_currentItem->isInquire());
  editPasteTreeAction->setEnabled(((m_cutItem!=0 && m_cutItem!=m_currentItem && m_cutItem->isInquire()) ||
				   (m_copyItem!=0 && m_copyItem!=m_currentItem && m_copyItem->isInquire())) &&
				  m_currentItem!=0 && m_currentItem->isInquire());
  editPasteRenameTreeAction->setEnabled(((m_cutItem!=0 && m_cutItem!=m_currentItem && m_cutItem->isInquire()) ||
				   (m_copyItem!=0 && m_copyItem!=m_currentItem && m_copyItem->isInquire())) &&
				  m_currentItem!=0 && m_currentItem->isInquire());
  // enable appropriate edit functions for inquires only
  editAdd_DBFieldAction->setEnabled(m_currentItem!=0 && m_currentItem->isInquire());
  editAdd_DBInquireAction->setEnabled(m_currentItem!=0 && m_currentItem->isInquire());
  editDelete_DB_objectAction->setEnabled(m_currentItem!=0);
}
void MainPanel::addDBField()
{
  if(m_currentItem==0){
    QMessageBox::warning(this,"DBeditor MainPanel::addDBInquire",
			 "Current DBTreeItem-pointer is NULL.");
    return;
  }

  std::stringstream msg;
  try{
    m_currentItem->addDBField();
  }catch(SctPixelRod::BaseException& b){
    msg << "\"";
    msg << b;
    msg << "\"";
  }catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    QMessageBox::warning(this,"DBeditor MainPanel::addDBField",
			 QString((msg.str()+ "caught while adding field.").c_str()));
    return;
  }
  emit contentChanged();
}
void MainPanel::addDBInquire()
{
  if(m_currentItem==0){
    QMessageBox::warning(this,"DBeditor MainPanel::addDBInquire",
			 "Current DBTreeItem-pointer is NULL.");
    return;
  }

  std::stringstream msg;
  try{
    m_currentItem->addDBInquire();
  }catch(SctPixelRod::BaseException& b){
    msg << "\"";
    msg << b;
    msg << "\"";
  }catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    QMessageBox::warning(this,"DBeditor MainPanel::addDBInquire",
			 QString((msg.str()+ "caught while adding inquire.").c_str()));
    return;
  }
  emit contentChanged();
}
void MainPanel::deleteDBObject()
{
  if(m_currentItem==0){
    QMessageBox::warning(this,"DBeditor MainPanel::deleteDBObject",
			 "Current DBTreeItem-pointer is NULL.");
    return;
  }

  std::stringstream msg;
  try{
    m_currentItem->deleteDBObject();
  }catch(SctPixelRod::BaseException& b){
    msg << "\"";
    msg << b;
    msg << "\"";
  }catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    QMessageBox::warning(this,"DBeditor MainPanel::deleteDBObject",
			 QString((msg.str()+ "caught while adding inquire.").c_str()));
    return;
  }
  emit contentChanged();
}
void MainPanel::openEditMenu(const QPoint &)
{
  DBListView *treeView = dynamic_cast<DBListView*>(sender());
  if(treeView!=0){
    if(treeView==m_leftTreeView->m_listView)
      leftSelection((DBTreeItem*)treeView->currentItem());
    else
      rightSelection((DBTreeItem*)treeView->currentItem());
    editMenu->exec(QCursor::pos());
  }
}
void MainPanel::editCopy()
{
  m_copyItem = m_currentItem;
  m_cutItem = 0;
}
void MainPanel::editCut()
{
  m_copyItem = 0;
  m_cutItem = m_currentItem;
}
void MainPanel::editPaste()
{
  DBTreeItem *item = m_copyItem;
  if(item==0)
    item = m_cutItem;
  if(item==0 || m_currentItem==0) return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::stringstream msg;
  try{
    if(item->isInquire())
      m_currentItem->addDBInquireOnly(item);
    else if(item->isField())
      m_currentItem->addDBField(item);
    else{
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"DBeditor MainPanel::addDBInquire","copy/cut object is of unknown type.");
      return;
    }
    // this was actually a cut&paste, so delete copied item
    if(m_cutItem!=0)
      item->deleteDBObject();
  }catch(SctPixelRod::BaseException& b){
    msg << "\"";
    msg << b;
    msg << "\"";
  }catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,"DBeditor MainPanel::addDBInquire",
			 QString((msg.str()+ "caught while adding inquire.").c_str()));
    return;
  }

  QApplication::restoreOverrideCursor();
  emit contentChanged();

  if(m_cutItem!=0)
    editPasteAction->setEnabled(false);
  m_cutItem = 0;
}
void MainPanel::editPasteTree(bool rename)
{
  DBTreeItem *item = m_copyItem;
  if(item==0)
    item = m_cutItem;
  if(item==0 || m_currentItem==0) return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::stringstream msg;
  try{
    if(item->isInquire()){
      QString newName="";
      if(rename){
        RenameWin rnw(this);
	std::string dname = item->getDecName();
	int pos = dname.find_last_of("/");
	if(pos!=(int)std::string::npos) dname.erase(pos, dname.length()-pos);
	PixLib::getDecNameCore(dname);
	newName = dname.c_str();
	while(std::string(newName.toLatin1().data())==dname){
	  newName = (dname+"_copy").c_str();
	  rnw.Name->setText(newName);
	  rnw.Name->setCursorPosition(newName.length());
	  rnw.Name->setSelection(0, newName.length());
	  rnw.Name->setFocus();
	  QApplication::restoreOverrideCursor();
	  if(rnw.exec()) newName = rnw.Name->text();
	  if(std::string(newName.toLatin1().data())==dname)
	    QMessageBox::warning(this,"DBeditor MainPanel::editPasteTree","old and new names are identical, please try again");
	  QApplication::setOverrideCursor(Qt::WaitCursor);
        }
      }
      m_currentItem->addDBInquire(item,true,rename?newName.toLatin1().data():0);
      // this was actually a cut&paste, so delete copied item
      if(m_cutItem!=0)
	item->deleteDBObject();
    }else{
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"DBeditor MainPanel::editPasteTree","Can't tree-copy DBFIeld");
      return;
    }
  }catch(SctPixelRod::BaseException& b){
    msg << "\"";
    msg << b;
    msg << "\"";
  }catch(...){
    msg << "Unknown exception ";
  }
  if(msg.str()!=""){
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,"DBeditor MainPanel::addDBInquire",
			 QString((msg.str()+ "caught while adding inquire.").c_str()));
    return;
  }

  QApplication::restoreOverrideCursor();
  emit contentChanged();

  m_cutItem = 0;
}
