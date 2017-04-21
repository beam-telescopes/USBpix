#include "OptionsPanel.h"

#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <Config/ConfMask.h>

#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QLineEdit>
#include <QToolTip>
#include <QCheckBox>
#include <QComboBox>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayout>

#include <math.h>

fileBrwsManager::fileBrwsManager(QWidget *parent, int type, QWidget *textField) : QObject(parent){
  m_type = type;
  m_textField = textField;
  m_wparent = parent;
}
fileBrwsManager::~fileBrwsManager(){
}
void fileBrwsManager::browse(){
  QLineEdit *ledt = dynamic_cast<QLineEdit*>(m_textField);
  if(ledt==0) return;
  QString qpath = QString::null;
  if(m_type==2) qpath = ledt->text();
  QStringList filter;
  filter += "Any file (*.*)";
//  QFileDialog fdia(m_wparent,"Specify output folder for file",qpath);
  QFileDialog fdia(0,"Specify output folder for file",qpath);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  if(m_type==2)
    fdia.setFileMode(QFileDialog::Directory);
  else
    fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    QStringList files = fdia.selectedFiles();
    if (!files.isEmpty()){
      qpath = files[0];
      qpath.replace("\\", "/");
      if(m_type==3){
	int ind = qpath.lastIndexOf("/");
	if(ind>=0) qpath = qpath.right(qpath.length()-ind-1);
      }
      ledt->setText(qpath);
    }
  }
}

optionsPanel::optionsPanel( PixLib::Config &in_cfg, QWidget* parent, Qt::WindowFlags fl, 
			    bool subGrps, bool embed, bool useScrollbar, bool noEdit) :
  QDialog (parent, fl), m_config(in_cfg), m_embed(embed), m_useScrollbar(useScrollbar), m_noEdit(noEdit) {

    QVBoxLayout *BaseLayout = new QVBoxLayout(this);
    QVBoxLayout *layout2 = new QVBoxLayout(0); 

    m_tabwidget = new QTabWidget(this);
    layout2->addWidget(m_tabwidget);
    for (int i=0; i<m_config.size(); i++) {
      ConfGroup &grp = m_config[i];
      QWidget *tabFillBox = new QWidget(m_tabwidget);
      QVBoxLayout *verticalLayout = new QVBoxLayout(tabFillBox);
      verticalLayout->setSpacing(4);
      verticalLayout->setMargin(11);
      verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
      QScrollArea *scrollArea = 0;
      if(m_useScrollbar){
	scrollArea = new QScrollArea(tabFillBox);
	scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
	scrollArea->setWidgetResizable(true);
      }
      QWidget *TabPage = new QWidget();
      TabPage->setObjectName(QString::fromUtf8("TabPage"));
      TabPage->setGeometry(QRect(0, 0, 618, 976));
      int nitems = fillTab(TabPage, grp);
      if(nitems>0){
	if(m_useScrollbar){
	  scrollArea->setWidget(TabPage);
	  verticalLayout->addWidget(scrollArea);
	}else
	  verticalLayout->addWidget(TabPage);
	m_tabwidget->addTab( tabFillBox,  QString(grp.name().c_str()));
      } else
	delete tabFillBox;
    }
    if(subGrps)
      loadSubConf(m_config);

    if(!embed){
      QHBoxLayout *layout1 = new QHBoxLayout(0); 
      if(!m_noEdit){
	saveB = new QPushButton(this);
	layout1->addWidget(saveB);
	saveB->setText("Save");
      }

      QSpacerItem *spacer1 = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
      layout1->addItem( spacer1 );

      cancB = new QPushButton(this);
      layout1->addWidget(cancB);
      cancB->setText(m_noEdit?"Close":"Cancel");

      QSpacerItem *spacer2 = new QSpacerItem( 21, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
      layout1->addItem( spacer2 );
      
      layout2->addLayout( layout1 );

      if(!m_noEdit)
	connect( saveB, SIGNAL( clicked() ), this, SLOT( save() ) );
      connect( cancB, SIGNAL( clicked() ), this, SLOT( reject() ) );
    }
    BaseLayout->addLayout( layout2 );

    //resize( QSize(450, 250).expandedTo(minimumSizeHint()) );
    setMinimumSize(600,300);
}
optionsPanel::~optionsPanel(){
//   for(std::map<std::string,QWidget*>::iterator it=m_handles.begin(); it!=m_handles.end(); it++){
//   }
}
void optionsPanel::loadSubConf(PixLib::Config &inConf)
{
  for(int j=0;j<inConf.subConfigSize();j++){
    for (int i=0; i<(inConf.subConfig(j)).size(); i++) {
      ConfGroup &grp = (inConf.subConfig(j))[i];
      QWidget *tabFillBox = new QWidget( m_tabwidget );
      QVBoxLayout *verticalLayout = new QVBoxLayout(tabFillBox);
      verticalLayout->setSpacing(4);
      verticalLayout->setMargin(11);
      verticalLayout->setObjectName("verticalLayout"+QString::number(inConf.size()+i));
      QScrollArea *scrollArea = 0;
      if(m_useScrollbar){
	scrollArea = new QScrollArea(tabFillBox);
	scrollArea->setObjectName("scrollArea"+QString::number(inConf.size()+i));
	scrollArea->setWidgetResizable(true);
      }
      QWidget *TabPage = new QWidget();
      TabPage->setObjectName("TabPage"+ QString::number(inConf.size()+i));
      TabPage->setGeometry(QRect(0, 0, 618, 976));
      int nitems = fillTab(TabPage, grp, inConf.subConfig(j).name().c_str());
      if(nitems>0){
	if(m_useScrollbar){
	  scrollArea->setWidget(TabPage);
	  verticalLayout->addWidget(scrollArea);
	} else
	  verticalLayout->addWidget(TabPage);
	m_tabwidget->addTab( tabFillBox,QString((inConf.subConfig(j).name()+"-"+grp.name()).c_str()));
      }else
	delete tabFillBox;
    }
    loadSubConf(inConf.subConfig(j));
  }
}
void optionsPanel::save(){
  // save to cfg
  for(std::map<std::string,QWidget*>::iterator it=m_handles.begin(); it!=m_handles.end(); it++){
    if(it->first.substr(it->first.length()-4,4)!="_exp"){ // exclude exponent spin box from ConfObj::FLOAT
      std::string cfgN,grpN, objN;
      ConfObj *objptr;
      cfgN = it->first;
      grpN = it->first;
      objN = it->first;
      unsigned int pos = (unsigned int)grpN.find(" / ");
      if(pos!=(unsigned int)std::string::npos){
	cfgN.erase(pos,grpN.length()-pos);
	grpN.erase(0,pos+3);
	objN.erase(0,pos+3);
	pos = (unsigned int)grpN.find("_");
	if(pos!=(unsigned int)std::string::npos){
	  grpN.erase(pos,grpN.length()-pos);
	  objN.erase(0,pos+1);
	}
	objptr = &(m_config.subConfig(cfgN)[grpN][objN]);
      } else{
	pos = (unsigned int)grpN.find("_");
	if(pos!=(unsigned int)std::string::npos){
	  grpN.erase(pos,grpN.length()-pos);
	  objN.erase(0,pos+1);
	}
	objptr = &(m_config[grpN][objN]);
      }
      ConfObj &obj = *objptr;
      switch(obj.type()){
      case ConfObj::STRING:
	((ConfString&) obj).m_value = ((QLineEdit*)it->second)->text().toLatin1().data();
	break;
      case ConfObj::BOOL:
	((ConfBool&) obj).m_value = ((QCheckBox*)it->second)->isChecked();
	break;
      case ConfObj::INT:
	//	WriteIntConf(((ConfInt&) obj), ((QSpinBox*)it->second)->value());
	((ConfInt&) obj).setValue(((QSpinBox*)it->second)->value());
	break;
      case ConfObj::FLOAT:{
	QDoubleSpinBox *mval = (QDoubleSpinBox*)it->second;
	QSpinBox       *eval = (QSpinBox*) m_handles[it->first+"_exp"];
	if(eval!=0)
	  ((ConfFloat&)obj).m_value = (float)(mval->value()*pow(10.,(double)eval->value()));
	else{
	  ((ConfFloat&)obj).m_value = (float) mval->value();
	}
	break;}
      case ConfObj::LIST:{
	int read_val = (int) ((ConfList&)obj).m_symbols[((QComboBox*)it->second)->currentText().toLatin1().data()];
	switch( ((ConfList&)obj).subtype() ){
	case ConfList::S32:
	  *((int *)((ConfList&)obj).m_value) = (int) read_val;
	  break;
	case ConfList::U32:
	  *((unsigned int *)((ConfList&)obj).m_value) = (unsigned int) read_val;
	  break;
	case ConfList::S16:
	  *((short int *)((ConfList&)obj).m_value) = (short int) read_val;
	  break;
	case ConfList::U16:
	  *((unsigned short int *)((ConfList&)obj).m_value) = (unsigned short int) read_val;
	  break;
	case ConfList::S8:
	  *((char *)((ConfList&)obj).m_value) = (char) read_val;
	  break;
	case ConfList::U8:
	  *((unsigned char *)((ConfList&)obj).m_value) = (unsigned char) read_val;
	  break;
	default:
	  break;
	}
	break;}
      case ConfObj::VECTOR:
	if(((ConfVector&)obj).subtype()==ConfVector::INT){
	  std::vector<int> &tmpVec = ((ConfVector&)obj).valueVInt();
	  tmpVec.clear();
	  for(int i=0;i<((QTableWidget*)it->second)->rowCount(); i++)
	    tmpVec.push_back(((QTableWidget*)it->second)->item(i,0)->text().toInt());
	  
	}
	if(((ConfVector&)obj).subtype()==ConfVector::UINT){
	  std::vector<unsigned int> &tmpVec = ((ConfVector&)obj).valueVUint();
	  tmpVec.clear();
	  for(int i=0;i<((QTableWidget*)it->second)->rowCount(); i++)
	    tmpVec.push_back(((QTableWidget*)it->second)->item(i,0)->text().toUInt());
			     
	}
	if(((ConfVector&)obj).subtype()==ConfVector::FLOAT){
	  std::vector<float> &tmpVec = ((ConfVector&)obj).valueVFloat();
	  tmpVec.clear();
	  for(int i=0;i<((QTableWidget*)it->second)->rowCount(); i++){
	    tmpVec.push_back(((QTableWidget*)it->second)->item(i,0)->text().toFloat());
	  }	
	}
	break;
      case ConfObj::MATRIX:{
	ConfMatrix &mapobj = (ConfMatrix&)obj;
	if(mapobj.subtype()==ConfMatrix::U16){
	  ConfMask<unsigned short int> &mmask = *((ConfMask<unsigned short int> *)mapobj.m_value);
	  for(int i=0;i<((QTableWidget*)it->second)->rowCount(); i++){
	    for(int j=0;j<((QTableWidget*)it->second)->columnCount(); j++){
 	      mmask[j][i] = ((QTableWidget*)it->second)->item(i,0)->text().toUInt();
	    }
	  }
	}else if(mapobj.subtype()==ConfMatrix::U1){
	  ConfMask<bool> &mmask = *((ConfMask<bool> *)mapobj.m_value);
	  for(int i=0;i<((QTableWidget*)it->second)->rowCount(); i++){
	    for(int j=0;j<((QTableWidget*)it->second)->columnCount(); j++){
 	      mmask[j][i] = (bool)((QTableWidget*)it->second)->item(i,0)->text().toUInt();
	    }
	  }
 	}
	break;}
      default:
	break;
      }
    }
  }
  // close panel
  if(!m_embed) accept();
}
int optionsPanel::fillTab(QWidget *tab, ConfGroup &cgrp, const char *subcfg_name){
  QVBoxLayout *layout1 = new QVBoxLayout( tab );
  int retVal = 0;
  for (int i=0; i<cgrp.size(); i++) {
    ConfObj &obj = cgrp[i];
    if(obj.visible()){
      retVal++;
      QHBoxLayout *layout2 = new QHBoxLayout( 0 );
      QString lname = obj.comment().c_str();
      QLabel *label = new QLabel(lname,tab);
      layout2->addWidget(label);
      QSpacerItem *spacer = new QSpacerItem( 41, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
      layout1->addItem( spacer );
      QWidget *tmpWgt=0, *expWgt=0;
      QPushButton *pbhandle=0;
      switch(obj.type()){
      case ConfObj::STRING:{
	tmpWgt = (QWidget*) new QLineEdit(tab);
	((QLineEdit*)tmpWgt)->setText(((ConfString&)obj).value().c_str());
	if((int)obj.name().find("passw")!=(int)std::string::npos)
	  ((QLineEdit*)tmpWgt)->setEchoMode(QLineEdit::Password);
	m_handles.insert(std::make_pair((subcfg_name==0)?obj.name().c_str():(std::string(subcfg_name)+" / "+obj.name()).c_str(),tmpWgt));
	int type = ((ConfString&)obj).getType();
	if(type>0){
	  pbhandle = new QPushButton("...",tab);
	  fileBrwsManager *fbm = new fileBrwsManager(this, type, tmpWgt);
	  QObject::connect(pbhandle, SIGNAL(clicked()), fbm, SLOT(browse()));
	}
	if(m_embed) connect((QLineEdit*)tmpWgt, SIGNAL(textChanged ( const QString & )), this, SLOT(save()));
	break;}
      case ConfObj::BOOL:{
	tmpWgt = (QWidget*) new QCheckBox("On/Off",tab);
	((QCheckBox*)tmpWgt)->setChecked(((ConfBool&)obj).value());
	m_handles.insert(std::make_pair((subcfg_name==0)?obj.name().c_str():(std::string(subcfg_name)+" / "+obj.name()).c_str(),tmpWgt));
	if(m_embed) connect((QCheckBox*)tmpWgt, SIGNAL(stateChanged(int)), this, SLOT(save()));
	break;}
      case ConfObj::INT:{
	tmpWgt = (QWidget*) new QSpinBox(tab);
	int minVal = 0;
	int maxVal = 1;
	switch(((ConfInt&)obj).subtype()){
	default:
	case ConfInt::S32:
	  minVal = -9999999;
	case ConfInt::U32:
	  maxVal = 9999999;
	  break;
	case ConfInt::U16:
	  maxVal = 65535;
	  break;
	case ConfInt::S16:
	  maxVal =  32767;
	  minVal = -32767;
	  break;
	case ConfInt::U8:
	  maxVal = 255;
	  break;
	case ConfInt::S8:
	  maxVal =  127;
	  minVal = -127;
	  break;
	}
	((QSpinBox*)tmpWgt)->setMaximum(maxVal);
	((QSpinBox*)tmpWgt)->setMinimum(minVal);

	((QSpinBox*)tmpWgt)->setValue(((ConfInt&)obj).getValue());
	m_handles.insert(std::make_pair((subcfg_name==0)?obj.name().c_str():(std::string(subcfg_name)+" / "+obj.name()).c_str(),tmpWgt));
	if(m_embed){
	  connect(tmpWgt, SIGNAL(valueChanged(int)), this, SLOT(save()));
	  //	  connect((QCheckBox*)tmpWgt, SIGNAL(editingFInished()), this, SLOT(save()));
	}
	break;}
      case ConfObj::FLOAT:{
	tmpWgt = (QWidget*) new QDoubleSpinBox(tab);
	// add int spin box for exponent to handle small numbers
	expWgt = new QSpinBox(tab);
	((QDoubleSpinBox*)tmpWgt)->setRange(-9999999, 9999999);
	((QDoubleSpinBox*)tmpWgt)->setDecimals(6);
	((QSpinBox*)expWgt)->setRange(-20,20);
	double value = (double)((ConfFloat&)obj).value();
	int expon = 0;
	if(fabs(value)<1e-3){
	  expon = (int) log10f(value);
	  value /= pow(10.,(double)expon);
	}
	((QDoubleSpinBox*)tmpWgt)->setValue(value);
	((QSpinBox*)expWgt)->setValue(expon);
	m_handles.insert(std::make_pair((subcfg_name==0)?obj.name().c_str():(std::string(subcfg_name)+" / "+obj.name()).c_str(),tmpWgt));
	m_handles.insert(std::make_pair((subcfg_name==0)?(obj.name()+"_exp").c_str():(std::string(subcfg_name)+" / "+obj.name()+"_exp").c_str(),expWgt));
	if(m_embed){
	  connect(tmpWgt, SIGNAL(valueChanged(double)), this, SLOT(save()));
	  //	  connect((QCheckBox*)tmpWgt, SIGNAL(editingFInished()), this, SLOT(save()));
	  connect(expWgt, SIGNAL(valueChanged(int)), this, SLOT(save()));
	}
	break;}
      case ConfObj::LIST:{
	QComboBox *cbhandle = new QComboBox(tab);
	//cbhandle->setEditable(true);
	tmpWgt = (QWidget*)cbhandle; 
	int currID=0;
	std::map<std::string, int>::const_iterator mapIT;
	for(mapIT = ((ConfList&)obj).m_symbols.begin(); mapIT != ((ConfList&)obj).m_symbols.end();mapIT++){
	  cbhandle->addItem(mapIT->first.c_str());
	  if(mapIT->first==((ConfList&)obj).sValue())
	    currID = cbhandle->count()-1;
	}
	cbhandle->setCurrentIndex(currID);
	m_handles.insert(std::make_pair((subcfg_name==0)?obj.name().c_str():(std::string(subcfg_name)+" / "+obj.name()).c_str(),tmpWgt));
	if(m_embed) connect((QComboBox*)tmpWgt, SIGNAL(currentIndexChanged ( int )), this, SLOT(save()));
	break;}
      case ConfObj::VECTOR:{
	if(((ConfVector&)obj).subtype()==ConfVector::INT || ((ConfVector&)obj).subtype()==ConfVector::UINT || 
	   ((ConfVector&)obj).subtype()==ConfVector::FLOAT){
	  QHBoxLayout *layout3 = new QHBoxLayout( 0 );
	  QTableWidget *table = new QTableWidget( tab );
	  layout3->addWidget(table);
	  QSpinBox *sb = new QSpinBox(tab);
	  sb->setMaximum(99999);
	  layout3->addWidget(sb);
	  unsigned int vsize=0;
	  void *vecPtr = 0;
	  if(((ConfVector&)obj).subtype()==ConfVector::INT){
	    std::vector<int> &tmpVec = ((ConfVector&)obj).valueVInt();
	    vecPtr = (void*)&tmpVec;
	    vsize = tmpVec.size();
	  }
	  if(((ConfVector&)obj).subtype()==ConfVector::UINT){
	    std::vector<unsigned int> &tmpVec = ((ConfVector&)obj).valueVUint();
	    vecPtr = (void*)&tmpVec;
	    vsize = tmpVec.size();
	  }
	  if(((ConfVector&)obj).subtype()==ConfVector::FLOAT){
	    std::vector<float> &tmpVec = ((ConfVector&)obj).valueVFloat();
	    vecPtr = (void*)&tmpVec;
	    vsize = tmpVec.size();
	  }
	  table->setRowCount(vsize);
	  sb->setValue(vsize);
	  table->setColumnCount(1);
	  for(unsigned int j=0;j<vsize; j++){
	    QTableWidgetItem *titem=0;
	    if(((ConfVector&)obj).subtype()==ConfVector::INT)
	      titem = new QTableWidgetItem(QString::number(((std::vector<int>*)vecPtr)->at(j)));
	    if(((ConfVector&)obj).subtype()==ConfVector::UINT)
	      titem = new QTableWidgetItem(QString::number(((std::vector<unsigned int>*)vecPtr)->at(j)));
	    if(((ConfVector&)obj).subtype()==ConfVector::FLOAT)
	      titem = new QTableWidgetItem(QString::number((double)((std::vector<float>*)vecPtr)->at(j),'g'));
	    if(titem!=0) table->setItem(j,0,titem);
	  }
	  connect(sb, SIGNAL(valueChanged(int)), this, SLOT(setTableNrows(int)));
	  tmpWgt = 0;
	  layout2->addLayout(layout3);
	  std::string objName = (subcfg_name==0)?obj.name():(std::string(subcfg_name)+" / "+obj.name());
	  m_handles.insert(std::make_pair(objName.c_str(),(QWidget*)table));
	  sb->setObjectName(QString((objName+"_SpinBox").c_str()));
	} else
	  tmpWgt = (QWidget*) new QLabel("unhandled ConfObj::VECTOR type",tab);
	break;}
      case ConfObj::MATRIX:{
	QHBoxLayout *layout3 = new QHBoxLayout( 0 );
	QTableWidget *table = new QTableWidget( tab );
	layout3->addWidget(table);
	QSpinBox *sbr = new QSpinBox(tab);
	sbr->setMaximum(99999);
	layout3->addWidget(sbr);
	QSpinBox *sbc = new QSpinBox(tab);
	sbc->setMaximum(99999);
	layout3->addWidget(sbc);
	ConfMatrix &mapobj = (ConfMatrix&)obj;
	if(mapobj.subtype()==ConfMatrix::U16){
	  ConfMask<unsigned short int> &mmask = *((ConfMask<unsigned short int> *)mapobj.m_value);
	  unsigned int nCol = mmask.get().size();
	  unsigned int nRow = mmask.get().front().size();
	  table->setRowCount(nRow);
	  sbr->setValue(nRow);
	  table->setColumnCount(nCol);
	  sbc->setValue(nCol);
	  for(unsigned int j=0;j<nRow; j++){
	    for(unsigned int k=0;k<nCol; k++){
	      QTableWidgetItem *titem = new QTableWidgetItem(QString::number(mmask[k][j]));
	      table->setItem(j,k,titem);
	    }
	  }
	} else if(mapobj.subtype()==ConfMatrix::U1){
	  ConfMask<bool> &mmask = *((ConfMask<bool> *)mapobj.m_value);
	  unsigned int nCol = mmask.get().size();
	  unsigned int nRow = mmask.get().front().size();
	  table->setRowCount(nRow);
	  sbr->setValue(nRow);
	  table->setColumnCount(nCol);
	  sbc->setValue(nCol);
	  for(unsigned int j=0;j<nRow; j++){
	    for(unsigned int k=0;k<nCol; k++){
	      QTableWidgetItem *titem = new QTableWidgetItem(QString::number(mmask[k][j]));
	      table->setItem(j,k,titem);
	    }
	  }
	} else{
	  tmpWgt = (QWidget*) new QLabel("unhandled ConfObj::MATRIX type",tab);
	  break;
	}
	// must not change size!
	sbr->setEnabled(false);
	sbc->setEnabled(false);
	tmpWgt = 0;
	layout2->addLayout(layout3);
	m_handles.insert(std::make_pair((subcfg_name==0)?obj.name().c_str():(std::string(subcfg_name)+" / "+obj.name()).c_str(),(QWidget*)table));
	break;}
      default:
	tmpWgt = (QWidget*) new QLabel("unhandled ConfObj type",tab);
      }
      if(tmpWgt!=0)
	layout2->addWidget(tmpWgt);
      if(expWgt!=0){
	QLabel *elab = new QLabel("E",tab);
	layout2->addWidget(elab);
	layout2->addWidget(expWgt);
      }
      if(pbhandle!=0) 
	layout2->addWidget(pbhandle);
      layout1->addLayout(layout2);
    }
  }
  return retVal;
}
void optionsPanel::setTableNrows(int nrow){
  QSpinBox *sb = dynamic_cast<QSpinBox*>(sender());
  if(sb!=0 && sb->objectName().right(8)=="_SpinBox"){
    QTableWidget *fieldTable = dynamic_cast<QTableWidget*>(m_handles[std::string(sb->objectName().left(sb->objectName().length()-8).toLatin1().data())]);
    if(fieldTable!=0){
      int oldNrow = fieldTable->rowCount();
      for(int i=nrow;i<oldNrow;i++)
	fieldTable->removeRow(i);
      for(int i=oldNrow;i<nrow;i++){
	fieldTable->insertRow(i);
	fieldTable->setItem(i,0, new QTableWidgetItem(QString("0")));
      }
    }
  }
}
