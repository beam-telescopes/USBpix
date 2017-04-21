#include "modstatus.h"
#include <stdlib.h>
#include <qmessagebox.h>
#include <iostream>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qtextbrowser.h>


using namespace std;


ModStatus::ModStatus(QWidget* parent , const char* name, bool modal , WFlags fl)
  :ModStatusBase(parent, name, modal, fl)
{
 

}


void ModStatus::getHisto(const QString &link)
{
  //cout<<"\n&&&"<< link<<endl;
 
  emit(s_startMA(link.section(",",0,0).mid(1).latin1(), 
		 link.section(",",1,1).latin1(),
		 le_mod->text().latin1()));
  // QString sel = te_scanoutput->selectedText();
//   sel.replace(" ","");
//   std::string ssel = sel.latin1();
//   PixScan *ps = new PixScan();
//   std::map<std::string, int> hiTypes = ps->getHistoTypes();
  
//   // PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType) hiTypes[(*cIT)->getHistoType()];


//   if(hiTypes[ssel]>0 || ssel == "OCCUPANCY") //because any text and OCCUPANCY have 0 
//     {
      
//       emit(s_startMA(le_mod->text().latin1(),hiTypes[ssel],para));
//     }
//   else
//     return;
}
