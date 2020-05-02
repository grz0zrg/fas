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

    extern double poly_blep(double phase_increment, double t);
    double raw_waveform(double phase, int type);
    
    extern float randf(float min, float max);
    extern float **createEnvelopes(unsigned int n);
    extern void freeEnvelopes(float **envs);

    extern char *create_filepath(char *directory, char *filename);

    int isPowerOfTwo(unsigned int x);

    double get_time(void);

    double lerp(double a, double b, double f);

#endif
