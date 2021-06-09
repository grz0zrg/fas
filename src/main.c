/*
    Copyright (c) 2017-2022, Julien Verneuil
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
    Oscillator-bank / filter-bank synthesizer built for the Fragment Synthesizer, a web-based image-synth collaborative audio/visual synthesizer.

    This collect Fragment settings and notes data over WebSocket, convert them to a suitable data structure and generate sounds in real-time.

    Only one client is supported.

    You can tweak this program : fas --h

    Can be used as a generic synthesizer if you feed it correctly!

    https://www.fsynth.com

    Author : Julien Verneuil
    License : Simplified BSD License
*/

#include "fas.h"

/**
 * Synth. commands from the network thread are smoothly processed here; it pass incoming data to the audio thread without allocations thanks to lock-free data structures.
 **/
void doSynthCommands() {
    void *queue_synth_void;
    while (lfds720_queue_bss_dequeue(&synth_commands_queue_state, NULL, &queue_synth_void) == 1) {
        struct _freelist_synth_commands *freelist_synth_command = (struct _freelist_synth_commands *)queue_synth_void;

        struct _synth_command *synth_command = freelist_synth_command->data;

        if (synth_command->type == FAS_CMD_SYNTH_SETTINGS) {
            uint32_t target = synth_command->value[0];
            FAS_FLOAT value = synth_command->value[1];

#ifdef DEBUG
    printf("CMD SYNTH_SETTINGS : target %i value %f\n", target, value);
    fflush(stdout);
#endif
            
            if (target == 0 && value > 0) {
                fpsChange(value);
            } else if (target == 1) {
                curr_synth.settings->gain_lr = value;
            }
        } else if (synth_command->type == FAS_CMD_CHN_SETTINGS) {
            uint32_t chn = synth_command->value[0];
            uint32_t target = synth_command->value[1];
            FAS_FLOAT value = synth_command->value[2];

#ifdef DEBUG
    printf("CMD CHN_SETTINGS : chn %i target %i value %f\n", chn, target, value);
    fflush(stdout);
#endif

            if (chn < fas_max_channels) {
                struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[chn];
                if (target == 0) {
                    chn_settings->muted = value;
                } else if (target == 1) {
                    if (value < frame_data_count) {
                        chn_settings->output_chn = value;
                    } else {
#ifdef DEBUG
    printf("CMD CHN_SETTINGS : chn output_chn >= device output channels\n");
    fflush(stdout);
#endif
                    }
                } else {
#ifdef DEBUG
    printf("CMD CHN_SETTINGS : chn targed does not exist\n");
    fflush(stdout);
#endif
                }
            } else {
#ifdef DEBUG
    printf("CMD CHN_SETTINGS : chn index does not exist\n");
    fflush(stdout);
#endif
            }
        } else if (synth_command->type == FAS_CMD_INSTRUMENT_SETTINGS) {
            uint32_t instrument = synth_command->value[0];
            uint32_t target = synth_command->value[1];
            FAS_FLOAT value = synth_command->value[2];
#ifdef DEBUG
    printf("CMD INSTRUMENT_SETTINGS : instrument %i target %i value %f\n", instrument, target, value);
    fflush(stdout);
#endif
            if (instrument < fas_max_instruments) {
                struct _synth_instrument *instrument_settings = &curr_synth.instruments[instrument];
                if (target == 0) {
                    // note : instrument is smoothly switched on note event basis (not directly here but later on)
                    if (value == FAS_GRANULAR && samples_count == 0) {
                        // do not allow synthesis based on samples when there is no samples
                        instrument_settings->next_type = FAS_VOID;
                    } else if (value == FAS_WAVETABLE_SYNTH && waves_count == 0) {
                        // do not allow synthesis based on waves when there is no waves
                        instrument_settings->next_type = FAS_VOID;
                    } else if (value == FAS_INPUT && fas_input_channels == 0) {
                        // do not allow input mode when there is no inputs
                        instrument_settings->next_type = FAS_VOID;
                    } else {
                        instrument_settings->next_type = value;
                    }

                    // start off a clean state
                    clearNotesQueue();
                } else if (target == 1) {
                    instrument_settings->muted = value;
                } else if (target == 2) {
                    instrument_settings->output_channel = value;
                } else if (target == 3) {
                    if (instrument_settings->next_type != instrument_settings->type) {
                        // note : parameters may be refreshed during instrument switch, so should be only updated when the type really switch later on
                        instrument_settings->next_p0 = value;
                    } else {
                        instrument_settings->p0 = value;
                        instrument_settings->next_p0 = value;
                    }
                } else if (target == 4) {
                    if (instrument_settings->next_type != instrument_settings->type) {
                        // note : parameters may be refreshed during instrument switch, so should be only updated when the type really switch later on
                        instrument_settings->next_p1 = value;
                    } else {
                        instrument_settings->p1 = value;
                        instrument_settings->next_p1;
                    }
                } else if (target == 5) {
                    if (instrument_settings->next_type != instrument_settings->type) {
                        // note : parameters may be refreshed during instrument switch, so should be only updated when the type really switch later on
                        instrument_settings->next_p2 = value;
                    } else {
                        instrument_settings->p2 = value;
                        instrument_settings->next_p2;
                    }
                } else if (target == 6) {
                    if (instrument_settings->next_type != instrument_settings->type) {
                        // note : parameters may be refreshed during instrument switch, so should be only updated when the type really switch later on
                        instrument_settings->next_p3 = value;
                    } else {
                        instrument_settings->p3 = value;
                        instrument_settings->next_p3;
                    }
                } else if (target == 7) {
                    if (instrument_settings->next_type != instrument_settings->type) {
                        // note : parameters may be refreshed during instrument switch, so should be only updated when the type really switch later on
                        instrument_settings->next_p4 = value;
                    } else {
                        instrument_settings->p4 = value;
                        instrument_settings->next_p4;
                    }
                }
            } else {
#ifdef DEBUG
    printf("CMD CHN_INSTRUMENT_SETTINGS : instrument index does not exist\n");
    fflush(stdout);
#endif
            }
        } else if (synth_command->type == FAS_CMD_NOTE_RESET) {
            unsigned int instrument_index = synth_command->value[0];
            unsigned int osc_index = synth_command->value[1];

#ifdef DEBUG
    printf("CMD NOTE_RESET : instrument %i osc bank index %i \n", instrument_index, osc_index);
    fflush(stdout);
#endif

            struct oscillator *osc = &curr_synth.oscillators[osc_index];
            osc->triggered[instrument_index] = 1;
        } else if (synth_command->type == FAS_CMD_CHN_FX_SETTINGS) {
            uint32_t chn = synth_command->value[0];
            uint32_t slot = synth_command->value[1];
            uint32_t target = synth_command->value[2];
            FAS_FLOAT value = synth_command->value[3];

#ifdef DEBUG
    printf("CMD CHN_FX_SETTINGS : chn %i slot %i target %i value %f\n", chn, slot, target, value);
    fflush(stdout);
#endif
            if (chn < fas_max_channels) {
                if (slot < FAS_MAX_FX_SLOTS) {
                    struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[chn];
                    struct _synth_fx_settings *fx_settings = &chn_settings->fx[slot];

                    if (target == 0) {
                        fx_settings->fx_id = value;

                        if (value < 0) { // this slot has been deleted; we need to shift everything after down to this slot (effect slots are handled linearly)
                            unsigned int i = 0;
                            for (i = slot; i < FAS_MAX_FX_SLOTS - 1; i += 1) {
                                struct _synth_fx_settings *fx_settings1 = &chn_settings->fx[i];
                                struct _synth_fx_settings *fx_settings2 = &chn_settings->fx[i + 1];

                                if (fx_settings2->fx_id == -1) {
                                    break;
                                }

                                fx_settings1->bypass = fx_settings2->bypass;
                                fx_settings1->fx_id = fx_settings2->fx_id;

                                unsigned int j = 0;
                                for (j = 0; j < FAS_MAX_FX_PARAMETERS; j += 1) {
                                    fx_settings1->fp[j] = fx_settings2->fp[j];

                                    updateEffectParameter(
#ifdef WITH_SOUNDPIPE
                                        sp,
#endif                    
                                        synth_fx[chn], chn_settings, i, j + 2, fx_settings1->fp[j]);
                                }
                            }

                            struct _synth_fx_settings *fx_settings_tail = &chn_settings->fx[i];
                            fx_settings_tail->fx_id = -1;
                        }
                    } else if (target == 1) {
                        fx_settings->bypass = value;
                    } else if (target >= 2) { // effect parameters
                        uint32_t fp_index = target - 2;
                        if (fp_index < FAS_MAX_FX_PARAMETERS) {
                            fx_settings->fp[fp_index] = value;

                            updateEffectParameter(
#ifdef WITH_SOUNDPIPE
                                sp,
#endif                    
                                synth_fx[chn], chn_settings, slot, target, value);
                        } else {
#ifdef DEBUG
    printf("CMD CHN_SETTINGS : fp index does not exist \n");
    fflush(stdout);
#endif  
                        }
                    }
                } else {
#ifdef DEBUG
    printf("CMD CHN_SETTINGS : fx slot does not exist \n");
    fflush(stdout);
#endif   
                }
            } else {
#ifdef DEBUG
    printf("CMD CHN_SETTINGS : chn does not exist \n");
    fflush(stdout);
#endif
            }
        }

        // once done push it back into the pool
        LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(freelist_synth_command->fe, freelist_synth_command);
        lfds720_freelist_n_threadsafe_push(&freelist_commands, NULL, &freelist_synth_command->fe);
    }
}

#ifdef INTERLEAVED_SAMPLE_FORMAT
static int audioCallback(float *inputBuffer, float *outputBuffer, unsigned long nframes) {
#else
static int audioCallback(float **inputBuffer, float **outputBuffer, unsigned long nframes) {
#endif
    LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE;

#ifdef INTERLEAVED_SAMPLE_FORMAT
    float *audio_out = outputBuffer;
    float *audio_in = inputBuffer;
#endif
    unsigned int i, j, k, d, s, e, w;

    struct _freelist_frames_data *freelist_frames_data;

    doSynthCommands();

    int read_status = 0;
    void *key;

    // audio callback commands
    if (audio_thread_state == FAS_AUDIO_DO_PAUSE) {
        last_gain_lr = curr_synth.settings->gain_lr;

        curr_synth.lerp_t = 0.0;

        audio_thread_state = FAS_AUDIO_PAUSE;
    } else if (audio_thread_state == FAS_AUDIO_DO_PLAY) {
        curr_synth.lerp_t = 0.0;
        curr_synth.curr_sample = 0;
        lerp_t_step = 1 / note_time_samples;

        // start off a clean state
        clearNotesQueue();

        trigger_note_on = 1;

        audio_thread_state = FAS_AUDIO_PLAY;
    } else if (audio_thread_state == FAS_AUDIO_DO_FLUSH_THEN_PAUSE) {
        // flush away callback data
        if (curr_notes != dummy_notes) {
            LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
            lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &curr_freelist_frames_data->fe);
        }

        curr_notes = dummy_notes;

        last_gain_lr = curr_synth.settings->gain_lr;

        curr_synth.lerp_t = 0.0;

        audio_thread_state = FAS_AUDIO_PAUSE;
    }

    if (audio_thread_state == FAS_AUDIO_PAUSE) {
        // paused audio
        for (i = 0; i < nframes; i += 1) {
#ifdef INTERLEAVED_SAMPLE_FORMAT
            for (j = 0; j < fas_max_channels; j += 1) {
                struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[j];

                if (chn_settings->output_chn >= 0) {
                    audio_out[i * 2 * frame_data_count + chn_settings->output_chn * 2] += chn_settings->last_sample_l * (1.0f - curr_synth.lerp_t) * last_gain_lr;
                    audio_out[i * 2 * frame_data_count + 1 + chn_settings->output_chn * 2] += chn_settings->last_sample_r * (1.0f - curr_synth.lerp_t) * last_gain_lr;
                }
            }
#else
            for (j = 0; j < fas_max_channels; j += 1) {
                struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[j];

                if (chn_settings->output_chn >= 0) {
                    int output_chn = chn_settings->output_chn * 2;
                    outputBuffer[output_chn][i] += chn_settings->last_sample_l * (1.0f - curr_synth.lerp_t) * last_gain_lr;
                    outputBuffer[output_chn + 1][i] += chn_settings->last_sample_r * (1.0f - curr_synth.lerp_t) * last_gain_lr;
                }
            }
#endif
            curr_synth.lerp_t += (1.0f / (FAS_FLOAT)nframes);
            curr_synth.lerp_t = fmin(curr_synth.lerp_t, 1.0f);
        }

        return 0;
    }

    // synth core - critical part :)
    struct note *_notes;
    unsigned int note_buffer_len = 0, pv_note_buffer_len = 0;

    for (i = 0; i < nframes; i += 1) {
        note_buffer_len = 0;
        pv_note_buffer_len = 0;

        for (k = 0; k < fas_max_instruments; k += 1) {
            pv_note_buffer_len += note_buffer_len;
            note_buffer_len = curr_notes[pv_note_buffer_len].osc_index;
            pv_note_buffer_len += 1;
            s = pv_note_buffer_len;
            e = s + note_buffer_len;

            struct _synth_instrument *instrument = &curr_synth.instruments[k];
            int synthesis_method = instrument->type;

            struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[instrument->output_channel];

            FAS_FLOAT output_l = 0;
            FAS_FLOAT output_r = 0;

            if (synthesis_method == FAS_ADDITIVE) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef MAGIC_CIRCLE
                    osc->mc_x[k] = osc->mc_x[k] + osc->mc_eps * osc->mc_y[k];
                    osc->mc_y[k] = -osc->mc_eps * osc->mc_x[k] + osc->mc_y[k];

                    FAS_FLOAT smp = osc->mc_y[k];
#else
                    // linear interpolation sampling
                    FAS_FLOAT phase_index = osc->phase_index[k];
                    int phase_index1 = (int)phase_index;
                    int phase_index2 = phase_index1 + 1;

                    FAS_FLOAT smp1 = fas_sine_wavetable[phase_index1];
                    FAS_FLOAT smp2 = fas_sine_wavetable[phase_index2];

                    FAS_FLOAT mu = phase_index - (FAS_FLOAT)phase_index1;

                    FAS_FLOAT smp = smp1 + mu * (smp2 - smp1);
                    //
#endif
                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

#ifdef PARTIAL_FX
                    int fx = (int)osc->fp1[k][0] % SP_OSC_MODS;
#ifdef WITH_SOUNDPIPE
                    if (fx == SP_CRUSH_MODS) {
                        sp_bitcrush *crush = (sp_bitcrush *)osc->sp_mods[k][SP_CRUSH_MODS];
                        
                        FAS_FLOAT bitdepth = 1.f + (osc->fp1[k][1] * 31.f);
                        FAS_FLOAT srate = lerp(osc->fp2[k][1], osc->fp2[k][0], curr_synth.lerp_t) * (FAS_FLOAT)fas_sample_rate;

                        crush->bitdepth = bitdepth;
                        crush->srate = fmax(1, srate);

                        sp_bitcrush_compute(sp, (sp_bitcrush *)osc->sp_mods[k][SP_CRUSH_MODS], &smp, &smp);
#ifndef MAGIC_CIRCLE
                        osc->phase_index[k] += osc->phase_step;
#endif  
                    } else if (fx == NOISE_MODS) {
#ifndef MAGIC_CIRCLE
                        osc->phase_index[k] += osc->phase_step * (1.0f + (fas_white_noise_table[osc->noise_index[k]++] * fas_noise_amount) * fmin(1, n->alpha));
#endif
                    } else {
#ifndef MAGIC_CIRCLE
                        osc->phase_index[k] += osc->phase_step;
#endif  
                    }
#endif
#endif

                    output_l += vl * smp;
                    output_r += vr * smp;

#ifndef MAGIC_CIRCLE
                    osc->phase_index[k] = fmod(osc->phase_index[k], fas_wavetable_size);
#endif
                }
            } else if (synthesis_method == FAS_SPECTRAL) {
                struct _synth_instrument_states *instruments_states = &fas_instrument_states[k];

                // accumulate frames until there is enough for a STFT frame
                if (instrument->p3) { // instrument
                    unsigned int input_instrument = instrument->p0 % fas_max_instruments;

                    if (k != input_instrument) {
                        struct _synth_instrument *instrument = &curr_synth.instruments[input_instrument];
                        instruments_states->in[0][instruments_states->position] = instrument->last_sample_l;
                        instruments_states->in[1][instruments_states->position] = instrument->last_sample_r;

                        instruments_states->position += 1;
                    }
                } else { // channel
                    unsigned int input_channel = instrument->p0 % fas_max_channels;

                    if (instrument->output_channel != input_channel) {
                        struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[input_channel];
                        instruments_states->in[0][instruments_states->position] = input_chn_settings->last_sample_l;
                        instruments_states->in[1][instruments_states->position] = input_chn_settings->last_sample_r;

                        instruments_states->position += 1;
                    }
                }

                if (instruments_states->position >= instruments_states->hop_size) {
                    if (instrument->p2 != 1) {
                        afSTFTforward(instruments_states->afSTFT_handle, instruments_states->in, instruments_states->stft_result);
                    }

                    // empty processing buffer
                    for (d = 0; d < 2; d += 1) {
                        for (w = 0; w < instruments_states->hop_size / 2; w += 1) {
                            instruments_states->stft_temp[d].re[w] = 0;
                            instruments_states->stft_temp[d].im[w] = 0;
                        }
                    }

                    // process incoming data
                    for (j = s; j < e; j += 1) {
                        struct note *n = &curr_notes[j];

                        struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                        FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                        FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                        FAS_FLOAT v[2] = { vl, vr };
                        FAS_FLOAT p[2] = { n->blue, n->alpha };

                        FAS_FLOAT bin_delta = ((FAS_FLOAT)(fas_sample_rate / 2) / instruments_states->hop_size);
                        FAS_FLOAT bin = osc->freq / bin_delta;

                        int ibin = round(bin);

                        // stereo spectral processing
                        for (d = 0; d < 2; d += 1) {                                
                            if (instrument->p2 == 1) {
                                instruments_states->stft_temp[d].re[ibin] = v[d];
                                instruments_states->stft_temp[d].im[ibin] = p[d];
                            } else {
                                FAS_FLOAT real = instruments_states->stft_result[d].re[ibin];
                                FAS_FLOAT imag = instruments_states->stft_result[d].im[ibin];

                                // polar
                                FAS_FLOAT mag = sqrtf(real * real + imag * imag);
                                FAS_FLOAT pha = atan2f(imag, real);

                                mag *= v[d];
                                pha *= p[d];

                                // rectangular
                                FAS_FLOAT creal = mag * cosf(pha);
                                FAS_FLOAT cimag = mag * sinf(pha);

                                instruments_states->stft_temp[d].re[ibin] = creal;
                                instruments_states->stft_temp[d].im[ibin] = cimag;
                            }
                        }
                    }

                    // copy processing result
                    for (d = 0; d < 2; d += 1) {
                        for (w = 0; w < instruments_states->hop_size / 2; w += 1) {
                            FAS_FLOAT re = instruments_states->stft_temp[d].re[w];
                            FAS_FLOAT im = instruments_states->stft_temp[d].im[w];

                            instruments_states->stft_result[d].re[w] = re;
                            instruments_states->stft_result[d].im[w] = im;
                        }
                    }

                    afSTFTinverse(instruments_states->afSTFT_handle, instruments_states->stft_result, instruments_states->out);

                    instruments_states->position = 0;
                }

                output_l += instruments_states->out[0][instruments_states->position];
                output_r += instruments_states->out[1][instruments_states->position];
            } else if (synthesis_method == FAS_GRANULAR) {
                int env_type = instrument->p0;
                FAS_FLOAT *gr_env = grain_envelope[env_type];

                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    unsigned int grain_index = n->osc_index * samples_count + n->psmp_index;
                    unsigned int si = curr_synth.bank_settings->h * samples_count;

                    struct grain *gr = &curr_synth.grains[grain_index];

                    FAS_FLOAT gr_out_l = 0, gr_out_r = 0;
                    computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, instrument->p3, gr_env, samples, n->psmp_index, fas_sample_rate, instrument->p1, instrument->p2, &gr_out_l, &gr_out_r);
/*
                    // WIP :  allow real-time density change
                    unsigned density_difference = n->density - n->pdensity;
                    if (density_difference < 0) {
                        gr->density[k] = -density_difference;

                        computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, n->density, gr_env, samples, n->psmp_index, instrument->p1, instrument->p2, &gr_out_l, &gr_out_r);

                        output_l += (vl * gr->density[k]) * gr_out_l * (1.0f - curr_synth.lerp_t);
                        output_r += (vr * gr->density[k]) * gr_out_r * (1.0f - curr_synth.lerp_t);

                        gr->density[k] = n->density;

                        gr_out_l = 0; gr_out_r = 0;
                        computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, 0, gr_env, samples, n->psmp_index, instrument->p1, instrument->p2, &gr_out_l, &gr_out_r);
                    } else {
                        computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, 0, gr_env, samples, n->psmp_index, instrument->p1, instrument->p2, &gr_out_l, &gr_out_r);
                    }
*/
                    // allow real-time sample change : cross-fade between old & new on a sudden sample change
                    if (n->psmp_index != n->smp_index) {
                        output_l += (vl * n->norm_density) * gr_out_l * (1.0f - curr_synth.lerp_t);
                        output_r += (vr * n->norm_density) * gr_out_r * (1.0f - curr_synth.lerp_t);

                        grain_index = n->osc_index * samples_count + n->smp_index;

                        gr_out_l = 0; gr_out_r = 0;
                        computeGrains(k, curr_synth.grains, grain_index, n->alpha, si, n->density, instrument->p3, gr_env, samples, n->smp_index, fas_sample_rate, instrument->p1, instrument->p2, &gr_out_l, &gr_out_r);

                        output_l += (vl * n->density) * gr_out_l;
                        output_r += (vr * n->density) * gr_out_r;
                    } else {
                        output_l += (vl * n->density) * gr_out_l;
                        output_r += (vr * n->density) * gr_out_r;
                    }
                }
            } else if (synthesis_method == FAS_FM) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT mod_phase_step = lerp(osc->fp4[k][3], osc->fp4[k][2], curr_synth.lerp_t);
                    FAS_FLOAT car_wav_size = osc->fp2[k][0];
                    FAS_FLOAT mod_wav_size = osc->fp2[k][1];

                    FAS_FLOAT fbf = (osc->fp1[k][0] + osc->fp1[k][1]) / 2; // 'anti-hunting' filter (simple low-pass)
                    FAS_FLOAT feedback_level = lerp(osc->fp4[k][1], osc->fp4[k][0], curr_synth.lerp_t);
                    FAS_FLOAT fb = fbf * feedback_level * mod_wav_size;

                    osc->phase_index2[k] += mod_phase_step;
                    osc->phase_index2[k] = fmod(osc->phase_index2[k], mod_wav_size);
                    
                    FAS_FLOAT fpm = osc->phase_index2[k] + fb;
                    fpm = fmod(fpm, mod_wav_size);
                    if (fpm < 0) fpm += mod_wav_size;

                    FAS_FLOAT mod_amplitude = lerp(osc->fp3[k][1], osc->fp3[k][0], curr_synth.lerp_t);

                    FAS_FLOAT smp_mod = osc->wav2[k][(int)fpm];
                    FAS_FLOAT mod = smp_mod * mod_amplitude * car_wav_size;
                    
                    osc->phase_index[k] += osc->fp1[k][3];
                    osc->phase_index[k] = osc->phase_index[k] + mod;

                    if (osc->phase_index[k] < 0) osc->phase_index[k] += car_wav_size;
                    osc->phase_index[k] = fmod(osc->phase_index[k], car_wav_size);

                    int phase_index1 = (int)osc->phase_index[k];
                    int phase_index2 = phase_index1 + 1;

                    FAS_FLOAT smp1 = osc->wav1[k][phase_index1];
                    FAS_FLOAT smp2 = osc->wav1[k][phase_index2];

                    FAS_FLOAT mu = osc->phase_index[k] - phase_index1;
                    FAS_FLOAT smp = smp1 + mu * (smp2 - smp1);

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    output_l += vl * smp;
                    output_r += vr * smp;

                    // feedback
                    osc->fp1[k][0] = osc->fp1[k][1];
                    osc->fp1[k][1] = smp;
                }
            } else if (synthesis_method == FAS_SUBTRACTIVE) {
                int filter_type = instrument->p0;
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    // implementation from http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
                    FAS_FLOAT smp = 0;
                    FAS_FLOAT t = osc->fphase[k] / M_PI2;

                    int waveform = ((int)fabs(floor(n->alpha))) % 6;

                    switch (waveform) {
                        case 0:
                            smp = raw_waveform(osc->fphase[k], 1);
                            smp -= poly_blep(osc->phase_increment, t);
                            break;
                        case 1:
                            smp = raw_waveform(osc->fphase[k], 2);
                            smp += poly_blep(osc->phase_increment, t);
                            smp -= poly_blep(osc->phase_increment, fmod(t + 0.5, 1.0));
                            break;
                        case 2:
                            smp = raw_waveform(osc->fphase[k], 3);
                            smp += poly_blep(osc->phase_increment, t);
                            smp -= poly_blep(osc->phase_increment, fmod(t + 0.5, 1.0));
                            smp = osc->phase_increment * smp + (1.0 - osc->phase_increment) * osc->pvalue[k];
                            osc->pvalue[k] = smp;
                            break;
                        case 3: // white noise
#ifdef WITH_SOUNDPIPE
                            sp_noise_compute(sp, (sp_noise *)osc->sp_gens[k][SP_WHITE_NOISE_GENERATOR], NULL, &smp);
#else
                            smp = fas_white_noise_table[(int)osc->phase_index[k]];

                            osc->phase_index[k] += osc->phase_step;
                            osc->phase_index[k] = fmod(osc->phase_index[k], fas_wavetable_size);
#endif
                            break;
#ifdef WITH_SOUNDPIPE
                        case 4: // pink noise
                            sp_pinknoise_compute(sp, (sp_pinknoise *)osc->sp_gens[k][SP_PINK_NOISE_GENERATOR], NULL, &smp);
                            break;
                        case 5: // brown noise
                            sp_brown_compute(sp, (sp_brown *)osc->sp_gens[k][SP_BROWN_NOISE_GENERATOR], NULL, &smp);
                            break;
#endif
                        default:
                            break;
                    }

                    osc->fphase[k] += osc->phase_increment;
                    while (osc->fphase[k] >= M_PI2) {
                        osc->fphase[k] -= M_PI2;
                    }

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

#ifdef WITH_SOUNDPIPE
                    if (filter_type == 0) {
                        sp_moogladder *moog_ladder = osc->sp_filters[k][SP_MOOG_FILTER];

                        // interpolate parameters
                        moog_ladder->freq = lerp(osc->fp1[k][0], osc->fp1[k][1], curr_synth.lerp_t);
                        moog_ladder->res = lerp(osc->fp1[k][2], osc->fp1[k][3], curr_synth.lerp_t);

                        sp_moogladder_compute(sp, moog_ladder, &smp, &smp); 
                    } else if (filter_type == 1) {
                        sp_diode *diode_ladder = osc->sp_filters[k][SP_DIODE_FILTER];

                        // interpolate parameters
                        diode_ladder->freq = lerp(osc->fp1[k][0], osc->fp1[k][1], curr_synth.lerp_t);
                        diode_ladder->res = lerp(osc->fp1[k][2], osc->fp1[k][3], curr_synth.lerp_t);

                        sp_diode_compute(sp, diode_ladder, &smp, &smp);
                    } else if (filter_type == 2) {
                        sp_wpkorg35 *korg = osc->sp_filters[k][SP_KORG35_FILTER];

                        // interpolate parameters
                        korg->cutoff = lerp(osc->fp1[k][0], osc->fp1[k][1], curr_synth.lerp_t);
                        korg->res = lerp(osc->fp1[k][2], osc->fp1[k][3], curr_synth.lerp_t);

                        sp_wpkorg35_compute(sp, korg, &smp, &smp);
                    } else if (filter_type == 3) {
                        sp_lpf18 *lpf18 = osc->sp_filters[k][SP_LPF18_FILTER];

                        // interpolate parameters
                        lpf18->cutoff = lerp(osc->fp1[k][0], osc->fp1[k][1], curr_synth.lerp_t);
                        lpf18->res = lerp(osc->fp1[k][2], osc->fp1[k][3], curr_synth.lerp_t);

                        sp_lpf18_compute(sp, lpf18, &smp, &smp);
                    }
#else
                    if (filter_type == 0) {
                        smp = huovilainen_moog(smp, n->cutoff, n->res, osc->fp1[k], osc->fp2[k], osc->fp3[k], 2);
                    }
#endif

                    output_l += vl * smp;
                    output_r += vr * smp;
                }
            } else if (synthesis_method == FAS_PHYSICAL_MODELLING) {
                int model_type = instrument->p0;
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

#ifdef WITH_SOUNDPIPE
                    if (model_type == 2) {
                        FAS_FLOAT trigger_l = ((n->previous_volume_l <= 0) || osc->triggered[k]) ? 1.f : 0.f;
                        FAS_FLOAT trigger_r = ((n->previous_volume_r <= 0) || osc->triggered[k]) ? 1.f : 0.f;
                        FAS_FLOAT bar_out_l = 0.;
                        FAS_FLOAT bar_out_r = 0.;

                        sp_bar_compute(sp, (sp_bar *)osc->sp_gens[k][SP_BAR_GENERATOR], &trigger_l, &bar_out_l);
                        sp_bar_compute(sp, (sp_bar *)osc->sp_gens[k][SP_BAR_GENERATOR], &trigger_r, &bar_out_r);

                        output_l += vl * bar_out_l;
                        output_r += vr * bar_out_r;

                        if (osc->triggered[k]) {
                            osc->triggered[k] = 0;
                        }
                    } else if (model_type == 1) {
                        FAS_FLOAT trigger_l = ((n->previous_volume_l <= 0) || osc->triggered[k]) ? 1.f : 0.f;
                        FAS_FLOAT trigger_r = ((n->previous_volume_r <= 0) || osc->triggered[k]) ? 1.f : 0.f;
                        FAS_FLOAT drip_out_l = 0.;
                        FAS_FLOAT drip_out_r = 0.;

                        sp_drip_compute(sp, (sp_drip *)osc->sp_gens[k][SP_DRIP_GENERATOR], &trigger_l, &drip_out_l);
                        sp_drip_compute(sp, (sp_drip *)osc->sp_gens[k][SP_DRIP_GENERATOR], &trigger_r, &drip_out_r);

                        output_l += vl * drip_out_l;
                        output_r += vr * drip_out_r;

                        if (osc->triggered[k]) {
                            osc->triggered[k] = 0;
                        }
                    } else if (model_type == 0) {
#endif
                    FAS_FLOAT phase_step = osc->freq / (FAS_FLOAT)fas_sample_rate * (osc->buffer_len + 0.5);

                    unsigned int curr_sample_index = osc->fphase[k];
                    unsigned int curr_sample_index2 = curr_sample_index + 1;

                    FAS_FLOAT mu = osc->fphase[k] - (FAS_FLOAT)curr_sample_index;

                    unsigned int si = k * osc->buffer_len;

                    unsigned int curr_sample = si + (curr_sample_index % osc->buffer_len);
                    unsigned int curr_sample2 = si + (curr_sample_index2 % osc->buffer_len);

                    FAS_FLOAT smp = osc->buffer[curr_sample];

                    FAS_FLOAT stretch = osc->fp1[k][0];
                    FAS_FLOAT in = 0.0f;
                    if (stretch <= randf(0.f, 1.f)) {
                        in = 0.5f * ((smp + mu * (osc->buffer[curr_sample2] - smp)) + osc->pvalue[k]);
                    } else {
                        in = smp;
                    }

                    // allpass
                    FAS_FLOAT delay = fabs((FAS_FLOAT)osc->buffer_len - ((FAS_FLOAT)fas_sample_rate / osc->freq));
                    FAS_FLOAT c = (1.0f - delay) / (1.0f + delay);

                    osc->buffer[curr_sample] = osc->fp4[k][0] + c * in;
                    osc->fp4[k][0] = in - c * osc->buffer[curr_sample];

                    FAS_FLOAT ol = osc->buffer[curr_sample];

                    osc->pvalue[k] = ol;

                    output_l += vl * ol;
                    output_r += vr * ol;

                    osc->fphase[k] += phase_step;
#ifdef WITH_SOUNDPIPE
                    }
#endif
                }
            } else if (synthesis_method == FAS_WAVETABLE_SYNTH) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    struct sample *smp = &waves[(int)osc->fp1[k][0]];

                    unsigned int curr_sample_index = osc->fp1[k][1];
                    unsigned int curr_sample_index2 = curr_sample_index + 1;

                    FAS_FLOAT mu = osc->fp1[k][1] - (FAS_FLOAT)curr_sample_index;

                    FAS_FLOAT wsmp = smp->data_l[curr_sample_index] + mu * (smp->data_l[curr_sample_index2] - smp->data_l[curr_sample_index]);

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    FAS_FLOAT fsmp = 0;
                    if (n->res > 0) {
                        // next sample interpolation
                        struct sample *nsmp = &waves[(int)osc->fp2[k][0]];

                        unsigned int nsample_index = osc->fp2[k][1];
                        unsigned int nsample_index2 = nsample_index + 1;
                        FAS_FLOAT nmu = osc->fp2[k][1] - (FAS_FLOAT)nsample_index;
                        FAS_FLOAT nwsmp = nsmp->data_l[nsample_index] + nmu * (nsmp->data_l[nsample_index2] - nsmp->data_l[nsample_index]);
                        //

                        fsmp = wsmp + fmin(osc->fp1[k][3], 1.0) * (nwsmp - wsmp);

                        osc->fp2[k][1] += osc->fp2[k][2];
 
                        osc->fp2[k][1] = fmod(osc->fp2[k][1], nsmp->frames);
                    } else {
                        fsmp = wsmp;
                    }

                    output_l += vl * fsmp;
                    output_r += vr * fsmp;

                    osc->fp1[k][1] += osc->fp1[k][2];
                    if (osc->fp1[k][1] >= smp->frames) {
                        osc->fp1[k][1] = fmod(osc->fp1[k][1], smp->frames);

                        if (osc->fp1[k][3] >= 1) {
                            unsigned int start_index = abs((int)round(n->blue)) % waves_count;
                            unsigned int stop_index = abs((int)round(n->alpha)) % waves_count;

                            unsigned int next_start_index = start_index;

                            if (n->blue < 0) {
                                osc->fp1[k][0] -= 1;
                                next_start_index = stop_index;
                            } else {
                                osc->fp1[k][0] += 1;
                            }

                            unsigned int current_index = osc->fp1[k][0];

                            if (current_index < start_index) {
                                osc->fp1[k][0] = next_start_index;
                            }

                            if (current_index > stop_index) {
                                osc->fp1[k][0] = next_start_index;
                            }

                            if (n->blue > 0) {
                                osc->fp2[k][0] = (unsigned int)(osc->fp1[k][0] + 1) % waves_count;
                            } else {
                                osc->fp2[k][0] = osc->fp1[k][0] - 1;
                                if (osc->fp2[k][0] < 0) {
                                    osc->fp2[k][0] = start_index;
                                }
                            }

                            struct sample *smp = &waves[(int)osc->fp1[k][0]];
                            osc->fp1[k][2] = osc->freq / smp->pitch / ((FAS_FLOAT)fas_sample_rate / (FAS_FLOAT)smp->samplerate);
                            osc->fp1[k][3] = 0;

                            struct sample *nsmp = &waves[(int)osc->fp2[k][0]];
                            osc->fp2[k][2] = osc->freq / nsmp->pitch / ((FAS_FLOAT)fas_sample_rate / (FAS_FLOAT)nsmp->samplerate);

                            //osc->fp1[k][1] = 0;
                        }
                    }

                    osc->fp1[k][3] += lerp(osc->fp3[k][1], osc->fp3[k][0], curr_synth.lerp_t);
                }
            } else if (synthesis_method == FAS_MODULATION) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    if (instrument->p0 == 0) {
                        // fx modulation
                        int chn = ((int)floor(instrument->p1)) % fas_max_channels;
                        int slot = ((int)floor(instrument->p2)) % FAS_MAX_FX_SLOTS;
                        int target = 2 + ((int)floor(instrument->p3)) % FAS_MAX_FX_PARAMETERS;
                        int easing_type = (int)instrument->p4 % (FAS_EASING_COUNT + 1);

                        if (chn >= 0 && slot >= 0 && target >= 0) {
                            struct _synth_chn_settings *target_chn_settings = &curr_synth.chn_settings[chn];

                            FAS_FLOAT value = lerp(n->palpha, n->alpha, applyEasing(easing_type, curr_synth.lerp_t));

                            updateEffectParameter(
#ifdef WITH_SOUNDPIPE
                                sp,
#endif                    
                                synth_fx[chn], target_chn_settings, slot, target, value);
                        }
                    } else if (instrument->p0 == 1) {
                        // chn settings modulation
                        int instrument_index = ((int)floor(instrument->p1)) % fas_max_instruments;
                        int param = ((int)floor(instrument->p2)) % 6;
                        int easing_type = ((int)floor(instrument->p4)) % (FAS_EASING_COUNT + 1);

                        if (instrument_index >= 0 && param >= 0) {
                            FAS_FLOAT value = lerp(n->palpha, n->alpha, applyEasing(easing_type, curr_synth.lerp_t));
                            
                            struct _synth_instrument *target_instrument = &curr_synth.instruments[instrument_index];
                            if (param == 0) {
                                target_instrument->p0 = value;
                            } else if (param == 1) {
                                target_instrument->p1 = value;
                            } else if (param == 2) {
                                target_instrument->p2 = value;
                            } else if (param == 3) {
                                target_instrument->p3 = value;
                            } else if (param == 4) {
                                target_instrument->p4 = value;
                            }
                        }
                    }
                }
            } else if (synthesis_method == FAS_INPUT) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    int chn_count = fas_input_channels / 2;
                    int chn = abs((int)n->blue) % chn_count;

#ifdef INTERLEAVED_SAMPLE_FORMAT
                    FAS_FLOAT inl = audio_in[i * 2 * chn_count + chn * 2];
                    FAS_FLOAT inr = audio_in[i * 2 * chn_count + 1 + chn * 2];
#else
                    FAS_FLOAT inl = inputBuffer[chn * 2][i];
                    FAS_FLOAT inr = inputBuffer[chn * 2 + 1][i];
#endif

                    FAS_FLOAT outl = inl;
                    FAS_FLOAT outr = inr;
 
#ifdef WITH_SOUNDTOUCH
                    FAS_FLOAT data[2] = { inl, inr };

                    soundtouch_setPitch(osc->st[k], lerp(osc->fp1[k][0], osc->fp1[k][1], curr_synth.lerp_t));

                    soundtouch_setTempo(osc->st[k], lerp(osc->fp1[k][2], osc->fp1[k][3], curr_synth.lerp_t) * 10.);
                    soundtouch_putSamples(osc->st[k], &data[0], 1);

                    unsigned int result = soundtouch_receiveSamples(osc->st[k], &data[0], 1);

                    if (result) {
                        outl = data[0];
                        outr = data[1];
                    }
#endif

                    output_l += outl * vl;
                    output_r += outr * vr;
                }
            }
#ifdef WITH_SOUNDPIPE
            else if (synthesis_method == FAS_BANDPASS) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    double bint = 0;
                    FAS_FLOAT bflt = modf(fabs(n->blue), &bint);

                    FAS_FLOAT il;
                    FAS_FLOAT ir;

                    if (bflt > 0) {
                        int chn = (int)bint % fas_max_channels;
                        struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[chn];

                        il = input_chn_settings->last_sample_l;
                        ir = input_chn_settings->last_sample_r;
                    } else {
                        int instrument_index = (int)bint % fas_max_instruments;
                        struct _synth_instrument *instrument = &curr_synth.instruments[instrument_index];

                        il = instrument->last_sample_l;
                        ir = instrument->last_sample_r; 
                    }

                    sp_butbp *butbp_l = (sp_butbp *)osc->sp_filters[k][SP_BANDPASS_FILTER_L];
                    sp_butbp *butbp_r = (sp_butbp *)osc->sp_filters[k][SP_BANDPASS_FILTER_R];

                    // interpolate parameters
                    butbp_l->bw = butbp_r->bw = lerp(osc->fp1[k][0], osc->fp1[k][1], curr_synth.lerp_t);

                    FAS_FLOAT sl = 0.0f;
                    FAS_FLOAT sr = 0.0f;

                    if (instrument->p0 == 1) {
                        sp_butbp_compute(sp, butbp_l, &il, &sl);
                        sp_butbp_compute(sp, butbp_r, &ir, &sr);
                        sp_butbp_compute(sp, butbp_l, &sl, &sl);
                        sp_butbp_compute(sp, butbp_r, &sr, &sr);
                    } else if (instrument->p0 == 2) {
                        sp_butbp_compute(sp, butbp_l, &il, &sl);
                        sp_butbp_compute(sp, butbp_r, &ir, &sr);
                        sp_butbp_compute(sp, butbp_l, &sl, &sl);
                        sp_butbp_compute(sp, butbp_r, &sr, &sr);
                        sp_butbp_compute(sp, butbp_l, &sl, &sl);
                        sp_butbp_compute(sp, butbp_r, &sr, &sr);
                    } else if (instrument->p0 == 3) {
                        sp_butbp_compute(sp, butbp_l, &il, &sl);
                        sp_butbp_compute(sp, butbp_r, &ir, &sr);
                        sp_butbp_compute(sp, butbp_l, &sl, &sl);
                        sp_butbp_compute(sp, butbp_r, &sr, &sr);
                        sp_butbp_compute(sp, butbp_l, &sl, &sl);
                        sp_butbp_compute(sp, butbp_r, &sr, &sr);
                        sp_butbp_compute(sp, butbp_l, &sl, &sl);
                        sp_butbp_compute(sp, butbp_r, &sr, &sr);
                    } else {
                        sp_butbp_compute(sp, butbp_l, &il, &sl);
                        sp_butbp_compute(sp, butbp_r, &ir, &sr);
                    }

                    output_l += sl * vl;
                    output_r += sr * vr;
                }
            } else if (synthesis_method == FAS_FORMANT_SYNTH) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    double bint = 0;
                    FAS_FLOAT bflt = modf(fabs(n->blue), &bint);

                    FAS_FLOAT il;
                    FAS_FLOAT ir;

                    if ((unsigned int)bint != instrument->output_channel) {
                        int chn = (unsigned int)bint % fas_max_channels;
                        struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[chn];

                        il = input_chn_settings->last_sample_l;
                        ir = input_chn_settings->last_sample_r;

                        sp_fofilt *fofilt_l = (sp_fofilt *)osc->sp_filters[k][SP_FORMANT_FILTER_L];
                        sp_fofilt *fofilt_r = (sp_fofilt *)osc->sp_filters[k][SP_FORMANT_FILTER_R];
                        
                        // interpolate parameters
                        fofilt_l->atk = fofilt_r->atk = lerp(fofilt_r->patk, fofilt_r->tatk, curr_synth.lerp_t);
                        fofilt_l->dec = fofilt_r->dec = lerp(fofilt_r->pdec, fofilt_r->tdec, curr_synth.lerp_t);

                        FAS_FLOAT sl = 0.0f;
                        FAS_FLOAT sr = 0.0f;

                        sp_fofilt_compute(sp, (sp_fofilt *)fofilt_l, &il, &sl);
                        sp_fofilt_compute(sp, (sp_fofilt *)fofilt_r, &ir, &sr);

                        output_l += sl * vl;
                        output_r += sr * vr;
                    }
                }
            } else if (synthesis_method == FAS_STRING_RESON) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    double bint = 0;
                    FAS_FLOAT bflt = modf(fabs(n->blue), &bint);

                    FAS_FLOAT il;
                    FAS_FLOAT ir;

                    if (bflt > 0) {
                        int chn = (int)bint % fas_max_channels;
                        struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[chn];

                        il = input_chn_settings->last_sample_l;
                        ir = input_chn_settings->last_sample_r;
                    } else {
                        int instrument_index = (int)bint % fas_max_instruments;
                        struct _synth_instrument *instrument = &curr_synth.instruments[instrument_index];

                        il = instrument->last_sample_l;
                        ir = instrument->last_sample_r; 
                    }

                    FAS_FLOAT sl = 0.0f;
                    FAS_FLOAT sr = 0.0f;

                    sp_streson_compute(sp, (sp_streson *)osc->sp_filters[k][SP_STRES_FILTER_L], &il, &sl);
                    sp_streson_compute(sp, (sp_streson *)osc->sp_filters[k][SP_STRES_FILTER_R], &ir, &sr);

                    output_l += sl * vl;
                    output_r += sr * vr;
                }
            } else if (synthesis_method == FAS_MODAL_SYNTH) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    double bint = 0;
                    FAS_FLOAT bflt = modf(fabs(n->blue), &bint);

                    FAS_FLOAT il;
                    FAS_FLOAT ir;

                    if (bflt > 0) {
                        int chn = (int)bint % fas_max_channels;
                        struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[chn];

                        il = input_chn_settings->last_sample_l;
                        ir = input_chn_settings->last_sample_r;
                    } else {
                        int instrument_index = (int)bint % fas_max_instruments;
                        struct _synth_instrument *instrument = &curr_synth.instruments[instrument_index];

                        il = instrument->last_sample_l;
                        ir = instrument->last_sample_r; 
                    }

                    FAS_FLOAT sl = 0.0f;
                    FAS_FLOAT sr = 0.0f;

                    sp_mode_compute(sp, (sp_mode *)osc->sp_filters[k][SP_MODE_FILTER_L], &il, &sl);
                    sp_mode_compute(sp, (sp_mode *)osc->sp_filters[k][SP_MODE_FILTER_R], &ir, &sr);

                    output_l += sl * vl;
                    output_r += sr * vr;
                }
            } else if (synthesis_method == FAS_PHASE_DISTORSION) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    double bint = 0;
                    FAS_FLOAT bflt = modf(fabs(n->blue), &bint);

                    FAS_FLOAT il;
                    FAS_FLOAT ir;

                    if (bflt > 0) {
                        int chn = (int)bint % fas_max_channels;
                        struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[chn];

                        il = input_chn_settings->last_sample_l;
                        ir = input_chn_settings->last_sample_r;
                    } else {
                        int instrument_index = (int)bint % fas_max_instruments;
                        struct _synth_instrument *instrument = &curr_synth.instruments[instrument_index];

                        il = instrument->last_sample_l;
                        ir = instrument->last_sample_r; 
                    }

                    FAS_FLOAT sl = 0.0f;
                    FAS_FLOAT sr = 0.0f;

                    sp_pdhalf *pd_gen = (sp_pdhalf *)osc->sp_gens[k][SP_PD_GENERATOR];

                    // interpolate parameters
                    pd_gen->amount = lerp(pd_gen->pamount, pd_gen->tamount, curr_synth.lerp_t);

                    sp_pdhalf_compute(sp, pd_gen, &il, &sl);
                    sp_pdhalf_compute(sp, pd_gen, &ir, &sr);

                    output_l += sl * vl;
                    output_r += sr * vr;
                }
            }
#endif
#ifdef WITH_FAUST
            else if (synthesis_method == FAS_FAUST) {
                for (j = s; j < e; j += 1) {
                    struct note *n = &curr_notes[j];

                    struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                    FAS_FLOAT vl = n->previous_volume_l + n->diff_volume_l * curr_synth.lerp_t;
                    FAS_FLOAT vr = n->previous_volume_r + n->diff_volume_r * curr_synth.lerp_t;

                    if (osc->faust_gens_len > 0) {
                        int faust_dsp_index = instrument->p0 % osc->faust_gens_len;

                        struct _fas_faust_dsp *fas_faust_dsp = osc->faust_gens[k][faust_dsp_index];

                        // update Faust DSP params
                        struct _fas_faust_ui_control *ctrl = fas_faust_dsp->controls;

                        struct _fas_faust_ui_control *tmp;
                        // note : p0 is used as the Faust generator index
                        tmp = getFaustControl(ctrl, "fs_p0");
                        if (tmp) {
                            *tmp->zone = fmax(tmp->min, fmin(tmp->max, instrument->p1));
                        }

                        tmp = getFaustControl(ctrl, "fs_p1");
                        if (tmp) {
                            *tmp->zone = fmax(tmp->min, fmin(tmp->max, instrument->p2));
                        }

                        tmp = getFaustControl(ctrl, "fs_p2");
                        if (tmp) {
                            *tmp->zone = fmax(tmp->min, fmin(tmp->max, instrument->p3));
                        }

                        tmp = getFaustControl(ctrl, "fs_p3");
                        if (tmp) {
                            *tmp->zone = fmax(tmp->min, fmin(tmp->max, instrument->p4));
                        }

                        int faust_dsp_input_count = getNumInputsCDSPInstance(fas_faust_dsp->dsp);
                        if (faust_dsp_input_count >= 1) {
                            double bint = 0;
                            FAS_FLOAT bflt = modf(fabs(n->blue), &bint);

                            FAS_FLOAT il;
                            FAS_FLOAT ir;

                            if (bflt > 0) {
                                int chn = (int)bint % fas_max_channels;
                                struct _synth_chn_settings *input_chn_settings = &curr_synth.chn_settings[chn];

                                il = input_chn_settings->last_sample_l;
                                ir = input_chn_settings->last_sample_r;
                            } else {
                                int instrument_index = (int)bint % fas_max_instruments;
                                struct _synth_instrument *instrument = &curr_synth.instruments[instrument_index];

                                il = instrument->last_sample_l;
                                ir = instrument->last_sample_r; 
                            }

                            FAS_FLOAT sl = 0.0f;
                            FAS_FLOAT sr = 0.0f;

                            FAUSTFLOAT *faust_input[2] = { &il, &ir };
                            FAUSTFLOAT *faust_output[2] = { &sl, &sr };

                            computeCDSPInstance(fas_faust_dsp->dsp, 1, faust_input, faust_output);

                            output_l += sl * vl;
                            output_r += sr * vr;
                        } else {
                            FAS_FLOAT sl = 0.0f;
                            FAS_FLOAT sr = 0.0f;

                            FAUSTFLOAT *faust_output[2] = { &sl, &sr };

                            computeCDSPInstance(fas_faust_dsp->dsp, 1, NULL, faust_output);

                            output_l += sl * vl;
                            output_r += sr * vr;
                        }
                    }
                }
            }
#endif
            else if (synthesis_method == FAS_VOID) {
                break;
            }

            if (!instrument->muted) {
                chn_settings->output_l += output_l;
                chn_settings->output_r += output_r;
            }

            instrument->last_sample_l = output_l;
            instrument->last_sample_r = output_r;
        }

        for (k = 0; k < fas_max_channels; k += 1) {
            struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[k];

            if (chn_settings->output_chn < 0) {
                continue;
            }

            // channel effects
            d = 0; j = 0;
            int fx_id = -1;
            do {
                struct _synth_fx *fx = NULL;

                if (synth_fx) {
                    int bypass = chn_settings->fx[d].bypass;
                    if (bypass) {
                        fx_id = -2;
                    } else {
                        fx_id = chn_settings->fx[d].fx_id;

                        fx = synth_fx[k];
                    }
                }

                if (fx_id == FX_CONV) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;
                    
                    sp_conv_compute(sp, (sp_conv *)fx->conv[j], &insl, &outsl);
                    sp_conv_compute(sp, (sp_conv *)fx->conv[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_ZITAREV) {
#ifdef WITH_SOUNDPIPE
                    sp_zitarev_compute(sp, (sp_zitarev *)fx->zitarev[d], &chn_settings->output_l, &chn_settings->output_r, &chn_settings->output_l, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_SCREV) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;
                    
                    sp_revsc_compute(sp, (sp_revsc *)fx->revsc[d], &insl, &insr, &outsl, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[d] + outsl * fx->wet[d];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[d] + outsr * fx->wet[d];
#endif
                } else if (fx_id == FX_AUTOWAH) {
#ifdef WITH_SOUNDPIPE
                    sp_autowah_compute(sp, (sp_autowah *)fx->autowah[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_autowah_compute(sp, (sp_autowah *)fx->autowah[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_PHASER) {
#ifdef WITH_SOUNDPIPE
                    sp_phaser_compute(sp, (sp_phaser *)fx->phaser[d], &chn_settings->output_l, &chn_settings->output_r, &chn_settings->output_l, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_DELAY) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;
                    
                    sp_delay_compute(sp, (sp_delay *)fx->delay[j], &insl, &outsl);
                    sp_delay_compute(sp, (sp_delay *)fx->delay[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_SMOOTH_DELAY) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;
                    sp_smoothdelay_compute(sp, (sp_smoothdelay *)fx->sdelay[j], &insl, &chn_settings->output_l);
                    sp_smoothdelay_compute(sp, (sp_smoothdelay *)fx->sdelay[j + 1], &insr, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_COMB) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;

                    sp_comb_compute(sp, (sp_comb *)fx->comb[j], &insl, &outsl);
                    sp_comb_compute(sp, (sp_comb *)fx->comb[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_BITCRUSH) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;

                    sp_bitcrush_compute(sp, (sp_bitcrush *)fx->bitcrush[j], &insl, &outsl);
                    sp_bitcrush_compute(sp, (sp_bitcrush *)fx->bitcrush[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_DISTORSION) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;

                    sp_dist_compute(sp, (sp_dist *)fx->dist[j], &insl, &outsl);
                    sp_dist_compute(sp, (sp_dist *)fx->dist[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_SATURATOR) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;

                    sp_saturator_compute(sp, (sp_saturator *)fx->saturator[j], &insl, &outsl);
                    sp_saturator_compute(sp, (sp_saturator *)fx->saturator[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_COMPRESSOR) {
#ifdef WITH_SOUNDPIPE
                    sp_compressor_compute(sp, (sp_compressor *)fx->compressor[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_compressor_compute(sp, (sp_compressor *)fx->compressor[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_PEAK_LIMITER) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;

                    sp_peaklim_compute(sp, (sp_peaklim *)fx->peaklimit[j], &insl, &outsl);
                    sp_peaklim_compute(sp, (sp_peaklim *)fx->peaklimit[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_CLIP) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAS_FLOAT outsl = 0;
                    FAS_FLOAT outsr = 0;

                    sp_clip_compute(sp, (sp_clip *)fx->clip[j], &insl, &outsl);
                    sp_clip_compute(sp, (sp_clip *)fx->clip[j + 1], &insr, &outsr);

                    chn_settings->output_l = chn_settings->output_l * fx->dry[j] + outsl * fx->wet[j];
                    chn_settings->output_r = chn_settings->output_r * fx->dry[j + 1] + outsr * fx->wet[j + 1];
#endif
                } else if (fx_id == FX_B_LOWPASS) {
#ifdef WITH_SOUNDPIPE
                    sp_butlp_compute(sp, (sp_butlp *)fx->butlp[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_butlp_compute(sp, (sp_butlp *)fx->butlp[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_B_HIGHPASS) {
#ifdef WITH_SOUNDPIPE
                    sp_buthp_compute(sp, (sp_buthp *)fx->buthp[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_buthp_compute(sp, (sp_buthp *)fx->buthp[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_B_BANDPASS) {
#ifdef WITH_SOUNDPIPE
                    sp_butbp_compute(sp, (sp_butbp *)fx->butbp[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_butbp_compute(sp, (sp_butbp *)fx->butbp[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_B_BANDREJECT) {
#ifdef WITH_SOUNDPIPE
                    sp_butbr_compute(sp, (sp_butbr *)fx->butbr[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_butbr_compute(sp, (sp_butbr *)fx->butbr[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_PAREQ) {
#ifdef WITH_SOUNDPIPE
                    sp_pareq_compute(sp, (sp_pareq *)fx->pareq[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_pareq_compute(sp, (sp_pareq *)fx->pareq[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_MOOG_LPF) {
#ifdef WITH_SOUNDPIPE
                    sp_moogladder_compute(sp, (sp_moogladder *)fx->mooglp[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_moogladder_compute(sp, (sp_moogladder *)fx->mooglp[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_DIODE_LPF) {
#ifdef WITH_SOUNDPIPE
                    sp_diode_compute(sp, (sp_diode *)fx->diodelp[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_diode_compute(sp, (sp_diode *)fx->diodelp[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_KORG_LPF) {
#ifdef WITH_SOUNDPIPE
                    sp_wpkorg35_compute(sp, (sp_wpkorg35 *)fx->korglp[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_wpkorg35_compute(sp, (sp_wpkorg35 *)fx->korglp[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_18_LPF) {
#ifdef WITH_SOUNDPIPE
                    sp_lpf18_compute(sp, (sp_lpf18 *)fx->lpf18[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_lpf18_compute(sp, (sp_lpf18 *)fx->lpf18[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_TBVCF) {
#ifdef WITH_SOUNDPIPE
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;
                    sp_tbvcf_compute(sp, (sp_tbvcf *)fx->tbvcf[j], &insl, &chn_settings->output_l);
                    sp_tbvcf_compute(sp, (sp_tbvcf *)fx->tbvcf[j + 1], &insr, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_FOLD) {
#ifdef WITH_SOUNDPIPE
                    sp_fold_compute(sp, (sp_fold *)fx->fold[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_fold_compute(sp, (sp_fold *)fx->fold[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_DC_BLOCK) {
#ifdef WITH_SOUNDPIPE
                    sp_dcblock_compute(sp, (sp_dcblock *)fx->dcblock[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_dcblock_compute(sp, (sp_dcblock *)fx->dcblock[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_LPC) {
#ifdef WITH_SOUNDPIPE
                    sp_lpc_compute(sp, (sp_lpc *)fx->lpc[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_lpc_compute(sp, (sp_lpc *)fx->lpc[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_WAVESET) {
#ifdef WITH_SOUNDPIPE
                    sp_waveset_compute(sp, (sp_waveset *)fx->wset[j], &chn_settings->output_l, &chn_settings->output_l);
                    sp_waveset_compute(sp, (sp_waveset *)fx->wset[j + 1], &chn_settings->output_r, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_PANNER) {
#ifdef WITH_SOUNDPIPE
                    sp_panst_compute(sp, (sp_panst *)fx->panner[d], &chn_settings->output_l, &chn_settings->output_r, &chn_settings->output_l, &chn_settings->output_r);
#endif
                } else if (fx_id == FX_FAUST) {
#ifdef WITH_FAUST
                    FAS_FLOAT insl = chn_settings->output_l;
                    FAS_FLOAT insr = chn_settings->output_r;

                    FAUSTFLOAT *faust_input[2] = { &insl, &insr };
                    FAUSTFLOAT *faust_output[2] = { &chn_settings->output_l, &chn_settings->output_r };

                    struct _synth_fx_settings *chn_fx_settings = &chn_settings->fx[j];

                    unsigned int fx_index = (unsigned int)chn_fx_settings->fp[0];
                    if (fx_index < fx->faust_effs_len) {
                        struct _fas_faust_dsp *fas_faust_dsp = fx->faust_effs[j][fx_index];

                        computeCDSPInstance(fas_faust_dsp->dsp, 1, faust_input, faust_output);
                    }
#endif
                }
                
                d += 1;
                j += 2;
            } while (fx_id != -1);
            //

            chn_settings->last_sample_l = chn_settings->output_l;
            chn_settings->last_sample_r = chn_settings->output_r;

            FAS_FLOAT chn_gain = chn_settings->last_chn_gain + (chn_settings->curr_chn_gain - chn_settings->last_chn_gain) * curr_synth.lerp_t;

#ifdef INTERLEAVED_SAMPLE_FORMAT
            audio_out[i * 2 * frame_data_count + chn_settings->output_chn * 2] += chn_settings->output_l * chn_gain * curr_synth.settings->gain_lr;
            audio_out[i * 2 * frame_data_count + 1 + chn_settings->output_chn * 2] += chn_settings->output_r * chn_gain * curr_synth.settings->gain_lr;
#else
            int output_chn = chn_settings->output_chn * 2;
            outputBuffer[output_chn][i] += chn_settings->output_l * chn_gain * curr_synth.settings->gain_lr;
            outputBuffer[output_chn + 1][i] += chn_settings->output_r * chn_gain * curr_synth.settings->gain_lr;
#endif

            chn_settings->output_l = 0;
            chn_settings->output_r = 0;
        }

        curr_synth.lerp_t += lerp_t_step * fas_smooth_factor;
        curr_synth.lerp_t = fmin(curr_synth.lerp_t, 1.0f);

        curr_synth.curr_sample += 1;

        // compute the next event
        if (curr_synth.curr_sample >= note_time_samples) {
            lerp_t_step = 0;

            curr_synth.curr_sample = 0;

            read_status = lfds720_ringbuffer_n_read(&rs, &key, NULL);
            if (read_status == 1) {
                freelist_frames_data = (struct _freelist_frames_data *)key;

                _notes = freelist_frames_data->data;

                // previously consumed notes data is pushed back into the pool
                if (curr_notes != dummy_notes) {
                    LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
                    lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &curr_freelist_frames_data->fe);
                }

                curr_notes = _notes;
                curr_freelist_frames_data = freelist_frames_data;

                curr_synth.lerp_t = 0;
                lerp_t_step = 1 / note_time_samples;

                //note_buffer_len = curr_notes[0].osc_index;

                fas_drop_counter = 0;

                // once we have notes data we prepare some pre-processing for later use (optimization, reseting filters on note-on etc.)
                note_buffer_len = 0;
                pv_note_buffer_len = 0;

                for (k = 0; k < fas_max_channels; k += 1) {
                    struct _synth_chn_settings *chn_settings = &curr_synth.chn_settings[k];

                    if (chn_settings->output_chn >= 0) {
                        // for smooth channel mute
                        if (chn_settings->muted != chn_settings->mute_state) {
                            if (chn_settings->muted) {
                                chn_settings->last_chn_gain = 1;
                                chn_settings->curr_chn_gain = 0;
                            } else {
                                chn_settings->last_chn_gain = 0;
                                chn_settings->curr_chn_gain = 1;
                            }

                            chn_settings->mute_state = chn_settings->muted;
                        } else {
                            if (chn_settings->muted) {
                                chn_settings->last_chn_gain = 0;
                                chn_settings->curr_chn_gain = 0;
                            } else {
                                chn_settings->last_chn_gain = 1;
                                chn_settings->curr_chn_gain = 1;
                            }
                        }
                    }
                }

                for (k = 0; k < fas_max_instruments; k += 1) {
                    // preprocess notes
                    pv_note_buffer_len += note_buffer_len;
                    note_buffer_len = curr_notes[pv_note_buffer_len].osc_index;
                    pv_note_buffer_len += 1;
                    s = pv_note_buffer_len;
                    e = s + note_buffer_len;

                    struct _synth_instrument *instrument = &curr_synth.instruments[k];
                    struct _synth_instrument_states *instrument_states = &fas_instrument_states[k];

                    // smooth processing of instrument type
                    if (instrument_states->state == 1) {
                        // old instrument faded off, switch to the new one
                        instrument->type = instrument->next_type;
                        instrument->p0 = instrument->next_p0;
                        instrument->p1 = instrument->next_p1;
                        instrument->p2 = instrument->next_p2;
                        instrument->p3 = instrument->next_p3;
                        instrument->p4 = instrument->next_p4;

                        instrument_states->state = 0;

                        // override current notes data to smoothly switch on and force an instrument note on trigger (to initialize the newly assigned instrument)
                        notesOn(curr_notes, s, e);
                    }

                    if (instrument->type != instrument->next_type) {
                        // let the actual instrument fade off (smooth transition) by inserting note off values
                        notesOff(curr_notes, s, e);
                        // indicate a transition state
                        instrument_states->state = 1;
                    }
                    //

                    int synthesis_method = instrument->type;
                    if (synthesis_method == FAS_ADDITIVE) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            double dummy_int_part;
                            osc->fp1[k][0] = fabs(n->blue);
                            osc->fp1[k][1] = modf(fabs(n->blue), &dummy_int_part);

                            osc->fp2[k][1] = osc->fp2[k][0];
                            osc->fp2[k][0] = n->res;

                            // make sure phase index is randomized and within bounds
                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || trigger_note_on) {
                                osc->phase_index[k] = randf(0, fas_wavetable_size);
                            }
                        }
                    } else if (synthesis_method == FAS_GRANULAR) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            // reset granular envelope; force grains creation
                            if (n->smp_index != n->psmp_index) {
                                unsigned int grain_index = n->osc_index * samples_count + n->smp_index;
                                unsigned int si = curr_synth.bank_settings->h * samples_count;

                                struct grain *gr = &curr_synth.grains[grain_index];

                                for (d = 0; d < gr->density[k]; d += 1) {
                                    gr = &curr_synth.grains[grain_index + (d * si)];

                                    gr->env_index[k] = FAS_ENVS_SIZE;
                                }
                            }

                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || trigger_note_on) {
                                unsigned int grain_index = n->osc_index * samples_count + n->smp_index;
                                unsigned int pgrain_index = n->osc_index * samples_count + n->psmp_index;
                                unsigned int si = curr_synth.bank_settings->h * samples_count;

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
                    } else if (synthesis_method == FAS_BANDPASS) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef WITH_SOUNDPIPE
                            sp_butbp *bpb_l = (sp_butbp *)osc->sp_filters[k][SP_BANDPASS_FILTER_L];

                            osc->fp1[k][0] = bpb_l->bw;
                            osc->fp1[k][1] = osc->bw[k] * fmin(fmax(fabs(n->alpha), 0.000001f), 10.f);
#endif
                        }  
                    } else if (synthesis_method == FAS_FORMANT_SYNTH) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            double dummy_int_part;
                            FAS_FLOAT blue_frac_part = modf(n->blue, &dummy_int_part);

#ifdef WITH_SOUNDPIPE
                            sp_fofilt *fofilt_l = (sp_fofilt *)osc->sp_filters[k][SP_FORMANT_FILTER_L];
                            sp_fofilt *fofilt_r = (sp_fofilt *)osc->sp_filters[k][SP_FORMANT_FILTER_R];
                            fofilt_l->patk = fofilt_r->patk = fofilt_r->atk;
                            fofilt_l->pdec = fofilt_r->pdec = fofilt_r->dec;
                            fofilt_l->tatk = fofilt_r->tatk = fmax(fabs(blue_frac_part) * 60.f, 0.000001f);
                            fofilt_l->tdec = fofilt_r->tdec = fmax(fabs(n->res), 0.000001f);

                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || trigger_note_on) {
                                sp_fofilt_reset(sp, (sp_fofilt *)osc->sp_filters[k][SP_FORMANT_FILTER_L]);
                                sp_fofilt_reset(sp, (sp_fofilt *)osc->sp_filters[k][SP_FORMANT_FILTER_R]);
                            }
#endif
                        }    
                    } else if (synthesis_method == FAS_STRING_RESON) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef WITH_SOUNDPIPE
                            sp_streson *streson_l = (sp_streson *)osc->sp_filters[k][SP_STRES_FILTER_L];
                            sp_streson *streson_r = (sp_streson *)osc->sp_filters[k][SP_STRES_FILTER_R];
                            streson_l->fdbgain = fmax(fabs(n->res), 0.000001f);
                            streson_r->fdbgain = fmax(fabs(n->res), 0.000001f);
#endif
                        }
                    } else if (synthesis_method == FAS_MODAL_SYNTH) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef WITH_SOUNDPIPE
                            sp_mode *mode_l = (sp_mode *)osc->sp_filters[k][SP_MODE_FILTER_L];
                            sp_mode *mode_r = (sp_mode *)osc->sp_filters[k][SP_MODE_FILTER_R];
                            mode_l->q = fmax(fabs(n->alpha), 0.000001f);
                            mode_r->q = fmax(fabs(n->alpha), 0.000001f);
#endif
                        }
                    } else if (synthesis_method == FAS_PHASE_DISTORSION) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef WITH_SOUNDPIPE
                            sp_pdhalf *pdhalf = (sp_pdhalf *)osc->sp_gens[k][SP_PD_GENERATOR];
                            pdhalf->ibipolar = 0;
                            pdhalf->pamount = pdhalf->amount;
                            pdhalf->tamount = fmin(fmax(n->alpha, -1.f), 1.f);
#endif
                        } 
                    } else if (synthesis_method == FAS_INPUT) {
#ifdef WITH_SOUNDTOUCH
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            FAS_FLOAT pv = fmax(0.01f, fabs(n->alpha));

                            osc->fp1[k][0] = fmax(0.01f, fabs(osc->fp1[k][1]));
                            osc->fp1[k][1] = pv;

                            double bint = 0;
                            FAS_FLOAT t = modf(fabs(n->blue), &bint);
                            FAS_FLOAT tv = fmax(0.001f, t);

                            osc->fp1[k][2] = fmax(0.001f, fabs(osc->fp1[k][3]));
                            osc->fp1[k][3] = tv;
                        }
#endif
                    } else if (synthesis_method == FAS_FM) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            double dummy_int_part;

                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || trigger_note_on) {
                                osc->pvalue[k] = 0;

                                osc->fp3[k][0] = modf(fabs(n->pblue), &dummy_int_part);
                                osc->fp4[k][0] = floor(fabs(n->pblue)) / 65536.0;
                                
                                osc->fp1[k][0] = 0;
                                osc->fp1[k][1] = 0;
                            }

                            if (instrument->p0 >= 0 && waves_count > 0) {
                                unsigned int index = instrument->p0 % waves_count;

                                struct sample *smp = &waves[index];

                                osc->wav1[k] = smp->data_l;

                                osc->fp1[k][3] = osc->freq / smp->pitch / ((FAS_FLOAT)fas_sample_rate / (FAS_FLOAT)smp->samplerate);
                                osc->fp2[k][0] = smp->frames;
                            } else {
                                osc->wav1[k] = fas_sine_wavetable;

                                osc->fp1[k][3] = osc->phase_step;
                                osc->fp2[k][0] = fas_wavetable_size;
                            }

                            if (instrument->p1 >= 0 && waves_count > 0) {
                                unsigned int index = (int)instrument->p1 % waves_count;

                                struct sample *smp = &waves[index];

                                osc->wav2[k] = smp->data_l;

                                osc->fp4[k][3] = osc->fp4[k][2];
                                osc->fp4[k][2] = n->alpha / smp->pitch / ((FAS_FLOAT)fas_sample_rate / (FAS_FLOAT)smp->samplerate);
                                osc->fp2[k][1] = smp->frames;
                            } else {
                                osc->wav2[k] = fas_sine_wavetable;

                                osc->fp4[k][3] = osc->fp4[k][2];
                                osc->fp4[k][2] = n->alpha / (FAS_FLOAT)fas_sample_rate * (FAS_FLOAT)fas_wavetable_size;
                                osc->fp2[k][1] = fas_wavetable_size;
                            }

                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || trigger_note_on) {
                                osc->fp4[k][3] = osc->fp4[k][2];
                            }

                            osc->fp3[k][1] = osc->fp3[k][0];
                            osc->fp3[k][0] = modf(fabs(n->blue), &dummy_int_part);

                            osc->fp4[k][1] = osc->fp4[k][0];
                            osc->fp4[k][0] = floor(fabs(n->blue)) / 65536.0;
                        }
                    } else if (synthesis_method == FAS_SUBTRACTIVE) {
                        int filter_type = instrument->p0;
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef WITH_SOUNDPIPE
                            SPFLOAT freq = osc->freq * n->cutoff;
                            SPFLOAT res = fabs(n->res);

                            // clamp to nyquist limit / filter limit
                            freq = fmin(freq, fas_sample_rate / 2 * FAS_FREQ_LIMIT_FACTOR);

                            if (filter_type == 0) {
                                sp_moogladder *spmf = (sp_moogladder *)osc->sp_filters[k][SP_MOOG_FILTER];
                                osc->fp1[k][0] = spmf->freq;
                                osc->fp1[k][1] = freq;
                                osc->fp1[k][2] = spmf->res;
                                osc->fp1[k][3] = res;
                            } else if (filter_type == 1) {
                                sp_diode *spdf = (sp_diode *)osc->sp_filters[k][SP_DIODE_FILTER];
                                osc->fp1[k][0] = spdf->freq;
                                osc->fp1[k][1] = freq;
                                osc->fp1[k][2] = spdf->res;
                                osc->fp1[k][3] = res;
                            } else if (filter_type == 2) {
                                sp_wpkorg35 *spkf = (sp_wpkorg35 *)osc->sp_filters[k][SP_KORG35_FILTER];
                                osc->fp1[k][0] = spkf->cutoff;
                                osc->fp1[k][1] = freq;
                                osc->fp1[k][2] = spkf->res;
                                osc->fp1[k][3] = res * 2;
                            } else if (filter_type == 3) {
                                sp_lpf18 *splf = (sp_lpf18 *)osc->sp_filters[k][SP_LPF18_FILTER];
                                osc->fp1[k][0] = splf->cutoff;
                                osc->fp1[k][1] = freq;
                                osc->fp1[k][2] = splf->res;
                                osc->fp1[k][3] = res;
                            }
#else
                            huovilainen_compute(osc->freq * n->cutoff, n->res, &n->cutoff, &n->res, (FAS_FLOAT)fas_sample_rate);

                            // reset standalone filter on note-off
                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || trigger_note_on) {
                                resetOscillator(osc, k);

                                osc->pvalue[k] = 0.0f;

                                osc->phase_index[k] = randf(0, fas_noise_wavetable_size - 1);
                            }
#endif
                        }
                    } else if (synthesis_method == FAS_PHYSICAL_MODELLING) {
                        int model_type = instrument->p0;
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

#ifdef WITH_SOUNDPIPE
                            if (model_type == 1) {
                                double dummy_int_part;
                                FAS_FLOAT blue_frac_part = modf(fabs(n->blue), &dummy_int_part);

                                sp_drip *drip = (sp_drip *)osc->sp_gens[k][SP_DRIP_GENERATOR];
                                drip->damp = blue_frac_part * 2.f;
                                drip->shake_max = n->res;
                                drip->freq1 = fmin(fabs(round(n->blue)), fas_sample_rate / 2 * FAS_FREQ_LIMIT_FACTOR);
                                drip->freq2 = fmin(fabs(round(n->alpha)), fas_sample_rate / 2 * FAS_FREQ_LIMIT_FACTOR);
                                drip->num_tubes = instrument->p1;
                            } else if (model_type == 2) {
                                double bint_part;
                                FAS_FLOAT blue_frac_part = modf(fabs(n->blue), &bint_part);

                                double aint_part;
                                FAS_FLOAT alpha_frac_part = modf(fabs(n->alpha), &aint_part);

                                sp_bar *bar = (sp_bar *)osc->sp_gens[k][SP_BAR_GENERATOR];
                                bar->scan = blue_frac_part;
                                bar->pos = alpha_frac_part;
                                bar->T30 = bint_part > 1 ? bint_part : 1;
                                bar->wid = (aint_part > 1 ? aint_part : 1) / 1000;
                                bar->vel = instrument->p3;
                            }
#endif

#ifndef WITH_SOUNDPIPE
                            huovilainen_compute(osc->freq * n->cutoff, n->res, &n->cutoff, &n->res, (FAS_FLOAT)fas_sample_rate);
#endif
                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || osc->triggered[k] == 1 || trigger_note_on) {
                                if (model_type == 0) {
                                    karplusTrigger(k, osc, n);

                                    osc->triggered[k] = 0;
                                }
                            }

                            double dummy_int_part;
                            osc->fp1[k][0] = modf(fabs(n->blue), &dummy_int_part);
                        }
                    } else if (synthesis_method == FAS_WAVETABLE_SYNTH) {
                        for (j = s; j < e; j += 1) {
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            double dummy_int_part;

                            if ((n->previous_volume_l <= 0 && n->previous_volume_r <= 0) || (osc->triggered[k] == 1 && instrument->p0 == 1) || trigger_note_on || n->res != osc->fp4[k][0]) {
                                int start_index = (int)fabs(round(n->blue)) % waves_count;
                                int stop_index = (int)fabs(round(n->alpha)) % waves_count;

                                if (n->blue > 0) {
                                    osc->fp1[k][0] = start_index;
                                    osc->fp2[k][0] = (start_index + 1) % waves_count;
                                } else {
                                    osc->fp1[k][0] = stop_index;
                                    osc->fp2[k][0] = stop_index - 1;
                                    if (osc->fp2[k][0] < 0) {
                                        osc->fp2[k][0] = start_index;
                                    }
                                }

                                struct sample *smp = &waves[(unsigned int)osc->fp1[k][0]];

                                osc->fp1[k][1] = 0;
                                osc->fp1[k][2] = osc->freq / smp->pitch / ((FAS_FLOAT)fas_sample_rate / (FAS_FLOAT)smp->samplerate);
                                osc->fp1[k][3] = 0;
                                
                                struct sample *nsmp = &waves[(unsigned int)osc->fp2[k][0]];

                                osc->fp2[k][1] = 0;
                                osc->fp2[k][2] = osc->freq / nsmp->pitch / ((FAS_FLOAT)fas_sample_rate / (FAS_FLOAT)nsmp->samplerate);

                                osc->triggered[k] = 0;

                                osc->fp3[k][0] = modf(fabs(n->blue), &dummy_int_part);

                                osc->fp4[k][0] = n->res;
                            }
                            
                            osc->fp4[k][0] = n->res;

                            osc->fp3[k][1] = osc->fp3[k][0];
                            osc->fp3[k][0] = modf(fabs(n->blue), &dummy_int_part);
                        }
#ifdef WITH_FAUST
                    } else if (synthesis_method == FAS_FAUST) {
                        for (j = s; j < e; j += 1) {
                            // update notes related parameters
                            struct note *n = &curr_notes[j];

                            struct oscillator *osc = &curr_synth.oscillators[n->osc_index];

                            if (osc->faust_gens_len) {
                                int faust_dsp_index = instrument->p0 % osc->faust_gens_len;

                                struct _fas_faust_dsp *fas_faust_dsp = osc->faust_gens[k][faust_dsp_index];

                                // update Faust DSP params
                                struct _fas_faust_ui_control *ctrl = fas_faust_dsp->controls;

                                struct _fas_faust_ui_control *tmp;
                                tmp = getFaustControl(ctrl, "fs_pr");
                                if (tmp) {
                                    *tmp->zone = fmax(tmp->min, fmin(tmp->max, n->previous_volume_l));
                                }

                                tmp = getFaustControl(ctrl, "fs_pg");
                                if (tmp) {
                                    *tmp->zone = fmax(tmp->min, fmin(tmp->max, n->previous_volume_r));
                                }

                                tmp = getFaustControl(ctrl, "fs_r");
                                if (tmp) {
                                    *tmp->zone = fmax(tmp->min, fmin(tmp->max, n->volume_l));
                                }

                                tmp = getFaustControl(ctrl, "fs_g");
                                if (tmp) {
                                    *tmp->zone = fmax(tmp->min, fmin(tmp->max, n->volume_r));
                                }

                                tmp = getFaustControl(ctrl, "fs_b");
                                if (tmp) {
                                    *tmp->zone = fmax(tmp->min, fmin(tmp->max, n->blue));
                                }

                                tmp = getFaustControl(ctrl, "fs_a");
                                if (tmp) {
                                    *tmp->zone = fmax(tmp->min, fmin(tmp->max, n->alpha));
                                }
                            }
                        }
#endif
                    } else if (synthesis_method == FAS_VOID) {
                        break;
                    }
                }


                trigger_note_on = 0;

#ifdef DEBUG
    frames_read += 1;
    if ((frames_read % 64) == 0) {
        printf("%lu frames read\n", frames_read);
        fflush(stdout);
    }
#endif

#ifdef PROFILE
#ifndef WITH_JACK
    printf("PortAudio stream CPU load : %f\n", Pa_GetStreamCpuLoad(stream));
    fflush(stdout);
#endif
#endif
            } else {
                // allow some frame drop, hold the current note events to FAS_MAX_DROP if that happen
                // ensure smooth audio in most situations (the only downside : it may sound delayed, latency impact depend on how many frames are dropped)
                fas_drop_counter += 1;
                if (fas_drop_counter >= fas_max_drop) {
                    if (curr_notes != dummy_notes) {
                        LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(curr_freelist_frames_data->fe, curr_freelist_frames_data);
                        lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &curr_freelist_frames_data->fe);
                    }
                    curr_notes = dummy_notes;

                    note_buffer_len = 0;

                    fas_drop_counter = 0;
                }
            }
        }
    }

    return 0;
}

#ifdef WITH_JACK
int jackCallback (jack_nframes_t nframes, void *arg) {
    cpu_load_measurer.measurementStartTime = get_time();

    int i = 0;
    for (i = 0; i < fas_input_channels; i += 1) {
        jack_in[i] = jack_port_get_buffer (input_ports[i], nframes);
    }

    for (i = 0; i < fas_output_channels; i += 1) {
        jack_out[i] = jack_port_get_buffer (output_ports[i], nframes);

        memset(jack_out[i], 0, sizeof(float) * nframes);
    }

    int r = audioCallback((float **)jack_in, (float **)jack_out, nframes);

    // compute CPU load (come from PortAudio)
    double measurementEndTime = get_time();

    double secondsFor100Percent = nframes * cpu_load_measurer.samplingPeriod;

    if (secondsFor100Percent > 0) {
        double measuredLoad = (measurementEndTime - cpu_load_measurer.measurementStartTime) / secondsFor100Percent;

#define LOWPASS_COEFFICIENT_0   (0.9)
#define LOWPASS_COEFFICIENT_1   (0.99999 - LOWPASS_COEFFICIENT_0)

        cpu_load_measurer.averageLoad = (LOWPASS_COEFFICIENT_0 * cpu_load_measurer.averageLoad) +
                                (LOWPASS_COEFFICIENT_1 * measuredLoad);

        cpu_load = (int)(cpu_load_measurer.averageLoad * 100);
    }

    return r;
}
#else
static int paCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long nframes,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *data) {
#ifdef INTERLEAVED_SAMPLE_FORMAT
    void *input_buffer = (void *)inputBuffer;
    memset(outputBuffer, 0, nframes * sizeof(float) * fas_output_channels);
    return audioCallback((float *)input_buffer, (float *)outputBuffer, nframes);
#else
    for (int i = 0; i < fas_output_channels; i += 1) {
        memset(&outputBuffer[i], 0, nframes * sizeof(float));
    }

    return audioCallback((float **)inputBuffer, (float **)outputBuffer, nframes);
#endif
}
#endif

void audioFlushThenPause() {
    audio_thread_state = FAS_AUDIO_DO_FLUSH_THEN_PAUSE;

    while (audio_thread_state != FAS_AUDIO_PAUSE);
}

void audioPause() {
    audio_thread_state = FAS_AUDIO_DO_PAUSE;

    while (audio_thread_state != FAS_AUDIO_PAUSE);
}

void audioPlay() {
    if (!fas_paused_by_client) {
        audio_thread_state = FAS_AUDIO_DO_PLAY;
    }
}

/**
 * free & pre-allocate a pool of slice frames data based on given height
 **/
void setHeight(unsigned int new_height) {
    struct lfds720_freelist_n_element *fe;
    struct _freelist_frames_data *freelist_frames_data;

    while (lfds720_freelist_n_threadsafe_pop(&freelist_frames, NULL, &fe)) {
        freelist_frames_data = LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT(*fe);

        free(freelist_frames_data->data);
    }

    unsigned int nc = (new_height + 1) * fas_max_instruments + sizeof(unsigned int);

    for (unsigned int i = 0; i < fas_frames_queue_size; i += 1) {
        ffd[i].data = malloc(sizeof(struct note) * nc);

        LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(ffd[i].fe, &ffd[i]);
        lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &ffd[i].fe);
    }
}

void initCommandsPool() {
    struct lfds720_freelist_n_element *fe;
    struct _freelist_synth_commands *freelist_synth_command;

    while (lfds720_freelist_n_threadsafe_pop(&freelist_commands, NULL, &fe)) {
        freelist_synth_command = LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT(*fe);

        free(freelist_synth_command->data);
    }

    for (unsigned int i = 0; i < fas_commands_queue_size; i += 1) {
        fsc[i].data = malloc(sizeof(struct _synth_command));

        LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(fsc[i].fe, &fsc[i]);
        lfds720_freelist_n_threadsafe_push(&freelist_commands, NULL, &fsc[i].fe);
    }
}

struct _freelist_synth_commands *getSynthCommandFreelist() {
    struct lfds720_freelist_n_element *fe;
    int pop_result = lfds720_freelist_n_threadsafe_pop(&freelist_commands, NULL, &fe);
    if (pop_result == 0) {
#ifdef DEBUG
        printf("getSynthCommand failed, commands pool is empty.\n");
        fflush(stdout);
#endif

        return NULL;
    }

    return LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT(*fe);
}

void freeRender() {
    if (fas_render_buffer) {
        free(fas_render_buffer);
        fas_render_buffer = NULL;
    }
}

void initRender(unsigned int h) {
    if (fas_render_target) {
        freeRender();

        fas_render_buffer = calloc(1, sizeof(unsigned char) * fas_render_width * h * 4 * frame_data_count);

        fas_render_frame_counter = fas_render_width;
    }
}

void render(struct user_session_data *usd, void *data, unsigned int channels) {
    if (fas_render_target) {
        if (fas_render_frame_counter == fas_render_width) {
            fas_render_frame_counter = 0;

            static char filename_buffer[1024];
            int error_code;

            error_code = snprintf(filename_buffer, 1024, "out/%s/%lu.png", fas_render_target, fas_render_counter);
            if (error_code < 0) {
                printf("render: sprintf error; ignoring render frame.");
            } else {
                error_code = lodepng_encode32_file(filename_buffer, fas_render_buffer, fas_render_width, usd->synth_h);
                if (error_code) {
                    printf("render: lodepng_encode32_file %u: %s\n", error_code, lodepng_error_text(error_code));
                }
            }

            fas_render_counter += 1;
        }

        // copy frame data to buffer
        unsigned int frame_data_index = 8;
        unsigned int li = 0, ri = 1;
        unsigned int i, j, y, index;
        size_t data_length = usd->expected_frame_length;

        if (usd->frame_data_size == sizeof(float)) {
            frame_data_index = 2;

            data_length /= usd->frame_data_size;
        }

        for (j = 0; j < channels; j += 1) {
            y = usd->synth_h - 1;

            for (i = 0; i < data_length; i += 4) {
                index = (fas_render_frame_counter + y * fas_render_width) * 4;

                if (usd->frame_data_size == sizeof(float)) {
                    float *cdata = (float *)data;

                    fas_render_buffer[index] = fmin(cdata[frame_data_index] * 255.0f, 255.0f);
                    fas_render_buffer[index + 1] = fmin(cdata[frame_data_index + 1] * 255.0f, 255.0f);
                    fas_render_buffer[index + 2] = fmin(cdata[frame_data_index + 2] * 255.0f, 255.0f);
                    fas_render_buffer[index + 3] = fmin(cdata[frame_data_index + 3] * 255.0f, 255.0f);
                } else {
                    unsigned char *dst = fas_render_buffer + index;
                    unsigned char *src = (unsigned char *)data + frame_data_index;

                    memcpy(dst, src, 4);
                }

                y -= 1;

                frame_data_index += 4;
            }
        }

        fas_render_frame_counter += 1;
    }
}

void freeUserSynthChnFxSettings(double ***synth_chn_fx_settings) {
    size_t i, j;
    if (synth_chn_fx_settings) {
        for (i = 0; i < fas_max_channels; i += 1) {
            for (j = 0; j < FAS_MAX_FX_SLOTS; j += 1) {
                free(synth_chn_fx_settings[i][j]);
            }
            free(synth_chn_fx_settings[i]);
        }
        free(synth_chn_fx_settings);
    }
}

void sendPerformances(struct lws *wsi, double latency) {
    static unsigned char p_load[LWS_SEND_BUFFER_PRE_PADDING + sizeof(int) * 2 + sizeof(double) + LWS_SEND_BUFFER_POST_PADDING];
    p_load[LWS_SEND_BUFFER_PRE_PADDING] = 0; // packet flag
    p_load[LWS_SEND_BUFFER_PRE_PADDING + sizeof(int)] = 0;
    p_load[LWS_SEND_BUFFER_PRE_PADDING + sizeof(double)] = 0;
#ifdef WITH_JACK
    int stream_load = cpu_load;
#else
    int stream_load = (int)(Pa_GetStreamCpuLoad(stream) * 100);
#endif
    memcpy(&p_load[LWS_SEND_BUFFER_PRE_PADDING + sizeof(int)], &stream_load, sizeof(int));
    memcpy(&p_load[LWS_SEND_BUFFER_PRE_PADDING + sizeof(int) * 2], &latency, sizeof(double));
    lws_write(wsi, &p_load[LWS_SEND_BUFFER_PRE_PADDING], sizeof(int) * 2 + sizeof(double), LWS_WRITE_BINARY);

    time(&stream_load_begin);
}

int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len) {
    LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE;

    struct user_session_data *usd = (struct user_session_data *)user;
    size_t n, m, i;
    unsigned char pid;
    int fd;
    int free_prev_data = 0;
    size_t remaining_payload;
    int is_final_fragment;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            // disallow more than one client
            if (clients) {
                printf("%s (%s) connection refused. (too many clients)\n",
                    usd->peer_name, usd->peer_ip);
                fflush(stdout);

                return -1;
            }
            
            // initialize a local copy of synth channel fx settings with the total number of parameters of the _synth_fx_settings struct
            usd->synth_chn_fx_settings = calloc(fas_max_channels, sizeof(double **));
            if (!usd->synth_chn_fx_settings) {
                printf("synth_chn_fx_settings calloc failed, connection refused\n");
                fflush(stdout);

                return -1;
            }

            for (n = 0; n < fas_max_channels; n += 1) {
                usd->synth_chn_fx_settings[n] = calloc(FAS_MAX_FX_SLOTS, sizeof(double *));
                if (!usd->synth_chn_fx_settings[n]) {
                    for (i = 0; i < n; i += 1) {
                        free(usd->synth_chn_fx_settings[i]);
                        free(usd->synth_chn_fx_settings);

                        printf("synth_chn_fx_settings[%lu] calloc failed, connection refused\n", n);
                        fflush(stdout);

                        return -1;
                    }
                }
                for (m = 0; m < FAS_MAX_FX_SLOTS; m += 1) {
                    usd->synth_chn_fx_settings[n][m] = calloc(FAS_MAX_FX_PARAMETERS, sizeof(double));
                    // TODO : check calloc return value
                }
            }

            // local instruments copy
            usd->instruments = calloc(fas_max_instruments, sizeof(struct _synth_instrument));
            if (!usd->instruments) {
                freeUserSynthChnFxSettings(usd->synth_chn_fx_settings);

                printf("instruments calloc failed, connection refused\n");
                fflush(stdout);
            
                return -1;
            }

            clients += 1;

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

            usd->connected = 1;

            usd->synth_h = 0;
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

#ifdef DEBUG_NETWORK
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

#ifdef DEBUG_NETWORK
if (remaining_payload != 0) {
    printf("Remaining packet payload: %lu\n", remaining_payload);
}
#endif

            if (is_final_fragment) {
#ifdef DEBUG_NETWORK
    printf("Full packet received, length: %lu\n", usd->packet_len);
#endif
                pid = usd->packet[0];

#ifdef DEBUG_NETWORK
    printf("Packet id: %u\n", pid);
    fflush(stdout);
#endif

                if (pid == BANK_SETTINGS) {
                    audioFlushThenPause();

                    // flush all waiting data
                    clearQueues();

                    // copy new settings
                    memcpy(curr_synth.bank_settings, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(struct _bank_settings));

#ifdef DEBUG
    printf("BANK_SETTINGS : %u, %u, %u, %f\n", curr_synth.bank_settings->h,
        curr_synth.bank_settings->octave, curr_synth.bank_settings->data_type, curr_synth.bank_settings->base_frequency);
#endif

                    // free grains & oscillator banks
                    freeGrains(&curr_synth.grains, samples_count, fas_max_instruments, usd->synth_h, fas_granular_max_density);

                    curr_synth.oscillators = freeOscillatorsBank(&curr_synth.oscillators, usd->synth_h, fas_max_instruments);

                    // pre-compute frames size (aka notes slice data)
                    usd->frame_data_size = curr_synth.bank_settings->data_type ? sizeof(float) : sizeof(unsigned char);

                    usd->expected_frame_length = 4 * usd->frame_data_size * curr_synth.bank_settings->h;
                    usd->expected_max_frame_length = 4 * usd->frame_data_size * curr_synth.bank_settings->h * fas_max_instruments;
                    size_t max_frame_data_len = usd->expected_frame_length * fas_max_instruments + sizeof(unsigned int);

                    // free frames data state
                    free(usd->frame_data);
                    free(usd->prev_frame_data);

                    usd->frame_data = NULL;
                    usd->prev_frame_data = NULL;

                    usd->frame_data = calloc(max_frame_data_len, usd->frame_data_size);
                    usd->prev_frame_data = calloc(max_frame_data_len, usd->frame_data_size);
                    if (usd->prev_frame_data == NULL || usd->frame_data == NULL) {
                        printf("BANK_SETTINGS : frame_data / prev_frame_data calloc failed.");

                        free(usd->frame_data);
                        free(usd->prev_frame_data);

                        goto free_packet;
                    }

                    usd->oscillators = freeOscillatorsBank(&usd->oscillators, usd->synth_h, fas_max_instruments);

                    usd->synth_h = curr_synth.bank_settings->h;

                    setHeight(usd->synth_h);

                    curr_synth.oscillators = createOscillatorsBank(
#ifdef WITH_SOUNDPIPE
                        sp,
#endif
                        curr_synth.bank_settings->h,
                        curr_synth.bank_settings->base_frequency, curr_synth.bank_settings->octave, fas_sample_rate, fas_sine_wavetable, fas_wavetable_size, fas_max_instruments);
                        
#ifdef WITH_FAUST

                    createFaustGenerators(
                        fas_faust_gens,
                        curr_synth.oscillators,
                        curr_synth.bank_settings->h,
                        fas_sample_rate,
                        fas_max_instruments
                    );
#endif

                    // pre-compute grains data
                    curr_synth.grains = createGrains(&samples, samples_count, usd->synth_h, curr_synth.bank_settings->base_frequency, curr_synth.bank_settings->octave, fas_sample_rate, fas_max_instruments, fas_granular_max_density);

                    //initRender(usd->synth_h);

                    initializeSynthChnSettings();

                    for (i = 0; i < fas_max_instruments; i += 1) {
                        curr_synth.instruments[i].type = FAS_VOID;
                    }

                    audioPlay();
                } else if (pid == FRAME_DATA) {
#ifdef DEBUG_FRAME_DATA
    printf("FRAME_DATA\n");

    static unsigned int frame_data_length[1];
    memcpy(&frame_data_length, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(frame_data_length));
    printf("Number of instruments in the frame: %u\n", *frame_data_length);
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

                    // compute latency between frames & treshold stream rate to avoid unecessary computations
                    uint64_t nowtime = ns();
                    double time_between_frames_ms = (double)(nowtime - frame_sync.lasttime) / 1000000UL;
                    frame_sync.lasttime = nowtime;

                    frame_sync.acc_time += time_between_frames_ms;
                    if (frame_sync.acc_time < note_time * 1000) {
#ifdef DEBUG_FRAME_DATA
                        printf("Skipping a frame. (< treshold)\n");
                        fflush(stdout);
#endif
                        goto free_packet;
                    } else {
#ifdef DEBUG_FRAME_DATA
                        printf("Frame latency %fms\nFrame overall time %fms\n", time_between_frames_ms, frame_sync.acc_time);
                        fflush(stdout);
#endif

                        frame_sync.acc_time = 0;

                        curr_synth.curr_sample = 0;
                    }

#ifdef DEBUG_FRAME_DATA
    lfds720_pal_uint_t frames_data_freelist_count;
    lfds720_freelist_n_query(&freelist_frames, LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *)&frames_data_freelist_count);
    printf("frames_data_freelist_count : %llu\n", frames_data_freelist_count);
#endif

                    struct lfds720_freelist_n_element *fe;
                    struct _freelist_frames_data *freelist_frames_data;
                    int pop_result = lfds720_freelist_n_threadsafe_pop(&freelist_frames, NULL, &fe);
                    if (pop_result == 0) {
#ifdef DEBUG
                        printf("Skipping a frame, notes buffer freelist is empty.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    memcpy(usd->prev_frame_data, &usd->frame_data[0], usd->expected_max_frame_length);

                    unsigned int instruments[1];
                    memcpy(&instruments, &usd->packet[PACKET_HEADER_LENGTH], sizeof(instruments));

                    if ((*instruments) >= fas_max_instruments) {
#ifdef DEBUG_FRAME_DATA
                        printf("Frame instruments > Max instruments. (%i instrument ignored)\n", (*instruments) - fas_max_instruments);
                        fflush(stdout);
#endif

                        (*instruments) = fas_max_instruments;

                        memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->expected_frame_length * (*instruments) - PACKET_HEADER_LENGTH);
                    } else if ((*instruments) < fas_max_instruments) {
                        memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->packet_len - PACKET_HEADER_LENGTH);
                        memset(&usd->frame_data[usd->expected_frame_length * (*instruments)], 0, usd->expected_frame_length * (fas_max_instruments - (*instruments)));
#ifdef DEBUG_FRAME_DATA
                        printf("Frame instruments (%i) < Max instruments.\n", instruments[0]);
                        fflush(stdout);
#endif
                    } else {
                        memcpy(usd->frame_data, &usd->packet[PACKET_HEADER_LENGTH], usd->packet_len - PACKET_HEADER_LENGTH);
                    }

                    //render(usd, usd->frame_data, (*instruments));

                    freelist_frames_data = LFDS720_FREELIST_N_GET_VALUE_FROM_ELEMENT(*fe);

                    memset(freelist_frames_data->data, 0, sizeof(struct note) * (usd->synth_h + 1) * fas_max_instruments + sizeof(unsigned int));

                    fillNotesBuffer(samples_count_m1, waves_count_m1, fas_granular_max_density, (*instruments), usd->frame_data_size,
                                    freelist_frames_data->data, usd->synth_h, usd->expected_frame_length,
                                    usd->prev_frame_data, usd->frame_data);

                    struct _freelist_frames_data *overwritten_notes = NULL;
                    lfds720_ringbuffer_n_write(&rs, (void *) (lfds720_pal_uint_t) freelist_frames_data, NULL, &overwrite_occurred_flag, (void *)&overwritten_notes, NULL);
                    if (overwrite_occurred_flag == LFDS720_MISC_FLAG_RAISED) {
                        // okay, push it back!
                        LFDS720_FREELIST_N_SET_VALUE_IN_ELEMENT(overwritten_notes->fe, overwritten_notes);
                        lfds720_freelist_n_threadsafe_push(&freelist_frames, NULL, &overwritten_notes->fe);
                    }

                    // check & send stream informations (load & latency)
                    time_t stream_load_end;
                    time(&stream_load_end);
                    double stream_load_time = difftime(stream_load_end,stream_load_begin);
                    if (stream_load_time > fas_stream_infos_send_delay) {
                        sendPerformances(wsi, time_between_frames_ms);
                    }
                } else if (pid == SYNTH_SETTINGS) {
                    uint32_t target = 0;
                    double value = 0;

                    struct _freelist_synth_commands *freelist_synth_command = getSynthCommandFreelist();
                    if (freelist_synth_command == NULL) {
#ifdef DEBUG
                        printf("Skipping synth settings change, commands pool is empty.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    freelist_synth_command->data->type = FAS_CMD_SYNTH_SETTINGS;
                    memcpy(&target, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(target));
                    memcpy(&value, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 8], sizeof(value));

                    freelist_synth_command->data->value[0] = target;
                    freelist_synth_command->data->value[1] = value;

                    if (lfds720_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)freelist_synth_command) == 0) {
#ifdef DEBUG
                        printf("Skipping synth settings change, commands queue is full.\n");
                        fflush(stdout);
#endif

                        goto free_packet; 
                    }
                } else if (pid == CHN_SETTINGS) {
                    uint32_t chn = 0;
                    uint32_t target = 0;
                    double value = 0;

                    memcpy(&chn, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(chn));
                    if (chn >= fas_max_channels) {
#ifdef DEBUG
                        printf("Skipping chn fx settings change, chn >= fas_max_channels.\n");
                        fflush(stdout);
#endif
                        goto free_packet;
                    }

                    struct _freelist_synth_commands *freelist_synth_command = getSynthCommandFreelist();
                    if (freelist_synth_command == NULL) {
#ifdef DEBUG
                        printf("Skipping chn settings change, commands pool is empty.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    freelist_synth_command->data->type = FAS_CMD_CHN_SETTINGS;
                    memcpy(&target, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 4], sizeof(target));
                    memcpy(&value, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 8], sizeof(value));

                    freelist_synth_command->data->value[0] = chn;
                    freelist_synth_command->data->value[1] = target;
                    freelist_synth_command->data->value[2] = value;

                    if (lfds720_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)freelist_synth_command) == 0) {
#ifdef DEBUG
                        printf("Skipping chn settings change, commands queue is full.\n");
                        fflush(stdout);
#endif

                        goto free_packet; 
                    }
                } else if (pid == INSTRUMENT_SETTINGS) {
                    uint32_t instrument = 0;
                    uint32_t target = 0;
                    double value = 0;

                    memcpy(&instrument, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(instrument));
                    if (instrument >= fas_max_instruments) {
#ifdef DEBUG
                        printf("Skipping instrument settings change, instrument index >= fas_max_instruments.\n");
                        fflush(stdout);
#endif
                        goto free_packet;
                    }

                    struct _freelist_synth_commands *freelist_synth_command = getSynthCommandFreelist();
                    if (freelist_synth_command == NULL) {
#ifdef DEBUG
                        printf("Skipping instrument settings change, commands pool is empty.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    freelist_synth_command->data->type = FAS_CMD_INSTRUMENT_SETTINGS;
                    memcpy(&target, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 4], sizeof(target));
                    memcpy(&value, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 8], sizeof(value));

                    if (target == 0) {
                        usd->instruments[instrument].type = value;
                    }

                    if (target == 3) {
                        usd->instruments[instrument].p0 = value;
                    }

                    if (target == 4) {
                        usd->instruments[instrument].p1 = value;
                    }

                    if (target == 5) {
                        usd->instruments[instrument].p2 = value;
                    }

                    // special case for synthesis type which require re-initialization for this parameter
                    if (target == 4 && usd->instruments[instrument].type == FAS_SPECTRAL) {
#ifdef DEBUG
                        printf("Spectral synthesis window change. (%i)\n", (uint32_t)value);
                        fflush(stdout);
#endif

                        audioPause();
                        createInstrumentState(&fas_instrument_states[instrument], (uint32_t)value);
                        audioPlay();
                    } else if (target == 5 &&
                        usd->instruments[instrument].p0 == 1 &&
                        usd->instruments[instrument].type == FAS_PHYSICAL_MODELLING) {
#ifdef DEBUG
                        printf("Physical modelling droplet deattack change. (%f)\n", value);
                        fflush(stdout);
#endif

                        audioPause();
                        updateOscillatorBank(
#ifdef WITH_SOUNDPIPE
                            sp,
#endif
                            &usd->oscillators, usd->synth_h, fas_max_instruments, fas_sample_rate, 0, value, 0);
                        audioPlay();
                    } else if (target == 4 &&
                        usd->instruments[instrument].p0 == 2 &&
                        usd->instruments[instrument].type == FAS_PHYSICAL_MODELLING) {
#ifdef DEBUG
                        printf("Physical modelling bar bcL change. (%f)\n", value);
                        fflush(stdout);
#endif

                        audioPause();
                        updateOscillatorBank(
#ifdef WITH_SOUNDPIPE
                            sp,
#endif
                            &usd->oscillators, usd->synth_h, fas_max_instruments, fas_sample_rate, 1, value, usd->instruments[instrument].p2);
                        audioPlay();
                    } else if (target == 5 &&
                        usd->instruments[instrument].p0 == 2 &&
                        usd->instruments[instrument].type == FAS_PHYSICAL_MODELLING) {
#ifdef DEBUG
                        printf("Physical modelling bar bcR change. (%f)\n", value);
                        fflush(stdout);
#endif

                        audioPause();
                        updateOscillatorBank(
#ifdef WITH_SOUNDPIPE
                            sp,
#endif
                            &usd->oscillators, usd->synth_h, fas_max_instruments, fas_sample_rate, 1, usd->instruments[instrument].p1, value);
                        audioPlay();
                    }

                    freelist_synth_command->data->value[0] = instrument;
                    freelist_synth_command->data->value[1] = target;
                    freelist_synth_command->data->value[2] = value;

                    if (lfds720_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)freelist_synth_command) == 0) {
#ifdef DEBUG
                        printf("Skipping instrument settings change, commands queue is full.\n");
                        fflush(stdout);
#endif

                        goto free_packet; 
                    }
                } else if (pid == CHN_FX_SETTINGS) {
                    uint32_t chn = 0;
                    uint32_t fx_slot = 0;
                    uint32_t target = 0;
                    double value = 0;

                    memcpy(&chn, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(chn));
                    if (chn >= fas_max_channels) {
#ifdef DEBUG
                        printf("Skipping chn fx settings change, chn >= fas_max_channels.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    struct _freelist_synth_commands *freelist_synth_command = getSynthCommandFreelist();
                    if (freelist_synth_command == NULL) {
#ifdef DEBUG
                        printf("Skipping chn fx settings change, commands pool is empty.\n");
                        fflush(stdout);
#endif

                        goto free_packet;
                    }

                    freelist_synth_command->data->type = FAS_CMD_CHN_FX_SETTINGS;
                    memcpy(&chn, &((char *) usd->packet)[PACKET_HEADER_LENGTH], sizeof(chn));
                    memcpy(&fx_slot, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 4], sizeof(fx_slot));
                    memcpy(&target, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 8], sizeof(target));
                    memcpy(&value, &((char *) usd->packet)[PACKET_HEADER_LENGTH + 16], sizeof(value));

                    usd->synth_chn_fx_settings[chn][fx_slot][target] = value;

                    // special case for effects which require re-initialization for this parameter
                    unsigned int curr_fx_id = usd->synth_chn_fx_settings[chn][fx_slot][0];

#ifdef WITH_SOUNDPIPE
                    if ((curr_fx_id == FX_CONV && (target == 2 || target == 3 || target == 4 || target == 5)) ||
                        (curr_fx_id == FX_DELAY && (target == 2 || target == 4)) ||
                        (curr_fx_id == FX_SMOOTH_DELAY && (target == 2 || target == 3 || target == 6) || target == 7) ||
                        (curr_fx_id == FX_COMB && (target == 2 || target == 4)) &&
                        (curr_fx_id == FX_LPC && target == 2) &&
                        (curr_fx_id == FX_WAVESET && target == 2)) {
                        audioPause();

                        double fp0 = usd->synth_chn_fx_settings[chn][fx_slot][2];
                        double fp1 = usd->synth_chn_fx_settings[chn][fx_slot][3];
                        double fp2 = usd->synth_chn_fx_settings[chn][fx_slot][4];
                        double fp3 = usd->synth_chn_fx_settings[chn][fx_slot][5];
                        double fp4 = usd->synth_chn_fx_settings[chn][fx_slot][6];
                        double fp5 = usd->synth_chn_fx_settings[chn][fx_slot][7];
                        double fp6 = usd->synth_chn_fx_settings[chn][fx_slot][8];
                        double fp7 = usd->synth_chn_fx_settings[chn][fx_slot][9];

                        if (target == 2) {
                            if (fp0 <= 0 && curr_fx_id != FX_CONV) {
                                fp0 = usd->synth_chn_fx_settings[chn][fx_slot][target] = 1;
                            }

                            if (fp1 <= 0 && curr_fx_id == FX_CONV) {
                                fp1 = usd->synth_chn_fx_settings[chn][fx_slot][3] = 2048;
                            }
                        } else if (target == 4) {
                            if (fp2 <= 0 && curr_fx_id != FX_CONV) {
                                fp2 = usd->synth_chn_fx_settings[chn][fx_slot][target] = 1;
                            }

                            if (fp3 <= 0 && curr_fx_id == FX_CONV) {
                                fp3 = usd->synth_chn_fx_settings[chn][fx_slot][5] = 2048;
                            }
                        }

                        if (curr_fx_id == FX_CONV) {
                            if (target == 2 || target == 3) {
                                if (target == 3) {
                                    if (!isPowerOfTwo((int)value) || !isValidConvPart((int)value)) {
                                        value = 1024;
                                    }

                                    fp1 = usd->synth_chn_fx_settings[chn][fx_slot][target] = value;
                                }

                                resetConvolution(sp, synth_fx[chn], impulses, impulses_count, fx_slot, 0, fp0, fp1);
                            } else if (target == 4 || target == 5) {
                                if (target == 5) {
                                    if (!isPowerOfTwo((int)value) || !isValidConvPart((int)value)) {
                                        value = 1024;
                                    }

                                    fp3 = usd->synth_chn_fx_settings[chn][fx_slot][target] = value;
                                }

                                resetConvolution(sp, synth_fx[chn], impulses, impulses_count, fx_slot, 1, fp2, fp3);
                            }
                        } else if (curr_fx_id == FX_DELAY) {
                            if (target == 2) {
                                resetDelays(sp, synth_fx[chn], fx_slot, 0, 0, fp0, fp1, 0, 0);
                            } else if (target == 4) {
                                resetDelays(sp, synth_fx[chn], fx_slot, 0, 1, fp2, fp3, 0, 0);
                            }
                        } else if (curr_fx_id == FX_SMOOTH_DELAY) {
                            if (target == 2 || target == 3) {
                                resetDelays(sp, synth_fx[chn], fx_slot, 1, 0, fp0, fp1, fp2, fp3);
                            } else if (target == 6 || target == 7) {
                                resetDelays(sp, synth_fx[chn], fx_slot, 1, 1, fp4, fp5, fp6, fp7);
                            }
                        } else if (curr_fx_id == FX_COMB) {
                            if (target == 2) {
                                resetComb(sp, synth_fx[chn], fx_slot, 0, fp0, fp1);
                            } else if (target == 4) {
                                resetComb(sp, synth_fx[chn], fx_slot, 1, fp2, fp3);
                            }
                        } else if (curr_fx_id == FX_LPC) {
                            if (target == 2) {
                                resetLpc(sp, synth_fx[chn], fx_slot, fp0);
                            }
                        } else if (curr_fx_id == FX_WAVESET) {
                            if (target == 2) {
                                resetWaveset(sp, synth_fx[chn], fx_slot, fp0);
                            }
                        }

                        audioPlay();
                    }
#endif

                    freelist_synth_command->data->value[0] = chn;
                    freelist_synth_command->data->value[1] = fx_slot;
                    freelist_synth_command->data->value[2] = target;
                    freelist_synth_command->data->value[3] = value;

                    if (lfds720_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)freelist_synth_command) == 0) {
#ifdef DEBUG
                        printf("Skipping chn fx settings change, commands queue is full.\n");
                        fflush(stdout);
#endif

                        goto free_packet; 
                    }
                } else if (pid == ACTION) {
                    static unsigned char action_type[1];
                    memcpy(&action_type, &((char *) usd->packet)[PACKET_HEADER_LENGTH - 7], sizeof(unsigned char));

#ifdef DEBUG
printf("ACTION : type %i\n", action_type[0]);
fflush(stdout);
#endif

                    if (action_type[0] == FAS_ACTION_WAVES_RELOAD) { // RELOAD waves
                        audioPause();

                        free_samples(&waves, waves_count);

#ifdef WITH_SOUNDPIPE
                        waves_count = load_samples(sp, &waves, fas_waves_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#else
                        waves_count = load_samples(&waves, fas_waves_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#endif

                        if (waves_count > 0) {
                            waves_count_m1 = waves_count - 1;
                        } else {
                            waves_count_m1 = 0;
                        }

                        audioPlay();
                    } else if (action_type[0] == FAS_ACTION_IMPULSES_RELOAD) { // RELOAD IMPULSES
                        audioPause();

                        free_samples(&impulses, impulses_count);

#ifdef WITH_SOUNDPIPE
                        impulses_count = load_samples(sp, &impulses, fas_impulses_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#else
                        impulses_count = load_samples(&impulses, fas_impulses_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#endif
                        if (impulses_count > 0) {
                            impulses_count_m1 = impulses_count - 1;
                        } else {
                            impulses_count_m1 = 0;
                        }

                        for (n = 0; n < fas_max_channels; n += 1) {
                            resetConvolutions(
#ifdef WITH_SOUNDPIPE
                                    sp,
#endif                    
                                    synth_fx[n], &curr_synth.chn_settings[n], impulses, impulses_count);
                        }

                        audioPlay();
                    } else if (action_type[0] == FAS_ACTION_SAMPLES_RELOAD) { // RELOAD SAMPLES
                        audioPause();

                        freeGrains(&curr_synth.grains, samples_count, fas_max_instruments, usd->synth_h, fas_granular_max_density);

                        free_samples(&samples, samples_count);

#ifdef WITH_SOUNDPIPE
                        samples_count = load_samples(sp, &samples, fas_grains_path, fas_sample_rate, fas_samplerate_converter_type, 1);
#else
                        samples_count = load_samples(&samples, fas_grains_path, fas_sample_rate, fas_samplerate_converter_type, 1);
#endif
                        if (samples_count > 0) {
                            samples_count_m1 = samples_count - 1;
                        } else {
                            samples_count_m1 = 0;
                        }

                        curr_synth.grains = createGrains(&samples, samples_count, usd->synth_h, curr_synth.bank_settings->base_frequency, curr_synth.bank_settings->octave, fas_sample_rate, fas_max_instruments, fas_granular_max_density);

                        audioPlay();
                    } else if (action_type[0] == FAS_ACTION_NOTE_RESET) { // RE-TRIGGER note
                        struct _freelist_synth_commands *freelist_synth_command = getSynthCommandFreelist();
                        if (freelist_synth_command == NULL) {
#ifdef DEBUG
                            printf("Skipping note reset, commands pool is empty.\n");
                            fflush(stdout);
#endif

                            goto free_packet;
                        }

                        freelist_synth_command->data->type = FAS_CMD_NOTE_RESET;

                        unsigned int *data_uint = (unsigned int *)&usd->packet[PACKET_HEADER_LENGTH];

                        freelist_synth_command->data->value[0] = data_uint[0];
                        freelist_synth_command->data->value[1] = data_uint[1];

                        if (lfds720_queue_bss_enqueue(&synth_commands_queue_state, NULL, (void *)freelist_synth_command) == 0) {
#ifdef DEBUG
                            printf("Skipping note reset, commands queue is full.\n");
                            fflush(stdout);
#endif

                            goto free_packet; 
                        }
                    } else if (action_type[0] == FAS_ACTION_PAUSE) {
                        if (usd->frame_data != NULL) {
                            // the audio thread will pause and fade off so ensure future notes data start off a correct state
                            memset(usd->prev_frame_data, 0, usd->expected_max_frame_length);
                        }

                        audioPause();

                        fas_paused_by_client = 1;

                        // just to refresh load stats
                        sendPerformances(wsi, 0);
                    } else if (action_type[0] == FAS_ACTION_RESUME) {
                        // reset performances stats
                        frame_sync.lasttime = ns();
                        frame_sync.acc_time = note_time * 1000;

                        fas_paused_by_client = 0;

                        audioPlay();
                    }
#ifdef WITH_FAUST
                    else if (action_type[0] == FAS_ACTION_FAUST_GENS) { // reload Faust generators
                            audioFlushThenPause();
                            clearQueues();

                            freeFaustGenerators(&curr_synth.oscillators, curr_synth.bank_settings->h, fas_max_instruments);

                            freeFaustFactories(fas_faust_gens);
                            fas_faust_gens = createFaustFactories(fas_faust_gens_path, fas_faust_libs_path);

                            createFaustGenerators(fas_faust_gens, curr_synth.oscillators, curr_synth.bank_settings->h, fas_sample_rate, fas_max_instruments);

                            audioPlay();
                    } else if (action_type[0] == FAS_ACTION_FAUST_EFFS) { // reload Faust effects
                            audioFlushThenPause();
                            clearQueues();

                            freeFaustEffects(synth_fx, fas_max_channels);

                            freeFaustFactories(fas_faust_effs);
                            fas_faust_effs = createFaustFactories(fas_faust_effs_path, fas_faust_libs_path);

                            createFaustEffects(fas_faust_effs, synth_fx, fas_max_channels, fas_sample_rate);

                            audioPlay();
                    }
#endif
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
            if(!usd->connected) {
                return 0;
            }

            if (clients > 0) {
                if (reason == LWS_CALLBACK_WS_PEER_INITIATED_CLOSE) {
                    audioFlushThenPause();
                }

                clearQueues();

                freeUserSynthChnFxSettings(usd->synth_chn_fx_settings);

                usd->synth_chn_fx_settings = NULL;

                free(usd->instruments);

                usd->instruments = NULL;

                if (usd->oscillators) {
                    usd->oscillators = freeOscillatorsBank(&usd->oscillators, usd->synth_h, fas_max_instruments);
                }

                if (curr_synth.oscillators) {
                    curr_synth.oscillators = freeOscillatorsBank(&curr_synth.oscillators, curr_synth.bank_settings->h, fas_max_instruments);
                }

                free(usd->prev_frame_data);
                free(usd->frame_data);

                usd->prev_frame_data = NULL;
                usd->frame_data = NULL;

                printf("Connection from %s (%s) closed.\n", usd->peer_name, usd->peer_ip);
                fflush(stdout);

                clients -= 1;
            }
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
        0,
        NULL
	},
	{ NULL, NULL, 0, 0, 0, NULL }
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
        fflush(stderr);
        return -1;
    }

    printf("Fragment Synthesizer successfully started and listening on %s:%u.\n", (fas_iface == NULL) ? "127.0.0.1" : fas_iface, fas_port);

    return 0;
}

#ifndef WITH_JACK
void *streamWatcher(void *args) {
    PaError err = paNoError;

    while (keep_running) {
#if defined(_WIN32) || defined(_WIN64)
        Sleep(5000);
#else
        sleep(5);
#endif

        // check if stream is still alive, if not this may need a restart...
        if (Pa_IsStreamActive(stream) == 0 && Pa_IsStreamStopped(stream) == 0) {
            fprintf(stderr, "Inactive stream... (time out ?)\n");
            fflush(stderr);
        }
    }

    return 0;
}
#endif

void int_handler(int dummy) {
    keep_running = 0;
}

void fpe_handler(int dummy) {
    fprintf(stderr, "SIGFPE\n");
    fflush(stderr);

    keep_running = 0;
}

int main(int argc, char **argv)
{
    int print_infos = 0;

    int i = 0, j = 0;

    static struct option long_options[] = {
        { "sample_rate",                required_argument, 0, 0 },
        { "frames",                     required_argument, 0, 1 },
        { "wavetable",                  required_argument, 0, 2 },
        { "wavetable_size",             required_argument, 0, 3 },
        { "deflate",                    required_argument, 0, 4 },
        { "rx_buffer_size",             required_argument, 0, 5 },
        { "port",                       required_argument, 0, 6 },
        { "frames_queue_size",          required_argument, 0, 7 },
        { "commands_queue_size",        required_argument, 0, 8 },
        { "ssl",                        required_argument, 0, 9 },
        { "iface",                      required_argument, 0, 10 },
        { "input_device",               required_argument, 0, 11 },
        { "device",                     required_argument, 0, 12 },
        { "output_channels",            required_argument, 0, 13 },
        { "input_channels",             required_argument, 0, 14 },
        { "i",                                no_argument, 0, 15 },
        { "noise_amount",               required_argument, 0, 16 },
        { "grains_dir",                 required_argument, 0, 17 },
        { "waves_dir",                  required_argument, 0, 18 },
        { "impulses_dir",               required_argument, 0, 19 },
        { "smooth_factor",              required_argument, 0, 20 },
        { "granular_max_density",       required_argument, 0, 21 },
        { "stream_infos_send_delay",    required_argument, 0, 22 },
        { "max_drop",                   required_argument, 0, 23 },
        { "samplerate_conv_type",       required_argument, 0, 24 },
        { "render",                     required_argument, 0, 25 },
        { "render_width",               required_argument, 0, 26 },
        { "render_convert",             required_argument, 0, 27 },
        { "faust_gens_dir",             required_argument, 0, 28 },
        { "faust_effs_dir",             required_argument, 0, 29 },
        { "faust_libs_dir",             required_argument, 0, 30 },
        { "max_instruments",            required_argument, 0, 31 },
        { "max_channels",               required_argument, 0, 32 },
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
                fas_deflate = strtoul(optarg, NULL, 0);
                break;
            case 5 :
                fas_rx_buffer_size = strtoul(optarg, NULL, 0);
                break;
            case 6 :
                fas_port = strtoul(optarg, NULL, 0);
                break;
            case 7:
                fas_frames_queue_size = strtoul(optarg, NULL, 0);
                break;
            case 8:
                fas_commands_queue_size = strtoul(optarg, NULL, 0);
                break;
            case 9:
                fas_ssl = strtoul(optarg, NULL, 0);
                break;
            case 10:
                fas_iface = optarg;
                break;
            case 11:
                fas_input_audio_device = strtoul(optarg, NULL, 0);
                if (fas_input_audio_device == 0) {
                    fas_input_audio_device_name = optarg;
                }
                break;
            case 12:
                fas_audio_device = strtoul(optarg, NULL, 0);
                if (fas_audio_device == 0) {
                    fas_audio_device_name = optarg;
                }
                break;
            case 13:
                fas_output_channels = strtoul(optarg, NULL, 0);
                break;
            case 14:
                fas_input_channels = strtoul(optarg, NULL, 0);
                break;
            case 15:
                print_infos = 1;
                break;
            case 16:
                fas_noise_amount = strtof(optarg, NULL);
                break;
            case 17:
                fas_grains_path = optarg;
                break;
            case 18:
                fas_waves_path = optarg;
                break;
            case 19:
                fas_impulses_path = optarg;
                break;
            case 20:
                fas_smooth_factor = strtod(optarg, NULL);
                break;
            case 21:
                fas_granular_max_density = strtoul(optarg, NULL, 0);
                break;
            case 22:
                fas_stream_infos_send_delay = strtoul(optarg, NULL, 0);
                break;
            case 23:
                fas_max_drop = strtoul(optarg, NULL, 0);
                break;
            case 24:
                fas_samplerate_converter_type = strtol(optarg, NULL, 0);
                break;
            case 25:
                fas_render_target = optarg;
                break;
            case 26:
                fas_render_width = strtoul(optarg, NULL, 0);
                break;
            case 27:
                fas_render_convert = optarg;
                break;
            case 28:
                fas_faust_gens_path = optarg;
                break;
            case 29:
                fas_faust_effs_path = optarg;
                break;
            case 30:
                fas_faust_libs_path = optarg;
                break;
            case 31:
                fas_max_instruments = strtoul(optarg, NULL, 0);
                break;
            case 32:
                fas_max_channels = strtoul(optarg, NULL, 0);
                break;
            default: print_usage();
                return EXIT_FAILURE;
        }
    }

    if (fas_grains_path == NULL) {
#ifdef __unix__
        struct stat s;
        int err = stat(fas_install_default_grains_path, &s);
        if (err == -1) {
            if (ENOENT == errno) {
                fas_grains_path = fas_default_grains_path;
            } else {
                fprintf(stderr, "stat() error while checking for '%s' directory.\n", fas_install_default_grains_path);
                return EXIT_FAILURE;
            }
        } else {
            if (S_ISDIR(s.st_mode)) {
                fas_grains_path = fas_install_default_grains_path;
                printf("'%s' directory detected, default grains directory.\n", fas_install_default_grains_path);
            } else {
                printf("'%s' is not a directory, defaulting to non-install grains path.\n", fas_install_default_grains_path);
                fas_grains_path = fas_default_grains_path;
            }
        }
#else
        fas_grains_path = fas_default_grains_path;
#endif
    }

    if (fas_waves_path == NULL) {
#ifdef __unix__
        struct stat s;
        int err = stat(fas_install_default_waves_path, &s);
        if (err == -1) {
            if (ENOENT == errno) {
                fas_waves_path = fas_default_waves_path;
            } else {
                fprintf(stderr, "stat() error while checking for '%s' directory.\n", fas_install_default_waves_path);
                return EXIT_FAILURE;
            }
        } else {
            if (S_ISDIR(s.st_mode)) {
                fas_waves_path = fas_install_default_waves_path;
                printf("'%s' directory detected, default waves directory.\n", fas_install_default_waves_path);
            } else {
                printf("'%s' is not a directory, defaulting to non-install waves path.\n", fas_install_default_waves_path);
                fas_waves_path = fas_default_waves_path;
            }
        }
#else
        fas_waves_path = fas_default_waves_path;
#endif
    }

    if (fas_impulses_path == NULL) {
#ifdef __unix__
        struct stat s;
        int err = stat(fas_install_default_impulses_path, &s);
        if (err == -1) {
            if (ENOENT == errno) {
                fas_impulses_path = fas_default_impulses_path;
            } else {
                fprintf(stderr, "stat() error while checking for '%s' directory.\n", fas_install_default_impulses_path);
                return EXIT_FAILURE;
            }
        } else {
            if (S_ISDIR(s.st_mode)) {
                fas_impulses_path = fas_install_default_impulses_path;
                printf("'%s' directory detected, default impulses directory.\n", fas_install_default_impulses_path);
            } else {
                printf("'%s' is not a directory, defaulting to non-install impulses path.\n", fas_install_default_impulses_path);
                fas_impulses_path = fas_default_impulses_path;
            }
        }
#else
        fas_impulses_path = fas_default_impulses_path;
#endif
    }

    if (fas_faust_gens_path == NULL) {
#ifdef __unix__
        struct stat s;
        int err = stat(fas_install_default_faust_gens_path, &s);
        if (err == -1) {
            if (ENOENT == errno) {
                fas_faust_gens_path = fas_default_faust_gens_path;
            } else {
                fprintf(stderr, "stat() error while checking for '%s' directory.\n", fas_install_default_faust_gens_path);
                return EXIT_FAILURE;
            }
        } else {
            if (S_ISDIR(s.st_mode)) {
                fas_faust_gens_path = fas_install_default_faust_gens_path;
                printf("'%s' directory detected, default faust generators directory.\n", fas_install_default_faust_gens_path);
            } else {
                printf("'%s' is not a directory, defaulting to non-install faust generators path.\n", fas_install_default_faust_gens_path);
                fas_faust_gens_path = fas_default_faust_gens_path;
            }
        }
#else
        fas_faust_gens_path = fas_default_faust_gens_path;
#endif
    }

    if (fas_faust_effs_path == NULL) {
#ifdef __unix__
        struct stat s;
        int err = stat(fas_install_default_faust_effs_path, &s);
        if (err == -1) {
            if (ENOENT == errno) {
                fas_faust_effs_path = fas_default_faust_effs_path;
            } else {
                fprintf(stderr, "stat() error while checking for '%s' directory.\n", fas_install_default_faust_effs_path);
                return EXIT_FAILURE;
            }
        } else {
            if (S_ISDIR(s.st_mode)) {
                fas_faust_effs_path = fas_install_default_faust_effs_path;
                printf("'%s' directory detected, default faust effects directory.\n", fas_install_default_faust_effs_path);
            } else {
                printf("'%s' is not a directory, defaulting to non-install faust effects path.\n", fas_install_default_faust_effs_path);
                fas_faust_effs_path = fas_default_faust_effs_path;
            }
        }
#else
        fas_faust_effs_path = fas_default_faust_effs_path;
#endif
    }

    if (fas_max_instruments == 0) {
        printf("Warning: max_instruments program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_MAX_INSTRUMENTS);

        fas_max_instruments = FAS_MAX_INSTRUMENTS;
    }

    if (fas_max_channels == 0) {
        printf("Warning: max_channels program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_MAX_CHANNELS);

        fas_max_channels = FAS_MAX_CHANNELS;
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

    if (fas_port == 0) {
        printf("Warning: port program option argument is invalid, should be > 0, the default value (%u) will be used.\n", FAS_PORT);

        fas_port = FAS_PORT;
    }

    if (fas_frames_per_buffer < 0) {
        printf("Warning: frames program option argument is invalid, should be >= 0 (where 0 mean variable number of frames), the default value (%u) will be used.\n", FAS_FRAMES_PER_BUFFER);

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

    if (fas_input_channels < 0 || (fas_input_channels % 2) != 0) {
        printf("Warning: input_channels program option argument is invalid, should be >= 0, the default value (%u) will be used.\n", FAS_INPUT_CHANNELS);

        fas_input_channels = FAS_INPUT_CHANNELS;
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

    if (fas_stream_infos_send_delay < 1.) {
        printf("Warning: fas_stream_infos_send_delay program option argument is invalid, should be >= 1, the default value (%i) will be used.\n", FAS_STREAM_INFOS_SEND_DELAY);

        fas_stream_infos_send_delay = FAS_STREAM_INFOS_SEND_DELAY;
    }

    if (fas_max_drop <= 0) {
        printf("Warning: max_drop program option argument is invalid, should be >= 0, the default value (%i) will be used.\n", FAS_MAX_DROP);

        fas_max_drop = FAS_MAX_DROP;
    }

    if (fas_render_width <= 0) {
        printf("Warning: render_width program option argument is invalid, should be >= 0, the default value (%i) will be used.\n", FAS_RENDER_WIDTH);

        fas_render_width = FAS_RENDER_WIDTH;
    }

    if (errno == ERANGE) {
        printf("Warning: One of the specified program option is out of range and was set to its default value.\n");
    }

    if (fas_samplerate_converter_type >= 0) {
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

    // Soundpipe related
#ifdef WITH_SOUNDPIPE
    sp_create(&sp);
    sp->sr = fas_sample_rate;
#endif

    if (print_infos != 1) {
        time(&stream_load_begin);

#ifdef WITH_SOUNDPIPE
        impulses_count = load_samples(sp, &impulses, fas_impulses_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#else
        impulses_count = load_samples(&impulses, fas_impulses_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#endif
        if (impulses_count > 0) {
            impulses_count_m1 = impulses_count - 1;
        }

#ifdef WITH_SOUNDPIPE
        waves_count = load_samples(sp, &waves, fas_waves_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#else
        waves_count = load_samples(&waves, fas_waves_path, fas_sample_rate, fas_samplerate_converter_type, 0);
#endif
        if (waves_count > 0) {
            waves_count_m1 = waves_count - 1;
        }

#ifdef WITH_SOUNDPIPE
        samples_count = load_samples(sp, &samples, fas_grains_path, fas_sample_rate, fas_samplerate_converter_type, 1);
#else
        samples_count = load_samples(&samples, fas_grains_path, fas_sample_rate, fas_samplerate_converter_type, 1);
#endif
        if (samples_count > 0) {
            samples_count_m1 = samples_count - 1;
        }

#ifdef WITH_FAUST
        // faust libs path setup
        if (fas_faust_libs_path == NULL) { // setup default faust libs path
            fas_faust_libs_path = fas_default_faust_libs_path;
        }

        printf("Faust library path set to '%s'\n", fas_faust_libs_path);
        //

        fas_faust_gens = createFaustFactories("./faust/generators", fas_faust_libs_path);
        fas_faust_effs = createFaustFactories("./faust/effects", fas_faust_libs_path);
#endif

        if (fas_wavetable) {
            fas_sine_wavetable = sine_wavetable_init(fas_wavetable_size);
            if (fas_sine_wavetable == NULL) {
                fprintf(stderr, "sine_wavetable_init() failed.\n");

                return EXIT_FAILURE;
            }

            fas_white_noise_table = wnoise_wavetable_init(fas_noise_wavetable_size, 1.0);
            if (fas_white_noise_table == NULL) {
                fprintf(stderr, "wnoise_wavetable_init() failed.\n");
                free(fas_sine_wavetable);

                return EXIT_FAILURE;
            }
        }

        grain_envelope = createEnvelopes(FAS_ENVS_SIZE);
    }

    curr_synth.oscillators = NULL;
    curr_synth.grains = NULL;
    curr_synth.settings = NULL;
    curr_synth.bank_settings = NULL;
    curr_synth.chn_settings = NULL;
    curr_synth.lerp_t = 0;

#ifdef WITH_JACK
    // Jack related
    int err = 0;

    const char *client_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	client = jack_client_open ("Fragment Audio Server", options, &status, NULL);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		
        goto error;
	}

	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}

	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

    jack_set_process_callback (client, jackCallback, 0);

    fas_sample_rate = jack_get_sample_rate (client);

    jack_in = calloc(fas_input_channels, sizeof(jack_default_audio_sample_t **));
    jack_out = calloc(fas_output_channels, sizeof(jack_default_audio_sample_t **));

    output_ports = calloc(fas_output_channels, sizeof(jack_port_t **));
    input_ports = calloc(fas_input_channels, sizeof(jack_port_t **));

    char portName[255];
    for (i = 0; i < fas_output_channels; i += 2) {
        snprintf(portName, 255, "output_%03d", i);
        output_ports[i] = jack_port_register (client, portName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        if (output_ports[i] == NULL) {
            fprintf(stderr, "jack_port_register() failed for '%s' port\n", portName);
        }

        snprintf(portName, 255, "output_%03d", i + 1);
        output_ports[i + 1] = jack_port_register (client, portName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        if (output_ports[i + 1] == NULL) {
            fprintf(stderr, "jack_port_register() failed for '%s' port\n", portName);
        }
    }

    for (i = 0; i < fas_input_channels; i += 2) {
        snprintf(portName, 255, "input_%03d", i);
        input_ports[i] = jack_port_register (client, portName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

        if (input_ports[i] == NULL) {
            fprintf(stderr, "jack_port_register() failed for '%s' port\n", portName);
        }

        snprintf(portName, 255, "input_%03d", i + 1);
        input_ports[i + 1] = jack_port_register (client, portName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

        if (input_ports[i + 1] == NULL) {
            fprintf(stderr, "jack_port_register() failed for '%s' port\n", portName);
        }
    }

    printf("\nJack: Using Jack (%u) with %u output ports", fas_sample_rate, fas_output_channels);
    printf("\nJack: Using Jack (%u) with %u input ports\n", fas_sample_rate, fas_input_channels);

    cpu_load_measurer.samplingPeriod = 1. / fas_sample_rate;
    cpu_load_measurer.averageLoad = 0.;
#else
    // PortAudio related
    PaError err;

    memset(&inputParameters, 0, sizeof(PaStreamParameters));
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

        printf("\nPortAudio device %i (%i) - %s\n==========\n", i, device_info->hostApi, device_info->name);
        printf("  max input channels : %i\n", device_info->maxInputChannels);
        printf("  max output channels : %i\n", device_info->maxOutputChannels);
        printf("  default low input latency : %f\n", device_info->defaultLowInputLatency);
        printf("  default low output latency : %f\n", device_info->defaultLowOutputLatency);
        printf("  default high input latency : %f\n", device_info->defaultHighInputLatency);
        printf("  default high output latency : %f\n", device_info->defaultHighOutputLatency);
        printf("  default sample rate : %f\n", device_info->defaultSampleRate);

        if (fas_audio_device_name) {
            if (strcmp(fas_audio_device_name, device_info->name) == 0) {
                fas_audio_device = i;
            }
        }

        if (fas_input_audio_device_name) {
            if (strcmp(fas_input_audio_device_name, device_info->name) == 0) {
                fas_input_audio_device = i;
            }
        }
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
#ifndef INTERLEAVED_SAMPLE_FORMAT
        outputParameters.sampleFormat |= paNonInterleaved;
#endif
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
    } else {
        device_info = Pa_GetDeviceInfo(fas_audio_device);
        if (device_info == NULL) {
            fprintf(stderr, "Error: Output device does not exist.\n");
            goto error;
        }

        device_max_output_channels = device_info->maxOutputChannels;
        if (fas_output_channels > device_max_output_channels) {
            printf("Warning: Requested output_channels program option is larger than the device output channels, defaulting to %i\n", device_max_output_channels);
            fas_output_channels = device_max_output_channels;
        }

        outputParameters.device = fas_audio_device;
        outputParameters.channelCount = fas_output_channels;
        outputParameters.sampleFormat = paFloat32;
#ifndef INTERLEAVED_SAMPLE_FORMAT
        outputParameters.sampleFormat |= paNonInterleaved;
#endif
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
    }

    int device_max_input_channels;
    if (fas_input_audio_device >= num_devices || fas_input_audio_device < 0) {
        inputParameters.device = Pa_GetDefaultInputDevice();
        if (inputParameters.device == paNoDevice) {
            fprintf(stderr, "Error: No default input device.\n");
            goto error;
        }

        device_max_input_channels = Pa_GetDeviceInfo(inputParameters.device)->maxInputChannels;
        if (fas_input_channels > device_max_input_channels) {
            printf("Warning: Requested input_channels program option is larger than the device input channels, defaulting to %i\n", device_max_input_channels);
            fas_input_channels = device_max_input_channels;
        }

        inputParameters.channelCount = fas_input_channels;
        inputParameters.sampleFormat = paFloat32;
#ifndef INTERLEAVED_SAMPLE_FORMAT
        inputParameters.sampleFormat |= paNonInterleaved;
#endif
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;
    } else {
        device_info = Pa_GetDeviceInfo(fas_input_audio_device);
        if (device_info == NULL) {
            fprintf(stderr, "Error: Input device does not exist.\n");
            goto error;
        }

        device_max_input_channels = device_info->maxInputChannels;
        if (fas_input_channels > device_max_input_channels) {
            printf("Warning: Requested input_channels program option is larger than the device input channels, defaulting to %i\n", device_max_input_channels);
            fas_input_channels = device_max_input_channels;
        }

        inputParameters.device = fas_input_audio_device;
        inputParameters.channelCount = fas_input_channels;
        inputParameters.sampleFormat = paFloat32;
#ifndef INTERLEAVED_SAMPLE_FORMAT
        inputParameters.sampleFormat |= paNonInterleaved;
#endif
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;
    }
#endif

    frame_data_count = fas_output_channels / 2;

    fas_instrument_states = createInstrumentsState(fas_max_instruments);

#ifndef WITH_JACK
    printf("\nPortAudio: Using device '%s' (%i) with %u output channels", Pa_GetDeviceInfo(outputParameters.device)->name, fas_sample_rate, fas_output_channels);
    printf("\nPortAudio: Using device '%s' (%i) with %u input channels\n", Pa_GetDeviceInfo(inputParameters.device)->name, fas_sample_rate, fas_input_channels);

    err = Pa_IsFormatSupported(fas_input_channels > 0 ? &inputParameters : NULL, &outputParameters, (double)fas_sample_rate);
    if (err != paFormatIsSupported) {
       printf("Pa_IsFormatSupported : Some device parameters are unsupported! (check sample rate)\n");
    }

    if (fas_frames_per_buffer == 0) {
        fas_frames_per_buffer = paFramesPerBufferUnspecified;
    }
#endif

    curr_synth.instruments = (struct _synth_instrument *)calloc(fas_max_instruments, sizeof(struct _synth_instrument));
    if (!curr_synth.instruments) {
        fprintf(stderr, "curr_synth.instruments calloc failed\n");
        fflush(stderr);

        goto error;  
    }

    curr_synth.settings = (struct _synth_settings*)calloc(1, sizeof(struct _synth_settings));
    if (!curr_synth.settings) {
        fprintf(stderr, "curr_synth.settings calloc failed\n");
        fflush(stderr);

        goto error;
    }

    curr_synth.settings->gain_lr = FAS_DEFAULT_GAIN;
    curr_synth.settings->fps = FAS_DEFAULT_FPS;

    fpsChange(FAS_DEFAULT_FPS);

    curr_synth.bank_settings = (struct _bank_settings*)calloc(1, sizeof(struct _bank_settings));
    if (!curr_synth.bank_settings) {
        fprintf(stderr, "curr_synth.bank_settings calloc failed\n");
        fflush(stderr);

        goto error;
    }

#ifndef WITH_JACK
    err = Pa_OpenStream(
              &stream,
              (fas_input_channels <= 0) ? NULL : &inputParameters,
              &outputParameters,
              fas_sample_rate,
              fas_frames_per_buffer,
              paDitherOff, // paClipOff // paNeverDropInput
              paCallback,
              NULL );
    if (err != paNoError) goto error;
#endif

    curr_synth.chn_settings = (struct _synth_chn_settings*)calloc(fas_max_channels, sizeof(struct _synth_chn_settings));

    synth_fx = (struct _synth_fx **)malloc(sizeof(struct _synth_fx *) * fas_max_channels);
    if (!synth_fx) {
        fprintf(stderr, "synth_fx alloc error.\n");
        goto quit;
    }

    createEffects(
#ifdef WITH_SOUNDPIPE
        sp,
#endif
        synth_fx, fas_max_channels, fas_sample_rate);
    
#ifdef WITH_FAUST
    createFaustEffects(fas_faust_effs, synth_fx, fas_max_channels, fas_sample_rate);
#endif

#if defined(_WIN32) || defined(_WIN64)
    struct lfds720_ringbuffer_n_element *re = NULL;
    re = malloc(sizeof(struct lfds720_ringbuffer_n_element) * (fas_frames_queue_size + 1));

    struct lfds720_queue_bss_element *synth_commands_queue_element =
        malloc(sizeof(struct lfds720_queue_bss_element) * fas_commands_queue_size);
#else
    struct lfds720_ringbuffer_n_element *re =
        aligned_alloc(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES, sizeof(struct lfds720_ringbuffer_n_element) * (fas_frames_queue_size + 1));

    struct lfds720_queue_bss_element *synth_commands_queue_element = NULL;
    synth_commands_queue_element = aligned_alloc(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES, sizeof(struct lfds720_queue_bss_element) * fas_commands_queue_size);
#endif

    if (re == NULL) {
        fprintf(stderr, "lfds rb data structures alloc./align. error.\n");
        goto quit;
    }

    if (synth_commands_queue_element == NULL) {
        fprintf(stderr, "lfds queue data structures alloc./align. error.\n");
        goto quit;
    }

    lfds720_ringbuffer_n_init_valid_on_current_logical_core(&rs, re, (fas_frames_queue_size + 1), NULL);
    lfds720_queue_bss_init_valid_on_current_logical_core(&synth_commands_queue_state, synth_commands_queue_element, fas_commands_queue_size, NULL);
    lfds720_freelist_n_init_valid_on_current_logical_core(&freelist_commands, NULL);
    lfds720_freelist_n_init_valid_on_current_logical_core(&freelist_frames, NULL);

    ffd = malloc(sizeof(struct _freelist_frames_data) * fas_frames_queue_size);
    if (ffd == NULL) {
        fprintf(stderr, "_freelist_frames_data data structure alloc. error.\n");
        goto quit;
    }

    fsc = malloc(sizeof(struct _freelist_synth_commands) * fas_commands_queue_size);
    if (fsc == NULL) {
        fprintf(stderr, "_freelist_synth_commands data structure alloc. error.\n");
        goto quit;
    }

    dummy_notes = calloc(fas_max_instruments, sizeof(struct note));
    if (dummy_notes == NULL) {
        fprintf(stderr, "note data structure alloc. error.\n");
        goto quit;
    }

    curr_notes = dummy_notes;

    initCommandsPool();

    fflush(stdout);
    fflush(stderr);

#ifdef WITH_JACK
	if (jack_activate (client)) {
		fprintf (stderr, "jack_activate() failed\n");
    
        goto quit;
	}
#else
    // portaudio stream watcher
    pthread_t tid;
    int watcherState = pthread_create(&tid, NULL, &streamWatcher, NULL);
    if (watcherState != 0) {
        fprintf(stderr, "pthread_create streamWatcher error %i\n", watcherState);
        goto quit;
    }
#endif

    srand(time(NULL));

    // start audio stream
#ifndef WITH_JACK
    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;
#endif

    if (start_server() < 0) {
        fprintf(stderr, "lws related error occured.\n");
        goto quit;
    }

    // websocket stuff
#ifdef __unix__
    signal(SIGINT, int_handler);
    // signal(SIGFPE, fpe_handler);
#endif
    do {
        lws_service(context, 1);

#if defined(_WIN32) || defined(_WIN64)
	if (_kbhit()) {
        keep_running = 0;
	}
#endif
    } while (keep_running);

quit:

#ifndef WITH_JACK
    pthread_join(tid, NULL);
#endif

    // thank you for your attention, bye. 
#ifdef WITH_JACK
    jack_client_close (client);

    free(output_ports);
    free(input_ports);

    free(jack_in);
    free(jack_out);
#else
    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;
#endif

    audio_thread_state = FAS_AUDIO_PAUSE;

    lws_context_destroy(context);

#ifndef WITH_JACK
    Pa_Terminate();
#endif

    freeInstrumentsState(fas_instrument_states, fas_max_instruments);

    // free synth
    if (curr_synth.oscillators) {
        freeOscillatorsBank(&curr_synth.oscillators, curr_synth.bank_settings->h, fas_max_instruments);
    }

    free(curr_synth.instruments);

    free(curr_synth.settings);

    if (curr_synth.grains) {
        freeGrains(&curr_synth.grains, samples_count, fas_max_instruments, curr_synth.bank_settings->h, fas_granular_max_density);
    }

    free(curr_synth.bank_settings);
    free(curr_synth.chn_settings);
    //

    free(fas_sine_wavetable);
    free(fas_white_noise_table);

#ifdef WITH_SOUNDPIPE
    if (sp) {
        sp_destroy(&sp);
    }
#endif

    if (synth_fx) {
        freeEffects(synth_fx, fas_max_channels);
        free(synth_fx);

        synth_fx = NULL;
    }

    freeEnvelopes(grain_envelope);
    free_samples(&impulses, impulses_count);
    free_samples(&waves, waves_count);
    free_samples(&samples, samples_count);

#ifdef WITH_FAUST
    freeFaustFactories(fas_faust_gens);
    freeFaustFactories(fas_faust_effs);
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    if (re) {
        lfds720_ringbuffer_n_cleanup(&rs, rb_element_cleanup_callback);
        free(re);
    }
#pragma GCC diagnostic pop

    lfds720_freelist_n_cleanup(&freelist_frames, flf_element_cleanup_callback);
    lfds720_freelist_n_cleanup(&freelist_commands, flc_element_cleanup_callback);

    if (ffd) {
        free(ffd);
    }

    if (fsc) {
        free(fsc);
    }

    if (dummy_notes) {
        free(dummy_notes);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    if (synth_commands_queue_element) {
        lfds720_queue_bss_cleanup(&synth_commands_queue_state, q_element_cleanup_callback);
        free(synth_commands_queue_element);
    }
#pragma GCC diagnostic pop

    freeRender();

    printf("Bye.\n");

    return err;

error:
#ifdef WITH_SOUNDPIPE
    if (sp) {
        sp_destroy(&sp);
    }
#endif

#ifndef WITH_JACK
    Pa_Terminate();

    if (err != paNoError) {
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    }
#endif

    freeInstrumentsState(fas_instrument_states, fas_max_instruments);

    freeEnvelopes(grain_envelope);

    free(fas_sine_wavetable);
    free(fas_white_noise_table);

    free_samples(&impulses, impulses_count);
    free_samples(&samples, samples_count);
    free_samples(&waves, waves_count);

#ifdef WITH_FAUST
    freeFaustFactories(fas_faust_gens);
    freeFaustFactories(fas_faust_effs);
#endif

    return err;
}
