#ifndef _FAS_GRAINS_H_
#define _FAS_GRAINS_H_

    #include <stdint.h>

    #include "tools.h"
    #include "samples.h"
    #include "constants.h"

    struct grain {
        float frame; // current position
        unsigned int frames; // duration
        unsigned int index; // grain position
        unsigned int env_type; // envelope type
        float speed;
        float env_index;
        float env_step;
    };

    extern struct grain *createGrains(struct sample **samples, unsigned int samples_count, unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate);
    extern struct grain *freeGrains(struct grain **g, unsigned int n);

#endif
