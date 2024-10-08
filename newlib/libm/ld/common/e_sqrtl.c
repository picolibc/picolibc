/*-
 * Copyright (c) 2007 Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _DOUBLE_DOUBLE_FLOAT
/* Return (x + ulp) for normal positive x. Assumes no overflow. */
static inline long double
inc(long double x)
{
	union IEEEl2bits u;

	u.e = x;
	if (++u.bits.manl == 0) {
		if (++u.bits.manh == 0) {
			u.bits.exp++;
			u.bits.manh |= LDBL_NBIT;
		}
	}
	return (u.e);
}

/* Return (x - ulp) for normal positive x. Assumes no underflow. */
static inline long double
dec(long double x)
{
	union IEEEl2bits u;

	u.e = x;
	if (u.bits.manl-- == 0) {
		if (u.bits.manh-- == LDBL_NBIT) {
			u.bits.exp--;
			u.bits.manh |= LDBL_NBIT;
		}
	}
	return (u.e);
}

#ifndef __GNUC__
#pragma STDC FENV_ACCESS ON
#endif

#endif

/*
 * This is slow, but simple and portable. You should use hardware sqrt
 * if possible.
 */

#define BIAS    (LDBL_MAX_EXP-1)

long double
sqrtl(long double x)
{
	union IEEEl2bits u;
	int k;
	long double lo, xn;
	fenv_t env;

	u.e = x;

	/* If x = NaN, then sqrt(x) = qNaN. */
	/* If x = Inf, then sqrt(x) = Inf. */
	/* If x = -Inf, then sqrt(x) = sNaN. */
	if (u.bits.exp == LDBL_INF_NAN_EXP) {
                if (u.bits.sign && u.bits.manh == LDBL_NBIT_INF && u.bits.manl == 0)
                        return __math_invalidl(x);
                return x + x;
        }

	/* If x = +-0, then sqrt(x) = +-0. */
	if ((u.bits.manh | u.bits.manl | u.bits.exp) == 0)
		return (x);

	/* If x < 0, then raise invalid and return NaN */
	if (u.bits.sign)
                return __math_invalidl(x);

	feholdexcept(&env);

	if (u.bits.exp == 0) {
		/* Adjust subnormal numbers. */
		u.e *= 0x1.0p514L;
		k = -514;
	} else {
		k = 0;
	}
	/*
	 * u.e is a normal number, so break it into u.e = e*2^n where
	 * u.e = (2*e)*2^2k for odd n and u.e = (4*e)*2^2k for even n.
	 */
	if ((u.bits.exp - (BIAS-1)) & 1) {	/* n is odd.     */
		k += u.bits.exp - BIAS;	/* 2k = n - 1.   */
#ifdef _DOUBLE_DOUBLE_FLOAT
                u.dbits.dl = scalbn(u.dbits.dl, BIAS - u.bits.exp);
#endif
		u.bits.exp = BIAS;		/* u.e in [1,2). */
	} else {
                k += u.bits.exp - (BIAS + 1);	/* 2k = n - 2.   */
#ifdef _DOUBLE_DOUBLE_FLOAT
                u.dbits.dl = scalbn(u.dbits.dl, (BIAS + 1) - u.bits.exp);
#endif
		u.bits.exp = (BIAS + 1);	/* u.e in [2,4). */
	}

	/*
	 * Newton's iteration.
	 * Split u.e into a high and low part to achieve additional precision.
	 */
	xn = (long double)sqrt((double)u.e);			/* 53-bit estimate of sqrtl(x). */
	xn = (xn + (u.e / xn)) * 0.5L;	/* 106-bit estimate. */

	lo = u.e;
#ifdef _DOUBLE_DOUBLE_FLOAT
        u.dbits.dl = 0.0;               /* Zero out lower double */
#else
	u.bits.manl = 0;		/* Zero out lower bits. */
#endif
	lo = (lo - u.e) / xn;		/* Low bits divided by xn. */
	xn = xn + (u.e / xn);		/* High portion of estimate. */
	u.e = xn + lo;			/* Combine everything. */

	u.bits.exp += (k >> 1) - 1;
#ifdef _DOUBLE_DOUBLE_FLOAT
        u.dbits.dl = scalbn(u.dbits.dl, (k>>1) -1);
#endif

#if defined(FE_INEXACT) && defined(FE_TOWARDZERO) && defined(FE_TONEAREST) && defined(FE_UPWARD) && !defined(_DOUBLE_DOUBLE_FLOAT)
        {
                int r;
                feclearexcept(FE_INEXACT);
                r = fegetround();
                fesetround(FE_TOWARDZERO);	/* Set to round-toward-zero. */
                xn = x / u.e;			/* Chopped quotient (inexact?). */

                if (!fetestexcept(FE_INEXACT)) { /* Quotient is exact. */
                        if (xn == u.e) {
                                fesetenv(&env);
                                return (u.e);
                        }
                        /* Round correctly for inputs like x = y**2 - ulp. */
                        xn = dec(xn);		/* xn = xn - ulp. */
                }

                if (r == FE_TONEAREST) {
                        xn = inc(xn);		/* xn = xn + ulp. */
                } else if (r == FE_UPWARD) {
                        u.e = inc(u.e);		/* u.e = u.e + ulp. */
                        xn = inc(xn);		/* xn  = xn + ulp. */
                }
                u.e = u.e + xn;				/* Chopped sum. */
                feupdateenv(&env);	/* Restore env and raise inexact */
                u.bits.exp--;
        }
#endif
	return (u.e);
}
