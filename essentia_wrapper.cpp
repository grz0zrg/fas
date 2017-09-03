// C wrapper (only some algorithms) for essentia : http://essentia.upf.edu
// See essentia_wrapper.h

#include "essentia_wrapper.h"

CEssentia newCEssentia() {
    return reinterpret_cast<void*>(new WEssentia());
}

void delCEssentia(CEssentia e) {
    delete reinterpret_cast<WEssentia*>(e);
}

void initializeSineModelCEssentia(CEssentia e, double sample_rate, unsigned int framesize, unsigned int hopsize) {
    reinterpret_cast<WEssentia*>(e)->initializeSineModel(sample_rate, framesize, hopsize);
}

void freeSineModelCEssentia(CEssentia e) {
    reinterpret_cast<WEssentia*>(e)->freeSineModel();
}

float* computeSineModelCEssentia(CEssentia e, float* m, float* f, float* p, unsigned int len) {
    return reinterpret_cast<WEssentia*>(e)->computeSineModel(m, f, p, len);
}
