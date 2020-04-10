#ifndef _FAS_H_
#define _FAS_H_

    // standard libraries
    #include <getopt.h>
    #include <assert.h>
    #include <errno.h>
    #include <string.h>
    #include <strings.h>
    #include <stdatomic.h>
    #include <time.h>
    #include <stdbool.h>

    #if defined(_WIN32) || defined(_WIN64)
        #include <conio.h>
    #endif

    #ifdef __unix__
        #include <sys/stat.h>
        #include <signal.h>
    #endif

    // libraries
    #include "portaudio.h"
    #include "libwebsockets.h"

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
#else
    #include "liblfds720.h"
#endif

#ifdef WITH_OSC
    #include "lo/lo.h"
#endif

    #include "lodepng/lodepng.h"

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
    #include "grains.h"
    #include "oscillators.h"
    #include "wavetables.h"
    #include "filters.h"
    #include "note.h"
    #include "usage.h"

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

    // program settings with associated default value
    unsigned int fas_sample_rate = FAS_SAMPLE_RATE;
    int fas_frames_per_buffer = FAS_FRAMES_PER_BUFFER;
    unsigned int fas_deflate = FAS_DEFLATE;
    unsigned int fas_wavetable = FAS_WAVETABLE;
    #ifdef FIXED_WAVETABLE
    unsigned int fas_wavetable_size = 65536;
    unsigned int fas_wavetable_size_m1 = 65535;
    #else
    unsigned int fas_wavetable_size = FAS_WAVETABLE_SIZE;
    unsigned int fas_wavetable_size_m1 = FAS_WAVETABLE_SIZE - 1;
    #endif
    unsigned int fas_noise_wavetable_size = 65536; // noise wavetable size shouldn't change because its lookup wrap is optimized (using a 16-bit index)
    unsigned int fas_audio = FAS_AUDIO;
    unsigned int fas_fps = FAS_FPS;
    unsigned int fas_port = FAS_PORT;
    unsigned int fas_rx_buffer_size = FAS_RX_BUFFER_SIZE;
    unsigned int fas_frames_queue_size = FAS_FRAMES_QUEUE_SIZE;
    unsigned int fas_commands_queue_size = FAS_COMMANDS_QUEUE_SIZE;
    unsigned int fas_ssl = FAS_SSL;
    unsigned int fas_osc_out = FAS_OSC_OUT;
    int fas_input_channels = FAS_INPUT_CHANNELS;
    int fas_output_channels = FAS_OUTPUT_CHANNELS;
    unsigned int fas_granular_max_density = FAS_GRANULAR_MAX_DENSITY;
    unsigned int frame_data_count = FAS_OUTPUT_CHANNELS / 2;
    unsigned int fas_stream_load_send_delay = FAS_STREAM_LOAD_SEND_DELAY;
    unsigned int fas_max_drop = FAS_MAX_DROP;
    unsigned int fas_render_width = FAS_RENDER_WIDTH;
    int fas_samplerate_converter_type = -1; // SRC_SINC_MEDIUM_QUALITY
    double fas_smooth_factor = FAS_SMOOTH_FACTOR;
    float fas_noise_amount = FAS_NOISE_AMOUNT;
    int fas_audio_device = -1;
    int fas_input_audio_device = -1;
    char *fas_iface = NULL;
    char *fas_osc_addr = "127.0.0.1";
    char *fas_osc_port = "57120";
    char *fas_audio_device_name = NULL;
    char *fas_input_audio_device_name = NULL;
    char *fas_render_target = NULL;
    char *fas_render_convert = NULL;
    char *fas_grains_path = NULL;
    char *fas_waves_path = NULL;
    char *fas_impulses_path = NULL;

    unsigned int fas_drop_counter = 0;

    lo_address fas_lo_addr;

    unsigned long int fas_render_counter = 0;
    unsigned long int fas_render_frame_counter = 0;

    unsigned char *fas_render_buffer = NULL;

    float *fas_sine_wavetable = NULL;
    float *fas_white_noise_table = NULL;
    uint16_t noise_index = 0.;

    unsigned int window_size = 8192;
    unsigned int hop_size = 2048;

    double acb_time = 0.;

    double note_time;
    double note_time_samples;
    double lerp_t_step;

    double last_gain_lr = 0.0;

    float *last_sample_l = NULL;
    float *last_sample_r = NULL;

    float *curr_chn_gain = NULL;
    float *last_chn_gain = NULL;
    unsigned int *chn_muted_state = NULL;

    atomic_int audio_thread_state = FAS_AUDIO_PAUSE;

    time_t stream_load_begin;

    PaStream *stream;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;

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

    float **grain_envelope;

    struct _synth_fx **synth_fx = NULL; 

    int clients = 0;

    atomic_int keep_running = 1;

    struct _synth_chn_states *fas_chn_states = NULL;

    // liblfds related
    enum lfds720_misc_flag overwrite_occurred_flag;

    struct lfds720_ringbuffer_n_state rs; // frames related data structure
    struct lfds720_queue_bss_state synth_commands_queue_state;
    struct lfds720_freelist_n_state freelist_commands;
    struct lfds720_freelist_n_state freelist_frames;

    struct _freelist_frames_data *ffd;
    struct _freelist_synth_commands *fsc;
    //

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

    void karplusTrigger(unsigned int chn, struct oscillator *osc, struct note *n) {
        unsigned int d = 0;

        if (n->previous_volume_l <= 0 && n->previous_volume_r <= 0) {
            memset(osc->fp1[chn], 0, sizeof(double) * 4);
            memset(osc->fp2[chn], 0, sizeof(double) * 4);
            memset(osc->fp3[chn], 0, sizeof(double) * 4);
            memset(osc->fp4[chn], 0, sizeof(double) * 4);
        }

        osc->pvalue[chn] = 0.0f;
        osc->fphase[chn] = 0.0f;

        // fill with noise & filter
        for (d = 0; d < osc->buffer_len; d += 1) {
            unsigned int bindex = chn * osc->buffer_len + d;
#ifdef WITH_SOUNDPIPE
            float si = 0.f;
            float so = 0.f;
            sp_noise_compute(sp, (sp_noise *)osc->sp_gens[chn][SP_WHITE_NOISE_GENERATOR], NULL, &si);

            sp_streson *streson = (sp_streson *)osc->sp_filters[chn][SP_STRES_FILTER_L];
            streson->freq = osc->freq * n->cutoff;
            streson->fdbgain = (n->res > 1.f) ? 1.f : n->res;
            sp_streson_compute(sp, streson, &si, &so);

            osc->buffer[bindex] = so;
#else
            osc->buffer[bindex] = fas_white_noise_table[d % fas_noise_wavetable_size];
            osc->buffer[bindex] = huovilainen_moog(osc->buffer[bindex], n->cutoff, n->res, osc->fp1[chn], osc->fp2[chn], osc->fp3[chn], 2);
#endif
        }
    }

    void freeChannelState(struct _synth_chn_states *state) {
        afSTFTfree(state->afSTFT_handle);

        for (int j = 0; j < 2; j += 1) {
            free(state->in[j]);
            free(state->out[j]);

            free(state->stft_result[j].re);
            free(state->stft_result[j].im);

            free(state->stft_temp[j].re);
            free(state->stft_temp[j].im);
        }
    }

    void createChannelState(struct _synth_chn_states *state, unsigned int hop_size) {
        if (hop_size > 1024) {
            return;
        }
        
        if (state->afSTFT_handle) {
            freeChannelState(state);
        }

        afSTFTinit(&state->afSTFT_handle, hop_size, 2, 2, 0, 0);

        for (int j = 0; j < 2; j += 1) {
            state->in[j] = (float *)calloc(hop_size, sizeof(float));
            state->out[j] = (float *)calloc(hop_size, sizeof(float));

            state->stft_result[j].re = (float *)calloc((hop_size + 1), sizeof(float));
            state->stft_result[j].im = (float *)calloc((hop_size + 1), sizeof(float));

            state->stft_temp[j].re = (float *)calloc((hop_size + 1), sizeof(float));
            state->stft_temp[j].im = (float *)calloc((hop_size + 1), sizeof(float));
        }

        state->hop_size = hop_size;
    }

    struct _synth_chn_states *createChannelsState(unsigned int channels) {
        struct _synth_chn_states *chn_states = (struct _synth_chn_states*)calloc(channels, sizeof(struct _synth_chn_states));

        for (unsigned int i = 0; i < channels; i += 1) {
            struct _synth_chn_states *state = &chn_states[i];

            createChannelState(state, FAS_STFT_HOP_SIZE);
        }

        return chn_states;
    }

    void freeChannelsState(struct _synth_chn_states *chn_states, unsigned int channels) {
        if (!chn_states) {
            return;
        }

        for (unsigned int i = 0; i < channels; i += 1) {
            struct _synth_chn_states *state = &chn_states[i];

            freeChannelState(state);
        }

        free(chn_states);
    }

    void freeSynth(struct _synth **s) {
        struct _synth *synth = *s;
        if (synth) {
            if (synth->oscillators && synth->settings) {
                synth->oscillators = freeOscillatorsBank(&synth->oscillators, synth->settings->h, frame_data_count);
            }

            if (synth->grains) {
                freeGrains(&synth->grains, samples_count, frame_data_count, synth->settings->h, fas_granular_max_density);
            }

            if (synth->chn_settings) {
                free(synth->chn_settings);
            }

            if (synth->settings) {
                free(synth->settings);
            }

            if (synth->gain) {
                free(synth->gain);
            }

            free(synth);
        }
    }

    void clearQueues() {
        void *key;
        while (lfds720_ringbuffer_n_read(&rs, &key, NULL) == 1) {
            struct _freelist_frames_data *freelist_frames_data = (struct _freelist_frames_data *)key;
            
            LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(freelist_frames_data->fe, freelist_frames_data);
            lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &freelist_frames_data->fe);
        }

        void *queue_synth_void;
        while(lfds720_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void)) {
            struct _freelist_synth_commands *freelist_synth_command = (struct _freelist_synth_commands *)queue_synth_void;

            LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(freelist_synth_command->fe, freelist_synth_command);
            lfds720_freelist_n_threadsafe_push(&freelist_commands, NULL, &freelist_synth_command->fe);
        }
    }

    // initialize chn settings (no fx, bypass off, void synthesis type)
    void initializeSynthChnSettings() {
        unsigned int i = 0, j = 0;
        
        for (i = 0; i < frame_data_count; i += 1) {   
            for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {  
                curr_synth.chn_settings[i].fx[j].fx_id = -1;
                curr_synth.chn_settings[i].fx[j].bypass = 0;
            }
            curr_synth.chn_settings[i].synthesis_method = FAS_VOID;
            curr_synth.chn_settings[i].muted = 0;
        }
    }

    #define _MAX(a,b) ((a) > (b) ? a : b)
    #define _MIN(a,b) ((a) < (b) ? a : b)

#endif
