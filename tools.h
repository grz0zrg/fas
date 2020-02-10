#ifndef _FAS_TOOLS_H_
#define _FAS_TOOLS_H_

    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>

    #include "constants.h"

    #define FAS_ENVS_COUNT 13

    extern double poly_blep(double phase_increment, double t);
    double raw_waveform(double phase, int type);
    
    extern float randf(float min, float max);
    extern float **createEnvelopes(unsigned int n);
    extern void freeEnvelopes(float **envs);

    int isPowerOfTwo(unsigned int x);

#endif
