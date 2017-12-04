
/* @(#)z_log10.c 1.0 98/08/13 */
/******************************************************************
 * Logarithm
 *
 * Input:
 *   x - floating point value
 *
 * Output:
 *   logarithm of x
 *
 * Description:
 *   This routine returns the logarithm of x (base 10).
 *
 *****************************************************************/

/*
FUNCTION
        <<log10>>, <<log10f>>---base 10 logarithms

INDEX
log10
INDEX
log10f

SYNOPSIS
        #include <math.h>
        double log10(double <[x]>);
        float log10f(float <[x]>);

DESCRIPTION
<<log10>> returns the base 10 logarithm of <[x]>.
It is implemented as <<log(<[x]>) / log(10)>>.

<<log10f>> is identical, save that it takes and returns <<float>> values.

RETURNS
<<log10>> and <<log10f>> return the calculated value.

See the description of <<log>> for information on errors.

PORTABILITY
<<log10>> is ANSI C.  <<log10f>> is an extension.

*/


#include "fdlibm.h"
#include "zmath.h"

#ifndef _DOUBLE_IS_32BITS

double
log10 (double x)
{
  return (logarithm (x, 1));
}

#endif /* _DOUBLE_IS_32BITS */
