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
#define _GNU_SOURCE
#define HAVE_FLOAT 1
#define X(x) (char *)x

#include <math.h>
#include <float.h>
//#include <ieeefp.h>
#include <stdio.h>
#include <stdint.h>

#ifdef INCLUDE_GENERATE
#define TEST_CONST
#else
#define TEST_CONST const
#endif

/* FIXME FIXME FIXME:
   Neither of __ieee_{float,double}_shape_type seem to be used anywhere
   except in libm/test.  If that is the case, please delete these from here.
   If that is not the case, please insert documentation here describing why
   they're needed.  */

#ifdef __IEEE_BIG_ENDIAN

typedef union 
{
  double value;
  struct 
  {
    unsigned int sign : 1;
    unsigned int exponent: 11;
    unsigned int fraction0:4;
    unsigned int fraction1:16;
    unsigned int fraction2:16;
    unsigned int fraction3:16;
    
  } number;
  struct 
  {
    unsigned int sign : 1;
    unsigned int exponent: 11;
    unsigned int quiet:1;
    unsigned int function0:3;
    unsigned int function1:16;
    unsigned int function2:16;
    unsigned int function3:16;
  } nan;
  struct 
  {
    uint32_t msw;
    uint32_t lsw;
  } parts;
    int32_t aslong[2];
} __ieee_double_shape_type;

#else

typedef union 
{
  double value;
  struct 
  {
#ifdef __SMALL_BITFIELDS
    uint32_t fraction3:16;
    uint32_t fraction2:16;
    uint32_t fraction1:16;
    uint32_t fraction0: 4;
#else
    uint32_t fraction1:32;
    uint32_t fraction0:20;
#endif
    uint32_t exponent :11;
    uint32_t sign     : 1;
  } number;
  struct 
  {
#ifdef __SMALL_BITFIELDS
    uint32_t function3:16;
    uint32_t function2:16;
    uint32_t function1:16;
    uint32_t function0:3;
#else
    uint32_t function1:32;
    uint32_t function0:19;
#endif
    uint32_t quiet:1;
    uint32_t exponent: 11;
    uint32_t sign : 1;
  } nan;
  struct 
  {
    uint32_t lsw;
    uint32_t msw;
  } parts;

  int32_t aslong[2];

} __ieee_double_shape_type;

#endif /* __IEEE_LITTLE_ENDIAN */

#ifdef __IEEE_BIG_ENDIAN

typedef union
{
  float value;
  struct 
  {
    uint32_t sign : 1;
    uint32_t exponent: 8;
    uint32_t fraction0: 7;
    uint32_t fraction1: 16;
  } number;
  struct 
  {
    uint32_t sign:1;
    uint32_t exponent:8;
    uint32_t quiet:1;
    uint32_t function0:6;
    uint32_t function1:16;
  } nan;
  uint32_t p1;
  
} __ieee_float_shape_type;

#else

typedef union
{
  float value;
  struct 
  {
    uint32_t fraction0: 7;
    uint32_t fraction1: 16;
    uint32_t exponent: 8;
    uint32_t sign : 1;
  } number;
  struct 
  {
    uint32_t function1:16;
    uint32_t function0:6;
    uint32_t quiet:1;
    uint32_t exponent:8;
    uint32_t sign:1;
  } nan;
  int32_t p1;
  
} __ieee_float_shape_type;

#endif /* __IEEE_LITTLE_ENDIAN */

extern int inacc;
extern int redo;
extern int reduce;
extern int verbose;

void checkf();
void enter();


double translate_from();

typedef struct 
{
  uint32_t msw, lsw;
} question_struct_type;


typedef TEST_CONST struct 
{
  char error_bit;
  char errno_val;
  char merror;
  int line;
  
  question_struct_type qs[3];
} one_line_type;


#define MVEC_START(x) one_line_type x[] =  {
#define MVEC_END    0,};


int mag_of_error (double, double);
int fmag_of_error (float, float);


#define ERROR_PERFECT 20
#define ERROR_FAIL -1

#define AAAA 15
#define AAA 10
#define AA  6
#define A   5
#define B   3
#define C   1 
#define VECOPEN(x,f) \
{\
  char buffer[100];\
   sprintf(buffer,"%s_vec.c",x);\
    f = fopen(buffer,"w");\
     fprintf(f,"#include \"test.h\"\n");\
      fprintf(f," one_line_type %s_vec[] = {\n", x);\
}

#define VECCLOSE(f,name,args)\
{\
  fprintf(f,"0,};\n");      \
   fprintf(f,"test_%s(int m)   {run_vector_1(m,%s_vec,(char *)(%s),\"%s\",\"%s\");   }	\n",\
	   name,\
	   name,name,name,args);\
	    fclose(f);\
}



typedef TEST_CONST struct 
{
  int line;
  
  char *string;
  double value;
  int endscan;
} double_type;

#define ENDSCAN_MASK	0x7f
#define ENDSCAN_IS_ZERO	0x80
#define ENDSCAN_IS_INF	0x80

typedef TEST_CONST struct {
  long int value;
  char end;
  char errno_val;
} int_scan_type;

typedef TEST_CONST struct 
{
  int line;
  int_scan_type octal;
  int_scan_type decimal;
  int_scan_type hex;
  int_scan_type normal;
  int_scan_type alphabetical;
  char *string;
} int_type;


typedef TEST_CONST struct 
{
  int line;
  double value;
  char *estring;
  int e1;
  int e2;
  int e3;
  char *fstring;
  int f1;
  int f2;
  int f3;
  char *gstring;
  int g1;
  char *gfstring;
} ddouble_type;

typedef TEST_CONST struct
{
  int line;
  double value;
  char *result;
  char *format_string;
  int mag;
} sprint_double_type;


typedef TEST_CONST struct
{
  int line;
  int32_t value;
  char *result;
  char *format_string;
} sprint_int_type;


void test_ieee (void);
void test_math2 (void);
void test_math (int vector);
void test_string (void);
void test_is (void);
void test_cvt (void);

void line (int);

void test_mok (double, double, int);
void test_mfok (float, float, int);
void test_iok (int, int);
void test_eok (int, int);
void test_sok (char *, char*);
void test_scok (char *, char*, int);
void test_scok2 (char *, char*, char*, int);
void newfunc (const char *);

void
run_vector_1 (int vector,
	      one_line_type *p,
	      char *func,
	      char *name,
	      char *args);
void
test_acos(int vector);

void
test_acosf(int vector);

void
test_acosh(int vector);

void
test_acoshf(int vector);

void
test_asin(int vector);

void
test_asinf(int vector);

void
test_asinh(int vector);

void
test_asinhf(int vector);

void
test_atan(int vector);

void
test_atan2(int vector);

void
test_atan2f(int vector);

void
test_atanf(int vector);

void
test_atanh(int vector);

void
test_atanhf(int vector);

void
test_ceil(int vector);

void
test_ceilf(int vector);

void
test_copysign(int vector);

void
test_copysignf(int vector);

void
test_cos(int vector);

void
test_cosf(int vector);

void
test_cosh(int vector);

void
test_coshf(int vector);

void
test_erf(int vector);

void
test_erfc(int vector);

void
test_erfcf(int vector);

void
test_erff(int vector);

void
test_exp(int vector);

void
test_expf(int vector);

void
test_fabs(int vector);

void
test_fabsf(int vector);

void
test_floor(int vector);

void
test_floorf(int vector);

void
test_fmod(int vector);

void
test_fmodf(int vector);

void
test_gamma(int vector);

void
test_gammaf(int vector);

void
test_hypot(int vector);

void
test_hypotf(int vector);

void
test_issignaling(int vector);

void
test_j0(int vector);

void
test_j0f(int vector);

void
test_j1(int vector);

void
test_j1f(int vector);

void
test_jn(int vector);

void
test_jnf(int vector);

void
test_log(int vector);

void
test_log10(int vector);

void
test_log10f(int vector);

void
test_log1p(int vector);

void
test_log1pf(int vector);

void
test_log2(int vector);

void
test_log2f(int vector);

void
test_logf(int vector);

void
test_modf(int vector);

void
test_modff(int vector);

void
test_pow_vec(int vector);

void
test_powf_vec(int vector);

void
test_scalb(int vector);

void
test_scalbf(int vector);

void
test_scalbn(int vector);

void
test_scalbnf(int vector);

void
test_sin(int vector);

void
test_sinf(int vector);

void
test_sinh(int vector);

void
test_sinhf(int vector);

void
test_sqrt(int vector);

void
test_sqrtf(int vector);

void
test_tan(int vector);

void
test_tanf(int vector);

void
test_tanh(int vector);

void
test_tanhf(int vector);

void
test_trunc(int vector);

void
test_truncf(int vector);

void
test_y0(int vector);

void
test_y0f(int vector);

void
test_y1(int vector);

void
test_y1f(int vector);

void
test_y1f(int vector);

void
test_ynf(int vector);

