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

            double grain_speed = frequency / smp->pitch / ((double)sample_rate / (double)smp->samplerate);

            if (grain_speed <= 0) {
                grain_speed = 1;
            }

            // channels dependent parameters
            g[gr_index].frame = calloc(frame_data_count, sizeof(double));
            g[gr_index].frames = calloc(frame_data_count, sizeof(unsigned int));
            //g[gr_index].index = calloc(frame_data_count, sizeof(unsigned int));
            g[gr_index].env_index = calloc(frame_data_count, sizeof(double));
            g[gr_index].env_step = calloc(frame_data_count, sizeof(double));
            g[gr_index].smp_index = calloc(frame_data_count, sizeof(unsigned int));
            g[gr_index].density = calloc(frame_data_count, sizeof(unsigned int));
            g[gr_index].speed = calloc(frame_data_count, sizeof(double));

            // initialization for each simultaneous channels
            for (int j = 0; j < frame_data_count; j += 1) {
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

inline void computeGrains(unsigned int channel, struct grain *g, unsigned int grain_index, float alpha, unsigned int si, unsigned int density, unsigned int density_offset, float *gr_env, struct sample *samples, unsigned int smp_index, unsigned int sample_rate, double min_duration, double max_duration, float *out_l, float *out_r) {
    struct grain *gr = &g[grain_index];
    struct sample *smp = &samples[smp_index];

    //unsigned int density_start = density_offset * si;

    for (int d = /*density_start*/0; d < (/*density_start + */si * gr->density[channel]); d += si) {
        gr = &g[grain_index + d];

        double env_index = gr->env_index[channel];

        if (env_index >= FAS_ENVS_SIZE) {
            double gr_speed = gr->speed[channel];

            float grain_start = (float)smp->frames - 1.0f;
            float grain_position = fabs(alpha);
            grain_start = roundf(grain_start * fmaxf(fminf(grain_position, 1.0f), 0.0f) * (1.0f - randf(0.0f, 1.0f) * floorf(fminf(grain_position - 0.0001f, 1.0f))));

            //gr->index[k] = grain_start;
            // old algorithm (percent of sample length, no cycle)
            // roundf(grain_start + fmaxf(randf(GRAIN_MIN_DURATION + min_duration, max_duration), GRAIN_MIN_DURATION) * (smp->frames - grain_start - 1)) + 1;
            gr->frames[channel] = roundf(fmaxf(randf(GRAIN_MIN_DURATION + min_duration, max_duration), GRAIN_MIN_DURATION) * (float)sample_rate);
            gr->env_step[channel] = fmax(((double)(FAS_ENVS_SIZE)) / (((double)gr->frames[channel]/* - (double)grain_start*/) / fabs(gr_speed)), 0.00000001);
            gr->env_index[channel] = 0.0f;
            gr->density[channel] = density;

            if (alpha < 0.0f) {
                if (gr_speed > 0.0f) {
                    gr->speed[channel] = -gr_speed;
                }

                gr->frame[channel] = grain_start + gr->frames[channel];
            } else {
                gr->speed[channel] = fabs(gr_speed);

                gr->frame[channel] = grain_start + gr->frames[channel];
                if (gr->frame[channel] > ((float)smp->frames - 1.0f)) {
                    gr->frame[channel] = 0;
                }
            }
        }

        double pos = gr->frame[channel];

        unsigned int sample_index = pos;
        unsigned int sample_index2 = sample_index + 1;

        float smp_l = smp->data_l[sample_index];
        float smp_r = smp->data_r[sample_index];

        float smp_l2 = smp->data_l[sample_index2];
        float smp_r2 = smp->data_r[sample_index2];

        float mu = pos - (float)sample_index;

        float smp_lv = smp_l + mu * (smp_l2 - smp_l);
        float smp_rv = smp_r + mu * (smp_r2 - smp_r);

        float env = gr_env[(unsigned int)round(env_index)];

        *out_l += smp_lv * env;
        *out_r += smp_rv * env;

        gr->frame[channel] += gr->speed[channel];

        if (gr->frame[channel] < 0) {
            gr->frame[channel] = ((float)smp->frames - 1.0f);
        }

        if (gr->frame[channel] > ((float)smp->frames - 1.0f)) {
            gr->frame[channel] = 0;
        }

        gr->env_index[channel] += gr->env_step[channel];
    }
}

struct grain *freeGrains(struct grain **g, unsigned int samples_count, unsigned int frame_data_count, unsigned int n, unsigned int max_density) {
    if (samples_count == 0) {
        return NULL;
    }

    struct grain *grains = *g;

    if (grains == NULL) {
        return NULL;
    }

    unsigned int grains_count = n * max_density * samples_count;

    int y = 0;
    for (y = 0; y < grains_count; y += samples_count) {
        for (int k = 0; k < samples_count; k += 1) {
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
