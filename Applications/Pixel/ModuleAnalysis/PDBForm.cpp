#include "RootStuff.h"
#include "PDBForm.h"
#include "LogClasses.h"
#include "LoginPanel.h"
#include "QAReport.h"
#include "CutPanel.h"
#include "OptWin.h"
#include "OptClass.h"
#include "StdTestWin.h"

#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <q3multilineedit.h>
#include <q3filedialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qcheckbox.h>
#include <q3process.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <q3groupbox.h>
#include <qregexp.h>
#include <q3textbrowser.h>
#include <q3table.h>
#include <qapplication.h>
#include <qcolor.h>
#include <qpushbutton.h>

#include <stdio.h>
#include <string>

#include <TGraph.h>
#include <TSystem.h>

PDBForm::PDBForm( QWidget* parent, const char* , bool , Qt::WFlags fl, ModItem *item, 
                  QApplication *app, const char *defpath)
  : QDialog(parent,fl), m_app(app){
  //  : PDBFormBase(parent,name,modal,fl), m_app(app){

  setupUi(this);
  QObject::connect(bareBow, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(BrowseButton, SIGNAL(clicked()), this, SLOT(Browse()));
  QObject::connect(BrowseRoot, SIGNAL(clicked()), this, SLOT(LoadRootPath()));
  QObject::connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(CreateButton, SIGNAL(clicked()), this, SLOT(CreatePDB()));
  QObject::connect(deltaTMCC, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(flexBow, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(moduleMass, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(nonstFEWire, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(nonstMCCWire, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(numMarks, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(PathBrowse, SIGNAL(clicked()), this, SLOT(BrowseOutput()));
  QObject::connect(pdbCombo, SIGNAL(activated(QString)), this, SLOT(LoadProfile()));
  QObject::connect(pdbMeasType, SIGNAL(activated(int)), this, SLOT(checkType()));
  QObject::connect(pdbRun, SIGNAL(valueChanged(int)), this, SLOT(Index()));
  QObject::connect(pullFEmax, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(pullFEmin, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(pullMCCmax, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(pullMCCmin, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(pullASSY, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(repFEWire, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(repMCCWire, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(SaveButton, SIGNAL(clicked()), this, SLOT(SaveProfile()));
  QObject::connect(userPenalty, SIGNAL(valueChanged(int)), this, SLOT(checkMech()));
  QObject::connect(mechDate, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(mechComment, SIGNAL(textChanged()), this, SLOT(checkMech()));
  QObject::connect(surveyBox, SIGNAL(toggled(bool)), this, SLOT(checkMech()));
  QObject::connect(thermCondCold, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  QObject::connect(thermCondWarm, SIGNAL(textChanged(QString)), this, SLOT(checkMech()));
  
  m_modItem = item;
  // lock module item so it can't be deleted
  if(m_modItem!=0)
    m_modItem->m_lock=true;

  modsn = QString::null;

  ModItem *child;
  
  m_User = QString::null;
  m_Passwd = QString::null;

  m_path = QString::null;
  if(defpath!=0) m_path = defpath;
  
  m_allBadPixs=0; 
  m_srcBadPixs=0; 
  m_srcMasked =0;
  m_digBadPixs=0; 
  m_anaBadPixs=0; 
  m_discBadPixs=0;
  m_mlBadPixs=0; 
  m_xtBadPixs=0; 
  m_intBadPixs=0; 
  m_totBadPixs=0;
  m_NTC = -999;
  m_IV_T = -999;

  IVitem  = 0;
  DIGitem = 0;
  Curritem= 0;
  ANAitem = 0;
  SRCitem = 0;
  AMBitem = 0;
  m_uploadMech = false;
  
  m_cutVersioned = true;
  m_uploadJava = false;
  m_uploadPASS = false;

  m_parent = (TopWin*)parent;
  if(m_parent!=0){
    savePlots->setChecked(m_parent->options->m_Opts->m_pdbPlots);
    removeFiles->setChecked(m_parent->options->m_Opts->m_pdbRemove);
  }
  // create QAReport object, needed for analogue/source analysis
  if(m_modItem!=0 && m_parent!=0){
    QString mod_path = m_modItem->m_lastpath;
    m_modItem->m_lastpath = QString::null;
    m_rep = new QAReport(m_modItem, m_parent, "QAReport", TRUE, Qt::WType_TopLevel, 0, app);
    m_rep->ReportText = ResultWindow; // re-directs to this text window
    TabWidget->addTab(m_rep->TabWidget->page(1),"Qualif. table"); // add tab with qualif. form table
    m_modItem->m_lastpath = mod_path;
    connect( ResultWindow, SIGNAL( linkClicked(const QString&) ), m_rep, SLOT(processLink(const QString&)));

  } else
    m_rep = 0;
  // load exisiting meas. types
  StdTestWin tmpwin(this,"tmpstdwin");
  int i;
  bool havetype[NTYPES];
  for(i=0;i<NTYPES;i++){
    havetype[i] = false;
    if(i>0)
      pdbMeasType->insertItem(tmpwin.prefix[i]);
  }

  child = 0;
  if(m_modItem!=0)
    child=(ModItem*)m_modItem->firstChild();
  while(child!=0){
    int mt = child->GetDID()->GetStdtest()/100;
    if(mt>0 && !havetype[mt]){
      pdbMeasType->setCurrentItem(mt);
      havetype[mt] = true;
    }
    child=(ModItem*)child->nextSibling();
  }
//   if(pdbMeasType->count()>0){
//     pdbMeasType->setCurrentItem(pdbMeasType->count()-1);
//   }
  
  retValue = IcheckType();

  // set date in mechanical form
  QString date, time;
  ModItem *MDitem=DIGitem;
  if(MDitem==0)
    MDitem = ANAitem;
  if(MDitem==0)
    MDitem = SRCitem;
  if(MDitem==0)
    MDitem = IVitem;
  if(MDitem==0)
    MDitem = Curritem;
  if(MDitem!=0){
    TopWin::DateFromLog(MDitem->GetDID()->GetLog()->comments.c_str(), date, time);
    mechDate->setText(Date(date));
  }

  // check java environment
  m_uploadPASS = false;
  m_uploadJava = false;
  Q3Process find_java(this);
  find_java.addArgument("java");
  find_java.addArgument("-version");
  if(find_java.start()){
    while(find_java.isRunning()){}; // wait for cmd to finish
    int java_status = find_java.exitStatus();
    //printf("java execution yields %d\n",java_status);
    if(!java_status){
      // let's check then if CLASSPATH has been set correctly and the
      // necessary files are there
      QString ClassPath = gSystem->Getenv("CLASSPATH");
      int pos1 = ClassPath.find("classes111.zip");
      int pos2 = ClassPath.find("PIXTEST_1_02_P.jar");
      int pos3 = ClassPath.find("MODULE.jar");
      if(pos1>=0 && pos2>=0 && pos3>=0){
	m_uploadJava = true;
	if(ClassPath.find("ADMIN.jar")>=0)
	  m_uploadPASS = true;
      } //else
	//printf("bad CLASSPATH: %d %d %d\n",pos1,pos2,pos3);
    }
  }
  if(!m_uploadJava)
    PDBJavaBox->setTitle("PDB files - NO JAVA-INSTALLATION FOUND");

  // check ssh environment
  m_uploadWeb = 0;
  m_pauseAfterWWW = true;
  
  Q3Process find_ssh(this);
  find_ssh.addArgument("ssh");
  find_ssh.addArgument("-h");
  if(find_ssh.start()){
    while(find_ssh.isRunning()){}; // wait for cmd to finish
    if(find_ssh.exitStatus()==0 || find_ssh.exitStatus()==1){
      m_uploadWeb = 1;
    }
  }
  if(!m_uploadWeb){
    find_ssh.clearArguments();
    find_ssh.addArgument("ssh2");
    find_ssh.addArgument("-h");
    if(find_ssh.start()){
      while(find_ssh.isRunning()){}; // wait for cmd to finish
      if(find_ssh.exitStatus()==0 || find_ssh.exitStatus()==1){
	m_uploadWeb = 2;
      }
    }
  }

  // the following only makes sense when applied to data
  if(m_modItem!=0){
    if(retValue){ // no reasonable electrical data -> allow mechanical upload at least
      CreateButton->setText("Upload");
      CreateButton->setEnabled(false);
      connect( TabWidget, SIGNAL( currentChanged(QWidget*) ), this, SLOT( enableUploadButt() ) );
      disconnect( CreateButton, SIGNAL( clicked() ), this, SLOT( CreatePDB() ) );
      if(m_uploadJava) // java environment set up correctly
        connect( CreateButton, SIGNAL( clicked() ), this, SLOT( launchJavaUpload() ) );
    }

  } else{
    CreateButton->hide();
    CancelButton->setText("Close");
    for(int i=1;i<TabWidget->count();i++)
      TabWidget->setTabEnabled(TabWidget->page(i),false);
  }
  // get temperature from IV logfile, check if its reasonable
  // (no temp. data means it's -1000, but TurboDAQ might write
  // rubbish in it; then write into panel
  m_IV_T = -999;
//   if(AMBitem!=0){
//     QString theDate, theTime;
//     const char* cm=IVitem->GetDID()->GetLog()->comments.c_str();        
//     TopWin::DateFromLog(cm, theDate, theTime);
//     double time=TopWin::BI_getTimeFromStrings(theDate, theTime);
//     m_IV_T = AMBitem->GetDID()->BI_getAvgValueOnInterval(BI_NTC,time+30, time+120); // range 0...2 min
//   }
  if(IVitem!=0 && IVitem->GetDID()->GetLog()->MyDCS->Tntc>-50)
    m_IV_T = IVitem->GetDID()->GetLog()->MyDCS->Tntc;
  if(m_IV_T>-999)
    pdbTemp->setText(QString::number(m_IV_T,'f',1));
  
  // load existing user profiles
  dir.setPath(m_parent->options->m_Opts->m_pdbpath.c_str());
  dir.setNameFilter("*.cfg");
  
  UserReset();
  
  LoadProfile();
  
  return;
}

PDBForm::~PDBForm(){
  // unlock item
  if(m_modItem!=0)
    m_modItem->m_lock=false;
  // we're not the parent of the QAReport, so must delete it explicitly
  delete m_rep;
}

QString PDBForm::Date(QString date){
  QString dd = date.mid(3,2);
  QString mm = date.left(2);
  QString yyyy = date.mid(6,4);
  
  return dd+"/"+mm+"/"+yyyy;
}

void PDBForm::UserReset(){

  int size = pdbCombo->count();

  for(int j=0; j<size; j++)
    pdbCombo->removeItem(0);
  
  for(uint i=0; i<dir.count(); i++)
    pdbCombo->insertItem(dir[i].left(dir[i].find(".",0)));

  if(dir.count()>0)
    pdbCombo->setCurrentItem(0);
  else{
    pdbUser->setText("");
    pdbLocation->setCurrentItem(0);
    pdbURL->setText("");
    pdbDesc->setText("");
    outputPath->setText("");
  }

  return;
}

void PDBForm::LoadProfile(){

  QString line = dir.path()+"/"+pdbCombo->currentText()+".cfg";

  QFile file(line);
  if(file.open(QIODevice::ReadOnly)){
    pdbUser->setText(pdbCombo->currentText());
    Q3TextStream stream( &file );
    while(!stream.eof()){
      line=stream.readLine();
      if(line.left(4)=="Loc:"){
        line.remove(0,5);
        for(int i=0;i<pdbLocation->count();i++){
          if(line==pdbLocation->text(i)) pdbLocation->setCurrentItem(i);
        }
      }
      if(line.left(4)=="URL:") pdbURL->setText(line.remove(0,5));
      if(line.left(4)=="Dsc:") pdbDesc->setText(line.remove(0,5));
      if(line.left(4)=="Out:") outputPath->setText(line.remove(0,5));
    }
    file.close();
  }
}

void PDBForm::SaveProfile(){
  QString profpath = dir.path();
  QString profLabel = pdbUser->displayText();
  profpath = Q3FileDialog::getExistingDirectory(profpath,this,"User Config Directory","Save Path",true);
  if(profpath.isEmpty()) return;
  m_parent->options->m_Opts->m_pdbpath = profpath.latin1();
  
  QFile file(profpath+"/"+profLabel+".cfg");
  if(file.open(QIODevice::WriteOnly)){
    Q3TextStream stream( &file );
    stream << "Loc: "+pdbLocation->currentText()+"\n";
    stream << "URL: "+pdbURL->displayText()+"\n";
    stream << "Dsc: "+pdbDesc->displayText()+"\n";
    stream << "Out: "+outputPath->text()+"\n";
    
    file.close();
    
    //pdbCombo->insertItem(pdbUser->displayText());
    
    dir.setPath(profpath);
    dir.setNameFilter("*.cfg");
    
    UserReset();
    for(int j=0;j<pdbCombo->count();j++){
      if(pdbCombo->text(j)==profLabel)
        pdbCombo->setCurrentItem(j);
    }
    LoadProfile();
  }
  
  return;
}

void PDBForm::Browse(){
  QString profpath = dir.path();
  profpath = Q3FileDialog::getExistingDirectory(profpath,this,"User Config Directory","Load Path",true);
  if(profpath!=QString::null){
    m_parent->options->m_Opts->m_pdbpath = profpath.latin1();
    dir.setPath(profpath);
    dir.setNameFilter("*.cfg");
    
    UserReset();
    LoadProfile();
  }
  return;
}

void PDBForm::BrowseOutput(){
  QString outpath = outputPath->text();
  QString myindex;
  if(pdbRun->value()<10)
    myindex="0"+pdbRun->text();
  else
    myindex=pdbRun->text();
  if(m_modItem!=0){
    outpath.replace("M%m","M"+m_modItem->text(0));
    outpath.replace("%m","M"+m_modItem->text(0));
  }
  if(modsn!=QString::null)
    outpath.replace("%s",modsn);
  outpath.replace("%t",pdbMeasType->currentText());
  outpath.replace("%i",myindex);
  if(outpath.isEmpty())
    outpath = dir.path();
  outpath = Q3FileDialog::getExistingDirectory(outpath,this,"User Config Directory","Load Path",true);
  if(modsn!=QString::null)
    outpath.replace(modsn,"%s");
  if(m_modItem!=0)
    outpath.replace("M"+m_modItem->text(0),"%m");
  outpath.replace(pdbMeasType->currentText(),"%t");
  outpath.replace("/"+myindex+"/","/%i/");
  outputPath->setText(outpath);
  return;
}

void PDBForm::checkType(){
  IcheckType();
}
int PDBForm::IcheckType(){
  StdTestWin tmpwin(this,"tmpstdwin");
  int i, loadID=5;
  for(i=0;i<NTYPES;i++)
    if(tmpwin.prefix[i]=="LOAD") loadID=i;

  // set mech. form according to type
  if(pdbMeasType->currentItem()<loadID)
    loadID = 0;
  else
    loadID = pdbMeasType->currentItem()-loadID+1;
  setMechForm(loadID);
  
  // store data item pointers for selected meas. type
  if(m_modItem==0) return 4;

  ModItem *child=(ModItem*)m_modItem->firstChild();
  while(child!=0){
    if((i=child->GetDID()->GetStdtest()%100)>=0 && 
       (pdbMeasType->currentItem()==0 || pdbMeasType->currentText()==tmpwin.prefix[child->GetDID()->GetStdtest()/100])){
      if(i==3)  IVitem=child;
      if(i==0)  DIGitem=child;
      if(i==20) Curritem=child;
      if(i==1&&ANAitem==0)  ANAitem=child; // use HVoff scan in the worst case
      if(i==6&&ANAitem==0)  ANAitem=child;
      if(i==5)  ANAitem=child; // use preferrably to above
      if(i==14) SRCitem=child;
      if(i==39) AMBitem=child;
    }
    child=(ModItem*)child->nextSibling();
  }

  // get SN, run ID and name meas. type - try to guess from name if not specified from data
  int pos;
  bool typeOK;
  ModItem *name_item = DIGitem;
  if(name_item==0)
    name_item = IVitem;
  if(name_item==0)
    name_item = ANAitem;
  if(name_item==0)
    name_item = SRCitem;
  if(name_item!=0){
    modsn=(QString)name_item->GetDID()->GetLog()->MyModule->modname.c_str();
    QString tmptype=((QString)name_item->GetDID()->GetPath());
    if(pdbMeasType->currentItem()>0){
      modtype = pdbMeasType->currentText();
      pos = tmptype.find(modtype);
      if(pos>0)
        tmptype.remove(0,pos);
    }else{
      pos = tmptype.find("_");
      if(pos>0)
        tmptype.remove(0,pos+1);
      modtype=tmptype.left(4);
      typeOK = false;
      for(pos=1;pos<NTYPES;pos++){
        if(modtype==tmpwin.prefix[pos]){
          typeOK = true;
          break;
        }
      }
      if(!typeOK)
        modtype=((QString)name_item->GetDID()->GetPath()).left(4);
    }
    tmptype.remove(0,4);
    pdbRun->setValue(tmptype.left(2).toInt(&typeOK));
  } else {
    modsn = m_modItem->text(0);
    modtype = "";
  }

  if(modsn.length()==8)          // bare module abbrev. SN
    modsn = "202102"+modsn;
  else if(modsn.length()==6)     // flex module abbrev. SN
    modsn = "20210020"+modsn;
  else if(modsn.length()!=14){   // need full SN if not abbreviated
    return 2;
  }
  if(modsn.left(6)=="202102")        // bare module
    urlpath="/pixprod/BareModule/"+modsn+"/"+modtype;
  else if(modsn.left(8)=="20210020") // flex module full SN
    urlpath="/pixprod/Module/"+modsn+"/"+modtype;
  else {                             // unknown object
    return 2;
  }

  // found any data?
  
  if(IVitem==0 && DIGitem==0 && Curritem==0 && ANAitem==0 && SRCitem==0){
    CreateButton->setEnabled(false);
    return 3;
  }
  
  // check for logfile
  if(IVitem !=0  && IVitem->GetDID()->GetLog()==0)  IVitem=0;
  if(DIGitem !=0 && DIGitem->GetDID()->GetLog()==0) DIGitem=0;
  if(ANAitem !=0 && ANAitem->GetDID()->GetLog()==0) ANAitem=0;
  if(SRCitem !=0 && SRCitem->GetDID()->GetLog()==0) SRCitem=0;
  if(IVitem==0 && DIGitem==0 && ANAitem==0 && SRCitem==0){
    CreateButton->setEnabled(false);
    return 1;
  }
  
  
  //  if(pdbMeasType->currentItem()>0 && modsn.left(8)=="20210020") // default comment for flex modules
  //    pdbComment->insertLine(pdbMeasType->currentText()+" test");

  Index();

  return 0;
}
void PDBForm::Index(){
  // set run index
  int runID=pdbRun->value()%100;
  if(runID<10)
    pdbURLPath->setText(urlpath+"/0"+QString::number(runID));
  else
    pdbURLPath->setText(urlpath+"/"+QString::number(runID));
  return;
}

void PDBForm::CreatePDB(){

  // checking mandatory fields
  QString warn_msg = QString::null;
  if(pdbUser->displayText().isEmpty())
    warn_msg += "PDB identity not specified.\n";
  if(pdbTemp->displayText().isEmpty() && IVitem!=0)
    warn_msg += "IV data exists but no Tsensor given.\n";
  if(outputPath->text().isEmpty())
    warn_msg += "No output path specified.";
  if(!warn_msg.isEmpty()){
    QMessageBox::warning(this,"Module Analysis","Mandatory data missing:\n"+warn_msg);
    return;
  }

  TabWidget->setCurrentPage(2);
  TabWidget->repaint();
  repaint();
  if(m_app!=0)
    m_app->processEvents();

  QString myindex;
  if(pdbRun->value()<10)
    myindex="0"+pdbRun->text();
  else
    myindex=pdbRun->text();

  if(outputPath->text().isEmpty())
    m_path = Q3FileDialog::getExistingDirectory(m_path,this,"Working PDB Directory","Save Path",true);
  else
    m_path = outputPath->text();

  if(m_modItem!=0){
    m_path.replace("M%m","M"+m_modItem->text(0));
    m_path.replace("%m","M"+m_modItem->text(0));
  }
  m_path.replace("%s",modsn);
  m_path.replace("%t",pdbMeasType->currentText());
  m_path.replace("%i",myindex);
  if(m_path.isEmpty()) return;
  // load into QAReport
  if(m_rep!=0)
    m_rep->LoadPath(m_path.latin1());
  
  // prepare cuts; create default if user hasn't done anything
  bool use_defcuts=false;
  m_selcutID=0;
  if(m_parent->m_cutpanel->ClassList->childCount()<=0){
    m_parent->m_cutpanel->NewClass();
    m_parent->m_cutpanel->RenameEdit->setText("Default cuts");
    m_parent->m_cutpanel->RenameClass();
    use_defcuts = true;
  } else{
    m_selcutID = ((CutClassItem*)m_parent->m_cutpanel->ClassList->currentItem())->itemID();
  }
  // run quality analysis
  if(m_rep!=0){
    m_rep->testTypeSel->setCurrentItem(pdbMeasType->currentItem());
    if(!savePlots->isChecked()){
      //    m_rep->FileName->setText(path);
      //  }else{
      m_rep->FileName->setText(""); // -> no "old-style" txt output, no plots
    }
    m_rep->StartReport();
  }
  /* txt file saving removed from QA report
  if(savePlots->isChecked()) // remove old-style txt output, only plot requested here
    remove((path+"/"+m_modItem->text(0)+"_QAReport.txt").latin1());
  */

  // using default cuts? then add version tag to run number
  int cv=0;
  m_cutVersioned=false;
  if((cv=((CutClassItem*)m_parent->m_cutpanel->ClassList->currentItem())->GetVersion())>1){
    m_cutVersioned=true;
    pdbRun->setMaxValue(999);
    pdbRun->setValue(cv*100+pdbRun->value());
  }
  // does not apply to bare modules:
  if(modsn.left(6)=="202102")
    m_cutVersioned=true;

  // remove old text files
  remove((m_path+"/PDBiv.txt").latin1());
  remove((m_path+"/PDBdig.txt").latin1());
  remove((m_path+"/BadDigital.txt").latin1());
  remove((m_path+"/PDBana.txt").latin1());
  remove((m_path+"/BadAnalog.txt").latin1());
  remove((m_path+"/PDBsrc.txt").latin1());
  remove((m_path+"/BadSource.txt").latin1());
  remove((m_path+"/PDBmech.txt").latin1());
  // create text files
  if(IVitem!=0)    CreateIV(m_rep);
  if(DIGitem!=0)   CreateDIG(m_rep);
  if(ANAitem!=0)   CreateANA(m_rep);
  if(SRCitem!=0)   CreateSRC(m_rep);


  if(use_defcuts) // if we created default cuts because nothing was there we have to clean up
    m_parent->m_cutpanel->DeleteClass();
  if(m_cutVersioned){ // restrict run index again
    pdbRun->setValue(pdbRun->value()%100);
    pdbRun->setMaxValue(99);
  }

  // prepare for PDB/Web upload
  ResultWindow->setText(ResultWindow->text() + 
                        "<br><br><b><font color=\"#0000FF\">Fill the form in the Mechanical tab or <br>"
                        "select the Upload tab to proceed</font></b><br><br><a name=\"proceed\">");
  ResultWindow->scrollToAnchor("proceed");
  ROOTcheckBox->setEnabled(true);
  ROOTcheckBox->setChecked(m_parent->options->m_Opts->m_CutCreateRoot);
  getRootPath();
  setWWW();
  TabWidget->repaint();
  if(m_app!=0)
    m_app->processEvents();
  CreateButton->setText("Upload");
  CreateButton->setEnabled(false);
  connect( TabWidget, SIGNAL( currentChanged(QWidget*) ), this, SLOT( enableUploadButt() ) );
  disconnect( CreateButton, SIGNAL( clicked() ), this, SLOT( CreatePDB() ) );
  if(m_uploadJava && m_cutVersioned){ // java environment set up correctly
    IVcheckBox->setEnabled(IVitem!=0);
    DIGcheckBox->setEnabled(DIGitem!=0);
    ANAcheckBox->setEnabled(ANAitem!=0);
    SRCcheckBox->setEnabled(SRCitem!=0);
    IVcheckBox->setChecked(IVitem!=0);
    DIGcheckBox->setChecked(DIGitem!=0);
    ANAcheckBox->setChecked(ANAitem!=0);
    SRCcheckBox->setChecked(SRCitem!=0);
    PASScheckBox->setEnabled(m_uploadPASS);
    if(m_uploadPASS)
      PASScheckBox->setChecked(true);
    connect( CreateButton, SIGNAL( clicked() ), this, SLOT( launchJavaUpload() ) );
  }else{             // bugger, user must do java upload manually
    connect( CreateButton, SIGNAL( clicked() ), this, SLOT( miscUpload() ) );
  }
  if(!m_cutVersioned && m_uploadJava) 
    PDBJavaBox->setTitle("PDB files - PIXEL CUTS NOT DEFAULT, UPLOAD NOT ALLOWED");
  return;
}

void PDBForm::CreateDIG(QAReport *rep){

  m_digBadPixs=0; 
  int chip, i, j, con, Mask, col, row;
  int ColPairs=NCOL*NCHIP/2, GoodCh=NCOL*NROW*NCHIP, BadCh[NCHIP], BadChip=0, GoodChip=NCHIP;
  QString ColPair;
  TGraph *gr[6];
  Double_t i_vdd_pwon, i_vdda_pwon, i_vdd_greg, i_vdda_greg, i_vdd_dscan, i_vdda_dscan, waste;
  TH2F *map = rep->m_badpixit[m_selcutID]->GetDID()->GetMap(-1,MAMASK);
  // map = DIGitem->GetDID()->GetMap(-1,RAW);
  // use bad pixel map from QA report
  // if bit 1 is set -> digitally bad  
  if(map==0) return;

  QFile pdbFile(m_path+"/PDBdig.txt");
  if(!pdbFile.open(QIODevice::WriteOnly)){
    QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/PDBdig.txt for writing!");
    return;
  }
  QFile rawFile(m_path+"/BadDigital.txt");
  if(!rawFile.open(QIODevice::WriteOnly)){
    QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/BadDigital.txt for writing!");
    return;
  }

  Q3TextStream stream( &pdbFile );
  Q3TextStream raw( &rawFile );

  for(i=0;i<NCHIP;i++) BadCh[i]=0;

  for(i=0;i<6;i++) gr[i]=0;
  if(Curritem!=0){  // currents TGraphs - if they exist
    // vdda pwon
    gr[0] = Curritem->GetDID()->GetGraph(5);
    // vdd pwon
    gr[1] = Curritem->GetDID()->GetGraph(4);
    // vdda globreg
    gr[2] = Curritem->GetDID()->GetGraph(1);
    // vdd globreg
    gr[3] = Curritem->GetDID()->GetGraph(0);
    // vdda dscan (bare) or XCK off (flex)
    gr[4] = Curritem->GetDID()->GetGraph(3);
    // vdd dscan (bare) or XCK off (flex)
    gr[5] = Curritem->GetDID()->GetGraph(2);
  }

  // checking col pairs and setting bad chips
  for(chip=0;chip<NCHIP;chip++){
    Mask=DIGitem->GetDID()->GetLog()->MyModule->Chips[chip]->ColMask;
    for(i=0;i<9;i++){
      if( !((Mask>>i)&1) ){
        ColPairs--;
        BadChip=BadChip|(1<<chip);
      }
    }
  }

  // finding bad channels
  raw << "chip\tcol\trow\n";
  for(i=1;i<map->GetNbinsX()+1;i++){
    for(j=1;j<map->GetNbinsY()+1;j++){
      con = (int)map->GetBinContent(map->GetBin(i,j));
      //      if( con!=100 ){
      if( con & 1 || con & 1024 ){ //counbting both hit and ToT ineff.
        DIGitem->GetDID()->PixXYtoCCR(i-1,j-1,&chip,&col,&row);
        GoodCh--;
        BadCh[chip]++;
        raw << chip << "\t" << col << "\t" << row << "\t" << con << "\n";
      }
    }
  }
  
  // before i forget :)
  rawFile.close();
  
  // assuming that no counts at all means bad chip
  for(chip=0;chip<NCHIP;chip++){
    if(BadCh[chip]==NCOL*NROW) BadChip=BadChip|(1<<chip);
  }
  
  // finally i can count good chips
  for(i=0;i<16;i++){
    if( (BadChip>>i)&1 ) GoodChip--;
  }

  m_digBadPixs = 46080 - GoodCh;
  
  // filling global informations
  QString date, time;
  TopWin::DateFromLog(DIGitem->GetDID()->GetLog()->comments.c_str(), date, time);
  /*
  //QStringList date = QStringList::split("\n",ldsp.edComments->text(),TRUE);
  QStringList date = QStringList::split("\n",DIGitem->GetDID()->GetLog()->comments.c_str(),TRUE);
  date = date.grep("Scan commenced");
  QStringList::Iterator it = date.end(); --it;
  QString line = (*it).latin1();
  date = date.split(' ',line,FALSE);
  it = date.end(); --it;
  */  
  stream << "%NewTest\n";
  stream << "SERIAL NUMBER\t: " << modsn << "\n";
  stream << "TEST MADE BY\t: "+pdbUser->displayText()+"\n";
  stream << "LOCATION NAME\t: "+pdbLocation->currentText()+"\n";
  stream << "Run Number\t: " << (int)pdbRun->value() << "\n";  
  //  stream << "TEST DATE\t: "+Date(*it)+"\n";
  stream << "TEST DATE\t: "+Date(date)+"\n";

  // what are we supposed to check? only good chips as for now
  if(GoodChip==NCHIP)
    stream << "PASSED\t\t: YES\nPROBLEM\t\t: NO\n";
  else
    stream << "PASSED\t\t: NO\nPROBLEM\t\t: YES\n";
  
  stream << "# --\n";
  if(pdbComment->text().isEmpty()){
    stream << "#%Comment\n";
    stream << "#COMMENT\t: \n";
  } else {
    stream << "%Comment\n";
    stream << "COMMENT\t\t: "+pdbComment->text()+"\n";
  }
  
  stream << "#\n";
  if(pdbDesc->text().isEmpty() || pdbURL->text().isEmpty()){
    stream << "#%Web link\n";
    stream << "#DESCRIPTION\t: \n";
    stream << "#URL\t\t: \n";
  } else {
    stream << "%Web link\n";
    stream << "DESCRIPTION\t: "+pdbDesc->displayText()+"\n";
    stream << "URL\t\t: "+pdbURL->displayText()+pdbURLPath->displayText()+"\n";
  }

  stream << "#\n";
  if(GoodCh!=NCHIP*NCOL*NROW)
    stream << "%TEST Rawdata\nFILENAME\t: BadDigital.txt\n";
  else
    stream << "#%TEST Rawdata\n#FILENAME\t: \n";
  
  if(modsn.left(8)=="20210020"){ // flex module
    stream << "#\n%MODULE DIGITAL\n";
    int aSP, aCH, dSP, dCH;
    aSP = m_parent->options->m_Opts->m_DDAsupp;
    aCH = m_parent->options->m_Opts->m_DDAchan;
    dSP = m_parent->options->m_Opts->m_DDsupp;
    dCH = m_parent->options->m_Opts->m_DDchan;
    stream << QString("I_VDDD\t\t: %1\n").arg(DIGitem->GetDID()->GetLog()->MyDCS->LVcurr[dSP][dCH],0,'f',2);
    stream << QString("I_VDDA\t\t: %1\n").arg(DIGitem->GetDID()->GetLog()->MyDCS->LVcurr[aSP][aCH],0,'f',1);
  } else // bare module
    stream << "#\n%BAREMOD DIGITAL\n";
  stream << "CHIPS_OK\t: " << GoodChip << "\n";
  stream << "COLPAIRS_OK\t: " << ColPairs << "\n";
  stream << "GOOD_CHANNELS\t: " << GoodCh << "\n";
  stream << "STATUS\t\t: "+pdbMeasType->currentText()+" "+GlobStatus->currentText()+"\n";
  
  // single chip checks
  for(chip=0;chip<NCHIP;chip++){
    stream << QString("#\n%CHIP_%1 DIGITAL\n").arg(chip,0,16).upper();
    i_vdda_pwon = 0;
    i_vdd_pwon = 0;
    if(gr[0]!=0) gr[0]->GetPoint(chip,waste,i_vdda_pwon);
    if(gr[1]!=0) gr[1]->GetPoint(chip,waste,i_vdd_pwon);
    stream << QString("I_VDD_PWON\t: %1\nI_VDDA_PWON\t: %2\n").arg(i_vdd_pwon,0,'f',2).arg(i_vdda_pwon,0,'f',2);
    i_vdda_greg = 0;
    i_vdd_greg = 0;
    if(gr[2]!=0) gr[2]->GetPoint(chip,waste,i_vdda_greg);
    if(gr[3]!=0) gr[3]->GetPoint(chip,waste,i_vdd_greg);
    stream << QString("I_VDD_GREG\t: %1\nI_VDDA_GREG\t: %2\n").arg(i_vdd_greg+i_vdd_pwon,0,'f',2).arg(i_vdda_greg+i_vdda_pwon,0,'f',2);
    i_vdda_dscan = 0;
    i_vdd_dscan = 0;
    if(gr[4]!=0) gr[4]->GetPoint(chip,waste,i_vdda_dscan);
    if(gr[5]!=0) gr[5]->GetPoint(chip,waste,i_vdd_dscan);
    stream << QString("I_VDD_DSCAN\t: %1\nI_VDDA_DSCAN\t: %2\n").arg(i_vdd_dscan,0,'f',2).arg(i_vdda_dscan,0,'f',2);
    Mask=DIGitem->GetDID()->GetLog()->MyModule->Chips[chip]->ColMask;
    ColPair = QString::null;
    for(i=0;i<9;i++){
      if( (Mask>>i)&1 ) ColPair="1"+ColPair;
      else ColPair="0"+ColPair;
    }
    
    // assuming that no counts at all means global register failure
    if(BadCh[chip]==NCOL*NROW)
      stream << "GLOBREG\t\t: NO\n";
    else
      stream << "GLOBREG\t\t: YES\n";
    
    stream << "COLPAIR_MASK\t: " << Mask << "\n";
    
    stream << "DIGINJ_GOOD\t: " << NCOL*NROW-BadCh[chip] << "\n";
    // missing
    stream << "STATUS\t\t: in the module\n";
  }
  
  pdbFile.close();
  
  return;
}

void PDBForm::CreateIV(QAReport *rep){

  //TGraph *ivscan;
  //int bdV=600;
  //Double_t x,y,i_oper=0,i_50=0,i_600=0;

  QFile pdbFile(m_path+"/PDBiv.txt");
  if(!pdbFile.open(QIODevice::WriteOnly)){
    QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/PDBiv.txt for writing!");
    return;
  }
  Q3TextStream stream( &pdbFile );
  
  /*
  // Getting data
  ivscan = IVitem->GetDID()->GetCorrIV();
  if(ivscan==0) return;

  for(int n=0; n<=ivscan->GetN(); n++){
    ivscan->GetPoint(n,x,y);
    // BDVoltage
    if(y>10 && bdV==600) bdV=(int)x;
    // i at 150V
    if(x==150) i_oper=y;
    // i at 100V
    if(x==100) i_50=y;
    // i at 600V or at compliance
    if(x==600 || (y==COMP && i_600==600)) i_600=y;
  }
  */

  QString date, time;
  TopWin::DateFromLog(IVitem->GetDID()->GetLog()->comments.c_str(), date, time);

  stream << "%NewTest\n";
  stream << "SERIAL NUMBER\t: " << modsn << "\n";
  stream << "TEST MADE BY\t: "+pdbUser->displayText()+"\n";
  stream << "LOCATION NAME\t: "+pdbLocation->currentText()+"\n";
  stream << "Run Number\t: " << (int)pdbRun->value() << "\n";  
  stream << "TEST DATE\t: "+Date(date)+"\n";

  // What do we check?  do as QAReport did
  if(rep->m_IV_OK)
    stream << "PASSED\t\t: YES\nPROBLEM\t\t: NO\n";
  else
    stream << "PASSED\t\t: NO\nPROBLEM\t\t: YES\n";

  stream << "# --\n";
  if(pdbComment->text().isEmpty()){
    stream << "#%Comment\n";
    stream << "#COMMENT\t: \n";
  } else {
    stream << "%Comment\n";
    stream << "COMMENT\t\t: "+pdbComment->text()+"\n";
  }
  
  stream << "#\n";
  if(pdbDesc->text().isEmpty() || pdbURL->text().isEmpty()){
    stream << "#%Web link\n";
    stream << "#DESCRIPTION\t: \n";
    stream << "#URL\t\t: \n";
  } else {
    stream << "%Web link\n";
    stream << "DESCRIPTION\t: "+pdbDesc->displayText()+"\n";
    stream << "URL\t\t: "+pdbURL->displayText()+pdbURLPath->displayText()+"\n";
  }

  stream << "#\n";
  stream << "%IVSTEST\n";
  stream << "TEMPERATURE\t: "+pdbTemp->displayText()+"\n";
  stream << "BDVOLTAGE\t: " << (int)rep->m_IV_bdV << "\n";
  stream << QString("I_OPERV\t\t: %1\n").arg(rep->m_IV_oper*1000,0,'f',0);
  stream << QString("I_50V\t\t: %1\n").arg(rep->m_IV_100*1000,0,'f',0);
  stream << QString("I_600V\t\t: %1\n").arg(rep->m_IV_max*1000,0,'f',0);
  stream << "STATUS\t\t: "+pdbMeasType->currentText()+" "+GlobStatus->currentText()+"\n";

  pdbFile.close();

  return;
}
void PDBForm::CreateANA(QAReport *rep){

  int totalBad=0;

  m_allBadPixs=0; 
  m_anaBadPixs=0; 
  m_discBadPixs=0;
  m_mlBadPixs=0; 
  m_xtBadPixs=0; 
  m_intBadPixs=0; 
  m_totBadPixs=0;
  m_NTC = -999;

  int chip, col, row, i,j, con, modcon;

  TH2F *map = rep->m_badpixit[m_selcutID]->GetDID()->GetMap(-1,MAMASK);
  if(map==0) return;

  QFile pdbFile(m_path+"/PDBana.txt");
  if(!pdbFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/PDBana.txt for writing!");
        return;
  }
  QFile rawFile(m_path+"/BadAnalog.txt");
  if(!rawFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/BadAnalog.txt for writing!");
        return;
  }

  Q3TextStream stream( &pdbFile );
  Q3TextStream raw( &rawFile );

  // finding bad channels
  raw << "chip\tcol\trow\tcode\n";
  for(i=1;i<map->GetNbinsX()+1;i++){
    for(j=1;j<map->GetNbinsY()+1;j++){
      con = (int)map->GetBinContent(map->GetBin(i,j));
      modcon = con;// - (con & 224); // remove source bits, taken care of elsewhere
      if( modcon ){
        rep->m_badpixit[m_selcutID]->GetDID()->PixXYtoCCR(i-1,j-1,&chip,&col,&row);
        raw << chip << "\t" << col << "\t" << row << "\t" << modcon << "\n";
        // individual failures
        if(con&1 || con&1024 || con&131072 || con&262144) // digital
          totalBad++;
        else if(con&32 || con&64 || con&128 || con&65536) // source matters
          totalBad++;
        else if(con&2){ // HV off disc. test
          m_discBadPixs++;
          m_allBadPixs++;
          totalBad++;
        }else if(con&2048){ // "analog" = bad thresh chi^2
          m_anaBadPixs++;
          m_allBadPixs++;
          totalBad++;
        }else if(con&16384){ // bad ToT fit
          m_totBadPixs++;
          m_allBadPixs++;
          totalBad++;
        }else if(con&8 || con&8192){ // merged and xtalk
          m_xtBadPixs++;
          m_allBadPixs++;
          totalBad++;
        }else if(con&4096){ // high/low monleak
          m_mlBadPixs++;
          m_allBadPixs++;
          totalBad++;
        }else if(con&32768){ // in-time too high
	  m_intBadPixs++;
	  m_allBadPixs++;
          totalBad++;
        }else if(con&256 || con&512){ // noise & threshold cut
          m_allBadPixs++;
          totalBad++;
	}
      }
    }
  }
  
  // filling global informations
  QString date, time;
  TopWin::DateFromLog(ANAitem->GetDID()->GetLog()->comments.c_str(), date, time);
  //  if(AMBitem==0)
    m_NTC = ANAitem->GetDID()->GetLog()->MyDCS->Tntc;
//   else{
//     QString theDate, theTime;
//     const char* cm=ANAitem->GetDID()->GetLog()->comments.c_str();        
//     TopWin::DateFromLog(cm, theDate, theTime);
//     double time=TopWin::BI_getTimeFromStrings(theDate, theTime);
//     m_NTC = AMBitem->GetDID()->BI_getAvgValueOnInterval(BI_NTC,time+30, time+120); // range 0...2 min
//   }

  stream << "%NewTest\n";
  stream << "SERIAL NUMBER\t: " << modsn << "\n";
  stream << "TEST MADE BY\t: "+pdbUser->displayText()+"\n";
  stream << "LOCATION NAME\t: "+pdbLocation->currentText()+"\n";
  stream << "Run Number\t: " << (int)pdbRun->value() << "\n";  
  stream << "TEST DATE\t: "+Date(date)+"\n";

  // copy decision from QA report
  if(rep->m_ANA_OK)
    stream << "PASSED\t\t: YES\nPROBLEM\t\t: NO\n";
  else
    stream << "PASSED\t\t: NO\nPROBLEM\t\t: YES\n";
  
  stream << "# --\n";
  if(pdbComment->text().isEmpty()){
    stream << "#%Comment\n";
    stream << "#COMMENT\t: \n";
  } else {
    stream << "%Comment\n";
    stream << "COMMENT\t\t: "+pdbComment->text()+"\n";
  }
  
  stream << "#\n";
  if(pdbDesc->text().isEmpty() || pdbURL->text().isEmpty()){
    stream << "#%Web link\n";
    stream << "#DESCRIPTION\t: \n";
    stream << "#URL\t\t: \n";
  } else {
    stream << "%Web link\n";
    stream << "DESCRIPTION\t: "+pdbDesc->displayText()+"\n";
    stream << "URL\t\t: "+pdbURL->displayText()+pdbURLPath->displayText()+"\n";
  }

  stream << "#\n";
  //  if(m_allBadPixs>0)
  if(totalBad>0)
    stream << "%TEST Rawdata\nFILENAME\t: BadAnalog.txt\n";
  else
    stream << "#%TEST Rawdata\n#FILENAME\t: \n";
  
  stream << "#\n%MODULE ANALOG\n";

  int aSP, aCH, dSP, dCH;
  aSP = m_parent->options->m_Opts->m_DDAsupp;
  aCH = m_parent->options->m_Opts->m_DDAchan;
  dSP = m_parent->options->m_Opts->m_DDsupp;
  dCH = m_parent->options->m_Opts->m_DDchan;
  stream << QString("I_VDD\t\t: %1\n").arg(ANAitem->GetDID()->GetLog()->MyDCS->LVcurr[dSP][dCH],0,'f',2);
  stream << QString("IVDDA\t\t: %1\n").arg(ANAitem->GetDID()->GetLog()->MyDCS->LVcurr[aSP][aCH],0,'f',1);
  stream << QString("TEMP\t\t: %1\n").arg(m_NTC,0,'f',2);
  stream << QString("THR_AVE\t\t: %1\n").arg(rep->m_avgThr,0,'f',1);
  stream << QString("THR_DISP\t: %1\n").arg(rep->m_sdThr,0,'f',1);
  stream << QString("NOISE\t\t: %1\n").arg(rep->m_avgNse,0,'f',1);
  stream << QString("XTALK\t\t: %1\n").arg(rep->m_avgXtalk,0,'f',2);
  stream << QString("THR_TIME\t: %1\n").arg(rep->m_avgITThresh,0,'f',1);
  stream << QString("THR_TIMD\t: %1\n").arg(rep->m_sdITThresh,0,'f',1);
  stream << QString("TOT_MIP\t\t: %1\n").arg(rep->m_avgTOT,0,'f',2);
  stream << QString("TOT_DISP\t: %1\n").arg(rep->m_sdTOT,0,'f',2);
  stream << "ANA_BAD\t\t: " << m_anaBadPixs << "\n";
  stream << "DISC_BAD\t: " << m_discBadPixs << "\n";
  stream << "MONLEAK_BAD\t: " << m_mlBadPixs << "\n";
  stream << "BAD_XTALK\t: " << m_xtBadPixs << "\n";
  stream << "INTIME_BAD\t: " << m_intBadPixs << "\n";
  stream << "TOT_BAD\t\t: " << m_totBadPixs << "\n";
  stream << "PIX_BAD\t\t: " << m_allBadPixs << "\n";
  stream << QString("VDD_MIN\t\t: %1\n").arg(rep->m_minVDD,0,'f',1);
  stream << QString("VDD_MAX\t\t: %1\n").arg(rep->m_maxVDD,0,'f',1);
  stream << QString("VDDA_MIN\t: %1\n").arg(rep->m_minVDDA,0,'f',1);
  stream << QString("VDDA_MAX\t: %1\n").arg(rep->m_maxVDDA,0,'f',1);
  stream << "GDAC_BAD\t: " << rep->m_GDACfail << "\n";
  stream << QString("THR_MIN\t\t: %1\n").arg(rep->m_minTHR,0,'f',1);
  stream << QString("THR_MAX\t\t: %1\n").arg(rep->m_maxTHR,0,'f',1);
  stream << QString("XCK_RANGE\t: %1\n").arg(rep->m_maxXCK,0,'f',1);
  stream << "STATUS\t\t: "+pdbMeasType->currentText()+" "+GlobStatus->currentText()+"\n";

  rawFile.close();
  pdbFile.close();
}

void PDBForm::CreateSRC(QAReport *rep){
  m_srcBadPixs = 0;
  m_srcMasked = 0;
  int hitBadPixs=0, totBadPixs=0, nseBadPixs=0, maskBadPixs=0;
  int chip, col, row, i,j, con, modcon;//, masked;

  TH2F *map = rep->m_badpixit[m_selcutID]->GetDID()->GetMap(-1,MAMASK);
  if(map==0) return;

  QFile pdbFile(m_path+"/PDBsrc.txt");
  if(!pdbFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/PDBsrc.txt for writing!");
        return;
  }
  QFile rawFile(m_path+"/BadSource.txt");
  if(!rawFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/BadSource.txt for writing!");
        return;
  }

  Q3TextStream stream( &pdbFile );
  Q3TextStream raw( &rawFile );

  // finding bad channels
  raw << "chip\tcol\trow\tcode\n";
  for(i=1;i<map->GetNbinsX()+1;i++){
    for(j=1;j<map->GetNbinsY()+1;j++){
      con = (int)map->GetBinContent(map->GetBin(i,j));
      modcon = con & 65760; // remove non-source bits, taken care of elsewhere
      rep->m_badpixit[m_selcutID]->GetDID()->PixXYtoCCR(i-1,j-1,&chip,&col,&row);
      // check if pixel was masked
      /*
      masked = !(SRCitem->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[0] &&
               (SRCitem->GetDID()->GetLog()->MyModule->Chips[chip]->ColMask >> (col/2)) && 
               SRCitem->GetDID()->GetLog()->MyModule->Chips[chip]->ReadoutMask->IsOn(col,row) &&
               SRCitem->GetDID()->GetLog()->MyModule->Chips[chip]->HitbusMask->IsOn(col,row) &&
                 SRCitem->GetDID()->GetLog()->MyModule->Chips[chip]->PreampMask->IsOn(col,row));
      modcon |= 128*(int)(bool)masked;
      */
      if( modcon ){
        raw << chip << "\t" << col << "\t" << row << "\t" << modcon << "\n";
        //        raw << chip << "\t" << col << "\t" << row << "\n";
        // individual failures
        if(con&1 || con&1024 || con&131072 || con&262144) // digital: not counted again
          ;
        else{
        // 32=source hit, 64=source ToT, 128=masked, 65536=hot in source. -> others not used here
          if(con&32){
            hitBadPixs++;
	    m_srcBadPixs++;
          } else if(con&64){
            totBadPixs++;
	    m_srcBadPixs++;
          } else if(con&128){
            maskBadPixs++;
	    m_srcMasked++;
          } else if(con&65536){
            nseBadPixs++;
	    m_srcBadPixs++;
	  }
        }
      }
    }
  }
  
  // filling global informations
  QString date, time;
  TopWin::DateFromLog(SRCitem->GetDID()->GetLog()->comments.c_str(), date, time);
  
  stream << "%NewTest\n";
  stream << "SERIAL NUMBER\t: " << modsn << "\n";
  stream << "TEST MADE BY\t: "+pdbUser->displayText()+"\n";
  stream << "LOCATION NAME\t: "+pdbLocation->currentText()+"\n";
  stream << "Run Number\t: " << (int)pdbRun->value() << "\n";  
  stream << "TEST DATE\t: "+Date(date)+"\n";

  // copy decision from QA report
  //if(rep->m_SRC_OK)
  //count bad pixels
  if(m_srcBadPixs<461)
    stream << "PASSED\t\t: YES\nPROBLEM\t\t: NO\n";
  else
    stream << "PASSED\t\t: NO\nPROBLEM\t\t: YES\n";
  
  stream << "# --\n";
  if(pdbComment->text().isEmpty()){
    stream << "#%Comment\n";
    stream << "#COMMENT\t: \n";
  } else {
    stream << "%Comment\n";
    stream << "COMMENT\t\t: "+pdbComment->text()+"\n";
  }
  
  stream << "#\n";
  if(pdbDesc->text().isEmpty() || pdbURL->text().isEmpty()){
    stream << "#%Web link\n";
    stream << "#DESCRIPTION\t: \n";
    stream << "#URL\t\t: \n";
  } else {
    stream << "%Web link\n";
    stream << "DESCRIPTION\t: "+pdbDesc->displayText()+"\n";
    stream << "URL\t\t: "+pdbURL->displayText()+pdbURLPath->displayText()+"\n";
  }

  stream << "#\n";
  if(m_srcBadPixs>0)
    stream << "%TEST Rawdata\nFILENAME\t: BadSource.txt\n";
  else
    stream << "#%TEST Rawdata\n#FILENAME\t: \n";
  
  stream << "#\n%MODULE SOURCE\n";

  int aSP, aCH, dSP, dCH, hSP, hCH;
  aSP = m_parent->options->m_Opts->m_DDAsupp;
  aCH = m_parent->options->m_Opts->m_DDAchan;
  dSP = m_parent->options->m_Opts->m_DDsupp;
  dCH = m_parent->options->m_Opts->m_DDchan;
  hSP = m_parent->options->m_Opts->m_BIASsupp;
  hCH = m_parent->options->m_Opts->m_BIASchan;

  float T_NTC=-999;
  //  if(AMBitem==0)
    T_NTC = SRCitem->GetDID()->GetLog()->MyDCS->Tntc;
//   else{
//     QString theDate, theTime;
//     const char* cm=SRCitem->GetDID()->GetLog()->comments.c_str();        
//     TopWin::DateFromLog(cm, theDate, theTime);
//     double time=TopWin::BI_getTimeFromStrings(theDate, theTime);
//     T_NTC = AMBitem->GetDID()->BI_getAvgValueOnInterval(BI_NTC,time, time+120); // range 0...2 min.
//   }

  stream << QString("I_VDD\t\t: %1\n").arg(SRCitem->GetDID()->GetLog()->MyDCS->LVcurr[dSP][dCH],0,'f',1);
  stream << QString("IVDDA\t\t: %1\n").arg(SRCitem->GetDID()->GetLog()->MyDCS->LVcurr[aSP][aCH],0,'f',1);
  stream << QString("TEMP\t\t: %1\n").arg(T_NTC,0,'f',1);
  stream << QString("V_DET\t\t: %1\n").arg(SRCitem->GetDID()->GetLog()->MyDCS->HVvolt[hSP][hCH],0,'f',1);
  stream << QString("I_LEAK\t\t: %1\n").arg(1000*SRCitem->GetDID()->GetLog()->MyDCS->HVcurr[hSP][hCH],0,'f',1);
  stream << "MASK_INIT\t: " << maskBadPixs << "\n";
  stream << "MASK_HOT\t: " << nseBadPixs << "\n";
  stream << "LOW_EFF\t\t: " << hitBadPixs << "\n";
  stream << "LOW_TOT\t\t: " << totBadPixs << "\n";
  stream << "STATUS\t\t: "+pdbMeasType->currentText()+" "+GlobStatus->currentText()+"\n";

  rawFile.close();
  pdbFile.close();
}

bool PDBForm::runJavaUpload(const char *javaMacro, const char *textFile, 
                            const char *user, const char *passwd, bool fileIsLast){
  if(m_path.isEmpty()) return true;

  QString orgText = ResultWindow->text(), newText="", lastAnch="endof";
  lastAnch += textFile;

  Q3Process exe_java(this);
  exe_java.setWorkingDirectory(QDir(m_path));
  exe_java.addArgument("java");
  exe_java.addArgument(javaMacro);
  if(!fileIsLast)
    exe_java.addArgument(textFile);
  exe_java.addArgument(user);
  exe_java.addArgument(passwd);
  if(fileIsLast)
    exe_java.addArgument(textFile);
  if(exe_java.start()){
    QMessageBox *stop_win = new QMessageBox("Abort upload","Abort " + QString(javaMacro) + " " + QString(textFile),
                                            QMessageBox::NoIcon,QMessageBox::Abort,0,0,
                                            NULL,"abort_upload",FALSE);
    stop_win->show();
    while(exe_java.isRunning()){ // wait for command to finish
      while(exe_java.canReadLineStdout()){
        newText += exe_java.readLineStdout() + "<br>";
        ResultWindow->setText(orgText + newText + "<a name=\"endoftxt\"></a>");
        ResultWindow->scrollToAnchor("endoftxt");
        ResultWindow->repaint();
        if(m_app!=0)
          m_app->processEvents();
      }
      if(!stop_win->isVisible()){
        exe_java.kill();
        break;
      }
      gSystem->Sleep(300);
    }
    delete stop_win;
    while(exe_java.canReadLineStdout()){
        newText += exe_java.readLineStdout() + "<br>";
        ResultWindow->setText(orgText + newText + "<a name=\"endoftxt\"></a>");
        ResultWindow->scrollToAnchor("endoftxt");
        ResultWindow->repaint();
        if(m_app!=0)
          m_app->processEvents();
    }
    ResultWindow->setText(orgText + newText + "<a name=\""+lastAnch+"\"></a>");
    ResultWindow->scrollToAnchor(lastAnch);
    ResultWindow->repaint();
    if(m_app!=0)
      m_app->processEvents();
    if(exe_java.canReadLineStderr()){
      QString mymsg = "Error uploading ";
      mymsg += textFile;
      mymsg += ".\n";
      while(exe_java.canReadLineStderr()){
        mymsg += exe_java.readLineStderr();
        mymsg += "\n";
      }
      QMessageBox::warning(this,"PDB upload error",mymsg);
      return true;
    }
  } else{
    QString mymsg = "Error launching ";
    mymsg += javaMacro;
    mymsg += ".\n";
    QMessageBox::warning(this,"PDB upload error",mymsg);
    return true;
  }
  return false;
}
void PDBForm::launchJavaUpload(const char *in_User, const char *in_PWD){
  disconnect( TabWidget, SIGNAL( currentChanged(QWidget*) ), this, SLOT( enableUploadButt() ) );
  TabWidget->setCurrentPage(2);
  TabWidget->repaint();
  if(m_app!=0)
    m_app->processEvents();
  if(IVcheckBox->isChecked() || DIGcheckBox->isChecked() || ANAcheckBox->isChecked() || SRCcheckBox->isChecked()
     || MECHcheckBox->isChecked() || (m_uploadPASS && PASScheckBox->isChecked())){
    QString User=in_User, Password=in_PWD;
    if(m_User!=QString::null &&  m_Passwd!=QString::null){
      User = m_User;
      Password = m_Passwd;
    } else if(in_User==0 && in_PWD==0){
      // get username and password for PDB
      LoginPanel lp(this,"loginpanel");
      lp.HostLabel->setText("Login details for PDB:");
      lp.HostName->hide();
      switch(pdbLocation->currentItem()){
      case 0:  // Marseille
	lp.UserID->setText("P_MAR");
	break;
      case 1:  // Milan
	lp.UserID->setText("P_MIL");
	break;
      case 2:  // Genoa
	lp.UserID->setText("P_GEN");
	break;
      case 3:  // LBL
	lp.UserID->setText("P_LBL");
	break;
      case 4:  // Bonn
	lp.UserID->setText("P_BON");
	break;
      case 5:  // Dortmund
	lp.UserID->setText("P_DOR");
	break;
      case 6:  // Wuppertal
	lp.UserID->setText("P_WUP");
	break;
      case 7:  // Siegen
	lp.UserID->setText("P_SIE");
	break;
      default:
	lp.UserID->setText("");
	break;
      }
      lp.exec();
      User = lp.UserID->text();
      Password = lp.Password->text();
    }
    // execute IV upload
    ResultWindow->setText(ResultWindow->text() + "<br>");
    ResultWindow->setText("Uploading files to PDB now<br>");
    FILE *exists;
    if(IVcheckBox->isChecked()){
      exists = fopen((m_path+"/PDBiv.txt").latin1(),"r");
      if(exists!=0){
        fclose(exists);
        if(runJavaUpload("UploadTestData", "PDBiv.txt", User.latin1(), Password.latin1()))
          return;
        if(removeFiles->isChecked())
          remove((m_path+"/PDBiv.txt").latin1());
      }
    }
    // execute DIG upload
    if(DIGcheckBox->isChecked()){
      exists = fopen((m_path+"/PDBdig.txt").latin1(),"r");
      if(exists!=0){
        fclose(exists);
        if(runJavaUpload("UploadModuleTest", "PDBdig.txt", User.latin1(), Password.latin1()))
          return;
        if(removeFiles->isChecked()){
          remove((m_path+"/PDBdig.txt").latin1());
          remove((m_path+"/BadDigital.txt").latin1());
        }
      }
    }
    // execute ANA upload
    if(ANAcheckBox->isChecked()){
      exists = fopen((m_path+"/PDBana.txt").latin1(),"r");
      if(exists!=0){
        fclose(exists);
        if(runJavaUpload("UploadModuleTest", "PDBana.txt", User.latin1(), Password.latin1()))
          return;
        if(removeFiles->isChecked()){
          remove((m_path+"/PDBana.txt").latin1());
          remove((m_path+"/BadAnalog.txt").latin1());
        }
      }
    }
    // execute SRC upload
    if(SRCcheckBox->isChecked()){
      exists = fopen((m_path+"/PDBsrc.txt").latin1(),"r");
      if(exists!=0){
        fclose(exists);
        if(runJavaUpload("UploadModuleTest", "PDBsrc.txt", User.latin1(), Password.latin1()))
          return;
        if(removeFiles->isChecked()){
          remove((m_path+"/PDBsrc.txt").latin1());
          remove((m_path+"/BadSource.txt").latin1());
        }
      }
    }
    // execute MECH upload
    if(MECHcheckBox->isChecked()){
      exists = fopen((m_path+"/PDBmech.txt").latin1(),"r");
      if(exists!=0){
        fclose(exists);
        if(runJavaUpload("UploadModuleTest", "PDBmech.txt", User.latin1(), Password.latin1()))
          return;
        if(removeFiles->isChecked()){
          remove((m_path+"/PDBmech.txt").latin1());
        }
      }
    }
    if(m_uploadPASS && PASScheckBox->isChecked()){
      QFile pdbFile(m_path+"/item_passed.txt");
      if(!pdbFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/item_passed.txt for writing!");
      } else {
        Q3TextStream stream( &pdbFile );
        stream << "%SET PASSED ITEMS\n";
        stream << modsn + "  YES\n";
        pdbFile.close();
        runJavaUpload("SetPASSEDItemsFromFile", "item_passed.txt", 
                      User.latin1(), Password.latin1(), true);
        remove((m_path+"/item_passed.txt").latin1());
      }
    }
  }

  // do remaining upload
  miscUpload();

  return;
}
void PDBForm::miscUpload(){
  disconnect( TabWidget, SIGNAL( currentChanged(QWidget*) ), this, SLOT( enableUploadButt() ) );
  
  // save to root file
  if(m_parent!=0 && m_modItem!=0 && !ROOTFile->text().isEmpty() && ROOTcheckBox->isChecked()){
    m_parent->SaveData(m_modItem, 1, ROOTFile->text().latin1(), "Auto-saved from PDBForm");
  }
  // upload to data/web server
  if(WWWcheckBox->isChecked() && !WebserverCmd->text().isEmpty()){
    // clear emeory if requested
    if(WWWclearBefore->isChecked() && m_modItem!=0){
      ModItem *tmpit, *datits = (ModItem*) m_modItem->firstChild();
      while(datits!=0){
        tmpit = datits;
        datits = (ModItem *) datits->nextSibling();
        DatSet *mydata = tmpit->GetDID();
        delete tmpit;
        delete mydata;
      }
    }
    // do the actual upload
    QString sysname = gSystem->GetName();
    // replace wildcards in command
    QString transl_cmd=WebserverCmd->text();
    QString hostname="localhost";
    QString usrname="guest";
    Q3Process *getenv;
    if(sysname=="WinNT"){
      hostname = gSystem->Getenv("COMPUTERNAME");
    } else{
      getenv = new Q3Process(this);
      getenv->addArgument("printenv");
      getenv->addArgument("HOSTNAME");
      getenv->start();
      while(getenv->isRunning()){};
      hostname= getenv->readLineStdout();
      delete getenv;
    }
    hostname = hostname.lower();
    if(sysname=="WinNT"){
      usrname = gSystem->Getenv("USERNAME");
    } else{
      getenv = new Q3Process(this);
      getenv->addArgument("printenv");
      getenv->addArgument("USER");
      getenv->start();
      while(getenv->isRunning()){};
      usrname= getenv->readLineStdout();
      delete getenv;
    }
    usrname = usrname.lower();

    transl_cmd = transl_cmd.replace("%m",modsn.right(6));
    transl_cmd = transl_cmd.replace("%s",modsn);
    transl_cmd = transl_cmd.replace("%h",hostname);
    transl_cmd = transl_cmd.replace("%u",usrname);

    ResultWindow->setText(ResultWindow->text() + "<br>");
    ResultWindow->setText(ResultWindow->text() + "Executing<br>\""+transl_cmd+"\"<br>");
    if(!WebserverName->text().isEmpty())
      ResultWindow->setText(ResultWindow->text() + "on "+WebserverName->text());
    else
      ResultWindow->setText(ResultWindow->text() + "locally");
    ResultWindow->setText(ResultWindow->text() + " now.<br><br><a name=\"webstart\">");
    ResultWindow->scrollToAnchor("webstart");

    QString exe_str;
    if(!WebserverName->text().isEmpty()){ // execute command via ssh(2) if server name is given
      exe_str = "ssh";
      if(m_uploadWeb==2)
        exe_str += "2";
      exe_str += " " + WebserverName->text() + " " + transl_cmd;
    }else   // execute command locally otherwise
      exe_str = WebserverCmd->text().latin1();
    // windows: open window via BAT file
    // unix: use exisitng shell (?)
    if(sysname=="WinNT"){
      FILE *bat = fopen("webexe.bat","w");
      fprintf(bat,"%s\n",exe_str.latin1());
      if(m_pauseAfterWWW)
	fprintf(bat,"pause\n");
      fclose(bat);
      system("webexe.bat");
      remove("webexe.bat");
    }else
      system(exe_str.latin1());

    /*
    LoginPanel lp(this,"loginpanel");
    lp.HostLabel->setText("Login details for ");
    lp.HostName->setText(WebserverName->text());
    lp.UserID->setText(m_parent->options->m_Opts->m_CutWebUser.c_str());
    lp.exec();
    QProcess exe_ssh(this);
    if(m_uploadWeb==1)
      exe_ssh.addArgument("ssh");
    else if(m_uploadWeb==2)
      exe_ssh.addArgument("ssh2");
    else
      return;
    exe_ssh.addArgument(User+"@"+WebserverName->text());
    exe_ssh.addArgument(WebserverCmd->text());
    if(!exe_ssh.start()){
      QMessageBox::warning(this,"MAF","Error launching Web upload");
      return;
    }
    gSystem->Sleep(500);
    exe_ssh.writeToStdin(Password);
    exe_ssh.closeStdin();
    QMessageBox *stop_win = new QMessageBox("Abort upload","Abort Web-upload",
                                            QMessageBox::NoIcon,QMessageBox::Abort,0,0,
                                            NULL,"abort_upload",FALSE);
    stop_win->show();
    int time_out = 0;
    while(exe_ssh.isRunning() && time_out<50){ // wait for command to finish
      while(exe_ssh.canReadLineStdout())
        ResultWindow->setText(ResultWindow->text() + exe_ssh.readLineStdout() + "<br>");
      if(!stop_win->isVisible()){
        exe_ssh.kill();
        break;
      }
      gSystem->Sleep(300);
      time_out++;
    }
    if(time_out==50){
      exe_ssh.kill();
      QMessageBox::warning(this,"Web upload error","ssh timed out");
    }
    delete stop_win;
    while(exe_ssh.canReadLineStdout()){
      ResultWindow->setText(ResultWindow->text() + exe_ssh.readLineStdout() + "<br>");
    }
    if(exe_ssh.canReadLineStderr()){
      QString mymsg = "Error executing.\n";
      while(exe_ssh.canReadLineStderr()){
        mymsg += exe_ssh.readLineStderr();
        mymsg += "\n";
      }
      QMessageBox::warning(this,"Web upload error",mymsg);
      return;
    }
    */
  }

  // after this there's nothing left to do, change run-button
  CreateButton->setText("Close");
  disconnect( CreateButton, SIGNAL( clicked() ), this, SLOT( launchJavaUpload() ) );
  connect( CreateButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
  return;
}
void PDBForm::LoadRootPath(){
  QStringList filter("ROOT file (*.root)");
  filter += "Any file (*.*)";
  Q3FileDialog fdia(ROOTFile->text(), QString::null,this,"select data file",TRUE);
  fdia.setFilters(filter);
  fdia.setMode(Q3FileDialog::AnyFile);
  if(fdia.exec() == QDialog::Accepted)
    ROOTFile->setText(fdia.selectedFile());
  return;
}
void PDBForm::getRootPath(){
  QString data_path=m_parent->options->m_Opts->m_DefDataPath.c_str();
  if(data_path.isEmpty() || data_path=="."){
    ROOTFile->setText("");
  }else{
    if(modsn.left(8)=="20210020") // flex module, need only right six digits for dir
      data_path += "M" + modsn.right(6) + "/data/" + modsn + "_" + modtype;
    else
      data_path += "M" + modsn + "/data/" + modsn + "_" + modtype;
    if(pdbRun->value()<10)
      data_path += "0"+pdbRun->text();
    else
      data_path += pdbRun->text();
    data_path += ".root";
    ROOTFile->setText(data_path);
  }
  return;
}
void PDBForm::enableUploadButt(){
  CreateButton->setEnabled(TabWidget->currentPageIndex()==4);
}

void PDBForm::CreateMECH(){

  bool isOK;

  if(m_path.isEmpty()) {
    if(outputPath->text().isEmpty())
      m_path = Q3FileDialog::getExistingDirectory(m_path,this,"Working PDB Directory","Save Path",true);
    else
      m_path = outputPath->text();
  }

  QString myindex;
  if(pdbRun->value()<10)
    myindex="0"+pdbRun->text();
  else
    myindex=pdbRun->text();

  if(m_modItem!=0){
    m_path.replace("M%m","M"+m_modItem->text(0));
    m_path.replace("%m","M"+m_modItem->text(0));
  }
  m_path.replace("%s",modsn);
  if(pdbMeasType->currentText().isEmpty())
    m_path.replace("%t/","");
  else
    m_path.replace("%t",pdbMeasType->currentText());
  m_path.replace("%i",myindex);
  if(m_path.isEmpty()) return;

  // check path with QAReport
  if(m_rep!=0)
    m_rep->LoadPath(m_path.latin1());

  QFile pdbFile(m_path+"/PDBmech.txt");
  if(!pdbFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"Module Analysis","Unable to open file "+m_path+"/PDBmech.txt for writing!");
        return;
  }
  Q3TextStream stream( &pdbFile );

  // filling global informations  
  stream << "%NewTest\n";
  stream << "SERIAL NUMBER\t: " << modsn << "\n";
  stream << "TEST MADE BY\t: "+pdbUser->displayText()+"\n";
  stream << "LOCATION NAME\t: "+pdbLocation->currentText()+"\n";
  stream << "Run Number\t: " << (int)pdbRun->value() << "\n";  
  stream << "TEST DATE\t: "+mechDate->text()+"\n";

  if(true)
    stream << "PASSED\t\t: YES\nPROBLEM\t\t: NO\n";
  else
    stream << "PASSED\t\t: NO\nPROBLEM\t\t: YES\n";
  
  stream << "# --\n";
  if(mechComment->text().isEmpty()){
    stream << "#%Comment\n";
    stream << "#COMMENT\t: \n";
  } else {
    stream << "%Comment\n";
    stream << "COMMENT\t\t: "+mechComment->text()+"\n";
  }
  
  stream << "#\n";

  stream << "#\n%MODULE MECHANICAL\n";
  stream << "NONSTICK_MCC\t: " << nonstMCCWire->text().toInt(&isOK) << "\n";
  stream << "REBOND_MCC\t: " << repMCCWire->text().toInt(&isOK) << "\n";
  stream << QString("MIN_PULL_MCC\t: %1\n").arg(pullMCCmin->text().toFloat(&isOK),0,'f',1);
  stream << QString("MAX_PULL_MCC\t: %1\n").arg(pullMCCmax->text().toFloat(&isOK),0,'f',1);
  stream << "NONSTICK_FE\t: " << nonstFEWire->text().toInt(&isOK) << "\n";
  stream << "REBOND_FE\t: " << repFEWire->text().toInt(&isOK) << "\n";
  stream << QString("MIN_PULL_FE\t: %1\n").arg(pullFEmin->text().toFloat(&isOK),0,'f',1);
  stream << QString("MAX_PULL_FE\t: %1\n").arg(pullFEmax->text().toFloat(&isOK),0,'f',1);
  stream << QString("AVE_PULL_ASSY\t: %1\n").arg(pullASSY->text().toFloat(&isOK),0,'f',1);
  stream << "AVE_PULL_LOAD\t: 0\n";
  float dtMCC = deltaTMCC->text().toFloat(&isOK);
  if(isOK)
    stream << QString("DELTAT_MCC\t: %1\n").arg(dtMCC,0,'f',1);
  else
    stream << "DELTAT_MCC\t: 0\n";
  stream << "NUM_VISIBLE\t: " << numMarks->text().toInt(&isOK) << "\n";
  float bbow = bareBow->text().toFloat(&isOK);
  if(isOK)
    stream << QString("BARE_BOW\t: %1\n").arg(bbow,0,'f',1);
  else
    stream << "BARE_BOW\t: -1.0\n";
  stream << QString("FLEX_BOW\t: %1\n").arg(flexBow->text().toFloat(&isOK),0,'f',1);
  float mmass = moduleMass->text().toFloat(&isOK);
  if(isOK)
    stream << QString("MASS\t\t: %1\n").arg(mmass,0,'f',1);
  else
    stream << "MASS\t\t: 0\n";
  stream << "MASS_EMPTY\t: 0\n";
  stream << "PENALTY\t\t: " << userPenalty->value() << "\n";
  thermCondWarm->text().toFloat(&isOK);
  if(isOK)
    stream << "SPARE_R1\t: " << thermCondWarm->text() << "\n";
  else
    stream << "SPARE_R1\t: 0\n";
  thermCondCold->text().toFloat(&isOK);
  if(isOK)
    stream << "SPARE_R2\t: " << thermCondCold->text() << "\n";
  else
    stream << "SPARE_R2\t: 0\n";
  stream << "SPARE_R3\t: 0\n";
  stream << "SPARE_I1\t: 0\n";
  stream << "SPARE_I2\t: 0\n";
  stream << "SPARE_I3\t: 0\n";
  if(surveyBox->isEnabled() && surveyBox->isChecked())
    stream << "SPARE_FL1\t: YES\n";
  else
    stream << "SPARE_FL1\t: NO\n";
  stream << "SPARE_FL2\t: NO\n";
  stream << "SPARE_FL3\t: NO\n";
  stream << "STATUS\t\t: "+pdbMeasType->currentText()+" "+GlobStatus->currentText()+"\n";
  //  stream << "STATUS\t\t: MODULE\n";
  pdbFile.close();

  return;
}
void PDBForm::checkMech(){
  m_uploadMech = true;
  bool isOK;
  int itest;
  float ftest;
  itest = nonstMCCWire->text().toInt(&isOK);
  m_uploadMech &= isOK;
  itest = repMCCWire->text().toInt(&isOK);
  m_uploadMech &= isOK;
  ftest = pullMCCmin->text().toFloat(&isOK);
  m_uploadMech &= isOK;
  ftest = pullMCCmax->text().toFloat(&isOK);
  m_uploadMech &= isOK;
  itest = nonstFEWire->text().toInt(&isOK);
  m_uploadMech &= isOK;
  itest = repFEWire->text().toInt(&isOK);
  m_uploadMech &= isOK;
  ftest = pullFEmin->text().toFloat(&isOK);
  m_uploadMech &= isOK;
  ftest = pullFEmax->text().toFloat(&isOK);
  m_uploadMech &= isOK;
  ftest = pullASSY->text().toFloat(&isOK);
  m_uploadMech &= isOK;
  itest = numMarks->text().toInt(&isOK);
  m_uploadMech &= (isOK&&itest>=0 && itest<=4);
//   ftest = bareBow->text().toFloat(&isOK);
//   m_uploadMech &= isOK;
  ftest = flexBow->text().toFloat(&isOK);
  if(m_rep!=0){
    m_rep->moduleBow->setEnabled(!isOK);
    if(isOK)
      m_rep->moduleBow->setValue((int) ftest);
  }
  m_uploadMech &= isOK;
  //  ftest = moduleMass->text().toFloat(&isOK);
  //  m_uploadMech &= isOK;
  if(surveyBox->isEnabled()){
    m_uploadMech = true;  // override FLEX-oriented decision if it's LOAD/STAVE data
    if(QColor( 255, 255, 255 )==thermCondWarm->paletteBackgroundColor()){
      ftest = thermCondWarm->text().toFloat(&isOK);
      m_uploadMech &= isOK;
    }
    if(QColor( 255, 255, 255 )==thermCondCold->paletteBackgroundColor()){
      ftest = thermCondCold->text().toFloat(&isOK);
      m_uploadMech &= isOK;
    }
  }

  // check date
  mechDate->text().left(2).toInt(&isOK);
  m_uploadMech &= isOK;
  mechDate->text().mid(3,2).toInt(&isOK);
  m_uploadMech &= isOK;
  mechDate->text().right(2).toInt(&isOK);
  m_uploadMech &= isOK;
  m_uploadMech &= mechDate->text().mid(2,1)=="/"&&mechDate->text().mid(5,1)=="/";

  // check if comment is requested
  if(userPenalty->value()>0){
    if(mechComment->text().isEmpty())
      m_uploadMech = false;
    mechComment->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
  } else
    mechComment->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );

  if(m_uploadMech){
    CreateMECH();
    mechStatus->setText("data OK, you may proceed to the upload tab");
  } else
    mechStatus->setText("missing/incorrect data - mech. upload will not be permitted");
  MECHcheckBox->setEnabled(m_uploadMech);
  MECHcheckBox->setChecked(m_uploadMech);
}
void PDBForm::saveTable(const char *path){
  if(m_rep!=0)
    m_rep->saveTable(path);
}
const char* PDBForm::getModName(){
  if(m_modItem!=0) return m_modItem->text(0).latin1();
  else             return 0;
}
void PDBForm::setMechForm(int isLoad){
  // LOAD properties
  surveyBox->setEnabled(isLoad);
  thermCondWarm->setEnabled(isLoad);
  thermCondCold->setEnabled(isLoad);
  tcwLabel->setEnabled(isLoad);
  tccLabel->setEnabled(isLoad);
  // FLEX properties
  nonstMCCWire->setEnabled(!isLoad);
  repMCCWire->setEnabled(!isLoad);
  nonstMCCLabel->setEnabled(!isLoad);
  repMCCLabel->setEnabled(!isLoad);
  pMCCminLabel->setEnabled(!isLoad);
  pMCCmaxLabel->setEnabled(!isLoad);
  pMCCunit->setEnabled(!isLoad);
  nstFELabel->setEnabled(!isLoad);
  repFELabel->setEnabled(!isLoad);
  pFEminLabel->setEnabled(!isLoad);
  pFEmaxLabel->setEnabled(!isLoad);
  pFEunit->setEnabled(!isLoad);
  avgPullLbael->setEnabled(!isLoad);
  avgPullUnit->setEnabled(!isLoad);
  //  dTMCCLabel->setEnabled(!isLoad);
  marksLabel->setEnabled(!isLoad);
  bbowLabel->setEnabled(!isLoad);
  //  textLabel1_7->setEnabled(!isLoad);
  //  fbowLabel->setEnabled(!isLoad);
  //  textLabel1_7_2->setEnabled(!isLoad);
  //  massLabel->setEnabled(!isLoad);
  //  massUnit->setEnabled(!isLoad);

  pullMCCmin->setEnabled(!isLoad);
  pullMCCmax->setEnabled(!isLoad);
  nonstFEWire->setEnabled(!isLoad);
  repFEWire->setEnabled(!isLoad);
  pullFEmin->setEnabled(!isLoad);
  pullFEmax->setEnabled(!isLoad);
  pullASSY->setEnabled(!isLoad);
  //  deltaTMCC->setEnabled(!isLoad);
  numMarks->setEnabled(!isLoad);
  bareBow->setEnabled(!isLoad);
  //  flexBow->setEnabled(!isLoad);
  //  moduleMass->setEnabled(!isLoad);
  if(isLoad){
    if(isLoad==1){
      thermCondWarm->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
      thermCondCold->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    }else if(isLoad==2){
      thermCondWarm->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
      thermCondCold->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    }else{
      thermCondCold->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
      thermCondWarm->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    }
    pullMCCmin->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    pullMCCmax->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    nonstFEWire->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    repFEWire->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    pullFEmin->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    pullFEmax->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    pullASSY->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    deltaTMCC->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    numMarks->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    bareBow->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    flexBow->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
  } else{
    thermCondWarm->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    thermCondCold->setPaletteBackgroundColor( QColor( 190, 190, 255 ) );
    pullMCCmin->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    pullMCCmax->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    nonstFEWire->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    repFEWire->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    pullFEmin->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    pullFEmax->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    pullASSY->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    deltaTMCC->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    numMarks->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    bareBow->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    flexBow->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
  }
}
void PDBForm::setWWW(){
  if(m_uploadWeb==0)
    groupBoxWWW->setTitle("Web upload: NOT POSSIBLE, CAN'T FIND SSH!");
  WWWcheckBox->setEnabled(m_uploadWeb);
  if(m_parent==0) return;
  WWWcheckBox->setChecked(m_uploadWeb && m_parent->options->m_Opts->m_CutWebCopy);
  WWWclearBefore->setEnabled(m_uploadWeb);
  WWWclearBefore->setChecked(m_uploadWeb && m_parent->options->m_Opts->m_pdbClear);
  if(m_parent->options->m_Opts->m_CutWebPath=="")
    WebserverName->setText("");
  else
    WebserverName->setText((m_parent->options->m_Opts->m_CutWebUser+"@"+
                            m_parent->options->m_Opts->m_CutWebPath).c_str());
  WebserverCmd->setText(m_parent->options->m_Opts->m_CutWebCmd.c_str());
  return;
}
