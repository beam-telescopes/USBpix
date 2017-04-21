#include <qapplication.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <q3multilineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>

#include <TSystem.h>
#include <TH1F.h>

#ifdef HAVE_PLDB
#include <Histo.h>
#ifdef PIXEL_ROD // can link to PixScan
// the following three includes are neccessary for
// successful compilation under windows with PixConfDB...
// I wish I knew why!
#include <TKey.h>
#include <TSystem.h>
#include <qobject.h>
#include <PixConfDBInterface.h>
#include <RootDB.h>
#include <PixScan.h>
#include <Config/Config.h>
#endif
#endif

#include "ModTree.h"
#include "StdTestWin.h"
#include "TopWin.h"
#include "PixelDataContainer.h"
#include "DataStuff.h"
#include "FitWin.h"
#include "OptWin.h"
#include "OptClass.h"
#include "RootStuff.h"
#include "MAEngine.h"
#include "CutPanel.h"
#include "CDatabase.h"
#include "RootDataList.h"
#include "QAReport.h"
#include "LogClasses.h"

#include <string>
#include <sstream>

namespace MAEngine{

void LocalLoadStd(TopWin *tw,ModItem *item, StdTestWin &stdwin, QApplication *app, QLineEdit *textwin){
  std::string filename, label;
  int type, i, pos;

  textwin->setText("Loading standard data...");
  textwin->repaint();
  if(app!=0)
    app->processEvents();
  
  
  ModItem *Mit=0;
  std::vector<ModItem*> Mit_arr;
  std::vector<int> exists;
  
  // resize vectors
  Mit_arr.resize(stdwin.NSLOTS);
  exists.resize(stdwin.NSLOTS);
  for(i=0;i<stdwin.NSLOTS;i++){
    Mit_arr[i] = 0;
    exists[i]  = 0;
  }
  for(i=0;i<stdwin.NSLOTS;i++){
    if(!stdwin.m_items[i]->m_fname->isEnabled()){
      Mit_arr[i] = stdwin.Mit_arr[i];
      exists[i]  = 1;
    }
    Mit = 0;
    type = stdwin.m_items[i]->m_type;
    filename = stdwin.m_items[i]->m_fname->text().latin1();
    label = stdwin.m_items[i]->m_label->text().latin1();
    if(stdwin.TestTypeSel->currentItem()>0){
      label += " - ";
      label += (stdwin.prefix[stdwin.TestTypeSel->currentItem()] + " " + 
                QString::number(stdwin.Index->value())).latin1();
    }
    // load ToT raw data if requested
    if((i==8 && stdwin.ToTLo_raw->isChecked()) || 
       (i==9 && stdwin.ToTHi_raw->isChecked())){
      pos = filename.find("_cal.out");
      if(pos>=0){
        filename.erase(pos,filename.length());
        filename += "_0.tot";
        type=TOTFR;
      }
    } 
    if(type==ONEDSCAN){
      if(((i==21 || i==31 || i==32) && tw->options->m_Opts->m_runBadFitsIntime) ||
         ((i==34 || i==35) && tw->options->m_Opts->m_runBadFitsTzero)  ||
         ((i==1 || i==2 || i==5 || i==6 || i==18 || i==19 || i==20) && tw->options->m_Opts->m_runBadFitsThresh) ||
         (i==7 && tw->options->m_Opts->m_runBadFitsXtalk) ){  // load bad fits data if requested
        pos = filename.find("_sfits.out");
        int goodInd=-1;
        if(pos>=0){
          for(int ii=0;ii<NCHIP;ii++){
            std::stringstream aa;
            aa << ii;
            std::string tmp_fname = filename;
            tmp_fname.erase(pos,filename.length());
            tmp_fname += "_badfits_";
            tmp_fname += aa.str();
            tmp_fname += ".bin";
            FILE *tmpf = fopen(tmp_fname.c_str(),"r");
            if(tmpf!=0){
              fclose(tmpf);
              goodInd = ii;
              break;
            }
          }
          if(goodInd>=0){
            std::stringstream bb;
            bb << goodInd;
            filename.erase(pos,filename.length());
            filename += "_badfits_";
            filename += bb.str();
            filename += ".bin";
            type=ONEDFIT;
          }
        }
      }
    }
    if(stdwin.m_items[i]->m_fname->isEnabled() && stdwin.m_items[i]->m_fname->text().left(13)!="no file found"){
      if(!stdwin.m_items[i]->m_fname->text().isEmpty()){
        Mit = tw->LoadData(item, filename.c_str(),label.c_str(),-type);
        // fit bad fits data and remove raw data if requested
        if(type==ONEDFIT && stdwin.m_items[i]->m_type==ONEDSCAN){
          float **calfac, capfac;
          calfac = new float*[NCHIP];
          int chip;
          if((Mit->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" ||
              Mit->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff") &&
             Mit->GetDID()->GetLog()->MyScan->GetOuter()=="No Scan"){
            tw->m_FitWin->ChipCalBox->setChecked(true);
            tw->m_FitWin->CPCButt_pressed();
            for(chip=0;chip<NCHIP;chip++){
              calfac[chip] = new float[4];
              if(Mit->GetDID()->GetLog()->MyModule->CHigh)
                capfac  = Mit->GetDID()->GetLog()->MyModule->Chips[chip]->Chi;
              else
                capfac  = Mit->GetDID()->GetLog()->MyModule->Chips[chip]->Clo;
              if(Mit->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE"){
                calfac[chip][0] = capfac*Mit->GetDID()->GetLog()->MyModule->Chips[chip]->Vcal/0.160218;
                calfac[chip][2] = Mit->GetDID()->GetLog()->MyModule->Chips[chip]->VcalQu;
                calfac[chip][2] *= capfac/0.160218;
                calfac[chip][3] = Mit->GetDID()->GetLog()->MyModule->Chips[chip]->VcalCu;
                calfac[chip][3] *= capfac/0.160218;
              }else if(Mit->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff"){
                calfac[chip][0]  = Mit->GetDID()->GetLog()->MyModule->Chips[chip]->Clo*
                  Mit->GetDID()->GetLog()->extcal/0.160218;
                calfac[chip][2] = 0;
                calfac[chip][3] = 0;
              }else{
                calfac[chip][0] = 1;
                calfac[chip][2] = 0;
                calfac[chip][3] = 0;
              }
              calfac[chip][1] = 0;
              //        tw->m_FitWin->ChipCfac->setText(chip,0,QString::number(calfac,'f',2));
            }
            tw->m_FitWin->setCalTable(calfac);
            for(chip=0;chip<NCHIP;chip++){
              delete[] calfac[chip];
              calfac[chip] = 0;
            }
            delete[] calfac;
          } else{
            tw->m_FitWin->ChipCalBox->setChecked(false);
            tw->m_FitWin->CPCButt_pressed();
            tw->m_FitWin->CfacEdit->setText("1.0");
          }
          tw->m_FitWin->FitType->setCurrentItem(0);
          tw->m_FitWin->FitSelected(0);
          tw->m_FitWin->divndfBox->setChecked(false);
          tw->m_FitWin->ChiCut->setValue(200);
	  Mit = PerformFit(Mit,true,false,false,tw,app);
          tw->RemRaw(Mit,false);
        }
      }
      if(Mit!=0){
        Mit->GetDID()->SetStdtest(i + 100*stdwin.TestTypeSel->currentItem());
        textwin->setText("...loaded " + stdwin.m_items[i]->m_label->text() + "...");
        textwin->repaint();
        if(app!=0)
          app->processEvents();
      }
      Mit_arr[i] = Mit;
    }
    if(stdwin.doToTComb->isChecked()) exists[19]=-1;
  }
  textwin->setText("...finished loading, processing data now.");
  textwin->repaint();
  if(app!=0)
    app->processEvents();
  tw->ProcStd(Mit_arr, exists);
  textwin->setText("");
  tw->SetCurrMod((Q3ListViewItem*)item);
  return;
}

ModItem* PerformFit(ModItem *item, bool badfitonly, bool prompt_user, bool warn_user, TopWin *tw, QApplication *app, int min_chip, int max_chip){
  if(item==0) return 0;
  ModItem *parnt, *datit, *temp;
  datit = item->DatParent();
  parnt = item->ModParent();

  int error = 0, retval, fitchip, chip_s=0, chip_e=NCHIP;
  if(min_chip>=0 && max_chip>=0){
    chip_s = min_chip;
    chip_e = max_chip+1;
  }

  QString tmp;
  bool selfit=((retval=item->GetDID()->GetMap(-1,CHI)!=0) && badfitonly);

  // check if fit data exists, and ask user if it should be overwritten
  if((retval=item->GetDID()->GetMap(-1,PARA)!=0) && !badfitonly && warn_user){
    QMessageBox ays("Confirm delete",
                    "This will overwrite existing fit data.\nAre you sure?",
                    QMessageBox::Warning,QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
                    tw, "ays",TRUE,Qt::WType_TopLevel);
    error = ays.exec()!=QMessageBox::Ok;
  }
  if(error) return 0;

  // get fit options from user, load calib with what's in the log file
  int chip;
  float **calfac, capfac;
  calfac = new float*[NCHIP];
  if(item->GetDID()->GetLog()!=0 && prompt_user){
    if((item->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" ||
        item->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff") &&
       item->GetDID()->GetLog()->MyScan->GetOuter()=="No Scan"){
      tw->m_FitWin->ChipCalBox->setChecked(true);
      tw->m_FitWin->CPCButt_pressed();
      for(chip=0;chip<NCHIP;chip++){
        calfac[chip] = new float[4];
        if(item->GetDID()->GetLog()->MyModule->CHigh)
          capfac  = item->GetDID()->GetLog()->MyModule->Chips[chip]->Chi;
        else
          capfac  = item->GetDID()->GetLog()->MyModule->Chips[chip]->Clo;
        if(item->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE"){
          calfac[chip][0] = capfac*item->GetDID()->GetLog()->MyModule->Chips[chip]->Vcal/0.160218;
          calfac[chip][2] = item->GetDID()->GetLog()->MyModule->Chips[chip]->VcalQu;
          calfac[chip][2] *= capfac/0.160218;
          calfac[chip][3] = item->GetDID()->GetLog()->MyModule->Chips[chip]->VcalCu;
          calfac[chip][3] *= capfac/0.160218;
        }else if(item->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff"){
          calfac[chip][0]  = item->GetDID()->GetLog()->MyModule->Chips[chip]->Clo*
            item->GetDID()->GetLog()->extcal/0.160218;
          calfac[chip][2] = 0;
          calfac[chip][3] = 0;
        }else{
          calfac[chip][0] = 1;
          calfac[chip][2] = 0;
          calfac[chip][3] = 0;
        }
        calfac[chip][1] = 0;
        //        m_FitWin->ChipCfac->setText(chip,0,QString::number(calfac,'f',2));
      }
      tw->m_FitWin->setCalTable(calfac);
      for(chip=0;chip<NCHIP;chip++){
        delete[] calfac[chip];
        calfac[chip] = 0;
      }
      delete[] calfac;
    } else{
      tw->m_FitWin->ChipCalBox->setChecked(false);
      tw->m_FitWin->CPCButt_pressed();
      tw->m_FitWin->CfacEdit->setText("1.0");
    }
  }
  // suggest choice of fit function
  if(item->GetDID()->m_fittype>=0 && prompt_user)
    tw->m_FitWin->FitType->setCurrentItem(item->GetDID()->m_fittype);
  // apply settings
  tw->m_FitWin->FitSelected(tw->m_FitWin->FitType->currentItem());
  // show fit window
  if(prompt_user && tw->m_FitWin->exec()==0) return 0;
  TF1 *fitfunc = RMain::GetFitFunc(tw->m_FitWin->FitType->currentItem()); 
  
  // run the fit
  tw->textwin->setText("Performing fit now, please be patient...");
  tw->textwin->repaint();
  if(app!=0)
    app->processEvents();
  QApplication::setOverrideCursor(Qt::waitCursor);
  gSystem->Sleep(500);
  error = item->GetDID()->FitStd(tw->rootmain->GetCal(),tw->m_FitWin->FitType->currentItem(),-2,
				 (float)tw->m_FitWin->ChiCut->value(),
                                 0,0,selfit,tw->m_FitWin->divndfBox->isChecked());
  if(error<0){
    tmp.sprintf("Error while initialising fit: %s",DatSet::ErrorToMsg(error).c_str());
    if(tw->options->m_Opts->m_showwarn) QMessageBox::warning(tw,"Module Analysis",tmp);
    QApplication::restoreOverrideCursor();
    return 0;
  }

  for(fitchip=chip_s;fitchip<chip_e;fitchip++){
  //  for(fitchip=0;fitchip<NCHIP;fitchip++){
    tmp.sprintf("...fitting chip %d (%s,%s)...",fitchip,item->ModParent()->text(0).latin1(),item->DatParent()->text(0).latin1());
    tw->textwin->setText(tmp);
    tw->textwin->repaint();
    if(app!=0)
      app->processEvents();
    // prepare function par's - might have been overwritten by DatSet::FitStd
    if(tw->m_FitWin->FitType->currentItem()==5){
      double par;
      bool OK;
      int i;
      for(i=0;i<4;i++){
        par = tw->m_FitWin->SetPar[i]->text().toDouble(&OK);
        if(!OK) par = 0;
        fitfunc->SetParameter(i,par);
        if(tw->m_FitWin->FixPar[i]->isChecked()) 
          fitfunc->SetParLimits(i,par,par);
      }
    }
    if(tw->m_FitWin->FitType->currentItem()==4){
      double par;
      bool OK;
      int i;
      for(i=0;i<3;i++){
        par = tw->m_FitWin->SetPar[i]->text().toDouble(&OK);
        if(!OK) par = 0;
        fitfunc->SetParameter(i,par);
        if(tw->m_FitWin->FixPar[i]->isChecked()) 
          fitfunc->SetParLimits(i,par,par);
      }
    }
    error = item->GetDID()->FitStd(tw->rootmain->GetCal(),tw->m_FitWin->FitType->currentItem(),fitchip,
                                   (float)tw->m_FitWin->ChiCut->value(),
                                   (float)tw->m_FitWin->FitXmin->value(),
                                   (float)tw->m_FitWin->FitXmax->value(), selfit,tw->m_FitWin->divndfBox->isChecked()); 
    if(error<0){
      tmp.sprintf("Error while fitting: %s",DatSet::ErrorToMsg(error).c_str());
      if(tw->options->m_Opts->m_showwarn) QMessageBox::warning(tw,"Module Analysis",tmp);
      QApplication::restoreOverrideCursor();
      return 0;
    }
  }
  QApplication::restoreOverrideCursor();
  tw->textwin->setText("...done!");
  
  // remove existing stuff and recreate
  temp = tw->DataToTree(parnt,datit->GetDID());
  delete datit;
  return temp;
}

void setNeededData(bool *loadData,int cuttype){

  StdTestWin stdwin(0,"tmptestwin");

  int testType;
  switch(cuttype){
  case 1:
  case 2:
    testType = 6;
    break;
  case 3:
    testType = 7;
    break;
  case 4:
    testType = 9;
    break;
  default:
    testType=-1;
  }

  if(testType<0){
    for(int i=0;i<stdwin.NSLOTS;i++)
      loadData[i] = true;
    return;
  }

  stdwin.TestTypeSel->setCurrentItem(testType); // set to specified meas. type
  int count=0;
  for(std::vector<StdTestItem*>::iterator STD_IT=stdwin.m_items.begin();
      STD_IT!=stdwin.m_items.end(); STD_IT++){
    loadData[count] = (*STD_IT)->m_testtp[testType];
    count++;
  }

  for(int i=24;i<stdwin.NSLOTS;i++) // never want range data except for first VDD scan
    loadData[i] = false;

  if(cuttype==1)// LOAD (stave) cuts: do not need src data
    loadData[14] = false;
  if(cuttype==3){// STAVE: manually disable timewalk, ToT, monleak
    loadData[ 8] = false;
    loadData[ 9] = false;
    loadData[11] = false;
    loadData[12] = false;
    loadData[15] = false;
    loadData[16] = false;
    loadData[17] = false;
    loadData[18] = false;
    loadData[19] = false;
  }

  return;
}

const char* getSpecialCutLabel(int cutID)
{
  switch(cutID){
  case 1:
    return "LOAD(stave) cuts";
  case 2:
    return "LOAD(sector) cuts";
  case 3:
    return "STAVE cuts";
  case 4:
    return "DISK cuts";
  case 5:
    return "BIST cuts";
  default:
    return "Default cuts";
  }
}
void* defineSpecialCuts(CutPanel &cutpanel, int cutID){
  //  cutpanel.NewClass();
  CutClassItem *cutClass = new CutClassItem(cutpanel.ClassList,getSpecialCutLabel(cutID));
  cutpanel.ClassList->setCurrentItem(cutClass);
  cutpanel.selectedClass(cutClass);
  cutpanel.cutverLabel->setText(QString::number(((CutClassItem*)cutpanel.ClassList->currentItem())->GetVersion()));
  CutParam *cp = cutpanel.GetCutPars(cutClass->itemID());
  switch(cutID){
  case 1:{ // LOAD (stave) cuts
    cp[5].enabled = false;
    cp[6].enabled = false;
    cp[7].enabled = false;
    cp[8].enabled = false;
    cp[9].enabled = false;
    cp[10].enabled = false;
    cp[12].enabled = false;
    cp[14].enabled = false;
    cp[15].enabled = false;
    cp[16].enabled = false;
    cp[17].enabled = false;
    cp[18].enabled = false;
    for(int cID=START_RANGE;cID<(NCUTS+NMCUTS+NCCUTS);cID++)
      cp[cID].enabled = false;
    break;}
  case 2:{ // LOAD (sector) cuts
    cp[8].enabled = false;
    cp[9].enabled = false;
    cp[10].enabled = false;
    cp[12].enabled = false;
    cp[14].enabled = false;
    cp[15].enabled = false;
    cp[16].enabled = false;
    cp[17].enabled = false;
    cp[18].enabled = false;
    for(int cID=START_RANGE;cID<(NCUTS+NMCUTS+NCCUTS);cID++)
      cp[cID].enabled = false;
    break;}
  case 3:{ // STAVE cuts
    cp[8].enabled = false;
    cp[9].enabled = false;
    cp[10].enabled = false;
    cp[12].enabled = false;
    cp[14].enabled = false;
    cp[17].enabled = false;
    cp[18].enabled = false;
    for(int cID=START_RANGE;cID<(NCUTS+NMCUTS+NCCUTS);cID++)
      cp[cID].enabled = false;
    break;}
  case 4:{ // DISK cuts
    cp[5].enabled = false;
    cp[6].enabled = false;
    cp[7].enabled = false;
    cp[8].enabled = false;
    cp[9].enabled = false;
    cp[10].enabled = false;
    cp[12].enabled = false;
    cp[14].enabled = false;
    cp[15].enabled = false;
    cp[16].enabled = false;
    cp[17].enabled = false;
    cp[18].enabled = false;
    cp[NCUTS+2].enabled = false;
    for(int cID=START_RANGE;cID<(NCUTS+NMCUTS+NCCUTS);cID++)
      cp[cID].enabled = false;
    break;}
  case 5:{ // BIST cuts
    cp[5].enabled = false;
    cp[6].enabled = false;
    cp[7].enabled = false;
    cp[8].enabled = false;
    cp[9].enabled = false;
    cp[10].enabled = false;
    cp[12].enabled = false;
    cp[14].enabled = false;
    cp[15].enabled = false;
    cp[16].enabled = false;
    cp[17].enabled = false;
    cp[18].enabled = false;
    // enable voltage check cuts
    cp[NCUTS+17].enabled = true;
    cp[NCUTS+18].enabled = true;
    for(int cID=START_RANGE;cID<(NCUTS+NMCUTS+NCCUTS);cID++)
      cp[cID].enabled = false;
    break;}
  default: // full cuts
    break;
  }
  cutpanel.Reset();
  return (void*) cutClass;
}

void getRootFromRDB(QString modSN, QString measType, QString path, bool retainName, QApplication *app){
  int ind=0, ii=0, pos, pos2;
  RDBManager rdbm(app);
  if(app!=0 && app->type()==QApplication::GuiClient) QApplication::setOverrideCursor(Qt::waitCursor);
  std::vector<std::string> listOfUrls = rdbm.getRootUrls(modSN, measType);
  if(app!=0 && app->type()==QApplication::GuiClient) QApplication::restoreOverrideCursor();
  if(listOfUrls.size()>1){
    if(app!=0 && app->type()==QApplication::GuiClient){
      RootDataList dl(0,"rootdl",TRUE);
      dl.moduleLabel->setText("Module " + modSN);
      dl.Comments->setText("Found more than one file for module " + modSN + 
			   "\nPlease sleect one.");
      dl.TextLabel1_2->hide();
      dl.AllButton->hide();
      dl.DeAllButton->hide();
      dl.FileList->setSelectionMode(Q3ListView::Single);
      for(std::vector<std::string>::iterator it = listOfUrls.begin(); it!=listOfUrls.end();it++){
	dl.LoadName((*it).c_str(),ii);
	ii++;
      }
      if(dl.exec()==QDialog::Accepted){
	Q3ListViewItem *rootIT = dl.FileList->firstChild();
	while(rootIT!=0){
	  if(rootIT->isSelected())
	    ind = dl.loadedItems[(long int)rootIT];
	  rootIT = rootIT->nextSibling();
	}
      }// else: use default of taking first in list
    } else
      return;
  }else if(listOfUrls.size()==0){
    if(app!=0 && app->type()==QApplication::GuiClient)
      QMessageBox::warning(0,"RDB download error","No file found for module " + modSN);
    return;
  }
  QString fname, rname;
  if(retainName){
    rname = listOfUrls[ind].c_str();
    pos = rname.find("/");
    pos2 = pos;
    while(pos>=0){
      pos2 = pos;
      pos = rname.find("/",pos+1);
    }
    rname.remove(0,pos2+1);
    fname = path + "/" + rname;
  } else{
    fname = path;
    QString indStrg = listOfUrls[ind].c_str();
    indStrg = indStrg.right(7);
    indStrg = indStrg.left(2);
    fname.replace("%m",modSN.right(6));
    fname.replace("%s",modSN);
    fname.replace("%t",measType);
    fname.replace("%i",indStrg);
  }
  rname = fname;
  pos = rname.find("/");
  pos2 = pos;
  while(pos>=0){
    pos2 = pos;
    pos = rname.find("/",pos+1);
  }
  rname.remove(pos2,rname.length()-pos2);
  if(app!=0 && app->type()==QApplication::GuiClient) QApplication::setOverrideCursor(Qt::waitCursor);
  // check full path and create missing directories
  checkPath(rname.latin1());
  // download
  rdbm.getRootFromUrl(listOfUrls[ind].c_str(), fname);
  if(app!=0 && app->type()==QApplication::GuiClient) QApplication::restoreOverrideCursor();
}
void getStaveRootFromRDB(QString staveSN, QString userPDB, QString passwdPDB, bool retainName, 
				   QString measType, QString fixedName, QString varName, QApplication *app, 
				   TopWin *tw){
  if(staveSN=="" || userPDB=="" || passwdPDB=="" || fixedName=="" ) return;
  CDatabaseCom cdb(userPDB.latin1(),passwdPDB.latin1(),staveSN.latin1());
  std::vector<std::string> modSNs;
  modSNs = cdb.getStaveSNs(*app);
  for(std::vector<std::string>::iterator it=modSNs.begin(); it!=modSNs.end();it++){
    QString tmpstr = (*it).c_str();
    QString fname = fixedName;
    if(!retainName){
      fname = varName;
      fname = fixedName + fname;
    }
    if(tw!=0){
      tw->textwin->setText("Downloading file for module "+tmpstr);
      tw->textwin->repaint();
    }
    tmpstr = "20210020" + tmpstr;
    if(app!=0)
      app->processEvents();      
    getRootFromRDB(tmpstr, measType,fname, retainName, app);
  }
  return;
}

int checkPath(const char *in_path){
  int error = 0;
  int pos=0, offs=-1, cf_err=0;
  QString path = in_path;
  while(pos>=0 && cf_err<=0){
    QString subpath=path;
    pos = path.findRev("/",offs);
    if(pos>=0)
      subpath.remove(pos+1,subpath.length()-pos);
    cf_err = DatSet::CheckFolder(subpath.latin1());
    if(cf_err<=0) // else: rest of path is OK, can stop here
      offs=pos-path.length()-1;
    else
      offs = subpath.length();//path.length()-offs;
  }
  while(pos>=0){
    QString subpath=path;
    pos = path.find("/",offs);
    if(pos>=0)
      subpath.remove(pos+1,subpath.length()-pos);
    int cf_err = DatSet::CheckFolder(subpath.latin1());
    if(cf_err<0)
      error = -1;
    offs=pos+1;
  }
  return error;
}

DatSet* LoadStdFromRoot(int type, const char *root_in){

  int i, ndat, dtype, error;
  char fname[2000], orgpath[2000];
  strcpy(orgpath,gDirectory->GetPath());
  TDirectory *root_base = gDirectory;
  TObject *tmpobj;
  TFile *input = new TFile(root_in);
  if(input==NULL)
    return 0;

  tmpobj = input->Get("ndat");
  if(tmpobj==0){
    root_base->cd();
    return 0;
  }
  sscanf(tmpobj->GetName(),"%d",&ndat);  
  //tmpobj = input->Get("module name");
  //sscanf(tmpobj->GetName(),"%s",mname);
  for(i=0;i<ndat;i++){
    sprintf(fname,"data-%d",i);
    try{
      input->cd(fname);
      tmpobj = gDirectory->Get("stdtest");
    }catch(...){
      tmpobj = 0;
    }
    if(tmpobj==0){
      root_base->cd();
      return 0;
    }
    sscanf(tmpobj->GetName(),"%d",&dtype);
    if((dtype%100)==type) break;
  }
  if(i==ndat && (dtype%100)!=type){
    input->Close();    
    root_base->cd();
    return 0;
  }

  // load data and close file
  //gDirectory->pwd();
  DatSet *data = new DatSet(orgpath, &error);
  input->Close();

  return data;
}
int WriteConfig(int type, const char *root_in, const char *cfg_out, const char *cfg_name, 
                bool doAscii, int phase1, int phase2){

  std::string cfgpath = cfg_out;

  DatSet *data = LoadStdFromRoot(type,root_in);
  if(data==0) return -2;

  if(data->GetLog()==0){
    delete data;
    return -3;
  }

  if(doAscii){
    // create cfg-path if requested and make sure that dir's exist
    if(cfg_name!=0){
      if(cfgpath.substr(cfgpath.length(), 1)!="/")
        cfgpath += "/";
      cfgpath += "M";
      cfgpath += data->GetLog()->MyModule->modname.c_str();
      cfgpath += "/";
      if(DatSet::CheckFolder(cfgpath.c_str())<0){
        delete data;
        return -4;
      }
      cfgpath += "configs/";
      if(DatSet::CheckFolder(cfgpath.c_str())<0){
        delete data;
        return -4;
      }
      cfgpath += cfg_name;
    }
    // write config stuff
    int retVal = 100*data->WriteConfig(cfgpath.c_str(), 0, (phase1+12), (phase2+12));
    delete data;
    return retVal;
  }else{
#ifdef HAVE_PLDB
    // create RootDB file
    data->GetLog()->MyModule->createRootDB(cfgpath.c_str());
    delete data;
    return 0;
#else
    delete data;
    return -5;
#endif
  }
}

void checkToT(DatSet *data, std::vector<float> &result){

  int i, chip, error, type;
  float tmp_res, chip_avg, chip_rms, chi2_per_ndf, max_chip_rms=0;

  TH1F *projection[5];

  RMain *rmain;
  optClass *tmp_opt=0;
  tmp_opt = new optClass(false);
  tmp_opt->m_UsrIUse[TOT_MAX]=false;
  tmp_opt->m_UsrIUse[AVTOT]=false;
  tmp_opt->m_doUsrRg=false;
  rmain = new RMain(tmp_opt);

  type = TOT_MAX;
  for(i=0;i<5;i++)
    projection[i]=0;
  error = rmain->GetProjection(type, data, data->GetMap(-1,type),projection,0);
  if(error){
    printf("Error %d in calling RMain::GetProjection for type %d\n", error, type);
    return;
  }
  // array element 0 covers all pixels
  projection[0]->Fit("gaus","0Q");
  tmp_res = (float) projection[0]->GetFunction("gaus")->GetParameter(1);
  result.push_back(tmp_res);
  tmp_res = (float) projection[0]->GetFunction("gaus")->GetParameter(2);
  result.push_back(tmp_res);
  tmp_res = (float) projection[0]->GetRMS();
  result.push_back(tmp_res);
  for(i=0;i<5;i++)
    if(projection[i]!=0) projection[i]->Delete();

  type = AVTOT;
  for(i=0;i<5;i++)
    projection[i]=0;
  error = rmain->GetProjection(type, data, data->GetMap(-1,type),projection,0);
  if(error){
    printf("Error %d in calling RMain::GetProjection for type %d\n", error, type);
    return;
  }
  // array element 0 covers all pixels
  projection[0]->Fit("gaus","0Q");
  tmp_res = (float) projection[0]->GetFunction("gaus")->GetParameter(1);
  result.push_back(tmp_res);
  tmp_res = (float) projection[0]->GetFunction("gaus")->GetParameter(2);
  result.push_back(tmp_res);
  chi2_per_ndf = (float) projection[0]->GetFunction("gaus")->GetChisquare();
  //printf("chi2: %f %f\n",chi2_per_ndf, (float)projection[0]->GetFunction("gaus")->GetNDF());
  if(projection[0]->GetFunction("gaus")->GetNDF()>0)
    chi2_per_ndf /= (float)(projection[0]->GetFunction("gaus")->GetNDF());
  
  tmp_res = (float) projection[0]->GetRMS();
  result.push_back(tmp_res);
  for(i=0;i<5;i++)
    if(projection[i]!=0) projection[i]->Delete();

  chip_avg=0;
  chip_rms=0;
  for(chip=0;chip<NCHIP;chip++){
    for(i=0;i<5;i++)
      projection[i]=0;
    error = rmain->GetProjection(type, data, data->GetMap(chip,type),projection,0);
    if(error){
      printf("Error %d in calling RMain::GetProjection for type %d, chip %d\n", error, type, chip);
      return;
    }
    chip_avg += (float) projection[0]->GetMean();
    chip_rms += (float) pow(projection[0]->GetMean(),2);
    if(((float) projection[0]->GetRMS())>max_chip_rms)
      max_chip_rms = (float) projection[0]->GetRMS();
    for(i=0;i<5;i++)
      if(projection[i]!=0) projection[i]->Delete();

  }
  tmp_res = sqrt((chip_rms-chip_avg*chip_avg/16)/15);
  result.push_back(tmp_res);
  result.push_back(chi2_per_ndf);
  result.push_back(max_chip_rms);

  return;
}
std::string getToTCheckLabel(int i){
  const int iexp=9;
  std::string labels[iexp]={"mean of max ToT ", "sigma of max ToT", "RMS of max ToT  ",
			    "mean of avg ToT ", "sigma of avg ToT", "RMS of avg ToT  ", "RMS of chip-avg ToT ",
                            "chi2/ndf of avg. ToT", "max. RMS of avg. ToT(chip)"};
  std::string nu = "value not used";
  if(i>=0 && i<iexp)
    return labels[i];
  else
    return nu;
}

ModItem* loadDataAny(const char *file, TopWin *tw){
  if(tw==0) return 0;
  QString file_name = file;
  int pos;
  file_name.replace(QRegExp("\\"),"/");
  if(file_name.right(5)==".root"){
    return tw->LoadRoot_Menu(false,file_name.latin1());
  } else{
    ModItem *mi = new ModItem(tw->ModuleList,"new module");
    DatSet *data = loadDataAscii(file_name.latin1());
    if(data!=0){
      tw->DataToTree(mi,data);
      if(data->GetLog()!=0) // get module name from logfile
	mi->setText(0,data->GetLog()->MyModule->modname.c_str());
      else{ // guess from file name
	pos = file_name.findRev("/");
	if(pos>0){
	  QString mname = file_name.right(file_name.length()-pos-1);
	  pos = mname.find("_");
	  if(pos>0)
	    mi->setText(0,mname.left(pos));
	}
      }
      mi->setOpen(TRUE);
      return mi;
    } else{
      delete mi;
      return 0;
    }
  }
}
DatSet* loadDataAscii(const char *file_name){
  //tw->m_fwin->SetType(tw->m_fwin->GuessType(file_name));
  int i, type = GuessType(file_name);//tw->m_fwin->m_seltype;
  if(type>DSET){
    DatSet *data = new DatSet("new data",file_name,type,&i);
    return data;
  }else
    return 0;
}
void fileExtensions(std::map<int,QString> &labels, std::map<int,QString> &extensions){
  labels.clear();
  extensions.clear();

  labels.insert(std::make_pair(ONEDSCAN,QString("1D scan (VCAL, strobe delay,...) - sfit data")));
  labels.insert(std::make_pair(TWODSCAN,QString("2D scan (TDAC tuning, timewalk,...) - sfit data")));
  labels.insert(std::make_pair(TOTR,QString("ToT raw data (source, hot pixel, calib.,...)")));
  labels.insert(std::make_pair(TOTF,QString("ToT calibration fit data")));
  labels.insert(std::make_pair(LEAK,QString("MonLeak data")));
  labels.insert(std::make_pair(IVSCAN,QString("IV scan data (TurboDAQ)")));
  labels.insert(std::make_pair(SENSORIV,QString("IV scan data (sensor wafer test from PDB)")));
  labels.insert(std::make_pair(DIG,QString("Binned hit data")));
  labels.insert(std::make_pair(TFDAC,QString("T/FDAC tuning results")));
  labels.insert(std::make_pair(MASKDAT,QString("Mask map")));
  labels.insert(std::make_pair(CHIPCURR,QString("Chip current measurement")));
  labels.insert(std::make_pair(BI_VIT_DATA,QString("Burn-in Summary"))); //added 27/4/04 by LT
  labels.insert(std::make_pair(REGTEST,QString("Register test output")));
  labels.insert(std::make_pair(ROOTF,QString("ROOT file with data sets")));
  labels.insert(std::make_pair(MULTIF,QString("Multiple files")));

  extensions.insert(std::make_pair(ONEDSCAN,QString("Sfits output (*sfits.out)")));
  extensions.insert(std::make_pair(TWODSCAN,QString("Sfits output (*sfits.out)")));
  extensions.insert(std::make_pair(TOTR,QString("TOT raw data (*.tot)")));
  extensions.insert(std::make_pair(TOTF,QString("ToT fit output (*cal.out)")));
  extensions.insert(std::make_pair(LEAK,QString("MonLeak data (*monleak.out)")));
  extensions.insert(std::make_pair(IVSCAN,QString("IV data (*.iv)")));
  extensions.insert(std::make_pair(SENSORIV,QString("IV data (*.ivu)")));
  extensions.insert(std::make_pair(DIG,QString("Binned data (*.bin)")));
  extensions.insert(std::make_pair(TFDAC,QString("T/FDAC data (*dacs*.out)")));
  extensions.insert(std::make_pair(MASKDAT,QString("Mask file (*.dat)")));
  extensions.insert(std::make_pair(CHIPCURR,QString("Pictmon file (*pictmon.out)")));
  extensions.insert(std::make_pair(BI_VIT_DATA,QString("Ambush V,I,T data files (*.log)"))); //added 27/4/04 by LT
  extensions.insert(std::make_pair(REGTEST,QString("Regtest txt-file (*.txt)")));
  extensions.insert(std::make_pair(ROOTF,QString("ROOT file (*.root)")));
  extensions.insert(std::make_pair(MULTIF,QString("Any file (*.*)")));

}

int GuessType(QString fname){
  QString ext;
  int pos;
  std::map<int,QString> labels, extensions;
  fileExtensions(labels, extensions);
  std::map<int,QString>::iterator it;
  for(it=extensions.begin(); it!=extensions.end(); it++){
    ext = it->second;
    pos = ext.find("(*dacs*.out)"); //T/FDAC files: special case, since *.out is not unique...
    if(pos>=0){
      pos = fname.find(".out");
      if(pos>=0){
	pos = fname.find("_tdacs_");
	if(pos<0){
	  pos = fname.find("_fdacs_");
	  if(pos>=0) return it->first;
	}else
	  return it->first;
      }
    } else{
      pos = ext.find("(*");
      if(pos>=0)
	ext.remove(0,pos+2);
      pos = ext.find(")");
      if(pos>=0)
	ext.remove(pos,ext.length());
      pos = fname.find(ext);
      if(pos>=0){
	if(it->first!=ONEDSCAN && it->first!=TWODSCAN)
	  return it->first;
	else{ // find out if it's 1D or 2D scan
	  int chip, col, row;
	  int scpt1, scpt2;
	  char c;
	  std::ifstream input(fname.latin1());
	  if(!input.is_open()) return NONE;
	  input >> chip >> col >> row >> scpt1;
	  // go to end and step back to last line
	  for(int i=3;i<500;i++){
	    input.seekg(-i,std::ios::end);
	    input.get(c);
	    if(c=='\n') break;
	  }
	  input >> chip >> col >> row >> scpt2;
	  input.close();
	  if(scpt1==scpt2) return ONEDSCAN;
	  else             return TWODSCAN;
	}
      }
    }
  }
  return NONE;
}

#ifdef PIXEL_ROD // can link to PixScan
DatSet* readFromPixScan(PixLib::PixScan *ps, PixLib::DBInquire *moduleIn, const char *label, int *error, int in_modID, const char *path){

  int i,j;
  int modID=in_modID;
  
  *error = 0;
  
  // some simple sanity checks
  if(moduleIn==0 && in_modID<0){
    *error = -1;
    return 0;
  }
  
  Config &scanCfg = ps->config();
  //scanCfg.dump(std::cout);
  
  // get module ID - needed to identify histograms for this module
  if(modID<0){
    RootDB *myrdb = dynamic_cast<RootDB*>(moduleIn->getDB());
    if(myrdb==0){
      *error = -1;
      return 0;
    }
    if(ps==0){
      *error = -1;
      return 0;
    }
    PixLib::fieldIterator fit = moduleIn->findField("general_ModuleId");
    if((*fit)!=0 && fit!=moduleIn->fieldEnd())
      myrdb->DBProcess(fit,PixLib::READ, modID);
  }
  // create logfile from module info and PixScan config
  // to do: create this also from module config, not just from DBInquire
  TLogFile *mylog = new TLogFile(moduleIn,scanCfg);

  // guess data type
  bool isscan = false;
  int dtype = NONE;
  DatSet *datit = 0;
  std::vector<PixLib::Histo*> hiVec;
  // create data
  // check if we have occupancy histograms
  //    if(modHisto.size()>0){
  isscan = (ps->getLoopVarNSteps(0)>1);
  if(ps->getHistogramFilled(PixLib::PixScan::OCCUPANCY) && ps->getHistogramKept(PixLib::PixScan::OCCUPANCY) &&
     !(ps->getHistogramFilled(PixLib::PixScan::LVL1) && ps->getHistogramKept(PixLib::PixScan::LVL1))){
    if (ps->getHistogramFilled(PixLib::PixScan::TOT_MEAN) && ps->getHistogramKept(PixLib::PixScan::TOT_MEAN)){
      dtype = TOTR; // ToT + occupancy 
    }else if (ps->getLoopAction(0)==PixLib::PixScan::SCURVE_FIT){
      dtype = ONEDFIT; // occupancy plus fit data
    }else{
      dtype = DIG; // means occupancy histos are stored
    }
    // create and fill data item with histograms in PixScan
    if(ps->getLoopVarNSteps(0)==0){
      Histo *hi = &(ps->getHisto(PixLib::PixScan::OCCUPANCY, modID, 0,0,0));
      //std::cout << "histo mod " << modID << " (single step) is " << hi->title() << std::endl;
      hiVec.push_back(hi);
    } else{
      for(i=0;i<ps->getLoopVarNSteps(0);i++){
        Histo *hi = &(ps->getHisto(PixLib::PixScan::OCCUPANCY, modID, 0,0,i));
        //std::cout << "histo mod " << modID << " index " << i << " is " << hi->title() << std::endl;
        hiVec.push_back(hi);
      }
    }
    datit = new DatSet(label, dtype, isscan, mylog, hiVec, error, path);
    //std::cout << (int)datit << " " << *error << " " << dtype << std::endl;
    if(datit!=0 && (*error)==0 && dtype==ONEDFIT){
      hiVec.clear();
      hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_MEAN, modID, 0,0,0)));
      //      std::cout << "thresh-histo mod " << modID << " is " << hiVec[hiVec.size()-1]->title() << std::endl;
      hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_SIGMA, modID, 0,0,0)));
      hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_CHI2, modID, 0,0,0)));
      *error = datit->LoadThrHisto(hiVec);
    }
    if(datit!=0 && (*error)==0 && dtype==TOTR){
      hiVec.clear();
      std::vector<PixLib::Histo*> sigVec;
      for(i=0;i<ps->getLoopVarNSteps(0);i++){
        Histo *hi = &(ps->getHisto(PixLib::PixScan::TOT_MEAN, modID, 0,0,i));
        hiVec.push_back(hi);
        hi = &(ps->getHisto(PixLib::PixScan::TOT_SIGMA, modID, 0,0,i));
        sigVec.push_back(hi);
      }
      *error = datit->LoadToTHisto(hiVec, sigVec);
    }
  }else if (ps->getHistogramFilled(PixLib::PixScan::LVL1) && ps->getHistogramKept(PixLib::PixScan::LVL1)){
    if (ps->getLvl1HistoBinned() && isscan){
      dtype = LVL1SCAN; // sliced LVL1 
      for(i=0;i<ps->getLoopVarNSteps(0);i++){
	for(j=0;j<16;j++){
	  Histo *hi = &(ps->getHisto(PixLib::PixScan::LVL1, modID, 0,0,i,j));
	  hiVec.push_back(hi);
	}
      }
    } else { // single/integrated LVL1 
      dtype = LVL1SHOT;
      for(j=0;j<16;j++){
	Histo *hi = &(ps->getHisto(PixLib::PixScan::LVL1, modID, 0,0,0,j));
	hiVec.push_back(hi);
      }
    }
    datit = new DatSet(label, dtype, isscan, mylog, hiVec, error, path);
    //std::cout << (int)datit << " " << *error << " " << dtype << std::endl;
  }else if (ps->getHistogramFilled(PixLib::PixScan::TOT_MEAN) && 
	    ps->getHistogramKept(PixLib::PixScan::TOT_MEAN)){
    dtype = TOTR; // ToT + occupancy 
    if(ps->getHistogramFilled(PixLib::PixScan::TOTCAL_PARA) && 
	    ps->getHistogramKept(PixLib::PixScan::TOTCAL_PARA))
      dtype = TOTFR; // have also fit info
    datit = new DatSet(label, dtype, isscan, mylog, error, path);
    if(datit!=0 && (*error)==0){
      hiVec.clear();
      std::vector<PixLib::Histo*> sigVec;
      for(i=0;i<ps->getLoopVarNSteps(0);i++){
        Histo *hi = &(ps->getHisto(PixLib::PixScan::TOT_MEAN, modID, 0,0,i));
        hiVec.push_back(hi);
        hi = &(ps->getHisto(PixLib::PixScan::TOT_SIGMA, modID, 0,0,i));
        sigVec.push_back(hi);
      }
      *error = datit->LoadToTHisto(hiVec, sigVec);
      if(dtype==TOTFR && (*error)==0){
	hiVec.clear();
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::TOTCAL_PARA, modID, 0,0,0)));
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::TOTCAL_PARB, modID, 0,0,0)));
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::TOTCAL_PARC, modID, 0,0,0)));
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::TOTCAL_CHI2, modID, 0,0,0)));
	*error = datit->LoadToTFit(hiVec);
      }
    }
  } else if(ps->getLoopAction(0)==PixLib::PixScan::SCURVE_FIT){
    // we've just got S-curve results
    if(ps->getLoopVarNSteps(1)>1){
      dtype = TWODSCAN;
      for(i=0;i<ps->getLoopVarNSteps(1);i++){
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_MEAN, modID, 0,i,0)));
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_SIGMA, modID, 0,i,0)));
        hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_CHI2, modID, 0,i,0)));
      }
      datit = new DatSet(label, dtype, isscan, mylog, hiVec, error, path);
    } else{
      hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_MEAN, modID, 0,0,0)));
      hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_SIGMA, modID, 0,0,0)));
      hiVec.push_back(&(ps->getHisto(PixLib::PixScan::SCURVE_CHI2, modID, 0,0,0)));
      dtype = ONEDSCAN;
      datit = new DatSet(label, dtype, false, mylog, hiVec, error, path);
    }
    //std::cout << (int)datit << " " << *error << " " << dtype << std::endl;
  }

  return datit;
  
}
DatSet* findPixScan(const char *fileName, const char *scanName, const char *grpName, const char *moduleName){

  TDirectory *memdir = gDirectory; // remember current dir. - needed for MA storage
  PixLib::RootDB *myDB = new PixLib::RootDB(fileName);
  PixLib::DBInquire *root, *scanInq, *dataInq, *modInq, *grpInq;
  std::string dataPath = fileName;
  dataPath += ":/";
  dataPath += scanName;
  dataPath += "/";
  dataPath += grpName;

  try{
    root = myDB->readRootRecord(1);
  } catch(...){
    delete myDB;
    return 0;
  }

  std::string name = scanName;
  try{
    scanInq = *(root->findRecord(name+"/PixScanResult"));
  }catch(...){
    delete myDB;
    return 0;
  }

  name = grpName;
  try{
    grpInq = *(scanInq->findRecord(name+"/PixModuleGroup"));
  }catch(...){
    delete myDB;
    return 0;
  }

  PixScan *ps = new PixScan();
  try{
    dataInq = *(grpInq->findRecord("Data_Scancfg/PixScanData"));
    ps->readConfig(dataInq);
    ps->readHisto(dataInq);
  }catch(...){
    delete myDB;
    return 0;
  }

  name = moduleName;
  try{
    modInq = *(grpInq->findRecord(name+"/PixModule"));
  }catch(...){
    delete myDB;
    return 0;
  }

  int error;
  memdir->cd(); // make sure MA histos are created in memory
  DatSet *retData = readFromPixScan(ps, modInq, scanName, &error, -1, dataPath.c_str());
  if(error){
    delete retData;
    retData = 0;
  }
  
  wrapPs *cfgPs = new wrapPs();
  cfgPs->config()=ps->config();
  retData->setPixScan(cfgPs);

  delete myDB;
  return retData;
}
#endif

} // end namespace MAEngine

