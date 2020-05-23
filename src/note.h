#ifndef _FAS_NOTE_H_
#define _FAS_NOTE_H_

  #include <math.h>
  #include "constants.h"
  #include "oscillators.h"

    // hold notes data, some are pre-computed for specific type of sound synthesis
    struct note {
        unsigned int osc_index;

        // common to all synthesis type
        FAS_FLOAT previous_volume_l;
        FAS_FLOAT previous_volume_r;
        FAS_FLOAT diff_volume_l;
        FAS_FLOAT diff_volume_r;

        // generic
        FAS_FLOAT volume_l; // red (mono = alpha)
        FAS_FLOAT volume_r; // green (mono = alpha)
        FAS_FLOAT blue;
        FAS_FLOAT pblue;
        FAS_FLOAT alpha;
        FAS_FLOAT palpha;

        // subtractive related
        FAS_FLOAT cutoff;
        FAS_FLOAT res;

        // granular related
        unsigned int density;

        FAS_FLOAT norm_density;

        unsigned int smp_index;
        unsigned int psmp_index;

        unsigned int wav_index;
        unsigned int pwav_index;
    };

    extern void fillNotesBuffer(unsigned int samples_count, unsigned int waves_count, unsigned int max_density,
                                unsigned int channels, unsigned int data_frame_size, struct note *note_buffer,
                                unsigned int h, size_t data_length, void *prev_data, void *data);

#endif
