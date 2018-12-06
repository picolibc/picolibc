
/* @(#)z_atan2.c 1.0 98/08/13 */

/*
FUNCTION
        <<atan2>>, <<atan2f>>---arc tangent of y/x

INDEX
   atan2
INDEX
   atan2f

SYNOPSIS
        #include <math.h>
        double atan2(double <[y]>,double <[x]>);
        float atan2f(float <[y]>,float <[x]>);

DESCRIPTION

<<atan2>> computes the inverse tangent (arc tangent) of <[y]>/<[x]>.
<<atan2>> produces the correct result even for angles near
@ifnottex
pi/2 or -pi/2
@end ifnottex
@tex
$\pi/2$ or $-\pi/2$
@end tex
(that is, when <[x]> is near 0).

<<atan2f>> is identical to <<atan2>>, save that it takes and returns
<<float>>.

RETURNS
<<atan2>> and <<atan2f>> return a value in radians, in the range of
@ifnottex
-pi to pi.
@end ifnottex
@tex
$-\pi$ to $\pi$.
@end tex

If both <[x]> and <[y]> are 0.0, <<atan2>> causes a <<DOMAIN>> error.

PORTABILITY
<<atan2>> is ANSI C.  <<atan2f>> is an extension.


*/

/******************************************************************
 * Arctangent2
 *
 * Input:
 *   v, u - floating point values
 *
 * Output:
 *   arctan2 of v / u 
 *
 * Description:
 *   This routine returns the arctan2 of v / u.
 *
 *****************************************************************/

#include "fdlibm.h"
#include "zmath.h"

#ifndef _DOUBLE_IS_32BITS

double
atan2 (double v,
        double u)
{
  return (atangent (0.0, v, u, 1));
}

#endif /* _DOUBLE_IS_32BITS */
