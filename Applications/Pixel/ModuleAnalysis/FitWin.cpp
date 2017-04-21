#include "FitWin.h"
#include "RootStuff.h"
#include "TopWin.h"
#include <q3table.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qstring.h>

#include <math.h>

FitWin::FitWin( QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl )
	      //    : FitWinBase( parent, name, modal, fl )
{
  setupUi(this);
  QObject::connect(ChipCalBox, SIGNAL(clicked()), this, SLOT(CPCButt_pressed()));
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(safe_accept()));
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(LoadButt, SIGNAL(clicked()), this, SLOT(Load_Clicked()));
  QObject::connect(FitType, SIGNAL(activated(int)), this, SLOT(FitSelected(int)));

  rootmain = ((TopWin*) parent)->rootmain;
  QString varname;
  int i;
  ChipCfac->horizontalHeader()->setLabel(0,"Slope");
  ChipCfac->horizontalHeader()->setLabel(1,"Offset");
  ChipCfac->horizontalHeader()->setLabel(2,"Quadr.");
  ChipCfac->horizontalHeader()->setLabel(3,"Cubic");
  for(i=0;i<ChipCfac->numCols();i++) {
    ChipCfac->setColumnWidth(i,62);
  }
  setCalTable(rootmain->GetCal());

  for(i=0;i<NPARMAX;i++){
    varname.sprintf("TextPar_%d",i);
    TextPar[i] = new QLabel( ConversionBox, varname );
    TextPar[i]->setGeometry( QRect( 26, 47+25*i, 28, 22 ) ); 
    varname.sprintf("Par%d",i);
    TextPar[i]->setText( tr( varname ) );

    varname.sprintf("SetPar_%d",i);
    SetPar[i] = new QLineEdit( ConversionBox, varname );
    SetPar[i]->setGeometry( QRect( 60, 47+25*i, 110, 22 ) ); 

    varname.sprintf("FixPar_%d",i);
    FixPar[i] = new QCheckBox( ConversionBox, varname );
    FixPar[i]->setGeometry( QRect( 176, 47+25*i, 35, 22 ) ); 
    FixPar[i]->setText( tr( "fix" ) );
  }

  FitSelected(FitType->currentItem());
}

FitWin::~FitWin()
{
    // no need to delete child widgets, Qt does it all for us
}
void FitWin::CPCButt_pressed()
{
  if(ChipCalBox->isOn()){
    ChipCfac->show();
    LoadButt->show();
    CfacEdit->hide();
  } else{
    CfacEdit->show();
    LoadButt->hide();
    ChipCfac->hide();
  }
}
void FitWin::safe_accept(){
  bool isOK;
  int i, j, ntr=0;
  if(ChipCalBox->isOn()){  // calibration per chip
    float *ftmp[NCHIP];
    for(i=0;i<NCHIP;i++){
      ftmp[i] = new float[4];
      for(j=0;j<4;j++){
	ftmp[i][j] = ChipCfac->text(i,j).toFloat(&isOK);
	if(isOK)
	  ntr++;
      }
    }
    if(ntr==4*NCHIP) rootmain->SetCal(ftmp);
    for(i=0;i<NCHIP;i++)
      delete[] ftmp[i];
  } else {                      // calibration per entire module
    float ftmp;
    ntr = sscanf(CfacEdit->text().latin1(),"%f",&ftmp);
    if(ntr==1) rootmain->SetCal(ftmp);
  }
  accept();
}

void FitWin::Load_Clicked(){
  int i,j,chipID;
  float Clow, Chigh, VCAL_sl, VCAL_off, VCAL_qu, VCAL_cu, offset, Cap;
  float **calfac;
  calfac = new float*[NCHIP];
  for(i=0;i<NCHIP;i++){
    calfac[i] = new float[4];
    for(j=1;j<4;j++)
      calfac[i][j] = 0;
  }
  QStringList filter("Capvcal file (*capvcal.txt)");
  filter +="Any file (*.*)";
  QString path;
  Q3FileDialog fdia(QString::null, QString::null,0,0,"select calibration file");;
  fdia.setFilters(filter);
  path = QString::null;
  if(fdia.exec() == QDialog::Accepted) 
    path = fdia.selectedFile();
  if(path.isEmpty()) return;
  FILE *in = fopen(path.latin1(),"r");
  char tmp[100], chipNo[6];
  if(in==NULL){
    QMessageBox::warning(this,"Module Analysis","can't open file");
    return;
  }
  if(fdia.selectedFilter()=="Capvcal file (*capvcal.txt)"){
    QMessageBox whichcap("Which capacitance?", 
			 "Do you want to use Clow or Chigh?",
			 QMessageBox::Question,QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton,
			 this, "choosecap",TRUE,Qt::WType_TopLevel);
    whichcap.setButtonText(QMessageBox::Yes,QString("Clow"));
    whichcap.setButtonText(QMessageBox::No ,QString("Chigh"));
    bool capsel = (whichcap.exec()==QMessageBox::Yes);
    // capvcal format
    fscanf(in,"%s",tmp); // read header
    for(i=0;i<NCHIP;i++){
      fscanf(in,"%d %s %f %f %f %f %e %e %f",&chipID, chipNo, &Clow, &Chigh, &VCAL_sl, &VCAL_off, 
	     &VCAL_qu, &VCAL_cu, &offset);
      if(VCAL_sl>0){
	VCAL_cu  = 0;
	VCAL_qu  = 0;
	VCAL_off = 0;
	offset   = 0;
      } else
	VCAL_sl  = -VCAL_sl;
      if(capsel)
	Cap = Clow;
      else
	Cap = Chigh;
      calfac[chipID][0] = Cap*VCAL_sl/0.160218;
      calfac[chipID][1] = Cap*VCAL_off/0.160218;
      calfac[chipID][2] = Cap*VCAL_qu/0.160218;
      calfac[chipID][3] = Cap*VCAL_cu/0.160218;
    }
  } else{
    // simple format: one calib. entry per chip per line
    for(i=0;i<NCHIP;i++){
      fscanf(in,"%f",&VCAL_sl);
      calfac[i][0] = VCAL_sl;
      for(j=1;j<4;j++)
	calfac[i][j] = 0;
    }
  }
  fclose(in);
  setCalTable(calfac);
  for(i=0;i<NCHIP;i++){
    delete calfac[i];
    calfac[i] = 0;
  }
  delete[] calfac;
  return;
}
void FitWin::FitSelected(int fittype){
  int i;
  // VCAL->e calibration box
  if(fittype<2 || (fittype>8 && fittype<12)){
    ConversionBox->show();
    ConversionBox->setTitle("Conversion DAC->physical value");
    ChipCalBox->show();
    CPCButt_pressed();
  } else{
    ChipCalBox->hide();
    ChipCfac->hide();
    CfacEdit->hide();
    LoadButt->hide();
    if(fittype==5  || fittype==4){
      ConversionBox->show();
      ConversionBox->setTitle("Initial parameter guesses");
    } else{
      ConversionBox->hide();
    }
  }
  // chi^2 and limits not needed for PixLib simple fit
  if(fittype==9){
    FitXmin->setEnabled(false);
    FitXmax->setEnabled(false);
    //    ChiCut->setEnabled(false);
  } else{
    FitXmin->setEnabled(true);
    FitXmax->setEnabled(true);
    //    ChiCut->setEnabled(true);
  }
  // fit parameter guesses
  if(fittype==5){
    for(i=0;i<4;i++){
      TextPar[i]->show();
      SetPar[i]->show();
      FixPar[i]->show();
    }
    for(i=4;i<NPARMAX;i++){
      TextPar[i]->hide();
      SetPar[i]->hide();
      FixPar[i]->hide();
    }
  } else if(fittype==4){
    for(i=0;i<3;i++){
      TextPar[i]->show();
      SetPar[i]->show();
      FixPar[i]->show();
    }
    for(i=3;i<NPARMAX;i++){
      TextPar[i]->hide();
      SetPar[i]->hide();
      FixPar[i]->hide();
    }
  } else{
    for(i=0;i<NPARMAX;i++){
      TextPar[i]->hide();
      SetPar[i]->hide();
      FixPar[i]->hide();
    }
  }
  return;
}

void FitWin::setCalTable(float **){//calfac){
//   char cfac[50];
//   int i,j;
//   for(i=0;i<ChipCfac->numRows();i++) {
//     for(j=0;j<4;j++) {
//       float cval = calfac[i][j];
//       if(fabs(cval)>.01 || cval==0)
// 	sprintf(cfac,"%.2f",cval);
//       else
// 	sprintf(cfac,"%.2e",cval);
//       ChipCfac->setText(i,j,cfac);
//     }
//   }
  return;
}
