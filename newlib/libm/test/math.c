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
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int inacc;

int merror;
double mretval = 64;
int traperror = 1;
char *mname;

int verbose;

void translate_to (FILE *file,
	    double r)
{
  __ieee_double_shape_type bits;
  bits.value = r;
  fprintf(file, "0x%08x, 0x%08x", bits.parts.msw, bits.parts.lsw);
}

int 
ffcheck (double is,
       one_line_type *p,
       char *name,
       int serrno,
	 int merror,
	 int is_float)
{
  /* Make sure the answer isn't to far wrong from the correct value */
  __ieee_double_shape_type correct, isbits;
  int mag;  
  isbits.value = is;
  
  correct.parts.msw = p->qs[0].msw;
  correct.parts.lsw = p->qs[0].lsw;
  
  int error_bit = p->error_bit;

  if (is_float) {
    if (error_bit > 31)
      error_bit = 31;
  } else  {
    if (error_bit > 63)
      error_bit = 63;
  }

  mag = mag_of_error(correct.value, is);
  
  if (isnan(correct.value) && isnan(is))
    mag = 64;

  if (mag < error_bit)
  {
    inacc ++;
    
    printf("%s:%d, inaccurate answer: bit %d (%08x%08x %08x%08x) (%g %g)\n",
	   name,  p->line, mag,
	   correct.parts.msw,
	   correct.parts.lsw,
	   isbits.parts.msw,
	   isbits.parts.lsw,
	   correct.value, is);
  }      
  
#if 0
  if (p->qs[0].merror != merror) 
  {
    /* Beware, matherr doesn't exist anymore.  */
    printf("testing %s_vec.c:%d, matherr wrong: %d %d\n",
	   name, p->line, merror, p->qs[0].merror);
  }

  if (p->qs[0].errno_val != errno) 
  {
    printf("testing %s_vec.c:%d, errno wrong: %d %d\n",
	   name, p->line, errno, p->qs[0].errno_val);
    
  }
#endif
  return mag;
}

double
thedouble (uint32_t msw,
	   uint32_t lsw)
{
  __ieee_double_shape_type x;
  
  x.parts.msw = msw;
  x.parts.lsw = lsw;
  return x.value;
}

int calc;
int reduce;


frontline (FILE *f,
       int mag,
       one_line_type *p,
       double result,
       int merror,
       int my_errno,
       char *args,
       char *name)
{
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
    translate_to(f, thedouble(p->qs[0].msw, p->qs[0].lsw));
  }
  
  fprintf(f, ", ");      

  fprintf(f,"0x%08x, 0x%08x", p->qs[1].msw, p->qs[1].lsw);
  

  if (args[2]) 
  {
    fprintf(f, ", ");      
    fprintf(f,"0x%08x, 0x%08x", p->qs[2].msw, p->qs[2].lsw);
  }
	
  fprintf(f,"},	/* %g=f(%g",result,
  	  thedouble(p->qs[1].msw, p->qs[1].lsw));

  if (args[2])
  {
    fprintf(f,", %g", thedouble(p->qs[2].msw,p->qs[2].lsw));
  }
  fprintf(f, ")*/\n");      
}

finish (FILE *f,
       int vector,
       double result,
       one_line_type *p,
       char *args,
       char *name)
{
  int mag;

  mag = ffcheck(result, p,name,  merror, errno, args[0] == 'f');    
  if (vector) 
  {    
    frontline(f, mag, p, result, merror, errno, args , name);
  }
} 
int redo;  

void
run_vector_1 (int vector,
       one_line_type *p,
       char *func,
       char *name,
       char *args)
{
  FILE *f;
  int mag;
  double result;  
  
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
	fprintf(f,"{1,1, 1,1, 0,0,0x%08x,0x%08x, 0x%08x, 0x%08x},\n",
		d.a, d.b, d4.a, d4.b);

      }

      for (k = -1.2; k < 1.2; k+= 0.01) 
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{1,1, 1,1, 0,0,0x%08x,0x%08x, 0x%08x, 0x%08x},\n",
		d.a, d.b, d4.a, d4.b);

      }
      for (k = -M_PI *2; k < M_PI *2; k+= M_PI/2) 
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{1,1, 1,1, 0,0,0x%08x,0x%08x, 0x%08x, 0x%08x},\n",
		d.a, d.b, d4.a, d4.b);

      }

      for (k = -30; k < 30; k+= 1.7) 
      {
	d.d = k;
	d4.d = k + 4;
	fprintf(f,"{2,2, 1,1, 0,0, 0x%08x,0x%08x, 0x%08x, 0x%08x},\n",
		d.a, d.b, d4.a, d4.b);

      }
      VECCLOSE(f, name, args);
      return;
    }
  }
 
  newfunc(name);
  while (p->line) 
  {
    double arg1 = thedouble(p->qs[1].msw, p->qs[1].lsw);
    double arg2 = thedouble(p->qs[2].msw, p->qs[2].lsw);

    double r;
    double rf;
    
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
      double a;
      
      typedef float (*pdblfunc) (float);
      
      /* Double function returning a double */
      
      if (arg1 < FLT_MAX )
      {
	arga = arg1;      
	result = ((pdblfunc)(func))(arga);
	finish(f, vector, result, p,args, name);       
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
       double a,b;
       
       float arga;
       float argb;
      
       typedef float (*pdblfunc) (float,float);
      

       if (arg1 < FLT_MAX && arg2 < FLT_MAX) 
       {
	 arga = arg1;      
	 argb = arg2;
	 result = ((pdblfunc)(func))(arga, argb);
	 finish(f, vector, result, p,args, name);       
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
       double a,b;
       
       float arga;
       float argb;
      
       typedef float (*pdblfunc) (int,float);
      

       if (arg1 < FLT_MAX && arg2 < FLT_MAX) 
       {
	 arga = arg1;      
	 argb = arg2;
	 result = ((pdblfunc)(func))((int)arga, argb);
	 finish(f, vector, result, p,args, name);       
       }
     }      

    p++;
  }
  if (vector)
  {
    VECCLOSE(f, name, args);
  }
}

void
test_math (int vector)
{
  test_acos(vector);
  test_acosf(vector);
  test_acosh(vector);
  test_acoshf(vector);
  test_asin(vector);
  test_asinf(vector);
  test_asinh(vector);
  test_asinhf(vector);
  test_atan(vector);
  test_atan2(vector);
  test_atan2f(vector);
  test_atanf(vector);
  test_atanh(vector);
  test_atanhf(vector);
  test_ceil(vector);
  test_ceilf(vector);
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
  test_y0(vector);
  test_y0f(vector);
  test_y1(vector);
  test_y1f(vector);
  test_y1f(vector);
  test_ynf(vector);
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
