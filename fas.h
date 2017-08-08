#ifndef _FAS_H_
#define _FAS_H_

    // standard libraries
    #include <getopt.h>
    #include <assert.h>
    #include <errno.h>
    #include <string.h>
    #include <strings.h>

    #if defined(_WIN32) || defined(_WIN64)
        #include <conio.h>
    #endif

    // libraries
    #include "portaudio.h"
    #include "libwebsockets.h"
    #include "inc/liblfds711.h"

    // fas
    #include "tools.h"
    #include "types.h"
    #include "grains.h"
    #include "oscillators.h"
    #include "wavetables.h"
    #include "note.h"
    #include "usage.h"

    struct _freelist_frames_data {
        struct lfds711_freelist_element fe;

        struct note *data;
    };

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
    unsigned int fas_fps = FAS_FPS;
    unsigned int fas_port = FAS_PORT;
    unsigned int fas_rx_buffer_size = FAS_RX_BUFFER_SIZE;
    unsigned int fas_realtime = FAS_REALTIME;
    unsigned int fas_frames_queue_size = FAS_FRAMES_QUEUE_SIZE;
    unsigned int fas_commands_queue_size = FAS_COMMANDS_QUEUE_SIZE;
    unsigned int fas_ssl = FAS_SSL;
    unsigned int fas_output_channels = FAS_OUTPUT_CHANNELS;
    unsigned int frame_data_count = FAS_OUTPUT_CHANNELS / 2;
    float fas_noise_amount = FAS_NOISE_AMOUNT;
    int fas_audio_device = -1;
    char *fas_iface = NULL;

    float *fas_sine_wavetable = NULL;
    float *fas_white_noise_table = NULL;
    uint16_t noise_index = 0.;

    double note_time;
    double note_time_samples;
    double lerp_t_step;

    PaStream *stream;

    struct lws_context *context;

    struct sample *samples = NULL;
    unsigned int samples_count = 0;

    float **grain_envelope;


    // liblfds related
    enum lfds711_misc_flag overwrite_occurred_flag;

    struct lfds711_ringbuffer_state rs; // frames related data structure
    struct lfds711_queue_bss_state synth_commands_queue_state;
    struct lfds711_freelist_state freelist_frames;

    struct _freelist_frames_data *ffd;
    //

    struct note *curr_notes = NULL;
    struct _freelist_frames_data *curr_freelist_frames_data = NULL;
    unsigned long frames_read = 0;

    void q_element_cleanup_callback(struct lfds711_queue_bss_state *qbsss, void *key, void *value) {

    }

    void rb_element_cleanup_callback(struct lfds711_ringbuffer_state *rs, void *key, void *value, enum lfds711_misc_flag unread_flag) {
        if (unread_flag == LFDS711_MISC_FLAG_RAISED) {

        }
    }

#endif
