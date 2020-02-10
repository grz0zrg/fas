#ifndef _FAS_TYPES_H_
#define _FAS_TYPES_H_

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

        int ip0;
        int ip1;
        int ip2;
        int ip3;

        double fp0;
        double fp1;
        double fp2;
        double fp3;
        double fp4;
        double fp5;
    };

    // channel settings
    struct _synth_chn_settings {
        unsigned int synthesis_method;
        int p0;
        double p1;
        double p2;
        double p3;

        // channel fx
        struct _synth_fx_settings fx[FAS_MAX_FX_SLOTS];
    };

    // synth. data
    struct _synth {
        struct _synth_settings *settings;
        struct _synth_gain *gain;
        struct oscillator *oscillators;
        struct grain *grains;
        struct _synth_chn_settings *chn_settings;
        
        int note;
        int chn;

        unsigned int samples_count;

        float lerp_t;
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
        struct _synth *synth;

        struct oscillator *oscillators;

        unsigned int synth_h;
    };
#endif
