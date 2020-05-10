#ifndef _FAS_FILTERS_H_
#define _FAS_FILTERS_H_

  #include <math.h>
  #include "constants.h"

  FAS_FLOAT fast_tanh(const FAS_FLOAT x);

  extern FAS_FLOAT huovilainen_moog(FAS_FLOAT input, FAS_FLOAT cutoff_computed, FAS_FLOAT res_computed, FAS_FLOAT *delay, FAS_FLOAT *stage, FAS_FLOAT *stageTanh, int oversample);
  extern void huovilainen_compute(FAS_FLOAT cutoff, FAS_FLOAT resonance, FAS_FLOAT *cutoff_computed, FAS_FLOAT *res_computed, FAS_FLOAT sampleRate);

#endif
