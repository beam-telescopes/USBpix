/* fitXface.h
 * provides minimal declarations for a typical user to properly interface to 
 * fitting routines
 */

typedef struct {
  double a0, mu, sigma;
} SCurve;

typedef struct {
  double cstpar, numpar, denompar;
} ToTcal;

int fitSCurve(double *x, double *y, int n, SCurve *curve, double *chi2, int flags);
int fitToTcal(double *x, double *y, double *yerr, int n, ToTcal *calpar, double *chi2, int flags);

/* flags */
#define MA_USER_WEIGHTS		0x01
#define MA_USER_GUESS		0x02
#define MA_MAX_LIKELIHOOD       0x04
#define MA_MIN_CHI2		0x08
#define MA_NO_WEIGHTING		0x10
