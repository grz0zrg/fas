#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "oscillators.h"

#ifdef WITH_FAUST
void createFaustGenerators(
    struct _faust_factories *faust_factories,
    struct oscillator *osc_bank,
    unsigned int n,
    unsigned int sample_rate,
    unsigned int frame_data_count) {
    if (!osc_bank) {
        return;
    }

    int y = 0, i = 0, k = 0;

    int nmo = n - 1;
    for (y = 0; y < n; y += 1) {
        int index = nmo - y;

        struct oscillator *osc = &osc_bank[index];

        osc->faust_gens_len = faust_factories->len;
        osc->faust_gens = malloc(sizeof(struct _fas_faust_dsp **) * frame_data_count);

        for (i = 0; i < frame_data_count; i += 1) {
            osc->faust_gens[i] = malloc(sizeof(struct _fas_faust_dsp *) * (faust_factories->len));
            for (k = 0; k < faust_factories->len; k += 1) {
                osc->faust_gens[i][k] = malloc(sizeof(struct _fas_faust_dsp));

                struct _fas_faust_ui_control *uiface = calloc(1, sizeof(struct _fas_faust_ui_control));
                
                UIGlue *ui = malloc(sizeof(UIGlue));
                ui->openTabBox = ui_open_tab_box;
                ui->openHorizontalBox = ui_open_horizontal_box;
                ui->openVerticalBox = ui_open_vertical_box;
                ui->closeBox = ui_close_box;
                ui->addButton = ui_add_button;
                ui->addCheckButton = ui_add_check_button;
                ui->addVerticalSlider = ui_add_vertical_slider;
                ui->addHorizontalSlider = ui_add_horizontal_slider;
                ui->addNumEntry = ui_add_num_entry;
                ui->addHorizontalBargraph = ui_add_horizontal_bargraph;
                ui->addVerticalBargraph = ui_add_vertical_bargraph;
                ui->addSoundfile = ui_add_sound_file;
                ui->declare = ui_declare;
                ui->uiInterface = uiface;
                
                llvm_dsp *dsp = createCDSPInstance(faust_factories->factories[k]);

                buildUserInterfaceCDSPInstance(dsp, ui);

                initCDSPInstance(dsp, sample_rate);

                // initialize on known controls
                struct _fas_faust_ui_control *tmp;
                tmp = getFaustControl(uiface, "fs_freq");
                if (tmp) {
                    *tmp->zone = osc->freq;
                }

                tmp = getFaustControl(uiface, "fs_freq_prev");
                if (tmp) {
                    *tmp->zone = osc->prev_freq;
                }

                tmp = getFaustControl(uiface, "fs_freq_next");
                if (tmp) {
                    *tmp->zone = osc->next_freq;
                }

                tmp = getFaustControl(uiface, "fs_bw");
                if (tmp) {
                    *tmp->zone = osc->bw[i];
                }
                //

                osc->faust_gens[i][k]->controls = uiface;
                osc->faust_gens[i][k]->ui = ui;
                osc->faust_gens[i][k]->dsp = dsp;
            }
        }
    }
}

void freeFaustGenerators(
        struct oscillator **o,
        unsigned int n,
        unsigned int frame_data_count
    ) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return;
    }

    int y = 0, i = 0, k = 0;
    for (y = 0; y < n; y += 1) {
        for (i = 0; i < frame_data_count; i += 1) {
            for (k = 0; k < oscs[y].faust_gens_len; k += 1) {
                struct _fas_faust_dsp *fdsp = oscs[y].faust_gens[i][k];

                deleteCDSPInstance(fdsp->dsp);

                freeFaustControls(fdsp->controls);
                free(fdsp->ui);

                free(fdsp);
            }

            free(oscs[y].faust_gens[i]);
        }
        free(oscs[y].faust_gens);
    }
}
#endif

struct oscillator *createOscillatorsBank(
#ifdef WITH_SOUNDPIPE
    sp_data *spd,
#endif

    unsigned int n,
    double base_frequency,
    unsigned int octaves,
    unsigned int sample_rate,
    unsigned int wavetable_size,
    unsigned int frame_data_count) {
    struct oscillator *oscillators = (struct oscillator*)malloc(n * sizeof(struct oscillator));

    if (oscillators == NULL) {
        printf("createOscillators alloc. error.");
        fflush(stdout);
        return NULL;
    }

    int y = 0, i = 0, k = 0, j = 0;
    int partials = 0;
    int index = 0;
    double octave_length = (double)n / octaves;
    double frequency;
    double frequency_prev;
    double frequency_next;
    double phase_increment;
    uint64_t phase_step;
    int nmo = n - 1;

    double nyquist_limit = sample_rate / 2;

    double max_frequency = base_frequency * pow(2.0, nmo / octave_length);

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2.0, y / octave_length);
        frequency_prev = base_frequency * pow(2.0, (y - 1) / octave_length);
        frequency_next = base_frequency * pow(2.0, (y + 1) / octave_length);
        phase_step = frequency / (double)sample_rate * wavetable_size;
        phase_increment = frequency * 2 * 3.141592653589 / (double)sample_rate;

        struct oscillator *osc = &oscillators[index];

        osc->freq = frequency;
        osc->prev_freq = frequency_prev;
        osc->next_freq = frequency_next;

#ifdef FIXED_WAVETABLE
        osc->phase_index = malloc(sizeof(uint16_t) * frame_data_count);
        osc->phase_index2 = malloc(sizeof(uint16_t) * frame_data_count);
#else
        osc->phase_index = malloc(sizeof(unsigned int) * frame_data_count);
        osc->phase_index2 = malloc(sizeof(unsigned int) * frame_data_count);
#endif

#ifdef MAGIC_CIRCLE
        osc->mc_eps = 2. * sin(2. * 3.141592653589 * (frequency / (double)sample_rate) / 2.);
        osc->mc_x = malloc(sizeof(float) * frame_data_count);
        osc->mc_y = malloc(sizeof(float) * frame_data_count);
#endif

        osc->fphase = malloc(sizeof(double) * frame_data_count);

        // subtractive with additive synthesis
#ifndef POLYBLEP
        partials = fmin((max_frequency - frequency) / frequency, 128) + 1;
        osc->max_harmonics = partials;

    #ifdef FIXED_WAVETABLE
        osc->harmo_phase_step = malloc(sizeof(uint16_t) * (partials + 1));
        osc->harmo_phase_index = malloc(sizeof(uint16_t *) * frame_data_count);
    #else
        osc->harmo_phase_step = malloc(sizeof(unsigned int) * (partials + 1));
        osc->harmo_phase_index = malloc(sizeof(unsigned int *) * frame_data_count);
    #endif

        osc->harmonics = malloc(sizeof(float) * ((partials + 1) * 2));

        // == substrative specials
        int tri_sign = -1.0;
        for (i = 0; i <= partials; i += 1) {
            osc->harmo_phase_step[i] = (frequency * (i + 1)) / (double)sample_rate * wavetable_size;
            osc->harmonics[i] = (1.0 / (double)(i + 2.0));
            osc->harmonics[i + partials] = (1.0 / pow((double)(i + 2), 2.0)) * tri_sign;

            if (((i + 1) % 2) == 0) {
                tri_sign = -tri_sign;
            }
        }
#endif
        // ==
        osc->fp1 = malloc(sizeof(double *) * frame_data_count);
        osc->fp2 = malloc(sizeof(double *) * frame_data_count);
        osc->fp3 = malloc(sizeof(double *) * frame_data_count);
        osc->fp4 = malloc(sizeof(double *) * frame_data_count);

        osc->triggered = calloc(frame_data_count, sizeof(unsigned int));

        osc->buffer_len = (double)sample_rate / frequency;
        osc->buffer = malloc(sizeof(double) * osc->buffer_len * frame_data_count);

        osc->noise_index = malloc(sizeof(uint16_t) * frame_data_count);

        osc->pvalue = malloc(sizeof(float) * frame_data_count);

        osc->bw = malloc(sizeof(double) * frame_data_count);

#ifdef WITH_SOUNDPIPE
        osc->sp_filters = malloc(sizeof(void **) * frame_data_count);
        osc->sp_mods = malloc(sizeof(void **) * frame_data_count);
        osc->sp_gens = malloc(sizeof(void **) * frame_data_count);

        sp_ftbl_create(spd, (sp_ftbl **)&osc->ft_void, 1);
#endif

        for (i = 0; i < frame_data_count; i += 1) {
            osc->phase_index[i] = rand() / (double)RAND_MAX * wavetable_size;
            osc->noise_index[i] = rand() / (double)RAND_MAX * 65536;
            osc->pvalue[i] = 0;

#ifdef MAGIC_CIRCLE
            osc->mc_x[i] = 1;
            osc->mc_y[i] = 0;
#endif

            osc->bw[i] = (fabs(frequency - frequency_prev) + fabs(frequency - frequency_next));

#ifdef WITH_SOUNDPIPE
            // Soundpipe filters
            osc->sp_filters[i] = malloc(sizeof(void *) * (SP_OSC_FILTERS + 2)); // + 2 : adjust for stereo filters / gens (formant / modal)

            sp_moogladder_create((sp_moogladder **)&osc->sp_filters[i][SP_MOOG_FILTER]);
            sp_moogladder_init(spd, osc->sp_filters[i][SP_MOOG_FILTER]);

            sp_diode_create((sp_diode **)&osc->sp_filters[i][SP_DIODE_FILTER]);
            sp_diode_init(spd, osc->sp_filters[i][SP_DIODE_FILTER]);

            sp_wpkorg35_create((sp_wpkorg35 **)&osc->sp_filters[i][SP_KORG35_FILTER]);
            sp_wpkorg35_init(spd, osc->sp_filters[i][SP_KORG35_FILTER]);

            sp_streson_create((sp_streson **)&osc->sp_filters[i][SP_STRES_FILTER_L]);
            sp_streson_init(spd, osc->sp_filters[i][SP_STRES_FILTER_L]);

            sp_streson_create((sp_streson **)&osc->sp_filters[i][SP_STRES_FILTER_R]);
            sp_streson_init(spd, osc->sp_filters[i][SP_STRES_FILTER_R]);

            sp_streson *streson_l = (sp_streson *)osc->sp_filters[i][SP_STRES_FILTER_L];
            streson_l->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_streson *streson_r = (sp_streson *)osc->sp_filters[i][SP_STRES_FILTER_R];
            streson_r->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_lpf18_create((sp_lpf18 **)&osc->sp_filters[i][SP_LPF18_FILTER]);
            sp_lpf18_init(spd, osc->sp_filters[i][SP_LPF18_FILTER]);

            sp_fofilt_create((sp_fofilt **)&osc->sp_filters[i][SP_FORMANT_FILTER_L]);
            sp_fofilt_init(spd, osc->sp_filters[i][SP_FORMANT_FILTER_L]);

            sp_fofilt_create((sp_fofilt **)&osc->sp_filters[i][SP_FORMANT_FILTER_R]);
            sp_fofilt_init(spd, osc->sp_filters[i][SP_FORMANT_FILTER_R]);

            sp_fofilt *fofilt_l = (sp_fofilt *)osc->sp_filters[i][SP_FORMANT_FILTER_L];
            fofilt_l->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_fofilt *fofilt_r = (sp_fofilt *)osc->sp_filters[i][SP_FORMANT_FILTER_R];
            fofilt_r->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_mode_create((sp_mode **)&osc->sp_filters[i][SP_MODE_FILTER_L]);
            sp_mode_init(spd, osc->sp_filters[i][SP_MODE_FILTER_L]);

            sp_mode_create((sp_mode **)&osc->sp_filters[i][SP_MODE_FILTER_R]);
            sp_mode_init(spd, osc->sp_filters[i][SP_MODE_FILTER_R]);

            double stabilized_modal_frequency = (sample_rate / frequency) < M_PI ? sample_rate / M_PI - 1 : frequency;

            sp_mode *mode_l = (sp_mode *)osc->sp_filters[i][SP_MODE_FILTER_L];
            mode_l->freq = fmin(stabilized_modal_frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_mode *mode_r = (sp_mode *)osc->sp_filters[i][SP_MODE_FILTER_R];
            mode_r->freq = fmin(stabilized_modal_frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_butbp_create((sp_butbp **)&osc->sp_filters[i][SP_BANDPASS_FILTER_L]);
            sp_butbp_init(spd, osc->sp_filters[i][SP_BANDPASS_FILTER_L]);

            sp_butbp_create((sp_butbp **)&osc->sp_filters[i][SP_BANDPASS_FILTER_R]);
            sp_butbp_init(spd, osc->sp_filters[i][SP_BANDPASS_FILTER_R]);

            sp_butbp *bpb_l = (sp_butbp *)osc->sp_filters[i][SP_BANDPASS_FILTER_L];
            bpb_l->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);
            bpb_l->bw = osc->bw[i];

            sp_butbp *bpb_r = (sp_butbp *)osc->sp_filters[i][SP_BANDPASS_FILTER_R];
            bpb_r->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);
            bpb_r->bw = osc->bw[i];

            // Soundpipe generator
            osc->sp_gens[i] = malloc(sizeof(void *) * SP_OSC_GENS);

            sp_noise_create((sp_noise **)&osc->sp_gens[i][SP_WHITE_NOISE_GENERATOR]);
            sp_noise_init(spd, osc->sp_gens[i][SP_WHITE_NOISE_GENERATOR]);

            sp_pinknoise_create((sp_pinknoise **)&osc->sp_gens[i][SP_PINK_NOISE_GENERATOR]);
            sp_pinknoise_init(spd, osc->sp_gens[i][SP_PINK_NOISE_GENERATOR]);

            sp_brown_create((sp_brown **)&osc->sp_gens[i][SP_BROWN_NOISE_GENERATOR]);
            sp_brown_init(spd, osc->sp_gens[i][SP_BROWN_NOISE_GENERATOR]);

            sp_drip_create((sp_drip **)&osc->sp_gens[i][SP_DRIP_GENERATOR]);
            sp_drip_init(spd, osc->sp_gens[i][SP_DRIP_GENERATOR], 0.09f);

            sp_drip *drip = (sp_drip *)osc->sp_gens[i][SP_DRIP_GENERATOR];
            drip->amp = 1.f;
            drip->freq = fmin(frequency, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);

            sp_pdhalf_create((sp_pdhalf **)&osc->sp_gens[i][SP_PD_GENERATOR]);
            sp_pdhalf_init(spd, osc->sp_gens[i][SP_PD_GENERATOR]);

            // Soundpipe modifiers (generic effects)
            osc->sp_mods[i] = malloc(sizeof(void *) * SP_OSC_MODS);

            sp_bitcrush_create((sp_bitcrush **)&osc->sp_mods[i][SP_CRUSH_MODS]);
            sp_bitcrush_init(spd, osc->sp_mods[i][SP_CRUSH_MODS]);

            sp_dist_create((sp_dist **)&osc->sp_mods[i][SP_WAVSH_MODS]);
            sp_dist_init(spd, osc->sp_mods[i][SP_WAVSH_MODS]);

            sp_dist *dist = (sp_dist *)osc->sp_mods[i][SP_WAVSH_MODS];
            dist->pregain = 1.f;
            dist->postgain = 1.f;

            sp_fold_create((sp_fold **)&osc->sp_mods[i][SP_FOLD_MODS]);
            sp_fold_init(spd, osc->sp_mods[i][SP_FOLD_MODS]);

            sp_conv_create((sp_conv **)&osc->sp_mods[i][SP_CONV_MODS]);
            sp_conv_init(spd, osc->sp_mods[i][SP_CONV_MODS], osc->ft_void, 2048);  
#endif

            // == PM
            osc->phase_index2[i] = rand() / (double)RAND_MAX * wavetable_size;

            osc->fphase[i] = 0;

            // subtractive with additive synthesis
#ifndef POLYBLEP
    #ifdef FIXED_WAVETABLE
            osc->harmo_phase_index[i] = malloc(sizeof(uint16_t) * (partials + 1));
    #else
            osc->harmo_phase_index[i] = malloc(sizeof(unsigned int) * (partials + 1));
    #endif

            for (k = 0; k <= partials; k += 1) {
                osc->harmo_phase_index[i][k] = rand() / (double)RAND_MAX * wavetable_size;
            }
#endif
            // ==

            osc->fp1[i] = calloc(6, sizeof(double));
            osc->fp2[i] = calloc(6, sizeof(double));
            osc->fp3[i] = calloc(6, sizeof(double));
            osc->fp4[i] = calloc(6, sizeof(double));
        }

        osc->phase_step = phase_step;
        osc->phase_increment = phase_increment;
    }

    return oscillators;
}

struct oscillator *freeOscillatorsBank(struct oscillator **o, unsigned int n, unsigned int frame_data_count) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

#ifdef WITH_FAUST
    freeFaustGenerators(o, n, frame_data_count);
#endif

    int y = 0, i = 0, k = 0, j = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
        free(oscs[y].fphase);
        free(oscs[y].noise_index);
        free(oscs[y].pvalue);
        free(oscs[y].buffer);

        free(oscs[y].triggered);

        free(oscs[y].phase_index2);

#ifndef POLYBLEP
        free(oscs[y].harmo_phase_step);
        free(oscs[y].harmonics);
#endif

#ifdef MAGIC_CIRCLE
        free(oscs[y].mc_x);
        free(oscs[y].mc_y);
#endif

        free(oscs[y].bw);

        for (i = 0; i < frame_data_count; i += 1) {
#ifndef POLYBLEP
            free(oscs[y].harmo_phase_index[i]);
#endif

            free(oscs[y].fp1[i]);
            free(oscs[y].fp2[i]);
            free(oscs[y].fp3[i]);
            free(oscs[y].fp4[i]);
            
#ifdef WITH_SOUNDPIPE
            sp_moogladder_destroy((sp_moogladder **)&oscs[y].sp_filters[i][SP_MOOG_FILTER]);
            sp_diode_destroy((sp_diode **)&oscs[y].sp_filters[i][SP_DIODE_FILTER]);
            sp_wpkorg35_destroy((sp_wpkorg35 **)&oscs[y].sp_filters[i][SP_KORG35_FILTER]);
            sp_streson_destroy((sp_streson **)&oscs[y].sp_filters[i][SP_STRES_FILTER_L]);
            sp_streson_destroy((sp_streson **)&oscs[y].sp_filters[i][SP_STRES_FILTER_R]);
            sp_lpf18_destroy((sp_lpf18 **)&oscs[y].sp_filters[i][SP_LPF18_FILTER]);
            sp_fofilt_destroy((sp_fofilt **)&oscs[y].sp_filters[i][SP_FORMANT_FILTER_L]);
            sp_fofilt_destroy((sp_fofilt **)&oscs[y].sp_filters[i][SP_FORMANT_FILTER_R]);
            sp_mode_destroy((sp_mode **)&oscs[y].sp_filters[i][SP_MODE_FILTER_L]);
            sp_mode_destroy((sp_mode **)&oscs[y].sp_filters[i][SP_MODE_FILTER_R]);
            sp_butbp_destroy((sp_butbp **)&oscs[y].sp_filters[i][SP_BANDPASS_FILTER_L]);
            sp_butbp_destroy((sp_butbp **)&oscs[y].sp_filters[i][SP_BANDPASS_FILTER_R]);

            free(oscs[y].sp_filters[i]);

            sp_noise_destroy((sp_noise **)&oscs[y].sp_gens[i][SP_WHITE_NOISE_GENERATOR]);
            sp_pinknoise_destroy((sp_pinknoise **)&oscs[y].sp_gens[i][SP_PINK_NOISE_GENERATOR]);
            sp_brown_destroy((sp_brown **)&oscs[y].sp_gens[i][SP_BROWN_NOISE_GENERATOR]);
            sp_drip_destroy((sp_drip **)&oscs[y].sp_gens[i][SP_DRIP_GENERATOR]);
            sp_pdhalf_destroy((sp_pdhalf **)&oscs[y].sp_gens[i][SP_PD_GENERATOR]);
            
            free(oscs[y].sp_gens[i]);

            sp_bitcrush_destroy((sp_bitcrush **)&oscs[y].sp_mods[i][SP_CRUSH_MODS]);
            sp_dist_destroy((sp_dist **)&oscs[y].sp_mods[i][SP_WAVSH_MODS]);
            sp_fold_destroy((sp_fold **)&oscs[y].sp_mods[i][SP_FOLD_MODS]);
            sp_conv_destroy((sp_conv **)&oscs[y].sp_mods[i][SP_CONV_MODS]);

            free(oscs[y].sp_mods[i]);
#endif
        }

#ifndef POLYBLEP
        free(oscs[y].harmo_phase_index);
#endif
        free(oscs[y].fp1);
        free(oscs[y].fp2);
        free(oscs[y].fp3);
        free(oscs[y].fp4);

#ifdef WITH_SOUNDPIPE
        sp_ftbl_destroy((sp_ftbl **)&oscs[y].ft_void);

        free(oscs[y].sp_filters);
        free(oscs[y].sp_gens);
        free(oscs[y].sp_mods);
#endif
    }

    free(oscs);

    return NULL;
}
