#ifndef _FUNCTION_H
#define _FUNCTION_H
#include <complex.h>

static inline double max(double a, double b){
  return a > b ? a : b;
}

static inline double min(double a, double b){
  return a > b ? b : a;
}

//norm of complex
static inline double cnorm(double complex c){
  double re = creal(c);
  double im = cimag(c);
  return re*re + im*im;
}

#endif
