/* Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com> */
/* sincos -- currently no more efficient than two separate calls to
   sin and cos. */
#include "fdlibm.h"
#if __OBSOLETE_MATH_FLOAT

#include <errno.h>

#ifdef HAVE_ALIAS_ATTRIBUTE
extern float _sinf(float);
extern float _cosf(float);
#else
#define _sinf sinf
#define _cosf cosf
#endif

#ifdef __STDC__
	void sincosf(float x, float *sinx, float *cosx)
#else
	void sincosf(x, sinx, cosx)
	float x;
        float *sinx;
        float *cosx;
#endif
{
  *sinx = _sinf (x);
  *cosx = _cosf (x);
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	void sincos(double x, double *sinx, double *cosx)
#else
	void sincos(x, sinx, cosx)
	double x;
        double sinx;
        double cosx;
#endif
{
  *sinx = _sinf((float) x);
  *cosx = _cosf((float) x);
}
#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH_FLOAT */
