
/* @(#)z_isinf.c 1.0 98/08/13 */
/******************************************************************
 * isinf
 *
 * Input:
 *   x - pointer to a floating point value
 *
 * Output:
 *   An integer that indicates if the number is infinite.
 *
 * Description:
 *   This routine returns an integer that indicates if the number
 *   passed in is infinite (1) or is finite (0).
 *
 *****************************************************************/

#include "fdlibm.h"
#include "zmath.h"

#ifndef _DOUBLE_IS_32BITS

int isinf (double x)
{
  __uint32_t lx, hx;
  int exp;

  EXTRACT_WORDS (hx, lx, x);
  exp = (hx & 0x7ff00000) >> 20;

  if ((exp == 0x7ff) && ((hx & 0xf0000 || lx) == 0))
    return (1);
  else
    return (0);
}

#endif /* _DOUBLE_IS_32BITS */
