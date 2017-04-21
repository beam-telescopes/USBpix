#include "DataStuff.h"
#include "RegPanel.h"
#include "ModTree.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qlineedit.h>

#include <TH1F.h>

RegPanel::RegPanel( ModItem *item, QWidget* parent, const char* , bool , Qt::WFlags fl)
 : QDialog(parent,fl){
  // : RegPanelBase(parent,name,modal,fl){

  setupUi(this);
  QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(chipBox, SIGNAL(valueChanged(int)), this, SLOT(updateResult(int)));

  m_item = item;
  if(item!=0 && item->ModParent()!=0)
    topLabel->setText("Register test info for module " + item->ModParent()->text(0));

  m_CPedit  [0] = bitsCP0;
  m_CPbutton[0] = statusCP0;
  m_CPedit  [1] = bitsCP1;
  m_CPbutton[1] = statusCP1;
  m_CPedit  [2] = bitsCP2;
  m_CPbutton[2] = statusCP2;
  m_CPedit  [3] = bitsCP3;
  m_CPbutton[3] = statusCP3;
  m_CPedit  [4] = bitsCP4;
  m_CPbutton[4] = statusCP4;
  m_CPedit  [5] = bitsCP5;
  m_CPbutton[5] = statusCP5;
  m_CPedit  [6] = bitsCP6;
  m_CPbutton[6] = statusCP6;
  m_CPedit  [7] = bitsCP7;
  m_CPbutton[7] = statusCP7;
  m_CPedit  [8] = bitsCP8;
  m_CPbutton[8] = statusCP8;

  updateResult(0);

}
RegPanel::~RegPanel(){
}
void RegPanel::updateResult(int chip){
  bool allOK = true;
  unsigned int j;
  unsigned int cont;
  QString bitStream="";

  // reset panel
  statusGR->setText("?");
  statusGR->setPaletteBackgroundColor(QColor("cyan"));
  bitsGR->setText("");
  for(j=0;j<9;j++){
    m_CPbutton[j]->setText("?");
    m_CPbutton[j]->setPaletteBackgroundColor(QColor("cyan"));
    m_CPedit[j]->setText("");
  }

  if(m_item==0 || chip<0 || chip>(NCHIP-1)) return;

  TH1F *regHi = m_item->GetDID()->GetSpecHi(chip);
  if(regHi==0) return;

  // GR bits
  allOK = true;
  for(j=10;j<15;j++){
    int maxBits = 15;
    if(j==14) maxBits = 14;
    cont = (unsigned int)regHi->GetBinContent(j);
    bitStream += bitToString(cont, allOK, maxBits);
  }

  bitsGR->setText(bitStream);

  if(allOK){
    statusGR->setText("OK");
    statusGR->setPaletteBackgroundColor(QColor("green"));
  }else{
    statusGR->setText("FAIL");
    statusGR->setPaletteBackgroundColor(QColor("red"));
  }


  // pixel reg. bits
  for(j=0;j<9;j++){
    cont = (unsigned int)regHi->GetBinContent(1+j);
    allOK = true;
    m_CPedit[j]->setText(bitToString(cont, allOK, 14));
    
    if(allOK){
      m_CPbutton[j]->setText("OK");
      m_CPbutton[j]->setPaletteBackgroundColor(QColor("green"));
    }else{
      m_CPbutton[j]->setText("FAIL");
      m_CPbutton[j]->setPaletteBackgroundColor(QColor("red"));
    }
  }
}
QString RegPanel::bitToString(unsigned int bits, bool &allOK, int maxBits){
  QString bitStream="";
  int i;
  for(i=0;i<31;i++){
    if(bits & (1<<i)){
      bitStream += "1";
    }else{
      bitStream += "0";
      allOK = false;
    }
    if(maxBits<0 && ((int)bits)<(1<<i)) break;
    if(maxBits>=0 && i==(maxBits-1)) break;
  }
  return bitStream;
}
