#include <math.h>

typedef union
{
  long double value;
  struct
  {
    unsigned mantissa[2];
    unsigned sign_exponent : 16;
    unsigned empty : 16;
  } parts;
} ieee_long_double_shape_type;



/* Get int from the exponent of a long double.  */
static __inline__ void
get_ld_exp (unsigned exp,long double d)
{
  ieee_long_double_shape_type u;
  u.value = d;
  exp = u.parts.sign_exponent;
}

/* Set exponent of a long double from an int.  */
static __inline__ void
set_ld_exp (long double d,unsigned exp)
{
  ieee_long_double_shape_type u;
  u.value = d;
  u.parts.sign_exponent = exp;
  d = u.value;
}

long double
hypotl (long double x, long double y)
{
  unsigned exx = 0U;
  unsigned eyy = 0U;
  unsigned  scale;
  long double xx =fabsl(x);
  long double yy =fabsl(y);
  if (!isfinite(xx) || !isfinite(yy))
    return  xx + yy; /* Return INF or NAN. */

  /* Scale to avoid overflow.*/
  get_ld_exp (exx, xx);
  get_ld_exp (eyy, yy);	
  scale = (exx > eyy ? exx : eyy);
  if (scale == 0)
    return 0.0L;		
  set_ld_exp (xx, exx - scale);
  set_ld_exp (yy, eyy - scale);	
  xx = sqrtl(xx * xx  + yy * yy);
  get_ld_exp (exx,xx);
  set_ld_exp (xx, exx + scale);
  return xx;
}
