#include <qwidget.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3filedialog.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "CfgForm.h"

CfgFormItem::CfgFormItem(QWidget* parent, const char* , const char *label, const char *filter)
  : QWidget(parent){

  m_Layout = new Q3VBoxLayout( 0, 0, 6, "myLayout"); 
  
  m_Label = new QLabel( parent, "Label" );
  m_Label->setText( trUtf8( label ) );
  m_Layout->addWidget( m_Label );
  
  Q3HBoxLayout *Layout2 = new Q3HBoxLayout( 0, 0, 6, "Layout2"); 
  
  m_FileName = new QLineEdit( parent, "FileName" );
  Layout2->addWidget( m_FileName );
  
  m_Button = new QPushButton( parent, "Button" );
  m_Button->setText( trUtf8( "Browse" ) );
  Layout2->addWidget( m_Button );
  m_Layout->addLayout( Layout2 );
  
  m_Filter = new QString(filter);
  
  connect( m_Button, SIGNAL( clicked() ), this, SLOT( browseFiles() ) );
  
}
CfgFormItem::~CfgFormItem(){
}
void CfgFormItem::browseFiles(){
  QStringList filter(*m_Filter);
  filter += "Any file (*.*)";
  Q3FileDialog fdia(QString::null, QString::null, this,"select file",TRUE);
  fdia.setFilters(filter);
  fdia.setMode(Q3FileDialog::AnyFile);
  if(fdia.exec() == QDialog::Accepted) 
    m_FileName->setText(fdia.selectedFile());
}

CfgForm::CfgForm( QWidget* parent, const char* , bool , Qt::WFlags fl)
  : QDialog(parent,fl){
  //  : CfgFormBase(parent,name,modal,fl){
  setupUi(this);
  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  Q3VBoxLayout *CfgFormBaseLayout = new Q3VBoxLayout( this, 11, 6, "CfgFormBaseLayout"); 
  Q3VBoxLayout *mainLayout = new Q3VBoxLayout( 0, 0, 6, "mainLayout"); 
  
  QSpacerItem* hspacer2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum , QSizePolicy::Expanding);
  mainLayout->addItem( hspacer2 );

  m_items.push_back(new CfgFormItem(this,"cfgfile","Main configuration file","Config file (*.cfg)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  m_items.push_back(new CfgFormItem(this,"tdfile","TDAC files (\"_0\" is replaced accordingly)",
				    "TDAC file (*tdacs_0.out)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  m_items.push_back(new CfgFormItem(this,"fdfile","FDAC files (\"_0\" is replaced accordingly)",
				    "FDAC file (*fdacs_0.out)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  m_items.push_back(new CfgFormItem(this,"rmfile","Readout mask files (\"_0\" is replaced accordingly)",
				    "Mask file (*_0.mask)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  m_items.push_back(new CfgFormItem(this,"rmfile","Strobe mask files (\"_0\" is replaced accordingly)",
				    "Mask file (*_0.mask)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  m_items.push_back(new CfgFormItem(this,"rmfile","Preamp mask files (\"_0\" is replaced accordingly)",
				    "Mask file (*_0.mask)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  m_items.push_back(new CfgFormItem(this,"rmfile","Hitbus mask files (\"_0\" is replaced accordingly)",
				    "Mask file (*_0.mask)"));
  mainLayout->addLayout( m_items[m_items.size()-1]->m_Layout );
  
  QSpacerItem* hspacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum , QSizePolicy::Expanding);
  mainLayout->addItem( hspacer );

  Q3HBoxLayout *bottomLayout = new Q3HBoxLayout( 0, 0, 6, "bottomLayout"); 
  bottomLayout->addWidget( okButton );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  bottomLayout->addItem( spacer );
  bottomLayout->addWidget( cancelButton );
  mainLayout->addLayout( bottomLayout );

  CfgFormBaseLayout->addLayout( mainLayout );
}
CfgForm::~CfgForm(){
}
