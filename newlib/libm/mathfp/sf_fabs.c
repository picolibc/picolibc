
/* @(#)z_fabsf.c 1.0 98/08/13 */
/******************************************************************
 * Floating-Point Absolute Value
 *
 * Input:
 *   x - floating-point number
 *
 * Output:
 *   absolute value of x
 *
 * Description:
 *   fabs computes the absolute value of a floating point number.
 *
 *****************************************************************/

#include "fdlibm.h"
#include "zmath.h"

float
fabsf (float x)
{
  switch (numtestf (x))
    {
      case NUMTEST_NAN:
        errno = EDOM;
        return (x);
      case NUMTEST_INF:
        errno = ERANGE;
        return (x);
      case NUMTEST_0:
        return (0.0);
      default:
        return (x < 0.0 ? -x : x);
    }
}

#ifdef _DOUBLE_IS_32BITS
double fabs (double x)
{
  return (double) fabsf ((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
