#include "vfitXface.h"
#include "fittingRoutines.h"
#include <stdio.h>
#include <math.h>

static double unitWeights[256] = {
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f
};

static double xVal[MAXBINS], yVal[MAXBINS];
PixelFit pixelFit[NPIXELS];

static double invRoot2 = 0.7071067811865475244;

/* function declarations */
int fitPixel(double *x,double *y,int n, int *nIters, double *coeffs, int flags, double *chi2);
int spFitPixel(double *x, double *y, int n, int *nIters, double *coeffs, int flags, double *chi2);
int defaultFitChi2(Fit *pFit); /* minimize chi2 */
int defaultFitLikelihood(Fit *pFit); /* maximize likelihood */
int defaultFitLikelihoodToT(Fit *pFit);
double *calculateFitWeights(FitData *pfd);

int fitSCurve(double *x, double *y, int n, SCurve *curve, double *chi2, int flags) {
	int nIters;
	int k;
	int status = 0;
	double *fx, *fy, c[3];
	fx = new double [ n ];
	fy = new double [ n ];

	/* convert doubles to double */
	for(k=0;k<n;++k) {
		fx[k] = (double)x[k];
		fy[k] = (double)y[k];
	}
	nIters = 0;
	if((flags == 0) || (flags & MA_MAX_LIKELIHOOD)) {
	  status = spFitPixel(fx, fy, n, &nIters, c, MA_MAX_LIKELIHOOD, chi2);
	} else if(flags & MA_MIN_CHI2) {
	  status = spFitPixel(fx, fy, n, &nIters, c, MA_MIN_CHI2, chi2);
	}
	curve->a0 = c[0];
	curve->mu = c[1];
	curve->sigma = c[2];
	delete [] fx;
	delete [] fy;
	return status;
}

int fitToTcal(double *x, double *y, double *yerr, int n, ToTcal *calpar, double *chi2, int flags){
  int nIters;
  int k;
  int status = 0;
  double *fx, *fy, *fye;
  fx  = new double [ n ];
  fy  = new double [ n ];
  fye = new double [ n ];
  FitPars s;
  Fit fit;
  FitData rawData;

  /* convert doubles to double */
  for(k=0;k<n;++k) {
    fx[k]  = (double)x[k];
    fy[k]  = (double)y[k];
    fye[k] = (double)yerr[k];
  }
  nIters = 0;

  if(!(flags & MA_MAX_LIKELIHOOD)) return -1; // can only do max LH

  rawData.x = fx; rawData.y = fy; rawData.ye = fye; rawData.n = n;

  defaultFitLikelihoodToT(&fit);

  s.par[0] = calpar->cstpar;
  s.par[1] = calpar->numpar;
  s.par[2] = calpar->denompar;
  fit.curve = &s;
  status = fit.fitSCurve(&rawData,&fit);
  calpar->cstpar   = s.par[0];
  calpar->numpar   = s.par[1];
  calpar->denompar = s.par[2];
  *chi2 = fit.chi2;
  //  *nIters = fit.nIters;

  delete [] fx;
  delete [] fy;
  delete [] fye;
  return status;
}
/* completely floating point pixel fitting routine */
int spFitPixel(double *x, double *y, int n, int *nIters, double *coeffs, int flags, double *chi2) {
	FitPars s;
	FitData rawData, extData, *pfd;
	Fit fit;
	int status;	
	if(flags & MA_MAX_LIKELIHOOD) {
		defaultFitLikelihood(&fit);
	} else if(flags & MA_MIN_CHI2) {
		defaultFitChi2(&fit);
	} else {
		return -1;
	}
	if((nIters != 0) && (*nIters > 0)) fit.maxIters = *nIters;
	fit.curve = &s;
	rawData.x = x; rawData.y = y; rawData.n = n;
	status = extractGoodData(&rawData,&extData) ? 0 : 1;
	pfd = &extData;
	if(status) return status;
	pfd->w = (double *)0;
	if(flags & MA_USER_WEIGHTS) {
		pfd->w = calculateFitWeights(&extData);
	} else if(flags & MA_NO_WEIGHTING) {
		pfd->w = unitWeights;
	}
	status = fit.fitSCurve(pfd,&fit);
	coeffs[0] = s.par[2];
	coeffs[1] = s.par[0];
	coeffs[2] = s.par[1] * invRoot2;
	*chi2 = fit.chi2;
	*nIters = fit.nIters;
	return status;
}

/*
 * if coeffs[0] != 0 => initial guess is assumed, not made by routine
 * if coeffs[0] == 0 => initial guess is made by routine
 */

int fitPixel(double *x,double *y, int n, int *nIters, double *coeffs, int flags, double *chi2) {
	FitPars s;
	FitData rawData, extData, *pfd;
	Fit fit;
	int k, status;	
	if(flags & MA_MAX_LIKELIHOOD) {
		defaultFitLikelihood(&fit);
	} else if(flags & MA_MIN_CHI2) {
		defaultFitChi2(&fit);
	} else {
		return -1;
	}
	if((nIters != 0) && (*nIters > 0)) fit.maxIters = *nIters;
	fit.curve = &s;
	for(k=0;k<n;++k) {
		xVal[k] = (double)x[k];
		yVal[k] = (double)y[k];
	}
	rawData.x = xVal; rawData.y = yVal; rawData.n = n;
	status = extractGoodData(&rawData,&extData) ? 0 : 1;
	pfd = &extData;
	if(status) 
		return status;
	if(s.par[2] != 0.0f && (abs_ext(s.par[2]-pfd->y[pfd->n-1]) > 0.001f)) {
		return 1; /* assume plateau = last value. jsv possibly better method? */
	}
	if(flags & MA_USER_GUESS) fit.parGuess = 0; /* actually s.a0 != 0.0 is a good comparison */
	pfd->w = (double *)0;
	if(flags & MA_USER_WEIGHTS) {
		pfd->w = calculateFitWeights(&extData);
	}
	status = fit.fitSCurve(pfd,&fit);
	coeffs[0] = (double)s.par[2];
	coeffs[1] = (double)s.par[0];
	coeffs[2] = (double)s.par[1] * invRoot2;
	*chi2 = (double)fit.chi2;
	*nIters = fit.nIters;
	return status;
}

#if 0
/* uses math function log */
double logLikelihood(FitData *pfd, FitPars *s) {
	double acc, r, p, y2, y3, N;
	double *x, *y, cutOff;
	int k, n;
	x = pfd->x;	y = pfd->y;	n = pfd->n;
	acc = 0.0f;
	N = s->a0;
	cutOff = 1.0e-6f; // jsv
	for(k=0;k<n;++k) {
		r = *y++; /* data point */
		p = errf_ext(*x++,s); /* probability */
		if(p == 0.0f) p =       cutOff; /* jsv */
		if(p == 1.0f) p = 1.0 - cutOff;
		y2 = r * log(p);
		y3 = (N - r) * log(1.0f - p);
		acc -= (y2 + y3);
	}
	return acc;
}
#else
/* uses lookup table for log */
double logLikelihood(FitData *pfd, FitPars *s) {
	double acc, r, y2, y3, N;
	double *x, *y, *p;
	int k, n;
	x = pfd->x;	y = pfd->y;	n = pfd->n;
	acc = 0.0f;
	N = s->par[2];
	for(k=0;k<n;++k) {
		r = *y++; /* data point */
		p = log_ext(*x++,s); /* pointer to log(probability) */
		y2 = r * (*p++); /* log(prob) */
		y3 = (N - r) * (*p); /* log(1-prob) */
		acc -= (y2 + y3);
	}
	return acc;
}
#endif

double logLikelihoodToT(FitData *pfd, FitPars *s) {
  double acc, y2, y3, sig;
  double *x, *y, *ye;
  int k, n;
  bool haveErr;

  x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
  haveErr = (ye!=0);

  acc = 0.0f;
  for(k=0;k<n;++k) {
    y3 = *y++; /* data point */
    if(haveErr)
      sig = *ye++; // error
    else
      sig = y3/10;
    y2 = s->par[0] + s->par[1]/(s->par[2]+*x++);
    //    acc -= log(y2/y3-1);
    if(fabs(sig)>=1e-3*y3 && sig!=0){ // ignore rubbish sigma values
      y2 = (y2-y3)/sig;
      acc -= log(y2);
    }
  }
  return acc;
}
double chiSquaredToT(FitData *pfd,FitPars *s) {
  double *x, *y, *ye, acc, y2, y3, sig;
  int k, n;
  bool haveErr;

  x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
  haveErr = (ye!=0);

  acc = 0.0f;
  for(k=0;k<n;++k) {
    y3 = *y++; /* data point */
    if(haveErr)
      sig = *ye++; // error
    else
      sig = y3/10;
    y2 = s->par[0] + s->par[1]/(s->par[2]+*x++);
    if(fabs(sig)>=1e-3*y3 && sig!=0) // ignore rubbish sigma values
      acc += (y2-y3)*(y2-y3)/sig/sig;
  }
  return acc;
}
double chiSquared(FitData *pfd,FitPars *s) {
	double acc[4], y0[4], y1[4], x0[4], y2[4], y3[4], a0, chi2;
	double *x, *y, *w;
	int j[4], k, m, n;
	x = pfd->x;	y = pfd->y;	n = pfd->n;	w = pfd->w;
	acc[0] = 0.0f; acc[1] = 0.0f; acc[2] = 0.0f; acc[3] = 0.0f;
	m = n % 4;
	a0 = s->par[2];
	if(w == (double *)0) {
	/* four accumulators are used simultaneously because I can implement four simultaneous 
		accumulators in the target processor */
	/* use binomial weighting */
		for(k=0;k<m;++k) {
			x0[0] = *x++; y0[0] = *y++; y1[0] = errf_ext(x0[0],s); j[0] = (int)(y1[0] * inverse_weight_lut_interval); 
			y2[0] = y0[0] - a0 * y1[0]; y3[0] = y2[0] * y2[0]; acc[0] += (y3[0] * data_weight[j[0]]); 
		}
		m = n / 4;
		for(k=0;k<m;++k) {
			x0[0] = *x++; y0[0] = *y++; y1[0] = errf_ext(x0[0],s); j[0] = (int)(y1[0] * inverse_weight_lut_interval); 
			y2[0] = y0[0] - a0 * y1[0]; y3[0] = y2[0] * y2[0]; acc[0] += (y3[0] * data_weight[j[0]]); 
			x0[1] = *x++; y0[1] = *y++; y1[1] = errf_ext(x0[1],s); j[1] = (int)(y1[1] * inverse_weight_lut_interval); 
			y2[1] = y0[1] - a0 * y1[1]; y3[1] = y2[1] * y2[1]; acc[1] += (y3[1] * data_weight[j[1]]);
			x0[2] = *x++; y0[2] = *y++; y1[2] = errf_ext(x0[2],s); j[2] = (int)(y1[2] * inverse_weight_lut_interval); 
			y2[2] = y0[2] - a0 * y1[2]; y3[2] = y2[2] * y2[2]; acc[2] += (y3[2] * data_weight[j[2]]);
			x0[3] = *x++; y0[3] = *y++; y1[3] = errf_ext(x0[3],s); j[3] = (int)(y1[3] * inverse_weight_lut_interval); 
			y2[3] = y0[3] - a0 * y1[3]; y3[3] = y2[3] * y2[3]; acc[3] += (y3[3] * data_weight[j[3]]);
		}
		chi2 = (acc[0] + acc[1] + acc[2] + acc[3]) / a0;
	} else {
	/* user defined weights */
		for(k=0;k<m;++k) {
			x0[0] = *x++; y0[0] = *y++; y1[0] = errf_ext(x0[0],s);
			y2[0] = y0[0] - a0 * y1[0]; y3[0] = y2[0] * y2[0] * (*w++); acc[0] += y3[0]; 
		}
		m = n / 4;
		for(k=0;k<m;++k) {
			x0[0] = *x++; y0[0] = *y++; y1[0] = errf_ext(x0[0],s);
			y2[0] = y0[0] - a0 * y1[0]; y3[0] = y2[0] * y2[0] * (*w++); acc[0] += y3[0]; 
			x0[1] = *x++; y0[1] = *y++; y1[1] = errf_ext(x0[1],s);
			y2[1] = y0[1] - a0 * y1[1]; y3[1] = y2[1] * y2[1] * (*w++); acc[1] += y3[1];
			x0[2] = *x++; y0[2] = *y++; y1[2] = errf_ext(x0[2],s);
			y2[2] = y0[2] - a0 * y1[2]; y3[2] = y2[2] * y2[2] * (*w++); acc[2] += y3[2];
			x0[3] = *x++; y0[3] = *y++; y1[3] = errf_ext(x0[3],s);
			y2[3] = y0[3] - a0 * y1[3]; y3[3] = y2[3] * y2[3] * (*w++); acc[3] += y3[3];
		}
		chi2 = (acc[0] + acc[1] + acc[2] + acc[3]);
	}
	return chi2;
}

/* important note:
 * declaring fitWeights statically implies this is no longer multi-thread safe
 * to make it multi-thread safe, one must allocate the memory before calling fitSCurve()
 * and free it afterwards.
 * Under normal circumstances, this will not be an issue
 */
static double fitWeights[MAXBINS];

/* the user is allowed to specify the weights used in chiSquared calculations.
	There exists a philosophical debate whether weights should be calculated from
	data, or from the expected values.
	Calculation from the data leads to ambiguities in the interpretation of the
	standard deviation, but is the accepted norm.
	Calculation from the expected value may be meaningful only in the case of
	an excellent fit, where the data and expected values are close enough that
	the difference in weights are insignificant.
 */
double *calculateFitWeights(FitData *pfd) {
	double *y, plateau;
	int k, n;
	y = pfd->y;
	n = pfd->n;
	plateau = y[n-1]; /* jsv. assume last value == plateau. subject to failure? */
	if(plateau < 0.001f) { /* dont want a zero plateau */
		return (double *)0;
	}
#if 0
/* chi2 weights are calculated from the data. a burning issue is how to treat
	zero data. */
	double lowCutOff = 0.999f; /* just less than 1.0f for good measure */
	double maxWeight;
	maxWeight = plateau / (plateau - 1.0f); /* treat zero data the same as one */
	for(k=0;k<n;++k) {
		data = *y++;
		if(data < lowCutOff) {
			fitWeights[k] = maxWeight;
		} else if(abs_ext(data-plateau) < lowCutOff) {
			fitWeights[k] = maxWeight;
		} else {
			fitWeights[k] = plateau / (data * (plateau - data));
		}
	}
#else
/* no weights */
	for(k=0;k<n;++k) 
		fitWeights[k] = 1.0f;
#endif
	return fitWeights;
}

int defaultFit(Fit *pFit) {
	return defaultFitLikelihood(pFit);
}

int defaultFitLikelihood(Fit *pFit) {
  pFit->npars = 2;
  pFit->deltaPar[1] = 0.1f;
  pFit->deltaPar[0] = 0.1f;
  pFit->maxIters = 100;
  pFit->epsilonPar[0] = 0.0001f; /* 0.01% */
  pFit->epsilonPar[1] = 0.0001f; /* 0.01% */
  pFit->chiSquared = chiSquared;
  pFit->fxn = logLikelihood;
  pFit->fitSCurve = fitSCurveAim;
  pFit->parGuess = sGuess;
  /* jsv required ??? */
  pFit->manualSeekMu.dt = 0.05f;
  pFit->manualSeekMu.tBegin = 0.0f;
  pFit->manualSeekMu.tFinal = 2.0f;
  return 0;
}

int defaultFitLikelihoodToT(Fit *pFit) {
  pFit->npars = 3;
  pFit->deltaPar[2] = 0.1f;
  pFit->deltaPar[1] = 0.1f;
  pFit->deltaPar[0] = 0.1f;
  pFit->maxIters = 300;
  pFit->epsilonPar[0] = 0.001f; /* 0.1% */
  pFit->epsilonPar[1] = 0.001f; /* 0.1% */
  pFit->epsilonPar[2] = 0.001f; /* 0.1% */
  pFit->chiSquared = chiSquaredToT;
  pFit->fxn = logLikelihoodToT;
  pFit->fitSCurve = fitSCurveAim;
  pFit->parGuess = totGuess;
  pFit->manualSeekMu.dt = 0.05f;
  pFit->manualSeekMu.tBegin = 0.0f;
  pFit->manualSeekMu.tFinal = 2.0f;
  return 0;
}

int defaultFitChi2(Fit *pFit) {
  pFit->npars = 2;
  pFit->deltaPar[1] = 0.01f;
  pFit->deltaPar[0] = 0.01f;
  pFit->maxIters = 100;
  /* jsv. important. epsilons must be smaller than deltas because of
     the "push" dampening of nextAbscissa()!!! */
  pFit->epsilonPar[0] = 0.001f;
  pFit->epsilonPar[1] = 0.001f;
  pFit->extraIter = 0;
  pFit->concavitySmallness = 0.005f;
  pFit->findMinSigma = manualFindMinSigma;
  pFit->findMinMu = manualFindMinMu;
  pFit->minPar[0] = 0.f;
  pFit->minPar[1] = 3.001f;
  pFit->manualSeekMu.dt = 0.25f;
  pFit->manualSeekMu.tBegin = 2.0f;
  pFit->manualSeekMu.tFinal = 2.0f;
  pFit->manualSeekSigma.dt = 0.25f;
  pFit->manualSeekSigma.tBegin = 0.25f;
  pFit->manualSeekSigma.tFinal = 4.0f;
  pFit->chiSquared = chiSquared;
  pFit->fxn = chiSquared;
  pFit->fitSCurve = fitSCurveParabolicAlgorithm;
  pFit->parGuess = sGuess;
  pFit->chi2CutOff = 10000.0;
  pFit->cutOffPar[0] = 15000.0;
  pFit->cutOffPar[1] = 7500.0;
  return 0;
}

