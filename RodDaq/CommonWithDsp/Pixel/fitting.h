#ifndef FITTING_ROUTINES_H
#define FITTING_ROUTINES_H
#include "primList.h"
#include "histogram.h"
#define NCHIPS 16
#define NCOLS 18
#define NROWS 160
#define NPIXELS (NCHIPS*NCOLS*NROWS)
#define MAXBINS (HISTOGRAM_BIN_MAX)

typedef struct { 
	float mu, sigma, a0, sigmaInv;
} GaussianCurve;

typedef struct {
	unsigned int pixelId, tPixel, iterations;
	GaussianCurve s;
	float chi2;
} PixelFit;

typedef struct {
	float *x, *y, *w;
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
	float x, y;
 } Point;

/* function declarations */

void loadFitArrayX(MDAT32 val[], UINT32 nBins, UINT32 mod, UINT32 chip);
void loadFitArrayY(UINT32 binSize, void *baseAddr, UINT32 delta, UINT32 nBins);

void setSigma(GaussianCurve *ps,float sigma);
float errf_ext(float x, GaussianCurve *psp);
float *log_ext(float x, GaussianCurve *psp);
float abs_ext(float x);
int sGuess(FitData *pfd, GaussianCurve *pSParams, void *vpfit);
float nextAbscissa(Point *pt);
float parabolicAbscissa(Point *pt);
int fitSCurveAim(FitData *pfd, void *vpfit);
int fitSCurveCrawl(FitData *pfd, void *vpfit);
int fitSCurveParabolicAlgorithm(FitData *pfd, void *vpfit);
int fitSCurveBruteForce(FitData *pfd, void *vpfit);
int defaultFit(Fit *pfit);
PixelFit *fitModule(void *histBase,int nBins);
float chiSquared(FitData *pfd,GaussianCurve *s);
float logLikelihood(FitData *pfd, GaussianCurve *s);
int extractGoodData(FitData *pfdi,FitData *pfdo);

extern far float data_weight[];
extern far float data_errf[];
extern far float data_logx[];
extern far float inverse_lut_interval;
extern far float inverse_weight_lut_interval;

#define WEIGHT_LUT_LENGTH 1001
#define LUT_WIDTH 3500
#define LUT_LENGTH (2*LUT_WIDTH+1)

typedef struct {
	int id, nBins, *binCount;
	float xStart, xFinal, dx;
	void (*fill)(void *pHist,float y);
	void (*close)(void *pHist);
} Hist1F;

Hist1F *hist1FOpen(int nBins,float xStart,float xFinal);

float manualFindMinSigma(FitData *pfd, GaussianCurve *ps, void *pFit);
float manualFindMinMu(FitData *pfd, GaussianCurve *ps, void *pFit);

#endif
