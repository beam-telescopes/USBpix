#include <qstring.h>
#include <q3filedialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3table.h>

#include "LoadStave.h"

LoadStave::LoadStave( QWidget* parent, const char* , bool , Qt::WFlags fl )
  : QDialog(parent,fl){
  //  : LoadStaveBase(parent,name,modal,fl){

  setupUi(this);
  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(browseRootPath()));
  QObject::connect(browseOutButton, SIGNAL(clicked()), this, SLOT(browseOutPath()));
  QObject::connect(rootFixedName, SIGNAL(textChanged(QString)), this, SLOT(checkPaths()));
  QObject::connect(loadPhaseButton, SIGNAL(clicked()), this, SLOT(loadPhase()));
  QObject::connect(measType, SIGNAL(activated(int)), this, SLOT(typeChanged()));
  QObject::connect(staveSN, SIGNAL(returnPressed()), this, SLOT(accept()));
  QObject::connect(browseListButton, SIGNAL(clicked()), this, SLOT(getModuleList()));

  outputName->hide();
  outputLabel->hide();
  browseOutButton->hide();
  browseListButton->hide();
  outputFile->hide();
  outputFileLabel->hide();
  phaseTable->hide();
  phaseLabel->hide();
  loadPhaseButton->hide();
}

LoadStave::~LoadStave(){
}

void LoadStave::browseRootPath(){
  QString path = Q3FileDialog::getExistingDirectory(rootFixedName->text(),this,"Multi Load",
						   "select ROOT base dir",FALSE);
  rootFixedName->setText(path);
  if(outputName->text().isEmpty())
    outputName->setText(path);
  okButton->setEnabled(true);
}
void LoadStave::browseOutPath(){
  QString path = Q3FileDialog::getExistingDirectory(outputName->text(),this,"Multi Load",
						   "select ROOT base dir",FALSE);
  outputName->setText(path);
}
void LoadStave::checkPaths(){
  okButton->setEnabled(!rootFixedName->text().isEmpty());
}
void LoadStave::loadPhase(){
  int phase1, phase2, nRow;
  char line[1000];
  QStringList filter("Text file (*.txt)");
  filter += "Any file (*.*)";
  Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted){
    FILE *input = fopen(fdia.selectedFile().latin1(),"r");
    if(input!=0){
      while(fgets(line,1000,input)!=0){
	sscanf(line,"%d %d",&phase1, &phase2);
	nRow = phaseTable->numRows()+1;
	phaseTable->setNumRows(nRow);
	phaseTable->setText(nRow-1,0,QString::number(phase1));
	phaseTable->setText(nRow-1,1,QString::number(phase2));
      }
      fclose(input);
    }
  }
}
void LoadStave::typeChanged(){
  QString outName;
  outName = "%m_"+ measType->currentText();
  if(loadRoot->isChecked())
    outName += "_source.cfg";
  else
    outName += "_tuned.cfg";
  outputFile->setText(outName);
}
void LoadStave::getModuleList(){
  char line[2000];
  QStringList filter("Text file (*.txt)");
  filter += "Any file (*.*)";
  Q3FileDialog fdia(QString::null,QString::null,this,"select file with module list",TRUE);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted){
    FILE *in = fopen(fdia.selectedFile().latin1(),"r");
    while(fgets(line,2000,in)!=0){
      if(line[strlen(line)-1]=='\n')
	line[strlen(line)-1]='\0'; // strip trailing \n
      QString newTxt = staveSN->text();
      if(newTxt.isEmpty())
	newTxt = line;
      else{
	newTxt += "; ";
	newTxt +=  line;
      }
      staveSN->setText(newTxt);
    }
    fclose(in);
    if(!staveSN->text().isEmpty())
      staveSN->setText(staveSN->text() + "; ");
  }
}
