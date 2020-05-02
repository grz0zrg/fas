#include <stdlib.h>

#include "wavetables.h"

float *sine_wavetable_init(unsigned int wavetable_size) {
    unsigned int i = 0;

    float *wt = (float*)calloc(wavetable_size + 1, sizeof(float));

    for (i = 0; i < wavetable_size; i += 1) {
        wt[i] = (float)sin(((double)i/(double)wavetable_size) * 3.141592653589 * 2.0);
    }
    // accomodate for linear interpolation
    wt[wavetable_size] = wt[0];

    return wt;
}

float *wnoise_wavetable_init(unsigned int wavetable_size, float amount) {
    unsigned int i = 0;

    float *wt = (float*)calloc(wavetable_size, sizeof(float));

    for (i = 0; i < wavetable_size; i += 1) {
        wt[i] = randf(-amount, amount);
    }

    return wt;
}
