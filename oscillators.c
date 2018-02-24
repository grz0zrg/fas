#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "oscillators.h"

struct oscillator *createOscillators(unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int wavetable_size, unsigned int frame_data_count) {
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
    uint64_t phase_step;
    int nmo = n - 1;

    double max_frequency = base_frequency * pow(2.0, nmo / octave_length);

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2.0, y / octave_length);
        phase_step = frequency / (double)sample_rate * wavetable_size;

        struct oscillator *osc = &oscillators[index];

        osc->freq = frequency;

        // == substrative specials
        partials = fmin((max_frequency - frequency) / frequency, 128) + 1;

        osc->max_harmonics = partials;
        // ==

#ifdef FIXED_WAVETABLE
        osc->phase_index = malloc(sizeof(uint16_t) * frame_data_count);
        osc->harmo_phase_step = malloc(sizeof(uint16_t) * (partials + 1));
        osc->harmo_phase_index = malloc(sizeof(uint16_t *) * frame_data_count);

        osc->phase_index2 = malloc(sizeof(uint16_t) * frame_data_count);
#else
        osc->phase_index = malloc(sizeof(unsigned int) * frame_data_count);
        osc->harmo_phase_step = malloc(sizeof(unsigned int) * (partials + 1));
        osc->harmo_phase_index = malloc(sizeof(unsigned int *) * frame_data_count);

        osc->phase_index2 = malloc(sizeof(unsigned int) * frame_data_count);
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
        osc->fp1 = malloc(sizeof(double *) * frame_data_count);
        osc->fp2 = malloc(sizeof(double *) * frame_data_count);
        osc->fp3 = malloc(sizeof(double *) * frame_data_count);
        // ==

        osc->noise_index = malloc(sizeof(uint16_t) * frame_data_count);

        osc->value = malloc(sizeof(float) * frame_data_count);

        for (i = 0; i < frame_data_count; i += 1) {
            osc->phase_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->noise_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->value[i] = 0;

            // == PM
            osc->phase_index2[i] = rand() / (double)RAND_MAX * wavetable_size;

            // == substrative specials
#ifdef FIXED_WAVETABLE
            osc->harmo_phase_index[i] = malloc(sizeof(uint16_t) * (partials + 1));
#else
            osc->harmo_phase_index[i] = malloc(sizeof(unsigned int) * (partials + 1));
#endif

            for (k = 0; k <= partials; k += 1) {
                osc->harmo_phase_index[i][k] = rand() / (double)RAND_MAX * wavetable_size;
            }

            osc->fp1[i] = calloc(6, sizeof(double));
            osc->fp2[i] = calloc(6, sizeof(double));
            osc->fp3[i] = calloc(6, sizeof(double));
            // ==
        }

        osc->phase_step = phase_step;
    }

    return oscillators;
}

// TODO : update it with subtractive specials (for now this function is unused but can be useful)
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
        new_osc->value = malloc(sizeof(float) * frame_data_count);

        for (int i = 0; i < frame_data_count; i += 1) {
            new_osc->phase_index[i] = o[y].phase_index[i];
            new_osc->noise_index[i] = o[y].noise_index[i];
            new_osc->value[i] = o[y].value[i];
        }
    }

    return new_oscillators;
}

struct oscillator *freeOscillators(struct oscillator **o, unsigned int n, unsigned int frame_data_count) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

    int y = 0, i = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
        free(oscs[y].noise_index);
        free(oscs[y].value);
        free(oscs[y].harmo_phase_step);

        free(oscs[y].phase_index2);
        free(oscs[y].harmonics);

        for (i = 0; i < frame_data_count; i += 1) {
            free(oscs[y].harmo_phase_index[i]);
            free(oscs[y].fp1[i]);
            free(oscs[y].fp2[i]);
            free(oscs[y].fp3[i]);
        }

        free(oscs[y].harmo_phase_index);

        free(oscs[y].fp1);
        free(oscs[y].fp2);
        free(oscs[y].fp3);
    }

    free(oscs);

    return NULL;
}
