/* Copyright (c) 2016 Yaakov Selkowitz <yselkowi@redhat.com> */
#define _DEFAULT_SOURCE
#include <complex.h>
#include <math.h>

float complex
clog10f(float complex z)
{
	float p, rr;

	rr = cabsf(z);
	p = log10f(rr);
	rr = atan2f(cimagf(z), crealf(z)) * (float) M_IVLN10;
	return (float complex) p + rr * I;
}
