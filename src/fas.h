#ifndef _FAS_H_
#define _FAS_H_

    // standard libraries
    #include <getopt.h>
    #include <assert.h>
    #include <errno.h>
    #include <string.h>
    #include <strings.h>
    #include <stdatomic.h>
    #include <stdbool.h>
    #include <time.h>

    #if defined(_WIN32) || defined(_WIN64)
        #include <conio.h>
    #endif

    #ifdef __unix__
        #include <sys/stat.h>
        #include <signal.h>
    #endif

    // libraries
#ifdef WITH_JACK
    #include <jack/jack.h>
#else
    #include "portaudio.h"
    #include <pthread.h>
#endif
    #include "libwebsockets.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

// compatibility layer to support liblfds 711 version (since default is liblfds720 which has ARM64 support but is not yet released)
#ifdef LFDS711
    #include "liblfds711.h"

    #define lfds720_queue_bss_dequeue lfds711_queue_bss_dequeue
    #define lfds720_queue_bss_enqueue lfds711_queue_bss_enqueue
    #define lfds720_queue_bss_state lfds711_queue_bss_state
    #define lfds720_queue_bss_element lfds711_queue_bss_element
    #define lfds720_queue_bss_cleanup lfds711_queue_bss_cleanup
    #define lfds720_queue_bss_init_valid_on_current_logical_core lfds711_queue_bss_init_valid_on_current_logical_core
    #define lfds720_freelist_n_element lfds711_freelist_element
    #define lfds720_ringbuffer_n_element lfds711_ringbuffer_element
    #define lfds720_ringbuffer_n_state lfds711_ringbuffer_state
    #define lfds720_freelist_n_state lfds711_freelist_state
    #define LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES LFDS711_PAL_ATOMIC_ISOLATION_IN_BYTES
    #define lfds720_ringbuffer_n_init_valid_on_current_logical_core lfds711_ringbuffer_init_valid_on_current_logical_core
    #define lfds720_freelist_n_init_valid_on_current_logical_core(x, w) lfds711_freelist_init_valid_on_current_logical_core(x, NULL, 0, w)
    #define LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT LFDS711_FREELIST_SET_VALUE_IN_ELEMENT
    #define LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT
    #define lfds720_freelist_n_threadsafe_push(x, y, z) lfds711_freelist_push(x, z, y)
    #define lfds720_freelist_n_threadsafe_pop(x, y, z) lfds711_freelist_pop(x, z, y)
    #define lfds720_ringbuffer_n_cleanup lfds711_ringbuffer_cleanup
    #define lfds720_freelist_n_cleanup lfds711_freelist_cleanup
    #define lfds720_ringbuffer_n_read lfds711_ringbuffer_read
    #define lfds720_ringbuffer_n_write lfds711_ringbuffer_write
    #define lfds720_misc_flag lfds711_misc_flag
    #define LFDS720_MISC_FLAG_RAISED LFDS711_MISC_FLAG_RAISED
    #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE LFDS711_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE
    #define lfds720_pal_uint_t lfds711_pal_uint_t
    #define lfds720_freelist_n_query lfds711_freelist_query
    #define LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT LFDS711_FREELIST_QUERY_SINGLETHREADED_GET_COUNT
#else
    #include "liblfds720.h"
#endif

    #include "lodepng/lodepng.h"
    #include "AHEasing/easing.h"

#ifdef WITH_SOUNDPIPE
      #include "soundpipe.h"

      sp_data *sp = NULL;
#endif

    // fas
#ifdef WITH_FAUST
      #include "faust.h"

      struct _faust_factories *fas_faust_gens = NULL;
      struct _faust_factories *fas_faust_effs = NULL;
#endif
    #include "tools.h"
    #include "effects.h"

    #include "types.h"

    struct _frame_sync {
        uint64_t lasttime;
        double acc_time;
    } frame_sync;

    struct _synth curr_synth;

    #include "grains.h"
    #include "oscillators.h"
    #include "wavetables.h"
    #include "filters.h"
    #include "note.h"
    #include "usage.h"
    #include "timer.h"

    struct _freelist_frames_data {
        struct lfds720_freelist_n_element fe;

        struct note *data;
    };

    struct _freelist_synth_commands {
        struct lfds720_freelist_n_element fe;

        struct _synth_command *data;
    };

    char* fas_default_grains_path = "./grains/";
    char* fas_install_default_grains_path = "/usr/local/share/fragment/grains/";

    char* fas_default_waves_path = "./waves/";
    char* fas_install_default_waves_path = "/usr/local/share/fragment/waves/";

    char *fas_default_impulses_path = "./impulses/";
    char *fas_install_default_impulses_path = "/usr/local/share/fragment/impulses/";

    char *fas_default_faust_gens_path = "./faust/generators";
    char *fas_install_default_faust_gens_path = "/usr/local/share/fragment/faust/generators";

    char *fas_default_faust_effs_path = "./faust/effects";
    char *fas_install_default_faust_effs_path = "/usr/local/share/fragment/faust/effects";

    char *fas_default_faust_libs_path = "./faustlibraries";

    // program settings with associated default value
    unsigned int fas_sample_rate = FAS_SAMPLE_RATE;
    int fas_frames_per_buffer = FAS_FRAMES_PER_BUFFER;
    unsigned int fas_deflate = FAS_DEFLATE;
    unsigned int fas_wavetable = FAS_WAVETABLE;
    unsigned int fas_wavetable_size = FAS_WAVETABLE_SIZE;
    unsigned int fas_wavetable_size_m1 = FAS_WAVETABLE_SIZE - 1;
    unsigned int fas_noise_wavetable_size = 65536; // noise wavetable size shouldn't change because its lookup wrap is optimized (using a 16-bit index)
    unsigned int fas_audio = FAS_AUDIO;
    unsigned int fas_port = FAS_PORT;
    unsigned int fas_rx_buffer_size = FAS_RX_BUFFER_SIZE;
    unsigned int fas_frames_queue_size = FAS_FRAMES_QUEUE_SIZE;
    unsigned int fas_commands_queue_size = FAS_COMMANDS_QUEUE_SIZE;
    unsigned int fas_ssl = FAS_SSL;
    int fas_input_channels = FAS_INPUT_CHANNELS;
    int fas_output_channels = FAS_OUTPUT_CHANNELS;
    unsigned int fas_granular_max_density = FAS_GRANULAR_MAX_DENSITY;
    unsigned int frame_data_count = FAS_OUTPUT_CHANNELS / 2;
    unsigned int fas_stream_infos_send_delay = FAS_STREAM_INFOS_SEND_DELAY;
    unsigned int fas_max_drop = FAS_MAX_DROP;
    unsigned int fas_render_width = FAS_RENDER_WIDTH;
    unsigned int fas_max_instruments = FAS_MAX_INSTRUMENTS;
    unsigned int fas_max_channels = FAS_MAX_CHANNELS;
    int fas_samplerate_converter_type = -1; // SRC_SINC_MEDIUM_QUALITY
    FAS_FLOAT fas_smooth_factor = FAS_SMOOTH_FACTOR;
    FAS_FLOAT fas_noise_amount = FAS_NOISE_AMOUNT;
    int fas_audio_device = -1;
    int fas_input_audio_device = -1;
    char *fas_iface = NULL;
    char *fas_audio_device_name = NULL;
    char *fas_input_audio_device_name = NULL;
    char *fas_render_target = NULL;
    char *fas_render_convert = NULL;
    char *fas_grains_path = NULL;
    char *fas_waves_path = NULL;
    char *fas_impulses_path = NULL;
    char *fas_faust_gens_path = NULL;
    char *fas_faust_effs_path = NULL;
    char *fas_faust_libs_path = NULL;

    unsigned int fas_drop_counter = 0;

    unsigned long int fas_render_counter = 0;
    unsigned long int fas_render_frame_counter = 0;

    unsigned char *fas_render_buffer = NULL;

    FAS_FLOAT *fas_sine_wavetable = NULL;
    FAS_FLOAT *fas_white_noise_table = NULL;
    uint16_t noise_index = 0.;

    unsigned int window_size = 8192;
    unsigned int hop_size = 2048;

    double note_time;
    FAS_FLOAT note_time_samples;
    FAS_FLOAT lerp_t_step;

    // a global note on trigger (gonna trigger re-initialization for all instruments)
    int trigger_note_on = 0;

    FAS_FLOAT last_gain_lr = 0.0;

    atomic_int audio_thread_state = FAS_AUDIO_PAUSE;

    time_t stream_load_begin;

#ifdef WITH_JACK
    jack_port_t **input_ports = NULL;
    jack_port_t **output_ports = NULL;
    jack_client_t *client;
    jack_default_audio_sample_t **jack_in = NULL;
    jack_default_audio_sample_t **jack_out = NULL;

    // from PortAudio
    typedef struct {
        double samplingPeriod;
        double measurementStartTime;
        double averageLoad;
    } CpuLoadMeasurer;

    CpuLoadMeasurer cpu_load_measurer;

    atomic_int cpu_load;
#else
    PaStream *stream;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
#endif

    struct lws_context *context;

    struct sample *samples = NULL;
    unsigned int samples_count = 0;
    unsigned int samples_count_m1 = 0;

    struct sample *waves = NULL;
    unsigned int waves_count = 0;
    unsigned int waves_count_m1 = 0;

    struct sample *impulses = NULL;
    unsigned int impulses_count = 0;
    unsigned int impulses_count_m1 = 0;

    FAS_FLOAT **grain_envelope;

    struct _synth_fx **synth_fx = NULL; 

    int clients = 0;

    atomic_int keep_running = 1;

    struct _synth_instrument_states *fas_instrument_states = NULL;

    // liblfds related
    enum lfds720_misc_flag overwrite_occurred_flag;

    struct lfds720_ringbuffer_n_state rs; // frames related data structure
    struct lfds720_queue_bss_state synth_commands_queue_state;
    struct lfds720_freelist_n_state freelist_commands;
    struct lfds720_freelist_n_state freelist_frames;

    struct _freelist_frames_data *ffd;
    struct _freelist_synth_commands *fsc;
    //

    struct note *dummy_notes = NULL;
    struct note *curr_notes = NULL;
    struct _freelist_frames_data *curr_freelist_frames_data = NULL;
    unsigned long frames_read = 0;

    void q_element_cleanup_callback(struct lfds720_queue_bss_state *qbsss, void *key, void *value) {

    }

    void rb_element_cleanup_callback(struct lfds720_ringbuffer_n_state *rs, void *key, void *value, enum lfds720_misc_flag unread_flag) {
        if (unread_flag == LFDS720_MISC_FLAG_RAISED) {

        }
    }

    // liblfds data structures cleanup callbacks
    void flf_element_cleanup_callback(struct lfds720_freelist_n_state *fs, struct lfds720_freelist_n_element *fe) {
        struct _freelist_frames_data *freelist_frames_data;
        freelist_frames_data = LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT(*fe);

        free(freelist_frames_data->data);
    }

    void flc_element_cleanup_callback(struct lfds720_freelist_n_state *fs, struct lfds720_freelist_n_element *fe) {
        struct _freelist_synth_commands *freelist_synth_command;
        freelist_synth_command = LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT(*fe);

        free(freelist_synth_command->data);
    }

    void karplusTrigger(unsigned int instrument_index, struct oscillator *osc, struct note *n) {
        unsigned int d = 0;

        memset(osc->fp1[instrument_index], 0, sizeof(FAS_FLOAT) * 4);
        memset(osc->fp2[instrument_index], 0, sizeof(FAS_FLOAT) * 4);
        memset(osc->fp3[instrument_index], 0, sizeof(FAS_FLOAT) * 4);
        memset(osc->fp4[instrument_index], 0, sizeof(FAS_FLOAT) * 4);

        osc->pvalue[instrument_index] = 0.0f;
        osc->fphase[instrument_index] = 0.0f;

        // fill with noise & filter
        for (d = 0; d < osc->buffer_len; d += 1) {
            unsigned int bindex = instrument_index * osc->buffer_len + d;
#ifdef WITH_SOUNDPIPE
            FAS_FLOAT si = 0.f;
            FAS_FLOAT so = 0.f;
            sp_noise_compute(sp, (sp_noise *)osc->sp_gens[instrument_index][SP_WHITE_NOISE_GENERATOR], NULL, &si);

            sp_streson *streson = (sp_streson *)osc->sp_filters[instrument_index][SP_STRES_FILTER_L];
            streson->freq = fmin(osc->freq * n->cutoff, fas_sample_rate / 2 * FAS_FREQ_LIMIT_FACTOR);
            streson->fdbgain = (n->res > 1.f) ? 1.f : n->res;
            sp_streson_compute(sp, streson, &si, &so);

            osc->buffer[bindex] = so;
#else
            osc->buffer[bindex] = fas_white_noise_table[d % fas_noise_wavetable_size];
            osc->buffer[bindex] = huovilainen_moog(osc->buffer[bindex], n->cutoff, n->res, osc->fp1[instrument_index], osc->fp2[instrument_index], osc->fp3[instrument_index], 2);
#endif
        }
    }

    void freeInstrumentState(struct _synth_instrument_states *state) {
        afSTFTfree(state->afSTFT_handle);

        state->afSTFT_handle = NULL;

        for (int j = 0; j < 2; j += 1) {
            free(state->in[j]);
            free(state->out[j]);

            free(state->stft_result[j].re);
            free(state->stft_result[j].im);

            free(state->stft_temp[j].re);
            free(state->stft_temp[j].im);
        }

        state->position = 0;
    }

    void createInstrumentState(struct _synth_instrument_states *state, unsigned int hop_size) {
        if (hop_size > 1024) {
            return;
        }
        
        if (state->afSTFT_handle) {
            freeInstrumentState(state);
        }

        state->afSTFT_handle = NULL;

        afSTFTinit(&state->afSTFT_handle, hop_size, 2, 2, 0, 0);

        for (int j = 0; j < 2; j += 1) {
            state->in[j] = (float *)calloc(hop_size, sizeof(float));
            state->out[j] = (float *)calloc(hop_size, sizeof(float));

            state->stft_result[j].re = (float *)calloc((hop_size + 1), sizeof(float));
            state->stft_result[j].im = (float *)calloc((hop_size + 1), sizeof(float));

            state->stft_temp[j].re = (float *)calloc((hop_size + 1), sizeof(float));
            state->stft_temp[j].im = (float *)calloc((hop_size + 1), sizeof(float));
        }

        state->position = 0;

        state->hop_size = hop_size;
    }

    struct _synth_instrument_states *createInstrumentsState(unsigned int instruments) {
        struct _synth_instrument_states *instrument_states = (struct _synth_instrument_states*)calloc(instruments, sizeof(struct _synth_instrument_states));

        for (unsigned int i = 0; i < instruments; i += 1) {
            struct _synth_instrument_states *state = &instrument_states[i];

            createInstrumentState(state, FAS_STFT_HOP_SIZE);
        }

        return instrument_states;
    }

    void freeInstrumentsState(struct _synth_instrument_states *instrument_states, unsigned int instruments) {
        if (!instrument_states) {
            return;
        }

        for (unsigned int i = 0; i < instruments; i += 1) {
            struct _synth_instrument_states *state = &instrument_states[i];

            freeInstrumentState(state);
        }

        free(instrument_states);
    }

    void freeSynth(struct _synth **s) {
        struct _synth *synth = *s;
        if (synth) {
            if (synth->oscillators && synth->bank_settings) {
                synth->oscillators = freeOscillatorsBank(&synth->oscillators, synth->bank_settings->h, fas_max_instruments);
            }

            if (synth->grains) {
                freeGrains(&synth->grains, samples_count, fas_max_instruments, synth->bank_settings->h, fas_granular_max_density);
            }

            if (synth->chn_settings) {
                free(synth->chn_settings);
            }

            if (synth->bank_settings) {
                free(synth->bank_settings);
            }

            if (synth->settings) {
                free(synth->settings);
            }

            if (synth->instruments) {
                free(synth->instruments);
            }

            free(synth);
        }
    }

    void clearNotesQueue() {
        void *key;
        while (lfds720_ringbuffer_n_read(&rs, &key, NULL) == 1) {
            struct _freelist_frames_data *freelist_frames_data = (struct _freelist_frames_data *)key;
            
            LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(freelist_frames_data->fe, freelist_frames_data);
            lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &freelist_frames_data->fe);
        }
    }

    void clearSynthQueue() {
        void *queue_synth_void;
        while(lfds720_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void)) {
            struct _freelist_synth_commands *freelist_synth_command = (struct _freelist_synth_commands *)queue_synth_void;

            LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(freelist_synth_command->fe, freelist_synth_command);
            lfds720_freelist_n_threadsafe_push(&freelist_commands, NULL, &freelist_synth_command->fe);
        }
    }

    void clearQueues() {
        clearNotesQueue();
        clearSynthQueue();
    }

    // initialize chn settings (no fx, bypass off)
    void initializeSynthChnSettings() {
        unsigned int i = 0, j = 0;
        
        for (i = 0; i < fas_max_instruments; i += 1) {   
            for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {  
                curr_synth.chn_settings[i].fx[j].fx_id = -1;
                curr_synth.chn_settings[i].fx[j].bypass = 0;
            }
            curr_synth.chn_settings[i].muted = 0;
            curr_synth.chn_settings[i].output_chn = -1;
        }
    }

    // easing functions
    FAS_FLOAT applyEasing(int type, FAS_FLOAT f) {
        if (type == 0) {
            return LinearInterpolation(f);
        } else if (type == 1) {
            return QuadraticEaseIn(f);
        } else if (type == 2) {
            return QuadraticEaseOut(f);
        } else if (type == 3) {
            return QuadraticEaseInOut(f);
        } else if (type == 4) {
            return CubicEaseIn(f);
        } else if (type == 5) {
            return CubicEaseOut(f);
        } else if (type == 6) {
            return CubicEaseInOut(f);
        } else if (type == 7) {
            return QuarticEaseIn(f);
        } else if (type == 8) {
            return QuarticEaseOut(f);
        } else if (type == 9) {
            return QuarticEaseInOut(f);
        } else if (type == 10) {
            return QuinticEaseIn(f);
        } else if (type == 11) {
            return QuinticEaseOut(f);
        } else if (type == 12) {
            return QuinticEaseInOut(f);
        } else if (type == 13) {
            return SineEaseIn(f);
        } else if (type == 14) {
            return SineEaseOut(f);
        } else if (type == 15) {
            return SineEaseInOut(f);
        } else if (type == 16) {
            return CircularEaseIn(f);
        } else if (type == 17) {
            return CircularEaseOut(f);
        } else if (type == 18) {
            return CircularEaseInOut(f);
        } else if (type == 19) {
            return ExponentialEaseIn(f);
        } else if (type == 20) {
            return ExponentialEaseOut(f);
        } else if (type == 21) {
            return ExponentialEaseInOut(f);
        } else if (type == 22) {
            return ElasticEaseIn(f);
        } else if (type == 23) {
            return ElasticEaseOut(f);
        } else if (type == 24) {
            return ElasticEaseInOut(f);
        } else if (type == 25) {
            return BackEaseIn(f);
        } else if (type == 26) {
            return BackEaseOut(f);
        } else if (type == 27) {
            return BackEaseInOut(f);
        } else if (type == 28) {
            return BounceEaseIn(f);
        } else if (type == 29) {
            return BounceEaseOut(f);
        } else if (type == 30) {
            return BounceEaseInOut(f);
        } else {
            return f;
        }
    }

    void fpsChange(uint32_t fps) {
        note_time = 1.0 / (double)fps;
        note_time_samples = round(note_time * fas_sample_rate);
        lerp_t_step = 1 / note_time_samples;
    }

    #define _MAX(a,b) ((a) > (b) ? a : b)
    #define _MIN(a,b) ((a) < (b) ? a : b)

#endif
