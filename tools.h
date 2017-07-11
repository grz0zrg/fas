#ifndef _FAS_TOOLS_H_
#define _FAS_TOOLS_H_

    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>

    #include "constants.h"

    #define FAS_ENVS_COUNT 9

    extern float randf(float min, float max);
    extern float **createEnvelopes(unsigned int n);
    extern void freeEnvelopes(float **envs);

#endif
