#ifndef _FAS_EFFECTS_H_
#define _FAS_EFFECTS_H_

    #include <stdlib.h>

#ifdef WITH_SOUNDPIPE

    #include "soundpipe.h"
    #include "Soundpipe/sp.h"
#endif

#ifdef WITH_FAUST
    #include "faust.h"
#endif

    #include "types.h"
    #include "samples.h"
    #include "tools.h"
    #include "constants.h"

    // synth. fx
    struct _synth_fx {
#ifdef WITH_SOUNDPIPE
        sp_zitarev *zitarev[FAS_MAX_FX_SLOTS];
        sp_revsc *revsc[FAS_MAX_FX_SLOTS];
        sp_panst *panner[FAS_MAX_FX_SLOTS];
        sp_autowah *autowah[FAS_MAX_FX_SLOTS * 2];
        sp_phaser *phaser[FAS_MAX_FX_SLOTS];
        sp_conv *conv[FAS_MAX_FX_SLOTS * 2];
        sp_delay *delay[FAS_MAX_FX_SLOTS * 2];
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
        sp_fold *fold[FAS_MAX_FX_SLOTS * 2];
        sp_dcblock *dcblock[FAS_MAX_FX_SLOTS * 2];
        sp_lpc *lpc[FAS_MAX_FX_SLOTS * 2];
        sp_waveset *wset[FAS_MAX_FX_SLOTS * 2];

        sp_ftbl *ft_void;
#endif

        FAS_FLOAT dry[FAS_MAX_FX_SLOTS * 2];
        FAS_FLOAT wet[FAS_MAX_FX_SLOTS * 2];

#ifdef WITH_FAUST
        struct _fas_faust_dsp **faust_effs[FAS_MAX_FX_SLOTS];
        size_t faust_effs_len;
#endif
    };
    void createEffects(
#ifdef WITH_SOUNDPIPE
        sp_data *spd,
#endif
        struct _synth_fx **fx, unsigned int max_channels, unsigned int sample_rate);

    void updateEffectParameter(
#ifdef WITH_SOUNDPIPE
        sp_data *sp,
#endif 
        struct _synth_fx *fxs,
        struct _synth_chn_settings *chns,
        unsigned int slot,
        unsigned int target,
        FAS_FLOAT value);

    void resetConvolutions(
#ifdef WITH_SOUNDPIPE
        sp_data *sp,
#endif
        struct _synth_fx *fxs,
        struct _synth_chn_settings *chns,
        struct sample *impulses,
        unsigned int impulses_count);

#ifdef WITH_SOUNDPIPE
    int isValidConvPart(int x);

    void resetConvolution(
        sp_data *sp,
        struct _synth_fx *fxs,
        struct sample *impulses,
        unsigned int impulses_count,
        unsigned int slot,
        unsigned int lr,
        FAS_FLOAT v1,
        FAS_FLOAT v2
    );

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
    );

    void resetComb(
        sp_data *sp,
        struct _synth_fx *fxs,
        unsigned int slot,
        unsigned int lr,
        FAS_FLOAT v1,
        FAS_FLOAT v2
    );

    void resetLpc(
        sp_data *sp,
        struct _synth_fx *fxs,
        unsigned int slot,
        FAS_FLOAT v1
    );

    void resetWaveset(
        sp_data *sp,
        struct _synth_fx *fxs,
        unsigned int slot,
        FAS_FLOAT v1
    );
#endif

    void freeEffects(struct _synth_fx **fx, unsigned int max_channels);

#ifdef WITH_FAUST
    void createFaustEffects(struct _faust_factories *faust_factories, struct _synth_fx **fxs, unsigned int max_channels, unsigned int sample_rate);
    void freeFaustEffects(struct _synth_fx **fxs, unsigned int max_channels);
#endif

#endif