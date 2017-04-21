#include "SelectionPanel.h"
#include <q3groupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

SelectionPanel::SelectionPanel( QWidget* parent, const char* , bool , Qt::WFlags fl) 
  : QDialog(parent,fl){
  //  : SelectionPanelBase(parent,name,modal,fl){
  setupUi(this);
  QObject::connect(OKButton, SIGNAL(clicked()), this, SLOT(OKpressed()));
  QObject::connect(AllButton, SIGNAL(clicked()), this, SLOT(SelectAll()));
  QObject::connect(NoneButton, SIGNAL(clicked()), this, SLOT(DeselectAll()));
  QObject::connect(CancelButton, SIGNAL(clicked()), this, SLOT(CancPressed()));
  QObject::connect(buttonANDOR, SIGNAL(toggled(bool)), this, SLOT(andorLabel(bool)));

  cutCheck.clear();
}
SelectionPanel::~SelectionPanel(){
}
void SelectionPanel::OKpressed(){
  int sel_mask=0;
  for(unsigned int i=0; i<cutCheck.size();i++){
    if(cutCheck[i]->isChecked())
      sel_mask += (1<<i);
  }
  done(sel_mask);
}
void SelectionPanel::CancPressed(){
  done(-1);
}
void SelectionPanel::SetAll(bool doset){
  for(unsigned int i=0; i<cutCheck.size();i++){
    if(cutCheck[i]->isEnabled())
      cutCheck[i]->setChecked(doset);
  }
}
void SelectionPanel::AddCheckBox(const char *name){
  int id = cutCheck.size();
  cutCheck.push_back(new QCheckBox(name,CheckGroup,"cb"+QString::number(id)));
  cutCheck[id]->setChecked(true);
  cutCheck[id]->setGeometry(20,40+20*id,250,20);
  setGeometry(geometry().x(),geometry().y(),300,200+20*cutCheck.size());
  if(QString(name).find("AND with above")>=0){
    cutCheck[id]->setEnabled(false);
    cutCheck[id]->setChecked(false);
    cutCheck[id]->hide();
  }
  repaint();
}
void SelectionPanel::andorLabel(bool isON){
  if(isON)
    buttonANDOR->setText("AND of all selected cuts");
  else
    buttonANDOR->setText("OR of all selected cuts");
}
