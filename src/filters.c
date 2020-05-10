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

#define MPI 3.14159265358979323846264338327950288

const FAS_FLOAT huovilainen_thermal = 0.000025;

// GNU GENERAL PUBLIC LICENSE (Version 3, 29 June 2007)
// http://www.gnu.org/licenses/gpl-3.0.txt

// https://github.com/ddiakopoulos/MoogLadders/blob/master/src/HuovilainenModel.h
FAS_FLOAT huovilainen_moog(FAS_FLOAT input, FAS_FLOAT cutoff_computed, FAS_FLOAT res_computed, FAS_FLOAT *delay, FAS_FLOAT *stage, FAS_FLOAT *stageTanh, int oversample) {
    for (int j = 0; j < oversample; j++) {
        FAS_FLOAT in = input - res_computed * delay[5];
        delay[0] = stage[0] = delay[0] + cutoff_computed * (tanh(in * huovilainen_thermal) - stageTanh[0]);
        for (int k = 1; k < 4; k++) {
            in = stage[k-1];
            stage[k] = delay[k] + cutoff_computed * ((stageTanh[k-1] = tanh(in * huovilainen_thermal)) - (k != 3 ? stageTanh[k] : tanh(delay[k] * huovilainen_thermal)));
            delay[k] = stage[k];
        }

        delay[5] = (stage[3] + delay[4]) * 0.5;
        delay[4] = stage[3];
    }

    return delay[5];
}

void huovilainen_compute(FAS_FLOAT cutoff, FAS_FLOAT resonance, FAS_FLOAT *cutoff_computed, FAS_FLOAT *res_computed, FAS_FLOAT sampleRate) {
    FAS_FLOAT fc =  cutoff / sampleRate;
    FAS_FLOAT f  =  fc * 0.5;
    FAS_FLOAT fc2 = fc * fc;
    FAS_FLOAT fc3 = fc * fc * fc;

    FAS_FLOAT fcr = 1.8730 * fc3 + 0.4955 * fc2 - 0.6490 * fc + 0.9988;
    FAS_FLOAT acr = -3.9364 * fc2 + 1.8409 * fc + 0.9968;

    *cutoff_computed = (1.0 - exp(-((2 * MPI) * f * fcr))) / huovilainen_thermal;
    *res_computed = 4.0 * resonance * acr;
}
