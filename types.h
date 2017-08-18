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

    struct _synth {
        struct _synth_settings *settings;
        struct _synth_gain *gain;
        struct oscillator *oscillators;
        struct grain *grains;

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

        // contain the current and previously processed audio-frame data
        char *frame_data;
        char *prev_frame_data;
        size_t expected_frame_length;
        size_t expected_max_frame_length;

        unsigned int frame_data_size;

        // contain user session related synth. data
        struct _synth *synth;

        struct oscillator *oscillators;

        unsigned int synth_h;
    };
#endif
