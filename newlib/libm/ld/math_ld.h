/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fdlibm.h"
#include <math.h>

#if defined(_HAVE_LONG_DOUBLE_MATH) && defined(_NEED_FLOAT_HUGE)
#define _HAVE_LD_SUPPORT
#endif

#ifdef _HAVE_LD_SUPPORT

typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

#define __kernel_rem_pio2 __kernel_rem_pio2l

int	__kernel_rem_pio2(double*,double*,int,int,int);
/* long double precision kernel functions */
long double __kernel_sinl(long double, long double, int);
long double __kernel_cosl(long double, long double);
long double __kernel_tanl(long double, long double, int);

#ifndef __FreeBSD__
#define	STRICT_ASSIGN(type, lval, rval)	((lval) = (rval))
#else
#ifdef FLT_EVAL_METHOD
// Attempt to get strict C99 semantics for assignment with non-C99 compilers.
#if FLT_EVAL_METHOD == 0 || __GNUC__ == 0
#define	STRICT_ASSIGN(type, lval, rval)	((lval) = (rval))
#else
#define	STRICT_ASSIGN(type, lval, rval) do {	\
	volatile type __lval;			\
						\
	if (sizeof(type) >= sizeof(long double))	\
		(lval) = (rval);		\
	else {					\
		__lval = (rval);		\
		(lval) = __lval;		\
	}					\
} while (0)
#endif
#endif
#endif

#endif

/* Guess long double layout based on compiler defines */

#if __LDBL_MANT_DIG__ == 64 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384
#define _INTEL80_FLOAT
#ifdef __LP64__
#define _INTEL80_FLOAT_PAD
#endif
#endif

#if __LDBL_MANT_DIG__ == 113 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384
#define _IEEE128_FLOAT
#endif

#if __LDBL_MANT_DIG__ == 106 && __LDBL_MAX_EXP__ == 1024
#define _DOUBLE_DOUBLE_FLOAT
#endif

#ifndef __FLOAT_WORD_ORDER__
#define __FLOAT_WORD_ORDER__     __BYTE_ORDER__
#endif

#include "math_private_openbsd.h"

#ifndef EXT_FRACLMBITS
#define EXT_FRACLMBITS 0
#endif

#ifndef EXT_FRACHMBITS
#define EXT_FRACHMBITS 0
#endif
