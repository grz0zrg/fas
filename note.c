#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "note.h"

// fill the notes buffer for each output channels
// data argument is the raw RGBA values received with the channels count indicated as the first entry
void fillNotesBuffer(unsigned int data_frame_size, struct note *note_buffer, unsigned int h, size_t data_length, void *prev_data, void *data) {
    double pvl = 0, pvr = 0, pl, pr, l, r;
    unsigned int i, j, frame_data_index = 16;
    unsigned int li = 0, ri = 1;
    unsigned int index = 0, note_osc_index = 0, osc_count = 0;
    double volume_l, volume_r;
    double inv_full_brightness = 1.0 / 255.0;

    unsigned int channels[1];
    unsigned int monophonic[1];
    unsigned int synthesis_type[1];

    memcpy(&channels, &((char *) data)[0], sizeof(channels));
    memcpy(&monophonic, &((char *) data)[4], sizeof(monophonic));
    memcpy(&synthesis_type, &((char *) data)[8], sizeof(synthesis_type));

    if (data_frame_size == sizeof(float)) {
        inv_full_brightness = 1.;

        frame_data_index = 4;

        data_length /= data_frame_size;
    }

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
            if (data_frame_size == sizeof(float)) {
                float *pdata = (float *)prev_data;
                float *cdata = (float *)data;

                pl = pdata[frame_data_index + li];
                pr = pdata[frame_data_index + ri];

                l = cdata[frame_data_index + li];
                r = cdata[frame_data_index + ri];
            } else {
                unsigned char *pdata = (unsigned char *)prev_data;
                unsigned char *cdata = (unsigned char *)data;

                pl = pdata[frame_data_index + li];
                pr = pdata[frame_data_index + ri];

                l = cdata[frame_data_index + li];
                r = cdata[frame_data_index + ri];
            }

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
        printf("Channel l/r (mono, %u) %u : %i oscillators \n", *synthesis_type, (j + 1), osc_count);
    } else {
        printf("Channel l/r (%u) %u : %i oscillators \n", *synthesis_type, (j + 1), osc_count);
    }
#endif
    }
}
