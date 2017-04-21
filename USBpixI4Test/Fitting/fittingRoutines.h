#ifndef _FITTINGROUTINES_H
#define _FITTINGROUTINES_H

#define NCHIPS 16
#define NCOLS 18
#define NROWS 160
#define NPIXELS (NCHIPS*NCOLS*NROWS)
#define MAXBINS 256
#define NPMAX 4

typedef struct { 
	double par[NPMAX];
} FitPars;

typedef struct {
	unsigned int pixelId, tPixel, tModule, iterations;
	FitPars s;
	double chi2;
} PixelFit;

typedef struct {
  double *x, *y, *w, *ye;
	int n;
} FitData;

#if 0
/* jsv. in a perfect world */
typedef struct {
	FitPars s;
	double chi2;
} PixelFit;
#endif

typedef struct {
	double dt, tBegin, tFinal;
} ManualSeek;

typedef struct {
  int npars;
  double deltaPar[NPMAX]; /* user provides initial deltaMu and deltaSigma */
  double epsilonPar[NPMAX]; /* user provides convergence criterion */
  double minPar[NPMAX]; /* par values below minPar are rounded to this value */
  double concavitySmallness; /* determines smallness criterion for 2nd derivative */
  int nIters, maxIters, converge, convergePar[NPMAX];
  int ndf; /* number of degrees of freedom */
  double chi2; /* final result for chi2 */
  double chi2CutOff; /* if we get chi2 greater than this value, abandon calculation */
  double cutOffPar[NPMAX]; /* if we get values greater than these, abandon */
  void * curve; /* additional information about the curve */
  /* options */
  int extraIter; /* 1 = after convergence, iterate once more for good measure */
  double (*findMinSigma)(FitData *pfd, FitPars *ps, void *pFit);
  double (*findMinMu)(FitData *pfd, FitPars *ps, void *pFit);
  double (*fxn)(FitData *pfd,FitPars *s);
  double (*chiSquared)(FitData *pfd,FitPars *s);
  int (*parGuess)(FitData *pfd,FitPars *sParams,void *vpfit);
  int (*fitSCurve)(FitData *pfd, void *vpfit);
  ManualSeek manualSeekMu, manualSeekSigma;
} Fit;

typedef struct {
	double x, y;
 } Point;

/* function declarations */
void setSigma(FitPars *ps,double sigma);
double errf_ext(double x, FitPars *psp);
double *log_ext(double x, FitPars *psp);
double abs_ext(double x);
int sGuess(FitData *pfd, FitPars *pSParams, void *vpfit);
int totGuess(FitData *pfd, FitPars *Params, void *vpfit);
double nextAbscissa(Point *pt);
double parabolicAbscissa(Point *pt);
int fitSCurveAim(FitData *pfd, void *vpfit);
int fitSCurveCrawl(FitData *pfd, void *vpfit);
int fitSCurveParabolicAlgorithm(FitData *pfd, void *vpfit);
int fitSCurveBruteForce(FitData *pfd, void *vpfit);
int defaultFit(Fit *pfit);
PixelFit *fitModule(void *histBase,int nBins);
double chiSquared(FitData *pfd,FitPars *s);
double logLikelihood(FitData *pfd, FitPars *s);
double logLikelihoodToT(FitData *pfd, FitPars *s);
int extractGoodData(FitData *pfdi,FitData *pfdo);
double manualFindMinSigma(FitData *pfd, FitPars *ps, void *pFit);
double manualFindMinMu(FitData *pfd, FitPars *ps, void *pFit);

extern double data_weight[];
extern double data_errf[];
extern double inverse_lut_interval;
extern double inverse_weight_lut_interval;

#endif
