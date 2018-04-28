#ifndef _FAS_SAMPLES_H_
#define _FAS_SAMPLES_H_

  #include <math.h>
  #include "inc/samplerate.h"
  #include "inc/Yin.h"

    struct sample {
        float *data; // unused after load
        float *data_l;
        float *data_r;
        uint32_t len;
        uint32_t frames;
        unsigned int chn;
        unsigned int chn_m1;
        double pitch; // hz
        int samplerate;
    };

    extern unsigned int load_waves(struct sample **waves, char* directory);
    extern unsigned int load_samples(struct sample **samples, char *directory, unsigned int sample_rate, int converter_type, int smooth_end);
    extern void free_samples(struct sample **s, unsigned int samples_count);

#endif
