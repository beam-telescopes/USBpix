#include "QRootApplication.h"
#include "DataStuff.h"
#include "TopWin.h"
#include "FileWin.h"
#include "ModTree.h"
#include "PDBForm.h"
#include "CutPanel.h"
#include "OptClass.h"
#include "OptWin.h"
#include "MAEngine.h"
#include "LogClasses.h"

#include <BaseException.h>

#include <TApplication.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TFile.h>
#include <TObject.h>
#include <TObjString.h>

#include <q3popupmenu.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qregexp.h>
#include <qcombobox.h>
#include <q3textbrowser.h>
#include <qdatetime.h>
#include <qdesktopwidget.h>

#include <stdio.h>
#include <vector>
#include <string>

int main( int argc, char* argv[] )
{
  int i, ret=0, error=0;
  std::vector<std::string> myargv;
  QString errormsg = "";
  
  // QApp. later screws argv up, so copy it now
  for(i=0;i<argc;i++)
    myargv.push_back(argv[i]);
  
  //  if(argc>2 && argc!=5 && argc!=6 && 
  //   !((argc==5 || argc==6) && myargv[1]=="cfg" && (myargv[2]=="tuned" || myargv[2]=="source"))){
  if( argc<2 || (argc==2 && myargv[1]!="-help")||  // just load root file
      (argc==5 && myargv[3]=="-s") ||              // load ascii data and save to ROOT file
      (argc==4 && myargv[1]=="totcheck") ||        // ToT check on source data from root file
      (argc>3 && argc<8 && myargv[1]=="pdb") ||    // run PDB ana on root file
                                                   // config file generation
      ((argc==8 || argc==7) && myargv[1]=="getrdb") ||
      ((((argc==5 || argc==6) && myargv[1]=="cfg" ) ||
	((argc==5 || argc==6) && myargv[1]=="rootcfg") ||
	(((argc==8 && myargv[5]=="-p") || (argc==9 && myargv[6]=="-p")) && myargv[1]=="cfg")) 
       && (myargv[2]=="tuned" || myargv[2]=="source"))){
    // all OK
  } else {
    errormsg +="Usage:\nModuleAnalysis <ascii-data|data.root>\n"
               "or\nModuleAnalysis <ascii-data-file> <data label> -s <ROOT output file>\n"
               "or\nModuleAnalysis cfg <tuned|source> <data.root> <output-path> [output.cfg] [-p PLL-phase1 PLL-phase2]\n"
               "or\nModuleAnalysis rootcfg <tuned|source> <data.root> <output-path>\n"
               "or\nModuleAnalysis pdb <data.root> <PDB-cfg file> [cut-file] [PDB-user PDB-passwd]\n"
               "or\nModuleAnalysis totcheck <data.root> <output.txt>\n"
               "or\nModuleAnalysis getrdb <module/stave SN> <PDB-user> <PDB-passwd> <meas. type> <output-path> [var. name]\n";
    error = -1;
  }

  QRootApplication *app=0;
  // if config file was requested then load, write and exit
  if(error==0 && (((argc==5 || argc==6) && myargv[1]=="cfg") ||
		  (argc==5 && myargv[1]=="rootcfg") ||
		  (((argc==8 && myargv[5]=="-p") || (argc==9 && myargv[6]=="-p")) && myargv[1]=="cfg"))){
    int type = 5;
    QString root_file = myargv[3].c_str(), cfg_file= myargv[4].c_str(), cfg_name = QString::null;
    // change Windows style if necessary
    root_file.replace(QRegExp("\\"),"/");
    cfg_file.replace(QRegExp("\\"),"/");
    // get phases if requested
    int phase1=-999, phase2=-999;
    if(argc>6){
      sscanf(myargv[argc-2].c_str(),"%d",&phase1);
      sscanf(myargv[argc-1].c_str(),"%d",&phase2);
    }
    // generate files
    if(myargv[2]=="source")
      type = 14;
    if(argc==6 || argc==9){
      cfg_name = myargv[5].c_str();
      error = MAEngine::WriteConfig(type, root_file.latin1(), cfg_file.latin1(), cfg_name.latin1(),
				  true, phase1, phase2);
    } else
      error = MAEngine::WriteConfig(type, root_file.latin1(), cfg_file.latin1(),0,
				  (myargv[1]=="cfg"), phase1, phase2);
    if(error){
      errormsg += "Error writing config data: " + QString::number(error);
      error *=10;
    }
  }
  else if(argc==4 && myargv[1]=="totcheck"){
    int imax, type = 14; // will request source data
    QString root_file = myargv[2].c_str();
    QString out_file  = myargv[3].c_str();

    FILE *out = fopen(out_file.latin1(),"a");
    if(out==0){
      errormsg += "Error opening file ";
      errormsg += out_file.latin1();
      error = -10;
    } else{
      
      //    app = new QRootApplication( 1, argv , 0, false);
      DatSet *data = MAEngine::LoadStdFromRoot(type, root_file.latin1());
      if(data==0){
	errormsg += "Error getting/processing source data from file ";
	errormsg += root_file.latin1();
	error = -20;
	fclose(out);
      } else{ // run the analysis
	std::vector<float> result;
	MAEngine::checkToT(data,result);
	fprintf(out,"Module %s -  ",data->GetLog()->MyModule->modname.c_str());
	imax = (int)result.size();
	for(i=0;i<imax;i++)
	  fprintf(out,"%s: %.2f   ",MAEngine::getToTCheckLabel(i).c_str(),result[i]);
	fprintf(out,"\n");
	fclose(out);
      }
    }
  }
  else if(argc==5 && myargv[3]=="-s"){
    QString infile  = myargv[1].c_str();
    QString inlabel = myargv[2].c_str();
    QString outfile = myargv[4].c_str();
    if(outfile.right(5)!=".root"){
      errormsg += "output file doesn't end on .root ";
      error = -10;
    } else{
      DatSet *data = MAEngine::loadDataAscii(infile.latin1());
      if(data==0){
	errormsg += "can't determine input data type ";
	error = -6;
      } else{
	data->ChangeName(inlabel.latin1());
	int ndata=0;
	QString open_mode = "RECREATE";
	FILE *testf = fopen(outfile.latin1(),"r");
	if(testf!=0){
	  open_mode = "UPDATE";
	  fclose(testf);
	}
	TFile *out = new TFile(outfile.latin1(),open_mode.latin1());
	if(out!=0){

	  bool modmode=true;
	  if(open_mode == "UPDATE"){
	    TObject *tmpto;
	    tmpto = out->Get("ModuleFolder");
	    if(tmpto==0)
	      modmode=false;
	  }
	  
	  TObjString ver_to(VERSION);
	  ver_to.Write("version");
	  QString qcmt = "Saved on ";
	  qcmt += QDateTime::currentDateTime().toString();
	  TObjString comment(qcmt.latin1());
	  comment.Write("comment");

	  QString mname="999999";
	  if(data->GetLog()!=0){
	    mname = data->GetLog()->MyModule->modname.c_str();
	  } else{
	    int pos = infile.findRev("/");
	    if(pos>0){
	      QString tmpname = infile.right(infile.length()-pos-1);
	      pos = tmpname.find("_");
	      if(pos>0)
		mname = tmpname;
	    }
	  }
	  if(modmode && !out->cd(mname.latin1())){
	    out->mkdir(mname.latin1(),"MA module");
	    out->cd(mname.latin1());
	  }

	  if(open_mode == "UPDATE"){
	    TObject *tmpto;
	    tmpto = out->Get("ndat");
	    sscanf(tmpto->GetName(),"%d",&ndata);
	  }
	  char dirnam[50];
	  sprintf(dirnam,"data-%d",ndata);
	  ndata++;

	  gDirectory->mkdir(dirnam,"ModuleAnalysis data");
	  gDirectory->cd(dirnam);
	  data->Write();
	  gDirectory->cd("..");
	  sprintf(dirnam,"%d",ndata);
	  TObjString ndata_to(dirnam);
	  ndata_to.Write("ndat");  
	  out->Close();
	} else{
	  errormsg += "can't open or create output file ";
	  error = -10;
	}
      }
    }
  }
  // automatic download from RDB
  else if((argc==7 || argc==8)&& myargv[1]=="getrdb"){
    // start root and QT application
    app = new QRootApplication( argc, argv , 0, false);
    //
    QString objSN=myargv[2].c_str(), userPDB=myargv[3].c_str(), passwdPDB=myargv[4].c_str();
    QString measType=myargv[5].c_str(), fixedName=myargv[6].c_str(), varName="";
    if(argc==8)
      varName=myargv[7].c_str();
    bool retainName = true;
    if(varName!="")
      retainName=false;
    if(objSN.left(8)=="20210020"){
      QString path = fixedName;
      if(!retainName)
	path += varName;
      MAEngine::getRootFromRDB(objSN, measType, path, retainName,app);
    } else if(objSN.left(10)=="2021200001")
      MAEngine::getStaveRootFromRDB(objSN, userPDB, passwdPDB, retainName, 
			  measType, fixedName, varName,app,0);
    else{
      errormsg += "SN doesn't seem to be from neither module nor stave/sector ";
      error = -9;
    }
    delete app;
    app = 0;
  }
  // automatic PDB analysis/upload
  else if(argc>3 && argc<8 && myargv[1]=="pdb"){
    // start root and QT application
    app = new QRootApplication( argc, argv );
    ModItem *mi=0;
    TopWin *tw = new TopWin(NULL,"Pixel Module Analysis", 0, false, app);
    // digest information from command line
    QString file_name = myargv[2].c_str();
    QString cfg_path, cfg_file=QString::null, cut_file=QString::null;
    QString PDBuser=QString::null, PDBpass=QString::null;
    int pos, tmppos;
    cfg_file = myargv[3].c_str();
    cfg_file.replace(QRegExp("\\\\"),"/");
    cfg_path = cfg_file;
    tmppos = 0;
    while(tmppos>=0){
      pos = tmppos;
      tmppos = cfg_path.find("/",tmppos+1);
    }
    if(pos>0){
      cfg_file.remove(0,pos+1);
      cfg_file.remove(cfg_file.length()-4,4);
      cfg_path.remove(pos+1,cfg_path.length()-pos);
    } else
      cfg_path = "./";
    if(argc==5 || argc==7){
      cut_file = myargv[4].c_str();
      cut_file.replace(QRegExp("\\\\"),"/");
    }
    if(argc==6){
      PDBuser = myargv[4].c_str();
      PDBpass = myargv[5].c_str();
    }
    if(argc==7){
      PDBuser = myargv[5].c_str();
      PDBpass = myargv[6].c_str();
    }
    // open root file
    file_name.replace(QRegExp("\\\\"),"/");
    if(file_name.right(5)==".root")
      mi = tw->LoadRoot_Menu(false,file_name.latin1());
    else{
      errormsg += "Specified file does not end with \".root\"";
      error = -3;
    }
    // load cut file if requested
    if(cut_file!=QString::null){
      int tmperr;
      if((tmperr=tw->m_cutpanel->LoadClass(cut_file.latin1()))!=0){
	errormsg += "Error " + QString::number(tmperr) + " loading cut file " + cut_file + "\n";
	error = -4;
      }
    } else // load default cuts
      tw->m_cutpanel->NewClass();

    // process data
    if(mi!=0 && error==0){
      tw->options->m_Opts->m_pdbpath = cfg_path.latin1();
      PDBForm pdb(tw,"PDB Form Manager",TRUE,Qt::WType_TopLevel,mi);
      if(pdb.retValue){
	errormsg += "Error " + QString::number(pdb.retValue) + " from PDB Form";
	error = -4;
      } else{
	// set profile
	for(int k=0; k<pdb.pdbCombo->count();k++){
	  pdb.pdbCombo->setCurrentItem(k);
	  if(pdb.pdbCombo->currentText()==cfg_file) break;
	}
	pdb.LoadProfile();
	// produce text files
	pdb.CreatePDB();
	/* was only for testing
	// dump text from report
	if(argc==4){
	  FILE *out = fopen(argv[3],"w");
	  if(out!=0){
	    QString outtxt = pdb.ResultWindow->text();
	    outtxt.replace(QRegExp("<br>"),"\n");
	    fprintf(out,"%s",outtxt.latin1());
	    fclose(out);
	  } else{
	    errormsg += "Error from PDB Form while writing text report to file";
	    error = -6;
	  }
	}
	*/
	// automatic java-upload
	if(PDBuser!=QString::null && PDBpass!=QString::null){
	  if(!pdb.m_uploadJava){
		errormsg += "Java environment not set up properly.\nPlease check your java installation\n"
		  "and your CLASSPATH variable\n";
		error = -5;
	  } else if(!pdb.m_cutVersioned){
		errormsg += "Non-standard cuts are used. \nPlease provide standard pixel cuts\n"
		  "or no cut file at all to use default cuts.\n";
		error = -6;
	  } else{
	    QString pdbfiles[4]={"PDBiv.txt","PDBdig.txt","PDBana.txt","PDBsrc.txt"};
	    QString pdbcomm[4]={"UploadTestData","UploadModuleTest","UploadModuleTest","UploadModuleTest"};
	    FILE *exists;
	    for(int nf=0;nf<4 && error==0;nf++){
	      exists = fopen((pdb.m_path+"/"+pdbfiles[nf]).latin1(),"r");
	      if(exists!=0){
		fclose(exists);
		if(pdb.runJavaUpload(pdbcomm[nf], pdbfiles[nf].latin1(), PDBuser, PDBpass)){
		  errormsg += "Error from PDB Form java upload\n";
		  error = -5;
		} else
		  remove((pdb.m_path+"/"+pdbfiles[nf]).latin1());
	      }
	    }
	  }
	}
      }
    }
  }
  // launch main GUI window
  else if(error==0){
    if(app==0)
    // start root and QT application
    app = new QRootApplication( argc, argv );
    TopWin *tw = new TopWin(NULL,"Pixel Module Analysis", 0, false, app);
    tw->show();
    //int xpos = (int)QApplication::desktop()->width()-tw->width()-10;
    //tw->move(xpos,10);
    app->setMainWidget(tw);
    // only one argument, must be a data file -> load it
    if(argc==2){
      ModItem *mi = MAEngine::loadDataAny(myargv[1].c_str(), tw);
      if(mi==0){
	errormsg += "Error: can't determine data type\n";
	error = -6;
      }
    }
    if(error==0){
      try{
	ret = app->exec();
      } catch(std::exception& s){
	std::stringstream msg;
	msg << "Std-lib exception ";
	msg << s.what();
	printf("%s not caught in execution of MA.\n", msg.str().c_str());
      } catch(SctPixelRod::BaseException& s){
	std::stringstream msg;
	msg << "SctPixelRod exception ";
	s.what(msg);
	printf("%s not caught in execution of MA.\n", msg.str().c_str());
      }catch(...){
	printf("Unknown exception not caught in execution of MA.\n");
      }
    }
    delete tw;
  }

  if(error){
    std::string system = gSystem->GetName();
    if(system=="WinNT"){
      if(app==0)
	// start root and QT application
	app = new QRootApplication( argc, argv );
      QMessageBox *mb = new QMessageBox("Errors",errormsg,QMessageBox::Warning, QMessageBox::Ok,0,0,0,
					"errbox",FALSE);
      mb->show();
      app->setMainWidget(mb);
      ret = app->exec() + 100*error;
    } else{
      printf("%s\n",errormsg.latin1());
      ret = error;
    }
  }

  return ret;
}
