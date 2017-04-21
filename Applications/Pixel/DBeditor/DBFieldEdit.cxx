#include "DBFieldEdit.h"

#include <PixConfDBInterface/PixConfDBInterface.h>
#include <Histo/Histo.h>

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QCheckBox>
#include <QComboBox>
#include <QApplication>
#include <QSpinBox>
#include <QTableWidget>
#include <QPlainTextEdit>

#include <string>

DBFieldEdit::DBFieldEdit( PixLib::DBInquire *in_inquire, bool newField, QWidget* parent, Qt::WindowFlags f )
  : QDialog(parent,f), m_field(0), m_inquire(in_inquire)
{
  setupUi(this);
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  hexBox->setEnabled(false);
  hexBox->hide();
  fieldTable->hide();
  fieldTxtEdit->hide();
  tableEntries->hide();
  if(newField){
    fieldLabel->setText("Enter field value:   ");
    typeLabel->setText("Select field type:   ");
    for(int i=0;i<11;i++)
      typeBox->addItem(PixLib::DBDataTypeNames[i], QVariant(i));
    
    connect( okButton, SIGNAL( clicked() ), this, SLOT( addField() ) );
    connect( fieldValue, SIGNAL( returnPressed() ), this, SLOT( addField() ) );
  } else {
    fieldLabel->setText("Enter inquire dec.name:   ");
    enterLabel->setText("Enter inquire name:   ");
    typeBox->hide();
    typeLabel->hide();
    connect( okButton, SIGNAL( clicked() ), this, SLOT( addInq() ) );
    connect( fieldValue, SIGNAL( returnPressed() ), this, SLOT( addInq() ) );
  }
}
DBFieldEdit::DBFieldEdit( PixLib::DBField *in_field, QWidget* parent, Qt::WindowFlags f )
  : QDialog(parent,f), m_field(in_field), m_inquire(0)
{
  setupUi(this);
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(tableEntries, SIGNAL(valueChanged(int)), this, SLOT(setTableNrows(int)));

  hexBox->setEnabled(false);
  hexBox->hide();
  typeBox->hide();
  fieldName->hide();
  enterLabel->hide();
  if(m_field!=0){
    fieldLabel->setText(m_field->getName().c_str());
    PixLib::PixConfDBInterface *myDB = m_field->getDB();
    if(myDB==0){
	typeLabel->setText("pointer to DB file is NULL - can't edit");
	okButton->setEnabled(false);
    } else{
      typeLabel->setText("is of type "+QString(PixLib::DBDataTypeNames[m_field->getDataType()]));
      switch(m_field->getDataType()){
      case PixLib::BOOL:{
	bool dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	fieldValue->setText(dbval?"true":"false");
	fieldTable->hide();
	fieldTxtEdit->hide();
	tableEntries->hide();
	break;
      }
      case PixLib::INT:{
	int dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	fieldValue->setText(QString::number(dbval));
	fieldTable->hide();
	fieldTxtEdit->hide();
	tableEntries->hide();
	break;
      }
      case PixLib::ULINT:{
	unsigned int dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	fieldValue->setText(QString::number(dbval,16));
	hexBox->setEnabled(true);
	hexBox->show();
	fieldTxtEdit->hide();
	fieldTable->hide();
	tableEntries->hide();
	break;
      }
      case PixLib::FLOAT:{
	float dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	fieldValue->setText(QString::number(dbval,'f',6));
	fieldTable->hide();
	fieldTxtEdit->hide();
	tableEntries->hide();
	break;
      }
      case PixLib::DOUBLE:{
	double dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	fieldValue->setText(QString::number(dbval,'g',6));
	fieldTable->hide();
	fieldTxtEdit->hide();
	tableEntries->hide();
	break;
      }
      case PixLib::STRING:{
	std::string dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	fieldTxtEdit->setPlainText(QString(dbval.c_str()));
	fieldTable->hide();
	fieldValue->hide();
	tableEntries->hide();
	break;
      }
      case PixLib::VECTORBOOL:{
	std::vector<bool> dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	int nrow=0;
	tableEntries->setValue((int)dbval.size());
	for(std::vector<bool>::iterator IT=dbval.begin(); IT!=dbval.end(); IT++){
	  fieldTable->item(nrow,0)->setText((*IT)?"true":"false");
	  nrow++;
	}
	fieldTxtEdit->hide();
	fieldValue->hide();
	break;
      }
      case PixLib::VECTORINT:{
	std::vector<int> dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	int nrow=0;
	tableEntries->setValue(dbval.size());
	for(std::vector<int>::iterator IT=dbval.begin(); IT!=dbval.end(); IT++){
	  fieldTable->item(nrow,0)->setText(QString::number(*IT));
	  nrow++;
	}
	fieldTxtEdit->hide();
	fieldValue->hide();
	break;
      }
      case PixLib::VECTORFLOAT:{
	std::vector<float> dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	int nrow=0;
	tableEntries->setValue(dbval.size());
	for(std::vector<float>::iterator IT=dbval.begin(); IT!=dbval.end(); IT++){
	  fieldTable->item(nrow,0)->setText(QString::number(*IT,'g',3));
	  nrow++;
	}
	fieldTxtEdit->hide();
	fieldValue->hide();
	break;
      }
      case PixLib::VECTORDOUBLE:{
	std::vector<double> dbval;
	myDB->DBProcess(m_field,PixLib::READ,dbval);
	int nrow=0;
	tableEntries->setValue(dbval.size());
	for(std::vector<double>::iterator IT=dbval.begin(); IT!=dbval.end(); IT++){
	  fieldTable->item(nrow,0)->setText(QString::number(*IT,'g',6));
	  nrow++;
	}
	fieldTxtEdit->hide();
	fieldValue->hide();
	break;
      }
      case PixLib::HISTO:{
	fieldTxtEdit->hide();
	fieldTable->hide();
	fieldValue->hide();
	tableEntries->hide();
	PixLib::Histo hi;
	myDB->DBProcess(m_field,PixLib::READ,hi);
	typeLabel->setText("type Histo is unhandled; histogram \""+QString(hi.title().c_str())+"\" has "+
			   QString::number(hi.nBin(0))+" bins in x"+
			   ((hi.nDim()>1)?(" and "+QString::number(hi.nBin(1))+" bins in y"):""));
	okButton->setEnabled(false);
	break;
      }
      default:
	fieldTxtEdit->hide();
	fieldTable->hide();
	fieldValue->hide();
	tableEntries->hide();
	typeLabel->setText("type "+QString(PixLib::DBDataTypeNames[m_field->getDataType()])+
			   " is unhandled/unknown");
	okButton->setEnabled(false);
      }
    }
  }
  connect( okButton, SIGNAL( clicked() ), this, SLOT( save() ) );
  connect( fieldValue, SIGNAL( returnPressed() ), this, SLOT( save() ) );
  connect( hexBox, SIGNAL( toggled(bool) ), this, SLOT( showHex(bool) ) );
}
DBFieldEdit::~DBFieldEdit()
{
}
void DBFieldEdit::save()
{
  if(m_field==0) reject();
  PixLib::PixConfDBInterface *myDB = m_field->getDB();
  if(myDB==0) reject();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  switch(m_field->getDataType()){
  case PixLib::BOOL:{
    bool dbval=(fieldValue->text()=="true");
    myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::INT:{
    bool isOK;
    int dbval = fieldValue->text().toInt(&isOK);
    if(isOK)
      myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::ULINT:{
    bool isOK;
    int base = 10;
    if(hexBox->isChecked() ) 
      base = 16;
    unsigned int dbval = fieldValue->text().toUInt(&isOK,base);
    if(isOK)
      myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::FLOAT:{
    bool isOK;
    float dbval = fieldValue->text().toFloat(&isOK);
    if(isOK)
      myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::DOUBLE:{
    bool isOK;
    double dbval = fieldValue->text().toDouble(&isOK);
    if(isOK)
      myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::STRING:{
    std::string dbval = fieldTxtEdit->toPlainText().toLatin1().data();
    myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORBOOL:{
    std::vector<bool> dbval;
    for(int i=0;i<tableEntries->value();i++)
      dbval.push_back(fieldTable->item(i,0)->text().toLower()=="true");
		      myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORINT:{
    std::vector<int> dbval;
    for(int i=0;i<tableEntries->value();i++)
      dbval.push_back(fieldTable->item(i,0)->text().toInt());
    myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORFLOAT:{
    std::vector<float> dbval;
    for(int i=0;i<tableEntries->value();i++)
      dbval.push_back(fieldTable->item(i,0)->text().toFloat());
    myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORDOUBLE:{
    std::vector<double> dbval;
    for(int i=0;i<tableEntries->value();i++)
      dbval.push_back(fieldTable->item(i,0)->text().toDouble());
    myDB->DBProcess(m_field,PixLib::COMMIT,dbval);
    break;
  }
  default:
    QApplication::restoreOverrideCursor();
    reject();
  }

  QApplication::restoreOverrideCursor();
  accept();
}
void DBFieldEdit::showHex(bool doShow)
{
  bool isOK;
  int oldBase = 16, newBase=10;
  if(doShow){
    oldBase = 10;
    newBase = 16;
  }
  unsigned int dbval = fieldValue->text().toUInt(&isOK, oldBase);
  if(isOK){
    fieldValue->setText(QString::number(dbval,newBase));    
  }
}
void DBFieldEdit::addField()
{
  if(m_inquire==0)
  {
    reject();
    return;
  }
  PixLib::PixConfDBInterface *myDB = m_inquire->getDB();
  if(myDB==0)
  {
    reject();
    return;
  }

  // user must enter value and name
  if(fieldName->text().isEmpty() || fieldValue->text().isEmpty()) reject();

  // create empty field
  QApplication::setOverrideCursor(Qt::WaitCursor);
  PixLib::DBField *new_field = myDB->makeField(fieldName->text().toLatin1().data());

  // fill field
  switch(typeBox->itemData(typeBox->currentIndex()).toInt()){
  case PixLib::BOOL:{
    bool dbval=(fieldValue->text()=="true");
    myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::INT:{
    bool isOK;
    int dbval = fieldValue->text().toInt(&isOK);
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::ULINT:{
    bool isOK;
    int base = 10;
    if(hexBox->isChecked() ) 
      base = 16;
    unsigned int dbval = fieldValue->text().toUInt(&isOK,base);
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::FLOAT:{
    bool isOK;
    float dbval = fieldValue->text().toFloat(&isOK);
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::DOUBLE:{
    bool isOK;
    double dbval = fieldValue->text().toDouble(&isOK);
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORBOOL:{
    std::vector<bool> dbval;
    dbval.push_back((fieldValue->text()=="true"));
    myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORINT:{
    bool isOK;
    std::vector<int> dbval;
    dbval.push_back(fieldValue->text().toInt(&isOK));
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORFLOAT:{
    bool isOK;
    std::vector<float> dbval;
    dbval.push_back(fieldValue->text().toFloat(&isOK));
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::VECTORDOUBLE:{
    bool isOK;
    std::vector<double> dbval;
    dbval.push_back(fieldValue->text().toDouble(&isOK));
    if(isOK)
      myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  case PixLib::STRING:{
    std::string dbval = fieldValue->text().toLatin1().data();
    myDB->DBProcess(new_field,PixLib::COMMIT,dbval);
    break;
  }
  default:
    QApplication::restoreOverrideCursor();
    delete new_field;
    reject();
    return;
  }

  m_inquire->pushField(new_field);
  QApplication::restoreOverrideCursor();
  accept();
}
void DBFieldEdit::addInq()
{
  if(m_inquire==0) reject();
  PixLib::PixConfDBInterface *myDB = m_inquire->getDB();
  if(myDB==0) reject();

  // user must enter value and name
  if(fieldName->text().isEmpty() || fieldValue->text().isEmpty()) reject();

  // create inquire
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString decName = m_inquire->getDecName().c_str();
  decName += fieldValue->text();
  PixLib::DBInquire *new_inq = myDB->makeInquire(fieldName->text().toLatin1().data(),decName.toLatin1().data());
  m_inquire->pushRecord(new_inq);
  myDB->DBProcess(m_inquire,PixLib::COMMITREPLACE);
  myDB->DBProcess(new_inq,PixLib::COMMIT);

  QApplication::restoreOverrideCursor();
  accept();
}
void DBFieldEdit::setTableNrows(int nrow){
  int oldNrow = fieldTable->rowCount();
  for(int i=nrow;i<oldNrow;i++)
    fieldTable->removeRow(i);
  for(int i=oldNrow;i<nrow;i++){
    fieldTable->insertRow(i);
    fieldTable->setItem(i,0, new QTableWidgetItem(QString("")));
  }
}
