/*
 * complex.h
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Danny Smith <dannysmith@users.sourceforge.net>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef _COMPLEX_H_
#define _COMPLEX_H_

/* All the headers include this file. */
#include <_mingw.h>

#if (defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
     || !defined __STRICT_ANSI__ 

/* These macros are specified by C99 standard */

#ifndef __cplusplus
#define complex _Complex
#endif

#define _Complex_I  (0.0F +  1.0iF)

/* GCC doesn't support _Imaginary type yet, so we don't
   define _Imaginary_I */

#define I _Complex_I

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef RC_INVOKED

/* TODO: float and long double versions */ 
double __attribute__ ((const)) creal (double _Complex);
double __attribute__ ((const)) cimag (double _Complex);
double __attribute__ ((const)) carg (double _Complex);
double __attribute__ ((const)) cabs (double _Complex);
double _Complex __attribute__ ((const)) cconj (double _Complex);
double _Complex __attribute__ ((const)) cacos (double _Complex);
double _Complex __attribute__ ((const)) casin (double _Complex);
double _Complex __attribute__ ((const)) catan (double _Complex);
double _Complex __attribute__ ((const)) ccos (double _Complex);
double _Complex __attribute__ ((const)) csin (double _Complex);
double _Complex __attribute__ ((const)) ctan (double _Complex);
double _Complex __attribute__ ((const)) cacosh (double _Complex);
double _Complex __attribute__ ((const)) casinh (double _Complex);
double _Complex __attribute__ ((const)) catanh (double _Complex);
double _Complex __attribute__ ((const)) ccosh (double _Complex);
double _Complex __attribute__ ((const)) csinh (double _Complex);
double _Complex __attribute__ ((const)) ctanh (double _Complex);
double _Complex __attribute__ ((const)) cexp (double _Complex);
double _Complex __attribute__ ((const)) clog (double _Complex);
double _Complex __attribute__ ((const)) cpow (double _Complex, double _Complex);
double _Complex __attribute__ ((const)) csqrt (double _Complex);
double _Complex __attribute__ ((const)) cproj (double _Complex);

#ifdef __GNUC__

__CRT_INLINE double __attribute__ ((const)) creal (double _Complex _Z)
{
  return __real__ _Z;
}

__CRT_INLINE double __attribute__ ((const)) cimag (double _Complex _Z)
{
  return __imag__ _Z;
}

__CRT_INLINE double _Complex __attribute__ ((const)) conj (double _Complex _Z)
{
  return __extension__ ~_Z;
}

__CRT_INLINE  double __attribute__ ((const)) carg (double _Complex _Z)
{
  double res;
  __asm__  ("fpatan;"
	   : "=t" (res) : "0" (__real__ _Z), "u" (__imag__ _Z) : "st(1)");
  return res;
}
#endif /* __GNUC__ */


#endif /* RC_INVOKED */

#ifdef __cplusplus
}
#endif 

#endif /* __STDC_VERSION__ >= 199901L */


#endif /* _COMPLEX_H */
