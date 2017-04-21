#include "FitClass.h"
#include "vfitXface.h"
#include "fittingRoutines.h"

#include <sstream>
#include <math.h>
#include <stdio.h>

//#include <TGraphErrors.h>
//#include <TF1.h>
//#include <TMath.h>

FitClass::FitClass()
{
	m_funcNames.insert(std::make_pair(0,"ScurveFit"));
	m_funcNames.insert(std::make_pair(1,"ToTfit"));
	m_funcNames.insert(std::make_pair(2,"linfit"));
	m_funcNames.insert(std::make_pair(3,"expfit"));
	m_funcNames.insert(std::make_pair(4,"cmplexpfit"));
	m_funcNames.insert(std::make_pair(5,"dblexpfit"));
	m_funcNames.insert(std::make_pair(6,"constfit"));
	m_funcNames.insert(std::make_pair(7,"parbfit"));
	m_funcNames.insert(std::make_pair(8,"gausfit"));
	m_funcNames.insert(std::make_pair(9,"ScurveFitVl"));
	m_funcNames.insert(std::make_pair(10,"ScurveFitVc"));
	m_funcNames.insert(std::make_pair(11,"ToTFitVl"));
	//   m_funcNames.insert(std::make_pair(12,"ToTFitVc"));

	m_funcFullNames.insert(std::make_pair(0,"S-curve fit (erfc)"));
	m_funcFullNames.insert(std::make_pair(1,"ToT-calibration fit"));
	m_funcFullNames.insert(std::make_pair(2,"Linear fit"));
	m_funcFullNames.insert(std::make_pair(3,"Exponential"));
	m_funcFullNames.insert(std::make_pair(4,"1-exp(-x)"));
	m_funcFullNames.insert(std::make_pair(5,"Double-exponential"));
	m_funcFullNames.insert(std::make_pair(6,"Constant"));
	m_funcFullNames.insert(std::make_pair(7,"Parabola"));
	m_funcFullNames.insert(std::make_pair(8,"Gaussian"));
	m_funcFullNames.insert(std::make_pair(9,"S-curve fit Virzi llh"));
	m_funcFullNames.insert(std::make_pair(10,"S-curve fit Virzi chi2"));
	m_funcFullNames.insert(std::make_pair(11,"ToT-calibration fit Virzi llh"));
	//   m_funcFullNames.insert(std::make_pair(12,"ToT-calibration fit Virzi chi2"));

	std::vector<std::string> tmp_names;
	// S-curve
	tmp_names.push_back("threshold");
	tmp_names.push_back("noise");
	tmp_names.push_back("plateau");
	m_parNames.insert(std::make_pair(0,tmp_names));
	tmp_names.clear();
	// ToT fit
	tmp_names.push_back("constant");
	tmp_names.push_back("numerat. par.");
	tmp_names.push_back("denomin. par.");
	m_parNames.insert(std::make_pair(1,tmp_names));
	tmp_names.clear();
	// linear fit
	tmp_names.push_back("constant");
	tmp_names.push_back("gradient");
	m_parNames.insert(std::make_pair(2,tmp_names));
	tmp_names.clear();
	// expon. fit
	tmp_names.push_back("factor");
	tmp_names.push_back("exponent par.");
	m_parNames.insert(std::make_pair(3,tmp_names));
	tmp_names.clear();
	// 1-exp fit
	tmp_names.push_back("constant");
	tmp_names.push_back("factor");
	tmp_names.push_back("exponent par.");
	m_parNames.insert(std::make_pair(4,tmp_names));
	tmp_names.clear();
	// double-exp fit
	tmp_names.push_back("constant");
	tmp_names.push_back("factor");
	tmp_names.push_back("exp. factor");
	tmp_names.push_back("exp. offset");
	m_parNames.insert(std::make_pair(5,tmp_names));
	tmp_names.clear();
	// const. fit
	tmp_names.push_back("constant");
	m_parNames.insert(std::make_pair(6,tmp_names));
	tmp_names.clear();
	// parabola fit
	tmp_names.push_back("constant");
	tmp_names.push_back("gradient");
	tmp_names.push_back("quad. par.");
	m_parNames.insert(std::make_pair(7,tmp_names));
	tmp_names.clear();
	// Gauss fit
	tmp_names.push_back("constant");
	tmp_names.push_back("mean");
	tmp_names.push_back("sigma");
	m_parNames.insert(std::make_pair(8,tmp_names));
	tmp_names.clear();
	// S-curve - Virzi liklh.
	tmp_names.push_back("threshold");
	tmp_names.push_back("noise");
	tmp_names.push_back("plateau");
	m_parNames.insert(std::make_pair(9,tmp_names));
	tmp_names.clear();
	// S-curve - Virzi chi2
	tmp_names.push_back("threshold");
	tmp_names.push_back("noise");
	tmp_names.push_back("plateau");
	m_parNames.insert(std::make_pair(10,tmp_names));
	tmp_names.clear();
	// ToT fit - Virzi liklh.
	tmp_names.push_back("constant");
	tmp_names.push_back("numerat. par.");
	tmp_names.push_back("denomin. par.");
	m_parNames.insert(std::make_pair(11,tmp_names));
	tmp_names.clear();
	//   // ToT fit - Virzi chi2
	//   tmp_names.push_back("constant");
	//   tmp_names.push_back("numerat. par.");
	//   tmp_names.push_back("denomin. par.");
	//   m_parNames.insert(std::make_pair(12,tmp_names));
	//   tmp_names.clear();

}
int FitClass::getNPar(int funcID)
{
	switch(funcID){
  case 0: // S-curve
	  return 3;
  case 1: // ToT-fit
	  return 3;
  case 2: // linear fit
	  return 2;
  case 3: // expon. fit
	  return 2;
  case 4: // compl. expon. fit
	  return 3;
  case 5: // double expon. fit
	  return 4;
  case 6: // constant
	  return 1;
  case 7: // parabola fit
	  return 3;
  case 8: // Gauss fit
	  return 3;
  case 9: // S-curve - Virzi likelihood
	  return 3;
  case 10: // S-curve - Virzi chi2
	  return 3;
  case 11: // ToT-fit - Virzi likelihood
	  return 3;
	  //   case 12: // ToT-fit - Virzi chi2
	  //     return 3;
  default:
	  return 0;
	}
}
void* FitClass::getFunc(int funcID)
{
	switch(funcID){
  case 0: // S-curve
	  return (void*)erfcfun;
  case 1: // ToT fit
	  return (void*)totfun;
  case 2: // linear fit
	  return (void*)linfun;
  case 3: // expon. fit
	  return (void*)expfun;
  case 4: // compl. expon. fit
	  return (void*)cplexpfun;
  case 5: // double expon. fit
	  return (void*)dblexpfun;
  case 6: // constant
	  return (void*)constfun;
  case 7: // parabola fit
	  return (void*)parbfun;
  case 8: // Gauss fit
	  return (void*)gausfun;
  case 9: // S-curve - Virzi likelihood
	  return (void*)erfcfun;
  case 10: // S-curve - Virzi chi2
	  return (void*)erfcfun;
  case 11: // ToT fit - Virzi likelihood
	  return (void*)totfun;
	  //   case 12: // ToT fit - Virzi chi2
	  //     return (void*)totfun;
  default:
	  return 0;
	}
}
bool FitClass::hasVCAL(int funcID)
{
	switch(funcID){
  case 0: // S-curve
  case 1: // ToT fit
  case 9: // S-curve - Virzi likelihood
  case 10: // S-curve - Virzi chi2
  case 11: // ToT fit - Virzi likelihood
  case 12: // ToT fit - Virzi chi2
	  return true;
  case 2: // linear fit
  case 3: // expon. fit
  case 4: // compl. expon. fit
  case 5: // double expon. fit
  case 6: // constant
  case 7: // parabola fit
  case 8: // Gauss fit
  default:
	  return false;
	}
}
int FitClass::runVirzi(int funcID)
{
	switch(funcID){
  case 9: // S-curve - Virzi likelihood
  case 11: // ToT fit - Virzi likelihood
	  return MA_MAX_LIKELIHOOD;
  case 10: // S-curve - Virzi chi2
	  //   case 12: // ToT fit - Virzi chi2
	  return MA_MIN_CHI2;
  case 0: // S-curve
  case 1: // ToT fit
  case 2: // linear fit
  case 3: // expon. fit
  case 4: // compl. expon. fit
  case 5: // double expon. fit
  case 6: // constant
  case 7: // parabola fit
  case 8: // Gauss fit
  default:
	  return -1;
	}
}
double FitClass::runFit(int npts, double *x, double *y, double *xerr, double *yerr, double *par, bool *fix_par, int funcID, double xmin, double xmax, bool show)
{
	// temporary implementation, still uses ROOT functionality

	int npar = getNPar(funcID);

	if(hasVCAL(funcID)) // allow special VCAL fit
		npar += 4;

	double chi2=-1;
	int vtype = -1;
	if((vtype=runVirzi(funcID))>-1)// run fit - Virzi
	{    
		double fchi2;
		// VCAL -> e calibration
		int myn;
		double *dx, *dy, *dye;
		dx  = new double[npts];
		dy  = new double[npts];
		dye = new double[npts];
		myn=0;
		for(int n=0;n<npts;n++)
		{
			if(xmin==xmax && xmin==0 && xmax==0)// no limits
			{ 
				dx[myn]  = par[3]+x[n]*par[4]+par[5]*x[n]*x[n]+par[6]*x[n]*x[n]*x[n];
				dy[myn]  = y[n];
				dye[myn] = yerr[n];
				myn++;
			} else// remove points outside limits
			{ 
				if(x[n]>xmin && x[n]<xmax){
					dx[myn]  = par[3]+x[n]*par[4]+par[5]*x[n]*x[n]+par[6]*x[n]*x[n]*x[n];
					dy[myn]  = y[n];
					dye[myn] = yerr[n];
					myn++;
				}
			}
		}
		void *fptr = getFunc(funcID);
		if(fptr == (void*)erfcfun)// S-curve fit
		{ 
			SCurve curve;
			fitSCurve(dx, dy, myn, &curve, &fchi2, vtype);
			par[0] = curve.mu;
			par[1] = curve.sigma;
			par[2] = curve.a0;
		}else if(fptr == (void*)totfun)// TOT-calib. fit
		{ 
			ToTcal curve;
			fitToTcal(dx, dy, dye, myn, &curve, &fchi2, vtype);
			par[0] = curve.cstpar;
			par[1] = curve.numpar;
			par[2] = curve.denompar;
		}
		// nothing else is supported, do nothing
		delete[] dx;
		delete[] dy;
		delete[] dye;
		chi2 = (double) fchi2;
	} else{
		//     // run fit - minuit
		//     std::string opt="Q0";
		//     if(show) opt="0";

		//     // guess parameters if all eq. 0
		//     bool guess=true;
		//     for(i=0;i<(npar-4*(int)hasVCAL(funcID));i++){
		//       if(par[i]!=0) guess = false;
		//     }
		//     if(guess)
		//       guessPar(funcID, x, y, npts, par);
		//     // prepare fit function
		//     std::string fname = "fcf";
		//     std::stringstream a;
		//     a << std::hex << (unsigned long int) this;
		//     fname += a.str();
		//     TF1 f(fname.c_str(),(double (*)(double *, double *))getFunc(funcID),x[0], x[npts-1],npar);
		//     f.SetParameters(par);
		//     if(fix_par!=0){ // some parameters are not supposed to be fitted
		//       for(i=0;i<npar;i++){
		// 	if(fix_par[i]){
		// 	  if(par[i]!=0)
		// 	    f.SetParLimits(i,par[i], par[i]);//fixes parameters
		// 	  else{
		// 	    f.SetParameter(i,1e-9);
		// 	    f.SetParLimits(i,1e-9,1e-9);
		// 	  }
		// 	}
		//       }
		//     }
		//     // create graph for fitting
		//     TGraphErrors g(npts, x, y, xerr, yerr);
		//     for(i=0;i<npts;i++)
		//       //printf("%lf %lf %lf %lf\n",x[i],y[i],xerr[i],yerr[i]);
		//       if(xmin!=xmax || xmin!=0 || xmax!=0)
		// 	g.Fit(fname.c_str(),opt.c_str(),"",xmin,xmax);
		//       else
		// 	g.Fit(fname.c_str(),opt.c_str());
		//     // get parameters and chi2
		//     chi2 = f.GetChisquare();
		//     for(i=0;i<npar;i++)
		//       par[i] = f.GetParameter(i);
		//     //printf("chi2 %lf\n",chi);
		//     //scanf("%d",&i);
		printf("ERROR: not implemented without ROOT\n");
	}

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
void FitClass::guessPar(int funcID, double *x, double *y, int npts, double *par)
{
	switch(funcID){
  case 1: // ToT fit
  case 11: // Virzi llh ToT fit
	  guesstot(x,y,npts, par);
	  return;
	  // the following don't need guessing
  case 9: // S-curve - Virzi likelihood
  case 10: // S-curve - Virzi chi2
  default: // rest not implemented
	  return;
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
	FitPars *fp = new FitPars();
	fp->par[0] = par[0];
	fp->par[1] = (double)sqrt(2.)*par[1];
	if(par[2]!=0)
		return par[2]*errf_ext((double)QfromVcal, fp);
	//    return 0.5*par[2]*TMath::Erfc((par[0]-QfromVcal)/sqrt(2.)/par[1]);
	else
		return 0.;
}
double FitClass::totfun(double *x, double *par)
{
	double denom = par[3]+x[0]*par[4]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0]  + par[2];
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
	return par[0]*exp(-x[0]*par[1]);
}
double FitClass::cplexpfun(double *x, double *par)
{
	return (par[0] - par[1]*(exp(-x[0]*par[2])-1));
}
double FitClass::dblexpfun(double *x, double *par)
{
	return par[0] + par[1]/2*(exp(par[2]*(x[0]-par[3])) - exp(-par[2]*(x[0]-par[3])));
}
double FitClass::constfun(double *x, double *par)
{
	return par[0];
}
double FitClass::parbfun(double *x, double *par)
{
	return x[0]*x[0]*par[2]+x[0]*par[1]+par[0];
}
double FitClass::gausfun(double *x, double *par)
{
	return par[0]*exp((par[1]-x[0])*(par[1]-x[0])/2/par[2]);
}
