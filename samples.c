#include "inc/sndfile.h"
#include "inc/tinydir.h"
#include "inc/Yin.h"

#include "samples.h"

unsigned int load_samples(struct sample *samples, char *directory) {
    unsigned int samples_count = 0;
    size_t path_len;

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    tinydir_dir dir;
    tinydir_open(&dir, directory);

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
                printf ("libsdnfile: Not able to open input file %s.\nlibsndfile: %s", filepath, sf_strerror(NULL));

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
            smp->chn = sfinfo.channels - 1;
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

            Yin *yin = (Yin *)malloc(sizeof(struct _Yin));

            Yin_init(yin, 4096, 0.05);
            smp->pitch = Yin_getPitch(yin, yin_samples);

            if (smp->pitch == -1) {
              smp->pitch = 440.0;
            }

            printf("Sample '%s' loaded, detected fundamental pitch is %fhz\n", file.name, smp->pitch);

            free(yin_samples);
            free(st_samples);
            free(yin);

            sf_close(audio_file);
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

    return samples_count;
}
