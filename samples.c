#include "inc/sndfile.h"
#include "inc/tinydir.h"
#include "inc/Yin.h"

#include "samples.h"

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

    while (dir.has_next) {
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
            sf_read_float(audio_file, smp->data, smp->len);

            sf_seek(audio_file, 0, SEEK_SET);

            // pitch detection
            int16_t *st_samples = (int16_t *)malloc(smp->len * sizeof(int16_t));
            sf_read_short(audio_file, st_samples, smp->len);

            int16_t *yin_samples = (int16_t *)malloc(smp->frames * sizeof(int16_t));
            for(int i = 0; i < smp->frames; i++) { // convert to mono for pitch detection
                yin_samples[i] = 0;
                for(int j = 0; j < sfinfo.channels; j++) {
                    yin_samples[i] += st_samples[i * sfinfo.channels + j];
                }
                yin_samples[i] /= sfinfo.channels;
            }

            smp->pitch = 0;
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

            sf_close(audio_file);
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

    *s = samples;

    return samples_count;
}
