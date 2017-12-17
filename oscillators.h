#ifndef _FAS_OSCILLATORS_H_
#define _FAS_OSCILLATORS_H_

    #include <stdint.h>

    struct oscillator {
        double freq;

        int max_harmonics;

        // filters fb
        double **fp1, **fp2, **fp3;

#ifdef FIXED_WAVETABLE
        uint16_t **harmo_phase_index;
        uint16_t *harmo_phase_step;

        uint16_t *phase_index;
        uint16_t phase_step;

        // fm/pm
        uint16_t *phase_index2;
#else
        unsigned int **harmo_phase_index;
        unsigned int *harmo_phase_step;

        unsigned int *phase_index;
        unsigned int phase_step;

        // fm/pm
        unsigned int *phase_index2;
#endif

        float *harmonics;

        float *value;

        uint16_t *noise_index;
    };

    extern struct oscillator *createOscillators(unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int wavetable_size, unsigned int frame_data_count);
    extern struct oscillator *freeOscillators(struct oscillator **oscs, unsigned int n);

    extern struct oscillator *copyOscillators(struct oscillator **oscs, unsigned int n, unsigned int frame_data_count);

#endif
