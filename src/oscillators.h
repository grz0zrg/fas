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
        FAS_FLOAT freq;
        FAS_FLOAT prev_freq;
        FAS_FLOAT next_freq;

        // bandwidth Hz
        FAS_FLOAT *bw;

        // generic parameters storage (initially used for filter parameters)
        FAS_FLOAT **fp1, **fp2, **fp3, **fp4;

        // MCF recursive algorithm for sinewave oscillator
#ifdef MAGIC_CIRCLE
        FAS_FLOAT mc_eps;
        FAS_FLOAT *mc_x;
        FAS_FLOAT *mc_y;
#endif

        // wavetable related oscillator
        FAS_FLOAT *phase_index;
        FAS_FLOAT phase_step;

        // fm/pm; modulator
        FAS_FLOAT *phase_index2;

        // noise wavetable index
        uint16_t *noise_index;

        // floating-point phase (for PolyBLEP subtractive waveforms / physical modelling) TODO : use wavetable phase (since we dropped integer based phase)
        FAS_FLOAT *fphase;
        FAS_FLOAT phase_increment;

        // generic parameter which generally represent a previous value
        FAS_FLOAT *pvalue;

        // for physical modelling (Karplus-Strong state table)
        FAS_FLOAT *buffer;
        unsigned int buffer_len;

        // trigger state; wether oscillator has been triggered
        unsigned int *triggered;

        // unallocated wave
        FAS_FLOAT **wav1, **wav2;

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
     * each oscillators in the bank may have additional per instrument parameters defined by frame_data_count
     **/
    extern struct oscillator *createOscillatorsBank(
#ifdef WITH_SOUNDPIPE
        sp_data *spd,
#endif
        unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int wavetable_size, unsigned int frame_data_count);

    struct oscillator *updateOscillatorBank(
    #ifdef WITH_SOUNDPIPE
        sp_data *spd,
    #endif
        struct oscillator **o,
        unsigned int n,
        unsigned int frame_data_count,
        unsigned int sample_rate,
        int target,
        FAS_FLOAT value1,
        FAS_FLOAT value2);

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
