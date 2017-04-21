#include <vfitXface.h>
#include <fittingRoutines.h>
#include <math.h>
#include <stdio.h>

typedef struct {
	void *fitWorkBase, *workPtr;
	int nBins;
	float *xVal, *yVal;
	float *fitWeights;
} FitWork;

static FitWork fitWork;

/* Local function definitions: */
int fitPixel(float *x,float *y,int n, int *nIters, float *coeffs, int flags, float *chi2);
float *calculateFitWeights(FitData *pfd);
float logLikelihoodTot(FitData *pfd, FitPars *s);
float logLikelihoodTotNew(FitData *pfd, FitPars *s);
float logLikelihoodTotLinear(FitData *pfd, FitPars *s);
float chiSquaredTot(FitData *pfd,FitPars *s);
float chiSquaredTotNew(FitData *pfd,FitPars *s);
float chiSquaredTotLinear(FitData *pfd,FitPars *s);

static float invRoot2 = 0.7071067811865475244f;

/*! sets up fitting for a maximization of likelihood */
int defaultFitLikelihood(Fit *pFit) {
	pFit->deltaSigma = 0.1f;
	pFit->deltaMu = 0.1f;
	pFit->maxIters = 500;
	pFit->muEpsilon = 0.0001f; /* 0.01% */
	pFit->sigmaEpsilon = 0.0001f; /* 0.01% */
	pFit->chiSquared = logLikelihood;
	pFit->fitSCurve = fitSCurveCrawl;//Aim;
	pFit->sGuess = sGuess;
	pFit->findMinSigma = manualFindMinSigma;
	pFit->findMinMu = manualFindMinMu;
	pFit->manualSeekMu.dt = 0.05f;
	pFit->manualSeekMu.tBegin = 0.0f;
	pFit->manualSeekMu.tFinal = 2.0f;
	return 0;
}


int defaultFitLikelihoodTot(FitCtrl *pFit) {
  pFit->npars = 3;
  pFit->deltaPar[2] = 0.1f;
  pFit->deltaPar[1] = 0.1f;
  pFit->deltaPar[0] = 0.1f;
  pFit->maxIters = 300;
  pFit->epsilonPar[0] = 0.001f; /* 0.1% */
  pFit->epsilonPar[1] = 0.001f; /* 0.1% */
  pFit->epsilonPar[2] = 0.001f; /* 0.1% */
  pFit->chiSquared = chiSquaredTot;
  pFit->fxn = logLikelihoodTot;
  pFit->fitSCurve = fitSCurveAimTot;
  pFit->parGuess = totGuess;
  pFit->manualSeekMu.dt = 0.05f;
  pFit->manualSeekMu.tBegin = 0.0f;
  pFit->manualSeekMu.tFinal = 2.0f;
  return 0;
}

/*! Uses new TOT fit function */
int defaultFitLikelihoodTotNew(FitCtrl *pFit) {
  pFit->npars = 3;
  pFit->deltaPar[2] = 0.1f;
  pFit->deltaPar[1] = 0.1f;
  pFit->deltaPar[0] = 0.1f;
  pFit->maxIters = 300;
  pFit->epsilonPar[0] = 0.001f; /* 0.1% */
  pFit->epsilonPar[1] = 0.001f; /* 0.1% */
  pFit->epsilonPar[2] = 0.001f; /* 0.1% */
  pFit->chiSquared = chiSquaredTotNew;
  pFit->fxn = logLikelihoodTotNew;
  pFit->fitSCurve = fitSCurveAimTot;
  pFit->parGuess = totGuessNew;
  pFit->manualSeekMu.dt = 0.05f;
  pFit->manualSeekMu.tBegin = 0.0f;
  pFit->manualSeekMu.tFinal = 2.0f;
  return 0;
}

/*!sets up fit to do linear TOT fit - hg debug */
int defaultFitLikelihoodTotLinear(FitCtrl *pFit) {
  pFit->npars = 2; 
  pFit->deltaPar[2] = 0.1f;
  pFit->deltaPar[1] = 0.1f;
  pFit->deltaPar[0] = 0.1f;
  pFit->maxIters = 300;
  pFit->epsilonPar[0] = 0.001f; /* 0.1% */
  pFit->epsilonPar[1] = 0.001f; /* 0.1% */
  pFit->epsilonPar[2] = 0.001f; /* 0.1% */
  pFit->chiSquared = chiSquaredTotLinear;
  pFit->fxn = logLikelihoodTotLinear;
  pFit->fitSCurve = fitSCurveAimTot;
  pFit->parGuess = totGuessLinear;
  pFit->manualSeekMu.dt = 0.05f;
  pFit->manualSeekMu.tBegin = 0.0f;
  pFit->manualSeekMu.tFinal = 2.0f;
  return 0;
}
/*! sets up fitting for a minimization of chi^2 */
int defaultFitChi2(Fit *pFit) {
	pFit->deltaSigma = 0.01f;
	pFit->deltaMu = 0.01f;
	pFit->maxIters = 200;
/* jsv. important. epsilons must be smaller than deltas because of
	the "push" dampening of nextAbscissa()!!! */
	pFit->muEpsilon = pFit->deltaMu * 0.99f;
	pFit->sigmaEpsilon = pFit->deltaSigma * 0.99f;
	pFit->muEpsilon = 0.0001f;
	pFit->sigmaEpsilon = 0.0001f;
	pFit->extraIter = 0;
	pFit->concavitySmallness = 0.005f;
	pFit->findMinSigma = manualFindMinSigma;
	pFit->findMinMu = manualFindMinMu;
	pFit->sigmaMin = 3.001f;
	pFit->manualSeekMu.dt = 0.25f;
	pFit->manualSeekMu.tBegin = 2.0f;
	pFit->manualSeekMu.tFinal = 2.0f;
	pFit->manualSeekSigma.dt = 0.25f;
	pFit->manualSeekSigma.tBegin = 0.25f;
	pFit->manualSeekSigma.tFinal = 4.0f;
	pFit->chiSquared = chiSquared;
	//pFit->chiSquared = logLikelihood;
	pFit->fitSCurve = fitSCurveParabolicAlgorithm;
	pFit->sGuess = sGuess;
	pFit->chi2CutOff = 10000.0;
	pFit->muCutOff = 15000.0;
	pFit->sigmaCutOff = 7500.0;
	return 0;
}

/*! for those who don't want to think about it */
int defaultFit(Fit *pFit) {
	return defaultFitLikelihood(pFit);
}

/*!
 * if coeffs[0] != 0 => initial guess is assumed, not made by routine \n
 * if coeffs[0] == 0 => initial guess is made by routine \n
 */

/************************************************************************************
 *
 ************************************************************************************/
int fitSCurve(float *x, float *y, float *yerr, int n, int *nIters, float *coeffs, int flags, float *chi2) {
	GaussianCurve s;
	FitData rawData, extData, *pfd;
	Fit fit;
	int status;
	if(flags & VFX_MAX_LIKELIHOOD) {
		defaultFitLikelihood(&fit);
	} else {
		defaultFitChi2(&fit);
	}
	if(*nIters > 0) fit.maxIters = *nIters;
	fit.curve = &s;
	s.a0 = (float)coeffs[0];
/*
	s.mu = (float)coeffs[1];
	setSigma(&s,(float)coeffs[2]);
 */
	rawData.x = x; rawData.y = y; rawData.ye = yerr; rawData.n = n;
	
/* extract non-trivial data for fitting */
	status = extractGoodData(&rawData, &extData) ? 0 : 1;
	pfd = &extData;
	if(status) {return status;}
// 	if(s.a0 != 0.0f && (fabs(s.a0-pfd->y[pfd->n-1]) > 0.001f)) {
// 		return 1; /* assume plateau = last value. jsv possibly better method? */
// 	}
	s.a0 = (float)coeffs[0];
	if(flags & VFX_USER_GUESS) fit.sGuess = 0; /* actually s.a0 != 0.0 is a good comparison */
	if(flags & VFX_USER_WEIGHTS) {
		pfd->w = calculateFitWeights(&extData);
	} else {
		pfd->w = (float *)0;
	}
	status = fit.fitSCurve(pfd,&fit);
	coeffs[0] = s.a0;
	coeffs[1] = s.mu;
	coeffs[2] = s.sigma * invRoot2;
	*chi2 = fit.chi2;
	*nIters = fit.nIters;
	return status;
}

int fitTotCalOld(float *x, float *y, float *yerr, int n, TotCal *calpar, int flags, float *chi2, int *nIters){
  int status = 0;
  FitPars s;
  FitCtrl fit;
  FitData rawData;
  if(!(flags & VFX_MAX_LIKELIHOOD)) {return -1;} // can only do max LH

  rawData.x = x; rawData.y = y; rawData.ye = yerr; rawData.n = n;

  defaultFitLikelihoodTot(&fit);

  s.par[0] = calpar->cstpar;
  s.par[1] = calpar->numpar;
  s.par[2] = calpar->denompar;
  fit.curve = &s;
  status = fit.fitSCurve(&rawData,&fit);
  calpar->cstpar   = s.par[0];
  calpar->numpar   = s.par[1];
  calpar->denompar = s.par[2];
  *chi2 = fit.chi2;
  *nIters = fit.nIters;

  return status;
}

int fitTotCalNew(float *x, float *y, float *yerr, int n, TotCal *calpar, int flags, float *chi2, int *nIters){
  int status = 0;
  FitPars s;
  FitCtrl fit;
  FitData rawData;
  if(!(flags & VFX_MAX_LIKELIHOOD)) {return -1;} // can only do max LH

  rawData.x = x; rawData.y = y; rawData.ye = yerr; rawData.n = n;

  defaultFitLikelihoodTotNew(&fit);

  s.par[0] = calpar->cstpar;
  s.par[1] = calpar->numpar;
  s.par[2] = calpar->denompar;
  fit.curve = &s;
  status = fit.fitSCurve(&rawData,&fit);
  calpar->cstpar   = s.par[0];
  calpar->numpar   = s.par[1];
  calpar->denompar = s.par[2];
  *chi2 = fit.chi2;
  *nIters = fit.nIters;

  return status;
}

/************************************************************************************
 *
 ************************************************************************************/
/*! uses math function log */
float logLikelihood(FitData *pfd, GaussianCurve *s) {
	float acc, r, p, y2, y3, N;
	float *x, *y, cutOff;
	int k, n;
	x = pfd->x;	y = pfd->y;	n = pfd->n;
	acc = 0.0f;
	N = s->a0;
	cutOff = 1.0e-6f; // jsv
	for(k=0;k<n;++k) {
		r = *y++; /* data point */
		p = errf_ext(*x++,s); /* probability */
		if(p == 0.0f) p =       cutOff; /* jsv */
		if(p == 1.0f) p = 1.0f - cutOff;
		y2 = r * log(p);
		y3 = (N - r) * log(1.0f - p);
		acc -= (y2 + y3);
	}
	return acc;
}
/*! uses lookup table for log */
// float logLikelihood(FitData *pfd, GaussianCurve *s) {
// 	float acc, r, y2, y3, N;
// 	float *x, *y, *p;
// 	int k, n;
// 	x = pfd->x;	y = pfd->y;	n = pfd->n;
// 	acc = 0.0f;
// 	N = s->a0;
// 	for(k=0;k<n;++k) {
// 		r = *y++; /* data point */
// 		p = log_ext(*x++,s); /* pointer to log(probability) */
// 		y2 = r * (*p++); /* log(prob) */
// 		y3 = (N - r) * (*p); /* log(1-prob) */
// 		acc -= (y2 + y3);
// 	}
// 	return acc;
// }
/*! uses errors provided by user */
// float logLikelihood(FitData *pfd, GaussianCurve *s) {
//   float acc, r, y2, y3, sig, N;
//   float *x, *y, *ye, *p;
//   int k, n;
//   int haveErr;
//   x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
//   haveErr = (ye!=0);

//   acc = 0.0f;
//   N = s->a0;
//   for(k=0;k<n;++k) {
//     y3 = *y++; /* data point */
//     if(haveErr)
//       sig = *ye++; // error
//     else
//       sig = y3/10;
//     y2 = N*errf_ext(*x++,s); /* probability */
//     if(fabs(sig)>=1e-3*y3 && sig!=0){ // ignore rubbish sigma values
// 		y2 = fabs((y2-y3)/sig);
//     	acc -= log_e(y2);
//     }
//   }
//   return acc;
// }


float logLikelihoodTot(FitData *pfd, FitPars *s) {
  float acc, y2, y3, sig;
  float *x, *y, *ye;
  int k, n;
 int haveErr;
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
    if(fabs(sig)>=1e-3*y3 && sig!=0){ // ignore rubbish sigma values
		y2 = fabs((y2-y3)/sig);
    	acc -= log_e(y2);
    }
  }
  return acc;
}

float logLikelihoodTotNew(FitData *pfd, FitPars *s) {
  float acc, y2, y3, sig;
  float *x, *y, *ye;
  int k, n;
 int haveErr;
  x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
  haveErr = (ye!=0);

  acc = 0.0f;
  for(k=0;k<n;++k) {
    y3 = *y++; /* data point */
    if(haveErr)
      sig = *ye++; // error
    else
      sig = y3/10;
    y2 = s->par[0] * (*x + s->par[1])/(s->par[2]+(*x));
    (*x)+=1.;
    if(fabs(sig)>=1e-3*y3 && sig!=0){ // ignore rubbish sigma values
		y2 = fabs((y2-y3)/sig);
    	acc += log_e(y2); // hg tbd changed to + for consistency in fitting routine
    }
  }
  return acc;
}


float logLikelihoodTotLinear(FitData *pfd, FitPars *s) {
  float acc, y2, y3, sig;
  float *x, *y, *ye;
  int k, n;
 int haveErr;
  x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
  haveErr = (ye!=0);

  acc = 0.0f;
  for(k=0;k<n;++k) {
    y3 = *y++; /* data point */
    if(haveErr)
      sig = *ye++; // error
    else
      sig = y3/10;
    y2 = s->par[0] + (*x++ * s->par[1]);
    if(fabs(sig)>=1e-3*y3 && sig!=0){ // ignore rubbish sigma values
		y2 = fabs((y2-y3)/sig);
    	acc -= log_e(y2);
    }
  }
  return acc;
}

float chiSquared(FitData *pfd,GaussianCurve *s) {
	float acc[4], y0[4], y1[4], x0[4], y2[4], y3[4], a0, chi2;
	float *x, *y, *w;
	int j[4], k, m, n;
	x = pfd->x;	y = pfd->y;	n = pfd->n;	w = pfd->w;
	acc[0] = 0.0f; acc[1] = 0.0f; acc[2] = 0.0f; acc[3] = 0.0f;
	m = n % 4;
	a0 = s->a0;
	if(w == (float *)0) {
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

float chiSquaredTot(FitData *pfd,FitPars *s) {
  float *x, *y, *ye, acc, y2, y3, sig;
  int k, n;
  int haveErr;
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

float chiSquaredTotNew(FitData *pfd,FitPars *s) {
  float *x, *y, *ye, acc, y2, y3, sig;
  int k, n;
  int haveErr;
  x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
 // haveErr = (ye!=0);
	haveErr = 0;

  acc = 0.0f;
  for(k=0;k<n;++k) {
    y3 = *y++; /* data point */
    if(haveErr)
      sig = *ye++; // error
    else
      sig = y3/10;
    y2 = s->par[0]*(s->par[1] + *x)/(s->par[2]+(*x));
    (*x)+=1.;
    if(fabs(sig)>=1e-3*y3 && sig!=0) // ignore rubbish sigma values
      acc += (y2-y3)*(y2-y3)/sig/sig;
  }
  return acc;
}

float chiSquaredTotLinear(FitData *pfd,FitPars *s) {
  float *x, *y, *ye, acc, y2, y3, sig;
  int k, n;
  int haveErr;
  x = pfd->x;	y = pfd->y;	n = pfd->n;  ye = pfd->ye;
 // haveErr = (ye!=0);
	haveErr = 0;

  acc = 0.0f;
  for(k=0;k<n;++k) {
    y3 = *y++; /* data point */
    if(haveErr)
      sig = *ye++; // error
    else
      sig = y3/10;
    y2 = s->par[0] + (s->par[1] + *x++);
    if(fabs(sig)>=1e-3*y3 && sig!=0) // ignore rubbish sigma values
      acc += (y2-y3)*(y2-y3)/sig/sig;
  }
  return acc;
}
/*! the user is allowed to specify the weights used in chiSquared calculations.
	There exists a philosophical debate whether weights should be calculated from
	data, or from the expected values.
	Calculation from the data leads to ambiguities in the interpretation of the
	standard deviation, but is the accepted norm.
	Calculation from the expected value may be meaningful only in the case of
	an excellent fit, where the data and expected values are close enough that
	the difference in weights are insignificant.
 */
float *calculateFitWeights(FitData *pfd) {
	float *y, plateau, *fitWeights;
//	float lowCutOff = 0.999f; /* just less than 1.0f for good measure */
//	float maxWeight;
	int k, n;
	fitWeights = fitWork.fitWeights;
	y = pfd->y;
	n = pfd->n;
	plateau = y[n-1]; /* jsv. assume last value == plateau. subject to failure? */
	if(plateau < 0.001f) { /* dont want a zero plateau */
		return (float *)0;
	}
#if 0
/* chi2 weights are calculated from the data. a burning issue is how to treat
	zero data. */
	maxWeight = plateau / (plateau - 1.0f); /* treat zero data the same as one */
	for(k=0;k<n;++k) {
		data = *y++;
		if(data < lowCutOff) {
			fitWeights[k] = maxWeight;
		} else if(fabs(data-plateau) < lowCutOff) {
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

