#include "DBTreeView.h"
#include "DBEdtEngine.h"
#include "DBTreeItem.h"

#include <PixConfDBInterface/PixConfDBInterface.h>

#include <QComboBox>
#include <QMessageBox>
#include <QApplication>
#include <QEvent>
#include <QLayout>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QTimer>

#include <string>

DBListView::DBListView(QWidget* parent)
  : QTreeWidget(parent)
{
  m_leftPressed=false;
  m_inTimer=false;
  setAcceptDrops(true);
}
void DBListView::dragEnterEvent(QDragEnterEvent *event){
  if (event->mimeData()->hasFormat("application/x-dbtreeitem") && !m_leftPressed) {
    // must accept even nothing happens right away, otherwise no further processing
    event->accept();
  } else {
    event->ignore();
  }
}
void DBListView::dragMoveEvent(QDragMoveEvent *event){
  if (event->mimeData()->hasFormat("application/x-dbtreeitem") && !m_leftPressed) {
    DBTreeItem *target = dynamic_cast<DBTreeItem*>(itemAt(event->pos()));
    // only accept if drop is possible: decides on cursor type
    if(target!=0 && target->isInquire()){
      event->accept();
    } else {
      event->ignore();
    }
  } else {
    event->ignore();
  }
}
void DBListView::dropEvent(QDropEvent *event){
  if (event->mimeData()->hasFormat("application/x-dbtreeitem") && !m_leftPressed) {
    DBTreeItem *target = dynamic_cast<DBTreeItem*>(itemAt(event->pos()));
    if(target!=0 && target->isInquire()){ // can only move or copy into DB records/inquires
      const QMimeData *mime = event->mimeData();
      QByteArray itemData = mime->data("application/x-dbtreeitem");
      qlonglong pseudoPtr = itemData.toLongLong();
      DBTreeItem *item = (DBTreeItem*)pseudoPtr;
      QMessageBox question(this);
      question.setWindowTitle("move or copy?");
      question.setText("Move or copy item "+item->text(0)+" to "+target->text(0)+"?");
      question.addButton("Move",QMessageBox::YesRole);
      question.addButton("Copy",QMessageBox::NoRole);
      question.exec();
      if(question.buttonRole(question.clickedButton())==QMessageBox::YesRole){
	event->setDropAction(Qt::MoveAction);
	event->accept();
      } else {
	event->acceptProposedAction();
      }
      if(item->isInquire()){
	target->addDBInquire(item);
      }else if(item->isField()){
	target->addDBField(item);
      }else{
	QMessageBox ::warning(this,"DBeditor DBListView::dropEvent","copy/move object is of unknown type.");
      }
      ((DBTreeView*)parent())->updateList();
    } else {
      event->ignore();
    }
  } else {
    event->ignore();
  }
}
void DBListView::mouseReleaseEvent(QMouseEvent *event){
  m_relFlag = true;
  QTreeWidget::mouseReleaseEvent(event);
}
void DBListView::mouseDoubleClickEvent(QMouseEvent *event){
  m_dcFlag = true;
  DBTreeItem* clickItem = 0;
  if(currentItem()!=0 && currentItem()->isSelected())
    clickItem = dynamic_cast<DBTreeItem*>(currentItem());
  if (clickItem==0) return;
  clickItem->edit();
  QTreeWidget::mouseDoubleClickEvent(event);
}
void DBListView::mousePressEvent(QMouseEvent *event){
  // first, process regular data handling
  m_dcFlag = false;
  m_relFlag = false;
  QTreeWidget::mousePressEvent(event);
  // wait for possible double click or release, then proceed (and block 2nd execution while waiting)
  if(event->button()==Qt::LeftButton && event->type()==QEvent::MouseButtonPress && !m_inTimer) {
    m_inTimer = true;
    QTimer::singleShot((QApplication::doubleClickInterval()+5), this, SLOT(prepareDrag()));
  }
}
void DBListView::prepareDrag(){
  m_inTimer = false;
  // register object for drag-and-drop
  if(!m_dcFlag && !m_relFlag){
    m_leftPressed=true;
    DBTreeItem* dragItem = 0;
    if(currentItem()!=0 && currentItem()->isSelected())
      dragItem = dynamic_cast<DBTreeItem*>(currentItem());
    if (dragItem==0) return;
    QByteArray itemData;
    // dirty hack, but OK for a start
    qlonglong pseudoPts = (qlonglong)dragItem;
    itemData.setNum(pseudoPts);
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dbtreeitem", itemData);
    mimeData->setText(dragItem->text(0));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    //    drag->setHotSpot(hotSpot);
    if(drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction)==Qt::MoveAction){
      dragItem->deleteDBObject();
      ((DBTreeView*)parent())->updateList();
    }
    // reset state after drag-process finished
    m_leftPressed=false;
  }
  m_dcFlag = false;
  m_relFlag = false;
}

DBTreeView::DBTreeView(DBEdtEngine &in_engine, QWidget* parent, Qt::WindowFlags fl)
  : QWidget(parent,fl), m_engine(in_engine)
{
  setupUi(this);
  QObject::connect(fileList, SIGNAL(activated(QString)), this, SLOT(fillView()));
  
  QVBoxLayout *DBTreeViewBaseLayout = new QVBoxLayout(this); 
  
  QHBoxLayout *layout1 = new QHBoxLayout(0); 
  layout1->addWidget( textLabel1 );
  layout1->addWidget( fileList );
  
  QVBoxLayout *layout2 = new QVBoxLayout(0); 
  layout2->addLayout( layout1 );
  
  m_listView = new DBListView(this);
  m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_listView->setColumnCount(2);
  QStringList colLabels;
  colLabels << "DBInquires/Fields";
  colLabels << "Type";
  m_listView->setHeaderLabels(colLabels);
  m_listView->setColumnWidth(0,300);
  m_listView->setColumnWidth(1,100);
  layout2->addWidget( m_listView );
  DBTreeViewBaseLayout->addLayout( layout2 );
}
DBTreeView::~DBTreeView()
{
}
void DBTreeView::updateList()
{
  QString currFile = fileList->currentText();
  fileList->clear();
  int oldInd=-1;
  int i=0;
  for(std::vector<std::string>::iterator it=m_engine.beginDBFiles();
      it!=m_engine.endDBFiles();it++,i++){
    fileList->addItem((*it).c_str());
    if(QString((*it).c_str())==currFile) oldInd=i;
  }
  if(oldInd>=0) fileList->setCurrentIndex(oldInd);
  fillView();
  return;
}
void DBTreeView::fillView()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_listView->clear();

  if(fileList->count()==0){
    QApplication::restoreOverrideCursor();
    return; // nothing to do
  }

  PixLib::PixConfDBInterface *file = m_engine.openFile(fileList->currentText().toLatin1().data());
  PixLib::DBInquire *root = file->readRootRecord(1);
  DBTreeItem *rootItem = new DBTreeItem(m_listView, root, fileList->currentText().toLatin1().data());
  delete file; // close file, not needed any longer
  rootItem->setExpanded(true);

  QApplication::restoreOverrideCursor();
  return;
}
