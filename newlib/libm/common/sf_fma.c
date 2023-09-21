/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

#if !_HAVE_FAST_FMAF

#if __FLT_EVAL_METHOD__ == 2 && defined(_HAVE_LONG_DOUBLE)

float
fmaf(float x, float y, float z)
{
    return (float) fmal((long double) x, (long double) y, (long double) z);
}

#else

typedef float FLOAT_T;

#define FMA fmaf
#define NEXTAFTER nextafterf
#define LDEXP ldexpf
#define FREXP frexpf
#define SCALBN scalbnf
#define SPLIT (0x1p12f + 1.0f)
#define COPYSIGN copysignf
#define FLOAT_MANT_DIG        __FLT_MANT_DIG__
#define FLOAT_MAX_EXP         __FLT_MAX_EXP__
#define FLOAT_MIN             __FLT_MIN__
#define ILOGB    ilogbf

static inline int
odd_mant(FLOAT_T x)
{
    return asuint(x) & 1;
}

static unsigned int
EXPONENT(FLOAT_T x)
{
    return _exponent32(asuint(x));
}

#include "fma_inc.h"

#endif

_MATH_ALIAS_f_fff(fma)

#endif
