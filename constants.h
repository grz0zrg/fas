#ifndef _FAS_CONSTANTS_H_
#define _FAS_CONSTANTS_H_

    #define linearInterpolate(y1, y2, mu) (y1 * (1-mu) + y2 * mu)

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
    #define CHN_SETTINGS 3
    #define ACTION 4

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
    #define FAS_ENVS_SIZE 65536 // do not change (due to optimizations)
    #define FAS_OSC_OUT 0
    #define FAS_AUDIO 1
    #define FAS_SMOOTH_FACTOR 8.0
    #define FAS_GRANULAR_MAX_DENSITY 128
    #define FAS_STREAM_LOAD_SEND_DELAY 2
    #define FAS_MAX_DROP 32

    // audio thread state&commands
    #define FAS_AUDIO_PLAY 0
    #define FAS_AUDIO_PAUSE 1

    // audio thread commands
    #define FAS_AUDIO_DO_PAUSE 10
    #define FAS_AUDIO_DO_PLAY 11
    #define FAS_AUDIO_DO_WAIT_SETTINGS 12

    // synthesis method
    #define FAS_ADDITIVE 0
    #define FAS_SPECTRAL 1
    #define FAS_GRANULAR 2
    #define FAS_FM 3 // experiments (may not work)
    #define FAS_SUBTRACTIVE 4
    #define FAS_VOID 5

    // granular constants
    #define GRAIN_MIN_DURATION 0.0001

#endif
