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
    int fit = 0;
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
        fit = 0;
        i = frequency;
        while (fit <= 64) {
            fit += 1;

            if ((i * fit) > max_frequency) {
                fit -= 1;

                break;
            }
        }

        osc->max_harmonics = fit;
        // ==

#ifdef FIXED_WAVETABLE
        osc->phase_index = malloc(sizeof(uint16_t) * frame_data_count);
        osc->harmo_phase_step = malloc(sizeof(uint16_t) * fit);
        osc->harmo_phase_index = malloc(sizeof(uint16_t *) * frame_data_count);
#else
        osc->phase_index = malloc(sizeof(unsigned int) * frame_data_count);
        osc->harmo_phase_step = malloc(sizeof(unsigned int) * fit);
        osc->harmo_phase_index = malloc(sizeof(unsigned int *) * frame_data_count);
#endif

        // == substrative specials
        for (i = 0; i < fit; i += 1) {
            osc->harmo_phase_step[i] = (frequency * (i + 1)) / (double)sample_rate * wavetable_size;
        }

        osc->fin = malloc(sizeof(double *) * frame_data_count);
        osc->fout = malloc(sizeof(double *) * frame_data_count);
        // ==

        osc->noise_index = malloc(sizeof(uint16_t) * frame_data_count);

        osc->value = malloc(sizeof(float) * frame_data_count);

        for (i = 0; i < frame_data_count; i += 1) {
            osc->phase_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->noise_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->value[i] = 0;

            // == substrative specials
#ifdef FIXED_WAVETABLE
            osc->harmo_phase_index[i] = malloc(sizeof(uint16_t) * fit);
#else
            osc->harmo_phase_index[i] = malloc(sizeof(unsigned int) * fit);
#endif

            for (k = 0; k < fit; k += 1) {
                osc->harmo_phase_index[i][k] = rand() / (double)RAND_MAX * wavetable_size;
            }

            osc->fin[i] = malloc(sizeof(double) * 4);
            osc->fout[i] = malloc(sizeof(double) * 4);

            for (k = 0; k < 4; k += 1) {
                osc->fin[i][k] = 0;
                osc->fout[i][k] = 0;
            }
            // ==
        }

        osc->phase_step = phase_step;
    }

    return oscillators;
}

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

struct oscillator *freeOscillators(struct oscillator **o, unsigned int n) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

    int y = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
        free(oscs[y].noise_index);
        free(oscs[y].value);
    }

    free(oscs);

    return NULL;
}
