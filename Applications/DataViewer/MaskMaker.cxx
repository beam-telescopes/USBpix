#include "MaskMaker.h"
#include <PixScan.h>

MaskMaker::MaskMaker( QWidget* parent, Qt::WindowFlags fl) : QDialog(parent, fl){
  setupUi(this);
}
MaskMaker::~MaskMaker(){
}
void MaskMaker::setHistoLabel(int histoType){
  histoLabel->setText("unknown value type");
  PixLib::PixScan ps;
  for(std::map<std::string, int>::iterator it = ps.getHistoTypes().begin();
      it!=ps.getHistoTypes().end(); it++){
    if(it->second==histoType) histoLabel->setText(it->first.c_str());
  }
}
double MaskMaker::getMin(){
  return minSpinBox->value();
}
double MaskMaker::getMax(){
  return maxSpinBox->value();
}
