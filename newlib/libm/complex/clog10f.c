#include <complex.h>
#include <math.h>

float complex
clog10f(float complex z)
{
	float complex w;
	float p, rr;

	rr = cabsf(z);
	p = log10f(rr);
	rr = atan2f(cimagf(z), crealf(z)) * (float) M_IVLN10;
	w = p + rr * I;
	return w;
}
