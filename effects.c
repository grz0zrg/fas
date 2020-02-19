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
    int j = 0;
    for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
        struct _synth_fx_settings *fx = &chns->fx[j];

        if (fx->fx_id == -1) {
            break;
        }
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

            sp_conv_destroy((sp_conv **)&fxs->conv[j]);
            sp_conv_destroy((sp_conv **)&fxs->conv[j + 1]);
            sp_conv_create((sp_conv **)&fxs->conv[j]);
            sp_conv_create((sp_conv **)&fxs->conv[j + 1]);

            sp_conv_init(sp, (sp_conv *)fxs->conv[j], imp_ftbl, fx->fp[1]);
            sp_conv_init(sp, (sp_conv *)fxs->conv[j + 1], imp_ftbl, fx->fp[1]);
        } else if (fx->fx_id == FX_VDELAY) {
            sp_vdelay_destroy((sp_vdelay **)&fxs->vdelay[j]);
            sp_vdelay_destroy((sp_vdelay **)&fxs->vdelay[j + 1]);
            sp_vdelay_create((sp_vdelay **)&fxs->vdelay[j]);
            sp_vdelay_create((sp_vdelay **)&fxs->vdelay[j + 1]);

            sp_vdelay_init(sp, (sp_vdelay *)fxs->vdelay[j], fx->fp[0]);
            sp_vdelay_init(sp, (sp_vdelay *)fxs->vdelay[j + 1], fx->fp[0]);

            sp_vdelay *vdelay = (sp_vdelay *)fxs->vdelay[j];
            vdelay->del = fx->fp[1];
            vdelay = (sp_vdelay *)fxs->vdelay[j + 1];
            vdelay->del = fx->fp[1];
        } else if (fx->fx_id == FX_SMOOTH_DELAY) {
            sp_smoothdelay_destroy((sp_smoothdelay **)&fxs->sdelay[j]);
            sp_smoothdelay_destroy((sp_smoothdelay **)&fxs->sdelay[j + 1]);
            sp_smoothdelay_create((sp_smoothdelay **)&fxs->sdelay[j]);
            sp_smoothdelay_create((sp_smoothdelay **)&fxs->sdelay[j + 1]);

            sp_smoothdelay_init(sp, (sp_smoothdelay *)fxs->sdelay[j], fx->fp[0], fx->fp[1]);
            sp_smoothdelay_init(sp, (sp_smoothdelay *)fxs->sdelay[j + 1], fx->fp[0], fx->fp[1]);

            sp_smoothdelay *sdelay = (sp_smoothdelay *)fxs->sdelay[j];
            sdelay->feedback = fx->fp[2];
            sdelay->del = fx->fp[3];
            sdelay = (sp_smoothdelay *)fxs->sdelay[j + 1];
            sdelay->feedback = fx->fp[2];
            sdelay->del = fx->fp[3];
        } else if (fx->fx_id == FX_COMB) {
            sp_comb_destroy((sp_comb **)&fxs->comb[j]);
            sp_comb_destroy((sp_comb **)&fxs->comb[j + 1]);
            sp_comb_create((sp_comb **)&fxs->comb[j]);
            sp_comb_create((sp_comb **)&fxs->comb[j + 1]);

            sp_comb_init(sp, (sp_comb *)fxs->comb[j], fx->fp[0]);
            sp_comb_init(sp, (sp_comb *)fxs->comb[j + 1], fx->fp[0]);

            sp_comb *comb = (sp_comb *)fxs->comb[j];
            comb->revtime = fx->fp[1];
            comb = (sp_comb *)fxs->comb[j + 1];
            comb->revtime = fx->fp[1];
        } else if (fx->fx_id == FX_AUTOWAH) {
            sp_autowah *autowah = (sp_autowah *)fxs->autowah[j];
            *autowah->level = fx->fp[0];
            *autowah->wah = fx->fp[1];
            *autowah->mix = fx->fp[2];
        }
    #endif
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
            }
        }
#endif

        free(fxs[i]);
    }
}

