#ifndef _FAS_EFFECTS_H_
#define _FAS_EFFECTS_H_

#ifdef WITH_SOUNDPIPE
    #include "soundpipe.h"
#endif

#ifdef WITH_FAUST
    #include "faust.h"
#endif

    #include <stdlib.h>
    #include "types.h"
    #include "samples.h"
    #include "tools.h"
    #include "constants.h"

    // synth. fx
    struct _synth_fx {
#ifdef WITH_SOUNDPIPE
        sp_zitarev *zitarev[FAS_MAX_FX_SLOTS];
        sp_revsc *revsc[FAS_MAX_FX_SLOTS];
        sp_autowah *autowah[FAS_MAX_FX_SLOTS * 2];
        sp_phaser *phaser[FAS_MAX_FX_SLOTS];
        sp_conv *conv[FAS_MAX_FX_SLOTS * 2];
        sp_vdelay *vdelay[FAS_MAX_FX_SLOTS * 2];
        sp_smoothdelay *sdelay[FAS_MAX_FX_SLOTS * 2];
        sp_comb *comb[FAS_MAX_FX_SLOTS * 2];
        sp_bitcrush *bitcrush[FAS_MAX_FX_SLOTS * 2];
        sp_dist *dist[FAS_MAX_FX_SLOTS * 2];
        sp_saturator *saturator[FAS_MAX_FX_SLOTS * 2];
        sp_compressor *compressor[FAS_MAX_FX_SLOTS * 2];
        sp_peaklim *peaklimit[FAS_MAX_FX_SLOTS * 2];
        sp_clip *clip[FAS_MAX_FX_SLOTS * 2];
        sp_butlp *butlp[FAS_MAX_FX_SLOTS * 2];
        sp_buthp *buthp[FAS_MAX_FX_SLOTS * 2];
        sp_butbp *butbp[FAS_MAX_FX_SLOTS * 2];
        sp_butbr *butbr[FAS_MAX_FX_SLOTS * 2];
        sp_pareq *pareq[FAS_MAX_FX_SLOTS * 2];
        sp_moogladder *mooglp[FAS_MAX_FX_SLOTS * 2];
        sp_diode *diodelp[FAS_MAX_FX_SLOTS * 2];
        sp_wpkorg35 *korglp[FAS_MAX_FX_SLOTS * 2];
        sp_lpf18 *lpf18[FAS_MAX_FX_SLOTS * 2];
        sp_tbvcf *tbvcf[FAS_MAX_FX_SLOTS * 2];

        sp_ftbl *ft_void;

#endif

        float dry[FAS_MAX_FX_SLOTS];
        float wet[FAS_MAX_FX_SLOTS];

#ifdef WITH_FAUST
        struct _fas_faust_dsp **faust_effs[FAS_MAX_FX_SLOTS];
        size_t faust_effs_len;
#endif
    };
    void createEffects(
#ifdef WITH_SOUNDPIPE
        sp_data *spd,
#endif
#ifdef WITH_FAUST
        struct _faust_factories *faust_factories,
#endif
        struct _synth_fx **fx, unsigned int frame_data_count, unsigned int sample_rate);
    void updateEffects(
#ifdef WITH_SOUNDPIPE
        sp_data *spd,
#endif
        struct _synth_fx *fxs, struct _synth_chn_settings *chns, struct sample *impulses, unsigned int impulses_count);

    void freeEffects(struct _synth_fx **fx, unsigned int frame_data_count);

#endif