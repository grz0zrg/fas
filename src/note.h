#ifndef _FAS_NOTE_H_
#define _FAS_NOTE_H_

  #include <math.h>
  #include "constants.h"
  #include "oscillators.h"

    // hold notes data, some are pre-computed for specific type of sound synthesis
    struct note {
        unsigned int osc_index;

        // common to all synthesis type
        float previous_volume_l;
        float previous_volume_r;
        float diff_volume_l;
        float diff_volume_r;

        // generic
        float volume_l; // red (mono = alpha)
        float volume_r; // green (mono = alpha)
        float blue;
        float alpha;
        float palpha;

        float blue_frac_part;

        // subtractive related
        double cutoff;
        double res;

#ifndef POLYBLEP
        unsigned int waveform;
        double exp;
#endif

        // granular related
        unsigned int density;

        float norm_density;

        unsigned int smp_index;
        unsigned int psmp_index;

        unsigned int wav_index;
        unsigned int pwav_index;
    };

    extern void fillNotesBuffer(unsigned int samples_count, unsigned int waves_count, unsigned int max_density,
                                unsigned int channels, unsigned int data_frame_size, struct note *note_buffer,
                                unsigned int h, size_t data_length, void *prev_data, void *data);

#endif
