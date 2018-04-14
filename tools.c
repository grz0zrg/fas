#include "tools.h"

// http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
// http://www.kvraudio.com/forum/viewtopic.php?t=375517
double poly_blep(double phase_increment, double t) {
    double dt = phase_increment / M_PI2;
    // 0 <= t < 1
    if (t < dt) {
        t /= dt;
        return t+t - t*t - 1.0;
    }
    // -1 < t < 0
    else if (t > 1.0 - dt) {
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    // 0 otherwise
    else return 0.0;
}

// http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
double raw_waveform(double phase, int type) {
    double value;
    switch (type) {
        case 0: // SINE
            value = sin(phase);
            break;
        case 1: // SAW
            value = (2.0 * phase / M_PI2) - 1.0;
            break;
        case 2: // SQUARE
            if (phase < M_PI) {
                value = 1.0;
            } else {
                value = -1.0;
            }
            break;
        case 3: // TRIANGLE
            value = -1.0 + (2.0 * phase / M_PI2);
            value = 2.0 * (fabs(value) - 0.5);
            break;
        default:
            break;
    }
    return value;
}

float randf(float min, float max) {
    if (min == 0.f && max == 0.f) {
        return 0.f;
    }

    float range = (max - min);
    float div = RAND_MAX / range;
    return min + (rand() / div);
}

float gaussian(float x, int L, float sigma) {
    return exp(-pow((x - L / 2) / (2 * sigma), 2));
}

float bessi0(float x) {
    double ax = fabs(x);

    if (ax < 3.75) {
        double y = x / 3.75;
        y = y * y;
        return 1.0 + y*(3.5156229+y*(3.0899424+y*(1.2067492+y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
    } else {
        double y = 3.75 / ax;
        return (exp(ax) / sqrt(ax)) * (0.39894228+y*(0.1328592e-1+y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2+y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1+y*0.392377e-2))))))));
   }
}

float **createEnvelopes(unsigned int n) {
    float **envs = (float **)malloc(FAS_ENVS_COUNT * sizeof(float *));

    float a0, a1, a2, a3, L, fN;

    for (int i = 0; i < FAS_ENVS_COUNT; i++) {
        envs[i] = (float *)malloc(n * sizeof(float));

        // thank to http://michaelkrzyzaniak.com/AudioSynthesis/2_Audio_Synthesis/11_Granular_Synthesis/1_Window_Functions/
        // NOTE : some fix is applied to ensure the boundaries land on 0
        switch(i) {
            case 0: // SINE
              for (int j = 0; j < n; j++) {
                  envs[i][j] = sin(M_PI * (double)j / (double)(n - 1));
              }
              break;

            case 1: // HANN
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 0.5f * (1.0f - cos(2.0f * M_PI * (double)j / (double)(n - 1)));
              }
              break;

            case 2: // HAMMING
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 0.54f - 0.46f * cos(2 * M_PI * (double)j / (double)(n - 1)) - 0.08f;
              }
              break;

            case 3: // TUKEY
              for (int j = 0; j < n; j++) {
                  float truncationHeight = 0.5f;
                  float f = 1.0f / (2.0f * truncationHeight) * (1.0f - cos(2.0f * M_PI * (double)j / (double)(n - 1)));
                  envs[i][j] = f < 1 ? f : 1;
              }
              break;

            case 4: // GAUSSIAN
              for (int j = 0; j < n; j++) {
                  float sigma = 0.3f;
                  envs[i][j] = fabsf(pow(E, -0.5f * pow(((j - (double)(n - 1) /   2) / (sigma * (double)(n - 1) / 2)), 2) ) - 0.003866f);
              }
              break;

            case 5: // CONFINED GAUSSIAN
                fN = (float)n;
                float sigma = fN * 0.2f;
                float L = (float)(n - 1);

                float numerator = 0.0f;
                float denominator = 0.0f;
                float fn = 0.0f;

                for (int j = 0; j < n; j++) {
                    fn = (float)j;
                    numerator = gaussian(-0.5f, L, sigma) * (gaussian(fn + fN, L, sigma) + gaussian(fn - fN, L, sigma));
                    denominator = gaussian(-0.5f + fN, L, sigma) + gaussian(-0.5f - fN, L, sigma);
                    envs[i][j] = fmaxf(gaussian(fn, L, sigma) - numerator / denominator - 0.000020f, 0.f);
                }
              break;

            case 6: // TRAPEZOIDAL
              for (int j = 0; j < n; j++) {
                  float slope = 10;
                  float x = (float) j / n;
                  float f1 = slope * x;
                  float f2 = -1 * slope * (x-(slope-1) / slope) + 1;
                  envs[i][j] = fmaxf((x < 0.5f ? (f1 < 1 ? f1 : 1) : (f2 < 1 ? f2 : 1)) - 0.000152f, 0.f);
              }
              break;

            case 7: // BLACKMAN
                a0 = 0.426591f;
                a1 = 0.496561f;
                a2 = 0.076849f;
                L = (float)(n - 1);
                for (int j = 0; j < n; j++) {
                    envs[i][j] = a0;
                    envs[i][j] -= a1 * cos((1.0f * 2.0f * (float)(M_PI) * j) / L);
                    envs[i][j] += a2 * cos((2.0f * 2.0f * (float)(M_PI) * j) / L);
                    envs[i][j] -= 0.006879f;
                }
              break;

            case 8: // BLACKMAN HARRIS
                a0 = 0.35875f;
                a1 = 0.48829f;
                a2 = 0.14128f;
                a3 = 0.01168f;
                L = (float)(n - 1);

                for (int j = 0; j < n; j++) {
                    envs[i][j] = a0;
                    envs[i][j] -= a1 * cos((1.0f * 2.0f * (float)(M_PI) * j) / L);
                    envs[i][j] += a2 * cos((2.0f * 2.0f * (float)(M_PI) * j) / L);
                    envs[i][j] -= a3 * cos((3.0f * 2.0f * (float)(M_PI) * j) / L);
                    envs[i][j] = fmaxf(envs[i][j] - 0.000060f, 0.0f);
                }
              break;

            case 9: // PARZEN
              for (int j = 0; j < n; j++) {
                  envs[i][j] = fmaxf(1.0f - fabs(((double)j - 0.5f * (double)(n - 1)) / (0.5f * (double)(n + 1))) - 0.000031f, 0.f);
              }
              break;

            case 10: // NUTALL
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 0.3635819f - 0.3635819f * cos(2*(float)(M_PI)*(double)j/(n-1)) + 0.1365995f * cos(4*(float)(M_PI)*(double)j/(n-1)) - 0.130411f *cos(6*(float)(M_PI)*(double)j/(n-1));
                  envs[i][j] -= 0.006188f;
              }
              break;

            case 11: // FLATTOP
              for (int j = 0; j < n; j++) {
                  envs[i][j] = fmaxf(1 - 1.93f*cos(2*(float)(M_PI)*(double)j/(n-1)) + 1.29f*cos(4*(float)(M_PI)*(double)j/(n-1)) - 0.388f*cos(6*(float)(M_PI)*(double)j/(n-1)) + 0.032f*cos(8*(float)(M_PI)*(double)j/(n-1)) * 0.215f, 0.0f);
              }
              break;

            case 12: // KAISER
              for (int j = 0; j < n; j++) {
                  double alpha = 3.0f;
                  envs[i][j] = bessi0((float)(M_PI) * alpha * sqrt(1 - pow((2 * (double)j / (n - 1)) - 1, 2))) / bessi0((float)(M_PI) * alpha);
                  envs[i][j] -= 0.000612f;
              }
              break;

            default:
              printf("Envelope type %i not defined, ignored.\n", i);
              free(envs[i]);
              break;
        }
    }

    return envs;
}

void freeEnvelopes(float **envs) {
    if (envs) {
        for (int i = 0; i < FAS_ENVS_COUNT; i++) {
            free(envs[i]);
        }
        free(envs);
    }
}
