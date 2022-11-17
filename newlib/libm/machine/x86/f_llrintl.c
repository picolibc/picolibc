/*
Copyright (c) 2007 Dave Korn 


x87 FP implementation contributed to Newlib by
Dave Korn, November 2007.  This file is placed in the
public domain.  Permission to use, copy, modify, and 
distribute this software is freely granted.
 */
/*
 * ====================================================
 * x87 FP implementation contributed to Newlib by
 * Dave Korn, November 2007.  This file is placed in the
 * public domain.  Permission to use, copy, modify, and 
 * distribute this software is freely granted.
 * ====================================================
 */

#ifdef __GNUC__
#if !defined(_SOFT_FLOAT)

#include <math.h>

/*
 * Fast math version of llrintl(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using inline x87 asms.
 * Exception:
 *	Governed by x87 FPCR.
 */

long long int _f_llrintl (long double x)
{
  long long int _result;
  __asm__("fistpll %0" : "=m" (_result) : "t" (x) : "st");
  return _result;
}

#endif /* !_SOFT_FLOAT */
#endif /* __GNUC__ */
