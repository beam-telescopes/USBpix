#ifndef __DSP_MATH_H__
#define __DSP_MATH_H__



#if defined(__GNUC__)
#include <math.h>
#endif


// use intrinsic assembler
#if ! defined(__GNUC__)
extern far float rsqrtf(float x); 
#define fabs _fabs
#define fabsf _fabsf
#define sqrtf(x) ((float) 1./(float) rsqrtf(x))
#endif

#endif
