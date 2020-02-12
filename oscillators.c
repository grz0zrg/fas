#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "oscillators.h"

struct oscillator *createOscillators(
#ifdef WITH_SOUNDPIPE
    sp_data *spd,
#endif
    unsigned int n,
    double base_frequency,
    unsigned int octaves,
    unsigned int sample_rate,
    unsigned int wavetable_size,
    unsigned int frame_data_count) {
    struct oscillator *oscillators = (struct oscillator*)malloc(n * sizeof(struct oscillator));

    if (oscillators == NULL) {
        printf("createOscillators alloc. error.");
        fflush(stdout);
        return NULL;
    }

    int y = 0, i = 0, k = 0;
    int partials = 0;
    int index = 0;
    double octave_length = (double)n / octaves;
    double frequency;
    double phase_increment;
    uint64_t phase_step;
    int nmo = n - 1;

    double max_frequency = base_frequency * pow(2.0, nmo / octave_length);

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2.0, y / octave_length);
        phase_step = frequency / (double)sample_rate * wavetable_size;
        phase_increment = frequency * 2 * 3.141592653589 / (double)sample_rate;

        struct oscillator *osc = &oscillators[index];

        osc->freq = frequency;

#ifdef FIXED_WAVETABLE
        osc->phase_index = malloc(sizeof(uint16_t) * frame_data_count);
        osc->phase_index2 = malloc(sizeof(uint16_t) * frame_data_count);
#else
        osc->phase_index = malloc(sizeof(unsigned int) * frame_data_count);
        osc->phase_index2 = malloc(sizeof(unsigned int) * frame_data_count);
#endif

        osc->fphase = malloc(sizeof(double) * frame_data_count);

        // subtractive with additive synthesis
#ifndef POLYBLEP
        partials = fmin((max_frequency - frequency) / frequency, 128) + 1;
        osc->max_harmonics = partials;

    #ifdef FIXED_WAVETABLE
        osc->harmo_phase_step = malloc(sizeof(uint16_t) * (partials + 1));
        osc->harmo_phase_index = malloc(sizeof(uint16_t *) * frame_data_count);
    #else
        osc->harmo_phase_step = malloc(sizeof(unsigned int) * (partials + 1));
        osc->harmo_phase_index = malloc(sizeof(unsigned int *) * frame_data_count);
    #endif

        osc->harmonics = malloc(sizeof(float) * ((partials + 1) * 2));

        // == substrative specials
        int tri_sign = -1.0;
        for (i = 0; i <= partials; i += 1) {
            osc->harmo_phase_step[i] = (frequency * (i + 1)) / (double)sample_rate * wavetable_size;
            osc->harmonics[i] = (1.0 / (double)(i + 2.0));
            osc->harmonics[i + partials] = (1.0 / pow((double)(i + 2), 2.0)) * tri_sign;

            if (((i + 1) % 2) == 0) {
                tri_sign = -tri_sign;
            }
        }
#endif
        // ==
        osc->fp1 = malloc(sizeof(double *) * frame_data_count);
        osc->fp2 = malloc(sizeof(double *) * frame_data_count);
        osc->fp3 = malloc(sizeof(double *) * frame_data_count);
        osc->fp4 = malloc(sizeof(double *) * frame_data_count);

        osc->triggered = 0;

        osc->buffer_len = (double)sample_rate / frequency;
        osc->buffer = malloc(sizeof(double) * osc->buffer_len * frame_data_count);

        osc->noise_index = malloc(sizeof(uint16_t) * frame_data_count);

        osc->pvalue = malloc(sizeof(float) * frame_data_count);

#ifdef WITH_SOUNDPIPE
        osc->sp_filters = malloc(sizeof(void **) * frame_data_count);
        osc->sp_gens = malloc(sizeof(void **) * frame_data_count);
#endif

        for (i = 0; i < frame_data_count; i += 1) {
            osc->phase_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->noise_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->pvalue[i] = 0;

#ifdef WITH_SOUNDPIPE
            // Soundpipe filters
            osc->sp_filters[i] = malloc(sizeof(void *) * SP_OSC_FILTERS);

            sp_moogladder_create((sp_moogladder **)&osc->sp_filters[i][SP_MOOG_FILTER]);
            sp_moogladder_init(spd, osc->sp_filters[i][SP_MOOG_FILTER]);

            sp_diode_create((sp_diode **)&osc->sp_filters[i][SP_DIODE_FILTER]);
            sp_diode_init(spd, osc->sp_filters[i][SP_DIODE_FILTER]);

            sp_wpkorg35_create((sp_wpkorg35 **)&osc->sp_filters[i][SP_KORG35_FILTER]);
            sp_wpkorg35_init(spd, osc->sp_filters[i][SP_KORG35_FILTER]);

            sp_streson_create((sp_streson **)&osc->sp_filters[i][SP_STRES_FILTER]);
            sp_streson_init(spd, osc->sp_filters[i][SP_STRES_FILTER]);

            sp_lpf18_create((sp_lpf18 **)&osc->sp_filters[i][SP_LPF18_FILTER]);
            sp_lpf18_init(spd, osc->sp_filters[i][SP_LPF18_FILTER]);

            sp_fofilt_create((sp_fofilt **)&osc->sp_filters[i][SP_FORMANT_FILTER]);
            sp_fofilt_init(spd, osc->sp_filters[i][SP_FORMANT_FILTER]);

            sp_fofilt *fofilt = (sp_fofilt *)osc->sp_filters[i][SP_FORMANT_FILTER];
            fofilt->freq = frequency;

            sp_mode_create((sp_mode **)&osc->sp_filters[i][SP_MODE_FILTER]);
            sp_mode_init(spd, osc->sp_filters[i][SP_MODE_FILTER]);

            sp_mode *mode = (sp_mode *)osc->sp_filters[i][SP_MODE_FILTER];
            mode->freq = frequency;

            sp_vocoder_create((sp_vocoder **)&osc->sp_filters[i][SP_VOCODER_FILTER]);
            sp_vocoder_init(spd, osc->sp_filters[i][SP_VOCODER_FILTER]);

            // Soundpipe generator
            osc->sp_gens[i] = malloc(sizeof(void *) * SP_OSC_GENS);

            sp_noise_create((sp_noise **)&osc->sp_gens[i][SP_WHITE_NOISE_GENERATOR]);
            sp_noise_init(spd, osc->sp_gens[i][SP_WHITE_NOISE_GENERATOR]);

            sp_pinknoise_create((sp_pinknoise **)&osc->sp_gens[i][SP_PINK_NOISE_GENERATOR]);
            sp_pinknoise_init(spd, osc->sp_gens[i][SP_PINK_NOISE_GENERATOR]);

            sp_brown_create((sp_brown **)&osc->sp_gens[i][SP_BROWN_NOISE_GENERATOR]);
            sp_brown_init(spd, osc->sp_gens[i][SP_BROWN_NOISE_GENERATOR]);

            sp_drip_create((sp_drip **)&osc->sp_gens[i][SP_DRIP_GENERATOR]);
            sp_drip_init(spd, osc->sp_gens[i][SP_DRIP_GENERATOR], 0.09f);

            sp_drip *drip = (sp_drip *)osc->sp_gens[i][SP_DRIP_GENERATOR];
            drip->amp = 1.f;
            drip->freq = frequency;

            sp_pdhalf_create((sp_pdhalf **)&osc->sp_gens[i][SP_PD_GENERATOR]);
            sp_pdhalf_init(spd, osc->sp_gens[i][SP_PD_GENERATOR]);

#endif

            // == PM
            osc->phase_index2[i] = rand() / (double)RAND_MAX * wavetable_size;

            osc->fphase[i] = 0;

            // subtractive with additive synthesis
#ifndef POLYBLEP
    #ifdef FIXED_WAVETABLE
            osc->harmo_phase_index[i] = malloc(sizeof(uint16_t) * (partials + 1));
    #else
            osc->harmo_phase_index[i] = malloc(sizeof(unsigned int) * (partials + 1));
    #endif

            for (k = 0; k <= partials; k += 1) {
                osc->harmo_phase_index[i][k] = rand() / (double)RAND_MAX * wavetable_size;
            }
#endif
            // ==

            osc->fp1[i] = calloc(6, sizeof(double));
            osc->fp2[i] = calloc(6, sizeof(double));
            osc->fp3[i] = calloc(6, sizeof(double));
            osc->fp4[i] = calloc(6, sizeof(double));
        }

        osc->phase_step = phase_step;
        osc->phase_increment = phase_increment;
    }

    return oscillators;
}

// TODO : update it with subtractive specials (for now this function is unused and have to be updated to be used properly)
struct oscillator *copyOscillators(struct oscillator **oscs, unsigned int n, unsigned int frame_data_count) {
    struct oscillator *o = *oscs;

    if (o == NULL) {
        return NULL;
    }

    struct oscillator *new_oscillators = (struct oscillator*)malloc(n * sizeof(struct oscillator));

    if (new_oscillators == NULL) {
        printf("copyOscillators alloc. error.");
        fflush(stdout);
        return NULL;
    }

    int y = 0;
    for (y = 0; y < n; y += 1) {
        struct oscillator *new_osc = &new_oscillators[y];

        new_osc->freq = o[y].freq;
        new_osc->phase_step = o[y].phase_step;

        #ifdef FIXED_WAVETABLE
            new_osc->phase_index = malloc(sizeof(uint16_t) * frame_data_count);
        #else
            new_osc->phase_index = malloc(sizeof(unsigned int) * frame_data_count);
        #endif

        new_osc->noise_index = malloc(sizeof(uint16_t) * frame_data_count);
        new_osc->pvalue = malloc(sizeof(float) * frame_data_count);

        for (int i = 0; i < frame_data_count; i += 1) {
            new_osc->phase_index[i] = o[y].phase_index[i];
            new_osc->noise_index[i] = o[y].noise_index[i];
            new_osc->pvalue[i] = o[y].pvalue[i];
        }
    }

    return new_oscillators;
}

struct oscillator *freeOscillators(struct oscillator **o, unsigned int n, unsigned int frame_data_count) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

    int y = 0, i = 0, k = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
        free(oscs[y].fphase);
        free(oscs[y].noise_index);
        free(oscs[y].pvalue);
        free(oscs[y].buffer);

        free(oscs[y].phase_index2);

#ifndef POLYBLEP
        free(oscs[y].harmo_phase_step);
        free(oscs[y].harmonics);
#endif

        for (i = 0; i < frame_data_count; i += 1) {
#ifndef POLYBLEP
            free(oscs[y].harmo_phase_index[i]);
#endif
            free(oscs[y].fp1[i]);
            free(oscs[y].fp2[i]);
            free(oscs[y].fp3[i]);
            free(oscs[y].fp4[i]);
#ifdef WITH_SOUNDPIPE
            sp_moogladder_destroy((sp_moogladder **)&oscs[y].sp_filters[i][SP_MOOG_FILTER]);
            sp_diode_destroy((sp_diode **)&oscs[y].sp_filters[i][SP_DIODE_FILTER]);
            sp_wpkorg35_destroy((sp_wpkorg35 **)&oscs[y].sp_filters[i][SP_KORG35_FILTER]);
            sp_streson_destroy((sp_streson **)&oscs[y].sp_filters[i][SP_STRES_FILTER]);
            sp_lpf18_destroy((sp_lpf18 **)&oscs[y].sp_filters[i][SP_LPF18_FILTER]);
            sp_fofilt_destroy((sp_fofilt **)&oscs[y].sp_filters[i][SP_FORMANT_FILTER]);
            sp_mode_destroy((sp_mode **)&oscs[y].sp_filters[i][SP_MODE_FILTER]);
            sp_vocoder_destroy((sp_vocoder **)&oscs[y].sp_filters[i][SP_VOCODER_FILTER]);

            free(oscs[y].sp_filters[i]);

            sp_noise_destroy((sp_noise **)&oscs[y].sp_gens[i][SP_WHITE_NOISE_GENERATOR]);
            sp_pinknoise_destroy((sp_pinknoise **)&oscs[y].sp_gens[i][SP_PINK_NOISE_GENERATOR]);
            sp_brown_destroy((sp_brown **)&oscs[y].sp_gens[i][SP_BROWN_NOISE_GENERATOR]);
            sp_drip_destroy((sp_drip **)&oscs[y].sp_gens[i][SP_DRIP_GENERATOR]);
            sp_pdhalf_destroy((sp_pdhalf **)&oscs[y].sp_gens[i][SP_PD_GENERATOR]);
            
            free(oscs[y].sp_gens[i]);
#endif
        }

#ifndef POLYBLEP
        free(oscs[y].harmo_phase_index);
#endif

        free(oscs[y].fp1);
        free(oscs[y].fp2);
        free(oscs[y].fp3);
        free(oscs[y].fp4);

#ifdef WITH_SOUNDPIPE
        free(oscs[y].sp_filters);
        free(oscs[y].sp_gens);
#endif
    }

    free(oscs);

    return NULL;
}
