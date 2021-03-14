#include "grains.h"

// granular synthesis : grains setup
// all possible grains (and sub-grains from max_density parameter) are pre-computed in memory
struct grain *createGrains(struct sample **s, unsigned int samples_count, unsigned int n, FAS_FLOAT base_frequency, unsigned int octaves, unsigned int sample_rate, unsigned int max_instruments, unsigned int max_density) {
    if (samples_count == 0) {
        return NULL;
    }

    unsigned int grains_count = n * max_density * samples_count;
    unsigned int y = 0;

    FAS_FLOAT octave_length = (FAS_FLOAT)n / octaves;

    struct sample *samples = *s;

    struct grain *g = (struct grain *)calloc(grains_count, sizeof(struct grain));
    for (unsigned int i = 0; i < grains_count; i += samples_count) {
        for (unsigned int k = 0; k < samples_count; k += 1) {
            int gr_index = i + k;

            struct sample *smp = &samples[k];

            FAS_FLOAT frequency = base_frequency * pow(2, (n-y) / octave_length);

            FAS_FLOAT grain_speed = frequency / smp->pitch / ((FAS_FLOAT)sample_rate / (FAS_FLOAT)smp->samplerate);

            if (grain_speed <= 0) {
                grain_speed = 1;
            }

            // channels dependent parameters
            g[gr_index].frame = calloc(max_instruments, sizeof(FAS_FLOAT));
            g[gr_index].frames = calloc(max_instruments, sizeof(unsigned int));
            //g[gr_index].index = calloc(max_instruments, sizeof(unsigned int));
            g[gr_index].env_index = calloc(max_instruments, sizeof(FAS_FLOAT));
            g[gr_index].env_step = calloc(max_instruments, sizeof(FAS_FLOAT));
            g[gr_index].smp_index = calloc(max_instruments, sizeof(unsigned int));
            g[gr_index].density = calloc(max_instruments, sizeof(unsigned int));
            g[gr_index].speed = calloc(max_instruments, sizeof(FAS_FLOAT));

            // initialization for each simultaneous channels
            for (unsigned int j = 0; j < max_instruments; j += 1) {
                g[gr_index].env_index[j] = FAS_ENVS_SIZE;
                g[gr_index].smp_index[j] = k;
                g[gr_index].density[j] = 1;
                g[gr_index].speed[j] = grain_speed;
            }
        }
        y++;
        y = y % n;
    }

    return g;
}

inline void computeGrains(unsigned int channel, struct grain *g, unsigned int grain_index, FAS_FLOAT alpha, unsigned int si, unsigned int density, FAS_FLOAT density_offset, FAS_FLOAT *gr_env, struct sample *samples, unsigned int smp_index, unsigned int sample_rate, FAS_FLOAT min_duration, FAS_FLOAT max_duration, FAS_FLOAT *out_l, FAS_FLOAT *out_r) {
    struct grain *gr = &g[grain_index];
    struct sample *smp = &samples[smp_index];

    //unsigned int density_start = density_offset * si;

    for (unsigned int d = /*density_start*/0; d < (/*density_start + */si * gr->density[channel]); d += si) {
        gr = &g[grain_index + d];

        FAS_FLOAT env_index = gr->env_index[channel];

        if (env_index >= FAS_ENVS_SIZE) {
            FAS_FLOAT gr_speed = gr->speed[channel];

            FAS_FLOAT grain_start = (FAS_FLOAT)smp->frames - 1.0f;
            FAS_FLOAT grain_position = fabs(alpha);
            grain_start = roundf(grain_start * fmax(fmin(grain_position, 1.0f), 0.0f) + (grain_start * (density_offset * randf(0.0f, 1.0f))));//(1.0f - randf(0.0f, 1.0f) * floor(fmin(grain_position - 0.0001f, 1.0f))));

            //gr->index[k] = grain_start;
            // old algorithm (percent of sample length, no cycles)
            // roundf(grain_start + fmax(randf(GRAIN_MIN_DURATION + min_duration, max_duration), GRAIN_MIN_DURATION) * (smp->frames - grain_start - 1)) + 1;
            gr->frames[channel] = fmax(roundf(fmax(randf(GRAIN_MIN_DURATION + min_duration, max_duration), GRAIN_MIN_DURATION) * (FAS_FLOAT)sample_rate), 0.00000001);
            gr->env_step[channel] = fmax(((FAS_FLOAT)(FAS_ENVS_SIZE)) / (((FAS_FLOAT)gr->frames[channel]/* - (FAS_FLOAT)grain_start*/) / fabs(gr_speed)), 0.00000001);
            gr->env_index[channel] = 0.0f;
            gr->density[channel] = density;

            env_index = 0.0f;

            if (alpha < 0.0f) {
                if (gr_speed > 0.0f) {
                    gr->speed[channel] = -gr_speed;
                }

                gr->frame[channel] = grain_start + gr->frames[channel];
            } else {
                gr->speed[channel] = fabs(gr_speed);

                gr->frame[channel] = grain_start + gr->frames[channel];
                if (gr->frame[channel] > ((FAS_FLOAT)smp->frames - 1.0f)) {
                    gr->frame[channel] -= ((FAS_FLOAT)smp->frames - 1.0f) + 1;
                }
            }
        }

        FAS_FLOAT pos = gr->frame[channel];

        unsigned int sample_index = ((unsigned int)pos) % smp->frames;
        unsigned int sample_index2 = sample_index + 1;

        FAS_FLOAT smp_l = smp->data_l[sample_index];
        FAS_FLOAT smp_r = smp->data_r[sample_index];

        FAS_FLOAT smp_l2 = smp->data_l[sample_index2];
        FAS_FLOAT smp_r2 = smp->data_r[sample_index2];

        FAS_FLOAT mu = pos - (FAS_FLOAT)sample_index;

#ifdef FAS_USE_CUBIC_INTERP
        unsigned int sample_index3 = sample_index2 + 1;
        unsigned int sample_index4 = sample_index3 + 1;

        FAS_FLOAT smp_l3 = smp->data_l[sample_index3];
        FAS_FLOAT smp_r3 = smp->data_r[sample_index3];

        FAS_FLOAT smp_l4 = smp->data_l[sample_index4];
        FAS_FLOAT smp_r4 = smp->data_r[sample_index4];

        FAS_FLOAT smp_lv = smp_l2 + 0.5 * mu*(smp_l3 - smp_l + mu*(2.0*smp_l - 5.0*smp_l2 + 4.0*smp_l3 - smp_l4 + mu*(3.0*(smp_l2 - smp_l3) + smp_l4 - smp_l)));
        FAS_FLOAT smp_rv = smp_r2 + 0.5 * mu*(smp_r3 - smp_r + mu*(2.0*smp_r - 5.0*smp_r2 + 4.0*smp_r3 - smp_r4 + mu*(3.0*(smp_r2 - smp_r3) + smp_r4 - smp_r)));
#else
        FAS_FLOAT smp_lv = smp_l + mu * (smp_l2 - smp_l);
        FAS_FLOAT smp_rv = smp_r + mu * (smp_r2 - smp_r);
#endif

        FAS_FLOAT env = gr_env[(unsigned int)roundf(env_index)];

        *out_l += smp_lv * env;
        *out_r += smp_rv * env;

        gr->frame[channel] += gr->speed[channel];

        if (gr->frame[channel] < 0) {
            gr->frame[channel] = ((FAS_FLOAT)smp->frames - 1.0f);
        }

        if (gr->frame[channel] > ((FAS_FLOAT)smp->frames - 1.0f)) {
            gr->frame[channel] = 0;
        }

        gr->env_index[channel] += gr->env_step[channel];
    }
}

struct grain *freeGrains(struct grain **g, unsigned int samples_count, unsigned int max_instruments, unsigned int n, unsigned int max_density) {
    if (samples_count == 0) {
        return NULL;
    }

    struct grain *grains = *g;

    if (grains == NULL) {
        return NULL;
    }

    unsigned int grains_count = n * max_density * samples_count;

    unsigned int y = 0;
    for (y = 0; y < grains_count; y += samples_count) {
        for (unsigned int k = 0; k < samples_count; k += 1) {
            int gr_index = y + k;

            free(grains[gr_index].frame);
            free(grains[gr_index].frames);
            //free(grains[gr_index].index);
            free(grains[gr_index].env_index);
            free(grains[gr_index].env_step);
            free(grains[gr_index].smp_index);
            free(grains[gr_index].density);
            free(grains[gr_index].speed);
        }
    }

    free(grains);

    return NULL;
}
