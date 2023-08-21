/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
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

//__FBSDID("$FreeBSD: src/lib/msun/src/s_fmal.c,v 1.7 2011/10/21 06:30:43 das Exp $");

/*
 * Denorms usually have an exponent biased by 1 so that they flow
 * smoothly into the smallest normal value with an exponent of
 * 1. However, m68k 80-bit long doubles includes exponent of zero for
 * normal values, so denorms use the same value, eliminating the
 * bias. That is set in s_fmal.c.
 */

#ifndef FLOAT_DENORM_BIAS
#define FLOAT_DENORM_BIAS   1
#endif

/*
 * A struct dd represents a floating-point number with twice the precision
 * of a FLOAT_T.  We maintain the invariant that "hi" stores the high-order
 * bits of the result.
 */
struct dd {
	FLOAT_T hi;
	FLOAT_T lo;
};

/*
 * Compute a+b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are finite, but make no assumptions about their relative
 * magnitudes.
 */
static inline struct dd
dd_add(FLOAT_T a, FLOAT_T b)
{
	struct dd ret;
	FLOAT_T s;

	ret.hi = a + b;
	s = ret.hi - a;
	ret.lo = (a - (ret.hi - s)) + (b - s);
	return (ret);
}

/*
 * Compute a+b, with a small tweak:  The least significant bit of the
 * result is adjusted into a sticky bit summarizing all the bits that
 * were lost to rounding.  This adjustment negates the effects of double
 * rounding when the result is added to another number with a higher
 * exponent.  For an explanation of round and sticky bits, see any reference
 * on FPU design, e.g.,
 *
 *     J. Coonen.  An Implementation Guide to a Proposed Standard for
 *     Floating-Point Arithmetic.  Computer, vol. 13, no. 1, Jan 1980.
 */
static inline FLOAT_T
add_adjusted(FLOAT_T a, FLOAT_T b)
{
	struct dd sum;

	sum = dd_add(a, b);
	if (sum.lo != 0) {
		if (!odd_mant(sum.hi))
			sum.hi = NEXTAFTER(sum.hi, (FLOAT_T)INFINITY * sum.lo);
	}
	return (sum.hi);
}

/*
 * Compute ldexp(a+b, scale) with a single rounding error. It is assumed
 * that the result will be subnormal, and care is taken to ensure that
 * double rounding does not occur.
 */
static inline FLOAT_T
add_and_denormalize(FLOAT_T a, FLOAT_T b, int scale)
{
	struct dd sum;
	int bits_lost;

	sum = dd_add(a, b);

	/*
	 * If we are losing at least two bits of accuracy to denormalization,
	 * then the first lost bit becomes a round bit, and we adjust the
	 * lowest bit of sum.hi to make it a sticky bit summarizing all the
	 * bits in sum.lo. With the sticky bit adjusted, the hardware will
	 * break any ties in the correct direction.
	 *
	 * If we are losing only one bit to denormalization, however, we must
	 * break the ties manually.
	 */
	if (sum.lo != 0) {
		bits_lost = -EXPONENT(sum.hi) - scale + FLOAT_DENORM_BIAS;
		if ((bits_lost != 1) ^ (int)odd_mant(sum.hi))
                        sum.hi = NEXTAFTER(sum.hi, (FLOAT_T)INFINITY * sum.lo);
	}
	return (LDEXP(sum.hi, scale));
}

/*
 * Compute a*b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are normalized, so no underflow or overflow will occur.
 * The current rounding mode must be round-to-nearest.
 */
static inline struct dd
dd_mul(FLOAT_T a, FLOAT_T b)
{
	static const FLOAT_T split = SPLIT;
	struct dd ret;
	FLOAT_T ha, hb, la, lb, p, q;

	p = a * split;
	ha = a - p;
	ha += p;
	la = a - ha;

	p = b * split;
	hb = b - p;
	hb += p;
	lb = b - hb;

	p = ha * hb;
	q = ha * lb + la * hb;

	ret.hi = p + q;
	ret.lo = p - ret.hi + q + la * lb;
	return (ret);
}

#ifdef _WANT_MATH_ERRNO
static FLOAT_T
_scalbn_no_errno(FLOAT_T x, int n)
{
        int save_errno = errno;
        x = SCALBN(x, n);
        errno = save_errno;
        return x;
}
#else
#define _scalbn_no_errno(a,b) SCALBN(a,b)
#endif

#ifdef __clang__
#pragma STDC FP_CONTRACT OFF
#endif

#if defined(FE_UPWARD) || defined(FE_DOWNWARD) || defined(FE_TOWARDZERO)
#define HAS_ROUNDING
#endif

/*
 * Fused multiply-add: Compute x * y + z with a single rounding error.
 *
 * We use scaling to avoid overflow/underflow, along with the
 * canonical precision-doubling technique adapted from:
 *
 *	Dekker, T.  A Floating-Point Technique for Extending the
 *	Available Precision.  Numer. Math. 18, 224-242 (1971).
 */
FLOAT_T
FMA(FLOAT_T x, FLOAT_T y, FLOAT_T z)
{
	FLOAT_T xs, ys, zs, adj;
	struct dd xy, r;
	int ex, ey, ez;
	int spread;

	/*
	 * Handle special cases. The order of operations and the particular
	 * return values here are crucial in handling special cases involving
	 * infinities, NaNs, overflows, and signed zeroes correctly.
	 */
        if (!isfinite(z) && isfinite(x) && isfinite(y))
                return z + z;
	if (!isfinite(x) || !isfinite(y) || !isfinite(z))
		return (x * y + z);
	if (x == (FLOAT_T) 0.0 || y == (FLOAT_T) 0.0)
		return (x * y + z);
	if (z == (FLOAT_T) 0.0)
		return (x * y);

	xs = FREXP(x, &ex);
	ys = FREXP(y, &ey);
	zs = FREXP(z, &ez);
#ifdef HAS_ROUNDING
	int oround = fegetround();
#endif
	spread = ex + ey - ez;

	/*
	 * If x * y and z are many orders of magnitude apart, the scaling
	 * will overflow, so we handle these cases specially.  Rounding
	 * modes other than FE_TONEAREST are painful.
	 */
	if (spread < -FLOAT_MANT_DIG) {
#ifdef FE_INEXACT
		feraiseexcept(FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
		if (!isnormal(z))
			feraiseexcept(FE_UNDERFLOW);
#endif
#ifdef HAS_ROUNDING
		switch (oround) {
		default:
                        break;
#ifdef FE_TOWARDZERO
		case FE_TOWARDZERO:
			if ((x > (FLOAT_T) 0.0) ^ (y < (FLOAT_T) 0.0) ^ (z < (FLOAT_T) 0.0))
				break;
			else
				return (NEXTAFTER(z, 0));
#endif
#ifdef FE_DOWNWARD
		case FE_DOWNWARD:
			if ((x > (FLOAT_T) 0.0) ^ (y < (FLOAT_T) 0.0))
				break;
			else
				return (NEXTAFTER(z, -(FLOAT_T)INFINITY));
#endif
#ifdef FE_UPWARD
                case FE_UPWARD:
			if ((x > (FLOAT_T) 0.0) ^ (y < (FLOAT_T) 0.0))
				return (NEXTAFTER(z, (FLOAT_T)INFINITY));
                        break;
#endif
		}
#endif
                return (z);
	}
	if (spread <= FLOAT_MANT_DIG * 2)
		zs = _scalbn_no_errno(zs, -spread);
	else
		zs = COPYSIGN(FLOAT_MIN, zs);

#ifdef HAS_ROUNDING
	fesetround(FE_TONEAREST);
#endif

	/*
	 * Basic approach for round-to-nearest:
	 *
	 *     (xy.hi, xy.lo) = x * y		(exact)
	 *     (r.hi, r.lo)   = xy.hi + z	(exact)
	 *     adj = xy.lo + r.lo		(inexact; low bit is sticky)
	 *     result = r.hi + adj		(correctly rounded)
	 */
	xy = dd_mul(xs, ys);
	r = dd_add(xy.hi, zs);

	spread = ex + ey;

	if (r.hi == (FLOAT_T) 0.0) {
		/*
		 * When the addends cancel to 0, ensure that the result has
		 * the correct sign.
		 */
#ifdef HAS_ROUNDING
		fesetround(oround);
#endif
		volatile FLOAT_T vzs = zs; /* XXX gcc CSE bug workaround */
		return (xy.hi + vzs + _scalbn_no_errno(xy.lo, spread));
	}

#ifdef HAS_ROUNDING
	if (oround != FE_TONEAREST) {
		/*
		 * There is no need to worry about double rounding in directed
		 * rounding modes.
		 */
		fesetround(oround);
		adj = r.lo + xy.lo;
		return (_scalbn_no_errno(r.hi + adj, spread));
	}
#endif

	adj = add_adjusted(r.lo, xy.lo);
	if (spread + ILOGB(r.hi) > -(FLOAT_MAX_EXP - FLOAT_DENORM_BIAS))
		return (_scalbn_no_errno(r.hi + adj, spread));
	else
		return (add_and_denormalize(r.hi, adj, spread));
}
