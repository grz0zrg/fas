#include "effects.h"

#ifdef WITH_FAUST
void createFaustEffects(
    struct _faust_factories *faust_factories,
    struct _synth_fx **fxs,
    unsigned int frame_data_count,
    unsigned int sample_rate) {
    if (fxs == NULL || faust_factories == NULL) {
        return;
    }

    unsigned int i = 0, j = 0, k = 0;
    for (i = 0; i < frame_data_count; i += 1) {
        struct _synth_fx *fx = fxs[i];
        
        fx->faust_effs_len = faust_factories->len;

        for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
            fx->faust_effs[j] = (struct _fas_faust_dsp **)malloc(sizeof(struct _fas_faust_dsp **) * (faust_factories->len));
            
            for (k = 0; k < faust_factories->len; k += 1) {
                struct _fas_faust_dsp *fdsp = malloc(sizeof(struct _fas_faust_dsp));

                fx->faust_effs[j][k] = fdsp;

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
                /*
                struct _fas_faust_ui_control *tmp;
                tmp = getFaustControl(uiface, "fs_freq");
                if (tmp) {
                    *tmp->zone = frequency;
                }
                */
                //

                fdsp->controls = uiface;
                fdsp->ui = ui;
                fdsp->dsp = dsp;
            }
        }
    }
}
#endif

void createEffects(
#ifdef WITH_SOUNDPIPE
    sp_data *spd,
#endif
    struct _synth_fx **fxs,
    unsigned int frame_data_count,
    unsigned int sample_rate
) {
    unsigned int i = 0, j = 0, k = 0;

    for (i = 0; i < frame_data_count; i += 1) {
        fxs[i] = (struct _synth_fx *)calloc(1, sizeof(struct _synth_fx));

        struct _synth_fx *fx = fxs[i];

#ifdef WITH_SOUNDPIPE
        sp_ftbl_create(spd, (sp_ftbl **)&fx->ft_void, 1);

        // stereo support
        for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
            sp_zitarev_create((sp_zitarev **)&fx->zitarev[j]);
            sp_zitarev_init(spd, (sp_zitarev *)fx->zitarev[j]);

            sp_revsc_create((sp_revsc **)&fx->revsc[j]);
            sp_revsc_init(spd, (sp_revsc *)fx->revsc[j]);

            sp_phaser_create((sp_phaser **)&fx->phaser[j]);
            sp_phaser_init(spd, (sp_phaser *)fx->phaser[j]);

            sp_panst_create((sp_panst **)&fx->panner[j]);
            sp_panst_init(spd, (sp_panst *)fx->panner[j]);
        }

        // no stereo support (so duplicate)
        for (j = 0; j < FAS_MAX_FX_SLOTS * 2; j += 2) {
            for (k = 0; k < 2; k += 1) {
                sp_autowah_create((sp_autowah **)&fx->autowah[j + k]);
                sp_autowah_init(spd, (sp_autowah *)fx->autowah[j + k]);

                sp_conv_create((sp_conv **)&fx->conv[j + k]);
                sp_conv_init(spd, (sp_conv *)fx->conv[j + k], fx->ft_void, 256);

                sp_delay_create((sp_delay **)&fx->delay[j + k]);
                sp_delay_init(spd, (sp_delay *)fx->delay[j + k], 1.f);

                sp_smoothdelay_create((sp_smoothdelay **)&fx->sdelay[j + k]);
                sp_smoothdelay_init(spd, (sp_smoothdelay *)fx->sdelay[j + k], 1.f, 1024);

                sp_comb_create((sp_comb **)&fx->comb[j + k]);
                sp_comb_init(spd, (sp_comb *)fx->comb[j + k], 0.1f);

                sp_bitcrush_create((sp_bitcrush **)&fx->bitcrush[j + k]);
                sp_bitcrush_init(spd, (sp_bitcrush *)fx->bitcrush[j + k]);

                sp_dist_create((sp_dist **)&fx->dist[j + k]);
                sp_dist_init(spd, (sp_dist *)fx->dist[j + k]);

                sp_saturator_create((sp_saturator **)&fx->saturator[j + k]);
                sp_saturator_init(spd, (sp_saturator *)fx->saturator[j + k]);

                sp_compressor_create((sp_compressor **)&fx->compressor[j + k]);
                sp_compressor_init(spd, (sp_compressor *)fx->compressor[j + k]);

                sp_peaklim_create((sp_peaklim **)&fx->peaklimit[j + k]);
                sp_peaklim_init(spd, (sp_peaklim *)fx->peaklimit[j + k]);

                sp_clip_create((sp_clip **)&fx->clip[j + k]);
                sp_clip_init(spd, (sp_clip *)fx->clip[j + k]);

                sp_butlp_create((sp_butlp **)&fx->butlp[j + k]);
                sp_butlp_init(spd, (sp_butlp *)fx->butlp[j + k]);

                sp_buthp_create((sp_buthp **)&fx->buthp[j + k]);
                sp_buthp_init(spd, (sp_buthp *)fx->buthp[j + k]);

                sp_butbp_create((sp_butbp **)&fx->butbp[j + k]);
                sp_butbp_init(spd, (sp_butbp *)fx->butbp[j + k]);

                sp_butbr_create((sp_butbr **)&fx->butbr[j + k]);
                sp_butbr_init(spd, (sp_butbr *)fx->butbr[j + k]);

                sp_pareq_create((sp_pareq **)&fx->pareq[j + k]);
                sp_pareq_init(spd, (sp_pareq *)fx->pareq[j + k]);

                sp_fold_create((sp_fold **)&fx->fold[j + k]);
                sp_fold_init(spd, (sp_fold *)fx->fold[j + k]);

                sp_dcblock_create((sp_dcblock **)&fx->dcblock[j + k]);
                sp_dcblock_init(spd, (sp_dcblock *)fx->dcblock[j + k]);

                sp_lpc_create((sp_lpc **)&fx->lpc[j + k]);
                sp_lpc_init(spd, (sp_lpc *)fx->lpc[j + k], 512);

                sp_waveset_create((sp_waveset **)&fx->wset[j + k]);
                sp_waveset_init(spd, (sp_waveset *)fx->wset[j + k], 1);

                sp_moogladder_create((sp_moogladder **)&fx->mooglp[j + k]);
                sp_moogladder_init(spd, (sp_moogladder *)fx->mooglp[j + k]);

                sp_diode_create((sp_diode **)&fx->diodelp[j + k]);
                sp_diode_init(spd, (sp_diode *)fx->diodelp[j + k]);

                sp_wpkorg35_create((sp_wpkorg35 **)&fx->korglp[j + k]);
                sp_wpkorg35_init(spd, (sp_wpkorg35 *)fx->korglp[j + k]);

                sp_lpf18_create((sp_lpf18 **)&fx->lpf18[j + k]);
                sp_lpf18_init(spd, (sp_lpf18 *)fx->lpf18[j + k]);

                sp_tbvcf_create((sp_tbvcf **)&fx->tbvcf[j + k]);
                sp_tbvcf_init(spd, (sp_tbvcf *)fx->tbvcf[j + k]);
            }
        }
#endif
    }
}

void updateEffectParameter(
#ifdef WITH_SOUNDPIPE
    sp_data *sp,
#endif 
    struct _synth_fx *fxs,
    struct _synth_chn_settings *chns,
    unsigned int slot,
    unsigned int target,
    FAS_FLOAT value) {
    unsigned int k = 0;

    unsigned int slot2 = slot * 2;

    struct _synth_fx_settings *fx = &chns->fx[slot];

#ifdef WITH_SOUNDPIPE
    if (fx->fx_id == FX_SMOOTH_DELAY) {
        sp_smoothdelay *sdelay_l = (sp_smoothdelay *)fxs->sdelay[slot2];
        sp_smoothdelay *sdelay_r = (sp_smoothdelay *)fxs->sdelay[slot2 + 1];

        if (target == 4) {
            sdelay_l->feedback = value;
        } else if (target == 5) {
            sdelay_l->del = value;
        } else if (target == 8) {
            sdelay_r->feedback = value;
        } else if (target == 9) {
            sdelay_r->del = value;
        }
    } else if (fx->fx_id == FX_DELAY) {
        sp_delay *delay_l = (sp_delay *)fxs->delay[slot2];
        sp_delay *delay_r = (sp_delay *)fxs->delay[slot2 + 1];
        
        if (target == 3) {
            delay_l->feedback = value;
        } else if (target == 5) {
            delay_r->feedback = value;
        } else if (target == 6) {
            fxs->dry[slot2] = value;
        } else if (target == 7) {
            fxs->wet[slot2] = value;
        } else if (target == 8) {
            fxs->dry[slot2 + 1] = value;
        } else if (target == 9) {
            fxs->wet[slot2 + 1] = value;
        }
    } else if (fx->fx_id == FX_CONV) {
        if (target == 6) {
            fxs->dry[slot2] = value;
        } else if (target == 7) {
            fxs->wet[slot2] = value;
        } else if (target == 8) {
            fxs->dry[slot2 + 1] = value;
        } else if (target == 9) {
            fxs->wet[slot2 + 1] = value;
        }
    } else if (fx->fx_id == FX_ZITAREV) {
        sp_zitarev *zita = (sp_zitarev *)fxs->zitarev[slot];

        if (target == 2) {
            *zita->in_delay = value;

            return;
        } else if (target == 3) {
            *zita->lf_x = value;

            return;
        } else if (target == 4) {
            *zita->rt60_low = value;

            return;
        } else if (target == 5) {
            *zita->rt60_mid = value;
            
            return;
        } else if (target == 6) {
            *zita->hf_damping = value;

            return;
        } else if (target == 7) {
            *zita->eq1_freq = value;

            return;
        } else if (target == 8) {
            *zita->eq1_level = value;
            
            return;
        } else if (target == 9) {
            *zita->eq2_freq = value;

            return;
        } else if (target == 10) {
            *zita->eq2_level = value;

            return;
        } else if (target == 11) {
            *zita->mix = value;
            
            return;
        } else if (target == 12) {
            *zita->level = value;

            return;
        }
    } else if (fx->fx_id == FX_PHASER) {
        sp_phaser *phaser = (sp_phaser *)fxs->phaser[slot];

        if (target == 2) {
            *phaser->MaxNotch1Freq = value;

            return;
        } else if (target == 3) {
            *phaser->MinNotch1Freq = value;

            return;
        } else if (target == 4) {
            *phaser->Notch_width = value;

            return;
        } else if (target == 5) {
            *phaser->NotchFreq = value;

            return;
        } else if (target == 6) {
            *phaser->VibratoMode = value;

            return;
        } else if (target == 7) {
            *phaser->depth = value;
            
            return;
        } else if (target == 8) {
            *phaser->feedback_gain = value;

            return;
        } else if (target == 9) {
            *phaser->invert = value;

            return;
        } else if (target == 10) {
            *phaser->level = value;

            return;
        } else if (target == 11) {
            *phaser->lfobpm = value;

            return;
        }
    } else if (fx->fx_id == FX_SCREV) {
        sp_revsc *revsc = (sp_revsc *)fxs->revsc[slot];

        if (target == 2) {
            revsc->feedback = value;

            return;
        } else if (target == 3) {
            revsc->lpfreq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);

            return;
        }
    } else if (fx->fx_id == FX_COMB) {
        if (target == 3) {
            sp_comb *comb = (sp_comb *)fxs->comb[slot2];

            comb->revtime = value;
        } else if (target == 5) {
            sp_comb *comb = (sp_comb *)fxs->comb[slot2 + 1];

            comb->revtime = value;
        } else if (target == 6) {
            fxs->dry[slot2] = value;
        } else if (target == 7) {
            fxs->wet[slot2] = value;
        } else if (target == 8) {
            fxs->dry[slot2 + 1] = value;
        } else if (target == 9) {
            fxs->wet[slot2 + 1] = value;
        }
    } else if (fx->fx_id == FX_AUTOWAH) {
        sp_autowah *autowah_l = (sp_autowah *)fxs->autowah[slot2];
        sp_autowah *autowah_r = (sp_autowah *)fxs->autowah[slot2 + 1];

        if (target == 2) {
            *autowah_l->level = value;
        } else if (target == 3) {
            *autowah_l->wah = value;
        } else if (target == 4) {
            *autowah_l->mix = value;
        } else if (target == 5) {
            *autowah_r->level = value;
        } else if (target == 6) {
            *autowah_r->wah = value;
        } else if (target == 7) {
            *autowah_r->mix = value;
        }
    } else if (fx->fx_id == FX_PANNER) {
        sp_panst *panner = (sp_panst *)fxs->panner[slot];
        if (target == 2) {
            panner->type = value;
        } else if (target == 3) {
            panner->pan = value;
        }
    }
#endif
#ifdef WITH_FAUST
    if (fx->fx_id == FX_FAUST) {
        struct _fas_faust_dsp *fdsp = fxs->faust_effs[slot][(unsigned int)fx->fp[0]];

        struct _fas_faust_ui_control *tmp;
        if (target == 3) {
            tmp = getFaustControl(fdsp->controls, "fs_p0");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 4) {
            tmp = getFaustControl(fdsp->controls, "fs_p1");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 5) {
            tmp = getFaustControl(fdsp->controls, "fs_p2");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 6) {
            tmp = getFaustControl(fdsp->controls, "fs_p3");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 7) {
            tmp = getFaustControl(fdsp->controls, "fs_p4");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 8) {
            tmp = getFaustControl(fdsp->controls, "fs_p5");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 9) {
            tmp = getFaustControl(fdsp->controls, "fs_p6");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 10) {
            tmp = getFaustControl(fdsp->controls, "fs_p7");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 11) {
            tmp = getFaustControl(fdsp->controls, "fs_p8");
            if (tmp) { *tmp->zone = value; }

            return;
        } else if (target == 12) {
            tmp = getFaustControl(fdsp->controls, "fs_p9");
            if (tmp) { *tmp->zone = value; }

            return;
        }
    }
#endif

    // stereo fx (computed as stereo but parameters are the same for both channels)
    for (k = 0; k < 2; k += 1) {
        unsigned int slot_index = slot2 + k;
#ifdef WITH_SOUNDPIPE
        if (fx->fx_id == FX_BITCRUSH) {
            sp_bitcrush *bitcrush = (sp_bitcrush *)fxs->bitcrush[slot_index];

            if (target == 2) {
                bitcrush->bitdepth = value;
            } else if (target == 3) {
                bitcrush->srate = value;
            } else if (target == 4) {
                fxs->dry[slot_index] = value;
            } else if (target == 5) {
                fxs->wet[slot_index] = value;
            }
        } else if (fx->fx_id == FX_DISTORSION) {
            sp_dist *dist = (sp_dist *)fxs->dist[slot_index];

            if (target == 2) {
                dist->pregain = value;
            } else if (target == 3) {
                dist->postgain = value;
            } else if (target == 4) {
                dist->shape1 = value;
            } else if (target == 5) {
                dist->shape2 = value;
            } else if (target == 6) {
                fxs->dry[slot_index] = value;
            } else if (target == 7) {
                fxs->wet[slot_index] = value;
            }
        } else if (fx->fx_id == FX_SATURATOR) {
            sp_saturator *saturator = (sp_saturator *)fxs->saturator[slot_index];

            if (target == 2) {
                saturator->drive = value;
            } else if (target == 3) {
                saturator->dcoffset = value;
            } else if (target == 4) {
                fxs->dry[slot_index] = value;
            } else if (target == 5) {
                fxs->wet[slot_index] = value;
            }
        } else if (fx->fx_id == FX_COMPRESSOR) {
            sp_compressor *compressor = (sp_compressor *)fxs->compressor[slot_index];

            if (target == 2) {
                *compressor->ratio = value;
            } else if (target == 3) {
                *compressor->thresh = value;
            } else if (target == 4) {
                *compressor->atk = value;
            } else if (target == 5) {
                *compressor->rel = value;
            }
        } else if (fx->fx_id == FX_PEAK_LIMITER) {
            sp_peaklim *peaklimit = (sp_peaklim *)fxs->peaklimit[slot_index];

            if (target == 2) {
                peaklimit->atk = value;
            } else if (target == 3) {
                peaklimit->rel = value;
            } else if (target == 4) {
                peaklimit->thresh = value;
            } else if (target == 5) {
                fxs->dry[slot_index] = value;
            } else if (target == 6) {
                fxs->wet[slot_index] = value;
            }
        } else if (fx->fx_id == FX_CLIP) {
            sp_clip *clip = (sp_clip *)fxs->clip[slot_index];

            if (target == 2) {
                clip->lim = value;
            } else if (target == 3) {
                fxs->dry[slot_index] = value;
            } else if (target == 4) {
                fxs->wet[slot_index] = value;
            }
        } else if (fx->fx_id == FX_B_LOWPASS) {
            sp_butlp *blp = (sp_butlp *)fxs->butlp[slot_index];

            if (target == 2) {
                blp->freq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            }
        } else if (fx->fx_id == FX_B_HIGHPASS) {
            sp_buthp *bhp = (sp_buthp *)fxs->buthp[slot_index];

            if (target == 2) {
                bhp->freq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            }
        } else if (fx->fx_id == FX_B_BANDPASS) {
            sp_butbp *bbp = (sp_butbp *)fxs->butbp[slot_index];

            if (target == 2) {
                bbp->freq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                bbp->bw = value;
            }
        } else if (fx->fx_id == FX_B_BANDREJECT) {
            sp_butbr *bbr = (sp_butbr *)fxs->butbr[slot_index];
            
            if (target == 2) {
                bbr->freq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                bbr->bw = value;
            }
        } else if (fx->fx_id == FX_PAREQ) {
            sp_pareq *peq = (sp_pareq *)fxs->pareq[slot_index];

            if (target == 2) {
                peq->fc = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                peq->v = value;
            } else if (target == 4) {
                peq->q = value;
            } else if (target == 5) {
                peq->mode = value;
            }
        } else if (fx->fx_id == FX_MOOG_LPF) {
            sp_moogladder *mooglp = (sp_moogladder *)fxs->mooglp[slot_index];
            
            if (target == 2) {
                mooglp->freq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                mooglp->res = value;
            }
        } else if (fx->fx_id == FX_DIODE_LPF) {
            sp_diode *diodelp = (sp_diode *)fxs->diodelp[slot_index];

            if (target == 2) {
                diodelp->freq = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                diodelp->res = value;
            }
        } else if (fx->fx_id == FX_KORG_LPF) {
            sp_wpkorg35 *korglp = (sp_wpkorg35 *)fxs->korglp[slot_index];

            if (target == 2) {
                korglp->cutoff = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                korglp->res = value;
            } else if (target == 4) {
                korglp->saturation = value;
            }
        } else if (fx->fx_id == FX_18_LPF) {
            sp_lpf18 *lpf18 = (sp_lpf18 *)fxs->lpf18[slot_index];

            if (target == 2) {
                lpf18->cutoff = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                lpf18->res = value;
            } else if (target == 4) {
                lpf18->dist = value;
            }
        } else if (fx->fx_id == FX_FOLD) {
            sp_fold *fold = (sp_fold *)fxs->fold[slot_index];
            
            if (target == 2) {
                fold->incr = value;
            }
        } else if (fx->fx_id == FX_WAVESET) {
            sp_waveset *wset = (sp_waveset *)fxs->wset[slot_index];
            
            if (target == 3) {
                wset->rep = value;
            }
        } else if (fx->fx_id == FX_TBVCF) {
            sp_tbvcf *tbvcf = (sp_tbvcf *)fxs->tbvcf[slot_index];
            
            if (target == 2) {
                tbvcf->fco = fmin(value, sp->sr / 2 * FAS_FREQ_LIMIT_FACTOR);
            } else if (target == 3) {
                tbvcf->res = value;
            } else if (target == 4) {
                tbvcf->dist = value;
            } else if (target == 5) {
                tbvcf->asym = value;
            }
        }
#endif
    }
}

#ifdef WITH_SOUNDPIPE
void resetConvolution(
    sp_data *sp,
    struct _synth_fx *fxs,
    struct sample *impulses,
    unsigned int impulses_count,
    unsigned int slot,
    unsigned int lr,
    FAS_FLOAT v1,
    FAS_FLOAT v2
) {
    sp_ftbl *imp_ftbl = fxs->ft_void;
    if (v1 < impulses_count) {
        struct sample *smp = &impulses[(unsigned int)v1];
        imp_ftbl = smp->ftbl;
    }

    unsigned int slot_index = slot * 2 + lr;

    sp_conv_destroy((sp_conv **)&fxs->conv[slot_index]);
    sp_conv_create((sp_conv **)&fxs->conv[slot_index]);

    sp_conv_init(sp, (sp_conv *)fxs->conv[slot_index], imp_ftbl, v2);
}

void resetDelays(
    sp_data *sp,
    struct _synth_fx *fxs,
    unsigned int slot,
    unsigned int type,
    unsigned int lr,
    FAS_FLOAT v1,
    FAS_FLOAT v2,
    FAS_FLOAT v3,
    FAS_FLOAT v4
) {
    unsigned int slot_index = slot * 2 + lr;

    if (type == 0) {
        sp_delay_destroy((sp_delay **)&fxs->delay[slot_index]);
        sp_delay_create((sp_delay **)&fxs->delay[slot_index]);

        sp_delay_init(sp, (sp_delay *)fxs->delay[slot_index], v1);

        sp_delay *delay = (sp_delay *)fxs->delay[slot_index];
        delay->feedback = v2;
    } else if (type == 1) {
        sp_smoothdelay_destroy((sp_smoothdelay **)&fxs->sdelay[slot_index]);
        sp_smoothdelay_create((sp_smoothdelay **)&fxs->sdelay[slot_index]);

        sp_smoothdelay_init(sp, (sp_smoothdelay *)fxs->sdelay[slot_index], v1, v2);

        sp_smoothdelay *sdelay = (sp_smoothdelay *)fxs->sdelay[slot_index];
        sdelay->feedback = v3;
        sdelay->del = v4;
    }
}

void resetComb(
    sp_data *sp,
    struct _synth_fx *fxs,
    unsigned int slot,
    unsigned int lr,
    FAS_FLOAT v1,
    FAS_FLOAT v2
) {
    unsigned int slot_index = slot * 2 + lr;

    sp_comb_destroy((sp_comb **)&fxs->comb[slot_index]);
    sp_comb_create((sp_comb **)&fxs->comb[slot_index]);

    sp_comb_init(sp, (sp_comb *)fxs->comb[slot_index], v1);

    sp_comb *comb = (sp_comb *)fxs->comb[slot_index];
    comb->revtime = v2;
}

void resetLpc(
    sp_data *sp,
    struct _synth_fx *fxs,
    unsigned int slot,
    FAS_FLOAT v1
) {
    unsigned int k = 0;
    for (k = 0; k < 2; k += 1) {
        unsigned int slot_index = slot * 2 + k;

        sp_lpc_destroy((sp_lpc **)&fxs->lpc[slot_index]);
        sp_lpc_create((sp_lpc **)&fxs->lpc[slot_index]);

        sp_lpc_init(sp, (sp_lpc *)fxs->lpc[slot_index], v1);
    }
}

void resetWaveset(
    sp_data *sp,
    struct _synth_fx *fxs,
    unsigned int slot,
    FAS_FLOAT v1
) {
    unsigned int k = 0;
    for (k = 0; k < 2; k += 1) {
        unsigned int slot_index = slot * 2 + k;

        sp_waveset_destroy((sp_waveset **)&fxs->wset[slot_index]);
        sp_waveset_create((sp_waveset **)&fxs->wset[slot_index]);

        sp_waveset_init(sp, (sp_waveset *)fxs->wset[slot_index], v1);
    }
}
#endif

void resetConvolutions(
#ifdef WITH_SOUNDPIPE
    sp_data *sp,
#endif
    struct _synth_fx *fxs,
    struct _synth_chn_settings *chns,
    struct sample *impulses,
    unsigned int impulses_count) {
    unsigned int j = 0, f = 0, k = 0;
    
    for (j = 0; j < FAS_MAX_FX_SLOTS * 2; j += 2) {
        struct _synth_fx_settings *fx = &chns->fx[f];

        if (fx->fx_id == -1) {
            break;
        }

#ifdef WITH_SOUNDPIPE
        if (fx->fx_id == FX_CONV) {
            resetConvolution(sp, fxs, impulses, impulses_count, f, 0, fx->fp[0], fx->fp[1]);
            resetConvolution(sp, fxs, impulses, impulses_count, f, 1, fx->fp[2], fx->fp[3]);
        }
#endif

        f += 1;
    }
}

#ifdef WITH_FAUST
void freeFaustEffects(struct _synth_fx **fxs, unsigned int frame_data_count) {
    if (fxs == NULL) {
        return;
    }

    unsigned int i = 0, j = 0, k = 0;
    for (i = 0; i < frame_data_count; i += 1) {
        struct _synth_fx *fx = fxs[i];

        for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
            for (k = 0; k < fx->faust_effs_len; k += 1) {
                struct _fas_faust_dsp *fdsp = fx->faust_effs[j][k];

                freeFaustControls(fdsp->controls);
                free(fdsp->ui);

                deleteCDSPInstance(fdsp->dsp);

                free(fdsp);
            }

            free(fx->faust_effs[j]);
        }
    }
}
#endif

void freeEffects(struct _synth_fx **fxs, unsigned int frame_data_count) {
    if (fxs == NULL) {
        return;
    }

#ifdef WITH_FAUST
    freeFaustEffects(fxs, frame_data_count);
#endif

    unsigned int i = 0, j = 0, k = 0;
    for (i = 0; i < frame_data_count; i += 1) {
        struct _synth_fx *fx = fxs[i];

#ifdef WITH_SOUNDPIPE
        sp_ftbl_destroy((sp_ftbl **)&fx->ft_void);

        for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
            sp_zitarev_destroy((sp_zitarev **)&fx->zitarev[j]);
            sp_revsc_destroy((sp_revsc **)&fx->revsc[j]);

            sp_phaser_destroy((sp_phaser **)&fx->phaser[j]);
            sp_panst_destroy((sp_panst **)&fx->panner[j]);
        }

        for (j = 0; j < FAS_MAX_FX_SLOTS * 2; j += 2) {
            for (k = 0; k < 2; k += 1) {
                sp_autowah_destroy((sp_autowah **)&fx->autowah[j + k]);
                sp_conv_destroy((sp_conv **)&fx->conv[j + k]);
                sp_delay_destroy((sp_delay **)&fx->delay[j + k]);
                sp_smoothdelay_destroy((sp_smoothdelay **)&fx->sdelay[j + k]);
                sp_comb_destroy((sp_comb **)&fx->comb[j + k]);
                sp_bitcrush_destroy((sp_bitcrush **)&fx->bitcrush[j + k]);
                sp_dist_destroy((sp_dist **)&fx->dist[j + k]);
                sp_saturator_destroy((sp_saturator **)&fx->saturator[j + k]);
                sp_compressor_destroy((sp_compressor **)&fx->compressor[j + k]);
                sp_peaklim_destroy((sp_peaklim **)&fx->peaklimit[j + k]);
                sp_clip_destroy((sp_clip **)&fx->clip[j + k]);
                sp_butlp_destroy((sp_butlp **)&fx->butlp[j + k]);
                sp_buthp_destroy((sp_buthp **)&fx->buthp[j + k]);
                sp_butbp_destroy((sp_butbp **)&fx->butbp[j + k]);
                sp_butbr_destroy((sp_butbr **)&fx->butbr[j + k]);
                sp_pareq_destroy((sp_pareq **)&fx->pareq[j + k]);
                sp_moogladder_destroy((sp_moogladder **)&fx->mooglp[j + k]);
                sp_diode_destroy((sp_diode **)&fx->diodelp[j + k]);
                sp_wpkorg35_destroy((sp_wpkorg35 **)&fx->korglp[j + k]);
                sp_lpf18_destroy((sp_lpf18 **)&fx->lpf18[j + k]);
                sp_tbvcf_destroy((sp_tbvcf **)&fx->tbvcf[j + k]);
                sp_fold_destroy((sp_fold **)&fx->fold[j + k]);
                sp_dcblock_destroy((sp_dcblock **)&fx->dcblock[j + k]);
                sp_lpc_destroy((sp_lpc **)&fx->lpc[j + k]);
                sp_waveset_destroy((sp_waveset **)&fx->wset[j + k]);
            }
        }
#endif

        free(fxs[i]);
    }
}
