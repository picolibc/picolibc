
/* @(#)z_fabs.c 1.0 98/08/13 */

/*
FUNCTION
       <<fabs>>, <<fabsf>>---absolute value (magnitude)
INDEX
        fabs
INDEX
        fabsf

SYNOPSIS
        #include <math.h>
       double fabs(double <[x]>);
       float fabsf(float <[x]>);

DESCRIPTION
<<fabs>> and <<fabsf>> calculate
@tex
$|x|$,
@end tex
the absolute value (magnitude) of the argument <[x]>, by direct
manipulation of the bit representation of <[x]>.

RETURNS
The calculated value is returned.

PORTABILITY
<<fabs>> is ANSI.
<<fabsf>> is an extension.

*/

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

#ifndef _DOUBLE_IS_32BITS

double
fabs (double x)
{
  switch (numtest (x))
    {
      case NAN:
        errno = EDOM;
        return (x);
      case INF:
        errno = ERANGE;
        return (x);
      case 0:
        return (0.0);
      default:
        return (x < 0.0 ? -x : x);
    }
}

#endif /* _DOUBLE_IS_32BITS */
