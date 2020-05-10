#include <stdlib.h>

#include "wavetables.h"

FAS_FLOAT *sine_wavetable_init(unsigned int wavetable_size) {
    unsigned int i = 0;

    FAS_FLOAT *wt = (FAS_FLOAT*)calloc(wavetable_size + 1, sizeof(FAS_FLOAT));

    for (i = 0; i < wavetable_size; i += 1) {
        wt[i] = (FAS_FLOAT)sin(((double)i/(double)wavetable_size) * 3.141592653589 * 2.0);
    }
    // accomodate for linear interpolation
    wt[wavetable_size] = wt[0];

    return wt;
}

FAS_FLOAT *wnoise_wavetable_init(unsigned int wavetable_size, FAS_FLOAT amount) {
    unsigned int i = 0;

    FAS_FLOAT *wt = (FAS_FLOAT*)calloc(wavetable_size, sizeof(FAS_FLOAT));

    for (i = 0; i < wavetable_size; i += 1) {
        wt[i] = randf(-amount, amount);
    }

    return wt;
}
