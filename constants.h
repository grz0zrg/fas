#ifndef _FAS_CONSTANTS_H_
#define _FAS_CONSTANTS_H_

    #define linearInterpolate(y1, y2, mu) (y1 * (1-mu) + y2 * mu)

    #ifndef M_PI
        #define M_PI (3.141592653589)
    #endif

    #ifndef M_PI2
        #define M_PI2 (3.141592653589 * 2.0)
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
    #define FAS_INPUT_CHANNELS 0
    #define FAS_SSL 0
    #define FAS_NOISE_AMOUNT 0.1
    #define FAS_ENVS_SIZE 65536 // do not change (due to optimizations)
    #define FAS_OSC_OUT 0
    #define FAS_AUDIO 1
    #define FAS_SMOOTH_FACTOR 8.0
    #define FAS_GRANULAR_MAX_DENSITY 128
    #define FAS_STREAM_LOAD_SEND_DELAY 2
    #define FAS_MAX_DROP 60 // 1 second
    #define FAS_RENDER_WIDTH 4096

    #define FAS_MAX_FX_SLOTS 24
    #define FAS_MAX_FX_PARAMETERS 10

    //#define FAS_USE_CUBIC_INTERP
    #define POLYBLEP

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
    #define FAS_FM 3
    #define FAS_SUBTRACTIVE 4
    #define FAS_PHYSICAL_MODELLING 5
    #define FAS_WAVETABLE_SYNTH 6
    #define FAS_FORMANT_SYNTH 7
    #define FAS_PHASE_DISTORSION 8
    #define FAS_MODAL_SYNTH 9
    #define FAS_INPUT 10
    #define FAS_VOID 11

    // granular constants
    #define GRAIN_MIN_DURATION 0.0001

    // Soundpipe
    #define SP_OSC_FILTERS 9 // MUST BE UPDATED WHEN LIST BELOW CHANGE!!!!
    #define SP_OSC_GENS 6 // MUST BE UPDATED WHEN LIST BELOW CHANGE!!!!
    #define SP_OSC_MODS 8 // MUST BE UPDATED WHEN LIST BELOW CHANGE!!!! (note : empty / pdhalf does count even if not allocated at the same place)

    // sp filters id
    #define SP_MOOG_FILTER 0
    #define SP_DIODE_FILTER 1
    #define SP_KORG35_FILTER 2
    #define SP_LPF18_FILTER 3
    #define SP_FORMANT_FILTER_L 4
    #define SP_FORMANT_FILTER_R 5
    #define SP_MODE_FILTER_L 6
    #define SP_MODE_FILTER_R 7
    #define SP_STRES_FILTER 8

    // sp generators id
    #define SP_WHITE_NOISE_GENERATOR 0
    #define SP_PINK_NOISE_GENERATOR 1
    #define SP_BROWN_NOISE_GENERATOR 2
    #define SP_SQUARE_GENERATOR 3
    #define SP_DRIP_GENERATOR 4
    #define SP_PD_GENERATOR 5

    // sp mods id
    #define SP_EMPTY_MODS 0
    #define SP_COMB_MODS 1
    #define SP_CRUSH_MODS 2
    #define SP_WAH_MODS 3
    #define SP_PD_MODS 4
    #define SP_WAVSH_MODS 5
    #define SP_FOLD_MODS 6
    #define SP_CONV_MODS 7

    // chn params type
    #define CHN_PARAM_SYNTH 0
    #define CHN_PARAM_P0 1
    #define CHN_PARAM_P1 2
    #define CHN_PARAM_P2 3
    #define CHN_PARAM_P3 4

    // sp effects id
    #define FX_CONV 0
    #define FX_ZITAREV 1
    #define FX_JCREV 2
    #define FX_SCREV 3
    #define FX_AUTOWAH 4
    #define FX_PHASER 5
    #define FX_COMB 6
    #define FX_VDELAY 7
    #define FX_SMOOTH_DELAY 8
    #define FX_BITCRUSH 9
    #define FX_DISTORSION 10
    #define FX_SATURATOR 11
    #define FX_COMPRESSOR 12
    #define FX_PEAK_LIMITER 13
    #define FX_CLIP 14
    #define FX_ALLPASS 15
    #define FX_B_LOWPASS 16
    #define FX_B_HIGHPASS 17
    #define FX_B_BANDPASS 18
    #define FX_B_BANDREJECT 19
    #define FX_PAREQ 20

#endif
