#include "MAFitting.h"
#include "RootDataList.h"
#include "PSFitWin.h"
#include "ParFitfun.h"
#include "LogClasses.h"
#include "OptClass.h"
#include "ModTree.h"

#include <q3listview.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <q3filedialog.h>
#include <qstring.h>
#include <QDoubleSpinBox>
#include <QMessageBox>

#include <RootDB.h>
#include <PixDBData.h>
#include <FitClass.h>
#include <PixDcs/SleepWrapped.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <Config/ConfMask.h>

#include <iostream>
#include <sstream>
#include <exception>

MAFitting::MAFitting(QApplication *app, optClass *opts)
  : m_app(app), m_opts(opts)
{
}
bool MAFitting::rootdbFit(ModItem *item, int loop, int chip)
{
  bool reload = false, saveData=false;
  QString tmptxt;

  PSFitWin *fw = new PSFitWin(0,"fitwin", true, 0, item);

  if(fw->exec()==QDialog::Accepted){
    int npar = fw->m_fitClass->getNPar(fw->FitType->currentItem());
    float *pars;
    bool *pfix;
    int *phis;
    pars = new float[npar];
    pfix = new bool[npar];
    phis = new int[npar];
    for(int i=0;i<npar; i++){
      if(fw->guessParBox->isChecked()){
	pars[i] = 0.;
	pfix[i] = fw->m_parFixed[i]->isChecked();
      } else{
	pars[i] = (float)fw->m_parValues[i]->value();
	pfix[i] = fw->m_parFixed[i]->isChecked();
      }
    }
    PixDBData *pdbd = dynamic_cast<PixDBData*>(item->GetDataItem());
    if(pdbd!=0){
      PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType)(item->GetPID()-PIXDBTYPES);
      PixLib::PixScan::HistogramType errType = PixLib::PixScan::MAX_HISTO_TYPES;
      if(fw->fromHistoBox->isChecked()){
	PixLib::PixScan *ps = new PixLib::PixScan();
	errType = (PixLib::PixScan::HistogramType)(ps->getHistoTypes()[fw->errorCBox->currentText().latin1()]);
	delete ps;
      }

      // open dialog for save fit results into RootDB later on
      ParFitfun *pff = new ParFitfun(0, "pff", true, 0, pdbd);
      pff->setCaption("Saving parameters to RootDB file");
      pff->funcBox->setCurrentItem(fw->FitType->currentItem());
      pff->funcBox->setEnabled(false);
      pff->genHistoBoxes(fw->FitType->currentItem(), false, true);
      
      
      std::vector<int> parhis;
      PixScan::HistogramType oldChiHi=PixScan::MAX_HISTO_TYPES;
      if(pff->exec()==QDialog::Accepted){
	saveData = true;
	pff->getPars(parhis, true);
	for(unsigned int k=0;k<parhis.size();k++){
	  if(k<(unsigned int)npar) phis[k] = parhis[k];
	}
	if(fw->refitBox->isChecked())
	  oldChiHi = (PixScan::HistogramType)parhis[parhis.size()-1]; // last argument is old chi2 histo
      }

      tmptxt = "Initialising fit for data "+item->DatParent()->text(0)+
	", module "+item->ModParent()->text(0);
      emit sendText(tmptxt);
      m_app->processEvents();
      pdbd->initFit(type, loop, pars, pfix, phis, fw->FitType->currentItem(),errType, oldChiHi, fw->vcalBox->isChecked());
      
      // determine chip flavour to guess max. no of FEs
      bool isFei4 = false;
      PixLib::Config &mcfg = pdbd->getModConfig();
      if(mcfg["general"].name()!="__TrashConfGroup__" && mcfg["general"]["FE_Flavour"].name()!="__TrashConfObj__"){
	std::string fstrg =  ((ConfList&)mcfg["general"]["FE_Flavour"]).sValue();
	if(fstrg=="FE_I4" || fstrg=="FE_I4A" || fstrg=="FE_I4B") isFei4 = true;
      }

      QApplication::setOverrideCursor(Qt::waitCursor);
      int minc = 0; 
      int maxc = isFei4?2:NCHIP;
      if(chip>=0){
	minc = chip;
	maxc = minc+1;
      }
      bool noAbort=true;
      for(int chip=minc;chip<maxc && noAbort;chip++){
	tmptxt = "Fitting chip "+QString::number(chip)+" from data "+item->DatParent()->text(0)+
	  ", module "+item->ModParent()->text(0);
	if(oldChiHi<PixScan::MAX_HISTO_TYPES)
	  tmptxt += " (bad fits only)";
	emit sendText(tmptxt);
	m_app->processEvents();
	float frErr = 0;
	if(fw->fractBox->isChecked())
	  frErr = fw->fractEdit->text().toFloat();
	FitThread fitthr(*pdbd, fw->FitType->currentItem(), chip,(float)fw->ChiCut->value(),(float)fw->FitXmin->value(),
		  (float)fw->FitXmax->value(),frErr,fw->divndfBox->isChecked());
	fitthr.start();
	QApplication::restoreOverrideCursor();
	QMessageBox abortwin(QMessageBox::NoIcon, "Abort fit", "fit in progress\nStop: stop and save\nAbort: stop w/o saving",
			     QMessageBox::Abort, 0, Qt::Dialog);
	QPushButton *stopButton = abortwin.addButton("Stop", QMessageBox::AcceptRole);
	abortwin.show();
	QMessageBox waitwin(QMessageBox::Information, "wait", "wait for fit abort",
			     QMessageBox::NoButton, 0, Qt::Dialog);
	while(fitthr.running()){
	  if(!abortwin.isVisible() && noAbort){
	    pdbd->abortFit(); // controlled abort, loop must wait till finished
	    noAbort = false;
	    waitwin.show();
	    saveData = (abortwin.clickedButton()==stopButton);
	  }
	  int col, row;
	  pdbd->getPix(col, row);
	  tmptxt = "Fitting chip "+QString::number(chip)+", col "+QString::number(col)+", row "+QString::number(row)+
	    " from data "+item->DatParent()->text(0)+ ", module "+item->ModParent()->text(0);
	  if(oldChiHi<PixScan::MAX_HISTO_TYPES)
	    tmptxt += " (bad fits only)";
	  emit sendText(tmptxt);
	  for(int kk=0;kk<10;kk++) m_app->processEvents();
	  PixLib::sleep(200);
	}
	QApplication::setOverrideCursor(Qt::waitCursor);
// 	pdbd->fitHisto(fw->FitType->currentItem(), chip,(float)fw->ChiCut->value(),(float)fw->FitXmin->value(),
// 		       (float)fw->FitXmax->value(),frErr,fw->divndfBox->isChecked());
      }
      tmptxt = noAbort?"Finished fit!":"Fit was aborted";
      emit sendText(tmptxt);
      m_app->processEvents();
      
      if(saveData){
	tmptxt = "Storing results";
	emit sendText(tmptxt);
	m_app->processEvents();
	for(int k=0;k<(int)parhis.size();k++){
	  PixLib::PixScan::HistogramType htype = (PixLib::PixScan::HistogramType)parhis[k];
	  pdbd->writePixLibHisto(pdbd->getParHisto(k), htype, 0, loop+1);
	}
	// store fit function name for later reference
	pdbd->writeFitType(fw->FitType->currentItem(), loop+1);
	// clear all, including histos
	pdbd->clearFit(true);
	reload = true;
	tmptxt = "Done!";
	emit sendText(tmptxt);
	m_app->processEvents();
      }
      delete pff;
      QApplication::restoreOverrideCursor();
    }
    delete[] pars;
    delete[] pfix;
    delete[] phis;
  }
  delete fw;

  return reload;
}
void MAFitting::rootdbFitAll(int chip_in)
{
  QString tmptxt;

  int ideb=0;
  printf("DEBUG %d\n",ideb); ideb++;

  QString fname = m_opts->m_DefDataPath.c_str();
  QStringList filter("ROOT file (*.root)");
  filter += "Any file (*.*)";
  Q3FileDialog fdia(fname,QString::null,0,"select data file",TRUE);
  fdia.setFilters(filter);
  printf("DEBUG %d\n",ideb); ideb++;
  if(fdia.exec() == QDialog::Accepted){
    fname = fdia.selectedFile();

    // open RootDB file
    RootDB *file = new RootDB(fname.latin1());
    DBInquire *root = file->readRootRecord(1);
    // create root and QT application
    RootDataList *dl = new RootDataList(0,"rootdl",TRUE);
    recordIterator iteri, iterii;  
    printf("DEBUG %d\n",ideb); ideb++;
    for(iteri = root->recordBegin(); iteri!=root->recordEnd(); iteri++){
      if((*iteri)->getName()=="PixScanResult"  ){
	std::string name, time_stamp="";
	fieldIterator tmpit = (*iteri)->findField("TimeStamp");
	std::string timestamp="";
	if((*tmpit)!=0 && tmpit!=(*iteri)->fieldEnd())
	  file->DBProcess(tmpit,READ, time_stamp);
	int pos = (int)time_stamp.find("Scan commenced at ");
	if(pos!=(int)std::string::npos)
	  time_stamp.erase(pos,18);
	name = TLogFile::GetNameFromPath((*iteri)->getDecName().c_str());
	dl->LoadName(name.c_str(), (long int)*iteri, time_stamp.c_str());
      }
    }
    dl->FileList->setSelectionMode( Q3ListView::Single );
    printf("DEBUG %d\n",ideb); ideb++;

    std::vector<PixDBData *> data;
    std::vector<std::string> paths, modules;
    if(dl->exec()){
      printf("DEBUG %d\n",ideb); ideb++;
      DBInquire *DBscan = 0;
      Q3ListViewItem *item = dl->FileList->firstChild();
      while(item!=0){
	if(item->isSelected()){
	  DBscan = (DBInquire*) dl->loadedItems[(long int)item]; // scan object
	  break;
	}
	item = item->nextSibling();
      }
      
      printf("DEBUG %d\n",ideb); ideb++;
      if(DBscan!=0){
	for(iteri = DBscan->recordBegin(); iteri!=DBscan->recordEnd(); iteri++){
	  if((*iteri)->getName()=="PixModuleGroup"  ){
	    for(iterii = (*iteri)->recordBegin(); iterii!=(*iteri)->recordEnd(); iterii++){
	      if((*iterii)->getName()=="PixModule"  ){
		modules.push_back(TLogFile::GetNameFromPath((*iterii)->getDecName().c_str()));
		std::string tmps = std::string(fname.latin1())+":"+(*iteri)->getDecName();
		tmps.erase(tmps.length()-1, 1);
		paths.push_back(tmps);
	      }
	    }
	  }
	}  
      }
      printf("DEBUG %d\n",ideb); ideb++;
      unsigned int i;
      for(i=0;i<paths.size();i++){
	data.push_back(new PixDBData("fit data", paths[i].c_str(), modules[i].c_str()));
      }
    }

    printf("DEBUG %d\n",ideb); ideb++;
    delete file;
    delete dl;
    printf("DEBUG %d\n",ideb); ideb++;

    if(data.size()>0){
      printf("DEBUG %d\n",ideb); ideb++;
      PSFitWin *fw = new PSFitWin(0,"fitwin",true, 0, data[0]);
      if(fw->exec()){
	// open dialog to save fit results into RootDB
	ParFitfun *pff = new ParFitfun(0, "pff", true, 0, data[0]);
	pff->setCaption("Saving parameters to RootDB file");
	pff->funcBox->setCurrentItem(fw->FitType->currentItem());
	pff->funcBox->setEnabled(false);
	pff->genHistoBoxes(fw->FitType->currentItem(), false, true);
	if(pff->exec()){
	  // fit
	  int npar = fw->m_fitClass->getNPar(fw->FitType->currentItem());
	  float *pars;
	  bool *pfix;
	  int *phis;
	  pars = new float[npar];
	  pfix = new bool[npar];
	  phis = new int[npar];
	  for(int i=0;i<npar; i++){
	    pars[i] = (float)fw->m_parValues[i]->value();
	    pfix[i] = fw->m_parFixed[i]->isChecked();
	  }
	  
  printf("DEBUG %d\n",ideb); ideb++;
	  PixScan *ps = new PixScan();
  printf("DEBUG %d\n",ideb); ideb++;
	  PixScan::HistogramType type = (PixScan::HistogramType) (ps->getHistoTypes()[fw->histoType->currentText().latin1()]);
	  PixScan::HistogramType errType = PixScan::MAX_HISTO_TYPES;
	  if(fw->fromHistoBox->isChecked())
	    errType = (PixScan::HistogramType)(ps->getHistoTypes()[fw->errorCBox->currentText().latin1()]);
	  delete ps;
	  
	  float frErr = 0;
	  if(fw->fractBox->isChecked())
	    frErr = fw->fractEdit->text().toFloat();
	  std::vector<int> parhis;
	  pff->getPars(parhis, true);
	  for(unsigned int k=0;k<parhis.size();k++) phis[k] = parhis[k];
	  
	  for(int i=0;i<(int)data.size();i++){
	    // determine chip flavour to guess max. no of FEs
	    bool isFei4 = false;
	    PixLib::Config &mcfg = data[i]->getModConfig();
	    if(mcfg["general"].name()!="__TrashConfGroup__" && mcfg["general"]["FE_Flavour"].name()!="__TrashConfObj__"){
	      std::string fstrg =  ((ConfList&)mcfg["general"]["FE_Flavour"]).sValue();
	      if(fstrg=="FE_I4" || fstrg=="FE_I4A" || fstrg=="FE_I4B") isFei4 = true;
	    }
	    int minc = 0; 
	    int maxc = isFei4?2:NCHIP;
	    if(chip_in>=0){
	      minc = chip_in;
	      maxc = minc+1;
	    }

	    PixScan::HistogramType oldChiHi=PixScan::MAX_HISTO_TYPES;
	    if(fw->refitBox->isChecked())
	      oldChiHi = (PixScan::HistogramType)parhis[parhis.size()-1]; // last argument is old chi2 histo
	    tmptxt = ("Initialising fit for module " + modules[i]).c_str();
	    emit sendText(tmptxt);
	    m_app->processEvents();
	    data[i]->initFit(type,fw->loopSel->value(),pars, pfix, phis, fw->FitType->currentItem(),errType, oldChiHi);
	    for(int chip=minc;chip<maxc;chip++){
	      tmptxt = "Fitting module " + QString(modules[i].c_str())+", chip " + QString::number(chip);
	      if(fw->refitBox->isChecked())
		tmptxt +=  " (only bad fits)";
	      emit sendText(tmptxt);
	      m_app->processEvents();
	      data[i]->fitHisto(fw->FitType->currentItem(), chip,(float)fw->ChiCut->value(),(float)fw->FitXmin->value(),
				(float)fw->FitXmax->value(),frErr,fw->divndfBox->isChecked());
	    }
	    tmptxt = ("Saving fit results for module " + modules[i]).c_str();
	    emit sendText(tmptxt);
	    m_app->processEvents();
	    for(int k=0;k<(int)parhis.size();k++){
	      PixScan::HistogramType stype = (PixLib::PixScan::HistogramType)parhis[k];
	      Histo *hi = data[i]->getParHisto(k);
	      if(fw->refitBox->isChecked()){ // merge old and new histos
		Histo *oldHi = data[i]->getGenericPixLibHisto(stype, 0, fw->loopSel->value()+1);
		Histo *oldChi = data[i]->getGenericPixLibHisto(oldChiHi, 0, fw->loopSel->value()+1);
		for(int binx=0;binx<hi->nBin(0);binx++){
		  for(int biny=0;biny<hi->nBin(1);biny++){
		    if(oldHi!=0 && oldChi!=0 && (*oldChi)(binx, biny) < (double)fw->ChiCut->value()){
		      hi->set(binx, biny, (*oldHi)(binx, biny));
		    }
		  }
		}
	      }
	      data[i]->writePixLibHisto(hi, stype, 0, fw->loopSel->value()+1);
	    }
	    tmptxt = ("Clearing tmp. fit results for module " + modules[i]).c_str();
	    emit sendText(tmptxt);
	    m_app->processEvents();
	    data[i]->clearFit(true);
	  }
	  // clean up
	  delete[] pars;
	  delete[] pfix;
	  delete[] phis;
	}
	// clean up
	delete pff;
      }
      delete fw;
    }
    
    // clean up data
    for(unsigned int i=0;i<data.size();i++){
      delete data[i];
    }

    tmptxt = "Done!";
    emit sendText(tmptxt);
    m_app->processEvents();
  }
}
MAFitting::FitThread::FitThread(PixDBData &data, int fittype, int chip, float chicut, float xmin, float xmax, float fracErr, bool useNdof) :
  m_data(data), m_fittype(fittype), m_chip(chip), m_chicut(chicut), m_xmin(xmin), m_xmax(xmax), m_fracErr(fracErr), m_useNdof(useNdof){
}
void MAFitting::FitThread::run(){
  m_data.fitHisto(m_fittype, m_chip, m_chicut, m_xmin, m_xmax, m_fracErr, m_useNdof);
}
