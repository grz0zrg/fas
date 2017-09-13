#include "grains.h"

struct grain *createGrains(struct sample **s, unsigned int samples_count, unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate) {
    if (samples_count == 0) {
        return NULL;
    }

    struct sample *samples = *s;

    struct grain *g = (struct grain *)malloc(n * sizeof(struct grain));
    for (int i = 0; i < n; i += 1) {
        struct sample *smp = &samples[i%samples_count];

        double octave_length = (double)n / octaves;
        double frequency;
        frequency = base_frequency * pow(2, (n-i) / octave_length);

        g[i].frames = 0;//floor(randf(0.001f, 0.02f) * sample_rate); // 1ms - 100ms
        g[i].frame = 0;
        g[i].index = 0;//floor(randf(0, smp->frames - g[i].frames - 1)) * (smp->chn + 1);
        g[i].speed = frequency / smp->pitch;
        if (g[i].speed <= 0) {
            g[i].speed = 1;
        }
        g[i].env_step = FAS_ENVS_SIZE / (g[i].frames / g[i].speed);
        g[i].env_index = 0;
        //g[i].index %= (smp->frames - g[i].frames);
    }

    return g;
}

struct grain *freeGrains(struct grain **g, unsigned int n) {
    struct grain *grains = *g;

    free(grains);

    return NULL;
}
