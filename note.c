#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "note.h"

// fill the notes buffer for each output channels
// data argument is the raw RGBA values received with the channels count indicated as the first entry
void fillNotesBuffer(unsigned int samples_count, unsigned int channels, unsigned int data_frame_size, struct note *note_buffer, unsigned int h, struct oscillator **o, size_t data_length, void *prev_data, void *data) {
    struct oscillator *oscs = *o;

    double pvl = 0, pvr = 0, pl, pr, l, r;
    unsigned int i, j, frame_data_index = 8;
    unsigned int li = 0, ri = 1;
    unsigned int index = 0, note_osc_index = 0, osc_count = 0;
    double volume_l, volume_r, blue, alpha, palpha;
    double inv_full_brightness = 1.0 / 255.0;

    unsigned int note_i = 0;

    unsigned int monophonic[1];

    memcpy(&monophonic, &((char *) data)[4], sizeof(monophonic));

    if (data_frame_size == sizeof(float)) {
        inv_full_brightness = 1.;

        frame_data_index = 2;

        data_length /= data_frame_size;
    }

    if ((*monophonic) == 1) {
        li = 3;
        ri = 3;
    }

    for (j = 0; j < channels; j += 1) {
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

                blue = cdata[frame_data_index + 2];
                palpha = pdata[frame_data_index + 3];
                alpha = cdata[frame_data_index + 3];
            } else {
                unsigned char *pdata = (unsigned char *)prev_data;
                unsigned char *cdata = (unsigned char *)data;

                pl = pdata[frame_data_index + li];
                pr = pdata[frame_data_index + ri];

                l = cdata[frame_data_index + li];
                r = cdata[frame_data_index + ri];

                blue = cdata[frame_data_index + 2] * inv_full_brightness;
                palpha = pdata[frame_data_index + 3] * inv_full_brightness;
                alpha = cdata[frame_data_index + 3] * inv_full_brightness;
            }

            frame_data_index += 4;

            struct note *_note = &note_buffer[index];
            _note->osc_index = y;

            _note->alpha = alpha;
            _note->blue = blue;

            _note->previous_a = palpha;
            _note->diff_a = abs(alpha - palpha);

            // for granular synthesis, samples and related
            double dummy;
            _note->smp_index = modf(fabs(blue), &dummy) * (samples_count + 1);

            _note->fm_mod_source = modf(fabs(blue), &dummy) * channels;

            if (l > 0 ) {
                volume_l = l * inv_full_brightness;
                pvl = pl * inv_full_brightness;
                _note->previous_volume_l = pvl;
                _note->volume_l = volume_l;
                _note->diff_volume_l = volume_l - pvl;
//note_buffer[note_osc_index].mag[note_i] = abs(r * inv_full_brightness);//20. * log10();
//                note_buffer[note_osc_index].frq[note_i] = oscs[y].freq;

//                note_i++;
            } else {
                if (pl > 0) {
                    pvl = pl * inv_full_brightness;

                    _note->previous_volume_l = pvl;
                    _note->diff_volume_l = -pvl;
                    _note->volume_l = 0;
                } else {
                    _note->previous_volume_l = 0;
                    _note->diff_volume_l = 0;
                    _note->volume_l = 0;

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
                _note->volume_r = volume_r;
                _note->diff_volume_r = volume_r - pvr;

                //note_buffer[note_osc_index].frq[osc_count] = oscs[y].freq;
            } else {
                if (pr > 0) {
                    pvr = pr * inv_full_brightness;

                    _note->previous_volume_r = pvr;
                    _note->diff_volume_r = -pvr;
                    _note->volume_r = 0;
                } else {
                    _note->previous_volume_r = 0;
                    _note->diff_volume_r = 0;
                    _note->volume_r = 0;

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
/*double a = 0.;
        for (i = 0; i < osc_count; i += 1) {
            a = fmax(note_buffer[i].volume_l, a);
        }

        for (i = 0; i < osc_count; i += 1) {
note_buffer[note_osc_index].mag[i] = 20. * log10(note_buffer[note_osc_index].mag[i] + 1);
        }
*/
        note_buffer[note_osc_index].osc_index = osc_count;

#ifdef DEBUG
    if ((*monophonic) == 1) {
        printf("Channel l/r (mono) %u : %i oscillators \n", (j + 1), osc_count);
    } else {
        printf("Channel l/r (stereo) %u : %i oscillators \n", (j + 1), osc_count);
    }
#endif
    }
}
