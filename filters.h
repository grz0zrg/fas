#ifndef _FAS_FILTERS_H_
#define _FAS_FILTERS_H_

  #include <math.h>

  extern double improved_moog(double input, double cutoff, double resonance, double drive, double *V, double *dV, double *tV, double sampleRate);

#endif
