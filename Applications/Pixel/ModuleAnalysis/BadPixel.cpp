#include "BadPixel.h"
#include "DataStuff.h"
#include "TopWin.h"
#include "ModTree.h"
#include "FileTypes.h"

#include <TH2F.h>

#include <q3filedialog.h>
#include <qstring.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qclipboard.h>

BadPixel::BadPixel( ModItem *item, QWidget* parent, const char* , bool , Qt::WFlags fl)
  : QDialog(parent,fl){
//  : BadPixelBase(parent,name,modal,fl){

  setupUi(this);
  QObject::connect(CloseButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(WrtieButt, SIGNAL(clicked()), this, SLOT(WriteToFile()));
  QObject::connect(MaskButt, SIGNAL(clicked()), this, SLOT(CreateMask()));
  QObject::connect(PixelList, SIGNAL(selectionChanged()), this, SLOT(copyToCB()));

  m_item = item;
  if(m_item==NULL) MaskButt->hide();
  // set up table with default headers
  Clear();
}

BadPixel::~BadPixel(){
}

void BadPixel::WriteToFile(){
  int i,j;
  Q3FileDialog fdia(QString::null, "Any file (*.*)",this,"select data file",TRUE);
  fdia.setMode(Q3FileDialog::AnyFile);
  if(fdia.exec() == QDialog::Accepted){
    FILE *out = fopen(fdia.selectedFile().latin1(),"w");
    if(out!=NULL){
      for(j=0;j<PixelList->numCols();j++){
	fprintf(out,"%s",PixelList->horizontalHeader()->label(j).latin1());
	if(j<PixelList->numCols()-1) fprintf(out,"\t");
	else                         fprintf(out,"\n");
      }
      for(i=0;i<PixelList->numRows();i++){
	for(j=0;j<PixelList->numCols();j++){
	  fprintf(out,"%s",PixelList->text(i,j).latin1());
	  if(j<PixelList->numCols()-1) fprintf(out,"\t");
	  else                         fprintf(out,"\n");
	}
      }
      fclose(out);
    }
  }
  return;
}

void BadPixel::Clear(){
  PixelList->setNumRows(1);
  PixelList->setNumCols(4);
  PixelList->horizontalHeader()->setLabel(0,"chip");
  PixelList->horizontalHeader()->setLabel(1,"col");
  PixelList->horizontalHeader()->setLabel(2,"row");
  PixelList->horizontalHeader()->setLabel(3,"cut value");
  for(int i=0;i<4;i++)
    PixelList->setText(0,i,"");
  PixelList->setNumRows(0);
  return;
}

void BadPixel::Init(const char *name, const char *title){
  Clear();
  PixelList->horizontalHeader()->setLabel(3,name);
  Headline->setText(title);
  return;
}

void BadPixel::CreateMask(){
  if(m_item==0) return;
  //  m_parent->MaskFromTable();
  ModItem *datit, *parnt, *test;
  const int nentries=PixelList->numRows(), nmaxpix=5000;
  int i,pixarr[nmaxpix],binx,biny,chip,col,row;
  if(nentries>nmaxpix){
	  QMessageBox::warning(NULL,"MAF","Too many bad pixels, can't proceed");
	return;
  }
  TH2F tmphi("a","b",8*NCOL,-0.5,(float)(NCOL*8)-0.5,2*NROW,-0.5,(float)(2*NROW)-0.5);
  for(i=0;i<nentries;i++){
    sscanf(PixelList->text(i,0).latin1(),"%d",&chip);
    sscanf(PixelList->text(i,1).latin1(),"%d",&col);
    sscanf(PixelList->text(i,2).latin1(),"%d",&row);
    m_item->GetDataItem()->PixCCRtoBins(chip,col,row,binx,biny);
    pixarr[i] = tmphi.GetBin(binx,biny);
  }
  m_item->GetDID()->CreateMask(pixarr,nentries);
  parnt = m_item->ModParent();
  datit = m_item->DatParent();

  test = (ModItem*) datit->firstChild();
  while(test!=NULL){
    parnt = test;
    if(test->text(0)=="Masked channels")
      delete test;
    test = (ModItem*) parnt->nextSibling();
  }
  parnt = new ModItem(datit,  "Masked channels",  datit->GetDID(), NONE);    
  test  = new ModItem(parnt,"Plot of masked channels",    datit->GetDID(), MAMASK);    
  test  = new ModItem(parnt,"Write mask to file",         datit->GetDID(), SAVEMASK);
  test  = new ModItem(parnt,"Write inverse mask to file", datit->GetDID(), INVMASK);
  // allow only one mask to be created
  MaskButt->hide();
  return;
}
void BadPixel::AddTabLine(int xval, int yval, float value) {
  int i,j,chip,row,col,nRows=PixelList->numRows()+1;
  float con=value;
  i=xval; j=yval;
  m_item->GetDataItem()->PixXYtoCCR(i-1,j-1,&chip,&col,&row);
  PixelList->setNumRows(nRows);
  PixelList->setText(nRows-1,0,QString::number(chip,'f', 0));
  PixelList->setText(nRows-1,1,QString::number(col, 'f', 0));
  PixelList->setText(nRows-1,2,QString::number(row, 'f', 0));
  PixelList->setText(nRows-1,3,QString::number(con, 'f', 2));
  return;
}
void BadPixel::AddTabLine(QString *text) {
  int i,nRows=PixelList->numRows()+1;
  PixelList->setNumRows(nRows);
  for(i=0;i<PixelList->numCols();i++)
    PixelList->setText(nRows-1,i,text[i]);
  return;
}
void BadPixel::copyToCB(){
  int i,j, ncsel;
  QString cbText="";
  for(i=0;i<PixelList->numRows();i++){
    ncsel=0;
    for(j=0;j<PixelList->numCols();j++){
      if(PixelList->isSelected(i,j)){
	cbText+=PixelList->text(i,j)+"\t";
	ncsel++;
      }
    }
    if(ncsel>0){
      cbText.remove(cbText.length()-1,1);
      cbText+="\n";
    }
  }
  QClipboard *cb = QApplication::clipboard();
  cb->setText(cbText);
}
