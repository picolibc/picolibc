/* Copyright (C) 2015 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

#if !defined(_LDBL_EQ_DBL) || !defined(HAVE_ALIAS_ATTRIBUTE)

long double
hypotl(long double x, long double y)
{
#ifdef _LDBL_EQ_DBL
    return hypot(x, y);
#else
    /* Keep it simple for now...  */
    return sqrtl((x * x) + (y * y));
#endif
}
#endif
