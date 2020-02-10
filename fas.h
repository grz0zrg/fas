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
    #include "inc/liblfds720.h"
    #include "lo/lo.h"
    #include "lib/lodepng.h"
    #include "essentia_wrapper.h"

    CEssentia cessentia;

    #ifdef WITH_SOUNDPIPE
      #include "soundpipe.h"

      sp_data *sp = NULL;
    #endif

    // fas
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

    char* fas_default_grains_path = "./grains/";
    char* fas_install_default_grains_path = "/usr/local/share/fragment/grains/";

    char* fas_default_waves_path = "./waves/";
    char* fas_install_default_waves_path = "/usr/local/share/fragment/waves/";

    char *fas_default_impulses_path = "./impulses/";
    char *fas_install_default_impulses_path = "/usr/local/share/fragment/impulses/";

    // program settings with associated default value
    unsigned int fas_sample_rate = FAS_SAMPLE_RATE;
    unsigned int fas_frames_per_buffer = FAS_FRAMES_PER_BUFFER;
    unsigned int fas_deflate = FAS_DEFLATE;
    unsigned int fas_wavetable = FAS_WAVETABLE;
    #ifdef FIXED_WAVETABLE
    unsigned int fas_wavetable_size = 65536;
    unsigned int fas_wavetable_size_m1 = 65535;
    #else
    unsigned int fas_wavetable_size = FAS_WAVETABLE_SIZE;
    unsigned int fas_wavetable_size_m1 = FAS_WAVETABLE_SIZE - 1;
    #endif
    unsigned int fas_audio = FAS_AUDIO;
    unsigned int fas_fps = FAS_FPS;
    unsigned int fas_port = FAS_PORT;
    unsigned int fas_rx_buffer_size = FAS_RX_BUFFER_SIZE;
    unsigned int fas_realtime = FAS_REALTIME;
    unsigned int fas_frames_queue_size = FAS_FRAMES_QUEUE_SIZE;
    unsigned int fas_commands_queue_size = FAS_COMMANDS_QUEUE_SIZE;
    unsigned int fas_ssl = FAS_SSL;
    unsigned int fas_osc_out = FAS_OSC_OUT;
    unsigned int fas_output_channels = FAS_OUTPUT_CHANNELS;
    unsigned int fas_granular_max_density = FAS_GRANULAR_MAX_DENSITY;
    unsigned int frame_data_count = FAS_OUTPUT_CHANNELS / 2;
    unsigned int fas_stream_load_send_delay = FAS_STREAM_LOAD_SEND_DELAY;
    unsigned int fas_max_drop = FAS_MAX_DROP;
    unsigned int fas_render_width = FAS_RENDER_WIDTH;
    int fas_samplerate_converter_type = -1; // SRC_SINC_MEDIUM_QUALITY
    double fas_smooth_factor = FAS_SMOOTH_FACTOR;
    float fas_noise_amount = FAS_NOISE_AMOUNT;
    int fas_audio_device = -1;
    char *fas_iface = NULL;
    char *fas_osc_addr = "127.0.0.1";
    char *fas_osc_port = "57120";
    char *fas_audio_device_name = NULL;
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

    float *last_sample_l = NULL;
    float *last_sample_r = NULL;

    atomic_int audio_thread_state = FAS_AUDIO_PLAY;

    time_t stream_load_begin;

    PaStream *stream;

    struct lws_context *context;

    struct sample *samples = NULL;
    unsigned int samples_count = 0;
    unsigned int samples_count_m1 = 0;
    unsigned int prev_samples_count = 0;

    struct sample *waves = NULL;
    unsigned int waves_count = 0;
    unsigned int waves_count_m1 = 0;

    struct sample *impulses = NULL;
    unsigned int impulses_count = 0;
    unsigned int impulses_count_m1 = 0;

    float **grain_envelope;

    struct _synth_fx **synth_fx = NULL; 

    int clients = 0;

    int keep_running = 1;


    // liblfds related
    enum lfds720_misc_flag overwrite_occurred_flag;

    struct lfds720_ringbuffer_n_state rs; // frames related data structure
    struct lfds720_queue_bss_state synth_commands_queue_state;
    struct lfds720_freelist_n_state freelist_frames;

    struct _freelist_frames_data *ffd;
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

    #define _MAX(a,b) ((a) > (b) ? a : b)
    #define _MIN(a,b) ((a) < (b) ? a : b)

#endif
