/*
Copyright (C) 1998, 2002 by Red Hat Inc. All rights reserved.

Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice
is preserved.
 */
#ifndef __F_MATH_H__
#define __F_MATH_H__

#include <sys/cdefs.h>
#include "fdlibm.h"

__inline__
static 
int 
check_finite (double x)
{  
  __int32_t hx;
  GET_HIGH_WORD(hx,x);
  return  (int)((__uint32_t)((hx&0x7fffffff)-0x7ff00000)>>31);
}

__inline__
static 
int 
check_finitef (float x)
{  
  __int32_t ix;
  GET_FLOAT_WORD(ix,x);
  return  (int)((__uint32_t)((ix&0x7fffffff)-0x7f800000)>>31);
}

float _f_expf (float x);
float _f_powf (float x, float y);
float _f_rintf (float x);
long int _f_lrintf (float x);
long long int _f_llrintf (float x);

double _f_exp (double x);
double _f_pow (double x, double y);
double _f_rint (double x);
long int _f_lrint (double x);
long int _f_lrintl (long double x);
long long int _f_llrint (double x);

long double _f_rintl (long double x);
long long int _f_llrintl (long double x);

#endif /* __F_MATH_H__ */
