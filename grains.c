#include "grains.h"

// setup granular synthesis grains, this also setup a maximum of "sub grains" with the max_density parameter
struct grain *createGrains(struct sample **s, unsigned int samples_count, unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int frame_data_count, unsigned int max_density) {
    if (samples_count == 0) {
        return NULL;
    }

    unsigned int grains_count = n * max_density;
    unsigned int y = 0;

    double octave_length = (double)n / octaves;

    struct sample *samples = *s;

    struct grain *g = (struct grain *)malloc(grains_count * sizeof(struct grain));
    for (int i = 0; i < grains_count; i += 1) {
        y = i%n;

        struct sample *smp = &samples[y%samples_count];

        double frequency = base_frequency * pow(2, (n-y) / octave_length);

        g[i].speed = frequency / (smp->pitch * (sample_rate / smp->samplerate));
        if (g[i].speed <= 0) {
            g[i].speed = 1;
        }

        g[i].frame = malloc(sizeof(float) * frame_data_count);
        g[i].frames = malloc(sizeof(unsigned int) * frame_data_count);
        g[i].index = malloc(sizeof(unsigned int) * frame_data_count);
        g[i].env_index = malloc(sizeof(uint16_t) * frame_data_count);
        g[i].env_step = malloc(sizeof(uint16_t) * frame_data_count);
        g[i].smp_index = malloc(sizeof(unsigned int) * frame_data_count);
        g[i].density = malloc(sizeof(unsigned int) * frame_data_count);

        // we setup these for each simultaneous channels that we will have
        for (int j = 0; j < frame_data_count; j += 1) {
            g[i].frames[j] = 0;
            g[i].frame[j] = 0;
            g[i].index[j] = 0;
            g[i].env_step[j] = FAS_ENVS_SIZE / (g[i].frames[j] / g[i].speed);
            g[i].env_index[j] = 0;
            g[i].smp_index[j] = 0;
            g[i].density[j] = 1;
        }
    }

    return g;
}

struct grain *freeGrains(struct grain **g, unsigned int n, unsigned int max_density) {
    struct grain *grains = *g;

    unsigned int grains_count = n * max_density;

    int y = 0;
    for (y = 0; y < grains_count; y += 1) {
        free(grains[y].frame);
        free(grains[y].frames);
        free(grains[y].index);
        free(grains[y].env_index);
        free(grains[y].env_step);
        free(grains[y].smp_index);
        free(grains[y].density);
    }

    free(grains);

    return NULL;
}
