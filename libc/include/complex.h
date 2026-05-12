/* $NetBSD: complex.h,v 1.3 2010/09/15 16:11:30 christos Exp $ */

/*
 * Copyright (c) 2010 Matthias Drochner.
 * Public domain.
 */

#ifndef _COMPLEX_H
#define _COMPLEX_H

#include <sys/cdefs.h>

#define complex    _Complex
#define _Complex_I 1.0fi
#define I          _Complex_I

_BEGIN_STD_C

/* 7.3.5 Trigonometric functions */
/* 7.3.5.1 The cacos functions */
double complex cacos(double complex) __picolibc_export;
float complex  cacosf(float complex) __picolibc_export;

/* 7.3.5.2 The casin functions */
double complex casin(double complex) __picolibc_export;
float complex  casinf(float complex) __picolibc_export;

/* 7.3.5.1 The catan functions */
double complex catan(double complex) __picolibc_export;
float complex  catanf(float complex) __picolibc_export;

/* 7.3.5.1 The ccos functions */
double complex ccos(double complex) __picolibc_export;
float complex  ccosf(float complex) __picolibc_export;

/* 7.3.5.1 The csin functions */
double complex csin(double complex) __picolibc_export;
float complex  csinf(float complex) __picolibc_export;

/* 7.3.5.1 The ctan functions */
double complex ctan(double complex) __picolibc_export;
float complex  ctanf(float complex) __picolibc_export;

/* 7.3.6 Hyperbolic functions */
/* 7.3.6.1 The cacosh functions */
double complex cacosh(double complex) __picolibc_export;
float complex  cacoshf(float complex) __picolibc_export;

/* 7.3.6.2 The casinh functions */
double complex casinh(double complex) __picolibc_export;
float complex  casinhf(float complex) __picolibc_export;

/* 7.3.6.3 The catanh functions */
double complex catanh(double complex) __picolibc_export;
float complex  catanhf(float complex) __picolibc_export;

/* 7.3.6.4 The ccosh functions */
double complex ccosh(double complex) __picolibc_export;
float complex  ccoshf(float complex) __picolibc_export;

/* 7.3.6.5 The csinh functions */
double complex csinh(double complex) __picolibc_export;
float complex  csinhf(float complex) __picolibc_export;

/* 7.3.6.6 The ctanh functions */
double complex ctanh(double complex) __picolibc_export;
float complex  ctanhf(float complex) __picolibc_export;

/* 7.3.7 Exponential and logarithmic functions */
/* 7.3.7.1 The cexp functions */
double complex cexp(double complex) __picolibc_export;
float complex  cexpf(float complex) __picolibc_export;

/* 7.3.7.2 The clog functions */
double complex clog(double complex) __picolibc_export;
float complex  clogf(float complex) __picolibc_export;

/* 7.3.8 Power and absolute-value functions */
/* 7.3.8.1 The cabs functions */
double         cabs(double complex) __picolibc_export;
float          cabsf(float complex) __picolibc_export;

/* 7.3.8.2 The cpow functions */
double complex cpow(double complex, double complex) __picolibc_export;
float complex  cpowf(float complex, float complex) __picolibc_export;

/* 7.3.8.3 The csqrt functions */
double complex csqrt(double complex) __picolibc_export;
float complex  csqrtf(float complex) __picolibc_export;

/* 7.3.9 Manipulation functions */
/* 7.3.9.1 The carg functions */
double         carg(double complex) __picolibc_export;
float          cargf(float complex) __picolibc_export;

/* 7.3.9.2 The cimag functions */
double         cimag(double complex) __picolibc_export;
float          cimagf(float complex) __picolibc_export;

/* 7.3.9.3 The conj functions */
double complex conj(double complex) __picolibc_export;
float complex  conjf(float complex) __picolibc_export;

/* 7.3.9.4 The cproj functions */
double complex cproj(double complex) __picolibc_export;
float complex  cprojf(float complex) __picolibc_export;

/* 7.3.9.5 The creal functions */
double         creal(double complex) __picolibc_export;
float          crealf(float complex) __picolibc_export;

#if __ISO_C_VISIBLE >= 2011
#if __HAVE_BUILTIN_COMPLEX
#define CMPLX(r, i)  __builtin_complex((double)(r), (double)(i))
#define CMPLXF(r, i) __builtin_complex((float)(r), (float)(i))
#define CMPLXL(r, i) __builtin_complex((long double)(r), (long double)(i))
#else
#define CMPLX(r, i)  ((double complex)((double)(r) + (double complex)I * (double)(i)))
#define CMPLXF(r, i) ((float complex)((float)(r) + (float complex)I * (float)(i)))
#define CMPLXL(r, i)                                                                      \
    ((long double complex)((long double)(r) + (long double complex)I * (long double)(i)))
#endif
#endif

#if __GNU_VISIBLE
double complex clog10(double complex) __picolibc_export;
float complex  clog10f(float complex) __picolibc_export;
#endif

#ifdef __HAVE_LONG_DOUBLE
long double complex csqrtl(long double complex) __picolibc_export;
long double         cabsl(long double complex) __picolibc_export;
long double complex cprojl(long double complex) __picolibc_export;
long double         creall(long double complex) __picolibc_export;
long double complex conjl(long double complex) __picolibc_export;
long double         cimagl(long double complex) __picolibc_export;

#ifdef __HAVE_LONG_DOUBLE_MATH
long double         cargl(long double complex) __picolibc_export;
long double complex casinl(long double complex) __picolibc_export;
long double complex cacosl(long double complex) __picolibc_export;
long double complex catanl(long double complex) __picolibc_export;
long double complex ccosl(long double complex) __picolibc_export;
long double complex csinl(long double complex) __picolibc_export;
long double complex ctanl(long double complex) __picolibc_export;
long double complex cacoshl(long double complex) __picolibc_export;
long double complex casinhl(long double complex) __picolibc_export;
long double complex catanhl(long double complex) __picolibc_export;
long double complex ccoshl(long double complex) __picolibc_export;
long double complex csinhl(long double complex) __picolibc_export;
long double complex ctanhl(long double complex) __picolibc_export;
long double complex cexpl(long double complex) __picolibc_export;
long double complex clogl(long double complex) __picolibc_export;
long double complex cpowl(long double complex, long double complex) __picolibc_export;
#if __GNU_VISIBLE
long double complex clog10l(long double complex) __picolibc_export;
#endif
#endif /* __HAVE_LONG_DOUBLE_MATH */

#endif /* __HAVE_LONG_DOUBLE */

_END_STD_C

#endif /* ! _COMPLEX_H */
