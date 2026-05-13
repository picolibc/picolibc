/* Copyright (c) 2016 Yaakov Selkowitz <yselkowi@redhat.com> */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "local-complex.h"

float complex
clog10f(float complex z)
{
    float p, rr;

    rr = cabsf(z);
    p = log10f(rr);
    rr = atan2f(cimagf(z), crealf(z)) * (float)M_IVLN10;
    return CMPLXF(p, rr);
}
