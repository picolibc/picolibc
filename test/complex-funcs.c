#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>

double complex c1, c2;
float complex f1, f2;
#ifdef _TEST_LONG_DOUBLE
long double complex l1, l2;
#endif

/*
 * Touch test to make sure all of the expected complex functions exist
 */

int
main(void)
{
    /* 7.3.5 Trigonometric functions */
    /* 7.3.5.1 The cacos functions */
    (void) cacos(c1);
    (void) cacosf(f1);

    /* 7.3.5.2 The casin functions */
    (void) casin(c1);
    (void) casinf(f1);
#ifdef _LDBL_EQ_DBL
    (void) casinl(l1);
#endif

    /* 7.3.5.1 The catan functions */
    (void) catan(c1);
    (void) catanf(f1);
#ifdef _LDBL_EQ_DBL
    (void) catanl(l1);
#endif

    /* 7.3.5.1 The ccos functions */
    (void) ccos(c1);
    (void) ccosf(f1);

    /* 7.3.5.1 The csin functions */
    (void) csin(c1);
    (void) csinf(f1);

    /* 7.3.5.1 The ctan functions */
    (void) ctan(c1);
    (void) ctanf(f1);

    /* 7.3.6 Hyperbolic functions */
    /* 7.3.6.1 The cacosh functions */
    (void) cacosh(c1);
    (void) cacoshf(f1);

    /* 7.3.6.2 The casinh functions */
    (void) casinh(c1);
    (void) casinhf(f1);

    /* 7.3.6.3 The catanh functions */
    (void) catanh(c1);
    (void) catanhf(f1);

    /* 7.3.6.4 The ccosh functions */
    (void) ccosh(c1);
    (void) ccoshf(f1);

    /* 7.3.6.5 The csinh functions */
    (void) csinh(c1);
    (void) csinhf(f1);

    /* 7.3.6.6 The ctanh functions */
    (void) ctanh(c1);
    (void) ctanhf(f1);

    /* 7.3.7 Exponential and logarithmic functions */
    /* 7.3.7.1 The cexp functions */
    (void) cexp(c1);
    (void) cexpf(f1);

    /* 7.3.7.2 The clog functions */
    (void) clog(c1);
    (void) clogf(f1);
#ifdef _LDBL_EQ_DBL
    (void) clogl(l1);
#endif

    /* 7.3.8 Power and absolute-value functions */
    /* 7.3.8.1 The cabs functions */
    /*#ifndef __LIBM0_SOURCE__ */
    /* avoid conflict with historical cabs(struct complex) */
    /* double cabs(c1) __RENAME(__c99_cabs);
       float cabsf(f1) __RENAME(__c99_cabsf);
       #endif
    */
#ifdef _TEST_LONG_DOUBLE
    (void) cabsl(l1) ;
#endif
    (void) cabs(c1) ;
    (void) cabsf(f1) ;

    /* 7.3.8.2 The cpow functions */
    (void) cpow(c1, c2);
    (void) cpowf(f1, f2);

    /* 7.3.8.3 The csqrt functions */
    (void) csqrt(c1);
    (void) csqrtf(f1);
#ifdef _TEST_LONG_DOUBLE
    (void) csqrtl(l1);
#endif

    /* 7.3.9 Manipulation functions */
    /* 7.3.9.1 The carg functions */ 
    (void) carg(c1);
    (void) cargf(f1);
#ifdef _LDBL_EQ_DBL
    (void) cargl(l1);
#endif

    /* 7.3.9.2 The cimag functions */
    (void) cimag(c1);
    (void) cimagf(f1);
#ifdef _TEST_LONG_DOUBLE
    (void) cimagl(l1);
#endif

    /* 7.3.9.3 The conj functions */
    (void) conj(c1);
    (void) conjf(f1);

    /* 7.3.9.4 The cproj functions */
    (void) cproj(c1);
    (void) cprojf(f1);

    /* 7.3.9.5 The creal functions */
    (void) creal(c1);
    (void) crealf(f1);
#ifdef _TEST_LONG_DOUBLE
    (void) creall(l1);
#endif

    (void) clog10(c1);
    (void) clog10f(f1);

#ifdef _LDBL_EQ_DBL
    (void) cacosl(l1);
    (void) ccosl(l1);
    (void) csinl(l1);
    (void) ctanl(l1);
    (void) cacoshl(l1);
    (void) casinhl(l1);
    (void) catanhl(l1);
    (void) ccoshl(l1);
    (void) csinhl(l1);
    (void) ctanhl(l1);
    (void) cexpl(l1);
    (void) cpowl(l1, l2);
#endif
#ifdef _TEST_LONG_DOUBLE
    (void) conjl(l1);
    (void) cprojl(l1);
#endif

    return 0;
}
