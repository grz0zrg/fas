#ifndef _FAS_OSCILLATORS_H_
#define _FAS_OSCILLATORS_H_

    #include <stdint.h>
    #include <math.h>

#ifdef WITH_SOUNDPIPE
    #include "soundpipe.h"
#endif

#ifdef WITH_FAUST
    #include "faust.h"
#endif

    #include "constants.h"
    #include "tools.h"

    struct oscillator {
        double freq;

        // above + below Hz gap
        double *bw;

        // filters fb
        double **fp1, **fp2, **fp3, **fp4;

#ifndef POLYBLEP
        int max_harmonics;
        float *harmonics;

    #ifdef FIXED_WAVETABLE
        uint16_t **harmo_phase_index;
        uint16_t *harmo_phase_step;
    #else
        unsigned int **harmo_phase_index;
        unsigned int *harmo_phase_step;
    #endif
#endif

#ifdef MAGIC_CIRCLE
        float mc_eps;
        float *mc_x;
        float *mc_y;
#endif

#ifdef FIXED_WAVETABLE
        uint16_t *phase_index;
        uint16_t phase_step;

        // fm/pm
        uint16_t *phase_index2;
#else
        unsigned int *phase_index;
        unsigned int phase_step;

        // fm/pm
        unsigned int *phase_index2;
#endif
        double *fphase;

        float *pvalue;

        double phase_increment;

        double *buffer;
        unsigned int buffer_len;

        unsigned int triggered;

        uint16_t *noise_index;

#ifdef WITH_FAUST
        struct _fas_faust_dsp ***faust_gens;
        size_t faust_gens_len;
#endif

#ifdef WITH_SOUNDPIPE
        void ***sp_filters;
        void ***sp_gens;
        void ***sp_mods;

        sp_ftbl *ft_void;
#endif
    };

    extern struct oscillator *createOscillators(
#ifdef WITH_SOUNDPIPE
        sp_data *spd,
#endif
#ifdef WITH_FAUST
        struct _faust_factories *faust_factories,
#endif
        unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int wavetable_size, unsigned int frame_data_count);
        
    extern struct oscillator *freeOscillators(struct oscillator **oscs, unsigned int n, unsigned int frame_data_count);

    extern struct oscillator *copyOscillators(struct oscillator **oscs, unsigned int n, unsigned int frame_data_count);

#endif
