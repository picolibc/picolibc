/* @(#)e_fmod.c 1.3 95/01/18 */
/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */




#define	BIAS (LDBL_MAX_EXP - 1)

/*
 * These macros add and remove an explicit integer bit in front of the
 * fractional mantissa, if the architecture doesn't have such a bit by
 * default already.
 */
#ifdef LDBL_IMPLICIT_NBIT
#define	LDBL_NBIT	0
#define	SET_NBIT(hx)	((hx) | (1ULL << LDBL_MANH_SIZE))
#define	HFRAC_BITS	LDBL_MANH_SIZE
#else
#define	LDBL_NBIT	0x80000000
#define	SET_NBIT(hx)	(hx)
#define	HFRAC_BITS	(LDBL_MANH_SIZE - 1)
#endif

#define	MANL_SHIFT	(LDBL_MANL_SIZE - 1)

static const long double one = 1.0l, Zero[] = {0.0l, -0.0l,};

/*
 * fmodl(x,y)
 * Return x mod y in exact arithmetic
 * Method: shift and subtract
 *
 * Assumptions:
 * - The low part of the mantissa fits in a manl_t exactly.
 * - The high part of the mantissa fits in an int64_t with enough room
 *   for an explicit integer bit in front of the fractional bits.
 */
long double
fmodl(long double x, long double y)
{
        union IEEEl2bits ux, uy;
	int64_t hx,hz;	/* We need a carry bit even if LDBL_MANH_SIZE is 32. */
	uint32_t hy;
	uint32_t lx,ly,lz;
	int ix,iy,n,sx;

	ux.e = x;
	uy.e = y;
	sx = ux.bits.sign;

        if (isnanl_inline(x) || isnanl_inline(y))
            return x + y;

        if (isinfl(x))
            return __math_invalidl(x);

        if (y == 0.0L)
            return __math_invalidl(y);

	if(ux.bits.exp<=uy.bits.exp) {
	    if((ux.bits.exp<uy.bits.exp) ||
	       (ux.bits.manh<=uy.bits.manh &&
		(ux.bits.manh<uy.bits.manh ||
		 ux.bits.manl<uy.bits.manl))) {
		return x;		/* |x|<|y| return x or x-y */
	    }
	    if(ux.bits.manh==uy.bits.manh &&
		ux.bits.manl==uy.bits.manl) {
		return Zero[sx];	/* |x|=|y| return x*0*/
	    }
	}

    /* determine ix = ilogb(x) */
	if(ux.bits.exp == 0) {	/* subnormal x */
	    ux.e *= 0x1.0p512l;
	    ix = ux.bits.exp - (BIAS + 512);
	} else {
	    ix = ux.bits.exp - BIAS;
	}

    /* determine iy = ilogb(y) */
	if(uy.bits.exp == 0) {	/* subnormal y */
	    uy.e *= 0x1.0p512l;
	    iy = uy.bits.exp - (BIAS + 512);
	} else {
	    iy = uy.bits.exp - BIAS;
	}

    /* set up {hx,lx}, {hy,ly} and align y to x */
	hx = SET_NBIT(ux.bits.manh);
	hy = SET_NBIT(uy.bits.manh);
	lx = ux.bits.manl;
	ly = uy.bits.manl;

    /* fix point fmod */
	n = ix - iy;

	while(n--) {
	    hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	    if(hz<0){hx = hx+hx+(lx>>MANL_SHIFT); lx = lx+lx;}
	    else {
		if ((hz|lz)==0)		/* return sign(x)*0 */
		    return Zero[sx];
		hx = hz+hz+(lz>>MANL_SHIFT); lx = lz+lz;
	    }
	}
	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0) {hx=hz;lx=lz;}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0)			/* return sign(x)*0 */
	    return Zero[sx];
	while(hx<(1LL<<HFRAC_BITS)) {	/* normalize x */
	    hx = hx+hx+(lx>>MANL_SHIFT); lx = lx+lx;
	    iy -= 1;
	}
	ux.bits.manh = hx; /* The mantissa is truncated here if needed. */
	ux.bits.manl = lx;
	if (iy < LDBL_MIN_EXP) {
	    ux.bits.exp = iy + (BIAS + 512);
	    ux.e *= 0x1p-512l;
	} else {
	    ux.bits.exp = iy + BIAS;
	}
	x = ux.e * one;		/* create necessary signal */
	return x;		/* exact output */
}
