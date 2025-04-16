/*-
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
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


#include <limits.h>
#ifndef dtype
#define dtype long
#define fn lrintl
#define DTYPE_MIN LONG_MIN
#define DTYPE_MAX LONG_MAX
#endif

/*
 * C99 says we should not raise a spurious inexact exception when an
 * invalid exception is raised.  Unfortunately, the set of inputs
 * that overflows depends on the rounding mode when 'long' has more
 * significant bits than 'long double'.  Hence, we bend over backwards for the
 * sake of correctness; an MD implementation could be more efficient.
 */
dtype
fn(long double x)
{
	x = rintl(x);

        /*
         * Bounds check to ensure the operation is defined and
         * to prevent an exception from being raised
         */
        if (!(DTYPE_MIN <= x && x <= DTYPE_MAX)) {
            __math_set_invalidl();
            return signbit(x) ? DTYPE_MIN : DTYPE_MAX;
        }
	return (dtype) x;
}
