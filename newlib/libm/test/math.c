/*
 * Copyright (c) 1994 Cygnus Support.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
  Test the library maths functions using trusted precomputed test
  vectors.

  These vectors were originally generated on a sun3 with a 68881 using
  80 bit precision, but ...

  Each function is called with a variety of interesting arguments.
  Note that many of the polynomials we use behave badly when the
  domain is stressed, so the numbers in the vectors depend on what is
  useful to test - eg sin(1e30) is pointless - the arg has to be
  reduced modulo pi, and after that there's no bits of significance
  left to evaluate with - any number would be just as precise as any
  other.


*/

#include "test.h"
#include <math.h>
#include <float.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef _IEEE_754_2008_SNAN
# define _IEEE_754_2008_SNAN 1
#endif

extern int inacc;

int merror;
double mretval = 64;
int traperror = 1;
char *mname;

void
translate_to (FILE *file,
	    double r)
{
  __ieee_double_shape_type bits;
  bits.value = r;
  fprintf(file, "0x%08lx, 0x%08lx", (unsigned long) bits.parts.msw, (unsigned long) bits.parts.lsw);
}

double
thedouble (uint32_t msw,
           uint32_t lsw, volatile double *r);

/* Convert double to float, preserving issignaling status and NaN sign */

#define to_float(f,d)	do {					\
		if (__issignaling(d)) {				\
			if (signbit(d))				\
				(f) = -__builtin_nansf("");	\
			else					\
				(f) =  __builtin_nansf("");	\
		} else if (isnan(d)) {				\
			if (signbit(d))				\
				(f) = -__builtin_nanf("");	\
			else					\
				(f) =  __builtin_nanf("");	\
		} else						\
			(f) = (float) (d);			\
	} while(0)

#define to_double(d,f)	do {					\
		if (__issignalingf(f)) {				\
			if (signbit(f))				\
				(d) = -__builtin_nans("");	\
			else					\
				(d) =  __builtin_nans("");	\
		} else if (isnanf(f)) {				\
			if (signbit(f))				\
				(d) = -__builtin_nan("");	\
			else					\
				(d) =  __builtin_nan("");	\
		} else						\
			(d) = (double) (f);			\
	} while(0)

int
ffcheck_id(double is,
	   one_line_type *p,
	   char *name,
	   int serrno,
	   int merror,
	   int id)
{
  /* Make sure the answer isn't to far wrong from the correct value */
  __ieee_double_shape_type correct = {}, isbits;
  int mag;
  isbits.value = is;

  (void) serrno;
  (void) merror;

  thedouble(p->qs[id].msw, p->qs[id].lsw, &correct.value);

  int error_bit = p->error_bit;

  mag = mag_of_error(correct.value, is);

  if (error_bit > 63)
    error_bit = 63;

  /* All signaling nans are the "same", as are all quiet nans.
   * On i386, just returning a value converts signaling nans to quiet
   * nans, let that pass
   */
  if (isnan(correct.value) && isnan(is)
#if !defined(__i386__) && !defined(__HAVE_68881__)
      && (issignaling(correct.value) == issignaling(is))
#endif
    )
  {
    mag = 64;
  }

  if (mag < error_bit)
  {
    inacc ++;

    printf("%s:%d, inaccurate answer: bit %d (%08lx%08lx %08lx%08lx) (%g %g)\n",
	   name,  p->line, mag,
	   (unsigned long) correct.parts.msw,
	   (unsigned long) correct.parts.lsw,
	   (unsigned long) isbits.parts.msw,
	   (unsigned long) isbits.parts.lsw,
	   correct.value, is);
  }

#if 0
  if (p->qs[id].merror != merror)
  {
    /* Beware, matherr doesn't exist anymore.  */
    printf("testing %s_vec.c:%d, matherr wrong: %d %d\n",
	   name, p->line, merror, p->qs[id].merror);
  }

  if (p->qs[id].errno_val != errno)
  {
    printf("testing %s_vec.c:%d, errno wrong: %d %d\n",
	   name, p->line, errno, p->qs[id].errno_val);

  }
#endif
  return mag;
}

int
ffcheck(double is,
	one_line_type *p,
	char *name,
	int serrno,
	int merror)
{
  return ffcheck_id(is, p, name, serrno, merror, 0);
}

int
fffcheck_id (float is,
	     one_line_type *p,
	     char *name,
	     int serrno,
	     int merror,
	     int id)
{
  /* Make sure the answer isn't to far wrong from the correct value */
  __ieee_float_shape_type correct, isbits;
  __ieee_double_shape_type correct_double;
  __ieee_double_shape_type is_double;
  int mag;

  (void) serrno;
  (void) merror;

  isbits.value = is;
  to_double(is_double.value, is);

  thedouble(p->qs[id].msw,p->qs[id].lsw, &correct_double.value);

  to_float(correct.value, correct_double.value);
//  printf("%s got 0x%08x want 0x%08x\n", name, isbits.p1, correct.p1);

  int error_bit = p->error_bit;

  if (error_bit > 31)
    error_bit = 31;

  mag = fmag_of_error(correct.value, is);

  /* All signaling nans are the "same", as are all quiet nans */
  if (isnan(correct.value) && isnan(is)
#if !defined(__i386__) && !defined(__m68k__)
      /* i386 calling convention ends up always converting snan into qnan */
      && (__issignalingf(correct.value) == __issignalingf(is))
#endif
	  )
  {
	mag = 32;
  }

  if (mag < error_bit)
  {
    inacc ++;

    printf("%s:%d, inaccurate answer: bit %d (%08lx is %08lx) (%g is %g) (is double 0x%08lx, 0x%08lx)\n",
	   name,  p->line, mag,
	   (unsigned long) (uint32_t) correct.p1,
	   (unsigned long) (uint32_t) isbits.p1,
	   (double) correct.value,
	   (double) is,
	   (unsigned long) (uint32_t) is_double.parts.msw,
	   (unsigned long) (uint32_t) is_double.parts.lsw);
  }

#if 0
  if (p->qs[id].merror != merror)
  {
    /* Beware, matherr doesn't exist anymore.  */
    printf("testing %s_vec.c:%d, matherr wrong: %d %d\n",
	   name, p->line, merror, p->qs[id].merror);
  }

  if (p->qs[id].errno_val != errno)
  {
    printf("testing %s_vec.c:%d, errno wrong: %d %d\n",
	   name, p->line, errno, p->qs[id].errno_val);

  }
#endif
  return mag;
}

int
fffcheck (float is,
	  one_line_type *p,
	  char *name,
	  int serrno,
	  int merror)
{
  return fffcheck_id(is, p, name, serrno, merror, 0);
}

static
volatile_memcpy(volatile void *dest, void *src, size_t len)
{
    volatile uint8_t *d = dest;
    uint8_t *s = src;
    while (len--)
        *d++ = *s++;
}

double
thedouble (uint32_t msw,
  uint32_t lsw, volatile double *r)
{
  __ieee_double_shape_type x;

  x.parts.msw = msw;
  x.parts.lsw = lsw;
#if !_IEEE_754_2008_SNAN
  if (isnan(x.value))
      x.parts.msw ^= 0x000c0000;
#endif
  if (r)
    volatile_memcpy(r, &x.value, sizeof(double));
  return x.value;
}

int calc;
extern int reduce;


void
frontline (FILE *f,
       int mag,
       one_line_type *p,
       double result,
       int merror,
       int my_errno,
       char *args,
       char *name)
{
  (void) name;
  /* float returns can never have more than 32 bits of accuracy */
  if (*args == 'f' && mag > 32)
    mag = 32;

  if (reduce && p->error_bit < mag)
  {
    fprintf(f, "{%2d,", p->error_bit);
  }
  else
  {
    fprintf(f, "{%2d,",mag);
  }


  fprintf(f,"%2d,%3d,", merror,my_errno);
  fprintf(f, "__LINE__, ");

  if (calc)
  {
    translate_to(f, result);
  }
  else
  {
    translate_to(f, thedouble(p->qs[0].msw, p->qs[0].lsw, NULL));
  }

  fprintf(f, ", ");

  fprintf(f,"0x%08lx, 0x%08lx", (unsigned long) p->qs[1].msw, (unsigned long) p->qs[1].lsw);


  if (args[2])
  {
    fprintf(f, ", ");
    fprintf(f,"0x%08lx, 0x%08lx", (unsigned long) p->qs[2].msw, (unsigned long) p->qs[2].lsw);
  }

  fprintf(f,"},	/* %g=f(%g",result,
  	  thedouble(p->qs[1].msw, p->qs[1].lsw, NULL));

  if (args[2])
  {
    fprintf(f,", %g", thedouble(p->qs[2].msw,p->qs[2].lsw, NULL));
  }
  fprintf(f, ")*/\n");
}

void
finish (FILE *f,
       int vector,
       double result,
       one_line_type *p,
       char *args,
       char *name)
{
  int mag;

  mag = ffcheck(result, p,name,  merror, errno);
  (void) f;
  (void) vector;
  (void) args;
  (void) mag;
#ifdef INCLUDE_GENERATE
  if (vector)
  {
    frontline(f, mag, p, result, merror, errno, args , name);
  }
#endif
}

void
finish2 (FILE *f,
	 int vector,
	 double result,
	 double result2,
	 one_line_type *p,
	 char *args,
	 char *name)
{
  int mag, mag2;

  mag = ffcheck(result, p,name,  merror, errno);
  mag2 = ffcheck_id(result2, p,name,  merror, errno, 2);
  if (mag2 < mag)
    mag = mag2;
  (void) f;
  (void) vector;
  (void) mag;
  (void) args;
#ifdef INCLUDE_GENERATE
  if (vector)
  {
    __ieee_double_shape_type result2_double;
    result2_double.value = result2;
    p->qs[2].msw = result2_double.parts.msw;
    p->qs[2].lsw = result2_double.parts.lsw;
    frontline(f, mag, p, result, merror, errno, args , name);
  }
#endif
}

void
ffinish (FILE *f,
       int vector,
       float fresult,
       one_line_type *p,
       char *args,
       char *name)
{
  int mag;

  mag = fffcheck(fresult, p,name,  merror, errno);
  (void) f;
  (void) vector;
  (void) args;
  (void) mag;
#ifdef INCLUDE_GENERATE
  if (vector)
  {
    frontline(f, mag, p, (double) fresult, merror, errno, args , name);
  }
#endif
}

void
ffinish2 (FILE *f,
	  int vector,
	  float fresult,
	  float fresult2,
	  one_line_type *p,
	  char *args,
	  char *name)
{
  int mag, mag2;

  mag = fffcheck(fresult, p,name,  merror, errno);
  mag2 = fffcheck_id(fresult2, p, name, merror, errno, 2);
  if (mag2 < mag)
    mag = mag2;
  (void) f;
  (void) vector;
  (void) args;
  (void) mag;
#ifdef INCLUDE_GENERATE
  if (vector)
  {
    __ieee_double_shape_type result2_double;
    to_double(result2_double.value, (double) fresult2);
    p->qs[2].msw = result2_double.parts.msw;
    p->qs[2].lsw = result2_double.parts.lsw;
    frontline(f, mag, p, (double) fresult, merror, errno, args , name);
  }
#endif
}

extern int redo;

bool
in_float_range(double arg)
{
	if (isinf(arg) || isnan(arg))
		return true;
	return -(double) FLT_MAX <= arg && arg <= (double) FLT_MAX;
}

void
run_vector_1 (int vector,
       one_line_type *p,
       char *func,
       char *name,
       char *args)
{
  FILE *f = NULL;
  double result;
  float fresult;

#ifdef INCLUDE_GENERATE
  if (vector)
  {

    VECOPEN(name, f);

    if (redo)
    {
      double k;
      union {
	struct { uint32_t a, b; };
	double d;
      } d, d4;

      for (k = -.2; k < .2; k+= 0.00132)
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{1,1, 1,1, 0,0,0x%08lx,0x%08lx, 0x%08lx, 0x%08lx},\n",
		(unsigned long) d.a, (unsigned long) d.b, (unsigned long) d4.a, (unsigned long) d4.b);

      }

      for (k = -1.2; k < 1.2; k+= 0.01)
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{1,1, 1,1, 0,0,0x%08lx,0x%08lx, 0x%08lx, 0x%08lx},\n",
		(unsigned long) d.a, (unsigned long) d.b, (unsigned long) d4.a, (unsigned long) d4.b);

      }
      for (k = -M_PI *2; k < M_PI *2; k+= M_PI/2)
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{1,1, 1,1, 0,0,0x%08lx,0x%08lx, 0x%08lx, 0x%08lx},\n",
		(unsigned long) d.a, (unsigned long) d.b, (unsigned long) d4.a, (unsigned long) d4.b);

      }

      for (k = -30; k < 30; k+= 1.7)
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{2,2, 1,1, 0,0, 0x%08lx,0x%08lx, 0x%08lx, 0x%08lx},\n",
		(unsigned long) d.a, (unsigned long) d.b, (unsigned long) d4.a, (unsigned long) d4.b);

      }
      VECCLOSE(f, name, args);
      return;
    }
  }
#endif

  newfunc(name);
  while (p->line)
  {
    volatile double arg1;
    thedouble(p->qs[1].msw, p->qs[1].lsw, &arg1);
    volatile double arg2;
    thedouble(p->qs[2].msw, p->qs[2].lsw, &arg2);

    errno = 0;
    merror = 0;
    mname = 0;


    line(p->line);

    merror = 0;
    errno = 123;

    if (strcmp(args,"dd")==0)
    {
      typedef double (*pdblfunc) (double);

      /* Double function returning a double */

      result = ((pdblfunc)(func))(arg1);

      finish(f,vector, result, p, args, name);
    }
    else  if (strcmp(args,"ff")==0)
    {
      float arga;

      typedef float (*pdblfunc) (float);

      /* Double function returning a double */

      if (in_float_range(arg1))
      {
	to_float(arga, arg1);
	fresult = ((pdblfunc)(func))(arga);
	ffinish(f, vector, fresult, p,args, name);
      }
    }
    else if (strcmp(args,"ddd")==0)
     {
       typedef double (*pdblfunc) (double,double);

       result = ((pdblfunc)(func))(arg1,arg2);
       finish(f, vector, result, p,args, name);
     }
     else  if (strcmp(args,"fff")==0)
     {
       volatile float arga;
       volatile float argb;

       typedef float (*pdblfunc) (float,float);

       if (in_float_range(arg1) && in_float_range(arg2))
       {
	 to_float(arga, arg1);
	 to_float(argb, arg2);
	 fresult = ((pdblfunc)(func))(arga, argb);
//	 printf("0x%08x = %s(0x%08x, 0x%08x)\n",
//		float_bits(fresult), name, float_bits(arga), float_bits(argb));
	 ffinish(f, vector, fresult, p,args, name);
       }
     }
     else if (strcmp(args,"did")==0)
     {
       typedef double (*pdblfunc) (int,double);

       result = ((pdblfunc)(func))((int)arg1,arg2);
       finish(f, vector, result, p,args, name);
     }
     else  if (strcmp(args,"fif")==0)
     {
       float argb;

       typedef float (*pdblfunc) (int,float);


       if (in_float_range(arg2))
       {
	 to_float(argb, arg2);
	 fresult = ((pdblfunc)(func))((int)arg1, argb);
	 ffinish(f, vector, fresult, p,args, name);
       }
     }
     else if (strcmp(args,"id")==0)
     {
       typedef int (*pdblfunc)(double);

       result = (double) ((pdblfunc)(func))(arg1);
       finish(f, vector, result, p, args, name);
     }
     else if (strcmp(args,"ddp") == 0)
     {
       typedef double (*pdblfunc)(double, double *);
       double result2;

       result = (double) ((pdblfunc)(func))(arg1, &result2);
       finish2(f, vector, result, result2, p, args, name);
     }
     else if (strcmp(args,"ffp") == 0)
     {
       typedef float (*pdblfunc)(float, float *);
       float result2;

       fresult = (float) ((pdblfunc)(func))(arg1, &result2);
       ffinish2(f, vector, fresult, result2, p, args, name);
     }
     else if (strcmp(args,"ddi") == 0)
     {
       typedef double (*pdblfunc)(double, int);

       result = ((pdblfunc)func) (arg1, (int) arg2);
       finish(f, vector, result, p, args, name);
     }
     else if (strcmp(args,"ffi") == 0)
     {
       typedef float (*pdblfunc)(float, int);

       fresult = ((pdblfunc)func) (arg1, (int) arg2);
       ffinish(f, vector, fresult, p, args, name);
     }
    p++;
  }
  if (vector && f)
  {
    VECCLOSE(f, name, args);
  }
}

void
test_math (int vector)
{
  (void) vector;
#if TEST_PART == 0 || TEST_PART == -1
  test_acos(vector);
  test_acosf(vector);
  test_acosh(vector);
  test_acoshf(vector);
  test_asin(vector);
  test_asinf(vector);
  test_asinh(vector);
  test_asinhf(vector);
  test_atan(vector);
#endif
#if TEST_PART > 0 || TEST_PART == -1
  test_atan2(vector);
  test_atan2f(vector);
#endif
#if TEST_PART == 0 || TEST_PART == -1
  test_atanf(vector);
  test_atanh(vector);
  test_atanhf(vector);
  test_ceil(vector);
  test_ceilf(vector);
  test_copysign(vector);
  test_copysignf(vector);
  test_cos(vector);
  test_cosf(vector);
  test_cosh(vector);
  test_coshf(vector);
  test_erf(vector);
  test_erfc(vector);
  test_erfcf(vector);
  test_erff(vector);
  test_exp(vector);
  test_expf(vector);
#endif
#if TEST_PART == 1 || TEST_PART == -1
  test_fabs(vector);
  test_fabsf(vector);
  test_floor(vector);
  test_floorf(vector);
  test_fmod(vector);
  test_fmodf(vector);
  test_gamma(vector);
  test_gammaf(vector);
  test_hypot(vector);
  test_hypotf(vector);
  test_issignaling(vector);
  test_j0(vector);
  test_j0f(vector);
  test_j1(vector);
  test_j1f(vector);
  test_jn(vector);
  test_jnf(vector);
  test_log(vector);
  test_log10(vector);
  test_log10f(vector);
  test_log1p(vector);
  test_log1pf(vector);
  test_log2(vector);
  test_log2f(vector);
  test_logf(vector);
#endif
#if TEST_PART == 2 || TEST_PART == -1
  test_modf(vector);
  test_modff(vector);
  test_pow_vec(vector);
  test_powf_vec(vector);
  test_scalb(vector);
  test_scalbf(vector);
  test_scalbn(vector);
  test_scalbnf(vector);
  test_sin(vector);
  test_sinf(vector);
  test_sinh(vector);
  test_sinhf(vector);
  test_sqrt(vector);
  test_sqrtf(vector);
  test_tan(vector);
  test_tanf(vector);
  test_tanh(vector);
  test_tanhf(vector);
  test_trunc(vector);
  test_truncf(vector);
  test_y0(vector);
  test_y0f(vector);
  test_y1(vector);
  test_y1f(vector);
  test_y1f(vector);
  test_ynf(vector);
#endif
}

/* These have to be played with to get to compile on machines which
   don't have the fancy <foo>f entry points
*/

#if 0
float cosf (float a) { return cos((double)a); }
float sinf (float  a) { return sin((double)a); }
float log1pf (float a) { return log1p((double)a); }
float tanf (float a) { return tan((double)a); }
float ceilf (float a) { return ceil(a); }
float floorf (float a) { return floor(a); }
#endif

/*ndef HAVE_FLOAT*/
#if 0

float fmodf(a,b) float a,b; { return fmod(a,b); }
float hypotf(a,b) float a,b; { return hypot(a,b); }

float acosf(a) float a; { return acos(a); }
float acoshf(a) float a; { return acosh(a); }
float asinf(a) float a; { return asin(a); }
float asinhf(a) float a; { return asinh(a); }
float atanf(a) float a; { return atan(a); }
float atanhf(a) float a; { return atanh(a); }

float coshf(a) float a; { return cosh(a); }
float erff(a) float a; { return erf(a); }
float erfcf(a) float a; { return erfc(a); }
float expf(a) float a; { return exp(a); }
float fabsf(a) float a; { return fabs(a); }

float gammaf(a) float a; { return gamma(a); }
float j0f(a) float a; { return j0(a); }
float j1f(a) float a; { return j1(a); }
float log10f(a) float a; { return log10(a); }

float logf(a) float a; { return log(a); }

float sinhf(a) float a; { return sinh(a); }
float sqrtf(a) float a; { return sqrt(a); }

float tanhf(a) float a; { return tanh(a); }
float y0f(a) float a; { return y0(a); }
float y1f(a) float a; { return y1(a); }
#endif
