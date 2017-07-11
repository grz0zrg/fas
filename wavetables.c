#include <stdlib.h>

#include "wavetables.h"

float *sine_wavetable_init(unsigned int wavetable_size) {
    int i = 0;

    float *wt = (float*)calloc(wavetable_size, sizeof(float));

    for (i = 0; i < wavetable_size; i += 1) {
        wt[i] = (float)sin(((double)i/(double)wavetable_size) * 3.141592653589 * 2.0);
    }

    return wt;
}

float *wnoise_wavetable_init(unsigned int wavetable_size, float amount) {
    int i = 0;

    float *wt = (float*)calloc(wavetable_size, sizeof(float));

    for (i = 0; i < wavetable_size; i += 1) {
        wt[i] = 1. + randf(-amount, amount);
    }

    return wt;
}
