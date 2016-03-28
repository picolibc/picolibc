#ifndef _LOCAL_MATH_H_
#define _LOCAL_MATH_H_

/* This header is required to define the types used by some of the
   mingw-w64 based files in the math subdir. */

typedef union __mingw_dbl_type_t {
  double x;
  unsigned long long val;
  struct {
    unsigned int low, high;
  } lh;
} __mingw_dbl_type_t;

typedef union __mingw_flt_type_t {
  float x;
  unsigned int val;
} __mingw_flt_type_t;

typedef union __mingw_ldbl_type_t
{
  long double x;
  struct {
    unsigned int low, high;
    int sign_exponent : 16;
    int res1 : 16;
    int res0 : 32;
  } lh;
} __mingw_ldbl_type_t;

typedef union __mingw_fp_types_t
{
  long double *ld;
  double *d;
  float *f;
  __mingw_ldbl_type_t *ldt;
  __mingw_dbl_type_t *dt;
  __mingw_flt_type_t *ft;
} __mingw_fp_types_t;

#include_next <math.h>

#endif /* _LOCAL_MATH_H_ */
