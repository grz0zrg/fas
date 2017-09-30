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

    int y = 0;
    int index = 0;
    double octave_length = (double)n / octaves;
    double frequency;
    uint64_t phase_step;
    int nmo = n - 1;

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2, y / octave_length);
        phase_step = frequency / (double)sample_rate * wavetable_size;

        oscillators[index].freq = frequency;

#ifdef FIXED_WAVETABLE
        oscillators[index].phase_index = malloc(sizeof(uint16_t) * frame_data_count);
#else
        oscillators[index].phase_index = malloc(sizeof(unsigned int) * frame_data_count);
#endif

        oscillators[index].noise_index = malloc(sizeof(uint16_t) * frame_data_count);

        oscillators[index].value = malloc(sizeof(float) * frame_data_count);

        for (int i = 0; i < frame_data_count; i += 1) {
            oscillators[index].phase_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            oscillators[index].noise_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            oscillators[index].value[i] = 0;
        }

        oscillators[index].phase_step = phase_step;
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
        new_oscillators[y].freq = o[y].freq;
        new_oscillators[y].phase_step = o[y].phase_step;

        #ifdef FIXED_WAVETABLE
            new_oscillators[y].phase_index = malloc(sizeof(uint16_t) * frame_data_count);
        #else
            new_oscillators[y].phase_index = malloc(sizeof(unsigned int) * frame_data_count);
        #endif

        new_oscillators[y].noise_index = malloc(sizeof(uint16_t) * frame_data_count);
        new_oscillators[y].value = malloc(sizeof(float) * frame_data_count);

        for (int i = 0; i < frame_data_count; i += 1) {
            new_oscillators[y].phase_index[i] = o[y].phase_index[i];
            new_oscillators[y].noise_index[i] = o[y].noise_index[i];
            new_oscillators[y].value[i] = o[y].value[i];
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
