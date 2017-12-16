#include "filters.h"

// Credits : http://www.musicdsp.org/showArchiveComment.php?ArchiveID=26
double moog_vcf(double input, double fc, double res, double *in, double *out) {
    double f = fc * 1.16;
    double fb = res * (1.0 - 0.15 * f * f);
    input -= out[3] * fb;
    input *= 0.35013 * (f*f)*(f*f);
    out[0] = input + 0.3 * in[0] + (1 - f) * out[0]; // Pole 1
    in[0]  = input;
    out[1] = out[0] + 0.3 * in[1] + (1 - f) * out[1];  // Pole 2
    in[1]  = out[0];
    out[2] = out[1] + 0.3 * in[2] + (1 - f) * out[2];  // Pole 3
    in[2]  = out[1];
    out[3] = out[2] + 0.3 * in[3] + (1 - f) * out[3];  // Pole 4
    in[3]  = out[2];
    return out[3];
}
