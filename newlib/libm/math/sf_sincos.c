/* Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com> */
/* sincos -- currently no more efficient than two separate calls to
   sin and cos. */
#include "fdlibm.h"
#if __OBSOLETE_MATH_FLOAT

#include <errno.h>

#ifdef _HAVE_ALIAS_ATTRIBUTE
extern float _sinf(float);
extern float _cosf(float);
#else
#define _sinf sinf
#define _cosf cosf
#endif

void __inhibit_new_builtin_calls
sincosf(float x, float *sinx, float *cosx)
{
    *sinx = _sinf(x);
    *cosx = _cosf(x);
}

#ifdef _DOUBLE_IS_32BITS

void __inhibit_new_builtin_calls
sincos(double x, double *sinx, double *cosx)
{
    *sinx = _sinf((float)x);
    *cosx = _cosf((float)x);
}
#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH_FLOAT */
