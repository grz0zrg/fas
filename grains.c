#include "grains.h"

// granular synthesis : grains setup
// all possible grains (and sub-grains from max_density parameter) are pre-computed in memory
struct grain *createGrains(struct sample **s, unsigned int samples_count, unsigned int n, double base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int frame_data_count, unsigned int max_density) {
    if (samples_count == 0) {
        return NULL;
    }

    unsigned int grains_count = n * max_density * samples_count;
    unsigned int y = 0;

    double octave_length = (double)n / octaves;

    struct sample *samples = *s;

    struct grain *g = (struct grain *)calloc(grains_count, sizeof(struct grain));
    for (int i = 0; i < grains_count; i += samples_count) {
        for (int k = 0; k < samples_count; k += 1) {
            int gr_index = i + k;

            struct sample *smp = &samples[k];

            double frequency = base_frequency * pow(2, (n-y) / octave_length);

            g[gr_index].speed = frequency / smp->pitch / ((double)sample_rate / (double)smp->samplerate);

            if (g[gr_index].speed <= 0) {
                g[gr_index].speed = 1;
            }

            // channels dependent parameters
            g[gr_index].frame = calloc(frame_data_count, sizeof(double));
            g[gr_index].frames = calloc(frame_data_count, sizeof(unsigned int));
            g[gr_index].index = calloc(frame_data_count, sizeof(unsigned int));
            g[gr_index].env_index = calloc(frame_data_count, sizeof(double));
            g[gr_index].env_step = calloc(frame_data_count, sizeof(double));
            g[gr_index].smp_index = calloc(frame_data_count, sizeof(unsigned int));
            g[gr_index].density = calloc(frame_data_count, sizeof(unsigned int));

            // initialization for each simultaneous channels
            for (int j = 0; j < frame_data_count; j += 1) {
                g[gr_index].env_index[j] = FAS_ENVS_SIZE;
                g[gr_index].smp_index[j] = k;
                g[gr_index].density[j] = 1;
            }
        }
        y++;
        y = y % n;
    }

    return g;
}

struct grain *freeGrains(struct grain **g, unsigned int samples_count, unsigned int n, unsigned int max_density) {
    struct grain *grains = *g;

    unsigned int grains_count = n * max_density * samples_count;

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
