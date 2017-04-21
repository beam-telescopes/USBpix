#include "ModuleCfgPage.h"
#include "STControlEngine.h"

#include <GeneralDBfunctions.h>

#include <PixConfDBInterface/RootDB.h>

#include <q3filedialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>

#include <vector>
#include <string>
#include <iostream>

ModuleCfgPage::ModuleCfgPage( QWidget* parent,  const char* , Qt::WindowFlags fl, int setup_type, int pp0_type )
    : QWidget( parent, fl ){
  //    : ModuleCfgPageBase( parent, name, fl ){
  setupUi(this);
  QObject::connect(BrowseButton, SIGNAL(clicked()), this, SLOT(browseClicked()));
  QObject::connect(SlotBox, SIGNAL(activated(int)), this, SLOT(slotChanged(int)));
  QObject::connect(SetupBox, SIGNAL(activated(int)), this, SLOT(defineSetup(int)));
  QObject::connect(stave_sectorBox, SIGNAL(activated(int)), this, SLOT(setStavePosMax()));
  QObject::connect(PP0Box, SIGNAL(activated(int)), this, SLOT(PP0Changed(int)));
  QObject::connect(SetupBox, SIGNAL(activated(int)), this, SLOT(setupChanged(int)));
  QObject::connect(PP0Box, SIGNAL(activated(int)), this, SLOT(saveLinks()));

  defineSetup(setup_type, pp0_type);
}
ModuleCfgPage::~ModuleCfgPage(){
}
void ModuleCfgPage::browseClicked(){
  QStringList filter;
  filter += "TruboDAQ Config file (*.cfg)";
  filter += "Any file (*.*)";
  Q3FileDialog fdia(QString::null,QString::null,this,"select data file",true);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted) {
    FileName->setText(fdia.selectedFile());
    emit haveFile();
  }
  std::string fname = fdia.selectedFile().latin1();
  ModName->setText(PixLib::getTdaqModuleName(fname).c_str());
}
void ModuleCfgPage::defineSetup(int type, int pp0){
  if(type<0 || type>SetupBox->count()) return;
  int i, j, in_o, out_o[4];
  bool isOK;

  QComboBox *obox[4]={Output0Box, Output1Box, Output2Box, Output3Box};

  in_o   = InputBox->currentText().toInt(&isOK);
  for(j=0;j<4;j++)
    out_o[j] = 16*(obox[j]->currentText().left(1).toInt(&isOK))
      +obox[j]->currentText().right(1).toInt(&isOK);

  SetupBox->setCurrentItem(type);
  SlotBox->setEnabled(true);
  InputBox->setEnabled(false);
  for(j=0;j<4;j++)
    obox[j]->setEnabled(false);
  SlotBox->clear();
  InputBox->clear();
  for(j=0;j<4;j++)
    obox[j]->clear();

  if(type<3){
    int slot_o=0;
    std::vector<int> in, out1, out2, out3, out4;
    PixLib::listLinks(in,out1,out2,out3,out4,type, pp0);
    for(i=0;i<(int)in.size();i++){
      if(in[i] == in_o) slot_o = i;
      SlotBox->insertItem(QString::number(i),i);
      InputBox->insertItem(QString::number(in[i]),i);
      Output0Box->insertItem(QString::number(out1[i]/16)+":"+QString::number(out1[i]%16),i);
      Output1Box->insertItem(QString::number(out2[i]/16)+":"+QString::number(out2[i]%16),i);
      Output2Box->insertItem(QString::number(out3[i]/16)+":"+QString::number(out3[i]%16),i);
      Output3Box->insertItem(QString::number(out4[i]/16)+":"+QString::number(out4[i]%16),i);
      /*
      Output1Box->insertItem(QString::number(out2[i]),i);
      Output2Box->insertItem(QString::number(out3[i]),i);
      Output3Box->insertItem(QString::number(out4[i]),i);
      */
    }
    SlotBox->setCurrentItem(slot_o);
    slotChanged(slot_o);
  } else{// free-style
    SlotBox->setEnabled(false);
    InputBox->setEnabled(true);
    Output0Box->setEnabled(true);
    Output1Box->setEnabled(true);
    Output2Box->setEnabled(true);
    Output3Box->setEnabled(true);
    for(i=0;i<48;i++)
      InputBox->insertItem(QString::number(i),i);
    InputBox->setCurrentItem(in_o);
    for(i=0;i<32;i++){
      for(j=0;j<4;j++)
	obox[j]->insertItem(QString::number(i/4)+":"+QString::number(i%4),i);
    }
    for(j=0;j<4;j++){
      i = out_o[j]%16+4*(out_o[j]/16);
      obox[j]->setCurrentItem(i);
    }
  }
  return;
}
void ModuleCfgPage::slotChanged(int slot){
  switch(SetupBox->currentItem()){
  case 0: // only switch output 0
  default:
    InputBox->setCurrentItem(slot);
    Output0Box->setCurrentItem(slot);
    Output1Box->setCurrentItem(slot);
    Output2Box->setCurrentItem(slot);
    Output3Box->setCurrentItem(slot);
    break;
  case 1:
    InputBox->setCurrentItem(slot);
    Output0Box->setCurrentItem(slot);
    Output1Box->setCurrentItem(slot);
    Output2Box->setCurrentItem(slot);
    Output3Box->setCurrentItem(slot);
    break;
  case 2:
    InputBox->setCurrentItem(slot);
    Output0Box->setCurrentItem(slot);
    Output1Box->setCurrentItem(slot);
    Output2Box->setCurrentItem(slot);
    Output3Box->setCurrentItem(slot);
    break;
  case 3: // free-style: output does not depend on input
    break;
  }
  return;
}
void ModuleCfgPage::setStavePosMax(){
  int assy_type = stave_sectorBox->currentItem();
  switch(assy_type){
  case 1: // stave
    staveID->setEnabled(true);
    modPosition->setEnabled(true);
    modPosition->setMaxValue(13);
    break;
  case 2: // sector
    staveID->setEnabled(true);
    modPosition->setEnabled(true);
    modPosition->setMaxValue(6);
    break;
  default:
  case 0: // undef./unknown
    staveID->setEnabled(false);
    modPosition->setEnabled(false);
  }
}

