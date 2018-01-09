#include "tools.h"

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
        switch(i) {
            case 0: // SINE
              for (int j = 0; j < n; j++) {
                  envs[i][j] = sin(M_PI * (double)j / (double)(n - 1));
              }
              break;

            case 1: // HANN
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 0.5 * (1.0 - cos(2.0 * M_PI * (double)j / (double)(n - 1)));
              }
              break;

            case 2: // HAMMING
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 0.54 - 0.46 * cos(2 * M_PI * (double)j / (double)(n - 1)) - 0.075;
              }
              break;

            case 3: // TUKEY
              for (int j = 0; j < n; j++) {
                  float truncationHeight = 0.5;
                  float f = 1.0 / (2.0 * truncationHeight) * (1.0 - cos(2.0 * M_PI * (double)j / (double)(n - 1)));
                  envs[i][j] = f < 1 ? f : 1;
              }
              break;

            case 4: // GAUSSIAN
              for (int j = 0; j < n; j++) {
                  float sigma = 0.3;
                  envs[i][j] = pow(E, -0.5* pow(((j - (double)(n - 1) /   2) / (sigma * (double)(n - 1) / 2)), 2) );
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
                    envs[i][j] = gaussian(fn, L, sigma) - numerator / denominator;
                }
              break;

            case 6: // TRAPEZOIDAL
              for (int j = 0; j < n; j++) {
                  float slope = 10;
                  float x = (float) j / n;
                  float f1 = slope * x;
                  float f2 = -1 * slope * (x-(slope-1) / slope) + 1;
                  envs[i][j] = x < 0.5 ? (f1 < 1 ? f1 : 1) : (f2 < 1 ? f2 : 1);
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
                }
              break;

            case 9: // PARZEN
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 1.0 - fabs(((double)j - 0.5 * (double)(n - 1)) / (0.5 * (double)(n + 1)));
              }
              break;

            case 10: // NUTALL
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 0.3635819 - 0.3635819 * cos(2*(float)(M_PI)*(double)j/(n-1)) + 0.1365995* cos(4*(float)(M_PI)*(double)j/(n-1)) - 0.130411*cos(6*(float)(M_PI)*(double)j/(n-1));
              }
              break;

            case 11: // FLATTOP
              for (int j = 0; j < n; j++) {
                  envs[i][j] = 1 - 1.93*cos(2*(float)(M_PI)*(double)j/(n-1)) + 1.29*cos(4*(float)(M_PI)*(double)j/(n-1)) - 0.388*cos(6*(float)(M_PI)*(double)j/(n-1)) + 0.032*cos(8*(float)(M_PI)*(double)j/(n-1)) * 0.215;
              }
              break;

            case 12: // KAISER
              for (int j = 0; j < n; j++) {
                  double alpha = 3.0;
                  envs[i][j] = bessi0((float)(M_PI) * alpha * sqrt(1 - pow((2 * (double)j / (n - 1)) - 1, 2))) / bessi0((float)(M_PI) * alpha);
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
