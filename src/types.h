#ifndef _FAS_TYPES_H_
#define _FAS_TYPES_H_

    #include "afSTFT/afSTFTlib.h"

    #include "constants.h"

    struct _synth_settings {
        unsigned int h;
        unsigned int octave;
        unsigned int data_type;
        double base_frequency;
    };

    struct _synth_gain {
        double gain_lr;
    };

    // fx settings
    struct _synth_fx_settings {
        int fx_id;
        int bypass;

        double fp[FAS_MAX_FX_PARAMETERS]; // fx parameters
    };

    // channel settings
    struct _synth_chn_settings {
        unsigned int synthesis_method;
        unsigned int muted;
        int p0;
        double p1;
        double p2;
        double p3;
        double p4;

        // channel fx
        struct _synth_fx_settings fx[FAS_MAX_FX_SLOTS];
    };

    // channels related states
    struct _synth_chn_states {
        unsigned int position;

        // spectral related
        void *afSTFT_handle;

        complexVector stft_result[2];
        complexVector stft_temp[2];

        float *in[2];
        float *out[2];

        unsigned int hop_size;
    };

    // synth. command
    struct _synth_command {
        unsigned int type;

        double value[4];
    };

    // synth. data
    struct _synth {
        // synth settings
        struct _synth_settings *settings;
        // gain settings
        struct _synth_gain *gain;
        // oscillators bank
        struct oscillator *oscillators;
        // granular synthesis grains data
        struct grain *grains;
        // channels settings
        struct _synth_chn_settings *chn_settings;
        
        int note;
        int chn;

        // pre-computed linear interpolation delta
        FAS_FLOAT lerp_t;

        // track current note-level sample (boundary defined by FPS)
        unsigned long curr_sample;
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
        double **synth_chn_settings;
        double ***synth_chn_fx_settings;

        struct oscillator *oscillators;

        unsigned int synth_h;
    };
#endif