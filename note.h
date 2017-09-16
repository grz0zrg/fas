#ifndef _FAS_NOTE_H_
#define _FAS_NOTE_H_

  #include <math.h>
  #include "oscillators.h"

    struct note {
        unsigned int osc_index;
        float previous_volume_l;
        float previous_volume_r;
        float volume_l;
        float volume_r;
        float diff_volume_l;
        float diff_volume_r;
        float noise_multiplier;
        float previous_a;
        float diff_a;
        float blue;
        float alpha;

        float *mag;
        float *frq;
        float *pha;

        unsigned int smp_index;
    };

    extern void fillNotesBuffer(unsigned int samples_count, unsigned int channels, unsigned int data_frame_size, struct note *note_buffer, unsigned int h, struct oscillator **o, size_t data_length, void *prev_data, void *data);

#endif
