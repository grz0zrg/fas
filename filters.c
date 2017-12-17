#include "filters.h"

// https://www.kvraudio.com/forum/viewtopic.php?p=5447795
inline double fast_tanh(const double x)
{
   const double ax = fabs(x);
   const double x2 = x * x;

   return ( x * ( 2.45550750702956 + 2.45550750702956 * ax +
      ( 0.893229853513558 + 0.821226666969744 * ax ) * x2 ) /
      ( 2.44506634652299 + ( 2.44506634652299 + x2 ) *
      fabs( x + 0.814642734961073 * x * ax )));
}

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

    dV0 = -g * (fast_tanh((drive * input + resonance * V[3]) / (2.0 * VT)) + tV[0]);
    V[0] += (dV0 + dV[0]) / (2.0 * sampleRate);
    dV[0] = dV0;
    tV[0] = fast_tanh(V[0] / (2.0 * VT));

    dV1 = g * (tV[0] - tV[1]);
    V[1] += (dV1 + dV[1]) / (2.0 * sampleRate);
    dV[1] = dV1;
    tV[1] = fast_tanh(V[1] / (2.0 * VT));

    dV2 = g * (tV[1] - tV[2]);
    V[2] += (dV2 + dV[2]) / (2.0 * sampleRate);
    dV[2] = dV2;
    tV[2] = fast_tanh(V[2] / (2.0 * VT));

    dV3 = g * (tV[2] - tV[3]);
    V[3] += (dV3 + dV[3]) / (2.0 * sampleRate);
    dV[3] = dV3;
    tV[3] = fast_tanh(V[3] / (2.0 * VT));

    return V[3];
}

const double huovilainen_thermal = 0.000025;

// GNU GENERAL PUBLIC LICENSE (Version 3, 29 June 2007)
//http://www.gnu.org/licenses/gpl-3.0.txt

// https://github.com/ddiakopoulos/MoogLadders/blob/master/src/HuovilainenModel.h
double huovilainen_moog(double input, double cutoff_computed, double res_computed, double *delay, double *stage, double *stageTanh, int oversample) {
    for (int j = 0; j < oversample; j++) {
        float in = input - res_computed * delay[5];
        delay[0] = stage[0] = delay[0] + cutoff_computed * (fast_tanh(in * huovilainen_thermal) - stageTanh[0]);
        for (int k = 1; k < 4; k++) {
            in = stage[k-1];
            stage[k] = delay[k] + cutoff_computed * ((stageTanh[k-1] = fast_tanh(in * huovilainen_thermal)) - (k != 3 ? stageTanh[k] : fast_tanh(delay[k] * huovilainen_thermal)));
            delay[k] = stage[k];
        }

        delay[5] = (stage[3] + delay[4]) * 0.5;
        delay[4] = stage[3];
    }

    return delay[5];
}

void huovilainen_compute(double cutoff, double resonance, double *cutoff_computed, double *res_computed, double sampleRate) {
    double fc =  cutoff / sampleRate;
    double f  =  fc * 0.5;
    double fc2 = fc * fc;
    double fc3 = fc * fc * fc;

    double fcr = 1.8730 * fc3 + 0.4955 * fc2 - 0.6490 * fc + 0.9988;
    double acr = -3.9364 * fc2 + 1.8409 * fc + 0.9968;

    *cutoff_computed = (1.0 - exp(-((2 * MPI) * f * fcr))) / huovilainen_thermal;
    *res_computed = 4.0 * resonance * acr;
}
