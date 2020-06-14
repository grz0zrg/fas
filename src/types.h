#ifndef _FAS_TYPES_H_
#define _FAS_TYPES_H_

    #include <stdatomic.h>

    #include "afSTFT/afSTFTlib.h"

    #include "constants.h"

    struct _bank_settings {
        unsigned int h;
        unsigned int octave;
        unsigned int data_type;
        double base_frequency;
    };

    struct _synth_settings {
        double gain_lr;
        unsigned int fps;
    };

    // fx settings
    struct _synth_fx_settings {
        int fx_id;
        int bypass;

        double fp[FAS_MAX_FX_PARAMETERS]; // fx parameters
    };

    // channel settings / states
    struct _synth_chn_settings {
        unsigned int muted;

        FAS_FLOAT output_l;
        FAS_FLOAT output_r;

        FAS_FLOAT last_sample_l;
        FAS_FLOAT last_sample_r;

        FAS_FLOAT last_chn_gain;
        FAS_FLOAT curr_chn_gain;

        unsigned int mute_state;

        // channel fx
        struct _synth_fx_settings fx[FAS_MAX_FX_SLOTS];
    };

    // instruments related states
    // was quickly hacked for spectral states but should probably go into _synth_instrument (just like some oscillators stuff also :P)
    struct _synth_instrument_states {
        unsigned int position;

        // spectral related
        void *afSTFT_handle;

        complexVector stft_result[2];
        complexVector stft_temp[2];

        float *in[2];
        float *out[2];

        unsigned int hop_size;
    };

    // synth. instrument
    struct _synth_instrument {
        int type;
        unsigned int muted;

        unsigned int output_channel;

        int p0;
        double p1;
        double p2;
        double p3;
        double p4;
        
        FAS_FLOAT last_sample_l;
        FAS_FLOAT last_sample_r;
    };

    // synth. command
    struct _synth_command {
        unsigned int type;

        double value[4];
    };

    // synth. data
    struct _synth {
        // bank settings
        struct _bank_settings *bank_settings;
        // synth settings
        struct _synth_settings *settings;
        // oscillators bank
        struct oscillator *oscillators;
        // granular synthesis grains data
        struct grain *grains;
        // channels settings
        struct _synth_chn_settings *chn_settings;
        // instruments settings
        struct _synth_instrument instruments[FAS_MAX_INSTRUMENTS];
        
        int note;
        int chn;

        // pre-computed linear interpolation delta
        FAS_FLOAT lerp_t;

        // track current note-level sample (boundary defined by FPS)
        atomic_int curr_sample;
    } curr_synth;

    struct user_session_data {
        char peer_name[PEER_NAME_BUFFER_LENGTH];
        char peer_ip[PEER_ADDRESS_BUFFER_LENGTH];

        // contain either a fragmented packet or the final packet data
        char *packet;
        size_t packet_len;
        int packet_skip;

        int connected;

        // current / previous audio-frame data
        char *frame_data;
        char *prev_frame_data;
        size_t expected_frame_length;
        size_t expected_max_frame_length;

        unsigned int frame_data_size;

        // user session related synth. data
        double ***synth_chn_fx_settings;
        struct _synth_instrument instruments[FAS_MAX_INSTRUMENTS];

        struct oscillator *oscillators;

        unsigned int synth_h;
    };

    struct _frame_sync {
        uint64_t lasttime;
        double acc_time;
    } frame_sync;
#endif
