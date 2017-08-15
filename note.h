#ifndef _FAS_NOTE_H_
#define _FAS_NOTE_H_

    struct note {
        unsigned int osc_index;
        float previous_volume_l;
        float previous_volume_r;
        float volume_l;
        float volume_r;
        float diff_volume_l;
        float diff_volume_r;
    };

    extern void fillNotesBuffer(unsigned int data_frame_size, struct note *note_buffer, unsigned int h, size_t data_length, void *prev_data, void *data);

#endif
