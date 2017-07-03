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

    TODO : data coming from the network for notes etc. should NOT be handled like it is right now (it should instead come with IEEE 754 representation or something...)
    TODO : thread-safe memory deallocation for synth. parameters change
*/

#include <getopt.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#endif

#include "portaudio.h"
#include "libwebsockets.h"
#include "inc/liblfds711.h"
#include "inc/sndfile.h"
#include "inc/tinydir.h"

#ifndef M_PI
    #define M_PI (3.141592653589)
#endif

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
#define FAS_RX_BUFFER_SIZE 4096
#define FAS_REALTIME 0
#define FAS_FRAMES_QUEUE_SIZE 7
#define FAS_COMMANDS_QUEUE_SIZE 16
#define FAS_OUTPUT_CHANNELS 2
#define FAS_SSL 0
#define FAS_NOISE_AMOUNT 0.1

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

struct _synth_settings {
    unsigned int h;
    unsigned int octave;
    double base_frequency;
};

struct _synth_gain {
    double gain_lr;
};

// granular synthesis
struct sample {
    float *data;
    uint32_t len;
    unsigned int chn;
};

struct sample *samples = NULL;
unsigned int samples_count = 0;

struct grain {
    unsigned int frame; // current position
    unsigned int frames; // duration
    unsigned int index; // grain position
    unsigned int direction;
    float *env;
};

struct oscillator {
    double freq;
#ifdef FIXED_WAVETABLE
    uint16_t *phase_index;
    uint16_t phase_step;
#else
    unsigned int *phase_index;
    unsigned int phase_step;
#endif
};

struct osc_norm {
    unsigned int divisor;
    unsigned int *indexes;
    size_t indexes_len;
    size_t indexes_curr;
};

struct note {
    unsigned int osc_index;
    float previous_volume_l;
    float previous_volume_r;
    float diff_volume_l;
    float diff_volume_r;
};

struct _synth {
    struct _synth_settings *settings;
    struct _synth_gain *gain;
    struct oscillator *oscillators;
#ifdef GRANULAR
    struct grain *grains;
#endif

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

    // contain user session related synth. data
    struct _synth *synth;

    unsigned int synth_h;
};

// liblfds related
enum lfds711_misc_flag overwrite_occurred_flag;

struct lfds711_ringbuffer_state rs; // frames related data structure
struct lfds711_queue_bss_state synth_commands_queue_state;
struct lfds711_freelist_state freelist_frames;

struct _freelist_frames_data *ffd;

struct _freelist_frames_data {
    struct lfds711_freelist_element fe;

    struct note *data;
};
//

struct note *curr_notes = NULL;
struct _freelist_frames_data *curr_freelist_frames_data = NULL;
unsigned long frames_read = 0;

struct osc_norm *osc_norms = NULL;

// liblfds data structures cleanup callbacks
void flf_element_cleanup_callback(struct lfds711_freelist_state *fs, struct lfds711_freelist_element *fe) {
    struct _freelist_frames_data *freelist_frames_data;
    freelist_frames_data = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

    free(freelist_frames_data->data);
}

// uneeded
void q_element_cleanup_callback(struct lfds711_queue_bss_state *qbsss, void *key, void *value) {

}

void rb_element_cleanup_callback(struct lfds711_ringbuffer_state *rs, void *key, void *value, enum lfds711_misc_flag unread_flag) {
    if (unread_flag == LFDS711_MISC_FLAG_RAISED) {

    }
}
// -

float lin(float y1, float y2, float mu) {
   return (y1 * (1 - mu) + y2 * mu);
}

float randf(float min, float max) {
    if (min == 0.f && max == 0.f) {
        return 0.f;
    }

    float range = (max - min);
    float div = RAND_MAX / range;
    return min + (rand() / div);
}

static int paCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *data )
{
    LFDS711_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE;


    static float last_sample_r = 0;
    static float last_sample_l = 0;

    float *out = (float*)outputBuffer;

    void *queue_synth_void;
    if (lfds711_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void) == 1) {
        struct _synth *queue_synth = (struct _synth *)queue_synth_void;

        if (queue_synth->oscillators) {
            curr_synth.oscillators = queue_synth->oscillators;
        }

        if (queue_synth->settings) {
            curr_synth.settings = queue_synth->settings;
        }

        if (queue_synth->gain) {
            curr_synth.gain = queue_synth->gain;
        }

#ifdef GRANULAR
        if (queue_synth->grains) {
            curr_synth.grains = queue_synth->grains;
        }
#endif
    }

    unsigned int i, j, k, s, e;

    if (curr_synth.oscillators == NULL ||
#ifdef GRANULAR
        curr_synth.grains == NULL ||
#endif
           curr_synth.settings == NULL ||
               curr_synth.gain == NULL) {
        for (i = 0; i < framesPerBuffer; i += 1) {
            for (j = 0; j < frame_data_count; j += 1) {
                *out++ = 0.0f;
                *out++ = 0.0f;
            }
        }

        curr_synth.lerp_t = 0.0;
        curr_synth.curr_sample = 0;

        lerp_t_step = 1 / note_time_samples;

        return paContinue;
    }

    void *key;
    struct note *_notes;
    struct _freelist_frames_data *freelist_frames_data;
    unsigned int note_buffer_len = 0, pv_note_buffer_len = 0;

    int read_status = 0;

    for (i = 0; i < framesPerBuffer; i += 1) {
        note_buffer_len = 0;
        pv_note_buffer_len = 0;

        for (k = 0; k < frame_data_count; k += 1) {
            float output_l = 0.0f;
            float output_r = 0.0f;

            if (curr_notes != NULL) {
                pv_note_buffer_len += note_buffer_len;
                note_buffer_len = curr_notes[pv_note_buffer_len].osc_index;
                pv_note_buffer_len += 1;
                s = pv_note_buffer_len;
                e = s + note_buffer_len;

                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

#ifdef GRANULAR
                    struct sample *smp = &samples[n->osc_index%samples_count];
                    struct grain *gr = &curr_synth.grains[n->osc_index];

                    float vl = (n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t);
                    float vr = (n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t);

                    output_l += vl * (smp->data[gr->frame + gr->index] * gr->env[gr->frame]);
                    output_r += vr * (smp->data[gr->frame + gr->index + smp->chn] * gr->env[gr->frame]);

                    gr->frame+=gr->direction;

                    if (gr->frame >= gr->frames) {
                        gr->frames = randf(0.001f, 0.02f) * smp->len; //* fas_sample_rate; // 1ms - 100ms
                        gr->index = (smp->len - gr->frames) * randf(0.0f, 1.0f);//floor(randf(0, smp->len - gr->frames - 1));
                        gr->frame = 0;
                        gr->direction = ((rand()%1) == 1) ? -1 : 1;
                    }

#else
                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

    #ifndef FIXED_WAVETABLE
                    float s = fas_sine_wavetable[osc->phase_index[k]];
    #else
                    float s = fas_sine_wavetable[osc->phase_index[k] & fas_wavetable_size_m1];
    #endif

                    float vl = (n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t);
                    float vr = (n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t);

                    output_l += vl * s;
                    output_r += vr * s;

                    osc->phase_index[k] += osc->phase_step * fas_white_noise_table[noise_index++];

    #ifndef FIXED_WAVETABLE
                    if (osc->phase_index[k] >= fas_wavetable_size) {
                        osc->phase_index[k] -= fas_wavetable_size;
                    }
    #endif
#endif
                }
            }

            //output_l /= (e-s);
            //output_r /= (e-s);

            last_sample_l = output_l * curr_synth.gain->gain_lr;
            last_sample_r = output_r * curr_synth.gain->gain_lr;

            *out++ = last_sample_l;
            *out++ = last_sample_r;
        }

        curr_synth.lerp_t += lerp_t_step;

        curr_synth.curr_sample += 1;

        if (curr_synth.curr_sample >= note_time_samples) {
            lerp_t_step = 0;

            curr_synth.curr_sample = 0;

            read_status = lfds711_ringbuffer_read(&rs, &key, NULL);
            if (read_status == 1) {
                freelist_frames_data = (struct _freelist_frames_data *)key;

                _notes = freelist_frames_data->data;

                if (curr_notes) {
                    LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
                    lfds711_freelist_push(&freelist_frames, &curr_freelist_frames_data->fe, NULL);
                }

                curr_notes = _notes;
                curr_freelist_frames_data = freelist_frames_data;

                curr_synth.lerp_t = 0;
                lerp_t_step = 1 / note_time_samples;

                note_buffer_len = curr_notes[0].osc_index;

#ifdef DEBUG
    frames_read += 1;
    if ((frames_read % 512) == 0) {
        printf("%lu frames read\n", frames_read);
    }
#endif

#ifdef PROFILE
    printf("PortAudio stream CPU load : %f\n", Pa_GetStreamCpuLoad(stream));
#endif
            } else {
                if (curr_notes) {
                    LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
                    lfds711_freelist_push(&freelist_frames, &curr_freelist_frames_data->fe, NULL);
                }

                curr_notes = NULL;

                note_buffer_len = 0;
            }
        }
    }

    return paContinue;
}

struct grain *createGrains(unsigned int n) {
    struct grain *g = (struct grain *)malloc(n * sizeof(struct grain));
    for (int i = 0; i < n; i += 1) {
        struct sample *smp = &samples[i%samples_count];

        g[i].frames = floor(randf(0.01f, 0.1f) * fas_sample_rate); // 1ms - 100ms
        g[i].frame = 0;
        g[i].index = floor(randf(0, smp->len - g[i].frames - 1));
        g[i].env = (float *)malloc(g[i].frames * sizeof(float));
        g[i].direction = 1;

        for (int j = 0; j < g[i].frames; j += 1) {
            g[i].env[j] = 1.0;
        }

        int slope_end = g[i].frames / 8;
        double step = 1.0 / slope_end;
        float s = 0;

        for (int j = 0; j < slope_end; j += 1) {
            g[i].env[j] = lin(0.0, 1.0, s);

            s += step;
        }

        s = 0;
        for (int j = (g[i].frames - slope_end); j < g[i].frames; j += 1) {
            g[i].env[j] = lin(1.0, 0.0, s);

            s += step;
        }
    }

    return g;
}

struct grain *freeGrains(struct grain *g, unsigned int n) {
    for (int i = 0; i < n; i += 1) {
        free(g[i].env);
    }

    free(g);

    return NULL;
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
    uint64_t phase_step;
    int nmo = n - 1;

    for (y = 0; y < n; y += 1) {
        index = nmo - y;

        frequency = base_frequency * pow(2, y / octave_length);
        phase_step = frequency / (double)fas_sample_rate * fas_wavetable_size;

        oscillators[index].freq = frequency;

#ifdef FIXED_WAVETABLE
        oscillators[index].phase_index = malloc(sizeof(uint16_t) * frame_data_count);
#else
        oscillators[index].phase_index = malloc(sizeof(unsigned int) * frame_data_count);
#endif

        for (int i = 0; i < frame_data_count; i += 1) {
            oscillators[index].phase_index[i] = rand() / (double)RAND_MAX * fas_wavetable_size;
        }

        oscillators[index].phase_step = phase_step;
    }

    return oscillators;
}

struct oscillator *freeOscillators(struct oscillator *oscs, unsigned int n) {
    if (oscs == NULL) {
        return NULL;
    }

    int y = 0;
    for (y = 0; y < n; y += 1) {
        free(oscs[y].phase_index);
    }

    free(oscs);

    return NULL;
}

void resetNorm(struct osc_norm *_osc_norm) {
    _osc_norm->indexes_len = 16;
    _osc_norm->indexes_curr = 0;
    _osc_norm->divisor = 0;

    _osc_norm->indexes = malloc(sizeof(unsigned int) * _osc_norm->indexes_len);
}

// fill the notes buffer for each output channels
// data argument is the raw RGBA values received with the channels count indicated as the first entry
void fillNotesBuffer(struct note *note_buffer, unsigned int h, size_t data_length, unsigned char *prev_data, unsigned char *data) {
    double pvl = 0, pvr = 0, pl, pr;
    unsigned int i, j, frame_data_index = 8;
    unsigned int l, r;
    unsigned int li = 0, ri = 1;
    unsigned int index = 0, note_osc_index = 0, osc_count = 0;
    double volume_l, volume_r;
    static double inv_full_brightness = 1.0 / 255.0;

    static unsigned int channels[1];
    static unsigned int monophonic[1];

    memcpy(&channels, &((char *) data)[0], sizeof(channels));
    memcpy(&monophonic, &((char *) data)[4], sizeof(monophonic));

    if ((*monophonic) == 1) {
        li = 3;
        ri = 3;
    }

    for (j = 0; j < (*channels); j += 1) {
        note_osc_index = index;
        index += 1;
        osc_count = 0;

        unsigned int y = h - 1;

        for (i = 0; i < data_length; i += 4) {
            pl = prev_data[frame_data_index + li];
            pr = prev_data[frame_data_index + ri];

            l = data[frame_data_index + li];
            r = data[frame_data_index + ri];

            frame_data_index += 4;

            struct note *_note = &note_buffer[index];
            _note->osc_index = y;

            if (l > 0 ) {
                volume_l = l * inv_full_brightness;
                pvl = pl * inv_full_brightness;
                _note->previous_volume_l = pvl;
                _note->diff_volume_l = volume_l - pvl;
            } else {
                if (pl > 0) {
                    pvl = pl * inv_full_brightness;

                    _note->previous_volume_l = pvl;
                    _note->diff_volume_l = -pvl;
                } else {
                    _note->previous_volume_l = 0;
                    _note->diff_volume_l = 0;

                    if (r == 0 && pr == 0) {
                        y -= 1;
                        continue;
                    }
                }
            }

            if (r > 0) {
                volume_r = r * inv_full_brightness;
                pvr = pr * inv_full_brightness;
                _note->previous_volume_r = pvr;
                _note->diff_volume_r = volume_r - pvr;
            } else {
                if (pr > 0) {
                    pvr = pr * inv_full_brightness;

                    _note->previous_volume_r = pvr;
                    _note->diff_volume_r = -pvr;
                } else {
                    _note->previous_volume_r = 0;
                    _note->diff_volume_r = 0;

                    if (l == 0 && pl == 0) {
                        y -= 1;
                        continue;
                    }
                }
            }

            index += 1;

            osc_count += 1;

            y -= 1;
        }

        note_buffer[note_osc_index].osc_index = osc_count;

#ifdef DEBUG
    if ((*monophonic) == 1) {
        printf("Channel l/r (mono) %u : %i oscillators \n", (j + 1), osc_count);
    } else {
        printf("Channel l/r %u : %i oscillators \n", (j + 1), osc_count);
    }
#endif
    }
}

void change_height(unsigned int new_height) {
  lfds711_freelist_cleanup(&freelist_frames, flf_element_cleanup_callback);

  for (int i = 0; i < fas_frames_queue_size; i += 1) {
      ffd[i].data = malloc(sizeof(struct note) * (new_height + 1) * frame_data_count + sizeof(unsigned int));

      LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(ffd[i].fe, &ffd[i]);
      lfds711_freelist_push(&freelist_frames, &ffd[i].fe, NULL);
  }
}

int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
    LFDS711_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE;

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

#ifdef DEBUG
    printf("Packet id: %u\n", pid);
#endif

                if (pid == SYNTH_SETTINGS) {
                    unsigned int h = 0;
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

                        usd->synth->oscillators = NULL;

#ifdef GRANULAR
                        usd->synth->grains = NULL;
#endif
                    } else {
                        h = usd->synth->settings->h;
                    }

#ifdef GRANULAR
                    freeGrains(usd->synth->grains, h);
#endif

                    usd->synth->oscillators = freeOscillators(usd->synth->oscillators, h);

                    usd->synth->gain = NULL;

                    memcpy(usd->synth->settings, &((char *) usd->packet)[PACKET_HEADER_LENGTH], 16);

#ifdef DEBUG
    printf("SYNTH_SETTINGS : %u, %u, %f\n", usd->synth->settings->h,
        usd->synth->settings->octave, usd->synth->settings->base_frequency);
#endif

                    usd->expected_frame_length = 4 * sizeof(unsigned char) * usd->synth->settings->h;
                    usd->expected_max_frame_length = 4 * sizeof(unsigned char) * usd->synth->settings->h * frame_data_count;
                    size_t max_frame_data_len = usd->expected_frame_length * frame_data_count + sizeof(unsigned int);
                    usd->frame_data = malloc(max_frame_data_len);
                    usd->prev_frame_data = calloc(max_frame_data_len, sizeof(unsigned char));

                    usd->synth_h = usd->synth->settings->h;

                    change_height(usd->synth_h);

                    usd->synth->oscillators = createOscillators(usd->synth->settings->h,
                        usd->synth->settings->base_frequency, usd->synth->settings->octave);

#ifdef GRANULAR
                    usd->synth->grains = createGrains(usd->synth_h);
#endif

                    if (lfds711_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
                        printf("Skipping packet, the synth commands queue is full.\n");
                        fflush(stdout);

                        free(usd->synth->settings);
                        usd->synth->oscillators = freeOscillators(usd->synth->oscillators, usd->synth->settings->h);
#ifdef GRANULAR
                        freeGrains(usd->synth->grains, usd->synth->settings->h);
#endif
                        free(usd->synth);
                        usd->synth = NULL;

                        goto free_packet;
                    }

                    usd->synth = NULL;
                } else if (pid == FRAME_DATA) {
#ifdef DEBUG
    printf("FRAME_DATA\n");

    static unsigned int frame_data_length[1];
    memcpy(&frame_data_length, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(frame_data_length));
    printf("Number of channels in the frame: %u\n", *frame_data_length);
#endif
                    //size_t frame_length = usd->packet_len - PACKET_HEADER_LENGTH - FRAME_HEADER_LENGTH;
                    //if (frame_length != usd->expected_frame_length) {
                    //    printf("Skipping a frame, the frame length %zu does not match the expected frame length %zu.\n", frame_length, usd->expected_frame_length);
                    //    fflush(stdout);
                    //    goto free_packet;
                    //}

                    if (usd->frame_data == NULL) {
                        printf("Skipping a frame (alloc. error) until a synth. settings change happen.\n");
                        fflush(stdout);
                        goto free_packet;
                    }

                    memcpy(usd->prev_frame_data, &usd->frame_data[0], usd->expected_max_frame_length);

                    memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->packet_len - PACKET_HEADER_LENGTH);

#ifdef DEBUG
    lfds711_pal_uint_t frames_data_freelist_count;
    lfds711_freelist_query(&freelist_frames, LFDS711_FREELIST_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *)&frames_data_freelist_count);
    printf("frames_data_freelist_count : %llu\n", frames_data_freelist_count);
#endif

                    struct lfds711_freelist_element *fe;
                    struct _freelist_frames_data *freelist_frames_data;
                    int pop_result = lfds711_freelist_pop(&freelist_frames, &fe, NULL);
                    if (pop_result == 0) {
                        printf("Skipping a frame, notes buffer freelist is empty.\n");
                        fflush(stdout);
                        goto free_packet;
                    }

                    freelist_frames_data = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

                    memset(freelist_frames_data->data, 0, sizeof(struct note) * (usd->synth_h + 1) * frame_data_count + sizeof(unsigned int));

                    fillNotesBuffer(freelist_frames_data->data, usd->synth_h, usd->expected_frame_length, (unsigned char *)usd->prev_frame_data, (unsigned char *)usd->frame_data);

                    struct _freelist_frames_data *overwritten_notes = NULL;
                    lfds711_ringbuffer_write(&rs, (void *) (lfds711_pal_uint_t) freelist_frames_data, NULL, &overwrite_occurred_flag, (void *)&overwritten_notes, NULL);
                    if (overwrite_occurred_flag == LFDS711_MISC_FLAG_RAISED) {
                        // okay, push it back!
                        LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(overwritten_notes->fe, overwritten_notes);
                        lfds711_freelist_push(&freelist_frames, &overwritten_notes->fe, NULL);
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
                    if (lfds711_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
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
            if (usd->synth) {
                if (usd->synth->oscillators && usd->synth->settings) {
                    usd->synth->oscillators = freeOscillators(usd->synth->oscillators, usd->synth->settings->h);
                }
#ifdef GRANULAR
                freeGrains(usd->synth->grains, usd->synth->settings->h);
#endif
                free(usd->synth->settings);
                free(usd->synth);
            }
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
        .port = 3003, .iface = fas_iface, .protocols = protocols, .extensions = NULL,
        .ssl_cert_filepath = NULL, .ssl_private_key_filepath = NULL, .ssl_ca_filepath = NULL,
        .gid = -1, .uid = -1, .options = 0, NULL, .ka_time = 0, .ka_probes = 0, .ka_interval = 0
    };

    context_info.port = fas_port;

    if (fas_deflate) {
        context_info.extensions = exts;
    }

    if (fas_ssl) {
        context_info.ssl_private_key_filepath = "./server.key";
        context_info.ssl_cert_filepath = "./server.crt";
        context_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    }

    context = lws_create_context(&context_info);

    if (context == NULL) {
        fprintf(stderr, "lws_create_context failed.\n");
        return -1;
    }

    printf("Fragment Synthesizer successfully started and listening on %s:%u.\n", (fas_iface == NULL) ? "127.0.0.1" : fas_iface, fas_port);

    return 0;
}

void load_grains() {
    size_t path_len;

    const char *grains_directory = "./grains/";

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    tinydir_dir dir;
    tinydir_open(&dir, grains_directory);

    samples = malloc(sizeof(struct sample) * 1);

    while (dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (file.is_reg) {
            if (strcmp(file.extension, "flac") == 0 ||
                  strcmp(file.extension, "wav") == 0) {
                size_t filepath_len = strlen(grains_directory) + strlen(file.name);
                char *filepath = (char *)malloc(filepath_len + 1);
                filepath[0] = '\0';
                strcat(filepath, grains_directory);
                strcat(filepath, file.name);
                filepath[filepath_len] = '\0';

                SNDFILE *audio_file;
                if (!(audio_file = sf_open(filepath, SFM_READ, &sfinfo))) {
                    printf ("libsdnfile: Not able to open input file %s.\n", filepath);
                    puts(sf_strerror(NULL));

                    free(filepath);

                    continue;
                }

                free(filepath);

                //printf("%i\n", sfinfo.frames);
                //printf("%i\n", sfinfo.channels);

                samples_count++;

                samples = (struct sample *)realloc(samples, sizeof(struct sample) * samples_count);

                struct sample *smp = &samples[samples_count - 1];
                smp->chn = sfinfo.channels - 1;
                smp->len = sfinfo.frames;
                smp->data = (float *)malloc(smp->len * sizeof(float));
                sf_read_float(audio_file, smp->data, smp->len);

                sf_close(audio_file);
            }
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
}

void print_usage() {
    printf("Usage: fas [list_of_settings]\n");
    printf(" possible settings with associated default value:\n");
    printf("  --i\n");
    printf("  --sample_rate %u\n", FAS_SAMPLE_RATE);
    printf("  --frames %u\n", FAS_FRAMES_PER_BUFFER);
    printf("  --noise_amount %f\n", FAS_NOISE_AMOUNT);
    //printf("  --wavetable %u\n", FAS_WAVETABLE);
#ifndef FIXED_WAVETABLE
    printf("  --wavetable_size %u\n", FAS_WAVETABLE_SIZE);
#endif
    printf("  --fps %u\n", FAS_FPS);
    printf("  --ssl %u\n", FAS_SSL);
    printf("  --deflate %u\n", FAS_DEFLATE);
    printf("  --rx_buffer_size %u\n", FAS_RX_BUFFER_SIZE);
    printf("  --port %u\n", FAS_PORT);
    printf("  --iface 127.0.0.1\n");
    printf("  --device -1\n");
    printf("  --output_channels %u\n", FAS_OUTPUT_CHANNELS);
#ifdef __unix__
    printf("  --alsa_realtime_scheduling %u\n", FAS_REALTIME);
#endif
    printf("  --frames_queue_size %u\n", FAS_FRAMES_QUEUE_SIZE);
    printf("  --commands_queue_size %u\n", FAS_COMMANDS_QUEUE_SIZE);
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

    fas_white_noise_table = (float*)calloc(65536, sizeof(float));

    for (i = 0; i < 65536; i += 1) {
        fas_white_noise_table[i] = 1. + randf(-fas_noise_amount, fas_noise_amount);
    }

    return 0;
}

int main(int argc, char **argv)
{
    int print_infos = 0;

    int i = 0;

    static struct option long_options[] = {
        { "sample_rate",              required_argument, 0, 0 },
        { "frames",                   required_argument, 0, 1 },
        { "wavetable",                required_argument, 0, 2 },
#ifndef FIXED_WAVETABLE
        { "wavetable_size",           required_argument, 0, 3 },
#endif
        { "fps",                      required_argument, 0, 4 },
        { "deflate",                  required_argument, 0, 5 },
        { "rx_buffer_size",           required_argument, 0, 6 },
        { "port",                     required_argument, 0, 7 },
        { "alsa_realtime_scheduling", required_argument, 0, 8 },
        { "frames_queue_size",        required_argument, 0, 9 },
        { "commands_queue_size",      required_argument, 0, 10 },
        { "ssl",                      required_argument, 0, 11 },
        { "iface",                    required_argument, 0, 12 },
        { "device",                   required_argument, 0, 13 },
        { "output_channels",          required_argument, 0, 14 },
        { "i",                              no_argument, 0, 15 },
        { "noise_amount",             required_argument, 0, 16 },
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
                fas_ssl = strtoul(optarg, NULL, 0);
                break;
            case 12:
                fas_iface = optarg;
                break;
            case 13:
                fas_audio_device = strtoul(optarg, NULL, 0);
                break;
            case 14:
                fas_output_channels = strtoul(optarg, NULL, 0);
                break;
            case 15:
                print_infos = 1;
                break;
            case 16:
                fas_noise_amount = strtof(optarg, NULL);
                break;
            default: print_usage();
                 return EXIT_FAILURE;
        }
    }

    if (fas_sample_rate == 0) {
        printf("Warning: sample_rate program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_SAMPLE_RATE);

        fas_sample_rate = FAS_SAMPLE_RATE;
    }

    if (fas_wavetable_size == 0) {
        printf("Warning: wavetable_size program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_WAVETABLE_SIZE);

        fas_wavetable_size = FAS_WAVETABLE_SIZE;
    }

    fas_wavetable_size_m1 = fas_wavetable_size - 1;

    if (fas_fps == 0) {
        printf("Warning: fps program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_FPS);

        fas_fps = FAS_FPS;
    }

    if (fas_port == 0) {
        printf("Warning: port program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_PORT);

        fas_port = FAS_PORT;
    }

    if (fas_frames_per_buffer == 0) {
        printf("Warning: frames program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_FRAMES_PER_BUFFER);

        fas_frames_per_buffer = FAS_FRAMES_PER_BUFFER;
    }

    if (fas_rx_buffer_size == 0) {
        printf("Warning: rx_buffer_size program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_RX_BUFFER_SIZE);

        fas_rx_buffer_size = FAS_RX_BUFFER_SIZE;
    }

    if (fas_frames_queue_size == 0) {
        printf("Warning: frames_queue_size program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_FRAMES_QUEUE_SIZE);

        fas_frames_queue_size = FAS_FRAMES_QUEUE_SIZE;
    }

    if (fas_commands_queue_size == 0) {
        printf("Warning: commands_queue_size program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_COMMANDS_QUEUE_SIZE);

        fas_commands_queue_size = FAS_COMMANDS_QUEUE_SIZE;
    }

    if (fas_output_channels < 2 || (fas_output_channels % 2) != 0) {
        printf("Warning: output_channels program option argument is invalid, should be >= 2, the default value (%u) will be used.\n", FAS_OUTPUT_CHANNELS);

        fas_output_channels = FAS_OUTPUT_CHANNELS;
    }

    if (fas_noise_amount < 0.) {
        printf("Warning: noise_amount program option argument is invalid, should be >= 0, the default value (%f) will be used.\n", FAS_NOISE_AMOUNT);

        fas_noise_amount = FAS_NOISE_AMOUNT;
    }

    if (errno == ERANGE) {
        printf("Warning: One of the specified program option is out of range and was set to its maximal value.\n");
    }

#ifdef GRANULAR
    load_grains();
#endif

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
    PaError err;

    memset(&outputParameters, 0, sizeof(PaStreamParameters));

    err = Pa_Initialize();
    if (err != paNoError) goto error;

    // get informations about devices
    int num_devices;
    num_devices = Pa_GetDeviceCount();
    if (num_devices < 0) {
        fprintf(stderr, "Error: Pa_CountDevices returned 0x%x\n", num_devices);
        err = num_devices;
        goto error;
    }

    const PaDeviceInfo *device_info;
    for (i = 0; i < num_devices; i += 1) {
        device_info = Pa_GetDeviceInfo(i);

        printf("\nPortAudio device %i - %s\n==========\n", i, device_info->name);
        printf("  max input channels : %i\n", device_info->maxInputChannels);
        printf("  max output channels : %i\n", device_info->maxOutputChannels);
        printf("  default low input latency : %f\n", device_info->defaultLowInputLatency);
        printf("  default low output latency : %f\n", device_info->defaultLowOutputLatency);
        printf("  default high input latency : %f\n", device_info->defaultHighInputLatency);
        printf("  default high output latency : %f\n", device_info->defaultHighOutputLatency);
        printf("  default sample rate : %f\n", device_info->defaultSampleRate);
    }

    printf("\n");

    if (print_infos == 1) {
        goto error;
    }

    curr_synth.settings = NULL;
    curr_synth.gain = NULL;
    curr_synth.oscillators = NULL;
    curr_synth.lerp_t = 0.0;
    curr_synth.curr_sample = 0;

    int device_max_output_channels;
    if (fas_audio_device >= num_devices || fas_audio_device < 0) {
        outputParameters.device = Pa_GetDefaultOutputDevice();
        if (outputParameters.device == paNoDevice) {
            fprintf(stderr, "Error: No default output device.\n");
            goto error;
        }

        device_max_output_channels = Pa_GetDeviceInfo(outputParameters.device)->maxOutputChannels;
        if (fas_output_channels > device_max_output_channels) {
            printf("Warning: Requested output_channels program option is larger than the device output channels, defaulting to %i\n", device_max_output_channels);
            fas_output_channels = device_max_output_channels;
        }

        outputParameters.channelCount = fas_output_channels;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
    } else {
        device_info = Pa_GetDeviceInfo(fas_audio_device);

        device_max_output_channels = device_info->maxOutputChannels;
        if (fas_output_channels > device_max_output_channels) {
            printf("Warning: Requested output_channels program option is larger than the device output channels, defaulting to %i\n", device_max_output_channels);
            fas_output_channels = device_max_output_channels;
        }

        outputParameters.device = fas_audio_device;
        outputParameters.channelCount = fas_output_channels;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
    }

    frame_data_count = fas_output_channels / 2;

    printf("\nPortAudio: Using device '%s' with %u output channels\n", Pa_GetDeviceInfo(outputParameters.device)->name, fas_output_channels);

    err = Pa_IsFormatSupported(NULL, &outputParameters, (double)fas_sample_rate);
    if (err != paFormatIsSupported) {
       printf("Pa_IsFormatSupported : Some device output parameters are unsupported!\n");
    }

#ifdef __unix__
    if (fas_realtime) {
        PaAlsa_EnableRealtimeScheduling(&stream, fas_realtime);
    }
#endif

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

#if defined(_WIN32) || defined(_WIN64)
    struct lfds711_ringbuffer_element *re =
        malloc(sizeof(struct lfds711_ringbuffer_element) * (fas_frames_queue_size + 1));

    struct lfds711_queue_bss_element *synth_commands_queue_element =
        malloc(sizeof(struct lfds711_queue_bss_element) * fas_commands_queue_size);
#else
    struct lfds711_ringbuffer_element *re =
        aligned_alloc(fas_frames_queue_size + 1, sizeof(struct lfds711_ringbuffer_element) * (fas_frames_queue_size + 1));

    struct lfds711_queue_bss_element *synth_commands_queue_element =
        aligned_alloc(fas_commands_queue_size, sizeof(struct lfds711_queue_bss_element) * fas_commands_queue_size);
#endif

    if (re == NULL) {
        fprintf(stderr, "lfds rb data structures alloc./align. error.\n");
        goto quit;
    }

    if (synth_commands_queue_element == NULL) {
        fprintf(stderr, "lfds queue data structures alloc./align. error.\n");
        free(re);
        goto quit;
    }

    lfds711_ringbuffer_init_valid_on_current_logical_core(&rs, re, (fas_frames_queue_size + 1), NULL);
    lfds711_queue_bss_init_valid_on_current_logical_core(&synth_commands_queue_state, synth_commands_queue_element, fas_commands_queue_size, NULL);
    lfds711_freelist_init_valid_on_current_logical_core(&freelist_frames, NULL, 0, NULL);

    ffd = malloc(sizeof(struct _freelist_frames_data) * fas_frames_queue_size);
    if (ffd == NULL) {
        fprintf(stderr, "_freelist_frames_data data structure alloc. error.\n");
        free(re);
        free(synth_commands_queue_element);
        goto quit;
    }

    fflush(stdout);
    fflush(stderr);

    srand(time(NULL));

    // websocket stuff
#ifdef __unix__
    struct timeval tv = { 0, 0 };
    fd_set fdset;
    int select_result;
    int fd = fileno(stdin);
#endif
    do {
        lws_service(context, 1);

#if defined(_WIN32) || defined(_WIN64)
	if (_kbhit()) {
            break;
	}
    } while (1);
#else
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
    } while ((select_result = select(fd + 1, &fdset, NULL, NULL, &tv)) == 0);
#endif

quit:

    // thank you for your attention, bye.
    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    lws_context_destroy(context);

    Pa_Terminate();

    free(fas_sine_wavetable);
    free(fas_white_noise_table);

    if (re) {
        lfds711_ringbuffer_cleanup(&rs, rb_element_cleanup_callback);
        free(re);
    }

    lfds711_freelist_cleanup(&freelist_frames, flf_element_cleanup_callback);

    if (ffd) {
        free(ffd);
    }

    if (synth_commands_queue_element) {
        lfds711_queue_bss_cleanup(&synth_commands_queue_state, q_element_cleanup_callback);
        free(synth_commands_queue_element);
    }
/*
    if (osc_norms) {
        free(osc_norms);
    }
*/
    printf("Bye.\n");

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
    free(fas_white_noise_table);

    for (i = 0; i < samples_count; i++) {
      struct sample *smp = &samples[i];
      free(smp->data);
    }
    free(samples);

    return err;
}
