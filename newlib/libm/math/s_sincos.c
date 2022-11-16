/* Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com> */
/* sincos -- currently no more efficient than two separate calls to
   sin and cos. */

#include "fdlibm.h"
#include <errno.h>
#include <math.h>

#ifdef _NEED_FLOAT64

void __inhibit_new_builtin_calls
sincos64(__float64 x, __float64 *sinx, __float64 *cosx)
{
    *sinx = _sin64(x);
    *cosx = _cos64(x);
}

_MATH_ALIAS_v_dDD(sincos)

#endif /* _NEED_FLOAT64 */
