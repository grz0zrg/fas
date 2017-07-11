#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "note.h"

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
