#ifndef _FAS_TOOLS_H_
#define _FAS_TOOLS_H_

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>

    #include "constants.h"

    #include <time.h>
    #include <sys/time.h>

    #define FAS_ENVS_COUNT 13

    extern FAS_FLOAT poly_blep(FAS_FLOAT phase_increment, FAS_FLOAT t);
    FAS_FLOAT raw_waveform(FAS_FLOAT phase, int type);
    
    extern FAS_FLOAT randf(FAS_FLOAT min, FAS_FLOAT max);
    extern FAS_FLOAT **createEnvelopes(unsigned int n);
    extern void freeEnvelopes(FAS_FLOAT **envs);

    extern char *create_filepath(char *directory, char *filename);

    int isPowerOfTwo(unsigned int x);

    double get_time(void);

    FAS_FLOAT lerp(FAS_FLOAT a, FAS_FLOAT b, FAS_FLOAT f);

#endif
