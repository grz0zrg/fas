/*
    Copyright (c) 2017, 2018, Julien Verneuil
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
    Additive/spectral/granular/PM synthesizer built for the Fragment Synthesizer, a web-based image-synth collaborative audio/visual synthesizer.

    This collect Fragment settings and notes data over WebSocket, convert them to a suitable data structure and generate sound from it in real-time for a smooth experience.

    Only one client is supported (altough many can connect, not tested but it may result in a big audio mess and likely a crash!)

    You can tweak this program by passing settings to its arguments, for help : fas --h

    Can be used as a generic additive/spectral/granular/PM synthesizer if you feed it correctly!

    https://www.fsynth.com

    Author : Julien Verneuil
    License : Simplified BSD License

    TODO : data coming from the network for notes etc. should NOT be handled like it is right now (it should instead come with IEEE 754 representation or something...)
*/

#include "fas.h"

CEssentia cessentia;

// liblfds data structures cleanup callbacks
void flf_element_cleanup_callback(struct lfds711_freelist_state *fs, struct lfds711_freelist_element *fe) {
    struct _freelist_frames_data *freelist_frames_data;
    freelist_frames_data = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

    free(freelist_frames_data->data);
}

unsigned int smp_s = 0;

float phase = 0.;
float *b = 0;

void reset_granular(unsigned int chn, struct oscillator *osc, struct note *n) {

}

void reset_phys_modelling(unsigned int chn, struct oscillator *osc, struct note *n) {
    unsigned int d = 0;

    memset(osc->fp1[chn], 0, sizeof(double) * 4);
    memset(osc->fp2[chn], 0, sizeof(double) * 4);
    memset(osc->fp3[chn], 0, sizeof(double) * 4);
    memset(osc->fp4[chn], 0, sizeof(double) * 4);

    osc->pvalue[chn] = 0.0f;
    osc->fphase_index[chn] = 0.0f;
    //osc->fp4[chn][0] = randf(0., 1.);

    // fill with noise & filter
    for (d = 0; d < osc->buffer_len; d += 1) {
        unsigned int bindex = chn * osc->buffer_len + d;
        osc->buffer[bindex] = fas_white_noise_table[d % fas_wavetable_size];
        osc->buffer[bindex] = huovilainen_moog(osc->buffer[bindex], n->cutoff, n->res, osc->fp1[chn], osc->fp2[chn], osc->fp3[chn], 2);
    }
}

static int paCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *data )
{
    LFDS711_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_LOGICAL_CORE;

    float *out = (float*)outputBuffer;

    unsigned int i, j, k, d, s, e;

    void *queue_synth_void;
    if (lfds711_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void) == 1) {
        struct _synth *queue_synth = (struct _synth *)queue_synth_void;

        if (queue_synth->oscillators) {
            if (curr_synth.oscillators) {
                freeOscillators(&curr_synth.oscillators, curr_synth.settings->h, frame_data_count);

                curr_synth.oscillators = NULL;
            }

            curr_synth.oscillators = queue_synth->oscillators;
        }

        if (queue_synth->gain) {
            if (curr_synth.gain) {
                free(curr_synth.gain);

                curr_synth.gain = NULL;
            }

            curr_synth.gain = queue_synth->gain;
        }

        if (queue_synth->grains) {
            if (curr_synth.grains) {
                freeGrains(&curr_synth.grains, curr_synth.samples_count, frame_data_count, curr_synth.settings->h, fas_granular_max_density);

                curr_synth.grains = NULL;
            }

            curr_synth.grains = queue_synth->grains;
        }

        if (queue_synth->settings) {
            if (curr_synth.settings) {
                free(curr_synth.settings);

                curr_synth.settings = NULL;
            }

            curr_synth.settings = queue_synth->settings;
        }

        if (queue_synth->chn_settings) {
            if (curr_synth.chn_settings) {
                free(curr_synth.chn_settings);

                curr_synth.chn_settings = NULL;
            }

            curr_synth.chn_settings = queue_synth->chn_settings;

            // do not allow synthesis based on samples when there is no samples
            if (samples_count == 0) {
                for (k = 0; k < frame_data_count; k += 1) {
                    struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[k];

                    if (chn_settings->synthesis_method == FAS_GRANULAR) {
                        chn_settings->synthesis_method = FAS_VOID;
                    }
                }
            }
        }

        // note retrigger (only used for physical modelling)
        if (queue_synth->chn > 0) {
            struct oscillator *osc = &curr_synth.oscillators[queue_synth->note];

            k = queue_synth->chn - 1;

            osc->triggered = 1;

            //struct note *n = &curr_notes[queue_synth->note];

            //reset_phys_modelling(k, osc, n);
        }

        curr_synth.samples_count = queue_synth->samples_count;

        free(queue_synth);
    }

    int read_status = 0;
    void *key;

    if (audio_thread_state == FAS_AUDIO_DO_PAUSE) {
        audio_thread_state = FAS_AUDIO_PAUSE;
    } else if (audio_thread_state == FAS_AUDIO_DO_PLAY) {
        audio_thread_state = FAS_AUDIO_PLAY;
    }

    if (curr_synth.oscillators == NULL ||
        curr_synth.settings == NULL ||
        curr_synth.gain == NULL ||
        curr_synth.chn_settings == NULL ||
        audio_thread_state == FAS_AUDIO_PAUSE ||
        audio_thread_state == FAS_AUDIO_DO_WAIT_SETTINGS) {
        for (i = 0; i < framesPerBuffer; i += 1) {
            for (j = 0; j < frame_data_count; j += 1) {
                *out++ = last_sample_l[j] * (1.0f - curr_synth.lerp_t);
                *out++ = last_sample_r[j] * (1.0f - curr_synth.lerp_t);
            }

            curr_synth.lerp_t += (1.0f / (float)framesPerBuffer);
            curr_synth.lerp_t = fminf(curr_synth.lerp_t, 1.0f);
        }

        curr_notes = NULL;

        curr_synth.lerp_t = 0.0;
        curr_synth.curr_sample = 0;

        lerp_t_step = 1 / note_time_samples;

        return paContinue;
    }

    struct note *_notes;
    struct _freelist_frames_data *freelist_frames_data;
    unsigned int note_buffer_len = 0, pv_note_buffer_len = 0;

    for (i = 0; i < framesPerBuffer; i += 1) {
        note_buffer_len = 0;
        pv_note_buffer_len = 0;
/*
        if (curr_synth.chn_settings[0].env_type == FAS_SPECTRAL) {
            smp_s = smp_s % hop_size;
            if (smp_s == 0 && curr_notes != NULL) {
                b = computeSineModelCEssentia(cessentia, curr_notes[0].mag, curr_notes[0].frq, curr_notes[0].pha, curr_notes[0].osc_index);
                //aubio_pvoc_rdo(pv, curr_notes[0].fftgrain, ifft_out);
            }

            if(b && curr_notes != NULL){
                    if (!isnan(b[smp_s])) {
                      *out++ = b[smp_s] * curr_synth.gain->gain_lr;//ifft_out->data[smp_s];
                      *out++ = b[smp_s] * curr_synth.gain->gain_lr;//ifft_out->data[smp_s];
                    }
            }

                    smp_s++;
        }
*/
        for (k = 0; k < frame_data_count; k += 1) {
            float output_l = 0.0f;
            float output_r = 0.0f;

            if (curr_notes) {
                pv_note_buffer_len += note_buffer_len;
                note_buffer_len = curr_notes[pv_note_buffer_len].osc_index;
                pv_note_buffer_len += 1;
                s = pv_note_buffer_len;
                e = s + note_buffer_len;

                struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[k];

                if (chn_settings->synthesis_method == FAS_ADDITIVE) {
                    for (j = s; j < e; j += 1) {
                        struct note *n = &curr_notes[j];

                        struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifndef FIXED_WAVETABLE
                        float s = fas_sine_wavetable[osc->phase_index[k]];
#else
                        float s = fas_sine_wavetable[osc->phase_index[k] & fas_wavetable_size_m1];
#endif

                        float vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                        float vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                        output_l += vl * s;
                        output_r += vr * s;

#ifdef BANDLIMITED_NOISE
                        osc->phase_index[k] += osc->phase_step * (1.0f + (fas_white_noise_table[osc->noise_index[k]++] * fas_noise_amount) * n->blue);
#else
                        osc->phase_index[k] += osc->phase_step;
#endif

#ifndef FIXED_WAVETABLE
                        if (osc->phase_index[k] >= fas_wavetable_size) {
                            osc->phase_index[k] -= fas_wavetable_size;
                        }
#endif
                    }
                } else if (chn_settings->synthesis_method == FAS_GRANULAR) {
                    int env_type = chn_settings->env_type;
                    float *gr_env = grain_envelope[env_type];

                    for (j = s; j < e; j += 1) {
                        struct note *n = &curr_notes[j];

                        struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                        float vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                        float vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                        unsigned int grain_index = n->osc_index * samples_count + n->psmp_index;
                        unsigned int si = curr_synth.settings->h * samples_count;

                        struct grain *gr = &curr_synth.grains[grain_index];

                        float gr_out_l = 0, gr_out_r = 0;
                        computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, chn_settings->p3, gr_env, samples, n->psmp_index, fas_sample_rate, chn_settings->p1, chn_settings->p2, &gr_out_l, &gr_out_r);
/*
                        // WIP :  allow real-time density change
                        unsigned density_difference = n->density - n->pdensity;
                        if (density_difference < 0) {
                            gr->density[k] = -density_difference;

                            computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, n->density, gr_env, samples, n->psmp_index, chn_settings->p1, chn_settings->p2, &gr_out_l, &gr_out_r);

                            output_l += (vl * gr->density[k]) * gr_out_l * (1.0f - curr_synth.lerp_t);
                            output_r += (vr * gr->density[k]) * gr_out_r * (1.0f - curr_synth.lerp_t);

                            gr->density[k] = n->density;

                            gr_out_l = 0; gr_out_r = 0;
                            computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, 0, gr_env, samples, n->psmp_index, chn_settings->p1, chn_settings->p2, &gr_out_l, &gr_out_r);
                        } else {
                            computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, 0, gr_env, samples, n->psmp_index, chn_settings->p1, chn_settings->p2, &gr_out_l, &gr_out_r);
                        }
*/
                        // allow real-time sample change : cross-fade between old & new on a sudden sample change
                        if (n->psmp_index != n->smp_index) {
                            output_l += (vl * n->norm_density) * gr_out_l * (1.0f - curr_synth.lerp_t);
                            output_r += (vr * n->norm_density) * gr_out_r * (1.0f - curr_synth.lerp_t);

                            grain_index = n->osc_index * samples_count + n->smp_index;

                            gr_out_l = 0; gr_out_r = 0;
                            computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, chn_settings->p3, gr_env, samples, n->smp_index, fas_sample_rate, chn_settings->p1, chn_settings->p2, &gr_out_l, &gr_out_r);

                            output_l += (vl * n->density) * gr_out_l;
                            output_r += (vr * n->density) * gr_out_r;
                        } else {
                            output_l += (vl * n->density) * gr_out_l;
                            output_r += (vr * n->density) * gr_out_r;
                        }
                    }
                } else if (chn_settings->synthesis_method == FAS_FM) {
                    for (j = s; j < e; j += 1) {
                        struct note *n = &curr_notes[j];

                        struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                        double phase_step = n->alpha / (double)fas_sample_rate * fas_wavetable_size;

#ifndef FIXED_WAVETABLE
                        float s = fas_sine_wavetable[osc->phase_index[k]];
                        float s2 = fas_sine_wavetable[osc->phase_index2[k]];
#else
                        float s = fas_sine_wavetable[osc->phase_index[k] & fas_wavetable_size_m1];
                        float s2 = fas_sine_wavetable[osc->phase_index2[k] & fas_wavetable_size_m1];
#endif

                        float vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                        float vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                        output_l += vl * s;
                        output_r += vr * s;

                        osc->phase_index[k] += osc->phase_step + ((s2 * n->blue_frac_part) * fas_wavetable_size);
                        osc->phase_index2[k] += phase_step;

#ifndef FIXED_WAVETABLE
                        if (osc->phase_index[k] >= fas_wavetable_size) {
                            osc->phase_index[k] -= fas_wavetable_size;
                        }

                        if (osc->phase_index2[k] >= fas_wavetable_size) {
                            osc->phase_index2[k] -= fas_wavetable_size;
                        }
#endif
                    }
                } else if (chn_settings->synthesis_method == FAS_SUBTRACTIVE) {
                  for (j = s; j < e; j += 1) {
                      struct note *n = &curr_notes[j];

                      struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                      // fundamental
#ifndef FIXED_WAVETABLE
                      float s = fas_sine_wavetable[osc->phase_index[k]];

                      uint16_t *hphase_index = (uint16_t *)osc->harmo_phase_index[k];
#else
                      float s = fas_sine_wavetable[osc->phase_index[k] & fas_wavetable_size_m1];

                      unsigned int *hphase_index = (unsigned int *)osc->harmo_phase_index[k];
#endif

                      // harmonics / waveform generation through additive synthesis
                      int odd = n->waveform; // sawtooth - square & triangle (pow wavetable below)
                      for (d = odd; d < osc->max_harmonics; d += (1 + odd)) {
#ifndef FIXED_WAVETABLE
                          s += fas_sine_wavetable[osc->harmo_phase_index[k][d]] * osc->harmonics[d + osc->max_harmonics * ((int)n->exp)];
#else
                          s += fas_sine_wavetable[osc->harmo_phase_index[k][d] & fas_wavetable_size_m1] * osc->harmonics[d + osc->max_harmonics * ((int)n->exp)];
#endif

                          osc->harmo_phase_index[k][d] += osc->harmo_phase_step[d];

#ifndef FIXED_WAVETABLE
                          if (osc->harmo_phase_index[k][d] >= fas_wavetable_size) {
                              osc->harmo_phase_index[k][d] -= fas_wavetable_size;
                          }
#endif
                      }

                      //s = improved_moog(s, osc->freq * n->cutoff, n->res, chn_settings->p1, osc->fp1[k], osc->fp2[k], osc->fp3[k], (double)fas_sample_rate);
                      s = huovilainen_moog(s, n->cutoff, n->res, osc->fp1[k], osc->fp2[k], osc->fp3[k], 2);

                      float vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                      float vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                      output_l += vl * s;
                      output_r += vr * s;

                      osc->phase_index[k] += osc->phase_step;

#ifndef FIXED_WAVETABLE
                      if (osc->phase_index[k] >= fas_wavetable_size) {
                          osc->phase_index[k] -= fas_wavetable_size;
                      }
#endif
                  }
                } else if (chn_settings->synthesis_method == FAS_PHYSICAL_MODELLING) {
                    for (j = s; j < e; j += 1) {
                        struct note *n = &curr_notes[j];

                        struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                        double phase_step = osc->freq / (double)fas_sample_rate * (osc->buffer_len + 0.5);

                        float vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                        float vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                        unsigned int curr_sample_index = osc->fphase_index[k];
                        unsigned int curr_sample_index2 = curr_sample_index + 1;

                        float mu = osc->fphase_index[k] - curr_sample_index;

                        unsigned int curr_sample = k * osc->buffer_len + (curr_sample_index % osc->buffer_len);
                        unsigned int curr_sample2 = k * osc->buffer_len + (curr_sample_index2 % osc->buffer_len);

                        float smp = osc->buffer[curr_sample];

                        float stretch = n->blue_frac_part;
                        float in = 0.0f;
                        if (stretch <= randf(0.f, 1.f)) {
                            in = 0.5f * ((smp + mu * (osc->buffer[curr_sample2] - smp)) + osc->pvalue[k]);
                        } else {
                            in = smp;
                        }

                        // allpass
                        float delay = fabsf((double)osc->buffer_len - ((double)fas_sample_rate / osc->freq));
                        float c = (1.0f - delay) / (1.0f + delay);

                        osc->buffer[curr_sample] = osc->fp4[k][0] + c * in;
                        osc->fp4[k][0] = in - c * osc->buffer[curr_sample];

                        float ol = osc->buffer[curr_sample];

                        osc->pvalue[k] = ol;

                        output_l += vl * ol;
                        output_r += vr * ol;

                        osc->fphase_index[k] += phase_step;
                    }
                }
            }

            last_sample_l[k] = output_l * curr_synth.gain->gain_lr;
            last_sample_r[k] = output_r * curr_synth.gain->gain_lr;

            *out++ = last_sample_l[k];
            *out++ = last_sample_r[k];
        }

        curr_synth.lerp_t += lerp_t_step * fas_smooth_factor;
        curr_synth.lerp_t = fminf(curr_synth.lerp_t, 1.0f);

        curr_synth.curr_sample += 1;

        // process the next event
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

                fas_drop_counter = 0;

                // once we have notes data we prepare some pre-processing for later use (optimization, reseting filters on note-on etc.)
                note_buffer_len = 0;
                pv_note_buffer_len = 0;

                for (k = 0; k < frame_data_count; k += 1) {
                    float output_l = 0.0f;
                    float output_r = 0.0f;

                    if (curr_notes) {
                        pv_note_buffer_len += note_buffer_len;
                        note_buffer_len = curr_notes[pv_note_buffer_len].osc_index;
                        pv_note_buffer_len += 1;
                        s = pv_note_buffer_len;
                        e = s + note_buffer_len;

                        struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[k];

                        if (chn_settings->synthesis_method == FAS_GRANULAR) {
                            for (j = s; j < e; j += 1) {
                                struct note *n = &curr_notes[j];

                                // reset granular envelope; force grains creation
                                if (n->smp_index != n->psmp_index) {
                                    unsigned int grain_index = n->osc_index * samples_count + n->smp_index;
                                    unsigned int si = curr_synth.settings->h * samples_count;

                                    struct grain *gr = &curr_synth.grains[grain_index];

                                    for (d = 0; d < gr->density[k]; d += 1) {
                                        gr = &curr_synth.grains[grain_index + (d * si)];

                                        gr->env_index[k] = FAS_ENVS_SIZE;
                                    }
                                }

                                if (n->previous_volume_l <= 0 && n->previous_volume_r <= 0) {
                                    unsigned int grain_index = n->osc_index * samples_count + n->smp_index;
                                    unsigned int pgrain_index = n->osc_index * samples_count + n->psmp_index;
                                    unsigned int si = curr_synth.settings->h * samples_count;

                                    struct grain *gr = &curr_synth.grains[grain_index];
                                    struct grain *gr2 = &curr_synth.grains[pgrain_index];

                                    for (d = 0; d < gr->density[k]; d += 1) {
                                        gr = &curr_synth.grains[grain_index + (d * si)];

                                        gr->env_index[k] = FAS_ENVS_SIZE;
                                    }

                                    for (d = 0; d < gr2->density[k]; d += 1) {
                                        gr2 = &curr_synth.grains[pgrain_index + (d * si)];

                                        gr2->env_index[k] = FAS_ENVS_SIZE;
                                    }
                                }
                            }
                        } else if (chn_settings->synthesis_method == FAS_SUBTRACTIVE || chn_settings->synthesis_method == FAS_FM) {
                            for (j = s; j < e; j += 1) {
                                struct note *n = &curr_notes[j];

                                struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                                // TODO : pre-compute when necessary
                                huovilainen_compute(osc->freq * n->cutoff, n->res, &n->cutoff, &n->res, (double)fas_sample_rate);

                                // reset filter on note-off
                                if (n->previous_volume_l <= 0 && n->previous_volume_r <= 0) {
                                    memset(osc->fp1[k], 0, sizeof(double) * 4);
                                    memset(osc->fp2[k], 0, sizeof(double) * 4);
                                    memset(osc->fp3[k], 0, sizeof(double) * 4);
                                }
                            }
                        } else if (chn_settings->synthesis_method == FAS_PHYSICAL_MODELLING) {
                            for (j = s; j < e; j += 1) {
                                struct note *n = &curr_notes[j];

                                struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                                huovilainen_compute(osc->freq * n->cutoff, n->res, &n->cutoff, &n->res, (double)fas_sample_rate);

                                if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || osc->triggered == 1) {
                                    reset_phys_modelling(k, osc, n);

                                    osc->triggered = 0;
                                }
                            }
                        }
                    }
                }

#ifdef DEBUG
    frames_read += 1;
    if ((frames_read % 512) == 0) {
        printf("%lu frames read\n", frames_read);
        fflush(stdout);
    }
#endif

#ifdef PROFILE
    printf("PortAudio stream CPU load : %f\n", Pa_GetStreamCpuLoad(stream));
    fflush(stdout);
#endif
            } else {
                // allow some frame drop, hold the current note events to FAS_MAX_DROP if that happen
                // ensure smooth audio in most situations (the only downside : it may sound delayed, the impact depend on how many frames are dropped)
                fas_drop_counter += 1;
                if (fas_drop_counter >= fas_max_drop) {
                    if (curr_notes) {
                        LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
                        lfds711_freelist_push(&freelist_frames, &curr_freelist_frames_data->fe, NULL);
                    }

                    curr_notes = NULL;

                    note_buffer_len = 0;

                    fas_drop_counter = 0;
                }
            }
        }
    }

    return paContinue;
}

void audioPause() {
    audio_thread_state = FAS_AUDIO_DO_PAUSE;

    while (audio_thread_state != FAS_AUDIO_PAUSE);
}

void audioPlay() {
    audio_thread_state = FAS_AUDIO_DO_PLAY;
}

void setHeight(unsigned int new_height) {
    struct lfds711_freelist_element *fe;
    struct _freelist_frames_data *freelist_frames_data;

    while (lfds711_freelist_pop(&freelist_frames, &fe, NULL)) {
        freelist_frames_data = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

        free(freelist_frames_data->data);
    }

    unsigned int nc = (new_height + 1) * frame_data_count + sizeof(unsigned int);

    for (int i = 0; i < fas_frames_queue_size; i += 1) {
        ffd[i].data = malloc(sizeof(struct note) * nc);

        LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(ffd[i].fe, &ffd[i]);
        lfds711_freelist_push(&freelist_frames, &ffd[i].fe, NULL);
    }
}

void oscSend(struct oscillator *oscillators, struct note *data) {
    if (!fas_osc_out || !oscillators) {
        return;
    }

    struct note *_notes;
    unsigned int note_buffer_len = 0, pv_note_buffer_len = 0;
    unsigned int i, j, k, s, e;
/*
    lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
    lo_message msg = lo_message_new();
*/
    for (k = 0; k < frame_data_count; k += 1) {
        pv_note_buffer_len += note_buffer_len;
        note_buffer_len = data[pv_note_buffer_len].osc_index;
        pv_note_buffer_len += 1;
        s = pv_note_buffer_len;
        e = s + note_buffer_len;

        for (j = s; j < e; j += 1) {
            struct note *n = &data[j];

            struct oscillator *osc = &oscillators[n->osc_index];
/*
            lo_message_add_int32(msg, n->osc_index);
            lo_message_add_double(msg, osc->freq);
            lo_message_add_float(msg, n->volume_l);
            lo_message_add_float(msg, n->volume_r);
            lo_message_add_float(msg, n->alpha);
*/
            //if (n->volume_l > 0 && n->volume_r > 0) {
                lo_send(fas_lo_addr, "/fragment", "idfff", n->osc_index, osc->freq, n->volume_l, n->volume_r, n->alpha);
            //}
        }
    }

    //lo_send_bundle(fas_lo_addr, bundle);
}

void freeSynth(struct _synth **s) {
    struct _synth *synth = *s;
    if (synth) {
        if (synth->oscillators && synth->settings) {
            synth->oscillators = freeOscillators(&synth->oscillators, synth->settings->h, frame_data_count);
        }

        if (synth->grains) {
            freeGrains(&synth->grains, samples_count, frame_data_count, synth->settings->h, fas_granular_max_density);
        }

        free(synth->chn_settings);
        free(synth->settings);
        free(synth->gain);
        free(synth);
    }
}

void clearQueues() {
    void *key;
    struct _freelist_frames_data *freelist_frames_data;
    while (lfds711_ringbuffer_read(&rs, &key, NULL)) {
        freelist_frames_data = (struct _freelist_frames_data *)key;

        LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(freelist_frames_data->fe, freelist_frames_data);
        lfds711_freelist_push(&freelist_frames, &freelist_frames_data->fe, NULL);
    }

    void *queue_synth_void;
    while(lfds711_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void)) {
        struct _synth *queue_synth = (struct _synth *)queue_synth_void;

        freeSynth(&queue_synth);
    }
}

int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len) {
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
    fflush(stdout);
#endif

                if (pid == SYNTH_SETTINGS) {
                    audioPause();

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
                        usd->synth->grains = NULL;
                        usd->synth->chn_settings = NULL;
                        usd->synth->chn = 0;

                        usd->synth->gain = NULL;
                    } else {
                        h = usd->synth->settings->h;
                    }

                    usd->synth->chn = 0;

                    clearQueues();

                    memcpy(usd->synth->settings, &((char *) usd->packet)[PACKET_HEADER_LENGTH], 24);

                    #ifdef DEBUG
                        printf("SYNTH_SETTINGS : %u, %u, %u, %f\n", usd->synth->settings->h,
                            usd->synth->settings->octave, usd->synth->settings->data_type, usd->synth->settings->base_frequency);
                    #endif

                    freeGrains(&usd->synth->grains, prev_samples_count, frame_data_count, usd->synth_h, fas_granular_max_density);

                    usd->synth->oscillators = freeOscillators(&usd->synth->oscillators, usd->synth_h, frame_data_count);

                    free(usd->synth->gain);

                    usd->synth->gain = NULL;

                    usd->frame_data_size = sizeof(unsigned char);
                    unsigned int frame_data_size = sizeof(unsigned char);
                    if (usd->synth->settings->data_type) {
                        usd->frame_data_size = sizeof(float);
                    }

                    usd->expected_frame_length = 4 * usd->frame_data_size * usd->synth->settings->h;
                    usd->expected_max_frame_length = 4 * usd->frame_data_size * usd->synth->settings->h * frame_data_count;
                    size_t max_frame_data_len = usd->expected_frame_length * frame_data_count + sizeof(unsigned int) * 2;

                    free(usd->frame_data);
                    free(usd->prev_frame_data);

                    usd->frame_data = NULL;
                    usd->prev_frame_data = NULL;

                    usd->synth->samples_count = samples_count;

                    usd->frame_data = calloc(max_frame_data_len, usd->frame_data_size);
                    usd->prev_frame_data = calloc(max_frame_data_len, usd->frame_data_size);

                    if (usd->oscillators) {
                        usd->oscillators = freeOscillators(&usd->oscillators, usd->synth_h, frame_data_count);
                    }
                    //usd->oscillators = (struct oscillator*)malloc(n * sizeof(struct oscillator));

                    usd->synth_h = usd->synth->settings->h;

                    setHeight(usd->synth_h);

                    // create a global copy of the oscillators for the user (for OSC)
                    if (fas_osc_out) {
                        usd->oscillators = createOscillators(usd->synth->settings->h,
                            usd->synth->settings->base_frequency, usd->synth->settings->octave, fas_sample_rate, fas_wavetable_size, frame_data_count);
                    }

                    usd->synth->oscillators = createOscillators(usd->synth->settings->h,
                        usd->synth->settings->base_frequency, usd->synth->settings->octave, fas_sample_rate, fas_wavetable_size, frame_data_count);

                    usd->synth->grains = createGrains(&samples, samples_count, usd->synth_h, usd->synth->settings->base_frequency, usd->synth->settings->octave, fas_sample_rate, frame_data_count, fas_granular_max_density);

                    free(usd->synth->chn_settings);
                    usd->synth->chn_settings = NULL;
/*
                    usd->synth->chn_settings = (struct _synth_chn_settings*)malloc(sizeof(struct _synth_chn_settings) * frame_data_count);
                    if (usd->synth->chn_settings == NULL) {
                        printf("chn_settings alloc. error.");
                        fflush(stdout);
                    }
                    memset(usd->synth->chn_settings, 0, sizeof(struct _synth_chn_settings) * frame_data_count);
*/
                    if (lfds711_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
                        printf("Skipping packet, the synth commands queue is full.\n");
                        fflush(stdout);

                        //free(usd->synth->chn_settings);
                        free(usd->synth->settings);
                        usd->synth->oscillators = freeOscillators(&usd->synth->oscillators, usd->synth->settings->h, frame_data_count);

                        freeGrains(&usd->synth->grains, samples_count, frame_data_count, usd->synth->settings->h, fas_granular_max_density);

                        free(usd->synth);
                        usd->synth = NULL;

                        goto free_packet;
                    }

                    usd->synth = NULL;

                    audioPlay();
                } else if (pid == FRAME_DATA) {
#ifdef DEBUG
    printf("FRAME_DATA\n");

    static unsigned int frame_data_length[1];
    memcpy(&frame_data_length, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(frame_data_length));
    printf("Number of channels in the frame: %u\n", *frame_data_length);
#endif

                    // drop frame packets when the audio thread is busy at doing something else than playing audio
                    if (audio_thread_state != FAS_AUDIO_PLAY) {
                        goto free_packet;
                    }

                    //size_t frame_length = usd->packet_len - PACKET_HEADER_LENGTH - FRAME_HEADER_LENGTH;
                    //if (frame_length != usd->expected_frame_length) {
                    //    printf("Skipping a frame, the frame length %zu does not match the expected frame length %zu.\n", frame_length, usd->expected_frame_length);
                    //    fflush(stdout);
                    //    goto free_packet;
                    //}

                    if (usd->frame_data == NULL) {
                        printf("Skipping a frame until a synth. settings change happen.\n");
                        fflush(stdout);
                        goto free_packet;
                    }

#ifdef DEBUG
    lfds711_pal_uint_t frames_data_freelist_count;
    lfds711_freelist_query(&freelist_frames, LFDS711_FREELIST_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *)&frames_data_freelist_count);
    printf("frames_data_freelist_count : %llu\n", frames_data_freelist_count);
#endif

                    struct lfds711_freelist_element *fe;
                    struct _freelist_frames_data *freelist_frames_data;
                    int pop_result = lfds711_freelist_pop(&freelist_frames, &fe, NULL);
                    if (pop_result == 0) {
#ifdef DEBUG
                        printf("Skipping a frame, notes buffer freelist is empty.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    memcpy(usd->prev_frame_data, &usd->frame_data[0], usd->expected_max_frame_length);

                    unsigned int channels[1];
                    memcpy(&channels, &usd->packet[PACKET_HEADER_LENGTH], sizeof(channels));

                    if ((*channels) > frame_data_count) {
#ifdef DEBUG
                        printf("Frame channels > Output channels. (%i chn ignored)\n", (*channels) - frame_data_count);
                        fflush(stdout);
#endif

                        (*channels) = frame_data_count;

                        memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->expected_frame_length * (*channels) - PACKET_HEADER_LENGTH);
                    } else if ((*channels) < frame_data_count) {
                        memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->packet_len - PACKET_HEADER_LENGTH);
                        memset(&usd->frame_data[usd->expected_frame_length * (*channels)], 0, usd->expected_frame_length * (frame_data_count - (*channels)));
#ifdef DEBUG
                        printf("Frame channels < Output channels.\n");
                        fflush(stdout);
#endif
                    } else {
                        memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->packet_len - PACKET_HEADER_LENGTH);
                    }

                    freelist_frames_data = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);

                    memset(freelist_frames_data->data, 0, sizeof(struct note) * (usd->synth_h + 1) * frame_data_count + sizeof(unsigned int));
/*
                    if (freelist_frames_data->data[0].mag == NULL) {
                        freelist_frames_data->data[0].mag = (float *)malloc(sizeof(float) * hop_size);
                        freelist_frames_data->data[0].pha = (float *)malloc(sizeof(float) * hop_size);
                        freelist_frames_data->data[0].frq = (float *)malloc(sizeof(float) * hop_size);

                        memset(freelist_frames_data->data[0].mag, 0, sizeof(float) * hop_size);
                        memset(freelist_frames_data->data[0].pha, 0, sizeof(float) * hop_size);
                        memset(freelist_frames_data->data[0].frq, 0, sizeof(float) * hop_size);
                    }
*/
                    fillNotesBuffer(samples_count_m1, fas_granular_max_density, (*channels), usd->frame_data_size, freelist_frames_data->data, usd->synth_h, &usd->synth->oscillators, usd->expected_frame_length, usd->prev_frame_data, usd->frame_data);

                    oscSend(usd->oscillators, freelist_frames_data->data);

                    struct _freelist_frames_data *overwritten_notes = NULL;
                    lfds711_ringbuffer_write(&rs, (void *) (lfds711_pal_uint_t) freelist_frames_data, NULL, &overwrite_occurred_flag, (void *)&overwritten_notes, NULL);
                    if (overwrite_occurred_flag == LFDS711_MISC_FLAG_RAISED) {
                        // okay, push it back!
                        LFDS711_FREELIST_SET_VALUE_IN_ELEMENT(overwritten_notes->fe, overwritten_notes);
                        lfds711_freelist_push(&freelist_frames, &overwritten_notes->fe, NULL);
                    }

                    // check & send stream load
                    time_t stream_load_end;
                    time(&stream_load_end);
                    double stream_load_time = difftime(stream_load_end,stream_load_begin);
                    if (stream_load_time > fas_stream_load_send_delay) {
                        static unsigned char p_load[LWS_SEND_BUFFER_PRE_PADDING + sizeof(double) + LWS_SEND_BUFFER_POST_PADDING];
                        p_load[LWS_SEND_BUFFER_PRE_PADDING] = 0;
                        double stream_load = Pa_GetStreamCpuLoad(stream);
                        memcpy(&p_load[LWS_SEND_BUFFER_PRE_PADDING], &stream_load, sizeof(double));
                        lws_write(wsi, &p_load[LWS_SEND_BUFFER_PRE_PADDING], sizeof(double), LWS_WRITE_BINARY);

                        time(&stream_load_begin);
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
                    usd->synth->grains = NULL;
                    usd->synth->chn_settings = NULL;
                    usd->synth->chn = 0;

                    usd->synth->samples_count = samples_count;

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
                } else if (pid == CHN_SETTINGS) {
                    static unsigned int channels_count[1];
                    memcpy(&channels_count, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(channels_count));

                    if (usd->synth == NULL) {
                        usd->synth = (struct _synth*)malloc(sizeof(struct _synth));
                        if (usd->synth == NULL) {
                            printf("Skipping packet due to synth data alloc. error.\n");
                            fflush(stdout);

                            goto free_packet;
                        }

                        usd->synth->chn_settings = (struct _synth_chn_settings*)malloc(sizeof(struct _synth_chn_settings) * (*channels_count));
                        if (usd->synth->chn_settings == NULL) {
                            printf("Skipping packet due to synth chn_settings alloc. error.\n");
                            fflush(stdout);

                            free(usd->synth);
                            usd->synth = NULL;

                            goto free_packet;
                        }
                    }
#ifdef DEBUG
printf("CHN_SETTINGS : chn count %i\n", *channels_count);
fflush(stdout);
#endif

                    int *data = (int *)usd->packet;
                    double *data_double = (double *)&usd->packet[PACKET_HEADER_LENGTH + 16];
                    int i = 0, i2 = 0;
                    for (n = 0; n < (*channels_count); n += 1) {
                        usd->synth->chn_settings[n].synthesis_method = data[4 + i];
                        usd->synth->chn_settings[n].env_type = data[4 + i + 1];
                        usd->synth->chn_settings[n].p1 = data_double[i2];
                        usd->synth->chn_settings[n].p2 = data_double[i2 + 1];
                        usd->synth->chn_settings[n].p3 = data_double[i2 + 2];

#ifdef DEBUG
printf("chn %i data : %i, %i, %f, %f, %f\n", n, usd->synth->chn_settings[n].synthesis_method, usd->synth->chn_settings[n].env_type, usd->synth->chn_settings[n].p1, usd->synth->chn_settings[n].p2, usd->synth->chn_settings[n].p3);
fflush(stdout);
#endif

                        i += 8;
                        i2 += 3;
                    }

                    usd->synth->oscillators = NULL;
                    usd->synth->settings = NULL;
                    usd->synth->gain = NULL;
                    usd->synth->grains = NULL;
                    usd->synth->chn = 0;

                    usd->synth->samples_count = samples_count;

                    if (lfds711_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
                        printf("Skipping packet, the synth commands queue is full.\n");
                        fflush(stdout);

                        free(usd->synth->chn_settings);
                        free(usd->synth);
                        usd->synth = NULL;

                        goto free_packet;
                    }

                    usd->synth = NULL;
                } else if (pid == ACTION) {
                    static unsigned char action_type[1];
                    memcpy(&action_type, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(action_type));

#ifdef DEBUG
printf("ACTION : type %i\n", action_type[0]);
fflush(stdout);
#endif

                    if (action_type[0] == 0) { // RELOAD SAMPLES
                        audioPause();

                        clearQueues();

                        prev_samples_count = samples_count;

                        free_samples(&samples, samples_count);
                        samples_count = load_samples(&samples, fas_grains_path, fas_sample_rate, fas_samplerate_converter_type);
                        samples_count_m1 = samples_count - 1;

                        audioPlay();
                    } else if (action_type[0] == 1) { // RE-TRIGGER note
                      unsigned int *data_uint = (unsigned int *)&usd->packet[PACKET_HEADER_LENGTH];
#ifdef DEBUG
printf("re-trigger chn : %i, note : %i\n", data_uint[0], data_uint[1]);
fflush(stdout);
#endif
                        if (usd->synth == NULL) {
                            usd->synth = (struct _synth*)malloc(sizeof(struct _synth));
                            if (usd->synth == NULL) {
                                printf("Skipping packet due to synth data alloc. error.\n");
                                fflush(stdout);

                                goto free_packet;
                            }
                        }

                        usd->synth->oscillators = NULL;
                        usd->synth->settings = NULL;
                        usd->synth->gain = NULL;
                        usd->synth->grains = NULL;
                        usd->synth->chn_settings = NULL;

                        usd->synth->samples_count = samples_count;

                        usd->synth->chn = data_uint[0];
                        usd->synth->note = data_uint[1];

                        if (lfds711_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)usd->synth) == 0) {
                            printf("Skipping packet, the synth commands queue is full.\n");
                            fflush(stdout);

                            free(usd->synth);
                            usd->synth = NULL;
                        }

                        usd->synth = NULL;
                    }
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
        case LWS_CALLBACK_CLOSED:
            if (reason != LWS_CALLBACK_CLOSED) {
                audioPause();
            }

            clearQueues();

            freeSynth(&usd->synth);

            if (usd->oscillators) {
                usd->oscillators = freeOscillators(&usd->oscillators, usd->synth_h, frame_data_count);
            }

            free(usd->prev_frame_data);
            free(usd->frame_data);

            usd->prev_frame_data = NULL;
            usd->frame_data = NULL;

            printf("Connection from %s (%s) closed.\n", usd->peer_name, usd->peer_ip);
            fflush(stdout);

            audioPlay();
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

int main(int argc, char **argv)
{
    int print_infos = 0;

    int i = 0;

    static struct option long_options[] = {
        { "sample_rate",                required_argument, 0, 0 },
        { "frames",                     required_argument, 0, 1 },
        { "wavetable",                  required_argument, 0, 2 },
#ifndef FIXED_WAVETABLE
        { "wavetable_size",             required_argument, 0, 3 },
#endif
        { "fps",                        required_argument, 0, 4 },
        { "deflate",                    required_argument, 0, 5 },
        { "rx_buffer_size",             required_argument, 0, 6 },
        { "port",                       required_argument, 0, 7 },
        { "alsa_realtime_scheduling",   required_argument, 0, 8 },
        { "frames_queue_size",          required_argument, 0, 9 },
        { "commands_queue_size",        required_argument, 0, 10 },
        { "ssl",                        required_argument, 0, 11 },
        { "iface",                      required_argument, 0, 12 },
        { "device",                     required_argument, 0, 13 },
        { "output_channels",            required_argument, 0, 14 },
        { "i",                                no_argument, 0, 15 },
        { "noise_amount",               required_argument, 0, 16 },
        { "osc_out",                    required_argument, 0, 17 },
        { "osc_addr",                   required_argument, 0, 18 },
        { "osc_port",                   required_argument, 0, 19 },
        { "grains_folder",              required_argument, 0, 20 },
        { "smooth_factor",              required_argument, 0, 21 },
        { "granular_max_density",       required_argument, 0, 22 },
        { "stream_load_send_delay",     required_argument, 0, 23 },
        { "max_drop",                   required_argument, 0, 24 },
        { "samplerate_conv_type",       required_argument, 0, 25 },
        //{ "render",                   required_argument, 0, 26 },
        //{ "render_convert",           required_argument, 0, 27 },
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
            case 17:
                fas_osc_out = strtoul(optarg, NULL, 0);
                break;
            case 18:
                fas_osc_addr = optarg;
                break;
            case 19:
                fas_osc_port = optarg;
                break;
            case 20:
                fas_grains_path = optarg;
                break;
            case 21:
                fas_smooth_factor = strtod(optarg, NULL);
              break;
            case 22:
                fas_granular_max_density = strtoul(optarg, NULL, 0);
              break;
            case 23:
                fas_stream_load_send_delay = strtoul(optarg, NULL, 0);
              break;
            case 24:
              fas_max_drop = strtoul(optarg, NULL, 0);
              break;
            case 25:
              fas_samplerate_converter_type = strtol(optarg, NULL, 0);
              break;
            /*case 26:
              fas_render_target = optarg;
              break;
            case 27:
              fas_render_convert = optarg;
              break;*/
            default: print_usage();
                 return EXIT_FAILURE;
        }
    }

    if (fas_grains_path == NULL) {
#ifdef __unix__
        struct stat s;
        int err = stat("/usr/local/share/fragment/grains/", &s);
        if (err == -1) {
            if (ENOENT == errno) {
                fas_grains_path = fas_default_grains_path;
            } else {
                printf("stat() error while checking for '/usr/local/share/fragment/grains/' directory.\n");
                return EXIT_FAILURE;
            }
        } else {
            if (S_ISDIR(s.st_mode)) {
                fas_grains_path = fas_install_default_grains_path;
                printf("'/usr/local/share/fragment/grains/' directory detected, default grains folder.\n");
            } else {
                printf("'/usr/local/share/fragment/grains/' is not a directory, defaulting to non-install grains path.\n");
                fas_grains_path = fas_default_grains_path;
            }
        }
#else
        fas_grains_path = fas_default_grains_path;
#endif
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

    if (fas_smooth_factor < 1.) {
        printf("Warning: smooth_factor program option argument is invalid, should be >= 1, the default value (%f) will be used.\n", FAS_SMOOTH_FACTOR);

        fas_noise_amount = FAS_SMOOTH_FACTOR;
    }

    if (fas_granular_max_density < 1.) {
        printf("Warning: granular_max_density program option argument is invalid, should be >= 1, the default value (%i) will be used.\n", FAS_GRANULAR_MAX_DENSITY);

        fas_noise_amount = FAS_GRANULAR_MAX_DENSITY;
    }

    if (fas_stream_load_send_delay < 1.) {
        printf("Warning: stream_load_send_delay program option argument is invalid, should be >= 1, the default value (%i) will be used.\n", FAS_STREAM_LOAD_SEND_DELAY);

        fas_stream_load_send_delay = FAS_STREAM_LOAD_SEND_DELAY;
    }

    if (fas_max_drop <= 0) {
        printf("Warning: max_drop program option argument is invalid, should be >= 0, the default value (%i) will be used.\n", FAS_MAX_DROP);

        fas_max_drop = FAS_MAX_DROP;
    }

    if (errno == ERANGE) {
        printf("Warning: One of the specified program option is out of range and was set to its maximal value.\n");
    }

    if (fas_samplerate_converter_type > 0) {
        const char *samplerate_converter_type = src_get_name(fas_samplerate_converter_type);
        if (samplerate_converter_type == NULL) {
            printf("Invalid samplerate conversion method, defaulting to SRC_SINC_MEDIUM_QUALITY\n");
            fflush(stdout);

            fas_samplerate_converter_type = SRC_SINC_MEDIUM_QUALITY;
        } else {
            printf("Samplerate conversion method : %s\n", samplerate_converter_type);
            fflush(stdout);
        }
    }

    if (print_infos != 1) {
        time(&stream_load_begin);

        samples_count = load_samples(&samples, fas_grains_path, fas_sample_rate, fas_samplerate_converter_type);
        samples_count_m1 = samples_count - 1;

        // fas setup
        note_time = 1 / (double)fas_fps;
        note_time_samples = round(note_time * fas_sample_rate);
        lerp_t_step = 1 / note_time_samples;

        if (fas_wavetable) {
            fas_sine_wavetable = sine_wavetable_init(fas_wavetable_size);
            if (fas_sine_wavetable == NULL) {
                fprintf(stderr, "sine_wavetable_init() failed.\n");

                return EXIT_FAILURE;
            }

            fas_white_noise_table = wnoise_wavetable_init(65536, 1.0);
            if (fas_white_noise_table == NULL) {
                fprintf(stderr, "wnoise_wavetable_init() failed.\n");
                free(fas_sine_wavetable);

                return EXIT_FAILURE;
            }
        }

        grain_envelope = createEnvelopes(FAS_ENVS_SIZE);

        // osc - http://liblo.sourceforge.net
        if (fas_osc_out) {
            fas_lo_addr = lo_address_new(fas_osc_addr, fas_osc_port);

            if (fas_lo_addr) {
                printf("\nOSC: Ready to send data to '%s:%s'\n", fas_osc_addr, fas_osc_port);
            } else {
                fas_osc_out = 0;
            }
        }
    }

#ifdef WITH_ESSENTIA
    cessentia = newCEssentia();
    initializeSineModelCEssentia(cessentia, fas_sample_rate, window_size, hop_size);
#endif

    curr_synth.oscillators = NULL;
    curr_synth.gain = NULL;
    curr_synth.grains = NULL;
    curr_synth.settings = NULL;
    curr_synth.chn_settings = NULL;
    curr_synth.samples_count = 0;
    curr_synth.lerp_t = 0;

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

    last_sample_l = calloc(frame_data_count, sizeof(float));
    last_sample_r = calloc(frame_data_count, sizeof(float));

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
              paDitherOff,
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

    // free callback synth copy
    if (curr_synth.oscillators) {
        freeOscillators(&curr_synth.oscillators, curr_synth.settings->h, frame_data_count);
    }

    free(curr_synth.gain);

    if (curr_synth.grains) {
        freeGrains(&curr_synth.grains, curr_synth.samples_count, frame_data_count, curr_synth.settings->h, fas_granular_max_density);
    }

    free(curr_synth.settings);
    free(curr_synth.chn_settings);
    //

    free(fas_sine_wavetable);
    free(fas_white_noise_table);

    freeEnvelopes(grain_envelope);
    free_samples(&samples, samples_count);

    #ifdef WITH_ESSENTIA
        freeSineModelCEssentia(cessentia);
        delCEssentia(cessentia);
    #endif

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

    free(last_sample_l);
    free(last_sample_r);

    printf("Bye.\n");

    return err;

ws_error:
    fprintf(stderr, "lws related error occured.\n");

    lws_context_destroy(context);

error:
    if (fas_osc_out) {
        lo_address_free(fas_lo_addr);
    }

    Pa_Terminate();

    if (err != paNoError) {
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    }

#ifdef WITH_ESSENTIA
    freeSineModelCEssentia(cessentia);
    delCEssentia(cessentia);
#endif

    freeEnvelopes(grain_envelope);

    free(fas_sine_wavetable);
    free(fas_white_noise_table);

    free_samples(&samples, samples_count);

    free(last_sample_l);
    free(last_sample_r);

    return err;
}
