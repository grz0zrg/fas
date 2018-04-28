#include "inc/sndfile.h"
#include "inc/tinydir.h"

#include "samples.h"

unsigned int notes_length = 120;
// TODO : generate it
char *notes[240] = {
  "c0","c#0","d0","d#0","e0","f0","f#0","g0","g#0","a0","a#0","b0",
  "c1","c#1","d1","d#1","e1","f1","f#1","g1","g#1","a1","a#1","b1",
  "c2","c#2","d2","d#2","e2","f2","f#2","g2","g#2","a2","a#2","b2",
  "c3","c#3","d3","d#3","e3","f3","f#3","g3","g#3","a3","a#3","b3",
  "c4","c#4","d4","d#4","e4","f4","f#4","g4","g#4","a4","a#4","b4",
  "c5","c#5","d5","d#5","e5","f5","f#5","g5","g#5","a5","a#5","b5",
  "c6","c#6","d6","d#6","e6","f6","f#6","g6","g#6","a6","a#6","b6",
  "c7","c#7","d7","d#7","e7","f7","f#7","g7","g#7","a7","a#7","b7",
  "c8","c#8","d8","d#8","e8","f8","f#8","g8","g#8","a8","a#8","b8",
  "c9","c#9","d9","d#9","e9","f9","f#9","g9","g#9","a9","a#9","b9",
  "c0","cs0","d0","ds0","e0","f0","fs0","g0","gs0","a0","as0","b0",
  "c1","cs1","d1","ds1","e1","f1","fs1","g1","gs1","a1","as1","b1",
  "c2","cs2","d2","ds2","e2","f2","fs2","g2","gs2","a2","as2","b2",
  "c3","cs3","d3","ds3","e3","f3","fs3","g3","gs3","a3","as3","b3",
  "c4","cs4","d4","ds4","e4","f4","fs4","g4","gs4","a4","as4","b4",
  "c5","cs5","d5","ds5","e5","f5","fs5","g5","gs5","a5","as5","b5",
  "c6","cs6","d6","ds6","e6","f6","fs6","g6","gs6","a6","as6","b6",
  "c7","cs7","d7","ds7","e7","f7","fs7","g7","gs7","a7","as7","b7",
  "c8","cs8","d8","ds8","e8","f8","fs8","g8","gs8","a8","as8","b8",
  "c9","cs9","d9","ds9","e9","f9","fs9","g9","gs9","a9","as9","b9"
};

struct fas_path {
    char *name;
    struct fas_path *next;
};

// https://github.com/tidalcycles/Dirt
void fix_samplerate (struct sample *sample, unsigned int samplerate, int converter_type) {
    SRC_DATA data;
    int max_output_frames;
    int channels = sample->chn;

    if (sample->samplerate == samplerate) {
        return;
    }

    data.src_ratio = (float) samplerate / (float) sample->samplerate;

    max_output_frames = sample->frames * data.src_ratio + 32;

    data.data_in = sample->data;
    data.input_frames = sample->frames;

    data.data_out = (float *) calloc(1, sizeof(float) * max_output_frames* channels);
    data.output_frames = max_output_frames;

    int conv_result = src_simple(&data, converter_type, channels);

    if (conv_result != 0) {
        const char *conv_error_str = src_strerror(conv_result);

        if (conv_error_str == NULL) {
            printf("fix_samplerate: Unknown conversion error.");
            fflush(stdout);
        } else {
            printf("fix_samplerate: %s", conv_error_str);
            fflush(stdout);
        }

        free(data.data_out);
    } else {
        if (sample->data) {
            free(sample->data);
        }

        sample->data = data.data_out;
        sample->samplerate = samplerate;
        sample->frames = data.output_frames_gen;
    }
}

char *create_filepath(char *directory, char *filename) {
    size_t filename_length = strlen(filename);
    size_t directory_length = strlen(directory);

    char *dir = directory;

    if (directory[directory_length - 1] != '/') {
        directory_length = directory_length + 1;

        dir = (char *)malloc(directory_length);
        dir[0] = '\0';
        strcat(dir, directory);
        dir[directory_length - 1] = '/';
        dir[directory_length] = '\0';
    }

    size_t filepath_len = strlen(dir) + filename_length;

    char *filepath = (char *)malloc(filepath_len + 1);
    filepath[0] = '\0';
    strcat(filepath, dir);
    strcat(filepath, filename);
    filepath[filepath_len] = '\0';

    if (dir != directory) {
        free(dir);
    }

    return filepath;
}

unsigned int load_samples(struct sample **s, char *directory, unsigned int samplerate, int converter_type, int smooth_end) {
    int f = 0;

    unsigned int samples_count = 0;
    size_t path_len;

    struct sample *samples = NULL;

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    tinydir_dir dir;
    int ret = tinydir_open_sorted(&dir, directory);

    if (ret == -1) {
        printf("tinydir_open failed for directory '%s'.\n", directory);

        return 0;
    }

    samples = calloc(1, sizeof(struct sample));

    double ratio = pow(2.0, 1.0 / 12.0);

    size_t dir_length = strlen(directory);
    char *current_dir = (char *)malloc(sizeof(char) * (dir_length + 1));
    memcpy(current_dir, directory, dir_length + 1);

    struct fas_path *level_dir = (struct fas_path *)malloc(sizeof(struct fas_path));
    level_dir->name = NULL;
    level_dir->next = NULL;
    struct fas_path *first_dir = level_dir;

    //while (dir.has_next) {
    for (f = 0; f <= dir.n_files; f++) {
        if (f == dir.n_files) {
            if (first_dir->name != NULL) {
                tinydir_close(&dir);
                ret = tinydir_open_sorted(&dir, first_dir->name);

                if (ret == -1) {
                    printf("tinydir_open failed for directory '%s'.\n", first_dir->name);

                    break;
                }

                char *tmp = current_dir;

                size_t dir_length = strlen(first_dir->name);
                current_dir = (char *)malloc(sizeof(char) * (dir_length + 1));
                memcpy(current_dir, first_dir->name, dir_length + 1);

                printf("switching to %s first dir %s\n", current_dir, first_dir->name);
                fflush(stdout);

                free(tmp);

                struct fas_path *tmp_path = first_dir;

                first_dir = first_dir->next;

                free(tmp_path->name);
                free(tmp_path);

                f = 0;
                continue;
            } else {
                free(current_dir);
                free(first_dir->name);
                free(first_dir->next);
                free(first_dir);
                break;
            }
        }

        int i, j, k;
        tinydir_file file;
        //tinydir_readfile(&dir, &file);
        tinydir_readfile_n(&dir, &file, f);

        size_t filename_length = strlen(file.name);

        // place all folders into a list, they will be fetched next recursively
        if (file.is_dir) {
            if (filename_length >= 2) {
                if (file.name[0] == '.' && file.name[1] == '.') {
                    continue;
                }
            }

            if (filename_length == 1) {
                if (file.name[0] == '.') {
                    continue;
                }
            }

            char *filepath = create_filepath(current_dir, file.name);

            level_dir->name = filepath;
            level_dir->next = (struct fas_path *)malloc(sizeof(struct fas_path));
            level_dir->next->name = NULL;
            level_dir->next->next = NULL;

            level_dir = level_dir->next;
        } else if (file.is_reg) {
            char *filepath = create_filepath(current_dir, file.name);

            SNDFILE *audio_file;
            if (!(audio_file = sf_open(filepath, SFM_READ, &sfinfo))) {
                printf ("libsdnfile: Not able to open input file %s.\nlibsndfile: %s\n", filepath, sf_strerror(NULL));

                free(filepath);

                //tinydir_next(&dir);
                continue;
            }

            free(filepath);

            samples_count++;

            samples = (struct sample *)realloc(samples, sizeof(struct sample) * samples_count);

            // insert into samples data structure
            struct sample *smp = &samples[samples_count - 1];
            smp->chn_m1 = sfinfo.channels - 1;
            smp->chn = sfinfo.channels;
            smp->len = sfinfo.frames * sfinfo.channels;
            smp->frames = sfinfo.frames;
            smp->samplerate = sfinfo.samplerate;
            smp->data = (float *)calloc(smp->len, sizeof(float));
            smp->pitch = 0;

            sf_count_t read_count = sf_read_float(audio_file, smp->data, smp->len);

            sf_seek(audio_file, 0, SEEK_SET);

            if (converter_type > 0) {
                fix_samplerate(smp, samplerate, converter_type);
            }

#ifdef FAS_USE_CUBIC_INTERP
            int pad_length = 4;
#else
            int pad_length = 1;
#endif

            int padded_frames_len = smp->frames + pad_length; // make room for interpolation methods

            smp->data_l = (float *)calloc(padded_frames_len, sizeof(float));
            smp->data_r = (float *)calloc(padded_frames_len, sizeof(float));

            // normalize samples
            unsigned int index = 0;
            float *max_value = (float *)calloc(sfinfo.channels, sizeof(float));
            for (i = 0; i < smp->frames; i++) {
                for (j = 0; j < sfinfo.channels; j++) {
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

            free(max_value);

            if (smp->chn > 1) {
                // copy l&r
                index = 0;
                for (i = 0; i < smp->frames * 2; i += 2) {
                    smp->data_l[index] = smp->data[i];
                    smp->data_r[index] = smp->data[i + 1];

                    index += 1;
                }
            } else {
                for (i = 0; i < smp->frames; i++) {
                    smp->data_l[i] = smp->data[i];
                    smp->data_r[i] = smp->data[i];
                }
            }

            // apply small amount of fade out/in (eliminate crackles)
            unsigned int smooth_samples = 32;
            float factor_step = 1.0f / (float)smooth_samples;
            float factor = 0.0f;

            if (smp->frames > (smooth_samples * 4) && smooth_end) {
                for (i = 0; i < smooth_samples; i ++) {
                    smp->data_l[i] *= factor;
                    smp->data_r[i] *= factor;

                    smp->data_l[smp->frames - i - 1] *= factor;
                    smp->data_r[smp->frames - i - 1] *= factor;

                    factor += factor_step;
                }
            }

            for (i = 0; i < pad_length; i += 1) {
                smp->data_l[smp->frames - i] = 0;
                smp->data_r[smp->frames - i] = 0;
            }

            free(smp->data);

            // make room to copy filename
            char *filename = calloc(filename_length + 1, sizeof(char));

            // == pitch detection
            // analyze file name to gather pitch informations first
            double o = 16.35 / 2;
            int detected = 0;
            for (j = 0; j < 2; j += 1) {
                unsigned int start = notes_length * j;
                for (i = 0; i < notes_length; i += 1) {
                    int n = i % 12;
                    if (n == 0) {
                        o *= 2;
                    }

                    char *_note_name = notes[i + start];
                    char *note_name = calloc(strlen(_note_name) + 1, sizeof(char));
                    strcpy(note_name, _note_name);

                    // check lowercase and uppercase note character
                    for (k = 0; k < 2; k += 1) {
                        char *res = strstr(file.name, note_name);
                        if (res) {
                            double frequency = o * pow(ratio, n);

                            smp->pitch = frequency;

                            free(note_name);

                            goto pitch_detected;
                        }

                        note_name[0] -= 32;
                    }

                    free(note_name);
                }
            }

        pitch_detected:

            // TODO : try to get raw pitch from filename
            if (smp->pitch == 0) {
                strcpy(filename, file.name);
                char *res = strtok(filename, "##");
                if (res) {
                    res = strtok (NULL, "##");
                    if (res) {
                        smp->pitch = strtod(res, NULL);
                        if (errno == ERANGE) {
                            smp->pitch = 0;
                        }
                    }
                }
            }

            free(filename);

            // try to guess it
            if (smp->pitch == 0) {
                int16_t *st_samples = (int16_t *)calloc(smp->len, sizeof(int16_t));
                sf_read_short(audio_file, st_samples, smp->len);

                int16_t *yin_samples = (int16_t *)calloc(smp->frames, sizeof(int16_t));

                // convert to mono for pitch detection
                for(i = 0; i < smp->frames; i++) {
                    yin_samples[i] = 0;
                    for(j = 0; j < sfinfo.channels; j++) {
                        index = i * sfinfo.channels + j;

                        yin_samples[i] += st_samples[index];
                    }
                    yin_samples[i] /= sfinfo.channels;
                }

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
                    printf("Sample %i '%s' loaded, fundamental pitch was not detected, 440hz as default.\n", samples_count, file.name);
                } else {
                    printf("Sample %i '%s' loaded, fundamental pitch is %fhz with %i%% uncertainty\n", samples_count, file.name, smp->pitch / 4, (int)(100 * uncertainty));
                }

                free(yin_samples);
                free(st_samples);
            } else {
                printf("Sample %i '%s' loaded, pitch %fhz was detected.\n", samples_count, file.name, smp->pitch);
            }

            sf_close(audio_file);
        }

        //tinydir_next(&dir);
    }

    tinydir_close(&dir);

    *s = samples;

    // print samples map
    double sstep = 1.0 / (double)samples_count;
    double scurr = 0.0;
    for (int i = 1; i <= samples_count; i += 1) {
        printf("Sample %i mapped to [%f, %f)\n", i, scurr, scurr + sstep);

        scurr += sstep;
    }
    fflush(stdout);

    return samples_count;
}

void free_samples(struct sample **s, unsigned int samples_count) {
    unsigned int i = 0;

    struct sample *samples = *s;

    for (i = 0; i < samples_count; i++) {
      struct sample *smp = &samples[i];
      free(smp->data_l);
      free(smp->data_r);
    }
    free(samples);
}
