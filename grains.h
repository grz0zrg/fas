#ifndef _FAS_GRAINS_H_
#define _FAS_GRAINS_H_

    #include <stdint.h>

    #include "tools.h"
    #include "samples.h"
    #include "constants.h"

    typedef struct grain grain;

    struct grain {
        float *frame; // current position
        unsigned int *frames; // duration
        unsigned int *index; // grain position
        unsigned int *density; // grain density
        unsigned int *smp_index;
        float speed;
        uint16_t *env_index;
        uint16_t *env_step;
    };

    extern struct grain *createGrains(struct sample **samples, unsigned int samples_count, unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int frame_data_count, unsigned int max_density);
    extern struct grain *freeGrains(struct grain **g, unsigned int samples_count, unsigned int n, unsigned int max_density);

#endif
