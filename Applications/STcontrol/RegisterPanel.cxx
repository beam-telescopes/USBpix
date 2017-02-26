#include <PixController/PixScan.h>
#include <Config/Config.h>
#include <GeneralDBfunctions.h>

#include "RegisterPanel.h"
#include "STControlEngine.h"
#include "STRodCrate.h"
#include "OptionsPanel.h"

#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QLayout>
#include <QString>
#include <QColor>
#include <QLineEdit>
#include <QAction>
#include <QTabWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDateTime>
#include <QTextBrowser>
#include <QGroupBox>

RegisterItem::RegisterItem( QWidget* parent, QString name, Qt::WindowFlags fl) : 
  QWidget(parent,fl){
  m_layout = new QHBoxLayout(); ;
  m_label  = new QLabel( parent );
  m_label->setText( name);
  m_layout->addWidget( m_label );
  QSpacerItem* spacer = new QSpacerItem( 10, 2, QSizePolicy::Expanding, QSizePolicy::Expanding );
  m_layout->addItem( spacer );
  m_button = new QPushButton( parent );
  m_button->setEnabled( false );
  m_button->setText( trUtf8( "details" ) );
  m_layout->addWidget( m_button );
  // no text initially
  m_infoText = "";
  // connect details button with the text report
  connect( m_button, SIGNAL( clicked() ), this, SLOT( detailsPressed() ) );  
}

RegisterItem::~RegisterItem(){
}

void RegisterItem::detailsPressed(){
  //QMessageBox::information(this,"Register test info",m_infoText);
  QDialog *infoW = new QDialog(this);
  infoW->setWindowTitle("Register test info");
  infoW->setMinimumSize(300,300);
  QTextBrowser *txtB = new QTextBrowser(infoW);
  QVBoxLayout *generalLayout = new QVBoxLayout(infoW); 
  generalLayout->addWidget(txtB);
  //QSpacerItem* spacer = new QSpacerItem( 2, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );
  //generalLayout->addItem( spacer );
  QPushButton *pb = new QPushButton("OK", infoW);
  generalLayout->addWidget(pb);
  connect( pb, SIGNAL( clicked() ), infoW, SLOT( accept() ) );  
  // format and set text content
  QTextCharFormat txtF(txtB->currentCharFormat());
  txtF.setFontFixedPitch(true);
  txtB->setCurrentCharFormat(txtF);
  txtB->insertPlainText(m_infoText);
  infoW->exec();
}
void RegisterItem::setStatus(StatusTag status, QString infoText){
  m_infoText = infoText;
  switch(status){
  case tunknown:  // no info
  default:
    if(m_button->styleSheet()!=QString::fromUtf8("background-color: rgb(150, 150, 150);"))
      m_button->setStyleSheet(QString::fromUtf8("background-color: rgb(150, 150, 150);"));
    m_button->setEnabled(false);
    break;
  case tOK:  // all OK
    if(m_button->styleSheet()!=QString::fromUtf8("background-color: rgb(0, 255, 0);"))
      m_button->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));
    m_button->setEnabled(true);
    break;
  case tfailed:  // error
    if(m_button->styleSheet()!=QString::fromUtf8("background-color: rgb(255, 0, 0);"))
      m_button->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    m_button->setEnabled(true);
    break;
  }
}
void RegisterItem::hide(){
  m_button->hide();
  m_label->hide();
}
void RegisterItem::show(){
  m_button->show();
  m_label->show();
}

RegisterPanel::RegisterPanel( STControlEngine &engine_in, QWidget* parent, Qt::WindowFlags fl) :
  QWidget(parent,fl), m_engine(engine_in), m_unblocked(true){

  setupUi(this);
  QObject::connect(actionPerform_all_tests, SIGNAL(triggered()), this, SLOT(testAll()));
  QObject::connect(actionPerform_all_tests_current, SIGNAL(triggered()), this, SLOT(testCurrent()));
  QObject::connect(clearAction, SIGNAL(triggered()), this, SLOT(clearAll()));

  Config &cfg = m_engine.chipTestCfg();
  for(int i=0; i<cfg.subConfigSize(); i++){
    QAction *act = new QAction(this);
    act->setObjectName("action_"+QString(cfg.subConfig(i).name().c_str()));
    act->setText(QString(cfg.subConfig(i).name().c_str()));
    m_allActions.push_back(act);
    popupMenu->addAction(act);
    switch(i){
    case 0:
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(testList0()));
      break;
    case 1:
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(testList1()));
      break;
    case 2:
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(testList2()));
      break;
    default:
      break;
    }
    act = new QAction(this);
    act->setObjectName("action_current_"+QString(cfg.subConfig(i).name().c_str()));
    act->setText(QString(cfg.subConfig(i).name().c_str()));
    m_currActions.push_back(act);
    popupMenu_8->addAction(act);
    switch(i){
    case 0:
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(testListCurrent0()));
      break;
    case 1:
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(testListCurrent1()));
      break;
    case 2:
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(testListCurrent2()));
      break;
    default:
      break;
    }
  }

  QVBoxLayout *generalLayout = new QVBoxLayout(mainTabs->widget(0)); 
  // add config. editor
  QLabel *edl = new QLabel("Configuration of Chip Tests:", mainTabs->widget(0));
  generalLayout->addWidget(edl);
  optionsPanel *op = new optionsPanel(cfg,this, 0, true, true, true, true);
  generalLayout->addWidget(op);
  QSpacerItem* Spacer23 = new QSpacerItem( 5, 2, QSizePolicy::Expanding, QSizePolicy::Expanding );
  generalLayout->addItem( Spacer23 );
  // add result buttons
  QGroupBox *grpbx = new QGroupBox("Chip Test results:", mainTabs->widget(0));
  QVBoxLayout *topLayout = new QVBoxLayout(grpbx); 
  m_regMod = new RegisterItem(grpbx,"Module",0);
  topLayout->addLayout(m_regMod->m_layout);
  m_regMCC = new RegisterItem(grpbx,"MCC",0);
  topLayout->addLayout(m_regMCC->m_layout);
  m_regMCC->hide(); // only show if MCC present
  QHBoxLayout *boxLayout = new QHBoxLayout();//topLayout); 
  QVBoxLayout *leftLayout = new QVBoxLayout();//boxLayout); 
  QSpacerItem* Spacer24 = new QSpacerItem( 5, 2, QSizePolicy::Expanding, QSizePolicy::Expanding );
  QVBoxLayout *rightLayout = new QVBoxLayout();//boxLayout); 
  for(unsigned int i=0;i<16;i++)
    m_regFE.push_back(new RegisterItem(grpbx,"FE"+QString::number(i),0));
  for(unsigned int i=0; i<m_regFE.size();i++){
    if((i%2)==0)
      leftLayout->addLayout(m_regFE[i]->m_layout);
    else
      rightLayout->addLayout(m_regFE[i]->m_layout);
    if(i>0) m_regFE[i]->hide(); // always keep FE 0 visible
  }
  boxLayout->addLayout(leftLayout);
  boxLayout->addItem( Spacer24 );
  boxLayout->addLayout(rightLayout);
  topLayout->addLayout(boxLayout);
  generalLayout->addWidget(grpbx);

  // update function for text panel
  connect( &m_engine, SIGNAL(sendRTStatus(const char*)), this, SLOT(setTextMessg(const char *)) );
  connect( &m_engine, SIGNAL(configChanged()), this, SLOT(setActionsEnabled()) );
  // en-/dis-able actions when scans are run
  connect( &m_engine, SIGNAL(finishedScanning()), this, SLOT(setActionsEnabled()) );
  connect( &m_engine, SIGNAL(beganScanning()), this, SLOT(setActionsDisabled()));

  // no current module set 
  m_crateID = -1;
  m_grpID = -1;
  m_modID = -1;
  // clear form
  clearInfo();
  setActions(true);
}
RegisterPanel::~RegisterPanel(){
}
void RegisterPanel::setTextMessg(const char *msg){
  m_messageBox->setText(msg);
}
void RegisterPanel::clearAll(){
  m_engine.clearModuleInfo();
  clearInfo();
}
void RegisterPanel::clearInfo(){
  m_regMod->setStatus(tunknown);
  m_regMCC->setStatus(tunknown);
  for(unsigned int i=0; i<m_regFE.size();i++)
    m_regFE[i]->setStatus(tunknown);
}
void RegisterPanel::setModule(int crateID, int grpID, int modID, const char *name)
{
  try{
  if(crateID>=0){// || grpID<0){
    m_crateID = crateID;
    m_grpID = grpID;
    m_modID = modID;
  }
  bool isen =   actionPerform_all_tests->isEnabled();
  STRodCrate *crate = 0;
  if(m_crateID>=0 && m_crateID<(int)m_engine.getSTRodCrates().size())
    crate = m_engine.getSTRodCrates()[m_crateID];
  std::string myName="";
  if(crate!=0 && m_grpID>=0 && m_modID>=0)
    myName = crate->getModName(m_grpID, m_modID);
  if(myName!=""){
    getDecNameCore(myName);
    // update status info
    if(name==0) myName = "current module: "+myName;
    else        myName = name;
    moduleLabel->setText(myName.c_str());
    actionPerform_all_tests_current->setEnabled( isen  && m_crateID>=0 );
    for(std::vector<QAction*>::iterator it=m_currActions.begin(); it!=m_currActions.end(); it++)
      (*it)->setEnabled( isen  && m_crateID>=0 );
    modStatus modS = crate->getPixModuleStatus(m_grpID, m_modID);
    m_regMod->setStatus(modS.modStat, "Module:\n"+QString(modS.modMsg.c_str()));
    if(modS.mccStat==tblocked){
      m_regMCC->hide();
    }else{
      m_regMCC->show();
      m_regMCC->setStatus(modS.mccStat, "Link/MCC:\n"+QString(modS.mccMsg.c_str()));
    }
    for(unsigned int i=0; i<m_regFE.size();i++){
      if(modS.feStat[i]==tblocked){
	m_regFE[i]->hide();
      }else{
	m_regFE[i]->show();
	m_regFE[i]->setStatus(modS.feStat[i], "FE "+QString::number(i)+"\n"+ QString(modS.feMsg[i].c_str()));
      }
    }
  }else if(m_unblocked){
    moduleLabel->setText("no module selected");
    clearInfo();
    actionPerform_all_tests_current->setEnabled( false );
    for(std::vector<QAction*>::iterator it=m_currActions.begin(); it!=m_currActions.end(); it++)
      (*it)->setEnabled( false );
  }
  }catch(...){
    QMessageBox::warning(this,"test panel","unexpected exception: "+QString::number(m_crateID)+" "+QString::number(m_grpID)
			 +" "+QString::number(m_modID));
  }
  return;
}

void RegisterPanel::testCurrent()
{
  for(int i=0; i<(int)m_allActions.size(); i++)
    testList(i, true);
}
void RegisterPanel::testAll()
{
  for(int i=0; i<(int)m_allActions.size(); i++)
    testList(i, false);
}
void RegisterPanel::setActions(bool isen)
{
  actionPerform_all_tests_current->setEnabled( isen && m_crateID>=0 );
  for(std::vector<QAction*>::iterator it=m_currActions.begin(); it!=m_currActions.end(); it++)
    (*it)->setEnabled( isen && m_crateID>=0 );
  actionPerform_all_tests->setEnabled( isen );
  for(std::vector<QAction*>::iterator it=m_allActions.begin(); it!=m_allActions.end(); it++)
    (*it)->setEnabled( isen );
  clearAction->setEnabled( isen );
  return;
}
void RegisterPanel::setLED(QLabel *label, int state)
{
  if(label==0) return;
  QPalette palette;
  if(label->isEnabled()){
    switch(state){
    case 0:
      //      label->setBackgroundColor(Qt::red);
      palette.setColor(label->backgroundRole(), QColor::fromRgb(255,0,0,255));
      break;
    case 1:
      //      label->setBackgroundColor(Qt::green);
      palette.setColor(label->backgroundRole(), QColor::fromRgb(0,255,0,255));
      break;
    case -1:
    default:
      //      label->setBackgroundColor(Qt::gray);
      palette.setColor(label->backgroundRole(), QColor::fromRgb(150,150,150,255));
    }
  } else
    //    label->setBackgroundColor(Qt::gray);
    palette.setColor(label->backgroundRole(), QColor::fromRgb(150,150,150,255));
  label->setPalette(palette);

}
void RegisterPanel::testList(int type, bool current){
  if((unsigned int)type >= m_allActions.size()){
    m_messageBox->setText("unknown test type");
    return;
  }
  setActions(false);
  m_messageBox->setText("started FE test "+m_allActions[type]->text());
  int crateID = m_crateID;
  int grpID =	m_grpID;
  int modID =	m_modID;
  QString label = moduleLabel->text();
  if(current)
    m_engine.runChipTest(m_allActions[type]->text().toLatin1().data(), crateID, grpID, modID);
  else
    m_engine.runChipTest(m_allActions[type]->text().toLatin1().data());
  if(m_crateID>=0)
    setModule(crateID, grpID, modID, label.toLatin1().data());
  setActions(true);
}
