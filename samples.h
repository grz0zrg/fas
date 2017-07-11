#ifndef _FAS_SAMPLES_H_
#define _FAS_SAMPLES_H_

    struct sample {
        float *data;
        uint32_t len;
        uint32_t frames;
        unsigned int chn;
        float pitch; // hz
        int samplerate;
    };

    extern unsigned int load_samples(struct sample *samples, char *directory);

#endif
