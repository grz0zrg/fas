#include "filters.h"

/*
    Copyright 2012 Stefano D'Angelo <zanga.mail@gmail.com>
    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.
    THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

// https://github.com/ddiakopoulos/MoogLadders/blob/master/src/ImprovedModel.h

#define MPI 3.14159265358979323846264338327950288
#define VT 0.312

double improved_moog(double input, double cutoff, double resonance, double drive, double *V, double *dV, double *tV, double sampleRate) {
    double x = (MPI * cutoff) / sampleRate;
    double g = 4.0 * MPI * VT * cutoff * (1.0 - x) / (1.0 + x);

    double dV0, dV1, dV2, dV3;

    dV0 = -g * (tanh((drive * input + resonance * V[3]) / (2.0 * VT)) + tV[0]);
    V[0] += (dV0 + dV[0]) / (2.0 * sampleRate);
    dV[0] = dV0;
    tV[0] = tanh(V[0] / (2.0 * VT));

    dV1 = g * (tV[0] - tV[1]);
    V[1] += (dV1 + dV[1]) / (2.0 * sampleRate);
    dV[1] = dV1;
    tV[1] = tanh(V[1] / (2.0 * VT));

    dV2 = g * (tV[1] - tV[2]);
    V[2] += (dV2 + dV[2]) / (2.0 * sampleRate);
    dV[2] = dV2;
    tV[2] = tanh(V[2] / (2.0 * VT));

    dV3 = g * (tV[2] - tV[3]);
    V[3] += (dV3 + dV[3]) / (2.0 * sampleRate);
    dV[3] = dV3;
    tV[3] = tanh(V[3] / (2.0 * VT));

    return V[3];
}
