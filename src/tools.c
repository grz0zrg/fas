#include "tools.h"

// http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
// http://www.kvraudio.com/forum/viewtopic.php?t=375517
FAS_FLOAT poly_blep(FAS_FLOAT phase_increment, FAS_FLOAT t) {
    FAS_FLOAT dt = phase_increment / M_PI2;
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
FAS_FLOAT raw_waveform(FAS_FLOAT phase, int type) {
    FAS_FLOAT value;
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
            value = 0;
            break;
    }
    return value;
}

FAS_FLOAT randf(FAS_FLOAT min, FAS_FLOAT max) {
    FAS_FLOAT random = ((FAS_FLOAT)rand()) / (FAS_FLOAT)RAND_MAX;

    FAS_FLOAT range = (max - min);
    return random * range + min;
}

FAS_FLOAT gaussian(FAS_FLOAT x, int L, FAS_FLOAT sigma) {
    return exp(-pow((x - L / 2) / (2 * sigma), 2));
}

FAS_FLOAT bessi0(FAS_FLOAT x) {
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

FAS_FLOAT **createEnvelopes(unsigned int n) {
    FAS_FLOAT **envs = (FAS_FLOAT **)malloc(FAS_ENVS_COUNT * sizeof(FAS_FLOAT *));

    FAS_FLOAT a0, a1, a2, a3, L, fN;

    for (unsigned int i = 0; i < FAS_ENVS_COUNT; i++) {
        envs[i] = (FAS_FLOAT *)calloc(n, sizeof(FAS_FLOAT));

        // thank to http://michaelkrzyzaniak.com/AudioSynthesis/2_Audio_Synthesis/11_Granular_Synthesis/1_Window_Functions/
        // NOTE : some fix is applied to ensure the boundaries land on 0
        switch(i) {
            case 0: // SINE
              for (unsigned int j = 0; j < n; j++) {
                  envs[i][j] = sin(M_PI * (double)j / (double)(n - 1));
              }
              break;

            case 1: // HANN
              for (unsigned int j = 0; j < n; j++) {
                  envs[i][j] = 0.5f * (1.0f - cos(2.0f * M_PI * (double)j / (double)(n - 1)));
              }
              break;

            case 2: // HAMMING
              for (unsigned int j = 0; j < n; j++) {
                  envs[i][j] = 0.54f - 0.46f * cos(2 * M_PI * (double)j / (double)(n - 1)) - 0.08f;
              }
              break;

            case 3: // TUKEY
              for (unsigned int j = 0; j < n; j++) {
                  FAS_FLOAT truncationHeight = 0.5f;
                  FAS_FLOAT f = 1.0f / (2.0f * truncationHeight) * (1.0f - cos(2.0f * M_PI * (double)j / (double)(n - 1)));
                  envs[i][j] = f < 1 ? f : 1;
              }
              break;

            case 4: // GAUSSIAN
              for (unsigned int j = 0; j < n; j++) {
                  FAS_FLOAT sigma = 0.3f;
                  envs[i][j] = fabs(pow(E, -0.5f * pow(((j - (double)(n - 1) /   2) / (sigma * (double)(n - 1) / 2)), 2) ) - 0.003866f);
              }
              break;

            case 5: // CONFINED GAUSSIAN
                fN = (FAS_FLOAT)n;
                FAS_FLOAT sigma = fN * 0.2f;
                FAS_FLOAT L = (FAS_FLOAT)(n - 1);

                FAS_FLOAT numerator = 0.0f;
                FAS_FLOAT denominator = 0.0f;
                FAS_FLOAT fn = 0.0f;

                for (unsigned int j = 0; j < n; j++) {
                    fn = (FAS_FLOAT)j;
                    numerator = gaussian(-0.5f, L, sigma) * (gaussian(fn + fN, L, sigma) + gaussian(fn - fN, L, sigma));
                    denominator = gaussian(-0.5f + fN, L, sigma) + gaussian(-0.5f - fN, L, sigma);
                    envs[i][j] = fmax(gaussian(fn, L, sigma) - numerator / denominator - 0.000020f, 0.f);
                }
              break;

            case 6: // TRAPEZOIDAL
              for (unsigned int j = 0; j < n; j++) {
                  FAS_FLOAT slope = 10;
                  FAS_FLOAT x = (FAS_FLOAT) j / n;
                  FAS_FLOAT f1 = slope * x;
                  FAS_FLOAT f2 = -1 * slope * (x-(slope-1) / slope) + 1;
                  envs[i][j] = fmax((x < 0.5f ? (f1 < 1 ? f1 : 1) : (f2 < 1 ? f2 : 1)) - 0.000152f, 0.f);
              }
              break;

            case 7: // BLACKMAN
                a0 = 0.426591f;
                a1 = 0.496561f;
                a2 = 0.076849f;
                L = (FAS_FLOAT)(n - 1);
                for (unsigned int j = 0; j < n; j++) {
                    envs[i][j] = a0;
                    envs[i][j] -= a1 * cos((1.0f * 2.0f * (FAS_FLOAT)(M_PI) * j) / L);
                    envs[i][j] += a2 * cos((2.0f * 2.0f * (FAS_FLOAT)(M_PI) * j) / L);
                    envs[i][j] -= 0.006879f;
                }
              break;

            case 8: // BLACKMAN HARRIS
                a0 = 0.35875f;
                a1 = 0.48829f;
                a2 = 0.14128f;
                a3 = 0.01168f;
                L = (FAS_FLOAT)(n - 1);

                for (unsigned int j = 0; j < n; j++) {
                    envs[i][j] = a0;
                    envs[i][j] -= a1 * cos((1.0f * 2.0f * (FAS_FLOAT)(M_PI) * j) / L);
                    envs[i][j] += a2 * cos((2.0f * 2.0f * (FAS_FLOAT)(M_PI) * j) / L);
                    envs[i][j] -= a3 * cos((3.0f * 2.0f * (FAS_FLOAT)(M_PI) * j) / L);
                    envs[i][j] = fmax(envs[i][j] - 0.000060f, 0.0f);
                }
              break;

            case 9: // PARZEN
              for (unsigned int j = 0; j < n; j++) {
                  envs[i][j] = fmax(1.0f - fabs(((double)j - 0.5f * (double)(n - 1)) / (0.5f * (double)(n + 1))) - 0.000031f, 0.f);
              }
              break;

            case 10: // NUTALL
              for (unsigned int j = 0; j < n; j++) {
                  envs[i][j] = 0.3635819f - 0.3635819f * cos(2*(FAS_FLOAT)(M_PI)*(double)j/(n-1)) + 0.1365995f * cos(4*(FAS_FLOAT)(M_PI)*(double)j/(n-1)) - 0.130411f *cos(6*(FAS_FLOAT)(M_PI)*(double)j/(n-1));
                  envs[i][j] -= 0.006188f;
              }
              break;

            case 11: // FLATTOP
              for (unsigned int j = 0; j < n; j++) {
                  envs[i][j] = fmax(1 - 1.93f*cos(2*(FAS_FLOAT)(M_PI)*(double)j/(n-1)) + 1.29f*cos(4*(FAS_FLOAT)(M_PI)*(double)j/(n-1)) - 0.388f*cos(6*(FAS_FLOAT)(M_PI)*(double)j/(n-1)) + 0.032f*cos(8*(FAS_FLOAT)(M_PI)*(double)j/(n-1)) * 0.215f, 0.0f);
              }
              break;

            case 12: // KAISER
              for (unsigned int j = 0; j < n; j++) {
                  double alpha = 3.0f;
                  envs[i][j] = bessi0((FAS_FLOAT)(M_PI) * alpha * sqrt(1 - pow((2 * (double)j / (n - 1)) - 1, 2))) / bessi0((FAS_FLOAT)(M_PI) * alpha);
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

void freeEnvelopes(FAS_FLOAT **envs) {
    if (envs) {
        for (unsigned int i = 0; i < FAS_ENVS_COUNT; i++) {
            free(envs[i]);
        }
        free(envs);
    }
}

int isPowerOfTwo(unsigned int x) {
    return (x && !(x & (x - 1)));
}

char *create_filepath(char *directory, char *filename) {
    int cdir = 0;

    size_t filename_length = strlen(filename);
    size_t directory_length = strlen(directory);

    char *dir = directory;

    if (directory[directory_length - 1] != '/') {
        directory_length = directory_length + 1;

        dir = (char *)malloc(directory_length + 1);
        if (!dir) {
            return NULL;
        }

        dir[0] = '\0';
        strcat(dir, directory);
        dir[directory_length - 1] = '/';
        dir[directory_length] = '\0';

        cdir = 1;
    }

    size_t filepath_len = strlen(dir) + filename_length;

    char *filepath = (char *)malloc(filepath_len + 1);
    if (!filepath) {
        if (cdir) {
            free(dir);
        }

        return NULL;
    }

    filepath[0] = '\0';
    strcat(filepath, dir);
    strcat(filepath, filename);
    filepath[filepath_len] = '\0';

    if (cdir) {
        free(dir);
    }

    return filepath;
}

// from PortAudio
double get_time(void) {
#ifdef __unix__
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (double)(tp.tv_sec + tp.tv_nsec * 1e-9);
#else
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (double) tv.tv_usec * 1e-6 + tv.tv_sec;
#endif
}

FAS_FLOAT lerp(FAS_FLOAT a, FAS_FLOAT b, FAS_FLOAT f) {
    return (a * (1.0 - f)) + (b * f);
}