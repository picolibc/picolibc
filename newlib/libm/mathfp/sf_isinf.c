
/* @(#)z_isinff.c 1.0 98/08/13 */
/******************************************************************
 * isinff
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

int isinff (float x)
{
  __uint32_t wx;
  int exp;

  GET_FLOAT_WORD (wx, x);
  exp = (wx & 0x7f800000) >> 23;

  if ((exp == 0x7f8) && !(wx & 0xf0000))
    return (1);
  else
    return (0);
}
