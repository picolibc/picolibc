/* Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com> */
/* sincos -- currently no more efficient than two separate calls to
   sin and cos. */
#define _GNU_SOURCE
#include "fdlibm.h"
#if __OBSOLETE_MATH_FLOAT

#include <errno.h>

void __no_builtin
sincosf(float x, float *sinx, float *cosx)
{
    *sinx = _sinf(x);
    *cosx = _cosf(x);
}

_MATH_ALIAS_v_fFF(sincos)

#else
#include "../common/sincosf.c"
#endif /* __OBSOLETE_MATH_FLOAT */
