
/* @(#)z_sinf.c 1.0 98/08/13 */
/******************************************************************
 * Sine
 *
 * Input:
 *   x - floating point value
 *
 * Output:
 *   sine of x
 *
 * Description:
 *   This routine returns the sine of x.
 *
 *****************************************************************/

#include "fdlibm.h"
#include "zmath.h"

void
sincosf (float x,
        float *sinx,
        float *cosx)
{
  *sinx = sin (x);
  *cosx = cos (x);
}

#ifdef _DOUBLE_IS_32BITS

void
sincos (double x, double *sinx, double *cosx)
{
  *sinx = (double) sinf ((float) x);
  *cosx = (double) cosf ((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
