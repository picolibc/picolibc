/* Floating point consts needed by STL class mumeric_limits<float>
 and  numeric_limits<double>.  Also used as return values by nan, inf */
  
#include <math.h>
/*
According to IEEE 754 a QNaN has exponent bits of all 1 values and
initial significand bit of 1.  A SNaN has has an exponent of all 1
values and initial significand bit of 0 (with one or more other
significand bits of 1). An Inf has significand of 0 and
exponent of all 1 values. A denormal value has all exponent bits of 0.

The following does _not_ follow those rules, but uses values
equal to those exported from MS C++ runtime lib, msvcprt.dll
for float and double. MSVC however, does not have long doubles.
*/


#define __DOUBLE_INF_REP { 0, 0, 0, 0x7ff0 }
#define __DOUBLE_QNAN_REP { 0, 0, 0, 0xfff8 }  /* { 0, 0, 0, 0x7ff8 }  */
#define __DOUBLE_SNAN_REP { 0, 0, 0, 0xfff0 }  /* { 1, 0, 0, 0x7ff0 }  */
#define __DOUBLE_DENORM_REP {1, 0, 0, 0}
#define D_NAN_MASK 0x7ff0000000000000LL /* this will mask NaN's and Inf's */

#define __FLOAT_INF_REP { 0, 0x7f80 }
#define __FLOAT_QNAN_REP { 0, 0xffc0 }  /* { 0, 0x7fc0 }  */
#define __FLOAT_SNAN_REP { 0, 0xff80 }  /* { 1, 0x7f80 }  */
#define __FLOAT_DENORM_REP {1,0}
#define F_NAN_MASK 0x7f800000

/* This assumes no implicit (hidden) bit in extended mode */
#define __LONG_DOUBLE_INF_REP { 0, 0, 0, 0x8000, 0x7fff }
#define __LONG_DOUBLE_QNAN_REP { 0, 0, 0, 0xc000, 0xffff } 
#define __LONG_DOUBLE_SNAN_REP { 0, 0, 0, 0x8000, 0xffff }
#define __LONG_DOUBLE_DENORM_REP {1, 0, 0, 0, 0}

union _ieee_rep
{
  unsigned short rep[5];
  float float_val;
  double double_val;
  long double ldouble_val;
} ;

const union _ieee_rep __QNAN = { __DOUBLE_QNAN_REP };
/*
const union _ieee_rep __SNAN = { __DOUBLE_SNAN_REP };
const union _ieee_rep __INF = { __DOUBLE_INF_REP };
const union _ieee_rep __DENORM = { __DOUBLE_DENORM_REP };
*/
/* ISO C99 */

#undef nan
/* FIXME */
double nan (const char * tagp __attribute__((unused)) )
	{ return __QNAN.double_val; } 


const union _ieee_rep __QNANF = { __FLOAT_QNAN_REP };   
/*
const union _ieee_rep __SNANF = { __FLOAT_SNAN_REP };   
const union _ieee_rep __INFF = { __FLOAT_INF_REP };   
const union _ieee_rep __DENORMF = { __FLOAT_DENORM_REP };   
*/

#undef nanf
/* FIXME */
float nanf(const char * tagp __attribute__((unused)) )
  { return __QNANF.float_val;}


const union _ieee_rep __QNANL = { __LONG_DOUBLE_QNAN_REP };
/*
const union _ieee_rep __SNANL = { __LONG_DOUBLE_SNAN_REP };
const union _ieee_rep __INFL = { __LONG_DOUBLE_INF_REP };
const union _ieee_rep __DENORML = { __LONG_DOUBLE_DENORM_REP };
*/

#undef nanl
/* FIXME */
long double nanl (const char * tagp __attribute__((unused)) )
  { return __QNANL.ldouble_val;} 
