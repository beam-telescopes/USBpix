#include <qcheckbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <q3filedialog.h>
#include <qdir.h>
#include <qtabwidget.h>
#include <qcombobox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <TMath.h>

#include <string>

#include "CutPanel.h"
#include "DataStuff.h"
#include "OptClass.h"
#include "OptWin.h"
#include "FileTypes.h"
#include "TopWin.h"
#include "MAEngine.h"

CutClassItem::CutClassItem(Q3ListView *parent,QString name) : Q3ListViewItem(parent,name){
  SetDefaultCuts();
  m_path="none";
  m_version=DEFCUTVER;
}
CutClassItem::~CutClassItem(){
}

void CutClassItem::SetDefaultCuts(){

  // bad pixels

  cp[0].min = 100;
  cp[0].max = 100;
  cp[0].enabled = true;
  cp[0].label = "hits (digital scan)";
  cp[0].unit = "%";

  cp[1].min = 50;
  cp[1].max = UNDEFCUT;
  cp[1].enabled = true;
  cp[1].label = "abs. noise diff. HVon-HVoff";
  cp[1].unit = "e";

  cp[2].min = 200;
  cp[2].max = UNDEFCUT;
  cp[2].enabled = true;
  cp[2].label = "noise (OR with above)";
  cp[2].unit = "e";

  cp[3].min = UNDEFCUT;
  cp[3].max = 3.5;
  cp[3].enabled = true;
  cp[3].label = "xtalk fraction (merged)";
  cp[3].unit = "%";

  cp[4].min = 2000;
  cp[4].max = UNDEFCUT;
  cp[4].enabled = true;
  cp[4].label = "xtalk noise (OR with above)";
  cp[4].unit = "e";

  cp[5].min = 1;
  cp[5].max = UNDEFCUT;
  cp[5].enabled = true;
  cp[5].label = "source hits";
  cp[5].unit = " ";

  cp[6].min = 5;
  cp[6].max = UNDEFCUT;
  cp[6].enabled = true;
  cp[6].label = "source max. ToT";
  cp[6].unit = "cnts.";

  cp[7].min = 0;
  cp[7].max = 0;
  cp[7].enabled = true;
  cp[7].label = "source masked";
  cp[7].unit = " ";

  cp[8].min = UNDEFCUT;
  cp[8].max = 5;
  cp[8].enabled = true;
  cp[8].label = "noise";
  cp[8].unit = "sigma";

  cp[9].min = -5;
  cp[9].max = 5;
  cp[9].enabled = true;
  cp[9].label = "diff. from mean threshold";
  cp[9].unit = "sigma";

//   cp[10].min = UNDEFCUT;
//   cp[10].max = 500;
//   cp[10].enabled = false;
//   cp[10].label = "diff. from mean threshold";
//   cp[10].unit = "e";

  cp[10].min = 48;
  cp[10].max = 52;
  cp[10].enabled = true;
  cp[10].label = "mean ToT (digital)";
  cp[10].unit = "cnts";

  cp[11].min = 0;
  cp[11].max = 50;
  cp[11].enabled = true;
  cp[11].label = "threshold S-fit chi^2";
  cp[11].unit = " ";

  cp[12].min = -5;
  cp[12].max = 5;
  cp[12].enabled = true;
  cp[12].label = "leakage current";
  cp[12].unit = "sigma";

  cp[13].min = UNDEFCUT;
  cp[13].max = 5;
  cp[13].enabled = true;
  cp[13].label = "crosstalk";
  cp[13].unit = "%";

  cp[14].min = 0;
  cp[14].max = 5;
  cp[14].enabled = true;
  cp[14].label = "ToT-calibration chi^2";
  cp[14].unit = " ";

  cp[15].min = 10;
  cp[15].max = 3000;
  cp[15].enabled = true;
  cp[15].label = "overdrive";
  cp[15].unit = "e";

  cp[16].min = UNDEFCUT;
  cp[16].max = 10;
  cp[16].enabled = true;
  cp[16].label = "source rate ratio";
  cp[16].unit = "";

  cp[17].min = 100;
  cp[17].max = 100;
  cp[17].enabled = true;
  cp[17].label = "hits (corr. CP, CP scan)";
  cp[17].unit = "%";

  cp[18].min = 0;
  cp[18].max = 0;
  cp[18].enabled = true;
  cp[18].label = "hits (wrong CP, CP scan)";
  cp[18].unit = "%";

  // cuts on entire module

  cp[NCUTS+0].min = UNDEFCUT;
  cp[NCUTS+0].max = 460;
  cp[NCUTS+0].enabled = true;
  cp[NCUTS+0].label = "Number bad pixels";
  cp[NCUTS+0].unit = " ";

  cp[NCUTS+1].min = 150;
  cp[NCUTS+1].max = UNDEFCUT;
  cp[NCUTS+1].enabled = false;
  cp[NCUTS+1].label = "Sensor break-down";
  cp[NCUTS+1].unit = "V";

  cp[NCUTS+2].min = UNDEFCUT;
  cp[NCUTS+2].max = 10;
  cp[NCUTS+2].enabled = true;
  cp[NCUTS+2].label = "Sensor current @ 150V";
  cp[NCUTS+2].unit = "uA";

  cp[NCUTS+3].min = UNDEFCUT;
  cp[NCUTS+3].max = 5000;
  cp[NCUTS+3].enabled = true;
  cp[NCUTS+3].label = "Average threshold";
  cp[NCUTS+3].unit = "e";

  cp[NCUTS+4].min = UNDEFCUT;
  cp[NCUTS+4].max = 200;
  cp[NCUTS+4].enabled = true;
  cp[NCUTS+4].label = "Threshold dispersion";
  cp[NCUTS+4].unit = "e";

  cp[NCUTS+5].min = UNDEFCUT;
  cp[NCUTS+5].max = 300;
  cp[NCUTS+5].enabled = true;
  cp[NCUTS+5].label = "Average noise";
  cp[NCUTS+5].unit = "e";

  cp[NCUTS+6].min = UNDEFCUT;
  cp[NCUTS+6].max = 6000;
  cp[NCUTS+6].enabled = true;
  cp[NCUTS+6].label = "Average in-time threshold";
  cp[NCUTS+6].unit = "e";

  cp[NCUTS+7].min = UNDEFCUT;
  cp[NCUTS+7].max = 300;
  cp[NCUTS+7].enabled = true;
  cp[NCUTS+7].label = "In-time threshold dispersion";
  cp[NCUTS+7].unit = "e";

  cp[NCUTS+8].min = UNDEFCUT;
  cp[NCUTS+8].max = 3;
  cp[NCUTS+8].enabled = true;
  cp[NCUTS+8].label = "Average crosstalk";
  cp[NCUTS+8].unit = "%";

  cp[NCUTS+9].min = 20;
  cp[NCUTS+9].max = 60;
  cp[NCUTS+9].enabled = true;
  cp[NCUTS+9].label = "Average ToT(mip)";
  cp[NCUTS+9].unit = "cnts.";

  cp[NCUTS+10].min = 0.05;
  cp[NCUTS+10].max = 3;
  cp[NCUTS+10].enabled = true;
  cp[NCUTS+10].label = "ToT(mip) dispersion";
  cp[NCUTS+10].unit = "cnts.";

  cp[NCUTS+11].min = UNDEFCUT;
  cp[NCUTS+11].max = UNDEFCUT;
  cp[NCUTS+11].enabled = true;
  cp[NCUTS+11].label = "Analogue T0";
  cp[NCUTS+11].unit = "?";

  cp[NCUTS+12].min = UNDEFCUT;
  cp[NCUTS+12].max = UNDEFCUT;
  cp[NCUTS+12].enabled = true;
  cp[NCUTS+12].label = "Digital T0";
  cp[NCUTS+12].unit = "?";

  cp[NCUTS+13].min = UNDEFCUT;
  cp[NCUTS+13].max = UNDEFCUT;
  cp[NCUTS+13].enabled = true;
  cp[NCUTS+13].label = "FDAC tuning disp.";
  cp[NCUTS+13].unit = "cnts.";

  cp[NCUTS+14].min = UNDEFCUT;
  cp[NCUTS+14].max = UNDEFCUT;
  cp[NCUTS+14].enabled = true;
  cp[NCUTS+14].label = "Dummy cut";
  cp[NCUTS+14].unit = " ";

  cp[NCUTS+15].min = UNDEFCUT;
  cp[NCUTS+15].max = 0.5;
  cp[NCUTS+15].enabled = true;
  cp[NCUTS+15].label = "ToT chip-RMS";
  cp[NCUTS+15].unit = "cnts.";

  cp[NCUTS+16].min = UNDEFCUT;
  cp[NCUTS+16].max = 48;
  cp[NCUTS+16].enabled = true;
  cp[NCUTS+16].label = "avg. ToT fit chi2/ndf";
  cp[NCUTS+16].unit = " ";

  cp[NCUTS+17].min = 2.0;
  cp[NCUTS+17].max = 2.5;
  cp[NCUTS+17].enabled = false;
  cp[NCUTS+17].label = "VDD";
  cp[NCUTS+17].unit = "V";

  cp[NCUTS+18].min = 1.5;
  cp[NCUTS+18].max = 1.9;
  cp[NCUTS+18].enabled = false;
  cp[NCUTS+18].label = "VDDA";
  cp[NCUTS+18].unit = "V";

  // range cuts

  cp[START_RANGE+0].min = UNDEFCUT;
  cp[START_RANGE+0].max = 2.0;
  cp[START_RANGE+0].enabled = true;
  cp[START_RANGE+0].label = "VDDmin (digital OK)";
  cp[START_RANGE+0].unit = "V";
       
  cp[START_RANGE+1].min = 2.3;
  cp[START_RANGE+1].max = UNDEFCUT;
  cp[START_RANGE+1].enabled = true;
  cp[START_RANGE+1].label = "VDDmax (digital OK)";
  cp[START_RANGE+1].unit = "V";
     
  cp[START_RANGE+2].min = 0;
  cp[START_RANGE+2].max = 100;
  cp[START_RANGE+2].enabled = true;
  cp[START_RANGE+2].label = "Xck digital - #bad pix.";
  cp[START_RANGE+2].unit = "cnts";
     
  cp[START_RANGE+3].min = 40.0;
  cp[START_RANGE+3].max = UNDEFCUT;
  cp[START_RANGE+3].enabled = true;
  cp[START_RANGE+3].label = "Xck with digital OK";
  cp[START_RANGE+3].unit = "MHz";
     
  cp[START_RANGE+4].min = UNDEFCUT;
  cp[START_RANGE+4].max = 10;
  cp[START_RANGE+4].enabled = true;
  cp[START_RANGE+4].label = "VDDA scans - #bad pix.";
  cp[START_RANGE+4].unit = "%";
     
  cp[START_RANGE+5].min = UNDEFCUT;
  cp[START_RANGE+5].max = 1.55;
  cp[START_RANGE+5].enabled = true;
  cp[START_RANGE+5].label = "VDDAmin (ana. OK)";
  cp[START_RANGE+5].unit = "V";
     
  cp[START_RANGE+6].min = 1.75;
  cp[START_RANGE+6].max = UNDEFCUT;
  cp[START_RANGE+6].enabled = true;
  cp[START_RANGE+6].label = "VDDAmax (ana. OK)";
  cp[START_RANGE+6].unit = "V";
       
  cp[START_RANGE+7].min = 31;
  cp[START_RANGE+7].max = 31;
  cp[START_RANGE+7].enabled = true;
  cp[START_RANGE+7].label = "GDAC good bits";
  cp[START_RANGE+7].unit = " ";
     
  cp[START_RANGE+8].min = UNDEFCUT;
  cp[START_RANGE+8].max = UNDEFCUT;
  cp[START_RANGE+8].enabled = true;
  cp[START_RANGE+8].label = "Min avg. threshold (GDAC)";
  cp[START_RANGE+8].unit = "e";
     
  cp[START_RANGE+9].min = UNDEFCUT;
  cp[START_RANGE+9].max = UNDEFCUT;
  cp[START_RANGE+9].enabled = true;
  cp[START_RANGE+9].label = "Max avg. threshold (GDAC)";
  cp[START_RANGE+9].unit = "e";

  // chip cuts

  cp[NCUTS+NMCUTS+0].min = UNDEFCUT;
  cp[NCUTS+NMCUTS+0].max = UNDEFCUT;
  cp[NCUTS+NMCUTS+0].enabled = false;
  cp[NCUTS+NMCUTS+0].label = "I_DD (reg. test)";
  cp[NCUTS+NMCUTS+0].unit = "mA";
     
  cp[NCUTS+NMCUTS+1].min = UNDEFCUT;
  cp[NCUTS+NMCUTS+1].max = UNDEFCUT;
  cp[NCUTS+NMCUTS+1].enabled = false;
  cp[NCUTS+NMCUTS+1].label = "I_DDA (reg. test)";
  cp[NCUTS+NMCUTS+1].unit = "mA";
       
  cp[NCUTS+NMCUTS+2].min = UNDEFCUT;
  cp[NCUTS+NMCUTS+2].max = UNDEFCUT;
  cp[NCUTS+NMCUTS+2].enabled = false;
  cp[NCUTS+NMCUTS+2].label = "I_DD (dig. scan)";
  cp[NCUTS+NMCUTS+2].unit = "mA";
     
  cp[NCUTS+NMCUTS+3].min = UNDEFCUT;
  cp[NCUTS+NMCUTS+3].max = UNDEFCUT;
  cp[NCUTS+NMCUTS+3].enabled = false;
  cp[NCUTS+NMCUTS+3].label = "I_DDA (dig. scan)";
  cp[NCUTS+NMCUTS+3].unit = "mA";

  cp[NCUTS+NMCUTS+4].min = UNDEFCUT;
  cp[NCUTS+NMCUTS+4].max = UNDEFCUT;
  cp[NCUTS+NMCUTS+4].enabled = false;
  cp[NCUTS+NMCUTS+4].label = "Number bad pixels/chip";
  cp[NCUTS+NMCUTS+4].unit = " ";

  cp[NCUTS+NMCUTS+5].min = UNDEFCUT;
  cp[NCUTS+NMCUTS+5].max = UNDEFCUT;
  cp[NCUTS+NMCUTS+5].enabled = false;
  cp[NCUTS+NMCUTS+5].label = "CP mask from regtest";
  cp[NCUTS+NMCUTS+5].unit = " ";

  cp[NCUTS+NMCUTS+6].min = 13;
  cp[NCUTS+NMCUTS+6].max = 17;
  cp[NCUTS+NMCUTS+6].enabled = false;
  cp[NCUTS+NMCUTS+6].label = "avg. ToT";
  cp[NCUTS+NMCUTS+6].unit = "cnts.";

}

int CutClassItem::itemID(){
  int cnt=0;
  Q3ListViewItem *item = parent();
  if(item==0)
    item = listView()->firstChild();
  else
    item = item->firstChild();
  while(item!=0){
    if(item==(Q3ListViewItem*)this)
      return cnt;
    cnt++;
    item = item->nextSibling();
  }
  return -1;
}

CutWidget::CutWidget(QWidget *parent, const char *name) : QWidget(parent,name){

    Layout = new Q3HBoxLayout(0,0,6);

    CutLabel = new QLabel( parent, "CutLabel" );
    //    CutLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)5, CutLabel->sizePolicy().hasHeightForWidth() ) );
    CutLabel->setMinimumSize( QSize( 190, 0 ) );
    CutLabel->setText( tr( "CutLabel" ) );
    Layout->addWidget( CutLabel );

    MinLabel = new QLabel( parent, "MinLabel" );
    MinLabel->setText( tr( "from" ) );
    Layout->addWidget( MinLabel );

    LowerCut = new QLineEdit( parent, "LowerCut" );
    LowerCut->setMinimumSize( QSize( 55, 0 ) );
    Layout->addWidget( LowerCut );

    MaxLabel = new QLabel( parent, "MaxLabel" );
    MaxLabel->setText( tr( "to" ) );
    Layout->addWidget( MaxLabel );

    UpperCut = new QLineEdit( parent, "UpperCut" );
    UpperCut->setMinimumSize( QSize( 55, 0 ) );
    Layout->addWidget( UpperCut );

    UnitLabel = new QLabel( parent, "UnitLabel" );
    UnitLabel->setMinimumSize( QSize( 45, 0 ) );
    UnitLabel->setText( tr( "" ) );
    Layout->addWidget( UnitLabel );

    //    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    //    Layout->addItem( spacer );

    Enabled = new QCheckBox( parent, "Enabled" );
    Enabled->setMinimumSize( QSize( 80, 0 ) );
    Enabled->setText( tr( "enabled" ) );
    Layout->addWidget( Enabled );

    connect(Enabled,SIGNAL(toggled(bool)),CutLabel,SLOT(setEnabled(bool)));
    connect(Enabled,SIGNAL(toggled(bool)),MinLabel,SLOT(setEnabled(bool)));
    connect(Enabled,SIGNAL(toggled(bool)),MaxLabel,SLOT(setEnabled(bool)));
    connect(Enabled,SIGNAL(toggled(bool)),UnitLabel,SLOT(setEnabled(bool)));
    connect(Enabled,SIGNAL(toggled(bool)),LowerCut,SLOT(setEnabled(bool)));
    connect(Enabled,SIGNAL(toggled(bool)),UpperCut,SLOT(setEnabled(bool)));

    m_cp = NULL;
}

void CutWidget::Reset(){
  if(m_cp==NULL) return;
  CutLabel->setText( m_cp->label.c_str());
  if(m_cp->min!=UNDEFCUT){
    if(TMath::Abs(m_cp->min)>0.03 || m_cp->min==0)
      LowerCut->setText(QString::number(m_cp->min,'f',2));
    else
      LowerCut->setText(QString::number(m_cp->min,'e',2));
  } else
    LowerCut->setText("...");
  if(m_cp->max!=UNDEFCUT){
    if(TMath::Abs(m_cp->max)>0.03 || m_cp->max==0)
      UpperCut->setText(QString::number(m_cp->max,'f',2));
    else
      UpperCut->setText(QString::number(m_cp->max,'e',2));
  } else
    UpperCut->setText("...");
  UnitLabel->setText(m_cp->unit.c_str());
  Enabled->setChecked(m_cp->enabled);
  CutLabel->setEnabled(m_cp->enabled);
  MinLabel->setEnabled(m_cp->enabled);
  MaxLabel->setEnabled(m_cp->enabled);
  UnitLabel->setEnabled(m_cp->enabled);
  LowerCut->setEnabled(m_cp->enabled);
  UpperCut->setEnabled(m_cp->enabled);

}

void CutWidget::Save(){
  if(m_cp==NULL) return;
  m_cp->label = CutLabel->text().latin1();
  if(LowerCut->text()=="...")
    m_cp->min = UNDEFCUT;
  else
    m_cp->min = LowerCut->text().toDouble();
  if(UpperCut->text()=="...")
    m_cp->max = UNDEFCUT;
  else
    m_cp->max = UpperCut->text().toDouble();
  m_cp->enabled = Enabled->isChecked();
  m_cp->unit = UnitLabel->text().latin1();
}

CutWidget::~CutWidget(){}

CutPanel::CutPanel( QWidget* parent, Qt::WFlags fl, int npixcuts, int user_nmcuts, QString cutfile) 
  : QDialog(parent,fl){
  //  : CutPanelBase(parent,name,modal,fl){

  setupUi(this);
  QObject::connect(CloseButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(ResetButt, SIGNAL(clicked()), this, SLOT(Reset()));
  QObject::connect(SaveButt, SIGNAL(clicked()), this, SLOT(Save()));
  QObject::connect(SCButt, SIGNAL(clicked()), this, SLOT(SaveClose()));
  QObject::connect(RenameButton, SIGNAL(clicked()), this, SLOT(RenameClass()));
  QObject::connect(NewButton, SIGNAL(clicked()), this, SLOT(NewClass()));
  QObject::connect(LoadButton, SIGNAL(clicked()), this, SLOT(LoadClass()));
  QObject::connect(SaveButton, SIGNAL(clicked()), this, SLOT(SaveClass()));
  QObject::connect(DeleteButton, SIGNAL(clicked()), this, SLOT(DeleteClass()));
  QObject::connect(ClassList, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(selectedClass(Q3ListViewItem*)));

  char clabel[500];

  cutdefBox->insertItem("Default cuts");
  for(int i=1;i<100; i++){
    QString clabel = MAEngine::getSpecialCutLabel(i);
    if(clabel=="Default cuts") break;
    cutdefBox->insertItem(clabel);
  }

  if(npixcuts>=0){
    ncuts = npixcuts;
    npcuts = npixcuts;
  } else{
    ncuts = NCUTS;
    npcuts = NCUTS;
  }
  if(user_nmcuts>=0)
    ncuts += user_nmcuts;
  else
    ncuts += NMCUTS;
  nmcuts = ncuts;
  ncuts += NCCUTS;

  m_cw  = new CutWidget*[ncuts];
  
  tab_2 = new QWidget( TabWidg, "tab_2");
  tab_3 = new QWidget( TabWidg, "tab_3");
  tab_4 = new QWidget( TabWidg, "tab_4");
  tab_5 = new QWidget( TabWidg, "tab_5");

  Q3VBoxLayout *tab2LayoutMain = new Q3VBoxLayout( tab_2, 11, 6, "tab2LayoutMain"); 
  Q3VBoxLayout *tab2LayoutSub  = new Q3VBoxLayout(0,0,6,"tab2LayoutSub"); 
  Q3VBoxLayout *tab3LayoutMain = new Q3VBoxLayout( tab_3, 11, 6, "tab3LayoutMain"); 
  Q3VBoxLayout *tab3LayoutSub  = new Q3VBoxLayout(0,0,6,"tab3LayoutSub"); 
  Q3VBoxLayout *tab4LayoutMain = new Q3VBoxLayout( tab_4, 11, 6, "tab4LayoutMain"); 
  Q3VBoxLayout *tab4LayoutSub  = new Q3VBoxLayout(0,0,6,"tab4LayoutSub"); 
  Q3VBoxLayout *tab5LayoutMain = new Q3VBoxLayout( tab_5, 11, 6, "tab5LayoutMain"); 
  Q3VBoxLayout *tab5LayoutSub  = new Q3VBoxLayout(0,0,6,"tab5LayoutSub"); 

  for(int i=0;i<ncuts;i++){
    sprintf(clabel,"cw%d",i);
    if(i<npcuts){  // bad pixel cuts
      m_cw[i] = new CutWidget(tab_2,clabel);
      tab2LayoutSub->addLayout( m_cw[i]->Layout );
    } else if(i<START_RANGE){ // general module cuts
      m_cw[i] = new CutWidget(tab_3,clabel);
      tab3LayoutSub->addLayout( m_cw[i]->Layout );
    } else if(i<ncuts-NCCUTS) {
      m_cw[i] = new CutWidget(tab_4,clabel);
      tab4LayoutSub->addLayout( m_cw[i]->Layout );
    } else {  // chip cuts
      m_cw[i] = new CutWidget(tab_5,clabel);
      tab5LayoutSub->addLayout( m_cw[i]->Layout );
    }
  }
  tab2LayoutMain->addLayout( tab2LayoutSub );
  tab3LayoutMain->addLayout( tab3LayoutSub );
  tab4LayoutMain->addLayout( tab4LayoutSub );
  tab5LayoutMain->addLayout( tab5LayoutSub );

  TabWidg->insertTab( tab_2, QString("Pixel cuts") );
  TabWidg->insertTab( tab_3, QString("Module cuts") );
  TabWidg->insertTab( tab_4, QString("Range cuts") );
  TabWidg->insertTab( tab_5, QString("Chip cuts") );

  int nread=1;
  char path[2000], *tmpc;
  m_cutfile = cutfile;
  QString cutpath = QDir::homeDirPath() + cutfile; 
  FILE *pf = fopen(cutpath.latin1(),"r");
  if(pf==NULL) return;
  while((tmpc=fgets(path,1999,pf))!=NULL){
    if(nread==1){
      if(path[strlen(path)-1]=='\n') path[strlen(path)-1] = '\0';
      LoadClass(path);
    }
  }
  fclose(pf);

  Reset();
  hideCCTab(((TopWin*) parentWidget())->options->m_Opts->m_showChipCuts);
}

CutPanel::~CutPanel(){
  QString cutpath = QDir::homeDirPath() + m_cutfile; 
  FILE *pf = fopen(cutpath.latin1(),"w");
  if(pf!=0){
    CutClassItem *item = (CutClassItem*) ClassList->firstChild();
    while(item!=NULL){
      fprintf(pf,"%s\n",item->m_path.c_str());
      item = (CutClassItem*) item->nextSibling();
    }
    fclose(pf);
  }
  delete[] m_cw;
}

void CutPanel::Reset(){
  for(int i=0;i<ncuts;i++){
    m_cw[i]->Reset();
  }
  if(ClassList->currentItem()!=NULL) CutClassLabel->setText(ClassList->currentItem()->text(0));
  optClass *oc = ((TopWin*) parentWidget())->options->m_Opts;
  IgnoreBadPix->setChecked(oc->m_CutIgnoreBad);
}

void CutPanel::Save(){
  // create default cuts to compare
  CutClassItem *defitem = new CutClassItem(ClassList,"Default cuts");
  bool changed=false;

  for(int i=0;i<ncuts;i++){
    m_cw[i]->Save();
    if(i<NCUTS && GetCutPars(i)!=0 &&
		(defitem->cp[i].min!=GetCutPars(i)->min
       || defitem->cp[i].max!=GetCutPars(i)->max ||
          defitem->cp[i].enabled!=GetCutPars(i)->enabled)) changed = true;
  }

  delete defitem;

  if(ClassList->currentItem()!=NULL && changed){
    ((CutClassItem*)ClassList->currentItem())->m_version=0;
    cutverLabel->setText("user-defined");
  }

  optClass *oc = ((TopWin*) parentWidget())->options->m_Opts;
  oc->m_CutIgnoreBad  = IgnoreBadPix->isChecked();
  Reset();
}

void CutPanel::SaveClose(){
  Save();
  accept();
}

CutParam* CutPanel::GetCutPars(int id){
  if(id<ncuts)
    return m_cw[id]->m_cp;

  CutParam *tmp = new CutParam;
  
  tmp->min = 0;
  tmp->max = 1;
  tmp->enabled = false;
  tmp->unit = "";
  tmp->label="rubbish";
  
  return tmp;
}

void CutPanel::selectedClass(Q3ListViewItem *item){
  int version = 0;
  if(item!=0){
    RenameEdit->setText(item->text(0));
    CutClassLabel->setText(item->text(0));
    CutClassItem* cci = (CutClassItem*)ClassList->currentItem();
    if(cci!=0){
      if((version=cci->GetVersion())>0)
	cutverLabel->setText(QString::number(version));
      else
	cutverLabel->setText("user-defined");
    }
  }
  for(int i=0;i<ncuts;i++){
    if(item!=0){
      m_cw[i]->LoadCuts(&((CutClassItem*)item)->cp[i]);
      m_cw[i]->Enabled->setEnabled((version>4 || i!=10));
    } else
      m_cw[i]->LoadCuts(0);
  }
  return;
}
void CutPanel::NewClass(){
  MAEngine::defineSpecialCuts(*(this), cutdefBox->currentItem());
//new CutClassItem(ClassList,"Default cuts");
//   ClassList->setCurrentItem(item);
//   selectedClass(item);
//   if(ClassList->currentItem()!=NULL){
//     CutClassLabel->setText(ClassList->currentItem()->text(0));
//     cutverLabel->setText(QString::number(((CutClassItem*)ClassList->currentItem())->GetVersion()));
//   }
  return;
}
void CutPanel::RenameClass(){
  ClassList->currentItem()->setText(0,RenameEdit->text());
  if(ClassList->currentItem()!=NULL) CutClassLabel->setText(ClassList->currentItem()->text(0));
  return;
}
void CutPanel::DeleteClass(){
  delete ClassList->currentItem();
  ClassList->setCurrentItem(ClassList->firstChild());
  selectedClass(ClassList->firstChild());
}
void CutPanel::LoadClass(){
  QString path = QString::null;
  if(((TopWin*) parentWidget())->options->m_Opts->m_DefCutPaths!=".")
    path = ((TopWin*) parentWidget())->options->m_Opts->m_DefCutPaths.c_str();
  Q3FileDialog fdia(path, "Cut file (*.cut)",this,"select data file",TRUE);
  if(fdia.exec() == QDialog::Accepted)
    LoadClass(fdia.selectedFile().latin1());
}

int CutPanel::LoadClass(const char *path){
  char tmpstr[2000];
  std::string intxt;
  int ok, i, lines=0, cutver=0;
  const int nline_max=1000;
  FILE *in = fopen(path,"r");
  if(in==NULL) return -2;
  fgets(tmpstr,1999,in);
  tmpstr[strlen(tmpstr)-1] = '\0';
  intxt = tmpstr;
  //  bool oldver=false;
  if(intxt.find("#MA VERSION")== std::string::npos){
    QString mymsg = "Cut file ";
    mymsg += path;
    mymsg += "\nis from a too old MA version. Cannot load.";
    QMessageBox::warning(this,"Cut panel load",mymsg);
    return -1;
  }else{
    intxt.erase(0,12);
    if(DatSet::OlderVersion(intxt.c_str(),"3.0.0")){
      //      oldver = true;
      QString mymsg = "Cut file ";
      mymsg += path;
      mymsg += "\nis from a too old MA version. Cannot load.";
      //      		   "New entries were added.";
      QMessageBox::warning(this,"Cut panel load",mymsg);
      return -1;
    }
  }
  while(intxt.find("#LABEL")==std::string::npos && lines<nline_max){
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    if(intxt.find("#CUT VERSION")!=std::string::npos)
      sscanf(tmpstr,"#CUT VERSION %d",&cutver);
    lines++;
  }
  if(lines>=nline_max){
    QMessageBox::warning(this,"Cut panel load","Can't find required LABEL search pattern\nAbort");
    return -1;
  }
  fgets(tmpstr,1999,in);
  tmpstr[strlen(tmpstr)-1] = '\0';
  CutClassItem *item = new CutClassItem(ClassList,tmpstr);
  item->SetPath(path);
  item->m_version=cutver;
  CutParam mycp;
  intxt=" ";
  lines = 0;
  while(intxt.find("#PIXELCUTS")==std::string::npos && lines<nline_max){
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    lines++;
  }
  if(lines>=nline_max){
    QMessageBox::warning(this,"Cut panel load","Can't find required PIXEL search pattern\nAbort");
    delete item;
    return -3;
  }
  for(i=0;i<npcuts;i++){      
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    if(intxt.find("#")!=std::string::npos || intxt=="") break;
    mycp.label = tmpstr;
    fgets(tmpstr,1999,in);
    intxt = tmpstr;
    if(intxt.find("#")!=std::string::npos || intxt=="") break;
    sscanf(tmpstr,"%lf",&(mycp.min));
    fgets(tmpstr,1999,in);
    sscanf(tmpstr,"%lf", &(mycp.max));
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    mycp.unit = tmpstr;
    fgets(tmpstr,1999,in);
    sscanf(tmpstr,"%d", &ok);
    if(cutver<5 && i==10) ok = false; // must not use old threshold cut
    mycp.enabled = (bool) ok;
    item->cp[i] = mycp;
  }
  lines = 0;
  while(intxt.find("#MODULECUTS")==std::string::npos && lines<nline_max){
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    lines++;
  }
  if(lines>=nline_max){
    QMessageBox::warning(this,"Cut panel load","Can't find required MODULE search pattern\nAbort");
    delete item;
    return -3;
  }
  for(i=npcuts;i<nmcuts;i++){
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    //    if(intxt.find("VDDmin (digital OK)")!=std::string::npos || intxt=="") break;
    if((intxt.find("#")!=std::string::npos && 
	!intxt.find("#bad pix"))|| intxt=="") break;
    mycp.label = tmpstr;
    fgets(tmpstr,1999,in);
    intxt = tmpstr;
    if(intxt.find("#")!=std::string::npos || intxt=="") break;
    sscanf(tmpstr,"%lf",&(mycp.min));
    fgets(tmpstr,1999,in);
    sscanf(tmpstr,"%lf", &(mycp.max));
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    mycp.unit = tmpstr;
    fgets(tmpstr,1999,in);
    sscanf(tmpstr,"%d", &ok);
    mycp.enabled = (bool) ok;
    item->cp[i] = mycp;
  }
  lines = 0;
  while(intxt.find("#CHIPCUTS")==std::string::npos && lines<nline_max){
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    lines++;
  }
  /*
  if(lines>=nline_max){
    QMessageBox::warning(this,"Cut panel load","Can't find required search pattern\nAbort");
    delete item;
    return -3;
  }
  */
  for(i=nmcuts;i<ncuts;i++){
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    intxt = tmpstr;
    if(intxt.find("#")!=std::string::npos || intxt=="") break;
    mycp.label = tmpstr;
    fgets(tmpstr,1999,in);
    intxt = tmpstr;
    if(intxt.find("#")!=std::string::npos || intxt=="") break;
    sscanf(tmpstr,"%lf",&(mycp.min));
    fgets(tmpstr,1999,in);
    sscanf(tmpstr,"%lf", &(mycp.max));
    fgets(tmpstr,1999,in);
    tmpstr[strlen(tmpstr)-1] = '\0';
    mycp.unit = tmpstr;
    fgets(tmpstr,1999,in);
    sscanf(tmpstr,"%d", &ok);
    mycp.enabled = (bool) ok;
    item->cp[i] = mycp;
  }
  fclose(in);
  ClassList->setCurrentItem(item);
  selectedClass(item);
  if(ClassList->currentItem()!=NULL) CutClassLabel->setText(ClassList->currentItem()->text(0));
  return 0;
}
void CutPanel::SaveClass(const char *inpath){
  int i;
  QString outfile = inpath;
  if(outfile.isEmpty()){
    QString path = QString::null;
    if(((TopWin*) parentWidget())->options->m_Opts->m_DefCutPaths!=".")
      path = ((TopWin*) parentWidget())->options->m_Opts->m_DefCutPaths.c_str();
    Q3FileDialog fdia(path, "Cut file (*.cut)",this,"select data file",TRUE);
    fdia.setMode(Q3FileDialog::AnyFile);
    if(fdia.exec() == QDialog::Accepted)
      outfile = fdia.selectedFile();
  }
  if(outfile.isEmpty()) return;
  
  ((CutClassItem*)ClassList->currentItem())->SetPath(outfile.latin1());
  FILE *out = fopen(outfile.latin1(),"w");
  if(out==NULL) return;
  fprintf(out,"#MA VERSION %s\n\n",VERSION);
  fprintf(out,"#CUT VERSION %d\n\n",((CutClassItem*)ClassList->currentItem())->m_version);
  fprintf(out,"#LABEL\n");
  fprintf(out,"%s\n\n",ClassList->currentItem()->text(0).latin1());
  fprintf(out,"#PIXELCUTS\n");
  CutParam mycp;
  for(i=0;i<npcuts;i++){
    mycp  = ((CutClassItem*)ClassList->currentItem())->cp[i];
    fprintf(out,"%s\n%lf\n%lf\n%s\n%d\n",mycp.label.c_str(),mycp.min, mycp.max, mycp.unit.c_str(), (int)mycp.enabled);
  }
  fprintf(out,"\n#MODULECUTS\n");
  for(i=npcuts;i<nmcuts;i++){
    mycp  = ((CutClassItem*)ClassList->currentItem())->cp[i];
    fprintf(out,"%s\n%lf\n%lf\n%s\n%d\n",mycp.label.c_str(),mycp.min, mycp.max, mycp.unit.c_str(), (int)mycp.enabled);
  }
  fprintf(out,"\n#CHIPCUTS\n");
  for(i=nmcuts;i<ncuts;i++){
    mycp  = ((CutClassItem*)ClassList->currentItem())->cp[i];
    fprintf(out,"%s\n%lf\n%lf\n%s\n%d\n",mycp.label.c_str(),mycp.min, mycp.max, mycp.unit.c_str(), (int)mycp.enabled);
  }
  fprintf(out,"\n#END\n");
  fclose(out);

  return;
}
void CutPanel::hideCCTab(bool doShow){
  TabWidg->setTabEnabled(tab_5,doShow);
}

