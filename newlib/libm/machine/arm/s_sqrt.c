/*-
 * Copyright (c) 2017 embedded brains GmbH
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

#if (__ARM_FP & 0x8) && !defined(__SOFTFP__)
#include "fdlibm.h"

double
sqrt(double x)
{
	double result;
#ifdef _WANT_MATH_ERRNO
        if (isless(x, 0.0))
            return __math_invalid(x);
#endif
#if __ARM_ARCH >= 6
	__asm__("vsqrt.f64 %P0, %P1" : "=w" (result) : "w" (x));
#else
	/* VFP9 Erratum 760019, see GCC sources "gcc/config/arm/vfp.md" */
	__asm__("vsqrt.f64 %P0, %P1" : "=&w" (result) : "w" (x));
#endif
	return result;
}

_MATH_ALIAS_d_d(sqrt)

#else
#include "../../math/s_sqrt.c"
#endif
