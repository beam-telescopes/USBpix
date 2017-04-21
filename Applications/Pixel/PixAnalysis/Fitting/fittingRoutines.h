#ifndef _FITTINGROUTINES_H
#define _FITTINGROUTINES_H

#define NCHIPS 16
#define NCOLS 18
#define NROWS 160
#define NPIXELS (NCHIPS*NCOLS*NROWS)
#define MAXBINS 256
#define NPMAX 4

#include "vfitXface.h"

typedef struct { 
	float mu, sigma, a0, sigmaInv;
} GaussianCurve;

typedef struct {
	float par[NPMAX];
} FitPars;

typedef struct {
	unsigned int pixelId, tPixel, iterations;
	GaussianCurve s;
	float chi2;
} PixelFit;

typedef struct {
	float *x, *y, *w, *ye; 
	int n;
} FitData;

#if 0
/* jsv. in a perfect world */
typedef struct {
	GaussianCurve s;
	float chi2;
} PixelFit;
#endif

typedef struct {
	float dt, tBegin, tFinal;
} ManualSeek;

typedef struct {
	float deltaMu, deltaSigma; /* user provides initial deltaMu and deltaSigma */
	float muEpsilon, sigmaEpsilon; /* user provides convergence criterion */
	float sigmaMin; /* sigma values below sigmaMin are rounded to this value */
	float concavitySmallness; /* determines smallness criterion for 2nd derivative */
	int nIters, maxIters, converge, muConverge, sigmaConverge;
	int ndf; /* number of degrees of freedom */
	float chi2; /* final result for chi2 */
	float chi2CutOff; /* if we get chi2 greater than this value, abandon calculation */
	float muCutOff, sigmaCutOff; /* if we get values greater than these, abandon */
	void * curve; /* additional information about the curve */
/* options */
	int extraIter; /* 1 = after convergence, iterate once more for good measure */
	float (*findMinSigma)(FitData *pfd, GaussianCurve *ps, void *pFit);
	float (*findMinMu)(FitData *pfd, GaussianCurve *ps, void *pFit);
	float (*chiSquared)(FitData *pfd,GaussianCurve *s);
	int (*sGuess)(FitData *pfd,GaussianCurve *sParams,void *vpfit);
	int (*fitSCurve)(FitData *pfd, void *vpfit);
	ManualSeek manualSeekMu, manualSeekSigma;
} Fit;

typedef struct {
  int npars;
  float deltaPar[NPMAX]; /* user provides initial deltaMu and deltaSigma */
  float epsilonPar[NPMAX]; /* user provides convergence criterion */
  float minPar[NPMAX]; /* par values below minPar are rounded to this value */
  float concavitySmallness; /* determines smallness criterion for 2nd derivative */
  int nIters, maxIters, converge, convergePar[NPMAX];
  int ndf; /* number of degrees of freedom */
  float chi2; /* final result for chi2 */
  float chi2CutOff; /* if we get chi2 greater than this value, abandon calculation */
  float cutOffPar[NPMAX]; /* if we get values greater than these, abandon */
  void * curve; /* additional information about the curve */
  /* options */
  int extraIter; /* 1 = after convergence, iterate once more for good measure */
  float (*findMinSigma)(FitData *pfd, FitPars *ps, void *pFit);
  float (*findMinMu)(FitData *pfd, FitPars *ps, void *pFit);
  float (*fxn)(FitData *pfd,FitPars *s);
  float (*chiSquared)(FitData *pfd,FitPars *s);
  int (*parGuess)(FitData *pfd,FitPars *sParams,void *vpfit);
  int (*fitSCurve)(FitData *pfd, void *vpfit);
  ManualSeek manualSeekMu, manualSeekSigma;
} FitCtrl;

typedef struct {
	float x, y;
 } Point;

/* function declarations */

void setSigma(GaussianCurve *ps,float sigma);
float errf_ext(float x, GaussianCurve *psp);
float *log_ext(float x, GaussianCurve *psp);
float log_e(float x);
float abs_ext(float x);
int sGuess(FitData *pfd, GaussianCurve *pSParams, void *vpfit);
int totGuess(FitData *pfd, FitPars *Params, void *vpfit);
int totGuessNew(FitData *pfd, FitPars *Params, void *vpfit);
int totGuessLinear(FitData *pfd, FitPars *Params, void *vpfit);
float nextAbscissa(Point *pt);
float parabolicAbscissa(Point *pt);
int fitSCurveAim(FitData *pfd, void *vpfit);
int fitSCurveAimTot(FitData *pfd, void *vpfit);
int fitSCurveCrawl(FitData *pfd, void *vpfit);
int fitSCurveParabolicAlgorithm(FitData *pfd, void *vpfit);
int fitSCurveBruteForce(FitData *pfd, void *vpfit);
int defaultFit(Fit *pfit);
float chiSquared(FitData *pfd,GaussianCurve *s);
float logLikelihood(FitData *pfd, GaussianCurve *s);
float logLikelihoodTot(FitData *pfd, FitPars *s);
float logLikelihoodTotNew(FitData *pfd, FitPars *s);
float logLikelihoodTotLinear(FitData *pfd, FitPars *s);
int extractGoodData(FitData *pfdi,FitData *pfdo);

extern float *data_weight;
// extern far float data_errf[];
// extern far float data_logx[];
extern float inverse_lut_interval;
extern float inverse_weight_lut_interval;

#define WEIGHT_LUT_LENGTH 1001
#define LUT_WIDTH 3500
#define LUT_LENGTH (2*LUT_WIDTH+1)

typedef struct {
	float data_weight[WEIGHT_LUT_LENGTH];
	float data_errf[LUT_LENGTH];
	float data_logx[LUT_LENGTH * 2];
} FitLUTs;


typedef struct {
	int id, nBins, *binCount;
	float xStart, xFinal, dx;
	void (*fill)(void *pHist,float y);
	void (*close)(void *pHist);
} Hist1F;

Hist1F *hist1FOpen(int nBins,float xStart,float xFinal);
void initFitting(void *workArea);

float manualFindMinSigma(FitData *pfd, GaussianCurve *ps, void *pFit);
float manualFindMinMu(FitData *pfd, GaussianCurve *ps, void *pFit);
int fitTotCal(float *x, float *y, float *yerr, int n, TotCal *calpar, int flags, float *chi2, float *nIters);
#endif
