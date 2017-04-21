#include "fittingRoutines.h"
#include <math.h>
#include <stdio.h>

int fitSCurveCrawl(FitData *pfd, void *vpfit) {
	double deltaSigma, deltaMu;
	double mu_old, mu_new;
	double sigma_old, sigma_new;
	FitPars sParams, sParamsWork, *psp;
	Fit *pFit = (Fit *)vpfit;

	Point point[3];
	int status;
	
	pFit->converge = 0; /* assume no convergence */

	pFit->ndf = pfd->n - 1; /* number of degrees of freedom */

/* take a guess at the S-curve parameters */
	if(pFit->parGuess) {
		status = pFit->parGuess(pfd,&sParams,pFit);
		if(status) {
		/* if too sharp a rise, not much more to do */
			pFit->convergePar[0] = 0; /* can't claim convergence */
			pFit->convergePar[1] = 0; /* can't claim convergence */
			pFit->converge = status; /* technically speaking, this is ok */
			pFit->nIters = 0;

			sParams.par[0] = pFit->findMinMu(pfd,&sParams,pFit);
			sParams.par[1] = pFit->findMinSigma(pfd,&sParams,pFit);

			psp = (FitPars *)pFit->curve;
			*psp = sParams;

			return 0;
		}
	}

/* initialize loop parameters */
	psp = &sParamsWork;
	psp->par[2] = sParams.par[2];
	mu_old = sParams.par[0];
	mu_new = mu_old;
	sigma_old = sParams.par[1];
	sigma_new = sigma_old;
	deltaSigma = pFit->deltaPar[1];
	deltaMu = pFit->deltaPar[0];
	pFit->nIters = 0;

	while(pFit->nIters++ < pFit->maxIters) {
	/* hold mu constant. find sigma */
		psp->par[0] = mu_old;

		setSigma(psp,sigma_old);
		point[0].x = psp->par[1];
		point[0].y = pFit->fxn(pfd,psp);

		setSigma(psp,sigma_old * (1.0f + deltaSigma));
		point[1].x = psp->par[1];
		point[1].y = pFit->fxn(pfd,psp);

		setSigma(psp,sigma_old * (1.0f - deltaSigma));
		point[2].x = psp->par[1];
		point[2].y = pFit->fxn(pfd,psp);

		sigma_new = nextAbscissa(point);

	/* hold new sigma constant. find new mu. */
		setSigma(psp,sigma_new);
	/* psp->mu = mu_old; valid line but unnecessary */
		point[0].x = psp->par[0];
		point[0].y = pFit->fxn(pfd,psp);

		psp->par[0] = (1.0f + deltaMu) * mu_old;
		point[1].x = psp->par[0];
		point[1].y = pFit->fxn(pfd,psp);

		psp->par[0] = (1.0f - deltaMu) * mu_old;
		point[2].x = psp->par[0];
		point[2].y = pFit->fxn(pfd,psp);

		mu_new = nextAbscissa(point);

	/* if converge, break */
		if((abs_ext(mu_new - mu_old) < pFit->epsilonPar[0] * mu_old) &&
			(abs_ext(sigma_new - sigma_old) < pFit->epsilonPar[1] * sigma_old)) {
			pFit->convergePar[0] = pFit->convergePar[1] = pFit->converge = 1;
			break;
		} else {
			mu_old = mu_new; sigma_old = sigma_new;
		}
	}

	psp = (FitPars *)pFit->curve;
	psp->par[2] = sParams.par[2];
	psp->par[0] = mu_new;
	setSigma(psp,sigma_new);
	pFit->chi2 = pFit->fxn(pfd,psp) / (double)pFit->ndf;

// jsv not implemented yet pFit->nIters -= pFit->extraIter; /* adjust for extra iterations */
	return pFit->converge ? 0 : 1; /* 0 = success */
}

double parabolicAbscissa(Point *pt) {
	double deltaX, denom, push;
	Point *p0, *p1, *p2;
	p0 = pt;	p1 = &pt[1]; p2 = &pt[2];
	deltaX = p0->x - p2->x;
	denom = 2.0f * p0->y - p1->y - p2->y; /* also is the negative of concavity */
	if(denom == 0.0f) return 0.0f; /* 0 is not a number we like */
	push = 0.5f * deltaX * (p1->y - p2->y) / denom;
	return p0->x + push;
}

/* jsv outstanding issue in nextAbscissa(). 
	should a push by deltaX be reduced to prevent oscillations? */
double nextAbscissa(Point *pt) {
	double deltaX, denom, push, rhoTiny, rhoSmall, rhoLargeDerivative;
//	double concavity;
	Point *p0, *p1, *p2;
	p0 = pt;	p1 = &pt[1]; p2 = &pt[2];
	deltaX = p0->x - p2->x;
/* ensure denominator is not zero */
	denom = 2.0f * p0->y - p1->y - p2->y; /* also is the negative of concavity */
//	concavity = - denom;
/* jsv. this is voodoo. if the concavity is negative, it means the extremum would
	be a maximum. We are looking for a minimum, so we must "push" in the other
	direction; the direction that would give a smaller value of y */
	if(denom >= 0.0f) {
		if(p1->y >= p2->y) return p2->x - deltaX;
		else return p1->x + deltaX;
	}
	push = 0.5f * deltaX * (p1->y - p2->y) / denom;
	rhoTiny = 0.001f; // jsv replace with fit.concavitySmallness */
	if(abs_ext(denom) < (rhoTiny * p0->y)) {
	/* the concativity is relatively small. dampen the push */
		if(push >= 0.0f) return p0->x + deltaX;
		else return p0->x - deltaX;
	}

	rhoSmall = 0.01f; // jsv
	rhoLargeDerivative = 0.10f; // jsv
	if((abs_ext(denom) < rhoSmall * p0->y) && 
		(abs_ext((p1->y - p2->y)/p0->y) > rhoLargeDerivative)){
		if(p1->y > p2->y) return p2->x - deltaX;
		else return p1->x + deltaX;
	}
	return p0->x + push;
}

/* manually finds mu for minimizing chi2 by scanning the region between
	mu_guess - tBegin * dmu through mu_guess + tFinal * dmu
	dmu = x[1] - x[0] = spacing between x coordinates */

double manualFindMinMu(FitData *pfd, FitPars *ps, void *vpFit) {
	double minmu, dmu, muFinal, chi2, minChi2;
	FitPars sParamsWork, *psw;
	Fit *pFit = (Fit *)vpFit;
	psw = &sParamsWork;

	dmu = pfd->x[1] - pfd->x[0];

/* setup scan parameters */
	psw->par[0] = ps->par[0] - pFit->manualSeekMu.tBegin * dmu;
	muFinal = ps->par[0] + pFit->manualSeekMu.tFinal * dmu;
	psw->par[2] = ps->par[2];
	setSigma(psw,ps->par[1]);

/* the first point is taken to be the current minimum */
	minChi2 = pFit->fxn(pfd,psw);
	minmu = psw->par[0];

	dmu = dmu * pFit->manualSeekMu.dt;
	while(psw->par[0] < muFinal) {
		psw->par[0] += dmu; /* next mu */
		chi2 = pFit->fxn(pfd,psw);
		if(chi2 <= minChi2) {
			minChi2 = chi2;
			minmu = psw->par[0];
		}
	}
	pFit->chi2 = minChi2 / (double)pFit->ndf;
	return minmu;
}

/* manually finds sigma for minimizing chi2 by scanning the region between
	sigma_guess - tBegin * dsigma through sigma_guess + tFinal * dsigma
	dsigma = x[1] - x[0] = spacing between x coordinates */


double manualFindMinSigma(FitData *pfd, FitPars *ps, void *vpFit) {
	double minSigma, dSigma, endSigma, chi2, minChi2, sigma;
	FitPars sParamsWork, *psw;
	Fit *pFit = (Fit *)vpFit;
	psw = &sParamsWork;

/* setup scan parameters */
	dSigma = pFit->manualSeekSigma.dt * (pfd->x[1] - pfd->x[0]); /* rescaled. jsv pass as argument? */
	endSigma = pFit->manualSeekSigma.tFinal * ps->par[1];
	sigma = pFit->manualSeekSigma.tBegin * ps->par[1];
	setSigma(psw,sigma); /* beginning value */
	psw->par[2] = ps->par[2];
	psw->par[0] = ps->par[0];

/* the first value is taken to be the current minimum. reasonable */
	minChi2 = pFit->fxn(pfd,psw);
	minSigma = psw->par[1];

	while(sigma < endSigma) {
		sigma += dSigma; /* next sigma */
		setSigma(psw,sigma);
		chi2 = pFit->fxn(pfd,psw);
		if(chi2 <= minChi2) {
			minChi2 = chi2;
			minSigma = sigma;
		}
	}
	pFit->chi2 = minChi2 / (double)pFit->ndf;
	return minSigma;
}

int fitSCurveAim(FitData *pfd, void *vpfit) {
  double deltaPar[NPMAX];
  //  double *fptr, chi2Min, chi2[9];
  double chi2old, chi2new, delFact[NPMAX], delFactOpt[NPMAX];
  FitPars sParams, sParamsWork, *psp;
  int k, l, ind, nvar, dirMin;
  Fit *pFit = (Fit *)vpfit;
  
  pFit->nIters = 0;
  pFit->converge = 0; /* assume no convergence */
  
  pFit->ndf = pfd->n - pFit->npars; /* degrees of freedom */
  
  sParams = *((FitPars *)pFit->curve);
  /* take a guess at the S-curve parameters */
  if(pFit->parGuess) {
    if(pFit->parGuess(pfd,&sParams,pFit)) {
      //psp = (FitPars *)pFit->curve;
      pFit->converge = 1;
      pFit->convergePar[0] = 0;
      pFit->convergePar[1] = 0;
      pFit->chi2 = 0.0f;
      return 0;
    }
  }
  
  /* initialize loop parameters */
  psp = &sParamsWork;
  for(k=0; k<pFit->npars; k++)
    deltaPar[k] = pFit->deltaPar[k] * sParams.par[k]; /* scaled */

  /** the loop begins **/
  
  while(!pFit->converge && (pFit->nIters++ < pFit->maxIters)) {
    
    dirMin = 0;
    
    while((dirMin >= 0) && (pFit->nIters++ < pFit->maxIters)) {
      
      /** calculate neighboring points **/
      chi2old = pFit->fxn(pfd,&sParams); // chi2 from unchanged pars
      dirMin = -1;
      // then vary par's by +-deltaPars and see if chi2 gets smaller
      nvar = (int)pow(3,(double)pFit->npars);
      for(k=0;k<pFit->npars; k++){ // initialise del factors
	delFact[k] = -1;
	delFactOpt[k] = 0;
      }
      for(l=0;l<nvar; l++){ // loop over (+1/0/-1)*delta-possibilities
	for(k=0;k<pFit->npars; k++){ // loop over pars
	  ind = (int)pow(3,(double)k);
	  if(l==0)
	    ind = l+1;
	  else
	    ind = (l/ind)*ind;
	  if(ind==l) delFact[k]+=1;
	  if(delFact[k]>1) delFact[k]=-1;
	  psp->par[k] = sParams.par[k] + delFact[k]*deltaPar[k];
	}
	chi2new = pFit->fxn(pfd,psp);
	if(chi2new<chi2old){
	  dirMin = l;
	  for(k=0;k<pFit->npars; k++)
	    delFactOpt[k] = delFact[k];
	}
      }
      if(dirMin>=0){
	for(k=0;k<pFit->npars; k++){ // choose min. settings
	  sParams.par[k] = sParams.par[k] + delFactOpt[k]*deltaPar[k];
	}
      }else{
	for(k=0;k<pFit->npars; k++) // decrease step size
	  deltaPar[k] *= 0.1f;
      }
    }

    pFit->converge = 1;
    for(k=0;k<pFit->npars; k++) // check successful convergency
      if(deltaPar[k] > (pFit->epsilonPar[k] * sParams.par[k]))
	pFit->converge = 0;
  }

  psp = (FitPars *)pFit->curve;
  *psp = sParams;
  pFit->chi2 = pFit->chiSquared(pfd,psp) / (double)pFit->ndf;
  return pFit->converge ? 0 : 1; /* 0 = success */
}

/* jsv. are there potential oscillatory problems by constantly rescaling
	deltaSigma & deltaMu? 
	what about the number of degrees of freedom? */

int fitSCurveParabolicAlgorithm(FitData *pfd, void *vpfit) {
	double deltaSigma, deltaMu, x0, x1, y0, y1;
	double a0, a1, b0, b1, c0, c1, invDet;
	double mu_old, mu_new, chi2[4], chi2Temp, chi2CutOff, temp;
	double sigma_old, sigma_new;
	FitPars sParams, sParamsWork, *psp;
	Point point[3];
	Point pointSigma[2], pointMu[2];
	int extraIter, status;
	Fit *pFit = (Fit *)vpfit;

	pFit->converge = 0; /* assume no convergence */

	pFit->ndf = pfd->n - 1; /* degrees of freedom */

	chi2CutOff = pFit->chi2CutOff * (double)pFit->ndf;

/* take a guess at the S-curve parameters */
	if(pFit->parGuess) {
		status = pFit->parGuess(pfd,&sParams,pFit);
		if(status) {
			psp = (FitPars *)pFit->curve;
			psp->par[2] = sParams.par[2];
			psp->par[0] = sParams.par[0];
			setSigma(psp,pFit->minPar[1]);
			pFit->converge = 1;
			pFit->convergePar[0] = 0;
			pFit->convergePar[1] = 0;
			return status;
		}
	}

/* initialize loop parameters */
	psp = &sParamsWork;
	psp->par[2] = sParams.par[2];
	mu_old = sParams.par[0];
	sigma_old = sParams.par[1];
	deltaSigma = pFit->deltaPar[1];
	deltaMu = pFit->deltaPar[0];
	pFit->nIters = 0;
	extraIter = pFit->extraIter;

	while(pFit->nIters++ < pFit->maxIters) {

	/*** sigma ***/
		setSigma(psp,(1.0f - deltaSigma) * sParams.par[1]);

	/** calculate neighboring points **/
		point[1].x = psp->par[0] = (1.0f + deltaMu) * sParams.par[0];
		point[1].y = chi2[3] = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		point[0].x = psp->par[0] = sParams.par[0];
		point[0].y = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		point[2].x = psp->par[0] = (1.0f - deltaMu) * sParams.par[0];
		point[2].y = chi2[1] = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		pointSigma[0].x = nextAbscissa(point);
//		pointSigma[0].x = parabolicAbscissa(point);
		pointSigma[0].y = psp->par[1];

	/** **/

		setSigma(psp,(1.0f + deltaSigma) * sParams.par[1]);

	/** calculate neighboring points **/
		point[1].x = psp->par[0] = (1.0f + deltaMu) * sParams.par[0];
		point[1].y = chi2[2] = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		point[0].x = psp->par[0] = sParams.par[0];
		point[0].y = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		point[2].x = psp->par[0] = (1.0f - deltaMu) * sParams.par[0];
		point[2].y = chi2[0] = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		pointSigma[1].x = nextAbscissa(point);
//		pointSigma[1].x = parabolicAbscissa(point);
		pointSigma[1].y = psp->par[1];

	/* calculate the line between these two sigma points */

		x0 = pointSigma[0].x; x1 = pointSigma[1].x;
		y0 = pointSigma[0].y; y1 = pointSigma[1].y;

		a0 = y1-y0; b0 = x0-x1; c0=x0*y1-x1*y0;

	/*** mu ***/

		psp->par[0] = (1.0f - deltaMu) * sParams.par[0];

	/** calculate neighboring points **/
		setSigma(psp,(1.0f + deltaSigma) * sParams.par[1]);
		point[1].x = psp->par[1];
		point[1].y = chi2[0]; /* already calculated */

		setSigma(psp,sParams.par[1]);
		point[0].x = psp->par[1];
		point[0].y = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		setSigma(psp,(1.0f - deltaSigma) * sParams.par[1]);
		point[2].x = psp->par[1];
		point[2].y = chi2[1]; /* already calculated */

		pointMu[0].x = psp->par[0];
		pointMu[0].y = nextAbscissa(point);
//		pointMu[0].y = parabolicAbscissa(point);

	/** **/

		psp->par[0] = (1.0f + deltaMu) * sParams.par[0];

	/** calculate neighboring points **/
		setSigma(psp,(1.0f + deltaSigma) * sParams.par[1]);
		point[1].x = psp->par[1];
		point[1].y = chi2[2]; /* already calculated */

		setSigma(psp,sParams.par[1]);
		point[0].x = psp->par[1];
		point[0].y = chi2Temp = pFit->fxn(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 1; 
		}			

		setSigma(psp,(1.0f - deltaSigma) * sParams.par[1]);
		point[2].x = psp->par[1];
		point[2].y = chi2[3]; /* already calculated */

		pointMu[1].x = psp->par[0];
		pointMu[1].y = nextAbscissa(point);
//		pointMu[1].y = parabolicAbscissa(point);

	/* calculate the line between these mu points */

		x0 = pointMu[0].x; x1 = pointMu[1].x;
		y0 = pointMu[0].y; y1 = pointMu[1].y;

		a1 = y1-y0; b1 = x0-x1; c1 = x0*y1-x1*y0;

	/* where do the lines intersect */

		temp = a0 * b1 - b0 * a1;
		if(temp == 0.0f) {
			return 1;
		}
		invDet = 1.0f / temp; /* not singular */
		mu_new = ( b1 * c0 - b0 * c1 ) * invDet;
		if(mu_new > pFit->cutOffPar[0]) {
			return 1;
		}
		sigma_new = ( a0 * c1 - a1 * c0 ) * invDet;
		if(sigma_new > pFit->cutOffPar[1]) {
			return 1;
		}

	/* if converge, break */
		if(abs_ext(mu_new - mu_old) < pFit->epsilonPar[0] * mu_old) {
			// jsv deltaMu = deltaMu / 2;
			pFit->convergePar[0] = 1;
		} else {
//			if(pFit->convergePar[0])
//				printf("mu deconverged %f %f\n", mu_old, sigma_old); // jsv
			pFit->convergePar[0] = 0; /* it is possible to deconverge */
			extraIter = pFit->extraIter;
		}
		
		if(abs_ext(sigma_new - sigma_old) < pFit->epsilonPar[1] * sigma_old) {
			// jsv deltaSigma = deltaSigma / 2;
			pFit->convergePar[1] = 1;
		} else {
//			if(pFit->convergePar[1])
//				printf("sigma deconverged %f %f\n", mu_old, sigma_old); // jsv
			pFit->convergePar[1] = 0; /* it is possible to deconverge */
			extraIter = pFit->extraIter;
		}

		if(pFit->convergePar[0] && pFit->convergePar[1]) {
			if(!extraIter) {
				pFit->converge = 1; 
				break;
			} else {
				--extraIter;
			}
		}

		sParams.par[0] = mu_old = mu_new;
		sigma_old = sigma_new;
		setSigma(&sParams,sigma_new);

	}

/* manually verify sigma */
	pFit->manualSeekSigma.dt = 0.5;
	pFit->manualSeekSigma.tBegin = 0.5;
	pFit->manualSeekSigma.tFinal = 1.5;
	sigma_new = pFit->findMinSigma(pfd,&sParams,pFit);
	setSigma(&sParams,sigma_new);
	mu_new = pFit->findMinMu(pfd,&sParams,pFit);
	sParams.par[0] = mu_new;

	psp = (FitPars *)pFit->curve;
	psp->par[2] = sParams.par[2];
	psp->par[0] = mu_new;
	setSigma(psp,sigma_new);
	pFit->chi2 = pFit->fxn(pfd,psp) / (double)pFit->ndf;
	pFit->nIters -= pFit->extraIter; /* adjust for extra iteration */
	return pFit->converge ? 0 : 1; /* 0 = success */
}
	
int fitSCurveBruteForce(FitData *pfd, void *vpfit) {
	double deltaSigma, deltaMu, maxSigma, maxMu;
	double sigma, dx, chi2, minChi2;
	FitPars *ps;
	FitPars sWork;
	Fit *pFit = (Fit *)vpfit;
	
	double *x, *y;
	int n;

	x = pfd->x; y = pfd->y; n = pfd->n;

	pFit->converge = 1; /* a meaningless number in this case, but default = 1 is more appropriate */
	pFit->ndf = n - 1;

	ps = (FitPars *)pFit->curve;
	minChi2 = 1.0e30f;
	dx = x[1] - x[0];
//	deltaSigma = pFit->deltaSigma * dx;
	deltaSigma = 0.5f * dx;
//	deltaMu = pFit->deltaMu * dx;
	deltaMu = 0.5f * dx;
	maxSigma = 20.0f * dx;
	maxMu = 200.0f * dx;
	sWork.par[2] = y[n-1];
	sWork.par[0] = deltaMu;
	while(sWork.par[0] < maxMu) {
		sigma = deltaSigma;
		setSigma(&sWork,sigma);
		while(sigma < maxSigma) {
			chi2 = pFit->fxn(pfd,&sWork);
			if(chi2 < minChi2) {
				*ps = sWork;
				pFit->chi2 = minChi2 = chi2;
			}
			sigma += deltaSigma;
			setSigma(&sWork,sigma);
		}
		sWork.par[0] += deltaMu;
	}
	pFit->ndf = n - 1;
	pFit->chi2 = pFit->chi2 / (double)pFit->ndf;
	pFit->converge = 1;
	return 0;
}

static double invRoot6 = 0.40824829046f; 
/* jsv adjust algorithm so that we use root(12) */
int totGuess(FitData *pfd, FitPars *Params, void *vpfit) {
  int i, npts = pfd->n, pts[3]={0,npts/2,npts-1};
  double Q[3], y[3], par[3];
  //if(npts>5) pts[0]=3;
  for(i=0;i<3;i++){
    Q[i] = (double)pfd->x[pts[i]];
    y[i] = (double)pfd->y[pts[i]];
  }

  par[2] = (Q[2]*(Q[1]-Q[0])/(Q[2]-Q[0]) - Q[1]*(y[1]-y[pts[0]])/(y[2]-y[0])) / 
    ((y[1]-y[0])/(y[2]-y[0])-(Q[1]-Q[0])/(Q[2]-Q[0]));
  par[1] = (y[1]-y[2])*(par[2]+Q[1])*(par[2]+Q[2])/(Q[2]-Q[1]);
  par[0] = 0;
  par[0] = y[0] - par[1]/(par[2]+Q[0]);
  for(i=0;i<3;i++)
    Params->par[i] = (double) par[i];
  return 0;
}

int sGuess(FitData *pfd, FitPars *pSParams, void *vpfit) {
	double pt1, pt2, pt3, *pdlo, *pdhi, y_lo, y_hi, x1, x2, x3, sigma;
	double *x, *y, a0, hits, aSquared, minSigma;
	int j, k, n, lo1, lo2, lo3, hi1, hi2, hi3, status;
	pSParams->par[2] = pfd->y[pfd->n-1]; /* assumes last data point sets plateau */
	a0 = pSParams->par[2];
	a0 -= 0.00001f; /* skim a token amount to ensure comparisons succeed */

	n = pfd->n; /* number of samples */

	minSigma = (pfd->x[1] - pfd->x[0]) * invRoot6;
	if(n == 2) {
		/* no interesting data point */
		pSParams->par[0] = 0.5f * (pfd->x[1] + pfd->x[0]);
		sigma = minSigma; /* we do not have accurate information */
		setSigma(pSParams,sigma);
		status = 1;
	} else if(n == 3) {
		/* one interesting data point */
		aSquared = a0 * a0;
		hits = pfd->y[1];
	/* this next part is voodoo. being at such small number of interesting points,
		the maximum likelihood lies along a curve. Intuitively, we have a feel for where
		the mean and sigma go. sigma varies continuously between sigma0 and 2 * sigma0,
		sigma0 = bin / root(6) */
		sigma = minSigma; 
		sigma = sigma * ( 1.0f + 4.0f * hits * (a0 - hits) / aSquared );
		setSigma(pSParams,sigma);
		pSParams->par[0] = pfd->x[0]; /* initial guess at mu */
		pSParams->par[0] = manualFindMinMu(pfd, pSParams, vpfit);
		status = 1;
	} else {
		x = pfd->x; y = pfd->y;
		pt1 = 0.16f * a0;
		pt2 = 0.50f * a0;
		pt3 = 0.84f * a0;
	/* find the range over which the data crosses the 16%, 50% and 84% points */
		hi1 = hi2 = hi3 = 0; /* invalid values */
		lo1 = lo2 = lo3 = 0; /* invalid values */
		pdlo = &y[0]; /* y */
		pdhi = &y[n-1]; /* y + n - 1 */
	/* important note: for the sake of speed we want to perform as few comparisons as
		possible. Therefore, the integer comparison occurs first in each of the 
		following operations. Depending on the logical value thereof, the next
		comparison will be made only if necessary. To further expedite the code,
		arrays are not used because there are only three members */
		j = n - 1;
		for(k=0;k<n;++pdlo, --pdhi) {
			y_lo = *pdlo;
			y_hi = *pdhi;
			if(!lo1 && (y_lo >= pt1)) lo1 = k;
			if(!lo2 && (y_lo >= pt2)) lo2 = k;
			if(!lo3 && (y_lo >= pt3)) lo3 = k;
			if(!hi1 && (y_hi <= pt1)) hi1 = j;
			if(!hi2 && (y_hi <= pt2)) hi2 = j;
			if(!hi3 && (y_hi <= pt3)) hi3 = j;
			--j;
			++k;
		}
		x1 = (x[lo1] + x[hi1]) * 0.5f;
		x2 = (x[lo2] + x[hi2]) * 0.5f;
		x3 = (x[lo3] + x[hi3]) * 0.5f;

	/* mu = threshold */

		pSParams->par[0] = x2;
		sigma = (x3 - x1) * 0.7071067811f;
		if(sigma < minSigma) {
			sigma = minSigma; 
		}
		setSigma(pSParams,sigma);
		status = 0;
	}
	return status;
}

// static double lut_interval = 0.001f;
double inverse_lut_interval = 1000.0f;
#define LUT_WIDTH 3500
#define LUT_LENGTH (2*LUT_WIDTH+1)

double inverse_weight_lut_interval = 1000.0f;
#define WEIGHT_LUT_LENGTH 1001

#ifdef CCS2
/* needed for dsp code */
#pragma DATA_SECTION(data_errf, "xpdata");
#pragma DATA_SECTION(data_weight, "xpdata");
#endif

#if 0
/* jsv. not needed yet. exp(-x^2) */
static double data_exp2[LUT_LENGTH]={
#include "exp.dat"
};
#endif

double data_errf[LUT_LENGTH]={
#include "errf_ext.dat"
};

double data_weight[WEIGHT_LUT_LENGTH]={
#include "binomial_weight.dat"
};

double data_logx[2*LUT_LENGTH]={
#include "logx_ext.dat"
};

double abs_ext(double x) {
	if(x < 0.0) return -x;
	return x;
}

#if 0
/* makes the log(prob) lut */
#include <math.h>
void make_logx(void) {
	int n;
	double t, u, v;
	FILE *fp;
	fp = fopen("logx_ext.dat","w");
	for(n=0;n<LUT_LENGTH-1;++n) {
		t = (double)data_errf[n];
		if(t == 0.0f) t =       1.0e-6;
		if(t == 1.0f) t = 1.0 - 1.0e-6;
		u = t;
		u = log(u);
		v = 1.0f - t;
		v = log(v);
		fprintf(fp,"%ff,\n%ff,\n", u, v);
	}
	t = (double)data_errf[n];
	if(t == 0.0f) t =       1.0e-6;
	if(t == 1.0f) t = 1.0 - 1.0e-6;
	u = t;
	u = log(u);
	v = 1.0f - t;
	v = log(v);
	fprintf(fp,"%ff,\n%ff\n", u, v);
	fclose(fp);
}
#endif

double *log_ext(double x, FitPars *psp) {
	double u, t;
	int n;
	u = (x - psp->par[0]) / psp->par[1];
	t = u * inverse_lut_interval;
	n = LUT_WIDTH + (int)t;
	if(n < 0) n = 0;
	if(n >= LUT_LENGTH) n = LUT_LENGTH-1; /* truncate at last value */
	n = 2 * n;
	return data_logx + n;
}

double errf_ext(double x, FitPars *psp) {
	double u, t;
	int n;
	u = (x - psp->par[0]) / psp->par[1];
	t = u * inverse_lut_interval;
	n = LUT_WIDTH + (int)t;
	if(n < 0) n = 0;
	if(n >= LUT_LENGTH) n = LUT_LENGTH-1; /* truncate at last value */
	return data_errf[n];
}

/* making a function call to set sigma simultaneously allows me to set
	the inverse, so I don't keep dividing ( an expensive option ) */
void setSigma(FitPars *ps,double sigma) {
	ps->par[1] = sigma;
}

/* returns the number of good data found. zero indicates no good data */
int extractGoodData(FitData *pfdi,FitData *pfdo) {
	int hitsStart, hitsEnd, nBins;
	double a0, *y;
	nBins = pfdi->n;
	y = pfdi->y;
	for(hitsStart = 0;hitsStart < nBins; ++hitsStart)
		if(y[hitsStart] > 0.0001) break; /* really compare to 0 */
	if(hitsStart) --hitsStart; /* want to include at least one zero */
	a0 = 0.9999f * y[nBins - 1]; /* just under last bin */
	for(hitsEnd = nBins - 2;hitsEnd; --hitsEnd)
		if(y[hitsEnd] < a0) break;
	/* add 2: 
		1 because of where we start comparing and
		1 because we want to include one instance of the maximum */
	nBins = 2 + hitsEnd - hitsStart; 
	if(nBins < 0)
		nBins = 0;
	pfdo->n = nBins;
	pfdo->y = &y[hitsStart];
	pfdo->x = &pfdi->x[hitsStart];
	return nBins;
}

