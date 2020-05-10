#ifndef _FAS_GRAINS_H_
#define _FAS_GRAINS_H_

    #include <stdint.h>

    #include "tools.h"
    #include "samples.h"
    #include "constants.h"

    typedef struct grain grain;

    struct grain {
        FAS_FLOAT *frame; // current sample position
        unsigned int *frames; // duration
        //unsigned int *index; // grain position
        unsigned int *density; // grain density
        unsigned int *smp_index;
        FAS_FLOAT *speed; // sample-based step
        FAS_FLOAT *env_index;
        FAS_FLOAT *env_step;
    };

    extern struct grain *createGrains(struct sample **samples, unsigned int samples_count, unsigned int n, FAS_FLOAT base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int frame_data_count, unsigned int max_density);
    extern void computeGrains(unsigned int channel, struct grain *g, unsigned int grain_index, FAS_FLOAT alpha, unsigned int si, unsigned int density, FAS_FLOAT density_offset, FAS_FLOAT *gr_env, struct sample *samples, unsigned int smp_index, unsigned int sample_rate, FAS_FLOAT min_duration, FAS_FLOAT max_duration, FAS_FLOAT *out_l, FAS_FLOAT *out_r);
    extern struct grain *freeGrains(struct grain **g, unsigned int samples_count, unsigned int frame_data_count, unsigned int n, unsigned int max_density);

#endif
