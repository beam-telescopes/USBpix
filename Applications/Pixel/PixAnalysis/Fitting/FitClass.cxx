#include "FitClass.h"
#include "vfitXface.h"
#include "fittingRoutines.h"

#include <sstream>

#ifdef WIN32
// get rid of ROOT warning - nothing we can do about them
#pragma warning(disable: 4996)
#endif
#include <TGraphErrors.h>
#include <TF1.h>
#include <TMath.h>

FitClass::FitClass()
{
  m_funcNames.insert(std::make_pair(0,"ScurveFit"));
  m_funcNames.insert(std::make_pair(1,"ToTfitNew"));
  m_funcNames.insert(std::make_pair(2,"ToTfit"));
  m_funcNames.insert(std::make_pair(3,"constfit"));
  m_funcNames.insert(std::make_pair(4,"linfit"));
  m_funcNames.insert(std::make_pair(5,"parbfit"));
  m_funcNames.insert(std::make_pair(6,"cubfit"));
  m_funcNames.insert(std::make_pair(7,"expfit"));
  m_funcNames.insert(std::make_pair(8,"cmplexpfit"));
  m_funcNames.insert(std::make_pair(9,"dblexpfit"));
  m_funcNames.insert(std::make_pair(10,"gausfit"));
  m_funcNames.insert(std::make_pair(11,"boxfit"));
  m_funcNames.insert(std::make_pair(12,"ScurveFitVl"));
  m_funcNames.insert(std::make_pair(13,"ScurveFitVc"));
  m_funcNames.insert(std::make_pair(14,"ToTfitVl"));
  m_funcNames.insert(std::make_pair(15,"ToTfitVlNew"));
  m_funcNames.insert(std::make_pair(16,"TOT_FE_I4_pol2"));

  m_funcFullNames.insert(std::make_pair(0,"S-curve fit (erfc)"));
  m_funcFullNames.insert(std::make_pair(1,"ToT-calibration fit (current style)"));
  m_funcFullNames.insert(std::make_pair(2,"ToT-calibration fit (TurboDAQ-style)"));
  m_funcFullNames.insert(std::make_pair(3,"Constant"));
  m_funcFullNames.insert(std::make_pair(4,"Linear fit"));
  m_funcFullNames.insert(std::make_pair(5,"Parabola"));
  m_funcFullNames.insert(std::make_pair(6,"Cubic fit"));
  m_funcFullNames.insert(std::make_pair(7,"Exponential"));
  m_funcFullNames.insert(std::make_pair(8,"1-exp(-x)"));
  m_funcFullNames.insert(std::make_pair(9,"Double-exponential"));
  m_funcFullNames.insert(std::make_pair(10,"Gaussian"));
  m_funcFullNames.insert(std::make_pair(11,"Box fit"));
  m_funcFullNames.insert(std::make_pair(12,"S-curve fit Virzi llh"));
  m_funcFullNames.insert(std::make_pair(13,"S-curve fit Virzi chi2"));
  m_funcFullNames.insert(std::make_pair(14,"ToT-calibration (TurboDAQ-style) Virzi"));
  m_funcFullNames.insert(std::make_pair(15,"ToT-calibration (current style) Virzi"));
  m_funcFullNames.insert(std::make_pair(16,"ToT-calibration FE-I4 Polynomial 2nd order"));

  std::vector<std::string> tmp_names;
  // S-curve
  tmp_names.push_back("threshold");
  tmp_names.push_back("noise");
  tmp_names.push_back("plateau");
  m_parNames.insert(std::make_pair(0,tmp_names));
  tmp_names.clear();
  // new ToT fit p0* (p1*Q)/(p2*Q)
  tmp_names.push_back("factor");
  tmp_names.push_back("numerat. par.");
  tmp_names.push_back("denomin. par.");
  m_parNames.insert(std::make_pair(1,tmp_names));
  tmp_names.clear();
  // ToT fit
  tmp_names.push_back("constant");
  tmp_names.push_back("numerat. par.");
  tmp_names.push_back("denomin. par.");
  m_parNames.insert(std::make_pair(2,tmp_names));
  tmp_names.clear();
  // const. fit
  tmp_names.push_back("constant");
  m_parNames.insert(std::make_pair(3,tmp_names));
  tmp_names.clear();
  // linear fit
  tmp_names.push_back("constant");
  tmp_names.push_back("gradient");
  m_parNames.insert(std::make_pair(4,tmp_names));
  tmp_names.clear();
  // parabola fit
  tmp_names.push_back("constant");
  tmp_names.push_back("gradient");
  tmp_names.push_back("quad. par.");
  m_parNames.insert(std::make_pair(5,tmp_names));
  tmp_names.clear();
  // cubic fit
  tmp_names.push_back("constant");
  tmp_names.push_back("gradient");
  tmp_names.push_back("quad. par.");
  tmp_names.push_back("cubic par.");
  m_parNames.insert(std::make_pair(6,tmp_names));
  tmp_names.clear();
  // expon. fit
  tmp_names.push_back("factor");
  tmp_names.push_back("exponent par.");
  m_parNames.insert(std::make_pair(7,tmp_names));
  tmp_names.clear();
  // 1-exp fit
  tmp_names.push_back("constant");
  tmp_names.push_back("factor");
  tmp_names.push_back("exponent par.");
  m_parNames.insert(std::make_pair(8,tmp_names));
  tmp_names.clear();
  // double-exp fit
  tmp_names.push_back("constant");
  tmp_names.push_back("factor");
  tmp_names.push_back("exp. factor");
  tmp_names.push_back("exp. offset");
  m_parNames.insert(std::make_pair(9,tmp_names));
  tmp_names.clear();
  // Gauss fit
  tmp_names.push_back("constant");
  tmp_names.push_back("mean");
  tmp_names.push_back("sigma");
  m_parNames.insert(std::make_pair(10,tmp_names));
  tmp_names.clear();
  // box fit
  tmp_names.push_back("plateau");
  tmp_names.push_back("centre");
  tmp_names.push_back("width");
  tmp_names.push_back("sigma_left");
  tmp_names.push_back("sigma_right");
  m_parNames.insert(std::make_pair(11,tmp_names));
  tmp_names.clear();
  // S-curve - Virzi liklh.
  tmp_names.push_back("threshold");
  tmp_names.push_back("noise");
  tmp_names.push_back("plateau");
  m_parNames.insert(std::make_pair(12,tmp_names));
  tmp_names.clear();
  // S-curve - Virzi chi2
  tmp_names.push_back("threshold");
  tmp_names.push_back("noise");
  tmp_names.push_back("plateau");
  m_parNames.insert(std::make_pair(13,tmp_names));
  tmp_names.clear();
  // ToT fit - Virzi
  tmp_names.push_back("constant");
  tmp_names.push_back("numerat. par.");
  tmp_names.push_back("denomin. par.");
  m_parNames.insert(std::make_pair(14,tmp_names));
  tmp_names.clear();
  // new ToT fit - Virzi
  tmp_names.push_back("constant");
  tmp_names.push_back("numerat. par.");
  tmp_names.push_back("denomin. par.");
  m_parNames.insert(std::make_pair(15,tmp_names));
  tmp_names.clear();
  // TOT_FE_I4_pol2
  tmp_names.push_back("constant");
  tmp_names.push_back("gradient");
  tmp_names.push_back("quad. par.");
  m_parNames.insert(std::make_pair(16,tmp_names));
  tmp_names.clear();

}
int FitClass::getNPar(int funcID)
{
  switch(funcID){
  case 0: // S-curve
    return 3;
  case 1: // new ToT-fit
    return 3;
  case 2: // TuboDAQ ToT-fit
    return 3;
  case 3: // constant
    return 1;
  case 4: // linear fit
    return 2;
  case 5: // parabola fit
    return 3;
  case 6: // cubic fit
    return 4;
  case 7: // expon. fit
    return 2;
  case 8: // compl. expon. fit
    return 3;
  case 9: // double expon. fit
    return 4;
  case 10: // Gauss fit
    return 3;
  case 11: // box fit
    return 5;
  case 12: // S-curve - Virzi likelihood
    return 3;
  case 13: // S-curve - Virzi chi2
    return 3;
  case 14: // ToT-fit - Virzi
    return 3;
  case 15: // new ToT-fit - Virzi
    return 3;
  case 16: // TOT_FE-i4_pol2
    return 3;
  default:
    return 0;
  }
}
void* FitClass::getFunc(int funcID)
{
  switch(funcID){
  case 0: // S-curve
    return (void*)erfcfun;
  case 1: // new ToT fit
    return (void*)newtotfun;
  case 2: // TurboDAQ ToT fit
    return (void*)totfun;
  case 3: // constant
    return (void*)constfun;
  case 4: // linear fit
    return (void*)linfun;
  case 5: // parabola fit
    return (void*)parbfun;
  case 6: // cubic fit
    return (void*)cubfun;
  case 7: // expon. fit
    return (void*)expfun;
  case 8: // compl. expon. fit
    return (void*)cplexpfun;
  case 9: // double expon. fit
    return (void*)dblexpfun;
  case 10: // Gauss fit
    return (void*)gausfun;
  case 11: // box fit
    return (void*)boxfun;
  case 12: // S-curve - Virzi likelihood
    return (void*)erfcfun;
  case 13: // S-curve - Virzi chi2
    return (void*)erfcfun;
  case 14: // ToT fit - Virzi likelihood
    return (void*)totfun;
  case 15: // ToT fit - Virzi chi2
    return (void*)newtotfun;
  case 16: // TOT_FE_I4_pol2
    return (void*)TOT_FE_I4_pol2;
  default:
    return 0;
  }
}
bool FitClass::hasVCAL(int funcID)
{
  switch(funcID){
  case 0: // S-curve
  case 1: // new ToT fit
  case 2: // TurboDAQ ToT fit
  case 12: // S-curve - Virzi likelihood
  case 13: // S-curve - Virzi chi2
  case 14: // TurboDAQ ToT fit - Virzi
  case 15: // new ToT fit - Virzi
  case 16: // TOT_FE_I4_pol2
    return true;
  case 3: // constant
  case 4: // linear fit
  case 5: // parabola fit
  case 6: // cubic fit
  case 7: // expon. fit
  case 8: // compl. expon. fit
  case 9: // double expon. fit
  case 10: // Gauss fit
  case 11: // box fit
  default:
    return false;
  }
}
int FitClass::runVirzi(int funcID)
{
  switch(funcID){
  case 12: // S-curve - Virzi likelihood
  case 14: // TurboDAQ ToT fit - Virzi
  case 15: // new ToT fit - Virzi
    return VFX_MAX_LIKELIHOOD;
  case 13: // S-curve - Virzi chi2
    return VFX_MIN_CHI2;
  // all others
  default:
    return -1;
  }
}
double FitClass::getChi2(int npts, double *x, double *y, double *yerr, double *par, int funcID){

  int i,nvalp=0;

  double(*fptr)(double *, double *) = (double(*)(double *, double *))getFunc(funcID);
  bool isErf = ((void*)erfcfun)==getFunc(funcID);
  double chisqu = 0.;
  double xval[1];
  double yval;
  for(i=0;i<npts;i++){
    xval[0] = x[i];
    yval = fptr(xval, par);
    if(yerr[i]!=0.){
      chisqu += TMath::Power((y[i]-yval)/yerr[i], 2);
      if(y[i]!=0. || !isErf) nvalp++;
    }
  }

  if(nvalp<3) return -1; // to few data points, so no meaningful chi2 -> failure code -1
  return chisqu;
}
double FitClass::runFit(int npts, double *x, double *y, double *xerr, double *yerr, double *par, bool *fix_par, int funcID, double xmin, double xmax, bool show)
{
  int i, npar = getNPar(funcID);

  if(hasVCAL(funcID)) // allow special VCAL fit
    npar += 4;

  double chi2=-1;
  int vtype = -1;
  if((vtype=runVirzi(funcID))>-1){
    // run fit - Virzi
    float fchi2;
    // VCAL -> e calibration
    int myn;
    float *dx, *dy, *dye;
    dx  = new float[npts];
    dy  = new float[npts];
    dye = new float[npts];
    myn=0;
    for(int n=0;n<npts;n++){
      if(xmin==xmax && xmin==0 && xmax==0){ // no limits
	dx[myn]  = (float) (par[3]+x[n]*par[4]+par[5]*x[n]*x[n]+par[6]*x[n]*x[n]*x[n]);
	dy[myn]  = (float) y[n];
	dye[myn] = (float) yerr[n];
	myn++;
      } else{ // remove points outside limits
	if(x[n]>xmin && x[n]<xmax){
	  dx[myn]  = (float) (par[3]+x[n]*par[4]+par[5]*x[n]*x[n]+par[6]*x[n]*x[n]*x[n]);
	  dy[myn]  = (float) y[n];
	  dye[myn] = (float) yerr[n];
	  myn++;
	}
      }
    }
    void *fptr = getFunc(funcID);
    int niters=0;
    if(fptr == (void*)erfcfun){ // S-curve fit
      float coeff[3]={100.,4000.,200.};
      coeff[0] = (float)par[2];
      //      vtype += VFX_USER_WEIGHTS;
      int status = fitSCurve(dx, dy, dye, myn, &niters, coeff, vtype, &fchi2);
      if(status!=0 && coeff[1]==4000. && coeff[2]==200.){
	par[0] = 0.;
	par[1] = 0.;
	par[2] = 0.;
	fchi2 = -1;
      } else{
	par[0] = (double)coeff[1];
	par[1] = (double)coeff[2];
	par[2] = (double)coeff[0];
      }
    }else if(fptr == (void*)totfun){ // TurboDAQ-style TOT-calib. fit
      TotCal curve;
      fitTotCalOld(dx, dy, dye, myn, &curve, vtype, &fchi2, &niters);
      par[0] = (double)curve.cstpar;
      par[1] = (double)curve.numpar;
      par[2] = (double)curve.denompar;
    }else if(fptr == (void*)newtotfun){ // PixelDAQ-style TOT-calib. fit
      TotCal curve;
      fitTotCalNew(dx, dy, dye, myn, &curve, vtype, &fchi2, &niters);
      par[0] = (double)curve.cstpar;
      par[1] = (double)curve.numpar;
      par[2] = (double)curve.denompar;
    }
    // nothing else is supported, do nothing
    delete[] dx;
    delete[] dy;
    delete[] dye;
    chi2 = (double) fchi2;
  } else{
    // run fit - minuit
    std::string opt="Q0";
    if(show) opt="0";
    
    // guess parameters if all eq. 0
    bool guess=true;
    for(i=0;i<(npar-4*(int)hasVCAL(funcID));i++){
      if(par[i]!=0) guess = false;
    }
    if(guess){
      guessPar(funcID, x, y, npts, par);
      if(funcID==0 && fix_par!=0) fix_par[2] = true; // S-curve fit: fix plateau value
    }
    // prepare fit function
    std::string fname = "fcf";
    std::stringstream a;
    a << std::hex << (unsigned long int) this;
    fname += a.str();
    TF1 f(fname.c_str(),(double (*)(double *, double *))getFunc(funcID),x[0], x[npts-1],npar);
    f.SetParameters(par);
    if(fix_par!=0){ // some parameters are not supposed to be fitted
      for(i=0;i<npar;i++){
	if(fix_par[i]){
	  if(par[i]!=0)
	    f.SetParLimits(i,par[i], par[i]);//fixes parameters
	  else{
	    f.SetParameter(i,1e-9);
	    f.SetParLimits(i,1e-9,1e-9);
	  }
	}
      }
    }
    // create graph for fitting
    TGraphErrors g(npts, x, y, xerr, yerr);
    //for(i=0;i<npts;i++)
      //printf("%lf %lf %lf %lf\n",x[i],y[i],xerr[i],yerr[i]);
      if(xmin!=xmax || xmin!=0 || xmax!=0)
	g.Fit(fname.c_str(),opt.c_str(),"",xmin,xmax);
      else
	g.Fit(fname.c_str(),opt.c_str());
    // get parameters and chi2
    chi2 = f.GetChisquare();
    for(i=0;i<npar;i++)
      par[i] = f.GetParameter(i);
    //printf("chi2 %lf\n",chi);
    //scanf("%d",&i);
  }
  if(chi2>=0) chi2 = getChi2(npts, x, y, yerr, par, funcID);
  return chi2;
}

std::vector<std::string> FitClass::getParNames(int funcID)
{
  static std::vector<std::string> dummy;
  if(funcID < (int)m_parNames.size())
    return m_parNames[funcID];
  else
    return dummy;
}
bool FitClass::guessPar(int funcID){
  // simplified version just to check if guessing is possible
  double x[4]={0.,1.,2.,3.};
  double y[4]={0.,1.,2.,3.};
  double *par = new double[getNPar(funcID)];
  bool retVal = guessPar(funcID, x, y, 4, par);
  delete[] par;
  return retVal;
}
bool FitClass::guessPar(int funcID, double *x, double *y, int npts, double *par)
{
  Fit fit;
  FitData pfd;
  fit.manualSeekSigma.dt = 0.25f;
  fit.manualSeekSigma.tBegin = 0.25f;
  fit.manualSeekSigma.tFinal = 4.0f;
  pfd.x = new float[npts];
  pfd.y = new float[npts];
  pfd.n = npts;
  for(int i=0;i<npts;i++){
    double Q = par[3]+x[i]*par[4]+par[5]*x[i]*x[i]+par[6]*x[i]*x[i]*x[i];
    pfd.x[i] = (float) Q;
    pfd.y[i] = (float) y[i];
  }
  switch(funcID){
  case 2:{ // TurboDAQ ToT fit Minuit
    FitPars Params;
    totGuess(&pfd, &Params, &fit);
      //    guesstot(x,y,npts, par);
    for(int i=0;i<3;i++)
      par[i] = (double)Params.par[i];
    return true;}
  case 1:{ // new ToT fit Minuit
    //    guesstot(x,y,npts, par);
    //    return true;
    FitPars Params;
    totGuessNew(&pfd, &Params, &fit);
    for(int i=0;i<3;i++)
      par[i] = (double)Params.par[i];
    return true;}
  case 0:{ // S-curve Minuit
    GaussianCurve pSParams;
    sGuess(&pfd, &pSParams, &fit);
    par[0] = (double)pSParams.mu;
    par[1] = (double)pSParams.sigma;
    par[2] = (double)pSParams.a0;

    delete[] pfd.x;
    delete[] pfd.y;
    return true;}
  // the following are guessed in Joe's code
  case 12: // S-curve - Virzi likelihood
  case 13: // S-curve - Virzi chi2
  case 14: // Virzi llh ToT fit
  case 15: // Virzi llh new ToT fit
    return true;
  default: // rest not implemented
    return false;
  }
}
void FitClass::guesstot(double *x, double *y, int npts, double *par)
{
  int i, pts[3]={0,npts/2,npts-1};
  double Q[3], tx[1]={x[pts[0]]};
  for(i=0;i<3;i++)
    Q[i] = par[3]+x[pts[i]]*par[4]+par[5]*x[pts[i]]*x[pts[i]]+par[6]*x[pts[i]]*x[pts[i]]*x[pts[i]];

  par[2] = (Q[2]*(Q[1]-Q[0])/(Q[2]-Q[0]) - Q[1]*(y[pts[1]]-y[pts[0]])/(y[pts[2]]-y[pts[0]])) / 
    ((y[pts[1]]-y[pts[0]])/(y[pts[2]]-y[pts[0]])-(Q[1]-Q[0])/(Q[2]-Q[0]));
  par[1] = (y[pts[1]]-y[pts[2]])*(par[2]+Q[1])*(par[2]+Q[2])/(Q[2]-Q[1]);
  par[0] = 0;
  par[0] = y[pts[0]] - totfun(tx,par);
}

//static fit functions
double FitClass::erfcfun(double *x, double *par)
{
  double QfromVcal = par[3]+x[0]*par[4]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  if(par[2]!=0)
    return 0.5*par[2]*TMath::Erfc((par[0]-QfromVcal)/sqrt(2.)/par[1]);
  else
    return 0.;
}
double FitClass::newtotfun(double *x, double *par)
{
  double QfromVcal = par[3]+x[0]*par[4]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  double denom = par[2]+QfromVcal;
  if(denom!=0)
    return par[0]*(par[1]+QfromVcal)/denom;
  else
    return 0;
}
double FitClass::totfun(double *x, double *par)
{
  double denom = par[3]+x[0]*par[4]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0] + par[2];
  if(denom!=0)
    return par[0]+par[1]/denom;
  else
    return 0;
}
double FitClass::linfun(double *x, double *par)
{
  return x[0]*par[1]+par[0];
}
double FitClass::expfun(double *x, double *par)
{
    return par[0]*TMath::Exp(-x[0]*par[1]);
}
double FitClass::cplexpfun(double *x, double *par)
{
  return (par[0] - par[1]*(TMath::Exp(-x[0]*par[2])-1));
}
double FitClass::dblexpfun(double *x, double *par)
{
  return par[0] + par[1]/2*(TMath::Exp(par[2]*(x[0]-par[3])) - exp(-par[2]*(x[0]-par[3])));
}
double FitClass::constfun(double* /*x*/, double *par)
{
  return par[0];
}
double FitClass::parbfun(double *x, double *par)
{
  return x[0]*x[0]*par[2]+x[0]*par[1]+par[0];
}
double FitClass::cubfun(double *x, double *par)
{
  return x[0]*x[0]*x[0]*par[3]+x[0]*x[0]*par[2]+x[0]*par[1]+par[0];
}
double FitClass::gausfun(double *x, double *par)
{
  return par[0]*TMath::Exp((par[1]-x[0])*(par[1]-x[0])/2/par[2]);
}
double FitClass::boxfun(double *x, double *par){
  return par[0]/2.*(TMath::Erf((x[0]-par[1]+par[2]/2.)/(TMath::Sqrt(2.)*par[3]))+
		    TMath::Erf((par[1]+par[2]/2.-x[0])/(TMath::Sqrt(2.)*par[4])));
}
double FitClass::TOT_FE_I4_pol2(double *x, double *par)
{ //double QfromVcal = par[3]+x[0]*par[4]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0];
  return x[0]*x[0]*par[2]+x[0]*par[1]+par[0];
}
int FitClass::getFuncID(std::string func_title){
  int funcID;
  const int FIDmax=99;
  for(funcID=0; funcID<FIDmax;funcID++)
    if(getFuncTitle(funcID)==func_title) break;
  if(funcID==FIDmax) funcID = -1;
  return funcID;
}
