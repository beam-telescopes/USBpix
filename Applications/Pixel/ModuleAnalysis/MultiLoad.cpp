#include <q3table.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qcombobox.h>
#include <q3filedialog.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <q3http.h>
#include <qpushbutton.h>

#include "MultiLoad.h"
#include "ModWin.h"
#include "StdTestWin.h"
#include "PDBForm.h"
#include "TopWin.h"
#include "OptClass.h"
#include "OptWin.h"
#include "LoginPanel.h"
#include "RenameWin.h"
#include "CDatabase.h"
#include "LoadStave.h"
#include "StdListChoice.h"
#include "MAEngine.h"

#include <string>

MultiLoad::MultiLoad( QWidget* parent, const char* , bool , Qt::WFlags fl )
  : QDialog(parent,fl){
  //  : MultiLoadBase(parent,name,modal,fl){
  setupUi(this);
  QObject::connect(addStaveButton, SIGNAL(clicked()), this, SLOT(addStave()));
  QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(browseModPath()));
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(downButton, SIGNAL(clicked()), this, SLOT(moveEntryDown()));
  QObject::connect(addRootButton, SIGNAL(clicked()), this, SLOT(addRoot()));
  QObject::connect(addListButton, SIGNAL(clicked()), this, SLOT(addModuleList()));
  QObject::connect(measIndex, SIGNAL(valueChanged(int)), this, SLOT(meastypeChanged()));
  QObject::connect(meastypeBox, SIGNAL(activated(int)), this, SLOT(meastypeChanged()));
  QObject::connect(moduleTable, SIGNAL(currentChanged(int,int)), this, SLOT(currentCell(int,int)));
  QObject::connect(upButton, SIGNAL(clicked()), this, SLOT(moveEntryUp()));
  QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addModule()));
  QObject::connect(uploadBox, SIGNAL(toggled(bool)), this, SLOT(getPDBLogin()));
  QObject::connect(runPDB, SIGNAL(toggled(bool)), this, SLOT(showPDB()));
  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(plotsBrwsButton, SIGNAL(clicked()), this, SLOT(browsePlotsPath()));
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeCell()));
  QObject::connect(restrStdTest, SIGNAL(clicked()), this, SLOT(restrictStdList()));

  int i;
  m_loadStd.clear();
  StdTestWin tmpwin(this,"tmptestwin");
  for(i=1;i<NTYPES;i++){
    meastypeBox->insertItem(tmpwin.prefix[i]);
  }
  m_tw = dynamic_cast<TopWin*>(parent);
  if(m_tw!=0){
    m_PDBForm = new PDBForm(m_tw,"PDB Form Manager",TRUE,Qt::WType_TopLevel);
    if(m_tw->options->m_Opts->m_DefDataPath!="."){
      pathToModules->setText(m_tw->options->m_Opts->m_DefDataPath.c_str());
      pathToRoot->setText((m_tw->options->m_Opts->m_DefDataPath+"%m/data/%t/%i/").c_str());
    }
    for(int i=0;i<tmpwin.NSLOTS;i++)
      m_loadStd.push_back(true);
  }else 
    m_PDBForm = 0;

  m_slc = new StdListChoice(this,"std_choice", false, 0, m_tw);

  moduleTable->setColumnWidth(0,280);
  moduleTable->setColumnWidth(1,150);
  m_currCol = -1;
  m_currRow = -1;

  m_lp = new LoginPanel(this,"loginpanel");
  m_lp->HostLabel->setText("Login details for PDB:");
  m_lp->HostName->hide();
  m_lp->UserID->setText("");
  m_lp->Password->setText("");
  
  m_lst = new LoadStave(this,"getStaveSN");
  m_lst->mtypeLabel->hide();
  m_lst->measType->hide();
  m_lst->okButton->setEnabled(true);
  if(m_tw!=0 && m_tw->options->m_Opts->m_DefDataPath!=".")
    m_lst->rootFixedName->setText(m_tw->options->m_Opts->m_DefDataPath.c_str());
  disconnect( m_lst->rootFixedName, SIGNAL( textChanged(const QString&) ), m_lst, SLOT( checkPaths() ) );

  cutsBox->insertItem("FLEX cuts");
  for(int i=1;i<100; i++){
    QString clabel = MAEngine::getSpecialCutLabel(i);
    if(clabel=="Default cuts") break;
    cutsBox->insertItem(clabel);
  }
  cutsBox->insertItem("use current cut set");

  uploadBox->hide(); //obsolete
  return;
}
MultiLoad::~MultiLoad(){
}
void MultiLoad::addModule(){
  ModWin getmod(this,"New Module", TRUE, Qt::WType_TopLevel);
  getmod.ModuleName->setFocus();
  if(getmod.exec()==1 && !getmod.ModuleName->text().isEmpty()){
    addModule(getmod.ModuleName->text().latin1());
  }
  return;
}
void MultiLoad::addModule(const char *mname, const char *staveSN, int pos){
  int nRows=moduleTable->numRows()+1;
  moduleTable->setNumRows(nRows);
  moduleTable->setText(nRows-1,0,mname);
  if(staveSN!=0){
    moduleTable->setText(nRows-1,1,staveSN);
    if(pos>=0)
      moduleTable->setText(nRows-1,2,QString::number(pos));
  }
  return;
}
void MultiLoad::browseModPath(){
  QString path = Q3FileDialog::getExistingDirectory(pathToModules->text(),this,"Multi Load",
						   "select module dir",FALSE);
  pathToModules->setText(path);
  pathToRoot->setText(path+"%m/data/%t/%i/");
}
void MultiLoad::showPDB(){
  if(m_PDBForm!=0 && runPDB->isChecked()){
    //m_PDBForm->savePlots->setEnabled(false);
    m_PDBForm->removeFiles->setEnabled(false);
    // allow setting of upload stuff
    m_PDBForm->TabWidget->setTabEnabled(m_PDBForm->TabWidget->page(4),true);
    m_PDBForm->setWWW();
    if(m_PDBForm->WWWcheckBox->isEnabled()){
      m_PDBForm->multiModeLabel->setEnabled(true);
      m_PDBForm->webUploadMode->setEnabled(true);
      if(m_tw!=0)
	m_PDBForm->webUploadMode->setCurrentItem(m_tw->options->m_Opts->m_webUploadOpt);
    }
    m_PDBForm->exec();
    checkPDBpath();
  }
}
void MultiLoad::meastypeChanged(){
  QString sind;
  sind.sprintf("%02d",measIndex->value());
  if(meastypeBox->currentItem()>0)
    ambushDir->setText("data/ambush_"+meastypeBox->currentText()+sind+"/");
  else
    ambushDir->setText("data/ambush/");
  m_slc->setType(meastypeBox->currentItem());
  m_loadStd.clear();
  for(int i=0;i<(int)m_slc->m_checkBoxes.size();i++)
    m_loadStd.push_back(m_slc->m_checkBoxes[i]->isEnabled() && m_slc->m_checkBoxes[i]->isChecked());
}
void MultiLoad::addRoot(){
  QString path=QString::null;
  int nRows=moduleTable->numRows()+1;
  if(m_tw!=0 && m_tw->options->m_Opts->m_DefDataPath!=".")
    path = m_tw->options->m_Opts->m_DefDataPath.c_str();
  QStringList filter("ROOT file (*.root)");
  filter += "Any file (*.*)";
  Q3FileDialog fdia(path, QString::null,this,"select data file",TRUE);
  fdia.setFilters(filter);
  path=QString::null;
  if(fdia.exec() == QDialog::Accepted){
    moduleTable->setNumRows(nRows);
    moduleTable->setText(nRows-1,0,fdia.selectedFile());
  }
}
void MultiLoad::removeCell(){
  if(m_currCol<0 || m_currRow<0) {
    QMessageBox::warning(this,"MultiLoad","You must select a cell before trying to remove it!");
    return;
  }
  int nRows=moduleTable->numRows();
  for(int col=0;col<moduleTable->numCols();col++){
    for(int i=m_currRow; i<nRows-1; i++)
      moduleTable->setText(i,col,moduleTable->text(i+1,col));
  }
  moduleTable->setNumRows(nRows-1);
  m_currCol = moduleTable->currentColumn();
  m_currRow = moduleTable->currentRow();
}
void MultiLoad::currentCell(int col,int row){
  m_currCol = row;
  m_currRow = col;
}
void MultiLoad::getPDBLogin(){
  checkPDBpath();
  // PDB login panel at this point is obsolete
}
void MultiLoad::addStave(){
  if(m_lst->exec()==QDialog::Accepted){
    QString userPDB,passwdPDB;
    if(m_tw!=0){
      userPDB = m_tw->options->m_Opts->m_userPDB.c_str();
      passwdPDB = m_tw->options->m_Opts->m_passwdPDB.c_str();
    }
    if(userPDB.isEmpty() || passwdPDB.isEmpty()){
      m_lp->exec();
      userPDB = m_lp->UserID->text();
      passwdPDB = m_lp->Password->text();
    }
    CDatabaseCom cdb(userPDB.latin1(),passwdPDB.latin1(),
		     (m_lst->staveSN->text()).latin1());
    QApplication::setOverrideCursor(Qt::waitCursor);
    std::vector<std::string> modSNs = cdb.getStaveSNs(*(m_tw->getApp()));
    int posCount=0;
    for(std::vector<std::string>::iterator it=modSNs.begin(); it!=modSNs.end();it++){
      posCount++;
      QString tmpstr = (*it).c_str();
      if(m_lst->loadRoot->isChecked() && !m_lst->rootFixedName->text().isEmpty()
	 && !m_lst->rootVarName->text().isEmpty() && m_lst->rootVarName->text().right(5)==".root"){
	QString tmp2Name, tmpName = m_lst->rootVarName->text();
	tmpName.replace("M%m","M"+tmpstr);
	tmpName.replace("%m",tmpstr);
	tmpName.replace("%s","20210020"+tmpstr);
	int ind=1, maxInd=0;
	QString myindex;
	for(ind=1;ind<16;ind++){
	  tmp2Name = tmpName;
	  myindex.sprintf("%02d",ind);
	  tmp2Name.replace("%i",myindex);
	  FILE *tmpF = fopen((m_lst->rootFixedName->text()+tmp2Name).latin1(),"r");
	  if(tmpF!=0){
	    fclose(tmpF);
	    maxInd = ind;
	  }
	}
	if(maxInd>0){
	  myindex.sprintf("%02d",maxInd);
	  tmpName.replace("%i",myindex);
	  addModule((m_lst->rootFixedName->text()+tmpName).latin1(), m_lst->staveSN->text(), posCount);
	}
      } else
	addModule(tmpstr.latin1(), m_lst->staveSN->text(), posCount);
    }
  }
  QApplication::restoreOverrideCursor();
}
void MultiLoad::moveEntryUp(){
  if(m_currCol<0 || m_currRow<0) {
    QMessageBox::warning(this,"MultiLoad","You must select a cell before trying to move it!");
    return;
  }
  if(m_currRow==0) {
    QMessageBox::warning(this,"MultiLoad","Current cell is at top, can't move up.");
    return;
  }
  for(int col=0;col<moduleTable->numCols();col++){
    QString moveTxt = moduleTable->text(m_currRow,col);
    QString origTxt = moduleTable->text(m_currRow-1,col);
    moduleTable->setText(m_currRow-1,col,moveTxt);
    moduleTable->setText(m_currRow,col,origTxt);
  }
}
void MultiLoad::moveEntryDown(){
  if(m_currCol<0 || m_currRow<0) {
    QMessageBox::warning(this,"MultiLoad","You must select a cell before trying to move it!");
    return;
  }
  if(m_currRow==(moduleTable->numRows()-1)) {
    QMessageBox::warning(this,"MultiLoad","Current cell is at bottom, can't move down.");
    return;
  }
  for(int col=0;col<moduleTable->numCols();col++){
    QString moveTxt = moduleTable->text(m_currRow,col);
    QString origTxt = moduleTable->text(m_currRow+1,col);
    moduleTable->setText(m_currRow+1,col,moveTxt);
    moduleTable->setText(m_currRow,col,origTxt);
  }
}
void MultiLoad::checkPDBpath(){
  if(uploadBox->isChecked()){
    if(m_PDBForm!=0){
      int posm = m_PDBForm->outputPath->text().find("%m");
      int poss = m_PDBForm->outputPath->text().find("%s");
      if(posm<0 && poss<0){
	QMessageBox::warning(this,"no_upload","PDB upload is requested but \noutput path is not module specific:\n"
			     "please use wildcards %m or %s in the path");
	uploadBox->setChecked(false);
      }
    } else
      uploadBox->setChecked(false);
  }
}
void MultiLoad::addModuleList(){
  m_lst->browseListButton->show();
  m_lst->staveSN->setText("");
  m_lst->snLabel->setText("Module SNs:      ");
  if(m_lst->exec()==QDialog::Accepted){
    QString modSN = m_lst->staveSN->text();
    int pos = modSN.find("; ");
    while(pos>=0){
      QString tmpstr = modSN.left(pos);
      if(m_lst->loadRoot->isChecked() && !m_lst->rootFixedName->text().isEmpty()
	 && !m_lst->rootVarName->text().isEmpty() && m_lst->rootVarName->text().right(5)==".root"){
	QString tmp2Name, tmpName = m_lst->rootVarName->text();
	tmpName.replace("M%m","M"+tmpstr);
	tmpName.replace("%m",tmpstr);
	tmpName.replace("%s","20210020"+tmpstr);
	int ind=1, maxInd=0;
	QString myindex;
	for(ind=1;ind<16;ind++){
	  tmp2Name = tmpName;
	  myindex.sprintf("%02d",ind);
	  tmp2Name.replace("%i",myindex);
	  FILE *tmpF = fopen((m_lst->rootFixedName->text()+tmp2Name).latin1(),"r");
	  if(tmpF!=0){
	    fclose(tmpF);
	    maxInd = ind;
	  }
	}
	if(maxInd>0){
	  myindex.sprintf("%02d",maxInd);
	  tmpName.replace("%i",myindex);
	  addModule((m_lst->rootFixedName->text()+tmpName).latin1());
	}
      } else
	addModule(tmpstr.latin1());
      modSN.remove(0,pos+2);
      pos = modSN.find("; ");
    }
  }
  m_lst->browseListButton->hide();
  m_lst->staveSN->setText("2021200001");
  m_lst->snLabel->setText("SN of stave/sector     ");
}
void MultiLoad::restrictStdList(){
  m_slc->exec();
  m_loadStd.clear();
  for(int i=0;i<(int)m_slc->m_checkBoxes.size();i++)
    m_loadStd.push_back(m_slc->m_checkBoxes[i]->isEnabled() && m_slc->m_checkBoxes[i]->isChecked());
}
void MultiLoad::browsePlotsPath()
{
  QString path = Q3FileDialog::getExistingDirectory(pathToModules->text(),this,"Multi Load",
						   "select plots dir",FALSE);
  plotsPath->setText(path);
}
