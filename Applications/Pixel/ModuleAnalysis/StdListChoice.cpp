#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qvariant.h>
#include <q3scrollview.h>
#include <qpushbutton.h>
#include <q3vbox.h> 
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "StdListChoice.h"
#include "TopWin.h"

StdListChoice::StdListChoice(QWidget *parent, const char* name, bool modal, Qt::WFlags fl, TopWin *tw)
 : QDialog(parent, name, modal, fl){
  // start with groups etc.
  Q3VBoxLayout *mainLayout = new Q3VBoxLayout( this, 11, 6, "StdTestWinBaseLayout"); 
  m_Heading = new QLabel( this, "Select files to be loaded for type ...");
  m_Heading->setAlignment( int( Qt::AlignVCenter | Qt::AlignHCenter ) );
  mainLayout->addWidget( m_Heading );
  // then the scroll view within
  m_scrlv = new Q3ScrollView(this,"scrv");
  m_scrlv->setVScrollBarMode(Q3ScrollView::AlwaysOn);
  Q3VBox *scrollBox = new Q3VBox( m_scrlv->viewport());
  m_scrlv->addChild(scrollBox);
  // then create the tick boxes into scroll view
  m_stdwin = new StdTestWin(tw,"dummy_std");
  QCheckBox *box;
  for(int i=0;i<m_stdwin->NSLOTS;i++){
    box = new QCheckBox(m_stdwin->m_items[i]->m_label->text(),scrollBox, "cbox_"+QString::number(i));
    m_checkBoxes.push_back(box);
  }
  mainLayout->addWidget( m_scrlv );
  // buttons
  Q3HBoxLayout *Layout1 = new Q3HBoxLayout( 0, 0, 6, "Layout39"); 
  QPushButton *ca = new QPushButton(this, "ca");
  ca->setText("Check All");
  Layout1->addWidget( ca );
  QSpacerItem* spacer_1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout1->addItem( spacer_1 );
  QPushButton *ua = new QPushButton(this, "ua");
  ua->setText("Uncheck All");
  Layout1->addWidget( ua );
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout1->addItem( spacer_2 );
  QPushButton *cb = new QPushButton(this, "cb");
  cb->setText("Close");
  Layout1->addWidget( cb );
  mainLayout->addLayout( Layout1 );
  connect( cb, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( ca, SIGNAL( clicked() ), this, SLOT( checkAll() ) );
  connect( ua, SIGNAL( clicked() ), this, SLOT( uncheckAll() ) );
  // some basic settings
  setBaseSize(500,600);
  setCaption( tr( "Standard Test File Selection" ) );
}
StdListChoice::~StdListChoice(){
  return;
}
void StdListChoice::setType(int type){
  if(type>=0 && type<NTYPES){ // check if reasonable type was given
    m_stdwin->TestTypeSel->setCurrentItem(type);
    m_stdwin->TypeSet();
    m_stdwin->GenerateNames();
    m_Heading->setText("Select files to be loaded for type "+m_stdwin->TestTypeSel->currentText());
    for(int i=0;i<m_stdwin->NSLOTS;i++){
      m_checkBoxes[i]->setText(m_stdwin->m_items[i]->m_label->text());
      if(m_stdwin->m_items[i]->m_fname->text().isEmpty()){
	m_checkBoxes[i]->setChecked(false);
	m_checkBoxes[i]->setEnabled(false);
      	m_checkBoxes[i]->hide();
      }else{
	m_checkBoxes[i]->setChecked(true);
	m_checkBoxes[i]->setEnabled(true);
	m_checkBoxes[i]->show();
      }
    }
  }
}
void StdListChoice::setAll(bool setTo){
  for(int i=0;i<m_stdwin->NSLOTS;i++)
    if(m_checkBoxes[i]->isEnabled())
      m_checkBoxes[i]->setChecked(setTo);
}
