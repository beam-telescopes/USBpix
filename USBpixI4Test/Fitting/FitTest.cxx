#include "FitClass.h"
#include <math.h>
#include <stdio.h>

int main(int, char**){
  double pars[7]={0.,1/sqrt(2.), 1., 0., 1., 0.,0.}, xv[1];
  FitClass *fc = new FitClass();
  for(double x = -5.; x<5.; x+=.01){
    xv[0] = x;
    printf("%lf %lf\n", x, fc->erfcfun(xv,pars));
  }
  return 0;
}
