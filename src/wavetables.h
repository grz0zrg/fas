#ifndef _FAS_WAVETABLE_H_
#define _FAS_WAVETABLE_H_

    #include "tools.h"
    
    extern FAS_FLOAT *sine_wavetable_init(unsigned int wavetable_size);
    extern FAS_FLOAT *wnoise_wavetable_init(unsigned int wavetable_size, FAS_FLOAT amount);

#endif
