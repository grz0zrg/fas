/*
    Copyright (c) 2017, Julien Verneuil
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
    Band-aid raw additive synthesizer built for the Fragment Synthesizer, a web-based Collaborative Spectral Synthesizer.

    This collect Fragment settings and notes data over WebSocket, convert them to a suitable data structure and generate sound from it in real-time for a smooth experience,
    this effectively serve as a band-aid for crackling audio until some heavier optimizations are done.

    Only one client is supported (altough many can connect, not tested but it may result in a big audio mess and likely a crash!)

    The audio callback contain its own synth. data structure,
    this data structure is filled from data coming from a lock-free ring buffer to ensure thread safety for the incoming notes data.

    There is a generic lock-free thread-safe commands queue for synth. parameter changes (gain, oscillators etc.).

    A free list data structure is used to handle data reuse, the program pre-allocate a pool of notes buffer that is reused.

    You can tweak this program by passing settings to its arguments, for help : fas --h

    Can be used as a generic additive synthesizer if you feed it correctly!

    https://www.fsynth.com

    Author : Julien Verneuil
    License : Simplified BSD License

    TODO : apply notes related optimizations which were done for CSP (which is not processing silent/useless notes)
    TODO : data types from JavaScript should NOT be handled as rough as it is right now
    TODO : default values for the program parameters should be elsewhere
    TODO : refactor/rename some data structures
    TODO : smooth volume transition when paused/stopped
    TODO : thread-safe memory deallocation for synth. parameters change
*/

#include <getopt.h>
#include <math.h>
#include <assert.h>

#include "portaudio.h"
#include "libwebsockets.h"
#include "inc/liblfds710.h"

#ifndef M_PI
    #define M_PI (3.141592653589)
#endif

#define PEER_NAME_BUFFER_LENGTH 64
#define PEER_ADDRESS_BUFFER_LENGTH 16

#define PACKET_HEADER_LENGTH 8

// packets id
#define SYNTH_SETTINGS 0
#define FRAME_DATA 1
#define GAIN_CHANGE 2

// program settings with associated default value
unsigned int fas_sample_rate = 44100;
unsigned int fas_frames_per_buffer = 512;
unsigned int fas_deflate = 0;
unsigned int fas_wavetable = 1;
unsigned int fas_wavetable_size = 8192;
unsigned int fas_fps = 60;
unsigned int fas_port = 3003;
unsigned int fas_rx_buffer_size = 4096;
unsigned int fas_realtime = 0;
unsigned int fas_frames_queue_size = 127;
unsigned int fas_commands_queue_size = 16;
unsigned int fas_max_height = 4096;

float *fas_sine_wavetable = NULL;

double note_time;
double note_time_samples;
double lerp_t_step;

// liblfds related
enum lfds710_misc_flag overwrite_occurred_flag;

struct lfds710_ringbuffer_state rs; // frames related data structure
struct lfds710_queue_bss_state synth_commands_queue_state;
struct lfds710_freelist_state freelist_frames;

struct _freelist_frames_data {
    struct lfds710_freelist_element fe;

    double *data;
};
//

struct lws_context *context;

struct _synth_settings {
    unsigned int h;
    unsigned int octave;
    double base_frequency;
};

struct _synth_gain {
    double gain_lr;
};

struct oscillator {
    double freq;
    unsigned int phase_index;
    unsigned int phase_step;
};

struct _synth {
    struct _synth_settings *settings;
    struct _synth_gain *gain;
    struct oscillator *oscillators;

    double lerp_t;
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

    // contain user session related synth. data
    struct _synth *synth;

    unsigned int synth_h;
};

double *curr_notes = NULL;
struct _freelist_frames_data *curr_freelist_frames_data = NULL;
unsigned long frames_read = 0;

// liblfds data structures cleanup callbacks
void flf_element_cleanup_callback(struct lfds710_freelist_state *fs, struct lfds710_freelist_element *fe) {
    struct _freelist_frames_data *freelist_frames_data;
    freelist_frames_data = LFDS710_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

    free(freelist_frames_data->data);
}

void q_element_cleanup_callback(struct lfds710_queue_bss_state *qbsss, void *key, void *value) {
    free(value);
}

void rb_element_cleanup_callback(struct lfds710_ringbuffer_state *rs, void *key, void *value, enum lfds710_misc_flag unread_flag) {
    if (unread_flag == LFDS710_MISC_FLAG_RAISED) {
        free(key);
    }
}
// -

static int paCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *data )
{
    LFDS710_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE;

    float *out = (float*)outputBuffer;

    enum lfds710_misc_flag rb_overwrite_occurred_flag;
    double *overwritten_data = NULL;

    void *queue_synth_void;
    if (lfds710_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void) == 1) {
        struct _synth *queue_synth = (struct _synth *)queue_synth_void;

        if (queue_synth->oscillators) {
            //free(curr_synth.oscillators);

            curr_synth.oscillators = queue_synth->oscillators;

            //lfds710_ringbuffer_cleanup(&rs, rb_element_cleanup_callback);
        }

        if (queue_synth->settings) {
            //free(curr_synth.settings);

            curr_synth.settings = queue_synth->settings;

            //lfds710_ringbuffer_cleanup(&rs, rb_element_cleanup_callback);
        }

        if (queue_synth->gain) {
            //free(curr_synth.gain);

            curr_synth.gain = queue_synth->gain;
        }
/*
        lfds710_ringbuffer_write(&free_data_rb_state, (void *) (lfds710_pal_uint_t) queue_synth, NULL, &rb_overwrite_occurred_flag, (void *)&overwritten_data, NULL);
        if (rb_overwrite_occurred_flag == LFDS710_MISC_FLAG_RAISED) {
            free(overwritten_data);
#ifdef DEBUG
    printf("Freeing rb synth. data!");
#endif
        }
*/
    }

    unsigned int i, j;

    if (curr_synth.oscillators == NULL ||
           curr_synth.settings == NULL ||
               curr_synth.gain == NULL) {
        for (i = 0; i < framesPerBuffer; i += 1) {
            *out++ = 0;
            *out++ = 0;
        }

        curr_synth.lerp_t = 0.0;
        curr_synth.curr_sample = 0;

        lerp_t_step = 1 / note_time_samples;

        return paContinue;
    }

    unsigned long note_buffer_len = curr_synth.settings->h * 5;

    void *key;
    double *_notes;
    struct _freelist_frames_data *freelist_frames_data;

    int read_status = 0;

    for (i = 0; i < framesPerBuffer; i += 1) {
        double output_l = 0.0;
        double output_r = 0.0;

        if (curr_notes != NULL) {
            for (j = 0; j < note_buffer_len; j += 5) {
                unsigned int osc_index = curr_notes[j];
                double previous_volume_l = curr_notes[j + 1];
                double previous_volume_r = curr_notes[j + 2];
                double diff_volume_l = curr_notes[j + 3];
                double diff_volume_r = curr_notes[j + 4];

                struct oscillator *osc = &curr_synth.oscillators[osc_index];

                double s = fas_sine_wavetable[osc->phase_index & (fas_wavetable_size - 1)];

                output_l += (previous_volume_l + diff_volume_l * curr_synth.lerp_t) * s;
                output_r += (previous_volume_r + diff_volume_r * curr_synth.lerp_t) * s;

                osc->phase_index += osc->phase_step;

                if (osc->phase_index >= fas_wavetable_size) {
                    osc->phase_index -= fas_wavetable_size;
                }
            }
        }

        *out++ = output_l * curr_synth.gain->gain_lr;
        *out++ = output_r * curr_synth.gain->gain_lr;

        curr_synth.lerp_t += lerp_t_step;

        curr_synth.curr_sample += 1;

        if (curr_synth.curr_sample >= note_time_samples) {
            lerp_t_step = 0;

            curr_synth.curr_sample = 0;

            read_status = lfds710_ringbuffer_read(&rs, &key, NULL);
            if (read_status == 1) {
                freelist_frames_data = (struct _freelist_frames_data *)key;

                _notes = freelist_frames_data->data;

                if (curr_notes) {
                    LFDS710_FREELIST_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
                    lfds710_freelist_push(&freelist_frames, &curr_freelist_frames_data->fe, NULL);
                }

                curr_notes = _notes;
                curr_freelist_frames_data = freelist_frames_data;

                curr_synth.lerp_t = 0;
                lerp_t_step = 1 / note_time_samples;

#ifdef DEBUG
    frames_read += 1;
    if ((frames_read % 512) == 0) {
        printf("%lu frames read", frames_read);
    }
#endif
            } else {
                curr_notes = NULL;
            }
        }
    }

    return paContinue;
}

struct oscillator *createOscillators(unsigned int n, double base_frequency, unsigned int octaves) {
    struct oscillator *oscillators = (struct oscillator*)malloc(n * sizeof(struct oscillator));

    if (oscillators == NULL) {
        printf("createOscillators alloc. error.");
        fflush(stdout);
        return NULL;
    }

    int y = 0;
    int index = 0;
    double octave_length = n / octaves;
    double frequency;
    unsigned int phase_step;
    int nmo = n - 1;

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2, y / octave_length);
        phase_step = frequency / (double)fas_sample_rate * fas_wavetable_size;

        oscillators[index].freq = frequency;
        oscillators[index].phase_index = rand() / (double)RAND_MAX * fas_wavetable_size;
        oscillators[index].phase_step = phase_step;
    }

    return oscillators;
}

void fillNotesBuffer(double *note_buffer, unsigned int h, size_t data_length, unsigned char *prev_data, unsigned char *data) {
    double pvl = 0, pvr = 0, pr, pg;
    unsigned int i;
    unsigned int r, g;
    unsigned int index = 0;
    double volume_l, volume_r;
    unsigned int y = h - 1;
    double inv_full_brightness = 1 / 255.0;

    for (i = 0; i < data_length; i += 4) {
        pr = prev_data[i];
        pg = prev_data[i + 1];

        r = data[i];
        g = data[i + 1];

        if (r > 0 || g > 0) {
            volume_l = r * inv_full_brightness;
            volume_r = g * inv_full_brightness;

            pvl = pr * inv_full_brightness;
            pvr = pg * inv_full_brightness;

            note_buffer[index] = y;
            note_buffer[index + 1] = pvl;
            note_buffer[index + 2] = pvr;
            note_buffer[index + 3] = volume_l - pvl;
            note_buffer[index + 4] = volume_r - pvr;
        } else {
            if (pr > 0 || pg > 0) {
                pvl = pr * inv_full_brightness;
                pvr = pg * inv_full_brightness;

                note_buffer[index] = y;
                note_buffer[index + 1] = pvl;
                note_buffer[index + 2] = pvr;
                note_buffer[index + 3] = -pvl;
                note_buffer[index + 4] = -pvr;
            } else {
                note_buffer[index] = y;
                note_buffer[index + 1] = 0.0;
                note_buffer[index + 2] = 0.0;
                note_buffer[index + 3] = 0.0;
                note_buffer[index + 4] = 0.0;
            }
        }

        y -= 1;

        index += 5;
    }
}

int  ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
    LFDS710_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE;

    struct user_session_data *usd = (struct user_session_data *)user;
    int n, m, fd;
    unsigned char pid;
    int free_prev_data = 0;
    size_t remaining_payload;
    int is_final_fragment;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            fd = lws_get_socket_fd(wsi);
            lws_get_peer_addresses(wsi, fd, usd->peer_name, PEER_NAME_BUFFER_LENGTH,
                usd->peer_ip, PEER_ADDRESS_BUFFER_LENGTH);

            printf("Connection successfully established from %s (%s).\n",
                usd->peer_name, usd->peer_ip);
            fflush(stdout);

            usd->packet = NULL;
            usd->packet_len = 0;
            usd->packet_skip = 0;
            usd->frame_data = NULL;
            usd->prev_frame_data = NULL;
            usd->synth = NULL;

            usd->synth_h = 0;

            // testing deflate options
            // lws_set_extension_option(wsi, "permessage-deflate", "rx_buf_size", "11");
            break;

        case LWS_CALLBACK_RECEIVE:
            is_final_fragment = lws_is_final_fragment(wsi);

            if (usd->packet_skip) {
                if (is_final_fragment) {
                    usd->packet_skip = 0;
                    usd->packet_len = 0;
                }

                return 0;
            }

            usd->packet_len += len;

            remaining_payload = lws_remaining_packet_payload(wsi);

            if (usd->packet == NULL) {
                // we initialize the first fragment or the final one
                // this mechanism depend on the rx buffer size
                usd->packet = (char *)malloc(len);
                if (usd->packet == NULL) {
                    if (is_final_fragment) {
                        printf("A packet was skipped due to alloc. error.\n");
                    } else {
                        printf("A packet will be skipped due to alloc. error.\n");

                        usd->packet_skip = 1;
                    }
                    fflush(stdout);

                    return 0;
                }

                memcpy(usd->packet, &((char *) in)[0], len);

#ifdef DEBUG
    printf("\nReceiving packet...\n");
#endif
            } else {
                // accumulate the packet fragments to construct the final one
                char *new_packet = (char *)realloc(usd->packet, usd->packet_len);

                if (new_packet == NULL) {
                    free(usd->packet);
                    usd->packet = NULL;

                    usd->packet_skip = 1;

                    printf("A packet will be skipped due to alloc. error.\n");
                    fflush(stdout);

                    return 0;
                }

                usd->packet = new_packet;

                memcpy(&(usd->packet)[usd->packet_len - len], &((char *) in)[0], len);
            }

#ifdef DEBUG
if (remaining_payload != 0) {
    printf("Remaining packet payload: %lu\n", remaining_payload);
}
#endif

            if (is_final_fragment) {
#ifdef DEBUG
    printf("Full packet received, length: %lu\n", usd->packet_len);
#endif
                pid = usd->packet[0];

                if (pid == SYNTH_SETTINGS) {
                    if (usd->synth == NULL) {
                        usd->synth = (struct _synth*)malloc(sizeof(struct _synth));

                        if (usd->synth == NULL) {
                            printf("Skipping packet due to synth data alloc. error.\n");
                            fflush(stdout);

                            goto free_packet;
                        }

                        usd->synth->settings = (struct _synth_settings*)malloc(sizeof(struct _synth_settings));

                        if (usd->synth->settings == NULL) {
                            printf("Skipping packet due to synth settings alloc. error.\n");
                            fflush(stdout);

                            free(usd->synth);
                            usd->synth = NULL;

                            goto free_packet;
                        }
                    }

                    usd->synth->gain = NULL;
                    usd->synth->oscillators = NULL;

                    memcpy(usd->synth->settings, &((char *) usd->packet)[PACKET_HEADER_LENGTH], 16);

#ifdef DEBUG
    printf("SYNTH_SETTINGS : %u, %u, %f\n", usd->synth->settings->h,
        usd->synth->settings->octave, usd->synth->settings->base_frequency);
#endif
                    if (usd->synth->oscillators) {
                        free(usd->synth->oscillators);
                        usd->synth->oscillators = NULL;
                    }

                    usd->expected_frame_length = 4 * sizeof(unsigned char) * usd->synth->settings->h;
                    usd->frame_data = malloc(usd->expected_frame_length);
                    usd->prev_frame_data = calloc(usd->expected_frame_length, sizeof(unsigned char));

                    usd->synth_h = usd->synth->settings->h;

                    if (usd->synth_h >= fas_max_height) {
                        printf("Client disconnected, synth. height reach max. height, please increase max. height.\n");
                        fflush(stdout);

                        free(usd->synth->settings);
                        free(usd->synth);
                        usd->synth = NULL;

                        free(usd->prev_frame_data);
                        free(usd->frame_data);
                        free(usd->packet);

                        return -1;
                    }

                    usd->synth->oscillators = createOscillators(usd->synth->settings->h,
                        usd->synth->settings->base_frequency, usd->synth->settings->octave);

                    if (lfds710_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
                        printf("Skipping packet, the synth commands queue is full.\n");
                        fflush(stdout);

                        free(usd->synth->settings);
                        free(usd->synth->oscillators);
                        free(usd->synth);
                        usd->synth = NULL;

                        goto free_packet;
                    }

                    usd->synth = NULL;
                } else if (pid == FRAME_DATA) {
#ifdef DEBUG
    printf("FRAME_DATA\n");
#endif
                    if ((usd->packet_len - PACKET_HEADER_LENGTH) != usd->expected_frame_length) {
                        printf("Skipping a frame, the frame length does not match the expected frame length.\n");
                        fflush(stdout);
                        goto free_packet;
                    }

                    if (usd->frame_data == NULL) {
                        printf("Skipping a frame (alloc. error) until a synth. settings change happen.\n");
                        fflush(stdout);
                        goto free_packet;
                    }

                    memcpy(usd->prev_frame_data, &usd->frame_data[0], usd->expected_frame_length);

                    memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->expected_frame_length);

                    struct lfds710_freelist_element *fe;
                    struct _freelist_frames_data *freelist_frames_data;
                    int pop_result = lfds710_freelist_pop(&freelist_frames, &fe, NULL);
                    if (pop_result == 0) {
                        printf("Skipping a frame, notes buffer freelist is empty.\n");
                        fflush(stdout);
                        goto free_packet;
                    }

                    freelist_frames_data = LFDS710_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

                    fillNotesBuffer(freelist_frames_data->data, usd->synth_h, usd->expected_frame_length, (unsigned char *)usd->prev_frame_data, (unsigned char *)usd->frame_data);

                    struct _freelist_frames_data *overwritten_notes = NULL;
                    lfds710_ringbuffer_write(&rs, (void *) (lfds710_pal_uint_t) freelist_frames_data, NULL, &overwrite_occurred_flag, (void *)&overwritten_notes, NULL);
                    if (overwrite_occurred_flag == LFDS710_MISC_FLAG_RAISED) {
                        // okay, push it back!
                        LFDS710_FREELIST_SET_VALUE_IN_ELEMENT(overwritten_notes->fe, overwritten_notes);
                        lfds710_freelist_push(&freelist_frames, &overwritten_notes->fe, NULL);
                    }
                } else if (pid == GAIN_CHANGE) {
                    if (usd->synth == NULL) {
                        usd->synth = (struct _synth*)malloc(sizeof(struct _synth));
                        if (usd->synth == NULL) {
                            printf("Skipping packet due to synth data alloc. error.\n");
                            fflush(stdout);

                            goto free_packet;
                        }

                        usd->synth->gain = (struct _synth_gain*)malloc(sizeof(struct _synth_gain));
                        if (usd->synth->gain == NULL) {
                            printf("Skipping packet due to synth gain alloc. error.\n");
                            fflush(stdout);

                            free(usd->synth);
                            usd->synth = NULL;

                            goto free_packet;
                        }
                    }

                    usd->synth->oscillators = NULL;
                    usd->synth->settings = NULL;

                    memcpy(usd->synth->gain, &((char *) usd->packet)[PACKET_HEADER_LENGTH], 8);
#ifdef DEBUG
    printf("GAIN_CHANGE : %f\n", usd->synth->gain->gain_lr);
#endif
                    if (lfds710_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
                        printf("Skipping packet, the synth commands queue is full.\n");
                        fflush(stdout);

                        free(usd->synth->gain);
                        free(usd->synth);
                        usd->synth = NULL;

                        goto free_packet;
                    }

                    usd->synth = NULL;
                }

free_packet:
                free(usd->packet);
                usd->packet = NULL;

                usd->packet_len = 0;
            }
#ifdef DEBUG
    fflush(stdout);
#endif
            break;

        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            free(usd->prev_frame_data);
            free(usd->frame_data);
            free(usd->packet);

            printf("Connection from %s (%s) close.\n", usd->peer_name, usd->peer_ip);
            fflush(stdout);
            break;

        default:
            break;
    }

    return 0;
}

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover; client_max_window_bits"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL }
};

static struct lws_protocols protocols[] = {
	{
		"fas-protocol",
		ws_callback,
		sizeof(struct user_session_data),
		4096,
	},
	{ NULL, NULL, 0, 0 }
};

int start_server(void) {
    protocols[0].rx_buffer_size = fas_rx_buffer_size;

    struct lws_context_creation_info context_info = {
        .port = 3003, .iface = NULL, .protocols = protocols, .extensions = NULL,
        .ssl_cert_filepath = NULL, .ssl_private_key_filepath = NULL, .ssl_ca_filepath = NULL,
        .gid = -1, .uid = -1, .options = 0, NULL, .ka_time = 0, .ka_probes = 0, .ka_interval = 0
    };

    context_info.port = fas_port;

    if (fas_deflate) {
        context_info.extensions = exts;
    }

    context = lws_create_context(&context_info);

    if (context == NULL) {
        fprintf(stderr, "lws_create_context failed.\n");
        return -1;
    }

    printf("Fragment Synthesizer successfully started and listening on port %u.\n", fas_port);

    return 0;
}

void print_usage() {
    printf("Usage: fas [list_of_settings]\n");
    printf(" possible settings with associated default value:\n");
    printf("  --sample_rate 44100\n");
    printf("  --frames 512\n");
    printf("  --wavetable 1\n");
    printf("  --wavetable_size 8192\n");
    printf("  --fps 60\n");
    printf("  --deflate 0\n");
    printf("  --rx_buffer_size 4096\n");
    printf("  --port 3003\n");
    printf("  --alsa_realtime_scheduling 0\n");
    printf("  --frames_queue_size 127\n");
    printf("  --commands_queue_size 16\n");
}

int fas_wavetable_init() {
    int i = 0;

    fas_sine_wavetable = (float*)calloc(fas_wavetable_size, sizeof(float));

    if (fas_sine_wavetable == NULL) {
        return -1;
    }

    for (i = 0; i < fas_wavetable_size; i += 1) {
        fas_sine_wavetable[i] = (float)sin(((double)i/(double)fas_wavetable_size) * M_PI * 2.0);
    }

    return 0;
}

int main(int argc, char **argv)
{
    static struct option long_options[] = {
        { "sample_rate",              required_argument, 0, 0 },
        { "frames",                   required_argument, 0, 1 },
        { "wavetable",                required_argument, 0, 2 },
        { "wavetable_size",           required_argument, 0, 3 },
        { "fps",                      required_argument, 0, 4 },
        { "deflate",                  required_argument, 0, 5 },
        { "rx_buffer_size",           required_argument, 0, 6 },
        { "port",                     required_argument, 0, 7 },
        { "alsa_realtime_scheduling", required_argument, 0, 8 },
        { "frames_queue_size",        required_argument, 0, 9 },
        { "commands_queue_size",      required_argument, 0, 10 },
        { "fas_max_height",           required_argument, 0, 10 },
        { 0, 0, 0, 0 }
    };

    int opt = 0;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "",
        long_options, &long_index)) != -1) {
        switch (opt) {
            case 0 :
                fas_sample_rate = strtoul(optarg, NULL, 0);
                break;
            case 1 :
                fas_frames_per_buffer = strtoul(optarg, NULL, 0);
                break;
            case 2 :
                fas_wavetable = strtoul(optarg, NULL, 0);
                break;
            case 3 :
                fas_wavetable_size = strtoul(optarg, NULL, 0);
                break;
            case 4 :
                fas_fps = strtoul(optarg, NULL, 0);
                break;
            case 5 :
                fas_deflate = strtoul(optarg, NULL, 0);
                break;
            case 6 :
                fas_rx_buffer_size = strtoul(optarg, NULL, 0);
                break;
            case 7 :
                fas_port = strtoul(optarg, NULL, 0);
                break;
            case 8 :
                fas_realtime = strtoul(optarg, NULL, 0);
                break;
            case 9:
                fas_frames_queue_size = strtoul(optarg, NULL, 0);
                break;
            case 10:
                fas_commands_queue_size = strtoul(optarg, NULL, 0);
                break;
            case 11:
                fas_max_height = strtoul(optarg, NULL, 0);
                break;
            default: print_usage();
                 return EXIT_FAILURE;
        }
    }

    if (fas_sample_rate == 0) {
        printf("Warning: sample_rate program option argument is invalid, should be > 0, the default value (44100) will be used.\n");

        fas_sample_rate = 44100;
    }

    if (fas_wavetable_size == 0) {
        printf("Warning: wavetable_size program option argument is invalid, should be > 0, the default value (8192) will be used.\n");

        fas_wavetable_size = 8192;
    }

    if (fas_fps == 0) {
        printf("Warning: fps program option argument is invalid, should be > 0, the default value (60) will be used.\n");

        fas_fps = 60;
    }

    if (fas_port == 0) {
        printf("Warning: port program option argument is invalid, should be > 0, the default value (3003) will be used.\n");

        fas_port = 3003;
    }

    if (fas_frames_per_buffer == 0) {
        printf("Warning: frames program option argument is invalid, should be > 0, the default value (512) will be used.\n");

        fas_frames_per_buffer = 512;
    }

    if (fas_rx_buffer_size == 0) {
        printf("Warning: rx_buffer_size program option argument is invalid, should be > 0, the default value (4096) will be used.\n");

        fas_rx_buffer_size = 4096;
    }

    if (fas_frames_queue_size == 0) {
        printf("Warning: frames_queue_size program option argument is invalid, should be > 0, the default value (127) will be used.\n");

        fas_frames_queue_size = 127;
    }

    if (fas_commands_queue_size == 0) {
        printf("Warning: commands_queue_size program option argument is invalid, should be > 0, the default value (16) will be used.\n");

        fas_commands_queue_size = 16;
    }

    if (fas_max_height < 512) {
        printf("Warning: fas_max_height program option argument is invalid, should be >= 512, the default value (4096) will be used.\n");

        fas_max_height = 4096;
    }

    if (errno == ERANGE) {
        printf("Warning: One of the specified program option is out of range and was set to its maximal value.\n");
    }

    // fas setup
    note_time = 1 / (double)fas_fps;
    note_time_samples = round(note_time * fas_sample_rate);
    lerp_t_step = 1 / note_time_samples;

    if (fas_wavetable) {
        if (fas_wavetable_init() < 0) {
            fprintf(stderr, "fas_wavetable_init() failed.\n");

            return EXIT_FAILURE;
        }
    }

    // PortAudio related
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto error;
    }

    curr_synth.settings = NULL;
    curr_synth.gain = NULL;
    curr_synth.oscillators = NULL;
    curr_synth.lerp_t = 0.0;
    curr_synth.curr_sample = 0;

    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    if (fas_realtime) {
        PaAlsa_EnableRealtimeScheduling(&stream, fas_realtime);
    }

    err = Pa_OpenStream(
              &stream,
              NULL,
              &outputParameters,
              fas_sample_rate,
              fas_frames_per_buffer,
              paNoFlag,
              paCallback,
              NULL );
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;

    if (start_server() < 0) {
        goto ws_error;
    }

    struct lfds710_ringbuffer_element *re =
        aligned_alloc(fas_frames_queue_size + 1, sizeof(struct lfds710_ringbuffer_element) * (fas_frames_queue_size + 1));

    struct lfds710_queue_bss_element *synth_commands_queue_element =
        aligned_alloc(fas_commands_queue_size, sizeof(struct lfds710_queue_bss_element) * fas_commands_queue_size);

    if (re == NULL) {
        fprintf(stderr, "lfds rb data structures alloc./align. error.\n");
        goto quit;
    }

    if (synth_commands_queue_element == NULL) {
        fprintf(stderr, "lfds queue data structures alloc./align. error.\n");
        free(re);
        goto quit;
    }

    lfds710_ringbuffer_init_valid_on_current_logical_core(&rs, re, (fas_frames_queue_size + 1), NULL);
    lfds710_queue_bss_init_valid_on_current_logical_core(&synth_commands_queue_state, synth_commands_queue_element, fas_commands_queue_size, NULL);
    lfds710_freelist_init_valid_on_current_logical_core(&freelist_frames, NULL, 0, NULL);

    int i = 0;
    struct _freelist_frames_data *ffd = malloc(sizeof(struct _freelist_frames_data) * (fas_frames_queue_size * 2));
    if (ffd == NULL) {
        fprintf(stderr, "_freelist_frames_data data structure alloc. error.\n");
        free(re);
        free(synth_commands_queue_element);
        goto quit;
    }

    for (i = 0; i < (fas_frames_queue_size * 2); i += 1)
    {
        ffd[i].data = malloc(sizeof(double) * (fas_max_height * 5));

        LFDS710_FREELIST_SET_VALUE_IN_ELEMENT(ffd[i].fe, &ffd[i]);
        lfds710_freelist_push(&freelist_frames, &ffd[i].fe, NULL);
    }

    // websocket stuff
    struct timeval tv = { 0, 0 };
    fd_set fdset;
    int select_result;
    int fd = fileno(stdin);
    do {
        lws_service(context, 1);

        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
    } while ((select_result = select(fd + 1, &fdset, NULL, NULL, &tv)) == 0);

quit:

    // thank you for your attention, bye.
    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    lws_context_destroy(context);

    Pa_Terminate();

    free(fas_sine_wavetable);

    if (re) {
        lfds710_ringbuffer_cleanup(&rs, rb_element_cleanup_callback);
        free(re);
    }

    lfds710_freelist_cleanup(&freelist_frames, flf_element_cleanup_callback);

    if (ffd) {
        free(ffd);
    }

    if (synth_commands_queue_element) {
        lfds710_queue_bss_cleanup(&synth_commands_queue_state, q_element_cleanup_callback);
        free(synth_commands_queue_element);
    }

    return err;

ws_error:
    fprintf(stderr, "lws related error occured.\n");

    lws_context_destroy(context);

error:
    Pa_Terminate();

    if (err != paNoError) {
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    }

    free(fas_sine_wavetable);

    return err;
}
