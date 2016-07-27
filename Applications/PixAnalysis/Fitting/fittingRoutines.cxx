#include "fittingRoutines.h"
#include <math.h>
#include <stdio.h>


/*! \brief Fit S curve

*/
int fitSCurveCrawl(FitData *pfd, void *vpfit) {
	float deltaSigma, deltaMu;
	float mu_old, mu_new=0.;
	float sigma_old, sigma_new=0.;
	GaussianCurve sParams, sParamsWork, *psp;
	Fit *pFit = (Fit *)vpfit;
	Point point[3];
	int status;
	pFit->converge = 0; // assume no convergence 

	pFit->ndf = pfd->n - 1; // number of degrees of freedom 

// take a guess at the S-curve parameters 
	if(pFit->sGuess) {
		status = pFit->sGuess(pfd,&sParams,pFit);
		if(status) {
		// if too sharp a rise, not much more to do 
			pFit->muConverge = 0; // can't claim convergence 
			pFit->sigmaConverge = 0; // can't claim convergence 
			pFit->converge = status; // technically speaking, this is ok 
			pFit->nIters = 0;

			sParams.mu = pFit->findMinMu(pfd,&sParams,pFit);
			sParams.sigma = pFit->findMinSigma(pfd,&sParams,pFit);

			psp = (GaussianCurve *)pFit->curve;
			*psp = sParams;

			return 0;
		}
	}

// initialize loop parameters 
	psp = &sParamsWork;
	psp->a0 = sParams.a0;
	mu_old = sParams.mu;
	sigma_old = sParams.sigma;
	deltaSigma = pFit->deltaSigma;
	deltaMu = pFit->deltaMu;
	pFit->nIters = 0;

	while(pFit->nIters++ < pFit->maxIters) {
	// hold mu constant. find sigma 
		psp->mu = mu_old;

		setSigma(psp,sigma_old);
		point[0].x = psp->sigma;
		point[0].y = pFit->chiSquared(pfd,psp);

		setSigma(psp,sigma_old * (1.0f + deltaSigma));
		point[1].x = psp->sigma;
		point[1].y = pFit->chiSquared(pfd,psp);

		setSigma(psp,sigma_old * (1.0f - deltaSigma));
		point[2].x = psp->sigma;
		point[2].y = pFit->chiSquared(pfd,psp);

		sigma_new = nextAbscissa(point);

	// hold new sigma constant. find new mu. 
		setSigma(psp,sigma_new);
	// psp->mu = mu_old; valid line but unnecessary 
		point[0].x = psp->mu;
		point[0].y = pFit->chiSquared(pfd,psp);

		psp->mu = (1.0f + deltaMu) * mu_old;
		point[1].x = psp->mu;
		point[1].y = pFit->chiSquared(pfd,psp);

		psp->mu = (1.0f - deltaMu) * mu_old;
		point[2].x = psp->mu;
		point[2].y = pFit->chiSquared(pfd,psp);

		mu_new = nextAbscissa(point);

	// if converge, break 
		if((fabsf(mu_new - mu_old) < pFit->muEpsilon * mu_old) &&
			(fabsf(sigma_new - sigma_old) < pFit->sigmaEpsilon * sigma_old)) {
			pFit->muConverge = pFit->sigmaConverge = pFit->converge = 1;
			break;
		} else {
			mu_old = mu_new; sigma_old = sigma_new;
		}
	}

	psp = (GaussianCurve *)pFit->curve;
	psp->a0 = sParams.a0;
	psp->mu = mu_new;
	setSigma(psp,sigma_new);
	pFit->chi2 = pFit->chiSquared(pfd,psp) / (float)pFit->ndf;

// jsv not implemented yet pFit->nIters -= pFit->extraIter; // adjust for extra iterations 
	return pFit->converge ? 0 : 1; // 0 = success 
}

/*! \brief Get parabolic abscissa

*/
float parabolicAbscissa(Point *pt) {  
	float deltaX, denom, push;
	Point *p0, *p1, *p2;
	p0 = pt;	p1 = &pt[1]; p2 = &pt[2];
	deltaX = p0->x - p2->x;
	denom = 2.0f * p0->y - p1->y - p2->y; // also is the negative of concavity 
	if(denom == 0.0f) {return 0.0f;} // 0 is not a number we like 
	push = 0.5f * deltaX * (p1->y - p2->y) / denom;
	return p0->x + push;
}

// jsv outstanding issue in nextAbscissa(). 
//	should a push by deltaX be reduced to prevent oscillations? 
/*! \brief Get next abscissa

*/
float nextAbscissa(Point *pt) {
	float deltaX, denom, push, rhoTiny, rhoSmall, rhoLargeDerivative;
//	float concavity;
	Point *p0, *p1, *p2;
	p0 = pt;	p1 = &pt[1]; p2 = &pt[2];
	deltaX = p0->x - p2->x;
// ensure denominator is not zero 
	denom = 2.0f * p0->y - p1->y - p2->y; // also is the negative of concavity 
//	concavity = - denom;
// jsv. this is voodoo. if the concavity is negative, it means the extremum would
//	be a maximum. We are looking for a minimum, so we must "push" in the other
//	direction; the direction that would give a smaller value of y 
	if(denom >= 0.0f) {
	  if(p1->y >= p2->y) {return p2->x - deltaX;}
	  else {return p1->x + deltaX;}
	}
	push = 0.5f * deltaX * (p1->y - p2->y) / denom;
	rhoTiny = 0.001f; // jsv replace with fit.concavitySmallness 
	if(fabsf(denom) < (rhoTiny * p0->y)) {
	// the concativity is relatively small. dampen the push 
	  if(push >= 0.0f) {return p0->x + deltaX;}
	  else {return p0->x - deltaX;}
	}

	rhoSmall = 0.01f; // jsv
	rhoLargeDerivative = 0.10f; // jsv
	if((fabsf(denom) < rhoSmall * p0->y) && 
		(fabsf((p1->y - p2->y)/p0->y) > rhoLargeDerivative)){
	  if(p1->y > p2->y) {return p2->x - deltaX;}
	  else {return p1->x + deltaX;}
	}
	return p0->x + push;
}

//! Manually finds mu for minimizing chi2 by scanning the region between \n
//	mu_guess - tBegin * dmu through mu_guess + tFinal * dmu \n
//	dmu = x[1] - x[0] = spacing between x coordinates 

float manualFindMinMu(FitData *pfd, GaussianCurve *ps, void *vpFit) {
	float minmu, dmu, muFinal, chi2, minChi2;
	GaussianCurve sParamsWork, *psw;
	Fit *pFit;
	pFit = (Fit *)vpFit;
	psw = &sParamsWork;

	dmu = pfd->x[1] - pfd->x[0];

// setup scan parameters 
	psw->mu = ps->mu - pFit->manualSeekMu.tBegin * dmu;
	muFinal = ps->mu + pFit->manualSeekMu.tFinal * dmu;
	psw->a0 = ps->a0;
	setSigma(psw,ps->sigma);

// the first point is taken to be the current minimum 
	minChi2 = pFit->chiSquared(pfd,psw);
	minmu = psw->mu;

	dmu = dmu * pFit->manualSeekMu.dt;
	while(psw->mu < muFinal) {
		psw->mu += dmu; // next mu 
		chi2 = pFit->chiSquared(pfd,psw);
		if(chi2 <= minChi2) {
			minChi2 = chi2;
			minmu = psw->mu;
		}
	}
	pFit->chi2 = minChi2 / (float)pFit->ndf;
	return minmu;
}

//! manually finds sigma for minimizing chi2 by scanning the region between \n
//	sigma_guess - tBegin * dsigma through sigma_guess + tFinal * dsigma \n
//	dsigma = x[1] - x[0] = spacing between x coordinates 

float manualFindMinSigma(FitData *pfd, GaussianCurve *ps, void *vpFit) {
        int niter = 0;
	float minSigma, dSigma, endSigma, chi2, minChi2, sigma;
	GaussianCurve sParamsWork, *psw;
	Fit *pFit;
	pFit = (Fit *)vpFit;
	psw = &sParamsWork;

// setup scan parameters 
	dSigma = pFit->manualSeekSigma.dt * (pfd->x[1] - pfd->x[0]); // rescaled. jsv pass as argument? 
	endSigma = pFit->manualSeekSigma.tFinal * ps->sigma;
	sigma = pFit->manualSeekSigma.tBegin * ps->sigma;
	setSigma(psw,sigma); // beginning value 
	psw->a0 = ps->a0;
	psw->mu = ps->mu;

// the first value is taken to be the current minimum. reasonable 
	minChi2 = pFit->chiSquared(pfd,psw);
	minSigma = psw->sigma;

	while(sigma < endSigma && niter < 10000) {
		sigma += dSigma; // next sigma 
		setSigma(psw,sigma);
		chi2 = pFit->chiSquared(pfd,psw);
		if(chi2 <= minChi2) {
			minChi2 = chi2;
			minSigma = sigma;
		}
		niter++;
	}
	pFit->chi2 = minChi2 / (float)pFit->ndf;
	return minSigma;
}

/*! \brief Fit S curve

*/
int fitSCurveAim(FitData *pfd, void *vpfit) {
	float deltaSigma, deltaMu, sigma, chi2Min, *fptr;
	float chi2[9];
	GaussianCurve sParams, sParamsWork, *psp;
	int k, dirMin;
	Fit *pFit;
	pFit = (Fit *)vpfit;

	pFit->nIters = 0;
	pFit->converge = 0; // assume no convergence 

	pFit->ndf = pfd->n - 2; // degrees of freedom 

// take a guess at the S-curve parameters 
	if(pFit->sGuess) {
		if(pFit->sGuess(pfd,&sParams,pFit)) {
			psp = (GaussianCurve *)pFit->curve;
			psp->a0 = sParams.a0;
			psp->mu = sParams.mu;
			setSigma(psp, sParams.sigma);
			pFit->converge = 1;
			pFit->muConverge = 0;
			pFit->sigmaConverge = 0;
			pFit->chi2 = 0.0f;
			return 0;
		}
	}

// initialize loop parameters 
	psp = &sParamsWork;
	psp->a0 = sParams.a0;
	deltaSigma = pFit->deltaSigma * sParams.sigma; // scaled 
	deltaMu = pFit->deltaMu * sParams.mu; // scaled 
//* the loop begins *

	while(!pFit->converge && (pFit->nIters++ < pFit->maxIters)) {

		dirMin = 0;
		fptr = chi2;
		for(k=0;k<9;++k) *fptr++ = 0.0f;

		while((dirMin >= 0) && (pFit->nIters++ < pFit->maxIters)) {

		//* calculate neighboring points *
	//* calculate neighboring points *
			setSigma(psp,sParams.sigma - deltaSigma);
			psp->mu = sParams.mu - deltaMu;
			if(chi2[0] == 0.0f)
				chi2[0] = pFit->chiSquared(pfd,psp);
			psp->mu = sParams.mu;
			if(chi2[7] == 0.0f)
				chi2[7] = pFit->chiSquared(pfd,psp);
			psp->mu = sParams.mu + deltaMu;
			if(chi2[6] == 0.0f)
				chi2[6] = pFit->chiSquared(pfd,psp);

			setSigma(psp,sParams.sigma);
			psp->mu = sParams.mu - deltaMu;
			if(chi2[1] == 0.0f)
				chi2[1] = pFit->chiSquared(pfd,psp);
			psp->mu = sParams.mu;
			if(chi2[8] == 0.0f)
				chi2[8] = pFit->chiSquared(pfd,psp);
			psp->mu = sParams.mu + deltaMu;
			if(chi2[5] == 0.0f)
				chi2[5] = pFit->chiSquared(pfd,psp);

			setSigma(psp,sParams.sigma + deltaSigma);
			psp->mu = sParams.mu - deltaMu;
			if(chi2[2] == 0.0f)
				chi2[2] = pFit->chiSquared(pfd,psp);
			psp->mu = sParams.mu;
			if(chi2[3] == 0.0f)
				chi2[3] = pFit->chiSquared(pfd,psp);
			psp->mu = sParams.mu + deltaMu;
			if(chi2[4] == 0.0f)
				chi2[4] = pFit->chiSquared(pfd,psp);

			dirMin = -1;
			chi2Min = chi2[8]; // first guess at minimum 
			for(k=0, fptr=chi2;k<8;++k, ++fptr) {
				if(*fptr < chi2Min) {
					chi2Min = *fptr;
					dirMin = k;
				}
			}

			switch(dirMin) {
			case -1:
				deltaSigma = deltaSigma * 0.1f;
				deltaMu = deltaMu * 0.1f;
				break;
			case 0:
				sigma = sParams.sigma - deltaSigma;
				setSigma(&sParams, sigma);
				sParams.mu = sParams.mu - deltaMu;
				chi2[8] = chi2[0];
				chi2[3] = chi2[1];
				chi2[4] = chi2[8];
				chi2[5] = chi2[7];
				chi2[0] = chi2[1] = chi2[2] = 
					chi2[6] = chi2[7] = 0.0f;
				break;
			case 1:
				sParams.mu = sParams.mu - deltaMu;
				chi2[8] = chi2[1];
				chi2[3] = chi2[2];
				chi2[4] = chi2[3];
				chi2[5] = chi2[8];
				chi2[6] = chi2[7];
				chi2[7] = chi2[0];
				chi2[0] = chi2[1] = chi2[2] = 0.0f;
				break;
			case 2:
				sigma = sParams.sigma + deltaSigma;
				setSigma(&sParams, sigma);
				sParams.mu = sParams.mu - deltaMu;
				chi2[8] = chi2[2];
				chi2[5] = chi2[3];
				chi2[6] = chi2[8];
				chi2[7] = chi2[1];
				chi2[0] = chi2[1] = chi2[2] = 
					chi2[3] = chi2[4] = 0.0f;
				break;
			case 3:
				sigma = sParams.sigma + deltaSigma;
				setSigma(&sParams, sigma);
				chi2[8] = chi2[3];
				chi2[5] = chi2[4];
				chi2[6] = chi2[5];
				chi2[7] = chi2[8];
				chi2[0] = chi2[1];
				chi2[1] = chi2[2];
				chi2[2] = chi2[3] = chi2[4] = 0.0f;
				break;
			case 4:
				sigma = sParams.sigma + deltaSigma;
				setSigma(&sParams, sigma);
				sParams.mu = sParams.mu + deltaMu;
				chi2[8] = chi2[4];
				chi2[7] = chi2[5];
				chi2[0] = chi2[8];
				chi2[1] = chi2[3];
				chi2[2] = chi2[3] = chi2[4] = 
					chi2[5] = chi2[6] = 0.0f;
				break;
			case 5:
				sParams.mu = sParams.mu + deltaMu;
				chi2[8] = chi2[5];
				chi2[7] = chi2[6];
				chi2[0] = chi2[7];
				chi2[1] = chi2[8];
				chi2[2] = chi2[3];
				chi2[3] = chi2[4];
				chi2[4] = chi2[5] = chi2[6] = 0.0f;
				break;
			case 6:
				sigma = sParams.sigma - deltaSigma;
				setSigma(&sParams, sigma);
				sParams.mu = sParams.mu + deltaMu;
				chi2[8] = chi2[6];
				chi2[1] = chi2[7];
				chi2[2] = chi2[8];
				chi2[3] = chi2[5];
				chi2[4] = chi2[5] = chi2[6] = 
					chi2[7] = chi2[0] = 0.0f;
				break;
			case 7:
				sigma = sParams.sigma - deltaSigma;
				setSigma(&sParams, sigma);
				chi2[8] = chi2[7];
				chi2[1] = chi2[0];
				chi2[2] = chi2[1];
				chi2[3] = chi2[8];
				chi2[4] = chi2[5];
				chi2[5] = chi2[6];
				chi2[6] = chi2[7] = chi2[0] = 0.0f;
				break;
			}
		}

		if(deltaSigma < (pFit->sigmaEpsilon * sParams.sigma) &&
			deltaMu < (pFit->muEpsilon * sParams.mu)) {
			pFit->converge = 1;
			break;
		}
	}
	psp = (GaussianCurve *)pFit->curve;
	*psp = sParams;
	pFit->chi2 = chiSquared(pfd,psp) / (float)pFit->ndf; // jsv member function?
	return pFit->converge ? 0 : 1; // 0 = success 
}


int fitSCurveAimTot(FitData *pfd, void *vpfit) {
  float deltaPar[NPMAX], epsilonPar[NPMAX];
  //  float *fptr, chi2Min, chi2[9];
  float chi2old, chi2new, delFact[NPMAX], delFactOpt[NPMAX];
  FitPars sParams, sParamsWork, *psp;
  int k, l, m, ind, nvar, dirMin;
  FitCtrl *pFit;
  pFit = (FitCtrl *)vpfit;
  
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
  for(k=0; k<pFit->npars; k++){
    deltaPar[k] = pFit->deltaPar[k] * sParams.par[k]; /* scaled */
    epsilonPar[k] = pFit->epsilonPar[k] * sParams.par[k]; /* scaled */
}

  /** the loop begins **/
  
  while(!pFit->converge && (pFit->nIters++ < pFit->maxIters)) {
    
    dirMin = 0;
    
    while((dirMin >= 0) && (pFit->nIters++ < pFit->maxIters)) {
      
      /** calculate neighboring points **/
      chi2old = pFit->fxn(pfd,&sParams); // chi2 from unchanged pars
      dirMin = -1;
      // then vary par's by +-deltaPars and see if chi2 gets smaller
      //calculate 3^pFit->npars
      nvar=1;
      for(m=0;m<pFit->npars;m++) nvar *= 3; //nvar = (int)pow(3,(float)pFit->npars);
      
      for(k=0;k<pFit->npars; k++){ // initialise del factors
		delFact[k] = -1;
		delFactOpt[k] = 0;
      }
      
      for(l=1;l<(nvar + 1); l++){ // loop over (+1/0/-1)*delta-possibilities      
      //hg tbd: requires comments This loop is used to walk through the different parameters */
		for(k=0;k<pFit->npars; k++){ // loop over pars
			psp->par[k] = sParams.par[k] + delFact[k]*deltaPar[k];	
	  		//calculate ind = 3^k
	  		ind=1; 		for(m=0;m<k;m++) ind *= 3; // ind = (int)pow(3,(float)k);
	  		ind = (l/ind)*ind;
	 		if(ind==l) delFact[k]++;
	  		if(delFact[k]>1) delFact[k]=-1;	
		}
		chi2new = pFit->fxn(pfd,psp);
		if(chi2new<chi2old){ 
	 		dirMin = l;
	 		chi2old = chi2new; //save updated value of chi2
	  		for(k=0;k<pFit->npars; k++)
	    		delFactOpt[k] = delFact[k];
		}
      }
      if(dirMin>=0){
		for(k=0;k<pFit->npars; k++){ // choose min. settings
	 		sParams.par[k] = sParams.par[k] + delFactOpt[k]*deltaPar[k];
	  	}
      }
      else{
		for(k=0;k<pFit->npars; k++) // decrease step size
	  		deltaPar[k] *= 0.1f;
      }
    }

    pFit->converge = 1;
    for(k=0;k<pFit->npars; k++){ // check successful convergency
    if((fabsf(deltaPar[k] * delFactOpt[k])) > fabsf(epsilonPar[k])) // hg tbd decide on convergence criteria  
    		pFit->converge = 0;
	}
  }

  psp = (FitPars *)pFit->curve;
  *psp = sParams;
  pFit->chi2 = pFit->chiSquared(pfd,psp) / (float)pFit->ndf;
  return pFit->converge ? 0 : 1; /* 0 = success */
}

// jsv. are there potential oscillatory problems by constantly rescaling
//	deltaSigma & deltaMu? 
//	what about the number of degrees of freedom? 
/*! \brief Fit S curve using parabolic algorithm

*/
int fitSCurveParabolicAlgorithm(FitData *pfd, void *vpfit) {
	float deltaSigma, deltaMu, x0, x1, y0, y1;
	float a0, a1, b0, b1, c0, c1, invDet;
	float mu_old, mu_new, chi2[4], chi2Temp, chi2CutOff, temp;
	float sigma_old, sigma_new;
	GaussianCurve sParams, sParamsWork, *psp;
	Point point[3];
	Point pointSigma[2], pointMu[2];
	int extraIter, status;
	Fit *pFit;
	pFit = (Fit *)vpfit;

	pFit->converge = 0; // assume no convergence 

	pFit->ndf = pfd->n - 1; // degrees of freedom 

	chi2CutOff = pFit->chi2CutOff * (float)pFit->ndf;

// take a guess at the S-curve parameters 
	if(pFit->sGuess) {
		status = pFit->sGuess(pfd,&sParams,pFit);
		if(status) {
			psp = (GaussianCurve *)pFit->curve;
			psp->a0 = sParams.a0;
			psp->mu = sParams.mu;
			setSigma(psp,pFit->sigmaMin);
			pFit->converge = 1;
			pFit->muConverge = 0;
			pFit->sigmaConverge = 0;
			return 2;//status;
		}
	}

// initialize loop parameters 
	psp = &sParamsWork;
	psp->a0 = sParams.a0;
	mu_old = sParams.mu;
	sigma_old = sParams.sigma;
	deltaSigma = pFit->deltaSigma;
	deltaMu = pFit->deltaMu;
	pFit->nIters = 0;
	extraIter = pFit->extraIter;

	while(pFit->nIters++ < pFit->maxIters) {

	// sigma 
		setSigma(psp,(1.0f - deltaSigma) * sParams.sigma);

	// calculate neighboring points 
		point[1].x = psp->mu = (1.0f + deltaMu) * sParams.mu;
		point[1].y = chi2[3] = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 3; 
		}			

		point[0].x = psp->mu = sParams.mu;
		point[0].y = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 4; 
		}			

		point[2].x = psp->mu = (1.0f - deltaMu) * sParams.mu;
		point[2].y = chi2[1] = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 5; 
		}			

		pointSigma[0].x = nextAbscissa(point);
//		pointSigma[0].x = parabolicAbscissa(point);
		pointSigma[0].y = psp->sigma;

	// 

		setSigma(psp,(1.0f + deltaSigma) * sParams.sigma);

	// calculate neighboring points 
		point[1].x = psp->mu = (1.0f + deltaMu) * sParams.mu;
		point[1].y = chi2[2] = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 6; 
		}			

		point[0].x = psp->mu = sParams.mu;
		point[0].y = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 7; 
		}			

		point[2].x = psp->mu = (1.0f - deltaMu) * sParams.mu;
		point[2].y = chi2[0] = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 8; 
		}			

		pointSigma[1].x = nextAbscissa(point);
//		pointSigma[1].x = parabolicAbscissa(point);
		pointSigma[1].y = psp->sigma;

	// calculate the line between these two sigma points 

		x0 = pointSigma[0].x; x1 = pointSigma[1].x;
		y0 = pointSigma[0].y; y1 = pointSigma[1].y;

		a0 = y1-y0; b0 = x0-x1; c0=x0*y1-x1*y0;

	// mu 

		psp->mu = (1.0f - deltaMu) * sParams.mu;

	// calculate neighboring points 
		setSigma(psp,(1.0f + deltaSigma) * sParams.sigma);
		point[1].x = psp->sigma;
		point[1].y = chi2[0]; // already calculated 

		setSigma(psp,sParams.sigma);
		point[0].x = psp->sigma;
		point[0].y = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 9; 
		}			

		setSigma(psp,(1.0f - deltaSigma) * sParams.sigma);
		point[2].x = psp->sigma;
		point[2].y = chi2[1]; // already calculated 

		pointMu[0].x = psp->mu;
		pointMu[0].y = nextAbscissa(point);
//		pointMu[0].y = parabolicAbscissa(point);

	// 

		psp->mu = (1.0f + deltaMu) * sParams.mu;

	// calculate neighboring points 
		setSigma(psp,(1.0f + deltaSigma) * sParams.sigma);
		point[1].x = psp->sigma;
		point[1].y = chi2[2]; // already calculated 

		setSigma(psp,sParams.sigma);
		point[0].x = psp->sigma;
		point[0].y = chi2Temp = pFit->chiSquared(pfd,psp);
		if(chi2Temp > chi2CutOff) { 
			return 10; 
		}			

		setSigma(psp,(1.0f - deltaSigma) * sParams.sigma);
		point[2].x = psp->sigma;
		point[2].y = chi2[3]; // already calculated 

		pointMu[1].x = psp->mu;
		pointMu[1].y = nextAbscissa(point);
//		pointMu[1].y = parabolicAbscissa(point);

	// calculate the line between these mu points 

		x0 = pointMu[0].x; x1 = pointMu[1].x;
		y0 = pointMu[0].y; y1 = pointMu[1].y;

		a1 = y1-y0; b1 = x0-x1; c1 = x0*y1-x1*y0;

	// where do the lines intersect 

		temp = a0 * b1 - b0 * a1;
		if(temp == 0.0f) {
			return 11;
		}
		invDet = 1.0f / temp; // not singular 
		mu_new = ( b1 * c0 - b0 * c1 ) * invDet;
		if(mu_new > pFit->muCutOff) {
			return 12;
		}
		sigma_new = ( a0 * c1 - a1 * c0 ) * invDet;
		if(sigma_new > pFit->sigmaCutOff) {
			return 13;
		}

	// if converge, break 
		if(fabsf(mu_new - mu_old) < pFit->muEpsilon * mu_old) {
			// jsv deltaMu = deltaMu / 2;
			pFit->muConverge = 1;
		} else {
//			if(pFit->muConverge)
//				dsp_printf("mu deconverged %f %f\n", mu_old, sigma_old); // jsv
			pFit->muConverge = 0; // it is possible to deconverge 
			extraIter = pFit->extraIter;
		}
		
		if(fabsf(sigma_new - sigma_old) < pFit->sigmaEpsilon * sigma_old) {
			// jsv deltaSigma = deltaSigma / 2;
			pFit->sigmaConverge = 1;
		} else {
//			if(pFit->sigmaConverge)
//				dsp_printf("sigma deconverged %f %f\n", mu_old, sigma_old); // jsv
			pFit->sigmaConverge = 0; // it is possible to deconverge 
			extraIter = pFit->extraIter;
		}

		if(pFit->muConverge && pFit->sigmaConverge) {
			if(!extraIter) {
				pFit->converge = 1; 
				break;
			} else {
				--extraIter;
			}
		}

		sParams.mu = mu_old = mu_new;
		sigma_old = sigma_new;
		setSigma(&sParams,sigma_new);

	}

// manually verify sigma 
	pFit->manualSeekSigma.dt = 0.5;
	pFit->manualSeekSigma.tBegin = 0.5;
	pFit->manualSeekSigma.tFinal = 1.5;
	sigma_new = pFit->findMinSigma(pfd,&sParams,pFit);
	setSigma(&sParams,sigma_new);
	mu_new = pFit->findMinMu(pfd,&sParams,pFit);
	sParams.mu = mu_new;

	psp = (GaussianCurve *)pFit->curve;
	psp->a0 = sParams.a0;
	psp->mu = mu_new;
	setSigma(psp,sigma_new);
	pFit->chi2 = pFit->chiSquared(pfd,psp) / (float)pFit->ndf;
	pFit->nIters -= pFit->extraIter; // adjust for extra iteration 
	return pFit->converge ? 0 : 14; // 0 = success 
}
	
/*! \brief Fit S curve using brute force

*/
int fitSCurveBruteForce(FitData *pfd, void *vpfit) {
	float deltaSigma, deltaMu, maxSigma, maxMu;
	float sigma, dx, chi2, minChi2;
	GaussianCurve *ps;
	GaussianCurve sWork;
	Fit *pFit = (Fit *)vpfit;
	float *x, *y;
	int n;
	x = pfd->x; y = pfd->y; n = pfd->n;

	pFit->converge = 1; // a meaningless number in this case, but default = 1 is more appropriate 
	pFit->ndf = n - 1;

	ps = (GaussianCurve *)pFit->curve;
	minChi2 = 1.0e30f;
	dx = x[1] - x[0];
//	deltaSigma = pFit->deltaSigma * dx;
	deltaSigma = 0.5f * dx;
//	deltaMu = pFit->deltaMu * dx;
	deltaMu = 0.5f * dx;
	maxSigma = 20.0f * dx;
	maxMu = 200.0f * dx;
	sWork.a0 = y[n-1];
	sWork.mu = deltaMu;
	while(sWork.mu < maxMu) {
		sigma = deltaSigma;
		setSigma(&sWork,sigma);
		while(sigma < maxSigma) {
			chi2 = pFit->chiSquared(pfd,&sWork);
			if(chi2 < minChi2) {
				*ps = sWork;
				pFit->chi2 = minChi2 = chi2;
			}
			sigma += deltaSigma;
			setSigma(&sWork,sigma);
		}
		sWork.mu += deltaMu;
	}
	pFit->ndf = n - 1;
	pFit->chi2 = pFit->chi2 / (float)pFit->ndf;
	pFit->converge = 1;
	return 0;
}

/*! \brief Guess TOT parameters by solving the equations

*/
int totGuess(FitData *pfd, FitPars *Params, void* /*vpfit*/) {
  float x[3], y[3];

  // select three sample points
  x[0] = pfd->x[0];
  x[1] = pfd->x[pfd->n/2];
  x[2] = pfd->x[pfd->n-1]; 
  y[0] = pfd->y[0];
  y[1] = pfd->y[pfd->n/2];
  y[2] = pfd->y[pfd->n-1];

  // guess values by solving the equations
  Params->par[2] = (float) (x[2]*(x[1]-x[0])/(x[2]-x[0]) - x[1]*(y[1]-y[0])/(y[2]-y[0])) / 
    ((y[1]-y[0])/(y[2]-y[0])-(x[1]-x[0])/(x[2]-x[0]));
  Params->par[1] = (float) (y[1]-y[2])*(Params->par[2]+x[1])*(Params->par[2]+x[2])/(x[2]-x[1]);
  Params->par[0] = (float) y[0] - Params->par[1]/(Params->par[2]+x[0]);
  return 0;
}

/*! \brief Guess using the new TOT parametrisation function */
int totGuessNew(FitData *pfd, FitPars *Params, void* /*vpfit*/) {
  float x[3], y[3];
  // select three sample points
  x[0] = pfd->x[0];
  x[1] = pfd->x[pfd->n/2];
  x[2] = pfd->x[pfd->n-1]; 
  y[0] = pfd->y[0];
  y[1] = pfd->y[pfd->n/2];
  y[2] = pfd->y[pfd->n-1];
  
  // guess values by solving the equations
  Params->par[1] = (float) (x[1]*x[2]*y[0]*(y[1] - y[2]) + x[0]*(x[1]*(y[0] - y[1])*y[2] + x[2]*y[1]*(-y[0] + y[2])))/(x[0]*y[0]*(y[1] - y[2]) + x[2]*(y[0] - y[1])*y[2] + x[1]*y[1]*(-y[0] + y[2])); 
  Params->par[2] = (float) (x[1]*x[2]*(y[1] - y[2]) + x[0]*(x[1]*(y[0] - y[1]) + x[2]*(-y[0] + y[2])))/(x[2]*(y[0] - y[1]) + x[0]*(y[1] - y[2]) + x[1]*(-y[0] + y[2]));
  Params->par[0] = (float) (x[0]*y[0]*(y[1] - y[2]) + x[2]*(y[0] - y[1])*y[2] + x[1]*y[1]*(-y[0] + y[2]))/(x[2]*(y[0] - y[1]) + x[0]*(y[1] - y[2]) + x[1]*(-y[0] + y[2]));
  return 0;
}

/*! \brief Guess TOT parameters using a linear fit

*/
int totGuessLinear(FitData *pfd, FitPars *Params, void* /*vpfit*/) {
  int i, npts, pts[3];
  float Q[3], y[3], par[3];
  npts = pfd->n;
  
  //if(npts>5) pts[0]=3;
  for(i=0;i<3;i++){
  	switch(i){
  		case 0: pts[i] = 0; break;
  		case 1: pts[i] = npts/2; break;
  		case 2: pts[i] = npts-1; break;
  	}
    Q[i] = pfd->x[pts[i]];
    y[i] = pfd->y[pts[i]];
  }

  par[0] = (Q[1]*y[0] - Q[0]*y[1])/(Q[1] - Q[0]);
  par[1] = (y[1] - y[0])/(Q[1] - Q[0]);
  for(i=0;i<3;i++)
    Params->par[i] = (float) par[i];
  return 0;
}

static float invRoot6 = 0.40824829046f; 
// jsv adjust algorithm so that we use root(12) 

/*! \brief Guess S curve

*/
int sGuess(FitData *pfd, GaussianCurve *pSParams, void *vpfit) {
	float pt1, pt2, pt3, *pdlo, *pdhi, y_lo, y_hi, x1, x2, x3, sigma;
	float *x, *y, a0, hits, aSquared, minSigma;
	int j, k, n, lo1, lo2, lo3, hi1, hi2, hi3, status;
	pSParams->a0 = pfd->y[pfd->n-1]; // assumes last data point sets plateau 
	a0 = pSParams->a0;
	a0 -= 0.00001f; // skim a token amount to ensure comparisons succeed 

	n = pfd->n; // number of samples 

	minSigma = (pfd->x[1] - pfd->x[0]) * invRoot6;
	if(n == 2) {
		// no interesting data point 
		pSParams->mu = 0.5f * (pfd->x[1] + pfd->x[0]);
		sigma = minSigma; // we do not have accurate information 
		setSigma(pSParams,sigma);
		status = 1;
	} else if(n == 3) {
		// one interesting data point 
		aSquared = a0 * a0;
		hits = pfd->y[1];
	// this next part is voodoo. being at such small number of interesting points,
		//the maximum likelihood lies along a curve. Intuitively, we have a feel for where
		///the mean and sigma go. sigma varies continuously between sigma0 and 2 * sigma0,
		//sigma0 = bin / root(6) 
		sigma = minSigma; 
		sigma = sigma * ( 1.0f + 4.0f * hits * (a0 - hits) / aSquared );
		setSigma(pSParams,sigma);
		pSParams->mu = pfd->x[0]; // initial guess at mu 
		pSParams->mu = manualFindMinMu(pfd, pSParams, vpfit);
		status = 1;
	} else {
		x = pfd->x; y = pfd->y;
		pt1 = 0.16f * a0;
		pt2 = 0.50f * a0;
		pt3 = 0.84f * a0;
	// find the range over which the data crosses the 16%, 50% and 84% points 
		hi1 = hi2 = hi3 = 0; // invalid values 
		lo1 = lo2 = lo3 = 0; // invalid values 
		pdlo = &y[0]; // y 
		pdhi = &y[n-1]; // y + n - 1 
	// important note: for the sake of speed we want to perform as few comparisons as
		//possible. Therefore, the integer comparison occurs first in each of the 
		//following operations. Depending on the logical value thereof, the next
		//comparison will be made only if necessary. To further expedite the code,
		//arrays are not used because there are only three members 
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

	// mu = threshold 

		pSParams->mu = x2;
		sigma = (x3 - x1) * 0.7071067811f;
		if(sigma < minSigma) {
			sigma = minSigma; 
		}
		setSigma(pSParams,sigma);
		status = 0;
	}
	return status;
}

// static float lut_interval = 0.001f;
float inverse_lut_interval = 1000.0f;

float inverse_weight_lut_interval = 1000.0f;

float data_errf[LUT_LENGTH]={
#include "errf_ext.dat"
};

float binomial_weight[WEIGHT_LUT_LENGTH]={
#include "binomial_weight.dat"
};

float data_logx[2*LUT_LENGTH]={
#include "logx_ext.dat"
};

float data_log[LUT_LENGTH]={
#include "logx.dat"
};

// default values to current unrelocated area 
static float *pDataErrf = data_errf;
static float *pDataLogx = data_logx;
static float *pDataLog = data_log;
float *data_weight = binomial_weight;



/*
// makes the log(prob) lut 
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
		fdsp_printf(fp,"%ff,\n%ff,\n", u, v);
	}
	t = (double)data_errf[n];
	if(t == 0.0f) t =       1.0e-6;
	if(t == 1.0f) t = 1.0 - 1.0e-6;
	u = t;
	u = log(u);
	v = 1.0f - t;
	v = log(v);
	fdsp_printf(fp,"%ff,\n%ff\n", u, v);
	dsp_fclose(fp);
	return;
}
*/

float *log_ext(float x, GaussianCurve *psp) {
	float u, t;
	int n;
	u = (x - psp->mu) * psp->sigmaInv;
	t = u * inverse_lut_interval;
	n = LUT_WIDTH + (int)t;
	if(n < 0) n = 0;
	if(n >= LUT_LENGTH) n = LUT_LENGTH-1; // truncate at last value 
	n = 2 * n;
	return pDataLogx + n;
//	return data_logx + n;
}

float log_e(float x) {
	float u;
	int n;
	u = 10.0;
	n = (int) (x *(LUT_LENGTH -1)/u);
	if(n < 0) n = 0;
	if(n >= LUT_LENGTH) n = LUT_LENGTH-1; // truncate at last value 
	return pDataLog[n];
}

float errf_ext(float x, GaussianCurve *psp) {
	float u, t;
	int n;
	u = (x - psp->mu) * psp->sigmaInv;
	t = u * inverse_lut_interval;
	n = LUT_WIDTH + (int)t;
	if(n < 0) n = 0;
	if(n >= LUT_LENGTH) n = LUT_LENGTH-1; // truncate at last value 
	return pDataErrf[n];
}

void setSigma(GaussianCurve *ps,float sigma) {
	ps->sigma = sigma;
	ps->sigmaInv = 1.0f / sigma;
	return;
}

// interface to output of slave histogram 

/*
float fitChi2(float *y,int n,GaussianCurve *s,float *x) {
	float acc = 0.0f, t;
	while(n--) {
		t = *y++ - errf_ext(*x++,s);
		acc += (t * t);
	}
	return acc;
}
*/

//! \brief returns the number of good data found. zero indicates no good data \n
int extractGoodData(FitData *pfdi,FitData *pfdo) {
	int hitsStart, hitsEnd, nBins;
	float a0, *y;
	nBins = pfdi->n;
	y = pfdi->y;
	for(hitsStart = 0;hitsStart < nBins; ++hitsStart)
		if(y[hitsStart] > 0.0001) break; // really compare to 0 
	if(hitsStart) --hitsStart; // want to include at least one zero 
	a0 = 0.9999f * y[nBins - 1]; // just under last bin 
	if(a0<5 && nBins>1) a0 = 0.9999f * y[nBins - 2]; // just under last bin 
	for(hitsEnd = nBins - 2;hitsEnd; --hitsEnd)
		if(y[hitsEnd] < a0) break;
	// add 2: 
	//	1 because of where we start comparing and
	//	1 because we want to include one instance of the maximum 
	nBins = 2 + hitsEnd - hitsStart; 
	if(nBins < 0)
		nBins = 0;
	pfdo->n = nBins;
	pfdo->y = &y[hitsStart];
	pfdo->x = &pfdi->x[hitsStart];
	pfdo->ye= pfdi->ye;
	return nBins;
}

