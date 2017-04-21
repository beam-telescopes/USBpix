/* Slave - fitXface.h */

/* fitxface.h
 * provides minimal declarations for a typical user to properly interface to 
 * fitting routines
 */

#ifndef FITXFACE_H
#define FITXFACE_H

typedef struct {
  float cstpar, numpar, denompar;
} TotCal;

int fitSCurve   (float *x, float *y, float *yerr, int n, int *nIters, float *coeffs, int flags, float *chi2);
int fitTotCalOld(float *x, float *y, float *yerr, int n, TotCal *calpar, int flags, float *chi2, int *nIters);
int fitTotCalNew(float *x, float *y, float *yerr, int n, TotCal *calpar, int flags, float *chi2, int *nIters);
void connectFitting();
void initFitting(void *workArea);
void setupInternalMemoryImage();
/* flags */
#define VFX_USER_WEIGHTS	0x01
#define VFX_USER_GUESS		0x02
#define VFX_MAX_LIKELIHOOD	0x04
#define VFX_MIN_CHI2		0x08
#define VFX_NO_WEIGHTING	0x10

#endif
