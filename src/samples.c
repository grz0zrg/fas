#include "sndfile.h"
#include "tinydir/tinydir.h"

#include "tools.h"
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
void resample (struct sample *sample, unsigned int samplerate, int converter_type) {
    SRC_DATA data;
    int max_output_frames;
    int channels = sample->chn;

    if ((unsigned int)sample->samplerate == samplerate) {
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
            printf("resample: Unknown conversion error.\n");
            fflush(stdout);
        } else {
            printf("resample: %s\n", conv_error_str);
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

double parse_pitch(const char *name) {
    double ratio = pow(2.0, 1.0 / 12.0);

    double o = 16.3515978313 / 2;
    int detected = 0;

    unsigned int i, j, k;
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
                char *res = strstr(name, note_name);
                if (res) {
                    double frequency = o * pow(ratio, n);

                    free(note_name);

                    return frequency;
                }

                note_name[0] -= 32;
            }

            free(note_name);
        }
    }

    return 0;
}

double parse_frequency(const char *name) {
    char *filename = calloc(strlen(name) + 1, sizeof(char));

    strcpy(filename, name);

    double pitch = 0;

    char *res = strtok(filename, "##");
    if (res) {
        res = strtok (NULL, "##");
        if (res) {
            pitch = strtod(res, NULL);
            if (errno == ERANGE) {
                pitch = 0;
            }
        }
    }

    free(filename);

    return pitch;
}

#ifdef WITH_AUBIO
int aubioNotesSort(const void *e1, const void *e2) {
    fvec_t *n1 = *(fvec_t * const *)e1;
    fvec_t *n2 = *(fvec_t * const *)e2;

    if (n1->data[0] > n2->data[0]) return 1;
    if (n1->data[0] < n2->data[0]) return -1;

    return 0;
}
#endif

unsigned int load_samples(
#ifdef WITH_SOUNDPIPE
        sp_data *sp,
#endif
        struct sample **s, 
        char *directory, 
        unsigned int samplerate, 
        int converter_type, 
        int pitch_detection) {
    unsigned int f = 0;

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

    size_t dir_length = strlen(directory);
    char *current_dir = (char *)malloc(sizeof(char) * (dir_length + 1));
    memcpy(current_dir, directory, dir_length + 1);

    struct fas_path *level_dir = (struct fas_path *)malloc(sizeof(struct fas_path));
    level_dir->name = NULL;
    level_dir->next = NULL;
    struct fas_path *first_dir = level_dir;

    double folder_pitch = 0;

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

                // detect pitch from folder name
                if (pitch_detection != 0) {
                    folder_pitch = parse_pitch(first_dir->name);
                    if (folder_pitch == 0) {
                        folder_pitch = parse_frequency(first_dir->name);
                    }
                }

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

        unsigned int i;
        int j;
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

            if (!filepath) {
                continue;
            }

            level_dir->name = filepath;
            level_dir->next = (struct fas_path *)malloc(sizeof(struct fas_path));
            level_dir->next->name = NULL;
            level_dir->next->next = NULL;

            level_dir = level_dir->next;
        } else if (file.is_reg) {
            char *filepath = create_filepath(current_dir, file.name);

            if (!filepath) {
                continue;
            }

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

            // adjust to current samplerate
            if (converter_type >= 0) {
                resample(smp, samplerate, converter_type);
            }

#ifdef FAS_USE_CUBIC_INTERP
            int pad_length = 4;
#else
            int pad_length = 1;
#endif

            int padded_frames_len = smp->frames + pad_length; // make room for interpolation methods

            smp->data_l = (FAS_FLOAT *)calloc(padded_frames_len, sizeof(FAS_FLOAT));
            smp->data_r = (FAS_FLOAT *)calloc(padded_frames_len, sizeof(FAS_FLOAT));

            // normalize samples
            unsigned int index = 0;
            FAS_FLOAT *max_value = (FAS_FLOAT *)calloc(sfinfo.channels, sizeof(FAS_FLOAT));
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

            for (j = 0; j < pad_length; j += 1) {
                smp->data_l[smp->frames + (pad_length - 1) - j] = 0;
                smp->data_r[smp->frames + (pad_length - 1) - j] = 0;
            }

            free(smp->data);

#ifdef WITH_SOUNDPIPE
            // embed sample infos into sp_ftbl
            sp_ftbl_bind(sp, &smp->ftbl, smp->data_l, smp->frames);
#endif

            if (pitch_detection == 0) {
                // it is either a single cycle waveform or an impulse file so adjust pitch for single cycle waveform
                smp->pitch = (double)smp->samplerate / smp->frames;

                goto close;
            }

            // == pitch detection
            // analyze file name to gather pitch informations first
            smp->pitch = parse_pitch(file.name);

            // get pitch from filename (raw frequency)
            if (smp->pitch == 0) {
                smp->pitch = parse_frequency(file.name);
            }

            // fallback to folder pitch
            if (smp->pitch == 0) {
                smp->pitch = folder_pitch;
            }

            // no pitch detected yet so try to guess it
            if (smp->pitch == 0) {
#ifdef WITH_AUBIO
                uint_t buffer_size = 2048;
                uint_t hop_size = 2048;
                aubio_notes_t *notes = new_aubio_notes("default", buffer_size, hop_size, smp->samplerate);
                if (notes == NULL) {
                    goto next;
                }

                aubio_notes_set_minioi_ms(notes, 0.012);

                fvec_t *input_buffer = new_fvec(hop_size);
                
                unsigned int hop_count = floor((double)smp->frames / hop_size);
                fvec_t **output_buffer = calloc(hop_count, sizeof(fvec_t *));
                for (i = 0; i < hop_count; i += 1) {
                    output_buffer[i] = new_fvec(3);
                }

                unsigned real_notes_count = 0;
                unsigned int hop = 0;
                while (hop != hop_count) {
                    for (i = 0; i < hop_size; i += 1) {
                        input_buffer->data[i] = smp->data_l[hop * hop_size + i];
                    }

                    aubio_notes_do(notes, input_buffer, output_buffer[hop]);

                    if (output_buffer[hop]->data[0] != 0) {
                        real_notes_count += 1;
                    }

                    hop += 1;
                }

                fvec_t **notes_buffer = NULL;

                if (real_notes_count == 0) {
                    goto cannot_guess;
                }

                notes_buffer = calloc(real_notes_count, sizeof(fvec_t *));

                unsigned int count = 0;
                for (i = 0; i < hop_count; i += 1) {
                    if (output_buffer[i]->data[0] != 0) {
                        notes_buffer[count] = output_buffer[i];
                        count += 1;
                    }
                }

                qsort(output_buffer, hop_count, sizeof(output_buffer[0]), aubioNotesSort);

                fvec_t *median_note = notes_buffer[real_notes_count / 2];

                smp->pitch = aubio_miditofreq(median_note->data[0]);

        cannot_guess:

                del_fvec(input_buffer);

                for (i = 0; i < hop_count; i += 1) {
                    del_fvec(output_buffer[i]);
                }
                free(output_buffer);
                free(notes_buffer);

                del_aubio_notes(notes);
#endif
        next:
                if (smp->pitch == 0) {
                    smp->pitch = 440.;
                    printf("Fundamental frequency was not detected, 440hz as default.\n");
                } else {
                    printf("Fundamental frequency %fhz was detected. (automatic)\n", smp->pitch);
                }
            } else {
                printf("Fundamental frequency %fhz was detected.\n", smp->pitch);
            }

        close:

            printf("Sample %i '%s' loaded.\n", samples_count, file.name);

            sf_close(audio_file);
        }

        //tinydir_next(&dir);
    }

    tinydir_close(&dir);

    *s = samples;

    // print samples map
    double sstep = 1.0 / (double)samples_count;
    double scurr = 0.0;
    for (unsigned int i = 1; i <= samples_count; i += 1) {
        printf("Sample %i mapped to [%f, %f)\n", i, scurr, scurr + sstep);

        scurr += sstep;
    }
    fflush(stdout);

    return samples_count;
}

void free_samples(struct sample **s, unsigned int samples_count) {
    unsigned int i = 0;

    struct sample *samples = *s;

    for (i = 0; i < samples_count; i += 1) {
        struct sample *smp = &samples[i];

        free(smp->data_l);
        free(smp->data_r);

#ifdef WITH_SOUNDPIPE
        sp_ftbl_destroy(&smp->ftbl);
#endif
    }
    free(samples);
}
