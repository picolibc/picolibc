
/* @(#)z_isnan.c 1.0 98/08/13 */

/*
FUNCTION
        <<isnan>>,<<isnanf>>,<<isinf>>,<<isinff>>,<<finite>>,<<finitef>>---test
for exceptional numbers

INDEX
        isnan
INDEX
        isinf
INDEX
        finite

INDEX
        isnanf
INDEX
        isinff
INDEX
        finitef

ANSI_SYNOPSIS
        #include <ieeefp.h>
        int isnan(double <[arg]>);
        int isinf(double <[arg]>);
        int finite(double <[arg]>);
        int isnanf(float <[arg]>);
        int isinff(float <[arg]>);
        int finitef(float <[arg]>);

TRAD_SYNOPSIS
        #include <ieeefp.h>
        int isnan(<[arg]>)
        double <[arg]>;
        int isinf(<[arg]>)
        double <[arg]>;
        int finite(<[arg]>);
        double <[arg]>;
        int isnanf(<[arg]>);
        float <[arg]>;
        int isinff(<[arg]>);
        float <[arg]>;
        int finitef(<[arg]>);
        float <[arg]>;


DESCRIPTION
        These functions provide information on the floating-point
        argument supplied.

        There are five major number formats -
        o+
        o zero
         a number which contains all zero bits.
        o subnormal
         Is used to represent  number with a zero exponent, but a nonzero fract
ion.
         o normal
          A number with an exponent, and a fraction
        o infinity
          A number with an all 1's exponent and a zero fraction.
        o NAN
          A number with an all 1's exponent and a nonzero fraction.

        o-

        <<isnan>> returns 1 if the argument is a nan. <<isinf>>
        returns 1 if the argument is infinity.  <<finite>> returns 1 if the
        argument is zero, subnormal or normal.

        The <<isnanf>>, <<isinff>> and <<finitef>> perform the same
        operations as their <<isnan>>, <<isinf>> and <<finite>>
        counterparts, but on single-precision floating-point numbers.

QUICKREF
        isnan - pure
QUICKREF
        isinf - pure
QUICKREF
        finite - pure
QUICKREF
        isnan - pure
QUICKREF
        isinf - pure
QUICKREF
        finite - pure
*/


/******************************************************************
 * isnan
 *
 * Input:
 *   x - pointer to a floating-point value
 *
 * Output:
 *   An integer that indicates if the number is NaN.
 *
 * Description:
 *   This routine returns an integer that indicates if the number
 *   passed in is NaN (1) or is finite (0).
 *
 *****************************************************************/

#include "fdlibm.h"
#include "zmath.h"

#ifndef _DOUBLE_IS_32BITS

int isnan (double x)
{
  __uint32_t lx, hx;
  int exp;

  EXTRACT_WORDS (hx, lx, x);
  exp = (hx & 0x7ff00000) >> 20;

  if ((exp == 0x7ff) && (hx & 0xf0000 || lx))
    return (1);
  else
    return (0);
}

#endif /* _DOUBLE_IS_32BITS */
