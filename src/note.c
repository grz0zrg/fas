#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "note.h"

// fill the notes buffer for instruments
// data argument is the raw RGBA values received with the channels count indicated as the first entry
void fillNotesBuffer(unsigned int samples_count, unsigned int waves_count, unsigned int max_density, 
                    unsigned int instruments, unsigned int data_frame_size, struct note *note_buffer,
                    unsigned int h, size_t data_length, void *prev_data, void *data) {
    FAS_FLOAT pvl = 0, pvr = 0, pl, pr, pb, pa, l, r;
    unsigned int i, j, frame_data_index = 8;
    unsigned int li = 0, ri = 1;
    unsigned int index = 0, note_osc_index = 0, osc_count = 0;
    FAS_FLOAT volume_l, volume_r, blue, alpha;
    FAS_FLOAT inv_full_brightness = 1.0 / 255.0;

    unsigned int note_i = 0;

    if (data_frame_size == sizeof(float)) {
        inv_full_brightness = 1.;

        frame_data_index = 2;

        data_length /= data_frame_size;
    }

    for (j = 0; j < instruments; j += 1) {
        note_osc_index = index;
        index += 1;
        osc_count = 0;

        unsigned int y = h - 1;

        for (i = 0; i < data_length; i += 4) {
            if (data_frame_size == sizeof(float)) {
                float *pdata = (float *)prev_data;
                float *cdata = (float *)data;

                pl = pdata[frame_data_index + li];
                pr = pdata[frame_data_index + ri];

                l = cdata[frame_data_index + li];
                r = cdata[frame_data_index + ri];

                pb = pdata[frame_data_index + 2];
                pa = pdata[frame_data_index + 3];

                blue = cdata[frame_data_index + 2];
                alpha = cdata[frame_data_index + 3];
            } else {
                unsigned char *pdata = (unsigned char *)prev_data;
                unsigned char *cdata = (unsigned char *)data;

                pl = pdata[frame_data_index + li];
                pr = pdata[frame_data_index + ri];

                l = cdata[frame_data_index + li];
                r = cdata[frame_data_index + ri];

                pb = pdata[frame_data_index + 2] * inv_full_brightness;
                pa = pdata[frame_data_index + 3] * inv_full_brightness;

                blue = cdata[frame_data_index + 2] * inv_full_brightness;
                alpha = cdata[frame_data_index + 3] * inv_full_brightness;
            }

            frame_data_index += 4;

            struct note *_note = &note_buffer[index];

            if (l > 0 ) {
                volume_l = l * inv_full_brightness;
                pvl = pl * inv_full_brightness;
                _note->previous_volume_l = (isinf(pvl) || isnan(pvl)) ? 0 : pvl;
                _note->volume_l = (isinf(volume_l)  || isnan(volume_l)) ? 0 : volume_l;
                _note->diff_volume_l = _note->volume_l - _note->previous_volume_l;
            } else {
                if (pl > 0) {
                    pvl = pl * inv_full_brightness;
                    pvl = (isinf(pvl) || isnan(pvl)) ? 0 : pvl;

                    _note->previous_volume_l = pvl;
                    _note->diff_volume_l = -pvl;
                    _note->volume_l = 0;

                    // keep previous values for B & A otherwise its going to be 0 at this point
                    alpha = pa;
                    blue = pb;
                } else {
                    _note->previous_volume_l = 0;
                    _note->diff_volume_l = 0;
                    _note->volume_l = -1;

                    if (r == 0 && pr == 0) {
                        y -= 1;
                        continue;
                    }
                }
            }

            if (r > 0) {
                volume_r = r * inv_full_brightness;
                pvr = pr * inv_full_brightness;
                _note->previous_volume_r = (isinf(pvr) || isnan(pvr)) ? 0 : pvr;
                _note->volume_r = (isinf(volume_r)  || isnan(volume_r)) ? 0 : volume_r;
                _note->diff_volume_r = _note->volume_r - _note->previous_volume_r;
            } else {
                if (pr > 0) {
                    pvr = pr * inv_full_brightness;
                    pvr = (isinf(pvr)  || isnan(pvr)) ? 0 : pvr;

                    _note->previous_volume_r = pvr;
                    _note->diff_volume_r = -pvr;
                    _note->volume_r = 0;

                    // keep previous values for B & A otherwise its going to be 0 at this point
                    alpha = pa;
                    blue = pb;
                } else {
                    _note->previous_volume_r = 0;
                    _note->diff_volume_r = 0;
                    _note->volume_r = -1;

                    if (l == 0 && pl == 0) {
                        y -= 1;
                        continue;
                    }
                }
            }

            if (_note->volume_l != -1 || _note->volume_r != -1) {
                _note->osc_index = y;

                _note->alpha = (isinf(alpha) || isnan(alpha)) ? 0 : alpha;
                _note->palpha = (isinf(pa) || isnan(pa)) ? 0 : pa;
                _note->pblue = (isinf(pb) || isnan(pb)) ? 0 : pb;
                _note->blue = (isinf(blue) || isnan(blue)) ? 0 : blue;

                _note->density = fabs(round(_note->blue));

                _note->norm_density = 1.0f / (_note->density + 0.0000001f);

                if (_note->density < 1) {
                    _note->density = 1;
                }

                if (_note->density >= max_density) {
                    _note->density = 1;
                }

                double dummy_int_part;
                double alpha_int_part;
                double palpha_int_part;
                FAS_FLOAT blue_frac_part = modf(fabs(_note->blue), &dummy_int_part);
                FAS_FLOAT pblue_frac_part = modf(fabs(_note->pblue), &dummy_int_part);
                FAS_FLOAT palpha_frac_part = modf(fabs(_note->palpha), &palpha_int_part);
                FAS_FLOAT alpha_frac_part = modf(fabs(_note->alpha), &alpha_int_part);

                // for granular synthesis, samples and related
                _note->smp_index = blue_frac_part * (samples_count + 1);
                _note->psmp_index = pblue_frac_part * (samples_count + 1);

                // for wavetable synthesis
                _note->wav_index = (int)alpha_int_part % (waves_count + 1);
                _note->pwav_index = (int)palpha_int_part % (waves_count + 1);

                // for subtractive synthesis
                _note->cutoff = fabs(_note->blue);
                _note->res = alpha_frac_part;
            }

            index += 1;

            osc_count += 1;

            y -= 1;
        }

        note_buffer[note_osc_index].osc_index = osc_count;

#ifdef DEBUG_FRAME_DATA
    printf("Instrument l/r (stereo) %u : %i oscillators \n", (j + 1), osc_count);
#endif
    }
}

void notesOn(struct note *cn, unsigned int start, unsigned int end) {
    unsigned int j = start;
    for (j = start; j < end; j += 1) {
        struct note *n = &cn[j];

        // override notes data
        n->previous_volume_l = 0;
        n->previous_volume_r = 0;

        n->diff_volume_l = -n->previous_volume_l;
        n->diff_volume_r = -n->previous_volume_r;
    }
}

void notesOff(struct note *cn, unsigned int start, unsigned int end) {
    unsigned int j = start;
    for (j = start; j < end; j += 1) {
        struct note *n = &cn[j];

        // override notes data
        n->volume_l = 0;
        n->volume_r = 0;

        n->diff_volume_l = -n->previous_volume_l;
        n->diff_volume_r = -n->previous_volume_r;
    }
}