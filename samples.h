#ifndef _FAS_SAMPLES_H_
#define _FAS_SAMPLES_H_

  #include <math.h>

#ifdef WITH_SOUNDPIPE
  #include "soundpipe.h"
#endif

  #include "inc/samplerate.h"
  #include "lib/Yin.h"

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

#ifdef WITH_SOUNDPIPE
        sp_ftbl *ftbl;
#endif
    };

    extern unsigned int load_waves(struct sample **waves, char* directory);
    extern unsigned int load_samples(
#ifdef WITH_SOUNDPIPE
        sp_data *sp,
#endif
      struct sample **samples,
      char *directory,
      unsigned int sample_rate,
      int converter_type,
      int pitch_detection,
      int smooth_end);
    extern void free_samples(struct sample **s, unsigned int samples_count);

#endif
