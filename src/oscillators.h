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
        // frequency Hz
        double freq;
        double prev_freq;
        double next_freq;

        // bandwidth Hz
        double *bw;

        // generic parameters storage (initially used for filter parameters)
        double **fp1, **fp2, **fp3, **fp4;

        // sub bank of oscillators to generate additively synthesized waveforms (for subtractive synthesis)
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

        // MCF recursive algorithm for sinewave oscillator
#ifdef MAGIC_CIRCLE
        float mc_eps;
        float *mc_x;
        float *mc_y;
#endif

        // wavetable related oscillator
#ifdef FIXED_WAVETABLE
        uint16_t *phase_index;
        uint16_t phase_step;

        // fm/pm; modulator
        uint16_t *phase_index2;
#else
        unsigned int *phase_index;
        unsigned int phase_step;

        // fm/pm; modulator
        unsigned int *phase_index2;
#endif

        // noise wavetable index
        uint16_t *noise_index;

        // floating-point phase (for PolyBLEP subtractive waveforms / physical modelling)
        double *fphase;
        double phase_increment;

        // generic parameter which generally represent a previous value
        float *pvalue;

        // for physical modelling (Karplus-Strong state table)
        double *buffer;
        unsigned int buffer_len;

        // trigger state; wether oscillator has been triggered
        unsigned int *triggered;

        // Faust generators
#ifdef WITH_FAUST
        struct _fas_faust_dsp ***faust_gens;
        size_t faust_gens_len;
#endif

        // Soundpipe generators/modifiers/filters
#ifdef WITH_SOUNDPIPE
        void ***sp_filters;
        void ***sp_gens;
        void ***sp_mods;

        sp_ftbl *ft_void;
#endif
    };

    /**
     * create an oscillator bank of N oscillators with a frequencies map defined by f(y) = base_frequency * (2 ^ (y / (n / octaves)))
     * each oscillators in the bank may have additional per channels parameters (frame_data_count is the total number of channels)
     **/
    extern struct oscillator *createOscillatorsBank(
#ifdef WITH_SOUNDPIPE
        sp_data *spd,
#endif
        unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int wavetable_size, unsigned int frame_data_count);
        
    extern struct oscillator *freeOscillatorsBank(struct oscillator **oscs, unsigned int n, unsigned int frame_data_count);

#ifdef WITH_FAUST
    extern void createFaustGenerators(
        struct _faust_factories *faust_factories,
        struct oscillator *osc_bank,
        unsigned int n,
        unsigned int sample_rate,
        unsigned int frame_data_count);

    extern void freeFaustGenerators(
        struct oscillator **o,
        unsigned int n,
        unsigned int frame_data_count);
#endif
#endif
