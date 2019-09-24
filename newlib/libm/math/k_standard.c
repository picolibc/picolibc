
/* @(#)k_standard.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _USE_WRITE
#include <stdio.h>			/* fputs(), stderr */
#define	WRITE2(u,v)	fputs(u, stderr)
#else	/* !defined(_USE_WRITE) */
#include <unistd.h>			/* write */
#define	WRITE2(u,v)	write(2, u, v)
#undef fflush
#endif	/* !defined(_USE_WRITE) */

#ifdef __STDC__
static const double zero = 0.0;	/* used as const */
#else
static double zero = 0.0;	/* used as const */
#endif

/* 
 * POSIX Standard conformance on exception cases.
 * Mapping:
 *	1 -- acos(|x|>1)
 *	2 -- asin(|x|>1)
 *	3 -- atan2(+-0,+-0)
 *	4 -- hypot overflow
 *	5 -- cosh overflow
 *	6 -- exp overflow
 *	7 -- exp underflow
 *	8 -- y0(0)
 *	9 -- y0(-ve)
 *	10-- y1(0)
 *	11-- y1(-ve)
 *	12-- yn(0)
 *	13-- yn(-ve)
 *	14-- lgamma(finite) overflow
 *	15-- lgamma(-integer)
 *	16-- log(0)
 *	17-- log(x<0)
 *	18-- log10(0)
 *	19-- log10(x<0)
 *	20-- pow(0.0,0.0)
 *	21-- pow(x,y) overflow
 *	22-- pow(x,y) underflow
 *	23-- pow(0,negative) 
 *	24-- pow(neg,non-integral)
 *	25-- sinh(finite) overflow
 *	26-- sqrt(negative)
 *      27-- fmod(x,0)
 *      28-- remainder(x,0)
 *	29-- acosh(x<1)
 *	30-- atanh(|x|>1)
 *	31-- atanh(|x|=1)
 *	32-- scalb overflow
 *	33-- scalb underflow
 *	34-- j0(|x|>X_TLOSS)
 *	35-- y0(x>X_TLOSS)
 *	36-- j1(|x|>X_TLOSS)
 *	37-- y1(x>X_TLOSS)
 *	38-- jn(|x|>X_TLOSS, n)
 *	39-- yn(x>X_TLOSS, n)
 *	40-- gamma(finite) overflow
 *	41-- gamma(-integer)
 *	42-- pow(NaN,0.0)
 */


#ifdef __STDC__
	double __kernel_standard(double x, double y, int type) 
#else
	double __kernel_standard(x,y,type) 
	double x,y; int type;
#endif
{
	double retval = 0.0;

#ifdef _USE_WRITE
        /* (void) fflush(_stdout_r(p)); */        
#endif
	switch(type) {
	    case 1:
	    case 101:
		/* acos(|x|>1) */
		retval = zero;
		errno = EDOM;
		break;
	    case 2:
	    case 102:
		/* asin(|x|>1) */
		retval = zero;
		errno = EDOM;
		break;
	    case 3:
	    case 103:
		/* atan2(+-0,+-0) */
		retval = zero;
		errno = EDOM;
		break;
	    case 4:
	    case 104:
		/* hypot(finite,finite) overflow */
		retval = HUGE_VAL;
		errno = ERANGE;
		break;
	    case 5:
	    case 105:
		/* cosh(finite) overflow */
		retval = HUGE_VAL;
		errno = ERANGE;
		break;
	    case 6:
	    case 106:
		/* exp(finite) overflow */
		retval = HUGE_VAL;
		errno = ERANGE;
		break;
	    case 7:
	    case 107:
		/* exp(finite) underflow */
		retval = zero;
		errno = ERANGE;
		break;
	    case 8:
	    case 108:
		/* y0(0) = -inf */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 9:
	    case 109:
		/* y0(x<0) = NaN */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 10:
	    case 110:
		/* y1(0) = -inf */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 11:
	    case 111:
		/* y1(x<0) = NaN */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 12:
	    case 112:
		/* yn(n,0) = -inf */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 13:
	    case 113:
		/* yn(x<0) = NaN */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 14:
	    case 114:
		/* lgamma(finite) overflow */
		retval = HUGE_VAL;
		errno = ERANGE;
		break;
	    case 15:
	    case 115:
		/* lgamma(-integer) or lgamma(0) */
		retval = HUGE_VAL;
		errno = EDOM;
		break;
	    case 16:
	    case 116:
		/* log(0) */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 17:
	    case 117:
		/* log(x<0) */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 18:
	    case 118:
		/* log10(0) */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 19:
	    case 119:
		/* log10(x<0) */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 20:
	    case 120:
		/* pow(0.0,0.0) */
		/* Not an error.  */
		retval = 1.0;
		break;
	    case 21:
	    case 121:
		/* pow(x,y) overflow */
		retval = HUGE_VAL;
		y *= 0.5;
		if(x<zero&&rint(y)!=y)
		  retval = -HUGE_VAL;
		errno = ERANGE;
		break;
	    case 22:
	    case 122:
		/* pow(x,y) underflow */
		retval =  zero;
		errno = ERANGE;
		break;
	    case 23:
	    case 123:
		/* 0**neg */
		retval = -HUGE_VAL;
		errno = EDOM;
		break;
	    case 24:
	    case 124:
		/* neg**non-integral */
		retval = zero/zero;
		errno = EDOM;
		break;
	    case 25:
	    case 125:
		/* sinh(finite) overflow */
		retval = ( (x>zero) ? HUGE_VAL : -HUGE_VAL);
		errno = ERANGE;
		break;
	    case 26:
	    case 126:
		/* sqrt(x<0) */
		retval = zero/zero;
		errno = EDOM;
		break;
            case 27:
	    case 127:
		/* fmod(x,0) */
		retval = zero/zero;
		errno = EDOM;
                break;
            case 28:
	    case 128:
                /* remainder(x,0) */
		retval = zero/zero;
		errno = EDOM;
                break;
            case 29:
	    case 129:
                /* acosh(x<1) */
		retval = zero/zero;
		errno = EDOM;
                break;
            case 30:
	    case 130:
                /* atanh(|x|>1) */
		retval = zero/zero;
		errno = EDOM;
                break;
            case 31:
	    case 131:
                /* atanh(|x|=1) */
		retval = x/zero;	/* sign(x)*inf */
		errno = EDOM;
                break;
	    case 32:
	    case 132:
		/* scalb overflow */
		retval = x > zero ? HUGE_VAL : -HUGE_VAL;
		errno = ERANGE;
		break;
	    case 33:
	    case 133:
		/* scalb underflow */
		retval = copysign(zero,x);
		errno = ERANGE;
		break;
	    case 34:
	    case 134:
		/* j0(|x|>X_TLOSS) */
		retval = zero;
		errno = ERANGE;
		break;
	    case 35:
	    case 135:
		/* y0(x>X_TLOSS) */
		retval = zero;
		errno = ERANGE;
		break;
	    case 36:
	    case 136:
		/* j1(|x|>X_TLOSS) */
		retval = zero;
		errno = ERANGE;
		break;
	    case 37:
	    case 137:
		/* y1(x>X_TLOSS) */
		retval = zero;
		errno = ERANGE;
		break;
	    case 38:
	    case 138:
		/* jn(|x|>X_TLOSS) */
		retval = zero;
		errno = ERANGE;
		break;
	    case 39:
	    case 139:
		/* yn(x>X_TLOSS) */
		retval = zero;
		errno = ERANGE;
		break;
	    case 40:
	    case 140:
		/* gamma(finite) overflow */
		retval = HUGE_VAL;
		errno = ERANGE;
		break;
	    case 41:
	    case 141:
		/* gamma(-integer) or gamma(0) */
		retval = HUGE_VAL;
		errno = EDOM;
		break;
	    case 42:
	    case 142:
		/* pow(NaN,0.0) */
		/* Not an error.  */
		retval = 1.0;
		break;
	}
	return retval;
}


