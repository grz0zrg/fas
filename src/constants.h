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
    #define BANK_SETTINGS 0
    #define FRAME_DATA 1
    #define SYNTH_SETTINGS 2
    #define CHN_SETTINGS 3
    #define CHN_FX_SETTINGS 4
    #define ACTION 5
    #define INSTRUMENT_SETTINGS 6

    // actions
    #define FAS_ACTION_SAMPLES_RELOAD 0
    #define FAS_ACTION_NOTE_RESET 1
    #define FAS_ACTION_FAUST_GENS 2
    #define FAS_ACTION_FAUST_EFFS 3
    #define FAS_ACTION_PAUSE 4
    #define FAS_ACTION_RESUME 5
    #define FAS_ACTION_WAVES_RELOAD 6
    #define FAS_ACTION_IMPULSES_RELOAD 7

    #define FAS_STFT_HOP_SIZE 1024

    #define FAS_EASING_COUNT 32

    // program settings constants
    #define FAS_SAMPLE_RATE 44100
    #define FAS_FRAMES_PER_BUFFER 0
    #define FAS_DEFLATE 0
    #define FAS_WAVETABLE 1
    #define FAS_WAVETABLE_SIZE 8192
    #define FAS_DEFAULT_FPS 60
    #define FAS_DEFAULT_GAIN 0.05
    #define FAS_PORT 3003
    #define FAS_RX_BUFFER_SIZE 8192
    #define FAS_FRAMES_QUEUE_SIZE 3
    #define FAS_COMMANDS_QUEUE_SIZE 512
    #define FAS_OUTPUT_CHANNELS 2
    #define FAS_INPUT_CHANNELS 0
    #define FAS_SSL 0
    #define FAS_NOISE_AMOUNT 0.1
    #define FAS_ENVS_SIZE 65536 // do not change (due to optimizations; when FIXED_WAVETABLE is enabled wavetables will have a length of 16 bits)
    #define FAS_OSC_OUT 0
    #define FAS_AUDIO 1
    #define FAS_SMOOTH_FACTOR 1.0
    #define FAS_GRANULAR_MAX_DENSITY 32
    #define FAS_STREAM_LOAD_SEND_DELAY 2
    #define FAS_MAX_DROP 60 // 1 second
    #define FAS_RENDER_WIDTH 4096

    // limit max. frequency for filters & some soundpipe effects (eq etc.), this is in percent of Nyquist frequency
    #define FAS_FREQ_LIMIT_FACTOR 0.75 // ~36.0kHz for 96kHz sampling rate

    #define FAS_MAX_FX_SLOTS 24
    #define FAS_MAX_FX_PARAMETERS 14

    //#define FAS_USE_CUBIC_INTERP

    // synth commands
    #define FAS_CMD_SYNTH_SETTINGS 0
    #define FAS_CMD_CHN_SETTINGS 1
    #define FAS_CMD_NOTE_RESET 2
    #define FAS_CMD_CHN_FX_SETTINGS 3
    #define FAS_CMD_INSTRUMENT_SETTINGS 4

    // audio thread states
    #define FAS_AUDIO_PLAY 0
    #define FAS_AUDIO_PAUSE 1

    // audio thread commands
    #define FAS_AUDIO_DO_PAUSE 10
    #define FAS_AUDIO_DO_PLAY 11
    #define FAS_AUDIO_DO_FLUSH_THEN_PAUSE 12

    // synthesis method
    #define FAS_ADDITIVE 0
    #define FAS_SPECTRAL 1
    #define FAS_GRANULAR 2
    #define FAS_FM 3
    #define FAS_SUBTRACTIVE 4
    #define FAS_PHYSICAL_MODELLING 5
    #define FAS_WAVETABLE_SYNTH 6
    #define FAS_BANDPASS 7
    #define FAS_FORMANT_SYNTH 8
    #define FAS_PHASE_DISTORSION 9
    #define FAS_STRING_RESON 10
    #define FAS_MODAL_SYNTH 11
    #define FAS_MODULATION 12
    #define FAS_INPUT 13
    #define FAS_FAUST 14
    #define FAS_VOID 15

    // granular constants
    #define GRAIN_MIN_DURATION 0.0000001

    // Soundpipe
    #define SP_OSC_FILTERS 12 // MUST BE UPDATED WHEN LIST BELOW CHANGE!!!!
    #define SP_OSC_GENS 7 // MUST BE UPDATED WHEN LIST BELOW CHANGE!!!!
    #define SP_OSC_MODS 7 // MUST BE UPDATED WHEN LIST BELOW CHANGE!!!! (note : empty / pdhalf does count even if not allocated at the same place)

    // sp filters id
    #define SP_MOOG_FILTER 0
    #define SP_DIODE_FILTER 1
    #define SP_KORG35_FILTER 2
    #define SP_LPF18_FILTER 3
    #define SP_FORMANT_FILTER_L 4
    #define SP_FORMANT_FILTER_R 5
    #define SP_MODE_FILTER_L 6
    #define SP_MODE_FILTER_R 7
    #define SP_BANDPASS_FILTER_L 8
    #define SP_BANDPASS_FILTER_R 9
    #define SP_STRES_FILTER_L 10
    #define SP_STRES_FILTER_R 11

    // sp generators id
    #define SP_WHITE_NOISE_GENERATOR 0
    #define SP_PINK_NOISE_GENERATOR 1
    #define SP_BROWN_NOISE_GENERATOR 2
    #define SP_SQUARE_GENERATOR 3
    #define SP_DRIP_GENERATOR 4
    #define SP_PD_GENERATOR 5
    #define SP_BAR_GENERATOR 6

    // sp mods id
    #define SP_EMPTY_MODS 0
    #define SP_CRUSH_MODS 1
    #define SP_PD_MODS 2
    #define SP_WAVSH_MODS 3
    #define SP_FOLD_MODS 4
    #define SP_CONV_MODS 5
    #define NOISE_MODS 6

    // chn params type
    #define CHN_PARAM_SYNTH 0
    #define CHN_PARAM_P0 1
    #define CHN_PARAM_P1 2
    #define CHN_PARAM_P2 3
    #define CHN_PARAM_P3 4

    // sp effects id
    #define FX_CONV 0
    #define FX_ZITAREV 1
    #define FX_SCREV 2
    #define FX_AUTOWAH 3
    #define FX_PHASER 4
    #define FX_COMB 5
    #define FX_DELAY 6
    #define FX_SMOOTH_DELAY 7
    #define FX_BITCRUSH 8
    #define FX_DISTORSION 9
    #define FX_SATURATOR 10
    #define FX_COMPRESSOR 11
    #define FX_PEAK_LIMITER 12
    #define FX_CLIP 13
    #define FX_B_LOWPASS 14
    #define FX_B_HIGHPASS 15
    #define FX_B_BANDPASS 16
    #define FX_B_BANDREJECT 17
    #define FX_PAREQ 18
    #define FX_MOOG_LPF 19
    #define FX_DIODE_LPF 20
    #define FX_KORG_LPF 21
    #define FX_18_LPF 22
    #define FX_TBVCF 23
    #define FX_FOLD 24
    #define FX_DC_BLOCK 25
    #define FX_LPC 26
    #define FX_WAVESET 27
    #define FX_PANNER 28
    #define FX_FAUST 29

    // max instruments
    #define FAS_MAX_INSTRUMENTS 24

#ifdef USE_DOUBLE
    #define FAS_FLOAT double
#else
    #define FAS_FLOAT float
#endif

#endif
