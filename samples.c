#include "inc/sndfile.h"
#include "inc/tinydir.h"
#include "inc/Yin.h"

#include "samples.h"

unsigned int notes_length = 120;
// TODO : generate it
char *notes[120] = {
  "C0","C#0","D0","D#0","E0","F0","F#0","G0","G#0","A0","A#0","B0",
  "C1","C#1","D1","D#1","E1","F1","F#1","G1","G#1","A1","A#1","B1",
  "C2","C#2","D2","D#2","E2","F2","F#2","G2","G#2","A2","A#2","B2",
  "C3","C#3","D3","D#3","E3","F3","F#3","G3","G#3","A3","A#3","B3",
  "C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4","A4","A#4","B4",
  "C5","C#5","D5","D#5","E5","F5","F#5","G5","G#5","A5","A#5","B5",
  "C6","C#6","D6","D#6","E6","F6","F#6","G6","G#6","A6","A#6","B6",
  "C7","C#7","D7","D#7","E7","F7","F#7","G7","G#7","A7","A#7","B7",
  "C8","C#8","D8","D#8","E8","F8","F#8","G8","G#8","A8","A#8","B8",
  "C9","C#9","D9","D#9","E9","F9","F#9","G9","G#9","A9","A#9","B9"
};

unsigned int load_samples(struct sample **s, char *directory) {
    unsigned int samples_count = 0;
    size_t path_len;

    struct sample *samples = NULL;

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    tinydir_dir dir;
    int ret = tinydir_open(&dir, directory);

    if (ret == -1) {
        printf("tinydir_open failed for directory '%s'.\n", directory);

        return 0;
    }

    samples = malloc(sizeof(struct sample) * 1);

    double ratio = pow(2.0, 1.0 / 12.0);

    while (dir.has_next) {
        int i, j;
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (file.is_reg) {
            size_t filepath_len = strlen(directory) + strlen(file.name);

            char *filepath = (char *)malloc(filepath_len + 1);
            filepath[0] = '\0';
            strcat(filepath, directory);
            strcat(filepath, file.name);
            filepath[filepath_len] = '\0';

            SNDFILE *audio_file;
            if (!(audio_file = sf_open(filepath, SFM_READ, &sfinfo))) {
                printf ("libsdnfile: Not able to open input file %s.\nlibsndfile: %s\n", filepath, sf_strerror(NULL));

                free(filepath);

                continue;
            }

            free(filepath);

            //printf("%i\n", sfinfo.frames);
            //printf("%i\n", sfinfo.channels);

            samples_count++;

            samples = (struct sample *)realloc(samples, sizeof(struct sample) * samples_count);

            // insert into samples data structure
            struct sample *smp = &samples[samples_count - 1];
            smp->chn_m1 = sfinfo.channels - 1;
            smp->chn = sfinfo.channels;
            smp->len = sfinfo.frames * sfinfo.channels;
            smp->frames = sfinfo.frames;
            smp->samplerate = sfinfo.samplerate;
            smp->data = (float *)malloc(smp->len * sizeof(float));
            smp->pitch = 0;
            sf_read_float(audio_file, smp->data, smp->len);

            sf_seek(audio_file, 0, SEEK_SET);

            // normalize samples
            unsigned int index = 0;
            float *max_value = (float *)malloc(sfinfo.channels * sizeof(float));
            for (i = 0; i < smp->frames; i++) {
                for(j = 0; j < sfinfo.channels; j++) {
                    index = i * sfinfo.channels + j;

                    max_value[j] = fmax(max_value[j], fabs(smp->data[index]));
                }
            }

            for (i = 0; i < smp->frames; i++) {
                for (j = 0; j < sfinfo.channels; j++) {
                    index = i * sfinfo.channels + j;

                    // normalize
                    smp->data[index] = smp->data[index] * (1.0f / max_value[j]);
                }
            }

            // == pitch detection
            // analyze file name to gather pitch informations first
            double o = 16.35 / 2;
            for (i = 0; i < notes_length; i += 1) {
                int n = i % 12;
                if (n == 0) {
                    o *= 2;
                }

                char *note_name = notes[i];

                char *res = strstr(file.name, note_name);
                if (res) {
                    double frequency = o * pow(ratio, n);

                    smp->pitch = frequency;

                    break;
                }
            }

            // TODO : try to get raw pitch from filename
            if (smp->pitch == 0) {

            }

            // try to guess it
            if (smp->pitch == 0) {
                int16_t *st_samples = (int16_t *)malloc(smp->len * sizeof(int16_t));
                sf_read_short(audio_file, st_samples, smp->len);

                int16_t *yin_samples = (int16_t *)malloc(smp->frames * sizeof(int16_t));

                // convert to mono for pitch detection
                for(i = 0; i < smp->frames; i++) {
                    yin_samples[i] = 0;
                    for(j = 0; j < sfinfo.channels; j++) {
                        index = i * sfinfo.channels + j;

                        yin_samples[i] += st_samples[index];
                    }
                    yin_samples[i] /= sfinfo.channels;
                }

                free(max_value);

                double uncertainty = 0.05;

                Yin *yin = (Yin *)malloc(sizeof(struct _Yin));
                while (uncertainty <= 0.75) { // max uncertainty before giving up is 75%
                    int p = 2;
                    int buffer_length = pow(2, p);

                    while (smp->pitch < 8) {
                        int yin_status = Yin_init(yin, buffer_length, uncertainty);

                        p++;
                        buffer_length = pow(2, p);

                        if (buffer_length > (smp->frames / 8 / 2)) {
                            break;
                        }

                        if (!yin_status) {
                            continue;
                        }

                        smp->pitch = Yin_getPitch(yin, yin_samples);
                        Yin_free(yin);

                        if (smp->pitch > 8) {
                            goto next; // ;)
                        }
                    }

                    uncertainty += 0.05; // increase uncertainty by 5%
                }
        next:

                free(yin);

                if (smp->pitch < 8) {
                    smp->pitch = 440. * 4.;
                    printf("Sample '%s' loaded, fundamental pitch was not detected, 440hz as default.\n", file.name);
                } else {
                    printf("Sample '%s' loaded, fundamental pitch is %fhz with %i%% uncertainty\n", file.name, smp->pitch / 4, (int)(100 * uncertainty));
                }

                free(yin_samples);
                free(st_samples);
            } else {
                printf("Sample '%s' loaded, pitch %fhz was detected.\n", file.name, smp->pitch);
            }

            sf_close(audio_file);
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

    *s = samples;

    return samples_count;
}
