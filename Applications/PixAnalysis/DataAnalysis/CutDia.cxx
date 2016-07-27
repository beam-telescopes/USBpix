#include "CutDia.h"
#include <stdlib.h>
#include <qmessagebox.h>
#include <PixScan.h>
#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include "Module.h"







CutDia::CutDia(QWidget* parent , const char* name, bool modal, WFlags fl)
  :cutdiabase(parent,name,modal,fl)
{
  PixScan *ps = new PixScan();
  std::map<std::string, int> hiTypes = ps->getHistoTypes(); 
  std::map<std::string, int> scanTypes = ps->getScanTypes(); 
  Modul *mod = new Modul("a","/b/d/512345/","c");
  std::map<std::string, int> actTypes = mod->getActionTypes(); 
  std::map<std::string, int> pixTypes = mod->getPixelTypes();


  cb_histotype->clear();

  for(std::map<std::string,int>::iterator IT=hiTypes.begin(); IT!=hiTypes.end(); IT++)
      {
	cb_histotype->insertItem((*IT).first.c_str());
      }

  cb_testtype->clear();

  for(std::map<std::string,int>::iterator IT=scanTypes.begin(); IT!=scanTypes.end(); IT++)
      {
	cb_testtype->insertItem((*IT).first.c_str());
      }
  cb_testtype->insertItem("ALL TYPES");
  cb_testtype->insertItem("BAD PIXEL");

  cb_actiontype->clear();

  for(std::map<std::string,int>::iterator IT=actTypes.begin(); IT!=actTypes.end(); IT++)
      {
	cb_actiontype->insertItem((*IT).first.c_str());
      }
 
  cb_pixeltype->clear();

  for(std::map<std::string,int>::iterator IT=pixTypes.begin(); IT!=pixTypes.end(); IT++)
      {
	cb_pixeltype->insertItem((*IT).first.c_str());
      }


  if(cb_testtype->currentText()=="BAD PIXEL")
    enablebadpixtotal(false);
  else
    enablebadpixtotal(true);
  
  if(actTypes[cb_actiontype->currentText().latin1()]==Modul::NBAD_PIXEL)
    {
      //le_badpixmin->setEnabled();
      enablebadpix(true);
    }
  else
    enablebadpix(false);
 

  connect(cb_testtype,SIGNAL(activated(const QString&)),this,SLOT(setTotalBadPixEnable()));

  
  delete mod;
  delete ps;
}


CutDia::~CutDia(){}


bool CutDia::setBadPixEnable()
{
   Modul *mod = new Modul("a","/b/d/512345/","c");
   std::map<std::string, int> actTypes = mod->getActionTypes();


   
   if(actTypes[cb_actiontype->currentText().latin1()]== Modul::NBAD_PIXEL)
     {
       //enable NBadPixMin/Max
       enablebadpix( true);
       return true;
     }
   else
     {
       //disable NBadPixMin/Max
       enablebadpix( false);
       return false;
     }
}
bool CutDia::setTotalBadPixEnable()
{
  if(cb_testtype->currentText()=="BAD PIXEL")
    {
      
      enablebadpixtotal(false);
      return true;
    }
  else
    {
      enablebadpixtotal(true);
      return false;
    }
}

void CutDia::enablebadpixtotal(bool ena)
{
  
   textLabel1_2->setEnabled(ena);
   textLabel1_2_2->setEnabled(ena);
   cb_histotype->setEnabled(ena);
   cb_actiontype->setEnabled(ena);
   

   if(ena==false)
     {  
       enablebadpix(false);
       textLabel2->setText("Min total BadPixel");
       textLabel3->setText("Max total BadPixel");
       cb_pixeltype->setEnabled(ena);
       textLabel1_3->setEnabled(ena);
     }
   else 
     {
       //textLabel2->setText("Min");
       //textLabel3->setText("Max");
       setBadPixEnable();
     }
}


void CutDia::enablebadpix(bool ena)
{
   le_badpixmin->setEnabled(ena);
   le_badpixmax->setEnabled(ena);
   labelBadPix1->setEnabled(ena);
   labelBadPix2->setEnabled(ena);
   cb_pixeltype->setEnabled(!ena);
   textLabel1_3->setEnabled(!ena);

   if(ena)
     {
       textLabel2->setText("Min in %");
       textLabel3->setText("Max in %");
     }
   else 
     {
       textLabel2->setText("Min");
       textLabel3->setText("Max");
     }
}
