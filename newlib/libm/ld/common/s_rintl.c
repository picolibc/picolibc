/*-
 * Copyright (c) 2008 David Schultz <das@FreeBSD.ORG>
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



#if LDBL_MAX_EXP != 0x4000
/* We also require the usual bias, min exp and expsign packing. */
#error "Unsupported long double format"
#endif

#define	BIAS	(LDBL_MAX_EXP - 1)

static const float
shift[2] = {
#if LDBL_MANT_DIG == 64
	0x1.0p63, -0x1.0p63
#elif LDBL_MANT_DIG == 113
	0x1.0p112, -0x1.0p112
#else
#error "Unsupported long double format"
#endif
};
static const float zero[2] = { 0.0, -0.0 };

long double
rintl(long double x)
{
	union IEEEl2bits u;
	u_int32_t expsign;
	int ex, sign;

	u.e = x;
	expsign = u.xbits.expsign;
	ex = expsign & 0x7fff;

	if (ex >= BIAS + LDBL_MANT_DIG - 1) {
		if (ex == BIAS + LDBL_MAX_EXP)
			return (x + x);	/* Inf, NaN, or unsupported format */
		return (x);		/* finite and already an integer */
	}
	sign = expsign >> 15;

	/*
	 * The following code assumes that intermediate results are
	 * evaluated in long double precision. If they are evaluated in
	 * greater precision, double rounding may occur, and if they are
	 * evaluated in less precision (as on i386), results will be
	 * wildly incorrect.
	 */
	x += (long double)shift[sign];
	x -= (long double)shift[sign];

	/*
	 * If the result is +-0, then it must have the same sign as x, but
	 * the above calculation doesn't always give this.  Fix up the sign.
	 */
	if (ex < BIAS && x == 0.0L)
		return ((long double)zero[sign]);

	return (x);
}

/*
 * We save and restore the floating-point environment to avoid raising
 * an inexact exception.  We can get away with using fesetenv()
 * instead of feclearexcept()/feupdateenv() to restore the environment
 * because the only exception defined for rint() is overflow, and
 * rounding can't overflow as long as emax >= p.
 */
long double
nearbyintl(long double x)
{
	long double ret;

        if (isnan(x))
            return x + x;
#if defined(FE_INEXACT) && !defined(__DOUBLE_NOEXCEPT)
	fenv_t env;
	fegetenv(&env);
#endif
	ret = rintl(x);
#if defined(FE_INEXACT) && !defined(__DOUBLE_NOEXCEPT)
	fesetenv(&env);
#endif
	return (ret);
}
