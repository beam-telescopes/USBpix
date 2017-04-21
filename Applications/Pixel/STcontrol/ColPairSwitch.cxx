#include <QPushButton>
#include <QPalette>
#include <QObject>
#include <QString>

#include "ColPairSwitch.h"

using namespace std;

ColourButton::ColourButton(const QString &text, QWidget *parent) : QPushButton(text, parent){
  setMaximumSize(QSize(22, 22));
  setAutoFillBackground(false);
  setStyleSheet("background-color: rgb(255, 0, 0);");
  setCheckable(true);
  setAutoRepeat(false);
  setAutoExclusive(false);
  setDefault(false);
  setFlat(false);
  QObject::connect(this, SIGNAL(clicked()),  this, SLOT(setState()));
}
ColourButton::~ColourButton(){
}
void ColourButton::setState(){
  if(isChecked())
    setStyleSheet("background-color: rgb(0, 255, 0);");
  else
    setStyleSheet("background-color: rgb(255, 0, 0);");
}

ColPairSwitch::ColPairSwitch(QWidget* parent, Qt::WindowFlags fl, int nButtons)
  : QDialog(parent,fl), m_nButtons(nButtons){

  setupUi(this);
  QObject::connect(allOnButton, SIGNAL(clicked()),  this, SLOT(allOn()));
  QObject::connect(allOffButton, SIGNAL(clicked()), this, SLOT(allOff()));

  for(int i=0;i<m_nButtons;i++){
    m_buttons.push_back(new ColourButton(QString::number(i+1), this));
    m_buttons[i]->setObjectName("PushButton_"+QString::number(i+1));
    hboxLayout->addWidget(m_buttons[i]);
  }
}

ColPairSwitch::~ColPairSwitch(){
}

void ColPairSwitch::allOff(){
  for(int i=0;i<m_nButtons;i++){
    m_buttons[i]->setChecked(false);
    m_buttons[i]->setState();
  }
}
void ColPairSwitch::allOn(){
  for(int i=0;i<m_nButtons;i++){
    m_buttons[i]->setChecked(true);
    m_buttons[i]->setState();
  }
}
