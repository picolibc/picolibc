/* @(#)s_scalbn.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * scalbnl (long double x, int n)
 * scalbnl(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

/*
 * We assume that a long double has a 15-bit exponent.  On systems
 * where long double is the same as double, scalbnl() is an alias
 * for scalbn(), so we don't use this routine.
 */


#if LDBL_MAX_EXP == 0x4000

long double
scalbnl (long double x, int n)
{
	union IEEEl2bits u;
	__int32_t k;
	u.e = x;
        k = u.bits.exp;				/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((u.bits.manh|u.bits.manl)==0) return x;	/* +-0 */
	    u.e *= 0x1p+128L;
	    k = u.bits.exp - 128;
            if (n< -50000) return __math_uflowl(u.bits.sign);
	    }
        if (k==0x7fff) return x+x;		/* NaN or Inf */
#if __SIZEOF_INT__ > 2
        if (n > 50000) 	/* in case integer overflow in n+k */
            return __math_oflowl(u.bits.sign);	/*overflow*/
#endif
        k = k+n;
        if (k >= 0x7fff) return __math_oflowl(u.bits.sign); /* overflow  */
        if (k > 0) 				/* normal result */
	    {u.bits.exp = k; return u.e;}
        if (k <= -128)
	    return __math_uflowl(u.bits.sign); 	/*underflow*/
	k += 128;				/* subnormal result */
	u.bits.exp = k;
        return check_uflowl(u.e*0x1p-128L);
}

#elif defined(_DOUBLE_DOUBLE_FLOAT)

long double
scalbnl(long double x, int n)
{
	union IEEEl2bits u;
        double dh, dl;

        u.e = x;
        dh = scalbn(u.dbits.dh, n);
        dl = scalbn(u.dbits.dl, n);
        return (long double) dh + (long double) dl;
}

#endif

#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(scalbnl, ldexpl);
#else
long double
ldexpl(long double x, int n)
{
    return scalbnl(x, n);
}
#endif
