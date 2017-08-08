#ifndef _FAS_CONSTANTS_H_
#define _FAS_CONSTANTS_H_

    #ifndef M_PI
        #define M_PI (3.141592653589)
    #endif

    #define E 2.718281828459045

    #define PEER_NAME_BUFFER_LENGTH 64
    #define PEER_ADDRESS_BUFFER_LENGTH 16

    #define PACKET_HEADER_LENGTH 8
    #define FRAME_HEADER_LENGTH 8

    // packets id
    #define SYNTH_SETTINGS 0
    #define FRAME_DATA 1
    #define GAIN_CHANGE 2

    // program settings constants
    #define FAS_SAMPLE_RATE 44100
    #define FAS_FRAMES_PER_BUFFER 512
    #define FAS_DEFLATE 0
    #define FAS_WAVETABLE 1
    #define FAS_WAVETABLE_SIZE 8192
    #define FAS_FPS 60
    #define FAS_PORT 3003
    #define FAS_RX_BUFFER_SIZE 8192
    #define FAS_REALTIME 0
    #define FAS_FRAMES_QUEUE_SIZE 7
    #define FAS_COMMANDS_QUEUE_SIZE 16
    #define FAS_OUTPUT_CHANNELS 2
    #define FAS_SSL 0
    #define FAS_NOISE_AMOUNT 0.1
    #define FAS_ENVS_SIZE 8192

#endif
