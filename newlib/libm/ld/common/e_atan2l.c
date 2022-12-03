
/* @(#)e_atan2.c 1.3 95/01/18 */
/* FreeBSD: head/lib/msun/src/e_atan2.c 176451 2008-02-22 02:30:36Z das */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

//__FBSDID("$FreeBSD: src/lib/msun/src/e_atan2l.c,v 1.3 2008/08/02 19:17:00 das Exp $");

/*
 * See comments in e_atan2.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */


#include "invtrig.h"

static const long double tiny = 1.0e-300l;
static const long double zero = 0.0l;
static const long double pi   = _M_PI_L;

long double
atan2l(long double y, long double x)
{
	union IEEEl2bits ux, uy;
	long double z;
	int32_t k,m;
	int16_t exptx, expsignx, expty, expsigny;

	uy.e = y;
	expsigny = uy.xbits.expsign;
	expty = expsigny & 0x7fff;
	ux.e = x;
	expsignx = ux.xbits.expsign;
	exptx = expsignx & 0x7fff;

	if ((exptx==BIAS+LDBL_MAX_EXP &&
	     ((ux.bits.manh&~LDBL_NBIT)|ux.bits.manl)!=0) ||	/* x is NaN */
	    (expty==BIAS+LDBL_MAX_EXP &&
	     ((uy.bits.manh&~LDBL_NBIT)|uy.bits.manl)!=0))	/* y is NaN */
	    return x+y;
	if (expsignx==BIAS && ((ux.bits.manh&~LDBL_NBIT)|ux.bits.manl)==0)
	    return atanl(y);					/* x=1.0 */
	m = ((expsigny>>15)&1)|((expsignx>>14)&2);	/* 2*sign(x)+sign(y) */

    /* when y = 0 */
	if(expty==0 && ((uy.bits.manh&~LDBL_NBIT)|uy.bits.manl)==0) {
	    switch(m) {
		case 0: 
		case 1: return y; 	/* atan(+-0,+anything)=+-0 */
		case 2: return  pi+tiny;/* atan(+0,-anything) = pi */
		case 3: return -pi-tiny;/* atan(-0,-anything) =-pi */
	    }
	}
    /* when x = 0 */
	if(exptx==0 && ((ux.bits.manh&~LDBL_NBIT)|ux.bits.manl)==0)
	    return (expsigny<0)?  -pio2_hi-tiny: pio2_hi+tiny;

    /* when x is INF */
	if(exptx==BIAS+LDBL_MAX_EXP) {
	    if(expty==BIAS+LDBL_MAX_EXP) {
		switch(m) {
		    case 0: return  pio2_hi*0.5l+tiny;/* atan(+INF,+INF) */
		    case 1: return -pio2_hi*0.5l-tiny;/* atan(-INF,+INF) */
		    case 2: return  1.5l*pio2_hi+tiny;/*atan(+INF,-INF)*/
		    case 3: return -1.5l*pio2_hi-tiny;/*atan(-INF,-INF)*/
		}
	    } else {
		switch(m) {
		    case 0: return  zero  ;	/* atan(+...,+INF) */
		    case 1: return -zero  ;	/* atan(-...,+INF) */
		    case 2: return  pi+tiny  ;	/* atan(+...,-INF) */
		    case 3: return -pi-tiny  ;	/* atan(-...,-INF) */
		}
	    }
	}
    /* when y is INF */
	if(expty==BIAS+LDBL_MAX_EXP)
	    return (expsigny<0)? -pio2_hi-tiny: pio2_hi+tiny;

    /* compute y/x */
	k = expty-exptx;
	if(k > LDBL_MANT_DIG+2) {			/* |y/x| huge */
	    z=pio2_hi+pio2_lo;
	    m&=1;
	}
	else if(expsignx<0&&k<-LDBL_MANT_DIG-2) z=0.0l; 	/* |y/x| tiny, x<0 */
	else z=atanl(check_uflowl(fabsl(y/x)));		/* safe to do y/x */
	switch (m) {
	    case 0: return       z  ;	/* atan(+,+) */
	    case 1: return      -z  ;	/* atan(-,+) */
	    case 2: return  pi-(z-pi_lo);/* atan(+,-) */
	    default: /* case 3 */
	    	    return  (z-pi_lo)-pi;/* atan(-,-) */
	}
}

#if __LDBL_MANT_DIG__ == 113
#if defined(_HAVE_ALIAS_ATTRIBUTE)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(atan2l, __atan2ieee128);
#endif
#endif
