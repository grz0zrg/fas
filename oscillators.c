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
    double octave_length = n / octaves;
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

        for (int i = 0; i < frame_data_count; i += 1) {
            oscillators[index].phase_index[i] = rand() / (double)RAND_MAX * wavetable_size;
        }

        oscillators[index].phase_step = phase_step;
    }

    return oscillators;
}

struct oscillator *freeOscillators(struct oscillator **o, unsigned int n) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

    int y = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
    }

    free(oscs);

    return NULL;
}
