#include "effects.h"

void createEffects(
#ifdef WITH_SOUNDPIPE
    sp_data *spd,
#endif
    struct _synth_fx **fxs,
    unsigned int frame_data_count
) {
    int i = 0, j = 0, k = 0;

    for (i = 0; i < frame_data_count; i += 1) {
        fxs[i] = (struct _synth_fx *)malloc(sizeof(struct _synth_fx));

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
        }

        // no stereo support (so duplicate)
        for (j = 0; j < FAS_MAX_FX_SLOTS * 2; j += 2) {
            for (k = 0; k < 2; k += 1) {
                sp_jcrev_create((sp_jcrev **)&fx->jcrev[j + k]);
                sp_jcrev_init(spd, (sp_jcrev *)fx->jcrev[j + k]);

                sp_autowah_create((sp_autowah **)&fx->autowah[j + k]);
                sp_autowah_init(spd, (sp_autowah *)fx->autowah[j + k]);

                sp_conv_create((sp_conv **)&fx->conv[j + k]);
                sp_conv_init(spd, (sp_conv *)fx->conv[j + k], fx->ft_void, 256);

                sp_vdelay_create((sp_vdelay **)&fx->vdelay[j + k]);
                sp_vdelay_init(spd, (sp_vdelay *)fx->vdelay[j + k], 1.f);

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

// initialize / update effects which depend on given parameters
void updateEffects(
#ifdef WITH_SOUNDPIPE
    sp_data *sp,
#endif
    struct _synth_fx *fxs,
    struct _synth_chn_settings *chns,
    struct sample *impulses,
    unsigned int impulses_count) {
    unsigned int j = 0, f = 0, k = 0;

    for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
        struct _synth_fx_settings *fx = &chns->fx[j];

        if (fx->fx_id == -1) {
            break;
        }

#ifdef WITH_SOUNDPIPE
        if (fx->fx_id == FX_ZITAREV) {
            sp_zitarev *zita = (sp_zitarev *)fxs->zitarev[j];
            *zita->in_delay = fx->fp[0];
            *zita->lf_x = fx->fp[1];
            *zita->rt60_low = fx->fp[2];
            *zita->rt60_mid = fx->fp[3];
            *zita->hf_damping = fx->fp[4];
            *zita->eq1_freq = fx->fp[5];
            *zita->eq1_level = fx->fp[6];
            *zita->eq2_freq = fx->fp[7];
            *zita->eq2_level = fx->fp[8];
            *zita->mix = fx->fp[9];
            *zita->level = fx->fp[10];
        } else if (fx->fx_id == FX_PHASER) {
            sp_phaser *phaser = (sp_phaser *)fxs->phaser[j];
            *phaser->MaxNotch1Freq = fx->fp[0];
            *phaser->MinNotch1Freq = fx->fp[1];
            *phaser->Notch_width = fx->fp[2];
            *phaser->NotchFreq = fx->fp[3];
            *phaser->VibratoMode = fx->fp[4];
            *phaser->depth = fx->fp[5];
            *phaser->feedback_gain = fx->fp[6];
            *phaser->invert = fx->fp[7];
            *phaser->level = fx->fp[8];
            *phaser->lfobpm = fx->fp[9];
        } else if (fx->fx_id == FX_SCREV) {
            sp_revsc *revsc = (sp_revsc *)fxs->revsc[j];
            revsc->feedback = fx->fp[0];
            revsc->lpfreq = fx->fp[1];
        }
#endif
    }

    for (j = 0; j < FAS_MAX_FX_SLOTS * 2; j += 2) {
        struct _synth_fx_settings *fx = &chns->fx[f];

        if (fx->fx_id == -1) {
            break;
        }

        for (k = 0; k < 2; k += 1) {
#ifdef WITH_SOUNDPIPE
            if (fx->fx_id == FX_CONV) {
                sp_ftbl *imp_ftbl = fxs->ft_void;
                if (fx->fp[0] < impulses_count) {
                    struct sample *smp = &impulses[(unsigned int)fx->fp[0]];
                    imp_ftbl = smp->ftbl;
                }

                if (isPowerOfTwo(fx->fp[1]) == 0) {
                    fx->fp[1] = 1024;
                }

                sp_conv_destroy((sp_conv **)&fxs->conv[j + k]);
                sp_conv_create((sp_conv **)&fxs->conv[j + k]);

                sp_conv_init(sp, (sp_conv *)fxs->conv[j + k], imp_ftbl, fx->fp[1]);
            } else if (fx->fx_id == FX_VDELAY) {
                sp_vdelay_destroy((sp_vdelay **)&fxs->vdelay[j + k]);
                sp_vdelay_create((sp_vdelay **)&fxs->vdelay[j + k]);

                sp_vdelay_init(sp, (sp_vdelay *)fxs->vdelay[j + k], fx->fp[0]);

                sp_vdelay *vdelay = (sp_vdelay *)fxs->vdelay[j + k];
                vdelay->del = fx->fp[1];
            } else if (fx->fx_id == FX_SMOOTH_DELAY) {
                sp_smoothdelay_destroy((sp_smoothdelay **)&fxs->sdelay[j + k]);
                sp_smoothdelay_create((sp_smoothdelay **)&fxs->sdelay[j + k]);

                sp_smoothdelay_init(sp, (sp_smoothdelay *)fxs->sdelay[j + k], fx->fp[0], fx->fp[1]);

                sp_smoothdelay *sdelay = (sp_smoothdelay *)fxs->sdelay[j + k];
                sdelay->feedback = fx->fp[2];
                sdelay->del = fx->fp[3];
            } else if (fx->fx_id == FX_COMB) {
                sp_comb_destroy((sp_comb **)&fxs->comb[j + k]);
                sp_comb_create((sp_comb **)&fxs->comb[j + k]);

                sp_comb_init(sp, (sp_comb *)fxs->comb[j + k], fx->fp[0]);

                sp_comb *comb = (sp_comb *)fxs->comb[j + k];
                comb->revtime = fx->fp[1];
            } else if (fx->fx_id == FX_AUTOWAH) {
                sp_autowah *autowah = (sp_autowah *)fxs->autowah[j + k];
                *autowah->level = fx->fp[0];
                *autowah->wah = fx->fp[1];
                *autowah->mix = fx->fp[2];
            } else if (fx->fx_id == FX_BITCRUSH) {
                sp_bitcrush *bitcrush = (sp_bitcrush *)fxs->bitcrush[j + k];
                bitcrush->bitdepth = fx->fp[0];
                bitcrush->srate = fx->fp[1];
            } else if (fx->fx_id == FX_DISTORSION) {
                sp_dist *dist = (sp_dist *)fxs->dist[j + k];
                dist->pregain = fx->fp[0];
                dist->postgain = fx->fp[1];
                dist->shape1 = fx->fp[2];
                dist->shape2 = fx->fp[3];
            } else if (fx->fx_id == FX_SATURATOR) {
                sp_saturator *saturator = (sp_saturator *)fxs->saturator[j + k];
                saturator->drive = fx->fp[0];
                saturator->dcoffset = fx->fp[1];
            } else if (fx->fx_id == FX_COMPRESSOR) {
                sp_compressor *compressor = (sp_compressor *)fxs->compressor[j + k];
                *compressor->ratio = fx->fp[0];
                *compressor->thresh = fx->fp[1];
                *compressor->atk = fx->fp[2];
                *compressor->rel = fx->fp[3];
            } else if (fx->fx_id == FX_PEAK_LIMITER) {
                sp_peaklim *peaklimit = (sp_peaklim *)fxs->peaklimit[j + k];
                peaklimit->atk = fx->fp[0];
                peaklimit->rel = fx->fp[1];
                peaklimit->thresh = fx->fp[2];
            } else if (fx->fx_id == FX_CLIP) {
                sp_clip *clip = (sp_clip *)fxs->clip[j + k];
                clip->lim = fx->fp[0];
            } else if (fx->fx_id == FX_B_LOWPASS) {
                sp_butlp *blp = (sp_butlp *)fxs->butlp[j + k];
                blp->freq = fx->fp[0];
            } else if (fx->fx_id == FX_B_HIGHPASS) {
                sp_buthp *bhp = (sp_buthp *)fxs->buthp[j + k];
                bhp->freq = fx->fp[0];
            } else if (fx->fx_id == FX_B_BANDPASS) {
                sp_butbp *bbp = (sp_butbp *)fxs->butbp[j + k];
                bbp->freq = fx->fp[0];
                bbp->bw = fx->fp[1];
            } else if (fx->fx_id == FX_B_BANDREJECT) {
                sp_butbr *bbr = (sp_butbr *)fxs->butbr[j + k];
                bbr->freq = fx->fp[0];
                bbr->bw = fx->fp[1];
            } else if (fx->fx_id == FX_PAREQ) {
                sp_pareq *peq = (sp_pareq *)fxs->pareq[j + k];
                peq->fc = fx->fp[0];
                peq->v = fx->fp[1];
                peq->q = fx->fp[2];
                peq->mode = fx->fp[3];
            } else if (fx->fx_id == FX_MOOG_LPF) {
                sp_moogladder *mooglp = (sp_moogladder *)fxs->mooglp[j + k];
                mooglp->freq = fx->fp[0];
                mooglp->res = fx->fp[1];
            } else if (fx->fx_id == FX_DIODE_LPF) {
                sp_diode *diodelp = (sp_diode *)fxs->diodelp[j + k];
                diodelp->freq = fx->fp[0];
                diodelp->res = fx->fp[1];
            } else if (fx->fx_id == FX_KORG_LPF) {
                sp_wpkorg35 *korglp = (sp_wpkorg35 *)fxs->korglp[j + k];
                korglp->cutoff = fx->fp[0];
                korglp->res = fx->fp[1];
                korglp->saturation = fx->fp[2];
            } else if (fx->fx_id == FX_18_LPF) {
                sp_lpf18 *lpf18 = (sp_lpf18 *)fxs->lpf18[j + k];
                lpf18->cutoff = fx->fp[0];
                lpf18->res = fx->fp[1];
                lpf18->dist = fx->fp[2];
            } else if (fx->fx_id == FX_TBVCF) {
                sp_tbvcf *tbvcf = (sp_tbvcf *)fxs->tbvcf[j + k];
                tbvcf->fco = fx->fp[0];
                tbvcf->res = fx->fp[1];
                tbvcf->dist = fx->fp[2];
                tbvcf->asym = fx->fp[3];
            }
        #endif
        }

        f += 1;
    }
}

void freeEffects(struct _synth_fx **fxs, unsigned int frame_data_count) {
    if (fxs == NULL) {
        return;
    }

    int i = 0, j = 0, k = 0;

    for (i = 0; i < frame_data_count; i += 1) {
        struct _synth_fx *fx = fxs[i];

#ifdef WITH_SOUNDPIPE
        sp_ftbl_destroy((sp_ftbl **)&fx->ft_void);

        for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
            sp_zitarev_destroy((sp_zitarev **)&fx->zitarev[j]);
            sp_revsc_destroy((sp_revsc **)&fx->revsc[j]);

            sp_phaser_destroy((sp_phaser **)&fx->phaser[j]);
        }

        for (j = 0; j < FAS_MAX_FX_SLOTS * 2; j += 2) {
            for (k = 0; k < 2; k += 1) {
                sp_jcrev_destroy((sp_jcrev **)&fx->jcrev[j + k]);
                sp_autowah_destroy((sp_autowah **)&fx->autowah[j + k]);
                sp_conv_destroy((sp_conv **)&fx->conv[j + k]);
                sp_vdelay_destroy((sp_vdelay **)&fx->vdelay[j + k]);
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
            }
        }
#endif

        free(fxs[i]);
    }
}

