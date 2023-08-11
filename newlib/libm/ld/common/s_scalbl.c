/*-
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//__FBSDID("$FreeBSD: src/lib/msun/src/s_scalbln.c,v 1.2 2005/03/07 04:57:50 das Exp $");

#include <limits.h>

long double
scalbl (long double x, long double fn)
{
    if (isnanl_inline(fn) || isnanl_inline(x))
        return x + fn;

    if (isinf(fn)) {
        if ((x == 0.0L && fn > 0.0L) || (isinf(x) && fn < 0.0L))
            return __math_invalidl(fn);
        if (fn > 0.0L)
            return fn*x;
        else
            return x/(-fn);
    }

    if (floorl(fn) != fn)
        return __math_invalidl(fn);

    if (fn > 4.0L * __LDBL_MAX_EXP__)
        fn = 4.0L * __LDBL_MAX_EXP__;

    if (fn < -4.0L * __LDBL_MAX_EXP__)
        fn = -4.0L * __LDBL_MAX_EXP__;

    return scalbnl(x, (int)fn);
}
