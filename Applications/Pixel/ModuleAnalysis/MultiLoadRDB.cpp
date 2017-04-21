#include "MultiLoadRDB.h"
#include "LogClasses.h"
#include "StdTestWin.h"

#include <q3filedialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qcheckbox.h> 
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <PixConfDBInterface.h>
#include <RootDB.h>

using namespace PixLib;

MultiLoadRDB::MultiLoadRDB(QWidget* parent, const char* , bool , Qt::WFlags fl, const char *path)
  : QDialog(parent, fl)
//  : MultiLoadRDBBase(parent, name, modal, fl)
{
  setupUi(this);
  QObject::connect(OKButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(BrowseButton, SIGNAL(clicked()), this, SLOT(browseFile()));
  QObject::connect(plotsBrwsButton, SIGNAL(clicked()), this, SLOT(browsePlots()));

  if(path!=0)
    m_path = path;
  else
    m_path = QString::null;
}
MultiLoadRDB::~MultiLoadRDB()
{
}
void MultiLoadRDB::browseFile()
{
  QStringList filter("RootDB data file (*.root)");
  filter += "Any file (*.*)";
  Q3FileDialog fdia(m_path, "Any file (*.*)",this,"select data file",TRUE);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted){
    fileName->setText(fdia.selectedFile());
    QApplication::setOverrideCursor(Qt::waitCursor);
    try{
      loadFile();
    }catch(...){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"ModuleAnalysis Error","Error opening/reading file "+fileName->text());
      return;
    }
    QApplication::restoreOverrideCursor();
    OKButton->setEnabled(true);
  }
}
void MultiLoadRDB::browsePlots()
{
  QString path = Q3FileDialog::getExistingDirectory(plotsPath->text(),this,"MultiLoadRDB::browsePlots",
						   "select output dir",false);
  if(path!="")
    plotsPath->setText(path);
}
void MultiLoadRDB::loadFile()
{

  int i;

  DBInquire *root;
  recordIterator baseIter, grpIter, modIter;

  std::vector<std::string> modNames;
  std::vector<int> stdIDs;

  RootDB *db  = new RootDB(fileName->text().latin1());

  root = db->readRootRecord(1);
  if(root==0){
    QMessageBox::warning(this,"ModuleAnalysis Error","Error opening/reading file "+fileName->text());
    return;
  }

  i=0;
  tabWidget->removePage(tab1);
  delete tab1;
  tab1 = new QWidget( tabWidget, "tab1" );
  Q3VBoxLayout *tabLayout = new Q3VBoxLayout( tab1, 11, 6, "tabLayout"); 
  Q3VBoxLayout *layoutScan = new Q3VBoxLayout( 0, 0, 6, "layoutScan"); 
  textLabel2 = new QLabel( tab1, "textLabel2" );
  textLabel2->setText( tr( "Select the associated type of scan for each data set:" ) );
  QSpacerItem *tspacer = new QSpacerItem( 51, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Q3HBoxLayout *tlayout = new Q3HBoxLayout( 0, 0, 6, "tlayout"); 
  QCheckBox *tcb = new QCheckBox("show TurboDAQ file names", tab1,"tcb"); 
  tcb->setChecked(true);
  connect(tcb,SIGNAL(toggled(bool)), this, SLOT(fillCombos(bool)));
  tlayout->addWidget( textLabel2 );
  tlayout->addItem( tspacer );
  tlayout->addWidget( tcb );
  layoutScan->addLayout(tlayout);
  for(baseIter = root->recordBegin(); baseIter!=root->recordEnd(); baseIter++){
    if((*baseIter)->getName()=="PixScanResult"){
      int stdID = -1;
      fieldIterator stdField = (*baseIter)->findField("StdTestID");
      if(stdField!=(*baseIter)->fieldEnd()){
	db->DBProcess(stdField,READ,stdID);
      }
      stdIDs.push_back(stdID);
      std::string name = TLogFile::GetNameFromPath((*baseIter)->getDecName().c_str());
      //printf("scan %s\n",name.c_str());
      QLabel *textLabel = new QLabel( tab1, "scanLabel" +QString::number(i));
      textLabel->setText(name.c_str());
      QSpacerItem *spacer = new QSpacerItem( 81, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
      QComboBox *box = new QComboBox( tab1, "scanBox"+QString::number(i));
      box->setMaximumWidth(400);
      Q3HBoxLayout *layout = new Q3HBoxLayout( 0, 0, 6, "layouts"+QString::number(i)); 
      layout->addWidget( textLabel );
      layout->addItem( spacer );
      layout->addWidget( box );
      layoutScan->addLayout(layout);
      for(grpIter = (*baseIter)->recordBegin(); grpIter!=(*baseIter)->recordEnd(); grpIter++){
	if((*grpIter)->getName()==("PixModuleGroup")){
	  for(modIter = (*grpIter)->recordBegin(); modIter!=(*grpIter)->recordEnd(); modIter++){
	    if((*modIter)->getName()=="PixModule"){
	      bool gotModule=false;
	      name = TLogFile::GetNameFromPath((*grpIter)->getDecName().c_str()) + "/" + 
		TLogFile::GetNameFromPath((*modIter)->getDecName().c_str());
	      for(std::vector<std::string>::iterator IT = modNames.begin(); IT!=modNames.end(); IT++){
		if((*IT)==name){
		  gotModule = true;
		  break;
		}
	      }
	      if(!gotModule) modNames.push_back(name);
	    }
	  }
	}
      }
      i++;
    }
  }
  fillCombos(tcb->isChecked(), stdIDs);
  QSpacerItem *spacerScan = new QSpacerItem( 51, 371, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layoutScan->addItem( spacerScan );
  tabLayout->addLayout( layoutScan );
  tabWidget->insertTab( tab1, "Scans");

  tabWidget->removePage(tab2);
  delete tab2;
  tab2 = new QWidget( tabWidget, "tab2" );
  Q3VBoxLayout *tabLayout_2 = new Q3VBoxLayout( tab2, 11, 6, "tabLayout_2"); 
  Q3VBoxLayout *layoutMod = new Q3VBoxLayout( 0, 0, 6, "layoutScan"); 
  textLabel3 = new QLabel( tab2, "textLabel3" );
  layoutMod->addWidget( textLabel3 );
  textLabel3->setText( tr( "Select modules to be analysed" ) );
  i=0;
  for(std::vector<std::string>::iterator IT = modNames.begin(); IT!=modNames.end(); IT++){
    //    printf("module %s\n",(*IT).c_str());
    QCheckBox *cb = new QCheckBox((*IT).c_str(), tab2,"modcb"+QString::number(i)); 
    cb->setChecked(true);
    layoutMod->addWidget(cb);
    i++;
  }
  layout5 = new QWidget(tab1);
  layout5->setObjectName(QString::fromUtf8("layout5"));
  layout5->setGeometry(QRect(12, 487, 462, 36));
  hboxLayout3 = new QHBoxLayout(layout5);
  hboxLayout3->setSpacing(6);
  hboxLayout3->setMargin(11);
  hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
  hboxLayout3->setContentsMargins(0, 0, 0, 0);
  allButton = new QPushButton(layout5);
  allButton->setObjectName(QString::fromUtf8("allButton"));
  hboxLayout3->addWidget(allButton);
  spacer2 = new QSpacerItem(81, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);
  hboxLayout3->addItem(spacer2);
  noneButton = new QPushButton(layout5);
  noneButton->setObjectName(QString::fromUtf8("noneButton"));
  layoutMod->addWidget( layout5 );
  allButton->setText( tr( "select all" ) );
  noneButton->setText( tr( "unselect all" ) );
  connect( allButton, SIGNAL( clicked() ), this, SLOT( selAll() ) );
  connect( noneButton, SIGNAL( clicked() ), this, SLOT( selNone() ) );
  QSpacerItem *spacerMod = new QSpacerItem( 51, 371, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layoutMod->addItem( spacerMod );
  tabLayout_2->addLayout( layoutMod );
  tabWidget->insertTab( tab2, "Modules");
  tabWidget->setCurrentPage(1);

  delete db;
}
void MultiLoadRDB::sel(bool issel)
{
  for(int i=0;i<10000;i++){
    QString label = "modcb"+QString::number(i);
    QCheckBox *cb = (QCheckBox*)tab2->child(label.latin1(),0,false);
    if(cb==0) break;
    cb->setChecked(issel);
  }
}
void MultiLoadRDB::getMods(std::vector<std::string> &mods)
{
  for(int i=0;i<10000;i++){
    QString label = "modcb"+QString::number(i);
    QCheckBox *cb = (QCheckBox*)tab2->child(label.latin1(),0,false);
    if(cb==0) break;
    if(cb->isChecked())
      mods.push_back(cb->text().latin1());
  }
}
void MultiLoadRDB::getScans(std::map<std::string, int> &scans)
{
  for(int i=0;i<10000;i++){
    QString label = "scanLabel" +QString::number(i);
    QLabel *textLabel = (QLabel*)tab1->child(label.latin1(),0,false);
    label = "scanBox" +QString::number(i);
    QComboBox *cb = (QComboBox*)tab1->child(label.latin1(),0,false);
    if(cb==0 || textLabel==0) break;
    if(cb->currentItem()>0){ // scan was selected
      scans.insert(std::make_pair(textLabel->text().latin1(), cb->currentItem()-1));
    }
  }
}
void MultiLoadRDB::fillCombos(bool mode)
{
  std::vector<int> stdIDs;
  fillCombos(mode, stdIDs);
}
void MultiLoadRDB::fillCombos(bool mode, std::vector<int> stdIDs)
{
  StdTestWin stdwin(0,"Load Std Files", TRUE, Qt::WType_TopLevel);
  stdwin.TestTypeSel->setCurrentItem(11); // set to "SYST"
  stdwin.TypeSet();

  for(int i=0;i<10000;i++){
    QString label = "scanLabel" +QString::number(i);
    QLabel *textLabel = (QLabel*)tab1->child(label.latin1(),0,false);
    label = "scanBox" +QString::number(i);
    QComboBox *cb = (QComboBox*)tab1->child(label.latin1(),0,false);
    if(cb==0 || textLabel==0) break;
    int cID = cb->currentItem();
    cb->clear();
    cb->insertItem("do not use");
    for(int j=0;j<stdwin.NSLOTS;j++){
      if(mode)
	cb->insertItem(stdwin.m_items[j]->m_defname);
      else
	cb->insertItem(stdwin.m_items[j]->m_label->text());
    }
    if(i<(int)stdIDs.size()){
      if(stdIDs[i]>=0)
	cb->setCurrentItem(stdIDs[i]+1);
    } else{
      cb->setCurrentItem(cID);
    }
  }
}
