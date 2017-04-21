#include "ConfigViewer.h"
#include "RenameWin.h"

#include <TMath.h>
#include <TSystem.h>
#include <TKey.h>

#include <PixConfDBInterface.h>
#include <RootDB.h>
#include <Histo.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <Config/ConfMask.h>
#include <PixScan.h>

#include <qvariant.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <q3table.h>
#include <qstring.h>
#include <q3filedialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

configViewer::configViewer(PixLib::Config &in_cfg, QWidget* parent, const char* name, 
			   bool modal, Qt::WFlags fl) : 
  QDialog(parent, name, modal, fl), m_config(in_cfg){
    if ( !name )
      setName( "configViewer" );
    setCaption("configuration viewer");

    Q3VBoxLayout *BaseLayout = new Q3VBoxLayout( this, 11, 6, "BaseLayout");
    Q3HBoxLayout *layout1 = new Q3HBoxLayout( 0, 0, 6, "layout1"); 
    Q3VBoxLayout *layout2 = new Q3VBoxLayout( 0, 0, 6, "layout2"); 

    m_tabwidget = new QTabWidget( this, "tabwidget" );
    layout2->addWidget(m_tabwidget);
    for (int i=0; i<m_config.size(); i++) {
      ConfGroup &grp = m_config[i];
      QWidget *TabPage = new QWidget( m_tabwidget, "TabPage"+QString::number(i) );
      fillTab(TabPage, grp);
      m_tabwidget->insertTab( TabPage,  grp.name().c_str());
    }
    loadSubConf(m_config);

    m_saveB = new QPushButton( this, "saveButton" );
    layout1->addWidget(m_saveB);
    m_saveB->setText("Save");
    QToolTip::add( m_saveB, "Save config to a RootDB config file" );

    QSpacerItem *spacer1 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer1 );

    QPushButton *closeB = new QPushButton( this, "closeButton" );
    layout1->addWidget(closeB);
    closeB->setText("Close");

//     QSpacerItem *spacer2 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
//     layout1->addItem( spacer2 );

    layout2->addLayout( layout1 );
    BaseLayout->addLayout( layout2 );

    resize( QSize(700, 400).expandedTo(minimumSizeHint()) );

    connect( closeB, SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( m_saveB, SIGNAL( clicked() ), this, SLOT( save() ) );
}
configViewer::~configViewer(){
}
void configViewer::fillTab(QWidget *tab, ConfGroup &cgrp){
  int i;

  Q3VBoxLayout *layout1 = new Q3VBoxLayout( tab, 0, 6, "mainlayout"); 

  Q3Table *table = new Q3Table( tab, QString("table_")+tab->name() );
  table->setNumCols(4);
  Q3Header *hdr = table->horizontalHeader();
  hdr->setLabel(0,"Name");
  hdr->setLabel(1,"Comment");
  hdr->setLabel(2,"Type");
  hdr->setLabel(3,"Value");
  table->setColumnWidth(0,150);
  table->setColumnWidth(1,250);
  layout1->addWidget(table);

  for (i=0; i<cgrp.size(); i++) {
    ConfObj &obj = cgrp[i];
    //    QHBoxLayout *layout2 = new QHBoxLayout( 0, 0, 6, "layout"+QString::number(i)); 
    QString ocmt  = obj.comment().c_str();
    QString oname = obj.name().c_str();
    int pos=oname.find("_");
    if(pos>=0)
      oname.remove(0,pos+1);
    int nRows = table->numRows();
    QString typeStr, valStr;
    switch( obj.type() ) {
    case ConfObj::INT : {
      typeStr = "INT"; 
      int ival=0;
      switch( ((ConfInt&)obj).subtype() ){
      case ConfInt::S32:
	ival = (int)((ConfInt&)obj).valueS32();
	break;
      case ConfInt::U32:
	ival = (int)((ConfInt&)obj).valueU32();
	break;
      case ConfInt::S16:
	ival = (int)((ConfInt&)obj).valueS16();
	break;
      case ConfInt::U16:
	ival = (int)((ConfInt&)obj).valueU16();
	break;
      case ConfInt::S8:
	ival = (int)((ConfInt&)obj).valueS8();
	break;
      case ConfInt::U8:
	ival = (int)((ConfInt&)obj).valueU8();
	break;
      default: ;
      }
      valStr  = QString::number(ival);
      break;}
    case ConfObj::FLOAT : 
      typeStr = "FLOAT";
      valStr  = QString::number(((ConfFloat&)obj).value(),'f',3);
      break;
    case ConfObj::LIST : 
      typeStr = "LIST"; 
      valStr  = ((ConfList&)obj).sValue().c_str();
      break;
    case ConfObj::BOOL : 
      typeStr = "BOOL";
      valStr  = ((ConfBool&)obj).value()?"true":"false";
      break;
    case ConfObj::STRING : 
      typeStr = "STRING";
      valStr  = ((ConfString&)obj).value().c_str();
      break;
    case ConfObj::MATRIX : 
      typeStr = "MATRIX ";
      switch(((ConfMatrix&)obj).subtype()){
      case ConfMatrix::U16:
	typeStr += "U16";
	break;
      case ConfMatrix::U1:
	typeStr += "U1";
	break;
      default:
	typeStr += "??";
      }
      valStr  = "unhandled";
      break;
    case ConfObj::VOID : 
      typeStr = "VOID"; 
      valStr  = "void";
      break;
    default: 
      typeStr = "Unrecognized";
    }
    table->setNumRows(nRows+1);
    table->setText(nRows,0,oname);
    table->setText(nRows,1,ocmt);
    if ( obj.type() != ConfObj::VECTOR &&  obj.type() != ConfObj::MATRIX) {
      table->setText(nRows,2,typeStr);
      table->setText(nRows,3,valStr);
    }else if(obj.type() == ConfObj::MATRIX) {
      QTablePushButton *tb = new QTablePushButton( (ConfMatrix&)obj, this, "showButt" );
      tb->setText("Show");
      table->setText(nRows,2,typeStr);
      table->setCellWidget(nRows,3, tb); 
    } else { //deal with VECTOR
      QStringList stringList;
      switch( ((ConfVector&)obj).subtype() ){
      case ConfVector::INT:{
	std::vector<int> &tmpVec = ((ConfVector&)obj).valueVInt();
	for(std::vector<int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
	  stringList << QString::number(*IT);
	break;}
      case ConfVector::UINT:{
	std::vector<unsigned int> &tmpVec = ((ConfVector&)obj).valueVUint();
	for(std::vector<unsigned int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
	  stringList << QString::number(*IT);
	break;}
      case ConfVector::FLOAT:{
	std::vector<float> &tmpVec = ((ConfVector&)obj).valueVFloat();
	for(std::vector<float>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
	  stringList << QString::number(*IT);
	break;}
      default:
	break;
      }
      Q3ComboTableItem *comboItem = new Q3ComboTableItem ( table, stringList );
      comboItem->setCurrentItem( 0 );
      table->setText(nRows,2,"VECTOR");
      table->setItem(nRows,3, comboItem); 
    }
  }
  QSpacerItem *spacer2 = new QSpacerItem( 21, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layout1->addItem( spacer2 );
}
void configViewer::save(const char *file, const char *label){
  QString fname, qlabel;
  if(label!=0)
    qlabel=label;
  else{
    RenameWin rnw(this," ", TRUE, Qt::WType_TopLevel);
    rnw.EnterLabel->setText("Scan label:");
    rnw.Name->validateAndSet("New scan config",16,0,16);
    rnw.Name->setFocus();
    rnw.exec();
    qlabel = rnw.Name->text();
  }
  bool go=(file!=0);
  if(!go){
    QStringList filter("RootDB cfg file (*.cfg.root)");
    filter += "Any file (*.*)";
    Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
    fdia.setFilters(filter);
    fdia.setCaption("Select RootDB file to save config to");
    go=(fdia.exec() == QDialog::Accepted);
    if(go)
      fname = fdia.selectedFile();
  } else
    fname = file;
  if(go){
    try{
      PixLib::RootDB outfile(fname.latin1(),"UPDATE");
      PixLib::DBInquire *root = outfile.readRootRecord(1);
      std::string decName = root->getDecName();
      decName += qlabel.latin1();
      //      printf("Adding inquire: %s\n",decName.c_str());
      PixLib::DBInquire *scanInq = outfile.makeInquire("PixScan", decName);
      root->pushRecord(scanInq);
      outfile.DBProcess(root,COMMITREPLACE);
      outfile.DBProcess(scanInq,COMMIT);
      m_config.write(scanInq);
    }catch(SctPixelRod::BaseException& exc){
      std::stringstream msg;
      msg << exc;
      QMessageBox::warning(this,"Module Analysis","Saving to RootDB file "+fname+
			   " failed with exception "+QString(msg.str().c_str())+".");
    }catch(...){
      QMessageBox::warning(this,"Module Analysis","Saving to RootDB file "+fname+" failed.");
    }
  }
}
void configViewer::loadSubConf(PixLib::Config &inConf)
{
  for(int j=0;j<inConf.subConfigSize();j++){
    for (int i=0; i<(inConf.subConfig(j)).size(); i++) {
      ConfGroup &grp = (inConf.subConfig(j))[i];
      QWidget *TabPage = new QWidget( m_tabwidget, "TabPage"+
				      QString::number(inConf.size()+i) );
      fillTab(TabPage, grp);
      m_tabwidget->insertTab( TabPage,(inConf.subConfig(j).name()+"-"+grp.name()).c_str());
    }
    loadSubConf(inConf.subConfig(j));
  }
}

QTablePushButton::QTablePushButton( PixLib::ConfMatrix &confObj, QWidget * parent, const char * name )
  : QPushButton(parent, name), m_obj(confObj)
{
  connect(this, SIGNAL(clicked()), this, SLOT(showContent()));
}
QTablePushButton::~QTablePushButton()
{
}
void QTablePushButton::showContent()
{
  matrixViewer *mv = new matrixViewer(m_obj, this, "matrixViewer", true);
  mv->exec();
  delete mv;
}
matrixViewer::matrixViewer(PixLib::ConfMatrix &in_cfg, QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, name, modal, fl), m_config(in_cfg)
{
  if ( !name )
    setName( "matrixViewer" );
  setCaption("ConfMatrix viewer");
  
  Q3VBoxLayout *BaseLayout = new Q3VBoxLayout( this, 11, 6, "BaseLayout");
  Q3HBoxLayout *layout1 = new Q3HBoxLayout( 0, 0, 6, "layout1"); 
  Q3VBoxLayout *layout2 = new Q3VBoxLayout( 0, 0, 6, "layout2"); 
  Q3Table *table = new Q3Table( this, "table" );
  layout2->addWidget(table);

  QSpacerItem *spacer1 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout1->addItem( spacer1 );
  QPushButton *closeB = new QPushButton( this, "closeButton" );
  layout1->addWidget(closeB);
  closeB->setText("Close");
  QSpacerItem *spacer2 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout1->addItem( spacer2 );

  layout2->addLayout( layout1 );
  BaseLayout->addLayout( layout2 );

  resize( QSize(700, 400).expandedTo(minimumSizeHint()) );
  connect( closeB, SIGNAL( clicked() ), this, SLOT( close() ) );

  // file table
  int sizec, sizer=0;
  switch(m_config.subtype()){
  case ConfMatrix::U16:{
    std::vector< std::vector<unsigned short int> > mask = ((PixLib::ConfMask<unsigned short int> &) m_config.valueU16()).get();
    sizec = mask.size();
    if(sizec>0)
      sizer = mask[0].size();
    table->setNumCols(sizec);
    table->setNumRows(sizer);
    for(int i=0;i<sizec;i++){
      table->setColumnWidth(i,50);
      for(int j=0;j<sizer;j++){
	table->setText(j,i,QString::number(mask[i][j]));
      }
    }
    break;}
  case ConfMatrix::U1:{
    std::vector< std::vector<bool> > mask = ((PixLib::ConfMask<bool> &) m_config.valueU1()).get();
    sizec = mask.size();
    if(sizec>0)
      sizer = mask[0].size();
    table->setNumCols(sizec);
    table->setNumRows(sizer);
    for(int i=0;i<sizec;i++){
      table->setColumnWidth(i,50);
      for(int j=0;j<sizer;j++){
	table->setText(j,i,mask[i][j]?"ON":"OFF");
      }
    }
    break;}
  default:
    return;
  }

}
matrixViewer::~matrixViewer()
{
}

