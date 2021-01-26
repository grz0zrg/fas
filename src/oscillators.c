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
    unsigned int max_instruments) {
    if (!osc_bank || faust_factories == NULL) {
        return;
    }

    unsigned int y = 0, i = 0, k = 0;

    int nmo = n - 1;
    for (y = 0; y < n; y += 1) {
        int index = nmo - y;

        struct oscillator *osc = &osc_bank[index];

        osc->faust_gens_len = faust_factories->len;
        osc->faust_gens = calloc(max_instruments, sizeof(struct _fas_faust_dsp **));

        if (osc->faust_gens_len == 0) {
            continue;
        }

        for (i = 0; i < max_instruments; i += 1) {
            osc->faust_gens[i] = calloc(faust_factories->len, sizeof(struct _fas_faust_dsp *));
            for (k = 0; k < faust_factories->len; k += 1) {
                osc->faust_gens[i][k] = calloc(1, sizeof(struct _fas_faust_dsp));

                struct _fas_faust_ui_control *uiface = calloc(1, sizeof(struct _fas_faust_ui_control));
                
                UIGlue *ui = calloc(1, sizeof(UIGlue));
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
        unsigned int max_instruments
    ) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return;
    }

    unsigned int y = 0, i = 0, k = 0;
    for (y = 0; y < n; y += 1) {
        if (oscs[y].faust_gens == NULL) {
            continue;
        }

        for (i = 0; i < max_instruments; i += 1) {
            if (oscs[y].faust_gens[i] == NULL) {
                continue;
            }

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
    unsigned int max_instruments) {
    struct oscillator *oscillators = (struct oscillator*)calloc(n, sizeof(struct oscillator));

    if (oscillators == NULL) {
        printf("createOscillators alloc. error.");
        fflush(stdout);
        return NULL;
    }

    unsigned int y = 0, i = 0, k = 0, j = 0;
    int partials = 0;
    int index = 0;
    FAS_FLOAT octave_length = (FAS_FLOAT)n / octaves;
    FAS_FLOAT frequency;
    FAS_FLOAT frequency_prev;
    FAS_FLOAT frequency_next;
    FAS_FLOAT phase_increment;
    FAS_FLOAT phase_step;
    int nmo = n - 1;

    FAS_FLOAT nyquist_limit = sample_rate / 2;

    FAS_FLOAT max_frequency = base_frequency * pow(2.0, nmo / octave_length);

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2.0, y / octave_length);
        frequency_prev = base_frequency * pow(2.0, (y - 1) / octave_length);
        frequency_next = base_frequency * pow(2.0, (y + 1) / octave_length);
        phase_step = frequency / (FAS_FLOAT)sample_rate * wavetable_size;
        phase_increment = frequency * 2 * 3.141592653589 / (FAS_FLOAT)sample_rate;

        struct oscillator *osc = &oscillators[index];

        osc->freq = frequency;
        osc->prev_freq = frequency_prev;
        osc->next_freq = frequency_next;

        osc->phase_index = malloc(sizeof(FAS_FLOAT) * max_instruments);
        osc->phase_index2 = malloc(sizeof(FAS_FLOAT) * max_instruments);

#ifdef MAGIC_CIRCLE
        osc->mc_eps = 2. * sin(2. * 3.141592653589 * (frequency / (FAS_FLOAT)sample_rate) / 2.);
        osc->mc_x = malloc(sizeof(FAS_FLOAT) * max_instruments);
        osc->mc_y = malloc(sizeof(FAS_FLOAT) * max_instruments);
#endif

        osc->fphase = malloc(sizeof(FAS_FLOAT) * max_instruments);

        // ==
        osc->fp1 = malloc(sizeof(FAS_FLOAT *) * max_instruments);
        osc->fp2 = malloc(sizeof(FAS_FLOAT *) * max_instruments);
        osc->fp3 = malloc(sizeof(FAS_FLOAT *) * max_instruments);
        osc->fp4 = malloc(sizeof(FAS_FLOAT *) * max_instruments);

        osc->wav1 = malloc(sizeof(FAS_FLOAT *) * max_instruments);
        osc->wav2 = malloc(sizeof(FAS_FLOAT *) * max_instruments);

        osc->triggered = calloc(max_instruments, sizeof(unsigned int));

        osc->buffer_len = (FAS_FLOAT)sample_rate / frequency;
        osc->buffer = malloc(sizeof(FAS_FLOAT) * osc->buffer_len * max_instruments);

        osc->noise_index = malloc(sizeof(uint16_t) * max_instruments);

        osc->pvalue = malloc(sizeof(FAS_FLOAT) * max_instruments);

        osc->bw = malloc(sizeof(FAS_FLOAT) * max_instruments);

#ifdef WITH_SOUNDPIPE
        osc->sp_filters = malloc(sizeof(void **) * max_instruments);
        osc->sp_mods = malloc(sizeof(void **) * max_instruments);
        osc->sp_gens = malloc(sizeof(void **) * max_instruments);

        sp_ftbl_create(spd, (sp_ftbl **)&osc->ft_void, 1);
#endif

        for (i = 0; i < max_instruments; i += 1) {
            osc->phase_index[i] = rand() / (FAS_FLOAT)RAND_MAX * wavetable_size;
            osc->noise_index[i] = rand() / (FAS_FLOAT)RAND_MAX * 65536;
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

            FAS_FLOAT stabilized_modal_frequency = (sample_rate / frequency) < M_PI ? sample_rate / M_PI - 1 : frequency;

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

            SPFLOAT stiffness = frequency * 2.41 / 10;
            sp_bar_create((sp_bar **)&osc->sp_gens[i][SP_BAR_GENERATOR]);
            sp_bar_init(spd, osc->sp_gens[i][SP_BAR_GENERATOR], stiffness, 0.001f);

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
#endif

            // == PM
            osc->phase_index2[i] = rand() / (FAS_FLOAT)RAND_MAX * wavetable_size;

            osc->fphase[i] = 0;

            osc->fp1[i] = calloc(6, sizeof(FAS_FLOAT));
            osc->fp2[i] = calloc(6, sizeof(FAS_FLOAT));
            osc->fp3[i] = calloc(6, sizeof(FAS_FLOAT));
            osc->fp4[i] = calloc(6, sizeof(FAS_FLOAT));
        }

        osc->phase_step = phase_step;
        osc->phase_increment = phase_increment;
    }

    return oscillators;
}

struct oscillator *updateOscillatorBank(
#ifdef WITH_SOUNDPIPE
    sp_data *spd,
#endif
    struct oscillator **o,
    unsigned int n,
    unsigned int max_instruments,
    unsigned int sample_rate,
    int target,
    FAS_FLOAT value1,
    FAS_FLOAT value2) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

    FAS_FLOAT nyquist_limit = sample_rate / 2;

    unsigned int y = 0, i = 0, k = 0, j = 0;
    for (y = 0; y < n; y += 1) {
        struct oscillator *osc = &oscs[n - 1 - y];
        for (i = 0; i < max_instruments; i += 1) {
#ifdef WITH_SOUNDPIPE
            if (target == 0) {
                sp_drip_destroy((sp_drip **)&oscs[y].sp_gens[i][SP_DRIP_GENERATOR]);

                sp_drip_create((sp_drip **)&osc->sp_gens[i][SP_DRIP_GENERATOR]);
                sp_drip_init(spd, osc->sp_gens[i][SP_DRIP_GENERATOR], value1);

                sp_drip *drip = (sp_drip *)osc->sp_gens[i][SP_DRIP_GENERATOR];
                drip->amp = 1.f;
                drip->freq = fmin(osc->freq, nyquist_limit * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 1) {
                sp_bar_destroy((sp_bar **)&oscs[y].sp_gens[i][SP_BAR_GENERATOR]);

                SPFLOAT stiffness;
                SPFLOAT imsec = 2.41;
                if (value2 == 1) {
                    if (value1 == 1) {
                        imsec = 2.41;
                    } else if (value1 == 2) {
                        imsec = 3.825;
                    } else {
                        imsec = 2.88;
                    }
                } else {
                    if (value1 == 1) {
                        imsec = 3.825;
                    } else if (value1 == 2) {
                        imsec = 3.2;
                    } else {
                        imsec = 4.398;
                    }
                }
                stiffness = osc->freq * imsec / 10;

                sp_bar_create((sp_bar **)&osc->sp_gens[i][SP_BAR_GENERATOR]);
                sp_bar_init(spd, osc->sp_gens[i][SP_BAR_GENERATOR], stiffness, 0.001f);
            }
#endif
        }
    }
}

struct oscillator *freeOscillatorsBank(struct oscillator **o, unsigned int n, unsigned int max_instruments) {
    struct oscillator *oscs = *o;

    if (oscs == NULL) {
        return NULL;
    }

#ifdef WITH_FAUST
    freeFaustGenerators(o, n, max_instruments);
#endif

    unsigned int y = 0, i = 0, k = 0, j = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
        free(oscs[y].fphase);
        free(oscs[y].noise_index);
        free(oscs[y].pvalue);
        free(oscs[y].buffer);

        free(oscs[y].triggered);

        free(oscs[y].phase_index2);

#ifdef MAGIC_CIRCLE
        free(oscs[y].mc_x);
        free(oscs[y].mc_y);
#endif

        free(oscs[y].bw);

        for (i = 0; i < max_instruments; i += 1) {
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
            sp_bar_destroy((sp_bar **)&oscs[y].sp_gens[i][SP_BAR_GENERATOR]);
            sp_drip_destroy((sp_drip **)&oscs[y].sp_gens[i][SP_DRIP_GENERATOR]);
            sp_pdhalf_destroy((sp_pdhalf **)&oscs[y].sp_gens[i][SP_PD_GENERATOR]);
            
            free(oscs[y].sp_gens[i]);

            sp_bitcrush_destroy((sp_bitcrush **)&oscs[y].sp_mods[i][SP_CRUSH_MODS]);
            sp_dist_destroy((sp_dist **)&oscs[y].sp_mods[i][SP_WAVSH_MODS]);

            free(oscs[y].sp_mods[i]);
#endif
        }

        free(oscs[y].fp1);
        free(oscs[y].fp2);
        free(oscs[y].fp3);
        free(oscs[y].fp4);

        free(oscs[y].wav1);
        free(oscs[y].wav2);

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
