#ifndef _FAS_WAVETABLE_H_
#define _FAS_WAVETABLE_H_

    #include "tools.h"
    
    extern float *sine_wavetable_init(unsigned int wavetable_size);
    extern float *wnoise_wavetable_init(unsigned int wavetable_size, float amount);

#endif
