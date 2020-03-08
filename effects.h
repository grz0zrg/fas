#ifndef _FAS_EFFECTS_H_
#define _FAS_EFFECTS_H_

#ifdef WITH_SOUNDPIPE
    #include "soundpipe.h"
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
        sp_jcrev *jcrev[FAS_MAX_FX_SLOTS * 2];
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

        float dry[FAS_MAX_FX_SLOTS];
        float wet[FAS_MAX_FX_SLOTS];
#endif
    };

#ifdef WITH_SOUNDPIPE
    void createEffects(sp_data *spd, struct _synth_fx **fx, unsigned int frame_data_count);
    void updateEffects(sp_data *spd, struct _synth_fx *fxs, struct _synth_chn_settings *chns, struct sample *impulses, unsigned int impulses_count);
#else
    void createEffects(struct _synth_fx **fx, unsigned int frame_data_count);
    void updateEffects(struct _synth_fx *fxs, struct _synth_chn_settings *chns, struct sample *impulses, unsigned int impulses_count);
#endif
    void freeEffects(struct _synth_fx **fx, unsigned int frame_data_count);

#endif