
/* @(#)z_isnanf.c 1.0 98/08/13 */
/******************************************************************
 * isnanf
 *
 * Input:
 *   x - pointer to a floating point value
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

int
_DEFUN (isnanf, (float),
        float x)
{
  __int32_t wx;
  int exp;

  GET_FLOAT_WORD (wx, x);
  exp = (wx & 0x7f800000) >> 23;

  if ((exp == 0x7f8) && (wx & 0x7fffff))
    return (1);
  else
    return (0);
}


#ifdef _DOUBLE_IS_32BITS

int
_DEFUN (isnan, (double),
        double x)
{
  return isnanf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */

